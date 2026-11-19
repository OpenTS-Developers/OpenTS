/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Exercises the order the mounted mixfiles are searched in, with plain archives the harness
// writes itself into a directory of its own.

#include <windows.h>

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#include "crc.h"
#include "mixfile.h"
#include "random.h"
#include "rawfile.h"
#include "rndstraw.h"

#include "doctest/doctest.h"

// Only an encrypted mixfile draws on the random numbers, and this harness writes none.
Random3Class::Random3Class(unsigned, unsigned) : Seed(0), Index(0) {}
int Random3Class::operator() (void) { return(0); }
RandomStraw CryptRandom;

// Caching a mixfile calls back into the game loop, which this harness does not run.
void Call_Back(void)
{
}

namespace {

struct Member
{
	char const * Name;
	char const * Contents;
};


// Writes an unencrypted mixfile in the plain format: the member count and data size, the
// directory sorted by CRC, then the members' data.
void Write_Mixfile(char const * filename, std::vector<Member> const & members)
{
	std::vector<MixFileClass::SubBlock> blocks;
	std::string data;

	for (Member const & member : members) {
		std::string name = member.Name;
		std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return((char)std::toupper(c)); });

		MixFileClass::SubBlock block;
		block.CRC = CRCEngine()(name.c_str(), (int)name.size());
		block.Offset = (int)data.size();
		block.Size = (int)std::strlen(member.Contents);
		blocks.push_back(block);
		data += member.Contents;
	}
	std::sort(blocks.begin(), blocks.end(), [](MixFileClass::SubBlock const & one, MixFileClass::SubBlock const & two) { return(one.CRC < two.CRC); });

	short const count = (short)blocks.size();
	int const size = (int)data.size();

	RawFileClass file(filename);
	file.Open(FileClass::WRITE);
	file.Write(&count, sizeof(count));
	file.Write(&size, sizeof(size));
	file.Write(blocks.data(), (int)(blocks.size() * sizeof(MixFileClass::SubBlock)));
	file.Write(data.data(), (int)data.size());
	file.Close();
}


std::string Found_In(char const * filename)
{
	MixFileClass * mixfile = NULL;
	if (!MixFileClass::Offset(filename, NULL, &mixfile) || mixfile == NULL) {
		return(std::string());
	}
	return(mixfile->Filename);
}


struct MixDirectoryFixture
{
	char OriginalDirectory[MAX_PATH];
	std::string Root;

	MixDirectoryFixture(void)
	{
		GetCurrentDirectory(sizeof(OriginalDirectory), OriginalDirectory);

		char temp[MAX_PATH];
		GetTempPath(sizeof(temp), temp);
		Root = std::string(temp) + "opents-mixorder-" + std::to_string(GetCurrentProcessId());
		CreateDirectory(Root.c_str(), NULL);
		SetCurrentDirectory(Root.c_str());
	}

	~MixDirectoryFixture(void)
	{
		for (char const * name : { "FIRST.MIX", "SECOND.MIX", "THIRD.MIX" }) {
			DeleteFile(name);
		}
		SetCurrentDirectory(OriginalDirectory);
		RemoveDirectory(Root.c_str());
	}
};

}


TEST_CASE_FIXTURE(MixDirectoryFixture, "a mixfile moved in the search order supplies a shared name from its new place")
{
	Write_Mixfile("FIRST.MIX", { { "SHARED.TXT", "first" }, { "ONLYFIRST.TXT", "first" } });
	Write_Mixfile("SECOND.MIX", { { "SHARED.TXT", "second" } });
	Write_Mixfile("THIRD.MIX", { { "SHARED.TXT", "third" } });

	MixFileClass * first = new MixFileClass("FIRST.MIX", NULL);
	MixFileClass * second = new MixFileClass("SECOND.MIX", NULL);
	MixFileClass * third = new MixFileClass("THIRD.MIX", NULL);
	REQUIRE_MESSAGE((first->Is_Valid() && second->Is_Valid() && third->Is_Valid()), "the harness's archives open");

	CHECK_MESSAGE(Found_In("SHARED.TXT") == "FIRST.MIX", "the archive mounted first supplies a shared name");

	third->Search_After(first);
	CHECK_MESSAGE(Found_In("SHARED.TXT") == "FIRST.MIX", "an archive moved behind the first leaves it in front");
	second->Search_Last();
	CHECK_MESSAGE((first->Next() == third && third->Next() == second), "an archive moved behind another is searched right after it");

	third->Search_After(NULL);
	CHECK_MESSAGE(Found_In("SHARED.TXT") == "THIRD.MIX", "an archive moved to the front supplies a shared name");
	CHECK_MESSAGE(Found_In("ONLYFIRST.TXT") == "FIRST.MIX", "a name only a later archive holds is still found");

	third->Search_Last();
	first->Search_Last();
	CHECK_MESSAGE(Found_In("SHARED.TXT") == "SECOND.MIX", "archives moved to the back are searched after the rest");
	first->Search_Last();
	CHECK_MESSAGE((second->Next() == third && third->Next() == first && first->Next_Valid() == NULL), "moving the last archive to the back keeps the list whole");

	MixFileClass * missing = new MixFileClass("MISSING.MIX", NULL);
	CHECK_MESSAGE(!missing->Is_Valid(), "an archive that is not there is not searched");
	missing->Search_After(NULL);
	missing->Search_Last();
	CHECK_MESSAGE(!missing->Is_Valid(), "moving an archive that failed to open leaves it out of the search");
	CHECK_MESSAGE(Found_In("SHARED.TXT") == "SECOND.MIX", "an archive that failed to open does not disturb the search");

	delete missing;
	delete first;
	delete second;
	delete third;
	CHECK_MESSAGE(Found_In("SHARED.TXT").empty(), "a deleted archive leaves the search");
}
