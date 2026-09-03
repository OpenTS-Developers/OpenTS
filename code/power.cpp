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

/* $Header: /CounterStrike/POWER.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : POWER.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 12/15/94                                                     *
 *                                                                                             *
 *                  Last Update : October 14, 1996 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   PowerClass::AI -- Process the power bar logic.                                            *
 *   PowerClass::Draw_It -- Renders the power bar graphic.                                     *
 *   PowerClass::Init_Clear -- Clears all the power bar variables.                             *
 *   PowerClass::One_Time -- One time processing for the power bar.                            *
 *   PowerClass::PowerButtonClass::Action -- Handles the mouse over the power bar area.        *
 *   PowerClass::PowerClass -- Default constructor for the power bar class.                    *
 *   PowerClass::Refresh_Cells -- Intercepts the redraw logic to see if sidebar to redraw too. *
 *   PowerClass::Power_Height -- Given a value figure where it falls on bar                    *
 *   PowerClass::Flash_Power -- Flag the power bar to flash.                                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "_bench.h"
#include "_convert.h"
#include "_map.h"
#include "_mixfile.h"
#include "_rect.h"
#include "_surface.h"
#include "_tooltip.h"
#include "bench.h"
#include "building.h"
#include "builtype.h"
#include "cctooltip.h"
#include "data.h"
#include "draw.h"
#include "globals.h"
#include "house.h"
#include "language/language.h"
#include "mixfile.h"
#include "savestream.h"
#include "surface.h"

#include "bench.hh"

#include <algorithm>

/*
**	Points to the shape to use for the "desired" power level indicator.
*/
void const * PowerClass::PowerPipShape;


