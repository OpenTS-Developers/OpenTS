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

/* $Header: /CounterStrike/LOADDLG.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : LOADDLG.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Maria Legg, Joe Bostic, Bill Randolph                        *
 *                                                                                             *
 *                   Start Date : March 19, 1995                                               *
 *                                                                                             *
 *                  Last Update : June 25, 1995 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   LoadOptionsClass::LoadOptionsClass -- class constructor                                   *
 *   LoadOptionsClass::~LoadOptionsClass -- class destructor                                   *
 *   LoadOptionsClass::Process -- main processing routine                                      *
 *   LoadOptionsClass::Clear_List -- clears the list box & Files arrays                        *
 *   LoadOptionsClass::Fill_List -- fills the list box from the gathered files                 *
 *   LoadOptionsClass::Num_From_Ext -- clears the list box & GameNum arrays                    *
 *   LoadOptionsClass::Compare -- for qsort                                                    *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "autosave.h"

#include "loaddlg.h"

#include "_ui.h"
#include "ui/screens/savegame/uisavegame.h"
#include "ui/uienginehost.h"
#include "ui/uishell.h"
#include "ui/uiview.h"

#include "campaign.h"
#include "conquer.h"
#include "data.h"
#include "gamedirs.h"
#include "globals.h"
#include "houstype.h"
#include "init.h"
#include "language/language.h"
#include "msgbox.h"
#include "ownrdraw.h"
#include "ui/screens/waitbox/uiwaitbox.h"
#include "saveload.h"
#include "savemgr.h"
#include "savever.h"
#include "scenario.h"
#include "session.h"
#include "win.h"

#include <algorithm>
#include <cstdio>
#include <vector>


/***********************************************************************************************
 * LoadOptionsClass::LoadOptionsClass -- class constructor                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      style      style for this load/save dialog (LOAD/SAVE/DELETE)                          *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *=============================================================================================*/
LoadOptionsClass::LoadOptionsClass(void) :
	Files(0),
	Style(NONE),
	Description(NULL),
	Callback(NULL),
	State(STATE_PENDING)
{
	Style = NONE;
	Description = NULL;
	Callback = NULL;
	Extension = "SAV";
	MinSpaceRequired = 2048;
	Files.Clear();
}


/***********************************************************************************************
 * LoadOptionsClass::~LoadOptionsClass -- class destructor                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *=============================================================================================*/
LoadOptionsClass::~LoadOptionsClass(void)
{
	for (int i = 0; i < Files.Count(); i++) {
		delete Files[i];
	}
	Files.Clear();
}


/// <summary>
/// Brings up the load game dialog.
/// This routine is used by the options menu to let the player pick a saved game and
/// resume it.
/// </summary>
/// <returns>bool; Was a game loaded?</returns>
bool LoadOptionsClass::Load(void)
{
	Style = LOAD;
	Description = NULL;
	return(Dialog());
}


/// <summary>
/// Brings up the save game dialog.
/// This routine is used by the options menu when the player wants to record the current
/// game. The description offered is used to prime the edit field.
/// </summary>
/// <param name="description">The description to suggest for the saved game.</param>
/// <returns>bool; Was the game saved?</returns>
bool LoadOptionsClass::Save(char *description)
{
	Style = SAVE;
	Description = description;
	return(Dialog());
}


/// <summary>
/// Brings up the delete game dialog.
/// This routine is used by the options menu to let the player clear out save games that
/// are no longer wanted.
/// </summary>
/// <returns>bool; Did the player go through with the deletion?</returns>
bool LoadOptionsClass::Delete(void)
{
	Style = WWDELETE;
	Description = NULL;
	return(Dialog());
}


/// <summary>
/// Handles a control notification from the load game dialog.
/// This routine records how the player left the dialog, so that the processing loop
/// knows whether a game was chosen or the player backed out.
/// </summary>
/// <param name="wparam">The identifier of the control that was activated.</param>
/// <param name="lparam">Window handle of the control that was activated.</param>
/// <param name="id">The notification code that accompanied the control.</param>
void LoadOptionsClass::Load_Dialog_On_WM_COMMAND(HWND window, WPARAM wparam, LPARAM lparam, int id)
{
	LoadOptionsClass * _this = (LoadOptionsClass *)GetWindowLongPtr(window, DWLP_USER);
	switch ((int)wparam) {
		case IDC_MISSION_LOAD_LIST:
			if (id == 2 && ListBox_GetCount((HWND)lparam) > 0) {
				_this->State = STATE_OK;
			}
			break;

		case IDOK:
		case IDCANCEL:
			if (id == 0) {
				_this->State = (LoadDialogState)wparam;
			}
			break;
	}
}


