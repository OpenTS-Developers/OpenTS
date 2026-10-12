/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "battlezoom.h"

#include "_keyboar.h"
#include "_map.h"
#include "_rect.h"
#include "_surface.h"
#include "_tactica.h"
#include "battlezoomlayout.h"
#include "globals.h"
#include "keyboard.h"
#include "session.h"
#include "surface.h"
#include "tactical.h"
#include "win.h"

#include <algorithm>


static int _Zoom = BATTLE_ZOOM_MIN;
static int _Pending = BATTLE_ZOOM_MIN;
static bool _Applying = false;

static bool _HasAnchor = false;
static Point2D _Anchor(0, 0);
static float _WheelRemainder = 0.0f;

static int _ButtonsHeld = 0;
static bool _DragInBattlefield = false;


static bool Can_Zoom(void)
{
	return(ScenarioActive && GameActive && GameInFocus && !Debug_Map && !IgnoreInput && TacticalMap != NULL && TacticalMap->ZoomFactor == 1.0);
}


static bool Is_Zoomed(void)
{
	return(_Zoom > BATTLE_ZOOM_MIN && ScenarioActive);
}


/// <summary>
/// Fetches the zoom factor the battlefield is shown at.
/// </summary>
/// <returns>int; 1 when the battlefield is not zoomed.</returns>
int Battle_Zoom_Factor(void)
{
	return(_Zoom);
}


/// <summary>
/// Is the tactical view being resized for a zoom change rather than a new battlefield area?
/// </summary>
bool Battle_Zoom_Applying(void)
{
	return(_Applying);
}


/// <summary>
/// Records the frame area the battlefield occupies and returns the native view for it. The
/// zoom factor drops to the nearest one the area allows, and to 1x outside a scenario, as when
/// the main menu changes the resolution after a zoomed match.
/// </summary>
/// <param name="battlefield">The battlefield area in frame pixels.</param>
/// <returns>Rect; The native tactical view, which equals the area when the battlefield is not zoomed.</returns>
Rect Battle_Zoom_Fit_View(Rect const & battlefield)
{
	BattlefieldRect = battlefield;
	_Zoom = ScenarioActive ? Battle_Zoom_Nearest_Allowed(battlefield, _Zoom) : (int)BATTLE_ZOOM_MIN;
	_Pending = _Zoom;

	return(Battle_Zoom_View_Rect(battlefield, _Zoom));
}


/// <summary>
/// Returns the battlefield to its unzoomed size about the same view centre, and drops any zoom
/// change still waiting. Messages on screen are kept.
/// </summary>
void Battle_Zoom_Reset(void)
{
	_Pending = BATTLE_ZOOM_MIN;
	_HasAnchor = false;
	_WheelRemainder = 0.0f;

	if (_Zoom != BATTLE_ZOOM_MIN) {
		_Zoom = BATTLE_ZOOM_MIN;
		_Applying = true;
		Map.Set_View_Dimensions(BattlefieldRect);
		_Applying = false;
		Map.Flag_To_Redraw(GS_REDRAW_ALL);
	}
}


/// <summary>
/// Applies a zoom change asked for by the wheel. The change waits while a key press is pending
/// or a mouse button is held, and is dropped while the player cannot zoom. The world position
/// under the cursor when the wheel turned stays under it.
/// </summary>
/// <param name="input_pending">Is a key or button press waiting to be processed this frame?</param>
void Battle_Zoom_AI(bool input_pending)
{
	if (!Can_Zoom()) {
		_Pending = _Zoom;
		_HasAnchor = false;
		return;
	}

	// A release the interface took never reaches Battle_Zoom_Mouse_Button.
	if (Keyboard == NULL || (!Keyboard->Down(KN_LMOUSE) && !Keyboard->Down(KN_RMOUSE))) {
		Battle_Zoom_Release_Buttons();
	}

	if (_Pending == _Zoom || input_pending || _ButtonsHeld > 0) {
		return;
	}

	Point2D frame = _HasAnchor ? _Anchor : Point2D(BattlefieldRect.X + BattlefieldRect.Width / 2, BattlefieldRect.Y + BattlefieldRect.Height / 2);
	Rect const before_view = TacticalRect;
	Point2D const before_point = Battle_Zoom_Screen_To_View(frame, BattlefieldRect, _Zoom);
	Point2D const center = TacticalMap->Get_Tactical_Position();

	_Zoom = _Pending;
	_Applying = true;
	Map.Set_View_Dimensions(BattlefieldRect);
	_Applying = false;

	Point2D const after_point = Battle_Zoom_Screen_To_View(frame, BattlefieldRect, _Zoom);
	TacticalMap->Set_Tactical_Position(Battle_Zoom_Anchored_Center(center, before_view, before_point, TacticalRect, after_point));

	Map.Flag_To_Redraw(GS_REDRAW_ALL);
	_HasAnchor = false;
}


/// <summary>
/// Takes a vertical wheel movement for the battlefield zoom.
/// </summary>
/// <param name="wheel">Wheel notches, positive away from the player.</param>
/// <param name="frame">The pointer position in frame pixels.</param>
/// <returns>bool; Was the movement used here? A movement outside the battlefield, or one made
/// while the player cannot zoom, is left for the sidebar.</returns>
bool Battle_Zoom_Wheel(float wheel, Point2D const & frame)
{
	if (!Can_Zoom() || !BattlefieldRect.Is_Point_Within(frame)) {
		_WheelRemainder = 0.0f;
		return(false);
	}

	if (_ButtonsHeld > 0) {
		_WheelRemainder = 0.0f;
		return(true);
	}

	int steps = Battle_Zoom_Wheel_Steps(_WheelRemainder, wheel);
	if (steps != 0) {
		_Pending = Battle_Zoom_Nearest_Allowed(BattlefieldRect, std::clamp(_Pending + steps, (int)BATTLE_ZOOM_MIN, (int)BATTLE_ZOOM_MAX));
		_Anchor = frame;
		_HasAnchor = true;
	}

	return(true);
}