/***********************************************************************************************
 * PowerClass::PowerClass -- Default constructor for the power bar class.                      *
 *                                                                                             *
 *    This is the default constructor for the power bar class. It doesn't really do anything.  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/20/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
PowerClass::PowerClass(void) :
	BASECLASS(),
	IsToRedraw(false),
	FlashTimer(0),
	FlashCount(0),
	UpdateTimer(0),
	GreenPipCount(0),
	YellowPipCount(0),
	RedPipCount(0),
	HasChanged(0),
	RecordedDrain(-1),
	RecordedPower(-1)
{
}


/// <summary>
/// Lists the members the power bar holds.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void PowerClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	// IsToRedraw -- a redraw flag; the load asks for a complete draw anyway.
	// FlashTimer -- both run off the system clock, so a saved value would carry the time of the
	// save into the loaded game.
	// UpdateTimer
	// FlashCount -- it counts the flashes still owed by a power level change, and the timer that
	// would pace them is not saved either.
	stream.Serialize(GreenPipCount);
	stream.Serialize(YellowPipCount);
	stream.Serialize(RedPipCount);
	stream.Serialize(HasChanged);
	stream.Serialize(RecordedDrain);
	stream.Serialize(RecordedPower);
	// PowerPipShape -- artwork fetched by One_Time.
}


/***********************************************************************************************
 * PowerClass::Init_Clear -- Clears all the power bar variables.                               *
 *                                                                                             *
 *    This routine is called in preparation for the start of a scenario. The power bar is      *
 *    initialized into the null state by this routine. As soon as the scenario starts, the     *
 *    power bar will rise to reflect the actual power output and drain.                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/07/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void PowerClass::Init_Clear(void)
{
	BASECLASS::Init_Clear();
	RecordedDrain = -1;
	RecordedPower = -1;
	FlashTimer = 0;
	FlashCount = 0;
	UpdateTimer = 0;
	GreenPipCount = 0;
	YellowPipCount = 0;
	RedPipCount = 0;
	HasChanged = 0;
}


/***********************************************************************************************
 * PowerClass::One_Time -- One time processing for the power bar.                              *
 *                                                                                             *
 * This routine is for code that truly only needs to be done once per game run.                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void PowerClass::One_Time(void)
{
	BASECLASS::One_Time();
}


/// <summary>
/// Fetches the power bar artwork for the house being played.
/// This routine is called when the playing house is established so that the sidebar has
/// its pip shapes on hand before anything is drawn.
/// </summary>
void PowerClass::Init_For_House(void)
{
	BASECLASS::Init_For_House();
	PowerPipShape = MFCD::Retrieve("POWERP.SHP");
}


/***********************************************************************************************
 * PowerClass::Flash_Power -- Flag the power bar to flash.                                     *
 *                                                                                             *
 *    This will cause the power bar to display with a flash so as to draw attention to         *
 *    itself. Typical use of this effect is when power is low.                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/14/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void PowerClass::Flash_Power(void)
{
	FlashCount = POWER_FLASH_COUNT;
	FlashTimer = POWER_FLASH_RATE;
}


/// <summary>
/// Determines how tall the power bar should be.
/// The bar is scaled so that a modest base uses only part of the available height and a
/// sprawling one approaches the full sidebar. This keeps the bar informative regardless of
/// how much power the player has brought online.
/// </summary>
/// <returns>Returns with the desired height of the power bar, measured in pips.</returns>
int PowerClass::Desired_Power_Height(void)
{
	int max_pips = Max_Power_Height();
	int drain = 0;
	int power = 0;

	for (int i = 0; i < Buildings.Count(); i++) {
		if (Buildings[i]->House == PlayerPtr) {
			drain += Buildings[i]->Class->Drain;
			power += Buildings[i]->Class->Power;
		}
	}

	int empty_pips = (400.0 / ((drain + power) + 400.0) * max_pips);
	empty_pips = std::max(empty_pips, 0);
	empty_pips = std::min(empty_pips, max_pips - 1);

	return(max_pips - empty_pips);
}


/// <summary>
/// Fetches the delay between power bar pip adjustments.
/// The bar animates toward its new height one pip at a time. This routine paces that
/// animation so that a tall bar takes its time while a short one settles quickly.
/// </summary>
/// <returns>Returns with the delay in game frames before the next pip adjustment.</returns>
int PowerClass::Update_Delay(void)
{
	int desired_pips = Desired_Power_Height();

	int current_pips = GreenPipCount + YellowPipCount + RedPipCount;
	if (current_pips > desired_pips) {
		current_pips = desired_pips;
	}

	return((double)current_pips / (double)desired_pips * 5.0);
}


/// <summary>
/// Determines the maximum height of the power bar.
/// This routine reports how many pips will fit alongside the sidebar at its current size.
/// Every other power bar height is measured against this figure.
/// </summary>
/// <returns>Returns with the maximum number of pips the power bar can hold.</returns>
int PowerClass::Max_Power_Height(void)
{
	return(SidebarClass::StripClass::SideBarGeneralEnums::OBJECT_HEIGHT * Map.Max_Visible() / POWER_PIP_HEIGHT);
}


/// <summary>
/// Determines the pip count of each color band in the power bar.
/// This routine splits the desired bar height between the green, yellow and red portions
/// according to how much power the player is producing and how much is being consumed.
/// The power bar logic uses it to decide which color of pip to add or remove.
/// </summary>
/// <param name="green">Filled in with the number of surplus power pips desired.</param>
/// <param name="yellow">Filled in with the number of marginal power pips desired.</param>
/// <param name="red">Filled in with the number of consumed power pips desired.</param>
/// <returns>Returns with the maximum number of pips the power bar can hold.</returns>
int PowerClass::Desired_Levels(int & green, int & yellow, int & red)
{
	int max_pips = Max_Power_Height();
	int desired_pips = Desired_Power_Height();

	int drain = 0;
	int power = 0;

	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass * bptr = Buildings[i];
		drain += bptr->Class->Drain;
		power += bptr->Class->Power;
	}

	double power_delta = PlayerPtr->Power_Output() - PlayerPtr->Power_Drain();

	double green_power = 0.0;
	double yellow_power = 100.0;

	if (power_delta < 0.0) {
		yellow_power = 0.0;
		green_power = 0.0;
	} else {
		if (power_delta < 100.0) {
			yellow_power = power_delta;
		}
		green_power = power_delta - yellow_power;
	}

	double red_fraction = 1.0;
	double green_fraction = 0.0;
	double yellow_fraction = 0.0;

	double total_power = PlayerPtr->Power_Drain() + yellow_power + green_power;

	if (total_power > 0.0) {
		red_fraction = PlayerPtr->Power_Drain() / total_power;
		green_fraction = green_power / total_power;
		yellow_fraction = yellow_power / total_power;
	}

	red = desired_pips * red_fraction;
	yellow = desired_pips * yellow_fraction;
	green = desired_pips * green_fraction;

	red += (desired_pips * green_fraction - green) + (desired_pips * yellow_fraction - yellow) + (desired_pips * red_fraction - red) + 0.01;

	return(max_pips);
}


/***********************************************************************************************
 * PowerClass::Draw_It -- Renders the power bar graphic.                                       *
 *                                                                                             *
 *    This routine will draw the power bar graphic to the LogicPage.                           *
 *                                                                                             *
 * INPUT:   complete -- Should the power bar be redrawn even if it isn't specifically flagged  *
 *                      to do so?                                                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/20/1994 JLB : Created.                                                                 *
 *   12/27/1994 JLB : Changes power bar color depending on amount of power.                    *
 *=============================================================================================*/
