/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Drives the speech queue on its own: the order lines come out in, what a
// duplicate does, the cap, the interrupt classes, and the standard slot.
// Needs no game data.

#include "voxqueue.h"

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


VoxType Pop(VoxQueueClass & queue)
{
	VoxType voice = VOX_NONE;
	queue.Next(voice);
	return(voice);
}


void Test_Order(void)
{
	VoxQueueClass queue;
	Check(!queue.Submit(VOX_CONSTRUCTION, 50, VOXC_QUEUE, VOX_NONE), "a queued line does not cut");
	queue.Submit(VOX_UNIT_READY, 50, VOXC_QUEUE, VOX_NONE);
	queue.Submit(VOX_LOW_POWER, 50, VOXC_QUEUE, VOX_NONE);
	Check(queue.Count() == 3, "three waiting");
	Check(Pop(queue) == VOX_CONSTRUCTION && Pop(queue) == VOX_UNIT_READY && Pop(queue) == VOX_LOW_POWER, "equal priorities play in order");
	Check(queue.Count() == 0 && Pop(queue) == VOX_NONE, "empty afterwards");

	queue.Submit(VOX_CONSTRUCTION, 50, VOXC_QUEUE, VOX_NONE);
	queue.Submit(VOX_BASE_UNDER_ATTACK, 100, VOXC_QUEUE, VOX_NONE);
	queue.Submit(VOX_UNIT_READY, 50, VOXC_QUEUE, VOX_NONE);
	Check(Pop(queue) == VOX_BASE_UNDER_ATTACK, "higher priority first");
	Check(Pop(queue) == VOX_CONSTRUCTION && Pop(queue) == VOX_UNIT_READY, "then the rest in order");
}


void Test_Dedupe(void)
{
	VoxQueueClass queue;
	queue.Submit(VOX_CONSTRUCTION, 50, VOXC_QUEUE, VOX_NONE);
	queue.Submit(VOX_CONSTRUCTION, 50, VOXC_QUEUE, VOX_NONE);
	Check(queue.Count() == 1, "a waiting line is not queued twice");
	queue.Submit(VOX_UNIT_READY, 50, VOXC_QUEUE, VOX_UNIT_READY);
	Check(queue.Count() == 1 && !queue.Contains(VOX_UNIT_READY), "the playing line is not queued");
	Check(!queue.Submit(VOX_NONE, 50, VOXC_QUEUE, VOX_NONE) && queue.Count() == 1, "no line is nothing");
}


void Test_Cap(void)
{
	VoxQueueClass queue;
	for (int i = 0; i < VoxQueueClass::MAX_PENDING; i++) {
		queue.Submit((VoxType)(VOX_CONSTRUCTION + i), i == 3 ? 20 : 50, VOXC_QUEUE, VOX_NONE);
	}
	Check(queue.Count() == VoxQueueClass::MAX_PENDING, "full");
	queue.Submit(VOX_BASE_UNDER_ATTACK, 50, VOXC_QUEUE, VOX_NONE);
	Check(queue.Count() == VoxQueueClass::MAX_PENDING, "still full");
	Check(!queue.Contains((VoxType)(VOX_CONSTRUCTION + 3)) && queue.Contains(VOX_BASE_UNDER_ATTACK), "the lowest priority line was dropped");

	queue.Submit(VOX_ACCOMPLISHED, 255, VOXC_CRITICAL, VOX_NONE);
	Check(queue.Contains(VOX_ACCOMPLISHED) && !queue.Contains(VOX_CONSTRUCTION), "with equal priorities the oldest goes");
	Check(Pop(queue) == VOX_ACCOMPLISHED, "critical plays first");

	VoxQueueClass tiny;
	for (int i = 0; i < VoxQueueClass::MAX_PENDING; i++) {
		tiny.Submit((VoxType)(VOX_CONSTRUCTION + i), 255, VOXC_CRITICAL, VOX_NONE);
	}
	tiny.Submit(VOX_LOW_POWER, 50, VOXC_QUEUE, VOX_NONE);
	Check(tiny.Count() == VoxQueueClass::MAX_PENDING && tiny.Contains(VOX_LOW_POWER) && !tiny.Contains(VOX_CONSTRUCTION), "a full queue of critical lines gives up its oldest");
}


void Test_Interrupts(void)
{
	VoxQueueClass queue;
	queue.Submit(VOX_CONSTRUCTION, 50, VOXC_QUEUE, VOX_NONE);
	queue.Submit(VOX_UNIT_READY, 50, VOXC_QUEUE, VOX_NONE);
	Check(!queue.Submit(VOX_INCOMING_TRANSMISSION, 50, VOXC_QUEUED_INTERRUPT, VOX_LOW_POWER), "a line asked for now does not cut");
	Check(Pop(queue) == VOX_INCOMING_TRANSMISSION, "but it goes ahead of the queue");
	Check(Pop(queue) == VOX_CONSTRUCTION, "the queue follows");

	queue.Submit(VOX_LOW_POWER, 50, VOXC_QUEUE, VOX_NONE);
	Check(queue.Submit(VOX_FAIL, 255, VOXC_INTERRUPT, VOX_CONSTRUCTION), "an interrupt cuts the playing line");
	Check(queue.Count() == 1 && Pop(queue) == VOX_FAIL, "and empties the queue behind it");
	Check(!queue.Submit(VOX_FAIL, 255, VOXC_INTERRUPT, VOX_NONE), "an interrupt with nothing playing cuts nothing");

	queue.Clear();
	queue.Submit(VOX_CONSTRUCTION, 50, VOXC_QUEUED_INTERRUPT, VOX_NONE);
	queue.Submit(VOX_ACCOMPLISHED, 255, VOXC_CRITICAL, VOX_NONE);
	Check(Pop(queue) == VOX_ACCOMPLISHED && Pop(queue) == VOX_CONSTRUCTION, "critical beats a line asked for now");
}


void Test_Standard(void)
{
	VoxQueueClass queue;
	Check(!queue.Submit(VOX_CONSTRUCTION, 50, VOXC_STANDARD, VOX_NONE), "standard line queued");
	queue.Submit(VOX_UNIT_READY, 50, VOXC_STANDARD, VOX_NONE);
	Check(queue.Count() == 1 && queue.Contains(VOX_CONSTRUCTION), "an equal standard line is dropped");
	queue.Submit(VOX_BASE_UNDER_ATTACK, 100, VOXC_STANDARD, VOX_NONE);
	Check(queue.Count() == 1 && queue.Contains(VOX_BASE_UNDER_ATTACK), "a higher standard line replaces it");
	queue.Submit(VOX_LOW_POWER, 50, VOXC_QUEUE, VOX_NONE);
	Check(Pop(queue) == VOX_LOW_POWER && Pop(queue) == VOX_BASE_UNDER_ATTACK, "the standard slot plays after the queue");
}

} // namespace


int main(void)
{
	Test_Order();
	Test_Dedupe();
	Test_Cap();
	Test_Interrupts();
	Test_Standard();

	std::printf("voxqueue: %d checks, %d failures\n", Checked, Failures);
	return(Failures == 0 ? 0 : 1);
}
