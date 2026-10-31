/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uishell.h"

#include "ui/rml/rmlfont.h"
#include "ui/rml/rmlfontfon.h"
#include "ui/rml/rmlimage.h"
#include "ui/rml/rmlkeys.h"
#include "ui/rml/rmlrender.h"
#include "ui/rml/rmlrendermath.h"
#include "ui/rml/rmlsurface.h"
#include "ui/rml/rmlsystem.h"
#include "ui/rml/rmlview.h"
#include "ui/screens/diplomacy/uidiplomacy.h"
#include "ui/screens/display/uidisplay.h"
#include "ui/screens/gamectrl/uigamectrl.h"
#include "ui/screens/gameopt/uigameopt.h"
#include "ui/screens/keyboard/uikeyboard.h"
#include "ui/screens/mainopt/uimainopt.h"
#include "ui/screens/mapgen/uimapgen.h"
#include "ui/screens/menu/uimenu.h"
#include "ui/screens/msgbox/uimsgbox.h"
#include "ui/screens/netlobby/uinetlobby.h"
#include "ui/screens/reconnect/uireconnect.h"
#include "ui/screens/restate/uirestate.h"
#include "ui/screens/savegame/uisavegame.h"
#include "ui/screens/scenario/uiscenario.h"
#include "ui/screens/skirmish/uiskirmish.h"
#include "ui/screens/skirmish/uiskirmishoptions.h"
#include "ui/screens/sound/uisound.h"
#include "ui/screens/version/uiversion.h"
#include "ui/screens/waitbox/uiwaitbox.h"
#include "ui/uihost.h"
#include "ui/uiinput.h"
#include "ui/uiscreen.h"
#include "ui/uiview.h"
#include "windowevent.hh"

#include "doctest/doctest.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#undef GetFirstChild
#undef GetNextSibling

#include <RmlUi/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Elements/ElementFormControlInput.h>
#include <RmlUi/Core/Elements/ElementFormControlSelect.h>
#include <RmlUi/Core/Elements/ElementFormControlTextArea.h>
#include <RmlUi/Core/Elements/ElementProgress.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "opents_strings.h"

#include <imgui.h>

extern int UITestSheetLookups;

namespace {

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
		int Invalid = 0;
		int Frames = 0;
		int Transforms = 0;
		int ClipMasks = 0;
		bool ClipMaskEnabled = false;
		bool ScissorOn = false;
		int Unclipped = 0;
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

		virtual Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override
		{
			Compiled++;

			std::uint32_t bytes = 0;
			bool finite = std::all_of(vertices.begin(), vertices.end(), [](Rml::Vertex const & vertex) {
				return(std::isfinite(vertex.position.x) && std::isfinite(vertex.position.y) && std::isfinite(vertex.tex_coord.x) && std::isfinite(vertex.tex_coord.y));
			});
			if (!UI_Render_Index_Range(std::span<int const>(indices.data(), indices.size()), vertices.size())
				|| vertices.size() > 65536 || !UI_Render_Byte_Count(vertices.size(), sizeof(Rml::Vertex), bytes) || !finite) {
				Invalid++;
			}

			return((Rml::CompiledGeometryHandle)Compiled);
		}

		virtual void RenderGeometry(Rml::CompiledGeometryHandle, Rml::Vector2f, Rml::TextureHandle) override
		{
			Rendered++;
			if (!ScissorOn) {
				Unclipped++;
			}
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

		virtual void EnableScissorRegion(bool enable) override
		{
			ScissorOn = enable;
		}

		virtual void SetScissorRegion(Rml::Rectanglei region) override
		{
			Scissors.push_back(region);
		}

		virtual void EnableClipMask(bool enable) override
		{
			ClipMaskEnabled = enable;
		}

		virtual void RenderToClipMask(Rml::ClipMaskOperation, Rml::CompiledGeometryHandle, Rml::Vector2f) override
		{
			ClipMasks++;
		}

		virtual void SetTransform(Rml::Matrix4f const * transform) override
		{
			if (transform != nullptr) {
				Transforms++;
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


class TestHostClass : public UIShellHostClass
{
	public:
		UIFrameRect Rect = { 0, 0, 1280, 800, 1.0f, 1.0f };
		bool Captured = false;
		bool Down[256] = {};
		bool Toggled[256] = {};
		int Presents = 0;
		int PresentsNow = 0;
		int Clears = 0;
		int Focuses = 0;
		int Now = 0;
		bool Held = false;
		UIShellClass * Shell = nullptr;
		std::function<void(void)> OnClear;

		virtual bool Key_Down(int virtualkey) const override
		{
			return(Down[virtualkey & 0xFF]);
		}

		virtual bool Key_Toggled(int virtualkey) const override
		{
			return(Toggled[virtualkey & 0xFF]);
		}

		virtual std::string System_Font_Path(char const *) const override
		{
			return(std::string());
		}

		std::string SideName;

		virtual std::string Side_Name(int) const override
		{
			return(SideName);
		}

		std::vector<std::string> Samples;
		int Clicks = 0;

		virtual void Play_Sample(char const * name, float) override
		{
			Samples.push_back(name);
		}

		virtual void Play_Click(void) override
		{
			Clicks++;
		}

		bool Animate = false;

		virtual bool Animate_Screens(void) const override
		{
			return(Animate);
		}

		int Magnification = 1;

		virtual int Art_Magnification(void) const override
		{
			return(Magnification);
		}

		virtual bool Bitmap_System_Font(void) const override
		{
			return(true);
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

		std::string Clipboard;

		virtual std::string Clipboard_Text(void) const override
		{
			return(Clipboard);
		}

		virtual void Set_Clipboard_Text(std::string const & text) override
		{
			Clipboard = text;
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

		virtual void Present_Now(void) override
		{
			Presents++;
			PresentsNow++;
			if (Shell != nullptr) {
				Shell->Render_Overlay();
			}
		}

		virtual bool Movie_Playing(void) const override
		{
			return(false);
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

		virtual char const * String(int) const override
		{
			return("string");
		}

		virtual void Log(char const * text) override
		{
			std::printf("  shell: %s", text);
		}

		std::vector<std::string> Errors;

		virtual void Show_Error(char const * text) override
		{
			Errors.emplace_back(text);
		}

		virtual int Milliseconds(void) const override
		{
			return(Held ? Now : (int)GetTickCount64());
		}
};


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


struct ShellFixtureType
{
	TestHostClass Host;
	RecordingRenderInterfaceClass * Render;
	CountingSystemClass * System;
	UIShellClass Shell;

	ShellFixtureType(std::unique_ptr<Rml::FileInterface> files = nullptr) :
		Render(new RecordingRenderInterfaceClass()),
		System(new CountingSystemClass(Host)),
		Shell(Host, std::unique_ptr<UIRmlSystemClass>(System), std::move(files), std::unique_ptr<UIRmlRenderClass>(Render))
	{
		Host.Shell = &Shell;
	}
};


struct PointType
{
	int X;
	int Y;
};


bool Send(UIShellClass & shell, WindowEvent const & event)
{
	return(shell.Handle_Window_Event(event));
}


bool Move(UIShellClass & shell, PointType at)
{
	WindowEvent event;
	event.Type = WINDOW_EVENT_MOUSE_MOVE;
	event.X = at.X;
	event.Y = at.Y;
	return(Send(shell, event));
}


bool Press(UIShellClass & shell, WindowMouseButton button, PointType at)
{
	WindowEvent event;
	event.Type = WINDOW_EVENT_MOUSE_DOWN;
	event.Button = button;
	event.X = at.X;
	event.Y = at.Y;
	return(Send(shell, event));
}


bool Release(UIShellClass & shell, WindowMouseButton button, PointType at)
{
	WindowEvent event;
	event.Type = WINDOW_EVENT_MOUSE_UP;
	event.Button = button;
	event.X = at.X;
	event.Y = at.Y;
	return(Send(shell, event));
}


bool Wheel(UIShellClass & shell, PointType at, float notches, bool horizontal)
{
	WindowEvent event;
	event.Type = WINDOW_EVENT_MOUSE_WHEEL;
	event.X = at.X;
	event.Y = at.Y;
	event.Wheel = notches;
	event.Horizontal = horizontal;
	return(Send(shell, event));
}


bool Key_Down(UIShellClass & shell, int virtualkey, int modifiers = 0)
{
	WindowEvent event;
	event.Type = WINDOW_EVENT_KEY_DOWN;
	event.VirtualKey = virtualkey;
	event.Modifiers = modifiers;
	return(Send(shell, event));
}


bool Key_Up(UIShellClass & shell, int virtualkey, int modifiers = 0)
{
	WindowEvent event;
	event.Type = WINDOW_EVENT_KEY_UP;
	event.VirtualKey = virtualkey;
	event.Modifiers = modifiers;
	return(Send(shell, event));
}


bool Text(UIShellClass & shell, char32_t code)
{
	WindowEvent event;
	event.Type = WINDOW_EVENT_TEXT;
	event.Text = code;
	return(Send(shell, event));
}


bool Focus(UIShellClass & shell, bool focused)
{
	WindowEvent event;
	event.Type = focused ? WINDOW_EVENT_FOCUS_GAINED : WINDOW_EVENT_FOCUS_LOST;
	return(Send(shell, event));
}


bool Capture_Lost(UIShellClass & shell)
{
	WindowEvent event;
	event.Type = WINDOW_EVENT_CAPTURE_LOST;
	return(Send(shell, event));
}


Rml::Vector2f Center_Of(Rml::Element * element)
{
	return(element->GetAbsoluteOffset(Rml::BoxArea::Border) + element->GetBox().GetSize(Rml::BoxArea::Border) * 0.5f);
}


PointType Element_Point(TestHostClass const & host, Rml::Element * element)
{
	Rml::Vector2f center = Center_Of(element);
	return(PointType{ (int)center.x + host.Rect.X, (int)center.y + host.Rect.Y });
}


void Click_Through_Hook(UIShellClass & shell, TestHostClass & host, Rml::Element * element)
{
	PointType at = Element_Point(host, element);

	Move(shell, at);
	host.Down[VK_LBUTTON] = true;
	Press(shell, WINDOW_BUTTON_LEFT, at);
	host.Down[VK_LBUTTON] = false;
	Release(shell, WINDOW_BUTTON_LEFT, at);
}


bool Slider_Parts(Rml::Element * slider, Rml::Element * & bar, Rml::Element * & track)
{
	bar = nullptr;
	track = nullptr;

	for (int index = 0; index < slider->GetNumChildren(true); index++) {
		Rml::Element * child = slider->GetChild(index);
		if (child->GetTagName() == "sliderbar") {
			bar = child;
		} else if (child->GetTagName() == "slidertrack") {
			track = child;
		}
	}

	return(bar != nullptr && track != nullptr);
}


PointType Past_Track_End(TestHostClass const & host, Rml::Element * bar, Rml::Element * track)
{
	int const y = (int)Center_Of(bar).y + host.Rect.Y;
	int const past = (int)(track->GetAbsoluteOffset(Rml::BoxArea::Border).x + track->GetBox().GetSize(Rml::BoxArea::Border).x) + host.Rect.X + 40;
	return(PointType{ past, y });
}


bool Drag_Slider_To_End(UIShellClass & shell, TestHostClass & host, Rml::Element * slider)
{
	Rml::Element * bar = nullptr;
	Rml::Element * track = nullptr;
	if (!Slider_Parts(slider, bar, track)) {
		return(false);
	}

	PointType const start = Element_Point(host, bar);
	PointType const end = Past_Track_End(host, bar, track);

	bool consumed = Move(shell, start);
	host.Down[VK_LBUTTON] = true;
	consumed = Press(shell, WINDOW_BUTTON_LEFT, start) && consumed;
	consumed = Move(shell, end) && consumed;
	host.Down[VK_LBUTTON] = false;
	consumed = Release(shell, WINDOW_BUTTON_LEFT, end) && consumed;
	return(consumed);
}


std::string Read_Text(std::filesystem::path const & path)
{
	std::ifstream stream(path, std::ios::binary);
	return(std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>()));
}


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


TEST_CASE("FreeType initializes a library of the 2.x API")
{
	FT_Library library = nullptr;
	CHECK_MESSAGE(FT_Init_FreeType(&library) == 0, "FreeType initializes a library");
	CHECK_MESSAGE(library != nullptr, "FreeType initializes a library");

	if (library != nullptr) {
		FT_Int major = 0;
		FT_Int minor = 0;
		FT_Int patch = 0;
		FT_Library_Version(library, &major, &minor, &patch);
		std::printf("  FreeType %d.%d.%d\n", major, minor, patch);
		CHECK_MESSAGE(major == 2, "FreeType reports the 2.x API");

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


TEST_CASE("Dear ImGui matches its header, creates a context, and draws a frame")
{
	CHECK_MESSAGE(IMGUI_CHECKVERSION(), "ImGui header and library agree on structure layouts");

	ImGuiContext * context = ImGui::CreateContext();
	CHECK_MESSAGE(context != nullptr, "ImGui creates a context");
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
	REQUIRE_MESSAGE(data != nullptr, "the first ImGui frame produces draw data");
	CHECK_MESSAGE(data->Valid, "the first ImGui frame produces draw data");
	REQUIRE_MESSAGE(data != nullptr, "the first ImGui frame draws geometry");
	CHECK_MESSAGE(data->CmdLists.Size > 0, "the first ImGui frame draws geometry");
	CHECK_MESSAGE(data->TotalVtxCount > 0, "the first ImGui frame draws geometry");

	ImTextureData * atlas = nullptr;
	if (data != nullptr && data->Textures != nullptr && data->Textures->Size == 1) {
		atlas = (*data->Textures)[0];
	}
	CHECK_MESSAGE(atlas != nullptr, "the first ImGui frame lists one texture, the font atlas");
	REQUIRE_MESSAGE(atlas != nullptr, "the font atlas asks to be created");
	CHECK_MESSAGE(atlas->Status == ImTextureStatus_WantCreate, "the font atlas asks to be created");
	REQUIRE_MESSAGE(atlas != nullptr, "the font atlas is RGBA32 with a size");
	CHECK_MESSAGE(atlas->Format == ImTextureFormat_RGBA32, "the font atlas is RGBA32 with a size");
	CHECK_MESSAGE(atlas->Width > 0, "the font atlas is RGBA32 with a size");
	CHECK_MESSAGE(atlas->Height > 0, "the font atlas is RGBA32 with a size");

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
	CHECK_MESSAGE(acknowledged, "the second ImGui frame leaves every texture acknowledged");

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
	CHECK_MESSAGE(textured, "every ImGui draw command carries a texture id");

	ImGui::DestroyContext(context);
}


TEST_CASE("the frame maps client pixels to game pixels and tells the bars from the picture")
{
	UIPointerPosition position = UI_Client_To_Overlay(0, 0, 640, 480, 100, 200);
	CHECK_MESSAGE(position.Inside, "an unscaled frame maps client pixels to itself");
	CHECK_MESSAGE(position.X == 100, "an unscaled frame maps client pixels to itself");
	CHECK_MESSAGE(position.Y == 200, "an unscaled frame maps client pixels to itself");

	position = UI_Client_To_Overlay(160, 0, 960, 720, 160, 0);
	CHECK_MESSAGE(position.Inside, "the top left corner of the frame is inside at the origin");
	CHECK_MESSAGE(position.X == 0, "the top left corner of the frame is inside at the origin");
	CHECK_MESSAGE(position.Y == 0, "the top left corner of the frame is inside at the origin");
	position = UI_Client_To_Overlay(160, 0, 960, 720, 159, 10);
	CHECK_MESSAGE(!position.Inside, "a point on the left bar is outside and keeps its offset");
	CHECK_MESSAGE(position.X == -1, "a point on the left bar is outside and keeps its offset");
	position = UI_Client_To_Overlay(160, 0, 960, 720, 1119, 719);
	CHECK_MESSAGE(position.Inside, "the last pixel of the frame is inside");
	CHECK_MESSAGE(position.X == 959, "the last pixel of the frame is inside");
	CHECK_MESSAGE(position.Y == 719, "the last pixel of the frame is inside");
	position = UI_Client_To_Overlay(160, 0, 960, 720, 1120, 719);
	CHECK_MESSAGE(!position.Inside, "the right edge is exclusive");
	CHECK_MESSAGE(position.X == 960, "the right edge is exclusive");

	position = UI_Client_To_Overlay(0, 0, 960, 720, 959, 719);
	CHECK_MESSAGE(position.Inside, "a fractional scale keeps its last pixel inside");
	position = UI_Client_To_Overlay(0, 0, 960, 720, 960, 0);
	CHECK_MESSAGE(!position.Inside, "a fractional scale keeps the edge exclusive");

	position = UI_Client_To_Overlay(0, 0, 960, 720, -5, 3);
	CHECK_MESSAGE(!position.Inside, "a negative client position is outside with its offset kept");
	CHECK_MESSAGE(position.X == -5, "a negative client position is outside with its offset kept");
}


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


void Drive(UIPresenterClass & presenter, char const * name, int value, char const * text)
{
	UIIntent intent;
	intent.Name = name;
	intent.Value = value;
	intent.Text = text;
	presenter.Queue(intent);
	presenter.Drain();
}


class RecordingDisplayServiceClass : public UIDisplayServiceClass
{
	public:
		std::vector<std::string> Calls;

		virtual void Set_Stretch_Movies(bool on) override { Calls.push_back(on ? "stretch on" : "stretch off"); }
		virtual void Set_System_Cursor(bool on) override { Calls.push_back(on ? "system cursor on" : "system cursor off"); }
};


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


TEST_CASE("display edits are held until the player accepts")
{
	RecordingDisplayServiceClass service;
	UIDisplayPresenterClass presenter(service, Display_Fixture());
	Drive(presenter, "select", 2);
	Drive(presenter, "stretch", 1);
	Drive(presenter, "systemcursor", 1);
	CHECK_MESSAGE(presenter.State.Selected == 2, "display edits are held until the player accepts");
	CHECK_MESSAGE(presenter.State.StretchMovies, "display edits are held until the player accepts");
	CHECK_MESSAGE(presenter.State.SystemCursor, "display edits are held until the player accepts");
	CHECK_MESSAGE(service.Calls.empty(), "display edits are held until the player accepts");
	CHECK_MESSAGE(!presenter.Picked.has_value(), "display edits are held until the player accepts");

	Drive(presenter, "ok");
	REQUIRE_MESSAGE(presenter.Result.has_value(), "accepting the display options applies both switches");
	CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "accepting the display options applies both switches");
	CHECK_MESSAGE((service.Calls == std::vector<std::string>{ "stretch on", "system cursor on" }), "accepting the display options applies both switches");
	REQUIRE_MESSAGE(presenter.Picked.has_value(), "accepting with a new row hands the caller that mode to try");
	CHECK_MESSAGE(presenter.Picked->Width == 1920, "accepting with a new row hands the caller that mode to try");
	CHECK_MESSAGE(presenter.Picked->Height == 1080, "accepting with a new row hands the caller that mode to try");
}


TEST_CASE("accepting on the starting row applies the switches and tries no mode")
{
	RecordingDisplayServiceClass service;
	UIDisplayPresenterClass presenter(service, Display_Fixture());
	Drive(presenter, "select", 2);
	Drive(presenter, "select", 1);
	Drive(presenter, "ok");
	REQUIRE_MESSAGE(presenter.Result.has_value(), "accepting on the starting row applies the switches and tries no mode");
	REQUIRE_MESSAGE(!presenter.Picked.has_value(), "accepting on the starting row applies the switches and tries no mode");
	CHECK_MESSAGE((service.Calls == std::vector<std::string>{ "stretch off", "system cursor off" }), "accepting on the starting row applies the switches and tries no mode");
}


TEST_CASE("cancelling the display options applies nothing")
{
	RecordingDisplayServiceClass service;
	UIDisplayPresenterClass presenter(service, Display_Fixture());
	Drive(presenter, "select", 0);
	Drive(presenter, "stretch", 1);
	Drive(presenter, "systemcursor", 1);
	Drive(presenter, "cancel");
	REQUIRE_MESSAGE(presenter.Result.has_value(), "cancelling the display options applies nothing");
	CHECK_MESSAGE(*presenter.Result == UI_RESULT_CANCELLED, "cancelling the display options applies nothing");
	CHECK_MESSAGE(service.Calls.empty(), "cancelling the display options applies nothing");
	CHECK_MESSAGE(!presenter.Picked.has_value(), "cancelling the display options applies nothing");
}


TEST_CASE("a row outside the list selects nothing")
{
	RecordingDisplayServiceClass service;
	UIDisplayPresenterClass presenter(service, Display_Fixture());
	Drive(presenter, "select", 7);
	CHECK_MESSAGE(presenter.State.Selected == -1, "a row outside the list selects nothing");
	Drive(presenter, "ok");
	REQUIRE_MESSAGE(presenter.Result.has_value(), "accepting with no row selected tries no mode");
	CHECK_MESSAGE(!presenter.Picked.has_value(), "accepting with no row selected tries no mode");
}


TEST_CASE("a pick with no starting row is a mode to try")
{
	RecordingDisplayServiceClass service;
	UIDisplayState state = Display_Fixture();
	state.Selected = -1;
	UIDisplayPresenterClass presenter(service, state);
	Drive(presenter, "select", 0);
	Drive(presenter, "ok");
	REQUIRE_MESSAGE(presenter.Picked.has_value(), "a pick with no starting row is a mode to try");
	CHECK_MESSAGE(presenter.Picked->Width == 640, "a pick with no starting row is a mode to try");
	CHECK_MESSAGE(presenter.Picked->Height == 400, "a pick with no starting row is a mode to try");
}


TEST_CASE("the mode confirmation counts down ten seconds and cancels when they pass")
{
	FakeClockClass clock;
	UIConfirmModePresenterClass presenter(clock);
	CHECK_MESSAGE(presenter.Seconds == 10, "the confirmation starts with the full ten seconds");

	clock.Now = 5000;
	presenter.Refresh();
	CHECK_MESSAGE(presenter.Seconds == 10, "the clock starts at the first refresh");
	CHECK_MESSAGE(!presenter.Result.has_value(), "the clock starts at the first refresh");

	clock.Now = 5000 + 8100;
	presenter.Refresh();
	CHECK_MESSAGE(presenter.Seconds == 2, "the seconds left count down");
	CHECK_MESSAGE(!presenter.Result.has_value(), "the seconds left count down");

	clock.Now = 5000 + 10000;
	presenter.Refresh();
	REQUIRE_MESSAGE(presenter.Result.has_value(), "silence cancels the confirmation when the timeout passes");
	CHECK_MESSAGE(*presenter.Result == UI_RESULT_CANCELLED, "silence cancels the confirmation when the timeout passes");
	CHECK_MESSAGE(presenter.TimedOut, "silence cancels the confirmation when the timeout passes");
	CHECK_MESSAGE(presenter.Seconds == 0, "silence cancels the confirmation when the timeout passes");
}


TEST_CASE("OK keeps the mode and a later refresh does not overturn it")
{
	FakeClockClass clock;
	UIConfirmModePresenterClass presenter(clock);
	presenter.Refresh();
	Drive(presenter, "ok");
	clock.Now = 20000;
	presenter.Refresh();
	REQUIRE_MESSAGE(presenter.Result.has_value(), "OK keeps the mode and a later refresh does not overturn it");
	CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "OK keeps the mode and a later refresh does not overturn it");
	CHECK_MESSAGE(!presenter.TimedOut, "OK keeps the mode and a later refresh does not overturn it");
}


TEST_CASE("Cancel refuses the mode before the timeout")
{
	FakeClockClass clock;
	UIConfirmModePresenterClass presenter(clock);
	presenter.Refresh();
	Drive(presenter, "cancel");
	REQUIRE_MESSAGE(presenter.Result.has_value(), "Cancel refuses the mode before the timeout");
	CHECK_MESSAGE(*presenter.Result == UI_RESULT_CANCELLED, "Cancel refuses the mode before the timeout");
	CHECK_MESSAGE(!presenter.TimedOut, "Cancel refuses the mode before the timeout");
}


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


TEST_CASE("virtual keys and RmlUi key identifiers map to each other")
{
	CHECK_MESSAGE(UI_Key_Identifier(0x41) == Rml::Input::KI_A, "letters, digits and function keys map to their identifiers");
	CHECK_MESSAGE(UI_Key_Identifier(0x39) == Rml::Input::KI_9, "letters, digits and function keys map to their identifiers");
	CHECK_MESSAGE(UI_Key_Identifier(0x70) == Rml::Input::KI_F1, "letters, digits and function keys map to their identifiers");
	CHECK_MESSAGE(UI_Key_Identifier(0x1B) == Rml::Input::KI_ESCAPE, "named keys map and an unassigned code stays unknown");
	CHECK_MESSAGE(UI_Key_Identifier(0x07) == Rml::Input::KI_UNKNOWN, "named keys map and an unassigned code stays unknown");

	bool roundtrip = true;
	for (int code = 0; code < 256; code++) {
		Rml::Input::KeyIdentifier key = UI_Key_Identifier(code);
		if (key != Rml::Input::KI_UNKNOWN && UI_Key_Identifier(UI_Virtual_Key(key)) != key) {
			roundtrip = false;
		}
	}
	CHECK_MESSAGE(roundtrip, "every identifier maps back to a virtual key that maps to it");
	CHECK_MESSAGE(UI_Virtual_Key(Rml::Input::KI_UNKNOWN) == 0, "the unknown identifier has no virtual key");

	CHECK_MESSAGE(UI_Key_Number(Rml::Input::KI_A, false, true, false) == 577, "Control and A make the KEYBOARD.INI number 577");
	CHECK_MESSAGE(UI_Key_Number(Rml::Input::KI_R, true, false, false) == 338, "Shift adds 256 and a bare key is its virtual key");
	CHECK_MESSAGE(UI_Key_Number(Rml::Input::KI_X, false, false, false) == 88, "Shift adds 256 and a bare key is its virtual key");
	CHECK_MESSAGE(UI_Key_Number(Rml::Input::KI_F5, false, false, true) == (0x74 | 0x400), "Alt adds 1024");
	CHECK_MESSAGE(UI_Key_Number(Rml::Input::KI_LSHIFT, true, false, false) == 0, "a modifier on its own is no key");
	CHECK_MESSAGE(UI_Key_Number(Rml::Input::KI_RCONTROL, false, true, false) == 0, "a modifier on its own is no key");
	CHECK_MESSAGE(UI_Key_Number(Rml::Input::KI_LMENU, false, false, true) == 0, "a modifier on its own is no key");
	CHECK_MESSAGE(UI_Key_Number(Rml::Input::KI_UNKNOWN, false, false, false) == 0, "an unknown key is no key");
}


TEST_CASE("the keyboard presenter lists, sorts, rebinds and resets the hotkeys")
{
	RecordingKeyboardServiceClass service;
	UIKeyboardPresenterClass presenter(service, Keyboard_Fixture());

	CHECK_MESSAGE((presenter.State.Categories == std::vector<std::string>{ "Interface", "Selection" }), "the categories are listed once each, sorted without regard to case");
	CHECK_MESSAGE(presenter.State.Category == 0, "the first category opens with its commands sorted by name and none selected");
	CHECK_MESSAGE((Visible_Commands(presenter.State) == std::vector<int>{ 3, 1 }), "the first category opens with its commands sorted by name and none selected");
	CHECK_MESSAGE(presenter.State.Selected == -1, "the first category opens with its commands sorted by name and none selected");
	REQUIRE_MESSAGE(presenter.State.Visible.size() == 2, "a row carries its command's name");
	CHECK_MESSAGE(presenter.State.Visible[0].Name == "Alliance", "a row carries its command's name");

	Drive(presenter, "category", 1);
	CHECK_MESSAGE((Visible_Commands(presenter.State) == std::vector<int>{ 2, 0 }), "another category lists its own commands with the description cleared");
	CHECK_MESSAGE(presenter.State.Description.empty(), "another category lists its own commands with the description cleared");

	Drive(presenter, "select", 0);
	CHECK_MESSAGE(presenter.State.Description == "Selects the view", "selecting a command shows its description and shortcut");
	CHECK_MESSAGE(presenter.State.Shortcut == "K577", "selecting a command shows its description and shortcut");

	Drive(presenter, "capture", 338);
	CHECK_MESSAGE(presenter.State.CapturedName == "K338", "a captured key names the command that holds it");
	CHECK_MESSAGE(presenter.State.AssignedTo == "Toggle Repair", "a captured key names the command that holds it");

	Drive(presenter, "capture", 999);
	CHECK_MESSAGE(presenter.State.AssignedTo.empty(), "a free key names no command");

	Drive(presenter, "capture", 338);
	Drive(presenter, "assign");
	CHECK_MESSAGE(presenter.Key_Of(0) == 338, "assigning moves the key to the selected command and unbinds its previous holder");
	CHECK_MESSAGE(presenter.Key_Of(1) == 0, "assigning moves the key to the selected command and unbinds its previous holder");
	CHECK_MESSAGE(presenter.Owner_Of(577) == -1, "assigning moves the key to the selected command and unbinds its previous holder");
	CHECK_MESSAGE(presenter.State.Shortcut == "K338", "assigning shows the new shortcut and clears the capture");
	CHECK_MESSAGE(presenter.State.Captured == 0, "assigning shows the new shortcut and clears the capture");
	CHECK_MESSAGE(presenter.State.AssignedTo.empty(), "assigning shows the new shortcut and clears the capture");

	Drive(presenter, "select", 2);
	Drive(presenter, "assign");
	CHECK_MESSAGE(presenter.Key_Of(2) == 0, "assigning an empty capture unbinds the command");
	CHECK_MESSAGE(presenter.State.Shortcut.empty(), "assigning an empty capture unbinds the command");

	Drive(presenter, "select", -1);
	Drive(presenter, "capture", 65);
	Drive(presenter, "assign");
	CHECK_MESSAGE(presenter.Owner_Of(65) == -1, "assigning with no command selected changes nothing");
	CHECK_MESSAGE(presenter.State.Captured == 65, "assigning with no command selected changes nothing");

	CHECK_MESSAGE(service.Calls.empty(), "nothing reaches the game before the player accepts");

	Drive(presenter, "ok");
	REQUIRE_MESSAGE(presenter.Result.has_value(), "OK saves the edited table");
	CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "OK saves the edited table");
	CHECK_MESSAGE(service.Calls == std::vector<std::string>{ "save 0=338" }, "OK saves the edited table");

	{
		RecordingKeyboardServiceClass quiet;
		UIKeyboardPresenterClass cancelled(quiet, Keyboard_Fixture());
		Drive(cancelled, "select", 3);
		Drive(cancelled, "capture", 70);
		Drive(cancelled, "assign");
		Drive(cancelled, "cancel");
		REQUIRE_MESSAGE(cancelled.Result.has_value(), "Cancel drops the edits without a call");
		CHECK_MESSAGE(*cancelled.Result == UI_RESULT_CANCELLED, "Cancel drops the edits without a call");
		CHECK_MESSAGE(quiet.Calls.empty(), "Cancel drops the edits without a call");
	}

	{
		RecordingKeyboardServiceClass declined;
		declined.ConfirmAnswer = false;
		UIKeyboardPresenterClass kept(declined, Keyboard_Fixture());
		Drive(kept, "category", 1);
		Drive(kept, "reset");
		CHECK_MESSAGE(declined.Calls == std::vector<std::string>{ "confirm" }, "a declined reset asks and changes nothing");
		CHECK_MESSAGE(kept.Key_Of(0) == 577, "a declined reset asks and changes nothing");
		CHECK_MESSAGE(kept.State.Category == 1, "a declined reset asks and changes nothing");
	}

	{
		RecordingKeyboardServiceClass confirmed;
		confirmed.ResetTable = { { 65, 3 } };
		UIKeyboardPresenterClass reset(confirmed, Keyboard_Fixture());
		Drive(reset, "category", 1);
		Drive(reset, "select", 0);
		Drive(reset, "reset");
		CHECK_MESSAGE((confirmed.Calls == std::vector<std::string>{ "confirm", "reset" }), "a confirmed reset reloads the table from the game");
		CHECK_MESSAGE(reset.Key_Of(3) == 65, "a confirmed reset reloads the table from the game");
		CHECK_MESSAGE(reset.Key_Of(0) == 0, "a confirmed reset reloads the table from the game");
		CHECK_MESSAGE(reset.State.Category == 0, "a reset reopens the first category with nothing selected");
		CHECK_MESSAGE(reset.State.Selected == -1, "a reset reopens the first category with nothing selected");
		CHECK_MESSAGE(reset.State.Description.empty(), "a reset reopens the first category with nothing selected");
	}
}


TEST_CASE("the sound presenter rounds volumes to slider levels and previews each move")
{
	CHECK_MESSAGE(UISoundPresenterClass::Level_Of(0.7f) == 7, "volumes round to slider levels the way the dialog did");
	CHECK_MESSAGE(UISoundPresenterClass::Level_Of(0.04f) == 0, "volumes round to slider levels the way the dialog did");
	CHECK_MESSAGE(UISoundPresenterClass::Level_Of(1.0f) == 10, "volumes round to slider levels the way the dialog did");

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
	CHECK_MESSAGE(service.Calls.empty(), "a slider reporting the level it already holds previews nothing");

	Drive(presenter, "score", 4);
	CHECK_MESSAGE(presenter.State.Score == 4, "a music slider move previews the new volume at once");
	CHECK_MESSAGE(service.Joined() == "score 0.4 feedback", "a music slider move previews the new volume at once");
	service.Calls.clear();

	Drive(presenter, "voice", 2);
	service.Calls.clear();
	Drive(presenter, "voice", 14);
	CHECK_MESSAGE(presenter.State.Voice == 10, "a slider level is clamped to the top step");
	CHECK_MESSAGE(service.Joined() == "voice 1.0 feedback", "a slider level is clamped to the top step");
	service.Calls.clear();

	Drive(presenter, "shuffle", 1);
	CHECK_MESSAGE(presenter.State.Shuffle, "turning shuffle on turns repeat off");
	CHECK_MESSAGE(!presenter.State.Repeat, "turning shuffle on turns repeat off");
	CHECK_MESSAGE(service.Joined() == "shuffle on; repeat off", "turning shuffle on turns repeat off");
	service.Calls.clear();

	Drive(presenter, "repeat", 1);
	CHECK_MESSAGE(presenter.State.Repeat, "turning repeat on turns shuffle off");
	CHECK_MESSAGE(!presenter.State.Shuffle, "turning repeat on turns shuffle off");
	CHECK_MESSAGE(service.Joined() == "repeat on; shuffle off", "turning repeat on turns shuffle off");
	service.Calls.clear();

	Drive(presenter, "repeat", 0);
	CHECK_MESSAGE(!presenter.State.Repeat, "turning repeat off leaves shuffle alone");
	CHECK_MESSAGE(service.Joined() == "repeat off", "turning repeat off leaves shuffle alone");
	service.Calls.clear();

	Drive(presenter, "play");
	CHECK_MESSAGE(service.Joined() == "play 6", "play starts the selected track");
	service.Calls.clear();

	Drive(presenter, "select", 2);
	Drive(presenter, "play");
	CHECK_MESSAGE(presenter.State.Selected == 2, "play starts a newly selected track");
	CHECK_MESSAGE(service.Joined() == "play 9", "play starts a newly selected track");
	service.Calls.clear();

	Drive(presenter, "select", 7);
	Drive(presenter, "play");
	CHECK_MESSAGE(presenter.State.Selected == -1, "a row outside the list selects nothing and plays nothing");
	CHECK_MESSAGE(service.Calls.empty(), "a row outside the list selects nothing and plays nothing");

	Drive(presenter, "stop");
	CHECK_MESSAGE(service.Joined() == "stop", "stop fades the music out");
	service.Calls.clear();

	Drive(presenter, "ok");
	REQUIRE_MESSAGE(presenter.Result.has_value(), "OK re-applies the levels without feedback and closes");
	CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "OK re-applies the levels without feedback and closes");
	CHECK_MESSAGE(service.Joined() == "score 0.4; sound 0.5; voice 1.0", "OK re-applies the levels without feedback and closes");
}


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


TEST_CASE("the game controls presenter names the starting slider positions it has names for")
{
	RecordingGameControlsServiceClass service;
	UIGameControlsState state;
	state.Speed = 3;
	state.Scroll = 3;
	state.Detail = 2;
	state.Difficulty = 1;
	state.InGame = false;
	state.HasDifficulty = true;
	state.SpeedNames = { "Slowest", "Slower", "Slow", "Medium", "Fast", "Faster", "Fastest" };
	state.DetailNames = { "Low", "Medium", "High" };

	UIGameControlsPresenterClass presenter(service, state);
	CHECK_MESSAGE(presenter.State.SpeedName == "Medium", "the presenter names the starting slider positions it has names for");
	CHECK_MESSAGE(presenter.State.DetailName == "High", "the presenter names the starting slider positions it has names for");
	CHECK_MESSAGE(presenter.State.ScrollName.empty(), "the presenter names the starting slider positions it has names for");

	Drive(presenter, "speed", 5);
	Drive(presenter, "detail", 9);
	Drive(presenter, "cameo", 1);
	Drive(presenter, "edge", 1);
	Drive(presenter, "difficulty", 2);
	CHECK_MESSAGE(presenter.State.Speed == 5, "edits are held in the state and clamped");
	CHECK_MESSAGE(presenter.State.Detail == 2, "edits are held in the state and clamped");
	CHECK_MESSAGE(presenter.State.CameoText, "edits are held in the state and clamped");
	CHECK_MESSAGE(presenter.State.EdgeScroll, "edits are held in the state and clamped");
	CHECK_MESSAGE(presenter.State.SpeedName == "Slower", "an edit renames the slider position, the speed by how far along the slider it lies");
	CHECK_MESSAGE(presenter.State.DetailName == "High", "an edit renames the slider position, the speed by how far along the slider it lies");
	CHECK_MESSAGE(service.Calls.empty(), "nothing is applied before the player accepts");

	Drive(presenter, "ok");
	REQUIRE_MESSAGE(presenter.Result.has_value(), "OK accepts the game controls");
	CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "OK accepts the game controls");
	CHECK_MESSAGE(service.Joined() == "speed 5; scroll 3; detail 2; cameo on; lines off; tooltips off; coasting off; edge on; difficulty 2; save", "OK applies the settings in the accept path's order and saves");
}


TEST_CASE("the game controls Sound button does nothing without an audio device")
{
	RecordingGameControlsServiceClass service;
	UIGameControlsState state;
	state.InGame = true;
	state.HasDifficulty = false;

	UIGameControlsPresenterClass presenter(service, state);
	Drive(presenter, "sound");
	REQUIRE_MESSAGE(!presenter.Result.has_value(), "the Sound button does nothing without an audio device");
	CHECK_MESSAGE(service.Calls.empty(), "the Sound button does nothing without an audio device");

	presenter.State.SoundEnabled = true;
	Drive(presenter, "sound");
	REQUIRE_MESSAGE(presenter.Result.has_value(), "the Sound button accepts and names the sound options next");
	CHECK_MESSAGE(presenter.Next == UIGameControlsPresenterClass::NEXT_SOUND, "the Sound button accepts and names the sound options next");
	CHECK_MESSAGE(service.Joined() == "speed 0; scroll 0; detail 0; cameo off; lines off; tooltips off; coasting off; edge off; save", "the in-game screen applies the game speed and no difficulty");
}


TEST_CASE("cancelling the game controls applies nothing")
{
	RecordingGameControlsServiceClass service;
	UIGameControlsState state;
	UIGameControlsPresenterClass presenter(service, state);
	Drive(presenter, "scroll", 1);
	Drive(presenter, "cancel");
	REQUIRE_MESSAGE(presenter.Result.has_value(), "Cancel applies nothing");
	CHECK_MESSAGE(*presenter.Result == UI_RESULT_CANCELLED, "Cancel applies nothing");
	CHECK_MESSAGE(service.Calls.empty(), "Cancel applies nothing");
}


class RecordingMapGenServiceClass : public UIMapGenServiceClass
{
	public:
		UIMapGenState Settings;
		std::vector<std::string> Calls;

		RecordingMapGenServiceClass(void)
		{
			Settings.Cliffs.Value = 50;
			Settings.Cliffs.Maximum = 80;
		}

		virtual void Read(UIMapGenState & state) override { state = Settings; }
		virtual void Set(char const * name, int value) override
		{
			Calls.push_back(std::string(name) + " " + std::to_string(value));
			if (std::strcmp(name, "cliffs") == 0) {
				Settings.Cliffs.Value = std::clamp(value, Settings.Cliffs.Minimum, Settings.Cliffs.Maximum);
			}
		}
		virtual void Preview(void) override { Calls.push_back("preview"); }
		virtual void Surprise(void) override { Calls.push_back("surprise"); }
		virtual void Save(void) override { Calls.push_back("save"); }
		virtual void Load(void) override { Calls.push_back("load"); }
		virtual void Delete(void) override { Calls.push_back("delete"); }
};


TEST_CASE("the map generator presenter opens on the generator settings and keeps them in bounds")
{
	RecordingMapGenServiceClass service;
	UIMapGenPresenterClass presenter(service);
	CHECK_MESSAGE(presenter.State.Cliffs.Value == 50, "the generator screen opens on the generator's settings");

	Drive(presenter, "cliffs", 70);
	CHECK_MESSAGE(presenter.State.Cliffs.Value == 70, "a slider move reaches the generator and the model follows it at once");
	REQUIRE_MESSAGE(service.Calls.size() == 1, "a slider move reaches the generator and the model follows it at once");
	CHECK_MESSAGE(service.Calls[0] == "cliffs 70", "a slider move reaches the generator and the model follows it at once");

	Drive(presenter, "cliffs", 100);
	CHECK_MESSAGE(presenter.State.Cliffs.Value == 80, "a reading past the generator's bound comes back as the bound");

	std::vector<UIMapGenOption> biomes = {
		{ "Tundra", 0 }, { "Taiga", 1 }, { "Temperate", 2 }, { "Desert", 3 }, { "Mutated", 4 },
	};
	UI_Sort_Map_Gen_Options(biomes);
	CHECK_MESSAGE(biomes.front().Label == "Desert", "the environments read alphabetically, each keeping the generator's number");
	CHECK_MESSAGE(biomes.front().Value == 3, "the environments read alphabetically, each keeping the generator's number");
	CHECK_MESSAGE(biomes.back().Label == "Tundra", "the environments read alphabetically, each keeping the generator's number");
	CHECK_MESSAGE(biomes.back().Value == 0, "the environments read alphabetically, each keeping the generator's number");

	CHECK_MESSAGE(UI_Map_Gen_Option_Value(biomes, 2) == 2, "a setting the generator holds is kept through the sort");
	CHECK_MESSAGE(UI_Map_Gen_Option_Value(biomes, 9) == 3, "and one it does not falls back to the first option shown");
}


TEST_CASE("the reconnect presenter hands over each kick vote once, for a seat it shows")
{
	UIReconnectPresenterClass presenter;
	presenter.State.Players.push_back({ "Host", 1.0f, "" });
	presenter.State.Players.push_back({ "Guest", 0.5f, "" });
	CHECK_MESSAGE(!presenter.Cancelled, "the notice opens with nothing asked for");
	CHECK_MESSAGE(presenter.Take_Kick_Vote() == -1, "the notice opens with nothing asked for");

	Drive(presenter, "kick", 1);
	CHECK_MESSAGE(presenter.Take_Kick_Vote() == 1, "a kick vote is handed over once");
	CHECK_MESSAGE(presenter.Take_Kick_Vote() == -1, "a kick vote is handed over once");

	Drive(presenter, "kick", 5);
	CHECK_MESSAGE(presenter.Take_Kick_Vote() == -1, "a kick must name a seat the notice shows");

	Drive(presenter, "cancel");
	CHECK_MESSAGE(presenter.Cancelled, "cancel asks to leave the game");
}


class RecordingDiplomacyServiceClass : public UIDiplomacyServiceClass
{
	public:
		virtual void Toggle_Alliance(int house) override { Calls.push_back("ally " + std::to_string(house)); }
		virtual void Toggle_Control(int house) override { Calls.push_back("give " + std::to_string(house)); }
		virtual void Set_Sends_To(int house, bool sends) override { Calls.push_back("send " + std::to_string(house) + (sends ? " on" : " off")); }
		virtual void Set_Muted(int house, bool muted) override { Calls.push_back("mute " + std::to_string(house) + (muted ? " on" : " off")); }
		virtual bool Chat_Edit_Open(void) override { return(Editing); }

		std::vector<std::string> Calls;
		bool Editing = false;
};


UIDiplomacyRow Diplomacy_Player(int house, std::string const & name)
{
	UIDiplomacyRow row;
	row.House = house;
	row.Name = name;
	row.Color = "#fcd400";
	row.Kills = "1234";
	row.HasAlly = true;
	row.AllyEnabled = true;
	row.Theirs = "Yes";
	row.TheirsYes = true;
	row.HasGive = true;
	row.GiveEnabled = true;
	row.Given = "No";
	row.HasChat = true;
	row.Send = true;
	row.Latency = "1500 ms";
	row.LatencyPoor = true;
	row.Waiting = true;
	row.WaitingTip = "Waited on them 1.8 s in the last 30 s";
	return(row);
}


UIDiplomacyRow Diplomacy_Observer(int house, std::string const & name)
{
	UIDiplomacyRow row;
	row.House = house;
	row.Name = name;
	row.Color = "#848a84";
	row.HasChat = true;
	row.Send = true;
	row.Latency = "1500 ms";
	return(row);
}


// The longest names a seat can hold, so the layout checks see the widest text.
UIDiplomacyState Diplomacy_State(int players, int observers)
{
	UIDiplomacyState state;
	state.ShowControl = true;
	state.ShowChat = true;
	state.Info = "Tiberium Garden Redux of the Long Name - 1:23:45 - Connection: Normal";
	state.Footer = "Waiting most on WWWWWWWWWWWWWWWW: 12.3 s in the last 30 s";
	for (int index = 0; index < players; index++) {
		state.Players.push_back(Diplomacy_Player(index, "WWWWWWWWWWWWWWWW"));
	}
	for (int index = 0; index < observers; index++) {
		state.Observers.push_back(Diplomacy_Observer(players + index, "WWWWWWWWWWWWWWWW"));
	}
	return(state);
}


TEST_CASE("the diplomacy presenter reports each action once and puts back a disabled box")
{
	RecordingDiplomacyServiceClass service;
	UIDiplomacyPresenterClass presenter(service);
	presenter.State = Diplomacy_State(3, 1);
	presenter.State.Players[1].AllyEnabled = false;
	presenter.State.Players[2].Mute = true;

	Drive(presenter, "ally", 0);
	Drive(presenter, "give", 0);
	Drive(presenter, "send", 0);
	Drive(presenter, "mute", 2);
	Drive(presenter, "send", 3);
	CHECK_MESSAGE((service.Calls == std::vector<std::string>{ "ally 0", "give 0", "send 0 off", "mute 2 off", "send 3 off" }), "each box asks the game for the change its row allows, flipping the state the row shows");
	CHECK_MESSAGE(presenter.Take_Acted(), "an action is reported once");
	CHECK_MESSAGE(!presenter.Take_Acted(), "an action is reported once");

	service.Calls.clear();
	Drive(presenter, "ally", 1);
	CHECK_MESSAGE(service.Calls.empty(), "a click on a disabled box asks for nothing but still counts, so the box is put back");
	CHECK_MESSAGE(presenter.Take_Acted(), "a click on a disabled box asks for nothing but still counts, so the box is put back");

	Drive(presenter, "ally", 99);
	CHECK_MESSAGE(service.Calls.empty(), "a click naming no row is ignored");
	CHECK_MESSAGE(!presenter.Take_Acted(), "a click naming no row is ignored");

	CHECK_MESSAGE(!presenter.Keys_Belong_To_Game(), "keys stay with the screen while no chat line is typed");
	service.Editing = true;
	CHECK_MESSAGE(presenter.Keys_Belong_To_Game(), "and go to the game while one is");

	CHECK_MESSAGE(!presenter.Closed, "the screen opens unclosed");
	Drive(presenter, "close");
	CHECK_MESSAGE(presenter.Closed, "close asks to take the screen down");
}


class RecordingRestateServiceClass : public UIRestateServiceClass
{
	public:
		int Bleeps = 0;

		virtual void Bleep(void) override { Bleeps++; }
};


int Ten_Per_Character(std::string const & text)
{
	return((int)text.size() * 10);
}


std::vector<int> Restate_Frames(UIRestatePresenterClass const & presenter)
{
	std::vector<int> frames;
	for (int line = 0; line < (int)presenter.Lines().size(); line++) {
		frames.push_back(presenter.Frame_Of(line));
	}
	return(frames);
}


std::string Restate_Lines(int count)
{
	std::string text;
	for (int line = 0; line < count; line++) {
		text += (line == 0 ? "" : "@") + std::string("Line ") + std::to_string(line + 1);
	}
	return(text);
}


TEST_CASE("the briefing breaks at @ and newlines, drops the spaces after a break, and wraps whole words")
{
	RecordingRestateServiceClass service;
	FakeClockClass clock;
	UIRestatePresenterClass presenter(service, clock, "Alpha beta gamma@   Delta@@Epsilon\nZeta Supercalifragilistic@", false);
	presenter.Lay_Out(Ten_Per_Character, 120, 14);
	std::vector<std::string> const expected = { "Alpha beta", "gamma", "Delta", "", "Epsilon", "Zeta", "Supercalifr", "agilistic" };
	REQUIRE_MESSAGE(presenter.Pages.size() == 1, "the briefing breaks at @ and newlines, drops the spaces after a break, and wraps whole words");
	CHECK_MESSAGE(presenter.Pages[0] == expected, "the briefing breaks at @ and newlines, drops the spaces after a break, and wraps whole words");

	UIRestatePresenterClass edge(service, clock, "abcd efghi jk", false);
	edge.Lay_Out(Ten_Per_Character, 100, 14);
	REQUIRE_MESSAGE(edge.Pages.size() == 1, "a word stays on its line when only its last letter crosses the width");
	CHECK_MESSAGE(edge.Pages[0] == std::vector<std::string>({ "abcd efghi", "jk" }), "a word stays on its line when only its last letter crosses the width");
}


TEST_CASE("a long briefing is cut into pages of the lines the box holds")
{
	RecordingRestateServiceClass service;
	FakeClockClass clock;
	UIRestatePresenterClass presenter(service, clock, Restate_Lines(30), false);
	presenter.Lay_Out(Ten_Per_Character, 400, 14);
	REQUIRE_MESSAGE(presenter.Pages.size() == 3, "a long briefing is cut into pages of the lines the box holds");
	REQUIRE_MESSAGE(presenter.Pages[0].size() == 14, "a long briefing is cut into pages of the lines the box holds");
	REQUIRE_MESSAGE(presenter.Pages[1].size() == 14, "a long briefing is cut into pages of the lines the box holds");
	CHECK_MESSAGE(presenter.Pages[2].size() == 2, "a long briefing is cut into pages of the lines the box holds");
}


TEST_CASE("the briefing screen ignores keys until its text is laid out")
{
	RecordingRestateServiceClass service;
	FakeClockClass clock;
	UIRestatePresenterClass presenter(service, clock, "One@Two", false);
	Drive(presenter, "next");
	REQUIRE_MESSAGE(!presenter.Result.has_value(), "the screen ignores keys until its text is laid out");
	CHECK_MESSAGE(!presenter.Typing, "the screen ignores keys until its text is laid out");

	presenter.Lay_Out(Ten_Per_Character, 400, 14);
	presenter.Refresh();
	clock.Now = 143;
	presenter.Refresh();
	CHECK_MESSAGE(presenter.Typing, "no line shows before the page's opening delay");
	CHECK_MESSAGE(Restate_Frames(presenter) == std::vector<int>({ -1, -1 }), "no line shows before the page's opening delay");

	std::vector<std::vector<int>> const stages = { { 0, -1 }, { 1, -1 }, { 2, -1 }, { 2, 0 }, { 2, 1 }, { 2, 2 } };
	std::vector<int> const bleeps = { 0, 0, 1, 1, 1, 1 };
	bool timed = true;
	for (std::size_t step = 0; step < stages.size(); step++) {
		clock.Now = 144 + 64 * (int)step;
		presenter.Refresh();
		timed = timed && Restate_Frames(presenter) == stages[step] && service.Bleeps == bleeps[step] && presenter.Typing;
	}
	CHECK_MESSAGE(timed, "each line fades in over three steps of 64 ms and sounds as it settles");

	clock.Now = 144 + 64 * 6;
	presenter.Refresh();
	CHECK_MESSAGE(!presenter.Typing, "the last line sounds one step later, as the page ends and the buttons come up");
	CHECK_MESSAGE(presenter.Done, "the last line sounds one step later, as the page ends and the buttons come up");
	CHECK_MESSAGE(!presenter.More, "the last line sounds one step later, as the page ends and the buttons come up");
	CHECK_MESSAGE(service.Bleeps == 2, "the last line sounds one step later, as the page ends and the buttons come up");
}


TEST_CASE("a late pass takes one step, not the steps it missed")
{
	RecordingRestateServiceClass service;
	FakeClockClass clock;
	UIRestatePresenterClass presenter(service, clock, "One@Two", false);
	presenter.Lay_Out(Ten_Per_Character, 400, 14);
	presenter.Refresh();
	clock.Now = 5000;
	presenter.Refresh();
	presenter.Refresh();
	CHECK_MESSAGE(Restate_Frames(presenter) == std::vector<int>({ 0, -1 }), "a late pass takes one step, not the steps it missed");
}


TEST_CASE("a click during the fade settles the page with one bleep and offers More")
{
	RecordingRestateServiceClass service;
	FakeClockClass clock;
	UIRestatePresenterClass presenter(service, clock, Restate_Lines(30), false);
	presenter.Lay_Out(Ten_Per_Character, 400, 14);
	presenter.Refresh();
	clock.Now = 144;
	presenter.Refresh();

	Drive(presenter, "skip");
	CHECK_MESSAGE(!presenter.Typing, "a click during the fade settles the page with one bleep and offers More");
	CHECK_MESSAGE(presenter.More, "a click during the fade settles the page with one bleep and offers More");
	CHECK_MESSAGE(service.Bleeps == 1, "a click during the fade settles the page with one bleep and offers More");
	CHECK_MESSAGE(Restate_Frames(presenter) == std::vector<int>(14, 2), "a click during the fade settles the page with one bleep and offers More");

	Drive(presenter, "resume");
	CHECK_MESSAGE(presenter.More, "resume does nothing while More is showing");
	CHECK_MESSAGE(!presenter.Result.has_value(), "resume does nothing while More is showing");

	Drive(presenter, "next");
	CHECK_MESSAGE(presenter.Page == 1, "Space shows the next page, which fades in afresh");
	CHECK_MESSAGE(presenter.Typing, "Space shows the next page, which fades in afresh");
	CHECK_MESSAGE(Restate_Frames(presenter) == std::vector<int>(14, -1), "Space shows the next page, which fades in afresh");

	presenter.Refresh();
	clock.Now = 144 + 143;
	presenter.Refresh();
	CHECK_MESSAGE(Restate_Frames(presenter)[0] == -1, "and waits its own opening delay");

	Drive(presenter, "ok");
	Drive(presenter, "more");
	CHECK_MESSAGE(presenter.Page == 2, "Enter settles a page and More moves on");
	CHECK_MESSAGE(presenter.Typing, "Enter settles a page and More moves on");

	Drive(presenter, "cancel");
	CHECK_MESSAGE(presenter.Done, "Escape settles the last page");
	CHECK_MESSAGE(!presenter.More, "Escape settles the last page");

	Drive(presenter, "more");
	Drive(presenter, "video");
	CHECK_MESSAGE(!presenter.Result.has_value(), "neither More nor a video the mission lacks answers at the end");

	Drive(presenter, "next");
	REQUIRE_MESSAGE(presenter.Result.has_value(), "Space at the end resumes the mission");
	CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "Space at the end resumes the mission");
	CHECK_MESSAGE(!presenter.ChoseVideo, "Space at the end resumes the mission");
}


TEST_CASE("Video at the end asks for the briefing movie")
{
	RecordingRestateServiceClass service;
	FakeClockClass clock;
	UIRestatePresenterClass presenter(service, clock, "One", true);
	presenter.Lay_Out(Ten_Per_Character, 400, 14);
	Drive(presenter, "next");
	Drive(presenter, "video");
	REQUIRE_MESSAGE(presenter.Result.has_value(), "Video at the end asks for the briefing movie");
	CHECK_MESSAGE(presenter.ChoseVideo, "Video at the end asks for the briefing movie");
}


TEST_CASE("an empty briefing goes straight to the buttons, silently")
{
	RecordingRestateServiceClass service;
	FakeClockClass clock;
	UIRestatePresenterClass presenter(service, clock, "", false);
	presenter.Lay_Out(Ten_Per_Character, 400, 14);
	CHECK_MESSAGE(presenter.Done, "an empty briefing goes straight to the buttons, silently");
	CHECK_MESSAGE(!presenter.Typing, "an empty briefing goes straight to the buttons, silently");
	CHECK_MESSAGE(presenter.Pages.empty(), "an empty briefing goes straight to the buttons, silently");
	CHECK_MESSAGE(service.Bleeps == 0, "an empty briefing goes straight to the buttons, silently");

	Drive(presenter, "resume");
	REQUIRE_MESSAGE(presenter.Result.has_value(), "and Resume closes it");
	CHECK_MESSAGE(!presenter.ChoseVideo, "and Resume closes it");
}


TEST_CASE("the string table carries every identifier the language header names")
{
	int entries = (int)(sizeof(OpenTSStringNames) / sizeof(OpenTSStringNames[0]));
	CHECK_MESSAGE(OpenTSStringNameCount == entries, "the string table's count matches its entries");
	CHECK_MESSAGE(OpenTSStringNameCount > 700, "the string table carries the language header's identifiers");

	int ok = -1;
	for (OpenTSStringName const & entry : OpenTSStringNames) {
		if (std::strcmp(entry.Name, "TXT_OK") == 0) {
			ok = entry.Id;
		}
	}
	CHECK_MESSAGE(ok == 10, "TXT_OK maps to its identifier");

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

	CHECK_MESSAGE(resolved, "every string a document names exists in the table");
	std::printf("  %d string references in the shipped documents\n", references);
}


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


void Test_Reconnect_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;
	UIReconnectPresenterClass presenter;
	presenter.State.TimeRemaining = "Time remaining: 60 seconds";
	presenter.State.Messages.push_back("Waiting for other players.");
	std::unique_ptr<UIViewClass> view = UI_Reconnect_View(presenter);

	CHECK_MESSAGE(Rml(*view).Prepare(context), "the reconnect view prepares against the test context");
	view->Show(true);
	view->Sync();
	context.Update();
	context.Render();
	CHECK_MESSAGE(system.Problems == problems, "the reconnect screen raises no RmlUi warning or error");

	Rml::ElementDocument * document = Rml(*view).Document();
	Rml::Element * remaining = document->GetElementById("timeremaining");
	REQUIRE_MESSAGE(remaining != nullptr, "reconnect time remaining uses the dialog bitmap font family");
	CHECK_MESSAGE(remaining->GetComputedValues().font_family() == "dlgsys", "reconnect time remaining uses the dialog bitmap font family");
	Rml::Element * message = document->QuerySelector("#messages .item");
	REQUIRE_MESSAGE(message != nullptr, "the reconnect message list keeps its sans font family");
	CHECK_MESSAGE(message->GetComputedValues().font_family() == "dlg-sans", "the reconnect message list keeps its sans font family");

	view->Release();
	context.Update();
}


// Every cell except the player name, which clips by design, must show its whole text.
bool Diplomacy_Cells_Fit(Rml::ElementDocument * document)
{
	Rml::ElementList cells;
	document->QuerySelectorAll(cells, ".row > span");
	for (Rml::Element * cell : cells) {
		if (cell->IsClassSet("c-name") || cell->GetComputedValues().display() == Rml::Style::Display::None) {
			continue;
		}
		if (cell->GetScrollWidth() > cell->GetClientWidth() + 0.5f) {
			std::printf("diplomacy cell overflows: '%s' %.1f > %.1f\n", cell->GetInnerRML().c_str(), cell->GetScrollWidth(), cell->GetClientWidth());
			return(false);
		}
	}
	for (char const * id : { "title", "info", "footer" }) {
		Rml::Element * element = document->GetElementById(id);
		if (element == nullptr || element->GetScrollWidth() > element->GetClientWidth() + 0.5f) {
			std::printf("diplomacy %s overflows\n", id);
			return(false);
		}
	}
	return(true);
}


float Bottom_Of(Rml::Element * element)
{
	return(element->GetAbsoluteOffset(Rml::BoxArea::Border).y + element->GetBox().GetSize(Rml::BoxArea::Border).y);
}


void Test_Diplomacy_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	struct Split {
		int Players;
		int Observers;
	};

