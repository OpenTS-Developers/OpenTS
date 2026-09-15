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


// What the map dialog closes with. Asking for a random map closes it too, because the Win32
// dialog hid itself around the generator and came back afterwards.
enum UIScenarioChoice
{
	UI_SCENARIO_ACCEPT,
	UI_SCENARIO_CANCEL,
	UI_SCENARIO_RANDOM,
};


// The engine call the map dialog makes. The game supplies one that reads the scenario off
// disk; the test harness supplies one that records the calls.
class UIScenarioServiceClass
{
	public:
		virtual ~UIScenarioServiceClass(void) = default;

		// The picture of the scenario at the given row, which the dialog shows as the
		// highlight moves. The scenario the dialog opened on is left selected either way.
		virtual void Preview(int index, UIMapPreviewImage & image) = 0;
};


// One row of the map list: the description the mission carries.
struct UIScenarioEntry
{
	std::string Label;
};


// What the dialog shows: the missions this machine holds, the row it opens on, and the
// picture of that row. Reveal is false on the pass that follows the generator.
struct UIScenarioState
{
	std::vector<UIScenarioEntry> Entries;
	int Selected = 0;
	UIMapPreviewImage Preview;
	bool Reveal = true;
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

// Runs the multiplayer map dialog, reopening it around the generator the way the Win32 dialog
// hid itself. Returns IDOK when the player took a map, otherwise IDCANCEL.
int UI_Scenario_Dialog(void);
