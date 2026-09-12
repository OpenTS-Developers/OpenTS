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
#include "ui/rml/rmlkeys.h"
#include "ui/rml/rmlrender.h"
#include "ui/rml/rmlsystem.h"
#include "ui/uihost.h"
#include "ui/uiview.h"

// windowsx.h, which win.h brings in, names two window walkers the way RmlUi names its
// element walkers.
#undef GetFirstChild
#undef GetNextSibling

#include <RmlUi/Core.h>

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>


namespace
{

// Sets a flag for the scope, whichever way the scope ends.
class UIReentryGuardClass
{
	public:
		explicit UIReentryGuardClass(bool & flag) :
			Flag(flag)
		{
			Flag = true;
		}

		~UIReentryGuardClass(void)
		{
			Flag = false;
		}

		UIReentryGuardClass(UIReentryGuardClass const &) = delete;
		UIReentryGuardClass & operator=(UIReentryGuardClass const &) = delete;

	private:
		bool & Flag;
};


class UISystemClockClass : public UIClockClass
{
	public:
		virtual int Milliseconds(void) override
		{
			return((int)GetTickCount64());
		}
};


int Key_Modifiers(void)
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


// The mouse, wheel, key and text messages a shown screen takes whole.
bool Input_Message(UINT message)
{
	switch (message) {
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONDBLCLK:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
			return(true);

		default:
			return(false);
	}
}


// The button a mouse message names, as RmlUi counts them: primary, secondary, middle, then
// the two side buttons.
int Message_Button(UINT message, WPARAM wparam)
{
	switch (message) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONUP:
			return(0);

		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONUP:
			return(1);

		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		case WM_MBUTTONUP:
			return(2);

		case WM_XBUTTONDOWN:
		case WM_XBUTTONDBLCLK:
		case WM_XBUTTONUP:
			return(GET_XBUTTON_WPARAM(wparam) == XBUTTON1 ? 3 : 4);

		default:
			return(-1);
	}
}


int Button_Virtual_Key(unsigned button)
{
	static int const keys[UIInputStateClass::BUTTON_COUNT] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2 };
	return(keys[button]);
}


// The modifiers are read from the keyboard state as each message arrives, so their presses
// are never owned by anyone.
bool Modifier_Key(int virtualkey)
{
	switch (virtualkey) {
		case VK_SHIFT:
		case VK_CONTROL:
		case VK_MENU:
		case VK_LSHIFT:
		case VK_RSHIFT:
		case VK_LCONTROL:
		case VK_RCONTROL:
		case VK_LMENU:
		case VK_RMENU:
			return(true);

		default:
			return(false);
	}
}

}


// The test document is a developer's check of the shell; F9 shows and hides it, and F6 the
// developer overlays. Its close button asks the shell to hide it at the next tick.
class UITestListenerClass : public Rml::EventListener
{
	public:
		explicit UITestListenerClass(UIShellClass & shell) :
			Shell(shell)
		{
		}

		virtual void ProcessEvent(Rml::Event &) override
		{
			Shell.Deferred.CloseTest = true;
		}

	private:
		UIShellClass & Shell;
};


UIShellClass::UIShellClass(UIShellHostClass & host, std::unique_ptr<UIRmlSystemClass> system, std::unique_ptr<Rml::FileInterface> file, std::unique_ptr<UIRmlRenderClass> render) :
	Host(host),
	System(std::move(system)),
	File(std::move(file)),
	Render(std::move(render))
{
}


// A shell still ready at destruction leaks its resources rather than touch a renderer that
// may already be gone.
UIShellClass::~UIShellClass(void)
{
}


void UIShellClass::Log(char const * format, ...)
{
	char buffer[512];
	va_list args;

	va_start(args, format);
	std::vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	Host.Log(buffer);
}


// While anything is shown or held, every input message is the shell's to look at.
bool UIShellClass::Active(void) const
{
	return(Input.Any_Owned() || !Modals.empty() || Documents_Visible() || UIDev_Active());
}


bool UIShellClass::Documents_Visible(void) const
{
	if (Context == nullptr) {
		return(false);
	}

	for (int index = 0; index < Context->GetNumDocuments(); index++) {
		Rml::ElementDocument * document = Context->GetDocument(index);
		if (document != nullptr && document->IsVisible()) {
			return(true);
		}
	}

	return(false);
}


