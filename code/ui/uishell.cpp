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
#include "ui/rml/rmlview.h"
#include "ui/uihost.h"

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


// Forgets the presses the shell owns, telling the documents they ended, and gives the
// capture back when the shell took it.
void UIShellClass::Drop_Presses(void)
{
	unsigned int owned = OwnedButtons;
	unsigned int devowned = DevOwnedButtons;
	OwnedButtons = 0;
	DevOwnedButtons = 0;

	for (int button = 0; button < 3; button++) {
		if (devowned & (1u << button)) {
			UIDev_Mouse_Button(button, false);
		} else if (owned & (1u << button)) {
			Context->ProcessMouseButtonUp(button, Key_Modifiers());
		}
	}

	if (TookCapture) {
		TookCapture = false;
		Host.Release_Capture();
	}
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

	if (OwnedButtons != 0) {
		Drop_Presses();
	}

	// The documents go while the context still exists; a caller hiding its notice
	// afterwards finds nothing to do.
	for (UIRmlViewClass * view : Modeless) {
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


UIRmlViewClass * UIShellClass::Modal(void) const
{
	return(Modals.empty() ? nullptr : Modals.back());
}


int UIShellClass::Modal_Depth(void) const
{
	return((int)Modals.size());
}


bool UIShellClass::Is_Modeless_Shown(UIRmlViewClass const & view) const
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

	{
		UIReentryGuardClass updating(InContext);
		Context->Update();
		UIDev_Tick();
	}

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

	if (OwnedButtons != 0 || position.Inside) {
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


void UIShellClass::Own_Press(int button)
{
	if (OwnedButtons == 0) {
		TookCapture = Host.Take_Capture();
	}
	OwnedButtons |= (1u << button);
}


void UIShellClass::Release_Press(int button)
{
	OwnedButtons &= ~(1u << button);
	DevOwnedButtons &= ~(1u << button);
	if (OwnedButtons == 0 && TookCapture) {
		TookCapture = false;
		Host.Release_Capture();
	}
}


bool UIShellClass::Handle_Button_Down(int button, LPARAM clientlparam)
{
	UIPointerPosition position = Pointer_Position(clientlparam);

	if (UIDev_Active()) {
		UIDev_Mouse_Position(position.X, position.Y);
		if (UIDev_Mouse_Button(button, true)) {
			Own_Press(button);
			DevOwnedButtons |= (1u << button);
			Host.Mark_Overlay_Dirty();
			return(true);
		}
	}

	if (!position.Inside && OwnedButtons == 0) {
		return(false);
	}

	int modifiers = Key_Modifiers();
	Context->ProcessMouseMove(position.X, position.Y, modifiers);
	MouseInside = position.Inside;

	bool interacting = !Context->ProcessMouseButtonDown(button, modifiers);
	Host.Mark_Overlay_Dirty();

	if (!interacting) {
		return(false);
	}

	Own_Press(button);
	return(true);
}


bool UIShellClass::Handle_Button_Up(int button, LPARAM clientlparam)
{
	UIPointerPosition position = Pointer_Position(clientlparam);

	if (DevOwnedButtons & (1u << button)) {
		UIDev_Mouse_Position(position.X, position.Y);
		UIDev_Mouse_Button(button, false);
		Release_Press(button);
		Host.Mark_Overlay_Dirty();
		return(true);
	}

	// A release the overlays did not own still ends the press they saw begin.
	if (UIDev_Active()) {
		UIDev_Mouse_Button(button, false);
	}

	if ((OwnedButtons & (1u << button)) == 0) {
		return(false);
	}

	int modifiers = Key_Modifiers();

	Context->ProcessMouseMove(position.X, position.Y, modifiers);
	Context->ProcessMouseButtonUp(button, modifiers);
	MouseInside = position.Inside;
	Host.Mark_Overlay_Dirty();

	Release_Press(button);
	return(true);
}


bool UIShellClass::Handle_Wheel(WPARAM wparam, LPARAM screenlparam)
{
	int x = GET_X_LPARAM(screenlparam);
	int y = GET_Y_LPARAM(screenlparam);
	Host.Screen_To_Client(x, y);

	UIFrameRect frame = Host.Frame();
	UIPointerPosition position = UI_Client_To_Overlay(frame.X, frame.Y, frame.Width, frame.Height, x, y);

	// Windows counts wheel movement away from the user as positive; ImGui scrolls up for it
	// and RmlUi scrolls down.
	float delta = (float)(short)HIWORD(wparam) / (float)WHEEL_DELTA;

	if (UIDev_Active()) {
		UIDev_Mouse_Position(position.X, position.Y);
		if (UIDev_Mouse_Wheel(delta)) {
			Host.Mark_Overlay_Dirty();
			return(true);
		}
	}

	if (!position.Inside) {
		return(false);
	}

	bool consumed = !Context->ProcessMouseWheel(Rml::Vector2f(0.0f, -delta), Key_Modifiers());
	Host.Mark_Overlay_Dirty();
	return(consumed);
}


bool UIShellClass::Handle_Key(UINT message, WPARAM wparam)
{
	if (UIDev_Key(wparam, message == WM_KEYDOWN)) {
		Host.Mark_Overlay_Dirty();
		return(true);
	}

	Rml::Input::KeyIdentifier key = UI_Key_Identifier((int)(wparam & 0xFF));
	if (key == Rml::Input::KI_UNKNOWN) {
		return(false);
	}

	bool propagated;
	if (message == WM_KEYDOWN) {
		propagated = Context->ProcessKeyDown(key, Key_Modifiers());
	} else {
		propagated = Context->ProcessKeyUp(key, Key_Modifiers());
	}

	Host.Mark_Overlay_Dirty();
	return(!propagated || Text_Input_Focused());
}


// Windows delivers a character beyond the basic plane as two messages; the first half
// waits for the second. Carriage returns become newlines and control characters stay out.
bool UIShellClass::Handle_Char(WPARAM wparam)
{
	wchar_t unit = (wchar_t)wparam;

	if (UIDev_Character(unit)) {
		Host.Mark_Overlay_Dirty();
		return(true);
	}

	if (unit >= 0xD800 && unit < 0xDC00) {
		HighSurrogate = unit;
		return(false);
	}

	char32_t code = unit;
	if (unit >= 0xDC00 && unit < 0xE000 && HighSurrogate != 0) {
		code = 0x10000 + (((char32_t)HighSurrogate - 0xD800) << 10) + ((char32_t)unit - 0xDC00);
	}
	HighSurrogate = 0;

	if (code == '\r') {
		code = '\n';
	}
	if ((code < 32 && code != '\n') || code == 127) {
		return(false);
	}

	bool consumed = !Context->ProcessTextInput((Rml::Character)code);
	Host.Mark_Overlay_Dirty();
	return(consumed);
}


UIResult UIShellClass::Run_Modal(UIRmlViewClass & view, UIServiceCallback const & service)
{
	if (!Ready) {
		return(UI_RESULT_FAILED_TO_OPEN);
	}
	if (!FontLoaded) {
		Log("UI: %s needs OpenSans.ttf, which did not load\n", view.Document_Name());
		return(UI_RESULT_FAILED_TO_OPEN);
	}

	// A legacy dialog and an RmlUi screen never show together; the visible one takes the mouse.
	assert(!Legacy_Dialog_Visible());

	// A style sheet that fails to load leaves the document usable and is reported as an error.
	int errors = System->Error_Count();
	if (!view.Prepare(*Context) || System->Error_Count() != errors) {
		Log("UI: %s could not be prepared; its legacy view stays in charge\n", view.Document_Name());
		view.Release();
		return(UI_RESULT_FAILED_TO_OPEN);
	}

	view.Presenter().Refresh();
	view.Sync();

	if (OwnedButtons != 0) {
		Drop_Presses();
	}

	char label[160];

	Modals.push_back(&view);
	view.Show(true);
	Host.Mark_Overlay_Dirty();
	std::snprintf(label, sizeof(label), "%s shown", view.Document_Name());
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
	if (OwnedButtons != 0) {
		Drop_Presses();
	}
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
		std::snprintf(label, sizeof(label), "%s closed", view.Document_Name());
		Render->Log_Resource_Counts(label);
		Host.Clear_Keyboard_Queue();
		Host.Focus_Main_Window();
	}

	return(result);
}


bool UIShellClass::Show_Modeless(UIRmlViewClass & view)
{
	if (!Ready || !FontLoaded || InContext) {
		return(false);
	}

	int errors = System->Error_Count();
	if (!view.Prepare(*Context) || System->Error_Count() != errors) {
		Log("UI: %s could not be prepared; its legacy view stays in charge\n", view.Document_Name());
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


void UIShellClass::Hide_Modeless(UIRmlViewClass & view)
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


bool UIShellClass::Handle_Window_Message(HWND hwnd, UINT message, WPARAM wparam, LPARAM clientlparam)
{
	if (!Ready || InHook || hwnd != Host.Main_Window()) {
		return(false);
	}

	// Another window taking the capture ends the presses the shell owns.
	if (message == WM_CAPTURECHANGED) {
		if (OwnedButtons != 0 && (HWND)clientlparam != Host.Main_Window()) {
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
		if (InContext) {
			Deferred.DevFocus = (wparam != 0) ? 1 : 0;
		} else {
			UIDev_Focus(wparam != 0);
		}
		if (wparam == 0 && MouseInside) {
			if (InContext) {
				Deferred.Leave = true;
			} else {
				UIReentryGuardClass hooking(InHook);
				Context->ProcessMouseLeave();
				MouseInside = false;
			}
		}
		return(false);
	}

	if (InContext || (OwnedButtons == 0 && Modals.empty() && !Documents_Visible() && !UIDev_Active())) {
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
