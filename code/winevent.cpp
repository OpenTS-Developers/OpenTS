/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "winevent.h"

#include <windowsx.h>


namespace
{

int Emit(WindowEvent const & event, WindowEvent * events, int capacity, int count)
{
	if (count < capacity) {
		events[count] = event;
		return(count + 1);
	}
	return(count);
}


WindowEvent Mouse_Event(WindowEventType type, WindowMouseButton button, int clicks, LPARAM lparam)
{
	WindowEvent event;
	event.Type = type;
	event.Button = button;
	event.Clicks = clicks;
	event.X = GET_X_LPARAM(lparam);
	event.Y = GET_Y_LPARAM(lparam);
	return(event);
}


WindowMouseButton X_Button(WPARAM wparam)
{
	return(GET_XBUTTON_WPARAM(wparam) == XBUTTON1 ? WINDOW_BUTTON_X1 : WINDOW_BUTTON_X2);
}

}


int WinEventTranslatorClass::Translate(UINT message, WPARAM wparam, LPARAM lparam, WinMessageContext const & context, WindowEvent * events, int capacity)
{
	WindowEvent event;

	switch (message) {
		case WM_MOUSEMOVE:
			return(Emit(Mouse_Event(WINDOW_EVENT_MOUSE_MOVE, WINDOW_BUTTON_LEFT, 1, lparam), events, capacity, 0));

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			return(Emit(Mouse_Event(WINDOW_EVENT_MOUSE_DOWN, WINDOW_BUTTON_LEFT, message == WM_LBUTTONDBLCLK ? 2 : 1, lparam), events, capacity, 0));

		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			return(Emit(Mouse_Event(WINDOW_EVENT_MOUSE_DOWN, WINDOW_BUTTON_RIGHT, message == WM_RBUTTONDBLCLK ? 2 : 1, lparam), events, capacity, 0));

		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
			return(Emit(Mouse_Event(WINDOW_EVENT_MOUSE_DOWN, WINDOW_BUTTON_MIDDLE, message == WM_MBUTTONDBLCLK ? 2 : 1, lparam), events, capacity, 0));

		case WM_XBUTTONDOWN:
		case WM_XBUTTONDBLCLK:
			return(Emit(Mouse_Event(WINDOW_EVENT_MOUSE_DOWN, X_Button(wparam), message == WM_XBUTTONDBLCLK ? 2 : 1, lparam), events, capacity, 0));

		case WM_LBUTTONUP:
			return(Emit(Mouse_Event(WINDOW_EVENT_MOUSE_UP, WINDOW_BUTTON_LEFT, 1, lparam), events, capacity, 0));

		case WM_RBUTTONUP:
			return(Emit(Mouse_Event(WINDOW_EVENT_MOUSE_UP, WINDOW_BUTTON_RIGHT, 1, lparam), events, capacity, 0));

		case WM_MBUTTONUP:
			return(Emit(Mouse_Event(WINDOW_EVENT_MOUSE_UP, WINDOW_BUTTON_MIDDLE, 1, lparam), events, capacity, 0));

		case WM_XBUTTONUP:
			return(Emit(Mouse_Event(WINDOW_EVENT_MOUSE_UP, X_Button(wparam), 1, lparam), events, capacity, 0));

		// Wheel messages carry a screen position.
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
			event = Mouse_Event(WINDOW_EVENT_MOUSE_WHEEL, WINDOW_BUTTON_LEFT, 1, lparam);
			event.X -= context.ClientX;
			event.Y -= context.ClientY;
			event.Wheel = (float)GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA;
			event.Horizontal = (message == WM_MOUSEHWHEEL);
			return(Emit(event, events, capacity, 0));

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
			event.Type = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN) ? WINDOW_EVENT_KEY_DOWN : WINDOW_EVENT_KEY_UP;
			event.VirtualKey = (int)(wparam & 0xFF);
			event.Modifiers = context.Modifiers;
			event.Repeat = event.Type == WINDOW_EVENT_KEY_DOWN && (lparam & (1 << 30)) != 0;
			event.System = (message == WM_SYSKEYDOWN || message == WM_SYSKEYUP);
			return(Emit(event, events, capacity, 0));

		case WM_CHAR:
			if (context.Unicode) {
				return(Text_Unit((wchar_t)wparam, events, capacity));
			}
			return(Text_Byte((unsigned char)wparam, context.CodePage, events, capacity));

		case WM_ACTIVATEAPP:
			Reset_Text();
			event.Type = (wparam != 0) ? WINDOW_EVENT_FOCUS_GAINED : WINDOW_EVENT_FOCUS_LOST;
			return(Emit(event, events, capacity, 0));

		case WM_CAPTURECHANGED:
			if (!context.CaptureElsewhere) {
				return(0);
			}
			Reset_Text();
			event.Type = WINDOW_EVENT_CAPTURE_LOST;
			return(Emit(event, events, capacity, 0));

		case WM_CANCELMODE:
			Reset_Text();
			event.Type = WINDOW_EVENT_CAPTURE_LOST;
			return(Emit(event, events, capacity, 0));

		case WM_INPUTLANGCHANGE:
			Reset_Text();
			event.Type = WINDOW_EVENT_KEYMAP_CHANGED;
			return(Emit(event, events, capacity, 0));

		case WM_PAINT:
			event.Type = WINDOW_EVENT_EXPOSED;
			return(Emit(event, events, capacity, 0));

		case WM_SIZE:
			if (wparam == SIZE_MINIMIZED) {
				return(0);
			}
			event.Type = WINDOW_EVENT_RESIZED;
			event.Width = LOWORD(lparam);
			event.Height = HIWORD(lparam);
			return(Emit(event, events, capacity, 0));

		case WM_MOVE:
			event.Type = WINDOW_EVENT_MOVED;
			return(Emit(event, events, capacity, 0));

		case WM_DISPLAYCHANGE:
			event.Type = WINDOW_EVENT_DISPLAY_CHANGED;
			return(Emit(event, events, capacity, 0));

		case WM_SYSCOMMAND:
			if (wparam != SC_CLOSE) {
				return(0);
			}
			event.Type = WINDOW_EVENT_CLOSE_REQUESTED;
			return(Emit(event, events, capacity, 0));

		default:
			return(0);
	}
}


