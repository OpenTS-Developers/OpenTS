/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the game controls: the service the presenter drives and the state it
// starts from. The presenter lives in uigamectrl.cpp so that the test harness can drive it
// against a recording service.

#include "ui/screens/gamectrl/uigamectrl.h"

#include "_map.h"
#include "_tooltip.h"
#include "_ui.h"
#include "audio/audioengine.h"
#include "cctooltip.h"
#include "data.h"
#include "gamedlg.h"
#include "globals.h"
#include "goptions.h"
#include "queue.h"
#include "session.h"
#include "techno.h"
#include "ui/uienginehost.h"
#include "ui/uishell.h"
#include "ui/uiview.h"


namespace
{

class UIGameControlsEngineServiceClass : public UIGameControlsServiceClass
{
	public:
		// A network game changes its speed for everyone through an event, never locally.
		virtual void Set_Game_Speed(int speed) override
		{
			if (Options.GameSpeed != speed) {
				if (GameActive == true && Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
					OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::GAMESPEED, speed));
				} else {
					Options.GameSpeed = speed;
				}
			}
		}

		virtual void Set_Scroll_Rate(int rate) override
		{
			Options.ScrollRate = rate;
		}

		virtual void Set_Detail_Level(int level) override
		{
			if (Options.DetailLevel != level) {
				Options.DetailLevel = level;
				Map.Reinit_Cell_Drawers();
			}
		}

		virtual void Set_Cameo_Text(bool on) override
		{
			if (Options.SidebarCameoText != on) {
				Options.SidebarCameoText = on;
				Map.Toggle_Cameo_Text(on);
			}
		}

		virtual void Set_Action_Lines(bool on) override
		{
			Options.ActionLines = on;
			TechnoClass::Set_Action_Lines(Options.ActionLines);
		}

		virtual void Set_Tool_Tips(bool on) override
		{
			Options.ToolTips = on;
			if (ToolTips != NULL && GameActive == true) {
				ToolTips->Activate(Options.ToolTips);
			}
		}

		virtual void Set_Scroll_Coasting(bool on) override
		{
			Options.ScrollMethod = on ? 0 : 1;
		}

		virtual void Set_Edge_Scroll(bool on) override
		{
			Options.AutoScroll = on;
		}

		virtual void Set_Difficulty(int difficulty) override
		{
			Options.Difficulty = difficulty;
		}

		virtual void Save(void) override
		{
			Options.Save_Settings();
		}
};

UIGameControlsEngineServiceClass _Service;


void Fetch_Names(std::vector<std::string> & names, int const * ids, int count)
{
	names.clear();
	for (int index = 0; index < count; index++) {
		names.push_back(Fetch_String(ids[index]));
	}
}

}


UIGameControlsServiceClass & UI_Game_Controls_Service(void)
{
	return(_Service);
}


// An Internet game has no game speed control, and only the frontend has the difficulty.
void UI_Game_Controls_State(UIGameControlsState & state)
{
	state = UIGameControlsState();

	state.Speed = Options.GameSpeed;
	state.Scroll = Options.ScrollRate;
	state.Detail = Options.DetailLevel;
	state.Difficulty = Options.Difficulty;
	state.CameoText = Options.SidebarCameoText;
	state.ActionLines = Options.ActionLines;
	state.ToolTips = Options.ToolTips;
	state.Coasting = (Options.ScrollMethod == 0);
	state.EdgeScroll = Options.AutoScroll;
	state.InGame = GameActive;
	state.HasSpeed = !(GameActive && Session.Type == GAME_INTERNET);
	state.HasDifficulty = !GameActive;
	state.SoundEnabled = AudioEngine.Is_Available();

	Fetch_Names(state.SpeedNames, GameSpeedNames, OptionsClass::MAX_SPEED_SETTING);
	Fetch_Names(state.ScrollNames, GameScrollSpeedNames, OptionsClass::MAX_SCROLL_SETTING);
	Fetch_Names(state.DetailNames, GameDetailLevelNames, OptionsClass::MAX_DETAIL_SETTING);
	Fetch_Names(state.DifficultyNames, GameDifficultyNames, OptionsClass::MAX_DIFFICULTY_SETTING);
}


bool UI_Game_Controls_Dialog(void)
{
	if (UIShell.Legacy_Dialog_Visible()) {
		return(false);
	}

	UIGameControlsState state;
	UI_Game_Controls_State(state);

	UIGameControlsPresenterClass presenter(UI_Game_Controls_Service(), state);
	std::unique_ptr<UIViewClass> view = UI_Game_Controls_View(presenter);

	if (UI_Run_Modal(*view) == UI_RESULT_FAILED_TO_OPEN) {
		return(false);
	}

	if (presenter.Next == UIGameControlsPresenterClass::NEXT_SOUND) {
		SpecialDialog = SDLG_SOUND;
	} else if (presenter.Next == UIGameControlsPresenterClass::NEXT_KEYBOARD) {
		SpecialDialog = SDLG_KEYBOARD;
	}
	return(true);
}
