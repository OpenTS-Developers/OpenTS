/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "event.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>


namespace NetPacket
{
	enum class Encoding
	{
		UNCOMPRESSED,
		COMPRESSED,
	};


	enum class DecodeError
	{
		NONE,
		EMPTY_PACKET,
		INVALID_EVENT_TYPE,
		INVALID_PREFIX,
		TRUNCATED_ENVELOPE,
		FRAMESYNC_NOT_ALONE,
		NESTED_ENVELOPE,
		SENDER_MISMATCH,
		INVALID_EVENT_LENGTH,
		TRUNCATED_EVENT,
		ZERO_MEGAMISSION_COUNT,
		TRUNCATED_MEGAMISSION,
		TRUNCATED_ADDPLAYER,
		TRAILING_BYTES,
		INVALID_CONNECTION,
		COUNT,
	};


	constexpr std::uint8_t NO_EVENT_TYPE = UINT8_MAX;


	struct DecodeFailure
	{
		DecodeError Code = DecodeError::NONE;
		std::size_t Offset = 0;
		std::uint8_t EventType = NO_EVENT_TYPE;
	};


	struct DecodedEvent
	{
		DecodedEvent(void) noexcept;
		DecodedEvent(EventClass const & event, std::vector<std::byte> add_player_data) noexcept;
		DecodedEvent(DecodedEvent const & other);
		DecodedEvent(DecodedEvent && other) noexcept;
		DecodedEvent & operator=(DecodedEvent const & other);
		DecodedEvent & operator=(DecodedEvent && other) noexcept;

		EventClass Event;
		std::vector<std::byte> AddPlayerData;

		private:
			void Bind_AddPlayer_Data(void) noexcept;
	};


	struct DecodeResult
	{
		bool Succeeded(void) const noexcept;

		DecodeFailure Failure;
		EventClass Envelope;
		bool HasEnvelope = false;
		std::vector<DecodedEvent> Events;
	};


	DecodeResult Decode_Event_Packet(std::span<std::byte const> packet, Encoding encoding, int expected_sender);

	char const * Error_Name(DecodeError error) noexcept;
}
