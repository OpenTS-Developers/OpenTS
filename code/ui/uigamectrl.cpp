/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uigamectrl.h"

#include "ui/uirmlview.h"

#include <utility>


static int Clamp_Level(int level, int count)
{
	if (level < 0) {
		return(0);
	}
	if (level > count - 1) {
		return(count - 1);
	}
	return(level);
}


static std::string Name_Of(std::vector<std::string> const & names, int level)
{
	if (level < 0 || level >= (int)names.size()) {
		return(std::string());
	}
	return(names[level]);
}


UIGameControlsPresenterClass::UIGameControlsPresenterClass(UIGameControlsServiceClass & service, UIGameControlsState state) :
	State(std::move(state)),
	Service(service)
{
	Update_Names();
}


void UIGameControlsPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "speed") {
		State.Speed = Clamp_Level(intent.Value, SPEED_LEVELS);
	} else if (intent.Name == "scroll") {
		State.Scroll = Clamp_Level(intent.Value, SCROLL_LEVELS);
	} else if (intent.Name == "detail") {
		State.Detail = Clamp_Level(intent.Value, DETAIL_LEVELS);
	} else if (intent.Name == "difficulty") {
		State.Difficulty = Clamp_Level(intent.Value, DIFFICULTY_LEVELS);
	} else if (intent.Name == "cameo") {
		State.CameoText = (intent.Value != 0);
	} else if (intent.Name == "lines") {
		State.ActionLines = (intent.Value != 0);
	} else if (intent.Name == "tooltips") {
		State.ToolTips = (intent.Value != 0);
	} else if (intent.Name == "coasting") {
		State.Coasting = (intent.Value != 0);
	} else if (intent.Name == "edge") {
		State.EdgeScroll = (intent.Value != 0);
	} else if (intent.Name == "ok") {
		Apply();
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "sound") {
		if (State.SoundEnabled) {
			Apply();
			Next = NEXT_SOUND;
			Result = UI_RESULT_ACCEPTED;
		}
	} else if (intent.Name == "keyboard") {
		Apply();
		Next = NEXT_KEYBOARD;
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "cancel") {
		Result = UI_RESULT_CANCELLED;
	}

	Update_Names();
}


void UIGameControlsPresenterClass::Refresh(void)
{
}


// The order is the one the dialog's accept path has always used.
void UIGameControlsPresenterClass::Apply(void)
{
	if (State.HasSpeed) {
		Service.Set_Game_Speed(State.Speed);
	}
	Service.Set_Scroll_Rate(State.Scroll);
	Service.Set_Detail_Level(State.Detail);
	Service.Set_Cameo_Text(State.CameoText);
	Service.Set_Action_Lines(State.ActionLines);
	Service.Set_Tool_Tips(State.ToolTips);
	Service.Set_Scroll_Coasting(State.Coasting);
	Service.Set_Edge_Scroll(State.EdgeScroll);
	if (State.HasDifficulty) {
		Service.Set_Difficulty(State.Difficulty);
	}
	Service.Save();
}


void UIGameControlsPresenterClass::Update_Names(void)
{
	State.SpeedName = Name_Of(State.SpeedNames, State.Speed);
	State.ScrollName = Name_Of(State.ScrollNames, State.Scroll);
	State.DetailName = Name_Of(State.DetailNames, State.Detail);
	State.DifficultyName = Name_Of(State.DifficultyNames, State.Difficulty);
}


namespace
{

class UIGameControlsViewClass : public UIRmlViewClass
{
	public:
		explicit UIGameControlsViewClass(UIGameControlsPresenterClass & presenter) :
			UIRmlViewClass(presenter, "gamectrl.rml", "gamectrl"),
			Data(presenter)
		{
		}

		// The model is small, so every field is re-read after each drain.
		virtual void Sync(void) override
		{
			Model.DirtyAllVariables();
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			UIGameControlsState & state = Data.State;
			return(model.Bind("speed", &state.Speed)
				&& model.Bind("scroll", &state.Scroll)
				&& model.Bind("detail", &state.Detail)
				&& model.Bind("difficulty", &state.Difficulty)
				&& model.Bind("cameo", &state.CameoText)
				&& model.Bind("lines", &state.ActionLines)
				&& model.Bind("tooltips", &state.ToolTips)
				&& model.Bind("coasting", &state.Coasting)
				&& model.Bind("edge", &state.EdgeScroll)
				&& model.Bind("ingame", &state.InGame)
				&& model.Bind("hasspeed", &state.HasSpeed)
				&& model.Bind("hasdifficulty", &state.HasDifficulty)
				&& model.Bind("soundenabled", &state.SoundEnabled)
				&& model.Bind("speedname", &state.SpeedName)
				&& model.Bind("scrollname", &state.ScrollName)
				&& model.Bind("detailname", &state.DetailName)
				&& model.Bind("difficultyname", &state.DifficultyName));
		}

	private:
		UIGameControlsPresenterClass & Data;
};

}


std::unique_ptr<UIRmlViewClass> UI_Game_Controls_View(UIGameControlsPresenterClass & presenter)
{
	return(std::make_unique<UIGameControlsViewClass>(presenter));
}
