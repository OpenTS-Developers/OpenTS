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
// not, the documents name their resources the way the shell resolves them and their
// strings by names the generated table knows, and the pointer mapping into the overlay
// behaves at the frame's edges.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <RmlUi/Core.h>
#include <RmlUi/Core/Elements/ElementProgress.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <imgui.h>

#include "ui/uicoord.h"
#include "ui/uimsgbox.h"
#include "ui/uirmlview.h"
#include "ui/uiscreen.h"
#include "ui/uisound.h"
#include "ui/uiversion.h"
#include "ui/uiwaitbox.h"

#include "opents_strings.h"

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


// Records the engine calls the sound presenter makes, in order, so the test can compare
// them with the ones the Win32 dialog made.
class RecordingSoundServiceClass : public UISoundServiceClass
{
	public:
		std::vector<std::string> Calls;

		virtual void Set_Score_Volume(float volume, bool feedback) override { Record("score", volume, feedback); }
		virtual void Set_Sound_Volume(float volume, bool feedback) override { Record("sound", volume, feedback); }
		virtual void Set_Voice_Volume(float volume, bool feedback) override { Record("voice", volume, feedback); }
		virtual void Set_Shuffle(bool on) override { Calls.push_back(on ? "shuffle on" : "shuffle off"); }
		virtual void Set_Repeat(bool on) override { Calls.push_back(on ? "repeat on" : "repeat off"); }
		virtual void Play(int theme) override { Calls.push_back("play " + std::to_string(theme)); }
		virtual void Stop(void) override { Calls.push_back("stop"); }

		std::string Joined(void) const
		{
			std::string all;
			for (std::string const & call : Calls) {
				all += (all.empty() ? "" : "; ") + call;
			}
			return(all);
		}

	private:
		void Record(char const * what, float volume, bool feedback)
		{
			char text[64];
			std::snprintf(text, sizeof(text), "%s %.1f%s", what, volume, feedback ? " feedback" : "");
			Calls.push_back(text);
		}
};


void Drive(UISoundPresenterClass & presenter, char const * name, int value = 0)
{
	UIIntent intent;
	intent.Name = name;
	intent.Value = value;
	presenter.Queue(intent);
	presenter.Drain();
}


// The sound presenter makes the calls the Win32 dialog procedure made, in the same order.
void Test_Sound_Presenter(void)
{
	Check(UISoundPresenterClass::Level_Of(0.7f) == 7 && UISoundPresenterClass::Level_Of(0.04f) == 0 && UISoundPresenterClass::Level_Of(1.0f) == 10, "volumes round to slider levels the way the dialog did");

	RecordingSoundServiceClass service;
	UISoundState state;
	state.Score = 7;
	state.Sound = 5;
	state.Voice = 10;
	state.Enabled = true;
	state.InGame = true;
	state.Tracks.push_back({ "01 - First [3:00]", 5 });
	state.Tracks.push_back({ "02 - Second [2:30]", 6 });
	state.Tracks.push_back({ "03 - Third [4:05]", 9 });
	state.Selected = 1;

	UISoundPresenterClass presenter(service, state);

	Drive(presenter, "score", 4);
	Check(presenter.State.Score == 4 && service.Joined() == "score 0.4 feedback", "a music slider move previews the new volume at once");
	service.Calls.clear();

	Drive(presenter, "voice", 14);
	Check(presenter.State.Voice == 10 && service.Joined() == "voice 1.0 feedback", "a slider level is clamped to the top step");
	service.Calls.clear();

	Drive(presenter, "shuffle", 1);
	Check(presenter.State.Shuffle && !presenter.State.Repeat && service.Joined() == "shuffle on; repeat off", "turning shuffle on turns repeat off");
	service.Calls.clear();

	Drive(presenter, "repeat", 1);
	Check(presenter.State.Repeat && !presenter.State.Shuffle && service.Joined() == "repeat on; shuffle off", "turning repeat on turns shuffle off");
	service.Calls.clear();

	Drive(presenter, "repeat", 0);
	Check(!presenter.State.Repeat && service.Joined() == "repeat off", "turning repeat off leaves shuffle alone");
	service.Calls.clear();

	Drive(presenter, "play");
	Check(service.Joined() == "play 6", "play starts the selected track");
	service.Calls.clear();

	Drive(presenter, "select", 2);
	Drive(presenter, "play");
	Check(presenter.State.Selected == 2 && service.Joined() == "play 9", "play starts a newly selected track");
	service.Calls.clear();

	Drive(presenter, "select", 7);
	Drive(presenter, "play");
	Check(presenter.State.Selected == -1 && service.Calls.empty(), "a row outside the list selects nothing and plays nothing");

	Drive(presenter, "stop");
	Check(service.Joined() == "stop", "stop fades the music out");
	service.Calls.clear();

	Drive(presenter, "ok");
	Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_ACCEPTED && service.Joined() == "score 0.4; sound 0.5; voice 1.0", "OK re-applies the levels without feedback and closes");
}


