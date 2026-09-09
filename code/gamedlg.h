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

/* $Header: /CounterStrike/GAMEDLG.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : GAMEDLG.H                                                    *
 *                                                                                             *
 *                   Programmer : Maria del Mar McCready Legg, Joe L. Bostic                   *
 *                                                                                             *
 *                   Start Date : Jan 8, 1995                                                  *
 *                                                                                             *
 *                  Last Update : Jan 18, 1995   [MML]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "gadget.h"
#include "goptions.h"

extern int GameSpeedNames[OptionsClass::MAX_SPEED_SETTING];
extern int GameScrollSpeedNames[OptionsClass::MAX_SCROLL_SETTING];
extern int GameDetailLevelNames[OptionsClass::MAX_DETAIL_SETTING];
extern int GameDifficultyNames[OptionsClass::MAX_DIFFICULTY_SETTING];

class GameControlsClass
{
	public:
		GameControlsClass(void) {};
		void Dialog(void);

		static int GetDifficultyLabel(int difficulty)
		{
			return(GameDifficultyNames[difficulty]);
		}

	private:
		/*
		 * This is the window handle of the game controls dialog while it is displayed. The
		 * player's settings are read back off its controls, so the handle is only
		 * meaningful between the dialog being created and destroyed.
		 */
		HWND _Dialog;
};
