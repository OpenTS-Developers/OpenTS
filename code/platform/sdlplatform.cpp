/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "platform/platform.h"

#include "dbgprint.h"
#include "gamewindow.h"
#include "platform/sdlevents.h"
#include "platform/sdlkeys.h"
#include "resource.h"
#include "win.h"

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>


namespace
{

bool _Started = false;
SDL_Window * _Window = nullptr;
int _NativeModals = 0;
bool _Watching = false;

bool _HeldKeys[256];
bool _HeldKeysStale = true;

SDL_Cursor * _SystemCursors[PLATFORM_CURSOR_COUNT];


void Set_Hint(char const * name, char const * value)
{
	if (!SDL_SetHint(name, value)) {
		DebugString("SDL: hint %s was not set: %s\n", name, SDL_GetError());
	}
}


void Set_Hint(char const * name, int value)
{
	char text[16];
	std::snprintf(text, sizeof(text), "%d", value);
	Set_Hint(name, text);
}


// Every hint is set, even where it repeats SDL's default, because the defaults change between
// SDL releases and each of these keeps a behavior the game had before SDL owned the window.
void Set_Hints(void)
{
	Set_Hint(SDL_HINT_QUIT_ON_LAST_WINDOW_CLOSE, "0");
	Set_Hint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
	Set_Hint(SDL_HINT_WINDOWS_CLOSE_ON_ALT_F4, "1");
	Set_Hint(SDL_HINT_WINDOWS_ENABLE_MENU_MNEMONICS, "0");
	Set_Hint(SDL_HINT_WINDOWS_RAW_KEYBOARD, "0");
	Set_Hint(SDL_HINT_WINDOWS_GAMEINPUT, "0");
	Set_Hint(SDL_HINT_WINDOWS_ERASE_BACKGROUND_MODE, "0");
	Set_Hint(SDL_HINT_WINDOWS_INTRESOURCE_ICON, IDI_SUN);
	Set_Hint(SDL_HINT_WINDOWS_INTRESOURCE_ICON_SMALL, IDI_SUN);
	Set_Hint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
	Set_Hint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");
	Set_Hint(SDL_HINT_WINDOW_ALLOW_TOPMOST, "0");
	Set_Hint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
	Set_Hint(SDL_HINT_MOUSE_AUTO_CAPTURE, "1");
	Set_Hint(SDL_HINT_MOUSE_EMULATE_WARP_WITH_RELATIVE, "0");
	Set_Hint(SDL_HINT_MOUSE_DOUBLE_CLICK_TIME, (int)GetDoubleClickTime());
	Set_Hint(SDL_HINT_MOUSE_DOUBLE_CLICK_RADIUS, GetSystemMetrics(SM_CXDOUBLECLK) / 2);
	Set_Hint(SDL_HINT_KEYCODE_OPTIONS, "french_numbers,latin_letters");
}


float Pixel_Density(void)
{
	float density = (_Window != nullptr) ? SDL_GetWindowPixelDensity(_Window) : 1.0f;
	return(density > 0.0f ? density : 1.0f);
}


void Dispatch(WindowEvent const & event)
{
	// The screen saver may start while the player is elsewhere, but not over the game.
	if (event.Type == WINDOW_EVENT_FOCUS_GAINED) {
		SDL_DisableScreenSaver();
	} else if (event.Type == WINDOW_EVENT_FOCUS_LOST) {
		SDL_EnableScreenSaver();
	}

	Game_Window_Handle_Event(event);

	// Windows asked for the pointer on every mouse move; SDL leaves choosing it to the game.
	if (event.Type == WINDOW_EVENT_MOUSE_MOVE || event.Type == WINDOW_EVENT_FOCUS_GAINED) {
		Game_Window_Update_Cursor();
	}
}


void Handle_SDL_Event(SDL_Event const & sdlevent)
{
	if (sdlevent.type == SDL_EVENT_QUIT) {
		DebugString("SDL: the system asked the game to quit\n");
	}

	std::vector<WindowEvent> events;
	Window_Events_From_SDL(sdlevent, Pixel_Density(), events);
	for (WindowEvent const & event : events) {
		Dispatch(event);
	}
}


bool Watched(Uint32 type)
{
	return(type == SDL_EVENT_WINDOW_EXPOSED || type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED);
}


// Repaints and resizes are handled as SDL reports them, so the picture keeps up while
// Windows runs its own loop for a window being dragged or sized.
bool SDLCALL Watch_Window(void *, SDL_Event * sdlevent)
{
	if (!Watched(sdlevent->type) || _Watching) {
		return(true);
	}

	_Watching = true;
	Handle_SDL_Event(*sdlevent);
	_Watching = false;
	return(true);
}


void Refresh_Held_Keys(void)
{
	if (!_HeldKeysStale) {
		return;
	}
	_HeldKeysStale = false;

	std::memset(_HeldKeys, 0, sizeof(_HeldKeys));

	int count = 0;
	bool const * state = SDL_GetKeyboardState(&count);
	SDL_Keymod const modifiers = SDL_GetModState();
	for (int scancode = 0; scancode < count; scancode++) {
		if (!state[scancode]) {
			continue;
		}

		SDL_Keycode keycode = SDL_GetKeyFromScancode((SDL_Scancode)scancode, modifiers, true);
		int key = Virtual_Key_From_SDL(scancode, (unsigned int)keycode, (unsigned int)modifiers);
		if (key > 0 && key < 256) {
			_HeldKeys[key] = true;
		}

		// Windows also answers for each side of a modifier.
		switch (scancode) {
			case SDL_SCANCODE_LSHIFT:	_HeldKeys[VK_LSHIFT] = true; break;
			case SDL_SCANCODE_RSHIFT:	_HeldKeys[VK_RSHIFT] = true; break;
			case SDL_SCANCODE_LCTRL:	_HeldKeys[VK_LCONTROL] = true; break;
			case SDL_SCANCODE_RCTRL:	_HeldKeys[VK_RCONTROL] = true; break;
			case SDL_SCANCODE_LALT:		_HeldKeys[VK_LMENU] = true; break;
			case SDL_SCANCODE_RALT:		_HeldKeys[VK_RMENU] = true; break;
			default:					break;
		}
	}
}


// The window's class cursor, a monochrome arrow among the game's resources, is the pointer
// shown wherever neither the game nor the interface chooses one.
SDL_Cursor * Resource_Arrow(void)
{
	HCURSOR handle = LoadCursorW(ProgramInstance, MAKEINTRESOURCEW(IDC_CURSOR1));
	ICONINFO info;
	if (handle == NULL || !GetIconInfo(handle, &info)) {
		return(nullptr);
	}

	SDL_Cursor * cursor = nullptr;
	BITMAP bitmap;
	if (info.hbmColor == NULL && GetObject(info.hbmMask, sizeof(bitmap), &bitmap) != 0 && bitmap.bmWidth % 8 == 0 && bitmap.bmHeight >= 2) {

		// A monochrome cursor stacks its AND mask above its XOR mask.
		int const width = bitmap.bmWidth;
		int const height = bitmap.bmHeight / 2;
		int const pitch = ((width + 31) / 32) * 4;

		struct
		{
			BITMAPINFOHEADER Header;
			RGBQUAD Colors[2];
		} layout = {};
		layout.Header.biSize = sizeof(BITMAPINFOHEADER);
		layout.Header.biWidth = width;
		layout.Header.biHeight = -(height * 2);
		layout.Header.biPlanes = 1;
		layout.Header.biBitCount = 1;
		layout.Header.biCompression = BI_RGB;

		std::vector<unsigned char> bits((size_t)pitch * height * 2);
		HDC dc = GetDC(NULL);
		int const rows = GetDIBits(dc, info.hbmMask, 0, height * 2, bits.data(), (BITMAPINFO *)&layout, DIB_RGB_COLORS);
		ReleaseDC(NULL, dc);

		if (rows == height * 2) {
			int const stride = width / 8;
			std::vector<Uint8> data((size_t)stride * height);
			std::vector<Uint8> mask((size_t)stride * height);
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < stride; x++) {
					unsigned char const andbits = bits[(size_t)y * pitch + x];
					unsigned char const xorbits = bits[(size_t)(y + height) * pitch + x];
					data[(size_t)y * stride + x] = (Uint8)~(andbits ^ xorbits);
					mask[(size_t)y * stride + x] = (Uint8)~andbits;
				}
			}
			cursor = SDL_CreateCursor(data.data(), mask.data(), width, height, (int)info.xHotspot, (int)info.yHotspot);
		}
	}

	DeleteObject(info.hbmMask);
	if (info.hbmColor != NULL) {
		DeleteObject(info.hbmColor);
	}
	return(cursor);
}


