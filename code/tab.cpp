/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

/* $Header: /CounterStrike/TAB.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TAB.CPP                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 12/15/94                                                     *
 *                                                                                             *
 *                  Last Update : September 20, 1995 [JLB]                                     *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   TabClass::AI -- Handles player I/O with the tab buttons.                                  *
 *   TabClass::Draw_It -- Displays the tab buttons as necessary.                               *
 *   TabClass::One_Time -- Performs one time initialization of tab handler class.              *
 *   TabClass::Set_Active -- Activates a "filefolder tab" button.                              *
 *   TabClass::TabClass -- Default construct for the tab button class.                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "tab.h"

#include "_convert.h"
#include "_map.h"
#include "_mixfile.h"
#include "_rules.h"
#include "_surface.h"
#include "dialog.h"
#include "draw.h"
#include "goptions.h"
#include "language/language.h"
#include "mixfile.h"
#include "queue.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "scheme.h"
#include "shapeset.h"
#include "surface.h"

ShapeSet const * TabClass::TabShape = NULL;


/***********************************************************************************************
 * TabClass::TabClass -- Default construct for the tab button class.                           *
 *                                                                                             *
 *    The default constructor merely sets the tab buttons to default non-selected state.       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/15/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
TabClass::TabClass(void) :
	FlasherTimer(0),
	IsToRedraw(false),
	MoneyFlashTimer(0)
{
}


/// <summary>
/// Lists the members the tab bar holds.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TabClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Credits);
	stream.Serialize(FlasherTimer);

	// IsToRedraw -- a redraw flag; the load asks for a complete draw anyway.
	stream.Serialize(MoneyFlashTimer);
	// TabShape -- artwork fetched by One_Time.
}


/***********************************************************************************************
 * TabClass::Draw_It -- Displays the tab buttons as necessary.                                 *
 *                                                                                             *
 *    This routine is called whenever the display is being redrawn (in some fashion). The      *
 *    parameter can be used to force the tab buttons to redraw completely. The default action  *
 *    is to only redraw if the tab buttons have been explicitly flagged to be redraw. The      *
 *    result of this is the elimination of unnecessary redraws.                                *
 *                                                                                             *
 * INPUT:   complete -- bool; Force redraw of the entire tab button graphics?                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/15/1994 JLB : Created.                                                                 *
 *   05/19/1995 JLB : New EVA style.                                                           *
 *=============================================================================================*/
#define	EVA_WIDTH		80
#define	TAB_HEIGHT		8
void TabClass::Draw_It(bool complete)
{
	if (!Debug_Map) {

		/*
		**	Redraw the top bar imagery if flagged to do so or if the entire display needs
		**	to be redrawn.
		*/
		if (complete || IsToRedraw) {

			int width  = CompositeSurface->Get_Width() + SidebarSurface->Get_Width();
			int rightx = width - 1;
			int tab_height = TAB_HEIGHT * 2/*RESFACTOR*/;

			for (int x = TabShape->Get_Width(); x < CompositeSurface->Get_Width(); x += TabShape->Get_Width()) {
				Draw_Shape(*CompositeSurface, *SidebarDrawer, TabShape, 1, Point2D(x, 0), CompositeSurface->Get_Rect());
			}

			int sidex = Options.IsSidebarOnRight ? 0 : LogicalSurface->Get_Width() - EVA_WIDTH * 2/*RESFACTOR*/;

			Draw_Shape(*LogicalSurface, *SidebarDrawer, TabShape, 0, Point2D(sidex, 0), VisibleRect);
			Draw_Credits_Tab();
			LogicalSurface->Draw_Line(Point2D(0, tab_height-(1* 2)), Point2D(rightx, tab_height-(1 * 2/*RESFACTOR*/)), TBLACK);
			Fancy_Text_Print(TXT_TAB_BUTTON_CONTROLS, *LogicalSurface, LogicalSurface->Get_Rect(), Point2D(sidex + (EVA_WIDTH/2) * 2/*RESFACTOR*/, 0), ColorSchemes[0], TBLACK, TextPrintType(TPF_USE_GRAD_PAL | TPF_CENTER | TPF_METAL12));

			if (LogicalSurface != TileSurface) {
				TileSurface->Blit_From(Rect(0, 0, TileSurface->Get_Width(), tab_height), *LogicalSurface, Rect(0, 0, TileSurface->Get_Width(), tab_height));
			}
		}
	}

	if (!Debug_Map) {
		Credits.Graphic_Logic(complete || IsToRedraw);
		IsToRedraw = false;
	}

	BASECLASS::Draw_It(complete);
}


