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


/*
** Define some Windows specific values that are used throghout the games
*/
#ifndef _WIN32

#define _MAX_DRIVE 3
#define _MAX_DIR   256
#define _MAX_FNAME 256
#define _MAX_EXT   256
#define _MAX_PATH  512
#define MAX_PATH   _MAX_PATH
#define _CONTROL   0x20  // space, first non-control character in ASCII

#undef _stricmp
#define stricmp  strcasecmp
#define _stricmp strcasecmp
#define strnicmp strncasecmp
#define memicmp  strncasecmp
#define __cdecl

#include <cctype>
#include <cstdio>
#include <cstring>

inline void _makepath(char * path, char const * drive, char const * dir, char const * fname, char const * ext)
{
	if (path == nullptr) return;

	size_t length = 0;

	auto append = [&](char const * text, size_t count) {
		size_t room = _MAX_PATH - 1 - length;
		if (count > room) count = room;
		memcpy(path + length, text, count);
		length += count;
	};

	if (drive != nullptr && drive[0] != '\0') {
		append(drive, 1);
		append(":", 1);
	}

	if (dir != nullptr && dir[0] != '\0') {
		size_t count = strlen(dir);
		append(dir, count);
		if (dir[count - 1] != '\\' && dir[count - 1] != '/') {
			append("/", 1);
		}
	}

	if (fname != nullptr) {
		append(fname, strlen(fname));
	}

	if (ext != nullptr && ext[0] != '\0') {
		if (ext[0] != '.') {
			append(".", 1);
		}
		append(ext, strlen(ext));
	}

	path[length] = '\0';
}

inline void _splitpath(char const * path, char * drive, char * dir, char * fname, char * ext)
{
	if (drive != nullptr) drive[0] = '\0';
	if (dir != nullptr) dir[0] = '\0';
	if (fname != nullptr) fname[0] = '\0';
	if (ext != nullptr) ext[0] = '\0';

	if (path == nullptr) return;

	char const * rest = path;

	if (path[0] != '\0' && path[1] == ':') {
		if (drive != nullptr) snprintf(drive, _MAX_DRIVE, "%.2s", path);
		rest = path + 2;
	}

	char const * name = rest;
	for (char const * scan = rest; *scan != '\0'; ++scan) {
		if (*scan == '\\' || *scan == '/') {
			name = scan + 1;
		}
	}

	if (dir != nullptr) snprintf(dir, _MAX_DIR, "%.*s", (int)(name - rest), rest);

	char const * dot = name + strlen(name);
	for (char const * scan = name; *scan != '\0'; ++scan) {
		if (*scan == '.') {
			dot = scan;
		}
	}

	if (fname != nullptr) snprintf(fname, _MAX_FNAME, "%.*s", (int)(dot - name), name);
	if (ext != nullptr) snprintf(ext, _MAX_EXT, "%s", dot);
}

inline static char* strupr(char* str)
{
	char* ret = str;
	while (*str != '\0') {
		*str = toupper(*str);
		++str;
	}
	return(ret);
}

inline static void strrev(char* str)
{
	int len = strlen(str);

	for (int i = 0; i < len / 2; i++) {
		char c = str[i];
		str[i] = str[len - i - 1];
		str[len - i - 1] = c;
	}
}

inline static void _strlwr(char* str)
{
	while (*str != '\0') {
		*str = tolower(*str);
		++str;
	}
}

#endif // not _WIN32
