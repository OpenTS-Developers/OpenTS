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

/* $Header: /CounterStrike/DISPLAY.CPP 3     3/09/97 8:04p Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : DISPLAY.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : September 10, 1993                                           *
 *                                                                                             *
 *                  Last Update : October 20, 1996 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   DisplayClass::Compute_Start_Pos -- Computes player's start pos from unit coords.          *
 *   DisplayClass::AI -- Handles the maintenance tasks for the map display.                    *
 *   DisplayClass::All_To_Look -- Direct all objects to look around for the player.            *
 *   DisplayClass::Calculated_Cell -- Fetch a map cell based on specified method.              *
 *   DisplayClass::Cell_Object -- Determines what has been clicked on.                         *
 *   DisplayClass::Cell_Shadow   -- Determine what shadow icon to use for the cell.            *
 *   DisplayClass::Center_Map -- Centers the map about the currently selected objects          *
 *   DisplayClass::Click_Cell_Calc -- Determines cell from screen X & Y.                       *
 *   DisplayClass::Closest_Free_Spot -- Finds the closest cell sub spot that is free.          *
 *   DisplayClass::Coord_To_Pixel -- Determines X and Y pixel coordinates.                     *
 *   DisplayClass::Cursor_Mark -- Set or resets the cursor display flag bits.                  *
 *   DisplayClass::DisplayClass -- Default constructor for display class.                      *
 *   DisplayClass::Draw_It -- Draws the tactical map.                                          *
 *   DisplayClass::Encroach_Shadow -- Causes the shadow to creep back by one cell.             *
 *   DisplayClass::Flag_Cell -- Flag the specified cell to be redrawn.                         *
 *   DisplayClass::Flag_To_Redraw -- Flags the display so that it will be redrawn as soon as poss*
 *   DisplayClass::Get_Occupy_Dimensions -- computes width & height of the given occupy list   *
 *   DisplayClass::Good_Reinforcement_Cell -- Checks cell for renforcement legality.           *
 *   DisplayClass::In_View -- Determines if cell is visible on screen.                         *
 *   DisplayClass::Init_Clear -- Clears the display to a known state.                          *
 *   DisplayClass::Init_IO -- Creates the map's button list                                    *
 *   DisplayClass::Init_Theater -- Theater-specific initialization                             *
 *   DisplayClass::Is_Spot_Free -- Determines if cell sub spot is free of occupation.          *
 *   DisplayClass::Map_Cell -- Mark specified cell as having been mapped.                      *
 *   DisplayClass::Mouse_Left_Held -- Handles the left button held down.                       *
 *   DisplayClass::Mouse_Left_Press -- Handles the left mouse button press.                    *
 *   DisplayClass::Mouse_Left_Release -- Handles the left mouse button release.                *
 *   DisplayClass::Mouse_Left_Up -- Handles the left mouse "cruising" over the map.            *
 *   DisplayClass::Mouse_Right_Press -- Handles the right mouse button press.                  *
 *   DisplayClass::Next_Object -- Searches for next object on display.                         *
 *   DisplayClass::One_Time -- Performs any special one time initializations.                  *
 *   DisplayClass::Passes_Proximity_Check -- Determines if building placement is near friendly sq*
 *   DisplayClass::Pixel_To_Coord -- converts screen coord to COORDINATE                       *
 *   DisplayClass::Prev_Object -- Searches for the previous object on the map.                 *
 *   DisplayClass::Read_INI -- Reads map control data from INI file.                           *
 *   DisplayClass::Redraw_Icons -- Draws all terrain icons necessary.                          *
 *   DisplayClass::Redraw_Shadow -- Draw the shadow overlay.                                   *
 *   DisplayClass::Refresh_Band -- Causes all cells under the rubber band to be redrawn.       *
 *   DisplayClass::Refresh_Cells -- Redraws all cells in list.                                 *
 *   DisplayClass::Remove -- Removes a game object from the rendering system.                  *
 *   DisplayClass::Repair_Mode_Control -- Controls the repair mode.                            *
 *   DisplayClass::Scroll_Map -- Scroll the tactical map in desired direction.                 *
 *   DisplayClass::Select_These -- All selectable objects in region are selected.              *
 *   DisplayClass::Sell_Mode_Control -- Controls the sell mode.                                *
 *   DisplayClass::Set_Cursor_Pos -- Controls the display and animation of the tac cursor.     *
 *   DisplayClass::Set_Cursor_Shape -- Changes the shape of the terrain square cursor.         *
 *   DisplayClass::Set_Tactical_Position -- Sets the tactical view position.                   *
 *   DisplayClass::Set_View_Dimensions -- Sets the tactical display screen coordinates.        *
 *   DisplayClass::Shroud_Cell -- Returns the specified cell into the shrouded condition.      *
 *   DisplayClass::Submit -- Adds a game object to the map rendering system.                   *
 *   DisplayClass::TacticalClass::Action -- Processes input for the tactical map.              *
 *   DisplayClass::Text_Overlap_List -- Creates cell overlap list for specified text string.   *
 *   DisplayClass::Write_INI -- Write the map data to the INI file specified.                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "display.h"

#include "_alpha.h"
#include "_astar.h"
#include "_convert.h"
#include "_keyboar.h"
#include "_map.h"
#include "_mixfile.h"
#include "_palette.h"
#include "_rect.h"
#include "_rtti.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "_tooltip.h"
#include "_zbuffer.h"
#include "airctype.h"
#include "animtype.h"
#include "astar.h"
#include "bsurface.h"
#include "building.h"
#include "builtype.h"
#include "bullettype.h"
#include "cctooltip.h"
#include "cell.h"
#include "conquer.h"
#include "convert.h"
#include "data.h"
#include "dbgprint.h"
#include "dsurface.h"
#include "effects.h"
#include "foot.h"
#include "globals.h"
#include "goptions.h"
#include "incdec.h"
#include "infatype.h"
#include "inline.h"
#include "isotype.h"
#include "keyboard.h"
#include "language\language.h"
#include "logic.h"
#include "mixfile.h"
#include "overtype.h"
#include "palette.h"
#include "queue.h"
#include "rules.h"
#include "savestream.h"
#include "session.h"
#include "smudtype.h"
#include "sidebar.h"
#include "suprtype.h"
#include "super.h"
#include "surface.h"
#include "tactical.h"
#include "tag.h"
#include "tagtype.h"
#include "terrtype.h"
#include "unittype.h"
#include "vein.h"
#include "vector3.h"
#include "vox.h"
#include "waypoint.h"
#include "xpipe.h"
#include "xstraw.h"
#include "zbuffer.h"

#include "color.hh"
#include "super.hh"

#include <algorithm>
#include <cstddef>
#include <vector>

/*
**	These layer control elements are used to group the displayable objects
**	so that proper overlap can be obtained.
*/
LayerClass DisplayClass::Layer[LAYER_COUNT];


void const * DisplayClass::ShadowShapes;
void const * DisplayClass::PlacementShapes;


/*
**	The main button that intercepts user input to the map
*/
DisplayClass::TacticalClass DisplayClass::TacButton;

void Bandbox_Selection_Callback(ObjectClass *object);


/***********************************************************************************************
 * DisplayClass::DisplayClass -- Default constructor for display class.                        *
 *                                                                                             *
 *    This constructor for the display class just initializes some of the display settings.    *
 *    Most settings are initialized with the correct values at the time that the Init function *
 *    is called. There are some cases where default values are wise and this routine fills     *
 *    those particular ones in.                                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/06/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
DisplayClass::DisplayClass(void) :
	ZoneCell(CELL_NONE),
	ZoneOffset(0,0),
	CursorSize(0),
	ProximityCheck(false),
	ShroudCheck(false),
	FollowingObject(false),
	FollowingObjectPtr(NULL),
	PendingObjectPtr(0),
	PendingObject(0),
	PendingHouse(HOUSE_NONE),
	IsRepairMode(false),
	IsSellMode(false),
	IsPowerMode(false),
	IsWaypointMode(false),
	IsTargettingMode(SUPER_NONE),
	DraggedWaypoint(NULL),
	DraggedWaypointCoord(0,0,0),
	IsRubberBand(false),
	IsTentative(false),
	IsShadowPresent(false),
	BandX(0),
	BandY(0),
	NewX(0),
	NewY(0)
{
	ShadowShapes = NULL;
	PlacementShapes = NULL;
}


/***********************************************************************************************
 * DisplayClass::One_Time -- Performs any special one time initializations.                    *
 *                                                                                             *
 *    This routine is called from the game initialization process. It is to perform any one    *
 *    time initializations necessary for the map display system. It allocates the staging      *
 *    buffer needed for the radar map.                                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine must be called ONCE and only once.                                 *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *   05/31/1994 JLB : Handles layer system now.                                                *
 *   06/02/1994 JLB : Takes care of misc display tables and data allocation.                   *
 *=============================================================================================*/
void DisplayClass::One_Time(void)
{
	BASECLASS::One_Time();

	PlacementShapes = MFCD::Retrieve("PLACE.SHP");
	ShadowShapes = MFCD::Retrieve("SHADOW.SHP");

	Rect rect = VisibleRect;
	if (Options.IsSidebarOnRight || Debug_Map) {
		rect.X = 0;
	} else {
		rect.X = SidebarClass::SIDE_WIDTH;
	}
	rect.Y = 16;
	rect.Width = rect.Width - SidebarClass::SIDE_WIDTH;
	rect.Height = rect.Height - 16;
	Set_View_Dimensions(rect);
}


/***********************************************************************************************
 * DisplayClass::Init_Clear -- clears the display to a known state                             *
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
 *   03/17/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
void DisplayClass::Init_Clear(void)
{
	BASECLASS::Init_Clear();

	/*
	**	Clear any object being placed
	*/
	PendingObjectPtr = 0;
	PendingObject = 0;
	PendingHouse = HOUSE_NONE;
	CursorSize = 0;
	IsTargettingMode = SUPER_NONE;
	IsRepairMode = false;
	IsRubberBand = false;
	IsTentative = false;
	IsSellMode = false;
	IsPowerMode = false;

	/*
	**	Empty all the display's layers
	*/
	for (LayerType layer = LAYER_FIRST; layer < LAYER_COUNT; layer++) {
		Layer[layer].Init();
	}
}


/***********************************************************************************************
 * DisplayClass::Init_IO -- clears & re-builds the map's button list                           *
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
 *   03/17/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
void DisplayClass::Init_IO(void)
{
	BASECLASS::Init_IO();
	/*
	**	Re-attach our buttons to the main map button list, only in non-edit mode.
	*/
	if (!Debug_Map) {
		TacButton.Zap();
		Add_A_Button(TacButton);
	}
}


/***********************************************************************************************
 * DisplayClass::Set_View_Dimensions -- Sets the tactical display screen coordinates.          *
 *                                                                                             *
 *    Use this routine to set the tactical map screen coordinates and dimensions. This routine *
 *    is typically used when the screen size or position changes as a result of the sidebar    *
 *    changing position or appearance.                                                         *
 *                                                                                             *
 * INPUT:   x,y   -- The X and Y pixel position on the screen for the tactical map upper left  *
 *                   corner.                                                                   *
 *                                                                                             *
 *          width -- The width of the tactical display (in icons). If this parameter is        *
 *                   omitted, then the width will be as wide as the screen will allow.         *
 *                                                                                             *
 *          height-- The height of the tactical display (in icons). If this parameter is       *
 *                   omitted, then the width will be as wide as the screen will allow.         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/06/1994 JLB : Created.                                                                 *
 *   06/27/1995 JLB : Adjusts tactical map position if necessary.                              *
 *=============================================================================================*/
void DisplayClass::Set_View_Dimensions(Rect const & dimensions)
{
	DebugString("Set_View_Dimensions(%d,%d,%d,%d)\n", dimensions.X, dimensions.Y, dimensions.Width, dimensions.Height);

	TacticalRect = dimensions;

	if (TacticalMap != NULL) {
		TacticalMap->Set_View_Dimensions(TacticalRect);

		Hide_Mouse();

		if (DepthBuffer != NULL) {
			DebugString("Deleting ZBuffer\n");
			delete DepthBuffer;
			DepthBuffer = NULL;
		}

		DepthBuffer = new ZBuffer(TacticalRect);
		DepthBuffer->Set_Scroll(ZBUFFER_MAX);
		DebugString("Allocating ZBuffer (%dx%d)\n", TacticalRect.Width, TacticalRect.Height);

		if (AlphaBuffer != NULL) {
			DebugString("Deleting ABuffer\n");
			delete AlphaBuffer;
			AlphaBuffer = NULL;
		}

		AlphaBuffer = new ABuffer(TacticalRect);
		DebugString("Allocating ABuffer (%dx%d)\n", TacticalRect.Width, TacticalRect.Height);

		Show_Mouse();
	}

	Map.Reposition_Sidebar();

	TacButton.X = TacticalRect.X;
	TacButton.Y = TacticalRect.Y;
	TacButton.Width = TacticalRect.Width;
	TacButton.Height = TacticalRect.Height;

	if (ToolTips != NULL) {
		ToolTip tooltip;
		tooltip.ID = 500;
		tooltip.Text = TXT_NONE;
		tooltip.Region = TacticalRect;
		ToolTips->Remove(500);
		ToolTips->Add(&tooltip);
	}

	/*
	**	For multiplayer games, initialize the inter-player message system.
	**	Do this after loading the scenario, so the map's upper-left corner is
	**	properly set.
	*/
	Session.Messages.Init(
		TacticalRect.X, TacticalRect.Y,     // x,y for messages
		6,                                  // max # msgs
		MAX_MESSAGE_LENGTH-14,              // max msg length
		7 * 2/*RESFACTOR*/,                 // font height in pixels
		-1, -1,                             // x,y for edit line (appears above msgs)
		0,                                  /// enable edit overflow
		20,                                 // min,
		MAX_MESSAGE_LENGTH - 14,            // max for trimming overflow
		TacticalRect.Width);                // Width in pixels of buffer

	Session.Messages.Set_Width(TacticalRect.Width);

	DebugString("Set_View_Dimensions(exit)\n");
}


/***********************************************************************************************
 * DisplayClass::Set_Cursor_Shape -- Changes the shape of the terrain square cursor.           *
 *                                                                                             *
 *    This routine is used to set up the terrain cursor according to the size of the object    *
 *    that is to be placed down. The terrain cursor looks like an arbitrary collection of      *
 *    hatched square overlays. Typical use is when placing buildings.                          *
 *                                                                                             *
 * INPUT:   list  -- A pointer to the list that contains offsets to the cells that are to      *
 *                   be marked.                                                                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/03/1994 JLB : Created.                                                                 *
 *   06/26/1995 JLB : Puts placement cursor into static buffer.                                *
 *=============================================================================================*/
void DisplayClass::Set_Cursor_Shape(Cell const * list)
{
	if (CursorSize) {
		Cursor_Mark(ZoneCell+ZoneOffset, false);
	}
	ZoneOffset = CELL_NONE;

	if (list) {
		//int	w,h;
		static Cell _list[120];

		memcpy(_list, list, sizeof(_list));
		CursorSize = _list;
		Cell dim = Get_Occupy_Dimensions (CursorSize);
		dim -= Cell(1,1);
		ZoneOffset = -dim / 2;
		Cursor_Mark(ZoneCell+ZoneOffset, true);
	} else {
		CursorSize = 0;
	}
}


/***********************************************************************************************
 * DisplayClass::Passes_Proximity_Check -- Determines if building placement is near friendly sq*
 *                                                                                             *
 *    This routine is used by the building placement cursor logic to determine whether the     *
 *    at the current cursor position if the building would be adjacent to another friendly     *
 *    building. In cases where this is not true, then the building cannot be placed at all.    *
 *    This determination is returned by the function.                                          *
 *                                                                                             *
 * INPUT:   object   -- The building object that the current placement system is examining.    *
 *                                                                                             *
 *          house    -- The house to base the proximity check upon. Typically this is the      *
 *                      player's house, but in multiplay, the computer needs to check for      *
 *                      proximity as well.                                                     *
 *                                                                                             *
 *          list     -- Pointer to the building's offset list.                                 *
 *                                                                                             *
 *          trycell  -- The cell to base the offset list on.                                   *
 *                                                                                             *
 * OUTPUT:  bool; Can the pending building object be placed at the present cursor location     *
 *                checking only for proximity to friendly buildings?  If this isn't for a      *
 *                building type object, then this routine always returns true.                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/06/1994 JLB : Created.                                                                 *
 *   06/07/1994 JLB : Handles concrete check.                                                  *
 *   10/11/1994 BWG : Added IsProximate check for ore refineries                               *
 *=============================================================================================*/
