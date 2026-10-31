/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "uicontrol.h"

#include "ccfile.h"
#include "ccini.h"


namespace {

char const * const INGAME = "Ingame";
char const * const PIPS = "Pips";

}


/// <summary>
/// Reads every value the file names and keeps the current value of any it leaves out.
/// </summary>
bool UIControlsClass::Read_INI(CCINIClass const & ini)
{
	IsAlwaysShowActionLines = ini.Get_Bool(INGAME, "AlwaysShowActionLines", IsAlwaysShowActionLines);

	IsMovementLineDashed = ini.Get_Bool(INGAME, "MovementLineDashed", IsMovementLineDashed);
	IsMovementLineDropShadow = ini.Get_Bool(INGAME, "MovementLineDropShadow", IsMovementLineDropShadow);
	IsMovementLineThick = ini.Get_Bool(INGAME, "MovementLineThick", IsMovementLineThick);
	MovementLineColor = ini.Get_RGBClass(INGAME, "MovementLineColor", MovementLineColor);
	MovementLineDropShadowColor = ini.Get_RGBClass(INGAME, "MovementLineDropShadowColor", MovementLineDropShadowColor);

	IsTargetLineDashed = ini.Get_Bool(INGAME, "TargetLineDashed", IsTargetLineDashed);
	IsTargetLineDropShadow = ini.Get_Bool(INGAME, "TargetLineDropShadow", IsTargetLineDropShadow);
	IsTargetLineThick = ini.Get_Bool(INGAME, "TargetLineThick", IsTargetLineThick);
	TargetLineColor = ini.Get_RGBClass(INGAME, "TargetLineColor", TargetLineColor);
	TargetLineDropShadowColor = ini.Get_RGBClass(INGAME, "TargetLineDropShadowColor", TargetLineDropShadowColor);

	IsTargetLaserDashed = ini.Get_Bool(INGAME, "TargetLaserDashed", IsTargetLaserDashed);
	IsTargetLaserDropShadow = ini.Get_Bool(INGAME, "TargetLaserDropShadow", IsTargetLaserDropShadow);
	IsTargetLaserThick = ini.Get_Bool(INGAME, "TargetLaserThick", IsTargetLaserThick);
	TargetLaserColor = ini.Get_RGBClass(INGAME, "TargetLaserColor", TargetLaserColor);
	TargetLaserDropShadowColor = ini.Get_RGBClass(INGAME, "TargetLaserDropShadowColor", TargetLaserDropShadowColor);
	TargetLaserTime = ini.Get_Int(INGAME, "TargetLaserTime", TargetLaserTime);

	IsShowNavComQueueLines = ini.Get_Bool(INGAME, "ShowNavComQueueLines", IsShowNavComQueueLines);
	IsNavComQueueLineDashed = ini.Get_Bool(INGAME, "NavComQueueLineDashed", IsNavComQueueLineDashed);
	IsNavComQueueLineDropShadow = ini.Get_Bool(INGAME, "NavComQueueLineDropShadow", IsNavComQueueLineDropShadow);
	IsNavComQueueLineThick = ini.Get_Bool(INGAME, "NavComQueueLineThick", IsNavComQueueLineThick);
	NavComQueueLineColor = ini.Get_RGBClass(INGAME, "NavComQueueLineColor", NavComQueueLineColor);
	NavComQueueLineDropShadowColor = ini.Get_RGBClass(INGAME, "NavComQueueLineDropShadowColor", NavComQueueLineDropShadowColor);

	InfantryHealthBarPosOffset = ini.Get_Point(INGAME, "InfantryHealthBarPos", InfantryHealthBarPosOffset);
	UnitHealthBarPosOffset = ini.Get_Point(INGAME, "UnitHealthBarPos", UnitHealthBarPosOffset);

	UnitGroupNumberOffset = ini.Get_Point(PIPS, "UnitGroupNumberOffset", UnitGroupNumberOffset);
	InfantryGroupNumberOffset = ini.Get_Point(PIPS, "InfantryGroupNumberOffset", InfantryGroupNumberOffset);
	BuildingGroupNumberOffset = ini.Get_Point(PIPS, "BuildingGroupNumberOffset", BuildingGroupNumberOffset);
	AircraftGroupNumberOffset = ini.Get_Point(PIPS, "AircraftGroupNumberOffset", AircraftGroupNumberOffset);
	UnitWithPipGroupNumberOffset = ini.Get_Point(PIPS, "UnitWithPipGroupNumberOffset", UnitWithPipGroupNumberOffset);
	InfantryWithPipGroupNumberOffset = ini.Get_Point(PIPS, "InfantryWithPipGroupNumberOffset", InfantryWithPipGroupNumberOffset);
	BuildingWithPipGroupNumberOffset = ini.Get_Point(PIPS, "BuildingWithPipGroupNumberOffset", BuildingWithPipGroupNumberOffset);
	AircraftWithPipGroupNumberOffset = ini.Get_Point(PIPS, "AircraftWithPipGroupNumberOffset", AircraftWithPipGroupNumberOffset);

	return(true);
}


