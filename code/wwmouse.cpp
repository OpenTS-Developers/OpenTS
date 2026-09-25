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

#include "_convert.h"
#include "convert.h"
#include "dbgprint.h"
#include "globals.h"
#include "goptions.h"
#include "misc.h"
#include "sdl/sdlwindow.h"
#include "shapeset.h"
#include "video.h"
#include "vidscale.h"
#include "win.h"

#include <vector>


/// <summary>
/// Works out how much larger than its shape the cursor should be drawn.
/// </summary>
/// <returns>int; A whole multiple between one and eight.</returns>
static int Cursor_Scale(void)
{
	if (Options.CursorScale < 0) {
		return(1);
	}

	if (Options.CursorScale > 0) {
		return(Options.CursorScale > 8 ? 8 : Options.CursorScale);
	}

	VideoScaleInfo const & scale = Video_Get_Scale_Info();
	float smaller = scale.ScaleX < scale.ScaleY ? scale.ScaleX : scale.ScaleY;

	int result = (int)(smaller + 0.5f);
	if (result < 1) result = 1;
	if (result > 8) result = 8;
	return(result);
}


/// <summary>
/// Draws one shape frame into a cursor.
/// The canvas covers the shape's whole frame rather than the trimmed part that holds
/// pixels, so the hotspot, which is measured from the frame's corner, still lands in the
/// right place. Palette entry zero is the transparent one.
/// </summary>
/// <returns>The cursor, or NULL if it could not be built.</returns>
static SDL_Cursor * Build_Cursor(ShapeSet const * shape, int frame, int hotx, int hoty, int scale)
{
	if (shape == NULL || MouseDrawer == NULL) {
		return(NULL);
	}

	Rect rect = shape->Get_Rect(frame);
	unsigned char const * data = (unsigned char const *)shape->Get_Data(frame);

	if (!rect.Is_Valid() || data == NULL) {
		return(NULL);
	}

	int width = shape->Get_Width() * scale;
	int height = shape->Get_Height() * scale;

	if (width <= 0 || height <= 0) {
		return(NULL);
	}

	std::vector<unsigned int> bits((size_t)width * height, 0);

	// The shapes are palette indices and the primary is 565, so the drawer's table is
	// what turns one into the other.
	unsigned short const * table = (unsigned short const *)MouseDrawer->Get_Translate_Table();

	for (int y = 0; y < rect.Height; y++) {
		for (int x = 0; x < rect.Width; x++) {

			unsigned char index = data[y * rect.Width + x];
			if (index == 0) {
				continue;
			}

			unsigned short pixel = table[index];
			unsigned int red = ((pixel >> 11) & 0x1F) << 3;
			unsigned int green = ((pixel >> 5) & 0x3F) << 2;
			unsigned int blue = (pixel & 0x1F) << 3;
			unsigned int argb = 0xFF000000U | (red << 16) | (green << 8) | blue;

			for (int suby = 0; suby < scale; suby++) {
				unsigned int * row = bits.data() + ((rect.Y + y) * scale + suby) * width + (rect.X + x) * scale;
				for (int subx = 0; subx < scale; subx++) {
					row[subx] = argb;
				}
			}
		}
	}

	int cursor_hotx = hotx * scale;
	int cursor_hoty = hoty * scale;
	if (cursor_hotx < 0) cursor_hotx = 0;
	if (cursor_hoty < 0) cursor_hoty = 0;
	if (cursor_hotx >= width) cursor_hotx = width - 1;
	if (cursor_hoty >= height) cursor_hoty = height - 1;

	return(Main_Window_Create_Cursor(bits.data(), width, height, cursor_hotx, cursor_hoty));
}


/// <summary>
/// Constructs the mouse handler object for the main window, which must already exist.
/// The mouse begins in a non-captured state.
/// </summary>
WWMouseClass::WWMouseClass(void) :
	MouseState(-1),
	IsCaptured(false),
	ConfiningRect(RECT_NONE),
	ReleasedState(0),
	CursorShape(NULL),
	CursorCacheScale(0),
	CurrentShape(NULL),
	CurrentFrame(0),
	CurrentHotX(0),
	CurrentHotY(0),
	CurrentCursor(NULL),
	CursorVisible(true)
{
	Calc_Confining_Rect();
}


/// <summary>
/// Destroys the mouse handler and every cursor it built. Delete it before Main_Window_Destroy.
/// </summary>
WWMouseClass::~WWMouseClass(void)
{
	Main_Window_Set_Cursor(NULL);
	Flush_Cursor_Cache();
	CurrentShape = NULL;
}


