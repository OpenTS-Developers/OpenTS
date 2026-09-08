/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The bgfx side of the UI overlay. With bgfxbackend.cpp it is one of the two translation
// units that include bgfx.

#include "ui/uirender.h"

#include "bgfxbackend.h"
#include "bgfxviews.hh"
#include "dbgprint.h"
#include "ui/uitexture.h"

#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>

#include <vs_debugdraw_fill_texture.bin.h>
#include <fs_debugdraw_fill_texture.bin.h>

#include <cassert>
#include <cstring>
#include <vector>


// The imgui shader the frame quad uses ignores the model matrix, which is where each
// document fragment's translation has to travel; this pair honors it.
static const bgfx::EmbeddedShader _EmbeddedShaders[] = {
	BGFX_EMBEDDED_SHADER(vs_debugdraw_fill_texture),
	BGFX_EMBEDDED_SHADER(fs_debugdraw_fill_texture),
	BGFX_EMBEDDED_SHADER_END()
};

static bgfx::VertexLayout _VertexLayout;


// A compiled document fragment, submitted many times with different translations.
struct UIGeometry
{
	bgfx::VertexBufferHandle Vertices;
	bgfx::IndexBufferHandle Indices;
};


// RmlUi reads a zero handle as no texture, and bgfx hands out index zero, so texture
// handles cross the boundary biased by one.
static bgfx::TextureHandle Texture_Handle(Rml::TextureHandle handle)
{
	bgfx::TextureHandle texture = { (uint16_t)(handle - 1) };
	return(texture);
}


UIRenderInterfaceClass::UIRenderInterfaceClass(void) :
	IsReady(false),
	Program(bgfx::kInvalidHandle),
	Sampler(bgfx::kInvalidHandle),
	WhiteTexture(bgfx::kInvalidHandle),
	ViewX(0),
	ViewY(0),
	ViewWidth(0),
	ViewHeight(0),
	ScissorEnabled(false),
	Scissor(Rml::Rectanglei::MakeInvalid())
{
}


bool UIRenderInterfaceClass::Init(void)
{
	if (IsReady) {
		return(true);
	}

	_VertexLayout.begin()
		.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
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
	IsReady = true;
	return(true);
}


