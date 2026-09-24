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

/* $Header: /CounterStrike/GAMEDLG.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : GAMEDLG.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Maria del Mar McCready Legg, Joe L. Bostic                   *
 *                                                                                             *
 *                   Start Date : Jan 8, 1995                                                  *
 *                                                                                             *
 *                  Last Update : Jan 18, 1995   [MML]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   OptionsClass::Process -- Handles all the options graphic interface.                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "gamedlg.h"

#include "dbgprint.h"
#include "globals.h"
#include "language/language.h"
#include "ui/screens/gamectrl/uigamectrl.h"

int GameSpeedNames[OptionsClass::MAX_SPEED_SETTING] = {
	TXT_SLOWEST,
	TXT_SLOWER,
	TXT_SLOW,
	TXT_MEDIUM,
	TXT_FAST,
	TXT_FASTER,
	TXT_FASTEST
};

int GameScrollSpeedNames[OptionsClass::MAX_SCROLL_SETTING] = {
	TXT_SLOWEST,
	TXT_SLOWER,
	TXT_SLOW,
	TXT_MEDIUM,
	TXT_FAST,
	TXT_FASTER,
	TXT_FASTEST
};

int GameDetailLevelNames[OptionsClass::MAX_DETAIL_SETTING] = {
	TXT_LOW,
	TXT_MEDIUM,
	TXT_HIGH
};

int GameDifficultyNames[OptionsClass::MAX_DIFFICULTY_SETTING] = {
	TXT_EASY,
	TXT_NORMAL,
	TXT_HARD
};


void GameControlsClass::Dialog(void)
{
	DebugString("GameControls: GameSpeed = %d, ScrollRate = %d, Detail = %d\n", Options.GameSpeed, Options.ScrollRate, Options.DetailLevel);

	UI_Game_Controls_Dialog();

	DebugString("GameControls: GameSpeed = %d, ScrollRate = %d, Detail = %d\n", Options.GameSpeed, Options.ScrollRate, Options.DetailLevel);
}
