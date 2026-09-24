/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "_rules.h"
#include "_ui.h"
#include "data.h"
#include "globals.h"
#include "goptions.h"
#include "houstype.h"
#include "language/language.h"
#include "mapgen.h"
#include "mplayer.h"
#include "msgbox.h"
#include "netshare.h"
#include "preview.h"
#include "rules.h"
#include "session.h"
#include "ui/screens/skirmish/uiskirmish.h"
#include "ui/uienginehost.h"
#include "ui/uipreview.h"
#include "ui/uishell.h"
#include "ui/uiview.h"
#include "utf8.h"
#include "win.h"

#include <cstdio>
#include <cstring>
#include <utility>


static int const UI_SKIRMISH_MIN_MONEY = 2500;

static int const UI_SKIRMISH_COLORS[] = {
	TXT_GOLD, TXT_RED, TXT_BLUE, TXT_GREEN, TXT_ORANGE, TXT_SKY_BLUE, TXT_PURPLE, TXT_PINK
};


static void Refresh_Preview(void)
{
	int index = Session.Options.ScenarioIndex;

	if (index < 0 || index >= Session.Scenarios.Count()
		|| stricmp(Session.Scenarios[index]->Get_Filename(), RANDOM_MAP_FILE_NAME) != 0) {
		Update_Network_Dialog_Preview();
		return;
	}

	delete MultiplayerMapPreview;
	MultiplayerMapPreview = new MapPreviewClass;
	MultiplayerMapPreview->Read_PCX_Preview("RandMap.img");
	if (MultiplayerMapPreview->Get_Preview_Surface() == NULL) {
		Update_Network_Dialog_Preview();
	}
}


static void Remember_Preferences(UISkirmishState const & state)
{
	UTF8::Copy(Session.Handle, sizeof(Session.Handle), state.Handle.c_str());

	if (state.Side >= 0 && state.Side < (int)state.Sides.size()) {
		Session.House = state.Sides[state.Side].Value;
	}

	Session.ColorIdx = state.Color;
	Session.PrefColor = state.Color;
}


static void Commit(UISkirmishState const & state)
{
	Remember_Preferences(state);

	Session.Options.UnitCount = state.UnitCount;
	BuildLevel = state.TechLevel;
	Session.Options.Credits = state.Credits;
	Session.Options.AIDifficulty = (DiffType)state.AILevel;
	Session.Options.AIPlayers = state.AIPlayers;
	Session.Options.GameSpeed = 6 - state.GameSpeed;
	Options.GameSpeed = Session.Options.GameSpeed;

	NodeNameType * who = new NodeNameType;
	if (who) {
		std::strcpy(who->Name, Session.Handle);
		who->Player.House = Session.House;
		who->Player.Color = Session.ColorIdx;
		who->Player.ProcessTime = -1;
		Session.Players.Add(who);
	}

	Session.Options.Bases = state.Bases;
	Session.Options.Goodies = state.Crates;
	Session.Options.FogOfWar = state.Fog;
	Session.Options.BridgeDestruction = state.Bridges;
	Session.Options.MCVRedeploy = state.Redeploy;
	Session.Options.ShortGame = state.ShortGame;
	Session.Options.HarvTruce = false;
	Session.Options.CrapEngineers = state.MultiEngineer;

	delete MultiplayerMapPreview;
	MultiplayerMapPreview = NULL;
}


static void Pick_Map(UISkirmishState & state)
{
	int old = Session.Options.ScenarioIndex;

	if (!Scenario_Dialog()) {
		Session.Options.ScenarioIndex = old;
		Set_Scenario_Info_From_Index(old);
		Refresh_Preview();
	} else if (Set_Scenario_Info_From_Index(Session.Options.ScenarioIndex)) {
		Refresh_Preview();
	} else {
		Session.Options.ScenarioIndex = old;
	}

	state.MapName = Session.Options.ScenarioDescription;
	UI_Map_Preview_Image(state.Preview);
}


