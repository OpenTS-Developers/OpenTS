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
#include "dsaudio.h"
#include "globals.h"
#include "goptions.h"
#include "incdec.h"
#include "init.h"
#include "language/language.h"
#include "ownrdraw.h"
#include "theme.h"
#include "winfix.h"

bool DialogInitialized = false;


/// <summary>
/// Handles the sound and music options dialog.
/// This routine brings up the sound controls and then services the owner draw dialog
/// handler until the player dismisses them. A cut down version of the dialog is used
/// when there is no game in progress, since the in game options do not apply there.
/// </summary>
/// <remarks>This routine will not return until the player closes the dialog.</remarks>
void SoundControlsClass::Dialog(void)
{
	int rc = -1;
	DebugString("SoundControls: GameSpeed = %d, ScrollRate = %d, Detail = %d\n", Options.GameSpeed, Options.ScrollRate, Options.DetailLevel);
	DialogInitialized = false;

	HWND dialog;
	if (!GameActive) {
		dialog = OwnerDraw::Begin_Dialog(IDD_SOUND_OPTIONS_DIALOG_LITE, (DLGPROC)Sound_Option_Dialog_Func);
	} else {
		dialog = OwnerDraw::Begin_Dialog(IDD_SOUND_OPTIONS_DIALOG, (DLGPROC)Sound_Option_Dialog_Func);
	}

	if (dialog) {

		SetWindowLong(dialog, DWL_USER, (LONG)&rc);

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
BOOL CALLBACK SoundControlsClass::Sound_Option_Dialog_Func(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	int rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);

	if (rc == 0) {
		switch (message) {
			case WM_INITDIALOG: {
					DialogInitialized = false;
					bool enabled = Audio_Available();

					/*
					**	Music volume slider.
					*/
					HWND track = GetDlgItem(window, IDC_MUSIC_VOLUME);
					if (track) {
						SendMessage(track, OD_TRACKSILENT, 0, 0);
						Slider_SetRange(track, 0, VOLUME_LEVELS);
						Slider_SetPos(track, (int)(Options.ScoreVolume * (double)VOLUME_LEVELS + 0.5));
						EnableWindow(track, enabled);
					}

					/*
					**	Sound volume slider.
					*/
					track = GetDlgItem(window, IDC_SOUND_VOLUME);
					if (track) {
						SendMessage(track, OD_TRACKSILENT, 0, 0);
						Slider_SetRange(track, 0, VOLUME_LEVELS);
						Slider_SetPos(track, (int)(Options.SoundVolume * (double)VOLUME_LEVELS + 0.5));
						EnableWindow(track, enabled);
					}

					track = GetDlgItem(window, IDC_VOICE_VOLUME);
					if (track) {
						SendMessage(track, OD_TRACKSILENT, 0, 0);
						Slider_SetRange(track, 0, VOLUME_LEVELS);
						Slider_SetPos(track, (int)(Options.VoiceVolume * (double)VOLUME_LEVELS + 0.5));
						EnableWindow(track, enabled);
					}

					if (GameActive) {

						/*
						**	Shuffle control.
						*/
						HWND button = GetDlgItem(window, IDC_SOUND_SHUFFLE);
						if (button) {
							Button_SetCheck(button, Options.IsScoreShuffle ? BST_CHECKED : BST_UNCHECKED);
							EnableWindow(button, enabled);
						}

						/*
						**	Repeat control.
						*/
						button = GetDlgItem(window, IDC_SOUND_REPEAT);
						if (button) {
							Button_SetCheck(button, Options.IsScoreRepeat ? BST_CHECKED : BST_UNCHECKED);
							EnableWindow(button, enabled);
						}

						/*
						**	Add all the themes to the list box. The list box entries are constructed
						**	and then stored into allocated EMS memory blocks.
						*/
						HWND list = GetDlgItem(window, IDC_SOUND_TRACKLIST);
						if (list) {
							int active_theme = 0;
							int visible_num = 1;

							ListBox_ResetContent(list);

							for (ThemeType index = THEME_FIRST; index < Theme.Max_Themes(); index++) {
								if (Theme.Is_Allowed(index)) {
									char buffer[100];
									int length = Theme.Track_Length(index);
									char const * fullname = Theme.Full_Name(index);

									sprintf(buffer, "%02d - %s [%d:%02d]", visible_num, fullname, length / 60, length % 60);
									visible_num++;

									int row = ListBox_AddString(list, buffer);
									if (row != LB_ERR) {
										ListBox_SetItemData(list, row, index);
										if (Theme.What_Is_Playing() == index) {
											active_theme = row;
										}
									}
								}
							}

							ListBox_SetCurSel(list, active_theme);
							ListBox_SetTopIndex(list, active_theme);
							EnableWindow(list, enabled);
						}
					}

					DialogInitialized = true;
				}

				break;

			case WM_COMMAND:
				switch (LOWORD(wparam)) {

					/*
					**	Toggle the shuffle button.
					*/
					case IDC_SOUND_SHUFFLE:
						Options.Set_Shuffle(Button_GetCheck((HWND)lparam) == BST_CHECKED);
						if (Button_GetCheck((HWND)lparam) == BST_CHECKED) {
							SendDlgItemMessage(window, IDC_SOUND_REPEAT, BM_SETCHECK, BST_UNCHECKED, 0);
							Options.Set_Repeat(false);
						}
						break;

					/*
					**	Toggle the repeat button.
					*/
					case IDC_SOUND_REPEAT:
						Options.Set_Repeat(Button_GetCheck((HWND)lparam) == BST_CHECKED);
						if (Button_GetCheck((HWND)lparam) == BST_CHECKED) {
							SendDlgItemMessage(window, IDC_SOUND_SHUFFLE, BM_SETCHECK, BST_UNCHECKED, 0);
							Options.Set_Shuffle(false);
						}
						break;

					/*
					**	Stop all themes from playing.
					*/
					case IDC_SOUND_STOP:
						if (HIWORD(wparam) == 0) {
							Theme.Queue_Song(THEME_QUIET);
						}
						break;

					case IDOK:
						if (HIWORD(wparam) == 0) {
							HWND button = GetDlgItem(window, IDC_MUSIC_VOLUME);
							if (button) {
								Options.Set_Score_Volume(Slider_GetPos(button) / (double)VOLUME_LEVELS, false);
							}
							button = GetDlgItem(window, IDC_SOUND_VOLUME);
							if (button) {
								Options.Set_Sound_Volume(Slider_GetPos(button) / (double)VOLUME_LEVELS, false);
							}
							button = GetDlgItem(window, IDC_VOICE_VOLUME);
							if (button) {
								Options.Set_Voice_Volume(Slider_GetPos(button) / (double)VOLUME_LEVELS, false);
							}
							int * res = (int *)GetWindowLong(window, DWL_USER);
							*res = IDOK;
						}
						break;

					/*
					**	Start the currently selected theme to play.
					*/
					case IDC_SOUND_PLAY:
						if (HIWORD(wparam) == 0) {
							HWND list = GetDlgItem(window, IDC_SOUND_TRACKLIST);
							if (list) {
								int row = ListBox_GetCurSel(list);
								if (row != LB_ERR) {
									ThemeType theme = (ThemeType)ListBox_GetItemData(list, row);
									Theme.Stop();
									Theme.Queue_Song(theme);
								}
							}
						}
						break;
				}
				break;

			/*
			 * Control volume.
			 */
			case WM_HSCROLL:
				if (DialogInitialized) {
					HWND track = (HWND)lparam;
					if (track == GetDlgItem(window, IDC_MUSIC_VOLUME)) {
						Options.Set_Score_Volume(Slider_GetPos(track) / (double)VOLUME_LEVELS, true);
					} else if (track == GetDlgItem(window, IDC_SOUND_VOLUME)) {
						Options.Set_Sound_Volume(Slider_GetPos(track) / (double)VOLUME_LEVELS, true);
					} else if (track == GetDlgItem(window, IDC_VOICE_VOLUME)) {
						Options.Set_Voice_Volume(Slider_GetPos(track) / (double)VOLUME_LEVELS, true);
					}
				}
				break;
		}
		return(FALSE);
	}

	return(rc);
}
