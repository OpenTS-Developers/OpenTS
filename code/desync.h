/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "sun.h"

#include <cstdint>
#include <string>

/*
 * Bookkeeping for the out-of-sync dialog: who has been heard from and who has left. Times
 * are milliseconds handed in, so it holds no engine state.
 */
class DesyncClass
{
	public:

		static constexpr std::int64_t HEARTBEAT_INTERVAL_MS = 1000;
		static constexpr std::int64_t HEARTBEAT_TIMEOUT_MS = 25000;
		static constexpr std::int64_t QUIT_DELAY_MS = 10000;

		// Every house counts as heard when the dialog opens, and the first heartbeat is due at once.
		void Begin(std::int64_t now);

		void Heard(int house, std::int64_t now);
		bool Is_Silent(int house, std::int64_t now) const;

		bool Heartbeat_Is_Due(std::int64_t now) const;
		void Heartbeat_Sent(std::int64_t now);

		// The name is kept because the computer renames a house it takes over.
		void Mark_Left(int house, char const * name);
		bool Has_Left(int house) const;
		char const * Left_Name(int house) const;

	private:

		static bool Is_House(int house);

		std::int64_t LastHeard[MAX_PLAYERS] = {};
		std::int64_t LastHeartbeat = 0;
		bool HasLeft[MAX_PLAYERS] = {};
		std::string LeftName[MAX_PLAYERS];
};
