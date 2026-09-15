/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ui/uiscreen.h"

#include <memory>
#include <string>
#include <vector>

class UIViewClass;


// What the in-game options menu answers with.
enum UIGameOptionsChoice
{
	UI_GAME_OPTIONS_RESUME,
	UI_GAME_OPTIONS_CONTROLS,
	UI_GAME_OPTIONS_BRIEFING,
	UI_GAME_OPTIONS_SAVE,
	UI_GAME_OPTIONS_LOAD,
	UI_GAME_OPTIONS_DELETE,
	UI_GAME_OPTIONS_ABORT,
};


struct UIGameOptionsState;


// The saved-game dialogs the menu raises over itself. The game supplies one; the harness can
// hand the screen a recording one.
class UIGameOptionsServiceClass
{
	public:
		virtual ~UIGameOptionsServiceClass(void) = default;

		// Re-reads which buttons take a press, after one of the dialogs has run.
		virtual void Read(UIGameOptionsState & state) = 0;

		virtual void Save(void) = 0;
		virtual void Delete(void) = 0;

		// True when a game was loaded, which closes the menu with the load.
		virtual bool Load(void) = 0;
};


// What the menu shows: the seven-button arrangement of a campaign or skirmish game, the
// three-button one of a network game, or the Internet game's five buttons over its two
// readings, and which of the buttons take a press.
struct UIGameOptionsState
{
	bool Solo = true;
	bool Internet = false;
	bool BriefingEnabled = true;
	bool LoadEnabled = true;
	bool SaveEnabled = true;
	bool DeleteEnabled = true;

	// The game speed, as the menu's bar shows it: the settings run fastest first and the bar
	// runs the other way, so this is the setting counted from the far end.
	int Speed = 0;
	std::vector<std::string> SpeedNames;
	std::string SpeedName;

	// The rung the game settled its timing on, counted the same way, with the reading beside
	// it. The player is shown this and does not set it.
	int Connection = 0;
	int ConnectionLowest = 0;
	int ConnectionHighest = 0;
	std::string ConnectionName;
};


// A leaf of buttons: each press closes the menu with its choice. Resume, Enter and Escape all
// carry on playing, as the Win32 menu's IDOK and its Resume button do. The Internet game's
// speed bar is the one control that changes anything, and the caller applies it on the way
// out as that menu's Resume button did. A solo game's saved-game dialogs run over the menu
// instead of closing it; a match in play leaves both to the caller, because saving and
// loading one are the other players' business.
class UIGameOptionsPresenterClass : public UIPresenterClass
{
	public:
		UIGameOptionsPresenterClass(UIGameOptionsServiceClass & service, UIGameOptionsState state);

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIGameOptionsState State;
		UIGameOptionsChoice Choice = UI_GAME_OPTIONS_RESUME;

		// Whether the player moved the speed bar, so the caller sends the setting only when
		// the menu actually changed it.
		bool SpeedChanged = false;

	private:
		void Update_Name(void);

		UIGameOptionsServiceClass & Service;
};


// The RmlUi view over an in-game options presenter, bound to gameopt.rml. The presenter must
// outlive it.
std::unique_ptr<UIViewClass> UI_Game_Options_View(UIGameOptionsPresenterClass & presenter);

// Which buttons the running game offers, which of them are live, and what its readings say.
void UI_Game_Options_State(UIGameOptionsState & state);

// Runs the in-game options menu, reopening it after a save or a delete the way the Win32 menu
// came back from behind them. Returns what the player settled on, which in a solo game is
// never save or delete because those are done before it returns.
UIGameOptionsChoice UI_Game_Options_Dialog(void);