	for (Split const split : { Split{ 8, 0 }, Split{ 1, 7 }, Split{ 7, 1 } }) {
		int problems = system.Problems;
		RecordingDiplomacyServiceClass service;
		UIDiplomacyPresenterClass presenter(service);
		presenter.State = Diplomacy_State(split.Players, split.Observers);
		presenter.State.Info = "Tiberium Garden - 1:23:45 - Connection: Normal";
		presenter.State.Footer = "Waiting most on Commander Kane 1: 12.3 s in the last 30 s";
		std::unique_ptr<UIViewClass> view = UI_Diplomacy_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the diplomacy view prepares against the test context");
		view->Show(false);
		view->Sync();
		context.Update();
		context.Render();
		CHECK_MESSAGE(system.Problems == problems, "the diplomacy screen raises no RmlUi warning or error");

		Rml::ElementDocument * document = Rml(*view).Document();
		Rml::Element * groups = document->GetElementById("groups");
		Rml::Element * footer = document->GetElementById("footer");
		Rml::Element * close = document->GetElementById("close");
		Rml::Element * reveal = document->GetElementById("reveal");
		REQUIRE_MESSAGE(groups != nullptr, "the diplomacy screen has its groups, footer and button");
		REQUIRE_MESSAGE(footer != nullptr, "the diplomacy screen has its groups, footer and button");
		REQUIRE_MESSAGE(close != nullptr, "the diplomacy screen has its groups, footer and button");
		CHECK_MESSAGE(reveal != nullptr, "the diplomacy screen has its groups, footer and button");
		if (groups == nullptr || footer == nullptr || close == nullptr || reveal == nullptr) {
			view->Release();
			continue;
		}

		Rml::ElementList rows;
		document->QuerySelectorAll(rows, ".row");
		int const shown = (int)std::count_if(rows.begin(), rows.end(), [](Rml::Element * row) { return(row->IsVisible(true)); });
		CHECK_MESSAGE(shown == 2 + split.Players + split.Observers, "the diplomacy screen draws a row for every seat");

		float const footertop = footer->GetAbsoluteOffset(Rml::BoxArea::Border).y;
		CHECK_MESSAGE(Bottom_Of(groups) <= footertop - 8.0f, "every seat fits above the footer with room to spare");
		CHECK_MESSAGE(Bottom_Of(close) <= Bottom_Of(reveal) - 8.0f, "the close button sits inside the frame");
		CHECK_MESSAGE(Diplomacy_Cells_Fit(document), "every column shows its whole heading and value");

		Rml::Element * head = document->QuerySelector("#players .row.head");
		Rml::Element * name = document->QuerySelector("#players .row .c-name");
		REQUIRE_MESSAGE(head != nullptr, "the rows span the 556dp inside the group");
		CHECK_MESSAGE(std::fabs(head->GetClientWidth() - 556.0f) < 0.5f, "the rows span the 556dp inside the group");
		REQUIRE_MESSAGE(name != nullptr, "with every column shown, the name still gets about 156dp");
		CHECK_MESSAGE(name->GetClientWidth() >= 150.0f, "with every column shown, the name still gets about 156dp");

		view->Release();
		context.Update();
	}

	RecordingDiplomacyServiceClass service;
	UIDiplomacyPresenterClass presenter(service);
	presenter.State = Diplomacy_State(4, 0);
	presenter.State.ShowControl = false;
	presenter.State.ShowChat = false;
	std::unique_ptr<UIViewClass> view = UI_Diplomacy_View(presenter);
	CHECK_MESSAGE(Rml(*view).Prepare(context), "the diplomacy view prepares without the control and chat columns");
	view->Show(false);
	view->Sync();
	context.Update();
	Rml::Element * name = Rml(*view).Document()->QuerySelector("#players .row .c-name");
	REQUIRE_MESSAGE(name != nullptr, "hidden columns give their width to the name");
	CHECK_MESSAGE(name->GetClientWidth() >= 340.0f, "hidden columns give their width to the name");
	view->Release();
	context.Update();
}


void Test_Version_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	{
		UIVersionPresenterClass presenter({ "Line 1", "Line 2" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the version view prepares against the test context");
		view->Show(true);
		context.Update();
		context.Render();
		CHECK_MESSAGE(system.Problems == problems, "the version screen raises no RmlUi warning or error");
		CHECK_MESSAGE(view->Is_Shown(), "the version screen is shown");

		Rml::ElementDocument * document = Rml(*view).Document();
		Rml::Element * lines = (document != nullptr) ? document->GetElementById("lines") : nullptr;

		Rml::Element * rows = (lines != nullptr && lines->GetNumChildren() > 0) ? lines->GetChild(0) : nullptr;
		int visible = 0;
		for (int index = 0; rows != nullptr && index < rows->GetNumChildren(); index++) {
			if (rows->GetChild(index)->IsVisible()) {
				visible++;
			}
		}
		REQUIRE_MESSAGE(rows != nullptr, "the version screen lists one paragraph per line");
		CHECK_MESSAGE(visible == 2, "the version screen lists one paragraph per line");

		Rml::Element * ok = (document != nullptr) ? document->GetElementById("ok") : nullptr;
		CHECK_MESSAGE(ok != nullptr, "the version screen has its OK button");

		if (ok != nullptr) {
			Rml::Vector2f at = Center_Of(ok);
			context.ProcessMouseMove((int)at.x, (int)at.y, 0);
			context.Update();
			CHECK_MESSAGE(!context.ProcessMouseButtonDown(0, 0), "a press on OK interacts with the document");
			context.ProcessMouseButtonUp(0, 0);
			REQUIRE_MESSAGE(!presenter.Result.has_value(), "the click queues an intent and does not act");
			CHECK_MESSAGE(presenter.Has_Pending(), "the click queues an intent and does not act");
			context.Update();
			presenter.Drain();
			REQUIRE_MESSAGE(presenter.Result.has_value(), "draining the click accepts the screen");
			CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "draining the click accepts the screen");
		}

		UIIntent late;
		late.Name = "cancel";
		presenter.Queue(late);
		presenter.Drain();
		REQUIRE_MESSAGE(presenter.Result.has_value(), "an intent after the result is dropped");
		CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "an intent after the result is dropped");
		CHECK_MESSAGE(!presenter.Has_Pending(), "an intent after the result is dropped");

		view->Release();
		context.Update();
	}

	for (int pass = 0; pass < 2; pass++) {
		bool escape = (pass == 0);
		UIVersionPresenterClass presenter({ "Line" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), (escape ? "the version view prepares for the Escape pass" : "the version view prepares for the Enter pass"));
		view->Show(true);
		context.Update();
		context.ProcessKeyDown(escape ? Rml::Input::KI_ESCAPE : Rml::Input::KI_RETURN, 0);
		context.ProcessKeyUp(escape ? Rml::Input::KI_ESCAPE : Rml::Input::KI_RETURN, 0);
		context.Update();
		presenter.Drain();

		UIResult expected = escape ? UI_RESULT_CANCELLED : UI_RESULT_ACCEPTED;
		REQUIRE_MESSAGE(presenter.Result.has_value(), (escape ? "Escape cancels the version screen" : "Enter accepts the version screen"));
		CHECK_MESSAGE(*presenter.Result == expected, (escape ? "Escape cancels the version screen" : "Enter accepts the version screen"));

		view->Release();
		context.Update();
	}

	{
		UIVersionPresenterClass presenter({});
		UIIntent intent;
		intent.Name = "ok";
		presenter.Queue(intent);
		CHECK_MESSAGE(presenter.Has_Pending(), "a queued intent is pending until drained");
		presenter.Discard();
		CHECK_MESSAGE(!presenter.Has_Pending(), "discarding drops queued intents without a result");
		CHECK_MESSAGE(!presenter.Result.has_value(), "discarding drops queued intents without a result");
	}

	{
		UIVersionPresenterClass presenter({});
		MissingViewClass view(presenter);
		int before = system.Problems;

		CHECK_MESSAGE(!view.Prepare(context), "a missing document fails preparation");
		CHECK_MESSAGE(system.Problems > before, "a failed preparation is reported");
		CHECK_MESSAGE(!context.GetDataModel("missing"), "a failed preparation leaves no data model behind");
		context.Update();
	}
}


