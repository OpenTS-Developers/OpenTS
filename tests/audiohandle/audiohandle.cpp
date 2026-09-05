/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Checks the audio handle's packing: the index and generation round-trip, the
// null value is distinct from every made handle, generations skip zero when they
// wrap, and the raw value is never negative as an int. Needs no game data.

#include "audio/audiohandle.h"

#include <cstdio>

namespace {

int Failures = 0;
int Checked = 0;


void Check(bool condition, char const * what)
{
	Checked++;
	if (!condition) {
		Failures++;
		std::printf("FAIL: %s\n", what);
	}
}


void Test_Null(void)
{
	AudioHandle handle;
	Check(handle.Is_Null(), "default handle is null");
	Check(handle.Raw() == 0, "null handle is raw zero");
	Check(handle == AudioHandle(), "null handles compare equal");
	Check(!AudioHandle::Make(0, 1).Is_Null(), "index 0 with generation 1 is not null");

	AudioHandle made = AudioHandle::Make(5, 9);
	made.Clear();
	Check(made.Is_Null(), "clear makes a handle null");
}


void Test_Round_Trip(void)
{
	for (unsigned index = 0; index <= AudioHandle::INDEX_MASK; index += 17) {
		for (unsigned generation = 1; generation <= AudioHandle::GENERATION_MASK; generation += 100003) {
			AudioHandle handle = AudioHandle::Make(index, generation);
			Check(handle.Index() == index, "index round-trips");
			Check(handle.Generation() == generation, "generation round-trips");
			Check((int)handle.Raw() >= 0, "raw value is non-negative as int");
			Check(AudioHandle::From_Raw(handle.Raw()) == handle, "raw value round-trips");
		}
	}

	AudioHandle top = AudioHandle::Make(AudioHandle::INDEX_MASK, AudioHandle::GENERATION_MASK);
	Check(top.Index() == AudioHandle::INDEX_MASK && top.Generation() == AudioHandle::GENERATION_MASK, "extreme values round-trip");
	Check((int)top.Raw() >= 0, "extreme value keeps bit 31 clear");

	Check(AudioHandle::Make(3, 4) != AudioHandle::Make(3, 5), "different generations differ");
	Check(AudioHandle::Make(3, 4) != AudioHandle::Make(4, 4), "different indices differ");
	Check(AudioHandle::Make(300, 4) == AudioHandle::Make(300 & AudioHandle::INDEX_MASK, 4), "index is masked");
}


void Test_Generation(void)
{
	Check(AudioHandle::Next_Generation(1) == 2, "generation advances");
	Check(AudioHandle::Next_Generation(0) == 1, "generation leaves zero");
	Check(AudioHandle::Next_Generation(AudioHandle::GENERATION_MASK) == 1, "generation wraps past zero");
	Check(AudioHandle::Next_Generation(AudioHandle::GENERATION_MASK - 1) == AudioHandle::GENERATION_MASK, "generation reaches the top value");

	// A slot handed out, recycled, and handed out again yields a different handle.
	unsigned generation = 1;
	AudioHandle first = AudioHandle::Make(7, generation);
	generation = AudioHandle::Next_Generation(generation);
	AudioHandle second = AudioHandle::Make(7, generation);
	Check(first != second, "reused slot yields a different handle");
	Check(first.Index() == second.Index(), "reused slot keeps its index");
}

} // namespace


int main(void)
{
	Test_Null();
	Test_Round_Trip();
	Test_Generation();

	std::printf("audiohandle: %d checks, %d failures\n", Checked, Failures);
	return(Failures == 0 ? 0 : 1);
}