SDL_SystemCursor System_Cursor_Of(PlatformCursorShape shape)
{
	switch (shape) {
		case PLATFORM_CURSOR_TEXT:			return(SDL_SYSTEM_CURSOR_TEXT);
		case PLATFORM_CURSOR_HAND:			return(SDL_SYSTEM_CURSOR_POINTER);
		case PLATFORM_CURSOR_RESIZE_NS:		return(SDL_SYSTEM_CURSOR_NS_RESIZE);
		case PLATFORM_CURSOR_RESIZE_EW:		return(SDL_SYSTEM_CURSOR_EW_RESIZE);
		case PLATFORM_CURSOR_RESIZE_NESW:	return(SDL_SYSTEM_CURSOR_NESW_RESIZE);
		case PLATFORM_CURSOR_RESIZE_NWSE:	return(SDL_SYSTEM_CURSOR_NWSE_RESIZE);
		case PLATFORM_CURSOR_MOVE:			return(SDL_SYSTEM_CURSOR_MOVE);
		case PLATFORM_CURSOR_UNAVAILABLE:	return(SDL_SYSTEM_CURSOR_NOT_ALLOWED);
		default:							return(SDL_SYSTEM_CURSOR_DEFAULT);
	}
}

}


/// <summary>
/// Starts SDL's video subsystem. It must run before the main window is created, and once the
/// settings that decide how the window looks have been read.
/// </summary>
/// <returns>False when SDL cannot start, which leaves the game without a window.</returns>
bool Platform_Init(void)
{
	if (_Started) {
		return(true);
	}

	Set_Hints();

	// The window class keeps the game's own name, which tools looking for the window use.
	if (!SDL_RegisterApp("Tiberian Sun", 0, ProgramInstance)) {
		DebugString("SDL: the window class was not registered: %s\n", SDL_GetError());
	}
	SDL_SetMainReady();

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		DebugString("SDL: video did not start: %s\n", SDL_GetError());
		SDL_UnregisterApp();
		return(false);
	}

	_Started = true;
	return(true);
}


