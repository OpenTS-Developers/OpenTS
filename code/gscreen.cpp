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

/* $Header: /CounterStrike/GSCREEN.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : GSCREEN.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 12/15/94                                                     *
 *                                                                                             *
 *                  Last Update : January 19, 1995 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   GScreenClass::Add_A_Button -- Add a gadget to the game input system.                      *
 *   GScreenClass::Blit_Display -- Redraw the display from the hidpage to the seenpage.        *
 *   GScreenClass::Flag_To_Redraw -- Flags the display to be redrawn.                          *
 *   GScreenClass::GScreenClass -- Default constructor for GScreenClass.                       *
 *   GScreenClass::Init -- Init's the entire display hierarchy by calling all Init routines.   *
 *   GScreenClass::Init_Clear -- Sets the map to a known state.                                *
 *   GScreenClass::Init_IO -- Initializes the Button list ('Buttons').                         *
 *   GScreenClass::Init_Theater -- Performs theater-specific initializations.                  *
 *   GScreenClass::Input -- Fetches input and processes gadgets.                               *
 *   GScreenClass::One_Time -- Handles one time class setups.                                  *
 *   GScreenClass::Remove_A_Button -- Removes a gadget from the game input system.             *
 *   GScreenClass::Render -- General drawing dispatcher an display update function.            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "gscreen.h"

#include "_bench.h"
#include "_keyboar.h"
#include "_map.h"
#include "_rect.h"
#include "_surface.h"
#include "_tactica.h"
#include "_tooltip.h"
#include "_xmouse.h"
#include "bench.h"
#include "cctooltip.h"
#include "gadget.h"
#include "goptions.h"
#include "keyboard.h"
#include "savestream.h"
#include "session.h"
#include "surface.h"
#include "tactical.h"
#include "video.h"

#include "bench.hh"

#include <algorithm>

void Multiplayer_Debug_Print(void);

GadgetClass * GScreenClass::Buttons = NULL;


/***********************************************************************************************
 * GScreenClass::GScreenClass -- Default constructor for GScreenClass.                         *
 *                                                                                             *
 *    This constructor merely sets the display system, so that it will redraw the first time   *
 *    the render function is called.                                                           *
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
GScreenClass::GScreenClass(void) :
	ScreenX(0),
	ScreenY(0),
	DrawFlags(GS_REDRAW_ALL)
{
}


/// <summary>
/// Lists the members the game screen holds.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void GScreenClass::Serialize(SaveStreamClass & stream)
{
	// Buttons -- the input button list, rebuilt from scratch by Init_IO.
	stream.Serialize(ScreenX);
	stream.Serialize(ScreenY);
	// DrawFlags -- a redraw request covers a single frame, and the load asks for a full one.
}


/***********************************************************************************************
 * GScreenClass::One_Time -- Handles one time class setups.                                    *
 *                                                                                             *
 * This routine (and all those that overload it) must perform truly one-time initialization.   *
 * Such init's would normally be done in the constructor, but other aspects of the game may    *
 * not have been initialized at the time the constructors are called (such as the file system, *
 * the display, or other WWLIB subsystems), so many initializations should be deferred to the  *
 * One_Time init's.                                                                            *
 *                                                                                             *
 * Any variables set in this routine should be declared as static, so they won't be modified   *
 * by the load/save process.  Non-static variables will be over-written by a loaded game.      *
 *                                                                                             *
 * This function allocates the shadow buffer that is used for quick screen updates. If         *
 * there were any data files to load, they would be loaded at this time as well.               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Call this routine only ONCE at the beginning of the game.                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/15/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void GScreenClass::One_Time(void)
{
	Buttons = 0;
}


/***********************************************************************************************
 * GScreenClass::Init -- Init's the entire display hierarchy by calling all Init routines.     *
 *                                                                                             *
 * This routine shouldn't be overloaded.  It's the main map initialization routine, and will   *
 * perform a complete map initialization, from mixfiles to clearing the buffers.  Calling this *
 * routine results in calling every initialization routine in the entire map hierarchy.        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      theater      theater to initialize to                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/28/1994 BR : Created.                                                                  *
 *=============================================================================================*/
void GScreenClass::Init(TheaterType theater)
{
	Init_Clear();
	Init_IO();
}


