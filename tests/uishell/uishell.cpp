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
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "ui/rml/rmlkeys.h"
#include "ui/rml/rmlrender.h"
#include "ui/rml/rmlsystem.h"
#include "ui/rml/rmlview.h"
#include "ui/screens/display/uidisplay.h"
#include "ui/screens/gamectrl/uigamectrl.h"
#include "ui/screens/keyboard/uikeyboard.h"
#include "ui/screens/mainopt/uimainopt.h"
#include "ui/screens/msgbox/uimsgbox.h"
#include "ui/screens/sound/uisound.h"
#include "ui/screens/version/uiversion.h"
#include "ui/screens/waitbox/uiwaitbox.h"
#include "ui/uicoord.h"
#include "ui/uihost.h"
#include "ui/uiscreen.h"
#include "ui/uishell.h"
#include "ui/uiunicode.h"
#include "ui/uiview.h"

// windowsx.h, which win.h brings in, names two window walkers the way RmlUi names its
// element walkers.
#undef GetFirstChild
#undef GetNextSibling

#include <RmlUi/Core.h>
#include <RmlUi/Core/Elements/ElementProgress.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <imgui.h>

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
// documents must stay within. It is also the renderer the harness's shell draws with,
// so a test can act from inside a render pass through OnRender.
class RecordingRenderInterfaceClass : public UIRmlRenderClass
{
	public:
		int Compiled = 0;
		int Rendered = 0;
		int ReleasedGeometry = 0;
		int Loaded = 0;
		int Generated = 0;
		int ReleasedTextures = 0;
		int Unsupported = 0;
		int Frames = 0;
		std::vector<Rml::Rectanglei> Scissors;
		std::function<void(void)> OnRender;

		virtual bool Init(void) override
		{
			return(true);
		}

		virtual void Shutdown(void) override
		{
		}

		virtual void Begin_Frame(int, int, int, int) override
		{
			Frames++;
		}

		virtual void Begin_Dev_Frame(int, int, int, int) override
		{
		}

		virtual void Render_ImGui(ImDrawData *) override
		{
		}

		virtual void Destroy_ImGui_Textures(void) override
		{
		}

		virtual int Texture_Limit(void) const override
		{
			return(4096);
		}

		virtual void Log_Resource_Counts(char const *) const override
		{
		}

		virtual Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex>, Rml::Span<const int>) override
		{
			Compiled++;
			return((Rml::CompiledGeometryHandle)Compiled);
		}

		virtual void RenderGeometry(Rml::CompiledGeometryHandle, Rml::Vector2f, Rml::TextureHandle) override
		{
			Rendered++;
			if (OnRender) {
				OnRender();
			}
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


// The program around the shell, as the harness plays it: a frame it can resize, a capture
// it can watch, and a keyboard clear that runs whatever the test wants pumped.
class TestHostClass : public UIShellHostClass
{
	public:
		UIFrameRect Rect = { 0, 0, 1280, 800, 1.0f, 1.0f };
		bool LegacyRequested = false;
		bool LegacyVisible = false;
		bool Captured = false;
		bool Unicode = false;
		unsigned int CodePage = 65001;
		bool Down[256] = {};
		int Presents = 0;
		int Clears = 0;
		int Focuses = 0;
		UIShellClass * Shell = nullptr;
		std::function<void(void)> OnClear;

		virtual bool Key_Down(int virtualkey) const override
		{
			return(Down[virtualkey & 0xFF]);
		}

		virtual bool Window_Is_Unicode(void) const override
		{
			return(Unicode);
		}

		virtual unsigned int Text_Code_Page(void) const override
		{
			return(CodePage);
		}

		int Applied = 0;
		int Restored = 0;
		UICursor LastCursor = UI_CURSOR_ARROW;

		virtual void Apply_Cursor(UICursor cursor) override
		{
			Applied++;
			LastCursor = cursor;
		}

		virtual void Restore_Game_Cursor(void) override
		{
			Restored++;
			LastCursor = UI_CURSOR_ARROW;
		}

		virtual HWND Main_Window(void) const override
		{
			return(nullptr);
		}

		virtual UIFrameRect Frame(void) const override
		{
			return(Rect);
		}

		virtual void Mark_Overlay_Dirty(void) override
		{
		}

		virtual void Present_If_Dirty(void) override
		{
			Presents++;
			if (Shell != nullptr) {
				Shell->Render_Overlay();
			}
		}

		virtual bool Movie_Playing(void) const override
		{
			return(false);
		}

		virtual bool Legacy_Dialog_Visible(void) const override
		{
			return(LegacyVisible);
		}

		virtual bool Legacy_Dialogs_Requested(void) const override
		{
			return(LegacyRequested);
		}

		virtual bool Developer_Keys_Armed(void) const override
		{
			return(false);
		}

		virtual void Clear_Keyboard_Queue(void) override
		{
			Clears++;
			if (OnClear) {
				OnClear();
			}
		}

		virtual void Focus_Main_Window(void) override
		{
			Focuses++;
		}

		virtual bool Take_Capture(void) override
		{
			bool took = !Captured;
			Captured = true;
			return(took);
		}

		virtual void Release_Capture(void) override
		{
			Captured = false;
		}

		virtual bool Screen_To_Client(int &, int &) const override
		{
			return(true);
		}

		virtual char const * String(int) const override
		{
			return("string");
		}

		virtual void Log(char const * text) override
		{
			std::printf("  shell: %s", text);
		}
};


// The shell's own system interface with a count of what RmlUi complained about.
class CountingSystemClass : public UIRmlSystemClass
{
	public:
		int Problems = 0;

		explicit CountingSystemClass(UIShellHostClass & host) :
			UIRmlSystemClass(host)
		{
		}

		virtual bool LogMessage(Rml::Log::Type type, Rml::String const & message) override
		{
			if (type == Rml::Log::LT_ERROR || type == Rml::Log::LT_ASSERT || type == Rml::Log::LT_WARNING) {
				Problems++;
			}
			return(UIRmlSystemClass::LogMessage(type, message));
		}
};


// A shell over the harness's host and recording interfaces, which it owns from construction.
struct ShellFixtureType
{
	TestHostClass Host;
	RecordingRenderInterfaceClass * Render;
	CountingSystemClass * System;
	UIShellClass Shell;

	ShellFixtureType(void) :
		Render(new RecordingRenderInterfaceClass()),
		System(new CountingSystemClass(Host)),
		Shell(Host, std::unique_ptr<UIRmlSystemClass>(System), nullptr, std::unique_ptr<UIRmlRenderClass>(Render))
	{
		Host.Shell = &Shell;
	}
};


bool Send(UIShellClass & shell, UINT message, WPARAM wparam = 0, LPARAM lparam = 0)
{
	return(shell.Handle_Window_Message(nullptr, message, wparam, lparam));
}


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


void Drive(UIPresenterClass & presenter, char const * name, int value = 0)
{
	UIIntent intent;
	intent.Name = name;
	intent.Value = value;
	presenter.Queue(intent);
	presenter.Drain();
}


// Records the engine call the display options presenter makes.
class RecordingDisplayServiceClass : public UIDisplayServiceClass
{
	public:
		std::vector<std::string> Calls;

		virtual void Set_Stretch_Movies(bool on) override { Calls.push_back(on ? "stretch on" : "stretch off"); }
};


// A clock the test moves by hand.
class FakeClockClass : public UIClockClass
{
	public:
		int Now = 0;

		virtual int Milliseconds(void) override { return(Now); }
};


UIDisplayState Display_Fixture(void)
{
	UIDisplayState state;
	state.Modes = { { 640, 400, "640 x 400" }, { 1280, 800, "1280 x 800" }, { 1920, 1080, "1920 x 1080" } };
	state.Selected = 1;
	return(state);
}


// The display presenter applies the movie switch as the player accepts and hands the caller a
// mode to try only when the row changed; the confirmation presenter cancels itself when its
// clock runs out.
void Test_Display_Presenter(void)
{
	{
		RecordingDisplayServiceClass service;
		UIDisplayPresenterClass presenter(service, Display_Fixture());
		Drive(presenter, "select", 2);
		Drive(presenter, "stretch", 1);
		Check(presenter.State.Selected == 2 && presenter.State.StretchMovies && service.Calls.empty() && !presenter.Picked.has_value(), "display edits are held until the player accepts");

		Drive(presenter, "ok");
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_ACCEPTED && service.Calls.size() == 1 && service.Calls[0] == "stretch on", "accepting the display options applies the movie switch");
		Check(presenter.Picked.has_value() && presenter.Picked->Width == 1920 && presenter.Picked->Height == 1080, "accepting with a new row hands the caller that mode to try");
	}

	{
		RecordingDisplayServiceClass service;
		UIDisplayPresenterClass presenter(service, Display_Fixture());
		Drive(presenter, "select", 2);
		Drive(presenter, "select", 1);
		Drive(presenter, "ok");
		Check(presenter.Result.has_value() && !presenter.Picked.has_value() && service.Calls.size() == 1 && service.Calls[0] == "stretch off", "accepting on the starting row applies the switch and tries no mode");
	}

	{
		RecordingDisplayServiceClass service;
		UIDisplayPresenterClass presenter(service, Display_Fixture());
		Drive(presenter, "select", 0);
		Drive(presenter, "stretch", 1);
		Drive(presenter, "cancel");
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_CANCELLED && service.Calls.empty() && !presenter.Picked.has_value(), "cancelling the display options applies nothing");
	}

	{
		RecordingDisplayServiceClass service;
		UIDisplayPresenterClass presenter(service, Display_Fixture());
		Drive(presenter, "select", 7);
		Check(presenter.State.Selected == -1, "a row outside the list selects nothing");
		Drive(presenter, "ok");
		Check(presenter.Result.has_value() && !presenter.Picked.has_value(), "accepting with no row selected tries no mode");
	}

	{
		RecordingDisplayServiceClass service;
		UIDisplayState state = Display_Fixture();
		state.Selected = -1;
		UIDisplayPresenterClass presenter(service, state);
		Drive(presenter, "select", 0);
		Drive(presenter, "ok");
		Check(presenter.Picked.has_value() && presenter.Picked->Width == 640 && presenter.Picked->Height == 400, "a pick with no starting row is a mode to try");
	}

	{
		FakeClockClass clock;
		UIConfirmModePresenterClass presenter(clock);
		Check(presenter.Seconds == 10, "the confirmation starts with the full ten seconds");

		clock.Now = 5000;
		presenter.Refresh();
		Check(presenter.Seconds == 10 && !presenter.Result.has_value(), "the clock starts at the first refresh");

		clock.Now = 5000 + 8100;
		presenter.Refresh();
		Check(presenter.Seconds == 2 && !presenter.Result.has_value(), "the seconds left count down");

		clock.Now = 5000 + 10000;
		presenter.Refresh();
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_CANCELLED && presenter.TimedOut && presenter.Seconds == 0, "silence cancels the confirmation when the timeout passes");
	}

	{
		FakeClockClass clock;
		UIConfirmModePresenterClass presenter(clock);
		presenter.Refresh();
		Drive(presenter, "ok");
		clock.Now = 20000;
		presenter.Refresh();
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_ACCEPTED && !presenter.TimedOut, "OK keeps the mode and a later refresh does not overturn it");
	}

	{
		FakeClockClass clock;
		UIConfirmModePresenterClass presenter(clock);
		presenter.Refresh();
		Drive(presenter, "cancel");
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_CANCELLED && !presenter.TimedOut, "Cancel refuses the mode before the timeout");
	}
}


