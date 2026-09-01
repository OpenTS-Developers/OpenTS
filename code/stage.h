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

/* $Header: /CounterStrike/STAGE.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : STAGE.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : June 17, 1994                                                *
 *                                                                                             *
 *                  Last Update : June 17, 1994   [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "ftimer.h"
#include "timer.h"

class StageClass {

		/*
		**	This handles the animation stage of the object. This includes smoke, walking,
		**	flapping, and rocket flames.
		*/
		unsigned Stage;

		/*
		**	This is the countdown timer for stage animation. When this counts down
		**	to zero, then the stage increments by one and the time cycle starts
		**	over again.
		*/
		CDTimerClass<FrameTimerClass> Timer;

		/*
		**	This is the value to assign the StageTimer whenever it needs to be reset. Thus,
		**	this value is the control of how fast the stage value increments.
		*/
		int Rate;

		/*
		 * This is the amount that the stage advances by whenever the timer expires. It is
		 * normally one, but a negative step runs the animation backwards and a step of
		 * zero holds it on whatever stage it had reached.
		 */
		int Step;

	public:
		StageClass(void) : Stage(0), Timer(0), Rate(0), Step(1) {};

		int Fetch_Stage(void) const {return(Stage);};
		int Fetch_Rate(void) const {return(Rate);};
		int Fetch_Step(void) const {return(Step);};
		void Set_Stage(int stage) {Stage = stage;};
		void Set_Rate(int rate) {Timer = rate; Rate = rate;};
		void Set_Step(int step) {Step = step;};
		void Adjust_Rate(int rate) {if (rate != Rate) Rate = rate;}
		void Just_Set_Rate(int rate) {Rate = rate;}
		void AI(void) {};

		// Carries the animation stage to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Stage);
			stream.Serialize(Timer);
			stream.Serialize(Rate);
			stream.Serialize(Step);
		}

		bool About_To_Change(void) const {return(Timer == 0 && Rate != 0);}
		bool Graphic_Logic(void) {
			if (About_To_Change()) {
				Stage += Step;
				Timer = Rate;
				return(true);
			}
			return(false);
		};

};
