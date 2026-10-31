/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#pragma once

#include "point.h"
#include "rgb.h"
#include "rtti.hh"
#include "stimer.h"

class CCINIClass;


/*
 * How one kind of order line is drawn: dashed or solid, single or doubled, and whether a
 * shadow is laid below it.
 */
struct UILineStyleType
{
	bool IsDashed = false;
	bool IsThick = false;
	bool IsDropShadow = false;
	RGBClass Color;
	RGBClass DropShadowColor;
};


/*
 * The presentation settings UI.INI carries. Every value has a default, so the file is
 * optional and may name only what it changes.
 */
class UIControlsClass
{
	public:
		bool Read_INI(CCINIClass const & ini);
		bool Read_INI_File(char const * filename, bool reset);

		UILineStyleType Movement_Line_Style(void) const;
		UILineStyleType Target_Line_Style(void) const;
		UILineStyleType Target_Laser_Style(void) const;
		UILineStyleType Navigation_Queue_Line_Style(void) const;
		Point2D Group_Number_Offset(RTTIType rtti, bool has_pips) const;

		bool IsAlwaysShowActionLines = false;

		bool IsMovementLineDashed = false;
		bool IsMovementLineDropShadow = false;
		bool IsMovementLineThick = false;
		RGBClass MovementLineColor = RGBClass(0, 170, 0);
		RGBClass MovementLineDropShadowColor = RGBClass(0, 0, 0);

		bool IsTargetLineDashed = false;
		bool IsTargetLineDropShadow = false;
		bool IsTargetLineThick = false;
		RGBClass TargetLineColor = RGBClass(173, 0, 0);
		RGBClass TargetLineDropShadowColor = RGBClass(0, 0, 0);

		bool IsTargetLaserDashed = true;
		bool IsTargetLaserDropShadow = false;
		bool IsTargetLaserThick = false;
		RGBClass TargetLaserColor = RGBClass(173, 0, 0);
		RGBClass TargetLaserDropShadowColor = RGBClass(0, 0, 0);
		int TargetLaserTime = TICKS_PER_SECOND;

		bool IsShowNavComQueueLines = true;
		bool IsNavComQueueLineDashed = false;
		bool IsNavComQueueLineDropShadow = false;
		bool IsNavComQueueLineThick = false;
		RGBClass NavComQueueLineColor = RGBClass(74, 77, 255);
		RGBClass NavComQueueLineDropShadowColor = RGBClass(0, 0, 0);

		Point2D UnitGroupNumberOffset = Point2D(-4, -4);
		Point2D InfantryGroupNumberOffset = Point2D(-4, -4);
		Point2D BuildingGroupNumberOffset = Point2D(-4, -4);
		Point2D AircraftGroupNumberOffset = Point2D(-4, -4);
		Point2D UnitWithPipGroupNumberOffset = Point2D(-4, -8);
		Point2D InfantryWithPipGroupNumberOffset = Point2D(-4, -8);
		Point2D BuildingWithPipGroupNumberOffset = Point2D(-4, -8);
		Point2D AircraftWithPipGroupNumberOffset = Point2D(-4, -8);

		// Health bar drawing positions
		Point2D InfantryHealthBarPosOffset = Point2D(-5, -24);
		Point2D UnitHealthBarPosOffset = Point2D(-15, -25);
};
