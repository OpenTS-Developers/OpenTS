/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <RmlUi/Core/Input.h>


// The modifier bits of a KEYBOARD.INI key number, above the Windows virtual key.
enum {
	UI_KEY_SHIFT = 0x100,
	UI_KEY_CTRL = 0x200,
	UI_KEY_ALT = 0x400
};


// Windows virtual keys and RmlUi key identifiers, either way. An unknown key answers
// KI_UNKNOWN or 0.
Rml::Input::KeyIdentifier UI_Key_Identifier(int virtualkey);
int UI_Virtual_Key(Rml::Input::KeyIdentifier key);

// The KEYBOARD.INI number of a key pressed in a document, or 0 for a press the game cannot
// bind: a modifier on its own, or a key with no virtual key.
int UI_Key_Number(Rml::Input::KeyIdentifier key, bool shift, bool ctrl, bool alt);
