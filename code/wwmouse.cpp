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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Library/wwmouse.cpp                          $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/11/97 10:14a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Callback_Process_Mouse -- Mouse O/S callback function.                                    *
 *   WWMouseClass::WWMouseClass -- Constructor for mouse handler object.                       *
 *   WWMouseClass::~WWMouseClass -- Destructor for mouse handler object.                       *
 *   WWMouseClass::Get_Mouse_State -- Fetch the current mouse visibility state.                *
 *   WWMouseClass::Set_Cursor -- Set the mouse cursor shape.                                   *
 *   WWMouseClass::Is_Data_Valid -- Determines if there is valid shape image data.             *
 *   WWMouseClass::Validate_Copy_Buffer -- Checks for and validates the background copy buffer.*
 *   WWMouseClass::Matching_Rect -- Finds rectangle of current cursor position & size.         *
 *   WWMouseClass::Save_Background -- Saves the background to a copy buffer.                   *
 *   WWMouseClass::Restore_Background -- Restores the image back where it came from.           *
 *   WWMouseClass::Draw_Mouse -- Manually draw the mouse to the surface specified.             *
 *   WWMouseClass::Erase_Mouse -- Restores the surface after a Draw_Mouse call.                *
 *   WWMouseClass::Raw_Draw_Mouse -- Draws the mouse to the surface specified.                 *
 *   WWMouseClass::Low_Show_Mouse -- Shows the mouse and saves the background.                 *
 *   WWMouseClass::Low_Hide_Mouse -- Restores the surface image in order to hide the mouse.    *
 *   WWMouseClass::Show_Mouse -- Shows the mouse on the visible surface.                       *
 *   WWMouseClass::Hide_Mouse -- Hides the mouse from the visible surface.                     *
 *   WWMouseClass::Capture_Mouse -- Capture the mouse into the mouse handler region.           *
 *   WWMouseClass::Release_Mouse -- Release the mouse back to the O/S.                         *
 *   WWMouseClass::Conditional_Hide_Mouse -- Hides the mouse if it would overlap the region spe*
 *   WWMouseClass::Conditional_Show_Mouse -- Releases the mouse hiding region tracking.        *
 *   WWMouseClass::Convert_Coordinate -- Convert an O/S coordinate into a logical coordinate.  *
 *   WWMouseClass::Get_Bounded_Position -- Fetches the mouse position from the O/S.            *
 *   WWMouseClass::Update_Mouse_Position -- Updates the mouse position to match that specified.*
 *   WWMouseClass::Process_Mouse -- Mouse processing callback routine.                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "wwmouse.h"

#include "dbgprint.h"
#include "misc.h"
#include "video.h"
#include "vidscale.h"
#include "win.h"
#include "wincursor.h"


/// <summary>
/// Constructs the mouse handler object.
/// The handler is bound to the game window and derives the confining rectangle from it.
/// The mouse begins in a non-captured state.
/// </summary>
/// <param name="window">Handle to the game window that the mouse is bound to.</param>
WWMouseClass::WWMouseClass(HWND window) :
	MouseState(-1),
	IsCaptured(false),
	Window(window),
	ConfiningRect(RECT_NONE)
{
	Calc_Confining_Rect();
}


/// <summary>
/// Recalculates the screen rectangle that the mouse is confined to.
/// This routine converts the game window's client area into screen coordinates. The mouse
/// capture logic clips the cursor to this rectangle, so the window creation and window
/// move handlers call this routine to keep it current.
/// </summary>
void WWMouseClass::Calc_Confining_Rect(void)
{
	RECT rect;
	GetClientRect(Window, &rect);

	POINT point;
	point.x = rect.left;
	point.y = rect.top;
	ClientToScreen(Window, &point);

	POINT lr;
	lr.x = rect.right;
	lr.y = rect.bottom;
	ClientToScreen(Window, &lr);

	ConfiningRect = Rect(point.x, point.y, lr.x-point.x, lr.y-point.y);
	DebugString("Calc_Confining_Rect(%d,%d,%d,%d)\n", point.x, point.y, lr.x-point.x, lr.y-point.y);
}


