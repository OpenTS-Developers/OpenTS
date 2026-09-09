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

#include "win.h"


// Needs the window, the renderer and the file search chain. A false return leaves every
// other entry point inert.
bool UI_Init(void);
void UI_Shutdown(void);

// True when a migrated screen should open its RmlUi view rather than its Win32 dialog. A
// caller reads it once at screen entry; the answer follows the LegacyDialogs setting.
bool UI_Use_Rml(void);

// True while a modal screen is shown or closing. The developer overlays are not screens.
bool UI_Screen_Shown(void);

// The frame moved or changed size inside the window.
void UI_On_Video_Change(void);

// Advances the documents and executes the intents their events queued. Called at the
// game's service points, never from a paint handler or the message pump.
void UI_Tick(void);

// Draws the visible documents over the frame the renderer has just submitted.
void UI_Render_Overlay(void);

// Offers a main window message to the shell before the game sees it. The position is the
// raw client one, taken before the router translated it. True means the message is consumed.
bool UI_Handle_Window_Message(HWND hwnd, UINT message, WPARAM wparam, LPARAM clientlparam);

// Offers a pumped message to the shell before dispatch, whichever window it is for. True
// means the message is consumed.
bool UI_Intercept_Pumped_Message(MSG const & msg);
