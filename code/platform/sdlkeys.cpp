/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "platform/sdlkeys.h"

#include "win.h"

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_scancode.h>


namespace
{

// Windows keeps the keypad's own codes only while Num Lock is on and Shift is up.
int Keypad_Key(SDL_Scancode scancode, SDL_Keymod modifiers)
{
	bool const numbers = (modifiers & SDL_KMOD_NUM) != 0 && (modifiers & SDL_KMOD_SHIFT) == 0;

	switch (scancode) {
		case SDL_SCANCODE_KP_0:			return(numbers ? VK_NUMPAD0 : VK_INSERT);
		case SDL_SCANCODE_KP_1:			return(numbers ? VK_NUMPAD1 : VK_END);
		case SDL_SCANCODE_KP_2:			return(numbers ? VK_NUMPAD2 : VK_DOWN);
		case SDL_SCANCODE_KP_3:			return(numbers ? VK_NUMPAD3 : VK_NEXT);
		case SDL_SCANCODE_KP_4:			return(numbers ? VK_NUMPAD4 : VK_LEFT);
		case SDL_SCANCODE_KP_5:			return(numbers ? VK_NUMPAD5 : VK_CLEAR);
		case SDL_SCANCODE_KP_6:			return(numbers ? VK_NUMPAD6 : VK_RIGHT);
		case SDL_SCANCODE_KP_7:			return(numbers ? VK_NUMPAD7 : VK_HOME);
		case SDL_SCANCODE_KP_8:			return(numbers ? VK_NUMPAD8 : VK_UP);
		case SDL_SCANCODE_KP_9:			return(numbers ? VK_NUMPAD9 : VK_PRIOR);
		case SDL_SCANCODE_KP_PERIOD:	return(numbers ? VK_DECIMAL : VK_DELETE);
		case SDL_SCANCODE_KP_DIVIDE:	return(VK_DIVIDE);
		case SDL_SCANCODE_KP_MULTIPLY:	return(VK_MULTIPLY);
		case SDL_SCANCODE_KP_MINUS:		return(VK_SUBTRACT);
		case SDL_SCANCODE_KP_PLUS:		return(VK_ADD);
		case SDL_SCANCODE_KP_ENTER:		return(VK_RETURN);
		default:						return(-1);
	}
}


// Windows names letter and digit keys after what the layout prints on them, and the comma,
// period, minus and plus keys after their character, wherever the layout puts them.
int Layout_Key(SDL_Keycode keycode)
{
	if (keycode >= 'a' && keycode <= 'z') {
		return((int)(keycode - 'a') + 'A');
	}
	if (keycode >= '0' && keycode <= '9') {
		return((int)keycode);
	}

	switch (keycode) {
		case ',':	return(VK_OEM_COMMA);
		case '.':	return(VK_OEM_PERIOD);
		case '-':	return(VK_OEM_MINUS);
		case '=':
		case '+':	return(VK_OEM_PLUS);
		default:	return(0);
	}
}


// The remaining keys take the code Windows gives the same position on a US layout.
int Position_Key(SDL_Scancode scancode)
{
	if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z) {
		return((int)(scancode - SDL_SCANCODE_A) + 'A');
	}
	if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9) {
		return((int)(scancode - SDL_SCANCODE_1) + '1');
	}
	if (scancode >= SDL_SCANCODE_F1 && scancode <= SDL_SCANCODE_F12) {
		return((int)(scancode - SDL_SCANCODE_F1) + VK_F1);
	}
	if (scancode >= SDL_SCANCODE_F13 && scancode <= SDL_SCANCODE_F24) {
		return((int)(scancode - SDL_SCANCODE_F13) + VK_F13);
	}

	switch (scancode) {
		case SDL_SCANCODE_0:				return('0');
		case SDL_SCANCODE_RETURN:			return(VK_RETURN);
		case SDL_SCANCODE_ESCAPE:			return(VK_ESCAPE);
		case SDL_SCANCODE_BACKSPACE:		return(VK_BACK);
		case SDL_SCANCODE_TAB:				return(VK_TAB);
		case SDL_SCANCODE_SPACE:			return(VK_SPACE);
		case SDL_SCANCODE_MINUS:			return(VK_OEM_MINUS);
		case SDL_SCANCODE_EQUALS:			return(VK_OEM_PLUS);
		case SDL_SCANCODE_LEFTBRACKET:		return(VK_OEM_4);
		case SDL_SCANCODE_RIGHTBRACKET:		return(VK_OEM_6);
		case SDL_SCANCODE_BACKSLASH:		return(VK_OEM_5);
		case SDL_SCANCODE_NONUSHASH:		return(VK_OEM_5);
		case SDL_SCANCODE_SEMICOLON:		return(VK_OEM_1);
		case SDL_SCANCODE_APOSTROPHE:		return(VK_OEM_7);
		case SDL_SCANCODE_GRAVE:			return(VK_OEM_3);
		case SDL_SCANCODE_COMMA:			return(VK_OEM_COMMA);
		case SDL_SCANCODE_PERIOD:			return(VK_OEM_PERIOD);
		case SDL_SCANCODE_SLASH:			return(VK_OEM_2);
		case SDL_SCANCODE_NONUSBACKSLASH:	return(VK_OEM_102);
		case SDL_SCANCODE_CAPSLOCK:			return(VK_CAPITAL);
		case SDL_SCANCODE_PRINTSCREEN:		return(VK_SNAPSHOT);
		case SDL_SCANCODE_SCROLLLOCK:		return(VK_SCROLL);
		case SDL_SCANCODE_PAUSE:			return(VK_PAUSE);
		case SDL_SCANCODE_INSERT:			return(VK_INSERT);
		case SDL_SCANCODE_HOME:				return(VK_HOME);
		case SDL_SCANCODE_PAGEUP:			return(VK_PRIOR);
		case SDL_SCANCODE_DELETE:			return(VK_DELETE);
		case SDL_SCANCODE_END:				return(VK_END);
		case SDL_SCANCODE_PAGEDOWN:			return(VK_NEXT);
		case SDL_SCANCODE_RIGHT:			return(VK_RIGHT);
		case SDL_SCANCODE_LEFT:				return(VK_LEFT);
		case SDL_SCANCODE_DOWN:				return(VK_DOWN);
		case SDL_SCANCODE_UP:				return(VK_UP);
		case SDL_SCANCODE_NUMLOCKCLEAR:		return(VK_NUMLOCK);
		case SDL_SCANCODE_APPLICATION:		return(VK_APPS);
		case SDL_SCANCODE_HELP:				return(VK_HELP);
		case SDL_SCANCODE_SELECT:			return(VK_SELECT);
		case SDL_SCANCODE_EXECUTE:			return(VK_EXECUTE);
		case SDL_SCANCODE_CLEAR:			return(VK_CLEAR);
		case SDL_SCANCODE_SLEEP:			return(VK_SLEEP);
		case SDL_SCANCODE_MUTE:				return(VK_VOLUME_MUTE);
		case SDL_SCANCODE_VOLUMEUP:			return(VK_VOLUME_UP);
		case SDL_SCANCODE_VOLUMEDOWN:		return(VK_VOLUME_DOWN);
		case SDL_SCANCODE_MEDIA_NEXT_TRACK:	return(VK_MEDIA_NEXT_TRACK);
		case SDL_SCANCODE_MEDIA_PREVIOUS_TRACK:	return(VK_MEDIA_PREV_TRACK);
		case SDL_SCANCODE_MEDIA_STOP:		return(VK_MEDIA_STOP);
		case SDL_SCANCODE_MEDIA_PLAY_PAUSE:	return(VK_MEDIA_PLAY_PAUSE);
		case SDL_SCANCODE_LCTRL:
		case SDL_SCANCODE_RCTRL:			return(VK_CONTROL);
		case SDL_SCANCODE_LSHIFT:
		case SDL_SCANCODE_RSHIFT:			return(VK_SHIFT);
		case SDL_SCANCODE_LALT:
		case SDL_SCANCODE_RALT:				return(VK_MENU);
		case SDL_SCANCODE_LGUI:				return(VK_LWIN);
		case SDL_SCANCODE_RGUI:				return(VK_RWIN);
		default:							return(0);
	}
}

}


