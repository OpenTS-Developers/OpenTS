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

/* $Header: /CounterStrike/POWER.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : POWER.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 12/15/94                                                     *
 *                                                                                             *
 *                  Last Update : December 15, 1994 [JLB]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "radar.h"


class PowerClass : public RadarClass
{
		typedef RadarClass BASECLASS;

	public:
		PowerClass(void);

		virtual void Serialize(SaveStreamClass & stream) override;

		/*
		**	Initialization
		*/
		virtual void One_Time(void) override;							// One-time inits

		virtual void Init_Clear(void) override;						// Clears all to known state
		virtual void Draw_It(bool complete=false) override;
		virtual void AI(KeyNumType &input, Point2D const & xy) override;
		void Flash_Power(void);

		virtual void Init_For_House(void) override;
		virtual void Reposition_Sidebar(void) override;
		virtual char const * Help_Text(int id) override;

		bool IsToRedraw;

	protected:
		enum PowerEnums {
			GADGET_POWER=999,
			POWER_X=8,
			POWER_Y=25,
			POWER_WIDTH=12,
			POWER_PIP_HEIGHT=4,
			POWER_PIP_EMPTY=0,
			POWER_PIP_GREEN=1,
			POWER_PIP_YELLOW=2,
			POWER_PIP_RED=3,
			POWER_PIP_WHITE=4,
			POWER_FLASH_COUNT=10,			/// Number of flash toggles when the power level changes.
			POWER_FLASH_RATE=3,				/// Delay between power bar flash toggles.
		};

	private:

		int Max_Power_Height(void);
		int Desired_Power_Height(void);
		int Desired_Levels(int & green, int & yellow, int & red);
		int Update_Delay(void);
		void Remove_Pip(void);
		void Add_Pip(void);

		/*
		**	If the power bar should be rendered with some flash effect then
		**	this specifies the duration that the flash will occur.
		*/
		CDTimerClass<SystemTimerClass> FlashTimer;
		int FlashCount;

		/*
		 * This is the delay before the next pip may be added to or taken from the power bar,
		 * so that a change in the power level reads as an animation rather than a jump.
		 */
		CDTimerClass<SystemTimerClass> UpdateTimer;

		/*
		 * These are the number of pips of each color the power bar is currently showing. The
		 * green band stands for the surplus power beyond the first hundred units, the yellow
		 * band for that first hundred, and the red band for the power already being consumed.
		 */
		int GreenPipCount;
		int YellowPipCount;
		int RedPipCount;

		/*
		 * If the power bar has not yet settled at the level it should be showing, then this
		 * flag will be true. It keeps the bar stepping toward that level and holds the flash
		 * off until the pips have stopped moving.
		 */
		bool HasChanged;

		int RecordedDrain;
		int RecordedPower;

		/*
		**	Points to the shape to use for the "desired" power level indicator.
		*/
		static void const * PowerPipShape;
};
