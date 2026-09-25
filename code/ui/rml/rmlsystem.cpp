/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/rml/rmlsystem.h"

#include "opents_strings.h"
#include "ui/uihost.h"
#include "ui/uiunicode.h"

#include <cstdio>
#include <cstring>


UIRmlSystemClass::UIRmlSystemClass(UIShellHostClass & host) :
	Host(host),
	Start(std::chrono::steady_clock::now())
{
}


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


void UIRmlSystemClass::SetMouseCursor(Rml::String const & name)
{
	if (name == "text") {
		Cursor = UI_CURSOR_TEXT;
	} else if (name == "pointer") {
		Cursor = UI_CURSOR_HAND;
	} else if (name == "move") {
		Cursor = UI_CURSOR_MOVE;
	} else if (name == "not-allowed") {
		Cursor = UI_CURSOR_UNAVAILABLE;
	} else {
		Cursor = UI_CURSOR_ARROW;
	}
}


void UIRmlSystemClass::SetClipboardText(Rml::String const & text)
{
	Host.Set_Clipboard_Text(text);
}


void UIRmlSystemClass::GetClipboardText(Rml::String & text)
{
	text = Host.Clipboard_Text();
	if (text.size() > UI_CLIPBOARD_MAX_BYTES) {
		text.clear();
	}
}
