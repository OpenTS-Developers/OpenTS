/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Geometry of the interface scale. The sidebar and the tab strip keep their artwork size in
// their own interface pixels and are drawn a whole number of times larger on the frame, while
// the battlefield fills the rest of the frame at its own pixels.

#pragma once

#include "point.h"
#include "rect.h"


enum {
	UI_SCALE_MIN = 1,
	UI_SCALE_MAX = 4,

	// The interface keeps at least this many interface pixels at any scale.
	UI_SCALE_MIN_WIDTH = 640,
	UI_SCALE_MIN_HEIGHT = 540,

	UI_SCALE_SIDEBAR_WIDTH = 168,
	UI_SCALE_TAB_HEIGHT = 16,

	// The battlefield left beside a scaled interface keeps at least this size.
	UI_SCALE_MIN_FIELD_WIDTH = 472,
	UI_SCALE_MIN_FIELD_HEIGHT = 524
};


int UI_Scale_Automatic(int height);
int UI_Scale_For(int width, int height, int configured);

Rect UI_Scale_Battlefield_Rect(int width, int height, int scale);

Point2D UI_Scale_Screen_To_Interface(Point2D const & point, Rect const & battlefield, int scale);
Point2D UI_Scale_Interface_To_Screen(Point2D const & point, Rect const & battlefield, int scale);
