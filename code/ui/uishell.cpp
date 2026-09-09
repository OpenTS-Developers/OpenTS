/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uishell.h"

#include "dbgprint.h"
#include "globals.h"
#include "goptions.h"
#include "movies.h"
#include "ui/uicoord.h"
#include "ui/uidev.h"
#include "ui/uifile.h"
#include "ui/uirender.h"
#include "ui/uisystem.h"
#include "video.h"

// windowsx.h, which win.h brings in, names two window walkers the way RmlUi names its
// element walkers.
#undef GetFirstChild
#undef GetNextSibling

#include <RmlUi/Core.h>


// The interfaces outlive Rml::Shutdown, which releases every resource through them.
static UISystemInterfaceClass _System;
static UIFileInterfaceClass _File;
static UIRenderInterfaceClass _Render;

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

static wchar_t _HighSurrogate = 0;

static Rml::Input::KeyIdentifier _KeyMap[256];


struct UIKeyMapping
{
	int VirtualKey;
	Rml::Input::KeyIdentifier Key;
};

static const UIKeyMapping _KeyMappings[] = {
	{ VK_BACK, Rml::Input::KI_BACK },
	{ VK_TAB, Rml::Input::KI_TAB },
	{ VK_CLEAR, Rml::Input::KI_CLEAR },
	{ VK_RETURN, Rml::Input::KI_RETURN },
	{ VK_PAUSE, Rml::Input::KI_PAUSE },
	{ VK_CAPITAL, Rml::Input::KI_CAPITAL },
	{ VK_ESCAPE, Rml::Input::KI_ESCAPE },
	{ VK_SPACE, Rml::Input::KI_SPACE },
	{ VK_PRIOR, Rml::Input::KI_PRIOR },
	{ VK_NEXT, Rml::Input::KI_NEXT },
	{ VK_END, Rml::Input::KI_END },
	{ VK_HOME, Rml::Input::KI_HOME },
	{ VK_LEFT, Rml::Input::KI_LEFT },
	{ VK_UP, Rml::Input::KI_UP },
	{ VK_RIGHT, Rml::Input::KI_RIGHT },
	{ VK_DOWN, Rml::Input::KI_DOWN },
	{ VK_SNAPSHOT, Rml::Input::KI_SNAPSHOT },
	{ VK_INSERT, Rml::Input::KI_INSERT },
	{ VK_DELETE, Rml::Input::KI_DELETE },
	{ VK_LWIN, Rml::Input::KI_LWIN },
	{ VK_RWIN, Rml::Input::KI_RWIN },
	{ VK_APPS, Rml::Input::KI_APPS },
	{ VK_MULTIPLY, Rml::Input::KI_MULTIPLY },
	{ VK_ADD, Rml::Input::KI_ADD },
	{ VK_SEPARATOR, Rml::Input::KI_SEPARATOR },
	{ VK_SUBTRACT, Rml::Input::KI_SUBTRACT },
	{ VK_DECIMAL, Rml::Input::KI_DECIMAL },
	{ VK_DIVIDE, Rml::Input::KI_DIVIDE },
	{ VK_NUMLOCK, Rml::Input::KI_NUMLOCK },
	{ VK_SCROLL, Rml::Input::KI_SCROLL },
	{ VK_SHIFT, Rml::Input::KI_LSHIFT },
	{ VK_CONTROL, Rml::Input::KI_LCONTROL },
	{ VK_MENU, Rml::Input::KI_LMENU },
	{ VK_LSHIFT, Rml::Input::KI_LSHIFT },
	{ VK_RSHIFT, Rml::Input::KI_RSHIFT },
	{ VK_LCONTROL, Rml::Input::KI_LCONTROL },
	{ VK_RCONTROL, Rml::Input::KI_RCONTROL },
	{ VK_LMENU, Rml::Input::KI_LMENU },
	{ VK_RMENU, Rml::Input::KI_RMENU },
	{ VK_OEM_1, Rml::Input::KI_OEM_1 },
	{ VK_OEM_PLUS, Rml::Input::KI_OEM_PLUS },
	{ VK_OEM_COMMA, Rml::Input::KI_OEM_COMMA },
	{ VK_OEM_MINUS, Rml::Input::KI_OEM_MINUS },
	{ VK_OEM_PERIOD, Rml::Input::KI_OEM_PERIOD },
	{ VK_OEM_2, Rml::Input::KI_OEM_2 },
	{ VK_OEM_3, Rml::Input::KI_OEM_3 },
	{ VK_OEM_4, Rml::Input::KI_OEM_4 },
	{ VK_OEM_5, Rml::Input::KI_OEM_5 },
	{ VK_OEM_6, Rml::Input::KI_OEM_6 },
	{ VK_OEM_7, Rml::Input::KI_OEM_7 },
	{ VK_OEM_8, Rml::Input::KI_OEM_8 },
	{ VK_OEM_102, Rml::Input::KI_OEM_102 },
};


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