/***********************************************************************************************
 * GScreenClass::Init_Clear -- Sets the map to a known state.                                  *
 *                                                                                             *
 * This routine (and those that overload it) clears any buffers and variables to a known       *
 * state.  It assumes that all buffers are allocated & valid.  The map should be displayable   *
 * after calling this function, and should draw basically an empty display.                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/28/1994 BR : Created.                                                                  *
 *=============================================================================================*/
void GScreenClass::Init_Clear(void)
{
	DrawFlags = GS_REDRAW_ALL;
}


/***********************************************************************************************
 * GScreenClass::Init_IO -- Initializes the Button list ('Buttons').                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/28/1994 BR : Created.                                                                  *
 *=============================================================================================*/
void GScreenClass::Init_IO(void)
{
	/*
	**	Reset the button list.  This means that any other elements of the map that need
	**	buttons must attach them after this routine is called!
	*/
	Buttons = 0;
}


/***********************************************************************************************
 * GScreenClass::Flag_To_Redraw -- Flags the display to be redrawn.                            *
 *                                                                                             *
 *    This function is used to flag the display system whether any rendering is needed. The    *
 *    parameter tells the system either to redraw EVERYTHING, or just that something somewhere *
 *    has changed and the individual Draw_It functions must be called. When a sub system       *
 *    determines that it needs to render something local to itself, it would call this routine *
 *    with a false parameter. If the entire screen gets trashed or needs to be rebuilt, then   *
 *    this routine will be called with a true parameter.                                       *
 *                                                                                             *
 * INPUT:   complete -- bool; Should the ENTIRE screen be redrawn?                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This doesn't actually draw the screen, it merely sets flags so that when the    *
 *             Render() function is called, the appropriate drawing steps will be performed.   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/15/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void GScreenClass::Flag_To_Redraw(GScreenRedrawFlags flags)
{
	if (TacticalMap != NULL) {
		TacticalMap->IsToRedraw = true;
	}

	if (flags != GS_REDRAW_DIRTY) {
		if (DrawFlags != GS_REDRAW_ALL) {
			DrawFlags = flags;
		}
		Map.Increment_Redraw_Counter();
	}
}


/***********************************************************************************************
 * GScreenClass::Input -- Fetches input and processes gadgets.                                 *
 *                                                                                             *
 *    This routine will fetch the keyboard/mouse input and dispatch this through the gadget    *
 *    system.                                                                                  *
 *                                                                                             *
 * INPUT:   key      -- Reference to the key code (for future examination).                    *
 *                                                                                             *
 *          x,y      -- Reference to mouse coordinates (for future examination).               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void GScreenClass::Input(KeyNumType & key, int & x, int & y)
{
	x = Get_Mouse_X();
	y = Get_Mouse_Y();

	if (Buttons != NULL) {

		/*
		**	If any buttons need redrawing, they will do so in the Input routine, and
		**	they should draw themselves to the HidPage.  So, flag ourselves for a Blit
		**	to show the newly drawn buttons.
		*/
		if (Buttons->Is_List_To_Redraw()) {
			Flag_To_Redraw();
		}

		Surface * oldpage = LogicalSurface;
		LogicalSurface = HiddenSurface;

		key = Buttons->Input();

		LogicalSurface = oldpage;

	} else {
		key = Keyboard->Check();

		if (key != 0) {
			key = Keyboard->Get();
		}
	}

	AI(key, Point2D(x, y));
}


/***********************************************************************************************
 * GScreenClass::Add_A_Button -- Add a gadget to the game input system.                        *
 *                                                                                             *
 *    This will add a gadget to the game input system. The gadget will be processed in         *
 *    subsequent calls to the GScreenClass::Input() function.                                  *
 *                                                                                             *
 * INPUT:   gadget   -- Reference to the gadget that will be added to the input system.        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void GScreenClass::Add_A_Button(GadgetClass & gadget)
{
	/*
	**	If this gadget is already in the list, remove it before adding it in:
	**	- If 1st gadget in list, use Remove_A_Button to remove it, to reset the
	**	value of 'Buttons' appropriately
	**	- Otherwise, just call the Remove function for that gadget to remove it
	**	from any list it may be in
	*/
	if (Buttons == &gadget) {
		Remove_A_Button(gadget);
	} else {
		gadget.Remove();
	}

	/*
	**	Now add the gadget to our list:
	**	- If there are not buttons, start the list with this one
	**	- Otherwise, add it to the tail of the existing list
	*/
	if (Buttons) {
		gadget.Add_Tail(*Buttons);
	} else {
		Buttons = &gadget;
	}
}


