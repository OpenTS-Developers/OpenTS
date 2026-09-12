/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uishell.h"

#include "ui/dev/uidev.h"
#include "ui/rml/rmlfile.h"
#include "ui/rml/rmlkeys.h"
#include "ui/rml/rmlrender.h"
#include "ui/rml/rmlsystem.h"
#include "ui/rml/rmlview.h"
#include "ui/uicoord.h"
#include "ui/uihost.h"

// windowsx.h, which win.h brings in, names two window walkers the way RmlUi names its
// element walkers.
#undef GetFirstChild
#undef GetNextSibling

#include <RmlUi/Core.h>

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <memory>


static UIShellHostClass * _Host = nullptr;

// The interfaces outlive Rml::Shutdown, which releases every resource through them.
static std::unique_ptr<UIRmlSystemClass> _System;
static UIRmlFileClass _File;
static UIRmlBgfxRenderClass _Render;

static Rml::Context * _Context = NULL;
static bool _Ready = false;
static bool _FontLoaded = false;

// Set while the context updates or renders. Work that arrives then waits for the next tick.
static bool _InContext = false;
static bool _InHook = false;
static bool _InTick = false;

static bool _PendingResize = false;
static bool _PendingLeave = false;
static bool _PendingRelease = false;
static int _PendingDevFocus = -1;

// The presses the shell consumed, as a mask over the mouse button indices, and whether it
// took the window's capture for them. Their releases belong to the shell wherever they land.
// The developer overlays' own presses are a subset that their release goes back to.
static unsigned int _OwnedButtons = 0;
static unsigned int _DevOwnedButtons = 0;
static bool _TookCapture = false;
static bool _MouseInside = false;

// The modal screen the runner is driving, and whether it is between releasing its document
// and handing the input back.
static UIRmlViewClass * _Modal = NULL;
static bool _ModalClosing = false;

static wchar_t _HighSurrogate = 0;

#ifdef _DEBUG

// The test document is a developer's check of the shell; F9 shows and hides it, and F6 the
// developer overlays.
static Rml::ElementDocument * _TestDocument = NULL;
static bool _PendingToggle = false;
static bool _PendingDevToggle = false;
static bool _CloseRequested = false;

class UITestListenerClass : public Rml::EventListener
{
	public:
		virtual void ProcessEvent(Rml::Event &) override
		{
			_CloseRequested = true;
		}
};

static UITestListenerClass _TestListener;

#endif


static void Log(char const * format, ...)
{
	char buffer[512];
	va_list args;

	va_start(args, format);
	std::vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	_Host->Log(buffer);
}


static int Key_Modifiers(void)
{
	int modifiers = 0;

	if (GetKeyState(VK_SHIFT) & 0x8000) {
		modifiers |= Rml::Input::KM_SHIFT;
	}
	if (GetKeyState(VK_CONTROL) & 0x8000) {
		modifiers |= Rml::Input::KM_CTRL;
	}
	if (GetKeyState(VK_MENU) & 0x8000) {
		modifiers |= Rml::Input::KM_ALT;
	}
	if (GetKeyState(VK_CAPITAL) & 1) {
		modifiers |= Rml::Input::KM_CAPSLOCK;
	}
	if (GetKeyState(VK_NUMLOCK) & 1) {
		modifiers |= Rml::Input::KM_NUMLOCK;
	}

	return(modifiers);
}


static bool Documents_Visible(void)
{
	if (_Context == NULL) {
		return(false);
	}

	for (int index = 0; index < _Context->GetNumDocuments(); index++) {
		Rml::ElementDocument * document = _Context->GetDocument(index);
		if (document != NULL && document->IsVisible()) {
			return(true);
		}
	}

	return(false);
}


static bool Text_Input_Focused(void)
{
	Rml::Element * focus = _Context->GetFocusElement();
	if (focus == NULL) {
		return(false);
	}

	Rml::String const & tag = focus->GetTagName();
	return(tag == "input" || tag == "textarea");
}


