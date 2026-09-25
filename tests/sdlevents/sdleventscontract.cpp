/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins how SDL events become the window events the game reads.

#include "sdl/sdlevents.h"

#include "win.h"

#include <SDL3/SDL_events.h>

#include <cstdio>
#include <cstring>
#include <vector>

namespace {

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-76s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


SDL_Event Event_Of(Uint32 type)
{
	SDL_Event event;
	std::memset(&event, 0, sizeof(event));
	event.type = type;
	return(event);
}


std::vector<WindowEvent> Translate(SDL_Event const & event, float pixeldensity = 1.0f, int modifiers = 0)
{
	std::vector<WindowEvent> events;
	Window_Events_From_SDL(event, pixeldensity, modifiers, events);
	return(events);
}


bool One(std::vector<WindowEvent> const & events, WindowEventType type)
{
	return(events.size() == 1 && events[0].Type == type);
}


SDL_Event Button(bool down, Uint8 button, Uint8 clicks, float x, float y)
{
	SDL_Event event = Event_Of(down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP);
	event.button.down = down;
	event.button.button = button;
	event.button.clicks = clicks;
	event.button.x = x;
	event.button.y = y;
	return(event);
}


SDL_Event Key(bool down, SDL_Scancode scancode, SDL_Keycode keycode, SDL_Keymod modifiers, bool repeat = false)
{
	SDL_Event event = Event_Of(down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP);
	event.key.down = down;
	event.key.scancode = scancode;
	event.key.key = keycode;
	event.key.mod = modifiers;
	event.key.repeat = repeat;
	return(event);
}

}


