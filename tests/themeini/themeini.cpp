/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Reads music track sections shaped like the shipped THEME.INI and with the
// keys OpenTS adds, and checks what each key becomes. Needs no game data.

#include "dbgprint.h"
#include "ini.h"
#include "side.h"
#include "theme.h"
#include "xstraw.h"

#include "doctest/doctest.h"

#include <cmath>
#include <cstring>
#include <string>

// The side list stands in for the one the rules declare.
SideType SideClass::From_Name(char const * name)
{
	if (stricmp(name, "GDI") == 0) return(SIDE_GDI);
	if (stricmp(name, "Nod") == 0) return(SIDE_NOD);
	return(SIDE_NONE);
}

namespace {

float Distance(float value, float expect)
{
	return(std::fabs(value - expect));
}


void Read(INIClass & ini, char const * text)
{
	BufferStraw straw(text, (int)std::strlen(text));
	ini.Load(straw);
}


ThemeControl Track(INIClass const & ini, char const * name)
{
	ThemeControl control;
	std::strcpy(control.Name, name);
	control.Fill_In(ini);
	return(control);
}


struct TiberianSunFixture
{
	INIClass ini;

	TiberianSunFixture(void)
	{
		Read(ini,
			"[Themes]\n"
			"1=INTRO\n"
			"[INTRO]\n"
			"Name=Intro theme\n"
			"Length=3.27\n"
			"Normal=no\n"
			"Repeat=yes\n");
	}
};


struct NewKeysFixture
{
	INIClass ini;

	NewKeysFixture(void)
	{
		Read(ini,
			"[LOOSE]\n"
			"Sound=MYTRACK\n"
			"Volume=0.5\n"
			"RequiredAddon=1\n"
			"[PERCENT]\n"
			"Volume=40\n"
			"[JUNK]\n"
			"Volume=loud\n");
	}
};


struct SidesFixture
{
	INIClass ini;

	SidesFixture(void)
	{
		Read(ini,
			"[ONE]\n"
			"Side=GDI\n"
			"[TWO]\n"
			"Side=Nod, GDI\n"
			"[SOME]\n"
			"Side=Bogus,Nod\n"
			"[BAD]\n"
			"Side=Bogus\n"
			"[NONE]\n"
			"Side=<none>\n");
	}
};

}


TEST_CASE_FIXTURE(TiberianSunFixture, "a Tiberian Sun track reads its title, length, and play flags")
{
	ThemeControl intro = Track(ini, "INTRO");
	CHECK_MESSAGE(std::string(intro.Fullname) == "Intro theme", "Name= is the title");
	CHECK_MESSAGE(Distance(intro.Duration, 3.27f) < 0.001f, "Length= is kept as written");
	CHECK_FALSE(intro.Normal);
	CHECK(intro.Repeat);
}


TEST_CASE_FIXTURE(TiberianSunFixture, "the keys a Tiberian Sun track leaves out take their defaults")
{
	ThemeControl intro = Track(ini, "INTRO");
	CHECK_MESSAGE(intro.Sound[0] == '\0', "without Sound= the file is named after the section");
	CHECK_MESSAGE(Distance(intro.Volume, 1.0f) < 0.001f, "Volume= defaults to full");
	CHECK_MESSAGE(intro.RequiredAddon == 0, "RequiredAddon= defaults to none");
	CHECK_MESSAGE(intro.Owners.empty(), "without Side= every side hears it");
	CHECK(intro.Allows_Side(SIDE_GDI));
	CHECK(intro.Allows_Side(SIDE_NOD));
}


TEST_CASE_FIXTURE(TiberianSunFixture, "a track without a section reports it")
{
	ThemeControl missing;
	std::strcpy(missing.Name, "NOSUCH");
	CHECK_FALSE(missing.Fill_In(ini));
}


TEST_CASE_FIXTURE(NewKeysFixture, "Sound=, Volume= and RequiredAddon= are read")
{
	ThemeControl loose = Track(ini, "LOOSE");
	CHECK(std::string(loose.Sound) == "MYTRACK");
	CHECK_MESSAGE(Distance(loose.Volume, 0.5f) < 0.001f, "Volume= as a fraction");
	CHECK(loose.RequiredAddon == 1);
}


TEST_CASE_FIXTURE(NewKeysFixture, "a Volume= above 1 is a percentage, and one that is not a number is ignored")
{
	CHECK(Distance(Track(ini, "PERCENT").Volume, 0.4f) < 0.001f);
	CHECK(Distance(Track(ini, "JUNK").Volume, 1.0f) < 0.001f);
}


TEST_CASE_FIXTURE(SidesFixture, "Side= names one side or a list, skipping a side the rules do not declare")
{
	Debug_Init(0, (const char **)NULL);
	ThemeControl one = Track(ini, "ONE");
	CHECK(one.Allows_Side(SIDE_GDI));
	CHECK_FALSE(one.Allows_Side(SIDE_NOD));

	ThemeControl two = Track(ini, "TWO");
	CHECK(two.Owners.size() == 2u);
	CHECK(two.Allows_Side(SIDE_GDI));
	CHECK(two.Allows_Side(SIDE_NOD));
	CHECK_FALSE(two.Allows_Side(SIDE_MUTANT));

	ThemeControl some = Track(ini, "SOME");
	CHECK(some.Owners.size() == 1u);
	CHECK(some.Allows_Side(SIDE_NOD));
}


TEST_CASE_FIXTURE(SidesFixture, "a list naming no declared side keeps the sides before it")
{
	Debug_Init(0, (const char **)NULL);
	ThemeControl bad;
	std::strcpy(bad.Name, "BAD");
	bad.Owners.push_back(SIDE_NOD);
	bad.Fill_In(ini);
	CHECK(bad.Owners.size() == 1u);
	CHECK(bad.Allows_Side(SIDE_NOD));
}


TEST_CASE_FIXTURE(SidesFixture, "<none> allows every side")
{
	ThemeControl none;
	std::strcpy(none.Name, "NONE");
	none.Owners.push_back(SIDE_NOD);
	none.Fill_In(ini);
	CHECK(none.Owners.empty());
}
