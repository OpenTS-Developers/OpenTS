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
#include <span>
#include <string>
#include <string_view>
#include <vector>


using ByteVector = std::vector<std::uint8_t>;


inline std::string Hex(std::span<std::uint8_t const> bytes)
{
	static char const digits[] = "0123456789abcdef";
	std::string text;
	text.reserve(bytes.size() * 2);
	for (std::uint8_t byte : bytes) {
		text += digits[byte >> 4];
		text += digits[byte & 0x0F];
	}
	return(text);
}


// Spaces in the text are ignored, so expected bytes can be grouped by field.
inline ByteVector From_Hex(std::string_view text)
{
	auto nibble = [](char c) -> int {
		if (c >= '0' && c <= '9') return(c - '0');
		if (c >= 'a' && c <= 'f') return(c - 'a' + 10);
		if (c >= 'A' && c <= 'F') return(c - 'A' + 10);
		return(-1);
	};

	ByteVector bytes;
	int high = -1;
	for (char c : text) {
		int value = nibble(c);
		if (value < 0) {
			continue;
		}
		if (high < 0) {
			high = value;
		} else {
			bytes.push_back(std::uint8_t((high << 4) | value));
			high = -1;
		}
	}
	return(bytes);
}


inline ByteVector Bytes_Of(std::string_view text)
{
	return(ByteVector(text.begin(), text.end()));
}
