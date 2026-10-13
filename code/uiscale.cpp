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

#include "_rect.h"
#include "_surface.h"
#include "_ui.h"
#include "globals.h"
#include "goptions.h"
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
	int const previous = _Scale;
	_Scale = Debug_Map ? (int)UI_SCALE_MIN : UI_Scale_For(visible.Width, visible.Height, Options.UIScale);
	if (_Scale != previous) {
		UIShell.On_Video_Change();
	}

	battlefield = UI_Scale_Battlefield_Rect(visible.Width, visible.Height, _Scale);
	if (!Options.IsSidebarOnRight && !Debug_Map) {
		battlefield.X = UI_SCALE_SIDEBAR_WIDTH * _Scale;
	}

	composite = Rect(0, 0, battlefield.Width, visible.Height);
	sidebar = Rect(0, 0, UI_SCALE_SIDEBAR_WIDTH, visible.Height / _Scale);
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
