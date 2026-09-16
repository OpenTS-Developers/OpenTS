/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the random map generator: what the screen reads out of the seed the
// generator keeps, and the entry the generator's own driver calls.

#include "ui/screens/mapgen/uimapgen.h"

#include "_ui.h"
#include "addon.h"
#include "data.h"
#include "language/language.h"
#include "mapgen.h"
#include "session.h"
#include "wdtnet.h"
#include "worlddom.h"
#include "ui/uienginehost.h"
#include "ui/uipreview.h"
#include "ui/uishell.h"
#include "ui/uiview.h"

#include <cstring>


namespace
{

int const UI_MAPGEN_BIOME_NAMES[BIOME_COUNT] = {
	TXT_BIOME_TUNDRA, TXT_BIOME_TAIGA, TXT_BIOME_TEMPERATE, TXT_BIOME_DESERT, TXT_BIOME_MUTATED
};

int const UI_MAPGEN_TIME_NAMES[TIME_OF_DAY_COUNT] = {
	TXT_TIME_MORNING, TXT_TIME_AFTERNOON, TXT_TIME_DUSK, TXT_TIME_NIGHT
};

int const UI_MAPGEN_SIZE_NAMES[MAPSIZE_COUNT] = {
	TXT_MAPSIZE_SMALL, TXT_MAPSIZE_MEDIUM, TXT_MAPSIZE_LARGE, TXT_MAPSIZE_VERY_LARGE
};


class UIMapGenEngineServiceClass : public UIMapGenServiceClass
{
	public:
		virtual void Read(UIMapGenState & state) override
		{
			MapSeedClass & seed = RandomMapGen.SeedData;
			seed.Fixup_Settings();

			state.Firestorm = Addon_Enabled(ADDON_FIRESTORM);

			// The expansion's own country is offered only where it is enabled, as the
			// dialog fills its box.
			state.Environments.clear();
			for (int index = BIOME_FIRST; index < BIOME_COUNT; index++) {
				if (index != BIOME_MUTATED || state.Firestorm) {
					state.Environments.push_back(UIMapGenOption{Fetch_String(UI_MAPGEN_BIOME_NAMES[index])});
				}
			}
			state.Environment = (seed.Biome >= 0 && seed.Biome < (int)state.Environments.size()) ? seed.Biome : 0;

			state.Times.clear();
			for (int index = TIME_OF_DAY_FIRST; index < TIME_OF_DAY_COUNT; index++) {
				state.Times.push_back(UIMapGenOption{Fetch_String(UI_MAPGEN_TIME_NAMES[index])});
			}
			state.Time = seed.Time;

			state.Sizes.clear();
			for (int index = 0; index < MAPSIZE_COUNT; index++) {
				state.Sizes.push_back(UIMapGenOption{Fetch_String(UI_MAPGEN_SIZE_NAMES[index])});
			}
			state.Width = seed.Width;
			state.Height = seed.Height;

			Set_Slider(state.Players, seed.NumPlayers, 2, MAX_PLAYERS);
			Set_Slider(state.Cliffs, seed.Cliffs, 0, 100);
			Set_Slider(state.Accessibility, seed.Accessibility, 0, 100);
			Set_Slider(state.Hills, seed.Hills, 0, 100);
			Set_Slider(state.TiberiumAmount, seed.Tiberium, 1, 100);
			Set_Slider(state.TiberiumFields, seed.TiberiumLayout, 0, 100);
			Set_Slider(state.Water, seed.WaterAmount, 0, 100);
			Set_Slider(state.Vegetation, seed.Vegetation, 0, 100);
			Set_Slider(state.Cities, seed.Cities, 0, 100);
			Set_Slider(state.Veinholes, seed.VeinholeMonsters, 0, 5);

			state.IonStorms = seed.UseIonStorms;
			state.Transitions = seed.UseTransitions;
			state.Lifeforms = seed.TiberiumWildlife > 0;

			bool const present = seed.Files_Present();
			state.LoadEnabled = present;
			state.DeleteEnabled = present;
			state.PreviewEnabled = !Debug_Map;

			Read_Territory(state);

			UI_Generated_Map_Preview_Image(state.Preview);
		}

		virtual void Set(char const * name, int value) override
		{
			MapSeedClass & seed = RandomMapGen.SeedData;

			// The tour fights its battles between four, so the reading it hides is not written.
			if (Territory() != NULL && std::strcmp(name, "players") == 0) {
				return;
			}

			if (std::strcmp(name, "environment") == 0) {
				seed.Biome = value;
			} else if (std::strcmp(name, "time") == 0) {
				seed.Time = value;
			} else if (std::strcmp(name, "width") == 0) {
				seed.Width = value;
			} else if (std::strcmp(name, "height") == 0) {
				seed.Height = value;
			} else if (std::strcmp(name, "players") == 0) {
				seed.NumPlayers = value;
			} else if (std::strcmp(name, "cliffs") == 0) {
				seed.Cliffs = value;
			} else if (std::strcmp(name, "accessibility") == 0) {
				seed.Accessibility = value;
			} else if (std::strcmp(name, "hills") == 0) {
				seed.Hills = value;
			} else if (std::strcmp(name, "tiberiumamount") == 0) {
				seed.Tiberium = value;
			} else if (std::strcmp(name, "tiberiumfields") == 0) {
				seed.TiberiumLayout = value;
			} else if (std::strcmp(name, "water") == 0) {
				seed.WaterAmount = value;
			} else if (std::strcmp(name, "vegetation") == 0) {
				seed.Vegetation = value;
			} else if (std::strcmp(name, "cities") == 0) {
				seed.Cities = value;
			} else if (std::strcmp(name, "veinholes") == 0) {
				seed.VeinholeMonsters = value;
			} else if (std::strcmp(name, "ionstorms") == 0) {
				seed.UseIonStorms = (value != 0);
			} else if (std::strcmp(name, "transitions") == 0) {
				seed.UseTransitions = (value != 0);
			} else if (std::strcmp(name, "lifeforms") == 0) {
				seed.TiberiumWildlife = (value != 0) ? 30 : 0;
			}
		}

