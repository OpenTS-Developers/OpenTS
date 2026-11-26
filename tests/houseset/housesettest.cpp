/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Holds HouseSet and HouseArray to their contract: every house has its own mark or value,
// set algebra works across all houses at once, and a save carries every slot, including
// those past the thirty-second house.
//
// Needs no game data.

#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vector>


// Stands in for the engine's house, which would drag the whole game in behind it. The
// templates read nothing but its heap index.
class HouseClass
{
	public:
		int HeapID;
};

#include "houseset.h"

#include "doctest/doctest.h"

namespace {

HouseClass House[HOUSE_MAX];


// Writes or reads the raw bytes of whatever it is handed, the way the save stream treats
// numbers and arrays of numbers.
class ByteStream
{
	public:
		explicit ByteStream(bool saving) : Saving(saving), Cursor(0) {}

		bool Is_Saving(void) const {return(Saving);}
		bool Is_Loading(void) const {return(!Saving);}

		template<typename T>
		void Serialize(T & value)
		{
			static_assert(std::is_trivially_copyable_v<T>);
			unsigned char * bytes = reinterpret_cast<unsigned char *>(&value);
			if (Saving) {
				Bytes.insert(Bytes.end(), bytes, bytes + sizeof(T));
			} else {
				std::memcpy(bytes, Bytes.data() + Cursor, sizeof(T));
				Cursor += sizeof(T);
			}
		}

