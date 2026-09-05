/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Exercises the out-of-sync bookkeeping and the pending multiplayer load without the engine.

#include "desync.h"
#include "mpload.h"

#include <cstdio>
#include <cstring>


namespace {

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-64s %s\n", what, condition ? "ok" : "FAILED");
	if (!condition) {
		Failures++;
	}
}

}	// namespace


int main(void)
{
	/*
	 * Nobody is silent when the dialog opens, and a heartbeat pushes the timeout out.
	 */
	{
		DesyncClass desync;
		desync.Begin(1000);
		Check(!desync.Is_Silent(2, 1000 + DesyncClass::HEARTBEAT_TIMEOUT_MS), "a house is not silent at the timeout");
		Check(desync.Is_Silent(2, 1000 + DesyncClass::HEARTBEAT_TIMEOUT_MS + 1), "a house is silent past the timeout");
		desync.Heard(2, 20000);
		Check(!desync.Is_Silent(2, 20000 + DesyncClass::HEARTBEAT_TIMEOUT_MS), "a heartbeat pushes the timeout out");
		Check(desync.Is_Silent(3, 20000 + DesyncClass::HEARTBEAT_TIMEOUT_MS), "another house keeps its own clock");
		Check(!desync.Is_Silent(8, 999999), "a house outside the table is never silent");
		Check(!desync.Is_Silent(-1, 999999), "a negative house is never silent");
	}

	/*
	 * The first heartbeat is due at once and the next one an interval later.
	 */
	{
		DesyncClass desync;
		desync.Begin(5000);
		Check(desync.Heartbeat_Is_Due(5000), "the first heartbeat is due when the dialog opens");
		desync.Heartbeat_Sent(5000);
		Check(!desync.Heartbeat_Is_Due(5000 + DesyncClass::HEARTBEAT_INTERVAL_MS - 1), "no heartbeat inside the interval");
		Check(desync.Heartbeat_Is_Due(5000 + DesyncClass::HEARTBEAT_INTERVAL_MS), "a heartbeat is due at the interval");
	}

	/*
	 * A departed player's name survives, and Begin forgets it.
	 */
	{
		DesyncClass desync;
		desync.Begin(0);
		desync.Mark_Left(4, "Rampa");
		Check(desync.Has_Left(4), "a marked house has left");
		Check(std::strcmp(desync.Left_Name(4), "Rampa") == 0, "the departed name is kept");
		desync.Mark_Left(4, "");
		Check(std::strcmp(desync.Left_Name(4), "Rampa") == 0, "an empty name does not replace the kept one");
		Check(!desync.Has_Left(1), "an unmarked house has not left");
		Check(std::strcmp(desync.Left_Name(9), "") == 0, "a house outside the table has no name");
		desync.Begin(1);
		Check(!desync.Has_Left(4) && desync.Left_Name(4)[0] == '\0', "Begin forgets who left");
	}

	/*
	 * Only a slot the numbered saves can hold may be requested.
	 */
	{
		Check(MultiplayerLoadClass::Slot_Is_Valid(0), "the first slot is valid");
		Check(MultiplayerLoadClass::Slot_Is_Valid(MULTIPLAYER_SAVE_SLOTS - 1), "the last slot is valid");
		Check(!MultiplayerLoadClass::Slot_Is_Valid(MULTIPLAYER_SAVE_SLOTS), "a slot past the last is refused");
		Check(!MultiplayerLoadClass::Slot_Is_Valid(-1), "a negative slot is refused");
	}

	/*
	 * A load is due a countdown after it is scheduled, and only one can be pending.
	 */
	{
		MultiplayerLoadClass load;
		Check(!load.Is_Pending(), "nothing is pending at first");
		Check(load.Seconds_Left(0) == 0, "nothing pending has no seconds left");
		Check(load.Slot() == -1, "nothing pending has no slot");
		Check(!load.Schedule(-1, 100), "an invalid slot cannot be scheduled");
		Check(load.Schedule(3, 100), "a valid slot is scheduled");
		Check(load.Is_Pending() && load.Slot() == 3, "the scheduled slot is kept");
		Check(!load.Schedule(4, 200), "a second request while pending is refused");
		Check(load.Slot() == 3, "the refused request changes nothing");
		Check(!load.Is_Due(100 + MultiplayerLoadClass::COUNTDOWN_MS - 1), "not due inside the countdown");
		Check(load.Is_Due(100 + MultiplayerLoadClass::COUNTDOWN_MS), "due when the countdown ends");
		Check(load.Seconds_Left(100) == 5, "five seconds at the start");
		Check(load.Seconds_Left(100 + 1) == 5, "still five just after the start");
		Check(load.Seconds_Left(100 + 4001) == 1, "one second near the end");
		Check(load.Seconds_Left(100 + MultiplayerLoadClass::COUNTDOWN_MS) == 1, "never below one while pending");
		Check(load.Seconds_Left(100 + MultiplayerLoadClass::COUNTDOWN_MS + 5000) == 1, "never below one when overdue");
		load.Clear();
		Check(!load.Is_Pending() && load.Slot() == -1, "Clear ends the pending load");
		Check(load.Schedule(4, 300), "a load can be scheduled again after Clear");
	}

	std::printf("\n%s\n", Failures == 0 ? "All checks passed." : "Some checks FAILED.");
	return(Failures == 0 ? 0 : 1);
}