/// <summary>
/// Handles a control notification from the save game dialog.
/// Picking a game in the list copies its description into the edit field, so that the
/// player can save over an existing game without typing the name out again. The buttons
/// record how the player left the dialog.
/// </summary>
/// <param name="wparam">The identifier of the control that was activated.</param>
/// <param name="lparam">Window handle of the control that was activated.</param>
/// <param name="id">The notification code that accompanied the control.</param>
void LoadOptionsClass::Save_Dialog_On_WM_COMMAND(HWND window, WPARAM wparam, LPARAM lparam, int id)
{
	LoadOptionsClass * _this = (LoadOptionsClass *)GetWindowLongPtr(window, DWLP_USER);
	switch ((int)wparam) {
		case IDC_MISSION_SAVE_LIST:

			/*
			**	If the user clicks on the list, see if the there is a new current
			**	item; if so, and if we're in SAVE mode, copy the list item into
			**	the save-game description field.
			*/
			if (id == 1 && ListBox_GetCount((HWND)lparam) > 0) {
				int row = ListBox_GetCurSel((HWND)lparam);
				if (row != LB_ERR) {

					/*
					**	Copy the game's description, UNLESS it's the empty slot; if
					**	it is, set the edit buffer to empty.
					*/
					FileEntryClass * fdata = (FileEntryClass *)ListBox_GetItemData((HWND)lparam, row);
					if (fdata->Valid) {
						SetWindowText(GetDlgItem(window, IDC_MISSION_SAVE_DESC), fdata->Descr);
					} else if (_this->Description != NULL) {
						SetWindowText(GetDlgItem(window, IDC_MISSION_SAVE_DESC), _this->Description);
					}
					SetFocus(GetDlgItem(window, IDC_MISSION_SAVE_DESC));
					Edit_SetSel(GetDlgItem(window, IDC_MISSION_SAVE_DESC), 0, -1);
				}
			}
			break;

		case IDOK:
		case IDCANCEL:
			if (id == 0) {
				_this->State = (LoadDialogState)wparam;
			}
			break;
	}
}


/// <summary>
/// Handles a control notification from the delete game dialog.
/// This routine records how the player left the dialog, so that the processing loop
/// knows whether to go ahead with the deletion.
/// </summary>
/// <param name="wparam">The identifier of the control that was activated.</param>
/// <param name="id">The notification code that accompanied the control.</param>
void LoadOptionsClass::Delete_Dialog_On_WM_COMMAND(HWND window, WPARAM wparam, LPARAM lparam, int id)
{
	LoadOptionsClass * _this = (LoadOptionsClass *)GetWindowLongPtr(window, DWLP_USER);
	switch ((int)wparam) {
		case IDOK:
		case IDCANCEL:
			if (id == 0) {
				_this->State = (LoadDialogState)wparam;
			}
			break;
	}
}


/// <summary>
/// Handles messages for the load game dialog.
/// The owner draw system is given first refusal on every message. What is left over is
/// used to set up the file list columns and to pass control activity along to the
/// command handler.
/// </summary>
/// <returns>Returns with the message result, or FALSE if nothing here dealt with it.</returns>
INT_PTR CALLBACK LoadOptionsClass::Load_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	INT_PTR rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);

	if (rc == 0) {

		switch (message) {
			case WM_MOVING:
				return(On_WM_MOVING(window, wparam, lparam));

			case WM_COMMAND:
				Load_Dialog_On_WM_COMMAND(window, LOWORD(wparam), lparam, HIWORD(wparam));
				break;

			case OD_SUBCLASSED:
				SendDlgItemMessage(window, IDC_MISSION_LOAD_LIST, OD_ADDCOLUMN, 0xF9, 2);
				SendDlgItemMessage(window, IDC_MISSION_LOAD_LIST, OD_ADDCOLUMN, 0x38, 255);
				SendDlgItemMessage(window, IDC_MISSION_LOAD_LIST, OD_ADDCOLUMN, 0, 315);
				break;
		}
		return(FALSE);
	}
	return(rc);
}


