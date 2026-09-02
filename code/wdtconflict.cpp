/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "data.h"
#include "language/language.h"
#include "wdtnet.h"

using namespace WorldDominationTour;


/// <summary>
/// Creates a conflict over the specified territory.
/// A conflict is the campaign's record of a battle still to be fought. The territory is
/// what supplies the map and the game options when the player finally commits to it.
/// </summary>
/// <param name="terr">The territory to be fought over. May be NULL, which leaves the
/// conflict as a placeholder with no battle attached to it.</param>
Conflict::Conflict(WDTTerritory * terr) :
	Territory(terr)
{
	/// nothing
}


/// <summary>
/// Creates a copy of a conflict record.
/// The campaign keeps its conflicts in a vector by value, so they are copied about freely.
/// </summary>
Conflict::Conflict(Conflict const & that) :
	Territory(that.Territory)
{
	/// nothing
}


/// <summary>
/// Assigns one conflict to another.
/// </summary>
/// <returns>Returns with a reference to this conflict.</returns>
Conflict & Conflict::operator=(Conflict const & that)
{
	Territory = that.Territory;
	return(*this);
}


/// <summary>
/// Determines if two conflicts are the same battle.
/// Two conflicts are considered equal when they are over the same territory.
/// </summary>
/// <returns>bool; Are both conflicts over the same territory?</returns>
bool Conflict::operator==(Conflict const & that) const
{
	return(Territory == that.Territory);
}


/// <summary>
/// Destroys the conflict record.
/// The territory itself belongs to the campaign map and outlives every conflict that
/// refers to it, so there is nothing here to clean up.
/// </summary>
Conflict::~Conflict(void)
{
	/// nothing
}


/// <summary>
/// Has this conflict been given a territory to be fought over?
/// A default constructed conflict is a placeholder with nowhere to fight, and the campaign
/// uses this test to tell the two apart.
/// </summary>
/// <returns>bool; Does this conflict refer to a territory?</returns>
bool Conflict::Is_Territory_Set(void) const
{
	return(Territory != NULL);
}


/// <summary>
/// Fetches the map index of the territory being fought over.
/// </summary>
/// <returns>Returns with the territory index, or zero if no territory has been set.</returns>
int Conflict::Get_Territory_Index(void) const
{
	if (Territory != NULL) {
		return(Territory->Index);
	}
	return(0);
}


typedef DynamicVectorClass<WorldDominationTour::GameOption *> WDT_GAME_OPTION_LIST;
WDT_GAME_OPTION_LIST WDTGameOptionList;


