/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins what the UI shell relies on without a window, a renderer or game data: the vendored
// toolkits start and stop under the engine's link settings, every shipped document loads
// and draws through the render interface methods the shell implements and none it does
// not, the documents name their resources the way the shell resolves them, and the
// pointer mapping into the overlay behaves at the frame's edges.

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <RmlUi/Core.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <imgui.h>

#include "ui/uicoord.h"

namespace {

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-76s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


// Counts every call RmlUi makes while a document is laid out and drawn. The methods the
// shell leaves at their defaults count as violations of the styling profile the shipped
// documents must stay within.
class RecordingRenderInterfaceClass : public Rml::RenderInterface
{
	public:
		int Compiled = 0;
		int Rendered = 0;
		int ReleasedGeometry = 0;
		int Loaded = 0;
		int Generated = 0;
		int ReleasedTextures = 0;
		int Unsupported = 0;
		std::vector<Rml::Rectanglei> Scissors;

		virtual Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex>, Rml::Span<const int>) override
		{
			Compiled++;
			return((Rml::CompiledGeometryHandle)Compiled);
		}

		virtual void RenderGeometry(Rml::CompiledGeometryHandle, Rml::Vector2f, Rml::TextureHandle) override
		{
			Rendered++;
		}

		virtual void ReleaseGeometry(Rml::CompiledGeometryHandle) override
		{
			ReleasedGeometry++;
		}

		virtual Rml::TextureHandle LoadTexture(Rml::Vector2i & dimensions, Rml::String const &) override
		{
			Loaded++;
			dimensions = Rml::Vector2i(1, 1);
			return((Rml::TextureHandle)(Loaded + Generated));
		}

		virtual Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte>, Rml::Vector2i) override
		{
			Generated++;
			return((Rml::TextureHandle)(Loaded + Generated));
		}

		virtual void ReleaseTexture(Rml::TextureHandle) override
		{
			ReleasedTextures++;
		}

		virtual void EnableScissorRegion(bool) override
		{
		}

		virtual void SetScissorRegion(Rml::Rectanglei region) override
		{
			Scissors.push_back(region);
		}

		virtual void EnableClipMask(bool enable) override
		{
			if (enable) {
				Unsupported++;
			}
		}

		virtual void RenderToClipMask(Rml::ClipMaskOperation, Rml::CompiledGeometryHandle, Rml::Vector2f) override
		{
			Unsupported++;
		}

		virtual void SetTransform(Rml::Matrix4f const * transform) override
		{
			if (transform != nullptr) {
				Unsupported++;
			}
		}

		virtual Rml::LayerHandle PushLayer(void) override
		{
			Unsupported++;
			return(0);
		}

		virtual void CompositeLayers(Rml::LayerHandle, Rml::LayerHandle, Rml::BlendMode, Rml::Span<const Rml::CompiledFilterHandle>) override
		{
			Unsupported++;
		}

		virtual void PopLayer(void) override
		{
			Unsupported++;
		}

		virtual Rml::TextureHandle SaveLayerAsTexture(void) override
		{
			Unsupported++;
			return(0);
		}

		virtual Rml::CompiledFilterHandle SaveLayerAsMaskImage(void) override
		{
			Unsupported++;
			return(0);
		}

		virtual Rml::CompiledFilterHandle CompileFilter(Rml::String const &, Rml::Dictionary const &) override
		{
			Unsupported++;
			return(0);
		}

		virtual void ReleaseFilter(Rml::CompiledFilterHandle) override
		{
		}

		virtual Rml::CompiledShaderHandle CompileShader(Rml::String const &, Rml::Dictionary const &) override
		{
			Unsupported++;
			return(0);
		}

		virtual void RenderShader(Rml::CompiledShaderHandle, Rml::CompiledGeometryHandle, Rml::Vector2f, Rml::TextureHandle) override
		{
			Unsupported++;
		}

		virtual void ReleaseShader(Rml::CompiledShaderHandle) override
		{
		}
};


class CountingSystemInterfaceClass : public Rml::SystemInterface
{
	public:
		int Problems = 0;

		virtual bool LogMessage(Rml::Log::Type type, Rml::String const & message) override
		{
			if (type == Rml::Log::LT_ERROR || type == Rml::Log::LT_ASSERT || type == Rml::Log::LT_WARNING) {
				Problems++;
				std::printf("  RmlUi: %s\n", message.c_str());
			}
			return(true);
		}
};


std::string Read_Text(std::filesystem::path const & path)
{
	std::ifstream stream(path, std::ios::binary);
	return(std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>()));
}


