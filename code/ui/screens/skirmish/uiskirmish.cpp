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
#include <cstdlib>
#include <utility>


UISkirmishPresenterClass::UISkirmishPresenterClass(UISkirmishServiceClass & service, UISkirmishState state) :
	State(std::move(state)),
	Service(service)
{
}


static int Clamp(int value, int low, int high)
{
	if (value < low) {
		return(low);
	}
	return(value > high ? high : value);
}


// Keeps one computer slot per AI player. New slots start at the session difficulty.
static void Resize_AI(UISkirmishState & state)
{
	if ((int)state.AI.size() < state.AIPlayers) {
		while ((int)state.AI.size() < state.AIPlayers) {
			UISkirmishAISlot slot;
			slot.Level = Clamp(state.AILevel, 0, 2);
			state.AI.push_back(slot);
		}
	} else if ((int)state.AI.size() > state.AIPlayers) {
		state.AI.resize(state.AIPlayers);
	}
}


void UISkirmishPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "handle") {
		State.Handle = intent.Text;
	} else if (intent.Name == "side") {
		if (intent.Value >= 0 && intent.Value < (int)State.Sides.size()) {
			State.Side = intent.Value;
		}
	} else if (intent.Name == "color") {
		if (intent.Value >= 0 && intent.Value < (int)State.Colors.size()) {
			State.Color = intent.Value;
		}
	} else if (intent.Name == "team") {
		State.Team = Clamp(intent.Value, 0, 4);
	} else if (intent.Name == "aiadd") {
		State.AIPlayers = Clamp(State.AIPlayers + 1, 1, State.AIPlayersMax);
		Resize_AI(State);
	} else if (intent.Name == "aisub") {
		State.AIPlayers = Clamp(State.AIPlayers - 1, 1, State.AIPlayersMax);
		Resize_AI(State);
	} else if (intent.Name == "aiside" || intent.Name == "aicolor" || intent.Name == "aidiff" || intent.Name == "aiteam") {
		int slot = intent.Value;
		if (slot >= 0 && slot < (int)State.AI.size()) {
			int value = std::atoi(intent.Text.c_str());
			if (intent.Name == "aiside") {
				State.AI[slot].Side = value;
			} else if (intent.Name == "aicolor") {
				State.AI[slot].Color = Clamp(value, -1, (int)State.Colors.size() - 1);
			} else if (intent.Name == "aidiff") {
				State.AI[slot].Level = Clamp(value, 0, 2);
			} else {
				State.AI[slot].Team = Clamp(value, 0, 4);
			}
		}
	} else if (intent.Name == "map") {
		Service.Pick_Map(State);
	} else if (intent.Name == "options") {
		Service.Configure_Options(State);
	} else if (intent.Name == "ok") {
		if (Service.Can_Start(State)) {
			Choice = UI_SKIRMISH_START;
			Result = UI_RESULT_ACCEPTED;
		}
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

		virtual void Sync(void) override
		{
			Model.DirtyVariable("handle");
			Model.DirtyVariable("side");
			Model.DirtyVariable("color");
			Model.DirtyVariable("team");
			Model.DirtyVariable("mapname");
			Model.DirtyVariable("aiplayers");
			Model.DirtyVariable("ai");

			Show_Preview();
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			Rml::StructHandle<UISkirmishOption> option = model.RegisterStruct<UISkirmishOption>();
			Rml::StructHandle<UISkirmishAISlot> ai = model.RegisterStruct<UISkirmishAISlot>();
			if (!option || !ai) {
				return(false);
			}
			option.RegisterMember("label", &UISkirmishOption::Label);
			option.RegisterMember("value", &UISkirmishOption::Value);
			option.RegisterMember("color", &UISkirmishOption::Color);

			ai.RegisterMember("side", &UISkirmishAISlot::Side);
			ai.RegisterMember("color", &UISkirmishAISlot::Color);
			ai.RegisterMember("level", &UISkirmishAISlot::Level);
			ai.RegisterMember("team", &UISkirmishAISlot::Team);

			UISkirmishState & state = Data.State;
			return(model.RegisterArray<std::vector<UISkirmishOption>>()
				&& model.RegisterArray<std::vector<UISkirmishAISlot>>()
				&& model.Bind("handle", &state.Handle)
				&& model.Bind("sides", &state.Sides)
				&& model.Bind("side", &state.Side)
				&& model.Bind("colors", &state.Colors)
				&& model.Bind("color", &state.Color)
				&& model.Bind("team", &state.Team)
				&& model.Bind("mapname", &state.MapName)
				&& model.Bind("aiplayers", &state.AIPlayers)
				&& model.Bind("ai", &state.AI));
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