bool UIShellClass::Text_Input_Focused(void) const
{
	Rml::Element * focus = Context->GetFocusElement();
	if (focus == nullptr) {
		return(false);
	}

	Rml::String const & tag = focus->GetTagName();
	return(tag == "input" || tag == "textarea");
}


void UIShellClass::Apply_Dimensions(void)
{
	UIFrameRect frame = Host.Frame();

	float ratio = frame.ScaleX < frame.ScaleY ? frame.ScaleX : frame.ScaleY;
	if (ratio <= 0.0f) {
		ratio = 1.0f;
	}

	Context->SetDimensions(Rml::Vector2i(frame.Width, frame.Height));
	Context->SetDensityIndependentPixelRatio(ratio);
}


UIPointerPosition UIShellClass::Pointer_Position(LPARAM clientlparam) const
{
	UIFrameRect frame = Host.Frame();
	return(UI_Client_To_Overlay(frame.X, frame.Y, frame.Width, frame.Height, GET_X_LPARAM(clientlparam), GET_Y_LPARAM(clientlparam)));
}


std::array<bool, UIInputStateClass::BUTTON_COUNT> UIShellClass::Physical_Buttons(void) const
{
	std::array<bool, UIInputStateClass::BUTTON_COUNT> held {};
	for (unsigned button = 0; button < held.size(); button++) {
		held[button] = Host.Key_Down(Button_Virtual_Key(button));
	}
	return(held);
}


// What is held as a screen opens or focus returns belongs to nobody now: its release is
// swallowed rather than handed to the game as the end of a press it never saw.
void UIShellClass::Quarantine_Held_Input(void)
{
	Input.Reset();

	for (unsigned key = VK_XBUTTON2 + 1; key < UIInputStateClass::KEY_COUNT; key++) {
		if (!Modifier_Key((int)key) && Host.Key_Down((int)key)) {
			Input.Press_Key(key, UI_INPUT_SUPPRESSED);
		}
	}

	std::array<bool, UIInputStateClass::BUTTON_COUNT> held = Physical_Buttons();
	for (unsigned button = 0; button < held.size(); button++) {
		if (held[button]) {
			Input.Press_Mouse(button, UI_INPUT_SUPPRESSED);
		}
	}
}


// A suppressed entry ends when the system says its key or button is up, so a release that
// went to another window cannot keep the shell active.
void UIShellClass::Reconcile_Held_Input(void)
{
	if (!Input.Any_Suppressed()) {
		return;
	}

	std::array<bool, UIInputStateClass::KEY_COUNT> keys {};
	for (unsigned key = 1; key < keys.size(); key++) {
		keys[key] = Host.Key_Down((int)key);
	}
	Input.Reconcile_Cancelled_Keys(keys);
	Input.Reconcile_Cancelled_Mouse(Physical_Buttons());
}


// Ends the presses the toolkits hold, telling them so, and suppresses every held button:
// their releases stay with the shell wherever they land.
void UIShellClass::Drop_Presses(void)
{
	for (unsigned button = 0; button < UIInputStateClass::BUTTON_COUNT; button++) {
		UIInputOwner owner = Input.Mouse_Owner(button);
		if (owner == UI_INPUT_IMGUI) {
			UIDev_Mouse_Button((int)button, false);
		} else if (owner == UI_INPUT_RML) {
			Context->ProcessMouseButtonUp((int)button, Key_Modifiers());
		}
	}

	Input.Cancel_Mouse();
	Release_UI_Capture();
	Reset_Text();
}


void UIShellClass::Release_UI_Capture(void)
{
	if (TookCapture) {
		TookCapture = false;
		Host.Release_Capture();
	}
}


void UIShellClass::Reset_Text(void)
{
	HighSurrogate = 0;
	Utf8.Reset();
	LegacyLead = 0;
}


// The pointer is the documents' while a screen is shown, a document holds a press, or it
// is over an element that takes it.
bool UIShellClass::Pointer_Owned(void) const
{
	return(!Modals.empty() || Input.Has_UI_Mouse() || (MouseInside && Context->IsMouseInteracting()));
}


