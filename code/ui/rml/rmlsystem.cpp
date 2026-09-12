/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/rml/rmlsystem.h"

#include "ui/uihost.h"

#include "opents_strings.h"

#include <cstdio>
#include <cstring>


UIRmlSystemClass::UIRmlSystemClass(UIShellHostClass & host) :
	Host(host),
	Start(std::chrono::steady_clock::now())
{
}


// UI animation follows the wall clock, never the game's deterministic timers.
double UIRmlSystemClass::GetElapsedTime(void)
{
	return(std::chrono::duration<double>(std::chrono::steady_clock::now() - Start).count());
}


bool UIRmlSystemClass::LogMessage(Rml::Log::Type type, Rml::String const & message)
{
	char const * level = "info";

	switch (type) {
		case Rml::Log::LT_ERROR:
			level = "error";
			Errors++;
			break;

		case Rml::Log::LT_ASSERT:
			level = "assert";
			Errors++;
			break;

		case Rml::Log::LT_WARNING:
			level = "warning";
			break;

		case Rml::Log::LT_DEBUG:
			level = "debug";
			break;

		default:
			break;
	}

	char line[1024];
	std::snprintf(line, sizeof(line), "UI %s: %s\n", level, message.c_str());
	Host.Log(line);
	return(true);
}


// Documents name their resources by bare file name, so one name resolves the same way from
// the ui directory, a loose override or a mix.
void UIRmlSystemClass::JoinPath(Rml::String & translated, Rml::String const &, Rml::String const & path)
{
	size_t start = path.find_last_of("/\\");
	translated = (start == Rml::String::npos) ? path : path.substr(start + 1);
}


static int String_Id(Rml::String const & name)
{
	for (OpenTSStringName const & entry : OpenTSStringNames) {
		if (std::strcmp(entry.Name, name.c_str()) == 0) {
			return(entry.Id);
		}
	}
	return(-1);
}


// A document names an engine string as [[TXT_NAME]]. An unknown name stays as typed so that
// it shows where it was written. The host's string is valid only until its next call, so
// the text is copied out at once.
int UIRmlSystemClass::TranslateString(Rml::String & translated, Rml::String const & input)
{
	int count = 0;
	size_t from = 0;

	translated.clear();

	while (from < input.size()) {
		size_t open = input.find("[[", from);
		size_t close = (open == Rml::String::npos) ? Rml::String::npos : input.find("]]", open + 2);

		if (close == Rml::String::npos) {
			translated.append(input, from, Rml::String::npos);
			break;
		}

		translated.append(input, from, open - from);

		Rml::String name = input.substr(open + 2, close - open - 2);
		int id = String_Id(name);

		if (id >= 0) {
			translated.append(Host.String(id));
			count++;
		} else {
			char line[256];
			std::snprintf(line, sizeof(line), "UI: no string named %s\n", name.c_str());
			Host.Log(line);
			translated.append(input, open, close + 2 - open);
		}

		from = close + 2;
	}

	return(count);
}
