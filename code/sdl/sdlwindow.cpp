/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "sdl/sdlwindow.h"

#include "dbgprint.h"
#include "gamewindow.h"
#include "resource.h"
#include "sdl/sdlevents.h"
#include "sdl/sdlinput.h"
#include "sdl/sdlkeys.h"
#include "win.h"

#include <commctrl.h>

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cmath>
#include <cstdio>
#include <vector>


namespace
{

bool _Started = false;
SDL_Window * _Window = nullptr;
int _NativeModals = 0;
bool _Watching = false;

SDLInputStateClass _Input;

SDL_Cursor * _SystemCursors[UI_CURSOR_COUNT];

// The SDL event types the main window posts when Windows takes its mouse capture away, and
// when a window drag, a resize or the system menu ends.
Uint32 _CaptureCancelled = 0;
Uint32 _ModalLoopEnded = 0;
UINT_PTR const WindowSubclass = 2;


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
	if (event.Type == WINDOW_EVENT_FOCUS_GAINED) {
		_Input.Focus_Gained();
	} else if (event.Type == WINDOW_EVENT_FOCUS_LOST) {
		_Input.Focus_Lost();
	}

	// The screen saver may start while the player is elsewhere, but not over the game.
	if (event.Type == WINDOW_EVENT_FOCUS_GAINED) {
		SDL_DisableScreenSaver();
	} else if (event.Type == WINDOW_EVENT_FOCUS_LOST) {
		SDL_EnableScreenSaver();
	}

	Game_Window_Handle_Event(event);
}