int main(void)
{
	SDL_Event motion = Event_Of(SDL_EVENT_MOUSE_MOTION);
	motion.motion.x = 12.75f;
	motion.motion.y = 34.0f;
	std::vector<WindowEvent> events = Translate(motion);
	Check(One(events, WINDOW_EVENT_MOUSE_MOVE) && events[0].X == 12 && events[0].Y == 34, "a mouse move lands on the pixel it is over");

	events = Translate(motion, 2.0f);
	Check(One(events, WINDOW_EVENT_MOUSE_MOVE) && events[0].X == 25 && events[0].Y == 68, "a window with a pixel density of two doubles the position");

	motion.motion.x = -0.5f;
	events = Translate(motion);
	Check(One(events, WINDOW_EVENT_MOUSE_MOVE) && events[0].X == -1, "a position left of the client area stays outside it");

	events = Translate(Button(true, SDL_BUTTON_LEFT, 1, 5.0f, 6.0f));
	Check(One(events, WINDOW_EVENT_MOUSE_DOWN) && events[0].Button == WINDOW_BUTTON_LEFT && events[0].Clicks == 1 && events[0].X == 5 && events[0].Y == 6, "a left press is a single click where it happened");

	events = Translate(Button(true, SDL_BUTTON_LEFT, 1, 5.0f, 6.0f), 1.0f, WINDOW_MOD_SHIFT);
	Check(One(events, WINDOW_EVENT_MOUSE_DOWN) && events[0].Modifiers == WINDOW_MOD_SHIFT, "and carries the modifiers held");

	events = Translate(Button(true, SDL_BUTTON_RIGHT, 2, 0.0f, 0.0f));
	Check(One(events, WINDOW_EVENT_MOUSE_DOWN) && events[0].Button == WINDOW_BUTTON_RIGHT && events[0].Clicks == 2, "the second press of a run of clicks is a double click");

	events = Translate(Button(true, SDL_BUTTON_MIDDLE, 3, 0.0f, 0.0f));
	Check(One(events, WINDOW_EVENT_MOUSE_DOWN) && events[0].Clicks == 1, "the third press starts over, as Windows counts them");

	events = Translate(Button(true, SDL_BUTTON_X2, 4, 0.0f, 0.0f));
	Check(One(events, WINDOW_EVENT_MOUSE_DOWN) && events[0].Button == WINDOW_BUTTON_X2 && events[0].Clicks == 2, "and the fourth is double again, on the second extra button too");

	events = Translate(Button(false, SDL_BUTTON_LEFT, 2, 0.0f, 0.0f));
	Check(One(events, WINDOW_EVENT_MOUSE_UP) && events[0].Clicks == 1, "a release is never a double click");

	events = Translate(Button(true, 9, 1, 0.0f, 0.0f));
	Check(events.empty(), "a button beyond the fifth is dropped");

	SDL_Event wheel = Event_Of(SDL_EVENT_MOUSE_WHEEL);
	wheel.wheel.y = -2.0f;
	wheel.wheel.mouse_x = 30.0f;
	wheel.wheel.mouse_y = 40.0f;
	events = Translate(wheel);
	Check(One(events, WINDOW_EVENT_MOUSE_WHEEL) && events[0].Wheel == -2.0f && !events[0].Horizontal && events[0].X == 30 && events[0].Y == 40, "two notches toward the player are -2 on the vertical wheel, where the mouse is");

	wheel.wheel.direction = SDL_MOUSEWHEEL_FLIPPED;
	events = Translate(wheel);
	Check(One(events, WINDOW_EVENT_MOUSE_WHEEL) && events[0].Wheel == 2.0f, "a flipped wheel is turned back the way the hand moved");

	wheel.wheel.direction = SDL_MOUSEWHEEL_NORMAL;
	wheel.wheel.x = 1.0f;
	wheel.wheel.y = 0.5f;
	events = Translate(wheel);
	Check(events.size() == 2 && !events[0].Horizontal && events[0].Wheel == 0.5f && events[1].Horizontal && events[1].Wheel == 1.0f, "a diagonal turn is a vertical turn and then a horizontal one");

	events = Translate(Key(true, SDL_SCANCODE_A, SDLK_A, (SDL_Keymod)(SDL_KMOD_LSHIFT | SDL_KMOD_RCTRL)), 1.0f, WINDOW_MOD_SHIFT | WINDOW_MOD_CTRL);
	Check(One(events, WINDOW_EVENT_KEY_DOWN) && events[0].VirtualKey == 'A' && events[0].Modifiers == (WINDOW_MOD_SHIFT | WINDOW_MOD_CTRL), "a key press carries its virtual key and the modifiers held");
	Check(events.size() == 1 && !events[0].Repeat && !events[0].System, "and is neither a repeat nor a system key");

	events = Translate(Key(true, SDL_SCANCODE_A, SDLK_A, SDL_KMOD_NONE, true));
	Check(One(events, WINDOW_EVENT_KEY_DOWN) && events[0].Repeat, "a held key's repeat is marked as one");

	events = Translate(Key(false, SDL_SCANCODE_A, SDLK_A, SDL_KMOD_NONE, true));
	Check(One(events, WINDOW_EVENT_KEY_UP) && !events[0].Repeat, "a release is never a repeat");

	events = Translate(Key(true, SDL_SCANCODE_F4, SDLK_F4, SDL_KMOD_LALT), 1.0f, WINDOW_MOD_ALT);
	Check(One(events, WINDOW_EVENT_KEY_DOWN) && events[0].System && events[0].Modifiers == WINDOW_MOD_ALT, "a key pressed with Alt held is a system key");

	events = Translate(Key(true, SDL_SCANCODE_F10, SDLK_F10, SDL_KMOD_NONE));
	Check(One(events, WINDOW_EVENT_KEY_DOWN) && events[0].System, "F10 is a system key on its own");

	events = Translate(Key(true, SDL_SCANCODE_Q, SDLK_Q, (SDL_Keymod)(SDL_KMOD_LALT | SDL_KMOD_LCTRL)), 1.0f, WINDOW_MOD_CTRL | WINDOW_MOD_ALT);
	Check(One(events, WINDOW_EVENT_KEY_DOWN) && !events[0].System, "a key pressed with Ctrl and Alt held is not a system key");

	events = Translate(Key(true, SDL_SCANCODE_LANG1, SDLK_UNKNOWN, SDL_KMOD_NONE));
	Check(events.empty(), "a key Windows has no code for is dropped");

	SDL_Event text = Event_Of(SDL_EVENT_TEXT_INPUT);
	text.text.text = "a\xD0\x9F\xF0\x9F\x98\x80";
	events = Translate(text);
	Check(events.size() == 3 && events[0].Type == WINDOW_EVENT_TEXT && events[0].Text == 'a' && events[1].Text == 0x041F && events[2].Text == 0x1F600, "typed text arrives one character at a time");

	text.text.text = "";
	Check(Translate(text).empty(), "empty text is no character");

	events = Translate(Event_Of(SDL_EVENT_WINDOW_FOCUS_LOST));
	Check(events.size() == 2 && events[0].Type == WINDOW_EVENT_CAPTURE_LOST && events[1].Type == WINDOW_EVENT_FOCUS_LOST, "losing the focus loses the capture first");
	Check(One(Translate(Event_Of(SDL_EVENT_WINDOW_FOCUS_GAINED)), WINDOW_EVENT_FOCUS_GAINED), "gaining the focus is a gain of focus");

	SDL_Event resize = Event_Of(SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED);
	resize.window.data1 = 800;
	resize.window.data2 = 600;
	events = Translate(resize);
	Check(One(events, WINDOW_EVENT_RESIZED) && events[0].Width == 800 && events[0].Height == 600, "a new pixel size is a resize to that size");
	resize.window.data2 = 0;
	Check(Translate(resize).empty(), "a size with no area is not a resize");
	Check(Translate(Event_Of(SDL_EVENT_WINDOW_RESIZED)).empty(), "a size in window coordinates is left to the pixel size");

	Check(One(Translate(Event_Of(SDL_EVENT_WINDOW_EXPOSED)), WINDOW_EVENT_EXPOSED), "an exposed window is exposed");
	Check(One(Translate(Event_Of(SDL_EVENT_WINDOW_MOVED)), WINDOW_EVENT_MOVED), "a moved window is moved");
	Check(One(Translate(Event_Of(SDL_EVENT_WINDOW_DISPLAY_CHANGED)), WINDOW_EVENT_DISPLAY_CHANGED), "a window moving to another display changes the display");
	Check(One(Translate(Event_Of(SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED)), WINDOW_EVENT_DISPLAY_CHANGED), "a display changing mode changes the display");
	Check(One(Translate(Event_Of(SDL_EVENT_WINDOW_CLOSE_REQUESTED)), WINDOW_EVENT_CLOSE_REQUESTED), "the close button asks to close");
	Check(One(Translate(Event_Of(SDL_EVENT_QUIT)), WINDOW_EVENT_CLOSE_REQUESTED), "a request to quit asks to close");
	Check(Translate(Event_Of(SDL_EVENT_WINDOW_MOUSE_ENTER)).empty(), "an event the game does not use produces nothing");

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
