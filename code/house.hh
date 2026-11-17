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

#pragma once


#define HOUSE_NAME_MAX	64

/**********************************************************************
**	The houses that can be played are listed here. Each has their own
**	personality and strengths.
*/
enum HousesType {
	HOUSE_NONE=-1,

	HOUSE_GOOD,					// Global Defense Initiative
	HOUSE_BAD,					// Brotherhood of Nod
	HOUSE_NEUTRAL,				// Civilians
	HOUSE_MUTANT,

	HOUSE_COUNT,
	HOUSE_FIRST=0,

	// Whoever starts at a numbered position, as Tiberian Sun patches and Yuri's Revenge each
	// number it; no house type is ever registered under these values.
	HOUSE_SPAWN_FIRST=50,
	HOUSE_SPAWN_LAST=57,
	HOUSE_PLAYER_AT_FIRST=4475,
	HOUSE_PLAYER_AT_LAST=4482
};


// What gave a house money or stored Tiberium.
enum IncomeType {
	INCOME_NONE=-1,					// Not income: Tiberium the house already had, moved out of one of its buildings.

	INCOME_HARVESTED,				// A harvester unloaded at a refinery.
	INCOME_BUILDING_CASH,			// A building paid its ProduceCashAmount.
	INCOME_BUILDING_STARTUP,		// A building paid its ProduceCashStartup to the house that captured it.
	INCOME_SOLD_BUILDING,			// A building was sold.
	INCOME_SOLD_UPGRADE,			// A building's upgrade was sold.
	INCOME_SOLD_UNIT,				// A unit was sold at a repair depot.
	INCOME_REFUND_PRODUCTION,		// Production was cancelled.
	INCOME_REFUND_UNDEPLOY,			// A sold building's unit could not be created or placed.
	INCOME_REFUND_FREE_UNIT,		// A building's free unit could not be created or placed.
	INCOME_CRATE,					// A crate gave money.
	INCOME_STOLEN_STORAGE,			// A captured building's stored Tiberium went to the capturing house.
	INCOME_STARTING_MULTIPLIER,		// A computer player's credit multiplier was applied at the start.
	INCOME_STARTING_CARRY_OVER,		// Money was brought over from the previous campaign mission.
	INCOME_STARTING_FILL_SILOS,		// FillSilos stocked the house's storage at the start.
	INCOME_TRIGGER,					// A trigger action gave credits.
	INCOME_DEBUG,					// A Debug build's money cheat.

	INCOME_COUNT,
	INCOME_FIRST=0
};
