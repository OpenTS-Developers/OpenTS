/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins how main window messages become window events: the button and position each mouse
// message carries, the key flags, the characters a split or code page byte sequence decodes
// to, and the window state messages the game acts on.

#include "winevent.h"

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


struct ResultType
{
	int Count = 0;
	WindowEvent Events[4];
};


ResultType Translate(WinEventTranslatorClass & translator, UINT message, WPARAM wparam, LPARAM lparam, WinMessageContext const & context = WinMessageContext())
{
	ResultType result;
	result.Count = translator.Translate(message, wparam, lparam, context, result.Events, 4);
	return(result);
}


bool One(ResultType const & result, WindowEventType type)
{
	return(result.Count == 1 && result.Events[0].Type == type);
}

}


int main(void)
{
	WinEventTranslatorClass translator;

	ResultType result = Translate(translator, WM_MOUSEMOVE, 0, MAKELPARAM(12, 34));
	Check(One(result, WINDOW_EVENT_MOUSE_MOVE) && result.Events[0].X == 12 && result.Events[0].Y == 34, "a mouse move carries its client position");

	result = Translate(translator, WM_MOUSEMOVE, 0, MAKELPARAM(-5, -7));
	Check(One(result, WINDOW_EVENT_MOUSE_MOVE) && result.Events[0].X == -5 && result.Events[0].Y == -7, "a position left of or above the client area stays negative");

	result = Translate(translator, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(1, 2));
	Check(One(result, WINDOW_EVENT_MOUSE_DOWN) && result.Events[0].Button == WINDOW_BUTTON_LEFT && result.Events[0].Clicks == 1, "a left press is a single click of the left button");

	result = Translate(translator, WM_RBUTTONDBLCLK, MK_RBUTTON, MAKELPARAM(1, 2));
	Check(One(result, WINDOW_EVENT_MOUSE_DOWN) && result.Events[0].Button == WINDOW_BUTTON_RIGHT && result.Events[0].Clicks == 2, "a right double click is a press that completes two clicks");

	result = Translate(translator, WM_MBUTTONUP, 0, MAKELPARAM(1, 2));
	Check(One(result, WINDOW_EVENT_MOUSE_UP) && result.Events[0].Button == WINDOW_BUTTON_MIDDLE, "a middle release is a release of the middle button");

	result = Translate(translator, WM_XBUTTONDOWN, MAKEWPARAM(0, XBUTTON2), MAKELPARAM(1, 2));
	Check(One(result, WINDOW_EVENT_MOUSE_DOWN) && result.Events[0].Button == WINDOW_BUTTON_X2, "the second extra button keeps its identity");

	WinMessageContext wheelcontext;
	wheelcontext.ClientX = 100;
	wheelcontext.ClientY = 200;
	result = Translate(translator, WM_MOUSEWHEEL, MAKEWPARAM(0, (WORD)(-2 * WHEEL_DELTA)), MAKELPARAM(130, 240), wheelcontext);
	Check(One(result, WINDOW_EVENT_MOUSE_WHEEL) && result.Events[0].X == 30 && result.Events[0].Y == 40, "a wheel turn's screen position becomes a client position");
	Check(result.Count == 1 && result.Events[0].Wheel == -2.0f && !result.Events[0].Horizontal, "and two notches toward the player are -2 on the vertical wheel");

	result = Translate(translator, WM_MOUSEHWHEEL, MAKEWPARAM(0, WHEEL_DELTA), 0);
	Check(One(result, WINDOW_EVENT_MOUSE_WHEEL) && result.Events[0].Horizontal && result.Events[0].Wheel == 1.0f, "a horizontal notch to the right is 1 on the horizontal wheel");

	WinMessageContext shifted;
	shifted.Modifiers = WINDOW_MOD_SHIFT | WINDOW_MOD_CTRL;
	result = Translate(translator, WM_KEYDOWN, 'A', 0, shifted);
	Check(One(result, WINDOW_EVENT_KEY_DOWN) && result.Events[0].VirtualKey == 'A' && result.Events[0].Modifiers == (WINDOW_MOD_SHIFT | WINDOW_MOD_CTRL), "a key press carries its virtual key and the modifiers held");
	Check(result.Count == 1 && !result.Events[0].Repeat && !result.Events[0].System, "and is neither a repeat nor a system key");

	result = Translate(translator, WM_KEYDOWN, 'A', 1 << 30);
	Check(One(result, WINDOW_EVENT_KEY_DOWN) && result.Events[0].Repeat, "a press of a key already down is a repeat");

	result = Translate(translator, WM_SYSKEYDOWN, VK_F4, 0);
	Check(One(result, WINDOW_EVENT_KEY_DOWN) && result.Events[0].System, "a key pressed with Alt held is a system key");

	result = Translate(translator, WM_KEYUP, VK_ESCAPE, (LPARAM)(3u << 30));
	Check(One(result, WINDOW_EVENT_KEY_UP) && result.Events[0].VirtualKey == VK_ESCAPE && !result.Events[0].Repeat, "a release is never a repeat");

	result = Translate(translator, WM_CHAR, 0x041F, 0);
	Check(One(result, WINDOW_EVENT_TEXT) && result.Events[0].Text == 0x041F, "a character on a wide window is the character itself");

	result = Translate(translator, WM_CHAR, 0xD83D, 0);
	Check(result.Count == 0, "the first half of a surrogate pair waits for the second");
	result = Translate(translator, WM_CHAR, 0xDE00, 0);
	Check(One(result, WINDOW_EVENT_TEXT) && result.Events[0].Text == 0x1F600, "and the pair arrives as one astral character");

	Translate(translator, WM_CHAR, 0xD83D, 0);
	result = Translate(translator, WM_CHAR, 'x', 0);
	Check(result.Count == 2 && result.Events[0].Text == 0xFFFD && result.Events[1].Text == 'x', "an unpaired first half becomes a replacement before the next character");

	result = Translate(translator, WM_CHAR, 0xDE00, 0);
	Check(One(result, WINDOW_EVENT_TEXT) && result.Events[0].Text == 0xFFFD, "an unpaired second half becomes a replacement");

	WinMessageContext narrow;
	narrow.Unicode = false;
	narrow.CodePage = CP_UTF8;
	result = Translate(translator, WM_CHAR, 0xC3, 0, narrow);
	Check(result.Count == 0, "a UTF-8 lead byte on a narrow window waits for the rest");
	result = Translate(translator, WM_CHAR, 0xA9, 0, narrow);
	Check(One(result, WINDOW_EVENT_TEXT) && result.Events[0].Text == 0xE9, "and two UTF-8 bytes arrive as one character");

	narrow.CodePage = 1251;
	char32_t const cyrillic[3] = { 0x041F, 0x0440, 0x0438 };
	unsigned char const bytes[3] = { 0xCF, 0xF0, 0xE8 };
	bool letters = true;
	for (int index = 0; index < 3; index++) {
		result = Translate(translator, WM_CHAR, bytes[index], 0, narrow);
		letters = letters && One(result, WINDOW_EVENT_TEXT) && result.Events[0].Text == cyrillic[index];
	}
	Check(letters, "each code page byte arrives as the Cyrillic letter it names, not a replacement");

	Translate(translator, WM_CHAR, 0xD83D, 0);
	result = Translate(translator, WM_ACTIVATEAPP, FALSE, 0);
	Check(One(result, WINDOW_EVENT_FOCUS_LOST), "deactivation is a loss of focus");
	result = Translate(translator, WM_CHAR, 'y', 0);
	Check(One(result, WINDOW_EVENT_TEXT) && result.Events[0].Text == 'y', "and forgets half a character typed before it");

	result = Translate(translator, WM_ACTIVATEAPP, TRUE, 0);
	Check(One(result, WINDOW_EVENT_FOCUS_GAINED), "activation is a gain of focus");

	WinMessageContext captured;
	result = Translate(translator, WM_CAPTURECHANGED, 0, 0, captured);
	Check(result.Count == 0, "a capture change to this window loses nothing");
	captured.CaptureElsewhere = true;
	result = Translate(translator, WM_CAPTURECHANGED, 0, 0, captured);
	Check(One(result, WINDOW_EVENT_CAPTURE_LOST), "a capture change to another window loses the capture");
	result = Translate(translator, WM_CANCELMODE, 0, 0);
	Check(One(result, WINDOW_EVENT_CAPTURE_LOST), "a cancelled mode loses the capture");

	result = Translate(translator, WM_INPUTLANGCHANGE, 0, 0);
	Check(One(result, WINDOW_EVENT_KEYMAP_CHANGED), "an input language change changes the key map");

	result = Translate(translator, WM_SIZE, SIZE_RESTORED, MAKELPARAM(800, 600));
	Check(One(result, WINDOW_EVENT_RESIZED) && result.Events[0].Width == 800 && result.Events[0].Height == 600, "a resize carries the new client size");
	result = Translate(translator, WM_SIZE, SIZE_MINIMIZED, 0);
	Check(result.Count == 0, "minimizing is not a resize");

	Check(One(Translate(translator, WM_PAINT, 0, 0), WINDOW_EVENT_EXPOSED), "a paint request exposes the window");
	Check(One(Translate(translator, WM_MOVE, 0, 0), WINDOW_EVENT_MOVED), "a move moves the window");
	Check(One(Translate(translator, WM_DISPLAYCHANGE, 0, 0), WINDOW_EVENT_DISPLAY_CHANGED), "a display change changes the display");
	Check(One(Translate(translator, WM_SYSCOMMAND, SC_CLOSE, 0), WINDOW_EVENT_CLOSE_REQUESTED), "the close command asks to close");
	Check(Translate(translator, WM_SYSCOMMAND, SC_MINIMIZE, 0).Count == 0, "any other system command is not a close");
	Check(Translate(translator, WM_TIMER, 1, 0).Count == 0, "a message with no event produces none");

	WindowEvent small[1];
	Translate(translator, WM_CHAR, 0xD83D, 0);
	int written = translator.Translate(WM_CHAR, 'z', 0, WinMessageContext(), small, 1);
	Check(written == 1 && small[0].Text == 0xFFFD, "no more events are written than there is room for");

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
