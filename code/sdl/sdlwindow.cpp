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
#if defined(_WIN32)
#include "win.h"
#include <commctrl.h>
#endif

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if defined(__APPLE__)
#include <objc/message.h>
#include <objc/runtime.h>
#endif

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <vector>


namespace
{

bool _Started = false;
SDL_Window * _Window = nullptr;
int _NativeModals = 0;
bool _Watching = false;
bool _Unattended = false;

SDLInputStateClass _Input;

SDL_Cursor * _SystemCursors[UI_CURSOR_COUNT];

// The SDL event types the main window posts when Windows takes its mouse capture away, and
// when a window drag, a resize or the system menu ends.
Uint32 _CaptureCancelled = 0;
Uint32 _ModalLoopEnded = 0;
#if !defined(_WIN32)
bool _RepaintRequested = false;
#endif
#if defined(__APPLE__)
int System_Double_Click_Ms(void)
{
	using Interval = double (*)(id, SEL);
	id const eventclass = (id)objc_getClass("NSEvent");
	double const seconds = ((Interval)objc_msgSend)(eventclass, sel_registerName("doubleClickInterval"));
	if (!(seconds > 0.0)) {
		return(500);
	}
	return((int)std::lround(seconds * 1000.0));
}


// A launch from Terminal is not the foreground app, so the window stays behind it and startup never sees the focus it waits for.
void Order_Window_Front(void)
{
	using SendBool = void (*)(id, SEL, BOOL);
	using SendId = void (*)(id, SEL, id);
	id const app = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSApplication"), sel_registerName("sharedApplication"));
	((SendBool)objc_msgSend)(app, sel_registerName("activateIgnoringOtherApps:"), YES);

	if (_Window == nullptr) {
		return;
	}
	id const window = (id)SDL_GetPointerProperty(SDL_GetWindowProperties(_Window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
	if (window != nullptr) {
		((SendId)objc_msgSend)(window, sel_registerName("makeKeyAndOrderFront:"), nil);
	}
}
#endif
#if defined(_WIN32)
UINT_PTR const WindowSubclass = 2;
#endif


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
#if defined(_WIN32)
	Set_Hint(SDL_HINT_WINDOWS_CLOSE_ON_ALT_F4, "1");
	Set_Hint(SDL_HINT_WINDOWS_ENABLE_MENU_MNEMONICS, "0");
	Set_Hint(SDL_HINT_WINDOWS_RAW_KEYBOARD, "0");
	Set_Hint(SDL_HINT_WINDOWS_GAMEINPUT, "0");
	Set_Hint(SDL_HINT_WINDOWS_ERASE_BACKGROUND_MODE, "0");
	Set_Hint(SDL_HINT_WINDOWS_INTRESOURCE_ICON, IDI_SUN);
	Set_Hint(SDL_HINT_WINDOWS_INTRESOURCE_ICON_SMALL, IDI_SUN);
#endif
	Set_Hint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
	Set_Hint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");
	Set_Hint(SDL_HINT_WINDOW_ALLOW_TOPMOST, "0");
	Set_Hint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
	Set_Hint(SDL_HINT_MOUSE_AUTO_CAPTURE, "1");
	Set_Hint(SDL_HINT_MOUSE_EMULATE_WARP_WITH_RELATIVE, "0");
#if defined(_WIN32)
	Set_Hint(SDL_HINT_MOUSE_DOUBLE_CLICK_TIME, (int)GetDoubleClickTime());
	Set_Hint(SDL_HINT_MOUSE_DOUBLE_CLICK_RADIUS, GetSystemMetrics(SM_CXDOUBLECLK) / 2);
#elif defined(__APPLE__)
	// macOS 14 leaves a terminal launch in the background unless this is set before SDL starts.
	Set_Hint(SDL_HINT_MAC_BACKGROUND_APP, "0");
	Set_Hint(SDL_HINT_MOUSE_DOUBLE_CLICK_TIME, System_Double_Click_Ms());
	// macOS has no system double-click distance. 2 is the Windows default.
	Set_Hint(SDL_HINT_MOUSE_DOUBLE_CLICK_RADIUS, 2);
#endif
	Set_Hint(SDL_HINT_KEYCODE_OPTIONS, "french_numbers,latin_letters");
}


#if defined(__APPLE__)
// An icon passed to SDL stays square in the Dock. Pixels outside the application-icon squircle are made transparent.
void Clear_Icon_Corners(std::vector<unsigned char> & pixels, int width, int height)
{
	float const exponent = 5.0f;
	float const halfwidth = width * 0.5f;
	float const halfheight = height * 0.5f;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			float const nx = std::fabs((x + 0.5f - halfwidth) / halfwidth);
			float const ny = std::fabs((y + 0.5f - halfheight) / halfheight);
			float const field = std::pow(nx, exponent) + std::pow(ny, exponent);
			float const gx = (nx > 0.0f) ? exponent * std::pow(nx, exponent - 1.0f) / halfwidth : 0.0f;
			float const gy = (ny > 0.0f) ? exponent * std::pow(ny, exponent - 1.0f) / halfheight : 0.0f;
			float const gradient = std::sqrt(gx * gx + gy * gy);
			float coverage = 1.0f;
			if (gradient > 0.0f) {
				coverage = std::clamp(0.5f + (1.0f - field) / gradient, 0.0f, 1.0f);
			}
			size_t const alpha = ((size_t)y * (size_t)width + (size_t)x) * 4 + 3;
			pixels[alpha] = (unsigned char)std::lround(pixels[alpha] * coverage);
		}
	}
}
#endif


