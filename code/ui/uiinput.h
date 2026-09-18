/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ui/uiinput.hh"

#include <array>
#include <cstddef>


bool UI_Consumes_Input(UIInputOwner owner);


class UIInputStateClass
{
	public:
		static constexpr unsigned KEY_COUNT = 256;
		static constexpr unsigned BUTTON_COUNT = 5;

		UIInputOwner Press_Key(unsigned key, UIInputOwner owner);
		UIInputOwner Release_Key(unsigned key);
		UIInputOwner Key_Owner(unsigned key) const;
		UIInputOwner Press_Mouse(unsigned button, UIInputOwner owner);
		UIInputOwner Release_Mouse(unsigned button);
		UIInputOwner Mouse_Owner(unsigned button) const;
		UIInputOwner Gesture_Owner(void) const;
		bool Has_UI_Mouse(void) const;
		bool Any_Owned(void) const;
		bool Any_Suppressed(void) const;
		void Cancel_UI(void);
		void Cancel_Mouse(void);
		void Cancel_Keys(void);
		void Cancel_All(void);
		void Reconcile_Cancelled_Keys(std::array<bool, KEY_COUNT> const & physical);
		void Reconcile_Cancelled_Mouse(std::array<bool, BUTTON_COUNT> const & physical);
		void Reset(void);

	private:
		std::array<UIInputOwner, KEY_COUNT> Keys {};
		std::array<UIInputOwner, BUTTON_COUNT> Buttons {};
};


struct UIInputText
{
	std::array<char32_t, 2> Codepoints {};
	unsigned Count = 0;
};


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