// The shell resolves [[TXT_NAME]] through the generated table, so a name a document uses
// must exist there.
void Test_Strings(void)
{
	int entries = (int)(sizeof(OpenTSStringNames) / sizeof(OpenTSStringNames[0]));
	Check(OpenTSStringNameCount == entries, "the string table's count matches its entries");
	Check(OpenTSStringNameCount > 700, "the string table carries the language header's identifiers");

	int ok = -1;
	for (OpenTSStringName const & entry : OpenTSStringNames) {
		if (std::strcmp(entry.Name, "TXT_OK") == 0) {
			ok = entry.Id;
		}
	}
	Check(ok == 10, "TXT_OK maps to its identifier");

	std::filesystem::path directory(OPENTS_UI_DIR);
	int references = 0;
	bool resolved = true;

	for (std::filesystem::directory_entry const & entry : std::filesystem::directory_iterator(directory)) {
		std::filesystem::path path = entry.path();
		if (path.extension().string() != ".rml") {
			continue;
		}

		std::string text = Read_Text(path);
		size_t from = 0;
		while (true) {
			size_t open = text.find("[[", from);
			size_t close = (open == std::string::npos) ? std::string::npos : text.find("]]", open + 2);
			if (close == std::string::npos) {
				break;
			}

			std::string name = text.substr(open + 2, close - open - 2);
			bool known = false;
			for (OpenTSStringName const & known_entry : OpenTSStringNames) {
				if (name == known_entry.Name) {
					known = true;
				}
			}
			if (!known) {
				std::printf("  %s names %s, which the table does not know\n", path.filename().string().c_str(), name.c_str());
				resolved = false;
			}

			references++;
			from = close + 2;
		}
	}

	Check(resolved, "every string a document names exists in the table");
	std::printf("  %d string references in the shipped documents\n", references);
}


// The name a document binds with data-model, or nothing.
std::string Data_Model_Name(std::string const & text)
{
	size_t start = text.find("data-model=\"");
	if (start == std::string::npos) {
		return("");
	}
	start += std::strlen("data-model=\"");
	size_t end = text.find('"', start);
	return(end == std::string::npos ? "" : text.substr(start, end - start));
}


class MissingViewClass : public UIRmlViewClass
{
	public:
		explicit MissingViewClass(UIPresenterClass & presenter) :
			UIRmlViewClass(presenter, "missing.rml", "missing")
		{
		}

		virtual void Sync(void) override
		{
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor &) override
		{
			return(true);
		}
};


