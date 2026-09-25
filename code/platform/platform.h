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


void Platform_Shutdown(void);

bool Platform_Create_Main_Window(bool windowed, int width, int height);
NativeWindow Platform_Native_Window(void);
bool Platform_Window_Drawable_Size(int & width, int & height);
bool Platform_Window_Minimized(void);

// The desktop position and size, in pixels, of the main window's client area.
bool Platform_Window_Client_Rect(int & x, int & y, int & width, int & height);

// Gives the client area the size in pixels, growing or shrinking the window about its
// middle and keeping it inside the usable area of its display.
void Platform_Resize_Window(int width, int height);

// Returns the refresh rate of the display showing the main window in hertz, or 0 when unknown.
int Platform_Window_Refresh_Rate(void);

void Platform_Pump_Events(void);

void Platform_Capture_Mouse(bool capture);
bool Platform_Mouse_Captured(void);
void Platform_Confine_Cursor(bool confine);

// Positions are in window client pixels.
bool Platform_Cursor_Position(int & x, int & y);
void Platform_Warp_Cursor(int x, int y);

// Keys are Windows virtual-key codes, reported as of the input event being handled. Mouse
// buttons report their current state, since a release outside the window may be lost.
bool Platform_Key_Down(int virtualkey);
bool Platform_Key_Toggled(int virtualkey);

// The name the keyboard layout gives a key, in UTF-8, or an empty string for no key.
std::string Platform_Key_Name(int virtualkey);

// The pixels are 32-bit ARGB, top row first. Returns NULL when the cursor cannot be made.
// The caller owns the cursor and frees it with Platform_Destroy_Cursor.
SDL_Cursor * Platform_Create_Cursor(unsigned int const * pixels, int width, int height, int hotx, int hoty);
void Platform_Destroy_Cursor(SDL_Cursor * cursor);

// Shows the cursor over the main window, or hides the pointer there when cursor is NULL.
void Platform_Set_Cursor(SDL_Cursor * cursor);

// UI_CURSOR_ARROW is the window's own arrow. The platform owns these cursors and frees them in
// Platform_Shutdown; do not destroy one.
SDL_Cursor * Platform_System_Cursor(UICursor shape);

std::string Platform_Clipboard_Text(void);
void Platform_Set_Clipboard_Text(std::string const & text);

// Shows an error over the main window, or on its own before the window exists, and waits
// for the player to dismiss it. The text is UTF-8.
void Platform_Error_Box(char const * title, char const * text);
