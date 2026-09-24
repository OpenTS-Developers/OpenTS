/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ui/uiinput.h"
#include "win.h"

#include <string>


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
		virtual void Mark_Overlay_Dirty(void) = 0;
		virtual void Present_If_Dirty(void) = 0;
		virtual void Present_Now(void) = 0;
		virtual bool Movie_Playing(void) const = 0;

		virtual void Play_Sample(char const * name, float volume) = 0;
		virtual void Play_Click(void) = 0;
		virtual bool Animate_Screens(void) const = 0;
		virtual int Art_Magnification(void) const = 0;
		virtual bool Bitmap_System_Font(void) const = 0;
		virtual bool Developer_Keys_Armed(void) const = 0;
		virtual void Clear_Keyboard_Queue(void) = 0;
		virtual void Focus_Main_Window(void) = 0;
		virtual bool Take_Capture(void) = 0;
		virtual void Release_Capture(void) = 0;

		virtual bool Key_Down(int virtualkey) const = 0;
		virtual bool Key_Toggled(int virtualkey) const = 0;
		virtual std::string System_Font_Path(char const * face) const = 0;
		virtual void Apply_Cursor(UICursor cursor) = 0;
		virtual void Restore_Game_Cursor(void) = 0;

		virtual char const * String(int id) const = 0;
		virtual void Log(char const * text) = 0;
		virtual int Milliseconds(void) const = 0;
};
