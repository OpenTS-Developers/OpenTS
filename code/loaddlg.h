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

/* $Header: /CounterStrike/LOADDLG.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : LOADDLG.H                                                    *
 *                                                                                             *
 *                   Programmer : Maria Legg, Joe Bostic, Bill Randolph                        *
 *                                                                                             *
 *                   Start Date : March 19, 1995                                               *
 *                                                                                             *
 *                  Last Update : March 19, 1995                                               *
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "session.h"

#include "house.hh"
#include "opents_version.h"

#include <cstddef>
#include <cstdint>

template<class T> class DynamicVectorClass;

class FileEntryClass {
	public:
		char Descr[128];    // save-game description
		char Filename[32];
		unsigned Scenario;  // scenario #
		HousesType House;   // house
		char PlayerName[64];
		int Num;            // save file number (from the extension)
		FILETIME DateTime;  // date/time stamp of file
		bool Valid;         // Is the scenario valid?
		GameType Type;

		FileEntryClass(void) :
			Scenario(0),
			House(HOUSE_NONE),
			Num(-1),
			Valid(true),
			Type(GAME_NORMAL)
		{
			Descr[0] = '\0';
			Filename[0] = '\0';
			PlayerName[0] = '\0';
			DateTime.dwHighDateTime = 0;
			DateTime.dwLowDateTime = 0;
		}
};

class LoadOptionsClass
{
	public:
		/*
		**	This defines the style of the dialog
		*/
		enum LoadStyleType {
			NONE = 0,
			LOAD,
			SAVE,
			WWDELETE
		};

		/*
		 * Save games carry this in their version header. Only a save stamped with the
		 * current value loads; the per-member save format has no reader for anything
		 * else, so every earlier stamp is refused outright.
		 *
		 * It is the packed project version. Development snapshots within one version
		 * share this value without promising that their saves interoperate.
		 */
		enum {
			GAMEVER_OPENTS = OPENTS_VERSION_PACKED
		};

		LoadOptionsClass (void);
		virtual ~LoadOptionsClass (void);

		bool Load(void);
		bool Save(char * description);
		bool Delete(void);

		void Pick_Filename(char * file_name);
		bool Files_Present(void);

		virtual bool Load_File(const char * file_name);
		virtual bool Save_File(const char * file_name, const char * descr);
		virtual bool Delete_File(const char * file_name);
		virtual bool Read_File(FileEntryClass * entry, WIN32_FIND_DATAA * ff);

	protected:
		/*
		**	Internal routines
		*/
		void Clear_List (void);                                     // clears the list & game # array
		void Fill_List (HWND window);                               // fills the list & game # array
		int Num_From_Ext (char *fname);                             // translates filename to file #
		static int __cdecl Compare(const void *p1, const void *p2); // for qsort()

		bool Dialog(void);

		// How many of the newest files the list reads headers for; reading one costs a disk open.
		virtual std::size_t Scan_Limit(void) const {return(SIZE_MAX);}

		/*
		 * These handlers are members so that they can reach the dialog's protected data.
		 */
		static void Load_Dialog_On_WM_COMMAND(HWND window, WPARAM wparam, LPARAM lparam, int id);
		static void Save_Dialog_On_WM_COMMAND(HWND window, WPARAM wparam, LPARAM lparam, int id);
		static void Delete_Dialog_On_WM_COMMAND(HWND window, WPARAM wparam, LPARAM lparam, int id);

		static LRESULT CALLBACK Load_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
		static LRESULT CALLBACK Save_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
		static LRESULT CALLBACK Delete_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

		/*
		**	This is the requested style of the dialog
		*/
		LoadStyleType Style;

		/*
		 * This is the filename extension carried by the save games this dialog works over.
		 * Both the search pattern and every generated filename are built from it, so a
		 * derived dialog can be pointed at a different set of files by changing it alone.
		 */
		char const * Extension;

		/*
		 * This points to the caller's buffer holding the description to suggest for the game
		 * about to be saved, and it receives whatever the player finally types. It is NULL
		 * for the load and delete styles, which have nothing to describe.
		 */
		char * Description;

		/*
		 * This is how much free disk space, expressed in bytes, must be available before the
		 * save dialog will open at all. A save that ran out of room part way through would
		 * leave an unusable file behind, so the room is checked for up front.
		 */
		unsigned int MinSpaceRequired;

	public:
		/*
		 * This points to the routine to call on every pass of the dialog's message loop, or
		 * NULL if there is none. It is what lets the game underneath keep running while the
		 * dialog is up.
		 */
		bool (*Callback)();

		/*
		 * This records the state of the dialog. It holds STATE_PENDING while the dialog is
		 * up and becomes the outcome that closed it, so a handler that rejects the player's
		 * choice can put it back to STATE_PENDING and leave the dialog standing.
		 */
		enum LoadDialogState {
			STATE_PENDING	= -1,		/// Awaiting input
			STATE_OK		= IDOK,		// OK pressed (confirmed action)
			STATE_CLOSE		= IDCANCEL	/// Closed via ESC / system event
		} State;

		/*
		**	This is an array of pointers to FileEntryClass objects.  These objects
		**	are allocated on the fly as files are found, and pointers to them are
		**	added to the vector list.  Thus, all the objects must be free'd before
		**	the vector list is cleared.  This list is used for sorting the files
		**	by date/time.
		*/
		DynamicVectorClass<FileEntryClass *> Files;
};


/*
 * The load dialog over the numbered multiplayer saves of a match. A pick is recorded rather
 * than loaded, since every machine loads together once the master has asked them to.
 */
class MultiplayerLoadOptionsClass : public LoadOptionsClass
{
	public:
		// The list is opened while the other machines wait, so the scan stays short.
		static constexpr std::size_t LIST_LIMIT = 32;

		MultiplayerLoadOptionsClass(void);

		virtual bool Load_File(const char * file_name);
		virtual bool Read_File(FileEntryClass * entry, WIN32_FIND_DATAA * ff);

		char const * Picked_File(void) const {return(Picked);}

	protected:
		virtual std::size_t Scan_Limit(void) const {return(LIST_LIMIT);}

	private:
		char Picked[32];
};
