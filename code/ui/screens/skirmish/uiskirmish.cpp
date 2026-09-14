/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/skirmish/uiskirmish.h"

#include "ui/rml/rmlsurface.h"
#include "ui/rml/rmlview.h"

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

#include <utility>


UISkirmishPresenterClass::UISkirmishPresenterClass(UISkirmishState state) :
	State(std::move(state))
{
}


// A reading is held to the bounds the rules gave it, because a document can be told to ask
// for anything and the game reads these straight into the session.
static int Clamp(int value, int low, int high)
{
	if (value < low) {
		return(low);
	}
	return(value > high ? high : value);
}


void UISkirmishPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "handle") {
		State.Handle = intent.Text;
	} else if (intent.Name == "side") {
		if (intent.Value >= 0 && intent.Value < (int)State.Sides.size()) {
			State.Side = intent.Value;
		}
	} else if (intent.Name == "colour") {
		if (intent.Value >= 0 && intent.Value < (int)State.Colours.size()) {
			State.Colour = intent.Value;
		}
	} else if (intent.Name == "bases") {
		State.Bases = intent.Value != 0;

		// The Win32 dialog clears a short game when bases go, and sets bases when one is
		// asked for, because a short game is decided by what a player has left standing.
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
	} else if (intent.Name == "units") {
		State.UnitCount = Clamp(intent.Value, State.UnitCountMin, State.UnitCountMax);
	} else if (intent.Name == "credits") {
		State.Credits = Clamp(intent.Value, State.CreditsMin, State.CreditsMax);
	} else if (intent.Name == "tech") {
		State.TechLevel = Clamp(intent.Value, 1, State.TechLevelMax);
	} else if (intent.Name == "ailevel") {
		State.AILevel = Clamp(intent.Value, 0, 2);
	} else if (intent.Name == "aiplayers") {
		State.AIPlayers = Clamp(intent.Value, 1, State.AIPlayersMax);
	} else if (intent.Name == "speed") {
		State.GameSpeed = Clamp(intent.Value, 0, 6);
	} else if (intent.Name == "map") {
		Choice = UI_SKIRMISH_PICK_MAP;
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "ok") {
		Choice = UI_SKIRMISH_START;
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "cancel") {
		Choice = UI_SKIRMISH_CANCEL;
		Result = UI_RESULT_CANCELLED;
	}
}


void UISkirmishPresenterClass::Refresh(void)
{
}


namespace
{

class UISkirmishViewClass : public UIRmlViewClass
{
	public:
		explicit UISkirmishViewClass(UISkirmishPresenterClass & presenter) :
			UIRmlViewClass(presenter, "skirmish.rml", "skirmish"),
			Data(presenter),
			Shown(-1)
		{
		}

		// Coming back from the map dialog, the setup is shown whole rather than opened.
		virtual void Loaded(void) override
		{
			if (!Data.State.Reveal && Document() != nullptr) {
				Document()->SetAttribute("reveal", Rml::String("none"));
			}
		}

		// The lists, the bounds and the map name are settled before the screen opens; the
		// rest is what the player can move. The preview is bytes, so it is handed over
		// rather than pushed through the model.
		virtual void Sync(void) override
		{
			Model.DirtyVariable("handle");
			Model.DirtyVariable("side");
			Model.DirtyVariable("colour");
			Model.DirtyVariable("bases");
			Model.DirtyVariable("crates");
			Model.DirtyVariable("fog");
			Model.DirtyVariable("bridges");
			Model.DirtyVariable("redeploy");
			Model.DirtyVariable("shortgame");
			Model.DirtyVariable("engineer");
			Model.DirtyVariable("units");
			Model.DirtyVariable("credits");
			Model.DirtyVariable("tech");
			Model.DirtyVariable("ailevel");
			Model.DirtyVariable("aiplayers");
			Model.DirtyVariable("speed");

			Show_Preview();
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			Rml::StructHandle<UISkirmishOption> option = model.RegisterStruct<UISkirmishOption>();
			if (!option) {
				return(false);
			}
			option.RegisterMember("label", &UISkirmishOption::Label);
			option.RegisterMember("value", &UISkirmishOption::Value);
			option.RegisterMember("colour", &UISkirmishOption::Colour);

			UISkirmishState & state = Data.State;
			return(model.RegisterArray<std::vector<UISkirmishOption>>()
				&& model.Bind("handle", &state.Handle)
				&& model.Bind("sides", &state.Sides)
				&& model.Bind("side", &state.Side)
				&& model.Bind("colours", &state.Colours)
				&& model.Bind("colour", &state.Colour)
				&& model.Bind("mapname", &state.MapName)
				&& model.Bind("bases", &state.Bases)
				&& model.Bind("crates", &state.Crates)
				&& model.Bind("fog", &state.Fog)
				&& model.Bind("bridges", &state.Bridges)
				&& model.Bind("redeploy", &state.Redeploy)
				&& model.Bind("shortgame", &state.ShortGame)
				&& model.Bind("engineer", &state.MultiEngineer)
				&& model.Bind("units", &state.UnitCount)
				&& model.Bind("unitsmin", &state.UnitCountMin)
				&& model.Bind("unitsmax", &state.UnitCountMax)
				&& model.Bind("credits", &state.Credits)
				&& model.Bind("creditsmin", &state.CreditsMin)
				&& model.Bind("creditsmax", &state.CreditsMax)
				&& model.Bind("creditsstep", &state.CreditsStep)
				&& model.Bind("tech", &state.TechLevel)
				&& model.Bind("techmax", &state.TechLevelMax)
				&& model.Bind("ailevel", &state.AILevel)
				&& model.Bind("aiplayers", &state.AIPlayers)
				&& model.Bind("aiplayersmax", &state.AIPlayersMax)
				&& model.Bind("speed", &state.GameSpeed));
		}

	private:
		void Show_Preview(void)
		{
			UIMapPreviewImage & preview = Data.State.Preview;
			if (preview.Generation == Shown || Document() == nullptr) {
				return;
			}

			UIRmlSurfaceElementClass * surface = rmlui_dynamic_cast<UIRmlSurfaceElementClass *>(Document()->GetElementById("preview"));
			if (surface == nullptr) {
				return;
			}

			Shown = preview.Generation;
			surface->Set_Image(preview.Width, preview.Height, preview.Pixels);
		}

		UISkirmishPresenterClass & Data;
		int Shown;
};

}


std::unique_ptr<UIViewClass> UI_Skirmish_View(UISkirmishPresenterClass & presenter)
{
	return(std::make_unique<UISkirmishViewClass>(presenter));
}
