/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uishell.h"

#include "platform/windowevent.hh"
#include "ui/dev/uidev.h"
#include "ui/rml/rmlfont.h"
#include "ui/rml/rmlkeys.h"
#include "ui/rml/rmlrender.h"
#include "ui/rml/rmlsystem.h"
#include "ui/rml/rmltexture.h"
#include "ui/uihost.h"
#include "ui/uireveal.h"
#include "ui/uiview.h"

// windowsx.h defines macros with the names of these RmlUi Element methods.
#undef GetFirstChild
#undef GetNextSibling

#include "ui/rml/rmlsurface.h"

#include <RmlUi/Core.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdarg>
#include <cstdio>


namespace
{

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


class UIHostClockClass : public UIClockClass
{
	public:
		explicit UIHostClockClass(UIShellHostClass & host) :
			Host(host)
		{
		}

		virtual int Milliseconds(void) override
		{
			return(Host.Milliseconds());
		}

	private:
		UIShellHostClass & Host;
};


bool Input_Event(WindowEventType type)
{
	switch (type) {
		case WINDOW_EVENT_MOUSE_MOVE:
		case WINDOW_EVENT_MOUSE_DOWN:
		case WINDOW_EVENT_MOUSE_UP:
		case WINDOW_EVENT_MOUSE_WHEEL:
		case WINDOW_EVENT_KEY_DOWN:
		case WINDOW_EVENT_KEY_UP:
		case WINDOW_EVENT_TEXT:
			return(true);

		default:
			return(false);
	}
}


int Button_Virtual_Key(unsigned button)
{
	static int const keys[UIInputStateClass::BUTTON_COUNT] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2 };
	return(keys[button]);
}


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


UIShellClass::UIShellClass(UIShellHostClass & host, std::unique_ptr<UIRmlSystemClass> system, std::unique_ptr<Rml::FileInterface> file, std::unique_ptr<UIRmlRenderClass> render) :
	Host(host),
	System(std::move(system)),
	File(std::move(file)),
	Render(std::move(render)),
	Fonts(std::make_unique<UIFontEngineClass>())
{
}


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

	Fonts->Set_Reference_Scale(ratio);
	PixelRatio = ratio;
	Apply_Font_Policy();

	int magnification = Host.Art_Magnification();
	if (magnification != ArtMagnification) {
		ArtMagnification = magnification;
		Render->Set_Art_Magnification(magnification);
		Fonts->Set_Magnification(magnification);
		Rml::ReleaseTextures(Render.get());
	}
}


void UIShellClass::Drop_Cached_Files(void)
{
	Rml::ReleaseTextures(Render.get());

	Rml::Factory::ClearStyleSheetCache();
	Rml::Factory::ClearTemplateCache();

	DialogFontTried = false;
	Ensure_Dialog_Font();

	UIReentryGuardClass updating(InContext);
	Rml::ReleaseFontResources();
}


UIPointerPosition UIShellClass::Pointer_Position(int x, int y) const
{
	UIFrameRect frame = Host.Frame();
	return(UI_Client_To_Overlay(frame.X, frame.Y, frame.Width, frame.Height, x, y));
}


std::array<bool, UIInputStateClass::BUTTON_COUNT> UIShellClass::Physical_Buttons(void) const
{
	std::array<bool, UIInputStateClass::BUTTON_COUNT> held {};
	for (unsigned button = 0; button < held.size(); button++) {
		held[button] = Host.Key_Down(Button_Virtual_Key(button));
	}
	return(held);
}


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


void UIShellClass::Drop_Presses(void)
{
	Context->ProcessMouseLeave();
	MouseInside = false;

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
}


void UIShellClass::Release_UI_Capture(void)
{
	if (TookCapture) {
		TookCapture = false;
		Host.Release_Capture();
	}
}