/// <summary>
/// Creates the list of game options that a territory can be described by.
/// Each option knows how to pull one rule out of a territory and phrase it for the player.
/// Process_Game_Options builds the list on demand, so any options surviving from an earlier
/// conflict are thrown away first.
/// </summary>
void Conflict::Init_Game_Options(void)
{
	enum {
		WDT_BITMASK_TIME_TRANSITIONS = 1 << 0,
		WDT_BITMASK_CREATURES = 1 << 1,
		WDT_BITMASK_ALLIANCES = 1 << 2,
		WDT_BITMASK_HARV_TRUCE = 1 << 3,
		WDT_BITMASK_BASES = 1 << 4,
		WDT_BITMASK_MCV_REDEPLOY = 1 << 5,
		WDT_BITMASK_FOG_OF_WAR = 1 << 6,
		WDT_BITMASK_BRIDGE_DESTROY = 1 << 7,
		WDT_BITMASK_CRATES = 1 << 8,
		WDT_BITMASK_BLUE_TIBERIUM = 1 << 9,
		WDT_BITMASK_SHORT_GAME = 1 << 10,
		WDT_BITMASK_CRAP_ENGINEER = 1 << 11,
		WDT_BITMASK_BIOME = 1 << 12,
		WDT_BITMASK_TIME_OF_DAY = 1 << 13,
		WDT_BITMASK_NUM_PLAYERS = 1 << 14,
		WDT_BITMASK_UNIT_COUNT = 1 << 15,
		WDT_BITMASK_TECHLEVEL = 1 << 16,
		WDT_BITMASK_CREDITS = 1 << 17,
		WDT_BITMASK_CLIFFS = 1 << 18,
		WDT_BITMASK_ACCESSABILITY = 1 << 19,
		WDT_BITMASK_HILLS = 1 << 20,
		WDT_BITMASK_TIBERIUM = 1 << 21,
		WDT_BITMASK_TIBERIUM_FIELDS = 1 << 22,
		WDT_BITMASK_WATER = 1 << 23,
		WDT_BITMASK_VEGETATION = 1 << 24,
		WDT_BITMASK_CITIES = 1 << 25,
		WDT_BITMASK_MAP_WIDTH = 1 << 26,
		WDT_BITMASK_MAP_HEIGHT = 1 << 27,
		WDT_BITMASK_SEED = 1 << 28,
		WDT_BITMASK_VEINHOLES = 1 << 29,
	};

	Deinit_Game_Options();

	WDTGameOptionList.Add(new NumberOfPlayersGameOption(WDT_BITMASK_NUM_PLAYERS
	));

	WDTGameOptionList.Add(new ValueGameOption<unsigned char>(WDT_BITMASK_TECHLEVEL,
		0,
		offsetof(WDTTerritory, TechLevel),
		TXT_WDT_TECHLEVEL
	));

	WDTGameOptionList.Add(new FlagGameOption(WDT_BITMASK_BASES,
		TXT_WDT_BASES_OFF, FALSE,
		TXT_WDT_BASES_ON, TRUE
	));

	WDTGameOptionList.Add(new FlagGameOption(WDT_BITMASK_SHORT_GAME,
		TXT_WDT_SHORT_GAME_OFF, TRUE,
		TXT_WDT_SHORT_GAME_ON, FALSE
	));

	WDTGameOptionList.Add(new ValueGameOption<unsigned short>(WDT_BITMASK_CREDITS,
		0,
		offsetof(WDTTerritory, StartingCredits),
		TXT_WDT_CREDITS
	));

	WDTGameOptionList.Add(new FlagGameOption(WDT_BITMASK_FOG_OF_WAR,
		TXT_NONE, TRUE,
		TXT_WDT_FOG_OF_WAR, FALSE
	));

	WDTGameOptionList.Add(new FlagGameOption(WDT_BITMASK_HARV_TRUCE,
		TXT_NONE, TRUE,
		TXT_WDT_HARV_TRUCE, FALSE
	));

	WDTGameOptionList.Add(new ValueGameOption<unsigned char>(WDT_BITMASK_UNIT_COUNT,
		0,
		offsetof(WDTTerritory, UnitCount),
		TXT_WDT_STARTING_UNITS
	));

	WDTGameOptionList.Add(new FlagGameOption(WDT_BITMASK_MCV_REDEPLOY,
		TXT_WDT_MCV_REDEPLOY_OFF, TRUE,
		TXT_WDT_MCV_REDEPLOY_ON, FALSE
	));

	WDTGameOptionList.Add(new FlagGameOption(WDT_BITMASK_CRATES,
		TXT_WDT_GOODIES_OFF, TRUE,
		TXT_WDT_GOODIES_ON, TRUE
	));

	WDTGameOptionList.Add(new RangedGameOptionT<char>(WDT_BITMASK_TIBERIUM,
		offsetof(WDTTerritory, TiberiumAmountDefault),
		TXT_WDT_TIBERIUM_RANGE,
		10, 30, 90, 90
	));

	WDTGameOptionList.Add(new FlagGameOption(WDT_BITMASK_VEINHOLES,
		TXT_NONE, TRUE,
		TXT_WDT_VEINHOLES, TRUE
	));

	WDTGameOptionList.Add(new FlagGameOption(WDT_BITMASK_CREATURES,
		TXT_NONE, TRUE,
		TXT_WDT_LIFEFORMS, TRUE
	));

	WDTGameOptionList.Add(new RangedGameOptionT<char>(WDT_BITMASK_TIBERIUM_FIELDS,
		offsetof(WDTTerritory, TiberiumFieldsDefault),
		TXT_WDT_TIBERIUM_FIELDS_RANGE,
		10, 30, 90, 90
	));

	WDTGameOptionList.Add(
		new MapSizeGameOption(WDT_BITMASK_MAP_WIDTH | WDT_BITMASK_MAP_HEIGHT
	));

	WDTGameOptionList.Add(new FlagGameOption(WDT_BITMASK_BRIDGE_DESTROY,
		TXT_WDT_BRIDGE_DESTROY_OFF, TRUE,
		TXT_WDT_BRIDGE_DESTROY_ON, FALSE
	));

	WDTGameOptionList.Add(new RangedGameOptionT<char>(WDT_BITMASK_ACCESSABILITY,
		offsetof(WDTTerritory, AccessibilityDefault),
		TXT_WDT_ACCESSIBILITY_RANGE,
		10, 10, 90, 90
	));

	WDTGameOptionList.Add(new RangedGameOptionT<char>(WDT_BITMASK_CLIFFS,
		offsetof(WDTTerritory, CliffsDefault),
		TXT_WDT_CLIFFS_RANGE,
		0, 0, 90, 90
	));

	WDTGameOptionList.Add(new FlagGameOption(WDT_BITMASK_TIME_TRANSITIONS,
		TXT_NONE, TRUE,
		TXT_WDT_TIME_OF_DAY_TRANSITIONS, TRUE
	));

	WDTGameOptionList.Add(new RangedGameOptionT<char>(WDT_BITMASK_HILLS,
		offsetof(WDTTerritory, HillsDefault),
		TXT_WDT_HILLS_RANGE,
		0, 0, 90, 90
	));

	WDTGameOptionList.Add(new RangedGameOptionT<char>(WDT_BITMASK_WATER,
		offsetof(WDTTerritory, WaterDefault),
		TXT_WDT_WATER_RANGE,
		0, 0, 90, 90
	));

	WDTGameOptionList.Add(new RangedGameOptionT<char>(WDT_BITMASK_VEGETATION,
		offsetof(WDTTerritory, VegetationDefault),
		TXT_WDT_VEGETATION_RANGE,
		0, 0, 90, 90
	));

	WDTGameOptionList.Add(new RangedGameOptionT<char>(WDT_BITMASK_CITIES,
		offsetof(WDTTerritory, CitiesDefault),
		TXT_WDT_CITIES_RANGE,
		0, 0, 90, 90
	));
}


