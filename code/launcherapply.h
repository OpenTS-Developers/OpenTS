/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#pragma once

#include "launch/gamelaunch.h"

#include <optional>
#include <string>


namespace Launch
{
	// The setup a running game was launched with, both parts as Launch::Apply applied them.
	struct SetupType {
		Launch::Game Game;
		Launch::Machine Machine;
	};

	// Empty until Launch::Apply succeeds.
	extern std::optional<SetupType> Setup;

	Game::LimitsType Limits(void);

	bool Wire_Network(Machine const & machine, std::string & fault);

	bool Apply(Game const & game, Machine const & machine, std::string & fault);

	// Call Prepare_Side_Roster before either, since the players' countries are judged against it.
	bool Judge_Load(Message::Load const & request, std::string & fault, FaultSite * site = nullptr);
	bool Apply_Load(Message::Load const & request, bool & gameloaded, std::string & fault);
}
