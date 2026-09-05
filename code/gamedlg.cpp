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

#include "_map.h"
#include "_tooltip.h"
#include "cctooltip.h"
#include "data.h"
#include "dbgprint.h"
#include "audio/audioengine.h"
#include "globals.h"
#include "init.h"
#include "language/language.h"
#include "ownrdraw.h"
#include "queue.h"
#include "session.h"
#include "techno.h"

#include "special.hh"

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


BOOL CALLBACK Game_Controls_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
void Game_Controls_Dialog_On_COMMAND(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

/***********************************************************************************************
 * OptionsClass::Process -- Handles all the options graphic interface.                         *
 *                                                                                             *
 *    This routine is the main control for the visual representation of the options            *
 *    screen. It handles the visual overlay and the player input.                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 * OUTPUT:  none                                                                               *
 * WARNINGS:   none                                                                            *
 * HISTORY:                                                                                    *
 *   12/31/1994 MML : Created.                                                                 *
 *=============================================================================================*/
void GameControlsClass::Dialog(void)
{
	int res = -1;

	DebugString("GameControls: GameSpeed = %d, ScrollRate = %d, Detail = %d\n", Options.GameSpeed, Options.ScrollRate, Options.DetailLevel);

	if (GameActive == true) {
		if (Session.Type == GAME_INTERNET) {
			_Dialog = OwnerDraw::Begin_Dialog(IDD_OPT_CTRL_GAME_WOL, (DLGPROC)Game_Controls_Dialog_Proc);
		} else {
			_Dialog = OwnerDraw::Begin_Dialog(IDD_OPT_CTRL_GAME_MP, (DLGPROC)Game_Controls_Dialog_Proc);
		}
	} else {
		_Dialog = OwnerDraw::Begin_Dialog(IDD_OPT_CTRL_GAME_SP, (DLGPROC)Game_Controls_Dialog_Proc);
	}

	if (_Dialog) {

		SetWindowLong(_Dialog, DWL_USER, (LONG)&res);

		OwnerDraw::Display_Dialog(_Dialog);

		while (res == -1) {
			if (OwnerDraw::Dialog_Message_Handler() == true) {
				res = 2;
			}
			if (!GameActive) {
				Title_Screen_Restore();
			}
		}
		if (res == 1) {
			Set();
			Options.Save_Settings();
		}

		OwnerDraw::End_Dialog(_Dialog);
	}

	DebugString("GameControls: GameSpeed = %d, ScrollRate = %d, Detail = %d\n", Options.GameSpeed, Options.ScrollRate, Options.DetailLevel);
}


/// <summary>
/// Sets the game options from the game controls dialog.
/// This routine is called when the player accepts the dialog. Each control is asked for
/// its current value and the answer is handed to the option it governs, along with any
/// notification the rest of the game needs -- the map is told to rebuild its cell drawers
/// when the detail level changes, and a game speed change during a network game is issued
/// as an event so that every player stays in step.
/// </summary>
void GameControlsClass::Set(void)
{
	HWND handle;

	handle = GetDlgItem(_Dialog, IDC_GAME_SPEED_SLIDER);
	if (handle) {
		int gamespeed = (OptionsClass::MAX_SPEED_SETTING-1) - Slider_GetPos(handle);
		if (Options.GameSpeed != gamespeed) {
			if (GameActive == true && Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
				OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::GAMESPEED, gamespeed));
			} else {
				Options.GameSpeed = gamespeed;
			}
		}
	}

	handle = GetDlgItem(_Dialog, IDC_SCROLL_SPEED_SLIDER);
	if (handle) {
		Options.ScrollRate = (OptionsClass::MAX_SCROLL_SETTING-1) - Slider_GetPos(handle);
	}

	handle = GetDlgItem(_Dialog, IDC_DETAIL_LEVEL_SLIDER);
	if (handle) {
		int detailevel = Slider_GetPos(handle);
		if (Options.DetailLevel != detailevel) {
			Options.DetailLevel = detailevel;
			Map.Reinit_Cell_Drawers();
		}
	}

	handle = GetDlgItem(_Dialog, IDC_SIDEBAR_TEXT);
	if (handle) {
		bool cameotext = Button_GetCheck(handle) == TRUE;
		if (Options.SidebarCameoText != cameotext) {
			Options.SidebarCameoText = cameotext;
			Map.Toggle_Cameo_Text(cameotext);
		}
	}

	handle = GetDlgItem(_Dialog, IDC_TARGET_LINES);
	if (handle) {
		Options.ActionLines = Button_GetCheck(handle) == TRUE;
		TechnoClass::Set_Action_Lines(Options.ActionLines);
	}

	handle = GetDlgItem(_Dialog, IDC_TOOLTIPS);
	if (handle) {
		Options.ToolTips = Button_GetCheck(handle) == TRUE;
		if (ToolTips != NULL && GameActive == true) {
			ToolTips->Activate(Options.ToolTips);
		}
	}

	handle = GetDlgItem(_Dialog, IDC_SCROLL_COASTING);
	if (handle) {
		Options.ScrollMethod = Button_GetCheck(handle) == TRUE ? 0 : 1;
	}

	if (GameActive == false) {
		handle = GetDlgItem(_Dialog, IDC_DIFFICULTY_SLIDER);
		if (handle) {
			Options.Difficulty = Slider_GetPos(handle);
		}
	}
}