std::vector<Rml::Element *> Visible_Buttons(Rml::ElementDocument * document)
{
	std::vector<Rml::Element *> buttons;
	if (document == nullptr) {
		return(buttons);
	}

	Rml::ElementList all;
	document->GetElementsByTagName(all, "button");
	for (Rml::Element * element : all) {
		if (element->IsVisible() && element->GetComputedValues().display() != Rml::Style::Display::None) {
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
	Rml::Vector2f at = Center_Of(element);
	context.ProcessMouseMove((int)at.x, (int)at.y, 0);
	context.Update();
	context.ProcessMouseButtonDown(0, 0);
	context.ProcessMouseButtonUp(0, 0);
	context.Update();
}


void Test_Message_Box_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	{
		UIMessageBoxPresenterClass presenter("Would you like to replay this mission?", { "First", "Second", "Third" }, 0);
		CHECK_MESSAGE(presenter.Button_Count() == 3, "three captions make three buttons");

		std::unique_ptr<UIViewClass> view = UI_Message_Box_View(presenter);
		CHECK_MESSAGE(Rml(*view).Prepare(context), "the message box view prepares against the test context");
		view->Show(true);
		context.Update();
		context.Render();
		CHECK_MESSAGE(system.Problems == problems, "the message box raises no RmlUi warning or error");

		Rml::Element * prompt = Rml(*view).Document()->QuerySelector("#text p");
		REQUIRE_MESSAGE(prompt != nullptr, "the replay question uses the dialog bitmap font family");
		CHECK_MESSAGE(prompt->GetComputedValues().font_family() == "dlgsys", "the replay question uses the dialog bitmap font family");

		std::vector<Rml::Element *> buttons = Visible_Buttons(Rml(*view).Document());
		CHECK_MESSAGE(buttons.size() == 3, "three buttons are visible");
		CHECK_MESSAGE(std::all_of(buttons.begin(), buttons.end(), [](Rml::Element * button) { return(button->GetComputedValues().font_family() == "dlgsys"); }), "message button captions use the dialog bitmap font family");
		bool ordered = buttons.size() == 3 && buttons[0]->GetInnerRML() == "First" && buttons[1]->GetInnerRML() == "Third" && buttons[2]->GetInnerRML() == "Second";
		CHECK_MESSAGE(ordered, "the buttons read first, third, second from left to right");

		if (buttons.size() == 3) {
			Click(context, buttons[1]);
			presenter.Drain();
			REQUIRE_MESSAGE(presenter.Result.has_value(), "a click on the middle button answers with the third button");
			CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "a click on the middle button answers with the third button");
			CHECK_MESSAGE(presenter.Choice == 2, "a click on the middle button answers with the third button");
		}

		view->Release();
		context.Update();
	}

	{
		UIMessageBoxPresenterClass presenter("Two buttons", { "OK", "Cancel", "" }, 0);
		std::unique_ptr<UIViewClass> view = UI_Message_Box_View(presenter);
		CHECK_MESSAGE(Rml(*view).Prepare(context), "a two-button box prepares");
		view->Show(true);
		context.Update();

		std::vector<Rml::Element *> buttons = Visible_Buttons(Rml(*view).Document());
		CHECK_MESSAGE(buttons.size() == 2, "two buttons are visible");
		if (buttons.size() == 2) {
			float panel = Rml(*view).Document()->GetElementById("chrome")->GetAbsoluteOffset(Rml::BoxArea::Border).x;
			float left = buttons[0]->GetAbsoluteOffset(Rml::BoxArea::Border).x - panel;
			float right = buttons[1]->GetAbsoluteOffset(Rml::BoxArea::Border).x - panel;
			CHECK_MESSAGE(left < 60.0f, "two buttons take the outer slots");
			CHECK_MESSAGE(right > 250.0f, "two buttons take the outer slots");
		}

		context.ProcessKeyDown(Rml::Input::KI_ESCAPE, 0);
		context.ProcessKeyUp(Rml::Input::KI_ESCAPE, 0);
		context.Update();
		presenter.Drain();
		REQUIRE_MESSAGE(presenter.Result.has_value(), "Escape answers with the second button");
		CHECK_MESSAGE(*presenter.Result == UI_RESULT_CANCELLED, "Escape answers with the second button");
		CHECK_MESSAGE(presenter.Choice == 1, "Escape answers with the second button");

		view->Release();
		context.Update();
	}

	{
		UIMessageBoxPresenterClass presenter("One button", { "OK", "", "" }, 0);
		std::unique_ptr<UIViewClass> view = UI_Message_Box_View(presenter);
		CHECK_MESSAGE(Rml(*view).Prepare(context), "a one-button box prepares");
		view->Show(true);
		context.Update();

		std::vector<Rml::Element *> buttons = Visible_Buttons(Rml(*view).Document());
		CHECK_MESSAGE(buttons.size() == 1, "one button is visible");
		if (buttons.size() == 1) {
			float panel = Rml(*view).Document()->GetElementById("chrome")->GetAbsoluteOffset(Rml::BoxArea::Border).x;
			float left = buttons[0]->GetAbsoluteOffset(Rml::BoxArea::Border).x - panel;
			CHECK_MESSAGE(left > 100.0f, "a lone button takes the middle slot");
			CHECK_MESSAGE(left < 200.0f, "a lone button takes the middle slot");
		}

		view->Release();
		context.Update();
	}

	{
		UIMessageBoxPresenterClass presenter("Network", { "OK", "", "" }, 0);
		presenter.Network = true;
		std::unique_ptr<UIViewClass> view = UI_Message_Box_View(presenter);
		CHECK_MESSAGE(Rml(*view).Prepare(context), "the network box prepares");
		view->Show(true);
		context.Update();

		Rml::Element * dialog = Rml(*view).Document()->GetElementById("reveal");
		REQUIRE_MESSAGE(dialog != nullptr, "the network box is the size its template comes to");
		CHECK_MESSAGE(dialog->GetBox().GetSize(Rml::BoxArea::Border) == Rml::Vector2f(436.0f, 147.0f), "the network box is the size its template comes to");

		Rml::Element * prompt = Rml(*view).Document()->QuerySelector("#text p");
		REQUIRE_MESSAGE(prompt != nullptr, "network messages use the dialog bitmap font family");
		CHECK_MESSAGE(prompt->GetComputedValues().font_family() == "dlgsys", "network messages use the dialog bitmap font family");

		std::vector<Rml::Element *> buttons = Visible_Buttons(Rml(*view).Document());
		CHECK_MESSAGE(buttons.size() == 1, "the network box shows its lone button");
		if (buttons.size() == 1) {
			CHECK_MESSAGE(buttons[0]->GetComputedValues().font_family() == "dlgsys", "the network button caption uses the dialog bitmap font family");
			float panel = Rml(*view).Document()->GetElementById("chrome")->GetAbsoluteOffset(Rml::BoxArea::Border).x;
			float left = buttons[0]->GetAbsoluteOffset(Rml::BoxArea::Border).x - panel;
			CHECK_MESSAGE(left == 180.0f, "a lone button stands where the template puts OK");
		}

		view->Release();
		context.Update();
	}

	{
		UIMessageBoxPresenterClass presenter("Default", { "Yes", "No", "Maybe" }, 2);
		std::unique_ptr<UIViewClass> view = UI_Message_Box_View(presenter);
		CHECK_MESSAGE(Rml(*view).Prepare(context), "a box with a default prepares");
		view->Show(true);
		context.Update();
		context.ProcessKeyDown(Rml::Input::KI_RETURN, 0);
		context.ProcessKeyUp(Rml::Input::KI_RETURN, 0);
		context.Update();
		presenter.Drain();
		REQUIRE_MESSAGE(presenter.Result.has_value(), "Enter answers with the default button");
		CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "Enter answers with the default button");
		CHECK_MESSAGE(presenter.Choice == 2, "Enter answers with the default button");

		view->Release();
		context.Update();
	}

	for (bool network : { false, true }) {
		UIMessageBoxPresenterClass presenter("Would you like to replay this mission from the beginning?\nChoose Yes to try again.", { "Yes", "No", "" }, 0);
		presenter.Network = network;
		std::unique_ptr<UIViewClass> view = UI_Message_Box_View(presenter);
		CHECK_MESSAGE(Rml(*view).Prepare(context), "a multiline message box prepares");
		view->Show(true);
		context.Update();
		context.Render();

		Rml::Element * text = Rml(*view).Document()->GetElementById("text");
		Rml::Element * prompt = Rml(*view).Document()->QuerySelector("#text p");
		REQUIRE_MESSAGE(prompt != nullptr, "multiline messages use the dialog bitmap font family");
		CHECK_MESSAGE(prompt->GetComputedValues().font_family() == "dlgsys", "multiline messages use the dialog bitmap font family");
		if (text != nullptr && prompt != nullptr) {
			Rml::Vector2f size = prompt->GetBox().GetSize(Rml::BoxArea::Border);
			Rml::Vector2f available = text->GetBox().GetSize(Rml::BoxArea::Content);
			CHECK_MESSAGE(size.y > 30.0f, "wrapped text and an explicit newline fit the message region");
			CHECK_MESSAGE(size.y <= available.y, "wrapped text and an explicit newline fit the message region");
			CHECK_MESSAGE(size.x <= available.x, "wrapped text and an explicit newline fit the message region");
		}
		CHECK_MESSAGE(system.Problems == problems, "multiline message boxes raise no RmlUi warning or error");

		view->Release();
		context.Update();
	}

	{
		UIMessageBoxPresenterClass presenter("Nothing", { "", "", "" }, 0);
		CHECK_MESSAGE(presenter.Button_Count() == 0, "empty captions make no buttons");
	}
}


std::vector<Rml::Element *> Visible_Of_Class(Rml::ElementDocument * document, char const * name)
{
	std::vector<Rml::Element *> found;
	if (document == nullptr) {
		return(found);
	}

	Rml::ElementList all;
	document->GetElementsByClassName(all, name);
	for (Rml::Element * element : all) {
		if (element->IsVisible() && element->GetComputedValues().display() != Rml::Style::Display::None) {
			found.push_back(element);
		}
	}
	return(found);
}


std::vector<Rml::Element *> Keyboard_Rows(Rml::ElementDocument * document);


std::vector<Rml::Element *> Visible_Rows(Rml::ElementDocument * document, char const * listid)
{
	std::vector<Rml::Element *> found;
	Rml::Element * list = document != nullptr ? document->GetElementById(listid) : nullptr;
	if (list == nullptr) {
		return(found);
	}

	Rml::Element * rows = list->GetNumChildren() > 0 && list->GetChild(0)->IsClassSet("rows") ? list->GetChild(0) : list;
	for (int index = 0; index < rows->GetNumChildren(); index++) {
		Rml::Element * row = rows->GetChild(index);
		if (row->IsClassSet("item") && row->IsVisible() && row->GetComputedValues().display() != Rml::Style::Display::None) {
			found.push_back(row);
		}
	}
	return(found);
}


std::vector<Rml::Element *> Keyboard_Rows(Rml::ElementDocument * document)
{
	std::vector<Rml::Element *> rows;
	Rml::ElementFormControlSelect * categories = document != nullptr ? rmlui_dynamic_cast<Rml::ElementFormControlSelect *>(document->GetElementById("categories")) : nullptr;
	if (categories != nullptr) {
		for (int index = 0; index < categories->GetNumOptions(); index++) {
			Rml::Element * option = categories->GetOption(index);
			if (!option->HasAttribute("data-for")) {
				rows.push_back(option);
			}
		}
	}

	std::vector<Rml::Element *> commands = Visible_Rows(document, "commands");
	rows.insert(rows.end(), commands.begin(), commands.end());
	return(rows);
}


void Pick_Category(Rml::ElementDocument * document, int index)
{
	Rml::ElementFormControlSelect * categories = document != nullptr ? rmlui_dynamic_cast<Rml::ElementFormControlSelect *>(document->GetElementById("categories")) : nullptr;
	if (categories != nullptr) {
		categories->SetSelection(index);
	}
}


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

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the sound view prepares against the test context");
		view->Show(true);
		context.Update();
		context.Render();
		CHECK_MESSAGE(system.Problems == problems, "the sound screen raises no RmlUi warning or error");

		presenter.Drain();
		CHECK_MESSAGE(presenter.State.Score == 7, "opening the sound screen plays no feedback");
		CHECK_MESSAGE(presenter.State.Sound == 5, "opening the sound screen plays no feedback");
		CHECK_MESSAGE(presenter.State.Voice == 10, "opening the sound screen plays no feedback");
		CHECK_MESSAGE(service.Calls.empty(), "opening the sound screen plays no feedback");

		Rml::ElementDocument * document = Rml(*view).Document();
		Rml::ElementList inputs;
		document->GetElementsByTagName(inputs, "input");
		int sliders = 0;
		for (Rml::Element * input : inputs) {
			if (input->GetAttribute<Rml::String>("type", "") == "range") {
				sliders++;
			}
		}
		CHECK_MESSAGE(sliders == 3, "the sound screen has three sliders");

		Rml::Element * score = document->GetElementById("score");
		REQUIRE_MESSAGE(score != nullptr, "the music slider starts at the music level");
		CHECK_MESSAGE(score->GetAttribute<int>("value", -1) == 7, "the music slider starts at the music level");

		std::vector<Rml::Element *> rows = Visible_Of_Class(document, "track");
		CHECK_MESSAGE(rows.size() == 3, "the track list shows one row per allowed track");
		REQUIRE_MESSAGE(rows.size() == 3, "the playing track's row is marked selected");
		CHECK_MESSAGE(rows[1]->IsClassSet("selected"), "the playing track's row is marked selected");
		CHECK_MESSAGE(!rows[0]->IsClassSet("selected"), "the playing track's row is marked selected");

		if (rows.size() == 3) {
			Click(context, rows[2]);
			presenter.Drain();
			view->Sync();
			context.Update();
			CHECK_MESSAGE(presenter.State.Selected == 2, "a click on a row selects it");
			CHECK_MESSAGE(rows[2]->IsClassSet("selected"), "a click on a row selects it");
			CHECK_MESSAGE(!rows[1]->IsClassSet("selected"), "a click on a row selects it");
		}

		Rml::Element * play = document->GetElementById("play");
		if (play != nullptr) {
			service.Calls.clear();
			Click(context, play);
			presenter.Drain();
			CHECK_MESSAGE(service.Joined() == "play 9", "the Play button plays the selected track");
		}

		Rml::Element * shuffle = document->GetElementById("shuffle");
		if (shuffle != nullptr) {
			service.Calls.clear();
			Click(context, shuffle);
			presenter.Drain();
			view->Sync();
			context.Update();
			CHECK_MESSAGE(presenter.State.Shuffle, "the shuffle switch turns shuffle on and shows it");
			CHECK_MESSAGE(service.Joined() == "shuffle on; repeat off", "the shuffle switch turns shuffle on and shows it");
			CHECK_MESSAGE(shuffle->HasAttribute("checked"), "the shuffle switch turns shuffle on and shows it");
		}

		if (score != nullptr) {
			service.Calls.clear();
			Rml::Dictionary parameters;
			parameters["value"] = Rml::Variant(3.0f);
			score->DispatchEvent(Rml::EventId::Change, parameters);
			presenter.Drain();
			view->Sync();
			context.Update();
			CHECK_MESSAGE(presenter.State.Score == 3, "a slider change previews the level and the slider follows the model");
			CHECK_MESSAGE(service.Joined() == "score 0.3 feedback", "a slider change previews the level and the slider follows the model");
			CHECK_MESSAGE(score->GetAttribute<int>("value", -1) == 3, "a slider change previews the level and the slider follows the model");
		}

		context.ProcessKeyDown(Rml::Input::KI_ESCAPE, 0);
		context.ProcessKeyUp(Rml::Input::KI_ESCAPE, 0);
		context.Update();
		presenter.Drain();
		REQUIRE_MESSAGE(presenter.Result.has_value(), "Escape closes the sound screen without reverting anything");
		CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "Escape closes the sound screen without reverting anything");

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

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the frontend sound view prepares");
		view->Show(true);
		context.Update();

		Rml::Element * music = Rml(*view).Document()->GetElementById("music");
		REQUIRE_MESSAGE(music != nullptr, "the frontend sound screen hides the music half");
		CHECK_MESSAGE(!music->IsVisible(), "the frontend sound screen hides the music half");
		CHECK_MESSAGE(Visible_Of_Class(Rml(*view).Document(), "track").empty(), "the frontend sound screen lists no tracks");

		view->Release();
		context.Update();
	}
}


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


void Test_Game_Controls_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	{
		RecordingGameControlsServiceClass service;
		UIGameControlsState state = Game_Controls_Fixture();
		state.InGame = true;
		state.HasDifficulty = false;
		state.SoundEnabled = true;

		UIGameControlsPresenterClass presenter(service, state);
		std::unique_ptr<UIViewClass> view = UI_Game_Controls_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the game controls view prepares against the test context");
		view->Show(true);
		context.Update();
		context.Render();
		CHECK_MESSAGE(system.Problems == problems, "the game controls screen raises no RmlUi warning or error");

		presenter.Drain();
		CHECK_MESSAGE(presenter.State.Speed == 4, "opening the game controls holds the starting settings and applies nothing");
		CHECK_MESSAGE(presenter.State.Scroll == 2, "opening the game controls holds the starting settings and applies nothing");
		CHECK_MESSAGE(presenter.State.Detail == 1, "opening the game controls holds the starting settings and applies nothing");
		CHECK_MESSAGE(service.Calls.empty(), "opening the game controls holds the starting settings and applies nothing");

		Rml::ElementDocument * document = Rml(*view).Document();
		CHECK_MESSAGE(Visible_Sliders(document) == 3, "the in-game screen has three sliders");

		Rml::Element * speed = document->GetElementById("speed");
		REQUIRE_MESSAGE(speed != nullptr, "the speed slider runs from slowest to fastest, so it starts at six minus the speed");
		CHECK_MESSAGE(speed->GetAttribute<int>("value", -1) == 2, "the speed slider runs from slowest to fastest, so it starts at six minus the speed");

		Rml::Element * speed_name = document->GetElementById("speed-name");
		REQUIRE_MESSAGE(speed_name != nullptr, "the speed's name shows beside its slider, read from the slider's end");
		CHECK_MESSAGE(speed_name->GetInnerRML() == "Slow", "the speed's name shows beside its slider, read from the slider's end");

		Rml::Element * detail_name = document->GetElementById("detail-name");
		REQUIRE_MESSAGE(detail_name != nullptr, "the detail level's name shows beside its slider");
		CHECK_MESSAGE(detail_name->GetInnerRML() == "Medium", "the detail level's name shows beside its slider");

		Rml::Element * sound = document->GetElementById("sound");
		Rml::Element * keyboard = document->GetElementById("keyboard");
		Rml::Element * options = document->GetElementById("ok-options");
		REQUIRE_MESSAGE(sound != nullptr, "the in-game screen has its Sound and Keyboard buttons");
		CHECK_MESSAGE(sound->IsVisible(), "the in-game screen has its Sound and Keyboard buttons");
		REQUIRE_MESSAGE(keyboard != nullptr, "the in-game screen has its Sound and Keyboard buttons");
		CHECK_MESSAGE(keyboard->IsVisible(), "the in-game screen has its Sound and Keyboard buttons");
		REQUIRE_MESSAGE(options != nullptr, "the in-game accept button reads Options Menu");
		CHECK_MESSAGE(options->IsVisible(true), "the in-game accept button reads Options Menu");

		if (speed != nullptr && speed_name != nullptr) {
			Rml::Dictionary parameters;
			parameters["value"] = Rml::Variant(5.0f);
			speed->DispatchEvent(Rml::EventId::Change, parameters);
			presenter.Drain();
			view->Sync();
			context.Update();
			CHECK_MESSAGE(presenter.State.Speed == 1, "dragging the speed slider changes the held speed and its name and applies nothing");
			CHECK_MESSAGE(speed_name->GetInnerRML() == "Faster", "dragging the speed slider changes the held speed and its name and applies nothing");
			CHECK_MESSAGE(service.Calls.empty(), "dragging the speed slider changes the held speed and its name and applies nothing");
		}

		Rml::Element * cameo = document->GetElementById("cameo");
		if (cameo != nullptr) {
			Click(context, cameo);
			presenter.Drain();
			view->Sync();
			context.Update();
			CHECK_MESSAGE(!presenter.State.CameoText, "a click on the cameo text switch turns it off and shows it");
			CHECK_MESSAGE(!cameo->HasAttribute("checked"), "a click on the cameo text switch turns it off and shows it");
		}

		if (sound != nullptr) {
			Click(context, sound);
			presenter.Drain();
			REQUIRE_MESSAGE(presenter.Result.has_value(), "the Sound button accepts the screen and names the sound options next");
			CHECK_MESSAGE(presenter.Next == UIGameControlsPresenterClass::NEXT_SOUND, "the Sound button accepts the screen and names the sound options next");
			CHECK_MESSAGE(service.Joined() == "speed 1; scroll 2; detail 1; cameo off; lines off; tooltips on; coasting off; edge off; save", "the Sound button applies the edited settings in order and saves");
		}

		view->Release();
		context.Update();
	}

	{
		RecordingGameControlsServiceClass service;
		UIGameControlsState state = Game_Controls_Fixture();
		state.InGame = false;
		state.HasDifficulty = true;

		UIGameControlsPresenterClass presenter(service, state);
		std::unique_ptr<UIViewClass> view = UI_Game_Controls_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the frontend game controls view prepares");
		view->Show(true);
		context.Update();

		Rml::ElementDocument * document = Rml(*view).Document();
		CHECK_MESSAGE(Visible_Sliders(document) == 4, "the frontend screen adds the difficulty slider");

		Rml::Element * difficulty_name = document->GetElementById("difficulty-name");
		REQUIRE_MESSAGE(difficulty_name != nullptr, "the difficulty's name shows beside its slider");
		CHECK_MESSAGE(difficulty_name->GetInnerRML() == "Hard", "the difficulty's name shows beside its slider");

		Rml::Element * sound = document->GetElementById("sound");
		Rml::Element * keyboard = document->GetElementById("keyboard");
		Rml::Element * main = document->GetElementById("ok-main");
		REQUIRE_MESSAGE(sound != nullptr, "the frontend screen has no Sound or Keyboard button");
		CHECK_MESSAGE(!sound->IsVisible(), "the frontend screen has no Sound or Keyboard button");
		REQUIRE_MESSAGE(keyboard != nullptr, "the frontend screen has no Sound or Keyboard button");
		CHECK_MESSAGE(!keyboard->IsVisible(), "the frontend screen has no Sound or Keyboard button");
		REQUIRE_MESSAGE(main != nullptr, "the frontend accept button reads Main Menu");
		CHECK_MESSAGE(main->IsVisible(true), "the frontend accept button reads Main Menu");

		Rml::Element * edge = document->GetElementById("edge");
		if (edge != nullptr) {
			Click(context, edge);
			presenter.Drain();
			view->Sync();
			context.Update();
			CHECK_MESSAGE(presenter.State.EdgeScroll, "a click on the edge scrolling switch turns it on and shows it");
			CHECK_MESSAGE(edge->HasAttribute("checked"), "a click on the edge scrolling switch turns it on and shows it");
		}

		context.ProcessKeyDown(Rml::Input::KI_ESCAPE, 0);
		context.ProcessKeyUp(Rml::Input::KI_ESCAPE, 0);
		context.Update();
		presenter.Drain();
		REQUIRE_MESSAGE(presenter.Result.has_value(), "Escape leaves the game controls with nothing applied");
		CHECK_MESSAGE(*presenter.Result == UI_RESULT_CANCELLED, "Escape leaves the game controls with nothing applied");
		CHECK_MESSAGE(service.Calls.empty(), "Escape leaves the game controls with nothing applied");

		view->Release();
		context.Update();
	}

	{
		RecordingGameControlsServiceClass service;
		UIGameControlsState state = Game_Controls_Fixture();
		state.InGame = true;
		state.HasDifficulty = false;
		state.SoundEnabled = false;

		UIGameControlsPresenterClass presenter(service, state);
		std::unique_ptr<UIViewClass> view = UI_Game_Controls_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the in-game controls view prepares");
		view->Show(true);
		context.Update();

		Rml::ElementDocument * document = Rml(*view).Document();

		Rml::Element * sound = document->GetElementById("sound");
		REQUIRE_MESSAGE(sound != nullptr, "the Sound button shows disabled without an audio device");
		CHECK_MESSAGE(sound->IsClassSet("disabled"), "the Sound button shows disabled without an audio device");

		if (sound != nullptr) {
			Click(context, sound);
			presenter.Drain();
			REQUIRE_MESSAGE(!presenter.Result.has_value(), "a disabled Sound button does nothing");
			CHECK_MESSAGE(service.Calls.empty(), "a disabled Sound button does nothing");
		}

		context.ProcessKeyDown(Rml::Input::KI_RETURN, 0);
		context.ProcessKeyUp(Rml::Input::KI_RETURN, 0);
		context.Update();
		presenter.Drain();
		REQUIRE_MESSAGE(presenter.Result.has_value(), "Enter accepts the in-game screen with a game speed and no difficulty");
		CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "Enter accepts the in-game screen with a game speed and no difficulty");
		CHECK_MESSAGE(service.Joined() == "speed 4; scroll 2; detail 1; cameo on; lines off; tooltips on; coasting off; edge off; save", "Enter accepts the in-game screen with a game speed and no difficulty");

		view->Release();
		context.Update();
	}
}


void Test_Display_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	{
		RecordingDisplayServiceClass service;
		UIDisplayPresenterClass presenter(service, Display_Fixture());
		std::unique_ptr<UIViewClass> view = UI_Display_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the display view prepares against the test context");
		view->Show(true);
		context.Update();
		context.Render();
		CHECK_MESSAGE(system.Problems == problems, "the display screen raises no RmlUi warning or error");

		Rml::ElementDocument * document = Rml(*view).Document();
		std::vector<Rml::Element *> rows = Visible_Rows(document, "modes");
		CHECK_MESSAGE(rows.size() == 3, "the display screen lists one row per mode");
		REQUIRE_MESSAGE(rows.size() == 3, "the row of the stored mode starts selected");
		CHECK_MESSAGE(rows[1]->IsClassSet("selected"), "the row of the stored mode starts selected");
		CHECK_MESSAGE(rows[1]->GetInnerRML() == "1280 x 800", "the row of the stored mode starts selected");

		if (rows.size() == 3) {
			Click(context, rows[2]);
			presenter.Drain();
			view->Sync();
			context.Update();
			CHECK_MESSAGE(presenter.State.Selected == 2, "a click on a row selects it");
			CHECK_MESSAGE(rows[2]->IsClassSet("selected"), "a click on a row selects it");
			CHECK_MESSAGE(!rows[1]->IsClassSet("selected"), "a click on a row selects it");
		}

		Rml::Element * stretch = document->GetElementById("stretch");
		if (stretch != nullptr) {
			Click(context, stretch);
			presenter.Drain();
			view->Sync();
			context.Update();
			CHECK_MESSAGE(presenter.State.StretchMovies, "the movie switch turns on and shows it without applying");
			CHECK_MESSAGE(stretch->HasAttribute("checked"), "the movie switch turns on and shows it without applying");
			CHECK_MESSAGE(service.Calls.empty(), "the movie switch turns on and shows it without applying");
		}

		Rml::Element * cursor = document->GetElementById("system-cursor");
		CHECK_MESSAGE(cursor != nullptr, "the display screen has the system pointer switch");
		if (cursor != nullptr) {
			Click(context, cursor);
			presenter.Drain();
			view->Sync();
			context.Update();
			CHECK_MESSAGE(presenter.State.SystemCursor, "the pointer switch turns on and shows it without applying");
			CHECK_MESSAGE(cursor->HasAttribute("checked"), "the pointer switch turns on and shows it without applying");
			CHECK_MESSAGE(service.Calls.empty(), "the pointer switch turns on and shows it without applying");
		}

		Rml::Element * ok = document->GetElementById("ok");
		Rml::Element * cancel = document->GetElementById("cancel");
		REQUIRE_MESSAGE(ok != nullptr, "OK sits left of Cancel");
		REQUIRE_MESSAGE(cancel != nullptr, "OK sits left of Cancel");
		CHECK_MESSAGE(ok->GetAbsoluteOffset(Rml::BoxArea::Border).x < cancel->GetAbsoluteOffset(Rml::BoxArea::Border).x, "OK sits left of Cancel");

		Rml::Element * reveal = document->GetElementById("reveal");
		if (ok != nullptr && reveal != nullptr) {
			float bottom = ok->GetAbsoluteOffset(Rml::BoxArea::Border).y + ok->GetBox().GetSize(Rml::BoxArea::Border).y;
			float frame = reveal->GetAbsoluteOffset(Rml::BoxArea::Border).y + reveal->GetBox().GetSize(Rml::BoxArea::Border).y;
			CHECK_MESSAGE(bottom <= frame - 19.0f, "the buttons keep the dialog's bottom margin below the pointer switch");
		}

		if (ok != nullptr) {
			Click(context, ok);
			presenter.Drain();
			REQUIRE_MESSAGE(presenter.Result.has_value(), "OK applies both switches");
			CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "OK applies both switches");
			CHECK_MESSAGE((service.Calls == std::vector<std::string>{ "stretch on", "system cursor on" }), "OK applies both switches");
			REQUIRE_MESSAGE(presenter.Picked.has_value(), "OK hands the caller the picked mode");
			CHECK_MESSAGE(presenter.Picked->Width == 1920, "OK hands the caller the picked mode");
			CHECK_MESSAGE(presenter.Picked->Height == 1080, "OK hands the caller the picked mode");
		}

		view->Release();
		context.Update();
	}

	{
		RecordingDisplayServiceClass service;
		UIDisplayPresenterClass presenter(service, Display_Fixture());
		std::unique_ptr<UIViewClass> view = UI_Display_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "a second display view prepares");
		view->Show(true);
		context.Update();

		std::vector<Rml::Element *> rows = Visible_Rows(Rml(*view).Document(), "modes");
		if (rows.size() == 3) {
			Click(context, rows[0]);
			presenter.Drain();
		}

		context.ProcessKeyDown(Rml::Input::KI_ESCAPE, 0);
		context.ProcessKeyUp(Rml::Input::KI_ESCAPE, 0);
		context.Update();
		presenter.Drain();
		REQUIRE_MESSAGE(presenter.Result.has_value(), "Escape leaves the display options with nothing applied");
		CHECK_MESSAGE(*presenter.Result == UI_RESULT_CANCELLED, "Escape leaves the display options with nothing applied");
		CHECK_MESSAGE(service.Calls.empty(), "Escape leaves the display options with nothing applied");
		CHECK_MESSAGE(!presenter.Picked.has_value(), "Escape leaves the display options with nothing applied");

		view->Release();
		context.Update();
	}

	{
		FakeClockClass clock;
		UIConfirmModePresenterClass presenter(clock);
		std::unique_ptr<UIViewClass> view = UI_Confirm_Mode_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the confirmation view prepares against the test context");
		presenter.Refresh();
		view->Show(true);
		view->Sync();
		context.Update();
		context.Render();
		CHECK_MESSAGE(system.Problems == problems, "the confirmation screen raises no RmlUi warning or error");

		Rml::ElementDocument * document = Rml(*view).Document();
		Rml::Element * prompt = document->GetElementById("text");
		REQUIRE_MESSAGE(prompt != nullptr, "the display confirmation uses the dialog bitmap font family");
		CHECK_MESSAGE(prompt->GetComputedValues().font_family() == "dlgsys", "the display confirmation uses the dialog bitmap font family");
		Rml::Element * seconds = document->GetElementById("seconds");
		REQUIRE_MESSAGE(seconds != nullptr, "the confirmation shows the ten seconds left");
		CHECK_MESSAGE(seconds->GetInnerRML() == "10", "the confirmation shows the ten seconds left");
		REQUIRE_MESSAGE(seconds != nullptr, "the display countdown inherits the dialog bitmap font family");
		CHECK_MESSAGE(seconds->GetComputedValues().font_family() == "dlgsys", "the display countdown inherits the dialog bitmap font family");

		clock.Now = 7500;
		presenter.Refresh();
		view->Sync();
		context.Update();
		REQUIRE_MESSAGE(seconds != nullptr, "the seconds shown follow the clock");
		CHECK_MESSAGE(seconds->GetInnerRML() == "3", "the seconds shown follow the clock");

		CHECK_MESSAGE(Visible_Buttons(document).size() == 2, "the confirmation has OK and Cancel");

		Rml::Element * ok = document->GetElementById("ok");
		if (ok != nullptr) {
			Click(context, ok);
			presenter.Drain();
			REQUIRE_MESSAGE(presenter.Result.has_value(), "OK keeps the mode");
			CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "OK keeps the mode");
			CHECK_MESSAGE(!presenter.TimedOut, "OK keeps the mode");
		}

		view->Release();
		context.Update();
	}

	{
		FakeClockClass clock;
		UIConfirmModePresenterClass presenter(clock);
		std::unique_ptr<UIViewClass> view = UI_Confirm_Mode_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "a second confirmation view prepares");
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
		REQUIRE_MESSAGE(presenter.Result.has_value(), "the confirmation cancels itself when the clock runs out");
		CHECK_MESSAGE(*presenter.Result == UI_RESULT_CANCELLED, "the confirmation cancels itself when the clock runs out");
		CHECK_MESSAGE(presenter.TimedOut, "the confirmation cancels itself when the clock runs out");
		REQUIRE_MESSAGE(seconds != nullptr, "the countdown ends at zero");
		CHECK_MESSAGE(seconds->GetInnerRML() == "0", "the countdown ends at zero");

		view->Release();
		context.Update();
	}
}