void UIRenderInterfaceClass::Shutdown(void)
{
	if (!IsReady) {
		return;
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
	IsReady = false;
}


// View state persists across frames and resets, and the prescale pass binds a framebuffer
// to a lower view, so everything the overlay relies on is set again each frame.
void UIRenderInterfaceClass::Begin_Frame(int x, int y, int width, int height)
{
	ViewX = x;
	ViewY = y;
	ViewWidth = width;
	ViewHeight = height;

	float projection[16];
	Backend_Build_Ortho_Projection(projection, width, height);

	bgfx::setViewFrameBuffer(VIEW_UI, BGFX_INVALID_HANDLE);
	bgfx::setViewMode(VIEW_UI, bgfx::ViewMode::Sequential);
	bgfx::setViewRect(VIEW_UI, (uint16_t)x, (uint16_t)y, (uint16_t)width, (uint16_t)height);
	bgfx::setViewTransform(VIEW_UI, NULL, projection);
}


void UIRenderInterfaceClass::Log_Resource_Counts(char const * when) const
{
	bgfx::Stats const * stats = bgfx::getStats();
	if (stats == NULL) {
		return;
	}

	DebugString("UI: %s; renderer holds %u textures, %u vertex buffers, %u index buffers\n",
				when, (unsigned)stats->numTextures, (unsigned)stats->numVertexBuffers, (unsigned)stats->numIndexBuffers);
}


Rml::CompiledGeometryHandle UIRenderInterfaceClass::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
{
	if (!IsReady || vertices.empty() || indices.empty()) {
		return(0);
	}

	bgfx::VertexBufferHandle vertexbuffer = bgfx::createVertexBuffer(bgfx::copy(vertices.data(), (uint32_t)(vertices.size() * sizeof(Rml::Vertex))), _VertexLayout);

	bgfx::IndexBufferHandle indexbuffer;
	if ((bgfx::getCaps()->supported & BGFX_CAPS_INDEX32) != 0) {
		indexbuffer = bgfx::createIndexBuffer(bgfx::copy(indices.data(), (uint32_t)(indices.size() * sizeof(int))), BGFX_BUFFER_INDEX32);
	} else {
		assert(vertices.size() <= 65536);
		std::vector<uint16_t> narrow(indices.size());
		for (size_t index = 0; index < indices.size(); index++) {
			narrow[index] = (uint16_t)indices[index];
		}
		indexbuffer = bgfx::createIndexBuffer(bgfx::copy(narrow.data(), (uint32_t)(narrow.size() * sizeof(uint16_t))));
	}

	if (!bgfx::isValid(vertexbuffer) || !bgfx::isValid(indexbuffer)) {
		if (bgfx::isValid(vertexbuffer)) {
			bgfx::destroy(vertexbuffer);
		}
		if (bgfx::isValid(indexbuffer)) {
			bgfx::destroy(indexbuffer);
		}
		return(0);
	}

	UIGeometry * geometry = new UIGeometry;
	geometry->Vertices = vertexbuffer;
	geometry->Indices = indexbuffer;
	return((Rml::CompiledGeometryHandle)geometry);
}


void UIRenderInterfaceClass::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture)
{
	if (!IsReady || handle == 0) {
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
}


void UIRenderInterfaceClass::ReleaseGeometry(Rml::CompiledGeometryHandle handle)
{
	if (handle == 0) {
		return;
	}

	UIGeometry * geometry = (UIGeometry *)handle;
	bgfx::destroy(geometry->Vertices);
	bgfx::destroy(geometry->Indices);
	delete geometry;
}


Rml::TextureHandle UIRenderInterfaceClass::LoadTexture(Rml::Vector2i & dimensions, Rml::String const & source)
{
	std::vector<unsigned char> rgba;
	int width = 0;
	int height = 0;

	if (!UI_Load_Image(source.c_str(), rgba, width, height)) {
		return(0);
	}

	dimensions.x = width;
	dimensions.y = height;
	return(GenerateTexture(Rml::Span<const Rml::byte>(rgba.data(), rgba.size()), dimensions));
}


Rml::TextureHandle UIRenderInterfaceClass::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i dimensions)
{
	if (!IsReady || dimensions.x <= 0 || dimensions.y <= 0) {
		return(0);
	}

	uint32_t size = (uint32_t)dimensions.x * (uint32_t)dimensions.y * 4;
	if (source.size() < size) {
		return(0);
	}

	bgfx::TextureHandle texture = bgfx::createTexture2D((uint16_t)dimensions.x, (uint16_t)dimensions.y, false, 1, bgfx::TextureFormat::RGBA8, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP, bgfx::copy(source.data(), size));
	if (!bgfx::isValid(texture)) {
		return(0);
	}

	return((Rml::TextureHandle)texture.idx + 1);
}


void UIRenderInterfaceClass::ReleaseTexture(Rml::TextureHandle texture)
{
	if (texture == 0) {
		return;
	}

	bgfx::destroy(Texture_Handle(texture));
}


void UIRenderInterfaceClass::EnableScissorRegion(bool enable)
{
	ScissorEnabled = enable;
}


void UIRenderInterfaceClass::SetScissorRegion(Rml::Rectanglei region)
{
	Scissor = region;
}


// bgfx scissors are window pixels, while RmlUi clips in the overlay's own; a region that
// clips everything away reports false so the draw can be skipped.
bool UIRenderInterfaceClass::Apply_Scissor(void) const
{
	if (!Scissor.Valid()) {
		return(false);
	}

	int left = ViewX + Scissor.Left();
	int top = ViewY + Scissor.Top();
	int right = left + Scissor.Width();
	int bottom = top + Scissor.Height();

	if (left < ViewX) left = ViewX;
	if (top < ViewY) top = ViewY;
	if (right > ViewX + ViewWidth) right = ViewX + ViewWidth;
	if (bottom > ViewY + ViewHeight) bottom = ViewY + ViewHeight;

	if (right <= left || bottom <= top) {
		return(false);
	}

	bgfx::setScissor((uint16_t)left, (uint16_t)top, (uint16_t)(right - left), (uint16_t)(bottom - top));
	return(true);
}
