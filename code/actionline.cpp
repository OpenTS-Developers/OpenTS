/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "actionline.h"

#include "_map.h"
#include "_rect.h"
#include "_tactica.h"
#include "abstract.h"
#include "cell.h"
#include "dsurface.h"
#include "globals.h"
#include "hostclock.h"
#include "mouse.h"
#include "point.h"
#include "rect.h"
#include "surface.h"
#include "tactical.h"
#include "uicontrol.h"
#include "win.h"
#include "xsurface.h"


namespace {

// The dash pattern repeats every sixteen pixels, which is what the line drawer walks.
int const PATTERN_LENGTH = 16;


void Draw_Run(Surface & surface, Point2D start, Point2D end, unsigned color, bool dashed, bool pattern[], int offset)
{
	if (Clip_Line_To_Rect(start, end, TacticalRect)) {
		if (dashed) {
			surface.Draw_Dashed_Line(start, end, color, pattern, offset);
		} else {
			surface.Draw_Line(start, end, color);
		}
	}
}

}


Coord Action_Line_Coord(AbstractClass const * target)
{
	Coord coord = target->Center_Coord();
	if (Map.In_Radar(coord.As_Cell()) && Map[coord].IsUnderBridge) {
		coord.Z = BRIDGE_LEPTON_HEIGHT + Map.Get_Height_GL(coord);
	}
	return(coord);
}


void Draw_Action_Line_Segment(Surface & surface, Coord const & start, Coord const & end, UILineStyleType const & style, int point_size, int dash_length, int dash_rate)
{
	Point2D start_point;
	Point2D end_point;
	TacticalMap->Coord_To_Pixel(start, start_point);
	TacticalMap->Coord_To_Pixel(end, end_point);
	start_point += Point2D(TacticalRect.X, TacticalRect.Y);
	end_point += Point2D(TacticalRect.X, TacticalRect.Y);

	unsigned color = DSurface::Build_Hicolor_Pixel(style.Color);
	unsigned drop_color = DSurface::Build_Hicolor_Pixel(style.DropShadowColor);

	bool pattern[PATTERN_LENGTH];
	for (int index = 0; index < PATTERN_LENGTH; index++) {
		pattern[index] = ((index / dash_length) & 1) == 0;
	}
	int offset = (dash_rate > 0) ? ((-(int)Host_Milliseconds() / dash_rate) & (PATTERN_LENGTH - 1)) : (7 * Frame % PATTERN_LENGTH);

	// A thick line is two rows; its shadow sits below both.
	int rows = style.IsThick ? 2 : 1;
	if (style.IsDropShadow) {
		for (int row = 0; row < rows; row++) {
			Point2D shift(0, rows + row);
			Draw_Run(surface, start_point + shift, end_point + shift, drop_color, style.IsDashed, pattern, offset);
		}
	}
	for (int row = 0; row < rows; row++) {
		Point2D shift(0, row);
		Draw_Run(surface, start_point + shift, end_point + shift, color, style.IsDashed, pattern, offset);
	}

	// The squares come from the unclipped ends, so an end beyond the view draws nothing
	// rather than sticking to its edge.
	int size = style.IsThick ? 4 : point_size;
	Point2D corner = style.IsThick ? Point2D(-2, -2) : Point2D(-1, -1);
	if (style.IsDropShadow) {
		int drop_size = size + (style.IsThick ? 3 : 2);
		Point2D drop_corner = corner + (style.IsThick ? Point2D(-2, -2) : Point2D(-1, -1));
		surface.Fill_Rect(Intersect(TacticalRect, Rect(start_point + drop_corner, drop_size, drop_size)), drop_color);
		surface.Fill_Rect(Intersect(TacticalRect, Rect(end_point + drop_corner, drop_size, drop_size)), drop_color);
		if (style.IsThick) {
			size--;
		}
	}
	surface.Fill_Rect(Intersect(TacticalRect, Rect(start_point + corner, size, size)), color);
	surface.Fill_Rect(Intersect(TacticalRect, Rect(end_point + corner, size, size)), color);
}
