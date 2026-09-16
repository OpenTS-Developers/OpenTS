/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/savegame/uisavegame.h"

#include "ui/rml/rmlview.h"

#include <utility>


UISaveGamePresenterClass::UISaveGamePresenterClass(UISaveGameState state) :
	State(std::move(state))
{
}


void UISaveGamePresenterClass::Execute(UIIntent const & intent)
{
	bool accept = false;

	if (intent.Name == "select") {
		if (intent.Value >= 0 && intent.Value < (int)State.Entries.size()) {
			State.Selected = intent.Value;

			/*
			**	If the user clicks on the list, see if the there is a new current
			**	item; if so, and if we're in SAVE mode, copy the list item into
			**	the save-game description field.
			*/
			if (State.Mode == UI_SAVE_GAME_SAVE) {
				UISaveGameEntry const & entry = State.Entries[State.Selected];

				/*
				**	Copy the game's description, UNLESS it's the empty slot; if
				**	it is, offer the description the dialog started from.
				*/
				if (entry.Valid) {
					State.Description = entry.Description;
				} else if (!State.Suggested.empty()) {
					State.Description = State.Suggested;
				}
			}
		}
	} else if (intent.Name == "description") {
		State.Description = intent.Text;
		accept = intent.Value != 0;
	} else if (intent.Name == "accept" || intent.Name == "ok") {
		accept = true;
	} else if (intent.Name == "cancel") {
		Result = UI_RESULT_CANCELLED;
	}

	if (accept && State.AcceptEnabled) {
		Accepted = true;
		Result = UI_RESULT_ACCEPTED;
	}
}


void UISaveGamePresenterClass::Refresh(void)
{
}


namespace
{

class UISaveGameViewClass : public UIRmlViewClass
{
	public:
		explicit UISaveGameViewClass(UISaveGamePresenterClass & presenter) :
			UIRmlViewClass(presenter, "savegame.rml", "savegame"),
			Data(presenter)
		{
		}

		virtual void Sync(void) override
		{
			Model.DirtyVariable("selected");
			Model.DirtyVariable("description");
			Model.DirtyVariable("acceptenabled");
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			Rml::StructHandle<UISaveGameEntry> entry = model.RegisterStruct<UISaveGameEntry>();
			if (!entry) {
				return(false);
			}
			entry.RegisterMember("description", &UISaveGameEntry::Description);
			entry.RegisterMember("date", &UISaveGameEntry::Date);
			entry.RegisterMember("time", &UISaveGameEntry::Time);

			UISaveGameState & state = Data.State;
			return(model.RegisterArray<std::vector<UISaveGameEntry>>()
				&& model.Bind("entries", &state.Entries)
				&& model.Bind("selected", &state.Selected)
				&& model.Bind("description", &state.Description)
				&& model.Bind("title", &state.Title)
				&& model.Bind("acceptcaption", &state.AcceptCaption)
				&& model.Bind("acceptenabled", &state.AcceptEnabled)
				&& model.BindFunc("mode", [&state](Rml::Variant & out) { out = (int)state.Mode; }));
		}

	private:
		UISaveGamePresenterClass & Data;
};

}


std::unique_ptr<UIViewClass> UI_Save_Game_View(UISaveGamePresenterClass & presenter)
{
	return(std::make_unique<UISaveGameViewClass>(presenter));
}
