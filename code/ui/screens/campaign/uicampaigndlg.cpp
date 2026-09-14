/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the campaign chooser: the campaigns it offers and the entry
// Choose_Campaign calls ahead of its Win32 dialog. The presenter and view live in
// uicampaign.cpp so that the test harness can drive them without the engine.

#include "ui/screens/campaign/uicampaign.h"

#include "_surface.h"
#include "_ui.h"
#include "campaign.h"
#include "data.h"
#include "gamedlg.h"
#include "globals.h"
#include "init.h"
#include "options.h"
#include "surface.h"
#include "ui/uienginehost.h"
#include "ui/uishell.h"
#include "ui/uiview.h"
#include "vector.h"

#include <utility>


// The chooser sits where the main menu's buttons were: a 400 pixel layout centred in the
// frame, with the buttons 147 pixels down it.
void UI_Campaign_State(UICampaignState & state)
{
	state = UICampaignState();

	for (int index = 0; index < Campaigns.Count(); index++) {
		CampaignClass * campaign = Campaigns[index];
		if (campaign == NULL || !Campaign_Available(campaign)) {
			continue;
		}

		UICampaignEntry entry;
		entry.Description = campaign->Description;
		entry.Campaign = index;
		state.Entries.push_back(entry);
	}

	for (int index = 0; index < OptionsClass::MAX_DIFFICULTY_SETTING; index++) {
		state.DifficultyNames.push_back(Fetch_String(GameDifficultyNames[index]));
	}

	state.Difficulty = Options.Difficulty;

	if (HiddenSurface != NULL) {
		state.Top = (HiddenSurface->Get_Height() - 400) / 2 + 147;
	}
}


bool UI_Campaign_Dialog(std::optional<UICampaignEntry> & picked)
{
	picked.reset();

	if (UIShell.Legacy_Dialog_Visible()) {
		return(false);
	}

	UICampaignState state;
	UI_Campaign_State(state);

	UICampaignPresenterClass presenter(std::move(state));
	std::unique_ptr<UIViewClass> view = UI_Campaign_View(presenter);

	UIResult result = UI_Run_Modal(*view);
	if (result == UI_RESULT_FAILED_TO_OPEN) {
		return(false);
	}

	if (result == UI_RESULT_ACCEPTED) {
		Options.Difficulty = presenter.State.Difficulty;
		picked = presenter.Picked;
	}

	return(true);
}