int Virtual_Key_From_SDL(int scancode, unsigned int keycode, unsigned int modifiers)
{
	SDL_Scancode const code = (SDL_Scancode)scancode;

	int key = Keypad_Key(code, (SDL_Keymod)modifiers);
	if (key >= 0) {
		return(key);
	}

	// The main block's printable keys follow the layout; nothing else does.
	bool const printable = (code >= SDL_SCANCODE_A && code <= SDL_SCANCODE_0)
		|| (code >= SDL_SCANCODE_MINUS && code <= SDL_SCANCODE_SLASH)
		|| code == SDL_SCANCODE_NONUSBACKSLASH;
	if (printable) {
		key = Layout_Key((SDL_Keycode)keycode);
		if (key != 0) {
			return(key);
		}
	}

	return(Position_Key(code));
}


// Keypad keys are looked for with Num Lock on first, so a navigation key is named after the
// main block's key and a keypad digit after the keypad's.
static int Scancode_Of(int virtualkey)
{
	static SDL_Keymod const tries[] = { SDL_KMOD_NUM, SDL_KMOD_NONE };

	for (SDL_Keymod modifiers : tries) {
		for (int scancode = SDL_SCANCODE_UNKNOWN + 1; scancode < SDL_SCANCODE_COUNT; scancode++) {
			SDL_Keycode keycode = SDL_GetKeyFromScancode((SDL_Scancode)scancode, SDL_KMOD_NONE, true);
			if (Virtual_Key_From_SDL(scancode, (unsigned int)keycode, (unsigned int)modifiers) == virtualkey) {
				return(scancode);
			}
		}
	}
	return(SDL_SCANCODE_UNKNOWN);
}


std::string Virtual_Key_Name(int virtualkey)
{
	if (virtualkey <= 0) {
		return(std::string());
	}

	switch (virtualkey) {
		case VK_SHIFT:		return("Shift");
		case VK_CONTROL:	return("Ctrl");
		case VK_MENU:		return("Alt");
		default:			break;
	}

	int const scancode = Scancode_Of(virtualkey);
	if (scancode == SDL_SCANCODE_UNKNOWN) {
		return(std::string());
	}

	char const * name = SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)scancode, SDL_KMOD_NONE, false));
	return((name != nullptr) ? name : "");
}
