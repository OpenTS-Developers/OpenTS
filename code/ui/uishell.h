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

		// True while a modal screen is shown or closing. The developer overlays are not
		// screens.
		bool Screen_Shown(void) const;

		// Settles every shown screen as though the session ended and takes its document off
		// the frame, for a caller about to draw a presentation of its own over the whole of
		// it. Each screen's runner tears it down when the caller hands control back.
		void End_Screens(void);

		// Advances what the shown screen looks like: its opening band, its layout and one
		// present. It executes nothing the player asked for, so a game wait can spend its
		// idle time here without the screen acting from inside that wait.
		void Serve_Shown_Screen(void);

		// Prepares, shows and drives a modal screen until its presenter reports a result or
		// the service reports the game ended, then releases it. The view's presenter must
		// outlive the call. A screen that hides its parent takes the screen below it down for
		// its own passes and puts it back, without a reveal, when it closes.
		UIResult Run_Modal(UIViewClass & view, UIServiceCallback const & service, bool hideparent = false);

		// The service driving the innermost running modal, or nothing while no screen runs,
		// for a screen opened from under another to be serviced as that one is.
		UIServiceCallback const * Running_Service(void) const;

		// Shows a document beside the game without taking its input: a notice the caller
		// updates while it works. It is drawn at once, because such a caller pumps nothing.
		// False when the shell or the document is not ready, so nothing is shown.
		bool Show_Modeless(UIViewClass & view);
		void Hide_Modeless(UIViewClass & view);

		// Advances the documents and presents the overlay now.
		void Refresh(void);

		// The system clock a timed screen's presenter reads.
		UIClockClass & Clock(void);

		// The sound a control makes as it is pressed. A view calls this from its own event
		// handler, because what counts as a press is the toolkit's business.
		void Play_Click(void);

		// The frame moved or changed size inside the window.
		void On_Video_Change(void);

		// The mounted archives changed under the shell, so the art and the dialog font load
		// again. A screen mid-pass has them loaded at the next safe point.
		void On_Archives_Change(void);

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

		// Answers WM_SETCURSOR over the client area: true when a document's pointer shape is
		// on the pointer, so the game's own stays off it.
		bool Handle_Set_Cursor(void);

		Rml::Context * Rml_Context(void) const { return(Context); }
		UIViewClass * Modal(void) const;
		int Modal_Depth(void) const;
		bool Is_Modeless_Shown(UIViewClass const & view) const;
		bool Revealing_Shown(void) const { return(Revealing); }
		UIInputStateClass const & Input_State(void) const { return(Input); }

	private:
		// Work that arrived while the context was updating or rendering, applied at the
		// next safe point in the order the fields are declared.
		struct DeferredWorkType
		{
			bool ToggleDev = false;
			bool Resize = false;
			bool DropArt = false;
			bool DropPresses = false;
			bool Leave = false;
			int DevFocus = -1;
		};

		void Log(char const * format, ...);
		bool Active(void) const;
		bool Documents_Visible(void) const;
		bool Text_Input_Focused(void) const;
		void Apply_Dimensions(void);
		void Drop_Cached_Art(void);
		UIPointerPosition Pointer_Position(LPARAM clientlparam) const;
		std::array<bool, UIInputStateClass::BUTTON_COUNT> Physical_Buttons(void) const;
		int Key_Modifiers(void) const;
		void Quarantine_Held_Input(void);
		void Reconcile_Held_Input(void);
		void Drop_Presses(void);
		void Release_UI_Capture(void);
		void Reset_Text(void);
		bool Pointer_Owned(void) const;
		void Apply_Cursor_Request(void);
		void Restore_Cursor(void);
		bool Prepare_View(UIViewClass & view);
		void Uncover(UIViewClass * covered);
		void Drain_Deferred(void);
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
		std::unique_ptr<UIFontEngineClass> Fonts;

		Rml::Context * Context = nullptr;
		bool Ready = false;
		bool FontLoaded = false;
		bool DialogFontTried = false;

		// RmlUi reads a face straight out of these bytes, so they outlive every document
		// drawn with them.
		std::vector<unsigned char> SystemFontData;

		void Register_Fonts(void);
		void Ensure_Dialog_Font(void);
		void Apply_Font_Policy(void);
		bool Load_Sheet_Font(char const * family);
		bool Advance_Reveal(UIViewClass & view, float full, int start, float & shown);
		void Advance_Shown_Reveal(UIViewClass & view);

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

		// The shape put on the pointer for the documents, or nothing while the game's
		// pointer is in charge.
		std::optional<UICursor> AppliedCursor;

		// The modal screens the runner is driving, innermost last, the service each is driven
		// by, and whether the innermost is between releasing its document and handing the
		// input back.
		std::vector<UIViewClass *> Modals;
		std::vector<UIServiceCallback const *> Services;
		bool ModalClosing = false;
		std::vector<UIViewClass *> Modeless;

		// The band the innermost modal is opening through, in pixels, when it began, how
		// wide it will come to and how many passes it has taken; nothing while no screen is
		// opening. The runner and the waits the game hands over both advance it.
		float RevealShown = 0.0f;
		int RevealStart = 0;
		float RevealWidth = 0.0f;
		bool Revealing = false;
		int RevealPasses = 0;

		bool DevWasActive = false;

		// The factor the art was last magnified by, so a frame that changes it reloads the art.
		int ArtMagnification = 1;
		float PixelRatio = 1.0f;
};