void Set_Window_Icon(MainWindowIcon const & icon)
{
	if (icon.Width <= 0 || icon.Height <= 0 || icon.Width > std::numeric_limits<int>::max() / 4) {
		return;
	}

	size_t const pitch = (size_t)icon.Width * 4;
	if ((size_t)icon.Height > icon.Pixels.size() / pitch) {
		return;
	}

#if defined(__APPLE__)
	std::vector<unsigned char> rounded(icon.Pixels.begin(), icon.Pixels.begin() + pitch * (size_t)icon.Height);
	Clear_Icon_Corners(rounded, icon.Width, icon.Height);
	void * source = rounded.data();
#else
	void * source = (void *)icon.Pixels.data();
#endif
	SDL_Surface * surface = SDL_CreateSurfaceFrom(icon.Width, icon.Height, SDL_PIXELFORMAT_RGBA32, source, (int)pitch);
	if (surface == nullptr) {
		DebugString("SDL: the window icon surface was not created: %s\n", SDL_GetError());
		return;
	}
	if (!SDL_SetWindowIcon(_Window, surface)) {
		DebugString("SDL: the window icon was not set: %s\n", SDL_GetError());
	}
	SDL_DestroySurface(surface);
}


float Pixel_Density(void)
{
	float density = (_Window != nullptr) ? SDL_GetWindowPixelDensity(_Window) : 1.0f;
	return(density > 0.0f ? density : 1.0f);
}


