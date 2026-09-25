/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "sdl/sdlinput.h"

#include "sdl/sdlkeys.h"
#include "win.h"
#include "windowevent.hh"

#include <SDL3/SDL_keyboard.h>

#include <cstring>


namespace
{

// Windows also answers for each side of a modifier.
int Side_Key(int scancode)
{
	switch (scancode) {
		case SDL_SCANCODE_LSHIFT:	return(VK_LSHIFT);
		case SDL_SCANCODE_RSHIFT:	return(VK_RSHIFT);
		case SDL_SCANCODE_LCTRL:	return(VK_LCONTROL);
		case SDL_SCANCODE_RCTRL:	return(VK_RCONTROL);
		case SDL_SCANCODE_LALT:		return(VK_LMENU);
		case SDL_SCANCODE_RALT:		return(VK_RMENU);
		default:					return(0);
	}
}


int Generic_Key(int sidekey)
{
	switch (sidekey) {
		case VK_LSHIFT:
		case VK_RSHIFT:		return(VK_SHIFT);
		case VK_LCONTROL:
		case VK_RCONTROL:	return(VK_CONTROL);
		case VK_LMENU:
		case VK_RMENU:		return(VK_MENU);
		default:			return(0);
	}
}


// Mouse buttons are read live, and a modifier is held by its sides.
bool Holdable_Key(int virtualkey)
{
	switch (virtualkey) {
		case VK_LBUTTON:
		case VK_RBUTTON:
		case VK_MBUTTON:
		case VK_XBUTTON1:
		case VK_XBUTTON2:
		case VK_SHIFT:
		case VK_CONTROL:
		case VK_MENU:
			return(false);

		default:
			return(virtualkey > 0 && virtualkey < 255);
	}
}


bool Windows_Holds(int virtualkey)
{
	return((GetKeyState(virtualkey) & 0x8000) != 0);
}

}


SDLInputStateClass::SDLInputStateClass(void)
{
	Reset();
}


void SDLInputStateClass::Reset(void)
{
	std::memset(PressedAs, 0, sizeof(PressedAs));
	std::memset(Unreported, 0, sizeof(Unreported));
	UnreportedCount = 0;
	RightAltPresses.clear();
	AltGr = false;
	KeyModifiers = SDL_GetModState();
	Rebuild_Held_Keys();
}


void SDLInputStateClass::Note_Queued_Key(SDL_KeyboardEvent const & key)
{
	if (key.down && key.scancode == SDL_SCANCODE_RALT && !key.repeat) {
		bool const altgr = Windows_Holds(VK_LCONTROL) && !SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_LCTRL] && !Unreported[VK_LCONTROL];
		RightAltPresses.push_back({ key.timestamp, altgr });
	}
}


bool SDLInputStateClass::Track_Key(SDL_KeyboardEvent const & key)
{
	KeyModifiers = key.mod;
	if (key.down && key.repeat) {
		return(false);
	}

	int const virtualkey = key.down ? Virtual_Key_From_SDL(key.scancode, key.key, key.mod, key.raw) : 0;
	bool const continued = key.down && (Unreported_Key(Side_Key(key.scancode)) || Unreported_Key(virtualkey));
	Report_Key(Side_Key(key.scancode));
	Report_Key(virtualkey);

	Press_Key(key.scancode, virtualkey, key.down);
	if (key.scancode == SDL_SCANCODE_RALT) {
		AltGr = key.down && Pressed_As_AltGr(key.timestamp);
	}
	Rebuild_Held_Keys();
	return(continued);
}


void SDLInputStateClass::Focus_Gained(void)
{
	SDL_Keymod const locks = SDL_KMOD_CAPS | SDL_KMOD_NUM | SDL_KMOD_SCROLL;
	KeyModifiers = (SDL_Keymod)((KeyModifiers & ~locks) | (SDL_GetModState() & locks));
	Hold_Windows_Keys();
}


void SDLInputStateClass::Focus_Lost(void)
{
	Release_Unreported_Keys(true);
}


void SDLInputStateClass::Hold_Windows_Keys(void)
{
	for (int virtualkey = 1; virtualkey < 255; virtualkey++) {
		if (Holdable_Key(virtualkey) && !HeldKeys[virtualkey] && Windows_Holds(virtualkey)) {
			Unreported[virtualkey] = true;
			UnreportedCount++;
		}
	}
	Rebuild_Held_Keys();
}


