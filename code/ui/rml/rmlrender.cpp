/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The bgfx side of the UI overlays. With bgfxbackend.cpp it is one of the two translation
// units that include bgfx.

#include "ui/rml/rmlrender.h"

#include "bgfxbackend.h"
#include "bgfxviews.hh"
#include "dbgprint.h"
#include "ui/rml/rmlrendermath.h"
#include "ui/rml/rmltexture.h"

#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>

#include <vs_debugdraw_fill_texture.bin.h>
#include <fs_debugdraw_fill_texture.bin.h>

#include <imgui.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>


// The imgui shader the frame quad uses ignores the model matrix, which is where each
// document fragment's translation has to travel; this pair honors it.
static const bgfx::EmbeddedShader _EmbeddedShaders[] = {
	BGFX_EMBEDDED_SHADER(vs_debugdraw_fill_texture),
	BGFX_EMBEDDED_SHADER(fs_debugdraw_fill_texture),
	BGFX_EMBEDDED_SHADER_END()
};

// RmlUi and Dear ImGui order their vertex members differently.
static bgfx::VertexLayout _VertexLayout;
static bgfx::VertexLayout _DevVertexLayout;

// What one geometry or texture may hold, what all of them may hold together, and the
// largest texture edge, whatever the device allows.
static const uint32_t UI_MAX_RESOURCE_BYTES = 64u * 1024u * 1024u;
static const uint64_t UI_MAX_GEOMETRY_BYTES = 128ull * 1024ull * 1024ull;
static const uint64_t UI_MAX_TEXTURE_BYTES = 128ull * 1024ull * 1024ull;
static const int UI_MAX_TEXTURE_DIMENSION = 4096;


// A compiled document fragment, submitted many times with different translations.
struct UIGeometry
{
	bgfx::VertexBufferHandle Vertices;
	bgfx::IndexBufferHandle Indices;
	unsigned int Bytes;
};


// RmlUi and Dear ImGui both read a zero handle as no texture, and bgfx hands out index
// zero, so texture handles cross either boundary biased by one.
static bgfx::TextureHandle Texture_Handle(Rml::TextureHandle handle)
{
	bgfx::TextureHandle texture = { (uint16_t)(handle - 1) };
	return(texture);
}


UIRmlBgfxRenderClass::UIRmlBgfxRenderClass(void) :
	IsReady(false),
	Program(bgfx::kInvalidHandle),
	Sampler(bgfx::kInvalidHandle),
	WhiteTexture(bgfx::kInvalidHandle),
	ViewX(0),
	ViewY(0),
	ViewWidth(0),
	ViewHeight(0),
	ScissorEnabled(false),
	Scissor(Rml::Rectanglei::MakeInvalid()),
	DevShortageLogged(false)
{
}


void UIRmlBgfxRenderClass::Report(char const * message)
{
	DebugString("UI renderer: %s\n", message);
}


bool UIRmlBgfxRenderClass::Init(void)
{
	if (IsReady) {
		return(true);
	}

	_VertexLayout.begin()
		.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		.end();

	_DevVertexLayout.begin()
		.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.end();

	bgfx::RendererType::Enum type = bgfx::getRendererType();
	bgfx::ShaderHandle vertexshader = bgfx::createEmbeddedShader(_EmbeddedShaders, type, "vs_debugdraw_fill_texture");
	bgfx::ShaderHandle fragmentshader = bgfx::createEmbeddedShader(_EmbeddedShaders, type, "fs_debugdraw_fill_texture");

	if (!bgfx::isValid(vertexshader) || !bgfx::isValid(fragmentshader)) {
		if (bgfx::isValid(vertexshader)) {
			bgfx::destroy(vertexshader);
		}
		if (bgfx::isValid(fragmentshader)) {
			bgfx::destroy(fragmentshader);
		}
		DebugString("UI: the overlay shaders are unavailable for %s\n", Backend_Renderer_Name());
		return(false);
	}

	bgfx::ProgramHandle program = bgfx::createProgram(vertexshader, fragmentshader, true);
	bgfx::UniformHandle sampler = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

	const unsigned int white = 0xFFFFFFFF;
	bgfx::TextureHandle whitetexture = bgfx::createTexture2D(1, 1, false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(&white, sizeof(white)));

	if (!bgfx::isValid(program) || !bgfx::isValid(sampler) || !bgfx::isValid(whitetexture)) {
		if (bgfx::isValid(program)) {
			bgfx::destroy(program);
		}
		if (bgfx::isValid(sampler)) {
			bgfx::destroy(sampler);
		}
		if (bgfx::isValid(whitetexture)) {
			bgfx::destroy(whitetexture);
		}
		DebugString("UI: the overlay renderer could not be created\n");
		return(false);
	}

	Program = program.idx;
	Sampler = sampler.idx;
	WhiteTexture = whitetexture.idx;
	Statistics = UIRenderStats();
	TextureBytes.clear();
	Clear_Error();
	IsReady = true;
	return(true);
}


