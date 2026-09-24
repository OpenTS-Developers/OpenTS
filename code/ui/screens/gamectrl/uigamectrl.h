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


class UIGameControlsServiceClass
{
	public:
		virtual ~UIGameControlsServiceClass(void) = default;
		virtual void Set_Game_Speed(int speed) = 0;
		virtual void Set_Scroll_Rate(int rate) = 0;
		virtual void Set_Detail_Level(int level) = 0;
		virtual void Set_Cameo_Text(bool on) = 0;
		virtual void Set_Action_Lines(bool on) = 0;
		virtual void Set_Tool_Tips(bool on) = 0;
		virtual void Set_Scroll_Coasting(bool on) = 0;
		virtual void Set_Edge_Scroll(bool on) = 0;
		virtual void Set_Difficulty(int difficulty) = 0;
		virtual void Save(void) = 0;
};


struct UIGameControlsState
{
	int Speed = 0;
	int Scroll = 0;
	int Detail = 0;
	int Difficulty = 0;
	bool CameoText = false;
	bool ActionLines = false;
	bool ToolTips = false;
	bool Coasting = false;
	bool EdgeScroll = false;
	bool InGame = false;
	bool HasSpeed = true;
	bool HasDifficulty = true;
	bool SoundEnabled = false;
	std::vector<std::string> SpeedNames;
	std::vector<std::string> ScrollNames;
	std::vector<std::string> DetailNames;
	std::vector<std::string> DifficultyNames;
	std::string SpeedName;
	std::string ScrollName;
	std::string DetailName;
	std::string DifficultyName;
};


class UIGameControlsPresenterClass : public UIPresenterClass
{
	public:
		enum {
			SPEED_LEVELS = 7,
			SCROLL_LEVELS = 7,
			DETAIL_LEVELS = 3,
			DIFFICULTY_LEVELS = 3
		};

		enum NextType {
			NEXT_NONE,
			NEXT_SOUND,
			NEXT_KEYBOARD
		};

		UIGameControlsPresenterClass(UIGameControlsServiceClass & service, UIGameControlsState state);
		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIGameControlsState State;
		NextType Next = NEXT_NONE;

	private:
		void Apply(void);
		void Update_Names(void);

		UIGameControlsServiceClass & Service;
};


std::unique_ptr<UIViewClass> UI_Game_Controls_View(UIGameControlsPresenterClass & presenter);

UIGameControlsServiceClass & UI_Game_Controls_Service(void);
void UI_Game_Controls_State(UIGameControlsState & state);

void UI_Game_Controls_Dialog(void);
