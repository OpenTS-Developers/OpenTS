/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "_surface.h"
#include "_ui.h"
#include "campaign.h"
#include "data.h"
#include "gamedlg.h"
#include "globals.h"
#include "init.h"
#include "options.h"
#include "surface.h"
#include "ui/screens/campaign/uicampaign.h"
#include "ui/uienginehost.h"
#include "ui/uishell.h"
#include "ui/uiview.h"
#include "vector.h"

#include <utility>


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


std::optional<UICampaignEntry> UI_Campaign_Dialog(void)
{
	UICampaignState state;
	UI_Campaign_State(state);

	UICampaignPresenterClass presenter(std::move(state));
	std::unique_ptr<UIViewClass> view = UI_Campaign_View(presenter);

	if (UI_Run_Modal(*view) != UI_RESULT_ACCEPTED) {
		return(std::nullopt);
	}

	Options.Difficulty = presenter.State.Difficulty;
	return(presenter.Picked);
}