void Test_Keyboard_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	{
		RecordingKeyboardServiceClass service;
		UIKeyboardPresenterClass presenter(service, Keyboard_Fixture());
		std::unique_ptr<UIViewClass> view = UI_Keyboard_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the keyboard view prepares against the test context");
		view->Show(true);
		view->Sync();
		context.Update();
		context.Render();
		CHECK_MESSAGE(system.Problems == problems, "the keyboard screen raises no RmlUi warning or error");

		Rml::ElementDocument * document = Rml(*view).Document();
		std::vector<Rml::Element *> rows = Keyboard_Rows(document);
		CHECK_MESSAGE(rows.size() == 4, "the keyboard screen lists the categories and the open category's commands");
		REQUIRE_MESSAGE(rows.size() == 4, "the first category is open with its commands sorted by name");
		CHECK_MESSAGE(rows[0]->GetInnerRML() == "Interface", "the first category is open with its commands sorted by name");
		CHECK_MESSAGE(rows[0]->HasAttribute("selected"), "the first category is open with its commands sorted by name");
		CHECK_MESSAGE(rows[2]->GetInnerRML() == "Alliance", "the first category is open with its commands sorted by name");
		CHECK_MESSAGE(rows[3]->GetInnerRML() == "Toggle Repair", "the first category is open with its commands sorted by name");

		if (rows.size() == 4) {
			Pick_Category(document, 1);
			presenter.Drain();
			view->Sync();
			context.Update();
			rows = Keyboard_Rows(document);
			REQUIRE_MESSAGE(rows.size() == 4, "picking a category lists its commands");
			CHECK_MESSAGE(rows[1]->HasAttribute("selected"), "picking a category lists its commands");
			CHECK_MESSAGE(rows[2]->GetInnerRML() == "Scatter", "picking a category lists its commands");
			CHECK_MESSAGE(rows[3]->GetInnerRML() == "Select View", "picking a category lists its commands");
		}

		Rml::Element * capture = document->GetElementById("capture");
		Rml::Element * description = document->GetElementById("description");
		Rml::Element * shortcut = document->GetElementById("shortcut");
		REQUIRE_MESSAGE(capture != nullptr, "the keyboard screen has its capture element, description and shortcut");
		REQUIRE_MESSAGE(description != nullptr, "the keyboard screen has its capture element, description and shortcut");
		CHECK_MESSAGE(shortcut != nullptr, "the keyboard screen has its capture element, description and shortcut");

		if (rows.size() == 4) {
			Click(context, rows[3]);
			presenter.Drain();
			view->Sync();
			context.Update();
			CHECK_MESSAGE(presenter.State.Selected == 0, "a click on a command shows its description and shortcut");
			CHECK_MESSAGE(description->GetInnerRML() == "Selects the view", "a click on a command shows its description and shortcut");
			CHECK_MESSAGE(shortcut->GetInnerRML() == "K577", "a click on a command shows its description and shortcut");
			CHECK_MESSAGE(context.GetFocusElement() == capture, "selecting a command focuses the capture element");
		}

		if (capture != nullptr) {
			capture->Focus();
			context.ProcessKeyDown(Rml::Input::KI_R, Rml::Input::KM_SHIFT);
			context.ProcessKeyUp(Rml::Input::KI_R, Rml::Input::KM_SHIFT);
			context.Update();
			presenter.Drain();
			view->Sync();
			context.Update();
			CHECK_MESSAGE(presenter.State.Captured == 338, "Shift and R in the capture element become the number 338 and name its holder");
			CHECK_MESSAGE(presenter.State.AssignedTo == "Toggle Repair", "Shift and R in the capture element become the number 338 and name its holder");
			CHECK_MESSAGE(capture->GetInnerRML().find("K338") != std::string::npos, "the capture element shows the captured key");

			context.ProcessKeyDown(Rml::Input::KI_LSHIFT, Rml::Input::KM_SHIFT);
			context.ProcessKeyUp(Rml::Input::KI_LSHIFT, 0);
			context.Update();
			presenter.Drain();
			CHECK_MESSAGE(presenter.State.Captured == 338, "a modifier on its own leaves the capture as it was");
		}

		Rml::Element * assign = document->GetElementById("assign");
		if (assign != nullptr) {
			Click(context, assign);
			presenter.Drain();
			view->Sync();
			context.Update();
			CHECK_MESSAGE(presenter.Key_Of(0) == 338, "Assign moves the key to the selected command and shows the new shortcut");
			CHECK_MESSAGE(presenter.Key_Of(1) == 0, "Assign moves the key to the selected command and shows the new shortcut");
			CHECK_MESSAGE(shortcut->GetInnerRML() == "K338", "Assign moves the key to the selected command and shows the new shortcut");
			CHECK_MESSAGE(presenter.State.Captured == 0, "Assign moves the key to the selected command and shows the new shortcut");
		}

		if (capture != nullptr) {
			capture->Focus();
			context.ProcessKeyDown(Rml::Input::KI_RETURN, 0);
			context.ProcessKeyUp(Rml::Input::KI_RETURN, 0);
			context.Update();
			presenter.Drain();
			REQUIRE_MESSAGE(presenter.Result.has_value(), "Enter in the capture element accepts the screen and saves");
			CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "Enter in the capture element accepts the screen and saves");
			CHECK_MESSAGE(service.Calls == std::vector<std::string>{ "save 0=338 2=88" }, "Enter in the capture element accepts the screen and saves");
		}

		view->Release();
		context.Update();
	}

	{
		RecordingKeyboardServiceClass service;
		service.ConfirmAnswer = false;
		UIKeyboardPresenterClass presenter(service, Keyboard_Fixture());
		std::unique_ptr<UIViewClass> view = UI_Keyboard_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "a second keyboard view prepares");
		view->Show(true);
		view->Sync();
		context.Update();

		Rml::Element * reset = Rml(*view).Document()->GetElementById("reset");
		if (reset != nullptr) {
			Click(context, reset);
			presenter.Drain();
			CHECK_MESSAGE(service.Calls == std::vector<std::string>{ "confirm" }, "Reset All asks first and a refusal changes nothing");
			CHECK_MESSAGE(presenter.Key_Of(0) == 577, "Reset All asks first and a refusal changes nothing");
		}

		Rml::Element * capture = Rml(*view).Document()->GetElementById("capture");
		if (capture != nullptr) {
			capture->Focus();
		}
		context.ProcessKeyDown(Rml::Input::KI_ESCAPE, 0);
		context.ProcessKeyUp(Rml::Input::KI_ESCAPE, 0);
		context.Update();
		presenter.Drain();
		REQUIRE_MESSAGE(presenter.Result.has_value(), "Escape in the capture element cancels the screen without saving");
		CHECK_MESSAGE(*presenter.Result == UI_RESULT_CANCELLED, "Escape in the capture element cancels the screen without saving");
		CHECK_MESSAGE(service.Calls.size() == 1, "Escape in the capture element cancels the screen without saving");

		view->Release();
		context.Update();
	}
}


std::vector<Rml::Element *> Buttons_Top_Down(Rml::ElementDocument * document)
{
	std::vector<Rml::Element *> buttons = Visible_Buttons(document);
	std::sort(buttons.begin(), buttons.end(), [](Rml::Element * a, Rml::Element * b) {
		return(a->GetAbsoluteOffset(Rml::BoxArea::Border).y < b->GetAbsoluteOffset(Rml::BoxArea::Border).y);
	});
	return(buttons);
}


void Press(Rml::Context & context, Rml::Input::KeyIdentifier key, int modifiers = 0)
{
	context.ProcessKeyDown(key, modifiers);
	context.ProcessKeyUp(key, modifiers);
	context.Update();
}


UIDisplayState Paged_Display_Fixture(void)
{
	UIDisplayState state;
	state.Modes = {
		{ 640, 400, "640 x 400" }, { 800, 600, "800 x 600" }, { 1024, 768, "1024 x 768" },
		{ 1280, 800, "1280 x 800" }, { 1400, 1050, "1400 x 1050" }, { 1600, 900, "1600 x 900" },
		{ 1680, 1050, "1680 x 1050" }, { 1920, 1080, "1920 x 1080" },
	};
	state.Selected = 0;
	return(state);
}


void Test_Keyboard_Navigation(Rml::Context & context, CountingSystemInterfaceClass & system, RecordingRenderInterfaceClass & render)
{
	int problems = system.Problems;

	{
		RecordingDisplayServiceClass service;
		UIDisplayPresenterClass presenter(service, Display_Fixture());
		std::unique_ptr<UIViewClass> view = UI_Display_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the display view prepares for the tab order");
		view->Show(true);
		view->Sync();
		context.Update();

		Rml::ElementDocument * document = Rml(*view).Document();
		Rml::Element * modes = document->GetElementById("modes");
		Rml::Element * stretch = document->GetElementById("stretch");
		Rml::Element * cursor = document->GetElementById("system-cursor");
		Rml::Element * ok = document->GetElementById("ok");
		Rml::Element * cancel = document->GetElementById("cancel");

		int unsupported = render.Unsupported;
		Press(context, Rml::Input::KI_TAB);
		CHECK_MESSAGE(context.GetFocusElement() == modes, "the first tab stop is the list, and it shows the focus");
		CHECK_MESSAGE(modes->IsPseudoClassSet("focus-visible"), "the first tab stop is the list, and it shows the focus");
		CHECK_MESSAGE(document->IsClassSet("keys"), "driving by keyboard marks the document");

		context.Render();
		CHECK_MESSAGE(render.Unsupported == unsupported, "the focus mark stays within the implemented render methods");

		Press(context, Rml::Input::KI_TAB);
		CHECK_MESSAGE(context.GetFocusElement() == stretch, "the next tab stop is the movie switch");

		Press(context, Rml::Input::KI_SPACE);
		presenter.Drain();
		view->Sync();
		context.Update();
		CHECK_MESSAGE(presenter.State.StretchMovies, "a space on the switch turns it on");
		CHECK_MESSAGE(stretch->HasAttribute("checked"), "a space on the switch turns it on");

		Press(context, Rml::Input::KI_TAB);
		CHECK_MESSAGE(context.GetFocusElement() == cursor, "the pointer switch follows the movie switch");
		Press(context, Rml::Input::KI_TAB);
		CHECK_MESSAGE(context.GetFocusElement() == ok, "OK follows the switches");
		Press(context, Rml::Input::KI_TAB);
		CHECK_MESSAGE(context.GetFocusElement() == cancel, "and Cancel follows OK");

		Press(context, Rml::Input::KI_RETURN);
		presenter.Drain();
		REQUIRE_MESSAGE(presenter.Result.has_value(), "Enter presses the button that holds the focus, not OK");
		CHECK_MESSAGE(*presenter.Result == UI_RESULT_CANCELLED, "Enter presses the button that holds the focus, not OK");

		view->Release();
		context.Update();
	}

	{
		RecordingDisplayServiceClass service;
		UIDisplayPresenterClass presenter(service, Display_Fixture());
		std::unique_ptr<UIViewClass> view = UI_Display_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the display view prepares for a space on a button");
		view->Show(true);
		view->Sync();
		context.Update();

		Rml::Element * ok = Rml(*view).Document()->GetElementById("ok");
		if (ok != nullptr) {
			ok->Focus();
			Press(context, Rml::Input::KI_SPACE);
			presenter.Drain();
			REQUIRE_MESSAGE(presenter.Result.has_value(), "a space presses the focused button");
			CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "a space presses the focused button");
		}

		view->Release();
		context.Update();
	}

	{
		RecordingDisplayServiceClass service;
		UIDisplayPresenterClass presenter(service, Display_Fixture());
		std::unique_ptr<UIViewClass> view = UI_Display_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the display view prepares for a disabled button");
		view->Show(true);
		view->Sync();
		context.Update();

		Rml::Element * ok = Rml(*view).Document()->GetElementById("ok");
		if (ok != nullptr) {
			ok->SetClass("disabled", true);
			context.Update();
			ok->Focus();

			Press(context, Rml::Input::KI_SPACE);
			presenter.Drain();
			CHECK_MESSAGE(!presenter.Result.has_value(), "a space on a disabled button presses nothing");

			Press(context, Rml::Input::KI_RETURN);
			presenter.Drain();
			REQUIRE_MESSAGE(presenter.Result.has_value(), "and Enter falls back to accepting the screen");
			CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "and Enter falls back to accepting the screen");
		}

		view->Release();
		context.Update();
	}

	{
		RecordingDisplayServiceClass service;
		UIDisplayPresenterClass presenter(service, Paged_Display_Fixture());
		std::unique_ptr<UIViewClass> view = UI_Display_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the display view prepares for list navigation");
		view->Show(true);
		view->Sync();
		context.Update();

		Rml::ElementDocument * document = Rml(*view).Document();
		Rml::Element * modes = document->GetElementById("modes");
		modes->SetProperty("height", "60dp");
		context.Update();

		auto step = [&](Rml::Input::KeyIdentifier key) {
			Press(context, key);
			presenter.Drain();
			view->Sync();
			context.Update();
		};

		std::vector<Rml::Element *> rows = Visible_Rows(document, "modes");
		CHECK_MESSAGE(rows.size() == 8, "the list holds every mode of the fixture");

		float height = rows.empty() ? 0.0f : rows[0]->GetBox().GetSize(Rml::BoxArea::Border).y;
		int page = (height > 0.0f) ? (int)(modes->GetClientHeight() / height) : 0;
		CHECK_MESSAGE(page > 1, "the shrunk list shows a page of more than one row and fewer than all");
		CHECK_MESSAGE(page < 8, "the shrunk list shows a page of more than one row and fewer than all");

		modes->Focus();
		step(Rml::Input::KI_DOWN);
		CHECK_MESSAGE(presenter.State.Selected == 1, "down moves to the next row");
		step(Rml::Input::KI_DOWN);
		step(Rml::Input::KI_UP);
		CHECK_MESSAGE(presenter.State.Selected == 1, "up moves back");
		step(Rml::Input::KI_END);
		CHECK_MESSAGE(presenter.State.Selected == 7, "End picks the last row");
		step(Rml::Input::KI_HOME);
		CHECK_MESSAGE(presenter.State.Selected == 0, "Home picks the first");
		step(Rml::Input::KI_NEXT);
		CHECK_MESSAGE(presenter.State.Selected == page, "Page Down moves by the rows the list shows");
		step(Rml::Input::KI_PRIOR);
		CHECK_MESSAGE(presenter.State.Selected == 0, "and Page Up comes back");

		context.ProcessTextInput('8');
		presenter.Drain();
		view->Sync();
		context.Update();
		CHECK_MESSAGE(presenter.State.Selected == 1, "typing a character picks the next row that starts with it");

		view->Release();
		context.Update();
	}

	{
		RecordingDisplayServiceClass service;
		UIDisplayPresenterClass presenter(service, Paged_Display_Fixture());
		std::unique_ptr<UIViewClass> view = UI_Display_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the display view prepares for a click then a key");
		view->Show(true);
		view->Sync();
		context.Update();

		std::vector<Rml::Element *> rows = Visible_Rows(Rml(*view).Document(), "modes");
		if (rows.size() == 8) {
			Click(context, rows[3]);
			presenter.Drain();
			view->Sync();
			context.Update();
			CHECK_MESSAGE(context.GetFocusElement() == rows[3], "a click leaves the focus on the row, not the list");

			Press(context, Rml::Input::KI_DOWN);
			presenter.Drain();
			view->Sync();
			context.Update();
			CHECK_MESSAGE(presenter.State.Selected == 4, "and the arrow still moves the list the row belongs to");
		}

		view->Release();
		context.Update();
	}

	{
		RecordingSoundServiceClass service;
		UISoundState state;
		state.Score = 7;
		state.Enabled = true;
		state.InGame = false;

		UISoundPresenterClass presenter(service, state);
		std::unique_ptr<UIViewClass> view = UI_Sound_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the sound view prepares for slider keys");
		view->Show(true);
		view->Sync();
		context.Update();

		Rml::Element * score = Rml(*view).Document()->GetElementById("score");
		auto step = [&](Rml::Input::KeyIdentifier key) {
			Press(context, key);
			presenter.Drain();
			view->Sync();
			context.Update();
		};

		if (score != nullptr) {
			score->Focus();
			step(Rml::Input::KI_HOME);
			CHECK_MESSAGE(presenter.State.Score == 0, "Home takes the slider to its lowest");
			step(Rml::Input::KI_DOWN);
			CHECK_MESSAGE(presenter.State.Score == 1, "down raises it by one step");
			step(Rml::Input::KI_UP);
			CHECK_MESSAGE(presenter.State.Score == 0, "up lowers it again");
			step(Rml::Input::KI_NEXT);
			CHECK_MESSAGE(presenter.State.Score == 2, "Page Down moves it by a fifth of the range");
			step(Rml::Input::KI_END);
			CHECK_MESSAGE(presenter.State.Score == 10, "End takes it to its highest");
			step(Rml::Input::KI_PRIOR);
			CHECK_MESSAGE(presenter.State.Score == 8, "and Page Up brings it back down");
		}

		view->Release();
		context.Update();
	}

	{
		UISaveGameState fixture;
		fixture.Mode = UI_SAVE_GAME_SAVE;
		fixture.Title = "SAVE";
		fixture.AcceptCaption = "Save";
		fixture.Description = "Mission 3";
		UISaveGameEntry slot;
		slot.Description = "[EMPTY SLOT]";
		fixture.Entries.push_back(slot);

		UISaveGamePresenterClass presenter(fixture);
		std::unique_ptr<UIViewClass> view = UI_Save_Game_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the save view prepares for a space in its field");
		view->Show(true);
		view->Sync();
		context.Update();

		Rml::ElementFormControlInput * field = rmlui_dynamic_cast<Rml::ElementFormControlInput *>(Rml(*view).Document()->GetElementById("description"));
		if (field != nullptr) {
			field->Focus();
			context.ProcessKeyDown(Rml::Input::KI_SPACE, 0);
			context.ProcessTextInput(' ');
			context.ProcessKeyUp(Rml::Input::KI_SPACE, 0);
			context.Update();
			presenter.Drain();
			CHECK_MESSAGE(!presenter.Accepted, "a space in a text field is text, not a button press");
			CHECK_MESSAGE(field->GetValue().find(' ') != std::string::npos, "a space in a text field is text, not a button press");
		}

		view->Release();
		context.Update();
	}

	{
		RecordingKeyboardServiceClass service;
		UIKeyboardPresenterClass presenter(service, Keyboard_Fixture());
		std::unique_ptr<UIViewClass> view = UI_Keyboard_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the keyboard view prepares for a captured space");
		view->Show(true);
		view->Sync();
		context.Update();

		Rml::Element * capture = Rml(*view).Document()->GetElementById("capture");
		if (capture != nullptr) {
			capture->Focus();
			Press(context, Rml::Input::KI_SPACE);
			presenter.Drain();
			CHECK_MESSAGE(presenter.State.Captured == UI_Key_Number(Rml::Input::KI_SPACE, false, false, false), "a space in the capture element is recorded as a shortcut");
		}

		view->Release();
		context.Update();
	}

	CHECK_MESSAGE(system.Problems == problems, "keyboard navigation raises no RmlUi warning or error");
}


void Test_Menu_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	UIMenuState state;
	state.Kind = UI_MENU_MULTIPLAYER_FIRESTORM;
	state.Title = "Select Multiplayer Game";
	state.Items.push_back(UIMenuItemType{"Internet", 101, false});
	state.Items.push_back(UIMenuItemType{"Network", 102, true});
	state.Items.push_back(UIMenuItemType{"Main Menu", 2, true});
	state.Top = 400;

	UIMenuPresenterClass presenter(state);
	std::unique_ptr<UIViewClass> view = UI_Menu_View(presenter);

	CHECK_MESSAGE(Rml(*view).Prepare(context), "the menu view prepares against the test context");
	view->Show(true);
	context.Update();
	context.Render();
	CHECK_MESSAGE(system.Problems == problems, "the menu raises no RmlUi warning or error");

	Rml::ElementDocument * document = Rml(*view).Document();
	CHECK_MESSAGE(document->IsClassSet("multiplayer"), "the document wears the template it was asked for");
	CHECK_MESSAGE(document->IsClassSet("firestorm"), "the document wears the template it was asked for");

	Rml::Element * dialog = document->GetElementById("reveal");
	REQUIRE_MESSAGE(dialog != nullptr, "the menu sits at the top edge it was given");
	CHECK_MESSAGE(dialog->GetAbsoluteOffset(Rml::BoxArea::Border).y == 400.0f, "the menu sits at the top edge it was given");
	REQUIRE_MESSAGE(dialog != nullptr, "the expansion's template is its own size");
	CHECK_MESSAGE(dialog->GetBox().GetSize(Rml::BoxArea::Border) == Rml::Vector2f(296.0f, 206.0f), "the expansion's template is its own size");

	std::vector<Rml::Element *> buttons = Buttons_Top_Down(document);
	CHECK_MESSAGE(buttons.size() == 3, "the menu has a button for each item it was given");

	if (buttons.size() == 3) {
		Click(context, buttons[0]);
		presenter.Drain();
		REQUIRE_MESSAGE(!presenter.Result.has_value(), "a button that takes no press answers nothing");
		CHECK_MESSAGE(presenter.Choice == 0, "a button that takes no press answers nothing");

		Click(context, buttons[1]);
		presenter.Drain();
		REQUIRE_MESSAGE(presenter.Result.has_value(), "a button answers with what the caller gave it");
		CHECK_MESSAGE(presenter.Choice == 102, "a button answers with what the caller gave it");
	}

	view->Release();
	context.Update();
}


void Test_Main_Options_Screen(Rml::Context & context, CountingSystemInterfaceClass & system, RecordingRenderInterfaceClass & render)
{
	int problems = system.Problems;

	{
		UIMainOptionsState state;
		state.SoundEnabled = true;
		UIMainOptionsPresenterClass presenter(state);
		std::unique_ptr<UIViewClass> view = UI_Main_Options_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the options menu view prepares against the test context");
		view->Show(true);
		context.Update();
		context.Render();
		CHECK_MESSAGE(system.Problems == problems, "the options menu raises no RmlUi warning or error");

		std::vector<Rml::Element *> buttons = Buttons_Top_Down(Rml(*view).Document());
		CHECK_MESSAGE(buttons.size() == 5, "the options menu has five buttons");
		bool ordered = buttons.size() == 5 && buttons[0]->GetId() == "settings" && buttons[1]->GetId() == "display" && buttons[2]->GetId() == "sound" && buttons[3]->GetId() == "keyboard" && buttons[4]->GetId() == "mainmenu";
		CHECK_MESSAGE(ordered, "the buttons run Game Settings, Display, Sound, Keyboard, Main Menu from the top");

		Rml::Element * dialog = Rml(*view).Document()->GetElementById("reveal");
		float center = (float)context.GetDimensions().y * 0.5f;
		REQUIRE_MESSAGE(dialog != nullptr, "without a top edge the menu sits in the middle");
		CHECK_MESSAGE(dialog->GetAbsoluteOffset(Rml::BoxArea::Border).y < center, "without a top edge the menu sits in the middle");
		CHECK_MESSAGE(dialog->GetAbsoluteOffset(Rml::BoxArea::Border).y + dialog->GetBox().GetSize(Rml::BoxArea::Border).y > center, "without a top edge the menu sits in the middle");

		render.Scissors.clear();
		context.Render();
		bool clipped = false;
		if (dialog != nullptr) {
			Rml::Vector2f at = dialog->GetAbsoluteOffset(Rml::BoxArea::Border).Round();
			for (Rml::Rectanglei const & scissor : render.Scissors) {
				if (scissor.Left() == (int)at.x && scissor.Top() == (int)at.y && scissor.Width() == 300 && scissor.Height() == 241) {
					clipped = true;
				}
			}
		}
		CHECK_MESSAGE(clipped, "the wallpaper is cut off at the menu's edges");

		context.SetDensityIndependentPixelRatio(2.0f);
		context.Update();
		bool doubled = dialog != nullptr && dialog->GetBox().GetSize(Rml::BoxArea::Border) == Rml::Vector2f(600.0f, 482.0f)
			&& buttons.size() == 5 && buttons[0]->GetBox().GetSize(Rml::BoxArea::Border) == Rml::Vector2f(380.0f, 48.0f);
		CHECK_MESSAGE(doubled, "the menu and its buttons are twice the size at twice the ratio");
		context.SetDensityIndependentPixelRatio(1.0f);
		context.Update();

		if (buttons.size() == 5) {
			Click(context, buttons[1]);
			presenter.Drain();
			REQUIRE_MESSAGE(presenter.Result.has_value(), "the Display button closes the menu with the display choice");
			CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "the Display button closes the menu with the display choice");
			CHECK_MESSAGE(presenter.Choice == UI_MAIN_OPTIONS_DISPLAY, "the Display button closes the menu with the display choice");
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

		CHECK_MESSAGE(Rml(*view).Prepare(context), "a second options menu view prepares");
		view->Show(true);
		context.Update();

		Rml::Element * dialog = Rml(*view).Document()->GetElementById("reveal");
		REQUIRE_MESSAGE(dialog != nullptr, "the menu sits at the top edge the game hands it");
		CHECK_MESSAGE(std::fabs(dialog->GetAbsoluteOffset(Rml::BoxArea::Border).y - 200.0f) < 1.0f, "the menu sits at the top edge the game hands it");

		Rml::Element * sound = Rml(*view).Document()->GetElementById("sound");
		REQUIRE_MESSAGE(sound != nullptr, "the Sound button shows disabled without an audio device");
		CHECK_MESSAGE(sound->IsClassSet("disabled"), "the Sound button shows disabled without an audio device");
		if (sound != nullptr) {
			Click(context, sound);
			presenter.Drain();
			CHECK_MESSAGE(!presenter.Result.has_value(), "a disabled Sound button does nothing");
		}

		context.ProcessKeyDown(Rml::Input::KI_ESCAPE, 0);
		context.ProcessKeyUp(Rml::Input::KI_ESCAPE, 0);
		context.Update();
		presenter.Drain();
		REQUIRE_MESSAGE(presenter.Result.has_value(), "Escape leaves the options menu");
		CHECK_MESSAGE(*presenter.Result == UI_RESULT_CANCELLED, "Escape leaves the options menu");
		CHECK_MESSAGE(presenter.Choice == UI_MAIN_OPTIONS_LEAVE, "Escape leaves the options menu");

		view->Release();
		context.Update();
	}

	{
		UIMainOptionsState state;
		state.SoundEnabled = true;
		UIMainOptionsPresenterClass presenter(state);
		Drive(presenter, "sound");
		REQUIRE_MESSAGE(presenter.Result.has_value(), "a live Sound button picks the sound options");
		CHECK_MESSAGE(presenter.Choice == UI_MAIN_OPTIONS_SOUND, "a live Sound button picks the sound options");

		UIMainOptionsPresenterClass keyboard(state);
		Drive(keyboard, "keyboard");
		UIMainOptionsPresenterClass settings(state);
		Drive(settings, "settings");
		UIMainOptionsPresenterClass leave(state);
		Drive(leave, "ok");
		CHECK_MESSAGE(keyboard.Choice == UI_MAIN_OPTIONS_KEYBOARD, "Keyboard, Game Settings and Main Menu each answer with their choice");
		CHECK_MESSAGE(settings.Choice == UI_MAIN_OPTIONS_SETTINGS, "Keyboard, Game Settings and Main Menu each answer with their choice");
		CHECK_MESSAGE(leave.Choice == UI_MAIN_OPTIONS_LEAVE, "Keyboard, Game Settings and Main Menu each answer with their choice");
		REQUIRE_MESSAGE(leave.Result.has_value(), "Keyboard, Game Settings and Main Menu each answer with their choice");
		CHECK_MESSAGE(*leave.Result == UI_RESULT_ACCEPTED, "Keyboard, Game Settings and Main Menu each answer with their choice");
	}
}


void Test_Wait_Box_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	{
		UIWaitBoxPresenterClass presenter("Mission saving - Please Wait...", false);
		std::unique_ptr<UIViewClass> view = UI_Wait_Box_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the wait box view prepares against the test context");
		view->Show(false);
		context.Update();
		context.Render();
		CHECK_MESSAGE(system.Problems == problems, "the wait box raises no RmlUi warning or error");

		Rml::ElementDocument * document = Rml(*view).Document();
		Rml::Element * text = document->GetElementById("text");
		REQUIRE_MESSAGE(text != nullptr, "the wait box shows its text");
		CHECK_MESSAGE(text->GetInnerRML() == "Mission saving - Please Wait...", "the wait box shows its text");

		Rml::Element * hidden = document->GetElementById("bar");
		REQUIRE_MESSAGE(hidden != nullptr, "a wait box without a bar hides it");
		CHECK_MESSAGE(hidden->GetComputedValues().display() == Rml::Style::Display::None, "a wait box without a bar hides it");

		presenter.Text = "Loading in 3 seconds...";
		view->Sync();
		context.Update();
		REQUIRE_MESSAGE(text != nullptr, "the wait box text follows the presenter");
		CHECK_MESSAGE(text->GetInnerRML() == "Loading in 3 seconds...", "the wait box text follows the presenter");

		view->Release();
		context.Update();
	}

	{
		UIWaitBoxPresenterClass presenter("Working - Please Wait", true);
		presenter.Set_Fraction(0.5);
		std::unique_ptr<UIViewClass> view = UI_Wait_Box_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "a wait box with a bar prepares");
		view->Show(false);
		context.Update();

		Rml::ElementDocument * document = Rml(*view).Document();
		Rml::Element * fill = document->GetElementById("bar");
		REQUIRE_MESSAGE(fill != nullptr, "a wait box with a bar shows one");
		CHECK_MESSAGE(fill->IsVisible(), "a wait box with a bar shows one");

		Rml::ElementProgress * progress = rmlui_dynamic_cast<Rml::ElementProgress *>(fill);
		REQUIRE_MESSAGE(progress != nullptr, "the fill stands at fifty at fifty percent");
		CHECK_MESSAGE(std::fabs(progress->GetValue() - 50.0f) < 0.01f, "the fill stands at fifty at fifty percent");

		presenter.Set_Fraction(1.5);
		view->Sync();
		context.Update();
		CHECK_MESSAGE(presenter.Percent == 100, "the fraction clamps to a full bar");
		REQUIRE_MESSAGE(progress != nullptr, "the fraction clamps to a full bar");
		CHECK_MESSAGE(std::fabs(progress->GetValue() - 100.0f) < 0.01f, "the fraction clamps to a full bar");

		view->Release();
		context.Update();
	}
}


class RecordingScenarioServiceClass : public UIScenarioServiceClass
{
	public:
		int Asked = 0;
		int Row = -1;

		int Generated = 0;
		int Made = -1;

		virtual void Preview(int index, UIMapPreviewImage & image) override
		{
			Asked++;
			Row = index;
			image.Width = 2;
			image.Height = 2;
			image.Pixels.assign(2 * 2 * 4, 128);
			image.Generation++;
		}

		virtual void Read(UIScenarioState &) override
		{
		}

		virtual int Random(void) override
		{
			Generated++;
			return(Made);
		}
};


void Test_Scenario_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	RecordingScenarioServiceClass service;
	UIScenarioState state;
	for (int index = 0; index < 4; index++) {
		UIScenarioEntry entry;
		entry.Label = "Map " + std::to_string(index + 1);
		state.Entries.push_back(entry);
	}

	UIScenarioPresenterClass presenter(service, std::move(state));
	std::unique_ptr<UIViewClass> view = UI_Scenario_View(presenter);

	CHECK_MESSAGE(Rml(*view).Prepare(context), "the map view prepares against the test context");
	view->Show(false);
	view->Sync();
	context.Update();
	context.Render();
	CHECK_MESSAGE(system.Problems == problems, "the map dialog raises no RmlUi warning or error");

	Rml::ElementDocument * document = Rml(*view).Document();
	Rml::Element * dialog = document->GetElementById("reveal");
	REQUIRE_MESSAGE(dialog != nullptr, "the map dialog is the size its template comes to");
	CHECK_MESSAGE(dialog->GetBox().GetSize(Rml::BoxArea::Border) == Rml::Vector2f(540.0f, 326.0f), "the map dialog is the size its template comes to");

	Rml::Element * maps = document->GetElementById("maps");
	REQUIRE_MESSAGE(maps != nullptr, "the mission list is its template's rect, frame and all");
	CHECK_MESSAGE(maps->GetBox().GetSize(Rml::BoxArea::Border) == Rml::Vector2f(263.0f, 232.0f), "the mission list is its template's rect, frame and all");

	CHECK_MESSAGE(rmlui_dynamic_cast<UIRmlSurfaceElementClass *>(document->GetElementById("preview")) != nullptr, "the map dialog holds a surface for the preview");

	Drive(presenter, "select", 2);
	CHECK_MESSAGE(presenter.State.Selected == 2, "moving the highlight asks the engine for that map's picture");
	CHECK_MESSAGE(service.Asked == 1, "moving the highlight asks the engine for that map's picture");
	CHECK_MESSAGE(service.Row == 2, "moving the highlight asks the engine for that map's picture");
	Drive(presenter, "select", 2);
	CHECK_MESSAGE(service.Asked == 1, "and standing still asks for nothing");
	Drive(presenter, "select", 9);
	CHECK_MESSAGE(presenter.State.Selected == 2, "a row the list does not hold is ignored");
	CHECK_MESSAGE(service.Asked == 1, "a row the list does not hold is ignored");

	service.Made = 3;
	Drive(presenter, "random");
	REQUIRE_MESSAGE(!presenter.Result.has_value(), "the generator button runs the generator over the dialog, which stays open");
	CHECK_MESSAGE(service.Generated == 1, "the generator button runs the generator over the dialog, which stays open");
	CHECK_MESSAGE(presenter.State.Selected == 3, "and the dialog takes the map it made, with its picture");
	CHECK_MESSAGE(service.Row == 3, "and the dialog takes the map it made, with its picture");

	view->Release();
	context.Update();
}


class RecordingNetServiceClass : public UINetLobbyServiceClass
{
	public:
		UINetLobbyState Model;

		int Reads = 0;
		int Games = 0;
		int LastGame = -1;
		std::vector<std::string> Said;
		std::vector<std::string> Kicked;
		int Accepts = 0;
		std::vector<int> Switches;
		int Sliders = 0;

		virtual void Read(UINetLobbyState & state) override
		{
			Reads++;
			state.Kind = Model.Kind;
			state.Host = Model.Host;
			state.Answered = Model.Answered;
			state.Games = Model.Games;
			state.Players = Model.Players;
			state.Chat = Model.Chat;
			state.Sides = Model.Sides;
			state.Colors = Model.Colors;
		}

		virtual void Set_Handle(char const *) override {}
		virtual void Select_Game(int index) override { Games++; LastGame = index; }
		virtual void Say(char const * text) override { Said.push_back(text); }
		virtual void Set_Side(int) override {}
		virtual void Set_Color(int) override {}
		virtual void Set_Switch(UINetSwitch which, bool) override { Switches.push_back((int)which); }
		virtual void Set_Slider(UINetSlider, int) override { Sliders++; }
		virtual void Kick(std::vector<std::string> const & names) override { Kicked = names; }
		virtual void Accept(void) override { Accepts++; }

		int Picked = 0;
		virtual void Pick_Map(void) override { Picked++; }