/***********************************************************************************************
 * GScreenClass::Remove_A_Button -- Removes a gadget from the game input system.               *
 *                                                                                             *
 * INPUT:   gadget   -- Reference to the gadget that will be removed from the input system.    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   'gadget' MUST be already a part of 'Buttons', or the new value of 'Buttons'     *
 *               will be invalid!                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void GScreenClass::Remove_A_Button(GadgetClass & gadget)
{
	Buttons = (GadgetClass *)gadget.Remove();
}


/***********************************************************************************************
 * GScreenClass::Render -- General drawing dispatcher an display update function.              *
 *                                                                                             *
 *    This routine should be called in the main game loop (once every game frame). It will     *
 *    call the Draw_It() function if necessary. All rendering is performed to the LogicPage    *
 *    which is set to the HIDPAGE. After rendering has been performed, the HIDPAGE is          *
 *    copied to the visible page.                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This actually updates the graphic display. As a result it can take quite a      *
 *             while to perform.                                                               *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/15/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void GScreenClass::Render(void)
{
	BStart(BENCH_GSCREEN_RENDER);

	Surface * oldpage = LogicalSurface;
	LogicalSurface = CompositeSurface;

	bool redraw = DrawFlags != GS_REDRAW_DIRTY;
	bool complete = DrawFlags == GS_REDRAW_ALL;

	TacticalMap->Render(*CompositeSurface, redraw, DRAW_PASS_PAN);
	TacticalMap->Render(*CompositeSurface, redraw, DRAW_PASS_BACKGROUND);
	Draw_It(complete);
	TacticalMap->Render(*CompositeSurface, redraw, DRAW_PASS_FOREGROUND);

	if (Buttons) Buttons->Draw_All(false);

#ifdef _DEBUG
	/*
	**	Draw the Editor's buttons
	*/
	if (Debug_Map) {
		if (Buttons) {
			Buttons->Draw_All();
		}
	}
#endif
	/*
	**	Draw the multiplayer message system to the Hidpage at this point.
	**	This way, they'll Blit along with the rest of the map.
	*/
	Session.Messages.Draw();
	if (Session.ShowInternetDebug) {
		Multiplayer_Debug_Print();
	}

	if (ToolTips != NULL) {
		ToolTips->Draw_Current();
	}

	Blit_Display();
	DrawFlags = GS_REDRAW_DIRTY;

	BEnd(BENCH_GSCREEN_RENDER);
	LogicalSurface = oldpage;
}


