/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "tutorial.h"

#include "dbgprint.h"
#include "ini.h"

#include <charconv>
#include <cstring>
#include <system_error>

/***************************************************************************
**	There are various tutorial messages that can appear in the game. These
**	are called upon by number and pointed to by this array.
*/
TutorialTextClass TutorialText;

static char const * const TUTORIAL_SECTION = "Tutorial";


static bool Parse_Line_Number(char const * entry, int & id)
{
	if (entry == NULL) {
		return(false);
	}

	char const * const end = entry + strlen(entry);
	std::from_chars_result const result = std::from_chars(entry, end, id);

	return(result.ec == std::errc() && result.ptr == end);
}


void TutorialTextClass::Read_Section(INIClass const & ini, LineList & into)
{
	int count = ini.Entry_Count(TUTORIAL_SECTION);

	for (int index = 0; index < count; index++) {
		char const * entry = ini.Get_Entry(TUTORIAL_SECTION, index);
		int id = 0;

		if (!Parse_Line_Number(entry, id)) {
			DebugString("Tutorial: %s is not a line number and was skipped.\n", entry != NULL ? entry : "");
			continue;
		}

		// A line with nothing after the "=" never became an entry, so no line can be blanked.
		std::string text = ini.Get_String(TUTORIAL_SECTION, entry);

		bool replaced = false;
		for (auto & line : into) {
			if (line.first == id) {
				line.second = std::move(text);
				replaced = true;
				break;
			}
		}

		if (!replaced) {
			into.emplace_back(id, std::move(text));
		}
	}
}


char const * TutorialTextClass::Find(LineList const & lines, int id)
{
	for (auto const & line : lines) {
		if (line.first == id) {
			return(line.second.c_str());
		}
	}

	return(NULL);
}


/// <summary>
/// Takes the lines a [Tutorial] section carries into the set the game starts with, replacing
/// any it already holds. An entry named by anything but a whole number is refused.
/// </summary>
void TutorialTextClass::Read_Base(INIClass const & ini)
{
	Read_Section(ini, Base);
}


/// <summary>
/// Takes the lines a scenario's [Tutorial] section carries. These stand in front of the base
/// set until the scenario is cleared.
/// </summary>
void TutorialTextClass::Read_Overrides(INIClass const & ini)
{
	Read_Section(ini, Overrides);
}


void TutorialTextClass::Clear_Overrides(void)
{
	Overrides.clear();
	Overrides.shrink_to_fit();
}


/// <summary>
/// The line this number names, from the scenario's own lines when it has one and the base set
/// otherwise.
/// </summary>
/// <returns>The line, or NULL when neither carries the number. It is valid until the next
/// read.</returns>
char const * TutorialTextClass::Fetch(int id) const
{
	char const * text = Find(Overrides, id);

	return(text != NULL ? text : Find(Base, id));
}