/// <summary>
/// Handles messages for the save game dialog.
/// The owner draw system is given first refusal on every message. What is left over is
/// used to set up the file list columns, cap the length of the description the player
/// may type, and pass control activity along to the command handler.
/// </summary>
/// <returns>Returns with the message result, or FALSE if nothing here dealt with it.</returns>
INT_PTR CALLBACK LoadOptionsClass::Save_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	INT_PTR rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);

	if (rc == 0) {

		switch (message) {
			case WM_MOVING:
				return(On_WM_MOVING(window, wparam, lparam));

			case WM_COMMAND:
				Save_Dialog_On_WM_COMMAND(window, LOWORD(wparam), lparam, HIWORD(wparam));
				break;

			case WM_INITDIALOG:
				SendMessage(GetDlgItem(window, IDC_MISSION_SAVE_DESC), EM_SETLIMITTEXT, 79, 0);
				break;

			case OD_SUBCLASSED:
				SendDlgItemMessage(window, IDC_MISSION_SAVE_LIST, OD_ADDCOLUMN, 0xF9, 2);
				SendDlgItemMessage(window, IDC_MISSION_SAVE_LIST, OD_ADDCOLUMN, 0x38, 255);
				SendDlgItemMessage(window, IDC_MISSION_SAVE_LIST, OD_ADDCOLUMN, 0, 315);
				break;
		}
		return(FALSE);
	}
	return(rc);
}


/// <summary>
/// Handles messages for the delete game dialog.
/// The owner draw system is given first refusal on every message. What is left over is
/// used to set up the file list columns and to pass control activity along to the
/// command handler.
/// </summary>
/// <returns>Returns with the message result, or FALSE if nothing here dealt with it.</returns>
INT_PTR CALLBACK LoadOptionsClass::Delete_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	INT_PTR rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);

	if (rc == 0) {

		switch (message) {
			case WM_COMMAND:
				Delete_Dialog_On_WM_COMMAND(window, LOWORD(wparam), lparam, HIWORD(wparam));
				break;

			case WM_MOVING:
				return(On_WM_MOVING(window, wparam, lparam));

			case OD_SUBCLASSED:
				SendDlgItemMessage(window, IDC_MISSION_DELETE_LIST, OD_ADDCOLUMN, 0xF9, 2);
				SendDlgItemMessage(window, IDC_MISSION_DELETE_LIST, OD_ADDCOLUMN, 0x38, 255);
				SendDlgItemMessage(window, IDC_MISSION_DELETE_LIST, OD_ADDCOLUMN, 0, 315);
				break;
		}
		return(FALSE);
	}
	return(rc);
}


/// <summary>
/// Is a saved game of this name already there? Asked before one is written, since a name the
/// folder holds is written over rather than added to.
/// </summary>
static bool Saved_Game_Exists(char const * name)
{
	return(GetFileAttributes(Saved_Game_Name(name).c_str()) != INVALID_FILE_ATTRIBUTES);
}


/***********************************************************************************************
 * LoadOptionsClass::Process -- main processing routine                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      false = User cancelled, true = operation completed                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *=============================================================================================*/