void SDLInputStateClass::Drop_Released_Keys(void)
{
	Release_Unreported_Keys(false);
}


int SDLInputStateClass::Modifiers(void) const
{
	int modifiers = 0;
	if (HeldKeys[VK_SHIFT]) {
		modifiers |= WINDOW_MOD_SHIFT;
	}
	if (HeldKeys[VK_CONTROL]) {
		modifiers |= WINDOW_MOD_CTRL;
	}
	if (HeldKeys[VK_MENU]) {
		modifiers |= WINDOW_MOD_ALT;
	}
	if ((KeyModifiers & SDL_KMOD_CAPS) != 0) {
		modifiers |= WINDOW_MOD_CAPS;
	}
	if ((KeyModifiers & SDL_KMOD_NUM) != 0) {
		modifiers |= WINDOW_MOD_NUM;
	}
	return(modifiers);
}


bool SDLInputStateClass::Key_Down(int virtualkey)
{
	if (virtualkey <= 0 || virtualkey >= 256) {
		return(false);
	}

	Release_Unreported_Keys(false);
	return(HeldKeys[virtualkey]);
}


bool SDLInputStateClass::Key_Toggled(int virtualkey) const
{
	switch (virtualkey) {
		case VK_CAPITAL:	return((KeyModifiers & SDL_KMOD_CAPS) != 0);
		case VK_NUMLOCK:	return((KeyModifiers & SDL_KMOD_NUM) != 0);
		case VK_SCROLL:		return((KeyModifiers & SDL_KMOD_SCROLL) != 0);
		default:			return(false);
	}
}


void SDLInputStateClass::Rebuild_Held_Keys(void)
{
	std::memset(HeldKeys, 0, sizeof(HeldKeys));
	for (int scancode = 0; scancode < SDL_SCANCODE_COUNT; scancode++) {
		if (PressedAs[scancode] != 0) {
			HeldKeys[PressedAs[scancode]] = true;
			HeldKeys[Side_Key(scancode)] = true;
		}
	}
	for (int virtualkey = 1; virtualkey < 255 && UnreportedCount > 0; virtualkey++) {
		if (Unreported[virtualkey]) {
			HeldKeys[virtualkey] = true;
			HeldKeys[Generic_Key(virtualkey)] = true;
		}
	}
	HeldKeys[0] = false;

	// Windows holds Left Ctrl down for as long as AltGr is held.
	if (AltGr) {
		HeldKeys[VK_CONTROL] = true;
		HeldKeys[VK_LCONTROL] = true;
	}
}


void SDLInputStateClass::Press_Key(SDL_Scancode scancode, int virtualkey, bool down)
{
	if (scancode <= SDL_SCANCODE_UNKNOWN || scancode >= SDL_SCANCODE_COUNT) {
		return;
	}
	PressedAs[scancode] = (down && virtualkey > 0 && virtualkey < 256) ? (unsigned char)virtualkey : 0;
}


bool SDLInputStateClass::Unreported_Key(int virtualkey) const
{
	return(virtualkey > 0 && virtualkey < 256 && Unreported[virtualkey]);
}


void SDLInputStateClass::Report_Key(int virtualkey)
{
	if (Unreported_Key(virtualkey)) {
		Unreported[virtualkey] = false;
		UnreportedCount--;
	}
}


// Takes what Note_Queued_Key noted when SDL queued this Right Alt press.
bool SDLInputStateClass::Pressed_As_AltGr(Uint64 timestamp)
{
	for (size_t index = 0; index < RightAltPresses.size(); index++) {
		if (RightAltPresses[index].Timestamp == timestamp) {
			bool const altgr = RightAltPresses[index].AltGr;
			RightAltPresses.erase(RightAltPresses.begin(), RightAltPresses.begin() + index + 1);
			return(altgr);
		}
	}
	return(false);
}


void SDLInputStateClass::Release_Unreported_Keys(bool all)
{
	if (UnreportedCount == 0) {
		return;
	}

	bool released = false;
	for (int virtualkey = 1; virtualkey < 255; virtualkey++) {
		if (Unreported[virtualkey] && (all || !Windows_Holds(virtualkey))) {
			Report_Key(virtualkey);
			released = true;
		}
	}
	if (released) {
		Rebuild_Held_Keys();
	}
}
