/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "voxqueue.h"


VoxQueueClass::VoxQueueClass(void) :
	Pending(0),
	Serial(0)
{
}


int VoxQueueClass::Rank(VoxControlType control)
{
	switch (control) {
		case VOXC_CRITICAL:
			return(0);
		case VOXC_QUEUED_INTERRUPT:
		case VOXC_INTERRUPT:
			return(1);
		case VOXC_QUEUE:
			return(2);
		default:
			return(3);
	}
}


// True when a plays before b: by class, then priority, then age.
bool VoxQueueClass::Before(EntryClass const & a, EntryClass const & b) const
{
	int ra = Rank(a.Control);
	int rb = Rank(b.Control);
	if (ra != rb) {
		return(ra < rb);
	}
	if (a.Priority != b.Priority) {
		return(a.Priority > b.Priority);
	}
	return(a.Order < b.Order);
}


void VoxQueueClass::Remove(int index)
{
	for (int i = index; i + 1 < Pending; i++) {
		Entries[i] = Entries[i + 1];
	}
	Pending--;
}


bool VoxQueueClass::Contains(VoxType voice) const
{
	for (int i = 0; i < Pending; i++) {
		if (Entries[i].Voice == voice) {
			return(true);
		}
	}
	return(false);
}


void VoxQueueClass::Clear(void)
{
	Pending = 0;
}


bool VoxQueueClass::Submit(VoxType voice, int priority, VoxControlType control, VoxType playing)
{
	if (voice == VOX_NONE) {
		return(false);
	}
	if (voice == playing || Contains(voice)) {
		return(false);
	}

	bool cut = false;
	if (control == VOXC_INTERRUPT) {
		Clear();
		cut = playing != VOX_NONE;
	}

	if (control == VOXC_STANDARD) {
		for (int i = 0; i < Pending; i++) {
			if (Entries[i].Control == VOXC_STANDARD) {
				if (Entries[i].Priority >= priority) {
					return(false);
				}
				Remove(i);
				break;
			}
		}
	}

	if (Pending >= MAX_PENDING) {
		// The oldest of the lowest-priority lines makes room, critical ones last.
		int victim = -1;
		for (int i = 0; i < Pending; i++) {
			if (victim < 0 || Rank(Entries[i].Control) > Rank(Entries[victim].Control)
				|| (Rank(Entries[i].Control) == Rank(Entries[victim].Control) && (Entries[i].Priority < Entries[victim].Priority
				|| (Entries[i].Priority == Entries[victim].Priority && Entries[i].Order < Entries[victim].Order)))) {
				victim = i;
			}
		}
		Remove(victim);
	}

	EntryClass & entry = Entries[Pending++];
	entry.Voice = voice;
	entry.Priority = priority;
	entry.Control = control;
	entry.Order = ++Serial;
	return(cut);
}


bool VoxQueueClass::Next(VoxType & voice)
{
	if (Pending == 0) {
		return(false);
	}
	int best = 0;
	for (int i = 1; i < Pending; i++) {
		if (Before(Entries[i], Entries[best])) {
			best = i;
		}
	}
	voice = Entries[best].Voice;
	Remove(best);
	return(true);
}
