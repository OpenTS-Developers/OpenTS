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

/* $Header: /CounterStrike/MOUSE.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : MOUSE.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 12/15/94                                                     *
 *                                                                                             *
 *                  Last Update : September 21, 1995 [JLB]                                     *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   MouseClass::AI -- Process player input as it relates to the mouse                         *
 *   MouseClass::Init_Clear -- Sets the mouse system to a known state                          *
 *   MouseClass::MouseClass -- Default constructor for the mouse handler class.                *
 *   MouseClass::Mouse_Small -- Controls the sizing of the mouse.                              *
 *   MouseClass::One_Time -- Performs the one time initialization of the mouse system.         *
 *   MouseClass::Override_Mouse_Shape -- Alters the shape of the mouse.                        *
 *   MouseClass::Revert_Mouse_Shape -- Reverts the mouse shape to the non overridden shape.    *
 *   MouseClass::Set_Default_Mouse -- Sets the mouse to match the shape specified.             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "mouse.h"

#include "_mixfile.h"
#include "animtype.h"
#include "builtype.h"
#include "cell.h"
#include "data.h"
#include "hashtable.h"
#include "isotype.h"
#include "mixfile.h"
#include "overtype.h"
#include "rawfile.h"
#include "saveload.h"
#include "savestream.h"
#include "scenario.h"
#include "shapeset.h"
#include "smudtype.h"
#include "terrtype.h"
#include "xmouse.h"


#define MOUSE_HOTSPOT_MIN 0
#define MOUSE_HOTSPOT_CENTER 12345
#define MOUSE_HOTSPOT_MAX 54321


/*
**	This points to the loaded mouse shapes.
*/
ShapeSet const * MouseClass::MouseShapes;

/*
**	This is the timer that controls the mouse animation. It is always at a fixed
**	rate so it uses the constant system timer.
*/
CDTimerClass<SystemTimerClass> MouseClass::Timer = 0;


