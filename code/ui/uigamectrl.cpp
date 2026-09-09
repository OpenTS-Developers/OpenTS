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


UIGameControlsPresenterClass::UIGameControlsPresenterClass(UIGameControlsServiceClass & service, UIGameControlsState state) :
	State(std::move(state)),
	Service(service)
{
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
		Apply();
		Next = NEXT_SOUND;
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "keyboard") {
		Apply();
		Next = NEXT_KEYBOARD;
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "cancel") {
		Result = UI_RESULT_CANCELLED;
	}
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
