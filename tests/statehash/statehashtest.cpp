/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Tests the game state checksum table: the combined value depends on the order, counts, and
// checksums of the parts but not on their names, and a double reaches the checksum as its
// binary64 bit pattern.
//
// Needs no game data.

#include "statehash.h"

#include "doctest/doctest.h"

#include <cstdint>
#include <cstring>
#include <vector>


namespace {

std::vector<StateHash::Part> Sample_Parts(void)
{
	return {
		{"Scenario", 1, 0x11111111u},
		{"Rules", 1, 0x22222222u},
		{"Units", 12, 0x33333333u},
		{"Cells", 4096, 0x44444444u},
	};
}

}


TEST_CASE("the combined value is repeatable and ignores part names")
{
	std::vector<StateHash::Part> parts = Sample_Parts();
	unsigned int const first = StateHash::Combine(parts);
	CHECK(StateHash::Combine(parts) == first);

	parts[2].Name = "Infantry";
	CHECK_MESSAGE(StateHash::Combine(parts) == first, "a name is a label only");
}


TEST_CASE("the combined value depends on part order, counts, and checksums")
{
	std::vector<StateHash::Part> const parts = Sample_Parts();
	unsigned int const reference = StateHash::Combine(parts);

	std::vector<StateHash::Part> swapped = parts;
	std::swap(swapped[1], swapped[2]);
	CHECK(StateHash::Combine(swapped) != reference);

	std::vector<StateHash::Part> counted = parts;
	counted[2].Count = 13;
	CHECK(StateHash::Combine(counted) != reference);

	std::vector<StateHash::Part> changed = parts;
	changed[3].CRC ^= 1;
	CHECK(StateHash::Combine(changed) != reference);

	std::vector<StateHash::Part> shorter = parts;
	shorter.pop_back();
	CHECK(StateHash::Combine(shorter) != reference);
}


TEST_CASE("a part prints as one aligned line")
{
	StateHash::Part const part = {"Units", 12, 0x0000abcdu};
	CHECK(StateHash::Format_Part(part) == "Units                        count=12     crc=0000abcd");
}


TEST_CASE("a breakdown row prints its position, label, and running checksum")
{
	CHECK(StateHash::Format_Row(42, "E1", 0x0000abcdu) == "00042  E1                        0000abcd");
}


TEST_CASE("a double is checksummed as its binary64 bit pattern")
{
	// 1.5 is 0x3FF8000000000000; on a little-endian machine the low word goes first.
	static_assert(sizeof(double) == 8);
	double const value = 1.5;
	std::uint64_t bits = 0;
	std::memcpy(&bits, &value, sizeof(bits));
	REQUIRE(bits == 0x3FF8000000000000ull);

	CRCEngine as_double;
	as_double(value);

	CRCEngine as_words;
	as_words((int)(bits & 0xFFFFFFFFu));
	as_words((int)(bits >> 32));

	CHECK(as_double() == as_words());
}