/// <summary>
/// Destroys the main window and stops SDL. Calling it again, or before Platform_Init, does
/// nothing. The renderer must have let go of the window first.
/// </summary>
void Platform_Shutdown(void)
{
	if (!_Started) {
		return;
	}

	for (SDL_Cursor * & cursor : _SystemCursors) {
		if (cursor != nullptr) {
			SDL_DestroyCursor(cursor);
			cursor = nullptr;
		}
	}

	if (_Window != nullptr) {
		SDL_RemoveEventWatch(Watch_Window, nullptr);
		SDL_DestroyWindow(_Window);
		_Window = nullptr;
	}

	SDL_Quit();
	SDL_UnregisterApp();
	_Started = false;
}


/// <summary>
/// Creates and shows the main window. A window for windowed play has a client area of the
/// size given and is centered on the primary display, kept clear of its top and left edges;
/// otherwise the window covers the display without changing its mode.
/// </summary>
/// <returns>False when the window could not be created.</returns>
bool Platform_Create_Main_Window(bool windowed, int width, int height)
{
	if (!_Started || _Window != nullptr) {
		return(false);
	}

	SDL_WindowFlags flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	flags |= windowed ? SDL_WINDOW_RESIZABLE : SDL_WINDOW_FULLSCREEN;

	_Window = SDL_CreateWindow("Tiberian Sun", width, height, flags);
	if (_Window == nullptr) {
		DebugString("SDL: the window was not created: %s\n", SDL_GetError());
		return(false);
	}

	if (windowed) {
		float const density = Pixel_Density();
		int const clientwidth = (int)std::lround(width / density);
		int const clientheight = (int)std::lround(height / density);
		SDL_SetWindowSize(_Window, clientwidth, clientheight);

		int top = 0;
		int left = 0;
		int bottom = 0;
		int right = 0;
		SDL_GetWindowBordersSize(_Window, &top, &left, &bottom, &right);

		SDL_Rect display;
		if (SDL_GetDisplayBounds(SDL_GetPrimaryDisplay(), &display)) {
			int const outerwidth = clientwidth + left + right;
			int const outerheight = clientheight + top + bottom;
			int const x = display.x + SDL_max((display.w - outerwidth) / 2, 0) + left;
			int const y = display.y + SDL_max((display.h - outerheight) / 2, 0) + top;
			SDL_SetWindowPosition(_Window, x, y);
		}
	}

	SDL_AddEventWatch(Watch_Window, nullptr);

	SDL_ShowWindow(_Window);
	SDL_RaiseWindow(_Window);

	// Text arrives as it is typed for as long as the window exists, as it did as characters
	// from Windows.
	SDL_StartTextInput(_Window);

	return(true);
}


