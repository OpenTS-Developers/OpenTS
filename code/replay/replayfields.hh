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

// The field numbers of docs/replay/replay.proto, one namespace per message. Each number must
// match the schema, since the numbers are the wire format.

namespace Replay::Field
{
	namespace RecordingInfo
	{
		constexpr std::uint32_t ENGINE = 1;
		constexpr std::uint32_t ENGINE_VERSION = 2;
		constexpr std::uint32_t DATA_VERSION = 3;
		constexpr std::uint32_t PACKAGE = 4;
		constexpr std::uint32_t PACKAGE_VERSION = 5;
		constexpr std::uint32_t FILES = 6;
		constexpr std::uint32_t RECORDED_AT = 7;
		constexpr std::uint32_t RECORDER = 8;
		constexpr std::uint32_t MAP_NAME = 9;
		constexpr std::uint32_t MAP_PATH = 10;
		constexpr std::uint32_t MAP_SHA256 = 11;
		constexpr std::uint32_t GAME_MODE = 12;
		constexpr std::uint32_t SEAL_VERSION = 13;
		constexpr std::uint32_t COLORS = 14;
		constexpr std::uint32_t INCOME_KINDS = 15;
	}

	namespace PlayerColor
	{
		constexpr std::uint32_t NUMBER = 1;
		constexpr std::uint32_t RGB = 2;
	}

	namespace IncomeKind
	{
		constexpr std::uint32_t NUMBER = 1;
		constexpr std::uint32_t ID = 2;
	}

	namespace Frame
	{
		constexpr std::uint32_t SKIPPED = 1;
		constexpr std::uint32_t HASH = 2;
		constexpr std::uint32_t EVENTS = 3;
		constexpr std::uint32_t VIEW = 4;
		constexpr std::uint32_t MESSAGES = 5;
		constexpr std::uint32_t FRAME_MICROS = 6;
		constexpr std::uint32_t SAMPLES = 7;
		constexpr std::uint32_t INCOME = 8;
		constexpr std::uint32_t SEALS = 9;
		constexpr std::uint32_t DEFEATED = 10;
	}

	namespace Event
	{
		constexpr std::uint32_t TYPE = 1;
		constexpr std::uint32_t HOUSE = 2;
		constexpr std::uint32_t DUE = 3;
		constexpr std::uint32_t PAYLOAD = 4;
		constexpr std::uint32_t LOCAL = 5;
		constexpr std::uint32_t ROLE = 6;
		constexpr std::uint32_t SEALED_PAYLOAD = 7;
	}

	namespace ViewState
	{
		constexpr std::uint32_t CAMERA = 1;
		constexpr std::uint32_t SELECTION = 2;
	}

	namespace Point
	{
		constexpr std::uint32_t X = 1;
		constexpr std::uint32_t Y = 2;
	}

	namespace Selection
	{
		constexpr std::uint32_t IDS = 1;
	}

	namespace PlayerMessage
	{
		constexpr std::uint32_t HOUSE = 1;
		constexpr std::uint32_t SENDER = 2;
		constexpr std::uint32_t TEXT = 3;
		constexpr std::uint32_t COLOR = 4;
		constexpr std::uint32_t SCOPE = 5;
		constexpr std::uint32_t RECIPIENTS = 6;
		constexpr std::uint32_t SALT = 7;
	}

	namespace HouseSample
	{
		constexpr std::uint32_t HOUSE = 1;
		constexpr std::uint32_t CREDITS = 2;
		constexpr std::uint32_t STORED_VALUE = 3;
		constexpr std::uint32_t SPENT = 4;
		constexpr std::uint32_t EARNED = 5;
		constexpr std::uint32_t POWER_OUTPUT = 6;
		constexpr std::uint32_t POWER_DRAIN = 7;
		constexpr std::uint32_t VEHICLES = 8;
		constexpr std::uint32_t INFANTRY = 9;
		constexpr std::uint32_t AIRCRAFT = 10;
		constexpr std::uint32_t BUILDINGS = 11;
		constexpr std::uint32_t ARMY_VALUE = 12;
		constexpr std::uint32_t BUILDING_VALUE = 13;
		constexpr std::uint32_t UNITS_KILLED = 14;
		constexpr std::uint32_t BUILDINGS_KILLED = 15;
		constexpr std::uint32_t UNITS_LOST = 16;
		constexpr std::uint32_t BUILDINGS_LOST = 17;
		constexpr std::uint32_t UNITS_BUILT = 18;
		constexpr std::uint32_t BUILDINGS_BUILT = 19;
		constexpr std::uint32_t SCORE = 20;
		constexpr std::uint32_t DEFEATED = 21;
	}

	namespace Income
	{
		constexpr std::uint32_t HOUSE = 1;
		constexpr std::uint32_t KIND = 2;
		constexpr std::uint32_t AMOUNT = 3;
	}

	namespace Seal
	{
		constexpr std::uint32_t THROUGH = 1;
		constexpr std::uint32_t HASH = 2;
	}

	namespace Checkpoint
	{
		constexpr std::uint32_t FRAME = 1;
		constexpr std::uint32_t CRC32 = 2;
		constexpr std::uint32_t STATE = 3;
	}

	namespace Statistics
	{
		constexpr std::uint32_t TYPES = 1;
		constexpr std::uint32_t HOUSES = 2;
	}

	namespace ObjectType
	{
		constexpr std::uint32_t CATEGORY = 1;
		constexpr std::uint32_t INDEX = 2;
		constexpr std::uint32_t ID = 3;
		constexpr std::uint32_t NAME = 4;
		constexpr std::uint32_t COST = 5;
		constexpr std::uint32_t ICON = 6;
		constexpr std::uint32_t NAVAL = 7;
		constexpr std::uint32_t UNSCORED = 8;
		constexpr std::uint32_t INSIGNIFICANT = 9;
	}

	namespace HouseStatistics
	{
		constexpr std::uint32_t HOUSE = 1;
		constexpr std::uint32_t NAME = 2;
		constexpr std::uint32_t COUNTRY_NAME = 3;
		constexpr std::uint32_t COLOR = 4;
		constexpr std::uint32_t START = 5;
		constexpr std::uint32_t ALLIES = 6;
		constexpr std::uint32_t OUTCOME = 7;
		constexpr std::uint32_t LEAVE = 8;
		constexpr std::uint32_t FINAL = 9;
		constexpr std::uint32_t COUNTS = 10;
		constexpr std::uint32_t KILLS = 11;
		constexpr std::uint32_t CRATES = 12;
	}

	namespace TypeCount
	{
		constexpr std::uint32_t KIND = 1;
		constexpr std::uint32_t CATEGORY = 2;
		constexpr std::uint32_t INDEX = 3;
		constexpr std::uint32_t COUNT = 4;
	}

	namespace CrateCount
	{
		constexpr std::uint32_t ID = 1;
		constexpr std::uint32_t COUNT = 2;
	}

	namespace HouseKills
	{
		constexpr std::uint32_t HOUSE = 1;
		constexpr std::uint32_t UNITS = 2;
		constexpr std::uint32_t BUILDINGS = 3;
	}

	namespace RecordingSummary
	{
		constexpr std::uint32_t LAST_FRAME = 1;
		constexpr std::uint32_t ENDED_AT = 2;
		constexpr std::uint32_t DESYNC_FRAME = 3;
		constexpr std::uint32_t COMPLETED = 4;
		constexpr std::uint32_t SEALS = 5;
		constexpr std::uint32_t ELAPSED_MICROS = 6;
		constexpr std::uint32_t GAME_MICROS = 7;
	}
}
