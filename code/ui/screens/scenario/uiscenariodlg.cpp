/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the multiplayer map dialog: the missions the machine holds, the picture
// of the highlighted one, and the generator it raises over itself. The presenter and the view
// live in uiscenario.cpp so that the test harness can drive them without the engine.

#include "ui/screens/scenario/uiscenario.h"

#include "_ui.h"
#include "conquer.h"
#include "language/language.h"
#include "mapgen.h"
#include "msgloop.h"
#include "netdlg2.h"
#include "netshare.h"
#include "preview.h"
#include "session.h"
#include "ui/uienginehost.h"
#include "ui/uipreview.h"
#include "ui/uishell.h"
#include "ui/uiview.h"
#include "win.h"

#include <utility>


namespace
{

// Reads the highlighted mission off disk. Browsing changes the scenario the globals name, so
// the row the dialog opened on is put back after every look, as the Win32 dialog's idle
// handler did; nothing is settled until the player accepts.
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


// The pass of the game the dialog runs over. A lobby waiting on the host has to be pumped
// through its own handler while the host browses, which is the one thing the ordinary
// service pass does not do.
bool Service_Pick(void)
{
	Windows_Message_Handler();

	if (Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) {
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


void UI_Scenario_State(UIScenarioState & state)
{
	state.Entries.clear();
	for (int index = 0; index < Session.Scenarios.Count(); index++) {
		UIScenarioEntry entry;
		entry.Label = Session.Scenarios[index]->Description();
		state.Entries.push_back(entry);
	}

	state.Selected = Session.Options.ScenarioIndex;
	if (state.Selected < 0 || state.Selected >= (int)state.Entries.size()) {
		state.Selected = 0;
	}

	UI_Scenario_Service().Preview(state.Selected, state.Preview);
}


/// <summary>
/// Runs the multiplayer map dialog. The generator runs over it rather than in its place.
/// </summary>
/// <returns>Returns with IDOK when the player took a map, otherwise IDCANCEL.</returns>
int UI_Scenario_Dialog(void)
{
	UIScenarioState state;
	UI_Scenario_State(state);

	UIScenarioPresenterClass presenter(UI_Scenario_Service(), std::move(state));
	std::unique_ptr<UIViewClass> view = UI_Scenario_View(presenter);

	UIResult result = UIShell.Run_Modal(*view, Service_Pick, true);
	if (result != UI_RESULT_ACCEPTED || presenter.Choice != UI_SCENARIO_ACCEPT) {
		return(IDCANCEL);
	}

	Session.Options.ScenarioIndex = presenter.State.Selected > 0 ? presenter.State.Selected : 0;
	return(IDOK);
}