// Letters, digits, the keypad digits and the function keys are contiguous in both codings.
static void Build_Key_Map(void)
{
	for (int code = 0; code < 256; code++) {
		_KeyMap[code] = Rml::Input::KI_UNKNOWN;
	}

	for (UIKeyMapping const & mapping : _KeyMappings) {
		_KeyMap[mapping.VirtualKey] = mapping.Key;
	}

	for (int letter = 0; letter < 26; letter++) {
		_KeyMap['A' + letter] = (Rml::Input::KeyIdentifier)(Rml::Input::KI_A + letter);
	}
	for (int digit = 0; digit < 10; digit++) {
		_KeyMap['0' + digit] = (Rml::Input::KeyIdentifier)(Rml::Input::KI_0 + digit);
		_KeyMap[VK_NUMPAD0 + digit] = (Rml::Input::KeyIdentifier)(Rml::Input::KI_NUMPAD0 + digit);
	}
	for (int function = 0; function < 12; function++) {
		_KeyMap[VK_F1 + function] = (Rml::Input::KeyIdentifier)(Rml::Input::KI_F1 + function);
	}
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
	VideoScaleInfo const & scale = Video_Get_Scale_Info();

	float ratio = scale.ScaleX < scale.ScaleY ? scale.ScaleX : scale.ScaleY;
	if (ratio <= 0.0f) {
		ratio = 1.0f;
	}

	_Context->SetDimensions(Rml::Vector2i(scale.DestWidth, scale.DestHeight));
	_Context->SetDensityIndependentPixelRatio(ratio);
}


static UIPointerPosition Pointer_Position(LPARAM clientlparam)
{
	VideoScaleInfo const & scale = Video_Get_Scale_Info();
	return(UI_Client_To_Overlay(scale.DestX, scale.DestY, scale.DestWidth, scale.DestHeight, GET_X_LPARAM(clientlparam), GET_Y_LPARAM(clientlparam)));
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
		if (GetCapture() == MainWindow) {
			ReleaseCapture();
		}
	}
}


#ifdef _DEBUG