		int Joins = 0;
		int Hosts = 0;
		int Starts = 0;
		bool Hostable = false;
		bool Startable = false;
		virtual void Join(void) override { Joins++; }
		virtual void Host(void) override { Hosts++; if (Hostable) { Model.Kind = UI_NET_LOBBY_HOST; } }
		virtual bool Can_Start(void) override { Starts++; return(Startable); }
};


static UINetPlayerRow Net_Player(char const * name)
{
	UINetPlayerRow row;
	row.Name = name;
	row.Color = "#ffd800";
	row.House = "gdii.pcx";
	row.Hint = "GDI";
	return(row);
}


void Test_Game_Options_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	class RecordingGameOptionsServiceClass : public UIGameOptionsServiceClass
	{
		public:
			bool Files = true;

			virtual void Read(UIGameOptionsState & state) override { state.LoadEnabled = Files; state.DeleteEnabled = Files; }
			virtual void Save(void) override {}
			virtual void Delete(void) override { Files = false; }
			virtual bool Load(void) override { return(false); }
	};
	RecordingGameOptionsServiceClass service;

	UIGameOptionsState state;
	UIGameOptionsPresenterClass presenter(service, state);
	std::unique_ptr<UIViewClass> view = UI_Game_Options_View(presenter);

	CHECK_MESSAGE(Rml(*view).Prepare(context), "the options menu view prepares against the test context");
	view->Show(true);
	view->Sync();
	context.Update();
	context.Render();
	CHECK_MESSAGE(system.Problems == problems, "the in-game options menu raises no RmlUi warning or error");

	Rml::ElementDocument * document = Rml(*view).Document();
	Rml::Element * load = (document != nullptr) ? document->GetElementById("load") : nullptr;
	Rml::Element * erase = (document != nullptr) ? document->GetElementById("delete") : nullptr;
	REQUIRE_MESSAGE(load != nullptr, "the menu opens with Load and Delete taking a press");
	REQUIRE_MESSAGE(erase != nullptr, "the menu opens with Load and Delete taking a press");
	CHECK_MESSAGE(!load->IsClassSet("disabled"), "the menu opens with Load and Delete taking a press");
	CHECK_MESSAGE(!erase->IsClassSet("disabled"), "the menu opens with Load and Delete taking a press");

	Drive(presenter, "delete");
	view->Sync();
	context.Update();
	CHECK_MESSAGE(!presenter.State.LoadEnabled, "a delete that leaves no saves takes the presses away on the same pass");
	REQUIRE_MESSAGE(load != nullptr, "a delete that leaves no saves takes the presses away on the same pass");
	REQUIRE_MESSAGE(erase != nullptr, "a delete that leaves no saves takes the presses away on the same pass");
	CHECK_MESSAGE(load->IsClassSet("disabled"), "a delete that leaves no saves takes the presses away on the same pass");
	CHECK_MESSAGE(erase->IsClassSet("disabled"), "a delete that leaves no saves takes the presses away on the same pass");

	view->Release();
	context.Update();

	UIGameOptionsState network;
	network.Solo = false;
	UIGameOptionsPresenterClass networkpresenter(service, network);
	std::unique_ptr<UIViewClass> networkview = UI_Game_Options_View(networkpresenter);
	CHECK_MESSAGE(Rml(*networkview).Prepare(context), "the network options menu view prepares");
	networkview->Show(true);
	networkview->Sync();
	context.Update();

	auto shown = [&](char const * id) {
		Rml::Element * element = Rml(*networkview).Document()->GetElementById(id);
		return(element != nullptr && element->IsVisible(true));
	};
	CHECK_MESSAGE(shown("load"), "a network game's menu offers Load and Save without Delete or Restate Briefing");
	CHECK_MESSAGE(shown("save"), "a network game's menu offers Load and Save without Delete or Restate Briefing");
	CHECK_MESSAGE(!shown("delete"), "a network game's menu offers Load and Save without Delete or Restate Briefing");
	CHECK_MESSAGE(!shown("briefing"), "a network game's menu offers Load and Save without Delete or Restate Briefing");

	networkview->Release();
	context.Update();
}


void Test_Save_Game_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	UISaveGameState fixture;
	fixture.Mode = UI_SAVE_GAME_SAVE;
	fixture.Title = "SAVE";
	fixture.AcceptCaption = "Save";
	fixture.Suggested = "Mission 3";
	fixture.Description = "Mission 3";
	UISaveGameEntry slot;
	slot.Description = "[EMPTY SLOT]";
	fixture.Entries.push_back(slot);
	UISaveGameEntry file;
	file.Description = "Before the bridge";
	file.Date = "1/1/2026";
	file.Time = "10:00";
	file.Valid = true;
	fixture.Entries.push_back(file);

	auto open = [&](UISaveGamePresenterClass & presenter, std::unique_ptr<UIViewClass> & view) {
		view = UI_Save_Game_View(presenter);
		bool prepared = Rml(*view).Prepare(context);
		view->Show(true);
		view->Sync();
		context.Update();
		return(prepared);
	};

	{
		UISaveGamePresenterClass presenter(fixture);
		std::unique_ptr<UIViewClass> view;
		CHECK_MESSAGE(open(presenter, view), "the save view prepares against the test context");
		context.Render();
		CHECK_MESSAGE(system.Problems == problems, "the save screen raises no RmlUi warning or error");

		Drive(presenter, "select", 1);
		CHECK_MESSAGE(presenter.State.Selected == 1, "picking a saved game hands its description to the field");
		CHECK_MESSAGE(presenter.State.Description == "Before the bridge", "picking a saved game hands its description to the field");
		Drive(presenter, "select", 0);
		CHECK_MESSAGE(presenter.State.Description == "Mission 3", "picking the empty slot hands the suggestion back");

		Drive(presenter, "description", 0, "Before the br");
		REQUIRE_MESSAGE(!presenter.Result.has_value(), "typing changes the description and accepts nothing");
		CHECK_MESSAGE(presenter.State.Description == "Before the br", "typing changes the description and accepts nothing");
		Drive(presenter, "description", 1, "Before the bridge II");
		CHECK_MESSAGE(presenter.Accepted, "a line break in the field accepts with what was typed");
		CHECK_MESSAGE(presenter.State.Description == "Before the bridge II", "a line break in the field accepts with what was typed");

		view->Release();
		context.Update();
	}

	{
		UISaveGamePresenterClass presenter(fixture);
		std::unique_ptr<UIViewClass> view;
		CHECK_MESSAGE(open(presenter, view), "the save view prepares for the field to follow a pick");

		Rml::ElementFormControlInput * field = rmlui_dynamic_cast<Rml::ElementFormControlInput *>(Rml(*view).Document()->GetElementById("description"));
		REQUIRE_MESSAGE(field != nullptr, "the description does not hold the focus while the list is untouched");
		CHECK_MESSAGE(context.GetFocusElement() != field, "the description does not hold the focus while the list is untouched");

		Drive(presenter, "select", 1);
		view->Sync();
		context.Update();
		view->Placed();
		context.Update();

		if (field != nullptr) {
			int start = 0;
			int end = 0;
			std::string selected;
			field->GetSelection(&start, &end, &selected);
			CHECK_MESSAGE(context.GetFocusElement() == field, "picking a saved game moves the focus to the description");
			CHECK_MESSAGE(selected == "Before the bridge", "with the whole description selected");

			context.ProcessTextInput('X');
			context.Update();
			CHECK_MESSAGE(field->GetValue() == "X", "so that typing replaces it");
		}

		view->Release();
		context.Update();
	}

	{
		UISaveGamePresenterClass presenter(fixture);
		std::unique_ptr<UIViewClass> view;
		CHECK_MESSAGE(open(presenter, view), "the save view prepares for its keys");

		Rml::ElementDocument * document = Rml(*view).Document();
		Rml::Element * field = (document != nullptr) ? document->GetElementById("description") : nullptr;
		CHECK_MESSAGE(field != nullptr, "the save screen has its description field");
		if (field != nullptr) {
			field->Focus();
			context.ProcessKeyDown(Rml::Input::KI_RETURN, 0);
			context.ProcessKeyUp(Rml::Input::KI_RETURN, 0);
			context.Update();
			presenter.Drain();
			CHECK_MESSAGE(presenter.Accepted, "Enter in the field reaches the presenter as a line break and accepts");
			CHECK_MESSAGE(presenter.State.Description == "Mission 3", "Enter in the field reaches the presenter as a line break and accepts");
		}

		view->Release();
		context.Update();
	}

	{
		UISaveGamePresenterClass presenter(fixture);
		std::unique_ptr<UIViewClass> view;
		CHECK_MESSAGE(open(presenter, view), "the save view prepares for Escape");

		Rml::ElementDocument * document = Rml(*view).Document();
		Rml::Element * field = (document != nullptr) ? document->GetElementById("description") : nullptr;
		if (field != nullptr) {
			field->Focus();
			context.ProcessKeyDown(Rml::Input::KI_ESCAPE, 0);
			context.ProcessKeyUp(Rml::Input::KI_ESCAPE, 0);
			context.Update();
			presenter.Drain();
			REQUIRE_MESSAGE(presenter.Result.has_value(), "Escape from the field cancels the screen");
			CHECK_MESSAGE(*presenter.Result == UI_RESULT_CANCELLED, "Escape from the field cancels the screen");
		}

		view->Release();
		context.Update();
	}

	{
		UISaveGamePresenterClass presenter(fixture);
		std::unique_ptr<UIViewClass> view;
		CHECK_MESSAGE(open(presenter, view), "the save view prepares for a double-click");

		std::vector<Rml::Element *> rows = Visible_Rows(Rml(*view).Document(), "files");
		CHECK_MESSAGE(rows.size() == 2, "the save screen lists the empty slot and the file");
		if (rows.size() == 2) {
			rows[1]->DispatchEvent(Rml::EventId::Dblclick, Rml::Dictionary());
			presenter.Drain();
			CHECK_MESSAGE(!presenter.Accepted, "a double-click in the save list saves nothing");
		}

		view->Release();
		context.Update();
	}

	{
		UISaveGameState loading = fixture;
		loading.Mode = UI_SAVE_GAME_LOAD;
		loading.Title = "LOAD";
		loading.AcceptCaption = "Load";

		UISaveGamePresenterClass presenter(loading);
		std::unique_ptr<UIViewClass> view;
		CHECK_MESSAGE(open(presenter, view), "the load view prepares for a double-click");

		std::vector<Rml::Element *> rows = Visible_Rows(Rml(*view).Document(), "files");
		if (rows.size() == 2) {
			rows[1]->DispatchEvent(Rml::EventId::Dblclick, Rml::Dictionary());
			presenter.Drain();
			CHECK_MESSAGE(presenter.Accepted, "a double-click in the load list loads that game");
		}

		view->Release();
		context.Update();
	}

	{
		UISaveGameState deleting = fixture;
		deleting.Mode = UI_SAVE_GAME_DELETE;
		deleting.Title = "DELETE";
		deleting.AcceptCaption = "Delete";

		UISaveGamePresenterClass presenter(deleting);
		std::unique_ptr<UIViewClass> view;
		CHECK_MESSAGE(open(presenter, view), "the delete view prepares for a double-click");

		std::vector<Rml::Element *> rows = Visible_Rows(Rml(*view).Document(), "files");
		if (rows.size() == 2) {
			rows[1]->DispatchEvent(Rml::EventId::Dblclick, Rml::Dictionary());
			presenter.Drain();
			CHECK_MESSAGE(!presenter.Accepted, "a double-click in the delete list deletes nothing");
		}

		view->Release();
		context.Update();
	}

	float const originalscale = context.GetDensityIndependentPixelRatio();
	Rml::Vector2i const originaldimensions = context.GetDimensions();
	for (float scale : { 1.0f, 2.0f }) {
		context.SetDensityIndependentPixelRatio(scale);
		context.SetDimensions(Rml::Vector2i(1280, 800) * (int)scale);
		for (UISaveGameMode mode : { UI_SAVE_GAME_LOAD, UI_SAVE_GAME_SAVE, UI_SAVE_GAME_DELETE }) {
			for (int count : { 2, 24 }) {
				std::printf("  save list mode %d, scale %.0f, rows %d\n", (int)mode, scale, count);
				UISaveGameState state = fixture;
				state.Mode = mode;
				UISaveGameEntry longfile = file;
				longfile.Description = std::string(79, 'W');
				longfile.Date = "12/31/2026";
				longfile.Time = "11:59 PM";
				state.Entries.assign(count, longfile);
				state.Entries[0] = slot;
				UISaveGamePresenterClass presenter(state);
				std::unique_ptr<UIViewClass> view;
				CHECK_MESSAGE(open(presenter, view), "the save list prepares for timestamp layout checks");
				context.Render();

				Rml::ElementDocument * document = Rml(*view).Document();
				std::vector<Rml::Element *> rows = Visible_Rows(document, "files");
				CHECK_MESSAGE(rows.size() == (std::size_t)count, "the timestamp fixture lists every entry");
				if (rows.size() == (std::size_t)count) {
					Rml::Element * description = rows[1]->QuerySelector(".cell-description");
					Rml::Element * stamp = rows[1]->QuerySelector(".cell-stamp");
					REQUIRE_MESSAGE(description != nullptr, "each saved game has a description and one timestamp field");
					CHECK_MESSAGE(stamp != nullptr, "each saved game has a description and one timestamp field");
					if (description != nullptr && stamp != nullptr) {
						CHECK_MESSAGE(stamp->GetInnerRML() == "12/31/2026 11:59 PM", "the timestamp joins the date and time with a space");
						float const descriptionright = description->GetAbsoluteOffset(Rml::BoxArea::Border).x + description->GetBox().GetSize(Rml::BoxArea::Border).x;
						float const stampleft = stamp->GetAbsoluteOffset(Rml::BoxArea::Border).x;
						float const stampright = stampleft + stamp->GetBox().GetSize(Rml::BoxArea::Border).x;
						float const rowright = rows[1]->GetAbsoluteOffset(Rml::BoxArea::Border).x + rows[1]->GetBox().GetSize(Rml::BoxArea::Border).x;
						CHECK_MESSAGE(stampleft - descriptionright >= 8.0f * scale - 1.0f, "the description leaves a gap before the timestamp");
						CHECK_MESSAGE(std::abs(rowright - stampright - 2.0f * scale) <= 1.0f, "the timestamp stays at the row's right inset");
						Rml::Element * heading = document->GetElementById("heading-stamp");
						REQUIRE_MESSAGE(heading != nullptr, "the timestamp heading is centered in its area");
						CHECK_MESSAGE(heading->GetComputedValues().text_align() == Rml::Style::TextAlign::Center, "the timestamp heading is centered in its area");
						if (heading != nullptr && count == 2) {
							float const headingright = heading->GetAbsoluteOffset(Rml::BoxArea::Border).x + heading->GetBox().GetSize(Rml::BoxArea::Border).x;
							CHECK_MESSAGE(std::abs(headingright - stampright) <= 1.0f, "the timestamp heading area ends at the timestamp's right edge");
						}
						CHECK_MESSAGE(description->GetComputedValues().overflow_x() == Rml::Style::Overflow::Hidden, "a long description is clipped before the timestamp");
						Rml::Element * list = document->GetElementById("files");
						float const listright = list->GetAbsoluteOffset(Rml::BoxArea::Padding).x + list->GetClientWidth();
						CHECK_MESSAGE(stampright <= listright, "the timestamp fits beside the list's scrollbar");
						CHECK_MESSAGE((list->GetScrollHeight() > list->GetClientHeight()) == (count == 24), "the layout fixture covers lists with and without scrolling");
					}
				}

				view->Release();
				context.Update();
			}
		}
	}
	context.SetDensityIndependentPixelRatio(originalscale);
	context.SetDimensions(originaldimensions);
	context.Update();
	CHECK_MESSAGE(system.Problems == problems, "the save list layouts raise no RmlUi warning or error");
}


void Test_Net_Browser_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	RecordingNetServiceClass service;
	service.Model.Kind = UI_NET_LOBBY_GAMES;
	for (int index = 0; index < 3; index++) {
		UINetOption game;
		game.Label = "Game " + std::to_string(index);
		service.Model.Games.push_back(game);
	}
	service.Model.Players.push_back(Net_Player("Someone"));

	UINetLobbyState state = service.Model;
	UINetLobbyPresenterClass presenter(service, state);
	std::unique_ptr<UIViewClass> view = UI_Net_Browser_View(presenter);

	CHECK_MESSAGE(Rml(*view).Prepare(context), "the browser prepares against the test context");
	view->Show(false);
	view->Sync();
	context.Update();
	context.Render();
	CHECK_MESSAGE(system.Problems == problems, "the browser raises no RmlUi warning or error");

	Rml::ElementDocument * document = Rml(*view).Document();
	Rml::Element * dialog = document->GetElementById("reveal");
	REQUIRE_MESSAGE(dialog != nullptr, "the browser is the size its template comes to");
	CHECK_MESSAGE(dialog->GetBox().GetSize(Rml::BoxArea::Border) == Rml::Vector2f(640.0f, 391.0f), "the browser is the size its template comes to");

	Rml::Element * chat = document->GetElementById("chat");
	REQUIRE_MESSAGE(chat != nullptr, "the message log is its template's rect, frame and all");
	CHECK_MESSAGE(chat->GetBox().GetSize(Rml::BoxArea::Border) == Rml::Vector2f(400.0f, 271.0f), "the message log is its template's rect, frame and all");

	Rml::Element * games = document->GetElementById("games");
	REQUIRE_MESSAGE(games != nullptr, "the game list is its template's rect");
	CHECK_MESSAGE(games->GetBox().GetSize(Rml::BoxArea::Border) == Rml::Vector2f(170.0f, 111.0f), "the game list is its template's rect");

	Drive(presenter, "game", 2);
	CHECK_MESSAGE(presenter.State.Game == 2, "moving the highlight asks the wire about that game");
	CHECK_MESSAGE(service.Games == 1, "moving the highlight asks the wire about that game");
	CHECK_MESSAGE(service.LastGame == 2, "moving the highlight asks the wire about that game");
	Drive(presenter, "game", 2);
	CHECK_MESSAGE(service.Games == 1, "and standing still asks for nothing");
	Drive(presenter, "game", 9);
	CHECK_MESSAGE(presenter.State.Game == 2, "a row the list does not hold is ignored");
	CHECK_MESSAGE(service.Games == 1, "a row the list does not hold is ignored");

	Drive(presenter, "say", 0, "hello");
	CHECK_MESSAGE(service.Said.empty(), "a line still being typed is not sent");
	Drive(presenter, "say", 1, "hi");
	CHECK_MESSAGE(service.Said.empty(), "a line of two characters is dropped, as the dialog drops it");
	Drive(presenter, "say", 1, "hello");
	REQUIRE_MESSAGE(service.Said.size() == 1, "ending a line sends it");
	CHECK_MESSAGE(service.Said[0] == "hello", "ending a line sends it");
	CHECK_MESSAGE(presenter.State.Say.empty(), "and empties the box");

	Drive(presenter, "say", 0, "typ");
	Drive(presenter, "join");
	presenter.Refresh();
	CHECK_MESSAGE(service.Joins == 1, "the join button asks to join and keeps the browser up with its chat line");
	REQUIRE_MESSAGE(!presenter.Result.has_value(), "the join button asks to join and keeps the browser up with its chat line");
	CHECK_MESSAGE(presenter.State.Say == "typ", "the join button asks to join and keeps the browser up with its chat line");
	Drive(presenter, "new");
	presenter.Refresh();
	CHECK_MESSAGE(service.Hosts == 1, "a new game the name refuses keeps the browser up");
	CHECK_MESSAGE(!presenter.Result.has_value(), "a new game the name refuses keeps the browser up");
	service.Hostable = true;
	Drive(presenter, "new");
	presenter.Refresh();
	REQUIRE_MESSAGE(presenter.Result.has_value(), "a new game moves the flow to the host's setup, which closes the browser without an answer of its own");
	CHECK_MESSAGE(presenter.Choice == UI_NET_NONE, "a new game moves the flow to the host's setup, which closes the browser without an answer of its own");

	view->Release();
	context.Update();
}


void Test_Net_Setup_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	RecordingNetServiceClass service;
	service.Model.Kind = UI_NET_LOBBY_HOST;
	service.Model.Host = true;
	service.Model.Players.push_back(Net_Player("Host"));
	service.Model.Players.push_back(Net_Player("Guest"));
	UINetOption side;
	side.Label = "GDI";
	service.Model.Sides.push_back(side);
	UINetOption color;
	color.Label = "Gold";
	color.Color = "#ffd800";
	service.Model.Colors.push_back(color);

	UINetLobbyState state = service.Model;
	state.Bases = true;
	state.ShortGame = true;

	UINetLobbyPresenterClass presenter(service, state);
	std::unique_ptr<UIViewClass> view = UI_Net_Setup_View(presenter);

	CHECK_MESSAGE(Rml(*view).Prepare(context), "the setup prepares against the test context");
	view->Show(false);
	view->Sync();
	context.Update();
	context.Render();
	CHECK_MESSAGE(system.Problems == problems, "the setup raises no RmlUi warning or error");

	Rml::ElementDocument * document = Rml(*view).Document();
	Rml::Element * dialog = document->GetElementById("reveal");
	REQUIRE_MESSAGE(dialog != nullptr, "the setup is the size its template comes to");
	CHECK_MESSAGE(dialog->GetBox().GetSize(Rml::BoxArea::Border) == Rml::Vector2f(640.0f, 391.0f), "the setup is the size its template comes to");

	Rml::Element * players = document->GetElementById("players");
	REQUIRE_MESSAGE(players != nullptr, "the player list is its template's rect");
	CHECK_MESSAGE(players->GetBox().GetSize(Rml::BoxArea::Border) == Rml::Vector2f(226.0f, 110.0f), "the player list is its template's rect");

	CHECK_MESSAGE(rmlui_dynamic_cast<UIRmlSurfaceElementClass *>(document->GetElementById("preview")) != nullptr, "the setup holds a surface for the map's picture");

	Drive(presenter, "bases", 0);
	CHECK_MESSAGE(!presenter.State.Bases, "clearing bases clears the short game with it");
	CHECK_MESSAGE(!presenter.State.ShortGame, "clearing bases clears the short game with it");
	Drive(presenter, "short", 1);
	CHECK_MESSAGE(presenter.State.Bases, "asking for a short game brings bases back");
	CHECK_MESSAGE(presenter.State.ShortGame, "asking for a short game brings bases back");

	Drive(presenter, "kick");
	CHECK_MESSAGE(service.Kicked.empty(), "the kick button does nothing while nobody is picked");
	Drive(presenter, "pick", 1);
	CHECK_MESSAGE(presenter.State.Players[1].Selected, "a picked row is marked");
	Drive(presenter, "kick");
	REQUIRE_MESSAGE(service.Kicked.size() == 1, "the kick button reports who was picked");
	CHECK_MESSAGE(service.Kicked[0] == "Guest", "the kick button reports who was picked");
	CHECK_MESSAGE(!presenter.State.Players[1].Selected, "and lets them go again");

	Drive(presenter, "pick", 1);
	Drive(presenter, "go");
	presenter.Refresh();
	CHECK_MESSAGE(service.Starts == 1, "a start the game refuses keeps the setup up with its pick");
	REQUIRE_MESSAGE(!presenter.Result.has_value(), "a start the game refuses keeps the setup up with its pick");
	CHECK_MESSAGE(presenter.State.Players[1].Selected, "a start the game refuses keeps the setup up with its pick");
	service.Startable = true;
	Drive(presenter, "go");
	REQUIRE_MESSAGE(presenter.Result.has_value(), "a start the game allows closes the setup with the go answer");
	CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "a start the game allows closes the setup with the go answer");
	CHECK_MESSAGE(presenter.Choice == UI_NET_GO, "a start the game allows closes the setup with the go answer");

	UINetLobbyPresenterClass left(service, state);
	service.Model.Kind = UI_NET_LOBBY_GAMES;
	left.Refresh();
	CHECK_MESSAGE(left.Result.has_value(), "a lobby the flow has left closes its screen");

	view->Release();
	context.Update();
}


class RecordingSkirmishServiceClass : public UISkirmishServiceClass
{
	public:
		int Picked = 0;
		int Options = 0;
		bool Startable = true;
		int Asked = 0;

		virtual void Pick_Map(UISkirmishState & state) override
		{
			Picked++;
			state.MapName = "Picked map";
		}

		virtual void Configure_Options(UISkirmishState &) override
		{
			Options++;
		}

		virtual bool Can_Start(UISkirmishState const &) override
		{
			Asked++;
			return(Startable);
		}
};


bool Row_Is_In_View(Rml::Element * list, Rml::Element * row)
{
	float client = list->GetAbsoluteOffset(Rml::BoxArea::Border).y + list->GetClientTop();
	float top = row->GetAbsoluteOffset(Rml::BoxArea::Border).y - client;
	return(top >= -1.5f && top + row->GetBox().GetSize(Rml::BoxArea::Border).y <= list->GetClientHeight() + 1.5f);
}


void Test_List_Scrolling(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	{
		RecordingSoundServiceClass service;
		UISoundState state;
		state.Enabled = true;
		state.InGame = true;
		for (int index = 0; index < 12; index++) {
			state.Tracks.push_back({ "Track " + std::to_string(index), index });
		}
		state.Selected = 10;

		UISoundPresenterClass presenter(service, state);
		std::unique_ptr<UIViewClass> view = UI_Sound_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the sound view prepares for a long track list");
		view->Show(true);
		view->Sync();
		context.Update();

		Rml::ElementDocument * document = Rml(*view).Document();
		Rml::Element * tracks = document->GetElementById("tracks");
		tracks->SetProperty("height", "60dp");
		context.Update();
		view->Placed();
		context.Update();

		std::vector<Rml::Element *> rows = Visible_Of_Class(document, "track");
		CHECK_MESSAGE(rows.size() == 12, "the track list holds every track");

		float down = tracks->GetScrollTop();
		CHECK_MESSAGE(down > 0.0f, "opening on a late track scrolls the list down to it");
		REQUIRE_MESSAGE(rows.size() == 12, "and the playing track is in view");
		CHECK_MESSAGE(Row_Is_In_View(tracks, rows[10]), "and the playing track is in view");

		Drive(presenter, "select", 0);
		view->Sync();
		context.Update();
		view->Placed();
		context.Update();
		CHECK_MESSAGE(tracks->GetScrollTop() < down, "picking the first track scrolls back up");
		REQUIRE_MESSAGE(rows.size() == 12, "and brings that track into view");
		CHECK_MESSAGE(Row_Is_In_View(tracks, rows[0]), "and brings that track into view");

		view->Release();
		context.Update();
	}

	{
		RecordingNetServiceClass service;
		service.Model.Kind = UI_NET_LOBBY_GAMES;
		for (int index = 0; index < 40; index++) {
			service.Model.Chat.push_back({ "Line " + std::to_string(index), "#70ff00" });
		}

		UINetLobbyState state = service.Model;
		UINetLobbyPresenterClass presenter(service, state);
		std::unique_ptr<UIViewClass> view = UI_Net_Browser_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the browser prepares for a full message log");
		view->Show(false);
		view->Sync();
		context.Update();
		view->Placed();
		context.Update();

		Rml::Element * chat = Rml(*view).Document()->GetElementById("chat");
		float bottom = chat->GetScrollHeight() - chat->GetClientHeight();
		CHECK_MESSAGE(bottom > 0.0f, "the log holds more lines than it can show");
		CHECK_MESSAGE(std::fabs(chat->GetScrollTop() - bottom) < 1.0f, "the log opens on its newest line");

		presenter.State.Chat.push_back({ "One more", "#70ff00" });
		view->Sync();
		context.Update();
		view->Placed();
		context.Update();

		bottom = chat->GetScrollHeight() - chat->GetClientHeight();
		CHECK_MESSAGE(std::fabs(chat->GetScrollTop() - bottom) < 1.0f, "and stays on the newest line as messages arrive");

		view->Release();
		context.Update();
	}

	CHECK_MESSAGE(system.Problems == problems, "list scrolling raises no RmlUi warning or error");
}


void Test_Skirmish_Screen(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	UISkirmishState state;
	state.Handle = "Player";
	for (int index = 0; index < 2; index++) {
		UISkirmishOption side;
		side.Label = index == 0 ? "GDI" : "Nod";
		side.Value = index;
		state.Sides.push_back(side);

		UISkirmishOption color;
		color.Label = index == 0 ? "Gold" : "Red";
		color.Value = index;
		color.Color = index == 0 ? "#ffdf5a" : "#ff1818";
		state.Colors.push_back(color);
	}
	state.MapName = "Grand Canyon (2-4)";
	state.AIPlayers = 2;
	state.AI.assign(state.AIPlayers, UISkirmishAISlot());
	state.Preview.Width = 4;
	state.Preview.Height = 2;
	state.Preview.Pixels.assign(4 * 2 * 4, 200);
	state.Preview.Generation = 1;

	RecordingSkirmishServiceClass service;
	UISkirmishPresenterClass presenter(service, state);
	std::unique_ptr<UIViewClass> view = UI_Skirmish_View(presenter);

	CHECK_MESSAGE(Rml(*view).Prepare(context), "the skirmish view prepares against the test context");
	view->Show(false);
	view->Sync();
	context.Update();
	context.Render();
	CHECK_MESSAGE(system.Problems == problems, "the skirmish screen raises no RmlUi warning or error");

	Rml::ElementDocument * document = Rml(*view).Document();
	Rml::Element * dialog = document->GetElementById("reveal");
	REQUIRE_MESSAGE(dialog != nullptr, "the setup is the size its template comes to");
	CHECK_MESSAGE(dialog->GetBox().GetSize(Rml::BoxArea::Border) == Rml::Vector2f(640.0f, 391.0f), "the setup is the size its template comes to");

	CHECK_MESSAGE(document->GetElementById("aislots") != nullptr, "the setup lists the AI opponents inline");
	CHECK_MESSAGE(rmlui_dynamic_cast<UIRmlSurfaceElementClass *>(document->GetElementById("preview")) != nullptr, "the setup holds a surface for the map preview");

	Drive(presenter, "aiadd");
	CHECK_MESSAGE(presenter.State.AIPlayers == 3, "adding an opponent raises the count");
	CHECK_MESSAGE(presenter.State.AI.size() == 3, "and adds a slot");
	Drive(presenter, "aisub");
	CHECK_MESSAGE(presenter.State.AIPlayers == 2, "removing an opponent lowers the count");
	CHECK_MESSAGE(presenter.State.AI.size() == 2, "and drops the slot");

	Drive(presenter, "aiside", 1, "1");
	CHECK_MESSAGE(presenter.State.AI[1].Side == 1, "a slot takes the side chosen for it");
	Drive(presenter, "aicolor", 1, "0");
	CHECK_MESSAGE(presenter.State.AI[1].Color == 0, "and its colour");
	Drive(presenter, "aidiff", 1, "2");
	CHECK_MESSAGE(presenter.State.AI[1].Level == 2, "and its difficulty");
	Drive(presenter, "aiteam", 1, "3");
	CHECK_MESSAGE(presenter.State.AI[1].Team == 3, "and its team");

	Drive(presenter, "team", 2);
	CHECK_MESSAGE(presenter.State.Team == 2, "the player takes the team chosen for it");

	Drive(presenter, "map");
	REQUIRE_MESSAGE(!presenter.Result.has_value(), "the map button runs the map dialog over the setup, which stays open");
	CHECK_MESSAGE(service.Picked == 1, "the map button runs the map dialog over the setup, which stays open");
	CHECK_MESSAGE(presenter.State.MapName == "Picked map", "and the setup takes the map that came back");

	Drive(presenter, "options");
	CHECK_MESSAGE(service.Options == 1, "the options button runs the options dialog over the setup");

	service.Startable = false;
	Drive(presenter, "ok");
	REQUIRE_MESSAGE(!presenter.Result.has_value(), "a map that cannot hold the players leaves the setup open");
	CHECK_MESSAGE(service.Asked == 1, "a map that cannot hold the players leaves the setup open");
	service.Startable = true;
	Drive(presenter, "ok");
	REQUIRE_MESSAGE(presenter.Result.has_value(), "and one that can starts the game");
	CHECK_MESSAGE(presenter.Choice == UI_SKIRMISH_START, "and one that can starts the game");

	view->Release();
	context.Update();
}


void Test_Skirmish_Options(Rml::Context & context, CountingSystemInterfaceClass & system)
{
	int problems = system.Problems;

	UISkirmishState state;
	state.UnitCountMin = 0;
	state.UnitCountMax = 10;
	state.UnitCount = 4;
	state.CreditsMin = 2500;
	state.CreditsMax = 10000;
	state.CreditsStep = 250;
	state.Credits = 5000;
	state.TechLevelMax = 10;
	state.TechLevel = 5;

	UISkirmishOptionsPresenterClass presenter(state);
	std::unique_ptr<UIViewClass> view = UI_Skirmish_Options_View(presenter);

	CHECK_MESSAGE(Rml(*view).Prepare(context), "the options view prepares against the test context");
	view->Show(false);
	view->Sync();
	context.Update();
	context.Render();
	CHECK_MESSAGE(system.Problems == problems, "the options screen raises no RmlUi warning or error");

	Drive(presenter, "short", 1);
	CHECK_MESSAGE(state.ShortGame, "asking for a short game brings bases back");
	CHECK_MESSAGE(state.Bases, "asking for a short game brings bases back");
	Drive(presenter, "bases", 0);
	CHECK_MESSAGE(!state.Bases, "taking bases away ends the short game");
	CHECK_MESSAGE(!state.ShortGame, "taking bases away ends the short game");

	Drive(presenter, "credits", 99999);
	CHECK_MESSAGE(state.Credits == 10000, "a reading past its bound is held at it");

	Drive(presenter, "ok");
	REQUIRE_MESSAGE(presenter.Result.has_value(), "OK accepts the options");
	CHECK_MESSAGE(*presenter.Result == UI_RESULT_ACCEPTED, "OK accepts the options");

	view->Release();
	context.Update();
}