/// <summary>
/// Handles the messages sent to the game controls dialog.
/// This routine gives the ownerdraw layer first refusal on every message. Anything it
/// leaves alone is used to prime the sliders and check boxes from the current options, to
/// track the label alongside a slider the player is dragging, and to route commands on to
/// Game_Controls_Dialog_On_COMMAND.
/// </summary>
/// <returns>Returns with a non-zero value if the message was consumed by the ownerdraw
/// layer.</returns>
BOOL CALLBACK Game_Controls_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	HWND handle;
	int index;

	int rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);
	if (rc == 0) {
		switch (message) {
			case WM_INITDIALOG:
				handle = GetDlgItem(window, IDC_GAME_SPEED_SLIDER);
				if (handle) {
					SendMessage(handle, OD_TRACKNUMBERS, 0, 0);
					Slider_SetRange(handle, 0, (OptionsClass::MAX_SPEED_SETTING-1));
					Slider_SetPos(handle, (OptionsClass::MAX_SPEED_SETTING-1) - Options.GameSpeed);
				}

				handle = GetDlgItem(window, IDC_SCROLL_SPEED_SLIDER);
				if (handle) {
					SendMessage(handle, OD_TRACKNUMBERS, 0, 0);
					Slider_SetRange(handle, 0, (OptionsClass::MAX_SCROLL_SETTING-1));
					Slider_SetPos(handle, (OptionsClass::MAX_SCROLL_SETTING-1) - Options.ScrollRate);
				}

				handle = GetDlgItem(window, IDC_DETAIL_LEVEL_SLIDER);
				if (handle) {
					SendMessage(handle, OD_TRACKNUMBERS, 0, 0);
					Slider_SetRange(handle, 0, (OptionsClass::MAX_DETAIL_SETTING-1));
					Slider_SetPos(handle, Options.DetailLevel);
				}

				handle = GetDlgItem(window, IDC_SIDEBAR_TEXT);
				if (handle) {
					Button_SetCheck(handle, Options.SidebarCameoText != false);
				}

				handle = GetDlgItem(window, IDC_TARGET_LINES);
				if (handle) {
					Button_SetCheck(handle, Options.ActionLines != false);
				}

				handle = GetDlgItem(window, IDC_TOOLTIPS);
				if (handle) {
					Button_SetCheck(handle, Options.ToolTips != false);
				}

				handle = GetDlgItem(window, IDC_SCROLL_COASTING);
				if (handle) {
					Button_SetCheck(handle, Options.ScrollMethod == 0);
				}

				if (GameActive == true) {
					handle = GetDlgItem(window, IDC_OPT_SOUND_BTN);
					if (handle) {
						EnableWindow(handle, AudioEngine.Is_Available());
					}
				} else {
					handle = GetDlgItem(window, IDC_DIFFICULTY_SLIDER);
					if (handle) {
						SendMessage(handle, OD_TRACKNUMBERS, 0, 0);
						Slider_SetRange(handle, 0, (OptionsClass::MAX_DIFFICULTY_SETTING-1));
						Slider_SetPos(handle, Options.Difficulty);
					}
				}
				break;

			case WM_COMMAND:
				Game_Controls_Dialog_On_COMMAND(window, LOWORD(wparam), 0, HIWORD(wparam));
				break;

			case WM_HSCROLL:
				if (LOWORD(wparam) == SB_THUMBTRACK) {
					index = HIWORD(wparam);
					int name;

					handle = 0;
					if ((HWND)lparam == GetDlgItem(window, IDC_GAME_SPEED_SLIDER)) {
						name = GameSpeedNames[index];
						handle = GetDlgItem(window, IDC_GAME_SPEED_LABEL);
					} else if ((HWND)lparam == GetDlgItem(window, IDC_SCROLL_SPEED_SLIDER)) {
						name = GameScrollSpeedNames[index];
						handle = GetDlgItem(window, IDC_SCROLL_SPEED_LABEL);
					} else if ((HWND)lparam == GetDlgItem(window, IDC_DETAIL_LEVEL_SLIDER)) {
						name = GameDetailLevelNames[index];
						handle = GetDlgItem(window, IDC_DETAIL_LEVEL_LABEL);
					} else if (GameActive == false && (HWND)lparam == GetDlgItem(window, IDC_DIFFICULTY_SLIDER)) {
						name = GameDifficultyNames[index];
						handle = GetDlgItem(window, IDC_DIFFICULTY_LABEL);
					}
					if (handle) {
						SetWindowText(handle, Fetch_String(name));
					}
				}
				break;
		}
		rc = 0;
	}
	return(rc);
}


/// <summary>
/// Handles the button presses of the game controls dialog.
/// This routine is called by the dialog procedure whenever a control notifies it. The
/// answer is stored back through the result pointer the dialog was created with, which is
/// what releases GameControlsClass::Dialog from its message loop.
/// </summary>
/// <param name="window">The game controls dialog window.</param>
/// <param name="message">The identifier of the control that was activated.</param>
/// <param name="lparam">The notification code the control sent.</param>
void Game_Controls_Dialog_On_COMMAND(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	int* retval = (int *)GetWindowLong(window, DWL_USER);

	switch ((INT)message) {
		case IDC_OPT_KEYBOARD_BTN:
			if (lparam == 0 && GameActive == true) {
				SpecialDialog = SDLG_KEYBOARD;
				*retval = IDOK;
			}
			break;

		case IDC_OPT_SOUND_BTN:
			if (lparam == 0 && GameActive == true) {
				SpecialDialog = SDLG_SOUND;
				*retval = IDOK;
			}
			break;

		case IDOK:
			if (lparam == 0) {
				*retval = IDOK;
			}
			break;

		case IDCANCEL:
			*retval = IDCANCEL;
			break;
	}
}
