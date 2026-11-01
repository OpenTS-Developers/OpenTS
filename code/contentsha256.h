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
#include <cstdint>
#include <string>


class ContentSha256
{
	public:
		ContentSha256(void);
		void Reset(void);
		void Update(void const * data, std::size_t size);
		void Final(unsigned char out[32]);

	private:
		void Transform(unsigned char const block[64]);

		std::uint32_t Hash[8];
		std::uint64_t Bits;
		unsigned char Block[64];
		std::size_t Used;
};


std::string Content_Sha256_Hex(unsigned char const digest[32]);

bool Content_Sha256_Matches(char const * hex, unsigned char const digest[32]);