/// <summary>
/// Recalculates the desktop rectangle of the main window's client area, which
/// Convert_Coordinate measures positions from. The window's move and resize handlers call
/// this routine to keep it current.
/// </summary>
void WWMouseClass::Calc_Confining_Rect(void)
{
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	Main_Window_Client_Rect(x, y, width, height);

	ConfiningRect = Rect(x, y, width, height);
	DebugString("Calc_Confining_Rect(%d,%d,%d,%d)\n", x, y, width, height);
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
		return(ReleasedState);
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
		Select_Cursor(cursor, shape, hotspot.X, hotspot.Y, Is_Captured());
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
		ReleasedState++;
		Show_Released_Pointer();
	} else {
		MouseState++;
		if (MouseState > 0) MouseState = 0;
		Set_Cursor_Visible(!Is_Hidden());
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
		ReleasedState--;
		Show_Released_Pointer();
	} else {
		MouseState--;
		Set_Cursor_Visible(!Is_Hidden());
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
		if (ReleasedState < 0) {
			ReleasedState = 0;
		}

		/*
		 * There is no exclusive display mode any more, so the pointer is kept inside
		 * the window by hand while the game covers the screen.
		 */
		if (!WindowedMode) {
			Main_Window_Confine_Cursor(true);
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
		Main_Window_Confine_Cursor(false);
		Main_Window_Capture_Mouse(false);
		if (ReleasedState < 0) {
			ReleasedState = 0;
		}
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
	x -= ConfiningRect.X;
	y -= ConfiningRect.Y;
	Client_To_Game(x, y);
}


// Converts a position in the window's client area into one held inside the frame.
void WWMouseClass::Client_To_Game(int & x, int & y) const
{
	Point2D point(x, y);
	Window_Point_To_Game(point);

	VideoScaleInfo const & scale = Video_Get_Scale_Info();
	x = point.X;
	y = point.Y;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x >= scale.GameWidth) x = scale.GameWidth-1;
	if (y >= scale.GameHeight) y = scale.GameHeight-1;
}


// Shows the window's arrow while the released pointer's show count allows it.
void WWMouseClass::Show_Released_Pointer(void) const
{
	Main_Window_Set_Cursor(ReleasedState >= 0 ? Main_Window_System_Cursor(UI_CURSOR_ARROW) : NULL);
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
	x = 0;
	y = 0;
	Main_Window_Cursor_Position(x, y);
	Client_To_Game(x, y);
}


void WWMouseClass::Flush_Cursor_Cache(void)
{
	for (CachedCursor const & entry : CursorCache) {
		Main_Window_Destroy_Cursor(entry.Cursor);
	}

	CursorCache.clear();
	CursorShape = NULL;
	CurrentCursor = NULL;
}


void WWMouseClass::Select_Cursor(ShapeSet const * shape, int frame, int hotx, int hoty, bool apply)
{
	int scale = Cursor_Scale();

	if (shape != CursorShape || scale != CursorCacheScale) {
		Flush_Cursor_Cache();
		CursorShape = shape;
		CursorCacheScale = scale;
		CursorCache.resize(shape->Get_Count() > 0 ? (size_t)shape->Get_Count() : 0);
	}

	CurrentShape = shape;
	CurrentFrame = frame;
	CurrentHotX = hotx;
	CurrentHotY = hoty;

	SDL_Cursor * cursor = NULL;

	if (frame >= 0 && frame < (int)CursorCache.size()) {
		CachedCursor & entry = CursorCache[frame];
		if (entry.Cursor != NULL && (entry.HotX != hotx || entry.HotY != hoty)) {
			Main_Window_Destroy_Cursor(entry.Cursor);
			entry.Cursor = NULL;
		}
		if (entry.Cursor == NULL) {
			entry.Cursor = Build_Cursor(shape, frame, hotx, hoty, scale);
			entry.HotX = hotx;
			entry.HotY = hoty;
		}
		cursor = entry.Cursor;
	}

	CurrentCursor = cursor;

	if (apply) {
		Main_Window_Set_Cursor(CursorVisible ? CurrentCursor : NULL);
	}
}


void WWMouseClass::Set_Cursor_Visible(bool visible)
{
	CursorVisible = visible;

	if (CurrentCursor == NULL) {
		return;
	}

	if (Is_Captured()) {
		Main_Window_Set_Cursor(visible ? CurrentCursor : NULL);
	}
}


/// <summary>
/// Shows the game's pointer over the window.
/// </summary>
/// <returns>False while the mouse is released or before a pointer has been built.</returns>
bool WWMouseClass::Show_Game_Pointer(void)
{
	if (!Is_Captured()) {
		return(false);
	}

	if (CurrentCursor == NULL) {
		return(false);
	}

	Main_Window_Set_Cursor(CursorVisible ? CurrentCursor : NULL);
	return(true);
}


/// <summary>
/// Rebuilds the pointer if the window has been resized enough to want a different size.
/// </summary>
void WWMouseClass::Refresh_Pointer_Scale(void)
{
	if (CurrentShape != NULL && Cursor_Scale() != CursorCacheScale) {
		Select_Cursor(CurrentShape, CurrentFrame, CurrentHotX, CurrentHotY, Is_Captured());
	}
}
