/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/


#include "always.h"

#ifdef STEVES_NEW_CATCHER

#include <crtdbg.h>
#include <new>


/// <summary>
/// Allocates a block of memory from the debugging heap.
/// This routine replaces the global allocator so that every allocation the game makes is
/// tagged with its origin and tracked by the debugging runtime's leak checker.
/// </summary>
/// <returns>Returns with a pointer to the block allocated, or NULL if the heap is
/// exhausted.</returns>
void * __cdecl operator new(unsigned int size)
{
	if (size == 0) size = 1;
	void * ptr = _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__);
	return(ptr);
}


/// <summary>
/// Returns a block of memory to the debugging heap.
/// This routine is the counterpart to the replacement allocator, handing the block back
/// to the debugging runtime so that its tracking record can be closed out. A NULL pointer
/// is tolerated and simply ignored.
/// </summary>
void __cdecl operator delete(void * ptr)
{
	if (ptr) _free_dbg(ptr, _NORMAL_BLOCK);
}


#endif