static std::vector<std::uint8_t> Build_Strike(unsigned int charset, unsigned int first, int height = 4, int ascent = 3)
{
	int const widths[2] = {3, 9};
	std::size_t const name = 130;
	std::size_t const picture = 134;
	std::size_t const second = picture + (std::size_t)((widths[0] + 7) / 8) * height;
	std::size_t const end = second + (std::size_t)((widths[1] + 7) / 8) * height;

	std::vector<std::uint8_t> fnt(end, 0);
	auto word = [&fnt](std::size_t at, unsigned int value) {
		fnt[at] = (std::uint8_t)(value & 0xFF);
		fnt[at + 1] = (std::uint8_t)(value >> 8);
	};

	word(0x00, 0x200);
	word(0x44, 8);
	word(0x4A, (unsigned int)ascent);
	fnt[0x55] = (std::uint8_t)charset;
	word(0x58, (unsigned int)height);
	fnt[0x5F] = (std::uint8_t)first;
	fnt[0x60] = (std::uint8_t)(first + 1);
	word(0x69, (unsigned int)name);
	word(0x6B, 0);

	word(0x76, (unsigned int)widths[0]);
	word(0x78, (unsigned int)picture);
	word(0x7A, (unsigned int)widths[1]);
	word(0x7C, (unsigned int)second);
	word(0x7E, 0);
	word(0x80, (unsigned int)end);

	fnt[name] = 'T';
	fnt[name + 1] = 0;
	fnt[picture] = 0xA0;
	fnt[second + (std::size_t)height] = 0x80;
	return(fnt);
}


static std::vector<std::uint8_t> Build_Font_Image(std::vector<std::vector<std::uint8_t>> const & fnts)
{
	std::vector<std::size_t> starts;
	std::size_t at = 0x100;
	for (std::vector<std::uint8_t> const & fnt : fnts) {
		starts.push_back(at);
		at += ((fnt.size() + 15) / 16) * 16;
	}

	std::vector<std::uint8_t> image(at, 0);
	auto word = [&image](std::size_t place, unsigned int value) {
		image[place] = (std::uint8_t)(value & 0xFF);
		image[place + 1] = (std::uint8_t)(value >> 8);
	};

	image[0] = 'M';
	image[1] = 'Z';
	word(0x3C, 0x40);
	image[0x40] = 'N';
	image[0x41] = 'E';
	word(0x40 + 0x24, 0x40);

	word(0x80, 4);
	word(0x82, 0x8008);
	word(0x84, (unsigned int)fnts.size());

	std::size_t entry = 0x8A;
	for (std::size_t index = 0; index < fnts.size(); index++) {
		std::size_t padded = ((fnts[index].size() + 15) / 16) * 16;
		word(entry, (unsigned int)(starts[index] >> 4));
		word(entry + 2, (unsigned int)(padded >> 4));
		std::memcpy(image.data() + starts[index], fnts[index].data(), fnts[index].size());
		entry += 12;
	}
	word(entry, 0);
	return(image);
}


static std::vector<std::uint8_t> Build_Font_Image(std::vector<std::uint8_t> const & fnt)
{
	return(Build_Font_Image(std::vector<std::vector<std::uint8_t>>{fnt}));
}


static bool Read_Strike(std::vector<std::uint8_t> const & fnt, UIRasterStrike & strike)
{
	return(UI_Read_Raster_Strike(std::span<std::uint8_t const>(fnt.data(), fnt.size()), strike));
}


void Test_Font_Shadow(Rml::Context & context)
{
	UIRasterStrike strike;
	CHECK_MESSAGE(Read_Strike(Build_Strike(0, 'M'), strike), "a strike reads for the shadow test");

	UIFontEngineClass engine;
	engine.Set_Fallback(nullptr);
	CHECK_MESSAGE(engine.Load_Strikes("T", { strike }), "the engine takes the strike as a face");
	Rml::FontFaceHandle face = engine.GetFontFaceHandle("T", Rml::Style::FontStyle::Normal, Rml::Style::FontWeight::Normal, 4);
	CHECK_MESSAGE(face != 0, "the strike's height finds its face");

	Rml::ElementDocument * document = context.LoadDocumentFromMemory(
		"<rml><body style='font-family: dlg-sans; font-size: 13px;'><p id='shadowed' style='font-effect: shadow(2px 3px #ff0000);'>M</p>"
		"<p id='outlined' style='font-effect: outline(1px #00ff00);'>M</p></body></rml>");
	CHECK_MESSAGE(document != nullptr, "the effect document loads");
	if (document == nullptr || face == 0) {
		return;
	}

	Rml::Property const * shadowed = document->GetElementById("shadowed")->GetProperty(Rml::PropertyId::FontEffect);
	Rml::Property const * outlined = document->GetElementById("outlined")->GetProperty(Rml::PropertyId::FontEffect);
	Rml::FontEffectsPtr shadow = shadowed != nullptr ? shadowed->Get<Rml::FontEffectsPtr>() : nullptr;
	Rml::FontEffectsPtr outline = outlined != nullptr ? outlined->Get<Rml::FontEffectsPtr>() : nullptr;
	REQUIRE_MESSAGE(shadow != nullptr, "both effects parse");
	CHECK_MESSAGE(outline != nullptr, "both effects parse");

	if (shadow != nullptr && outline != nullptr) {
		Rml::FontEffectsHandle effects = engine.PrepareFontEffects(face, shadow->list);
		CHECK_MESSAGE(effects != 0, "a shadow prepares on a game face, once for the same effects");
		CHECK_MESSAGE(engine.PrepareFontEffects(face, shadow->list) == effects, "a shadow prepares on a game face, once for the same effects");
		CHECK_MESSAGE(engine.PrepareFontEffects(face, outline->list) == 0, "an outline, which needs glyphs of its own, leaves a game face plain");

		Rml::TexturedMeshList meshes;
		Rml::String const language;
		Rml::TextShapingContext shaping{ language };
		Rml::ColourbPremultiplied white(255, 255, 255, 255);
		int const width = engine.GenerateString(context.GetRenderManager(), face, effects, "M", Rml::Vector2f(10.0f, 20.0f), white, 1.0f, shaping, meshes);
		CHECK_MESSAGE(width == 3, "a shadowed line draws the shadow and the text as two layers");
		CHECK_MESSAGE(meshes.size() == 2, "a shadowed line draws the shadow and the text as two layers");
		if (meshes.size() == 2 && !meshes[0].mesh.vertices.empty() && !meshes[1].mesh.vertices.empty()) {
			Rml::Vertex const & under = meshes[0].mesh.vertices[0];
			Rml::Vertex const & text = meshes[1].mesh.vertices[0];
			CHECK_MESSAGE(under.position.x == text.position.x + 2.0f, "the shadow sits at its offset from the text");
			CHECK_MESSAGE(under.position.y == text.position.y + 3.0f, "the shadow sits at its offset from the text");
			CHECK_MESSAGE(under.colour.red == 255, "the shadow takes its own color and the text keeps the text color");
			CHECK_MESSAGE(under.colour.green == 0, "the shadow takes its own color and the text keeps the text color");
			CHECK_MESSAGE(text.colour.green == 255, "the shadow takes its own color and the text keeps the text color");
		}

		engine.GenerateString(context.GetRenderManager(), face, effects, "M", Rml::Vector2f(10.0f, 40.0f), white, 1.0f, shaping, meshes);
		REQUIRE_MESSAGE(meshes.size() == 2, "a second line adds to the same two layers");
		CHECK_MESSAGE(meshes[0].mesh.vertices.size() == meshes[1].mesh.vertices.size(), "a second line adds to the same two layers");
	}

	context.UnloadDocument(document);
	context.Update();
}


void Test_Raster_Font(void)
{
	char32_t const CYRILLIC_A = 0x0410;
	char32_t const CYRILLIC_BE = 0x0411;
	char32_t const EURO = 0x20AC;

	std::vector<std::uint8_t> fnt = Build_Strike(0, 'A');

	UIRasterStrike strike;
	CHECK_MESSAGE(Read_Strike(fnt, strike), "a strike reads");
	CHECK_MESSAGE(strike.Points == 8, "a strike carries its size and its baseline");
	CHECK_MESSAGE(strike.Height == 4, "a strike carries its size and its baseline");
	CHECK_MESSAGE(strike.Ascent == 3, "a strike carries its size and its baseline");
	CHECK_MESSAGE(strike.Face == "T", "a strike names its face");
	CHECK_MESSAGE(strike.Advance(U'A') == 3, "a character's advance is its own width");
	CHECK_MESSAGE(strike.Advance(U'B') == 9, "a character's advance is its own width");
	CHECK_MESSAGE(strike.Advance(U'C') == 0, "a character the strike does not carry has no width");
	CHECK_MESSAGE(strike.Find(U'C') == nullptr, "a character the strike does not carry has no width");
	REQUIRE_MESSAGE(strike.Find(U'A') != nullptr, "a glyph's first row comes out of the top bits of its first column");
	REQUIRE_MESSAGE(strike.Find(U'A')->Coverage.size() == 12, "a glyph's first row comes out of the top bits of its first column");
	CHECK_MESSAGE(strike.Find(U'A')->Coverage[0] == 255, "a glyph's first row comes out of the top bits of its first column");
	CHECK_MESSAGE(strike.Find(U'A')->Coverage[1] == 0, "a glyph's first row comes out of the top bits of its first column");
	CHECK_MESSAGE(strike.Find(U'A')->Coverage[2] == 255, "a glyph's first row comes out of the top bits of its first column");
	REQUIRE_MESSAGE(strike.Find(U'B') != nullptr, "a glyph wider than eight reads its ninth pixel from a second column");
	CHECK_MESSAGE(strike.Find(U'B')->Coverage[8] == 255, "a glyph wider than eight reads its ninth pixel from a second column");
	CHECK_MESSAGE(strike.Find(U'B')->Coverage[0] == 0, "a glyph wider than eight reads its ninth pixel from a second column");

	std::vector<std::uint8_t> clipped(fnt.begin(), fnt.begin() + 140);
	UIRasterStrike refused;
	CHECK_MESSAGE(!UI_Read_Raster_Strike(std::span<std::uint8_t const>(clipped.data(), clipped.size()), refused), "a strike cut short is refused");

	CHECK_MESSAGE(UI_Raster_Charset_Page(0) == 1252, "a charset byte names its code page");
	CHECK_MESSAGE(UI_Raster_Charset_Page(238) == 1250, "a charset byte names its code page");
	CHECK_MESSAGE(UI_Raster_Charset_Page(204) == 1251, "a charset byte names its code page");
	CHECK_MESSAGE(UI_Raster_Charset_Page(161) == 1253, "a charset byte names its code page");
	CHECK_MESSAGE(UI_Raster_Charset_Page(162) == 1254, "a charset byte names its code page");
	CHECK_MESSAGE(UI_Raster_Charset_Page(186) == 1257, "a charset byte names its code page");
	CHECK_MESSAGE(UI_Raster_Charset_Page(177) == 1252, "a charset the engine has no table for is read as the page the dialogs were drawn in");
	CHECK_MESSAGE(UI_Raster_Charset_Page(99) == 1252, "a charset the engine has no table for is read as the page the dialogs were drawn in");

	UIRasterStrike cyrillic;
	CHECK_MESSAGE(Read_Strike(Build_Strike(204, 0xC0), cyrillic), "a strike in another code page reads");
	CHECK_MESSAGE(cyrillic.Advance(CYRILLIC_A) == 3, "its characters are the ones its page shows");
	CHECK_MESSAGE(cyrillic.Advance(CYRILLIC_BE) == 9, "its characters are the ones its page shows");
	CHECK_MESSAGE(cyrillic.Advance((char32_t)0xC0) == 0, "the byte itself carries nothing");

	UIRasterStrike ascii;
	CHECK_MESSAGE(Read_Strike(Build_Strike(204, 'A'), ascii), "the low row is the same character whatever the page");
	CHECK_MESSAGE(ascii.Advance(U'A') == 3, "the low row is the same character whatever the page");

	UIRasterStrike undefined;
	CHECK_MESSAGE(Read_Strike(Build_Strike(0, 0x80), undefined), "a byte its page leaves undefined is dropped and the one beside it is kept");
	REQUIRE_MESSAGE(undefined.Glyphs.size() == 1, "a byte its page leaves undefined is dropped and the one beside it is kept");
	CHECK_MESSAGE(undefined.Advance(EURO) == 3, "a byte its page leaves undefined is dropped and the one beside it is kept");

	UIRasterStrike control;
	CHECK_MESSAGE(Read_Strike(Build_Strike(0, 0x1F), control), "a character below a space is dropped");
	REQUIRE_MESSAGE(control.Glyphs.size() == 1, "a character below a space is dropped");
	CHECK_MESSAGE(control.Advance(U' ') == 9, "a character below a space is dropped");

	for (UIRasterStrike const & sorted : {strike, cyrillic}) {
		bool ascending = true;
		for (std::size_t index = 1; index < sorted.Glyphs.size(); index++) {
			ascending = ascending && sorted.Glyphs[index - 1].Code < sorted.Glyphs[index].Code;
		}
		CHECK_MESSAGE(ascending, "a strike holds its glyphs in code order, each code once");
	}

	std::vector<std::uint8_t> image = Build_Font_Image(fnt);
	std::vector<UIRasterStrike> strikes;
	CHECK_MESSAGE(UI_Read_Raster_Font(std::span<std::uint8_t const>(image.data(), image.size()), strikes), "a font image gives up the strikes it carries");
	REQUIRE_MESSAGE(strikes.size() == 1, "a font image gives up the strikes it carries");
	CHECK_MESSAGE(strikes[0].Advance(U'B') == 9, "a font image gives up the strikes it carries");

	std::vector<UIRasterStrike> twice;
	std::vector<std::uint8_t> pair = Build_Font_Image({Build_Strike(0, 'A'), Build_Strike(204, 0xC0)});
	CHECK_MESSAGE(UI_Read_Raster_Font(std::span<std::uint8_t const>(pair.data(), pair.size()), twice), "a file carrying a second strike of a height it already gave up is read once");
	REQUIRE_MESSAGE(twice.size() == 1, "a file carrying a second strike of a height it already gave up is read once");
	CHECK_MESSAGE(twice[0].Advance(U'A') == 3, "a file carrying a second strike of a height it already gave up is read once");
	CHECK_MESSAGE(twice[0].Advance(CYRILLIC_A) == 0, "a file carrying a second strike of a height it already gave up is read once");

	image[1] = 'X';
	CHECK_MESSAGE(!UI_Read_Raster_Font(std::span<std::uint8_t const>(image.data(), image.size()), strikes), "bytes that are not a sixteen-bit image give up nothing");
	CHECK_MESSAGE(strikes.empty(), "bytes that are not a sixteen-bit image give up nothing");

	std::vector<UIRasterStrike> family;
	family.push_back(strike);
	CHECK_MESSAGE(UI_Merge_Raster_Strikes(family, {cyrillic}) == 2, "a merge adds the characters the other cut carries");
	REQUIRE_MESSAGE(family.size() == 1, "a merge adds the characters the other cut carries");
	CHECK_MESSAGE(family[0].Advance(U'A') == 3, "a merge adds the characters the other cut carries");
	CHECK_MESSAGE(family[0].Advance(CYRILLIC_A) == 3, "a merge adds the characters the other cut carries");

	std::vector<std::uint8_t> altered = Build_Strike(0, 'A');
	altered[134] = 0x40;
	UIRasterStrike other;
	CHECK_MESSAGE(Read_Strike(altered, other), "a second cut of the same characters reads");

	std::vector<UIRasterStrike> mine;
	mine.push_back(strike);
	CHECK_MESSAGE(UI_Merge_Raster_Strikes(mine, {other}) == 0, "the strike read first keeps a character both carry");
	CHECK_MESSAGE(mine[0].Find(U'A')->Coverage[0] == 255, "the strike read first keeps a character both carry");
	CHECK_MESSAGE(mine[0].Find(U'A')->Coverage[1] == 0, "the strike read first keeps a character both carry");

	std::vector<UIRasterStrike> reversed;
	reversed.push_back(other);
	CHECK_MESSAGE(UI_Merge_Raster_Strikes(reversed, {strike}) == 0, "which cut wins follows the order they are merged in, not which ran last");
	CHECK_MESSAGE(reversed[0].Find(U'A')->Coverage[0] == 0, "which cut wins follows the order they are merged in, not which ran last");
	CHECK_MESSAGE(reversed[0].Find(U'A')->Coverage[1] == 255, "which cut wins follows the order they are merged in, not which ran last");

	UIRasterStrike taller;
	CHECK_MESSAGE(Read_Strike(Build_Strike(204, 0xC0, 5), taller), "a strike of another height reads");
	std::vector<UIRasterStrike> unmatched;
	unmatched.push_back(strike);
	CHECK_MESSAGE(UI_Merge_Raster_Strikes(unmatched, {taller}) == 0, "a cut whose height is not already there is left out rather than added beside it");
	CHECK_MESSAGE(unmatched.size() == 1, "a cut whose height is not already there is left out rather than added beside it");

	UIRasterStrike raised;
	CHECK_MESSAGE(Read_Strike(Build_Strike(204, 0xC0, 4, 2), raised), "a strike on another baseline reads");
	std::vector<UIRasterStrike> mismatched;
	mismatched.push_back(strike);
	CHECK_MESSAGE(UI_Merge_Raster_Strikes(mismatched, {raised}) == 0, "a cut putting its baseline elsewhere at the same height is left out");
	CHECK_MESSAGE(mismatched[0].Advance(CYRILLIC_A) == 0, "a cut putting its baseline elsewhere at the same height is left out");

	std::vector<UIRasterCell> cells;
	int width = 0;
	int height = 0;
	CHECK_MESSAGE(UI_Raster_Strike_Layout(family[0], UI_RASTER_ATLAS_LIMIT, cells, width, height), "every glyph of a strike gets a cell");
	CHECK_MESSAGE(cells.size() == family[0].Glyphs.size(), "every glyph of a strike gets a cell");
	CHECK_MESSAGE(width <= UI_RASTER_ATLAS_LIMIT, "the atlas stays inside the limit and is whole rows of the strike");
	CHECK_MESSAGE(height <= UI_RASTER_ATLAS_LIMIT, "the atlas stays inside the limit and is whole rows of the strike");
	CHECK_MESSAGE(height % family[0].Height == 0, "the atlas stays inside the limit and is whole rows of the strike");

	bool placed = true;
	for (std::size_t index = 0; index < cells.size(); index++) {
		placed = placed && cells[index].Code == family[0].Glyphs[index].Code
			&& cells[index].X >= 0 && cells[index].X + cells[index].Advance <= width
			&& cells[index].Y >= 0 && cells[index].Y + family[0].Height <= height;
		for (std::size_t earlier = 0; earlier < index; earlier++) {
			bool apart = cells[index].Y != cells[earlier].Y
				|| cells[index].X + cells[index].Advance <= cells[earlier].X
				|| cells[earlier].X + cells[earlier].Advance <= cells[index].X;
			placed = placed && apart;
		}
	}
	CHECK_MESSAGE(placed, "the cells sit inside the atlas in the strike's order without overlapping");

	CHECK_MESSAGE(!UI_Raster_Strike_Layout(family[0], 2, cells, width, height), "a strike that cannot be laid inside the limit is refused");
	CHECK_MESSAGE(cells.empty(), "a strike that cannot be laid inside the limit is refused");

	UIRasterStrike nothing;
	CHECK_MESSAGE(!UI_Raster_Strike_Layout(nothing, UI_RASTER_ATLAS_LIMIT, cells, width, height), "a strike carrying nothing has no atlas");
}


void Test_Surface_Element(Rml::Context & context, RecordingRenderInterfaceClass & render, CountingSystemInterfaceClass & system)
{
	std::uint16_t const source[] = { 0x0000, 0xFFFF, 0xF800, 0x07E0, 0x001F, 0x8410 };
	std::vector<std::uint8_t> rgba;

	bool converted = UI_Hicolor_To_RGBA(std::span<std::uint16_t const>(source, 6), 2, 3, 2, rgba);
	REQUIRE_MESSAGE(converted, "a 565 picture converts to four bytes a pixel");
	CHECK_MESSAGE(rgba.size() == 2 * 3 * 4, "a 565 picture converts to four bytes a pixel");

	if (converted && rgba.size() == 24) {
		std::uint8_t const expected[24] = {
			0, 0, 0, 255,        255, 255, 255, 255,
			255, 0, 0, 255,      0, 255, 0, 255,
			0, 0, 255, 255,      131, 129, 131, 255
		};
		CHECK_MESSAGE(std::memcmp(rgba.data(), expected, sizeof(expected)) == 0, "and widens each channel by repeating its top bits");
	}

	std::uint16_t const pitched[] = { 0xF800, 0x07E0, 0x0000, 0x0000, 0x001F, 0xFFFF };
	CHECK_MESSAGE(UI_Hicolor_To_RGBA(std::span<std::uint16_t const>(pitched, 6), 2, 2, 4, rgba), "a pitch wider than the picture is followed");
	REQUIRE_MESSAGE(rgba.size() == 16, "a pitch wider than the picture is followed");
	CHECK_MESSAGE(rgba[8] == 0, "a pitch wider than the picture is followed");
	CHECK_MESSAGE(rgba[10] == 255, "a pitch wider than the picture is followed");
	CHECK_MESSAGE(!UI_Hicolor_To_RGBA(std::span<std::uint16_t const>(pitched, 5), 2, 2, 4, rgba), "a picture short of its own size converts to nothing");
	CHECK_MESSAGE(rgba.empty(), "a picture short of its own size converts to nothing");

	int x = -1;
	int y = -1;
	int width = -1;
	int height = -1;
	CHECK_MESSAGE(UI_Surface_Fit(200, 100, 100, 100, x, y, width, height), "a wide picture fits the width of its box and centers down it");
	CHECK_MESSAGE(x == 0, "a wide picture fits the width of its box and centers down it");
	CHECK_MESSAGE(y == 25, "a wide picture fits the width of its box and centers down it");
	CHECK_MESSAGE(width == 100, "a wide picture fits the width of its box and centers down it");
	CHECK_MESSAGE(height == 50, "a wide picture fits the width of its box and centers down it");
	CHECK_MESSAGE(UI_Surface_Fit(100, 200, 100, 100, x, y, width, height), "a tall picture fits the height and centers across");
	CHECK_MESSAGE(x == 25, "a tall picture fits the height and centers across");
	CHECK_MESSAGE(y == 0, "a tall picture fits the height and centers across");
	CHECK_MESSAGE(width == 50, "a tall picture fits the height and centers across");
	CHECK_MESSAGE(height == 100, "a tall picture fits the height and centers across");
	CHECK_MESSAGE(UI_Surface_Fit(50, 50, 100, 100, x, y, width, height), "a picture in proportion with its box fills it");
	CHECK_MESSAGE(x == 0, "a picture in proportion with its box fills it");
	CHECK_MESSAGE(y == 0, "a picture in proportion with its box fills it");
	CHECK_MESSAGE(width == 100, "a picture in proportion with its box fills it");
	CHECK_MESSAGE(height == 100, "a picture in proportion with its box fills it");
	CHECK_MESSAGE(UI_Surface_Fit(3, 1, 10, 10, x, y, width, height), "a picture the box does not divide fills the side that runs out and centers on the other");
	CHECK_MESSAGE(x == 0, "a picture the box does not divide fills the side that runs out and centers on the other");
	CHECK_MESSAGE(y == 3, "a picture the box does not divide fills the side that runs out and centers on the other");
	CHECK_MESSAGE(width == 10, "a picture the box does not divide fills the side that runs out and centers on the other");
	CHECK_MESSAGE(height == 3, "a picture the box does not divide fills the side that runs out and centers on the other");
	CHECK_MESSAGE(!UI_Surface_Fit(0, 10, 100, 100, x, y, width, height), "a picture of nothing is not placed");
	CHECK_MESSAGE(width == 0, "a picture of nothing is not placed");
	CHECK_MESSAGE(height == 0, "a picture of nothing is not placed");

	std::uint8_t const pair[8] = { 10, 20, 30, 255, 200, 210, 220, 255 };
	std::vector<std::uint8_t> scaled;
	CHECK_MESSAGE(UI_Scale_RGBA_Nearest(std::span<std::uint8_t const>(pair, 8), 2, 1, 4, 2, scaled), "a picture stretches by repeating whole pixels");
	REQUIRE_MESSAGE(scaled.size() == 32, "a picture stretches by repeating whole pixels");
	CHECK_MESSAGE(scaled[0] == 10, "a picture stretches by repeating whole pixels");
	CHECK_MESSAGE(scaled[4] == 10, "a picture stretches by repeating whole pixels");
	CHECK_MESSAGE(scaled[8] == 200, "a picture stretches by repeating whole pixels");
	CHECK_MESSAGE(scaled[12] == 200, "a picture stretches by repeating whole pixels");
	CHECK_MESSAGE(std::memcmp(scaled.data(), scaled.data() + 16, 16) == 0, "a picture stretches by repeating whole pixels");
	std::uint8_t const four[16] = { 1, 0, 0, 255, 2, 0, 0, 255, 3, 0, 0, 255, 4, 0, 0, 255 };
	CHECK_MESSAGE(UI_Scale_RGBA_Nearest(std::span<std::uint8_t const>(four, 16), 4, 1, 2, 1, scaled), "and shrinks by dropping them");
	REQUIRE_MESSAGE(scaled.size() == 8, "and shrinks by dropping them");
	CHECK_MESSAGE(scaled[0] == 2, "and shrinks by dropping them");
	CHECK_MESSAGE(scaled[4] == 4, "and shrinks by dropping them");
	CHECK_MESSAGE(!UI_Scale_RGBA_Nearest(std::span<std::uint8_t const>(pair, 8), 2, 1, 0, 1, scaled), "a picture is not stretched to nothing");
	CHECK_MESSAGE(scaled.empty(), "a picture is not stretched to nothing");

	char const * markup =
		"<rml><head><style>"
		"body { width: 400px; height: 300px; }"
		"surface { display: block; width: 120px; height: 80px; }"
		"</style></head><body><surface id=\"picture\"/></body></rml>";

	int rendered = render.Rendered;
	int generated = render.Generated;
	int unsupported = render.Unsupported;
	int problems = system.Problems;

	Rml::ElementDocument * document = context.LoadDocumentFromMemory(markup);
	CHECK_MESSAGE(document != nullptr, "a document holding a surface loads");
	if (document == nullptr) {
		return;
	}

	document->Show();
	context.Update();
	context.Render();
	CHECK_MESSAGE(render.Rendered == rendered, "a surface with no picture draws nothing and makes no texture");
	CHECK_MESSAGE(render.Generated == generated, "a surface with no picture draws nothing and makes no texture");

	UIRmlSurfaceElementClass * surface = rmlui_dynamic_cast<UIRmlSurfaceElementClass *>(document->GetElementById("picture"));
	CHECK_MESSAGE(surface != nullptr, "the surface tag instances the element the shell registered");

	if (surface != nullptr) {
		surface->Set_Image(4, 2, std::vector<std::uint8_t>(4 * 2 * 4, 255));
		context.Update();
		context.Render();
		CHECK_MESSAGE(render.Generated > generated, "a surface handed pixels makes a texture of them");
		CHECK_MESSAGE(render.Rendered > rendered, "and draws them");

		rendered = render.Rendered;
		surface->Set_Image(0, 0, std::vector<std::uint8_t>());
		context.Update();
		context.Render();
		CHECK_MESSAGE(render.Rendered == rendered, "a surface whose picture is taken away draws nothing again");
	}

	CHECK_MESSAGE(render.Unsupported == unsupported, "a surface stays within the implemented render methods");
	CHECK_MESSAGE(system.Problems == problems, "a surface raises no RmlUi warning or error");

	document->Close();
	context.Update();
}


void Restate_Pass(UIRestatePresenterClass & presenter, UIViewClass & view, Rml::Context & context)
{
	presenter.Refresh();
	presenter.Drain();
	view.Sync();
	context.Update();
	view.Placed();
}


void Test_Restate_Screen(Rml::Context & context, CountingSystemInterfaceClass & system, RecordingRenderInterfaceClass & render)
{
	int problems = system.Problems;

	{
		RecordingRestateServiceClass service;
		FakeClockClass clock;
		UIRestatePresenterClass presenter(service, clock, Restate_Lines(16), true);
		std::unique_ptr<UIViewClass> view = UI_Restate_View(presenter);

		CHECK_MESSAGE(Rml(*view).Prepare(context), "the objectives view prepares against the test context");
		view->Show(true);
		context.Update();
		view->Placed();
		CHECK_MESSAGE(presenter.LaidOut, "the screen lays its briefing out once the page's font is known");
		REQUIRE_MESSAGE(presenter.Pages.size() == 2, "the screen lays its briefing out once the page's font is known");
		CHECK_MESSAGE(presenter.Pages[0].size() == 14, "the screen lays its briefing out once the page's font is known");

		Restate_Pass(presenter, *view, context);
		Rml::ElementDocument * document = Rml(*view).Document();
		Rml::Element * page = document->GetElementById("page");
		CHECK_MESSAGE(page->GetNumChildren() == 14, "the page holds a line for each row, hidden until its turn");
		CHECK_MESSAGE(page->GetChild(0)->GetComputedValues().visibility() == Rml::Style::Visibility::Hidden, "the page holds a line for each row, hidden until its turn");
		CHECK_MESSAGE(Visible_Buttons(document).empty(), "no button shows while the page fades in");
		CHECK_MESSAGE(document->GetElementById("skip")->IsVisible(), "no button shows while the page fades in");

		clock.Now = 144;
		Restate_Pass(presenter, *view, context);
		Rml::Element * first = page->GetChild(0);
		CHECK_MESSAGE(first->GetComputedValues().visibility() == Rml::Style::Visibility::Visible, "the first line comes up as a white blob");
		CHECK_MESSAGE(first->GetComputedValues().color() == Rml::Colourb(252, 252, 252, 255), "the first line comes up as a white blob");
		CHECK_MESSAGE(page->GetChild(1)->GetComputedValues().visibility() == Rml::Style::Visibility::Hidden, "the first line comes up as a white blob");

		int unsupported = render.Unsupported;
		context.Render();
		CHECK_MESSAGE(render.Unsupported == unsupported, "a line drawn as a blob stays within the implemented render methods");

		Click(context, document->GetElementById("skip"));
		Restate_Pass(presenter, *view, context);
		std::vector<Rml::Element *> buttons = Visible_Buttons(document);
		CHECK_MESSAGE(presenter.More, "a click settles the page in the side's ink and offers More");
		CHECK_MESSAGE(page->GetChild(13)->GetComputedValues().color() == Rml::Colourb(32, 212, 96, 255), "a click settles the page in the side's ink and offers More");
		REQUIRE_MESSAGE(buttons.size() == 1, "a click settles the page in the side's ink and offers More");
		CHECK_MESSAGE(buttons[0]->GetId() == "more", "a click settles the page in the side's ink and offers More");

		Press(context, Rml::Input::KI_SPACE);
		Restate_Pass(presenter, *view, context);
		CHECK_MESSAGE(presenter.Page == 1, "Space puts up the next page");
		CHECK_MESSAGE(presenter.Typing, "Space puts up the next page");
		CHECK_MESSAGE(page->GetNumChildren() == 2, "Space puts up the next page");

		Press(context, Rml::Input::KI_RETURN);
		Restate_Pass(presenter, *view, context);
		buttons = Visible_Buttons(document);
		CHECK_MESSAGE(presenter.Done, "Enter settles the last page, and Resume and Video come up side by side");
		REQUIRE_MESSAGE(buttons.size() == 2, "Enter settles the last page, and Resume and Video come up side by side");
		CHECK_MESSAGE(buttons[0]->GetId() == "resume", "Enter settles the last page, and Resume and Video come up side by side");
		CHECK_MESSAGE(buttons[1]->GetId() == "video", "Enter settles the last page, and Resume and Video come up side by side");

		Click(context, document->GetElementById("video"));
		presenter.Drain();
		REQUIRE_MESSAGE(presenter.Result.has_value(), "Video asks for the briefing movie");
		CHECK_MESSAGE(presenter.ChoseVideo, "Video asks for the briefing movie");

		view->Release();
		context.Update();
	}

	{
		RecordingRestateServiceClass service;
		FakeClockClass clock;
		UIRestatePresenterClass presenter(service, clock, Restate_Lines(2), false);
		std::unique_ptr<UIViewClass> view = UI_Restate_View(presenter);

		Rml(*view).Prepare(context);
		view->Show(true);
		context.Update();
		view->Placed();
		Restate_Pass(presenter, *view, context);

		Rml::ElementDocument * document = Rml(*view).Document();
		float const box = document->GetElementById("box")->GetAbsoluteOffset(Rml::BoxArea::Border).y;
		float const page = document->GetElementById("page")->GetAbsoluteOffset(Rml::BoxArea::Border).y;
		CHECK_MESSAGE(page - box == 120.0f, "a one-page briefing sits in the middle of the text box");

		Press(context, Rml::Input::KI_ESCAPE);
		Restate_Pass(presenter, *view, context);
		std::vector<Rml::Element *> buttons = Visible_Buttons(document);
		CHECK_MESSAGE(presenter.Done, "without a movie Resume stands alone");
		REQUIRE_MESSAGE(buttons.size() == 1, "without a movie Resume stands alone");
		CHECK_MESSAGE(buttons[0]->GetId() == "resume", "without a movie Resume stands alone");

		Press(context, Rml::Input::KI_ESCAPE);
		presenter.Drain();
		REQUIRE_MESSAGE(presenter.Result.has_value(), "Escape at the end resumes the mission");
		CHECK_MESSAGE(!presenter.ChoseVideo, "Escape at the end resumes the mission");

		view->Release();
		context.Update();
	}

	{
		RecordingRestateServiceClass service;
		FakeClockClass clock;
		UIRestatePresenterClass presenter(service, clock, Restate_Lines(2), false);
		std::unique_ptr<UIViewClass> view = UI_Restate_View(presenter);

		Rml(*view).Prepare(context);
		Rml::ElementDocument * document = Rml(*view).Document();
		CHECK_MESSAGE(!UI_Apply_Style_Sheet(*document, "side-none.rcss"), "a side with no style sheet leaves the screen as it was");
		CHECK_MESSAGE(UI_Apply_Style_Sheet(*document, "side-nod.rcss"), "a side's style sheet is added to a loaded screen");

		view->Show(true);
		context.Update();
		view->Placed();
		Restate_Pass(presenter, *view, context);
		clock.Now = 144;
		Restate_Pass(presenter, *view, context);
		clock.Now = 208;
		Restate_Pass(presenter, *view, context);

		Rml::Element * page = document->GetElementById("page");
		CHECK_MESSAGE(page->GetChild(0)->GetComputedValues().color() == Rml::Colourb(252, 180, 176, 255), "Nod's pale stage comes from its style sheet");

		Drive(presenter, "next");
		Restate_Pass(presenter, *view, context);
		CHECK_MESSAGE(page->GetChild(0)->GetComputedValues().color() == Rml::Colourb(252, 28, 28, 255), "and its settled text outranks the screen's own color");
		CHECK_MESSAGE(page->GetChild(0)->IsVisible(), "and its settled text outranks the screen's own color");

		view->Release();
		context.Update();
	}

	CHECK_MESSAGE(system.Problems == problems, "the objectives screen raises no RmlUi warning or error");
}


