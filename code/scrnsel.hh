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

#pragma once

// Enums in Select_Game() must match order of buttons in Main_Menu().
enum {
	SEL_TIMEOUT = -1,				// main menu timeout--go into attract mode
	SEL_NEW_SCENARIO,				// Expansion scenario to play.
	SEL_CAMPAIGN_GAME,				// start a new game
	SEL_LOAD_GAME,					// load a saved game
	SEL_MULTIPLAYER_GAME,			// play modem/null-modem/network game
	SEL_INTRO,						// couch-potato mode
	SEL_OPTIONS,
	SEL_EXIT,						// exit to DOS
	SEL_FAME,						// view the hall o' fame
	SEL_INTERNET_RETURN,			// retired; slot kept to keep the button order aligned
	SEL_VIEW_CREDITS,
	SEL_VERSION,
	SEL_NONE,						// placeholder default value
};
