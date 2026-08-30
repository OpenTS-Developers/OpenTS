/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "netglobal.h"

#include <cstring>
#include <limits>
#include <type_traits>


namespace {

static_assert(std::is_trivially_copyable_v<GlobalPacketType>);


/// <summary>Checks that a fixed wire string contains a terminator.</summary>
bool Has_Terminator(char const * text, std::size_t capacity)
{
	return(std::memchr(text, '\0', capacity) != NULL);
}


/// <summary>Checks a player index against the current session roster.</summary>
bool Is_Active_Player(NetGlobalValidationContext const & context, int player)
{
	return(player >= 0 && player < static_cast<int>(context.ActivePlayers.size()) && context.ActivePlayers[player]);
}

}	// namespace


/// <summary>Clears an outgoing packet before selecting its command.</summary>
void Initialize_Global_Packet(GlobalPacketType & packet, NetCommandType command) noexcept
{
	std::memset(&packet, 0, sizeof(packet));
	packet.Command = command;
}


/// <summary>Identifies public in-game discovery commands.</summary>
bool Net_Global_Command_Is_Public(NetCommandType command)
{
	return(command == NET_QUERY_GAME || command == NET_QUERY_PLAYER);
}


/// <summary>Identifies commands restricted to session members.</summary>
bool Net_Global_Command_Requires_Member(NetCommandType command)
{
	switch (command) {
		case NET_SIGN_OFF:
		case NET_MESSAGE:
		case NET_PROGRESS_REPORT:
		case NET_READY_TO_GO:
		case NET_PROPOSE_KICK:
			return(true);

		default:
			return(false);
	}
}


/// <summary>Validates an in-game global packet before dispatch.</summary>
NetGlobalDecodeError Validate_In_Game_Global(GlobalPacketType const & packet, std::size_t packet_length, NetGlobalValidationContext const & context)
{
	if (packet_length != NET_GLOBAL_PACKET_SIZE) {
		return(NetGlobalDecodeError::INVALID_LENGTH);
	}

	bool const is_public = Net_Global_Command_Is_Public(packet.Command);
	bool const requires_member = Net_Global_Command_Requires_Member(packet.Command);
	if (!is_public && !requires_member) {
		return(NetGlobalDecodeError::INVALID_COMMAND);
	}
	if (requires_member && !context.SenderIsMember) {
		return(NetGlobalDecodeError::SENDER_NOT_MEMBER);
	}

	switch (packet.Command) {
		case NET_QUERY_PLAYER:
			if (!Has_Terminator(packet.Name, sizeof(packet.Name))) {
				return(NetGlobalDecodeError::UNTERMINATED_NAME);
			}
			break;

		case NET_MESSAGE:
			if (!Has_Terminator(packet.Name, sizeof(packet.Name))) {
				return(NetGlobalDecodeError::UNTERMINATED_NAME);
			}
			if (!Has_Terminator(packet.Message.Buf, sizeof(packet.Message.Buf))) {
				return(NetGlobalDecodeError::UNTERMINATED_MESSAGE);
			}
			if (context.SenderPlayerColor < 0 || context.SenderPlayerColor >= MAX_MPLAYER_COLORS) {
				return(NetGlobalDecodeError::INVALID_COLOR);
			}
			break;

		case NET_PROGRESS_REPORT:
			if (packet.Progress.Percent < 0 || packet.Progress.Percent > 100) {
				return(NetGlobalDecodeError::INVALID_PROGRESS);
			}
			break;

		case NET_PROPOSE_KICK: {
			if (!Is_Active_Player(context, context.SenderPlayerID) || packet.Kick.KickeeID >= context.ActivePlayers.size()
				|| !context.ActivePlayers[packet.Kick.KickeeID]) {
				return(NetGlobalDecodeError::INVALID_KICK_PLAYER);
			}
			if (context.SenderPlayerID == static_cast<int>(packet.Kick.KickeeID)) {
				return(NetGlobalDecodeError::SELF_KICK);
			}
			break;
		}

		default:
			break;
	}

	return(NetGlobalDecodeError::NONE);
}


/// <summary>Counts a rejection and selects sparse diagnostics.</summary>
NetGlobalRejectionRecord NetGlobalRejectionCounters::Record(NetGlobalDecodeError error) noexcept
{
	std::size_t const index = static_cast<std::size_t>(error);
	if (error == NetGlobalDecodeError::NONE || index >= Counts.size()) {
		return(NetGlobalRejectionRecord{});
	}

	std::uint32_t & count = Counts[index];
	if (count != std::numeric_limits<std::uint32_t>::max()) {
		count++;
	}

	return(NetGlobalRejectionRecord{count, count == 1 || (count & (count - 1)) == 0});
}


/// <summary>Returns one rejection category's count.</summary>
std::uint32_t NetGlobalRejectionCounters::Count(NetGlobalDecodeError error) const noexcept
{
	std::size_t const index = static_cast<std::size_t>(error);
	return(index < Counts.size() ? Counts[index] : 0);
}


/// <summary>Returns a stable global-packet rejection name.</summary>
char const * Net_Global_Error_Name(NetGlobalDecodeError error) noexcept
{
	switch (error) {
		case NetGlobalDecodeError::NONE: return("none");
		case NetGlobalDecodeError::INVALID_LENGTH: return("invalid length");
		case NetGlobalDecodeError::INVALID_COMMAND: return("invalid command");
		case NetGlobalDecodeError::SENDER_NOT_MEMBER: return("sender is not a session member");
		case NetGlobalDecodeError::UNTERMINATED_NAME: return("unterminated player name");
		case NetGlobalDecodeError::UNTERMINATED_MESSAGE: return("unterminated message");
		case NetGlobalDecodeError::INVALID_COLOR: return("invalid session-member color");
		case NetGlobalDecodeError::INVALID_PROGRESS: return("invalid progress value");
		case NetGlobalDecodeError::INVALID_KICK_PLAYER: return("invalid kick player");
		case NetGlobalDecodeError::SELF_KICK: return("self kick proposal");
		case NetGlobalDecodeError::DUPLICATE_KICK_PROPOSAL: return("duplicate kick proposal");
		case NetGlobalDecodeError::KICK_PROPOSAL_QUEUE_FULL: return("kick proposal queue full");
		case NetGlobalDecodeError::COUNT: break;
	}

	return("unknown global packet error");
}