bool UIShellClass::Handle_Set_Cursor(void)
{
	if (!Ready || !Pointer_Owned()) {
		return(false);
	}

	UICursor request = System->Cursor_Request();
	if (request == UI_CURSOR_ARROW) {
		return(false);
	}

	Host.Apply_Cursor(request);
	AppliedCursor = request;
	return(true);
}


// A hover that changed the request shows the new shape at once rather than at the next
// WM_SETCURSOR, which only a pointer move brings.
void UIShellClass::Apply_Cursor_Request(void)
{
	UICursor request = Pointer_Owned() ? System->Cursor_Request() : UI_CURSOR_ARROW;
	if (request == AppliedCursor) {
		return;
	}

	if (request == UI_CURSOR_ARROW) {
		Restore_Cursor();
	} else {
		Host.Apply_Cursor(request);
		AppliedCursor = request;
	}
}


void UIShellClass::Restore_Cursor(void)
{
	AppliedCursor = UI_CURSOR_ARROW;
	Host.Restore_Game_Cursor();
}


void UIShellClass::Toggle_Test_Document(void)
{
#ifdef _DEBUG
	if (!FontLoaded) {
		Log("UI: the test document needs the font, which did not load\n");
		return;
	}

	if (TestDocument == nullptr) {
		TestDocument = Context->LoadDocument("test.rml");
		if (TestDocument == nullptr) {
			Log("UI: test.rml did not load\n");
			return;
		}

		Rml::Element * close = TestDocument->GetElementById("close");
		if (close != nullptr) {
			if (TestListener == nullptr) {
				TestListener = std::make_unique<UITestListenerClass>(*this);
			}
			close->AddEventListener(Rml::EventId::Click, TestListener.get());
		}

		TestDocument->Show();
		Render->Log_Resource_Counts("test document loaded and shown");
	} else if (TestDocument->IsVisible()) {
		TestDocument->Hide();
		Render->Log_Resource_Counts("test document hidden");
	} else {
		TestDocument->Show();
		Render->Log_Resource_Counts("test document shown");
	}

	Host.Mark_Overlay_Dirty();
#endif
}


void UIShellClass::Drain_Deferred(void)
{
	DeferredWorkType work = Deferred;
	Deferred = DeferredWorkType();

#ifdef _DEBUG
	if (work.ToggleTest) {
		Toggle_Test_Document();
	}
	if (work.ToggleDev) {
		UIDev_Toggle(*Render);
		Host.Mark_Overlay_Dirty();
	}
	if (work.CloseTest) {
		if (TestDocument != nullptr && TestDocument->IsVisible()) {
			TestDocument->Hide();
			Render->Log_Resource_Counts("test document closed");
			Host.Mark_Overlay_Dirty();
		}
	}
#endif

	if (work.Resize) {
		Apply_Dimensions();
	}
	if (work.DropPresses) {
		Drop_Presses();
	}
	if (work.Leave) {
		Context->ProcessMouseLeave();
		MouseInside = false;
	}
	if (work.DevFocus >= 0) {
		UIDev_Focus(work.DevFocus != 0);
	}
}


bool UIShellClass::Init(void)
{
	if (Ready) {
		return(true);
	}

	if (!Render->Init()) {
		return(false);
	}

	Rml::SetSystemInterface(System.get());
	if (File != nullptr) {
		Rml::SetFileInterface(File.get());
	}
	Rml::SetRenderInterface(Render.get());

	if (!Rml::Initialise()) {
		Log("UI: RmlUi did not initialise\n");
		Render->Shutdown();
		return(false);
	}

	UIFrameRect frame = Host.Frame();
	Context = Rml::CreateContext("main", Rml::Vector2i(frame.Width, frame.Height));
	if (Context == nullptr) {
		Log("UI: the context could not be created\n");
		Rml::Shutdown();
		Render->Shutdown();
		return(false);
	}

	Apply_Dimensions();

	FontLoaded = Rml::LoadFontFace("OpenSans.ttf");
	if (!FontLoaded) {
		Log("UI: OpenSans.ttf did not load, so no document can be shown\n");
	}

	Ready = true;
	Log("UI: RmlUi %s ready over a %dx%d frame at %.2f pixels per dp\n",
		Rml::GetVersion().c_str(), frame.Width, frame.Height, Context->GetDensityIndependentPixelRatio());
	return(true);
}


