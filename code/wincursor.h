/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The mouse pointer, as a system cursor built from the game's own shapes.
//
// The system composites it over the presented frame, so pointing the mouse costs nothing:
// the cursor never touches a game surface and moving it needs no new frame.

#pragma once

#include "win.h"

class ShapeSet;


void Win_Cursor_Set(ShapeSet const * shape, int frame, int hotx, int hoty, bool apply);
void Win_Cursor_Set_Visible(bool visible);
bool Win_Cursor_Handle_Set_Cursor(void);
void Win_Cursor_Refresh(void);
void Win_Cursor_Shutdown(void);
