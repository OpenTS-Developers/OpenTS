/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the in-game options menu: which buttons the running game offers, and the
// entry Game_Options_Dialog calls ahead of its Win32 dialog. The presenter and view live in
// uigameopt.cpp so that the test harness can drive them without the engine.

#include "ui/screens/gameopt/uigameopt.h"

#include "_ui.h"
#include "loaddlg.h"
#include "savemgr.h"
#include "scenario.h"
#include "session.h"
#include "ui/uienginehost.h"
#include "ui/uishell.h"
#include "ui/uiview.h"

#include <cstring>


void UI_Game_Options_State(UIGameOptionsState & state)
{
	bool const reveal = state.Reveal;

	state = UIGameOptionsState();
	state.Reveal = reveal;
	state.Solo = (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH);

	if (state.Solo) {
		bool const present = LoadOptionsClass().Files_Present();
		state.LoadEnabled = present;
		state.DeleteEnabled = present;
		state.BriefingEnabled = (Session.Type != GAME_SKIRMISH);
	} else {
		state.SaveEnabled = SaveManager.Is_Multiplayer_Saving_Allowed();
		state.LoadEnabled = SaveManager.Multiplayer_Load_Is_Allowed()
			&& MultiplayerLoadOptionsClass().Files_Present();
	}
}


bool UI_Game_Options_Dialog(UIGameOptionsChoice & choice)
{
	choice = UI_GAME_OPTIONS_RESUME;

	if (UIShell.Legacy_Dialog_Visible()) {
		return(false);
	}

	bool reveal = true;

	for (;;) {
		UIGameOptionsState state;
		state.Reveal = reveal;
		UI_Game_Options_State(state);

		UIGameOptionsPresenterClass presenter(state);
		std::unique_ptr<UIViewClass> view = UI_Game_Options_View(presenter);

		UIResult result = UI_Run_Modal(*view);
		if (result == UI_RESULT_FAILED_TO_OPEN) {
			// Only the first pass can still fall back; a later one has already shown the menu.
			return(!reveal);
		}

		choice = presenter.Choice;

		// The Win32 menu hid itself around a save or a delete and came back with its buttons
		// re-tested, so those two are done here and the menu opens again without revealing.
		if (choice == UI_GAME_OPTIONS_SAVE) {
			char description[512];
			std::strncpy(description, Scen->Description, sizeof(description) - 1);
			description[sizeof(description) - 1] = '\0';
			LoadOptionsClass().Save(description);
		} else if (choice == UI_GAME_OPTIONS_DELETE) {
			LoadOptionsClass().Delete();
		} else if (choice == UI_GAME_OPTIONS_LOAD) {
			if (LoadOptionsClass().Load()) {
				return(true);
			}
		} else {
			return(true);
		}

		reveal = false;
	}
}
