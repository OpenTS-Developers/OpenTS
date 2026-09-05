/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Exercises the single-producer, single-consumer rings the audio engine hands
// commands and PCM through: capacity limits, wrap-around, partial reads, the
// frame counters, and sequence integrity with a producer and a consumer thread.
// Needs no game data.

#include "audio/audioring.h"

#include <cstdio>
#include <thread>
#include <vector>

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


struct Item {
	uint32_t Sequence;
	uint32_t Payload;
};


void Test_Command_Ring_Basics(void)
{
	SpscRingClass<Item, 8> ring;

	Check(ring.Is_Empty(), "new ring is empty");
	Check(ring.Capacity() == 8, "capacity reports N");

	for (uint32_t i = 0; i < 8; i++) {
		Check(ring.Push(Item{i, i * 3}), "push fits until capacity");
	}
	Check(!ring.Push(Item{99, 99}), "push fails when full");
	Check(ring.Count() == 8, "count at capacity");

	Item item;
	for (uint32_t i = 0; i < 8; i++) {
		Check(ring.Pop(item), "pop until empty");
		Check(item.Sequence == i && item.Payload == i * 3, "pop returns items in order");
	}
	Check(!ring.Pop(item), "pop fails when empty");

	// Push and pop across the wrap point many times.
	uint32_t next = 100;
	uint32_t expect = 100;
	for (int round = 0; round < 50; round++) {
		for (int i = 0; i < 5; i++) {
			Check(ring.Push(Item{next, 0}), "push after wrap");
			next++;
		}
		for (int i = 0; i < 5; i++) {
			Check(ring.Pop(item) && item.Sequence == expect, "pop after wrap keeps order");
			expect++;
		}
	}
}


void Test_Command_Ring_Threads(void)
{
	SpscRingClass<Item, 64> ring;
	uint32_t const TOTAL = 200000;
	bool ordered = true;
	bool consistent = true;

	std::thread consumer([&]() {
		uint32_t expect = 0;
		Item item;
		while (expect < TOTAL) {
			if (ring.Pop(item)) {
				if (item.Sequence != expect) ordered = false;
				if (item.Payload != item.Sequence * 7u + 1u) consistent = false;
				expect++;
			}
		}
	});

	for (uint32_t i = 0; i < TOTAL; ) {
		if (ring.Push(Item{i, i * 7u + 1u})) {
			i++;
		}
	}
	consumer.join();

	Check(ordered, "threaded sequence stays in order");
	Check(consistent, "threaded payloads are read whole");
}


void Test_Pcm_Ring_Basics(void)
{
	PcmRingClass ring;

	Check(!ring.Init(0, 2), "init refuses zero frames");
	Check(ring.Init(100, 2), "init succeeds");
	Check(ring.Capacity() == 128, "capacity rounds up to a power of two");
	Check(ring.Channels() == 2, "channels stored");
	Check(ring.Available_Read() == 0 && ring.Available_Write() == 128, "new ring is empty");

	int16_t frames[2 * 128];
	for (int i = 0; i < 2 * 128; i++) {
		frames[i] = (int16_t)i;
	}

	Check(ring.Write(frames, 100) == 100, "write fits");
	Check(ring.Available_Read() == 100 && ring.Available_Write() == 28, "counts after write");
	Check(ring.Write(frames, 50) == 28, "write clamps to the room left");
	Check(ring.Frames_Pushed() == 128, "pushed counter");

	int16_t out[2 * 128];
	Check(ring.Read(out, 60) == 60, "partial read");
	Check(out[0] == 0 && out[119] == 119, "read returns the earliest frames");
	Check(ring.Frames_Consumed() == 60, "consumed counter");
	Check(ring.Read(out, 200) == 68, "read clamps to what is available");
	Check(out[0] == 120, "read continues where it left off");
	Check(ring.Available_Read() == 0, "empty after draining");
	Check(ring.Read(out, 10) == 0, "read on empty ring returns zero");

	// Wrap: the write cursor is at 128, so the next write starts at slot 0.
	Check(ring.Write(frames, 96) == 96, "write across the wrap point");
	Check(ring.Read(out, 96) == 96, "read across the wrap point");
	bool intact = true;
	for (int i = 0; i < 2 * 96; i++) {
		if (out[i] != frames[i]) intact = false;
	}
	Check(intact, "data survives the wrap");

	Check(ring.Write(frames, 40) == 40, "write for discard");
	Check(ring.Discard(15) == 15, "discard drops frames");
	Check(ring.Available_Read() == 25, "count after discard");
	Check(ring.Discard(100) == 25, "discard clamps to what is available");
	Check(ring.Frames_Consumed() == 128 + 96 + 40, "consumed counter keeps counting past capacity");

	ring.Reset();
	Check(ring.Frames_Consumed() == 0 && ring.Frames_Pushed() == 0, "reset zeroes both counters");
}


void Test_Pcm_Ring_Threads(void)
{
	PcmRingClass ring;
	Check(ring.Init(1024, 1), "threaded ring init");

	uint32_t const TOTAL = 500000;
	bool intact = true;

	std::thread consumer([&]() {
		int16_t out[300];
		uint32_t expect = 0;
		while (expect < TOTAL) {
			unsigned got = ring.Read(out, 300);
			for (unsigned i = 0; i < got; i++) {
				if (out[i] != (int16_t)(expect & 0x7FFF)) intact = false;
				expect++;
			}
		}
	});

	int16_t in[257];
	uint32_t next = 0;
	while (next < TOTAL) {
		unsigned count = 257;
		if (TOTAL - next < count) count = TOTAL - next;
		for (unsigned i = 0; i < count; i++) {
			in[i] = (int16_t)((next + i) & 0x7FFF);
		}
		unsigned written = ring.Write(in, count);
		next += written;
		if (written < count) {
			// The consumer is behind; the remainder is regenerated on the next pass.
		}
	}
	consumer.join();

	Check(intact, "threaded PCM stream arrives whole and in order");
}

} // namespace


int main(void)
{
	Test_Command_Ring_Basics();
	Test_Command_Ring_Threads();
	Test_Pcm_Ring_Basics();
	Test_Pcm_Ring_Threads();

	std::printf("audioring: %d checks, %d failures\n", Checked, Failures);
	return(Failures == 0 ? 0 : 1);
}
