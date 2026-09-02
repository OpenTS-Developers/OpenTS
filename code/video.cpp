/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine's side of the presenter. The game draws its frame into the visible surface
// as it always has; this decides when that frame reaches the screen and where in the
// window it lands, and hands it to the renderer behind video.h.

#include "always.h"

#include "video.h"

#include "_surface.h"
#include "bgfxbackend.h"
#include "dbgprint.h"
#include "dsurface.h"
#include "globals.h"
#include "goptions.h"
#include "hostclock.h"
#include "misc.h"
#include "surface.h"
#include "wincursor.h"

#include <cstdlib>


/*
 * The size of the frame the game renders into. It is not tied to the window, which may be
 * any size, nor to the desktop, whose mode the game no longer changes.
 */
int VideoModeWidth = 0;
int VideoModeHeight = 0;

/*
 * Is the game running in a framed, resizable window rather than in a borderless one
 * covering the whole screen? The display mode is never changed either way.
 */
bool WindowedMode = false;

static HWND _Window = NULL;
static bool _Initialized = false;
static VideoScaleInfo _ScaleInfo;

// Set whenever the visible surface is written to, and cleared once that frame has been
// presented. A frame that is skipped for pacing stays marked, so the next present shows
// the newest content rather than a stale one.
static bool _FrameIsDirty = false;
static unsigned int _LastPresentTime = 0;
static unsigned int _PresentInterval = 16;

// Presents can nest, because a dialog repainting itself presents from inside the paint
// that the engine's own present provoked.
static bool _Presenting = false;


/// <summary>
/// Works out the shortest sensible gap between presents from the display's refresh rate.
/// </summary>
static void Update_Present_Interval(void)
{
	int refresh = 0;
	HDC dc = GetDC(_Window);

	if (dc != NULL) {
		refresh = GetDeviceCaps(dc, VREFRESH);
		ReleaseDC(_Window, dc);
	}

	if (refresh <= 1) {
		refresh = 60;
	}

	_PresentInterval = (unsigned int)(1000 / refresh);
	if (_PresentInterval < 3) {
		_PresentInterval = 3;
	}
	if (_PresentInterval > 100) {
		_PresentInterval = 100;
	}
}


/// <summary>
/// Works out where the game's frame sits inside the window.
/// The frame keeps its shape, so it is grown by whichever of the two axes runs out first
/// and centered in what is left over.
/// </summary>
static void Update_Scale_Info(void)
{
	RECT client;

	_ScaleInfo.GameWidth = VideoModeWidth;
	_ScaleInfo.GameHeight = VideoModeHeight;

	if (_Window == NULL || !GetClientRect(_Window, &client)) {
		client.left = 0;
		client.top = 0;
		client.right = VideoModeWidth;
		client.bottom = VideoModeHeight;
	}

	_ScaleInfo.WindowWidth = client.right - client.left;
	_ScaleInfo.WindowHeight = client.bottom - client.top;

	if (_ScaleInfo.GameWidth <= 0 || _ScaleInfo.GameHeight <= 0 || _ScaleInfo.WindowWidth <= 0 || _ScaleInfo.WindowHeight <= 0) {
		_ScaleInfo.DestX = 0;
		_ScaleInfo.DestY = 0;
		_ScaleInfo.DestWidth = _ScaleInfo.WindowWidth;
		_ScaleInfo.DestHeight = _ScaleInfo.WindowHeight;
		_ScaleInfo.ScaleX = 1.0f;
		_ScaleInfo.ScaleY = 1.0f;
		return;
	}

	double scalex = (double)_ScaleInfo.WindowWidth / (double)_ScaleInfo.GameWidth;
	double scaley = (double)_ScaleInfo.WindowHeight / (double)_ScaleInfo.GameHeight;
	double scale = (scalex < scaley) ? scalex : scaley;

	if (Options.IntegerScaling && scale >= 1.0) {
		scale = (double)(int)scale;
	}

	_ScaleInfo.DestWidth = (int)((double)_ScaleInfo.GameWidth * scale);
	_ScaleInfo.DestHeight = (int)((double)_ScaleInfo.GameHeight * scale);
	_ScaleInfo.DestX = (_ScaleInfo.WindowWidth - _ScaleInfo.DestWidth) / 2;
	_ScaleInfo.DestY = (_ScaleInfo.WindowHeight - _ScaleInfo.DestHeight) / 2;
	_ScaleInfo.ScaleX = (float)((double)_ScaleInfo.DestWidth / (double)_ScaleInfo.GameWidth);
	_ScaleInfo.ScaleY = (float)((double)_ScaleInfo.DestHeight / (double)_ScaleInfo.GameHeight);
}


