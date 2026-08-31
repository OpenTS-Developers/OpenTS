/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "netsemantic.h"


namespace NetSemantic
{
	/// <summary>Checks a signed index against a collection size.</summary>
	bool Index_Is_Valid(int index, std::size_t count) noexcept
	{
		return(index >= 0 && static_cast<std::size_t>(index) < count);
	}


	/// <summary>Checks that a synchronized object's current owner matches its sender.</summary>
	bool Subject_Owner_Is_Valid(int sender, int owner) noexcept
	{
		return(sender >= 0 && sender == owner);
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


	/// <summary>Checks that a timing event came from the resolved master.</summary>
	bool Timing_Authority_Is_Valid(int sender, int master) noexcept
	{
		return(master >= 0 && sender == master);
	}


	/// <summary>Validates legacy propagation delay for its negotiated protocol.</summary>
	bool Response_Time_Is_Valid(unsigned int delay, unsigned int minimum_delay, unsigned int frame_send_rate, bool compressed) noexcept
	{
		if (delay < minimum_delay) {
			return(false);
		}
		if (!compressed) {
			return(true);
		}
		return(frame_send_rate >= 1 && frame_send_rate <= 10 && delay >= 2 * frame_send_rate
			&& delay <= 250 && delay % frame_send_rate == 0);
	}


	/// <summary>Checks synchronized frame-rate and scheduling bounds.</summary>
	bool Timing_Values_Are_Valid(unsigned int desired_frame_rate, unsigned int max_ahead, unsigned int frame_send_rate) noexcept
	{
		if (desired_frame_rate == 0 || desired_frame_rate > 60 || frame_send_rate == 0 || frame_send_rate > 10) {
			return(false);
		}

		unsigned int const minimum_max_ahead = frame_send_rate == 1 ? 4 : 3 * frame_send_rate;
		return(max_ahead >= minimum_max_ahead && max_ahead <= 250 && max_ahead % frame_send_rate == 0);
	}
}