// Records the engine calls the keyboard presenter makes; keys are named by number.
class RecordingKeyboardServiceClass : public UIKeyboardServiceClass
{
	public:
		std::vector<std::string> Calls;
		bool ConfirmAnswer = true;
		std::vector<UIHotkeyBinding> ResetTable;

		virtual std::string Key_Name(int key) override
		{
			return("K" + std::to_string(key));
		}

		virtual bool Confirm_Reset(void) override
		{
			Calls.push_back("confirm");
			return(ConfirmAnswer);
		}

		virtual void Reset(std::vector<UIHotkeyBinding> & bindings) override
		{
			Calls.push_back("reset");
			bindings = ResetTable;
		}

		// The table is listed by command so the order the presenter keeps it in does not matter.
		virtual void Save(std::vector<UIHotkeyBinding> const & bindings) override
		{
			std::vector<UIHotkeyBinding> sorted = bindings;
			std::sort(sorted.begin(), sorted.end(), [](UIHotkeyBinding const & a, UIHotkeyBinding const & b) {
				return(a.Command < b.Command);
			});
			std::string call = "save";
			for (UIHotkeyBinding const & binding : sorted) {
				call += " " + std::to_string(binding.Command) + "=" + std::to_string(binding.Key);
			}
			Calls.push_back(call);
		}
};


UIKeyboardState Keyboard_Fixture(void)
{
	UIKeyboardState state;
	state.Commands = {
		{ "Selection", "Select View", "Selects the view" },
		{ "Interface", "Toggle Repair", "Toggles repair mode" },
		{ "selection", "Scatter", "Scatters the selection" },
		{ "Interface", "Alliance", "Toggles an alliance" },
	};
	state.Bindings = { { 577, 0 }, { 338, 1 }, { 88, 2 } };
	return(state);
}


std::vector<int> Visible_Commands(UIKeyboardState const & state)
{
	std::vector<int> commands;
	for (UIHotkeyRow const & row : state.Visible) {
		commands.push_back(row.Command);
	}
	return(commands);
}


// The key map turns Windows virtual keys into RmlUi identifiers and back, and a press in a
// document into the KEYBOARD.INI number the game binds.
void Test_Keys(void)
{
	Check(UI_Key_Identifier(0x41) == Rml::Input::KI_A && UI_Key_Identifier(0x39) == Rml::Input::KI_9 && UI_Key_Identifier(0x70) == Rml::Input::KI_F1, "letters, digits and function keys map to their identifiers");
	Check(UI_Key_Identifier(0x1B) == Rml::Input::KI_ESCAPE && UI_Key_Identifier(0x07) == Rml::Input::KI_UNKNOWN, "named keys map and an unassigned code stays unknown");

	bool roundtrip = true;
	for (int code = 0; code < 256; code++) {
		Rml::Input::KeyIdentifier key = UI_Key_Identifier(code);
		if (key != Rml::Input::KI_UNKNOWN && UI_Key_Identifier(UI_Virtual_Key(key)) != key) {
			roundtrip = false;
		}
	}
	Check(roundtrip, "every identifier maps back to a virtual key that maps to it");
	Check(UI_Virtual_Key(Rml::Input::KI_UNKNOWN) == 0, "the unknown identifier has no virtual key");

	Check(UI_Key_Number(Rml::Input::KI_A, false, true, false) == 577, "Control and A make the KEYBOARD.INI number 577");
	Check(UI_Key_Number(Rml::Input::KI_R, true, false, false) == 338 && UI_Key_Number(Rml::Input::KI_X, false, false, false) == 88, "Shift adds 256 and a bare key is its virtual key");
	Check(UI_Key_Number(Rml::Input::KI_F5, false, false, true) == (0x74 | 0x400), "Alt adds 1024");
	Check(UI_Key_Number(Rml::Input::KI_LSHIFT, true, false, false) == 0 && UI_Key_Number(Rml::Input::KI_RCONTROL, false, true, false) == 0 && UI_Key_Number(Rml::Input::KI_LMENU, false, false, true) == 0, "a modifier on its own is no key");
	Check(UI_Key_Number(Rml::Input::KI_UNKNOWN, false, false, false) == 0, "an unknown key is no key");
}


