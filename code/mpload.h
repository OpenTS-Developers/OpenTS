/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "autosave.h"

#include <cstdint>

/*
 * The numbered multiplayer save every machine has agreed to load, and when. Times are
 * milliseconds handed in, so it holds no engine state.
 */
class MultiplayerLoadClass
{
	public:

		static constexpr std::int64_t COUNTDOWN_MS = 5000;

		static bool Slot_Is_Valid(int slot) {return(slot >= 0 && slot < MULTIPLAYER_SAVE_SLOTS);}

		// Refuses an invalid slot and a second request while one is pending.
		bool Schedule(int slot, std::int64_t now);
		bool Is_Pending(void) const {return(IsPending);}
		bool Is_Due(std::int64_t now) const;

		// Rounded up, and never below one while a load is pending.
		int Seconds_Left(std::int64_t now) const;
		std::int64_t Milliseconds_Left(std::int64_t now) const;

		// The scheduled slot, or -1 while nothing is pending.
		int Slot(void) const {return(SlotNumber);}
		void Clear(void);

	private:

		bool IsPending = false;
		std::int64_t DueAt = 0;
		int SlotNumber = -1;
};

extern MultiplayerLoadClass MultiplayerLoad;

// The clock the countdown and the heartbeats are measured on.
std::int64_t Monotonic_Milliseconds(void);
