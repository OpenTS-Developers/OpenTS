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


namespace NetGlobal
{
	enum class DecodeError
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


	struct ValidationContext
	{
		bool SenderIsMember = false;
		int SenderPlayerID = -1;
		int SenderPlayerColor = -1;
		std::array<bool, MAX_PLAYERS> ActivePlayers = {};
	};


	struct RejectionRecord
	{
		std::uint32_t Count = 0;
		bool ShouldLog = false;
	};


	class RejectionCounters
	{
		public:
			RejectionRecord Record(DecodeError error) noexcept;
			std::uint32_t Count(DecodeError error) const noexcept;

		private:
			std::array<std::uint32_t, static_cast<std::size_t>(DecodeError::COUNT)> Counts = {};
	};


	void Initialize_Packet(GlobalPacketType & packet, NetCommandType command) noexcept;

	DecodeError Validate_In_Game_Packet(GlobalPacketType const & packet, std::size_t packet_length, ValidationContext const & context);

	char const * Error_Name(DecodeError error) noexcept;
}
