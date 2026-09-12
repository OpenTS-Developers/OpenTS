/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The UI shell owns the RmlUi context, the overlay pass and the input hook. The engine
// holds one instance, UIShell, declared in _ui.h; no toolkit type appears here.

#pragma once

#include "ui/uicoord.h"
#include "ui/uiinput.h"
#include "ui/uiscreen.h"
#include "win.h"

#include <array>
#include <functional>
#include <memory>
#include <vector>

namespace Rml
{
	class Context;
	class ElementDocument;
	class EventListener;
	class FileInterface;
}

class UIRmlRenderClass;
class UIRmlSystemClass;
class UIViewClass;
class UIShellHostClass;

// Runs the game for one pass under a modal screen and reports whether it ended.
using UIServiceCallback = std::function<bool(void)>;


class UIShellClass
{
	public:
		// The interfaces are owned from here on and outlive Rml::Shutdown, which releases
		// every resource through them. A null file interface leaves RmlUi's own in place.
		UIShellClass(UIShellHostClass & host, std::unique_ptr<UIRmlSystemClass> system, std::unique_ptr<Rml::FileInterface> file, std::unique_ptr<UIRmlRenderClass> render);
		~UIShellClass(void);

		UIShellClass(UIShellClass const &) = delete;
		UIShellClass & operator=(UIShellClass const &) = delete;

		// Needs the window, the renderer and the file search chain. A false return leaves
		// every other entry point inert.
		bool Init(void);
		void Shutdown(void);

		// True when a migrated screen should open its RmlUi view rather than its Win32
		// dialog. A caller reads it once at screen entry; the answer follows the
		// LegacyDialogs setting.
		bool Use_Rml(void) const;

		// True while a modal screen is shown or closing. The developer overlays are not
		// screens.
		bool Screen_Shown(void) const;

		// True while a Win32 dialog is on screen. A screen asked to open over one keeps its
		// legacy view, because the visible dialog takes the mouse before a document can.
		bool Legacy_Dialog_Visible(void) const;

		// Prepares, shows and drives a modal screen until its presenter reports a result or
		// the service reports the game ended, then releases it. The view's presenter must
		// outlive the call.
		UIResult Run_Modal(UIViewClass & view, UIServiceCallback const & service);

		// Shows a document beside the game without taking its input: a notice the caller
		// updates while it works. It is drawn at once, because such a caller pumps nothing.
		// False when the shell or the document is not ready, so the caller opens its Win32
		// presentation.
		bool Show_Modeless(UIViewClass & view);
		void Hide_Modeless(UIViewClass & view);

		// Advances the documents and presents the overlay now.
		void Refresh(void);

		// The system clock a timed screen's presenter reads.
		UIClockClass & Clock(void);

		// The frame moved or changed size inside the window.
		void On_Video_Change(void);

		// Advances the documents and the developer overlays. Called at the game's service
		// points, never from a paint handler or the message pump; a modal screen's runner
		// drains its intents after each call.
		void Tick(void);

		// Draws the visible documents over the frame the renderer has just submitted.
		void Render_Overlay(void);

		// Offers a main window message to the shell before the game sees it. The position
		// is the raw client one, taken before the router translated it. True means the
		// message is consumed.
		bool Handle_Window_Message(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

		// Offers a pumped message to the shell before dispatch, whichever window it is for.
		// True means the message is consumed.
		bool Intercept_Pumped_Message(MSG const & msg);

		// Answers WM_SETCURSOR over the client area: true when a document's pointer shape is
		// on the pointer, so the game's own stays off it.
		bool Handle_Set_Cursor(void);

		Rml::Context * Rml_Context(void) const { return(Context); }
		UIViewClass * Modal(void) const;
		int Modal_Depth(void) const;
		bool Is_Modeless_Shown(UIViewClass const & view) const;
		UIInputStateClass const & Input_State(void) const { return(Input); }

	private:
		friend class UITestListenerClass;

		// Work that arrived while the context was updating or rendering, applied at the
		// next safe point in the order the fields are declared.
		struct DeferredWorkType
		{
			bool ToggleTest = false;
			bool ToggleDev = false;
			bool CloseTest = false;
			bool Resize = false;
			bool DropPresses = false;
			bool Leave = false;
			int DevFocus = -1;
		};

		void Log(char const * format, ...);
		bool Active(void) const;
		bool Documents_Visible(void) const;
		bool Text_Input_Focused(void) const;
		void Apply_Dimensions(void);
		UIPointerPosition Pointer_Position(LPARAM clientlparam) const;
		std::array<bool, UIInputStateClass::BUTTON_COUNT> Physical_Buttons(void) const;
		void Quarantine_Held_Input(void);
		void Reconcile_Held_Input(void);
		void Drop_Presses(void);
		void Release_UI_Capture(void);
		void Reset_Text(void);
		bool Pointer_Owned(void) const;
		void Apply_Cursor_Request(void);
		void Restore_Cursor(void);
		bool Prepare_View(UIViewClass & view);
		void Drain_Deferred(void);
		void Toggle_Test_Document(void);
		bool Handle_Mouse_Move(LPARAM clientlparam);
		bool Handle_Button_Down(int button, LPARAM clientlparam);
		bool Handle_Button_Up(int button, LPARAM clientlparam);
		bool Handle_Wheel(WPARAM wparam, LPARAM screenlparam, bool horizontal);
		bool Handle_Key(UINT message, WPARAM wparam, LPARAM lparam);
		bool Handle_Char(WPARAM wparam);
		bool Feed_Text_Unit(wchar_t unit);
		bool Feed_Text_Byte(unsigned char byte);
		bool Handle_Text(char32_t code);

		UIShellHostClass & Host;
		std::unique_ptr<UIRmlSystemClass> System;
		std::unique_ptr<Rml::FileInterface> File;
		std::unique_ptr<UIRmlRenderClass> Render;

		Rml::Context * Context = nullptr;
		bool Ready = false;
		bool FontLoaded = false;

		// Set while the context updates or renders, while the hook runs, and while a tick
		// runs; each refuses to re-enter itself.
		bool InContext = false;
		bool InHook = false;
		bool InTick = false;
		DeferredWorkType Deferred;

		// Who holds each key and button. A press the toolkits own takes the window's
		// capture; TookCapture says the shell took it and must give it back.
		UIInputStateClass Input;
		bool TookCapture = false;
		bool MouseInside = false;

		// Text arrives one unit or byte per message; these carry a sequence between them.
		wchar_t HighSurrogate = 0;
		UIUTF8DecoderClass Utf8;
		unsigned char LegacyLead = 0;

		// The shape last put on the pointer for the documents, if any.
		UICursor AppliedCursor = UI_CURSOR_ARROW;

		// The modal screens the runner is driving, innermost last, and whether the
		// innermost is between releasing its document and handing the input back.
		std::vector<UIViewClass *> Modals;
		bool ModalClosing = false;
		std::vector<UIViewClass *> Modeless;

		bool DevWasActive = false;

		// The Debug test document a developer key shows over the game.
		Rml::ElementDocument * TestDocument = nullptr;
		std::unique_ptr<Rml::EventListener> TestListener;
};