bool DisplayClass::Passes_Proximity_Check(ObjectTypeClass const * object, HousesType house, Cell const * list, Cell const & trycell) const
{
	if (house != PlayerPtr->HeapID) {
		return(true);
	}

	/*
	**	In editor mode, the proximity check always passes.
	*/
	if (Debug_Map) {
		return(true);
	}

	if (list == NULL || trycell == CELL_NONE) {
		return(true);
	}

	if (object == NULL || object->RTTI != RTTI_BUILDINGTYPE) {
		return(true);
	}

	BuildingTypeClass const * building = (BuildingTypeClass const *)object;

	int height = building->Height();
	int width = building->Width();

	int adj = (building->Adjacent + 1);

	int tryX = trycell.X;
	int tryY = trycell.Y;

	int xmin = tryX - adj;
	int ymin = tryY - adj;

	int xmax = xmin + ((2 * adj) + width);
	int ymax = ymin + ((2 * adj) + height);

	bool retval = false;

	/*
	**	Scan through all cells that the building foundation would cover. If any adjacent
	**	cells to these are of friendly persuasion, then consider the proximity check to
	**	have been a success.
	*/
	for (int x = xmin; x < xmax; x++) {
		for (int y = ymin; y < ymax; y++) {
			Cell newcell(x, y);

			if ((short)x < tryX || (short)x >= tryX + width || (short)y < tryY || (short)y >= tryY + height) {
				CellClass * cellptr = &Map[newcell];

				/*
				**	The special cell ownership flag allows building adjacent
				**	to friendly walls and bibs even though there is no official
				**	building located there.
				*/
				//BG: Modified so only walls can be placed next to walls - buildings can't.
				if (building->IsWall) {

					if (cellptr->Owner == house) {
						retval = true;
						//break;
					}
				}

				BuildingClass * newbase = cellptr->Cell_Building();

				// we've found a building...
				if (newbase != NULL && newbase->House->HeapID == house && ((BuildingClass *)newbase)->Class->IsBase) {
					retval = true;
					//break;
				}
			}
		}
	}

	return(retval);
}


/// <summary>
/// Determines if building placement is clear of the shroud.
/// This routine is used by the building placement cursor logic to determine whether the
/// building would sit upon ground that the player cannot currently see. Buildings that lay
/// their own tile down are allowed to build into the shroud regardless.
/// </summary>
/// <param name="object">The building object that the current placement system is examining.</param>
/// <param name="house">The house to base the shroud check upon.</param>
/// <param name="list">Pointer to the building's offset list.</param>
/// <param name="trycell">The cell to base the offset list on.</param>
/// <returns>bool; Can the pending building object be placed at the present cursor location
/// checking only for the shroud? If this isn't for a building type object, then this
/// routine always returns true.</returns>
bool DisplayClass::Passes_Shroud_Check(ObjectTypeClass const * object, HousesType house, Cell const * list, Cell const & trycell) const
{
	if (house != PlayerPtr->HeapID) {
		return(true);
	}

	/*
	**	In editor mode, the proximity check always passes.
	*/
	if (Debug_Map) {
		return(true);
	}

	if (list == NULL || trycell == CELL_NONE) {
		return(true);
	}

	if (object == NULL || object->RTTI != RTTI_BUILDINGTYPE) {
		return(true);
	}

	BuildingTypeClass const * building = (BuildingTypeClass const *)object;

	int height = building->Height();
	int width = building->Width();

	int tryX = trycell.X;
	int tryY = trycell.Y;

	for (int x = tryX; x < tryX + width; x++) {
		for (int y = tryY; y < tryY + height; y++) {
			Coord coord = Coord(Cell(x, y));
			coord.Z = Map.Get_Height_GL(coord);

			if (Map.Is_Shrouded(coord)) {
				if (building->ToTile == NULL) {
					return(false);
				}
				return(true);
			}
		}
	}

	return(true);
}


/***********************************************************************************************
 * DisplayClass::Set_Cursor_Pos -- Controls the display and animation of the tac cursor.       *
 *                                                                                             *
 *    This routine controls the location, display, and animation of the                        *
 *    tactical map cursor.                                                                     *
 *                                                                                             *
 * INPUT:   pos   -- Position to move the cursor do. If -1 is passed then                      *
 *                   the cursor will just be hidden. If the position                           *
 *                   passed is the same as the last position passed in,                        *
 *                   then animation could occur (based on timers).                             *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/22/1991 JLB : Created.                                                                 *
 *   06/02/1994 JLB : Converted to member function.                                            *
 *   06/08/1994 JLB : If position is -1, then follow mouse.                                    *
 *   02/28/1995 JLB : Forces placement cursor to fit on map.                                   *
 *=============================================================================================*/
Cell DisplayClass::Set_Cursor_Pos(Cell const & xpos)
{
	Cell	prevpos;			// Last position of cursor (for jump-back reasons).
	Cell	pos = xpos;

	/*
	**	Follow the mouse position if no cell number is provided.
	*/
	if (pos == CELL_NONE) {
		Point2D tl = TacticalRect.TopLeft;
		if (TacticalRect.Is_Point_Within(MouseCursor->Get_Mouse_Point())) {
			Point2D mouse = MouseCursor->Get_Mouse_Point();
			Cell click = Map.Click_Cell_Calc(mouse);
			if (click != CELL_NONE) {
				pos = click;
			}
		} else {
			Point2D mouse = MouseCursor->Get_Mouse_Point();
			mouse -= tl;
			pos = TacticalMap->Pixel_To_Cell(mouse);
		}

	}

	if (CursorSize == NULL) {
		prevpos = ZoneCell;
		ZoneCell = pos;
		return(prevpos);
	}

	/*
	**	Adjusts the position so that the placement cursor is never part way off the
	**	tactical map.
	*/
	if (!Debug_Map) {
		pos = TacticalMap->Clamp_Cursor_To_Tactical(pos + ZoneOffset, CursorSize) - ZoneOffset;
	}

	/*
	**	This checks to see if NO animation or drawing is to occur and, if so,
	**	exits.
	*/
	if (pos == ZoneCell) return(pos);

	prevpos = ZoneCell;

	/*
	**	If the cursor is visible, then handle the graphic update.
	**	Otherwise, just update the global position of the cursor.
	*/
	if (CursorSize != NULL) {

		/*
		**	Erase the old cursor (if it exists) AND the cursor is moving.
		*/
		if (pos != ZoneCell && ZoneCell != CELL_NONE) {
			Cursor_Mark(ZoneCell+ZoneOffset, false);
		}

		/*
		**	Render the cursor (could just be animation).
		*/
		if (pos != CELL_NONE) {
			Cursor_Mark(pos+ZoneOffset, true);
		}
	}
	ZoneCell = pos;
	ProximityCheck = Passes_Proximity_Check(PendingObject, PendingHouse, CursorSize, ZoneCell+ZoneOffset);
	ShroudCheck = Passes_Shroud_Check(PendingObject, PendingHouse, CursorSize, ZoneCell+ZoneOffset);

	return(prevpos);
}


/***********************************************************************************************
 * DisplayClass::Get_Occupy_Dimensions -- computes width & height of the given occupy list     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      w      ptr to fill in with height                                                      *
 *      h      ptr to fill in with width                                                       *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/31/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
Cell DisplayClass::Get_Occupy_Dimensions(Cell const * list) const
{
	int min_x = MAP_CELL_W;
	int max_x = -MAP_CELL_W;
	int min_y = MAP_CELL_H;
	int max_y = -MAP_CELL_H;
	int x,y;

	Cell cell(0, 0);

	if (list != NULL) {
		/*
		**	Loop through all cell offsets, accumulating max & min x- & y-coords
		*/
		while (*list != REFRESH_EOL) {
			/*
			**	Compute x & y coords of the current cell offset.  We can't use Cell_X()
			** & Cell_Y(), because they use shifts to compute the values, and if the
			**	offset is negative we'll get a bogus coordinate!
			*/
			x = list->X;
			y = list->Y;

			max_x = std::max(max_x, x);
			min_x = std::min(min_x, x);
			max_y = std::max(max_y, y);
			min_y = std::min(min_y, y);

			list++;
		}

		cell.X = std::max(1, max_x - min_x + 1);
		cell.Y = std::max(1, max_y - min_y + 1);
	}
	return(cell);
}


/***********************************************************************************************
 * DisplayClass::Cursor_Mark -- Set or resets the cursor display flag bits.                    *
 *                                                                                             *
 *    This routine will clear or set the cursor display bits on the map.                       *
 *    If the bit is set, then the cursor will be rendered on that map                          *
 *    icon.                                                                                    *
 *                                                                                             *
 * INPUT:   pos   -- Position of the upper left corner of the cursor.                          *
 *                                                                                             *
 *          on    -- Should the bit be turned on?                                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Be sure that every call to set the bits is matched by a                         *
 *             corresponding call to clear the bits.                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/04/1991 JLB : Created.                                                                 *
 *   06/02/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
void DisplayClass::Cursor_Mark(Cell const &pos, bool on)
{
	Cell const * ptr;
	CellClass * cellptr;

	if (pos == CELL_NONE) return;

	/*
	**	For every cell in the CursorSize list, invoke its Redraw_Objects and
	**	toggle its IsCursorHere flag
	*/
	ptr = CursorSize;
	while (*ptr != REFRESH_EOL) {
		Cell cell = Cell(pos + *ptr++);
		if (In_Radar(cell)) {
			cellptr = &(*this)[cell];
			if (on) {
				cellptr->IsCursorHere = true;
			} else {
				cellptr->IsCursorHere = false;
			}
		}
	}
}


/***********************************************************************************************
 * DisplayClass::AI -- Handles the maintenance tasks for the map display.                      *
 *                                                                                             *
 *    This routine is called once per game display frame (15 times per second). It handles     *
 *    the mouse shape tracking and map scrolling as necessary.                                 *
 *                                                                                             *
 * INPUT:   input -- The next key just fetched from the input queue.                           *
 *                                                                                             *
 *          x,y   -- Mouse coordinates.                                                        *
 *                                                                                             *
 * OUTPUT:  Modifies the input code if necessary. When the input code is consumed, it gets     *
 *          set to 0.                                                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/01/1994 JLB : Created.                                                                 *
 *   06/02/1994 JLB : Filters mouse click input.                                               *
 *   06/07/1994 JLB : Fixed so template click will behave right.                               *
 *   10/14/1994 JLB : Changing cursor shape over target.                                       *
 *   12/31/1994 JLB : Takes mouse coordinates as parameters.                                   *
 *   06/27/1995 JLB : Breaks out of rubber band mode if mouse leaves map.                      *
 *=============================================================================================*/
void DisplayClass::AI(KeyNumType & input, Point2D const & xy)
{
	BASECLASS::AI(input, xy);
}


/***********************************************************************************************
 * DisplayClass::Submit -- Adds a game object to the map rendering system.                     *
 *                                                                                             *
 *    This routine is used to add an arbitrary (but tangible) game object to the map. It will  *
 *    be rendered (made visible) once it is submitted to this function. This function builds   *
 *    the list of game objects that get rendered each frame as necessary. It is possible to    *
 *    submit the game object to different rendering layers. All objects in a layer get drawn   *
 *    at the same time. Using this layer method it becomes possible to have objects "below"    *
 *    other objects.                                                                           *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object to add.                                          *
 *                                                                                             *
 *          layer    -- The layer to add the object to.                                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *   05/31/1994 JLB : Improved layer system.                                                   *
 *   05/31/1994 JLB : Sorts object position if this is for the ground layer.                   *
 *=============================================================================================*/
void DisplayClass::Submit(ObjectClass const * object)
{
	if (object) {
		if (object->Layer != LAYER_NONE) {
			Remove(object);
		}
		LayerType layer = object->In_Which_Layer();
		if (layer != LAYER_NONE) {
			if (Layer[layer].Submit(object, (layer == LAYER_GROUND))) {
				((ObjectClass *)object)->Layer = layer;
			}
		}
	}
}


/***********************************************************************************************
 * DisplayClass::Remove -- Removes a game object from the rendering system.                    *
 *                                                                                             *
 *    Every object that is to disappear from the map must be removed from the rendering        *
 *    system.                                                                                  *
 *                                                                                             *
 * INPUT:   object   -- The object to remove.                                                  *
 *                                                                                             *
 *          layer    -- The layer to remove it from.                                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *   05/31/1994 JLB : Improved layer system.                                                   *
 *=============================================================================================*/
void DisplayClass::Remove(ObjectClass const * object)
{
	assert(object != 0);
	assert(object->IsActive);

	if (object) {
		if (object->Layer != LAYER_NONE) {
			if (Layer[object->Layer].Delete((ObjectClass *)object)) {
				((ObjectClass *)object)->Layer = LAYER_NONE;
			}

			if (object->Layer != LAYER_NONE) {
				for (int l = LAYER_FIRST; l < LAYER_COUNT; l++) {
					while (Layer[l].Delete((ObjectClass *)object)) { } // delete all instances
				}
				((ObjectClass *)object)->Layer = LAYER_NONE;
			}
		}
	}
}


/***********************************************************************************************
 * DisplayClass::Scroll_Map -- Scroll the tactical map in desired direction.                   *
 *                                                                                             *
 *    This routine is used to scroll the tactical map view in the desired                      *
 *    direction. It can also be used to determine if scrolling would be                        *
 *    legal without actually performing any scrolling action.                                  *
 *                                                                                             *
 * INPUT:   facing   -- The direction to scroll the tactical map.                              *
 *                                                                                             *
 *          distance -- The distance in leptons to scroll the map.                             *
 *                                                                                             *
 *          really   -- Should the map actually be scrolled?  If false,                        *
 *                      then only the legality of a scroll is checked.                         *
 *                                                                                             *
 * OUTPUT:  bool; Would scrolling in the desired direction be possible?                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/1992 JLB : Created.                                                                 *
 *   05/20/1994 JLB : Converted to member function.                                            *
 *   08/09/1995 JLB : Added distance parameter.                                                *
 *   08/10/1995 JLB : Any direction scrolling.                                                 *
 *=============================================================================================*/
bool DisplayClass::Scroll_Map(FacingType facing, int & distance, bool really)
{
	/*
	**	If the distance is invalid then no further checking is required. Bail
	**	with a no-can-do flag.
	*/
	if (distance == 0 && !really) return(false);

	if (!really) {
		return(TacticalMap->Scroll_Dir(facing) != FACING_NONE);
	}

	TacticalMap->Scroll_Map(facing, distance);
	return(true);
}


/***********************************************************************************************
 * DisplayClass::Map_Cell -- Mark specified cell as having been mapped.                        *
 *                                                                                             *
 *    This routine maps the specified cell. The cell must not already                          *
 *    have been mapped and the mapping player must be the human.                               *
 *    This routine will update any adjacent cell map icon as appropriate.                      *
 *                                                                                             *
 * INPUT:   cell  -- The cell to be mapped.                                                    *
 *                                                                                             *
 *          house -- The player that is doing the mapping.                                     *
 *                                                                                             *
 * OUTPUT:  bool; Was action taken to map this cell?                                           *
 *                                                                                             *
 * WARNINGS:   none.                                                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/05/1992 JLB : Created.                                                                 *
 *   04/30/1994 JLB : Converted to member function.                                            *
 *   05/24/1994 JLB : Takes pointer to HouseClass.                                             *
 *   02/20/1996 JLB : Allied units reveal the map for the player.                              *
 *=============================================================================================*/