/***********************************************************************************************
 * GScreenClass::Blit_Display -- Redraw the display from the hidpage to the seenpage.          *
 *                                                                                             *
 *    This routine is used to copy the correct display from the HIDPAGE                        *
 *    to the SEENPAGE.                                                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1994 JLB : Created.                                                                 *
 *   05/01/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
void GScreenClass::Blit_Display(void)
{
	BStart(BENCH_BLIT_DISPLAY);
	Update_Visible_Surface(CompositeSurface, NULL);
	BEnd(BENCH_BLIT_DISPLAY);
}


/// <summary>
/// Repaints the dialog controls that the last frame drew over.
/// The dialogs are ordinary child windows that paint themselves onto the game's own
/// surfaces, so a frame put on top of them takes their pixels with it. Windows is asked
/// to repaint them straight away, and the controls are grandchildren of the main window
/// rather than children, so the whole subtree has to be included.
/// </summary>
void Heal_Dialog_Controls(void)
{
	if (_dialog_count <= 0 || MainWindow == NULL) {
		return;
	}

	for (HWND child = GetWindow(MainWindow, GW_CHILD); child != NULL; child = GetWindow(child, GW_HWNDNEXT)) {
		if (IsWindowVisible(child)) {
			RedrawWindow(child, NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_ALLCHILDREN);
		}
	}
}


/// <summary>
/// Presents a rendered surface onto the visible surface.
/// This is the low level routine that gets a finished frame in front of the player. The
/// destination is the visible surface, adjusted for the screen shake and for a sidebar
/// sitting on the left, and the source is narrowed when the tactical map is zoomed. Any
/// strip that the shake has left uncovered is filled with black.
/// </summary>
/// <param name="surface">The surface holding the frame to present.</param>
/// <param name="rect">The portion of the surface to present, or NULL for all of it.</param>
void Update_Visible_Surface(Surface *surface, Rect *rect)
{
	Rect fill_rect;

	if (rect == NULL) {
		fill_rect = surface->Get_Rect();
		rect = &fill_rect;
	}

	Rect dest_rect(0, 0, surface->Get_Width(), surface->Get_Height());

	/// Screen shake handling
	if (Map.ScreenX == 0 && Map.ScreenY == 0) {
		// Do nothing
	} else {
		if (Map.ScreenX > 0) {
			dest_rect.X += Map.ScreenX;
			dest_rect.Width -= Map.ScreenX;
		} else if (Map.ScreenX < 0) {
			dest_rect.Width += Map.ScreenX;
		}
		if (Map.ScreenY > 0) {
			dest_rect.Y += Map.ScreenY;
			dest_rect.Height -= Map.ScreenY;
		} else if (Map.ScreenY < 0) {
			dest_rect.Height += Map.ScreenY;
		}
	}

	/// Adjust for sidebar position
	if (!Options.IsSidebarOnRight && !Debug_Map) {
		dest_rect.X += std::max(std::min(SidebarSurface->Get_Width(), VisibleRect.Width - dest_rect.Width), 0);
	}

	/*
	 * Copy input rect to source rect
	 */
	Rect src_rect = *rect;

	/// Apply zoom factor if tactical map is zoomed
	if (TacticalMap && (TacticalMap->ZoomFactor != 1.0)) {

		/*
		 * Compute zoomed source rect centered
		 */
		int zoom_surface_width = surface->Get_Width();
		int zoom_surface_height = surface->Get_Height();

		Rect tmp;
		double zoomed_width = (double)zoom_surface_width / TacticalMap->ZoomFactor;
		tmp.X = (int)(((double)zoom_surface_width - zoomed_width) / 2.0);
		double zoomed_height = (double)zoom_surface_height / TacticalMap->ZoomFactor;
		tmp.Y = (int)(((double)zoom_surface_height - zoomed_height) / 2.0);
		tmp.Width = (int)zoomed_width;
		tmp.Height = (int)zoomed_height;

		src_rect = tmp;

	} else {

		if (src_rect.Width >= dest_rect.Width) {
			src_rect.Width = dest_rect.Width;
		}
		if (src_rect.Height >= dest_rect.Height) {
			src_rect.Height = dest_rect.Height;
		}

	}

	if (Map.ScreenY < 0) {
		src_rect.Y -= Map.ScreenY;
	}

	if (Map.ScreenX < 0) {
		src_rect.X -= Map.ScreenX;
	}

	/// Draw filler for X offset
	if (Map.ScreenX != 0) {
		fill_rect.Set(fill_rect.X, 0, abs(Map.ScreenX), surface->Get_Height());
		fill_rect.X = Map.ScreenX < 0 ? dest_rect.X + dest_rect.Width : dest_rect.X - Map.ScreenX;

		VisibleSurface->Fill_Rect(VisibleSurface->Get_Rect(), fill_rect, 0);
	}

	/// Draw filler for Y offset
	if (Map.ScreenY != 0) {
		fill_rect.Set(0, fill_rect.Y, surface->Get_Width(), abs(Map.ScreenY));
		fill_rect.Y = Map.ScreenY < 0 ? dest_rect.Y + dest_rect.Height : dest_rect.Y - Map.ScreenY;

		VisibleSurface->Fill_Rect(VisibleSurface->Get_Rect(), fill_rect, 0);
	}

	/*
	 * Now blit the source surface to the visible surface
	 */
	VisibleSurface->Blit_From(dest_rect, *surface, src_rect, false, true);

	Heal_Dialog_Controls();
	Video_Present_If_Dirty();
}


/// <summary>
/// Performs the per frame logic for the game screen.
/// This routine eases the screen shake offset back toward its resting position, so that a
/// jolt requested by an explosion or a superweapon settles down of its own accord. Derived
/// screens call this routine from their own logic pass.
/// </summary>
/// <param name="xy">The current mouse position.</param>
void GScreenClass::AI(KeyNumType &, Point2D const & xy)
{
	ScreenX < 0 ? ScreenX += 1 : ScreenX > 0 ? ScreenX -= 1 : 0;
	ScreenY < 0 ? ScreenY += 1 : ScreenY > 0 ? ScreenY -= 1 : 0;
}