// RmlUi has released everything through this object by now, so anything still counted is
// a leak worth a log line.
void UIRmlBgfxRenderClass::Shutdown(void)
{
	if (!IsReady) {
		return;
	}

	if (Statistics.GeometryCount != 0 || Statistics.TextureCount != 0) {
		DebugString("UI renderer: %u geometries and %u textures were never released\n", Statistics.GeometryCount, Statistics.TextureCount);
	}

	bgfx::TextureHandle whitetexture = { WhiteTexture };
	bgfx::UniformHandle sampler = { Sampler };
	bgfx::ProgramHandle program = { Program };

	bgfx::destroy(whitetexture);
	bgfx::destroy(sampler);
	bgfx::destroy(program);

	WhiteTexture = bgfx::kInvalidHandle;
	Sampler = bgfx::kInvalidHandle;
	Program = bgfx::kInvalidHandle;
	Statistics = UIRenderStats();
	TextureBytes.clear();
	IsReady = false;
}


// View state persists across frames and resets, and the prescale pass binds a framebuffer
// to a lower view, so everything the overlays rely on is set again each frame.
void UIRmlBgfxRenderClass::Set_View(unsigned short view, int x, int y, int width, int height)
{
	ViewX = x;
	ViewY = y;
	ViewWidth = width;
	ViewHeight = height;

	float projection[16];
	Backend_Build_Ortho_Projection(projection, width, height);

	bgfx::setViewFrameBuffer(view, BGFX_INVALID_HANDLE);
	bgfx::setViewMode(view, bgfx::ViewMode::Sequential);
	bgfx::setViewRect(view, (uint16_t)x, (uint16_t)y, (uint16_t)width, (uint16_t)height);
	bgfx::setViewTransform(view, NULL, projection);
}


void UIRmlBgfxRenderClass::Begin_Frame(int x, int y, int width, int height)
{
	Statistics.DrawCalls = 0;
	Set_View(VIEW_UI, x, y, width, height);
}


void UIRmlBgfxRenderClass::Begin_Dev_Frame(int x, int y, int width, int height)
{
	Set_View(VIEW_DEV, x, y, width, height);
}


// The policy cap keeps a document from asking for a texture the device would allow but the
// process should not spend; Dear ImGui sizes its atlas by this too.
int UIRmlBgfxRenderClass::Texture_Limit(void) const
{
	if (!IsReady) {
		return(0);
	}

	int limit = (int)bgfx::getCaps()->limits.maxTextureSize;
	return(limit < UI_MAX_TEXTURE_DIMENSION ? limit : UI_MAX_TEXTURE_DIMENSION);
}


void UIRmlBgfxRenderClass::Log_Resource_Counts(char const * when) const
{
	DebugString("UI: %s; %u geometries (%llu bytes), %u textures (%llu bytes)%s%s\n",
				when, Statistics.GeometryCount, (unsigned long long)Statistics.GeometryBytes,
				Statistics.TextureCount, (unsigned long long)Statistics.TextureBytes,
				Error()[0] != '\0' ? "; error: " : "", Error());
}


// Two calls stay in reserve for the presenter's own quads.
bool UIRmlBgfxRenderClass::Draw_Available(void)
{
	if ((uint64_t)Statistics.DrawCalls + 2 >= bgfx::getCaps()->limits.maxDrawCalls) {
		return(Fail("the frame's draw-call limit was reached"));
	}
	return(true);
}


