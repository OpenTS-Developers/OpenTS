/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "_ui.h"
#include "ccini.h"
#include "conquer.h"
#include "language/language.h"
#include "mapgen.h"
#include "msgloop.h"
#include "netdlg2.h"
#include "netshare.h"
#include "preview.h"
#include "rect.h"
#include "session.h"
#include "ui/screens/scenario/uiscenario.h"
#include "ui/uienginehost.h"
#include "ui/uipreview.h"
#include "ui/uishell.h"
#include "ui/uiview.h"
#include "win.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>


namespace
{

class UIScenarioEngineServiceClass : public UIScenarioServiceClass
{
	public:
		virtual void Preview(int index, UIMapPreviewImage & image) override
		{
			int original = Session.Options.ScenarioIndex;

			if (index >= 0 && index < Session.Scenarios.Count()) {
				Set_Scenario_Info_From_Index(index);

				if (stricmp(Session.Scenarios[index]->Get_Filename(), RANDOM_MAP_FILE_NAME) == 0) {
					delete MultiplayerMapPreview;
					MultiplayerMapPreview = new MapPreviewClass;
					MultiplayerMapPreview->Read_PCX_Preview("RandMap.img");
					if (MultiplayerMapPreview->Get_Preview_Surface() == NULL) {
						Update_Network_Dialog_Preview();
					}
				} else {
					Update_Network_Dialog_Preview();
				}
			}

			UI_Map_Preview_Image(image);

			Session.Options.ScenarioIndex = original;
			Set_Scenario_Info_From_Index(original);
		}

		virtual void Read(UIScenarioState & state) override
		{
			UI_Scenario_State(state);
		}

		virtual int Random(void) override
		{
			return(CreateRandomMap());
		}
};


bool Service_Pick(void)
{
	Windows_Message_Handler();

	if (Session.Is_Network_Game()) {
		return(Net2Callback());
	}

	Call_Back();
	return(false);
}

}


UIScenarioServiceClass & UI_Scenario_Service(void)
{
	static UIScenarioEngineServiceClass service;
	return(service);
}


namespace
{

std::string Display_Theater(std::string const & name)
{
	std::string result = name;
	for (std::size_t index = 0; index < result.size(); index++) {
		result[index] = (index == 0)
			? (char)std::toupper((unsigned char)result[index])
			: (char)std::tolower((unsigned char)result[index]);
	}
	return(result);
}


std::string Build_Meta(UIScenarioEntry const & entry)
{
	std::string meta;
	auto add = [&meta](std::string const & part) {
		if (part.empty()) {
			return;
		}
		if (!meta.empty()) {
			meta += " \xC2\xB7 ";
		}
		meta += part;
	};

	if (entry.Players > 0) {
		add(std::to_string(entry.Players) + " players");
	}
	add(Display_Theater(entry.Theater));
	if (entry.Width > 0 && entry.Height > 0) {
		add(std::to_string(entry.Width) + "\xC3\x97" + std::to_string(entry.Height));
	}
	add(entry.Official ? "Official" : "Custom");

	return(meta);
}


void Read_Map_Section(char const * filename, std::string & theater, int & width, int & height)
{
	theater.clear();
	width = 0;
	height = 0;

	CCFileClass file(filename);
	INIClass ini;
	if (!ini.Load(file)) {
		return;
	}

	char buffer[64];
	ini.Get_String("Map", "Theater", "", buffer, sizeof(buffer));
	theater = buffer;

	Rect size = ini.Get_Rect("Map", "Size", Rect(0, 0, 0, 0));
	width = size.Width;
	height = size.Height;
}


void Add_Option(std::vector<UIScenarioOption> & options, std::string const & label, int value)
{
	UIScenarioOption option;
	option.Label = label;
	option.Value = value;
	options.push_back(option);
}


void Build_Options(UIScenarioState & state)
{
	std::vector<int> counts;
	for (UIScenarioEntry const & entry : state.All) {
		if (entry.Players > 0 && std::find(counts.begin(), counts.end(), entry.Players) == counts.end()) {
			counts.push_back(entry.Players);
		}
	}
	std::sort(counts.begin(), counts.end());

	state.Players.clear();
	Add_Option(state.Players, "Any", 0);
	for (int count : counts) {
		Add_Option(state.Players, std::to_string(count) + " players", count);
	}

	state.TheaterNames.clear();
	for (UIScenarioEntry const & entry : state.All) {
		if (!entry.Theater.empty() && std::find(state.TheaterNames.begin(), state.TheaterNames.end(), entry.Theater) == state.TheaterNames.end()) {
			state.TheaterNames.push_back(entry.Theater);
		}
	}

	state.Theaters.clear();
	Add_Option(state.Theaters, "Any", 0);
	for (std::size_t index = 0; index < state.TheaterNames.size(); index++) {
		Add_Option(state.Theaters, Display_Theater(state.TheaterNames[index]), (int)(index + 1));
	}

	state.Sources.clear();
	Add_Option(state.Sources, "Any", 0);
	Add_Option(state.Sources, "Official", 1);
	Add_Option(state.Sources, "Custom", 2);
}

}


void UI_Scenario_State(UIScenarioState & state)
{
	state.All.clear();
	for (int index = 0; index < Session.Scenarios.Count(); index++) {
		UIScenarioEntry entry;
		entry.Index = index;
		entry.Label = Session.Scenarios[index]->Description();
		entry.Official = Session.Scenarios[index]->Get_Official();
		entry.Players = RandomMapWaypointCount(index);
		Read_Map_Section(Session.Scenarios[index]->Get_Filename(), entry.Theater, entry.Width, entry.Height);
		entry.Meta = Build_Meta(entry);
		state.All.push_back(entry);
	}

	Build_Options(state);

	state.Query.clear();
	state.Player = 0;
	state.Theater = 0;
	state.Source = 0;

	state.Entries = state.All;

	int selected = Session.Options.ScenarioIndex;
	if (selected < 0 || selected >= (int)state.All.size()) {
		selected = state.All.empty() ? -1 : 0;
	}
	state.Selected = selected;
	state.Meta = selected >= 0 ? state.All[selected].Meta : std::string();
	state.Preview = UIMapPreviewImage();

	if (selected >= 0) {
		UI_Scenario_Service().Preview(selected, state.Preview);
	}
}


bool UI_Scenario_Dialog(void)
{
	UIScenarioState state;
	UI_Scenario_State(state);

	UIScenarioPresenterClass presenter(UI_Scenario_Service(), std::move(state));
	std::unique_ptr<UIViewClass> view = UI_Scenario_View(presenter);

	UIResult result = UIShell.Run_Modal(*view, Service_Pick, true);
	if (result != UI_RESULT_ACCEPTED || presenter.Choice != UI_SCENARIO_ACCEPT || presenter.State.Selected < 0) {
		return(false);
	}

	Session.Options.ScenarioIndex = presenter.State.Selected;
	return(true);
}