/***********************************************************************************************
 * WWMouseClass::Get_Mouse_State -- Fetch the current mouse visibility state.                  *
 *                                                                                             *
 *    This routine is used to retrieve the current mouse state as it relates to visiblity.     *
 *    By using this routine it is possible to determine if the mouse is visible.               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current mouse visibility state. If the return value is less than  *
 *          0 (i.e., negative), then the mouse is hidden.                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int WWMouseClass::Get_Mouse_State(void) const
{
	if (!Is_Captured()) {
		ShowCursor(FALSE);
		int state = ShowCursor(TRUE);
		return(state);
	}
	return(MouseState);
}


/***********************************************************************************************
 * WWMouseClass::Set_Cursor -- Set the mouse cursor shape.                                     *
 *                                                                                             *
 *    This routine sets the mouse cursor image and hot-spot. The shape only applies to the     *
 *    mouse when it is captured (the normal case). Repeated calls to this routine is used      *
 *    to give the mouse animation.                                                             *
 *                                                                                             *
 * INPUT:   xhotspot, yhotspot   -- The X,Y offset from the upper left corner of the shape     *
 *                                  that specifies the hot-spot of the image. Positive values  *
 *                                  are right and down from the upper left corner.             *
 *                                                                                             *
 *          cursor   -- Pointer to the shape data.                                             *
 *                                                                                             *
 *          shape    -- The shape number to use within the shape data set specified.           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Set_Cursor(Point2D const & hotspot, ShapeSet const * cursor, int shape)
{
	if (cursor != NULL) {
		Win_Cursor_Set(cursor, shape, hotspot.X, hotspot.Y, Is_Captured());
	}
}


/***********************************************************************************************
 * WWMouseClass::Show_Mouse -- Shows the mouse on the visible surface.                         *
 *                                                                                             *
 *    This routine is called when the mouse can be shown on the visible surface.               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Show_Mouse(void)
{
	if (!Is_Captured()) {
		ShowCursor(TRUE);
	} else {
		MouseState++;
		if (MouseState > 0) MouseState = 0;
		Win_Cursor_Set_Visible(!Is_Hidden());
	}
}


/***********************************************************************************************
 * WWMouseClass::Hide_Mouse -- Hides the mouse from the visible surface.                       *
 *                                                                                             *
 *    This routine is called when the mouse is desired to be hidden from the visible surface.  *
 *    Typically, this must occur if the pixels where the mouse is located will be accessed.    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Hide_Mouse(void)
{
	if (!Is_Captured()) {
		ShowCursor(FALSE);
	} else {
		MouseState--;
		Win_Cursor_Set_Visible(!Is_Hidden());
	}
}


/***********************************************************************************************
 * WWMouseClass::Capture_Mouse -- Capture the mouse into the mouse handler region.             *
 *                                                                                             *
 *    This routine will confine the mouse to the confining rectangle and take over drawing     *
 *    of the mouse image from the operating system. The typical state is to keep the mouse     *
 *    captured throughout the lifetime of the owning program.                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Capture_Mouse(void)
{
	if (this != NULL && !Is_Captured()) {
		DebugString("Capture_Mouse()\n");
		Hide_Mouse();
		IsCaptured = true;

		/*
		 * The game's pointer is the O/S pointer, so its display count has to come
		 * back up; it was left negative while the game drew a pointer of its own.
		 */
		while (ShowCursor(TRUE) < 0) {}

		/*
		 * There is no exclusive display mode any more, so the pointer is kept inside
		 * the window by hand while the game covers the screen.
		 */
		if (!WindowedMode) {
			RECT clip_rect;
			GetClientRect(Window, &clip_rect);
			ClientToScreen(Window, (LPPOINT)&clip_rect.left);
			ClientToScreen(Window, (LPPOINT)&clip_rect.right);
			ClipCursor(&clip_rect);
		}

		Show_Mouse();
	}
}


