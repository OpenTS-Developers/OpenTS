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


// What the generator answers with.
enum UIMapGenChoiceType
{
	UI_MAPGEN_CANCEL,
	UI_MAPGEN_ACCEPT,
};


// One entry of a box the player picks from.
struct UIMapGenOption
{
	std::string Label;
};


// One reading the player drags: where it stands, the bounds the generator holds it to, and
// whether it may be dragged at all.
struct UIMapGenSlider
{
	int Value = 0;
	int Minimum = 0;
	int Maximum = 100;
	bool Enabled = true;
};


// What the generator shows. The expansion's template carries the veinhole reading and the
// three switches; the base game's does not, so a class on the body chooses. A battle fought
// over a tour territory is a third arrangement: the tour settles how many play and bounds or
// fixes the rest, and a setting it fixed is shown locked rather than taken away.
struct UIMapGenState
{
	bool Firestorm = false;
	bool Territory = false;

	std::vector<UIMapGenOption> Environments;
	std::vector<UIMapGenOption> Times;
	std::vector<UIMapGenOption> Sizes;
	int Environment = 0;
	int Time = 0;
	int Width = 0;
	int Height = 0;

	UIMapGenSlider Players;
	UIMapGenSlider Cliffs;
	UIMapGenSlider Accessibility;
	UIMapGenSlider Hills;
	UIMapGenSlider TiberiumAmount;
	UIMapGenSlider TiberiumFields;
	UIMapGenSlider Water;
	UIMapGenSlider Vegetation;
	UIMapGenSlider Cities;
	UIMapGenSlider Veinholes;

	bool IonStorms = false;
	bool Transitions = false;
	bool Lifeforms = false;

	bool EnvironmentEnabled = true;
	bool TimeEnabled = true;
	bool WidthEnabled = true;
	bool HeightEnabled = true;
	bool IonStormsEnabled = true;
	bool TransitionsEnabled = true;
	bool LifeformsEnabled = true;
	bool SurpriseEnabled = true;

	bool LoadEnabled = false;
	bool DeleteEnabled = false;
	bool PreviewEnabled = true;

	// The picture the generator last drew, which travels as bytes.
	UIMapPreviewImage Preview;
};


// What the screen asks the generator for. The engine implements it; the harness hands the
// screen a recording one.
class UIMapGenServiceClass
{
	public:
		virtual ~UIMapGenServiceClass(void) = default;

		// Copies the generator's settings into the model.
		virtual void Read(UIMapGenState & state) = 0;

		// Writes one reading or one box back, named as the document names it.
		virtual void Set(char const * name, int value) = 0;

		// Draws a picture of the map the settings describe.
		virtual void Preview(void) = 0;

		// Rolls every setting at once, as the Surprise Me button does.
		virtual void Surprise(void) = 0;

		// The three file operations the dialog offers over its saved seeds.
		virtual void Save(void) = 0;
		virtual void Load(void) = 0;
		virtual void Delete(void) = 0;
};


// A leaf over the generator's settings: the readings and boxes write straight through, and
// the buttons close it or raise what they raise.
class UIMapGenPresenterClass : public UIPresenterClass
{
	public:
		explicit UIMapGenPresenterClass(UIMapGenServiceClass & service);

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIMapGenState State;
		UIMapGenChoiceType Choice = UI_MAPGEN_CANCEL;

		// What the screen must close and reopen around, because the generator raises a dialog
		// of its own for it.
		enum RaiseType { RAISE_NOTHING, RAISE_SAVE, RAISE_LOAD, RAISE_DELETE };
		RaiseType Raise = RAISE_NOTHING;

	private:
		UIMapGenServiceClass & Service;
};


// The RmlUi view over a generator presenter, bound to mapgen.rml. The presenter must
// outlive it.
std::unique_ptr<UIViewClass> UI_Map_Generator_View(UIMapGenPresenterClass & presenter);

// The generator's own service.
UIMapGenServiceClass & UI_Map_Generator_Service(void);

// Runs the generator as an RmlUi screen, reopening it around anything it raises. False means
// it could not run as one and the caller should open its Win32 dialog; otherwise choice
// carries whether the player took the map.
bool UI_Map_Generator_Dialog(UIMapGenChoiceType & choice);
