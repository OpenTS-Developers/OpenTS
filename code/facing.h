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

/* $Header: /CounterStrike/FACING.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FACING.H                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 03/21/95                                                     *
 *                                                                                             *
 *                  Last Update : March 21, 1995 [JLB]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "face.h"
#include "ftimer.h"
#include "timer.h"
#include "win.h"

#include "facing.hh"

/*
**	This is a general facing handler class. It is used in those cases where facing needs to be
**	kept track of, but there could also be an associated desired facing. The current facing
**	is supposed to transition to the desired state over time. Using this class facilitates this
**	processing as well as isolating the rest of the code from the internals.
*/
class FacingClass
{
	public:
		FacingClass(void);
		FacingClass(int rate);

		DirType Current(void) const;
		DirType Desired(void) const;

		bool Set_Desired(DirType const & facing);
		void Set_ROT(int rate);

		bool Set(DirType const & facing);

		bool Is_Rotating(void) const;
		bool Is_Rotating_CW(void) const;
		bool Is_Rotating_CCW(void) const;
		DirType Difference(void);
		DirType Difference(DirType const & dir);

		/// The whole arc of the turn in progress, not the rotation left to do.
		int Turn_Arc(void) const {return((DesiredFacing - StartFacing).As_Int());}

		// Carries the facing to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(DesiredFacing);
			stream.Serialize(StartFacing);
			stream.Serialize(RotationTimer);
			stream.Serialize(ROT);
		}

	private:
		DirType DesiredFacing;

		/*
		 * This is the facing that the turn under way started from. The facing as of this
		 * moment is interpolated between it and DesiredFacing by the RotationTimer.
		 */
		DirType StartFacing;

		/*
		 * This is the number of game frames left before the turn under way completes, set
		 * to the arc of that turn divided by the rate of turn. While it is running, the
		 * facing reads as a point partway between StartFacing and DesiredFacing.
		 */
		CDTimerClass<FrameTimerClass> RotationTimer;

		/*
		 * This is the rate of turn, expressed as the binary angle stepped through per game
		 * frame and clamped to just under a half circle. If it is zero, then this facing
		 * never rotates -- it snaps to whatever facing is desired the moment one is set.
		 */
		DirType ROT;
};


inline FacingType Facing_Add(int first, int second)
{
	return((FacingType)((first + second) & 7));
}
inline FacingType Facing_Sub(int first, int second)
{
	return((FacingType)((first - second) & 7));
}
