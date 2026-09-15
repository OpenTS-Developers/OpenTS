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


// What the setup closes with. Picking a map closes it too, because the Win32 dialog hid
// itself around the map dialog and came back afterwards.
enum UISkirmishChoice
{
	UI_SKIRMISH_START,
	UI_SKIRMISH_CANCEL,
};


struct UISkirmishState;


// The map dialog the setup raises over itself. The game supplies one; the harness can hand
// the screen a recording one.
class UISkirmishServiceClass
{
	public:
		virtual ~UISkirmishServiceClass(void) = default;

		// Runs the map dialog over the setup and writes back the map the player settled on,
		// which is the one the setup opened with when they backed out.
		virtual void Pick_Map(UISkirmishState & state) = 0;

		// Whether the map the setup is on has starting points for the players it asks for.
		// The player is told why it does not, over the setup.
		virtual bool Can_Start(UISkirmishState const & state) = 0;
};


// One row of a chooser: what it reads, the value the game knows it by, and the color the
// dropdown draws it in, which only the player colors carry.
struct UISkirmishOption
{
	std::string Label;
	int Value = 0;
	std::string Color;
};


// What the setup shows. The slider bounds travel with their readings because the rules give
// two of them, and the game speed reading counts the other way from the option it sets.
struct UISkirmishState
{
	std::string Handle;

	std::vector<UISkirmishOption> Sides;
	int Side = 0;
	std::vector<UISkirmishOption> Colors;
	int Color = 0;

	std::string MapName;
	UIMapPreviewImage Preview;

	bool Bases = true;
	bool Crates = true;
	bool Fog = false;
	bool Bridges = true;
	bool Redeploy = true;
	bool ShortGame = false;
	bool MultiEngineer = false;

	int UnitCount = 0;
	int UnitCountMin = 0;
	int UnitCountMax = 0;
	int Credits = 0;
	int CreditsMin = 0;
	int CreditsMax = 0;
	int CreditsStep = 1;
	int TechLevel = 1;
	int TechLevelMax = 1;
	int AILevel = 0;
	int AIPlayers = 1;
	int AIPlayersMax = 7;
	int GameSpeed = 0;
};


// Holds the settings until the player starts or cancels, and raises the map dialog over
// itself. Bases and a short game switch each other the way the Win32 dialog's check boxes do.
class UISkirmishPresenterClass : public UIPresenterClass
{
	public:
		UISkirmishPresenterClass(UISkirmishServiceClass & service, UISkirmishState state);

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UISkirmishState State;
		UISkirmishChoice Choice = UI_SKIRMISH_CANCEL;

	private:
		UISkirmishServiceClass & Service;
};


// The RmlUi view over a skirmish presenter, bound to skirmish.rml. The presenter must
// outlive it.
std::unique_ptr<UIViewClass> UI_Skirmish_View(UISkirmishPresenterClass & presenter);

// The settings the running game offers, and the map it opens on.
void UI_Skirmish_State(UISkirmishState & state);

// Runs the skirmish setup. True when the player asked for the game to begin.
bool UI_Skirmish_Dialog(void);
