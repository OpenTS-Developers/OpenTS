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

/* $Header: /CounterStrike/MPLAYER.CPP 3     3/13/97 2:06p Steve_tall $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : MPLAYER.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Bill Randolph                                                *
 *                                                                                             *
 *                   Start Date : April 14, 1995                                               *
 *                                                                                             *
 *                  Last Update : November 30, 1995 [BRR]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Select_MPlayer_Game -- prompts user for NULL-Modem, Modem, or Network game                *
 *   Clear_Listbox -- clears the given list box                                                *
 *   Clear_Vector -- clears the given NodeNameType vector                                      *
 *   Computer_Message -- "sends" a message from the computer                                   *
 *   Garble_Message -- "garbles" a message                                                     *
 *   Surrender_Dialog -- Prompts user for surrendering                                         *
 *   Abort_Dialog -- Prompts user for confirmation on aborting the mission                     *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "mplayer.h"

#include "addon.h"
#include "msgbox.h"
#include "session.h"
#include "ui/screens/menu/uimenu.h"

class ListClass;

/// <summary>
/// Prompts the player for which kind of multiplayer game to start.
/// </summary>
/// <returns>Returns with the chosen game type, or GAME_NORMAL if the player backed out.</returns>
GameType Select_MPlayer_Game (void)
{
	GameType retval = GAME_NORMAL;
	AddonType addon;
	if (Addon_Installed(ADDON_ANY) == ADDON_FIRESTORM && !Select_Game_Type_Dialog(addon)) {
		return(retval);
	}

	bool firestorm = (Addon_Installed(ADDON_FIRESTORM) == ADDON_FIRESTORM);

	UIMenuState menu;
	menu.Kind = firestorm ? UI_MENU_MULTIPLAYER_FIRESTORM : UI_MENU_MULTIPLAYER;
	menu.Title = "Select Multiplayer Game";
	// Neither the online service these led to nor the tour it hosted can be reached, so the
	// entries are left on the menu but never answer.
	menu.Items.push_back(UIMenuItemType{"Internet", GAME_INTERNET, false});
	if (firestorm) {
		menu.Items.push_back(UIMenuItemType{"World Domination! (Internet)", GAME_WDT, false});
	}
	menu.Items.push_back(UIMenuItemType{"Network", GAME_IPX, true});
	menu.Items.push_back(UIMenuItemType{"Skirmish", GAME_SKIRMISH, true});
	menu.Items.push_back(UIMenuItemType{"Main Menu", GAME_NORMAL, true});
	UI_Menu_Place(menu);

	int chosen = UI_Menu_Dialog(menu, GAME_NORMAL);
	if (chosen == GAME_IPX || chosen == GAME_SKIRMISH) {
		retval = (GameType)chosen;
	}

	Session.Read_Scenario_Descriptions();
	return(retval);
}	/* end of Select_MPlayer_Game */


/***************************************************************************
 * Surrender_Dialog -- Prompts user for surrendering                       *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      0 = user cancels, 1 = user wants to surrender.                     *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   07/05/1995 BRR : Created.                                             *
 *=========================================================================*/
int Surrender_Dialog(int text)
{
	return(WWMessageBox()._Process(text, 1, TXT_OK, TXT_CANCEL) == 0);
}


/***************************************************************************
 * Clear_Vector -- clears the given NodeNameType vector                    *
 *                                                                         *
 * INPUT:                                                                  *
 *      vector      ptr to vector to clear                                 *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/29/1995 BRR : Created.                                             *
 *=========================================================================*/
void Clear_Vector(DynamicVectorClass <NodeNameType *> * vector)
{
	int i;

	//------------------------------------------------------------------------
	// Clear the 'Players' Vector
	//------------------------------------------------------------------------
	for (i = 0; i < vector->Count(); i++) {
		delete (*vector)[i];
	}
	vector->Clear();

}	// end of Clear_Vector


/************************** end of mplayer.cpp *****************************/
