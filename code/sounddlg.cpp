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

/* $Header: /CounterStrike/SOUNDDLG.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SOUNDDLG.CPP                                                 *
 *                                                                                             *
 *                   Programmer : Maria del Mar McCready-Legg, Joe L. Bostic                   *
 *                                                                                             *
 *                   Start Date : Jan 8, 1995                                                  *
 *                                                                                             *
 *                  Last Update : September 22, 1995 [JLB]                                     *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   MusicListClass::Draw_Entry -- Draw the score line in a list box.                          *
 *   SoundControlsClass::Process -- Handles all the options graphic interface.                 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "sounddlg.h"

#include "dbgprint.h"
#include "globals.h"
#include "goptions.h"
#include "incdec.h"
#include "init.h"
#include "language/language.h"
#include "ownrdraw.h"
#include "ui/screens/sound/uisound.h"
#include "ui/uiscreen.h"
#include "ui/uishell.h"
#include "winfix.h"

bool DialogInitialized = false;

// The presenter the dialog procedure is a view of, for the life of one Dialog call.
static UISoundPresenterClass * _Presenter = NULL;


static void Queue_And_Drain(UISoundPresenterClass & presenter, char const * name, int value = 0)
{
	UIIntent intent;
	intent.Name = name;
	intent.Value = value;
	presenter.Queue(intent);
	presenter.Drain();
}


static void Sync_Switches(HWND window, UISoundState const & state)
{
	SendDlgItemMessage(window, IDC_SOUND_SHUFFLE, BM_SETCHECK, state.Shuffle ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(window, IDC_SOUND_REPEAT, BM_SETCHECK, state.Repeat ? BST_CHECKED : BST_UNCHECKED, 0);
}


/// <summary>
/// Handles the sound and music options dialog.
/// This routine brings up the sound controls and then services the owner draw dialog
/// handler until the player dismisses them. A cut down version of the dialog is used
/// when there is no game in progress, since the in game options do not apply there.
/// </summary>
/// <remarks>This routine will not return until the player closes the dialog.</remarks>
static void Run_Win32_Dialog(void)
{
	int rc = -1;

	DialogInitialized = false;

	UISoundState state;
	UI_Sound_State(state);
	UISoundPresenterClass presenter(UI_Sound_Service(), state);
	_Presenter = &presenter;

	HWND dialog;
	if (!GameActive) {
		dialog = OwnerDraw::Begin_Dialog(IDD_SOUND_OPTIONS_DIALOG_LITE, SoundControlsClass::Sound_Option_Dialog_Func);
	} else {
		dialog = OwnerDraw::Begin_Dialog(IDD_SOUND_OPTIONS_DIALOG, SoundControlsClass::Sound_Option_Dialog_Func);
	}

	if (dialog) {
		SetWindowLongPtr(dialog, DWLP_USER, (LONG_PTR)&rc);
		OwnerDraw::Display_Dialog(dialog);

		while (rc == -1) {
			if (OwnerDraw::Dialog_Message_Handler() == true) {
				rc = 2;
			}

			if (!GameActive) {
				Title_Screen_Restore();
			}
		}

		OwnerDraw::End_Dialog(dialog);
	}

	_Presenter = NULL;
}


void SoundControlsClass::Dialog(void)
{
	DebugString("SoundControls: GameSpeed = %d, ScrollRate = %d, Detail = %d\n", Options.GameSpeed, Options.ScrollRate, Options.DetailLevel);

	if (!UI_Use_Rml() || !UI_Sound_Dialog()) {
		Run_Win32_Dialog();
	}

	DebugString("SoundControls: GameSpeed = %d, ScrollRate = %d, Detail = %d\n", Options.GameSpeed, Options.ScrollRate, Options.DetailLevel);
}


/***********************************************************************************************
 * SoundControlsClass::Process -- Handles all the options graphic interface.                   *
 *                                                                                             *
 *    This routine is the main control for the visual representation of the options            *
 *    screen. It handles the visual overlay and the player input.                              *
 *                                                                                             *
 * INPUT:      none                                                                            *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:    12/31/1994 MML : Created.                                                       *
 *=============================================================================================*/