void WinEventTranslatorClass::Reset_Text(void)
{
	HighSurrogate = 0;
	Utf8.Reset();
	LegacyLead = 0;
}


// An unpaired surrogate becomes U+FFFD, so a broken pair still shows where it was.
int WinEventTranslatorClass::Text_Unit(wchar_t unit, WindowEvent * events, int capacity)
{
	WindowEvent event;
	event.Type = WINDOW_EVENT_TEXT;
	int count = 0;

	if (unit >= 0xD800 && unit < 0xDC00) {
		if (HighSurrogate != 0) {
			event.Text = 0xFFFD;
			count = Emit(event, events, capacity, count);
		}
		HighSurrogate = unit;
		return(count);
	}

	char32_t code = unit;
	if (unit >= 0xDC00 && unit < 0xE000) {
		code = (HighSurrogate != 0) ? 0x10000 + (((char32_t)HighSurrogate - 0xD800) << 10) + ((char32_t)unit - 0xDC00) : 0xFFFD;
	} else if (HighSurrogate != 0) {
		event.Text = 0xFFFD;
		count = Emit(event, events, capacity, count);
	}
	HighSurrogate = 0;

	event.Text = code;
	return(Emit(event, events, capacity, count));
}


int WinEventTranslatorClass::Text_Byte(unsigned char byte, unsigned int codepage, WindowEvent * events, int capacity)
{
	WindowEvent event;
	event.Type = WINDOW_EVENT_TEXT;
	int count = 0;

	if (codepage == CP_UTF8) {
		UIInputText text = Utf8.Feed(byte);
		for (unsigned index = 0; index < text.Count; index++) {
			event.Text = text.Codepoints[index];
			count = Emit(event, events, capacity, count);
		}
		return(count);
	}

	char bytes[2];
	int length;
	if (LegacyLead != 0) {
		bytes[0] = (char)LegacyLead;
		bytes[1] = (char)byte;
		length = 2;
		LegacyLead = 0;
	} else if (IsDBCSLeadByteEx(codepage, byte)) {
		LegacyLead = byte;
		return(0);
	} else {
		bytes[0] = (char)byte;
		length = 1;
	}

	wchar_t wide[2];
	int converted = MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS, bytes, length, wide, 2);
	event.Text = 0xFFFD;
	if (converted == 1) {
		event.Text = wide[0];
	} else if (converted == 2 && wide[0] >= 0xD800 && wide[0] < 0xDC00 && wide[1] >= 0xDC00 && wide[1] < 0xE000) {
		event.Text = 0x10000 + (((char32_t)wide[0] - 0xD800) << 10) + ((char32_t)wide[1] - 0xDC00);
	}
	return(Emit(event, events, capacity, count));
}


WinMessageContext Win_Message_Context(HWND window, UINT message, LPARAM lparam)
{
	WinMessageContext context;
	context.Unicode = IsWindowUnicode(window) != FALSE;
	context.CodePage = GetACP();

	if ((GetKeyState(VK_SHIFT) & 0x8000) != 0) {
		context.Modifiers |= WINDOW_MOD_SHIFT;
	}
	if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
		context.Modifiers |= WINDOW_MOD_CTRL;
	}
	if ((GetKeyState(VK_MENU) & 0x8000) != 0) {
		context.Modifiers |= WINDOW_MOD_ALT;
	}

	if (message == WM_MOUSEWHEEL || message == WM_MOUSEHWHEEL) {
		POINT origin = { 0, 0 };
		ClientToScreen(window, &origin);
		context.ClientX = origin.x;
		context.ClientY = origin.y;
	}

	context.CaptureElsewhere = (message == WM_CAPTURECHANGED) && (HWND)lparam != window;

	return(context);
}
