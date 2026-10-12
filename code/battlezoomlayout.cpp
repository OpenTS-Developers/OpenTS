/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "battlezoomlayout.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>


/// <summary>
/// Can the battlefield area show the tactical view at this zoom factor?
/// </summary>
/// <returns>bool; True for the unzoomed view of any valid area, and for a larger factor whose
/// native view keeps the minimum size.</returns>
bool Battle_Zoom_Allowed(Rect const & battlefield, int zoom)
{
	if (!battlefield.Is_Valid() || zoom < BATTLE_ZOOM_MIN || zoom > BATTLE_ZOOM_MAX) {
		return(false);
	}

	if (zoom == BATTLE_ZOOM_MIN) {
		return(true);
	}

	return(battlefield.Width / zoom >= BATTLE_ZOOM_MIN_VIEW_WIDTH && battlefield.Height / zoom >= BATTLE_ZOOM_MIN_VIEW_HEIGHT);
}


/// <summary>
/// Finds the allowed zoom factor closest to the one asked for, taking the smaller factor on a tie.
/// </summary>
/// <returns>int; The factor to use, which is the unzoomed factor when no other fits.</returns>
int Battle_Zoom_Nearest_Allowed(Rect const & battlefield, int zoom)
{
	int best = BATTLE_ZOOM_MIN;

	for (int factor = BATTLE_ZOOM_MIN; factor <= BATTLE_ZOOM_MAX; factor++) {
		if (Battle_Zoom_Allowed(battlefield, factor) && std::abs(factor - zoom) < std::abs(best - zoom)) {
			best = factor;
		}
	}

	return(best);
}


/// <summary>
/// The native tactical view for a battlefield area. It shares the area's top left corner and
/// equals the area when the factor is the unzoomed one.
/// </summary>
Rect Battle_Zoom_View_Rect(Rect const & battlefield, int zoom)
{
	zoom = std::clamp(zoom, (int)BATTLE_ZOOM_MIN, (int)BATTLE_ZOOM_MAX);

	return(Rect(battlefield.X, battlefield.Y, battlefield.Width / zoom, battlefield.Height / zoom));
}


/// <summary>
/// The part of the battlefield area that shows the magnified view. Rows and columns that the
/// whole-number magnification cannot fill are split evenly around it.
/// </summary>
Rect Battle_Zoom_Screen_Rect(Rect const & battlefield, int zoom)
{
	zoom = std::clamp(zoom, (int)BATTLE_ZOOM_MIN, (int)BATTLE_ZOOM_MAX);

	Rect view = Battle_Zoom_View_Rect(battlefield, zoom);
	int width = view.Width * zoom;
	int height = view.Height * zoom;

	return(Rect(battlefield.X + (battlefield.Width - width) / 2, battlefield.Y + (battlefield.Height - height) / 2, width, height));
}


/// <summary>
/// Converts a frame position into the native tactical view.
/// </summary>
/// <returns>Point2D; A position over the battlefield area, including its unfilled border, comes
/// back inside the native view. Any other position comes back unchanged.</returns>
Point2D Battle_Zoom_Screen_To_View(Point2D const & point, Rect const & battlefield, int zoom)
{
	if (zoom <= BATTLE_ZOOM_MIN || !battlefield.Is_Point_Within(point)) {
		return(point);
	}

	Rect screen = Battle_Zoom_Screen_Rect(battlefield, zoom);
	Rect view = Battle_Zoom_View_Rect(battlefield, zoom);

	int x = std::clamp(point.X, screen.X, screen.X + screen.Width - 1);
	int y = std::clamp(point.Y, screen.Y, screen.Y + screen.Height - 1);

	return(Point2D(view.X + (x - screen.X) / zoom, view.Y + (y - screen.Y) / zoom));
}


/// <summary>
/// Converts a position in the native tactical view into the frame.
/// </summary>
/// <returns>Point2D; A position inside the native view comes back at the middle of the frame
/// pixels it covers. Any other position comes back unchanged.</returns>
Point2D Battle_Zoom_View_To_Screen(Point2D const & point, Rect const & battlefield, int zoom)
{
	Rect view = Battle_Zoom_View_Rect(battlefield, zoom);

	if (zoom <= BATTLE_ZOOM_MIN || !view.Is_Point_Within(point)) {
		return(point);
	}

	Rect screen = Battle_Zoom_Screen_Rect(battlefield, zoom);

	return(Point2D(screen.X + (point.X - view.X) * zoom + zoom / 2, screen.Y + (point.Y - view.Y) * zoom + zoom / 2));
}


/// <summary>
/// Works out the tactical view centre that keeps a world position under the same frame position
/// after the native view changes size.
/// </summary>
/// <param name="center">The view centre before the change, in absolute pixels.</param>
/// <param name="before_view">The native view before the change.</param>
/// <param name="before_point">The anchor in native view pixels before the change.</param>
/// <param name="after_view">The native view after the change.</param>
/// <param name="after_point">The same anchor in native view pixels after the change.</param>
/// <returns>Point2D; The new view centre before any clamping to the map.</returns>
Point2D Battle_Zoom_Anchored_Center(Point2D const & center, Rect const & before_view, Point2D const & before_point, Rect const & after_view, Point2D const & after_point)
{
	return(Point2D(
		center.X - before_view.Width / 2 + (before_point.X - before_view.X) + after_view.Width / 2 - (after_point.X - after_view.X),
		center.Y - before_view.Height / 2 + (before_point.Y - before_view.Y) + after_view.Height / 2 - (after_point.Y - after_view.Y)));
}


/// <summary>
/// Turns wheel movement into whole zoom steps. A fraction from a smooth wheel or touchpad is
/// kept in the remainder until it adds up to a step, and is dropped when the wheel turns the
/// other way.
/// </summary>
/// <returns>int; Steps to zoom in, or out when negative.</returns>
int Battle_Zoom_Wheel_Steps(float & remainder, float wheel)
{
	if ((remainder < 0.0f && wheel > 0.0f) || (remainder > 0.0f && wheel < 0.0f)) {
		remainder = 0.0f;
	}

	remainder += wheel;

	int steps = (int)std::trunc(remainder);
	remainder -= (float)steps;

	return(steps);
}
