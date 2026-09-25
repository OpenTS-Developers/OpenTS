/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins the virtual-key code each SDL key becomes, since saved hotkeys store those codes, and
// the names the keyboard screen shows for them.

#include "sdl/sdlkeys.h"

#include "win.h"

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_scancode.h>

#include <cstdio>

namespace {

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-76s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


int Key(SDL_Scancode scancode, SDL_Keycode keycode, SDL_Keymod modifiers = SDL_KMOD_NONE)
{
	return(Virtual_Key_From_SDL((int)scancode, (unsigned int)keycode, (unsigned int)modifiers));
}


// A key whose layout gives it no character, as SDL reports for keys like the arrows.
int Unprinted(SDL_Scancode scancode, SDL_Keymod modifiers = SDL_KMOD_NONE)
{
	return(Key(scancode, SDLK_UNKNOWN, modifiers));
}


int Scanned(SDL_Scancode scancode, unsigned int raw, HKL layout, SDL_Keymod modifiers = SDL_KMOD_NONE)
{
	return(Virtual_Key_From_SDL((int)scancode, (unsigned int)SDLK_UNKNOWN, (unsigned int)modifiers, raw, layout));
}


// The German layout, which the checks load only when the session lacks it and unload after.
struct GermanLayout
{
	HKL Layout = NULL;
	bool Loaded = false;

	GermanLayout(void)
	{
		HKL layouts[64];
		int const count = GetKeyboardLayoutList(64, layouts);
		for (int index = 0; index < count; index++) {
			if (LOWORD((UINT_PTR)layouts[index]) == 0x0407) {
				Layout = layouts[index];
				return;
			}
		}
		Layout = LoadKeyboardLayoutW(L"00000407", KLF_NOTELLSHELL);
		Loaded = (Layout != NULL);
	}

	~GermanLayout(void)
	{
		if (Loaded) {
			UnloadKeyboardLayout(Layout);
		}
	}
};

}


