/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "battlezoom.h"

#include "_keyboar.h"
#include "_map.h"
#include "_rect.h"
#include "_surface.h"
#include "_tactica.h"
#include "battlezoomlayout.h"
#include "battlezoomstate.hh"
#include "bsurface.h"
#include "globals.h"
#include "keyboard.h"
#include "session.h"
#include "surface.h"
#include "tactical.h"
#include "uiscale.h"
#include "uiscalelayout.h"
#include "win.h"

#include <algorithm>
#include <memory>


static BattleZoomStateClass _State;
static bool _Applying = false;


static bool Can_Zoom(void)
{
	return(ScenarioActive && GameActive && GameInFocus && !Debug_Map && !IgnoreInput && TacticalMap != NULL && TacticalMap->ZoomFactor == 1.0);
}


static bool Is_Zoomed(void)
{
	return(_State.Factor() > BATTLE_ZOOM_MIN && ScenarioActive);
}


static bool Is_Scaled(void)
{
	return(UI_Scale() > UI_SCALE_MIN && ScenarioActive);
}


/// <summary>
/// Does Battle_Zoom_Present draw the composite surface this frame? It does while the interface
/// is scaled, and while the battlefield is zoomed outside a trigger zoom.
/// </summary>
bool Battle_Zoom_Presents(void)
{
	if (VisibleSurface == NULL || TacticalMap == NULL) {
		return(false);
	}

	return(Is_Scaled() || (Is_Zoomed() && TacticalMap->ZoomFactor == 1.0));
}


/// <summary>
/// The area chat and game messages are laid out in. A scaled interface draws them in
/// interface pixels on a plane over the battlefield, so the area starts at that plane's corner.
/// </summary>
Rect Battle_Zoom_Message_Rect(void)
{
	int const scale = UI_Scale();
	if (scale <= UI_SCALE_MIN) {
		return(BattlefieldRect);
	}

	return(Rect(0, 0, BattlefieldRect.Width / scale, BattlefieldRect.Height / scale));
}


/// <summary>
/// Fetches the zoom factor the battlefield is shown at.
/// </summary>
/// <returns>int; 1 when the battlefield is not zoomed.</returns>
int Battle_Zoom_Factor(void)
{
	return(_State.Factor());
}


/// <summary>
/// Is the tactical view being resized for a zoom change rather than a new battlefield area?
/// </summary>
bool Battle_Zoom_Applying(void)
{
	return(_Applying);
}


/// <summary>
/// Records the frame area the battlefield occupies and returns the native view for it. The
/// zoom factor drops to the nearest one the area allows, and to 1x outside a scenario, as when
/// the main menu changes the resolution after a zoomed match.
/// </summary>
/// <param name="battlefield">The battlefield area in frame pixels.</param>
/// <returns>Rect; The native tactical view, which equals the area when the battlefield is not zoomed.</returns>
Rect Battle_Zoom_Fit_View(Rect const & battlefield)
{
	BattlefieldRect = battlefield;
	return(_State.Fit_View(battlefield, ScenarioActive));
}


/// <summary>
/// Returns the battlefield to its unzoomed size about the same view centre, and drops any zoom
/// change still waiting. Messages on screen are kept.
/// </summary>
void Battle_Zoom_Reset(void)
{
	bool const changed = _State.Factor() != BATTLE_ZOOM_MIN;
	_State.Reset();
	if (changed) {
		_Applying = true;
		Map.Set_View_Dimensions(BattlefieldRect);
		_Applying = false;
		Map.Flag_To_Redraw(GS_REDRAW_ALL);
	}
}


/// <summary>
/// Applies a zoom change asked for by the wheel. The change waits while a key press is pending
/// or a mouse button is held, and is dropped while the player cannot zoom. The map position at
/// the recorded pointer location stays there when the factor changes, subject to map limits.
/// </summary>
/// <param name="input_pending">Is a key or button press waiting to be processed this frame?</param>
void Battle_Zoom_AI(bool input_pending)
{
	// A release the interface took never reaches Battle_Zoom_Mouse_Button.
	unsigned held = 0;
	if (Keyboard != nullptr) {
		KeyNumType const keys[] = { KN_LMOUSE, KN_RMOUSE, (KeyNumType)VK_MBUTTON, (KeyNumType)VK_XBUTTON1, (KeyNumType)VK_XBUTTON2 };
		for (int button = WINDOW_BUTTON_LEFT; button <= WINDOW_BUTTON_X2; button++) {
			if (Keyboard->Down(keys[button])) {
				held |= BattleZoomStateClass::Button_Mask((WindowMouseButton)button);
			}
		}
	}
	_State.Reconcile_Buttons(held);

	int const before_factor = _State.Factor();
	Point2D frame;
	if (!_State.Apply_Pending(BattlefieldRect, Can_Zoom(), input_pending, frame)) {
		return;
	}

	Rect const before_view = TacticalRect;
	Point2D const before_point = Battle_Zoom_Screen_To_View(frame, BattlefieldRect, before_factor);
	Point2D const center = TacticalMap->Get_Tactical_Position();

	_Applying = true;
	Map.Set_View_Dimensions(BattlefieldRect);
	_Applying = false;

	Point2D const after_point = Battle_Zoom_Screen_To_View(frame, BattlefieldRect, _State.Factor());
	TacticalMap->Set_Tactical_Position(Battle_Zoom_Anchored_Center(center, before_view, before_point, TacticalRect, after_point));

	Map.Flag_To_Redraw(GS_REDRAW_ALL);
}


