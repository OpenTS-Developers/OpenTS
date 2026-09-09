/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "win.h"

#include <stdint.h>
#include <vector>


// A saved identity is four bytes, and is numbered rather than taken from the address the
// object sat at, so a save does not depend on the pointer width of the build that wrote it.
using SwizzleIDType = uint32_t;


class SwizzlePointerClass
{
	public:
		SwizzlePointerClass(SwizzleIDType id = 0, void * pointer = nullptr) : ID(id), Pointer(pointer) {}

	public:
		SwizzleIDType ID;

		/*
		 * This is where the object was loaded to.
		 */
		void * Pointer;
};


class SwizzleRequestClass
{
	public:
		SwizzleRequestClass(SwizzleIDType id = 0, void * pointer = nullptr, char const * ownertype = nullptr, SwizzleIDType ownerid = 0, char const * slottype = nullptr, char const * file = nullptr, unsigned int line = 0) :
			ID(id), Pointer(pointer), OwnerType(ownertype), OwnerID(ownerid), SlotType(slottype), File(file), Line(line) {}

	public:
		/*
		 * This is the swizzle ID this request asks after, and the pointer that needs
		 * filling in once the object that ID names has announced where it landed.
		 */
		SwizzleIDType ID;
		void * Pointer;

		/*
		 * These describe where the request came from, for the report when nothing ever
		 * answers it: the record being read at the time, that record's own swizzle ID,
		 * the type of the pointer slot, and the line that serialized it. The strings are
		 * the static ones type identification and the source location hand out, so
		 * carrying them costs nothing.
		 */
		char const * OwnerType;
		SwizzleIDType OwnerID;
		char const * SlotType;
		char const * File;
		unsigned int Line;
};


class SwizzleManagerClass
{
	public:
		SwizzleManagerClass(void);

		void Begin_Save(void);
		SwizzleIDType ID_Of(void const * pointer);

		void Swizzle(void ** pointer, SwizzleIDType id, char const * ownertype = nullptr, SwizzleIDType ownerid = 0, char const * slottype = nullptr, char const * file = nullptr, unsigned int line = 0);
		void Here_I_Am(SwizzleIDType id, void * pointer);

		void Resolve(void);
		void Discard(void);

	private:
		/*
		 * These are the pointers read back from the save file that still hold a swizzle ID
		 * instead of a real address. They stay in the order the file presented them, so a
		 * report of unanswered requests follows the shape of the save game.
		 */
		std::vector<SwizzleRequestClass> RequestTable;

		/*
		 * These are the addresses the loaded objects have announced themselves at. The
		 * table is sorted when the requests are resolved against it.
		 */
		std::vector<SwizzlePointerClass> PointerTable;
};


extern SwizzleManagerClass Swizzler;

template<class T>
inline void Swizzle_Here_I_Am(SwizzleIDType id, T * ptr)
{
	Swizzler.Here_I_Am(id, (void *)ptr);
}