bool DisplayClass::Map_Cell(Cell const & cell, HouseClass * house)
{
	CellClass * cellptr = &(*this)[cell];

	bool wasfogged = cellptr->IsFogMapped == false;
	bool changed = !cellptr->IsFogMapped || !cellptr->IsMapped;
	bool newlymapped = changed;

	cellptr->IsToFog = false;

	/*
	**	Mark the cell as being mapped. This must be done first because
	**	if the IsVisible flag must be set, then it might affect the
	**	adjacent cell processing.
	*/
	cellptr->IsFogMapped = true;
	cellptr->IsMapped = true;

	signed char sframe = TacticalMap->Cell_Shadow(cell, false);
	if (sframe != cellptr->ShadowFrame) {
		changed = true;
		cellptr->ShadowFrame = sframe;
	}
	if (cellptr->ShadowFrame == -1) {
		cellptr->IsVisible = true;
	}

	signed char fframe = TacticalMap->Cell_Shadow(cell, true);
	if (fframe != cellptr->FogFrame) {
		changed = true;
		cellptr->FogFrame = fframe;
	}
	if (cellptr->FogFrame == -1) {
		cellptr->IsFogVisible = true;
	}

	if (changed) {
		TacticalMap->Flag_Cell(*cellptr);
	}

	/*
	**	Check out all adjacent cells to see if they need
	**	to be mapped as well. This is necessary because of the
	**	"unique" method of showing shadowed cells. Many combinations
	**	are not allowed, and to fix this, just map the cells until
	**	all is ok.
	*/
	for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
		int	shadow;
		int fog;

		Cell c = Adjacent_Cell(cell, dir);
		CellClass * cptr = &(*this)[c];

		if (c != cell && !cptr->IsVisible) {
			shadow = TacticalMap->Cell_Shadow(c, false);

			if (shadow == -1) {
				if (!cptr->IsMapped) {
					Map_Cell(c, house);
				} else {
					cptr->IsVisible = true;
					TacticalMap->Flag_Cell(*cptr);
					for (FacingType dir2 = FACING_FIRST; dir2 < FACING_COUNT; dir2++) {
						Cell cc = Adjacent_Cell(c, dir2);
						CellClass * scptr = &(*this)[cc];
						signed char sframe = TacticalMap->Cell_Shadow(cc, false);
						if (sframe != scptr->ShadowFrame) {
							scptr->ShadowFrame = sframe;
							TacticalMap->Flag_Cell(*scptr);
						}
					}
				}
			} else {
				if (shadow != -2 && !cptr->IsMapped) {
					Map_Cell(c, house);
				} else {
					if (shadow >= 0 && shadow != cptr->ShadowFrame) {
						cptr->ShadowFrame = shadow;
						TacticalMap->Flag_Cell(*cptr);
					}
				}
			}
		}

		if (c != cell && !cptr->IsFogVisible) {
			fog = TacticalMap->Cell_Shadow(c, true);

			if (fog == -1) {
				if (!cptr->IsFogMapped) {
					Map_Cell(c, house);
				} else {
					cptr->IsFogVisible = true;
					TacticalMap->Flag_Cell(*cptr);
					for (FacingType dir2 = FACING_FIRST; dir2 < FACING_COUNT; dir2++) {
						Cell cc = Adjacent_Cell(c, dir2);
						CellClass * fcptr = &(*this)[cc];
						signed char fframe = TacticalMap->Cell_Shadow(cc, true);
						if (fframe != fcptr->FogFrame) {
							fcptr->FogFrame = fframe;
							TacticalMap->Flag_Cell(*fcptr);
						}
					}
				}
			} else {
				if (fog != -2 && !cptr->IsFogMapped) {
					Map_Cell(c, house);
				} else {
					if (fog >= 0 && fog != cptr->FogFrame) {
						cptr->FogFrame = fog;
						TacticalMap->Flag_Cell(*cptr);
					}
				}
			}
		}
	}

	if (changed) {
		Map.Reveal_Nearby_Technos(cellptr, house, newlymapped);
	}

	if (cellptr->IsFogMapped && wasfogged && Scen->Special.IsFogOfWar) {
		cellptr->Unfog_Cell();
	}

	return(changed);
}


/// <summary>
/// Marks the specified cell as no longer fogged.
/// This is the fog of war counterpart to Shadow_Map_Cell. The cell is unfogged and the fog
/// artwork of the adjacent cells is brought up to date, unfogging any neighbor whose fog
/// piece would otherwise have no legal artwork.
/// </summary>
/// <param name="cell">The cell that is to be unfogged.</param>
/// <param name="house">The player that is doing the unfogging.</param>
/// <returns>bool; Was action taken to unfog this cell?</returns>
bool DisplayClass::Fog_Map_Cell(Cell const & cell, HouseClass * house)
{
	CellClass * cellptr = &(*this)[cell];

	bool wasfogged = cellptr->IsFogMapped == false;
	bool changed = !cellptr->IsFogMapped;
	bool newlymapped = changed;

	cellptr->IsToFog = false;

	/*
	** Mark the cell as being mapped. This must be done first because
	**	if the IsVisible flag must be set, then it might affect the
	**	adjacent cell processing.
	*/
	cellptr->IsFogMapped = true;

	signed char fframe = TacticalMap->Cell_Shadow(cell, true);
	if (fframe != cellptr->FogFrame) {
		changed = true;
		cellptr->FogFrame = fframe;
	}
	if (cellptr->FogFrame == -1) {
		cellptr->IsFogVisible = true;
	}

	if (changed) {
		TacticalMap->Flag_Cell(*cellptr);
	}

	/*
	**	Check out all adjacent cells to see if they need
	**	to be mapped as well. This is necessary because of the
	**	"unique" method of showing shadowed cells. Many combinations
	**	are not allowed, and to fix this, just map the cells until
	**	all is ok.
	*/
	for (int dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
		int fog;

		Cell c = Adjacent_Cell(cell, (FacingType)dir);
		CellClass * cptr = &(*this)[c];

		if (c != cell && !cptr->IsFogVisible) {
			fog = TacticalMap->Cell_Shadow(c, true);

			if (fog == -1) {
				if (!cptr->IsFogMapped) {
					Fog_Map_Cell(c, house);
				} else {
					cptr->IsFogVisible = true;
					TacticalMap->Flag_Cell(*cptr);
					for (int dir2 = FACING_FIRST; dir2 < FACING_COUNT; dir2++) {
						Cell cc = Adjacent_Cell(c, (FacingType)dir2);
						CellClass * fcptr = &(*this)[cc];
						signed char fframe = TacticalMap->Cell_Shadow(cc, true);
						if (fframe != fcptr->FogFrame) {
							fcptr->FogFrame = fframe;
							TacticalMap->Flag_Cell(*fcptr);
						}
					}
				}
			} else {
				if (fog != -2 && !cptr->IsFogMapped) {
					Fog_Map_Cell(c, house);
				} else {
					if (fog >= 0 && fog != cptr->FogFrame) {
						cptr->FogFrame = fog;
						TacticalMap->Flag_Cell(*cptr);
					}
				}
			}
		}
	}

	if (changed) {
		Map.Reveal_Nearby_Technos(cellptr, house, newlymapped);
	}

	if (cellptr->IsFogMapped && wasfogged && Scen->Special.IsFogOfWar) {
		cellptr->Unfog_Cell();
	}

	return(changed);
}


/// <summary>
/// Marks the specified cell as no longer shrouded.
/// This routine reveals the cell to the player and brings the shadow artwork of the
/// adjacent cells up to date. Any neighbor whose shadow piece would have no legal artwork
/// is revealed as well, so that the edge of the shroud always draws correctly.
/// </summary>
/// <param name="cell">The cell that is to be revealed.</param>
/// <param name="house">The player that is doing the revealing.</param>
/// <returns>bool; Was action taken to reveal this cell?</returns>
bool DisplayClass::Shadow_Map_Cell(Cell const & cell, HouseClass * house)
{
	CellClass * cellptr = &(*this)[cell];

	/// Unused here -- unlike Map_Cell, this routine never unfogs the cell.
	bool wasfogged = cellptr->IsFogMapped == false;

	bool changed = !cellptr->IsMapped;
	bool newlymapped = changed;

	/*
	**	Mark the cell as being mapped. This must be done first because
	**	if the IsVisible flag must be set, then it might affect the
	**	adjacent cell processing.
	*/
	cellptr->IsMapped = true;

	signed char sframe = TacticalMap->Cell_Shadow(cell, false);
	if (sframe != cellptr->ShadowFrame) {
		changed = true;
		cellptr->ShadowFrame = sframe;
	}
	if (cellptr->ShadowFrame == -1) {
		cellptr->IsVisible = true;
	}

	if (changed) {
		TacticalMap->Flag_Cell(*cellptr);
	}

	/*
	**	Check out all adjacent cells to see if they need
	**	to be mapped as well. This is necessary because of the
	**	"unique" method of showing shadowed cells. Many combinations
	**	are not allowed, and to fix this, just map the cells until
	**	all is ok.
	*/
	for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
		int	shadow;

		Cell c = Adjacent_Cell(cell, dir);
		CellClass * cptr = &(*this)[c];

		if (c != cell && !cptr->IsVisible) {
			shadow = TacticalMap->Cell_Shadow(c, false);

			if (shadow == -1) {
				if (!cptr->IsMapped) {
					Shadow_Map_Cell(c, house);
				} else {
					cptr->IsVisible = true;
					TacticalMap->Flag_Cell(*cptr);
					for (FacingType dir2 = FACING_FIRST; dir2 < FACING_COUNT; dir2++) {
						Cell cc = Adjacent_Cell(c, dir2);
						CellClass * scptr = &(*this)[cc];
						signed char sframe = TacticalMap->Cell_Shadow(cc, false);
						if (sframe != scptr->ShadowFrame) {
							scptr->ShadowFrame = sframe;
							TacticalMap->Flag_Cell(*scptr);
						}
					}
				}
			} else {
				if (shadow != -2 && !cptr->IsMapped) {
					Shadow_Map_Cell(c, house);
				} else {
					if (shadow >= 0 && shadow != cptr->ShadowFrame) {
						cptr->ShadowFrame = shadow;
						TacticalMap->Flag_Cell(*cptr);
					}
				}
			}
		}
	}

	if (changed) {
		Map.Reveal_Nearby_Technos(cellptr, house, newlymapped);
	}

	return(changed);
}


/***********************************************************************************************
 * DisplayClass::Cell_Object -- Determines what has been clicked on.                           *
 *                                                                                             *
 *    This routine is used to determine what the player has clicked on.                        *
 *    It is passed the cell that the click was on and it then examines                         *
 *    the cell and returns with a pointer to the object that is there.                         *
 *                                                                                             *
 * INPUT:   cell  -- The cell that has been clicked upon.                                      *
 *                                                                                             *
 *          x,y   -- Optional offsets from the upper left corner of the cell to be used in     *
 *                   determining exactly which object in the cell is desired.                  *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the object that is "clickable" in                        *
 *          the specified cell.                                                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/14/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectClass * DisplayClass::Cell_Object(Cell const & cell, Point2D const & point) const
{
	return(*this)[cell].Cell_Object(point);
}


/***********************************************************************************************
 * DisplayClass::Next_Object -- Searches for next object on display.                           *
 *                                                                                             *
 *    This utility routine is used to find the "next" object from the object specified. This   *
 *    is typically used when <TAB> is pressed and the current object shifts.                   *
 *                                                                                             *
 * INPUT:   object   -- The current object to base the "next" calculation off of.              *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the next object. If there is no objects available,       *
 *          then NULL is returned.                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/20/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectClass * DisplayClass::Next_Object(ObjectClass * object) const
{
	ObjectClass * firstobj = NULL;
	bool foundmatch = false;

	if (object == NULL) {
		foundmatch = true;
	}

	static LayerType _layers[] = {
		LAYER_GROUND,
		LAYER_AIR,
		LAYER_TOP
	};

	for (int lindex = 0; lindex < ARRAY_SIZE(_layers); lindex++) {
		for (int uindex = 0; uindex < Layer[_layers[lindex]].Count(); uindex++) {
			ObjectClass * obj = Layer[_layers[lindex]][uindex];

			/*
			**	Verify that the object can be selected by and is owned by the player.
			*/
			if (obj != NULL && obj->Is_Players_Army()) {
				if (firstobj == NULL) firstobj = obj;
				if (foundmatch) return(obj);
				if (object == obj) foundmatch = true;
			}
		}
	}
	return(firstobj);
}


/***********************************************************************************************
 * DisplayClass::Prev_Object -- Searches for the previous object on the map.                   *
 *                                                                                             *
 *    This routine will search for the previous object. Previous is defined as the one listed  *
 *    before the specified object in the ground layer. If there is no specified object, then   *
 *    the last object in the ground layer is returned.                                         *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object that "previous" is to be defined from.           *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the object previous to the specified one.                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectClass * DisplayClass::Prev_Object(ObjectClass * object)  const
{
	ObjectClass * firstobj = NULL;
	bool foundmatch = false;

	if (object == NULL) {
		foundmatch = true;
	}

	static LayerType _layers[] = {
		LAYER_TOP,
		LAYER_AIR,
		LAYER_GROUND
	};

	for (int lindex = 0; lindex < ARRAY_SIZE(_layers); lindex++) {
		for (int uindex = Layer[_layers[lindex]].Count()-1; uindex >= 0; uindex--) {
			ObjectClass * obj = Layer[_layers[lindex]][uindex];

			/*
			**	Verify that the object can be selected by and is owned by the player.
			*/
			if (obj != NULL && obj->Is_Players_Army()) {
				if (firstobj == NULL) firstobj = obj;
				if (foundmatch) return(obj);
				if (object == obj) foundmatch = true;
			}
		}
	}

	return(firstobj);
}


/***********************************************************************************************
 * DisplayClass::Calculated_Cell -- Fetch a map cell based on specified method.                *
 *                                                                                             *
 *    Find a cell meeting the specified requirements. This function is                         *
 *    used for scenario reinforcements.                                                        *
 *                                                                                             *
 * INPUT:   dir   -- Method of picking a map cell.                                             *
 *                                                                                             *
 *          waypoint -- Closest waypoint to use for finding appropriate map edge.              *
 *                                                                                             *
 *          cell  -- Cell to find closest edge to if waypoint not specified.                   *
 *                                                                                             *
 *          loco  -- The locomotion of the reinforcements that are trying to enter.            *
 *                                                                                             *
 *          zonecheck   -- Is zone checking required?                                          *
 *                                                                                             *
 *          mzone    -- The movement zone type to check against (only if zone checking).       *
 *                                                                                             *
 * OUTPUT:  Returns with the calculated cell. If 0, then this indicates                        *
 *          that no legal cell was found.                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/1992 JLB : Created.                                                                 *
 *   04/11/1994 JLB : Revamped.                                                                *
 *   05/18/1994 JLB : Converted to member function.                                            *
 *   12/18/1995 JLB : Handles edge preference scan.                                            *
 *   06/24/1996 JLB : Removed Dune II legacy code.                                             *
 *   06/25/1996 JLB : Rewrote and greatly simplified.                                          *
 *   10/05/1996 JLB : Checks for zone and crushable status.                                    *
 *=============================================================================================*/
Cell DisplayClass::Calculated_Cell(SourceType dir, Cell const & waypoint, Cell const & cell, SpeedType loco, bool zonecheck, MZoneType mzone) const
{
	bool vert = false;
	bool horz = false;
	int x = 0;
	int y = 0;
	Cell punt;				// If all else fails, return this cell location.
	int zone = -1;			// Tentative zone for legality checking.
	int index;

	/*
	**	Waypoint edge detection for ground based reinforcements that have a waypoint origin are
	**	determined by finding the closest map edge to the waypoint. Reinforcement location
	**	scanning starts from that position.
	*/
	Cell trycell(0,0);
	if (waypoint != CELL_NONE) {
		trycell = waypoint;
	}
	if (trycell == CELL_NONE) {
		trycell = cell;
	}

	/*
	**	If zone checking is requested, then find the correct zone to use.
	*/
	if (zonecheck && trycell != CELL_NONE) {
		zone = Map.Get_Cell_Zone(trycell, mzone, true);
	}

	/*
	**	If the cell or waypoint specified as been detected as legal, then set up the map edge
	**	scanning values accordingly.
	*/
	Point2D trypoint = Cell_To_LocalRect_Point(Point2D(trycell.X, trycell.Y));
	if (trycell != CELL_NONE) {
		x = trypoint.X - LocalRect.X;
		x = std::min(x, (-trypoint.X + (LocalRect.X+LocalRect.Width)));
		x *= 2;

		y = trypoint.Y - 2 * LocalRect.Y;
		y = std::min(y, (-trypoint.Y + 2 * (LocalRect.Y+LocalRect.Height)));

		if (x < y) {
			vert = true;
			horz = false;
			if ((trypoint.X-LocalRect.X) < LocalRect.Width/2) {
				x = -1;
				dir = SOURCE_WEST;
			} else {
				x = LocalRect.Width;
				dir = SOURCE_EAST;
			}
			y = trypoint.Y - LocalRect.Y;

		} else {

			vert = false;
			horz = true;
			if ((trypoint.Y - 2 * LocalRect.Y) < (2 * LocalRect.Height)/2) {
				y = -1;
				dir = SOURCE_NORTH;
			} else {
				y = 2 * LocalRect.Height;
				dir = SOURCE_SOUTH;
			}
			x = trypoint.X - LocalRect.X;
		}

	} else {

		/*
		**	If no map edge can be inferred from the waypoint, then go with the
		**	map edge specified by the edge parameter.
		*/
		if (!vert && !horz) {
			switch (dir) {
				default:
				case SOURCE_NORTH:
					horz = true;
					y = 0;
					x = Random_Pick(1, LocalRect.Width) - 1;
					break;

				case SOURCE_SOUTH:
					horz = true;
					y = 2 * LocalRect.Height + 2;
					x = Random_Pick(1, LocalRect.Width) - 1;
					break;

				case SOURCE_EAST:
					vert = true;
					x = LocalRect.Width;
					y = Random_Pick(1, 2 * LocalRect.Height) - 1;
					break;

				case SOURCE_WEST:
					vert = true;
					x = 0;
					y = Random_Pick(0, 2 * LocalRect.Height) - 1;
					break;

			}
		}
	}

	/*
	**	Determine the default reinforcement cell if all else fails.
	*/
	punt = Cell(LocalRect_To_Cell_Point(Point2D(1, LocalRect.Width/2)));

	/*
	**	Scan through the vertical and horizontal edges of the map looking for
	**	a relatively clear cell for object placement. The cell scanned is
	**	from the edge position specified by the X and Y variables.
	*/
	if (vert) {
		int modifier = (dir != SOURCE_EAST) ? 1 : -1;

		for (index = 0; index < LocalRect.Height * 2; index++) {
			int yy = (y + index) % (LocalRect.Height * 2);

			Cell outcell(LocalRect_To_Cell_Point(Point2D(x, yy)));
			Cell incell(LocalRect_To_Cell_Point(Point2D(x + modifier, yy)));

			if (Good_Reinforcement_Cell(outcell, incell, loco, zone, mzone)) {
				return(outcell);
			}
		}
		return(punt);
	}

	if (horz) {
		int modifier = (dir == SOURCE_NORTH) ? 1 : -1;

		if (dir == SOURCE_SOUTH) {
			DynamicVectorClass<Cell> cells;

			for (x = 0; x < LocalRect.Width; x++) {
				for (y = 0; y < 15; y++) {
					Cell newcell(LocalRect_To_Cell_Point(Point2D(x, y + 2 * LocalRect.Height)));
					if (!In_Local_Radar(newcell, true) && Good_Reinforcement_Cell(newcell, trycell, loco, zone, mzone)) {
						cells.Add(newcell);
						break;
					}
				}
			}
			index = cells.Count();

			if (trycell == CELL_NONE) {
				punt = cells[Random_Pick(0, index)];
			} else {
				int dist = -1;
				Cell best = CELL_NONE;
				for (int i = 0; i < cells.Count(); i++) {
					int d = trycell.Distance_To(cells[i]);
					if (dist == -1 || d < dist) {
						best = cells[i];
						dist = d;
					}
				}
				punt = best;
			}

			return(punt);
		} else {
			for (index = 0; index < LocalRect.Width; index++) {
				int xx = (x + index) % LocalRect.Width;

				Cell outcell(LocalRect_To_Cell_Point(Point2D(xx, y)));
				Cell incell(LocalRect_To_Cell_Point(Point2D(xx, y + modifier)));

				if (Good_Reinforcement_Cell(outcell, incell, loco, zone, mzone)) {
					return(outcell);
				}
			}
		}
	}

	/*
	**	If there was no success in finding a suitable reinforcement edge cell, then return
	**	with the default 'punt' cell location.
	*/
	return(punt);
}


