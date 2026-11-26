/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "always.h"

#include "spawner.h"

#include "ccfile.h"
#include "ccini.h"
#include "dbgprint.h"
#include "launch/gamelaunch.h"


/// <summary>
/// Logs each computer slot played at a difficulty other than the one the file asks for.
/// </summary>
static void Log_Handicaps(Spawner::Config const & config)
{
	for (int index = config.HumanCount; index < Spawner::Config::SLOT_COUNT; index++) {
		Spawner::Config::SlotType const & seat = config.Slots[index];
		if (seat.Occupancy != Spawner::Config::OccupancyType::Computer) {
			continue;
		}

		int played = Spawner::Config::Playable_Handicap(seat.Handicap);
		if (played != seat.Handicap) {
			DebugString("[Spawner] Seat %d asked for difficulty %d and is played at %d.\n",
				index + 1, seat.Handicap, played);
		}
	}
}


/// <summary>
/// Reads SPAWN.INI and writes the launch request it describes: a load request for a resume, and
/// a play request otherwise.
/// </summary>
/// <param name="file">Set to the file as read when SPAWN.INI exists, even when it gives no request; empty when it is missing.</param>
/// <param name="fault">Where to leave the sentence describing why the file gives no request.</param>
/// <returns>bool; Was a request written? The launcher still checks it against the request rules.</returns>
bool Spawner::Read_Request(Launch::Message::Request & request, std::optional<Config> & file, std::string & fault)
{
	file.reset();

	CCFileClass source("SPAWN.INI");
	if (!source.Is_Available()) {
		return(Launch::Fault(fault, "SPAWN.INI is missing. The game was started with -SPAWN, which needs it."));
	}

	CCINIClass ini;
	ini.Load(source, false);
	Config & config = file.emplace();
	config.Read_INI(ini);

	if (!config.Is_Launchable(fault)) {
		return(false);
	}

	config.To_Request(request);
	if (std::holds_alternative<Launch::Message::Play>(request.Verb)) {
		Log_Handicaps(config);
	}

	DebugString("[Spawner] Read SPAWN.INI.\n");
	return(true);
}
