/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins what the scenario reader relies on from the file holder: a name is matched without
// regard to case and only while bytes are held, a new file replaces the old one, and a file
// with no bytes or a clear empties it.

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "scenfile.h"

namespace {

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-64s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


std::vector<char> Bytes_Of(char const * text)
{
	return(std::vector<char>(text, text + std::strlen(text)));
}

}


int main(void)
{
	/*
	 * A holder with nothing in it answers for no name at all.
	 */
	{
		ScenarioFileClass file;

		Check(!file.Is_Present(), "a fresh holder holds nothing");
		Check(file.Size() == 0, "and has no bytes");
		Check(!file.Matches("SPAWNMAP.INI"), "an empty holder matches no name");
		Check(!file.Matches(NULL), "a null name never matches");
	}

	/*
	 * The name is matched the way the game names files, without regard to case, and the
	 * bytes come back exactly as they were read.
	 */
	{
		ScenarioFileClass file;
		file.Assign("spawnmap.ini", Bytes_Of("[Basic]\nName=One\n"));

		Check(file.Is_Present(), "an assigned file is present");
		Check(file.Matches("spawnmap.ini"), "the name is matched as given");
		Check(file.Matches("SPAWNMAP.INI"), "the name is matched without regard to case");
		Check(!file.Matches("SPAWNMAP.MAP"), "another name misses");
		Check(!file.Matches(NULL), "a null name misses a held file too");
		Check(std::string(file.Name()) == "spawnmap.ini", "the name is kept as given");
		Check(std::string(file.Data(), file.Size()) == "[Basic]\nName=One\n", "the bytes are kept as read");
	}

	/*
	 * A new file replaces the old one outright; nothing of the old name or bytes remains.
	 */
	{
		ScenarioFileClass file;
		file.Assign("GDI1A.MAP", Bytes_Of("one"));
		file.Assign("GDI2A.MAP", Bytes_Of("second"));

		Check(!file.Matches("GDI1A.MAP"), "a new file replaces the old name");
		Check(file.Matches("GDI2A.MAP") && file.Size() == 6, "and the old bytes");
	}

	/*
	 * A file with no bytes, or a clear, leaves the holder as it began.
	 */
	{
		ScenarioFileClass file;
		file.Assign("GDI1A.MAP", Bytes_Of("one"));
		file.Assign("GDI2A.MAP", std::vector<char>());

		Check(!file.Is_Present() && !file.Matches("GDI2A.MAP"), "a file with no bytes clears the holder");

		file.Assign("GDI1A.MAP", Bytes_Of("one"));
		file.Clear();

		Check(!file.Is_Present() && !file.Matches("GDI1A.MAP") && file.Size() == 0, "clearing empties it");
	}

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