/***********************************************************************************************
 * DisplayClass::Good_Reinforcement_Cell -- Checks cell for renforcement legality.             *
 *                                                                                             *
 *    This routine will check the secified cell (given the specified conditions) and determine *
 *    if that is a good cell for reinforcement purposes. It checks for passability of the cell *
 *    as well as zone and whether blocking walls can be crushed.                               *
 *                                                                                             *
 * INPUT:   outcell  -- The cell that is just outside the edge of the map.                     *
 *                                                                                             *
 *          incell   -- The cell that is just inside the edge of the map.                      *
 *                                                                                             *
 *          loco     -- The locomotion type of the reinforcement.                              *
 *                                                                                             *
 *          zone     -- The zone that the eventual movement destination lies. A reinforcement  *
 *                      edge must fall within the same zone.                                   *
 *                                                                                             *
 *          mzone    -- The zone check type to check against (if zone checking required)       *
 *                                                                                             *
 * OUTPUT:  bool; Is the specified cell good for reinforcement purposes?                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool DisplayClass::Good_Reinforcement_Cell(Cell const & outcell, Cell const & incell, SpeedType loco, int zone, MZoneType mzone) const
{
	/*
	**	If the map edge location is not clear for object placement, then this is not
	**	a good cell for reinforcement purposes.
	*/
	if (!(*this)[outcell].Is_Clear_To_Move(loco, true, true)) {
		return(false);
	}

	/*
	**	If it looks like the on-map cell cannot be driven on to, then return with
	**	the failure code.
	*/
	if (!(*this)[incell].Is_Clear_To_Move(loco, true, true, zone, mzone)) {
		return(false);
	}

	/*
	**	If the reinforcement cell is already occupied, then return a failure code.
	*/
	if ((*this)[outcell].Cell_Techno() != NULL) {
		return(false);
	}
	if ((*this)[incell].Cell_Techno() != NULL) return(false);

	/*
	**	All tests have passed, return with success code.
	*/
//Mono_Printf("<%04X>\n", incell);Keyboard->Get();
	return(true);
}


/***********************************************************************************************
 * DisplayClass::TacticalClass::Action -- Processes input for the tactical map.                *
 *                                                                                             *
 *    This routine handles the input directed at the tactical map. Since input, in this        *
 *    regard, includes even the presence of the mouse over the tactical map, this routine      *
 *    is called nearly every game frame. It handles adjusting the mouse shape as well as       *
 *    giving orders to units.                                                                  *
 *                                                                                             *
 * INPUT:   flags -- The gadget event flags that triggered the call to this function.          *
 *                                                                                             *
 *          key   -- A reference to the keyboard event (if any).                               *
 *                                                                                             *
 * OUTPUT:  bool; Should processing be aborted on any succeeding buttons in the chain?         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/17/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int DisplayClass::TacticalClass::Action(unsigned flags, KeyNumType & key)
{
	int		x,y;					// Sub cell pixel coordinates.
	bool		fog, shadow;
	ObjectClass * object = 0;
	ActionType action = ACTION_NONE;		// Action possible with currently selected object.
	Point2D pixel(0, 0);

	/*
	**	Set some working variables that depend on the mouse position. For the press
	**	or release event, special mouse queuing storage variables are used. Other
	**	events must use the current mouse position globals.
	*/
	if (flags & (LEFTPRESS|LEFTRELEASE|RIGHTPRESS|RIGHTRELEASE)) {
		x = Keyboard->MouseQX;
		y = Keyboard->MouseQY;
		pixel = Point2D(x, y);
	} else {
		x = Get_Mouse_X();
		y = Get_Mouse_Y();
		pixel = Get_Mouse_Point();
	}

	Coord coord;
	TacticalMap->Pixel_To_Coord(pixel);
	Cell cell;

	pixel -= TacticalRect.Top_Left();

	Map.Resolve_Point(pixel, cell, coord, object, fog, shadow);

	/*
	**	Cause any displayed cursor to move along with the mouse cursor.
	*/
	if (cell != Map.ZoneCell) {
		Map.Set_Cursor_Pos(cell);
	}

	return(GadgetClass::Action(0, key));
}


/***********************************************************************************************
 * DisplayClass::Mouse_Right_Press -- Handles the right mouse button press.                    *
 *                                                                                             *
 *    This routine is called when the right mouse button is pressed. This action is supposed   *
 *    to cancel whatever mode or process is active. If there is nothing to cancel, then it     *
 *    will default to unselecting any units that might be currently selected.                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void DisplayClass::Mouse_Right_Press(Point2D const & point)
{
	//nothing
}


/// <summary>
/// Handles the right mouse button being released.
/// This routine cancels whatever mode or process is currently active -- pending placement
/// first, then repair, sell, power, targetting and waypoint mode. If there is nothing at
/// all to cancel, then it falls back to unselecting whatever the player had selected.
/// </summary>
void DisplayClass::Mouse_Right_Release(Point2D const & point)
{
	if (PendingObjectPtr && PendingObjectPtr->Is_Techno()) {
		//PendingObjectPtr->Transmit_Message(RADIO_OVER_OUT);
		PendingObjectPtr = NULL;
		PendingObject = NULL;
		PendingHouse = HOUSE_NONE;
		Set_Cursor_Shape(NULL);
	} else {
		if (IsRepairMode) {
			Repair_Mode_Control(0);
		} else {
			if (IsSellMode) {
				Sell_Mode_Control(0);
			} else {
				if (IsPowerMode) {
					Power_Mode_Control(0);
				} else {
					if (IsTargettingMode != SUPER_NONE) {
						IsTargettingMode = SUPER_NONE;
						PlayerPtr->TargetingSW = NULL;
					} else {
						if (IsWaypointMode) {
							Waypoint_Mode_Control(0);
						} else {
							Unselect_All();
						}
					}
				}
			}
		}
	}

	// If it breaks... call 228.
	Set_Default_Mouse(MOUSE_NORMAL, Map.IsSmall);
}


/***********************************************************************************************
 * DisplayClass::Mouse_Left_Up -- Handles the left mouse "cruising" over the map.              *
 *                                                                                             *
 *    This routine is called continuously while the mouse is over the tactical map but there   *
 *    are no mouse buttons pressed. Typically, this adjusts the mouse shape and the pop-up     *
 *    help text.                                                                               *
 *                                                                                             *
 * INPUT:   shadow   -- Is the mouse hovering over shadowed terrain?                           *
 *                                                                                             *
 *          object   -- Pointer to the object that the mouse is currently over (may be NULL).  *
 *                                                                                             *
 *          action   -- This is the action that the currently selected object (if any) will    *
 *                      perform if the left mouse button were clicked at this location.        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/24/1995 JLB : Created.                                                                 *
 *   07/05/1995 JLB : Removed pop up help text for shadow and terrain after #3.                *
 *=============================================================================================*/
void DisplayClass::Mouse_Left_Up(Cell const & cell, bool shadow, ObjectClass * object, ActionType action, bool wsmall)
{
	IsTentative = false;

	AbstractClass * target = NULL;
	if (object != NULL) {
		target = object;
	} else {
		if (cell != CELL_NONE) {
			target = &Map[cell];
		}
	}

	if (IsWaypointMode && DraggedWaypoint != NULL) {
		if (PlayerPtr->Waypoint_At(cell) == NULL && Map.In_Local_Radar(cell)) {
			DraggedWaypoint->Location = cell.As_Coord();
			DraggedWaypoint->Location.Z = Map.Get_Height_GL(DraggedWaypoint->Location) + (Map[cell].IsUnderBridge ? BRIDGE_LEPTON_HEIGHT : 0);
		}
	}

	ActionType waypoint_action = Action_To_Waypoint_Action(action, cell);

	if (WaypointColor == RGBClass(0, 0, 0)) {
		unsigned short color = ((unsigned short *)MouseDrawer->Get_Translate_Table())[1];
		WaypointColor = RGBClass((color >> DSurface::RedRight) << DSurface::RedLeft,
								(color >> DSurface::GreenRight) << DSurface::GreenLeft,
								(color >> DSurface::BlueRight) << DSurface::BlueLeft
								);
	}

	switch (waypoint_action) {
		case ACTION_PLACE_WAYPOINT:
		case ACTION_NO_PLACE_WAYPOINT:
		case ACTION_LOOP_WAYPOINT_PATH:
			Update_Waypoint_Color(PlayerPtr->SelectedPath);
			break;

		case ACTION_ENTER_WAYPOINT_MODE:
		case ACTION_FOLLOW_WAYPOINT:
		case ACTION_SELECT_WAYPOINT:
		case ACTION_ATTACK_WAYPOINT:
		case ACTION_ENTER_WAYPOINT:
		case ACTION_PATROL_WAYPOINT: {
			PathType path = PATH_NONE;
			char waypt_id;
			WaypointClass * waypt = PlayerPtr->Waypoint_At(cell);
			PlayerPtr->Fetch_Waypoint_Data(waypt, path, waypt_id);
			if (!IsWaypointMode) {
				PlayerPtr->SelectedPath = path;
			}
			Update_Waypoint_Color(path);
			break;
		}

		default: {
			unsigned short * table = (unsigned short *)MouseDrawer->Get_Translate_Table();
			table[1] = DSurface::Build_Hicolor_Pixel(WaypointColor.Get_Red(), WaypointColor.Get_Green(), WaypointColor.Get_Blue());
			Update_Waypoint_Color(-1);
			if (!IsWaypointMode) {
				PlayerPtr->SelectedPath = PATH_NONE;
			}
			break;
		}
	}

	/*
	**	Don't allow selection of an object that is located in shadowed terrain.
	**	In fact, just show the normal move cursor in order to keep the shadowed
	**	terrain a mystery.
	*/
	if (shadow) {
		switch (waypoint_action) {
			case ACTION_TOTE:
				Set_Default_Mouse(MOUSE_NO_TOTE, wsmall);
				break;

			case ACTION_NO_ENTER:
			case ACTION_NO_ENTER_TUNNEL:
				Set_Default_Mouse(MOUSE_NO_ENTER, wsmall);
				break;

			case ACTION_DAMAGE:
				Set_Default_Mouse(MOUSE_NORMAL, wsmall);
				break;

			case ACTION_GREPAIR:
				Set_Default_Mouse(MOUSE_NORMAL, wsmall);
				break;

			case ACTION_NO_DEPLOY:
				Set_Default_Mouse(MOUSE_NO_DEPLOY, wsmall);
				break;

			case ACTION_GUARD_AREA:
				Set_Default_Mouse(MOUSE_AREA_GUARD, wsmall);
				break;

			case ACTION_CHEM_BOMB:
				Set_Default_Mouse(MOUSE_CHEMBOMB, wsmall);
				break;

			case ACTION_NONE:
				Set_Default_Mouse(MOUSE_NORMAL, wsmall);
				break;

			case ACTION_NO_SELL:
			case ACTION_SELL:
			case ACTION_SELL_UNIT:
				Set_Default_Mouse(MOUSE_NO_SELL_BACK, wsmall);
				break;

			case ACTION_NO_GREPAIR:
			case ACTION_NO_REPAIR:
			case ACTION_REPAIR:
				Set_Default_Mouse(MOUSE_NO_REPAIR, wsmall);
				break;

			case ACTION_NUKE_BOMB:
				Set_Default_Mouse(MOUSE_NUCLEAR_BOMB, wsmall);
				break;

			case ACTION_TOGGLE_POWER:
			case ACTION_NO_TOGGLE_POWER:
				Set_Default_Mouse(MOUSE_NO_TOGGLE_POWER, wsmall);
				break;

			case ACTION_EMPULSE:
				Set_Default_Mouse(MOUSE_EM_PULSE, wsmall);
				break;

			case ACTION_ION_CANNON:
			case ACTION_DROP_POD:
				Set_Default_Mouse(MOUSE_AIR_STRIKE, wsmall);
				break;

			case ACTION_EMPULSE_RANGE:
				Set_Default_Mouse(MOUSE_EM_PULSE_RANGE, wsmall);
				break;

			case ACTION_HEAL:
				Set_Default_Mouse(MOUSE_HEAL, wsmall);
				break;

			case ACTION_NOMOVE:
				if (CurrentObject.Count() && CurrentObject[0]->Is_Techno() && ((TechnoClass *)CurrentObject[0])->TClass->IsMoveToShroud) {
					Set_Default_Mouse(MOUSE_CAN_MOVE, wsmall);
					break;
				}
				Set_Default_Mouse(MOUSE_NO_MOVE, wsmall);
				break;
				// Fall into next case for non aircraft object types.

			case ACTION_MOVE:
			case ACTION_ATTACK:
				Set_Default_Mouse(MOUSE_CAN_MOVE, wsmall);
				break;

			case ACTION_PLACE_WAYPOINT:
				Set_Default_Mouse(MOUSE_PLACE_WAYPOINT, wsmall);
				break;

			case ACTION_NO_PLACE_WAYPOINT:
				Set_Default_Mouse(MOUSE_NO_PLACE_WAYPOINT, wsmall);
				break;

			case ACTION_ENTER_WAYPOINT_MODE:
				Set_Default_Mouse(MOUSE_ENTER_WAYPOINT_MODE, wsmall);
				break;

			case ACTION_SELECT_WAYPOINT:
				Set_Default_Mouse(MOUSE_SELECT_WAYPOINT, wsmall);
				break;

			case ACTION_LOOP_WAYPOINT_PATH:
				Set_Default_Mouse(MOUSE_LOOP_WAYPOINT_PATH, wsmall);
				break;

			case ACTION_ATTACK_WAYPOINT:
				Set_Default_Mouse(MOUSE_ATTACK_WAYPOINT, wsmall);
				break;

			case ACTION_PATROL_WAYPOINT:
				Set_Default_Mouse(MOUSE_PATROL_WAYPOINT, wsmall);
				break;

			case ACTION_FOLLOW_WAYPOINT:
				Set_Default_Mouse(MOUSE_FOLLOW_WAYPOINT, wsmall);
				break;

			case ACTION_ENTER_WAYPOINT:
				Set_Default_Mouse(MOUSE_ENTER_WAYPOINT, wsmall);
				break;

			default:
				Set_Default_Mouse(MOUSE_NORMAL, wsmall);
				break;
		}
	} else {

		/*
		**	Change the mouse shape according to the default action that will occur
		**	if the mouse button were clicked at this location.
		*/
		switch (waypoint_action) {
			case ACTION_TOTE:
				Set_Default_Mouse(MOUSE_TOTE, wsmall);
				break;

			case ACTION_NO_ENTER:
				Set_Default_Mouse(MOUSE_NO_ENTER, wsmall);
				break;

			case ACTION_GREPAIR:
				Set_Default_Mouse(MOUSE_GREPAIR, wsmall);
				break;

			case ACTION_TOGGLE_SELECT:
			case ACTION_SELECT:
				Set_Default_Mouse(MOUSE_CAN_SELECT, wsmall);
				break;

			case ACTION_NO_DEPLOY:
				Set_Default_Mouse(MOUSE_NO_DEPLOY, wsmall);
				break;

			case ACTION_GUARD_AREA:
				Set_Default_Mouse(MOUSE_AREA_GUARD, wsmall);
				break;

			case ACTION_CHEM_BOMB:
				Set_Default_Mouse(MOUSE_CHEMBOMB, wsmall);
				break;

			case ACTION_MOVE:
			case ACTION_RALLY_TO_POINT:
				Set_Default_Mouse(MOUSE_CAN_MOVE, wsmall);
				break;

			case ACTION_ATTACK:
				if (target != NULL && CurrentObject.Count() == 1 && CurrentObject[0]->Is_Techno() && ((TechnoClass *)CurrentObject[0])->In_Range(target, 0)) {
					Set_Default_Mouse(MOUSE_STAY_ATTACK, wsmall);
					break;
				}
				// fall into next case.

			case ACTION_HARVEST:
				Set_Default_Mouse(MOUSE_CAN_ATTACK, wsmall);
				break;

			case ACTION_SABOTAGE:
				Set_Default_Mouse(MOUSE_DEMOLITIONS, wsmall);
				break;

			case ACTION_ENTER:
			case ACTION_CAPTURE:
			case ACTION_ENTER_TUNNEL:
				Set_Default_Mouse(MOUSE_ENTER, wsmall);
				break;

			case ACTION_NOMOVE:
				Set_Default_Mouse(MOUSE_NO_MOVE, wsmall);
				break;

			case ACTION_NO_SELL:
				Set_Default_Mouse(MOUSE_NO_SELL_BACK, wsmall);
				break;

			case ACTION_NO_REPAIR:
			case ACTION_NO_GREPAIR:
				Set_Default_Mouse(MOUSE_NO_REPAIR, wsmall);
				break;

			case ACTION_SELF:
				Set_Default_Mouse(MOUSE_DEPLOY, wsmall);
				break;

			case ACTION_REPAIR:
				Set_Default_Mouse(MOUSE_REPAIR, wsmall);
				break;

			case ACTION_SELL_UNIT:
				Set_Default_Mouse(MOUSE_SELL_UNIT, wsmall);
				break;

			case ACTION_NO_TOGGLE_POWER:
				Set_Default_Mouse(MOUSE_NO_TOGGLE_POWER, wsmall);
				break;

			case ACTION_TOGGLE_POWER:
				Set_Default_Mouse(MOUSE_TOGGLE_POWER, wsmall);
				break;

			case ACTION_SELL:
				Set_Default_Mouse(MOUSE_SELL_BACK, wsmall);
				break;

			case ACTION_NUKE_BOMB:
				Set_Default_Mouse(MOUSE_NUCLEAR_BOMB, wsmall);
				break;

			case ACTION_EMPULSE:
				Set_Default_Mouse(MOUSE_EM_PULSE, wsmall);
				break;

			case ACTION_EMPULSE_RANGE:
				Set_Default_Mouse(MOUSE_EM_PULSE_RANGE, wsmall);
				break;

			case ACTION_ION_CANNON:
			case ACTION_DROP_POD:
				Set_Default_Mouse(MOUSE_AIR_STRIKE, wsmall);
				break;

			case ACTION_HEAL:
				Set_Default_Mouse(MOUSE_HEAL, wsmall);
				break;

			case ACTION_PLACE_WAYPOINT:
				Set_Default_Mouse(MOUSE_PLACE_WAYPOINT, wsmall);
				break;

			case ACTION_NO_PLACE_WAYPOINT:
				Set_Default_Mouse(MOUSE_NO_PLACE_WAYPOINT, wsmall);
				break;

			case ACTION_ENTER_WAYPOINT_MODE:
				Set_Default_Mouse(MOUSE_ENTER_WAYPOINT_MODE, wsmall);
				break;

			case ACTION_SELECT_WAYPOINT:
				Set_Default_Mouse(MOUSE_SELECT_WAYPOINT, wsmall);
				break;

			case ACTION_LOOP_WAYPOINT_PATH:
				Set_Default_Mouse(MOUSE_LOOP_WAYPOINT_PATH, wsmall);
				break;

			case ACTION_ATTACK_WAYPOINT:
				Set_Default_Mouse(MOUSE_ATTACK_WAYPOINT, wsmall);
				break;

			case ACTION_PATROL_WAYPOINT:
				Set_Default_Mouse(MOUSE_PATROL_WAYPOINT, wsmall);
				break;

			case ACTION_FOLLOW_WAYPOINT:
				Set_Default_Mouse(MOUSE_FOLLOW_WAYPOINT, wsmall);
				break;

			case ACTION_ENTER_WAYPOINT:
				Set_Default_Mouse(MOUSE_ENTER_WAYPOINT, wsmall);
				break;

			default:
				Set_Default_Mouse(MOUSE_NORMAL, wsmall);
				break;
		}
	}
}