/***********************************************************************************************
 * MouseClass::MouseClass -- Default constructor for the mouse handler class.                  *
 *                                                                                             *
 *    This is the default constructor for the mouse handling class. It merely sets up the      *
 *    mouse system to its default state.                                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
MouseClass::MouseClass(void) :
	IsSmall(false),
	CurrentMouseShape(MOUSE_NORMAL),
	NormalMouseShape(MOUSE_NORMAL),
	Frame(0)
{
}


/***********************************************************************************************
 * MouseClass::Set_Default_Mouse -- Sets the mouse to match the shape specified.               *
 *                                                                                             *
 *    This routine is used to inform the display system as to which mouse shape is desired.    *
 *                                                                                             *
 * INPUT:   mouse -- The mouse shape number to set the mouse to.                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void MouseClass::Set_Default_Mouse(MouseType mouse, bool size)
{
	assert((unsigned)mouse < MOUSE_COUNT);

	NormalMouseShape = mouse;
	Override_Mouse_Shape(mouse, size);
}


/***********************************************************************************************
 * MouseClass::Revert_Mouse_Shape -- Reverts the mouse shape to the non overridden shape.      *
 *                                                                                             *
 *    Use this routine to cancel the effects of Override_Mouse_Shape(). It will revert the     *
 *    mouse back to the original shape.                                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/27/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void MouseClass::Revert_Mouse_Shape(void)
{
	Override_Mouse_Shape(NormalMouseShape, false);
}


/***********************************************************************************************
 * MouseClass::Mouse_Small -- Controls the sizing of the mouse.                                *
 *                                                                                             *
 *    This routine is called to change the mouse sizing override. If the mouse can change      *
 *    size to that specified, then the mouse imagery will be changed. If a change of imagery   *
 *    cannot occur (due to lack of appropriate artwork), then no action will be performed.     *
 *                                                                                             *
 * INPUT:   small -- Should the mouse be made small? If not, then it will be made large.       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void MouseClass::Mouse_Small(bool wsmall)
{
	MouseStruct const * control = &MouseControl[CurrentMouseShape];

	if (IsSmall == wsmall) {
		return;
	}

	IsSmall	= wsmall;

	int frame = Get_Mouse_Current_Frame(CurrentMouseShape, wsmall);
	Point2D hotspot = Get_Mouse_Hotspot(CurrentMouseShape);

	MouseCursor->Set_Cursor(hotspot, MouseShapes, frame);
}


/// <summary>
/// Fetches the shape frame to display for the mouse specified.
/// This routine takes the current animation stage into account, and will use the small
/// version of the cursor when small cursors are asked for and the shape has one.
/// </summary>
/// <param name="wsmall">Should the small version of the cursor be used?</param>
/// <returns>Returns with the shape frame number to display.</returns>
int MouseClass::Get_Mouse_Current_Frame(MouseType mouse, bool wsmall) const
{
	MouseStruct const * control = &MouseControl[mouse];

	if (wsmall) {
		if (control->SmallFrame != -1) {
			return(control->SmallFrame + Frame);
		}
	}

	return(control->StartFrame + Frame);
}


/// <summary>
/// Fetches the hotspot of the mouse shape specified.
/// This routine translates the symbolic hotspot values held in the mouse control table
/// into a real pixel offset within the cursor shape. It is used whenever the cursor
/// image changes, so that the mouse driver knows which pixel does the pointing.
/// </summary>
/// <returns>Returns with the hotspot offset from the upper left corner of the shape.</returns>
Point2D MouseClass::Get_Mouse_Hotspot(MouseType mouse) const
{
	Point2D hotspot(0,0);

	if (MouseShapes == NULL) {
		return(hotspot);
	}

	MouseStruct const * control = &MouseControl[mouse];

	if (control->X == MOUSE_HOTSPOT_CENTER) {
		hotspot.X = MouseShapes->Get_Width() / 2;
	}
	if (control->X == MOUSE_HOTSPOT_MAX) {
		hotspot.X = MouseShapes->Get_Width();
	}

	if (control->Y == MOUSE_HOTSPOT_CENTER) {
		hotspot.Y = MouseShapes->Get_Height() / 2;
	}
	if (control->Y == MOUSE_HOTSPOT_MAX) {
		hotspot.Y = MouseShapes->Get_Height();
	}

	return(hotspot);
}


/***********************************************************************************************
 * MouseClass::Override_Mouse_Shape -- Alters the shape of the mouse.                          *
 *                                                                                             *
 *    This routine is used to alter the shape of the mouse as needed.                          *
 *    Typical mouse shape change occurs when scrolling the map or                              *
 *    selecting targets.                                                                       *
 *                                                                                             *
 * INPUT:   mouse -- The mouse shape number to use.                                            *
 *                                                                                             *
 * OUTPUT:  bool; Was the mouse shape changed?                                                 *
 *                                                                                             *
 * WARNINGS:   This is not intended to be used as a means to hide the                          *
 *             mouse. Nor will it work correctly if the mouse shape                            *
 *             file hasn't been loaded.                                                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1994 JLB : Created.                                                                 *
 *   06/03/1994 JLB : Made into member function.                                               *
 *   12/24/1994 JLB : Added small control parameter.                                           *
 *=============================================================================================*/
