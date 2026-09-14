/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/campaign/uicampaign.h"

#include "ui/rml/rmlview.h"

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

#include <string>
#include <utility>


UICampaignPresenterClass::UICampaignPresenterClass(UICampaignState state) :
	State(std::move(state))
{
	Name_Difficulty();
}


// The Win32 dialog left the reading at whatever its template held until the bar first moved,
// so it opened naming a difficulty that was not the one set. The reading follows the bar here.
void UICampaignPresenterClass::Name_Difficulty(void)
{
	if (State.Difficulty >= 0 && State.Difficulty < (int)State.DifficultyNames.size()) {
		State.DifficultyName = State.DifficultyNames[State.Difficulty];
	} else {
		State.DifficultyName.clear();
	}
}


void UICampaignPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "select") {
		if (intent.Value >= 0 && intent.Value < (int)State.Entries.size()) {
			State.Selected = intent.Value;
		}
	} else if (intent.Name == "difficulty") {
		State.Difficulty = intent.Value;
		Name_Difficulty();
	} else if (intent.Name == "ok") {
		if (State.Selected >= 0 && State.Selected < (int)State.Entries.size()) {
			Picked = State.Entries[State.Selected];
		}
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "cancel") {
		Result = UI_RESULT_CANCELLED;
	}
}


void UICampaignPresenterClass::Refresh(void)
{
}


namespace
{

class UICampaignViewClass : public UIRmlViewClass
{
	public:
		explicit UICampaignViewClass(UICampaignPresenterClass & presenter) :
			UIRmlViewClass(presenter, "campaign.rml", "campaign"),
			Data(presenter)
		{
		}

		// The chooser sits where the main menu's buttons were, which takes the top the engine
		// measured over the centred position the style sheet gives it.
		virtual void Loaded(void) override
		{
			Rml::Element * dialog = Document()->GetElementById("reveal");
			if (dialog == nullptr || Data.State.Top < 0) {
				return;
			}

			dialog->SetProperty("top", std::to_string(Data.State.Top) + "dp");
			dialog->SetProperty("margin-top", "0dp");
		}

		// The campaign list is settled before the screen opens; the row and the bar are not.
		virtual void Sync(void) override
		{
			Model.DirtyVariable("selected");
			Model.DirtyVariable("difficulty");
			Model.DirtyVariable("difficultyname");
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			Rml::StructHandle<UICampaignEntry> entry = model.RegisterStruct<UICampaignEntry>();
			if (!entry) {
				return(false);
			}
			entry.RegisterMember("description", &UICampaignEntry::Description);
			entry.RegisterMember("campaign", &UICampaignEntry::Campaign);

			UICampaignState & state = Data.State;
			return(model.RegisterArray<std::vector<UICampaignEntry>>()
				&& model.Bind("entries", &state.Entries)
				&& model.Bind("selected", &state.Selected)
				&& model.Bind("difficulty", &state.Difficulty)
				&& model.Bind("difficultyname", &state.DifficultyName));
		}

	private:
		UICampaignPresenterClass & Data;
};

}


std::unique_ptr<UIViewClass> UI_Campaign_View(UICampaignPresenterClass & presenter)
{
	return(std::make_unique<UICampaignViewClass>(presenter));
}
