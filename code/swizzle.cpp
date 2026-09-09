/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "swizzle.h"

#include "conquer.h"
#include "dbgprint.h"
#include "msgbox.h"

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>


SwizzleManagerClass Swizzler;


/// <summary>
/// Reduces a source path to the file name alone.
/// The source location a request carries is the path the compiler was given, which is
/// long enough to bury the rest of the report.
/// </summary>
/// <param name="path">The path to trim, which may be NULL.</param>
/// <returns>Returns with the file name, or a placeholder if there is no path.</returns>
static char const * Base_Name(char const * path)
{
	if (path == NULL) {
		return("<unknown>");
	}

	char const * name = path;
	for (char const * ptr = path; *ptr != '\0'; ptr++) {
		if (*ptr == '\\' || *ptr == '/') {
			name = ptr + 1;
		}
	}
	return(name);
}


namespace {

std::unordered_map<void const *, SwizzleIDType> IDTable;
SwizzleIDType NextID = 1;

}	// namespace


/// <summary>
/// Constructor for the pointer swizzle manager.
/// The request and pointer tables are given generous room up front, since a save game
/// hands the swizzler thousands of pointers in a single pass.
/// </summary>
SwizzleManagerClass::SwizzleManagerClass(void)
{
	RequestTable.reserve(1000);
	PointerTable.reserve(1000);
}


/// <summary>
/// Starts a fresh numbering of saved identities.
/// </summary>
/// <remarks>An identity only has to be unique within one saved game, so the save code
/// calls this before it writes the first record.</remarks>
void SwizzleManagerClass::Begin_Save(void)
{
	IDTable.clear();
	NextID = 1;
}


/// <summary>
/// Fetches the identity a pointer is saved under.
/// Pointers are numbered in the order the save first meets them, so what a save holds
/// does not depend on where the objects happened to sit in memory, or on how wide a
/// pointer is on the host that wrote it.
/// </summary>
/// <param name="pointer">The pointer being written, which may be null.</param>
/// <returns>Returns with the identity to write in the pointer's place, or zero for a
/// null pointer.</returns>
SwizzleIDType SwizzleManagerClass::ID_Of(void const * pointer)
{
	if (pointer == nullptr) {
		return(0);
	}

	auto found = IDTable.find(pointer);
	if (found != IDTable.end()) {
		return(found->second);
	}

	SwizzleIDType const id = NextID++;
	IDTable.emplace(pointer, id);
	return(id);
}


/// <summary>
/// Registers a saved pointer to be resolved later.
/// The load code calls this routine for every pointer it reads back, since the value on
/// disk is a swizzle ID rather than an address. The request is remembered and the pointer
/// is cleared until the tables are resolved and the real address is known.
/// </summary>
/// <param name="pointer">Pointer to the pointer that needs resolving.</param>
/// <param name="id">The identity read from the save in the pointer's place.</param>
/// <param name="ownertype">The type of the record being read, for the failure report.</param>
/// <param name="ownerid">The swizzle ID of the record being read.</param>
/// <param name="slottype">The type the pointer slot names.</param>
void SwizzleManagerClass::Swizzle(void ** pointer, SwizzleIDType id, char const * ownertype, SwizzleIDType ownerid, char const * slottype, char const * file, unsigned int line)
{
	if (pointer == NULL) {
		return;
	}

	if (id == 0) {
		*pointer = nullptr;
		return;
	}

	RequestTable.emplace_back(id, pointer, ownertype, ownerid, slottype, file, line);

	*pointer = NULL;
}


/// <summary>
/// Announces the real address of a swizzle ID.
/// Objects call this routine as they are loaded, to say where they ended up. The requests
/// gathered by Swizzle are matched against these announcements when the tables are
/// resolved.
/// </summary>
/// <param name="id">The swizzle ID the object was saved under.</param>
/// <param name="pointer">The address the object now resides at.</param>
void SwizzleManagerClass::Here_I_Am(SwizzleIDType id, void * pointer)
{
	PointerTable.emplace_back(id, pointer);
}


