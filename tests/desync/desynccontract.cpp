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
	 * Only a bare 8.3 .NET name other than the fixed one may be requested.
	 */
	{
		Check(MultiplayerLoadClass::Name_Is_Valid("SVGM_000.NET"), "a numbered client save is valid");
		Check(MultiplayerLoadClass::Name_Is_Valid("a.net"), "a short lower-case name is valid");
		Check(MultiplayerLoadClass::Name_Is_Valid("ABCDEFGH.NET"), "an eight-character base is valid");
		Check(!MultiplayerLoadClass::Name_Is_Valid("ABCDEFGHI.NET"), "a nine-character base is refused");
		Check(!MultiplayerLoadClass::Name_Is_Valid("SAVEGAME.NET"), "the fixed multiplayer save is refused");
		Check(!MultiplayerLoadClass::Name_Is_Valid("savegame.net"), "the fixed name is refused in any case");
		Check(!MultiplayerLoadClass::Name_Is_Valid("..\\X.NET"), "a parent path is refused");
		Check(!MultiplayerLoadClass::Name_Is_Valid("A/B.NET"), "a separator is refused");
		Check(!MultiplayerLoadClass::Name_Is_Valid("X.SAV"), "another extension is refused");
		Check(!MultiplayerLoadClass::Name_Is_Valid("X.NETS"), "a longer extension is refused");
		Check(!MultiplayerLoadClass::Name_Is_Valid("X"), "a name without an extension is refused");
		Check(!MultiplayerLoadClass::Name_Is_Valid(".NET"), "an empty base is refused");
		Check(!MultiplayerLoadClass::Name_Is_Valid(""), "an empty name is refused");
		Check(!MultiplayerLoadClass::Name_Is_Valid(NULL), "a null name is refused");
		Check(!MultiplayerLoadClass::Name_Is_Valid("A B.NET"), "a space is refused");
	}

	/*
	 * A load is due a countdown after it is scheduled, and only one can be pending.
	 */
	{
		MultiplayerLoadClass load;
		Check(!load.Is_Pending(), "nothing is pending at first");
		Check(load.Seconds_Left(0) == 0, "nothing pending has no seconds left");
		Check(!load.Schedule("SAVEGAME.NET", 100), "the fixed name cannot be scheduled");
		Check(load.Schedule("SVGM_003.NET", 100), "a valid name is scheduled");
		Check(load.Is_Pending() && std::strcmp(load.File_Name(), "SVGM_003.NET") == 0, "the scheduled name is kept");
		Check(!load.Schedule("SVGM_004.NET", 200), "a second request while pending is refused");
		Check(std::strcmp(load.File_Name(), "SVGM_003.NET") == 0, "the refused request changes nothing");
		Check(!load.Is_Due(100 + MultiplayerLoadClass::COUNTDOWN_MS - 1), "not due inside the countdown");
		Check(load.Is_Due(100 + MultiplayerLoadClass::COUNTDOWN_MS), "due when the countdown ends");
		Check(load.Seconds_Left(100) == 5, "five seconds at the start");
		Check(load.Seconds_Left(100 + 1) == 5, "still five just after the start");
		Check(load.Seconds_Left(100 + 4001) == 1, "one second near the end");
		Check(load.Seconds_Left(100 + MultiplayerLoadClass::COUNTDOWN_MS) == 1, "never below one while pending");
		Check(load.Seconds_Left(100 + MultiplayerLoadClass::COUNTDOWN_MS + 5000) == 1, "never below one when overdue");
		load.Clear();
		Check(!load.Is_Pending() && load.File_Name()[0] == '\0', "Clear ends the pending load");
		Check(load.Schedule("SVGM_004.NET", 300), "a load can be scheduled again after Clear");
	}

	std::printf("\n%s\n", Failures == 0 ? "All checks passed." : "Some checks FAILED.");
	return(Failures == 0 ? 0 : 1);
}
