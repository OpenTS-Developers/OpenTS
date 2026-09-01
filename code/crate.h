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

/* $Header: /CounterStrike/CRATE.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CRATE.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 08/26/96                                                     *
 *                                                                                             *
 *                  Last Update : August 26, 1996 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "coord.h"
#include "ftimer.h"
#include "globals.h"
#include "timer.h"


class CrateClass {
	public:
		CrateClass(void) : Location(0,0) {}
		void Init(void) {Make_Invalid();}
		bool Create_Crate(Cell const & cell);
		bool Is_Here(Cell const & cell) const {return(Is_Valid() && cell == Location);}
		bool Remove_It(void);
		bool Is_Expired(void) const {return(Is_Valid() && Timer == 0);}
		bool Is_Valid(void) const {return(Location != CELL_NONE);}

		// Carries the crate to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Timer);
			stream.Serialize(Location);
		}

	private:
		static bool Put_Crate(Cell & cell);
		static bool Get_Crate(Cell const & cell);

		void Make_Invalid(void) {Location = CELL_NONE;Timer.Stop();}

		CDTimerClass<FrameTimerClass> Timer;

		/*
		 * This is the cell the crate overlay was placed in. It is CELL_NONE while this
		 * crate object is not tracking a crate at all, so it doubles as the validity flag
		 * for the whole object.
		 */
		Cell Location;
};