HWND Window_Handle(void)
{
	if (_Window == nullptr) {
		return(NULL);
	}
	return((HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(_Window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
}


void Post_Event(Uint32 type)
{
	if (type != 0) {
		SDL_Event event;
		SDL_zero(event);
		event.type = type;
		SDL_PushEvent(&event);
	}
}


// Windows can end the capture while the window keeps the focus, for a system menu or another
// window taking the mouse, and SDL reports neither. The window's own releases are ignored.
LRESULT CALLBACK Watch_Messages(HWND window, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR, DWORD_PTR)
{
	if (message == WM_CANCELMODE || (message == WM_CAPTURECHANGED && lparam != 0 && (HWND)lparam != window)) {
		Post_Event(_CaptureCancelled);
	}

	// SDL reset the keyboard when the drag, the resize or the system menu began.
	LRESULT const result = DefSubclassProc(window, message, wparam, lparam);
	if (message == WM_EXITSIZEMOVE || message == WM_EXITMENULOOP) {
		Post_Event(_ModalLoopEnded);
	}
	return(result);
}


void Handle_SDL_Event(SDL_Event const & sdlevent)
{
	if (_CaptureCancelled != 0 && sdlevent.type == _CaptureCancelled) {
		WindowEvent event;
		event.Type = WINDOW_EVENT_CAPTURE_LOST;
		Dispatch(event);
		return;
	}

	_Input.Drop_Released_Keys();

	// Windows' key state is stale once the focus has gone elsewhere.
	if (_ModalLoopEnded != 0 && sdlevent.type == _ModalLoopEnded) {
		if (_Window != nullptr && (SDL_GetWindowFlags(_Window) & SDL_WINDOW_INPUT_FOCUS) != 0) {
			_Input.Hold_Windows_Keys();
		}
		return;
	}

	if (sdlevent.type == SDL_EVENT_QUIT) {
		DebugString("SDL: the system asked the game to quit\n");
	}

	int const before = _Input.Modifiers();
	bool continued = false;
	if (sdlevent.type == SDL_EVENT_KEY_DOWN || sdlevent.type == SDL_EVENT_KEY_UP) {
		continued = _Input.Track_Key(sdlevent.key);
	}

	std::vector<WindowEvent> events;
	Window_Events_From_SDL(sdlevent, Pixel_Density(), _Input.Modifiers(), events);
	for (WindowEvent & event : events) {
		if (event.Type == WINDOW_EVENT_KEY_DOWN && continued) {
			event.Repeat = true;
		}

		// Windows reports the release of Alt itself as a system key too.
		if (event.Type == WINDOW_EVENT_KEY_UP && (before & WINDOW_MOD_ALT) != 0 && (before & WINDOW_MOD_CTRL) == 0) {
			event.System = true;
		}
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


bool SDLCALL Watch_Keys(void *, SDL_Event * sdlevent)
{
	if (sdlevent->type == SDL_EVENT_KEY_DOWN) {
		_Input.Note_Queued_Key(sdlevent->key);
	}
	return(true);
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


SDL_SystemCursor System_Cursor_Of(UICursor shape)
{
	switch (shape) {
		case UI_CURSOR_TEXT:		return(SDL_SYSTEM_CURSOR_TEXT);
		case UI_CURSOR_HAND:		return(SDL_SYSTEM_CURSOR_POINTER);
		case UI_CURSOR_RESIZE_NS:	return(SDL_SYSTEM_CURSOR_NS_RESIZE);
		case UI_CURSOR_RESIZE_EW:	return(SDL_SYSTEM_CURSOR_EW_RESIZE);
		case UI_CURSOR_RESIZE_NESW:	return(SDL_SYSTEM_CURSOR_NESW_RESIZE);
		case UI_CURSOR_RESIZE_NWSE:	return(SDL_SYSTEM_CURSOR_NWSE_RESIZE);
		case UI_CURSOR_MOVE:		return(SDL_SYSTEM_CURSOR_MOVE);
		case UI_CURSOR_UNAVAILABLE:	return(SDL_SYSTEM_CURSOR_NOT_ALLOWED);
		default:					return(SDL_SYSTEM_CURSOR_DEFAULT);
	}
}

}


static bool Start_SDL(void)
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

	Uint32 const events = SDL_RegisterEvents(2);
	if (events != 0) {
		_CaptureCancelled = events;
		_ModalLoopEnded = events + 1;
	}
	_Started = true;
	return(true);
}


/// <summary>
/// Destroys the main window and stops SDL. Calling it again, or before the window was created,
/// does nothing. The renderer must have let go of the window first.
/// </summary>
void Main_Window_Destroy(void)
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
		SDL_RemoveEventWatch(Watch_Keys, nullptr);
		RemoveWindowSubclass(Window_Handle(), Watch_Messages, WindowSubclass);
		SDL_DestroyWindow(_Window);
		_Window = nullptr;
	}

	SDL_Quit();
	SDL_UnregisterApp();
	_Started = false;
}


/// <summary>
/// Starts SDL, then creates and shows the main window. A window for windowed play has a client
/// area of the size given and is centered on the primary display, kept clear of its top and
/// left edges; otherwise the window covers the display without changing its mode.
/// </summary>
/// <returns>False when SDL could not start or the window could not be created.</returns>
bool Main_Window_Create(bool windowed, int width, int height)
{
	if (!Start_SDL() || _Window != nullptr) {
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
	SDL_AddEventWatch(Watch_Keys, nullptr);
	SetWindowSubclass(Window_Handle(), Watch_Messages, WindowSubclass, 0);
	_Input.Reset();

	SDL_ShowWindow(_Window);
	SDL_RaiseWindow(_Window);

	// Typed text is always on, as characters from Windows were.
	SDL_StartTextInput(_Window);

	return(true);
}


NativeWindow Main_Window_Native(void)
{
	NativeWindow window = { NATIVE_WINDOW_DEFAULT, nullptr, nullptr };
	if (_Window != nullptr) {
		window.Handle = SDL_GetPointerProperty(SDL_GetWindowProperties(_Window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
	}
	return(window);
}


bool Main_Window_Drawable_Size(int & width, int & height)
{
	if (_Window == nullptr || !SDL_GetWindowSizeInPixels(_Window, &width, &height)) {
		return(false);
	}
	return(width > 0 && height > 0);
}


bool Main_Window_Minimized(void)
{
	return(_Window != nullptr && (SDL_GetWindowFlags(_Window) & SDL_WINDOW_MINIMIZED) != 0);
}


bool Main_Window_Client_Rect(int & x, int & y, int & width, int & height)
{
	if (_Window == nullptr || !SDL_GetWindowPosition(_Window, &x, &y)) {
		return(false);
	}

	float const density = Pixel_Density();
	x = (int)std::floor((float)x * density);
	y = (int)std::floor((float)y * density);
	return(Main_Window_Drawable_Size(width, height));
}


// A window grown about its middle can be pushed past the edges of the display, and a title
// bar above its top edge cannot be grabbed to bring the window back.
void Main_Window_Resize(int width, int height)
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


int Main_Window_Refresh_Rate(void)
{
	if (_Window == nullptr) {
		return(0);
	}

	SDL_DisplayMode const * mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(_Window));
	return(mode != nullptr ? (int)std::lround(mode->refresh_rate) : 0);
}


void Main_Window_Pump_Events(void)
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
}


// Brackets a dialog with a message loop of its own, so the focus it takes from the main
// window is not taken for the player switching away.
static void Begin_Native_Modal(void)
{
	_NativeModals++;
}


// The focus changes a dialog of the game's own caused are dropped, and the current focus is
// passed on instead, because the player never left the game.
static void End_Native_Modal(void)
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


void Main_Window_Capture_Mouse(bool capture)
{
	if (_Window == nullptr) {
		return;
	}
	SDL_CaptureMouse(capture);

	// SDL still records a capture Windows took away, so it does not ask for it again.
	HWND const window = Window_Handle();
	if (capture && (SDL_GetWindowFlags(_Window) & SDL_WINDOW_MOUSE_CAPTURE) != 0 && GetCapture() != window) {
		SetCapture(window);
	}
}


bool Main_Window_Mouse_Captured(void)
{
	HWND const window = Window_Handle();
	return(window != NULL && GetCapture() == window);
}


void Main_Window_Confine_Cursor(bool confine)
{
	if (_Window != nullptr) {
		SDL_SetWindowMouseGrab(_Window, confine);
	}
}


// The position is read from the system rather than from the last event, so it is current
// even while the game is not pumping events.
bool Main_Window_Cursor_Position(int & x, int & y)
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


void Main_Window_Warp_Cursor(int x, int y)
{
	if (_Window != nullptr) {
		float const density = Pixel_Density();
		SDL_WarpMouseInWindow(_Window, (float)x / density, (float)y / density);
	}
}


// Mouse buttons follow the player's left-handed button setting.
bool Main_Window_Key_Down(int virtualkey)
{
	switch (virtualkey) {
		case VK_LBUTTON:	return((SDL_GetGlobalMouseState(nullptr, nullptr) & SDL_BUTTON_LMASK) != 0);
		case VK_RBUTTON:	return((SDL_GetGlobalMouseState(nullptr, nullptr) & SDL_BUTTON_RMASK) != 0);
		case VK_MBUTTON:	return((SDL_GetGlobalMouseState(nullptr, nullptr) & SDL_BUTTON_MMASK) != 0);
		case VK_XBUTTON1:	return((SDL_GetGlobalMouseState(nullptr, nullptr) & SDL_BUTTON_X1MASK) != 0);
		case VK_XBUTTON2:	return((SDL_GetGlobalMouseState(nullptr, nullptr) & SDL_BUTTON_X2MASK) != 0);
		default:			break;
	}

	return(_Started && _Input.Key_Down(virtualkey));
}


bool Main_Window_Key_Toggled(int virtualkey)
{
	return(_Input.Key_Toggled(virtualkey));
}


std::string Main_Window_Key_Name(int virtualkey)
{
	return(Virtual_Key_Name(virtualkey));
}


SDL_Cursor * Main_Window_Create_Cursor(unsigned int const * pixels, int width, int height, int hotx, int hoty)
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
	return(cursor);
}


void Main_Window_Destroy_Cursor(SDL_Cursor * cursor)
{
	if (_Started && cursor != nullptr) {
		SDL_DestroyCursor(cursor);
	}
}


void Main_Window_Set_Cursor(SDL_Cursor * cursor)
{
	if (!_Started) {
		return;
	}

	if (cursor == nullptr) {
		SDL_HideCursor();
		return;
	}

	SDL_SetCursor(cursor);
	SDL_ShowCursor();
}


SDL_Cursor * Main_Window_System_Cursor(UICursor shape)
{
	if (!_Started || shape < 0 || shape >= UI_CURSOR_COUNT) {
		return(nullptr);
	}

	SDL_Cursor * & cursor = _SystemCursors[shape];
	if (cursor == nullptr) {
		if (shape == UI_CURSOR_ARROW) {
			cursor = Resource_Arrow();
		}
		if (cursor == nullptr) {
			cursor = SDL_CreateSystemCursor(System_Cursor_Of(shape));
		}
	}
	return(cursor);
}


std::string Main_Window_Clipboard_Text(void)
{
	if (!_Started) {
		return(std::string());
	}

	char * text = SDL_GetClipboardText();
	std::string result = (text != nullptr) ? text : "";
	SDL_free(text);
	return(result);
}


void Main_Window_Set_Clipboard_Text(std::string const & text)
{
	if (_Started) {
		SDL_SetClipboardText(text.c_str());
	}
}


void Main_Window_Error_Box(char const * title, char const * text)
{
	Begin_Native_Modal();
	if (!SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, text, _Window)) {
		DebugString("SDL: the message box was not shown: %s\n", SDL_GetError());
	}
	End_Native_Modal();
}