NativeWindow Platform_Native_Window(void)
{
	NativeWindow window = { NATIVE_WINDOW_DEFAULT, nullptr, nullptr };
	if (_Window != nullptr) {
		window.Handle = SDL_GetPointerProperty(SDL_GetWindowProperties(_Window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
	}
	return(window);
}


bool Platform_Window_Drawable_Size(int & width, int & height)
{
	if (_Window == nullptr || !SDL_GetWindowSizeInPixels(_Window, &width, &height)) {
		return(false);
	}
	return(width > 0 && height > 0);
}


bool Platform_Window_Minimized(void)
{
	return(_Window != nullptr && (SDL_GetWindowFlags(_Window) & SDL_WINDOW_MINIMIZED) != 0);
}


bool Platform_Window_Client_Rect(int & x, int & y, int & width, int & height)
{
	if (_Window == nullptr || !SDL_GetWindowPosition(_Window, &x, &y)) {
		return(false);
	}

	float const density = Pixel_Density();
	x = (int)std::floor((float)x * density);
	y = (int)std::floor((float)y * density);
	return(Platform_Window_Drawable_Size(width, height));
}


// A window grown about its middle can be pushed past the edges of the display, and a title
// bar above its top edge cannot be grabbed to bring the window back.
void Platform_Resize_Window(int width, int height)
{
	if (_Window == nullptr) {
		return;
	}

	float const density = Pixel_Density();
	int const newwidth = (int)std::lround(width / density);
	int const newheight = (int)std::lround(height / density);

	int x = 0;
	int y = 0;
	int oldwidth = 0;
	int oldheight = 0;
	SDL_GetWindowPosition(_Window, &x, &y);
	SDL_GetWindowSize(_Window, &oldwidth, &oldheight);
	x += (oldwidth - newwidth) / 2;
	y += (oldheight - newheight) / 2;

	int top = 0;
	int left = 0;
	int bottom = 0;
	int right = 0;
	SDL_GetWindowBordersSize(_Window, &top, &left, &bottom, &right);

	SDL_Rect usable;
	if (SDL_GetDisplayUsableBounds(SDL_GetDisplayForWindow(_Window), &usable)) {
		if (x + newwidth + right > usable.x + usable.w) x = usable.x + usable.w - newwidth - right;
		if (y + newheight + bottom > usable.y + usable.h) y = usable.y + usable.h - newheight - bottom;
		if (x - left < usable.x) x = usable.x + left;
		if (y - top < usable.y) y = usable.y + top;
	}

	SDL_SetWindowSize(_Window, newwidth, newheight);
	SDL_SetWindowPosition(_Window, x, y);
}


int Platform_Window_Refresh_Rate(void)
{
	if (_Window == nullptr) {
		return(0);
	}

	SDL_DisplayMode const * mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(_Window));
	return(mode != nullptr ? (int)std::lround(mode->refresh_rate) : 0);
}


/// <summary>
/// Hands every event SDL has gathered to the game, in the order they happened.
/// </summary>
void Platform_Pump_Events(void)
{
	if (_Window == nullptr) {
		return;
	}

	SDL_Event sdlevent;
	while (SDL_PollEvent(&sdlevent)) {
		if (!Watched(sdlevent.type)) {
			Handle_SDL_Event(sdlevent);
		}
	}

	_HeldKeysStale = true;
}


void Platform_Begin_Native_Modal(void)
{
	_NativeModals++;
}


// The focus changes a dialog of the game's own caused are dropped, and the current focus is
// passed on instead, because the player never left the game.
void Platform_End_Native_Modal(void)
{
	if (_NativeModals == 0 || --_NativeModals != 0 || _Window == nullptr) {
		return;
	}

	SDL_PumpEvents();
	SDL_FlushEvent(SDL_EVENT_WINDOW_FOCUS_LOST);
	SDL_FlushEvent(SDL_EVENT_WINDOW_FOCUS_GAINED);

	WindowEvent focus;
	focus.Type = ((SDL_GetWindowFlags(_Window) & SDL_WINDOW_INPUT_FOCUS) != 0) ? WINDOW_EVENT_FOCUS_GAINED : WINDOW_EVENT_FOCUS_LOST;
	Dispatch(focus);
}


void Platform_Capture_Mouse(bool capture)
{
	if (_Window != nullptr) {
		SDL_CaptureMouse(capture);
	}
}


bool Platform_Mouse_Captured(void)
{
	return(_Window != nullptr && (SDL_GetWindowFlags(_Window) & SDL_WINDOW_MOUSE_CAPTURE) != 0);
}


void Platform_Confine_Cursor(bool confine)
{
	if (_Window != nullptr) {
		SDL_SetWindowMouseGrab(_Window, confine);
	}
}


// The position is read from the system rather than from the last event, so it is current
// even while the game is not pumping events.
bool Platform_Cursor_Position(int & x, int & y)
{
	if (_Window == nullptr) {
		return(false);
	}

	float globalx = 0.0f;
	float globaly = 0.0f;
	SDL_GetGlobalMouseState(&globalx, &globaly);

	int windowx = 0;
	int windowy = 0;
	SDL_GetWindowPosition(_Window, &windowx, &windowy);

	float const density = Pixel_Density();
	x = (int)std::floor((globalx - (float)windowx) * density);
	y = (int)std::floor((globaly - (float)windowy) * density);
	return(true);
}


void Platform_Warp_Cursor(int x, int y)
{
	if (_Window != nullptr) {
		float const density = Pixel_Density();
		SDL_WarpMouseInWindow(_Window, (float)x / density, (float)y / density);
	}
}


/// <summary>
/// Reports whether a key or mouse button is held. Mouse buttons are read from the system as
/// they stand, with the left and right buttons as the player sees them; keys are as of the
/// last pump.
/// </summary>
bool Platform_Key_Down(int virtualkey)
{
	switch (virtualkey) {
		case VK_LBUTTON:	return((SDL_GetGlobalMouseState(nullptr, nullptr) & SDL_BUTTON_LMASK) != 0);
		case VK_RBUTTON:	return((SDL_GetGlobalMouseState(nullptr, nullptr) & SDL_BUTTON_RMASK) != 0);
		case VK_MBUTTON:	return((SDL_GetGlobalMouseState(nullptr, nullptr) & SDL_BUTTON_MMASK) != 0);
		case VK_XBUTTON1:	return((SDL_GetGlobalMouseState(nullptr, nullptr) & SDL_BUTTON_X1MASK) != 0);
		case VK_XBUTTON2:	return((SDL_GetGlobalMouseState(nullptr, nullptr) & SDL_BUTTON_X2MASK) != 0);
		default:			break;
	}

	if (virtualkey <= 0 || virtualkey >= 256 || !_Started) {
		return(false);
	}

	Refresh_Held_Keys();
	return(_HeldKeys[virtualkey]);
}


bool Platform_Key_Toggled(int virtualkey)
{
	SDL_Keymod const modifiers = SDL_GetModState();
	switch (virtualkey) {
		case VK_CAPITAL:	return((modifiers & SDL_KMOD_CAPS) != 0);
		case VK_NUMLOCK:	return((modifiers & SDL_KMOD_NUM) != 0);
		case VK_SCROLL:		return((modifiers & SDL_KMOD_SCROLL) != 0);
		default:			return(false);
	}
}


std::string Platform_Key_Name(int virtualkey)
{
	return(Virtual_Key_Name(virtualkey));
}


PlatformCursor * Platform_Create_Cursor(unsigned int const * pixels, int width, int height, int hotx, int hoty)
{
	if (!_Started || pixels == nullptr || width <= 0 || height <= 0) {
		return(nullptr);
	}

	SDL_Surface * surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_ARGB8888, (void *)pixels, width * 4);
	if (surface == nullptr) {
		return(nullptr);
	}

	SDL_Cursor * cursor = SDL_CreateColorCursor(surface, hotx, hoty);
	SDL_DestroySurface(surface);
	return((PlatformCursor *)cursor);
}


