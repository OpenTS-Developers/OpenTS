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
#include "data.h"
#include "dbgprint.h"
#include "gamedlg.h"
#include "language\language.h"
#include "loaddlg.h"
#include "ownrdraw.h"
#include "queue.h"
#include "restate.h"
#include "saveload.h"
#include "scenario.h"
#include "stats.h"

#include "special.hh"

void Game_Options_On_INITDIALOG(HWND window);
BOOL CALLBACK Game_Options_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
BOOL CALLBACK Abort_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
void Abort_Dialog_On_COMMAND(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

/// <summary>
/// Displays the in game options dialog.
/// This routine is used by the special dialog handler when the player calls up the options
/// screen. Which dialog appears depends on the kind of game in progress. Game input stays
/// locked out for as long as the dialog is up, and if the player asked for the mission
/// briefing it is restated on the way out.
/// </summary>
void Game_Options_Dialog(void)
{
	int rc = 0;

	HWND dialog;
	if (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH) {
		dialog = OwnerDraw::Begin_Dialog(IDD_OPT_CTRL_SP, (DLGPROC)Game_Options_Dialog_Proc);
	} else if (Session.Type == GAME_INTERNET) {
		dialog = OwnerDraw::Begin_Dialog(IDD_OPT_CTRL_WOL, (DLGPROC)Game_Options_Dialog_Proc);
	} else {
		dialog = OwnerDraw::Begin_Dialog(IDD_OPT_CTRL_MP, (DLGPROC)Game_Options_Dialog_Proc);
	}

	IgnoreInput = true;
	Keyboard->Clear();

	if (dialog) {

		SetWindowLong(dialog, DWL_USER, (LONG)&rc);

		OwnerDraw::Display_Dialog(dialog);

		while (rc == 0) {
			if (OwnerDraw::Dialog_Message_Handler() == true) {
				rc = IDOK;
			}
		}
		OwnerDraw::End_Dialog(dialog);
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


/// <summary>
/// Handles messages for the in game options dialog.
/// This routine offers every message to the owner draw system first. What is left it uses
/// to service the option buttons -- save, load, delete, briefing, resume, abort and
/// settings -- either acting on them directly or noting the player's choice for
/// Game_Options_Dialog to deal with once the dialog comes down. Dragging the game speed or
/// connection quality slider updates the label beside it.
/// </summary>
/// <returns>Returns with TRUE if the owner draw system consumed the message.</returns>
BOOL CALLBACK Game_Options_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	static int GameConnectionQualityNames[] = {
		TXT_WORST_CONNECTION,
		TXT_POOR_CONNECTION,
		TXT_GOOD_CONNECTION,
		TXT_BEST_CONNECTION
	};

	BOOL rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);

	HWND handle;

	if (rc) {
		return(rc);
	}

	switch (message) {

		case WM_INITDIALOG:
			Game_Options_On_INITDIALOG(window);
			break;

		case WM_COMMAND: {
			int code = HIWORD(wparam);
			int* retval = (int *)GetWindowLong(window, DWL_USER);

			switch (LOWORD(wparam)) {

				case IDC_SAVE_GAME:
					if (!code) {
						if (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH) {
							ShowWindow(window, SW_HIDE);
							UpdateWindow(MainWindow);
							char description[512];
							strcpy(description, Scen->Description);
							LoadOptionsClass().Save(description);
							Game_Options_On_INITDIALOG(window);
							ShowWindow(window, SW_SHOW);
							UpdateWindow(window);
						} else if (Is_Multiplayer_Saving_Allowed()) {
							OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::SAVEGAME));
							*retval = IDC_SAVE_GAME;
						}
					}
					break;

				case IDC_LOAD_GAME:
					if (!code) {
						ShowWindow(window, SW_HIDE);
						UpdateWindow(MainWindow);
						if (LoadOptionsClass().Load()) {
							*retval = IDC_LOAD_GAME;
						} else {
							ShowWindow(window, SW_SHOW);
							UpdateWindow(window);
						}
					}
					break;

				case IDC_BRIEFING:
					if (!code) {
						*retval = IDC_BRIEFING;
					}
					break;

				case IDC_DELETE_GAME:
					if (!code) {
						ShowWindow(window, SW_HIDE);
						UpdateWindow(MainWindow);
						LoadOptionsClass().Delete();
						Game_Options_On_INITDIALOG(window);
						ShowWindow(window, SW_SHOW);
						UpdateWindow(window);
					}
					break;

				case IDC_RESUME_MISSION:
					if (!code) {
						if (Session.Type == GAME_INTERNET) {
							handle = GetDlgItem(window, IDC_CTRLWOL_CONNECTION);
							if (handle) {
								int fudge = 3 - SendMessage(handle, TBM_GETPOS, 0, 0);
								if (fudge != Session.LatencyFudge) {
									OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::LATENCYFUDGE, fudge));
									DebugString("LATENCYFUDGE event created - %d\n", fudge);
								}
							}
							handle = GetDlgItem(window, IDC_GAME_SPEED_SLIDER);
							if (handle) {
								int speed = (OptionsClass::MAX_SPEED_SETTING-1) - SendMessage(handle, TBM_GETPOS, 0, 0);
								if (Options.GameSpeed != speed) {
									OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::GAMESPEED, speed));
								}
							}
						}
						*retval = IDOK;
					}
					break;

				case IDC_ABORT_MISSION:
					if (!code) {
						if (Session.Type == GAME_INTERNET) {
							SpecialDialog = SDLG_SURRENDER;
							if (!WestwoodOnline_Tournament) {
								SpecialDialog = SDLG_ABORT;
							}
						} else {
							SpecialDialog = SDLG_ABORT;
						}
						*retval = IDCANCEL;
					}
					break;

				case IDC_GAME_CONTROLS:
					if (!code) {
						SpecialDialog = SDLG_SETTINGS;
						*retval = IDOK;
					}
					break;

				default:
					break;
			}
			break;
		}

		case WM_HSCROLL: {
			if (LOWORD(wparam) == SB_THUMBTRACK) {
				int pos = HIWORD(wparam);
				int textid;

				if ((HWND)lparam == GetDlgItem(window, IDC_GAME_SPEED_SLIDER)) {
					textid = GameSpeedNames[pos];
					handle = GetDlgItem(window, IDC_GAME_SPEED_LABEL);
				} else if ((HWND)lparam == GetDlgItem(window, IDC_CTRLWOL_CONNECTION)) {
					textid = GameConnectionQualityNames[pos];
					handle = GetDlgItem(window, IDC_SCROLL_SPEED_LABEL);
				} else {
					break;
				}

				if (handle) {
					Static_SetText(handle, Fetch_String(textid));
				}
			}
			break;
		}

		default:
			break;
	}

	return(FALSE);
}


