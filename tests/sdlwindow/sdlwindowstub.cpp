/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Stands in for the game's instance handle and debug log. The executable has no cursor
// resource, so the SDL layer falls back to the system arrow.

#include "dbgprint.h"
#include "win.h"


HINSTANCE ProgramInstance = GetModuleHandleW(NULL);


void __cdecl DebugString(char const *, ...)
{
}
