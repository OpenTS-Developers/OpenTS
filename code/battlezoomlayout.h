/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Geometry of the battlefield zoom. The tactical view is rendered at its native size in the
// top left corner of the battlefield area and magnified by a whole number into the middle of
// that area, so the sidebar and the tab strip keep their size while the battlefield zooms.

#pragma once

#include "point.h"
#include "rect.h"


enum {
	BATTLE_ZOOM_MIN = 1,
	BATTLE_ZOOM_MAX = 4,

	// A zoomed native view smaller than this is not offered.
	BATTLE_ZOOM_MIN_VIEW_WIDTH = 472,
	BATTLE_ZOOM_MIN_VIEW_HEIGHT = 524
};


bool Battle_Zoom_Allowed(Rect const & battlefield, int zoom);
int Battle_Zoom_Nearest_Allowed(Rect const & battlefield, int zoom);

Rect Battle_Zoom_View_Rect(Rect const & battlefield, int zoom);
Rect Battle_Zoom_Screen_Rect(Rect const & battlefield, int zoom);

Point2D Battle_Zoom_Screen_To_View(Point2D const & point, Rect const & battlefield, int zoom);
Point2D Battle_Zoom_View_To_Screen(Point2D const & point, Rect const & battlefield, int zoom);
Point2D Battle_Zoom_Edge_Point(Point2D const & point, Rect const & battlefield, int zoom, Rect const & frame, int interface_scale);

Point2D Battle_Zoom_Anchored_Center(Point2D const & center, Rect const & before_view, Point2D const & before_point, Rect const & after_view, Point2D const & after_point);

int Battle_Zoom_Wheel_Steps(float & remainder, float wheel);
