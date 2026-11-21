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
#include <optional>
#include <string>
#include <variant>
#include <vector>

// The messages that describe a game, as plain structs. Encode writes the canonical encoding,
// Decode accepts any valid one, and fields kept in Unknown are written back in field-number
// order.

namespace Launch
{
	using ProtoWire::Bytes;
	using ProtoWire::ByteSpan;
	using ProtoMessage::UnknownField;
	using ProtoMessage::UnknownFields;
	using ProtoMessage::LimitsType;
	using ProtoMessage::RuleError;
	using ProtoMessage::RuleBreak;
	using ProtoMessage::Encoded;

	// Open enumerations: a value outside the list is kept as it was read.
	enum class GameKind : std::int32_t {
		GAME_KIND_UNKNOWN = 0,
		GAME_KIND_CAMPAIGN = 1,
		GAME_KIND_SKIRMISH = 2,
		GAME_KIND_MULTIPLAYER = 3,
	};
}

// One struct per message, named as the schema names it, with the codec functions beside them
// so that argument-dependent lookup finds them.
namespace Launch::Message
{
	struct Player {
		std::uint32_t House = 0;
		std::string Name;
		bool Human = false;
		bool Observer = false;
		std::int32_t Country = 0;
		std::string CountryName;
		std::int32_t Color = 0;
		std::int32_t Start = 0;
		std::optional<std::int32_t> AILevel;
		std::vector<std::uint32_t> Allies;
		std::optional<std::int32_t> ResolvedCountry;
		std::optional<std::int32_t> ResolvedColor;
		std::optional<std::int32_t> ResolvedStart;
		UnknownFields Unknown;
	};

	struct Option {
		std::string Key;
		std::variant<std::monostate, std::int64_t, bool, std::string> Value;
		UnknownFields Unknown;
	};

	// The Game message: what every machine in a game shares. A replay stores one as LNCH.
	struct Game {
		GameKind Kind = GameKind::GAME_KIND_UNKNOWN;
		std::optional<std::uint32_t> Seed;
		std::vector<Player> Players;
		std::vector<Option> Options;
		bool Anonymous = false;
		UnknownFields Unknown;
	};

	struct FileHash {
		std::string Path;
		std::string Hash;
		UnknownFields Unknown;
	};

	// Appends the canonical encoding of a message to out.
	void Encode(Game const & message, Bytes & out);
	void Encode(Player const & message, Bytes & out);
	void Encode(Option const & message, Bytes & out);
	void Encode(FileHash const & message, Bytes & out);

	// Replaces message with the decoded bytes; false on malformed input, message partly filled.
	bool Decode(ByteSpan data, Game & message);
	bool Decode(ByteSpan data, Player & message);
	bool Decode(ByteSpan data, Option & message);
	bool Decode(ByteSpan data, FileHash & message);

	// As Decode, but false also on a broken limit, which broken names (NONE when malformed).
	bool Decode(ByteSpan data, Game & message, LimitsType const & limits, RuleBreak & broken);

	// Returns the first of the limits that message breaks, or NONE.
	RuleBreak Check_Limits(Game const & message, LimitsType const & limits);

	// Returns NOT_ASCENDING when a list in message is out of the order the format sets, or NONE.
	RuleBreak Check_Order(Game const & message);

	// For the codec of another namespace whose messages nest these.
	bool Read_Message(ByteSpan data, Game & message, ProtoMessage::DecodeContext * context);
	bool Read_Message(ByteSpan data, Player & message, ProtoMessage::DecodeContext * context);
	bool Read_Message(ByteSpan data, Option & message, ProtoMessage::DecodeContext * context);
	bool Read_Message(ByteSpan data, FileHash & message, ProtoMessage::DecodeContext * context);
}

namespace Launch
{
	using Message::Encode;
	using Message::Decode;
	using Message::Check_Limits;
	using Message::Check_Order;
}
