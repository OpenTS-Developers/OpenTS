/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// What the shell needs from the program around it. The engine supplies its window, frame,
// keyboard queue and dialogs; a test supplies a host it controls by hand.

#pragma once

#include "ui/uiinput.hh"
#include "win.h"


// Where the game's frame lands in the client area, in client pixels, and how many of them
// one game pixel spans.
struct UIFrameRect
{
	int X;
	int Y;
	int Width;
	int Height;
	float ScaleX;
	float ScaleY;
};


class UIShellHostClass
{
	public:
		virtual ~UIShellHostClass(void) = default;

		virtual HWND Main_Window(void) const = 0;
		virtual UIFrameRect Frame(void) const = 0;

		// The overlay changed and should be drawn at the next present.
		virtual void Mark_Overlay_Dirty(void) = 0;
		virtual void Present_If_Dirty(void) = 0;
		virtual bool Movie_Playing(void) const = 0;

		// A Win32 dialog is on screen and takes the mouse before a document can.
		virtual bool Legacy_Dialog_Visible(void) const = 0;
		// The player asked for the Win32 dialogs instead of the documents.
		virtual bool Legacy_Dialogs_Requested(void) const = 0;
		virtual bool Developer_Keys_Armed(void) const = 0;

		// Drops the queued keys and mouse events. The engine's implementation pumps the
		// window messages to do so, which re-enters the shell.
		virtual void Clear_Keyboard_Queue(void) = 0;
		virtual void Focus_Main_Window(void) = 0;

		// Takes the mouse capture for the main window. False when it already held it, so
		// the caller knows not to release what it did not take.
		virtual bool Take_Capture(void) = 0;
		virtual void Release_Capture(void) = 0;
		virtual bool Screen_To_Client(int & x, int & y) const = 0;

		// Whether a key or mouse button is physically down. VK_LBUTTON and VK_RBUTTON name the
		// primary and secondary buttons as the messages do, whatever the user swapped.
		virtual bool Key_Down(int virtualkey) const = 0;
		// True when text messages carry UTF-16 units; otherwise they carry one byte each of
		// the code page below.
		virtual bool Window_Is_Unicode(void) const = 0;
		virtual unsigned int Text_Code_Page(void) const = 0;

		// Shows the pointer shape a document asked for, and puts the game's own back.
		virtual void Apply_Cursor(UICursor cursor) = 0;
		virtual void Restore_Game_Cursor(void) = 0;

		// An engine string by identifier. The result is valid until the next call.
		virtual char const * String(int id) const = 0;
		virtual void Log(char const * text) = 0;
};
