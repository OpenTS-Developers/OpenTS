/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "spawnhouse.h"
#include "win.h"

#include <cstring>

static_assert(HOUSE_SPAWN_LAST - HOUSE_SPAWN_FIRST + 1 == SPAWN_HOUSE_COUNT, "one Spawn alias per start position");
static_assert(HOUSE_PLAYER_AT_LAST - HOUSE_PLAYER_AT_FIRST + 1 == SPAWN_HOUSE_COUNT, "one Player @ alias per start position");

namespace {

constexpr char const * const SpawnNames[SPAWN_HOUSE_COUNT] = {
	"Spawn1", "Spawn2", "Spawn3", "Spawn4", "Spawn5", "Spawn6", "Spawn7", "Spawn8"
};

constexpr char const * const PlayerAtNames[SPAWN_HOUSE_COUNT] = {
	"<Player @ A>", "<Player @ B>", "<Player @ C>", "<Player @ D>",
	"<Player @ E>", "<Player @ F>", "<Player @ G>", "<Player @ H>"
};

}


int Spawn_House_Waypoint(char const * name)
{
	if (name == nullptr) {
		return -1;
	}

	for (int spawn_waypoint = 0; spawn_waypoint < SPAWN_HOUSE_COUNT; spawn_waypoint++) {
		if (_stricmp(name, SpawnNames[spawn_waypoint]) == 0 || _stricmp(name, PlayerAtNames[spawn_waypoint]) == 0) {
			return spawn_waypoint;
		}
	}
	return -1;
}


int Spawn_House_Waypoint(HousesType house)
{
	if (house >= HOUSE_SPAWN_FIRST && house <= HOUSE_SPAWN_LAST) {
		return house - HOUSE_SPAWN_FIRST;
	}
	if (house >= HOUSE_PLAYER_AT_FIRST && house <= HOUSE_PLAYER_AT_LAST) {
		return house - HOUSE_PLAYER_AT_FIRST;
	}
	return -1;
}


HousesType Spawn_House_Type(int spawn_waypoint)
{
	if (spawn_waypoint < 0 || spawn_waypoint >= SPAWN_HOUSE_COUNT) {
		return HOUSE_NONE;
	}
	return static_cast<HousesType>(HOUSE_SPAWN_FIRST + spawn_waypoint);
}


char const * Spawn_House_Name(int spawn_waypoint)
{
	if (spawn_waypoint < 0 || spawn_waypoint >= SPAWN_HOUSE_COUNT) {
		return nullptr;
	}
	return SpawnNames[spawn_waypoint];
}
