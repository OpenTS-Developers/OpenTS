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


// One row of the campaign list: what the row reads and the campaign it stands for.
struct UICampaignEntry
{
	std::string Description;
	int Campaign = 0;
};


// What the chooser shows: the campaigns worth offering, the row it opens on, the
// difficulty the settings hold with the names to print it by, and the top edge the
// chooser sits at in the frame, or -1 to sit in the middle.
struct UICampaignState
{
	std::vector<UICampaignEntry> Entries;
	std::vector<std::string> DifficultyNames;
	int Selected = 0;
	int Difficulty = 0;
	std::string DifficultyName;
	int Top = -1;
};


// Holds the picked row and the difficulty until the player accepts. Accepting reports both;
// cancelling reports neither, and the settings keep the difficulty they had.
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


// The RmlUi view over a campaign presenter, bound to campaign.rml. The presenter must
// outlive it.
std::unique_ptr<UIViewClass> UI_Campaign_View(UICampaignPresenterClass & presenter);

// The campaigns on offer and the difficulty the settings hold.
void UI_Campaign_State(UICampaignState & state);

// Runs the campaign chooser as an RmlUi screen. False means it could not run as one and the
// caller should open its Win32 dialog; otherwise picked carries the campaign, or nothing when
// the player backed out.
bool UI_Campaign_Dialog(std::optional<UICampaignEntry> & picked);
