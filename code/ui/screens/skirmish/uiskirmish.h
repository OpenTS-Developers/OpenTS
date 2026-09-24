/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ui/uipreview.h"
#include "ui/uiscreen.h"

#include <memory>
#include <string>
#include <vector>

class UIViewClass;


enum UISkirmishChoice
{
	UI_SKIRMISH_START,
	UI_SKIRMISH_CANCEL,
};


struct UISkirmishState;


class UISkirmishServiceClass
{
	public:
		virtual ~UISkirmishServiceClass(void) = default;
		virtual void Pick_Map(UISkirmishState & state) = 0;
		virtual bool Can_Start(UISkirmishState const & state) = 0;
};


struct UISkirmishOption
{
	std::string Label;
	int Value = 0;
	std::string Color;
};


struct UISkirmishState
{
	std::string Handle;

	std::vector<UISkirmishOption> Sides;
	int Side = 0;
	std::vector<UISkirmishOption> Colors;
	int Color = 0;

	std::string MapName;
	UIMapPreviewImage Preview;

	bool Bases = true;
	bool Crates = true;
	bool Fog = false;
	bool Bridges = true;
	bool Redeploy = true;
	bool ShortGame = false;
	bool MultiEngineer = false;

	int UnitCount = 0;
	int UnitCountMin = 0;
	int UnitCountMax = 0;
	int Credits = 0;
	int CreditsMin = 0;
	int CreditsMax = 0;
	int CreditsStep = 1;
	int TechLevel = 1;
	int TechLevelMax = 1;
	int AILevel = 0;
	int AIPlayers = 1;
	int AIPlayersMax = 7;
	int GameSpeed = 0;
};


class UISkirmishPresenterClass : public UIPresenterClass
{
	public:
		UISkirmishPresenterClass(UISkirmishServiceClass & service, UISkirmishState state);
		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UISkirmishState State;
		UISkirmishChoice Choice = UI_SKIRMISH_CANCEL;

	private:
		UISkirmishServiceClass & Service;
};


std::unique_ptr<UIViewClass> UI_Skirmish_View(UISkirmishPresenterClass & presenter);

void UI_Skirmish_State(UISkirmishState & state);

bool UI_Skirmish_Dialog(void);