bool MouseClass::Override_Mouse_Shape(MouseType mouse, bool wsmall)
{
	assert((unsigned)mouse < MOUSE_COUNT);

	MouseStruct const * control = &MouseControl[mouse];
	static bool startup = false;

	/*
	**	Only certain mouse shapes have a small counterpart. If the requested mouse
	**	shape is not one of these, then force the small size override flag to false.
	*/
	if (control->SmallFrame == -1) {
		wsmall = false;
	}

	/*
	**	If the mouse shape is going to change, then inform the mouse driver of the
	**	change.
	*/
	if (!startup || (MouseShapes && ((mouse != CurrentMouseShape) || (wsmall != IsSmall)))) {
		startup = true;

		Timer = control->FrameRate;
		Frame = 0;

		MouseCursor->Set_Cursor(Get_Mouse_Hotspot(mouse), MouseShapes, Get_Mouse_Current_Frame(mouse, wsmall));
		CurrentMouseShape = mouse;
		IsSmall = wsmall;
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * MouseClass::AI -- Process player input as it relates to the mouse                           *
 *                                                                                             *
 *    This routine will is to be called once per game tick and is passed the player keyboard   *
 *    or mouse input code. It processes this code and updates the mouse shape as appropriate.  *
 *                                                                                             *
 * INPUT:   input -- The player input code as returned from Keyboard->Get().                   *
 *                                                                                             *
 *          x,y   -- The mouse coordinate values to use.                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *   12/31/1994 JLB : Uses mouse coordinate parameters.                                        *
 *   03/27/1995 JLB : New animation control.                                                   *
 *   05/28/1995 JLB : Moderates animation so is more steady regardless of speed.               *
 *   06/30/1995 JLB : Uses constant timer system.                                              *
 *=============================================================================================*/
void MouseClass::AI(KeyNumType &input, Point2D const & xy)
{
	MouseStruct const * control = &MouseControl[CurrentMouseShape];

	if (control->FrameRate && Timer == 0) {

		Frame++;
		Frame %= control->FrameCount;
		Timer = control->FrameRate;

		MouseCursor->Set_Cursor(Get_Mouse_Hotspot(CurrentMouseShape), MouseShapes, Get_Mouse_Current_Frame(CurrentMouseShape, IsSmall));
	}

	BASECLASS::AI(input, xy);
}


/***********************************************************************************************
 * MouseClass::One_Time -- Performs the one time initialization of the mouse system.           *
 *                                                                                             *
 *    Use this routine to load the mouse data file and perform any other necessary one time    *
 *    preparations for the game.                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only call this routine ONCE.                                                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void MouseClass::One_Time(void)
{
	BASECLASS::One_Time();

	/*
	**	Override the mouse shape file with the one in the current directory, but only if there
	**	is an override file available.
	*/
	#ifdef _DEBUG
		RawFileClass file("MOUSE.SHP");
		if (file.Is_Available()) {
			MouseShapes = (ShapeSet const *)Load_Alloc_Data(file);
		} else {
			MouseShapes = (ShapeSet const *)MFCD::Retrieve("MOUSE.SHP");
		}
	#else
		MouseShapes = (ShapeSet const *)MFCD::Retrieve("MOUSE.SHP");
	#endif
}


/***********************************************************************************************
 * MouseClass::Init_Clear -- Sets the mouse system to a known state                            *
 *                                                                                             *
 *    This routine will reset the mouse handling system. Typically, this routine is called     *
 *    when preparing for the beginning of a new scenario.                                      *
 *                                                                                             *
 * INPUT:   theater  -- The theater that the scenario will take place.                         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void MouseClass::Init_Clear(void)
{
	BASECLASS::Init_Clear();
	IsSmall = false;
	NormalMouseShape = MOUSE_NORMAL;
}


/// <summary>
/// Loads the map layer from a save game stream.
/// This routine tears down the cell array and the zone tables, reads the members of the
/// whole display chain back, then reallocates the map and reads the cells and zone data
/// back into it. Object pointers within the restored state are remapped by the swizzle
/// manager, and the theater specific type data is reinitialized to match the scenario.
/// </summary>
/// <returns>Returns with S_OK if the map was loaded, otherwise the stream error.</returns>
HRESULT MouseClass::Load(SaveStreamClass & stream)
{
	int i;

	HRESULT result = BASECLASS::Load(stream);
	if (SUCCEEDED(result)) {
		int theater;
		stream.Serialize(theater);
		result = stream.Result();
		if (FAILED(result)) {
			return(result);
		}

		LastTheater = THEATER_NONE;

		/*
		**	Free the cell array, because we're about to overwrite its pointers
		*/
		Free_Cells();

		delete CellSubzones;
		CellSubzones = NULL;
		delete CellZones;
		CellZones = NULL;
		delete ZoneAdjacency;
		ZoneAdjacency = NULL;

		for (i = 0; i < SUBZONE_COUNT; i++) {
			SubzoneTracking[i].Clear();
		}

		for (i = 0; i < MZONE_COUNT; i++) {
			delete Zones[i];
			Zones[i] = NULL;
		}

		for (i = 0; i < SUBZONE_COUNT; i++) {
			delete SubzoneConnectionHashTable[i];
			SubzoneConnectionHashTable[i] = NULL;
		}

		Array.Clear();

		stream.Set_Context("MouseClass");
		Serialize(stream);
		result = stream.Result();
		if (FAILED(result)) {
			return(result);
		}

		/*
		**	Reallocate the cell array
		*/
		Alloc_Cells();

		/*
		**	Init all cells to empty
		*/
		Init_Cells();

		CellSubzones = NULL;
		CellZones = NULL;

		Set_Map_Dimensions(PlayRect, 1, 0, false);

		if (CellSubzones) {
			delete CellSubzones;
			CellSubzones = NULL;
		}
		if (CellZones) {
			delete CellZones;
			CellZones = NULL;
		}

		CellSubzones = new CellSubzoneStruct[CellZoneCount];
		CellZones = new CellZoneStruct[CellZoneCount];
		ZoneAdjacency = new ZONE_PAIR_HASH_SET(20, 256, SubzoneHash);

		for (i = 0; i < SUBZONE_COUNT; i++) {
			int v = (1 << (i + 1));
			SubzoneTracking[i].Clear();
			SubzoneTracking[i].Set_Growth_Step((4 * PlayRect.Width * PlayRect.Height) / (v * v));
			SubzoneConnectionHashTable[i] = new SUBZONE_CONNECTION_HASH_SET(20, 256, SubzoneHash);
		}

		stream.Serialize_Bytes(CellZones, (int)(sizeof(*CellZones) * CellZoneCount));
		result = stream.Result();
		if (FAILED(result)) {
			return(result);
		}

		for (i = 0; i < MZONE_COUNT; i++) {
			Zones[i] = new unsigned short[ZoneCount];
			stream.Serialize_Bytes(Zones[i], (int)(sizeof(unsigned short) * ZoneCount));
			result = stream.Result();
			if (FAILED(result)) {
				return(result);
			}
		}

		stream.Serialize(ZoneConnections);
		result = stream.Result();
		if (FAILED(result)) {
			return(result);
		}

		for (i = 0; i < Array.Length(); i++) {
			delete Array[i];
			Array[i] = NULL;
		}
		int count;
		stream.Serialize(count);
		result = stream.Result();
		if (FAILED(result)) {
			return(result);
		}
		for (i = 0; i < count; i++) {
			if (Load_Object(stream) == NULL) {
				return(stream.Result());
			}
		}

		TerrainTypeClass::Init(Scen->Theater);
		if (Scen->Theater != LastTheater) {
			IsometricTileTypeClass::Read_Control_File(Scen->Theater, true);
		} else {
			IsometricTileTypeClass::Clear_Use_Counts();
		}
		IsometricTileTypeClass::Load_Tiles(false, false);
		OverlayTypeClass::Init(Scen->Theater);
		BuildingTypeClass::Init(Scen->Theater);
		AnimTypeClass::Init(Scen->Theater);
		SmudgeTypeClass::Init(Scen->Theater);
		DraggedWaypoint = NULL;
		LastTheater = Scen->Theater;

		result = S_OK;
	}
	return(result);
}


