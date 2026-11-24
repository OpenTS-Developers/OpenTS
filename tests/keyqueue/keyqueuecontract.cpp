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
#include "windowevent.hh"

#include "doctest/doctest.h"

namespace {

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


TEST_CASE("a key press is queued with the modifiers held and the character it typed, then its release")
{
	WWKeyboardClass keyboard;
	keyboard.Handle_Window_Event(Key(true, 'A', WINDOW_MOD_SHIFT));
	keyboard.Handle_Window_Event(Text('A'));
	keyboard.Handle_Window_Event(Key(false, 'A', WINDOW_MOD_SHIFT));

	unsigned short key = keyboard.Get();
	CHECK(key == ('A' | WWKEY_SHIFT_BIT));
	CHECK(keyboard.To_ASCII(key) == 'A');

	key = keyboard.Get();
	CHECK(key == ('A' | WWKEY_SHIFT_BIT | WWKEY_RLS_BIT));
	CHECK_MESSAGE(keyboard.To_ASCII(key) == 0, "the release types nothing");
	CHECK_MESSAGE(keyboard.Check() == 0, "nothing else was queued for the character");
}


TEST_CASE("an accented character is read back from its key")
{
	WWKeyboardClass keyboard;
	keyboard.Handle_Window_Event(Key(true, 'E'));
	keyboard.Handle_Window_Event(Text(0x00E9));

	unsigned short key = keyboard.Get();
	CHECK(key == 'E');
	CHECK(keyboard.To_ASCII(key) == 0x00E9);
}


TEST_CASE("a held key's repeat neither queues nor types, and text after its release is kept")
{
	WWKeyboardClass keyboard;
	keyboard.Handle_Window_Event(Key(true, 'Q'));
	keyboard.Handle_Window_Event(Key(true, 'Q', 0, true));
	keyboard.Handle_Window_Event(Text('q'));

	unsigned short key = keyboard.Get();
	CHECK(key == 'Q');
	CHECK(keyboard.To_ASCII(key) == 0);
	CHECK_MESSAGE(keyboard.Check() == 0, "the repeat's character is dropped");

	keyboard.Handle_Window_Event(Key(false, 'Q'));
	keyboard.Get();
	keyboard.Handle_Window_Event(Text(0x65E5));
	key = (keyboard.Check() != 0) ? keyboard.Get() : 0;
	CHECK(key == KN_TEXT);
	CHECK(keyboard.To_ASCII(key) == 0x65E5);
}


TEST_CASE("a dead key types nothing and the next key carries the first character the two typed")
{
	WWKeyboardClass keyboard;
	keyboard.Handle_Window_Event(Key(true, VK_OEM_7));
	keyboard.Handle_Window_Event(Key(true, 'X'));
	keyboard.Handle_Window_Event(Text(0x00B4));
	keyboard.Handle_Window_Event(Text('x'));

	unsigned short key = keyboard.Get();
	CHECK(key == VK_OEM_7);
	CHECK(keyboard.To_ASCII(key) == 0);

	key = keyboard.Get();
	CHECK(key == 'X');
	CHECK(keyboard.To_ASCII(key) == 0x00B4);

	key = keyboard.Get();
	CHECK_MESSAGE(key == KN_TEXT, "a second character arrives as a text entry of its own");
	CHECK(keyboard.To_ASCII(key) == 'x');
}


TEST_CASE("text with no key press, as an input method sends it, is a text entry")
{
	WWKeyboardClass keyboard;
	keyboard.Handle_Window_Event(Text(0x65E5));

	unsigned short key = keyboard.Get();
	CHECK(key == KN_TEXT);
	CHECK(keyboard.To_ASCII(key) == 0x65E5);
}


TEST_CASE("clearing the queue forgets the press a character would have joined")
{
	WWKeyboardClass keyboard;
	keyboard.Handle_Window_Event(Key(true, 'Z'));
	keyboard.Clear();
	keyboard.Handle_Window_Event(Text('z'));

	unsigned short key = keyboard.Get();
	CHECK(key == KN_TEXT);
	CHECK(keyboard.To_ASCII(key) == 'z');
}


TEST_CASE("Esc gives its control character")
{
	WWKeyboardClass keyboard;
	keyboard.Handle_Window_Event(Key(true, VK_ESCAPE));
	CHECK(keyboard.To_ASCII(keyboard.Get()) == 0x1B);
}


TEST_CASE("Enter gives a carriage return with any modifier")
{
	WWKeyboardClass keyboard;
	keyboard.Handle_Window_Event(Key(true, VK_RETURN, WINDOW_MOD_SHIFT));
	CHECK(keyboard.To_ASCII(keyboard.Get()) == 0x0D);
}


TEST_CASE("Backspace gives a backspace")
{
	WWKeyboardClass keyboard;
	keyboard.Handle_Window_Event(Key(true, VK_BACK));
	CHECK(keyboard.To_ASCII(keyboard.Get()) == 0x08);
}


TEST_CASE("Tab gives a tab")
{
	WWKeyboardClass keyboard;
	keyboard.Handle_Window_Event(Key(true, VK_TAB));
	CHECK(keyboard.To_ASCII(keyboard.Get()) == 0x09);
}


TEST_CASE("text arriving after Enter leaves Enter its carriage return and arrives as a text entry")
{
	WWKeyboardClass keyboard;
	keyboard.Handle_Window_Event(Key(true, VK_RETURN));
	keyboard.Handle_Window_Event(Text(0x65E5));

	unsigned short key = keyboard.Get();
	CHECK(keyboard.To_ASCII(key) == 0x0D);

	key = keyboard.Get();
	CHECK(key == KN_TEXT);
	CHECK(keyboard.To_ASCII(key) == 0x65E5);
}


TEST_CASE("only the key fetched last reads back a character")
{
	WWKeyboardClass keyboard;
	keyboard.Handle_Window_Event(Key(true, 'B'));
	keyboard.Handle_Window_Event(Text('b'));
	keyboard.Handle_Window_Event(Key(true, 'C'));
	keyboard.Handle_Window_Event(Text('c'));

	unsigned short first = keyboard.Get();
	unsigned short second = keyboard.Get();
	CHECK(keyboard.To_ASCII(second) == 'c');
	CHECK(keyboard.To_ASCII(first) == 0);
}


TEST_CASE("Ctrl and Alt become their modifier bits")
{
	WWKeyboardClass keyboard;
	keyboard.Handle_Window_Event(Key(true, 'M', WINDOW_MOD_CTRL | WINDOW_MOD_ALT));
	CHECK(keyboard.Get() == ('M' | WWKEY_CTRL_BIT | WWKEY_ALT_BIT));
}


TEST_CASE("a click carries where it happened and types nothing")
{
	WWKeyboardClass keyboard;
	keyboard.Handle_Window_Event(Click(true, WINDOW_BUTTON_LEFT, 30, 40));

	unsigned short key = keyboard.Get();
	CHECK(key == KN_LMOUSE);
	CHECK(keyboard.MouseQX == 30);
	CHECK(keyboard.MouseQY == 40);
	CHECK(keyboard.To_ASCII(key) == 0);
}


TEST_CASE("a double click's second press waits for its own release")
{
	WWKeyboardClass keyboard;
	keyboard.Handle_Window_Event(Click(true, WINDOW_BUTTON_RIGHT, 5, 6, 2));
	CHECK(keyboard.Get() == KN_RMOUSE);
	CHECK(keyboard.Check() == 0);

	keyboard.Handle_Window_Event(Click(false, WINDOW_BUTTON_RIGHT, 5, 6, 2));
	CHECK(keyboard.Get() == (KN_RMOUSE | KN_RLSE_BIT));
}


TEST_CASE("the extra mouse buttons are not queued")
{
	WWKeyboardClass keyboard;
	keyboard.Handle_Window_Event(Click(true, WINDOW_BUTTON_X1, 1, 1));
	CHECK(keyboard.Check() == 0);
}