// The keyboard presenter edits a copy of the hotkey table the way the Win32 dialog edited the
// game's: a key moves to the selected command, an empty capture unbinds it, OK saves and Cancel
// drops the edits.
void Test_Keyboard_Presenter(void)
{
	RecordingKeyboardServiceClass service;
	UIKeyboardPresenterClass presenter(service, Keyboard_Fixture());

	Check(presenter.State.Categories == std::vector<std::string>{ "Interface", "Selection" }, "the categories are listed once each, sorted without regard to case");
	Check(presenter.State.Category == 0 && Visible_Commands(presenter.State) == std::vector<int>{ 3, 1 } && presenter.State.Selected == -1, "the first category opens with its commands sorted by name and none selected");
	Check(presenter.State.Visible.size() == 2 && presenter.State.Visible[0].Name == "Alliance", "a row carries its command's name");

	Drive(presenter, "category", 1);
	Check(Visible_Commands(presenter.State) == std::vector<int>{ 2, 0 } && presenter.State.Description.empty(), "another category lists its own commands with the description cleared");

	Drive(presenter, "select", 0);
	Check(presenter.State.Description == "Selects the view" && presenter.State.Shortcut == "K577", "selecting a command shows its description and shortcut");

	Drive(presenter, "capture", 338);
	Check(presenter.State.CapturedName == "K338" && presenter.State.AssignedTo == "Toggle Repair", "a captured key names the command that holds it");

	Drive(presenter, "capture", 999);
	Check(presenter.State.AssignedTo.empty(), "a free key names no command");

	Drive(presenter, "capture", 338);
	Drive(presenter, "assign");
	Check(presenter.Key_Of(0) == 338 && presenter.Key_Of(1) == 0 && presenter.Owner_Of(577) == -1, "assigning moves the key to the selected command and unbinds its previous holder");
	Check(presenter.State.Shortcut == "K338" && presenter.State.Captured == 0 && presenter.State.AssignedTo.empty(), "assigning shows the new shortcut and clears the capture");

	Drive(presenter, "select", 2);
	Drive(presenter, "assign");
	Check(presenter.Key_Of(2) == 0 && presenter.State.Shortcut.empty(), "assigning an empty capture unbinds the command");

	Drive(presenter, "select", -1);
	Drive(presenter, "capture", 65);
	Drive(presenter, "assign");
	Check(presenter.Owner_Of(65) == -1 && presenter.State.Captured == 65, "assigning with no command selected changes nothing");

	Check(service.Calls.empty(), "nothing reaches the game before the player accepts");

	Drive(presenter, "ok");
	Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_ACCEPTED && service.Calls == std::vector<std::string>{ "save 0=338" }, "OK saves the edited table");

	{
		RecordingKeyboardServiceClass quiet;
		UIKeyboardPresenterClass cancelled(quiet, Keyboard_Fixture());
		Drive(cancelled, "select", 3);
		Drive(cancelled, "capture", 70);
		Drive(cancelled, "assign");
		Drive(cancelled, "cancel");
		Check(cancelled.Result.has_value() && *cancelled.Result == UI_RESULT_CANCELLED && quiet.Calls.empty(), "Cancel drops the edits without a call");
	}

	{
		RecordingKeyboardServiceClass declined;
		declined.ConfirmAnswer = false;
		UIKeyboardPresenterClass kept(declined, Keyboard_Fixture());
		Drive(kept, "category", 1);
		Drive(kept, "reset");
		Check(declined.Calls == std::vector<std::string>{ "confirm" } && kept.Key_Of(0) == 577 && kept.State.Category == 1, "a declined reset asks and changes nothing");
	}

	{
		RecordingKeyboardServiceClass confirmed;
		confirmed.ResetTable = { { 65, 3 } };
		UIKeyboardPresenterClass reset(confirmed, Keyboard_Fixture());
		Drive(reset, "category", 1);
		Drive(reset, "select", 0);
		Drive(reset, "reset");
		Check(confirmed.Calls == std::vector<std::string>{ "confirm", "reset" } && reset.Key_Of(3) == 65 && reset.Key_Of(0) == 0, "a confirmed reset reloads the table from the game");
		Check(reset.State.Category == 0 && reset.State.Selected == -1 && reset.State.Description.empty(), "a reset reopens the first category with nothing selected");
	}
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

	Drive(presenter, "score", 7);
	Drive(presenter, "sound", 5);
	Drive(presenter, "voice", 10);
	Check(service.Calls.empty(), "a slider reporting the level it already holds previews nothing");

	Drive(presenter, "score", 4);
	Check(presenter.State.Score == 4 && service.Joined() == "score 0.4 feedback", "a music slider move previews the new volume at once");
	service.Calls.clear();

	Drive(presenter, "voice", 2);
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


// Records the engine calls the game controls presenter makes, in order.
class RecordingGameControlsServiceClass : public UIGameControlsServiceClass
{
	public:
		std::vector<std::string> Calls;

		virtual void Set_Game_Speed(int speed) override { Calls.push_back("speed " + std::to_string(speed)); }
		virtual void Set_Scroll_Rate(int rate) override { Calls.push_back("scroll " + std::to_string(rate)); }
		virtual void Set_Detail_Level(int level) override { Calls.push_back("detail " + std::to_string(level)); }
		virtual void Set_Cameo_Text(bool on) override { Calls.push_back(on ? "cameo on" : "cameo off"); }
		virtual void Set_Action_Lines(bool on) override { Calls.push_back(on ? "lines on" : "lines off"); }
		virtual void Set_Tool_Tips(bool on) override { Calls.push_back(on ? "tooltips on" : "tooltips off"); }
		virtual void Set_Scroll_Coasting(bool on) override { Calls.push_back(on ? "coasting on" : "coasting off"); }
		virtual void Set_Edge_Scroll(bool on) override { Calls.push_back(on ? "edge on" : "edge off"); }
		virtual void Set_Difficulty(int difficulty) override { Calls.push_back("difficulty " + std::to_string(difficulty)); }
		virtual void Save(void) override { Calls.push_back("save"); }

		std::string Joined(void) const
		{
			std::string all;
			for (std::string const & call : Calls) {
				all += (all.empty() ? "" : "; ") + call;
			}
			return(all);
		}
};


void Drive(UIGameControlsPresenterClass & presenter, char const * name, int value = 0)
{
	UIIntent intent;
	intent.Name = name;
	intent.Value = value;
	presenter.Queue(intent);
	presenter.Drain();
}


// The game controls presenter applies the accepted settings in the order the dialog's accept
// path always used, and only when accepted.
void Test_Game_Controls_Presenter(void)
{
	{
		RecordingGameControlsServiceClass service;
		UIGameControlsState state;
		state.Speed = 3;
		state.Scroll = 3;
		state.Detail = 2;
		state.Difficulty = 1;
		state.InGame = false;
		state.HasSpeed = true;
		state.HasDifficulty = true;
		state.SpeedNames = { "Slowest", "Slower", "Slow", "Medium", "Fast", "Faster", "Fastest" };
		state.DetailNames = { "Low", "Medium", "High" };

		UIGameControlsPresenterClass presenter(service, state);
		Check(presenter.State.SpeedName == "Medium" && presenter.State.DetailName == "High" && presenter.State.ScrollName.empty(), "the presenter names the starting slider positions it has names for");

		Drive(presenter, "speed", 5);
		Drive(presenter, "detail", 9);
		Drive(presenter, "cameo", 1);
		Drive(presenter, "edge", 1);
		Drive(presenter, "difficulty", 2);
		Check(presenter.State.Speed == 5 && presenter.State.Detail == 2 && presenter.State.CameoText && presenter.State.EdgeScroll, "edits are held in the state and clamped");
		Check(presenter.State.SpeedName == "Faster" && presenter.State.DetailName == "High", "an edit renames the slider position");
		Check(service.Calls.empty(), "nothing is applied before the player accepts");

		Drive(presenter, "ok");
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_ACCEPTED, "OK accepts the game controls");
		Check(service.Joined() == "speed 5; scroll 3; detail 2; cameo on; lines off; tooltips off; coasting off; edge on; difficulty 2; save", "OK applies the settings in the accept path's order and saves");
	}

	{
		RecordingGameControlsServiceClass service;
		UIGameControlsState state;
		state.InGame = true;
		state.HasSpeed = false;
		state.HasDifficulty = false;

		UIGameControlsPresenterClass presenter(service, state);
		Drive(presenter, "sound");
		Check(!presenter.Result.has_value() && service.Calls.empty(), "the Sound button does nothing without an audio device");

		presenter.State.SoundEnabled = true;
		Drive(presenter, "sound");
		Check(presenter.Result.has_value() && presenter.Next == UIGameControlsPresenterClass::NEXT_SOUND, "the Sound button accepts and names the sound options next");
		Check(service.Joined() == "scroll 0; detail 0; cameo off; lines off; tooltips off; coasting off; edge off; save", "an Internet game applies no game speed and no difficulty");
	}

	{
		RecordingGameControlsServiceClass service;
		UIGameControlsState state;
		UIGameControlsPresenterClass presenter(service, state);
		Drive(presenter, "scroll", 1);
		Drive(presenter, "cancel");
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_CANCELLED && service.Calls.empty(), "Cancel applies nothing");
	}
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


