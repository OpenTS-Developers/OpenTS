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


namespace NetGlobal
{
	namespace {

		static_assert(std::is_trivially_copyable_v<GlobalPacketType>);
		constexpr std::size_t PACKET_SIZE = sizeof(GlobalPacketType);


		/// <summary>Checks that a fixed wire string contains a terminator.</summary>
		bool Has_Terminator(char const * text, std::size_t capacity)
		{
			return(std::memchr(text, '\0', capacity) != NULL);
		}


		/// <summary>Checks a player index against the current session roster.</summary>
		bool Is_Active_Player(ValidationContext const & context, int player)
		{
			return(player >= 0 && player < static_cast<int>(context.ActivePlayers.size()) && context.ActivePlayers[player]);
		}

	}	// namespace


	/// <summary>Clears an outgoing packet before selecting its command.</summary>
	void Initialize_Packet(GlobalPacketType & packet, NetCommandType command) noexcept
	{
		std::memset(&packet, 0, sizeof(packet));
		packet.Command = command;
	}


	/// <summary>Resolves a sender through an exact endpoint or one unique zero-port roster entry.</summary>
	EndpointResolution Resolve_Sender(Endpoint const & sender, std::span<Endpoint const> roster) noexcept
	{
		int match = -1;
		for (std::size_t index = 0; index < roster.size(); index++) {
			if (roster[index].IP == sender.IP && roster[index].Port == sender.Port) {
				if (match >= 0) {
					return(EndpointResolution{DecodeError::AMBIGUOUS_SENDER});
				}
				match = static_cast<int>(index);
			}
		}
		if (match >= 0) {
			return(EndpointResolution{DecodeError::NONE, EndpointMatch::EXACT, match});
		}

		for (std::size_t index = 0; index < roster.size(); index++) {
			if (roster[index].IP == sender.IP && roster[index].Port == 0) {
				if (match >= 0) {
					return(EndpointResolution{DecodeError::AMBIGUOUS_SENDER});
				}
				match = static_cast<int>(index);
			}
		}
		if (match >= 0) {
			return(EndpointResolution{DecodeError::NONE, EndpointMatch::ZERO_PORT, match});
		}

		return(EndpointResolution{});
	}


	/// <summary>Identifies public in-game discovery commands.</summary>
	static bool Command_Is_Public(NetCommandType command)
	{
		return(command == NET_QUERY_GAME || command == NET_QUERY_PLAYER);
	}


	/// <summary>Identifies commands restricted to session members.</summary>
	static bool Command_Requires_Member(NetCommandType command)
	{
		switch (command) {
			case NET_SIGN_OFF:
			case NET_MESSAGE:
			case NET_PROGRESS_REPORT:
			case NET_READY_TO_GO:
			case NET_PROPOSE_KICK:
			case NET_MOVIE_SKIP:
				return(true);

			default:
				return(false);
		}
	}


	/// <summary>Validates an in-game global packet before dispatch.</summary>
	DecodeError Validate_In_Game_Packet(GlobalPacketType const & packet, std::size_t packet_length, ValidationContext const & context)
	{
		if (packet_length != PACKET_SIZE) {
			return(DecodeError::INVALID_LENGTH);
		}

		bool const is_public = Command_Is_Public(packet.Command);
		bool const requires_member = Command_Requires_Member(packet.Command);
		if (!is_public && !requires_member) {
			return(DecodeError::INVALID_COMMAND);
		}
		if (requires_member && !context.SenderIsMember) {
			return(DecodeError::SENDER_NOT_MEMBER);
		}

		switch (packet.Command) {
			case NET_QUERY_PLAYER:
				if (!Has_Terminator(packet.Name, sizeof(packet.Name))) {
					return(DecodeError::UNTERMINATED_NAME);
				}
				break;

			case NET_MESSAGE:
				if (!Has_Terminator(packet.Name, sizeof(packet.Name))) {
					return(DecodeError::UNTERMINATED_NAME);
				}
				if (!Has_Terminator(packet.Message.Buf, sizeof(packet.Message.Buf))) {
					return(DecodeError::UNTERMINATED_MESSAGE);
				}
				if (context.SenderPlayerColor < 0 || context.SenderPlayerColor >= MAX_MPLAYER_COLORS) {
					return(DecodeError::INVALID_COLOR);
				}
				break;

			case NET_PROGRESS_REPORT:
				if (packet.Progress.Percent < 0 || packet.Progress.Percent > 100) {
					return(DecodeError::INVALID_PROGRESS);
				}
				break;

			case NET_PROPOSE_KICK: {
				if (!Is_Active_Player(context, context.SenderPlayerID) || packet.Kick.KickeeID >= context.ActivePlayers.size()
					|| !context.ActivePlayers[packet.Kick.KickeeID]) {
					return(DecodeError::INVALID_KICK_PLAYER);
				}
				if (context.SenderPlayerID == static_cast<int>(packet.Kick.KickeeID)) {
					return(DecodeError::SELF_KICK);
				}
				break;
			}

			default:
				break;
		}

		return(DecodeError::NONE);
	}


	/// <summary>Counts a rejection and selects sparse diagnostics.</summary>
	RejectionRecord RejectionCounters::Record(DecodeError error) noexcept
	{
		std::size_t const index = static_cast<std::size_t>(error);
		if (error == DecodeError::NONE || index >= Counts.size()) {
			return(RejectionRecord{});
		}

		std::uint32_t & count = Counts[index];
		if (count != std::numeric_limits<std::uint32_t>::max()) {
			count++;
		}

		return(RejectionRecord{count, count == 1 || (count & (count - 1)) == 0});
	}


	/// <summary>Returns one rejection category's count.</summary>
	std::uint32_t RejectionCounters::Count(DecodeError error) const noexcept
	{
		std::size_t const index = static_cast<std::size_t>(error);
		return(index < Counts.size() ? Counts[index] : 0);
	}


	/// <summary>Returns a stable global-packet rejection name.</summary>
	char const * Error_Name(DecodeError error) noexcept
	{
		switch (error) {
			case DecodeError::NONE: return("none");
			case DecodeError::INVALID_LENGTH: return("invalid length");
			case DecodeError::INVALID_COMMAND: return("invalid command");
			case DecodeError::SENDER_NOT_MEMBER: return("sender is not a session member");
			case DecodeError::UNTERMINATED_NAME: return("unterminated player name");
			case DecodeError::UNTERMINATED_MESSAGE: return("unterminated message");
			case DecodeError::INVALID_COLOR: return("invalid session-member color");
			case DecodeError::INVALID_PROGRESS: return("invalid progress value");
			case DecodeError::INVALID_KICK_PLAYER: return("invalid kick player");
			case DecodeError::SELF_KICK: return("self kick proposal");
			case DecodeError::DUPLICATE_KICK_PROPOSAL: return("duplicate kick proposal");
			case DecodeError::KICK_PROPOSAL_QUEUE_FULL: return("kick proposal queue full");
			case DecodeError::AMBIGUOUS_SENDER: return("ambiguous session-member endpoint");
			case DecodeError::COUNT: break;
		}

		return("unknown global packet error");
	}
}