/// <summary>
/// Converts the configured filter into the one the renderer names.
/// </summary>
static BackendScaleMode Backend_Scale_Mode(void)
{
	switch (Options.ScaleMode) {
		case VIDEO_SCALE_LINEAR:
			return(BACKEND_SCALE_LINEAR);

		case VIDEO_SCALE_NEAREST:
			return(BACKEND_SCALE_NEAREST);

		default:
			return(BACKEND_SCALE_PIXELART);
	}
}


/// <summary>
/// Starts the presenter on the game's window.
/// </summary>
/// <param name="window">The main window. Its client area receives the frame.</param>
/// <returns>bool; Did the presenter start? A false return is fatal to the game.</returns>
bool Video_Init(HWND window)
{
	RECT client;

	if (_Initialized) {
		return(true);
	}

	if (window == NULL || !GetClientRect(window, &client)) {
		return(false);
	}

	_Window = window;

	BackendRenderer renderer = (BackendRenderer)Options.Renderer;
	if (!Backend_Init(window, client.right - client.left, client.bottom - client.top, renderer, Options.VSync)) {
		return(false);
	}

	DebugString("Video: renderer is %s\n", Backend_Renderer_Name());

	_Initialized = true;

	if (!Backend_Set_Frame_Size(VideoModeWidth, VideoModeHeight)) {
		Backend_Shutdown();
		_Initialized = false;
		return(false);
	}

	Update_Scale_Info();
	Update_Present_Interval();
	return(true);
}


/// <summary>
/// Stops the presenter and releases the renderer.
/// </summary>
void Video_Shutdown(void)
{
	if (!_Initialized) {
		return;
	}

	Win_Cursor_Shutdown();
	Backend_Shutdown();
	_Initialized = false;
	_Window = NULL;
	_FrameIsDirty = false;
}


/// <summary>
/// Moves the game to a different render resolution.
/// The caller replaces the surfaces afterwards; this only resizes what the frame is
/// presented from and leaves the previous mode untouched when it fails.
/// </summary>
/// <param name="width">The new frame width.</param>
/// <param name="height">The new frame height.</param>
/// <returns>bool; Was the mode changed?</returns>
bool Video_Set_Mode(int width, int height)
{
	if (!_Initialized || width <= 0 || height <= 0) {
		return(false);
	}

	if (!Backend_Set_Frame_Size(width, height)) {
		return(false);
	}

	VideoModeWidth = width;
	VideoModeHeight = height;

	Update_Scale_Info();
	Update_Present_Interval();
	Win_Cursor_Refresh();
	_FrameIsDirty = true;
	return(true);
}


/// <summary>
/// Tells the presenter the window's client area changed size.
/// </summary>
void Video_On_Resize(int width, int height)
{
	if (!_Initialized || width <= 0 || height <= 0) {
		return;
	}

	Backend_On_Resize(width, height);
	Update_Scale_Info();
	Update_Present_Interval();
	Win_Cursor_Refresh();
	Video_Mark_Dirty();
}


/// <summary>
/// Tells the presenter the desktop's display settings changed.
/// The window may now be on a monitor that refreshes at a different rate.
/// </summary>
void Video_On_Display_Change(void)
{
	if (!_Initialized) {
		return;
	}

	Update_Present_Interval();
	Video_Mark_Dirty();
}


/// <summary>
/// Records that the visible surface has been drawn to since the last present.
/// </summary>
void Video_Mark_Dirty(void)
{
	_FrameIsDirty = true;
}