void Test_Effects_Documents(Rml::Context & context, RecordingRenderInterfaceClass & render, CountingSystemInterfaceClass & system)
{
	char const * rotated =
		"<rml><head><style>"
		"body { width: 400px; height: 300px; }"
		"#box { display: block; width: 100px; height: 60px; background-color: #ffffff; transform: rotate(10deg); }"
		"</style></head><body><div id=\"box\"/></body></rml>";

	char const * rounded =
		"<rml><head><style>"
		"body { width: 400px; height: 300px; }"
		"#clip { display: block; width: 100px; height: 60px; overflow: hidden; border-radius: 12px; background-color: #ffffff; }"
		"#inner { display: block; width: 200px; height: 200px; background-color: #ff0000; }"
		"</style></head><body><div id=\"clip\"><div id=\"inner\"/></div></body></rml>";

	struct {
		char const * Markup;
		bool Transform;
		char const * What;
	} const cases[] = {
		{ rotated, true, "a rotated element" },
		{ rounded, false, "a rounded element that clips its content" }
	};

	for (auto const & effect : cases) {
		int rendered = render.Rendered;
		int unsupported = render.Unsupported;
		int transforms = render.Transforms;
		int masks = render.ClipMasks;
		int problems = system.Problems;

		Rml::ElementDocument * document = context.LoadDocumentFromMemory(effect.Markup);
		std::string name = effect.What;
		CHECK_MESSAGE(document != nullptr, ((name + " loads").c_str()));
		if (document == nullptr) {
			continue;
		}

		document->Show();
		context.Update();
		context.Render();

		CHECK_MESSAGE(render.Rendered > rendered, ((name + " draws geometry").c_str()));
		CHECK_MESSAGE((effect.Transform ? render.Transforms > transforms : render.ClipMasks > masks), ((name + " asks the renderer for the effect it needs").c_str()));
		CHECK_MESSAGE(render.Unsupported == unsupported, ((name + " stays within the implemented render methods").c_str()));
		CHECK_MESSAGE(system.Problems == problems, ((name + " raises no RmlUi warning or error").c_str()));

		document->Close();
		context.Update();
	}

	CHECK_MESSAGE(!render.ClipMaskEnabled, "a document that clipped leaves no mask in force behind it");
}


// RmlUi set up the way the game sets it up, over interfaces that record what the documents ask
// of them. Every case gets its own, so a case run alone sees what it sees in the full run, and
// the shutdown at its end must release every geometry and texture the case compiled.
struct DocumentFixture
{
	RecordingRenderInterfaceClass render;
	CountingSystemInterfaceClass system;
	Rml::Context * context = nullptr;

	DocumentFixture(void)
	{
		std::filesystem::path directory(OPENTS_UI_DIR);
		std::filesystem::current_path(directory);

		Rml::SetRenderInterface(&render);
		Rml::SetSystemInterface(&system);
		REQUIRE_MESSAGE(Rml::Initialise(), "RmlUi initialises with the recording interfaces");

		UI_Register_Surface_Element();

		std::string shipped = (directory / "Arimo.ttf").string();
		REQUIRE_MESSAGE(Rml::LoadFontFace(shipped), "the shipped font loads");
		REQUIRE_MESSAGE(Rml::LoadFontFace(shipped, "dlg-sans", Rml::Style::FontStyle::Normal), "and stands in for the dialogs' sans family");
		REQUIRE_MESSAGE(Rml::LoadFontFace(shipped, "dlgsys", Rml::Style::FontStyle::Normal), "and for the bitmap family the art would supply");
		REQUIRE_MESSAGE(Rml::LoadFontFace(shipped, "fullfnt", Rml::Style::FontStyle::Normal), "and for the family the old menu font's screens use");

		context = Rml::CreateContext("test", Rml::Vector2i(1280, 800));
		REQUIRE_MESSAGE(context != nullptr, "a context is created");
	}

	~DocumentFixture(void)
	{
		Rml::RemoveContext("test");
		Rml::Shutdown();

		CHECK_MESSAGE(render.ReleasedGeometry == render.Compiled, "every compiled geometry is released by shutdown");
		CHECK_MESSAGE(render.ReleasedTextures == render.Loaded + render.Generated, "every texture is released by shutdown");
	}
};


TEST_CASE_FIXTURE(DocumentFixture, "every UI document names its resources by bare file name, loads, and draws cleanly within the context")
{
	std::filesystem::path directory(OPENTS_UI_DIR);

	int documents = 0;
	int templates = 0;
	for (std::filesystem::directory_entry const & entry : std::filesystem::directory_iterator(directory)) {
		std::filesystem::path path = entry.path();
		std::string extension = path.extension().string();

		if (extension == ".rml" || extension == ".rcss") {
			std::string what = path.filename().string() + " names its resources by bare file name";
			CHECK_MESSAGE(References_Are_Bare(Read_Text(path)), what.c_str());
		}

		if (extension == ".rml" && Read_Text(path).find("<template") != std::string::npos) {
			templates++;
			continue;
		}

		if (extension != ".rml" || context == nullptr) {
			continue;
		}

		documents++;
		int rendered = render.Rendered;
		int unsupported = render.Unsupported;
		int invalid = render.Invalid;
		int problems = system.Problems;
		render.Scissors.clear();

		std::string model = Data_Model_Name(Read_Text(path));
		if (!model.empty()) {
			Rml::DataModelConstructor constructor = context->CreateDataModel(model, nullptr, true);
			constructor.BindEventCallback("queue", [](Rml::DataModelHandle, Rml::Event &, Rml::VariantList const &) {});
		}

		Rml::ElementDocument * document = context->LoadDocument(path.string());
		std::string name = path.filename().string();
		CHECK_MESSAGE(document != nullptr, ((name + " loads").c_str()));
		if (document == nullptr) {
			continue;
		}

		document->Show();
		context->Update();
		context->Render();

		CHECK_MESSAGE(render.Rendered > rendered, ((name + " draws geometry").c_str()));
		CHECK_MESSAGE(render.Unsupported == unsupported, ((name + " stays within the implemented render methods").c_str()));
		CHECK_MESSAGE(render.Invalid == invalid, ((name + " compiles whole, in-range, finite geometry").c_str()));
		CHECK_MESSAGE(system.Problems == problems, ((name + " raises no RmlUi warning or error").c_str()));

		bool clipped = true;
		for (Rml::Rectanglei const & scissor : render.Scissors) {
			if (!scissor.Valid() || scissor.Left() < 0 || scissor.Top() < 0 || scissor.Right() > 1280 || scissor.Bottom() > 800) {
				clipped = false;
			}
		}
		CHECK_MESSAGE(clipped, ((name + " scissors within the context").c_str()));

		document->Close();
		context->Update();

		if (!model.empty()) {
			context->RemoveDataModel(model);
		}
	}

	CHECK_MESSAGE(documents > 0, "the ui directory holds at least one document");
	CHECK_MESSAGE(templates > 0, "and at least one template the documents are built on");
}