static void Apply_Dimensions(void)
{
	UIFrameRect frame = _Host->Frame();

	float ratio = frame.ScaleX < frame.ScaleY ? frame.ScaleX : frame.ScaleY;
	if (ratio <= 0.0f) {
		ratio = 1.0f;
	}

	_Context->SetDimensions(Rml::Vector2i(frame.Width, frame.Height));
	_Context->SetDensityIndependentPixelRatio(ratio);
}


static UIPointerPosition Pointer_Position(LPARAM clientlparam)
{
	UIFrameRect frame = _Host->Frame();
	return(UI_Client_To_Overlay(frame.X, frame.Y, frame.Width, frame.Height, GET_X_LPARAM(clientlparam), GET_Y_LPARAM(clientlparam)));
}


// Forgets the presses the shell owns, telling the documents they ended, and gives the
// capture back when the shell took it.
static void Drop_Presses(void)
{
	unsigned int owned = _OwnedButtons;
	unsigned int devowned = _DevOwnedButtons;
	_OwnedButtons = 0;
	_DevOwnedButtons = 0;

	for (int button = 0; button < 3; button++) {
		if (devowned & (1u << button)) {
			UIDev_Mouse_Button(button, false);
		} else if (owned & (1u << button)) {
			_Context->ProcessMouseButtonUp(button, Key_Modifiers());
		}
	}

	if (_TookCapture) {
		_TookCapture = false;
		_Host->Release_Capture();
	}
}


#ifdef _DEBUG

static void Toggle_Test_Document(void)
{
	if (!_FontLoaded) {
		Log("UI: the test document needs the font, which did not load\n");
		return;
	}

	if (_TestDocument == NULL) {
		_TestDocument = _Context->LoadDocument("test.rml");
		if (_TestDocument == NULL) {
			Log("UI: test.rml did not load\n");
			return;
		}

		Rml::Element * close = _TestDocument->GetElementById("close");
		if (close != NULL) {
			close->AddEventListener(Rml::EventId::Click, &_TestListener);
		}

		_TestDocument->Show();
		_Render.Log_Resource_Counts("test document loaded and shown");
	} else if (_TestDocument->IsVisible()) {
		_TestDocument->Hide();
		_Render.Log_Resource_Counts("test document hidden");
	} else {
		_TestDocument->Show();
		_Render.Log_Resource_Counts("test document shown");
	}

	_Host->Mark_Overlay_Dirty();
}

#endif


bool UI_Init(UIShellHostClass & host)
{
	if (_Ready) {
		return(true);
	}

	_Host = &host;

	if (!_Render.Init()) {
		return(false);
	}

	_System = std::make_unique<UIRmlSystemClass>(host);

	Rml::SetSystemInterface(_System.get());
	Rml::SetFileInterface(&_File);
	Rml::SetRenderInterface(&_Render);

	if (!Rml::Initialise()) {
		Log("UI: RmlUi did not initialise\n");
		_Render.Shutdown();
		_System.reset();
		return(false);
	}

	UIFrameRect frame = host.Frame();
	_Context = Rml::CreateContext("main", Rml::Vector2i(frame.Width, frame.Height));
	if (_Context == NULL) {
		Log("UI: the context could not be created\n");
		Rml::Shutdown();
		_Render.Shutdown();
		_System.reset();
		return(false);
	}

	Apply_Dimensions();

	_FontLoaded = Rml::LoadFontFace("OpenSans.ttf");
	if (!_FontLoaded) {
		Log("UI: OpenSans.ttf did not load, so no document can be shown\n");
	}

	_Ready = true;
	Log("UI: RmlUi %s ready over a %dx%d frame at %.2f pixels per dp\n",
		Rml::GetVersion().c_str(), frame.Width, frame.Height, _Context->GetDensityIndependentPixelRatio());
	return(true);
}


void UI_Shutdown(void)
{
	if (!_Ready) {
		return;
	}

	_Ready = false;
	_Modal = NULL;
	_ModalClosing = false;

	if (_OwnedButtons != 0) {
		Drop_Presses();
	}

	UIDev_Shutdown(_Render);

#ifdef _DEBUG
	_TestDocument = NULL;
	_PendingToggle = false;
	_PendingDevToggle = false;
	_CloseRequested = false;
#endif

	Rml::RemoveContext("main");
	_Context = NULL;

	Rml::Shutdown();
	_Render.Shutdown();
	_System.reset();
	_FontLoaded = false;
}