/// <summary>
/// Puts the visible surface on the screen whatever its state.
/// </summary>
void Video_Present(void)
{
	if (!_Initialized || _Presenting || VisibleSurface == NULL) {
		return;
	}

	DSurface * surface = (DSurface *)VisibleSurface;
	void * pixels = surface->Get_Buffer();

	if (pixels == NULL) {
		return;
	}

	_Presenting = true;
	Backend_Present(pixels, surface->Stride(), _ScaleInfo.DestX, _ScaleInfo.DestY, _ScaleInfo.DestWidth, _ScaleInfo.DestHeight, Backend_Scale_Mode());
	_Presenting = false;

	_FrameIsDirty = false;
	_LastPresentTime = Host_Milliseconds();
}


/// <summary>
/// Puts the visible surface on the screen if it has changed and the display is ready for
/// another frame.
/// A skipped present leaves the frame marked, so the next one shows the newest content.
/// This never waits: the game loop is not paced by presentation.
/// </summary>
void Video_Present_If_Dirty(void)
{
	if (!_FrameIsDirty) {
		return;
	}

	unsigned int now = Host_Milliseconds();
	if ((now - _LastPresentTime) < _PresentInterval) {
		return;
	}

	Video_Present();
}


/// <summary>
/// Reports where the game's frame is drawn inside the window.
/// </summary>
VideoScaleInfo const & Video_Get_Scale_Info(void)
{
	return(_ScaleInfo);
}


/// <summary>
/// Compares two display modes by width and then height.
/// </summary>
static int __cdecl Compare_Modes(void const * left, void const * right)
{
	int const * lhs = (int const *)left;
	int const * rhs = (int const *)right;

	if (lhs[0] != rhs[0]) {
		return(lhs[0] - rhs[0]);
	}
	return(lhs[1] - rhs[1]);
}


/// <summary>
/// Collects the display resolutions that fall within the given bounds.
/// Only the sizes matter; the desktop decides the color depth, and duplicates that differ
/// only by refresh rate are reported once.
/// </summary>
/// <param name="minwidth">The narrowest mode to report.</param>
/// <param name="minheight">The shortest mode to report.</param>
/// <param name="maxwidth">The widest mode to report.</param>
/// <param name="maxheight">The tallest mode to report.</param>
/// <returns>A caller owned array of width and height pairs ending in a zero pair, or NULL
/// when nothing matched.</returns>
int * EnumDisplayModes(int minwidth, int minheight, int maxwidth, int maxheight)
{
	DEVMODE devmode;
	int count = 0;
	int capacity = 0;
	int * modes = NULL;

	for (int pass = 0; pass < 2; pass++) {

		count = 0;

		for (int index = 0; ; index++) {
			memset(&devmode, 0, sizeof(devmode));
			devmode.dmSize = sizeof(devmode);

			if (!EnumDisplaySettings(NULL, index, &devmode)) {
				break;
			}

			int width = (int)devmode.dmPelsWidth;
			int height = (int)devmode.dmPelsHeight;

			if (width < minwidth || width > maxwidth || height < minheight || height > maxheight) {
				continue;
			}

			if (modes != NULL) {
				// The list is being filled from a second enumeration; should it have
				// grown since the one that sized the array, the extra modes are dropped.
				if (count >= capacity) {
					break;
				}
				modes[count * 2] = width;
				modes[count * 2 + 1] = height;
			}
			count++;
		}

		if (modes != NULL) {
			break;
		}

		if (count == 0) {
			return(NULL);
		}

		capacity = count;
		modes = new int[(count + 1) * 2];
	}

	qsort(modes, count, sizeof(int) * 2, Compare_Modes);

	// The same size is listed once per refresh rate and color depth it supports.
	int unique = 0;
	for (int index = 0; index < count; index++) {
		if (unique == 0 || modes[unique * 2 - 2] != modes[index * 2] || modes[unique * 2 - 1] != modes[index * 2 + 1]) {
			modes[unique * 2] = modes[index * 2];
			modes[unique * 2 + 1] = modes[index * 2 + 1];
			unique++;
		}
	}

	modes[unique * 2] = 0;
	modes[unique * 2 + 1] = 0;
	return(modes);
}