void Bandbox_Selection_Callback(ObjectClass *object);

/***********************************************************************************************
 * DisplayClass::Mouse_Left_Release -- Handles the left mouse button release.                  *
 *                                                                                             *
 *    This routine is called when the left mouse button is released over the tactical map.     *
 *    The release event is the workhorse of the game. Most actions occur at the moment of      *
 *    mouse release.                                                                           *
 *                                                                                             *
 * INPUT:   cell     -- The cell that the mouse is over.                                       *
 *                                                                                             *
 *          x,y      -- The mouse pixel coordinate.                                            *
 *                                                                                             *
 *          object   -- Pointer to the object that the mouse is over.                          *
 *                                                                                             *
 *          action   -- The action that the currently selected object (if any) will            *
 *                      perform.                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/24/1995 JLB : Created.                                                                 *
 *   03/27/1995 JLB : Handles sell and repair actions.                                         *
 *=============================================================================================*/
void DisplayClass::Mouse_Left_Release(Coord const & coord, Cell const & cell, ObjectClass * object, ActionType action, bool wsmall)
{
	if (object != NULL && ScenarioActive && GameActive && !Debug_Map && object->Visual_Character() == VISUAL_HIDDEN) {
		object = NULL;
	}

	if (PendingObjectPtr) {

		if (PendingObject->RTTI == RTTI_BUILDINGTYPE) {
			ProximityCheck = Passes_Proximity_Check(PendingObject, PendingHouse, CursorSize, ZoneCell+ZoneOffset);
		}

		if (PendingObject->RTTI == RTTI_BUILDINGTYPE && object != NULL && object->RTTI == RTTI_BUILDING) {
			BuildingClass *building = (BuildingClass *)object;
			if (building->Can_Upgrade((const BuildingTypeClass *)PendingObject, PlayerPtr)) {
				ProximityCheck = true;
			}
		}

		/*
		**	Try to place the pending object onto the map.
		*/
		if (ProximityCheck && ShroudCheck) {
			OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::PLACE, PendingObjectPtr->RTTI, cell + ZoneOffset));
		} else {
			Speak(VOX_DEPLOY);
		}

	} else {

		if (IsRubberBand) {
			TacticalMap->IsToRedraw = true;

			if (!Keyboard->Down(KN_LSHIFT)) {
				Unselect_All();
			}

			TacticalMap->Select_Rubber_Band(Bandbox_Selection_Callback);
			TechnoClass::Reset_Action_Line_Timer();

			IsRubberBand = false;
			Set_Default_Mouse(MOUSE_NORMAL, wsmall);
			IsTentative = false;
			drag_select_aborted = true;

		} else {

			/*
			**	Toggle the select state of the object.
			*/
			if (action == ACTION_TOGGLE_SELECT) {
				if (!object || !CurrentObject.Count() || !CurrentObject[0]->Owner_HouseClass()->Is_Player_Control()) {
					action = ACTION_SELECT;
				} else {
					if (object->IsSelected) {
						object->Unselect();
					} else {
						object->Select();
					}
				}
			}

			/*
			**	Selection of other object action.
			*/
			if ((action == ACTION_SELECT && object != NULL) || (action == ACTION_NONE && object && object->Class_Of()->IsSelectable && !object->IsSelected)) {
				BuildingClass *building = (BuildingClass *)object;
				if (building->RTTI != RTTI_BUILDING || building->House->Is_Player_Control() || building->TranslucencyLevel != 15 || building->Is_Sensed_By_Player()) {
					Unselect_All();
					object->Select();
					Set_Default_Mouse(MOUSE_NORMAL, wsmall);
					TechnoClass::Reset_Action_Line_Timer();
				}
			}

			/*
			**	If an action was detected as possible, then pass this action event
			**	to all selected objects.
			*/
			if (action != ACTION_NONE && action != ACTION_SELECT) {

				/*
				**	Pass the action to all the selected objects. But first, redetermine
				**	what action that object should perform. This, seemingly redundant
				**	process, is necessary since multiple objects could be selected and each
				**	might perform a different action when the click occurs.
				*/
				Active_Click(object, cell, action);
				TechnoClass::Reset_Action_Line_Timer();

				if (action == ACTION_TOGGLE_POWER) {
					if (object->RTTI == RTTI_BUILDING) {
						BuildingClass *building = ((BuildingClass *)object);
						if (building->IsOn) {
							OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::POWEROFF, TargetClass(building)));
						} else {
							OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::POWERON, TargetClass(building)));
						}
					}
				}

				if (action == ACTION_LOOP_WAYPOINT_PATH) {
					Coord coord = cell.As_Coord();
					coord.Z = (Map[cell].IsUnderBridge ? BRIDGE_LEPTON_HEIGHT : 0) + Map.Get_Height_GL(coord);
					PlayerPtr->Select_Waypoint(coord);
				}

				if (action == ACTION_DRAG_WAYPOINT) {
					if (DraggedWaypoint != NULL) {
						if (!PlayerPtr->Waypoint_At(cell) && Map.In_Local_Radar(cell)) {
							DraggedWaypoint->Location = cell;
							DraggedWaypoint->Location.Z = (Map[cell].IsUnderBridge ? BRIDGE_LEPTON_HEIGHT : 0) + Map.Get_Height_GL(DraggedWaypoint->Location);
						}
						DraggedWaypoint = NULL;
						Show_Mouse();
					}
				}

				if (action == ACTION_PLACE_WAYPOINT) {
					if (PlayerPtr->SelectedPath != PATH_NONE) {
						Coord coord = cell.As_Coord();
						coord.Z = (Map[cell].IsUnderBridge ? BRIDGE_LEPTON_HEIGHT : 0) + Map.Get_Height_GL(coord);
						PlayerPtr->Place_Waypoint(coord);
						if (!PlayerPtr->Can_Add_Waypoint_To_Path()) {
							Waypoint_Mode_Control(0, false);
						}
					}
				}

				if (action == ACTION_ENTER_WAYPOINT_MODE) {
					char dummy;
					WaypointClass * wp = PlayerPtr->Waypoint_At(cell);
					PlayerPtr->Fetch_Waypoint_Data(wp, PlayerPtr->SelectedPath, dummy);
					Waypoint_Mode_Control(1, true);
				}

				if (action == ACTION_SELECT_WAYPOINT) {
					char dummy;
					WaypointClass * wp = PlayerPtr->Waypoint_At(cell);
					PlayerPtr->Fetch_Waypoint_Data(wp, PlayerPtr->SelectedPath, dummy);
					Update_Waypoint_Color(PlayerPtr->SelectedPath);
					DraggedWaypoint = wp;
					DraggedWaypointCoord = wp->Location;
					Hide_Mouse();
				}

				if (action == ACTION_REPAIR && object->RTTI == RTTI_BUILDING) {
					OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::REPAIR, TargetClass(object)));
				}

				if (action == ACTION_SELL_UNIT && object) {
					switch ((RTTIType)object->RTTI) {
						case RTTI_AIRCRAFT:
						case RTTI_UNIT:
							OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::SELL, TargetClass(object)));
							break;

						default:
							break;
					}

				}
				if (action == ACTION_SELL) {
					if (object) {
						OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::SELL, TargetClass(object)));
					} else {
						OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::SELLCELL, cell));
					}
				}

				SuperWeaponTypeClass *stype;
				if (PlayerPtr->TargetingSW != NULL) {
					stype = PlayerPtr->TargetingSW->Class;
				} else {
					stype = SuperWeaponTypeClass::From_Action(action);
				}

				if (stype != NULL) {
					OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::SPECIAL_PLACE, stype->HeapID, cell));
				}

				PlayerPtr->TargetingSW = NULL;
			}

			IsTentative = false;
		}
	}
}


/// <summary>
/// Selects an object that the rubber band has caught.
/// This routine is handed to the tactical map's rubber band scan so that it is called for
/// every object inside the band. Only the player's own selectable objects are taken, and
/// buildings are passed over unless they are of the sort that can undeploy into a unit.
/// </summary>
/// <param name="object">The object that the rubber band has caught.</param>
void Bandbox_Selection_Callback(ObjectClass *object)
{
	HouseClass *house = object->Owner_HouseClass();
	BuildingClass *bptr = object->RTTI == RTTI_BUILDING ? (BuildingClass *)object : NULL;

	if (house != NULL && house->Is_Player_Control()) {
		if (object->Class_Of()->IsSelectable) {
			bool selectable = false;
			if (bptr != NULL) {
				if (bptr->Class->UndeploysInto != NULL && !bptr->Class->IsConstructionYard && !bptr->Class->IsMobileWar) {
					selectable = true;
				}
			} else {
				selectable = true;
			}
			if (selectable) {
				if (!object->IsInLimbo) {
					if (object->Select()) {
						AllowVoice = false;
					}
				}
			}
		}
	}
}


/***********************************************************************************************
 * DisplayClass::Mouse_Left_Press -- Handles the left mouse button press.                      *
 *                                                                                             *
 *    Handle the left mouse button press while over the tactical map. If it isn't is           *
 *    repair or sell mode, then a tentative transition to rubber band mode is flagged. If the  *
 *    mouse moves a sufficient distance from this recorded position, then rubber band mode     *
 *    is officially started.                                                                   *
 *                                                                                             *
 * INPUT:   x,y   -- The mouse coordinates at the time of the press.                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void DisplayClass::Mouse_Left_Press(Point2D const & point)
{
	if (!IsRepairMode && !IsPowerMode && !IsSellMode && IsTargettingMode == SUPER_NONE && !PendingObject) {
		IsTentative = true;
		BandX = point.X;
		BandY = point.Y;
		NewX = point.X;
		NewY = point.Y;
	}
}


/// <summary>
/// Clamps a value so that it lies within the specified range.
/// </summary>
/// <param name="original">The value that is to be clamped.</param>
/// <param name="minval">The lowest value that is allowed.</param>
/// <param name="maxval">One past the highest value that is allowed.</param>
/// <returns>Returns with the value forced into the range.</returns>
template<class T> inline
T Bound2(T original, T minval, T maxval)
{
	T ret = original;
	if (ret < minval) ret = minval;
	if (ret >= maxval) ret = maxval-1;
	return(ret);
};

/***********************************************************************************************
 * DisplayClass::Mouse_Left_Held -- Handles the left button held down.                         *
 *                                                                                             *
 *    This routine is called continuously while the left mouse button is held down over        *
 *    the tactical map. This handles the rubber band mode detection and dragging.              *
 *                                                                                             *
 * INPUT:   x,y   -- The mouse coordinate.                                                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void DisplayClass::Mouse_Left_Held(Point2D const & point)
{
	if (IsRubberBand && !IsWaypointMode) {
		Rect rect = TacticalRect;
		Point2D npoint = point;
		int x = Bound2(npoint.X, 0, rect.Width);
		int y = Bound2(npoint.Y, 0, rect.Height);
		npoint.X = x;
		npoint.Y = y;
		if (npoint != (Point2D &)NewX) {
			TacticalMap->IsToRedraw = true;
			TacticalMap->Modify_Rubber_Band(npoint);
		}
	} else {

		/*
		**	If the mouse is still held down while a tentative extended select is possible, then
		**	check to see if the mouse has moved a sufficient distance in order to activate
		**	extended select mode.
		*/
		if (IsTentative) {

			/*
			**	The mouse must have moved a minimum distance before rubber band mode can be
			**	initiated.
			*/
			if ((point - (Point2D &)BandX).Length() > 4) {
				IsRubberBand = true;
				IsTentative = false;
				if (!IsWaypointMode) {
					TacticalMap->IsToRedraw = true;
					TacticalMap->Start_Rubber_Band((Point2D &)BandX);
					Map.Override_Mouse_Shape(MOUSE_NORMAL);
				}
			}
		}
	}
}


/***********************************************************************************************
 * DisplayClass::Compute_Start_Pos -- Computes player's start pos from unit coords.            *
 *                                                                                             *
 * Use this function in multiplayer games, to compute the scenario starting Tactical Pos.      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/28/1995 JLB : Commented.                                                               *
 *   06/26/1995 JLB : Fixed building loop.                                                     *
 *   10/20/1996 JLB : Doesn't wrap.                                                            *
 *=============================================================================================*/
