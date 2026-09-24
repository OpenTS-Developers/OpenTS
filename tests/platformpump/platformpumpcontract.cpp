/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins what the game receives from the platform's event pump: each event SDL queues reaches
// the sink once and in order.

#include "platform/platform.h"
#include "platform/windowevent.hh"

#include "win.h"

#include <SDL3/SDL_events.h>

#include <cstdio>
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


std::vector<WindowEvent> Received;


bool Record(WindowEvent const & event)
{
	Received.push_back(event);
	return(true);
}


void Update_Cursor(void)
{
}


void Push_Key(bool down, SDL_Scancode scancode, SDL_Keycode keycode, SDL_Keymod modifiers = SDL_KMOD_NONE)
{
	SDL_Event event = {};
	event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
	event.key.scancode = scancode;
	event.key.key = keycode;
	event.key.mod = modifiers;
	event.key.down = down;
	SDL_PushEvent(&event);
}


// The events of one type the last pump delivered.
std::vector<WindowEvent> Pumped(WindowEventType type)
{
	Received.clear();
	Platform_Pump_Events();

	std::vector<WindowEvent> result;
	for (WindowEvent const & event : Received) {
		if (event.Type == type) {
			result.push_back(event);
		}
	}
	return(result);
}

}


int main(void)
{
	PlatformEventSink sink;
	sink.Handle_Event = Record;
	sink.Update_Cursor = Update_Cursor;

	if (!Platform_Init(sink) || !Platform_Create_Main_Window(true, 64, 48)) {
		std::printf("the main window could not be created\n\nFAILED\n");
		return(1);
	}
	Platform_Pump_Events();

	Push_Key(true, SDL_SCANCODE_A, SDLK_A);
	Push_Key(true, SDL_SCANCODE_B, SDLK_B);
	std::vector<WindowEvent> keys = Pumped(WINDOW_EVENT_KEY_DOWN);
	Check(keys.size() == 2 && keys[0].VirtualKey == 'A' && keys[1].VirtualKey == 'B', "each queued key reaches the sink once, in order");

	Push_Key(false, SDL_SCANCODE_A, SDLK_A);
	Push_Key(false, SDL_SCANCODE_B, SDLK_B);
	Pumped(WINDOW_EVENT_KEY_UP);

	Platform_Shutdown();

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
