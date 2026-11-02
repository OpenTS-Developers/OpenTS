/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "gametext.h"


/// <summary>
/// Gives text written for a wrapping text display with its line breaks as newlines: an at
/// sign becomes a newline, a backslash and an at sign become an at sign, and two backslashes
/// become one. Any other backslash is kept.
/// </summary>
std::string Game_Text_Lines(std::string_view text)
{
	std::string lines;
	lines.reserve(text.size());

	for (std::size_t index = 0; index < text.size(); index++) {
		char const c = text[index];
		if (c == '\\' && index + 1 < text.size() && (text[index + 1] == '@' || text[index + 1] == '\\')) {
			lines.push_back(text[++index]);
		} else {
			lines.push_back((c == '@') ? '\n' : c);
		}
	}

	return(lines);
}


/// <summary>
/// Rewrites a null-terminated text in place as Game_Text_Lines does. The text never grows.
/// </summary>
void Game_Text_Lines_In_Place(char * text)
{
	if (text == nullptr) {
		return;
	}

	char * out = text;
	for (char const * in = text; *in != '\0'; in++) {
		if (*in == '\\' && (in[1] == '@' || in[1] == '\\')) {
			*out++ = *++in;
		} else {
			*out++ = (*in == '@') ? '\n' : *in;
		}
	}
	*out = '\0';
}
