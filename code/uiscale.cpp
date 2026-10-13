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
/// editor always uses 1x.
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
/// Lays the surfaces out again when the chosen interface scale differs from the current one,
/// as after the player changes it in the display options. Call it before a scenario is read.
/// </summary>
void UI_Scale_Refit(void)
{
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
/// The scale the game's dialogs are drawn at: the interface scale during a scenario, and 1x
/// elsewhere, where the menus sit over artwork that keeps its size.
/// </summary>
int UI_Dialog_Scale(void)
{
	return(ScenarioActive ? _Scale : (int)UI_SCALE_MIN);
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
/// Copies part of the sidebar surface onto the visible surface, enlarged by the interface scale.
/// </summary>
/// <param name="source">The part of the sidebar surface to show, in interface pixels.</param>
void UI_Scale_Show_Sidebar(Rect const & source)
{
	int const left = Options.IsSidebarOnRight ? BattlefieldRect.X + BattlefieldRect.Width : 0;
	Rect const dest(left + source.X * _Scale, source.Y * _Scale, source.Width * _Scale, source.Height * _Scale);

	VisibleSurface->Blit_From(dest, *SidebarSurface, source);
}