// On macOS the size is already the window's point size. Elsewhere it is a pixel size.
static int Client_Points(int size)
{
#if defined(__APPLE__)
	return(size);
#else
	return((int)std::lround(size / Pixel_Density()));
#endif
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


#if defined(_WIN32)
HWND Window_Handle(void)
{
	if (_Window == nullptr) {
		return(NULL);
	}
	return((HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(_Window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
}
#endif


void Post_Event(Uint32 type)
{
	if (type != 0) {
		SDL_Event event;
		SDL_zero(event);
		event.type = type;
		SDL_PushEvent(&event);
	}
}


#if defined(_WIN32)
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
#endif


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


bool Start_SDL(void)
{
	if (_Started) {
		return(true);
	}

	Set_Hints();

#if defined(_WIN32)
	// The window class keeps the game's own name, which tools looking for the window use.
	if (!SDL_RegisterApp("Tiberian Sun", 0, ProgramInstance)) {
		DebugString("SDL: the window class was not registered: %s\n", SDL_GetError());
	}
#endif
	SDL_SetMainReady();

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		DebugString("SDL: video did not start: %s\n", SDL_GetError());
#if defined(_WIN32)
		SDL_UnregisterApp();
#endif
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


// Brackets a dialog with a message loop of its own, so the focus it takes from the main
// window is not taken for the player switching away.
void Begin_Native_Modal(void)
{
	_NativeModals++;
}


// The focus changes a dialog of the game's own caused are dropped, and the current focus is
// passed on instead, because the player never left the game.
void End_Native_Modal(void)
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


}


/// <summary>
/// Destroys the main window, if there is one, and stops SDL. Calling it again, or before SDL
/// started, does nothing. The renderer must have let go of the window first.
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
#if defined(_WIN32)
		RemoveWindowSubclass(Window_Handle(), Watch_Messages, WindowSubclass);
#endif
		SDL_DestroyWindow(_Window);
		_Window = nullptr;
	}

	SDL_Quit();
#if defined(_WIN32)
	SDL_UnregisterApp();
#endif
	_Started = false;
}


/// <summary>
/// Starts SDL, then creates and shows the main window. A window for windowed play has a client
/// area of the size given and is centered on the primary display, kept clear of its top and
/// left edges; otherwise the window covers the display without changing its mode.
/// </summary>
/// <returns>False when SDL could not start or the window could not be created.</returns>
bool Main_Window_Create(bool windowed, int width, int height, MainWindowIcon const * icon)
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

	if (icon != nullptr) {
		Set_Window_Icon(*icon);
	}

	if (windowed) {
		int const clientwidth = Client_Points(width);
		int const clientheight = Client_Points(height);
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
#if defined(_WIN32)
	SetWindowSubclass(Window_Handle(), Watch_Messages, WindowSubclass, 0);
#endif
	_Input.Reset();

	if (!_Unattended) {
		SDL_ShowWindow(_Window);
		SDL_RaiseWindow(_Window);
#if defined(__APPLE__)
		Order_Window_Front();
#endif
	}

	// Typed text is always on, as characters from Windows were.
	SDL_StartTextInput(_Window);

	return(true);
}


NativeWindow Main_Window_Native(void)
{
	NativeWindow window = { NATIVE_WINDOW_DEFAULT, nullptr, nullptr };
	if (_Window != nullptr) {
#if defined(__APPLE__)
		window.Handle = SDL_GetPointerProperty(SDL_GetWindowProperties(_Window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#elif defined(_WIN32)
		window.Handle = SDL_GetPointerProperty(SDL_GetWindowProperties(_Window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#endif
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

	int const newwidth = Client_Points(width);
	int const newheight = Client_Points(height);

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


// A mode is either pixels or points. On macOS, dividing by the desktop density lists the point size.
static std::pair<int, int> Mode_Size(SDL_DisplayMode const * mode, float screen)
{
	float const density = mode->pixel_density > 0.0f ? mode->pixel_density : 1.0f;
#if defined(__APPLE__)
	float const scale = screen > 0.0f ? density / screen : 1.0f;
#else
	(void)screen;
	float const scale = density;
#endif
	return(std::pair<int, int>((int)std::lround(mode->w * scale), (int)std::lround(mode->h * scale)));
}


static float Primary_Screen_Density(SDL_DisplayID display)
{
#if defined(__APPLE__)
	SDL_DisplayMode const * desktop = SDL_GetCurrentDisplayMode(display);
	return((desktop != nullptr && desktop->pixel_density > 0.0f) ? desktop->pixel_density : 1.0f);
#else
	(void)display;
	return(1.0f);
#endif
}


std::vector<std::pair<int, int>> Main_Window_Fullscreen_Sizes(void)
{
	std::vector<std::pair<int, int>> sizes;
	if (!_Started) {
		return(sizes);
	}

	int count = 0;
	SDL_DisplayID const display = SDL_GetPrimaryDisplay();
	SDL_DisplayMode ** modes = SDL_GetFullscreenDisplayModes(display, &count);
	if (modes == nullptr) {
		DebugString("SDL: the display modes were not listed: %s\n", SDL_GetError());
		return(sizes);
	}

	float const screen = Primary_Screen_Density(display);

	for (int index = 0; index < count; index++) {
		sizes.push_back(Mode_Size(modes[index], screen));
	}
	SDL_free(modes);

	std::sort(sizes.begin(), sizes.end());
	sizes.erase(std::unique(sizes.begin(), sizes.end()), sizes.end());
	return(sizes);
}


bool Main_Window_Desktop_Size(int & width, int & height)
{
	if (!_Started) {
		return(false);
	}

	SDL_DisplayID const display = SDL_GetPrimaryDisplay();
	SDL_DisplayMode const * desktop = SDL_GetCurrentDisplayMode(display);
	if (desktop == nullptr) {
		return(false);
	}

	std::pair<int, int> const size = Mode_Size(desktop, Primary_Screen_Density(display));
	width = size.first;
	height = size.second;
	return(width > 0 && height > 0);
}


float Main_Window_Content_Scale(void)
{
	if (!_Started) {
		return(1.0f);
	}

	float const scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
	return(scale > 0.0f ? scale : 1.0f);
}


void Main_Window_Pump_Events(void)
{
	if (_Window == nullptr) {
		return;
	}

#if !defined(_WIN32)
	if (_RepaintRequested) {
		_RepaintRequested = false;
		SDL_Event exposed;
		SDL_zero(exposed);
		exposed.type = SDL_EVENT_WINDOW_EXPOSED;
		exposed.window.windowID = SDL_GetWindowID(_Window);
		Handle_SDL_Event(exposed);
	}
#endif

	SDL_Event sdlevent;
	while (SDL_PollEvent(&sdlevent)) {
		if (!Watched(sdlevent.type)) {
			Handle_SDL_Event(sdlevent);
		}
	}
}


void Main_Window_Request_Repaint(void)
{
#if defined(_WIN32)
	HWND const window = Window_Handle();
	if (window != NULL) {
		InvalidateRect(window, NULL, FALSE);
	}
#else
	if (_Window != nullptr) {
		_RepaintRequested = true;
	}
#endif
}


// SDL_RaiseWindow would take the foreground from another program.
void Main_Window_Take_Focus(void)
{
#if defined(_WIN32)
	HWND const window = Window_Handle();
	if (window != NULL) {
		SetFocus(window);
	}
#else
	// Raising the window would take the foreground from another program, and macOS
	// has no focus call that leaves the active application unchanged.
#endif
}


void Main_Window_Capture_Mouse(bool capture)
{
	if (_Window == nullptr) {
		return;
	}
	SDL_CaptureMouse(capture);

#if defined(_WIN32)
	// SDL still records a capture Windows took away, so it does not ask for it again.
	HWND const window = Window_Handle();
	if (capture && (SDL_GetWindowFlags(_Window) & SDL_WINDOW_MOUSE_CAPTURE) != 0 && GetCapture() != window) {
		SetCapture(window);
	}
#endif
}


bool Main_Window_Mouse_Captured(void)
{
#if defined(_WIN32)
	HWND const window = Window_Handle();
	return(window != NULL && GetCapture() == window);
#else
	return(_Window != nullptr && (SDL_GetWindowFlags(_Window) & SDL_WINDOW_MOUSE_CAPTURE) != 0);
#endif
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


float Main_Window_Pixel_Density(void)
{
	return(Pixel_Density());
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
		// Otherwise SDL shows its own arrow in place of a destroyed cursor.
		if (SDL_GetCursor() == cursor) {
			SDL_SetCursor(Main_Window_System_Cursor(UI_CURSOR_ARROW));
		}
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
		cursor = SDL_CreateSystemCursor(System_Cursor_Of(shape));
	}
	return(cursor);
}


void Main_Window_Set_Arrow(SDL_Cursor * cursor)
{
	if (!_Started) {
		if (cursor != nullptr) {
			SDL_DestroyCursor(cursor);
		}
		return;
	}

	SDL_Cursor * & arrow = _SystemCursors[UI_CURSOR_ARROW];
	if (arrow == cursor) {
		return;
	}

	SDL_Cursor * const previous = arrow;
	arrow = cursor;
	if (previous != nullptr && SDL_GetCursor() == previous) {
		SDL_SetCursor(Main_Window_System_Cursor(UI_CURSOR_ARROW));
	}
	if (previous != nullptr) {
		SDL_DestroyCursor(previous);
	}
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


void Main_Window_Set_Unattended(bool unattended)
{
	_Unattended = unattended;
}


static void Report_Unattended_Box(char const * title, char const * text)
{
	DebugString("SDL: %s: %s\n", title, text);
	std::fprintf(stderr, "%s\n", text);
	std::fflush(stderr);
}


void Main_Window_Error_Box(char const * title, char const * text)
{
	if (_Unattended) {
		Report_Unattended_Box(title, text);
		return;
	}

	Begin_Native_Modal();
	if (!SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, text, _Window)) {
		DebugString("SDL: the message box was not shown: %s\n", SDL_GetError());
	}
	End_Native_Modal();
}


bool Main_Window_Confirm_Box(char const * title, char const * text, char const * yes, char const * no)
{
	if (_Unattended) {
		Report_Unattended_Box(title, text);
		return(false);
	}

	SDL_MessageBoxButtonData const buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, yes },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, no },
	};

	SDL_MessageBoxData box;
	SDL_zero(box);
	box.flags = SDL_MESSAGEBOX_WARNING | SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT;
	box.window = _Window;
	box.title = title;
	box.message = text;
	box.numbuttons = SDL_arraysize(buttons);
	box.buttons = buttons;

	Begin_Native_Modal();
	int button = -1;
	bool const shown = SDL_ShowMessageBox(&box, &button);
	if (!shown) {
		DebugString("SDL: the message box was not shown: %s\n", SDL_GetError());
	}
	End_Native_Modal();
	return(!shown || button == 1);
}
