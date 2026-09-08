/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uisystem.h"

#include "dbgprint.h"
#include "win.h"


UISystemInterfaceClass::UISystemInterfaceClass(void) :
	StartTime(timeGetTime())
{
}


// UI animation follows the wall clock, never the game's deterministic timers.
double UISystemInterfaceClass::GetElapsedTime(void)
{
	return((double)(timeGetTime() - StartTime) / 1000.0);
}


bool UISystemInterfaceClass::LogMessage(Rml::Log::Type type, Rml::String const & message)
{
	char const * level = "info";

	switch (type) {
		case Rml::Log::LT_ERROR:
			level = "error";
			break;

		case Rml::Log::LT_ASSERT:
			level = "assert";
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

	DebugString("UI %s: %s\n", level, message.c_str());
	return(true);
}


// Documents name their resources by bare file name, so one name resolves the same way from
// the ui directory, a loose override or a mix.
void UISystemInterfaceClass::JoinPath(Rml::String & translated, Rml::String const &, Rml::String const & path)
{
	size_t start = path.find_last_of("/\\");
	translated = (start == Rml::String::npos) ? path : path.substr(start + 1);
}