void PowerClass::Draw_It(bool complete)
{
	if (complete || IsToRedraw) {
		BStart(BENCH_POWER);

		if (Map.IsSidebarActive) {
			IsToRedraw = false;
			Map.IsToBlitSidebar = true;

			Rect rect = SidebarSurface->Get_Rect();
			int x = POWER_X;
			int y = SidebarRect.Y + POWER_Y;

			int num = Max_Power_Height() - RedPipCount - YellowPipCount - GreenPipCount;

			int index;
			for (index = 0; index < num; index++) {
				Draw_Shape(*SidebarSurface, *SidebarDrawer, (ShapeSet const *)PowerPipShape, POWER_PIP_EMPTY, Point2D(x,y), rect, SHAPE_WIN_REL);
				y += POWER_PIP_HEIGHT;
			}

			index = 0;
			if (FlashCount > 0) {
				if ((FlashCount % 2) == 0) {
					Draw_Shape(*SidebarSurface, *SidebarDrawer, (ShapeSet const *)PowerPipShape, POWER_PIP_WHITE, Point2D(x,y), rect, SHAPE_WIN_REL);
					y += POWER_PIP_HEIGHT;
					index++;
				}
			}

			if (GreenPipCount > 0) {
				while (index < GreenPipCount) {
					Draw_Shape(*SidebarSurface, *SidebarDrawer, (ShapeSet const *)PowerPipShape, POWER_PIP_GREEN, Point2D(x,y), rect, SHAPE_WIN_REL);
					y += POWER_PIP_HEIGHT;
					index++;
				}
				index = 0;
			}

			if (YellowPipCount > 0) {
				while (index < YellowPipCount) {
					Draw_Shape(*SidebarSurface, *SidebarDrawer, (ShapeSet const *)PowerPipShape, POWER_PIP_YELLOW, Point2D(x,y), rect, SHAPE_WIN_REL);
					y += POWER_PIP_HEIGHT;
					index++;
				}
				index = 0;
			}

			if (RedPipCount > 0) {
				while (index < RedPipCount) {
					Draw_Shape(*SidebarSurface, *SidebarDrawer, (ShapeSet const *)PowerPipShape, POWER_PIP_RED, Point2D(x,y), rect, SHAPE_WIN_REL);
					y += POWER_PIP_HEIGHT;
					index++;
				}
				index = 0;
			}
		}
		BEnd(BENCH_POWER);
	}

	BASECLASS::Draw_It(complete);
}


/// <summary>
/// Removes one pip from the power bar.
/// This routine trims whichever color band carries more pips than it should, so that the
/// bar shrinks toward the level the player's power warrants.
/// </summary>
void PowerClass::Remove_Pip(void)
{
	int green;
	int red;
	int yellow;

	Desired_Levels(green, yellow, red);

	if (GreenPipCount > green) {
		GreenPipCount--;
	} else if (RedPipCount > red) {
		RedPipCount--;
	} else if (YellowPipCount > yellow) {
		YellowPipCount--;
	}
}


/// <summary>
/// Adds one pip to the power bar.
/// This routine grows whichever color band falls short of the level the player's power
/// warrants. It is used by the power bar logic to animate the bar one pip at a time.
/// </summary>
void PowerClass::Add_Pip(void)
{
	int green;
	int red;
	int yellow;

	Desired_Levels(green, yellow, red);

	if ( RedPipCount < red ) {
		RedPipCount++;
	} else if ( GreenPipCount < green ) {
		GreenPipCount++;
	} else if ( YellowPipCount < yellow ) {
		YellowPipCount++;
	}
}