TEST_CASE_FIXTURE(DocumentFixture, "a document that clipped leaves no mask in force behind it")
{
	Test_Effects_Documents(*context, render, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "a raster font strike reads its size, baseline, face and glyphs")
{
	Test_Raster_Font();
}


TEST_CASE_FIXTURE(DocumentFixture, "a raster font draws its shadow under the text")
{
	Test_Font_Shadow(*context);
}


TEST_CASE_FIXTURE(DocumentFixture, "the surface element converts and draws a picture")
{
	Test_Surface_Element(*context, render, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the skirmish screen")
{
	Test_Skirmish_Screen(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the skirmish options screen")
{
	Test_Skirmish_Options(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the map screen")
{
	Test_Scenario_Screen(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the network game browser")
{
	Test_Net_Browser_Screen(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the network game setup screen")
{
	Test_Net_Setup_Screen(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the version screen")
{
	Test_Version_Screen(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the save game screen")
{
	Test_Save_Game_Screen(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the in-game options menu")
{
	Test_Game_Options_Screen(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the message box")
{
	Test_Message_Box_Screen(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the reconnect screen")
{
	Test_Reconnect_Screen(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the diplomacy screen")
{
	Test_Diplomacy_Screen(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the sound screen")
{
	Test_Sound_Screen(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the game controls screen")
{
	Test_Game_Controls_Screen(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the display screen")
{
	Test_Display_Screen(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the keyboard screen")
{
	Test_Keyboard_Screen(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the screens move the focus through their tab order by keyboard")
{
	Test_Keyboard_Navigation(*context, system, render);
}


TEST_CASE_FIXTURE(DocumentFixture, "a long list scrolls to keep its selection in view")
{
	Test_List_Scrolling(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the main menu")
{
	Test_Menu_Screen(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the options menu")
{
	Test_Main_Options_Screen(*context, system, render);
}


TEST_CASE_FIXTURE(DocumentFixture, "the wait box")
{
	Test_Wait_Box_Screen(*context, system);
}


TEST_CASE_FIXTURE(DocumentFixture, "the mission objectives screen")
{
	Test_Restate_Screen(*context, system, render);
}


class MissingFileInterfaceClass : public Rml::FileInterface
{
	public:
		std::string Missing;

		virtual Rml::FileHandle Open(Rml::String const & path) override
		{
			if (std::filesystem::path(path).filename().string() == Missing) {
				return(0);
			}
			return(reinterpret_cast<Rml::FileHandle>(std::fopen(path.c_str(), "rb")));
		}

		virtual void Close(Rml::FileHandle file) override
		{
			std::fclose(reinterpret_cast<FILE *>(file));
		}

		virtual size_t Read(void * buffer, size_t size, Rml::FileHandle file) override
		{
			return(std::fread(buffer, 1, size, reinterpret_cast<FILE *>(file)));
		}

		virtual bool Seek(Rml::FileHandle file, long offset, int origin) override
		{
			return(std::fseek(reinterpret_cast<FILE *>(file), offset, origin) == 0);
		}

		virtual size_t Tell(Rml::FileHandle file) override
		{
			return((size_t)std::ftell(reinterpret_cast<FILE *>(file)));
		}
};


void Test_Missing_UI_Files(ShellFixtureType & fixture, MissingFileInterfaceClass & reader)
{
	UIShellClass & shell = fixture.Shell;
	for (char const * name : { "menu.rml", "dialog.rml", "menu.rcss", "kit.rcss", "Arimo.ttf" }) {
		reader.Missing = name;
		std::size_t const errors = fixture.Host.Errors.size();
		CHECK_MESSAGE(shell.Init(), "the shell starts for a missing-resource test");

		UIMenuState state;
		state.Items.push_back({ "Exit", 1, true });
		UIMenuPresenterClass presenter(state);
		std::unique_ptr<UIViewClass> view = UI_Menu_View(presenter);
		int passes = 0;
		UIResult result = shell.Run_Modal(*view, [&]() { passes++; return(true); });
		CHECK_MESSAGE(result == UI_RESULT_FAILED_TO_OPEN, "a missing required resource refuses the menu before its service runs");
		CHECK_MESSAGE(passes == 0, "a missing required resource refuses the menu before its service runs");
		REQUIRE_MESSAGE(fixture.Host.Errors.size() == errors + 1, "the visible error names the missing resource");
		CHECK_MESSAGE(fixture.Host.Errors.back().find(name) != std::string::npos, "the visible error names the missing resource");
		CHECK_MESSAGE(!view->Is_Shown(), "the failed menu leaves no shown view, modal or data model");
		CHECK_MESSAGE(shell.Modal_Depth() == 0, "the failed menu leaves no shown view, modal or data model");
		CHECK_MESSAGE(!shell.Rml_Context()->GetDataModel("menu"), "the failed menu leaves no shown view, modal or data model");
		shell.Rml_Context()->Update();
		CHECK_MESSAGE(shell.Rml_Context()->GetNumDocuments() == 0, "the failed menu leaves no document after cleanup");

		reader.Missing.clear();
		if (std::strcmp(name, "Arimo.ttf") == 0) {
			shell.Shutdown();
			CHECK_MESSAGE(shell.Init(), "the shell restarts after restoring the font");
		}
		result = shell.Run_Modal(*view, [&]() { passes++; return(true); });
		CHECK_MESSAGE(result == UI_RESULT_SESSION_ENDED, "restoring the resource allows the menu to open without another error");
		CHECK_MESSAGE(passes == 1, "restoring the resource allows the menu to open without another error");
		CHECK_MESSAGE(fixture.Host.Errors.size() == errors + 1, "restoring the resource allows the menu to open without another error");
		shell.Shutdown();
	}
}


void Test_Button_Clicks(UIShellClass & shell, TestHostClass & host)
{
	for (float ratio : { 1.0f, 2.0f }) {
		for (char const * kind : { "", "wide", "tall" }) {
			for (int point = 0; point < 4; point++) {
				std::printf("  button clicks: ratio %.0f, kind %s, point %d\n", ratio, *kind != '\0' ? kind : "standard", point);
				UIVersionPresenterClass presenter({ "button clicks" });
				std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
				int passes = 0;
				int clicks = host.Clicks;
				PointType at {};
				bool stable = false;
				bool queued = false;
				bool pressed = false;
				UIResult result = shell.Run_Modal(*view, [&](void) {
					passes++;
					Rml::Element * button = Rml(*view).Document()->GetElementById("ok");
					if (passes == 1) {
						host.Rect.ScaleX = ratio;
						host.Rect.ScaleY = ratio;
						shell.On_Video_Change();
						if (*kind != '\0') {
							button->SetClass(kind, true);
						}
						shell.Tick();
						Rml::Vector2f origin = button->GetAbsoluteOffset(Rml::BoxArea::Border);
						Rml::Vector2f size = button->GetBox().GetSize(Rml::BoxArea::Border);
						at = { (int)(origin.x + size.x * 0.5f), (int)std::ceil(origin.y) + 1 };
						if (point == 1 || point == 3) {
							at.Y = (int)(origin.y + size.y * 0.5f);
						} else if (point == 2) {
							at.Y = (int)(origin.y + size.y) - 1;
						}
						Press(shell, WINDOW_BUTTON_LEFT, at);
						shell.Tick();
						pressed = button->IsPseudoClassSet("active");
						stable = button->GetAbsoluteOffset(Rml::BoxArea::Border) == origin
							&& button->GetBox().GetSize(Rml::BoxArea::Border) == size;
					} else if (passes == 2) {
						if (point == 3) {
							at.Y -= (int)(40.0f * ratio);
						}
						Release(shell, WINDOW_BUTTON_LEFT, at);
						queued = presenter.Has_Pending();
					}
					return(passes >= 3);
				});
				REQUIRE_MESSAGE(pressed, "a button press shows feedback and sounds once");
				CHECK_MESSAGE(host.Clicks == clicks + 1, "a button press shows feedback and sounds once");
				CHECK_MESSAGE(stable, "pressing a button keeps its hit area in place across a frame");
				CHECK_MESSAGE((point == 3 ? !queued && result == UI_RESULT_SESSION_ENDED : queued && result == UI_RESULT_ACCEPTED), "releasing inside the original button activates it; dragging outside cancels");
			}
		}
	}
	host.Rect.ScaleX = 1.0f;
	host.Rect.ScaleY = 1.0f;
	shell.On_Video_Change();
}


TEST_CASE("the shell runs modal screens over its host, routes their input, and survives missing UI files")
{
	// The shell reads its documents from the working directory, as the game reads them beside it.
	std::filesystem::current_path(std::filesystem::path(OPENTS_UI_DIR));

	auto files = std::make_unique<MissingFileInterfaceClass>();
	MissingFileInterfaceClass & reader = *files;
	ShellFixtureType fixture(std::move(files));
	UIShellClass & shell = fixture.Shell;
	TestHostClass & host = fixture.Host;

	CHECK_MESSAGE(shell.Init(), "the shell initializes over the injected interfaces");
	CHECK_MESSAGE(shell.Rml_Context() != nullptr, "the shell holds a context");
	CHECK_MESSAGE(!shell.Screen_Shown(), "no screen is shown at start");
	CHECK_MESSAGE(shell.Modal() == nullptr, "no screen is shown at start");
	CHECK_MESSAGE(shell.Modal_Depth() == 0, "no screen is shown at start");

	{
		UIVersionPresenterClass presenter({ "one", "two" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
		int passes = 0;
		bool shownInside = false;
		bool consumedWhileOpening = false;
		int presents = host.Presents;

		host.OnClear = [&](void) {
			if (host.Clears == 1) {
				consumedWhileOpening = Press(shell, WINDOW_BUTTON_LEFT, PointType{ 10, 10 });
			}
		};

		UIResult result = shell.Run_Modal(*view, [&](void) {
			passes++;
			if (passes == 1) {
				shownInside = shell.Screen_Shown() && shell.Modal() == view.get() && shell.Modal_Depth() == 1;
			}
			if (passes == 3) {
				Key_Down(shell, VK_RETURN);
			}
			return(false);
		});
		host.OnClear = nullptr;

		CHECK_MESSAGE(result == UI_RESULT_ACCEPTED, "Enter accepts the modal");
		CHECK_MESSAGE(passes == 3, "the runner stops on the pass that produced the result");
		CHECK_MESSAGE(host.Presents - presents == 2, "the runner presents after each pass that continues");
		CHECK_MESSAGE(shownInside, "the modal is the shown screen while the service runs");
		CHECK_MESSAGE(consumedWhileOpening, "a press pumped while the screen opens is consumed");
		CHECK_MESSAGE(host.Clears == 2, "the keyboard queue is cleared at open and at close");
		CHECK_MESSAGE(host.Focuses == 1, "focus returns to the main window once");
		CHECK_MESSAGE(!shell.Screen_Shown(), "the modal stack is empty after the close");
		CHECK_MESSAGE(shell.Modal() == nullptr, "the modal stack is empty after the close");
	}

	Test_Button_Clicks(shell, host);

	{
		UIVersionPresenterClass presenter({ "escape" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
		UIResult result = shell.Run_Modal(*view, [&](void) {
			Key_Down(shell, VK_ESCAPE);
			return(false);
		});
		CHECK_MESSAGE(result == UI_RESULT_CANCELLED, "Escape cancels the modal");
	}

	{
		UIVersionPresenterClass presenter({ "ended" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
		int passes = 0;
		UIResult result = shell.Run_Modal(*view, [&](void) {
			passes++;
			return(true);
		});
		CHECK_MESSAGE(result == UI_RESULT_SESSION_ENDED, "a service reporting the game ended closes the modal at once");
		CHECK_MESSAGE(passes == 1, "a service reporting the game ended closes the modal at once");
		CHECK_MESSAGE(!shell.Screen_Shown(), "a modal ended by the game leaves nothing shown");
	}

	{
		UIVersionPresenterClass outer({ "outer" });
		std::unique_ptr<UIViewClass> outerview = UI_Version_View(outer);
		int outerPasses = 0;
		std::optional<UIResult> inner;
		UIResult result = shell.Run_Modal(*outerview, [&](void) {
			outerPasses++;
			if (outerPasses == 1) {
				UIMessageBoxPresenterClass presenter("inner", { "OK" }, 0);
				std::unique_ptr<UIViewClass> view = UI_Message_Box_View(presenter);
				inner = shell.Run_Modal(*view, [](void) { return(true); });
				return(false);
			}
			return(true);
		});
		REQUIRE_MESSAGE(inner.has_value(), "a game ending under a nested screen closes each runner on its next pass");
		CHECK_MESSAGE(*inner == UI_RESULT_SESSION_ENDED, "a game ending under a nested screen closes each runner on its next pass");
		CHECK_MESSAGE(result == UI_RESULT_SESSION_ENDED, "a game ending under a nested screen closes each runner on its next pass");
		CHECK_MESSAGE(outerPasses == 2, "a game ending under a nested screen closes each runner on its next pass");
		CHECK_MESSAGE(!shell.Screen_Shown(), "and leaves nothing shown to take the score screen's key");
		CHECK_MESSAGE(!Key_Down(shell, VK_SPACE), "and leaves nothing shown to take the score screen's key");
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
				Key_Down(shell, VK_ESCAPE);
			}
			return(false);
		});
		fixture.Render->OnRender = nullptr;

		REQUIRE_MESSAGE(resized, "a resize arriving inside a render is deferred");
		CHECK_MESSAGE(unchangedInside, "a resize arriving inside a render is deferred");
		CHECK_MESSAGE(appliedBefore, "the deferred resize is applied before the next tick");

		host.Rect = { 0, 0, 1280, 800, 1.0f, 1.0f };
		shell.On_Video_Change();
		CHECK_MESSAGE(shell.Rml_Context()->GetDimensions() == Rml::Vector2i(1280, 800), "a resize outside a render is applied at once");
	}

	{
		shell.On_Archives_Change(-1);
		CHECK_MESSAGE(shell.Side_Sheet().empty(), "no side style sheet applies before a side is mounted");

		host.SideName = "Nod";
		shell.On_Archives_Change(1);
		CHECK_MESSAGE(shell.Side_Sheet() == "side-nod.rcss", "a mounted side names its style sheet after itself, in lower case");

		RecordingRestateServiceClass service;
		UIRestatePresenterClass presenter(service, shell.Clock(), "Line", false);
		std::unique_ptr<UIViewClass> view = UI_Restate_View(presenter);
		Rml::Colourb ink;
		int passes = 0;

		shell.Run_Modal(*view, [&](void) {
			passes++;
			UIIntent intent;
			if (presenter.Typing) {
				intent.Name = "next";
				presenter.Queue(intent);
			} else if (presenter.Done) {
				Rml::Element * page = Rml(*view).Document()->GetElementById("page");
				if (page != nullptr && page->GetNumChildren() > 0) {
					ink = page->GetChild(0)->GetComputedValues().color();
				}
				intent.Name = "resume";
				presenter.Queue(intent);
			}
			return(passes > 50);
		});
		CHECK_MESSAGE(ink == Rml::Colourb(252, 28, 28, 255), "a screen opened while Nod is mounted takes its colors from side-nod.rcss");

		host.SideName.clear();
		CHECK_MESSAGE(shell.Side_Sheet().empty(), "a side without a name has no style sheet");
	}

	{
		UIVersionPresenterClass presenter({ "archives" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
		int passes = 0;
		int released = 0;
		int fetched = 0;
		int lookups = 0;
		bool dropped = false;
		bool refetched = false;

		shell.Run_Modal(*view, [&](void) {
			passes++;
			if (passes == 1) {
				released = fixture.Render->ReleasedTextures;
				fetched = fixture.Render->Loaded + fixture.Render->Generated;
				lookups = UITestSheetLookups;
				shell.On_Archives_Change(0);
				dropped = fixture.Render->ReleasedTextures > released;
			}
			if (passes == 2) {
				refetched = fixture.Render->Loaded + fixture.Render->Generated > fetched;
				Key_Down(shell, VK_ESCAPE);
			}
			return(false);
		});

		CHECK_MESSAGE(dropped, "an archive change lets the art already loaded go");
		CHECK_MESSAGE(refetched, "a document fetches its art again over the archives now mounted");
		CHECK_MESSAGE(UITestSheetLookups > lookups, "the dialog font sheets are looked for again");

		int const problems = fixture.System->Problems;
		UIVersionPresenterClass after({ "reread" });
		std::unique_ptr<UIViewClass> afterview = UI_Version_View(after);
		bool built = false;

		shell.Run_Modal(*afterview, [&](void) {
			Rml::ElementDocument * document = Rml(*afterview).Document();
			built = document != nullptr && document->GetElementById("chrome") != nullptr;
			Key_Down(shell, VK_ESCAPE);
			return(false);
		});

		REQUIRE_MESSAGE(built, "a screen opened after the change still builds from its template");
		CHECK_MESSAGE(fixture.System->Problems == problems, "a screen opened after the change still builds from its template");
	}

	{
		UIVersionPresenterClass presenter({ "remount" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
		int passes = 0;
		int released = 0;
		bool asked = false;
		bool heldInside = false;
		bool appliedBefore = false;

		fixture.Render->OnRender = [&](void) {
			if (!asked) {
				asked = true;
				released = fixture.Render->ReleasedTextures;
				shell.On_Archives_Change(0);
				heldInside = fixture.Render->ReleasedTextures == released;
			}
		};

		shell.Run_Modal(*view, [&](void) {
			passes++;
			if (passes == 2) {
				appliedBefore = fixture.Render->ReleasedTextures > released;
				Key_Down(shell, VK_ESCAPE);
			}
			return(false);
		});
		fixture.Render->OnRender = nullptr;

		REQUIRE_MESSAGE(asked, "an archive change arriving inside a render is deferred");
		CHECK_MESSAGE(heldInside, "an archive change arriving inside a render is deferred");
		CHECK_MESSAGE(appliedBefore, "the deferred archive change is applied before the next tick");
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
				swallowed = Release(shell, WINDOW_BUTTON_LEFT, PointType{ 10, 10 }) && Key_Up(shell, 'A');
				host.Down[VK_LBUTTON] = false;
				host.Down['A'] = false;
				quiet = !presenter.Has_Pending() && shell.Input_State().Mouse_Owner(0) == UI_INPUT_NONE && shell.Input_State().Key_Owner('A') == UI_INPUT_NONE;
			}
			if (passes == 2) {
				Key_Down(shell, VK_RETURN);
			}
			return(false);
		});
		CHECK_MESSAGE(suppressed, "input held as a screen opens is suppressed");
		CHECK_MESSAGE(swallowed, "the releases of suppressed input are swallowed");
		REQUIRE_MESSAGE(quiet, "suppressed releases queue nothing and a fresh press still accepts");
		CHECK_MESSAGE(result == UI_RESULT_ACCEPTED, "suppressed releases queue nothing and a fresh press still accepts");
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
				Press(shell, WINDOW_BUTTON_LEFT, PointType{ 10, 10 });
				Key_Down(shell, 'A');
				owned = shell.Input_State().Mouse_Owner(0) == UI_INPUT_RML && shell.Input_State().Key_Owner('A') == UI_INPUT_RML && host.Captured;
				host.Captured = false;
				Capture_Lost(shell);
				cancelled = shell.Input_State().Mouse_Owner(0) == UI_INPUT_SUPPRESSED && shell.Input_State().Key_Owner('A') == UI_INPUT_RML && !host.Captured && shell.Modal() == view.get();
				swallowed = Release(shell, WINDOW_BUTTON_LEFT, PointType{ 10, 10 }) && !presenter.Has_Pending();
				Key_Up(shell, 'A');
			}
			if (passes == 2) {
				Key_Down(shell, VK_ESCAPE);
			}
			return(false);
		});
		CHECK_MESSAGE(owned, "a modal owns the presses it is given and takes the capture");
		CHECK_MESSAGE(cancelled, "losing the capture cancels the modal's held buttons and nothing else");
		CHECK_MESSAGE(swallowed, "the release of a cancelled press is swallowed");
	}

	{
		RecordingSoundServiceClass service;
		UISoundState state;
		state.Score = 5;
		state.Sound = 5;
		state.Voice = 5;
		state.Enabled = true;
		UISoundPresenterClass presenter(service, state);
		std::unique_ptr<UIViewClass> view = UI_Sound_View(presenter);
		int passes = 0;
		bool owned = false;
		bool dropped = false;
		bool still = false;

		shell.Run_Modal(*view, [&](void) {
			passes++;
			Rml::Element * score = Rml(*view).Document()->GetElementById("score");
			Rml::Element * bar = nullptr;
			Rml::Element * track = nullptr;
			if (passes == 2 && score != nullptr && Slider_Parts(score, bar, track)) {
				PointType const start = Element_Point(host, bar);
				Move(shell, start);
				host.Down[VK_LBUTTON] = true;
				Press(shell, WINDOW_BUTTON_LEFT, start);
				owned = shell.Input_State().Mouse_Owner(0) == UI_INPUT_RML && host.Captured;

				host.Down[VK_LBUTTON] = false;
				Focus(shell, false);
				dropped = shell.Input_State().Mouse_Owner(0) == UI_INPUT_SUPPRESSED && !host.Captured;
				Focus(shell, true);
				Move(shell, Past_Track_End(host, bar, track));
			}
			if (passes == 3 && score != nullptr) {
				still = presenter.State.Score == 5 && score->GetAttribute<int>("value", -1) == 5 && !presenter.Has_Pending();
				Key_Down(shell, VK_ESCAPE);
			}
			return(false);
		});
		Key_Up(shell, VK_ESCAPE);
		shell.Tick();

		CHECK_MESSAGE(owned, "a press on a slider's bar is the document's and takes the capture");
		CHECK_MESSAGE(dropped, "losing the application cancels the held button and gives the capture back");
		CHECK_MESSAGE(still, "a pointer that comes back without the button moves the slider nowhere");
	}

	{
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
						Press(shell, WINDOW_BUTTON_LEFT, PointType{ 10, 10 });
						Key_Down(shell, VK_ESCAPE);
					}
					return(innerpasses >= 5);
				});
				restored = innerresult == UI_RESULT_CANCELLED && shell.Modal() == outerview.get() && shell.Modal_Depth() == 1 && shell.Input_State().Mouse_Owner(0) == UI_INPUT_SUPPRESSED;
				swallowed = Release(shell, WINDOW_BUTTON_LEFT, PointType{ 10, 10 });
				host.Down[VK_LBUTTON] = false;
			}
			if (passes == 2) {
				Key_Down(shell, VK_RETURN);
			}
			return(false);
		});
		CHECK_MESSAGE(nested, "a nested modal is the shown screen at depth two");
		REQUIRE_MESSAGE(restored, "closing the inner modal restores the outer one, which still accepts");
		CHECK_MESSAGE(result == UI_RESULT_ACCEPTED, "closing the inner modal restores the outer one, which still accepts");
		CHECK_MESSAGE(swallowed, "a press held across the inner close is swallowed by the outer");
	}

	{
		UIVersionPresenterClass outer({ "serviced" });
		UIMessageBoxPresenterClass inner("Under it", { "OK" }, 0);
		std::unique_ptr<UIViewClass> outerview = UI_Version_View(outer);
		std::unique_ptr<UIViewClass> innerview = UI_Message_Box_View(inner);
		int passes = 0;
		int innerpasses = 0;
		bool outerseen = false;
		bool innerseen = false;
		bool outerback = false;

		UIServiceCallback innerservice = [&](void) {
			innerpasses++;
			if (innerpasses == 1) {
				innerseen = shell.Running_Service() == &innerservice;
				Key_Down(shell, VK_RETURN);
			}
			return(innerpasses >= 4);
		};
		UIServiceCallback outerservice = [&](void) {
			passes++;
			if (passes == 1) {
				outerseen = shell.Running_Service() == &outerservice;
				shell.Run_Modal(*innerview, innerservice);
				outerback = shell.Running_Service() == &outerservice;
			}
			if (passes == 2) {
				Key_Down(shell, VK_RETURN);
			}
			return(false);
		};

		CHECK_MESSAGE(shell.Running_Service() == nullptr, "no service runs while no screen is shown");
		shell.Run_Modal(*outerview, outerservice);
		Key_Up(shell, VK_RETURN);
		shell.Tick();
		REQUIRE_MESSAGE(outerseen, "each running screen reports the service driving it");
		CHECK_MESSAGE(innerseen, "each running screen reports the service driving it");
		REQUIRE_MESSAGE(outerback, "closing a screen hands the service back to the one below, and the last to nobody");
		CHECK_MESSAGE(shell.Running_Service() == nullptr, "closing a screen hands the service back to the one below, and the last to nobody");
	}

	{
		UIVersionPresenterClass outer({ "covered" });
		UIMessageBoxPresenterClass inner("Over it", { "OK" }, 0);
		std::unique_ptr<UIViewClass> outerview = UI_Version_View(outer);
		std::unique_ptr<UIViewClass> innerview = UI_Message_Box_View(inner);
		int passes = 0;
		bool shownbefore = false;
		bool hidden = true;
		bool shownafter = false;
		bool stillopen = false;

		shell.Run_Modal(*outerview, [&](void) {
			passes++;
			if (passes == 1) {
				shownbefore = outerview->Is_Shown();
				int innerpasses = 0;
				shell.Run_Modal(*innerview, [&](void) {
					innerpasses++;
					hidden = hidden && !outerview->Is_Shown();
					if (innerpasses == 1) {
						Key_Down(shell, VK_RETURN);
					}
					return(innerpasses >= 4);
				}, true);
				shownafter = outerview->Is_Shown();
				stillopen = !Rml(*outerview).Document()->IsClassSet("revealing") && !shell.Revealing_Shown();
			}
			if (passes == 2) {
				Key_Down(shell, VK_RETURN);
			}
			return(false);
		});
		REQUIRE_MESSAGE(shownbefore, "a screen is shown before and after the one it raises over itself");
		CHECK_MESSAGE(shownafter, "a screen is shown before and after the one it raises over itself");
		CHECK_MESSAGE(hidden, "and is hidden for every pass that one runs");
		CHECK_MESSAGE(stillopen, "and comes back without opening again");
	}

	{
		UIVersionPresenterClass presenter({ "messages" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
		bool taken = false;
		bool focused = false;

		shell.Run_Modal(*view, [&](void) {
			taken = Move(shell, PointType{ 2000, 2000 })
				&& Press(shell, WINDOW_BUTTON_X1, PointType{ 10, 10 })
				&& Release(shell, WINDOW_BUTTON_X1, PointType{ 10, 10 })
				&& Wheel(shell, PointType{ 10, 10 }, 1.0f, true);
			host.Down['C'] = true;
			Focus(shell, true);
			focused = shell.Input_State().Key_Owner('C') == UI_INPUT_SUPPRESSED;
			host.Down['C'] = false;
			Key_Up(shell, 'C');
			Key_Down(shell, VK_ESCAPE);
			return(false);
		});
		CHECK_MESSAGE(taken, "a modal takes moves, side buttons and the horizontal wheel");
		CHECK_MESSAGE(focused, "focus returning to a shown screen quarantines what is held");

		CHECK_MESSAGE(Key_Up(shell, VK_ESCAPE), "the release of the key that closed a screen is swallowed");
		shell.Tick();
		CHECK_MESSAGE(!shell.Input_State().Any_Owned(), "nothing stays owned once the closing key is up and the shell has ticked");

		host.Down['C'] = true;
		Focus(shell, true);
		CHECK_MESSAGE(shell.Input_State().Key_Owner('C') == UI_INPUT_NONE, "focus returning to an idle shell quarantines nothing");
		host.Down['C'] = false;
		CHECK_MESSAGE(!Key_Up(shell, 'Z'), "a stray release meets an idle shell and reaches the game");
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
			Text(shell, 0xE9);
			Rml(*view).Document()->RemoveEventListener(Rml::EventId::Textinput, &recorder);
			Key_Down(shell, VK_ESCAPE);
			return(false);
		});
		REQUIRE_MESSAGE(recorder.Texts.size() == 1, "a typed character reaches the document as its UTF-8 text");
		CHECK_MESSAGE(recorder.Texts[0] == "\xC3\xA9", "a typed character reaches the document as its UTF-8 text");
	}

	{
		UIVersionPresenterClass presenter({ "text" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
		Rml::String written;
		Rml::String repeated;
		Rml::String kept;
		bool focused = false;

		shell.Run_Modal(*view, [&](void) {
			Rml::ElementDocument * document = Rml(*view).Document();
			Rml::ElementFormControlTextArea * area = rmlui_dynamic_cast<Rml::ElementFormControlTextArea *>(document->AppendChild(document->CreateElement("textarea")));
			Rml::ElementFormControlInput * field = rmlui_dynamic_cast<Rml::ElementFormControlInput *>(document->AppendChild(document->CreateElement("input")));
			if (area != nullptr && field != nullptr) {
				area->Focus();
				Key_Down(shell, 'A');
				Text(shell, 'A');
				Key_Up(shell, 'A');
				Key_Down(shell, VK_RETURN);
				Key_Up(shell, VK_RETURN);
				Key_Down(shell, 'B');
				Text(shell, 'B');
				Key_Up(shell, 'B');
				written = area->GetValue();

				WindowEvent repeat;
				repeat.Type = WINDOW_EVENT_KEY_DOWN;
				repeat.VirtualKey = VK_RETURN;
				Key_Down(shell, VK_RETURN);
				repeat.Repeat = true;
				Send(shell, repeat);
				Key_Up(shell, VK_RETURN);
				repeated = area->GetValue();

				field->SetAttribute("type", "text");
				field->SetValue("Name");
				field->Focus();
				focused = document->GetContext()->GetFocusElement() == field;
				field->Select();
				Key_Down(shell, VK_RETURN);
				kept = field->GetValue();
				Key_Up(shell, VK_RETURN);
			}
			Key_Down(shell, VK_ESCAPE);
			return(false);
		});
		CHECK_MESSAGE(written == "A\nB", "Enter in a text area types a new line");
		CHECK_MESSAGE(repeated == "A\nB\n\n", "and each repeat of Enter types another");
		REQUIRE_MESSAGE(focused, "Enter in a one-line field leaves its selected text alone");
		CHECK_MESSAGE(kept == "Name", "Enter in a one-line field leaves its selected text alone");
	}

	{
		UISaveGameState state;
		state.Mode = UI_SAVE_GAME_SAVE;
		state.Title = "SAVE";
		state.AcceptCaption = "Save";
		UISaveGameEntry slot;
		slot.Description = "[EMPTY SLOT]";
		state.Entries.push_back(slot);

		UISaveGamePresenterClass presenter(state);
		std::unique_ptr<UIViewClass> view = UI_Save_Game_View(presenter);
		Rml::String typed;

		shell.Run_Modal(*view, [&](void) {
			Rml::ElementDocument * document = Rml(*view).Document();
			Rml::ElementFormControlInput * field = (document != nullptr)
				? rmlui_dynamic_cast<Rml::ElementFormControlInput *>(document->GetElementById("description")) : nullptr;
			if (field != nullptr) {
				field->SetValue("");
				field->Focus();
				Text(shell, 0x041F);
				Text(shell, 0x0440);
				Text(shell, 0x0438);
				Text(shell, 0x0432);
				Text(shell, 0x0435);
				Text(shell, 0x0442);
				typed = field->GetValue();
			}
			Key_Down(shell, VK_ESCAPE);
			return(false);
		});
		CHECK_MESSAGE(typed == "\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82", "Cyrillic typed into a field reaches it as the letters themselves");
	}

	{
		char const * sample = "\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80";

		Rml::String before;
		fixture.System->GetClipboardText(before);
		fixture.System->SetClipboardText(sample);
		Rml::String after;
		fixture.System->GetClipboardText(after);
		CHECK_MESSAGE(after == sample, "clipboard text goes through the host and survives a round trip");
		CHECK_MESSAGE(host.Clipboard == sample, "clipboard text goes through the host and survives a round trip");
		fixture.System->SetClipboardText(before);
	}

	{
		UIVersionPresenterClass presenter({ "cursor" });
		std::unique_ptr<UIViewClass> view = UI_Version_View(presenter);
		int applied = host.Applied;
		int restored = host.Restored;
		bool opened = false;
		bool arrow = false;
		bool requested = false;
		bool shown = false;

		shell.Run_Modal(*view, [&](void) {
			opened = host.Applied > applied && host.LastCursor == UI_CURSOR_ARROW && host.Restored == restored;
			arrow = shell.Handle_Set_Cursor() && host.LastCursor == UI_CURSOR_ARROW;
			fixture.System->SetMouseCursor("text");
			requested = fixture.System->Cursor_Request() == UI_CURSOR_TEXT;
			shown = shell.Handle_Set_Cursor() && host.LastCursor == UI_CURSOR_TEXT;
			Key_Down(shell, VK_ESCAPE);
			return(false);
		});
		CHECK_MESSAGE(opened, "opening a screen puts the window's arrow on the pointer before any request");
		CHECK_MESSAGE(arrow, "choosing the pointer over a shown screen is the screen's even with no request");
		CHECK_MESSAGE(requested, "a document's pointer request is kept");
		CHECK_MESSAGE(shown, "choosing the pointer shows the requested shape while a screen is shown");
		CHECK_MESSAGE(host.Restored - restored == 1, "closing a screen puts the game's pointer back once and forgets the request");
		CHECK_MESSAGE(fixture.System->Cursor_Request() == UI_CURSOR_ARROW, "closing a screen puts the game's pointer back once and forgets the request");
		CHECK_MESSAGE(host.LastCursor == UI_CURSOR_ARROW, "closing a screen puts the game's pointer back once and forgets the request");
		CHECK_MESSAGE(!shell.Handle_Set_Cursor(), "choosing the pointer is the game's again once nothing is shown");
	}

	{
		RecordingSoundServiceClass service;
		UISoundState state;
		state.Score = 5;
		state.Sound = 5;
		state.Voice = 5;
		state.Enabled = true;
		UISoundPresenterClass presenter(service, state);
		std::unique_ptr<UIViewClass> view = UI_Sound_View(presenter);
		int passes = 0;
		int dragged = -1;
		bool consumed = false;
		bool held = false;
		bool kept = false;

		shell.Run_Modal(*view, [&](void) {
			passes++;
			Rml::Element * score = Rml(*view).Document()->GetElementById("score");
			if (passes == 2 && score != nullptr) {
				service.Calls.clear();
				consumed = Drag_Slider_To_End(shell, host, score);
				dragged = score->GetAttribute<int>("value", -1);
			}
			if (passes == 3 && score != nullptr) {
				held = presenter.State.Score == dragged && score->GetAttribute<int>("value", -1) == dragged;
			}
			if (passes == 4 && score != nullptr) {
				kept = presenter.State.Score == dragged && score->GetAttribute<int>("value", -1) == dragged;
				Key_Down(shell, VK_ESCAPE);
			}
			return(false);
		});
		Key_Up(shell, VK_ESCAPE);
		shell.Tick();

		REQUIRE_MESSAGE(consumed, "a drag to the end of the music slider reaches the document and lands on the top level");
		CHECK_MESSAGE(dragged == UISoundPresenterClass::LEVELS, "a drag to the end of the music slider reaches the document and lands on the top level");
		CHECK_MESSAGE(held, "the level a drag left is what the model holds after the pass");
		REQUIRE_MESSAGE(kept, "the level stays put on the next pass and previews once");
		CHECK_MESSAGE(service.Calls.size() == 1, "the level stays put on the next pass and previews once");
	}

	{
		UIMessageBoxPresenterClass presenter("Quarantine", { "OK", "Cancel", "" }, 0);
		std::unique_ptr<UIViewClass> view = UI_Message_Box_View(presenter);
		int passes = 0;
		bool consumed = false;
		bool unpressed = false;
		bool answered = false;

		shell.Run_Modal(*view, [&](void) {
			passes++;
			Rml::ElementList buttons;
			Rml(*view).Document()->GetElementsByTagName(buttons, "button");
			if (buttons.empty()) {
				return(true);
			}
			PointType at = Element_Point(host, buttons[0]);

			if (passes == 2) {
				host.Down[VK_LBUTTON] = true;
				Focus(shell, true);
				consumed = Press(shell, WINDOW_BUTTON_LEFT, at);
				unpressed = !buttons[0]->IsPseudoClassSet("active");
				host.Down[VK_LBUTTON] = false;
				Release(shell, WINDOW_BUTTON_LEFT, at);
			}
			if (passes == 3) {
				answered = !presenter.Result.has_value();
				Move(shell, at);
				Press(shell, WINDOW_BUTTON_LEFT, at);
				Release(shell, WINDOW_BUTTON_LEFT, at);
			}
			return(false);
		});

		CHECK_MESSAGE(consumed, "a press quarantined by the window activating is kept from the game");
		CHECK_MESSAGE(unpressed, "that press never reaches the document, so nothing is left pressed");
		CHECK_MESSAGE(answered, "and it answers nothing");
		REQUIRE_MESSAGE(presenter.Result.has_value(), "the next click on the same button answers normally");
		CHECK_MESSAGE(presenter.Choice == 0, "the next click on the same button answers normally");
	}

	{
		for (int loss = 0; loss < 2; loss++) {
			UIMessageBoxPresenterClass presenter("Delete it?", { "Yes", "No", "" }, 1);
			std::unique_ptr<UIViewClass> view = UI_Message_Box_View(presenter);
			int passes = 0;
			bool held = false;
			bool quiet = false;

			shell.Run_Modal(*view, [&](void) {
				passes++;
				Rml::ElementList buttons;
				Rml(*view).Document()->GetElementsByTagName(buttons, "button");
				if (buttons.empty()) {
					return(true);
				}
				PointType at = Element_Point(host, buttons[0]);

				if (passes == 2) {
					Move(shell, at);
					host.Down[VK_LBUTTON] = true;
					Press(shell, WINDOW_BUTTON_LEFT, at);
					held = shell.Input_State().Mouse_Owner(0) == UI_INPUT_RML;

					host.Down[VK_LBUTTON] = false;
					if (loss == 0) {
						Focus(shell, false);
						Focus(shell, true);
					} else {
						Capture_Lost(shell);
					}
				}
				if (passes == 3) {
					quiet = !presenter.Result.has_value() && !presenter.Has_Pending();
					Key_Down(shell, VK_ESCAPE);
				}
				return(false);
			});

			CHECK_MESSAGE(held, (loss == 0 ? "a press on the question's first button is the document's" : "and again for the capture being taken back"));
			CHECK_MESSAGE(quiet, (loss == 0 ? "losing the application over a held button answers nothing" : "and neither does the capture going"));
		}
	}

	{
		UIMainOptionsState state;
		state.SoundEnabled = false;
		state.Top = 120;
		UIMainOptionsPresenterClass presenter(state);
		std::unique_ptr<UIViewClass> view = UI_Main_Options_View(presenter);
		int passes = 0;
		bool found = false;
		bool refused = false;
		bool silent = false;
		int const clicks = host.Clicks;

		shell.Run_Modal(*view, [&](void) {
			passes++;
			Rml::ElementDocument * document = Rml(*view).Document();
			Rml::Element * sound = document->GetElementById("sound");
			Rml::Element * display = document->GetElementById("display");
			found = sound != nullptr && display != nullptr;
			if (!found) {
				return(true);
			}
			if (passes == 2) {
				Click_Through_Hook(shell, host, sound);
			}
			if (passes == 3) {
				refused = !presenter.Result.has_value();
				silent = host.Clicks == clicks;
				Click_Through_Hook(shell, host, display);
			}
			return(passes > 4);
		});

		REQUIRE_MESSAGE(found, "a click on the refused sound button answers nothing and leaves the menu open");
		REQUIRE_MESSAGE(refused, "a click on the refused sound button answers nothing and leaves the menu open");
		CHECK_MESSAGE(passes == 3, "a click on the refused sound button answers nothing and leaves the menu open");
		REQUIRE_MESSAGE(presenter.Result.has_value(), "a click reaches the button beneath the pointer and answers with its choice");
		CHECK_MESSAGE(presenter.Choice == UI_MAIN_OPTIONS_DISPLAY, "a click reaches the button beneath the pointer and answers with its choice");
		REQUIRE_MESSAGE(silent, "a press sounds the dialogs' click, and a disabled button sounds nothing");
		CHECK_MESSAGE(host.Clicks == clicks + 1, "a press sounds the dialogs' click, and a disabled button sounds nothing");
	}

	{
		UIMainOptionsState state;
		state.SoundEnabled = true;
		UIMainOptionsPresenterClass presenter(state);
		std::unique_ptr<UIViewClass> view = UI_Main_Options_View(presenter);

		std::size_t const played = host.Samples.size();
		host.Animate = true;

		std::chrono::steady_clock::time_point began = std::chrono::steady_clock::now();
		auto Waited = [&](void) {
			return((int)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - began).count());
		};

		int passes = 0;
		float narrowest = 1000.0f;
		float opened = -1.0f;
		float chromeleft = -1.0f;
		float chromewidth = -1.0f;
		bool anchored = true;
		bool stepped = true;
		bool clipped = true;

		shell.Run_Modal(*view, [&](void) {
			passes++;
			Rml::Element * dialog = Rml(*view).Document()->GetElementById("reveal");
			if (dialog == nullptr) {
				return(true);
			}

			float width = dialog->GetBox().GetSize().x;
			if (passes > 1 && width < narrowest) {
				narrowest = width;
			}
			if (passes > 2 && width - opened > 24.5f) {
				stepped = false;
			}
			opened = width;

			if (passes > 2 && width < 300.0f) {
				if (fixture.Render->Unclipped > 0) {
					clipped = false;
				}
				for (Rml::Rectanglei const & scissor : fixture.Render->Scissors) {
					if (scissor.Width() > (int)width + 1) {
						clipped = false;
					}
				}
			}
			fixture.Render->Unclipped = 0;
			fixture.Render->Scissors.clear();

			Rml::Element * chrome = Rml(*view).Document()->GetElementById("chrome");
			if (chrome != nullptr && passes > 1) {
				float left = chrome->GetAbsoluteOffset(Rml::BoxArea::Border).x;
				if (chromeleft >= 0.0f && std::fabs(left - chromeleft) > 0.5f) {
					anchored = false;
				}
				chromeleft = left;
				chromewidth = chrome->GetBox().GetSize().x;
			}
			return(Waited() > 3000 || (passes > 2 && width >= 300.0f));
		});

		int waited = Waited();
		host.Animate = false;

		CHECK_MESSAGE(narrowest > 0.0f, "a screen is mostly hidden behind its band when it opens");
		CHECK_MESSAGE(narrowest < 300.0f, "a screen is mostly hidden behind its band when it opens");
		CHECK_MESSAGE(anchored, "and what is inside the band holds still while the band widens around it");
		CHECK_MESSAGE(stepped, "which widens by one step a pass at most");
		CHECK_MESSAGE(clipped, "and nothing draws outside the band while it opens");
		CHECK_MESSAGE(chromewidth == 300.0f, "so the screen is uncovered rather than squeezed out to its full width");
		CHECK_MESSAGE(opened >= 300.0f, "and the band has let the whole of it out in the quarter second the schedule takes");
		CHECK_MESSAGE(waited >= 250, "and the band has let the whole of it out in the quarter second the schedule takes");
		CHECK_MESSAGE(waited < 3000, "and the band has let the whole of it out in the quarter second the schedule takes");
		REQUIRE_MESSAGE(host.Samples.size() == played + 1, "opening it sounds once, as the dialog layer sounds it");
		CHECK_MESSAGE(host.Samples.back() == "EMBLEM.AUD", "opening it sounds once, as the dialog layer sounds it");
	}

	{
		for (int hide = 0; hide < 2; hide++) {
			UIVersionPresenterClass outer({ "opening" });
			UIMessageBoxPresenterClass inner("Over it", { "OK", "", "" }, 0);
			std::unique_ptr<UIViewClass> outerview = UI_Version_View(outer);
			std::unique_ptr<UIViewClass> innerview = UI_Message_Box_View(inner);
			int passes = 0;
			bool opening = false;
			bool whole = false;
			bool stayed = false;

			host.Animate = true;
			shell.Run_Modal(*outerview, [&](void) {
				passes++;
				Rml::ElementDocument * document = Rml(*outerview).Document();
				Rml::Element * dialog = document->GetElementById("reveal");
				if (dialog == nullptr) {
					return(true);
				}

				Rml::Element * chrome = document->GetElementById("chrome");
				auto uncovered = [&](void) {
					return(!document->IsClassSet("revealing") && chrome != nullptr && dialog->GetBox().GetSize().x >= chrome->GetBox().GetSize().x);
				};

				if (passes == 1) {
					opening = shell.Revealing_Shown() && !uncovered();

					int innerpasses = 0;
					shell.Run_Modal(*innerview, [&](void) {
						innerpasses++;
						if (innerpasses == 1) {
							Key_Down(shell, VK_RETURN);
						}
						return(false);
					}, hide != 0);
					whole = uncovered();
				}
				if (passes == 3) {
					stayed = uncovered();
					Key_Down(shell, VK_ESCAPE);
				}
				return(false);
			});
			host.Animate = false;

			CHECK_MESSAGE(opening, (hide == 0 ? "a screen still behind its band can have another raised over it" : "and can have one raised that hides it"));
			CHECK_MESSAGE(whole, (hide == 0 ? "which puts the screen underneath out whole" : "and so does one that hid it"));
			CHECK_MESSAGE(stayed, (hide == 0 ? "and leaves it out once the band it took is gone" : "and leaves a hidden one out when it comes back"));
		}
	}

	{
		RecordingDisplayServiceClass service;
		UIDisplayPresenterClass presenter(service, Display_Fixture());
		std::unique_ptr<UIViewClass> view = UI_Display_View(presenter);
		int passes = 0;
		bool listed = false;
		bool marked = false;

		shell.Run_Modal(*view, [&](void) {
			passes++;
			Rml::ElementDocument * document = Rml(*view).Document();
			std::vector<Rml::Element *> rows = Visible_Rows(document, "modes");
			Rml::Element * ok = document->GetElementById("ok");
			listed = rows.size() == 3 && ok != nullptr;
			if (!listed) {
				return(true);
			}
			if (passes == 2) {
				Click_Through_Hook(shell, host, rows[2]);
			}
			if (passes == 3) {
				marked = rows[2]->IsClassSet("selected") && !rows[1]->IsClassSet("selected");
				Click_Through_Hook(shell, host, ok);
			}
			return(passes > 4);
		});

		REQUIRE_MESSAGE(listed, "clicking a mode row marks that row chosen in the document");
		CHECK_MESSAGE(marked, "clicking a mode row marks that row chosen in the document");
		CHECK_MESSAGE(presenter.State.Selected == 2, "and accepting hands back the mode the row carried, not its index");
		REQUIRE_MESSAGE(presenter.Picked.has_value(), "and accepting hands back the mode the row carried, not its index");
		CHECK_MESSAGE(presenter.Picked->Width == 1920, "and accepting hands back the mode the row carried, not its index");
		CHECK_MESSAGE(presenter.Picked->Height == 1080, "and accepting hands back the mode the row carried, not its index");
	}

	{
		RecordingGameControlsServiceClass service;
		UIGameControlsState state = Game_Controls_Fixture();
		state.InGame = true;
		state.SoundEnabled = true;
		UIGameControlsPresenterClass presenter(service, state);
		std::unique_ptr<UIViewClass> view = UI_Game_Controls_View(presenter);
		int passes = 0;
		bool found = false;
		bool dragged = false;
		bool quiet = false;
		std::string named;

		shell.Run_Modal(*view, [&](void) {
			passes++;
			Rml::ElementDocument * document = Rml(*view).Document();
			Rml::Element * cameo = document->GetElementById("cameo");
			Rml::Element * speed = document->GetElementById("speed");
			Rml::Element * name = document->GetElementById("speed-name");
			Rml::Element * sound = document->GetElementById("sound");
			found = cameo != nullptr && speed != nullptr && name != nullptr && sound != nullptr;
			if (!found) {
				return(true);
			}
			if (passes == 2) {
				Click_Through_Hook(shell, host, cameo);
				dragged = Drag_Slider_To_End(shell, host, speed);
			}
			if (passes == 3) {
				named = name->GetInnerRML();
				quiet = service.Calls.empty();
				Click_Through_Hook(shell, host, sound);
			}
			return(passes > 4);
		});

		REQUIRE_MESSAGE(found, "the speed slider runs backwards, so dragging it to the far end takes the fastest setting");
		REQUIRE_MESSAGE(dragged, "the speed slider runs backwards, so dragging it to the far end takes the fastest setting");
		CHECK_MESSAGE(presenter.State.Speed == 0, "the speed slider runs backwards, so dragging it to the far end takes the fastest setting");
		CHECK_MESSAGE(named == "Fastest", "the speed slider runs backwards, so dragging it to the far end takes the fastest setting");
		CHECK_MESSAGE(!presenter.State.CameoText, "a switch and a slider change the screen without reaching the engine");
		CHECK_MESSAGE(quiet, "a switch and a slider change the screen without reaching the engine");
		CHECK_MESSAGE(service.Joined() == "speed 0; scroll 2; detail 1; cameo off; lines off; tooltips on; coasting off; edge off; difficulty 2; save", "the next-screen button applies every setting in one order and saves");
		REQUIRE_MESSAGE(presenter.Result.has_value(), "and asks for the screen its button names");
		CHECK_MESSAGE(presenter.Next == UIGameControlsPresenterClass::NEXT_SOUND, "and asks for the screen its button names");
	}

	{
		RecordingKeyboardServiceClass service;
		UIKeyboardPresenterClass presenter(service, Keyboard_Fixture());
		std::unique_ptr<UIViewClass> view = UI_Keyboard_View(presenter);
		int passes = 0;
		bool listed = false;
		bool focused = false;
		bool captured = false;
		bool modified = false;
		bool moved = false;

		shell.Run_Modal(*view, [&](void) {
			passes++;
			Rml::ElementDocument * document = Rml(*view).Document();
			std::vector<Rml::Element *> rows = Keyboard_Rows(document);
			Rml::Element * capture = document->GetElementById("capture");
			Rml::Element * assign = document->GetElementById("assign");
			Rml::Element * ok = document->GetElementById("ok");
			listed = rows.size() == 4 && capture != nullptr && assign != nullptr && ok != nullptr;
			if (!listed) {
				return(true);
			}

			if (passes == 2) {
				Click_Through_Hook(shell, host, rows[2]);
			}
			if (passes == 3) {
				focused = shell.Rml_Context()->GetFocusElement() == capture;
				Key_Down(shell, 'X');
				Key_Up(shell, 'X');
			}
			if (passes == 4) {
				captured = presenter.State.Captured == 88 && presenter.State.AssignedTo == "Scatter" && capture->GetInnerRML().find("K88") != std::string::npos;

				// The host reports no keys held, so Shift can only come from the events.
				Key_Down(shell, 'R', WINDOW_MOD_SHIFT);
				Key_Up(shell, 'R', WINDOW_MOD_SHIFT);
			}
			if (passes == 5) {
				modified = presenter.State.Captured == 338 && presenter.State.AssignedTo == "Toggle Repair";
				Click_Through_Hook(shell, host, assign);
			}
			if (passes == 6) {
				moved = presenter.Key_Of(3) == 338 && presenter.Key_Of(1) == 0;
				Click_Through_Hook(shell, host, ok);
			}
			return(passes > 7);
		});

		REQUIRE_MESSAGE(listed, "a click on a command row selects the command it names");
		CHECK_MESSAGE(presenter.State.Selected == 3, "a click on a command row selects the command it names");
		CHECK_MESSAGE(focused, "and moves the focus to the capture element the keys go to");
		CHECK_MESSAGE(captured, "a key sent through the hook becomes its hotkey number there and names the command holding it");
		CHECK_MESSAGE(modified, "the same key with a modifier on its event is a different number naming a different command");
		CHECK_MESSAGE(moved, "assigning it moves the key to the selected command and off its old owner");
		CHECK_MESSAGE(service.Calls == std::vector<std::string>{ "save 0=577 2=88 3=338" }, "and accepting saves the table the edits left behind");
	}

	{
		FakeClockClass clock;
		UIConfirmModePresenterClass presenter(clock);
		std::unique_ptr<UIViewClass> view = UI_Confirm_Mode_View(presenter);
		int passes = 0;
		int counted = -1;

		shell.Run_Modal(*view, [&](void) {
			passes++;
			if (passes == 2) {
				clock.Now = UIConfirmModePresenterClass::DEFAULT_TIMEOUT - 2000;
			}
			if (passes == 3) {
				counted = presenter.Seconds;
				clock.Now = UIConfirmModePresenterClass::DEFAULT_TIMEOUT;
			}
			return(passes > 5);
		});

		CHECK_MESSAGE(counted == 2, "the confirmation counts down as the runner refreshes it");
		REQUIRE_MESSAGE(presenter.Result.has_value(), "and silence closes it when the deadline passes, with no intent involved");
		CHECK_MESSAGE(*presenter.Result == UI_RESULT_CANCELLED, "and silence closes it when the deadline passes, with no intent involved");
		CHECK_MESSAGE(presenter.TimedOut, "and silence closes it when the deadline passes, with no intent involved");
		CHECK_MESSAGE(presenter.Seconds == 0, "and silence closes it when the deadline passes, with no intent involved");
	}

	{
		UIWaitBoxPresenterClass presenter("Working", false);
		std::unique_ptr<UIViewClass> view = UI_Wait_Box_View(presenter);

		int const presentsnow = host.PresentsNow;
		CHECK_MESSAGE(shell.Show_Modeless(*view), "a notice shows beside the game");
		CHECK_MESSAGE(shell.Is_Modeless_Shown(*view), "the shell lists the notice while it shows");
		CHECK_MESSAGE(view->Is_Shown(), "the shell lists the notice while it shows");
		CHECK_MESSAGE(!shell.Screen_Shown(), "a notice is not a screen");

		CHECK_MESSAGE(host.PresentsNow > presentsnow, "showing a notice presents at once rather than when the interval allows");

		int const hidden = host.PresentsNow;
		shell.Hide_Modeless(*view);
		CHECK_MESSAGE(!shell.Is_Modeless_Shown(*view), "hiding the notice unlists it");
		CHECK_MESSAGE(!view->Is_Shown(), "hiding the notice unlists it");
		CHECK_MESSAGE(host.PresentsNow > hidden, "and taking it away presents at once too");
		CHECK_MESSAGE(shell.Show_Modeless(*view), "the notice shows again");

		shell.Shutdown();
		CHECK_MESSAGE(!shell.Is_Modeless_Shown(*view), "shutdown releases a notice still shown");
		CHECK_MESSAGE(!view->Is_Shown(), "shutdown releases a notice still shown");
		shell.Hide_Modeless(*view);
	}

	{
		CHECK_MESSAGE(shell.Init(), "the shell initializes for the tips");

		RecordingNetServiceClass service;
		service.Model.Kind = UI_NET_LOBBY_HOST;
		service.Model.Host = true;
		service.Model.Players.push_back(Net_Player("Host"));

		UINetLobbyState state = service.Model;
		UINetLobbyPresenterClass presenter(service, state);
		std::unique_ptr<UIViewClass> view = UI_Net_Setup_View(presenter);
		CHECK_MESSAGE(shell.Show_Modeless(*view), "the lobby shows");
		shell.Tick();

		Rml::Element * tip = Rml(*view).Document()->GetElementById("tip");
		Rml::ElementList cells;
		Rml(*view).Document()->GetElementsByClassName(cells, "cell-house");
		REQUIRE_MESSAGE(tip != nullptr, "the lobby carries a tip and a country cell");
		CHECK_MESSAGE(!cells.empty(), "the lobby carries a tip and a country cell");

		if (tip != nullptr && !cells.empty()) {
			CHECK_MESSAGE(!tip->IsClassSet("up"), "no tip shows before the pointer rests anywhere");

			host.Held = true;
			host.Now = 10000;
			Move(shell, Element_Point(host, cells[0]));
			shell.Tick();
			CHECK_MESSAGE(tip->GetInnerRML() == "GDI", "the tip reads the country the cell names");
			CHECK_MESSAGE(!tip->IsClassSet("up"), "the tip waits while the pointer has only just arrived");

			host.Now += 999;
			shell.Tick();
			CHECK_MESSAGE(!tip->IsClassSet("up"), "the tip is still down a moment before the second is up");

			host.Now += 2;
			shell.Tick();
			CHECK_MESSAGE(tip->IsClassSet("up"), "the tip shows once the pointer has rested a second");
			shell.Tick();

			Rml::Vector2f at = tip->GetAbsoluteOffset(Rml::BoxArea::Border);
			Rml::Vector2f size = tip->GetBox().GetSize(Rml::BoxArea::Border);
			CHECK_MESSAGE(at.y > Center_Of(cells[0]).y, "the tip sits below the pointer");
			CHECK_MESSAGE(at.x >= 0.0f, "the tip stays on the screen");
			CHECK_MESSAGE(at.y >= 0.0f, "the tip stays on the screen");
			CHECK_MESSAGE(at.x + size.x <= (float)host.Rect.Width, "the tip stays on the screen");
			CHECK_MESSAGE(at.y + size.y <= (float)host.Rect.Height, "the tip stays on the screen");

			host.Down[VK_LBUTTON] = true;
			Press(shell, WINDOW_BUTTON_LEFT, Element_Point(host, cells[0]));
			host.Down[VK_LBUTTON] = false;
			Release(shell, WINDOW_BUTTON_LEFT, Element_Point(host, cells[0]));
			shell.Tick();
			CHECK_MESSAGE(!tip->IsClassSet("up"), "a press takes the tip down");

			host.Now += 100;
			Move(shell, PointType{ host.Rect.Width / 2, host.Rect.Height / 2 });
			Move(shell, Element_Point(host, cells[0]));
			shell.Tick();
			host.Now += 301;
			shell.Tick();
			CHECK_MESSAGE(tip->IsClassSet("up"), "a tip that follows one just taken down comes sooner");

			Move(shell, PointType{ host.Rect.Width - 2, host.Rect.Height - 2 });
			shell.Tick();
			CHECK_MESSAGE(!tip->IsClassSet("up"), "the tip goes once the pointer leaves the control");

			Move(shell, Element_Point(host, cells[0]));
			shell.Tick();
			host.Now += 1001;
			shell.Tick();
			CHECK_MESSAGE(tip->IsClassSet("up"), "the tip comes back over the country cell");

			cells[0]->GetParentNode()->RemoveChild(cells[0]).reset();
			shell.Tick();
			CHECK_MESSAGE(!tip->IsClassSet("up"), "the tip goes when its control is deleted");
			host.Held = false;
		}

		shell.Hide_Modeless(*view);
		shell.Shutdown();
	}

	{
		CHECK_MESSAGE(shell.Init(), "the shell initializes for the diplomacy screen");

		RecordingDiplomacyServiceClass service;
		UIDiplomacyPresenterClass presenter(service);
		presenter.State = Diplomacy_State(2, 0);
		std::unique_ptr<UIViewClass> view = UI_Diplomacy_View(presenter);
		CHECK_MESSAGE(shell.Show_Modeless(*view), "the diplomacy screen shows beside the game");
		shell.Tick();

		Rml::ElementDocument * document = Rml(*view).Document();
		Rml::Element * reveal = document->GetElementById("reveal");
		Rml::Element * box = document->QuerySelector("#players .row:nth-child(5) .chat input");
		REQUIRE_MESSAGE(reveal != nullptr, "the screen carries its frame and a send box");
		CHECK_MESSAGE(box != nullptr, "the screen carries its frame and a send box");

		if (reveal != nullptr && box != nullptr) {
			PointType beside{ host.Rect.X + 4, host.Rect.Y + host.Rect.Height / 2 };
			CHECK_MESSAGE(reveal->GetAbsoluteOffset(Rml::BoxArea::Border).x > 4.0f, "the test point lies outside the frame");
			CHECK_MESSAGE(!Press(shell, WINDOW_BUTTON_LEFT, beside), "a click beside the screen goes to the map");
			Release(shell, WINDOW_BUTTON_LEFT, beside);

			PointType at = Element_Point(host, box);
			Move(shell, at);
			host.Down[VK_LBUTTON] = true;
			CHECK_MESSAGE(Press(shell, WINDOW_BUTTON_LEFT, at), "a click on a box stays with the screen");
			host.Down[VK_LBUTTON] = false;
			Release(shell, WINDOW_BUTTON_LEFT, at);
			shell.Tick();
			presenter.Drain();
			CHECK_MESSAGE(service.Calls == std::vector<std::string>{ "send 1 off" }, "the click asks for the change its row allows");

			Key_Down(shell, VK_SPACE);
			Key_Up(shell, VK_SPACE);
			shell.Tick();
			presenter.Drain();
			CHECK_MESSAGE(service.Calls.size() == 1, "space after the click does not click the box again");

			CHECK_MESSAGE(!Key_Down(shell, 'A'), "an ordinary key goes to the game");
			Key_Up(shell, 'A');

			std::this_thread::sleep_for(std::chrono::milliseconds(320));
			CHECK_MESSAGE(Key_Down(shell, VK_TAB), "tab stays with the screen");
			Key_Up(shell, VK_TAB);
			presenter.Drain();
			CHECK_MESSAGE(presenter.Closed, "and asks to close it");
		}

		shell.Hide_Modeless(*view);
		shell.Shutdown();
	}

	CHECK_MESSAGE(shell.Init(), "the shell initializes again after a shutdown");
	shell.Shutdown();

	CHECK_MESSAGE(fixture.Render->ReleasedGeometry == fixture.Render->Compiled, "the shell releases every geometry it compiled");
	CHECK_MESSAGE(fixture.Render->ReleasedTextures == fixture.Render->Loaded + fixture.Render->Generated, "the shell releases every texture it made");
	CHECK_MESSAGE(fixture.Render->Invalid == 0, "the shell's screens compile only geometry the renderer accepts");
	CHECK_MESSAGE(fixture.System->Problems == 0, "the shell's screens raise no RmlUi warning or error");
	Test_Missing_UI_Files(fixture, reader);
}

}
