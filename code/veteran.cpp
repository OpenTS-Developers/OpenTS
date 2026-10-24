/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

/* $Header: /CounterStrike/CREW.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CREW.CPP                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 23, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 23, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "veteran.h"

#include "_rules.h"
#include "globals.h"
#include "rules.h"

#include <algorithm>

static const char *AbilityName[ABILITY_COUNT] = {
	"FASTER",
	"STRONGER",
	"FIREPOWER",
	"SCATTER",
	"ROF",
	"SIGHT",
	"CLOAK",
	"TIBERIUM_PROOF",
	"VEIN_PROOF",
	"SELF_HEAL",
	"EXPLODES",
	"RADAR_INVISIBLE",
	"SENSORS",
	"FEARLESS",
	"C4",
	"TIBERIUM_HEAL",
	"GUARD_AREA",
	"CRUSHER"
};


/// <summary>
/// Converts an ability name into its ability type.
/// This routine is used when reading the veteran and elite ability lists out of the
/// rules, where the abilities are spelled out by name. The comparison ignores case.
/// </summary>
/// <param name="name">The ability name to look up.</param>
/// <returns>Returns with the ability type that matches. If the name is not recognized,
/// ABILITY_NONE is returned.</returns>
AbilityType Ability_From_Name(const char *name)
{
	for (int ability = ABILITY_FIRST; ability < ABILITY_COUNT; ability++) {
		if (stricmp(AbilityName[ability], name) == 0) {
			return((AbilityType)ability);
		}
	}
	return(ABILITY_NONE);
}


/// <summary>
/// Constructor for the veterancy tracker.
/// Every unit begins its career as a rookie with no experience to its name.
/// </summary>
VeterancyClass::VeterancyClass(PromotedCallback callback) :
	On_Unit_Promoted_Callback(std::move(callback)),
	Experience(0)
{
}


/// <summary>
/// Destructor for the veterancy tracker.
/// </summary>
VeterancyClass::~VeterancyClass(void)
{
}


/// <summary>
/// Sets the experience value, up to the maximum allowed experience set by VeteranCap.
/// If this causes the unit to be promoted, produces an event call to handle changes as a result of the promotion.
/// </summary>
/// <param name="value">The amount of experience to set. Cannot exceed the maximum allowed experience set by VeteranCap.
/// <param name="notify_player">Passed to the callback to determine whether the player should be notified on the promotion. This
/// is relevant in cases where promotion occurs artificially, such as spawning an elite unit via triggers. Defaults to true.
void VeterancyClass::Set_Experience(double value, bool notify_player)
{
	auto old_rank = Get_Rank();

	Experience = std::min(value, Rule->VeteranCap);

	auto new_rank = Get_Rank();

	if (new_rank > old_rank) {
		On_Unit_Promoted_Callback(notify_player);
	}
}


/// <summary>
/// Credits the unit with a kill.
/// This routine is called when a trainable unit destroys something. The experience
/// earned is weighed against what the killer itself is worth, so cheap units are
/// promoted by prey that would barely register for an expensive one. Experience is
/// capped at the ceiling the rules specify.
/// </summary>
/// <param name="cost">The cost of the unit that made the kill.</param>
/// <param name="value">The point value of the victim.</param>
void VeterancyClass::Made_A_Kill(int cost, int value)
{
	double experience_earned = value / (cost * Rule->VeteranRatio);	
	Set_Experience(Experience + experience_earned);
}


/// <summary>
/// Has the unit earned veteran status?
/// This is the middle rank, one promotion above rookie and one below elite.
/// </summary>
/// <returns>bool; Is the unit a veteran?</returns>
/// <remarks>The ranks are exclusive. An elite unit answers false here, so a caller that
/// wants "veteran or better" must ask about elite as well.</remarks>
bool VeterancyClass::Is_Veteran(void) const
{
	return(Experience >= 1 && Experience < 2);
}


/// <summary>
/// Is the unit still a rookie?
/// This is the rank a unit is built at, and it holds until enough kills have been
/// accumulated to earn the first promotion.
/// </summary>
/// <returns>bool; Is the unit a rookie?</returns>
bool VeterancyClass::Is_Rookie(void) const
{
	return(Experience >= 0 && Experience < 1);
}


/// <summary>
/// Is the unit a raw recruit?
/// A dumbass sits below rookie on the promotion ladder, its negative experience making
/// it perform worse than a unit that has never seen action at all.
/// </summary>
/// <returns>bool; Is the unit a dumbass?</returns>
bool VeterancyClass::Is_Dumbass(void) const
{
	return(Experience < 0);
}


/// <summary>
/// Has the unit reached elite status?
/// This is the top of the promotion ladder. Combat code asks this when deciding whether
/// the unit's elite bonuses and abilities apply.
/// </summary>
/// <returns>bool; Is the unit elite?</returns>
bool VeterancyClass::Is_Elite(void) const
{
	return(Experience >= 2);
}


/// <summary>
/// Marks or unmarks the unit as a green recruit.
/// A dumbass has negative experience, which makes it perform worse than a rookie.
/// Clearing the mark restores it to plain rookie status.
/// </summary>
/// <param name="set">Should the unit be made a dumbass?</param>
void VeterancyClass::Set_Dumbass()
{
	Set_Experience(-.25);
}


/// <summary>
/// Returns the unit to rookie status.
/// This routine wipes out whatever experience the unit had accumulated, putting it back
/// where it started life.
/// </summary>
void VeterancyClass::Set_Rookie()
{
	Set_Experience(0);
}


/// <summary>
/// Promotes or demotes the unit to veteran status.
/// Clearing veteran status drops the unit back to rookie rather than to whatever it
/// happened to be before.
/// </summary>
/// <param name="notify_player">Should the player be notified on this promotion?</param>
void VeterancyClass::Set_Veteran(bool notify_player)
{
	Set_Experience(1, notify_player);
}


/// <summary>
/// Promotes or demotes the unit to elite status.
/// Clearing elite status does not merely undo the promotion -- it drops the unit all the
/// way back to rookie.
/// </summary>
/// <param name="notify_player">Should the player be notified on this promotion?</param>
void VeterancyClass::Set_Elite(bool notify_player)
{
	Set_Experience(2, notify_player);
}


/// <summary>
/// Converts the veterancy into a whole number.
/// This routine is used when the veterancy must be stored somewhere that cannot hold a
/// fractional value, such as a save game record or a scenario file entry.
/// </summary>
/// <returns>Returns with the veterancy expressed as a whole number.</returns>
int VeterancyClass::To_Integer(void) const
{
	return(int(Experience * 100));
}


/// <summary>
/// Sets the veterancy from its whole number form.
/// This is the counterpart to To_Integer, used when reading a unit's veterancy back out
/// of a save game or a scenario file.
/// This function is used mainly for spawning pre-placed units on the map with preset amount of experience, so it currently
/// set to never notify players of units being promoted from this.
/// </summary>
/// <param name="value">The stored veterancy value to restore.</param>
void VeterancyClass::From_Integer(int value)
{
	Set_Experience(value / 100.0, false);
}


/// <summary>
/// Helper functions for fetching the current veterancy rank.
/// </summary>
/// <returns>Returns a VeterancyRankType enum value that specifies the veterancy rank of the techno associated with this class instance.</returns>
VeterancyRankType VeterancyClass::Get_Rank() const
{
	if (Is_Elite()) {
		return RANK_ELITE;
	}
	if (Is_Veteran()) {
		return RANK_VETERAN;
	}
	if (Is_Rookie()) {
		return RANK_ROOKIE;
	}

	return RANK_DUMBASS;
}


/// <summary>
/// Getter for experience.
/// </summary>
/// <returns>Returns the current experience value.</returns>
double VeterancyClass::Get_Experience() const {
	return Experience;
}
