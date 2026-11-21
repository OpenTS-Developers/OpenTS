/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstdint>

// The field numbers of the messages that describe a game, one namespace per message. Each
// number must match docs/replay/replay.proto, since the numbers are the wire format.

namespace Launch::Field
{
	namespace Game
	{
		constexpr std::uint32_t KIND = 1;
		constexpr std::uint32_t SEED = 2;
		constexpr std::uint32_t PLAYERS = 3;
		constexpr std::uint32_t OPTIONS = 4;
		constexpr std::uint32_t ANONYMOUS = 5;
	}

	namespace Player
	{
		constexpr std::uint32_t HOUSE = 1;
		constexpr std::uint32_t NAME = 2;
		constexpr std::uint32_t HUMAN = 3;
		constexpr std::uint32_t OBSERVER = 4;
		constexpr std::uint32_t COUNTRY = 5;
		constexpr std::uint32_t COUNTRY_NAME = 6;
		constexpr std::uint32_t COLOR = 7;
		constexpr std::uint32_t START = 8;
		constexpr std::uint32_t AI_LEVEL = 9;
		constexpr std::uint32_t ALLIES = 10;
		constexpr std::uint32_t RESOLVED_COUNTRY = 11;
		constexpr std::uint32_t RESOLVED_COLOR = 12;
		constexpr std::uint32_t RESOLVED_START = 13;
	}

	namespace Option
	{
		constexpr std::uint32_t KEY = 1;
		constexpr std::uint32_t NUMBER = 2;
		constexpr std::uint32_t FLAG = 3;
		constexpr std::uint32_t TEXT = 4;
	}

	namespace FileHash
	{
		constexpr std::uint32_t PATH = 1;
		constexpr std::uint32_t HASH = 2;
	}
}
