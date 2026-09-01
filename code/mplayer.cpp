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

#include "_surface.h"
#include "addon.h"
#include "init.h"
#include "msgbox.h"
#include "ownrdraw.h"
#include "session.h"

class ListClass;

BOOL CALLBACK Select_MPlayer_Game_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

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

	HWND dialog;

	if (Addon_Installed(ADDON_FIRESTORM) == ADDON_FIRESTORM) {
		dialog = OwnerDraw::Begin_Dialog(IDD_MPLAYER_SELECT_GAME_FS, Select_MPlayer_Game_Dialog_Proc);
	} else {
		dialog = OwnerDraw::Begin_Dialog(IDD_MPLAYER_SELECT_GAME, Select_MPlayer_Game_Dialog_Proc);
	}


	if (dialog) {

		int rc;
		SetWindowLong(dialog, DWL_USER, (LONG)&rc);

		bool process = true;
		while (process) {
			OwnerDraw::Move_Dialog(dialog, -1, (HiddenSurface->Get_Height() - 400) / 2 + 147);
			OwnerDraw::Display_Dialog(dialog);
			rc = -1;
			while (rc == -1) {
				if (OwnerDraw::Dialog_Message_Handler() == true) {
					break;
				}
				Title_Screen_Restore();
			}

			ShowWindow(dialog, SW_HIDE);
			UpdateWindow(MainWindow);
			switch (rc) {
				case IDC_NETWORK:
					retval = GAME_IPX;
					break;
				case IDC_SKIRMISH:
					retval = GAME_SKIRMISH;
					break;
				default:
					retval = GAME_NORMAL;
					process = false;
					break;
			}
			if (retval != GAME_NORMAL) {
				break;
			}
		}

		OwnerDraw::End_Dialog(dialog);
		Session.Read_Scenario_Descriptions();
	}
	return(retval);
}	/* end of Select_MPlayer_Game */


/// <summary>
/// Handles the messages for the multiplayer game type dialog.
/// </summary>
/// <returns>Returns with the result of the ownerdraw handler, or false when the message was
/// left unhandled.</returns>
BOOL CALLBACK Select_MPlayer_Game_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	int * retval;
	HWND handle;

	int rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);

	if (message == WM_INITDIALOG) {
		// Neither the online service these led to nor the tour it hosted can be reached,
		// so the buttons are left on the dialog but never answer.
		handle = GetDlgItem(window, IDC_INTERNET);
		if (handle) {
			EnableWindow(handle, FALSE);
		}
		handle = GetDlgItem(window, IDC_WORLDDOM);
		if (handle) {
			EnableWindow(handle, FALSE);
		}
	}

	if (rc != 0) {
		return(rc);
	}

	if (message == WM_COMMAND) {
		retval = (int *)GetWindowLong(window, DWL_USER);
		*retval = LOWORD(wparam);
	}
	return(false);
}


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
