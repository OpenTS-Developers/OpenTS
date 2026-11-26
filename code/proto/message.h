/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "proto/wire.h"

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

// What the message structs of launch.proto and replay.proto share: the fields a decoder keeps
// without reading, the limits both schemas set, and the rule a message broke.

namespace ProtoMessage
{
	// A field kept as read: unknown to this codec, of another wire type, or engine-private.
	struct UnknownField {
		std::uint32_t Number = 0;
		ProtoWire::Bytes Encoded;    // tag and value, as read
	};

	using UnknownFields = std::vector<UnknownField>;

	// The limits on a message's contents that both schemas set.
	struct LimitsType {
		std::size_t String = 1024;
		std::size_t Players = 64;
	};

	struct DecodeContext;

	enum class RuleError : std::uint8_t {
		NONE,
		TOO_LONG,          // a string or bytes value is longer than its limit
		TOO_MANY,          // a repeated field has more entries than its limit
		NOT_ASCENDING,     // a list the format orders is out of order or repeats a value
		INVALID,           // the values break a rule the schema sets for them
	};

	// A broken rule and the field that broke it, given as the field's message in the schema and
	// its number.
	struct RuleBreak {
		RuleError Error = RuleError::NONE;
		std::string_view Message;    // such as "Frame"; empty when Error is NONE
		std::uint32_t Field = 0;

		bool operator==(RuleBreak const &) const = default;

		bool operator==(RuleError error) const
		{
			return(Error == error);
		}
	};

	// Returns the canonical encoding of a message whose namespace declares Encode for it.
	template<typename T>
	ProtoWire::Bytes Encoded(T const & message)
	{
		ProtoWire::Bytes out;
		Encode(message, out);
		return(out);
	}
}