void DisplayClass::Compute_Start_Pos(void)
{
	/*
	**	Find the summation cell-x & cell-y for all the player's units, infantry,
	**	and buildings.  Buildings are weighted so that they count 16 times more
	**	than units or infantry.
	*/
	Coord coord(0,0,0);
	int num = 0;
	for (int i = 0; i < Technos.Count(); i++) {
		TechnoClass * tech = Technos[i];
		if (!tech->IsInLimbo && tech->IsOwnedByPlayer) {
			coord += tech->PositionCoord;
			num++;
		}
	}

	coord.Z = 0;

	/*
	**	Divide each coord by 'num' to compute the average value
	*/
	if (num != 0) {
		coord /= num;
	}

	/*
	**	Tactical position is based on the cell of the upper left corner. Make adjustments
	**	and bound the calculated location to the map dimensions.
	*/
	Scen->Views[0] = Scen->Views[1] = Scen->Views[2] = Scen->Views[3] = Cell(coord);
	Scen->AltHome = Scen->Home;

	if (TacticalMap != NULL) {
		TacticalMap->Set_Tactical_Position(coord);
	}
}


/***********************************************************************************************
 * DisplayClass::Sell_Mode_Control -- Controls the sell mode.                                  *
 *                                                                                             *
 *    This routine will control the sell mode for the player.                                  *
 *                                                                                             *
 * INPUT:   control  -- The mode to set the sell state to.                                     *
 *                      0  = Turn sell mode off.                                               *
 *                      1  = Turn sell mode on.                                                *
 *                      -1 = Toggle sell mode.                                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void DisplayClass::Sell_Mode_Control(int control)
{
	bool mode = IsSellMode;
	switch (control) {
		case 0:
			mode = false;
			break;

		case -1:
			mode = (IsSellMode == false);
			break;

		case 1:
			mode = true;
			break;
	}

	if (mode != IsSellMode && !PendingObject) {
		Set_Default_Mouse(MOUSE_NORMAL, false);
		IsRepairMode = false;
		IsPowerMode = false;
		IsWaypointMode = false;
		if (mode && PlayerPtr->CurBuildings > 0) {
			IsSellMode = true;
			Unselect_All();
		} else {
			IsSellMode = false;
			Revert_Mouse_Shape();
		}
	}
}


/// <summary>
/// Controls the waypoint mode.
/// This routine will control the waypoint editing mode for the player. Turning the mode on
/// cancels the other special modes and starts a fresh path; turning it off puts any
/// waypoint that was being dragged back where it came from.
/// </summary>
/// <param name="control">The mode to set the waypoint state to. Zero turns waypoint mode
/// off, one turns it on, and -1 toggles it.</param>
/// <param name="edit_selected_path">Should the path already selected be edited rather than a new one
/// started?</param>
void DisplayClass::Waypoint_Mode_Control(int control, bool edit_selected_path)
{
	bool mode = IsWaypointMode;
	switch (control) {
		case 0:
			mode = false;
			break;

		case -1:
			mode = (mode == false);
			break;

		case 1:
			mode = true;
			break;
	}

	if (mode != IsWaypointMode && !PendingObject) {
		IsRepairMode = false;
		IsSellMode = false;
		IsPowerMode = false;
		PathType path = PlayerPtr->New_Waypoint_Path();
		if (mode) {
			if (path == PATH_NONE && (!edit_selected_path || PlayerPtr->SelectedPath == PATH_NONE)) {
				IsWaypointMode = false;
				Revert_Mouse_Shape();
				DraggedWaypoint = NULL;
			} else {
				IsWaypointMode = true;
				if (!edit_selected_path) {
					PlayerPtr->SelectedPath = path;
				}
				Unselect_All();
			}
		} else {
			IsWaypointMode = false;
			Revert_Mouse_Shape();
			WaypointClass *wptr = DraggedWaypoint;
			if (wptr != NULL) {
				DraggedWaypoint->Location = DraggedWaypointCoord;
				DraggedWaypoint = NULL;
				MouseCursor->Show_Mouse();
			}
		}
	}
}


/// <summary>
/// Controls the power mode.
/// This routine will control the power mode for the player. Turning the mode on cancels the
/// other special modes and unselects whatever the player had selected. The mode is refused
/// if the player has no buildings to redistribute power between.
/// </summary>
/// <param name="control">The mode to set the power state to. Zero turns power mode off, one
/// turns it on, and -1 toggles it.</param>
void DisplayClass::Power_Mode_Control(int control)
{
	bool mode = IsPowerMode;
	switch (control) {
		case 0:
			mode = false;
			break;

		case -1:
			mode = (IsPowerMode == false);
			break;

		case 1:
			mode = true;
			break;
	}

	if (mode != IsPowerMode && !PendingObject) {
		Set_Default_Mouse(MOUSE_NORMAL, false);
		IsRepairMode = false;
		IsSellMode = false;
		IsWaypointMode = false;
		if (mode && PlayerPtr->CurBuildings > 0) {
			IsPowerMode = true;
			Unselect_All();
		} else {
			IsPowerMode = false;
			Revert_Mouse_Shape();
		}
	}
}


/***********************************************************************************************
 * DisplayClass::Repair_Mode_Control -- Controls the repair mode.                              *
 *                                                                                             *
 *    This routine is used to control the repair mode for the player.                          *
 *                                                                                             *
 * INPUT:   control  -- The mode to set the repair to.                                         *
 *                      0 = Turn repair off.                                                   *
 *                      1 = Turn repair on.                                                    *
 *                      -1= Toggle repair state.                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void DisplayClass::Repair_Mode_Control(int control)
{
	bool mode = IsRepairMode;
	switch (control) {
		case 0:
			mode = false;
			break;

		case -1:
			mode = (IsRepairMode == false);
			break;

		case 1:
			mode = true;
			break;
	}

	if (mode != IsRepairMode && !PendingObject) {
		IsSellMode = false;
		IsPowerMode = false;
		IsWaypointMode = false;
		Set_Default_Mouse(MOUSE_NORMAL, false);
		if (mode && PlayerPtr->CurBuildings > 0) {
			IsRepairMode = true;
			Unselect_All();
		} else {
			IsRepairMode = false;
			Revert_Mouse_Shape();
		}
	}
}


/***********************************************************************************************
 * DisplayClass::Closest_Free_Spot -- Finds the closest cell sub spot that is free.            *
 *                                                                                             *
 *    Use this routine to find the sub cell spot closest to the coordinate specified that is   *
 *    free from occupation. Typical use of this is for infantry destination calculation.       *
 *                                                                                             *
 * INPUT:   coord -- The coordinate to use as the starting point when finding the closest      *
 *                   free spot.                                                                *
 *                                                                                             *
 *          any   -- Ignore occupation and just return the closest sub cell spot?              *
 *                                                                                             *
 * OUTPUT:  Returns with the coordinate of the closest free (possibly) sub cell location.      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/22/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
Coord DisplayClass::Closest_Free_Spot(Coord const & coord, bool any) const
{
	CellClass *cellptr = &Map[coord];
	return(cellptr->Closest_Free_Spot(coord, any, (cellptr->IsUnderBridge && coord.Z >= LEVEL_LEPTON_H * (cellptr->Height + BRIDGE_CELL_HEIGHT))));
}


/***********************************************************************************************
 * DisplayClass::Is_Spot_Free -- Determines if cell sub spot is free of occupation.            *
 *                                                                                             *
 *    Use this routine to determine if the coordinate (rounded to the nearest sub cell         *
 *    position) is free for placement. Typical use of this would be for infantry placement.    *
 *                                                                                             *
 * INPUT:   coord -- The coordinate to examine for "freeness". The coordinate is rounded to    *
 *          the nearest free sub cell spot.                                                    *
 *                                                                                             *
 * OUTPUT:  Is the sub spot indicated by the coordinate free from previous occupation?         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/22/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool DisplayClass::Is_Spot_Free(Coord const & coord, bool bridge) const
{
	return(Map[coord].Is_Spot_Free(CellClass::Spot_Index(coord), bridge));
}


/***********************************************************************************************
 * DisplayClass::Encroach_Shadow -- Causes the shadow to creep back by one cell.               *
 *                                                                                             *
 *    This routine will cause the shadow to creep back by one cell. Multiple calls to this     *
 *    routine will result in the shadow becoming more and more invasive until only the sight   *
 *    range of player controlled units will keep the shadow pushed back.                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void DisplayClass::Encroach_Shadow(void)
{
	int x;
	int y;
	Cell cell;

	for (y = 0; y < MAP_CELL_H; y++) {
		for (x = 0; x < MAP_CELL_W; x++) {
			cell.X = x;
			cell.Y = y;
			if (!In_Radar(cell)) continue;
			Cell c;
			c.X = x;
			c.Y = y;
			CellClass * cellptr = &(*this)[c];
			if (cellptr->IsVisible || !cellptr->IsMapped) continue;

			cellptr->IsToShroud = true;
		}
	}

	/*
	**	Mark all shadow edge cells to be fully shrouded. All adjacent mapped
	**	cell should become partially shrouded.
	*/
	for (y = 0; y < MAP_CELL_H; y++) {
		for (x = 0; x < MAP_CELL_W; x++) {
			cell.X = x;
			cell.Y = y;
			if (!In_Radar(cell)) continue;

			if ((*this)[cell].IsToShroud) {
				(*this)[cell].IsToShroud = false;
				Shroud_Cell(cell);
			}
		}
	}

	All_To_Look();

	Flag_To_Redraw(GS_REDRAW_TACTICAL);
}


/// <summary>
/// Causes the fog of war to creep back over the map.
/// This is the fog counterpart of Encroach_Shadow. Every cell that no player controlled
/// object can currently see is returned to the fogged condition, so that only what is
/// actually being watched stays clear.
/// </summary>
void DisplayClass::Encroach_Fog(void)
{
	Reset_Iterator();
	CellClass *cellptr = Iterate();
	while (cellptr) {
		if (!cellptr->IsFogVisible && cellptr->IsFogMapped) {
			cellptr->IsToFog = true;
		}
		cellptr = Iterate();
	}
	All_To_Look(false, true);
	Reset_Iterator();
	cellptr = Iterate();
	while (cellptr) {
		if (cellptr->IsToFog) {
			cellptr->IsToFog = false;
			Fog_Cell(cellptr->Fetch_CellID());
		}
		cellptr = Iterate();
	}
}


/// <summary>
/// Returns the specified cell to the fogged condition.
/// This routine is called when the fog of war is to regrow over a cell that the player can
/// no longer see. Adjacent cells are brought up to date as well, and may be fogged outright
/// when the partial fog artwork has no legal piece for the combination that would result.
/// </summary>
/// <param name="cell">The cell that the fog is to be regrown upon.</param>
void DisplayClass::Fog_Cell(Cell const & cell)
{
	if (!In_Radar(cell)) return;

	CellClass * cellptr = &(*this)[cell];
	bool fog = false;

	if (cellptr->IsFogMapped || cellptr->IsFogVisible) {
		fog = true;
	}

	cellptr->IsFogMapped = false;
	cellptr->IsFogVisible = false;
	cellptr->FogFrame = -2;

	if (cellptr->IsMapped) {
		TacticalMap->Flag_Cell(*cellptr);
	}

	for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
		Cell c = Adjacent_Cell(cell, dir);
		CellClass * cptr = &(*this)[c];
		int fog = TacticalMap->Cell_Shadow(cptr->Fetch_CellID(), true);

		if (fog == -2 && cptr->FogFrame != -2) {
			Fog_Cell(c);
		} else {
			if ((cptr->IsFogVisible || fog != cptr->FogFrame) && fog >= 0 && cptr->FogFrame >= -1) {
				cptr->FogFrame = fog;
				cptr->IsFogMapped = true;
				cptr->IsFogVisible = false;
				TacticalMap->Flag_Cell(*cptr);
			}
		}
	}
	if (fog) {
		cellptr->Fog_Cell();
	}
}


/***********************************************************************************************
 * DisplayClass::Shroud_Cell -- Returns the specified cell into the shrouded condition.        *
 *                                                                                             *
 *    This routine is called to add the shroud back to the cell specified. Typical of this     *
 *    would be when the shroud is to regenerate.                                               *
 *                                                                                             *
 * INPUT:   cell  -- The cell that the shroud is to be regenerated upon.                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Adjacent cells might be affected by this routine. The affect is determined      *
 *             according to the legality of the partial shadow artwork. In the illegal cases   *
 *             the adjacent cell might become shrouded as well.                                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1995 JLB : Created.                                                                 *
 *   06/17/1996 JLB : Modified to handle the new shadow pieces.                                *
 *=============================================================================================*/
void DisplayClass::Shroud_Cell(Cell const & cell)
{
	if (!In_Radar(cell)) return;

	CellClass * cellptr = &(*this)[cell];
	if (cellptr->IsMapped) {

		cellptr->IsMapped = false;
		cellptr->IsVisible = false;
		TacticalMap->Flag_Cell(*cellptr);

		/*
		**	Check adjacent cells. There might be some weird combination of
		**	shrouded cells such that more cells must be shrouded in order for
		**	this to work.
		*/
		for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
			Cell c = Adjacent_Cell(cell, dir);
			CellClass * cptr = &(*this)[c];

			/*
			**	If this adjacent cell must be completely shrouded as a result
			**	of the map change, yet it isn't already shrouded, then recursively
			**	shroud that cell.
			*/
			if (c != cell) {
				cptr->IsVisible = false;
				TacticalMap->Flag_Cell(*cptr);
			}
		}
	}
}


