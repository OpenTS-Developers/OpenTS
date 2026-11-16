/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Checks that simulation value types that need no game data start in a defined state after
// default construction, so that a save or a checksum never reads memory nothing wrote.

#include "tmission.h"

#include "doctest/doctest.h"

TEST_CASE("A default team mission is no mission with a zero argument")
{
	TeamMissionClass mission;
	CHECK(mission.Mission == TMISSION_NONE);
	CHECK(mission.Data.Value == 0);

	TeamMissionClass list[4];
	for (TeamMissionClass const & entry : list) {
		CHECK(entry.Mission == TMISSION_NONE);
		CHECK(entry.Data.Value == 0);
	}
}