// Drives the version screen the way the runner and the player do: a click on OK queues an
// intent that the drain turns into a result, Enter and Escape do the same through the
// document, and a screen that cannot be prepared leaves nothing behind.
void Test_Version_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	{
		UIVersionPresenterClass presenter({ "Line 1", "Line 2" });
		std::unique_ptr<UIRmlViewClass> view = UI_Version_View(presenter);

		Check(view->Prepare(context), "the version view prepares against the test context");
		view->Show(true);
		context.Update();
		context.Render();
		Check(system.Problems == problems, "the version screen raises no RmlUi warning or error");
		Check(view->Is_Shown(), "the version screen is shown");

		Rml::ElementDocument * document = view->Document();
		Rml::Element * lines = (document != nullptr) ? document->GetElementById("lines") : nullptr;

		// The data-for template stays in the tree hidden beside the paragraphs it produced.
		int visible = 0;
		for (int index = 0; lines != nullptr && index < lines->GetNumChildren(); index++) {
			if (lines->GetChild(index)->IsVisible()) {
				visible++;
			}
		}
		Check(lines != nullptr && visible == 2, "the version screen lists one paragraph per line");

		Rml::Element * ok = (document != nullptr) ? document->GetElementById("ok") : nullptr;
		Check(ok != nullptr, "the version screen has its OK button");

		if (ok != nullptr) {
			Rml::Vector2f at = ok->GetAbsoluteOffset(Rml::BoxArea::Border) + ok->GetBox().GetSize(Rml::BoxArea::Border) * 0.5f;
			context.ProcessMouseMove((int)at.x, (int)at.y, 0);
			context.Update();
			Check(!context.ProcessMouseButtonDown(0, 0), "a press on OK interacts with the document");
			context.ProcessMouseButtonUp(0, 0);
			Check(!presenter.Result.has_value() && presenter.Has_Pending(), "the click queues an intent and does not act");
			context.Update();
			presenter.Drain();
			Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_ACCEPTED, "draining the click accepts the screen");
		}

		UIIntent late;
		late.Name = "cancel";
		presenter.Queue(late);
		presenter.Drain();
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_ACCEPTED && !presenter.Has_Pending(), "an intent after the result is dropped");

		view->Release();
		context.Update();
	}

	for (int pass = 0; pass < 2; pass++) {
		bool escape = (pass == 0);
		UIVersionPresenterClass presenter({ "Line" });
		std::unique_ptr<UIRmlViewClass> view = UI_Version_View(presenter);

		Check(view->Prepare(context), escape ? "the version view prepares for the Escape pass" : "the version view prepares for the Enter pass");
		view->Show(true);
		context.Update();
		context.ProcessKeyDown(escape ? Rml::Input::KI_ESCAPE : Rml::Input::KI_RETURN, 0);
		context.ProcessKeyUp(escape ? Rml::Input::KI_ESCAPE : Rml::Input::KI_RETURN, 0);
		context.Update();
		presenter.Drain();

		UIResult expected = escape ? UI_RESULT_CANCELLED : UI_RESULT_ACCEPTED;
		Check(presenter.Result.has_value() && *presenter.Result == expected, escape ? "Escape cancels the version screen" : "Enter accepts the version screen");

		view->Release();
		context.Update();
	}

	{
		UIVersionPresenterClass presenter({});
		UIIntent intent;
		intent.Name = "ok";
		presenter.Queue(intent);
		Check(presenter.Has_Pending(), "a queued intent is pending until drained");
		presenter.Discard();
		Check(!presenter.Has_Pending() && !presenter.Result.has_value(), "discarding drops queued intents without a result");
	}

	{
		UIVersionPresenterClass presenter({});
		MissingViewClass view(presenter);
		int before = system.Problems;

		Check(!view.Prepare(context), "a missing document fails preparation");
		Check(system.Problems > before, "a failed preparation is reported");
		Check(!context.GetDataModel("missing"), "a failed preparation leaves no data model behind");
		context.Update();
	}
}


// The visible buttons of a shown message box, left to right.
std::vector<Rml::Element *> Visible_Buttons(Rml::ElementDocument * document)
{
	std::vector<Rml::Element *> buttons;
	if (document == nullptr) {
		return(buttons);
	}

	Rml::ElementList all;
	document->GetElementsByTagName(all, "button");
	for (Rml::Element * element : all) {
		if (element->IsVisible()) {
			buttons.push_back(element);
		}
	}

	std::sort(buttons.begin(), buttons.end(), [](Rml::Element * a, Rml::Element * b) {
		return(a->GetAbsoluteOffset(Rml::BoxArea::Border).x < b->GetAbsoluteOffset(Rml::BoxArea::Border).x);
	});
	return(buttons);
}


void Click(Rml::Context & context, Rml::Element * element)
{
	Rml::Vector2f at = element->GetAbsoluteOffset(Rml::BoxArea::Border) + element->GetBox().GetSize(Rml::BoxArea::Border) * 0.5f;
	context.ProcessMouseMove((int)at.x, (int)at.y, 0);
	context.Update();
	context.ProcessMouseButtonDown(0, 0);
	context.ProcessMouseButtonUp(0, 0);
	context.Update();
}


