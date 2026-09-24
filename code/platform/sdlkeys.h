/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once


// Takes an SDL scancode, the keycode the current layout gives it, and the SDL modifier state.
// Returns the Windows virtual-key code for the key, or 0 when the key has none.
int Virtual_Key_From_SDL(int scancode, unsigned int keycode, unsigned int modifiers);
