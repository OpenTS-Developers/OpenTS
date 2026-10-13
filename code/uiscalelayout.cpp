/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "uiscalelayout.h"

#include <algorithm>


/// <summary>
/// The interface scale chosen for a frame height when the player has not chosen one: 1x up to
/// 1440 rows, 3x at 2160 rows, and one step per further 1080 rows, up to 4x.
/// </summary>
int UI_Scale_Automatic(int height)
{
	return(std::clamp(height >= 2160 ? std::max(3, height / 1080) : height / 1080, (int)UI_SCALE_MIN, (int)UI_SCALE_MAX));
}


/// <summary>
/// Picks the interface scale for a frame.
/// </summary>
/// <param name="configured">The player's choice from 1 to 4, or 0 or less for the automatic scale.</param>
/// <returns>int; The chosen scale, lowered until the interface and the battlefield beside it
/// keep their minimum sizes, and never below 1.</returns>
int UI_Scale_For(int width, int height, int configured)
{
	int scale = configured > 0 ? std::min(configured, (int)UI_SCALE_MAX) : UI_Scale_Automatic(height);

	while (scale > UI_SCALE_MIN
		&& (width / scale < UI_SCALE_MIN_WIDTH || height / scale < UI_SCALE_MIN_HEIGHT
			|| width - UI_SCALE_SIDEBAR_WIDTH * scale < UI_SCALE_MIN_FIELD_WIDTH
			|| height - UI_SCALE_TAB_HEIGHT * scale < UI_SCALE_MIN_FIELD_HEIGHT)) {
		scale--;
	}

	return(std::max(scale, (int)UI_SCALE_MIN));
}


/// <summary>
/// The frame area the battlefield fills beside a right-hand sidebar and below the tab strip,
/// both drawn at the given scale.
/// </summary>
Rect UI_Scale_Battlefield_Rect(int width, int height, int scale)
{
	return(Rect(0, UI_SCALE_TAB_HEIGHT * scale, width - UI_SCALE_SIDEBAR_WIDTH * scale, height - UI_SCALE_TAB_HEIGHT * scale));
}


/// <summary>
/// Converts a frame position into the coordinates the sidebar and the tab strip use.
/// </summary>
/// <returns>Point2D; A position right of the battlefield lands in sidebar coordinates, which
/// start at the battlefield's right edge. A position above the battlefield lands in tab strip
/// coordinates. A position over the battlefield, or any position at 1x, comes back unchanged.</returns>
Point2D UI_Scale_Screen_To_Interface(Point2D const & point, Rect const & battlefield, int scale)
{
	if (scale <= UI_SCALE_MIN) {
		return(point);
	}

	int const right = battlefield.X + battlefield.Width;

	if (point.X >= right) {
		return(Point2D(right + (point.X - right) / scale, point.Y / scale));
	}

	if (point.Y < battlefield.Y) {
		return(Point2D(point.X / scale, point.Y / scale));
	}

	return(point);
}


/// <summary>
/// Converts a sidebar or tab strip position into the frame.
/// </summary>
/// <returns>Point2D; The middle of the frame pixels the interface pixel covers. A battlefield
/// position, or any position at 1x, comes back unchanged.</returns>
Point2D UI_Scale_Interface_To_Screen(Point2D const & point, Rect const & battlefield, int scale)
{
	if (scale <= UI_SCALE_MIN) {
		return(point);
	}

	int const right = battlefield.X + battlefield.Width;

	if (point.X >= right) {
		return(Point2D(right + (point.X - right) * scale + scale / 2, point.Y * scale + scale / 2));
	}

	if (point.Y < UI_SCALE_TAB_HEIGHT) {
		return(Point2D(point.X * scale + scale / 2, point.Y * scale + scale / 2));
	}

	return(point);
}