bool UIRmlBgfxRenderClass::Record_Texture(unsigned short index, unsigned int bytes)
{
	TextureBytes[index] = bytes;
	Statistics.TextureCount++;
	Statistics.TextureBytes += bytes;
	return(true);
}


void UIRmlBgfxRenderClass::Forget_Texture(unsigned short index)
{
	std::unordered_map<unsigned short, unsigned int>::iterator entry = TextureBytes.find(index);
	if (entry != TextureBytes.end()) {
		Statistics.TextureCount--;
		Statistics.TextureBytes -= entry->second;
		TextureBytes.erase(entry);
	}
}


Rml::CompiledGeometryHandle UIRmlBgfxRenderClass::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
{
	if (!IsReady) {
		return(0);
	}
	if (!UI_Render_Index_Range(std::span<int const>(indices.data(), indices.size()), vertices.size())) {
		Fail("a fragment's indices do not name whole triangles over its vertices");
		return(0);
	}
	for (Rml::Vertex const & vertex : vertices) {
		if (!std::isfinite(vertex.position.x) || !std::isfinite(vertex.position.y) || !std::isfinite(vertex.tex_coord.x) || !std::isfinite(vertex.tex_coord.y)) {
			Fail("a fragment holds a vertex that is not finite");
			return(0);
		}
	}

	uint32_t vertexbytes = 0;
	uint32_t indexbytes = 0;
	bool wideindices = (bgfx::getCaps()->supported & BGFX_CAPS_INDEX32) != 0;
	if (!UI_Render_Byte_Count(vertices.size(), sizeof(Rml::Vertex), vertexbytes)
		|| !UI_Render_Byte_Count(indices.size(), wideindices ? sizeof(int) : sizeof(uint16_t), indexbytes)) {
		Fail("a fragment is too large to measure");
		return(0);
	}
	if (!wideindices && vertices.size() > 65536) {
		Fail("a fragment over 65536 vertices needs 32-bit indices, which this renderer lacks");
		return(0);
	}
	if ((uint64_t)vertexbytes + indexbytes > UI_MAX_RESOURCE_BYTES) {
		Fail("a fragment is larger than the renderer accepts");
		return(0);
	}
	if (Statistics.GeometryBytes + vertexbytes + indexbytes > UI_MAX_GEOMETRY_BYTES) {
		Fail("the documents hold more geometry than the renderer accepts");
		return(0);
	}

	bgfx::VertexBufferHandle vertexbuffer = bgfx::createVertexBuffer(bgfx::copy(vertices.data(), vertexbytes), _VertexLayout);

	bgfx::IndexBufferHandle indexbuffer;
	if (wideindices) {
		indexbuffer = bgfx::createIndexBuffer(bgfx::copy(indices.data(), indexbytes), BGFX_BUFFER_INDEX32);
	} else {
		std::vector<uint16_t> narrow(indices.size());
		for (size_t index = 0; index < indices.size(); index++) {
			narrow[index] = (uint16_t)indices[index];
		}
		indexbuffer = bgfx::createIndexBuffer(bgfx::copy(narrow.data(), indexbytes));
	}

	if (!bgfx::isValid(vertexbuffer) || !bgfx::isValid(indexbuffer)) {
		if (bgfx::isValid(vertexbuffer)) {
			bgfx::destroy(vertexbuffer);
		}
		if (bgfx::isValid(indexbuffer)) {
			bgfx::destroy(indexbuffer);
		}
		Fail("a fragment's buffers could not be created");
		return(0);
	}

	UIGeometry * geometry = new UIGeometry;
	geometry->Vertices = vertexbuffer;
	geometry->Indices = indexbuffer;
	geometry->Bytes = vertexbytes + indexbytes;
	Statistics.GeometryCount++;
	Statistics.GeometryBytes += geometry->Bytes;
	return((Rml::CompiledGeometryHandle)geometry);
}


