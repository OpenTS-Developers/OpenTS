/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins the automatic-save ring a client relies on: the name each slot is written under, the
// order the slots turn in, what a launch file may seed, and when the next save falls due.

#include <cstdio>
#include <string>

#include "autosave.h"

namespace {

using KindType = AutosaveClass::KindType;

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-64s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


std::string Next_Name(AutosaveClass & autosave, KindType kind)
{
	return(AutosaveClass::File_Name(kind, autosave.Advance(kind)));
}

}


int main(void)
{
	/*
	 * A fresh ring writes its first slot, named the way a client reads it back: the prefix, a
	 * single digit, and the save extension.
	 */
	{
		AutosaveClass autosave;

		Check(Next_Name(autosave, KindType::Campaign) == "AUTOSAVE1.SAV",
			"the first campaign autosave is slot one");
		Check(Next_Name(autosave, KindType::Skirmish) == "AUTOSAVE_SKIRMISH1.SAV",
			"a skirmish autosave keeps the prefix a client files by");
	}

	/*
	 * The ring turns through its five slots and starts over, and each ring turns alone.
	 */
	{
		AutosaveClass autosave;
		std::string names;

		for (int index = 0; index <= AutosaveClass::SLOT_COUNT; index++) {
			names += Next_Name(autosave, KindType::Campaign) + " ";
		}

		Check(names == "AUTOSAVE1.SAV AUTOSAVE2.SAV AUTOSAVE3.SAV AUTOSAVE4.SAV AUTOSAVE5.SAV AUTOSAVE1.SAV ",
			"the campaign slots turn in order and start over");
		Check(autosave.Campaign_Slot() == 1, "a save records the slot that follows the one written");
		Check(autosave.Skirmish_Slot() == 0, "turning the campaign ring leaves the skirmish ring alone");
	}

	/*
	 * A launch file seeds where each ring continues from, and a slot the ring does not hold
	 * starts it over rather than naming a file outside the ring.
	 */
	{
		AutosaveClass autosave;

		autosave.Seed_Slots(4, 2);
		Check(Next_Name(autosave, KindType::Campaign) == "AUTOSAVE5.SAV" &&
			Next_Name(autosave, KindType::Campaign) == "AUTOSAVE1.SAV",
			"a seeded campaign ring continues from the slot named");
		Check(Next_Name(autosave, KindType::Skirmish) == "AUTOSAVE_SKIRMISH3.SAV",
			"a seeded skirmish ring continues from its own slot");

		autosave.Seed_Slots(-2, AutosaveClass::SLOT_COUNT);
		Check(autosave.Campaign_Slot() == 0 && autosave.Skirmish_Slot() == 0,
			"a seed the ring does not hold starts it over");
	}

	/*
	 * A save falls due one interval after the last one was scheduled, is armed once, and is
	 * pushed a full interval out by any save that completes in between.
	 */
	{
		AutosaveClass autosave;

		autosave.Schedule(100);
		Check(!autosave.Is_Due(100000), "no interval means no save ever falls due");

		autosave.Set_Interval(300);
		autosave.Schedule(100);
		Check(!autosave.Is_Due(399) && autosave.Is_Due(400) && autosave.Is_Due(401),
			"a save falls due one interval after it was scheduled");

		autosave.Arm();
		Check(!autosave.Is_Due(400), "an armed request is not due again");
		Check(autosave.Take_Armed() && !autosave.Take_Armed(), "an armed request is taken once");

		autosave.Arm();
		autosave.Schedule(400);
		Check(!autosave.Take_Armed(), "a completed save drops an armed request");
		Check(!autosave.Is_Due(699) && autosave.Is_Due(700),
			"a completed save pushes the next one a full interval out");

		autosave.Set_Interval(0);
		autosave.Schedule(700);
		Check(!autosave.Is_Due(100000), "turning the interval off stops the saves");

		autosave.Set_Interval(-5);
		Check(autosave.Interval() == 0, "an interval below zero is off");

		autosave.Arm();
		autosave.Arm();
		Check(autosave.Take_Armed() && !autosave.Take_Armed(),
			"explicit requests coalesce with the timed interval disabled");
		Check(!autosave.Is_Due(100000), "an explicit request does not enable timed saves");
	}

	/*
	 * A quick save keeps one file per kind of game, so a skirmish never writes over a campaign.
	 */
	Check(Quick_Save_File_Name(KindType::Campaign) == "QUICKSAVE.SAV",
		"the campaign quick save has one fixed name");
	Check(Quick_Save_File_Name(KindType::Skirmish) == "QUICKSAVE_SKIRMISH.SAV",
		"the skirmish quick save keeps a file of its own");

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
