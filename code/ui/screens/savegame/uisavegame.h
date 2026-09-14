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


// Which of the three dialogs is showing. The numbers are what the document's classes test.
enum UISaveGameMode
{
	UI_SAVE_GAME_LOAD = 0,
	UI_SAVE_GAME_SAVE = 1,
	UI_SAVE_GAME_DELETE = 2,
};


// One row of the file list: what its three cells read, and the entry it stands for.
struct UISaveGameEntry
{
	std::string Description;
	std::string Date;
	std::string Time;
};


// What the dialog shows: which of the three it is, the files it found, the row it opens on,
// the description a save starts from, and the captions the mode gives its title and button.
struct UISaveGameState
{
	UISaveGameMode Mode = UI_SAVE_GAME_LOAD;
	std::vector<UISaveGameEntry> Entries;
	int Selected = 0;
	std::string Description;
	std::string Title;
	std::string AcceptCaption;
	bool AcceptEnabled = true;
};


// Holds the picked row and the typed description until the player accepts. The caller does
// the loading, saving or deleting, as the Win32 dialog's own loop did.
class UISaveGamePresenterClass : public UIPresenterClass
{
	public:
		explicit UISaveGamePresenterClass(UISaveGameState state);

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UISaveGameState State;
		bool Accepted = false;
};


// The RmlUi view over a saved-game presenter, bound to savegame.rml. The presenter must
// outlive it.
std::unique_ptr<UIViewClass> UI_Save_Game_View(UISaveGamePresenterClass & presenter);