/// <summary>
/// Takes a vertical wheel movement for the battlefield zoom.
/// </summary>
/// <param name="wheel">Wheel notches, positive away from the player.</param>
/// <param name="frame">The pointer position in frame pixels.</param>
/// <returns>bool; Was the movement used here? A movement outside the battlefield, or one made
/// while the player cannot zoom, is left for the sidebar.</returns>
bool Battle_Zoom_Wheel(float wheel, Point2D const & frame)
{
	return(_State.Wheel(BattlefieldRect, Can_Zoom(), wheel, frame));
}


/// <summary>
/// Tracks a mouse button press or release. While the button that started a press over the
/// battlefield is held, the pointer stays mapped onto the battlefield, so a drag that leaves
/// it ends at the edge of the view.
/// </summary>
/// <param name="button">The button whose state changed.</param>
/// <param name="down">Is this a press?</param>
/// <param name="frame">The pointer position in frame pixels.</param>
void Battle_Zoom_Mouse_Button(WindowMouseButton button, bool down, Point2D const & frame)
{
	_State.Mouse_Button(button, down, BattlefieldRect, frame);
}


/// <summary>
/// Forgets every held mouse button, as when the window loses the keyboard focus.
/// </summary>
void Battle_Zoom_Release_Buttons(void)
{
	_State.Release_Buttons();
}


void Battle_Zoom_Cancel_Pending(void)
{
	_State.Cancel_Pending();
}


/// <summary>
/// Converts a frame position into the coordinates the game uses. A position over the zoomed
/// battlefield becomes a native view position, and a drag started there stays clamped to it.
/// A position over a scaled sidebar or tab strip becomes an interface position. Other
/// positions are left alone.
/// </summary>
/// <param name="point">The position to convert in place.</param>
void Battle_Zoom_Frame_To_View(Point2D & point)
{
	if (Is_Zoomed()) {
		if (_State.Dragging()) {
			point.X = std::clamp(point.X, BattlefieldRect.X, BattlefieldRect.X + BattlefieldRect.Width - 1);
			point.Y = std::clamp(point.Y, BattlefieldRect.Y, BattlefieldRect.Y + BattlefieldRect.Height - 1);
		}

		point = Battle_Zoom_Screen_To_View(point, BattlefieldRect, _State.Factor());
	}

	// Battlefield positions, native or not, lie outside the areas the interface scale converts.
	if (Is_Scaled()) {
		point = UI_Scale_Screen_To_Interface(point, BattlefieldRect, UI_Scale());
	}
}


/// <summary>
/// Converts a native view or interface position back into the frame.
/// </summary>
/// <returns>Point2D; The frame position, or the position unchanged when neither the zoom nor
/// the interface scale applies to it.</returns>
Point2D Battle_Zoom_View_To_Frame(Point2D const & point)
{
	if (Is_Zoomed() && TacticalRect.Is_Point_Within(point)) {
		return(Battle_Zoom_View_To_Screen(point, BattlefieldRect, _State.Factor()));
	}

	if (Is_Scaled()) {
		return(UI_Scale_Interface_To_Screen(point, BattlefieldRect, UI_Scale()));
	}

	return(point);
}


/// <summary>
/// Converts a native view position into the frame position the edge scroll tests. A position
/// on an edge of the native view lands on the same edge of the battlefield area, so the frame
/// pixels a whole-number magnification leaves unfilled still count as that edge.
/// </summary>
Point2D Battle_Zoom_Edge_Point(Point2D const & point)
{
	if (!Is_Zoomed() || !TacticalRect.Is_Point_Within(point)) {
		return(point);
	}

	Point2D frame = Battle_Zoom_View_To_Screen(point, BattlefieldRect, _State.Factor());

	if (point.X == TacticalRect.X) {
		frame.X = BattlefieldRect.X;
	} else if (point.X == TacticalRect.X + TacticalRect.Width - 1) {
		frame.X = BattlefieldRect.X + BattlefieldRect.Width - 1;
	}

	if (point.Y == TacticalRect.Y) {
		frame.Y = BattlefieldRect.Y;
	} else if (point.Y == TacticalRect.Y + TacticalRect.Height - 1) {
		frame.Y = BattlefieldRect.Y + BattlefieldRect.Height - 1;
	}

	return(frame);
}


// The message plane's background; no message text or backdrop uses this colour.
static unsigned short const MESSAGE_PLANE_KEY = 0xF81F;

