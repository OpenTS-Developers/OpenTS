/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins what a text trigger relies on from the tutorial table: which entry names count as
// line numbers, that a scenario's lines stand in front of the base set and go away with the
// scenario, and that no line can be blanked.

#include <cstdio>
#include <cstring>
#include <string>

#include "ini.h"
#include "tutorial.h"
#include "xstraw.h"

namespace {

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-76s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


void Read(INIClass & ini, char const * text)
{
	BufferStraw straw(text, (int)std::strlen(text));
	ini.Load(straw);
}


bool Line_Is(TutorialTextClass const & tutorial, int id, char const * expected)
{
	char const * text = tutorial.Fetch(id);
	return(text != NULL && std::strcmp(text, expected) == 0);
}

}


int main(void)
{
	/*
	 * The base set comes from a [Tutorial] section, and a number nothing carries has no line.
	 */
	{
		INIClass ini;
		Read(ini,
			"[Tutorial]\n"
			"120=Hold the ridge.\n"
			"121=The transports are away.\n");

		TutorialTextClass tutorial;
		tutorial.Read_Base(ini);

		Check(Line_Is(tutorial, 120, "Hold the ridge."), "a numbered line reads back");
		Check(Line_Is(tutorial, 121, "The transports are away."), "and so does the one after it");
		Check(tutorial.Fetch(122) == NULL, "a number nothing carries has no line");
		Check(tutorial.Fetch(0) == NULL, "and neither has zero");
	}

	/*
	 * A second base read layers on top, the way the rules stack a language file over the
	 * file they were read from.
	 */
	{
		INIClass first;
		Read(first, "[Tutorial]\n120=Hold the ridge.\n121=Base only.\n");

		INIClass second;
		Read(second, "[Tutorial]\n120=Halte den Kamm.\n122=Added later.\n");

		TutorialTextClass tutorial;
		tutorial.Read_Base(first);
		tutorial.Read_Base(second);

		Check(Line_Is(tutorial, 120, "Halte den Kamm."), "a second base read replaces a line it names");
		Check(Line_Is(tutorial, 121, "Base only."), "leaves one it does not alone");
		Check(Line_Is(tutorial, 122, "Added later."), "and adds one the first read never had");
	}

	/*
	 * An entry name has to be a whole number. Anything else is refused rather than read as
	 * the number in front of it.
	 */
	{
		INIClass ini;
		Read(ini,
			"[Tutorial]\n"
			"120=A number.\n"
			"-3=A negative number.\n"
			"Foo=A name.\n"
			"120junk=A number with a tail.\n"
			"99999999999999=Past what a line number holds.\n");

		TutorialTextClass tutorial;
		tutorial.Read_Base(ini);

		Check(Line_Is(tutorial, 120, "A number."), "a whole number names a line");
		Check(Line_Is(tutorial, -3, "A negative number."), "a negative number names one too");
		Check(tutorial.Fetch(0) == NULL, "a name is refused rather than read as zero");
		Check(!Line_Is(tutorial, 120, "A number with a tail."), "a number with a tail is refused rather than truncated");
		Check(Line_Is(tutorial, 120, "A number."), "and leaves the line it would have replaced alone");
	}

	/*
	 * Two spellings of one number collapse, and the later one wins, which is how the INI
	 * reader already resolves a repeated key.
	 */
	{
		INIClass ini;
		Read(ini, "[Tutorial]\n120=First.\n0120=Second.\n");

		TutorialTextClass tutorial;
		tutorial.Read_Base(ini);

		Check(Line_Is(tutorial, 120, "Second."), "the later of two spellings of one number wins");
	}

	/*
	 * A scenario's lines stand in front of the base set, and go away with the scenario.
	 */
	{
		INIClass base;
		Read(base, "[Tutorial]\n120=Hold the ridge.\n121=Base only.\n");

		INIClass scenario;
		Read(scenario, "[Tutorial]\n120=Hold the bridge instead.\n200=A line of the mission's own.\n");

		TutorialTextClass tutorial;
		tutorial.Read_Base(base);
		tutorial.Read_Overrides(scenario);

		Check(Line_Is(tutorial, 120, "Hold the bridge instead."), "a scenario replaces a line the base set carries");
		Check(Line_Is(tutorial, 200, "A line of the mission's own."), "and adds one of its own");
		Check(Line_Is(tutorial, 121, "Base only."), "a base line the scenario leaves alone still reads");

		tutorial.Clear_Overrides();

		Check(Line_Is(tutorial, 120, "Hold the ridge."), "clearing the scenario's lines brings the base line back");
		Check(tutorial.Fetch(200) == NULL, "and takes the mission's own line away");
		Check(Line_Is(tutorial, 121, "Base only."), "while the rest of the base set is untouched");
	}

	/*
	 * A scenario with no section of its own changes nothing.
	 */
	{
		INIClass base;
		Read(base, "[Tutorial]\n120=Hold the ridge.\n");

		INIClass scenario;
		Read(scenario, "[Basic]\nName=A mission with no tutorial text.\n");

		TutorialTextClass tutorial;
		tutorial.Read_Base(base);
		tutorial.Read_Overrides(scenario);

		Check(Line_Is(tutorial, 120, "Hold the ridge."), "a scenario carrying no section leaves the base set alone");
	}

	/*
	 * A line cannot be blanked: the reader drops an entry with nothing after the "=", so it
	 * never reaches the table to replace anything.
	 */
	{
		INIClass base;
		Read(base, "[Tutorial]\n120=Hold the ridge.\n");

		INIClass scenario;
		Read(scenario, "[Tutorial]\n120=\n121=   \n");

		TutorialTextClass tutorial;
		tutorial.Read_Base(base);
		tutorial.Read_Overrides(scenario);

		Check(Line_Is(tutorial, 120, "Hold the ridge."), "a scenario cannot blank a base line");
		Check(tutorial.Fetch(121) == NULL, "and cannot add an empty one");
	}

	/*
	 * A line is held whole however long it is.
	 */
	{
		std::string const line(900, 'x');
		std::string const text = "[Tutorial]\n120=" + line + "\n";

		INIClass ini;
		Read(ini, text.c_str());

		TutorialTextClass tutorial;
		tutorial.Read_Base(ini);

		Check(Line_Is(tutorial, 120, line.c_str()), "a line far past the old buffer is held whole");
	}

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