void UIShellClass::Shutdown(void)
{
	if (!Ready) {
		return;
	}

	Ready = false;
	Modals.clear();
	ModalClosing = false;

	if (Input.Gesture_Owner() != UI_INPUT_NONE) {
		Drop_Presses();
	}
	Input.Reset();
	Reset_Text();

	// The documents go while the context still exists; a caller hiding its notice
	// afterwards finds nothing to do.
	for (UIViewClass * view : Modeless) {
		view->Release();
	}
	Modeless.clear();

	UIDev_Shutdown(*Render);

	TestDocument = nullptr;
	Deferred = DeferredWorkType();

	Rml::RemoveContext("main");
	Context = nullptr;

	Rml::Shutdown();
	Render->Shutdown();
	FontLoaded = false;
}


bool UIShellClass::Use_Rml(void) const
{
	return(Ready && !Host.Legacy_Dialogs_Requested());
}


bool UIShellClass::Screen_Shown(void) const
{
	return(!Modals.empty() || ModalClosing);
}


bool UIShellClass::Legacy_Dialog_Visible(void) const
{
	return(Host.Legacy_Dialog_Visible());
}


UIViewClass * UIShellClass::Modal(void) const
{
	return(Modals.empty() ? nullptr : Modals.back());
}


int UIShellClass::Modal_Depth(void) const
{
	return((int)Modals.size());
}


bool UIShellClass::Is_Modeless_Shown(UIViewClass const & view) const
{
	return(std::find(Modeless.begin(), Modeless.end(), &view) != Modeless.end());
}


void UIShellClass::On_Video_Change(void)
{
	if (!Ready) {
		return;
	}

	if (InContext) {
		Deferred.Resize = true;
	} else {
		Apply_Dimensions();
	}

	Host.Mark_Overlay_Dirty();
}


void UIShellClass::Tick(void)
{
	if (!Ready || InTick || InContext) {
		return;
	}

	UIReentryGuardClass ticking(InTick);

	Drain_Deferred();
	Reconcile_Held_Input();

	{
		UIReentryGuardClass updating(InContext);
		Context->Update();
		UIDev_Tick();
	}

	Apply_Cursor_Request();

	// An overlay closed from inside its own frame still needs one present to clear.
	bool devactive = UIDev_Active();
	if (Documents_Visible() || devactive || DevWasActive) {
		Host.Mark_Overlay_Dirty();
	}
	DevWasActive = devactive;
}


void UIShellClass::Render_Overlay(void)
{
	if (!Ready || InContext || Host.Movie_Playing()) {
		return;
	}

	bool documents = Documents_Visible();
	bool overlays = UIDev_Active();
	if (!documents && !overlays) {
		return;
	}

	UIFrameRect frame = Host.Frame();
	if (frame.Width <= 0 || frame.Height <= 0) {
		return;
	}

	if (documents) {
		Render->Begin_Frame(frame.X, frame.Y, frame.Width, frame.Height);

		UIReentryGuardClass rendering(InContext);
		Context->Render();
	}

	if (overlays) {
		Render->Begin_Dev_Frame(frame.X, frame.Y, frame.Width, frame.Height);
		UIDev_Render(*Render);
	}

	Drain_Deferred();
}


bool UIShellClass::Handle_Mouse_Move(LPARAM clientlparam)
{
	UIPointerPosition position = Pointer_Position(clientlparam);

	UIDev_Mouse_Position(position.X, position.Y);
	if (UIDev_Wants_Mouse()) {
		Host.Mark_Overlay_Dirty();
		return(false);
	}

	if (Input.Has_UI_Mouse() || position.Inside) {
		Context->ProcessMouseMove(position.X, position.Y, Key_Modifiers());
		MouseInside = position.Inside;
		Host.Mark_Overlay_Dirty();
	} else if (MouseInside) {
		Context->ProcessMouseLeave();
		MouseInside = false;
		Host.Mark_Overlay_Dirty();
	}

	return(false);
}


