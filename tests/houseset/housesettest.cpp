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
#include <cstdio>
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


namespace {

int Failures = 0;
int Checked = 0;


void Check(bool passed, char const * what)
{
	Checked++;
	if (!passed) {
		std::printf("FAILED %s\n", what);
		Failures++;
	}
}


HouseClass House[HOUSE_MAX];


// Writes or reads the raw bytes of whatever it is handed, the way the save stream treats
// numbers and arrays of numbers.
class ByteStream
{
	public:
		explicit ByteStream(bool saving) : Saving(saving), Cursor(0) {}

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


void Check_Set(void)
{
	HouseSet set;
	Check(!set.Any(), "a fresh set holds no house");

	set.Set(&House[0]);
	set.Set(&House[40]);
	set.Set(&House[HOUSE_MAX - 1]);
	Check(set[&House[0]], "the first house is marked");
	Check(set[&House[40]], "a house past the thirty-second is marked");
	Check(set[&House[HOUSE_MAX - 1]], "the last house is marked");
	Check(!set[&House[1]], "an unmarked house reads clear");
	Check(!set[&House[8]], "a house one byte along reads clear");
	Check(!set[static_cast<HouseClass const *>(nullptr)], "no house reads clear");

	set.Clear(&House[40]);
	Check(!set[&House[40]], "clearing a house removes only its mark");
	Check(set[&House[0]] && set[&House[HOUSE_MAX - 1]], "clearing a house leaves the others");

	set.Clear();
	Check(!set.Any(), "clearing the set removes every mark");
}


void Check_Algebra(void)
{
	HouseSet mapped;
	HouseSet visible;
	mapped.Set(&House[2]);
	mapped.Set(&House[33]);
	mapped.Set(&House[50]);
	visible.Set(&House[33]);

	HouseSet stale = mapped & ~visible;
	Check(stale[&House[2]] && stale[&House[50]], "mapped and not visible keeps the unseen houses");
	Check(!stale[&House[33]], "mapped and not visible drops the house still seeing");

	HouseSet both = stale | visible;
	Check(both == mapped, "the parts join back into the whole");
	Check(both != stale, "sets with different houses differ");

	HouseSet copy = mapped;
	copy &= visible;
	Check(copy == visible, "&= keeps only the houses in both");
	copy |= stale;
	Check(copy == mapped, "|= adds the other houses");
}


void Check_Array(void)
{
	HouseArray<std::uint16_t> counts;
	Check(counts[&House[0]] == 0 && counts[&House[63]] == 0, "a fresh array holds zero for every house");

	counts[&House[5]] += 2;
	counts[&House[60]] = 300;
	Check(counts[&House[5]] == 2, "a house keeps its own value");
	Check(counts[&House[60]] == 300, "a value past what a byte holds survives");
	Check(counts[&House[6]] == 0, "a neighbor's value is untouched");

	HouseArray<std::uint16_t> const & view = counts;
	Check(view[&House[60]] == 300, "a const array reads the same value");

	counts.Fill(7);
	Check(counts[&House[0]] == 7 && counts[&House[63]] == 7, "filling sets every house");
}


void Check_Save(void)
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

	Check(set_back == set, "a set comes back from a save with every house");
	Check(counts_back[&House[3]] == 9 && counts_back[&House[63]] == 1234, "an array comes back from a save with every value");
	Check(in.Cursor == out.Bytes.size(), "a load reads exactly what the save wrote");
}


void Check_CRC(void)
{
	HouseSet low;
	HouseSet high;
	low.Set(&House[0]);
	high.Set(&House[32]);

	CRCRecorder a;
	CRCRecorder b;
	low.Compute_CRC(a);
	high.Compute_CRC(b);
	Check(a.Values.size() == 2 && b.Values.size() == 2, "a set feeds the checksum both halves");
	Check(a.Values != b.Values, "houses either side of the thirty-second reach the checksum apart");
}


void Check_Layout(void)
{
	Check(std::is_trivially_copyable_v<HouseSet>, "a set survives the raw copy cells are moved with");
	Check(std::is_trivially_copyable_v<HouseArray<std::uint16_t>>, "an array survives the raw copy cells are moved with");
	Check(sizeof(HouseSet) == 8, "a set is one 64-bit word");
}

} // namespace


int main(void)
{
	for (int index = 0; index < HOUSE_MAX; index++) {
		House[index].HeapID = index;
	}

	Check_Set();
	Check_Algebra();
	Check_Array();
	Check_Save();
	Check_CRC();
	Check_Layout();

	std::printf("%-52s %s\n", "House sets and arrays",
		Failures == 0 ? "ok" : "FAILED");
	std::printf("checked %d cases, %d mismatches\n", Checked, Failures);

	return(Failures == 0 ? 0 : 1);
}