/// <summary>
/// Runs the load, save or delete dialog as an RmlUi screen, doing the same work on the
/// player's pick that the Win32 loop does and reopening wherever that loop stayed standing.
/// </summary>
/// <returns>bool; Was the screen shown at all? False leaves State untouched and the caller
/// opens its Win32 dialog instead.</returns>
bool LoadOptionsClass::Rml_Dialog(void)
{
	char buffer[256];

	for (;;) {
		Gather_Files();

		UISaveGameState state;
		switch (Style) {
			case SAVE:
				state.Mode = UI_SAVE_GAME_SAVE;
				state.Title = "SAVE";
				state.AcceptCaption = "Save";
				break;

			case WWDELETE:
				state.Mode = UI_SAVE_GAME_DELETE;
				state.Title = "DELETE";
				state.AcceptCaption = "Delete";
				break;

			default:
				state.Mode = UI_SAVE_GAME_LOAD;
				state.Title = "LOAD";
				state.AcceptCaption = "Load";
				break;
		}

		for (int index = 0; index < Files.Count(); index++) {
			FileEntryClass const * file = Files[index];
			UISaveGameEntry entry;
			entry.Description = file->Descr;
			char date[128];
			char timeofday[128];
			if (Stamp_Strings(*file, date, sizeof(date), timeofday, sizeof(timeofday))) {
				entry.Date = date;
				entry.Time = timeofday;
			}
			state.Entries.push_back(entry);
		}

		state.Selected = Initial_Row();
		state.AcceptEnabled = Files.Count() > 0;
		if (Style == SAVE && Description != NULL) {
			state.Description = Description;
		}

		UISaveGamePresenterClass presenter(std::move(state));
		std::unique_ptr<UIViewClass> view = UI_Save_Game_View(presenter);

		UIResult result = UI_Run_Modal(*view);
		if (result == UI_RESULT_FAILED_TO_OPEN) {
			Clear_List();
			return(false);
		}

		if (!presenter.Accepted) {
			Clear_List();
			State = STATE_CLOSE;
			return(true);
		}

		int const row = presenter.State.Selected;
		FileEntryClass * entry = (row >= 0 && row < Files.Count()) ? Files[row] : NULL;
		State = STATE_OK;

		if (entry == NULL) {
			Clear_List();
			return(true);
		}

		switch (Style) {

			/*
			**	Load: a failed load reports itself and leaves the list standing so that the
			**	player can try another game.
			*/
			case LOAD:
				if (entry->Num != -1) {
					Init_Campaigns();
				}
				if (!Load_File(entry->Filename)) {
					WWMessageBox().Process(TXT_ERROR_LOADING_GAME, TXT_OK, TXT_NONE, TXT_NONE);
					State = STATE_PENDING;
				}
				break;

			/*
			**	Save: an empty description or a refused overwrite leaves the list standing.
			*/
			case SAVE: {
				std::string typed = presenter.State.Description;
				if (typed.empty()) {
					WWMessageBox().Process(TXT_MUSTENTER_DESCRIPTION, TXT_OK, TXT_NONE, TXT_NONE);
					State = STATE_PENDING;
					break;
				}

				char const * filename = NULL;
				char picked[256];
				if (entry->Valid) {
					filename = entry->Filename;
				} else {
					Pick_Filename(picked);
					filename = picked;
				}

				if (filename == NULL) {
					break;
				}

				if (Saved_Game_Exists(filename)
					&& WWMessageBox()._Process(TXT_CONFIRM_SAVE, 1, TXT_YES, TXT_NO, TXT_NONE)) {
					State = STATE_PENDING;
					break;
				}

				if (!Save_File(filename, typed.c_str())) {
					WWMessageBox().Process(TXT_ERROR_SAVING_GAME, TXT_OK, TXT_NONE, TXT_NONE);
					State = STATE_PENDING;
					break;
				}

				int const confirmation = Save_Confirmation();
				if (confirmation != TXT_NONE) {
					WWMessageBox().Process(confirmation, TXT_OK, TXT_NONE, TXT_NONE);
				}
				if (Description) {
					strcpy(Description, typed.c_str());
				}
				break;
			}

			/*
			**	Delete: the list stays standing while there is anything left to delete.
			*/
			case WWDELETE:
				sprintf(buffer, "%s\n%s", Fetch_String(TXT_DELETE_FILE_QUERY), entry->Descr);
				if (!WWMessageBox()._Process(buffer, 1, TXT_YES, TXT_NO, TXT_NONE)) {
					Delete_File(entry->Filename);
					if (Files.Count() > 1) {
						State = STATE_PENDING;
					}
				} else {
					State = STATE_PENDING;
				}
				break;

			default:
				break;
		}

		Clear_List();

		if (State != STATE_PENDING) {
			return(true);
		}
	}
}