int UIShellClass::Key_Modifiers(void) const
{
	int modifiers = 0;

	if (Host.Key_Down(VK_SHIFT)) {
		modifiers |= Rml::Input::KM_SHIFT;
	}
	if (Host.Key_Down(VK_CONTROL)) {
		modifiers |= Rml::Input::KM_CTRL;
	}
	if (Host.Key_Down(VK_MENU)) {
		modifiers |= Rml::Input::KM_ALT;
	}
	if (Host.Key_Toggled(VK_CAPITAL)) {
		modifiers |= Rml::Input::KM_CAPSLOCK;
	}
	if (Host.Key_Toggled(VK_NUMLOCK)) {
		modifiers |= Rml::Input::KM_NUMLOCK;
	}

	return(modifiers);
}


bool UIShellClass::Pointer_Owned(void) const
{
	return(!Modals.empty() || Input.Has_UI_Mouse() || (MouseInside && Context->IsMouseInteracting()));
}


/// <returns>True when the interface set the cursor, and the caller should leave it
/// alone.</returns>
bool UIShellClass::Handle_Set_Cursor(void)
{
	if (!Ready || !Pointer_Owned()) {
		return(false);
	}

	UICursor request = System->Cursor_Request();
	Host.Apply_Cursor(request);
	AppliedCursor = request;
	return(true);
}


void UIShellClass::Apply_Cursor_Request(void)
{
	if (!Pointer_Owned()) {
		if (AppliedCursor.has_value()) {
			Restore_Cursor();
		}
		return;
	}

	UICursor request = System->Cursor_Request();
	if (AppliedCursor != request) {
		Host.Apply_Cursor(request);
		AppliedCursor = request;
	}
}


void UIShellClass::Restore_Cursor(void)
{
	AppliedCursor.reset();
	Host.Restore_Game_Cursor();
}


