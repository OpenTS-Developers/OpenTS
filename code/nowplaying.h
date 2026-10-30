/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#pragma once

#include "rect.h"


void Now_Playing_Set_View(Rect const & area);
void Now_Playing_AI(void);
void Now_Playing_Draw(void);
void Now_Playing_Announce(void);
bool Now_Playing_Key(int virtualkey, bool repeat);

#if defined(__APPLE__)
// enabled names this process as the one the system media keys control.
void Now_Playing_System_Keys(bool enabled, char const * title);
#endif
