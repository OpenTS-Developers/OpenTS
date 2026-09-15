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

/* $Header: /CounterStrike/GAUGE.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : GAUGE.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic, Maria del Mar McCready Legg                   *
 *                                                                                             *
 *                   Start Date : 01/15/95                                                     *
 *                                                                                             *
 *                  Last Update : January 16, 1995 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   GaugeClass::Action -- Handles input events for the gauge.                                 *
 *   GaugeClass::Draw_Me -- Draws the body of the gauge.                                       *
 *   GaugeClass::Pixel_To_Value -- Convert a pixel offset into a gauge value.                  *
 *   GaugeClass::Set_Maximum -- Sets the maximum value for the gauge.                          *
 *   GaugeClass::Set_Value -- Set the value of the gauge.                                      *
 *   GaugeClass::Value_To_Pixel -- Convert gauge value to pixel offset.                        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "gauge.h"

#include "_convert.h"
#include "_surface.h"
#include "control.h"
#include "convert.h"
#include "dialog.h"
#include "dsurface.h"
#include "scheme.h"
#include "vector.h"
#include "xmouse.h"

#include "color.hh"

#include <algorithm>


/***************************************************************************
 * GaugeClass::GaugeClass -- class constructor                             *
 *                                                                         *
 * INPUT:    id                     -- button ID                           *
 *                                                                         *
 *             x,y                     -- upper-left corner, in pixels     *
 *                                                                         *
 *             w,h                     -- width, height, in pixels         *
 *                                                                         *
 * OUTPUT:   none.                                                         *
 *                                                                         *
 * WARNINGS: none.                                                         *
 *                                                                         *
 * HISTORY:  01/05/1995 MML : Created.                                     *
 *=========================================================================*/
GaugeClass::GaugeClass(unsigned id, int x, int y, int w, int h)
	: BASECLASS(id, x, y, w, h, LEFTHELD|LEFTPRESS|LEFTRELEASE, true)
{
	Set_Maximum(255);
	Set_Value(0);

	HasThumb = true;
	IsHorizontal = (w > h);
	IsColorized = true;

	ClickDiff = 0;
}


/***********************************************************************************************
 * GaugeClass::Set_Maximum -- Sets the maximum value for the gauge.                            *
 *                                                                                             *
 *    This routine will set the maximum value for the gauge. This is the largest value that    *
 *    the current setting may reach. The ability to change this allows the gauge to use and    *
 *    return values that are convenient for the programmer's current needs.                    *
 *                                                                                             *
 * INPUT:   value -- The value to use as the gauge maximum.                                    *
 *                                                                                             *
 * OUTPUT:  bool; Was the gauge maximum changed? A false indicates that the specified value    *
 *                already matches the current maximum.                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int GaugeClass::Set_Maximum(int value)
{
	if (value != MaxValue) {
		MaxValue = value;
		Flag_To_Redraw();
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * GaugeClass::Set_Value -- Set the value of the gauge.                                        *
 *                                                                                             *
 *    This routine will set the current value for the gauge. This value is clipped to the      *
 *    limits of the gauge maximum.                                                             *
 *                                                                                             *
 * INPUT:   value -- The value to set at the new current value.                                *
 *                                                                                             *
 * OUTPUT:  bool; Was the current setting changed? A false indicates that the setting          *
 *                specified is the same as what was already there.                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int GaugeClass::Set_Value(int value)
{
	value = std::clamp(value, 0, MaxValue);
	if (value != CurValue) {
		CurValue = value;
		Flag_To_Redraw();
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * GaugeClass::Pixel_To_Value -- Convert a pixel offset into a gauge value.                    *
 *                                                                                             *
 *    Use this routine to convert the specified pixel offset into a gauge value. This is used  *
 *    in translating mouse clicks into a corresponding setting for the gauge.                  *
 *                                                                                             *
 * INPUT:   pixel -- The pixel offset form the start of the gauge.                             *
 *                                                                                             *
 * OUTPUT:  Returns with the setting value in gauge coordinates.                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int GaugeClass::Pixel_To_Value(int pixel)
{
	int maximum;

	if (IsHorizontal) {
		pixel -= X+1;
		maximum = Width;
	} else {
		pixel -= Y+1;
		maximum = Height;
	}
	maximum -= 2;
	pixel = std::clamp(pixel, 0, maximum);
	return(int(MaxValue * ((double)pixel / maximum)));
}


/***********************************************************************************************
 * GaugeClass::Value_To_Pixel -- Convert gauge value to pixel offset.                          *
 *                                                                                             *
 *    Use this routine to convert the specified gauge value into a pixel offset from the       *
 *    star of the gauge. This is used for thumb positioning.                                   *
 *                                                                                             *
 * INPUT:   value -- The value to convert to a pixel offset.                                   *
 *                                                                                             *
 * OUTPUT:  Returns with the pixel offset of the specified value from the start of the         *
 *          gauge.                                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int GaugeClass::Value_To_Pixel(int value)
{
	int maximum;
	int start;
	if (IsHorizontal) {
		maximum = Width;
		start = X;
	} else {
		maximum = Height;
		start = Y;
	}
	maximum -= 2;
	return(int(start + (maximum * ((double)value / MaxValue))));
}


/***********************************************************************************************
 * GaugeClass::Draw_Me -- Draws the body of the gauge.                                         *
 *                                                                                             *
 *    This routine will draw the body of the gauge if necessary.                               *
 *                                                                                             *
 * INPUT:      forced   -- Should the gauge be redrawn regardless of the current redraw flag?  *
 *                                                                                             *
 * OUTPUT:     bool; Was the gauge redrawn?                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:    01/16/1995 JLB : Created.                                                       *
 *=============================================================================================*/