int main(void)
{
	Check(Key(SDL_SCANCODE_A, SDLK_A) == 'A', "a letter key is its capital letter");
	Check(Key(SDL_SCANCODE_Q, SDLK_A) == 'A', "the key an AZERTY layout prints A on is A");
	Check(Key(SDL_SCANCODE_Y, SDLK_Z) == 'Z', "the key a QWERTZ layout prints Z on is Z");
	Check(Key(SDL_SCANCODE_SEMICOLON, SDLK_M) == 'M', "a letter off the US letter keys is still that letter");
	Check(Key(SDL_SCANCODE_1, SDLK_1) == '1' && Key(SDL_SCANCODE_0, SDLK_0) == '0', "a digit key is its digit");
	Check(Key(SDL_SCANCODE_1, SDLK_AMPERSAND) == '1', "a digit key printing a symbol is still its digit");
	Check(Key(SDL_SCANCODE_M, SDLK_COMMA) == VK_OEM_COMMA, "the key a layout prints a comma on is the comma key");
	Check(Key(SDL_SCANCODE_RIGHTBRACKET, SDLK_PLUS) == VK_OEM_PLUS, "the key a layout prints a plus on is the plus key");
	Check(Key(SDL_SCANCODE_EQUALS, SDLK_EQUALS) == VK_OEM_PLUS, "the US equals key is the plus key");
	Check(Key(SDL_SCANCODE_MINUS, (SDL_Keycode)0xDF) == VK_OEM_MINUS, "without a scan code, a key printing a letter with no code keeps its US position");
	Check(Key(SDL_SCANCODE_GRAVE, SDLK_GRAVE) == VK_OEM_3 && Key(SDL_SCANCODE_SLASH, SDLK_SLASH) == VK_OEM_2, "the other punctuation keys keep their US positions");
	Check(Key(SDL_SCANCODE_NONUSBACKSLASH, SDLK_LESS) == VK_OEM_102, "the extra key beside left Shift is its own key");

	Check(Unprinted(SDL_SCANCODE_KP_7, SDL_KMOD_NUM) == VK_NUMPAD7, "a keypad digit with Num Lock on is the keypad digit");
	Check(Unprinted(SDL_SCANCODE_KP_7) == VK_HOME, "the same key with Num Lock off is Home");
	Check(Unprinted(SDL_SCANCODE_KP_7, (SDL_Keymod)(SDL_KMOD_NUM | SDL_KMOD_LSHIFT)) == VK_HOME, "and with Shift held is Home too");
	Check(Unprinted(SDL_SCANCODE_KP_5) == VK_CLEAR && Unprinted(SDL_SCANCODE_KP_0) == VK_INSERT, "the keypad centre and zero are Clear and Insert without Num Lock");
	Check(Unprinted(SDL_SCANCODE_KP_PERIOD, SDL_KMOD_NUM) == VK_DECIMAL && Unprinted(SDL_SCANCODE_KP_PERIOD) == VK_DELETE, "the keypad point is the decimal key or Delete");
	Check(Unprinted(SDL_SCANCODE_KP_ENTER) == VK_RETURN, "the keypad Enter is Enter");
	Check(Unprinted(SDL_SCANCODE_KP_PLUS) == VK_ADD && Unprinted(SDL_SCANCODE_KP_MINUS) == VK_SUBTRACT, "the keypad operators are the keypad's own keys");
	Check(Unprinted(SDL_SCANCODE_KP_MULTIPLY) == VK_MULTIPLY && Unprinted(SDL_SCANCODE_KP_DIVIDE) == VK_DIVIDE, "whether or not Num Lock is on");

	Check(Unprinted(SDL_SCANCODE_LSHIFT) == VK_SHIFT && Unprinted(SDL_SCANCODE_RSHIFT) == VK_SHIFT, "either Shift is Shift");
	Check(Unprinted(SDL_SCANCODE_LCTRL) == VK_CONTROL && Unprinted(SDL_SCANCODE_RCTRL) == VK_CONTROL, "either Ctrl is Ctrl");
	Check(Unprinted(SDL_SCANCODE_LALT) == VK_MENU && Unprinted(SDL_SCANCODE_RALT) == VK_MENU, "either Alt is Alt");
	Check(Unprinted(SDL_SCANCODE_LGUI) == VK_LWIN && Unprinted(SDL_SCANCODE_RGUI) == VK_RWIN, "the Windows keys keep their sides");

	Check(Unprinted(SDL_SCANCODE_F1) == VK_F1 && Unprinted(SDL_SCANCODE_F12) == VK_F12, "the function keys run from F1 to F12");
	Check(Unprinted(SDL_SCANCODE_F13) == VK_F13 && Unprinted(SDL_SCANCODE_F24) == VK_F24, "and on from F13 to F24");
	Check(Unprinted(SDL_SCANCODE_PAGEUP) == VK_PRIOR && Unprinted(SDL_SCANCODE_PAGEDOWN) == VK_NEXT, "Page Up and Page Down are Prior and Next");
	Check(Unprinted(SDL_SCANCODE_LEFT) == VK_LEFT && Unprinted(SDL_SCANCODE_DOWN) == VK_DOWN, "the arrows are the arrows");
	Check(Unprinted(SDL_SCANCODE_ESCAPE) == VK_ESCAPE && Unprinted(SDL_SCANCODE_BACKSPACE) == VK_BACK, "Escape and Backspace are Escape and Back");
	Check(Key(SDL_SCANCODE_RETURN, SDLK_RETURN) == VK_RETURN && Key(SDL_SCANCODE_SPACE, SDLK_SPACE) == VK_SPACE, "Enter and Space are Return and Space");
	Check(Unprinted(SDL_SCANCODE_SCROLLLOCK) == VK_SCROLL && Unprinted(SDL_SCANCODE_PAUSE) == VK_PAUSE, "Scroll Lock and Pause are their own keys");
	Check(Unprinted(SDL_SCANCODE_PRINTSCREEN) == VK_SNAPSHOT, "Print Screen is the snapshot key");
	Check(Unprinted(SDL_SCANCODE_CAPSLOCK) == VK_CAPITAL && Unprinted(SDL_SCANCODE_NUMLOCKCLEAR) == VK_NUMLOCK, "Caps Lock and Num Lock are their own keys");
	Check(Unprinted(SDL_SCANCODE_LANG1) == 0, "a key Windows has no code for is dropped");

	bool named = true;
	for (int letter = 0; letter < 26; letter++) {
		char name[2] = { (char)('A' + letter), '\0' };
		SDL_Scancode scancode = SDL_GetScancodeFromName(name);
		named = named && Unprinted(scancode) == 'A' + letter;
	}
	Check(named, "every letter key named by SDL is its letter when its layout prints none");

	HKL const current = GetKeyboardLayout(0);
	Check(Scanned(SDL_SCANCODE_F1, 0x3B, current) == VK_F1 && Scanned(SDL_SCANCODE_UP, 0xE048, current) == VK_UP, "a scan code leaves the function and arrow keys their fixed codes");
	Check(Scanned(SDL_SCANCODE_KP_8, 0x48, current, SDL_KMOD_NUM) == VK_NUMPAD8 && Scanned(SDL_SCANCODE_KP_8, 0x48, current) == VK_UP, "and the keypad still follows Num Lock");
	Check(Scanned(SDL_SCANCODE_RSHIFT, 0x36, current) == VK_SHIFT && Scanned(SDL_SCANCODE_RCTRL, 0xE01D, current) == VK_CONTROL, "and either side of a modifier is still the modifier");
	Check(Scanned(SDL_SCANCODE_PAUSE, 0xE046, current) == VK_PAUSE, "and Pause is Pause");

	bool native = true;
	for (unsigned int raw = 0x02; raw <= 0x35; raw++) {
		UINT const expected = MapVirtualKeyExW(raw, MAPVK_VSC_TO_VK_EX, current);
		if (expected >= 'A' && expected <= 'Z') {
			native = native && Scanned(SDL_SCANCODE_A, raw, current) == (int)expected;
		}
	}
	Check(native, "a character key takes the code the active layout gives its scan code");

	{
		GermanLayout german;
		if (german.Layout != NULL && MapVirtualKeyExW(0x0C, MAPVK_VSC_TO_VK_EX, german.Layout) == 0xDB) {
			Check(Scanned(SDL_SCANCODE_MINUS, 0x0C, german.Layout) == 0xDB, "on a German layout, the sharp s key keeps its own code");
			Check(Scanned(SDL_SCANCODE_SLASH, 0x35, german.Layout) == VK_OEM_MINUS, "and the minus key keeps the minus code");
			Check(Scanned(SDL_SCANCODE_SEMICOLON, 0x27, german.Layout) == 0xC0 && Scanned(SDL_SCANCODE_LEFTBRACKET, 0x1A, german.Layout) == 0xBA, "and the umlaut keys keep theirs");
			Check(Scanned(SDL_SCANCODE_Y, 0x15, german.Layout) == 'Z' && Scanned(SDL_SCANCODE_Z, 0x2C, german.Layout) == 'Y', "and the Z and Y keys swap");
			Check(Virtual_Key_Name(0xDB, german.Layout) == "\xC3\x9F" && Virtual_Key_Name(0xC0, german.Layout) == "\xC3\x96", "and those keys are named by what German prints on them");
		} else {
			std::printf("%-76s %s\n", "the German layout checks", "skipped: the system has no German layout");
		}
	}

	// With no video subsystem started, SDL names keys from its US layout.
	Check(Virtual_Key_Name('A') == "A" && Virtual_Key_Name('7') == "7", "a letter or digit key is named by what it prints");
	Check(Virtual_Key_Name(VK_OEM_COMMA) == ",", "a punctuation key is named by what it prints");
	Check(Virtual_Key_Name(VK_NUMPAD7) == "Keypad 7", "a keypad digit is named as the keypad's");
	Check(Virtual_Key_Name(VK_HOME) == "Home" && Virtual_Key_Name(VK_INSERT) == "Insert", "a navigation key is named after the main block's key");
	Check(Virtual_Key_Name(VK_F1) == "F1" && Virtual_Key_Name(VK_F12) == "F12", "a function key is named by its number");
	Check(Virtual_Key_Name(VK_RETURN) == "Return" && Virtual_Key_Name(VK_SPACE) == "Space", "Enter and Space have SDL's names");
	Check(Virtual_Key_Name(VK_SHIFT) == "Shift" && Virtual_Key_Name(VK_CONTROL) == "Ctrl" && Virtual_Key_Name(VK_MENU) == "Alt", "the modifiers are named without a side");
	Check(Virtual_Key_Name(0).empty() && Virtual_Key_Name(0xFF).empty(), "a code no key produces has no name");

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