// The press's owner is decided here and kept until its release: the overlays first, then
// a shown screen, which takes every press, then whichever document the pointer is over.
bool UIShellClass::Handle_Button_Down(int button, LPARAM clientlparam)
{
	Input.Reconcile_Cancelled_Mouse(Physical_Buttons());

	UIPointerPosition position = Pointer_Position(clientlparam);
	int modifiers = Key_Modifiers();
	bool haduimouse = Input.Has_UI_Mouse();
	UIInputOwner owner = UI_INPUT_GAME;

	if (UIDev_Active()) {
		UIDev_Mouse_Position(position.X, position.Y);
		if (UIDev_Mouse_Button(button, true)) {
			owner = UI_INPUT_IMGUI;
		}
	}

	if (owner == UI_INPUT_GAME) {
		bool feed = position.Inside || Input.Gesture_Owner() != UI_INPUT_NONE;
		if (feed) {
			Context->ProcessMouseMove(position.X, position.Y, modifiers);
			MouseInside = position.Inside;
		}
		if (!Modals.empty()) {
			if (feed) {
				Context->ProcessMouseButtonDown(button, modifiers);
			}
			owner = UI_INPUT_RML;
		} else if (feed && !Context->ProcessMouseButtonDown(button, modifiers)) {
			owner = UI_INPUT_RML;
		}
	}

	UIInputOwner latched = Input.Press_Mouse((unsigned)button, owner);
	if (!haduimouse && Input.Has_UI_Mouse() && !TookCapture) {
		TookCapture = Host.Take_Capture();
	}

	Host.Mark_Overlay_Dirty();
	return(UI_Consumes_Input(latched));
}


bool UIShellClass::Handle_Button_Up(int button, LPARAM clientlparam)
{
	UIPointerPosition position = Pointer_Position(clientlparam);
	UIInputOwner owner = Input.Release_Mouse((unsigned)button);

	if (owner == UI_INPUT_IMGUI) {
		UIDev_Mouse_Position(position.X, position.Y);
		UIDev_Mouse_Button(button, false);
	} else {
		// A release the overlays did not own still ends the press they saw begin.
		if (UIDev_Active()) {
			UIDev_Mouse_Button(button, false);
		}
		if (owner == UI_INPUT_RML) {
			int modifiers = Key_Modifiers();
			Context->ProcessMouseMove(position.X, position.Y, modifiers);
			Context->ProcessMouseButtonUp(button, modifiers);
			MouseInside = position.Inside;
		}
	}

	if (!Input.Has_UI_Mouse()) {
		Release_UI_Capture();
	}

	Host.Mark_Overlay_Dirty();
	return(UI_Consumes_Input(owner));
}


bool UIShellClass::Handle_Wheel(WPARAM wparam, LPARAM screenlparam, bool horizontal)
{
	int x = GET_X_LPARAM(screenlparam);
	int y = GET_Y_LPARAM(screenlparam);
	Host.Screen_To_Client(x, y);

	UIFrameRect frame = Host.Frame();
	UIPointerPosition position = UI_Client_To_Overlay(frame.X, frame.Y, frame.Width, frame.Height, x, y);

	// Windows counts wheel movement away from the user as positive; ImGui scrolls up for it
	// and RmlUi scrolls down. Sideways, both count rightward as positive.
	float delta = (float)(short)HIWORD(wparam) / (float)WHEEL_DELTA;

	if (!horizontal && UIDev_Active()) {
		UIDev_Mouse_Position(position.X, position.Y);
		if (UIDev_Mouse_Wheel(delta)) {
			Host.Mark_Overlay_Dirty();
			return(true);
		}
	}

	if (!position.Inside) {
		return(false);
	}

	Rml::Vector2f movement = horizontal ? Rml::Vector2f(delta, 0.0f) : Rml::Vector2f(0.0f, -delta);
	bool consumed = !Context->ProcessMouseWheel(movement, Key_Modifiers());
	Host.Mark_Overlay_Dirty();
	return(consumed);
}