// Every factory builds an RmlUi view, so a test reaches the document through it.
static UIRmlViewClass & Rml(UIViewClass & view)
{
	return(static_cast<UIRmlViewClass &>(view));
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
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);

		Check(Rml(*view).Prepare(context), "the version view prepares against the test context");
		view->Show(true);
		context.Update();
		context.Render();
		Check(system.Problems == problems, "the version screen raises no RmlUi warning or error");
		Check(view->Is_Shown(), "the version screen is shown");

		Rml::ElementDocument * document = Rml(*view).Document();
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
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);

		Check(Rml(*view).Prepare(context), escape ? "the version view prepares for the Escape pass" : "the version view prepares for the Enter pass");
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

		std::unique_ptr<UIViewClass> view = UI_Message_Box_View(presenter);
		Check(Rml(*view).Prepare(context), "the message box view prepares against the test context");
		view->Show(true);
		context.Update();
		context.Render();
		Check(system.Problems == problems, "the message box raises no RmlUi warning or error");

		std::vector<Rml::Element *> buttons = Visible_Buttons(Rml(*view).Document());
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
		std::unique_ptr<UIViewClass> view = UI_Message_Box_View(presenter);
		Check(Rml(*view).Prepare(context), "a two-button box prepares");
		view->Show(true);
		context.Update();

		std::vector<Rml::Element *> buttons = Visible_Buttons(Rml(*view).Document());
		Check(buttons.size() == 2, "two buttons are visible");
		if (buttons.size() == 2) {
			float panel = Rml(*view).Document()->GetElementById("panel")->GetAbsoluteOffset(Rml::BoxArea::Border).x;
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
		std::unique_ptr<UIViewClass> view = UI_Message_Box_View(presenter);
		Check(Rml(*view).Prepare(context), "a one-button box prepares");
		view->Show(true);
		context.Update();

		std::vector<Rml::Element *> buttons = Visible_Buttons(Rml(*view).Document());
		Check(buttons.size() == 1, "one button is visible");
		if (buttons.size() == 1) {
			float panel = Rml(*view).Document()->GetElementById("panel")->GetAbsoluteOffset(Rml::BoxArea::Border).x;
			float left = buttons[0]->GetAbsoluteOffset(Rml::BoxArea::Border).x - panel;
			Check(left > 100.0f && left < 200.0f, "a lone button takes the middle slot");
		}

		view->Release();
		context.Update();
	}

	{
		UIMessageBoxPresenterClass presenter("Default", { "Yes", "No", "Maybe" }, 2);
		std::unique_ptr<UIViewClass> view = UI_Message_Box_View(presenter);
		Check(Rml(*view).Prepare(context), "a box with a default prepares");
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
		std::unique_ptr<UIViewClass> view = UI_Sound_View(presenter);

		Check(Rml(*view).Prepare(context), "the sound view prepares against the test context");
		view->Show(true);
		context.Update();
		context.Render();
		Check(system.Problems == problems, "the sound screen raises no RmlUi warning or error");

		// Seeding a range input dispatches its change event, so opening queues the starting levels.
		presenter.Drain();
		Check(presenter.State.Score == 7 && presenter.State.Sound == 5 && presenter.State.Voice == 10 && service.Calls.empty(), "opening the sound screen plays no feedback");

		Rml::ElementDocument * document = Rml(*view).Document();
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
		std::unique_ptr<UIViewClass> view = UI_Sound_View(presenter);

		Check(Rml(*view).Prepare(context), "the frontend sound view prepares");
		view->Show(true);
		context.Update();

		Rml::Element * music = Rml(*view).Document()->GetElementById("music");
		Check(music != nullptr && !music->IsVisible(), "the frontend sound screen hides the music half");
		Check(Visible_Of_Class(Rml(*view).Document(), "track").empty(), "the frontend sound screen lists no tracks");

		view->Release();
		context.Update();
	}
}


// The range inputs a document shows, counting only those inside a visible row.
int Visible_Sliders(Rml::ElementDocument * document)
{
	int sliders = 0;
	Rml::ElementList inputs;
	document->GetElementsByTagName(inputs, "input");
	for (Rml::Element * input : inputs) {
		if (input->GetAttribute<Rml::String>("type", "") == "range" && input->IsVisible(true)) {
			sliders++;
		}
	}
	return(sliders);
}


UIGameControlsState Game_Controls_Fixture(void)
{
	UIGameControlsState state;
	state.Speed = 4;
	state.Scroll = 2;
	state.Detail = 1;
	state.Difficulty = 2;
	state.CameoText = true;
	state.ToolTips = true;
	state.SpeedNames = { "Slowest", "Slower", "Slow", "Medium", "Fast", "Faster", "Fastest" };
	state.ScrollNames = state.SpeedNames;
	state.DetailNames = { "Low", "Medium", "High" };
	state.DifficultyNames = { "Easy", "Normal", "Hard" };
	return(state);
}


// Drives the game controls screen in its three variants: the sliders hold edits and name
// their positions, the switches toggle, Sound applies and names the next screen, and the
// frontend and Internet variants hide the controls their Win32 templates lack.
void Test_Game_Controls_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	{
		RecordingGameControlsServiceClass service;
		UIGameControlsState state = Game_Controls_Fixture();
		state.InGame = true;
		state.HasSpeed = true;
		state.HasDifficulty = false;
		state.SoundEnabled = true;

		UIGameControlsPresenterClass presenter(service, state);
		std::unique_ptr<UIViewClass> view = UI_Game_Controls_View(presenter);

		Check(Rml(*view).Prepare(context), "the game controls view prepares against the test context");
		view->Show(true);
		context.Update();
		context.Render();
		Check(system.Problems == problems, "the game controls screen raises no RmlUi warning or error");

		// Seeding a range input dispatches its change event, so opening queues the starting values.
		presenter.Drain();
		Check(presenter.State.Speed == 4 && presenter.State.Scroll == 2 && presenter.State.Detail == 1 && service.Calls.empty(), "opening the game controls holds the starting settings and applies nothing");

		Rml::ElementDocument * document = Rml(*view).Document();
		Check(Visible_Sliders(document) == 3, "the in-game screen has three sliders");

		Rml::Element * speed = document->GetElementById("speed");
		Check(speed != nullptr && speed->GetAttribute<int>("value", -1) == 2, "the speed slider runs from slowest to fastest, so it starts at six minus the speed");

		Rml::Element * speed_name = document->GetElementById("speed-name");
		Check(speed_name != nullptr && speed_name->GetInnerRML() == "Fast", "the speed's name shows beside its slider");

		Rml::Element * detail_name = document->GetElementById("detail-name");
		Check(detail_name != nullptr && detail_name->GetInnerRML() == "Medium", "the detail level's name shows beside its slider");

		Rml::Element * sound = document->GetElementById("sound");
		Rml::Element * keyboard = document->GetElementById("keyboard");
		Rml::Element * options = document->GetElementById("ok-options");
		Check(sound != nullptr && sound->IsVisible() && keyboard != nullptr && keyboard->IsVisible(), "the in-game screen has its Sound and Keyboard buttons");
		Check(options != nullptr && options->IsVisible(true), "the in-game accept button reads Options Menu");

		if (speed != nullptr && speed_name != nullptr) {
			Rml::Dictionary parameters;
			parameters["value"] = Rml::Variant(5.0f);
			speed->DispatchEvent(Rml::EventId::Change, parameters);
			presenter.Drain();
			view->Sync();
			context.Update();
			Check(presenter.State.Speed == 1 && speed_name->GetInnerRML() == "Slower" && service.Calls.empty(), "dragging the speed slider changes the held speed and its name and applies nothing");
		}

		Rml::Element * cameo = document->GetElementById("cameo");
		if (cameo != nullptr) {
			Click(context, cameo);
			presenter.Drain();
			view->Sync();
			context.Update();
			Check(!presenter.State.CameoText && !cameo->HasAttribute("checked"), "a click on the cameo text switch turns it off and shows it");
		}

		if (sound != nullptr) {
			Click(context, sound);
			presenter.Drain();
			Check(presenter.Result.has_value() && presenter.Next == UIGameControlsPresenterClass::NEXT_SOUND, "the Sound button accepts the screen and names the sound options next");
			Check(service.Joined() == "speed 1; scroll 2; detail 1; cameo off; lines off; tooltips on; coasting off; edge off; save", "the Sound button applies the edited settings in order and saves");
		}

		view->Release();
		context.Update();
	}

	{
		RecordingGameControlsServiceClass service;
		UIGameControlsState state = Game_Controls_Fixture();
		state.InGame = false;
		state.HasSpeed = true;
		state.HasDifficulty = true;

		UIGameControlsPresenterClass presenter(service, state);
		std::unique_ptr<UIViewClass> view = UI_Game_Controls_View(presenter);

		Check(Rml(*view).Prepare(context), "the frontend game controls view prepares");
		view->Show(true);
		context.Update();

		Rml::ElementDocument * document = Rml(*view).Document();
		Check(Visible_Sliders(document) == 4, "the frontend screen adds the difficulty slider");

		Rml::Element * difficulty_name = document->GetElementById("difficulty-name");
		Check(difficulty_name != nullptr && difficulty_name->GetInnerRML() == "Hard", "the difficulty's name shows beside its slider");

		Rml::Element * sound = document->GetElementById("sound");
		Rml::Element * keyboard = document->GetElementById("keyboard");
		Rml::Element * main = document->GetElementById("ok-main");
		Check(sound != nullptr && !sound->IsVisible() && keyboard != nullptr && !keyboard->IsVisible(), "the frontend screen has no Sound or Keyboard button");
		Check(main != nullptr && main->IsVisible(true), "the frontend accept button reads Main Menu");

		Rml::Element * edge = document->GetElementById("edge");
		if (edge != nullptr) {
			Click(context, edge);
			presenter.Drain();
			view->Sync();
			context.Update();
			Check(presenter.State.EdgeScroll && edge->HasAttribute("checked"), "a click on the edge scrolling switch turns it on and shows it");
		}

		context.ProcessKeyDown(Rml::Input::KI_ESCAPE, 0);
		context.ProcessKeyUp(Rml::Input::KI_ESCAPE, 0);
		context.Update();
		presenter.Drain();
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_CANCELLED && service.Calls.empty(), "Escape leaves the game controls with nothing applied");

		view->Release();
		context.Update();
	}

	{
		RecordingGameControlsServiceClass service;
		UIGameControlsState state = Game_Controls_Fixture();
		state.InGame = true;
		state.HasSpeed = false;
		state.HasDifficulty = false;
		state.SoundEnabled = false;

		UIGameControlsPresenterClass presenter(service, state);
		std::unique_ptr<UIViewClass> view = UI_Game_Controls_View(presenter);

		Check(Rml(*view).Prepare(context), "the Internet game controls view prepares");
		view->Show(true);
		context.Update();

		Rml::ElementDocument * document = Rml(*view).Document();
		Check(Visible_Sliders(document) == 2, "the Internet screen has no game speed slider");

		Rml::Element * sound = document->GetElementById("sound");
		Check(sound != nullptr && sound->IsClassSet("disabled"), "the Sound button shows disabled without an audio device");

		if (sound != nullptr) {
			Click(context, sound);
			presenter.Drain();
			Check(!presenter.Result.has_value() && service.Calls.empty(), "a disabled Sound button does nothing");
		}

		context.ProcessKeyDown(Rml::Input::KI_RETURN, 0);
		context.ProcessKeyUp(Rml::Input::KI_RETURN, 0);
		context.Update();
		presenter.Drain();
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_ACCEPTED && service.Joined() == "scroll 2; detail 1; cameo on; lines off; tooltips on; coasting off; edge off; save", "Enter accepts the Internet screen without a game speed or a difficulty");

		view->Release();
		context.Update();
	}
}


