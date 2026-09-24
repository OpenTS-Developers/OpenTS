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

class UIFontEngineClass;
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
		bool Show_Modeless(UIViewClass & view);
		void Hide_Modeless(UIViewClass & view);
		void Refresh(void);

		UIClockClass & Clock(void);
		void Play_Click(void);

		void On_Video_Change(void);
		void On_Archives_Change(void);
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
		bool Handle_Wheel(int x, int y, float notches, bool horizontal, int modifiers);
		bool Handle_Key(bool down, int virtualkey, bool repeat, int modifiers);
		bool Handle_Text(char32_t code);

		UIShellHostClass & Host;
		std::unique_ptr<UIRmlSystemClass> System;
		std::unique_ptr<Rml::FileInterface> File;
		std::unique_ptr<UIRmlRenderClass> Render;
		std::unique_ptr<UIFontEngineClass> Fonts;

		Rml::Context * Context = nullptr;
		bool Ready = false;
		bool FontLoaded = false;
		bool DialogFontTried = false;

		std::vector<unsigned char> SystemFontData;

		void Register_Fonts(void);
		void Ensure_Dialog_Font(void);
		void Apply_Font_Policy(void);
		bool Load_Sheet_Font(char const * family);
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

		bool DevWasActive = false;

		int ArtMagnification = 1;
		float PixelRatio = 1.0f;
};