/***********************************************************************************************
 * DisplayClass::Read_INI -- Reads map control data from INI file.                             *
 *                                                                                             *
 *    This routine is used to read the map control data from the INI                           *
 *    file.                                                                                    *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the loaded INI file data.                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The TriggerClass INI data must have been read before calling this function.     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/27/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void DisplayClass::Read_INI(CCINIClass const & ini)
{
	/*
	**	Read the map dimensions.
	*/
	char const * const name = "Map";

	Rect rect = ini.Get_Rect(name, "Size", Rect(1, 1, 50, 50));

	Init_Cells();

	Set_Map_Dimensions(rect, true, 0, true);

	int cell_height = ini.Get_Int(name, "Level", 0);

	char fill[32];
	IsometricTileType ittype = IsometricTileTypeClass::ClearTile;
	ini.Get_String(name, "Fill", "Clear", fill, sizeof(fill));
	if (stricmp(fill, "Water") == 0) {
		ittype = IsometricTileTypeClass::WaterSet;
	}

	Reset_Iterator();
	CellClass *cptr = Iterate();
	while (cptr != NULL) {
		cptr->ITType = ittype;
		cptr->SubTile = 0;
		cptr->Height += cell_height;
		cptr = Iterate();
	}

	/*
	**	The theater is determined at this point. There is specific data that
	**	is custom to this data. Load the custom data (as it related to terrain)
	**	at this point.
	*/
	Scen->Theater = ini.Get_TheaterType(name, "Theater", THEATER_TEMPERATE);

	/*
	**	Now that the theater is known, init the entire map hierarchy
	*/
	Init(Scen->Theater);

	Session.Update_Progress(63);
	Call_Back();

	/*
	**	Special initializations occur when the theater is known.
	*/
	int uniqueid = Scen->UniqueID;
	TerrainTypeClass::Init(Scen->Theater);
	if (Scen->Theater != LastTheater) {
		IsometricTileTypeClass::Read_Control_File(Scen->Theater, true);
	} else {
		IsometricTileTypeClass::Clear_Use_Counts();
	}
	Scen->UniqueID = uniqueid + 10000;
	OverlayTypeClass::Init(Scen->Theater);
	UnitTypeClass::Init(Scen->Theater);
	InfantryTypeClass::Init(Scen->Theater);
	BuildingTypeClass::Init(Scen->Theater);
	BulletTypeClass::Init(Scen->Theater);
	AnimTypeClass::Init(Scen->Theater);
	AircraftTypeClass::Init(Scen->Theater);
	SmudgeTypeClass::Init(Scen->Theater);
	VeinholeMonsterClass::Init(Scen->Theater);

	Session.Update_Progress(65);
	Call_Back();

	/*
	**	Read the Waypoint entries.
	*/
	Scen->Read_Waypoints(ini);

	/*
	**	Set the starting position (do this after Init(), which clears the cells'
	**	IsWaypoint flags).
	*/
	if (!Scen->Is_Valid_Waypoint(Scen->Home)) {
		Scen->Set_Waypoint(Scen->Home, Cell((PlayRect.Width + PlayRect.Height) / 2, (PlayRect.Width + PlayRect.Height) / 2));
	}

	Cell homecell = Scen->Get_Waypoint_Cell(Scen->GlobalFlags[0].Value ? Scen->AltHome : Scen->Home);
	Scen->Views[0] = Scen->Views[1] = Scen->Views[2] = Scen->Views[3] = homecell;
	Coord homecoord = homecell;
	TacticalMap->Set_Tactical_Position(homecoord);

	/*
	 * Loop through all CellTags entries.
	 */
	char const * const celltags = "CellTags";
	int len = ini.Entry_Count(celltags);
	for (int index = 0; index < len; index++) {

		/*
		**	Get a cell trigger and cell assignment.
		*/
		char const * cellentry = ini.Get_Entry(celltags, index);
		TagTypeClass * tp;
		Cell cell;

		char buffer[128];
		if (ini.Get_String(celltags, cellentry, "", buffer, sizeof(buffer))) {
			tp = TagTypeClass::Find_Or_Make(buffer);
		} else {
			tp = NULL;
		}

		int val = atoi(cellentry);

		if (NewINIFormat >= 4) {
			cell = Cell(val % 1000, val / 1000);
		} else {
			cell = Cell(val % 128, val / 128);
		}

		if (tp != NULL && !(*this)[cell].Tag) {
			TagClass * tt = Find_Or_Make(tp);
			if (tt) {
				tt->Set_Position(cell);
				(*this)[cell].Attach_Tag(tt);
				TaggedCells.Add(cell);
			}
		}
	}
	Session.Update_Progress(67);
	Call_Back();

	/*
	**	Read the map template data.
	*/

	BSurface staging_buffer(640, 400, 2);

	int size = (staging_buffer.Width * staging_buffer.BBP) * staging_buffer.Height;

	static char const * const ISOMAPPACK1 = "IsoMapPack";
	len = ini.Get_UUBlock(ISOMAPPACK1, staging_buffer.Lock(), size);
	if (len > 0) {
		BufferStraw bstraw(staging_buffer.Lock(), len);
		Map.Read_Binary_1(bstraw);
		staging_buffer.Unlock();
	}
	staging_buffer.Unlock();

	static char const * const ISOMAPPACK2 = "IsoMapPack2";
	len = ini.Get_UUBlock(ISOMAPPACK2, staging_buffer.Lock(), size);
	if (len > 0) {
		BufferStraw bstraw(staging_buffer.Lock(), len);
		Map.Read_Binary_2(bstraw);
		staging_buffer.Unlock();
	}
	staging_buffer.Unlock();

	static char const * const ISOMAPPACK3 = "IsoMapPack3";
	len = ini.Get_UUBlock(ISOMAPPACK3, staging_buffer.Lock(), size);
	if (len > 0) {
		BufferStraw bstraw(staging_buffer.Lock(), len);
		Map.Read_Binary_3(bstraw);
		staging_buffer.Unlock();
	}
	staging_buffer.Unlock();

	static char const * const ISOMAPPACK4 = "IsoMapPack4";
	len = ini.Get_UUBlock(ISOMAPPACK4, staging_buffer.Lock(), size);
	if (len > 0) {
		BufferStraw bstraw(staging_buffer.Lock(), len);
		Map.Read_Binary_4(bstraw);
		staging_buffer.Unlock();
	}
	staging_buffer.Unlock();

	static char const * const ISOMAPPACK5 = "IsoMapPack5";
	std::size_t const record_size = sizeof(Cell) + sizeof(BlubCell.ITType)
		+ sizeof(BlubCell.SubTile) + sizeof(BlubCell.Height)
		+ sizeof(BlubCell.IsIceGrowthAllowed);
	std::size_t const maximum_payload_size = static_cast<std::size_t>(MAP_CELL_TOTAL) * record_size + sizeof(Cell);
	std::size_t const lzo_block_size = 8 * 1024;
	std::size_t const maximum_block_count = (maximum_payload_size + lzo_block_size - 1) / lzo_block_size;
	// Each block carries two 16-bit counts and has compressed storage twice its input size.
	std::size_t const maximum_decoded_size = maximum_block_count * (2 * lzo_block_size + 2 * sizeof(unsigned short));
	if (ini.Entry_Count(ISOMAPPACK5) > 0) {
		std::vector<unsigned char> decoded(maximum_decoded_size + 1);
		len = ini.Get_UUBlock(ISOMAPPACK5, decoded.data(), static_cast<int>(decoded.size()));
		if (len > 0 && static_cast<std::size_t>(len) < decoded.size()) {
			BufferStraw bstraw(decoded.data(), len);
			Map.Read_Binary_5(bstraw);
		} else if (len > 0) {
			DebugString("IsoMapPack5 exceeds the maximum terrain payload; ignoring the section.\n");
		}
	}

	Session.Update_Progress(68);
	Call_Back();

	IsometricTileTypeClass::Load_Tiles(Debug_Map || Debug_ForceScenario, Scen->IsRandom);

	Session.Update_Progress(69);
	Call_Back();

	LocalRect = ini.Get_Rect(name, "LocalSize", PlayRect);

	Map.Set_Local_Dimensions(LocalRect);

	LastTheater = Scen->Theater;
}


/***********************************************************************************************
 * DisplayClass::Write_INI -- Write the map data to the INI file specified.                    *
 *                                                                                             *
 *    This routine will output all the data of this map to the INI database specified.         *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI handler to store the map data to.                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Any existing map data in the INI database will be replaced by this function.    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void DisplayClass::Write_INI(CCINIClass & ini)
{
	char entry[20];

	/*
	**	Save the map parameters.
	*/
	static char const * const NAME = "Map";
	ini.Clear(NAME);
	ini.Put_TheaterType(NAME, "Theater", Scen->Theater);
	ini.Put_Rect(NAME, "Size", PlayRect);
	ini.Put_Rect(NAME, "LocalSize", LocalRect);

	/*
	**	Save the Waypoint entries.
	*/
	Scen->Write_Waypoints(ini);

	/*
	**	Save the cell's triggers.
	*/
	static char const * const CELLTRIG = "CellTags";
	ini.Clear(CELLTRIG);
	for (int y = 0; y < MAP_CELL_H; y++) {
		for (int x = 0; x < MAP_CELL_W; x++) {
			if ((*this)[Cell(x,y)].Tag != NULL) {
				TagClass * tp = (*this)[Cell(x,y)].Tag;
				if (tp != NULL && tp->Class != NULL) {

					/*
					**	Generate entry name.
					*/
					wsprintf(entry, "%d", x + (y * 1000));

					/*
					**	Save entry.
					*/
					ini.Put_String(CELLTRIG, entry, tp->Class->Name());
				}
			}
		}
	}

	/*
	**	Write the map template data out to the ini file.
	*/
	static char const * const MAPPACK1 = "IsoMapPack";
	static char const * const MAPPACK2 = "IsoMapPack2";
	static char const * const MAPPACK3 = "IsoMapPack3";
	static char const * const MAPPACK4 = "IsoMapPack4";
	static char const * const MAPPACK5 = "IsoMapPack5";
	BufferPipe bpipe(AlternateSurface->Lock(), AlternateSurface->Get_Width() * AlternateSurface->Get_Height() * AlternateSurface->Bytes_Per_Pixel());
	int len = Map.Write_Binary_5(bpipe);
	ini.Clear(MAPPACK1);
	ini.Clear(MAPPACK2);
	ini.Clear(MAPPACK3);
	ini.Clear(MAPPACK4);
	ini.Clear(MAPPACK5);
	if (len > 0) {
		ini.Put_UUBlock(MAPPACK5, AlternateSurface->Lock(), len);
		AlternateSurface->Unlock();
	}
	AlternateSurface->Unlock();
}


/// <summary>
/// Records the current state of the map into a buffer.
/// This routine takes a snapshot of the theater, the map bounds, the waypoint list and the
/// terrain of every cell, so that Restore_Map_State can put the map back the way it was.
/// </summary>
/// <param name="stash">The buffer to record the map state into.</param>
/// <returns>Returns with the number of bytes recorded into the buffer.</returns>
/// <remarks>Be sure that the buffer is big enough to hold an entry for every cell.</remarks>
int DisplayClass::Stash_Map_State(void * stash, int)
{
	unsigned char *data = (unsigned char *)stash;

	(*(unsigned int *)data) = Scen->Theater;
	data += sizeof(Scen->Theater);

	(*(Rect *)data) = PlayRect;
	data += sizeof(PlayRect);

	(*(Rect *)data) = LocalRect;
	data += sizeof(LocalRect);

	int *wcount = ((int *)data);
	data += sizeof(*wcount);

	int count = 0;

	for (int i = 0; i < WAYPT_COUNT; i++) {
		if (Scen->Is_Valid_Waypoint((WAYPOINT)i)) {

			(*(int *)data) = i;
			data += sizeof(i);

			int val = Scen->Get_Waypoint_Cell((WAYPOINT)i).X + Scen->Get_Waypoint_Cell((WAYPOINT)i).Y * 1000;
			(*(int *)data) = val;
			data += sizeof(val);

			count++;
		}
	}

	int *nptr = (int *)data;

	*wcount = count;

	data += sizeof(*nptr);

	int cnum = 0;
	Reset_Iterator();
	CellClass * cptr = Iterate();
	while (cptr != NULL) {

		(*(Cell *)data) = cptr->CellID;
		data += sizeof(cptr->CellID);

		(*(IsometricTileType *)data) = cptr->ITType;
		data += sizeof(cptr->ITType);

		(*(unsigned char *)data) = cptr->SubTile;
		data += sizeof(cptr->SubTile);

		(*(unsigned char *)data) = cptr->Height;
		data += sizeof(cptr->Height);

		(*(unsigned char *)data) = cptr->Ramp;
		data += sizeof(cptr->Ramp);

		(*(OverlayType *)data) = cptr->Overlay;
		data += sizeof(cptr->Overlay);

		(*(unsigned char *)data) = cptr->OverlayData;
		data += sizeof(cptr->OverlayData);

		(*(unsigned char *)data) = cptr->IsIceGrowthAllowed;
		data += sizeof(cptr->IsIceGrowthAllowed);

		unsigned int tag = 0;
		if (cptr->Tag != NULL) {
			if (cptr->Tag->Class != NULL) {
				tag = (unsigned int)cptr->Tag->Class;
			}
		}

		(*(unsigned int *)data) = tag;
		data += sizeof(tag);

		cnum++;
		cptr = Iterate();
	}
	*nptr = cnum;

	return(data - (unsigned char *)stash);
}


/// <summary>
/// Restores the map from a previously stashed state.
/// This routine puts back the theater, the map bounds, the waypoints and the terrain of
/// every cell as they were when the stash was taken, discarding whatever has happened to
/// the map since.
/// </summary>
/// <param name="stash">The buffer holding the stashed map state.</param>
/// <remarks>The buffer must have been filled by Stash_Map_State for this same map.</remarks>
void DisplayClass::Restore_Map_State(void * stash)
{
	unsigned char *data = (unsigned char *)stash;

	Scen->Theater = (TheaterType)(*(unsigned int *)data);
	data += sizeof(Scen->Theater);

	PlayRect = (*(Rect *)data);
	data += sizeof(PlayRect);

	LocalRect = (*(Rect *)data);
	data += sizeof(LocalRect);

	int wcount = (*(int *)data);
	data += sizeof(int);

	while (wcount > 0) {
		int wp = (*(int *)data);
		data += sizeof(wp);

		int val = (*(int *)data);
		data += sizeof(val);

		Cell cell(val % 1000, val / 1000);
		Scen->Set_Waypoint(wp, cell);
		wcount--;
	}

	int num = (*(int *)data);
	data += sizeof(num);

	Reset_Iterator();
	CellClass * cptr = Iterate();
	while (cptr != NULL) {

		//cptr->CellID = (*(Cell *)data);
		data += sizeof(cptr->CellID);

		cptr->ITType = (*(IsometricTileType *)data);
		data += sizeof(cptr->ITType);

		cptr->SubTile = (*(unsigned char *)data);
		data += sizeof(cptr->SubTile);

		cptr->Height = (*(unsigned char *)data);
		data += sizeof(cptr->Height);

		cptr->Ramp = (*(unsigned char *)data);
		data += sizeof(cptr->Ramp);

		cptr->Overlay = (*(OverlayType *)data);
		data += sizeof(cptr->Overlay);

		cptr->OverlayData = (*(unsigned char *)data);
		data += sizeof(cptr->OverlayData);

		cptr->IsIceGrowthAllowed = (*(unsigned char *)data);
		data += sizeof(cptr->IsIceGrowthAllowed);

		int tag = (*(unsigned int *)data);
		data += sizeof(tag);

		cptr = Iterate();
	}
}


/***********************************************************************************************
 * DisplayClass::All_To_Look -- Direct all objects to look around for the player.              *
 *                                                                                             *
 *    This routine will scan through all objects and tell them to look if they are supposed    *
 *    to be able to reveal the map for the player. This routine may be necessary in cases      *
 *    of gap generator reshroud logic.                                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/23/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void DisplayClass::All_To_Look(bool units_only, bool is_fog)
{
	for (int index = 0; index < Layer[LAYER_GROUND].Count(); index++) {
		TechnoClass * tech = Dynamic_Cast<TechnoClass *>(Layer[LAYER_GROUND][index]);
		if (tech != NULL) {
			if (tech->RTTI == RTTI_BUILDING && units_only) continue;

			if (tech->House->Is_Player_Control()) {
				if (tech->IsDiscoveredByPlayer) {
					tech->Look(false, is_fog);
				}
			} else {
				if (tech->RTTI == RTTI_BUILDING && Rule->IsAllyReveal && tech->House->Is_Ally(PlayerPtr)) {
					tech->Look(is_fog, false);
				}
			}
		}
	}
}


/// <summary>
/// Directs the objects near a point to look around for the player.
/// This routine works like All_To_Look, but only those objects whose sight reaches the
/// specified area are told to look. Use it when only part of the map needs revealing again
/// rather than the whole of it.
/// </summary>
/// <param name="center">The center of the area that is to be revealed.</param>
/// <param name="distance">How far past the center an object's sight may start, in leptons.</param>
void DisplayClass::Constrained_Look(Coord const & center, LEPTON distance)
{
	for (int index = 0; index < Layer[LAYER_GROUND].Count(); index++) {
		TechnoClass * tech = Dynamic_Cast<TechnoClass *>(Layer[LAYER_GROUND][index]);
		if (tech != NULL) {

//			if (tech->What_Am_I() == RTTI_BUILDING && units_only) continue;

			if (tech->House->Is_Player_Control()) {
				if (tech->IsDiscoveredByPlayer && Distance(center, tech->Center_Coord()) <= (tech->TClass->SightRange * CELL_LEPTON_W) + distance) {
					tech->Look();
				}
			} else {
				if (tech->RTTI == RTTI_BUILDING && Rule->IsAllyReveal && tech->House->Is_Ally(PlayerPtr) &&
					Distance(tech->Center_Coord(), center) <= (tech->TClass->SightRange * CELL_LEPTON_W) + distance) {
					tech->Look();
				}
			}
		}
	}
}


/***********************************************************************************************
 * DisplayClass::Center_Map -- Centers the map about the currently selected objects            *
 *                                                                                             *
 *    This routine will average the position of all the selected objects and then center       *
 *    the map about those objects.                                                             *
 *                                                                                             *
 * INPUT:   center   -- The is an optional center about override coordinate. If specified,     *
 *                      then the map will be centered about that coordinate. Otherwise it      *
 *                      will center about the average location of all selected objects.        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The map position changes by this routine.                                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/1995 JLB : Created.                                                                 *
 *   09/16/1996 JLB : Takes coordinate to center about (as override).                          *
 *=============================================================================================*/
void DisplayClass::Center_Map(void)
{
	int index;

	if (CurrentObject.Count() > 0) {

		Coord center(0,0,0);
		for (index = 0; index < CurrentObject.Count(); index++) {
			center += CurrentObject[index]->PositionCoord;
		}

		Coord coord = center;

		center /= CurrentObject.Count();

		if (CurrentObject.Count() > 2) {
			Coord bestc(0,0,0);
			int bestd = 0;
			for (index = 0; index < CurrentObject.Count(); index++) {
				int d = Distance(center, CurrentObject[index]->PositionCoord);
				if (d > bestd) {
					bestc = CurrentObject[index]->PositionCoord;
					bestd = d;
				}
			}

			center = (coord - bestc) / (CurrentObject.Count() - 1);
		}

		TacticalMap->Set_Tactical_Position(center);
	}
}


/// <summary>
/// Rebuilds the tile drawers for every cell on the map.
/// This routine is called when the drawing options change, since the drawer each cell holds
/// on to is no longer the correct one to use.
/// </summary>
void DisplayClass::Reinit_Cell_Drawers(void)
{
	IsometricTileTypeClass::Init_Drawers();
	Map.Reset_Iterator();
	CellClass * cptr = Map.Iterate();
	BlubCell.Drawer = NULL;
	while (cptr != NULL) {
		cptr->Init_Drawer();
		cptr = Map.Iterate();
	}
	Flag_To_Redraw(GS_REDRAW_TACTICAL);
}


/// <summary>
/// Recalculates the lighting of every cell on the map.
/// This routine is called when something changes the lighting of the whole world -- an ion
/// storm arriving or departing, or the lighting rules being altered.
/// </summary>
void DisplayClass::Update_Cell_Colors(void)
{
	Map.Reset_Iterator();
	CellClass * cptr = Map.Iterate();
	while (cptr != NULL) {
		cptr->Recalc_Light();
		cptr = Map.Iterate();
	}
}