// Drives the display options and the mode confirmation: the resolution rows select, the
// switch toggles, OK hands the caller a mode, and the confirmation counts down and cancels
// itself when its clock runs out.
void Test_Display_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	{
		RecordingDisplayServiceClass service;
		UIDisplayPresenterClass presenter(service, Display_Fixture());
		std::unique_ptr<UIViewClass> view = UI_Display_View(presenter);

		Check(Rml(*view).Prepare(context), "the display view prepares against the test context");
		view->Show(true);
		context.Update();
		context.Render();
		Check(system.Problems == problems, "the display screen raises no RmlUi warning or error");

		Rml::ElementDocument * document = Rml(*view).Document();
		std::vector<Rml::Element *> rows = Visible_Of_Class(document, "mode");
		Check(rows.size() == 3, "the display screen lists one row per mode");
		Check(rows.size() == 3 && rows[1]->IsClassSet("selected") && rows[1]->GetInnerRML() == "1280 x 800", "the row of the stored mode starts selected");

		if (rows.size() == 3) {
			Click(context, rows[2]);
			presenter.Drain();
			view->Sync();
			context.Update();
			Check(presenter.State.Selected == 2 && rows[2]->IsClassSet("selected") && !rows[1]->IsClassSet("selected"), "a click on a row selects it");
		}

		Rml::Element * stretch = document->GetElementById("stretch");
		if (stretch != nullptr) {
			Click(context, stretch);
			presenter.Drain();
			view->Sync();
			context.Update();
			Check(presenter.State.StretchMovies && stretch->HasAttribute("checked") && service.Calls.empty(), "the movie switch turns on and shows it without applying");
		}

		Rml::Element * ok = document->GetElementById("ok");
		Rml::Element * cancel = document->GetElementById("cancel");
		Check(ok != nullptr && cancel != nullptr && ok->GetAbsoluteOffset(Rml::BoxArea::Border).x < cancel->GetAbsoluteOffset(Rml::BoxArea::Border).x, "OK sits left of Cancel");

		if (ok != nullptr) {
			Click(context, ok);
			presenter.Drain();
			Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_ACCEPTED && service.Calls.size() == 1 && service.Calls[0] == "stretch on", "OK applies the movie switch");
			Check(presenter.Picked.has_value() && presenter.Picked->Width == 1920 && presenter.Picked->Height == 1080, "OK hands the caller the picked mode");
		}

		view->Release();
		context.Update();
	}

	{
		RecordingDisplayServiceClass service;
		UIDisplayPresenterClass presenter(service, Display_Fixture());
		std::unique_ptr<UIViewClass> view = UI_Display_View(presenter);

		Check(Rml(*view).Prepare(context), "a second display view prepares");
		view->Show(true);
		context.Update();

		std::vector<Rml::Element *> rows = Visible_Of_Class(Rml(*view).Document(), "mode");
		if (rows.size() == 3) {
			Click(context, rows[0]);
			presenter.Drain();
		}

		context.ProcessKeyDown(Rml::Input::KI_ESCAPE, 0);
		context.ProcessKeyUp(Rml::Input::KI_ESCAPE, 0);
		context.Update();
		presenter.Drain();
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_CANCELLED && service.Calls.empty() && !presenter.Picked.has_value(), "Escape leaves the display options with nothing applied");

		view->Release();
		context.Update();
	}

	{
		FakeClockClass clock;
		UIConfirmModePresenterClass presenter(clock);
		std::unique_ptr<UIViewClass> view = UI_Confirm_Mode_View(presenter);

		Check(Rml(*view).Prepare(context), "the confirmation view prepares against the test context");
		presenter.Refresh();
		view->Show(true);
		view->Sync();
		context.Update();
		context.Render();
		Check(system.Problems == problems, "the confirmation screen raises no RmlUi warning or error");

		Rml::ElementDocument * document = Rml(*view).Document();
		Rml::Element * seconds = document->GetElementById("seconds");
		Check(seconds != nullptr && seconds->GetInnerRML() == "10", "the confirmation shows the ten seconds left");

		clock.Now = 7500;
		presenter.Refresh();
		view->Sync();
		context.Update();
		Check(seconds != nullptr && seconds->GetInnerRML() == "3", "the seconds shown follow the clock");

		Check(Visible_Buttons(document).size() == 2, "the confirmation has OK and Cancel");

		Rml::Element * ok = document->GetElementById("ok");
		if (ok != nullptr) {
			Click(context, ok);
			presenter.Drain();
			Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_ACCEPTED && !presenter.TimedOut, "OK keeps the mode");
		}

		view->Release();
		context.Update();
	}

	{
		FakeClockClass clock;
		UIConfirmModePresenterClass presenter(clock);
		std::unique_ptr<UIViewClass> view = UI_Confirm_Mode_View(presenter);

		Check(Rml(*view).Prepare(context), "a second confirmation view prepares");
		presenter.Refresh();
		view->Show(true);
		view->Sync();
		context.Update();

		clock.Now = 10000;
		presenter.Refresh();
		presenter.Drain();
		view->Sync();
		context.Update();

		Rml::Element * seconds = Rml(*view).Document()->GetElementById("seconds");
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_CANCELLED && presenter.TimedOut, "the confirmation cancels itself when the clock runs out");
		Check(seconds != nullptr && seconds->GetInnerRML() == "0", "the countdown ends at zero");

		view->Release();
		context.Update();
	}
}