int GaugeClass::Draw_Me(int forced)
{
	if (BASECLASS::Draw_Me(forced)) {

		/*
		**	Hide the mouse
		*/
		Conditional_Hide_Mouse(Rect(X, Y, Width, Height));

		/*
		**	Draw the body & set text color
		*/
		Draw_Box (Rect(X, Y, Width, Height), BOXSTYLE_DOWN, true);

		/*
		**	Colorize the inside of the gauge if indicated.
		*/
		if (IsColorized) {
			int middle = Value_To_Pixel(CurValue);
			int color = ColorSchemes[GadgetClass::Get_Color_Scheme()]->Bright;
			if (IsHorizontal) {
				if (middle >= (X + 1))
					LogicalSurface->Fill_Rect(Rect(X+1, Y+1, middle-X-1, Height-2), NormalDrawer->Convert_Pixel(color));
			} else {
				if (middle >= (Y + 1))
					LogicalSurface->Fill_Rect(Rect(X+1, Y+1, Width-2, middle-Y-1), NormalDrawer->Convert_Pixel(color));
			}
		}

		if (HasThumb) {
			Draw_Thumb();
		}

		/*
		**	Display the mouse
		*/
		Conditional_Show_Mouse();
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * GaugeClass::Action -- Handles input events for the gauge.                                   *
 *                                                                                             *
 *    This routine will handle input event processing for the gauge. It will adjust the        *
 *    current setting of the gauge according to the mouse position.                            *
 *                                                                                             *
 * INPUT:   flags -- The input event that is the reason for this function call.                *
 *          key   -- The key code that caused the event.                                       *
 *                                                                                             *
 * OUTPUT:  bool; Was the even recognized, processed, and no further gadget scanning is        *
 *                desired (for this pass).                                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 * HISTORY:                                                                                    *
 *   01/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int GaugeClass::Action(unsigned flags, KeyNumType &key)
{
	/*
	**	If there's no thumb on this gauge, it's a display-only device.
	*/
	if (!HasThumb) {
		return(false);
	}

	/*
	**	We might end up clearing the event bits. Make sure that the sticky
	**	process is properly updated anyway.
	*/
	Sticky_Process(flags);

	/*
	**	If the thumb is currently being "dragged around", then update the slider
	**	position according to the mouse position. In all other cases, ignore the
	**	button being held down.
	*/
	if ((flags & LEFTPRESS) || ((flags & LEFTHELD) && StuckOn == this)) {

		/*
		**	Compute the difference between where we clicked, and the edge of
		**	the thumb (only if we clicked on the thumb.)
		*/
		if (flags & LEFTPRESS) {
			int curpix = Value_To_Pixel(CurValue);
			int clickpix = (IsHorizontal ? Get_Mouse_X() : Get_Mouse_Y());

			if ( (clickpix > curpix) && (clickpix - curpix) < Thumb_Pixels()) {
				ClickDiff = (clickpix - curpix);
			} else {
				ClickDiff = 0;
			}

			int testval = Pixel_To_Value(IsHorizontal ?
				Get_Mouse_X() - ClickDiff : Get_Mouse_Y() - ClickDiff);

			/*
			**	Correct for round-down errors in Pixel_To_Value() and
			**	Value_To_Pixel(); make ClickDiff exactly right so that
			**	at this point, Get_Mouse_n() - ClickDiff converts to
			**	CurValue.
			*/
			while (testval < CurValue && ClickDiff > 0) {
				ClickDiff--;
				testval = Pixel_To_Value(IsHorizontal ?
				Get_Mouse_X() - ClickDiff : Get_Mouse_Y() - ClickDiff);
			}
		}

		/*
		**	If no change occurred in the gauge, just call Control's Action routine,
		**	but turn off the flags so it won't fill in 'key' with the button ID.
		**	Thus, no button ID will be returned by Input.
		*/
		if (!Set_Value(Pixel_To_Value(IsHorizontal ?
			Get_Mouse_X() - ClickDiff : Get_Mouse_Y() - ClickDiff))) {

			flags &= ~(LEFTHELD|LEFTRELEASE|LEFTPRESS);
			BASECLASS::Action(0, key);
			key = KN_NONE;
			return(true);
		}

	} else {

		/*
		**	Ignore the left mouse button being held down if this gauge is not
		**	currently in "sticky" mode. This allows processing of the LEFTPRESS
		**	by any derived classes such that this gauge can be more closely
		**	controlled.
		*/
		flags &= ~LEFTHELD;
	}
	return(BASECLASS::Action(flags, key));
}


/***********************************************************************************************
 * GaugeClass::Draw_Thumb -- Draws the body of the gauge.                                      *
 *                                                                                             *
 *    This routine will draw the body of the gauge if necessary.                               *
 *                                                                                             *
 * INPUT:      none.                                                                           *
 *                                                                                             *
 * OUTPUT:     none.                                                                           *
 *                                                                                             *
 * WARNINGS:   none.                                                                           *
 *                                                                                             *
 * HISTORY:    01/16/1995 MML : Created.                                                       *
 *=============================================================================================*/
void GaugeClass::Draw_Thumb(void)
{
	int x = Value_To_Pixel(CurValue);

	if ((x + 4) > Value_To_Pixel(MaxValue)) {
		x = Value_To_Pixel(MaxValue) - 2;
	}

	if (x < X) {
		x = X;
	}

	if (IsHorizontal) {
		Draw_Box(Rect(x, Y, 4, Height), BOXSTYLE_RAISED, true);
	} else {
		Draw_Box(Rect(X, x, Width, 4),  BOXSTYLE_RAISED, true);
	}
}


/***********************************************************************************************
 * TriColorGaugeClass::TriColorGaugeClass -- Constructor for 3 color (red\yellow\green) gauge. *
 *                                                                                             *
 *    This routine will draw the body of the gauge if necessary.                               *
 *                                                                                             *
 * INPUT:      See below.                                                                      *
 *                                                                                             *
 * OUTPUT:     none.                                                                           *
 *                                                                                             *
 * WARNINGS:   none.                                                                           *
 *                                                                                             *
 * HISTORY:    01/16/1995 MML : Created.                                                       *
 *=============================================================================================*/
TriColorGaugeClass::TriColorGaugeClass(unsigned id, int x, int y, int w, int h)
	: BASECLASS(id, x, y, w, h),
	RedLimit(0),				// maximum value for red
	YellowLimit(0)				// maximum value for yellow
{
}


/***********************************************************************************************
 * TriColorGaugeClass::Set_Red_Limit -- Set the value for the red area of gauge.               *
 *                                                                                             *
 * INPUT:      int value.                                                                      *
 *                                                                                             *
 * OUTPUT:     bool true of false.                                                             *
 *                                                                                             *
 * WARNINGS:   none.                                                                           *
 *                                                                                             *
 * HISTORY:    01/16/1995 MML : Created.                                                       *
 *=============================================================================================*/
int TriColorGaugeClass::Set_Red_Limit(int value)
{
	if (value >= 0 && value < MaxValue) {

//		if (value > YellowLimit) {
//			RedLimit = YellowLimit;
//			YellowLimit = value;
//		} else {
			RedLimit = value;
//		}
		Flag_To_Redraw();
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * TriColorGaugeClass::Set_Yellow_Limit -- Set the value for the yellow area of gauge.         *
 *                                                                                             *
 * INPUT:      int value.                                                                      *
 *                                                                                             *
 * OUTPUT:     bool true of false.                                                             *
 *                                                                                             *
 * WARNINGS:   none.                                                                           *
 *                                                                                             *
 * HISTORY:    01/16/1995 MML : Created.                                                       *
 *=============================================================================================*/
int TriColorGaugeClass::Set_Yellow_Limit(int value)
{
	if (value >= 0 && value < MaxValue) {

//		if (value < RedLimit) {
//			YellowLimit = RedLimit;
//			RedLimit = value;
//		} else {
			YellowLimit = value;
//		}
		Flag_To_Redraw();
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * TriColorGaugeClass::Draw_Me -- Draw the tri color gauge.                                    *
 *                                                                                             *
 * INPUT:      int forced -- draw or not?                                                      *
 *                                                                                             *
 * OUTPUT:     bool true of false.                                                             *
 *                                                                                             *
 * WARNINGS:   none.                                                                           *
 *                                                                                             *
 * HISTORY:    01/16/1995 MML : Created.                                                       *
 *=============================================================================================*/
int TriColorGaugeClass::Draw_Me(int forced)
{
	if (BASECLASS::Draw_Me(forced)) {

		/*
		**	Hide the mouse
		*/
		Conditional_Hide_Mouse(Rect(X, Y, Width, Height));

		/*
		**	Draw the body & set text color
		*/
		Draw_Box (Rect(X, Y, Width, Height), (IsDisabled ? BOXSTYLE_RAISED : BOXSTYLE_DOWN), true);

		/*
		**	Colorize the inside of the gauge if indicated.
		*/
		int red    = Value_To_Pixel(RedLimit);
		int yellow = Value_To_Pixel(YellowLimit);
		int middle = Value_To_Pixel(CurValue);

		if (CurValue <= RedLimit) {
			if (IsHorizontal) {
				LogicalSurface->Fill_Rect(Rect(X+1, Y+1, middle-X+1, Height-2), NormalDrawer->Convert_Pixel(PINK));
			} else {
				LogicalSurface->Fill_Rect(Rect(X+1, Y+1, Width-2, middle-Y+1), NormalDrawer->Convert_Pixel(PINK));
			}
		} else if (CurValue > RedLimit && CurValue <= YellowLimit) {
			if (IsHorizontal) {
				LogicalSurface->Fill_Rect(Rect(X+1, Y+1, red-X+1,    Height-2), NormalDrawer->Convert_Pixel(PINK));
				LogicalSurface->Fill_Rect(Rect(red-X+1, Y+1, middle-X-1, Height-2), NormalDrawer->Convert_Pixel(YELLOW));
			} else {
				LogicalSurface->Fill_Rect(Rect(X+1, Y+1, Width-2, red-Y+1),    NormalDrawer->Convert_Pixel(PINK));
				LogicalSurface->Fill_Rect(Rect(X+1, red-Y+1, X+Width-2, middle-Y+1), NormalDrawer->Convert_Pixel(YELLOW));
			}
		} else if (CurValue > YellowLimit && CurValue <= MaxValue) {

			if (IsHorizontal) {
				LogicalSurface->Fill_Rect(Rect(X+1,    Y+1, red-X+1,    Height-2), NormalDrawer->Convert_Pixel(PINK));
				LogicalSurface->Fill_Rect(Rect(red,    Y+1, yellow-X+1, Height-2), NormalDrawer->Convert_Pixel(YELLOW));
				LogicalSurface->Fill_Rect(Rect(yellow, Y+1, middle-X+1, Height-2), NormalDrawer->Convert_Pixel(GREEN));
			} else {
				LogicalSurface->Fill_Rect(Rect(X+1, Y+1,    Width-2, red-Y+1),    NormalDrawer->Convert_Pixel(PINK));
				LogicalSurface->Fill_Rect(Rect(X+1, red,    Width-2, yellow-Y+1), NormalDrawer->Convert_Pixel(YELLOW));
				LogicalSurface->Fill_Rect(Rect(X+1, yellow, Width-2, middle-Y+1), NormalDrawer->Convert_Pixel(GREEN));
			}
		}

		if (HasThumb) {
			Draw_Thumb();
		}

		/*
		**	Display the mouse
		*/
		Conditional_Show_Mouse();
		return(true);
	}
	return(false);
}