		bool Saving;
		std::size_t Cursor;
		std::vector<unsigned char> Bytes;
};


class CRCRecorder
{
	public:
		void operator()(int value) {Values.push_back(value);}
		std::vector<int> Values;
};


// Each house carries its own heap index, which is what the sets and arrays are keyed by.
struct HouseFixture
{
	HouseFixture(void)
	{
		for (int index = 0; index < HOUSE_MAX; index++) {
			House[index].HeapID = index;
		}
	}
};


TEST_CASE_FIXTURE(HouseFixture, "a house set marks each house, the ones past the thirty-second included")
{
	HouseSet set;
	CHECK_MESSAGE(!set.Any(), "a fresh set holds no house");

	set.Set(&House[0]);
	set.Set(&House[40]);
	set.Set(&House[HOUSE_MAX - 1]);
	CHECK_MESSAGE(set[&House[0]], "the first house is marked");
	CHECK_MESSAGE(set[&House[40]], "a house past the thirty-second is marked");
	CHECK_MESSAGE(set[&House[HOUSE_MAX - 1]], "the last house is marked");
	CHECK_MESSAGE(!set[&House[1]], "an unmarked house reads clear");
	CHECK_MESSAGE(!set[&House[8]], "a house one byte along reads clear");
	CHECK_MESSAGE(!set[static_cast<HouseClass const *>(nullptr)], "no house reads clear");

	set.Clear(&House[40]);
	CHECK_MESSAGE(!set[&House[40]], "clearing a house removes only its mark");
	CHECK_MESSAGE(set[&House[0]], "clearing a house leaves the others");
	CHECK_MESSAGE(set[&House[HOUSE_MAX - 1]], "clearing a house leaves the others");

	set.Clear();
	CHECK_MESSAGE(!set.Any(), "clearing the set removes every mark");
}


TEST_CASE_FIXTURE(HouseFixture, "house sets combine, and their parts join back into the whole")
{
	HouseSet mapped;
	HouseSet visible;
	mapped.Set(&House[2]);
	mapped.Set(&House[33]);
	mapped.Set(&House[50]);
	visible.Set(&House[33]);

	HouseSet stale = mapped & ~visible;
	CHECK_MESSAGE(stale[&House[2]], "mapped and not visible keeps the unseen houses");
	CHECK_MESSAGE(stale[&House[50]], "mapped and not visible keeps the unseen houses");
	CHECK_MESSAGE(!stale[&House[33]], "mapped and not visible drops the house still seeing");

	HouseSet both = stale | visible;
	CHECK_MESSAGE(both == mapped, "the parts join back into the whole");
	CHECK_MESSAGE(both != stale, "sets with different houses differ");

	HouseSet copy = mapped;
	copy &= visible;
	CHECK_MESSAGE(copy == visible, "&= keeps only the houses in both");
	copy |= stale;
	CHECK_MESSAGE(copy == mapped, "|= adds the other houses");
}


TEST_CASE_FIXTURE(HouseFixture, "a house array holds a value for each house, beyond what a byte holds too")
{
	HouseArray<std::uint16_t> counts;
	CHECK_MESSAGE(counts[&House[0]] == 0, "a fresh array holds zero for every house");
	CHECK_MESSAGE(counts[&House[63]] == 0, "a fresh array holds zero for every house");

	counts[&House[5]] += 2;
	counts[&House[60]] = 300;
	CHECK_MESSAGE(counts[&House[5]] == 2, "a house keeps its own value");
	CHECK_MESSAGE(counts[&House[60]] == 300, "a value past what a byte holds survives");
	CHECK_MESSAGE(counts[&House[6]] == 0, "a neighbor's value is untouched");

	HouseArray<std::uint16_t> const & view = counts;
	CHECK_MESSAGE(view[&House[60]] == 300, "a const array reads the same value");

	counts.Fill(7);
	CHECK_MESSAGE(counts[&House[0]] == 7, "filling sets every house");
	CHECK_MESSAGE(counts[&House[63]] == 7, "filling sets every house");
}


TEST_CASE_FIXTURE(HouseFixture, "a house set and array come back from a save, reading exactly what it wrote")
{
	HouseSet set;
	set.Set(&House[1]);
	set.Set(&House[62]);
	HouseArray<std::uint16_t> counts;
	counts[&House[3]] = 9;
	counts[&House[63]] = 1234;

	ByteStream out(true);
	set.Serialize(out);
	counts.Serialize(out);

	HouseSet set_back;
	HouseArray<std::uint16_t> counts_back;
	ByteStream in(false);
	in.Bytes = out.Bytes;
	set_back.Serialize(in);
	counts_back.Serialize(in);

	CHECK_MESSAGE(set_back == set, "a set comes back from a save with every house");
	CHECK_MESSAGE(counts_back[&House[3]] == 9, "an array comes back from a save with every value");
	CHECK_MESSAGE(counts_back[&House[63]] == 1234, "an array comes back from a save with every value");
	CHECK_MESSAGE(in.Cursor == out.Bytes.size(), "a load reads exactly what the save wrote");
	CHECK_MESSAGE(out.Bytes.size() == sizeof(std::uint64_t) + sizeof(std::uint64_t) + 2 * sizeof(std::uint16_t), "an array saves only the houses that hold a value");

	HouseArray<std::uint16_t> stale;
	stale[&House[7]] = 5;
	ByteStream again(false);
	again.Bytes = out.Bytes;
	again.Cursor = sizeof(std::uint64_t);
	stale.Serialize(again);
	CHECK_MESSAGE(stale[&House[7]] == 0, "a load clears the houses the save left out");
	CHECK_MESSAGE(stale[&House[63]] == 1234, "a load clears the houses the save left out");
}


TEST_CASE_FIXTURE(HouseFixture, "a house set feeds the checksum both halves, apart")
{
	HouseSet low;
	HouseSet high;
	low.Set(&House[0]);
	high.Set(&House[32]);

	CRCRecorder a;
	CRCRecorder b;
	low.Compute_CRC(a);
	high.Compute_CRC(b);
	REQUIRE_MESSAGE(a.Values.size() == 2, "a set feeds the checksum both halves");
	CHECK_MESSAGE(b.Values.size() == 2, "a set feeds the checksum both halves");
	CHECK_MESSAGE(a.Values != b.Values, "houses either side of the thirty-second reach the checksum apart");
}


TEST_CASE_FIXTURE(HouseFixture, "a house array adds each house that has a value to the checksum, with its slot")
{
	HouseArray<std::uint16_t> counts;
	CRCRecorder empty;
	counts.Compute_CRC(empty);
	CHECK_MESSAGE(empty.Values.empty(), "an array with no values adds nothing");

	counts[&House[2]] = 1;
	counts[&House[40]] = 300;
	CRCRecorder filled;
	counts.Compute_CRC(filled);
	CHECK_MESSAGE(filled.Values == std::vector<int>({2, 1, 40, 300}), "each value goes in after its slot, in slot order");

	HouseArray<std::uint16_t> moved;
	moved[&House[3]] = 1;
	moved[&House[40]] = 300;
	CRCRecorder other;
	moved.Compute_CRC(other);
	CHECK_MESSAGE(other.Values != filled.Values, "the same value under another house reaches the checksum apart");
}


TEST_CASE_FIXTURE(HouseFixture, "a house set is one 64-bit word and survives the raw copy cells are moved with")
{
	CHECK_MESSAGE(std::is_trivially_copyable_v<HouseSet>, "a set survives the raw copy cells are moved with");
	CHECK_MESSAGE(std::is_trivially_copyable_v<HouseArray<std::uint16_t>>, "an array survives the raw copy cells are moved with");
	CHECK_MESSAGE(sizeof(HouseSet) == 8, "a set is one 64-bit word");
}

} // namespace