// Drives the keyboard screen: the category and command rows select, a key pressed in the
// focused capture element becomes the captured number, Assign moves it, and the dialog keys
// keep their meaning inside the capture.
void Test_Keyboard_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	{
		RecordingKeyboardServiceClass service;
		UIKeyboardPresenterClass presenter(service, Keyboard_Fixture());
		std::unique_ptr<UIViewClass> view = UI_Keyboard_View(presenter);

		Check(Rml(*view).Prepare(context), "the keyboard view prepares against the test context");
		view->Show(true);
		view->Sync();
		context.Update();
		context.Render();
		Check(system.Problems == problems, "the keyboard screen raises no RmlUi warning or error");

		Rml::ElementDocument * document = Rml(*view).Document();
		std::vector<Rml::Element *> rows = Visible_Of_Class(document, "row");
		Check(rows.size() == 4, "the keyboard screen lists the categories and the open category's commands");
		Check(rows.size() == 4 && rows[0]->GetInnerRML() == "Interface" && rows[0]->IsClassSet("selected") && rows[2]->GetInnerRML() == "Alliance" && rows[3]->GetInnerRML() == "Toggle Repair", "the first category is open with its commands sorted by name");

		if (rows.size() == 4) {
			Click(context, rows[1]);
			presenter.Drain();
			view->Sync();
			context.Update();
			rows = Visible_Of_Class(document, "row");
			Check(rows.size() == 4 && rows[1]->IsClassSet("selected") && rows[2]->GetInnerRML() == "Scatter" && rows[3]->GetInnerRML() == "Select View", "a click on a category lists its commands");
		}

		Rml::Element * capture = document->GetElementById("capture");
		Rml::Element * description = document->GetElementById("description");
		Rml::Element * shortcut = document->GetElementById("shortcut");
		Check(capture != nullptr && description != nullptr && shortcut != nullptr, "the keyboard screen has its capture element, description and shortcut");

		if (rows.size() == 4) {
			Click(context, rows[3]);
			presenter.Drain();
			view->Sync();
			context.Update();
			Check(presenter.State.Selected == 0 && description->GetInnerRML() == "Selects the view" && shortcut->GetInnerRML() == "K577", "a click on a command shows its description and shortcut");
			Check(context.GetFocusElement() == capture, "selecting a command focuses the capture element");
		}

		if (capture != nullptr) {
			capture->Focus();
			context.ProcessKeyDown(Rml::Input::KI_R, Rml::Input::KM_SHIFT);
			context.ProcessKeyUp(Rml::Input::KI_R, Rml::Input::KM_SHIFT);
			context.Update();
			presenter.Drain();
			view->Sync();
			context.Update();
			Check(presenter.State.Captured == 338 && presenter.State.AssignedTo == "Toggle Repair", "Shift and R in the capture element become the number 338 and name its holder");
			Check(capture->GetInnerRML().find("K338") != std::string::npos, "the capture element shows the captured key");

			context.ProcessKeyDown(Rml::Input::KI_LSHIFT, Rml::Input::KM_SHIFT);
			context.ProcessKeyUp(Rml::Input::KI_LSHIFT, 0);
			context.Update();
			presenter.Drain();
			Check(presenter.State.Captured == 338, "a modifier on its own leaves the capture as it was");
		}

		Rml::Element * assign = document->GetElementById("assign");
		if (assign != nullptr) {
			Click(context, assign);
			presenter.Drain();
			view->Sync();
			context.Update();
			Check(presenter.Key_Of(0) == 338 && presenter.Key_Of(1) == 0 && shortcut->GetInnerRML() == "K338" && presenter.State.Captured == 0, "Assign moves the key to the selected command and shows the new shortcut");
		}

		if (capture != nullptr) {
			capture->Focus();
			context.ProcessKeyDown(Rml::Input::KI_RETURN, 0);
			context.ProcessKeyUp(Rml::Input::KI_RETURN, 0);
			context.Update();
			presenter.Drain();
			Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_ACCEPTED && service.Calls == std::vector<std::string>{ "save 0=338 2=88" }, "Enter in the capture element accepts the screen and saves");
		}

		view->Release();
		context.Update();
	}

	{
		RecordingKeyboardServiceClass service;
		service.ConfirmAnswer = false;
		UIKeyboardPresenterClass presenter(service, Keyboard_Fixture());
		std::unique_ptr<UIViewClass> view = UI_Keyboard_View(presenter);

		Check(Rml(*view).Prepare(context), "a second keyboard view prepares");
		view->Show(true);
		view->Sync();
		context.Update();

		Rml::Element * reset = Rml(*view).Document()->GetElementById("reset");
		if (reset != nullptr) {
			Click(context, reset);
			presenter.Drain();
			Check(service.Calls == std::vector<std::string>{ "confirm" } && presenter.Key_Of(0) == 577, "Reset All asks first and a refusal changes nothing");
		}

		Rml::Element * capture = Rml(*view).Document()->GetElementById("capture");
		if (capture != nullptr) {
			capture->Focus();
		}
		context.ProcessKeyDown(Rml::Input::KI_ESCAPE, 0);
		context.ProcessKeyUp(Rml::Input::KI_ESCAPE, 0);
		context.Update();
		presenter.Drain();
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_CANCELLED && service.Calls.size() == 1, "Escape in the capture element cancels the screen without saving");

		view->Release();
		context.Update();
	}
}


// The buttons of a document from top to bottom.
std::vector<Rml::Element *> Buttons_Top_Down(Rml::ElementDocument * document)
{
	std::vector<Rml::Element *> buttons = Visible_Buttons(document);
	std::sort(buttons.begin(), buttons.end(), [](Rml::Element * a, Rml::Element * b) {
		return(a->GetAbsoluteOffset(Rml::BoxArea::Border).y < b->GetAbsoluteOffset(Rml::BoxArea::Border).y);
	});
	return(buttons);
}


// Drives the options menu: each button closes it with its choice, a dead Sound button does
// nothing, Escape leaves, and the panel takes the top edge the game hands it.
void Test_Main_Options_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	{
		UIMainOptionsState state;
		state.SoundEnabled = true;
		UIMainOptionsPresenterClass presenter(state);
		std::unique_ptr<UIViewClass> view = UI_Main_Options_View(presenter);

		Check(Rml(*view).Prepare(context), "the options menu view prepares against the test context");
		view->Show(true);
		context.Update();
		context.Render();
		Check(system.Problems == problems, "the options menu raises no RmlUi warning or error");

		std::vector<Rml::Element *> buttons = Buttons_Top_Down(Rml(*view).Document());
		Check(buttons.size() == 5, "the options menu has five buttons");
		bool ordered = buttons.size() == 5 && buttons[0]->GetId() == "settings" && buttons[1]->GetId() == "display" && buttons[2]->GetId() == "sound" && buttons[3]->GetId() == "keyboard" && buttons[4]->GetId() == "mainmenu";
		Check(ordered, "the buttons run Game Settings, Display, Sound, Keyboard, Main Menu from the top");

		Rml::Element * panel = Rml(*view).Document()->GetElementById("panel");
		float centre = (float)context.GetDimensions().y * 0.5f;
		Check(panel != nullptr && panel->GetAbsoluteOffset(Rml::BoxArea::Border).y < centre && panel->GetAbsoluteOffset(Rml::BoxArea::Border).y + panel->GetBox().GetSize(Rml::BoxArea::Border).y > centre, "without a top edge the menu sits in the middle");

		if (buttons.size() == 5) {
			Click(context, buttons[1]);
			presenter.Drain();
			Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_ACCEPTED && presenter.Choice == UI_MAIN_OPTIONS_DISPLAY, "the Display button closes the menu with the display choice");
		}

		view->Release();
		context.Update();
	}

	{
		UIMainOptionsState state;
		state.SoundEnabled = false;
		state.Top = 200;
		UIMainOptionsPresenterClass presenter(state);
		std::unique_ptr<UIViewClass> view = UI_Main_Options_View(presenter);

		Check(Rml(*view).Prepare(context), "a second options menu view prepares");
		view->Show(true);
		context.Update();

		Rml::Element * panel = Rml(*view).Document()->GetElementById("panel");
		Check(panel != nullptr && std::fabs(panel->GetAbsoluteOffset(Rml::BoxArea::Border).y - 200.0f) < 1.0f, "the menu sits at the top edge the game hands it");

		Rml::Element * sound = Rml(*view).Document()->GetElementById("sound");
		Check(sound != nullptr && sound->IsClassSet("disabled"), "the Sound button shows disabled without an audio device");
		if (sound != nullptr) {
			Click(context, sound);
			presenter.Drain();
			Check(!presenter.Result.has_value(), "a disabled Sound button does nothing");
		}

		context.ProcessKeyDown(Rml::Input::KI_ESCAPE, 0);
		context.ProcessKeyUp(Rml::Input::KI_ESCAPE, 0);
		context.Update();
		presenter.Drain();
		Check(presenter.Result.has_value() && *presenter.Result == UI_RESULT_CANCELLED && presenter.Choice == UI_MAIN_OPTIONS_LEAVE, "Escape leaves the options menu");

		view->Release();
		context.Update();
	}

	{
		UIMainOptionsState state;
		state.SoundEnabled = true;
		UIMainOptionsPresenterClass presenter(state);
		Drive(presenter, "sound");
		Check(presenter.Result.has_value() && presenter.Choice == UI_MAIN_OPTIONS_SOUND, "a live Sound button picks the sound options");

		UIMainOptionsPresenterClass keyboard(state);
		Drive(keyboard, "keyboard");
		UIMainOptionsPresenterClass settings(state);
		Drive(settings, "settings");
		UIMainOptionsPresenterClass leave(state);
		Drive(leave, "ok");
		Check(keyboard.Choice == UI_MAIN_OPTIONS_KEYBOARD && settings.Choice == UI_MAIN_OPTIONS_SETTINGS && leave.Choice == UI_MAIN_OPTIONS_LEAVE && leave.Result.has_value() && *leave.Result == UI_RESULT_ACCEPTED, "Keyboard, Game Settings and Main Menu each answer with their choice");
	}
}


