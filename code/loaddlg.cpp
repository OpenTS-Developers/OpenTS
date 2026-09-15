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

#include "ui/screens/savegame/uisavegame.h"
#include "ui/uienginehost.h"
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
#include "ui/screens/waitbox/uiwaitbox.h"
#include "saveload.h"
#include "savemgr.h"
#include "savever.h"
#include "scenario.h"
#include "session.h"
#include "utf8.h"
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
	DescriptionSize(0),
	State(STATE_PENDING)
{
	Style = NONE;
	Description = NULL;
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
/// <param name="description">The description to suggest for the saved game. It receives the
/// one the player typed, cut to what the buffer holds.</param>
/// <param name="size">The bytes the description buffer holds.</param>
/// <returns>bool; Was the game saved?</returns>
bool LoadOptionsClass::Save(char *description, std::size_t size)
{
	Style = SAVE;
	Description = description;
	DescriptionSize = size;
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
/// Runs the load, save or delete screen, acting on the player's pick and reopening wherever
/// the dialog it replaced stayed standing.
/// </summary>
/// <returns>bool; Did the player commit to the action the screen offers?</returns>
bool LoadOptionsClass::Dialog(void)
{
	// A save that ran out of room part way through would leave an unusable file behind.
	if (Style == SAVE && Disk_Space_Available() < MinSpaceRequired) {
		WWMessageBox().Process(TXT_DISKFULL, TXT_OK, TXT_NONE, TXT_NONE);
		return(false);
	}

	State = STATE_PENDING;

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

		UIResult result = UI_Run_Modal(*view, true);
		if (result == UI_RESULT_FAILED_TO_OPEN) {
			Clear_List();
			return(false);
		}

		if (!presenter.Accepted) {
			Clear_List();
			State = STATE_CLOSE;
			return(false);
		}

		int const row = presenter.State.Selected;
		FileEntryClass * entry = (row >= 0 && row < Files.Count()) ? Files[row] : NULL;
		State = STATE_OK;

		if (entry == NULL) {
			Clear_List();
			return(State == STATE_OK);
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
				// The field bounds characters; the caller's buffer bounds bytes, and the
				// file must carry what the caller gets back.
				std::string typed = presenter.State.Description;
				if (Description != NULL && DescriptionSize > 0) {
					typed.resize(UTF8::Boundary_Before(typed.c_str(), DescriptionSize - 1));
				}
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
				if (Description != NULL) {
					UTF8::Copy(Description, DescriptionSize, typed.c_str());
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
			return(State == STATE_OK);
		}
	}
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
