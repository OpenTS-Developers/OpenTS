/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "uitooltip.h"

#include "_map.h"
#include "_rect.h"
#include "_ui.h"
#include "_xmouse.h"
#include "goptions.h"
#include "ui/screens/gametip/uigametip.h"
#include "ui/uishell.h"
#include "ui/uiview.h"
#include "video.h"
#include "vidscale.h"


// The game placed a tooltip this many frame pixels under the pointer.
static const int UI_TOOLTIP_BELOW_POINTER = 16;


UIToolTip::UIToolTip(void) :
	BASECLASS(),
	OpenFailed(false),
	Region(0, 0, 0, 0),
	Drawn(false)
{
}


UIToolTip::~UIToolTip()
{
	Close();
}


/// <summary>
/// Chooses the region the tooltip stays inside: the sidebar when the pointer is over it, and
/// the tactical view otherwise.
/// </summary>
/// <returns>bool; Is the tooltip fit to be shown? A hidden mouse suppresses it.</returns>
bool UIToolTip::Update(ToolTipText * text)
{
	if (Get_Mouse_State() < 0) {
		return(false);
	}

	if (Options.IsSidebarOnRight == true) {
		Region = (text->Pos.X <= TacticalRect.X + TacticalRect.Width) ? TacticalRect : SidebarRect;
	} else {
		Region = (text->Pos.X <= SidebarRect.X + SidebarRect.Width) ? SidebarRect : TacticalRect;
	}

	return(Open());
}


void UIToolTip::Reset(const ToolTipText * text)
{
	Take_Down();
	BASECLASS::Reset(text);
}


/// <summary>
/// Shows the current tooltip from the tactical pass, which also takes down a tooltip that has
/// nothing left to say. The sidebar pass draws nothing, because the tooltip is not painted
/// onto either surface.
/// </summary>
void UIToolTip::Draw_Current(bool sidebar)
{
	if (sidebar) {
		return;
	}

	Drawn = false;
	BASECLASS::Draw_Current(true);
	if (!Drawn) {
		Take_Down();
	}
}


void UIToolTip::Draw(const ToolTipText * text)
{
	Drawn = true;
	if (Presenter == nullptr || View == nullptr) {
		return;
	}

	VideoScaleInfo const & scale = Video_Get_Scale_Info();

	Point2D pointer = text->Pos;
	Point2D topleft(Region.X, Region.Y);
	Point2D bottomright(Region.X + Region.Width, Region.Y + Region.Height);
	Point2D below(0, UI_TOOLTIP_BELOW_POINTER);
	Point2D origin(0, 0);
	Game_Point_To_Window(pointer);
	Game_Point_To_Window(topleft);
	Game_Point_To_Window(bottomright);
	Game_Point_To_Window(below);
	Game_Point_To_Window(origin);

	UIGameTipPlacement & placement = Presenter->Placement;
	placement.PointerX = (float)(pointer.X - scale.DestX);
	placement.PointerY = (float)(pointer.Y - scale.DestY);
	placement.Below = (float)(below.Y - origin.Y);
	placement.Left = (float)(topleft.X - scale.DestX);
	placement.Top = (float)(topleft.Y - scale.DestY);
	placement.Right = (float)(bottomright.X - scale.DestX);
	placement.Bottom = (float)(bottomright.Y - scale.DestY);

	Presenter->Text = text->Text;
	Presenter->Wanted = true;
	View->Sync();
}


/// <summary>
/// Fetches the text to display for a tooltip.
/// This routine is called by the tooltip manager when a region becomes due for display.
/// The game map supplies the string, and nothing is offered while the mouse is hidden.
/// </summary>
/// <returns>Returns with a pointer to the help text, or NULL if there is none.</returns>
const char * UIToolTip::ToolTip_Text(int id)
{
	if (Get_Mouse_State() < 0) {
		return(NULL);
	}
	return(Map.Help_Text(id));
}


/// <summary>
/// Opens the tooltip document, and opens it again when the player's side has changed so that
/// it takes that side's style sheet.
/// </summary>
/// <returns>bool; Is the document open? After it fails to open, it is not tried again and
/// no tooltip is shown.</returns>
bool UIToolTip::Open(void)
{
	std::string const sheet = UIShell.Side_Sheet();
	if (View != nullptr && sheet == Sheet) {
		return(true);
	}

	Close();
	if (OpenFailed) {
		return(false);
	}

	Presenter = std::make_unique<UIGameTipPresenterClass>();
	View = UI_Game_Tip_View(*Presenter);
	if (!UIShell.Show_Modeless(*View)) {
		View.reset();
		Presenter.reset();
		OpenFailed = true;
		return(false);
	}

	Sheet = sheet;
	return(true);
}


void UIToolTip::Close(void)
{
	if (View != nullptr) {
		UIShell.Hide_Modeless(*View);
		View.reset();
		Presenter.reset();
	}
}


void UIToolTip::Take_Down(void)
{
	if (Presenter != nullptr && Presenter->Wanted) {
		Presenter->Wanted = false;
		View->Sync();
		Video_Mark_Overlay_Dirty();
	}
}