/// <summary>
/// Fetches the help text for whatever lies under the mouse.
/// This routine is called by the help system when the mouse lingers over the tactical map.
/// Enemy objects are described in generic terms so that the player learns nothing that
/// should not be visible, and cloaked or invisible objects yield no text at all.
/// </summary>
/// <param name="id">The identifier of the gadget that the mouse is over.</param>
/// <returns>Returns with a pointer to the help text to display, or NULL if there is nothing
/// worth saying about this spot.</returns>
char const * DisplayClass::Help_Text(int id)
{
	if (id < 500 || id > 900 || Is_Scrolling()) {
		return(NULL);
	}

	Cell cell;
	Coord coord;
	ObjectClass * object;
	bool fog, shadow;

	Map.Resolve_Point(Get_Mouse_Point() - TacticalRect.TopLeft, cell, coord, object, fog, shadow);

	/*
	**	Give a generic help message when over shadow terrain.
	*/
	if (!Map[coord].IsMapped && MainWindow) {
		return(Fetch_String(TXT_SHADOW));
	}

	TechnoClass * techno = Dynamic_Cast<TechnoClass *>(object);
	if (techno != NULL && !techno->IsOwnedByPlayer) {
		if (techno->Cloak == CLOAKED && !Map[techno->Center_Coord()].Is_Sensed(PlayerPtr->HeapID)) {
			return(NULL);
		}
		if (techno->TClass->IsInvisible) {
			return(NULL);
		}
	}

	/*
	**	If the mouse is held over objects on the map, then help text may
	**	pop up that tells what the object is. This call informs the help
	**	system of the text name for the object under the mouse.
	*/
	if (object != NULL) {
		char const * text;

		/*
		**	Fetch the name of the object. If it is an enemy object, then
		**	the exact identity is glossed over with a generic text.
		*/
		text = object->Full_Name();

		if (techno != NULL) {
			if (object->RTTI != RTTI_BUILDING || !((BuildingClass *)object)->IsNominal) {
				if (!dynamic_cast<TechnoTypeClass const *>(object->Class_Of())->IsNominal) {

					if (!techno->House->Is_Ally(PlayerPtr)) {
						switch ((RTTIType)object->RTTI) {
							case RTTI_INFANTRY:
								text = Fetch_String(TXT_ENEMY_SOLDIER);
								break;

							case RTTI_UNIT:
							case RTTI_AIRCRAFT:
								text = Fetch_String(TXT_ENEMY_VEHICLE);
								break;

							case RTTI_BUILDING:
								text = Fetch_String(TXT_ENEMY_STRUCTURE);
								break;
						}
					}
				}
			}
		} else {
			/*
			 * This reports the tiberium in the cell instead of the object, so the tiberium
			 * name only ever appears when the cell holds a non-techno object such as a
			 * terrain object.
			 */
			text = ((*this)[cell].Tiberium_Name());
		}
		return(text);
	}

	return(NULL);
}


/// <summary>
/// Handles the sidebar being repositioned.
/// Each layer of the map overrides this routine so that it can move whatever gadgets it
/// owns to suit the sidebar's new location.
/// </summary>
void DisplayClass::Reposition_Sidebar(void)
{
	if (UnusedWindow) {
		CloseWindow(UnusedWindow);
		UnusedWindow = 0;
	}
}


/// <summary>
/// Handles a window message by handing it straight back to Windows.
/// </summary>
/// <returns>Returns with whatever the default window procedure decides.</returns>
LRESULT DisplayClass::Windows_Message_Proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return(DefWindowProc(hWnd, Msg, wParam, lParam));
}


/// <summary>
/// Loads the display layers from the save game stream.
/// </summary>
/// <param name="stream">The stream to read the layers from.</param>
/// <returns>Returns with S_OK if every layer was read, otherwise the failure code of the
/// layer that could not be read.</returns>
HRESULT DisplayClass::Load(IStream * stream)
{
	HRESULT result = S_OK;
	for (LayerType layer = LAYER_FIRST; layer < LAYER_COUNT; layer++) {
		result = Layer[layer].Load(stream);
		if (FAILED(result)) break;
	}
	return(result);
}


/// <summary>
/// Saves the display layers to the save game stream.
/// </summary>
/// <param name="stream">The stream to write the layers to.</param>
/// <returns>Returns with S_OK if every layer was written, otherwise the failure code of the
/// layer that could not be written.</returns>
HRESULT DisplayClass::Save(IStream * stream)
{
	HRESULT result = S_OK;
	for (LayerType layer = LAYER_FIRST; layer < LAYER_COUNT; layer++) {
		result = Layer[layer].Save(stream);
		if (FAILED(result)) break;
	}
	return(result);
}


/// <summary>
/// Lists the members the display holds.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void DisplayClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	// Layer -- the display layers are shared, and Load and Save carry them through their own
	// persistence.
	stream.Serialize(ZoneCell);
	stream.Serialize(ZoneOffset);

	// CursorSize -- it names a scratch list local to Set_Cursor_Shape whose address means nothing
	// in a fresh process, and the placement cursor is laid down again the moment the pending
	// object is next moved.
	stream.Serialize(ProximityCheck);
	stream.Serialize(ShroudCheck);
	stream.Serialize(FollowingObject);
	stream.Serialize(FollowingObjectPtr);
	stream.Serialize(PendingObjectPtr);
	stream.Serialize(PendingObject);
	stream.Serialize(PendingHouse);
	stream.Serialize(IsRepairMode);
	stream.Serialize(IsSellMode);
	stream.Serialize(IsPowerMode);
	stream.Serialize(IsWaypointMode);
	stream.Serialize(IsTargettingMode);
	// DraggedWaypoint -- a waypoint drag cannot outlive the button that started it, and the load
	// abandons it outright.
	// DraggedWaypointCoord
	// WaypointColor
	// IsRubberBand -- likewise the rubber band selection, which no held button survives to
	// continue.
	// IsTentative
	// BandX
	// BandY
	// NewX
	// NewY
	// TacButton -- the tactical input gadget is reattached by Init_IO.
	// IsShadowPresent -- set afresh by each icon draw.
	// ShadowShapes -- artwork fetched by One_Time.
	// PlacementShapes
}


/// <summary>
/// Performs the player's order upon the currently selected objects.
/// This routine is called when the player clicks on the tactical map while something is
/// selected. Every selected object is told to act upon the target. A group ordered to move
/// is spread out around the destination so that the units do not all pile onto one cell.
/// </summary>
/// <param name="object">The object clicked upon, or NULL if the click landed on the ground.</param>
/// <param name="cell">The cell that was clicked upon.</param>
/// <param name="action">The action that the player intends to perform.</param>
void DisplayClass::Active_Click(ObjectClass * object, Cell cell, ActionType action)
{
	int index;

	PathType waypoint_path = PATH_NONE;
	char waypoint_id = 0;

	if (Debug_Map) {
		return;
	}

	cell = Map.Clip_To_Map(cell);
	WaypointClass * waypoint = PlayerPtr->Waypoint_At(cell);
	if (waypoint != NULL) {
		PlayerPtr->Fetch_Waypoint_Data(waypoint, waypoint_path, waypoint_id);
	}

	for (index = 0; index < CurrentObject.Count(); index++) {
		CurrentObject[index]->Set_Waypoint_Path(waypoint_path, waypoint_id);
		if (CurrentObject[index]->Is_Foot()) {
			((TechnoClass *)CurrentObject[index])->IsOnPatrol = false;
		}
	}

	if (object != NULL) {
		for (index = 0; index < CurrentObject.Count(); index++) {
			ObjectClass * tobject = CurrentObject[index];
			tobject->Active_Click_With(tobject->What_Action(object), object, false);
			AllowVoice = false;
		}
	} else if (action == ACTION_MOVE || action == ACTION_PATROL_WAYPOINT) {
		DynamicVectorClass<ObjectClass *> movers;

		for (index = 0; index < CurrentObject.Count(); index++) {
			if (CurrentObject[index]->Is_Foot()) {
				FootClass *foot = (FootClass *)CurrentObject[index];
				if (foot->CurrentTube >= 0) {
					foot->Active_Click_With(foot->What_Action(cell), cell, false);
					AllowVoice = false;
				} else {
					movers.Add(CurrentObject[index]);
				}
			} else {
				ObjectClass * tobject = CurrentObject[index];
				tobject->Active_Click_With(tobject->What_Action(cell), cell, false);
				AllowVoice = false;
			}
		}

		if (movers.Count() > 0) {
			Coord center = Vector_Center(movers);
			ObjectClass * anchor = Vector_Closest_Object(movers, center);

			IndexClass<int, ObjectClass *> distance_sorted;

			for (index = 0; index < movers.Count(); index++) {
				distance_sorted.Add_Index(index + 1000 * Distance(movers[index]->Center_Coord(), anchor->PositionCoord), movers[index]);
				if (action == ACTION_PATROL_WAYPOINT) {
					((TechnoClass *)movers[index])->IsOnWaypointPatrol = true;
				}
			}

			movers.Clear();

			DynamicVectorClass<Cell> command_cells;
			DynamicVectorClass<bool> command_issued;

			for (index = 0; index < distance_sorted.Count(); index++) {
				movers.Add(distance_sorted.Fetch_By_Position(index));
				command_cells.Add(cell);
				command_issued.Add(false);
			}

			Cell closest_cell = anchor->PositionCell;
			int zone;
			int diff1, diff2;

			for (index = 0; index < movers.Count(); index++) {
				FootClass * mover = (FootClass *)movers[index];
				Cell assigned_cell = cell;

				if (index == 0) {
					movers[index]->Active_Click_With(movers[index]->What_Action(cell), cell, false);
					Map[cell].IsToGrowVeins = true;
					command_issued[index] = true;
					AllowVoice = false;

				} else if (!command_issued[index]) {

					Coord const & dest = mover->Destination_Coord();
					Cell destcell(Lepton_To_Cell(dest.X), Lepton_To_Cell(dest.Y));

					int height = Map[cell].Height + BRIDGE_CELL_HEIGHT * Map[cell].IsUnderBridge;

					Vector3 pos(cell.X + 0.5, cell.Y + 0.5, 0);
					Vector3 dir(destcell.X - closest_cell.X, destcell.Y - closest_cell.Y, 0);
					float len = dir.Length();
					dir = (len != 0.0) ? dir / len : dir;

					int tries = 0;
					bool blocked = false;

					const TechnoTypeClass *ttype = mover->TClass;
					zone = Map.Get_Cell_Zone(cell, ttype->MZone, true);

					Cell trycell;
					Cell fallback_cell = cell;
					Cell * chosen;

					for (; tries < 6; tries++) {
						pos = pos + dir;
						trycell = Cell((int)pos.X, (int)pos.Y);
						CellClass * cptr = &Map[trycell];

						if (!In_Local_Radar(trycell, true)) {
							chosen = &fallback_cell;
							break;
						}

						int tryzone = Map.Get_Cell_Zone(trycell, ttype->MZone, cptr->IsUnderBridge);
						MoveType move = mover->Can_Enter_Cell(cptr, FACING_NONE, height);

						if (tryzone != zone) {
							blocked = true;
						} else if (cptr->IsToGrowVeins) {
							fallback_cell = trycell;
						} else {
							if (cptr->IsUnderBridge) {
								if (abs(height - cptr->Height - BRIDGE_CELL_HEIGHT) > 2) {
									chosen = &fallback_cell;
									break;
								}
							}
							if (!cptr->IsUnderBridge) {
								if (abs(height - cptr->Height) > 2) {
									chosen = &fallback_cell;
									break;
								}
							}
							if (move < MOVE_FRIENDLY_DESTROYABLE || move == MOVE_TEMP) {
								if (blocked) {
									diff1 = abs(trycell.X - cell.X);
									diff2 = abs(trycell.Y - cell.Y);
									if (diff1 > diff2) diff2 = diff1;
									zone = diff2 + 3;
									Search.Test_Cell_Walk(trycell, cell, mover, Map[trycell].IsUnderBridge, Map[cell].IsUnderBridge, MZONE_NONE);
									chosen = &fallback_cell;
								} else {
									trycell = Cell((int)pos.X, (int)pos.Y);
									chosen = &trycell;
								}
								break;
							}
							blocked = true;
						}
					}

					if (tries >= 6) {
						chosen = &fallback_cell;
					}

					assigned_cell = *chosen;
					command_cells[index] = assigned_cell;
					FootClass * foot = dynamic_cast<FootClass *>(mover);
					if (foot != NULL) {
						foot->WaypointOffsetCell = assigned_cell - cell;
					}
					mover->Active_Click_With(mover->What_Action(assigned_cell), assigned_cell, false);
					Map[assigned_cell].IsToGrowVeins = true;
					command_issued[index] = true;
				}

				for (int scan = index + 1; scan < movers.Count(); scan++) {
					if (!command_issued[scan]) {
						if (mover->PositionCell == movers[scan]->PositionCell) {
							movers[scan]->Active_Click_With(movers[scan]->What_Action(assigned_cell), assigned_cell, false);
							command_issued[scan] = true;
						}
					}
				}
			}

			for (index = 0; index < movers.Count(); index++) {
				((TechnoClass *)movers[index])->IsOnWaypointPatrol = false;
			}

			for (index = 0; index < command_cells.Count(); index++) {
				Map[command_cells[index]].IsToGrowVeins = false;
			}
		}
	} else {
		for (index = 0; index < CurrentObject.Count(); index++) {
			ObjectClass * tobject = CurrentObject[index];
			tobject->Active_Click_With(tobject->What_Action(cell), cell, false);
			AllowVoice = false;
		}
	}
	AllowVoice = true;
}


/// <summary>
/// Converts an action into its waypoint equivalent.
/// This routine is used when the mouse is over a cell that holds one of the player's
/// waypoints. The order that would normally be given is translated into the matching
/// waypoint order, so that the whole path is affected rather than just the one cell.
/// </summary>
/// <param name="action">The action that would apply if there were no waypoint here.</param>
/// <param name="cell">The cell that the action would be performed upon.</param>
/// <returns>Returns with the action to use. If there is no waypoint at the cell, then the
/// original action is returned unchanged.</returns>
ActionType DisplayClass::Action_To_Waypoint_Action(ActionType action, Cell const & cell)
{
	if (PlayerPtr->Waypoint_At(cell) != NULL) {
		switch (action) {
			case ACTION_MOVE:
			case ACTION_NOMOVE:
				return(ACTION_FOLLOW_WAYPOINT);
			case ACTION_ATTACK:
			case ACTION_HARVEST:
				return(ACTION_ATTACK_WAYPOINT);
			case ACTION_ENTER:
			case ACTION_CAPTURE:
				return(ACTION_ENTER_WAYPOINT);
			default:
				return(action);
		}
	}
	return(action);
}


/// <summary>
/// Sets the mouse cursor colors to match a waypoint path.
/// This routine is used by the waypoint mode so that the cursor is drawn in the same color
/// as the path the player is currently laying down.
/// </summary>
/// <param name="index">The waypoint path to take the color from, or -1 for the plain
/// cursor color.</param>
void DisplayClass::Update_Waypoint_Color(int index)
{
	int offset;
	if (index == -1) {
		offset = 0;
	} else {
		offset = 8 * (index % 12);
	}
	unsigned short * table = (unsigned short *)MouseDrawer->Get_Translate_Table();
	for (int i = 0; i < 8; i++) {
		const RGBClass rgb = WaypointPalette[i + offset];
		table[i + 1] = DSurface::Build_Hicolor_Pixel(rgb.Get_Red(), rgb.Get_Green(), rgb.Get_Blue());
	}
}


/// <summary>
/// Cancels any rubber band selection that is in progress.
/// This routine is called when the game must drop out of drag select without the player
/// completing it -- a scenario ending, a trigger taking control, or the mouse wandering off
/// of the tactical map. The derived map layers override it to abandon their own state too.
/// </summary>
void DisplayClass::Abort_Drag_Select(void)
{
	if (TacticalMap != NULL) {
		TacticalMap->End_Rubber_Band();
	}

	IsRubberBand = false;
	IsTentative = false;
	drag_select_aborted = true;
	Set_Default_Mouse(MOUSE_NORMAL, false);
	DebugString("Display: Abort_Drag_Select()\n");
}


/// <summary>
/// Fetches the object that the tactical view is following.
/// </summary>
/// <returns>Returns with a pointer to the object being followed. If follow mode is not
/// active, then NULL is returned.</returns>
ObjectClass * DisplayClass::Object_To_Follow(void) const
{
	if (FollowingObject && FollowingObjectPtr != NULL) {
		return(FollowingObjectPtr);
	}
	return(NULL);
}


/// <summary>
/// Sets the object that the tactical view is to follow.
/// This routine is used by the follow mode. Handing it NULL is how the caller turns
/// following back off.
/// </summary>
/// <param name="object">The object to follow, or NULL to follow nothing at all.</param>
void DisplayClass::Set_To_Follow(ObjectClass * object)
{
	FollowingObjectPtr = object;
	FollowingObject = object != NULL ? true : false;
}