/// <summary>
/// Saves the map layer to a save game stream.
/// This routine writes the theater, the members of the whole display chain, the zone tables
/// and zone connections, and then every valid cell, in the order that Load expects to find
/// them. Each cell writes its own contents as a record of its own.
/// </summary>
/// <returns>Returns with S_OK if the map was written, otherwise the stream error.</returns>
HRESULT MouseClass::Save(SaveStreamClass & stream)
{
	int i;
	int count;

	HRESULT result = BASECLASS::Save(stream);
	if (SUCCEEDED(result)) {
		int theater = Scen->Theater;
		stream.Serialize(theater);
		result = stream.Result();
		if (FAILED(result)) {
			return(result);
		}

		Serialize(stream);
		result = stream.Result();
		if (FAILED(result)) {
			return(result);
		}

		stream.Serialize_Bytes(CellZones, (int)(sizeof(*CellZones) * CellZoneCount));
		result = stream.Result();
		if (FAILED(result)) {
			return(result);
		}

		for (i = 0; i < MZONE_COUNT; i++) {
			stream.Serialize_Bytes(Zones[i], (int)(sizeof(unsigned short) * ZoneCount));
			result = stream.Result();
			if (FAILED(result)) {
				return(result);
			}
		}

		stream.Serialize(ZoneConnections);
		result = stream.Result();
		if (FAILED(result)) {
			return(result);
		}

		count = 0;
		Reset_Iterator();
		CellClass *cptr = Iterate();
		while (cptr != NULL) {
			Cell cell = cptr->CellID;
			if (Is_Valid(cell)) {
				count++;
			}
			cptr = Iterate();
		}
		stream.Serialize(count);
		result = stream.Result();
		if (FAILED(result)) {
			return(result);
		}
		Reset_Iterator();
		cptr = Iterate();
		while (cptr != NULL) {
			Cell cell = cptr->CellID;
			if (Is_Valid(cell)) {
				Save_Object(stream, cptr);
				count--;
			}
			cptr = Iterate();
		}
		if (count != 0) {
			return(result);
		}

		result = S_OK;
	}
	return(result);
}


