/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <array>
#include <cstddef>


enum UIInputOwner
{
	UI_INPUT_NONE,
	UI_INPUT_GAME,
	UI_INPUT_RML,
	UI_INPUT_IMGUI,
	UI_INPUT_SUPPRESSED
};


enum UICursor
{
	UI_CURSOR_ARROW,
	UI_CURSOR_TEXT,
	UI_CURSOR_HAND,
	UI_CURSOR_RESIZE_NS,
	UI_CURSOR_RESIZE_EW,
	UI_CURSOR_RESIZE_NESW,
	UI_CURSOR_RESIZE_NWSE,
	UI_CURSOR_MOVE,
	UI_CURSOR_UNAVAILABLE
};


struct UIPointerPosition
{
	int X;
	int Y;
	bool Inside;
};


inline UIPointerPosition UI_Client_To_Overlay(int destx, int desty, int destwidth, int destheight, int clientx, int clienty)
{
	UIPointerPosition position;
	position.X = clientx - destx;
	position.Y = clienty - desty;
	position.Inside = position.X >= 0 && position.Y >= 0 && position.X < destwidth && position.Y < destheight;
	return(position);
}


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