bool UI_Use_Rml(void)
{
	return(_Ready && !_Host->Legacy_Dialogs_Requested());
}


bool UI_Screen_Shown(void)
{
	return(_Modal != NULL || _ModalClosing);
}


void UI_On_Video_Change(void)
{
	if (!_Ready) {
		return;
	}

	if (_InContext) {
		_PendingResize = true;
	} else {
		Apply_Dimensions();
	}

	_Host->Mark_Overlay_Dirty();
}


void UI_Tick(void)
{
	if (!_Ready || _InTick || _InContext) {
		return;
	}

	_InTick = true;

#ifdef _DEBUG
	if (_PendingToggle) {
		_PendingToggle = false;
		Toggle_Test_Document();
	}
	if (_PendingDevToggle) {
		_PendingDevToggle = false;
		UIDev_Toggle(_Render);
		_Host->Mark_Overlay_Dirty();
	}
	if (_CloseRequested) {
		_CloseRequested = false;
		if (_TestDocument != NULL && _TestDocument->IsVisible()) {
			_TestDocument->Hide();
			_Render.Log_Resource_Counts("test document closed");
			_Host->Mark_Overlay_Dirty();
		}
	}
#endif

	if (_PendingResize) {
		_PendingResize = false;
		Apply_Dimensions();
	}
	if (_PendingRelease) {
		_PendingRelease = false;
		Drop_Presses();
	}
	if (_PendingLeave) {
		_PendingLeave = false;
		_Context->ProcessMouseLeave();
		_MouseInside = false;
	}
	if (_PendingDevFocus >= 0) {
		UIDev_Focus(_PendingDevFocus != 0);
		_PendingDevFocus = -1;
	}

	_InContext = true;
	_Context->Update();
	UIDev_Tick();
	_InContext = false;

	// An overlay closed from inside its own frame still needs one present to clear.
	static bool devwasactive = false;
	bool devactive = UIDev_Active();
	if (Documents_Visible() || devactive || devwasactive) {
		_Host->Mark_Overlay_Dirty();
	}
	devwasactive = devactive;

	_InTick = false;
}


void UI_Render_Overlay(void)
{
	if (!_Ready || _InContext || _Host->Movie_Playing()) {
		return;
	}

	bool documents = Documents_Visible();
	bool overlays = UIDev_Active();
	if (!documents && !overlays) {
		return;
	}

	UIFrameRect frame = _Host->Frame();
	if (frame.Width <= 0 || frame.Height <= 0) {
		return;
	}

	if (documents) {
		_Render.Begin_Frame(frame.X, frame.Y, frame.Width, frame.Height);

		_InContext = true;
		_Context->Render();
		_InContext = false;
	}

	if (overlays) {
		_Render.Begin_Dev_Frame(frame.X, frame.Y, frame.Width, frame.Height);
		UIDev_Render(_Render);
	}
}


static bool Handle_Mouse_Move(LPARAM clientlparam)
{
	UIPointerPosition position = Pointer_Position(clientlparam);

	UIDev_Mouse_Position(position.X, position.Y);
	if (UIDev_Wants_Mouse()) {
		_Host->Mark_Overlay_Dirty();
		return(false);
	}

	if (_OwnedButtons != 0 || position.Inside) {
		_Context->ProcessMouseMove(position.X, position.Y, Key_Modifiers());
		_MouseInside = position.Inside;
		_Host->Mark_Overlay_Dirty();
	} else if (_MouseInside) {
		_Context->ProcessMouseLeave();
		_MouseInside = false;
		_Host->Mark_Overlay_Dirty();
	}

	return(false);
}


static void Own_Press(int button)
{
	if (_OwnedButtons == 0) {
		_TookCapture = _Host->Take_Capture();
	}
	_OwnedButtons |= (1u << button);
}


static void Release_Press(int button)
{
	_OwnedButtons &= ~(1u << button);
	_DevOwnedButtons &= ~(1u << button);
	if (_OwnedButtons == 0 && _TookCapture) {
		_TookCapture = false;
		_Host->Release_Capture();
	}
}