/// <summary>
/// Lists the members the mouse handler holds.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void MouseClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);
	// MouseShapes -- the cursor artwork and the table that drives it, both established by
	// One_Time.
	// MouseControl
	// IsSmall -- the cursor the player is looking at, which the input pass chooses again from
	// whatever lies beneath it.
	// CurrentMouseShape
	// NormalMouseShape
	// Timer
	// Frame
}


/// <summary>
/// Fetches the first shape frame used by a mouse cursor.
/// This routine is used to locate a mouse shape's animation within the combined mouse
/// shape file.
/// </summary>
/// <returns>Returns with the starting frame number of the mouse shape specified.</returns>
int MouseClass::Get_Mouse_Start_Frame(MouseType mouse) const
{
	return(MouseControl[mouse].StartFrame);
}


/// <summary>
/// Fetches the number of animation frames for a mouse shape.
/// </summary>
/// <returns>Returns with the frame count of the mouse shape specified.</returns>
int MouseClass::Get_Mouse_Frame_Count(MouseType mouse) const
{
	return(MouseControl[mouse].FrameCount);
}


/*
**	This array of structures is used to control the mouse animation
**	sequences.
*/
#define	HSMIN	MOUSE_HOTSPOT_MIN
#define	HSMAX	MOUSE_HOTSPOT_MAX
#define	HSCNR	MOUSE_HOTSPOT_CENTER

