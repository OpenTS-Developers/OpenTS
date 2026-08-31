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
}