bool LoadOptionsClass::Dialog(void)
{
	// A save that ran out of room part way through would leave an unusable file behind.
	if (Style == SAVE && Disk_Space_Available() < MinSpaceRequired) {
		WWMessageBox().Process(TXT_DISKFULL, TXT_OK, TXT_NONE, TXT_NONE);
		return(false);
	}

	if (UIShell.Use_Rml() && !UIShell.Legacy_Dialog_Visible()) {
		State = STATE_PENDING;
		if (Rml_Dialog()) {
			return(State == STATE_OK);
		}
	}

	/*
	**	Dialog variables
	*/
	HWND dialog = 0;
	HWND list = 0;

	char buffer[256];

	switch (Style) {
		case LOAD:
			dialog = OwnerDraw::Begin_Dialog(IDD_MISSION_LOAD, Load_Dialog_Proc);
			list = GetDlgItem(dialog, IDC_MISSION_LOAD_LIST);
			break;

		case SAVE:
			dialog = OwnerDraw::Begin_Dialog(IDD_MISSION_SAVE, Save_Dialog_Proc);
			list = GetDlgItem(dialog, IDC_MISSION_SAVE_LIST);
			break;

		case WWDELETE:
			dialog = OwnerDraw::Begin_Dialog(IDD_MISSION_DELETE, Delete_Dialog_Proc);
			list = GetDlgItem(dialog, IDC_MISSION_DELETE_LIST);
			break;
	}

	State = STATE_PENDING;

	if (dialog) {

		/*
		**	Initialize.
		*/
		SetWindowLongPtr(dialog, DWLP_USER, (LONG_PTR)this);

		if (list != 0) {
			Fill_List(list);
			EnableWindow(GetDlgItem(dialog, 1), bool(ListBox_GetCount(list) > 0));
		}

		OwnerDraw::Display_Dialog(dialog);

		/*
		**	Main Processing Loop.
		*/
		do {
			while (State == STATE_PENDING) {
				if (OwnerDraw::Dialog_Message_Handler() == true) {
					State = STATE_CLOSE;
				}

				/*
				**	Invoke game callback.
				*/
				if (Callback) {
					Callback();
				}

				/*
				**	If we have just received input focus again after running in the background then
				**	we need to redraw.
				*/
				if (!GameActive) {
					Title_Screen_Restore(0);
				}
			}

			if (State == STATE_OK) {
				LRESULT row = ListBox_GetCurSel(list);

				if (row != LB_ERR) {
					FileEntryClass * entry = (FileEntryClass *)ListBox_GetItemData(list, row);

					/*
					**	Process input.
					*/
					switch (Style) {
						/*
						**	Load: if load fails, present a message, and stay in the dialog
						**	to allow the user to try another game
						*/
						case LOAD: {
							if (entry->Num != -1) {
								Init_Campaigns();
							}

							ShowWindow(dialog, SW_HIDE);
							UpdateWindow(MainWindow);

							if (!Load_File(entry->Filename)) {
								WWMessageBox().Process(TXT_ERROR_LOADING_GAME, TXT_OK, TXT_NONE, TXT_NONE);
								ShowWindow(dialog, SW_SHOW);
								State = STATE_PENDING;
							}
							break;
						}

						/*
						**	Save: Save the game & exit the dialog
						*/
						case SAVE: {
							GetWindowText(GetDlgItem(dialog, IDC_MISSION_SAVE_DESC), buffer, DESCRIP_MAX+36);

							if (strlen(buffer) == 0) {
								WWMessageBox().Process(TXT_MUSTENTER_DESCRIPTION, TXT_OK, TXT_NONE, TXT_NONE);
								SetFocus(GetDlgItem(dialog, IDC_MISSION_SAVE_DESC));
								Edit_SetSel(GetDlgItem(dialog, IDC_MISSION_SAVE_DESC), -1, -1);
								State = STATE_PENDING;
								break;
							}

							const char * filename = NULL;
							char test_filename[256];

							if (entry && entry->Valid) {
								filename = entry->Filename;
							} else {
								Pick_Filename(test_filename);
								filename = test_filename;
							}

							if (filename != NULL) {
								bool exists = Saved_Game_Exists(filename);
								if (exists && WWMessageBox()._Process(TXT_CONFIRM_SAVE, 1, TXT_YES, TXT_NO, TXT_NONE))
									State = STATE_PENDING;
								else {
									if (!Save_File(filename, buffer)) {
										WWMessageBox().Process(TXT_ERROR_SAVING_GAME, TXT_OK, TXT_NONE, TXT_NONE);
										State = STATE_PENDING;
									} else {
										int confirmation = Save_Confirmation();
										if (confirmation != TXT_NONE) {
											WWMessageBox().Process(confirmation, TXT_OK, TXT_NONE, TXT_NONE);
										}
										if (Description) {
											strcpy(Description, buffer);
										}
									}
								}
							}
							break;
						}

						/*
						**	Delete: delete the file & stay in the dialog, to allow the user
						**	to delete multiple files.
						*/
						case WWDELETE: {
							sprintf(buffer, "%s\n%s", Fetch_String(TXT_DELETE_FILE_QUERY), entry->Descr);

							if (!WWMessageBox()._Process(buffer, 1, TXT_YES, TXT_NO, TXT_NONE)) {
								Delete_File(entry->Filename);
								ListBox_DeleteString(list, row);
								ListBox_SetCurSel(list, 0);
								if (ListBox_GetCount(list) > 0) {
									State = STATE_PENDING;
									break;
								}
							} else {
								State = STATE_PENDING;
							}
							break;
						}
					}
				}
			}
		} while (State == STATE_PENDING);

		Clear_List();

		OwnerDraw::End_Dialog(dialog);
	}

	return(State == STATE_OK ? true : false);
}


