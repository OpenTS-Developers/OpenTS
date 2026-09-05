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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/always.h                               $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 8/28/01 3:21p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

// Disable warning about exception handling not being enabled. It's used as part of STL - in a part of STL we don't use.
#pragma warning(disable : 4530)

/*
**	Define for debug memory allocation to include __FILE__ and __LINE__ for every memory allocation.
**	This helps find leaks.
*/
#ifdef _DEBUG
#define STEVES_NEW_CATCHER

#ifdef _MSC_VER
#ifdef STEVES_NEW_CATCHER

#include <crtdbg.h>
#include <cstdlib>
#include <malloc.h>

#define   malloc(s)         _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   calloc(c, s)      _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   realloc(p, s)     _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   _expand(p, s)     _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   free(p)           _free_dbg(p, _NORMAL_BLOCK)
#define   _msize(p)         _msize_dbg(p, _NORMAL_BLOCK)

void * __cdecl operator new(unsigned int size);
void __cdecl operator delete(void * ptr);

#endif	//STEVES_NEW_CATCHER
#endif	//_MSC_VER
#endif	//_DEBUG


// Jani: Intel's C++ compiler issues too many warnings in WW libraries when using warning level 4
#if defined (__ICL)    // Detect Intel compiler
#pragma warning (3)
#pragma warning ( disable: 981 ) // parameters defined in unspecified order
#pragma warning ( disable: 279 ) // controlling expressaion is constant
#pragma warning ( disable: 271 ) // trailing comma is nonstandard
#pragma warning ( disable: 171 ) // invalid type conversion
#pragma warning ( disable: 1 ) // last line of file ends without a newline
#endif

// Jani: MSVC doesn't necessarily inline code with inline keyword. Using __forceinline results better inlining
// and also prints out a warning if inlining wasn't possible. __forceinline is MSVC specific.
#if defined(_MSC_VER)
#define WWINLINE __forceinline
#else
#define WWINLINE inline
#endif


/*
**	This includes the minimum set of compiler defines and pragmas in order to bring the
**	various compilers to a common behavior such that the C&C engine will compile without
**	error or warning.
*/
#include "visualc.h"


#ifndef	NULL
	#define	NULL		0
#endif

/**********************************************************************
**	This macro serves as a general way to determine the number of elements
**	within an array.
*/
#ifndef ARRAY_SIZE
#define	ARRAY_SIZE(x)		int(sizeof(x)/sizeof(x[0]))
#endif

#ifndef size_of
#define size_of(typ,id) sizeof(((typ*)0)->id)
#endif

#ifndef OFFSET_OF
#define OFFSET_OF(typ,m)	((size_t)&(((typ*)0)->m))
#endif

/// The codebase uses both spellings; map them to one so both resolve to a single symbol.
#ifndef strcmpi
#define strcmpi stricmp
#endif
#ifndef _strupr
#define _strupr strupr
#endif
#ifndef _stricmp
#define _stricmp stricmp
#endif
