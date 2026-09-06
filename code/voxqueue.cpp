/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "voxqueue.h"

#include <algorithm>
#include <tuple>


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
	std::move(Entries + index + 1, Entries + Pending, Entries + index);
	Pending--;
}


bool VoxQueueClass::Contains(VoxType voice) const
{
	return(std::any_of(Entries, Entries + Pending, [voice](EntryClass const & entry) { return(entry.Voice == voice); }));
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
		EntryClass const * standard = std::find_if(Entries, Entries + Pending, [](EntryClass const & entry) { return(entry.Control == VOXC_STANDARD); });
		if (standard != Entries + Pending) {
			if (standard->Priority >= priority) {
				return(false);
			}
			Remove((int)(standard - Entries));
		}
	}

	if (Pending >= MAX_PENDING) {
		// The oldest of the lowest-priority lines makes room, critical ones last.
		EntryClass const * victim = std::min_element(Entries, Entries + Pending, [](EntryClass const & a, EntryClass const & b) {
			return(std::make_tuple(-Rank(a.Control), a.Priority, a.Order) < std::make_tuple(-Rank(b.Control), b.Priority, b.Order));
		});
		Remove((int)(victim - Entries));
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
	EntryClass const * best = std::min_element(Entries, Entries + Pending, [this](EntryClass const & a, EntryClass const & b) { return(Before(a, b)); });
	voice = best->Voice;
	Remove((int)(best - Entries));
	return(true);
}
