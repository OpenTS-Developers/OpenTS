/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Two unencrypted archives can hold the same member name. The one searched first supplies
// it. These cases mount archives the harness writes itself and check which copy is found.

#include "always.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <vector>

#include "crc.h"
#include "mixfile.h"
#include "random.h"
#include "rndstraw.h"

#include "doctest/doctest.h"

RandomStraw CryptRandom;

void Call_Back(void)
{
}

// RandomStraw constructs its generators. This harness never draws a number from them.
Random3Class::Random3Class(unsigned seed1, unsigned seed2) :
	Seed(seed1),
	Index(seed2)
{
}

int Random3Class::operator()(void)
{
	return(0);
}

int Random3Class::operator()(int, int)
{
	return(0);
}


namespace {

std::filesystem::path Make_Directory(void)
{
	std::filesystem::path dir = std::filesystem::temp_directory_path() / "opents-mixorder";
	std::filesystem::remove_all(dir);
	std::filesystem::create_directory(dir);
	return(dir);
}


struct Member {
	char const * Name;
	int Size;
};


void Write_Byte(FILE * file, unsigned char value)
{
	REQUIRE(std::fwrite(&value, 1, 1, file) == 1);
}


void Write_Int(FILE * file, int value)
{
	for (int index = 0; index < 4; index++) {
		Write_Byte(file, (unsigned char)((value >> (8 * index)) & 0xFF));
	}
}


void Write_Mix(std::filesystem::path const & path, std::vector<Member> members)
{
	struct Entry {
		int CRC;
		int Offset;
		int Size;
	};

	std::vector<Entry> entries;
	int data_size = 0;
	for (Member const & member : members) {
		char upper[64];
		std::snprintf(upper, sizeof(upper), "%s", member.Name);
		strupr(upper);

		Entry entry;
		entry.CRC = CRCEngine()(upper, (int)std::strlen(upper));
		entry.Offset = data_size;
		entry.Size = member.Size;
		entries.push_back(entry);
		data_size += member.Size;
	}

	std::sort(entries.begin(), entries.end(), [](Entry const & left, Entry const & right) {
		return(left.CRC < right.CRC);
	});

	FILE * file = std::fopen(path.string().c_str(), "wb");
	REQUIRE(file != nullptr);
	Write_Byte(file, (unsigned char)(entries.size() & 0xFF));
	Write_Byte(file, (unsigned char)((entries.size() >> 8) & 0xFF));
	Write_Int(file, data_size);
	for (Entry const & entry : entries) {
		Write_Int(file, entry.CRC);
		Write_Int(file, entry.Offset);
		Write_Int(file, entry.Size);
	}
	std::vector<unsigned char> payload;
	for (Member const & member : members) {
		payload.insert(payload.end(), (std::size_t)member.Size, (unsigned char)member.Size);
	}
	if (!payload.empty()) {
		REQUIRE(std::fwrite(payload.data(), 1, payload.size(), file) == payload.size());
	}
	std::fclose(file);
}


int Member_Size(char const * name)
{
	int size = -1;
	if (!MixFileClass::Offset(name, nullptr, nullptr, nullptr, &size)) {
		return(-1);
	}
	return(size);
}


struct Mounted {
	std::filesystem::path Directory;
	std::vector<MixFileClass *> Mixes;

	Mounted(void) : Directory(Make_Directory()) {}

	~Mounted(void)
	{
		for (MixFileClass * mix : Mixes) {
			delete mix;
		}
		std::filesystem::remove_all(Directory);
	}

	void Add(char const * filename, std::vector<Member> members)
	{
		std::filesystem::path const path = Directory / filename;
		Write_Mix(path, members);
		Mixes.push_back(new MixFileClass(path.string().c_str(), nullptr));
	}
};

}


TEST_CASE("a campaign movie archive is searched ahead of the other movie archives")
{
	Mounted mounted;
	mounted.Add("MOVIES01.MIX", {{"INTRO.VQA", 11}, {"ONLY01.VQA", 13}});
	mounted.Add("MOVIES02.MIX", {{"INTRO.VQA", 22}, {"ONLY02.VQA", 23}});

	CHECK(Member_Size("INTRO.VQA") == 11);

	MixFileClass::Order_Movie_Archives(2);
	CHECK(Member_Size("INTRO.VQA") == 22);
	CHECK(Member_Size("ONLY01.VQA") == 13);
	CHECK(Member_Size("ONLY02.VQA") == 23);

	MixFileClass::Order_Movie_Archives(0);
	CHECK(Member_Size("INTRO.VQA") == 11);

	MixFileClass::Order_Movie_Archives(1);
	CHECK(Member_Size("INTRO.VQA") == 11);

	MixFileClass::Order_Movie_Archives(99);
	CHECK(Member_Size("INTRO.VQA") == 11);
}


TEST_CASE("an archive mounted before the movie archives still wins")
{
	Mounted mounted;
	mounted.Add("PATCH.MIX", {{"INTRO.VQA", 7}});
	mounted.Add("MOVIES01.MIX", {{"INTRO.VQA", 11}});
	mounted.Add("SCORES.MIX", {{"ONLY.VQA", 5}});
	mounted.Add("MOVIES02.MIX", {{"INTRO.VQA", 22}, {"SCORE.VQA", 22}});
	mounted.Add("SIDECD02.MIX", {{"INTRO.VQA", 9}, {"SCORE.VQA", 9}});

	MixFileClass::Order_Movie_Archives(2);

	CHECK(Member_Size("INTRO.VQA") == 7);
	CHECK(Member_Size("SCORE.VQA") == 22);
	CHECK(Member_Size("ONLY.VQA") == 5);
}


TEST_CASE("the movie archive name match ignores letter case")
{
	Mounted mounted;
	mounted.Add("MOVIES01.MIX", {{"INTRO.VQA", 11}});
	mounted.Add("movies02.mix", {{"INTRO.VQA", 22}});

	MixFileClass::Order_Movie_Archives(2);
	CHECK(Member_Size("INTRO.VQA") == 22);
}


TEST_CASE("one movie archive and an empty list are left alone")
{
	MixFileClass::Order_Movie_Archives(2);

	Mounted mounted;
	mounted.Add("MOVIES01.MIX", {{"INTRO.VQA", 11}});
	MixFileClass::Order_Movie_Archives(2);
	CHECK(Member_Size("INTRO.VQA") == 11);
}
