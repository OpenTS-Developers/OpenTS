/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "session.h"

#include <array>
#include <cstddef>
#include <cstdint>


constexpr std::size_t NET_GLOBAL_PACKET_SIZE = sizeof(GlobalPacketType);


enum class NetGlobalDecodeError
{
	NONE,
	INVALID_LENGTH,
	INVALID_COMMAND,
	SENDER_NOT_MEMBER,
	UNTERMINATED_NAME,
	UNTERMINATED_MESSAGE,
	INVALID_COLOR,
	INVALID_PROGRESS,
	INVALID_KICK_PLAYER,
	SELF_KICK,
	DUPLICATE_KICK_PROPOSAL,
	KICK_PROPOSAL_QUEUE_FULL,
	COUNT,
};


struct NetGlobalValidationContext
{
	bool SenderIsMember = false;
	int SenderPlayerID = -1;
	int SenderPlayerColor = -1;
	std::array<bool, MAX_PLAYERS> ActivePlayers = {};
};


struct NetGlobalRejectionRecord
{
	std::uint32_t Count = 0;
	bool ShouldLog = false;
};


class NetGlobalRejectionCounters
{
	public:
		NetGlobalRejectionRecord Record(NetGlobalDecodeError error) noexcept;
		std::uint32_t Count(NetGlobalDecodeError error) const noexcept;

	private:
		std::array<std::uint32_t, static_cast<std::size_t>(NetGlobalDecodeError::COUNT)> Counts = {};
};


void Initialize_Global_Packet(GlobalPacketType & packet, NetCommandType command) noexcept;

NetGlobalDecodeError Validate_In_Game_Global(GlobalPacketType const & packet, std::size_t packet_length, NetGlobalValidationContext const & context);

bool Net_Global_Command_Is_Public(NetCommandType command);

bool Net_Global_Command_Requires_Member(NetCommandType command);

char const * Net_Global_Error_Name(NetGlobalDecodeError error) noexcept;
