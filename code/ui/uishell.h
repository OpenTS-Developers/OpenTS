/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ui/uiinput.h"
#include "ui/uiscreen.h"

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace Rml
{
	class Context;
	class FileInterface;
}

class UIFontRegistryClass;
class UIRmlRenderClass;
class UIRmlSystemClass;
class UIViewClass;
class UIShellHostClass;
struct WindowEvent;

using UIServiceCallback = std::function<bool(void)>;


class UIShellClass
{
	public:
		UIShellClass(UIShellHostClass & host, std::unique_ptr<UIRmlSystemClass> system, std::unique_ptr<Rml::FileInterface> file, std::unique_ptr<UIRmlRenderClass> render);
		~UIShellClass(void);

		UIShellClass(UIShellClass const &) = delete;
		UIShellClass & operator=(UIShellClass const &) = delete;

		bool Init(void);
		void Shutdown(void);

		bool Screen_Shown(void) const;
		void Serve_Shown_Screen(void);
		UIResult Run_Modal(UIViewClass & view, UIServiceCallback const & service, bool hideparent = false);
		UIServiceCallback const * Running_Service(void) const;
		bool Show_Modeless(UIViewClass & view, bool reveal = false);
		bool Owns_Pointer(void) const;
		void Hide_Modeless(UIViewClass & view);
		void Refresh(void);

		UIClockClass & Clock(void);
		void Play_Click(void);

		void On_Video_Change(void);
		void On_Archives_Change(int side);
		std::string Side_Sheet(void) const;
		void Tick(void);
		void Render_Overlay(void);

		bool Handle_Window_Event(WindowEvent const & event);
		bool Handle_Set_Cursor(void);

		Rml::Context * Rml_Context(void) const { return(Context); }
		UIViewClass * Modal(void) const;
		int Modal_Depth(void) const;
		bool Is_Modeless_Shown(UIViewClass const & view) const;
		bool Revealing_Shown(void) const { return(Revealing); }
		UIInputStateClass const & Input_State(void) const { return(Input); }

	private:
		struct DeferredWorkType
		{
			bool ToggleDev = false;
			bool Resize = false;
			bool DropFiles = false;
			bool DropPresses = false;
			bool Leave = false;
			int DevFocus = -1;
		};

		void Log(char const * format, ...);
		void Report_Open_Error(UIViewClass const & view, char const * reason);
		bool Active(void) const;
		bool Documents_Visible(void) const;
		bool Text_Input_Focused(void) const;
		void Apply_Dimensions(void);
		void Drop_Cached_Files(void);
		UIPointerPosition Pointer_Position(int x, int y) const;
		std::array<bool, UIInputStateClass::BUTTON_COUNT> Physical_Buttons(void) const;
		int Key_Modifiers(void) const;
		void Quarantine_Held_Input(void);
		void Reconcile_Held_Input(void);
		void Drop_Presses(void);
		void Release_UI_Capture(void);
		bool Pointer_Owned(void) const;
		void Apply_Cursor_Request(void);
		void Restore_Cursor(void);
		bool Prepare_View(UIViewClass & view);
		void Uncover(UIViewClass * covered);
		void Drain_Deferred(void);
		bool Handle_Input_Event(WindowEvent const & event);
		bool Handle_Mouse_Move(int x, int y, int modifiers);
		bool Handle_Button_Down(int button, int x, int y, int modifiers);
		bool Handle_Button_Up(int button, int x, int y, int modifiers);
		bool Handle_Wheel(int x, int y, float delta, bool horizontal, int modifiers);
		bool Handle_Key(bool down, int code, bool repeat, int modifiers);
		bool Handle_Text(char32_t code);

		UIShellHostClass & Host;
		std::unique_ptr<UIRmlSystemClass> System;
		std::unique_ptr<Rml::FileInterface> File;
		std::unique_ptr<UIRmlRenderClass> Render;
		std::unique_ptr<UIFontRegistryClass> Fonts;

		Rml::Context * Context = nullptr;
		bool Ready = false;
		int Side = -1;

		bool Advance_Reveal(UIViewClass & view, float full, int start, float & shown);
		void Advance_Shown_Reveal(UIViewClass & view);

		bool InContext = false;
		bool InHook = false;
		bool InTick = false;
		DeferredWorkType Deferred;

		UIInputStateClass Input;
		bool TookCapture = false;
		bool MouseInside = false;

		std::optional<UICursor> AppliedCursor;

		std::vector<UIViewClass *> Modals;
		std::vector<UIServiceCallback const *> Services;
		bool ModalClosing = false;
		std::vector<UIViewClass *> Modeless;

		float RevealShown = 0.0f;
		int RevealStart = 0;
		float RevealWidth = 0.0f;
		bool Revealing = false;
		int RevealPasses = 0;
		// A screen beside the game slides in on its own, while the game keeps calling Tick.
		UIViewClass * ModelessRevealing = nullptr;
		float ModelessRevealShown = 0.0f;
		int ModelessRevealStart = 0;
		float ModelessRevealWidth = 0.0f;

		bool DevWasActive = false;

		int ArtMagnification = 1;
};
