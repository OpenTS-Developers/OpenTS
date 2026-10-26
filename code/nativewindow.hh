/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once


enum NativeWindowType
{
	NATIVE_WINDOW_DEFAULT,
	NATIVE_WINDOW_WAYLAND,
};


// The native handles bgfx needs to present into a window supplied by the application shell.
// Handle is an HWND on Windows and an NSWindow on macOS.
// Display is unused on platforms where the window identifies its display by itself.
struct NativeWindow
{
	NativeWindowType Type;
	void * Display;
	void * Handle;
};