/// <summary>
/// Reads the named file through the game's file layer. A missing or unreadable file leaves
/// the current values alone, which after a reset are the defaults.
/// </summary>
/// <param name="reset">Return every value to its default before reading.</param>
/// <returns>bool; Was the file read?</returns>
bool UIControlsClass::Read_INI_File(char const * filename, bool reset)
{
	if (reset) {
		*this = UIControlsClass();
	}

	CCFileClass file(filename);
	if (!file.Is_Available()) {
		return(false);
	}

	CCINIClass ini;
	if (!ini.Load(file, false)) {
		return(false);
	}
	return(Read_INI(ini));
}


UILineStyleType UIControlsClass::Movement_Line_Style(void) const
{
	return(UILineStyleType{IsMovementLineDashed, IsMovementLineThick, IsMovementLineDropShadow, MovementLineColor, MovementLineDropShadowColor});
}


UILineStyleType UIControlsClass::Target_Line_Style(void) const
{
	return(UILineStyleType{IsTargetLineDashed, IsTargetLineThick, IsTargetLineDropShadow, TargetLineColor, TargetLineDropShadowColor});
}


UILineStyleType UIControlsClass::Target_Laser_Style(void) const
{
	return(UILineStyleType{IsTargetLaserDashed, IsTargetLaserThick, IsTargetLaserDropShadow, TargetLaserColor, TargetLaserDropShadowColor});
}


UILineStyleType UIControlsClass::Navigation_Queue_Line_Style(void) const
{
	return(UILineStyleType{IsNavComQueueLineDashed, IsNavComQueueLineThick, IsNavComQueueLineDropShadow, NavComQueueLineColor, NavComQueueLineDropShadowColor});
}


/// <summary>
/// Returns where an object's control group number is printed, relative to the corner its
/// pips start from. Any kind other than infantry, a structure or an aircraft uses the
/// vehicle offsets.
/// </summary>
/// <param name="has_pips">Does the object's type draw pips?</param>
Point2D UIControlsClass::Group_Number_Offset(RTTIType rtti, bool has_pips) const
{
	switch (rtti) {
		case RTTI_INFANTRY:
			return(has_pips ? InfantryWithPipGroupNumberOffset : InfantryGroupNumberOffset);

		case RTTI_BUILDING:
			return(has_pips ? BuildingWithPipGroupNumberOffset : BuildingGroupNumberOffset);

		case RTTI_AIRCRAFT:
			return(has_pips ? AircraftWithPipGroupNumberOffset : AircraftGroupNumberOffset);

		default:
			return(has_pips ? UnitWithPipGroupNumberOffset : UnitGroupNumberOffset);
	}
}