// A fresh press decides its owner; a repeat and the release follow it. Keys the toolkits do
// not own are still shown to RmlUi so its modifier state keeps up, as before.
bool UIShellClass::Handle_Key(UINT message, WPARAM wparam, LPARAM lparam)
{
	unsigned virtualkey = (unsigned)(wparam & 0xFF);
	int modifiers = Key_Modifiers();
	Rml::Input::KeyIdentifier key = UI_Key_Identifier((int)virtualkey);

	if (message == WM_KEYUP) {
		UIInputOwner owner = Input.Release_Key(virtualkey);
		if (owner == UI_INPUT_IMGUI) {
			UIDev_Key(wparam, false);
		} else if (owner != UI_INPUT_SUPPRESSED) {
			if (UIDev_Active()) {
				UIDev_Key(wparam, false);
			}
			if (key != Rml::Input::KI_UNKNOWN) {
				Context->ProcessKeyUp(key, modifiers);
			}
		}
		Host.Mark_Overlay_Dirty();
		return(UI_Consumes_Input(owner));
	}

	bool repeat = (lparam & (1 << 30)) != 0;
	if (!repeat) {
		Input.Release_Key(virtualkey);
	}

	UIInputOwner owner = Input.Key_Owner(virtualkey);
	if (owner != UI_INPUT_NONE) {
		if (owner == UI_INPUT_IMGUI) {
			UIDev_Key(wparam, true);
		} else if (owner == UI_INPUT_RML && key != Rml::Input::KI_UNKNOWN) {
			Context->ProcessKeyDown(key, modifiers);
		}
	} else {
		if (UIDev_Key(wparam, true)) {
			owner = UI_INPUT_IMGUI;
		} else if (!Modals.empty()) {
			if (key != Rml::Input::KI_UNKNOWN) {
				Context->ProcessKeyDown(key, modifiers);
			}
			owner = UI_INPUT_RML;
		} else if (key == Rml::Input::KI_UNKNOWN) {
			owner = UI_INPUT_GAME;
		} else {
			bool propagated = Context->ProcessKeyDown(key, modifiers);
			owner = (!propagated || Text_Input_Focused()) ? UI_INPUT_RML : UI_INPUT_GAME;
		}
		Input.Press_Key(virtualkey, owner);
	}

	Host.Mark_Overlay_Dirty();
	return(UI_Consumes_Input(owner));
}


bool UIShellClass::Handle_Char(WPARAM wparam)
{
	if (Host.Window_Is_Unicode()) {
		return(Feed_Text_Unit((wchar_t)wparam));
	}
	return(Feed_Text_Byte((unsigned char)wparam));
}


// A character beyond the basic plane arrives as two units; the first waits for the second,
// and either half on its own becomes U+FFFD.
bool UIShellClass::Feed_Text_Unit(wchar_t unit)
{
	if (unit >= 0xD800 && unit < 0xDC00) {
		bool consumed = false;
		if (HighSurrogate != 0) {
			consumed = Handle_Text(0xFFFD);
		}
		HighSurrogate = unit;
		return(consumed);
	}

	char32_t code = unit;
	if (unit >= 0xDC00 && unit < 0xE000) {
		code = (HighSurrogate != 0) ? 0x10000 + (((char32_t)HighSurrogate - 0xD800) << 10) + ((char32_t)unit - 0xDC00) : 0xFFFD;
	} else if (HighSurrogate != 0) {
		Handle_Text(0xFFFD);
	}
	HighSurrogate = 0;

	return(Handle_Text(code));
}


// A narrow window delivers text one byte per message: UTF-8 under the UTF-8 code page, else
// the code page's own single and double bytes.
bool UIShellClass::Feed_Text_Byte(unsigned char byte)
{
	unsigned int codepage = Host.Text_Code_Page();

	if (codepage == CP_UTF8) {
		UIInputText text = Utf8.Feed(byte);
		bool consumed = false;
		for (unsigned index = 0; index < text.Count; index++) {
			consumed = Handle_Text(text.Codepoints[index]) || consumed;
		}
		return(consumed);
	}

	char bytes[2];
	int count;
	if (LegacyLead != 0) {
		bytes[0] = (char)LegacyLead;
		bytes[1] = (char)byte;
		count = 2;
		LegacyLead = 0;
	} else if (IsDBCSLeadByteEx(codepage, byte)) {
		LegacyLead = byte;
		return(false);
	} else {
		bytes[0] = (char)byte;
		count = 1;
	}

	wchar_t wide[2];
	int converted = MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS, bytes, count, wide, 2);
	char32_t code = 0xFFFD;
	if (converted == 1) {
		code = wide[0];
	} else if (converted == 2 && wide[0] >= 0xD800 && wide[0] < 0xDC00 && wide[1] >= 0xDC00 && wide[1] < 0xE000) {
		code = 0x10000 + (((char32_t)wide[0] - 0xD800) << 10) + ((char32_t)wide[1] - 0xDC00);
	}
	return(Handle_Text(code));
}


