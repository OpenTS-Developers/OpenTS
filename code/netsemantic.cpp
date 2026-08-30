/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "netsemantic.h"

#include <limits>


namespace NetSemantic
{
	/// <summary>Checks a signed index against a collection size.</summary>
	bool Index_Is_Valid(int index, std::size_t count) noexcept
	{
		return(index >= 0 && static_cast<std::size_t>(index) < count);
	}


	/// <summary>Checks a game-speed selector before table lookup.</summary>
	bool Game_Speed_Is_Valid(int game_speed) noexcept
	{
		return(game_speed >= 0 && game_speed <= 6);
	}


	/// <summary>Checks a latency-margin selector before use.</summary>
	bool Latency_Fudge_Is_Valid(int latency_fudge) noexcept
	{
		return(latency_fudge >= 0 && latency_fudge <= 3);
	}


	/// <summary>Checks an animation type or its sentinel.</summary>
	bool Animation_Type_Is_Valid(int animation, int none, std::size_t count) noexcept
	{
		return(animation == none || Index_Is_Valid(animation, count));
	}


	/// <summary>Checks an animation owner or its sentinel.</summary>
	bool Animation_Owner_Is_Valid(int owner, int none, std::size_t count) noexcept
	{
		return(owner == none || Index_Is_Valid(owner, count));
	}


	/// <summary>Checks that a timing event came from the master.</summary>
	bool Timing_Authority_Is_Valid(int sender, int master) noexcept
	{
		return(master >= 0 && sender == master);
	}


	/// <summary>Validates and decodes settings carried by a timing event.</summary>
	std::optional<NetTiming::TimingSettings> Decode_Timing_Settings(std::uint16_t desired_frame_rate, std::uint16_t wire_max_ahead,
		std::uint8_t frame_send_rate, unsigned int fog_padding) noexcept
	{
		if (desired_frame_rate == 0 || desired_frame_rate > 60 || fog_padding > std::numeric_limits<std::uint16_t>::max()
			|| wire_max_ahead < fog_padding) {
			return(std::nullopt);
		}

		NetTiming::TimingSettings const settings{frame_send_rate, wire_max_ahead - fog_padding};
		return(NetTiming::Timing_Settings_Are_Valid(settings) ? std::optional<NetTiming::TimingSettings>(settings) : std::nullopt);
	}


	/// <summary>Checks reported process and round-trip times.</summary>
	bool Network_Report_Is_Valid(std::uint16_t process_milliseconds, std::uint16_t round_trip_milliseconds) noexcept
	{
		return(process_milliseconds <= NetTiming::MAXIMUM_PROCESS_MILLISECONDS
			&& (round_trip_milliseconds <= NetTiming::MAXIMUM_REPORTED_RTT || round_trip_milliseconds == UINT16_MAX));
	}
}
