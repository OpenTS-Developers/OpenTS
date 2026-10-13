/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "battlezoomlayout.h"
#include "windowevent.hh"


class BattleZoomStateClass
{
public:
	int Factor(void) const;
	Rect Fit_View(Rect const & battlefield, bool scenario_active);
	bool Wheel(Rect const & battlefield, bool can_zoom, float wheel, Point2D const & frame);
	bool Apply_Pending(Rect const & battlefield, bool can_zoom, bool input_pending, Point2D & frame);
	void Cancel_Pending(void);
	void Reset(void);
	void Start_At(Rect const & battlefield, int zoom);

	static unsigned Button_Mask(WindowMouseButton button);
	void Mouse_Button(WindowMouseButton button, bool down, Rect const & battlefield, Point2D const & frame);
	void Reconcile_Buttons(unsigned held);
	void Release_Buttons(void);
	bool Dragging(void) const;

private:
	int Zoom = BATTLE_ZOOM_MIN;
	int Pending = BATTLE_ZOOM_MIN;
	bool HasAnchor = false;
	Point2D Anchor = Point2D(0, 0);
	float WheelRemainder = 0.0f;
	unsigned ButtonsHeld = 0;
	unsigned DragButton = 0;
};