/***********************************************************************************************
 * PowerClass::AI -- Process the power bar logic.                                              *
 *                                                                                             *
 *    Use this routine to process the power bar logic. This consists of animation effects.     *
 *                                                                                             *
 * INPUT:   input -- The player input value to be consumed or ignored as appropriate.          *
 *                                                                                             *
 *          x,y   -- Mouse coordinate parameters to use.                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/20/1994 JLB : Created.                                                                 *
 *   12/31/1994 JLB : Uses mouse coordinate parameters.                                        *
 *=============================================================================================*/
void PowerClass::AI(KeyNumType &input, Point2D const &xy)
{
	if (Map.IsSidebarActive) {

		if (!HasChanged && FlashCount > 0) {
			if (FlashTimer == 0) {
				IsToRedraw = true;
				FlashCount--;
				Map.SidebarClass::IsToRedraw = true;
				Map.Flag_To_Redraw();
				FlashTimer = POWER_FLASH_RATE;
			}
		}

		/*
		 * If the recorded power or drain value has changed we need to adjust for
		 * it.
		 */
		if (PlayerPtr->Power_Drain() != RecordedDrain || PlayerPtr->Power_Output() != RecordedPower || HasChanged) {

			IsToRedraw = true;
			Map.SidebarClass::IsToRedraw = true;
			Map.Flag_To_Redraw();

			/*
			 * Flag to flash the top of the power bar if we're adjusting the bar height.
			 */
			if (PlayerPtr->Power_Drain() != RecordedDrain || PlayerPtr->Power_Output() != RecordedPower) {
				HasChanged = true;
				Flash_Power();
			}

			RecordedDrain = PlayerPtr->Power_Drain();
			RecordedPower = PlayerPtr->Power_Output();

			int green, yellow, red;
			Desired_Levels(green, yellow, red);

			if (UpdateTimer == 0) {
				HasChanged = false;

				/*
				 * If we need to move the red level height then do so.
				 */
				if (RedPipCount != red) {
					HasChanged = true;

					if (RedPipCount > red) {
						RedPipCount--;
						Add_Pip();
					} else {
						RedPipCount++;
						Remove_Pip();
					}

				/*
				 * If we need to move the green level height then do so.
				 */
				} else if (GreenPipCount != green) {
					HasChanged = true;

					if (GreenPipCount > green) {
						GreenPipCount--;
						Add_Pip();
					} else {
						GreenPipCount++;
						Remove_Pip();
					}

				/*
				 * If we need to move the yellow level height then do so.
				 */
				} else if (YellowPipCount != yellow) {
					HasChanged = true;

					if (YellowPipCount > yellow) {
						YellowPipCount--;
						Add_Pip();
					} else {
						YellowPipCount++;
						Remove_Pip();
					}
				}

				if (HasChanged) {
					UpdateTimer = Update_Delay();
				}
			}
		}
	}

	BASECLASS::AI(input, xy);
}


/// <summary>
/// Repositions the power bar tooltip region.
/// This routine is called when the sidebar slides. It re-registers the power bar with the
/// tooltip manager so that hovering over the bar still reports the power figures.
/// </summary>
void PowerClass::Reposition_Sidebar(void)
{
	BASECLASS::Reposition_Sidebar();

	if (ToolTips != NULL) {

		ToolTip tt;
		tt.Text = TXT_NONE;
		tt.ID = GADGET_POWER;
		tt.Region.X = SidebarRect.X + POWER_X;
		tt.Region.Y = SidebarRect.Y + POWER_Y;
		tt.Region.Width = POWER_WIDTH;
		tt.Region.Height = (SidebarClass::StripClass::SideBarGeneralEnums::OBJECT_HEIGHT) * Map.Max_Visible();

		ToolTips->Remove(tt.ID);
		ToolTips->Add(&tt);
	}
}


/// <summary>
/// Fetches the help text for a sidebar gadget.
/// This routine supplies the power and drain readout that appears when the mouse lingers
/// over the power bar. Any other gadget is handed on to the sidebar to answer.
/// </summary>
/// <param name="id">The gadget to fetch the help text for.</param>
/// <returns>Returns with a pointer to the help text for the gadget specified.</returns>
/// <remarks>The power bar text is built in a shared buffer, so use it before calling
/// this routine again.</remarks>
const char * PowerClass::Help_Text(int id)
{
	static char _str[128];

	if (id == GADGET_POWER) {
		sprintf(_str, Fetch_String(TXT_POWER_DRAIN), PlayerPtr->Power, PlayerPtr->Drain);
		return(_str);
	}

	return(BASECLASS::Help_Text(id));
}
