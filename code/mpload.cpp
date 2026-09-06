/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "mpload.h"

#include <chrono>


bool MultiplayerLoadClass::Schedule(int slot, std::int64_t now)
{
	if (IsPending || !Slot_Is_Valid(slot)) {
		return(false);
	}

	SlotNumber = slot;
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
	SlotNumber = -1;
}


std::int64_t Monotonic_Milliseconds(void)
{
	return(std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now().time_since_epoch()).count());
}
