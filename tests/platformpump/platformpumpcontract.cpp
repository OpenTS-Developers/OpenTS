/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins what the game receives from the platform's event pump: each event SDL queues reaches
// the game once and in order, and the keys the platform reports held during an event are
// those held as of that event, however many events one pump delivers. It also pins that the
// lock keys are read again when the window regains focus, and that a capture Windows takes
// away reaches the game as a lost capture.

#include "gamewindow.h"
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

// Whether the platform reported Ctrl held while each received event was handled.
std::vector<bool> CtrlHeld;


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


void Push_Window(SDL_EventType type)
{
	SDL_Event event = {};
	event.type = type;
	SDL_PushEvent(&event);
}


void Push_Click(void)
{
	SDL_Event event = {};
	event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
	event.button.button = SDL_BUTTON_LEFT;
	event.button.down = true;
	event.button.clicks = 1;
	SDL_PushEvent(&event);
}


// An event the last pump delivered, and whether Ctrl was reported held while it was handled.
struct Delivered
{
	WindowEvent Event;
	bool Ctrl;
};


// The events of one type the last pump delivered.
std::vector<Delivered> Pumped(WindowEventType type)
{
	Received.clear();
	CtrlHeld.clear();
	Platform_Pump_Events();

	std::vector<Delivered> result;
	for (size_t index = 0; index < Received.size(); index++) {
		if (Received[index].Type == type) {
			result.push_back({ Received[index], CtrlHeld[index] });
		}
	}
	return(result);
}

}


// Stands in for the game's window handler and records what the platform hands it.
bool Game_Window_Handle_Event(WindowEvent const & event)
{
	Received.push_back(event);
	CtrlHeld.push_back(Platform_Key_Down(VK_CONTROL));
	return(true);
}


void Game_Window_Update_Cursor(void)
{
}


int main(void)
{
	if (!Platform_Init() || !Platform_Create_Main_Window(true, 64, 48)) {
		std::printf("the main window could not be created\n\nFAILED\n");
		return(1);
	}
	Platform_Pump_Events();

	Push_Key(true, SDL_SCANCODE_A, SDLK_A);
	Push_Key(true, SDL_SCANCODE_B, SDLK_B);
	std::vector<Delivered> keys = Pumped(WINDOW_EVENT_KEY_DOWN);
	Check(keys.size() == 2 && keys[0].Event.VirtualKey == 'A' && keys[1].Event.VirtualKey == 'B', "each queued key reaches the game once, in order");

	Push_Key(false, SDL_SCANCODE_A, SDLK_A);
	Push_Key(false, SDL_SCANCODE_B, SDLK_B);
	Pumped(WINDOW_EVENT_KEY_UP);

	Check(!Platform_Key_Down(VK_CONTROL), "no key is held before one is pressed");
	Push_Key(true, SDL_SCANCODE_LCTRL, SDLK_LCTRL, SDL_KMOD_LCTRL);
	Push_Key(true, SDL_SCANCODE_A, SDLK_A, SDL_KMOD_LCTRL);
	keys = Pumped(WINDOW_EVENT_KEY_DOWN);
	Check(keys.size() == 2 && keys[1].Ctrl && (keys[1].Event.Modifiers & WINDOW_MOD_CTRL) != 0, "a key pressed after Ctrl in the same pump sees Ctrl held");
	Check(Platform_Key_Down(VK_LCONTROL) && !Platform_Key_Down(VK_RCONTROL), "and the side it was pressed on");

	Push_Click();
	std::vector<Delivered> clicks = Pumped(WINDOW_EVENT_MOUSE_DOWN);
	Check(clicks.size() == 1 && (clicks[0].Event.Modifiers & WINDOW_MOD_CTRL) != 0, "a click while Ctrl is held carries Ctrl");

	Push_Key(false, SDL_SCANCODE_A, SDLK_A, SDL_KMOD_LCTRL);
	Push_Key(false, SDL_SCANCODE_LCTRL, SDLK_LCTRL);
	Push_Key(true, SDL_SCANCODE_B, SDLK_B);
	Push_Click();
	Received.clear();
	CtrlHeld.clear();
	Platform_Pump_Events();
	bool released = false;
	for (size_t index = 0; index < Received.size(); index++) {
		if (Received[index].Type == WINDOW_EVENT_KEY_DOWN && Received[index].VirtualKey == 'B') {
			released = !CtrlHeld[index] && (Received[index].Modifiers & WINDOW_MOD_CTRL) == 0;
		}
		if (Received[index].Type == WINDOW_EVENT_MOUSE_DOWN) {
			released = released && (Received[index].Modifiers & WINDOW_MOD_CTRL) == 0;
		}
	}
	Check(released, "a key or click after Ctrl's release in the same pump sees it released");

	Push_Key(false, SDL_SCANCODE_B, SDLK_B);
	Pumped(WINDOW_EVENT_KEY_UP);

	SDL_SetModState(SDL_KMOD_CAPS);
	Push_Window(SDL_EVENT_WINDOW_FOCUS_GAINED);
	Pumped(WINDOW_EVENT_FOCUS_GAINED);
	Check(Platform_Key_Toggled(VK_CAPITAL), "the lock keys are read again when the window regains focus");
	SDL_SetModState(SDL_KMOD_NONE);
	Push_Window(SDL_EVENT_WINDOW_FOCUS_GAINED);
	Pumped(WINDOW_EVENT_FOCUS_GAINED);

	HWND const window = (HWND)Platform_Native_Window().Handle;

	Platform_Capture_Mouse(true);
	bool const captured = Platform_Mouse_Captured() && GetCapture() == window;
	Check(captured, "the window can take the mouse capture");
	if (captured) {
		SendMessageW(window, WM_CANCELMODE, 0, 0);
		Check(!Platform_Mouse_Captured(), "a capture Windows cancels is reported gone at once");
		Check(Pumped(WINDOW_EVENT_CAPTURE_LOST).size() == 1, "and reaches the game as one lost capture");

		Platform_Capture_Mouse(true);
		Check(Platform_Mouse_Captured() && GetCapture() == window, "the capture can be taken again after it was cancelled");

		Platform_Capture_Mouse(false);
		Check(!Platform_Mouse_Captured() && Pumped(WINDOW_EVENT_CAPTURE_LOST).empty(), "releasing it is no lost capture");

		HWND other = CreateWindowExW(0, L"STATIC", L"", WS_POPUP, 0, 0, 8, 8, NULL, NULL, GetModuleHandleW(NULL), NULL);
		Platform_Capture_Mouse(true);
		SetCapture(other);
		Check(!Platform_Mouse_Captured() && Pumped(WINDOW_EVENT_CAPTURE_LOST).size() == 1, "another window taking the capture is one lost capture");
		ReleaseCapture();
		DestroyWindow(other);
		Platform_Capture_Mouse(false);
		Pumped(WINDOW_EVENT_CAPTURE_LOST);
	}

	Platform_Shutdown();

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
