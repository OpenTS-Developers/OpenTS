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


enum UISaveGameMode
{
	UI_SAVE_GAME_LOAD = 0,
	UI_SAVE_GAME_SAVE = 1,
	UI_SAVE_GAME_DELETE = 2,
};


struct UISaveGameEntry
{
	std::string Description;
	std::string Date;
	std::string Time;
	bool Valid = false;
};


struct UISaveGameState
{
	UISaveGameMode Mode = UI_SAVE_GAME_LOAD;
	std::vector<UISaveGameEntry> Entries;
	int Selected = 0;
	std::string Description;
	std::string Suggested;
	std::string Title;
	std::string AcceptCaption;
	bool AcceptEnabled = true;
};


class UISaveGamePresenterClass : public UIPresenterClass
{
	public:
		explicit UISaveGamePresenterClass(UISaveGameState state);
		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UISaveGameState State;
		bool Accepted = false;

		// Counts picks that refilled the description; the view focuses the field after each.
		int DescriptionPicks = 0;
};


std::unique_ptr<UIViewClass> UI_Save_Game_View(UISaveGamePresenterClass & presenter);
