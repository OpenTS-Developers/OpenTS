/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uiinput.h"


namespace
{

bool Is_UI(UIInputOwner owner)
{
	return(owner == UI_INPUT_RML || owner == UI_INPUT_IMGUI);
}

}


bool UI_Consumes_Input(UIInputOwner owner)
{
	return(Is_UI(owner) || owner == UI_INPUT_SUPPRESSED);
}


UIInputOwner UIInputStateClass::Press_Key(unsigned key, UIInputOwner owner)
{
	if (key >= Keys.size()) {
		return(UI_INPUT_NONE);
	}
	if (Keys[key] == UI_INPUT_NONE) {
		Keys[key] = owner;
	}
	return(Keys[key]);
}


UIInputOwner UIInputStateClass::Release_Key(unsigned key)
{
	if (key >= Keys.size()) {
		return(UI_INPUT_NONE);
	}
	UIInputOwner owner = Keys[key];
	Keys[key] = UI_INPUT_NONE;
	return(owner);
}


UIInputOwner UIInputStateClass::Key_Owner(unsigned key) const
{
	return(key < Keys.size() ? Keys[key] : UI_INPUT_NONE);
}


UIInputOwner UIInputStateClass::Press_Mouse(unsigned button, UIInputOwner owner)
{
	if (button >= Buttons.size()) {
		return(UI_INPUT_NONE);
	}
	if (Buttons[button] == UI_INPUT_NONE) {
		Buttons[button] = owner;
	}
	return(Buttons[button]);
}


UIInputOwner UIInputStateClass::Release_Mouse(unsigned button)
{
	if (button >= Buttons.size()) {
		return(UI_INPUT_NONE);
	}
	UIInputOwner owner = Buttons[button];
	Buttons[button] = UI_INPUT_NONE;
	return(owner);
}


UIInputOwner UIInputStateClass::Mouse_Owner(unsigned button) const
{
	return(button < Buttons.size() ? Buttons[button] : UI_INPUT_NONE);
}


UIInputOwner UIInputStateClass::Gesture_Owner(void) const
{
	for (UIInputOwner owner : Buttons) {
		if (owner != UI_INPUT_NONE) {
			return(owner);
		}
	}
	return(UI_INPUT_NONE);
}


bool UIInputStateClass::Has_UI_Mouse(void) const
{
	for (UIInputOwner owner : Buttons) {
		if (Is_UI(owner)) {
			return(true);
		}
	}
	return(false);
}


bool UIInputStateClass::Any_Owned(void) const
{
	for (UIInputOwner owner : Keys) {
		if (owner != UI_INPUT_NONE) {
			return(true);
		}
	}
	return(Gesture_Owner() != UI_INPUT_NONE);
}


bool UIInputStateClass::Any_Suppressed(void) const
{
	for (UIInputOwner owner : Keys) {
		if (owner == UI_INPUT_SUPPRESSED) {
			return(true);
		}
	}
	for (UIInputOwner owner : Buttons) {
		if (owner == UI_INPUT_SUPPRESSED) {
			return(true);
		}
	}
	return(false);
}


void UIInputStateClass::Cancel_UI(void)
{
	for (UIInputOwner & owner : Keys) {
		if (Is_UI(owner)) {
			owner = UI_INPUT_SUPPRESSED;
		}
	}
	for (UIInputOwner & owner : Buttons) {
		if (Is_UI(owner)) {
			owner = UI_INPUT_SUPPRESSED;
		}
	}
}


void UIInputStateClass::Cancel_Mouse(void)
{
	for (UIInputOwner & owner : Buttons) {
		if (owner != UI_INPUT_NONE) {
			owner = UI_INPUT_SUPPRESSED;
		}
	}
}


void UIInputStateClass::Cancel_All(void)
{
	for (UIInputOwner & owner : Keys) {
		if (owner != UI_INPUT_NONE) {
			owner = UI_INPUT_SUPPRESSED;
		}
	}
	for (UIInputOwner & owner : Buttons) {
		if (owner != UI_INPUT_NONE) {
			owner = UI_INPUT_SUPPRESSED;
		}
	}
}


void UIInputStateClass::Reconcile_Cancelled_Keys(std::array<bool, KEY_COUNT> const & physical)
{
	for (std::size_t key = 0; key < Keys.size(); key++) {
		if (Keys[key] == UI_INPUT_SUPPRESSED && !physical[key]) {
			Keys[key] = UI_INPUT_NONE;
		}
	}
}


void UIInputStateClass::Reconcile_Cancelled_Mouse(std::array<bool, BUTTON_COUNT> const & physical)
{
	for (std::size_t button = 0; button < Buttons.size(); button++) {
		if (Buttons[button] == UI_INPUT_SUPPRESSED && !physical[button]) {
			Buttons[button] = UI_INPUT_NONE;
		}
	}
}


void UIInputStateClass::Reset(void)
{
	Keys.fill(UI_INPUT_NONE);
	Buttons.fill(UI_INPUT_NONE);
}


UIInputText UIUTF8DecoderClass::Feed(unsigned char byte)
{
	UIInputText result;

	if (Remaining != 0) {
		if ((byte & 0xC0) == 0x80) {
			Value = (Value << 6) | (byte & 0x3F);
			if (--Remaining == 0) {
				bool valid = Value >= Minimum && Value <= 0x10FFFF && !(Value >= 0xD800 && Value <= 0xDFFF);
				result.Codepoints[result.Count++] = valid ? Value : 0xFFFD;
				Reset();
			}
			return(result);
		}
		result.Codepoints[result.Count++] = 0xFFFD;
		Reset();
	}

	if (byte < 0x80) {
		result.Codepoints[result.Count++] = byte;
	} else if (byte >= 0xC2 && byte <= 0xDF) {
		Value = byte & 0x1F;
		Minimum = 0x80;
		Remaining = 1;
	} else if (byte >= 0xE0 && byte <= 0xEF) {
		Value = byte & 0x0F;
		Minimum = 0x800;
		Remaining = 2;
	} else if (byte >= 0xF0 && byte <= 0xF4) {
		Value = byte & 0x07;
		Minimum = 0x10000;
		Remaining = 3;
	} else {
		result.Codepoints[result.Count++] = 0xFFFD;
	}
	return(result);
}


void UIUTF8DecoderClass::Reset(void)
{
	Value = 0;
	Minimum = 0;
	Remaining = 0;
}
