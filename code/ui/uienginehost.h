/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ui/uihost.h"
#include "ui/uiscreen.h"

#include <string>

class UIViewClass;


UIShellHostClass & UI_Engine_Host(void);

std::string UI_Color_Text(COLORREF color);

bool UI_Service_Game(void);

UIResult UI_Run_Modal(UIViewClass & view, bool hideparent = false);

void UI_Serve_Screen(void);

void UI_On_Archives_Change(void);
