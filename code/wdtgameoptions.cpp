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
/// Destructor for game option objects.
/// The options are static descriptors that own nothing of their own; this routine exists
/// so that the derived options can be destroyed through a base pointer.
/// </summary>
GameOption::~GameOption(void)
{
	//nothing
}


/// <summary>
/// Can the player change this option for the territory?
/// A territory locks down some of the game options and leaves the rest for the player to
/// set as he pleases before the battle begins.
/// </summary>
/// <returns>bool; Is this option left to the player's discretion?</returns>
bool GameOption::Is_User_Modifiable(const WDTTerritory * territory)
{
	return((territory->UserModBooleans & Bitmask) != 0);
}


/// <summary>
/// Adds a separating comma to the option string.
/// The game options append their descriptions one after another, so each calls this
/// routine first in order to keep the list readable.
/// </summary>
/// <param name="str">The string being built up.</param>
/// <param name="len">The maximum number of characters that may be appended.</param>
void WDT_Game_Option_Append_Comma(char *str, int len)
{
	if (*str != '\0') {
		strncat(str, ", ", len);
	}
}


/// <summary>
/// Appends the wording for this option's band to the display string.
/// The option owns a run of strings, one per band, so the description reads as low or
/// high rather than as a raw number.
/// </summary>
/// <param name="str">The string to append the description to.</param>
/// <param name="len">The maximum number of characters that may be appended.</param>
void RangedGameOption::Get_String(const WDTTerritory * territory, char * str, int len)
{
	int stringid = Get_In_Range_Of(territory);
	WDT_Game_Option_Append_Comma(str, len);
	strncat(str, Fetch_String(StringID + stringid), len);
}


/// <summary>
/// Determines which band of this option's range the value falls into.
/// The bands run from very low to very high and are what the descriptive wording and the
/// display priority are both chosen from.
/// </summary>
/// <param name="value">The setting to classify.</param>
/// <returns>Returns with the band the value belongs to.</returns>
int RangedGameOption::In_Range_Of(unsigned int value)
{
	if (value <= Range0) {
		return(WDT_GAME_OPT_RANGE_VERY_LOW);
	}
	if (value <= Range1) {
		return(WDT_GAME_OPT_RANGE_LOW);
	}
	if (value >= Range3) {
		return(WDT_GAME_OPT_RANGE_VERY_HIGH);
	}
	if (value >= Range2) {
		return(WDT_GAME_OPT_RANGE_HIGH);
	}
	return(WDT_GAME_OPT_RANGE_MIDDLE);
}


/// <summary>
/// Determines how prominently this ranged option should be listed for the territory.
/// A setting sitting in the middle of its range is unremarkable and is treated the same
/// as one the player is free to change.
/// </summary>
/// <returns>Returns with the display priority this option has earned.</returns>
int RangedGameOption::Get_Display_Priority(const WDTTerritory * territory)
{
	if (Is_User_Modifiable(territory) || Get_In_Range_Of(territory) == WDT_GAME_OPT_RANGE_MIDDLE) {
		return(WDT_GAME_OPT_USER_MOD);
	}
	return(WDT_GAME_OPT_PRIORITY);
}


/// <summary>
/// Determines how prominently this flag should be listed for the territory.
/// The option carries its own priority for each of its two states, so a rule that is
/// switched on can be shown more urgently than one that is switched off.
/// </summary>
/// <returns>Returns with the display priority this option has earned.</returns>
int FlagGameOption::Get_Display_Priority(const WDTTerritory * territory)
{
	if (Is_User_Modifiable(territory)) {
		return(WDT_GAME_OPT_USER_MOD);
	}
	if (territory->Booleans & Bitmask) {
		return(OnValue);
	}
	return(OffValue);
}


/// <summary>
/// Appends the on or off wording of this flag to the display string.
/// A flag whose current state has no wording assigned to it stays silent.
/// </summary>
/// <param name="str">The string to append the description to.</param>
/// <param name="len">The maximum number of characters that may be appended.</param>
void FlagGameOption::Get_String(const WDTTerritory * territory, char * str, int len)
{
	int stringid = (territory->Booleans & Bitmask) ? OnString : OffString;
	if (stringid != TXT_NONE) {
		WDT_Game_Option_Append_Comma(str, len);
		strncat(str, Fetch_String(stringid), len);
	}
}