// Drives the message box the way its callers do: the buttons keep the Win32 template's order
// and slots, a click answers with the button's number, Enter with the default and Escape
// with the second button.
void Test_Message_Box_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	{
		UIMessageBoxPresenterClass presenter("Do you want to abort the mission?", { "First", "Second", "Third" }, 0);
		Check(presenter.Button_Count() == 3, "three captions make three buttons");

		std::unique_ptr<UIRmlViewClass> view = UI_Message_Box_View(presenter);
		Check(view->Prepare(context), "the message box view prepares against the test context");
		view->Show(true);
		context.Update();
		context.Render();
		Check(system.Problems == problems, "the message box raises no RmlUi warning or error");

		std::vector<Rml::Element *> buttons = Visible_Buttons(view->Document());
		Check(buttons.size() == 3, "three buttons are visible");
		bool ordered = buttons.size() == 3 && buttons[0]->GetInnerRML() == "First" && buttons[1]->GetInnerRML() == "Third" && buttons[2]->GetInnerRML() == "Second";
		Check(ordered, "the buttons read first, third, second from left to right");

		if (buttons.size() == 3) {
			Click(context, buttons[1]);
			presenter.Drain();
			Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_ACCEPTED && presenter.Choice == 2, "a click on the middle button answers with the third button");
		}

		view->Release();
		context.Update();
	}

	{
		UIMessageBoxPresenterClass presenter("Two buttons", { "OK", "Cancel", "" }, 0);
		std::unique_ptr<UIRmlViewClass> view = UI_Message_Box_View(presenter);
		Check(view->Prepare(context), "a two-button box prepares");
		view->Show(true);
		context.Update();

		std::vector<Rml::Element *> buttons = Visible_Buttons(view->Document());
		Check(buttons.size() == 2, "two buttons are visible");
		if (buttons.size() == 2) {
			float panel = view->Document()->GetElementById("panel")->GetAbsoluteOffset(Rml::BoxArea::Border).x;
			float left = buttons[0]->GetAbsoluteOffset(Rml::BoxArea::Border).x - panel;
			float right = buttons[1]->GetAbsoluteOffset(Rml::BoxArea::Border).x - panel;
			Check(left < 60.0f && right > 250.0f, "two buttons take the outer slots");
		}

		context.ProcessKeyDown(Rml::Input::KI_ESCAPE, 0);
		context.ProcessKeyUp(Rml::Input::KI_ESCAPE, 0);
		context.Update();
		presenter.Drain();
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_CANCELLED && presenter.Choice == 1, "Escape answers with the second button");

		view->Release();
		context.Update();
	}

	{
		UIMessageBoxPresenterClass presenter("One button", { "OK", "", "" }, 0);
		std::unique_ptr<UIRmlViewClass> view = UI_Message_Box_View(presenter);
		Check(view->Prepare(context), "a one-button box prepares");
		view->Show(true);
		context.Update();

		std::vector<Rml::Element *> buttons = Visible_Buttons(view->Document());
		Check(buttons.size() == 1, "one button is visible");
		if (buttons.size() == 1) {
			float panel = view->Document()->GetElementById("panel")->GetAbsoluteOffset(Rml::BoxArea::Border).x;
			float left = buttons[0]->GetAbsoluteOffset(Rml::BoxArea::Border).x - panel;
			Check(left > 100.0f && left < 200.0f, "a lone button takes the middle slot");
		}

		view->Release();
		context.Update();
	}

	{
		UIMessageBoxPresenterClass presenter("Default", { "Yes", "No", "Maybe" }, 2);
		std::unique_ptr<UIRmlViewClass> view = UI_Message_Box_View(presenter);
		Check(view->Prepare(context), "a box with a default prepares");
		view->Show(true);
		context.Update();
		context.ProcessKeyDown(Rml::Input::KI_RETURN, 0);
		context.ProcessKeyUp(Rml::Input::KI_RETURN, 0);
		context.Update();
		presenter.Drain();
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_ACCEPTED && presenter.Choice == 2, "Enter answers with the default button");

		view->Release();
		context.Update();
	}

	{
		UIMessageBoxPresenterClass presenter("Nothing", { "", "", "" }, 0);
		Check(presenter.Button_Count() == 0, "empty captions make no buttons");
	}
}