static bool Handle_Button_Down(int button, LPARAM clientlparam)
{
	UIPointerPosition position = Pointer_Position(clientlparam);

	if (UIDev_Active()) {
		UIDev_Mouse_Position(position.X, position.Y);
		if (UIDev_Mouse_Button(button, true)) {
			Own_Press(button);
			_DevOwnedButtons |= (1u << button);
			_Host->Mark_Overlay_Dirty();
			return(true);
		}
	}

	if (!position.Inside && _OwnedButtons == 0) {
		return(false);
	}

	int modifiers = Key_Modifiers();
	_Context->ProcessMouseMove(position.X, position.Y, modifiers);
	_MouseInside = position.Inside;

	bool interacting = !_Context->ProcessMouseButtonDown(button, modifiers);
	_Host->Mark_Overlay_Dirty();

	if (!interacting) {
		return(false);
	}

	Own_Press(button);
	return(true);
}


static bool Handle_Button_Up(int button, LPARAM clientlparam)
{
	UIPointerPosition position = Pointer_Position(clientlparam);

	if (_DevOwnedButtons & (1u << button)) {
		UIDev_Mouse_Position(position.X, position.Y);
		UIDev_Mouse_Button(button, false);
		Release_Press(button);
		_Host->Mark_Overlay_Dirty();
		return(true);
	}

	// A release the overlays did not own still ends the press they saw begin.
	if (UIDev_Active()) {
		UIDev_Mouse_Button(button, false);
	}

	if ((_OwnedButtons & (1u << button)) == 0) {
		return(false);
	}

	int modifiers = Key_Modifiers();

	_Context->ProcessMouseMove(position.X, position.Y, modifiers);
	_Context->ProcessMouseButtonUp(button, modifiers);
	_MouseInside = position.Inside;
	_Host->Mark_Overlay_Dirty();

	Release_Press(button);
	return(true);
}


static bool Handle_Wheel(WPARAM wparam, LPARAM screenlparam)
{
	int x = GET_X_LPARAM(screenlparam);
	int y = GET_Y_LPARAM(screenlparam);
	_Host->Screen_To_Client(x, y);

	UIFrameRect frame = _Host->Frame();
	UIPointerPosition position = UI_Client_To_Overlay(frame.X, frame.Y, frame.Width, frame.Height, x, y);

	// Windows counts wheel movement away from the user as positive; ImGui scrolls up for it
	// and RmlUi scrolls down.
	float delta = (float)(short)HIWORD(wparam) / (float)WHEEL_DELTA;

	if (UIDev_Active()) {
		UIDev_Mouse_Position(position.X, position.Y);
		if (UIDev_Mouse_Wheel(delta)) {
			_Host->Mark_Overlay_Dirty();
			return(true);
		}
	}

	if (!position.Inside) {
		return(false);
	}

	bool consumed = !_Context->ProcessMouseWheel(Rml::Vector2f(0.0f, -delta), Key_Modifiers());
	_Host->Mark_Overlay_Dirty();
	return(consumed);
}


static bool Handle_Key(UINT message, WPARAM wparam)
{
	if (UIDev_Key(wparam, message == WM_KEYDOWN)) {
		_Host->Mark_Overlay_Dirty();
		return(true);
	}

	Rml::Input::KeyIdentifier key = UI_Key_Identifier((int)(wparam & 0xFF));
	if (key == Rml::Input::KI_UNKNOWN) {
		return(false);
	}

	bool propagated;
	if (message == WM_KEYDOWN) {
		propagated = _Context->ProcessKeyDown(key, Key_Modifiers());
	} else {
		propagated = _Context->ProcessKeyUp(key, Key_Modifiers());
	}

	_Host->Mark_Overlay_Dirty();
	return(!propagated || Text_Input_Focused());
}


