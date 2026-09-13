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

#include <string>


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
		// Presents whatever the interval since the last one, for a caller that will not
		// pump again before it blocks.
		virtual void Present_Now(void) = 0;
		virtual bool Movie_Playing(void) const = 0;

		// Plays a sound effect by name at a volume from zero to one. A host with no audio,
		// or a player who turned it off, does nothing.
		virtual void Play_Sample(char const * name, float volume) = 0;

		// The sound a control makes as it is pressed, which the rules name rather than the
		// shell, so a mod that changes the dialogs' click changes this one.
		virtual void Play_Click(void) = 0;

		// Whether a screen opens the way the Win32 dialogs did, through a widening band.
		// A harness says no, so its screens are whole from the first pass.
		virtual bool Animate_Screens(void) const = 0;

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
		// Whether a key that latches rather than repeats, Caps Lock or Num Lock, is on.
		virtual bool Key_Toggled(int virtualkey) const = 0;
		// Where the platform keeps the named font file, empty when it has no such face.
		// The shell names no path of its own, so a port answers this its own way.
		virtual std::string System_Font_Path(char const * face) const = 0;
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
