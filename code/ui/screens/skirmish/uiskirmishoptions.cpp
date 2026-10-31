/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/skirmish/uiskirmishoptions.h"

#include "ui/rml/rmlview.h"

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>


UISkirmishOptionsPresenterClass::UISkirmishOptionsPresenterClass(UISkirmishState & state) :
	State(state)
{
}


static int Clamp(int value, int low, int high)
{
	if (value < low) {
		return(low);
	}
	return(value > high ? high : value);
}


void UISkirmishOptionsPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "bases") {
		State.Bases = intent.Value != 0;
		if (!State.Bases) {
			State.ShortGame = false;
		}
	} else if (intent.Name == "short") {
		State.ShortGame = intent.Value != 0;
		if (State.ShortGame) {
			State.Bases = true;
		}
	} else if (intent.Name == "crates") {
		State.Crates = intent.Value != 0;
	} else if (intent.Name == "fog") {
		State.Fog = intent.Value != 0;
	} else if (intent.Name == "bridges") {
		State.Bridges = intent.Value != 0;
	} else if (intent.Name == "redeploy") {
		State.Redeploy = intent.Value != 0;
	} else if (intent.Name == "engineer") {
		State.MultiEngineer = intent.Value != 0;
	} else if (intent.Name == "paranoid") {
		State.Paranoid = intent.Value != 0;
	} else if (intent.Name == "units") {
		State.UnitCount = Clamp(intent.Value, State.UnitCountMin, State.UnitCountMax);
	} else if (intent.Name == "credits") {
		State.Credits = Clamp(intent.Value, State.CreditsMin, State.CreditsMax);
	} else if (intent.Name == "tech") {
		State.TechLevel = Clamp(intent.Value, 1, State.TechLevelMax);
	} else if (intent.Name == "speed") {
		State.GameSpeed = Clamp(intent.Value, 0, 6);
	} else if (intent.Name == "ok") {
		Accepted = true;
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "cancel") {
		Result = UI_RESULT_CANCELLED;
	}
}


void UISkirmishOptionsPresenterClass::Refresh(void)
{
}


namespace
{

class UISkirmishOptionsViewClass : public UIRmlViewClass
{
	public:
		explicit UISkirmishOptionsViewClass(UISkirmishOptionsPresenterClass & presenter) :
			UIRmlViewClass(presenter, "skirmishoptions.rml", "skirmishoptions"),
			Data(presenter)
		{
		}

		virtual void Sync(void) override
		{
			Model.DirtyVariable("bases");
			Model.DirtyVariable("crates");
			Model.DirtyVariable("fog");
			Model.DirtyVariable("bridges");
			Model.DirtyVariable("redeploy");
			Model.DirtyVariable("shortgame");
			Model.DirtyVariable("engineer");
			Model.DirtyVariable("paranoid");
			Model.DirtyVariable("units");
			Model.DirtyVariable("credits");
			Model.DirtyVariable("tech");
			Model.DirtyVariable("speed");
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			UISkirmishState & state = Data.State;
			return(model.Bind("bases", &state.Bases)
				&& model.Bind("crates", &state.Crates)
				&& model.Bind("fog", &state.Fog)
				&& model.Bind("bridges", &state.Bridges)
				&& model.Bind("redeploy", &state.Redeploy)
				&& model.Bind("shortgame", &state.ShortGame)
				&& model.Bind("engineer", &state.MultiEngineer)
				&& model.Bind("paranoid", &state.Paranoid)
				&& model.Bind("units", &state.UnitCount)
				&& model.Bind("unitsmin", &state.UnitCountMin)
				&& model.Bind("unitsmax", &state.UnitCountMax)
				&& model.Bind("credits", &state.Credits)
				&& model.Bind("creditsmin", &state.CreditsMin)
				&& model.Bind("creditsmax", &state.CreditsMax)
				&& model.Bind("creditsstep", &state.CreditsStep)
				&& model.Bind("tech", &state.TechLevel)
				&& model.Bind("techmax", &state.TechLevelMax)
				&& model.Bind("speed", &state.GameSpeed));
		}

	private:
		UISkirmishOptionsPresenterClass & Data;
};

}


std::unique_ptr<UIViewClass> UI_Skirmish_Options_View(UISkirmishOptionsPresenterClass & presenter)
{
	return(std::make_unique<UISkirmishOptionsViewClass>(presenter));
}