void Platform_Destroy_Cursor(PlatformCursor * cursor)
{
	if (_Started && cursor != nullptr) {
		SDL_DestroyCursor((SDL_Cursor *)cursor);
	}
}


void Platform_Set_Cursor(PlatformCursor * cursor)
{
	if (!_Started) {
		return;
	}

	if (cursor == nullptr) {
		SDL_HideCursor();
		return;
	}

	SDL_SetCursor((SDL_Cursor *)cursor);
	SDL_ShowCursor();
}


PlatformCursor * Platform_System_Cursor(PlatformCursorShape shape)
{
	if (!_Started || shape < 0 || shape >= PLATFORM_CURSOR_COUNT) {
		return(nullptr);
	}

	SDL_Cursor * & cursor = _SystemCursors[shape];
	if (cursor == nullptr) {
		if (shape == PLATFORM_CURSOR_ARROW) {
			cursor = Resource_Arrow();
		}
		if (cursor == nullptr) {
			cursor = SDL_CreateSystemCursor(System_Cursor_Of(shape));
		}
	}
	return((PlatformCursor *)cursor);
}


std::string Platform_Clipboard_Text(void)
{
	if (!_Started) {
		return(std::string());
	}

	char * text = SDL_GetClipboardText();
	std::string result = (text != nullptr) ? text : "";
	SDL_free(text);
	return(result);
}


void Platform_Set_Clipboard_Text(std::string const & text)
{
	if (_Started) {
		SDL_SetClipboardText(text.c_str());
	}
}


void Platform_Error_Box(char const * title, char const * text)
{
	Platform_Begin_Native_Modal();
	if (!SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, text, _Window)) {
		DebugString("SDL: the message box was not shown: %s\n", SDL_GetError());
	}
	Platform_End_Native_Modal();
}
