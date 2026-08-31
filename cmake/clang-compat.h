/*******************************************************************************
 *                                O P E N  T S
 ******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 ******************************************************************************/

#pragma once

#include <bx/bx.h>

#if defined(__clang__) && defined(_M_IX86)
#undef __stdcall
#define __stdcall __attribute__((stdcall))
#endif
