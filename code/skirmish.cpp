/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "skirmish.h"

#include "_xmouse.h"
#include "houstype.h"
#include "init.h"
#include "netshare.h"
#include "newmenu.h"
#include "session.h"
#include "ui/screens/skirmish/uiskirmish.h"


/// <summary>
/// Handles the skirmish game setup dialog.
/// This routine is used by the main menu when the player picks a skirmish game. The house
/// and side rules are re-read first so that the dialog offers the current playable sides,
/// and the chosen settings are recorded as the player's multiplayer preferences on the way
/// out.
/// </summary>
/// <returns>bool; Did the player accept the settings and ask for the game to start?</returns>
bool Skirmish_Mode_Dialog(void)
{
	Prepare_Side_Roster();

	Hide_Mouse();
	Draw_Menu_Background();
	Show_Mouse();

	bool started = UI_Skirmish_Dialog();

	if (MultiplayerMapPreview != NULL) {
		delete MultiplayerMapPreview;
		MultiplayerMapPreview = NULL;
	}

	Session.Write_MultiPlayer_Settings();

	if (!started) {
		Hide_Mouse();
		Draw_Menu_Background();
		Show_Mouse();
	}

	return(started);
}
