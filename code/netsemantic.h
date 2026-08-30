/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "nettiming.h"

#include <cstddef>
#include <cstdint>
#include <optional>


namespace NetSemantic
{
	bool Index_Is_Valid(int index, std::size_t count) noexcept;

	bool Game_Speed_Is_Valid(int game_speed) noexcept;

	bool Latency_Fudge_Is_Valid(int latency_fudge) noexcept;

	bool Animation_Type_Is_Valid(int animation, int none, std::size_t count) noexcept;

	bool Animation_Owner_Is_Valid(int owner, int none, std::size_t count) noexcept;

	bool Timing_Authority_Is_Valid(int sender, int master) noexcept;

	std::optional<NetTiming::TimingSettings> Decode_Timing_Settings(std::uint16_t desired_frame_rate, std::uint16_t wire_max_ahead,
		std::uint8_t frame_send_rate, unsigned int fog_padding) noexcept;

	bool Network_Report_Is_Valid(std::uint16_t process_milliseconds, std::uint16_t round_trip_milliseconds) noexcept;
}
