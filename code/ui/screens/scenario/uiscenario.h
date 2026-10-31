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


enum UIScenarioChoice
{
	UI_SCENARIO_ACCEPT,
	UI_SCENARIO_CANCEL,
};


struct UIScenarioState;


class UIScenarioServiceClass
{
	public:
		virtual ~UIScenarioServiceClass(void) = default;
		virtual void Preview(int index, UIMapPreviewImage & image) = 0;
		virtual void Read(UIScenarioState & state) = 0;
		virtual int Random(void) = 0;
};


struct UIScenarioEntry
{
	int Index = 0;
	std::string Label;
	int Players = 0;
	std::string Theater;
	int Width = 0;
	int Height = 0;
	bool Official = true;
	std::string Meta;
};


struct UIScenarioOption
{
	std::string Label;
	int Value = 0;
};


struct UIScenarioState
{
	std::vector<UIScenarioEntry> All;
	std::vector<UIScenarioEntry> Entries;
	int Selected = -1;
	std::string Query;
	std::string Meta;

	std::vector<UIScenarioOption> Players;
	int Player = 0;
	std::vector<std::string> TheaterNames;
	std::vector<UIScenarioOption> Theaters;
	int Theater = 0;
	std::vector<UIScenarioOption> Sources;
	int Source = 0;

	UIMapPreviewImage Preview;
};


class UIScenarioPresenterClass : public UIPresenterClass
{
	public:
		UIScenarioPresenterClass(UIScenarioServiceClass & service, UIScenarioState state);
		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIScenarioState State;
		UIScenarioChoice Choice = UI_SCENARIO_CANCEL;

	private:
		void Select(int index);
		void Apply_Filter(void);

		UIScenarioServiceClass & Service;
};


std::unique_ptr<UIViewClass> UI_Scenario_View(UIScenarioPresenterClass & presenter);

UIScenarioServiceClass & UI_Scenario_Service(void);
void UI_Scenario_State(UIScenarioState & state);

bool UI_Scenario_Dialog(void);
