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


enum class NetPacketEncoding
{
	UNCOMPRESSED,
	COMPRESSED,
};


enum class NetPacketDecodeError
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


constexpr std::uint8_t NET_PACKET_NO_EVENT_TYPE = UINT8_MAX;


struct NetPacketDecodeFailure
{
	NetPacketDecodeError Code = NetPacketDecodeError::NONE;
	std::size_t Offset = 0;
	std::uint8_t EventType = NET_PACKET_NO_EVENT_TYPE;
};


struct NetDecodedEvent
{
	NetDecodedEvent(void) noexcept;
	NetDecodedEvent(EventClass const & event, std::vector<std::byte> add_player_data) noexcept;
	NetDecodedEvent(NetDecodedEvent const & other);
	NetDecodedEvent(NetDecodedEvent && other) noexcept;
	NetDecodedEvent & operator=(NetDecodedEvent const & other);
	NetDecodedEvent & operator=(NetDecodedEvent && other) noexcept;

	EventClass Event;
	std::vector<std::byte> AddPlayerData;

	private:
		void Bind_AddPlayer_Data(void) noexcept;
};


struct NetPacketDecodeResult
{
	bool Succeeded(void) const noexcept;

	NetPacketDecodeFailure Failure;
	EventClass Envelope;
	bool HasEnvelope = false;
	std::vector<NetDecodedEvent> Events;
};


NetPacketDecodeResult Decode_Event_Packet(std::span<std::byte const> packet, NetPacketEncoding encoding, int expected_sender);

char const * Net_Packet_Error_Name(NetPacketDecodeError error) noexcept;
