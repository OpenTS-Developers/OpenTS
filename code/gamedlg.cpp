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
#include "_ui.h"
#include "audio/audioengine.h"
#include "cctooltip.h"
#include "data.h"
#include "dbgprint.h"
#include "globals.h"
#include "init.h"
#include "language/language.h"
#include "ownrdraw.h"
#include "queue.h"
#include "session.h"
#include "techno.h"
#include "ui/screens/gamectrl/uigamectrl.h"
#include "ui/uiscreen.h"
#include "ui/uishell.h"

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


INT_PTR CALLBACK Game_Controls_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
void Game_Controls_Dialog_On_COMMAND(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

// The presenter the dialog procedure is a view of, for the life of one Dialog call.
static UIGameControlsPresenterClass * _Presenter = NULL;


static void Queue_And_Drain(UIGameControlsPresenterClass & presenter, char const * name, int value = 0)
{
	UIIntent intent;
	intent.Name = name;
	intent.Value = value;
	presenter.Queue(intent);
	presenter.Drain();
}


// Hands the controls' current values to the presenter as the accept path always read them:
// a speed or scroll slider shows its value reversed.
static void Read_Controls(HWND window, UIGameControlsPresenterClass & presenter)
{
	HWND handle = GetDlgItem(window, IDC_GAME_SPEED_SLIDER);
	if (handle) {
		Queue_And_Drain(presenter, "speed", (OptionsClass::MAX_SPEED_SETTING-1) - Slider_GetPos(handle));
	}

	handle = GetDlgItem(window, IDC_SCROLL_SPEED_SLIDER);
	if (handle) {
		Queue_And_Drain(presenter, "scroll", (OptionsClass::MAX_SCROLL_SETTING-1) - Slider_GetPos(handle));
	}

	handle = GetDlgItem(window, IDC_DETAIL_LEVEL_SLIDER);
	if (handle) {
		Queue_And_Drain(presenter, "detail", Slider_GetPos(handle));
	}

	handle = GetDlgItem(window, IDC_SIDEBAR_TEXT);
	if (handle) {
		Queue_And_Drain(presenter, "cameo", Button_GetCheck(handle) == TRUE);
	}

	handle = GetDlgItem(window, IDC_TARGET_LINES);
	if (handle) {
		Queue_And_Drain(presenter, "lines", Button_GetCheck(handle) == TRUE);
	}

	handle = GetDlgItem(window, IDC_TOOLTIPS);
	if (handle) {
		Queue_And_Drain(presenter, "tooltips", Button_GetCheck(handle) == TRUE);
	}

	handle = GetDlgItem(window, IDC_SCROLL_COASTING);
	if (handle) {
		Queue_And_Drain(presenter, "coasting", Button_GetCheck(handle) == TRUE);
	}

	handle = GetDlgItem(window, IDC_EDGE_SCROLL);
	if (handle) {
		Queue_And_Drain(presenter, "edge", Button_GetCheck(handle) == TRUE);
	}

	if (GameActive == false) {
		handle = GetDlgItem(window, IDC_DIFFICULTY_SLIDER);
		if (handle) {
			Queue_And_Drain(presenter, "difficulty", Slider_GetPos(handle));
		}
	}
}

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
void GameControlsClass::Run_Win32_Dialog(void)
{
	int res = -1;

	UIGameControlsState state;
	UI_Game_Controls_State(state);
	UIGameControlsPresenterClass presenter(UI_Game_Controls_Service(), state);
	_Presenter = &presenter;

	if (GameActive == true) {
		if (Session.Type == GAME_INTERNET) {
			_Dialog = OwnerDraw::Begin_Dialog(IDD_OPT_CTRL_GAME_WOL, Game_Controls_Dialog_Proc);
		} else {
			_Dialog = OwnerDraw::Begin_Dialog(IDD_OPT_CTRL_GAME_MP, Game_Controls_Dialog_Proc);
		}
	} else {
		_Dialog = OwnerDraw::Begin_Dialog(IDD_OPT_CTRL_GAME_SP, Game_Controls_Dialog_Proc);
	}

	if (_Dialog) {

		SetWindowLongPtr(_Dialog, DWLP_USER, (LONG_PTR)&res);

		OwnerDraw::Display_Dialog(_Dialog);

		while (res == -1) {
			if (OwnerDraw::Dialog_Message_Handler() == true) {
				res = 2;
			}
			if (!GameActive) {
				Title_Screen_Restore();
			}
		}

		OwnerDraw::End_Dialog(_Dialog);
	}

	_Presenter = NULL;

	// The Sound and Keyboard buttons accept the settings and name the screen that follows.
	if (presenter.Next == UIGameControlsPresenterClass::NEXT_SOUND) {
		SpecialDialog = SDLG_SOUND;
	} else if (presenter.Next == UIGameControlsPresenterClass::NEXT_KEYBOARD) {
		SpecialDialog = SDLG_KEYBOARD;
	}
}


void GameControlsClass::Dialog(void)
{
	DebugString("GameControls: GameSpeed = %d, ScrollRate = %d, Detail = %d\n", Options.GameSpeed, Options.ScrollRate, Options.DetailLevel);

	if (!UIShell.Use_Rml() || !UI_Game_Controls_Dialog()) {
		Run_Win32_Dialog();
	}

	DebugString("GameControls: GameSpeed = %d, ScrollRate = %d, Detail = %d\n", Options.GameSpeed, Options.ScrollRate, Options.DetailLevel);
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
INT_PTR CALLBACK Game_Controls_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	HWND handle;
	int index;

	INT_PTR rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);
	if (rc == 0) {
		UIGameControlsPresenterClass * presenter = _Presenter;
		if (presenter == NULL) {
			return(FALSE);
		}
		UIGameControlsState const & state = presenter->State;

		switch (message) {
			case WM_INITDIALOG:
				handle = GetDlgItem(window, IDC_GAME_SPEED_SLIDER);
				if (handle) {
					SendMessage(handle, OD_TRACKNUMBERS, 0, 0);
					Slider_SetRange(handle, 0, (OptionsClass::MAX_SPEED_SETTING-1));
					Slider_SetPos(handle, (OptionsClass::MAX_SPEED_SETTING-1) - state.Speed);
				}

				handle = GetDlgItem(window, IDC_SCROLL_SPEED_SLIDER);
				if (handle) {
					SendMessage(handle, OD_TRACKNUMBERS, 0, 0);
					Slider_SetRange(handle, 0, (OptionsClass::MAX_SCROLL_SETTING-1));
					Slider_SetPos(handle, (OptionsClass::MAX_SCROLL_SETTING-1) - state.Scroll);
				}

				handle = GetDlgItem(window, IDC_DETAIL_LEVEL_SLIDER);
				if (handle) {
					SendMessage(handle, OD_TRACKNUMBERS, 0, 0);
					Slider_SetRange(handle, 0, (OptionsClass::MAX_DETAIL_SETTING-1));
					Slider_SetPos(handle, state.Detail);
				}

				handle = GetDlgItem(window, IDC_SIDEBAR_TEXT);
				if (handle) {
					Button_SetCheck(handle, state.CameoText);
				}

				handle = GetDlgItem(window, IDC_TARGET_LINES);
				if (handle) {
					Button_SetCheck(handle, state.ActionLines);
				}

				handle = GetDlgItem(window, IDC_TOOLTIPS);
				if (handle) {
					Button_SetCheck(handle, state.ToolTips);
				}

				handle = GetDlgItem(window, IDC_SCROLL_COASTING);
				if (handle) {
					Button_SetCheck(handle, state.Coasting);
				}

				handle = GetDlgItem(window, IDC_EDGE_SCROLL);
				if (handle) {
					Button_SetCheck(handle, state.EdgeScroll);
				}

				if (GameActive == true) {
					handle = GetDlgItem(window, IDC_OPT_SOUND_BTN);
					if (handle) {
						EnableWindow(handle, state.SoundEnabled);
					}
				} else {
					handle = GetDlgItem(window, IDC_DIFFICULTY_SLIDER);
					if (handle) {
						SendMessage(handle, OD_TRACKNUMBERS, 0, 0);
						Slider_SetRange(handle, 0, (OptionsClass::MAX_DIFFICULTY_SETTING-1));
						Slider_SetPos(handle, state.Difficulty);
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
	int* retval = (int *)GetWindowLongPtr(window, DWLP_USER);
	UIGameControlsPresenterClass * presenter = _Presenter;
	if (presenter == NULL) {
		return;
	}

	switch ((INT)message) {
		case IDC_OPT_KEYBOARD_BTN:
			if (lparam == 0 && GameActive == true) {
				Read_Controls(window, *presenter);
				Queue_And_Drain(*presenter, "keyboard");
				*retval = IDOK;
			}
			break;

		case IDC_OPT_SOUND_BTN:
			if (lparam == 0 && GameActive == true) {
				Read_Controls(window, *presenter);
				Queue_And_Drain(*presenter, "sound");
				*retval = IDOK;
			}
			break;

		case IDOK:
			if (lparam == 0) {
				Read_Controls(window, *presenter);
				Queue_And_Drain(*presenter, "ok");
				*retval = IDOK;
			}
			break;

		case IDCANCEL:
			Queue_And_Drain(*presenter, "cancel");
			*retval = IDCANCEL;
			break;
	}
}
