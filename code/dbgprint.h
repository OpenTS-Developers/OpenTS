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

#pragma once

#include <filesystem>

#if defined(_WIN32)

#include <sal.h>

// Initialization function with support for UTF-16 command-line arguments,
// for use with wmain().
void Debug_Init(int argc, wchar_t const * argv[]);

// WinMain() doesn't provide command-line arguments in the typical format,
// but they can be retrieved with a function call. This is a convenience wrapper
// for Debug_Init() used in WinMain().
void Debug_Init(void);

// Dedicated reporting function for GetLastError() on Windows.
char const * Last_Error_Text(unsigned long error);

#else

// The annotation and the calling convention are the compiler's own. Off Windows
// they carry no meaning, so they cost nothing to spell.
#define _Printf_format_string_
#define __cdecl

#endif

#if defined(__GNUC__)
// printf() formatting attribute supported by clang and gcc
#define DEBUG_PRINTF_ATTR __attribute__((format(printf, 1, 2)))
#else
#define DEBUG_PRINTF_ATTR
#endif

void Debug_Init(int argc, char const * argv[]);
bool Debug_Enable_Console(void);
void Debug_Console_Hold(void);
std::filesystem::path Debug_Log_File_Path(void);
std::filesystem::path Debug_Directory(void);
bool Delete_Files_Older_Than(const std::filesystem::path & directory, const std::string_view & pattern, unsigned int days);

void __cdecl DebugString(_Printf_format_string_ char const * string, ...) DEBUG_PRINTF_ATTR;
void __cdecl DebugStringNoPrefix(_Printf_format_string_ char const * string, ...) DEBUG_PRINTF_ATTR;
