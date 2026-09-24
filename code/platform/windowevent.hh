/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once


enum WindowEventType
{
	WINDOW_EVENT_NONE,
	WINDOW_EVENT_MOUSE_MOVE,
	WINDOW_EVENT_MOUSE_DOWN,
	WINDOW_EVENT_MOUSE_UP,
	WINDOW_EVENT_MOUSE_WHEEL,
	WINDOW_EVENT_KEY_DOWN,
	WINDOW_EVENT_KEY_UP,
	WINDOW_EVENT_TEXT,
	WINDOW_EVENT_FOCUS_GAINED,
	WINDOW_EVENT_FOCUS_LOST,
	WINDOW_EVENT_CAPTURE_LOST,
	WINDOW_EVENT_KEYMAP_CHANGED,
	WINDOW_EVENT_EXPOSED,
	WINDOW_EVENT_RESIZED,
	WINDOW_EVENT_MOVED,
	WINDOW_EVENT_DISPLAY_CHANGED,
	WINDOW_EVENT_CLOSE_REQUESTED,
};


// The order matches the interface's button slots.
enum WindowMouseButton
{
	WINDOW_BUTTON_LEFT,
	WINDOW_BUTTON_RIGHT,
	WINDOW_BUTTON_MIDDLE,
	WINDOW_BUTTON_X1,
	WINDOW_BUTTON_X2,
};


enum WindowKeyModifier
{
	WINDOW_MOD_SHIFT = 0x01,
	WINDOW_MOD_CTRL = 0x02,
	WINDOW_MOD_ALT = 0x04,
};


// Mouse positions are in window client pixels, before the frame is scaled to the window.
struct WindowEvent
{
	WindowEventType Type = WINDOW_EVENT_NONE;

	int X = 0;
	int Y = 0;
	WindowMouseButton Button = WINDOW_BUTTON_LEFT;

	// Two or more on the press that completes a double click.
	int Clicks = 1;

	// Wheel notches; positive is away from the player, or to the right when Horizontal.
	float Wheel = 0.0f;
	bool Horizontal = false;

	// The Windows virtual-key code, which is the engine's identity for a key.
	int VirtualKey = 0;
	int Modifiers = 0;
	bool Repeat = false;

	// Set for a key pressed with Alt held, or F10, which Windows reports as system keys.
	bool System = false;

	char32_t Text = 0;

	int Width = 0;
	int Height = 0;
};
