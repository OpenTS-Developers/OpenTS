/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins the battlefield zoom geometry: which factors each battlefield size offers, where the
// native view and its magnified picture sit, how frame and view positions convert, and the
// view centre that keeps the world under the cursor.

#include "battlezoomlayout.h"
#include "battlezoomstate.hh"
#include "uiscalelayout.h"

#include <cstdio>
#include <initializer_list>

namespace {

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-76s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


bool Same(Rect const & rect, int x, int y, int w, int h)
{
	return(rect.X == x && rect.Y == y && rect.Width == w && rect.Height == h);
}


bool Same(Point2D const & point, int x, int y)
{
	return(point.X == x && point.Y == y);
}


// A world pixel under a native view position, measured the way the tactical view measures it.
Point2D World(Point2D const & center, Rect const & view, Point2D const & point)
{
	return(Point2D(center.X - view.Width / 2 + (point.X - view.X), center.Y - view.Height / 2 + (point.Y - view.Y)));
}

}


int main()
{
	// The battlefield areas a 1080p, 720p and 2160p frame give with the sidebar on the right.
	Rect const hd(0, 16, 1752, 1064);
	Rect const small(0, 16, 1112, 704);
	Rect const uhd(0, 16, 3672, 2144);

	Check(Battle_Zoom_Allowed(hd, 1) && Battle_Zoom_Allowed(small, 1), "the unzoomed view is always allowed");
	Check(Battle_Zoom_Allowed(hd, 2) && !Battle_Zoom_Allowed(hd, 3), "1080p offers 2x but not 3x");
	Check(!Battle_Zoom_Allowed(small, 2), "720p offers no zoom");
	Check(Battle_Zoom_Allowed(uhd, 4), "2160p offers 4x");
	Check(!Battle_Zoom_Allowed(hd, 0) && !Battle_Zoom_Allowed(uhd, 5), "factors outside 1 to 4 are refused");
	Check(!Battle_Zoom_Allowed(Rect(0, 0, 0, 0), 1), "an empty battlefield allows nothing");

	Check(Battle_Zoom_Nearest_Allowed(hd, 3) == 2 && Battle_Zoom_Nearest_Allowed(hd, 4) == 2, "an unavailable factor falls back to the largest allowed");
	Check(Battle_Zoom_Nearest_Allowed(small, 2) == 1, "with no zoom available the view stays unzoomed");
	Check(Battle_Zoom_Nearest_Allowed(uhd, 3) == 3, "an allowed factor is kept");

	Check(Same(Battle_Zoom_View_Rect(hd, 1), 0, 16, 1752, 1064), "the unzoomed view is the battlefield");
	Check(Same(Battle_Zoom_View_Rect(hd, 2), 0, 16, 876, 532), "a 2x view is half the battlefield at its corner");
	Check(Same(Battle_Zoom_Screen_Rect(hd, 2), 0, 16, 1752, 1064), "an even battlefield is filled exactly");

	Rect const odd(0, 16, 1760, 1601);
	Check(Same(Battle_Zoom_Screen_Rect(odd, 3), 1, 17, 1758, 1599), "unfilled pixels are split around the picture");

	Check(Same(Battle_Zoom_Screen_To_View(Point2D(1000, 500), hd, 1), 1000, 500), "an unzoomed position is unchanged");
	Check(Same(Battle_Zoom_Screen_To_View(Point2D(0, 16), hd, 2), 0, 16), "the battlefield corner is the view corner");
	Check(Same(Battle_Zoom_Screen_To_View(Point2D(3, 19), hd, 2), 1, 17), "each view pixel covers 2x2 frame pixels");
	Check(Same(Battle_Zoom_Screen_To_View(Point2D(1751, 1079), hd, 2), 875, 547), "the far corner is the last view pixel");
	Check(Same(Battle_Zoom_Screen_To_View(Point2D(1800, 500), hd, 2), 1800, 500), "a sidebar position is unchanged");
	Check(Same(Battle_Zoom_Screen_To_View(Point2D(100, 5), hd, 2), 100, 5), "a tab strip position is unchanged");
	Check(Same(Battle_Zoom_Screen_To_View(Point2D(0, 16), odd, 3), 0, 16), "an unfilled border pixel maps to the nearest view pixel");

	bool roundtrip = true;
	for (int y = 16; y < 16 + 532; y += 37) {
		for (int x = 0; x < 876; x += 41) {
			Point2D const view(x, y);
			if (!(Battle_Zoom_Screen_To_View(Battle_Zoom_View_To_Screen(view, hd, 2), hd, 2) == view)) {
				roundtrip = false;
			}
		}
	}
	Check(roundtrip, "a view position survives the trip to the frame and back");
	Check(Same(Battle_Zoom_View_To_Screen(Point2D(1800, 500), hd, 2), 1800, 500), "a position outside the view is not converted");

	Point2D const center(5000, 3000);
	Point2D const cursor(1000, 500);
	Rect const before = Battle_Zoom_View_Rect(hd, 1);
	Rect const after = Battle_Zoom_View_Rect(hd, 2);
	Point2D const before_point = Battle_Zoom_Screen_To_View(cursor, hd, 1);
	Point2D const after_point = Battle_Zoom_Screen_To_View(cursor, hd, 2);
	Point2D const anchored = Battle_Zoom_Anchored_Center(center, before, before_point, after, after_point);
	Check(World(center, before, before_point) == World(anchored, after, after_point), "zooming in keeps the world pixel under the cursor");

	Point2D const back = Battle_Zoom_Anchored_Center(anchored, after, after_point, before, before_point);
	Check(World(anchored, after, after_point) == World(back, before, before_point), "and zooming out again keeps it too");

	float remainder = 0.0f;
	Check(Battle_Zoom_Wheel_Steps(remainder, 1.0f) == 1, "a wheel notch is one step");
	Check(Battle_Zoom_Wheel_Steps(remainder, -2.0f) == -2, "notches toward the player step out");
	Check(Battle_Zoom_Wheel_Steps(remainder, 0.4f) == 0 && Battle_Zoom_Wheel_Steps(remainder, 0.4f) == 0, "fractions wait");
	Check(Battle_Zoom_Wheel_Steps(remainder, 0.4f) == 1, "until they add up to a step");
	Check(Battle_Zoom_Wheel_Steps(remainder, -1.0f) == -1, "turning back drops the fraction left over");

	for (WindowMouseButton button : { WINDOW_BUTTON_LEFT, WINDOW_BUTTON_RIGHT, WINDOW_BUTTON_MIDDLE, WINDOW_BUTTON_X1, WINDOW_BUTTON_X2 }) {
		BattleZoomStateClass state;
		Point2D anchor;
		state.Fit_View(hd, true);
		state.Mouse_Button(button, true, hd, cursor);
		Check(state.Wheel(hd, true, 1.0f, cursor), "a held button's wheel request is consumed by zoom");
		state.Reconcile_Buttons(BattleZoomStateClass::Button_Mask(button));
		Check(!state.Apply_Pending(hd, true, false, anchor) && state.Factor() == 1, "a pending zoom waits for the held button");
		state.Mouse_Button(button, false, hd, Point2D(1800, 600));
		Check(state.Apply_Pending(hd, true, false, anchor) && state.Factor() == 2 && anchor == cursor, "release applies the queued factor at its recorded pointer location");
	}

	BattleZoomStateClass state;
	Point2D anchor;
	state.Fit_View(hd, true);
	state.Wheel(hd, true, 1.0f, cursor);
	Check(!state.Apply_Pending(hd, true, true, anchor) && state.Factor() == 1, "pending keyboard input defers the factor change");
	Check(state.Apply_Pending(hd, true, false, anchor) && anchor == cursor, "an idle frame applies the retained pointer anchor");

	state.Reset();
	state.Wheel(hd, true, 1.0f, cursor);
	state.Reconcile_Buttons(BattleZoomStateClass::Button_Mask(WINDOW_BUTTON_MIDDLE));
	Check(!state.Apply_Pending(hd, true, false, anchor), "a polled middle button blocks a request queued before its press");
	state.Reconcile_Buttons(0);
	Check(state.Apply_Pending(hd, true, false, anchor), "a release consumed by the interface is reconciled from live state");

	state.Reset();
	state.Mouse_Button(WINDOW_BUTTON_LEFT, true, hd, cursor);
	state.Mouse_Button(WINDOW_BUTTON_LEFT, true, hd, cursor);
	state.Wheel(hd, true, 1.0f, cursor);
	state.Mouse_Button(WINDOW_BUTTON_LEFT, false, hd, cursor);
	Check(!state.Dragging() && state.Apply_Pending(hd, true, false, anchor), "a repeated press cannot leave a phantom held button after release");

	state.Reset();
	state.Mouse_Button(WINDOW_BUTTON_LEFT, true, hd, cursor);
	state.Mouse_Button(WINDOW_BUTTON_RIGHT, true, hd, cursor);
	state.Wheel(hd, true, 1.0f, cursor);
	state.Mouse_Button(WINDOW_BUTTON_LEFT, false, hd, cursor);
	Check(!state.Dragging() && !state.Apply_Pending(hd, true, false, anchor), "releasing the drag's button ends its clamp but another hold still defers zoom");
	state.Mouse_Button(WINDOW_BUTTON_RIGHT, false, hd, cursor);
	Check(state.Apply_Pending(hd, true, false, anchor), "the pending change waits for the last button to release");

	state.Reset();
	state.Mouse_Button(WINDOW_BUTTON_LEFT, true, hd, Point2D(1800, 500));
	Check(!state.Dragging(), "a sidebar press does not clamp the pointer onto the battlefield");
	state.Release_Buttons();
	Check(!state.Dragging(), "focus release clears the active drag");
	state.Wheel(hd, true, 1.0f, cursor);
	state.Cancel_Pending();
	Check(!state.Apply_Pending(hd, true, false, anchor) && state.Factor() == 1, "focus loss cancels a pending request before focus can return");

	state.Wheel(hd, true, 1.0f, cursor);
	Check(!state.Apply_Pending(hd, false, false, anchor), "input lock or trigger zoom cancels a waiting factor");
	Check(!state.Apply_Pending(hd, true, false, anchor), "unlocking input does not revive a cancelled request");
	state.Wheel(hd, true, 0.6f, cursor);
	state.Apply_Pending(hd, false, false, anchor);
	state.Wheel(hd, true, 0.6f, cursor);
	Check(!state.Apply_Pending(hd, true, false, anchor), "disabled zoom also discards a smooth wheel's partial step");

	state.Reset();
	Check(!state.Wheel(hd, false, 1.0f, cursor) && !state.Wheel(hd, true, 1.0f, Point2D(1800, 500)), "disabled zoom and sidebar input are returned to sidebar scrolling");
	state.Wheel(hd, true, 0.6f, cursor);
	state.Wheel(hd, true, 0.5f, Point2D(1800, 500));
	state.Wheel(hd, true, 0.6f, cursor);
	Check(!state.Apply_Pending(hd, true, false, anchor), "a wheel event outside the battlefield clears its partial step");

	state.Reset();
	state.Fit_View(uhd, true);
	state.Wheel(uhd, true, 1.4f, cursor);
	state.Apply_Pending(uhd, true, false, anchor);
	state.Fit_View(uhd, true);
	state.Wheel(uhd, true, 0.7f, cursor);
	Check(state.Apply_Pending(uhd, true, false, anchor) && state.Factor() == 3, "applying a factor keeps a smooth wheel's unused fraction");
	state.Fit_View(hd, true);
	Check(state.Factor() == 2, "a smaller battlefield falls back to its largest available factor");
	state.Wheel(hd, true, -1.0f, cursor);
	state.Reset();
	Check(state.Factor() == 1 && !state.Dragging() && !state.Apply_Pending(hd, true, false, anchor), "scenario and trigger resets clear the factor, gesture, and pending change");
	state.Wheel(hd, true, 1.0f, cursor);
	state.Apply_Pending(hd, true, false, anchor);
	Check(Same(state.Fit_View(hd, false), hd.X, hd.Y, hd.Width, hd.Height) && state.Factor() == 1, "the menu always fits an unzoomed view after a zoomed match");
	Check(!state.Apply_Pending(hd, true, false, anchor), "starting another scenario cannot apply the previous match's request");
	state.Fit_View(small, true);
	Check(state.Wheel(small, true, 1.0f, cursor) && !state.Apply_Pending(small, true, false, anchor), "a battlefield with no larger factor still consumes the wheel");
	Check(Battle_Zoom_Allowed(Rect(0, 16, 472, 384), 1), "the supported 640x400 frame keeps its smaller unzoomed battlefield");

	Rect const odd_uhd(11, 17, 3673, 2145);
	for (int factor = 1; factor <= 4; factor++) {
		Rect const view = Battle_Zoom_View_Rect(odd_uhd, factor);
		bool samples_match = true;
		for (int y = view.Y; y < view.Y + view.Height; y += 37) {
			for (int x = view.X; x < view.X + view.Width; x += 41) {
				Point2D const point(x, y);
				samples_match &= Battle_Zoom_Screen_To_View(Battle_Zoom_View_To_Screen(point, odd_uhd, factor), odd_uhd, factor) == point;
			}
		}
		Check(samples_match, "odd-dimension frame and view positions round-trip at each supported factor");
	}

	Check(UI_Scale_Automatic(720) == 1 && UI_Scale_Automatic(1080) == 1 && UI_Scale_Automatic(1440) == 1, "the automatic interface scale is 1x below 2160 rows");
	Check(UI_Scale_Automatic(2160) == 3 && UI_Scale_Automatic(4320) == 4, "and 3x at 2160 rows, 4x at 4320");
	Check(UI_Scale_For(3840, 2160, 0) == 3 && UI_Scale_For(1920, 1080, 0) == 1 && UI_Scale_For(2560, 1440, 0) == 1, "an automatic scale fits the common frames");
	Check(UI_Scale_For(1920, 1080, 3) == 2, "a chosen scale the frame cannot fit is lowered until it fits");
	Check(UI_Scale_For(1280, 720, 2) == 1, "the scale never drops below 1x");
	Check(UI_Scale_For(3840, 2160, 9) == 4, "a chosen scale above 4x is held at 4x");

	Rect const scaled = UI_Scale_Battlefield_Rect(3840, 2160, 3);
	Check(Same(scaled, 0, 48, 3336, 2112), "a 3x interface leaves the battlefield beside a 504-pixel sidebar and below a 48-pixel tab strip");
	Check(Same(UI_Scale_Battlefield_Rect(1920, 1080, 1), 0, 16, 1752, 1064), "a 1x interface keeps the stock layout");

	Check(Same(UI_Scale_Screen_To_Interface(Point2D(3336 + 30, 300), scaled, 3), 3336 + 10, 100), "a sidebar position becomes a sidebar position from the battlefield's right edge");
	Check(Same(UI_Scale_Screen_To_Interface(Point2D(300, 30), scaled, 3), 100, 10), "a tab strip position becomes a tab strip position");
	Check(Same(UI_Scale_Screen_To_Interface(Point2D(500, 500), scaled, 3), 500, 500), "a battlefield position is left alone");
	Check(Same(UI_Scale_Screen_To_Interface(Point2D(1800, 5), hd, 1), 1800, 5), "nothing is converted at 1x");

	bool interface_roundtrip = true;
	for (Point2D const & point : { Point2D(3336, 0), Point2D(3336 + 167, 719), Point2D(0, 0), Point2D(1111, 15) }) {
		interface_roundtrip &= UI_Scale_Screen_To_Interface(UI_Scale_Interface_To_Screen(point, scaled, 3), scaled, 3) == point;
	}
	Check(interface_roundtrip, "an interface position survives the trip to the frame and back");

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