/// <summary>
/// Draws the tab backdrop for the credits and the mission timer.
/// This routine lays down the sidebar tab imagery that the credits readout is printed
/// over, and prints the mission timer alongside it whenever a timer is running. The
/// credit display calls this before it prints the new money value.
/// </summary>
void TabClass::Draw_Credits_Tab(void)
{
	Draw_Shape(*SidebarSurface, *SidebarDrawer, TabShape, 2, Point2D(0, 0), SidebarSurface->Get_Rect());

	if (Scen->MissionTimer.Is_Active()) {
		bool light = ((int)Scen->MissionTimer < TICKS_PER_MINUTE * Rule->TimerWarning) || Map.FlasherTimer > 0;
		Draw_Shape(*CompositeSurface, *SidebarDrawer, TabShape, /*light ? 4 :*/ 2, Point2D(TacticalRect.Width - TabShape->Get_Width(), 0), VisibleRect);

		int time = Scen->MissionTimer;

		int seconds = time / TICKS_PER_SECOND;
		int hours = seconds / 60 / 60;
		int minutes = seconds / 60;

		seconds = seconds % 60;
		minutes = minutes % 60;

		if (hours != 0) {
			Fancy_Text_Print(TXT_TIME_FORMAT_HOURS, *CompositeSurface, CompositeSurface->Get_Rect(),
				Point2D(TacticalRect.Width - TabShape->Get_Width() / 2, 0), ColorSchemes[0], TBLACK,
				TextPrintType(TPF_METAL12 | TPF_CENTER | TPF_USE_GRAD_PAL), hours, minutes, seconds);
		} else {
			Fancy_Text_Print(TXT_TIME_FORMAT_NO_HOURS, *CompositeSurface, CompositeSurface->Get_Rect(),
				Point2D(TacticalRect.Width - TabShape->Get_Width() / 2, 0), ColorSchemes[0], TBLACK,
				TextPrintType(TPF_METAL12 | TPF_CENTER | TPF_USE_GRAD_PAL), minutes, seconds);
		}
	}
	BASECLASS::IsToBlitSidebar = true;
}


/// <summary>
/// Draws the specified tab in its highlighted state.
/// This routine is used to give the player some feedback while a tab is being pressed.
/// The tab imagery and its label are redrawn in the highlight style.
/// </summary>
/// <param name="tab">The tab to highlight; zero for the controls tab, non-zero for the
/// sidebar tab.</param>
void TabClass::Hilite_Tab(int tab)
{
	int xpos = 0;
	int text = TXT_TAB_BUTTON_CONTROLS;
	int textx = (EVA_WIDTH/2) * 2;

	if (tab) {
		xpos = (320-EVA_WIDTH) * 2;
		//text = TXT_TAB_SIDEBAR;
		//textx = (320-(EVA_WIDTH/2)) * 2;
	} else {
		xpos = Options.IsSidebarOnRight ? 0 : LogicalSurface->Get_Rect().Width - textx*2;
	}

	Draw_Shape(*LogicalSurface, *SidebarDrawer, TabShape, 1, Point2D(xpos, 0), VisibleRect);
	Fancy_Text_Print(text, *LogicalSurface, LogicalSurface->Get_Rect(), Point2D(xpos + textx, 0), ColorSchemes[0], TBLACK, TextPrintType(TPF_METAL12 | TPF_CENTER | TPF_USE_GRAD_PAL));
}


