/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/gameopt/uigameopt.h"

#include "ui/rml/rmlview.h"

#include <RmlUi/Core/ElementDocument.h>

#include <utility>


UIGameOptionsPresenterClass::UIGameOptionsPresenterClass(UIGameOptionsState state) :
	State(std::move(state))
{
}


void UIGameOptionsPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "controls") {
		Choice = UI_GAME_OPTIONS_CONTROLS;
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "briefing") {
		if (State.BriefingEnabled) {
			Choice = UI_GAME_OPTIONS_BRIEFING;
			Result = UI_RESULT_ACCEPTED;
		}
	} else if (intent.Name == "save") {
		if (State.SaveEnabled) {
			Choice = UI_GAME_OPTIONS_SAVE;
			Result = UI_RESULT_ACCEPTED;
		}
	} else if (intent.Name == "load") {
		if (State.LoadEnabled) {
			Choice = UI_GAME_OPTIONS_LOAD;
			Result = UI_RESULT_ACCEPTED;
		}
	} else if (intent.Name == "delete") {
		if (State.DeleteEnabled) {
			Choice = UI_GAME_OPTIONS_DELETE;
			Result = UI_RESULT_ACCEPTED;
		}
	} else if (intent.Name == "abort") {
		Choice = UI_GAME_OPTIONS_ABORT;
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "resume" || intent.Name == "ok" || intent.Name == "cancel") {
		Choice = UI_GAME_OPTIONS_RESUME;
		Result = UI_RESULT_ACCEPTED;
	}
}


void UIGameOptionsPresenterClass::Refresh(void)
{
}


namespace
{

class UIGameOptionsViewClass : public UIRmlViewClass
{
	public:
		explicit UIGameOptionsViewClass(UIGameOptionsPresenterClass & presenter) :
			UIRmlViewClass(presenter, "gameopt.rml", "gameopt"),
			Data(presenter)
		{
		}

		// Coming back from a save or a delete, the menu is shown whole rather than opened.
		virtual void Loaded(void) override
		{
			if (!Data.State.Reveal && Document() != nullptr) {
				Document()->SetAttribute("reveal", Rml::String("none"));
			}
		}

		virtual void Sync(void) override
		{
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			UIGameOptionsState & state = Data.State;
			return(model.Bind("solo", &state.Solo)
				&& model.Bind("briefingenabled", &state.BriefingEnabled)
				&& model.Bind("loadenabled", &state.LoadEnabled)
				&& model.Bind("saveenabled", &state.SaveEnabled)
				&& model.Bind("deleteenabled", &state.DeleteEnabled));
		}

	private:
		UIGameOptionsPresenterClass & Data;
};

}


std::unique_ptr<UIViewClass> UI_Game_Options_View(UIGameOptionsPresenterClass & presenter)
{
	return(std::make_unique<UIGameOptionsViewClass>(presenter));
}
