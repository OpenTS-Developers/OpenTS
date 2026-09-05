/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstdint>

/*
 * The multiplayer save every machine has agreed to load, and when. Times are milliseconds
 * handed in, so it holds no engine state.
 */
class MultiplayerLoadClass
{
	public:

		static constexpr std::int64_t COUNTDOWN_MS = 5000;

		// An 8.3 name and its terminator, which is all a request carries.
		static constexpr int NAME_MAX = 13;

		// A bare 8.3 name ending in .NET that is not the fixed multiplayer save.
		static bool Name_Is_Valid(char const * name);

		// Refuses an invalid name and a second request while one is pending.
		bool Schedule(char const * name, std::int64_t now);
		bool Is_Pending(void) const {return(IsPending);}
		bool Is_Due(std::int64_t now) const;

		// Rounded up, and never below one while a load is pending.
		int Seconds_Left(std::int64_t now) const;
		std::int64_t Milliseconds_Left(std::int64_t now) const;

		char const * File_Name(void) const {return(FileName);}
		void Clear(void);

	private:

		bool IsPending = false;
		std::int64_t DueAt = 0;
		char FileName[NAME_MAX] = "";
};

extern MultiplayerLoadClass MultiplayerLoad;

// The clock the countdown and the heartbeats are measured on.
std::int64_t Monotonic_Milliseconds(void);
