/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "autosave.h"
#include "mpload.h"

#include <string>

/*
 * The save and load policy of a running game: when a save is written and under which name,
 * and how every machine of a network match agrees to load one. Serialization stays in
 * saveload.cpp.
 */
class SaveManagerClass
{
	public:

		AutosaveClass Autosave;
		MultiplayerLoadClass MultiplayerLoad;

		bool Request_Save_Game(char const * file_name, char const * descr, bool quiet = false);
		void Request_Quick_Save(void);
		bool Request_Multiplayer_Save(char const * descr, bool quiet);
		void Post_Save_Notice(int text);

		void Reset_Multiplayer_Save_State(void);
		void Disable_Multiplayer_Saving(void);
		bool Is_Multiplayer_Saving_Allowed(void) const;
		void Multiplayer_Saves_Begin_Match(bool resumed);

		bool Multiplayer_Load_Is_Allowed(void) const;
		bool Multiplayer_Load_Prompt(void);
		bool Multiplayer_Load_Request(int slot);
		void Multiplayer_Load_Receive(int slot);
		bool Multiplayer_Load_Is_Pending(void) const;
		bool Multiplayer_Load_Is_In_Progress(void) const;
		void Multiplayer_Load_Unseat(int index);

		// The frame-boundary save and load work, in the order the main loop ran it.
		void Service(void);

	private:

		void Autosave_Service(void);
		void Quick_Save_Service(void);
		void Process_Pending_Save_Game(void);
		void Process_Pending_Load_Game(void);
		bool Perform_Multiplayer_Load(char const * file_name);
		int Next_Multiplayer_Save_Slot(void);
		void Write_Spawn_Copy(void);

		bool MultiplayerSavingAllowed = true;
		bool MultiplayerSavePending = false;
		bool MultiplayerSaveQuiet = false;
		bool MultiplayerLoadInProgress = false;
		bool SpawnCopyPending = false;
		std::string PendingSaveFileName;
		std::string PendingSaveDescription;
		bool QuickSaveRequested = false;
};

extern SaveManagerClass SaveManager;