static void Toggle_Test_Document(void)
{
	if (!_FontLoaded) {
		DebugString("UI: the test document needs the font, which did not load\n");
		return;
	}

	if (_TestDocument == NULL) {
		_TestDocument = _Context->LoadDocument("test.rml");
		if (_TestDocument == NULL) {
			DebugString("UI: test.rml did not load\n");
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

	Video_Mark_Overlay_Dirty();
}

#endif


bool UI_Init(void)
{
	if (_Ready) {
		return(true);
	}

	Build_Key_Map();

	if (!_Render.Init()) {
		return(false);
	}

	Rml::SetSystemInterface(&_System);
	Rml::SetFileInterface(&_File);
	Rml::SetRenderInterface(&_Render);

	if (!Rml::Initialise()) {
		DebugString("UI: RmlUi did not initialise\n");
		_Render.Shutdown();
		return(false);
	}

	VideoScaleInfo const & scale = Video_Get_Scale_Info();
	_Context = Rml::CreateContext("main", Rml::Vector2i(scale.DestWidth, scale.DestHeight));
	if (_Context == NULL) {
		DebugString("UI: the context could not be created\n");
		Rml::Shutdown();
		_Render.Shutdown();
		return(false);
	}

	Apply_Dimensions();

	_FontLoaded = Rml::LoadFontFace("OpenSans.ttf");
	if (!_FontLoaded) {
		DebugString("UI: OpenSans.ttf did not load, so no document can be shown\n");
	}

	_Ready = true;
	DebugString("UI: RmlUi %s ready over a %dx%d frame at %.2f pixels per dp\n",
				Rml::GetVersion().c_str(), scale.DestWidth, scale.DestHeight, _Context->GetDensityIndependentPixelRatio());
	return(true);
}


void UI_Shutdown(void)
{
	if (!_Ready) {
		return;
	}

	_Ready = false;

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
	_FontLoaded = false;
}


bool UI_Use_Rml(void)
{
	return(_Ready && !Options.LegacyDialogs);
}


bool UI_Screen_Shown(void)
{
	// No screen exists yet; the modal runner that shows one reports it here.
	return(false);
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

	Video_Mark_Overlay_Dirty();
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
		Video_Mark_Overlay_Dirty();
	}
	if (_CloseRequested) {
		_CloseRequested = false;
		if (_TestDocument != NULL && _TestDocument->IsVisible()) {
			_TestDocument->Hide();
			_Render.Log_Resource_Counts("test document closed");
			Video_Mark_Overlay_Dirty();
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
		Video_Mark_Overlay_Dirty();
	}
	devwasactive = devactive;

	_InTick = false;
}


void UI_Render_Overlay(void)
{
	if (!_Ready || _InContext || Movie_Is_Playing()) {
		return;
	}

	bool documents = Documents_Visible();
	bool overlays = UIDev_Active();
	if (!documents && !overlays) {
		return;
	}

	VideoScaleInfo const & scale = Video_Get_Scale_Info();
	if (scale.DestWidth <= 0 || scale.DestHeight <= 0) {
		return;
	}

	if (documents) {
		_Render.Begin_Frame(scale.DestX, scale.DestY, scale.DestWidth, scale.DestHeight);

		_InContext = true;
		_Context->Render();
		_InContext = false;
	}

	if (overlays) {
		_Render.Begin_Dev_Frame(scale.DestX, scale.DestY, scale.DestWidth, scale.DestHeight);
		UIDev_Render(_Render);
	}
}


static bool Handle_Mouse_Move(LPARAM clientlparam)
{
	UIPointerPosition position = Pointer_Position(clientlparam);

	UIDev_Mouse_Position(position.X, position.Y);
	if (UIDev_Wants_Mouse()) {
		Video_Mark_Overlay_Dirty();
		return(false);
	}

	if (_OwnedButtons != 0 || position.Inside) {
		_Context->ProcessMouseMove(position.X, position.Y, Key_Modifiers());
		_MouseInside = position.Inside;
		Video_Mark_Overlay_Dirty();
	} else if (_MouseInside) {
		_Context->ProcessMouseLeave();
		_MouseInside = false;
		Video_Mark_Overlay_Dirty();
	}

	return(false);
}


static void Own_Press(int button)
{
	if (_OwnedButtons == 0) {
		_TookCapture = (GetCapture() != MainWindow);
		if (_TookCapture) {
			SetCapture(MainWindow);
		}
	}
	_OwnedButtons |= (1u << button);
}


static void Release_Press(int button)
{
	_OwnedButtons &= ~(1u << button);
	_DevOwnedButtons &= ~(1u << button);
	if (_OwnedButtons == 0 && _TookCapture) {
		_TookCapture = false;
		if (GetCapture() == MainWindow) {
			ReleaseCapture();
		}
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
			Video_Mark_Overlay_Dirty();
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
	Video_Mark_Overlay_Dirty();

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
		Video_Mark_Overlay_Dirty();
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
	Video_Mark_Overlay_Dirty();

	Release_Press(button);
	return(true);
}


static bool Handle_Wheel(WPARAM wparam, LPARAM screenlparam)
{
	POINT point;
	point.x = GET_X_LPARAM(screenlparam);
	point.y = GET_Y_LPARAM(screenlparam);
	ScreenToClient(MainWindow, &point);

	VideoScaleInfo const & scale = Video_Get_Scale_Info();
	UIPointerPosition position = UI_Client_To_Overlay(scale.DestX, scale.DestY, scale.DestWidth, scale.DestHeight, point.x, point.y);

	// Windows counts wheel movement away from the user as positive; ImGui scrolls up for it
	// and RmlUi scrolls down.
	float delta = (float)(short)HIWORD(wparam) / (float)WHEEL_DELTA;

	if (UIDev_Active()) {
		UIDev_Mouse_Position(position.X, position.Y);
		if (UIDev_Mouse_Wheel(delta)) {
			Video_Mark_Overlay_Dirty();
			return(true);
		}
	}

	if (!position.Inside) {
		return(false);
	}

	bool consumed = !_Context->ProcessMouseWheel(Rml::Vector2f(0.0f, -delta), Key_Modifiers());
	Video_Mark_Overlay_Dirty();
	return(consumed);
}


static bool Handle_Key(UINT message, WPARAM wparam)
{
	if (UIDev_Key(wparam, message == WM_KEYDOWN)) {
		Video_Mark_Overlay_Dirty();
		return(true);
	}

	Rml::Input::KeyIdentifier key = _KeyMap[wparam & 0xFF];
	if (key == Rml::Input::KI_UNKNOWN) {
		return(false);
	}

	bool propagated;
	if (message == WM_KEYDOWN) {
		propagated = _Context->ProcessKeyDown(key, Key_Modifiers());
	} else {
		propagated = _Context->ProcessKeyUp(key, Key_Modifiers());
	}

	Video_Mark_Overlay_Dirty();
	return(!propagated || Text_Input_Focused());
}


// Windows delivers a character beyond the basic plane as two messages; the first half
// waits for the second. Carriage returns become newlines and control characters stay out.
static bool Handle_Char(WPARAM wparam)
{
	wchar_t unit = (wchar_t)wparam;

	if (UIDev_Character(unit)) {
		Video_Mark_Overlay_Dirty();
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
	Video_Mark_Overlay_Dirty();
	return(consumed);
}


bool UI_Handle_Window_Message(HWND hwnd, UINT message, WPARAM wparam, LPARAM clientlparam)
{
	if (!_Ready || _InHook || hwnd != MainWindow) {
		return(false);
	}

	// Another window taking the capture ends the presses the shell owns.
	if (message == WM_CAPTURECHANGED) {
		if (_OwnedButtons != 0 && (HWND)clientlparam != MainWindow) {
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

	if (_InContext || (_OwnedButtons == 0 && !Documents_Visible() && !UIDev_Active())) {
		return(false);
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

	_InHook = false;
	return(consumed);
}


bool UI_Intercept_Pumped_Message(MSG const & msg)
{
#ifdef _DEBUG
	if (_Ready && Debug_Flag && (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) && msg.wParam == VK_F9) {
		if (msg.message == WM_KEYDOWN && (msg.lParam & (1 << 30)) == 0) {
			_PendingToggle = true;
		}
		return(true);
	}
	if (_Ready && Debug_Flag && (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) && msg.wParam == VK_F6) {
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
