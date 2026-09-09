/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The UI shell owns the RmlUi context, the overlay pass and the input hook. The rest of
// the engine reaches it through these functions alone; no toolkit type appears here.

#pragma once

#include "ui/uiscreen.h"
#include "win.h"

class UIRmlViewClass;


// Needs the window, the renderer and the file search chain. A false return leaves every
// other entry point inert.
bool UI_Init(void);
void UI_Shutdown(void);

// True when a migrated screen should open its RmlUi view rather than its Win32 dialog. A
// caller reads it once at screen entry; the answer follows the LegacyDialogs setting.
bool UI_Use_Rml(void);

// True while a modal screen is shown or closing. The developer overlays are not screens.
bool UI_Screen_Shown(void);

// True while a Win32 dialog is on screen. A screen asked to open over one keeps its legacy
// view, because the visible dialog takes the mouse before a document can.
bool UI_Legacy_Dialog_Visible(void);

// Prepares, shows and drives a modal screen until its presenter reports a result or the game
// ends, then releases it. The view's presenter must outlive the call.
UIResult UI_Run_Modal(UIRmlViewClass & view);

// Shows a document beside the game without taking its input: a notice the caller updates
// while it works. It is drawn at once, because such a caller pumps nothing. False when the
// shell or the document is not ready, so the caller opens its Win32 presentation.
bool UI_Show_Modeless(UIRmlViewClass & view);
void UI_Hide_Modeless(UIRmlViewClass & view);

// Advances the documents and presents the overlay now.
void UI_Refresh(void);

// The system clock a timed screen's presenter reads.
UIClockClass & UI_Clock(void);

// The frame moved or changed size inside the window.
void UI_On_Video_Change(void);

// Advances the documents and the developer overlays. Called at the game's service points,
// never from a paint handler or the message pump; a modal screen's runner drains its intents
// after each call.
void UI_Tick(void);

// Draws the visible documents over the frame the renderer has just submitted.
void UI_Render_Overlay(void);

// Offers a main window message to the shell before the game sees it. The position is the
// raw client one, taken before the router translated it. True means the message is consumed.
bool UI_Handle_Window_Message(HWND hwnd, UINT message, WPARAM wparam, LPARAM clientlparam);

// Offers a pumped message to the shell before dispatch, whichever window it is for. True
// means the message is consumed.
bool UI_Intercept_Pumped_Message(MSG const & msg);
