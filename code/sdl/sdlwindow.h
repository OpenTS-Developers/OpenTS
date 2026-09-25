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

#include <string>


struct SDL_Cursor;


void Main_Window_Destroy(void);

bool Main_Window_Create(bool windowed, int width, int height);
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

void Main_Window_Pump_Events(void);

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

// The pixels are 32-bit ARGB, top row first. Returns NULL when the cursor cannot be made.
// The caller owns the cursor and frees it with Main_Window_Destroy_Cursor.
SDL_Cursor * Main_Window_Create_Cursor(unsigned int const * pixels, int width, int height, int hotx, int hoty);
void Main_Window_Destroy_Cursor(SDL_Cursor * cursor);

// Shows the cursor over the main window, or hides the pointer there when cursor is NULL.
void Main_Window_Set_Cursor(SDL_Cursor * cursor);

// UI_CURSOR_ARROW is the window's own arrow. The SDL layer owns these cursors and frees them in
// Main_Window_Destroy; do not destroy one.
SDL_Cursor * Main_Window_System_Cursor(UICursor shape);

std::string Main_Window_Clipboard_Text(void);
void Main_Window_Set_Clipboard_Text(std::string const & text);

// Shows an error over the main window, or on its own before the window exists, and waits
// for the player to dismiss it. The text is UTF-8.
void Main_Window_Error_Box(char const * title, char const * text);