/// <summary>
/// Prepares the controls of the game options dialog.
/// This routine is called when the dialog is created, and again whenever a save or delete
/// has changed what is on disk. It decides which buttons the current game type allows the
/// player to use and primes the game speed and connection quality sliders.
/// </summary>
void Game_Options_On_INITDIALOG(HWND window)
{
	HWND handle;

	if (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH) {
		bool present = LoadOptionsClass().Files_Present();

		handle = GetDlgItem(window, IDC_LOAD_GAME);
		if (handle) {
			EnableWindow(handle, present);
		}

		handle = GetDlgItem(window, IDC_DELETE_GAME);
		if (handle) {
			EnableWindow(handle, present);
		}
	}

	if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
		handle = GetDlgItem(window, IDC_SAVE_GAME);
		if (handle) {
			EnableWindow(handle, Is_Multiplayer_Saving_Allowed());
		}
	}

	if (Session.Type == GAME_INTERNET) {

		handle = GetDlgItem(window, IDC_CTRLWOL_CONNECTION);
		if (handle) {
			SetSliderRangeAndPos(handle, 0, 3, 3 - Session.LatencyFudge);
		}

		handle = GetDlgItem(window, IDC_GAME_SPEED_SLIDER);
		if (handle) {
			Slider_SetRange(handle, 0, OptionsClass::MAX_SPEED_SETTING-1);
			Slider_SetPos(handle, (OptionsClass::MAX_SPEED_SETTING-1) - Options.GameSpeed);
		}
	}

	if (Session.Type == GAME_SKIRMISH) {
		handle = GetDlgItem(window, IDC_BRIEFING);
		if (handle) {
			EnableWindow(handle, FALSE);
		}
	}

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
	int rc = 0;

	HWND dialog = OwnerDraw::Begin_Dialog(IDD_MISSION_ABORT, (DLGPROC)Abort_Dialog_Proc);

	if (dialog) {

		SetWindowLong(dialog, DWL_USER, (LONG)&rc);

		OwnerDraw::Display_Dialog(dialog);

		while (rc == 0) {
			if (OwnerDraw::Dialog_Message_Handler() == true) {
				rc = IDOK;
			}
		}
		OwnerDraw::End_Dialog(dialog);
	}
	return(rc);
}


/// <summary>
/// Handles messages for the abort mission dialog.
/// This routine offers every message to the owner draw system first. What is left it uses
/// to relabel the restart button as a surrender for a multiplayer game, and to pass button
/// presses along to Abort_Dialog_On_COMMAND.
/// </summary>
/// <returns>Returns with the result of the owner draw default dialog handler.</returns>
BOOL CALLBACK Abort_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	HWND handle;

	int rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);
	if (rc == 0) {
		switch (message) {
			case WM_INITDIALOG:
				handle = GetDlgItem(window, IDC_RESTART_MISSION);
				if (Session.Type != GAME_NORMAL) {
					SetWindowText(handle, Fetch_String(TXT_SURRENDER));
					if (PlayerPtr->IsDefeated || PlayerPtr->IsToWin || PlayerPtr->IsToLose || PlayerPtr->IsToDie) {
						EnableWindow(handle, FALSE);
					}
				}
				break;

			case WM_COMMAND:
				Abort_Dialog_On_COMMAND(window, LOWORD(wparam), 0, HIWORD(wparam));
				break;
		}
		rc = 0;
	}
	return(rc);
}


/// <summary>
/// Handles a button press in the abort mission dialog.
/// This routine records the player's choice in the result variable that Abort_Dialog
/// attached to the dialog window, which is what ends the dialog's message pump.
/// </summary>
/// <param name="message">The control identifier of the button that was pressed.</param>
/// <param name="lparam">The notification code that came with the button press.</param>
void Abort_Dialog_On_COMMAND(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	int* retval = (int *)GetWindowLong(window, DWL_USER);

	switch ((int)message) {
		case IDC_ABORT_MISSION:
			if (lparam == 0) {
				*retval = IDOK;
			}
			break;

		case IDC_RESTART_MISSION:
			if (lparam == 0) {
				*retval = IDABORT;
			}
			break;

		case IDOK:
		case IDCANCEL:
			if (lparam == 0) {
				*retval = IDCANCEL;
			}
			break;
	}
}
