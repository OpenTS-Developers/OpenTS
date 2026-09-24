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


class UIGameOptionsServiceClass
{
	public:
		virtual ~UIGameOptionsServiceClass(void) = default;
		virtual void Read(UIGameOptionsState & state) = 0;
		virtual void Save(void) = 0;
		virtual void Delete(void) = 0;
		virtual bool Load(void) = 0;
};


struct UIGameOptionsState
{
	bool Solo = true;
	bool Internet = false;
	bool BriefingEnabled = true;
	bool LoadEnabled = true;
	bool SaveEnabled = true;
	bool DeleteEnabled = true;

	int Speed = 0;
	std::vector<std::string> SpeedNames;
	std::string SpeedName;

	int Connection = 0;
	int ConnectionLowest = 0;
	int ConnectionHighest = 0;
	std::string ConnectionName;
};


class UIGameOptionsPresenterClass : public UIPresenterClass
{
	public:
		UIGameOptionsPresenterClass(UIGameOptionsServiceClass & service, UIGameOptionsState state);
		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIGameOptionsState State;
		UIGameOptionsChoice Choice = UI_GAME_OPTIONS_RESUME;

		bool SpeedChanged = false;

	private:
		void Update_Name(void);

		UIGameOptionsServiceClass & Service;
};


std::unique_ptr<UIViewClass> UI_Game_Options_View(UIGameOptionsPresenterClass & presenter);

void UI_Game_Options_State(UIGameOptionsState & state);

UIGameOptionsChoice UI_Game_Options_Dialog(void);