/// <summary>
/// Fetches a save game filename that is not already in use.
/// This routine is used when the player saves into an empty slot and there is no
/// existing file to write over.
/// </summary>
/// <param name="name">Buffer to fill in with the filename chosen.</param>
/// <remarks>Be sure the buffer is big enough to hold a complete filename.</remarks>
void LoadOptionsClass::Pick_Filename(char *name)
{
	do {
		sprintf(name, "SAVE%04lX.%3s", rand(), Extension);
	} while (Saved_Game_Exists(name));
}


/***********************************************************************************************
 * LoadOptionsClass::Clear_List -- clears the list box & Files arrays                          *
 *                                                                                             *
 * This step is essential, because it frees all the strings allocated for list items.          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *=============================================================================================*/
void LoadOptionsClass::Clear_List(void)
{
	/*
	**	Clear the array of game numbers
	*/
	for (int i = 0; i < Files.Count(); i++) {
		delete Files[i];
	}
	Files.Clear();
}


/***********************************************************************************************
 * LoadOptionsClass::Gather_Files -- reads the saved games into the Files list               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *   06/25/1995 JLB : Shows which saved games are "(old)".                                     *
 *=============================================================================================*/
void LoadOptionsClass::Gather_Files(void)
{
	FileEntryClass * fdata = NULL;  // for adding entries to 'Files'
	WIN32_FIND_DATAA ff;            // for FindFirstFile

	/*
	**	Make sure the list is empty
	*/
	Clear_List();

	/*
	**	Add the Empty Slot entry
	*/
	if (Style == SAVE) {
		fdata = new FileEntryClass;
		strcpy(fdata->Descr, Fetch_String(TXT_EMPTY_SLOT));
		if (PlayerPtr != NULL) {
			fdata->Scenario = Scen->Scenario;
			fdata->House = Scen->PlayerHouse;
			fdata->Num = Scen->Campaign;
			strcpy(fdata->PlayerName, PlayerPtr->Class->GivenName);
		} else {
			fdata->Scenario = 0;
			fdata->House = (HousesType)Session.House;
			fdata->Num = -1;
			strcpy(fdata->PlayerName, Session.Handle);
		}
		SYSTEMTIME time;
		GetSystemTime(&time);
		SystemTimeToFileTime(&time, &fdata->DateTime);
		fdata->Type = Session.Type;
		fdata->Valid = false;
		Files.Add(fdata);
	}

	char buffer[128];
	sprintf(buffer, "*.%3s", Extension);

	/*
	**	Find all savegame files
	*/
	std::vector<WIN32_FIND_DATAA> found;

	HANDLE hFind = FindFirstFile(Saved_Game_Name(buffer).c_str(), &ff);

	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((ff.dwFileAttributes & (FILE_ATTRIBUTE_TEMPORARY|FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_HIDDEN)) != 0) {
				continue;
			}
			found.push_back(ff);
		} while (FindNextFile(hFind, &ff));

		FindClose(hFind);
	}

	// Newest first, so a bounded scan reads the headers of the files that matter.
	std::sort(found.begin(), found.end(), [](WIN32_FIND_DATAA const & a, WIN32_FIND_DATAA const & b) {
		return(CompareFileTime(&a.ftLastWriteTime, &b.ftLastWriteTime) > 0);
	});
	if (found.size() > Scan_Limit()) {
		found.resize(Scan_Limit());
	}

	fdata = NULL;
	for (WIN32_FIND_DATAA & record : found) {
		if (fdata == NULL) {
			fdata = new FileEntryClass;
		}

		/*
		**	get the game's info; if success, add it to the list
		*/
		if (Read_File(fdata, &record) == true) {
			Files.Add(fdata);
			fdata = NULL;
		}
	}

	if (fdata != NULL) {
		delete fdata;
	}

	if (Files.Count() > 0) {

		/*
		**	Now sort the list in order of Date/Time (newest first, oldest last)
		*/
		qsort((void *)(&Files[0]), Files.Count(), sizeof(class FileEntryClass *), LoadOptionsClass::Compare);
	}
}


