/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ini.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

namespace {

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-64s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


std::string Entry(INIClass const & ini, char const * section, char const * name)
{
	char value[128];
	ini.Get_String(section, name, "", value, sizeof(value));
	return(value);
}


bool Round_Trips(std::size_t size)
{
	std::vector<unsigned char> source(size);
	for (std::size_t index = 0; index < source.size(); index++) {
		source[index] = static_cast<unsigned char>((index * 37 + 11) & 0xff);
	}

	INIClass ini;
	if (!ini.Put_UUBlock("Binary", source.data(), static_cast<int>(source.size()))) {
		return(false);
	}

	std::vector<unsigned char> decoded(size, 0xff);
	int count = ini.Get_UUBlock("Binary", decoded.data(), static_cast<int>(decoded.size()));
	return(count == static_cast<int>(source.size()) && decoded == source);
}

}


int Test_UUBlock(void)
{
	{
		INIClass ini;
		std::array<unsigned char, 1> one{'M'};
		std::array<unsigned char, 2> two{'M', 'a'};
		std::array<unsigned char, 3> three{'M', 'a', 'n'};

		ini.Put_UUBlock("One", one.data(), static_cast<int>(one.size()));
		ini.Put_UUBlock("Two", two.data(), static_cast<int>(two.size()));
		ini.Put_UUBlock("Three", three.data(), static_cast<int>(three.size()));

		Check(Entry(ini, "One", "1") == "TQ==", "one input byte receives two padding characters");
		Check(Entry(ini, "Two", "1") == "TWE=", "two input bytes receive one padding character");
		Check(Entry(ini, "Three", "1") == "TWFu", "three input bytes form one complete Base64 group");
	}

	{
		INIClass ini;
		std::array<unsigned char, 54> source{};
		ini.Put_String("Binary", "stale", "value");
		bool stored = ini.Put_UUBlock("Binary", source.data(), static_cast<int>(source.size()));

		Check(stored && ini.Entry_Count("Binary") == 2,
			"writing a block replaces its section with numbered entries");
		Check(Entry(ini, "Binary", "1") == std::string(70, 'A'),
			"encoded data is split after 70 characters");
		Check(Entry(ini, "Binary", "2") == "AA", "data after the line boundary is retained");
	}

	{
		std::array<std::size_t, 11> sizes{1, 2, 3, 51, 52, 53, 54, 55, 4095, 4096, 8194};
		bool valid = std::all_of(sizes.begin(), sizes.end(), Round_Trips);
		Check(valid, "binary blocks round-trip across Base64 and INI boundaries");
	}

	{
		INIClass ini;
		ini.Put_String("Binary", "1", std::string(70, 'A').c_str());
		ini.Put_String("Binary", "2", "AA");

		std::array<unsigned char, 54> decoded;
		decoded.fill(0xff);
		int count = ini.Get_UUBlock("Binary", decoded.data(), static_cast<int>(decoded.size()));
		Check(count == static_cast<int>(decoded.size())
			&& std::all_of(decoded.begin(), decoded.end(), [](unsigned char value) { return(value == 0); }),
			"decoding carries an incomplete group across INI entries");
	}

	{
		INIClass ini;
		ini.Put_String("Binary", "1", "T Q#");

		unsigned char decoded = 0;
		int count = ini.Get_UUBlock("Binary", &decoded, 1);
		Check(count == 1 && decoded == 'M',
			"non-Base64 separators are ignored while decoding");
	}

	{
		INIClass ini;
		ini.Put_String("Binary", "1", "TQ==");

		unsigned char decoded = 0;
		int count = ini.Get_UUBlock("Binary", &decoded, 1);
		Check(count == 1 && decoded == 'M', "padding completes the final encoded block");
	}

	{
		INIClass ini;
		ini.Put_String("Binary", "1", "TQ");

		unsigned char decoded = 0xa5;
		int count = ini.Get_UUBlock("Binary", &decoded, 1);
		Check(count == 1 && decoded == 'M', "missing Base64 padding remains accepted");
	}

	{
		INIClass ini;
		ini.Put_String("Binary", "1", std::string(70, 'A').c_str());
		ini.Put_String("Binary", "2", "AA");

		std::array<unsigned char, 18> decoded;
		decoded.fill(0xa5);
		int count = ini.Get_UUBlock("Binary", decoded.data(), 17);
		Check(count == 17 && std::all_of(decoded.begin(), decoded.begin() + 17,
			[](unsigned char value) { return(value == 0); }) && decoded.back() == 0xa5,
			"decoding stops at the destination buffer boundary");
	}

	{
		INIClass ini;
		unsigned char value = 0;
		Check(!ini.Put_UUBlock(nullptr, &value, 1)
			&& !ini.Put_UUBlock("Binary", nullptr, 1)
			&& !ini.Put_UUBlock("Binary", &value, 0),
			"invalid writes are rejected");
		Check(ini.Get_UUBlock(nullptr, &value, 1) == 0
			&& ini.Get_UUBlock("Binary", nullptr, 1) == 0
			&& ini.Get_UUBlock("Binary", &value, 0) == 0,
			"invalid reads produce no output");
	}

	return(Failures);
}
