/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

/* $Header: /CounterStrike/FACING.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FACING.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 03/21/95                                                     *
 *                                                                                             *
 *                  Last Update : March 21, 1995 [JLB]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   FacingClass::FacingClass -- Default constructor for the facing class.                     *
 *   FacingClass::Rotation_Adjust -- Perform a rotation adjustment to current facing.          *
 *   FacingClass::Set_Current -- Sets the current rotation value.                              *
 *   FacingClass::Set_Desired -- Sets the desired facing  value.                               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "facing.h"

#include "syncrechook.h"

#include <algorithm>
#if defined(_MSC_VER)
#include <intrin.h>
#endif


/***********************************************************************************************
 * FacingClass::FacingClass -- Default constructor for the facing class.                       *
 *                                                                                             *
 *    This default constructor merely sets the desired and current facing values to be the     *
 *    same (North).                                                                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
FacingClass::FacingClass(void) :
	DesiredFacing(DIR_N),
	StartFacing(DIR_N),
	ROT(DIR_N)
{
}


/// <summary>
/// Constructor for the facing class that presets the rate of turn.
/// This constructor starts the desired and current facing values out the same (North),
/// then submits the rate specified to Set_ROT.
/// </summary>
/// <param name="rate">The rate of turn to assign, expressed in binary angle units.</param>
FacingClass::FacingClass(int rate) :
	DesiredFacing(DIR_N),
	StartFacing(DIR_N),
	ROT(DIR_N)
{
	Set_ROT(rate);
}


/***********************************************************************************************
 * FacingClass::Set_Desired -- Sets the desired facing  value.                                 *
 *                                                                                             *
 *    This routine is used to set the desired facing value without altering the current        *
 *    facing setting. Typical use of this routine is when a vehicle needs to face a            *
 *    direction, but currently isn't facing the correct direction. After this routine is       *
 *    called, it is presumed that subsequent calls to Rotation_Adjust() will result in the     *
 *    eventual alignment of the current facing.                                                *
 *                                                                                             *
 * INPUT:   facing   -- The new facing to assign to the desired value.                         *
 *                                                                                             *
 * OUTPUT:  bool; Did the desired facing value actually change by this routine call?           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FacingClass::Set_Desired(DirType const & facing)
{
	if (DesiredFacing != facing) {
		StartFacing = Current();
		DesiredFacing = facing;

		if (ROT.Facing > 0) {
			RotationTimer = abs(Turn_Arc()) / ROT.Facing;
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Snaps both the current and the desired facing to the value specified.
/// Use this routine when an object must be pointed in a direction outright rather than
/// turning to it over time. Any turn in progress is abandoned.
/// </summary>
/// <param name="facing">The new facing to snap to.</param>
/// <returns>bool; Did the facing actually change?</returns>
bool FacingClass::Set(DirType const & facing)
{
	Sync_Record_Facing(facing, (unsigned)(uintptr_t)_ReturnAddress());

	if (Current() != facing) {
		DesiredFacing = facing;
		StartFacing = facing;
		RotationTimer = 0;
		return(true);
	}

	RotationTimer = 0;
	return(false);
}


/// <summary>
/// Fetches the facing as of this moment.
/// While a turn is under way, the facing is interpolated between the facing the turn
/// started from and the facing it is heading for, according to how much of the turn
/// remains. Once the turn is over, the desired facing is what is returned.
/// </summary>
/// <returns>Returns with the facing the object is presently turned to.</returns>
DirType FacingClass::Current(void) const
{
	if (Is_Rotating()) {
		short diff = Turn_Arc();
		short rot = abs(diff) / ROT.Facing;
		DirType dir = DesiredFacing;

		if (rot > 0) {
			dir.Facing -= RotationTimer * (diff / rot);
		}
		return(dir);
	}
	return(DesiredFacing);
}


/// <summary>
/// Fetches the facing that is being turned toward.
/// This is the facing the object will settle at once any rotation under way has run
/// its course.
/// </summary>
DirType FacingClass::Desired(void) const
{
	return(DesiredFacing);
}


/// <summary>
/// Is the facing in the middle of a turn?
/// A facing with no rate of turn assigned never rotates -- it arrives at whatever
/// facing is desired the moment it is asked for.
/// </summary>
bool FacingClass::Is_Rotating(void) const
{
	return(ROT.Facing > 0 && RotationTimer);
}


/// <summary>
/// Is the facing turning clockwise?
/// Use this routine when the direction of the turn matters and not merely the fact of
/// it. A facing that is not turning at all answers false.
/// </summary>
bool FacingClass::Is_Rotating_CW(void) const
{
	if (Is_Rotating()) {
		return(Turn_Arc() > 0);
	}
	return(false);
}


/// <summary>
/// Is the facing turning counterclockwise?
/// Use this routine when the direction of the turn matters and not merely the fact of
/// it. A facing that is not turning at all answers false.
/// </summary>
bool FacingClass::Is_Rotating_CCW(void) const
{
	if (Is_Rotating()) {
		return(Turn_Arc() < 0);
	}
	return(false);
}


/// <summary>
/// Determines how much turning remains to be done.
/// </summary>
/// <returns>Returns with the difference between the desired facing and the facing as of
/// this moment.</returns>
DirType FacingClass::Difference(void)
{
	return(Desired() - Current());
}


/// <summary>
/// Determines how far this facing is turned away from another.
/// </summary>
/// <param name="facing">The facing to measure against.</param>
/// <returns>Returns with the difference between the facing specified and the facing as
/// of this moment.</returns>
DirType FacingClass::Difference(DirType const & facing)
{
	return(facing - Current());
}


/// <summary>
/// Sets the rate of turn for this facing.
/// This routine is used when an object's turning speed is established or changed. The
/// rate submitted is clamped to just under a half circle.
/// </summary>
/// <param name="rate">The rate of turn, expressed in binary angle units.</param>
void FacingClass::Set_ROT(int rate)
{
	ROT.From_Dir256((Dir256)std::min(rate, DIR_S - 1));
}