/// <summary>
/// Prints a save's date and its time the way the list's own columns read them.
/// </summary>
/// <returns>bool; Does the save carry a stamp at all?</returns>
bool LoadOptionsClass::Stamp_Strings(FileEntryClass const & entry, char * date, std::size_t datesize,
	char * timeofday, std::size_t timesize)
{
	date[0] = '\0';
	timeofday[0] = '\0';

	if (entry.DateTime.dwHighDateTime == -1 && entry.DateTime.dwLowDateTime == -1) {
		return(false);
	}

	FILETIME ft;
	SYSTEMTIME time;
	FileTimeToLocalFileTime(&entry.DateTime, &ft);
	FileTimeToSystemTime(&ft, &time);
	GetDateFormat(LANG_USER_DEFAULT, TIME_NOMINUTESORSECONDS, &time, NULL, date, (int)datesize);
	GetTimeFormat(LANG_USER_DEFAULT, TIME_NOSECONDS, &time, NULL, timeofday, (int)timesize);
	return(true);
}


/// <summary>
/// The row the dialog opens on: the newest loadable save for a load, and the first row,
/// which is the empty slot, for a save or a delete.
/// </summary>
int LoadOptionsClass::Initial_Row(void) const
{
	if (Style == LOAD) {
		for (int index = 0; index < Files.Count(); index++) {
			if (Files[index]->Valid) {
				return(index);
			}
		}
	}
	return(0);
}


void LoadOptionsClass::Fill_List(HWND window)
{
	OwnerDraw::CellData thecell;
	char buffer[128];

	Gather_Files();

	if (Files.Count() > 0) {
		ListBox_ResetContent(window);

		/*
		**	Now add every file's name to the list box
		*/
		for (int i = 0; i < Files.Count(); i++) {
			FileEntryClass * fdata = Files[i];

			int row = ListBox_AddString(window, fdata);

			if (fdata->Type != GAME_NORMAL) {
				thecell.type = OwnerDraw::CellData::TEXT;
				thecell.string.set("*");
				SendMessage(window, OD_SETCELL, MAKEWPARAM(200, row), (LPARAM)&thecell);
			}

			char timeofday[128];
			if (Stamp_Strings(*fdata, buffer, sizeof(buffer), timeofday, sizeof(timeofday))) {
				thecell.type = OwnerDraw::CellData::TEXT;
				thecell.string.set(buffer);
				SendMessage(window, OD_SETCELL, MAKEWPARAM(255, row), (LPARAM)&thecell);
				thecell.type = OwnerDraw::CellData::TEXT;
				thecell.string.set(timeofday);
				SendMessage(window, OD_SETCELL, MAKEWPARAM(315, row), (LPARAM)&thecell);
			}

			ListBox_SetItemData(window, row, (LPARAM)fdata);
		}

		int const row = Initial_Row();
		ListBox_SetCurSel(window, row);
		ListBox_SetTopIndex(window, row);
	}
}


/// <summary>
/// Are there any save games available to load?
/// This routine is used to decide whether the load option should be offered to the
/// player at all. It settles the question as cheaply as it can, so it stops at the
/// first save game it can actually read.
/// </summary>
/// <returns>bool; Was at least one loadable save game found?</returns>
bool LoadOptionsClass::Files_Present(void)
{
	bool files_found = false;

	char pattern[64];
	sprintf(pattern, "*.%3s", Extension);

	WIN32_FIND_DATAA find_data;
	HANDLE hFind = FindFirstFile(Saved_Game_Name(pattern).c_str(), &find_data);

	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((find_data.dwFileAttributes & (FILE_ATTRIBUTE_TEMPORARY|FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_HIDDEN)) != 0) {
				continue;
			}

			FileEntryClass entry;
			if (Read_File(&entry, &find_data) == true) {
				files_found = true;
				break;
			}
		} while (FindNextFile(hFind, &find_data));

		FindClose(hFind);
	}

	return(files_found);
}


/***********************************************************************************************
 * LoadOptionsClass::Compare -- for qsort                                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      p1,p2      ptrs to elements to compare                                                 *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      0 = same, -1 = (*p1) goes BEFORE (*p2), 1 = (*p1) goes AFTER (*p2)                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *=============================================================================================*/
int __cdecl LoadOptionsClass::Compare(const void * p1, const void * p2)
{
	FileEntryClass * fe1, * fe2;

	fe1 = *((FileEntryClass **)p1);
	fe2 = *((FileEntryClass **)p2);

	int res = CompareFileTime(&fe1->DateTime, &fe2->DateTime);
	return(-res);
}


