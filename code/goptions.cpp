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

/* $Header: /counterstrike/GOPTIONS.CPP 6     3/15/97 7:18p Steve_tall $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : OPTIONS.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : June 8, 1994                                                 *
 *                                                                                             *
 *                  Last Update : July 27, 1995 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   OptionsClass::Process -- Handles all the options graphic interface.                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "goptions.h"

#include "_keyboar.h"
#include "_map.h"
#include "_xmouse.h"
#include "data.h"
#include "language/language.h"
#include "restate.h"
#include "scenario.h"
#include "ui/screens/abort/uiabort.h"
#include "ui/screens/gameopt/uigameopt.h"


/// <summary>
/// Displays the in game options dialog.
/// This routine is used by the special dialog handler when the player calls up the options
/// screen. Which dialog appears depends on the kind of game in progress. Game input stays
/// locked out for as long as the dialog is up, and if the player asked for the mission
/// briefing it is restated on the way out.
/// </summary>
void Game_Options_Dialog(void)
{
	int rc = IDOK;

	IgnoreInput = true;
	Keyboard->Clear();

	switch (UI_Game_Options_Dialog()) {
		case UI_GAME_OPTIONS_CONTROLS:
			SpecialDialog = SDLG_SETTINGS;
			break;

		case UI_GAME_OPTIONS_BRIEFING:
			rc = IDC_BRIEFING;
			break;

		case UI_GAME_OPTIONS_LOAD:
			rc = IDC_LOAD_GAME;
			break;

		case UI_GAME_OPTIONS_ABORT:
			SpecialDialog = SDLG_ABORT;
			rc = IDCANCEL;
			break;

		default:
			break;
	}

	Keyboard->Clear();

	if (rc == IDC_BRIEFING) {
		Restate_Mission(Scen);
	}

	IgnoreInput = Scen->IsInputLocked;

	if (rc == IDC_LOAD_GAME) {
		if (IDC_LOAD_GAME) {
			if (MouseCursor->Is_Hidden() == false && Scen->IsInputLocked == 1) {
				Hide_Mouse();
			} else if (MouseCursor->Is_Hidden() == true && Scen->IsInputLocked == 0) {
				Show_Mouse();
			}
		}
	}

	Map.Flag_To_Redraw(GS_REDRAW_ALL);
}


int Network_Quality_Text_ID(NetTiming::ConnectionQuality quality)
{
	switch (quality) {
		case NetTiming::ConnectionQuality::Fast: return(TXT_BEST_CONNECTION);
		case NetTiming::ConnectionQuality::Normal: return(TXT_GOOD_CONNECTION);
		case NetTiming::ConnectionQuality::Poor: return(TXT_POOR_CONNECTION);
		case NetTiming::ConnectionQuality::Bad: return(TXT_WORST_CONNECTION);
	}
	return(TXT_WORST_CONNECTION);
}


/// <summary>
/// Displays the abort mission dialog and waits for an answer.
/// This routine is used by the special dialog handler when the player asks to abandon or
/// surrender the mission. It does not return until the player has settled on one of the
/// choices offered.
/// </summary>
/// <returns>Returns with IDOK to quit the mission, IDABORT to restart or surrender it, or
/// IDCANCEL to carry on playing.</returns>
int Abort_Dialog(void)
{
	switch (UI_Abort_Dialog()) {
		case UI_ABORT_QUIT:
			return(IDOK);

		case UI_ABORT_RESTART:
			return(IDABORT);

		default:
			return(IDCANCEL);
	}
}
