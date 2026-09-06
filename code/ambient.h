/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Looping sounds that follow objects. The table stands beside the objects
// rather than inside them: an object is attached by pointer, its sound is
// re-aimed at its centre every tick, and the attachment travels with a save
// while the playing sound itself does not.

#pragma once

#include "audio/audiohandle.h"
#include "voc.hh"

class ObjectClass;
class SaveStreamClass;


class AmbientSoundTable
{
	public:
		AmbientSoundTable(void) = default;

		// Replaces any sound the object already has; VOC_NONE detaches.
		void Attach(ObjectClass * object, VocType voc);
		void Detach(ObjectClass const * object);
		VocType Attached(ObjectClass const * object) const;

		// Once per game tick. An object in limbo falls silent until it returns.
		void AI(void);

		void Clear(void);
		void Serialize(SaveStreamClass & stream);
		int Count(void) const;

	private:
		enum { MAX_ENTRIES = 256 };

		struct EntryClass {
			ObjectClass * Object = nullptr;
			VocType Voc = VOC_NONE;
			AudioHandle Handle;
			bool Started = false;
		};

		int Find(ObjectClass const * object) const;
		int Find_Free(void) const;

		EntryClass Entries[MAX_ENTRIES];
};

extern AmbientSoundTable AmbientSounds;
