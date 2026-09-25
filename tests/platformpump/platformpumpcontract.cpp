/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins what the game receives from the platform's pump: each event once and in order, with
// the keys reported held as of that event, however many events one pump delivers.

#include "gamewindow.h"
#include "platform/platform.h"
#include "platform/windowevent.hh"

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

	// Windows reports the Left Ctrl it presses for AltGr only while the Right Alt press is queued.
	BYTE state[256] = {};
	state[VK_CONTROL] = 0x80;
	state[VK_LCONTROL] = 0x80;
	SetKeyboardState(state);
	Push_Key(true, SDL_SCANCODE_RALT, SDLK_RALT, SDL_KMOD_RALT);
	std::memset(state, 0, sizeof(state));
	SetKeyboardState(state);
	Push_Key(true, SDL_SCANCODE_Q, SDLK_Q, SDL_KMOD_RALT);
	Push_Key(false, SDL_SCANCODE_Q, SDLK_Q, SDL_KMOD_RALT);
	Push_Key(false, SDL_SCANCODE_RALT, SDLK_RALT);
	keys = Pumped(WINDOW_EVENT_KEY_DOWN);
	Check(keys.size() == 2 && keys[1].Ctrl && keys[1].Event.Modifiers == (WINDOW_MOD_CTRL | WINDOW_MOD_ALT) && !keys[1].Event.System, "a key typed with AltGr in one pump carries AltGr's Ctrl");
	Check(!Platform_Key_Down(VK_CONTROL), "and Ctrl is released with AltGr");

	Push_Key(true, SDL_SCANCODE_RALT, SDLK_RALT, SDL_KMOD_RALT);
	Push_Key(true, SDL_SCANCODE_Q, SDLK_Q, SDL_KMOD_RALT);
	keys = Pumped(WINDOW_EVENT_KEY_DOWN);
	Check(keys.size() == 2 && !keys[1].Ctrl && keys[1].Event.Modifiers == WINDOW_MOD_ALT && keys[1].Event.System, "a Right Alt that is not AltGr is Alt alone");
	Push_Key(false, SDL_SCANCODE_Q, SDLK_Q, SDL_KMOD_RALT);
	Push_Key(false, SDL_SCANCODE_RALT, SDLK_RALT);
	Pumped(WINDOW_EVENT_KEY_UP);

	SDL_SetModState(SDL_KMOD_CAPS);
	Push_Window(SDL_EVENT_WINDOW_FOCUS_GAINED);
	Pumped(WINDOW_EVENT_FOCUS_GAINED);
	Check(Platform_Key_Toggled(VK_CAPITAL), "the lock keys are read again when the window regains focus");
	SDL_SetModState(SDL_KMOD_NONE);
	Push_Window(SDL_EVENT_WINDOW_FOCUS_GAINED);
	Pumped(WINDOW_EVENT_FOCUS_GAINED);

	HWND const window = (HWND)Platform_Native_Window().Handle;

	// SDL reports no key still held after it resets the keyboard, and drops such a key's release.
	std::memset(state, 0, sizeof(state));
	state[VK_SHIFT] = 0x80;
	state[VK_LSHIFT] = 0x80;
	SetKeyboardState(state);
	Push_Window(SDL_EVENT_WINDOW_FOCUS_GAINED);
	Pumped(WINDOW_EVENT_FOCUS_GAINED);
	Check(Platform_Key_Down(VK_SHIFT) && Platform_Key_Down(VK_LSHIFT) && !Platform_Key_Down(VK_RSHIFT), "a Shift held as the window gains the focus is held");
	Push_Click();
	clicks = Pumped(WINDOW_EVENT_MOUSE_DOWN);
	Push_Key(true, SDL_SCANCODE_A, SDLK_A);
	keys = Pumped(WINDOW_EVENT_KEY_DOWN);
	Check(clicks.size() == 1 && (clicks[0].Event.Modifiers & WINDOW_MOD_SHIFT) != 0 && keys.size() == 1 && (keys[0].Event.Modifiers & WINDOW_MOD_SHIFT) != 0, "and a click or a key after it carries Shift");
	Push_Key(false, SDL_SCANCODE_A, SDLK_A);
	Pumped(WINDOW_EVENT_KEY_UP);

	std::memset(state, 0, sizeof(state));
	SetKeyboardState(state);
	Push_Click();
	clicks = Pumped(WINDOW_EVENT_MOUSE_DOWN);
	Check(clicks.size() == 1 && (clicks[0].Event.Modifiers & WINDOW_MOD_SHIFT) == 0 && !Platform_Key_Down(VK_SHIFT), "it is released when Windows releases it, though SDL reports no release");

	state[VK_SHIFT] = 0x80;
	state[VK_LSHIFT] = 0x80;
	SetKeyboardState(state);
	Push_Window(SDL_EVENT_WINDOW_FOCUS_GAINED);
	Pumped(WINDOW_EVENT_FOCUS_GAINED);
	Push_Window(SDL_EVENT_WINDOW_FOCUS_LOST);
	Pumped(WINDOW_EVENT_FOCUS_LOST);
	Check(!Platform_Key_Down(VK_SHIFT), "losing the focus releases it");

	Push_Window(SDL_EVENT_WINDOW_FOCUS_GAINED);
	Pumped(WINDOW_EVENT_FOCUS_GAINED);
	Push_Key(true, SDL_SCANCODE_LSHIFT, SDLK_LSHIFT, SDL_KMOD_LSHIFT);
	Pumped(WINDOW_EVENT_KEY_DOWN);
	std::memset(state, 0, sizeof(state));
	SetKeyboardState(state);
	Check(Platform_Key_Down(VK_SHIFT), "once SDL reports the key, Windows no longer releases it");
	Push_Key(false, SDL_SCANCODE_LSHIFT, SDLK_LSHIFT);
	Pumped(WINDOW_EVENT_KEY_UP);
	Check(!Platform_Key_Down(VK_SHIFT), "and SDL's release does");

	SendMessageW(window, WM_ENTERSIZEMOVE, 0, 0);
	state[VK_SHIFT] = 0x80;
	state[VK_LSHIFT] = 0x80;
	SetKeyboardState(state);
	SendMessageW(window, WM_EXITSIZEMOVE, 0, 0);
	Pumped(WINDOW_EVENT_NONE);
	Check(Platform_Key_Down(VK_SHIFT), "a Shift held through a drag of the window is held after it");
	std::memset(state, 0, sizeof(state));
	SetKeyboardState(state);
	Pumped(WINDOW_EVENT_NONE);

	state[VK_CONTROL] = 0x80;
	state[VK_LCONTROL] = 0x80;
	SetKeyboardState(state);
	Push_Window(SDL_EVENT_WINDOW_FOCUS_GAINED);
	Pumped(WINDOW_EVENT_FOCUS_GAINED);
	Push_Key(true, SDL_SCANCODE_RALT, SDLK_RALT, SDL_KMOD_RALT);
	std::memset(state, 0, sizeof(state));
	SetKeyboardState(state);
	Push_Key(true, SDL_SCANCODE_Q, SDLK_Q, SDL_KMOD_RALT);
	keys = Pumped(WINDOW_EVENT_KEY_DOWN);
	Check(keys.size() == 2 && !keys[1].Ctrl && keys[1].Event.Modifiers == WINDOW_MOD_ALT, "a Right Alt pressed with a Left Ctrl held into the window is not AltGr");
	Push_Key(false, SDL_SCANCODE_Q, SDLK_Q, SDL_KMOD_RALT);
	Push_Key(false, SDL_SCANCODE_RALT, SDLK_RALT);
	Pumped(WINDOW_EVENT_KEY_UP);

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
