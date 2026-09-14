/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the skirmish setup: the settings the game offers, the map dialog it
// reopens around, and what the choices come to in the session. The presenter and the view
// live in uiskirmish.cpp so that the test harness can drive them without the engine.

#include "ui/screens/skirmish/uiskirmish.h"

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
#include "ui/rml/rmlrendermath.h"
#include "ui/uienginehost.h"
#include "ui/uipreview.h"
#include "ui/uishell.h"
#include "ui/uiview.h"
#include "win.h"

#include <cstdio>
#include <cstring>
#include <utility>


// The least a skirmish can be played for, as the Win32 dialog's own constant has it.
static int const UI_SKIRMISH_MIN_MONEY = 2500;

// The colours a player may wear, in the order the dialog lists them.
static int const UI_SKIRMISH_COLOURS[] = {
	TXT_GOLD, TXT_RED, TXT_BLUE, TXT_GREEN, TXT_ORANGE, TXT_SKY_BLUE, TXT_PURPLE, TXT_PINK
};


// A colour as a document writes one, rounded to what the game's 16-bit frame showed of it.
static std::string Colour_Text(COLORREF colour)
{
	std::uint8_t red = (std::uint8_t)GetRValue(colour);
	std::uint8_t green = (std::uint8_t)GetGValue(colour);
	std::uint8_t blue = (std::uint8_t)GetBValue(colour);
	UI_Render_Quantize_565(red, green, blue);

	char text[16];
	std::snprintf(text, sizeof(text), "#%02x%02x%02x", (unsigned)red, (unsigned)green, (unsigned)blue);
	return(std::string(text));
}


// A random map keeps its picture in a file of its own; every other map carries one.
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


// The handle, side and colour, which the Win32 dialog kept whether the player started or not.
static void Remember_Preferences(UISkirmishState const & state)
{
	std::strncpy(Session.Handle, state.Handle.c_str(), sizeof(Session.Handle) - 1);
	Session.Handle[sizeof(Session.Handle) - 1] = 0;

	if (state.Side >= 0 && state.Side < (int)state.Sides.size()) {
		Session.House = state.Sides[state.Side].Value;
	}

	Session.ColorIdx = state.Colour;
	Session.PrefColor = state.Colour;
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


// The map dialog, which the Win32 setup hid itself around and came back from.
static void Pick_Map(UISkirmishState & state)
{
	int old = Session.Options.ScenarioIndex;

	if (Scenario_Dialog(MainWindow) == IDCANCEL) {
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

	int const palette = (int)(sizeof(UI_SKIRMISH_COLOURS) / sizeof(UI_SKIRMISH_COLOURS[0]));
	state.Colours.clear();
	for (int index = 0; index < MAX_PLAYERS && index < palette; index++) {
		UISkirmishOption option;
		option.Label = Fetch_String(UI_SKIRMISH_COLOURS[index]);
		option.Value = index;
		option.Colour = Colour_Text(PlayerColorTable[index]);
		state.Colours.push_back(option);
	}
	state.Colour = (Session.PrefColor >= 0 && Session.PrefColor < (int)state.Colours.size()) ? Session.PrefColor : 0;

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

	// The bar counts the other way from the setting it makes, so faster is further right.
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


/// <summary>
/// Runs the skirmish setup as an RmlUi screen, reopening it around the map dialog.
/// </summary>
/// <param name="started">Did the player ask for the game to begin? Meaningless when this
/// returns false.</param>
/// <returns>bool; Was the screen shown at all? False leaves the session untouched and the
/// caller opens its Win32 dialog instead.</returns>
bool UI_Skirmish_Dialog(bool & started)
{
	started = false;

	if (UIShell.Legacy_Dialog_Visible()) {
		return(false);
	}

	UISkirmishState state;
	UI_Skirmish_State(state);

	bool reveal = true;

	for (;;) {
		state.Reveal = reveal;

		UISkirmishPresenterClass presenter(state);
		std::unique_ptr<UIViewClass> view = UI_Skirmish_View(presenter);

		UIResult result = UI_Run_Modal(*view);
		if (result == UI_RESULT_FAILED_TO_OPEN) {
			// Only the first pass can still fall back; a later one has already been shown.
			return(!reveal);
		}

		state = std::move(presenter.State);
		reveal = false;

		if (result == UI_RESULT_SESSION_ENDED || presenter.Choice == UI_SKIRMISH_CANCEL) {
			Remember_Preferences(state);
			return(true);
		}

		if (presenter.Choice == UI_SKIRMISH_PICK_MAP) {
			Pick_Map(state);
			continue;
		}

		// A map with fewer starting points than the players asked for cannot be played.
		int waypoints = RandomMapWaypointCount(Session.Options.ScenarioIndex);
		if (waypoints < state.AIPlayers + 1) {
			char buffer[256];
			std::snprintf(buffer, sizeof(buffer), Fetch_String(TXT_SCENARIO_TOO_SMALL), waypoints);
			WWMessageBox().Process(buffer, TXT_OK);
			continue;
		}

		Commit(state);
		started = true;
		return(true);
	}
}
