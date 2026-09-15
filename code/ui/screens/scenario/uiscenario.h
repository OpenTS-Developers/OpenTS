/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ui/uipreview.h"
#include "ui/uiscreen.h"

#include <memory>
#include <string>
#include <vector>

class UIViewClass;


// What the map dialog closes with.
enum UIScenarioChoice
{
	UI_SCENARIO_ACCEPT,
	UI_SCENARIO_CANCEL,
};


struct UIScenarioState;


// The engine calls the map dialog makes. The game supplies one that reads the scenario off
// disk; the test harness supplies one that records the calls.
class UIScenarioServiceClass
{
	public:
		virtual ~UIScenarioServiceClass(void) = default;

		// The picture of the scenario at the given row, which the dialog shows as the
		// highlight moves. The scenario the dialog opened on is left selected either way.
		virtual void Preview(int index, UIMapPreviewImage & image) = 0;

		// Fills the list and the highlight from the maps the session offers.
		virtual void Read(UIScenarioState & state) = 0;

		// Runs the generator over the dialog and returns the row its map took. A player who
		// backs out of the generator is answered with the first row, as the Win32 dialog
		// answered them.
		virtual int Random(void) = 0;
};


// One row of the map list: the description the mission carries.
struct UIScenarioEntry
{
	std::string Label;
};


// What the dialog shows: the missions this machine holds, the row it opens on, and the
// picture of that row.
struct UIScenarioState
{
	std::vector<UIScenarioEntry> Entries;
	int Selected = 0;
	UIMapPreviewImage Preview;
};


// Holds the highlighted row until the player settles, and asks the engine for a picture as
// the highlight moves, which is what the Win32 dialog did between messages.
class UIScenarioPresenterClass : public UIPresenterClass
{
	public:
		UIScenarioPresenterClass(UIScenarioServiceClass & service, UIScenarioState state);

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIScenarioState State;
		UIScenarioChoice Choice = UI_SCENARIO_CANCEL;

	private:
		UIScenarioServiceClass & Service;
};


// The RmlUi view over a map presenter, bound to scenario.rml. The presenter must outlive it.
std::unique_ptr<UIViewClass> UI_Scenario_View(UIScenarioPresenterClass & presenter);

// The game's service and the missions it holds.
UIScenarioServiceClass & UI_Scenario_Service(void);
void UI_Scenario_State(UIScenarioState & state);

// Runs the multiplayer map dialog, with the generator raised over it. True when the player
// took a map.
bool UI_Scenario_Dialog(void);
