/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <SDL3/SDL_events.h>

#include <deque>


// The keys held and the lock states as of the input event being dispatched, by the Windows
// virtual-key codes the game reads.
class SDLInputStateClass
{
	public:
		SDLInputStateClass(void);

		// SDL sees no key before its window has the focus, so only the lock keys can start set.
		void Reset(void);

		// Runs as SDL queues a key event, while Windows still shows the Left Ctrl it presses
		// with AltGr.
		void Note_Queued_Key(SDL_KeyboardEvent const & key);

		// Returns true for a press of a key held since before SDL's last keyboard reset, which
		// Windows reports as a repeat.
		bool Track_Key(SDL_KeyboardEvent const & key);

		// SDL resets the keyboard on a focus loss and when a window drag, a resize or the system
		// menu starts; these hold again the keys Windows still holds.
		void Focus_Gained(void);
		void Hold_Windows_Keys(void);

		void Focus_Lost(void);

		// Drops the keys held through a keyboard reset that Windows has since released, since SDL
		// drops their release.
		void Drop_Released_Keys(void);

		// The WINDOW_MOD_* modifiers held and locks on.
		int Modifiers(void) const;

		bool Key_Down(int virtualkey);
		bool Key_Toggled(int virtualkey) const;

	private:
		struct RightAltPress
		{
			Uint64 Timestamp;
			bool AltGr;
		};

		// Each held scancode keeps the code it was pressed as, so its release clears that code.
		bool HeldKeys[256];
		unsigned char PressedAs[SDL_SCANCODE_COUNT];
		SDL_Keymod KeyModifiers;

		// Right Alt presses queued but not yet handled, and whether each was AltGr.
		std::deque<RightAltPress> RightAltPresses;

		// Set while the Right Alt held was pressed as AltGr.
		bool AltGr;

		// Set, by virtual-key code and by side for a modifier, for a key held through a keyboard
		// reset, which is read from Windows until SDL reports the key.
		bool Unreported[256];
		int UnreportedCount;

		void Rebuild_Held_Keys(void);
		void Press_Key(SDL_Scancode scancode, int virtualkey, bool down);
		bool Unreported_Key(int virtualkey) const;
		void Report_Key(int virtualkey);
		bool Pressed_As_AltGr(Uint64 timestamp);
		void Release_Unreported_Keys(bool all);
};