void UIRmlBgfxRenderClass::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture)
{
	if (!IsReady || handle == 0) {
		return;
	}
	if (!std::isfinite(translation.x) || !std::isfinite(translation.y)) {
		Fail("a fragment's translation is not finite");
		return;
	}
	if (!Draw_Available()) {
		return;
	}

	if (ScissorEnabled && !Apply_Scissor()) {
		return;
	}

	UIGeometry const * geometry = (UIGeometry const *)handle;

	float transform[16];
	memset(transform, 0, sizeof(transform));
	transform[0] = 1.0f;
	transform[5] = 1.0f;
	transform[10] = 1.0f;
	transform[12] = translation.x;
	transform[13] = translation.y;
	transform[15] = 1.0f;
	bgfx::setTransform(transform);

	bgfx::TextureHandle sampled = { WhiteTexture };
	if (texture != 0) {
		sampled = Texture_Handle(texture);
	}

	bgfx::UniformHandle sampler = { Sampler };
	bgfx::ProgramHandle program = { Program };

	bgfx::setVertexBuffer(0, geometry->Vertices);
	bgfx::setIndexBuffer(geometry->Indices);
	bgfx::setTexture(0, sampler, sampled, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA));
	bgfx::submit(VIEW_UI, program);
	Statistics.DrawCalls++;
}


void UIRmlBgfxRenderClass::ReleaseGeometry(Rml::CompiledGeometryHandle handle)
{
	if (handle == 0) {
		return;
	}

	UIGeometry * geometry = (UIGeometry *)handle;
	bgfx::destroy(geometry->Vertices);
	bgfx::destroy(geometry->Indices);
	Statistics.GeometryCount--;
	Statistics.GeometryBytes -= geometry->Bytes;
	delete geometry;
}


Rml::TextureHandle UIRmlBgfxRenderClass::LoadTexture(Rml::Vector2i & dimensions, Rml::String const & source)
{
	std::vector<unsigned char> rgba;
	int width = 0;
	int height = 0;

	if (!UI_Load_Image(source.c_str(), rgba, width, height)) {
		char message[320];
		std::snprintf(message, sizeof(message), "%s did not decode", source.c_str());
		Fail(message);
		return(0);
	}

	dimensions.x = width;
	dimensions.y = height;
	return(GenerateTexture(Rml::Span<const Rml::byte>(rgba.data(), rgba.size()), dimensions));
}


Rml::TextureHandle UIRmlBgfxRenderClass::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i dimensions)
{
	if (!IsReady) {
		return(0);
	}

	int limit = Texture_Limit();
	if (dimensions.x <= 0 || dimensions.y <= 0 || dimensions.x > limit || dimensions.y > limit) {
		Fail("a texture is empty or larger on a side than the renderer accepts");
		return(0);
	}

	uint32_t size = 0;
	if (!UI_Render_Byte_Count((size_t)dimensions.x * (size_t)dimensions.y, 4, size) || size > UI_MAX_RESOURCE_BYTES) {
		Fail("a texture is larger than the renderer accepts");
		return(0);
	}
	if (source.size() != size) {
		Fail("a texture's pixels do not match its dimensions");
		return(0);
	}
	if (Statistics.TextureBytes + size > UI_MAX_TEXTURE_BYTES) {
		Fail("the documents hold more texture than the renderer accepts");
		return(0);
	}

	bgfx::TextureHandle texture = bgfx::createTexture2D((uint16_t)dimensions.x, (uint16_t)dimensions.y, false, 1, bgfx::TextureFormat::RGBA8, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP, bgfx::copy(source.data(), size));
	if (!bgfx::isValid(texture)) {
		Fail("a texture could not be created");
		return(0);
	}

	Record_Texture(texture.idx, size);
	return((Rml::TextureHandle)texture.idx + 1);
}


void UIRmlBgfxRenderClass::ReleaseTexture(Rml::TextureHandle texture)
{
	if (texture == 0) {
		return;
	}

	bgfx::TextureHandle handle = Texture_Handle(texture);
	if (TextureBytes.find(handle.idx) == TextureBytes.end()) {
		Fail("a texture the renderer does not hold was released");
		return;
	}

	Forget_Texture(handle.idx);
	bgfx::destroy(handle);
}


