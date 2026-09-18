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

/* $Header: /CounterStrike/GSCREEN.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : GSCREEN.H                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 12/15/94                                                     *
 *                                                                                             *
 *                  Last Update : December 15, 1994 [JLB]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "keyboard.h"

#include "mouse.hh"
#include "theater.hh"

class GadgetClass;
class SaveStreamClass;
class Surface;
class Point2D;
template<class T> class TRect;
typedef TRect<int> Rect;

extern Surface * HiddenSurface;


/*
 * This is the redraw scope requested for the next GScreenClass::Render(). The levels
 * escalate: GS_REDRAW_DIRTY leaves the decision to the individual subsystem and its
 * dirty-region flags, GS_REDRAW_TACTICAL asks for a lighter refresh of the tactical map,
 * and GS_REDRAW_ALL forces the full Draw_It redraw. GS_REDRAW_DIRTY is also the neutral
 * state that DrawFlags resets to after each render.
 */
enum GScreenRedrawFlags
{
	GS_REDRAW_DIRTY,		/// Redraw only subsystems or regions that have been marked dirty.
	GS_REDRAW_TACTICAL,		/// Redraw the tactical map.
	GS_REDRAW_ALL,			/// Redraw the entire screen including the sidebar.
};


class GScreenClass
{
	friend class Tactical;

	public:

		GScreenClass(void);
		virtual ~GScreenClass(void) {}

		virtual void Serialize(SaveStreamClass & stream);

		/*
		**	Initialization
		*/
		virtual void One_Time(void);						// One-time initializations
		virtual void Init(TheaterType = THEATER_NONE);		// Inits everything
		virtual void Init_Clear(void);						// Clears all to known state
		virtual void Init_IO(void);							// Inits button list

		/*
		**	Player I/O is routed through here. It is called every game tick.
		*/
		virtual void Input(KeyNumType & key, int & x, int & y);
		virtual void AI(KeyNumType &, Point2D const & xy);
		virtual void Add_A_Button(GadgetClass & gadget);
		virtual void Remove_A_Button(GadgetClass & gadget);

		/*
		**	Called when map needs complete updating.
		*/
		virtual void Flag_To_Redraw(GScreenRedrawFlags flags = GS_REDRAW_DIRTY);

		/*
		**	Render maintenance routine (call every game tick). Probably no need
		**	to override this in derived classes.
		*/
		virtual void Render(void);

		/*
		**	Is called when actual drawing is required. This is the function to
		**	override in derived classes.
		*/
		virtual void Draw_It(bool =false) {};

		/*
		**	This moves the hidpage up to the seenpage.
		*/
		virtual void Blit_Display(void);

		/*
		**	Changes the mouse shape as indicated.
		*/
		virtual void Set_Default_Mouse(MouseType mouse, bool wsmall) = 0;
		virtual bool Override_Mouse_Shape(MouseType mouse, bool wsmall) = 0;
		virtual void Revert_Mouse_Shape(void) = 0;
		virtual void Mouse_Small(bool wsmall) = 0;

		/*
		**	This points to the buttons that are used for input. All of the derived classes will
		**	attached their specific buttons to this list.
		*/
		static GadgetClass * Buttons;

		/*
		 * These are the pixel offsets that the screen is currently shaken by. The
		 * composited surface is blitted to the display shifted by them, with the edge
		 * that gets exposed filled in, and they ease back toward zero a pixel per frame
		 * so that a jolt from an explosion settles down of its own accord.
		 */
		int ScreenX;
		int ScreenY;

	private:
		/*
		 * This records how much of the screen the next render must redraw. It only ever
		 * escalates while a frame is pending and drops back to GS_REDRAW_DIRTY once the
		 * render is done, so a redraw request covers a single frame only.
		 */
		GScreenRedrawFlags DrawFlags;
};

void Update_Visible_Surface(Surface *surface = HiddenSurface, Rect *rect = NULL);
