/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "uiscale.h"

#include "_map.h"
#include "_rect.h"
#include "_surface.h"
#include "_ui.h"
#include "globals.h"
#include "goptions.h"
#include "init.h"
#include "surface.h"
#include "ui/uishell.h"
#include "uiscalelayout.h"


static int _Scale = UI_SCALE_MIN;

// Set from the start of a scenario, so its briefing is drawn at the interface scale, until the
// main menu shows again.
static bool _Starting = false;


/// <summary>
/// Fetches the interface scale of the current layout.
/// </summary>
/// <returns>int; 1 when the sidebar and tab strip are drawn at their artwork size.</returns>
int UI_Scale(void)
{
	return(_Scale);
}


/// <summary>
/// Fetches the frame height in interface pixels, which is the height the sidebar lays itself
/// out in.
/// </summary>
int UI_Height(void)
{
	return(VisibleRect.Height / _Scale);
}


/// <summary>
/// Chooses the interface scale for a frame and works out the surface sizes for it. The map
/// editor always uses 1x. Call it only where the surfaces are then allocated at these sizes.
/// </summary>
/// <param name="visible">The whole frame.</param>
/// <param name="battlefield">Receives the frame area the battlefield fills.</param>
/// <param name="composite">Receives the size of the composite and tile surfaces.</param>
/// <param name="sidebar">Receives the size of the sidebar surface, in interface pixels.</param>
void UI_Scale_Layout(Rect const & visible, Rect & battlefield, Rect & composite, Rect & sidebar)
{
	_Scale = Debug_Map ? (int)UI_SCALE_MIN : UI_Scale_For(visible.Width, visible.Height, Options.UIScale);

	battlefield = UI_Scale_Battlefield_Rect(visible.Width, visible.Height, _Scale);
	if (!Options.IsSidebarOnRight && !Debug_Map) {
		battlefield.X = UI_SCALE_SIDEBAR_WIDTH * _Scale;
	}

	composite = Rect(0, 0, battlefield.Width, visible.Height);
	sidebar = Rect(0, 0, UI_SCALE_SIDEBAR_WIDTH, visible.Height / _Scale);
}


/// <summary>
/// The frame area the battlefield fills at the scale the surfaces were allocated for.
/// </summary>
Rect UI_Scale_Battlefield(Rect const & visible)
{
	Rect battlefield = UI_Scale_Battlefield_Rect(visible.Width, visible.Height, _Scale);
	if (!Options.IsSidebarOnRight && !Debug_Map) {
		battlefield.X = UI_SCALE_SIDEBAR_WIDTH * _Scale;
	}
	return(battlefield);
}


/// <summary>
/// Lays the surfaces out again when the chosen interface scale differs from the current one,
/// as after the player changes it in the display options. Call it before a scenario is read.
/// </summary>
void UI_Scale_Refit(void)
{
	_Starting = true;

	int const wanted = Debug_Map ? (int)UI_SCALE_MIN : UI_Scale_For(VisibleRect.Width, VisibleRect.Height, Options.UIScale);
	if (wanted == _Scale) {
		return;
	}

	Rect battlefield;
	Rect composite;
	Rect sidebar;
	UI_Scale_Layout(VisibleRect, battlefield, composite, sidebar);

	Allocate_Surfaces(VisibleRect, composite, composite, sidebar);
	LogicalSurface = HiddenSurface;
	Map.Set_View_Dimensions(battlefield);
}


/// <summary>
/// The scale the game's dialogs are drawn at: the interface scale while a scenario starts or runs, and 1x
/// elsewhere, where the menus sit over artwork that keeps its size.
/// </summary>
int UI_Dialog_Scale(void)
{
	return(ScenarioActive || _Starting ? _Scale : (int)UI_SCALE_MIN);
}


/// <summary>
/// Tells the dialog layer when the dialog scale has changed, as when a scenario starts or ends.
/// </summary>
void UI_Scale_Sync_Dialogs(void)
{
	static int _shown = UI_SCALE_MIN;

	int const scale = UI_Dialog_Scale();
	if (scale != _shown) {
		_shown = scale;
		UIShell.On_Video_Change();
	}
}


/// <summary>
/// Copies part of a 16-bit surface onto another, drawing each source pixel as a square of
/// scale by scale pixels. The part that would land outside the destination is left out.
/// </summary>
/// <param name="dest">The surface drawn on.</param>
/// <param name="at">Where the top left source pixel lands.</param>
/// <param name="source">The surface copied from.</param>
/// <param name="from">The part of the source to copy.</param>
/// <param name="scale">How many times larger to draw it, at least 1.</param>
void UI_Scale_Copy(Surface & dest, Point2D const & at, Surface const & source, Rect const & from, int scale)
{
	Rect const part = Intersect(source.Get_Rect(), from);
	if (!part.Is_Valid() || scale < 1) {
		return;
	}

	unsigned short const * in = (unsigned short const *)source.Lock(part.Top_Left());
	unsigned short * out = (unsigned short *)dest.Lock();
	if (in != nullptr && out != nullptr) {
		int const in_pitch = source.Stride() / 2;
		int const out_pitch = dest.Stride() / 2;
		int const left = at.X + (part.X - from.X) * scale;
		int const top = at.Y + (part.Y - from.Y) * scale;

		for (int y = 0; y < part.Height * scale; y++) {
			int const row = top + y;
			if (row < 0 || row >= dest.Get_Height()) {
				continue;
			}

			unsigned short const * line = in + (y / scale) * in_pitch;
			unsigned short * target = out + row * out_pitch;
			for (int x = 0; x < part.Width * scale; x++) {
				int const column = left + x;
				if (column >= 0 && column < dest.Get_Width()) {
					target[column] = line[x / scale];
				}
			}
		}
	}

	if (out != nullptr) {
		dest.Unlock();
	}
	if (in != nullptr) {
		source.Unlock();
	}
}


/// <summary>
/// Copies part of the sidebar surface onto the visible surface, enlarged by the interface scale.
/// </summary>
/// <param name="source">The part of the sidebar surface to show, in interface pixels.</param>
void UI_Scale_Show_Sidebar(Rect const & source)
{
	int const left = Options.IsSidebarOnRight ? BattlefieldRect.X + BattlefieldRect.Width : 0;

	UI_Scale_Copy(*VisibleSurface, Point2D(left + source.X * _Scale, source.Y * _Scale), *SidebarSurface, source, _Scale);
}


/// <summary>
/// Returns the game's dialogs to 1x, as the main menu shows again after a scenario.
/// </summary>
void UI_Scale_End_Scenario(void)
{
	_Starting = false;
}
