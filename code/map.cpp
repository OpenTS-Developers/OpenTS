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

/* $Header: /CounterStrike/MAP.CPP 3     3/14/97 5:15p Joe_b $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : MAP.CPP                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : September 10, 1993                                           *
 *                                                                                             *
 *                  Last Update : October 5, 1996 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   MapClass::Base_Region -- Finds the owner and base zone for specified cell.                *
 *   MapClass::Cell_Region -- Determines the region from a specified cell number.              *
 *   MapClass::Cell_Threat -- Gets a houses threat value for a cell                            *
 *   MapClass::Close_Object -- Finds a clickable close object to the specified coordinate.     *
 *   MapClass::Destroy_Bridge_At -- Destroyes the bridge at location specified.                *
 *   MapClass::Detach -- Remove specified object from map references.                          *
 *   MapClass::In_Radar -- Is specified cell in the radar map?                                 *
 *   MapClass::Init -- clears all cells                                                        *
 *   MapClass::Intact_Bridge_Count -- Determine the number of intact bridges.                  *
 *   MapClass::Logic -- Handles map related logic functions.                                   *
 *   MapClass::Nearby_Location -- Finds a generally clear location near a specified cell.      *
 *   MapClass::One_Time -- Performs special one time initializations for the map.              *
 *   MapClass::Overlap_Down -- computes & marks object's overlap cells                         *
 *   MapClass::Overlap_Up -- Computes & clears object's overlap cells                          *
 *   MapClass::Overpass -- Performs any final cleanup to a freshly constructed map.            *
 *   MapClass::Pick_Up -- Removes specified object from the map.                               *
 *   MapClass::Place_Down -- Places the specified object onto the map.                         *
 *   MapClass::Place_Random_Crate -- Places a crate at random location on map.                 *
 *   MapClass::Read_Binary -- Reads the binary data from the straw specified.                  *
 *   MapClass::Remove_Crate -- Remove a crate from the specified cell.                         *
 *   MapClass::Set_Map_Dimensions -- Initialize the map.                                       *
 *   MapClass::Sight_From -- Mark as visible the cells within a specified radius.              *
 *   MapClass::Validate -- validates every cell on the map                                     *
 *   MapClass::Write_Binary -- Pipes the map template data to the destination specified.       *
 *   MapClass::Zone_Reset -- Resets all zone numbers to match the map.                         *
 *   MapClass::Zone_Span -- Flood fills the specified zone from the cell origin.               *
 *   MapClass::Pick_Random_Location -- Picks a random location on the map.                     *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "map.h"

#include "_alpha.h"
#include "_map.h"
#include "_rect.h"
#include "_rtti.h"
#include "_rules.h"
#include "_tactica.h"
#include "_zbuffer.h"
#include "anim.h"
#include "astar.h"
#include "building.h"
#include "builtype.h"
#include "ccrand.h"
#include "cell.h"
#include "conquer.h"
#include "dbgprint.h"
#include "foot.h"
#include "globals.h"
#include "house.h"
#include "houstype.h"
#include "incdec.h"
#include "inline.h"
#include "isotype.h"
#include "lcwpipe.h"
#include "lcwstraw.h"
#include "lzopipe.h"
#include "lzostraw.h"
#include "mapgen.h"
#include "overlay.h"
#include "overtype.h"
#include "partsys.h"
#include "psystype.h"
#include "rules.h"
#include "savestream.h"
#include "smartdeform.h"
#include "tactical.h"
#include "tag.h"
#include "tiberium.h"
#include "tube.h"
#include "vein.h"
#include "zbuffer.h"

#include "overlay.hh"
#include "ramp.hh"

#include <algorithm>
#include <new>
#include <utility>

Cell const MapClass::RadiusOffset[] = {
	/* 0  */	Cell(0,0),
	/* 1  */	Cell(1,-1),Cell(0,-1),Cell(-1,-1),Cell(-1,0),Cell(1,0),Cell(-1,1),Cell(0,1),Cell(1,1),
	/* 2  */	Cell(-1,-2),Cell(0,-2),Cell(1,-2),Cell(-2,-1),Cell(2,-1),Cell(-2,0),Cell(2,0),Cell(-2,1),Cell(2,1),Cell(-1,2),Cell(0,2),Cell(1,2),
	/* 3  */	Cell(-1,-3),Cell(0,-3),Cell(1,-3),Cell(-2,-2),Cell(2,-2),Cell(-3,-1),Cell(3,-1),Cell(-3,0),Cell(3,0),Cell(-3,1),Cell(3,1),Cell(-2,2),Cell(2,2),Cell(-1,3),Cell(0,3),Cell(1,3),
	/* 4  */	Cell(-1,-4),Cell(0,-4),Cell(1,-4),Cell(-3,-3),Cell(-2,-3),Cell(2,-3),Cell(3,-3),Cell(-3,-2),Cell(3,-2),Cell(-4,-1),Cell(4,-1),Cell(-4,0),Cell(4,0),Cell(-4,1),Cell(4,1),Cell(-3,2),Cell(3,2),Cell(-3,3),Cell(-2,3),Cell(2,3),Cell(3,3),Cell(-1,4),Cell(0,4),Cell(1,4),
	/* 5  */	Cell(-1,-5),Cell(0,-5),Cell(1,-5),Cell(-3,-4),Cell(-2,-4),Cell(2,-4),Cell(3,-4),Cell(-4,-3),Cell(4,-3),Cell(-4,-2),Cell(4,-2),Cell(-5,-1),Cell(5,-1),Cell(-5,0),Cell(5,0),Cell(-5,1),Cell(5,1),Cell(-4,2),Cell(4,2),Cell(-4,3),Cell(4,3),Cell(-3,4),Cell(-2,4),Cell(2,4),Cell(3,4),Cell(-1,5),Cell(0,5),Cell(1,5),
	/* 6  */	Cell(-1,-6),Cell(0,-6),Cell(1,-6),Cell(-3,-5),Cell(-2,-5),Cell(2,-5),Cell(3,-5),Cell(-4,-4),Cell(4,-4),Cell(-5,-3),Cell(5,-3),Cell(-5,-2),Cell(5,-2),Cell(-6,-1),Cell(6,-1),Cell(-6,0),Cell(6,0),Cell(-6,1),Cell(6,1),Cell(-5,2),Cell(5,2),Cell(-5,3),Cell(5,3),Cell(-4,4),Cell(4,4),Cell(-3,5),Cell(-2,5),Cell(2,5),Cell(3,5),Cell(-1,6),Cell(0,6),Cell(1,6),
	/* 7  */	Cell(-1,-7),Cell(0,-7),Cell(1,-7),Cell(-3,-6),Cell(-2,-6),Cell(2,-6),Cell(3,-6),Cell(-5,-5),Cell(-4,-5),Cell(4,-5),Cell(5,-5),Cell(-5,-4),Cell(5,-4),Cell(-6,-3),Cell(6,-3),Cell(-6,-2),Cell(6,-2),Cell(-7,-1),Cell(7,-1),Cell(-7,0),Cell(7,0),Cell(-7,1),Cell(7,1),Cell(-6,2),Cell(6,2),Cell(-6,3),Cell(6,3),Cell(-5,4),Cell(5,4),Cell(-5,5),Cell(-4,5),Cell(4,5),Cell(5,5),Cell(-3,6),Cell(-2,6),Cell(2,6),Cell(3,6),Cell(-1,7),Cell(0,7),Cell(1,7),
	/* 8  */	Cell(-1,-8),Cell(0,-8),Cell(1,-8),Cell(-3,-7),Cell(-2,-7),Cell(2,-7),Cell(3,-7),Cell(-5,-6),Cell(-4,-6),Cell(4,-6),Cell(5,-6),Cell(-6,-5),Cell(6,-5),Cell(-6,-4),Cell(6,-4),Cell(-7,-3),Cell(7,-3),Cell(-7,-2),Cell(7,-2),Cell(-8,-1),Cell(8,-1),Cell(-8,0),Cell(8,0),Cell(-8,1),Cell(8,1),Cell(-7,2),Cell(7,2),Cell(-7,3),Cell(7,3),Cell(-6,4),Cell(6,4),Cell(-6,5),Cell(6,5),Cell(-5,6),Cell(-4,6),Cell(4,6),Cell(5,6),Cell(-3,7),Cell(-2,7),Cell(2,7),Cell(3,7),Cell(-1,8),Cell(0,8),Cell(1,8),
	/* 9  */	Cell(-1,-9),Cell(0,-9),Cell(1,-9),Cell(-3,-8),Cell(-2,-8),Cell(2,-8),Cell(3,-8),Cell(-5,-7),Cell(-4,-7),Cell(4,-7),Cell(5,-7),Cell(-6,-6),Cell(6,-6),Cell(-7,-5),Cell(7,-5),Cell(-7,-4),Cell(7,-4),Cell(-8,-3),Cell(8,-3),Cell(-8,-2),Cell(8,-2),Cell(-9,-1),Cell(9,-1),Cell(-9,0),Cell(9,0),Cell(-9,1),Cell(9,1),Cell(-8,2),Cell(8,2),Cell(-8,3),Cell(8,3),Cell(-7,4),Cell(7,4),Cell(-7,5),Cell(7,5),Cell(-6,6),Cell(6,6),Cell(-5,7),Cell(-4,7),Cell(4,7),Cell(5,7),Cell(-3,8),Cell(-2,8),Cell(2,8),Cell(3,8),Cell(-1,9),Cell(0,9),Cell(1,9),
	/* 10 */	Cell(-1,-10),Cell(0,-10),Cell(1,-10),Cell(-3,-9),Cell(-2,-9),Cell(2,-9),Cell(3,-9),Cell(-5,-8),Cell(-4,-8),Cell(4,-8),Cell(5,-8),Cell(-7,-7),Cell(-6,-7),Cell(6,-7),Cell(7,-7),Cell(-7,-6),Cell(7,-6),Cell(-8,-5),Cell(8,-5),Cell(-8,-4),Cell(8,-4),Cell(-9,-3),Cell(9,-3),Cell(-9,-2),Cell(9,-2),Cell(-10,-1),Cell(10,-1),Cell(-10,0),Cell(10,0),Cell(-10,1),Cell(10,1),Cell(-9,2),Cell(9,2),Cell(-9,3),Cell(9,3),Cell(-8,4),Cell(8,4),Cell(-8,5),Cell(8,5),Cell(-7,6),Cell(7,6),Cell(-7,7),Cell(-6,7),Cell(6,7),Cell(7,7),Cell(-5,8),Cell(-4,8),Cell(4,8),Cell(5,8),Cell(-3,9),Cell(-2,9),Cell(2,9),Cell(3,9),Cell(-1,10),Cell(0,10),Cell(1,10)
};

Cell const MapClass::OcclusionOffset[] = {
	/* 0  */	Cell(0,0),
	/* 1  */	Cell(-1,1),Cell(0,1),Cell(1,1),Cell(1,0),Cell(-1,0),Cell(1,-1),Cell(0,-1),Cell(-1,-1),
	/* 2  */	Cell(1,1),Cell(0,1),Cell(-1,1),Cell(1,1),Cell(-1,1),Cell(1,0),Cell(-1,0),Cell(1,-1),Cell(-1,-1),Cell(1,-1),Cell(0,-1),Cell(-1,-1),
	/* 3  */	Cell(0,1),Cell(0,1),Cell(0,1),Cell(1,1),Cell(-1,1),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,-1),Cell(-1,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),
	/* 4  */	Cell(0,1),Cell(0,1),Cell(0,1),Cell(1,1),Cell(1,1),Cell(-1,1),Cell(-1,1),Cell(1,1),Cell(-1,1),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,-1),Cell(-1,-1),Cell(1,-1),Cell(1,-1),Cell(-1,-1),Cell(-1,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),
	/* 5  */	Cell(0,1),Cell(0,1),Cell(0,1),Cell(1,1),Cell(1,1),Cell(-1,1),Cell(-1,1),Cell(1,1),Cell(-1,1),Cell(1,1),Cell(-1,1),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,-1),Cell(-1,-1),Cell(1,-1),Cell(-1,-1),Cell(1,-1),Cell(1,-1),Cell(-1,-1),Cell(-1,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),
	/* 6  */	Cell(0,1),Cell(0,1),Cell(0,1),Cell(1,1),Cell(0,1),Cell(0,1),Cell(-1,1),Cell(1,1),Cell(-1,1),Cell(1,1),Cell(-1,1),Cell(1,0),Cell(1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,-1),Cell(-1,-1),Cell(1,-1),Cell(-1,-1),Cell(1,-1),Cell(0,-1),Cell(0,-1),Cell(-1,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),
	/* 7  */	Cell(0,1),Cell(0,1),Cell(0,1),Cell(1,1),Cell(0,1),Cell(0,1),Cell(-1,1),Cell(1,1),Cell(1,1),Cell(-1,1),Cell(-1,1),Cell(1,1),Cell(-1,1),Cell(1,1),Cell(-1,1),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,-1),Cell(-1,-1),Cell(1,-1),Cell(-1,-1),Cell(1,-1),Cell(1,-1),Cell(-1,-1),Cell(-1,-1),Cell(1,-1),Cell(0,-1),Cell(0,-1),Cell(-1,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),
	/* 8  */	Cell(0,1),Cell(0,1),Cell(0,1),Cell(0,1),Cell(0,1),Cell(0,1),Cell(0,1),Cell(1,1),Cell(1,1),Cell(-1,1),Cell(-1,1),Cell(1,1),Cell(-1,1),Cell(1,1),Cell(-1,1),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,-1),Cell(-1,-1),Cell(1,-1),Cell(-1,-1),Cell(1,-1),Cell(1,-1),Cell(-1,-1),Cell(-1,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),
	/* 9  */	Cell(0,1),Cell(0,1),Cell(0,1),Cell(0,1),Cell(0,1),Cell(0,1),Cell(0,1),Cell(1,1),Cell(1,1),Cell(-1,1),Cell(-1,1),Cell(1,1),Cell(-1,1),Cell(1,1),Cell(-1,1),Cell(1,1),Cell(-1,1),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,-1),Cell(-1,-1),Cell(1,-1),Cell(-1,-1),Cell(1,-1),Cell(-1,-1),Cell(1,-1),Cell(1,-1),Cell(-1,-1),Cell(-1,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),
	/* 10 */	Cell(0,1),Cell(0,1),Cell(0,1),Cell(0,1),Cell(0,1),Cell(0,1),Cell(0,1),Cell(1,1),Cell(1,1),Cell(-1,1),Cell(-1,1),Cell(1,1),Cell(1,1),Cell(-1,1),Cell(-1,1),Cell(1,1),Cell(-1,1),Cell(1,1),Cell(-1,1),Cell(1,1),Cell(-1,1),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,0),Cell(-1,0),Cell(1,-1),Cell(-1,-1),Cell(1,-1),Cell(-1,-1),Cell(1,-1),Cell(-1,-1),Cell(1,-1),Cell(1,-1),Cell(-1,-1),Cell(-1,-1),Cell(1,-1),Cell(1,-1),Cell(-1,-1),Cell(-1,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1),Cell(0,-1)
};

/*
 * This is a running total of the entries in the RadiusOffset table, so that entry N covers
 * every cell out to a radius of N. The sums are spelled out rather than folded together so
 * that the size of each ring (8, 12, 16, ...) stays visible.
 */
int const MapClass::RadiusCount[] = {
	/* 0  */	1,
	/* 1  */	1 + 8,
	/* 2  */	1 + 8 + 12,
	/* 3  */	1 + 8 + 12 + 16,
	/* 4  */	1 + 8 + 12 + 16 + 24,
	/* 5  */	1 + 8 + 12 + 16 + 24 + 28,
	/* 6  */	1 + 8 + 12 + 16 + 24 + 28 + 32,
	/* 7  */	1 + 8 + 12 + 16 + 24 + 28 + 32 + 40,
	/* 8  */	1 + 8 + 12 + 16 + 24 + 28 + 32 + 40 + 44,
	/* 9  */	1 + 8 + 12 + 16 + 24 + 28 + 32 + 40 + 44 + 48,
	/* 10 */	1 + 8 + 12 + 16 + 24 + 28 + 32 + 40 + 44 + 48 + 56,
};


CellClass BlubCell;

/*
 * This is what each movement class makes of each kind of terrain. A cell is only ever joined
 * to a movement zone when this reads TRAVERSAL_PASSABLE for it, so this one table decides zone
 * grouping, hierarchical path gating, and every reachability flood in the game.
 */
int MZonePassability[MZONE_COUNT][PASSABLE_COUNT] = {
	/// LAND                CRUSH                 BLOCKED               WATER                 PARTIALLY_BLOCKED     NO                    OUTSIDE
	{ TRAVERSAL_PASSABLE,   TRAVERSAL_IMPASSABLE, TRAVERSAL_IMPASSABLE, TRAVERSAL_IMPASSABLE, TRAVERSAL_IMPASSABLE, TRAVERSAL_IMPASSABLE, TRAVERSAL_ILLEGAL },	/// NORMAL
	{ TRAVERSAL_PASSABLE,   TRAVERSAL_PASSABLE,   TRAVERSAL_IMPASSABLE, TRAVERSAL_IMPASSABLE, TRAVERSAL_IMPASSABLE, TRAVERSAL_IMPASSABLE, TRAVERSAL_ILLEGAL },	/// CRUSHER
	{ TRAVERSAL_PASSABLE,   TRAVERSAL_PASSABLE,   TRAVERSAL_PASSABLE,   TRAVERSAL_IMPASSABLE, TRAVERSAL_IMPASSABLE, TRAVERSAL_IMPASSABLE, TRAVERSAL_ILLEGAL },	/// DESTROYER
	{ TRAVERSAL_PASSABLE,   TRAVERSAL_PASSABLE,   TRAVERSAL_PASSABLE,   TRAVERSAL_PASSABLE,   TRAVERSAL_IMPASSABLE, TRAVERSAL_IMPASSABLE, TRAVERSAL_ILLEGAL },	/// AMPHIBIOUS_DESTROYER
	{ TRAVERSAL_PASSABLE,   TRAVERSAL_PASSABLE,   TRAVERSAL_IMPASSABLE, TRAVERSAL_PASSABLE,   TRAVERSAL_IMPASSABLE, TRAVERSAL_IMPASSABLE, TRAVERSAL_ILLEGAL },	/// AMPHIBIOUS_CRUSHER
	{ TRAVERSAL_PASSABLE,   TRAVERSAL_IMPASSABLE, TRAVERSAL_IMPASSABLE, TRAVERSAL_PASSABLE,   TRAVERSAL_IMPASSABLE, TRAVERSAL_IMPASSABLE, TRAVERSAL_ILLEGAL },	/// AMPHIBIOUS
	{ TRAVERSAL_PASSABLE,   TRAVERSAL_PASSABLE,   TRAVERSAL_PASSABLE,   TRAVERSAL_IMPASSABLE, TRAVERSAL_IMPASSABLE, TRAVERSAL_PASSABLE,   TRAVERSAL_ILLEGAL },	/// SUBTERANNEAN
	{ TRAVERSAL_PASSABLE,   TRAVERSAL_IMPASSABLE, TRAVERSAL_IMPASSABLE, TRAVERSAL_IMPASSABLE, TRAVERSAL_PASSABLE,   TRAVERSAL_IMPASSABLE, TRAVERSAL_ILLEGAL },	/// INFANTRY
	{ TRAVERSAL_PASSABLE,   TRAVERSAL_PASSABLE,   TRAVERSAL_PASSABLE,   TRAVERSAL_IMPASSABLE, TRAVERSAL_PASSABLE,   TRAVERSAL_IMPASSABLE, TRAVERSAL_ILLEGAL },	/// INFANTRY_DESTROYER
	{ TRAVERSAL_PASSABLE,   TRAVERSAL_PASSABLE,   TRAVERSAL_PASSABLE,   TRAVERSAL_PASSABLE,   TRAVERSAL_PASSABLE,   TRAVERSAL_PASSABLE,   TRAVERSAL_ILLEGAL },	/// FLYER
};


int LastAdjacentZone;

/*
 * This is the facing that runs across a bridge deck, sideways to the direction the span
 * itself travels, indexed by a bridge tile's offset within its tile set. Stepping a deck
 * cell this way and the opposite way reaches the pair of cells alongside it. Both the road
 * and the train bridge sets are indexed through it, and the two entries that are
 * FACING_NONE are the tiles that are not span pieces at all.
 */
FacingType BridgeSideFacings[BRIDGE_COUNT] = {
	FACING_N,
	FACING_N,
	FACING_NONE,
	FACING_E,
	FACING_E,
	FACING_NONE,
	FACING_N,
	FACING_N,
	FACING_N,
	FACING_N,
	FACING_N,
	FACING_E,
	FACING_E,
	FACING_E,
	FACING_E,
	FACING_E
};


unsigned int Pick_Random_UInt(unsigned int start, unsigned int end);
double Random_Fraction(void);


#define IS_TILE_IN_SET(setname, setsize) (tile >= IsometricTileTypeClass::setname && tile < IsometricTileTypeClass::setname + setsize)


/// <summary>
/// Constructs the map object.
/// There is nothing to do here. The map is a global object that exists for the life of
/// the game; One_Time and Init_Clear are what actually bring it into a usable state.
/// </summary>
MapClass::MapClass(void)
{
	//nothing
}


/// <summary>
/// Destroys the map object.
/// This routine releases the zone and subzone tables and shuts down the systems that
/// live and die with the map -- the veinhole monsters and the tiberium spread and
/// growth logic.
/// </summary>
MapClass::~MapClass(void)
{
	VeinholeMonsterClass::Reset();
	VeinholeMonsterClass::Clear_Global_Data();
	TiberiumClass::Deinit_Tiberium_Spread_System();
	TiberiumClass::Deinit_Tiberium_Growth_System();

	if (CellZones != NULL) {
		delete [] CellZones;
		CellZones = NULL;
	}

	if (CellSubzones != NULL) {
		delete [] CellSubzones;
		CellSubzones = NULL;
	}
}


/// <summary>
/// Lists the members the map holds.
/// The zone and subzone tables are raw blocks whose size the archive itself establishes, so
/// MouseClass::Load and MouseClass::Save carry them separately once the counts have arrived.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void MapClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	// ZoneAdjacency -- scratch for the zone rebuild, which fills it again from the loaded terrain.
	// Zones
	stream.Serialize(ZoneCount);
	// ZoneConnections -- likewise part of the zone graph, read outside the archive.
	// CellZones
	stream.Serialize(CellZoneCount);
	// CellSubzones -- the subzone graph, grown again from the loaded terrain.
	// SubzoneTrackingEntryCount
	// SubzoneConnectionStaging
	// SubzoneTracking
	stream.Serialize(PendingBridgeCells);
	stream.Serialize(DirtyIceCells);
	stream.Serialize(PlayRect);
	stream.Serialize(LocalRect);

	// IterX -- the cell iterators, which name a slot of a cell array that the load throws away.
	// Reset_Iterator establishes them before any walk.
	// IterY
	// IterColumn
	// IterCell
	// LocalIterX
	// LocalIterY
	stream.Serialize(MapRect);
	stream.Serialize(TotalValue);
	stream.Serialize(DeformMask);
	stream.Serialize(DeformCell);
	stream.Serialize(DeformFrame);
	stream.Serialize(CrackedIce);

	// Array -- the cell array is reallocated by the load, and each cell reinstalls itself in
	// CellClass::Post_Load.
	// RadiusCount -- constant scan tables.
	// RadiusOffset
	// OcclusionOffset
	stream.Serialize(XSize);
	stream.Serialize(YSize);
	stream.Serialize(Size);
	stream.Serialize(Crates);
	stream.Serialize(Redraws);
	stream.Serialize(TaggedCells);
}


/// <summary>
/// Converts a playfield rectangle point into cell space.
/// The playfield is held as a rotated square, so this routine turns a point in that square
/// into the diamond cell space the rest of the map code works in.
/// </summary>
/// <returns>Returns with the equivalent point in cell coordinates.</returns>
Point2D MapClass::PlayRect_To_Cell_Point(Point2D const & point) const
{
	int x = ((point.Y + 1) >> 1) + point.X;
	int y = ((point.Y >> 1) - point.X) + PlayRect.Width;
	return(Point2D(x, y));
}


/// <summary>
/// Converts a local view point into cell space.
/// The point is measured from the corner of the local view rectangle rather than from the
/// playfield, so it is rebased onto the playfield on the way through.
/// </summary>
/// <returns>Returns with the equivalent point in cell coordinates.</returns>
Point2D MapClass::LocalRect_To_Cell_Point(Point2D const & point) const
{
	return(PlayRect_To_Cell_Point(point + LocalRect.Top_Left()));
}


/// <summary>
/// Converts a cell point into playfield rectangle space.
/// This routine is the inverse of PlayRect_To_Cell_Point, odd playfield widths included.
/// </summary>
/// <returns>Returns with the equivalent point in playfield rectangle space.</returns>
Point2D MapClass::Cell_To_PlayRect_Point(Point2D const & point) const
{
	int px = point.X;
	int py = point.Y;
	int w = PlayRect.Width;

	int d;
	if ((w & 1) != 0) {
		d = px - py + 1;
	} else {
		d = px - py;
	}
	int x = w / 2 + (d >> 1);
	int y = px + py - w;
	return(Point2D(x, y));
}


/// <summary>
/// Converts a cell point into local view space.
/// This routine is the inverse of LocalRect_To_Cell_Point. The result is measured from the
/// corner of the local view rectangle.
/// </summary>
/// <returns>Returns with the equivalent point in local view space.</returns>
Point2D MapClass::Cell_To_LocalRect_Point(Point2D const & point) const
{
	return(Cell_To_PlayRect_Point(point) - LocalRect.Top_Left());
}


/// <summary>
/// Converts a playfield rectangle location into a cell.
/// This is the Cell flavor of PlayRect_To_Cell_Point, for callers that carry the location
/// about in a Cell rather than a point.
/// </summary>
/// <param name="cell">The playfield rectangle location, carried in a Cell.</param>
/// <returns>Returns with the cell that location falls on.</returns>
Cell MapClass::PlayRect_To_Cell(Cell const & cell)
{
	Point2D pt = PlayRect_To_Cell_Point(Point2D(cell.X, cell.Y));
	return(Cell(pt.X, pt.Y));
}


/// <summary>
/// Converts a local view location into a cell.
/// This is the Cell flavor of LocalRect_To_Cell_Point, for callers that carry the location
/// about in a Cell rather than a point.
/// </summary>
/// <param name="cell">The local view location, carried in a Cell.</param>
/// <returns>Returns with the cell that location falls on.</returns>
Cell MapClass::LocalRect_To_Cell(Cell const & cell)
{
	Point2D pt = LocalRect_To_Cell_Point(Point2D(cell.X, cell.Y));
	return(Cell(pt.X, pt.Y));
}


/// <summary>
/// Converts a cell into a playfield rectangle location.
/// This is the Cell flavor of Cell_To_PlayRect_Point, for callers that carry the location
/// about in a Cell rather than a point.
/// </summary>
/// <returns>Returns with the playfield rectangle location, carried in a Cell.</returns>
Cell MapClass::Cell_To_PlayRect(Cell const & cell)
{
	Point2D pt = Cell_To_PlayRect_Point(Point2D(cell.X, cell.Y));
	return(Cell(pt.X, pt.Y));
}


/// <summary>
/// Converts a cell into a local view location.
/// This is the Cell flavor of Cell_To_LocalRect_Point, for callers that carry the location
/// about in a Cell rather than a point.
/// </summary>
/// <returns>Returns with the local view location, carried in a Cell.</returns>
Cell MapClass::Cell_To_LocalRect(Cell const & cell)
{
	Point2D pt = Cell_To_LocalRect_Point(Point2D(cell.X, cell.Y));
	return(Cell(pt.X, pt.Y));
}


/// <summary>
/// Fetches the cell that contains the specified coordinate.
/// This routine never fails. A coordinate that lies outside the map is handed the scratch
/// cell instead, so callers may reach past the edge of the map without checking first --
/// but whatever they write there is thrown away.
/// </summary>
/// <returns>Returns with a reference to the cell that holds that coordinate.</returns>
CellClass & MapClass::operator[](Coord const & coord) const
{
	int x = coord.X / CELL_LEPTON_W;
	int y = coord.Y / CELL_LEPTON_H;

	int cellnum = x + y * MAP_CELL_H;

	if (cellnum >= 0 && cellnum < Array.Length() && Array[cellnum] != NULL) {
		return(*Array[cellnum]);
	}

	BlubCell.CellID = Cell(x, y);
	return(BlubCell);
};


/// <summary>
/// Fetches the cell at the specified cell location.
/// This routine never fails. A location that lies outside the map is handed the scratch
/// cell instead, so callers may reach past the edge of the map without checking first --
/// but whatever they write there is thrown away.
/// </summary>
/// <returns>Returns with a reference to the cell at that location.</returns>
CellClass & MapClass::operator[](Cell const & cell) const
{
	int cellnum = cell.X + cell.Y * MAP_CELL_H;

	if (cellnum >= 0 && cellnum < MAP_CELL_TOTAL && Array[cellnum] != NULL) {
		return(*Array[cellnum]);
	}

	BlubCell.CellID = cell;
	return(BlubCell);
};


/// <summary>
/// Determines if a real cell exists at the location specified.
/// The map only keeps cell objects for the ground it actually covers. Use this routine
/// before treating a cell reference as anything more than scratch space, since the
/// subscript operator will hand out the throwaway cell rather than fail.
/// </summary>
/// <returns>bool; Is there a genuine cell at that location?</returns>
/// <remarks>The location is not range checked. It must lie within the cell array.</remarks>
bool MapClass::Is_Valid(Cell const & cell)
{
	int cellnum = cell.X + cell.Y * MAP_CELL_H;

	if (Array[cellnum] != NULL) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * MapClass::One_Time -- Performs special one time initializations for the map.                *
 *                                                                                             *
 *    This routine is used by the game initialization function in order to perform any one     *
 *    time initializations required for the map. This includes allocation of the map and       *
 *    setting up its default dimensions.                                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine MUST be called once and only once.                                 *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *   12/01/1994 BR : Added CellTriggers initialization                                         *
 *=============================================================================================*/
void MapClass::One_Time(void)
{
	BASECLASS::One_Time();

	XSize = MAP_CELL_W;
	YSize = MAP_CELL_H;
	Size = XSize * YSize;

	/*
	**	Allocate the cell array.
	*/
	Alloc_Cells();

	int i;
	for (i = 0; i < ARRAY_SIZE(SubzoneTracking); i++) {
		SubzoneTracking[i].Clear();
	}

	for (i = 0; i < MZONE_COUNT; i++) {
		Zones[i] = NULL;
	}
}


/***********************************************************************************************
 * MapClass::Init_Clear -- clears the map & buffers to a known state                           *
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
void MapClass::Init_Clear(void)
{
	DebugString("MapClass::Init_Clear entry\n");
	Call_Back();
	DepthBuffer->Fill(0xFFFF);
	AlphaBuffer->Fill(127);
	Call_Back();
	BASECLASS::Init_Clear();
	Call_Back();
	/// Init_Cells();
	//Call_Back();
	DeformCell = CELL_NONE;
	DeformMask = 0;
	for (int index = 0; index < ARRAY_SIZE(Crates); index++) {
		Crates[index].Init();
	}
	Redraws = 0;
	DebugString("MapClass::Init_Clear done\n");
}


/***********************************************************************************************
 * MapClass::Alloc_Cells -- allocates the cell array                                           *
 *                                                                                             *
 * This routine should be called at One_Time, and after loading the Map object from a save     *
 * game, but prior to loading the cell objects.                                                *
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
void MapClass::Alloc_Cells(void)
{
	/*
	**	Assume that whatever the contents of the VectorClass are is garbage
	**	(it may have been loaded from a save-game file), so zero it out first.
	*/
	new (&Array) VectorClass<CellClass *>;
	Array.Resize(Size);

	for (int i = 0; i < Size; i++) {
		Array[i] = NULL;
	}
}


/***********************************************************************************************
 * MapClass::Free_Cells -- frees the cell array                                                *
 *                                                                                             *
 * This routine is used by the Load_Game routine to free the map's cell array before loading   *
 * the map object from disk; the array is then re-allocated & cleared before the cell objects  *
 * are loaded.                                                                                 *
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
void MapClass::Free_Cells(void)
{
	int i;

	for (i = 0; i < Array.Length(); i++) {
		CellClass * cptr = Array[i];
		if (cptr != NULL) {
			cptr->BridgeDeckCell = NULL;
			cptr->UnusedCell = 0;
		}
	}

	for (i = 0; i < Array.Length(); i++) {
		if (Array[i] != NULL) {
			delete Array[i];
			Array[i] = NULL;
		}
	}

	IsometricTileTypeClass::Free_Unused_Drawers(0, true);

	if (Array.Length() < MAP_CELL_TOTAL) {
		Array.Resize(MAP_CELL_TOTAL);
		for (i = 0; i < Array.Length(); i++) {
			Array[i] = NULL;
		}
	}
}


/***********************************************************************************************
 * MapClass::Init_Cells -- Initializes the cell array to a fresh state.                        *
 *                                                                                             *
 * This routine is used by Init_Clear to set the cells to a known state; it's also used by     *
 * the Load_Game routine to init all cells before loading a set of cells from disk, so it      *
 * needs to be called separately from the other Init_xxx() routines.                           *
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
void MapClass::Init_Cells(void)
{
	TotalValue = 0;
	for (int y = 0; y < MAP_CELL_H; y++) {
		for (int x = 0; x < MAP_CELL_W; x++) {
			int cellnum = x + y * MAP_CELL_H;
			if (cellnum < Array.Length() && Array[cellnum] != NULL) {
				new (Array[cellnum]) CellClass;
			}
		}
	}
}


/***********************************************************************************************
 * MapClass::Set_Map_Dimensions -- Set map dimensions.                                         *
 *                                                                                             *
 *    This routine is used to set the legal limits and position of the                         *
 *    map as it relates to the overall map array. Typically, this is                           *
 *    called by the scenario loading code.                                                     *
 *                                                                                             *
 * INPUT:   x,y   -- The X and Y coordinate of the "upper left" corner                         *
 *                   of the map.                                                               *
 *                                                                                             *
 *          w,h   -- The width and height of the legal map.                                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/14/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void MapClass::Set_Map_Dimensions(Rect const & rect, bool reset_cells, int cell_height, bool refresh_map)
{
	int i = 0;

	int w = PlayRect.Width;
	int h = 2 * Map.PlayRect.Height + 8;
	CellClass *cptr;
	CellClass *cells = NULL;
	CellClass *cells_base = NULL;

	Cell cell(CELL_NONE);

	for (i = 0; i < Objects.Count(); i++) {
		Objects[i]->Mark(MARK_UP);
	}

	for (i = 0; i < Array.Length(); i++) {
		if (!In_Radar(Cell(i % MAP_CELL_W, i / MAP_CELL_H))) {
			CellClass *c = Array[i];
			if (c != NULL) {
				c->Height = cell_height;
			}
		}
	}

	int cellcount = 0;

	if (!reset_cells) {
		cells = (CellClass *) new char[sizeof(CellClass) * w * h];//new CellClass[w * h];
		cells_base = cells;
		Reset_Iterator();
		cptr = Iterate();
		cell = cptr->Fetch_CellID();
		while (cptr != NULL) {
			*cells = *cptr;
			cells->BridgeDeckCell = NULL;
			cells->UnusedCell = NULL;
			cptr = Iterate();
			cellcount++;
			cells++;
		}
	}

	PlayRect = rect;
	PlayRect.X = 0;
	PlayRect.Y = 0;

	if (PlayRect.Width > 63 || PlayRect.Height > 63) {
		Map.Set_Radar_Scale(2);
	}

	MapRegionClass::MapStartDiagonal = Map.PlayRect.Width;
	MapRegionClass::MapEndDiagonal = Map.PlayRect.Width + 2 * Map.PlayRect.Height;

	MapRect.X = 1;
	MapRect.Y = 1;
	int msize = rect.Height + rect.Width - 1;
	MapRect.Width = msize;
	MapRect.Height = msize;

	for (int y = 0; y < 2 * MapRect.Height + 2; y++) {
		for (int x = 0; x < MapRect.Width + 2; x++) {
			Cell cell(x, y);
			if (In_Radar(cell)) {
				int idx = (MAP_CELL_H * y) + x;
				CellClass *c = Array[idx];
				if (c == NULL) {
					CellClass *cc = new CellClass;
					cc->Set_CellID(cell);
					Array[idx] = cc;
				} else {
					c = new(c) CellClass;
					c->Set_CellID(cell);
				}
				Array[idx]->Height = cell_height;
			}
		}
	}

	if (!reset_cells) {
		Reset_Iterator();
		Cell icell = Iterate()->CellID;
		Cell cellpos = Cell(rect.Y + rect.X, rect.Y - rect.X);
		cell = (icell - cell) - cellpos;

		cells = cells_base;
		while (cellcount > 0) {
			cptr = cells;
			Cell scell = cptr->CellID;
			Cell shifted = scell + cell;

			CellClass *c = &(*this)[shifted];

			Cell dcell = c->CellID;
			if (c != NULL) {
				*c = *cptr;
				c->Set_CellID(dcell);
			}

			cellcount--;
			cells++;
		}

		Reset_Iterator();
		cptr = Iterate();
		while (cptr != NULL) {

			Cell c = cptr->Fetch_CellID();

			if (c.X + c.Y < PlayRect.Width - 2 * rect.Y + 1 ||
				c.Y - c.X > PlayRect.Width + 2 * rect.X - 1 ||
				c.X + c.Y > PlayRect.Width + 2 * (h + PlayRect.Height - rect.Y - Map.PlayRect.Height) ||
				c.X - c.Y > PlayRect.Width + 2 * (w - rect.X - Map.PlayRect.Width) - 1) {

				int cellindex = c.X + c.Y * MAP_CELL_W;

				delete Array[cellindex];
				Array[cellindex] = new CellClass;
				Array[cellindex]->Set_CellID(c);
				Array[cellindex]->ITType = ISOTILE_NONE;
				Array[cellindex]->SubTile = 0;
				Array[cellindex]->Ramp = 0;
				Array[cellindex]->Overlay = OVERLAY_NONE;
				Array[cellindex]->Height = cell_height;
			}
			cptr = Iterate();
		}

		for (i = 0; i < WAYPT_COUNT; i++) {
			if (Scen->Is_Valid_Waypoint(i)) {
				Scen->Set_Waypoint(i, Scen->Get_Waypoint_Cell(i) + cell);
			}
		}

		for (i = 0; i < Tubes.Count(); i++) {
			Tubes[i]->Enter += cell;
			Tubes[i]->Exit += cell;
		}

		Coord coord = Coord(cell, 0) - Coord(CELL_LEPTON_W / 2, CELL_LEPTON_H / 2, 0);

		for (i = 0; i < Objects.Count(); i++) {
			ObjectClass *obj = Objects[i];
			if (obj->RTTI != RTTI_ANIM) {
				obj->Set_Coord(coord + obj->Get_Coord());
			}
		}

		for (i = 0; i < Anims.Count(); i++) {
			AnimClass *anim = Anims[i];
			if (anim->xObject == NULL) {
				anim->Set_Coord(coord + anim->Get_Coord());
			}
		}

		ScenarioInit++;

		for (i = 0; i < Objects.Count(); i++) {
			if (Objects[i]->RTTI != RTTI_PARTICLESYSTEM || ((ParticleSystemClass *)Objects[i])->Class->Behaves_Like() != PSYS_BEHAVIOR_GAS) {
				Cell cell = Objects[i]->PositionCell;
				const Cell *list = Objects[i]->Occupy_List();
				while (*list != REFRESH_EOL) {
					Cell ncell = cell + *list;
					if (!In_Radar(ncell)) {
						Objects[i]->Limbo();
						delete Objects[i];
						i--;
						break;
					}
					list++;
				}
			}
		}

		ScenarioInit--;

		for (i = 0; i < Array.Length(); i++) {
			CellClass *c1 = Array[i];
			CellClass **c2 = &Array[i];
			if (c1 != NULL) {
				if (!In_Radar(Cell(i % MAP_CELL_W, i / MAP_CELL_H))) {
					*c2 = NULL;
					delete c1;
				}
			}
		}

		Reset_Iterator();
		cptr = Iterate();
		while (cptr != NULL) {
			if (cptr->IsBridgeDeck && cptr->BridgeDeckCell == NULL) {
				FacingType facing = cptr->IsBridgeEastWest ? FACING_N : FACING_E;
				if (cptr->Overlay == OVERLAY_BRIDGE1 || cptr->Overlay == OVERLAY_BRIDGE2) {
					cptr->Set_Under_Bridge(facing);
				} else {
					cptr->Set_Under_Rail_Bridge(facing);
				}
			}
			cptr = Iterate();
		}

		bool freshcalled;
		if (refresh_map) {
			Fresh_Map();
			freshcalled = true;
		} else {
			freshcalled = false;
		}

		for (i = 0; i < Objects.Count(); i++) {
			Objects[i]->Mark(MARK_DOWN);
		}

		Map.Flag_To_Redraw(GS_REDRAW_ALL);

		if (freshcalled) {
			refresh_map = false;
		}
	}

	if (refresh_map) {
		Fresh_Map();
	}

	new(&BlubCell) CellClass;
	delete (void *)cells_base; /// operator delete is called directly here.
}


/// <summary>
/// Rebuilds the zone and subzone data for the current playfield.
/// This routine is called whenever the map changes size. The zone layer and the subzone
/// graph the pathfinder searches over are both thrown away and grown again from the
/// terrain, since neither survives a change of dimensions.
/// </summary>
void MapClass::Fresh_Map(void)
{
	int i;

	if (CellZones != NULL) {
		delete [] CellZones;
		CellZones = NULL;
	}
	if (CellSubzones != NULL) {
		delete [] CellSubzones;
		CellSubzones = NULL;
	}

	CellZoneCount = (PlayRect.Width + PlayRect.Height + 1) * (PlayRect.Width + PlayRect.Height + 1);

	CellZones = new CellZoneStruct[CellZoneCount];
	CellSubzones = new CellSubzoneStruct[CellZoneCount];

	Search.Update_Map_Dimensions(PlayRect);

	for (i = 0; i < SUBZONE_COUNT; i++) {
		int v = (1 << (i + 1));
		SubzoneTracking[i].Set_Growth_Step((4 * PlayRect.Width * PlayRect.Height) / (v * v));
	}

	Overpass();
	Compute_Zone_Connections();
	Zone_Reset();
	Reset_All_Subzones();
}


/// <summary>
/// Sets the local view rectangle of the map.
/// The requested rectangle is trimmed to fit inside the playfield. Any object that finds
/// itself within the new local area is locked to it and takes a fresh look around, so that
/// the ground it can see is revealed the moment the area opens up.
/// </summary>
/// <param name="size">The desired local view rectangle; it is clipped to the playfield.</param>
void MapClass::Set_Local_Dimensions(Rect const & size)
{
	LocalRect = Intersect(size, PlayRect);
	LocalRect.X = std::max(LocalRect.X, 2);
	LocalRect.Y = std::max(LocalRect.Y, 2);
	LocalRect.Width = std::min(LocalRect.Width, PlayRect.Width - LocalRect.X - 2);
	LocalRect.Height = std::min(LocalRect.Height, PlayRect.Height - LocalRect.Y - (2 * 3));

	Map.Flag_To_Redraw(GS_REDRAW_ALL);
	for (int i = 0; i < Technos.Count(); i++) {
		TechnoClass * t = Technos[i];
		bool was = t->IsLocked;
		t->IsLocked = In_Local_Radar(t->PositionCell);
		if (!was && t->IsLocked && t->House->Is_Player_Control() && t->RTTI != RTTI_BUILDING && t->IsActive && !t->IsInLimbo) {
			t->Look();
		}
	}
}


/***********************************************************************************************
 * MapClass::Sight_From -- Mark as visible the cells within a specified radius.                *
 *                                                                                             *
 *    This routine is used to reveal the cells around a specific location.                     *
 *    Typically, as a unit moves or is deployed, this routine will be                          *
 *    called. Since it deals with MANY cells, it needs to be extremely                         *
 *    fast.                                                                                    *
 *                                                                                             *
 * INPUT:   cell     -- The coordinate that the sighting originates from.                      *
 *                                                                                             *
 *          sightrange-- The distance in cells that sighting extends.                          *
 *                                                                                             *
 *          incremental-- Is this an incremental sighting. In other                            *
 *                      words, has this function been called before where                      *
 *                      the center coordinate is no more than one cell                         *
 *                      distant from the last time?                                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/19/1992 JLB : Created.                                                                 *
 *   03/08/1994 JLB : Updated to use sight table and incremental flag.                         *
 *   05/18/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
void MapClass::Sight_From(Coord const & xcoord, int sightrange, HouseClass * house, bool incremental, bool dont_map, bool unfog, bool byheight)
{
	int xx;				// Center cell X coordinate (bounds checking).
	Cell const * ptr;	// Offset pointer.
	Cell const * ptr2;	// Offset pointer.
	int count;			// Counter for number of offsets to process.

	int cell_height = xcoord.Z / LEVEL_LEPTON_H;

	Coord coord = xcoord;
	coord.X = coord.X + (TacticalMap->Z_Lepton_To_Pixel(coord.Z) / -CELL_PIXEL_W) * CELL_LEPTON;
	coord.Y = coord.Y + (TacticalMap->Z_Lepton_To_Pixel(coord.Z) / -CELL_PIXEL_W) * CELL_LEPTON;

	Cell cell = coord.As_Cell();                            /// Center cell as is appears on the map
	Cell hoffset = cell - xcoord.As_Cell() - Cell(2, 2);    /// Height offset between real and apparent cells

	/*
	**	Units that are off-map cannot sight.
	*/
	if (!In_Radar(cell)) return;
	if (!sightrange) return;
	sightrange = std::min(sightrange, 10);

	/*
	**	Determine logical cell coordinate for center scan point.
	*/
	xx = cell.X;

	/*
	**	Incremental scans only scan the outer rings. Full scans
	**	scan all internal cells as well.
	*/
	count = RadiusCount[sightrange];
	ptr = &RadiusOffset[0];
	ptr2 = &OcclusionOffset[0];
	if (!Rule->IsRevealByHeight && incremental) {
		if (sightrange > 2) {
			ptr += RadiusCount[sightrange-3];
			ptr2 += RadiusCount[sightrange-3];
			count -= RadiusCount[sightrange-3];
		}
	}
	ptr2--;

	if (house != NULL && PlayerPtr != NULL) {
		if (house != PlayerPtr) {
			if ((house->RadarSpied & (1 << PlayerPtr->Class->House)) != 0) {
				house = PlayerPtr;
			}
		}
		if (house != PlayerPtr) {
			if (house->Is_Ally(PlayerPtr) && Rule->IsAllyReveal) {
				house = PlayerPtr;
			}
		}
	}

	if (house != PlayerPtr) {
		return;
	}

	/*
	**	Process all offsets required for the desired scan.
	*/
	while (count--) {
		Cell newcell;		// New cell with offset.
		int xdiff;			// New cell's X coordinate distance from center.

		newcell = cell + *ptr++;
		ptr2++;

		/*
		**	Determine if the map edge has been wrapped. If so,
		**	then don't process the cell.
		*/
		if (!In_Radar(newcell)) continue;
		xdiff = newcell.X - xx;
		xdiff = abs(xdiff);
		if (xdiff > sightrange) continue;
		if (Distance(newcell, cell) > sightrange) continue;

		/*
		**	Map the cell. For incremental scans, then update
		**	adjacent cells as well. For full scans, just update
		**	the cell itself.
		*/
		bool ok = false;
		CellClass * cellptr = &(*this)[newcell];
		if (byheight && Rule->IsRevealByHeight) {
			if (Map[newcell - hoffset + *ptr2].Height <= cell_height + 3) {
				ok = true;
			}
		} else {
			ok = true;
		}

		if (ok) {
			cellptr->IsToFog = false;
			if (unfog) {
				if ((!cellptr->IsFogVisible || !cellptr->IsFogMapped) && cellptr->IsMapped) {
					Map.Fog_Map_Cell(newcell, house);
				}
			} else {
				if ((!cellptr->IsMapped || !cellptr->IsVisible || !cellptr->IsFogVisible || !cellptr->IsFogMapped) && !dont_map) {
					Map.Map_Cell(newcell, house);
				}
			}
		}
	}
}


/***********************************************************************************************
 * MapClass::In_Radar -- Is specified cell in the radar map?                                   *
 *                                                                                             *
 *    This determines if the specified cell can be within the navigable                        *
 *    bounds of the map. Technically, this means, any cell that can be                         *
 *    scanned by radar. If a cell returns false from this function, then                       *
 *    the player could never move to or pass over this cell.                                   *
 *                                                                                             *
 * INPUT:   cell  -- The cell to examine.                                                      *
 *                                                                                             *
 * OUTPUT:  bool; Is this cell possible to be displayed on radar?                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/1992 JLB : Created.                                                                 *
 *   04/30/1994 JLB : Converted to member function.                                            *
 *   05/01/1994 JLB : Speeded up.                                                              *
 *=============================================================================================*/
bool MapClass::In_Radar(Cell const & cell) const
{
	int x = cell.X;
	int y = cell.Y;
	if ((x + y > PlayRect.Width) &&
		(x - y < PlayRect.Width) &&
		(y - x < PlayRect.Width) &&
		(x + y <= PlayRect.Width + 2 * PlayRect.Height)) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Determines if the specified coordinate is within the radar map.
/// This is the coordinate flavor of the cell test above -- the coordinate is reduced to
/// the cell that contains it and that cell is checked.
/// </summary>
/// <returns>bool; Is this coordinate possible to be displayed on radar?</returns>
bool MapClass::In_Radar(Coord const & coord) const
{
	return(In_Radar(coord.As_Cell()));
}


/***********************************************************************************************
 * MapClass::Place_Down -- Places the specified object onto the map.                           *
 *                                                                                             *
 *    This routine is used to place an object onto the map. It updates the "occupier" of the   *
 *    cells that this object covers. The cells are determined from the Occupy_List function    *
 *    provided by the object. Only one cell can have an occupier and this routine is the only  *
 *    place that sets this condition.                                                          *
 *                                                                                             *
 * INPUT:   cell     -- The cell to base object occupation around.                             *
 *                                                                                             *
 *          object   -- The object to place onto the map.                                      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/31/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void MapClass::Place_Down(Cell const & cell, ObjectClass * object)
{
	if (!object) return;

	if (object->Class_Of()->IsFootprint && object->In_Which_Layer() == LAYER_GROUND)  {
		Cell xlist[32];
		List_Copy(object->Occupy_List(), ARRAY_SIZE(xlist), xlist);
		Cell const * list = xlist;
		while (*list != REFRESH_EOL) {
			Cell newcell = cell + *list++;
			if (Is_Valid(newcell)) {
				(*this)[newcell].Occupy_Down(object, object->IsOnBridge);
				(*this)[newcell].Recalc_Attributes();
			}
		}
	}
}


/***********************************************************************************************
 * MapClass::Pick_Up -- Removes specified object from the map.                                 *
 *                                                                                             *
 *    The object specified is removed from the map by this routine. This will remove the       *
 *    occupation flag for all the cells that the object covers. The cells that are covered     *
 *    are determined from the Occupy_List function.                                            *
 *                                                                                             *
 * INPUT:   cell     -- The cell that the object is centered about.                            *
 *                                                                                             *
 *          object   -- Pointer to the object that will be removed.                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/31/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void MapClass::Pick_Up(Cell const & cell, ObjectClass * object)
{
	if (!object) return;

	if (object->Class_Of()->IsFootprint && object->In_Which_Layer() == LAYER_GROUND)  {
		Cell xlist[32];
		List_Copy(object->Occupy_List(), ARRAY_SIZE(xlist), xlist);
		Cell const * list = xlist;
		while (*list != REFRESH_EOL) {
			Cell newcell = cell + *list++;
			if (Is_Valid(newcell)) {
				(*this)[newcell].Occupy_Up(object, object->IsOnBridge);
				(*this)[newcell].Recalc_Attributes();
			}
		}
	}
}


/***********************************************************************************************
 * MapClass::Overpass -- Performs any final cleanup to a freshly constructed map.              *
 *                                                                                             *
 *    This routine will clean up anything necessary with the presumption that the map has      *
 *    been freshly created. Such things to clean up include various tiberium concentrations.   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the total credit value of the tiberium on the map.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1994 JLB : Created.                                                                 *
 *   02/13/1995 JLB : Returns total tiberium worth.                                            *
 *   02/15/1995 JLB : Optimal scan.                                                            *
 *=============================================================================================*/
int MapClass::Overpass(void)
{
	int i;
	int value = 0;

	for (i = 0; i < Anims.Count(); i++) {
		if (Anims[i]->IsToDeleteOnOverpass) {
			delete Anims[i];
			i--;
		}
	}

	Reset_Iterator();

	CellClass * iter = Iterate();
	while (iter) {
		iter->IsHorizontalLine = false;
		iter->IsVerticalLine = false;
		iter = Iterate();
	}

	Reset_Iterator();

	iter = Iterate();
	while (iter) {

		iter->UnusedCell = NULL;
		iter->Init_Drawer();
		iter->IsAnimAttached = false;

		if (iter->Tag) {

			if (iter->Tag->Is_Cross_Horizontal()) {
				for (i = 0; i < Map.MapRect.Width; i++) {
					Map[Cell(i + MapRect.X, iter->CellID.Y)].IsHorizontalLine = true;
				}
			}
			else if (iter->Tag->Is_Cross_Vertical()) {
				for (i = 0; i < Map.MapRect.Height; i++) {
					Map[Cell(iter->CellID.X, i + MapRect.Y)].IsVerticalLine = true;
				}
			}
		}

		/*
		**	Smooth out Tiberium. Cells that are not surrounded by other tiberium
		**	will be reduced in density.
		*/
		value += iter->Tiberium_Adjust(false);
		iter->Recalc_Attributes();
		if (iter->Overlay != OVERLAY_NONE) {
			if (OverlayTypes[iter->Overlay]->IsWall) {
				iter->Set_Wall_Owner();
			}
		}

		iter = Iterate();
	}
	return(value);
}


/// <summary>
/// Repairs one span of a high bridge.
/// The span is followed from its anchor cell, the broken pieces are restored, the supports
/// are put back under the raised center pieces, and the bridge deck is laid down again. The
/// zones and subzone connections are re-linked as the span is mended, so traffic may cross
/// the bridge once more. The repair gives up if the far end of the span is never reached.
/// </summary>
/// <param name="cptr">Pointer to the cell that anchors the span; the repair starts here.</param>
/// <param name="dir">The direction to follow the span in.</param>
/// <param name="dirty">Optional rectangle to fill with the area to redraw, or NULL.</param>
void MapClass::Repair_High_Bridge_Span(CellClass * cptr, FacingType dir, Rect * dirty)
{
	Cell cell = CELL_NONE;
	CellClass * cellptr = &Map[cptr->Fetch_CellID()];

	bool reset_zones = false;
	DynamicVectorClass<Cell> cells;

	/*
	 * Walk up to 30 cells in the requested facing, repairing bridge middle
	 * pieces and stopping at the far end piece.
	 */
	int length = 1;
	while (length < 30) {
		Cell newcell = Adjacent_Cell(cellptr->Fetch_CellID(), dir);
		cell = newcell;
		cellptr = &Map[newcell];

		int ittype = cellptr->ITType - IsometricTileTypeClass::BridgeSet + 1;

		if (dir == FACING_E) {

			/*
			 * East-west bridge bottom-right end pieces terminate the walk.
			 */
			if (ittype == IsometricTileTypeClass::BridgeBottomRight1 && cellptr->SubTile == 4) {
				Set_Bridge_End_State(cell, false, false);
				reset_zones |= Register_Subzone_Connections(Adjacent_Cell(cell, FACING_W));
				break;
			}
			if (ittype == IsometricTileTypeClass::BridgeBottomRight2 && cellptr->SubTile == 4) {
				Set_Bridge_End_State(cell, false, false);
				reset_zones |= Register_Subzone_Connections(Adjacent_Cell(cell, FACING_W));
				break;
			}

			/*
			 * East-west bridge middle pieces.
			 */
			if ((ittype == IsometricTileTypeClass::BridgeMiddle1 || ittype == IsometricTileTypeClass::BridgeMiddle1 + 3 || ittype == IsometricTileTypeClass::BridgeMiddle1 + 4 || ittype == IsometricTileTypeClass::BridgeMiddle1 + 1 || ittype == IsometricTileTypeClass::BridgeMiddle1 + 2) && cellptr->SubTile == 4) {

				Set_Bridge_Middle_State(cell, IsometricTileType(IsometricTileTypeClass::BridgeMiddle1 + IsometricTileTypeClass::BridgeSet - 1), ISOTILE_INVALID, -1, false);
				reset_zones |= Register_Subzone_Connections(cell);

				if (ittype == IsometricTileTypeClass::BridgeMiddle1 + 4) {
					Map[cell].Height += BRIDGE_CELL_HEIGHT;
					Map[Adjacent_Cell(cell, FACING_N)].Height += BRIDGE_CELL_HEIGHT;
					Map[Adjacent_Cell(cell, FACING_S)].Height += BRIDGE_CELL_HEIGHT;

					for (int x = 0; x < 2; x++) {
						for (int y = -2; y < 3; y++) {
							cells.Add(Cell(cell + Cell(x, y)));
						}
					}
				}
			}

		} else if (dir == FACING_S) {

			/*
			 * South bridge bottom-left end pieces terminate the walk.
			 */
			if (ittype == IsometricTileTypeClass::BridgeBottomLeft1 && cellptr->SubTile == 2) {
				Set_Bridge_End_State(cell, false, false);
				reset_zones |= Register_Subzone_Connections(Adjacent_Cell(cell, FACING_N));
				break;
			}
			if (ittype == IsometricTileTypeClass::BridgeBottomLeft2 && cellptr->SubTile == 2) {
				Set_Bridge_End_State(cell, false, false);
				reset_zones |= Register_Subzone_Connections(Adjacent_Cell(cell, FACING_N));
				break;
			}

			/*
			 * South bridge middle pieces.
			 */
			if ((ittype == IsometricTileTypeClass::BridgeMiddle2 || ittype == IsometricTileTypeClass::BridgeMiddle2 + 3 || ittype == IsometricTileTypeClass::BridgeMiddle2 + 4 || ittype == IsometricTileTypeClass::BridgeMiddle2 + 2 || ittype == IsometricTileTypeClass::BridgeMiddle2 + 1) && cellptr->SubTile == 2) {

				Set_Bridge_Middle_State(cell, IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle2 - 1), ISOTILE_INVALID, -1, false);
				reset_zones |= Register_Subzone_Connections(cell);

				if (ittype == IsometricTileTypeClass::BridgeMiddle2 + 4) {
					Map[cell].Height += BRIDGE_CELL_HEIGHT;
					Map[Adjacent_Cell(cell, FACING_E)].Height += BRIDGE_CELL_HEIGHT;
					Map[Adjacent_Cell(cell, FACING_W)].Height += BRIDGE_CELL_HEIGHT;

					for (int y = 0; y < 2; y++) {
						for (int x = -2; x < 3; x++) {
							cells.Add(Cell(cell + Cell(x, y)));
						}
					}
				}
			}
		}

		length++;
	}

	/*
	 * If the entire span was scanned without finding the end piece there is
	 * nothing more to do.
	 */
	if (length == 30) {
		return;
	}

	/*
	 * An end piece was found. Re-walk the span placing the bridge-piece
	 * overlay on each undamaged middle cell, and accumulate the dirty
	 * rectangle for the caller.
	 */
	cellptr = &Map[cptr->Fetch_CellID()];

	if (dirty != NULL) {
		Coord coord = Coord(cptr->Fetch_CellID(), LEVEL_LEPTON_H * cptr->Height);
		Point2D point;
		TacticalMap->Coord_To_Pixel(coord, point);
		dirty->X = point.X;
		dirty->Y = point.Y;
	}

	if (length > 0) {
		int remaining = length;
		do {
			int ittype = cellptr->ITType - IsometricTileTypeClass::BridgeSet + 1;
			if ((ittype != IsometricTileTypeClass::BridgeMiddle1 || (cellptr->SubTile & 1) != 0) && (ittype != IsometricTileTypeClass::BridgeMiddle2 || (unsigned char)cellptr->SubTile >= 5)) {
				if (dir == FACING_E) {
					new OverlayClass(OverlayTypes[OVERLAY_BRIDGE1], cellptr->Fetch_CellID(), HOUSE_NONE);
				} else {
					new OverlayClass(OverlayTypes[OVERLAY_BRIDGE2], cellptr->Fetch_CellID(), HOUSE_NONE);
				}
			}

			Cell newcell = Adjacent_Cell(cellptr->Fetch_CellID(), dir);
			cell = newcell;
			cellptr = &Map[newcell];

			remaining--;
		} while (remaining != 0);
	}

	/*
	 * Finalize the dirty rectangle from the two span endpoints.
	 */
	if (dirty != NULL) {
		Coord coord = Coord(cell, LEVEL_LEPTON_H * Map[cell].Height);
		Point2D point;
		TacticalMap->Coord_To_Pixel(coord, point);

		int x1 = dirty->X;
		int y1 = dirty->Y;
		int minx = dirty->X;
		if (dirty->X >= point.X) {
			minx = point.X;
		}
		int miny = dirty->Y;
		dirty->X = minx - 64;
		if (y1 >= point.Y) {
			miny = point.Y;
		}
		dirty->Y = miny - 64;
		dirty->Width = abs(x1 - point.X) + 128;
		dirty->Height = abs(y1 - point.Y) + 128;
	}

	if (reset_zones) {
		Zone_Reset();
	}

	if (cells.Count() > 0) {
		Recalc_Cells_In_List(cells);
	}
}


/// <summary>
/// Repairs one span of a train bridge.
/// This routine is the train bridge counterpart of Repair_High_Bridge_Span. The span is
/// followed from its anchor cell, the broken pieces are restored, the supports are put back
/// under the raised center pieces, and the rail deck is laid down again. The repair gives
/// up if the far end of the span is never reached.
/// </summary>
/// <param name="cptr">Pointer to the cell that anchors the span; the repair starts here.</param>
/// <param name="dir">The direction to follow the span in.</param>
/// <param name="dirty">Optional rectangle to fill with the area to redraw, or NULL.</param>
void MapClass::Repair_Train_Bridge_Span(CellClass * cptr, FacingType dir, Rect * dirty)
{
	Cell cell = CELL_NONE;
	CellClass * cellptr = &Map[cptr->Fetch_CellID()];

	bool reset_zones = false;
	DynamicVectorClass<Cell> cells;

	/*
	 * Walk up to 30 cells in the requested facing, repairing bridge middle
	 * pieces and stopping at the far end piece.
	 */
	int length = 1;
	while (length < 30) {
		Cell newcell = Adjacent_Cell(cellptr->Fetch_CellID(), dir);
		cell = newcell;
		cellptr = &Map[newcell];

		int ittype = cellptr->ITType - IsometricTileTypeClass::TrainBridgeSet + 1;

		if (dir == FACING_E) {

			/*
			 * East-west bridge bottom-right end pieces terminate the walk.
			 */
			if (ittype == IsometricTileTypeClass::BridgeBottomRight1 && cellptr->SubTile == 4) {
				Set_Bridge_End_State(cell, false, false);
				reset_zones |= Register_Subzone_Connections(Adjacent_Cell(cell, FACING_W));
				break;
			}
			if (ittype == IsometricTileTypeClass::BridgeBottomRight2 && cellptr->SubTile == 4) {
				Set_Bridge_End_State(cell, false, false);
				reset_zones |= Register_Subzone_Connections(Adjacent_Cell(cell, FACING_W));
				break;
			}

			/*
			 * East-west bridge middle pieces.
			 */
			if ((ittype == IsometricTileTypeClass::BridgeMiddle1 || ittype == IsometricTileTypeClass::BridgeMiddle1 + 3 || ittype == IsometricTileTypeClass::BridgeMiddle1 + 4 || ittype == IsometricTileTypeClass::BridgeMiddle1 + 1 || ittype == IsometricTileTypeClass::BridgeMiddle1 + 2) && cellptr->SubTile == 4) {

				Set_Bridge_Middle_State(cell, IsometricTileType(IsometricTileTypeClass::BridgeMiddle1 + IsometricTileTypeClass::TrainBridgeSet - 1), ISOTILE_INVALID, -1, false);
				reset_zones |= Register_Subzone_Connections(cell);

				if (ittype == IsometricTileTypeClass::BridgeMiddle1 + 4) {
					Map[cell].Height += BRIDGE_CELL_HEIGHT;
					Map[Adjacent_Cell(cell, FACING_N)].Height += BRIDGE_CELL_HEIGHT;
					Map[Adjacent_Cell(cell, FACING_S)].Height += BRIDGE_CELL_HEIGHT;

					for (int x = 0; x < 2; x++) {
						for (int y = -2; y < 3; y++) {
							cells.Add(Cell(cell + Cell(x, y)));
						}
					}
				}
			}

		} else if (dir == FACING_S) {

			/*
			 * South bridge bottom-left end pieces terminate the walk.
			 */
			if (ittype == IsometricTileTypeClass::BridgeBottomLeft1 && cellptr->SubTile == 2) {
				Set_Bridge_End_State(cell, false, false);
				reset_zones |= Register_Subzone_Connections(Adjacent_Cell(cell, FACING_N));
				break;
			}
			if (ittype == IsometricTileTypeClass::BridgeBottomLeft2 && cellptr->SubTile == 2) {
				Set_Bridge_End_State(cell, false, false);
				reset_zones |= Register_Subzone_Connections(Adjacent_Cell(cell, FACING_N));
				break;
			}

			/*
			 * South bridge middle pieces.
			 */
			if ((ittype == IsometricTileTypeClass::BridgeMiddle2 || ittype == IsometricTileTypeClass::BridgeMiddle2 + 3 || ittype == IsometricTileTypeClass::BridgeMiddle2 + 4 || ittype == IsometricTileTypeClass::BridgeMiddle2 + 2 || ittype == IsometricTileTypeClass::BridgeMiddle2 + 1) && cellptr->SubTile == 2) {

				Set_Bridge_Middle_State(cell, IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle2 - 1), ISOTILE_INVALID, -1, false);
				reset_zones |= Register_Subzone_Connections(cell);

				if (ittype == IsometricTileTypeClass::BridgeMiddle2 + 4) {
					Map[cell].Height += BRIDGE_CELL_HEIGHT;
					Map[Adjacent_Cell(cell, FACING_E)].Height += BRIDGE_CELL_HEIGHT;
					Map[Adjacent_Cell(cell, FACING_W)].Height += BRIDGE_CELL_HEIGHT;

					for (int y = 0; y < 2; y++) {
						for (int x = -2; x < 3; x++) {
							cells.Add(Cell(cell + Cell(x, y)));
						}
					}
				}
			}
		}

		length++;
	}

	/*
	 * If the entire span was scanned without finding the end piece there is
	 * nothing more to do.
	 */
	if (length == 30) {
		return;
	}

	/*
	 * An end piece was found. Re-walk the span placing the bridge-piece
	 * overlay on each undamaged middle cell, and accumulate the dirty
	 * rectangle for the caller.
	 */
	cellptr = &Map[cptr->Fetch_CellID()];

	if (dirty != NULL) {
		Coord coord = Coord(cptr->Fetch_CellID(), LEVEL_LEPTON_H * cptr->Height);
		Point2D point;
		TacticalMap->Coord_To_Pixel(coord, point);
		dirty->X = point.X;
		dirty->Y = point.Y;
	}

	if (length > 0) {
		int remaining = length;
		do {
			int ittype = cellptr->ITType - IsometricTileTypeClass::TrainBridgeSet + 1;
			if ((ittype != IsometricTileTypeClass::BridgeMiddle1 || (cellptr->SubTile & 1) != 0) && (ittype != IsometricTileTypeClass::BridgeMiddle2 || (unsigned char)cellptr->SubTile >= 5)) {
				if (dir == FACING_E) {
					new OverlayClass(OverlayTypes[OVERLAY_RAIL_BRIDGE1], cellptr->Fetch_CellID(), HOUSE_NONE);
				} else {
					new OverlayClass(OverlayTypes[OVERLAY_RAIL_BRIDGE2], cellptr->Fetch_CellID(), HOUSE_NONE);
				}
			}

			Cell newcell = Adjacent_Cell(cellptr->Fetch_CellID(), dir);
			cell = newcell;
			cellptr = &Map[newcell];

			remaining--;
		} while (remaining != 0);
	}

	/*
	 * Finalize the dirty rectangle from the two span endpoints.
	 */
	if (dirty != NULL) {
		Coord coord = Coord(cell, LEVEL_LEPTON_H * Map[cell].Height);
		Point2D point;
		TacticalMap->Coord_To_Pixel(coord, point);

		int x1 = dirty->X;
		int y1 = dirty->Y;
		int minx = dirty->X;
		if (dirty->X >= point.X) {
			minx = point.X;
		}
		int miny = dirty->Y;
		dirty->X = minx - 64;
		if (y1 >= point.Y) {
			miny = point.Y;
		}
		dirty->Y = miny - 64;
		dirty->Width = abs(x1 - point.X) + 128;
		dirty->Height = abs(y1 - point.Y) + 128;
	}

	if (reset_zones) {
		Zone_Reset();
	}

	if (cells.Count() != 0) {
		Recalc_Cells_In_List(cells);
	}
}


/// <summary>
/// Repairs every high bridge on the map.
/// The map is combed for bridge anchors and each span found is handed to
/// Repair_High_Bridge_Span, with any collapsed center pieces raised back up on the way.
/// </summary>
void MapClass::Repair_All_High_Bridges(void)
{
	for (int y = MapRect.Y; y < MapRect.Y + MapRect.Height; y++) {
		for (int x = MapRect.X; x < MapRect.X + MapRect.Width; x++) {
			Cell cell(x, y);

			/*
			 * Skip cells that have no allocated cell object in this map's array.
			 */
			if (Array[(short)x + (short)y * MAP_CELL_W] == NULL) {
				continue;
			}
			CellClass * cptr = &Map[cell];

			int bridgetile = cptr->ITType - IsometricTileTypeClass::BridgeSet + 1;

			/*
			 * East-west bridge, top-left end piece.
			 */
			if ((bridgetile == IsometricTileTypeClass::BridgeTopLeft1 || bridgetile == IsometricTileTypeClass::BridgeTopLeft2) && cptr->SubTile == 8) {
				Repair_High_Bridge_Span(cptr, FACING_E);
			}

			/*
			 * East-west bridge, middle pieces.
			 */
			if ((bridgetile == IsometricTileTypeClass::BridgeMiddle1 || bridgetile == IsometricTileTypeClass::BridgeMiddle1 + 3 || bridgetile == IsometricTileTypeClass::BridgeMiddle1 + 4 || bridgetile == IsometricTileTypeClass::BridgeMiddle1 + 1 || bridgetile == IsometricTileTypeClass::BridgeMiddle1 + 2) && cptr->SubTile == 5) {

				if (bridgetile == IsometricTileTypeClass::BridgeMiddle1 + 4) {
					Set_Bridge_Middle_State(cell, IsometricTileType(IsometricTileTypeClass::BridgeMiddle1 + IsometricTileTypeClass::BridgeSet - 1), ISOTILE_INVALID, -1, false);

					Cell base = Adjacent_Cell(cell, FACING_W);
					Map[base].Height += BRIDGE_CELL_HEIGHT;
					Map[Adjacent_Cell(base, FACING_N)].Height += BRIDGE_CELL_HEIGHT;
					Map[Adjacent_Cell(base, FACING_S)].Height += BRIDGE_CELL_HEIGHT;
				}

				Repair_High_Bridge_Span(cptr, FACING_E);
			}

			/*
			 * South bridge, top-right end piece.
			 */
			if ((bridgetile == IsometricTileTypeClass::BridgeTopRight1 || bridgetile == IsometricTileTypeClass::BridgeTopRight2) && cptr->SubTile == 12) {
				Repair_High_Bridge_Span(cptr, FACING_S);
			}

			/*
			 * South bridge, middle pieces.
			 */
			if ((bridgetile == IsometricTileTypeClass::BridgeMiddle2 || bridgetile == IsometricTileTypeClass::BridgeMiddle2 + 3 || bridgetile == IsometricTileTypeClass::BridgeMiddle2 + 4 || bridgetile == IsometricTileTypeClass::BridgeMiddle2 + 2 || bridgetile == IsometricTileTypeClass::BridgeMiddle2 + 1) && cptr->SubTile == 7) {

				if (bridgetile == IsometricTileTypeClass::BridgeMiddle2 + 4) {
					Set_Bridge_Middle_State(cell, IsometricTileType(IsometricTileTypeClass::BridgeMiddle2 + IsometricTileTypeClass::BridgeSet - 1), ISOTILE_INVALID, -1, false);

					Cell base = Adjacent_Cell(cell, FACING_N);
					Map[base].Height += BRIDGE_CELL_HEIGHT;
					Map[Adjacent_Cell(base, FACING_E)].Height += BRIDGE_CELL_HEIGHT;
					Map[Adjacent_Cell(base, FACING_W)].Height += BRIDGE_CELL_HEIGHT;
				}

				Repair_High_Bridge_Span(cptr, FACING_S);
			}
		}
	}
}

/// <summary>
/// Repairs the high bridge nearest to the cell specified.
/// The search works outward from the cell until it meets a bridge anchor, and that one span
/// is put back together. Use this routine when the bridge to mend is known only by
/// something standing near it.
/// </summary>
/// <param name="cell">The cell to search outward from.</param>
void MapClass::Repair_High_Bridge_From_Cell(Cell const & cell)
{
	int radius = 1;

	while (true) {

		int extent = MapRect.Width;
		if (extent <= MapRect.Height) {
			extent = MapRect.Height;
		}
		if (radius >= extent) {
			break;
		}

		for (int dy = -radius; dy < radius; dy++) {
			for (int dx = -radius; dx < radius; dx++) {

				Cell candidate = cell + Cell(dx, dy);

				if (candidate.X >= MapRect.X && candidate.X <= MapRect.X + MapRect.Width &&
					candidate.Y >= MapRect.Y && candidate.Y <= MapRect.Y + MapRect.Height) {

					int idx = candidate.X + candidate.Y * MAP_CELL_W;
					if (Array[idx] != NULL) {

						CellClass * cptr = &Map[candidate];

						int bridgetile = cptr->ITType - IsometricTileTypeClass::BridgeSet + 1;

						/*
						 * East-west bridge, top-left end piece.
						 */
						if ((bridgetile == IsometricTileTypeClass::BridgeTopLeft1 || bridgetile == IsometricTileTypeClass::BridgeTopLeft2) && cptr->SubTile == 8) {
							Set_Bridge_End_State(candidate, false, false);
							Repair_High_Bridge_Span(cptr, FACING_E);
							return;
						}

						/*
						 * East-west bridge, middle pieces.
						 */
						if ((bridgetile == IsometricTileTypeClass::BridgeMiddle1 || bridgetile == IsometricTileTypeClass::BridgeMiddle1 + 3 || bridgetile == IsometricTileTypeClass::BridgeMiddle1 + 4 || bridgetile == IsometricTileTypeClass::BridgeMiddle1 + 1 || bridgetile == IsometricTileTypeClass::BridgeMiddle1 + 2) && cptr->SubTile == 5) {

							if (bridgetile == IsometricTileTypeClass::BridgeMiddle1 + 4) {
								Set_Bridge_Middle_State(candidate, IsometricTileType(IsometricTileTypeClass::BridgeMiddle1 + IsometricTileTypeClass::BridgeSet - 1), ISOTILE_INVALID, -1, false);

								Cell base = Adjacent_Cell(candidate, FACING_W);
								Map[base].Height += BRIDGE_CELL_HEIGHT;
								Map[Adjacent_Cell(base, FACING_N)].Height += BRIDGE_CELL_HEIGHT;
								Map[Adjacent_Cell(base, FACING_S)].Height += BRIDGE_CELL_HEIGHT;
							}

							Repair_High_Bridge_Span(cptr, FACING_E);
							return;
						}

						/*
						 * South bridge, top-right end piece.
						 */
						if ((bridgetile == IsometricTileTypeClass::BridgeTopRight1 || bridgetile == IsometricTileTypeClass::BridgeTopRight2) && cptr->SubTile == 12) {
							Set_Bridge_End_State(candidate, false, false);
							Repair_High_Bridge_Span(cptr, FACING_S);
							return;
						}

						/*
						 * South bridge, middle pieces.
						 */
						if ((bridgetile == IsometricTileTypeClass::BridgeMiddle2 || bridgetile == IsometricTileTypeClass::BridgeMiddle2 + 3 || bridgetile == IsometricTileTypeClass::BridgeMiddle2 + 4 || bridgetile == IsometricTileTypeClass::BridgeMiddle2 + 2 || bridgetile == IsometricTileTypeClass::BridgeMiddle2 + 1) && cptr->SubTile == 7) {

							if (bridgetile == IsometricTileTypeClass::BridgeMiddle2 + 4) {
								Set_Bridge_Middle_State(candidate, IsometricTileType(IsometricTileTypeClass::BridgeMiddle2 + IsometricTileTypeClass::BridgeSet - 1), ISOTILE_INVALID, -1, false);

								Cell base = Adjacent_Cell(candidate, FACING_N);
								Map[base].Height += BRIDGE_CELL_HEIGHT;
								Map[Adjacent_Cell(base, FACING_E)].Height += BRIDGE_CELL_HEIGHT;
								Map[Adjacent_Cell(base, FACING_W)].Height += BRIDGE_CELL_HEIGHT;
							}

							Repair_High_Bridge_Span(cptr, FACING_S);
							return;
						}
					}
				}
			}
		}

		radius++;
	}
}

/// <summary>
/// Repairs the train bridge nearest to the cell specified.
/// This routine is the train bridge counterpart of Repair_High_Bridge_From_Cell. The search
/// works outward from the cell until it meets a bridge anchor, and that one span is put
/// back together.
/// </summary>
/// <param name="cell">The cell to search outward from.</param>
void MapClass::Repair_Train_Bridge_From_Cell(Cell const & cell)
{
	int radius = 1;

	while (true) {

		int extent = MapRect.Width;
		if (extent <= MapRect.Height) {
			extent = MapRect.Height;
		}
		if (radius >= extent) {
			break;
		}

		for (int dy = -radius; dy < radius; dy++) {
			for (int dx = -radius; dx < radius; dx++) {

				Cell candidate = cell + Cell(dx, dy);

				if (candidate.X >= MapRect.X && candidate.X <= MapRect.X + MapRect.Width &&
					candidate.Y >= MapRect.Y && candidate.Y <= MapRect.Y + MapRect.Height) {

					int idx = candidate.X + candidate.Y * MAP_CELL_W;
					if (Array[idx] != NULL) {

						CellClass * cptr = &Map[candidate];

						int bridgetile = cptr->ITType - IsometricTileTypeClass::TrainBridgeSet + 1;

						/*
						 * East-west train bridge, top-left end piece.
						 */
						if ((bridgetile == IsometricTileTypeClass::BridgeTopLeft1 || bridgetile == IsometricTileTypeClass::BridgeTopLeft2) && cptr->SubTile == 8) {
							Set_Bridge_End_State(candidate, false, false);
							Repair_Train_Bridge_Span(cptr, FACING_E, NULL);
							return;
						}

						/*
						 * East-west train bridge, middle pieces.
						 */
						if ((bridgetile == IsometricTileTypeClass::BridgeMiddle1 || bridgetile == IsometricTileTypeClass::BridgeMiddle1 + 3 || bridgetile == IsometricTileTypeClass::BridgeMiddle1 + 4 || bridgetile == IsometricTileTypeClass::BridgeMiddle1 + 1 || bridgetile == IsometricTileTypeClass::BridgeMiddle1 + 2) && cptr->SubTile == 5) {

							if (bridgetile == IsometricTileTypeClass::BridgeMiddle1 + 4) {
								Set_Bridge_Middle_State(candidate, IsometricTileType(IsometricTileTypeClass::BridgeMiddle1 + IsometricTileTypeClass::TrainBridgeSet - 1), ISOTILE_INVALID, -1, false);

								Cell base = Adjacent_Cell(candidate, FACING_W);
								Map[base].Height += BRIDGE_CELL_HEIGHT;
								Map[Adjacent_Cell(base, FACING_N)].Height += BRIDGE_CELL_HEIGHT;
								Map[Adjacent_Cell(base, FACING_S)].Height += BRIDGE_CELL_HEIGHT;
							}

							Repair_Train_Bridge_Span(cptr, FACING_E, NULL);
							return;
						}

						/*
						 * South train bridge, top-right end piece.
						 */
						if ((bridgetile == IsometricTileTypeClass::BridgeTopRight1 || bridgetile == IsometricTileTypeClass::BridgeTopRight2) && cptr->SubTile == 12) {
							Set_Bridge_End_State(candidate, false, false);
							Repair_Train_Bridge_Span(cptr, FACING_S, NULL);
							return;
						}

						/*
						 * South train bridge, middle pieces.
						 */
						if ((bridgetile == IsometricTileTypeClass::BridgeMiddle2 || bridgetile == IsometricTileTypeClass::BridgeMiddle2 + 3 || bridgetile == IsometricTileTypeClass::BridgeMiddle2 + 4 || bridgetile == IsometricTileTypeClass::BridgeMiddle2 + 2 || bridgetile == IsometricTileTypeClass::BridgeMiddle2 + 1) && cptr->SubTile == 7) {

							if (bridgetile == IsometricTileTypeClass::BridgeMiddle2 + 4) {
								Set_Bridge_Middle_State(candidate, IsometricTileType(IsometricTileTypeClass::BridgeMiddle2 + IsometricTileTypeClass::TrainBridgeSet - 1), ISOTILE_INVALID, -1, false);

								Cell base = Adjacent_Cell(candidate, FACING_N);
								Map[base].Height += BRIDGE_CELL_HEIGHT;
								Map[Adjacent_Cell(base, FACING_E)].Height += BRIDGE_CELL_HEIGHT;
								Map[Adjacent_Cell(base, FACING_W)].Height += BRIDGE_CELL_HEIGHT;
							}

							Repair_Train_Bridge_Span(cptr, FACING_S, NULL);
							return;
						}
					}
				}
			}
		}

		radius++;
	}
}


/***********************************************************************************************
 * MapClass::Write_Binary -- Pipes the map template data to the destination specified.         *
 *                                                                                             *
 *    This stores the template data from the map to the output pipe specified. The template    *
 *    data consists of the template type number and template icon number for every cell on     *
 *    the map. The output is organized in such a way so as to get maximum compression.         *
 *                                                                                             *
 * INPUT:   pipe  -- Reference to the output pipe that will receive the map template data.     *
 *                                                                                             *
 * OUTPUT:  Returns with the total number of bytes output to the pipe.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
/// <summary>
/// Writes the full map template data through an LCW-compressing pipe.
/// Outputs every cell's ITType, then every SubTile, then every Height in three passes.
/// </summary>
/// <param name="pipe">Destination pipe (wrapped in an LCWPipe compressor) receiving the data.</param>
/// <returns>Total number of bytes written to the pipe.</returns>
int MapClass::Write_Binary_1(Pipe & pipe)
{
	int total = 0;

	LCWPipe comp(LCWPipe::COMPRESS);
	comp.Put_To(&pipe);

	for (int i = 0; i < Array.Length(); i++) {
		CellClass * cellptr = Array[i];
		if (cellptr) {
			total += comp.Put(&cellptr->ITType, 2);
		}
		else {
			total += comp.Put(&BlubCell.ITType, 2);
		}
	}

	for (int j = 0; j < Array.Length(); j++) {
		CellClass * cellptr = Array[j];
		if (cellptr) {
			total += comp.Put(&cellptr->SubTile, 1);
		}
		else {
			total += comp.Put(&BlubCell.SubTile, 1);
		}
	}

	for (int k = 0; k < Array.Length(); k++) {
		CellClass * cellptr = Array[k];
		if (cellptr) {
			total += comp.Put(&cellptr->Height, 1);
		}
		else {
			total += comp.Put(&BlubCell.Height, 1);
		}
	}

	return(total);
}


/// <summary>
/// Writes the map cells out in the version 2 format.
/// This is the LCW compressed flavor of the map, and the terminator is padded out with a
/// run of empty cells. Only cells with something worth keeping are written out.
/// </summary>
/// <param name="pipe">The pipe to send the compressed map data to.</param>
/// <returns>Returns with the number of bytes written to the pipe.</returns>
int MapClass::Write_Binary_2(Pipe & pipe)
{
	int total = 0;

	LCWPipe comp(LCWPipe::COMPRESS);
	comp.Put_To(&pipe);

	int i;
	for (i = 0; i < Array.Length(); i++) {
		CellClass * cellptr = Array[i];
		if (i != 0 && cellptr) {
			if (In_Radar(cellptr->Fetch_CellID())) {
				if (cellptr->Fetch_CellID() != CELL_NONE && cellptr != &BlubCell && (cellptr->ITType != ISOTILE_NONE || cellptr->Height > 0)) {
					Cell cell = cellptr->Fetch_CellID();
					total += comp.Put(&cell, sizeof(cell));
					total += comp.Put(&cellptr->ITType, sizeof(cellptr->ITType));
					total += comp.Put(&cellptr->SubTile, sizeof(cellptr->SubTile));
					total += comp.Put(&cellptr->Height, sizeof(cellptr->Height));
				}
			}
		}
	}

	Cell empty = CELL_NONE;
	total += comp.Put(&empty, sizeof(empty));
	for (i = 0; i < 100; i++) {
		total += comp.Put(&empty, sizeof(empty));
	}
	comp.End();

	return(total);
}


/// <summary>
/// Writes the map cells out in the version 3 format.
/// This is the plain uncompressed flavor of the map. Only cells with something worth keeping
/// are written out.
/// </summary>
/// <param name="pipe">The pipe to send the map data to.</param>
/// <returns>Returns with the number of bytes written to the pipe.</returns>
int MapClass::Write_Binary_3(Pipe & pipe)
{
	int total = 0;

	for (int i = 0; i < Array.Length(); i++) {
		CellClass * cellptr = Array[i];
		if (i != 0 && cellptr) {
			if (In_Radar(cellptr->Fetch_CellID())) {
				if (cellptr->Fetch_CellID() != CELL_NONE && cellptr != &BlubCell && (cellptr->ITType != ISOTILE_NONE || cellptr->Height > 0)) {
					Cell cell = cellptr->Fetch_CellID();
					total += pipe.Put(&cell, sizeof(cell));
					total += pipe.Put(&cellptr->ITType, sizeof(cellptr->ITType));
					total += pipe.Put(&cellptr->SubTile, sizeof(cellptr->SubTile));
					total += pipe.Put(&cellptr->Height, sizeof(cellptr->Height));
				}
			}
		}
	}

	Cell empty = CELL_NONE;
	total += pipe.Put(&empty, sizeof(empty));

	return(total);
}


/// <summary>
/// Writes the map cells out in the version 4 format.
/// This is the LZO compressed flavor of the map -- version 5 without the per cell ice
/// growth flag. Only cells with something worth keeping are written out.
/// </summary>
/// <param name="pipe">The pipe to send the compressed map data to.</param>
/// <returns>Returns with the number of bytes written to the pipe.</returns>
int MapClass::Write_Binary_4(Pipe & pipe)
{
	LZOPipe comp(LZOPipe::COMPRESS);
	comp.Put_To(&pipe);
	int total = 0;

	for (int i = 0; i < Array.Length(); i++) {
		CellClass * cellptr = Array[i];
		if (i != 0 && cellptr) {
			if (In_Radar(cellptr->Fetch_CellID())) {
				if (cellptr->Fetch_CellID() != CELL_NONE && cellptr != &BlubCell && (cellptr->ITType != ISOTILE_NONE || cellptr->Height > 0)) {
					Cell cell = cellptr->Fetch_CellID();
					total += comp.Put(&cell, sizeof(cell));
					total += comp.Put(&cellptr->ITType, sizeof(cellptr->ITType));
					total += comp.Put(&cellptr->SubTile, sizeof(cellptr->SubTile));
					total += comp.Put(&cellptr->Height, sizeof(cellptr->Height));
				}
			}
		}
	}

	Cell empty = CELL_NONE;
	total += comp.Put(&empty, sizeof(empty));
	total += comp.End();

	return(total);
}


/// <summary>
/// Writes the map cells out in the version 5 format.
/// This is the newest of the map formats -- LZO compressed, and the only one that carries
/// each cell's ice growth flag. Only cells with something worth keeping are written out.
/// </summary>
/// <param name="pipe">The pipe to send the compressed map data to.</param>
/// <returns>Returns with the number of bytes written to the pipe.</returns>
int MapClass::Write_Binary_5(Pipe & pipe)
{
	LZOPipe comp(LZOPipe::COMPRESS);
	comp.Put_To(&pipe);
	int total = 0;

	for (int i = 0; i < Array.Length(); i++) {
		CellClass * cellptr = Array[i];
		if (i != 0 && cellptr) {
			if (In_Radar(cellptr->Fetch_CellID())) {
				if (cellptr->Fetch_CellID() != CELL_NONE && cellptr != &BlubCell && (cellptr->ITType != ISOTILE_NONE || cellptr->Height > 0)) {
					Cell cell = cellptr->Fetch_CellID();
					total += comp.Put(&cell, sizeof(cell));
					total += comp.Put(&cellptr->ITType, sizeof(cellptr->ITType));
					total += comp.Put(&cellptr->SubTile, sizeof(cellptr->SubTile));
					total += comp.Put(&cellptr->Height, sizeof(cellptr->Height));
					total += comp.Put(&cellptr->IsIceGrowthAllowed, sizeof(cellptr->IsIceGrowthAllowed));
				}
			}
		}
	}

	Cell empty = CELL_NONE;
	total += comp.Put(&empty, sizeof(empty));
	total += comp.End();

	return(total);
}


/***********************************************************************************************
 * MapClass::Read_Binary -- Reads the binary data from the straw specified.                    *
 *                                                                                             *
 *    This routine will retrieve the map template data from the straw specified.               *
 *                                                                                             *
 * INPUT:   straw -- Reference to the straw that will supply the map template data.            *
 *                                                                                             *
 * OUTPUT:  bool; Was the template data retrieved?                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool MapClass::Read_Binary_1(Straw & straw)
{
	LCWStraw decomp(LCWStraw::DECOMPRESS);
	decomp.Get_From(&straw);

	int i;

	for (i = 0; i < MAP_CELL_TOTAL/16; i++) {
		CellClass * cellptr = &(*this)[Cell(i % 128, i / 128)];
		if (cellptr != NULL) {
			cellptr->ITType = ISOTILE_NONE;
			decomp.Get(&cellptr->ITType, sizeof(cellptr->ITType)/2);
			cellptr->ITType = IsometricTileTypeClass::Fixup_Tile_Type(cellptr->ITType);
		} else {
			decomp.Get(&BlubCell.ITType, sizeof(BlubCell.ITType)/2);
		}
	}

	for (i = 0; i < MAP_CELL_TOTAL/16; i++) {
		CellClass * cellptr = &(*this)[Cell(i % 128, i / 128)];
		if (cellptr != NULL) {
			decomp.Get(&cellptr->SubTile, sizeof(cellptr->SubTile));
		} else {
			decomp.Get(&BlubCell.SubTile, sizeof(BlubCell.SubTile));
		}
	}

	for (i = 0; i < MAP_CELL_TOTAL/16; i++) {
		CellClass * cellptr = &(*this)[Cell(i % 128, i / 128)];
		if (cellptr != NULL) {
			decomp.Get(&cellptr->Height, sizeof(cellptr->Height));
		} else {
			decomp.Get(&BlubCell.Height, sizeof(BlubCell.Height));
		}
	}

	new (&BlubCell) CellClass;
	return(true);
}


/// <summary>
/// Reads the map cells from a version 2 saved map.
/// This format is LCW compressed rather than LZO. Cells the map no longer has room for are
/// read past and thrown away.
/// </summary>
/// <param name="straw">The straw supplying the compressed map data.</param>
/// <returns>bool; Was the map read? This routine always succeeds.</returns>
bool MapClass::Read_Binary_2(Straw & straw)
{
	LCWStraw decomp(LCWStraw::DECOMPRESS);
	decomp.Get_From(&straw);

	Cell cell;
	decomp.Get(&cell, sizeof(cell));

	while (cell != CELL_NONE) {
		CellClass * cellptr = &(*this)[cell];
		if (cellptr != NULL && cellptr != &BlubCell) {
			cellptr->ITType = ISOTILE_NONE;
			decomp.Get(&cellptr->ITType, sizeof(cellptr->ITType));
			cellptr->ITType = IsometricTileTypeClass::Fixup_Tile_Type(cellptr->ITType);
			decomp.Get(&cellptr->SubTile, sizeof(cellptr->SubTile));
			decomp.Get(&cellptr->Height, sizeof(cellptr->Height));
		} else {
			char tmp[sizeof(cellptr->ITType) + sizeof(cellptr->SubTile) + sizeof(cellptr->Height)];
			decomp.Get(&tmp, sizeof(tmp));
		}
		cell = CELL_NONE;
		decomp.Get(&cell, sizeof(cell));
	}
	new (&BlubCell) CellClass;
	return(true);
}


/// <summary>
/// Reads the map cells from a version 3 saved map.
/// This is the plain uncompressed flavor of the map. Cells the map no longer has room for are
/// read past and thrown away.
/// </summary>
/// <param name="straw">The straw supplying the map data.</param>
/// <returns>bool; Was the map read? This routine always succeeds.</returns>
bool MapClass::Read_Binary_3(Straw & straw)
{
	Cell cell;
	straw.Get(&cell, sizeof(cell));

	while (cell != CELL_NONE) {
		CellClass * cellptr = &(*this)[cell];
		if (cellptr != NULL && cellptr != &BlubCell) {
			cellptr->ITType = ISOTILE_NONE;
			straw.Get(&cellptr->ITType, sizeof(cellptr->ITType));
			cellptr->ITType = IsometricTileTypeClass::Fixup_Tile_Type(cellptr->ITType);
			straw.Get(&cellptr->SubTile, sizeof(cellptr->SubTile));
			straw.Get(&cellptr->Height, sizeof(cellptr->Height));
		} else {
			char tmp[sizeof(cellptr->ITType) + sizeof(cellptr->SubTile) + sizeof(cellptr->Height)];
			straw.Get(&tmp, sizeof(tmp));
		}
		cell = CELL_NONE;
		straw.Get(&cell, sizeof(cell));
	}
	new (&BlubCell) CellClass;
	return(true);
}


/// <summary>
/// Reads the map cells from a version 4 saved map.
/// This is the LZO compressed format that came before the ice growth flag was recorded.
/// Cells the map no longer has room for are read past and thrown away.
/// </summary>
/// <param name="straw">The straw supplying the compressed map data.</param>
/// <returns>bool; Was the map read? False if the data was damaged or ended before its
/// terminator, in which case the cells read before that point are still in place.</returns>
bool MapClass::Read_Binary_4(Straw & straw)
{
	LZOStraw decomp(LZOStraw::DECOMPRESS);
	decomp.Get_From(&straw);

	// A pack that runs out before its CELL_NONE terminator was cut short, even when every
	// block it did carry decompressed.
	bool terminated = false;
	Cell cell;

	while (decomp.Get(&cell, sizeof(cell)) == sizeof(cell)) {
		if (cell == CELL_NONE) {
			terminated = true;
			break;
		}
		CellClass * cellptr = &(*this)[cell];
		if (cellptr != NULL && cellptr != &BlubCell) {
			cellptr->ITType = ISOTILE_NONE;
			decomp.Get(&cellptr->ITType, sizeof(cellptr->ITType));
			cellptr->ITType = IsometricTileTypeClass::Fixup_Tile_Type(cellptr->ITType);
			decomp.Get(&cellptr->SubTile, sizeof(cellptr->SubTile));
			decomp.Get(&cellptr->Height, sizeof(cellptr->Height));
		} else {
			char tmp[sizeof(cellptr->ITType) + sizeof(cellptr->SubTile) + sizeof(cellptr->Height)];
			decomp.Get(&tmp, sizeof(tmp));
		}
	}
	new (&BlubCell) CellClass;
	return(terminated && !decomp.Is_Damaged());
}


/// <summary>
/// Reads the map cells from a version 5 saved map.
/// This is the newest of the map formats -- LZO compressed, and the only one that carries
/// each cell's ice growth flag. Cells the map no longer has room for are read past and
/// thrown away.
/// </summary>
/// <param name="straw">The straw supplying the compressed map data.</param>
/// <returns>bool; Was the map read? False if the data was damaged or ended before its
/// terminator, in which case the cells read before that point are still in place.</returns>
bool MapClass::Read_Binary_5(Straw & straw)
{
	LZOStraw decomp(LZOStraw::DECOMPRESS);
	decomp.Get_From(&straw);

	// A pack that runs out before its CELL_NONE terminator was cut short, even when every
	// block it did carry decompressed.
	bool terminated = false;
	Cell cell;

	while (decomp.Get(&cell, sizeof(cell)) == sizeof(cell)) {
		if (cell == CELL_NONE) {
			terminated = true;
			break;
		}
		CellClass * cellptr = &(*this)[cell];
		if (cellptr != NULL && cellptr != &BlubCell) {
			cellptr->ITType = ISOTILE_NONE;
			decomp.Get(&cellptr->ITType, sizeof(cellptr->ITType));
			cellptr->ITType = IsometricTileTypeClass::Fixup_Tile_Type(cellptr->ITType);
			decomp.Get(&cellptr->SubTile, sizeof(cellptr->SubTile));
			decomp.Get(&cellptr->Height, sizeof(cellptr->Height));
			decomp.Get(&cellptr->IsIceGrowthAllowed, sizeof(cellptr->IsIceGrowthAllowed));
		} else {
			char tmp[sizeof(cellptr->ITType) + sizeof(cellptr->SubTile) + sizeof(cellptr->Height) + sizeof(cellptr->IsIceGrowthAllowed)];
			decomp.Get(&tmp, sizeof(tmp));
		}
	}
	new (&BlubCell) CellClass;
	return(terminated && !decomp.Is_Damaged());
}


/***********************************************************************************************
 * MapClass::Logic -- Handles map related logic functions.                                     *
 *                                                                                             *
 *    Manages tiberium growth and spread.                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/11/1995 JLB : Created.                                                                 *
 *   07/09/1995 JLB : Handles two directional scan.                                            *
 *   08/01/1995 JLB : Gives stronger weight to blossom trees.                                  *
 *=============================================================================================*/
void MapClass::Logic(void)
{
	/*
	**	Crate regeneration is handled here.
	*/
	if (Session.Type != GAME_NORMAL && Session.Options.Goodies) {

		/*
		**	Find any crate that has expired and then regenerate it at a new
		**	spot.
		*/
		for (int index = 0; index < ARRAY_SIZE(Crates); index++) {
			if (Crates[index].Is_Expired()) {
				Crates[index].Remove_It();
				Place_Random_Crate();
			}
		}
	}
}


/***********************************************************************************************
 * MapClass::Cell_Region -- Determines the region from a specified cell number.                *
 *                                                                                             *
 *    Use this routine to determine what region a particular cell lies in.                     *
 *                                                                                             *
 * INPUT:   cell  -- The cell number to examine.                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the region that the specified cell occupies.                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/15/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int MapClass::Cell_Region(Cell const & cell)
{
	return((cell.X / REGION_WIDTH) + 1) + (((cell.Y / REGION_HEIGHT) + 1) * MAP_REGION_WIDTH);
}


/***************************************************************************
 * MapClass::Cell_Threat -- Gets a houses threat value for a cell          *
 *                                                                         *
 * INPUT:   CELL        cell    - the cell number to check                 *
 *            HouseType house   - the house to check                       *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   04/25/1995 PWG : Created.                                             *
 *=========================================================================*/
int MapClass::Cell_Threat(Cell const & cell, HouseClass const & house)
{
	int threat = house.Regions[Map.Cell_Region(Map[cell].Fetch_CellID())].Threat_Value();

#if NEVER
	if (!threat && Map[cell].IsVisible) {
		threat = 1;
	}
#endif
	return(threat);
}


/***********************************************************************************************
 * MapClass::Place_Random_Crate -- Places a crate at random location on map.                   *
 *                                                                                             *
 *    This routine will place a crate at a random location on the map. This routine will only  *
 *    make a limited number of attempts to place and if unsuccessful, it will not place any.   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Was a crate successfully placed?                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool MapClass::Place_Random_Crate(void)
{
	/*
	**	Find a crate index that is free for assignment. If there are
	**	no free slots, then return with failure to place crate.
	*/
	int crateindex = 0;
	for (crateindex = 0; crateindex < ARRAY_SIZE(Crates); crateindex++) {
		if (!Crates[crateindex].Is_Valid()) break;
	}
	if (crateindex == ARRAY_SIZE(Crates)) {
		return(false);
	}

	/*
	**	Give a good effort to scan for and place a crate down on the map.
	*/
	for (int index = 0; index < 1000; index++) {
		Cell cell = Map.Pick_Random_Location();

		if (Crates[crateindex].Create_Crate(cell)) {
			return(true);
		}
	}
	return(false);
}


/***********************************************************************************************
 * MapClass::Remove_Crate -- Remove a crate from the specified cell.                           *
 *                                                                                             *
 *    This will examine the cell and remove any crates there.                                  *
 *                                                                                             *
 * INPUT:   cell  -- The cell to examine for crates and remove from.                           *
 *                                                                                             *
 * OUTPUT:  bool; Was a crate found at the location specified and was it removed?              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/26/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool MapClass::Remove_Crate(Cell const & cell)
{
	TacticalMap->Flag_Cell(Map[cell]);

	if (Session.Type != GAME_NORMAL) {
		for (int index = 0; index < ARRAY_SIZE(Crates); index++) {
			if (Crates[index].Is_Here(cell)) {
				return(Crates[index].Remove_It());
			}
		}
	}

//	if (Session.Type == GAME_NORMAL) {
		CellClass * cellptr = &(*this)[cell];
		if (cellptr->Overlay != OVERLAY_NONE && OverlayTypes[cellptr->Overlay]->IsCrate) {
			Rect dirty = Union(cellptr->Overlay_Render_Rect(), cellptr->Overlay_Shadow_Render_Rect());
			dirty.Y -= TacticalRect.Y;
			TacticalMap->Register_Dirty_Area(dirty);
			cellptr->Overlay = OVERLAY_NONE;
			cellptr->OverlayData = 0;
			return(true);
		}
//	} else {
//		for (int index = 0; index < ARRAY_SIZE(Crates); index++) {
//			if (Crates[index].Is_Here(cell)) {
//				return(Crates[index].Remove_It());
//			}
//		}
//	}

	return(false);
}


/***************************************************************************
 * MapClass::Validate -- validates every cell on the map                   *
 *                                                                         *
 * This is a debugging routine, designed to detect memory trashers that    *
 * alter the map.  This routine is slow, but thorough.                     *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      true = map is OK, false = an error was found                       *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   07/08/1995 BRR : Created.                                             *
 *=========================================================================*/
int MapClass::Validate(void)
{
#if NEVER
	CELL cell;
	TemplateType ttype;
	unsigned char ticon;
	TemplateTypeClass const *tclass;
	unsigned char map[13*8];
	OverlayType overlay;
	SmudgeType smudge;
	ObjectClass * obj;
	LandType land;
	int i;

	BlubCell = &Array[797];

	if (BlubCell->Overlapper[1]) {
		obj = BlubCell->Overlapper[1];
		if (obj) {
			if (obj->IsInLimbo)
			obj = obj;
		}
	}

	/*
	**	Check every cell on the map, even those that aren't displayed,
	**	in the hopes of detecting a memory trasher.
	*/
	for (cell = 0; cell < MAP_CELL_TOTAL; cell++) {
		/*
		**	Validate Template & Icon data
		*/
		ttype = (*this)[cell].TType;
		ticon = (*this)[cell].TIcon;
		if (ttype >= TEMPLATE_COUNT && ttype != TEMPLATE_NONE)
			return(false);

		/*
		**	To validate the icon value, we have to get a copy of the template's
		**	"icon map"; this map will have 0xff's in spots where there is no
		**	icon.  If the icon value is out of range or points to an invalid spot,
		**	return an error.
		*/
		if (ttype != TEMPLATE_NONE) {
			tclass = TemplateTypes[ttype];
			ticon = (*this)[cell].TIcon;
			Mem_Copy(Get_Icon_Set_Map(tclass->Get_Image_Data()), map, tclass->Width * tclass->Height);
			if (ticon < 0 || ticon >= (tclass->Width * tclass->Height) || map[ticon] == ISOTILE_NONE_LEGACY) {
				return(false);
			}
		}

		/*
		**	Validate Overlay
		*/
		overlay = (*this)[cell].Overlay;
		if (overlay < OVERLAY_NONE || overlay >= OVERLAY_COUNT) {
			return(false);
		}

		/*
		**	Validate Smudge
		*/
		smudge = (*this)[cell].Smudge;
		if (smudge < SMUDGE_NONE || smudge >= SMUDGE_COUNT) {
			return(false);
		}

		/*
		**	Validate LandType
		*/
		land = (*this)[cell].Land_Type();
		if (land < LAND_CLEAR || land >= LAND_COUNT) {
			return(false);
		}

		/*
		**	Validate Occupier
		*/
		obj = (*this)[cell].Cell_Occupier();
		if (obj) {
			if (
				((unsigned int)obj & 0xff000000) ||
				((unsigned int)obj->Next & 0xff000000) ||
//				((unsigned int)obj->Trigger & 0xff000000) ||
				obj->IsInLimbo ||
				((unsigned int)obj->PositionCell >= MAP_CELL_TOTAL)) {

				return(false);
			}
		}

		/*
		**	Validate Overlappers
		*/
		for (i = 0; i < ARRAY_SIZE((*this)[cell].CellClass::Overlapper); i++) {
			obj = (*this)[cell].Overlapper[i];
			if (obj) {
				if (
					((unsigned int)obj & 0xff000000) ||
					((unsigned int)obj->Next & 0xff000000) ||
//					((unsigned int)obj->Trigger & 0xff000000) ||
					obj->IsInLimbo ||
					((unsigned int)obj->PositionCell >= MAP_CELL_TOTAL)) {

					return(false);
				}
			}
		}
	}
#endif
	return(true);
}


/***********************************************************************************************
 * MapClass::Close_Object -- Finds a clickable close object to the specified coordinate.       *
 *                                                                                             *
 *    This routine is used by the mouse input processing code to find a clickable object       *
 *    close to coordinate specified. This is for targeting as well as selection determination. *
 *                                                                                             *
 * INPUT:   coord -- The coordinate to scan for close object from.                             *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to an object that is nearby the specified coordinate.       *
 *                                                                                             *
 * WARNINGS:   There could be a cloaked object at the location, but it won't be considered     *
 *             if it is not owned by the player.                                               *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/20/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectClass * MapClass::Close_Object(Coord const & coord) const
{
	ObjectClass * object = 0;
	int distance = 0;
	Cell cell = coord.As_Cell();

	/*
	**	Scan through current and adjacent cells, looking for the
	**	closest object (within reason) to the specified coordinate.
	*/
	static Cell _offsets[] = {Cell(0,0), Cell(-1,0), Cell(1,0), Cell(0,-1), Cell(0,1), Cell(-1,1), Cell(1,1), Cell(1,-1), Cell(-1,-1)};
	for (int index = 0; index < ARRAY_SIZE(_offsets); index++) {

		/*
		**	Examine the cell for close object. Make sure that the cell actually is a
		**	legal one.
		*/
		Cell newcell = cell + _offsets[index];
		if (In_Radar(newcell)) {

			/*
			**	Search through all objects that occupy this cell and then
			**	find the closest object. Check against any previously found object
			**	to ensure that it is actually closer.
			*/
			ObjectClass * o = (*this)[newcell].Cell_Occupier();
			while (o != NULL) {

				/*
				**	Special case check to ignore cloaked object if not owned by the player.
				*/
				TechnoClass * t = Dynamic_Cast<TechnoClass *>(o);
				if (!t || t->IsOwnedByPlayer || t->Cloak != CLOAKED) {
					int d=-1;
					if (o->RTTI == RTTI_BUILDING) {
						d = Distance(coord, (Coord)newcell);
						if (d > 0x00B5) d = -1;
					} else {
						d = Distance(coord, o->Center_Coord());
					}
					if (d >= 0 && (!object || d < distance)) {
						distance = d;
						object = o;
					}
				}
				o = o->Next;
			}
		}
	}

	/*
	**	Only return the object if it is within 1/4 cell distance from the specified
	**	coordinate.
	*/
	if (object && distance > 0xB5) {
		object = 0;
	}
	return(object);
}


/// <summary>
/// Stages a link between two subzones for the rebuild in progress to write out.
/// A rebuild stages the same pair many times and the first staging wins, so the cross block
/// flag is the one from the fill that reached the boundary first.
/// </summary>
static void Stage_Subzone_Link(SubzoneLinkStaging & staging, int subzone1, int subzone2, bool crossblock)
{
	staging.try_emplace(ZonePair((unsigned short)subzone1, (unsigned short)subzone2), crossblock);
}


/***********************************************************************************************
 * MapClass::Zone_Reset -- Resets all zone numbers to match the map.                           *
 *                                                                                             *
 *    This routine will rescan the map and fill in the zone values for each of the cells.      *
 *    All cells that are contiguous are given the same zone number.                            *
 *                                                                                             *
 * INPUT:   method   -- The method to recalculate the zones upon. If 1 then recalc non         *
 *                      crushable zone. If 2 then recalc crushable zone. If 3, then            *
 *                      recalc both zones.                                                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This is a time consuming routine. Call it as infrequently as possible. It must  *
 *             be called whenever something that would affect contiguousness occurs. Example:  *
 *             when a bridge is built or destroyed.                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/22/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int MapClass::Zone_Reset(void)
{
	int i;
	int j;
	int k;
	int bestzone = -1;
	int bestspan = -1;

	CellZoneStruct * end = &CellZones[CellZoneCount];

	DynamicVectorClass<PassabilityType> vec;
	vec.Set_Growth_Step(300);

	ZoneAdjacency.clear();

	for (i = 0; i < MZONE_COUNT; i++) {
		if (Zones[i] != NULL) {
			delete Zones[i];
			Zones[i] = NULL;
		}
	}

	CellZoneStruct * czone = CellZones;
	for (; czone < end; czone++) {
		czone->ZoneID = 0;
	}

	int zone = 1;			// Starting zone number.

	vec.Add(PASSABLE_OUTSIDE);

	int skip = 0;
	czone = CellZones;
	while (czone < end) {
		PassabilityType pass = (PassabilityType)czone->Passability;
		if (pass != PASSABLE_OUTSIDE && czone->ZoneID == 0) {
			LastAdjacentZone = 0;
			int span = Zone_Span(czone, zone, skip);
			if (span > bestspan) {
				bestzone = (unsigned short)zone;
				bestspan = span;
			}
			vec.Add(pass);
			zone++;
			czone += skip;
		} else {
			czone++;
		}
	}

	ZoneCount = (unsigned short)zone;

	for (i = ZoneConnections.Count() - 1; i >= 0; i--) {
		ZoneConnectionClass * connection = &ZoneConnections[i];
		if (connection->IsPassable) {
			int from_zone = Get_Cell_Zone_ID(connection->From);
			int to_zone = Get_Cell_Zone_ID(connection->To);
			if (to_zone != from_zone) {
				if (to_zone < from_zone) {
					std::swap(to_zone, from_zone);
				}
				ZoneAdjacency.emplace((unsigned short)from_zone, (unsigned short)to_zone);
			}
		}
	}

	unsigned short * zone_degree = new unsigned short[ZoneCount];
	for (i = 0; i < ZoneCount; i++) {
		zone_degree[i] = 0;
	}

	for (ZonePair const & pair : ZoneAdjacency) {
		zone_degree[pair.second]++;
		zone_degree[pair.first]++;
	}

	unsigned short ** zone_neighbors = new unsigned short *[ZoneCount];
	for (i = 0; i < ZoneCount; i++) {
		zone_neighbors[i] = new unsigned short[zone_degree[i]];
	}

	for (i = 0; i < ZoneCount; i++) {
		zone_degree[i] = 0;
	}

	for (ZonePair const & pair : ZoneAdjacency) {
		zone_neighbors[pair.second][zone_degree[pair.second]] = pair.first;
		zone_neighbors[pair.first][zone_degree[pair.first]] = pair.second;
		zone_degree[pair.second]++;
		zone_degree[pair.first]++;
	}

	unsigned char * zone_passability = new unsigned char[ZoneCount];
	for (i = 0; i < ZoneCount; i++) {
		zone_passability[i] = vec[i];
	}

	unsigned short * stack = new unsigned short[ZoneCount];

	for (MZoneType mzone = MZONE_FIRST; mzone < MZONE_COUNT; mzone++) {
		int next_movement_zone = 2;
		int * table = MZonePassability[mzone];

		unsigned short * nzone = new unsigned short[ZoneCount];
		Zones[mzone] = nzone;

		for (j = 0; j < ZoneCount; j++) {
			nzone[j] = table[zone_passability[j]] != TRAVERSAL_PASSABLE;
		}

		for (int zone_index = 0; zone_index < ZoneCount; zone_index++) {
			if (nzone[zone_index] == 0) {
				int stackcount = 1;
				stack[0] = zone_index;
				nzone[zone_index] = next_movement_zone;
				int passable = table[zone_passability[zone_index]];
				while (stackcount) {
					int current = stack[--stackcount];
					unsigned short * neighbors = zone_neighbors[current];
					int degree = zone_degree[current];
					for (k = degree - 1; k >= 0; k--) {
						int neighbor = neighbors[k];
						if (table[zone_passability[neighbor]] == passable && nzone[neighbor] == 0) {
							stack[stackcount++] = neighbor;
							nzone[neighbor] = next_movement_zone;
						}
					}
				}
				next_movement_zone++;
			}
		}

		nzone[0] = -1;
	}

	for (i = 0 ; i < ZoneCount; i++) {
		delete zone_neighbors[i];
	}

	delete [] zone_passability;
	delete [] zone_neighbors;
	delete [] zone_degree;
	delete [] stack;

	return(Zones[0][bestzone]);
}


/***********************************************************************************************
 * MapClass::Zone_Span -- Flood fills the specified zone from the cell origin.                 *
 *                                                                                             *
 *    This routine is used to fill a zone value into the map. The map is "flood filled" from   *
 *    the cell specified. All adjacent (8 connected) and generally passable terrain cells are  *
 *    given the zone number specified. This routine checks for legality before filling         *
 *    occurs. The routine is safe to call even if the legality of the cell is unknown at the   *
 *    time of the call.                                                                        *
 *                                                                                             *
 * INPUT:   cell  -- The cell to begin filling from.                                           *
 *                                                                                             *
 *          zone  -- The zone number to assign to all adjacent cells.                          *
 *                                                                                             *
 *          check -- The zone type to check against.                                           *
 *                                                                                             *
 * OUTPUT:  Returns with the number of cells marked by this routine.                           *
 *                                                                                             *
 * WARNINGS:   This routine is slow and recursive. Only use when necessary.                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/25/1995 JLB : Created.                                                                 *
 *   10/05/1996 JLB : Examines crushable walls.                                                *
 *=============================================================================================*/
int MapClass::Zone_Span(CellZoneStruct * data, int zone, int & skip)
{
	int passability = data->Passability;
	bool nopass = passability == PASSABLE_NO;
	int cell_height = data->Height;

	CellZoneStruct * begin = data;
	CellZoneStruct * end = data;

	/*
	**	Find the full extent of the current span by first scanning leftward
	**	until a boundary is reached.
	*/
	while (begin->Passability == passability) {
		if (abs(begin->Height - cell_height) >= 2) {
			break;
		}
		begin->ZoneID = zone;
		cell_height = begin->Height;
		begin--;
	}

	int begin_zone = begin->ZoneID;
	if (begin_zone != 0 && (abs(begin->Height - cell_height) < 2 || nopass) && begin_zone != LastAdjacentZone && begin_zone != (unsigned short)zone) {
		ZoneAdjacency.emplace((unsigned short)begin_zone, (unsigned short)zone);
		LastAdjacentZone = begin_zone;
	}

	/*
	 * Scan rightward until a boundary is reached. This will then define the
	 * extent of the current span. The end pointer advances, so its ZoneID is
	 * the active zone boundary; data remains fixed at the seed cell.
	 */
	while (end->Passability == passability) {
		if (abs(end->Height - cell_height) >= 4) {
			break;
		}
		end->ZoneID = zone;
		cell_height = end->Height;
		end++;
	}

	int end_zone = end->ZoneID;
	if (end_zone != 0 && (abs(end->Height - cell_height) < 2 || nopass) && end_zone != LastAdjacentZone && end_zone != (unsigned short)zone) {
		ZoneAdjacency.emplace((unsigned short)end_zone, (unsigned short)zone);
		LastAdjacentZone = end_zone;
	}

	int filled = end - begin - 1;
	begin++;
	int skip1 = 0;
	int skip2 = 0;
	skip = end - data - 1;
	end--;

	int stride = PlayRect.Height + PlayRect.Width + 1;
	int diag = stride + 1;

	CellZoneStruct * fbegin = begin - diag;
	CellZoneStruct * fbegin2 = begin + stride - 1;
	CellZoneStruct * fend2 = end + diag;
	CellZoneStruct * fend = end - stride + 1;

	/*
	**	At this point we know the bounds of the current span. Fill in the zone values
	**	for the entire span.
	*/
	while (fbegin <= fend) {
		int zzone = fbegin->ZoneID;
		CellZoneStruct * adjacent;
		if (fbegin < fend - 1) {
			adjacent = fbegin + diag;
		} else {
			adjacent = fbegin == fend - 1 ? fbegin + stride : fbegin - 1 + stride;
		}
		if (zzone == 0) {
			if (fbegin->Passability == passability && abs(fbegin->Height - adjacent->Height) < 2) {
				filled += Zone_Span(fbegin, zone, skip1);
				fbegin += skip1;
			} else {
				fbegin++;
			}
		} else {
			if (zzone != (unsigned short)zone && zzone != LastAdjacentZone && (abs(fbegin->Height - adjacent->Height) < 2 || nopass)) {
				ZoneAdjacency.emplace((unsigned short)zzone, (unsigned short)zone);
				LastAdjacentZone = zzone;
			}
			fbegin++;
		}
	}

	/*
	**	Now scan the upper and lower shadow rows. If any of these rows contain
	**	candidate cells, then recursively call the span process for them. Take
	**	note that the adjacent span scanning starts one cell wider on each
	**	end of the scan. This is necessary because diagonals are considered
	**	adjacent.
	*/
	while (fbegin2 <= fend2) {
		int id = fbegin2->ZoneID;
		CellZoneStruct * adjacent;
		if (fbegin2 < fend2 - 1) {
			adjacent = fbegin2 - stride + 1;
		} else {
			adjacent = fbegin2 == fend2 - 1 ? fbegin2 - stride : fbegin2 - diag;
		}
		if (id == 0) {
			if (fbegin2->Passability == passability && abs(fbegin2->Height - adjacent->Height) < 2) {
				filled += MapClass::Zone_Span(fbegin2, zone, skip2);
				fbegin2 += skip2;
			} else {
				fbegin2++;
			}
		} else {
			if (id != (unsigned short)zone && id != LastAdjacentZone && (abs(fbegin2->Height - adjacent->Height) < 2 || nopass)) {
				ZoneAdjacency.emplace((unsigned short)id, (unsigned short)zone);
				LastAdjacentZone = id;
			}
			fbegin2++;
		}
	}

	return(filled);
}


/// <summary>
/// Determines if two cells can be reached from one another.
/// This is the reachability test the unit and AI code leans on before committing to a move.
/// Ground out beyond the local radar is given the benefit of the doubt, since nothing out
/// there has been zoned yet.
/// </summary>
/// <param name="from">The cell being moved from.</param>
/// <param name="to">The cell being moved to.</param>
/// <param name="mzone">The movement zone layer to compare in. MZONE_NONE always agrees.</param>
/// <param name="from_bridge">Should the source cell answer for the bridge above it?</param>
/// <param name="to_bridge">Should the destination cell answer for the bridge above it?</param>
/// <param name="leavemap">Should a move heading off the local radar be allowed?</param>
/// <returns>bool; Are the two cells in the same zone?</returns>
bool MapClass::Is_Same_Cell_Zone(Cell const & from, Cell const & to, MZoneType mzone, bool from_bridge, bool to_bridge, bool leavemap)
{
	if (mzone == MZONE_NONE) {
		return(true);
	}

	bool flocal = Map.In_Local_Radar(from);
	bool finradar = Map.In_Radar(from);

	if (!flocal && finradar) {
		return(true);
	}

	bool tlocal = Map.In_Local_Radar(to);
	bool tinradar = Map.In_Radar(to);

	if (leavemap && flocal && !tlocal && tinradar) {
		return(true);
	}

	return(Get_Cell_Zone(from, mzone, from_bridge) == Get_Cell_Zone(to, mzone, to_bridge));
}


/// <summary>
/// Fetches the movement zone that a cell belongs to.
/// This routine is the workhorse behind every reachability question the unit code asks.
/// Ground beneath a bridge is a special case -- it can be reached from either bank -- so
/// when bridge resolution is asked for, the answer comes from the span above instead.
/// </summary>
/// <param name="mzone">The movement zone layer to look the cell up in.</param>
/// <param name="bridge">Should a cell under a bridge answer for the span above it?</param>
/// <returns>Returns with the zone the cell belongs to. An under-bridge cell with no
/// crossing recorded returns -1.</returns>
int MapClass::Get_Cell_Zone(Cell const & cell, MZoneType mzone, bool bridge)
{
	Cell const * p;
	if (bridge && Map[cell].IsUnderBridge) {
		int index = Zone_Connection_Index(cell, 1, 0);
		if (index != -1) {
			ZoneConnectionClass & connection = ZoneConnections[index];
			if (connection.IsPassable) {
				p = &connection.From;
			} else {
				CellClass * cptr = &Map[cell];
				FacingType facing = connection.From.X == connection.To.X ? FACING_S : FACING_E;
				while (cptr->IsUnderBridge) {
					cptr = &cptr->Adjacent_Cell(facing);
				}
				if ((cptr->Is_Tile_Bridge() || cptr->Is_Tile_Train_Bridge()) && cptr->Land_Type() != LAND_ROCK) {
					p = &connection.To;
				} else {
					p = &connection.From;
				}
			}
		} else {
			return(-1);
		}
	} else {
		p = &cell;
	}
	return(Zones[mzone][Get_Cell_Zone_ID(*p)]);
}


/// <summary>
/// Returns the raw ZoneID stored for the given cell.
/// Looks up CellZones using the cell's flat zone index.
/// </summary>
/// <param name="cell">The cell whose zone id is requested.</param>
/// <returns>The cell's ZoneID.</returns>
int MapClass::Get_Cell_Zone_ID(Cell const & cell)
{
	return(CellZones[Get_Cell_Zone_Index(cell)].ZoneID);
}


/// <summary>
/// Converts a cell coordinate into a flat index into the CellZones array.
/// Index = cell.X + cell.Y * (PlayRect.Height + PlayRect.Width + 1).
/// </summary>
/// <param name="cell">The cell to convert.</param>
/// <returns>The flat array index for the cell.</returns>
int MapClass::Get_Cell_Zone_Index(Cell const & cell)
{
	return(cell.X + cell.Y * (PlayRect.Height + PlayRect.Width + 1));
}


/// <summary>
/// Converts a cell coordinate into a flat index into the CellSubzones array.
/// Index = cell.X + cell.Y * (PlayRect.Height + PlayRect.Width + 1).
/// </summary>
/// <param name="cell">The cell to convert.</param>
/// <returns>The flat array index for the cell.</returns>
int MapClass::Get_Cell_Subzone_Index(Cell const & cell)
{
	return(cell.X + cell.Y * (PlayRect.Height + PlayRect.Width + 1));
}


/// <summary>
/// Incrementally re-assigns a single cell's normal zone from a passable neighbor.
/// If the surrounding topology has too many zone transitions, triggers a full Zone_Reset.
/// Baseline neighbor must be PASSABLE_LAND and the cell itself land-passable to join.
/// </summary>
/// <param name="cell">The cell whose zone assignment is being updated.</param>
void MapClass::Update_Cell_Zone(Cell const & cell)
{
	CellZoneStruct * cell_data = &CellZones[Get_Cell_Zone_Index(cell)];

	/*
	 * Calculate the offsets for accessing neighbors per facing in the flattened map array.
	 */
	int stride = Map.PlayRect.Width + Map.PlayRect.Height + 1;
	int offsets[FACING_COUNT] = {
		-(stride + 0),
		-(stride - 1),
		1,
		stride + 1,
		stride + 0,
		stride - 1,
		-1,
		-(stride + 1),
	};

	/*
	 * Don't assign a zone to cells outside the map.
	 */
	if (cell_data->Passability != PASSABLE_OUTSIDE) {

		/*
		 * Try to find a neighboring cell that's land passable (as a baseline).
		 */
		CellZoneStruct * land_neighbor = NULL;
		bool has_land_neighbor = false;
		for (FacingType i = FACING_FIRST; i < FACING_COUNT; i++) {
			land_neighbor = &cell_data[offsets[i]];
			if (land_neighbor->Passability == PASSABLE_LAND) {
				has_land_neighbor = true;
				break;
			}
		}

		/*
		 * If the cell neighbors another passable cell, we need to check the
		 * topology is not too complicated.
		 */
		if (has_land_neighbor && land_neighbor != NULL) {
			int transitions = 0;
			int prev_zone = 0;

			for (FacingType j = FACING_FIRST; j < FACING_COUNT; j++) {
				CellZoneStruct * neighbor = &cell_data[offsets[j]];
				if (Zones[MZONE_NORMAL][neighbor->ZoneID] != Zones[MZONE_NORMAL][prev_zone] && neighbor->Passability != PASSABLE_OUTSIDE) {
					prev_zone = neighbor->ZoneID;
					transitions++;
				}
			}

			/*
			 * If there are only a few transitions, join this cell to its passable neighbor's zone.
			 */
			if (transitions <= 3 && cell_data->Passability == PASSABLE_LAND) {
				cell_data->ZoneID = land_neighbor->ZoneID;
				return;
			}
		}

		/*
		 * If we've failed, recalculate the map's zones completely.
		 */
		Map.Zone_Reset();
	}
}


/// <summary>
/// Re-zones a single cell that has just been built upon.
/// This is the companion of Update_Cell_Zone, used when something was placed on the cell
/// rather than cleared from it -- the cell joins whichever neighbor now matches it instead
/// of having to find open ground. Surroundings too tangled to judge fall back on a full
/// Zone_Reset.
/// </summary>
/// <param name="cell">The cell whose zone needs revisiting.</param>
void MapClass::Update_Cell_Zone_Constructively(Cell const & cell)
{
	CellZoneStruct * cell_data = &CellZones[Get_Cell_Zone_Index(cell)];
	int passability = cell_data->Passability;

	/*
	 * Calculate the offsets for accessing neighbors per facing in the flattened map array.
	 */
	int stride = Map.PlayRect.Width + Map.PlayRect.Height + 1;
	int offsets[FACING_COUNT] = {
		-(stride + 0),
		-(stride - 1),
		1,
		stride + 1,
		stride + 0,
		stride - 1,
		-1,
		-(stride + 1),
	};

	/*
	 * Don't assign a zone to cells outside the map.
	 */
	if (passability != PASSABLE_OUTSIDE) {

		/*
		 * Try to find a neighboring cell that has the same passability.
		 */
		CellZoneStruct * land_neighbor = NULL;
		bool has_land_neighbor = false;
		for (FacingType i = FACING_FIRST; i < FACING_COUNT; i++) {
			land_neighbor = &cell_data[offsets[i]];
			if (land_neighbor->Passability == passability) {
				has_land_neighbor = true;
				break;
			}
		}

		/*
		 * If the cell neighbors another passable cell, we need to check the
		 * topology is not too complicated.
		 */
		if (has_land_neighbor && land_neighbor != NULL) {
			int transitions = 0;
			int prev_zone = 0;

			for (FacingType j = FACING_FIRST; j < FACING_COUNT; j++) {
				if (Zones[MZONE_NORMAL][cell_data[offsets[j]].ZoneID] != Zones[MZONE_NORMAL][prev_zone] && cell_data[offsets[j]].Passability != PASSABLE_OUTSIDE) {
					prev_zone = cell_data[offsets[j]].ZoneID;
					transitions++;
				}
			}

			/*
			 * If there are only a few transitions, join this cell to its passable neighbor's zone.
			 */
			if (transitions <= 3) {
				cell_data->ZoneID = land_neighbor->ZoneID;
				return;
			}
		}

		/*
		 * If we've failed, recalculate the map's zones completely.
		 */
		Map.Zone_Reset();
	}
}


/// <summary>
/// Builds the list of places where the map joins itself.
/// Tunnels and bridges link ground that the terrain alone would keep apart, so this routine
/// hunts them all down and records them. The zone and subzone rebuilds work from that list
/// when stitching the two ends of each crossing together.
/// </summary>
void MapClass::Compute_Zone_Connections(void)
{
	static int _bridge_subtiles1[] = {
		7, 7, -1, 7, 7, -1, 4, 4,
		4, 4,  4, 2, 2,  2, 2, 2
	};
	static FacingType _facings[] = {
		FACING_E, FACING_E, FACING_NONE, FACING_S, FACING_S, FACING_NONE, FACING_E, FACING_E,
		FACING_E, FACING_E,    FACING_E, FACING_S, FACING_S,    FACING_S, FACING_S, FACING_S
	};
	static int _bridge_subtiles2[] = {
		-1, -1, 4, -1, -1, 2, 4, 4,
		 4,  4, 4,  2,  2, 2, 2, 2
	};

	ZoneConnections.Clear();
	Reset_Iterator();
	CellClass * cptr = Iterate();

	while (cptr != NULL) {

		if (!cptr->Is_Tile_Bridge() && !cptr->Is_Tile_Train_Bridge()) {

			/*
			 * A tunnel entrance pairs with its exit cell; record the connection once,
			 * from the lower numbered cell of the pair.
			 */
			if (cptr->Has_Tunnel()) {
				if ((cptr->Adjacent_Cell(FACING_E).Has_Tunnel() && cptr->Adjacent_Cell(FACING_W).Has_Tunnel()) ||
					(cptr->Adjacent_Cell(FACING_S).Has_Tunnel() && cptr->Adjacent_Cell(FACING_N).Has_Tunnel())) {

					Cell cell = cptr->Get_Tunnel()->Exit;
					if (Map_Cell_Index(cptr->CellID) < Map_Cell_Index(cell)) {
						Cell exit = cptr->Get_Tunnel()->Exit;
						ZoneConnectionClass con;
						con.From = cptr->CellID;
						con.To = exit;
						con.IsPassable = true;
						con.Type = 1;
						ZoneConnections.Add(con);
					}
				}
			}

		} else {

			int faceindex;
			if (cptr->Is_Tile_Bridge()) {
				faceindex = cptr->ITType - IsometricTileTypeClass::BridgeSet;
			} else {
				faceindex = cptr->ITType - IsometricTileTypeClass::TrainBridgeSet;
			}

			/*
			 * Only walk the span from its starting subtile.
			 */
			if (_bridge_subtiles1[faceindex] == cptr->SubTile) {

				bool crossed = false;
				bool passable = true;
				FacingType facing = _facings[faceindex];

				/*
				 * Walk across the span until the far end subtile has been passed or
				 * the walk leaves the radar map.
				 */
				CellClass * bptr = cptr;
				for (;;) {
					bptr = &bptr->Adjacent_Cell(facing);

					if (In_Radar(bptr->CellID)) {

						if (!crossed) {

							if (bptr->Is_Tile_Bridge()) {
								int index = bptr->ITType - IsometricTileTypeClass::BridgeSet;
								if (index != -1 && _bridge_subtiles2[index] == bptr->SubTile) {
									crossed = true;
								}

							} else if (bptr->Is_Tile_Train_Bridge()) {
								int index = bptr->ITType - IsometricTileTypeClass::TrainBridgeSet;
								if (index != -1 && _bridge_subtiles2[index] == bptr->SubTile) {
									crossed = true;
								}
							} else if (!bptr->IsUnderBridge) {
								passable = false;
							}
							continue;
						}

					} else if (!crossed) {
						break;
					}

					/*
					 * Record the bridge span as a zone connection.
					 */
					ZoneConnectionClass con;
					con.From = cptr->CellID;
					con.To = bptr->Adjacent_Cell(Facing_Sub((unsigned char)facing, FACING_180)).CellID;
					con.IsPassable = passable;
					con.Type = 0;
					ZoneConnections.Add(con);
					break;
				}
			}
		}

		cptr = Iterate();
	}
}


/// <summary>
/// Searches ZoneConnections (starting at index) for a bridge-span connection covering the cell.
/// Matches when the cell lies within the span's axis range and within maxdist of the span line.
/// </summary>
/// <param name="cell">The cell to find a covering connection for.</param>
/// <param name="maxdist">Maximum perpendicular distance from the span line to accept.</param>
/// <param name="index">The connection index to begin searching from.</param>
/// <returns>The matching connection index, or -1 if none found.</returns>
int MapClass::Zone_Connection_Index(Cell const & cell, int maxdist, int index)
{
	for (int i = index; i < ZoneConnections.Count(); i++) {
		ZoneConnectionClass * con = &ZoneConnections[i];
		if (con->Type == 0) {
			Cell f = con->From;
			Cell t = con->To;
			if ((f.X - t.X) == 0) {
				if (cell.Y >= f.Y && cell.Y <= t.Y && abs(cell.X - f.X) <= maxdist) {
					return(i);
				}
			} else {
				if (cell.X <= t.X && cell.X >= f.X && abs(cell.Y - f.Y) <= maxdist) {
					return(i);
				}
			}
		}
	}
	return(-1);
}


/// <summary>
/// Takes the bridge connections over a cell out of service.
/// This routine is called once a span has come down. With the connections gone the
/// pathfinder stops routing traffic across the bridge.
/// </summary>
/// <param name="cell">A cell the bridge crosses over.</param>
/// <returns>bool; Was there still a live connection to take down?</returns>
bool MapClass::Unregister_Subzone_Connections(Cell const & cell)
{
	int index = Zone_Connection_Index(cell, 3);
	if (index == -1) {
		Compute_Zone_Connections();
		index = Zone_Connection_Index(cell, 3);
		if (index == -1) {
			return(false);
		}
	}

	bool result = false;

	while (index != -1) {
		if (ZoneConnections[index].IsPassable) {
			Unregister_Subzone_Connection(&ZoneConnections[index]);
			ZoneConnections[index].IsPassable = false;
			result = true;
		}
		index = Zone_Connection_Index(cell, 3, index + 1);
	};

	return(result);
}


/// <summary>
/// Brings the bridge connections over a cell back into service.
/// This routine is called once a span has been repaired. The connections it turns back on
/// are what let the pathfinder route traffic across the bridge again.
/// </summary>
/// <param name="cell">A cell the bridge crosses over.</param>
/// <returns>bool; Did a restored connection join ground that was apart?</returns>
bool MapClass::Register_Subzone_Connections(Cell const & cell)
{
	int index = Zone_Connection_Index(cell, 3);
	if (index == -1) {
		Compute_Zone_Connections();
		index = Zone_Connection_Index(cell, 3);
		if (index == -1) {
			return(false);
		}
	}

	bool result = false;

	while (index != -1) {
		if (!ZoneConnections[index].IsPassable) {
			ZoneConnections[index].IsPassable = true;
			Register_Subzone_Connection(&ZoneConnections[index]);
			if (!Is_Same_Cell_Zone(ZoneConnections[index].From, ZoneConnections[index].To)) {
				result = true;
			}
		}
		index = Zone_Connection_Index(cell, 3, index + 1);
	};

	return(result);
}


/***********************************************************************************************
 * MapClass::Nearby_Location -- Finds a generally clear location near a specified cell.        *
 *                                                                                             *
 *    This routine is used to find a location that probably will be ok to move to that is      *
 *    located as close as possible to the specified cell. The computer uses this when it has   *
 *    determined the ideal location for an object, but then needs to give a valid movement     *
 *    destination to a unit.                                                                   *
 *                                                                                             *
 * INPUT:   cell  -- The cell that scanning should radiate out from.                           *
 *                                                                                             *
 *          zone  -- The zone that must be matched to find a legal location (value of -1 means *
 *                   any zone will do).                                                        *
 *                                                                                             *
 *                                                                                             *
 *          check -- The type of zone to check against. Only valid if a zone value is given.   *
 *                                                                                             *
 * OUTPUT:  Returns with the cell that is generally clear (legal to move to) that is close     *
 *          to the specified cell.                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/05/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
Cell MapClass::Nearby_Location(Cell const & cell, SpeedType speed, int zone, MZoneType check, bool checkbridge, Point2D dimensions, bool checkoverlay, bool checkheight, bool checkburrow, bool allowunderbridge, Cell const & nearto) const
{
	Cell topten[24];
	int count = 0;

	int xx = cell.X;
	int yy = cell.Y;

	bool found = false;

	if (zone == 0xFFFF) {
		zone = -1;
	}

	/*
	 * Determine the starting cell's ground height. If bridge scanning is enabled
	 * and the start cell is under a bridge, raise the reference height to the
	 * bridge deck.
	 */
	int height = Map[cell].Height;
	if (checkbridge) {
		if (Map[cell].IsUnderBridge) {
			height += BRIDGE_CELL_HEIGHT;
		}
	}

	/*
	 * Limit the radius of the scan to the size of the visible play area, but never
	 * larger than 32 cells.
	 */
	int maxradius = PlayRect.Height + PlayRect.Width;
	if (maxradius > 32) {
		maxradius = 32;
	}

	/*
	**	Radiate outward from the specified location, looking for the closest
	**	location that is generally clear.
	*/
	int negradius = 0;
	for (int radius = 0; radius < maxradius; radius++, negradius--) {

		/*
		**	Scan the top and bottom rows of the "box".
		*/
		for (int x = negradius; x <= radius; x++) {

			Cell newcell = Cell(xx + x, yy - radius);
			CellClass * cellptr = &Map[newcell];
			if (((MapClass *)this)->In_Local_Radar(cellptr, true) && ((MapClass *)this)->Is_Clear_To_Move(newcell, dimensions.X, dimensions.Y, speed, zone, check, -1, checkbridge, checkoverlay)) {
				if (!checkheight || abs(int(height - BRIDGE_CELL_HEIGHT * cellptr->IsUnderBridge - cellptr->Height)) < 2) {
					if ((!checkburrow || cellptr->Can_Burrow_Here()) && (allowunderbridge || !cellptr->IsUnderBridge)) {
						topten[count++] = newcell;
						if (checkbridge || newcell == TacticalMap->Coord_To_Cell(Coord(newcell, 0))) {
							found = true;
						}
					}
				}
			}
			if (count == ARRAY_SIZE(topten)) break;

			newcell = Cell(xx + x, yy + radius);
			cellptr = &Map[newcell];
			if (((MapClass *)this)->In_Local_Radar(cellptr, true)) {
				cellptr = &Map[newcell];
				if (((MapClass *)this)->Is_Clear_To_Move(newcell, dimensions.X, dimensions.Y, speed, zone, check, -1, checkbridge, checkoverlay)) {
					if (!checkheight || abs(int(height - BRIDGE_CELL_HEIGHT * cellptr->IsUnderBridge - cellptr->Height)) < 2) {
						if ((!checkburrow || cellptr->Can_Burrow_Here()) && (allowunderbridge || !cellptr->IsUnderBridge)) {
							topten[count++] = newcell;
							if (checkbridge || newcell == TacticalMap->Coord_To_Cell(Coord(newcell, 0))) {
								found = true;
							}
						}
					}
				}
			}
			if (count == ARRAY_SIZE(topten)) break;
		}

		if (count == ARRAY_SIZE(topten)) break;

		/*
		**	Scan the left and right columns of the "box".
		*/
		for (int y = negradius + 1; y <= radius - 1; y++) {

			Cell newcell = Cell(xx - radius, yy + y);
			CellClass * cellptr = &Map[newcell];
			if (((MapClass *)this)->In_Local_Radar(cellptr, true) && ((MapClass *)this)->Is_Clear_To_Move(newcell, dimensions.X, dimensions.Y, speed, zone, check, -1, checkbridge, checkoverlay)) {
				if (!checkheight || abs(int(height - BRIDGE_CELL_HEIGHT * cellptr->IsUnderBridge - cellptr->Height)) < 2) {
					if ((!checkburrow || cellptr->Can_Burrow_Here()) && (allowunderbridge || !cellptr->IsUnderBridge)) {
						topten[count++] = newcell;
						if (checkbridge || newcell == TacticalMap->Coord_To_Cell(Coord(newcell, 0))) {
							found = true;
						}
					}
				}
			}
			if (count == ARRAY_SIZE(topten)) break;

			newcell = Cell(xx + radius, yy + y);
			cellptr = &Map[newcell];
			if (((MapClass *)this)->In_Local_Radar(cellptr, true) && ((MapClass *)this)->Is_Clear_To_Move(newcell, dimensions.X, dimensions.Y, speed, zone, check, -1, checkbridge, checkoverlay)) {
				if (!checkheight || abs(int(height - BRIDGE_CELL_HEIGHT * cellptr->IsUnderBridge - cellptr->Height)) < 2) {
					if ((!checkburrow || cellptr->Can_Burrow_Here()) && (allowunderbridge || !cellptr->IsUnderBridge)) {
						topten[count++] = newcell;
						if (checkbridge || newcell == TacticalMap->Coord_To_Cell(Coord(newcell, 0))) {
							found = true;
						}
					}
				}
			}
			if (count == ARRAY_SIZE(topten)) break;
		}

		if (count == ARRAY_SIZE(topten)) break;
		if (found) break;
	}

	if (count > 0) {

		/*
		 * Partition the gathered cells into those that are currently visible on the
		 * tactical display and those that are not.
		 */
		Cell visible[ARRAY_SIZE(topten)];
		Cell hidden[ARRAY_SIZE(topten)];
		int visiblecount = 0;
		int hiddencount = 0;

		int index;
		for (index = 0; index < count; index++) {
			if (TacticalMap->Coord_To_Cell(Coord(topten[index], 0)) == topten[index]) {
				visible[visiblecount++] = topten[index];
			} else {
				hidden[hiddencount++] = topten[index];
			}
		}

		/*
		 * If no preferred destination was supplied, pick a pseudo-random cell from the
		 * visible list (or the hidden list if nothing was visible).
		 */
		if (nearto == CELL_NONE) {
			if (visiblecount != 0) {
				return(visible[Frame % visiblecount]);
			}
			return(hidden[Frame % hiddencount]);
		}

		/*
		 * A preferred destination was supplied, so return the gathered cell that lies
		 * closest to it.
		 */
		int total = visiblecount;
		if (visiblecount == 0) {
			total = hiddencount;
		}

		Cell best = CELL_NONE;
		double bestdist = 100000.0;
		for (index = 0; index < total; index++) {
			Cell trycell = visible[index];
			if (visiblecount == 0) {
				trycell = hidden[index];
			}
			double dist = std::sqrt((double)((trycell.Y - nearto.Y) * (trycell.Y - nearto.Y) + (trycell.X - nearto.X) * (trycell.X - nearto.X)));
			if (dist < bestdist) {
				bestdist = dist;
				best = trycell;
			}
		}
		return(best);
	}

	return(CELL_NONE);
}


/// <summary>
/// Determines if a block of cells is clear to move into.
/// Use this routine when something needs room rather than a single cell -- a building
/// footprint or a landing site. Every cell of the block must pass the ordinary move test
/// for the block as a whole to be considered clear.
/// </summary>
/// <param name="cell">The upper left corner of the area to test.</param>
/// <param name="width">The width of the area in cells.</param>
/// <param name="height">The height of the area in cells.</param>
/// <param name="speed">The locomotion type that has to get through.</param>
/// <param name="zone">The zone the move test should check against.</param>
/// <param name="check">The movement zone layer to validate against.</param>
/// <param name="cell_height">The height the mover sits at.</param>
/// <param name="checkbridge">Should bridges be taken into account?</param>
/// <param name="block_overlays">Should a cell carrying any overlay count as blocked?</param>
/// <returns>bool; Is every cell of the area clear?</returns>
bool MapClass::Is_Clear_To_Move(Cell const & cell, int width, int height, SpeedType speed, int zone, MZoneType check, int cell_height, bool checkbridge, bool block_overlays)
{
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			Cell newcell = Cell(x,y) + cell;
			CellClass * cellptr = &Map[newcell];
			if ((block_overlays && cellptr->Overlay != OVERLAY_NONE) || !cellptr->Is_Clear_To_Move(speed, false, false, zone, check, cell_height, checkbridge)) {
				return(false);
			}
		}
	}
	return(true);
}


/***********************************************************************************************
 * MapClass::Base_Region -- Finds the owner and base zone for specified cell.                  *
 *                                                                                             *
 *    This routine is used to determine what base the specified cell is close to and what      *
 *    zone of that base the cell lies in. This routine is particularly useful in letting the   *
 *    computer know when the player targets a destination near a computer's base.              *
 *                                                                                             *
 * INPUT:   cell     -- The cell that is to be checked.                                        *
 *                                                                                             *
 *          house    -- Reference to the house type number. This value will be set if a base   *
 *                      was found nearby the specified cell.                                   *
 *                                                                                             *
 *          zone     -- The zone that the cell is located in IF the cell is near a base.       *
 *                                                                                             *
 *                                                                                             *
 * OUTPUT:  Was a base near the specified cell found? If not, then the 'house' and 'zone'      *
 *          reference values are left in an undefined state and the return value will be       *
 *          false.                                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/05/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool MapClass::Base_Region(Cell const & cell, HousesType & house, ZoneType & zone) const
{
	if (cell != CELL_NONE && In_Radar(cell)) {
		for (int index = HOUSE_FIRST; index < Houses.Count(); index++) {
			HouseClass * h = Houses[index];

			if (h && !h->IsDefeated && h->Center != COORD_NONE) {
				zone = h->Which_Zone(cell);
				if (zone != ZONE_NONE) {
					house = h->Class->House;
					return(true);
				}
			}
		}
	}
	return(false);
}


/*
 * Enum for bridge middle tile offsets.
 */
enum {
	BRIDGE_MIDDLE_OK,
	BRIDGE_MIDDLE_DAMAGED_1, /// damaged, connected one way
	BRIDGE_MIDDLE_DAMAGED_2, /// damaged, connected the other way
	BRIDGE_MIDDLE_DAMAGED_3, /// damaged, connected both ways
	BRIDGE_MIDDLE_DESTROYED
};


/// <summary>
/// Marks a bridge end as damaged or whole.
/// A bridge end is drawn from several cells sharing one tile piece, so this routine spreads
/// the new state out from the cell given to every neighbor drawn from that same piece. The
/// damage and repair code calls it whenever an end changes condition.
/// </summary>
/// <param name="cell">Any cell of the bridge end to change.</param>
/// <param name="damaged">Should the end be shown as damaged?</param>
/// <param name="recursive">Is this one of the spreading calls rather than the original
/// request?</param>
void MapClass::Set_Bridge_End_State(Cell const & cell, bool damaged, bool recursive)
{
	CellClass * cellptr = &Map[cell];
	if (!recursive) {
		if (cellptr->ITType == ISOTILE_NONE || cellptr->ITType == ISOTILE_NONE_LEGACY) {
			return;
		}
		IsometricTileTypeClass * isotype = IsometricTileTypes[cellptr->ITType];
		if (!isotype->Is_Randomized(cellptr->SubTile)) {
			return;
		}
		Point2D point;
		TacticalMap->Coord_To_Pixel((Coord)cell, point);
		point.Y -= LEVEL_PIXEL_H * cellptr->Height;
		TacticalMap->Register_Dirty_Area(Rect(point - Point2D(128, 128), 256, 256));
	}

	if (damaged != (bool)cellptr->IsBridgeDamaged) {
		cellptr->IsBridgeDamaged = damaged;
		IsometricTileType ittype = cellptr->ITType;
		Map.Radar_Background(cellptr->CellID);
		FacingType facing = FACING_FIRST;
		for (int i = 0; i < FACING_COUNT; i++) {
			Cell adjacent = Adjacent_Cell(cell, facing);
			CellClass * c = &Map[adjacent];
			if (c->ITType == ittype) {
				Set_Bridge_End_State(adjacent, damaged, true);
			}
			facing = FacingType(unsigned(facing + FACING_45) % FACING_COUNT);
		}
	}
}


/// <summary>
/// Sets the tile piece shown by a stretch of bridge middle.
/// A middle section is drawn from several cells that must all agree, so this routine
/// spreads the new piece out from the cell given to every neighbor still showing the old
/// one. The damage and repair code calls it whenever a section changes condition.
/// </summary>
/// <param name="cell">Any cell of the middle section to change.</param>
/// <param name="new_tile">The tile piece the section should show from now on.</param>
/// <param name="match_tile">The piece a neighbor must show to be carried along. Only
/// consulted on the spreading calls.</param>
/// <param name="cell_height">The height to re-evaluate the cell at.</param>
/// <param name="recursive">Is this one of the spreading calls rather than the original
/// request?</param>
void MapClass::Set_Bridge_Middle_State(Cell const & cell, IsometricTileType new_tile, IsometricTileType match_tile, int cell_height, bool recursive)
{
	CellClass * cellptr = &Map[cell];
	IsometricTileType old_tile;
	if (!recursive) {
		Point2D point;
		TacticalMap->Coord_To_Pixel((Coord)cell, point);
		point.Y -= LEVEL_PIXEL_H * cellptr->Height;
		TacticalMap->Register_Dirty_Area(Rect(point - Point2D(128, 128), 256, 256));
		old_tile = cellptr->ITType;
		if (new_tile == old_tile) {
			return;
		}
	} else {
		old_tile = match_tile;
	}

	if (cellptr->ITType != new_tile) {
		cellptr->ITType = new_tile;
		cellptr->Recalc_Attributes(cell_height);
		Map.Radar_Background(cellptr->CellID);
		FacingType facing = FACING_FIRST;
		for (FacingType i = FACING_FIRST; i < FACING_COUNT; i++) {
			Cell adjacent = Adjacent_Cell(cell, facing);
			CellClass * c = &Map[adjacent];
			if (c->ITType == old_tile) {
				Set_Bridge_Middle_State(adjacent, new_tile, old_tile, cell_height, true);
			}
			facing = Facing_Add(facing, FACING_45);
		}
	}
}


/// <summary>
/// Damages the bottom-right half of an east-west train bridge.
/// This routine ages one side of a span by a single stage, taking the deck and the tile
/// piece under it one step further toward ruin.
/// </summary>
/// <param name="cell">The cell being worked from; the piece dealt with is its neighbor in
/// the given direction.</param>
/// <param name="dir">The facing that leads to that piece.</param>
void MapClass::Internal_Damage_Train_Bridge_EW_BottomRight(Cell const & cell, FacingType dir)
{
	Cell adjacent = Adjacent_Cell(cell, dir);
	CellClass * adjacentptr = &Map[adjacent];
	if (adjacentptr->IsBridgeDeck) {
		if (adjacentptr->OverlayData > OVERLAYDATA_BRIDGE_EW_FULL4) {
			if (adjacentptr->OverlayData == OVERLAYDATA_BRIDGE_EW_TRANSITION2) {
				adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_EW_DAMAGED;
			}
		} else {
			adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_EW_TRANSITION1;
		}
	}
	IsometricTileType ittype = IsometricTileType(adjacentptr->ITType - IsometricTileTypeClass::TrainBridgeSet + 1);
	if (ittype == IsometricTileTypeClass::BridgeBottomRight1 || ittype == IsometricTileTypeClass::BridgeBottomRight2) {
		Set_Bridge_End_State(adjacent, true, false);
	} else if (ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_OK) {
		Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_OK), ISOTILE_INVALID, -1, false);
	} else if (ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2)  {
		Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2), ISOTILE_INVALID, -1, false);
	}
}


/// <summary>
/// Damages the top-left half of an east-west train bridge.
/// This routine ages one side of a span by a single stage, taking the deck and the tile
/// piece under it one step further toward ruin.
/// </summary>
/// <param name="cell">The cell being worked from; the piece dealt with is its neighbor in
/// the given direction.</param>
/// <param name="dir">The facing that leads to that piece.</param>
void MapClass::Internal_Damage_Train_Bridge_EW_TopLeft(Cell const & cell, FacingType dir)
{
	Cell adjacent = Adjacent_Cell(cell, dir);
	CellClass * adjacentptr = &Map[adjacent];
	if (adjacentptr->IsBridgeDeck) {
		if (adjacentptr->OverlayData > OVERLAYDATA_BRIDGE_EW_FULL4) {
			if (adjacentptr->OverlayData == OVERLAYDATA_BRIDGE_EW_TRANSITION1) {
				adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_EW_DAMAGED;
			}
		} else {
			adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_EW_TRANSITION2;
		}
	}
	IsometricTileType ittype = IsometricTileType(adjacentptr->ITType - IsometricTileTypeClass::TrainBridgeSet + 1);
	if (ittype == IsometricTileTypeClass::BridgeTopLeft1 || ittype == IsometricTileTypeClass::BridgeTopLeft2) {
		Set_Bridge_End_State(adjacent, true, false);
	} else if (ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_OK) {
		Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_1), ISOTILE_INVALID, -1, false);
	} else if (ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_1)  {
		Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2), ISOTILE_INVALID, -1, false);
	}
}


/// <summary>
/// Destroys the bottom-right half of an east-west train bridge.
/// This routine takes over once a span has been damaged past saving. It works its way
/// along the deck dropping each piece in turn, and lets the bridge cells back down onto
/// the ground they used to carry traffic over.
/// </summary>
/// <param name="cell">The cell being worked from; the piece dealt with is its neighbor in
/// the given direction.</param>
/// <param name="dir">The facing that leads to that piece, and on along the span.</param>
void MapClass::Internal_Destroy_Train_Bridge_EW_BottomRight(Cell const & cell, FacingType dir)
{
	Cell adjacent = Adjacent_Cell(cell, dir);
	CellClass * adjacentptr = &Map[adjacent];
	if (adjacentptr->IsBridgeDeck) {
		if (adjacentptr->OverlayData > OVERLAYDATA_BRIDGE_EW_DAMAGED) {
			if (adjacentptr->OverlayData == OVERLAYDATA_BRIDGE_EW_END2) {
				Internal_Destroy_Train_Bridge_EW_BottomRight(adjacent, dir);
				adjacentptr->Set_Under_Rail_Bridge(FACING_N, false);
				adjacentptr->OverlayData = 0;
				adjacentptr->Overlay = OVERLAY_NONE;
				Map.Radar_Background(adjacentptr->CellID);
			}
		} else {
			adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_EW_END1;
		}
	}
	IsometricTileType ittype = IsometricTileType(adjacentptr->ITType - IsometricTileTypeClass::TrainBridgeSet + 1);
	if (ittype != IsometricTileTypeClass::BridgeBottomRight1 && ittype != IsometricTileTypeClass::BridgeBottomRight2) {
		if (ittype == IsometricTileTypeClass::BridgeMiddle1 || ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2) {
			Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2), ISOTILE_INVALID, -1, false);
		} else if (ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3) {
			Internal_Destroy_Train_Bridge_EW_BottomRight(adjacent, dir);
			if ((adjacentptr->SubTile & 1) != 0) {
				Map[adjacent + Cell(-1, 0)].Destroy_Bridge();
				Map[adjacent + Cell(-1, -1)].Destroy_Bridge();
				Map[adjacent + Cell(-1, 1)].Destroy_Bridge();
				Set_Bridge_Middle_State(adjacent,  IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3), ISOTILE_INVALID, Map[adjacent + Cell(-1, 0)].Height - BRIDGE_CELL_HEIGHT, 0);
			} else {
				Map[adjacent + Cell(0, 0)].Destroy_Bridge();
				Map[adjacent + Cell(0, -1)].Destroy_Bridge();
				Map[adjacent + Cell(0, 1)].Destroy_Bridge();
				Set_Bridge_Middle_State(adjacent,  IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3), ISOTILE_INVALID, Map[adjacent].Height - BRIDGE_CELL_HEIGHT, 0);
			}
		}
	} else {
		Set_Bridge_End_State(adjacent, true, false);
	}
}


/// <summary>
/// Destroys the top-left half of an east-west train bridge.
/// This routine takes over once a span has been damaged past saving. It works its way
/// along the deck dropping each piece in turn, and lets the bridge cells back down onto
/// the ground they used to carry traffic over.
/// </summary>
/// <param name="cell">The cell being worked from; the piece dealt with is its neighbor in
/// the given direction.</param>
/// <param name="dir">The facing that leads to that piece, and on along the span.</param>
void MapClass::Internal_Destroy_Train_Bridge_EW_TopLeft(Cell const & cell, FacingType dir)
{
	Cell adjacent = Adjacent_Cell(cell, dir);
	CellClass * adjacentptr = &Map[adjacent];
	if (adjacentptr->IsBridgeDeck) {
		if (adjacentptr->OverlayData > OVERLAYDATA_BRIDGE_EW_DAMAGED) {
			if (adjacentptr->OverlayData == OVERLAYDATA_BRIDGE_EW_END1) {
				Internal_Destroy_Train_Bridge_EW_TopLeft(adjacent, dir);
				adjacentptr->Set_Under_Rail_Bridge(FACING_N, false);
				adjacentptr->OverlayData = 0;
				adjacentptr->Overlay = OVERLAY_NONE;
				Map.Radar_Background(adjacentptr->CellID);
			}
		} else {
			adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_EW_END2;
		}
	}
	IsometricTileType ittype = IsometricTileType(adjacentptr->ITType - IsometricTileTypeClass::TrainBridgeSet + 1);
	if (ittype != IsometricTileTypeClass::BridgeTopLeft1 && ittype != IsometricTileTypeClass::BridgeTopLeft2) {
		if (ittype == IsometricTileTypeClass::BridgeMiddle1 || ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_1) {
			Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2), ISOTILE_INVALID, -1, false);
		} else if (ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3) {
			Internal_Destroy_Train_Bridge_EW_TopLeft(adjacent, dir);
			if ((adjacentptr->SubTile & 1) != 0) {
				Map[adjacent + Cell(-1, 0)].Destroy_Bridge();
				Map[adjacent + Cell(-1, -1)].Destroy_Bridge();
				Map[adjacent + Cell(-1, 1)].Destroy_Bridge();
				Set_Bridge_Middle_State(adjacent,  IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3), ISOTILE_INVALID, Map[adjacent + Cell(-1, 0)].Height - BRIDGE_CELL_HEIGHT, 0);
			} else {
				Map[adjacent + Cell(0, 0)].Destroy_Bridge();
				Map[adjacent + Cell(0, -1)].Destroy_Bridge();
				Map[adjacent + Cell(0, 1)].Destroy_Bridge();
				Set_Bridge_Middle_State(adjacent,  IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3), ISOTILE_INVALID, Map[adjacent].Height - BRIDGE_CELL_HEIGHT, 0);
			}
		}
	} else {
		Set_Bridge_End_State(adjacent, true, false);
	}
}


/// <summary>
/// Damages the bottom-left half of a north-south train bridge.
/// This routine ages one side of a span by a single stage, taking the deck and the tile
/// piece under it one step further toward ruin.
/// </summary>
/// <param name="cell">The cell being worked from; the piece dealt with is its neighbor in
/// the given direction.</param>
/// <param name="dir">The facing that leads to that piece.</param>
void MapClass::Internal_Damage_Train_Bridge_NS_BottomLeft(Cell const & cell, FacingType dir)
{
	Cell adjacent = Adjacent_Cell(cell, dir);
	CellClass * adjacentptr = &Map[adjacent];
	if (adjacentptr->IsBridgeDeck) {
		if (adjacentptr->OverlayData >= OVERLAYDATA_BRIDGE_NS_FULL1) {
			if (adjacentptr->OverlayData > OVERLAYDATA_BRIDGE_NS_FULL4) {
				if (adjacentptr->OverlayData == OVERLAYDATA_BRIDGE_NS_TRANSITION1) {
					adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_NS_DAMAGED;
				}
			} else {
				adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_NS_TRANSITION2;
			}
		}
	}
	IsometricTileType ittype = IsometricTileType(adjacentptr->ITType - IsometricTileTypeClass::TrainBridgeSet + 1);
	if (ittype == IsometricTileTypeClass::BridgeBottomLeft1 || ittype == IsometricTileTypeClass::BridgeBottomLeft2) {
		Set_Bridge_End_State(adjacent, true, false);
	} else if (ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_OK) {
		Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_OK), ISOTILE_INVALID, -1, false);
	} else if (ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2)  {
		Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2), ISOTILE_INVALID, -1, false);
	}
}


/// <summary>
/// Damages the top-right half of a north-south train bridge.
/// This routine ages one side of a span by a single stage, taking the deck and the tile
/// piece under it one step further toward ruin.
/// </summary>
/// <param name="cell">The cell being worked from; the piece dealt with is its neighbor in
/// the given direction.</param>
/// <param name="dir">The facing that leads to that piece.</param>
void MapClass::Internal_Damage_Train_Bridge_NS_TopRight(Cell const & cell, FacingType dir)
{
	Cell adjacent = Adjacent_Cell(cell, dir);
	CellClass * adjacentptr = &Map[adjacent];
	if (adjacentptr->IsBridgeDeck) {
		if (adjacentptr->OverlayData >= OVERLAYDATA_BRIDGE_NS_FULL1) {
			if (adjacentptr->OverlayData > OVERLAYDATA_BRIDGE_NS_FULL4) {
				if (adjacentptr->OverlayData == OVERLAYDATA_BRIDGE_NS_TRANSITION2) {
					adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_NS_DAMAGED;
				}
			} else {
				adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_NS_TRANSITION1;
			}
		}
	}
	IsometricTileType ittype = IsometricTileType(adjacentptr->ITType - IsometricTileTypeClass::TrainBridgeSet + 1);
	if (ittype == IsometricTileTypeClass::BridgeTopRight1 || ittype == IsometricTileTypeClass::BridgeTopRight2) {
		Set_Bridge_End_State(adjacent, true, false);
	} else if (ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_OK) {
		Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_1), ISOTILE_INVALID, -1, false);
	} else if (ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_1)  {
		Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2), ISOTILE_INVALID, -1, false);
	}
}


/// <summary>
/// Destroys the bottom-left half of a north-south train bridge.
/// This routine takes over once a span has been damaged past saving. It works its way
/// along the deck dropping each piece in turn, and lets the bridge cells back down onto
/// the ground they used to carry traffic over.
/// </summary>
/// <param name="cell">The cell being worked from; the piece dealt with is its neighbor in
/// the given direction.</param>
/// <param name="dir">The facing that leads to that piece, and on along the span.</param>
void MapClass::Internal_Destroy_Train_Bridge_NS_BottomLeft(Cell const & cell, FacingType dir)
{
	Cell adjacent = Adjacent_Cell(cell, dir);
	CellClass * adjacentptr = &Map[adjacent];
	if (adjacentptr->IsBridgeDeck) {
		if (adjacentptr->OverlayData >= OVERLAYDATA_BRIDGE_NS_FULL1) {
			if (adjacentptr->OverlayData > OVERLAYDATA_BRIDGE_NS_DAMAGED) {
				if (adjacentptr->OverlayData == OVERLAYDATA_BRIDGE_NS_END1) {
					Internal_Destroy_Train_Bridge_NS_BottomLeft(adjacent, dir);
					adjacentptr->Set_Under_Rail_Bridge(FACING_W, false);
					adjacentptr->OverlayData = 0;
					adjacentptr->Overlay = OVERLAY_NONE;
					Map.Radar_Background(adjacentptr->CellID);
				}
			} else {
				adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_NS_END2;
			}
		}
	}
	IsometricTileType ittype = IsometricTileType(adjacentptr->ITType - IsometricTileTypeClass::TrainBridgeSet + 1);
	if (ittype != IsometricTileTypeClass::BridgeBottomLeft1 && ittype != IsometricTileTypeClass::BridgeBottomLeft2) {
		if (ittype == IsometricTileTypeClass::BridgeMiddle2 || ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2) {
			Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2), ISOTILE_INVALID, -1, false);
		} else if (ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3) {
			Internal_Destroy_Train_Bridge_NS_BottomLeft(adjacent, dir);
			if (adjacentptr->SubTile > 4) {
				Map[adjacent + Cell(-1, -1)].Destroy_Bridge();
				Map[adjacent + Cell(0, -1)].Destroy_Bridge();
				Map[adjacent + Cell(1, -1)].Destroy_Bridge();
				Set_Bridge_Middle_State(adjacent,  IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3), ISOTILE_INVALID, Map[adjacent + Cell(0, -1)].Height - BRIDGE_CELL_HEIGHT, 0);
			} else {
				Map[adjacent + Cell(-1, 0)].Destroy_Bridge();
				Map[adjacent + Cell(0, 0)].Destroy_Bridge();
				Map[adjacent + Cell(1, 0)].Destroy_Bridge();
				Set_Bridge_Middle_State(adjacent,  IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3), ISOTILE_INVALID, Map[adjacent].Height - BRIDGE_CELL_HEIGHT, 0);
			}
		}
	} else {
		Set_Bridge_End_State(adjacent, true, false);
	}
}


/// <summary>
/// Destroys the top-right half of a north-south train bridge.
/// This routine takes over once a span has been damaged past saving. It works its way
/// along the deck dropping each piece in turn, and lets the bridge cells back down onto
/// the ground they used to carry traffic over.
/// </summary>
/// <param name="cell">The cell being worked from; the piece dealt with is its neighbor in
/// the given direction.</param>
/// <param name="dir">The facing that leads to that piece, and on along the span.</param>
void MapClass::Internal_Destroy_Train_Bridge_NS_TopRight(Cell const & cell, FacingType dir)
{
	Cell adjacent = Adjacent_Cell(cell, dir);
	CellClass * adjacentptr = &Map[adjacent];
	if (adjacentptr->IsBridgeDeck) {
		if (adjacentptr->OverlayData >= OVERLAYDATA_BRIDGE_NS_FULL1) {
			if (adjacentptr->OverlayData > OVERLAYDATA_BRIDGE_NS_DAMAGED) {
				if (adjacentptr->OverlayData == OVERLAYDATA_BRIDGE_NS_END2) {
					Internal_Destroy_Train_Bridge_NS_TopRight(adjacent, dir);
					adjacentptr->Set_Under_Rail_Bridge(FACING_W, false);
					adjacentptr->OverlayData = 0;
					adjacentptr->Overlay = OVERLAY_NONE;
					Map.Radar_Background(adjacentptr->CellID);
				}
			} else {
				adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_NS_END1;
			}
		}
	}
	IsometricTileType ittype = IsometricTileType(adjacentptr->ITType - IsometricTileTypeClass::TrainBridgeSet + 1);
	if (ittype != IsometricTileTypeClass::BridgeTopRight1 && ittype != IsometricTileTypeClass::BridgeTopRight2) {
		if (ittype == IsometricTileTypeClass::BridgeMiddle2 || ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_1) {
			Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2), ISOTILE_INVALID, -1, false);
		} else if (ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3) {
			Internal_Destroy_Train_Bridge_NS_TopRight(adjacent, dir);
			if (adjacentptr->SubTile > 4) {
				Map[adjacent + Cell(-1, -1)].Destroy_Bridge();
				Map[adjacent + Cell(0, -1)].Destroy_Bridge();
				Map[adjacent + Cell(1, -1)].Destroy_Bridge();
				Set_Bridge_Middle_State(adjacent,  IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3), ISOTILE_INVALID, Map[adjacent + Cell(0, -1)].Height - BRIDGE_CELL_HEIGHT, 0);
			} else {
				Map[adjacent + Cell(-1, 0)].Destroy_Bridge();
				Map[adjacent + Cell(0, 0)].Destroy_Bridge();
				Map[adjacent + Cell(1, 0)].Destroy_Bridge();
				Set_Bridge_Middle_State(adjacent,  IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3), ISOTILE_INVALID, Map[adjacent].Height - BRIDGE_CELL_HEIGHT, 0);
			}
		}
	} else {
		Set_Bridge_End_State(adjacent, true, false);
	}
}


/// <summary>
/// Repairs a broken train bridge near the given cell.
/// This routine is used by an engineer sent out to a damaged rail bridge. A low bridge
/// lying close by is handed straight to the low bridge repair; otherwise it works out
/// which span the order meant and puts the deck back a section at a time until the
/// crossing is whole again. It is the counterpart of Repair_Bridge.
/// </summary>
/// <param name="cell">Where the repair was ordered; the bridge itself may be a few cells
/// away.</param>
void MapClass::Repair_Train_Bridge(Cell const & cell)
{
	/*
	 * Scan the 5x5 block of cells around the supplied cell looking for a low
	 * bridge overlay. If one is found, hand off to the low bridge repair routine
	 * and exit.
	 */
	for (int x = -2; x < 3; x++) {
		for (int y = -2; y < 3; y++) {
			if (Is_Low_Bridge(cell + Cell(x, y))) {
				Repair_Low_Bridge_Span(cell + Cell(x, y));
				return;
			}
		}
	}

	/*
	 * Locate a cell that is part of a bridge. Start at the supplied cell and, if it
	 * is not itself bridge related, walk up to three cells out along each of the
	 * eight facings until a bridge cell is found.
	 */
	CellClass * cellptr = &Map[cell];

	int facing = FACING_FIRST;
	if (!cellptr->IsUnderBridge && !cellptr->WasUnderBridge) {
		for (int index = 0; index < FACING_COUNT; index++) {

			Cell adjacent = Adjacent_Cell(cell, Facing_Add(facing, FACING_0));
			cellptr = &Map[adjacent];
			if (cellptr->IsUnderBridge || cellptr->WasUnderBridge) {
				break;
			}

			adjacent = Adjacent_Cell(adjacent, Facing_Add(facing, FACING_0));
			cellptr = &Map[adjacent];
			if (cellptr->IsUnderBridge || cellptr->WasUnderBridge) {
				break;
			}

			adjacent = Adjacent_Cell(adjacent, Facing_Add(facing, FACING_0));
			cellptr = &Map[adjacent];
			if (cellptr->IsUnderBridge || cellptr->WasUnderBridge) {
				break;
			}

			facing = Facing_Add(facing, FACING_45);
		}
	}

	/*
	 * Nothing bridge related found nearby - there is nothing to repair.
	 */
	if (!cellptr->IsUnderBridge && !cellptr->WasUnderBridge) {
		return;
	}

	/*
	 * Resolve the span anchor cell.
	 */
	Cell origin;
	if (cellptr->IsUnderBridge) {

		/*
		 * A live bridge cell points at the bridge owner (or is the owner).
		 */
		origin = cellptr->Get_Bridge_Deck_Cell();

	} else {

		/*
		 * An already damaged ("was under") bridge cell. Walk along the bridge body
		 * until a cell that is no longer marked as a former bridge cell is found,
		 * giving up after four steps, then step back two cells to recover the span
		 * end.
		 */
		int count = 0;
		int step = cellptr->IsBridgeEastWest ? FACING_S : FACING_E;
		Cell walk = cellptr->CellID;
		CellClass * walkptr;
		while (true) {
			walk = Adjacent_Cell(walk, Facing_Add(step, FACING_0));
			walkptr = &Map[walk];
			if (!walkptr->WasUnderBridge) {
				break;
			}
			if (++count >= 4) {
				return;
			}
		}

		FacingType back = Facing_Sub(step, FACING_180);
		origin = Adjacent_Cell(Adjacent_Cell(walk, back), back);
	}

	/*
	 * Walk the span looking for the end or middle tiles, repairing as we go.
	 */
	Cell spancell = origin;
	int advance = cellptr->IsBridgeEastWest ? FACING_W : FACING_N;

	char zonechanged = 0;
	IsometricTileType ittype = ISOTILE_CLEAR;

	DynamicVectorClass<Cell> cells;

	Rect dirty;

	while (true) {

		/*
		 * Bail the moment the span steps outside the playable rectangle.
		 */
		if (!Map.In_Radar(spancell)) {
			break;
		}

		if (Array[spancell.X + spancell.Y * MAP_CELL_W] != NULL) {

			CellClass * spanptr = &Map[spancell];
			ittype = IsometricTileType(spanptr->ITType - IsometricTileTypeClass::TrainBridgeSet + 1);

			/*
			 * East/west bridge head - restore it and register the dirty area.
			 */
			if ((ittype == IsometricTileTypeClass::BridgeTopLeft1
					|| ittype == IsometricTileTypeClass::BridgeTopLeft2) && spanptr->SubTile == 8) {
				Set_Bridge_End_State(spancell, false, false);
				Repair_Train_Bridge_Span(&Map[spancell], FACING_E, &dirty);
				TacticalMap->Register_Dirty_Area(dirty, false);
				break;
			}

			/*
			 * East/west bridge middle piece.
			 */
			if ((ittype == IsometricTileTypeClass::BridgeMiddle1
					|| ittype == IsometricTileTypeClass::BridgeMiddle1 + 3
					|| ittype == IsometricTileTypeClass::BridgeMiddle1 + 4
					|| ittype == IsometricTileTypeClass::BridgeMiddle1 + 1
					|| ittype == IsometricTileTypeClass::BridgeMiddle1 + 2) && spanptr->SubTile == 5) {

				if (ittype == IsometricTileTypeClass::BridgeMiddle1 + 4) {

					/*
					 * A fully destroyed middle - raise the three affected cells back
					 * to bridge height and queue them for recalculation.
					 */
					Set_Bridge_Middle_State(spancell, IsometricTileType(IsometricTileTypeClass::BridgeMiddle1 + IsometricTileTypeClass::TrainBridgeSet - 1), ISOTILE_INVALID, -1, false);

					Cell middle = Adjacent_Cell(spancell, FACING_W);
					Map[middle].Height += BRIDGE_CELL_HEIGHT;
					Map[Adjacent_Cell(middle, FACING_N)].Height += BRIDGE_CELL_HEIGHT;
					Map[Adjacent_Cell(middle, FACING_S)].Height += BRIDGE_CELL_HEIGHT;

					zonechanged = Register_Subzone_Connections(middle);

					for (int dx = 0; dx < 2; dx++) {
						for (int dy = -2; dy < 3; dy++) {
							cells.Add(Cell(middle + Cell(dx, dy)));
						}
					}
				}

				Repair_Train_Bridge(spancell + Cell(-2, 0));
				Repair_Train_Bridge_Span(&Map[spancell], FACING_E, &dirty);
				TacticalMap->Register_Dirty_Area(dirty, false);
				break;
			}

			/*
			 * North/south bridge head - restore it and register the dirty area.
			 */
			if ((ittype == IsometricTileTypeClass::BridgeTopRight1
					|| ittype == IsometricTileTypeClass::BridgeTopRight2) && spanptr->SubTile == 12) {
				Set_Bridge_End_State(spancell, false, false);
				Repair_Train_Bridge_Span(&Map[spancell], FACING_S, &dirty);
				TacticalMap->Register_Dirty_Area(dirty, false);
				break;
			}

			/*
			 * North/south bridge middle piece.
			 */
			if ((ittype == IsometricTileTypeClass::BridgeMiddle2
					|| ittype == IsometricTileTypeClass::BridgeMiddle2 + 3
					|| ittype == IsometricTileTypeClass::BridgeMiddle2 + 4
					|| ittype == IsometricTileTypeClass::BridgeMiddle2 + 1
					|| ittype == IsometricTileTypeClass::BridgeMiddle2 + 2) && spanptr->SubTile == 7) {

				if (ittype == IsometricTileTypeClass::BridgeMiddle2 + 4) {

					Set_Bridge_Middle_State(spancell, IsometricTileType(IsometricTileTypeClass::BridgeMiddle2 + IsometricTileTypeClass::TrainBridgeSet - 1), ISOTILE_INVALID, -1, false);

					Cell middle = Adjacent_Cell(spancell, FACING_N);
					Map[middle].Height += BRIDGE_CELL_HEIGHT;
					Map[Adjacent_Cell(middle, FACING_E)].Height += BRIDGE_CELL_HEIGHT;
					Map[Adjacent_Cell(middle, FACING_W)].Height += BRIDGE_CELL_HEIGHT;

					zonechanged = Register_Subzone_Connections(middle);

					for (int dy = 0; dy < 2; dy++) {
						for (int dx = -2; dx < 3; dx++) {
							cells.Add(Cell(middle + Cell(dx, dy)));
						}
					}
				}

				Repair_Train_Bridge(spancell + Cell(0, -2));
				Repair_Train_Bridge_Span(&Map[spancell], FACING_S, &dirty);
				TacticalMap->Register_Dirty_Area(dirty, false);
				break;
			}
		}

		spancell = Adjacent_Cell(spancell, Facing_Add(advance, FACING_0));
	}

	if (zonechanged) {
		Zone_Reset();
	}

	if (cells.Count() > 0) {
		Recalc_Cells_In_List(cells);
	}
}


/// <summary>
/// Destroys one section of a train bridge.
/// This routine follows the deck along until it reaches the end of a section, then clears
/// the bridge away behind that end, springs the destruction triggers over the length of
/// it, and carries on into the next section. A span whose end is nowhere nearby is left
/// alone. It is the rail counterpart of Destroy_High_Bridge_Span.
/// </summary>
/// <param name="dir">The direction to follow the deck -- FACING_E for an east/west bridge,
/// FACING_S for a north/south one.</param>
/// <param name="dirty">Gathers up the area that will need redrawing. May be NULL.</param>
/// <returns>bool; Was a span end found and dealt with?</returns>
bool MapClass::Destroy_Train_Bridge_Span(Cell const & cell, FacingType dir, Rect * dirty)
{
	CellClass * cellptr = &Map[cell];

	int count = 1;

	CellClass * startptr;
	bool was_owner;
	Cell ownercell;
	bool sprung;
	int index;

	/*
	 * Remember the first adjacent cell of the span. It is used as one endpoint
	 * when springing the bridge destruction triggers.
	 */
	Cell spanstart = Adjacent_Cell(cellptr->Fetch_CellID(), dir);
	Cell lastcell = spanstart;

	IsometricTileType ittype;

	/*
	 * Walk along the span looking for the end tile, giving up after 30 cells.
	 */
	while (true) {
		lastcell = Adjacent_Cell(cellptr->Fetch_CellID(), dir);
		cellptr = &Map[lastcell];

		ittype = IsometricTileType(cellptr->ITType - IsometricTileTypeClass::TrainBridgeSet + 1);

		if (dir == FACING_E) {

			if (ittype == IsometricTileTypeClass::BridgeBottomRight1 && cellptr->SubTile == 4
				|| ittype == IsometricTileTypeClass::BridgeBottomRight2 && cellptr->SubTile == 4
				|| (ittype == IsometricTileTypeClass::BridgeMiddle1
					|| ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3
					|| ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_1
					|| ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2)
					&& cellptr->SubTile == 4) {
				break;
			}

		} else if (dir == FACING_S
			&& (ittype == IsometricTileTypeClass::BridgeBottomLeft1 && cellptr->SubTile == 2
				|| ittype == IsometricTileTypeClass::BridgeBottomLeft2 && cellptr->SubTile == 2
				|| (ittype == IsometricTileTypeClass::BridgeMiddle2
					|| ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3
					|| ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_1
					|| ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2)
					&& cellptr->SubTile == 2)) {
			break;
		}

		if (++count >= 30) {
			break;
		}
	}

	/*
	 * If no span end was found within range, there is nothing to destroy.
	 */
	if (count != 30) {

		/*
		 * Re-fetch the span start cell. Its height anchors the dirty rectangle.
		 */
		startptr = &Map[cell];

		if (dirty != NULL) {

			/*
			 * Compute the screen rectangle spanning from the original cell to the
			 * located span end and union it into the supplied dirty rectangle.
			 */
			Point2D startpix;
			Coord coord;
			coord = Coord(cell, LEVEL_LEPTON_H * startptr->Height);
			TacticalMap->Coord_To_Pixel(coord, startpix);

			Point2D endpix;
			coord = Coord(lastcell, LEVEL_LEPTON_H * startptr->Height);
			TacticalMap->Coord_To_Pixel(coord, endpix);

			int minx = startpix.X;
			if (startpix.X >= endpix.X) {
				minx = endpix.X;
			}
			int miny = startpix.Y;
			if (startpix.Y >= endpix.Y) {
				miny = endpix.Y;
			}

			*dirty = Union(*dirty, Rect(minx - 64, miny - 64, abs(startpix.X - endpix.X) + 128, abs(startpix.Y - endpix.Y) + 128));
		}

		/*
		 * Walk the span again, springing the bridge destruction triggers and locating
		 * the transition from a bridge owner cell to a non-owner cell.
		 */
		was_owner = false;
		ownercell = Cell(-1, -1);
		sprung = false;

		for (index = 0; index < count; index++) {

			if (startptr->IsBridgeDeck && was_owner) {
				ownercell = startptr->CellID;
			}

			if (!startptr->IsBridgeDeck && !was_owner && ownercell != Cell(-1, -1)) {

				/*
				 * Found the span end owner. Clear the rail bridge state on the cell
				 * behind it, remove its overlay, refresh the radar background, then
				 * recurse to continue.
				 */
				Cell behind = Adjacent_Cell(startptr->Fetch_CellID(), Facing_Sub(dir, FACING_180));
				CellClass * behindptr = &Map[behind];

				behindptr->Set_Under_Rail_Bridge((dir == FACING_E) ? FACING_N : FACING_W, false);
				behindptr->OverlayData = 0;
				behindptr->Overlay = OVERLAY_NONE;
				Map.Radar_Background(behindptr->Fetch_CellID());

				Destroy_Train_Bridge_Span(cell, dir, dirty);
				return(true);
			}

			was_owner = !startptr->IsBridgeDeck;

			if (was_owner && !sprung) {
				Spring_Bridge_Destruction_Triggers(spanstart, lastcell);
				sprung = true;
			}

			Cell next = Adjacent_Cell(startptr->Fetch_CellID(), dir);
			startptr = &Map[next];
		}
	}

	return(false);
}


/// <summary>
/// Tears down what is left of a train bridge.
/// Given a cell beside a span that has just come down, this routine finds the bridge it
/// belonged to and follows it out to its ends, taking the rest of the deck with it. It is
/// the rail counterpart of Destroy_High_Bridge_Connections.
/// </summary>
/// <param name="cell">A cell next to the span that was destroyed.</param>
void MapClass::Destroy_Train_Bridge_Connections(Cell const & cell)
{
	/*
	 * Scan the eight adjacent cells looking for the first one that is part of
	 * a bridge (either currently under a bridge or formerly under one).
	 */
	CellClass * cellptr = &BlubCell;
	FacingType facing = FACING_N;
	for (int index = 0; index < FACING_COUNT; index++) {
		cellptr = &Map[Adjacent_Cell(cell, facing)];
		if (cellptr->IsUnderBridge || cellptr->WasUnderBridge) {
			break;
		}
		facing = Facing_Add(facing, FACING_45);
	}

	/*
	 * If neither bridge flag is set on the located cell, there is nothing to do.
	 */
	if (!cellptr->IsUnderBridge && !cellptr->WasUnderBridge) {
		return;
	}

	Cell origin;
	if (cellptr->IsUnderBridge) {

		/*
		 * A live bridge cell points us at the bridge owner (or itself if it is
		 * the owner) which is the anchor used to walk the span.
		 */
		origin = cellptr->Get_Bridge_Deck_Cell();

	} else {

		/*
		 * An already-damaged ("was under") bridge cell. Walk along the bridge
		 * body until a cell that is no longer marked as a former bridge cell is
		 * found, giving up after four steps.
		 */
		Cell walk = cellptr->CellID;
		int count = 0;
		int step = (cellptr->IsBridgeEastWest ? FACING_S : FACING_E);
		CellClass * walkptr;
		while (true) {
			walk = Adjacent_Cell(walk, (FacingType)step);
			walkptr = &Map[walk];
			if (!walkptr->WasUnderBridge) {
				break;
			}
			count++;
			if (count >= 4) {
				return;
			}
		}

		/*
		 * Step two cells back in the opposite orientation to recover the span end.
		 */
		FacingType back = Facing_Sub(step, FACING_180);
		origin = Adjacent_Cell(Adjacent_Cell(walk, back), back);
	}

	FacingType advance = cellptr->IsBridgeEastWest ? FACING_W : FACING_N;
	Rect dirty = RECT_NONE;
	Cell current = origin;

	/*
	 * Bail out the moment we step outside the visible map rectangle.
	 */
	if (current.X < MapRect.X) {
		return;
	}

	while (true) {

		if (current.X > MapRect.X + MapRect.Width) {
			return;
		}
		if (current.Y < MapRect.Y || current.Y > MapRect.Y + MapRect.Height) {
			return;
		}

		int cellnum = current.X + current.Y * MAP_CELL_W;
		if (Array[cellnum] != NULL) {

				CellClass * spanptr = &Map[current];
				IsometricTileType ittype = IsometricTileType(spanptr->ITType - IsometricTileTypeClass::TrainBridgeSet + 1);

				if ((ittype == IsometricTileTypeClass::BridgeTopLeft1
						|| ittype == IsometricTileTypeClass::BridgeTopLeft2) && spanptr->SubTile == 8) {
					if (Destroy_Train_Bridge_Span(current, FACING_E, &dirty)) {
						if (dirty != RECT_NONE) {
							TacticalMap->Register_Dirty_Area(dirty, false);
						}
					}
					return;
				}

				if ((ittype == IsometricTileTypeClass::BridgeMiddle1
						|| ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3
						|| ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_1
						|| ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2) && spanptr->SubTile == 5) {
					if (Destroy_Train_Bridge_Span(current, FACING_E, &dirty)) {
						if (dirty != RECT_NONE) {
							TacticalMap->Register_Dirty_Area(dirty, false);
						}
					}
					return;
				}

				if ((ittype == IsometricTileTypeClass::BridgeTopRight1
						|| ittype == IsometricTileTypeClass::BridgeTopRight2) && spanptr->SubTile == 12) {
					if (Destroy_Train_Bridge_Span(current, FACING_S, &dirty)) {
						if (dirty != RECT_NONE) {
							TacticalMap->Register_Dirty_Area(dirty, false);
						}
					}
					return;
				}

				if ((ittype == IsometricTileTypeClass::BridgeMiddle2
						|| ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3
						|| ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2
						|| ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_1) && spanptr->SubTile == 7) {
					break;
				}
		}

		current = Adjacent_Cell(current, advance);
		if (current.X < MapRect.X) {
			return;
		}
	}

	if (Destroy_Train_Bridge_Span(current, FACING_S, &dirty)) {
		if (dirty != RECT_NONE) {
			TacticalMap->Register_Dirty_Area(dirty, false);
		}
	}
}


/// <summary>
/// Damages the train bridge at a cell.
/// This routine is used by Damage_Bridge once the target turns out to be a rail bridge.
/// Each call ages the span one stage further -- whole, cracked, then gone -- and when the
/// last stage is reached the deck drops away and the movement zones are rebuilt around the
/// gap. It is the rail counterpart of Damage_High_Bridge.
/// </summary>
/// <param name="cell">The cell of the bridge, or of the ground beneath it.</param>
/// <returns>bool; Did the span come down this time?</returns>
bool MapClass::Damage_Train_Bridge(Cell const & cell)
{
	CellClass * bridge_deck_cell = &Map[cell];
	IsometricTileType ittype = IsometricTileType(bridge_deck_cell->ITType - IsometricTileTypeClass::TrainBridgeSet + 1);

	if (!bridge_deck_cell->IsUnderBridge) {

		if (ittype != IsometricTileTypeClass::BridgeMiddle1 &&
			ittype != IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_1 &&
			ittype != IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2 &&
			ittype != IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3 &&
			ittype != IsometricTileTypeClass::BridgeMiddle2 &&
			ittype != IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_1 &&
			ittype != IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2 &&
			ittype != IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3) {
			return(false);
		}
	}

	if (!bridge_deck_cell->IsUnderBridge) {

		int subtile = bridge_deck_cell->SubTile;
		if (ittype != IsometricTileTypeClass::BridgeMiddle1 &&
				ittype != IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3 &&
				ittype != IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_1 &&
				ittype != IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2) {

			if (subtile > 4) {
				return(false);
			}

			Cell middle_cell = bridge_deck_cell->CellID;
			while (subtile != 2) {
				if (subtile < 2) {
					middle_cell = Adjacent_Cell(middle_cell, FACING_E);
				} else {
					middle_cell = Adjacent_Cell(middle_cell, FACING_W);
				}
				subtile = Map[middle_cell].SubTile;
			}

			CellClass * middle_cellptr = &Map[middle_cell];

			if (ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3) {
				Cell zone_cell = middle_cellptr->CellID;

				if (middle_cellptr->SubTile > 4) {
					zone_cell.Y--;
					Map[Cell(middle_cell.X - 1, middle_cell.Y - 1)].Destroy_Bridge();
					Map[Cell(middle_cell.X, middle_cell.Y - 1)].Destroy_Bridge();
					Map[Cell(middle_cell.X + 1, middle_cell.Y - 1)].Destroy_Bridge();
					Cell cellid = middle_cellptr->Fetch_CellID();
					Set_Bridge_Middle_State(
						middle_cellptr->Fetch_CellID(),
						IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3),
						ISOTILE_INVALID,
						Map[Cell(cellid.X, cellid.Y - 1)].Height - BRIDGE_CELL_HEIGHT,
						false
					);
				} else {
					Map[Cell(middle_cell.X - 1, middle_cell.Y)].Destroy_Bridge();
					Map[Cell(middle_cell.X, middle_cell.Y)].Destroy_Bridge();
					Map[Cell(middle_cell.X + 1, middle_cell.Y)].Destroy_Bridge();
					Set_Bridge_Middle_State(
						middle_cellptr->Fetch_CellID(),
						IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3),
						ISOTILE_INVALID,
						middle_cellptr->Height - BRIDGE_CELL_HEIGHT,
						false
					);
				}

				Internal_Destroy_Train_Bridge_NS_BottomLeft(middle_cellptr->Fetch_CellID(), FACING_S);
				Internal_Destroy_Train_Bridge_NS_TopRight(middle_cellptr->Fetch_CellID(), FACING_N);

				Destroy_Train_Bridge_Connections(Adjacent_Cell(middle_cellptr->Fetch_CellID(), FACING_N));
				Destroy_Train_Bridge_Connections(Adjacent_Cell(middle_cellptr->Fetch_CellID(), FACING_S));

				if (Unregister_Subzone_Connections(zone_cell)) {
					Zone_Reset();
				}

				DynamicVectorClass<Cell> cells;
				for (int y = 0; y < 2; y++) {
					for (int x = -2; x < 3; x++) {
						cells.Add(Cell(zone_cell.X + x, zone_cell.Y + y));
					}
				}
				Map.Recalc_Cells_In_List(cells);
			}

			if (ittype == IsometricTileTypeClass::BridgeMiddle2 ||
				ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_1 ||
				ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2) {

				Set_Bridge_Middle_State(
					middle_cellptr->Fetch_CellID(),
					IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2),
					ISOTILE_INVALID,
					-1,
					false
				);
				Internal_Damage_High_Bridge_NS_BottomLeft(middle_cellptr->Fetch_CellID(), FACING_S);
				Internal_Damage_High_Bridge_NS_TopRight(middle_cellptr->Fetch_CellID(), FACING_N);
			}

			return(false);
		}

		if ((subtile & 1) != 0) {
			return(false);
		}

		Cell middle_cell = bridge_deck_cell->CellID;
		while (subtile != 4) {
			if (subtile < 4) {
				middle_cell = Adjacent_Cell(middle_cell, FACING_S);
			} else {
				middle_cell = Adjacent_Cell(middle_cell, FACING_N);
			}
			subtile = Map[middle_cell].SubTile;
		}

		CellClass * middle_cellptr = &Map[middle_cell];

		if (ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3) {
			Cell zone_cell = middle_cellptr->CellID;

			if ((middle_cellptr->SubTile & 1) != 0) {
				zone_cell.X--;
				Map[Cell(middle_cell.X - 1, middle_cell.Y - 1)].Destroy_Bridge();
				Map[Cell(middle_cell.X - 1, middle_cell.Y)].Destroy_Bridge();
				Map[Cell(middle_cell.X - 1, middle_cell.Y + 1)].Destroy_Bridge();
				Cell cellid = middle_cellptr->Fetch_CellID();
				Set_Bridge_Middle_State(
					middle_cellptr->Fetch_CellID(),
					IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3),
					ISOTILE_INVALID,
					Map[Cell(cellid.X - 1, cellid.Y)].Height - BRIDGE_CELL_HEIGHT,
					false
				);
			} else {
				Map[Cell(middle_cell.X, middle_cell.Y - 1)].Destroy_Bridge();
				Map[Cell(middle_cell.X, middle_cell.Y)].Destroy_Bridge();
				Map[Cell(middle_cell.X, middle_cell.Y + 1)].Destroy_Bridge();
				Set_Bridge_Middle_State(
					middle_cellptr->Fetch_CellID(),
					IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3),
					ISOTILE_INVALID,
					middle_cellptr->Height - BRIDGE_CELL_HEIGHT,
					false
				);
			}

			Internal_Destroy_Train_Bridge_EW_BottomRight(middle_cellptr->Fetch_CellID(), FACING_E);
			Internal_Destroy_Train_Bridge_EW_TopLeft(middle_cellptr->Fetch_CellID(), FACING_W);

			Destroy_Train_Bridge_Connections(Adjacent_Cell(middle_cellptr->Fetch_CellID(), FACING_W));
			Destroy_Train_Bridge_Connections(Adjacent_Cell(middle_cellptr->Fetch_CellID(), FACING_E));

			if (Unregister_Subzone_Connections(zone_cell)) {
				Zone_Reset();
			}

			DynamicVectorClass<Cell> cells;
			for (int x = 0; x < 2; x++) {
				for (int y = -2; y < 3; y++) {
					cells.Add(Cell(zone_cell.X + x, zone_cell.Y + y));
				}
			}
			Map.Recalc_Cells_In_List(cells);
		}

		if (ittype == IsometricTileTypeClass::BridgeMiddle1 ||
			ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_1 ||
			ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2) {

			Set_Bridge_Middle_State(
				middle_cellptr->Fetch_CellID(),
				IsometricTileType(IsometricTileTypeClass::TrainBridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2),
				ISOTILE_INVALID,
				-1,
				false
			);
			Internal_Damage_Train_Bridge_EW_BottomRight(middle_cellptr->Fetch_CellID(), FACING_E);
			Internal_Damage_Train_Bridge_EW_TopLeft(middle_cellptr->Fetch_CellID(), FACING_W);
		}

		return(false);
	} else {
		if (!bridge_deck_cell->IsBridgeDeck) {
			bridge_deck_cell = bridge_deck_cell->BridgeDeckCell;
		}

		int overlay_data = bridge_deck_cell->OverlayData;
		FacingType dir = overlay_data < OVERLAYDATA_BRIDGE_NS_FULL1 ? FACING_E : FACING_S;

		switch (overlay_data) {
			case OVERLAYDATA_BRIDGE_EW_FULL1:
			case OVERLAYDATA_BRIDGE_EW_FULL2:
			case OVERLAYDATA_BRIDGE_EW_FULL3:
			case OVERLAYDATA_BRIDGE_EW_FULL4:
			case OVERLAYDATA_BRIDGE_EW_TRANSITION1:
			case OVERLAYDATA_BRIDGE_EW_TRANSITION2:
				bridge_deck_cell->OverlayData = OVERLAYDATA_BRIDGE_EW_DAMAGED;
				Internal_Damage_Train_Bridge_EW_BottomRight(bridge_deck_cell->Fetch_CellID(), dir);
				Internal_Damage_Train_Bridge_EW_TopLeft(bridge_deck_cell->Fetch_CellID(), Facing_Sub(dir, FACING_180));
				return(false);

			case OVERLAYDATA_BRIDGE_EW_DAMAGED:
				Internal_Destroy_Train_Bridge_EW_BottomRight(bridge_deck_cell->Fetch_CellID(), dir);
				Internal_Destroy_Train_Bridge_EW_TopLeft(bridge_deck_cell->Fetch_CellID(), Facing_Sub(dir, FACING_180));
				bridge_deck_cell->Set_Under_Rail_Bridge(FACING_N, false);
				break;

			case OVERLAYDATA_BRIDGE_EW_END1:
				Internal_Destroy_Train_Bridge_EW_BottomRight(bridge_deck_cell->Fetch_CellID(), dir);
				bridge_deck_cell->Set_Under_Rail_Bridge(FACING_N, false);
				break;

			case OVERLAYDATA_BRIDGE_EW_END2:
				Internal_Destroy_Train_Bridge_EW_TopLeft(bridge_deck_cell->Fetch_CellID(), Facing_Sub(dir, FACING_180));
				bridge_deck_cell->Set_Under_Rail_Bridge(FACING_N, false);
				break;

			case OVERLAYDATA_BRIDGE_NS_FULL1:
			case OVERLAYDATA_BRIDGE_NS_FULL2:
			case OVERLAYDATA_BRIDGE_NS_FULL3:
			case OVERLAYDATA_BRIDGE_NS_FULL4:
			case OVERLAYDATA_BRIDGE_NS_TRANSITION1:
			case OVERLAYDATA_BRIDGE_NS_TRANSITION2:
				bridge_deck_cell->OverlayData = OVERLAYDATA_BRIDGE_NS_DAMAGED;
				Internal_Damage_Train_Bridge_NS_BottomLeft(bridge_deck_cell->Fetch_CellID(), dir);
				Internal_Damage_Train_Bridge_NS_TopRight(bridge_deck_cell->Fetch_CellID(), Facing_Sub(dir, FACING_180));
				return(false);

			case OVERLAYDATA_BRIDGE_NS_DAMAGED:
				Internal_Destroy_Train_Bridge_NS_BottomLeft(bridge_deck_cell->Fetch_CellID(), dir);
				Internal_Destroy_Train_Bridge_NS_TopRight(bridge_deck_cell->Fetch_CellID(), Facing_Sub(dir, FACING_180));
				bridge_deck_cell->Set_Under_Rail_Bridge(FACING_W, false);
				break;

			case OVERLAYDATA_BRIDGE_NS_END1:
				Internal_Destroy_Train_Bridge_NS_TopRight(bridge_deck_cell->Fetch_CellID(), Facing_Sub(dir, FACING_180));
				bridge_deck_cell->Set_Under_Rail_Bridge(FACING_W, false);
				break;

			case OVERLAYDATA_BRIDGE_NS_END2:
				Internal_Destroy_Train_Bridge_NS_BottomLeft(bridge_deck_cell->Fetch_CellID(), dir);
				bridge_deck_cell->Set_Under_Rail_Bridge(FACING_W, false);
				break;

			default:
				return(false);
		}

		bridge_deck_cell->OverlayData = 0;
		bridge_deck_cell->Overlay = OVERLAY_NONE;
		Destroy_Train_Bridge_Connections(cell);
		if (Unregister_Subzone_Connections(bridge_deck_cell->CellID)) {
			Zone_Reset();
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Damages the bottom-right half of an east-west high bridge.
/// This routine ages one side of a span by a single stage, taking the deck and the tile
/// piece under it one step further toward ruin.
/// </summary>
/// <param name="cell">The cell being worked from; the piece dealt with is its neighbor in
/// the given direction.</param>
/// <param name="dir">The facing that leads to that piece.</param>
void MapClass::Internal_Damage_High_Bridge_EW_BottomRight(Cell const & cell, FacingType dir)
{
	Cell adjacent = Adjacent_Cell(cell, dir);
	CellClass * adjacentptr = &Map[adjacent];
	if (adjacentptr->IsBridgeDeck) {
		if (adjacentptr->OverlayData > OVERLAYDATA_BRIDGE_EW_FULL4) {
			if (adjacentptr->OverlayData == OVERLAYDATA_BRIDGE_EW_TRANSITION2) {
				adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_EW_DAMAGED;
			}
		} else {
			adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_EW_TRANSITION1;
		}
	}
	IsometricTileType ittype = IsometricTileType(adjacentptr->ITType - IsometricTileTypeClass::BridgeSet + 1);
	if (ittype == IsometricTileTypeClass::BridgeBottomRight1 || ittype == IsometricTileTypeClass::BridgeBottomRight2) {
		Set_Bridge_End_State(adjacent, true, false);
	} else if (ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_OK) {
		Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_OK), ISOTILE_INVALID, -1, false);
	} else if (ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2)  {
		Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2), ISOTILE_INVALID, -1, false);
	}
}


/// <summary>
/// Damages the top-left half of an east-west high bridge.
/// This routine ages one side of a span by a single stage, taking the deck and the tile
/// piece under it one step further toward ruin.
/// </summary>
/// <param name="cell">The cell being worked from; the piece dealt with is its neighbor in
/// the given direction.</param>
/// <param name="dir">The facing that leads to that piece.</param>
void MapClass::Internal_Damage_High_Bridge_EW_TopLeft(Cell const & cell, FacingType dir)
{
	Cell adjacent = Adjacent_Cell(cell, dir);
	CellClass * adjacentptr = &Map[adjacent];
	if (adjacentptr->IsBridgeDeck) {
		if (adjacentptr->OverlayData > OVERLAYDATA_BRIDGE_EW_FULL4) {
			if (adjacentptr->OverlayData == OVERLAYDATA_BRIDGE_EW_TRANSITION1) {
				adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_EW_DAMAGED;
			}
		} else {
			adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_EW_TRANSITION2;
		}
	}
	IsometricTileType ittype = IsometricTileType(adjacentptr->ITType - IsometricTileTypeClass::BridgeSet + 1);
	if (ittype == IsometricTileTypeClass::BridgeTopLeft1 || ittype == IsometricTileTypeClass::BridgeTopLeft2) {
		Set_Bridge_End_State(adjacent, true, false);
	} else if (ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_OK) {
		Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_1), ISOTILE_INVALID, -1, false);
	} else if (ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_1)  {
		Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2), ISOTILE_INVALID, -1, false);
	}
}


/// <summary>
/// Destroys the bottom-right half of an east-west high bridge.
/// This routine takes over once a span has been damaged past saving. It works its way
/// along the deck dropping each piece in turn, and lets the bridge cells back down onto
/// the ground they used to carry traffic over.
/// </summary>
/// <param name="cell">The cell being worked from; the piece dealt with is its neighbor in
/// the given direction.</param>
/// <param name="dir">The facing that leads to that piece, and on along the span.</param>
void MapClass::Internal_Destroy_High_Bridge_EW_BottomRight(Cell const & cell, FacingType dir)
{
	Cell adjacent = Adjacent_Cell(cell, dir);
	CellClass * adjacentptr = &Map[adjacent];
	if (adjacentptr->IsBridgeDeck) {
		if (adjacentptr->OverlayData > OVERLAYDATA_BRIDGE_EW_DAMAGED) {
			if (adjacentptr->OverlayData == OVERLAYDATA_BRIDGE_EW_END2) {
				Internal_Destroy_High_Bridge_EW_BottomRight(adjacent, dir);
				adjacentptr->Set_Under_Bridge(FACING_N, false);
				adjacentptr->OverlayData = 0;
				adjacentptr->Overlay = OVERLAY_NONE;
				Map.Radar_Background(adjacentptr->CellID);
			}
		} else {
			adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_EW_END1;
		}
	}
	IsometricTileType ittype = IsometricTileType(adjacentptr->ITType - IsometricTileTypeClass::BridgeSet + 1);
	if (ittype != IsometricTileTypeClass::BridgeBottomRight1 && ittype != IsometricTileTypeClass::BridgeBottomRight2) {
		if (ittype == IsometricTileTypeClass::BridgeMiddle1 || ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2) {
			Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2), ISOTILE_INVALID, -1, false);
		} else if (ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3) {
			Internal_Destroy_High_Bridge_EW_BottomRight(adjacent, dir);
			if ((adjacentptr->SubTile & 1) != 0) {
				Map[adjacent + Cell(-1, 0)].Destroy_Bridge();
				Map[adjacent + Cell(-1, -1)].Destroy_Bridge();
				Map[adjacent + Cell(-1, 1)].Destroy_Bridge();
				Set_Bridge_Middle_State(adjacent,  IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3), ISOTILE_INVALID, Map[adjacent + Cell(-1, 0)].Height - BRIDGE_CELL_HEIGHT, 0);
			} else {
				Map[adjacent + Cell(0, 0)].Destroy_Bridge();
				Map[adjacent + Cell(0, -1)].Destroy_Bridge();
				Map[adjacent + Cell(0, 1)].Destroy_Bridge();
				Set_Bridge_Middle_State(adjacent,  IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3), ISOTILE_INVALID, Map[adjacent].Height - BRIDGE_CELL_HEIGHT, 0);
			}
		}
	} else {
		Set_Bridge_End_State(adjacent, true, false);
	}
}


/// <summary>
/// Destroys the top-left half of an east-west high bridge.
/// This routine takes over once a span has been damaged past saving. It works its way
/// along the deck dropping each piece in turn, and lets the bridge cells back down onto
/// the ground they used to carry traffic over.
/// </summary>
/// <param name="cell">The cell being worked from; the piece dealt with is its neighbor in
/// the given direction.</param>
/// <param name="dir">The facing that leads to that piece, and on along the span.</param>
void MapClass::Internal_Destroy_High_Bridge_EW_TopLeft(Cell const & cell, FacingType dir)
{
	Cell adjacent = Adjacent_Cell(cell, dir);
	CellClass * adjacentptr = &Map[adjacent];
	if (adjacentptr->IsBridgeDeck) {
		if (adjacentptr->OverlayData > OVERLAYDATA_BRIDGE_EW_DAMAGED) {
			if (adjacentptr->OverlayData == OVERLAYDATA_BRIDGE_EW_END1) {
				Internal_Destroy_High_Bridge_EW_TopLeft(adjacent, dir);
				adjacentptr->Set_Under_Bridge(FACING_N, false);
				adjacentptr->OverlayData = 0;
				adjacentptr->Overlay = OVERLAY_NONE;
				Map.Radar_Background(adjacentptr->CellID);
			}
		} else {
			adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_EW_END2;
		}
	}
	IsometricTileType ittype = IsometricTileType(adjacentptr->ITType - IsometricTileTypeClass::BridgeSet + 1);
	if (ittype != IsometricTileTypeClass::BridgeTopLeft1 && ittype != IsometricTileTypeClass::BridgeTopLeft2) {
		if (ittype == IsometricTileTypeClass::BridgeMiddle1 || ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_1) {
			Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2), ISOTILE_INVALID, -1, false);
		} else if (ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3) {
			Internal_Destroy_High_Bridge_EW_TopLeft(adjacent, dir);
			if ((adjacentptr->SubTile & 1) != 0) {
				Map[adjacent + Cell(-1, 0)].Destroy_Bridge();
				Map[adjacent + Cell(-1, -1)].Destroy_Bridge();
				Map[adjacent + Cell(-1, 1)].Destroy_Bridge();
				Set_Bridge_Middle_State(adjacent,  IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3), ISOTILE_INVALID, Map[adjacent + Cell(-1, 0)].Height - BRIDGE_CELL_HEIGHT, 0);
			} else {
				Map[adjacent + Cell(0, 0)].Destroy_Bridge();
				Map[adjacent + Cell(0, -1)].Destroy_Bridge();
				Map[adjacent + Cell(0, 1)].Destroy_Bridge();
				Set_Bridge_Middle_State(adjacent,  IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3), ISOTILE_INVALID, Map[adjacent].Height - BRIDGE_CELL_HEIGHT, 0);
			}
		}
	} else {
		Set_Bridge_End_State(adjacent, true, false);
	}
}


/// <summary>
/// Damages the bottom-left half of a north-south high bridge.
/// This routine ages one side of a span by a single stage, taking the deck and the tile
/// piece under it one step further toward ruin.
/// </summary>
/// <param name="cell">The cell being worked from; the piece dealt with is its neighbor in
/// the given direction.</param>
/// <param name="dir">The facing that leads to that piece.</param>
void MapClass::Internal_Damage_High_Bridge_NS_BottomLeft(Cell const & cell, FacingType dir)
{
	Cell adjacent = Adjacent_Cell(cell, dir);
	CellClass * adjacentptr = &Map[adjacent];
	if (adjacentptr->IsBridgeDeck) {
		if (adjacentptr->OverlayData >= OVERLAYDATA_BRIDGE_NS_FULL1) {
			if (adjacentptr->OverlayData > OVERLAYDATA_BRIDGE_NS_FULL4) {
				if (adjacentptr->OverlayData == OVERLAYDATA_BRIDGE_NS_TRANSITION1) {
					adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_NS_DAMAGED;
				}
			} else {
				adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_NS_TRANSITION2;
			}
		}
	}
	IsometricTileType ittype = IsometricTileType(adjacentptr->ITType - IsometricTileTypeClass::BridgeSet + 1);
	if (ittype == IsometricTileTypeClass::BridgeBottomLeft1 || ittype == IsometricTileTypeClass::BridgeBottomLeft2) {
		Set_Bridge_End_State(adjacent, true, false);
	} else if (ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_OK) {
		Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_OK), ISOTILE_INVALID, -1, false);
	} else if (ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2)  {
		Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2), ISOTILE_INVALID, -1, false);
	}
}


/// <summary>
/// Damages the top-right half of a north-south high bridge.
/// This routine ages one side of a span by a single stage, taking the deck and the tile
/// piece under it one step further toward ruin.
/// </summary>
/// <param name="cell">The cell being worked from; the piece dealt with is its neighbor in
/// the given direction.</param>
/// <param name="dir">The facing that leads to that piece.</param>
void MapClass::Internal_Damage_High_Bridge_NS_TopRight(Cell const & cell, FacingType dir)
{
	Cell adjacent = Adjacent_Cell(cell, dir);
	CellClass * adjacentptr = &Map[adjacent];
	if (adjacentptr->IsBridgeDeck) {
		if (adjacentptr->OverlayData >= OVERLAYDATA_BRIDGE_NS_FULL1) {
			if (adjacentptr->OverlayData > OVERLAYDATA_BRIDGE_NS_FULL4) {
				if (adjacentptr->OverlayData == OVERLAYDATA_BRIDGE_NS_TRANSITION2) {
					adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_NS_DAMAGED;
				}
			} else {
				adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_NS_TRANSITION1;
			}
		}
	}
	IsometricTileType ittype = IsometricTileType(adjacentptr->ITType - IsometricTileTypeClass::BridgeSet + 1);
	if (ittype == IsometricTileTypeClass::BridgeTopRight1 || ittype == IsometricTileTypeClass::BridgeTopRight2) {
		Set_Bridge_End_State(adjacent, true, false);
	} else if (ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_OK) {
		Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_1), ISOTILE_INVALID, -1, false);
	} else if (ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_1)  {
		Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2), ISOTILE_INVALID, -1, false);
	}
}


/// <summary>
/// Destroys the bottom-left half of a north-south high bridge.
/// This routine takes over once a span has been damaged past saving. It works its way
/// along the deck dropping each piece in turn, and lets the bridge cells back down onto
/// the ground they used to carry traffic over.
/// </summary>
/// <param name="cell">The cell being worked from; the piece dealt with is its neighbor in
/// the given direction.</param>
/// <param name="dir">The facing that leads to that piece, and on along the span.</param>
void MapClass::Internal_Destroy_High_Bridge_NS_BottomLeft(Cell const & cell, FacingType dir)
{
	Cell adjacent = Adjacent_Cell(cell, dir);
	CellClass * adjacentptr = &Map[adjacent];
	if (adjacentptr->IsBridgeDeck) {
		if (adjacentptr->OverlayData >= OVERLAYDATA_BRIDGE_NS_FULL1) {
			if (adjacentptr->OverlayData > OVERLAYDATA_BRIDGE_NS_DAMAGED) {
				if (adjacentptr->OverlayData == OVERLAYDATA_BRIDGE_NS_END1) {
					Internal_Destroy_High_Bridge_NS_BottomLeft(adjacent, dir);
					adjacentptr->Set_Under_Bridge(FACING_W, false);
					adjacentptr->OverlayData = 0;
					adjacentptr->Overlay = OVERLAY_NONE;
					Map.Radar_Background(adjacentptr->CellID);
				}
			} else {
				adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_NS_END2;
			}
		}
	}
	IsometricTileType ittype = IsometricTileType(adjacentptr->ITType - IsometricTileTypeClass::BridgeSet + 1);
	if (ittype != IsometricTileTypeClass::BridgeBottomLeft1 && ittype != IsometricTileTypeClass::BridgeBottomLeft2) {
		if (ittype == IsometricTileTypeClass::BridgeMiddle2 || ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2) {
			Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2), ISOTILE_INVALID, -1, false);
		} else if (ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3) {
			Internal_Destroy_High_Bridge_NS_BottomLeft(adjacent, dir);
			if (adjacentptr->SubTile > 4) {
				Map[adjacent + Cell(-1, -1)].Destroy_Bridge();
				Map[adjacent + Cell(0, -1)].Destroy_Bridge();
				Map[adjacent + Cell(1, -1)].Destroy_Bridge();
				Set_Bridge_Middle_State(adjacent,  IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3), ISOTILE_INVALID, Map[adjacent + Cell(0, -1)].Height - BRIDGE_CELL_HEIGHT, 0);
			} else {
				Map[adjacent + Cell(-1, 0)].Destroy_Bridge();
				Map[adjacent + Cell(0, 0)].Destroy_Bridge();
				Map[adjacent + Cell(1, 0)].Destroy_Bridge();
				Set_Bridge_Middle_State(adjacent,  IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3), ISOTILE_INVALID, Map[adjacent].Height - BRIDGE_CELL_HEIGHT, 0);
			}
		}
	} else {
		Set_Bridge_End_State(adjacent, true, false);
	}
}


/// <summary>
/// Destroys the top-right half of a north-south high bridge.
/// This routine takes over once a span has been damaged past saving. It works its way
/// along the deck dropping each piece in turn, and lets the bridge cells back down onto
/// the ground they used to carry traffic over.
/// </summary>
/// <param name="cell">The cell being worked from; the piece dealt with is its neighbor in
/// the given direction.</param>
/// <param name="dir">The facing that leads to that piece, and on along the span.</param>
void MapClass::Internal_Destroy_High_Bridge_NS_TopRight(Cell const & cell, FacingType dir)
{
	Cell adjacent = Adjacent_Cell(cell, dir);
	CellClass * adjacentptr = &Map[adjacent];
	if (adjacentptr->IsBridgeDeck) {
		if (adjacentptr->OverlayData >= OVERLAYDATA_BRIDGE_NS_FULL1) {
			if (adjacentptr->OverlayData > OVERLAYDATA_BRIDGE_NS_DAMAGED) {
				if (adjacentptr->OverlayData == OVERLAYDATA_BRIDGE_NS_END2) {
					Internal_Destroy_High_Bridge_NS_TopRight(adjacent, dir);
					adjacentptr->Set_Under_Bridge(FACING_W, false);
					adjacentptr->OverlayData = 0;
					adjacentptr->Overlay = OVERLAY_NONE;
					Map.Radar_Background(adjacentptr->CellID);
				}
			} else {
				adjacentptr->OverlayData = OVERLAYDATA_BRIDGE_NS_END1;
			}
		}
	}
	IsometricTileType ittype = IsometricTileType(adjacentptr->ITType - IsometricTileTypeClass::BridgeSet + 1);
	if (ittype != IsometricTileTypeClass::BridgeTopRight1 && ittype != IsometricTileTypeClass::BridgeTopRight2) {
		if (ittype == IsometricTileTypeClass::BridgeMiddle2 || ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_1) {
			Set_Bridge_Middle_State(adjacent, IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2), ISOTILE_INVALID, -1, false);
		} else if (ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3) {
			Internal_Destroy_High_Bridge_NS_TopRight(adjacent, dir);
			if (adjacentptr->SubTile > 4) {
				Map[adjacent + Cell(-1, -1)].Destroy_Bridge();
				Map[adjacent + Cell(0, -1)].Destroy_Bridge();
				Map[adjacent + Cell(1, -1)].Destroy_Bridge();
				Set_Bridge_Middle_State(adjacent,  IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3), ISOTILE_INVALID, Map[adjacent + Cell(0, -1)].Height - BRIDGE_CELL_HEIGHT, 0);
			} else {
				Map[adjacent + Cell(-1, 0)].Destroy_Bridge();
				Map[adjacent + Cell(0, 0)].Destroy_Bridge();
				Map[adjacent + Cell(1, 0)].Destroy_Bridge();
				Set_Bridge_Middle_State(adjacent,  IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3), ISOTILE_INVALID, Map[adjacent].Height - BRIDGE_CELL_HEIGHT, 0);
			}
		}
	} else {
		Set_Bridge_End_State(adjacent, true, false);
	}
}


/// <summary>
/// Repairs a broken bridge near the given cell.
/// This routine is used by an engineer sent out to a damaged bridge. It works out which
/// bridge the order meant -- low or high -- finds the span it belongs to, and puts the
/// deck back a section at a time until the crossing is whole again. It is the counterpart
/// of Repair_Train_Bridge.
/// </summary>
/// <param name="cell">Where the repair was ordered; the bridge itself may be a few cells
/// away.</param>
void MapClass::Repair_Bridge(Cell const & cell)
{
	/*
	 * Scan the 5x5 block of cells around the supplied cell looking for a low
	 * bridge overlay. If one is found, hand off to the low bridge repair routine
	 * and exit.
	 */
	for (int x = -2; x < 3; x++) {
		for (int y = -2; y < 3; y++) {
			if (Is_Low_Bridge(cell + Cell(x, y))) {
				Repair_Low_Bridge_Span(cell + Cell(x, y));
				return;
			}
		}
	}

	/*
	 * Locate a cell that is part of a bridge. Start at the supplied cell and, if it
	 * is not itself bridge related, walk up to three cells out along each of the
	 * eight facings until a bridge cell is found.
	 */
	CellClass * cellptr = &Map[cell];

	char facing = FACING_FIRST;
	if (!cellptr->IsUnderBridge && !cellptr->WasUnderBridge) {
		for (int index = 0; index < FACING_COUNT; index++) {

			Cell adjacent = Adjacent_Cell(cell, Facing_Add(facing, FACING_0));
			cellptr = &Map[adjacent];
			if (cellptr->IsUnderBridge || cellptr->WasUnderBridge) {
				break;
			}

			adjacent = Adjacent_Cell(adjacent, Facing_Add(facing, FACING_0));
			cellptr = &Map[adjacent];
			if (cellptr->IsUnderBridge || cellptr->WasUnderBridge) {
				break;
			}

			adjacent = Adjacent_Cell(adjacent, Facing_Add(facing, FACING_0));
			cellptr = &Map[adjacent];
			if (cellptr->IsUnderBridge || cellptr->WasUnderBridge) {
				break;
			}

			facing = Facing_Add(facing, FACING_45);
		}
	}

	/*
	 * Nothing bridge related found nearby - there is nothing to repair.
	 */
	if (!cellptr->IsUnderBridge && !cellptr->WasUnderBridge) {
		return;
	}

	/*
	 * Resolve the span anchor cell.
	 */
	Cell origin;
	if (cellptr->IsUnderBridge) {

		/*
		 * A live bridge cell points at the bridge owner (or is the owner).
		 */
		origin = cellptr->Get_Bridge_Deck_Cell();

	} else {

		/*
		 * An already damaged ("was under") bridge cell. Walk along the bridge body
		 * until a cell that is no longer marked as a former bridge cell is found,
		 * giving up after four steps, then step back two cells to recover the span
		 * end.
		 */
		int count = 0;
		int step = cellptr->IsBridgeEastWest ? FACING_S : FACING_E;
		Cell walk = cellptr->CellID;
		CellClass * walkptr;
		while (true) {
			walk = Adjacent_Cell(walk, Facing_Add(step, FACING_0));
			walkptr = &Map[walk];
			if (!walkptr->WasUnderBridge) {
				break;
			}
			if (++count >= 4) {
				return;
			}
		}

		FacingType back = Facing_Sub(step, FACING_180);
		origin = Adjacent_Cell(Adjacent_Cell(walk, back), back);
	}

	/*
	 * Walk the span looking for the end or middle tiles, repairing as we go.
	 */
	Cell spancell = origin;
	int advance = cellptr->IsBridgeEastWest ? FACING_W : FACING_N;

	char zonechanged = 0;
	IsometricTileType ittype = ISOTILE_CLEAR;

	DynamicVectorClass<Cell> cells;

	Rect dirty;

	/*
	 * Bail the moment the span steps outside the map rectangle.
	 */
	if (spancell.X >= MapRect.X) {

		while (true) {

			if (spancell.X > MapRect.X + MapRect.Width) {
				break;
			}
			if (spancell.Y < MapRect.Y || spancell.Y > MapRect.Y + MapRect.Height) {
				break;
			}

			if (Array[spancell.X + spancell.Y * MAP_CELL_W] != NULL) {

				CellClass * spanptr = &Map[spancell];
				ittype = IsometricTileType(spanptr->ITType - IsometricTileTypeClass::BridgeSet + 1);

				/*
				 * East/west bridge head - restore it and register the dirty area.
				 */
				if ((ittype == IsometricTileTypeClass::BridgeTopLeft1
						|| ittype == IsometricTileTypeClass::BridgeTopLeft2) && spanptr->SubTile == 8) {
					Set_Bridge_End_State(spancell, false, false);
					Repair_High_Bridge_Span(&Map[spancell], FACING_E, &dirty);
					TacticalMap->Register_Dirty_Area(dirty, false);
					break;
				}

				/*
				 * East/west bridge middle piece.
				 */
				if ((ittype == IsometricTileTypeClass::BridgeMiddle1
						|| ittype == IsometricTileTypeClass::BridgeMiddle1 + 3
						|| ittype == IsometricTileTypeClass::BridgeMiddle1 + 4
						|| ittype == IsometricTileTypeClass::BridgeMiddle1 + 1
						|| ittype == IsometricTileTypeClass::BridgeMiddle1 + 2) && spanptr->SubTile == 5) {

					if (ittype == IsometricTileTypeClass::BridgeMiddle1 + 4) {

						/*
						 * A fully destroyed middle - raise the three affected cells back
						 * to bridge height and queue them for recalculation.
						 */
						Set_Bridge_Middle_State(spancell, IsometricTileType(IsometricTileTypeClass::BridgeMiddle1 + IsometricTileTypeClass::BridgeSet - 1), ISOTILE_INVALID, -1, false);

						Cell middle = Adjacent_Cell(spancell, FACING_W);
						Map[middle].Height += BRIDGE_CELL_HEIGHT;
						Map[Adjacent_Cell(middle, FACING_N)].Height += BRIDGE_CELL_HEIGHT;
						Map[Adjacent_Cell(middle, FACING_S)].Height += BRIDGE_CELL_HEIGHT;

						zonechanged = Register_Subzone_Connections(middle);

						for (int dx = 0; dx < 2; dx++) {
							for (int dy = -2; dy < 3; dy++) {
								cells.Add(Cell(middle + Cell(dx, dy)));
							}
						}
					}

					Repair_Bridge(spancell + Cell(-2, 0));
					Repair_High_Bridge_Span(&Map[spancell], FACING_E, &dirty);
					TacticalMap->Register_Dirty_Area(dirty, false);
					break;
				}

				/*
				 * North/south bridge head - restore it and register the dirty area.
				 */
				if ((ittype == IsometricTileTypeClass::BridgeTopRight1
						|| ittype == IsometricTileTypeClass::BridgeTopRight2) && spanptr->SubTile == 12) {
					Set_Bridge_End_State(spancell, false, false);
					Repair_High_Bridge_Span(&Map[spancell], FACING_S, &dirty);
					TacticalMap->Register_Dirty_Area(dirty, false);
					break;
				}

				/*
				 * North/south bridge middle piece.
				 */
				if ((ittype == IsometricTileTypeClass::BridgeMiddle2
						|| ittype == IsometricTileTypeClass::BridgeMiddle2 + 3
						|| ittype == IsometricTileTypeClass::BridgeMiddle2 + 4
						|| ittype == IsometricTileTypeClass::BridgeMiddle2 + 1
						|| ittype == IsometricTileTypeClass::BridgeMiddle2 + 2) && spanptr->SubTile == 7) {

					if (ittype == IsometricTileTypeClass::BridgeMiddle2 + 4) {

						Set_Bridge_Middle_State(spancell, IsometricTileType(IsometricTileTypeClass::BridgeMiddle2 + IsometricTileTypeClass::BridgeSet - 1), ISOTILE_INVALID, -1, false);

						Cell middle = Adjacent_Cell(spancell, FACING_N);
						Map[middle].Height += BRIDGE_CELL_HEIGHT;
						Map[Adjacent_Cell(middle, FACING_E)].Height += BRIDGE_CELL_HEIGHT;
						Map[Adjacent_Cell(middle, FACING_W)].Height += BRIDGE_CELL_HEIGHT;

						zonechanged = Register_Subzone_Connections(middle);

						for (int dy = 0; dy < 2; dy++) {
							for (int dx = -2; dx < 3; dx++) {
								cells.Add(Cell(middle + Cell(dx, dy)));
							}
						}
					}

					Repair_Bridge(spancell + Cell(0, -2));
					Repair_High_Bridge_Span(&Map[spancell], FACING_S, &dirty);
					TacticalMap->Register_Dirty_Area(dirty, false);
					break;
				}
			}

			spancell = Adjacent_Cell(spancell, Facing_Add(advance, FACING_0));
			if (spancell.X < MapRect.X) {
				break;
			}
		}
	}

	if (zonechanged) {
		Zone_Reset();
	}

	if (cells.Count() > 0) {
		Recalc_Cells_In_List(cells);
	}
}


/// <summary>
/// Springs the bridge destruction triggers along a span.
/// Every tag lying on or beside the span is fired with TEVENT_BRIDGE_DESTROYED, so that a
/// scenario can react to a bridge coming down wherever along its length it was hit. Low
/// and high bridges are treated alike.
/// </summary>
/// <param name="cell1">One end of the span.</param>
/// <param name="cell2">The other end of the span.</param>
void MapClass::Spring_Bridge_Destruction_Triggers(Cell cell1, Cell cell2)
{
	bool is_east_west = false;
	Cell current = cell1;
	Cell end = cell2;

	if (cell1.Y == cell2.Y) {
		is_east_west = true;
		end = cell2;
		current = cell1;
		if (cell1.X > cell2.X) {
			end = cell1;
			current = cell2;
		}
	} else {
		if (cell1.Y > cell2.Y) {
			end = cell1;
			current = cell2;
		} else {
			end = cell2;
			current = cell1;
		}
	}

	while (current != end) {
		CellClass * cptr = &Map[current];
		if (cptr->Tag != NULL) {
			cptr->Tag->Spring(TEVENT_BRIDGE_DESTROYED);
		}

		if (is_east_west) {
			cptr = &Map[Adjacent_Cell(current, FACING_S)];
			if (cptr->Tag != NULL) {
				cptr->Tag->Spring(TEVENT_BRIDGE_DESTROYED);
			}
			cptr = &Map[Adjacent_Cell(current, FACING_N)];
			if (cptr->Tag != NULL) {
				cptr->Tag->Spring(TEVENT_BRIDGE_DESTROYED);
			}
			cptr = &Map[Adjacent_Cell(cptr->CellID, FACING_N)];
			if (cptr->Tag != NULL) {
				cptr->Tag->Spring(TEVENT_BRIDGE_DESTROYED);
			}
			current += Cell(1, 0);
		} else {
			cptr = &Map[Adjacent_Cell(current, FACING_E)];
			if (cptr->Tag != NULL) {
				cptr->Tag->Spring(TEVENT_BRIDGE_DESTROYED);
			}
			cptr = &Map[Adjacent_Cell(current, FACING_W)];
			if (cptr->Tag != NULL) {
				cptr->Tag->Spring(TEVENT_BRIDGE_DESTROYED);
			}
			cptr = &Map[Adjacent_Cell(cptr->CellID, FACING_W)];
			if (cptr->Tag != NULL) {
				cptr->Tag->Spring(TEVENT_BRIDGE_DESTROYED);
			}
			current += Cell(0, 1);
		}
	}
}


/// <summary>
/// Destroys one section of a high bridge.
/// This routine follows the deck along until it reaches the end of a section, then clears
/// the bridge away behind that end, springs the destruction triggers over the length of
/// it, and carries on into the next section. A span whose end is nowhere nearby is left
/// alone. It is the high bridge counterpart of Destroy_Train_Bridge_Span.
/// </summary>
/// <param name="dir">The direction to follow the deck -- FACING_E for an east/west bridge,
/// FACING_S for a north/south one.</param>
/// <param name="dirty">Gathers up the area that will need redrawing. May be NULL.</param>
/// <returns>bool; Was a span end found and dealt with?</returns>
bool MapClass::Destroy_High_Bridge_Span(Cell const & cell, FacingType dir, Rect * dirty)
{
	CellClass * cellptr = &Map[cell];

	int count = 1;

	CellClass * startptr;
	bool was_owner;
	Cell ownercell;
	bool sprung;
	int index;

	/*
	 * Remember the first adjacent cell of the span. It is used as one endpoint
	 * when springing the bridge destruction triggers.
	 */
	Cell spanstart = Adjacent_Cell(cellptr->Fetch_CellID(), dir);
	Cell lastcell = spanstart;

	IsometricTileType ittype;

	/*
	 * Walk along the span looking for the end tile, giving up after 30 cells.
	 */
	while (true) {
		lastcell = Adjacent_Cell(cellptr->Fetch_CellID(), dir);
		cellptr = &Map[lastcell];

		ittype = IsometricTileType(cellptr->ITType - IsometricTileTypeClass::BridgeSet + 1);

		if (dir == FACING_E) {

			if (ittype == IsometricTileTypeClass::BridgeBottomRight1 && cellptr->SubTile == 4
				|| ittype == IsometricTileTypeClass::BridgeBottomRight2 && cellptr->SubTile == 4
				|| (ittype == IsometricTileTypeClass::BridgeMiddle1
					|| ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3
					|| ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_1
					|| ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2)
					&& cellptr->SubTile == 4) {
				break;
			}

		} else if (dir == FACING_S
			&& (ittype == IsometricTileTypeClass::BridgeBottomLeft1 && cellptr->SubTile == 2
				|| ittype == IsometricTileTypeClass::BridgeBottomLeft2 && cellptr->SubTile == 2
				|| (ittype == IsometricTileTypeClass::BridgeMiddle2
					|| ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3
					|| ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_1
					|| ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2)
					&& cellptr->SubTile == 2)) {
			break;
		}

		if (++count >= 30) {
			break;
		}
	}

	/*
	 * If no span end was found within range, there is nothing to destroy.
	 */
	if (count != 30) {

		/*
		 * Re-fetch the span start cell. Its height anchors the dirty rectangle.
		 */
		startptr = &Map[cell];

		if (dirty != NULL) {

			/*
			 * Compute the screen rectangle spanning from the original cell to the
			 * located span end and union it into the supplied dirty rectangle.
			 */
			Point2D startpix;
			Coord coord;
			coord = Coord(cell, LEVEL_LEPTON_H * startptr->Height);
			TacticalMap->Coord_To_Pixel(coord, startpix);

			Point2D endpix;
			coord = Coord(lastcell, LEVEL_LEPTON_H * startptr->Height);
			TacticalMap->Coord_To_Pixel(coord, endpix);

			int minx = startpix.X;
			if (startpix.X >= endpix.X) {
				minx = endpix.X;
			}
			int miny = startpix.Y;
			if (startpix.Y >= endpix.Y) {
				miny = endpix.Y;
			}

			*dirty = Union(*dirty, Rect(minx - 64, miny - 64, abs(startpix.X - endpix.X) + 128, abs(startpix.Y - endpix.Y) + 128));
		}

		/*
		 * Walk the span again, springing the bridge destruction triggers and locating
		 * the transition from a bridge owner cell to a non-owner cell.
		 */
		was_owner = false;
		ownercell = Cell(-1, -1);
		sprung = false;
		for (index = 0; index < count; index++) {

			if (startptr->IsBridgeDeck && was_owner) {
				ownercell = startptr->CellID;
			}

			if (!startptr->IsBridgeDeck && !was_owner && ownercell != Cell(-1, -1)) {

				/*
				 * Found the span end owner. Clear the bridge state on the cell
				 * behind it, remove its overlay, refresh the radar background, then
				 * recurse to continue.
				 */
				Cell behind = Adjacent_Cell(startptr->Fetch_CellID(), Facing_Sub(dir, FACING_180));
				CellClass * behindptr = &Map[behind];

				behindptr->Set_Under_Bridge((dir == FACING_E) ? FACING_N : FACING_W, false);
				behindptr->OverlayData = 0;
				behindptr->Overlay = OVERLAY_NONE;
				Map.Radar_Background(behindptr->Fetch_CellID());

				Destroy_High_Bridge_Span(cell, dir, dirty);
				return(true);
			}

			was_owner = !startptr->IsBridgeDeck;

			if (was_owner && !sprung) {
				Spring_Bridge_Destruction_Triggers(spanstart, lastcell);
				sprung = true;
			}

			Cell next = Adjacent_Cell(startptr->Fetch_CellID(), dir);
			startptr = &Map[next];
		}
	}

	return(false);
}


/// <summary>
/// Tears down what is left of a high bridge.
/// Given a cell beside a span that has just come down, this routine finds the bridge it
/// belonged to and follows it out to its ends, taking the rest of the deck with it. It is
/// the high bridge counterpart of Destroy_Train_Bridge_Connections.
/// </summary>
/// <param name="cell">A cell next to the span that was destroyed.</param>
void MapClass::Destroy_High_Bridge_Connections(Cell const & cell)
{
	/*
	 * Scan the eight adjacent cells looking for the first one that is part of
	 * a bridge (either currently under a bridge or formerly under one).
	 */
	CellClass * cellptr = &BlubCell;
	FacingType facing = FACING_N;
	for (int index = 0; index < FACING_COUNT; index++) {
		cellptr = &Map[Adjacent_Cell(cell, facing)];
		if (cellptr->IsUnderBridge || cellptr->WasUnderBridge) {
			break;
		}
		facing = Facing_Add(facing, FACING_45);
	}

	/*
	 * If neither bridge flag is set on the located cell, there is nothing to do.
	 */
	if (!cellptr->IsUnderBridge && !cellptr->WasUnderBridge) {
		return;
	}

	Cell origin;
	if (cellptr->IsUnderBridge) {

		/*
		 * A live bridge cell points us at the bridge owner (or itself if it is
		 * the owner) which is the anchor used to walk the span.
		 */
		origin = cellptr->Get_Bridge_Deck_Cell();

	} else {

		/*
		 * An already-damaged ("was under") bridge cell. Walk along the bridge
		 * body until a cell that is no longer marked as a former bridge cell is
		 * found, giving up after four steps.
		 */
		Cell walk = cellptr->CellID;
		int count = 0;
		FacingType step = cellptr->IsBridgeEastWest ? FACING_S : FACING_E;
		CellClass * walkptr;
		while (true) {
			walk = Adjacent_Cell(walk, step);
			walkptr = &Map[walk];
			if (!walkptr->WasUnderBridge) {
				break;
			}
			if (++count >= 4) {
				return;
			}
		}

		/*
		 * Step two cells back in the opposite direction to recover the span end.
		 */
		FacingType back = Facing_Sub(step, FACING_180);
		origin = Adjacent_Cell(Adjacent_Cell(walk, back), back);
	}

	Cell current = origin;
	Rect dirty = RECT_NONE;
	FacingType advance = cellptr->IsBridgeEastWest ? FACING_W : FACING_N;

	while (true) {

		/*
		 * Bail out the moment we step outside the visible diamond playfield.
		 */
		if (!Map.In_Radar(current)) {
			return;
		}

		int cellnum = current.X + current.Y * MAP_CELL_W;
		if (Array[cellnum] != NULL) {

		CellClass * spanptr = &Map[current];
		IsometricTileType ittype = IsometricTileType(spanptr->ITType - IsometricTileTypeClass::BridgeSet + 1);

		if ((ittype == IsometricTileTypeClass::BridgeTopLeft1
				|| ittype == IsometricTileTypeClass::BridgeTopLeft2) && spanptr->SubTile == 8) {
			if (Destroy_High_Bridge_Span(current, FACING_E, &dirty)) {
				if (dirty != RECT_NONE) {
					TacticalMap->Register_Dirty_Area(dirty, false);
				}
			}
			return;
		}

		if ((ittype == IsometricTileTypeClass::BridgeMiddle1
				|| ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3
				|| ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_1
				|| ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2) && spanptr->SubTile == 5) {
			if (Destroy_High_Bridge_Span(current, FACING_E, &dirty)) {
				if (dirty != RECT_NONE) {
					TacticalMap->Register_Dirty_Area(dirty, false);
				}
			}
			return;
		}

		if ((ittype == IsometricTileTypeClass::BridgeTopRight1
				|| ittype == IsometricTileTypeClass::BridgeTopRight2) && spanptr->SubTile == 12) {
			if (Destroy_High_Bridge_Span(current, FACING_S, &dirty)) {
				if (dirty != RECT_NONE) {
					TacticalMap->Register_Dirty_Area(dirty, false);
				}
			}
			return;
		}

		if ((ittype == IsometricTileTypeClass::BridgeMiddle2
				|| ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3
				|| ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2
				|| ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_1) && spanptr->SubTile == 7) {
			break;
		}

		}

		current = Adjacent_Cell(current, advance);
	}

	if (Destroy_High_Bridge_Span(current, FACING_S, &dirty)) {
		if (dirty != RECT_NONE) {
			TacticalMap->Register_Dirty_Area(dirty, false);
		}
	}
}


/// <summary>
/// Damages the high bridge at a cell.
/// This routine is used by Damage_Bridge once the target turns out to be a concrete road
/// bridge. Each call ages the span one stage further -- whole, cracked, then gone -- and
/// when the last stage is reached the deck drops away, the destruction triggers spring,
/// and the movement zones are rebuilt around the gap.
/// </summary>
/// <param name="cell">The cell of the bridge, or of the ground beneath it.</param>
/// <returns>bool; Did the span come down this time?</returns>
bool MapClass::Damage_High_Bridge(Cell const & cell)
{
	CellClass * bridge_deck_cell = &Map[cell];
	IsometricTileType ittype = IsometricTileType(bridge_deck_cell->ITType - IsometricTileTypeClass::BridgeSet + 1);

	IsometricTileType bridge_middle_1 = IsometricTileTypeClass::BridgeMiddle1;
	IsometricTileType bridge_middle_2 = IsometricTileTypeClass::BridgeMiddle2;

	if (!bridge_deck_cell->IsUnderBridge) {

		if (ittype != bridge_middle_1 &&
			ittype != bridge_middle_1 + BRIDGE_MIDDLE_DAMAGED_1 &&
			ittype != bridge_middle_1 + BRIDGE_MIDDLE_DAMAGED_2 &&
			ittype != bridge_middle_1 + BRIDGE_MIDDLE_DAMAGED_3 &&
			ittype != bridge_middle_2 &&
			ittype != bridge_middle_2 + BRIDGE_MIDDLE_DAMAGED_1 &&
			ittype != bridge_middle_2 + BRIDGE_MIDDLE_DAMAGED_2 &&
			ittype != bridge_middle_2 + BRIDGE_MIDDLE_DAMAGED_3) {
			return(false);
		}
	}

	if (!bridge_deck_cell->IsUnderBridge) {

		int subtile = bridge_deck_cell->SubTile;
		if (ittype != IsometricTileTypeClass::BridgeMiddle1 &&
				ittype != IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3 &&
				ittype != IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_1 &&
				ittype != IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2) {

			if (subtile <= 4) {

				Cell middle_cell = bridge_deck_cell->CellID;
				while (subtile != 2) {
					if (subtile < 2) {
						middle_cell = Adjacent_Cell(middle_cell, FACING_E);
					} else {
						middle_cell = Adjacent_Cell(middle_cell, FACING_W);
					}
					subtile = Map[middle_cell].SubTile;
				}

				CellClass * middle_cellptr = &Map[middle_cell];

				if (ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3) {
					Cell zone_cell = middle_cellptr->CellID;

					if (middle_cellptr->SubTile > 4) {
						zone_cell.Y--;
						Map[Cell(middle_cell.X - 1, middle_cell.Y - 1)].Destroy_Bridge();
						Map[Cell(middle_cell.X, middle_cell.Y - 1)].Destroy_Bridge();
						Map[Cell(middle_cell.X + 1, middle_cell.Y - 1)].Destroy_Bridge();
						Cell cellid = middle_cellptr->Fetch_CellID();
						Set_Bridge_Middle_State(
							middle_cellptr->Fetch_CellID(),
							IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3),
							ISOTILE_INVALID,
							Map[Cell(cellid.X, cellid.Y - 1)].Height - BRIDGE_CELL_HEIGHT,
							false
						);
					} else {
						Map[Cell(middle_cell.X - 1, middle_cell.Y)].Destroy_Bridge();
						Map[Cell(middle_cell.X, middle_cell.Y)].Destroy_Bridge();
						Map[Cell(middle_cell.X + 1, middle_cell.Y)].Destroy_Bridge();
						Set_Bridge_Middle_State(
							middle_cellptr->Fetch_CellID(),
							IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_3),
							ISOTILE_INVALID,
							middle_cellptr->Height - BRIDGE_CELL_HEIGHT,
							false
						);
					}

					Internal_Destroy_High_Bridge_NS_BottomLeft(middle_cellptr->Fetch_CellID(), FACING_S);
					Internal_Destroy_High_Bridge_NS_TopRight(middle_cellptr->Fetch_CellID(), FACING_N);

					Destroy_High_Bridge_Connections(Adjacent_Cell(middle_cellptr->Fetch_CellID(), FACING_N));
					Destroy_High_Bridge_Connections(Adjacent_Cell(middle_cellptr->Fetch_CellID(), FACING_S));

					if (Unregister_Subzone_Connections(zone_cell)) {
						Zone_Reset();
					}

					DynamicVectorClass<Cell> cells;
					for (int y = 0; y < 2; y++) {
						for (int x = -2; x < 3; x++) {
							cells.Add(Cell(zone_cell.X + x, zone_cell.Y + y));
						}
					}
					Map.Recalc_Cells_In_List(cells);
					return(true);
				}

				if (ittype == IsometricTileTypeClass::BridgeMiddle2 ||
					ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_1 ||
					ittype == IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2) {

					Set_Bridge_Middle_State(
						middle_cellptr->Fetch_CellID(),
						IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle2 + BRIDGE_MIDDLE_DAMAGED_2),
						ISOTILE_INVALID,
						-1,
						false
					);
					Internal_Damage_High_Bridge_NS_BottomLeft(middle_cellptr->Fetch_CellID(), FACING_S);
					Internal_Damage_High_Bridge_NS_TopRight(middle_cellptr->Fetch_CellID(), FACING_N);
				}
			}

			return(false);
		}

		if ((subtile & 1) != 0) {
			return(false);
		}

		Cell middle_cell = bridge_deck_cell->CellID;
		while (subtile != 4) {
			if (subtile < 4) {
				middle_cell = Adjacent_Cell(middle_cell, FACING_S);
			} else {
				middle_cell = Adjacent_Cell(middle_cell, FACING_N);
			}
			subtile = Map[middle_cell].SubTile;
		}

		CellClass * middle_cellptr = &Map[middle_cell];

		if (ittype == IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3) {
			Cell zone_cell = middle_cellptr->CellID;

			if ((middle_cellptr->SubTile & 1) != 0) {
				zone_cell.X--;
				Map[Cell(middle_cell.X - 1, middle_cell.Y - 1)].Destroy_Bridge();
				Map[Cell(middle_cell.X - 1, middle_cell.Y)].Destroy_Bridge();
				Map[Cell(middle_cell.X - 1, middle_cell.Y + 1)].Destroy_Bridge();
				Cell cellid = middle_cellptr->Fetch_CellID();
				Set_Bridge_Middle_State(
					middle_cellptr->Fetch_CellID(),
					IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3),
					ISOTILE_INVALID,
					Map[Cell(cellid.X - 1, cellid.Y)].Height - BRIDGE_CELL_HEIGHT,
					false
				);
			} else {
				Map[Cell(middle_cell.X, middle_cell.Y - 1)].Destroy_Bridge();
				Map[Cell(middle_cell.X, middle_cell.Y)].Destroy_Bridge();
				Map[Cell(middle_cell.X, middle_cell.Y + 1)].Destroy_Bridge();
				Set_Bridge_Middle_State(
					middle_cellptr->Fetch_CellID(),
					IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_3),
					ISOTILE_INVALID,
					middle_cellptr->Height - BRIDGE_CELL_HEIGHT,
					false
				);
			}

			Internal_Destroy_High_Bridge_EW_BottomRight(middle_cellptr->Fetch_CellID(), FACING_E);
			Internal_Destroy_High_Bridge_EW_TopLeft(middle_cellptr->Fetch_CellID(), FACING_W);

			Destroy_High_Bridge_Connections(Adjacent_Cell(middle_cellptr->Fetch_CellID(), FACING_W));
			Destroy_High_Bridge_Connections(Adjacent_Cell(middle_cellptr->Fetch_CellID(), FACING_E));

			if (Unregister_Subzone_Connections(zone_cell)) {
				Zone_Reset();
			}

			DynamicVectorClass<Cell> cells;
			for (int x = 0; x < 2; x++) {
				for (int y = -2; y < 3; y++) {
					cells.Add(Cell(zone_cell.X + x, zone_cell.Y + y));
				}
			}
			Map.Recalc_Cells_In_List(cells);
			return(true);
		}

		if (ittype != IsometricTileTypeClass::BridgeMiddle1 &&
			ittype != IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_1 &&
			ittype != IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2) {
			return(false);
		}

		Set_Bridge_Middle_State(
			middle_cellptr->Fetch_CellID(),
			IsometricTileType(IsometricTileTypeClass::BridgeSet + IsometricTileTypeClass::BridgeMiddle1 + BRIDGE_MIDDLE_DAMAGED_2),
			ISOTILE_INVALID,
			-1,
			false
		);
		Internal_Damage_High_Bridge_EW_BottomRight(middle_cellptr->Fetch_CellID(), FACING_E);
		Internal_Damage_High_Bridge_EW_TopLeft(middle_cellptr->Fetch_CellID(), FACING_W);

		return(false);
	} else {
		if (!bridge_deck_cell->IsBridgeDeck) {
			bridge_deck_cell = bridge_deck_cell->BridgeDeckCell;
		}

		int overlay_data = bridge_deck_cell->OverlayData;
		FacingType dir = overlay_data < OVERLAYDATA_BRIDGE_NS_FULL1 ? FACING_E : FACING_S;

		switch (overlay_data) {
			case OVERLAYDATA_BRIDGE_EW_FULL1:
			case OVERLAYDATA_BRIDGE_EW_FULL2:
			case OVERLAYDATA_BRIDGE_EW_FULL3:
			case OVERLAYDATA_BRIDGE_EW_FULL4:
			case OVERLAYDATA_BRIDGE_EW_TRANSITION1:
			case OVERLAYDATA_BRIDGE_EW_TRANSITION2:
				bridge_deck_cell->OverlayData = OVERLAYDATA_BRIDGE_EW_DAMAGED;
				Internal_Damage_High_Bridge_EW_BottomRight(bridge_deck_cell->Fetch_CellID(), dir);
				Internal_Damage_High_Bridge_EW_TopLeft(bridge_deck_cell->Fetch_CellID(), Facing_Sub(dir, FACING_180));
				return(false);

			case OVERLAYDATA_BRIDGE_EW_DAMAGED:
				Internal_Destroy_High_Bridge_EW_BottomRight(bridge_deck_cell->Fetch_CellID(), dir);
				Internal_Destroy_High_Bridge_EW_TopLeft(bridge_deck_cell->Fetch_CellID(), Facing_Sub(dir, FACING_180));
				bridge_deck_cell->Set_Under_Bridge(FACING_N, false);
				bridge_deck_cell->OverlayData = 0;
				bridge_deck_cell->Overlay = OVERLAY_NONE;
				Destroy_High_Bridge_Connections(cell);
				break;

			case OVERLAYDATA_BRIDGE_EW_END1:
				Internal_Destroy_High_Bridge_EW_BottomRight(bridge_deck_cell->Fetch_CellID(), dir);
				bridge_deck_cell->Set_Under_Bridge(FACING_N, false);
				bridge_deck_cell->OverlayData = 0;
				bridge_deck_cell->Overlay = OVERLAY_NONE;
				Destroy_High_Bridge_Connections(cell);
				break;

			case OVERLAYDATA_BRIDGE_EW_END2:
				Internal_Destroy_High_Bridge_EW_TopLeft(bridge_deck_cell->Fetch_CellID(), Facing_Sub(dir, FACING_180));
				bridge_deck_cell->Set_Under_Bridge(FACING_N, false);
				bridge_deck_cell->OverlayData = 0;
				bridge_deck_cell->Overlay = OVERLAY_NONE;
				Destroy_High_Bridge_Connections(cell);
				break;

			case OVERLAYDATA_BRIDGE_NS_FULL1:
			case OVERLAYDATA_BRIDGE_NS_FULL2:
			case OVERLAYDATA_BRIDGE_NS_FULL3:
			case OVERLAYDATA_BRIDGE_NS_FULL4:
			case OVERLAYDATA_BRIDGE_NS_TRANSITION1:
			case OVERLAYDATA_BRIDGE_NS_TRANSITION2:
				bridge_deck_cell->OverlayData = OVERLAYDATA_BRIDGE_NS_DAMAGED;
				Internal_Damage_High_Bridge_NS_BottomLeft(bridge_deck_cell->Fetch_CellID(), dir);
				Internal_Damage_High_Bridge_NS_TopRight(bridge_deck_cell->Fetch_CellID(), Facing_Sub(dir, FACING_180));
				return(false);

			case OVERLAYDATA_BRIDGE_NS_DAMAGED:
				Internal_Destroy_High_Bridge_NS_BottomLeft(bridge_deck_cell->Fetch_CellID(), dir);
				Internal_Destroy_High_Bridge_NS_TopRight(bridge_deck_cell->Fetch_CellID(), Facing_Sub(dir, FACING_180));
				bridge_deck_cell->Set_Under_Bridge(FACING_W, false);
				bridge_deck_cell->OverlayData = 0;
				bridge_deck_cell->Overlay = OVERLAY_NONE;
				Destroy_High_Bridge_Connections(cell);
				break;

			case OVERLAYDATA_BRIDGE_NS_END1:
				Internal_Destroy_High_Bridge_NS_TopRight(bridge_deck_cell->Fetch_CellID(), Facing_Sub(dir, FACING_180));
				bridge_deck_cell->Set_Under_Bridge(FACING_W, false);
				bridge_deck_cell->OverlayData = 0;
				bridge_deck_cell->Overlay = OVERLAY_NONE;
				Destroy_High_Bridge_Connections(cell);
				break;

			case OVERLAYDATA_BRIDGE_NS_END2:
				Internal_Destroy_High_Bridge_NS_BottomLeft(bridge_deck_cell->Fetch_CellID(), dir);
				bridge_deck_cell->Set_Under_Bridge(FACING_W, false);
				bridge_deck_cell->OverlayData = 0;
				bridge_deck_cell->Overlay = OVERLAY_NONE;
				Destroy_High_Bridge_Connections(cell);
				break;

			default:
				return(false);
		}

		if (Unregister_Subzone_Connections(bridge_deck_cell->CellID)) {
			Zone_Reset();
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * MapClass::Detach -- Remove specified object from map references.                            *
 *                                                                                             *
 *    This routine will take the object (represented by a target value) and remove all         *
 *    references to it from the map. Typically, this is used to remove trigger reference.      *
 *                                                                                             *
 * INPUT:   target   -- The target object to remove from the map.                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void MapClass::Detach(AbstractClass const * target, bool all)
{
	if (target->RTTI == RTTI_TAG) {
		for (int index = 0; index < TaggedCells.Count(); index++) {
			CellClass *cellptr = &(*this)[TaggedCells[index]];
			if (cellptr != NULL && cellptr->Tag == (TagClass *)target) {
				cellptr->Attach_Tag(NULL);
				TaggedCells.Delete(cellptr->CellID);
				index--;
			}
		}
		BlubCell.Detach(target);
	}

	if (target->RTTI == RTTI_TAG) {
		MapTags.Delete((TagClass *)target);
	}
}


/***********************************************************************************************
 * MapClass::Pick_Random_Location -- Picks a random location on the map.                       *
 *                                                                                             *
 *    This routine will pick a random location on the map. It performs no legality checking    *
 *    other than forcing the cell to be on the map proper.                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a cell that is within the map.                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/25/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
Cell MapClass::Pick_Random_Location(void) const
{
	int x = Map.MapRect.X + Random_Pick(0, Map.MapRect.Width-1);
	int y = Map.MapRect.Y + Random_Pick(0, Map.MapRect.Height-1);
	return(Cell(x, y));
}


/***********************************************************************************************
 * MapClass::Shroud_The_Map -- cover the whole map in darkness (usually from blackout crate)   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a cell that is within the map.                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/19/1996 BWG : Created.                                                                 *
 *=============================================================================================*/
void MapClass::Shroud_The_Map(void)
{
	Reset_Iterator();
	CellClass *cellptr = Iterate();

	while (cellptr != NULL) {
		cellptr->IsMapped = false;
		cellptr->IsVisible = false;
		cellptr->IsFogMapped = false;
		cellptr->IsFogVisible = false;

		cellptr = Iterate();
	}

	Map.All_To_Look();
	PlayerPtr->IsVisionary = false;
	Map.Complete_Radar_Refresh();
	Flag_To_Redraw(GS_REDRAW_ALL);
}


/// <summary>
/// Reveals the whole map to the player.
/// This routine is used by the reveal crate, by the reveal team mission, and by the
/// player's own defeat, which leaves them free to watch the rest of the game play out.
/// A player who can see everything already is left alone, unless the fog is to go as well.
/// </summary>
/// <param name="unfog">Lift the fog of war too, and discard the stand-ins fogged structures leave behind.</param>
void MapClass::Reveal_The_Map(bool unfog)
{
	if (!PlayerPtr->IsVisionary || unfog) {
		PlayerPtr->IsVisionary = true;
		Map.Reset_Iterator();
		CellClass *cellptr = Map.Iterate();
		while (cellptr != NULL) {
			if (unfog) {
				Map.Map_Cell(cellptr->CellID, PlayerPtr);
			} else {
				Map.Shadow_Map_Cell(cellptr->CellID, PlayerPtr);
			}
			cellptr = Map.Iterate();
		}

		Map.Complete_Radar_Refresh();
		Map.Flag_To_Redraw(GS_REDRAW_TACTICAL);
	}
}


/// <summary>
/// Fetches the height of the ground at a world coordinate.
/// </summary>
/// <param name="coord">World coordinate to query.</param>
/// <returns>Returns with the height of the ground at that coordinate.</returns>
int MapClass::Get_Height_GL(Coord const & coord)
{
	return(Map[coord].Get_Height(coord));
}


/// <summary>
/// Fetches the next cell of the playable map.
/// This routine walks every cell inside the play area, one call at a time. Call
/// Reset_Iterator first, then keep calling this until it comes back empty handed.
/// </summary>
/// <returns>Returns with a pointer to the current cell. Otherwise, NULL is
/// returned.</returns>
CellClass * MapClass::Iterate(void)
{
	/*
	 * Save the current iterator cell to return later.
	 */
	CellClass ** iter = IterCell;

	/*
	 * If we're currently iterating a row, move to the next cell (diagonally)
	 * and mark that we've got one cell fewer left in the row.
	 */
	if (IterColumn != 0) {
		IterX++;
		IterY--;
		IterColumn--;
		IterCell = iter - (MAP_CELL_W - 1);
		return(*iter);
	}

	/*
	 * We've reached the end of the row,
	 * swap X and Y to reset to the beginning of the row.
	 */
	unsigned x = IterX;
	unsigned y = IterY;
	IterX = y;
	IterY = x;

	/*
	 * Since we're iterating a diamond, its rows vary in size by 1.
	 * It also impacts which way we need to shift the iterator
	 * to change to the next row.
	 */
	if (((IterX + IterY) - (PlayRect.Width + 1)) % 2) {
		IterY++;
		IterColumn = PlayRect.Width - 1;
	} else {
		IterX++;
		IterColumn = PlayRect.Width - 2;
	}

	/*
	 * Advance the iterated cell pointer.
	 */
	IterCell = &Array[IterX + (IterY * MAP_CELL_H)];

	return(*iter);
}


/// <summary>
/// Resets the full-map diamond iterator to the start of the PlayRect.
/// Initializes IterX/IterY, the per-row column counter, and the iterator cell pointer.
/// </summary>
void MapClass::Reset_Iterator(void)
{
	IterX = 1;
	IterY = PlayRect.Width;
	IterColumn = PlayRect.Width - 1;
	IterCell = &Array[IterX + (IterY * MAP_CELL_H)];
}


/// <summary>
/// Tests whether an entire rectangle lies within the local radar (visible tactical) area.
/// Returns true only if all four corner cells are within the local radar.
/// </summary>
/// <param name="rect">Cell-space rectangle to test.</param>
/// <param name="useheight">If true, account for cell height/ramps when testing each corner.</param>
/// <returns>True if all four corners of the rectangle are within the local radar area.</returns>
bool MapClass::In_Local_Radar(Rect const & rect, bool useheight) const
{
	return(In_Local_Radar(Cell(rect.X, rect.Y), useheight) &&
		In_Local_Radar(Cell(rect.X + rect.Width - 1, rect.Y), useheight) &&
		In_Local_Radar(Cell(rect.X, rect.Y + rect.Height - 1), useheight) &&
		In_Local_Radar(Cell(rect.X + rect.Width - 1, rect.Y + rect.Height - 1), useheight) ? 1 : 0);
}


/// <summary>
/// Determines if a cell lies within the visible area.
/// The visible area is a diamond, so this is not the simple rectangle test it might look
/// like. Elevated ground is drawn higher up the screen, so a tall cell can come into view
/// while the flat ground beside it is still off the bottom edge.
/// </summary>
/// <param name="useheight">Should the cell's elevation be taken into account?</param>
/// <returns>bool; Is the cell within the visible area?</returns>
bool MapClass::In_Local_Radar(Cell const & cell, bool useheight) const
{
	int x = cell.X;
	int y = cell.Y;
	int cell_height = 0;

	if (useheight) {
		CellClass *cellptr = &Map[cell];

		cell_height = cellptr->Height;
		/// fudge ramps at the top of the map so that they end up considered not in the local rect
		if (cellptr->Ramp && x + y < PlayRect.Width + 2*LocalRect.Y + 4 + cell_height) {
			cell_height++;
		}
	}

	if ((x + y > PlayRect.Width + 2*LocalRect.Y + cell_height) &&
		(x + y <= PlayRect.Width + 2*(LocalRect.Y + LocalRect.Height + 1) + cell_height) &&
		(x - y < 2*(LocalRect.X + LocalRect.Width) - PlayRect.Width) &&
		(y - x < PlayRect.Width - 2*LocalRect.X)) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Determines if a cell lies within the visible area.
/// This overload takes the cell object itself, which spares the caller a lookup when it
/// already has one in hand.
/// </summary>
/// <param name="useheight">Should the cell's elevation be taken into account?</param>
/// <returns>bool; Is the cell within the visible area?</returns>
bool MapClass::In_Local_Radar(CellClass const * cell, bool useheight) const
{
	int x = cell->CellID.X;
	int y = cell->CellID.Y;
	int cell_height = 0;
	if (useheight) {

		cell_height = cell->Height;
		/// fudge ramps at the top of the map so that they end up considered not in the local rect
		if (cell->Ramp && x + y < PlayRect.Width + 2*LocalRect.Y + 4 + cell_height) {
			cell_height++;
		}
	}

	if ((x + y > PlayRect.Width + 2*LocalRect.Y + cell_height) &&
		(x + y <= PlayRect.Width + 2*(LocalRect.Y + LocalRect.Height + 1) + cell_height) &&
		(x - y < 2*(LocalRect.X + LocalRect.Width) - PlayRect.Width) &&
		(y - x < PlayRect.Width - 2*LocalRect.X)) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Determines if a coordinate lies within the visible area.
/// This overload spares the caller the conversion when all it has is a world coordinate.
/// The elevation of the ground under the coordinate is not considered.
/// </summary>
/// <returns>bool; Is the coordinate within the visible area?</returns>
bool MapClass::In_Local_Radar(Coord const & coord) const
{
	return(In_Local_Radar(coord.As_Cell()));
}


/// <summary>
/// Determines if a cell lies within the given view area.
/// This is In_Local_Radar measured against a view rectangle of the caller's choosing
/// rather than the one the player is looking at.
/// </summary>
/// <param name="useheight">Should the cell's elevation be taken into account?</param>
/// <returns>bool; Is the cell within the area?</returns>
bool MapClass::In_Area_Radar(Rect const & rect, Cell const & cell, bool useheight) const
{
	int x = cell.X;
	int y = cell.Y;
	int cell_height = 0;
	if (useheight) {
		CellClass *cellptr = &Map[cell];

		cell_height = cellptr->Height;

		/// fudge ramps at the top of the map so that they end up considered not in the local rect
		if (cellptr->Ramp && x + y < PlayRect.Width + 2*rect.Y + 4 + cell_height) {
			cell_height++;
		}
	}

	if ((x + y > PlayRect.Width + 2*rect.Y + cell_height) &&
		(x + y <= PlayRect.Width + 2*(rect.Y + rect.Height + 1) + cell_height) &&
		(x - y < 2*(rect.X + rect.Width) - PlayRect.Width) &&
		(y - x < PlayRect.Width - 2*rect.X)) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the next cell of the visible area.
/// This routine walks the cells the player can actually see, one call at a time. Call
/// Reset_Local_Iterator first, then keep calling this until it comes back empty handed.
/// </summary>
/// <returns>Returns with a pointer to the next visible cell. Otherwise, NULL is
/// returned.</returns>
CellClass * MapClass::Local_Iterate(void)
{
	while (LocalIterX + LocalIterY <= PlayRect.Width + 2 * (LocalRect.Height + LocalRect.Y) + 12) {

		int x = LocalIterX;
		int y = LocalIterY;

		Cell cell(x, y);

		if (x - y >= 2 * (LocalRect.X + LocalRect.Width) - PlayRect.Width - 2) {
			int t = ((LocalIterX - PlayRect.Width) + LocalIterY - 1) & 1;
			bool odd = (t == 1);

			int delta = odd ? 2 : 1;

			int w = LocalRect.Width;
			w -= delta;

			LocalIterX -= w;
			LocalIterY += w;

			if (odd) {
				LocalIterY++;
			} else {
				LocalIterX++;
			}
		} else {
			LocalIterX++;
			LocalIterY--;
		}

		if (In_Local_Radar(cell, true)) {
			return(&(*this)[cell]);
		}
	}

	return(NULL);
}


/// <summary>
/// Resets the local-view iterator to the start of the local radar rectangle.
/// Initializes LocalIterX/LocalIterY from the LocalRect and PlayRect dimensions.
/// </summary>
void MapClass::Reset_Local_Iterator(void)
{
	LocalIterX = LocalRect.X + LocalRect.Y + 1;
	LocalIterY = PlayRect.Width - LocalRect.X + LocalRect.Y;
}


/// <summary>
/// Snaps a cell onto a coarse grid aligned to the local view.
/// The grid is anchored to the top corner of the visible area, so neighboring cells all
/// collapse onto a common sample point. How far down the view the cell sits can pull that
/// sample back toward the near edge.
/// </summary>
/// <param name="spacing">The grid spacing in cells. A spacing of one leaves the cell be.</param>
/// <param name="unbiased">Should the grid point be taken as it stands, without the pull toward
/// the near edge?</param>
/// <returns>Returns with a pointer to the cell the sample landed on.</returns>
CellClass * MapClass::Get_Local_Grid_Cell(Cell const & cell, int spacing, bool unbiased)
{
	if (spacing == 1) {
		return(&Map[cell]);
	}

	int x = cell.X;
	int y = cell.Y;
	Cell local(LocalRect.X + LocalRect.Y + 1, PlayRect.Width - LocalRect.X + LocalRect.Y);
	int xx = (6000 * spacing + local.X - x) % spacing;
	int yy = (6000 * spacing + local.Y - y) % spacing;

	if (xx > 0) {
		x += spacing - xx;
	}
	if (yy > 0) {
		y += spacing - yy;
	}

	int dsum = local.X + local.Y;
	if (unbiased || (y + x < dsum + 2 * LocalRect.Height / 3)) {
		return(&Map[Cell(x, y)]);
	} else if (y + x < dsum + 4 * LocalRect.Height / 3 && spacing > 2) {
		return(&Map[Cell(x, y) - Cell((spacing / 2), (spacing / 2))]);
	} else {
		return(&Map[Cell(x, y) - Cell((spacing - 1), (spacing - 1))]);
	}
}


/// <summary>
/// Deforms the terrain at a cell into a crater.
/// This routine is used when a sizeable explosion goes off. One randomly chosen corner of
/// the cell slumps at once and the rest is left to follow a few frames later, so the ground
/// appears to cave in rather than snap into its new shape. Cells at the edge of the
/// playable area are left well alone.
/// </summary>
/// <param name="forced">Should the ground be reshaped even where that is normally
/// refused?</param>
/// <returns>bool; Was the terrain deformed?</returns>
bool MapClass::Deform_Terrain(Cell const & cell, bool forced)
{
	FacingType face = FACING_FIRST;
	for (int i = 0; i < FACING_COUNT; i++) {
		if (!Map.In_Radar(Adjacent_Cell(cell, face))) {
			return(false);
		}
		face++;
		face = Facing_Add(face, FACING_0);
	}

	int mask = 1 << Random_Pick(0, 3);

	if (Deform_Cell(cell, -1, forced, mask)) {
		DeformMask = mask ^ 15;
		DeformCell = cell;
		DeformFrame = Frame + 5;
		return(true);
	}
	return(false);
}


/// <summary>
/// Handles the second half of a terrain deformation.
/// This routine is called once per game frame. Deform_Terrain leaves the remainder of a
/// crater to settle a few frames after the blast that started it, and this is where that
/// deferred work is carried out.
/// </summary>
void MapClass::Terrain_Deformation_AI(void)
{
	if (DeformMask != 0 && DeformFrame == Frame) {
		Deform_Cell(DeformCell, -1, false, DeformMask);
		DeformMask = 0;
	}
}

/// <summary>
/// Increments the map's redraw counter (Redraws).
/// </summary>
void MapClass::Increment_Redraw_Counter(void)
{
	Redraws++;
}


/// <summary>
/// Asks any gate in the cell to let a unit through.
/// This routine is used by the locomotors as they come up on a cell. A friendly gate is
/// told to open; an enemy gate is only worth trying if it happens to be standing open
/// already, and the unit will have to wait it out otherwise.
/// </summary>
/// <param name="foot">The unit asking to pass.</param>
/// <returns>bool; May the unit enter the cell?</returns>
bool MapClass::Try_Open_Gate(FootClass * foot, Cell const & cell)
{
	FootClass * optr = (FootClass *)Map[cell].Cell_Occupier();
	while (optr != NULL) {

		if (foot != optr) {

			BuildingClass * bptr = (BuildingClass *)optr;
			if (bptr->RTTI == RTTI_BUILDING && bptr->Class->IsGate) {

				if (bptr->House->Is_Ally(foot)) {
					return(bptr->Open_Gate());
				}

				if (bptr->Is_Gate_Open()) {
					return(true);
				}
			}
		}

		optr = (FootClass *)optr->Next;
	}
	return(true);
}

/*
 * Ice logic.
 * Ice consists of 3 tile sets.
 * Tiles 0-15 are normal full ice tiles.
 * Tile 16 is cracked ice.
 * Tiles 17-63 are ice shores.
 */


/// <summary>
/// Re-dresses a cell of ice and the ice around it.
/// This is the routine the rest of the ice code calls once it has changed a cell. The cell
/// and its neighbors are each given whichever full ice, edge or shore tile now suits them,
/// which keeps the sheet looking whole however it was cut about.
/// </summary>
/// <param name="smooth_shore">Should the shoreline around the cell be redressed too?</param>
void MapClass::Smoothen_Ice(Cell const & cell, bool smooth_shore)
{
	if (TheaterClass::As_Reference(Scen->Theater).IsIceGrowth) {
		IsometricTileType ice1 = IsometricTileTypeClass::Ice1Set;
		IsometricTileType ice1_cracked = IsometricTileType(IsometricTileTypeClass::Ice1Set + ICE_CRACKED);
		IsometricTileType ice1_edge = IsometricTileType(IsometricTileTypeClass::Ice1Set + ICE_EDGE);
		IsometricTileType ice1_end = IsometricTileType(IsometricTileTypeClass::Ice1Set + ICE_COUNT);
		IsometricTileType ice2 = IsometricTileTypeClass::Ice2Set;
		IsometricTileType ice2_cracked = IsometricTileType(IsometricTileTypeClass::Ice2Set + ICE_CRACKED);
		IsometricTileType ice2_edge = IsometricTileType(IsometricTileTypeClass::Ice2Set + ICE_EDGE);
		IsometricTileType ice2_end = IsometricTileType(IsometricTileTypeClass::Ice2Set + ICE_COUNT);
		IsometricTileType ice3 = IsometricTileTypeClass::Ice3Set;
		IsometricTileType ice3_cracked = IsometricTileType(IsometricTileTypeClass::Ice3Set + ICE_CRACKED);
		IsometricTileType ice3_edge = IsometricTileType(IsometricTileTypeClass::Ice3Set + ICE_EDGE);
		IsometricTileType ice3_end = IsometricTileType(IsometricTileTypeClass::Ice3Set + ICE_COUNT);

		IsometricTileType tile = Map[cell].ITType;
		DirtyIceCells.Delete(cell);
		DirtyIceCells.Add(cell);

		if (tile == ice1 || tile == ice2 || tile == ice3) {
			for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir = FacingType(dir + FACING_90)) {
				if (Map[Adjacent_Cell(cell, dir)].Is_Tile_Shore()) {
					Map[cell].ITType = ice1_edge;
					tile = ice1_edge;
					break;
				}
			}
		}

		if (tile == ice1 || tile == ice2 || tile == ice3) {
			Smoothen_Full_Ice(cell + Cell(0, -1), false);
			Smoothen_Full_Ice(cell + Cell(0, 1), false);
			Smoothen_Full_Ice(cell + Cell(1, 0), false);
			Smoothen_Full_Ice(cell + Cell(-1, 0), false);
		}

		if (tile >= ice1 && tile <= ice1_cracked || tile >= ice2 && tile <= ice2_cracked || tile >= ice3 && tile <= ice3_cracked) {
			Smoothen_Full_Ice(cell, false);
		} else if (tile >= ice1_edge && tile < ice1_end || tile >= ice2_edge && tile < ice2_end || tile >= ice3_edge && tile < ice3_end) {
			Smoothen_Ice_Edge(cell);
		}

		if (tile != ice1 && tile != ice2 && tile != ice3) {
			Smoothen_Full_Ice(cell + Cell(0, -1), false);
			Smoothen_Full_Ice(cell + Cell(0, 1), false);
			Smoothen_Full_Ice(cell + Cell(1, 0), false);
			Smoothen_Full_Ice(cell + Cell(-1, 0), false);
		}

		{ /// scope to hide variable declaration
			for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
				Smoothen_Ice_Edge(Map[cell].Adjacent_Cell(dir).Fetch_CellID());
			}
		}

		if (smooth_shore) {
			for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
				Smoothen_Ice_Shore(Map[cell].Adjacent_Cell(dir).Fetch_CellID());
			}
		}
	}
}


/// <summary>
/// Picks the solid ice tile that suits a cell's surroundings.
/// Full ice comes in variants according to which of the four neighboring cells are ice as
/// well, so that a sheet reads as one surface rather than a patchwork of squares.
/// </summary>
/// <param name="not_cracked">Should a cell of cracked ice be left as it is?</param>
void MapClass::Smoothen_Full_Ice(Cell const & cell, bool not_cracked)
{
	if (TheaterClass::As_Reference(Scen->Theater).IsIceGrowth) {
		int index = 0;

		IsometricTileType ice1_cracked = IsometricTileType(IsometricTileTypeClass::Ice1Set + ICE_CRACKED);
		IsometricTileType ice2_cracked = IsometricTileType(IsometricTileTypeClass::Ice2Set + ICE_CRACKED);
		IsometricTileType ice3_cracked = IsometricTileType(IsometricTileTypeClass::Ice3Set + ICE_CRACKED);

		IsometricTileType oldtile = Map[cell].ITType;
		if (oldtile < IsometricTileTypeClass::Ice1Set ||
			!not_cracked && oldtile == ice1_cracked ||
			oldtile > ice1_cracked && oldtile < IsometricTileTypeClass::Ice2Set ||
			!not_cracked && oldtile == ice2_cracked ||
			oldtile > ice2_cracked && oldtile < IsometricTileTypeClass::Ice3Set ||
			!not_cracked && oldtile == ice3_cracked ||
			oldtile > ice3_cracked) {

			return;
		}

		IsometricTileType tile_n = Map[cell + Cell(0, -1)].ITType;
		IsometricTileType tile_s = Map[cell + Cell(0, 1)].ITType;
		IsometricTileType tile_w = Map[cell + Cell(1, 0)].ITType;
		IsometricTileType tile_e = Map[cell + Cell(-1, 0)].ITType;

		/// North neighbor
		if (tile_n < IsometricTileTypeClass::Ice1Set ||
			(tile_n >= IsometricTileTypeClass::Ice1Set + ICE_CRACKED && tile_n < IsometricTileTypeClass::Ice2Set) ||
			(tile_n >= IsometricTileTypeClass::Ice2Set + ICE_CRACKED && tile_n < IsometricTileTypeClass::Ice3Set) ||
			tile_n >= IsometricTileTypeClass::Ice3Set + ICE_CRACKED) {
			index |= 1; /// bit 0 for North
		}

		/// South neighbor
		if (tile_s < IsometricTileTypeClass::Ice1Set ||
			(tile_s >= IsometricTileTypeClass::Ice1Set + ICE_CRACKED && tile_s < IsometricTileTypeClass::Ice2Set) ||
			(tile_s >= IsometricTileTypeClass::Ice2Set + ICE_CRACKED && tile_s < IsometricTileTypeClass::Ice3Set) ||
			tile_s >= IsometricTileTypeClass::Ice3Set + ICE_CRACKED) {
			index |= 4; /// bit 2 for South
		}

		/// West neighbor
		if (tile_w < IsometricTileTypeClass::Ice1Set ||
			(tile_w >= IsometricTileTypeClass::Ice1Set + ICE_CRACKED && tile_w < IsometricTileTypeClass::Ice2Set) ||
			(tile_w >= IsometricTileTypeClass::Ice2Set + ICE_CRACKED && tile_w < IsometricTileTypeClass::Ice3Set) ||
			tile_w >= IsometricTileTypeClass::Ice3Set + ICE_CRACKED) {
			index |= 2; /// bit 1 for West
		}

		/// East neighbor
		if (tile_e < IsometricTileTypeClass::Ice1Set ||
			(tile_e >= IsometricTileTypeClass::Ice1Set + ICE_CRACKED && tile_e < IsometricTileTypeClass::Ice2Set) ||
			(tile_e >= IsometricTileTypeClass::Ice2Set + ICE_CRACKED && tile_e < IsometricTileTypeClass::Ice3Set) ||
			tile_e >= IsometricTileTypeClass::Ice3Set + ICE_CRACKED) {
			index |= 8; /// bit 3 for East
		}

		if (index > 0 && IsometricTileTypeClass::Ice1Set + index + 1 != oldtile && IsometricTileTypeClass::Ice2Set + index + 1 != oldtile && IsometricTileTypeClass::Ice3Set + index + 1 != oldtile ||
			index == 0 && oldtile != IsometricTileTypeClass::Ice1Set && oldtile != IsometricTileTypeClass::Ice1Set + 1 && oldtile != IsometricTileTypeClass::Ice2Set && oldtile != IsometricTileTypeClass::Ice2Set + 1 && oldtile != IsometricTileTypeClass::Ice3Set && oldtile != IsometricTileTypeClass::Ice3Set + 1) {

			Rect dirty = Map[cell].Cell_Render_Rect();

			IsometricTileType set = Pick_Ice_Tile_Set();
			Map[cell].ITType = IsometricTileType(index + set + (index != 0 ? 1 : 0));
			Map[cell].SubTile = 0;
			Map[cell].Recalc_Attributes();

			dirty = Union(dirty, Map[cell].Cell_Render_Rect());
			dirty.Y -= TacticalRect.Y;
			TacticalMap->Register_Dirty_Area(dirty);

			DirtyIceCells.Delete(cell);
			DirtyIceCells.Add(cell);
		}
	}
}


/*
 * Used to smoothen ice tiles.
 * Positive numbers are shores.
 * -1 means no shore piece suits the cell. The shore smoother leaves such a cell alone;
 * the full ice smoother turns it into plain ice.
 * -2 is a random non-shore ice tile.
 */
char const IceSetLut[] = {
	-1, 33, 2,  2,  34, 37, 2,  2,
	4,  26, 6,  6,  4,  26, 6,  6,
	35, 45, 17, 17, 38, 41, 17, 17,
	4,  26, 6,  6,  4,  26, 6,  6,
	8,  21, 10, 10, 27, 31, 10, 10,
	12, 23, 14, 14, 12, 23, 14, 14,
	8,  21, 10, 10, 27, 31, 10, 10,
	12, 23, 14, 14, 12, 23, 14, 14,
	32, 36, 25, 25, 44, 40, 25, 25,
	19, 30, 20, 20, 19, 30, 20, 20,
	39, 43, 29, 29, 42, 46, 29, 29,
	19, 30, 20, 20, 19, 30, 20, 20,
	8,  21, 10, 10, 27, 31, 10, 10,
	12, 23, 14, 14, 12, 23, 14, 14,
	8,  21, 10, 10, 27, 31, 10, 10,
	12, 23, 14, 14, 12, 23, 14, 14,
	1,  1,  3,  3,  16, 16, 3,  3,
	5,  5,  7,  7,  5,  5,  7,  7,
	24, 24, 18, 18, 28, 28, 18, 18,
	5,  5,  7,  7,  5,  5,  7,  7,
	9,  9,  11, 11, 22, 22, 11, 11,
	13, 13, -2, -2, 13, 13, -2, -2,
	9,  9,  11, 11, 22, 22, 11, 11,
	13, 13, -2, -2, 13, 13, -2, -2,
	1,  1,  3,  3,  16, 16, 3,  3,
	5,  5,  7,  7,  5,  5,  7,  7,
	24, 24, 18, 18, 28, 28, 18, 18,
	5,  5,  7,  7,  5,  5,  7,  7,
	9,  9,  11, 11, 22, 22, 11, 11,
	13, 13, -2, -2, 13, 13, -2, -2,
	9,  9,  11, 11, 22, 22, 11, 11,
	13, 13, -2, -2, 13, 13, -2, -2
};


/// <summary>
/// Picks the shore tile that suits a cell's surroundings.
/// This routine dresses the land where it runs up against an ice sheet. A cell that ends
/// up hemmed in by ice on every side stops being a shore at all and becomes plain ice.
/// </summary>
void MapClass::Smoothen_Ice_Shore(Cell const & cell)
{
	if (TheaterClass::As_Reference(Scen->Theater).IsIceGrowth) {
		int index = 0;

		CellClass *cptr = &Map[cell];

		IsometricTileType ice3 = IsometricTileTypeClass::Ice3Set;
		IsometricTileType iceshore = IsometricTileTypeClass::IceShoreSet;
		IsometricTileType oldtile = cptr->ITType;
		IsometricTileType ice3_count = IsometricTileType(ice3 + ICE3_COUNT);
		if (oldtile != ISOTILE_NONE && oldtile != ISOTILE_CLEAR && (oldtile < iceshore || oldtile >= iceshore + ICE_SHORE_COUNT)) {
			return;
		}

		IsometricTileType tile_n = Map[cell + Cell(0, -1)].ITType;
		IsometricTileType tile_s = Map[cell + Cell(0, 1)].ITType;
		IsometricTileType tile_w = Map[cell + Cell(1, 0)].ITType;
		IsometricTileType tile_e = Map[cell + Cell(-1, 0)].ITType;
		IsometricTileType tile_ne = Map[cell + Cell(1, -1)].ITType;
		IsometricTileType tile_nw = Map[cell + Cell(-1, -1)].ITType;
		IsometricTileType tile_se = Map[cell + Cell(1, 1)].ITType;
		IsometricTileType tile_sw = Map[cell + Cell(-1, 1)].ITType;

		if (tile_ne >= IsometricTileTypeClass::Ice1Set && tile_ne < ice3_count) {
			index |= MASK_FACINGF_NE;
		}
		if (tile_w >= IsometricTileTypeClass::Ice1Set && tile_w < ice3_count) {
			index |= MASK_FACINGF_W;
		}
		if (tile_se >= IsometricTileTypeClass::Ice1Set && tile_se < ice3_count) {
			index |= MASK_FACINGF_SE;
		}
		if (tile_s >= IsometricTileTypeClass::Ice1Set && tile_s < ice3_count) {
			index |= MASK_FACINGF_S;
		}
		if (tile_sw >= IsometricTileTypeClass::Ice1Set && tile_sw < ice3_count) {
			index |= MASK_FACINGF_SW;
		}
		if (tile_e >= IsometricTileTypeClass::Ice1Set && tile_e < ice3_count) {
			index |= MASK_FACINGF_E;
		}
		if (tile_nw >= IsometricTileTypeClass::Ice1Set && tile_nw < ice3_count) {
			index |= MASK_FACINGF_NW;
		}
		if (tile_n >= IsometricTileTypeClass::Ice1Set && tile_n < ice3_count) {
			index |= MASK_FACINGF_N;
		}

		IsometricTileType newtile = (IsometricTileType)IceSetLut[index];
		if (newtile < ISOTILE_FIRST) {
			if (newtile != ISOTILE_INVALID_CLIFF) {
				return;
			}

			Rect dirty = Map[cell].Cell_Render_Rect();

			Map[cell].ITType = Pick_Ice_Tile_Set();
			Map[cell].SubTile = 0;
			Map[cell].Recalc_Attributes();

			Smoothen_Ice(cell, true);
			dirty = Union(dirty, Map[cell].Cell_Render_Rect());
			dirty.Y -= TacticalRect.Y;
			TacticalMap->Register_Dirty_Area(dirty);

			DirtyIceCells.Delete(cell);
			DirtyIceCells.Add(cell);

		} else {
			if (oldtile == newtile + IsometricTileTypeClass::IceShoreSet) {
				return;
			}

			Rect dirty = Map[cell].Cell_Render_Rect();

			Map[cell].ITType = IsometricTileType(newtile + IsometricTileTypeClass::IceShoreSet);
			Map[cell].SubTile = 0;
			Map[cell].Recalc_Attributes();

			dirty = Union(dirty, Map[cell].Cell_Render_Rect());
			dirty.Y -= TacticalRect.Y;
			TacticalMap->Register_Dirty_Area(dirty);

			DirtyIceCells.Delete(cell);
			DirtyIceCells.Add(cell);
		}
	}
}


/// <summary>
/// Picks the ice edge tile that suits a cell's surroundings.
/// This routine dresses the boundary where an ice sheet meets the water it floats in, so
/// that the two read as one continuous surface after the ice has grown or been broken.
/// </summary>
void MapClass::Smoothen_Ice_Edge(Cell const & cell)
{
	if (TheaterClass::As_Reference(Scen->Theater).IsIceGrowth) {
		int index = 0;

		IsometricTileType oldtile = Map[cell].ITType;

		IsometricTileType ice1_edge = IsometricTileType(IsometricTileTypeClass::Ice1Set + ICE_EDGE);
		IsometricTileType ice1_end = IsometricTileType(IsometricTileTypeClass::Ice1Set + ICE_COUNT);
		IsometricTileType ice2_edge = IsometricTileType(IsometricTileTypeClass::Ice2Set + ICE_EDGE);
		IsometricTileType ice2_end = IsometricTileType(IsometricTileTypeClass::Ice2Set + ICE_COUNT);
		IsometricTileType ice3_edge = IsometricTileType(IsometricTileTypeClass::Ice3Set + ICE_EDGE);
		IsometricTileType ice3_end = IsometricTileType(IsometricTileTypeClass::Ice3Set + ICE_COUNT);
		IsometricTileType water_end = IsometricTileType(IsometricTileTypeClass::WaterSet + WATER_COUNT);
		IsometricTileType shore_end = IsometricTileType(IsometricTileTypeClass::ShorePieces + SHORE_PIECES_COUNT);

		if ((oldtile < IsometricTileTypeClass::WaterSet || oldtile >= IsometricTileTypeClass::WaterSet + WATER_COUNT) &&
			(oldtile < (IsometricTileTypeClass::Ice1Set + ICE_EDGE) || oldtile >= (IsometricTileTypeClass::Ice1Set + ICE_COUNT)) &&
			(oldtile < ice2_edge || oldtile >= ice2_end) &&
			(oldtile < ice3_edge || oldtile >= ice3_end)) {
			return;
		}

		IsometricTileType tile_n = Map[cell + Cell(0, -1)].ITType;
		IsometricTileType tile_s = Map[cell + Cell(0, 1)].ITType;
		IsometricTileType tile_w = Map[cell + Cell(1, 0)].ITType;
		IsometricTileType tile_e = Map[cell + Cell(-1, 0)].ITType;
		IsometricTileType tile_ne = Map[cell + Cell(1, -1)].ITType;
		IsometricTileType tile_nw = Map[cell + Cell(-1, -1)].ITType;
		IsometricTileType tile_se = Map[cell + Cell(1, 1)].ITType;
		IsometricTileType tile_sw = Map[cell + Cell(-1, 1)].ITType;

		if ((tile_ne < IsometricTileTypeClass::WaterSet || tile_ne >= water_end) &&
			(tile_ne < ice1_edge || tile_ne >= ice1_end) &&
			(tile_ne < ice2_edge || tile_ne >= ice2_end) &&
			(tile_ne < ice3_edge || tile_ne >= ice3_end) &&
			(tile_ne < IsometricTileTypeClass::ShorePieces || tile_ne >= shore_end)) {

			index |= MASK_FACINGF_NE;
		}

		if ((tile_w < IsometricTileTypeClass::WaterSet || tile_w >= water_end) &&
			(tile_w < ice1_edge || tile_w >= ice1_end) &&
			(tile_w < ice2_edge || tile_w >= ice2_end) &&
			(tile_w < ice3_edge || tile_w >= ice3_end) &&
			(tile_w < IsometricTileTypeClass::ShorePieces || tile_w >= shore_end)) {

			index |= MASK_FACINGF_W;
		}

		if ((tile_se < IsometricTileTypeClass::WaterSet || tile_se >= water_end) &&
			(tile_se < ice1_edge || tile_se >= ice1_end) &&
			(tile_se < ice2_edge || tile_se >= ice2_end) &&
			(tile_se < ice3_edge || tile_se >= ice3_end) &&
			(tile_se < IsometricTileTypeClass::ShorePieces || tile_se >= shore_end)) {

			index |= MASK_FACINGF_SE;
		}

		if ((tile_s < IsometricTileTypeClass::WaterSet || tile_s >= water_end) &&
			(tile_s < ice1_edge || tile_s >= ice1_end) &&
			(tile_s < ice2_edge || tile_s >= ice2_end) &&
			(tile_s < ice3_edge || tile_s >= ice3_end) &&
			(tile_s < IsometricTileTypeClass::ShorePieces || tile_s >= shore_end)) {

			index |= MASK_FACINGF_S;
		}

		if ((tile_sw < IsometricTileTypeClass::WaterSet || tile_sw >= water_end) &&
			(tile_sw < ice1_edge || tile_sw >= ice1_end) &&
			(tile_sw < ice2_edge || tile_sw >= ice2_end) &&
			(tile_sw < ice3_edge || tile_sw >= ice3_end) &&
			(tile_sw < IsometricTileTypeClass::ShorePieces || tile_sw >= shore_end)) {

			index |= MASK_FACINGF_SW;
		}

		if ((tile_e < IsometricTileTypeClass::WaterSet || tile_e >= water_end) &&
			(tile_e < ice1_edge || tile_e >= ice1_end) &&
			(tile_e < ice2_edge || tile_e >= ice2_end) &&
			(tile_e < ice3_edge || tile_e >= ice3_end) &&
			(tile_e < IsometricTileTypeClass::ShorePieces || tile_e >= shore_end)) {

			index |= MASK_FACINGF_E;
		}

		if ((tile_nw < IsometricTileTypeClass::WaterSet || tile_nw >= water_end) &&
			(tile_nw < ice1_edge || tile_nw >= ice1_end) &&
			(tile_nw < ice2_edge || tile_nw >= ice2_end) &&
			(tile_nw < ice3_edge || tile_nw >= ice3_end) &&
			(tile_nw < IsometricTileTypeClass::ShorePieces || tile_nw >= shore_end)) {

			index |= MASK_FACINGF_NW;
		}

		if ((tile_n < IsometricTileTypeClass::WaterSet || tile_n >= water_end) &&
			(tile_n < ice1_edge || tile_n >= ice1_end) &&
			(tile_n < ice2_edge || tile_n >= ice2_end) &&
			(tile_n < ice3_edge || tile_n >= ice3_end) &&
			(tile_n < IsometricTileTypeClass::ShorePieces || tile_n >= shore_end)) {

			index |= MASK_FACINGF_N;
		}

		IsometricTileType newtile = (IsometricTileType)IceSetLut[index];
		if (newtile == ISOTILE_INVALID_CLIFF) {
			newtile = (IsometricTileType)(ICE_CRACKED-1);
		}

		if (newtile < ISOTILE_FIRST) {
			Rect dirty = Map[cell].Cell_Render_Rect();

			Map[cell].ITType = IsometricTileType(Pick_Ice_Tile_Set() + ICE_COUNT - 1);
			Map[cell].SubTile = 0;
			Map[cell].Recalc_Attributes();

			dirty = Union(dirty, Map[cell].Cell_Render_Rect());
			dirty.Y -= TacticalRect.Y;
			TacticalMap->Register_Dirty_Area(dirty);

			DirtyIceCells.Delete(cell);
			DirtyIceCells.Add(cell);

		} else {
			if (oldtile != newtile + ice1_edge - 1 && oldtile != newtile + ice2_edge - 1) {
				Rect dirty = Map[cell].Cell_Render_Rect();

				IsometricTileType set = Pick_Ice_Tile_Set();
				Map[cell].ITType = IsometricTileType(set + newtile + ICE_CRACKED);
				Map[cell].SubTile = 0;
				Map[cell].Recalc_Attributes();

				dirty = Union(dirty, Map[cell].Cell_Render_Rect());
				dirty.Y -= TacticalRect.Y;
				TacticalMap->Register_Dirty_Area(dirty);

				DirtyIceCells.Delete(cell);
				DirtyIceCells.Add(cell);

				Smoothen_Full_Ice(cell + Cell(0, -1), false);
				Smoothen_Full_Ice(cell + Cell(0, 1), false);
				Smoothen_Full_Ice(cell + Cell(1, 0), false);
				Smoothen_Full_Ice(cell + Cell(-1, 0), false);
			}
		}
	}
}


/// <summary>
/// Cracks the ice at a cell.
/// This is what the ice does the first time something heavy crosses it -- it splits, makes
/// a suitably alarming noise, and is left to refreeze in its own time. Cross the same cell
/// again before it has healed and the ice gives way altogether.
/// </summary>
/// <param name="cellptr">The cell being crossed.</param>
/// <param name="object">The object crossing the ice. Nothing happens if it is riding a
/// bridge over the top. May be NULL.</param>
/// <returns>bool; Did the ice give way?</returns>
bool MapClass::Crack_Ice(CellClass * cellptr, FootClass * object)
{
	if (TheaterClass::As_Reference(Scen->Theater).IsIceGrowth) {
		if (object == NULL || !object->IsOnBridge) {
			IsometricTileType tile = cellptr->ITType;
			IsometricTileType ice1end = IsometricTileType(IsometricTileTypeClass::Ice1Set + ICE_CRACKED);
			IsometricTileType ice2end = IsometricTileType(IsometricTileTypeClass::Ice2Set + ICE_CRACKED);
			IsometricTileType ice3end = IsometricTileType(IsometricTileTypeClass::Ice3Set + ICE_CRACKED);

			if (tile != ice1end && tile != ice2end && tile != ice3end) {
				if ((tile >= IsometricTileTypeClass::Ice1Set && tile < ice1end) || (tile >= IsometricTileTypeClass::Ice2Set && tile < ice2end) || (tile >= IsometricTileTypeClass::Ice3Set && tile < ice3end)) {
					cellptr->ITType = IsometricTileType(Pick_Ice_Tile_Set() + ICE_CRACKED);
					cellptr->SubTile = 0;
					cellptr->Recalc_Attributes();

					Smoothen_Ice(cellptr->Fetch_CellID(), false);
					cellptr->Recalc_Attributes();

					CrackedIceStruct cracked_ice;
					cracked_ice.CellID = cellptr->Fetch_CellID();
					cracked_ice.SolidifyFrame = Frame + Rule->IceSolidifyDelay;
					CrackedIce.Add(cracked_ice);

					Rect dirty = cellptr->Cell_Render_Rect();
					dirty.Y -= TacticalRect.Y;
					TacticalMap->Register_Dirty_Area(dirty);

					if (Rule->IceCrackSounds.Count() > 0) {
						Sound_Effect((VocType)Rule->IceCrackSounds.Pick(Scen->RandomNumber()));
					}
				}
				return(false);
			}

			return(Break_Ice(cellptr, object));
		}
	}

	return(false);
}


/// <summary>
/// Tests whether the given isometric tile type is water or any ice tile.
/// </summary>
/// <param name="tile">Isometric tile type to test.</param>
/// <returns>True if the tile is in the water set or any ice set; otherwise false.</returns>
bool MapClass::Is_Tile_Water_Ice(IsometricTileType tile)
{
	int ice_start = IsometricTileTypeClass::Ice1Set;
	int ice_end = IsometricTileTypeClass::Ice3Set + ICE_COUNT;
	if (IS_TILE_IN_SET(WaterSet, WATER_COUNT) || (tile >= ice_start && tile < ice_end)) return(true);
	return(false);
}


/// <summary>
/// Breaks a patch of ice open into water.
/// A two by two block of ice, laid out ahead of whatever broke it, gives way and becomes
/// open water. Anything caught standing on it goes with it -- vehicles that cannot swim
/// start sinking, and infantry and aircraft are lost outright.
/// </summary>
/// <param name="cellptr">The corner cell of the block that gives way.</param>
/// <param name="object">The object breaking the ice; its facing decides which way the
/// block lies. May be NULL.</param>
/// <returns>bool; Was the ice broken?</returns>
bool MapClass::Break_Ice(CellClass * cellptr, FootClass * object)
{
	static const Point2D _directions[] = {
		Point2D(1,-1),
		Point2D(1,-1),
		Point2D(1,1),
		Point2D(1,1),
		Point2D(1,1),
		Point2D(-1,1),
		Point2D(-1,1),
		Point2D(-1,-1),
		Point2D(0,0)
	};

	if (TheaterClass::As_Reference(Scen->Theater).IsIceGrowth) {
		FacingType facing = FACING_NONE;
		if (object != NULL) {
			if (object->IsOnBridge || object->Is_Moving_Onto_Bridge()) {
				return(false);
			}
			facing = object->PrimaryFacing.Current().As_Dir8();
		} else {
			facing = FACING_COUNT;
		}

		//Point2D direction = _directions[facing];
		Cell cell = cellptr->Fetch_CellID();

		int x, y;
		for (x = 0; x < 2; x++) {
			for (y = 0; y < 2; y++) {
				Cell newcell = Cell(x * _directions[facing].X, y * _directions[facing].Y) + cell;
				CellClass * newcellptr = &Map[newcell];
				if (!Is_Tile_Water_Ice(newcellptr->ITType)) {
					return(false);
				}
			}
		}

		for (x = 0; x < 2; x++) {
			for (y = 0; y < 2; y++) {
				Cell newcell = Cell(x * _directions[facing].X, y * _directions[facing].Y) + cell;
				CellClass * newcellptr = &Map[newcell];

				newcellptr->ITType = IsometricTileType(Pick_Ice_Tile_Set() + ICE_EDGE + 1);
				newcellptr->SubTile = 0;
				newcellptr->Recalc_Attributes();

				Smoothen_Ice(newcellptr->Fetch_CellID(), false);
				newcellptr->Recalc_Attributes();
			}
		}

		for (x = 0; x < 2; x++) {
			for (y = 0; y < 2; y++) {
				Cell newcell = Cell(x * _directions[facing].X, y * _directions[facing].Y) + cell;
				CellClass * newcellptr = &Map[newcell];

				ObjectClass * occupier = newcellptr->Cell_Occupier();
				while (occupier != NULL) {
					FootClass * foot = dynamic_cast<FootClass *>(occupier);
					occupier = occupier->Next;

					if (foot != NULL) {
						MZoneType mzone = foot->TClass->MZone;
						if (foot->RTTI != RTTI_AIRCRAFT && foot->RTTI != RTTI_INFANTRY) {
							if (mzone != MZONE_AMPHIBIOUS_DESTROYER && mzone != MZONE_AMPHIBIOUS_CRUSHER && mzone != MZONE_AMPHIBIOUS) {
								foot->IsSinking = true;
								foot->Stun();
								new AnimClass(Rule->Wake, foot->PositionCoord);
							}
						} else {
							if (foot->IsActive && foot->Tag != NULL) {
								foot->Tag->Spring(TEVENT_DESTROYED_ANY, foot);
							}
							if (foot->IsActive && foot->Tag != NULL) {
								foot->Tag->Spring(TEVENT_DESTROYED_ANY_X, foot);
							}
							foot->Delete_Me();
							new AnimClass(Rule->Wake, foot->PositionCoord);
						}
					}
				}
			}
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Handles the spread of ice across the water.
/// This routine is called once per game frame. Where the scenario allows ice to grow, the
/// thin tiles at the fringe of a sheet thicken into full ice and the surrounding tiles are
/// re-dressed to suit, so the sheet creeps outward as the mission wears on.
/// </summary>
/// <returns>bool; Did any ice grow this frame?</returns>
bool MapClass::Ice_Growth_AI(void)
{
	if (TheaterClass::As_Reference(Scen->Theater).IsIceGrowth && Scen->IsIceGrowth) {
		IsometricTileType & ice1 = IsometricTileTypeClass::Ice1Set;
		IsometricTileType & ice2 = IsometricTileTypeClass::Ice2Set;
		IsometricTileType ice1_lat = IsometricTileType(ice1 + ICE_LAT);
		IsometricTileType ice1_end = IsometricTileType(ice1 + ICE_COUNT-1);
		IsometricTileType ice11 = ice1;
		IsometricTileType ice2_end = IsometricTileType(ice2 + ICE_COUNT-1);
		IsometricTileType ice2_lat = IsometricTileType(ice2 + ICE_LAT);
		IsometricTileType ice1_cracked = IsometricTileType(ice1 + ICE_CRACKED);
		IsometricTileType ice2_cracked = IsometricTileType(ice2 + ICE_CRACKED);

		bool smoothed = false;

		Reset_Iterator();
		CellClass * iter = Iterate();
		while (iter != NULL) {
			IsometricTileType tile = iter->ITType;
			if (iter->IsIceGrowthAllowed) {
				if (tile != ice1_cracked && tile != ice2_cracked && (tile >= ice1_lat && tile < ice1_end || tile >= ice2_lat && tile < ice2_end)) {
					iter->IsToGrowIce = true;
				}
			}
			iter = Iterate();
		}

		Reset_Iterator();
		iter = Iterate();
		while (iter != NULL) {
			if (iter->IsToGrowIce) {
				IsometricTileType tile = iter->ITType;
				if (tile > ice1_cracked && tile < ice2_lat || tile > ice2_cracked) {
					iter->ITType = ice11;
					Smoothen_Ice(iter->Fetch_CellID(), false);
					smoothed = true;
				}
				iter->IsToGrowIce = false;
			}
			iter = Iterate();
		}
		return(smoothed);
	}
	return(false);
}


/// <summary>
/// Handles the refreezing of cracked ice.
/// This routine is called once per game frame. Ice that was cracked by something crossing
/// it heals back to solid after IceSolidifyDelay has passed, taking any cracked neighbors
/// along with it. Away from the snow theater there is no ice to look after.
/// </summary>
void MapClass::Ice_Solidification_AI(void)
{
	if (!TheaterClass::As_Reference(Scen->Theater).IsIceGrowth) {
		CrackedIce.Clear();
		return;
	}

	if (CrackedIce.Count() && Scen->IsIceGrowth) {
		IsometricTileType ice1 = IsometricTileTypeClass::Ice1Set;
		IsometricTileType ice2 = IsometricTileTypeClass::Ice2Set;
		IsometricTileType ice3 = IsometricTileTypeClass::Ice3Set;

		IsometricTileType ice1_cracked = IsometricTileType(IsometricTileTypeClass::Ice1Set + ICE_CRACKED);
		IsometricTileType ice2_cracked = IsometricTileType(IsometricTileTypeClass::Ice2Set + ICE_CRACKED);
		IsometricTileType ice3_cracked = IsometricTileType(IsometricTileTypeClass::Ice3Set + ICE_CRACKED);

		for (int i = CrackedIce.Count() - 1; i >= 0; i--) {
			Cell cracked_cell = CrackedIce[i].CellID;
			int solidify_frame = CrackedIce[i].SolidifyFrame;
			CellClass * cellptr = &Map[cracked_cell];

			if (solidify_frame < Frame) {
				if (cellptr->ITType > ice1 && cellptr->ITType < ice1 + ICE_EDGE || cellptr->ITType > ice2 && cellptr->ITType < ice2 + ICE_EDGE || cellptr->ITType > ice3 && cellptr->ITType < ice3 + ICE_EDGE) {
					Map[cracked_cell].ITType = Pick_Ice_Tile_Set();
					for (int facing = FACING_FIRST; facing < FACING_COUNT; facing += FACING_90) {
						CellClass * adjacent = &Map[cracked_cell].Adjacent_Cell(FacingType(facing));
						if (adjacent->ITType == ice1_cracked || adjacent->ITType == ice2_cracked || adjacent->ITType == ice3_cracked) {
							adjacent->ITType = Pick_Ice_Tile_Set();
						}
					}

					Smoothen_Ice(cracked_cell, false);

					Rect dirty = Map[cracked_cell].Cell_Render_Rect();
					dirty.Y -= TacticalRect.Y;
					TacticalMap->Register_Dirty_Area(dirty);

					Map[cracked_cell].Recalc_Attributes();

					for (int ffacing = FACING_FIRST; ffacing < FACING_COUNT; ffacing += FACING_90) {
						CellClass * adjacent = &Map[cracked_cell].Adjacent_Cell(FacingType(ffacing));
						Smoothen_Ice(adjacent->Fetch_CellID(), false);
						if (adjacent->ITType == ice1 || adjacent->ITType == ice2 || adjacent->ITType == ice3) {
							dirty = adjacent->Cell_Render_Rect();
							dirty.Y -= TacticalRect.Y;
							TacticalMap->Register_Dirty_Area(dirty);
						}
					}

					CrackedIce.Delete_Index(i);
				}
			}
		}
	}
}


/// <summary>
/// Maps a cell to its 3x3 sector index within the local view rectangle.
/// Divides the local rect into a 3x3 grid and returns the 1-based sector number for the cell.
/// </summary>
/// <param name="cell">Cell to classify.</param>
/// <returns>1-based sector index (1-9) within the local 3x3 grid.</returns>
int MapClass::Cell_To_Local_Sector_Index(Cell const & cell)
{
	int w1 = (2 * LocalRect.Width - 1) % 3;
	int w2 = (2 * LocalRect.Width - 1) / 3;

	int h1 = (2 * LocalRect.Height - 1) % 3;
	int h2 = (2 * LocalRect.Height - 1) / 3;

	int x1 = LocalRect.X + LocalRect.Y + 1;
	int y1 = PlayRect.Width - LocalRect.X + LocalRect.Y;

	Cell pos = Cell(x1, y1);

	int x2 = (cell.X + (pos.Y - (w1 ? 1 : 0) - pos.X - cell.Y)) / w2;
	int y2 = (cell.X + (cell.Y - (h1 ? 1 : 0) - pos.X - pos.Y)) / h2;

	if (x2 == 3) {
		x2 = 2;
	}
	if (y2 == 3) {
		y2 = 2;
	}

	return(x2 + 3 * y2 + 1);
}


/// <summary>
/// Fetches the cell that stands for one sector of the local view.
/// The visible area is carved into a three by three grid of sectors. This routine is the
/// counterpart of Cell_To_Local_Sector_Index and turns a sector back into a cell.
/// </summary>
/// <param name="index">The sector number, 1 through 9.</param>
/// <returns>Returns with the cell that represents the sector.</returns>
Cell MapClass::Local_Sector_Index_To_Cell(int index)
{
	int w1 = (2 * LocalRect.Width - 1) % 3;
	int w2 = (2 * LocalRect.Width - 1) / 3;

	int h1 = (2 * LocalRect.Height - 1) % 3;
	int h2 = (2 * LocalRect.Height - 1) / 3;

	int x = (index - 1) % 3;
	int y = (index - 1) / 3;

	int x1 = LocalRect.X + LocalRect.Y + 1;
	int y1 = PlayRect.Width - LocalRect.X + LocalRect.Y;

	Cell pos = Cell(x1, y1);

	int sector_x1 = (w1 ? 1 : 0) + (x * w2);
	int sector_y1 = (h1 ? 1 : 0) + (y * h2);

	int sector_x2 = sector_x1 + ((sector_x1 & 1) ? 1 : 0);
	int sector_y2 = sector_y1 + ((sector_y1 & 1) ? 1 : 0);

	int x2 = (sector_y2 + sector_x2) / 2;
	int y2 = (sector_y1 - sector_x1) / 2;

	return(Cell(x2,y2) + Cell(x1, y1));
}


/// <summary>
/// Tests whether an iso-tile type belongs to the shore-pieces tile set.
/// </summary>
/// <param name="tile">Iso-tile type to test.</param>
/// <returns>True if the tile is a shore piece; false otherwise.</returns>
bool MapClass::Is_Tile_Shore(IsometricTileType tile)
{
	if (IS_TILE_IN_SET(ShorePieces, SHORE_PIECES_COUNT)) return(true);

	return(false);
}


/// <summary>
/// Determines if an isometric tile is a cliff face.
/// Cliffs, cliff ramps and waterfalls all count, since none of them is ground a unit can
/// cross. A waterfall is the awkward case -- at either end of one the tile spills out onto
/// ordinary terrain, and those subtiles are not cliff at all.
/// </summary>
/// <param name="subtile">The subtile being examined; it spares the flat ends of a
/// waterfall.</param>
/// <returns>bool; Is the tile a cliff?</returns>
bool MapClass::Is_Tile_Cliff(IsometricTileType tile, int subtile)
{
	if (IS_TILE_IN_SET(CliffSet, CLIFF_COUNT)) return(true);

	if (IS_TILE_IN_SET(WaterfallEast, WATERFALL_EAST_COUNT)) {
		if (tile == IsometricTileTypeClass::WaterfallEast || tile == IsometricTileTypeClass::WaterfallEast + WATERFALL_EAST_COUNT-1) {
			if (subtile == 0 || subtile == 4) {
				return(false);
			}
		}
		return(true);
	}

	if (IS_TILE_IN_SET(WaterfallWest, WATERFALL_WEST_COUNT)) {
		if (tile == IsometricTileTypeClass::WaterfallWest || tile == IsometricTileTypeClass::WaterfallWest + WATERFALL_WEST_COUNT-1) {
			if (subtile == 1 || subtile == 3) {
				return(false);
			}
		}
		return(true);
	}

	if (IS_TILE_IN_SET(WaterfallSouth, WATERFALL_SOUTH_COUNT)) {
		if (tile == IsometricTileTypeClass::WaterfallSouth || tile == IsometricTileTypeClass::WaterfallSouth + WATERFALL_SOUTH_COUNT-1) {
			if (subtile == 0 || subtile == 1) {
				return(false);
			}
		}
		return(true);
	}

	if (IS_TILE_IN_SET(WaterfallNorth, WATERFALL_NORTH_COUNT)) {
		if (tile == IsometricTileTypeClass::WaterfallNorth || tile == IsometricTileTypeClass::WaterfallNorth + WATERFALL_NORTH_COUNT-1) {
			if (subtile == 2 || subtile == 3) {
				return(false);
			}
		}
		return(true);
	}

	if (IS_TILE_IN_SET(CliffRamps, CLIFF_RAMPS_COUNT)) return(true);

	return(false);
}


/// <summary>
/// Squares off the map's high ground and fences it with cliffs.
/// This routine is used by the random map generator once the terrain heights have settled.
/// Every cell is offered to the high ground fill, and the bare ground left along the borders
/// is then dressed with cliff tiles.
/// </summary>
/// <param name="region_id">The region seed to stamp onto the cells, or -1 to leave the
/// regions unconstrained.</param>
/// <returns>bool; Did every pass finish without a region seed conflict?</returns>
bool MapClass::Build_All_Cliffs(int, int region_id)
{
	bool expanded = true;

	bool is_temp_data = false;
	if (RMGCellData == NULL) {
		RMGCellData = new MapRegionClass::CellData[MapCellStride * MapCellStride];
		is_temp_data = true;
	}

	int r = MapCellStride;
	int s = r * r;
	for (int i = 0; i < s; i++) {
		MapRegionClass::Get_Cell_Data(i).InFillReach = true;
	}

	Map.Set_Cursor_Shape(NULL);

	Map.Reset_Iterator();
	CellClass * cptr = Map.Iterate();
	while (cptr != NULL && expanded) {
		expanded = Expand_High_Ground(cptr, region_id);
		cptr = Map.Iterate();
	}

	Map.Reset_Iterator();
	cptr = Map.Iterate();
	while (cptr != NULL && expanded) {
		if (cptr->Ramp == 0 && cptr->Is_Tile_Clear()) {
			expanded = Place_Cliff(cptr, region_id);
		}
		cptr = Map.Iterate();
	}

	ObjectClass * obj = Map.PendingObjectPtr;
	Map.PendingObject = NULL;
	if (obj) {
		delete obj;
		Map.PendingObjectPtr = NULL;
	}

	if (is_temp_data) {
		delete [] RMGCellData;
		RMGCellData = NULL;
	}

	return(expanded);
}


/// <summary>
/// Raises a cell and its neighbors to join the high ground.
/// The generator grows its plateaus with this routine. A cell is taken when the high ground
/// around it says it ought to be high too, which squares off ragged edges and fills the notches
/// no cliff piece could tile, and the fill then spreads to all eight neighbors.
/// </summary>
/// <param name="cellptr">The cell to consider.</param>
/// <param name="region_id">The region the raised cells are marked as belonging to. Pass -1 to
/// fill without regard for what other regions have claimed.</param>
/// <returns>bool; Was the cell free to join this region? A cell already claimed by another
/// region halts the fill.</returns>
bool MapClass::Expand_High_Ground(CellClass * cellptr, int region_id)
{
	bool expand = false;
	int mask = Get_High_Ground_Mask(cellptr);

	if ((mask & (MG_FACINGF_N | MG_FACINGF_W)) == (MG_FACINGF_N | MG_FACINGF_W) &&
		(mask & (MG_FACINGF_SW | MG_FACINGF_NE)) == (MG_FACINGF_SW | MG_FACINGF_NE)) {
		expand = true;
	}

	if ((mask & MG_FACINGF_W) != 0) {
		if ((Get_High_Ground_Mask(&Map[cellptr->Fetch_CellID() + Cell(1, 0)]) & MG_FACINGF_E) != 0) {
			expand = true;
		}
	}

	if ((mask & MG_FACINGF_E) != 0) {
		if ((Get_High_Ground_Mask(&Map[cellptr->Fetch_CellID() + Cell(-1, 0)]) & MG_FACINGF_W) != 0) {
			expand = true;
		}
	}

	if ((mask & MG_FACINGF_S) != 0) {
		if ((Get_High_Ground_Mask(&Map[cellptr->Fetch_CellID() + Cell(0, -1)]) & MG_FACINGF_N) != 0) {
			expand = true;
		}
	}

	if ((mask & MG_FACINGF_N) != 0) {
		if ((Get_High_Ground_Mask(&Map[cellptr->Fetch_CellID() + Cell(0, 1)]) & MG_FACINGF_S) != 0) {
			expand = true;
		}
	}

	if ((mask & (MG_FACINGF_SW | MG_FACINGF_NE)) == (MG_FACINGF_SW | MG_FACINGF_NE) &&
		((mask & (MG_FACINGF_S | MG_FACINGF_E)) != (MG_FACINGF_S | MG_FACINGF_E) ||
		 (mask & (MG_FACINGF_N | MG_FACINGF_NW | MG_FACINGF_W)) != 0) &&
		((mask & (MG_FACINGF_N | MG_FACINGF_W)) != (MG_FACINGF_N | MG_FACINGF_W) ||
		 (mask & (MG_FACINGF_S | MG_FACINGF_SE | MG_FACINGF_E)) != 0)) {
		expand = true;
	}

	if ((mask & (MG_FACINGF_NW | MG_FACINGF_SE)) == (MG_FACINGF_NW | MG_FACINGF_SE) &&
		((mask & (MG_FACINGF_W | MG_FACINGF_S)) != (MG_FACINGF_W | MG_FACINGF_S) ||
		 (mask & (MG_FACINGF_N | MG_FACINGF_E | MG_FACINGF_NE)) != 0) &&
		((mask & (MG_FACINGF_N | MG_FACINGF_E)) != (MG_FACINGF_N | MG_FACINGF_E) ||
		 (mask & (MG_FACINGF_W | MG_FACINGF_SW | MG_FACINGF_S)) != 0)) {
		expand = true;
	}

	if ((mask & (MG_FACINGF_W | MG_FACINGF_S | MG_FACINGF_SE)) == (MG_FACINGF_W | MG_FACINGF_SE) ||
		(mask & (MG_FACINGF_N | MG_FACINGF_W | MG_FACINGF_NE)) == (MG_FACINGF_W | MG_FACINGF_NE) ||
		(mask & (MG_FACINGF_SW | MG_FACINGF_S | MG_FACINGF_E)) == (MG_FACINGF_SW | MG_FACINGF_E) ||
		(mask & (MG_FACINGF_N | MG_FACINGF_NW | MG_FACINGF_E)) == (MG_FACINGF_NW | MG_FACINGF_E) ||
		(mask & (MG_FACINGF_S | MG_FACINGF_E | MG_FACINGF_NE)) == (MG_FACINGF_S | MG_FACINGF_NE) ||
		(mask & (MG_FACINGF_NW | MG_FACINGF_W | MG_FACINGF_S)) == (MG_FACINGF_NW | MG_FACINGF_S) ||
		(mask & (MG_FACINGF_N | MG_FACINGF_SE | MG_FACINGF_E)) == (MG_FACINGF_N | MG_FACINGF_SE) ||
		(mask & (MG_FACINGF_N | MG_FACINGF_W | MG_FACINGF_SW)) == (MG_FACINGF_N | MG_FACINGF_SW)) {
		expand = true;
	}

	if ((mask & (MG_FACINGF_N | MG_FACINGF_S)) == (MG_FACINGF_N | MG_FACINGF_S) ||
		(mask & (MG_FACINGF_W | MG_FACINGF_E)) == (MG_FACINGF_W | MG_FACINGF_E)) {
		expand = true;
	}

	if (expand) {
		int cell_region_id = RandomMapGen.Get_Cell_Data_Region(cellptr->Fetch_CellID());
		if (cell_region_id > 0 && cell_region_id != region_id && region_id != -1) {
			return(false);
		}
		cellptr->Height += 4;
		RandomMapGen.Set_Cell_Data_Region(cellptr->Fetch_CellID(), region_id);
		for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
			Expand_High_Ground(&cellptr->Adjacent_Cell(dir), region_id);
		}
	}
	return(true);
}


/*
 * This is the offset from the cell being walled at which each cliff piece is placed, indexed
 * by the variant the cliff pass picked.
 */
const Cell CliffVariantOffsets[] = {
	Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0),
	Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0),
	Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0),
	Cell(0, 0), Cell(1, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0), Cell(0, 0),
};


/// <summary>
/// Places the cliff tile that suits one cell below high ground.
/// This is the cliff pass of the generator's height regions. The high ground around the cell
/// picks the cliff piece, with several interchangeable pieces for each shape so that a long
/// escarpment does not come out visibly tiled. The mutated Firestorm biome will occasionally
/// get a crystal cliff instead.
/// </summary>
/// <param name="cellptr">The cell to consider.</param>
/// <param name="region_id">The height region being walled.</param>
/// <returns>bool; Did the cliff piece go down without a clash?</returns>
bool MapClass::Place_Cliff(CellClass * cellptr, int region_id)
{
	int random6 = Pick_Random_UInt(0, 5);
	int mask = Get_High_Ground_Mask(cellptr);

	bool crystal = Addon_Enabled(ADDON_FIRESTORM) && RandomMapGen.SeedData.Biome == BIOME_MUTATED && !Debug_Map;

	int variant = -1;
	int south;
	int east;
	int southeast;

	/*
	 * Two adjacent neighbors above the cell call for an inside corner piece.
	 */
	if ((mask & (MG_FACINGF_N | MG_FACINGF_W)) == (MG_FACINGF_N | MG_FACINGF_W)) {

		south = Get_High_Ground_Mask(&Map[cellptr->Fetch_CellID() + Cell(0, 1)]);
		east = Get_High_Ground_Mask(&Map[cellptr->Fetch_CellID() + Cell(1, 0)]);

		if ((south & MG_FACINGF_W) != 0 || (east & MG_FACINGF_N) != 0) {
			variant = 34;
		} else {
			variant = random6 % 3 + 9;
			if (crystal) {
				if (Random_Fraction() < 0.05) {
					variant = IsometricTileTypeClass::CrystalCliff - IsometricTileTypeClass::CliffSet + 4;
				}
			}
		}

	} else if ((mask & (MG_FACINGF_N | MG_FACINGF_E)) == (MG_FACINGF_N | MG_FACINGF_E)) {
		variant = 39;

	} else if ((mask & (MG_FACINGF_E | MG_FACINGF_S)) == (MG_FACINGF_E | MG_FACINGF_S)) {
		variant = 33;

	} else if ((mask & (MG_FACINGF_S | MG_FACINGF_W)) == (MG_FACINGF_S | MG_FACINGF_W)) {
		variant = 40;
	}

	/*
	 * A single neighbor above calls for a straight escarpment piece.
	 */
	if (variant == -1) {

		if ((mask & MG_FACINGF_E) != 0) {

			south = Get_High_Ground_Mask(&Map[cellptr->Fetch_CellID() + Cell(0, 1)]);
			southeast = Get_High_Ground_Mask(&Map[cellptr->Fetch_CellID() + Cell(1, 1)]);

			if ((mask & MG_FACINGF_SE) != 0 && (mask & (MG_FACINGF_SW | MG_FACINGF_S)) == 0 && (south & (MG_FACINGF_E | MG_FACINGF_S)) != (MG_FACINGF_E | MG_FACINGF_S)) {
				variant = random6 % 3 + 35;
			} else if ((mask & (MG_FACINGF_SE | MG_FACINGF_S)) != 0 || (southeast & MG_FACINGF_E) != 0 || (random6 & 1) == 0) {
				variant = 38;
			} else {
				variant = 1;
			}

		} else if ((mask & MG_FACINGF_W) != 0) {

			south = Get_High_Ground_Mask(&Map[cellptr->Fetch_CellID() + Cell(0, 1)]);

			if ((mask & MG_FACINGF_SW) == 0 || (mask & (MG_FACINGF_SE | MG_FACINGF_S)) != 0 || (south & (MG_FACINGF_S | MG_FACINGF_W)) == (MG_FACINGF_S | MG_FACINGF_W)) {
				variant = 18;
			} else {
				variant = random6 % 3 + 15;
			}

		} else if ((mask & MG_FACINGF_S) != 0) {

			east = Get_High_Ground_Mask(&Map[cellptr->Fetch_CellID() + Cell(1, 0)]);
			southeast = Get_High_Ground_Mask(&Map[cellptr->Fetch_CellID() + Cell(1, 1)]);

			if ((mask & MG_FACINGF_SE) == 0 || (mask & (MG_FACINGF_NE | MG_FACINGF_E)) != 0 || (east & (MG_FACINGF_E | MG_FACINGF_S)) == (MG_FACINGF_E | MG_FACINGF_S)) {
				if ((mask & (MG_FACINGF_E | MG_FACINGF_SE)) != 0 || (southeast & MG_FACINGF_S) != 0 || (random6 & 1) == 0) {
					variant = 26;
				} else {
					variant = 21;
				}
			} else {
				variant = random6 % 3 + 23;
			}

		} else if ((mask & MG_FACINGF_N) != 0) {

			east = Get_High_Ground_Mask(&Map[cellptr->Fetch_CellID() + Cell(1, 0)]);

			if ((mask & MG_FACINGF_NE) == 0 || (mask & (MG_FACINGF_E | MG_FACINGF_SE)) != 0 || (east & (MG_FACINGF_N | MG_FACINGF_E)) == (MG_FACINGF_N | MG_FACINGF_E)) {
				variant = 8;
			} else {
				variant = random6 % 3 + 5;
			}
		}
	}

	/*
	 * Nothing but a lone diagonal neighbor above, so an outside corner piece it is.
	 */
	if (variant == -1) {

		if ((mask & MG_FACINGF_NE) != 0) {
			variant = 2;

		} else if ((mask & MG_FACINGF_SE) != 0) {
			variant = (random6 & 1) + 29;

		} else if ((mask & MG_FACINGF_SW) != 0) {
			variant = 22;

		} else if ((mask & MG_FACINGF_NW) != 0) {
			variant = random6 % 3 + 12;
			if (crystal) {
				if (Random_Fraction() < 0.05) {
					variant = IsometricTileTypeClass::CrystalCliff - IsometricTileTypeClass::CliffSet + 3;
				}
			}

		} else {
			return(true);
		}
	}

	if (variant <= 0) {
		return(true);
	}

	Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::CliffSet + variant - 1];
	const Cell & varcell =  CliffVariantOffsets[variant - 1];

	Map.Set_Cursor_Pos(varcell + cellptr->Fetch_CellID());

	crystal = true;

	bool picked = Pick_Random_Tile_Variant(ISOTILE_CLEAR, 0, cellptr->Height, region_id, crystal);

	bool success = picked || crystal;

	return(success);
}


/// <summary>
/// Fetches the pattern of high ground around a cell.
/// The generator's height pass runs off this pattern -- which of the eight neighbors stand one
/// step higher decides whether the cell is raised to join them and which cliff piece is set
/// against it.
/// </summary>
/// <param name="cellptr">The cell to examine.</param>
/// <returns>Returns with a MG_FACINGF_* bitmask of the neighbors one step above, or zero if the
/// cell lies outside the area being filled.</returns>
int MapClass::Get_High_Ground_Mask(CellClass * cellptr)
{
	int height = cellptr->Height;
	Cell origin = cellptr->Fetch_CellID();
	int mask = 0;

	if (!Map.In_Radar(origin) || !MapRegionClass::Get_Cell_Data(origin).InFillReach) {
		return(0);
	}

	CellClass * cptr_ne = &Map[origin + Cell(1, -1)];
	CellClass * cptr_e = &Map[origin + Cell(1, 0)];
	CellClass * cptr_se = &Map[origin + Cell(1, 1)];
	CellClass * cptr_s = &Map[origin + Cell(0, 1)];
	CellClass * cptr_sw = &Map[origin + Cell(-1, 1)];
	CellClass * cptr_w = &Map[origin + Cell(-1, 0)];
	CellClass * cptr_nw = &Map[origin + Cell(-1, -1)];
	CellClass * cptr_n = &Map[origin + Cell(0, -1)];

	if (Map.In_Radar(origin + Cell(-1, -1)) && cptr_nw->Height == height + 4 && !cptr_nw->Is_Tile_Cliff()) {
		mask |= MG_FACINGF_NW;
	}
	if (Map.In_Radar(origin + Cell(0, -1)) && cptr_n->Height == height + 4 && !cptr_n->Is_Tile_Cliff()) {
		mask |= MG_FACINGF_N;
	}
	if (Map.In_Radar(origin + Cell(1, -1)) && cptr_ne->Height == height + 4 && !cptr_ne->Is_Tile_Cliff()) {
		mask |= MG_FACINGF_NE;
	}
	if (Map.In_Radar(origin + Cell(-1, 0)) && cptr_w->Height == height + 4 && !cptr_w->Is_Tile_Cliff()) {
		mask |= MG_FACINGF_W;
	}
	if (Map.In_Radar(origin + Cell(1, 0)) && cptr_e->Height == height + 4 && !cptr_e->Is_Tile_Cliff()) {
		mask |= MG_FACINGF_E;
	}
	if (Map.In_Radar(origin + Cell(-1, 1)) && cptr_sw->Height == height + 4 && !cptr_sw->Is_Tile_Cliff()) {
		mask |= MG_FACINGF_SW;
	}
	if (Map.In_Radar(origin + Cell(0, 1)) && cptr_s->Height == height + 4 && !cptr_s->Is_Tile_Cliff()) {
		mask |= MG_FACINGF_S;
	}
	if (Map.In_Radar(origin + Cell(1, 1)) && cptr_se->Height == height + 4 && !cptr_se->Is_Tile_Cliff()) {
		mask |= MG_FACINGF_SE;
	}
	return(mask);
}


/// <summary>
/// Builds the map's bodies of water and dresses their coastlines.
/// This is the top level of the generator's water pass. Every cell is offered to the flood, the
/// badly joined water is pruned back out again, and the coastline is then dressed with shore
/// pieces from the land side and the water side in turn.
/// </summary>
/// <param name="region_id">The region the flooded cells are marked as belonging to.</param>
/// <returns>bool; Did every pass complete without running into another region?</returns>
bool MapClass::Build_All_Shores(int region_id)
{
	Map.Set_Cursor_Shape(NULL);

	bool is_temp_data = false;
	if (RMGCellData == NULL) {
		RMGCellData = new MapRegionClass::CellData[MapCellStride * MapCellStride];
		is_temp_data = true;
	}

	int r = MapCellStride;
	int s = r * r;
	for (int i = 0; i < s; i++) {
		MapRegionClass::Get_Cell_Data(i).WaterMask = -1;
	}

	Map.Reset_Iterator();
	CellClass * cptr = Map.Iterate();
	bool expanded = true;
	while (cptr != NULL && expanded) {
		expanded = Expand_Water(cptr, region_id);
		cptr = Map.Iterate();
	}

	Map.Reset_Iterator();
	cptr = Map.Iterate();
	while (cptr != NULL && expanded) {
		Prune_Water(cptr);
		cptr = Map.Iterate();
	}

	Map.Reset_Iterator();
	cptr = Map.Iterate();
	while (cptr != NULL && expanded) {
		expanded = Place_Shore(cptr, 1, region_id);
		cptr = Map.Iterate();
	}

	Map.Reset_Iterator();
	cptr = Map.Iterate();
	while (cptr != NULL && expanded) {
		expanded = Place_Shore(cptr, 2, region_id);
		cptr = Map.Iterate();
	}

	ObjectClass * obj = Map.PendingObjectPtr;
	Map.PendingObject = NULL;
	if (obj) {
		delete obj;
		Map.PendingObjectPtr = NULL;
	}

	if (is_temp_data) {
		delete [] RMGCellData;
		RMGCellData = NULL;
	}

	return(expanded);
}


/// <summary>
/// Clears a water cell that joins its neighbors badly.
/// Water that meets other water only at a corner leaves a pinch no ship could sail through and
/// no shore piece could tile, so the generator simply takes such cells back out again.
/// </summary>
/// <param name="cellptr">The water cell to test.</param>
void MapClass::Prune_Water(CellClass * cellptr)
{
	IsometricTileType itype = cellptr->ITType;
	if (itype >= IsometricTileTypeClass::WaterSet && itype < IsometricTileType(IsometricTileTypeClass::WaterSet + WATER_COUNT-2)) {
		int mask = Get_Water_Mask(cellptr, 2);
		if (mask == (MG_FACINGF_N | MG_FACINGF_NE | MG_FACINGF_E | MG_FACINGF_SE | MG_FACINGF_NW) ||
			mask == (MG_FACINGF_SE | MG_FACINGF_S | MG_FACINGF_SW | MG_FACINGF_W | MG_FACINGF_NW) ||
			mask == (MG_FACINGF_N | MG_FACINGF_NE | MG_FACINGF_SW | MG_FACINGF_W | MG_FACINGF_NW) ||
			mask == (MG_FACINGF_NE | MG_FACINGF_E | MG_FACINGF_SE | MG_FACINGF_S | MG_FACINGF_SW) ||
			mask == (MG_FACINGF_N | MG_FACINGF_E | MG_FACINGF_SE | MG_FACINGF_NW) ||
			mask == (MG_FACINGF_SE | MG_FACINGF_S | MG_FACINGF_W | MG_FACINGF_NW) ||
			mask == (MG_FACINGF_N | MG_FACINGF_NE | MG_FACINGF_SW | MG_FACINGF_W) ||
			mask == (MG_FACINGF_NE | MG_FACINGF_E | MG_FACINGF_S | MG_FACINGF_SW)) {

			cellptr->ITType = ISOTILE_NONE;
			cellptr->SubTile = 0;
			for (FacingType facing = FACING_N; facing < FACING_COUNT; facing++) {
				CellClass * adjptr = &cellptr->Adjacent_Cell(facing);
				if (My_In_Radar(adjptr->Fetch_CellID())) {
					MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(adjptr->Fetch_CellID());
					data.WaterMask = -1;
					data.WaterMask = Get_Water_Mask(adjptr, 2);
				}
			}
		}
	}
}


/// <summary>
/// Floods a cell and its neighbors into a water region.
/// The generator grows its lakes and rivers with this routine. A cell is taken when the water
/// around it says it ought to be water too, which drowns the pinches and narrow necks of land
/// that no shore piece could tile, and the fill then spreads to all eight neighbors.
/// </summary>
/// <param name="cellptr">The cell to consider.</param>
/// <param name="region_id">The region the flooded cells are marked as belonging to.</param>
/// <returns>bool; Was the cell free to join this region? A cell already claimed by another
/// region halts the fill.</returns>
bool MapClass::Expand_Water(CellClass * cellptr, int region_id)
{
	bool expand = false;

	MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(cellptr->Fetch_CellID());
	data.WaterMask = Get_Water_Mask(cellptr, 2);

	int mask = Get_Water_Mask(cellptr, 0);
	if (mask > 0) {

		if (mask == (MG_FACINGF_NE | MG_FACINGF_E | MG_FACINGF_S) ||
			mask == (MG_FACINGF_E | MG_FACINGF_S | MG_FACINGF_SW)) {
			expand = true;
		}

		if ((mask & (MG_FACINGF_W | MG_FACINGF_N)) == (MG_FACINGF_W | MG_FACINGF_N) && (mask & (MG_FACINGF_NE | MG_FACINGF_SW)) == 0 ||
			(mask & (MG_FACINGF_E | MG_FACINGF_N)) == (MG_FACINGF_E | MG_FACINGF_N) && (mask & (MG_FACINGF_SE | MG_FACINGF_NW)) == 0 ||
			(mask & (MG_FACINGF_E | MG_FACINGF_S)) == (MG_FACINGF_E | MG_FACINGF_S) && (mask & (MG_FACINGF_NE | MG_FACINGF_SW)) == 0 ||
			(mask & (MG_FACINGF_S | MG_FACINGF_W)) == (MG_FACINGF_S | MG_FACINGF_W) && (mask & (MG_FACINGF_SE | MG_FACINGF_NW)) == 0) {
			expand = true;
		}

		if ((mask & MG_FACINGF_W) != 0) {
			int mask1 = Get_Water_Mask(&Map[cellptr->Fetch_CellID() + Cell(1, 0)], 0);
			int mask2 = Get_Water_Mask(&Map[cellptr->Fetch_CellID() + Cell(2, 0)], 0);
			if ((mask1 & MG_FACINGF_E) != 0 || (mask2 & MG_FACINGF_E) != 0) {
				expand = true;
			}
		}

		if ((mask & MG_FACINGF_E) != 0) {
			int mask1 = Get_Water_Mask(&Map[cellptr->Fetch_CellID() + Cell(-1, 0)], 0);
			int mask2 = Get_Water_Mask(&Map[cellptr->Fetch_CellID() + Cell(-2, 0)], 0);
			if ((mask1 & MG_FACINGF_W) != 0 || (mask2 & MG_FACINGF_W) != 0) {
				expand = true;
			}
		}

		if ((mask & MG_FACINGF_S) != 0) {
			int mask1 = Get_Water_Mask(&Map[cellptr->Fetch_CellID() + Cell(0, -1)], 0);
			int mask2 = Get_Water_Mask(&Map[cellptr->Fetch_CellID() + Cell(0, -2)], 0);
			if ((mask1 & MG_FACINGF_N) != 0 || (mask2 & MG_FACINGF_N) != 0) {
				expand = true;
			}
		}

		if ((mask & MG_FACINGF_N) != 0) {
			int mask1 = Get_Water_Mask(&Map[cellptr->Fetch_CellID() + Cell(0, 1)], 0);
			int mask2 = Get_Water_Mask(&Map[cellptr->Fetch_CellID() + Cell(0, 2)], 0);
			if ((mask1 & MG_FACINGF_S) != 0 || (mask2 & MG_FACINGF_S) != 0) {
				expand = true;
			}
		}

		if ((mask & (MG_FACINGF_N | MG_FACINGF_S)) == (MG_FACINGF_N | MG_FACINGF_S) ||
			(mask & (MG_FACINGF_E | MG_FACINGF_W)) == (MG_FACINGF_E | MG_FACINGF_W) ||
			expand) {

			int cell_region_id = RandomMapGen.Get_Cell_Data_Region(cellptr->Fetch_CellID());
			if (cell_region_id > 0 && cell_region_id != region_id) {
				return(false);
			}
			cellptr->ITType = IsometricTileType(IsometricTileTypeClass::WaterSet);
			cellptr->SubTile = 0;
			RandomMapGen.Set_Cell_Data_Region(cellptr->Fetch_CellID(), region_id);
			for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
				CellClass * adjptr = &cellptr->Adjacent_Cell(dir);
				if (My_In_Radar(adjptr->Fetch_CellID())) {
					MapRegionClass::Get_Cell_Data(adjptr->Fetch_CellID()).WaterMask = -1;
					Expand_Water(adjptr, region_id);
				}
			}
		}
	}
	return(true);
}


/// <summary>
/// Counts how far a straight stretch of shore runs.
/// The shore placer uses the length to alternate between the interchangeable shore pieces, so
/// that a long coastline is not laid down as one tile repeated end to end.
/// </summary>
/// <param name="cellptr">The cell to start counting from.</param>
/// <param name="direction">Facing of the shore edge at that cell.</param>
/// <returns>Returns with the number of cells in the run, never less than one.</returns>
int MapClass::Straight_Shore_Length(CellClass * cellptr, FacingType direction)
{
	FacingType snap[4] = {
		FACING_E,
		FACING_S,
		FACING_E,
		FACING_S,
	};

	int count = 1;
	FacingType s_direction = snap[direction / 2];
	CellClass * adj1 = &cellptr->Adjacent_Cell(s_direction);
	CellClass * adj2 = &adj1->Adjacent_Cell(direction);
	bool side_1_is_land = adj1->Is_Tile_Water() == false;
	bool side_2_is_water = adj2->Is_Tile_Water();

	while (side_1_is_land && side_2_is_water) {
		count++;
		adj1 = &adj1->Adjacent_Cell(s_direction);
		adj2 = &adj2->Adjacent_Cell(s_direction);
		side_1_is_land = adj1->Is_Tile_Water() == false;
		side_2_is_water = adj2->Is_Tile_Water();
	}

	return(count);
}


/*
 * These are the shore pieces gathered into groups, one entry per variant of the ShorePieces
 * tile set. Several variants are only randomized forms of the one piece, so they share a
 * group; two shore tiles may be laid against each other only when their groups agree.
 */
int ShorePieceGroups[] = {
	0, 0, 0, 1, 2, 3, 4, 4, 5, 5,
	5, 6, 7, 8, 9, 9, 10, 10, 10, 11,
	12, 13, 14, 14, 15, 15, 15, 16, 17, 18,
	19, 19, 20, 20, 21, 21, 22, 22, 23, 23,
	24, 25
};


/*
 * This is the way each variant of the ShorePieces tile set faces, expressed as a FacingType
 * but held as an int so that the difference between two of them can be taken directly. Two
 * pieces whose facings are three to five steps apart point roughly away from one another, and
 * that is what marks a pairing as no good.
 */
int ShorePieceFacings[] = {
	4, 4, 4, 4, 4, 4, 3, 3, 2, 2,
	2, 2, 2, 2, 1, 1, 0, 0, 0, 0,
	0, 0, 7, 7, 6, 6, 6, 6, 6, 6,
	5, 5, 3, 3, 1, 1, 7, 7, 5, 5,
	4, 4
};


/*
 * This is where each variant of the ShorePieces tile set anchors, given as the offset from the
 * cell the shore is being grown from to the cell the placement cursor belongs at. A piece
 * covers more than the one cell, so it has to be hung off its own corner rather than the cell
 * that asked for it.
 */
Cell ShorePieceOffsets[] = {
	Cell(0, -1),
	Cell(0, -1),
	Cell(0, -1),
	Cell(0, -1),
	Cell(0, -2),
	Cell(-1, -2),
	Cell(-1, -1),
	Cell(-1, -1),
	Cell(-1, 0),
	Cell(-1, 0),
	Cell(-1, 0),
	Cell(-1, 0),
	Cell(-2, -1),
	Cell(-2, 0),
	Cell(-1, 0),
	Cell(-1, 0),
	Cell(0, 0),
	Cell(0, 0),
	Cell(0, 0),
	Cell(0, 0),
	Cell(0, 0),
	Cell(-1, 0),
	Cell(0, 0),
	Cell(0, 0),
	Cell(0, 0),
	Cell(0, 0),
	Cell(0, 0),
	Cell(0, 0),
	Cell(0, -1),
	Cell(0, 0),
	Cell(0, -1),
	Cell(0, -1),
	Cell(-1, -1),
	Cell(-1, -1),
	Cell(-1, 0),
	Cell(-1, 0),
	Cell(0, 0),
	Cell(0, 0),
	Cell(0, -1),
	Cell(0, -1),
	Cell(0, 0),
	Cell(0, 0)
};


/// <summary>
/// Places the shore tile that suits one cell at the water's edge.
/// This is the per-cell worker of the generator's shore pass. The water lying around the cell
/// picks the shore piece, and along a straight coast the length of the run decides which of the
/// interchangeable pieces is used, so the shoreline does not come out visibly tiled.
/// </summary>
/// <param name="cellptr">The cell to consider.</param>
/// <param name="pass">Which pass this is: 1 for the land side of the shore, 2 for the water
/// side.</param>
/// <param name="region_id">The water region being shored.</param>
/// <returns>bool; Did the shore piece go down without a clash?</returns>
bool MapClass::Place_Shore(CellClass * cellptr, int pass, int region_id)
{
	int random = Pick_Random_UInt(0, 5);

	int mask;
	if (pass == 2) {
		mask = Get_Water_Mask(cellptr, 1);
	} else {
		mask = Get_Water_Mask(cellptr, 0);
	}
	if (mask == 0) {
		return(true);
	}

	int index;
	int count;
	CellClass * adj1;
	CellClass * adj2;
	bool side_1_is_land;
	bool side_2_is_water;

	if (pass == 2) {

		if ((mask & (MG_FACINGF_N | MG_FACINGF_W)) == (MG_FACINGF_N | MG_FACINGF_W)) {
			if ((mask & (MG_FACINGF_NE | MG_FACINGF_SW)) == (MG_FACINGF_NE | MG_FACINGF_SW)) {
				index = (random & 1) + 23;
			} else {
				index = (mask & MG_FACINGF_NE) != 0 ? 21 : 30;
			}
		} else if ((mask & (MG_FACINGF_N | MG_FACINGF_E)) == (MG_FACINGF_N | MG_FACINGF_E)) {
			if ((mask & (MG_FACINGF_NW | MG_FACINGF_SE)) == (MG_FACINGF_NW | MG_FACINGF_SE)) {
				index = (random & 1) + 15;
			} else {
				index = (mask & MG_FACINGF_SE) != 0 ? 14 : 22;
			}
		} else if ((mask & (MG_FACINGF_S | MG_FACINGF_E)) == (MG_FACINGF_S | MG_FACINGF_E)) {
			if ((mask & (MG_FACINGF_NE | MG_FACINGF_SW)) == (MG_FACINGF_NE | MG_FACINGF_SW)) {
				index = (random & 1) + 7;
			} else {
				index = (mask & MG_FACINGF_NE) != 0 ? 13 : 6;
			}
		} else {
			if ((mask & (MG_FACINGF_W | MG_FACINGF_S)) != (MG_FACINGF_W | MG_FACINGF_S)) {
				return(true);
			}
			if ((mask & (MG_FACINGF_NW | MG_FACINGF_SE)) == (MG_FACINGF_NW | MG_FACINGF_SE)) {
				index = (random & 1) + 31;
			} else {
				index = (mask & MG_FACINGF_SE) != 0 ? 5 : 29;
			}
		}

	} else if ((mask & MG_FACINGF_E) != 0) {
		count = 1;
		adj1 = &cellptr->Adjacent_Cell(FACING_S);
		adj2 = &adj1->Adjacent_Cell(FACING_E);
		side_1_is_land = adj1->Is_Tile_Water() == false;
		side_2_is_water = adj2->Is_Tile_Water();
		while (side_1_is_land && side_2_is_water) {
			count++;
			adj1 = &adj1->Adjacent_Cell(FACING_S);
			adj2 = &adj2->Adjacent_Cell(FACING_S);
			side_1_is_land = adj1->Is_Tile_Water() == false;
			side_2_is_water = adj2->Is_Tile_Water();
		}
		if ((count & 1) != 0 && (mask & MG_FACINGF_N) == 0 || (mask & MG_FACINGF_SE) == 0 || (mask & (MG_FACINGF_S | MG_FACINGF_SW)) != 0) {
			index = 12;
		} else {
			index = abs(random % 3) + 9;
		}
	} else if ((mask & MG_FACINGF_W) != 0) {
		count = 1;
		adj1 = &cellptr->Adjacent_Cell(FACING_S);
		adj2 = &adj1->Adjacent_Cell(FACING_W);
		side_1_is_land = adj1->Is_Tile_Water() == false;
		side_2_is_water = adj2->Is_Tile_Water();
		while (side_1_is_land && side_2_is_water) {
			count++;
			adj1 = &adj1->Adjacent_Cell(FACING_S);
			adj2 = &adj2->Adjacent_Cell(FACING_S);
			side_1_is_land = adj1->Is_Tile_Water() == false;
			side_2_is_water = adj2->Is_Tile_Water();
		}
		if ((count & 1) != 0 && (mask & MG_FACINGF_N) == 0 || (mask & MG_FACINGF_SW) == 0 || (mask & (MG_FACINGF_SE | MG_FACINGF_S)) != 0) {
			index = 28;
		} else {
			index = random % 3 + 25;
		}
	} else if ((mask & MG_FACINGF_S) != 0) {
		count = 1;
		adj1 = &cellptr->Adjacent_Cell(FACING_E);
		adj2 = &adj1->Adjacent_Cell(FACING_S);
		side_1_is_land = adj1->Is_Tile_Water() == false;
		side_2_is_water = adj2->Is_Tile_Water();
		while (side_1_is_land && side_2_is_water) {
			count++;
			adj1 = &adj1->Adjacent_Cell(FACING_E);
			adj2 = &adj2->Adjacent_Cell(FACING_E);
			side_1_is_land = adj1->Is_Tile_Water() == false;
			side_2_is_water = adj2->Is_Tile_Water();
		}
		if ((count & 1) != 0 || (mask & MG_FACINGF_SE) == 0 || (mask & (MG_FACINGF_NE | MG_FACINGF_E)) != 0) {
			index = 4;
		} else {
			index = random % 3 + 1;
		}
	} else if ((mask & MG_FACINGF_N) != 0) {
		count = 1;
		adj1 = &cellptr->Adjacent_Cell(FACING_E);
		adj2 = &adj1->Adjacent_Cell(FACING_N);
		side_1_is_land = adj1->Is_Tile_Water() == false;
		side_2_is_water = adj2->Is_Tile_Water();
		while (side_1_is_land && side_2_is_water) {
			count++;
			adj1 = &adj1->Adjacent_Cell(FACING_E);
			adj2 = &adj2->Adjacent_Cell(FACING_E);
			side_1_is_land = adj1->Is_Tile_Water() == false;
			side_2_is_water = adj2->Is_Tile_Water();
		}
		if ((count & 1) != 0 || (mask & MG_FACINGF_NE) == 0 || (mask & (MG_FACINGF_E | MG_FACINGF_SE)) != 0) {
			index = 20;
		} else {
			index = random % 3 + 17;
		}
	} else {

		if ((mask & MG_FACINGF_NE) != 0) {
			index = (random & 1) + 35;
		} else if ((mask & MG_FACINGF_SE) != 0) {
			index = (random & 1) + 33;
		} else if ((mask & MG_FACINGF_SW) != 0) {
			index = (random & 1) + 39;
		} else {
			if ((mask & MG_FACINGF_NW) == 0) {
				return(true);
			}
			index = (random & 1) + 37;
		}
	}

	if (index > 0) {

		Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::ShorePieces + index - 1];

		Cell cellid = cellptr->Fetch_CellID();
		Cell cursor = Map.Set_Cursor_Pos(cellid + ShorePieceOffsets[index - 1]);

		bool result = true;
		if (pass == 1) {
			Pick_Random_Tile_Variant(ISOTILE_CLEAR, ISOTILE_CLEAR, cellptr->Height, region_id, result);
		} else {
			if (pass != 2) {
				return(true);
			}
			Pick_Random_Tile_Variant(IsometricTileTypeClass::ShorePieces, IsometricTileTypeClass::ShorePieces + SHORE_PIECES_COUNT - 1, cellptr->Height, region_id, result);
		}

		return(result != false);
	}
	return(true);
}


/// <summary>
/// Fetches the pattern of water lying around a cell.
/// The generator's water pass runs almost entirely off this pattern -- which of the eight
/// neighbors are already water decides whether a cell is flooded, pruned back out, or given a
/// shore piece.
/// </summary>
/// <param name="cellptr">The cell to examine.</param>
/// <param name="control">Which cells qualify for an answer: 0 for clear cells only, 1 for
/// anything that is not already water, higher for any cell at all.</param>
/// <returns>Returns with a MG_FACINGF_* bitmask of the water neighbors, or zero if the cell
/// does not qualify.</returns>
/// <remarks>The answer is cached per cell, so a caller that changes the terrain must clear the
/// cached mask of every cell affected.</remarks>
int MapClass::Get_Water_Mask(CellClass * cellptr, int control)
{
	Cell cell = cellptr->Fetch_CellID();
	int x = cell.X;
	int y = cell.Y;

	IsometricTileType water_start = IsometricTileType(IsometricTileTypeClass::WaterSet);
	IsometricTileType water_end = IsometricTileType(water_start + WATER_COUNT);

	int mask = 0;

	if (!My_In_Radar(cell)) {
		return(mask);
	}

	if (control == 0) {
		if (!cellptr->CellClass::Is_Tile_Clear()) {
			return(mask);
		}
	} else if (control == 1) {
		if (cellptr->ITType >= water_start && cellptr->ITType < water_end) {
			return(mask);
		}
	}

	cell = cellptr->Fetch_CellID();
	MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(cell);
	if (!data.InFillReach) {
		return(mask);
	}
	if (data.WaterMask >= 0) {
		return(data.WaterMask);
	}

	CellClass ** cptr = &Array[x + y * MAP_CELL_H - MAP_CELL_W - 1];

	CellClass * tptr = cptr[0];
	if (tptr && tptr->Is_Tile_Water()) {
		mask |= MG_FACINGF_NW;
	}
	cptr += 1;
	tptr = cptr[0];
	if (tptr && tptr->Is_Tile_Water()) {
		mask |= MG_FACINGF_N;
	}
	cptr += 1;
	tptr = cptr[0];
	if (tptr && tptr->Is_Tile_Water()) {
		mask |= MG_FACINGF_NE;
	}
	cptr += MAP_CELL_W - 2;
	tptr = cptr[0];
	if (tptr && tptr->Is_Tile_Water()) {
		mask |= MG_FACINGF_W;
	}
	cptr += 2;
	tptr = cptr[0];
	if (tptr && tptr->Is_Tile_Water()) {
		mask |= MG_FACINGF_E;
	}
	cptr += MAP_CELL_W - 2;
	tptr = cptr[0];
	if (tptr && tptr->Is_Tile_Water()) {
		mask |= MG_FACINGF_SW;
	}
	cptr += 1;
	tptr = cptr[0];
	if (tptr && tptr->Is_Tile_Water()) {
		mask |= MG_FACINGF_S;
	}
	cptr += 1;
	tptr = cptr[0];
	if (tptr && tptr->Is_Tile_Water()) {
		mask |= MG_FACINGF_SE;
	}
	return(mask);
}


/// <summary>
/// Stamps the pending iso-tile onto the map for the map generator.
/// The tile is laid over the cells it covers, but only where it will not spoil what the
/// generator has already built. Terrain belonging to another region is left standing, and a
/// shore piece that would face against the shore it meets, or that would land on a cliff, is
/// refused outright so the caller can try something else.
/// </summary>
/// <param name="tile">Lowest iso-tile type that may be overwritten.</param>
/// <param name="max_tile">Highest iso-tile type that may be overwritten.</param>
/// <param name="base_height">Height the placed tile is raised by.</param>
/// <param name="seed">The region the tile belongs to, or -1 for none.</param>
/// <param name="success">Cleared when the tile clashes with a shore or cliff already on the
/// ground. Left alone otherwise.</param>
/// <returns>bool; Did the placement go through without a clash?</returns>
bool MapClass::Pick_Random_Tile_Variant(IsometricTileType tile, int max_tile, int base_height, int seed, bool & success)
{
	if (Map.PendingObject->RTTI == RTTI_ISOTILETYPE) {

		IsometricTileTypeClass * ttype = (IsometricTileTypeClass *)Map.PendingObject;
		IsoTileSet const * image = (IsoTileSet const *)ttype->Get_Image_Data();
		if (image != NULL) {

			int row = 0;
			if (ttype->Height > 0) {

				do {
					for (int col = 0; col < ttype->Width; col++) {

						Cell cell = Map.ZoneCell + Cell(col, row);

						/*
						 * Skip subtiles that fall outside the visible diamond playfield.
						 */
						if (cell.X + cell.Y <= Map.PlayRect.Width
							|| cell.X - cell.Y >= Map.PlayRect.Width
							|| cell.Y - cell.X >= Map.PlayRect.Width
							|| cell.X + cell.Y > Map.PlayRect.Width + 2 * Map.PlayRect.Height) {
							continue;
						}

						CellClass * cellptr = &Map[cell];

						int subtile = col + ttype->Width * row;
						IsoTileRecord const * record = image->Fetch_Record_Pointer_Unsafe(subtile);
						if (record == NULL) {
							continue;
						}

						int id = RandomMapGen.Get_Cell_Data_Region(cell);
						if (id > 0 && id != seed && seed != -1) {

							int existing = cellptr->ITType - IsometricTileTypeClass::ShorePieces;
							int pending = ttype->HeapID - IsometricTileTypeClass::ShorePieces;

							if (!cellptr->Is_Tile_Clear()) {
								if (existing < 0 || pending < 0 || existing > 41 || pending > 41
									|| ShorePieceGroups[existing] != ShorePieceGroups[pending]) {
									success = false;
									return(false);
								}
								return(true);
							}

							RandomMapGen.Set_Cell_Data_Region(cell, seed);

						} else if (id == seed) {

							int existing = cellptr->ITType - IsometricTileTypeClass::ShorePieces;
							int pending = ttype->HeapID - IsometricTileTypeClass::ShorePieces;

							if (existing >= 0 && pending >= 0 && existing <= 41 && pending <= 41) {
								int diff = abs(ShorePieceFacings[existing] - ShorePieceFacings[pending]);
								if (diff >= 3 && diff <= 5) {
									success = false;
									return(false);
								}
							}

						} else if (cellptr->Is_Tile_Clear()) {
							RandomMapGen.Set_Cell_Data_Region(cell, seed);

						} else {
							if (seed != -1) {
								success = false;
								return(false);
							}
							continue;
						}

						if (cellptr->Is_Tile_Clear()
							|| (cellptr->ITType >= tile && cellptr->ITType <= max_tile)) {

							cellptr->ITType = ttype->HeapID;
							cellptr->SubTile = subtile;
							cellptr->Height = base_height + record->Height;
							continue;
						}

						/*
						 * The existing tile is outside the replaceable range. Only a shore-vs-cliff
						 * conflict blocks placement; anything else is silently skipped.
						 */
						if (cellptr->Is_Tile_Shore() && MapClass::Is_Tile_Cliff(ttype->HeapID, subtile)
							|| (ttype->HeapID >= IsometricTileTypeClass::ShorePieces
								&& ttype->HeapID < IsometricTileTypeClass::ShorePieces + SHORE_PIECES_COUNT
								&& cellptr->Is_Tile_Cliff())) {
							success = false;
						}
						return(false);
					}

					row++;
				} while (row < ttype->Height);

				return(true);
			}

			return(true);
		}
	}

	return(false);
}


/// <summary>
/// Reduces tiberium within a radius-5 disc around the given cell.
/// For each offset within distance 5, reduces the tiberium amount by 1.
/// </summary>
/// <param name="cell">Center cell of the tiberium-reduction area.</param>
void MapClass::Area_Reduce_Tiberium(Cell const & cell)
{
	for (int x = -5; x <= 5; x++) {
		for (int y = -5; y <= 5; y++) {
			Cell c1 = Cell(cell + Cell(x,y));
			Cell c2 = (cell - c1);
			if (c2.Length() <= 5) {
				CellClass * cellptr = &Map[cell];
				if (cellptr != NULL) {
					cellptr->Reduce_Tiberium(1);
				}
			}
		}
	}
}


/// <summary>
/// Fetches the neighbor pattern of an east-west low bridge cell.
/// The damage code uses the pattern to decide which damaged overlay the cell should wear, so
/// that a broken span still looks like it joins up with whatever is left of it.
/// </summary>
/// <param name="cell">The low bridge cell whose neighbors are examined.</param>
/// <returns>Returns with a four bit code describing the bridge pieces to the east and
/// west.</returns>
int MapClass::Get_Low_Bridge_EW_Neighbor_Mask(Cell const & cell)
{
	OverlayType o1 = Map[cell + Cell(-1, 0)].Overlay;
	OverlayType o2 = Map[cell + Cell(1, 0)].Overlay;

	int index = 0;
	switch (o2) {
		case OVERLAY_LOWBRIDGE_05:
		case OVERLAY_LOWBRIDGE_07:
		case OVERLAY_LOWBRIDGE_09:
		case OVERLAY_LOWBRIDGE_20:
			index |= 1;
			break;
		case OVERLAY_LOWBRIDGE_08:
		case OVERLAY_LOWBRIDGE_27:
			index |= 2;
			break;
		default:
			break;
	}

	switch (o1) {
		case OVERLAY_LOWBRIDGE_06:
		case OVERLAY_LOWBRIDGE_07:
		case OVERLAY_LOWBRIDGE_08:
		case OVERLAY_LOWBRIDGE_22:
			index |= 4;
			break;
		case OVERLAY_LOWBRIDGE_09:
		case OVERLAY_LOWBRIDGE_27:
			index |= 8;
			break;
		default:
			break;
	}

	return(index);
}


/// <summary>
/// Fetches the neighbor pattern of a north-south low bridge cell.
/// North-south counterpart of Get_Low_Bridge_EW_Neighbor_Mask. The damage code turns the
/// pattern into the overlay a damaged piece should wear.
/// </summary>
/// <param name="cell">The low bridge cell whose neighbors are examined.</param>
/// <returns>Returns with a four bit code describing the bridge pieces to the north and
/// south.</returns>
int MapClass::Get_Low_Bridge_NS_Neighbor_Mask(Cell const & cell)
{
	OverlayType o1 = Map[cell + Cell(0, -1)].Overlay;
	OverlayType o2 = Map[cell + Cell(0, 1)].Overlay;

	int index = 0;
	switch (o1) {
		case OVERLAY_LOWBRIDGE_14:
		case OVERLAY_LOWBRIDGE_16:
		case OVERLAY_LOWBRIDGE_18:
		case OVERLAY_LOWBRIDGE_24:
			index |= 1;
			break;
		case OVERLAY_LOWBRIDGE_17:
		case OVERLAY_LOWBRIDGE_28:
			index |= 2;
			break;
		default:
			break;
	}

	switch (o2) {
		case OVERLAY_LOWBRIDGE_15:
		case OVERLAY_LOWBRIDGE_16:
		case OVERLAY_LOWBRIDGE_17:
		case OVERLAY_LOWBRIDGE_26:
			index |= 4;
			break;
		case OVERLAY_LOWBRIDGE_18:
		case OVERLAY_LOWBRIDGE_28:
			index |= 8;
			break;
		default:
			break;
	}

	return(index);
}


/// <summary>
/// Damages the low bridge that this cell belongs to.
/// This is the entry point the combat code calls when a weapon lands on a low bridge. The
/// cell's overlay says which way the bridge runs and where its center line lies; the east-west
/// or north-south worker does the damage.
/// </summary>
/// <param name="cell">The cell that was hit.</param>
/// <returns>bool; Was the bridge destroyed by this hit?</returns>
bool MapClass::Damage_Low_Bridge(Cell const & cell)
{
	OverlayType otype = Map[cell].Overlay;
	if (otype >= OVERLAY_LOWBRIDGE_01 && otype <= OVERLAY_LOWBRIDGE_09 || otype >= OVERLAY_LOWBRIDGE_19 && otype <= OVERLAY_LOWBRIDGE_22 || otype == OVERLAY_LOWBRIDGE_27) {
		if (!Is_Low_Bridge(cell + Cell(0, -1))) {
			return(Damage_Low_Bridge_EW(cell + Cell(0, 1)));
		} else {
			if (!Is_Low_Bridge(cell + Cell(0, -2))) {
				return(Damage_Low_Bridge_EW(cell));
			} else {
				return(Damage_Low_Bridge_EW(cell - Cell(0, 1)));
			}
		}
	} else if (otype >= OVERLAY_LOWBRIDGE_10 && otype <= OVERLAY_LOWBRIDGE_18 || otype >= OVERLAY_LOWBRIDGE_23 && otype <= OVERLAY_LOWBRIDGE_26 || otype == OVERLAY_LOWBRIDGE_28) {
		if (!Is_Low_Bridge(cell + Cell(-1, 0))) {
			return(Damage_Low_Bridge_NS(cell + Cell(1, 0)));
		} else {
			if (!Is_Low_Bridge(cell + Cell(-2, 0))) {
				return(Damage_Low_Bridge_NS(cell));
			} else {
				return(Damage_Low_Bridge_NS(cell - Cell(1, 0)));
			}
		}
	}
	return(false);
}


/// <summary>
/// Damages the center span of an east-west low bridge.
/// The span degrades a stage at a time and drags its flanking pieces down with it. Past the
/// last stage the bridge is destroyed, which means rebuilding the zones, repainting the radar
/// and springing the destruction triggers.
/// </summary>
/// <param name="cell">The center cell of the span that was hit.</param>
/// <returns>bool; Was the bridge destroyed outright?</returns>
bool MapClass::Damage_Low_Bridge_EW(Cell const & cell)
{
	CellClass * cellptr1 = &Map[cell];
	CellClass * cellptr2 = &Map[cell - Cell(0, 1)];
	CellClass * cellptr3 = &Map[cell + Cell(0, 1)];

	OverlayType otype = cellptr1->Overlay;
	bool didwork = false;

	Rect draw_rect = Union(cellptr1->Overlay_Render_Rect(), cellptr1->Overlay_Shadow_Render_Rect());
	Rect recalc_rect(0, 0, 0, 0);

	do {
		if (otype == OVERLAY_LOWBRIDGE_19) {
			cellptr2->Overlay = OVERLAY_LOWBRIDGE_20;
			cellptr3->Overlay = OVERLAY_LOWBRIDGE_20;
			cellptr1->Overlay = OVERLAY_LOWBRIDGE_20;
			Damage_Low_Bridge_Piece_EW(cell - Cell(1, 0));
		} else if (otype == OVERLAY_LOWBRIDGE_21) {
			cellptr2->Overlay = OVERLAY_LOWBRIDGE_22;
			cellptr3->Overlay = OVERLAY_LOWBRIDGE_22;
			cellptr1->Overlay = OVERLAY_LOWBRIDGE_22;
			Damage_Low_Bridge_Piece_EW(cell + Cell(1, 0));
		} else if (otype < OVERLAY_LOWBRIDGE_07) {
			cellptr2->Overlay = OVERLAY_LOWBRIDGE_07;
			cellptr3->Overlay = OVERLAY_LOWBRIDGE_07;
			cellptr1->Overlay = OVERLAY_LOWBRIDGE_07;
			Damage_Low_Bridge_Piece_EW(cell - Cell(1, 0));
			Damage_Low_Bridge_Piece_EW(cell + Cell(1, 0));
		} else if (otype < OVERLAY_LOWBRIDGE_10) {
			cellptr2->Overlay = OVERLAY_LOWBRIDGE_27;
			cellptr3->Overlay = OVERLAY_LOWBRIDGE_27;
			cellptr1->Overlay = OVERLAY_LOWBRIDGE_27;
			Map.Radar_Background(cellptr2->CellID);
			Map.Radar_Background(cellptr3->CellID);
			Map.Radar_Background(cellptr1->CellID);
			Damage_Low_Bridge_Piece_EW(cell - Cell(1, 0));
			Damage_Low_Bridge_Piece_EW(cell + Cell(1, 0));
			Spring_Low_Bridge_Destroyed_EW(cell);
			didwork = true;
			recalc_rect = Rect(cell.X - 1, cell.Y - 1, 3, 3);
		} else {
			break;
		}

		draw_rect = Union(draw_rect, Union(cellptr1->Overlay_Render_Rect(), cellptr1->Overlay_Shadow_Render_Rect())) - TacticalRect.TopLeft;
		TacticalMap->Register_Dirty_Area(draw_rect);

		cellptr1->Recalc_Attributes(-1);
		cellptr2->Recalc_Attributes(-1);
		cellptr3->Recalc_Attributes(-1);
		cellptr1->Kill_Illegal_Occupiers();
		cellptr2->Kill_Illegal_Occupiers();
		cellptr3->Kill_Illegal_Occupiers();

		if (didwork) {
			Zone_Reset();
		}

		if (recalc_rect.Is_Valid()) {
			Recalc_Cells_In_Rect(recalc_rect);
		}
	} while (false);

	return(didwork);
}


/// <summary>
/// Damages the center span of a north-south low bridge.
/// North-south counterpart of Damage_Low_Bridge_EW. The span degrades a stage at a time and
/// drags its flanking pieces down with it; past the last stage the bridge is destroyed and the
/// zones, the radar and the destruction triggers all have to be dealt with.
/// </summary>
/// <param name="cell">The center cell of the span that was hit.</param>
/// <returns>bool; Was the bridge destroyed outright?</returns>
bool MapClass::Damage_Low_Bridge_NS(Cell const & cell)
{
	CellClass * cellptr1 = &Map[cell];
	CellClass * cellptr2 = &Map[cell - Cell(1, 0)];
	CellClass * cellptr3 = &Map[cell + Cell(1, 0)];

	OverlayType otype = cellptr1->Overlay;
	bool didwork = false;

	Rect draw_rect = Union(cellptr1->Overlay_Render_Rect(), cellptr1->Overlay_Shadow_Render_Rect());
	Rect recalc_rect(0, 0, 0, 0);

	do {
		if (otype == OVERLAY_LOWBRIDGE_23) {
			cellptr2->Overlay = OVERLAY_LOWBRIDGE_24;
			cellptr3->Overlay = OVERLAY_LOWBRIDGE_24;
			cellptr1->Overlay = OVERLAY_LOWBRIDGE_24;
			Damage_Low_Bridge_Piece_NS(cell + Cell(0, 1));
		} else if (otype == OVERLAY_LOWBRIDGE_25) {
			cellptr2->Overlay = OVERLAY_LOWBRIDGE_26;
			cellptr3->Overlay = OVERLAY_LOWBRIDGE_26;
			cellptr1->Overlay = OVERLAY_LOWBRIDGE_26;
			Damage_Low_Bridge_Piece_NS(cell - Cell(0, 1));
		} else if (otype < OVERLAY_LOWBRIDGE_16) {
			cellptr2->Overlay = OVERLAY_LOWBRIDGE_16;
			cellptr3->Overlay = OVERLAY_LOWBRIDGE_16;
			cellptr1->Overlay = OVERLAY_LOWBRIDGE_16;
			Damage_Low_Bridge_Piece_NS(cell - Cell(0, 1));
			Damage_Low_Bridge_Piece_NS(cell + Cell(0, 1));
		} else if (otype < OVERLAY_LOWBRIDGE_19) {
			cellptr2->Overlay = OVERLAY_LOWBRIDGE_28;
			cellptr3->Overlay = OVERLAY_LOWBRIDGE_28;
			cellptr1->Overlay = OVERLAY_LOWBRIDGE_28;
			Map.Radar_Background(cellptr2->CellID);
			Map.Radar_Background(cellptr3->CellID);
			Map.Radar_Background(cellptr1->CellID);
			Damage_Low_Bridge_Piece_NS(cell - Cell(0, 1));
			Damage_Low_Bridge_Piece_NS(cell + Cell(0, 1));
			Spring_Low_Bridge_Destroyed_NS(cell);
			didwork = true;
			recalc_rect = Rect(cell.X - 1, cell.Y - 1, 3, 3);
		} else {
			break;
		}

		draw_rect = Union(draw_rect, Union(cellptr1->Overlay_Render_Rect(), cellptr1->Overlay_Shadow_Render_Rect())) - TacticalRect.TopLeft;
		TacticalMap->Register_Dirty_Area(draw_rect);

		cellptr1->Recalc_Attributes(-1);
		cellptr2->Recalc_Attributes(-1);
		cellptr3->Recalc_Attributes(-1);
		cellptr1->Kill_Illegal_Occupiers();
		cellptr2->Kill_Illegal_Occupiers();
		cellptr3->Kill_Illegal_Occupiers();

		if (didwork) {
			Zone_Reset();
		}

		if (recalc_rect.Is_Valid()) {
			Recalc_Cells_In_Rect(recalc_rect);
		}
	} while (false);

	return(didwork);
}


/// <summary>
/// Springs the destruction triggers for a north-south low bridge.
/// The full extent of the run is measured first, since a trigger may be watching any cell of
/// the bridge rather than the one that happened to be hit.
/// </summary>
/// <param name="cell">A cell of the destroyed run.</param>
void MapClass::Spring_Low_Bridge_Destroyed_NS(Cell cell)
{
	Cell tmp = cell;
	do {
		tmp = Adjacent_Cell(tmp, FACING_N);
	} while (Is_Low_Bridge(tmp));

	Cell start = Adjacent_Cell(tmp, FACING_S);

	tmp = cell;
	do {
		tmp = Adjacent_Cell(tmp, FACING_S);
	} while (Is_Low_Bridge(tmp));

	Cell end = Adjacent_Cell(tmp, FACING_N);

	Spring_Bridge_Destruction_Triggers(start, end);
}


/// <summary>
/// Springs the destruction triggers for an east-west low bridge.
/// The full extent of the run is measured first, since a trigger may be watching any cell of
/// the bridge rather than the one that happened to be hit.
/// </summary>
/// <param name="cell">A cell of the destroyed run.</param>
void MapClass::Spring_Low_Bridge_Destroyed_EW(Cell cell)
{
	Cell tmp = cell;
	do {
		tmp = Adjacent_Cell(tmp, FACING_W);
	} while (Is_Low_Bridge(tmp));

	Cell start = Adjacent_Cell(tmp, FACING_E);

	tmp = cell;
	do {
		tmp = Adjacent_Cell(tmp, FACING_E);
	} while (Is_Low_Bridge(tmp));

	Cell end = Adjacent_Cell(tmp, FACING_W);

	Spring_Bridge_Destruction_Triggers(start, end);
}


/// <summary>
/// Damages one piece of an east-west low bridge.
/// The piece takes whichever overlay suits the neighbors it has left, which is what gives a
/// blown span its ragged ends. The span damage routine calls this for the pieces flanking the
/// cell that was hit.
/// </summary>
/// <param name="cell">The center cell of the piece to damage.</param>
void MapClass::Damage_Low_Bridge_Piece_EW(Cell const & cell)
{
	if (!Is_Low_Bridge(cell)) {
		return;
	}

	CellClass * cellptr1 = &Map[cell];
	CellClass * cellptr2 = &Map[cell - Cell(0, 1)];
	CellClass * cellptr3 = &Map[cell + Cell(0, 1)];

	Rect draw_rect = Union(cellptr1->Overlay_Render_Rect(), cellptr1->Overlay_Shadow_Render_Rect());

	OverlayType lookup[16] = {
		OVERLAY_NONE,
		OVERLAY_LOWBRIDGE_06,
		OVERLAY_LOWBRIDGE_09,
		OVERLAY_NONE,
		OVERLAY_LOWBRIDGE_05,
		OVERLAY_LOWBRIDGE_07,
		OVERLAY_LOWBRIDGE_09,
		OVERLAY_NONE,
		OVERLAY_LOWBRIDGE_08,
		OVERLAY_LOWBRIDGE_08,
		OVERLAY_LOWBRIDGE_27,
		OVERLAY_NONE,
		OVERLAY_NONE,
		OVERLAY_NONE,
		OVERLAY_NONE,
		OVERLAY_NONE
	};

	int mask = Get_Low_Bridge_EW_Neighbor_Mask(cell);
	OverlayType current = Map[cell].Overlay;

	if (mask > 0) {
		OverlayType newoverlay;

		if (current >= OVERLAY_LOWBRIDGE_19) {
			if (current == OVERLAY_LOWBRIDGE_19) {
				newoverlay = OVERLAY_LOWBRIDGE_20;
			} else if (current == OVERLAY_LOWBRIDGE_21) {
				newoverlay = OVERLAY_LOWBRIDGE_22;
			} else {
				return;
			}
		} else {
			newoverlay = lookup[mask];
			if (current == newoverlay) {
				return;
			}
		}

		cellptr3->Overlay = cellptr2->Overlay = cellptr1->Overlay = newoverlay;

		draw_rect = Union(draw_rect, Union(cellptr1->Overlay_Render_Rect(), cellptr1->Overlay_Shadow_Render_Rect())) - TacticalRect.TopLeft;
		TacticalMap->Register_Dirty_Area(draw_rect, false);

		if (newoverlay == OVERLAY_LOWBRIDGE_27) {
			Map.Radar_Background(cell);
			Map.Radar_Background(cell + Cell(0, 1));
			Map.Radar_Background(cell - Cell(0, 1));
		}

		cellptr1->Recalc_Attributes(-1);
		cellptr2->Recalc_Attributes(-1);
		cellptr3->Recalc_Attributes(-1);
		cellptr1->Kill_Illegal_Occupiers();
		cellptr2->Kill_Illegal_Occupiers();
		cellptr3->Kill_Illegal_Occupiers();
	}
}


/// <summary>
/// Damages one piece of a north-south low bridge.
/// North-south counterpart of Damage_Low_Bridge_Piece_EW. The piece takes whichever overlay
/// suits the neighbors it has left, which is what gives a blown span its ragged ends.
/// </summary>
/// <param name="cell">The center cell of the piece to damage.</param>
void MapClass::Damage_Low_Bridge_Piece_NS(Cell const & cell)
{
	if (!Is_Low_Bridge(cell)) {
		return;
	}

	CellClass * cellptr1 = &Map[cell];
	CellClass * cellptr2 = &Map[cell - Cell(1, 0)];
	CellClass * cellptr3 = &Map[cell + Cell(1, 0)];

	Rect draw_rect = Union(cellptr1->Overlay_Render_Rect(), cellptr1->Overlay_Shadow_Render_Rect());

	OverlayType lookup[16] = {
		OVERLAY_NONE,
		OVERLAY_LOWBRIDGE_15,
		OVERLAY_LOWBRIDGE_18,
		OVERLAY_NONE,
		OVERLAY_LOWBRIDGE_14,
		OVERLAY_LOWBRIDGE_16,
		OVERLAY_LOWBRIDGE_18,
		OVERLAY_NONE,
		OVERLAY_LOWBRIDGE_17,
		OVERLAY_LOWBRIDGE_17,
		OVERLAY_LOWBRIDGE_28,
		OVERLAY_NONE,
		OVERLAY_NONE,
		OVERLAY_NONE,
		OVERLAY_NONE,
		OVERLAY_NONE
	};

	int mask = Get_Low_Bridge_NS_Neighbor_Mask(cell);
	OverlayType current = cellptr1->Overlay;

	if (mask > 0) {
		OverlayType newoverlay;

		if (current >= OVERLAY_LOWBRIDGE_23) {
			if (current == OVERLAY_LOWBRIDGE_23) {
				newoverlay = OVERLAY_LOWBRIDGE_24;
			} else if (current == OVERLAY_LOWBRIDGE_25) {
				newoverlay = OVERLAY_LOWBRIDGE_26;
			} else {
				return;
			}
		} else {
			newoverlay = lookup[mask];
			if (newoverlay == current) {
				return;
			}
		}

		cellptr3->Overlay = cellptr2->Overlay = cellptr1->Overlay = newoverlay;

		draw_rect = Union(draw_rect, Union(cellptr1->Overlay_Render_Rect(), cellptr1->Overlay_Shadow_Render_Rect())) - TacticalRect.TopLeft;
		TacticalMap->Register_Dirty_Area(draw_rect, false);

		if (newoverlay == OVERLAY_LOWBRIDGE_28) {
			Map.Radar_Background(cell);
			Map.Radar_Background(cell + Cell(1, 0));
			Map.Radar_Background(cell - Cell(1, 0));
		}

		cellptr1->Recalc_Attributes(-1);
		cellptr2->Recalc_Attributes(-1);
		cellptr3->Recalc_Attributes(-1);
		cellptr1->Kill_Illegal_Occupiers();
		cellptr2->Kill_Illegal_Occupiers();
		cellptr3->Kill_Illegal_Occupiers();
	}
}


/// <summary>
/// Repairs the low bridge that this cell belongs to.
/// This is the entry point the repair code calls. The cell's overlay says which way the bridge
/// runs and where its center line lies; the east-west or north-south worker does the rest.
/// </summary>
/// <param name="cell">Any cell of the low bridge to repair.</param>
void MapClass::Repair_Low_Bridge_Span(Cell const & cell)
{
	OverlayType overlay = Map[cell].Overlay;

	if (overlay >= OVERLAY_LOWBRIDGE_01 && overlay <= OVERLAY_LOWBRIDGE_09 || overlay >= OVERLAY_LOWBRIDGE_19 && overlay <= OVERLAY_LOWBRIDGE_22 || overlay == OVERLAY_LOWBRIDGE_27) {
		if (!Is_Low_Bridge(cell - Cell(0, 1))) {
			Repair_Low_Bridge_EW(cell + Cell(0, 1));
		} else {
			if (!Is_Low_Bridge(cell - Cell(0, 2))) {
				Repair_Low_Bridge_EW(cell);
			} else {
				Repair_Low_Bridge_EW(cell - Cell(0, 1));
			}
		}
	} else if (overlay >= OVERLAY_LOWBRIDGE_10 && overlay <= OVERLAY_LOWBRIDGE_18 || overlay >= OVERLAY_LOWBRIDGE_23 && overlay <= OVERLAY_LOWBRIDGE_26 || overlay == OVERLAY_LOWBRIDGE_28) {
		if (!Is_Low_Bridge(cell - Cell(1, 0))) {
			Repair_Low_Bridge_NS(cell + Cell(1, 0));
		} else {
			if (!Is_Low_Bridge(cell - Cell(2, 0))) {
				Repair_Low_Bridge_NS(cell);
			} else {
				Repair_Low_Bridge_NS(cell - Cell(1, 0));
			}
		}
	}
}


/// <summary>
/// Repairs an east-west low bridge run.
/// The run is walked from its western end and every piece along it is put back to its intact
/// overlay, so a repaired bridge comes back whole rather than a cell at a time.
/// </summary>
/// <param name="cell">A cell on the center line of the run to repair.</param>
void MapClass::Repair_Low_Bridge_EW(Cell const & cell)
{
	Cell cellnum = cell;
	bool changed = false;

	do {
		cellnum.X--;
	} while (Is_Low_Bridge(cellnum));
	cellnum.X++;

	Rect recalc_rect(0, 0, 0, 0);

	do {
		CellClass * cellptr1 = &Map[cellnum];
		CellClass * cellptr2 = &Map[cellnum + Cell(0, -1)];
		CellClass * cellptr3 = &Map[cellnum + Cell(0, 1)];

		Rect draw_rect = Union(cellptr1->Overlay_Render_Rect(), cellptr1->Overlay_Shadow_Render_Rect());

		OverlayType otype = cellptr1->Overlay;
		OverlayType newtype = otype;

		switch (otype) {
			case OVERLAY_LOWBRIDGE_19:
			case OVERLAY_LOWBRIDGE_20:
				newtype = OVERLAY_LOWBRIDGE_19;
				break;

			case OVERLAY_LOWBRIDGE_21:
			case OVERLAY_LOWBRIDGE_22:
				newtype = OVERLAY_LOWBRIDGE_21;
				break;

			case OVERLAY_LOWBRIDGE_05:
			case OVERLAY_LOWBRIDGE_06:
			case OVERLAY_LOWBRIDGE_07:
			case OVERLAY_LOWBRIDGE_08:
			case OVERLAY_LOWBRIDGE_09:
			case OVERLAY_LOWBRIDGE_27:
				newtype = (OverlayType)(Pick_Random_UInt(0, 3) + OVERLAY_LOWBRIDGE_01);
				recalc_rect = Union(recalc_rect, Rect(cellnum.X, cellnum.Y - 1, 1, 3));
				changed = true;
				break;

			default:
				break;
		}

		if (newtype != otype) {
			cellptr1->Overlay = newtype;
			cellptr2->Overlay = newtype;
			cellptr3->Overlay = newtype;

			draw_rect = Union(draw_rect, Union(cellptr1->Overlay_Render_Rect(), cellptr1->Overlay_Shadow_Render_Rect())) - TacticalRect.TopLeft;
			TacticalMap->Register_Dirty_Area(draw_rect, false);

			if (otype == OVERLAY_LOWBRIDGE_27) {
				Map.Radar_Background(cellnum);
				Map.Radar_Background(cellnum + Cell(0, 1));
				Map.Radar_Background(cellnum - Cell(0, 1));
			}

			cellptr1->Recalc_Attributes(-1);
			cellptr2->Recalc_Attributes(-1);
			cellptr3->Recalc_Attributes(-1);
		}

		cellnum.X++;
	} while (Is_Low_Bridge(cellnum));

	if (changed) {
		Zone_Reset();
	}

	if (recalc_rect.Is_Valid()) {
		Recalc_Cells_In_Rect(recalc_rect);
	}
}


/// <summary>
/// Repairs a north-south low bridge run.
/// North-south counterpart of Repair_Low_Bridge_EW. The run is walked from its northern end
/// and every piece along it is put back to its intact overlay.
/// </summary>
/// <param name="cell">A cell on the center line of the run to repair.</param>
void MapClass::Repair_Low_Bridge_NS(Cell const & cell)
{
	Cell cellnum = cell;
	Rect recalc_rect(0, 0, 0, 0);
	bool changed = false;

	do {
		cellnum.Y--;
	} while (Is_Low_Bridge(cellnum));
	cellnum.Y++;

	do {
		CellClass * cellptr1 = &Map[cellnum];
		CellClass * cellptr2 = &Map[cellnum + Cell(-1, 0)];
		CellClass * cellptr3 = &Map[cellnum + Cell(1, 0)];

		Rect draw_rect = Union(cellptr1->Overlay_Render_Rect(), cellptr1->Overlay_Shadow_Render_Rect());

		OverlayType otype = cellptr1->Overlay;
		OverlayType newtype = otype;

		switch (otype) {
			case OVERLAY_LOWBRIDGE_23:
			case OVERLAY_LOWBRIDGE_24:
				newtype = OVERLAY_LOWBRIDGE_23;
				break;

			case OVERLAY_LOWBRIDGE_25:
			case OVERLAY_LOWBRIDGE_26:
				newtype = OVERLAY_LOWBRIDGE_25;
				break;

			case OVERLAY_LOWBRIDGE_14:
			case OVERLAY_LOWBRIDGE_15:
			case OVERLAY_LOWBRIDGE_16:
			case OVERLAY_LOWBRIDGE_17:
			case OVERLAY_LOWBRIDGE_18:
			case OVERLAY_LOWBRIDGE_28:
				newtype = (OverlayType)(Pick_Random_UInt(0, 3) + OVERLAY_LOWBRIDGE_10);
				recalc_rect = Union(recalc_rect, Rect(cellnum.X - 1, cellnum.Y, 3, 1));
				changed = true;
				break;

			default:
				break;
		}

		if (newtype != otype) {
			cellptr1->Overlay = newtype;
			cellptr2->Overlay = newtype;
			cellptr3->Overlay = newtype;

			draw_rect = Union(draw_rect, Union(cellptr1->Overlay_Render_Rect(), cellptr1->Overlay_Shadow_Render_Rect())) - TacticalRect.TopLeft;
			TacticalMap->Register_Dirty_Area(draw_rect, false);

			if (otype == OVERLAY_LOWBRIDGE_28) {
				Map.Radar_Background(cellnum);
				Map.Radar_Background(cellnum + Cell(1, 0));
				Map.Radar_Background(cellnum - Cell(1, 0));
			}

			cellptr1->Recalc_Attributes(-1);
			cellptr2->Recalc_Attributes(-1);
			cellptr3->Recalc_Attributes(-1);
		}

		cellnum.Y++;
	} while (Is_Low_Bridge(cellnum));

	if (changed) {
		Zone_Reset();
	}

	if (recalc_rect.Is_Valid()) {
		Recalc_Cells_In_Rect(recalc_rect);
	}
}


/// <summary>
/// Tests whether the cell's overlay is any low bridge variant (OVERLAY_LOWBRIDGE_01..28).
/// </summary>
/// <param name="cell">The cell to test.</param>
/// <returns>True if the cell holds a low bridge overlay; false otherwise.</returns>
bool MapClass::Is_Low_Bridge(Cell const & cell)
{
	OverlayType overlay = Map[cell].Overlay;
	return(overlay >= OVERLAY_LOWBRIDGE_01 && overlay <= OVERLAY_LOWBRIDGE_28);
}


/// <summary>
/// Fetches a shrouded cell near the unit.
/// Team scripts use this routine to send a scout somewhere the house has not looked yet. The
/// search radiates outward from the unit and one of the nearest unexplored cells is taken at
/// random, so a pack of scouts does not all set off for the same corner of the map.
/// </summary>
/// <param name="foot">The unit to search around.</param>
/// <returns>Returns with a pointer to the shrouded cell picked. If nothing nearby is shrouded,
/// the unit's own cell is returned.</returns>
CellClass * MapClass::Find_Nearby_Shroud(FootClass * foot)
{
	Cell cells[24];

	Cell center = foot->Center_Coord().As_Cell();
	int xx = center.X;
	int yy = center.Y;

	int found = 0;

	/*
	 * Limit the radius of the scan to the size of the visible play area, but never
	 * larger than 32 cells.
	 */
	int radius = 32;
	if (PlayRect.Width + PlayRect.Height <= 32) {
		radius = PlayRect.Width + PlayRect.Height;
	}

	/*
	 * Radiate outward from the unit's location in concentric square rings, gathering
	 * any shrouded cells that lie within the local radar area.
	 */
	for (int ring = 0; ring < radius; ring++) {

		/*
		**	Scan the top and bottom rows of the "box".
		*/
		for (int x = -ring; x <= ring; x++) {

			Cell cell = Cell(xx + x, yy - ring);
			if (In_Local_Radar(cell, true)) {
				Coord coord(cell, 0);
				coord.Z = Get_Height_GL(coord);
				if (Is_Shrouded(coord)) {
					cells[found++] = cell;
				}
			}
			if (found == 24) {
				break;
			}

			cell = Cell(xx + x, yy + ring);
			if (In_Local_Radar(cell, true)) {
				Coord coord(cell, 0);
				coord.Z = Get_Height_GL(coord);
				if (Is_Shrouded(coord)) {
					cells[found++] = cell;
				}
			}
			if (found == 24) {
				break;
			}
		}
		if (found == 24) {
			break;
		}

		/*
		**	Scan the left and right columns of the "box".
		*/
		for (int y = -ring + 1; y <= ring - 1; y++) {

			Cell cell = Cell(xx - ring, yy + y);
			if (In_Local_Radar(cell, true)) {
				Coord coord(cell, 0);
				coord.Z = Get_Height_GL(coord);
				if (Is_Shrouded(coord)) {
					cells[found++] = cell;
				}
			}
			if (found == 24) {
				break;
			}

			cell = Cell(xx + ring, yy + y);
			if (In_Local_Radar(cell, true)) {
				Coord coord(cell, 0);
				coord.Z = Map[coord].Get_Height(coord);
				if (Is_Shrouded(coord)) {
					cells[found++] = cell;
				}
			}
			if (found == 24) {
				break;
			}
		}
		if (found == 24) {
			break;
		}
	}

	if (found > 0) {
		return(&Map[cells[Scen->RandomNumber(0, found - 1)]]);
	}

	return(&Map[center]);
}


/// <summary>
/// Tears down map subsystems: deletes all movement zones and subzone connection structures,
/// clears subzone tracking and connection hash tables, and resets the veinhole monster and
/// Tiberium spread/growth systems.
/// </summary>
void MapClass::Shutdown(void)
{
	int index;

	for (index = 0; index < MZONE_COUNT; index++) {
		if (Zones[index] != NULL) {
			delete Zones[index];
			Zones[index] = NULL;
		}
	}

	ZoneAdjacency.clear();

	for (index = 0; index < SUBZONE_COUNT; index++) {
		SubzoneTracking[index].Clear();
		SubzoneConnectionStaging[index].clear();
	}

	VeinholeMonsterClass::Reset();
	VeinholeMonsterClass::Clear_Global_Data();
	TiberiumClass::Deinit_Tiberium_Spread_System();
	TiberiumClass::Deinit_Tiberium_Growth_System();
}


/// <summary>
/// Collapses a destroyable cliff into a slope.
/// The cliff tile is swapped for its ruined counterpart and a pair of slope pieces, which
/// leaves a way up where there was none -- so the zones and subzones are rebuilt, everything
/// standing on the tile is retargeted, and a shower of rubble is thrown over the wreckage.
/// </summary>
/// <param name="cptr">Cell holding the destroyable cliff tile that came down.</param>
void MapClass::Collapse_Cliff(CellClass * cptr)
{
	if (cptr->ITType == IsometricTileTypeClass::DestroyableCliffs + DESTROYABLE_CLIFFS_COUNT-2) {

		Cell point = Cell(-(cptr->SubTile % 6), cptr->SubTile / -6) + cptr->CellID;

		/*
		 * Stamp the destroyed cliff replacement tile onto the map, then throw it away.
		 */
		ObjectClass * tile = IsometricTileTypes[IsometricTileTypeClass::DestroyableCliffs]->Create_One_Of(NULL);
		tile->Set_Coord(Coord(point));
		tile->IsInLimbo = false;
		tile->IsDown = true;
		tile->Mark(MARK_UP);
		delete tile;

		/*
		 * Place the two slope set pieces that form the new sloped ground.
		 */
		ObjectClass * slope1 = IsometricTileTypes[IsometricTileTypeClass::SlopeSetPieces]->Create_One_Of(NULL);
		slope1->Unlimbo(Coord(point), DIR_N);

		ObjectClass * slope2 = IsometricTileTypes[IsometricTileTypeClass::SlopeSetPieces + 1]->Create_One_Of(NULL);
		slope2->Unlimbo(Coord(point + Cell(3, 0)), DIR_N);

		Zone_Reset();

		IsometricTileTypeClass * ittype = IsometricTileTypes[IsometricTileTypeClass::DestroyableCliffs];
		IsoTileSet const * image = (IsoTileSet const *)ittype->Get_Image_Data();

		/*
		 * Recalculate the attributes of every cell the cliff tile touches.
		 */
		int x;
		int y;
		for (y = 0; y < ittype->Height; y++) {
			for (x = 0; x < ittype->Width; x++) {
				Cell cell = point + Cell(x, y);
				if (image->Fetch_Record_Pointer_Unsafe(x + y * ittype->Width) != NULL) {
					if (In_Local_Radar(cell, true)) {
						CellSubzones[Get_Cell_Subzone_Index(cell)].SubzoneID[SUBZONE_FINE] = 0;
						Map[cell].Recalc_Attributes();
					}
				}
			}
		}

		/*
		 * Rebuild the subzone graph for those cells.
		 */
		for (y = 0; y < ittype->Height; y++) {
			for (x = 0; x < ittype->Width; x++) {
				Cell cell = point + Cell(x, y);
				if (image->Fetch_Record_Pointer_Unsafe(x + y * ittype->Width) != NULL) {
					if (In_Local_Radar(cell, true) && CellSubzones[Get_Cell_Subzone_Index(cell)].SubzoneID[SUBZONE_FINE] == 0) {
						Update_Cell_Subzones(cell);
					}
				}
			}
		}

		/*
		 * Refresh mission targets and the radar background for those cells.
		 */
		for (y = 0; y < ittype->Height; y++) {
			for (x = 0; x < ittype->Width; x++) {
				Cell cell = point + Cell(x, y);
				if (image->Fetch_Record_Pointer_Unsafe(x + y * ittype->Width) != NULL) {
					if (In_Local_Radar(cell, true)) {
						TechnoClass::Remove_Target(&Map[cell]);
						Map.Radar_Background(cell);
					}
				}
			}
		}

		/*
		 * Mark the affected screen region as dirty so it gets redrawn.
		 */
		Point2D pixel1;
		TacticalMap->Coord_To_Pixel(Coord(point, LEVEL_LEPTON_H * Map[point].Height), pixel1);

		Point2D pixel2;
		TacticalMap->Coord_To_Pixel(Coord(point + Cell(5, 3), LEVEL_LEPTON_H * Map[point + Cell(5, 3)].Height), pixel2);

		Rect dirty;
		dirty.X = pixel1.X - 2 * ISO_TILE_PIXEL_W;
		dirty.Y = pixel1.Y - 2 * ISO_TILE_PIXEL_H;
		dirty.Width = pixel2.X + 4 * ISO_TILE_PIXEL_W - pixel1.X;
		dirty.Height = pixel2.Y + 4 * ISO_TILE_PIXEL_H - pixel1.Y;
		TacticalMap->Register_Dirty_Area(dirty, false);

		/*
		 * Scatter the debris animations across the collapsed area.
		 */
		AnimTypeClass * med1 = AnimTypes[AnimTypeClass::From_Name("XGRYMED1")];
		AnimTypeClass * med2 = AnimTypes[AnimTypeClass::From_Name("XGRYMED2")];
		AnimTypeClass * sml1 = AnimTypes[AnimTypeClass::From_Name("XGRYSML1")];

		for (y = point.Y; y < point.Y + 3; y++) {
			for (x = point.X; x < point.X + 5; x++) {
				Cell cell(x, y);
				AnimTypeClass * type = NULL;
				for (int i = 0; i < 2; i++) {
					switch (Scen->RandomNumber(0, 2)) {
						case 0:
							type = med1;
							break;

						case 1:
							type = med2;
							break;

						case 2:
							type = sml1;
							break;
					}
					AnimClass * anim = new AnimClass(type, Coord(cell, LEVEL_LEPTON_H * Map[cell].Height) + Coord(Scen->RandomNumber(-12, 12), Scen->RandomNumber(-8, 8), 0), Random_Pick(0, 2), 1, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL), 0);
				}
			}
		}

	} else if (cptr->ITType == IsometricTileTypeClass::DestroyableCliffs + DESTROYABLE_CLIFFS_COUNT-1) {

		Cell point = Cell(-(cptr->SubTile % 4), -(cptr->SubTile >> 2)) + cptr->CellID;

		/*
		 * Stamp the destroyed cliff replacement tile onto the map, then throw it away.
		 */
		ObjectClass * tile = IsometricTileTypes[IsometricTileTypeClass::DestroyableCliffs + DESTROYABLE_CLIFFS_COUNT - 1]->Create_One_Of(NULL);
		tile->Set_Coord(Coord(point));
		tile->IsInLimbo = false;
		tile->IsDown = true;
		tile->Mark(MARK_UP);
		delete tile;

		/*
		 * Place the two slope set pieces that form the new sloped ground.
		 */
		ObjectClass * slope1 = IsometricTileTypes[IsometricTileTypeClass::SlopeSetPieces + 3]->Create_One_Of(NULL);
		slope1->Unlimbo(Coord(point), DIR_N);

		ObjectClass * slope2 = IsometricTileTypes[IsometricTileTypeClass::SlopeSetPieces + 2]->Create_One_Of(NULL);
		slope2->Unlimbo(Coord(point + Cell(0, 3)), DIR_N);

		Zone_Reset();

		IsometricTileTypeClass * ittype = IsometricTileTypes[IsometricTileTypeClass::DestroyableCliffs + DESTROYABLE_CLIFFS_COUNT - 1];
		IsoTileSet const * image = (IsoTileSet const *)ittype->Get_Image_Data();

		/*
		 * Recalculate the attributes of every cell the cliff tile touches.
		 */
		int x;
		int y;
		for (y = 0; y < ittype->Height; y++) {
			for (x = 0; x < ittype->Width; x++) {
				Cell cell = point + Cell(x, y);
				if (image->Fetch_Record_Pointer_Unsafe(x + y * ittype->Width) != NULL) {
					if (In_Local_Radar(cell, true)) {
						CellSubzones[Get_Cell_Subzone_Index(cell)].SubzoneID[SUBZONE_FINE] = 0;
						Map[cell].Recalc_Attributes();
					}
				}
			}
		}

		/*
		 * Rebuild the subzone graph for those cells.
		 */
		for (y = 0; y < ittype->Height; y++) {
			for (x = 0; x < ittype->Width; x++) {
				Cell cell = point + Cell(x, y);
				if (image->Fetch_Record_Pointer_Unsafe(x + y * ittype->Width) != NULL) {
					if (In_Local_Radar(cell, true) && CellSubzones[Get_Cell_Subzone_Index(cell)].SubzoneID[SUBZONE_FINE] == 0) {
						Update_Cell_Subzones(cell);
					}
				}
			}
		}

		/*
		 * Refresh mission targets and the radar background for those cells.
		 */
		for (y = 0; y < ittype->Height; y++) {
			for (x = 0; x < ittype->Width; x++) {
				Cell cell = point + Cell(x, y);
				if (image->Fetch_Record_Pointer_Unsafe(x + y * ittype->Width) != NULL) {
					if (In_Local_Radar(cell, true)) {
						TechnoClass::Remove_Target(&Map[cell]);
						Map.Radar_Background(cell);
					}
				}
			}
		}

		/*
		 * Mark the affected screen region as dirty so it gets redrawn.
		 */
		Point2D pixel1;
		TacticalMap->Coord_To_Pixel(Coord(point, LEVEL_LEPTON_H * Map[point].Height), pixel1);

		Point2D pixel2;
		TacticalMap->Coord_To_Pixel(Coord(point + Cell(3, 5), LEVEL_LEPTON_H * Map[point + Cell(3, 5)].Height), pixel2);

		Rect dirty;
		dirty.X = pixel2.X - 2 * ISO_TILE_PIXEL_W;
		dirty.Y = pixel1.Y - 2 * ISO_TILE_PIXEL_H;
		dirty.Width = pixel1.X + 4 * ISO_TILE_PIXEL_W - pixel2.X;
		dirty.Height = pixel2.Y + 4 * ISO_TILE_PIXEL_H - pixel1.Y;
		TacticalMap->Register_Dirty_Area(dirty, false);

		/*
		 * Scatter the debris animations across the collapsed area.
		 */
		AnimTypeClass * med1 = AnimTypes[AnimTypeClass::From_Name("XGRYMED1")];
		AnimTypeClass * med2 = AnimTypes[AnimTypeClass::From_Name("XGRYMED2")];
		AnimTypeClass * sml1 = AnimTypes[AnimTypeClass::From_Name("XGRYSML1")];

		for (y = point.Y; y < point.Y + 5; y++) {
			for (x = point.X; x < point.X + 3; x++) {
				Cell cell(x, y);
				AnimTypeClass * type = NULL;
				for (int i = 0; i < 2; i++) {
					switch (Scen->RandomNumber(0, 2)) {
						case 0:
							type = med1;
							break;

						case 1:
							type = med2;
							break;

						case 2:
							type = sml1;
							break;
					}
					AnimClass * anim = new AnimClass(type, Coord(cell, LEVEL_LEPTON_H * Map[cell].Height) + Coord(Scen->RandomNumber(-12, 12), Scen->RandomNumber(-8, 8), 0), Random_Pick(0, 2), 1, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL), 0);
				}
			}
		}
	}
}


/// <summary>
/// Rebuilds every subzone coarseness level from scratch.
/// Clears each level's tracking list, calls Reset_Subzone for it, then resets the search state.
/// </summary>
void MapClass::Reset_All_Subzones(void)
{
	for (int index = SUBZONE_COUNT-1; index >= 0; index--) {
		SubzoneTracking[index].Clear();
		Reset_Subzone(index);
	}
	Search.Reset();
}


/// <summary>
/// Rebuilds one level of the subzone graph from scratch.
/// Every cell of the play area is sorted into a fresh subzone and the bridge and tunnel links
/// are stitched back in, leaving the hierarchical path search a graph it can trust. Use this
/// routine when the map has changed too much to be patched cell by cell.
/// </summary>
/// <param name="subzone">The subzone level to rebuild.</param>
void MapClass::Reset_Subzone(int subzone)
{
	SubzoneConnectionStaging[subzone].clear();

	DynamicVectorClass<SubzoneTrackingStruct> * track = &SubzoneTracking[subzone];
	CellSubzoneStruct * subend = &CellSubzones[CellZoneCount];

	/*
	 * Reset the per-cell subzone identifiers for this level and refresh the
	 * cached zone id and height from the master zone array.
	 */
	CellSubzoneStruct * subptr = CellSubzones;
	if (subptr < subend) {
		CellZoneStruct * zoneptr = CellZones;
		do {
			subptr->SubzoneID[subzone] = 0;
			subptr->ZoneID = zoneptr->ZoneID;
			subptr->Height = zoneptr->Height;
			subptr++;
			zoneptr++;
		} while (subptr < subend);
	}

	/*
	 * Seed the tracking list with the sentinel entry (index 0). This entry
	 * represents the "impassable" placeholder and is never a real subzone.
	 */
	int entry_count = 1;
	track->Add(SubzoneTrackingStruct());
	(*track)[0].ParentSubzoneID = 0;
	(*track)[0].Passability = PASSABLE_OUTSIDE;

	/*
	 * Walk every cell in the diamond shaped play area. Whenever an unassigned
	 * passable cell is found, flood fill a new subzone from it and record the
	 * resulting subzone in the tracking list.
	 */
	Rect bounds;
	bounds.Width = 1 << (subzone + 1);
	subptr = CellSubzones;
	CellZoneStruct * zoneptr = CellZones;
	bounds.Height = bounds.Width;
	int regionmask = bounds.Width - 1;
	int rowwidth = Map.PlayRect.Width + Map.PlayRect.Height + 1;
	int X = 0;
	bounds.X = 0;
	bounds.Y = 0;
	int rowx = 0;
	int Y = 0;
	while (subptr < subend) {
		int passability = zoneptr->Passability;
		int parent;
		if (subzone + 1 < SUBZONE_COUNT) {
			parent = subptr->SubzoneID[subzone + 1];
		} else {
			parent = 0;
		}

		if (passability == PASSABLE_OUTSIDE || subptr->SubzoneID[subzone] != 0) {
			subptr++;
			zoneptr++;
			X++;
			rowx = X;
		} else {
			Cell cell;
			cell.Y = Y;
			LastAdjacentZone = 0;
			cell.X = X;
			int count = Subzone_Span(subptr, subzone, entry_count, bounds, cell);

			track->Add(SubzoneTrackingStruct());

			SubzoneTrackingStruct * added = &(*track)[(unsigned short)entry_count];
			added->ParentSubzoneID = (unsigned short)parent;
			added->Passability = (PassabilityType)passability;
			added->Connections.Set_Growth_Step(16);
			added->ThreatRegion = (short)X / REGION_WIDTH + MAP_REGION_WIDTH * ((short)Y / REGION_HEIGHT) + (MAP_REGION_WIDTH + 1);

			zoneptr = &zoneptr[count];
			entry_count++;
			X = count + rowx;
			subptr = &subptr[count];
			rowx = X;
		}

		if (X == rowwidth) {
			X = 0;
			Y++;
			rowx = 0;
			bounds.X = 0;
			bounds.Y = Y - (regionmask & Y);
		} else if ((X & regionmask) == 0) {
			bounds.X = X;
		}
	}

	SubzoneTrackingEntryCount[subzone] = (unsigned short)entry_count;

	/*
	 * Re-register the zone-to-zone connections for this subzone level.
	 */
	Register_Subzone_Zone_Connections(subzone);

	Register_Staged_Subzone_Connections(subzone);
}


/// <summary>
/// Grows one subzone outward from a seed cell.
/// This is the low level fill routine behind the subzone rebuild. Cells join the subzone while
/// they share the seed's zone and stay within a height step of their neighbor, and the fill is
/// penned inside the seed's block so that no subzone ever straddles two of them. Where the fill
/// comes up against a subzone that is already numbered, the pair is staged as a link.
/// </summary>
/// <param name="seed">The cell entry to seed the fill from.</param>
/// <param name="subzone_level">The subzone level being rebuilt.</param>
/// <param name="subzone_id">The identifier to stamp onto every cell taken.</param>
/// <param name="bounds">The block the fill may not leave.</param>
/// <param name="cell">Map position of the seed cell.</param>
/// <returns>Returns with the distance the caller should advance its scan along the row.</returns>
int MapClass::Subzone_Span(CellSubzoneStruct * seed, int subzone_level, int subzone_id, Rect const & bounds, Cell const & cell)
{
	int zone = seed->ZoneID;

	CellSubzoneStruct * begin = seed;
	CellSubzoneStruct * end = seed;

	int x = cell.X;
	int y = cell.Y;

	int xmin = bounds.X;
	int ymin = bounds.Y;
	int last_adjacent = -1;
	int begin_x = x;
	int xmax = bounds.Width + bounds.X - 1;
	int end_x = x;
	int ymax = bounds.Height + bounds.Y - 1;

	SubzoneLinkStaging & staging = SubzoneConnectionStaging[subzone_level];
	int prev_height = seed->Height;

	/*
	**	Find the full extent of the current span by first scanning leftward
	**	until a boundary is reached.
	*/
	while (true) {
		if (x < xmin) {
			break;
		}
		if (abs(begin->Height - prev_height) >= 2) {
			break;
		}
		begin->SubzoneID[subzone_level] = (short)subzone_id;
		prev_height = begin->Height;
		int prev_zone = begin[-1].ZoneID;
		begin--;
		x--;
		if (prev_zone != zone) {
			break;
		}
	}

	begin_x = x;
	int begin_subzone = begin->SubzoneID[subzone_level];
	if (begin_subzone > 0 && abs(begin->Height - prev_height) < 2 && begin_subzone != last_adjacent) {
		Cell probe;
		probe = Cell(x, y);
		if (In_Local_Radar(probe, true)) {
			probe = Cell(x + 1, y);
			if (In_Local_Radar(probe, true)) {
				Stage_Subzone_Link(staging, begin_subzone, (unsigned short)subzone_id, false);
				last_adjacent = begin_subzone;
			}
		}
	}

	/*
	**	Scan rightward until a boundary is reached. This will then define the
	**	extent of the current span.
	*/
	prev_height = seed->Height;
	while (end->ZoneID == zone) {
		if (end_x > xmax) {
			break;
		}
		if (abs(end->Height - prev_height) >= 2) {
			break;
		}
		end->SubzoneID[subzone_level] = (short)subzone_id;
		prev_height = end->Height;
		end++;
		end_x++;
	}

	int end_subzone = end->SubzoneID[subzone_level];
	if (end_subzone > 0 && abs(end->Height - prev_height) < 2 && end_subzone != last_adjacent) {
		Cell probe;
		probe.X = end_x;
		probe.Y = y;
		if (In_Local_Radar(probe, true)) {
			probe = Cell(end_x - 1, y);
			if (In_Local_Radar(probe, true)) {
				Stage_Subzone_Link(staging, end_subzone, (unsigned short)subzone_id, false);
				last_adjacent = end_subzone;
			}
		}
	}

	int skip = end - seed - 1;
	int stride = PlayRect.Width + PlayRect.Height + 1;

	CellSubzoneStruct * above = &begin[-stride];
	CellSubzoneStruct * below = &begin[stride];
	x = begin_x;

	Cell span_cell;
	span_cell.X = 0;
	span_cell.Y = 0;

	/*
	 * Fill the row above the current span, recursing into any same-zone neighbor
	 * spans and recording connections to adjacent foreign subzones.
	 */
	while (x <= end_x) {
		int shadow_subzone = above->SubzoneID[subzone_level];
		CellSubzoneStruct * span_ptr;
		if (x == begin_x) {
			Cell left;
			left.Y = y;
			left.X = x + 1;
			span_cell = left;
			span_ptr = &above[stride + 1];
		} else if (x < end_x) {
			Cell middle;
			middle.X = x;
			middle.Y = y;
			span_cell = middle;
			span_ptr = &above[stride];
		} else {
			Cell right;
			right.X = x - 1;
			right.Y = y;
			span_cell = right;
			span_ptr = &above[stride - 1];
		}

		if (shadow_subzone != 0 || y <= ymin || x < xmin || x > xmax) {
			if (shadow_subzone != (unsigned short)subzone_id && shadow_subzone != last_adjacent) {
				if (shadow_subzone != 0) {
					if (abs(span_ptr->Height - above->Height) < 2 && In_Local_Radar(span_cell, true)) {
						Cell shadow_cell;
						shadow_cell.Y = y - 1;
						shadow_cell.X = x;
						if (In_Local_Radar(shadow_cell, true)) {
							Stage_Subzone_Link(staging, shadow_subzone, (unsigned short)subzone_id, x < xmin || x > xmax);
							last_adjacent = shadow_subzone;
						}
					}
				}
			}
			above++;
			x++;
		} else if (above->ZoneID == zone) {
			if (abs(span_ptr->Height - above->Height) < 2) {
				Cell shadow_cell;
				shadow_cell.X = x;
				shadow_cell.Y = y - 1;
				Subzone_Span(above, subzone_level, subzone_id, bounds, shadow_cell);
				continue;
			}
			above++;
			x++;
		} else {
			above++;
			x++;
		}
	}

	/*
	 * Fill the row below the current span using the same rules.
	 */
	x = begin_x;
	while (x <= end_x) {
		int shadow_subzone = below->SubzoneID[subzone_level];
		CellSubzoneStruct * span_ptr;
		if (x == begin_x) {
			Cell left;
			left.X = x + 1;
			left.Y = y;
			span_cell = left;
			span_ptr = &below[-stride + 1];
		} else if (x < end_x) {
			Cell middle;
			middle.X = x;
			middle.Y = y;
			span_cell = middle;
			span_ptr = &below[-stride];
		} else {
			Cell right;
			span_ptr = &below[-stride - 1];
			right.X = x - 1;
			right.Y = y;
			span_cell = right;
		}

		if (shadow_subzone != 0 || y >= ymax || x < xmin || x > xmax) {
			if (shadow_subzone != (unsigned short)subzone_id && shadow_subzone != last_adjacent) {
				if (shadow_subzone != 0) {
					if (abs(below->Height - span_ptr->Height) < 2 && In_Local_Radar(span_cell, true)) {
						Cell shadow_cell;
						shadow_cell.X = x;
						shadow_cell.Y = y + 1;
						if (In_Local_Radar(shadow_cell, true)) {
							Stage_Subzone_Link(staging, shadow_subzone, (unsigned short)subzone_id, x < xmin || x > xmax);
							last_adjacent = shadow_subzone;
						}
					}
				}
			}
			below++;
			x++;
		} else if (below->ZoneID == zone && abs(below->Height - span_ptr->Height) < 2) {
			Subzone_Span(below, subzone_level, subzone_id, bounds, Cell(x, y + 1));
		} else {
			below++;
			x++;
		}
	}

	return(skip);
}


/// <summary>
/// Re-registers all active ZoneConnections into the given subzone level's hash table.
/// Calls Register_Zone_Connection_Entries for each connection with IsPassable set.
/// </summary>
/// <param name="subzone">The subzone (coarseness) level to register connections for.</param>
void MapClass::Register_Subzone_Zone_Connections(int subzone)
{
	for (int i = 0; i < ZoneConnections.Count(); i++) {
		ZoneConnectionClass & connection = ZoneConnections[i];
		if (connection.IsPassable) {
			Register_Zone_Connection_Entries(connection, subzone);
		}
	}
}


/// <summary>
/// Records the subzone links a bridge or tunnel makes at one subzone level.
/// A subzone rebuild only links cells that touch, so a span has to be stitched in by hand.
/// This routine stages the link between the two ends of the span and between the lanes either
/// side of them; the rebuild later unpacks the staged pairs into the subzone adjacency lists.
/// </summary>
/// <param name="connection">The bridge or tunnel connection to stitch in.</param>
/// <param name="index">The subzone level whose staging set receives the links.</param>
void MapClass::Register_Zone_Connection_Entries(ZoneConnectionClass & connection, int index)
{
	SubzoneLinkStaging & staging = SubzoneConnectionStaging[index];

	Cell from = connection.From;
	Cell to = connection.To;

	Cell enter1;
	Cell enter2;
	Cell newcell1;
	Cell newcell2;

	CellClass * cell1ptr = &Map[from];

	if (!cell1ptr->Is_Tile_Bridge() && !cell1ptr->Is_Tile_Train_Bridge()) {

		FacingType enter_dir = cell1ptr->Get_Tunnel()->EnterDir;
		enter1 = Adjacent_Cell(from, (FacingType)(enter_dir + FACING_90));
		enter2 = Adjacent_Cell(from, (FacingType)(enter_dir - FACING_90));

		CellClass * enterptr1 = &Map[enter1];
		TubeClass * tube1 = enterptr1->Get_Tunnel();
		CellClass * enterptr2 = &Map[enter2];
		TubeClass * tube2 = enterptr2->Get_Tunnel();

		if (tube1 == NULL || tube2 == NULL) {
			return;
		}

		newcell1 = Follow_Path(enter1, tube1->Count, tube1->Dirs);
		newcell2 = Follow_Path(enter2, tube2->Count, tube2->Dirs);
	} else {
		IsometricTileType bridge_set = cell1ptr->Is_Tile_Bridge() ? IsometricTileTypeClass::BridgeSet : IsometricTileTypeClass::TrainBridgeSet;
		FacingType dir = BridgeSideFacings[cell1ptr->ITType - bridge_set];

		enter1 = Adjacent_Cell(from, dir);
		enter2 = Adjacent_Cell(from, (FacingType)(dir + FACING_180));
		newcell1 = Adjacent_Cell(to, dir);
		newcell2 = Adjacent_Cell(to, (FacingType)(dir - FACING_180));
	}

	{
		int from_zone = CellSubzones[Get_Cell_Subzone_Index(from)].SubzoneID[index];
		int to_zone = CellSubzones[Get_Cell_Subzone_Index(to)].SubzoneID[index];

		Stage_Subzone_Link(staging, from_zone, to_zone, false);
	}

	{
		int from_zone = CellSubzones[Get_Cell_Subzone_Index(enter1)].SubzoneID[index];
		int to_zone = CellSubzones[Get_Cell_Subzone_Index(newcell1)].SubzoneID[index];

		Stage_Subzone_Link(staging, from_zone, to_zone, false);
	}

	{
		int from_zone = CellSubzones[Get_Cell_Subzone_Index(enter2)].SubzoneID[index];
		int to_zone = CellSubzones[Get_Cell_Subzone_Index(newcell2)].SubzoneID[index];

		Stage_Subzone_Link(staging, from_zone, to_zone, false);
	}
}


/// <summary>
/// Writes the links staged for one subzone level into the two subzones each one joins.
/// Every link is recorded in both subzones, so the graph the route search walks is undirected.
/// The route search reads a subzone's neighbors in list order and settles a tie among equally
/// cheap blocks by which of them it reached first, so the order the links are written in is
/// the order it breaks those ties in.
/// </summary>
/// <param name="subzone">The subzone level whose staged links are to be written out.</param>
void MapClass::Register_Staged_Subzone_Connections(int subzone)
{
	DynamicVectorClass<SubzoneTrackingStruct> & track = SubzoneTracking[subzone];

	for (auto const & [link, crossblock] : SubzoneConnectionStaging[subzone]) {
		SubzoneConnectionStruct conn;
		conn.SubzoneID = link.first;
		conn.IsCrossBlock = crossblock;
		track[link.second].Connections.Add(conn);

		SubzoneConnectionStruct conn2;
		conn2.SubzoneID = link.second;
		conn2.IsCrossBlock = crossblock;
		track[link.first].Connections.Add(conn2);
	}
}


/// <summary>
/// Fetches the deck cell that stands in for a cell beneath a bridge.
/// The path search calls this routine to lift its start or end point up onto the bridge, since
/// the ground below belongs to another zone entirely. The cell's offset across the span is
/// carried over, so the answer lands in the same lane of the deck.
/// </summary>
/// <param name="cptr">The cell to resolve.</param>
/// <param name="isbridge">Should a cell beneath a bridge be lifted onto the deck?</param>
/// <returns>Returns with the deck cell to use, or the cell's own position when it is not
/// beneath a bridge.</returns>
Cell MapClass::Get_Bridge_Zone_Connection_Cell(CellClass * cptr, bool isbridge)
{
	Cell ncell;

	if (isbridge && cptr->IsUnderBridge) {
		int cidx = Map.Zone_Connection_Index(cptr->CellID, 2, 0);
		if (cidx == -1) {
			ncell = Find_Bridge_Span_End_Cell(cptr->CellID, cptr->CellID);
			if (ncell != CELL_NONE) {
				return(ncell);
			}
		}

		ZoneConnectionClass *zcon = &Map.ZoneConnections[cidx];
		if (cptr->IsBridgeEastWest) {
			ncell = Cell(0, cptr->CellID.Y - zcon->From.Y);
		} else {
			ncell = Cell(cptr->CellID.X - zcon->From.X, 0);
		}

		if (zcon->IsPassable) {
			if ((zcon->From + ncell - cptr->CellID).Length() < (zcon->To + ncell - cptr->CellID).Length()) {
				return(zcon->From + ncell);
			} else {
				return(zcon->To + ncell);
			}
		} else {
			FacingType facing = zcon->From.X != zcon->To.X ? FACING_E : FACING_S;
			while (cptr->IsUnderBridge) {
				cptr = &cptr->Adjacent_Cell(facing);
			}
			if ((cptr->Is_Tile_Bridge() || cptr->Is_Tile_Train_Bridge()) && cptr->Land_Type() != LAND_ROCK) {
				return(ncell + zcon->To);
			} else {
				return(ncell + zcon->From);
			}
		}
	}

	return(cptr->CellID);
}


/// <summary>
/// Fetches the cell a unit should head for to cross this connection.
/// Unit movement code calls this routine when its destination lies on a bridge or a tunnel.
/// The endpoint nearest the reference is preferred, but a span that has been blown offers only
/// whichever of its ends is still standing.
/// </summary>
/// <param name="cell">The cell to resolve a connection for.</param>
/// <param name="reference">Cell to measure against when picking between the two endpoints.</param>
/// <returns>Returns with the endpoint chosen. A cell on no connection is its own answer.</returns>
Cell MapClass::Get_Zone_Connection_Destination(Cell const & cell, Cell const & reference)
{
	Cell ncell;

	int cidx = Map.Zone_Connection_Index(cell, 1, 0);
	if (cidx != -1) {

		ZoneConnectionClass *zcon = &Map.ZoneConnections[cidx];

		if (zcon->IsPassable) {
			if ((reference - zcon->From).Length() < (reference - zcon->To).Length()) {
				return(zcon->From);
			} else {
				return(zcon->To);
			}
		} else {
			FacingType facing = zcon->From.X != zcon->To.X ? FACING_E : FACING_S;
			CellClass *cptr = &Map[cell];
			while (cptr->IsUnderBridge) {
				cptr = &cptr->Adjacent_Cell(facing);
			}
			if ((cptr->Is_Tile_Bridge() || cptr->Is_Tile_Train_Bridge()) && cptr->Land_Type() != LAND_ROCK) {
				return(zcon->To);
			} else {
				return(zcon->From);
			}
		}
	}

	ncell = Find_Bridge_Span_End_Cell(cell, reference);
	if (ncell != CELL_NONE) {
		return(ncell);
	}

	return(cell);
}


/// <summary>
/// Finds the nearer end of the bridge that passes over this cell.
/// The ground beneath a bridge is walkable but the deck above it is not reachable from there,
/// so movement code uses this routine to fetch the deck cell a unit would have to make for.
/// Ends that lie outside the local radar are no use and are passed over.
/// </summary>
/// <param name="cell">The cell to resolve. A cell not under a bridge is its own answer.</param>
/// <param name="reference">Cell to measure against when both ends would serve.</param>
/// <returns>Returns with the bridge end cell chosen. If neither end will serve, CELL_NONE is
/// returned.</returns>
Cell MapClass::Find_Bridge_Span_End_Cell(Cell const & cell, Cell const & reference)
{
	CellClass * cptr = &Map[cell];

	if (!cptr->IsUnderBridge) {
		return(cptr->CellID);
	}

	FacingType fwd = cptr->IsBridgeEastWest ? FACING_E : FACING_S;
	FacingType bwd = Facing_Sub(fwd, FACING_180);

	CellClass * fcell = cptr;
	CellClass * bcell = cptr;

	Cell fresult = CELL_NONE;
	Cell bresult = CELL_NONE;

	while (true) {

		if (fcell->IsUnderBridge) {
			fcell = &fcell->Adjacent_Cell(fwd);
			if (!fcell->IsUnderBridge && (fcell->Is_Tile_Bridge() || fcell->Is_Tile_Train_Bridge()) && fcell->Land_Type() != LAND_ROCK) {
				fresult = fcell->CellID;
			}
		}

		if (bcell->IsUnderBridge) {
			bcell = &bcell->Adjacent_Cell(bwd);
			if (bcell->IsUnderBridge) {
				continue;
			}
			if ((bcell->Is_Tile_Bridge() || bcell->Is_Tile_Train_Bridge()) && bcell->Land_Type() != LAND_ROCK) {
				bresult = bcell->CellID;
			}
			if (bcell->IsUnderBridge) {
				continue;
			}
		}

		if (!fcell->IsUnderBridge) {
			break;
		}
	}

	Cell result = CELL_NONE;

	if (Map.In_Local_Radar(fresult, true)) {
		result = fresult;
	}

	if (Map.In_Local_Radar(bresult, true)) {
		if (result != CELL_NONE) {
			short bdist = bresult.Distance_To(reference);
			short fdist = fresult.Distance_To(reference);
			if (bdist < fdist) {
				result = bresult;
			}
		} else {
			result = bresult;
		}
	}

	return(result);
}


/// <summary>
/// Fetches the end of a bridge that belongs to a given subzone.
/// Ground beneath a bridge is not part of the bridge's own subzone, so the path search uses
/// this routine to find the deck cell -- or one of the cells alongside it -- that is, and
/// prices the detour over the bridge from there.
/// </summary>
/// <param name="cell">The cell to resolve. A cell not under a bridge is its own answer.</param>
/// <param name="subzone_level">The subzone level to match at.</param>
/// <param name="subzone_id">The subzone a candidate cell has to belong to.</param>
/// <returns>Returns with the matching bridge end cell. If neither end will serve, CELL_NONE is
/// returned.</returns>
Cell MapClass::Find_Bridge_End_Cell_For_Subzone(Cell const & cell, int subzone_level, int subzone_id)
{
	CellClass * cellptr = &Map[cell];
	if (!cellptr->IsUnderBridge) {
		return(cellptr->Fetch_CellID());
	}

	FacingType facing = cellptr->IsBridgeEastWest ? FACING_E : FACING_S;
	FacingType reverse = Facing_Sub(facing, FACING_180);

	Cell bridge_start = CELL_NONE;
	Cell bridge_end = CELL_NONE;

	CellClass * cptr1 = cellptr;
	CellClass * cptr2 = cellptr;

	while (true) {
		if (cptr1->IsUnderBridge) {
			cptr1 = &cptr1->Adjacent_Cell(facing);
			if (!cptr1->IsUnderBridge && (cptr1->Is_Tile_Bridge() || cptr1->Is_Tile_Train_Bridge()) && cptr1->Land_Type() != LAND_ROCK) {
				bridge_start = cptr1->CellID;
			}
		}
		if (cptr2->IsUnderBridge) {
			cptr2 = &cptr2->Adjacent_Cell(reverse);
			if (!cptr2->IsUnderBridge && (cptr2->Is_Tile_Bridge() || cptr2->Is_Tile_Train_Bridge()) && cptr2->Land_Type() != LAND_ROCK) {
				bridge_end = cptr2->CellID;
			}
		}
		if (!cptr2->IsUnderBridge && !cptr1->IsUnderBridge) {
			break;
		}
	}

	Cell cells[6];
	cells[0] = bridge_start;
	cells[1] = Adjacent_Cell(bridge_start, Facing_Add(facing, FACING_90));
	cells[2] = Adjacent_Cell(bridge_start, Facing_Sub(facing, FACING_90));
	cells[3] = bridge_end;
	cells[4] = Adjacent_Cell(bridge_end, Facing_Add(facing, FACING_90));
	cells[5] = Adjacent_Cell(bridge_end, Facing_Sub(facing, FACING_90));

	for (int i = 0; i < ARRAY_SIZE(cells); i++) {
		Cell c = cells[i];
		if (Map.In_Local_Radar(c, true)) {
			if (CellSubzones[Get_Cell_Subzone_Index(c)].SubzoneID[subzone_level] == subzone_id) {
				return(c);
			}
		}
	}

	return(CELL_NONE);
}


/// <summary>
/// Works out what a unit can still reach from inside one subzone.
/// The path search turns to this routine when a route it had already planned turns out to be
/// blocked. Unlike the subzone graph itself, the search here honors whatever is actually
/// standing in the way, so it can tell a subzone the unit cannot cross at all from one whose
/// exits have merely been closed off.
/// </summary>
/// <param name="cptr">The cell the unit came to a halt at.</param>
/// <param name="subzone_level">The subzone level being examined.</param>
/// <param name="connections">Filled in with the neighboring subzones that proved
/// unreachable.</param>
/// <param name="foot">The unit whose ability to enter a cell decides what counts as
/// blocked.</param>
/// <returns>bool; Is part of the subzone itself cut off from the starting cell?</returns>
bool MapClass::Build_Reachable_Subzones(CellClass * cptr, int subzone_level, DynamicVectorClass<unsigned short> const & connections, FootClass const * foot)
{
	/*
	 * The flood is bounded by one coarse block, so the scratch it needs is a fixed size and
	 * is kept here rather than built up per call. _subzone_flood_stack is the depth first
	 * stack of cells still to be flooded, and _subzone_flood_visited is the visited grid,
	 * of stride 8 and indexed by the low bits of the cell coordinates.
	 */
	static CellClass * _subzone_flood_stack[64];
	static char _subzone_flood_visited[8][8];

	int dim = 1 << (subzone_level + 1);

	DynamicVectorClass<unsigned short> list;
	list.Clear();

	if (dim > 0) {
		for (int i = 0; i < dim; i++) {
			memset(_subzone_flood_visited[i], 0, dim);
		}
	}

	unsigned short other_subzone = 0;
	int count = 1;
	unsigned short start_subzone = CellSubzones[Get_Cell_Subzone_Index(cptr->CellID)].SubzoneID[subzone_level];
	_subzone_flood_stack[0] = cptr;
	_subzone_flood_visited[(dim - 1) & cptr->CellID.X][(dim - 1) & cptr->CellID.Y] = 1;

	int * mzone_table = MZonePassability[foot->TClass->MZone];

	while (true) {

		FacingType facing = FACING_N;
		int newcount = count - 1;
		CellClass * cur = _subzone_flood_stack[newcount];
		CellClass ** writeptr = &_subzone_flood_stack[newcount];

		while (true) {

			CellClass * adj = &cur->Adjacent_Cell(facing);
			int ax = (dim - 1) & adj->CellID.X;
			int ay = (dim - 1) & adj->CellID.Y;
			unsigned short adj_subzone = CellSubzones[Get_Cell_Subzone_Index(adj->CellID)].SubzoneID[subzone_level];

			if (foot->Can_Enter_Cell(adj, facing, adj->Height, NULL, true) == MOVE_OK || mzone_table[adj->Passability] != TRAVERSAL_PASSABLE) {

				if (adj_subzone == start_subzone) {

					if (!_subzone_flood_visited[ax][ay]) {
						_subzone_flood_visited[ax][ay] = 1;
						*writeptr = adj;
						newcount++;
						writeptr++;
					}

				} else if (adj_subzone != other_subzone) {

					if (adj_subzone != 0) {
						other_subzone = adj_subzone;

						int j;
						for (j = list.Count() - 1; j >= 0; j--) {
							if (list[j] == adj_subzone) {
								break;
							}
						}
						if (j == -1) {
							list.Add(adj_subzone);
						}
					}
				}
			}

			facing++;
			if (facing >= FACING_COUNT) {
				break;
			}
		}

		count = newcount;
		if (count <= 0) {
			break;
		}
	}

	int bx = (dim - 1) & cptr->CellID.X;
	int by = (dim - 1) & cptr->CellID.Y;

	for (int row = 0; row < dim; row++) {
		for (int col = 0; col < dim; col++) {

			Cell c = cptr->CellID + Cell(bx, by);

			if (In_Local_Radar(c, true)) {
				if (start_subzone == (unsigned short)CellSubzones[Get_Cell_Subzone_Index(c)].SubzoneID[subzone_level] && !_subzone_flood_visited[row][col]) {
					return(true);
				}
			}
		}
	}

	SubzoneTrackingStruct & track = SubzoneTracking[subzone_level][start_subzone];
	int last = track.Connections.Count() - 1;
	if (last >= 0) {
		SubzoneConnectionStruct * conn = &track.Connections[last];
		int remaining = last + 1;
		do {
			int sub = conn->SubzoneID;

			int j;
			for (j = list.Count() - 1; j >= 0; j--) {
				if (list[j] == sub) {
					break;
				}
			}
			if (j == -1) {
				((DynamicVectorClass<unsigned short> &)connections).Add(sub);
			}

			conn--;
			remaining--;
		} while (remaining != 0);
	}

	return(false);
}


/// <summary>
/// Rebuilds the subzone graph around a changed cell.
/// A wall going up, a crater or a collapsed cliff only disturbs the subzones in its own
/// neighborhood, so this routine tears down and refills just that patch at each level instead
/// of rebuilding the map. Any bridge or tunnel connection reaching into the patch is hooked
/// back up, and the path search is reset afterwards, since the subzone numbering it had been
/// working from has moved underneath it.
/// </summary>
/// <param name="cell">The cell whose terrain has changed.</param>
void MapClass::Update_Cell_Subzones(Cell const & cell)
{
	if (In_Local_Radar(cell)) {

		int y;
		int x;
		Cell trycell;
		for (int subzone = 2; subzone >= 0; subzone--) {

			/*
			 * Compute the square window of cells (aligned to the level's grid size)
			 * that surrounds the changed cell for this coarseness level.
			 */
			int var = subzone + 1;
			Rect bounds;
			bounds.Width = 1 << var;
			bounds.Height = bounds.Width;
			bounds.X = cell.X - cell.X % bounds.Width;
			bounds.Y = cell.Y - cell.Y % bounds.Height;

			SubzoneConnectionStaging[subzone].clear();

			DynamicVectorClass<unsigned short> collected;

			DynamicVectorClass<SubzoneTrackingStruct> * track = &SubzoneTracking[subzone];

			/*
			 * Collect the distinct subzone ids that currently occupy the window and
			 * clear each cell's subzone id for this level (refreshing its zone id).
			 */
			for (y = bounds.Y; y < bounds.Y + bounds.Height; y++) {
				for (x = bounds.X; x < bounds.Width + bounds.X; x++) {
					CellSubzoneStruct * subptr = &CellSubzones[(short)x + (short)y * (PlayRect.Width + PlayRect.Height + 1)];
					unsigned short subid = subptr->SubzoneID[subzone];
					if (subid != 0) {
						int found;
						for (found = collected.Count() - 1; found >= 0; found--) {
							if (collected[found] == subid) {
								break;
							}
						}
						if (found == -1) {
							collected.Add(subid);
						}
					}
					subptr->SubzoneID[subzone] = 0;
					subptr->ZoneID = CellZones[(short)x + (short)y * (PlayRect.Width + PlayRect.Height + 1)].ZoneID;
				}
			}

			/*
			 * Remove the collected subzones from the tracking graph. For each one,
			 * delete the mirror connection entry from every neighbor it connected to,
			 * then clear its own connection list.
			 */
			for (int idx = collected.Count() - 1; idx >= 0; idx--) {
				unsigned short subid = collected[idx];
				SubzoneTrackingStruct * entry = &SubzoneTracking[subzone][subid];
				for (int i = entry->Connections.Count() - 1; i >= 0; i--) {
					SubzoneTrackingStruct * neighbor = &SubzoneTracking[subzone][entry->Connections[i].SubzoneID];
					int j;
					for (j = neighbor->Connections.Count() - 1; j >= 0; j--) {
						if (neighbor->Connections[j].SubzoneID == subid) {
							neighbor->Connections.Delete_Index(j);
							break;
						}
					}
				}
				entry->Connections.Clear();
			}

			/*
			 * Flood fill fresh subzones for the now unassigned passable cells in the
			 * window, recording a tracking entry for each new subzone.
			 */
			int entry_count = SubzoneTrackingEntryCount[subzone];
			for (y = bounds.Y; y < bounds.Y + bounds.Height; y++) {
				for (x = bounds.X; x < bounds.Width + bounds.X; x++) {

					Cell cella;
					cella.X = x;
					cella.Y = y;
					if (In_Local_Radar(cella)) {

						CellSubzoneStruct * subptr = &CellSubzones[(short)y * (PlayRect.Width + PlayRect.Height + 1) + (short)x];
						int passability = CellZones[(short)y * (PlayRect.Width + PlayRect.Height + 1) + (short)x].Passability;
						if (passability != PASSABLE_OUTSIDE && subptr->SubzoneID[subzone] == 0) {

							trycell.Y = y;
							trycell.X = x;
							Subzone_Span(subptr, subzone, entry_count, bounds, trycell);

							track->Add(SubzoneTrackingStruct());

							SubzoneTrackingStruct * added = &(*track)[entry_count];
							int parent = 0;
							if (subzone != 2) {
								parent = subptr->SubzoneID[subzone + 1];
							}
							added->ParentSubzoneID = (unsigned short)parent;
							added->Passability = (PassabilityType)passability;
							added->Connections.Set_Growth_Step(16);
							added->ThreatRegion = (short)x / REGION_WIDTH + MAP_REGION_WIDTH * ((short)y / REGION_HEIGHT) + (MAP_REGION_WIDTH + 1);

							entry_count++;
							if (entry_count == 0) {
								Reset_All_Subzones();
								return;
							}
						}
					}
				}
			}
			SubzoneTrackingEntryCount[subzone] = entry_count;

			/*
			 * Re-register any zone connections whose endpoints fall inside the window.
			 */
			for (int connection_index = ZoneConnections.Count() - 1; connection_index >= 0; connection_index--) {
				ZoneConnectionClass & connection = ZoneConnections[connection_index];
				if ((connection.From.X >= bounds.X && connection.From.X < bounds.Width + bounds.X
					&& connection.From.Y >= bounds.Y && connection.From.Y < bounds.Y + bounds.Height)
					|| (connection.To.X >= bounds.X && connection.To.X < bounds.Width + bounds.X
					&& connection.To.Y >= bounds.Y && connection.To.Y < bounds.Y + bounds.Height)) {
					if (connection.IsPassable) {
						Register_Zone_Connection_Entries(connection, subzone);
					}
				}
			}

			Register_Staged_Subzone_Connections(subzone);
		}

		/*
		 * Propagate the parent subzone identifiers down through the coarser levels
		 * for the finest (8 cell) window around the changed cell.
		 */
		int basex = cell.X - cell.X % 8;
		int basey = cell.Y - cell.Y % 8;
		for (y = basey; y < basey + 8; y++) {
			for (x = basex; x < basex + 8; x++) {
				trycell.X = x;
				trycell.Y = y;
				if (In_Local_Radar(trycell)) {
					CellSubzoneStruct * subptr = &CellSubzones[(short)x + (short)y * (PlayRect.Width + PlayRect.Height + 1)];
					for (int level = 0; level < 2; level++) {
						SubzoneTracking[level][subptr->SubzoneID[level]].ParentSubzoneID = subptr->SubzoneID[level + 1];
					}
				}
			}
		}
		Search.Reset();
	}
}


/// <summary>
/// Removes a connection's subzone links from every subzone level's tracking graph.
/// Deletes both directions of the main endpoint pair and the two flanking bridge-adjacent pairs.
/// </summary>
/// <param name="connection">The zone connection to unregister.</param>
void MapClass::Unregister_Subzone_Connection(ZoneConnectionClass * connection)
{
	Cell from = connection->From;
	Cell to = connection->To;

	CellClass * from_cptr = &Map[from];
	Cell none = CELL_NONE;
	Cell from_adj1 = none;
	Cell from_adj2 = none;
	Cell to_adj1 = none;
	Cell to_adj2 = none;

	if (from_cptr->Is_Tile_Bridge() || from_cptr->Is_Tile_Train_Bridge()) {
		bool is_bridge = from_cptr->Is_Tile_Bridge();
		int tile = from_cptr->ITType;
		int bridgeset = is_bridge ? IsometricTileTypeClass::BridgeSet : IsometricTileTypeClass::TrainBridgeSet;
		FacingType facing = BridgeSideFacings[tile - bridgeset];
		from_adj1 = Adjacent_Cell(from, facing);
		from_adj2 = Adjacent_Cell(from, Facing_Sub(facing, FACING_180));
		to_adj1 = Adjacent_Cell(to, facing);
		to_adj2 = Adjacent_Cell(to, Facing_Sub(facing, FACING_180));
	}

	for (int i = 0; i < ARRAY_SIZE(SubzoneTracking); i++) {
		{
			int sidx1 = CellSubzones[Get_Cell_Subzone_Index(from)].SubzoneID[i];
			int sidx2 = CellSubzones[Get_Cell_Subzone_Index(to)].SubzoneID[i];

			SubzoneConnectionStruct conn1;
			conn1.SubzoneID = sidx2;
			conn1.IsCrossBlock = false;
			SubzoneTracking[i][sidx1].Connections.Delete(conn1);

			SubzoneConnectionStruct conn2;
			conn2.SubzoneID = sidx1;
			conn2.IsCrossBlock = false;
			SubzoneTracking[i][sidx2].Connections.Delete(conn2);
		}

		{
			int sidx1 = CellSubzones[Get_Cell_Subzone_Index(from_adj1)].SubzoneID[i];
			int sidx2 = CellSubzones[Get_Cell_Subzone_Index(to_adj1)].SubzoneID[i];

			SubzoneConnectionStruct conn1;
			conn1.SubzoneID = sidx2;
			conn1.IsCrossBlock = false;
			SubzoneTracking[i][sidx1].Connections.Delete(conn1);

			SubzoneConnectionStruct conn2;
			conn2.SubzoneID = sidx1;
			conn2.IsCrossBlock = false;
			SubzoneTracking[i][sidx2].Connections.Delete(conn2);
		}

		{
			int sidx1 = CellSubzones[from_adj2.X + from_adj2.Y * (PlayRect.Width + PlayRect.Height + 1)].SubzoneID[i];
			int sidx2 = CellSubzones[to_adj2.X + to_adj2.Y * (PlayRect.Width + PlayRect.Height + 1)].SubzoneID[i];

			SubzoneConnectionStruct conn1;
			conn1.SubzoneID = sidx2;
			conn1.IsCrossBlock = false;
			SubzoneTracking[i][sidx1].Connections.Delete(conn1);

			SubzoneConnectionStruct conn2;
			conn2.SubzoneID = sidx1;
			conn2.IsCrossBlock = false;
			SubzoneTracking[i][sidx2].Connections.Delete(conn2);
		}
	}
}


/// <summary>
/// Adds a connection's subzone links into every subzone level's tracking graph.
/// Adds both directions of the main endpoint pair and the two flanking bridge-adjacent pairs.
/// </summary>
/// <param name="connection">The zone connection to register.</param>
void MapClass::Register_Subzone_Connection(ZoneConnectionClass * connection)
{
	Cell from = connection->From;
	Cell to = connection->To;

	CellClass * from_cptr = &Map[from];
	Cell from_adj1;
	Cell from_adj2;
	Cell to_adj1;
	Cell to_adj2 = CELL_NONE;

	if (from_cptr->Is_Tile_Bridge() || from_cptr->Is_Tile_Train_Bridge()) {
		bool is_bridge = from_cptr->Is_Tile_Bridge();
		int tile = from_cptr->ITType;
		int bridgeset = is_bridge ? IsometricTileTypeClass::BridgeSet : IsometricTileTypeClass::TrainBridgeSet;
		FacingType facing = BridgeSideFacings[tile - bridgeset];
		from_adj1 = Adjacent_Cell(from, facing);
		from_adj2 = Adjacent_Cell(from, Facing_Sub(facing, FACING_180));
		to_adj1 = Adjacent_Cell(to, facing);
		to_adj2 = Adjacent_Cell(to, Facing_Sub(facing, FACING_180));
	}

	for (int i = 0; i < ARRAY_SIZE(SubzoneTracking); i++) {
		{
			int sidx1 = CellSubzones[Get_Cell_Subzone_Index(from)].SubzoneID[i];
			int sidx2 = CellSubzones[Get_Cell_Subzone_Index(to)].SubzoneID[i];

			SubzoneConnectionStruct conn1;
			conn1.SubzoneID = sidx2;
			conn1.IsCrossBlock = false;
			SubzoneTracking[i][sidx1].Connections.Add(conn1);

			SubzoneConnectionStruct conn2;
			conn2.SubzoneID = sidx1;
			conn2.IsCrossBlock = false;
			SubzoneTracking[i][sidx2].Connections.Add(conn2);
		}

		{
			int sidx1 = CellSubzones[Get_Cell_Subzone_Index(from_adj1)].SubzoneID[i];
			int sidx2 = CellSubzones[Get_Cell_Subzone_Index(to_adj1)].SubzoneID[i];

			SubzoneConnectionStruct conn1;
			conn1.SubzoneID = sidx2;
			conn1.IsCrossBlock = false;
			SubzoneTracking[i][sidx1].Connections.Add(conn1);

			SubzoneConnectionStruct conn2;
			conn2.SubzoneID = sidx1;
			conn2.IsCrossBlock = false;
			SubzoneTracking[i][sidx2].Connections.Add(conn2);
		}

		{
			int sidx1 = CellSubzones[Get_Cell_Subzone_Index(from_adj2)].SubzoneID[i];
			int sidx2 = CellSubzones[to_adj2.X + to_adj2.Y * (PlayRect.Height + PlayRect.Width + 1)].SubzoneID[i];

			SubzoneConnectionStruct conn1;
			conn1.SubzoneID = sidx2;
			conn1.IsCrossBlock = false;
			SubzoneTracking[i][sidx1].Connections.Add(conn1);

			SubzoneConnectionStruct conn2;
			conn2.SubzoneID = sidx1;
			conn2.IsCrossBlock = false;
			SubzoneTracking[i][sidx2].Connections.Add(conn2);
		}
	}
}

/// <summary>
/// Hands every wall on the map to a house.
/// A wall overlay carries no owner of its own, so this routine gives each one to whichever
/// house has the nearest building capable of owning walls. Walls that arrive as plain terrain
/// end up belonging to somebody rather than to nobody at all.
/// </summary>
void MapClass::Initialize_Wall_Ownership(void)
{
	Reset_Iterator();
	CellClass * cellptr = MapClass::Iterate();

	while (cellptr != NULL) {
		if (cellptr->Overlay != OVERLAY_NONE) {
			if (OverlayTypes[cellptr->Overlay]->IsWall) {
				int bestindex = -1;
				int index = 0;
				int bestdist = 0x7FFFFFFF;

				for (index = 0; index < Buildings.Count(); index++) {
					BuildingClass * bptr = Buildings[index];
					int dist = bptr->Distance(cellptr);
					if (bptr->IsActive && bptr->IsDown && bptr->House->Class->IsWallOwner) {
						if (bestindex == -1 || dist < bestdist) {
							bestdist = dist;
							bestindex = index;
						}
					}
				}

				if (bestindex != -1) {
					cellptr->Owner = Buildings[bestindex]->House->HeapID;
				}
			}
		}
		cellptr = Iterate();
	}
}


/// <summary>
/// Rebuilds the body of water around a cell and re-dresses its shores.
/// This routine grows the water out from the seed, throws away any body that came out
/// unusable, and then lays the shore tiles on the land side and the water side so the new
/// coastline draws properly. The scratch data the region builder works on is created and
/// thrown away here when the map generator is not already running.
/// </summary>
/// <param name="origin">The cell the water is grown out from.</param>
/// <returns>bool; Did the whole region come out without running into a conflict?</returns>
bool MapClass::Rebuild_Shores_At(Cell const & origin)
{
	Map.Set_Cursor_Shape(NULL);

	bool is_temp_data = false;
	if (RMGCellData == NULL) {
		RMGCellData = new MapRegionClass::CellData[MapCellStride * MapCellStride];
		is_temp_data = true;
	}

	int r = MapCellStride;
	int s = r * r;
	for (int i = 0; i < s; i++) {
		MapRegionClass::Get_Cell_Data(i).InFillReach = true;
		MapRegionClass::Get_Cell_Data(i).WaterMask = -1;
	}

	Map.Reset_Iterator();
	CellClass * cptr = Map.Iterate();
	bool expanded = true;
	while (cptr != NULL && expanded) {
		expanded = Expand_Water(cptr, 0);
		cptr = Map.Iterate();
	}

	Map.Reset_Iterator();
	cptr = Map.Iterate();
	while (cptr != NULL && expanded) {
		Prune_Water(cptr);
		cptr = Map.Iterate();
	}

	Mark_Fill_Area(origin, true);

	Map.Reset_Iterator();
	cptr = Map.Iterate();
	while (cptr != NULL && expanded) {
		expanded = Place_Shore(cptr, 1, 0);
		cptr = Map.Iterate();
	}

	Map.Reset_Iterator();
	cptr = Map.Iterate();
	while (cptr != NULL && expanded) {
		expanded = Place_Shore(cptr, 2, 0);
		cptr = Map.Iterate();
	}

	ObjectClass * obj = Map.PendingObjectPtr;
	Map.PendingObject = NULL;
	if (obj) {
		delete obj;
		Map.PendingObjectPtr = NULL;
	}

	for (int j = 0; j < s; j++) {
		MapRegionClass::Get_Cell_Data(j).InFillReach = true;
	}

	if (is_temp_data) {
		delete [] RMGCellData;
		RMGCellData = NULL;
	}

	return(expanded);
}


/// <summary>
/// Marks out the stretch of ground that belongs with a cell, and how far it reaches.
/// This routine flags every cell of a piece -- water joining water, or clear ground at the
/// seed's own height -- and then flags the ring of cells around it as well. The cliff and
/// shore passes work from those two marks to decide what they may reshape and what they must
/// merely blend into.
/// </summary>
/// <param name="origin">The cell the fill is grown out from.</param>
/// <param name="on_water">Should the fill follow water rather than clear ground?</param>
void MapClass::Mark_Fill_Area(Cell const & origin, bool on_water)
{
	DynamicVectorClass<Cell> region_cells;
	region_cells.Clear();
	region_cells.Set_Growth_Step(100);

	DynamicVectorClass<Cell> frontier_cells;
	frontier_cells.Clear();
	frontier_cells.Set_Growth_Step(1000);

	for (int i = MapCellStride * MapCellStride - 1; i >= 0; i--) {
		MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(i);
		data.InFillArea = false;
		data.InFillReach = false;
	}

	frontier_cells.Add(origin);
	region_cells.Add(origin);

	if (In_Radar(origin)) {
		MapRegionClass::Get_Cell_Data(origin).InFillArea = true;
	}

	int height = Map[origin].Height;

	while (frontier_cells.Count()) {
		Cell cell = frontier_cells[frontier_cells.Count() - 1];
		frontier_cells.Delete_Index(frontier_cells.Count() - 1);

		for (int dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
			Cell adjacent = Adjacent_Cell(cell, (FacingType)dir);
			if (In_Radar(adjacent)) {
				MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(adjacent);
				CellClass * adjptr = &Map[adjacent];
				if (!data.InFillArea) {
					if (on_water && adjptr->Is_Tile_With_Water() || !on_water && adjptr->Height == height && adjptr->Is_Tile_Clear()) {
						frontier_cells.Add(adjacent);
						region_cells.Add(adjacent);
						data.InFillArea = true;
					}
				}
			}
		}
	}

	for (int j = 0; j < region_cells.Count(); j++) {
		Cell cell = region_cells[j];
		for (int y = cell.Y - 1; y < cell.Y + 2; y++) {
			for (int x = cell.X - 1; x < cell.X + 2; x++) {
				Cell influence(x, y);
				if (In_Radar(influence)) {
					MapRegionClass::Get_Cell_Data(Cell(x, y)).InFillReach = true;
				}
			}
		}
	}
}


/// <summary>
/// Rebuilds the cliffs around a cell whose ground height has changed.
/// This routine grows the height change out from the seed and then lays cliff tiles along the
/// border of the new region, so that it meets the surrounding land cleanly instead of ending
/// in a step. The scratch data the region builder works on is created and thrown away here
/// when the map generator is not already running.
/// </summary>
/// <param name="cell">The cell the height change is grown out from.</param>
/// <returns>bool; Did the whole region come out without running into a conflict?</returns>
bool MapClass::Rebuild_Cliffs_At(Cell const & cell)
{
	int s = MapCellStride * MapCellStride;
	bool expanded = true;

	bool is_temp_data = false;
	if (RMGCellData == NULL) {
		RMGCellData = new MapRegionClass::CellData[s];
		is_temp_data = true;
	} else {
		for (int i = 0; i < s; i++) {
			MapRegionClass::Get_Cell_Data(i).InFillReach = true;
		}
	}

	Map.Set_Cursor_Shape(NULL);

	Map.Reset_Iterator();
	CellClass * cptr = Map.Iterate();
	while (cptr != NULL && expanded) {
		expanded = Expand_High_Ground(cptr, 0);
		cptr = Map.Iterate();
	}

	Mark_Fill_Area(cell, false);

	Map.Reset_Iterator();
	cptr = Map.Iterate();
	while (cptr != NULL && expanded) {
		if (cptr->Ramp == RAMP_NONE && cptr->Is_Tile_Clear()) {
			expanded = Place_Cliff(cptr, 0);
		}
		cptr = Map.Iterate();
	}

	ObjectClass * obj = Map.PendingObjectPtr;
	Map.PendingObject = NULL;
	if (obj) {
		delete obj;
		Map.PendingObjectPtr = NULL;
	}

	for (int i = 0; i < s; i++) {
		MapRegionClass::Get_Cell_Data(i).InFillReach = true;
	}

	if (is_temp_data) {
		delete [] RMGCellData;
		RMGCellData = NULL;
	}

	return(expanded);
}


/*
 * These are the four regions of an aligned 2x2 block of threat regions, expressed as cell
 * offsets from the upper left cell of the block. They run left to right and top to bottom, so
 * the low bit of the index is the eastern half of the block and the high bit is the southern.
 */
Cell RegionQuadrantOffsets[4] = {
	Cell(0, 0), Cell(REGION_WIDTH, 0), Cell(0, REGION_HEIGHT), Cell(REGION_WIDTH, REGION_HEIGHT),
};


/*
 * These are the quadrants of a region block that lie along the side of it facing a given
 * direction, indexed by that facing. A step from one block to another is priced from the
 * quadrants of each that face the other, so the first table is read with the facing that points
 * at the destination and the second with the facing that points back. A diagonal facing names
 * its single corner quadrant twice.
 */
int FromRegionQuadrants[FACING_COUNT][2] = {
	{ 0, 1 }, { 1, 1 }, { 1, 3 }, { 3, 3 }, { 2, 3 }, { 2, 2 }, { 0, 2 }, { 0, 0 },
};
int ToRegionQuadrants[FACING_COUNT][2] = {
	{ 2, 3 }, { 2, 2 }, { 0, 2 }, { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 3 }, { 3, 3 },
};


/// <summary>
/// Fetches the threat a house faces moving through a subzone.
/// The hierarchical path search calls this routine to price a step of the route it is
/// building, so that a path can be steered clear of ground the house has reason to fear. Where
/// the two subzones sit in different threat regions, the value is taken from the corner of
/// each region that faces the other.
/// </summary>
/// <param name="house">The house whose view of the danger is wanted.</param>
/// <param name="level">The subzone level being priced; this also chooses between the single
/// subzone and subzone to subzone forms.</param>
/// <param name="from_subzone">The subzone being moved out of.</param>
/// <param name="to_subzone">The subzone being moved into.</param>
/// <returns>Returns with the threat value. Levels that carry no threat rating return
/// zero.</returns>
int MapClass::Region_Threat(HouseClass * house, int level, int from_subzone, int to_subzone)
{
	switch (level) {
		default:
			break;

		case 0:
			return(0);

		case 1:
			return(house->Regions[Map.SubzoneTracking[level][to_subzone].ThreatRegion].Threat_Value());

		case 2: {
			int from_region = Map.SubzoneTracking[level][from_subzone].ThreatRegion;
			int index = from_region - 1;
			int col = index % MAP_REGION_WIDTH;
			int row = (from_region - col) / MAP_REGION_HEIGHT;
			int from_x = REGION_WIDTH * col;
			int from_y = REGION_HEIGHT * (row - 1);

			short from_block_x = from_x - (from_x / REGION_WIDTH % 2 != 0);
			short from_block_y = from_y - (from_y / REGION_HEIGHT % 2 != 0);

			int to_region = Map.SubzoneTracking[level][to_subzone].ThreatRegion;
			index = to_region - 1;
			col = index % MAP_REGION_WIDTH;
			row = (to_region - col) / MAP_REGION_HEIGHT;
			int to_x = REGION_WIDTH * col;
			int to_y = REGION_HEIGHT * (row - 1);

			short to_block_x = to_x - (to_x / REGION_WIDTH % 2 != 0);
			short to_block_y = to_y - (to_y / REGION_HEIGHT % 2 != 0);

			if (from_block_x == to_block_x && from_block_y == to_block_y) {
				return((house->Regions[Cell_Region(Cell(from_x, from_y))].Threat_Value() + house->Regions[Cell_Region(Cell(to_x, to_y))].Threat_Value()) >> 1);
			}

			int facing;
			if (to_block_x > from_block_x) {
				if (to_block_y > from_block_y) {
					facing = FACING_SE;
				} else if (to_block_y >= from_block_y) {
					facing = FACING_E;
				} else {
					facing = FACING_NE;
				}
			} else if (to_block_x < from_block_x) {
				if (to_block_y > from_block_y) {
					facing = FACING_SW;
				} else if (to_block_y >= from_block_y) {
					facing = FACING_W;
				} else {
					facing = FACING_NW;
				}
			} else if (to_block_y > from_block_y) {
				facing = FACING_S;
			} else {
				facing = FACING_N;
			}

			int from_threat1 = house->Regions[Cell_Region(RegionQuadrantOffsets[FromRegionQuadrants[facing][0]] + Cell(from_block_x, from_block_y))].Threat_Value();
			int from_threat2 = house->Regions[Cell_Region(RegionQuadrantOffsets[FromRegionQuadrants[facing][1]] + Cell(from_block_x, from_block_y))].Threat_Value();
			int from_threat = from_threat1;
			if (from_threat1 >= from_threat2) {
				from_threat = from_threat2;
			}

			int to_threat1 = house->Regions[Cell_Region(RegionQuadrantOffsets[ToRegionQuadrants[facing][0]] + Cell(to_block_x, to_block_y))].Threat_Value();
			int to_threat2 = house->Regions[Cell_Region(RegionQuadrantOffsets[ToRegionQuadrants[facing][1]] + Cell(to_block_x, to_block_y))].Threat_Value();
			if (to_threat1 >= to_threat2) {
				to_threat1 = to_threat2;
			}

			return((to_threat1 + from_threat) >> 1);
		}
	}
	return(0);
}


/// <summary>
/// Scans the square block of cells within the given radius around a cell for any occupier.
/// Returns as soon as an occupied cell is found.
/// </summary>
/// <param name="cell">Center cell of the search block.</param>
/// <param name="radius">Half-width of the square search block in cells.</param>
/// <returns>True if any cell in the block has an occupier, false otherwise.</returns>
bool MapClass::Is_Something_Nearby(Cell const & cell, int radius)
{
	for (int y = cell.Y - radius; y <= cell.Y + radius; y++) {
		for (int x = cell.X - radius; x <= cell.X + radius; x++) {
			CellClass * cptr = &Map[Cell(x,y)];
			if (cptr->Cell_Occupier() != NULL) {
				return(true);
			}
		}
	}
	return(false);
}


/// <summary>
/// Determines if a coordinate is still under the shroud.
/// Which cell a coordinate appears over depends on how high it is, so the height is folded
/// into the lookup before the shroud is consulted.
/// </summary>
/// <returns>bool; Is the coordinate still shrouded?</returns>
bool MapClass::Is_Shrouded(Coord const & coord)
{
	int level_height = coord.Z / LEVEL_LEPTON_H;
	if ((level_height & 1) != 0) {
		int offset = level_height / 2 + 1;
		Cell cell = coord.As_Cell();
		CellClass * cptr = &Map[Cell(cell.X - offset, cell.Y - offset)];
		if (cptr->IsMapped) {
			return(false);
		}
		cptr = &cptr->Adjacent_Cell(FACING_SE);
		if (!cptr->IsMapped) {
			return(true);
		}
	} else {
		int offset = level_height / 2;
		Cell cell = coord.As_Cell();
		CellClass * cptr = &Map[Cell(cell.X - offset, cell.Y - offset)];
		if (!cptr->IsMapped) {
			return(true);
		}
	}
	return(false);
}


/// <summary>
/// Determines if a coordinate is hidden under the fog of war.
/// Which cell a coordinate appears over depends on how high it is, so the height is folded
/// into the lookup before the fog is consulted. A player given the whole map, an observer or
/// a defeated player outside coach mode, sees through the fog, and nothing is reported as
/// hidden from them.
/// </summary>
/// <returns>bool; Is the coordinate under the fog?</returns>
bool MapClass::Is_Fogged(Coord const & coord)
{
	CellClass * cptr;

	if (!Session.ObiWan) {
		int level_height = coord.Z / LEVEL_LEPTON_H;
		if ((level_height & 1) != 0) {
			int offset = level_height / 2 + 1;
			Cell cell = coord.As_Cell();
			cptr = &Map[Cell(cell.X - offset, cell.Y - offset)];
			if (cptr->IsFogMapped) {
				return(false);
			}
			cptr = &cptr->Adjacent_Cell(FACING_SE);
		} else {
			int offset = level_height / 2;
			Cell cell = coord.As_Cell();
			cptr = &Map[Cell(cell.X - offset, cell.Y - offset)];
		}
		if (!cptr->IsFogMapped) {
			return(true);
		}
	}
	return(false);
}


/// <summary>
/// Reveals the objects that stand where this cell is drawn.
/// Raising the ground lifts a cell up the screen, so cells at several different heights can
/// end up sharing one spot in the view. This routine follows that line and reveals whatever it
/// finds standing on ground high enough to be seen there.
/// </summary>
/// <param name="house">The house the discovered objects are revealed to.</param>
/// <param name="onradar">Should the cells be refreshed on the radar as well?</param>
void MapClass::Reveal_Nearby_Technos(CellClass * cptr, HouseClass * house, bool onradar)
{
	Cell cell = cptr->CellID;

	for (int i = 1; i < 15; i += 2) {
		CellClass * cellptr = &Map[cell];
		int cell_height = cellptr->Height;
		if (cell_height >= i - 2 && cell_height <= i) {
			if (onradar) {
				Map.Radar_Cell(cellptr->CellID);
			}
			TechnoClass * t = cellptr->Cell_Techno();
			if (t != NULL) {
				t->Revealed(house);
			}
		}
		cell += Cell(1,1);
	}
}


/// <summary>
/// Brings the fog of war up.
/// This routine draws the fog over every cell that is not already under it, so that the map
/// starts out hidden. Cells the player has since uncovered are left as they are.
/// </summary>
void MapClass::Init_Fog_System(void)
{
	Reset_Iterator();
	CellClass * cellptr = Iterate();
	while (cellptr != NULL) {
		if (!cellptr->IsFogMapped) {
			cellptr->Fog_Cell();
		}
		cellptr = Iterate();
	}
}


/// <summary>
/// Shuts the fog of war down.
/// This routine lifts the fog from every cell of the map, which is what leaves the terrain
/// plainly visible once the fog is no longer wanted.
/// </summary>
void MapClass::Deinit_Fog_System(void)
{
	Reset_Iterator();
	CellClass * cellptr = Iterate();
	while (cellptr != NULL) {
		cellptr->Unfog_Cell();
		cellptr = Iterate();
	}
}


/// <summary>
/// Determines if a block of cells is free for a house to use.
/// This routine checks that the house is not already occupying any part of the area and that
/// the whole of it lies on the playable map.
/// </summary>
/// <param name="house">The house index whose occupancy is being tested for.</param>
/// <returns>bool; Is the area clear of this house and on the map?</returns>
bool MapClass::Is_Area_Available(Rect const & rect, int house)
{
	int mask = 1 << house;
	for (int x = rect.X; x < rect.X + rect.Width; x++) {
		for (int y = rect.Y; y < rect.Y + rect.Height; y++) {
			CellClass * cptr = &Map[Cell(x,y)];
			if (cptr->OccupiedBy & mask) {
				return(false);
			}
		}
	}
	return(In_Local_Radar(rect));
}


/// <summary>
/// Collects every cell within the given rectangle into a list and
/// forwards it to Recalc_Cells_In_List for attribute/subzone recalculation.
/// </summary>
/// <param name="rect">Rectangular cell region (in cell coordinates) to recalculate.</param>
void MapClass::Recalc_Cells_In_Rect(Rect const & rect)
{
	DynamicVectorClass<Cell> cells;
	for (int x = rect.X; x < rect.X + rect.Width; x++) {
		for (int y = rect.Y; y < rect.Y + rect.Height; y++) {
			cells.Add(Cell(x,y));
		}
	}
	Recalc_Cells_In_List(cells);
}


/// <summary>
/// Recalculates cell attributes for each on-radar cell in the list and clears its subzone ID,
/// then in a second pass updates the subzones for any cell whose subzone ID was cleared.
/// </summary>
/// <param name="vec">List of cells to recalculate (processed back to front).</param>
void MapClass::Recalc_Cells_In_List(DynamicVectorClass<Cell> const & vec)
{
	int i;
	for (i = vec.Count() - 1; i >= 0; i--) {
		if (In_Local_Radar(vec[i])) {
			CellSubzones[Get_Cell_Zone_Index(vec[i])].SubzoneID[0] = 0;
			CellClass * cptr = &Map[vec[i]];
			cptr->Recalc_Attributes();
		}
	}
	for (i = vec.Count() - 1; i >= 0; i--) {
		Cell cell = vec[i];
		if (In_Local_Radar(cell)) {
			if (CellSubzones[Get_Cell_Zone_Index(cell)].SubzoneID[0] == 0) {
				Update_Cell_Subzones(cell);
			}

		}
	}
}


/// <summary>
/// Projects a cell onto the nearest edge of the map.
/// This routine is used to work out where something enters or leaves the playable area --
/// aircraft flying off, reinforcements arriving -- by carrying the cell out to whichever
/// border of the view it already lies closest to.
/// </summary>
/// <param name="inset">Should the result sit one cell inside the border rather than on it?</param>
/// <returns>Returns with the cell on the nearest edge of the local view.</returns>
Cell MapClass::Closest_Edge_Cell(Cell const & cell, bool inset)
{
	/// Cell_To_LocalRect_Point??
	Point2D point = Cell_To_PlayRect_Point(Point2D(cell.X, cell.Y));
	point = point - LocalRect.Top_Left();

	Point2D center = Point2D(LocalRect.Width / 2, LocalRect.Height / 2);

	int x = point.X - center.X;
	int y = point.Y - center.Y;

	if (abs(x) < abs(y)) {
		if (point.X < center.X) {
			point.X = inset ? 1 : 0;
		} else {
			point.X = inset ? LocalRect.Width - 1 : LocalRect.Width - 0;
		}
	} else {
		if (point.Y < center.Y) {
			point.Y = inset ? 1 : 0;
		} else {
			point.Y = inset ? LocalRect.Height - 1 : LocalRect.Height - 0;
		}
	}

	point = LocalRect_To_Cell_Point(point);
	return(Cell(point.X,point.Y));
}


/// <summary>
/// Marks the cells left behind by collapsed bridge spans.
/// This routine walks every bridge connection that is no longer passable and flags the ground
/// its span used to cover, recording which way the bridge ran across it. The repair code needs
/// those flags later to know where a destroyed bridge is allowed to be rebuilt.
/// </summary>
void MapClass::Set_WasUnderBridge_Flags(void)
{
	for (int index = ZoneConnections.Count() - 1; index >= 0; index--) {
		ZoneConnectionClass * con = &ZoneConnections[index];

		if (con->Type == 0 && !con->IsPassable) {

			/*
			 * A vertical connection (the bridge runs north/south) steps along the
			 * Y axis. The cells that are flagged run perpendicular along the X axis.
			 */
			if (con->From.X == con->To.X) {
				FacingType facing = con->From.Y < con->To.Y ? FACING_S : FACING_N;
				Cell cell = Adjacent_Cell(con->From, facing);
				for (; cell != con->To; cell = Adjacent_Cell(cell, facing)) {
					if (!Map[cell].IsUnderBridge) {
						for (int offset = -2; offset <= 1; offset++) {
							CellClass & cptr = Map[cell + Cell(offset, 0)];
							cptr.IsBridgeEastWest = false;
							cptr.WasUnderBridge = true;
						}
					}
				}

			/*
			 * A horizontal connection (the bridge runs east/west) steps along the
			 * X axis. The cells that are flagged run perpendicular along the Y axis.
			 */
			} else {
				FacingType facing = con->From.X < con->To.X ? FACING_E : FACING_W;
				Cell cell = Adjacent_Cell(con->From, facing);
				for (; cell != con->To; cell = Adjacent_Cell(cell, facing)) {
					if (!Map[cell].IsUnderBridge) {
						for (int offset = -2; offset <= 1; offset++) {
							CellClass & cptr = Map[cell + Cell(0, offset)];
							cptr.IsBridgeEastWest = true;
							cptr.WasUnderBridge = true;
						}
					}
				}
			}
		}
	}
}


/// <summary>
/// Clips a cell to lie within the visible part of the map.
/// The playable area is a diamond on screen, so a cell outside it is slid back along the
/// diagonals until it falls inside. Terrain height is taken into account, since raised ground
/// near the top edge is drawn further up than flat ground is.
/// </summary>
/// <returns>Returns with the nearest cell that lies inside the local radar.</returns>
Cell MapClass::Clip_To_Map(Cell const & cell)
{
	int x = cell.X;
	int y = cell.Y;

	if (x - y >= 2 * (LocalRect.X + LocalRect.Width) - PlayRect.Width) {
		int tmp = (x + PlayRect.Width + 2 * (1 - LocalRect.X - LocalRect.Width) - y);
		x -= tmp / 2;
		y += tmp / 2;
	} else if (y - x >= PlayRect.Width - 2 * LocalRect.X) {
		int tmp = (y + 2 * LocalRect.X + 2 - PlayRect.Width - x);
		y -= tmp / 2;
		x += tmp / 2;
	}

	CellClass *cptr = &Map[Cell(x,y)];
	int cell_height = cptr->Height;
	int ramp = cptr->Ramp;

	if (ramp != 0) {
		if (x + y < cell_height + PlayRect.Width + 2 * LocalRect.Y + 4) {
			cell_height++;
		}
	}

	if (x + y <= cell_height + PlayRect.Width + 2 * LocalRect.Y) {
		do {
			x++;
			y++;
		} while (!In_Local_Radar(Cell(x,y)));
	} else if (x + y > cell_height + PlayRect.Width + 2 * (LocalRect.Y + LocalRect.Height) + 2) {
		do {
			x--;
			y--;
		} while (!In_Local_Radar(Cell(x,y)));
	}

	return(Cell(x,y));
}


/// <summary>
/// Fetches the part of a multi-cell tile nearest an object.
/// A large isometric tile covers several cells and not every one of them is actually filled
/// in. This routine picks the filled, on-map cell of the tile lying closest to the object, so
/// that an order aimed anywhere on the tile lands somewhere it can be met.
/// </summary>
/// <param name="tarcell">The cell that was ordered; its tile is the one examined.</param>
/// <param name="objcell">The cell that closeness is measured from.</param>
/// <returns>Returns with the closest usable cell of the tile. A cell carrying no tile is
/// returned unchanged, and if the tile offers nothing usable, CELL_NONE is returned.</returns>
Cell MapClass::Closest_Passable_Cell(Cell const & tarcell, Cell const & objcell)
{
	Cell best_cell = CELL_NONE;
	double best_dist = 10000.0;
	CellClass *cptr = &Map[tarcell];

	if (cptr->ITType < ISOTILE_FIRST) {
		return(tarcell);
	}

	IsometricTileTypeClass *iptr = IsometricTileTypes[cptr->ITType];
	IsoTileSet *set = (IsoTileSet*)iptr->Get_Image_Data();

	int w = iptr->Width;
	int h = iptr->Height;
	int s = cptr->SubTile;

	Cell d = tarcell - Cell(s % w, s / w);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			Cell cell = d + Cell(x, y);
			if (set->Fetch_Record_Pointer_Unsafe(x + y * iptr->Width) != NULL && In_Local_Radar(cell)) {
				int xdiff = cell.X - objcell.X;
				int ydiff = cell.Y - objcell.Y;
				double dist = std::sqrt(double(xdiff * xdiff + ydiff * ydiff));
				if (dist < best_dist) {
					best_dist = dist;
					best_cell = cell;
				}
			}
		}
	}

	return(best_cell);
}


/// <summary>
/// Brings the cells that have lost their ice up to date.
/// Ice is broken a cell at a time, but the terrain and zone bookkeeping it disturbs is far
/// cheaper to redo in one go. This routine is called once the damage for the frame has been
/// tallied, and it clears the pending list as it goes.
/// </summary>
void MapClass::Recalc_Ice_Cells(void)
{
	for (int i = DirtyIceCells.Count() - 1; i >= 0; i--) {
		Map[DirtyIceCells[i]].Recalc_Attributes();
	}
	Zone_Reset();
	Recalc_Cells_In_List(DirtyIceCells);
	DirtyIceCells.Clear();
}


/// <summary>
/// Damages whatever kind of bridge sits at this cell.
/// This routine works out which of the low, high and train bridge handlers applies, lets it do
/// the work, and then collapses the cells that handler queued up. Use this rather than the
/// individual handlers -- it is the door the weapon and trigger code comes in through.
/// </summary>
/// <param name="cell">The cell of the bridge, or of a cell beneath it, that was hit.</param>
/// <returns>bool; Was a bridge span destroyed?</returns>
bool MapClass::Damage_Bridge(Cell const & cell)
{
	bool result = false;
	PendingBridgeCells.Clear();
	CellClass *cptr = &Map[cell];

	if (cptr->Overlay >= OVERLAY_LOWBRIDGE_01 && cptr->Overlay <= OVERLAY_LOWBRIDGE_26) {
		result = Damage_Low_Bridge(cell);
	} else {
		IsometricTileType ittype = IsometricTileType(cptr->ITType - IsometricTileTypeClass::BridgeSet + 1);
		CellClass * bcptr = cptr->Get_Bridge_Deck_CellClass();

		if (
			(bcptr != NULL && (bcptr->Overlay == OVERLAY_BRIDGE1 || bcptr->Overlay == OVERLAY_BRIDGE2)) ||
			(
				ittype == IsometricTileTypeClass::BridgeMiddle1 + 0 ||
				ittype == IsometricTileTypeClass::BridgeMiddle1 + 1 ||
				ittype == IsometricTileTypeClass::BridgeMiddle1 + 2 ||
				ittype == IsometricTileTypeClass::BridgeMiddle1 + 3 ||
				ittype == IsometricTileTypeClass::BridgeMiddle2 + 0 ||
				ittype == IsometricTileTypeClass::BridgeMiddle2 + 1 ||
				ittype == IsometricTileTypeClass::BridgeMiddle2 + 2 ||
				ittype == IsometricTileTypeClass::BridgeMiddle2 + 3
			)
		) {
			result = Damage_High_Bridge(cell);
		} else {
			ittype = IsometricTileType(cptr->ITType - IsometricTileTypeClass::TrainBridgeSet + 1);
			if (
				(bcptr != NULL && (bcptr->Overlay == OVERLAY_RAIL_BRIDGE1 || bcptr->Overlay == OVERLAY_RAIL_BRIDGE2)) ||
				(
					ittype == IsometricTileTypeClass::BridgeMiddle1 + 0 ||
					ittype == IsometricTileTypeClass::BridgeMiddle1 + 1 ||
					ittype == IsometricTileTypeClass::BridgeMiddle1 + 2 ||
					ittype == IsometricTileTypeClass::BridgeMiddle1 + 3 ||
					ittype == IsometricTileTypeClass::BridgeMiddle2 + 0 ||
					ittype == IsometricTileTypeClass::BridgeMiddle2 + 1 ||
					ittype == IsometricTileTypeClass::BridgeMiddle2 + 2 ||
					ittype == IsometricTileTypeClass::BridgeMiddle2 + 3
				)
			) {
				result = Damage_Train_Bridge(cell);
			}
		}
	}

	for (int i = 0; i < PendingBridgeCells.Count(); i++) {
		Map[PendingBridgeCells[i]].On_Bridge_Collapse();
	}

	PendingBridgeCells.Clear();
	return(result);
}


/// <summary>
/// Determines if there is a broken bridge here worth repairing.
/// This routine is used when something tries to repair a bridge. The search is loose enough
/// that only the neighborhood of the damaged span has to be indicated rather than the exact
/// cell, and high, train and low bridges are each recognized.
/// </summary>
/// <param name="cell">The cell the repair attempt was aimed at.</param>
/// <returns>bool; Is there a repairable bridge within reach of this cell?</returns>
bool MapClass::Can_Repair_Bridge(Cell const & cell)
{
	static short _x[] = {
		1, 1, 0, 2, 2, 2, 0, 0,
		0, 0, 0, 2, 2, 2, 2, 2
	};
	static short _y[] = {
		2, 2, 2, 1, 1, 0, 2, 2,
		2, 2, 2, 0, 0, 0, 0, 0
	};
	static FacingType _facings[] = {
		FACING_E,       FACING_E,    FACING_W,    FACING_S,    FACING_S,    FACING_N, FACING_NONE, FACING_NONE,
		FACING_NONE, FACING_NONE, FACING_NONE, FACING_NONE, FACING_NONE, FACING_NONE, FACING_NONE, FACING_NONE
	};

	bool found = false;
	CellClass * cellptr = NULL;
	int offset = -1;

	for (int y = -2; y < 3; y++) {
		for (int x = -2; x < 3; x++) {

			IsometricTileType ittype = Map[Cell(cell + Cell(x, y))].ITType;

			if (ittype >= IsometricTileTypeClass::TrainBridgeSet && ittype < IsometricTileTypeClass::TrainBridgeSet + TRAIN_BRIDGE_COUNT) {
				found = true;
				cellptr = &Map[Cell(cell + Cell(x, y))];
				offset = ittype - IsometricTileTypeClass::TrainBridgeSet;
			} else if (ittype >= IsometricTileTypeClass::BridgeSet && ittype < IsometricTileTypeClass::BridgeSet + BRIDGE_COUNT) {
				found = true;
				cellptr = &Map[Cell(cell + Cell(x, y))];
				offset = ittype - IsometricTileTypeClass::BridgeSet;
			} else {
				Cell cellnum = cell + Cell(x, y);
				if (Is_Low_Bridge(cellnum)) {
					found = false;
					cellptr = &Map[Cell(cell + Cell(x, y))];
				}
			}
		}
	}

	if (found) {

		int width = IsometricTileTypes[cellptr->ITType]->Width;
		Cell topleft(cellptr->CellID.X - cellptr->SubTile % width, cellptr->CellID.Y - cellptr->SubTile / width);

		Cell entry = topleft + Cell(_x[offset], _y[offset]);
		FacingType dir = _facings[offset];
		Cell search = entry;

		if (dir != FACING_NONE) {
			while (true) {
				int index = Zone_Connection_Index(search, 3, 0);
				if (index < 0) {
					break;
				}
				ZoneConnectionClass * con = &ZoneConnections[index];
				if (!con->IsPassable) {
					return(true);
				}
				if (con->From == entry) {
					entry = con->To;
					search = Adjacent_Cell(entry, dir);
				} else {
					if (con->To != entry) {
						return(true);
					}
					entry = con->From;
					search = Adjacent_Cell(entry, dir);
				}
			}
		}
	} else if (cellptr != NULL) {

		if (cellptr->Overlay >= OVERLAY_LOWBRIDGE_01 && cellptr->Overlay <= OVERLAY_LOWBRIDGE_09 ||
			cellptr->Overlay >= OVERLAY_LOWBRIDGE_19 && cellptr->Overlay <= OVERLAY_LOWBRIDGE_22 ||
			cellptr->Overlay == OVERLAY_LOWBRIDGE_27) {

			for (Cell walk = cellptr->CellID; ; walk.X--) {
				if (!Is_Low_Bridge(walk)) {
					break;
				}
				OverlayType overlay = Map[walk].Overlay;
				if (overlay == OVERLAY_LOWBRIDGE_27 || overlay == OVERLAY_LOWBRIDGE_28) {
					return(true);
				}
			}

			for (Cell scan = cellptr->CellID; ; scan.X++) {
				if (!Is_Low_Bridge(scan)) {
					break;
				}
				OverlayType overlay = Map[scan].Overlay;
				if (overlay == OVERLAY_LOWBRIDGE_27 || overlay == OVERLAY_LOWBRIDGE_28) {
					return(true);
				}
			}

		} else {

			for (Cell scan = cellptr->CellID; ; scan.Y++) {
				if (!Is_Low_Bridge(scan)) {
					break;
				}
				OverlayType overlay = Map[scan].Overlay;
				if (overlay == OVERLAY_LOWBRIDGE_27 || overlay == OVERLAY_LOWBRIDGE_28) {
					return(true);
				}
			}

			for (Cell walk = cellptr->CellID; ; walk.Y--) {
				if (!Is_Low_Bridge(walk)) {
					break;
				}
				OverlayType overlay = Map[walk].Overlay;
				if (overlay == OVERLAY_LOWBRIDGE_27 || overlay == OVERLAY_LOWBRIDGE_28) {
					return(true);
				}
			}
		}
	}

	return(false);
}


/// <summary>
/// Checks that the zone tables agree with the pathfinder.
/// This is a development aid. Every pair of cells the zone tables claim to be mutually
/// reachable is handed to the path search, and any pair the search cannot actually walk is
/// written out to MapCheck.txt. Ground and bridge approaches are tested separately, since a
/// cell beneath a bridge belongs to two zones at once.
/// </summary>
/// <returns>bool; Did every same-zone cell pair prove walkable?</returns>
bool MapClass::Check_Map_Integrity(void)
{
	bool from_ground;
	bool from_bridge;
	bool to_ground;
	bool to_bridge;
	CellClass * from;
	CellClass * to;
	CellClass *bcptr;

	bool passed = true;

	FILE * file = fopen("MapCheck.txt", "w");

	Reset_Iterator();
	from = Iterate();
	while (from != NULL) {
		if (In_Local_Radar(from)) {

			for (MZoneType mzone = MZONE_NORMAL; mzone < MZONE_COUNT; mzone++) {
				if (mzone != MZONE_FLYER && mzone != MZONE_SUBTERANNEAN) {

					from_ground = MZonePassability[mzone][from->Passability] == TRAVERSAL_PASSABLE;

					from_bridge = from->IsUnderBridge
						&& ((from->IsBridgeEastWest && ((bcptr = from->BridgeDeckCell) == NULL || bcptr->CellID.Y - from->CellID.Y != 2))
							|| (!from->IsBridgeEastWest && ((bcptr = from->BridgeDeckCell) == NULL || bcptr->CellID.X - from->CellID.X != 2)));

					if (from_ground || from_bridge) {
						Reset_Local_Iterator();
						to = Local_Iterate();
						while (to != NULL) {

							if ((to->CellID.X + to->CellID.Y > from->CellID.X + from->CellID.Y) || (to->CellID.X + to->CellID.Y == from->CellID.X + from->CellID.Y && to->CellID.X - to->CellID.Y > from->CellID.X - from->CellID.Y)) {
								to_ground = MZonePassability[mzone][to->Passability] == TRAVERSAL_PASSABLE;
								to_bridge = to->IsUnderBridge
									&& ((to->IsBridgeEastWest && ((bcptr = to->BridgeDeckCell) == NULL || bcptr->CellID.Y - to->CellID.Y != 2))
										|| (!to->IsBridgeEastWest && ((bcptr = to->BridgeDeckCell) == NULL || bcptr->CellID.X - to->CellID.X != 2)));

								if (to_ground
									&& from_ground
									&& Is_Same_Cell_Zone(from->CellID, to->CellID, mzone, false, false, false)
									&& Search.Test_Cell_Walk(from->CellID, to->CellID, NULL, false, false, mzone) == 0x7FFFFFFF) {
									passed = false;
									fprintf(file, "Failure: (%d,%d)->(%d,%d) ; (ground->ground) ; MZONE %s\n", from->CellID.X, from->CellID.Y, to->CellID.X, to->CellID.Y, _mzones[mzone]);
								}

								if (to_bridge) {
									if (from_bridge
										&& Is_Same_Cell_Zone(from->CellID, to->CellID, mzone, true, true, false)
										&& Search.Test_Cell_Walk(from->CellID, to->CellID, NULL, true, true, mzone) == 0x7FFFFFFF) {
										passed = false;
										fprintf(file, "Failure: (%d,%d)->(%d,%d) ; (bridge->bridge) ; MZONE %s\n", from->CellID.X, from->CellID.Y, to->CellID.X, to->CellID.Y, _mzones[mzone]);
									}

									if (from_ground
										&& Is_Same_Cell_Zone(from->CellID, to->CellID, mzone, false, true, false)
										&& Search.Test_Cell_Walk(from->CellID, to->CellID, NULL, false, true, mzone) == 0x7FFFFFFF) {
										passed = false;
										fprintf(file, "Failure: (%d,%d)->(%d,%d) ; (ground->bridge) ; MZONE %s\n", from->CellID.X, from->CellID.Y, to->CellID.X, to->CellID.Y, _mzones[mzone]);
									}
								}

								if (to_ground
									&& from_bridge
									&& Is_Same_Cell_Zone(from->CellID, to->CellID, mzone, true, false, false)
									&& Search.Test_Cell_Walk(from->CellID, to->CellID, NULL, true, false, mzone) == 0x7FFFFFFF) {
									passed = false;
									fprintf(file, "Failure: (%d,%d)->(%d,%d) ; (bridge->ground) ; MZONE %s\n", from->CellID.X, from->CellID.Y, to->CellID.X, to->CellID.Y, _mzones[mzone]);
								}
							}

							to = Local_Iterate();
						}
					}
				}
			}
		}

		from = Iterate();
	}

	fclose(file);
	return(passed);
}


/// <summary>
/// Finds the first hostile firestorm wall blocking a path.
/// This routine walks the cells the path crosses looking for a firestorm wall belonging to a
/// house that has its firestorm defense switched on. Walls belonging to the house given are
/// passed over, so nobody is ever stopped by their own barrier.
/// </summary>
/// <param name="house">The house traveling the path; its own walls are ignored.</param>
/// <returns>Returns with the coordinate of the first wall that blocks the path. If nothing
/// blocks it, COORD_NONE is returned.</returns>
Coord MapClass::Firestorm_On_Path(Coord const & from, Coord const & to, HouseClass * house)
{
	if (from != to) {

		if ((short)(from.X / CELL_LEPTON_W) == (short)(to.X / CELL_LEPTON_W)) {
			Cell current_cell(from);
			Cell target_cell(to);
			int step = current_cell.Y < target_cell.Y ? 1 : -1;

			while (current_cell != target_cell) {
				BuildingClass * building = Map[current_cell].Cell_Building();
				if (building != NULL && building->Class->IsFirestormWall) {
					HouseClass * owner = building->House;
					if (owner->FirestormDefenseActivated && owner != house) {
						return(Coord(current_cell));
					}
				}

				current_cell.Y += step;
			}

		} else if ((short)(from.Y / CELL_LEPTON_H) == (short)(to.Y / CELL_LEPTON_H)) {
			Cell current_cell(from);
			Cell target_cell(to);
			int step = current_cell.X < target_cell.X ? 1 : -1;

			while (current_cell != target_cell) {
				BuildingClass * building = Map[current_cell].Cell_Building();
				if (building != NULL && building->Class->IsFirestormWall) {
					HouseClass * owner = building->House;
					if (owner->FirestormDefenseActivated && owner != house) {
						return(Coord(current_cell));
					}
				}

				current_cell.X += step;
			}

		} else {
			int x_step = from.X < to.X ? CELL_LEPTON_W : -CELL_LEPTON_W;
			int y_step = from.Y < to.Y ? CELL_LEPTON_H : -CELL_LEPTON_H;
			int current_x = x_step < 0 ? from.X - from.X % CELL_LEPTON_W : from.X - from.X % CELL_LEPTON_W + CELL_LEPTON_W;
			int current_y = y_step < 0 ? from.Y - from.Y % CELL_LEPTON_H : from.Y - from.Y % CELL_LEPTON_H + CELL_LEPTON_H;
			int delta_x = to.X - from.X;
			double x_progress = (double)(current_x - from.X) / (double)delta_x;
			int delta_y = to.Y - from.Y;
			double y_progress = (double)(current_y - from.Y) / (double)delta_y;
			double inv_delta_x = 1.0 / (double)delta_x;
			double inv_delta_y = 1.0 / (double)delta_y;

			while ((x_progress <= 1.0 || y_progress <= 1.0) && x_progress >= 0.0 && y_progress >= 0.0) {
				BuildingClass * building = Map[Coord(current_x, current_y)].Cell_Building();
				if (building != NULL && building->Class->IsFirestormWall) {
					HouseClass * owner = building->House;
					if (owner->FirestormDefenseActivated && owner != house) {
						return(Coord(current_x, current_y, 0));
					}
				}

				if (x_progress < y_progress) {
					current_x += x_step;
					x_progress = (double)(current_x - from.X) * inv_delta_x;
				} else {
					current_y += y_step;
					y_progress = (double)(current_y - from.Y) * inv_delta_y;
				}
			}
		}
	}

	return(COORD_NONE);
}


/// <summary>
/// Fills the gap between a new firestorm wall and a nearby friendly one.
/// This routine is used when a firestorm wall section is built, so that it joins up with any
/// existing stretch of the same house's wall instead of leaving a hole in the barrier. Each
/// cardinal direction is considered in turn, and a gap is closed only when every cell between
/// the two ends is clear to build on.
/// </summary>
/// <param name="cell">The cell the firestorm wall was placed at.</param>
/// <param name="owner">The house whose wall this is; only its own sections are joined to.</param>
/// <param name="type">The firestorm wall type to build the connecting sections from.</param>
void MapClass::Place_Firestorm_Wall(Cell const & cell, HouseClass * owner, BuildingTypeClass * type)
{
	for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir = FacingType(dir + FACING_90)) {

		Cell current_cell = cell;
		int radius = type->ThreatRange >> 8;

		for (int distance = 0; distance < radius; distance++) {
			current_cell = (Cell)Adjacent_Cell(current_cell, dir);

			BuildingClass * building = Map[current_cell].Cell_Building();
			if (building != NULL && building->Class->IsFirestormWall && building->House == owner) {
				if (distance > 0) {
					Cell place_cell = (Cell)Adjacent_Cell(cell, dir);
					for (int i = 0; i < distance; i++) {
						ObjectClass * building = type->Create_One_Of(owner);
						building->Unlimbo(place_cell);
						place_cell = (Cell)Adjacent_Cell(place_cell, dir);
					}
				}
				break;
			}

			if (!Map[current_cell].Is_Clear_To_Build(type->Speed, type, owner)) {
				break;
			}
		}
	}
}


/// <summary>
/// Like Place_Firestorm_Wall but for overlay-based walls: scans the four cardinal directions for
/// an existing matching wall overlay owned by the same house and fills the gap with new segments.
/// Returns immediately if the building type has no associated overlay.
/// </summary>
/// <param name="cell">Origin cell to scan and fill outward from.</param>
/// <param name="owner">House that must own the matching overlay and owns the new segments.</param>
/// <param name="type">Wall building type whose ToOverlay defines the matching overlay.</param>
void MapClass::Place_Wall(Cell const & cell, HouseClass * owner, BuildingTypeClass * type)
{
	OverlayTypeClass const * overtype = type->ToOverlay;
	if (overtype == NULL) {
		return;
	}

	for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir = FacingType(dir + FACING_90)) {

		Cell current_cell = cell;
		int radius = type->ThreatRange >> 8;

		for (int distance = 0; distance < radius; distance++) {
			current_cell = (Cell)Adjacent_Cell(current_cell, dir);

			OverlayType overlay = Map[current_cell].Overlay;
			if (overlay != OVERLAY_NONE && overtype->HeapID == overlay && owner->HeapID == Map[current_cell].Owner) {
				if (distance > 0) {
					Cell place_cell = (Cell)Adjacent_Cell(cell, dir);
					for (int i = 0; i < distance; i++) {
						ObjectClass * building = type->Create_One_Of(owner);
						building->Unlimbo(place_cell);
						place_cell = (Cell)Adjacent_Cell((Cell)place_cell, dir);
					}
				}
				break;
			}

			if (!Map[current_cell].Is_Clear_To_Build(type->Speed, type, owner)) {
				break;
			}
		}
	}
}
