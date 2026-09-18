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


UIGameOptionsPresenterClass::UIGameOptionsPresenterClass(UIGameOptionsServiceClass & service, UIGameOptionsState state) :
	State(std::move(state)),
	Service(service)
{
	Update_Name();
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
			if (!State.Solo) {
				Choice = UI_GAME_OPTIONS_SAVE;
				Result = UI_RESULT_ACCEPTED;
			} else {
				Service.Save();
				Service.Read(State);
			}
		}
	} else if (intent.Name == "load") {
		if (State.LoadEnabled) {
			if (!State.Solo || Service.Load()) {
				Choice = UI_GAME_OPTIONS_LOAD;
				Result = UI_RESULT_ACCEPTED;
			}
		}
	} else if (intent.Name == "delete") {
		if (State.DeleteEnabled) {
			Service.Delete();
			Service.Read(State);
		}
	} else if (intent.Name == "abort") {
		Choice = UI_GAME_OPTIONS_ABORT;
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "speed") {
		State.Speed = intent.Value;
		SpeedChanged = true;
		Update_Name();
	} else if (intent.Name == "resume" || intent.Name == "ok" || intent.Name == "cancel") {
		Choice = UI_GAME_OPTIONS_RESUME;
		Result = UI_RESULT_ACCEPTED;
	}
}


void UIGameOptionsPresenterClass::Refresh(void)
{
}


void UIGameOptionsPresenterClass::Update_Name(void)
{
	State.SpeedName = (State.Speed >= 0 && State.Speed < (int)State.SpeedNames.size())
		? State.SpeedNames[State.Speed] : std::string();
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

		virtual void Sync(void) override
		{
			Model.DirtyAllVariables();
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			UIGameOptionsState & state = Data.State;
			return(model.Bind("solo", &state.Solo)
				&& model.Bind("internet", &state.Internet)
				&& model.Bind("briefingenabled", &state.BriefingEnabled)
				&& model.Bind("loadenabled", &state.LoadEnabled)
				&& model.Bind("saveenabled", &state.SaveEnabled)
				&& model.Bind("deleteenabled", &state.DeleteEnabled)
				&& model.Bind("speed", &state.Speed)
				&& model.Bind("speedname", &state.SpeedName)
				&& model.Bind("connection", &state.Connection)
				&& model.Bind("connectionlowest", &state.ConnectionLowest)
				&& model.Bind("connectionhighest", &state.ConnectionHighest)
				&& model.Bind("connectionname", &state.ConnectionName));
		}

	private:
		UIGameOptionsPresenterClass & Data;
};

}


std::unique_ptr<UIViewClass> UI_Game_Options_View(UIGameOptionsPresenterClass & presenter)
{
	return(std::make_unique<UIGameOptionsViewClass>(presenter));
}
