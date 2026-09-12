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

class UIViewClass;


UIShellHostClass & UI_Engine_Host(void);

// One pass of the game under a modal screen: the message pump, then Main_Loop in a network
// session or Call_Back otherwise. True when the game ended.
bool UI_Service_Game(void);

// Runs a modal screen on UIShell with the game serviced each pass.
UIResult UI_Run_Modal(UIViewClass & view);
