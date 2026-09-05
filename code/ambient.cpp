/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "ambient.h"

#include "object.h"
#include "savestream.h"
#include "voc.h"


AmbientSoundTable::AmbientSoundTable(void)
{
	Clear();
}


int AmbientSoundTable::Find(ObjectClass const * object) const
{
	if (object == nullptr) {
		return(-1);
	}
	for (int i = 0; i < MAX_ENTRIES; i++) {
		if (Entries[i].Object == object) {
			return(i);
		}
	}
	return(-1);
}


void AmbientSoundTable::Attach(ObjectClass * object, VocType voc)
{
	if (object == nullptr) {
		return;
	}
	if (voc == VOC_NONE) {
		Detach(object);
		return;
	}
	int slot = Find(object);
	if (slot < 0) {
		slot = Find(nullptr);
		if (slot < 0) {
			return;
		}
		Entries[slot].Object = object;
	} else if (Entries[slot].Voc != voc && Entries[slot].Handle.Is_Valid()) {
		Entries[slot].Handle.Stop();
	}
	Entries[slot].Voc = voc;
	Entries[slot].Handle.Clear();
}


void AmbientSoundTable::Detach(ObjectClass const * object)
{
	int slot = Find(object);
	if (slot < 0) {
		return;
	}
	if (Entries[slot].Handle.Is_Valid()) {
		Entries[slot].Handle.Stop();
	}
	Entries[slot].Object = nullptr;
	Entries[slot].Voc = VOC_NONE;
	Entries[slot].Handle.Clear();
}


VocType AmbientSoundTable::Attached(ObjectClass const * object) const
{
	int slot = Find(object);
	return(slot < 0 ? VOC_NONE : Entries[slot].Voc);
}


void AmbientSoundTable::AI(void)
{
	for (int i = 0; i < MAX_ENTRIES; i++) {
		EntryClass & entry = Entries[i];
		if (entry.Object == nullptr) {
			continue;
		}
		if (entry.Object->IsInLimbo) {
			if (entry.Handle.Is_Valid()) {
				entry.Handle.Stop();
				entry.Handle.Clear();
			}
			continue;
		}
		Play_If_In_Range(entry.Voc, entry.Object->Center_Coord(), &entry.Handle);
	}
}


void AmbientSoundTable::Clear(void)
{
	for (int i = 0; i < MAX_ENTRIES; i++) {
		if (Entries[i].Handle.Is_Valid()) {
			Entries[i].Handle.Stop();
		}
		Entries[i].Object = nullptr;
		Entries[i].Voc = VOC_NONE;
		Entries[i].Handle.Clear();
	}
}


int AmbientSoundTable::Count(void) const
{
	int count = 0;
	for (int i = 0; i < MAX_ENTRIES; i++) {
		if (Entries[i].Object != nullptr) {
			count++;
		}
	}
	return(count);
}


// Entries are packed to the front on the way out, so a load fills the first
// count slots; the pointers stay in the slots the swizzler patches.
void AmbientSoundTable::Serialize(SaveStreamClass & stream)
{
	int count = Count();
	if (stream.Is_Loading()) {
		Clear();
	}
	stream.Serialize(count);
	if (count < 0 || count > MAX_ENTRIES) {
		stream.Fail();
		return;
	}

	if (stream.Is_Saving()) {
		for (int i = 0; i < MAX_ENTRIES; i++) {
			if (Entries[i].Object != nullptr) {
				int voc = Entries[i].Voc;
				stream.Serialize(Entries[i].Object);
				stream.Serialize(voc);
			}
		}
		return;
	}

	for (int i = 0; i < count; i++) {
		int voc = VOC_NONE;
		stream.Serialize(Entries[i].Object);
		stream.Serialize(voc);
		Entries[i].Voc = (VocType)voc;
		Entries[i].Handle.Clear();
	}
}
