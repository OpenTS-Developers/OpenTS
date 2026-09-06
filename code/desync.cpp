/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "desync.h"


void DesyncClass::Begin(std::int64_t now)
{
	for (int house = 0; house < MAX_PLAYERS; house++) {
		LastHeard[house] = now;
		HasLeft[house] = false;
		LeftName[house].clear();
	}
	LastHeartbeat = now - HEARTBEAT_INTERVAL_MS;
}


void DesyncClass::Heard(int house, std::int64_t now)
{
	if (Is_House(house)) {
		LastHeard[house] = now;
	}
}


bool DesyncClass::Is_Silent(int house, std::int64_t now) const
{
	return(Is_House(house) && now - LastHeard[house] > HEARTBEAT_TIMEOUT_MS);
}


bool DesyncClass::Heartbeat_Is_Due(std::int64_t now) const
{
	return(now - LastHeartbeat >= HEARTBEAT_INTERVAL_MS);
}


void DesyncClass::Heartbeat_Sent(std::int64_t now)
{
	LastHeartbeat = now;
}


void DesyncClass::Mark_Left(int house, char const * name)
{
	if (!Is_House(house)) {
		return;
	}
	HasLeft[house] = true;
	if (name != NULL && name[0] != '\0') {
		LeftName[house] = name;
	}
}


bool DesyncClass::Has_Left(int house) const
{
	return(Is_House(house) && HasLeft[house]);
}


char const * DesyncClass::Left_Name(int house) const
{
	return(Is_House(house) ? LeftName[house].c_str() : "");
}


bool DesyncClass::Is_House(int house)
{
	return(house >= 0 && house < MAX_PLAYERS);
}