/// <summary>
/// Restores the game held in the file specified.
/// A message box is displayed while the load runs, and the scenario is taken out of
/// play first so that nothing tries to tick while the game state is being replaced.
/// </summary>
/// <returns>bool; Was the game loaded?</returns>
bool LoadOptionsClass::Load_File(const char * file_name)
{
	UIWaitBoxClass box;
	box.Show(Fetch_String(TXT_LOADING));
	ScenarioActive = false;
	TacticalActive = false;
	bool loaded = Load_Game(file_name);
	box.Hide();
	return(loaded);
}


/// <summary>
/// Saves the current game to the file specified.
/// A message box is displayed while the save runs, since writing a save game takes long
/// enough that the player would otherwise think the game had locked up.
/// </summary>
/// <param name="descr">The description to record alongside the saved game.</param>
/// <returns>bool; Was the game saved?</returns>
bool LoadOptionsClass::Save_File(const char * file_name, const char * descr)
{
	UIWaitBoxClass box;
	box.Show(Fetch_String(TXT_SAVING_GAME));
	bool saved = SaveManager.Request_Save_Game(file_name, descr, false,
		SaveManagerClass::NoticeType::Requested);
	box.Hide();
	return(saved);
}


/// <summary>
/// A saved game reports itself in the message list at the frame boundary, so the dialog shows
/// no box of its own.
/// </summary>
int LoadOptionsClass::Save_Confirmation(void) const
{
	return(TXT_NONE);
}


/// <summary>
/// Removes the save game file specified.
/// </summary>
/// <returns>bool; Was the file deleted?</returns>
bool LoadOptionsClass::Delete_File(const char * file_name)
{
	if (DeleteFile(Saved_Game_Name(file_name).c_str()) == TRUE) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Fills in a save game list entry from a file found on disk.
/// This routine peeks at the save game's header to recover the description, scenario
/// and player it belongs to. A save written by an older game version is still accepted,
/// but its description is marked so the player can tell.
/// </summary>
/// <param name="fdata">The list entry to fill in.</param>
/// <param name="ff">The find record naming the file to examine.</param>
/// <returns>bool; Was a usable save game found in the file?</returns>
bool LoadOptionsClass::Read_File(FileEntryClass * fdata, WIN32_FIND_DATAA * ff)
{
	if (fdata == NULL && ff == NULL) {
		return(false);
	}

	SaveVersionInfo savever;

	/*
	 * get the game's info;
	 */
	bool ok = Get_Savefile_Info(ff->cFileName, &savever);
	if (!ok) {
		return(false);
	}

	if (savever.Get_Internal_Version() != ExpectedGameVersion) {
		return(false);
	}

	snprintf(fdata->Descr, sizeof(fdata->Descr), "%s", savever.Get_Scenario_Description());

	fdata->Valid = ok;
	fdata->Scenario = savever.Get_Scenario_Number();
	fdata->Num = savever.Get_Campaign_Number();
	fdata->Type = (GameType)savever.Get_Game_Type();
	strcpy(fdata->Filename, ff->cFileName);
	strcpy(fdata->PlayerName, savever.Get_Player_House());
	if (strlen(fdata->Filename) == 0) {
		strcpy(fdata->Filename, ff->cAlternateFileName);
	}
	fdata->DateTime.dwHighDateTime = ff->ftLastWriteTime.dwHighDateTime;
	fdata->DateTime.dwLowDateTime = ff->ftLastWriteTime.dwLowDateTime;
	return(true);
}


MultiplayerLoadOptionsClass::MultiplayerLoadOptionsClass(void)
{
	Extension = "NET";
	Picked[0] = '\0';
}


/// <summary>
/// Records the pick without loading it; every machine loads together once the master asks.
/// </summary>
bool MultiplayerLoadOptionsClass::Load_File(const char * file_name)
{
	std::snprintf(Picked, sizeof(Picked), "%s", file_name);
	return(true);
}


/// <summary>
/// Lists a numbered save of this kind of game and nothing else.
/// </summary>
bool MultiplayerLoadOptionsClass::Read_File(FileEntryClass * entry, WIN32_FIND_DATAA * ff)
{
	if (entry == NULL || ff == NULL || Multiplayer_Save_Slot(ff->cFileName) < 0) {
		return(false);
	}
	return(LoadOptionsClass::Read_File(entry, ff) && entry->Type == Session.Type);
}