// A resource reference is what follows one of the attribute or function openers up to its
// closing quote or parenthesis.
bool References_Are_Bare(std::string const & text)
{
	static char const * const openers[] = { "href=\"", "src=\"", "url(" };

	for (char const * opener : openers) {
		size_t at = text.find(opener);
		while (at != std::string::npos) {
			size_t start = at + std::strlen(opener);
			size_t end = text.find_first_of("\")", start);
			if (end == std::string::npos) {
				return(false);
			}
			std::string reference = text.substr(start, end - start);
			if (reference.find_first_of("/\\") != std::string::npos) {
				std::printf("  %s is not a bare name\n", reference.c_str());
				return(false);
			}
			at = text.find(opener, end);
		}
	}

	return(true);
}


void Test_FreeType(void)
{
	FT_Library library = nullptr;
	Check(FT_Init_FreeType(&library) == 0 && library != nullptr, "FreeType initialises a library");

	if (library != nullptr) {
		FT_Int major = 0;
		FT_Int minor = 0;
		FT_Int patch = 0;
		FT_Library_Version(library, &major, &minor, &patch);
		std::printf("  FreeType %d.%d.%d\n", major, minor, patch);
		Check(major == 2, "FreeType reports the 2.x API");

		FT_Done_FreeType(library);
	}
}


void Draw_ImGui_Test_Window(void)
{
	ImGui::SetNextWindowSize(ImVec2(400.0f, 300.0f), ImGuiCond_Always);
	ImGui::Begin("Test window");
	ImGui::TextUnformatted("A frame drawn without a renderer.");
	if (ImGui::BeginTable("rows", 3, ImGuiTableFlags_Borders)) {
		ImGui::TableSetupColumn("Process");
		ImGui::TableSetupColumn("Frame %");
		ImGui::TableSetupColumn("Average");
		ImGui::TableHeadersRow();
		for (int row = 0; row < 3; row++) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Row %d", row);
			ImGui::TableNextColumn();
			ImGui::Text("%.1f", row * 10.0f);
			ImGui::TableNextColumn();
			ImGui::Text("%d", row * 100);
		}
		ImGui::EndTable();
	}
	ImGui::End();
}


// The shell drives Dear ImGui through the 1.92 texture contract; this walks that contract
// with the harness standing in for the renderer.
void Test_ImGui(void)
{
	Check(IMGUI_CHECKVERSION(), "ImGui header and library agree on structure layouts");

	ImGuiContext * context = ImGui::CreateContext();
	Check(context != nullptr, "ImGui creates a context");
	std::printf("  Dear ImGui %s\n", ImGui::GetVersion());
	if (context == nullptr) {
		return;
	}

	ImGuiIO & io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;
	io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures | ImGuiBackendFlags_RendererHasVtxOffset;
	io.DisplaySize = ImVec2(1280.0f, 800.0f);
	io.DeltaTime = 1.0f / 60.0f;

	ImGui::NewFrame();
	Draw_ImGui_Test_Window();
	ImGui::Render();

	ImDrawData * data = ImGui::GetDrawData();
	Check(data != nullptr && data->Valid, "the first ImGui frame produces draw data");
	Check(data != nullptr && data->CmdLists.Size > 0 && data->TotalVtxCount > 0, "the first ImGui frame draws geometry");

	ImTextureData * atlas = nullptr;
	if (data != nullptr && data->Textures != nullptr && data->Textures->Size == 1) {
		atlas = (*data->Textures)[0];
	}
	Check(atlas != nullptr, "the first ImGui frame lists one texture, the font atlas");
	Check(atlas != nullptr && atlas->Status == ImTextureStatus_WantCreate, "the font atlas asks to be created");
	Check(atlas != nullptr && atlas->Format == ImTextureFormat_RGBA32 && atlas->Width > 0 && atlas->Height > 0, "the font atlas is RGBA32 with a size");

	if (atlas != nullptr) {
		atlas->SetTexID((ImTextureID)1);
		atlas->SetStatus(ImTextureStatus_OK);
	}

	ImGui::NewFrame();
	Draw_ImGui_Test_Window();
	ImGui::Render();
	data = ImGui::GetDrawData();

	bool acknowledged = data != nullptr && data->Textures != nullptr;
	if (acknowledged) {
		for (ImTextureData * texture : *data->Textures) {
			if (texture->Status != ImTextureStatus_OK) {
				acknowledged = false;
			}
		}
	}
	Check(acknowledged, "the second ImGui frame leaves every texture acknowledged");

	bool textured = data != nullptr;
	if (textured) {
		for (ImDrawList const * list : data->CmdLists) {
			for (ImDrawCmd const & command : list->CmdBuffer) {
				if (command.UserCallback == nullptr && command.ElemCount > 0 && command.GetTexID() == ImTextureID_Invalid) {
					textured = false;
				}
			}
		}
	}
	Check(textured, "every ImGui draw command carries a texture id");

	ImGui::DestroyContext(context);
}


