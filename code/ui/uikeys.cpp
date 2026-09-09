/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uikeys.h"

#include <windows.h>


namespace
{

struct UIKeyMapping
{
	int VirtualKey;
	Rml::Input::KeyIdentifier Key;
};

const UIKeyMapping _KeyMappings[] = {
	{ VK_BACK, Rml::Input::KI_BACK },
	{ VK_TAB, Rml::Input::KI_TAB },
	{ VK_CLEAR, Rml::Input::KI_CLEAR },
	{ VK_RETURN, Rml::Input::KI_RETURN },
	{ VK_PAUSE, Rml::Input::KI_PAUSE },
	{ VK_CAPITAL, Rml::Input::KI_CAPITAL },
	{ VK_ESCAPE, Rml::Input::KI_ESCAPE },
	{ VK_SPACE, Rml::Input::KI_SPACE },
	{ VK_PRIOR, Rml::Input::KI_PRIOR },
	{ VK_NEXT, Rml::Input::KI_NEXT },
	{ VK_END, Rml::Input::KI_END },
	{ VK_HOME, Rml::Input::KI_HOME },
	{ VK_LEFT, Rml::Input::KI_LEFT },
	{ VK_UP, Rml::Input::KI_UP },
	{ VK_RIGHT, Rml::Input::KI_RIGHT },
	{ VK_DOWN, Rml::Input::KI_DOWN },
	{ VK_SNAPSHOT, Rml::Input::KI_SNAPSHOT },
	{ VK_INSERT, Rml::Input::KI_INSERT },
	{ VK_DELETE, Rml::Input::KI_DELETE },
	{ VK_LWIN, Rml::Input::KI_LWIN },
	{ VK_RWIN, Rml::Input::KI_RWIN },
	{ VK_APPS, Rml::Input::KI_APPS },
	{ VK_MULTIPLY, Rml::Input::KI_MULTIPLY },
	{ VK_ADD, Rml::Input::KI_ADD },
	{ VK_SEPARATOR, Rml::Input::KI_SEPARATOR },
	{ VK_SUBTRACT, Rml::Input::KI_SUBTRACT },
	{ VK_DECIMAL, Rml::Input::KI_DECIMAL },
	{ VK_DIVIDE, Rml::Input::KI_DIVIDE },
	{ VK_NUMLOCK, Rml::Input::KI_NUMLOCK },
	{ VK_SCROLL, Rml::Input::KI_SCROLL },
	{ VK_SHIFT, Rml::Input::KI_LSHIFT },
	{ VK_CONTROL, Rml::Input::KI_LCONTROL },
	{ VK_MENU, Rml::Input::KI_LMENU },
	{ VK_LSHIFT, Rml::Input::KI_LSHIFT },
	{ VK_RSHIFT, Rml::Input::KI_RSHIFT },
	{ VK_LCONTROL, Rml::Input::KI_LCONTROL },
	{ VK_RCONTROL, Rml::Input::KI_RCONTROL },
	{ VK_LMENU, Rml::Input::KI_LMENU },
	{ VK_RMENU, Rml::Input::KI_RMENU },
	{ VK_OEM_1, Rml::Input::KI_OEM_1 },
	{ VK_OEM_PLUS, Rml::Input::KI_OEM_PLUS },
	{ VK_OEM_COMMA, Rml::Input::KI_OEM_COMMA },
	{ VK_OEM_MINUS, Rml::Input::KI_OEM_MINUS },
	{ VK_OEM_PERIOD, Rml::Input::KI_OEM_PERIOD },
	{ VK_OEM_2, Rml::Input::KI_OEM_2 },
	{ VK_OEM_3, Rml::Input::KI_OEM_3 },
	{ VK_OEM_4, Rml::Input::KI_OEM_4 },
	{ VK_OEM_5, Rml::Input::KI_OEM_5 },
	{ VK_OEM_6, Rml::Input::KI_OEM_6 },
	{ VK_OEM_7, Rml::Input::KI_OEM_7 },
	{ VK_OEM_8, Rml::Input::KI_OEM_8 },
	{ VK_OEM_102, Rml::Input::KI_OEM_102 },
};

Rml::Input::KeyIdentifier _Identifiers[256];
int _VirtualKeys[256];
bool _Built = false;


// Letters, digits, the keypad digits and the function keys are contiguous in both codings.
// Where two virtual keys share an identifier, the one Windows reports for an unqualified
// press comes first and is the one the identifier maps back to.
void Build(void)
{
	for (int code = 0; code < 256; code++) {
		_Identifiers[code] = Rml::Input::KI_UNKNOWN;
		_VirtualKeys[code] = 0;
	}

	for (UIKeyMapping const & mapping : _KeyMappings) {
		_Identifiers[mapping.VirtualKey] = mapping.Key;
	}

	for (int letter = 0; letter < 26; letter++) {
		_Identifiers['A' + letter] = (Rml::Input::KeyIdentifier)(Rml::Input::KI_A + letter);
	}
	for (int digit = 0; digit < 10; digit++) {
		_Identifiers['0' + digit] = (Rml::Input::KeyIdentifier)(Rml::Input::KI_0 + digit);
		_Identifiers[VK_NUMPAD0 + digit] = (Rml::Input::KeyIdentifier)(Rml::Input::KI_NUMPAD0 + digit);
	}
	for (int function = 0; function < 12; function++) {
		_Identifiers[VK_F1 + function] = (Rml::Input::KeyIdentifier)(Rml::Input::KI_F1 + function);
	}

	for (int code = 0; code < 256; code++) {
		int key = (int)_Identifiers[code];
		if (key != Rml::Input::KI_UNKNOWN && key < 256 && _VirtualKeys[key] == 0) {
			_VirtualKeys[key] = code;
		}
	}

	_Built = true;
}

}


Rml::Input::KeyIdentifier UI_Key_Identifier(int virtualkey)
{
	if (!_Built) {
		Build();
	}
	if (virtualkey < 0 || virtualkey > 255) {
		return(Rml::Input::KI_UNKNOWN);
	}
	return(_Identifiers[virtualkey]);
}


int UI_Virtual_Key(Rml::Input::KeyIdentifier key)
{
	if (!_Built) {
		Build();
	}
	if ((int)key <= 0 || (int)key >= 256) {
		return(0);
	}
	return(_VirtualKeys[(int)key]);
}


int UI_Key_Number(Rml::Input::KeyIdentifier key, bool shift, bool ctrl, bool alt)
{
	switch (key) {
		case Rml::Input::KI_LSHIFT:
		case Rml::Input::KI_RSHIFT:
		case Rml::Input::KI_LCONTROL:
		case Rml::Input::KI_RCONTROL:
		case Rml::Input::KI_LMENU:
		case Rml::Input::KI_RMENU:
			return(0);

		default:
			break;
	}

	int virtualkey = UI_Virtual_Key(key);
	if (virtualkey == 0) {
		return(0);
	}
	return(virtualkey | (shift ? UI_KEY_SHIFT : 0) | (ctrl ? UI_KEY_CTRL : 0) | (alt ? UI_KEY_ALT : 0));
}
