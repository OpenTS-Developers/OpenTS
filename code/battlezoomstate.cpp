/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "battlezoomstate.hh"

#include <algorithm>


int BattleZoomStateClass::Factor(void) const
{
	return(Zoom);
}


Rect BattleZoomStateClass::Fit_View(Rect const & battlefield, bool scenario_active)
{
	Zoom = scenario_active ? Battle_Zoom_Nearest_Allowed(battlefield, Zoom) : (int)BATTLE_ZOOM_MIN;
	Pending = Zoom;
	HasAnchor = false;
	if (!scenario_active) {
		Cancel_Pending();
		Release_Buttons();
	}
	return(Battle_Zoom_View_Rect(battlefield, Zoom));
}


bool BattleZoomStateClass::Wheel(Rect const & battlefield, bool can_zoom, float wheel, Point2D const & frame)
{
	if (!can_zoom || !battlefield.Is_Point_Within(frame)) {
		WheelRemainder = 0.0f;
		return(false);
	}

	int const steps = Battle_Zoom_Wheel_Steps(WheelRemainder, wheel);
	if (steps != 0) {
		Pending = Battle_Zoom_Nearest_Allowed(battlefield, std::clamp(Pending + steps, (int)BATTLE_ZOOM_MIN, (int)BATTLE_ZOOM_MAX));
		Anchor = frame;
		HasAnchor = true;
	}
	return(true);
}


bool BattleZoomStateClass::Apply_Pending(Rect const & battlefield, bool can_zoom, bool input_pending, Point2D & frame)
{
	if (!can_zoom) {
		Cancel_Pending();
		return(false);
	}
	if (Pending == Zoom || input_pending || ButtonsHeld != 0) {
		return(false);
	}

	frame = HasAnchor ? Anchor : Point2D(battlefield.X + battlefield.Width / 2, battlefield.Y + battlefield.Height / 2);
	Zoom = Pending;
	HasAnchor = false;
	return(true);
}


void BattleZoomStateClass::Cancel_Pending(void)
{
	Pending = Zoom;
	HasAnchor = false;
	WheelRemainder = 0.0f;
}


void BattleZoomStateClass::Reset(void)
{
	Zoom = BATTLE_ZOOM_MIN;
	Cancel_Pending();
	Release_Buttons();
}


unsigned BattleZoomStateClass::Button_Mask(WindowMouseButton button)
{
	return(button >= WINDOW_BUTTON_LEFT && button <= WINDOW_BUTTON_X2 ? 1u << button : 0u);
}


void BattleZoomStateClass::Mouse_Button(WindowMouseButton button, bool down, Rect const & battlefield, Point2D const & frame)
{
	unsigned const mask = Button_Mask(button);
	if (down) {
		if (ButtonsHeld == 0 && battlefield.Is_Point_Within(frame)) {
			DragButton = mask;
		}
		ButtonsHeld |= mask;
	} else {
		ButtonsHeld &= ~mask;
		DragButton &= ButtonsHeld;
	}
}


void BattleZoomStateClass::Reconcile_Buttons(unsigned held)
{
	ButtonsHeld = held;
	DragButton &= held;
}


void BattleZoomStateClass::Release_Buttons(void)
{
	ButtonsHeld = 0;
	DragButton = 0;
}


bool BattleZoomStateClass::Dragging(void) const
{
	return(DragButton != 0);
}