// Windows delivers a character beyond the basic plane as two messages; the first half
// waits for the second. Carriage returns become newlines and control characters stay out.
static bool Handle_Char(WPARAM wparam)
{
	wchar_t unit = (wchar_t)wparam;

	if (UIDev_Character(unit)) {
		_Host->Mark_Overlay_Dirty();
		return(true);
	}

	if (unit >= 0xD800 && unit < 0xDC00) {
		_HighSurrogate = unit;
		return(false);
	}

	char32_t code = unit;
	if (unit >= 0xDC00 && unit < 0xE000 && _HighSurrogate != 0) {
		code = 0x10000 + (((char32_t)_HighSurrogate - 0xD800) << 10) + ((char32_t)unit - 0xDC00);
	}
	_HighSurrogate = 0;

	if (code == '\r') {
		code = '\n';
	}
	if ((code < 32 && code != '\n') || code == 127) {
		return(false);
	}

	bool consumed = !_Context->ProcessTextInput((Rml::Character)code);
	_Host->Mark_Overlay_Dirty();
	return(consumed);
}


bool UI_Legacy_Dialog_Visible(void)
{
	return(_Host != nullptr && _Host->Legacy_Dialog_Visible());
}


// The mouse, wheel, key and text messages a shown screen takes whole.
static bool Input_Message(UINT message)
{
	switch (message) {
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
			return(true);

		default:
			return(false);
	}
}


UIResult UI_Run_Modal(UIRmlViewClass & view, UIServiceCallback const & service)
{
	if (!_Ready) {
		return(UI_RESULT_FAILED_TO_OPEN);
	}
	if (!_FontLoaded) {
		Log("UI: %s needs OpenSans.ttf, which did not load\n", view.Document_Name());
		return(UI_RESULT_FAILED_TO_OPEN);
	}

	// A legacy dialog and an RmlUi screen never show together; the visible one takes the mouse.
	assert(!UI_Legacy_Dialog_Visible());

	// A style sheet that fails to load leaves the document usable and is reported as an error.
	int errors = _System->Error_Count();
	if (!view.Prepare(*_Context) || _System->Error_Count() != errors) {
		Log("UI: %s could not be prepared; its legacy view stays in charge\n", view.Document_Name());
		view.Release();
		return(UI_RESULT_FAILED_TO_OPEN);
	}

	view.Presenter().Refresh();
	view.Sync();

	if (_OwnedButtons != 0) {
		Drop_Presses();
	}

	char label[160];
	UIRmlViewClass * previous = _Modal;

	_Modal = &view;
	view.Show(true);
	_Host->Mark_Overlay_Dirty();
	std::snprintf(label, sizeof(label), "%s shown", view.Document_Name());
	_Render.Log_Resource_Counts(label);
	_Host->Clear_Keyboard_Queue();

	UIResult result = UI_RESULT_SESSION_ENDED;

	while (true) {
		bool ended = service();
		if (!_Ready) {
			break;
		}

		UI_Tick();
		view.Presenter().Refresh();
		view.Presenter().Drain();
		view.Sync();

		if (ended) {
			break;
		}
		if (view.Presenter().Result.has_value()) {
			result = *view.Presenter().Result;
			break;
		}

		_Host->Mark_Overlay_Dirty();
		_Host->Present_If_Dirty();
	}

	_ModalClosing = true;
	if (_OwnedButtons != 0) {
		Drop_Presses();
	}
	view.Presenter().Discard();
	view.Release();

	if (_Ready) {
		_InContext = true;
		_Context->Update();
		_InContext = false;
	}

	_Modal = previous;
	_ModalClosing = false;

	if (_Ready) {
		_Host->Mark_Overlay_Dirty();
		std::snprintf(label, sizeof(label), "%s closed", view.Document_Name());
		_Render.Log_Resource_Counts(label);
		_Host->Clear_Keyboard_Queue();
		_Host->Focus_Main_Window();
	}

	return(result);
}


bool UI_Show_Modeless(UIRmlViewClass & view)
{
	if (!_Ready || !_FontLoaded || _InContext) {
		return(false);
	}

	int errors = _System->Error_Count();
	if (!view.Prepare(*_Context) || _System->Error_Count() != errors) {
		Log("UI: %s could not be prepared; its legacy view stays in charge\n", view.Document_Name());
		view.Release();
		return(false);
	}

	view.Presenter().Refresh();
	view.Sync();
	view.Show(false);
	UI_Refresh();
	return(true);
}