MouseClass::MouseStruct MouseClass::MouseControl[MOUSE_COUNT] = {
	{0,		1,		0,		1,		HSMIN,	HSMIN},	// MOUSE_NORMAL

	{2,		1,		0,		-1,		HSCNR,	HSMIN},	// MOUSE_N
	{3,		1,		0,		-1,		HSMAX,	HSMIN},	// MOUSE_NE
	{4,		1,		0,		-1,		HSMAX,	HSCNR},	// MOUSE_E
	{5,		1,		0,		-1,		HSMAX,	HSMAX},	// MOUSE_SE
	{6,		1,		0,		-1,		HSCNR,	HSMAX},	// MOUSE_S
	{7,		1,		0,		-1,		HSMIN,	HSMAX},	// MOUSE_SW
	{8,		1,		0,		-1,		HSMIN,	HSCNR},	// MOUSE_W
	{9,		1,		0,		-1,		HSMIN,	HSMIN},	// MOUSE_NW

	{10,	1,		0,		-1,		HSCNR,	HSMIN},	// MOUSE_NO_N
	{11,	1,		0,		-1,		HSMAX,	HSMIN},	// MOUSE_NO_NE
	{12,	1,		0,		-1,		HSMAX,	HSCNR},	// MOUSE_NO_E
	{13,	1,		0,		-1,		HSMAX,	HSMAX},	// MOUSE_NO_SE
	{14,	1,		0,		-1,		HSCNR,	HSMAX},	// MOUSE_NO_S
	{15,	1,		0,		-1,		HSMIN,	HSMAX},	// MOUSE_NO_SW
	{16,	1,		0,		-1,		HSMIN,	HSCNR},	// MOUSE_NO_W
	{17,	1,		0,		-1,		HSMIN,	HSMIN},	// MOUSE_NO_NW

	{18,	13,		4,		-1,		HSCNR,	HSCNR},	// MOUSE_CAN_SELECT
	{31,	10,		4,		42,		HSCNR,	HSCNR},	// MOUSE_CAN_MOVE
	{41,	1,		0,		52,		HSCNR,	HSCNR},	// MOUSE_NO_MOVE
	{53,	5,		4,		63,		HSCNR,	HSCNR},	// MOUSE_STAY_ATTACK
	{58,	5,		4,		63,		HSCNR,	HSCNR},	// MOUSE_CAN_ATTACK
	{68,	5,		4,		73,		HSCNR,	HSCNR},	// MOUSE_AREA_GUARD
	{78,	10,		4,		-1,		HSCNR,	HSCNR},	/// MOUSE_TOTE
	{88,	1,		0,		-1,		HSCNR,	HSCNR},	/// MOUSE_NO_TOTE
	{89,	10,		4,		100,	HSCNR,	HSCNR},	// MOUSE_ENTER
	{99,	1,		0,		63,		HSCNR,	HSCNR},	// MOUSE_NO_ENTER
	{110,	9,		4,		-1,		HSCNR,	HSCNR},	// MOUSE_DEPLOY
	{119,	1,		0,		-1,		HSCNR,	HSCNR},	// MOUSE_NO_DEPLOY
	{120,	9,		4,		-1,		HSCNR,	HSCNR},	/// MOUSE_UNDEPLOY
	{129,	10,		4,		-1,		HSCNR,	HSCNR},	// MOUSE_SELL_BACK
	{139,	10,		4,		-1,		HSCNR,	HSCNR},	// MOUSE_SELL_UNIT
	{149,	1,		0,		-1,		HSCNR,	HSCNR},	// MOUSE_NO_SELL_BACK
	{150,	20,		4,		-1,		HSCNR,	HSCNR},	// MOUSE_GREPAIR
	{170,	20,		4,		-1,		HSCNR,	HSCNR},	// MOUSE_REPAIR
	{190,	1,		0,		-1,		HSCNR,	HSCNR},	// MOUSE_NO_REPAIR

	{191,	10,		4,		-1,		HSCNR,	HSCNR},	/// MOUSE_WAYPOINT
	{201,	10,		4,		-1,		HSCNR,	HSCNR},	/// MOUSE_PLACE_WAYPOINT
	{211,	1,		0,		-1,		HSCNR,	HSCNR},	/// MOUSE_NO_PLACE_WAYPOINT
	{212,	7,		4,		-1,		HSCNR,	HSCNR},	/// MOUSE_SELECT_WAYPOINT
	{219,	10,		4,		-1,		HSCNR,	HSCNR},	/// MOUSE_ENTER_WAYPOINT_MODE
	{229,	10,		4,		-1,		HSCNR,	HSCNR},	/// MOUSE_FOLLOW_WAYPOINT
	{239,	10,		4,		-1,		HSCNR,	HSCNR},	/// MOUSE_WAYPOINT_TOTE
	{249,	10,		4,		-1,		HSCNR,	HSCNR},	/// MOUSE_WAYPOINT_REPAIR
	{259,	10,		4,		-1,		HSCNR,	HSCNR},	/// MOUSE_ATTACK_WAYPOINT
	{269,	10,		4,		-1,		HSCNR,	HSCNR},	/// MOUSE_ENTER_WAYPOINT
	{356,	1,		0,		-1,		HSCNR,	HSCNR},	/// MOUSE_LOOP_WAYPOINT_PATH

	{279,	20,		4,		-1,		HSCNR,	HSCNR},	// MOUSE_AIR_STRIKE
	{299,	10,		4,		-1,		HSCNR,	HSCNR},	/// MOUSE_CHEMBOMB
	{309,	10,		4,		-1,		HSCNR,	HSCNR},	// MOUSE_DEMOLITIONS
	{319,	10,		4,		-1,		HSCNR,	HSCNR},	// MOUSE_NUCLEAR_BOMB

	{329,	16,		2,		-1,		HSCNR,	HSCNR},	/// MOUSE_TOGGLE_POWER
	{345,	1,		0,		-1,		HSCNR,	HSCNR},	/// MOUSE_NO_TOGGLE_POWER

	{346,	10,		4,		42,		HSCNR,	HSCNR},	// MOUSE_HEAL

	{357,	20,		3,		-1,		HSCNR,	HSCNR},	/// MOUSE_EM_PULSE
	{377,	1,		0,		-1,		HSCNR,	HSCNR},	/// MOUSE_EM_PULSE_RANGE

	{378,	1,		0,		-1,		HSCNR,	HSCNR},	/// MOUSE_SCROLL_COASTING
	{379,	1,		0,		-1,		HSCNR,	HSCNR},	/// MOUSE_SCROLL_COASTING_N
	{380,	1,		0,		-1,		HSCNR,	HSCNR},	/// MOUSE_SCROLL_COASTING_NE
	{381,	1,		0,		-1,		HSCNR,	HSCNR},	/// MOUSE_SCROLL_COASTING_E
	{382,	1,		0,		-1,		HSCNR,	HSCNR},	/// MOUSE_SCROLL_COASTING_SE
	{383,	1,		0,		-1,		HSCNR,	HSCNR},	/// MOUSE_SCROLL_COASTING_S
	{384,	1,		0,		-1,		HSCNR,	HSCNR},	/// MOUSE_SCROLL_COASTING_SW
	{385,	1,		0,		-1,		HSCNR,	HSCNR},	/// MOUSE_SCROLL_COASTING_W
	{386,	1,		0,		-1,		HSCNR,	HSCNR},	/// MOUSE_SCROLL_COASTING_NW

	{387,	10,		4,		-1,		HSCNR,	HSCNR},	/// MOUSE_PATROL_WAYPOINT
};