		virtual void Preview(void) override
		{
			RandomMapGen.Generate_Random_Map(true);
		}

		virtual void Surprise(void) override
		{
			RandomMapGen.SeedData.Randomize();
		}

		virtual void Save(void) override
		{
			RandomMapGen.SeedData.MapDescription[0] = '\0';
			RandomMapGen.SeedData.LoadOptionsClass::Save(RandomMapGen.SeedData.MapDescription, sizeof(RandomMapGen.SeedData.MapDescription));
		}

		virtual void Load(void) override
		{
			if (RandomMapGen.SeedData.LoadOptionsClass::Load() == true) {
				RandomMapGen.Generate_Random_Map(true);
			}
		}

		virtual void Delete(void) override
		{
			RandomMapGen.SeedData.LoadOptionsClass::Delete();
		}

	private:
		static void Set_Slider(UIMapGenSlider & slider, int value, int minimum, int maximum)
		{
			slider.Minimum = minimum;
			slider.Maximum = maximum;
			slider.Value = (value < minimum) ? minimum : ((value > maximum) ? maximum : value);
		}

		static WDTTerritory * Territory(void)
		{
			if (Session.Type != GAME_INTERNET || !Session.IsWDT) {
				return(NULL);
			}
			return(WDT_Get_Territory(Session.WDTTerritory));
		}

		// A reading the tour left nothing to choose between is shown over the whole range and
		// locked, which is what the dialog's own slider setup did with it.
		static void Bound(UIMapGenSlider & slider, int minimum, int maximum, bool allowed)
		{
			if (maximum <= minimum) {
				slider.Minimum = 0;
				slider.Maximum = 100;
				slider.Enabled = false;
				return;
			}
			slider.Minimum = minimum;
			slider.Maximum = maximum;
			slider.Enabled = allowed;
			slider.Value = (slider.Value < minimum) ? minimum : ((slider.Value > maximum) ? maximum : slider.Value);
		}

		static void Read_Territory(UIMapGenState & state)
		{
			WDTTerritory * territory = Territory();
			state.Territory = territory != NULL;
			if (territory == NULL) {
				return;
			}

			Bound(state.Cliffs, territory->CliffsMin, territory->CliffsMax, territory->UserModCliffs != 0);
			Bound(state.Accessibility, territory->AccessibilityMin, territory->AccessibilityMax, territory->UserModAccessability != 0);
			Bound(state.Hills, territory->HillsMin, territory->HillsMax, territory->UserModHills != 0);
			Bound(state.TiberiumAmount, territory->TiberiumAmountMin, territory->TiberiumAmountMax, territory->UserModTiberiumAmount != 0);
			Bound(state.TiberiumFields, territory->TiberiumFieldsMin, territory->TiberiumFieldsMax, territory->UserModTiberiumFields != 0);
			Bound(state.Water, territory->WaterMin, territory->WaterMax, territory->UserModWater != 0);
			Bound(state.Vegetation, territory->VegetationMin, territory->VegetationMax, territory->UserModVegetation != 0);
			Bound(state.Cities, territory->CitiesMin, territory->CitiesMax, territory->UserModCities != 0);
			state.Veinholes.Enabled = territory->UserModVeinholeMonsters != 0;

			// The tour names the number of players itself, and the dialog's own reading said so
			// with a pair of boxes rather than a bar.
			state.Players.Value = 4;
			state.Players.Enabled = false;

			state.EnvironmentEnabled = territory->UserModBiome != 0;
			state.TimeEnabled = territory->UserModTime != 0;
			state.WidthEnabled = territory->UserModWidth != 0;
			state.HeightEnabled = territory->UserModHeight != 0;
			state.TransitionsEnabled = territory->UserModTimeTransitions != 0;
			state.LifeformsEnabled = territory->UserModTiberiumCreatures != 0;

			// Nothing to roll leaves the button dead, as the dialog left it.
			state.SurpriseEnabled = territory->UserModBiome || territory->UserModTime
				|| territory->UserModCliffs || territory->UserModAccessability
				|| territory->UserModHills || territory->UserModTiberiumAmount
				|| territory->UserModTiberiumFields || territory->UserModWater
				|| territory->UserModVegetation || territory->UserModCities
				|| territory->UserModWidth || territory->UserModHeight
				|| territory->UserModVeinholeMonsters;
		}
};

}


UIMapGenServiceClass & UI_Map_Generator_Service(void)
{
	static UIMapGenEngineServiceClass service;
	return(service);
}


UIMapGenChoiceType UI_Map_Generator_Dialog(void)
{
	UIMapGenPresenterClass presenter(UI_Map_Generator_Service());
	std::unique_ptr<UIViewClass> view = UI_Map_Generator_View(presenter);

	UIResult result = UI_Run_Modal(*view, true);
	return((result == UI_RESULT_ACCEPTED) ? presenter.Choice : UI_MAPGEN_CANCEL);
}