// The visible elements of one class in a document, in document order.
std::vector<Rml::Element *> Visible_Of_Class(Rml::ElementDocument * document, char const * name)
{
	std::vector<Rml::Element *> found;
	if (document == nullptr) {
		return(found);
	}

	Rml::ElementList all;
	document->GetElementsByClassName(all, name);
	for (Rml::Element * element : all) {
		if (element->IsVisible()) {
			found.push_back(element);
		}
	}
	return(found);
}


// Drives the sound screen: the sliders, the track rows, the switches and the buttons each
// queue the intent the presenter expects, and the frontend state hides the music half.
void Test_Sound_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	{
		RecordingSoundServiceClass service;
		UISoundState state;
		state.Score = 7;
		state.Sound = 5;
		state.Voice = 10;
		state.Enabled = true;
		state.InGame = true;
		state.Tracks.push_back({ "01 - First [3:00]", 5 });
		state.Tracks.push_back({ "02 - Second [2:30]", 6 });
		state.Tracks.push_back({ "03 - Third [4:05]", 9 });
		state.Selected = 1;

		UISoundPresenterClass presenter(service, state);
		std::unique_ptr<UIRmlViewClass> view = UI_Sound_View(presenter);

		Check(view->Prepare(context), "the sound view prepares against the test context");
		view->Show(true);
		context.Update();
		context.Render();
		Check(system.Problems == problems, "the sound screen raises no RmlUi warning or error");

		Rml::ElementDocument * document = view->Document();
		Rml::ElementList inputs;
		document->GetElementsByTagName(inputs, "input");
		int sliders = 0;
		for (Rml::Element * input : inputs) {
			if (input->GetAttribute<Rml::String>("type", "") == "range") {
				sliders++;
			}
		}
		Check(sliders == 3, "the sound screen has three sliders");

		Rml::Element * score = document->GetElementById("score");
		Check(score != nullptr && score->GetAttribute<int>("value", -1) == 7, "the music slider starts at the music level");

		std::vector<Rml::Element *> rows = Visible_Of_Class(document, "track");
		Check(rows.size() == 3, "the track list shows one row per allowed track");
		Check(rows.size() == 3 && rows[1]->IsClassSet("selected") && !rows[0]->IsClassSet("selected"), "the playing track's row is marked selected");

		if (rows.size() == 3) {
			Click(context, rows[2]);
			presenter.Drain();
			view->Sync();
			context.Update();
			Check(presenter.State.Selected == 2 && rows[2]->IsClassSet("selected") && !rows[1]->IsClassSet("selected"), "a click on a row selects it");
		}

		Rml::Element * play = document->GetElementById("play");
		if (play != nullptr) {
			service.Calls.clear();
			Click(context, play);
			presenter.Drain();
			Check(service.Joined() == "play 9", "the Play button plays the selected track");
		}

		Rml::Element * shuffle = document->GetElementById("shuffle");
		if (shuffle != nullptr) {
			service.Calls.clear();
			Click(context, shuffle);
			presenter.Drain();
			view->Sync();
			context.Update();
			Check(presenter.State.Shuffle && service.Joined() == "shuffle on; repeat off" && shuffle->HasAttribute("checked"), "the shuffle switch turns shuffle on and shows it");
		}

		if (score != nullptr) {
			service.Calls.clear();
			Rml::Dictionary parameters;
			parameters["value"] = Rml::Variant(3.0f);
			score->DispatchEvent(Rml::EventId::Change, parameters);
			presenter.Drain();
			view->Sync();
			context.Update();
			Check(presenter.State.Score == 3 && service.Joined() == "score 0.3 feedback" && score->GetAttribute<int>("value", -1) == 3, "a slider change previews the level and the slider follows the model");
		}

		context.ProcessKeyDown(Rml::Input::KI_ESCAPE, 0);
		context.ProcessKeyUp(Rml::Input::KI_ESCAPE, 0);
		context.Update();
		presenter.Drain();
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_ACCEPTED, "Escape closes the sound screen without reverting anything");

		view->Release();
		context.Update();
	}

	{
		RecordingSoundServiceClass service;
		UISoundState state;
		state.Enabled = true;
		state.InGame = false;

		UISoundPresenterClass presenter(service, state);
		std::unique_ptr<UIRmlViewClass> view = UI_Sound_View(presenter);

		Check(view->Prepare(context), "the frontend sound view prepares");
		view->Show(true);
		context.Update();

		Rml::Element * music = view->Document()->GetElementById("music");
		Check(music != nullptr && !music->IsVisible(), "the frontend sound screen hides the music half");
		Check(Visible_Of_Class(view->Document(), "track").empty(), "the frontend sound screen lists no tracks");

		view->Release();
		context.Update();
	}
}


