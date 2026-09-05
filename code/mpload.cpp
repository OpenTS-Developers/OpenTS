/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "mpload.h"

#include <cctype>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstring>


bool MultiplayerLoadClass::Name_Is_Valid(char const * name)
{
	if (name == NULL) {
		return(false);
	}

	std::size_t length = std::strlen(name);
	if (length > NAME_MAX - 1) {
		return(false);
	}

	char const * dot = std::strchr(name, '.');
	if (dot == NULL || dot == name || dot - name > 8) {
		return(false);
	}
	for (char const * c = name; c < dot; c++) {
		if (!std::isalnum(static_cast<unsigned char>(*c)) && *c != '_' && *c != '-') {
			return(false);
		}
	}
	if (_stricmp(dot + 1, "NET") != 0) {
		return(false);
	}

	return(_stricmp(name, "SAVEGAME.NET") != 0);
}


bool MultiplayerLoadClass::Schedule(char const * name, std::int64_t now)
{
	if (IsPending || !Name_Is_Valid(name)) {
		return(false);
	}

	std::snprintf(FileName, sizeof(FileName), "%s", name);
	DueAt = now + COUNTDOWN_MS;
	IsPending = true;
	return(true);
}


bool MultiplayerLoadClass::Is_Due(std::int64_t now) const
{
	return(IsPending && now >= DueAt);
}


int MultiplayerLoadClass::Seconds_Left(std::int64_t now) const
{
	if (!IsPending) {
		return(0);
	}

	int seconds = (int)((Milliseconds_Left(now) + 999) / 1000);
	return(seconds < 1 ? 1 : seconds);
}


std::int64_t MultiplayerLoadClass::Milliseconds_Left(std::int64_t now) const
{
	if (!IsPending || DueAt <= now) {
		return(0);
	}
	return(DueAt - now);
}


void MultiplayerLoadClass::Clear(void)
{
	IsPending = false;
	DueAt = 0;
	FileName[0] = '\0';
}


std::int64_t Monotonic_Milliseconds(void)
{
	return(std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now().time_since_epoch()).count());
}