void UIShellClass::Drain_Deferred(void)
{
	DeferredWorkType work = Deferred;
	Deferred = DeferredWorkType();

#ifdef _DEBUG
	if (work.ToggleDev) {
		UIDev_Toggle(*Render);
		Host.Mark_Overlay_Dirty();
	}
#endif

	if (work.Resize) {
		Apply_Dimensions();
	}
	if (work.DropFiles) {
		Drop_Cached_Files();
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


static char const * const UI_SHEET_FONT_FAMILY = "dlgsys";
static char const * const UI_SANS_FONT_FAMILY = "dlg-sans";
static char const * const UI_SANS_RASTER_FILES[] = {
	"sserife.fon",		// Western
	"sserifee.fon",		// Central European
	"sserifer.fon",		// Cyrillic
	"sserifeg.fon",		// Greek
	"sserifet.fon",		// Turkish
	"ssee1257.fon",		// Baltic
};
static char const * const UI_SANS_FONT_FILE = "micross.ttf";
static char const * const UI_SHIPPED_FONT_FILE = "Arimo.ttf";

static char const * const UI_REVEAL_SOUND = "EMBLEM.AUD";
static const float UI_REVEAL_VOLUME = 64.0f / 255.0f;

static char const * const UI_REVEAL_DONE = "UI: %s opened over %d passes in %d ms\n";


bool UIShellClass::Load_Sheet_Font(char const * family)
{
	std::string index = std::string(family) + "i.pcx";
	std::string alpha = std::string(family) + "a.pcx";

	UIImageIndexed indexsheet;
	UIImageIndexed alphasheet;
	if (!UI_Load_Indexed_Image(index.c_str(), indexsheet) || !UI_Load_Indexed_Image(alpha.c_str(), alphasheet)) {
		return(false);
	}

	return(Fonts->Load_Sheets(family, indexsheet, alphasheet));
}


void UIShellClass::Register_Fonts(void)
{
	Fonts->Set_Fallback(Rml::GetFontEngineInterface());
	Rml::SetFontEngineInterface(Fonts.get());

	bool sansloaded = false;
	std::vector<UIRasterStrike> strikes;
	for (char const * const name : UI_SANS_RASTER_FILES) {
		std::string path = Host.System_Font_Path(name);
		std::vector<std::uint8_t> bytes;
		if (path.empty() || !UI_Read_File(path.c_str(), bytes)) {
			continue;
		}

		std::vector<UIRasterStrike> cut;
		if (!UI_Read_Raster_Font(std::span<std::uint8_t const>(bytes.data(), bytes.size()), cut)) {
			continue;
		}

		if (strikes.empty()) {
			strikes = std::move(cut);
			Log("UI: %s answers for %s with %d strikes\n", name, UI_SANS_FONT_FAMILY, (int)strikes.size());
			continue;
		}

		int added = UI_Merge_Raster_Strikes(strikes, cut);
		if (added > 0) {
			Log("UI: %s adds %d characters to %s\n", name, added, UI_SANS_FONT_FAMILY);
		}
	}

	if (!strikes.empty()) {
		sansloaded = Fonts->Load_Strikes(UI_SANS_FONT_FAMILY, strikes);
	}

	std::string sans = Host.System_Font_Path(UI_SANS_FONT_FILE);
	if (!sans.empty() && UI_Read_File(sans.c_str(), SystemFontData)) {
		sansloaded = Rml::LoadFontFace(Rml::Span<const Rml::byte>(SystemFontData.data(), SystemFontData.size()), UI_SANS_FONT_FAMILY, Rml::Style::FontStyle::Normal) || sansloaded;
	}

	FontLoaded = Rml::LoadFontFace(UI_SHIPPED_FONT_FILE);
	if (!FontLoaded) {
		Log("UI: %s did not load, so no document can be shown\n", UI_SHIPPED_FONT_FILE);
		return;
	}

	if (!sansloaded) {
		Log("UI: neither %s nor %s is on this machine, so %s is the shipped face\n",
			UI_SANS_RASTER_FILES[0], UI_SANS_FONT_FILE, UI_SANS_FONT_FAMILY);
	}

	Rml::LoadFontFace(UI_SHIPPED_FONT_FILE, UI_SANS_FONT_FAMILY, Rml::Style::FontStyle::Normal);

	Rml::LoadFontFace(UI_SHIPPED_FONT_FILE, UI_SHEET_FONT_FAMILY, Rml::Style::FontStyle::Normal);
}


void UIShellClass::Apply_Font_Policy(void)
{
	Fonts->Set_Use_Strikes(Host.Bitmap_System_Font() && std::abs(PixelRatio - 1.0f) < 0.001f);
}


void UIShellClass::Ensure_Dialog_Font(void)
{
	if (DialogFontTried || !Ready) {
		return;
	}

	DialogFontTried = true;
	if (!Load_Sheet_Font(UI_SHEET_FONT_FAMILY)) {
		Log("UI: the %s sheets are not in the game's art, so %s stays the shipped face\n", UI_SHEET_FONT_FAMILY, UI_SHEET_FONT_FAMILY);
	}
}


bool UIShellClass::Advance_Reveal(UIViewClass & view, float full, int start, float & shown)
{
	shown = UI_Reveal_Width(full, Context->GetDensityIndependentPixelRatio(), Clock().Milliseconds() - start, shown);

	if (shown >= full) {
		view.Reveal_Done();
		return(false);
	}

	view.Reveal_To(shown);
	return(true);
}


void UIShellClass::Advance_Shown_Reveal(UIViewClass & view)
{
	if (!Revealing) {
		return;
	}

	RevealPasses++;
	Revealing = Advance_Reveal(view, RevealWidth, RevealStart, RevealShown);
	if (!Revealing) {
		Log(UI_REVEAL_DONE, view.Name(), RevealPasses, Clock().Milliseconds() - RevealStart);
	}
}


/// <summary>
/// Advances the modal screen from the game's own message loops, where Run_Modal is not the
/// one driving it.
/// </summary>
void UIShellClass::Serve_Shown_Screen(void)
{
	if (!Ready || Modals.empty() || ModalClosing || InContext || InTick) {
		return;
	}

	Advance_Shown_Reveal(*Modals.back());

	Tick();
	Host.Mark_Overlay_Dirty();
	Host.Present_If_Dirty();
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
		Log("UI: RmlUi did not initialize\n");
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

	UI_Register_Surface_Element();
	Register_Fonts();

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
	Services.clear();
	ModalClosing = false;

	if (Input.Gesture_Owner() != UI_INPUT_NONE) {
		Drop_Presses();
	}
	Input.Reset();
	if (AppliedCursor.has_value()) {
		Restore_Cursor();
	}

	for (UIViewClass * view : Modeless) {
		view->Release();
	}
	Modeless.clear();

	UIDev_Shutdown(*Render);

	Deferred = DeferredWorkType();

	Rml::RemoveContext("main");
	Context = nullptr;

	Rml::Shutdown();
	Render->Shutdown();
	FontLoaded = false;
}


bool UIShellClass::Screen_Shown(void) const
{
	return(!Modals.empty() || ModalClosing);
}



UIViewClass * UIShellClass::Modal(void) const
{
	return(Modals.empty() ? nullptr : Modals.back());
}


UIServiceCallback const * UIShellClass::Running_Service(void) const
{
	return(Services.empty() ? nullptr : Services.back());
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


/// <summary>
/// Drops the pictures, style sheets, templates and dialog font read from the archives. A
/// screen already shown keeps what it was built with; the next one opened is built anew.
/// </summary>
void UIShellClass::On_Archives_Change(void)
{
	if (!Ready) {
		return;
	}

	if (InContext) {
		Deferred.DropFiles = true;
	} else {
		Drop_Cached_Files();
	}

	Host.Mark_Overlay_Dirty();
}


/// <summary>
/// Advances the interface by one pass. A call made while the toolkit is already running is
/// ignored rather than nested, so it is safe from anywhere.
/// </summary>
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

	for (UIViewClass * view : Modals) {
		view->Placed();
	}
	for (UIViewClass * view : Modeless) {
		view->Placed();
	}

	Apply_Cursor_Request();

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

	UIFrameRect frame = Host.Frame();
	if (frame.Width <= 0 || frame.Height <= 0) {
		return;
	}

	Render->Begin_Frame(frame.X, frame.Y, frame.Width, frame.Height);

	bool documents = Documents_Visible();
	bool overlays = UIDev_Active();

	if (documents) {
		UIReentryGuardClass rendering(InContext);
		Context->Render();
	}

	if (overlays) {
		Render->Begin_Dev_Frame(frame.X, frame.Y, frame.Width, frame.Height);
		UIDev_Render(*Render);
	}

	Drain_Deferred();
}


bool UIShellClass::Handle_Mouse_Move(int x, int y)
{
	UIPointerPosition position = Pointer_Position(x, y);

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


bool UIShellClass::Handle_Button_Down(int button, int x, int y)
{
	Input.Reconcile_Cancelled_Mouse(Physical_Buttons());

	if (Input.Mouse_Owner((unsigned)button) == UI_INPUT_SUPPRESSED) {
		Host.Mark_Overlay_Dirty();
		return(true);
	}

	UIPointerPosition position = Pointer_Position(x, y);
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


bool UIShellClass::Handle_Button_Up(int button, int x, int y)
{
	UIPointerPosition position = Pointer_Position(x, y);
	UIInputOwner owner = Input.Release_Mouse((unsigned)button);

	if (owner == UI_INPUT_IMGUI) {
		UIDev_Mouse_Position(position.X, position.Y);
		UIDev_Mouse_Button(button, false);
	} else {
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


bool UIShellClass::Handle_Wheel(int x, int y, float delta, bool horizontal)
{
	UIPointerPosition position = Pointer_Position(x, y);

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


bool UIShellClass::Handle_Key(bool down, int code, bool repeat)
{
	unsigned virtualkey = (unsigned)(code & 0xFF);
	int modifiers = Key_Modifiers();
	Rml::Input::KeyIdentifier key = UI_Key_Identifier((int)virtualkey);

	if (!down) {
		UIInputOwner owner = Input.Release_Key(virtualkey);
		if (owner == UI_INPUT_IMGUI) {
			UIDev_Key((int)virtualkey, false);
		} else if (owner != UI_INPUT_SUPPRESSED) {
			if (UIDev_Active()) {
				UIDev_Key((int)virtualkey, false);
			}
			if (key != Rml::Input::KI_UNKNOWN) {
				Context->ProcessKeyUp(key, modifiers);
			}
		}
		Host.Mark_Overlay_Dirty();
		return(UI_Consumes_Input(owner));
	}

	if (!repeat) {
		Input.Release_Key(virtualkey);
	}

	UIInputOwner owner = Input.Key_Owner(virtualkey);
	if (owner != UI_INPUT_NONE) {
		if (owner == UI_INPUT_IMGUI) {
			UIDev_Key((int)virtualkey, true);
		} else if (owner == UI_INPUT_RML && key != Rml::Input::KI_UNKNOWN) {
			Context->ProcessKeyDown(key, modifiers);
		}
	} else {
		if (UIDev_Key((int)virtualkey, true)) {
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


/// <summary>
/// Shows a screen and does not return until it closes. The caller owns the view, which is
/// released on the way out whatever closed it.
/// </summary>
/// <param name="service">Run once a pass. Returning true closes the screen with
/// UI_RESULT_SESSION_ENDED, which is how the game ends a screen the player did not
/// answer.</param>
/// <returns>What closed the screen. UI_RESULT_FAILED_TO_OPEN means it never opened, and
/// the caller should carry on as though the player had chosen nothing.</returns>
UIResult UIShellClass::Run_Modal(UIViewClass & view, UIServiceCallback const & service, bool hideparent)
{
	if (!Ready) {
		Log("UI: %s cannot open; the interface system did not start\n", view.Name());
		return(UI_RESULT_FAILED_TO_OPEN);
	}
	if (!FontLoaded) {
		Log("UI: %s needs %s, which did not load\n", view.Name(), UI_SHIPPED_FONT_FILE);
		return(UI_RESULT_FAILED_TO_OPEN);
	}

	if (Revealing && !Modals.empty()) {
		Modals.back()->Reveal_Done();
		Revealing = false;
	}

	RevealShown = 0.0f;
	RevealStart = Clock().Milliseconds();
	Ensure_Dialog_Font();
	Apply_Font_Policy();

	if (!Prepare_View(view)) {
		return(UI_RESULT_FAILED_TO_OPEN);
	}

	view.Presenter().Refresh();
	view.Sync();

	if (Input.Gesture_Owner() != UI_INPUT_NONE) {
		Drop_Presses();
	}
	Quarantine_Held_Input();

	char label[160];

	UIViewClass * covered = (hideparent && !Modals.empty()) ? Modals.back() : nullptr;
	if (covered != nullptr) {
		covered->Hide();
	}

	Modals.push_back(&view);
	Services.push_back(&service);
	view.Show(true);

	Tick();
	if (Render->Error()[0] != '\0') {
		Log("UI: %s could not be shown (%s)\n", view.Name(), Render->Error());
		view.Release();
		Modals.pop_back();
		Services.pop_back();
		Uncover(covered);
		return(UI_RESULT_FAILED_TO_OPEN);
	}

	RevealWidth = Host.Animate_Screens() ? view.Reveal_Width() : 0.0f;
	RevealShown = 0.0f;
	RevealStart = Clock().Milliseconds();
	RevealPasses = 1;
	Revealing = RevealWidth > 0.0f;
	if (Revealing) {
		Advance_Shown_Reveal(view);
		Tick();
	}

	Host.Mark_Overlay_Dirty();
	std::snprintf(label, sizeof(label), "%s shown", view.Name());
	Render->Log_Resource_Counts(label);

	Host.Clear_Keyboard_Queue();
	if (Revealing) {
		Host.Play_Sample(UI_REVEAL_SOUND, UI_REVEAL_VOLUME);
	}

	UIResult result = UI_RESULT_SESSION_ENDED;

	while (true) {
		bool ended = service();
		if (!Ready) {
			break;
		}

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

		Advance_Shown_Reveal(view);

		Tick();
		Host.Mark_Overlay_Dirty();
		Host.Present_If_Dirty();
		if (!Revealing) {
			RevealShown = 0.0f;
		}
	}

	ModalClosing = true;
	if (Ready && Input.Gesture_Owner() != UI_INPUT_NONE) {
		Drop_Presses();
	}
	Input.Cancel_UI();
	view.Presenter().Discard();
	view.Release();

	if (Ready) {
		UIReentryGuardClass updating(InContext);
		Context->Update();
	}

	if (!Modals.empty() && Modals.back() == &view) {
		Modals.pop_back();
		Services.pop_back();
	}

	Revealing = false;
	RevealShown = 0.0f;
	Uncover(covered);
	ModalClosing = false;

	if (Ready) {
		Host.Mark_Overlay_Dirty();
		std::snprintf(label, sizeof(label), "%s closed", view.Name());
		Render->Log_Resource_Counts(label);
		System->Reset_Cursor_Request();
		Apply_Cursor_Request();
		Host.Clear_Keyboard_Queue();
		Host.Focus_Main_Window();
	}

	return(result);
}


void UIShellClass::Uncover(UIViewClass * covered)
{
	if (covered == nullptr || !Ready || Modals.empty() || Modals.back() != covered) {
		return;
	}

	covered->Show(true);
}


bool UIShellClass::Prepare_View(UIViewClass & view)
{
	Render->Clear_Error();
	int errors = System->Error_Count();

	bool ready = view.Prepare(*this) && System->Error_Count() == errors && Render->Error()[0] == '\0';
	if (!ready) {
		Log("UI: %s could not be prepared (%s)\n", view.Name(),
			Render->Error()[0] != '\0' ? Render->Error() : "see the toolkit's log above");
		view.Release();
	}
	return(ready);
}


/// <summary>
/// Puts a screen up beside the game and returns at once. The caller owns the view and must
/// keep it alive until Hide_Modeless.
/// </summary>
/// <returns>False when the screen could not be opened, and nothing is shown.</returns>
bool UIShellClass::Show_Modeless(UIViewClass & view)
{
	if (!Ready || !FontLoaded || InContext) {
		return(false);
	}

	Ensure_Dialog_Font();
	Apply_Font_Policy();

	if (!Prepare_View(view)) {
		return(false);
	}

	view.Presenter().Refresh();
	view.Sync();
	view.Show(false);
	Modeless.push_back(&view);
	Refresh();

	char label[160];
	std::snprintf(label, sizeof(label), "%s shown beside the game", view.Name());
	Render->Log_Resource_Counts(label);
	return(true);
}


void UIShellClass::Hide_Modeless(UIViewClass & view)
{
	auto const unlisted = std::remove(Modeless.begin(), Modeless.end(), &view);
	bool const listed = unlisted != Modeless.end();
	Modeless.erase(unlisted, Modeless.end());
	view.Release();

	if (listed && Ready) {
		char label[160];
		std::snprintf(label, sizeof(label), "%s hidden", view.Name());
		Render->Log_Resource_Counts(label);
	}

	if (Ready && !InContext) {
		{
			UIReentryGuardClass updating(InContext);
			Context->Update();
		}
		Host.Mark_Overlay_Dirty();
		Host.Present_Now();
	}
}


void UIShellClass::Play_Click(void)
{
	Host.Play_Click();
}


UIClockClass & UIShellClass::Clock(void)
{
	static UIHostClockClass clock(Host);
	return(clock);
}


/// <summary>
/// Advances the interface and presents it at once, rather than at the next frame.
/// </summary>
void UIShellClass::Refresh(void)
{
	if (!Ready || InContext) {
		return;
	}

	Tick();
	Host.Mark_Overlay_Dirty();

	Host.Present_Now();
}


/// <returns>True when the interface took the event, and the game must not act on it.
/// A notification such as a lost capture is acted on and still answered false, because the
/// game needs it too.</returns>
bool UIShellClass::Handle_Window_Event(WindowEvent const & event)
{
	if (!Ready || InHook) {
		return(false);
	}

	bool const key = (event.Type == WINDOW_EVENT_KEY_DOWN || event.Type == WINDOW_EVENT_KEY_UP);

#ifdef _DEBUG
	if (Host.Developer_Keys_Armed() && key && !event.System && event.VirtualKey == VK_F6) {
		if (event.Type == WINDOW_EVENT_KEY_DOWN && !event.Repeat) {
			Deferred.ToggleDev = true;
		}
		return(true);
	}
#endif

	if (event.Type == WINDOW_EVENT_CAPTURE_LOST) {
		if (Input.Gesture_Owner() != UI_INPUT_NONE) {
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

	if (event.Type == WINDOW_EVENT_FOCUS_GAINED || event.Type == WINDOW_EVENT_FOCUS_LOST) {
		bool activated = (event.Type == WINDOW_EVENT_FOCUS_GAINED);
		if (InContext) {
			Deferred.DevFocus = activated ? 1 : 0;
		} else {
			UIDev_Focus(activated);
		}
		if (!activated) {
			if (Input.Gesture_Owner() != UI_INPUT_NONE) {
				if (InContext) {
					Deferred.DropPresses = true;
				} else {
					UIReentryGuardClass hooking(InHook);
					Drop_Presses();
				}
			}
			if (MouseInside) {
				if (InContext) {
					Deferred.Leave = true;
				} else {
					UIReentryGuardClass hooking(InHook);
					Context->ProcessMouseLeave();
					MouseInside = false;
				}
			}
			Input.Cancel_Keys();
			Release_UI_Capture();
		} else if (Active()) {
			Quarantine_Held_Input();
		}
		return(false);
	}

	// System keys, such as those pressed with Alt held, belong to the game.
	if (!Input_Event(event.Type) || (key && event.System)) {
		return(false);
	}

	if (InContext || !Active()) {
		return(false);
	}

	if (ModalClosing) {
		return(true);
	}

	UIReentryGuardClass hooking(InHook);
	bool consumed = Handle_Input_Event(event);

	if (!Modals.empty()) {
		consumed = true;
	}

	return(consumed);
}


bool UIShellClass::Handle_Input_Event(WindowEvent const & event)
{
	switch (event.Type) {
		case WINDOW_EVENT_MOUSE_MOVE:
			return(Handle_Mouse_Move(event.X, event.Y));

		case WINDOW_EVENT_MOUSE_DOWN:
			return(Handle_Button_Down((int)event.Button, event.X, event.Y));

		case WINDOW_EVENT_MOUSE_UP:
			return(Handle_Button_Up((int)event.Button, event.X, event.Y));

		case WINDOW_EVENT_MOUSE_WHEEL:
			return(Handle_Wheel(event.X, event.Y, event.Wheel, event.Horizontal));

		case WINDOW_EVENT_KEY_DOWN:
		case WINDOW_EVENT_KEY_UP:
			return(Handle_Key(event.Type == WINDOW_EVENT_KEY_DOWN, event.VirtualKey, event.Repeat));

		case WINDOW_EVENT_TEXT:
			return(Handle_Text(event.Text));

		default:
			return(false);
	}
}
