/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Tests the synchronized generator's read-only accessors: the frame checksum reads them every
// frame, so reading must leave the random sequence unchanged.
//
// Needs no game data.

#include "random.h"
#include "syncrechook.h"

#include "doctest/doctest.h"

SyncRecorderClass SyncRecorder;

void Sync_Record_Random_Impl(Random2Class const &, int, int, int, bool, unsigned)
{
}

TEST_CASE("Peek returns the next draw")
{
	Random2Class generator(0x1234);

	for (int draw = 0; draw < 3 * Random2Class::TABLE_SIZE; draw++) {
		int const peeked = generator.Peek();
		CHECK(generator.Peek() == peeked);
		CHECK(generator() == peeked);
	}
}

TEST_CASE("Reading the position and next value leaves the sequence unchanged")
{
	Random2Class read(77);
	Random2Class untouched(77);

	for (int draw = 0; draw < 3 * Random2Class::TABLE_SIZE; draw++) {
		int const index = read.Index_1();
		static_cast<void>(read.Peek());
		static_cast<void>(read.Index_2());
		CHECK(read.Index_1() == index);
		CHECK(read() == untouched());
	}
}
