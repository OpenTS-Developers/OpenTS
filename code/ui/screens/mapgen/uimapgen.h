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


enum UIMapGenChoiceType
{
	UI_MAPGEN_CANCEL,
	UI_MAPGEN_ACCEPT,
};


struct UIMapGenOption
{
	std::string Label;

	// The generator's enum value; the list is sorted, so a row's index is not the value.
	int Value = 0;
};


struct UIMapGenSlider
{
	int Value = 0;
	int Minimum = 0;
	int Maximum = 100;
	bool Enabled = true;
};


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

	UIMapPreviewImage Preview;
};


void UI_Sort_Map_Gen_Options(std::vector<UIMapGenOption> & options);

// Returns wanted when an option holds it, otherwise the first option's value, or 0 when the
// list is empty.
int UI_Map_Gen_Option_Value(std::vector<UIMapGenOption> const & options, int wanted);


class UIMapGenServiceClass
{
	public:
		virtual ~UIMapGenServiceClass(void) = default;

		virtual void Read(UIMapGenState & state) = 0;
		virtual void Set(char const * name, int value) = 0;
		virtual void Preview(void) = 0;
		virtual void Surprise(void) = 0;
		virtual void Save(void) = 0;
		virtual void Load(void) = 0;
		virtual void Delete(void) = 0;
};


class UIMapGenPresenterClass : public UIPresenterClass
{
	public:
		explicit UIMapGenPresenterClass(UIMapGenServiceClass & service);
		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIMapGenState State;
		UIMapGenChoiceType Choice = UI_MAPGEN_CANCEL;

	private:
		UIMapGenServiceClass & Service;
};


std::unique_ptr<UIViewClass> UI_Map_Generator_View(UIMapGenPresenterClass & presenter);

UIMapGenServiceClass & UI_Map_Generator_Service(void);

UIMapGenChoiceType UI_Map_Generator_Dialog(void);
