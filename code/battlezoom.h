/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Battlefield zoom: the mouse wheel over the tactical map magnifies it by a whole number while
// the sidebar and tab strip keep their size. BattlefieldRect holds the frame area the map
// occupies and TacticalRect the smaller native view that is rendered and magnified into it.
// Frame positions over the battlefield reach the game in native view pixels.

#pragma once

#include "point.h"
#include "rect.h"
#include "windowevent.hh"

class Surface;


int Battle_Zoom_Factor(void);

Rect Battle_Zoom_Fit_View(Rect const & battlefield);
bool Battle_Zoom_Applying(void);
void Battle_Zoom_Reset(void);
void Battle_Zoom_AI(bool input_pending);

bool Battle_Zoom_Wheel(float wheel, Point2D const & frame);

void Battle_Zoom_Mouse_Button(WindowMouseButton button, bool down, Point2D const & frame);
void Battle_Zoom_Release_Buttons(void);
void Battle_Zoom_Cancel_Pending(void);
void Battle_Zoom_Frame_To_View(Point2D & point);
Point2D Battle_Zoom_View_To_Frame(Point2D const & point);
Point2D Battle_Zoom_Edge_Point(Point2D const & point);

bool Battle_Zoom_Present(Surface & surface);