// Drives the wait box: the text follows the presenter, the frame appears only with a bar, and
// the fill follows the percentage.
void Test_Wait_Box_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	{
		UIWaitBoxPresenterClass presenter("Mission saving - Please Wait...", false);
		std::unique_ptr<UIRmlViewClass> view = UI_Wait_Box_View(presenter);

		Check(view->Prepare(context), "the wait box view prepares against the test context");
		view->Show(false);
		context.Update();
		context.Render();
		Check(system.Problems == problems, "the wait box raises no RmlUi warning or error");

		Rml::ElementDocument * document = view->Document();
		Rml::Element * text = document->GetElementById("text");
		Check(text != nullptr && text->GetInnerRML() == "Mission saving - Please Wait...", "the wait box shows its text");

		Rml::Element * frame = document->GetElementById("frame");
		Check(frame != nullptr && !frame->IsVisible(), "a wait box without a bar hides the frame");

		presenter.Text = "Loading in 3 seconds...";
		view->Sync();
		context.Update();
		Check(text != nullptr && text->GetInnerRML() == "Loading in 3 seconds...", "the wait box text follows the presenter");

		view->Release();
		context.Update();
	}

	{
		UIWaitBoxPresenterClass presenter("Working - Please Wait", true);
		presenter.Set_Fraction(0.5);
		std::unique_ptr<UIRmlViewClass> view = UI_Wait_Box_View(presenter);

		Check(view->Prepare(context), "a wait box with a bar prepares");
		view->Show(false);
		context.Update();

		Rml::ElementDocument * document = view->Document();
		Rml::Element * frame = document->GetElementById("frame");
		Rml::Element * fill = document->GetElementById("fill");
		Check(frame != nullptr && frame->IsVisible(), "a wait box with a bar shows the frame");

		Rml::ElementProgress * progress = rmlui_dynamic_cast<Rml::ElementProgress *>(fill);
		Check(progress != nullptr && std::fabs(progress->GetValue() - 50.0f) < 0.01f, "the fill stands at fifty at fifty percent");

		presenter.Set_Fraction(1.5);
		view->Sync();
		context.Update();
		Check(presenter.Percent == 100 && progress != nullptr && std::fabs(progress->GetValue() - 100.0f) < 0.01f, "the fraction clamps to a full bar");

		view->Release();
		context.Update();
	}
}


void Test_Documents(void)
{
	std::filesystem::path directory(OPENTS_UI_DIR);

	// The shell resolves bare file names through the game's file system; the harness has none,
	// so it runs from the ui directory and RmlUi's own file interface finds the same names.
	std::filesystem::current_path(directory);

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

		// A document over a data model lays out against a permissive stand-in for its screen.
		std::string model = Data_Model_Name(Read_Text(path));
		if (!model.empty()) {
			context->CreateDataModel(model, nullptr, true);
		}

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

		if (!model.empty()) {
			context->RemoveDataModel(model);
		}
	}

	Check(documents > 0, "the ui directory holds at least one document");

	if (context != nullptr) {
		Test_Version_Screen(*context, system);
		Test_Message_Box_Screen(*context, system);
		Test_Sound_Screen(*context, system);
		Test_Wait_Box_Screen(*context, system);
	}

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
	Test_Sound_Presenter();
	Test_Strings();
	Test_Documents();

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