namespace
{

class UISkirmishEngineServiceClass : public UISkirmishServiceClass
{
	public:
		virtual void Pick_Map(UISkirmishState & state) override
		{
			::Pick_Map(state);
		}

		virtual bool Can_Start(UISkirmishState const & state) override
		{
			int waypoints = RandomMapWaypointCount(Session.Options.ScenarioIndex);
			if (waypoints >= state.AIPlayers + 1) {
				return(true);
			}

			char buffer[256];
			std::snprintf(buffer, sizeof(buffer), Fetch_String(TXT_SCENARIO_TOO_SMALL), waypoints);
			WWMessageBox().Process(buffer, TXT_OK);
			return(false);
		}
};

}


void UI_Skirmish_State(UISkirmishState & state)
{
	state.Handle = Session.Handle;

	state.Sides.clear();
	state.Side = 0;
	for (int index = 0; index < HouseTypes.Count(); index++) {
		HouseTypeClass * house = HouseTypes[index];
		if (!house->IsMultiplay) {
			continue;
		}

		UISkirmishOption option;
		option.Label = (char const *)house->GivenName;
		option.Value = index;
		if (index == Session.House) {
			state.Side = (int)state.Sides.size();
		}
		state.Sides.push_back(option);
	}

	int const palette = (int)(sizeof(UI_SKIRMISH_COLORS) / sizeof(UI_SKIRMISH_COLORS[0]));
	state.Colors.clear();
	for (int index = 0; index < MAX_PLAYERS && index < palette; index++) {
		UISkirmishOption option;
		option.Label = Fetch_String(UI_SKIRMISH_COLORS[index]);
		option.Value = index;
		option.Color = UI_Color_Text(PlayerColorTable[index]);
		state.Colors.push_back(option);
	}
	state.Color = (Session.PrefColor >= 0 && Session.PrefColor < (int)state.Colors.size()) ? Session.PrefColor : 0;

	state.UnitCountMin = SessionClass::CountMin[1];
	state.UnitCountMax = SessionClass::CountMax[1];
	state.UnitCount = Session.Options.UnitCount;

	state.TechLevelMax = MPLAYER_BUILD_LEVEL_MAX;
	state.TechLevel = BuildLevel;

	state.AILevel = Session.Options.AIDifficulty;

	state.CreditsMin = UI_SKIRMISH_MIN_MONEY;
	state.CreditsMax = Rule->MPMaxMoney;
	state.CreditsStep = 250;
	state.Credits = Session.Options.Credits;

	state.AIPlayersMax = 7;
	state.AIPlayers = Session.Options.AIPlayers > 1 ? Session.Options.AIPlayers : 1;

	state.GameSpeed = 6 - Session.Options.GameSpeed;

	state.Bases = Session.Options.Bases;
	state.Crates = Session.Options.Goodies;
	state.Fog = Session.Options.FogOfWar;
	state.Bridges = Session.Options.BridgeDestruction;
	state.Redeploy = Session.Options.MCVRedeploy;
	state.ShortGame = Session.Options.ShortGame;
	state.MultiEngineer = Session.Options.CrapEngineers;

	Set_Scenario_Info_From_Index(0);
	Session.Options.ScenarioIndex = 0;
	state.MapName = Session.Options.ScenarioDescription;

	Clear_Vector(&Session.Players);
	Clear_Vector(&Session.Computers);

	Update_Network_Dialog_Preview();
	UI_Map_Preview_Image(state.Preview);
}


bool UI_Skirmish_Dialog(void)
{
	UISkirmishState state;
	UI_Skirmish_State(state);

	UISkirmishEngineServiceClass service;
	UISkirmishPresenterClass presenter(service, std::move(state));
	std::unique_ptr<UIViewClass> view = UI_Skirmish_View(presenter);

	UIResult result = UI_Run_Modal(*view);
	if (result == UI_RESULT_FAILED_TO_OPEN) {
		return(false);
	}

	if (result != UI_RESULT_ACCEPTED || presenter.Choice != UI_SKIRMISH_START) {
		Remember_Preferences(presenter.State);
		return(false);
	}

	Commit(presenter.State);
	return(true);
}
