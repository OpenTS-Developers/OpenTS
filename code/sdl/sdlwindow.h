/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "nativewindow.hh"
#include "ui/uiinput.h"

#include <span>
#include <string>
#include <utility>
#include <vector>


struct SDL_Cursor;


struct MainWindowIcon
{
	std::span<unsigned char const> Pixels;
	int Width;
	int Height;
};


void Main_Window_Destroy(void);

// Set before Main_Window_Create, an unattended run's window is never shown, and each message
// box writes its text to the debug log and stderr and answers no.
void Main_Window_Set_Unattended(bool unattended);

// Optional icon pixels are straight-alpha RGBA, top row first, borrowed only for this call.
bool Main_Window_Create(bool windowed, int width, int height, MainWindowIcon const * icon = nullptr);
NativeWindow Main_Window_Native(void);
bool Main_Window_Drawable_Size(int & width, int & height);
bool Main_Window_Minimized(void);

// The desktop position and size, in pixels, of the main window's client area.
bool Main_Window_Client_Rect(int & x, int & y, int & width, int & height);

// Gives the client area the size in pixels, growing or shrinking the window about its
// middle and keeping it inside the usable area of its display.
void Main_Window_Resize(int width, int height);

// Returns the refresh rate of the display showing the main window in hertz, or 0 when unknown.
int Main_Window_Refresh_Rate(void);

// The primary display's mode sizes in pixels, sorted and without repeats.
std::vector<std::pair<int, int>> Main_Window_Fullscreen_Sizes(void);

// The primary display's current size in the same pixels as Main_Window_Fullscreen_Sizes.
bool Main_Window_Desktop_Size(int & width, int & height);

// The scale Windows asks content to be drawn at on the primary display. One leaves sizes alone.
float Main_Window_Content_Scale(void);

void Main_Window_Pump_Events(void);

// Requests made before a pump reach the game as one WINDOW_EVENT_EXPOSED.
void Main_Window_Request_Repaint(void);

// Does not bring the game in front of another program.
void Main_Window_Take_Focus(void);

void Main_Window_Capture_Mouse(bool capture);
bool Main_Window_Mouse_Captured(void);
void Main_Window_Confine_Cursor(bool confine);

// Positions are in window client pixels.
bool Main_Window_Cursor_Position(int & x, int & y);
void Main_Window_Warp_Cursor(int x, int y);

// Keys are Windows virtual-key codes, reported as of the input event being handled, except that
// a key held while the window gained the focus is read from Windows until SDL reports it. Mouse
// buttons report their current state, since a release outside the window may be lost.
bool Main_Window_Key_Down(int virtualkey);
bool Main_Window_Key_Toggled(int virtualkey);

// The name the keyboard layout gives a key, in UTF-8, or an empty string for no key.
std::string Main_Window_Key_Name(int virtualkey);

// Physical pixels per window point. A cursor image is measured in points.
float Main_Window_Pixel_Density(void);

// The pixels are 32-bit ARGB, top row first. Returns NULL when the cursor cannot be made.
// The caller owns the cursor and frees it with Main_Window_Destroy_Cursor.
SDL_Cursor * Main_Window_Create_Cursor(unsigned int const * pixels, int width, int height, int hotx, int hoty);
void Main_Window_Destroy_Cursor(SDL_Cursor * cursor);

// Shows the cursor over the main window, or hides the pointer there when cursor is NULL.
void Main_Window_Set_Cursor(SDL_Cursor * cursor);

// UI_CURSOR_ARROW is the arrow set by Main_Window_Set_Arrow, or the system's. The SDL layer owns
// these cursors and frees them in Main_Window_Destroy; do not destroy one.
SDL_Cursor * Main_Window_System_Cursor(UICursor shape);

// Takes ownership of cursor and destroys the previous arrow; NULL restores the system's arrow.
void Main_Window_Set_Arrow(SDL_Cursor * cursor);

std::string Main_Window_Clipboard_Text(void);
void Main_Window_Set_Clipboard_Text(std::string const & text);

// Shows an error over the main window, or on its own before the window exists, and waits
// for the player to dismiss it. The text is UTF-8.
void Main_Window_Error_Box(char const * title, char const * text);

// Returns false for no or Escape, and true for yes or when the box cannot be shown.
bool Main_Window_Confirm_Box(char const * title, char const * text, char const * yes, char const * no);