void Test_Coordinates(void)
{
	UIPointerPosition position = UI_Client_To_Overlay(0, 0, 640, 480, 100, 200);
	Check(position.Inside && position.X == 100 && position.Y == 200, "an unscaled frame maps client pixels to itself");

	// A 640x480 frame doubled into a 1280x720 window sits at x 160 with bars either side.
	position = UI_Client_To_Overlay(160, 0, 960, 720, 160, 0);
	Check(position.Inside && position.X == 0 && position.Y == 0, "the top left corner of the frame is inside at the origin");
	position = UI_Client_To_Overlay(160, 0, 960, 720, 159, 10);
	Check(!position.Inside && position.X == -1, "a point on the left bar is outside and keeps its offset");
	position = UI_Client_To_Overlay(160, 0, 960, 720, 1119, 719);
	Check(position.Inside && position.X == 959 && position.Y == 719, "the last pixel of the frame is inside");
	position = UI_Client_To_Overlay(160, 0, 960, 720, 1120, 719);
	Check(!position.Inside && position.X == 960, "the right edge is exclusive");

	// A 640x480 frame at one and a half times fills a 960x720 window exactly.
	position = UI_Client_To_Overlay(0, 0, 960, 720, 959, 719);
	Check(position.Inside, "a fractional scale keeps its last pixel inside");
	position = UI_Client_To_Overlay(0, 0, 960, 720, 960, 0);
	Check(!position.Inside, "a fractional scale keeps the edge exclusive");

	position = UI_Client_To_Overlay(0, 0, 960, 720, -5, 3);
	Check(!position.Inside && position.X == -5, "a negative client position is outside with its offset kept");
}


void Test_Documents(void)
{
	std::filesystem::path directory(OPENTS_UI_DIR);

	RecordingRenderInterfaceClass render;
	CountingSystemInterfaceClass system;
	Rml::SetRenderInterface(&render);
	Rml::SetSystemInterface(&system);

	Check(Rml::Initialise(), "RmlUi initialises with the recording interfaces");
	std::printf("  RmlUi %s\n", Rml::GetVersion().c_str());

	Check(Rml::LoadFontFace((directory / "OpenSans.ttf").string()), "the shipped font loads");

	Rml::Context * context = Rml::CreateContext("test", Rml::Vector2i(1280, 800));
	Check(context != nullptr, "a context is created");

	int documents = 0;
	for (std::filesystem::directory_entry const & entry : std::filesystem::directory_iterator(directory)) {
		std::filesystem::path path = entry.path();
		std::string extension = path.extension().string();

		if (extension == ".rml" || extension == ".rcss") {
			std::string what = path.filename().string() + " names its resources by bare file name";
			Check(References_Are_Bare(Read_Text(path)), what.c_str());
		}

		if (extension != ".rml" || context == nullptr) {
			continue;
		}

		documents++;
		int rendered = render.Rendered;
		int unsupported = render.Unsupported;
		int problems = system.Problems;
		render.Scissors.clear();

		Rml::ElementDocument * document = context->LoadDocument(path.string());
		std::string name = path.filename().string();
		Check(document != nullptr, (name + " loads").c_str());
		if (document == nullptr) {
			continue;
		}

		document->Show();
		context->Update();
		context->Render();

		Check(render.Rendered > rendered, (name + " draws geometry").c_str());
		Check(render.Unsupported == unsupported, (name + " stays within the implemented render methods").c_str());
		Check(system.Problems == problems, (name + " raises no RmlUi warning or error").c_str());

		bool clipped = true;
		for (Rml::Rectanglei const & scissor : render.Scissors) {
			if (!scissor.Valid() || scissor.Left() < 0 || scissor.Top() < 0 || scissor.Right() > 1280 || scissor.Bottom() > 800) {
				clipped = false;
			}
		}
		Check(clipped, (name + " scissors within the context").c_str());

		document->Close();
		context->Update();
	}

	Check(documents > 0, "the ui directory holds at least one document");

	if (context != nullptr) {
		Rml::RemoveContext("test");
	}
	Rml::Shutdown();

	Check(render.ReleasedGeometry == render.Compiled, "every compiled geometry is released by shutdown");
	Check(render.ReleasedTextures == render.Loaded + render.Generated, "every texture is released by shutdown");
	std::printf("  %d geometries, %d generated textures, %d loaded textures\n", render.Compiled, render.Generated, render.Loaded);
}

}


int main(void)
{
	Test_FreeType();
	Test_ImGui();
	Test_Coordinates();
	Test_Documents();

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
