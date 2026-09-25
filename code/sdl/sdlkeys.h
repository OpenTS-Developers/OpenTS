/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <string>


// Takes an SDL scancode, the keycode the current layout gives it, the SDL modifier state, and
// the Windows scan code, or 0 for none. Returns the virtual-key code the keyboard layout
// gives the key, or 0 when the key has none. A null layout is the thread's current one.
int Virtual_Key_From_SDL(int scancode, unsigned int keycode, unsigned int modifiers, unsigned int raw = 0, void * layout = nullptr);

// Returns the layout's character or SDL's English name for a virtual-key code, or "" for none.
std::string Virtual_Key_Name(int virtualkey, void * layout = nullptr);