// Carriage returns become newlines and control characters stay out, as before.
bool UIShellClass::Handle_Text(char32_t code)
{
	if (code == '\r') {
		code = '\n';
	}
	if ((code < 32 && code != '\n') || code == 127) {
		return(false);
	}

	if (UIDev_Active()) {
		bool wanted;
		if (code > 0xFFFF) {
			char32_t offset = code - 0x10000;
			wanted = UIDev_Character((wchar_t)(0xD800 + (offset >> 10)));
			wanted = UIDev_Character((wchar_t)(0xDC00 + (offset & 0x3FF))) || wanted;
		} else {
			wanted = UIDev_Character((wchar_t)code);
		}
		if (wanted) {
			Host.Mark_Overlay_Dirty();
			return(true);
		}
	}

	bool consumed = !Context->ProcessTextInput((Rml::Character)code);
	Host.Mark_Overlay_Dirty();
	return(consumed);
}


UIResult UIShellClass::Run_Modal(UIViewClass & view, UIServiceCallback const & service)
{
	if (!Ready) {
		return(UI_RESULT_FAILED_TO_OPEN);
	}
	if (!FontLoaded) {
		Log("UI: %s needs OpenSans.ttf, which did not load\n", view.Name());
		return(UI_RESULT_FAILED_TO_OPEN);
	}

	// A legacy dialog and an RmlUi screen never show together; the visible one takes the mouse.
	assert(!Legacy_Dialog_Visible());

	// A style sheet that fails to load leaves the document usable and is reported as an error.
	int errors = System->Error_Count();
	if (!view.Prepare(*this) || System->Error_Count() != errors) {
		Log("UI: %s could not be prepared; its legacy view stays in charge\n", view.Name());
		view.Release();
		return(UI_RESULT_FAILED_TO_OPEN);
	}

	view.Presenter().Refresh();
	view.Sync();

	if (Input.Gesture_Owner() != UI_INPUT_NONE) {
		Drop_Presses();
	}
	Quarantine_Held_Input();
	Reset_Text();

	char label[160];

	Modals.push_back(&view);
	view.Show(true);
	Host.Mark_Overlay_Dirty();
	std::snprintf(label, sizeof(label), "%s shown", view.Name());
	Render->Log_Resource_Counts(label);
	Host.Clear_Keyboard_Queue();

	UIResult result = UI_RESULT_SESSION_ENDED;

	while (true) {
		bool ended = service();
		if (!Ready) {
			break;
		}

		Tick();
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

		Host.Mark_Overlay_Dirty();
		Host.Present_If_Dirty();
	}

	ModalClosing = true;
	if (Ready && Input.Gesture_Owner() != UI_INPUT_NONE) {
		Drop_Presses();
	}
	Input.Cancel_UI();
	Reset_Text();
	view.Presenter().Discard();
	view.Release();

	if (Ready) {
		UIReentryGuardClass updating(InContext);
		Context->Update();
	}

	// A shutdown inside the loop has already emptied the stack.
	if (!Modals.empty() && Modals.back() == &view) {
		Modals.pop_back();
	}
	ModalClosing = false;

	if (Ready) {
		Host.Mark_Overlay_Dirty();
		std::snprintf(label, sizeof(label), "%s closed", view.Name());
		Render->Log_Resource_Counts(label);
		System->Reset_Cursor_Request();
		Restore_Cursor();
		Host.Clear_Keyboard_Queue();
		Host.Focus_Main_Window();
	}

	return(result);
}


bool UIShellClass::Show_Modeless(UIViewClass & view)
{
	if (!Ready || !FontLoaded || InContext) {
		return(false);
	}

	int errors = System->Error_Count();
	if (!view.Prepare(*this) || System->Error_Count() != errors) {
		Log("UI: %s could not be prepared; its legacy view stays in charge\n", view.Name());
		view.Release();
		return(false);
	}

	view.Presenter().Refresh();
	view.Sync();
	view.Show(false);
	Modeless.push_back(&view);
	Refresh();
	return(true);
}