static std::unique_ptr<BSurface> _MessagePlane;


static void Draw_Scaled_Messages(int scale)
{
	Rect const area = Battle_Zoom_Message_Rect();
	if (!area.Is_Valid()) {
		return;
	}

	if (_MessagePlane == nullptr || _MessagePlane->Get_Width() != area.Width || _MessagePlane->Get_Height() != area.Height) {
		_MessagePlane = std::make_unique<BSurface>(area.Width, area.Height, 2);
	}

	_MessagePlane->Fill(MESSAGE_PLANE_KEY);
	Surface * oldpage = LogicalSurface;
	LogicalSurface = _MessagePlane.get();
	Session.Messages.Draw();
	LogicalSurface = oldpage;

	unsigned short const * source = (unsigned short const *)_MessagePlane->Lock();
	unsigned short * dest = (unsigned short *)VisibleSurface->Lock(BattlefieldRect.Top_Left());
	if (source != nullptr && dest != nullptr) {
		int const source_pitch = _MessagePlane->Stride() / 2;
		int const dest_pitch = VisibleSurface->Stride() / 2;

		for (int y = 0; y < area.Height; y++) {
			for (int x = 0; x < area.Width; x++) {
				unsigned short const pixel = source[y * source_pitch + x];
				if (pixel == MESSAGE_PLANE_KEY) {
					continue;
				}

				for (int row = 0; row < scale; row++) {
					unsigned short * out = dest + (y * scale + row) * dest_pitch + x * scale;
					for (int column = 0; column < scale; column++) {
						out[column] = pixel;
					}
				}
			}
		}
	}

	if (dest != nullptr) {
		VisibleSurface->Unlock();
	}
	if (source != nullptr) {
		_MessagePlane->Unlock();
	}
}


/// <summary>
/// Shows the composite surface with the battlefield magnified by the zoom or a trigger zoom and
/// the tab strip enlarged by the interface scale. Chat and game messages are drawn over the
/// battlefield, enlarged by the interface scale.
/// </summary>
/// <param name="surface">The surface being presented.</param>
/// <returns>bool; Was the surface shown here? It is only when Battle_Zoom_Presents allows it
/// and the surface is the composite surface.</returns>
bool Battle_Zoom_Present(Surface & surface)
{
	if (!Battle_Zoom_Presents() || &surface != CompositeSurface) {
		return(false);
	}

	int const scale = UI_Scale();

	// Whole-number enlargements are copied pixel by pixel. A GDI stretch of the tab strip wiped
	// the battlefield already copied to the visible surface at some sizes.
	if (TacticalMap->ZoomFactor != 1.0) {

		// A trigger zoom magnifies the middle of the view to fill the battlefield, by a factor
		// the rules may set to a fraction, so GDI stretches it.
		int const width = (int)(TacticalRect.Width / TacticalMap->ZoomFactor);
		int const height = (int)(TacticalRect.Height / TacticalMap->ZoomFactor);
		Rect const shown(TacticalRect.X + (TacticalRect.Width - width) / 2, TacticalRect.Y + (TacticalRect.Height - height) / 2, width, height);
		VisibleSurface->Blit_From(BattlefieldRect, surface, shown, false, true);

	} else {

		// Screen shake moves the magnified picture by whole native pixels.
		int const zoom = _State.Factor();
		Rect const screen = Battle_Zoom_Screen_Rect(BattlefieldRect, zoom);
		Rect const source = TacticalRect - Point2D(Map.ScreenX / zoom, Map.ScreenY / zoom);
		Rect const shown = Intersect(source, TacticalRect);
		Rect const dest(screen.X + (shown.X - source.X) * zoom, screen.Y + (shown.Y - source.Y) * zoom, shown.Width * zoom, shown.Height * zoom);

		if (!(dest == BattlefieldRect)) {
			VisibleSurface->Fill_Rect(BattlefieldRect, 0);
		}
		if (shown.Is_Valid()) {
			UI_Scale_Copy(*VisibleSurface, dest.Top_Left(), surface, shown, zoom);
		}
	}

	int const strip_width = std::min(surface.Get_Width(), BattlefieldRect.X + BattlefieldRect.Width);
	Rect const strip(0, 0, strip_width / scale, BattlefieldRect.Y / scale);
	if (strip.Is_Valid()) {
		if (strip.Width * scale != strip_width) {
			VisibleSurface->Fill_Rect(Rect(0, 0, strip_width, BattlefieldRect.Y), 0);
		}
		UI_Scale_Copy(*VisibleSurface, Point2D(0, 0), surface, strip, scale);
	}

	if (scale > UI_SCALE_MIN) {
		Draw_Scaled_Messages(scale);
	} else {
		Surface * oldpage = LogicalSurface;
		LogicalSurface = VisibleSurface;
		Session.Messages.Draw();
		LogicalSurface = oldpage;
	}

	return(true);
}
