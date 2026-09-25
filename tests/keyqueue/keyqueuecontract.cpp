/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins the key queue the game's input loops read, including the character each press typed,
// which chat, edit boxes and the high-score name read back.

#include "keyboard.h"
#include "platform/windowevent.hh"

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


WindowEvent Key(bool down, int virtualkey, int modifiers = 0, bool repeat = false)
{
	WindowEvent event;
	event.Type = down ? WINDOW_EVENT_KEY_DOWN : WINDOW_EVENT_KEY_UP;
	event.VirtualKey = virtualkey;
	event.Modifiers = modifiers;
	event.Repeat = repeat;
	return(event);
}


WindowEvent Text(char32_t code)
{
	WindowEvent event;
	event.Type = WINDOW_EVENT_TEXT;
	event.Text = code;
	return(event);
}


WindowEvent Click(bool down, WindowMouseButton button, int x, int y, int clicks = 1)
{
	WindowEvent event;
	event.Type = down ? WINDOW_EVENT_MOUSE_DOWN : WINDOW_EVENT_MOUSE_UP;
	event.Button = button;
	event.X = x;
	event.Y = y;
	event.Clicks = clicks;
	return(event);
}

}


int main(void)
{
	WWKeyboardClass keyboard;

	keyboard.Handle_Window_Event(Key(true, 'A', WINDOW_MOD_SHIFT));
	keyboard.Handle_Window_Event(Text('A'));
	keyboard.Handle_Window_Event(Key(false, 'A', WINDOW_MOD_SHIFT));
	unsigned short key = keyboard.Get();
	Check(key == ('A' | WWKEY_SHIFT_BIT), "a key press is queued with the modifiers held");
	Check(keyboard.To_ASCII(key) == 'A', "and gives back the character it typed");
	key = keyboard.Get();
	Check(key == ('A' | WWKEY_SHIFT_BIT | WWKEY_RLS_BIT), "its release follows it");
	Check(keyboard.To_ASCII(key) == 0, "and types nothing");
	Check(keyboard.Check() == 0, "and nothing else was queued for the character");

	keyboard.Handle_Window_Event(Key(true, 'E'));
	keyboard.Handle_Window_Event(Text(0x00E9));
	key = keyboard.Get();
	Check(key == 'E' && keyboard.To_ASCII(key) == 0x00E9, "an accented character is read back from its key");

	keyboard.Handle_Window_Event(Key(true, 'Q'));
	keyboard.Handle_Window_Event(Key(true, 'Q', 0, true));
	keyboard.Handle_Window_Event(Text('q'));
	key = keyboard.Get();
	Check(key == 'Q' && keyboard.To_ASCII(key) == 0, "a held key's repeat neither queues nor types");
	Check(keyboard.Check() == 0, "and its character is dropped");

	keyboard.Handle_Window_Event(Key(false, 'Q'));
	keyboard.Get();
	keyboard.Handle_Window_Event(Text(0x65E5));
	key = (keyboard.Check() != 0) ? keyboard.Get() : 0;
	Check(key == KN_TEXT && keyboard.To_ASCII(key) == 0x65E5, "text after the repeating key's release is kept");

	keyboard.Handle_Window_Event(Key(true, VK_OEM_7));
	keyboard.Handle_Window_Event(Key(true, 'X'));
	keyboard.Handle_Window_Event(Text(0x00B4));
	keyboard.Handle_Window_Event(Text('x'));
	key = keyboard.Get();
	Check(key == VK_OEM_7 && keyboard.To_ASCII(key) == 0, "a dead key types nothing of its own");
	key = keyboard.Get();
	Check(key == 'X' && keyboard.To_ASCII(key) == 0x00B4, "the next key carries the first character the two typed");
	key = keyboard.Get();
	Check(key == KN_TEXT && keyboard.To_ASCII(key) == 'x', "and a second character arrives as a text entry of its own");

	keyboard.Handle_Window_Event(Text(0x65E5));
	key = keyboard.Get();
	Check(key == KN_TEXT && keyboard.To_ASCII(key) == 0x65E5, "text with no key press, as an input method sends it, is a text entry");

	keyboard.Handle_Window_Event(Key(true, 'Z'));
	keyboard.Clear();
	keyboard.Handle_Window_Event(Text('z'));
	key = keyboard.Get();
	Check(key == KN_TEXT && keyboard.To_ASCII(key) == 'z', "clearing the queue forgets the press a character would have joined");

	keyboard.Handle_Window_Event(Key(true, VK_ESCAPE));
	Check(keyboard.To_ASCII(keyboard.Get()) == 0x1B, "Esc gives its control character");
	keyboard.Handle_Window_Event(Key(true, VK_RETURN, WINDOW_MOD_SHIFT));
	Check(keyboard.To_ASCII(keyboard.Get()) == 0x0D, "Enter gives a carriage return with any modifier");
	keyboard.Handle_Window_Event(Key(true, VK_BACK));
	Check(keyboard.To_ASCII(keyboard.Get()) == 0x08, "Backspace gives a backspace");
	keyboard.Handle_Window_Event(Key(true, VK_TAB));
	Check(keyboard.To_ASCII(keyboard.Get()) == 0x09, "Tab gives a tab");

	keyboard.Handle_Window_Event(Key(true, VK_RETURN));
	keyboard.Handle_Window_Event(Text(0x65E5));
	key = keyboard.Get();
	Check(keyboard.To_ASCII(key) == 0x0D, "text arriving after Enter leaves Enter its carriage return");
	key = keyboard.Get();
	Check(key == KN_TEXT && keyboard.To_ASCII(key) == 0x65E5, "and arrives as a text entry");

	keyboard.Handle_Window_Event(Key(true, 'B'));
	keyboard.Handle_Window_Event(Text('b'));
	keyboard.Handle_Window_Event(Key(true, 'C'));
	unsigned short first = keyboard.Get();
	unsigned short second = keyboard.Get();
	Check(keyboard.To_ASCII(second) == 0 && keyboard.To_ASCII(first) == 0, "only the key fetched last reads back a character");

	keyboard.Handle_Window_Event(Key(true, 'M', WINDOW_MOD_CTRL | WINDOW_MOD_ALT));
	key = keyboard.Get();
	Check(key == ('M' | WWKEY_CTRL_BIT | WWKEY_ALT_BIT), "Ctrl and Alt become their modifier bits");

	keyboard.Handle_Window_Event(Click(true, WINDOW_BUTTON_LEFT, 30, 40));
	key = keyboard.Get();
	Check(key == KN_LMOUSE && keyboard.MouseQX == 30 && keyboard.MouseQY == 40, "a click carries where it happened");
	Check(keyboard.To_ASCII(key) == 0, "and types nothing");

	keyboard.Handle_Window_Event(Click(true, WINDOW_BUTTON_RIGHT, 5, 6, 2));
	Check(keyboard.Get() == KN_RMOUSE && keyboard.Get() == (KN_RMOUSE | KN_RLSE_BIT), "a double click is a press and a release");

	keyboard.Handle_Window_Event(Click(true, WINDOW_BUTTON_X1, 1, 1));
	Check(keyboard.Check() == 0, "the extra mouse buttons are not queued");

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