/// <summary>
/// Determines how prominently the player count should be listed for this territory.
/// A count the player is stuck with matters more than one he may change himself.
/// </summary>
/// <returns>Returns with the display priority this option has earned.</returns>
int NumberOfPlayersGameOption::Get_Display_Priority(const WDTTerritory * territory)
{
	if (Is_User_Modifiable(territory)) {
		return(WDT_GAME_OPT_USER_MOD);
	}
	return(WDT_GAME_OPT_PRIORITY);
}


/// <summary>
/// Appends the player count restriction of the territory to the display string.
/// Nothing is added when the player is free to pick the number of players for himself.
/// </summary>
/// <param name="str">The string to append the description to.</param>
/// <param name="len">The maximum number of characters that may be appended.</param>
void NumberOfPlayersGameOption::Get_String(const WDTTerritory * territory, char * str, int len)
{
	if (!Is_User_Modifiable(territory)) {
		int stringid = territory->NumPlayers == 4 ? TXT_WDT_FOUR_PLAYER_ONLY : TXT_WDT_TWO_PLAYER_ONLY;
		if (stringid != TXT_NONE) {
			WDT_Game_Option_Append_Comma(str, len);
			strncat(str, Fetch_String(stringid), len);
		}
	}
}


/// <summary>
/// Determines how prominently the map size should be listed for this territory.
/// An unusually shaped battlefield is worth calling out, while an ordinary one can be
/// left further down the summary.
/// </summary>
/// <returns>Returns with the display priority this option has earned.</returns>
int MapSizeGameOption::Get_Display_Priority(const WDTTerritory * territory)
{
	static int _table[4][4] = {
		{ WDT_GAME_OPT_PRIORITY, WDT_GAME_OPT_PRIORITY,	 WDT_GAME_OPT_PRIORITY,	 WDT_GAME_OPT_PRIORITY },
		{ WDT_GAME_OPT_PRIORITY, WDT_GAME_OPT_NORMAL,	 WDT_GAME_OPT_USER_MOD,	 WDT_GAME_OPT_PRIORITY },
		{ WDT_GAME_OPT_PRIORITY, WDT_GAME_OPT_USER_MOD,	 WDT_GAME_OPT_NORMAL,	 WDT_GAME_OPT_PRIORITY },
		{ WDT_GAME_OPT_PRIORITY, WDT_GAME_OPT_PRIORITY,	 WDT_GAME_OPT_PRIORITY,	 WDT_GAME_OPT_PRIORITY }
	};

	if (Is_User_Modifiable(territory)) {
		return(WDT_GAME_OPT_USER_MOD);
	}
	return(_table[territory->Width][territory->Height]);
}


/// <summary>
/// Appends a description of the territory's map size to the display string.
/// This routine is used while building the summary of a territory so that the player can
/// see at a glance what shape of battlefield is being fought over.
/// </summary>
/// <param name="str">The string to append the description to.</param>
/// <param name="len">The maximum number of characters that may be appended.</param>
void MapSizeGameOption::Get_String(const WDTTerritory * territory, char * str, int len)
{
	static int _table[4][4] = {
		/// "Tiny Map" "Small Map" "Narrow Map" "Tall & Narrow Map"
		{ TXT_WDT_MAP_TINY,	TXT_WDT_MAP_SMALL,	TXT_WDT_MAP_NARROW,	TXT_WDT_MAP_TALL_NARROW },
		/// "Small Map" "Small Map" "" "Narrow Map"
		{ TXT_WDT_MAP_SMALL,	TXT_WDT_MAP_SMALL,	TXT_NONE,	TXT_WDT_MAP_NARROW },
		/// "Short Map" "" "Large Map" "Large Map"
		{ TXT_WDT_MAP_SHORT,	TXT_NONE,	TXT_WDT_MAP_LARGE,	TXT_WDT_MAP_LARGE },
		/// "Short & Wide Map" "Short Map" "Large Map" "Huge Map"
		{ TXT_WDT_MAP_SHORT_WIDE,	TXT_WDT_MAP_SHORT,	TXT_WDT_MAP_LARGE,	TXT_WDT_MAP_HUGE }
	};

	int stringid = _table[territory->Width][territory->Height];
	if (stringid != TXT_NONE) {
		WDT_Game_Option_Append_Comma(str, len);
		strncat(str, Fetch_String(stringid), len);
	}
}
