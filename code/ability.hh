/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstring>


/// Names derived from AbilityName

enum AbilityType {
	ABILITY_NONE = -1,

	ABILITY_FASTER,
	ABILITY_STRONGER,
	ABILITY_FIREPOWER,
	ABILITY_SCATTER,
	ABILITY_ROF,
	ABILITY_SIGHT,
	ABILITY_CLOAK,
	ABILITY_TIBERIUM_PROOF,
	ABILITY_VEIN_PROOF,
	ABILITY_SELF_HEAL,
	ABILITY_EXPLODES,
	ABILITY_RADAR_INVISIBLE,
	ABILITY_SENSORS,
	ABILITY_FEARLESS,
	ABILITY_C4,
	ABILITY_TIBERIUM_HEAL,
	ABILITY_GUARD_AREA,
	ABILITY_CRUSHER,

	ABILITY_COUNT,
	ABILITY_FIRST = 0
};


struct AbilityFlagsType {
	public:
		AbilityFlagsType()
		{
			/// Init all flags to false.
			for (int type = ABILITY_FIRST; type < ABILITY_COUNT; type++) {
				AbilitiesFlag[type] = false;
			}
		}

		AbilityFlagsType & operator=(AbilityFlagsType const & that)
		{
			if (this != &that) {
				memcpy(this, &that, sizeof(*this));
			}
			return(*this);
		}

		bool const & operator[](AbilityType type) const { return(AbilitiesFlag[type]); }
		bool & operator[](AbilityType type) { return(AbilitiesFlag[type]); }

		// Carries the ability set to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(AbilitiesFlag);
		}

	private:
		/*
		 * These are the abilities present in this set, one flag per AbilityType. A unit type
		 * keeps one such set for the abilities its veterans earn and another for its elites.
		 */
		bool AbilitiesFlag[ABILITY_COUNT];
};
