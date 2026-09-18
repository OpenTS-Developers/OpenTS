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
#include <optional>
#include <string>
#include <vector>

class UIViewClass;


struct UICampaignEntry
{
	std::string Description;
	int Campaign = 0;
};


struct UICampaignState
{
	std::vector<UICampaignEntry> Entries;
	std::vector<std::string> DifficultyNames;
	int Selected = 0;
	int Difficulty = 0;
	std::string DifficultyName;
	int Top = -1;
};


class UICampaignPresenterClass : public UIPresenterClass
{
	public:
		explicit UICampaignPresenterClass(UICampaignState state);
		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UICampaignState State;
		std::optional<UICampaignEntry> Picked;

	private:
		void Name_Difficulty(void);
};


std::unique_ptr<UIViewClass> UI_Campaign_View(UICampaignPresenterClass & presenter);

void UI_Campaign_State(UICampaignState & state);

std::optional<UICampaignEntry> UI_Campaign_Dialog(void);
