/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "proto/message.h"

#include <cstddef>
#include <cstdint>

// Constants and enumerations of the replay file format that docs/replay/REPLAY-FORMAT.md
// defines. The enumerations keep the names replay.proto gives them.

namespace Replay
{
	constexpr std::uint32_t CONTAINER_VERSION = 1;
	constexpr std::size_t PREAMBLE_SIZE = 8;
	constexpr std::size_t SECTION_HEADER_SIZE = 24;

	// Both sizes of a section the writer has not finished.
	constexpr std::uint64_t OPEN_SIZE = 0xFFFFFFFFFFFFFFFFull;

	constexpr std::uint32_t SEAL_VERSION = 1;
	constexpr std::uint32_t SEAL_INTERVAL = 900;

	// The most frame records the writer adds between sync flushes of the frame stream.
	constexpr std::uint32_t FLUSH_RECORDS = 60;

	// A tag is four ASCII characters in file order, so "INFO" is 0x4F464E49.
	constexpr std::uint32_t Make_Tag(char a, char b, char c, char d)
	{
		return(std::uint32_t(std::uint8_t(a)) | (std::uint32_t(std::uint8_t(b)) << 8)
			| (std::uint32_t(std::uint8_t(c)) << 16) | (std::uint32_t(std::uint8_t(d)) << 24));
	}

	constexpr std::uint32_t MAGIC = Make_Tag('C', 'N', 'C', 'R');
	constexpr std::uint32_t TAG_INFO = Make_Tag('I', 'N', 'F', 'O');
	constexpr std::uint32_t TAG_LNCH = Make_Tag('L', 'N', 'C', 'H');
	constexpr std::uint32_t TAG_MAP = Make_Tag('M', 'A', 'P', ' ');
	constexpr std::uint32_t TAG_INIT = Make_Tag('I', 'N', 'I', 'T');
	constexpr std::uint32_t TAG_FRMS = Make_Tag('F', 'R', 'M', 'S');
	constexpr std::uint32_t TAG_CKPT = Make_Tag('C', 'K', 'P', 'T');
	constexpr std::uint32_t TAG_STAT = Make_Tag('S', 'T', 'A', 'T');
	constexpr std::uint32_t TAG_END = Make_Tag('E', 'N', 'D', ' ');

	enum class Codec : std::uint32_t {
		STORED = 0,
		DEFLATE = 1,
	};

	// The format's size limits: those of a message, and those of a section. Each reader has a
	// separate copy.
	struct LimitsType : ProtoMessage::LimitsType {
		std::size_t EventsPerFrame = 16384;
		std::size_t EventPayload = 4096;             // each payload and sealed_payload
		std::size_t IncomeKinds = 4096;
		std::uint64_t SmallSection = 1u << 20;       // INFO, LNCH, END
		std::uint64_t LargeSection = 32u << 20;      // MAP, INIT, STAT
		std::uint64_t Checkpoint = 64u << 20;
		std::uint64_t FrameRecord = 1u << 20;
	};

	// Open enumerations: a value outside the list is kept as it was read.
	enum class GameKind : std::int32_t {
		GAME_KIND_UNKNOWN = 0,
		GAME_KIND_CAMPAIGN = 1,
		GAME_KIND_SKIRMISH = 2,
		GAME_KIND_MULTIPLAYER = 3,
	};

	enum class EventRole : std::int32_t {
		EVENT_ROLE_NONE = 0,
		EVENT_ROLE_CHAT_PROOF = 1,
	};

	enum class MessageScope : std::int32_t {
		MESSAGE_SCOPE_UNKNOWN = 0,
		MESSAGE_SCOPE_ALL = 1,
		MESSAGE_SCOPE_TEAM = 2,
		MESSAGE_SCOPE_PRIVATE = 3,
		MESSAGE_SCOPE_OBSERVERS = 4,
	};

	enum class ObjectCategory : std::int32_t {
		OBJECT_CATEGORY_UNKNOWN = 0,
		OBJECT_CATEGORY_BUILDING = 1,
		OBJECT_CATEGORY_INFANTRY = 2,
		OBJECT_CATEGORY_VEHICLE = 3,
		OBJECT_CATEGORY_AIRCRAFT = 4,
	};

	enum class Outcome : std::int32_t {
		OUTCOME_UNKNOWN = 0,
		OUTCOME_WON = 1,
		OUTCOME_LOST = 2,
		OUTCOME_DRAW = 3,
	};

	enum class LeaveReason : std::int32_t {
		LEAVE_REASON_NONE = 0,
		LEAVE_REASON_RESIGNED = 1,
		LEAVE_REASON_DISCONNECTED = 2,
	};

	enum class CountKind : std::int32_t {
		COUNT_KIND_UNKNOWN = 0,
		COUNT_KIND_BUILT = 1,
		COUNT_KIND_KILLED = 2,
		COUNT_KIND_LOST = 3,
		COUNT_KIND_CAPTURED = 4,
		COUNT_KIND_REMAINING = 5,
	};
}
