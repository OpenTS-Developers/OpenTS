/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins the keys the SDL layer reports held, by the Windows codes the game reads, as SDL's key
// events and Windows' own key state change.

#include "sdl/sdlinput.h"

#include "win.h"
#include "windowevent.hh"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>

#include <cstdio>
#include <initializer_list>

namespace {

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-76s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


Uint64 Clock = 0;


SDL_KeyboardEvent Key(bool down, SDL_Scancode scancode, SDL_Keycode keycode, SDL_Keymod modifiers = SDL_KMOD_NONE)
{
	SDL_KeyboardEvent key = {};
	key.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
	key.timestamp = ++Clock;
	key.scancode = scancode;
	key.key = keycode;
	key.mod = modifiers;
	key.down = down;
	return(key);
}


// Queues and then handles a key, as the SDL layer does.
void Press(SDLInputStateClass & input, SDL_KeyboardEvent const & key)
{
	input.Note_Queued_Key(key);
	input.Track_Key(key);
}


void Windows_Holds(std::initializer_list<int> keys)
{
	BYTE state[256] = {};
	for (int key : keys) {
		state[key] = 0x80;
	}
	SetKeyboardState(state);
}

}


int main(void)
{
	Windows_Holds({});
	SDLInputStateClass input;

	Press(input, Key(true, SDL_SCANCODE_LSHIFT, SDLK_LSHIFT, SDL_KMOD_LSHIFT));
	Check(input.Key_Down(VK_SHIFT) && input.Key_Down(VK_LSHIFT) && !input.Key_Down(VK_RSHIFT) && input.Modifiers() == WINDOW_MOD_SHIFT, "a Shift press holds Shift and the side it was pressed on");
	Press(input, Key(false, SDL_SCANCODE_LSHIFT, SDLK_LSHIFT));
	Check(!input.Key_Down(VK_SHIFT) && !input.Key_Down(VK_LSHIFT) && input.Modifiers() == 0, "and its release lets both go");

	// Windows shows the Left Ctrl it presses for AltGr only while the Right Alt press is queued.
	SDL_KeyboardEvent altgr = Key(true, SDL_SCANCODE_RALT, SDLK_RALT, SDL_KMOD_RALT);
	Windows_Holds({ VK_CONTROL, VK_LCONTROL });
	input.Note_Queued_Key(altgr);
	Windows_Holds({});
	input.Track_Key(altgr);
	Check(input.Key_Down(VK_CONTROL) && input.Key_Down(VK_LCONTROL) && input.Modifiers() == (WINDOW_MOD_CTRL | WINDOW_MOD_ALT), "a Right Alt queued while Windows shows Left Ctrl is AltGr, with its Ctrl");
	Press(input, Key(false, SDL_SCANCODE_RALT, SDLK_RALT));
	Check(!input.Key_Down(VK_CONTROL) && input.Modifiers() == 0, "and Ctrl is released with AltGr");

	Press(input, Key(true, SDL_SCANCODE_RALT, SDLK_RALT, SDL_KMOD_RALT));
	Check(!input.Key_Down(VK_CONTROL) && input.Key_Down(VK_RMENU) && input.Modifiers() == WINDOW_MOD_ALT, "a Right Alt queued without it is Alt alone");
	Press(input, Key(false, SDL_SCANCODE_RALT, SDLK_RALT));

	SDL_SetModState(SDL_KMOD_CAPS);
	input.Focus_Gained();
	Check(input.Key_Toggled(VK_CAPITAL) && (input.Modifiers() & WINDOW_MOD_CAPS) != 0, "the lock keys are read again when the window regains the focus");
	SDL_SetModState(SDL_KMOD_NONE);
	input.Focus_Gained();
	Check(!input.Key_Toggled(VK_CAPITAL), "and again when a lock goes off");

	// SDL reports no key still held after it resets the keyboard, and drops such a key's release.
	Windows_Holds({ VK_SHIFT, VK_LSHIFT });
	input.Focus_Gained();
	Check(input.Key_Down(VK_SHIFT) && input.Key_Down(VK_LSHIFT) && !input.Key_Down(VK_RSHIFT) && input.Modifiers() == WINDOW_MOD_SHIFT, "a Shift Windows holds as the window gains the focus is held");
	Windows_Holds({});
	input.Drop_Released_Modifiers();
	Check(input.Modifiers() == 0, "and released for the next event once Windows releases it");
	Windows_Holds({ VK_SHIFT, VK_LSHIFT });
	input.Focus_Gained();
	Windows_Holds({});
	Check(!input.Key_Down(VK_SHIFT), "and read released at once, with no event");

	Windows_Holds({ VK_SHIFT, VK_LSHIFT });
	input.Focus_Gained();
	input.Focus_Lost();
	Check(!input.Key_Down(VK_SHIFT), "losing the focus releases it");

	input.Focus_Gained();
	Press(input, Key(true, SDL_SCANCODE_LSHIFT, SDLK_LSHIFT, SDL_KMOD_LSHIFT));
	Windows_Holds({});
	Check(input.Key_Down(VK_SHIFT), "once SDL reports the key, Windows no longer releases it");
	Press(input, Key(false, SDL_SCANCODE_LSHIFT, SDLK_LSHIFT));
	Check(!input.Key_Down(VK_SHIFT), "and SDL's release does");

	Windows_Holds({ VK_MENU, VK_RMENU });
	input.Hold_Windows_Modifiers();
	Check(input.Key_Down(VK_MENU) && input.Key_Down(VK_RMENU) && !input.Key_Down(VK_LMENU), "a modifier Windows holds after a window drag is held");
	Windows_Holds({});
	input.Drop_Released_Modifiers();

	Windows_Holds({ VK_CONTROL, VK_LCONTROL });
	input.Focus_Gained();
	SDL_KeyboardEvent ralt = Key(true, SDL_SCANCODE_RALT, SDLK_RALT, SDL_KMOD_RALT);
	input.Note_Queued_Key(ralt);
	Windows_Holds({});
	input.Drop_Released_Modifiers();
	input.Track_Key(ralt);
	Check(!input.Key_Down(VK_CONTROL) && input.Modifiers() == WINDOW_MOD_ALT, "a Right Alt pressed with a Left Ctrl held into the window is not AltGr");
	Press(input, Key(false, SDL_SCANCODE_RALT, SDLK_RALT));

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
