/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine's side of the shell: its host over the window, video, keyboard, options and
// dialogs, and the service pass a modal screen runs the game with.

#pragma once

#include "ui/uihost.h"
#include "ui/uiscreen.h"

#include <string>

class UIViewClass;


UIShellHostClass & UI_Engine_Host(void);

// A color as a document writes one, "#rrggbb".
std::string UI_Color_Text(COLORREF color);

// One pass of the game under a modal screen: the message pump, then Main_Loop in a network
// session or Call_Back otherwise. True when the game ended.
bool UI_Service_Game(void);

// Runs a modal screen on UIShell. A screen opened while another runs is serviced as that one
// is, so what the outer screen keeps alive stays alive under it; the first screen has the game
// serviced each pass. A screen that hides its parent takes the screen below it down for its
// own passes.
UIResult UI_Run_Modal(UIViewClass & view, bool hideparent = false);

// One pass of a screen standing over the running game, for a game wait to spend its idle time
// on. Nothing happens when no screen is shown.
void UI_Serve_Screen(void);

// The mounted archives changed, so a screen's art, dialog font and style sheets are read
// again. Call it once the new archives are mounted and the old ones gone.
void UI_On_Archives_Change(void);
