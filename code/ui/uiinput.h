/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Who owns each held key and mouse button, decided at the press and returned at the
// release, so a gesture ends where it began whatever moved in between. No toolkit or
// window type appears here; the shell's hook feeds it and a harness drives it by hand.

#pragma once

#include "ui/uiinput.hh"

#include <array>
#include <cstddef>


// True when the owner's message must be kept from the game: a toolkit owns it or it was
// cancelled. Input nobody owns is the game's, including a release whose press the shell
// never saw.
bool UI_Consumes_Input(UIInputOwner owner);


class UIInputStateClass
{
	public:
		static constexpr unsigned KEY_COUNT = 256;
		static constexpr unsigned BUTTON_COUNT = 5;

		// Latches the owner of a press and returns the owner in charge: a held key or button
		// keeps its first owner.
		UIInputOwner Press_Key(unsigned key, UIInputOwner owner);
		UIInputOwner Release_Key(unsigned key);
		UIInputOwner Key_Owner(unsigned key) const;
		UIInputOwner Press_Mouse(unsigned button, UIInputOwner owner);
		UIInputOwner Release_Mouse(unsigned button);
		UIInputOwner Mouse_Owner(unsigned button) const;

		// The owner of the first held button, which owns the pointer's motion.
		UIInputOwner Gesture_Owner(void) const;
		bool Has_UI_Mouse(void) const;
		bool Any_Owned(void) const;
		bool Any_Suppressed(void) const;

		// What a toolkit held becomes suppressed when its screen closes.
		void Cancel_UI(void);
		// Every held button becomes suppressed when another window takes the capture.
		void Cancel_Mouse(void);
		// Everything held becomes suppressed when the window loses focus.
		void Cancel_All(void);
		// Suppressed entries are forgotten once the physical key or button is up, so a
		// release lost to another window cannot hold them forever.
		void Reconcile_Cancelled_Keys(std::array<bool, KEY_COUNT> const & physical);
		void Reconcile_Cancelled_Mouse(std::array<bool, BUTTON_COUNT> const & physical);
		void Reset(void);

	private:
		std::array<UIInputOwner, KEY_COUNT> Keys {};
		std::array<UIInputOwner, BUTTON_COUNT> Buttons {};
};


// The code points one byte of text completed: none while a sequence is pending, two when
// a malformed prefix is repaired and the byte after it stands on its own.
struct UIInputText
{
	std::array<char32_t, 2> Codepoints {};
	unsigned Count = 0;
};


// Decodes the UTF-8 bytes a narrow window delivers one message at a time. A malformed
// sequence becomes U+FFFD and never swallows the byte that follows it.
class UIUTF8DecoderClass
{
	public:
		UIInputText Feed(unsigned char byte);
		void Reset(void);

	private:
		char32_t Value = 0;
		char32_t Minimum = 0;
		unsigned Remaining = 0;
};
