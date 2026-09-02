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

/* $Header: /CounterStrike/CREDITS.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CREDITS.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 17, 1994                                               *
 *                                                                                             *
 *                  Last Update : March 13, 1995 [JLB]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CreditClass::AI -- Handles updating the credit display.                                   *
 *   CreditClass::CreditClass -- Default constructor for the credit class object.              *
 *   CreditClass::Graphic_Logic -- Handles the credit redraw logic.                            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "credits.h"

#include "_bench.h"
#include "_map.h"
#include "_rules.h"
#include "_surface.h"
#include "bench.h"
#include "dialog.h"
#include "dsurface.h"
#include "globals.h"
#include "house.h"
#include "language/language.h"
#include "rules.h"
#include "scenario.h"
#include "scheme.h"
#include "tab.h"
#include "voc.h"
#include "vox.h"

#include "bench.hh"
#include "color.hh"

#include <algorithm>


/***********************************************************************************************
 * CreditClass::CreditClass -- Default constructor for the credit class object.                *
 *                                                                                             *
 *    This is the constructor for the credit class object. It merely sets the credit display   *
 *    state to null.                                                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
CreditClass::CreditClass(void) :
	Credits(0),
	Current(0),
	IsToRedraw(false),
	IsUp(false),
	IsAudible(false),
	Countdown(0)
{
}


/***********************************************************************************************
 * CreditClass::Graphic_Logic -- Handles the credit redraw logic.                              *
 *                                                                                             *
 *    This routine should be called whenever the main game screen is to be updated. It will    *
 *    check to see if the credit display should be redrawn. If so, it will redraw it.          *
 *                                                                                             *
 * INPUT:   forced   -- Should the credit display be redrawn regardless of whether the redraw  *
 *                      flag is set? This is typically the case when the screen needs to be    *
 *                      redrawn from scratch.                                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void CreditClass::Graphic_Logic(bool forced)
{
	if (forced || IsToRedraw) {
		BStart(BENCH_TABS);

		int xx = SidebarSurface->Get_Width() / 2;

		/*
		**	Play a sound effect when the money display changes, but only if a sound
		**	effect was requested.
		*/
		if (IsAudible) {
			if (IsUp) {
				Sound_Effect(VocType(Rule->CreditTicks[0]), .5);
			} else  {
				Sound_Effect(VocType(Rule->CreditTicks[1]), .5);
			}
		}

		/*
		**	Display the new current value.
		*/
		TabClass::Draw_Credits_Tab();

		if (PlayerPtr->IsObserver) {
			int hours = Current / 3600;
			int minutes = (Current / 60) % 60;
			int seconds = Current % 60;
			if (hours != 0) {
				Fancy_Text_Print(TXT_TIME_FORMAT_HOURS, *SidebarSurface, SidebarSurface->Get_Rect(), Point2D(xx, 0), ColorSchemes[0], TBLACK, TextPrintType(TPF_USE_GRAD_PAL|TPF_CENTER|TPF_METAL12), hours, minutes, seconds);
			} else {
				Fancy_Text_Print(TXT_TIME_FORMAT_NO_HOURS, *SidebarSurface, SidebarSurface->Get_Rect(), Point2D(xx, 0), ColorSchemes[0], TBLACK, TextPrintType(TPF_USE_GRAD_PAL|TPF_CENTER|TPF_METAL12), minutes, seconds);
			}
		} else {
			Fancy_Text_Print("%ld", *SidebarSurface, SidebarSurface->Get_Rect(), Point2D(xx, 0), ColorSchemes[0], TBLACK, TextPrintType(TPF_USE_GRAD_PAL|TPF_CENTER|TPF_METAL12), Current);
		}

		if (Scen->MissionTimer.Is_Active()) {
			int secs = Scen->MissionTimer / TICKS_PER_SECOND;
			int mins = secs / 60;
			int hours = mins / 60;
			secs %= 60;
			mins %= 60;

			/*
			**	Speak mission timer reminders.
			*/
			VoxType vox = VOX_NONE;
			switch ((int)Scen->MissionTimer) {
				case (1 * TICKS_PER_MINUTE):
					vox = VOX_TIME_1;
					break;
				case (2 * TICKS_PER_MINUTE):
					vox = VOX_TIME_2;
					break;
				case (3 * TICKS_PER_MINUTE):
					vox = VOX_TIME_3;
					break;
				case (4 * TICKS_PER_MINUTE):
					vox = VOX_TIME_4;
					break;
				case (5 * TICKS_PER_MINUTE):
					vox = VOX_TIME_5;
					break;
				case (10 * TICKS_PER_MINUTE):
					vox = VOX_TIME_10;
					break;
				case (20 * TICKS_PER_MINUTE):
					vox = VOX_TIME_20;
					break;
			}
			if (vox != VOX_NONE) {
				Speak(vox);
				Map.FlasherTimer = 7;
			}
		}

		IsToRedraw = false;
		IsAudible = false;
		BEnd(BENCH_TABS);
	}
}


/***********************************************************************************************
 * CreditClass::AI -- Handles updating the credit display.                                     *
 *                                                                                             *
 *    This routine handles the logic that controls the rate of credit change in the credit     *
 *    display. It doesn't actually redraw the credit display, but will flag it to be redrawn   *
 *    if it detects that a change is to occur.                                                 *
 *                                                                                             *
 * INPUT:   forced   -- Should the credit display immediately reflect the current credit       *
 *                      total for the player? This is usually desired when initially loading   *
 *                      a scenario or saved game.                                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void CreditClass::AI(bool forced)
{
	static int _last = 0;

	if (!forced && Frame == _last) return;
	_last = Frame;

	/*
	 * An observer has no money to count; the readout keeps the elapsed match time instead,
	 * silently and to the second.
	 */
	if (PlayerPtr->IsObserver) {
		Credits = Scen->ElapsedTimer / TIMER_SECOND;
		if (Current != Credits) {
			Current = Credits;
			IsAudible = false;
			IsToRedraw = true;
			Map.Flag_To_Redraw();
			Map.IsToRedrawCredits = true;
		}
		return;
	}

	Credits = PlayerPtr->Available_Money();

	/*
	**	Make sure that the credit counter doesn't drop below zero.
	*/
	Credits = std::max(Credits, 0);

	if (Scen->MissionTimer.Is_Active() || Scen->MissionTimer) {
		IsToRedraw = true;
		Map.Flag_To_Redraw();
	}

	if (Current == Credits) return;

	if (forced) {
		IsAudible = false;
		Current = Credits;
	} else {

		if (Countdown) Countdown--;
		if (Countdown) return;

		/*
		**	Determine the amount to change the display toward the
		**	desired value.
		*/
		int adder = Credits - Current;

		if (adder > 0) {
			Countdown = 1;
		} else {
			Countdown = 3;
		}

		adder = abs(adder);
		adder >>= 3;
//		adder >>= 4;
//		adder >>= 5;
		adder = std::clamp(adder, 1, 71+72);
		if (Current > Credits) adder = -adder;
		Current += adder;
		if (Current-adder != Current) {
			IsAudible = true;
			IsUp = (adder > 0);
		}
	}
	IsToRedraw = true;
	Map.Flag_To_Redraw();
	Map.IsToRedrawCredits = true;
}