void UIRmlBgfxRenderClass::EnableScissorRegion(bool enable)
{
	ScissorEnabled = enable;
}


void UIRmlBgfxRenderClass::SetScissorRegion(Rml::Rectanglei region)
{
	Scissor = region;
}


// bgfx scissors are window pixels, while RmlUi clips in the overlay's own; a region that
// clips everything away reports false so the draw can be skipped.
bool UIRmlBgfxRenderClass::Apply_Scissor(void) const
{
	if (!Scissor.Valid()) {
		return(false);
	}

	UIRenderClip clip;
	if (!UI_Render_Clip_Rect((float)Scissor.Left(), (float)Scissor.Top(), (float)Scissor.Right(), (float)Scissor.Bottom(), ViewX, ViewY, ViewWidth, ViewHeight, clip)) {
		return(false);
	}

	bgfx::setScissor(clip.X, clip.Y, clip.Width, clip.Height);
	return(true);
}


// Dear ImGui asks for its textures through status requests; each is answered here and
// acknowledged, and a destroyed texture keeps its pixels so that ImGui can ask again. A
// request the renderer refuses is left unanswered, so ImGui asks again next frame.
void UIRmlBgfxRenderClass::Update_ImGui_Texture(ImTextureData * texture)
{
	if (texture->Status == ImTextureStatus_WantCreate) {
		int limit = Texture_Limit();
		uint32_t size = 0;
		if (texture->Format != ImTextureFormat_RGBA32) {
			Fail("an overlay texture is not RGBA");
			return;
		}
		if (texture->Width <= 0 || texture->Height <= 0 || texture->Width > limit || texture->Height > limit) {
			Fail("an overlay texture is empty or larger on a side than the renderer accepts");
			return;
		}
		if (!UI_Render_Byte_Count((size_t)texture->Width * (size_t)texture->Height, 4, size) || size > UI_MAX_RESOURCE_BYTES
			|| Statistics.TextureBytes + size > UI_MAX_TEXTURE_BYTES || (size_t)texture->GetSizeInBytes() != size) {
			Fail("an overlay texture is larger than the renderer accepts");
			return;
		}

		// A texture created with its pixels is immutable in bgfx, and the atlas keeps growing.
		bgfx::TextureHandle handle = bgfx::createTexture2D((uint16_t)texture->Width, (uint16_t)texture->Height, false, 1, bgfx::TextureFormat::RGBA8, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
		if (!bgfx::isValid(handle)) {
			DebugString("UI: a %dx%d overlay texture could not be created\n", texture->Width, texture->Height);
			return;
		}

		bgfx::updateTexture2D(handle, 0, 0, 0, 0, (uint16_t)texture->Width, (uint16_t)texture->Height, bgfx::copy(texture->GetPixels(), size));
		Record_Texture(handle.idx, size);
		texture->SetTexID((ImTextureID)handle.idx + 1);
		texture->SetStatus(ImTextureStatus_OK);
	} else if (texture->Status == ImTextureStatus_WantUpdates) {
		bgfx::TextureHandle handle = { (uint16_t)(texture->TexID - 1) };
		int pitch = texture->GetPitch();

		for (ImTextureRect const & rect : texture->Updates) {
			uint32_t size = (uint32_t)((rect.h - 1) * pitch + rect.w * texture->BytesPerPixel);
			bgfx::updateTexture2D(handle, 0, 0, rect.x, rect.y, rect.w, rect.h, bgfx::copy(texture->GetPixelsAt(rect.x, rect.y), size), (uint16_t)pitch);
		}

		texture->SetStatus(ImTextureStatus_OK);
	}

	if (texture->Status == ImTextureStatus_WantDestroy && texture->UnusedFrames > 0) {
		if (texture->TexID != ImTextureID_Invalid) {
			bgfx::TextureHandle handle = { (uint16_t)(texture->TexID - 1) };
			Forget_Texture(handle.idx);
			bgfx::destroy(handle);
			texture->SetTexID(ImTextureID_Invalid);
		}
		texture->SetStatus(ImTextureStatus_Destroyed);
	}
}


void UIRmlBgfxRenderClass::Destroy_ImGui_Textures(void)
{
	for (ImTextureData * texture : ImGui::GetPlatformIO().Textures) {
		if (texture->TexID != ImTextureID_Invalid) {
			bgfx::TextureHandle handle = { (uint16_t)(texture->TexID - 1) };
			Forget_Texture(handle.idx);
			bgfx::destroy(handle);
			texture->SetTexID(ImTextureID_Invalid);
		}
		texture->SetStatus(ImTextureStatus_Destroyed);
	}
}


// ImGui rebuilds its geometry every frame, so it travels in transient buffers; its colors
// carry straight alpha, unlike the premultiplied documents.
void UIRmlBgfxRenderClass::Render_ImGui(ImDrawData * data)
{
	if (!IsReady || data == NULL || !data->Valid || data->DisplaySize.x <= 0.0f || data->DisplaySize.y <= 0.0f) {
		return;
	}

	if (data->Textures != NULL) {
		for (ImTextureData * texture : *data->Textures) {
			if (texture->Status != ImTextureStatus_OK) {
				Update_ImGui_Texture(texture);
			}
		}
	}

	float identity[16];
	memset(identity, 0, sizeof(identity));
	identity[0] = 1.0f;
	identity[5] = 1.0f;
	identity[10] = 1.0f;
	identity[15] = 1.0f;

	bgfx::UniformHandle sampler = { Sampler };
	bgfx::ProgramHandle program = { Program };
	ImDrawCallback resetstate = ImGui::GetPlatformIO().DrawCallback_ResetRenderState;

	for (ImDrawList const * list : data->CmdLists) {
		uint32_t vertexcount = (uint32_t)list->VtxBuffer.Size;
		uint32_t indexcount = (uint32_t)list->IdxBuffer.Size;
		if (vertexcount == 0 || indexcount == 0) {
			continue;
		}

		if (bgfx::getAvailTransientVertexBuffer(vertexcount, _DevVertexLayout) < vertexcount || bgfx::getAvailTransientIndexBuffer(indexcount) < indexcount) {
			if (!DevShortageLogged) {
				DebugString("UI: an overlay draw list did not fit the transient buffers and was skipped\n");
				DevShortageLogged = true;
			}
			continue;
		}

		bgfx::TransientVertexBuffer vertices;
		bgfx::TransientIndexBuffer indices;
		bgfx::allocTransientVertexBuffer(&vertices, vertexcount, _DevVertexLayout);
		bgfx::allocTransientIndexBuffer(&indices, indexcount);
		memcpy(vertices.data, list->VtxBuffer.Data, vertexcount * sizeof(ImDrawVert));
		memcpy(indices.data, list->IdxBuffer.Data, indexcount * sizeof(ImDrawIdx));

		for (ImDrawCmd const & command : list->CmdBuffer) {
			if (command.UserCallback != NULL) {
				if (command.UserCallback != resetstate) {
					command.UserCallback(list, &command);
				}
				continue;
			}
			if (command.ElemCount == 0) {
				continue;
			}

			UIRenderClip clip;
			if (!UI_Render_Clip_Rect(command.ClipRect.x - data->DisplayPos.x, command.ClipRect.y - data->DisplayPos.y,
									 command.ClipRect.z - data->DisplayPos.x, command.ClipRect.w - data->DisplayPos.y,
									 ViewX, ViewY, ViewWidth, ViewHeight, clip)) {
				continue;
			}
			if (!Draw_Available()) {
				return;
			}

			bgfx::TextureHandle sampled = { WhiteTexture };
			ImTextureID id = command.GetTexID();
			if (id != ImTextureID_Invalid) {
				sampled.idx = (uint16_t)(id - 1);
			}

			bgfx::setScissor(clip.X, clip.Y, clip.Width, clip.Height);
			bgfx::setTransform(identity);
			bgfx::setVertexBuffer(0, &vertices, command.VtxOffset, vertexcount - command.VtxOffset);
			bgfx::setIndexBuffer(&indices, command.IdxOffset, command.ElemCount);
			bgfx::setTexture(0, sampler, sampled, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA, BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA));
			bgfx::submit(VIEW_DEV, program);
			Statistics.DrawCalls++;
		}
	}
}
