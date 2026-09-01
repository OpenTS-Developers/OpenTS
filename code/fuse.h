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

/* $Header: /CounterStrike/FUSE.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FUSE.H                                                       *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 24, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 24, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <climits>

#include "coord.h"
#include "ftimer.h"
#include "timer.h"


enum FuseResultType {
	FUSE_WAIT,
	FUSE_EXPLODE_CLOSE,
	FUSE_EXPLODE_FAR,
};


/****************************************************************************
**	The fuse is used by projectiles to determine whether detonation should
**	occur. This is usually determined by tracking the distance to the
**	designated target reaches zero or when the timer expires.
*/
class FuseClass {
	public:
		FuseClass(void);
		~FuseClass(void) {};

		void Arm_Fuse(Coord const & location, Coord const & target, int arming=0, int time=INT_MAX);
		FuseResultType Fuse_Checkup(Coord const & newlocation);
		Coord Fuse_Target(void) {return(HeadTo);}

		/*
		**	Fuses can detonate if enough time has elapsed. This value counts
		**	down. When it reaches zero, detonation occurs.
		*/
		CDTimerClass<FrameTimerClass> Timer;

		// Carries the fuse to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Timer);
			stream.Serialize(Arming);
			stream.Serialize(HeadTo);
			stream.Serialize(Proximity);
		}

	private:

		/*
		**	Some fuses need a certain amount of time before detonation can
		**	occur. This counts down and when it reaches zero, normal fuse
		**	detonation checking can occur.
		*/
		CDTimerClass<FrameTimerClass> Arming;

		/*
		**	This is the designated impact point of the projectile. The fuse
		**	will trip when the closest point to this location has been reached.
		*/
		Coord HeadTo;

		/*
		**	This is the running proximity value to the impact point. This value
		**	will progressively get smaller. Detonation occurs when it reaches
		**	zero or when it starts to grow larger.
		*/
		int Proximity;
};
