/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstddef>


namespace NetSemantic
{
	bool Index_Is_Valid(int index, std::size_t count) noexcept;

	bool Subject_Owner_Is_Valid(int sender, int owner) noexcept;

	bool Game_Speed_Is_Valid(int game_speed) noexcept;

	bool Latency_Fudge_Is_Valid(int latency_fudge) noexcept;

	bool Animation_Type_Is_Valid(int animation, int none, std::size_t count) noexcept;

	bool Animation_Owner_Is_Valid(int owner, int none, std::size_t count) noexcept;
}
