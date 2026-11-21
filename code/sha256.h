/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>


// SHA-256 as FIPS 180-4 defines it. Data may arrive in pieces of any size.
class SHA256Class
{
	public:
		using DigestType = std::array<std::uint8_t, 32>;

		SHA256Class(void);

		void Reset(void);
		void Hash(void const * data, std::size_t size);
		void Hash(std::span<std::uint8_t const> data);
		DigestType Result(void) const;

		static DigestType Digest(void const * data, std::size_t size);
		static DigestType Digest(std::span<std::uint8_t const> data);

	private:
		void Process_Block(std::uint8_t const * block);

		std::uint32_t State[8];
		std::uint64_t Length;
		std::uint8_t Buffer[64];
		std::size_t Buffered;
};