/// <summary>
/// Fills in every outstanding pointer request.
/// The requests registered by Swizzle are matched
/// against the addresses announced by Here_I_Am, and each real address is written into the
/// pointer that asked for it. Both tables are emptied once the matching is done.
/// </summary>
/// <remarks>Every ID handed to Swizzle must have been announced by Here_I_Am. An unmatched
/// request is fatal: by this point the game state it belonged to has already been replaced,
/// so the orphans are reported and the game exits rather than run on dangling pointers.
/// </remarks>
void SwizzleManagerClass::Resolve(void)
{
	if (RequestTable.empty()) {
		Discard();
		return;
	}

	std::sort(PointerTable.begin(), PointerTable.end(),
		[](SwizzlePointerClass const & left, SwizzlePointerClass const & right) { return(left.ID < right.ID); });

	/*
	 * An ID announced twice means two objects claim one identity, or one record was read
	 * twice. The requests for it are about to be answered with whichever arrival sorted
	 * first, so say so.
	 */
	for (unsigned int index = 1; index < PointerTable.size(); index++) {
		if (PointerTable[index].ID == PointerTable[index - 1].ID) {
			DebugString("SWIZZLE: ID %08X was announced twice, at %p and at %p.\n",
				PointerTable[index].ID, PointerTable[index - 1].Pointer, PointerTable[index].Pointer);
		}
	}

	int orphans = 0;
	SwizzleRequestClass const * first_orphan = NULL;

	for (SwizzleRequestClass const & request : RequestTable) {

		auto entry = std::lower_bound(PointerTable.begin(), PointerTable.end(), request.ID,
			[](SwizzlePointerClass const & left, SwizzleIDType id) { return(left.ID < id); });

		if (entry != PointerTable.end() && entry->ID == request.ID) {
			*(void **)request.Pointer = entry->Pointer;
			continue;
		}

		DebugString("SWIZZLE: Nothing announced ID %08X, wanted by a %s slot in %s record %08X, serialized at %s(%u).\n",
			request.ID,
			request.SlotType != NULL ? request.SlotType : "<unknown>",
			request.OwnerType != NULL ? request.OwnerType : "<unknown>",
			request.OwnerID,
			Base_Name(request.File),
			request.Line);

		if (orphans == 0) {
			first_orphan = &request;
		}
		orphans++;
	}

	if (orphans > 0) {
		char txt[512];
		sprintf(txt, "Save game load failed!  %d pointer(s) could not be remapped.\n\n"
			"The first names ID %08X, wanted by a %s slot in %s record %08X,\n"
			"serialized at %s(%u).\n\n"
			"The game will now exit.",
			orphans,
			first_orphan->ID,
			first_orphan->SlotType != NULL ? first_orphan->SlotType : "<unknown>",
			first_orphan->OwnerType != NULL ? first_orphan->OwnerType : "<unknown>",
			first_orphan->OwnerID,
			Base_Name(first_orphan->File),
			first_orphan->Line);
		WWMessageBox()._Process(txt, 0);

		Emergency_Exit();
		exit(EXIT_FAILURE);
	}

	Discard();
}


/// <summary>
/// Takes back everything registered since the mark.
/// The slots those requests name were left null by Swizzle and nothing has filled them,
/// since Resolve does not run until the load has succeeded, so dropping the requests is
/// all it takes to let the objects holding them be destroyed.
/// </summary>
void SwizzleManagerClass::Abandon(MarkType const & mark)
{
	RequestTable.resize(mark.Requests);
	PointerTable.resize(mark.Pointers);
}


/// <summary>
/// Throws away every pending request and announcement.
/// The load code calls this routine before it starts reading, so that whatever a load that
/// gave up partway through left behind cannot be resolved into the game that follows it.
/// </summary>
void SwizzleManagerClass::Discard(void)
{
	RequestTable.clear();
	PointerTable.clear();
}