void UI_Hide_Modeless(UIRmlViewClass & view)
{
	view.Release();

	if (_Ready && !_InContext) {
		_InContext = true;
		_Context->Update();
		_InContext = false;
		_Host->Mark_Overlay_Dirty();
		_Host->Present_If_Dirty();
	}
}


namespace
{

class UISystemClockClass : public UIClockClass
{
	public:
		virtual int Milliseconds(void) override
		{
			return((int)GetTickCount64());
		}
};

UISystemClockClass _Clock;

}


UIClockClass & UI_Clock(void)
{
	return(_Clock);
}


void UI_Refresh(void)
{
	if (!_Ready || _InContext) {
		return;
	}

	UI_Tick();
	_Host->Mark_Overlay_Dirty();
	_Host->Present_If_Dirty();
}


bool UI_Handle_Window_Message(HWND hwnd, UINT message, WPARAM wparam, LPARAM clientlparam)
{
	if (!_Ready || _InHook || hwnd != _Host->Main_Window()) {
		return(false);
	}

	// Another window taking the capture ends the presses the shell owns.
	if (message == WM_CAPTURECHANGED) {
		if (_OwnedButtons != 0 && (HWND)clientlparam != _Host->Main_Window()) {
			_TookCapture = false;
			if (_InContext) {
				_PendingRelease = true;
			} else {
				_InHook = true;
				Drop_Presses();
				_InHook = false;
			}
		}
		return(false);
	}

	if (message == WM_ACTIVATEAPP) {
		if (_InContext) {
			_PendingDevFocus = (wparam != 0) ? 1 : 0;
		} else {
			UIDev_Focus(wparam != 0);
		}
		if (wparam == 0 && _MouseInside) {
			if (_InContext) {
				_PendingLeave = true;
			} else {
				_InHook = true;
				_Context->ProcessMouseLeave();
				_MouseInside = false;
				_InHook = false;
			}
		}
		return(false);
	}

	if (_InContext || (_OwnedButtons == 0 && _Modal == NULL && !Documents_Visible() && !UIDev_Active())) {
		return(false);
	}

	// A closing screen has released its document; the messages it would have taken still end here.
	if (_ModalClosing) {
		return(Input_Message(message));
	}

	_InHook = true;
	bool consumed = false;

	switch (message) {
		case WM_MOUSEMOVE:
			consumed = Handle_Mouse_Move(clientlparam);
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			consumed = Handle_Button_Down(0, clientlparam);
			break;

		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			consumed = Handle_Button_Down(1, clientlparam);
			break;

		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
			consumed = Handle_Button_Down(2, clientlparam);
			break;

		case WM_LBUTTONUP:
			consumed = Handle_Button_Up(0, clientlparam);
			break;

		case WM_RBUTTONUP:
			consumed = Handle_Button_Up(1, clientlparam);
			break;

		case WM_MBUTTONUP:
			consumed = Handle_Button_Up(2, clientlparam);
			break;

		case WM_MOUSEWHEEL:
			consumed = Handle_Wheel(wparam, clientlparam);
			break;

		case WM_KEYDOWN:
		case WM_KEYUP:
			consumed = Handle_Key(message, wparam);
			break;

		case WM_CHAR:
			consumed = Handle_Char(wparam);
			break;

		default:
			break;
	}

	// A shown screen takes every mouse and key message, as a visible legacy dialog does.
	if (_Modal != NULL && Input_Message(message)) {
		consumed = true;
	}

	_InHook = false;
	return(consumed);
}


bool UI_Intercept_Pumped_Message(MSG const & msg)
{
#ifdef _DEBUG
	if (_Ready && _Host->Developer_Keys_Armed() && (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) && msg.wParam == VK_F9) {
		if (msg.message == WM_KEYDOWN && (msg.lParam & (1 << 30)) == 0) {
			_PendingToggle = true;
		}
		return(true);
	}
	if (_Ready && _Host->Developer_Keys_Armed() && (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) && msg.wParam == VK_F6) {
		if (msg.message == WM_KEYDOWN && (msg.lParam & (1 << 30)) == 0) {
			_PendingDevToggle = true;
		}
		return(true);
	}
#else
	(void)msg;
#endif
	return(false);
}
