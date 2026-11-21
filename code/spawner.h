/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#pragma once

#include "spawnerconfig.h"

#include <optional>
#include <string>

namespace Launch::Message {
	struct Request;
}


namespace Spawner
{
	// file's Describe_Fault names a later fault in the request by its SPAWN.INI section and key.
	bool Read_Request(Launch::Message::Request & request, std::optional<Config> & file, std::string & fault);
}
