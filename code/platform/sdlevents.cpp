/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "platform/sdlevents.h"

#include "platform/sdlkeys.h"
#include "utf8.h"
#include "win.h"

#include <SDL3/SDL_events.h>

#include <cmath>


namespace
{

int Pixel(float coordinate, float pixeldensity)
{
	return((int)std::floor(coordinate * pixeldensity));
}


bool Mouse_Button(Uint8 button, WindowMouseButton & result)
{
	switch (button) {
		case SDL_BUTTON_LEFT:	result = WINDOW_BUTTON_LEFT; return(true);
		case SDL_BUTTON_RIGHT:	result = WINDOW_BUTTON_RIGHT; return(true);
		case SDL_BUTTON_MIDDLE:	result = WINDOW_BUTTON_MIDDLE; return(true);
		case SDL_BUTTON_X1:		result = WINDOW_BUTTON_X1; return(true);
		case SDL_BUTTON_X2:		result = WINDOW_BUTTON_X2; return(true);
		default:				return(false);
	}
}


int Key_Modifiers(SDL_Keymod modifiers, bool altgr)
{
	int result = 0;
	if ((modifiers & SDL_KMOD_SHIFT) != 0) {
		result |= WINDOW_MOD_SHIFT;
	}
	if ((modifiers & SDL_KMOD_CTRL) != 0) {
		result |= WINDOW_MOD_CTRL;
	}
	if ((modifiers & SDL_KMOD_ALT) != 0) {
		result |= WINDOW_MOD_ALT;
	}
	if ((modifiers & SDL_KMOD_CAPS) != 0) {
		result |= WINDOW_MOD_CAPS;
	}
	if ((modifiers & SDL_KMOD_NUM) != 0) {
		result |= WINDOW_MOD_NUM;
	}

	// Windows presses Left Ctrl along with AltGr, which SDL leaves out; hotkeys saved under
	// Windows recorded AltGr as Ctrl+Alt.
	if ((modifiers & SDL_KMOD_RALT) != 0 && altgr) {
		result |= WINDOW_MOD_CTRL;
	}

	return(result);
}


WindowEvent Window_Event(WindowEventType type)
{
	WindowEvent event;
	event.Type = type;
	return(event);
}

}


void Window_Events_From_SDL(SDL_Event const & sdlevent, float pixeldensity, bool altgr, std::vector<WindowEvent> & events)
{
	WindowEvent event;

	switch (sdlevent.type) {
		case SDL_EVENT_MOUSE_MOTION:
			event.Type = WINDOW_EVENT_MOUSE_MOVE;
			event.X = Pixel(sdlevent.motion.x, pixeldensity);
			event.Y = Pixel(sdlevent.motion.y, pixeldensity);
			events.push_back(event);
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (!Mouse_Button(sdlevent.button.button, event.Button)) {
				break;
			}
			event.Type = sdlevent.button.down ? WINDOW_EVENT_MOUSE_DOWN : WINDOW_EVENT_MOUSE_UP;
			event.X = Pixel(sdlevent.button.x, pixeldensity);
			event.Y = Pixel(sdlevent.button.y, pixeldensity);

			// Windows reports every second press of a run of clicks as a double click.
			if (sdlevent.button.down && sdlevent.button.clicks != 0 && sdlevent.button.clicks % 2 == 0) {
				event.Clicks = 2;
			}
			events.push_back(event);
			break;

		case SDL_EVENT_MOUSE_WHEEL:
		{
			float direction = (sdlevent.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) ? -1.0f : 1.0f;
			event.Type = WINDOW_EVENT_MOUSE_WHEEL;
			event.X = Pixel(sdlevent.wheel.mouse_x, pixeldensity);
			event.Y = Pixel(sdlevent.wheel.mouse_y, pixeldensity);
			if (sdlevent.wheel.y != 0.0f) {
				event.Wheel = sdlevent.wheel.y * direction;
				event.Horizontal = false;
				events.push_back(event);
			}
			if (sdlevent.wheel.x != 0.0f) {
				event.Wheel = sdlevent.wheel.x * direction;
				event.Horizontal = true;
				events.push_back(event);
			}
			break;
		}

		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
			event.VirtualKey = Virtual_Key_From_SDL((int)sdlevent.key.scancode, (unsigned int)sdlevent.key.key, (unsigned int)sdlevent.key.mod, (unsigned int)sdlevent.key.raw);
			if (event.VirtualKey == 0) {
				break;
			}
			event.Type = sdlevent.key.down ? WINDOW_EVENT_KEY_DOWN : WINDOW_EVENT_KEY_UP;
			event.Modifiers = Key_Modifiers(sdlevent.key.mod, altgr);
			event.Repeat = sdlevent.key.down && sdlevent.key.repeat;

			// Windows treats keys pressed with Alt, but not with AltGr, as system keys, and F10.
			event.System = ((event.Modifiers & WINDOW_MOD_ALT) != 0 && (event.Modifiers & WINDOW_MOD_CTRL) == 0) || event.VirtualKey == VK_F10;
			events.push_back(event);
			break;

		case SDL_EVENT_TEXT_INPUT:
			if (sdlevent.text.text != NULL) {
				char const * text = sdlevent.text.text;
				event.Type = WINDOW_EVENT_TEXT;
				while (*text != '\0') {
					event.Text = UTF8::Decode(text);
					events.push_back(event);
				}
			}
			break;

		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			events.push_back(Window_Event(WINDOW_EVENT_FOCUS_GAINED));
			break;

		// Windows took the capture away from a window that lost the focus, which the game
		// relies on to end a drag.
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			events.push_back(Window_Event(WINDOW_EVENT_CAPTURE_LOST));
			events.push_back(Window_Event(WINDOW_EVENT_FOCUS_LOST));
			break;

		case SDL_EVENT_WINDOW_EXPOSED:
			events.push_back(Window_Event(WINDOW_EVENT_EXPOSED));
			break;

		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			if (sdlevent.window.data1 > 0 && sdlevent.window.data2 > 0) {
				event.Type = WINDOW_EVENT_RESIZED;
				event.Width = sdlevent.window.data1;
				event.Height = sdlevent.window.data2;
				events.push_back(event);
			}
			break;

		case SDL_EVENT_WINDOW_MOVED:
			events.push_back(Window_Event(WINDOW_EVENT_MOVED));
			break;

		case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
		case SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED:
			events.push_back(Window_Event(WINDOW_EVENT_DISPLAY_CHANGED));
			break;

		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		case SDL_EVENT_QUIT:
			events.push_back(Window_Event(WINDOW_EVENT_CLOSE_REQUESTED));
			break;

		case SDL_EVENT_KEYMAP_CHANGED:
			events.push_back(Window_Event(WINDOW_EVENT_KEYMAP_CHANGED));
			break;

		default:
			break;
	}
}
