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
#include <vector>

// What message structs share: the fields a decoder keeps without reading, the limits a decoder
// applies, and the limit a message broke.

namespace ProtoMessage
{
	// A field kept as read: unknown to this codec, of another wire type, or engine-private.
	struct UnknownField {
		std::uint32_t Number = 0;
		ProtoWire::Bytes Encoded;    // tag and value, as read
	};

	using UnknownFields = std::vector<UnknownField>;

	// The limits on a message's contents.
	struct LimitsType {
		std::size_t EventsPerRecord = 16384;
		std::size_t EventPayload = 4096;
		std::size_t String = 1024;
		std::size_t Players = 64;
		std::size_t IncomeKinds = 4096;
	};

	enum class RuleError : std::uint8_t {
		NONE,
		STRING_TOO_LONG,
		TOO_MANY_PLAYERS,
		TOO_MANY_HOUSES,
		TOO_MANY_EVENTS,
		PAYLOAD_TOO_LARGE,
		NOT_ASCENDING,     // a list the format orders is out of order or repeats a value
		TOO_MANY_INCOME_KINDS,
		INCOME_KIND,       // two income kinds share a number or id, an id is empty, or a kind is not listed
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