// Drives the wait box: the text follows the presenter, the frame appears only with a bar, and
// the fill follows the percentage.
void Test_Wait_Box_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	{
		UIWaitBoxPresenterClass presenter("Mission saving - Please Wait...", false);
		std::unique_ptr<UIViewClass> view = UI_Wait_Box_View(presenter);

		Check(Rml(*view).Prepare(context), "the wait box view prepares against the test context");
		view->Show(false);
		context.Update();
		context.Render();
		Check(system.Problems == problems, "the wait box raises no RmlUi warning or error");

		Rml::ElementDocument * document = Rml(*view).Document();
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
		std::unique_ptr<UIViewClass> view = UI_Wait_Box_View(presenter);

		Check(Rml(*view).Prepare(context), "a wait box with a bar prepares");
		view->Show(false);
		context.Update();

		Rml::ElementDocument * document = Rml(*view).Document();
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

		// A document over a data model lays out against a permissive stand-in for its screen,
		// which takes the intents a control queues as it is seeded.
		std::string model = Data_Model_Name(Read_Text(path));
		if (!model.empty()) {
			Rml::DataModelConstructor constructor = context->CreateDataModel(model, nullptr, true);
			constructor.BindEventCallback("queue", [](Rml::DataModelHandle, Rml::Event &, Rml::VariantList const &) {});
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
		Test_Game_Controls_Screen(*context, system);
		Test_Display_Screen(*context, system);
		Test_Keyboard_Screen(*context, system);
		Test_Main_Options_Screen(*context, system);
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


// The shell over the harness's host: it runs after Test_Documents has shut RmlUi down, as
// one RmlUi lives per process, and the working directory is still the ui directory.
void Test_Shell(void)
{
	ShellFixtureType fixture;
	UIShellClass & shell = fixture.Shell;
	TestHostClass & host = fixture.Host;

	Check(!shell.Use_Rml(), "a shell not yet initialised opens no document");
	Check(shell.Init(), "the shell initialises over the injected interfaces");
	Check(shell.Rml_Context() != nullptr, "the shell holds a context");
	Check(shell.Use_Rml(), "documents are used while the host asks for no legacy dialogs");
	host.LegacyRequested = true;
	Check(!shell.Use_Rml(), "the LegacyDialogs setting turns the documents off");
	host.LegacyRequested = false;
	Check(!shell.Screen_Shown() && shell.Modal() == nullptr && shell.Modal_Depth() == 0, "no screen is shown at start");

	{
		UIVersionPresenterClass presenter({ "one", "two" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
		int passes = 0;
		bool shownInside = false;
		bool consumedWhileOpening = false;
		int presents = host.Presents;

		host.OnClear = [&](void) {
			// The engine's clear pumps the window messages; a press arriving then meets the
			// screen that has just been shown.
			if (host.Clears == 1) {
				consumedWhileOpening = Send(shell, WM_LBUTTONDOWN, 0, MAKELPARAM(10, 10));
			}
		};

		UIResult result = shell.Run_Modal(*view, [&](void) {
			passes++;
			if (passes == 1) {
				shownInside = shell.Screen_Shown() && shell.Modal() == view.get() && shell.Modal_Depth() == 1;
			}
			if (passes == 3) {
				Send(shell, WM_KEYDOWN, VK_RETURN);
			}
			return(false);
		});
		host.OnClear = nullptr;

		Check(result == UI_RESULT_ACCEPTED, "Enter accepts the modal");
		Check(passes == 3, "the runner stops on the pass that produced the result");
		Check(host.Presents - presents == 2, "the runner presents after each pass that continues");
		Check(shownInside, "the modal is the shown screen while the service runs");
		Check(consumedWhileOpening, "a press pumped while the screen opens is consumed");
		Check(host.Clears == 2, "the keyboard queue is cleared at open and at close");
		Check(host.Focuses == 1, "focus returns to the main window once");
		Check(!shell.Screen_Shown() && shell.Modal() == nullptr, "the modal stack is empty after the close");
	}

	{
		UIVersionPresenterClass presenter({ "escape" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
		UIResult result = shell.Run_Modal(*view, [&](void) {
			Send(shell, WM_KEYDOWN, VK_ESCAPE);
			return(false);
		});
		Check(result == UI_RESULT_CANCELLED, "Escape cancels the modal");
	}

	{
		UIVersionPresenterClass presenter({ "ended" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
		int passes = 0;
		UIResult result = shell.Run_Modal(*view, [&](void) {
			passes++;
			return(true);
		});
		Check(result == UI_RESULT_SESSION_ENDED && passes == 1, "a service reporting the game ended closes the modal at once");
		Check(!shell.Screen_Shown(), "a modal ended by the game leaves nothing shown");
	}

	{
		UIVersionPresenterClass presenter({ "resize" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
		int passes = 0;
		bool resized = false;
		bool unchangedInside = false;
		bool appliedBefore = false;

		fixture.Render->OnRender = [&](void) {
			if (!resized) {
				resized = true;
				host.Rect.Width = 640;
				host.Rect.Height = 400;
				shell.On_Video_Change();
				unchangedInside = shell.Rml_Context()->GetDimensions() == Rml::Vector2i(1280, 800);
			}
		};

		shell.Run_Modal(*view, [&](void) {
			passes++;
			if (passes == 2) {
				appliedBefore = shell.Rml_Context()->GetDimensions() == Rml::Vector2i(640, 400);
				Send(shell, WM_KEYDOWN, VK_ESCAPE);
			}
			return(false);
		});
		fixture.Render->OnRender = nullptr;

		Check(resized && unchangedInside, "a resize arriving inside a render is deferred");
		Check(appliedBefore, "the deferred resize is applied before the next tick");

		host.Rect = { 0, 0, 1280, 800, 1.0f, 1.0f };
		shell.On_Video_Change();
		Check(shell.Rml_Context()->GetDimensions() == Rml::Vector2i(1280, 800), "a resize outside a render is applied at once");
	}

	{
		UIVersionPresenterClass presenter({ "held" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
		int passes = 0;
		bool suppressed = false;
		bool swallowed = false;
		bool quiet = false;

		host.Down[VK_LBUTTON] = true;
		host.Down['A'] = true;
		UIResult result = shell.Run_Modal(*view, [&](void) {
			passes++;
			if (passes == 1) {
				suppressed = shell.Input_State().Mouse_Owner(0) == UI_INPUT_SUPPRESSED && shell.Input_State().Key_Owner('A') == UI_INPUT_SUPPRESSED;
				swallowed = Send(shell, WM_LBUTTONUP, 0, MAKELPARAM(10, 10)) && Send(shell, WM_KEYUP, 'A');
				host.Down[VK_LBUTTON] = false;
				host.Down['A'] = false;
				quiet = !presenter.Has_Pending() && shell.Input_State().Mouse_Owner(0) == UI_INPUT_NONE && shell.Input_State().Key_Owner('A') == UI_INPUT_NONE;
			}
			if (passes == 2) {
				Send(shell, WM_KEYDOWN, VK_RETURN);
			}
			return(false);
		});
		Check(suppressed, "input held as a screen opens is suppressed");
		Check(swallowed, "the releases of suppressed input are swallowed");
		Check(quiet && result == UI_RESULT_ACCEPTED, "suppressed releases queue nothing and a fresh press still accepts");
	}

	{
		UIVersionPresenterClass presenter({ "capture" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
		int passes = 0;
		bool owned = false;
		bool cancelled = false;
		bool swallowed = false;

		shell.Run_Modal(*view, [&](void) {
			passes++;
			if (passes == 1) {
				Send(shell, WM_LBUTTONDOWN, 0, MAKELPARAM(10, 10));
				Send(shell, WM_KEYDOWN, 'A');
				owned = shell.Input_State().Mouse_Owner(0) == UI_INPUT_RML && shell.Input_State().Key_Owner('A') == UI_INPUT_RML && host.Captured;
				host.Captured = false;
				Send(shell, WM_CAPTURECHANGED, 0, (LPARAM)1);
				cancelled = shell.Input_State().Mouse_Owner(0) == UI_INPUT_SUPPRESSED && shell.Input_State().Key_Owner('A') == UI_INPUT_RML && !host.Captured && shell.Modal() == view.get();
				swallowed = Send(shell, WM_LBUTTONUP, 0, MAKELPARAM(10, 10)) && !presenter.Has_Pending();
				Send(shell, WM_KEYUP, 'A');
			}
			if (passes == 2) {
				Send(shell, WM_KEYDOWN, VK_ESCAPE);
			}
			return(false);
		});
		Check(owned, "a modal owns the presses it is given and takes the capture");
		Check(cancelled, "losing the capture cancels the modal's held buttons and nothing else");
		Check(swallowed, "the release of a cancelled press is swallowed");
	}

	{
		// Two live screens need distinct data models, so the inner one is a message box.
		UIVersionPresenterClass outer({ "outer" });
		UIMessageBoxPresenterClass inner("Nested", { "OK", "Cancel" }, 0);
		std::unique_ptr<UIViewClass> outerview = UI_Version_View(outer);
		std::unique_ptr<UIViewClass> innerview = UI_Message_Box_View(inner);
		int passes = 0;
		bool nested = false;
		bool restored = false;
		bool swallowed = false;

		UIResult result = shell.Run_Modal(*outerview, [&](void) {
			passes++;
			if (passes == 1) {
				int innerpasses = 0;
				UIResult innerresult = shell.Run_Modal(*innerview, [&](void) {
					innerpasses++;
					if (innerpasses == 1) {
						nested = shell.Modal() == innerview.get() && shell.Modal_Depth() == 2;
						host.Down[VK_LBUTTON] = true;
						Send(shell, WM_LBUTTONDOWN, 0, MAKELPARAM(10, 10));
						Send(shell, WM_KEYDOWN, VK_ESCAPE);
					}
					return(innerpasses >= 5);
				});
				restored = innerresult == UI_RESULT_CANCELLED && shell.Modal() == outerview.get() && shell.Modal_Depth() == 1 && shell.Input_State().Mouse_Owner(0) == UI_INPUT_SUPPRESSED;
				swallowed = Send(shell, WM_LBUTTONUP, 0, MAKELPARAM(10, 10));
				host.Down[VK_LBUTTON] = false;
			}
			if (passes == 2) {
				Send(shell, WM_KEYDOWN, VK_RETURN);
			}
			return(false);
		});
		Check(nested, "a nested modal is the shown screen at depth two");
		Check(restored && result == UI_RESULT_ACCEPTED, "closing the inner modal restores the outer one, which still accepts");
		Check(swallowed, "a press held across the inner close is swallowed by the outer");
	}

	{
		UIVersionPresenterClass presenter({ "messages" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
		bool taken = false;
		bool focused = false;

		shell.Run_Modal(*view, [&](void) {
			taken = Send(shell, WM_MOUSEMOVE, 0, MAKELPARAM(2000, 2000))
				&& Send(shell, WM_XBUTTONDOWN, MAKEWPARAM(0, XBUTTON1), MAKELPARAM(10, 10))
				&& Send(shell, WM_XBUTTONUP, MAKEWPARAM(0, XBUTTON1), MAKELPARAM(10, 10))
				&& Send(shell, WM_MOUSEHWHEEL, MAKEWPARAM(0, WHEEL_DELTA), MAKELPARAM(10, 10));
			host.Down['C'] = true;
			Send(shell, WM_ACTIVATEAPP, 1);
			focused = shell.Input_State().Key_Owner('C') == UI_INPUT_SUPPRESSED;
			host.Down['C'] = false;
			Send(shell, WM_KEYUP, 'C');
			Send(shell, WM_KEYDOWN, VK_ESCAPE);
			return(false);
		});
		Check(taken, "a modal takes moves, side buttons and the horizontal wheel");
		Check(focused, "focus returning to a shown screen quarantines what is held");

		// The key that closed the screen is released after it; the shell swallows that release
		// and, once it ticks, holds nothing.
		Check(Send(shell, WM_KEYUP, VK_ESCAPE), "the release of the key that closed a screen is swallowed");
		shell.Tick();
		Check(!shell.Input_State().Any_Owned(), "nothing stays owned once the closing key is up and the shell has ticked");

		host.Down['C'] = true;
		Send(shell, WM_ACTIVATEAPP, 1);
		Check(shell.Input_State().Key_Owner('C') == UI_INPUT_NONE, "focus returning to an idle shell quarantines nothing");
		host.Down['C'] = false;
		Check(!Send(shell, WM_KEYUP, 'Z'), "a stray release meets an idle shell and reaches the game");
	}

	{
		class TextRecorderClass : public Rml::EventListener
		{
			public:
				std::vector<Rml::String> Texts;

				virtual void ProcessEvent(Rml::Event & event) override
				{
					Texts.push_back(event.GetParameter<Rml::String>("text", ""));
				}
		};

		TextRecorderClass recorder;
		UIVersionPresenterClass presenter({ "text" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);

		shell.Run_Modal(*view, [&](void) {
			Rml(*view).Document()->AddEventListener(Rml::EventId::Textinput, &recorder);
			Send(shell, WM_CHAR, 0xC3);
			Send(shell, WM_CHAR, 0xA9);
			Rml(*view).Document()->RemoveEventListener(Rml::EventId::Textinput, &recorder);
			Send(shell, WM_KEYDOWN, VK_ESCAPE);
			return(false);
		});
		Check(recorder.Texts.size() == 1 && recorder.Texts[0] == "\xC3\xA9", "two UTF-8 bytes on a narrow window reach the document as one character");
	}

	{
		char const * sample = "\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80";
		std::wstring wide;
		std::string text;

		std::wstring expected = { (wchar_t)0x00E9, (wchar_t)0x20AC, (wchar_t)0xD83D, (wchar_t)0xDE00 };
		Check(UI_UTF8_To_UTF16(sample, wide) && wide == expected, "UTF-8 converts to UTF-16");
		Check(UI_UTF16_To_UTF8(wide, text) && text == sample, "UTF-16 converts back to the same UTF-8");
		Check(!UI_UTF8_To_UTF16("\xC0\xAF", wide), "an overlong sequence is refused, not repaired");
		Check(!UI_UTF16_To_UTF8(std::wstring(1, (wchar_t)0xD800), text), "an unpaired surrogate is refused, not repaired");

		// The developer's clipboard is put back once the round trip is checked.
		Rml::String before;
		fixture.System->GetClipboardText(before);
		fixture.System->SetClipboardText(sample);
		Rml::String after;
		fixture.System->GetClipboardText(after);
		Check(after == sample, "clipboard text survives a round trip");
		fixture.System->SetClipboardText(before);
	}

	{
		UIVersionPresenterClass presenter({ "cursor" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
		int restored = host.Restored;
		bool requested = false;
		bool shown = false;

		shell.Run_Modal(*view, [&](void) {
			fixture.System->SetMouseCursor("text");
			requested = fixture.System->Cursor_Request() == UI_CURSOR_TEXT;
			shown = shell.Handle_Set_Cursor() && host.LastCursor == UI_CURSOR_TEXT;
			Send(shell, WM_KEYDOWN, VK_ESCAPE);
			return(false);
		});
		Check(requested, "a document's pointer request is kept");
		Check(shown, "WM_SETCURSOR shows the requested shape while a screen is shown");
		Check(host.Restored - restored == 1 && fixture.System->Cursor_Request() == UI_CURSOR_ARROW && host.LastCursor == UI_CURSOR_ARROW, "closing a screen puts the game's pointer back once and forgets the request");
		Check(!shell.Handle_Set_Cursor(), "WM_SETCURSOR is the game's again once nothing is shown");
	}

	{
		UIWaitBoxPresenterClass presenter("Working", false);
		std::unique_ptr<UIViewClass> view = UI_Wait_Box_View(presenter);

		Check(shell.Show_Modeless(*view), "a notice shows beside the game");
		Check(shell.Is_Modeless_Shown(*view) && view->Is_Shown(), "the shell lists the notice while it shows");
		Check(!shell.Screen_Shown(), "a notice is not a screen");
		shell.Hide_Modeless(*view);
		Check(!shell.Is_Modeless_Shown(*view) && !view->Is_Shown(), "hiding the notice unlists it");
		Check(shell.Show_Modeless(*view), "the notice shows again");

		shell.Shutdown();
		Check(!shell.Is_Modeless_Shown(*view) && !view->Is_Shown(), "shutdown releases a notice still shown");
		shell.Hide_Modeless(*view);
		Check(!shell.Use_Rml(), "a shut-down shell opens no document");
	}

	Check(shell.Init(), "the shell initialises again after a shutdown");
	shell.Shutdown();

	Check(fixture.Render->ReleasedGeometry == fixture.Render->Compiled, "the shell releases every geometry it compiled");
	Check(fixture.Render->ReleasedTextures == fixture.Render->Loaded + fixture.Render->Generated, "the shell releases every texture it made");
	Check(fixture.System->Problems == 0, "the shell's screens raise no RmlUi warning or error");
}

}


int main(void)
{
	Test_FreeType();
	Test_ImGui();
	Test_Coordinates();
	Test_Display_Presenter();
	Test_Game_Controls_Presenter();
	Test_Keys();
	Test_Keyboard_Presenter();
	Test_Sound_Presenter();
	Test_Strings();
	Test_Documents();
	Test_Shell();

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
