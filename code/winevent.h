/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "platform/windowevent.hh"
#include "ui/uiinput.h"
#include "win.h"


// What a message cannot say about itself, read from the window when the message arrives.
struct WinMessageContext
{
	bool Unicode = true;
	unsigned int CodePage = CP_UTF8;
	int Modifiers = 0;

	// The screen position of the client area's top-left corner.
	int ClientX = 0;
	int ClientY = 0;

	// Whether a WM_CAPTURECHANGED names a window other than this one.
	bool CaptureElsewhere = false;
};


// Turns window messages into window events, holding a character that arrives split across
// messages until it is complete.
class WinEventTranslatorClass
{
	public:
		// Returns the number of events written, which is 0 for a message with no event.
		int Translate(UINT message, WPARAM wparam, LPARAM lparam, WinMessageContext const & context, WindowEvent * events, int capacity);
		void Reset_Text(void);

	private:
		int Text_Unit(wchar_t unit, WindowEvent * events, int capacity);
		int Text_Byte(unsigned char byte, unsigned int codepage, WindowEvent * events, int capacity);

		wchar_t HighSurrogate = 0;
		UIUTF8DecoderClass Utf8;
		unsigned char LegacyLead = 0;
};


WinMessageContext Win_Message_Context(HWND window, UINT message, LPARAM lparam);