/***********************************************************************************************
 * WWMouseClass::Release_Mouse -- Release the mouse back to the O/S.                           *
 *                                                                                             *
 *    This is the counterpart routine to Capture_Mouse. This routine will return the drawing   *
 *    and movement controls back to the operating system. Although the mouse will probably     *
 *    be able to roam outside the confining rectangle, the coordinates returned by this class  *
 *    are clipped to the confining rectangle anyway. This gives the impression that the mouse  *
 *    is still at a legal position. The presumption is that the mouse needs to be released to  *
 *    the O/S for reasons outside of the game itself. As such, the shouldn't detect any        *
 *    illegal mouse position.                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   All mouse shape changes won't be relected while the mouse is released. The O/S  *
 *             handles drawing the mouse in that case.                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Release_Mouse(void)
{
	if (this != NULL && Is_Captured()) {
		DebugString("Release_Mouse()\n");
		Hide_Mouse();
		IsCaptured = false;
		ClipCursor(NULL);
		if (GetCapture() == Window) ReleaseCapture();
		while (ShowCursor(TRUE) < 0) {}

		SetCursor(LoadCursor(NULL, IDC_ARROW));

		Show_Mouse();
	}
}


/***********************************************************************************************
 * WWMouseClass::Conditional_Hide_Mouse -- Hides the mouse if it would overlap the region spec *
 *                                                                                             *
 *    This routine will hide the mouse if it lies within the region specified or if it moves   *
 *    within the region.                                                                       *
 *                                                                                             *
 * INPUT:   rect  -- The rectangle that the mouse should not be drawn within.                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Conditional_Hide_Mouse(Rect )
{
	Hide_Mouse();
}


/***********************************************************************************************
 * WWMouseClass::Conditional_Show_Mouse -- Releases the mouse hiding region tracking.          *
 *                                                                                             *
 *    This routine will release the region hiding tracking that was set up with a previous     *
 *    call to Conditional_Hide_Mouse().                                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Conditional_Show_Mouse(void)
{
	Show_Mouse();
}


/***********************************************************************************************
 * WWMouseClass::Convert_Coordinate -- Convert an O/S coordinate into a logical coordinate.    *
 *                                                                                             *
 *    Sometimes you come across system mouse coordinates and they need to be converted into    *
 *    game logical coordinates. This routine will perform this function.                       *
 *                                                                                             *
 * INPUT:   x,y   -- Reference to the coordinates that will be converted into game logical     *
 *                   coordinates.                                                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The coordinates will be bound as well as transformed by the confining rectangle.*
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Convert_Coordinate(int & x, int & y) const
{
	/*
	**	Convert the mouse position to legal bounds.
	*/
	POINT point;
	point.x = x - ConfiningRect.X;
	point.y = y - ConfiningRect.Y;
	Window_Point_To_Game(point);

	VideoScaleInfo const & scale = Video_Get_Scale_Info();
	x = point.x;
	y = point.y;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x >= scale.GameWidth) x = scale.GameWidth-1;
	if (y >= scale.GameHeight) y = scale.GameHeight-1;
}


/***********************************************************************************************
 * WWMouseClass::Get_Bounded_Position -- Fetches the mouse position from the O/S.              *
 *                                                                                             *
 *    Fetches the mouse coordinates from the O/S and converts them into logical coordinates.   *
 *                                                                                             *
 * INPUT:   x,y   -- Reference to the coordinates that will be set by this routine.            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Get_Bounded_Position(int & x, int & y) const
{
	/*
	**	Get the mouse's current real cursor position
	*/
	POINT pt;
	GetCursorPos(&pt);			// get the current cursor position
	x = pt.x;
	y = pt.y;
	Convert_Coordinate(x, y);
}