INT_PTR CALLBACK SoundControlsClass::Sound_Option_Dialog_Func(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	INT_PTR rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);

	if (rc == 0) {
		UISoundPresenterClass * presenter = _Presenter;
		if (presenter == NULL) {
			return(FALSE);
		}
		UISoundState const & state = presenter->State;

		switch (message) {
			case WM_INITDIALOG: {
					DialogInitialized = false;

					// Seeding the sliders must neither write the options nor make a sound.
					HWND track = GetDlgItem(window, IDC_MUSIC_VOLUME);
					if (track) {
						SendMessage(track, OD_TRACKSILENT, 0, 0);
						Slider_SetRange(track, 0, VOLUME_LEVELS);
						Slider_SetPos(track, state.Score);
						EnableWindow(track, state.Enabled);
					}

					track = GetDlgItem(window, IDC_SOUND_VOLUME);
					if (track) {
						SendMessage(track, OD_TRACKSILENT, 0, 0);
						Slider_SetRange(track, 0, VOLUME_LEVELS);
						Slider_SetPos(track, state.Sound);
						EnableWindow(track, state.Enabled);
					}

					track = GetDlgItem(window, IDC_VOICE_VOLUME);
					if (track) {
						SendMessage(track, OD_TRACKSILENT, 0, 0);
						Slider_SetRange(track, 0, VOLUME_LEVELS);
						Slider_SetPos(track, state.Voice);
						EnableWindow(track, state.Enabled);
					}

					if (GameActive) {
						HWND button = GetDlgItem(window, IDC_SOUND_SHUFFLE);
						if (button) {
							Button_SetCheck(button, state.Shuffle ? BST_CHECKED : BST_UNCHECKED);
							EnableWindow(button, state.Enabled);
						}

						button = GetDlgItem(window, IDC_SOUND_REPEAT);
						if (button) {
							Button_SetCheck(button, state.Repeat ? BST_CHECKED : BST_UNCHECKED);
							EnableWindow(button, state.Enabled);
						}

						HWND list = GetDlgItem(window, IDC_SOUND_TRACKLIST);
						if (list) {
							ListBox_ResetContent(list);
							for (UISoundTrack const & entry : state.Tracks) {
								int row = ListBox_AddString(list, entry.Label.c_str());
								if (row != LB_ERR) {
									ListBox_SetItemData(list, row, entry.Theme);
								}
							}
							int selected = (state.Selected >= 0) ? state.Selected : 0;
							ListBox_SetCurSel(list, selected);
							ListBox_SetTopIndex(list, selected);
							EnableWindow(list, state.Enabled);
						}
					}

					DialogInitialized = true;
				}
				break;

			case WM_COMMAND:
				switch (LOWORD(wparam)) {
					case IDC_SOUND_SHUFFLE:
						Queue_And_Drain(*presenter, "shuffle", Button_GetCheck((HWND)lparam) == BST_CHECKED);
						Sync_Switches(window, state);
						break;

					case IDC_SOUND_REPEAT:
						Queue_And_Drain(*presenter, "repeat", Button_GetCheck((HWND)lparam) == BST_CHECKED);
						Sync_Switches(window, state);
						break;

					case IDC_SOUND_STOP:
						if (HIWORD(wparam) == 0) {
							Queue_And_Drain(*presenter, "stop");
						}
						break;

					case IDOK:
						if (HIWORD(wparam) == 0) {
							Queue_And_Drain(*presenter, "ok");
							if (presenter->Result.has_value()) {
								int * res = (int *)GetWindowLongPtr(window, DWLP_USER);
								*res = IDOK;
							}
						}
						break;

					case IDC_SOUND_PLAY:
						if (HIWORD(wparam) == 0) {
							HWND list = GetDlgItem(window, IDC_SOUND_TRACKLIST);
							if (list) {
								int row = ListBox_GetCurSel(list);
								if (row != LB_ERR) {
									Queue_And_Drain(*presenter, "select", row);
									Queue_And_Drain(*presenter, "play");
								}
							}
						}
						break;
				}
				break;

			case WM_HSCROLL:
				if (DialogInitialized) {
					HWND track = (HWND)lparam;

					if (track == GetDlgItem(window, IDC_MUSIC_VOLUME)) {
						Queue_And_Drain(*presenter, "score", Slider_GetPos(track));
					} else if (track == GetDlgItem(window, IDC_SOUND_VOLUME)) {
						Queue_And_Drain(*presenter, "sound", Slider_GetPos(track));
					} else if (track == GetDlgItem(window, IDC_VOICE_VOLUME)) {
						Queue_And_Drain(*presenter, "voice", Slider_GetPos(track));
					}
				}
				break;
		}

		return(FALSE);
	}

	return(rc);
}
