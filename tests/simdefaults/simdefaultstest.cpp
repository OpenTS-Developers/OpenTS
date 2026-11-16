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

#include "event.h"
#include "target.h"
#include "tmission.h"

#include "doctest/doctest.h"

#include <algorithm>
#include <cstring>
#include <new>

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

TEST_CASE("An empty target has a zero ID")
{
	alignas(TargetClass) unsigned char storage[sizeof(TargetClass)];
	std::memset(storage, 0xCD, sizeof(storage));
	TargetClass * target = new (storage) TargetClass;
	CHECK(!target->Is_Valid());
	CHECK(target->Value() == 0);

	target->Decode(0x01123456);
	target->Invalidate();
	CHECK(target->Value() == 0);
}

TEST_CASE("A default event is empty and zero in every byte")
{
	alignas(EventClass) unsigned char storage[sizeof(EventClass)];
	std::memset(storage, 0xCD, sizeof(storage));
	EventClass * event = new (storage) EventClass;
	CHECK(event->Type == EventClass::EMPTY);
	CHECK(std::all_of(storage, storage + sizeof(storage), [](unsigned char byte) {return(byte == 0);}));
}
