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

#include <SDL3/SDL_keyboard.h>

#include <string>


// Returns the virtual-key code the layout gives the key, or 0 for none; raw is the Windows scan
// code or 0, and a null layout is the thread's current one.
int Virtual_Key_From_SDL(SDL_Scancode scancode, SDL_Keycode keycode, SDL_Keymod modifiers, Uint16 raw = 0, HKL layout = NULL);

// Returns the layout's character or SDL's English name for a virtual-key code, or "" for none.
std::string Virtual_Key_Name(int virtualkey, HKL layout = NULL);