/// <summary>
/// Tracks a mouse button press or release. While the button that started a press over the
/// battlefield is held, the pointer stays mapped onto the battlefield, so a drag that leaves
/// it ends at the edge of the view.
/// </summary>
/// <param name="down">Is this a press?</param>
/// <param name="frame">The pointer position in frame pixels.</param>
void Battle_Zoom_Mouse_Button(bool down, Point2D const & frame)
{
	if (down) {
		if (_ButtonsHeld++ == 0) {
			_DragInBattlefield = BattlefieldRect.Is_Point_Within(frame);
		}
	} else if (_ButtonsHeld > 0 && --_ButtonsHeld == 0) {
		_DragInBattlefield = false;
	}
}


/// <summary>
/// Forgets every held mouse button, as when the window loses the keyboard focus.
/// </summary>
void Battle_Zoom_Release_Buttons(void)
{
	_ButtonsHeld = 0;
	_DragInBattlefield = false;
}


/// <summary>
/// Converts a frame position into the coordinates the game uses. A position over the zoomed
/// battlefield becomes a native view position; any other position is left alone.
/// </summary>
/// <param name="point">The position to convert in place.</param>
void Battle_Zoom_Frame_To_View(Point2D & point)
{
	if (!Is_Zoomed()) {
		return;
	}

	if (_DragInBattlefield) {
		point.X = std::clamp(point.X, BattlefieldRect.X, BattlefieldRect.X + BattlefieldRect.Width - 1);
		point.Y = std::clamp(point.Y, BattlefieldRect.Y, BattlefieldRect.Y + BattlefieldRect.Height - 1);
	}

	point = Battle_Zoom_Screen_To_View(point, BattlefieldRect, _Zoom);
}


/// <summary>
/// Converts a native view position back into the frame.
/// </summary>
/// <returns>Point2D; The frame position, or the position unchanged when the battlefield is not
/// zoomed or the position lies outside the native view.</returns>
Point2D Battle_Zoom_View_To_Frame(Point2D const & point)
{
	if (!Is_Zoomed()) {
		return(point);
	}

	return(Battle_Zoom_View_To_Screen(point, BattlefieldRect, _Zoom));
}


/// <summary>
/// Converts a native view position into the frame position the edge scroll tests. A position
/// on an edge of the native view lands on the same edge of the battlefield area, so the frame
/// pixels a whole-number magnification leaves unfilled still count as that edge.
/// </summary>
Point2D Battle_Zoom_Edge_Point(Point2D const & point)
{
	if (!Is_Zoomed() || !TacticalRect.Is_Point_Within(point)) {
		return(point);
	}

	Point2D frame = Battle_Zoom_View_To_Screen(point, BattlefieldRect, _Zoom);

	if (point.X == TacticalRect.X) {
		frame.X = BattlefieldRect.X;
	} else if (point.X == TacticalRect.X + TacticalRect.Width - 1) {
		frame.X = BattlefieldRect.X + BattlefieldRect.Width - 1;
	}

	if (point.Y == TacticalRect.Y) {
		frame.Y = BattlefieldRect.Y;
	} else if (point.Y == TacticalRect.Y + TacticalRect.Height - 1) {
		frame.Y = BattlefieldRect.Y + BattlefieldRect.Height - 1;
	}

	return(frame);
}


/// <summary>
/// Shows the composite surface with the battlefield magnified. The tab strip is copied as it
/// is, and chat and game messages are drawn at frame size over the magnified battlefield.
/// </summary>
/// <param name="surface">The surface being presented.</param>
/// <returns>bool; Was the surface shown here? It is not when the battlefield is not zoomed,
/// when the surface is not the composite surface, or while a trigger zoom is in effect.</returns>
bool Battle_Zoom_Present(Surface & surface)
{
	if (!Is_Zoomed() || &surface != CompositeSurface || VisibleSurface == NULL || TacticalMap == NULL || TacticalMap->ZoomFactor != 1.0) {
		return(false);
	}

	Rect strip(0, 0, std::min(surface.Get_Width(), BattlefieldRect.X + BattlefieldRect.Width), BattlefieldRect.Y);
	if (strip.Is_Valid()) {
		VisibleSurface->Blit_From(strip, surface, strip, false, true);
	}

	// Screen shake moves the magnified picture by whole native pixels.
	Rect const screen = Battle_Zoom_Screen_Rect(BattlefieldRect, _Zoom);
	Rect const source = TacticalRect - Point2D(Map.ScreenX / _Zoom, Map.ScreenY / _Zoom);
	Rect const shown = Intersect(source, TacticalRect);

	Rect const dest(screen.X + (shown.X - source.X) * _Zoom, screen.Y + (shown.Y - source.Y) * _Zoom, shown.Width * _Zoom, shown.Height * _Zoom);
	if (!(dest == BattlefieldRect)) {
		VisibleSurface->Fill_Rect(BattlefieldRect, 0);
	}

	if (shown.Is_Valid()) {
		VisibleSurface->Blit_From(dest, surface, shown, false, true);
	}

	Surface * oldpage = LogicalSurface;
	LogicalSurface = VisibleSurface;
	Session.Messages.Draw();
	LogicalSurface = oldpage;

	return(true);
}
