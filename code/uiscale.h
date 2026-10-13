/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Interface scale: the sidebar and the tab strip are drawn a whole number of times larger
// than their artwork, so they stay readable at high game resolutions. The sidebar surface
// and the tab strip work in interface pixels; the battlefield works in frame pixels.

#pragma once

#include "rect.h"


int UI_Scale(void);
int UI_Height(void);

void UI_Scale_Layout(Rect const & visible, Rect & battlefield, Rect & composite, Rect & sidebar);
void UI_Scale_Refit(void);

int UI_Dialog_Scale(void);
void UI_Scale_Sync_Dialogs(void);

void UI_Scale_Show_Sidebar(Rect const & source);