/// <summary>
/// Destroys the game option list.
/// Use this routine to release the option objects that Init_Game_Options created.
/// </summary>
void Conflict::Deinit_Game_Options(void)
{
	unsigned i = WDTGameOptionList.Count();
	while (i--) {
		GameOption * opt = WDTGameOptionList[i];
		if (opt != NULL) {
			delete WDTGameOptionList[i];
		}
	}
}


/// <summary>
/// Builds the game option summary text for this conflict.
/// This routine is used by the World Domination Tour screens to tell the player what kind
/// of battle he is about to pick. Each option contributes one comma separated phrase, and
/// the summary is deliberately brief, so the options flagged as most worth knowing are
/// described first and the merely user modifiable ones are left out altogether.
/// </summary>
/// <param name="str">The string to append the option descriptions to.</param>
/// <param name="len">The size of the destination string.</param>
void Conflict::Process_Game_Options(char * str, int len)
{
	if (!WDTGameOptionList.Count()) {
		Init_Game_Options();
	}

	WDTTerritory *territory = Territory;
	int left = 6;
	int value = 0;
	for (GameOption * opt : WDTGameOptionList) {
		if (!left) {
			break;
		}
		if ((value & opt->Bitmask) == 0 && opt->Get_Display_Priority(territory) == WDT_GAME_OPT_PRIORITY) {
			opt->Get_String(territory, str, len);
			left--;
			value |= opt->Bitmask;
		}
	}

	for (GameOption * opt : WDTGameOptionList) {
		if (!left) {
			break;
		}
		if ((value & opt->Bitmask) == 0 && opt->Get_Display_Priority(territory) != WDT_GAME_OPT_USER_MOD) {
			opt->Get_String(territory, str, len);
			left--;
			value |= opt->Bitmask;
		}
	}
}