void UIShellClass::Hide_Modeless(UIViewClass & view)
{
	Modeless.erase(std::remove(Modeless.begin(), Modeless.end(), &view), Modeless.end());
	view.Release();

	if (Ready && !InContext) {
		{
			UIReentryGuardClass updating(InContext);
			Context->Update();
		}
		Host.Mark_Overlay_Dirty();
		Host.Present_If_Dirty();
	}
}


UIClockClass & UIShellClass::Clock(void)
{
	static UISystemClockClass clock;
	return(clock);
}


void UIShellClass::Refresh(void)
{
	if (!Ready || InContext) {
		return;
	}

	Tick();
	Host.Mark_Overlay_Dirty();
	Host.Present_If_Dirty();
}


bool UIShellClass::Handle_Window_Message(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (!Ready || InHook || hwnd != Host.Main_Window()) {
		return(false);
	}

	// Another window taking the capture, or the system cancelling it, ends the presses the
	// shell holds; the window no longer has the capture to give back.
	if (message == WM_CAPTURECHANGED || message == WM_CANCELMODE) {
		bool lost = (message == WM_CANCELMODE) || (HWND)lparam != Host.Main_Window();
		if (lost && Input.Gesture_Owner() != UI_INPUT_NONE) {
			TookCapture = false;
			if (InContext) {
				Deferred.DropPresses = true;
			} else {
				UIReentryGuardClass hooking(InHook);
				Drop_Presses();
			}
		}
		return(false);
	}

	if (message == WM_ACTIVATEAPP) {
		bool activated = (wparam != 0);
		if (InContext) {
			Deferred.DevFocus = activated ? 1 : 0;
		} else {
			UIDev_Focus(activated);
		}
		if (!activated) {
			if (MouseInside) {
				if (InContext) {
					Deferred.Leave = true;
				} else {
					UIReentryGuardClass hooking(InHook);
					Context->ProcessMouseLeave();
					MouseInside = false;
				}
			}
			Input.Cancel_All();
			Release_UI_Capture();
			Reset_Text();
		} else if (Active()) {
			Quarantine_Held_Input();
			Reset_Text();
		}
		return(false);
	}

	if (message == WM_INPUTLANGCHANGE) {
		Reset_Text();
		return(false);
	}

	if (InContext || !Active()) {
		return(false);
	}

	// A closing screen has released its document; the messages it would have taken still end here.
	if (ModalClosing) {
		return(Input_Message(message));
	}

	UIReentryGuardClass hooking(InHook);
	bool consumed = false;

	switch (message) {
		case WM_MOUSEMOVE:
			consumed = Handle_Mouse_Move(lparam);
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONDBLCLK:
			consumed = Handle_Button_Down(Message_Button(message, wparam), lparam);
			break;

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
			consumed = Handle_Button_Up(Message_Button(message, wparam), lparam);
			break;

		case WM_MOUSEWHEEL:
			consumed = Handle_Wheel(wparam, lparam, false);
			break;

		case WM_MOUSEHWHEEL:
			consumed = Handle_Wheel(wparam, lparam, true);
			break;

		case WM_KEYDOWN:
		case WM_KEYUP:
			consumed = Handle_Key(message, wparam, lparam);
			break;

		case WM_CHAR:
			consumed = Handle_Char(wparam);
			break;

		default:
			break;
	}

	// A shown screen takes every mouse and key message, as a visible legacy dialog does.
	if (!Modals.empty() && Input_Message(message)) {
		consumed = true;
	}

	return(consumed);
}


bool UIShellClass::Intercept_Pumped_Message(MSG const & msg)
{
#ifdef _DEBUG
	if (Ready && Host.Developer_Keys_Armed() && (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) && msg.wParam == VK_F9) {
		if (msg.message == WM_KEYDOWN && (msg.lParam & (1 << 30)) == 0) {
			Deferred.ToggleTest = true;
		}
		return(true);
	}
	if (Ready && Host.Developer_Keys_Armed() && (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) && msg.wParam == VK_F6) {
		if (msg.message == WM_KEYDOWN && (msg.lParam & (1 << 30)) == 0) {
			Deferred.ToggleDev = true;
		}
		return(true);
	}
#else
	(void)msg;
#endif
	return(false);
}
