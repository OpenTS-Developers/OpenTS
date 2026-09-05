/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The order EVA lines wait in. Each request carries a class that says where it
// goes: critical lines first, then lines asked for right away, then the queue
// by priority and age, then the one standard slot. The queue holds nothing
// about playback, so it is tested on its own.

#pragma once

#include "vox.hh"


enum VoxControlType {
	VOXC_QUEUE,               // waits its turn by priority, then by age
	VOXC_CRITICAL,            // goes ahead of everything waiting
	VOXC_QUEUED_INTERRUPT,    // goes ahead of the queue but never cuts a playing line
	VOXC_STANDARD,            // one slot, replaced only by a higher priority
	VOXC_INTERRUPT            // empties the queue and cuts the playing line
};


class VoxQueueClass
{
	public:
		enum { MAX_PENDING = 8 };

		VoxQueueClass(void);

		// Returns true when the playing line must be cut for this one. A line
		// already playing or waiting is not added again. When the queue is full
		// the oldest of the lowest-priority lines is dropped.
		bool Submit(VoxType voice, int priority, VoxControlType control, VoxType playing);

		// Takes the next line to play. False when nothing waits.
		bool Next(VoxType & voice);

		bool Contains(VoxType voice) const;
		int Count(void) const { return(Pending); }
		void Clear(void);

	private:
		struct EntryClass {
			VoxType Voice;
			int Priority;
			VoxControlType Control;
			unsigned Order;
		};

		static int Rank(VoxControlType control);
		bool Before(EntryClass const & a, EntryClass const & b) const;
		void Remove(int index);

		EntryClass Entries[MAX_PENDING];
		int Pending;
		unsigned Serial;
};