/***********************************************************************************************
 * TabClass::AI -- Handles player I/O with the tab buttons.                                    *
 *                                                                                             *
 *    This routine is called every game tick and passed whatever key the player has supplied.  *
 *    If the input selects a tab button, then the graphic gets updated accordingly.            *
 *                                                                                             *
 * INPUT:   input -- The player's input character (might be mouse click).                      *
 *                                                                                             *
 *          x,y   -- Mouse coordinates at time of input.                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/15/1994 JLB : Created.                                                                 *
 *   12/31/1994 JLB : Uses mouse coordinate parameters.                                        *
 *   05/31/1995 JLB : Fixed to handle mouse shape properly.                                    *
 *   08/25/1995 JLB : Handles new scrolling option.                                            *
 *=============================================================================================*/
void TabClass::AI(KeyNumType &input, Point2D const & xy)
{
	if (!Map.IsRubberBand) {

		if (xy.Y >= 0 && xy.Y < (TAB_HEIGHT * 2/*RESFACTOR*/) && xy.X < (VisibleSurface->Get_Width() - 1) && xy.X > 0) {

			bool 	ok = false;

			/*
			**	If the mouse is at the top of the screen, then the tab bars only work
			**	in certain areas. If the special scroll modification is not active, then
			**	the tabs never work when the mouse is at the top of the screen.
			*/
			if (xy.Y > 0) {
				ok = true;
			}

			if (ok) {
				if (input == KN_LMOUSE) {
					int sel = 0;
					if (Options.IsSidebarOnRight) {
						if (xy.X >= (EVA_WIDTH * 2/*RESFACTOR*/)) sel = -1;
					} else {
						if (xy.X <= VisibleRect.Width - (EVA_WIDTH * 2/*RESFACTOR*/) || xy.X >= VisibleRect.Width) sel = -1;
					}
					if (sel >= 0) {
						Set_Active(sel);
						input = KN_NONE;
					}
				}

				Override_Mouse_Shape(MOUSE_NORMAL, false);
			}
		}
	}

	if (MoneyFlashTimer == 1) {
		IsToRedraw = true;
		Flag_To_Redraw();
	}

	Credits.AI();
	BASECLASS::AI(input, xy);
}


/***********************************************************************************************
 * TabClass::Set_Active -- Activates a "filefolder tab" button.                                *
 *                                                                                             *
 *    This function is used to activate one of the file folder tab buttons that appear at the  *
 *    top edge of the screen.                                                                  *
 *                                                                                             *
 * INPUT:   select   -- The button to activate. 0 = left button, 1=next button, etc.           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/15/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TabClass::Set_Active(int select)
{
	switch (select) {
		case 0:
			Queue_Options();
			break;

		case 1:
			BASECLASS::Activate(-1);
			break;

		default:
			break;
	}
}


/***********************************************************************************************
 * TabClass::One_Time -- Performs one time initialization of tab handler class.                *
 *                                                                                             *
 *    This routine will perform any one time initializations of the tab handler class. This    *
 *    typically includes the loading of the shapes that appear on it.                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/20/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TabClass::One_Time(void)
{
	BASECLASS::One_Time();
}


/// <summary>
/// Initializes the tab bar for the player's house.
/// This routine is called once the player's house has been established. It fetches the
/// tab artwork and resets the credits readout so that it will tick up from nothing.
/// </summary>
void TabClass::Init_For_House(void)
{
	BASECLASS::Init_For_House();
	TabShape = (ShapeSet const *)MixFileClass::Retrieve("TABS.SHP");
	Credits.Current = 0;
}


/// <summary>
/// Flashes the credits readout to catch the player's eye.
/// This routine is used when something has happened that the player really ought to
/// notice about the state of his funds, such as running short of cash. The tab bar is
/// flagged for redraw and the money display pulses for a moment before settling down.
/// </summary>
void TabClass::Flash_Money(void)
{
	IsToRedraw = true;
	Flag_To_Redraw();
	MoneyFlashTimer = 7;
}
