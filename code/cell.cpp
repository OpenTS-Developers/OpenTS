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

/* $Header: /CounterStrike/CELL.CPP 4     3/14/97 1:15p Joe_b $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CELL.CPP                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 29, 1994                                               *
 *                                                                                             *
 *                  Last Update : October 6, 1996 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CellClass::Adjacent_Cell -- Determines the adjacent cell according to facing.             *
 *   CellClass::Adjust_Threat -- Allows adjustment of threat at cell level                     *
 *   CellClass::Can_Tiberium_Germinate -- Determines if Tiberium can begin growth in the cell. *
 *   CellClass::Can_Tiberium_Grow -- Determines if Tiberium can grow in this cell.             *
 *   CellClass::Can_Tiberium_Spread -- Determines if Tiberium can spread from this cell.       *
 *   CellClass::CellClass -- Constructor for cell objects.                                     *
 *   CellClass::Cell_Building -- Return with building at specified cell.                       *
 *   CellClass::Cell_Color   -- Determine what radar color to use for this cell.               *
 *   CellClass::Cell_Coord -- Returns the coordinate of this cell.                             *
 *   CellClass::Cell_Find_Object -- Returns ptr to RTTI type occupying cell                    *
 *   CellClass::Cell_Infantry -- Returns with pointer of first infantry unit.                  *
 *   CellClass::Cell_Object -- Returns with clickable object in cell.                          *
 *   CellClass::Cell_Techno -- Return with the unit/building at specified cell.                *
 *   CellClass::Cell_Terrain -- Determines terrain object in cell.                             *
 *   CellClass::Cell_Unit -- Returns with pointer to unit occupying cell.                      *
 *   CellClass::Cell_Vessel -- Returns with pointer to a vessel located in the cell.           *
 *   CellClass::Clear_Icon -- Calculates what the clear icon number should be.                 *
 *   CellClass::Closest_Free_Spot -- returns free spot closest to given coord                  *
 *   CellClass::Concrete_Calc -- Calculates the concrete icon to use for the cell.             *
 *   CellClass::Draw_It -- Draws the cell imagery at the location specified.                   *
 *   CellClass::Flag_Place -- Places a house flag down on the cell.                            *
 *   CellClass::Flag_Remove -- Removes the house flag from the cell.                           *
 *   CellClass::Goodie_Check -- Performs crate discovery logic.                                *
 *   CellClass::Grow_Tiberium -- Grows the tiberium in the cell.                               *
 *   CellClass::Incoming -- Causes objects in cell to "run for cover".                         *
 *   CellClass::Is_Bridge_Here -- Checks to see if this is a bridge occupied cell.             *
 *   CellClass::Is_Clear_To_Build -- Determines if cell can be built upon.                     *
 *   CellClass::Is_Clear_To_Move -- Determines if the cell is generally clear for travel       *
 *   CellClass::Occupy_Down -- Flag occupation of specified cell.                              *
 *   CellClass::Occupy_Up -- Removes occupation flag from the specified cell.                  *
 *   CellClass::Overlap_Down -- This routine is used to mark a cell as being spilled over (over*
 *   CellClass::Overlap_Unit -- Marks cell as being overlapped by unit.                        *
 *   CellClass::Overlap_Up -- Removes overlap flag for the cell.                               *
 *   CellClass::Read -- Reads a particular cell value from a save game file.                   *
 *   CellClass::Recalc_Attributes -- Recalculates the ground type attributes for the cell.     *
 *   CellClass::Redraw_Objects -- Redraws all objects overlapping this cell.                   *
 *   CellClass::Reduce_Tiberium -- Reduces the tiberium in the cell by the amount specified.   *
 *   CellClass::Reduce_Wall -- Damages a wall, if damage is high enough.                       *
 *   CellClass::Reserve_Cell -- Marks a cell as being occupied by the specified unit ID.       *
 *   CellClass::Shimmer -- Causes all objects in the cell to shimmer.                          *
 *   CellClass::Spot_Index -- returns cell sub-coord index for given COORDINATE                *
 *   CellClass::Spread_Tiberium -- Spread Tiberium from this cell to an adjacent cell.         *
 *   CellClass::Tiberium_Adjust -- Adjust the look of the Tiberium for smooth.                 *
 *   CellClass::Wall_Update -- Updates the imagery for wall objects in cell.                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "cell.h"

#include "_alpha.h"
#include "_bench.h"
#include "_convert.h"
#include "_logic.h"
#include "_mixfile.h"
#include "_rtti.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "abuffer.h"
#include "anim.h"
#include "astar.h"
#include "bench.h"
#include "building.h"
#include "builtype.h"
#include "combat.h"
#include "dbgprint.h"
#include "draw.h"
#include "dsurface.h"
#include "fog.h"
#include "globals.h"
#include "houstype.h"
#include "incdec.h"
#include "infatype.h"
#include "inline.h"
#include "ion.h"
#include "isotype.h"
#include "light.h"
#include "lightcalc.h"
#include "lightcon.h"
#include "logic.h"
#include "mixfile.h"
#include "overlay.h"
#include "overtype.h"
#include "rules.h"
#include "savestream.h"
#include "scheme.h"
#include "session.h"
#include "shapeset.h"
#include "smudtype.h"
#include "sun.h"
#include "super.h"
#include "suprtype.h"
#include "tactical.h"
#include "tag.h"
#include "terrain.h"
#include "tiberium.h"
#include "tracker.h"
#include "tube.h"
#include "unit.h"
#include "unittype.h"
#include "vein.h"
#include "vox.h"
#include "warhead.h"

#include "bench.hh"
#include "ramp.hh"
#include "tube.hh"

#include <algorithm>


/// <summary>
/// Fetches the ground height at a point within this cell.
/// A ramp cell slopes across its own width, so a single height for the whole cell will not
/// serve. Use this routine whenever something must be made to sit on the ground exactly.
/// </summary>
/// <param name="point">The position within the cell to sample.</param>
/// <returns>Returns with the height of the ground at that position, in leptons.</returns>
LEPTON CellClass::Get_Height(Point2D const & point) const
{
	static double _level_height = LEVEL_LEPTON_H;
	static double _level_slope = _level_height / CELL_LEPTON_W;

	static struct {
		double XChange;
		double YChange;
		double Base;
		double Max;
		double Extra;
	} _ramp_control[RAMP_COUNT-1] = {
		{  1.0,	 0.0,	 0.0,							_level_height,					0.0 },						// 1
		{  0.0,	 1.0,	 0.0,							_level_height,					0.0 },						// 2
		{ -1.0,	 0.0,	 _level_height,					_level_height,					0.0 },						// 3
		{  0.0,	-1.0,	 _level_height,					_level_height,					0.0 },						// 4
		{  1.0,	 1.0,	-_level_height,					_level_height,					0.0 },						// 5
		{ -1.0,	 1.0,	 0.0,							_level_height,					0.0 },						// 6
		{ -1.0,	-1.0,	 _level_height,					_level_height,					0.0 },						// 7
		{  1.0,	-1.0,	 0.0,							_level_height,					0.0 },						// 8
		{  1.0,	 1.0,	 0.0,							_level_height,					0.0 },						// 9
		{ -1.0,	 1.0,	 _level_height,					_level_height,					0.0 },						// 10
		{ -1.0,	-1.0,	 _level_height + _level_height, _level_height,					0.0 },						// 11
		{  1.0,	-1.0,	 _level_height,					_level_height,					0.0 },						// 12
		{  1.0,	 1.0,	 0.0,							_level_height + _level_height,	0.0 },						// 13
		{ -1.0,	 1.0,	 _level_height,					_level_height + _level_height,	0.0 },						// 14
		{ -1.0,	-1.0,	 _level_height + _level_height, _level_height + _level_height,	0.0 },						// 15
		{  1.0,	-1.0,	 _level_height,					_level_height + _level_height,	0.0 },						// 16
		{  0.0,	 0.0,	 0.0,							_level_height * 0.5,			_level_height * 0.5	 },		// 17
		{  0.0,	 0.0,	 _level_height,					_level_height * 0.5,			_level_height * -0.5 },		// 18
		{  0.0,	 0.0,	 0.0,							_level_height * 0.5,			_level_height * 0.5	 },		// 19
		{  0.0,	 0.0,	 _level_height,					_level_height * 0.5,			_level_height * -0.5 }		// 20
	};

	LEPTON height = LEPTON(LEVEL_LEPTON_H * Height + 0.5);

	if (Ramp > 0) {
		int ramp = Ramp - 1;
		double rampheight = ((point.X & (CELL_LEPTON_W - 1)) * _ramp_control[ramp].XChange * _level_slope) +
							((point.Y & (CELL_LEPTON_H - 1)) * _ramp_control[ramp].YChange * _level_slope) +
							_ramp_control[ramp].Base +
							_ramp_control[ramp].Extra;

		if (rampheight < 0.0) {
			rampheight = 0.0;
		}
		if (rampheight > _ramp_control[ramp].Max) {
			rampheight = _ramp_control[ramp].Max;
		}

		height += rampheight;
	}

	return(height);
}


/// <summary>
/// Destroys the cell object.
/// This routine throws away the list of objects fogged over this cell and gives up the cell's
/// claim on the shared drawer it was using.
/// </summary>
CellClass::~CellClass(void)
{
	if (FoggedObjects) {
		delete FoggedObjects;
		FoggedObjects = NULL;
	}

	OccupierPtr = 0;

	if (CellID != CELL_NONE && Drawer) {
		Dec_Drawer_Ref_Count(Drawer);
		Drawer = NULL;
	}
}


/***********************************************************************************************
 * CellClass::CellClass -- Constructor for cell objects.                                       *
 *                                                                                             *
 *    A cell object is constructed into an empty state. It contains no specific objects,       *
 *    templates, or overlays.                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/09/1994 JLB : Created.                                                                 *
 *   02/20/1996 JLB : Uses initializer list.                                                   *
 *=============================================================================================*/
CellClass::CellClass(void) :
	BASECLASS(),
	CellID(CELL_NONE),
	IsPlot(false),
	IsCursorHere(false),
	IsMapped(false),
	IsVisible(false),
	IsFogMapped(false),
	IsFogVisible(false),
	IsWaypoint(false),
	IsRadarCursor(false),
	IsFlagged(false),
	IsToShroud(false),
	IsToFog(false),
	IsBridgeDeck(false),
	IsUnderBridge(false),
	IsBridgeTraversable(false),
	WasUnderBridge(false),
	IsBridgeEastWest(false),
	IsBridgeSurface(false),
	IsBridgeDamaged(false),
	IsToGrowIce(false),
	IsToGrowVeins(false),
	IsOvershadowed(false),
	IsAnimAttached(false),
	IsPredictedPath(false),
	IsAffectedByEMP(false),
	IsHorizontalLine(false),
	IsVerticalLine(false),
	IsFogged(false),
	ITType(TILE_NONE),
	Overlay(OVERLAY_NONE),
	OverlayData(0),
	Smudge(SMUDGE_NONE),
	SmudgeData(0),
	Owner(HOUSE_NONE),
	InfType(HOUSE_NONE),
	BridgeInfType(HOUSE_NONE),
	OccupierPtr(NULL),
	BridgeOccupierPtr(NULL),
	Land(LAND_CLEAR),
	FoggedObjects(NULL),
	BridgeDeckCell(NULL),
	UnusedCell(NULL),
	Drawer(NULL),
	Tag(NULL),
	Passability(PASSABLE_LAND),
	LastRedrawFrame(-1),
	LastUnknownDrawFrame(-1),
	LastBridgeDrawFrame(-1),
	LastBridgeDrawRect(Rect(0,0,0,0)),
	Tube(-1),
	ShadowFrame(-2),
	FogFrame(-2),
	CloakedBy(0),
	SensedBy(0),
	OccupiedBy(0),
	Intensity(0x10000),
	Ambient(0),
	Brightness(NORMAL_LIGHT),
	TileBrightness(NORMAL_LIGHT),
	AltBrightness(NORMAL_LIGHT),
	RedTint(NORMAL_LIGHT),
	GreenTint(NORMAL_LIGHT),
	BlueTint(NORMAL_LIGHT),
	LastBridgeDrawRedraws(-1),
	IsIceGrowthAllowed(false),
	SubTile(0),
	Height(0),
	Ramp(0),
	Elevation(0),
	AdjacentObjectCount(0)
{
	Create_ID();
	Flag.Composite = 0;
	BridgeFlag.Composite = 0;
}


/// <summary>
/// Fetches the color this cell shows on the map preview.
/// This routine is used when building the small preview image that is stored with a map. A
/// visible building shows in its owner's color, a bridge in the bridge color, and everything
/// else in the colors of the terrain tile itself.
/// </summary>
/// <param name="terrainonly">Should buildings be ignored so that only the terrain shows?</param>
/// <returns>Returns with the pair of preview pixels for this cell, packed into one value.</returns>
int CellClass::Preview_Cell_Color(unsigned char & unknown, bool terrainonly) const
{
	unknown = 0;
	BuildingClass * obj = Cell_Building();

	if (!terrainonly && obj != NULL && !obj->Class->IsInvisible && !obj->Class->IsInvisibleInGame && obj->Visual_Character() != VISUAL_HIDDEN) {
		ColorScheme * owner_scheme = ColorSchemes[obj->House->Scheme];
		unsigned int color = owner_scheme->Converter->Convert_Pixel(owner_scheme->Color);
		return((color << 16) | color);
	}

	if (IsUnderBridge) {
		RGBClass bridge_color = OverlayTypes[OVERLAY_BRIDGE1]->Get_Radar_Color(0);
		unsigned int color = DSurface::Build_Hicolor_Pixel(bridge_color.Get_Red(), bridge_color.Get_Green(), bridge_color.Get_Blue());
		return((color << 16) | color);
	}

	IsometricTileTypeClass * ittype;

	int icon;
	if (ITType != ISOTILE_NONE) {
		ittype = IsometricTileTypes[ITType];
	} else {
		ittype = IsometricTileTypes[TILE_CLEAR];
	}

	icon = 0;
	if (ITType != ISOTILE_NONE) {
		if (ittype->NumTileTypesInSet > 1) {
			if (ittype->Is_Randomized(SubTile)) {
				icon = IsBridgeDamaged ? 1 : 0;
			} else {
				int numtiles = ittype->NumTileTypesInSet;
				icon = Clear_Icon(ITType, numtiles);
			}
		}
	} else {
		icon = Clear_Icon(TILE_CLEAR, ittype->NumTileTypesInSet);
	}

	IsometricTileTypeClass * next_isotype = ittype->Next_Tile_From_Set(icon);
	unsigned short * tile = next_isotype->Preview_Tile(SubTile, Height);
	return(((tile[0] << 16) | tile[1]));
}


/***********************************************************************************************
 * CellClass::Cell_Color   -- Determine what radar color to use for this cell.                 *
 *                                                                                             *
 *    Use this routine to determine what radar color to render a radar                         *
 *    pixel with. This routine is called many many times to render the                         *
 *    radar map, so it must be fast.                                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the color to display the radar pixel with.                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/01/1994 JLB : Created.                                                                 *
 *   04/30/1994 JLB : Converted to member function.                                            *
 *   05/31/1994 JLB : Takes into account any stealth characteristics of object.                *
 *=============================================================================================*/
void CellClass::Cell_Color(RGBClass & lowcolor, RGBClass & highcolor) const
{
	/*
	 * A terrain object (tree, etc.) overrides the cell color with its own radar color.
	 */
	TerrainClass * tptr = Cell_Terrain();
	if (tptr != NULL) {
		highcolor = lowcolor = tptr->Class->RadarColor;
		return;
	}

	/*
	 * Cells covered by a (high) bridge use the bridge overlay radar color.
	 */
	if (IsUnderBridge) {
		highcolor = lowcolor = OverlayTypes[OVERLAY_BRIDGE1]->Get_Radar_Color(0);
		return;
	}

	IsometricTileTypeClass * itype = (ITType != ISOTILE_NONE) ? IsometricTileTypes[ITType] : IsometricTileTypes[TILE_CLEAR];

	OverlayType overlay = Overlay;
	if (overlay != OVERLAY_NONE && overlay != OVERLAY_LOWBRIDGE_27 && overlay != OVERLAY_LOWBRIDGE_28) {
		OverlayTypeClass * otype = OverlayTypes[overlay];

		/*
		 * Non-tiberium overlays draw with their own radar color.
		 */
		if (!otype->IsTiberium) {
			if (overlay >= OVERLAY_LOWBRIDGE_01 && overlay <= OVERLAY_LOWBRIDGE_26) {
				highcolor = lowcolor = OverlayTypes[overlay]->Get_Radar_Color(1);
			} else {
				highcolor = lowcolor = OverlayTypes[overlay]->Get_Radar_Color(OverlayData);
			}
			return;
		}

		/*
		 * Tiberium overlays that have a cell animation use the animation's palette color.
		 */
		if (otype->CellAnim != NULL) {
			ShapeSet * shape = (ShapeSet *)otype->CellAnim->Get_Image_Data();
			highcolor = lowcolor = shape->Get_Color(OverlayData);
			return;
		}

		/*
		 * Tiberium present in this cell. Pick the overlay shape for the tiberium type
		 * (taking ramp/variety into account) and use its palette color. The high and
		 * low colors are swapped for the second and third tiberium types and veinholes.
		 */
		TiberiumType tib = Tiberium_Type_Here();
		if (tib != TIBERIUM_NONE) {
			TiberiumClass * tiberium = Tiberiums[tib];

			int index;
			if (Ramp != 0) {
				index = tiberium->Variety + tiberium->Overlay->HeapID + tiberium->RampVariety / 4 * (Ramp - 1) + CellID.X * CellID.Y % (tiberium->RampVariety / 4);
			} else {
				index = tiberium->Overlay->HeapID + CellID.X * CellID.Y % tiberium->Variety;
			}

			ShapeSet * shape = (ShapeSet *)OverlayTypes[index]->Get_Image_Data();
			RGBClass color = shape->Get_Color(OverlayData);

			if ((Overlay >= OVERLAY_TIBERIUM2_01 && Overlay <= OVERLAY_TIBERIUM2_12) ||
				(Overlay >= OVERLAY_TIBERIUM3_01 && Overlay <= OVERLAY_TIBERIUM3_12)) {
				color = RGBClass(color.Get_Red(), color.Get_Blue(), color.Get_Green());
			}

			highcolor = lowcolor = color;
			return;
		}
	}

	/*
	 * No object or overlay color applies. Use the iso tile's stored low/high terrain
	 * colors, scaled by theater and interpolated toward a brighter shade by cell height.
	 */
	int tile = 0;
	if (ITType != ISOTILE_NONE) {
		if (itype->NumTileTypesInSet > 1) {
			if (itype->Is_Randomized(SubTile)) {
				tile = IsBridgeDamaged;
			} else {
				tile = Clear_Icon(ITType, itype->NumTileTypesInSet);
			}
		}
	} else {
		tile = Clear_Icon(TILE_CLEAR, itype->NumTileTypesInSet);
	}

	IsometricTileTypeClass * next = itype->Next_Tile_From_Set(tile);
	if (next->ImageData == NULL && next->IsFileLoaded) {
		next->Load_Tile_Image();
	}

	if (((IsoTileSet *)next->ImageData)->Fetch_Record_Pointer_Unsafe(SubTile) != NULL) {
		IsoTileRecord const * record = ((IsoTileSet *)next->ImageData)->Fetch_Record_Pointer_Unsafe(SubTile);
		RGBClass lowest(record->LowColor.Red, record->LowColor.Green, record->LowColor.Blue);
		RGBClass highest(record->HighColor.Red, record->HighColor.Green, record->HighColor.Blue);

		TheaterClass const & theater = TheaterClass::As_Reference(Scen->Theater);

		lowest = RGBClass().Set(lowest, theater.LowRadarBrightness);
		highest = RGBClass().Set(highest, theater.LowRadarBrightness);

		RGBClass lowres;
		RGBClass hires;
		lowres.Set(lowest, theater.HighRadarBrightness);
		hires.Set(highest, theater.HighRadarBrightness);

		lowcolor = RGBClass().Lerp(lowest, lowres, (double)Height / 12.0);
		highcolor = RGBClass().Lerp(highest, hires, (double)Height / 12.0);
	} else {
		lowcolor = RGBClass(0, 0, 0);
		highcolor = RGBClass(0, 0, 0);
	}
}


/***********************************************************************************************
 * CellClass::Cell_Techno -- Return with the unit/building at specified cell.                  *
 *                                                                                             *
 *    Returns an object located in the cell. If there is a                                     *
 *    building present, it returns a pointer to that, otherwise it returns                     *
 *    a pointer to one of the units there. If nothing is present in the                        *
 *    specified cell, then it returns NULL.                                                    *
 *                                                                                             *
 * INPUT:   x,y   -- Coordinate offset (from upper left corner) to use as an aid in selecting  *
 *                   the desired object within the cell.                                       *
 *                                                                                             *
 * OUTPUT:  Returns a pointer to a building or unit located in cell. If                        *
 *          nothing present, just returns NULL.                                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/05/1992 JLB : Created.                                                                 *
 *   04/30/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
TechnoClass * CellClass::Cell_Techno(Point2D const & xy, bool bridge, TechnoClass const * notthis) const
{
	ObjectClass * object;
	Coord		click;			// Coordinate of click relative to cell corner.
	TechnoClass * close = NULL;
	int		distance = 0;	// Recorded closest distance.

	/*
	**	Create a coordinate value that represent the pixel location within the cell. This is
	**	actually the lower significant bits (leptons) of a regular coordinate value.
	*/
	click = Coord(PIXEL_TO_LEPTON(xy.X),PIXEL_TO_LEPTON(xy.Y));

	ObjectClass * occupier = Cell_Occupier(bridge);
	if (occupier != NULL) {
		object = occupier;
		while (object != NULL) {
			if (object->Is_Techno() && object != (ObjectClass *) notthis) {
				Coord coord = object->Center_Coord();
				int dist = Point2D(Coord_Fraction(coord) - click).Length();
				if (!close || dist < distance) {
					close = (TechnoClass *)object;
					distance = dist;
				}
			}
			object = object->Next;
		}
	}
	return(close);
}


/***************************************************************************
 * CellClass::Cell_Find_Object -- Returns ptr to RTTI type occupying cell  *
 *                                                                         *
 * INPUT:      RTTIType the RTTI type we are searching for                 *
 *                                                                         *
 * OUTPUT:      none                                                       *
 *                                                                         *
 * WARNINGS:   none                                                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   03/17/1995 PWG : Created.                                             *
 *   06/12/1995 JLB : Returns object class pointer.                        *
 *=========================================================================*/
ObjectClass * CellClass::Cell_Find_Object(RTTIType rtti, bool bridge) const
{
	assert(rtti != RTTI_NONE);

	if (GameActive) {
		ObjectClass * object = Cell_Occupier(bridge);

		while (object != NULL) {
			if (object->RTTI == rtti) {
				return(object);
			}
			object = object->Next;
		}
	}
	return(NULL);
}


/***********************************************************************************************
 * CellClass::Cell_Building -- Return with building at specified cell.                         *
 *                                                                                             *
 *    Given a cell, determine if there is a building associated                                *
 *    and return with a pointer to this building.                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the building associated with the                         *
 *          cell. If there is no building associated, then NULL is                             *
 *          returned.                                                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/05/1992 JLB : Created.                                                                 *
 *   04/30/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
BuildingClass * CellClass::Cell_Building(void) const
{
	return((BuildingClass *)Cell_Find_Object(RTTI_BUILDING));
}


/***********************************************************************************************
 * CellClass::Cell_Terrain -- Determines terrain object in cell.                               *
 *                                                                                             *
 *    This routine is used to determine the terrain object (if any) that                       *
 *    overlaps this cell.                                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the terrain object that overlaps                         *
 *          this cell. If there is no terrain object present, then NULL                        *
 *          is returned.                                                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
TerrainClass * CellClass::Cell_Terrain(bool bridge) const
{
	return((TerrainClass *)Cell_Find_Object(RTTI_TERRAIN, bridge));
}


/***********************************************************************************************
 * CellClass::Cell_Object -- Returns with clickable object in cell.                            *
 *                                                                                             *
 *    This routine is used to determine which object is to be selected                         *
 *    by a player click upon the cell. Not all objects that overlap the                        *
 *    cell are selectable by the player. This routine sorts out which                          *
 *    is which and returns with the appropriate object pointer.                                *
 *                                                                                             *
 * INPUT:   x,y   -- Coordinate (from upper left corner of cell) to use as a guide when        *
 *                   selecting the object within the cell. This plays a role in those cases    *
 *                   where several objects (such as infantry) exist within the same cell.      *
 *                                                                                             *
 * OUTPUT:  Returns with pointer to the object clickable within the                            *
 *          cell. NULL is returned if there is no clickable object                             *
 *          present.                                                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/13/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectClass * CellClass::Cell_Object(Point2D const & xy, bool bridge) const
{
	ObjectClass * ptr;

	/*
	**	Hack so that aircraft landed on helipads can still be selected if directly
	**	clicked on.
	*/
	ptr = (ObjectClass *)Cell_Find_Object(RTTI_AIRCRAFT);
	if (ptr) {
		return(ptr);
	}

	ptr = Cell_Techno(xy, bridge);
	if (ptr) {
		return(ptr);
	}
	ptr = Cell_Terrain();
	return(ptr);
}


/***********************************************************************************************
 * CellClass::Is_Clear_To_Build -- Determines if cell can be built upon.                       *
 *                                                                                             *
 *    This determines if the cell can become a proper foundation for                           *
 *    building placement.                                                                      *
 *                                                                                             *
 * INPUT:   loco     -- The locomotion of the object trying to consider if this cell is        *
 *                      generally clear. Buildings use the value of SPEED_NONE.                *
 *                                                                                             *
 * OUTPUT:  bool; Is this cell generally clear (usually for building purposes)?                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1994 JLB : Created.                                                                 *
 *   06/25/1996 JLB : Handles different locomotion types.                                      *
 *   10/05/1996 JLB : Checks for crushable walls and crushable object.                         *
 *=============================================================================================*/
bool CellClass::Is_Clear_To_Build(SpeedType loco, BuildingTypeClass * what, HouseClass * who) const
{
	/*
	**	During scenario initialization, passability is always guaranteed.
	*/
	if (ScenarioInit) return(true);

	if (what) {
		if (what->IsLaserFence) {

			if (Cell_Building() != NULL) {
				return(false);
			}

			if (Cell_Terrain() != NULL) {
				return(false);
			}

		} else if ((!what->IsLaserFencePost && !what->IsGate)) {
			if (!what->ToTile) {

				/*
				**	If there is an object there, then don't allow building.
				*/
				if (Cell_Object() != NULL) {
					return(false);
				}

				if ((Flag.Composite & 0x3F) != 0) {
					return(false);
				}

			} else {

				if (ITType >= ISOTILE_FIRST && ITType < IsometricTileTypes.Count() && !IsometricTileTypes[ITType]->IsMorphable) {
					return(false);
				}

				if (Cell_Building() != NULL) {
					return(false);
				}
			}

		} else {

			/*
			 * A laser fence post or gate may only be placed on an empty cell or
			 * on top of an existing laser fence section owned by the same house.
			 */
			ObjectClass * obj = Cell_Object();
			if (obj != NULL) {
				if (obj->RTTI != RTTI_BUILDING) {
					return(false);
				}
				if (!((BuildingClass *)obj)->Class->IsLaserFence || obj->Owner_HouseClass() != who) {
					return(false);
				}
			}

			if ((Flag.Composite & 0x3F) != 0) {
				return(false);
			}
		}
	}

	if (Debug_Map) {
		if (!Map.In_Radar(CellID)) {
			return(false);
		}
	} else {
		if (!Map.In_Local_Radar(this)) {
			return(false);
		}
	}

	/*
	**	Walls are always considered to block the terrain for general passability
	**	purposes. In normal game mode, all overlays are not buildable.
	*/
	if (Overlay != OVERLAY_NONE) {
		HouseClass * owner = NULL;
		if (Owner >= HOUSE_FIRST) {
			owner = Houses[Owner];
		}

		if (Overlay == OVERLAY_BRICK_WALL || Overlay == OVERLAY_SANDBAG_WALL) {
			if (what != NULL) {
				if (what->ToOverlay != NULL && what->ToOverlay->HeapID == Overlay && OverlayData >= 0x10 || what == Rule->WallTower || what == Rule->GDIGateOne || what == Rule->GDIGateTwo) {
					if (owner == who) {
						return(true);
					}
				}
			}
		}

		if (Overlay == OVERLAY_NOD_WALL) {
			if (what != NULL) {
				if ((what->ToOverlay != NULL && what->ToOverlay->HeapID == OVERLAY_NOD_WALL && OverlayData >= 0x10) || what == Rule->NodGateOne || what == Rule->NodGateTwo) {
					if (owner == who) {
						return(true);
					}
				}
			}
		}

		if (what != NULL && what->IsLaserFence) {
			if (Overlay == OVERLAY_VEINS || Tiberium_Type_Here() != TIBERIUM_NONE) {
				if (Is_Bridge_Here() || WasUnderBridge || Ramp != RAMP_NONE) {
					return(false);
				}
				return(true);
			}
		}

		if (!Debug_Map || OverlayTypes[Overlay]->IsWall) {
			return(false);
		}
	}

	/*
	**	Building on certain kinds of terrain is prohibited -- bridges in particular.
	**	If the locomotion type is SPEED_NONE, then this check is presumed to be
	**	for the purposes of building.
	*/
	if (loco == SPEED_NONE) {
		if (Is_Bridge_Here() || WasUnderBridge || Ramp != RAMP_NONE) {
			return(false);
		}

		return(::Ground[Land_Type()].Build);

	} else {

		if (::Ground[Land_Type()].Cost[loco] == 0) {
			return(false);
		}
		return(true);
	}

	return(true);
}


/// <summary>
/// Re-evaluates this cell's LAT (lookahead transition) tile after the terrain has
/// changed. For each LAT family (rough, sand, green, pave, crystal, swamp, blue mold)
/// the four orthogonal neighbors are examined and the tile is replaced with the
/// transition piece that matches the neighbor pattern; ramp tiles are smoothed
/// against the neighboring ramps. The tile art is loaded if the tile was changed.
/// </summary>
/// <returns>true if the cell's tile type was changed by this call.</returns>
bool CellClass::Fixup_LAT(void)
{
	/*
	 * Fetch the ranges of the LAT transition tile sets. A range is invalid
	 * (and its family is skipped) if the theater lacks the tile set.
	 */
	IsometricTileType cleartorough1 = IsometricTileTypeClass::ClearToRoughLat;
	IsometricTileType cleartorough2 = IsometricTileType(cleartorough1 + 15);
	if (cleartorough1 == ISOTILE_INVALID) {
		cleartorough2 = ISOTILE_INVALID;
	}
	IsometricTileType cleartosand1 = IsometricTileTypeClass::ClearToSandLat;
	IsometricTileType cleartosand2 = IsometricTileType(cleartosand1 + 15);
	if (cleartosand1 == ISOTILE_INVALID) {
		cleartosand2 = ISOTILE_INVALID;
	}
	IsometricTileType cleartogreen1 = IsometricTileTypeClass::ClearToGreenLat;
	IsometricTileType cleartogreen2 = IsometricTileType(cleartogreen1 + 15);
	if (cleartogreen1 == ISOTILE_INVALID) {
		cleartogreen2 = ISOTILE_INVALID;
	}
	IsometricTileType cleartopave1 = IsometricTileTypeClass::ClearToPaveLat;
	IsometricTileType cleartopave2 = IsometricTileType(cleartopave1 + 15);
	if (cleartopave1 == ISOTILE_INVALID) {
		cleartopave2 = ISOTILE_INVALID;
	}
	IsometricTileType miscpave2 = IsometricTileTypeClass::MiscPaveTile;
	IsometricTileType miscpave1 = miscpave2;
	miscpave2 = IsometricTileType(miscpave2 + MISC_PAVE_TILE_COUNT - 1);
	if (miscpave1 == ISOTILE_INVALID) {
		miscpave2 = ISOTILE_INVALID;
	}
	IsometricTileType pavedroad2 = IsometricTileTypeClass::PavedRoads;
	IsometricTileType pavedroad1 = pavedroad2;
	pavedroad2 = IsometricTileType(pavedroad2 + 7);
	if (pavedroad1 == ISOTILE_INVALID) {
		pavedroad2 = ISOTILE_INVALID;
	}
	IsometricTileType median2 = IsometricTileTypeClass::Medians;
	IsometricTileType median1 = median2;
	median2 = IsometricTileType(median2 + 13);
	if (median1 == ISOTILE_INVALID) {
		median2 = ISOTILE_INVALID;
	}
	IsometricTileType rampsmooth1 = IsometricTileTypeClass::RampSmooth;
	IsometricTileType rampsmooth2 = IsometricTileType(rampsmooth1 + RAMP_SMOOTH_COUNT - 1);
	if (rampsmooth1 == ISOTILE_INVALID) {
		rampsmooth2 = ISOTILE_INVALID;
	}
	IsometricTileType ramp1 = IsometricTileTypeClass::RampStart;
	IsometricTileType ramp2 = IsometricTileType(ramp1 + 3);
	if (ramp1 == ISOTILE_INVALID) {
		ramp2 = ISOTILE_INVALID;
	}
	IsometricTileType crystal1 = IsometricTileTypeClass::ClearToCrystalLat;
	IsometricTileType crystal2 = IsometricTileType(crystal1 + 15);
	if (crystal1 == ISOTILE_INVALID) {
		crystal2 = ISOTILE_INVALID;
	}

	IsometricTileType crystalcliff1 = IsometricTileTypeClass::CrystalCliff;

	IsometricTileType watertoswamp1 = IsometricTileTypeClass::WaterToSwampLat;
	IsometricTileType watertoswamp2 = IsometricTileType(watertoswamp1 + 15);
	if (watertoswamp1 == ISOTILE_INVALID) {
		watertoswamp2 = ISOTILE_INVALID;
	}
	IsometricTileType swamp1 = IsometricTileTypeClass::SwampTile;
	IsometricTileType swamp2 = IsometricTileType(swamp1 + SWAMP_COUNT);
	if (swamp1 == ISOTILE_INVALID) {
		swamp2 = ISOTILE_INVALID;
	}
	IsometricTileType mold1 = IsometricTileTypeClass::ClearToBlueMoldLat;
	IsometricTileType mold2 = IsometricTileType(mold1 + 15);
	if (mold1 == ISOTILE_INVALID) {
		mold2 = ISOTILE_INVALID;
	}

	IsometricTileType old_ittype = ITType;

	/*
	 * Fix up rough LAT transition tiles.
	 */
	if (old_ittype == IsometricTileTypeClass::RoughTile || old_ittype >= cleartorough1 && old_ittype <= cleartorough2) {
		int offset = 0;
		unsigned int facing = 0;
		for (int i = 0; i < FACING_COUNT / 2; i++) {
			CellClass const & cptr = Adjacent_Cell((FacingType)facing);
			IsometricTileType ittype = cptr.ITType;
			if (ittype != IsometricTileTypeClass::RoughTile && (ittype < cleartorough1 || ittype > cleartorough2)) {
				offset |= 1 << i;
			}
			facing = Facing_Add(facing, FACING_90);
		}
		if (offset != 0) {
			ITType = IsometricTileType(cleartorough1 + offset);
		} else {
			ITType = IsometricTileTypeClass::RoughTile;
		}
	}

	/*
	 * Fix up sand LAT transition tiles.
	 */
	IsometricTileType type;
	if (cleartosand1 != ISOTILE_INVALID) {
		type = ITType;
		if (type == IsometricTileTypeClass::SandTile || type >= cleartosand1 && type <= cleartosand2) {
			int offset = 0;
			unsigned int facing = 0;
			for (int i = 0; i < FACING_COUNT / 2; i++) {
				CellClass const & cptr = Adjacent_Cell((FacingType)facing);
				IsometricTileType ittype = cptr.ITType;
				if (ittype != IsometricTileTypeClass::SandTile && (ittype < cleartosand1 || ittype > cleartosand2)) {
					offset |= 1 << i;
				}
				facing = Facing_Add(facing, FACING_90);
			}
			if (offset != 0) {
				ITType = IsometricTileType(cleartosand1 + offset);
			} else {
				ITType = IsometricTileTypeClass::SandTile;
			}
		}
	}

	/*
	 * Fix up green LAT transition tiles.
	 */
	if (cleartogreen1 != ISOTILE_INVALID) {
		type = ITType;
		if (type == IsometricTileTypeClass::GreenTile || type >= cleartogreen1 && type <= cleartogreen2) {
			int offset = 0;
			unsigned int facing = 0;
			for (int i = 0; i < FACING_COUNT / 2; i++) {
				CellClass const & cptr = Adjacent_Cell((FacingType)facing);
				IsometricTileType ittype = cptr.ITType;
				if (ittype != IsometricTileTypeClass::GreenTile && (ittype < cleartogreen1 || ittype > cleartogreen2)) {
					offset |= 1 << i;
				}
				facing = Facing_Add(facing, FACING_90);
			}
			if (offset != 0) {
				ITType = IsometricTileType(cleartogreen1 + offset);
			} else {
				ITType = IsometricTileTypeClass::GreenTile;
			}
		}
	}

	/*
	 * Fix up pavement LAT transition tiles. Miscellaneous pavement, medians,
	 * and paved roads all count as pavement for neighbor purposes.
	 */
	if (cleartopave1 != ISOTILE_INVALID) {
		type = ITType;
		if (type == IsometricTileTypeClass::PaveTile || type >= cleartopave1 && type <= cleartopave2) {
			int offset = 0;
			unsigned int facing = 0;
			for (int i = 0; i < FACING_COUNT / 2; i++) {
				CellClass const & cptr = Adjacent_Cell((FacingType)facing);
				IsometricTileType ittype = cptr.ITType;
				if (ittype != IsometricTileTypeClass::PaveTile
				&& (ittype < cleartopave1 || ittype > cleartopave2)
				&& (ittype < miscpave1 || ittype > miscpave2)
				&& (ittype < median1 || ittype > median2)
				&& (ittype < pavedroad1 || ittype > pavedroad2)) {
					offset |= 1 << i;
				}
				facing = Facing_Add(facing, FACING_90);
			}
			if (offset != 0) {
				ITType = IsometricTileType(cleartopave1 + offset);
			} else {
				ITType = IsometricTileTypeClass::PaveTile;
			}
		}
	}

	/*
	 * Fix up crystal LAT transition tiles. Crystal cliff pieces count as
	 * crystal on the appropriate half of the cliff tile.
	 */
	type = ITType;
	if (type == IsometricTileTypeClass::CrystalTile || type >= crystal1 && type <= crystal2) {
		int offset = 0;
		unsigned int facing = 0;
		for (int i = 0; i < FACING_COUNT / 2; i++) {
			CellClass const & cptr = Adjacent_Cell((FacingType)facing);
			IsometricTileType ittype = cptr.ITType;
			CellClass const & subptr = Adjacent_Cell((FacingType)facing);
			int subtile = subptr.SubTile;
			if (ittype != IsometricTileTypeClass::CrystalTile
			&& (ittype < crystal1 || ittype > crystal2)
			&& (ittype != crystalcliff1 || subtile >= 2)
			&& (ittype != crystalcliff1 + 1 || subtile < 2)
			&& (ittype != crystalcliff1 + 4 || (subtile & 1) != 0)
			&& (ittype != crystalcliff1 + 5 || (subtile & 1) == 0)) {
				offset |= 1 << i;
			}
			facing = Facing_Add(facing, FACING_90);
		}
		if (offset != 0) {
			ITType = IsometricTileType(crystal1 + offset);
		} else {
			ITType = IsometricTileTypeClass::CrystalTile;
		}
	}

	/*
	 * Fix up swamp LAT transition tiles.
	 */
	type = ITType;
	if (type == IsometricTileTypeClass::SwampTile || type >= watertoswamp1 && type <= watertoswamp2) {
		int offset = 0;
		unsigned int facing = 0;
		for (int i = 0; i < FACING_COUNT / 2; i++) {
			CellClass const & cptr = Adjacent_Cell((FacingType)facing);
			IsometricTileType ittype = cptr.ITType;
			if ((ittype < swamp1 || ittype > swamp2) && (ittype < watertoswamp1 || ittype > watertoswamp2)) {
				offset |= 1 << i;
			}
			facing = Facing_Add(facing, FACING_90);
		}
		if (offset != 0) {
			ITType = IsometricTileType(watertoswamp1 + offset);
		} else {
			ITType = IsometricTileTypeClass::SwampTile;
		}
	}

	/*
	 * Fix up blue mold LAT transition tiles.
	 */
	type = ITType;
	if (type == IsometricTileTypeClass::BlueMoldTile || type >= mold1 && type <= mold2) {
		int offset = 0;
		unsigned int facing = 0;
		for (int i = 0; i < FACING_COUNT / 2; i++) {
			CellClass const & cptr = Adjacent_Cell((FacingType)facing);
			IsometricTileType ittype = cptr.ITType;
			if (ittype != IsometricTileTypeClass::BlueMoldTile && (ittype < mold1 || ittype > mold2)) {
				offset |= 1 << i;
			}
			facing = Facing_Add(facing, FACING_90);
		}
		if (offset != 0) {
			ITType = IsometricTileType(mold1 + offset);
		} else {
			ITType = IsometricTileTypeClass::BlueMoldTile;
		}
	}

	/*
	 * Smooth ramp tiles against the neighboring ramps along the ramp's axis.
	 */
	type = ITType;
	if (type >= ramp1 && type <= ramp2 || type >= rampsmooth1 && type <= rampsmooth2) {
		int offset = 0;
		if (Ramp == RAMP_WEST) {
			if (Map[::Adjacent_Cell(Fetch_CellID(), FACING_W)].Ramp == RAMP_NONE) {
				offset |= 1;
			}
			if (Map[::Adjacent_Cell(Fetch_CellID(), FACING_E)].Ramp == RAMP_NONE) {
				offset |= 2;
			}
			if (offset != 0) {
				ITType = IsometricTileType(offset + rampsmooth1 - 1);
			}
		}
		if (Ramp == RAMP_NORTH) {
			if (Map[::Adjacent_Cell(Fetch_CellID(), FACING_N)].Ramp == RAMP_NONE) {
				offset |= 1;
			}
			if (Map[::Adjacent_Cell(Fetch_CellID(), FACING_S)].Ramp == RAMP_NONE) {
				offset |= 2;
			}
			if (offset != 0) {
				ITType = IsometricTileType(offset + rampsmooth1 + 2);
			}
		}
		if (Ramp == RAMP_EAST) {
			if (Map[::Adjacent_Cell(Fetch_CellID(), FACING_E)].Ramp == RAMP_NONE) {
				offset |= 1;
			}
			if (Map[::Adjacent_Cell(Fetch_CellID(), FACING_W)].Ramp == RAMP_NONE) {
				offset |= 2;
			}
			if (offset != 0) {
				ITType = IsometricTileType(offset + rampsmooth1 + 5);
			}
		}
		if (Ramp == RAMP_SOUTH) {
			if (Map[::Adjacent_Cell(Fetch_CellID(), FACING_S)].Ramp == RAMP_NONE) {
				offset |= 1;
			}
			if (Map[::Adjacent_Cell(Fetch_CellID(), FACING_N)].Ramp == RAMP_NONE) {
				offset |= 2;
			}
			if (offset != 0) {
				ITType = IsometricTileType(offset + rampsmooth1 + 8);
			}
		}
		if (offset == 0) {
			ITType = IsometricTileType(Ramp + ramp1 - 1);
		}
	}

	/*
	 * If the tile was changed, make sure its art is loaded.
	 */
	if (old_ittype != ITType) {
		IsometricTileTypeClass * ttype = IsometricTileTypes[ITType];
		if (ttype->Get_Image_Data() == NULL) {
			ttype->Load_Tile_Image();
		}
	}
	return(old_ittype != ITType);
}


/// <summary>
/// Assigns ownership of the wall in this cell.
/// A wall answers to whichever wall owning house has the closest building to it. Use this
/// routine when a wall section appears, or when the buildings around one have changed, so
/// that the wall ends up crediting the right house. Cells without a wall are left alone.
/// </summary>
void CellClass::Set_Wall_Owner(void)
{
	if (Overlay != OVERLAY_NONE && OverlayTypes[Overlay]->IsWall) {
		HousesType owner = HOUSE_NONE;
		int dist = 0x7FFFFFFF;
		for (int i = 0; i < Buildings.Count(); i++) {
			BuildingClass * bld = Buildings[i];
			if (bld->IsActive && bld->IsDown && bld->House->Class->IsWallOwner) {
				int d = bld->Distance(this);
				if (d < dist) {
					dist = d;
					owner = bld->Owner();
				}
			}
		}
		Owner = owner;
	}
}


/***********************************************************************************************
 * CellClass::Recalc_Attributes -- Recalculates the ground type attributes for the cell.       *
 *                                                                                             *
 *    This routine recalculates the ground type in the cell. The speeds the find path          *
 *    algorithm and other determinations of the cell type.                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/29/1994 JLB : Created.                                                                 *
 *   06/20/1994 JLB : Knows about template pointer in cell object.                             *
 *=============================================================================================*/
void CellClass::Recalc_Attributes(int cell_height)
{
	if (this == &BlubCell) {
		return;
	}

	CellZoneStruct * zone = &Map.CellZones[Map.Get_Cell_Zone_Index(CellID)];
	CellSubzoneStruct * subzone = &Map.CellSubzones[Map.Get_Cell_Zone_Index(CellID)];

	OverlayType overlay = Overlay;

	/*
	**	Check for wall effects.
	*/
	if (overlay != OVERLAY_NONE) {
		Land = OverlayTypes[overlay]->Land;
		if (Land == LAND_WALL || Land == LAND_RAILROAD || OverlayTypes[overlay]->IsNoUseTileLandType) {
			if (ITType != ISOTILE_NONE && ITType < IsometricTileTypes.Count()) {
				IsometricTileTypeClass * itype = IsometricTileTypes[ITType];
				Ramp = itype->Ramp_Type(SubTile);
			}
			if (Rule->CliffBackImpassability != 0) {
				if (Height + 4 <= Map[CellID + Cell(+0, -1)].Height
				|| (Height + 4 <= Map[CellID + Cell(-1, +0)].Height)
				|| (Height + 4 <= Map[CellID + Cell(+2, +2)].Height)
				|| (Height + 4 <= Map[CellID + Cell(+1, +1)].Height)
				|| (Height + 4 <= Map[CellID + Cell(-1, +1)].Height)
				|| (Height + 4 <= Map[CellID + Cell(+1, -1)].Height)) {
					if (Rule->CliffBackImpassability == 2) {
						Land = LAND_ROCK;
					}
				}
			}
			Recalc_Passability();
			zone->Height = Height;
			subzone->Height = Height;
			zone->Passability = Passability;
			return;
		}
	}

	if (ITType >= IsometricTileTypes.Count()) {
		ITType = ISOTILE_NONE;
	}

	/*
	**	If there is a template associated with this cell, then fetch the
	**	land type given the template type and icon number.
	*/
	if (ITType != ISOTILE_NONE) {
		IsometricTileTypeClass * itype = IsometricTileTypes[ITType];
		bool valid = itype->Is_Tile_Index_Valid(SubTile, false);

		if (Ramp != RAMP_NONE && !valid) {
			valid = itype->Is_Tile_Index_Valid(SubTile, true);
		}

		if (!valid) {
			ITType = ISOTILE_NONE;
			SubTile = 0;
			Land = LAND_CLEAR;
			Ramp = RAMP_NONE;
			if (Rule->CliffBackImpassability != 0) {
				if (Height + 4 <= Map[CellID + Cell(+0, -1)].Height
				|| (Height + 4 <= Map[CellID + Cell(-1, +0)].Height)
				|| (Height + 4 <= Map[CellID + Cell(+2, +2)].Height)
				|| (Height + 4 <= Map[CellID + Cell(+1, +1)].Height)
				|| (Height + 4 <= Map[CellID + Cell(-1, +1)].Height)
				|| (Height + 4 <= Map[CellID + Cell(+1, -1)].Height)) {
					if (Rule->CliffBackImpassability == 2 && Land == LAND_CLEAR) {
						Land = LAND_ROCK;
					}
				}
			}
			Recalc_Passability();
			zone->Height = Height;
			subzone->Height = Height;
			zone->Passability = Passability;
			return;
		}

		Ramp = itype->Ramp_Type(SubTile);
		Fixup_LAT();
		if (Overlay != OVERLAY_NONE) {
			if (Tiberium_Type_Here() != TIBERIUM_NONE) {
				if (Ramp > RAMP_SOUTH) {
					Land = itype->Land_Type(SubTile);;
					Overlay = OVERLAY_NONE;
					OverlayData = 0;
				} else if (!OverlayTypes[Overlay]->Land) {
					Land = LAND_TIBERIUM;
				}
			} else if (!OverlayTypes[Overlay]->IsNoUseTileLandType) {
				Land = itype->Land_Type(SubTile);
			}
		} else {
			Land = itype->Land_Type(SubTile);
		}

		if (Land == LAND_TUNNEL) {
			if (Tube < TUBE_FIRST || Tube >= Tubes.Count()) {
				int tile = ISOTILE_INVALID;
				bool tunnel = false;

				if (ITType >= IsometricTileTypeClass::Tunnels && ITType <= IsometricTileTypeClass::Tunnels + 3) {
					tile = IsometricTileTypeClass::Tunnels;
					tunnel = true;
				} else if (ITType >= IsometricTileTypeClass::TrackTunnels && ITType <= IsometricTileTypeClass::TrackTunnels + 3) {
					tile = IsometricTileTypeClass::TrackTunnels;
					tunnel = true;
				} else if (ITType >= IsometricTileTypeClass::DirtTunnels && ITType <= IsometricTileTypeClass::DirtTunnels + 3) {
					tile = IsometricTileTypeClass::DirtTunnels;
					tunnel = true;
				} else if (ITType >= IsometricTileTypeClass::DirtTrackTunnels && ITType <= IsometricTileTypeClass::DirtTrackTunnels + 3) {
					tile = IsometricTileTypeClass::DirtTrackTunnels;
					tunnel = true;
				}

				if (tunnel) {
					static FacingType _facings[5] = { FACING_E, FACING_S, FACING_W, FACING_N, FACING_NONE };
					int tunnel_dir = ITType - tile;
					if (tunnel_dir != -1) {
						new TubeClass(CellID, _facings[tunnel_dir]);
					}
				}
			}
		}

		if (cell_height != -1) {
			Height = cell_height;
		}

		int width;
		int height;
		itype->Get_Tile_Pixel_Dimensions(SubTile, width, height);
		Elevation = (height - CELL_PIXEL_H / 2) / (CELL_PIXEL_H / 4);
		if (!IsAnimAttached && itype->Anim != ANIM_NONE && itype->AttachesTo == SubTile) {
			Coord anim_coord = Coord(CellID, LEVEL_LEPTON_H * Height) + TacticalMap->Pixel_To_Coord_Absolute(itype->Offset);
			AnimClass * anim = new AnimClass(AnimTypes[itype->Anim], anim_coord, 0, -1, ShapeFlags_Type(SHAPE_ZREAD | SHAPE_WIN_REL | SHAPE_CENTER), 0);
			anim->Attach_To_Cell(itype->ZAdjust);
			IsAnimAttached = true;
		}

		if (itype->IsShadowCaster) {
			const Cell * list = itype->Shadow_Caster_List();
			if (list) {
				while (*list != REFRESH_EOL && *list != CELL_NONE) {
					Cell pos = CellID + *list;
					Map[pos].IsOvershadowed = true;
					list++;
				}
			}
		}
	} else {
		/*
		**	No template is the same as clear terrain.
		*/
		if (overlay == OVERLAY_NONE || OverlayTypes[overlay]->IsNoUseTileLandType) {
			Land = LAND_CLEAR;
		} else {
			Land = OverlayTypes[overlay]->Land;
		}
		Ramp = RAMP_NONE;
	}

	if (Rule->CliffBackImpassability != 0) {
		if (Height + 4 <= Map[CellID + Cell(+0, -1)].Height
		|| (Height + 4 <= Map[CellID + Cell(-1, +0)].Height)
		|| (Height + 4 <= Map[CellID + Cell(+2, +2)].Height)
		|| (Height + 4 <= Map[CellID + Cell(+1, +1)].Height)
		|| (Height + 4 <= Map[CellID + Cell(-1, +1)].Height)
		|| (Height + 4 <= Map[CellID + Cell(+1, -1)].Height)) {
			if (Rule->CliffBackImpassability == 2) {
				if (Land == LAND_CLEAR || Land == LAND_WATER || Land == LAND_BEACH || Land == LAND_ICE) {
					Land = LAND_ROCK;
				}
			}
		}
	}

	Recalc_Passability();
	zone->Height = Height;
	zone->Passability = Passability;
	subzone->Height = Height;
}


/// <summary>
/// Destroys the bridge deck passing over this cell.
/// Anything standing on the deck is killed outright and anything hanging beneath it is turned
/// loose to fall, then the cell is queued up for the map to knock the span down. Debris and an
/// explosion usually accompany the collapse.
/// </summary>
/// <remarks>Nothing happens at all while the map editor is running.</remarks>
void CellClass::Destroy_Bridge(void)
{
	if (!Debug_Map) {
		ObjectClass * occupier = Cell_Occupier();
		while (occupier != NULL) {
			ObjectClass * next = occupier->Next;
			occupier->Take_Damage(occupier->Strength, 0, Rule->C4Warhead, NULL, true, true);
			occupier = next;
		}

		occupier = Cell_Occupier(true);
		while (occupier != NULL) {
			ObjectClass * next = occupier->Next;
			occupier->Fall_From_Height();
			occupier = next;
		}

		Map.PendingBridgeCells.Add(CellID);

		if (Rule->BridgeExplosions.Count() > 0 && Random_Double(0.0, 1.0) < 0.95) {
			Coord coord = CellID;
			coord.Z = LEVEL_LEPTON_H * Height + BRIDGE_LEPTON_HEIGHT;
			coord.X += Random_Double(-0.5, 0.5) * 50;
			coord.Y += Random_Double(-0.5, 0.5) * 50;

			if (Random_Double(0.0, 1.0) < 0.5) {
				new AnimClass(Rule->MetallicDebris[Random_Pick(0, Rule->MetallicDebris.Count() - 1)], coord);
			}

			new AnimClass(Rule->BridgeExplosions[Random_Pick(0, Rule->BridgeExplosions.Count() - 1)], coord, Random_Pick(1, 5));
		}
	}
}


/// <summary>
/// Marks the cells that one high-bridge deck piece occupies and overshadows.
/// The deck sprite is wider than the cell that hosts it, so this routine reaches out to the
/// neighboring cells and flags each one as being under the bridge, as walk-on bridge surface,
/// or as merely overshadowed, according to how far the deck stretches over it. It is called
/// once for each bridge overlay cell as the span is laid down or taken away.
/// </summary>
/// <param name="facing">The axis across the deck's width, not the direction traffic travels.
/// Only FACING_N (east-west bridge) and FACING_W (north-south bridge) are ever used.</param>
/// <param name="state">Is the bridge being placed? False clears the flags and tears the
/// bridge down here.</param>
void CellClass::Set_Under_Bridge(FacingType facing, bool state)
{
	/*
	 * Preload this deck cell's overlay frame for the bridge orientation. This is
	 * immediately overwritten by the assignment below, so it is redundant.
	 */
	if (facing == FACING_N) {
		OverlayData = OVERLAYDATA_BRIDGE_EW_FULL1;
	} else {
		OverlayData = OVERLAYDATA_BRIDGE_NS_FULL1;
	}

	/*
	 * This cell hosts the raised bridge deck piece: mark it as the deck, under the
	 * bridge, traversable, drawn surface, and overshadowed. WasUnderBridge is the
	 * inverse of IsUnderBridge so a destroyed span can be found and repaired later.
	 */
	IsBridgeDeck = state;
	IsUnderBridge = state;
	IsBridgeTraversable = state;
	IsBridgeSurface = state;
	IsOvershadowed = state;
	WasUnderBridge = !state;
	IsBridgeEastWest = (facing == FACING_N);
	if (state) {
		OverlayData = (facing != FACING_N) ? OVERLAYDATA_BRIDGE_NS_FULL1 : OVERLAYDATA_BRIDGE_EW_FULL1;
	} else {
		OverlayData = 0;
		Destroy_Bridge();
	}
	Map.Radar_Background(CellID);

	/*
	 * One cell across the deck (a single step in the `facing` direction): under the
	 * deck, part of the walkable surface, and traversable. Every cell other than the
	 * deck cell points its BridgeDeckCell back at this deck.
	 */
	CellClass * across1 = &Adjacent_Cell(facing);
	across1->BridgeDeckCell = state ? this : NULL;
	across1->IsUnderBridge = state;
	across1->IsBridgeTraversable = state;
	across1->IsBridgeSurface = state;
	across1->IsOvershadowed = state;
	across1->WasUnderBridge = !state;
	across1->IsBridgeEastWest = (facing == FACING_N);
	if (state) {
		across1->OverlayData = (facing != FACING_N) ? OVERLAYDATA_BRIDGE_NS_FULL1 : OVERLAYDATA_BRIDGE_EW_FULL1;
	} else {
		across1->OverlayData = 0;
		across1->Destroy_Bridge();
	}
	Map.Radar_Background(across1->CellID);

	/*
	 * Two cells across -- the far edge of the under-bridge strip. Under the deck and
	 * part of the surface, but deliberately NOT traversable: a unit cannot step off
	 * the far edge of the deck here.
	 */
	CellClass * across2 = &across1->Adjacent_Cell(facing);
	across2->BridgeDeckCell = state ? this : NULL;
	across2->IsUnderBridge = state;
	across2->IsBridgeTraversable = false;
	across2->IsBridgeSurface = state;
	across2->IsOvershadowed = state;
	across2->WasUnderBridge = !state;
	across2->IsBridgeEastWest = (facing == FACING_N);
	if (state) {
		across2->OverlayData = (facing != FACING_N) ? OVERLAYDATA_BRIDGE_NS_FULL1 : OVERLAYDATA_BRIDGE_EW_FULL1;
	} else {
		across2->OverlayData = 0;
		across2->Destroy_Bridge();
	}
	Map.Radar_Background(across2->CellID);

	/*
	 * Three cells across -- only the raised deck surface reaches this far. The surface
	 * strip is the under-bridge strip shifted one cell along `facing` (the deck is
	 * drawn lifted), so this edge is surface-only.
	 */
	CellClass * across3 = &across2->Adjacent_Cell(facing);
	across3->IsBridgeSurface = state;

	/*
	 * One cell across the deck on the opposite side (a step against `facing`): under
	 * the deck and traversable (the approach cell where a unit steps onto the bridge),
	 * but not part of the drawn surface.
	 */
	CellClass * back = &Adjacent_Cell(Facing_Sub(facing, FACING_180));
	back->BridgeDeckCell = state ? this : NULL;
	back->IsUnderBridge = state;
	back->IsBridgeTraversable = state;
	back->WasUnderBridge = !state;
	back->IsBridgeSurface = false;
	back->IsOvershadowed = state;
	back->IsBridgeEastWest = (facing == FACING_N);
	if (state) {
		back->OverlayData = (facing != FACING_N) ? OVERLAYDATA_BRIDGE_NS_FULL1 : OVERLAYDATA_BRIDGE_EW_FULL1;
	} else {
		back->OverlayData = 0;
		back->Destroy_Bridge();
	}
	Map.Radar_Background(back->CellID);

	/*
	 * A north-south bridge (FACING_W) reaches one cell further across on the opposite
	 * side than an east-west bridge does: its isometric footprint overshadows a second
	 * cell beyond `back`. Flag that cell as overshadowed and link it to this deck. It is
	 * shadow-only -- not under the bridge, not surface, not traversable. An east-west
	 * bridge (FACING_N) does not reach this far and has no equivalent cell.
	 */
	if (facing == FACING_W) {
		CellClass * back2 = &back->Adjacent_Cell(FACING_E);
		back2->BridgeDeckCell = state ? this : NULL;
		back2->IsOvershadowed = state;
	}
}


/// <summary>
/// Marks the cells that one rail high-bridge deck piece occupies and overshadows.
/// This is the train bridge counterpart of Set_Under_Bridge and covers the same strip of
/// cells in the same way. It is called once for each rail bridge overlay cell as the span
/// is laid down or taken away.
/// </summary>
/// <param name="facing">The axis across the deck's width, not the direction traffic travels.
/// Only FACING_N (east-west bridge) and FACING_W (north-south bridge) are ever used.</param>
/// <param name="state">Is the bridge being placed? False clears the flags and tears the
/// bridge down here.</param>
void CellClass::Set_Under_Rail_Bridge(FacingType facing, bool state)
{
	/*
	 * Preload this deck cell's overlay frame for the bridge orientation. This is
	 * immediately overwritten by the assignment below, so it is redundant.
	 */
	if (facing == FACING_N) {
		OverlayData = OVERLAYDATA_BRIDGE_EW_FULL1;
	} else {
		OverlayData = OVERLAYDATA_BRIDGE_NS_FULL1;
	}

	/*
	 * This cell hosts the raised bridge deck piece: mark it as the deck, under the
	 * bridge, traversable, drawn surface, and overshadowed. WasUnderBridge is the
	 * inverse of IsUnderBridge so a destroyed span can be found and repaired later.
	 */
	IsBridgeDeck = state;
	IsUnderBridge = state;
	IsBridgeTraversable = state;
	IsBridgeSurface = state;
	IsOvershadowed = state;
	WasUnderBridge = !state;
	IsBridgeEastWest = (facing == FACING_N);
	if (state) {
		OverlayData = (facing != FACING_N) ? OVERLAYDATA_BRIDGE_NS_FULL1 : OVERLAYDATA_BRIDGE_EW_FULL1;
	} else {
		OverlayData = 0;
		Destroy_Bridge();
	}
	Map.Radar_Background(CellID);

	/*
	 * One cell across the deck (a single step in the `facing` direction): under the
	 * deck, part of the walkable surface, and traversable. Every cell other than the
	 * deck cell points its BridgeDeckCell back at this deck.
	 */
	CellClass * across1 = &Adjacent_Cell(facing);
	across1->BridgeDeckCell = state ? this : NULL;
	across1->IsUnderBridge = state;
	across1->IsBridgeTraversable = state;
	across1->IsBridgeSurface = state;
	across1->IsOvershadowed = state;
	across1->WasUnderBridge = !state;
	across1->IsBridgeEastWest = (facing == FACING_N);
	if (state) {
		across1->OverlayData = (facing != FACING_N) ? OVERLAYDATA_BRIDGE_NS_FULL1 : OVERLAYDATA_BRIDGE_EW_FULL1;
	} else {
		across1->OverlayData = 0;
		across1->Destroy_Bridge();
	}
	Map.Radar_Background(across1->CellID);

	/*
	 * Two cells across -- the far edge of the under-bridge strip. Under the deck and
	 * part of the surface, but deliberately NOT traversable: a unit cannot step off
	 * the far edge of the deck here.
	 */
	CellClass * across2 = &across1->Adjacent_Cell(facing);
	across2->BridgeDeckCell = state ? this : NULL;
	across2->IsUnderBridge = state;
	across2->IsBridgeTraversable = false;
	across2->IsBridgeSurface = state;
	across2->IsOvershadowed = state;
	across2->WasUnderBridge = !state;
	across2->IsBridgeEastWest = (facing == FACING_N);
	if (state) {
		across2->OverlayData = (facing != FACING_N) ? OVERLAYDATA_BRIDGE_NS_FULL1 : OVERLAYDATA_BRIDGE_EW_FULL1;
	} else {
		across2->OverlayData = 0;
		across2->Destroy_Bridge();
	}
	Map.Radar_Background(across2->CellID);

	/*
	 * Three cells across -- only the raised deck surface reaches this far. The surface
	 * strip is the under-bridge strip shifted one cell along `facing` (the deck is
	 * drawn lifted), so this edge is surface-only.
	 */
	CellClass * across3 = &across2->Adjacent_Cell(facing);
	across3->IsBridgeSurface = state;

	/*
	 * One cell across the deck on the opposite side (a step against `facing`): under
	 * the deck and traversable (the approach cell where a unit steps onto the bridge),
	 * but not part of the drawn surface.
	 */
	CellClass * back = &Adjacent_Cell(Facing_Sub(facing, FACING_180));
	back->BridgeDeckCell = state ? this : NULL;
	back->IsUnderBridge = state;
	back->IsBridgeTraversable = state;
	back->WasUnderBridge = !state;
	back->IsBridgeSurface = false;
	back->IsOvershadowed = state;
	back->IsBridgeEastWest = (facing == FACING_N);
	if (state) {
		back->OverlayData = (facing != FACING_N) ? OVERLAYDATA_BRIDGE_NS_FULL1 : OVERLAYDATA_BRIDGE_EW_FULL1;
	} else {
		back->OverlayData = 0;
		back->Destroy_Bridge();
	}
	Map.Radar_Background(back->CellID);

	/*
	 * A north-south bridge (FACING_W) reaches one cell further across on the opposite
	 * side than an east-west bridge does: its isometric footprint overshadows a second
	 * cell beyond `back`. Flag that cell as overshadowed and link it to this deck. It is
	 * shadow-only -- not under the bridge, not surface, not traversable. An east-west
	 * bridge (FACING_N) does not reach this far and has no equivalent cell.
	 */
	if (facing == FACING_W) {
		CellClass * back2 = &back->Adjacent_Cell(FACING_E);
		back2->BridgeDeckCell = state ? this : NULL;
		back2->IsOvershadowed = state;
	}
}


/***********************************************************************************************
 * CellClass::Occupy_Down -- Flag occupation of specified cell.                                *
 *                                                                                             *
 *    This routine is used to mark the cell as being occupied by the specified object.         *
 *                                                                                             *
 * INPUT:   object   -- The object that is to occupy the cell                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1994 JLB : Created.                                                                 *
 *   11/29/1994 JLB : Simplified.                                                              *
 *=============================================================================================*/
void CellClass::Occupy_Down(ObjectClass * object, bool bridge)
{
	assert(object != NULL && object->IsActive);

	ObjectClass * optr = Cell_Occupier(bridge);

	if (object == NULL) return;

	/*
	**	Always add buildings to the end of the occupation chain. This is necessary because
	**	the occupation chain is a single list even though buildings occupy more than one
	**	cell. If more than one building is allowed to occupy the same cell, then this chain
	**	logic will fail.
	*/
	if (object->RTTI == RTTI_BUILDING && optr) {
		while (optr->Next != NULL) {
			assert(optr != object);
			assert(optr->RTTI != RTTI_BUILDING);
			optr = optr->Next;
		}
		optr->Next = object;
		object->Next = NULL;
	} else if (bridge) {
		if (optr == NULL || optr->Next != object) {
			object->Next = optr;
			BridgeOccupierPtr = object;
		}
	} else {
		if (optr == NULL || optr->Next != object) {
			object->Next = optr;
			OccupierPtr = object;
		}
	}

	/*
	**	If being placed down on a visible square, then flag this
	**	techno object as being revealed to the player.
	*/
	if (!Map.Is_Shrouded(Cell_Coord()) && !Map.Is_Fogged(Cell_Coord()) || Session.Type != GAME_NORMAL) {
		object->Revealed(PlayerPtr);
	}

	/*
	**	Special occupy bit set.
	*/
	if (object->RTTI != RTTI_INFANTRY && object->Occupies_Cells()) {
		if (object->RTTI == RTTI_BUILDING) {
			object->Set_Occupy_Bit(CellID);
		} else {
			object->Set_Occupy_Bit(object->Get_Coord());
		}
	}
}


/***********************************************************************************************
 * CellClass::Occupy_Up -- Removes occupation flag from the specified cell.                    *
 *                                                                                             *
 *    This routine will lift the object from the cell and free the cell to be occupied by      *
 *    another object. Only if the cell was previously marked with the object specified, will   *
 *    the object be lifted off. This routine is the counterpart to Occupy_Down().              *
 *                                                                                             *
 * INPUT:   object   -- The object that is being lifted off.                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1994 JLB : Created.                                                                 *
 *   11/29/1994 JLB : Fixed to handle next pointer in previous object.                         *
 *=============================================================================================*/
void CellClass::Occupy_Up(ObjectClass * object, bool bridge)
{
	assert(object != NULL && object->IsActive);

	if (object == NULL) return;

	ObjectClass * optr = Cell_Occupier(bridge);		// Working pointer to the objects in the chain.

	if (optr == object) {
		if (bridge) {
			BridgeOccupierPtr = object->Next;
		} else {
			OccupierPtr = object->Next;
		}
		object->Next = NULL;
	} else {
		bool found = false;
		while (optr != NULL) {
			if (optr->Next == object) {
				optr->Next = object->Next;
				object->Next = NULL;
				found = true;
				break;
			}
			optr = optr->Next;
		}
//		assert(found);
	}

	/*
	**	Special occupy bit clear.
	*/
	if (object->RTTI != RTTI_INFANTRY && object->Occupies_Cells()) {
		if (object->RTTI == RTTI_BUILDING) {
			object->Clear_Occupy_Bit(CellID);
		} else {
			object->Clear_Occupy_Bit(object->Get_Coord());
		}
	}
}


/***********************************************************************************************
 * CellClass::Cell_Unit -- Returns with pointer to unit occupying cell.                        *
 *                                                                                             *
 *    This routine will determine if a unit is occupying the cell and if so, return a pointer  *
 *    to it. If there is no unit occupying the cell, then NULL is returned.                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with pointer to unit occupying cell, else NULL.                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
UnitClass * CellClass::Cell_Unit(bool bridge) const
{
	return((UnitClass*)Cell_Find_Object(RTTI_UNIT, bridge));
}


/// <summary>
/// Fetches the aircraft occupying this cell.
/// </summary>
/// <param name="bridge">Should the bridge deck be searched rather than the ground?</param>
/// <returns>Returns with a pointer to the aircraft found, or NULL if there is none.</returns>
AircraftClass * CellClass::Cell_Aircraft(bool bridge) const
{
	return((AircraftClass*)Cell_Find_Object(RTTI_AIRCRAFT, bridge));
}


/***********************************************************************************************
 * CellClass::Cell_Infantry -- Returns with pointer of first infantry unit occupying the cell. *
 *                                                                                             *
 *    This routine examines the cell and returns a pointer to the first infantry unit          *
 *    that occupies it. If there is no infantry unit in the cell, then NULL is returned.       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with pointer to infantry unit occupying the cell or NULL if none are       *
 *          present.                                                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/21/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
InfantryClass * CellClass::Cell_Infantry(bool bridge) const
{
	return((InfantryClass*)Cell_Find_Object(RTTI_INFANTRY, bridge));
}


/// <summary>
/// Draws the building placement cursor over this cell.
/// This routine is called for each cell the placement cursor covers while the player carries a
/// pending building around the map. It decides whether the building may legally sit here --
/// weighing upgrades, proximity and shroud as the case demands -- and draws the valid cursor
/// or the blocked one to suit.
/// </summary>
/// <param name="xpoint">The pixel position to draw the cell at.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
/// <param name="zeroalpha">Should the cursor be drawn with the zero alpha blend?</param>
/// <returns>bool; May the pending building be placed on this cell?</returns>
bool CellClass::Draw_Placement_Cursor(Point2D const & xpoint, Rect const & cliprect, bool zeroalpha)
{
	Point2D point;
	point.X = ISO_TILE_PIXEL_W / 2;

	bool ok = false;
	if (IsCursorHere) {

		/*
		 * Fetch the locomotion type of the object being placed. It is needed by
		 * the clear-to-build test below to validate the building footing.
		 */
		SpeedType loco = SPEED_NONE;
		if (Map.PendingObjectPtr != NULL && Map.PendingObjectPtr->RTTI == RTTI_BUILDING) {
			loco = ((BuildingClass *)Map.PendingObjectPtr)->Class->Speed;
		}

		/*
		 * Compute the vertical draw offsets. The cursor is lifted according to
		 * the cell's height level and nudged up further on ramp (sloped) cells.
		 */
		int lift = LEVEL_PIXEL_H_1 * Height;
		point.Y = -1 - lift;
		int height_offset = -2 - lift;
		if (Ramp) {
			height_offset -= 10;
		}

		/*
		 * Only a building type can ever be placed; anything else stays illegal.
		 */
		if (Map.PendingObject->RTTI == RTTI_BUILDINGTYPE) {

			Coord coord(CellID, 0);
			coord.Z = Map.Get_Height_GL(coord);

			BuildingTypeClass * ptype = (BuildingTypeClass *)Map.PendingObject;

			if (Map.Is_Shrouded(coord)) {

				/*
				 * Under shroud, only a tile-laying building (e.g. a base node) may
				 * be placed, and only when the proximity requirement is satisfied.
				 */
				if (Map.ProximityCheck && Map.PendingObject != NULL
						&& Map.PendingObject->RTTI == RTTI_BUILDINGTYPE
						&& ptype->ToTile != NULL) {
					ok = true;
				} else {
					ok = false;
				}

			} else if (!ptype->PowersUpBuilding.empty()) {

				/*
				 * This object is an upgrade. It may be placed only onto an existing
				 * building in this cell that is able to accept the upgrade.
				 */
				BuildingClass * building = (BuildingClass *)Cell_Object();
				if (building != NULL && building->RTTI == RTTI_BUILDING
						&& building->Can_Upgrade(ptype, PlayerPtr)) {
					ok = true;
				}

			} else {

				/*
				 * Normal placement. The footprint must be clear to build, and a
				 * tile-laying building may not be placed on top of a road.
				 */
				if (Map.ProximityCheck && Map.PendingObject != NULL
						&& Map.PendingObject->RTTI == RTTI_BUILDINGTYPE
						&& Is_Clear_To_Build(loco, ptype, PlayerPtr)
						&& !(ptype->ToTile != NULL && Land == LAND_ROAD)) {
					ok = true;
				}
			}
		}

		/*
		 * Draw the cursor shape -- the valid cursor for a legal placement, or the
		 * cell's blocked/ramp cursor otherwise.
		 */
		ShapeFlags_Type flags = ShapeFlags_Type(zeroalpha
			? SHAPE_ZERO_ALPHA|SHAPE_WIN_REL|SHAPE_CENTER|SHAPE_TRANSLUCENT50
			: SHAPE_NONZERO_ALPHA|SHAPE_WIN_REL|SHAPE_CENTER|SHAPE_TRANSLUCENT50);

		if (ok) {
			Draw_Shape(*LogicalSurface, *NormalDrawer, (ShapeSet *)DisplayClass::PlacementShapes,
				0, point + xpoint, cliprect, flags, 0, height_offset);
		} else {
			Draw_Shape(*LogicalSurface, *NormalDrawer, (ShapeSet *)DisplayClass::PlacementShapes,
				Ramp + 1, point + xpoint, cliprect, flags, 0, height_offset);
		}
	}

	return(ok);
}


/// <summary>
/// Draws one shroud shape into the alpha buffer.
/// The artwork comes from the fog shape set when the scenario is playing with fog of war and
/// from the shroud set otherwise. Unlike the fog pass, the shape is laid straight over
/// whatever the alpha buffer already held.
/// </summary>
/// <param name="drawpoint">The pixel position to draw the shape at.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
/// <param name="shapenum">The shroud shape to draw.</param>
void CellClass::Draw_Shroud_Or_Fog_Shape(Point2D const & drawpoint, Rect const & cliprect, int shapenum)
{
	static bool shapes_loaded = false;
	static ShapeSet const * shadow_shapes = NULL;
	static ShapeSet const * fog_shapes = NULL;

	if (!shapes_loaded) {
		shapes_loaded = true;
		shadow_shapes = (ShapeSet const *)MFCD::Retrieve("SHROUD.SHP");
		fog_shapes = (ShapeSet const *)MFCD::Retrieve("FOG.SHP");
	}

	ShapeSet const * shapes;
	if (Scen->Special.IsFogOfWar) {
		shapes = fog_shapes;
	} else {
		shapes = shadow_shapes;
	}

	Rect shaperect = shapes->Get_Rect(shapenum);

	if (drawpoint.Y + shaperect.Y + shaperect.Height < cliprect.Y + 1) return;
	if (drawpoint.Y >= cliprect.Height + cliprect.Y) return;
	if (drawpoint.X + shaperect.X + shaperect.Width < cliprect.X + 1) return;
	if (drawpoint.X >= cliprect.X + cliprect.Width) return;

	int inter_top = std::max(cliprect.Y, drawpoint.Y + shaperect.Y);
	int src_y = inter_top - drawpoint.Y - shaperect.Y;
	int inter_bottom = std::min(cliprect.Y + cliprect.Height, drawpoint.Y + shaperect.Y + shaperect.Height);

	int inter_left = std::max(cliprect.X, drawpoint.X + shaperect.X);
	int src_x = inter_left - drawpoint.X - shaperect.X;
	int inter_right = std::min(drawpoint.X + shaperect.X + shaperect.Width, cliprect.X + cliprect.Width);

	int shape_skip = drawpoint.X - inter_right + shaperect.X + shaperect.Width + src_x;
	int alpha_skip = inter_left - inter_right + AlphaBuffer->Get_Buffer_Width();

	unsigned char * shapedata = (unsigned char *)shapes->Get_Data(shapenum);
	unsigned short * alphaptr = AlphaBuffer->Get_Buffer_Offset(Point2D(inter_left, inter_top - TacticalRect.Y));

	unsigned char * shapeptr = (unsigned char *)&shapedata[src_x + src_y * shaperect.Width];
	if (&alphaptr[inter_right - inter_left + (inter_bottom - inter_top) * AlphaBuffer->Get_Buffer_Width() + 2] >= AlphaBuffer->Get_Buffer_End()) {
		for (int i = inter_top; i < inter_bottom; i++) {
			for (int j = inter_left; j < inter_right; j++) {
				unsigned char pixel = *shapeptr++;
				if (pixel != 0xFE) {
					*alphaptr = pixel;
				}
				alphaptr++;
				alphaptr = AlphaBuffer->Wrap_Overflow(alphaptr);
			}
			shapeptr += shape_skip;
			alphaptr += alpha_skip;
			alphaptr = AlphaBuffer->Wrap_Overflow(alphaptr);
		}
	} else {
		for (int i = inter_top; i < inter_bottom; i++) {
			for (int j = inter_left; j < inter_right; j++) {
				unsigned char pixel = *shapeptr++;
				if (pixel != 0xFE) {
					*alphaptr = pixel;
				}
				++alphaptr;
			}
			shapeptr += shape_skip;
			alphaptr += alpha_skip;
		}
	}
}


/// <summary>
/// Draws one fog shape into the alpha buffer.
/// Fog dims what is under it rather than hiding it, so this routine blends the shape against
/// the alpha buffer instead of writing straight over it. Use Draw_Shroud_Or_Fog_Shape for the
/// opaque shroud pass.
/// </summary>
/// <param name="drawpoint">The pixel position to draw the shape at.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
/// <param name="shapenum">The fog shape to draw.</param>
void CellClass::Draw_Fog_Shape(Point2D const & drawpoint, Rect const & cliprect, int shapenum)
{
	static bool shapes_loaded = false;
	static ShapeSet const * shapes = NULL;

	if (!shapes_loaded) {
		shapes_loaded = true;
		shapes = (ShapeSet const *)MFCD::Retrieve("FOG.SHP");
	}

	Rect shaperect = shapes->Get_Rect(shapenum);

	if (drawpoint.Y + shaperect.Y + shaperect.Height < cliprect.Y + 1) return;
	if (drawpoint.Y >= cliprect.Height + cliprect.Y) return;
	if (drawpoint.X + shaperect.X + shaperect.Width < cliprect.X + 1) return;
	if (drawpoint.X >= cliprect.X + cliprect.Width) return;

	int inter_top = std::max(cliprect.Y, drawpoint.Y + shaperect.Y);
	int src_y = inter_top - drawpoint.Y - shaperect.Y;
	int inter_bottom = std::min(cliprect.Y + cliprect.Height, drawpoint.Y + shaperect.Y + shaperect.Height);

	int inter_left = std::max(cliprect.X, drawpoint.X + shaperect.X);
	int src_x = inter_left - drawpoint.X - shaperect.X;
	int inter_right = std::min(drawpoint.X + shaperect.X + shaperect.Width, cliprect.X + cliprect.Width);

	int shape_skip = drawpoint.X - inter_right + shaperect.X + shaperect.Width + src_x;
	int alpha_skip = inter_left - inter_right + AlphaBuffer->Get_Buffer_Width();

	unsigned char * shapedata = (unsigned char *)shapes->Get_Data(shapenum);
	unsigned short * alphaptr = AlphaBuffer->Get_Buffer_Offset(Point2D(inter_left, inter_top - TacticalRect.Y));

	unsigned char * shapeptr = (unsigned char *)&shapedata[src_x + src_y * shaperect.Width];
	if (&alphaptr[inter_right - inter_left + (inter_bottom - inter_top) * AlphaBuffer->Get_Buffer_Width() + 2] >= AlphaBuffer->Get_Buffer_End()) {
		for (int i = inter_top; i < inter_bottom; i++) {
			for (int j = inter_left; j < inter_right; j++) {
				unsigned char pixel = *shapeptr++;
				if (pixel <= 0x7F) {
					if (*alphaptr == 127) {
						*alphaptr = pixel;
					} else {
						int value = (unsigned short)*alphaptr + pixel - 127;
						*alphaptr = (value <= 0) ? 0 : value;
					}
				}
				alphaptr++;
				alphaptr = AlphaBuffer->Wrap_Overflow(alphaptr);
			}
			shapeptr += shape_skip;
			alphaptr += alpha_skip;
			alphaptr = AlphaBuffer->Wrap_Overflow(alphaptr);
		}
	} else {
		for (int i = inter_top; i < inter_bottom; i++) {
			for (int j = inter_left; j < inter_right; j++) {
				unsigned char pixel = *shapeptr++;
				if (pixel <= 0x7F) {
					if (*alphaptr == 127) {
						*alphaptr = pixel;
					} else {
						int value = (unsigned short)*alphaptr + pixel - 127;
						*alphaptr = (value <= 0) ? 0 : value;
					}
				}
				++alphaptr;
			}
			shapeptr += shape_skip;
			alphaptr += alpha_skip;
		}
	}
}


static ShapeSet const * Tiberium_Overlay_Image(CellClass const & cell, TiberiumClass const & tiberium)
{
	int overlay = tiberium.Overlay->HeapID;
	if (cell.Ramp != RAMP_NONE) {
		overlay += tiberium.Variety
			+ tiberium.RampVariety / 4 * (cell.Ramp - 1)
			+ cell.CellID.X * cell.CellID.Y % (tiberium.RampVariety / 4);
	} else {
		overlay += cell.CellID.X * cell.CellID.Y % tiberium.Variety;
	}
	return((ShapeSet const *)OverlayTypes[overlay]->Get_Image_Data());
}


/// <summary>
/// Draws the shadow cast by the overlay on this cell.
/// This routine is the darkened companion pass to Draw_Overlay, drawing the shadow artwork
/// the overlay's shape set carries alongside the overlay itself.
/// </summary>
/// <param name="xpoint">The pixel position to draw the cell at.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
void CellClass::Draw_Overlay_Shadow(Point2D const & xpoint, Rect const & cliprect)
{
	int yoffset = LEVEL_PIXEL_H_1 * Height;

	OverlayTypeClass const * overlay = OverlayTypes[Overlay];
	if (overlay->IsTiberium) {
		TiberiumType tibtype = Tiberium_Type_Here();
		if (tibtype == TIBERIUM_NONE) return;
		ShapeSet const * tiberium_image = Tiberium_Overlay_Image(*this, *Tiberiums[tibtype]);
		if (tiberium_image == NULL || OverlayData >= tiberium_image->Get_Count() / 2) return;
	}

	ShapeSet *shape = (ShapeSet *)overlay->Get_Image_Data();

	Point2D point = Overlay_Draw_Offset();

	point = point + xpoint;
	point -= cliprect.Top_Left();

	if (IsBridgeDeck) {
		if (OverlayData >= OVERLAYDATA_BRIDGE_NS_FULL1 && OverlayData <= OVERLAYDATA_BRIDGE_NS_END2) {
			point += Point2D(-(ISO_TILE_PIXEL_W / 4), ISO_TILE_PIXEL_H / 4);
		}
	}
	if (Drawer == NULL) {
		Init_Drawer();
	}
	Draw_Shape(
		*LogicalSurface,
		*Drawer,
		shape,
		OverlayData + shape->Get_Count() / 2,
		point,
		cliprect,
		ShapeFlags_Type(SHAPE_ZWRITE|SHAPE_WIN_REL|SHAPE_CENTER|SHAPE_DARKEN),
		0,
		-2 - yoffset
		);
}


/// <summary>
/// Draws the overlay that sits on this cell.
/// This routine handles every flavor of overlay -- bridge decks, tiberium, walls, veins and
/// ordinary ground clutter -- each of which wants its own color scheme, lighting and depth
/// treatment. Veinholes are left to the veinhole monster to draw.
/// </summary>
/// <param name="xpoint">The pixel position to draw the cell at.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
void CellClass::Draw_Overlay(Point2D const & xpoint, Rect const & cliprect)
{
	if (Overlay == OVERLAY_VEINHOLE || Overlay == OVERLAY_VEINHOLE_DUMMY) {
		return;
	}

	OverlayTypeClass * otype = OverlayTypes[Overlay];

	ShapeSet const * shape = (ShapeSet const *)otype->Get_Image_Data();

	Point2D point = Overlay_Draw_Offset();

	point = point + xpoint;
	int yoffset = LEVEL_PIXEL_H_1 * (Height + (IsBridgeDeck ? BRIDGE_CELL_HEIGHT : 0));
	point -= cliprect.Top_Left();

	if (Drawer == NULL) {
		Init_Drawer();
	}

	if (IsBridgeDeck) {
		if (LastBridgeDrawFrame != Frame || LastBridgeDrawRedraws != Map.Redraws || LastBridgeDrawRect != cliprect) {

			int shapenum = OverlayData;
			if (shapenum == 0 || shapenum == 9) {
				/// A 4x4 variation lookup keyed on the low bits of the cell position. It nudges
				/// the bridge overlay frame by 0 - 3 so that a run of them does not visibly repeat.
				static int _bridge_variation[16] = {
					0, 1, 2, 3,
					3, 2, 1, 0,
					2, 3, 0, 1,
					1, 0, 3, 2
				};
				shapenum += _bridge_variation[(CellID.X & 3) | (4 * (CellID.Y & 3))];
			}

			Draw_Shape(*LogicalSurface, *Drawer, shape, shapenum, point, cliprect, (ShapeFlags_Type)(SHAPE_CENTER | SHAPE_WIN_REL | SHAPE_ALPHA | SHAPE_ZWRITE), 0, -2 - yoffset, ZGRAD_GROUND, AltBrightness);
			LastBridgeDrawFrame = Frame;
			LastBridgeDrawRect = TacticalRect;
			LastBridgeDrawRedraws = Map.Redraws;
		}
		return;
	}

	if (otype->IsTiberium) {
		TiberiumType tibtype = Tiberium_Type_Here();
		if (tibtype == TIBERIUM_NONE) {
			return;
		}

		TiberiumClass * tiberium = Tiberiums[tibtype];
		ColorScheme * scheme = ColorSchemes[tiberium->Color];

		ShapeSet const * tibshape = Tiberium_Overlay_Image(*this, *tiberium);
		if (tibshape == NULL || OverlayData >= tibshape->Get_Count() / 2) {
			return;
		}

		if (Ramp != RAMP_NONE) {
			Draw_Shape(*LogicalSurface, *scheme->Converter, tibshape, OverlayData, point, cliprect, (ShapeFlags_Type)(SHAPE_CENTER | SHAPE_WIN_REL | SHAPE_ALPHA | SHAPE_ZWRITE), 0, -2 - yoffset, ZGRAD_GROUND, NORMAL_LIGHT, SlopeZShapes[Ramp - 1]);
		} else {
			Draw_Shape(*LogicalSurface, *scheme->Converter, tibshape, OverlayData, point, cliprect, (ShapeFlags_Type)(SHAPE_CENTER | SHAPE_WIN_REL | SHAPE_ALPHA | SHAPE_ZWRITE), 0, -2 - yoffset, ZGRAD_GROUND, NORMAL_LIGHT);
		}
	} else if (otype->IsWall) {
		Draw_Shape(*LogicalSurface, *ColorSchemes[PlayerPtr->Scheme]->Converter, shape, OverlayData, point, cliprect, (ShapeFlags_Type)(SHAPE_CENTER | SHAPE_WIN_REL | SHAPE_ALPHA | SHAPE_ZWRITE), 0, -2 - yoffset, ZGRAD_90DEG, Brightness);
	} else if (otype->HeapID == OVERLAY_VEINS) {
		ConvertClass *drawer = ColorSchemes[PlayerPtr->Scheme]->Converter;
		if (Ramp != RAMP_NONE) {
			Draw_Shape(*LogicalSurface, *drawer, shape, OverlayData, point, cliprect, (ShapeFlags_Type)(SHAPE_CENTER | SHAPE_WIN_REL | SHAPE_ALPHA | SHAPE_ZWRITE), 0, -2 - yoffset, ZGRAD_GROUND, TileBrightness, SlopeZShapes[Ramp - 1], 0, Point2D(CELL_PIXEL_W, -2));
		} else {
			Draw_Shape(*LogicalSurface, *drawer, shape, OverlayData, point, cliprect, (ShapeFlags_Type)(SHAPE_CENTER | SHAPE_WIN_REL | SHAPE_ALPHA | SHAPE_ZWRITE), 0, -2 - yoffset, ZGRAD_GROUND, TileBrightness);
		}
	} else {
		int offsetY = (otype->IsDrawFlat) ? 0 : CELL_PIXEL_W / -2;
		if (otype->IsARock) {
			offsetY = 0;
		}
		Draw_Shape(*LogicalSurface, *Drawer, shape, OverlayData, point, cliprect, (ShapeFlags_Type)(SHAPE_CENTER | SHAPE_WIN_REL | SHAPE_ALPHA | SHAPE_ZWRITE), 0, offsetY - yoffset - 2, otype->IsDrawFlat ? ZGRAD_GROUND : ZGRAD_90DEG, TileBrightness);
	}
}


/// <summary>
/// Fetches the screen rectangle this cell's overlay occupies.
/// This routine is used by the tactical map when working out how much of the display an
/// overlay redraw will dirty. Tiberium resolves to the growth variety this particular cell
/// shows, so that the rectangle agrees with the artwork that will actually be drawn.
/// </summary>
/// <returns>Returns with the screen rectangle the overlay draws into. An empty rectangle is
/// returned if the cell has no overlay.</returns>
Rect CellClass::Overlay_Render_Rect(void) const
{
	if (Overlay == OVERLAY_NONE) {
		return(Rect(0, 0, 0, 0));
	}

	ShapeSet const * image = NULL;
	OverlayTypeClass const * overlay = OverlayTypes[Overlay];

	if (overlay->IsTiberium) {
		TiberiumType tib = Tiberium_Type_Here();
		if (tib != TIBERIUM_NONE) {
			TiberiumClass * tiberium = Tiberiums[tib];
			image = Tiberium_Overlay_Image(*this, *tiberium);
		}
	} else {
		image = (ShapeSet const *)overlay->Get_Image_Data();
	}

	Point2D point;
	TacticalMap->Coord_To_Pixel(Coord_Whole(CellID), point);

	point += Overlay_Draw_Offset();
	point.X -= ISO_TILE_PIXEL_W / 2;

	if (image == NULL || (overlay->IsTiberium && OverlayData >= image->Get_Count() / 2)) {
		return(Rect(0, 0, 0, 0));
	}

	int width = image->Get_Width() / 2;
	int height = image->Get_Height() / 2;
	Point2D rect_point = point;
	rect_point -= Point2D(width, height);
	return(image->Get_Rect(OverlayData) + rect_point);
}


/// <summary>
/// Fetches the screen rectangle this cell's overlay shadow occupies.
/// This routine is used by the tactical map when working out how much of the display an
/// overlay redraw will dirty. It is the companion to Overlay_Render_Rect.
/// </summary>
/// <returns>Returns with the screen rectangle the shadow draws into. An empty rectangle is
/// returned if the cell has no overlay artwork.</returns>
Rect CellClass::Overlay_Shadow_Render_Rect(void) const
{
	ShapeSet const * image = NULL;

	if (Overlay != OVERLAY_NONE) {
		OverlayTypeClass const * overlay = OverlayTypes[Overlay];
		if (overlay->IsTiberium) {
			TiberiumType tibtype = Tiberium_Type_Here();
			if (tibtype == TIBERIUM_NONE) return(Rect(0, 0, 0, 0));
			ShapeSet const * tiberium_image = Tiberium_Overlay_Image(*this, *Tiberiums[tibtype]);
			if (tiberium_image == NULL || OverlayData >= tiberium_image->Get_Count() / 2) return(Rect(0, 0, 0, 0));
		}
		image = (ShapeSet const *)overlay->Get_Image_Data();
	}

	if (!image) {
		return(Rect(0, 0, 0, 0));
	}

	int frame = OverlayData + image->Get_Count() / 2;

	Point2D point;
	TacticalMap->Coord_To_Pixel(Coord_Whole(CellID), point);

	point += Overlay_Draw_Offset();
	point.X -= ISO_TILE_PIXEL_W / 2;

	if (IsBridgeDeck) {
		if (OverlayData >= OVERLAYDATA_BRIDGE_NS_FULL1 && OverlayData <= OVERLAYDATA_BRIDGE_NS_END2) {
			point += Point2D(-(ISO_TILE_PIXEL_W / 4), ISO_TILE_PIXEL_H / 4);
		}
	}

	return(image->Get_Rect(frame) + (point - Point2D(image->Get_Width() / 2, image->Get_Height() / 2)));
}


/// <summary>
/// Fetches the screen rectangle this cell's terrain occupies.
/// This routine is used by the tactical map when working out how much of the display a cell
/// redraw will dirty. Tile artwork that reaches up out of its own cell grows the rectangle
/// upward so that the overhang is covered too.
/// </summary>
/// <returns>Returns with the screen rectangle the cell draws into.</returns>
Rect CellClass::Cell_Render_Rect(void) const
{
	Point2D point;
	TacticalMap->Coord_To_Pixel(Coord_Whole(CellID), point);
	point.X += ISO_TILE_PIXEL_W / -2;
	point.Y += TacticalRect.Y - LEVEL_PIXEL_H * Height;

	Rect rect;
	rect.X = point.X;
	rect.Y = point.Y;
	rect.Width = ISO_TILE_PIXEL_W;
	rect.Height = ISO_TILE_PIXEL_H;

	if (ITType == ISOTILE_NONE) {
		return(rect);
	}

	if (ITType >= ISOTILE_FIRST && ITType < IsometricTileTypes.Count()) {
		IsometricTileTypeClass * itype = IsometricTileTypes[ITType];
		int subtile = SubTile;
		IsoTileSet const * image = (IsoTileSet const *)itype->Get_Image_Data();
		if (image == NULL) {
			return(Rect());
		}
		subtile = std::min(subtile, image->Tile_Count() - 1);
		IsoTileRecord const * tile = image->Fetch_Record_Pointer_Unsafe(subtile);
		if (tile->IsHasExtraData) {
			int dy = std::max(0, tile->Y - tile->ExtraY);
			rect.Y -= dy;
			rect.Height += dy;
		}
		return(rect);

	} else {
		DebugString("Invalid tile at (%d, %d)\n", CellID.X, CellID.Y);
		return(rect);
	}

}


/// <summary>
/// Fetches the pixel offset this cell's overlay is drawn at.
/// This routine takes the offset that belongs to the overlay type and adjusts it for the
/// cell's height level and, on a bridge deck, for the lift of the raised roadway.
/// </summary>
/// <returns>Returns with the offset to apply to the cell's draw position.</returns>
Point2D CellClass::Overlay_Draw_Offset(void) const
{
	Point2D offset = ::Overlay_Draw_Offset(Overlay);
	if (IsBridgeDeck) {
		offset.Y -= ISO_TILE_PIXEL_H / 2 + 1;
		if (OverlayData >= OVERLAYDATA_BRIDGE_NS_FULL1 && OverlayData <= OVERLAYDATA_BRIDGE_NS_END2) {
			offset.Y -= ISO_TILE_PIXEL_H / 2;
		}
	}
	offset.Y += TacticalRect.Y - LEVEL_PIXEL_H_1 * Height;
	offset += Point2D(ISO_TILE_PIXEL_W / 2, ISO_TILE_PIXEL_H / 2);
	return(offset);
}


/// <summary>
/// Wipes this cell's footprint out of the depth buffer.
/// This routine is used ahead of a cell redraw so that whatever the previous frame wrote to
/// the depth buffer here cannot reject the artwork that is about to be drawn over it.
/// </summary>
/// <param name="point">The pixel position of the cell.</param>
/// <param name="cliprect">The clipping rectangle to work within.</param>
void CellClass::Wipe_Depth(Point2D const & point, Rect const & cliprect)
{
	IsometricTileTypes[ISOTILE_CLEAR]->Draw_Tile(TileDrawers[0], 0, *LogicalSurface, point.X, point.Y, cliprect, Height, TileBrightness, 1, 0, false, true, false, false);
}


/// <summary>
/// Draws the shroud and the fog covering this cell.
/// This routine asks the tactical map which shroud and fog frames the cell currently needs and
/// then renders them into the alpha buffer. The fog pass is skipped when fog of war is not in
/// play, or once the player has been given the whole map.
/// </summary>
/// <param name="point">The pixel position to draw the cell at.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
void CellClass::Draw_Shroud_And_Fog(Point2D const & point, Rect const & cliprect)
{
	ShadowFrame = TacticalMap->Cell_Shadow(CellID, false);

	int shadow_frame = ShadowFrame;
	if (shadow_frame == -2) {
		shadow_frame = 15;
	} else if (shadow_frame == -1) {
		shadow_frame = 0;
	}

	Draw_Shroud_Or_Fog_Shape(point, cliprect, shadow_frame);

	FogFrame = TacticalMap->Cell_Shadow(CellID, true);

	if (!Scen->Special.IsFogOfWar || Session.ObiWan) {
		return;
	}

	int fog_frame = FogFrame;
	if (fog_frame == -2) {
		fog_frame = 15;
	} else if (fog_frame == -1) {
		fog_frame = 0;
	}

	Draw_Fog_Shape(point, cliprect, fog_frame);
}


/// <summary>
/// Draws the shadow cast by this cell's terrain tile.
/// This routine is called by the tactical map's terrain pass. Tile artwork that is not marked
/// as a shadow caster draws nothing.
/// </summary>
/// <param name="point">The pixel position to draw the cell at.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
void CellClass::Draw_Shadow_Cast(Point2D const & point, Rect const & cliprect)
{
	int subtile;
	IsometricTileTypeClass * isotype;

	Fetch_Icon(isotype, subtile, NULL, false);
	if (isotype->IsShadowCaster) {
		Point2D drawpoint = point;
		drawpoint.Y -= LEVEL_PIXEL_H_1 * Height;
		isotype->Draw_Shadow_Caster(subtile, LogicalSurface, drawpoint, cliprect, -2 - LEVEL_PIXEL_H_1 * (Height - 4));
	}
}


/// <summary>
/// Fetches the isometric tile artwork this cell displays.
/// This routine is the common lookup shared by the cell drawing routines. A cell with no tile
/// assigned to it falls back to the clear tile.
/// </summary>
/// <param name="ittype">Set to the isometric tile type this cell draws with.</param>
/// <param name="subtile">Set to the sub-tile within that tile type.</param>
/// <param name="icon">Set to the tile variation to draw.</param>
/// <param name="doicon">Should the tile variation be fetched as well?</param>
/// <remarks>The icon pointer is only written when doicon is true; pass NULL otherwise.</remarks>
inline void CellClass::Fetch_Icon(IsometricTileTypeClass *& ittype, int & subtile, int * icon, bool doicon) const
{
	if (ITType != ISOTILE_NONE) {
		ittype = IsometricTileTypes[ITType];
		subtile = SubTile;
		if (ittype->NumTileTypesInSet > 1) {
			if (ittype->Is_Randomized(SubTile)) {
				if (doicon) *icon = IsBridgeDamaged;
			} else {
				if (doicon) *icon = Clear_Icon(ITType, ittype->NumTileTypesInSet);
			}
		}
	} else {
		subtile = 0;
		ittype = IsometricTileTypes[TILE_CLEAR];
		if (doicon) *icon = Clear_Icon(TILE_CLEAR, ittype->NumTileTypesInSet);
	}
}


/***********************************************************************************************
 * CellClass::Draw_It -- Draws the cell imagery at the location specified.                     *
 *                                                                                             *
 *    This is the gruntwork cell rendering code. It draws the cell at the screen location      *
 *    specified. This routine doesn't draw any overlapping or occupying units. It only         *
 *    deals with the ground (cell) layer -- icon level.                                        *
 *                                                                                             *
 * INPUT:   x,y   -- The screen coordinates to render the cell imagery at.                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1994 JLB : Created.                                                                 *
 *   08/21/1994 JLB : Revised for simple template objects.                                     *
 *   11/01/1994 BRR : Updated placement cursor; draws actual object                            *
 *   11/14/1994 BRR : Added remapping code to show passable areas                              *
 *   12/02/1994 BRR : Added trigger display                                                    *
 *   12/11/1994 JLB : Mixes up clear terrain through pseudo-random table.                      *
 *   04/25/1995 JLB : Smudges drawn BELOW overlays.                                            *
 *   07/22/1996 JLB : Objects added to draw process.                                           *
 *=============================================================================================*/
void CellClass::Draw_It(Point2D const & xdrawpoint, Rect const & cliprect, bool objects) const
{
	if (!objects) {
		BStart(BENCH_CELL);

		if (Drawer == NULL) {
			((CellClass *)this)->Init_Drawer(NULL);
		}

		IsometricTileTypeClass * ittype = NULL;
		int	icon = 0;		// The icon number to use from the template set.
		int subtile = 0;

		/*
		**	Fetch a pointer to the template type associated with this cell.
		*/
		Fetch_Icon(ittype, subtile, &icon, true);

		CellCount++;

		Point2D drawpoint = xdrawpoint;
		drawpoint.Y -= LEVEL_PIXEL_H_1 * Height;

		/*
		**	This is the underlying terrain icon.
		*/
		if (ittype->Get_Image_Data()) {
			ittype->Draw_Tile(Drawer, subtile, *LogicalSurface, drawpoint.X, drawpoint.Y + TacticalRect.Y, cliprect, Height, TileBrightness, true, icon, false, false, false, 0);
		}

		/*
		**	Redraw any smudge.
		*/
		if (Smudge != SMUDGE_NONE) {
			SmudgeTypes[Smudge]->Draw_It(drawpoint + Point2D(ISO_TILE_PIXEL_W / 2, TacticalRect.Y) - cliprect.TopLeft, cliprect, SmudgeData, LEVEL_LEPTON_H * Height, CellID);
		}

		BEnd(BENCH_CELL);
	}
}


/// <summary>
/// Checks if this cell continues a wall of the type specified.
/// A gate structure stands in for the wall it is built into, so this routine answers true for
/// a gate that faces the right way as well as for the wall overlay itself. It is used by the
/// wall building and wall drawing code when deciding how a run of wall joins up.
/// </summary>
/// <param name="type">The wall overlay being matched, or OVERLAY_NONE to test for any wall.</param>
/// <param name="facing">The direction the wall runs, used to pick out a matching gate.</param>
/// <returns>bool; Does this cell continue the wall?</returns>
bool CellClass::Has_Wall_Or_Gate(OverlayType type, FacingType facing) const
{
	if (Overlay == type && Overlay != OVERLAY_NONE) {
		return(true);
	}
	if (type == OVERLAY_NONE) {
		if (Overlay == OVERLAY_GDI_WALL || Overlay == OVERLAY_NOD_WALL) {
			return(true);
		}
		return(false);
	}

	if (type == OVERLAY_GDI_WALL || type == OVERLAY_SANDBAG_WALL) {
		BuildingClass *obj = (BuildingClass *)OccupierPtr;
		while (obj != NULL) {
			if (obj->RTTI == RTTI_BUILDING) {
				BuildingTypeClass * btype = obj->Class;
				if (obj->Strength > 0) {
					if ((btype == Rule->GDIGateOne && (facing == FACING_E || facing == FACING_W)) ||
						(btype == Rule->GDIGateTwo && (facing == FACING_N || facing == FACING_S)) ||
						(btype == Rule->WallTower)) {
						return(true);
					}
				}
			}
			obj = (BuildingClass *)obj->Next;
		}
	}

	if (type == OVERLAY_NOD_WALL) {
		BuildingClass *obj = (BuildingClass *)OccupierPtr;
		while (obj != NULL) {
			if (obj->RTTI == RTTI_BUILDING) {
				BuildingTypeClass * btype = obj->Class;
				if (obj->Strength > 0) {
					if ((btype == Rule->NodGateOne && (facing == FACING_E || facing == FACING_W)) ||
						(btype == Rule->NodGateTwo && (facing == FACING_N || facing == FACING_S))) {
						return(true);
					}
				}
			}
			obj = (BuildingClass *)obj->Next;
		}
	}

	return(false);
}


/***********************************************************************************************
 * CellClass::Wall_Update -- Updates the imagery for wall objects in cell.                     *
 *                                                                                             *
 *    This routine will examine the cell and the adjacent cells to determine what the wall     *
 *    should look like with the cell. It will then update the wall's imagery value and flag    *
 *    the cell to be redrawn if necessary. This routine should be called whenever the wall     *
 *    or an adjacent wall is created or destroyed.                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1994 JLB : Created.                                                                 *
 *   09/19/1994 BWG : Updated to handle partially-damaged walls.                               *
 *=============================================================================================*/
void CellClass::Wall_Update(bool)
{
	static FacingType _offsets[FACING_COUNT / 2 + 1] = {FACING_N, FACING_E, FACING_S, FACING_W, FACING_NONE};

	for (unsigned index = 0; index < ARRAY_SIZE(_offsets); index++) {
		CellClass & newcell = Adjacent_Cell(_offsets[index]);
		newcell.Register_For_Redraw();

		if (newcell.Overlay != OVERLAY_NONE && OverlayTypes[newcell.Overlay]->IsWall) {
			int	icon = 0;

			/*
			**	Build the icon number according to walls located in the adjacent
			**	cells.
			*/
			for (unsigned i = 0; i < FACING_COUNT / 2; i++) {
				CellClass & adjacent = newcell.Adjacent_Cell(_offsets[i]);
				if (adjacent.Has_Wall_Or_Gate(newcell.Overlay, _offsets[i])) {
					icon |= 1 << i;
				}
			}
			newcell.OverlayData = (newcell.OverlayData & 0xFFF0) | icon;

			/*
			**	Handle special cases for the incomplete damaged wall sets. If a damage stage
			**	is calculated, but there is no artwork for it, then consider the wall to be
			**	completely destroyed.
			*/
			bool recalc_around = false;
			PassabilityType old_passability = newcell.Passability;

			if ((newcell.Overlay == OVERLAY_BRICK_WALL || newcell.Overlay == OVERLAY_NOD_WALL) && (newcell.OverlayData == 3 * OVERLAYDATA_WALL_DAMAGE_STAGE || newcell.OverlayData == 2 * OVERLAYDATA_WALL_DAMAGE_STAGE)) {
				newcell.Overlay = OVERLAY_NONE;
				newcell.OverlayData = 0;
				newcell.Owner = HOUSE_NONE;
				Detach_This_From_All(&newcell, true);
				recalc_around = true;
			}
			if (newcell.Overlay == OVERLAY_SANDBAG_WALL && (newcell.OverlayData == OVERLAYDATA_WALL_DAMAGE_STAGE || newcell.OverlayData == 2 * OVERLAYDATA_WALL_DAMAGE_STAGE)) {
				newcell.Overlay = OVERLAY_NONE;
				newcell.OverlayData = 0;
				newcell.Owner = HOUSE_NONE;
				Detach_This_From_All(&newcell, true);
				recalc_around = true;
			}
			if (newcell.Overlay == OVERLAY_CYCLONE_WALL && newcell.OverlayData == 2 * OVERLAYDATA_WALL_DAMAGE_STAGE) {
				newcell.Overlay = OVERLAY_NONE;
				newcell.OverlayData = 0;
				newcell.Owner = HOUSE_NONE;
				Detach_This_From_All(&newcell, true);
				recalc_around = true;
			}
			if (newcell.Overlay == OVERLAY_FENCE && (newcell.OverlayData == OVERLAYDATA_WALL_DAMAGE_STAGE || newcell.OverlayData == 2 * OVERLAYDATA_WALL_DAMAGE_STAGE)) {
				newcell.Overlay = OVERLAY_NONE;
				newcell.OverlayData = 0;
				newcell.Owner = HOUSE_NONE;
				Detach_This_From_All(&newcell, true);
				recalc_around = true;
			}
			if (newcell.Overlay == OVERLAY_BARBWIRE_WALL && newcell.OverlayData == OVERLAYDATA_WALL_DAMAGE_STAGE) {
				newcell.Overlay = OVERLAY_NONE;
				newcell.OverlayData = 0;
				newcell.Owner = HOUSE_NONE;
				Detach_This_From_All(&newcell, true);
				recalc_around = true;
			}

			newcell.Recalc_Attributes();

			if (newcell.Passability != old_passability) {
				if (recalc_around) {
					Map.Update_Cell_Zone(newcell.CellID);
					Map.Update_Cell_Subzones(newcell.CellID);
					for (int dir = FACING_FIRST; (unsigned)dir < FACING_COUNT; dir++) {
						CellClass & adjacent = Map[::Adjacent_Cell(newcell.CellID, (FacingType)dir)];
						adjacent.AdjacentObjectCount--;
					}
				} else {
					Map.Update_Cell_Zone_Constructively(newcell.CellID);
					Map.Update_Cell_Subzones(newcell.CellID);
				}
			}
		}
	}
}


/***********************************************************************************************
 * CellClass::Cell_Coord -- Returns the coordinate of this cell.                               *
 *                                                                                             *
 *    This support function will determine the coordinate of this cell and return it.          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with coordinate value of cell.                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
Coord CellClass::Cell_Coord(void) const
{
	Coord coord(Fetch_CellID());
	coord.Z = Get_Height();
	return(coord);
}


/***********************************************************************************************
 * CellClass::Reduce_Tiberium -- Reduces the tiberium in the cell by the amount specified.     *
 *                                                                                             *
 *    This routine will lower the tiberium level in the cell. It is used by the harvesting     *
 *    process as well as by combat damage to the tiberium fields.                              *
 *                                                                                             *
 * INPUT:   levels   -- The number of levels to reduce the tiberium.                           *
 *                                                                                             *
 * OUTPUT:  bool; Was the tiberium level reduced by at least one level?                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int CellClass::Reduce_Tiberium(int levels)
{
	Rect dirty = Union(Overlay_Render_Rect(), Overlay_Shadow_Render_Rect());
	dirty.Y -= TacticalRect.Y;

	TiberiumType tibtype = Tiberium_Type_Here();
	int reducer = levels;

	if (levels > 0 && tibtype != TIBERIUM_NONE) {
		TiberiumClass * tiberium = Tiberiums[tibtype];
		if (OverlayData == OVERLAYDATA_TIBERIUM_MAX) {
			tiberium->Queue_Growth(CellID);
		}
		if (OverlayData+1 > levels) {
			OverlayData -= levels;
			reducer = levels;
		} else {
			PassabilityType passability = Passability;
			Overlay = OVERLAY_NONE;
			reducer = OverlayData;
			OverlayData = 0;
			Recalc_Attributes();
			if (passability != Passability) {
				Map.Update_Cell_Zone(CellID);
				Map.Update_Cell_Subzones(CellID);
			}
			Map.Flag_Background_Update(CellID);
			tiberium->Clear_Spread_State(CellID);
			for (int facing = FACING_FIRST; facing < FACING_COUNT; facing++) {
				Cell adjacent = (Cell)::Adjacent_Cell(CellID, (FacingType)facing);
				if (Map.In_Radar(adjacent) && !tiberium->SpreadState[Map_Cell_Index(adjacent)]) {
					tiberium->Queue_Spread(adjacent);
				}
			}
		}
		TacticalMap->Register_Dirty_Area(dirty);
		return(reducer);
	}
	return(0);
}


/***********************************************************************************************
 * CellClass::Reduce_Wall -- Damages a wall, if damage is high enough.                         *
 *                                                                                             *
 *    This routine will change the wall shape used for a wall if it's damaged.                 *
 *                                                                                             *
 * INPUT:   damage   -- The number of damage points the wall was hit with. If this value is    *
 *                      -1, then the entire wall at this cell will be destroyed.               *
 *                                                                                             *
 * OUTPUT:  bool; Was the wall destroyed?                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/15/1995 BWG : Created.                                                                 *
 *   03/19/1995 JLB : Updates cell information if wall was destroyed.                          *
 *   10/06/1996 JLB : Updates zone as necessary.                                               *
 *=============================================================================================*/
int CellClass::Reduce_Wall(int damage)
{
	if (Overlay != OVERLAY_NONE) {
		bool destroyed = false;
		OverlayTypeClass const * wall = OverlayTypes[Overlay];

		if (wall->IsWall) {

			/*
			**	If the damage was great enough to ensure wall destruction, reduce the wall by one
			**	level (no more). Otherwise determine wall reduction based on a percentage chance
			**	proportional to the damage received and the wall's strength.
			*/
			if (damage == -1 || damage >= wall->DamagePoints || ScenarioInit) {
				destroyed = true;
			} else {
				destroyed = Random_Pick(0, wall->DamagePoints) < damage;
			}

			/*
			**	If the wall is destroyed, destroy it and check for any adjustments to
			**	adjacent walls.
			*/
			if (destroyed) {
				Rect rect = Union(Overlay_Render_Rect(), Overlay_Shadow_Render_Rect());
				rect.Y -= TacticalRect.Y;
				TacticalMap->Register_Dirty_Area(rect);
				OverlayData+=OVERLAYDATA_WALL_DAMAGE_STAGE;

				if ((OverlayData>>OVERLAYDATA_WALL_DAMAGE_SHIFT) == wall->DamageLevels-1 && wall->DamageLevels > 2) {
					for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir = FacingType(dir + FACING_90)) {
						CellClass * adjacent = (CellClass *)&Adjacent_Cell(dir);
						if (adjacent->Overlay != OVERLAY_NONE && OverlayTypes[adjacent->Overlay]->IsWall && adjacent->Overlay == Overlay && adjacent->OverlayData < OVERLAYDATA_WALL_DAMAGE_STAGE) {
							adjacent->Reduce_Wall(200);
						}
					}
				}

				if (damage == -1 ||
					(OverlayData>>OVERLAYDATA_WALL_DAMAGE_SHIFT) >= wall->DamageLevels ||
					((OverlayData>>OVERLAYDATA_WALL_DAMAGE_SHIFT) == wall->DamageLevels-1 && (OverlayData & OVERLAYDATA_WALL_FRAME_MASK)==0)	) {

					Owner = HOUSE_NONE;
					Overlay = OVERLAY_NONE;
					OverlayData = 0;
					Recalc_Attributes();
					Map.Update_Cell_Zone(CellID);
					Map.Update_Cell_Subzones(CellID);
					Map.Radar_Background(CellID);
					Adjacent_Cell(FACING_N).Wall_Update();
					Adjacent_Cell(FACING_W).Wall_Update();
					Adjacent_Cell(FACING_S).Wall_Update();
					Adjacent_Cell(FACING_E).Wall_Update();
					Detach_This_From_All(this);

					for (int dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
						Adjacent_Cell((FacingType)dir).AdjacentObjectCount--;
					}
					return(true);
				}
			}
		}
	}
	return(false);
}


/***********************************************************************************************
 * CellClass::Spot_Index -- returns cell sub-coord index for given COORDINATE                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      coord      COORDINATE to compute index for                                             *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      index into StoppingCoord that's closest to this coord                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/1994 BR : Created.                                                                  *
 *   12/10/1994 JLB : Uses alternate sub-position algorithm.                                   *
 *=============================================================================================*/
int CellClass::Spot_Index(Coord const & coord)
{
	Point2D rel = Coord_Fraction(coord);		// Sub coordinate value within cell.

	/*
	**	If the coordinate is close enough to the center of the cell, then return
	**	the center position index.
	*/
	if (Distance(rel, Point2D(CELL_LEPTON/2,CELL_LEPTON/2)) < 60) {
		return(0);
	}

	/*
	**	Since the center cell position has been eliminated, a simple comparison
	**	as related to the center of the cell can be used to determine the sub
	**	position. Take advantage of the fact that the sub positions are organized
	**	from left to right, top to bottom.
	*/
	int index = 0;
	if (rel.X > CELL_LEPTON/2) index |= 0x01;
	if (rel.Y > CELL_LEPTON/2) index |= 0x02;
	if (index == 0) return(0);
	return(index+1);
}


/// <summary>
/// Determines if one of this cell's occupancy spots is vacant.
/// A cell is divided into spots so that several infantry can share it. Use this routine when
/// looking for somewhere in the cell to put a passenger down.
/// </summary>
/// <param name="spot_index">The occupancy spot to examine.</param>
/// <param name="bridge">Should the bridge deck be examined rather than the ground?</param>
/// <returns>bool; Is the spot free to be occupied?</returns>
bool CellClass::Is_Spot_Free(int spot_index, bool bridge) const
{
	if (spot_index == 0 || spot_index == 1) {
		return(false);
	}

	if (bridge) {
		return(! (BridgeFlag.Composite & (1 << spot_index)) );
	}
	else {
		return(! (Flag.Composite & (1 << spot_index)) );
	}
}


/***********************************************************************************************
 * CellClass::Closest_Free_Spot -- returns free spot closest to given coord                    *
 *                                                                                             *
 * Similar to the CellClass::Free_Spot; this routine finds the spot in                         *
 * the cell closest to the given coordinate, and returns the COORDINATE of                     *
 * that spot if it's available, NULL if it's not.                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *  coord   coordinate to check (only sub cell position examined)                              *
 *                                                                                             *
 *          any   -- If only the closest spot is desired regardless of whether it is free or   *
 *                   not, then this parameter will be true.                                    *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *  COORDINATE of free spot, NULL if none. The coordinate return value does not alter the cell *
 *             coordinate data portions of the coordinate passed in. Only the lower sub-cell   *
 *             data is altered.                                                                *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *  none.                                                                                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/08/1994 BR : Created.                                                                  *
 *   12/10/1994 JLB : Picks best of closest stopping positions.                                *
 *   12/21/1994 JLB : Adds a mix-up factor if center location is occupied.                     *
 *=============================================================================================*/
Coord CellClass::Closest_Free_Spot(Coord const & xcoord, bool any, bool bridge) const
{
	int spot_index = Spot_Index(xcoord);

	/*
	**	This precalculated sequence table records the closest spots to any given spot. Sequential
	**	examination of these spots for availability ensures that the closest available one is
	**	discovered first.
	*/
	static unsigned char _sequence[5][4] = {
		{1,2,3,4},
		{0,2,3,4},
		{0,1,4,3},
		{0,1,4,2},
		{0,2,3,1}
	};

	/*
	**	In the case of the center coordinate being requested, but is occupied, then all other
	**	sublocations are equidistant. Instead of picking a static sequence of examination, the
	**	order is mixed up by way of this table.
	*/
	static unsigned char _alternate[4][4] = {
		{1,2,3,4},
		{2,3,4,1},
		{3,4,1,2},
		{4,1,2,3},
	};

	Coord coord = Coord_Whole(xcoord);

	/*
	**	Cells occupied by buildings or vehicles don't have any free spots.
	*/

	bool is_vehicle_occupied;
	if (bridge) {
		is_vehicle_occupied = BridgeFlag.Occupy.Vehicle;
	} else {
		is_vehicle_occupied = Flag.Occupy.Vehicle;
	}

	if (!any) {
		if (is_vehicle_occupied || (Flag.Occupy.Monolith && (!Get_Gate() || !Get_Gate()->Is_Gate_Open()))) {
			return(COORD_NONE);
		}

	}

	/*
	**	If just the nearest position is desired regardless of whether occupied or not,
	**	then just return with the stopping coordinate value.
	*/
	if (any || Is_Spot_Free(spot_index, bridge)) {
		coord += StoppingCoordAbs[spot_index];
		coord.Z = Map.Get_Height_GL(xcoord);
		if (bridge) {
			coord.Z += BRIDGE_LEPTON_HEIGHT;
		}
		return(coord);
	}

	/*
	**	Scan through all available sub-locations in the cell in order to determine
	**	the closest one to the coordinate requested. Use precalculated table so that
	**	when the first free position is found, bail.
	*/
	unsigned char * sequence;
	if (spot_index == 0) {
		sequence = &_alternate[Random_Pick(0, 3)][0];
	} else {
		sequence = &_sequence[spot_index][0];
	}
	for (int index = 0; index < 4; index++) {
		int pos = *sequence++;

		if (Is_Spot_Free(pos, bridge)) {
			coord += StoppingCoordAbs[pos];
			coord.Z = Map.Get_Height_GL(xcoord);
			if (bridge) {
				coord.Z += BRIDGE_LEPTON_HEIGHT;
			}
			return(coord);
		}
	}

	/*
	**	No free spot could be found so return a NULL coordinate.
	*/
	return(COORD_NONE);
}


/// <summary>
/// Fetches the gate building that occupies this cell.
/// This routine is used when a unit needs to know whether the structure barring its way is
/// something that might open for it.
/// </summary>
/// <returns>Returns with a pointer to the gate in this cell, or NULL if there is none.</returns>
BuildingClass * CellClass::Get_Gate(void) const
{
	BuildingClass * building = Cell_Building();
	if (building != NULL && !building->Class->IsGate) {
		building = NULL;
	}
	return(building);
}


/***********************************************************************************************
 * CellClass::Clear_Icon -- Calculates what the clear icon number should be.                   *
 *                                                                                             *
 *    This support routine will determine what the clear icon number would be for the cell.    *
 *    The icon number is determined by converting the cell number into an index into a         *
 *    lookup table. This yields what appears to be a randomized map without the necessity of   *
 *    generating and recording randomized map numbers.                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the icon number for clear terrain if it were displayed at the         *
 *          cell.                                                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *   06/09/1995 JLB : Uses 16 entry scramble algorithm.                                        *
 *=============================================================================================*/
int CellClass::Clear_Icon(IsometricTileType type, int seq_length) const
{
	static bool _init = false;
	static int _small_pattern[16] = { 0, 1, 2, 3, 3, 2, 1, 0, 2, 3, 0, 1, 1, 0, 3, 2 };
	static int _large_pattern[64];
	static Point2D _adjacent_icon[8] = {
		{-1,-1}, {0,-1}, {1,-1}, {-1,0}, {1,0}, {-1,1}, {0,1}, {1,1}
	};

	if (!_init) {
		_init = true;
		memset(_large_pattern, 0xFF, sizeof(_large_pattern));

		for (int i = 0; i < ARRAY_SIZE(_large_pattern); ++i) {
			int random = -1;
			for (int j = 0; j < ARRAY_SIZE(_large_pattern); j++) {
				random = NonCriticalRandomNumber() & 7;
				bool duplicate = false;

				for (int k = 0; k < ARRAY_SIZE(_adjacent_icon); ++k) {
					int num = i + _adjacent_icon[k].X + _adjacent_icon[k].Y * 8;

					if (num < 0) num += 64;
					if (num >= 64) num -= 64;

					if (_large_pattern[num] == random) {
						duplicate = true;
						break;
					}
				}

				if (!duplicate) {
					break;
				}
			}
			_large_pattern[i] = random;
		}
	}

	Cell cell = Fetch_CellID();
	if (SubTile != 0) {
		IsometricTileTypeClass const * isotype = IsometricTileTypes[type];
		if (isotype->Get_Image_Data() == NULL) {
			return(0);
		}
		Point2D p = IsoTile_Dimensions(isotype);
		cell = cell - Cell(SubTile % p.X, SubTile / p.X);
		cell.X /= p.X;
		cell.Y /= p.Y;
	}
	if (seq_length <= 4) {
		return(_small_pattern[(cell.X & 0x03) | ((cell.Y & 0x03) << 2)]);
	} else {
		return(_large_pattern[(cell.X & 0x07) + ((cell.Y & 0x07) << 3)]);
	}
}


/***********************************************************************************************
 * CellClass::Incoming -- Causes objects in cell to "run for cover".                           *
 *                                                                                             *
 *    This routine is called whenever a great, but slow moving, threat is presented to the     *
 *    occupants of a cell. The occupants will, in most cases, stop what they are doing and     *
 *    try to get out of the way.                                                               *
 *                                                                                             *
 * INPUT:   threat      -- The coordinate source of the threat.                                *
 *                                                                                             *
 *          forced      -- If this threat is so major that the occupants should stop what      *
 *                         they are doing, then this parameter should be set to true.          *
 *                                                                                             *
 *          nokidding   -- Override the scatter to also affect human controlled objects.       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/10/1995 JLB : Created.                                                                 *
 *   08/02/1996 JLB : Added the "nokidding" parameter.                                         *
 *=============================================================================================*/
void CellClass::Incoming(Coord const & threat, bool forced, bool nokidding, bool bridge)
{
	ObjectClass * object = NULL;
	TechnoClass * techno = NULL;

	object = Cell_Occupier(bridge);
	bool elite = false;
	if (!nokidding) {
		while (object != NULL) {
			techno = Dynamic_Cast<TechnoClass *>(object);
			if (techno != NULL && techno->Veterancy.Is_Elite()) {
				elite = true;
				break;
			}
			object = object->Next;
		}
	}

	object = Cell_Occupier(bridge);

	DynamicVectorClass<ObjectClass *> objs;
	while (object != NULL) {
		objs.Add(object);
		object = object->Next;
	}

	for (int i = 0; i < objs.Count(); i++) {

		/*
		**	Special check to make sure that friendly units never scatter.
		*/
		techno = Dynamic_Cast<TechnoClass *>(objs[i]);
		if (elite || nokidding || Rule->IsScatter ||
			(techno != NULL && ((techno->Has_Ability(ABILITY_SCATTER) || techno->House->IQ >= Rule->IQScatter)))) {
			objs[i]->Scatter(threat, forced, nokidding);
		}
	}
}


/***********************************************************************************************
 * CellClass::Adjacent_Cell -- Determines the adjacent cell according to facing.               *
 *                                                                                             *
 *    Use this routine to return a reference to the adjacent cell in the direction specified.  *
 *                                                                                             *
 * INPUT:   face  -- The direction to use when determining the adjacent cell.                  *
 *                                                                                             *
 * OUTPUT:  Returns with a reference to the adjacent cell.                                     *
 *                                                                                             *
 * WARNINGS:   If the facing value is invalid, then a reference to the same cell is returned.  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
CellClass const & CellClass::Adjacent_Cell(FacingType face) const
{
	if ((unsigned)face >= FACING_COUNT) {
		return(*this);
	}

	Cell newcell = ::Adjacent_Cell(Fetch_CellID(), face);
	return(Map[newcell]);
}


/***************************************************************************
 * CellClass::Adjust_Threat -- Allows adjustment of threat at cell level   *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   04/24/1995 PWG : Created.                                             *
 *=========================================================================*/
void CellClass::Adjust_Threat(HousesType house, int threat_value)
{
	int region = Map.Cell_Region(Fetch_CellID());

	for (HousesType lp = HOUSE_FIRST; lp < Houses.Count(); lp ++) {
		HouseClass * house_ptr = Houses[lp];
		if (house_ptr && house_ptr->HeapID != house && (!house_ptr->Is_Human_Player() || !house_ptr->Is_Ally(house))) {
			house_ptr->Adjust_Threat(region, threat_value);
		}
	}
	//if (Debug_Threat) {
	//	Map.Flag_To_Redraw();
	//}
}


/***********************************************************************************************
 * CellClass::Tiberium_Adjust -- Adjust the look of the Tiberium for smoothing purposes.       *
 *                                                                                             *
 *    This routine will adjust the level of the Tiberium in the cell so that it will           *
 *    smoothly blend with the adjacent Tiberium. This routine should only be called for        *
 *    new Tiberium cells. Existing cells that contain Tiberium follow a different growth       *
 *    pattern.                                                                                 *
 *                                                                                             *
 * INPUT:   pregame  -- Is this a pregame call? Such a call will mixup the Tiberium overlay    *
 *                      used.                                                                  *
 *                                                                                             *
 * OUTPUT:  Returns with the added Tiberium value that is now available for harvesting.        *
 *                                                                                             *
 * WARNINGS:   The return value is only valid for the initial placement. Tiberium growth will  *
 *             increase the net worth of the existing Tiberium.                                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/16/1995 JLB : Created.                                                                 *
 *   02/20/1996 JLB : Takes into account the ore type.                                         *
 *=============================================================================================*/
int CellClass::Tiberium_Adjust(bool pregame)
{
	if (Overlay != OVERLAY_NONE) {
		static int _adj[13] = {0,1,3,4,6,7,8,10,11,7,0,1,6};

		TiberiumType tibtype = Tiberium_Type_Here();
		if (tibtype != TIBERIUM_NONE) {
			TiberiumClass * tiberium = Tiberiums[tibtype];
			int	count = 0;

			/*
			**	Mixup the Tiberium overlays so that they don't look the same.
			**	Since the type of ore is known, also record the nominal
			**	value per step of that ore type.
			*/
			int value = tiberium->CreditValue;
			if (pregame) {
				int first = tiberium->Overlay->HeapID;
				Overlay = (OverlayType)Random_Pick(first, first + tiberium->Variety - 1);
			}

			/*
			**	Add up all adjacent cells that contain tiberium.
			** (Skip those cells which aren't on the map)
			*/
			for (FacingType face = FACING_FIRST; face < FACING_COUNT; face++) {
				CellClass & adjc = Adjacent_Cell(face);

				if (adjc.Tiberium_Type_Here() == tibtype) {
					count++;
				}
			}

			OverlayData = _adj[count % Tiberiums[tibtype]->FrameCount];
			return((OverlayData+1) * value);
		}
	}
	return(0);
}


/***********************************************************************************************
 * CellClass::Goodie_Check -- Performs crate discovery logic.                                  *
 *                                                                                             *
 *    Call this routine whenever an object enters a cell. It will check for the existence      *
 *    of a crate and generate any "goodie" it might contain.                                   *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object that is triggering this crate.                   *
 *                                                                                             *
 * OUTPUT:  Can the object continue to enter this cell? A false return value means that the    *
 *          cell is now occupied and must not be entered.                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/22/1995 JLB : Created.                                                                 *
 *   07/08/1995 JLB : Added a bunch of goodies to the crates.                                  *
 *   06/17/1996 JLB : Revamped for Red Alert                                                   *
 *=============================================================================================*/
bool CellClass::Goodie_Check(FootClass * object)
{
	if (object != NULL && Overlay != OVERLAY_NONE && OverlayTypes[Overlay]->IsCrate) {
		bool force_mcv = false;
		int force_money = 0;
		int damage;
		Coord coord;

		if (Session.Type != GAME_NORMAL && object->House->Class->IsMultiplayPassive) {
			return(true);
		}

		if (OverlayTypes[Overlay]->IsCrateTrigger) {
			if (object->Tag != NULL) {
				DebugString("Springing trigger on crate at %d,%d\n", CellID.X, CellID.Y);
				object->Tag->Spring(TEVENT_PICKUP_CRATE, object);
				if (!object->IsActive) {
					return(false);
				}
			}
			Scen->IsCrateBeenPickedUp = true;
		}

		/*
		**	Determine the total number of shares for all the crate powerups. This is used as
		**	the base pool to determine the odds from.
		*/
		int total_shares = 0;
		int index;
		for (index = CRATE_FIRST; index < CRATE_COUNT; index++) {
			total_shares += CrateShares[index];
		}

		/*
		**	Pick a random crate powerup according to the shares allotted to each powerup.
		**	In solo play, the bonus item is dependant upon the rules control.
		*/
		CrateType powerup = CRATE_MONEY;
		if (Session.Type == GAME_NORMAL) {

			/*
			**	Solo play has money amount determined by rules.ini file.
			*/
			force_money = Rule->SoloCrateMoney;

			if (OverlayTypes[Overlay] == Rule->CrateImg) {
				powerup = Rule->SilverCrate;
			}

			if (OverlayTypes[Overlay] == Rule->WoodCrateImg) {
				powerup = Rule->WoodCrate;
			}

#if NEVER
			if (OverlayTypes[Overlay] == Rule->WaterCrateImg) {
				powerup = Rule->WaterCrate;
			}
#endif

		} else {
			int pick = Random_Pick(1, total_shares);

			int share_count = 0;
			for (powerup = CRATE_FIRST; powerup < CRATE_COUNT; powerup++) {
				share_count += CrateShares[powerup];
				if (pick <= share_count) break;
			}
			assert(powerup != CRATE_COUNT);

			/*
			**	Possibly force it to be an MCV if there is
			**	sufficient money and no buildings left.
			*/
			if (object->House->CurBuildings == 0 &&
					object->House->Available_Money() > 1500 &&
					object->House->Count_Owned(object->House->UQuantity, Rule->BaseUnit) == 0 &&
					Session.Options.Bases) {
				powerup = CRATE_UNIT;
				force_mcv = true;
			}

			/*
			**	Depending on what was picked, there might be an alternate goodie if the selected
			**	goodie would have no effect.
			*/
			switch (powerup) {
				case CRATE_UNIT:
					if (object->House->CurUnits > 50) powerup = CRATE_MONEY;
					break;

				case CRATE_SQUAD:
					if (object->House->CurInfantry > 100) powerup = CRATE_MONEY;
					break;

				case CRATE_DARKNESS:
					//if (object->House->IsGPSActive) powerup = CRATE_MONEY;
					break;

				case CRATE_ARMOR:
					if (object->ArmorBias != 1) powerup = CRATE_MONEY;
					break;

				case CRATE_SPEED:
					if (object->SpeedBias != 1 || object->RTTI == RTTI_AIRCRAFT) powerup = CRATE_MONEY;
					break;

				case CRATE_FIREPOWER:
					if (object->FirepowerBias != 1 || !object->Is_Weapon_Equipped()) powerup = CRATE_MONEY;
					break;

				case CRATE_REVEAL:
//					if (object->House->IsVisionary) {
//						if (object->House->IsGPSActive) {
//							powerup = CRATE_MONEY;
//						} else {
//							powerup = CRATE_DARKNESS;
//						}
//					}
					break;

				case CRATE_CLOAK:
					if (object->IsCloakable) powerup = CRATE_MONEY;
					break;

				case CRATE_HEAL_BASE:
//					if (object->House->BScan == 0) powerup = CRATE_UNIT;
					break;

				case CRATE_MONEY:
					break;
			}
			/*
			**	If the powerup is money but there is insufficient money to build a refinery but there is a construction
			**	yard available, then force the money to be enough to rebuild the refinery.
			*/
#if 0
			if (powerup == CRATE_MONEY && (object->House->BScan & (STRUCTF_CONST|STRUCTF_REFINERY)) == STRUCTF_CONST &&
						object->House->Available_Money() < BuildingTypeClass::As_Reference(STRUCT_REFINERY).Cost * object->House->CostBias) {

				force_money = BuildingTypeClass::As_Reference(STRUCT_REFINERY).Cost * object->House->CostBias;
			}

			/*
			**	Special override for water crates so that illegal goodies items
			**	won't appear.
			*/
			if (Overlay == OVERLAY_WATER_CRATE) {
				switch (powerup) {
					case CRATE_UNIT:
					case CRATE_SQUAD:
						powerup = CRATE_MONEY;
						break;

					default:
						break;
				}
			}
#endif
		}

		/*
		**	Keep track of the number of each type of crate found
		*/
		if (Session.Type == GAME_INTERNET) {
			object->House->TotalCrates->Increment_Unit_Total(powerup);
		}

		/*
		**	Remove the crate from the map.
		*/
		Map.Remove_Crate(Fetch_CellID());
//		Map[CellID].Overlay = OVERLAY_NONE;

		if (Session.Type != GAME_NORMAL && Rule->IsMPCrates && Session.Options.Goodies) {
			Map.Place_Random_Crate();
		}

		if (powerup == CRATE_SQUAD) {
			powerup = CRATE_MONEY;
		}

		/*
		**	Generate any corresponding animation associated with this crate powerup.
		*/
//		if (CrateAnims[powerup] != ANIM_NONE) {
//			new AnimClass(AnimTypes[CrateAnims[powerup]], Cell_Coord());
//		}

		/*
		**	Create the effect requested.
		*/
		double data = CrateData[powerup];
		bool tospeak = false;
		switch (powerup) {

			/*
			 * A cloud of poison gas damages the crate cell and all adjacent cells.
			 */
			case CRATE_GAS: {
				DebugString("Crate at %d,%d contains poison gas\n", CellID.X, CellID.Y);
				WarheadTypeClass * gas = WarheadTypeClass::From_Name("GAS");
				damage = data;
				Explosion_Damage(Center_Coord(), damage, NULL, gas, false);
				for (FacingType facing = FACING_N; facing < FACING_COUNT; facing++) {
					Explosion_Damage(Adjacent_Cell(facing).Center_Coord(), damage, NULL, gas, false);
				}
				break;
			}

			/*
			 * A patch of tiberium is created at and scattered around the crate.
			 */
			case CRATE_TIBERIUM: {
				DebugString("Crate at %d,%d contains tiberium\n", CellID.X, CellID.Y);
				TiberiumType tibtype = (TiberiumType)Random_Pick(0, Tiberiums.Count()-1);
				if (tibtype == TIBERIUM_CRUENTUS) {
					tibtype = TIBERIUM_RIPARIUS;
				}
				Place_Tiberium(tibtype, 1);
				int count = Random_Pick(10, 20);
				while (count--) {
					Coord scatter = Coord_Scatter(Center_Coord(), Random_Pick(0, 3 * CELL_LEPTON), true);
					Map[scatter].Place_Tiberium(tibtype, 1);
				}
				break;
			}

			/*
			**	Shroud the world in blackness.
			*/
			case CRATE_DARKNESS:
				DebugString("Crate at %d,%d contains 'shroud'\n", CellID.X, CellID.Y);
				if (object->House->Is_Player_Control()) {
					Map.Shroud_The_Map();
				}
				break;

			/*
			**	Reveal the entire map.
			*/
			case CRATE_REVEAL:
				DebugString("Crate at %d,%d contains 'reveal'\n", CellID.X, CellID.Y);
				if (object->House->Is_Player_Control()) {
					Map.Reveal_The_Map();
				}
				break;

			/*
			**	Try to create a unit where the crate was.
			*/
			case CRATE_UNIT: {
				DebugString("Crate at %d,%d contains a unit\n", CellID.X, CellID.Y);
				UnitTypeClass const * utp = NULL;

				/*
				**	Give the player an MCV if he has no base left but does have more than enough
				**	money to rebuild a new base. Of course, if he already has an MCV, then don't
				**	give him another one.
				*/
				if (force_mcv) {
					utp = object->House->Get_Preferred(Rule->BaseUnit);
				}

				/*
				**	If the player has a base and a refinery, but no harvester, then give him
				**	a free one.
				*/
				if (utp == NULL && object->House->Owns_Any(object->House->BQuantity, Rule->BuildRefinery) && object->House->Count_Owned(object->House->UQuantity, Rule->HarvesterUnit) == 0) {
					utp = object->House->Get_Preferred(Rule->HarvesterUnit);
				}

				/*
				**	Check for special unit type override value.
				*/
				if (Rule->UnitCrateType != NULL) {
					utp = Rule->UnitCrateType;
				}

				/*
				**	If no unit type has been determined, then pick one at random.
				*/
				auto qualifies = [&](UnitTypeClass const * candidate) {
					return candidate->IsCrateGoodie && (candidate->Ownable & object->Owner_HouseClass()->Acted_Mask()) != 0 && (Session.Options.Bases || !Rule->BaseUnit.Is_In_List(candidate));
				};
				bool any_goodie = false;
				for (int index = UNIT_FIRST; index < UnitTypes.Count() && !any_goodie; index++) {
					any_goodie = qualifies(UnitTypes[index]);
				}

				// Redrawing with nothing to draw would never end.
				while (utp == NULL && any_goodie) {
					utp = UnitTypes[Random_Pick(UNIT_FIRST, (UnitType)(UnitTypes.Count()-1))];
					if (!qualifies(utp)) {
						utp = NULL;
					}
				}

				if (utp != NULL) {
					UnitClass * goodie_unit = (UnitClass *)utp->Create_One_Of(object->House);
					if (goodie_unit != NULL) {
						if (goodie_unit->Unlimbo(Cell_Coord())) {
							return(false);
						}

						/*
						**	Try to place the object into a nearby cell if something is preventing
						**	placement at the crate location.
						*/
						Cell cell = Map.Nearby_Location(Fetch_CellID(), goodie_unit->Class->Speed);
						if (cell != CELL_NONE && goodie_unit->Unlimbo(cell)) {
							return(false);
						}
						delete goodie_unit;
						powerup = CRATE_MONEY;
						goto crate_money;
					}
				}
			}
			break;

			/*
			**	Create a squad of miscellaneous composition.
			*/
			case CRATE_SQUAD:
#if NEVER
				for (index = 0; index < 5; index++) {
					static InfantryType _inf[] = {
						INFANTRY_E1,INFANTRY_E1,INFANTRY_E1,INFANTRY_E1,INFANTRY_E1,INFANTRY_E1,
						INFANTRY_E2,
						INFANTRY_E3,
						INFANTRY_ENGINEER
					};
					if (!InfantryTypes[_inf[Random_Pick(0, ARRAY_SIZE(_inf)-1)]]->Create_And_Place(Fetch_CellID(), object->Owner_HouseClass())) {
						if (index == 0) {
							goto crate_money;
						}
					}
				}
				return(false);
#endif
				break;

			/*
			**	Give the player money.
			*/
			case CRATE_MONEY:
crate_money:
				DebugString("Crate at %d,%d contains money\n", CellID.X, CellID.Y);
				if (!force_money) {
					force_money = Random_Pick((int)data, (int)data+900);
				}
				if (!object->House->Is_Player_Control() || Session.Type != GAME_NORMAL) {
					object->House->Refund_Money(force_money);
				} else {
					PlayerPtr->Refund_Money(force_money);
				}
				break;

			/*
			**	A group of explosions are triggered around the crate.
			*/
			case CRATE_EXPLOSION:
				DebugString("Crate at %d,%d contains explosives\n", CellID.X, CellID.Y);
				damage = data;
				if (object != NULL) {
					int d = damage;
					object->Take_Damage(d, 0, Rule->C4Warhead, 0, true);
				}
				for (index = 0; index < 5; index++) {
					Coord frag_coord = Coord_Scatter(Cell_Coord(), Random_Pick(0, 2 * CELL_LEPTON));
					Explosion_Damage(frag_coord, damage, NULL, Rule->C4Warhead);
					AnimTypeClass const * anim = Combat_Anim(damage, Rule->C4Warhead, LAND_CLEAR, frag_coord);
					new AnimClass(anim, frag_coord, 0, 1, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ZGRAD), Get_Explosion_Z(frag_coord));
					Combat_Lighting(frag_coord, damage, Rule->C4Warhead, false);
				}
				break;

			/*
			**	A napalm blast is triggered.
			*/
			case CRATE_NAPALM:
				DebugString("Crate at %d,%d contains napalm\n", CellID.X, CellID.Y);
				coord = (Cell_Coord() + object->Center_Coord()) / 2;
				new AnimClass(AnimTypes[ANIM_NAPALM3], coord);
				damage = data;
				if (object != NULL) {
					int d = damage;
					object->Take_Damage(d, 0, Rule->FlameDamage, NULL, true);
				}
				Explosion_Damage(coord, damage, NULL, Rule->FlameDamage);
				break;

			/*
			**	All objects within a certain range will gain the ability to cloak.
			*/
			case CRATE_CLOAK:
				DebugString("Crate at %d,%d contains cloaking device\n", CellID.X, CellID.Y);
				for (index = 0; index < DisplayClass::Layer[LAYER_GROUND].Count(); index++) {
					ObjectClass * obj = DisplayClass::Layer[LAYER_GROUND][index];

					if (obj != NULL && obj->IsDown && obj->Is_Techno() && Distance(Cell_Coord(), obj->Center_Coord()) < Rule->CrateRadius) {
						((TechnoClass *)obj)->IsCloakable = true;
					}
				}
				break;

			/*
			 * All techno objects within a certain range gain veterancy.
			 */
			case CRATE_VETERAN:
				DebugString("Crate at %d,%d contains veterancy(TM)\n", CellID.X, CellID.Y);
				for (index = 0; index < DisplayClass::Layer[LAYER_GROUND].Count(); index++) {
					ObjectClass * obj = DisplayClass::Layer[LAYER_GROUND][index];

					if (obj != NULL && obj->IsDown && obj->Is_Techno() &&
						((TechnoClass *)obj)->TClass->IsTrainable && Distance(Cell_Coord(), obj->Center_Coord()) < Rule->CrateRadius) {
						for (int count = 0; count < data; count++) {
							VeterancyClass * vet = &((TechnoClass *)obj)->Veterancy;
							if (vet->Is_Veteran()) vet->Set_Elite(true);
							if (vet->Is_Rookie()) vet->Set_Veteran(true);
							if (vet->Is_Dumbass()) vet->Set_Rookie(true);
						}
					}
				}
				break;

			/*
			**	All of the player's objects heal up.
			*/
			case CRATE_HEAL_BASE:
				DebugString("Crate at %d,%d contains base healing\n", CellID.X, CellID.Y);
				if (object->IsOwnedByPlayer) {
					Sound_Effect(Rule->HealCrateSound, object->Center_Coord());
				}
				for (index = 0; index < Logic.Count(); index++) {
					ObjectClass * obj = Logic[index];

					if (obj && object->Is_Techno() && object->House == obj->Owner_HouseClass()) {
						int damage = obj->Strength - obj->Class_Of()->MaxStrength;
						obj->Take_Damage(damage, 0, Rule->C4Warhead, NULL, true, true);
					}
				}
				break;

			case CRATE_ICBM: {
				DebugString("Crate at %d,%d contains ICBM\n", CellID.X, CellID.Y);
				SuperWeaponTypeClass * super = SuperWeaponTypeClass::From_Action(ACTION_NUKE_BOMB);
				int sindex = -1;
				for (index = 0; index < object->House->SuperWeapon.Count(); index++) {
					if (object->House->SuperWeapon[index]->Class->Type == SUPER_FIRST) {
						sindex = index;
						break;
					}
				}
				if (sindex != -1 && object->House->SuperWeapon[super->HeapID]->Enable(true, false, false) && object->IsOwnedByPlayer) {
					Map.Add(RTTI_SPECIAL, super->HeapID);
					Map.Column[1].Flag_To_Redraw();
				}
				break;
			}

			case CRATE_ARMOR:
				DebugString("Crate at %d,%d contains armor\n", CellID.X, CellID.Y);
				for (index = 0; index < DisplayClass::Layer[LAYER_GROUND].Count(); index++) {
					ObjectClass * obj = DisplayClass::Layer[LAYER_GROUND][index];

					if (obj != NULL && obj->Is_Techno() && Distance(Cell_Coord(), obj->Center_Coord()) < Rule->CrateRadius && ((TechnoClass *)obj)->ArmorBias == 1) {
						double val = ((TechnoClass *)obj)->ArmorBias * data;
						((TechnoClass *)obj)->ArmorBias = val;
						if (obj->Owner_HouseClass()->Is_Player_Control()) tospeak = true;
					}
				}
				if (tospeak) Speak(VOX_UPGRADE_ARMOR);
				break;

			case CRATE_SPEED:
				DebugString("Crate at %d,%d contains speed\n", CellID.X, CellID.Y);
				for (index = 0; index < DisplayClass::Layer[LAYER_GROUND].Count(); index++) {
					ObjectClass * obj = DisplayClass::Layer[LAYER_GROUND][index];

					if (obj && obj->Is_Foot() && Distance(Cell_Coord(), obj->Center_Coord()) < Rule->CrateRadius && ((FootClass *)obj)->SpeedBias == 1 && obj->RTTI != RTTI_AIRCRAFT) {
						double val = ((FootClass *)obj)->SpeedBias * data;
						((FootClass *)obj)->SpeedBias = val;
						if (((FootClass *)obj)->IsOwnedByPlayer) tospeak = true;
					}
				}
				if (tospeak) Speak(VOX_UPGRADE_SPEED);
				break;

			case CRATE_FIREPOWER:
				DebugString("Crate at %d,%d contains firepower\n", CellID.X, CellID.Y);
				for (index = 0; index < DisplayClass::Layer[LAYER_GROUND].Count(); index++) {
					ObjectClass * obj = DisplayClass::Layer[LAYER_GROUND][index];

					if (obj && obj->Is_Techno() && Distance(Cell_Coord(), obj->Center_Coord()) < Rule->CrateRadius && ((TechnoClass *)obj)->FirepowerBias == 1) {

						double val = ((TechnoClass *)obj)->FirepowerBias * data;
						((TechnoClass *)obj)->FirepowerBias = val;
						if (obj->Owner_HouseClass()->Is_Player_Control()) tospeak = true;
					}
				}
				if (tospeak) Speak(VOX_UPGRADE_FIREPOWER);
				break;

			case CRATE_INVULN:
				break;

			case CRATE_ION_STORM:
				break;

			default:
				break;
		}

		if (CrateAnims[powerup] != ANIM_NONE) {
			coord = Cell_Coord() + Coord(0, 0, 200);
			new AnimClass(AnimTypes[CrateAnims[powerup]], coord);
		}
	}

	return(true);
}


/***********************************************************************************************
 * CellClass::Flag_Place -- Places a house flag down on the cell.                              *
 *                                                                                             *
 *    This routine will place the house flag at this cell location.                            *
 *                                                                                             *
 * INPUT:   house -- The house that is having its flag placed here.                            *
 *                                                                                             *
 * OUTPUT:  Was the flag successfully placed here?                                             *
 *                                                                                             *
 * WARNINGS:   Failure to place means that the cell is impassable for some reason.             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CellClass::Flag_Place(HousesType house)
{
	if (!IsFlagged && Is_Clear_To_Move(SPEED_TRACK, false, false)) {
		IsFlagged = true;
		Owner = house;
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * CellClass::Flag_Remove -- Removes the house flag from the cell.                             *
 *                                                                                             *
 *    This routine will free the cell of any house flag that may be located there.             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Was there a flag here that was removed?                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CellClass::Flag_Remove(void)
{
	if (IsFlagged) {
		IsFlagged = false;
		Owner = HOUSE_NONE;
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * CellClass::Shimmer -- Causes all objects in the cell to shimmer.                            *
 *                                                                                             *
 *    This routine is called when some event would cause a momentary disruption in the         *
 *    cloaking device. All objects that are cloaked in the cell will have their cloaking       *
 *    device shimmer.                                                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void CellClass::Shimmer(void)
{
	ObjectClass * object = Cell_Occupier();

	while (object) {
		object->Do_Shimmer();
		object = object->Next;
	}
}


/***********************************************************************************************
 * CellClass::Is_Clear_To_Move -- Determines if the cell is generally clear for travel         *
 *                                                                                             *
 *    This routine is called when determining general passability for purposes of zone         *
 *    calculation. Only blockages that cannot be circumvented are considered to make a cell    *
 *    impassable. All other obstructions can either be destroyed or are temporary.             *
 *                                                                                             *
 * INPUT:   loco     -- The locomotion type to use when determining passablility.              *
 *                                                                                             *
 *          ignoreinfantry -- Should infantry in the cell be ignored for movement purposes?    *
 *                                                                                             *
 *          ignorevehicles -- If vehicles should be ignored, then this flag will be true.      *
 *                                                                                             *
 *          zone     -- If specified, the zone must match this value or else movement is       *
 *                      presumed disallowed.                                                   *
 *                                                                                             *
 *          check    -- This specifies the zone type that this check applies to.               *
 *                                                                                             *
 * OUTPUT:  Is the cell generally passable to ground targeting?                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/25/1995 JLB : Created.                                                                 *
 *   06/25/1996 JLB : Uses tracked vehicles as a basis for zone check.                         *
 *   10/05/1996 JLB : Allows checking for crushable blockages.                                 *
 *=============================================================================================*/
bool CellClass::Is_Clear_To_Move(SpeedType loco, bool ignoreinfantry, bool ignorevehicles, int zone, MZoneType check, int cell_height, bool checkbridge) const
{
	/*
	**	Flying objects always consider every cell passable since they can fly over everything.
	*/
	if (loco == SPEED_WINGED) {
		return(true);
	}

	/*
	**	If a zone was specified, then see if the cell is in a legal
	**	zone to allow movement.
	*/
	if (zone != -1) {
		if (zone != Map.Get_Cell_Zone(CellID, check, checkbridge)) {
			return(false);
		}
	}

	if (cell_height != -1) {
		if (cell_height != Height) {
			if (!IsUnderBridge || cell_height != Height + BRIDGE_CELL_HEIGHT) {
				return(false);
			}
		} else if (IsUnderBridge && !checkbridge) {
			return(false);
		}
	}

	bool bridge = false;
	int composite;
	if ((cell_height == -1 || cell_height == Height + BRIDGE_CELL_HEIGHT) && IsUnderBridge) {
		bridge = true;
		composite = BridgeFlag.Composite;
	} else {
		composite = Flag.Composite;
	}

	/*
	**	Check the occupy bits for passable legality. If ignore infantry is true, then
	**	don't consider infnatry.
	*/
	if (ignoreinfantry) {
		composite &= 0xE0;			// Drop the infantry occupation bits.
	}
	if (ignorevehicles) {
		composite &= 0x5F;			// Drop the vehicle/building bit.
	}
	if (composite != 0) {
		return(false);
	}

	/*
	**	Fetch the land type of the cell -- to be modified and used later.
	*/
	LandType land = Land_Type();

	/*
	**	Walls are always considered to block the terrain for general passability
	**	purposes unless this is a wall crushing check or if the checking object
	**	can destroy walls.
	*/
	OverlayTypeClass const * overlay = NULL;
	if (Overlay != OVERLAY_NONE) {
	 	overlay = OverlayTypes[Overlay];
	}
	if (overlay != NULL && overlay->IsWall) {
		if (check != MZONE_DESTROYER && check != MZONE_AMPHIBIOUS_DESTROYER && check != MZONE_INFANTRY_DESTROYER && (check != MZONE_CRUSHER && check != MZONE_AMPHIBIOUS_CRUSHER || !overlay->IsCrushable)) {
			return(false);
		}

		/*
		**	Crushing objects consider crushable walls as clear rather than the
		**	typical LAND_WALL setting.
		*/
		land = LAND_CLEAR;
	}

	/*
	**	See if the ground type is impassable to this locomotion type and if
	**	so, return the error condition.
	*/
	if (::Ground[land].Cost[loco] == 0 && !bridge) {
		return(false);
	}

	/*
	**	All checks passed, so this cell must be passable.
	*/
	return(true);
}


/***********************************************************************************************
 * CellClass::Is_Bridge_Here -- Checks to see if this is a bridge occupied cell.               *
 *                                                                                             *
 *    This routine will examine this cell and if there is a bridge here, it will return        *
 *    true.                                                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is there a bridge located in this cell?                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CellClass::Is_Bridge_Here(void) const
{
	return(IsUnderBridge);
}


/***********************************************************************************************
 * CellClass::Can_Tiberium_Grow -- Determines if Tiberium can grow in this cell.               *
 *                                                                                             *
 *    This checks the cell to see if Tiberium can grow at least one level in it. Tiberium can  *
 *    grow only if there is Tiberium already present. It can only grow to a certain level      *
 *    and then all further growth is suspended.                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Can Tiberium grow in this cell?                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/14/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CellClass::Can_Tiberium_Grow(void) const
{
	if (!Scen->IsTibGrowth) return(false);

	TiberiumType tiberium = Tiberium_Type_Here();

	if (tiberium == TIBERIUM_NONE) return(false);

	TiberiumClass *tptr = Tiberiums[tiberium];

	if (OverlayData >= tptr->FrameCount - 1) return(false);

	if (tptr->GrowthPercentage < 0.00001) return(false);

	return(true);
}


/***********************************************************************************************
 * CellClass::Can_Tiberium_Spread -- Determines if Tiberium can spread from this cell.         *
 *                                                                                             *
 *    This routine will examine the cell and determine if there is sufficient Tiberium         *
 *    present that Tiberium spores will spread to adjacent cells. If the Tiberium level is     *
 *    too low, spreading will not occur.                                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Can Tiberium spread from this cell into adjacent cells?                      *
 *                                                                                             *
 * WARNINGS:   This routine does not check to see if, in fact, there are any adjacent cells    *
 *             available to spread to.                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/14/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CellClass::Can_Tiberium_Spread(void) const
{
	if (!Scen->Special.IsTSpread) return(false);

	TiberiumType tiberium = Tiberium_Type_Here();

	if (tiberium == TIBERIUM_NONE) return(false);

	if (OverlayData <= tiberium / 2) return(false);

	if (Tiberiums[tiberium]->SpreadPercentage < 0.00001) return(false);

	if (Cell_Occupier() != NULL) return(false);

	return(true);
}


/***********************************************************************************************
 * CellClass::Grow_Tiberium -- Grows the tiberium in the cell.                                 *
 *                                                                                             *
 *    This routine will cause the tiberium to grow in the cell.                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Did Tiberium grow in the cell?                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/14/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CellClass::Grow_Tiberium(void)
{
	if (Can_Tiberium_Grow()) {
		return(Place_Tiberium(Tiberium_Type_Here(), 1));
	}
	return(false);
}


/***********************************************************************************************
 * CellClass::Spread_Tiberium -- Spread Tiberium from this cell to an adjacent cell.           *
 *                                                                                             *
 *    This routine will cause the Tiberium to spread from this cell into an adjacent (random)  *
 *    cell.                                                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Did the Tiberium spread?                                                     *
 *                                                                                             *
 * WARNINGS:   If there are no adjacent cells that the tiberium can spread to, then this       *
 *             routine will fail.                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/14/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CellClass::Spread_Tiberium(bool forced)
{
	if (!forced) {
		if (!Can_Tiberium_Spread()) return(false);
	}

	TiberiumType tibtype = Tiberium_Type_Here();

	if (!forced) {
		if (tibtype == TIBERIUM_NONE) return(false);
	}

	if (tibtype == TIBERIUM_NONE) {
		tibtype = TIBERIUM_FIRST;
	}

	TiberiumClass * tiberium = Tiberiums[tibtype];
	FacingType offset = Random_Pick(FACING_N, FACING_NW);
	for (FacingType index = FACING_N; index < FACING_COUNT; index++) {
		CellClass * newcell = &Adjacent_Cell(FacingType(Facing_Add(index, offset))); //was (index+offset);

		if (newcell != NULL && newcell->Can_Tiberium_Germinate(tiberium)) {
			return(newcell->Place_Tiberium(tibtype, 5));
		}
	}
	return(false);
}


/***********************************************************************************************
 * CellClass::Can_Tiberium_Germinate -- Determines if Tiberium can begin growth in the cell.   *
 *                                                                                             *
 *    This routine will examine the cell and determine if Tiberium can start growth in it.     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Can Tiberium grow in this cell?                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/14/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CellClass::Can_Tiberium_Germinate(TiberiumClass const * tiberium) const
{
	if (!Map.In_Local_Radar(CellID)) return(false);

	if (IsUnderBridge || WasUnderBridge) return(false);

	/*
	**	Don't allow Tiberium to grow on a cell with a building unless that building is
	**	invisible. In such a case, the Tiberium must grow or else the location of the
	**	building will be revealed.
	*/
	BuildingClass const * building = Cell_Building();
	if (building != NULL && building->Strength > 0 && !building->Class->IsInvisible && !building->Class->IsInvisibleInGame) return(false);

	TerrainClass * terrain = Cell_Terrain();
	if (terrain != NULL && terrain->Class->IsTiberiumSpawn) return(false);

	if (!Ground[Land_Type()].Build) return(false);

	if (Overlay != OVERLAY_NONE) return(false);

	if (Ramp > RAMP_SOUTH || (Ramp != RAMP_NONE && tiberium != NULL && tiberium->RampVariety == 0)) return(false);

	if (ITType >= ISOTILE_FIRST && ITType < IsometricTileTypes.Count() && !IsometricTileTypes[ITType]->IsAllowTiberium) return(false);

	return(true);
}


/// <summary>
/// Lists the members this cell carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void CellClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(CellID);

	// Post_Load installs the cell in the array slot this coordinate names, so a coordinate
	// that names none is refused here, while the record can still be thrown away whole.
	if (stream.Is_Loading() && Map.Cell_Slot(CellID) < 0) {
		stream.Fail();
		return;
	}

	/*
	 * The snapshot list is built only once something standing here has been fogged over,
	 * so whether the cell has one at all travels ahead of its contents.
	 */
	bool hasfogged = (FoggedObjects != NULL);
	stream.Serialize(hasfogged);
	if (stream.Is_Loading() && hasfogged) {
		FoggedObjects = new FOGGED_OBJECT_LIST;
	}
	if (hasfogged) {
		stream.Serialize(*FoggedObjects);
	}

	stream.Serialize(BridgeDeckCell);
	stream.Serialize(UnusedCell);
	// Drawer -- no save has ever carried it; a shared object the cell picks again when it is lit.
	stream.Serialize(ITType);
	stream.Serialize(Tag);
	stream.Serialize(Overlay);
	stream.Serialize(Smudge);
	// Passability -- no save has ever carried it either; derived from the terrain and whatever is
	// standing here.
	stream.Serialize(Owner);
	stream.Serialize(InfType);
	stream.Serialize(BridgeInfType);
	stream.Serialize(LastRedrawFrame);
	stream.Serialize(LastUnknownDrawFrame);
	stream.Serialize(LastBridgeDrawFrame);
	stream.Serialize(LastBridgeDrawRect);
	stream.Serialize(CloakedBy);
	stream.Serialize(SensedBy);
	stream.Serialize(OccupiedBy);
	stream.Serialize(OccupierPtr);
	stream.Serialize(BridgeOccupierPtr);
	stream.Serialize(Land);
	stream.Serialize(Intensity);
	stream.Serialize(Ambient);
	stream.Serialize(Brightness);
	stream.Serialize(TileBrightness);
	stream.Serialize(AltBrightness);
	stream.Serialize(RedTint);
	stream.Serialize(GreenTint);
	stream.Serialize(BlueTint);
	stream.Serialize(Tube);
	stream.Serialize(LastBridgeDrawRedraws);
	stream.Serialize(IsIceGrowthAllowed);
	stream.Serialize(SubTile);
	stream.Serialize(Height);
	stream.Serialize(Ramp);
	stream.Serialize(Elevation);
	stream.Serialize(OverlayData);
	stream.Serialize(SmudgeData);
	stream.Serialize(ShadowFrame);
	stream.Serialize(FogFrame);
	stream.Serialize(AdjacentObjectCount);

	/*
	 * Each set of sub position flags is carried as the composite byte it shares storage
	 * with, which is every one of its eight bits in a single trip.
	 */
	stream.Serialize(Flag.Composite);
	stream.Serialize(BridgeFlag.Composite);

	SERIALIZE_BIT(stream, IsPlot);
	SERIALIZE_BIT(stream, IsCursorHere);
	SERIALIZE_BIT(stream, IsMapped);
	SERIALIZE_BIT(stream, IsVisible);
	SERIALIZE_BIT(stream, IsFogVisible);
	SERIALIZE_BIT(stream, IsFogMapped);
	SERIALIZE_BIT(stream, IsWaypoint);
	SERIALIZE_BIT(stream, IsRadarCursor);
	SERIALIZE_BIT(stream, IsFlagged);
	SERIALIZE_BIT(stream, IsToShroud);
	SERIALIZE_BIT(stream, IsToFog);
	SERIALIZE_BIT(stream, IsBridgeDeck);
	SERIALIZE_BIT(stream, IsUnderBridge);
	SERIALIZE_BIT(stream, IsBridgeTraversable);
	SERIALIZE_BIT(stream, WasUnderBridge);
	SERIALIZE_BIT(stream, IsBridgeEastWest);
	SERIALIZE_BIT(stream, IsBridgeSurface);
	SERIALIZE_BIT(stream, IsBridgeDamaged);
	SERIALIZE_BIT(stream, IsToGrowIce);
	SERIALIZE_BIT(stream, IsToGrowVeins);
	SERIALIZE_BIT(stream, IsOvershadowed);
	SERIALIZE_BIT(stream, IsAnimAttached);
	SERIALIZE_BIT(stream, IsPredictedPath);
	SERIALIZE_BIT(stream, IsAffectedByEMP);
	SERIALIZE_BIT(stream, IsHorizontalLine);
	SERIALIZE_BIT(stream, IsVerticalLine);
	SERIALIZE_BIT(stream, IsFogged);
}


/// <summary>
/// Hands the cell back to the map's cell array.
/// A cell is loaded as a free standing object, so the array slot its own coordinate names
/// has to be pointed at it -- and whatever placeholder was there thrown away -- before
/// anything can reach the cell by position.
/// </summary>
void CellClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	int id = Map.Cell_Slot(CellID);
	if (id < 0) {
		return;
	}

	if (Map.Array[id] != NULL) {
		delete Map.Array[id];
		Map.Array[id] = NULL;
	}
	Map.Array[id] = this;
}


/// <summary>
/// Recalculates the movement passability of this cell.
/// This routine boils the cell's terrain, overlay and occupiers down to the single
/// passability value that the zone builder and the pathfinder work from. Call it whenever
/// something that could block or unblock the cell has changed.
/// </summary>
void CellClass::Recalc_Passability(void)
{
	if (!Map.In_Local_Radar(CellID)) {
		Passability = PASSABLE_OUTSIDE;
		return;
	}

	if (Overlay != OVERLAY_NONE) {
		OverlayTypeClass const * overlay = OverlayTypes[Overlay];
		if (overlay->IsCrushable) {
			Passability = PASSABLE_CRUSH;
			return;
		}
		if (overlay->IsWall) {
			Passability = PASSABLE_BLOCKED;
			return;
		}
		if (Ground[overlay->Land].Cost[SPEED_WHEEL] == 0) {
			Passability = PASSABLE_NO;
			return;
		}
	}

	if (Land == LAND_WATER || Land == LAND_BEACH) {
		Passability = PASSABLE_WATER;
		return;
	}

	if (Ground[Land].Cost[SPEED_WHEEL] <= 0.01) {
		Passability = PASSABLE_NO;
		return;
	}

	ObjectClass * occupier = OccupierPtr;
	while (occupier != NULL) {
		switch (occupier->Fetch_RTTI()) {
			case RTTI_BUILDING:
				if (((BuildingClass *)occupier)->Class->IsFirestormWall) {
					if (((BuildingClass *)occupier)->House->FirestormDefenseActivated) {
						Passability = PASSABLE_NO;
						return;
					}
				} else if (((BuildingClass *)occupier)->Class->IsLaserFence) {
					if (((BuildingClass *)occupier)->LaserFenceFrame != 12 && ((BuildingClass *)occupier)->LaserFenceFrame != 8) {
						Passability = PASSABLE_NO;
					}
				}
				break;

			case RTTI_TERRAIN:
				if (((TerrainClass *)occupier)->Occupation_Bits() != 7) {
					Passability = PASSABLE_PARTIALLY_BLOCKED;
					return;
				}

				Passability = PASSABLE_BLOCKED;
				return;
		}
		occupier = occupier->Next;
	}

	Passability = PASSABLE_LAND;
}


/// <summary>
/// Sets up the drawer and lighting values that this cell draws with.
/// Pass a drawer to force the cell onto it, or NULL to have the cell work its own lighting
/// out from the scenario and claim whichever shared drawer suits. Cells off the edge of the
/// map fall back to plain daylight.
/// </summary>
/// <param name="drawer">The drawer to force onto the cell, or NULL to pick one to suit.</param>
/// <remarks>The lighting values passed in are only honored when a drawer is supplied as
/// well. Otherwise they are recomputed from the scenario and whatever was passed is
/// discarded.</remarks>
void CellClass::Init_Drawer(LightConvertClass * drawer, int intensity, int ambient, int brightness, int tile_brightness, int alt_brightness)
{
	bool got_drawer = false;
	int red_tint, green_tint, blue_tint;

	if (drawer != NULL) {

		/*
		 * We got a drawer passed in, so use it.
		 */
		Dec_Drawer_Ref_Count(Drawer);
		Drawer = drawer;
		Inc_Drawer_Ref_Count(Drawer);

		/*
		 * Use the tint values from the drawer.
		 */
		red_tint = Drawer->NormalRedTint;
		green_tint = Drawer->NormalGreenTint;
		blue_tint = Drawer->NormalBlueTint;

		got_drawer = true;

	} else if (CellID != Cell(0, 0) && CellID != Cell(-1, -1)) {

		/*
		 * No drawer provided, so let's try to find or make one.
		 * First, adjust the light values based on the current light conditions.
		 */
		Init_Light(intensity, ambient, brightness, tile_brightness, alt_brightness, red_tint, green_tint, blue_tint);

		/*
		 * Check if the current drawer fits out requirements. If not, dispose of it.
		 */
		if (Drawer != NULL) {
			int r_clamped = red_tint, g_clamped = green_tint, b_clamped = blue_tint;
			Clamp_Tile_RGB(r_clamped, g_clamped, b_clamped);
			if (Drawer->NormalRedTint != r_clamped ||
				Drawer->NormalGreenTint != g_clamped ||
				Drawer->NormalBlueTint != b_clamped) {
				Dec_Drawer_Ref_Count(Drawer);
				Drawer = NULL;
			}
		}

		/*
		 * If now we don't have a drawer, find or make one with the required properties.
		 */
		if (Drawer == NULL) {
			Drawer = IsometricTileTypeClass::Find_Or_Make_Drawer(red_tint, green_tint, blue_tint);
			if (Drawer != NULL) {
				Inc_Drawer_Ref_Count(Drawer);
			}
		}

		got_drawer = true;
	}

	/*
	 * We've got a drawer, so set the color properties.
	 */
	if (got_drawer) {
		Intensity = intensity;
		Ambient = ambient;
		Brightness = brightness;
		TileBrightness = tile_brightness;
		AltBrightness = alt_brightness;
		RedTint = red_tint;
		GreenTint = green_tint;
		BlueTint = blue_tint;
		return;
	}

	/*
	 * Fallback: make a default drawer.
	 */
	Drawer = IsometricTileTypeClass::Find_Or_Make_Drawer(NORMAL_LIGHT, NORMAL_LIGHT, NORMAL_LIGHT);
	if (Drawer != NULL) {
		Inc_Drawer_Ref_Count(Drawer);
	}
	Brightness = NORMAL_LIGHT;
	TileBrightness = NORMAL_LIGHT;
	AltBrightness = NORMAL_LIGHT;
	RedTint = NORMAL_LIGHT;
	GreenTint = NORMAL_LIGHT;
	BlueTint = NORMAL_LIGHT;
	Intensity = 0x10000;
	Ambient = 0;
}


/// <summary>
/// Works out which drawer and lighting values this cell would use.
/// This routine makes the same choice as Init_Drawer but leaves the cell alone, so a caller
/// can ask what a cell's lighting would come to without committing it to anything.
/// </summary>
/// <param name="drawer">Receives the drawer the cell would draw through.</param>
/// <param name="tile_brightness">Receives the brightness to draw the ground tile with.</param>
/// <param name="alt_brightness">Receives the brightness to use at bridge deck height.</param>
void CellClass::Pick_Drawer(LightConvertClass * & drawer, int & intensity, int & ambient, int & brightness, int & tile_brightness, int & alt_brightness) const
{
	intensity = 0x10000;
	ambient = 0;
	brightness = NORMAL_LIGHT;
	tile_brightness = NORMAL_LIGHT;
	alt_brightness = NORMAL_LIGHT;

	int red_tint, green_tint, blue_tint;

	if (CellID != Cell(0, 0) && CellID != Cell(-1, -1)) {

		/*
		 * First, adjust the light values based on the current light conditions.
		 */
		Init_Light(intensity, ambient, brightness, tile_brightness, alt_brightness, red_tint, green_tint, blue_tint);

		/*
		 * Check if the current drawer fits out requirements.
		 */
		bool new_drawer = false;
		if (Drawer != NULL) {
			int r_clamped = red_tint, g_clamped = green_tint, b_clamped = blue_tint;
			Clamp_Tile_RGB(r_clamped, g_clamped, b_clamped);
			if (Drawer->NormalRedTint == r_clamped &&
				Drawer->NormalGreenTint == g_clamped &&
				Drawer->NormalBlueTint == b_clamped) {
				drawer = Drawer;
			} else {
				new_drawer = true;
			}
		} else {
			new_drawer = true;
		}

		/*
		 * We need a new drawer, find or make one with the required properties.
		 */
		if (new_drawer) {
			drawer = IsometricTileTypeClass::Find_Or_Make_Drawer(red_tint, green_tint, blue_tint);
		}

	} else {
		drawer = IsometricTileTypeClass::Find_Or_Make_Drawer(NORMAL_LIGHT, NORMAL_LIGHT, NORMAL_LIGHT);
	}
}


/// <summary>
/// Computes the lighting values that apply to this cell.
/// This routine gathers the scenario's ambient light, every light source within reach of
/// the cell, and the height based level lighting, then folds them all into one set of
/// brightness and tint values. Cells off the edge of the map are given plain daylight.
/// </summary>
/// <param name="intensity">Receives the light intensity scale, as a 16.16 fixed point
/// value.</param>
/// <param name="tile_brightness">Receives the brightness to draw the ground tile with.</param>
/// <param name="alt_brightness">Receives the brightness to use at bridge deck height.</param>
void CellClass::Init_Light(int & intensity, int & ambient, int & brightness, int & tile_brightness, int & alt_brightness, int & red_tint, int & green_tint, int & blue_tint) const
{
	if (CellID != Cell(0, 0) && CellID != Cell(-1, -1)) {

		brightness = NORMAL_LIGHT * Scen->CurrentAmbientLight / 100;
		red_tint = NORMAL_LIGHT * Scen->RedTint / 100;
		green_tint = NORMAL_LIGHT * Scen->GreenTint / 100;
		blue_tint = NORMAL_LIGHT * Scen->BlueTint / 100;
		ambient = 0;

		for (int i = 0; i < LightSources.Count(); i++) {
			LightSourceClass * light = LightSources[i];
			if (light->Is_Enabled()) {
				Coord cc = Cell_Coord();
				unsigned int visibility = light->Visibility;
				unsigned int dx = cc.X - ((Coord)light->Position).X;
				unsigned int dy = cc.Y - ((Coord)light->Position).Y;
				if (dx * dx + dy * dy <= visibility * visibility) {
					dx = cc.X - ((Coord)light->Position).X;
					dy = cc.Y - ((Coord)light->Position).Y;
					unsigned int dist = Point2D(dx, dy).Length();
					if (dist <= visibility) {
						int num = (NORMAL_LIGHT * visibility - NORMAL_LIGHT * dist) / visibility;
						ambient += (num * light->Intensity) / NORMAL_LIGHT;
						red_tint += (num * light->RedTint) / NORMAL_LIGHT;
						green_tint += (num * light->GreenTint) / NORMAL_LIGHT;
						blue_tint += (num * light->BlueTint) / NORMAL_LIGHT;
					}
				}
			}
		}

		brightness += ambient;
		alt_brightness = brightness;

		if (IonStormClass::Is_Ion_Storm_Active()) {
			brightness += Height * Scen->IonLevelLight - Scen->IonGroundLight;
			alt_brightness += (Height + BRIDGE_CELL_HEIGHT) * Scen->IonLevelLight - Scen->IonGroundLight;
		} else {
			brightness += Height * Scen->LevelLight - Scen->GroundLight;
			alt_brightness += (Height + BRIDGE_CELL_HEIGHT) * Scen->LevelLight - Scen->GroundLight;
		}

		tile_brightness = brightness;

		Adjust_Tile_RGB(intensity, tile_brightness, red_tint, green_tint, blue_tint);

		alt_brightness = (alt_brightness * intensity) >> 16;

		brightness = std::min(brightness, 2000);
		alt_brightness = std::min(alt_brightness, 2000);

		brightness = std::max(brightness, 0);
		tile_brightness = std::max(tile_brightness, 0);
		alt_brightness = std::max(alt_brightness, 0);

	} else {
		intensity = 0x10000;
		ambient = 0;
		brightness = NORMAL_LIGHT;
		tile_brightness = NORMAL_LIGHT;
		alt_brightness = NORMAL_LIGHT;
		red_tint = NORMAL_LIGHT;
		green_tint = NORMAL_LIGHT;
		blue_tint = NORMAL_LIGHT;
	}
}


/// <summary>
/// Recomputes this cell's lighting from the current scenario conditions.
/// This routine is called when the ambient light changes -- an ion storm rolling in, or a
/// scripted lighting change -- so that the cell's brightness follows along without having
/// to go looking for a new drawer.
/// </summary>
void CellClass::Recalc_Light(void)
{
	Brightness = Ambient + (NORMAL_LIGHT * Scen->CurrentAmbientLight) / 100;
	AltBrightness = Brightness;

	if (IonStormClass::Is_Ion_Storm_Active()) {
		Brightness += Height * Scen->IonLevelLight - Scen->IonGroundLight;
		AltBrightness += (Height + BRIDGE_CELL_HEIGHT) * Scen->IonLevelLight - Scen->IonGroundLight;
	} else {
		Brightness += Height * Scen->LevelLight - Scen->GroundLight;
		AltBrightness += (Height + BRIDGE_CELL_HEIGHT) * Scen->LevelLight - Scen->GroundLight;
	}

	TileBrightness = (Brightness * Intensity) >> 16;
	AltBrightness = (AltBrightness * Intensity) >> 16;

	Brightness = std::min<int>(Brightness, 2000);
	TileBrightness = std::min<int>(TileBrightness, 2000);
	AltBrightness = std::min<int>(AltBrightness, 2000);

	Brightness = std::max<int>(Brightness, 0);
	TileBrightness = std::max<int>(TileBrightness, 0);
	AltBrightness = std::max<int>(AltBrightness, 0);
}


/// <summary>
/// Determines which way something should bounce off this cell.
/// This routine is used when a projectile or a piece of debris arrives at this cell from
/// the specified origin. The heights of the cells flanking the approach decide whether the
/// deflection comes straight back or veers off to one side.
/// </summary>
/// <param name="xorigin">The coordinate the object arrived from.</param>
/// <returns>Returns with the facing the object should bounce toward.</returns>
FacingType CellClass::Bounce_Direction(Coord const & xorigin) const
{
	Cell origin = Map[xorigin].CellID;

	int dx = CellID.X - origin.X;
	int dy = CellID.Y - origin.Y;

	static FacingType _facing_lookup[5] = { FACING_NONE, FACING_E, FACING_SW, FACING_S, FACING_SE }; /// the index below is not range checked
	FacingType base_direction = _facing_lookup[(CellID.X - origin.X) + 3 * (CellID.Y - origin.Y)]; /// dx + 3 * dy

	if (dx != 0 && dy != 0) {
		bool bounce_left;
		bool bounce_right;

		if (dx == dy) {
			bounce_left = Map[origin + Cell(dx, 0)].Height >= Map[xorigin].Height + 2;
			bounce_right = Map[origin + Cell(0, dy)].Height >= Map[xorigin].Height + 2;
		} else {
			bounce_left = Map[origin + Cell(dy, 0)].Height >= Map[xorigin].Height + 2;
			bounce_right = Map[origin + Cell(0, dx)].Height >= Map[xorigin].Height + 2;
		}

		if (bounce_left) {
			if (bounce_right) {
				return((FacingType)base_direction);
			} else {
				return(Facing_Sub(base_direction, FACING_45));
			}
		} else {
			if (bounce_right) {
				return((FacingType)(base_direction + FACING_45));
			} else {
				return((FacingType)base_direction);
			}
		}
	}
	return(base_direction);
}


/// <summary>
/// Claims a reference to the specified tile drawer.
/// Drawers are shared between all the cells that light identically, so a cell must claim
/// one before it starts drawing through it.
/// </summary>
/// <param name="convert">The drawer being claimed.</param>
void CellClass::Inc_Drawer_Ref_Count(LightConvertClass * convert) const
{
	convert->Add_Reference();
}


/// <summary>
/// Releases this cell's claim on the specified tile drawer.
/// Drawers are shared between all the cells that light identically, so a cell must give up
/// its reference when it stops using one. The release is skipped while the game is shutting
/// down, since the drawers are being torn down regardless.
/// </summary>
/// <param name="convert">The drawer being given up.</param>
void CellClass::Dec_Drawer_Ref_Count(LightConvertClass * convert) const
{
	if (GameActive) {
		convert->Remove_Reference();
	}
}


/// <summary>
/// Does a tunnel run through this cell?
/// </summary>
/// <returns>bool; Is this cell part of a tunnel?</returns>
bool CellClass::Has_Tunnel(void) const
{
	return(Tube >= TUBE_FIRST && Tube < Tubes.Count() && Land == LAND_TUNNEL);
}


/// <summary>
/// Is there a tunnel at this cell or close by to the north west?
/// This is the north west counterpart of Is_Near_Tunnel_ES, used to keep the approach to a
/// tunnel mouth clear.
/// </summary>
/// <returns>bool; Is a tunnel near enough to the north west to matter?</returns>
bool CellClass::Is_Near_Tunnel_NW(void) const
{
	if (Has_Tunnel()) {
		return(true);
	}

	CellClass const * north1 = &Adjacent_Cell(FACING_N);
	if (north1 != NULL) {
		CellClass const * north2 = &north1->Adjacent_Cell(FACING_N);
		if (north2 != NULL) {
			if (north1->Has_Tunnel() && !north2->Has_Tunnel()) {
				return(true);
			}
			CellClass const * north3 = &north2->Adjacent_Cell(FACING_N);
			if (north3 != NULL) {
				if (north2->Has_Tunnel() && !north3->Has_Tunnel()) {
					return(true);
				}
			}
		}
	}

	CellClass const * west1 = &Adjacent_Cell(FACING_W);
	if (west1 != NULL) {
		CellClass const * west2 = &west1->Adjacent_Cell(FACING_W);
		if (west2 != NULL) {
			if (west1->Has_Tunnel() && !west2->Has_Tunnel()) {
				return(true);
			}
			CellClass const * west3 = &west2->Adjacent_Cell(FACING_W);
			if (west3 != NULL) {
				if (west2->Has_Tunnel() &&
				!west3->Has_Tunnel()) {
					return(true);
				}
			}
		}
	}

	return(false);
}


/// <summary>
/// Is there a tunnel at this cell or close by to the south east?
/// A tunnel mouth needs clear ground in front of it, and this routine reports whether an
/// approach from the south east is spoken for.
/// </summary>
/// <returns>bool; Is a tunnel near enough to the south east to matter?</returns>
bool CellClass::Is_Near_Tunnel_ES(void) const
{
	if (Has_Tunnel()) {
		return(true);
	}

	CellClass const * south_east = &Adjacent_Cell(FACING_SE);
	if (south_east != NULL) {
		CellClass const * south_east2 = &south_east->Adjacent_Cell(FACING_SE);
		if (south_east2 != NULL) {
			if (south_east->Has_Tunnel() || south_east2->Has_Tunnel()) {
				return(true);
			}

			CellClass const * south = &south_east->Adjacent_Cell(FACING_S);
			CellClass const * east = &south_east->Adjacent_Cell(FACING_E);
			if (south != NULL && east != NULL) {
				if (south->Has_Tunnel() || east->Has_Tunnel()) {
					return(true);
				}
			}
		}
	}

	return(false);
}


/// <summary>
/// May the specified object enter the tunnel at this cell?
/// The tunnel system turns nobody away, so this routine always agrees. It exists as the
/// hook the movement code consults before committing an object to a tube.
/// </summary>
bool CellClass::Can_Enter_Tunnel(FootClass const * foot) const
{
	return(true);
}


/// <summary>
/// Fetches the tunnel that passes through this cell.
/// </summary>
/// <returns>Returns with a pointer to the tube, or NULL if no tunnel runs here.</returns>
TubeClass * CellClass::Get_Tunnel(void) const
{
	if (Tube >= TUBE_FIRST && Tube < Tubes.Count()) {
		return(Tubes[Tube]);
	}
	return(NULL);
}


/// <summary>
/// Forces this cell's occupiers to re-orient to its slope.
/// This routine is called after the terrain here has been reshaped, so that any vehicle or
/// aircraft standing on the cell settles onto the new ramp instead of hanging at the angle
/// it was given by the old one.
/// </summary>
void CellClass::Force_New_Slope_For_Occupiers(void) const
{
	int count = 0;
	ObjectClass * occupier = Cell_Occupier();
	ObjectClass * occupiers[10];

	while (occupier) {
		if (count >= 10) break;
		occupiers[count] = occupier;
		occupier = occupier->Next;
		count++;
	}

	for (int i = 0; i < count; i++) {
		occupier = occupiers[i];
		occupier->HeightAGL = 0;
		if (occupier->RTTI == RTTI_UNIT || occupier->RTTI == RTTI_AIRCRAFT) {
			((FootClass *)occupier)->Locomotion->Force_New_Slope(Ramp);
		}
	}
}


/// <summary>
/// Fetches the name of the tiberium associated with this cell.
/// </summary>
/// <returns>Returns with the tiberium's name, or NULL if there is no tiberium here.</returns>
char const * CellClass::Tiberium_Name(void) const
{
	TiberiumType tib = Tiberium_Type_Here();

	if (tib != TIBERIUM_NONE) {
		return(Tiberiums[tib]->GivenName);
	}

	return(NULL);
}


/// <summary>
/// Fetches the kind of tiberium associated with this cell.
/// A tiberium overlay answers directly. Failing that, a tiberium spawning terrain object
/// standing here -- a blossom tree -- answers with whatever it seeds the ground with.
/// </summary>
/// <returns>Returns with the tiberium type found, or TIBERIUM_NONE if there is none.</returns>
TiberiumType CellClass::Tiberium_Type_Here(void) const
{
	TiberiumType tib = Which_Tiberium_Type(Overlay);

	if (tib != TIBERIUM_NONE) return(tib);

	TerrainClass * terrain = Cell_Terrain();
	if (terrain) {
		if (terrain->Class && terrain->Class->IsTiberiumSpawn) {
			return(TiberiumType(terrain->Class->TiberiumToSpawn));
		}
	}

	return(TIBERIUM_NONE);
}


/// <summary>
/// Determines what the tiberium growing here is worth.
/// The harvesters and the computer's value estimates use this routine. The worth scales
/// with how much has grown, so a fully ripened cell fetches the most.
/// </summary>
/// <returns>Returns with the credit value of this cell's tiberium, or zero if bare.</returns>
int CellClass::Tiberium_Value(void) const
{
	TiberiumType tib = Which_Tiberium_Type(Overlay);

	if (tib == TIBERIUM_NONE) {
		return(0);
	}

	return(Tiberiums[tib]->CreditValue * (OverlayData + 1));
}


/// Handy macro to shorten all the tileset checks
#define IS_SET_VALID(setname) (IsometricTileTypeClass::setname != ISOTILE_INVALID)
#define IS_TILE_IN_SET(setname, setsize) (ITType >= IsometricTileTypeClass::setname && ITType < IsometricTileTypeClass::setname + setsize)
#define IS_TILE_IN_VALID_SET(setname, setsize) (IS_SET_VALID(setname) && IS_TILE_IN_SET(setname, setsize))


/// <summary>
/// Is the tile on this cell open water?
/// </summary>
/// <returns>bool; Is this a water tile?</returns>
bool CellClass::Is_Tile_Water(void) const
{
	return(IS_TILE_IN_SET(WaterSet, WATER_COUNT));
}


/// <summary>
/// Fetches the height of whatever occupies this cell.
/// The height is measured to the top of the cell's contents, so a building reports its own
/// bulk while a passing unit reports a nominal amount. Use this routine when something has
/// to clear the cell's contents rather than just its ground.
/// </summary>
/// <returns>Returns with the height, in leptons, of the top of this cell's contents.</returns>
LEPTON CellClass::Occupier_Height(void) const
{
	Coord coord(CELL_LEPTON_W / 2, CELL_LEPTON_H / 2, 0);
	coord.Z = Get_Height(coord);

	BuildingClass * building = Cell_Building();
	if (building) {
		Point3D dimensions = building->Class->Lepton_Dimensions();
		coord.Z += dimensions.Z;
	} else {
		TechnoClass * techno = Cell_Techno();
		if (techno != NULL) {
			coord.Z += CELL_LEPTON / 3;
		}
	}

	return(coord.Z);
}


/// <summary>
/// Removes every reference this cell holds to the specified object.
/// This routine is called when an object is about to be destroyed, so that the cell is not
/// left pointing at it. The trigger tag, the bridge deck link and any fogged object record
/// are all severed.
/// </summary>
/// <param name="target">The object that is going away.</param>
void CellClass::Detach(AbstractClass const * target)
{
	if (Tag == target) {
		if (Tag != NULL) {
			Tag->AttachCount--;
		}
		Tag = NULL;
		Map.TaggedCells.Delete(CellID);
	}
	if (BridgeDeckCell == target) {
		BridgeDeckCell = NULL;
	}
	if (UnusedCell == target) {
		UnusedCell = NULL;
	}
	if (FoggedObjects != NULL) {
		if (target->RTTI == RTTI_FOGGEDOBJECT) {
			FoggedObjects->Delete((FoggedObjectClass *)target);
		}
	}
}


ClassID CellClass::Class_ID(void) const
{
	return(ClassID_CellClass);
}


/// <summary>
/// Sets the map location that this cell object represents.
/// </summary>
void CellClass::Set_CellID(Cell const & cell)
{
	CellID = cell;
}


/// <summary>
/// Attaches a trigger tag to this cell.
/// Any tag already attached is released first. The map keeps a list of tagged cells for
/// the trigger logic to walk, and this routine maintains that list.
/// </summary>
/// <param name="tag">The tag to attach, or NULL to merely release the current one.</param>
void CellClass::Attach_Tag(TagClass *tag)
{
	if (Tag != NULL) {
		Tag->AttachCount--;
	}

	Tag = tag;
	if (Tag != NULL) {
		Map.TaggedCells.Add(CellID);
		Tag->AttachCount++;
	}
}


/// <summary>
/// Registers this cell's screen area for redraw.
/// This routine hands the cell's tile rectangle to the tactical map so that whatever has
/// changed here is repainted on the next render pass.
/// </summary>
void CellClass::Register_As_Dirty(void)
{
	Point2D point;
	TacticalMap->Coord_To_Pixel(Coord_Whole(CellID), point);
	point.Y += TacticalRect.Y - LEVEL_PIXEL_H_1 * Height;
	point -= Point2D(ISO_TILE_PIXEL_W / 2, ISO_TILE_PIXEL_H / 2);
	TacticalMap->Register_Dirty_Area(Rect(point, ISO_TILE_PIXEL_W, ISO_TILE_PIXEL_H));
}


/// <summary>
/// Fetches the vein overlay frame for this cell.
/// A vein patch has to join up with the veins around it, so the drawing code uses this
/// routine to pick the piece that fits the cell's cardinal neighbors.
/// </summary>
/// <returns>Returns with the frame index to draw the vein overlay with.</returns>
int CellClass::Get_Vein_Frame(void) const
{
	int frame = 0;
	for (int i = 0; i < FACING_COUNT / 2; i++) {
		CellClass const & adjacent = Adjacent_Cell(FacingType(i << FACING_45));
		if (adjacent.Overlay != OVERLAY_NONE && (adjacent.Ramp <= RAMP_NONE || OverlayTypes[adjacent.Overlay]->IsVeins)) {
			if ((adjacent.Overlay != OVERLAY_VEINS && OverlayTypes[adjacent.Overlay]->IsVeins) ||
				(adjacent.Overlay == OVERLAY_VEINS && adjacent.OverlayData >= OVERLAYDATA_FIRST_SOLID_VEIN)) {
				frame |= 1 << i;
			}
		}
	}
	return(frame);
}


/// <summary>
/// May veins spread onto this cell?
/// This routine is used by the veinhole growth logic to decide where the vein field is
/// allowed to creep next. Water, rock, ice and beach turn it away, as does any overlay
/// that is not itself vein, and the cardinal neighbors must be just as hospitable.
/// </summary>
/// <returns>bool; May veins be placed here?</returns>
bool CellClass::Can_Place_Veins(void)
{
	if (Ramp <= RAMP_SOUTH) {
		if (Land != LAND_WATER && Land != LAND_ROCK && Land != LAND_ICE && Land != LAND_BEACH) {
			if (Overlay == OVERLAY_NONE || OverlayTypes[Overlay]->IsVeins) {
				for (int dir = FACING_N; dir < FACING_COUNT; dir += FACING_90) {
					CellClass & adjacent = Adjacent_Cell(FacingType(dir));
					if (adjacent.Ramp > RAMP_SOUTH && Ramp == RAMP_NONE) {
						if (adjacent.Overlay == OVERLAY_NONE || !OverlayTypes[adjacent.Overlay]->IsVeins) {
							return(false);
						}
					}
					if (adjacent.Land == LAND_WATER || adjacent.Land == LAND_ROCK || adjacent.Land == LAND_ICE || adjacent.Land == LAND_BEACH) {
						return(false);
					}
					if (adjacent.Overlay != OVERLAY_NONE && !OverlayTypes[adjacent.Overlay]->IsVeins) {
						return(false);
					}
				}
				return(true);
			}
		}
	}
	return(false);
}


/// <summary>
/// Recomputes the vein overlay imagery for this cell and its four cardinal
/// neighbors after a vein change, and registers every affected screen and
/// radar area for redraw.
/// </summary>
void CellClass::Redraw_Veins(void)
{
	/*
	 * Remember the area this cell currently occupies on screen so it can be
	 * flagged for redraw after the vein imagery is recomputed.
	 */
	Rect dirty(0, 0, 0, 0);
	dirty = Union(dirty, Union(Overlay_Render_Rect(), Overlay_Shadow_Render_Rect()));

	/*
	 * Recompute the vein frame for this cell. If there is no connecting vein
	 * frame, or the cell is a ramp, then the vein overlay is removed.
	 */
	OverlayData = 0;
	int frame = Get_Vein_Frame();
	if (frame == 0 || Ramp != RAMP_NONE) {
		Overlay = OVERLAY_NONE;
	} else {
		OverlayData = 3 * frame + abs(Scen->RandomNumber() % 3);
	}
	Map.Radar_Background(CellID);

	/*
	 * Now update every cardinal neighbor that is itself a (non-solid) vein
	 * cell, since changing this cell can change how the neighbors connect.
	 */
	for (FacingType dir = FACING_N; dir < FACING_COUNT; dir = FacingType(dir + FACING_90)) {
		CellClass & adjacent = Adjacent_Cell(dir);

		if (adjacent.Ramp == RAMP_NONE && adjacent.Overlay == OVERLAY_VEINS && adjacent.OverlayData < OVERLAYDATA_FIRST_SOLID_VEIN) {

			int adjframe = adjacent.Get_Vein_Frame();

			/*
			 * Capture the neighbor's render area both before and after the
			 * overlay change so the union covers the whole affected region.
			 */
			Rect rect(0, 0, 0, 0);
			rect = Union(rect, Union(adjacent.Overlay_Render_Rect(), adjacent.Overlay_Shadow_Render_Rect()));

			if (adjframe == 0) {
				adjacent.Overlay = OVERLAY_NONE;
			} else {
				adjacent.OverlayData = 3 * adjframe + abs(Scen->RandomNumber() % 3);
			}
			adjacent.Recalc_Attributes();

			rect = Union(rect, Union(adjacent.Overlay_Render_Rect(), adjacent.Overlay_Shadow_Render_Rect())) - TacticalRect.TopLeft;
			TacticalMap->Register_Dirty_Area(rect);
			Map.Radar_Background(adjacent.CellID);
		}
	}

	/*
	 * Finally flag this cell's combined (old + new) area for redraw and
	 * recalculate its land attributes.
	 */
	dirty = Union(dirty, Union(Overlay_Render_Rect(), Overlay_Shadow_Render_Rect())) - TacticalRect.TopLeft;
	TacticalMap->Register_Dirty_Area(dirty);
	Recalc_Attributes();
}


/// <summary>
/// Places the veins overlay on this cell (if veins are allowed here) and spreads to the four
/// cardinal neighbors, converting eligible neighbors to veins as well. The cell's radar
/// background and dirty render areas are updated as the overlay changes.
/// </summary>
void CellClass::Place_Veins(void)
{
	Rect rect(0, 0, 0, 0);

	if (Can_Place_Veins()) {

		rect = Union(rect, Union(Overlay_Render_Rect(), Overlay_Shadow_Render_Rect()));

		Map.Radar_Background(CellID);
		Overlay = OVERLAY_VEINS;

		if (Ramp == RAMP_NONE) {
			OverlayData = abs(Scen->RandomNumber()) % 3 + OVERLAYDATA_FIRST_SOLID_VEIN;

			for (FacingType dir = FACING_N; dir < FACING_COUNT; dir = FacingType(dir + FACING_90)) {
				CellClass & adjacent = Adjacent_Cell(dir);

				/*
				 * Skip neighbors that are already solid veins, or that carry some other
				 * vein-type overlay.
				 */
				if (adjacent.Overlay == OVERLAY_VEINS && adjacent.OverlayData >= OVERLAYDATA_FIRST_SOLID_VEIN) {
					continue;
				}
				if (adjacent.Overlay != OVERLAY_VEINS && adjacent.Overlay != OVERLAY_NONE && OverlayTypes[adjacent.Overlay]->IsVeins) {
					continue;
				}

				if (adjacent.Ramp == RAMP_NONE) {
					int frame = adjacent.Get_Vein_Frame();
					if (adjacent.Overlay == OVERLAY_NONE || (frame <= 15 && adjacent.OverlayData / 3 != frame)) {
						Rect adjrect(0, 0, 0, 0);
						adjrect = Union(adjrect, Union(adjacent.Overlay_Render_Rect(), adjacent.Overlay_Shadow_Render_Rect()));
						adjacent.Overlay = OVERLAY_VEINS;
						adjacent.OverlayData = 3 * frame + abs(Scen->RandomNumber() % 3);
						adjacent.Recalc_Attributes();
						Point2D origin = TacticalRect.TopLeft;
						adjrect = Union(adjrect, Union(adjacent.Overlay_Render_Rect(), adjacent.Overlay_Shadow_Render_Rect())) - origin;
						TacticalMap->Register_Dirty_Area(adjrect);
					}
				} else {
					Rect adjrect(0, 0, 0, 0);
					adjrect = Union(adjrect, Union(adjacent.Overlay_Render_Rect(), adjacent.Overlay_Shadow_Render_Rect()));
					adjacent.Overlay = OVERLAY_VEINS;
					adjacent.OverlayData = 2 * adjacent.Ramp + (abs(Scen->RandomNumber()) & 1) + OVERLAYDATA_FIRST_RAMP_VEIN;
					adjrect = Union(adjrect, Union(adjacent.Overlay_Render_Rect(), adjacent.Overlay_Shadow_Render_Rect())) - TacticalRect.TopLeft;
					TacticalMap->Register_Dirty_Area(adjrect);
				}

				Map.Radar_Background(adjacent.CellID);
			}

			Trigger_Veins();
		} else {
			OverlayData = 2 * Ramp + (abs(Scen->RandomNumber()) & 1) + OVERLAYDATA_FIRST_RAMP_VEIN;
		}

		rect = Union(rect, Union(Overlay_Render_Rect(), Overlay_Shadow_Render_Rect())) - TacticalRect.TopLeft;
		TacticalMap->Register_Dirty_Area(rect);
		Recalc_Attributes();
	}
}


/// <summary>
/// Is the tile on this cell plain clear ground?
/// A cell that has never been given a tile counts as clear.
/// </summary>
/// <returns>bool; Is this a clear tile?</returns>
bool CellClass::Is_Tile_Clear(void) const
{
	if (ITType == ISOTILE_NONE || ITType == ISOTILE_CLEAR) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Is the tile on this cell a ramp?
/// Both the plain ramps and the smoothed ramp transitions count.
/// </summary>
/// <returns>bool; Is this a ramp tile?</returns>
bool CellClass::Is_Tile_Ramp(void) const
{
	if (IS_TILE_IN_SET(RampStart, RAMP_BASE_COUNT) ||
		IS_TILE_IN_SET(RampSmooth, RAMP_SMOOTH_COUNT)) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Is the tile on this cell a cliff?
/// This routine covers every rock face the theater provides -- plain cliffs, cliff ramps,
/// water caves, waterfalls, destroyable cliffs and the bridge pieces cut into them. The
/// walkable subtiles at the ends of a waterfall are the one exception.
/// </summary>
/// <returns>bool; Is this a cliff tile?</returns>
bool CellClass::Is_Tile_Cliff(void) const
{
	if (IS_TILE_IN_VALID_SET(CliffSet, CLIFF_COUNT)) return(true);

	if (IS_TILE_IN_VALID_SET(WaterfallEast, WATERFALL_EAST_COUNT)) {
		if (ITType == IsometricTileTypeClass::WaterfallEast || ITType == IsometricTileTypeClass::WaterfallEast + WATERFALL_EAST_COUNT-1) {
			if (SubTile == 0 || SubTile == 4) {
				return(false);
			}
		}
		return(true);
	}

	if (IS_TILE_IN_VALID_SET(WaterfallWest, WATERFALL_WEST_COUNT)) {
		if (ITType == IsometricTileTypeClass::WaterfallWest || ITType == IsometricTileTypeClass::WaterfallWest + WATERFALL_WEST_COUNT-1) {
			if (SubTile == 1 || SubTile == 3) {
				return(false);
			}
		}
		return(true);
	}

	if (IS_TILE_IN_VALID_SET(WaterfallSouth, WATERFALL_SOUTH_COUNT)) {
		if (ITType == IsometricTileTypeClass::WaterfallSouth || ITType == IsometricTileTypeClass::WaterfallSouth + WATERFALL_SOUTH_COUNT-1) {
			if (SubTile == 0 || SubTile == 1) {
				return(false);
			}
		}
		return(true);
	}

	if (IS_TILE_IN_VALID_SET(WaterfallNorth, WATERFALL_NORTH_COUNT)) {
		if (ITType == IsometricTileTypeClass::WaterfallNorth || ITType == IsometricTileTypeClass::WaterfallNorth + WATERFALL_NORTH_COUNT-1) {
			if (SubTile == 2 || SubTile == 3) {
				return(false);
			}
		}
		return(true);
	}

	if (IS_TILE_IN_VALID_SET(CliffRamps, CLIFF_RAMPS_COUNT)) return(true);
	if (IS_TILE_IN_VALID_SET(WaterCaves, WATER_CAVES_COUNT)) return(true);
	if (IS_TILE_IN_VALID_SET(BridgeSet, BRIDGE_COUNT)) return(true);
	if (IS_TILE_IN_VALID_SET(TrainBridgeSet, TRAIN_BRIDGE_COUNT)) return(true);
	if (IS_TILE_IN_VALID_SET(DestroyableCliffs, DESTROYABLE_CLIFFS_COUNT)) return(true);
	if (IS_TILE_IN_VALID_SET(WaterCliffs, WATER_CLIFFS_COUNT)) return(true);

	return(false);
}


/// <summary>
/// Is the tile on this cell a shoreline piece?
/// </summary>
/// <returns>bool; Is this a shore tile?</returns>
bool CellClass::Is_Tile_Shore(void) const
{
	if (IS_TILE_IN_SET(ShorePieces, SHORE_PIECES_COUNT)) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Does the tile on this cell hold any water?
/// Shore pieces, open water and the waterfalls all qualify. Use this routine when any wet
/// part of the tile matters, rather than a tile that is water all the way across.
/// </summary>
/// <returns>bool; Is there water in this tile?</returns>
bool CellClass::Is_Tile_With_Water(void) const
{
	if (Is_Tile_Shore()) {
		return(true);
	}

	if (IS_TILE_IN_SET(WaterSet, WATER_COUNT) ||
		IS_TILE_IN_SET(WaterfallEast, WATERFALL_EAST_COUNT) ||
		IS_TILE_IN_SET(WaterfallWest, WATERFALL_WEST_COUNT) ||
		IS_TILE_IN_SET(WaterfallSouth, WATERFALL_SOUTH_COUNT) ||
		IS_TILE_IN_SET(WaterfallNorth, WATERFALL_NORTH_COUNT)) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Is the tile on this cell swamp?
/// The water to swamp transition pieces count as swamp as well. A theater that has no
/// swamp tiles at all always answers no.
/// </summary>
/// <returns>bool; Is this a swamp tile?</returns>
bool CellClass::Is_Tile_Swamp(void) const
{
	if (IS_SET_VALID(SwampTile)) {
		if (IS_TILE_IN_SET(SwampTile, SWAMP_COUNT) ||
			IS_TILE_IN_SET(WaterToSwampLat, 16)) {
			return(true);
		}
	}

	return(false);
}


/// <summary>
/// Is the tile on this cell one of the miscellaneous pavement pieces?
/// </summary>
/// <returns>bool; Is this a miscellaneous pavement tile?</returns>
bool CellClass::Is_Tile_Misc_Pavement(void) const
{
	if (IS_TILE_IN_SET(MiscPaveTile, MISC_PAVE_TILE_COUNT)) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Is the tile on this cell pavement?
/// </summary>
/// <returns>bool; Is this a pavement tile?</returns>
bool CellClass::Is_Tile_Pavement(void) const
{
	if (IS_TILE_IN_SET(PaveTile, PAVE_TILE_COUNT)) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Is the tile on this cell a dirt road?
/// Junctions, curves and straight runs all count as dirt road.
/// </summary>
/// <returns>bool; Is this a dirt road tile?</returns>
bool CellClass::Is_Tile_Dirt_Road(void) const
{
	if (IS_TILE_IN_SET(DirtRoadJunction, DIRT_ROAD_JUNCTION_COUNT) ||
		IS_TILE_IN_SET(DirtRoadCurve, DIRT_ROAD_CURVE_COUNT) ||
		IS_TILE_IN_SET(DirtRoadStraight, DIRT_ROAD_STRAIGHT_COUNT)) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Is the tile on this cell a paved road?
/// </summary>
/// <returns>bool; Is this a paved road tile?</returns>
bool CellClass::Is_Tile_Paved_Road(void) const
{
	if (IS_TILE_IN_SET(PavedRoads, PAVED_ROAD_COUNT)) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Is the tile on this cell a paved road end cap?
/// </summary>
/// <returns>bool; Is this a paved road end tile?</returns>
bool CellClass::Is_Tile_Paved_Road_End(void) const
{
	if (IS_TILE_IN_SET(PavedRoadEnds, PAVED_ROAD_ENDS_COUNT)) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Is the tile on this cell a sloped paved road?
/// These are the road pieces that carry a paved road up or down a ramp.
/// </summary>
/// <returns>bool; Is this a paved road slope tile?</returns>
bool CellClass::Is_Tile_Paved_Road_Slope(void) const
{
	if (IS_TILE_IN_SET(PavedRoadSlopes, PAVED_ROAD_SLOPES_COUNT)) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Is the tile on this cell a road median?
/// These are the divider strips laid between the lanes of a paved road.
/// </summary>
/// <returns>bool; Is this a road median tile?</returns>
bool CellClass::Is_Tile_Road_Median(void) const
{
	if (IS_TILE_IN_SET(Medians, 14)) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Is the tile on this cell one of the ice tiles?
/// Every thickness of ice the theater provides counts.
/// </summary>
/// <returns>bool; Is this an ice tile?</returns>
bool CellClass::Is_Tile_Ice(void) const
{
	if (ITType >= IsometricTileTypeClass::Ice1Set && ITType < IsometricTileTypeClass::Ice2Set + ICE2_COUNT) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Is the tile on this cell part of a road bridge?
/// </summary>
/// <returns>bool; Is this a bridge tile?</returns>
bool CellClass::Is_Tile_Bridge(void) const
{
	if (IS_TILE_IN_SET(BridgeSet, BRIDGE_COUNT)) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Is the tile on this cell part of a railway bridge?
/// </summary>
/// <returns>bool; Is this a train bridge tile?</returns>
bool CellClass::Is_Tile_Train_Bridge(void) const
{
	if (IS_TILE_IN_SET(TrainBridgeSet, TRAIN_BRIDGE_COUNT)) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Is the tile on this cell a clear to sand transition?
/// These are the blend tiles the smoothing pass lays down where clear ground meets sand.
/// </summary>
/// <returns>bool; Is this one of the clear to sand transition tiles?</returns>
bool CellClass::Is_Tile_Clear_To_Sand_LAT(void) const
{
	if (IS_TILE_IN_SET(ClearToSandLat, 16)) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Is the tile on this cell a clear to green transition?
/// These are the blend tiles the smoothing pass lays down where clear ground meets green
/// terrain.
/// </summary>
/// <returns>bool; Is this one of the clear to green transition tiles?</returns>
bool CellClass::Is_Tile_Clear_To_Green_LAT(void) const
{
	if (IS_TILE_IN_SET(ClearToGreenLat, 16)) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Should this cell be drawn as if it were cloaked?
/// This routine is used by the display code to decide whether the cell's contents may be
/// shown to the local player, taking any sensor coverage the player has into account.
/// </summary>
/// <param name="house">The house whose cloaking is to be considered.</param>
/// <returns>bool; Should the cell be drawn cloaked?</returns>
bool CellClass::Should_Draw_As_Cloaked(HousesType house) const
{
	if (PlayerPtr != NULL) {
		if (Is_Cloaked(house)) {
			if (house == PlayerPtr->HeapID){
				return(true);
			}
			if (!Is_Sensed(PlayerPtr->HeapID)) {
				return(true);
			}
		}
	}
	return(false);
}


/// <summary>
/// Fetches the coordinate of the center of this cell.
/// The height component follows the lay of the land, so the coordinate sits on the terrain
/// surface rather than at sea level.
/// </summary>
/// <returns>Returns with the coordinate of the cell's center point.</returns>
Coord CellClass::Center_Coord(void) const
{
	Point2D pt = Point2D((CellID.X * CELL_LEPTON_W) + CELL_LEPTON_W / 2, (CellID.Y * CELL_LEPTON_H) + CELL_LEPTON_H / 2);
	Coord coord;
	coord.X = pt.X;
	coord.Y = pt.Y;
	coord.Z = Get_Height(pt);
	return(coord);
}


/// <summary>
/// Fetches the coordinate of this cell, allowing for a bridge overhead.
/// Use this routine rather than Center_Coord when the caller wants the surface an object
/// would actually stand on -- for a cell that lies under a bridge, that is the deck.
/// </summary>
/// <returns>Returns with the coordinate of the cell's center.</returns>
Coord CellClass::As_Coord(void) const
{
	if (IsUnderBridge) {
		return(Center_Coord() + Coord(0, 0, BRIDGE_LEPTON_HEIGHT));
	}

	return(Center_Coord());
}


/// <summary>
/// Is the tile on this cell a destroyable cliff?
/// </summary>
/// <returns>bool; Is this a cliff face that can be collapsed?</returns>
bool CellClass::Is_Tile_Destroyable_Cliff(void) const
{
	if (ITType == IsometricTileTypeClass::DestroyableCliffs + DESTROYABLE_CLIFFS_COUNT-2) {
		return(true);
	}

	if (ITType == IsometricTileTypeClass::DestroyableCliffs + DESTROYABLE_CLIFFS_COUNT-1) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Sets the veins on this cell upon whoever is standing in them.
/// This routine will attach the vein attack animation to the cell when an occupier is low
/// enough to the ground and has no immunity to the veins.
/// </summary>
void CellClass::Trigger_Veins(void)
{
	if (Overlay == OVERLAY_VEINS && OverlayData >= OVERLAYDATA_FIRST_SOLID_VEIN && Ramp == RAMP_NONE && !IsAnimAttached) {
		ObjectClass * occupier = Cell_Occupier();
		while (occupier != NULL) {
			if (occupier->HeightAGL <= 5 && occupier->Is_Techno() &&
				!occupier->TClass->IsImmuneToVeins && !((TechnoClass *)occupier)->Has_Ability(ABILITY_VEIN_PROOF)) {

				new AnimClass(Rule->VeinAttack, Cell_Coord() - Coord(CELL_LEPTON_W / 2, CELL_LEPTON_H / 2));
				IsAnimAttached = true;
			}
			occupier = occupier->Next;
		}
	}
}


/// <summary>
/// Hides this cell under the fog of war.
/// This routine takes a snapshot of the structures standing here so that they can still be
/// drawn while the cell is fogged, and deselects the mobile objects the player is no
/// longer entitled to see. It does nothing unless fog of war is enabled for the scenario.
/// </summary>
void CellClass::Fog_Cell(void)
{
	if (Scen->Special.IsFogOfWar) {
		Cell cell = CellID;

		for (int height = 1; height <= 15; height += 2) {
			CellClass *cptr = &Map[cell];

			if (cptr->Height >= height - 2 && cptr->Height <= height) {
				if (!cptr->IsFogged) {
					FOGGED_OBJECT_LIST * fogged_objects = new FOGGED_OBJECT_LIST;
					fogged_objects->Resize(1);
					fogged_objects->Set_Growth_Step(1);

					cptr->IsFogged = true;
					ObjectClass * occupier = cptr->Cell_Occupier();

					while (occupier != NULL) {
						int rtti = occupier->What_Am_I();
						if (rtti == RTTI_BUILDING) {
							BuildingClass * building = (BuildingClass *)occupier;
							building->PositionCell;
							if (building->Should_Fog()) {
								bool fade;
								if (building->Considered_Vehicle() || building->TranslucencyLevel == 15) {
									fade = false;
								} else {
									fade = true;
								}
								building->Make_Fogged(fogged_objects, cptr, fade);
							}
						} else if (rtti == RTTI_UNIT || rtti == RTTI_AIRCRAFT || rtti == RTTI_INFANTRY) {
							occupier->Unselect();
						}
						occupier = occupier->Next;
					}

					if (fogged_objects->Count() > 0) {
						cptr->FoggedObjects = fogged_objects;
					} else {
						delete fogged_objects;
					}
				}
			}
			cell += Cell(1,1);
		}
	}
}


/// <summary>
/// Lifts the fog of war from this cell.
/// This routine is the counterpart of Fog_Cell. The stale snapshots taken while the cell
/// was hidden are thrown away so the real objects show through again.
/// </summary>
void CellClass::Unfog_Cell(void)
{
	Cell cell = CellID;
	for (int i = 1; i < 15; i += 2) {
		CellClass *cptr = &Map[cell];
		int height = cptr->Height;
		if (height >= i - 2 && height <= i) {
			cptr->IsFogged = false;
			cptr->Remove_Fogged_Objects();
		}
		cell += Cell(1,1);
	}
}


/// <summary>
/// Discards the fogged object snapshots attached to this cell.
/// This routine is called as the cell is revealed again. A snapshot that spans several
/// cells is unhooked from all of them before it is destroyed.
/// </summary>
void CellClass::Remove_Fogged_Objects(void)
{
	if (FoggedObjects != NULL) {
		for (int i = FoggedObjects->Count() - 1; i >= 0; i--) {
			FoggedObjectClass * obj = (*FoggedObjects)[i];
			if (obj->Get_Head_Record_Occupy_List() != NULL) {
				Cell const * list = obj->Get_Head_Record_Occupy_List();
				while (*list != REFRESH_EOL) {
					Coord coord = obj->Position;
					Cell cell = *list + coord.As_Cell();
					CellClass * cptr = &Map[cell];
					if (cptr != this && cptr->FoggedObjects != NULL) {
						cptr->FoggedObjects->Delete(obj);
					}
					list++;
				}
			}
			delete obj;
		}
		FoggedObjects->Clear();
		delete FoggedObjects;
		FoggedObjects = NULL;
	}
}


/// <summary>
/// Fetches the pattern of neighboring cells occupied by the house specified.
/// This routine is used where the shape of a house's presence around a cell matters, such
/// as when deciding how a wall or fence should connect to its neighbors.
/// </summary>
/// <returns>Returns with a facing bit mask of the adjacent cells that house occupies. If
/// the house does not occupy this cell at all, -1 is returned.</returns>
int CellClass::Occupation_Mask(HousesType house) const
{
	if (!(OccupiedBy & (1 << house))) {
		return(-1);
	}

	int mask = 0;
	for (int dir = 0; dir < FACING_COUNT; dir++) {
		CellClass const & adjacent = Adjacent_Cell(FacingType(dir));
		if (adjacent.OccupiedBy & (1 << house)) {
			mask |= (1 << dir);
		}
	}
	return(mask);
}


/// <summary>
/// Removes some of the weed growth from this cell.
/// If a veinhole monster owns the veins here, it is told to shrink them; otherwise the
/// leftover vein artwork is merely refreshed.
/// </summary>
int CellClass::Reduce_Weed(void)
{
	VeinholeMonsterClass * veinhole = VeinholeMonsterClass::Get_Vein_Owner_At(CellID);
	if (veinhole != NULL) {
		veinhole->Reduce_Veins_At(this);
	} else {
		Map[CellID].Redraw_Veins();
	}
	return(0);
}


/// <summary>
/// Flags this cell and everything laid over it as needing a redraw.
/// This routine will hand the cell's render area to the tactical map as a dirty region and
/// refresh the matching radar pixel.
/// </summary>
void CellClass::Register_For_Redraw(void)
{
	Rect rect = Union(Union(Cell_Render_Rect(), Overlay_Render_Rect()), Overlay_Shadow_Render_Rect());
	TacticalMap->Register_Dirty_Area(rect - TacticalRect.TopLeft);
	Map.Radar_Background(CellID);
}


/// <summary>
/// Can a subterranean unit surface at this cell?
/// This routine is used by the tunnel locomotion when it hunts for somewhere to dig out.
/// The ground must allow burrowing and be clear of slopes, bridges, buildings and terrain.
/// </summary>
/// <returns>bool; Can a unit burrow up through this cell?</returns>
bool CellClass::Can_Burrow_Here(void) const
{
	if (!Map.In_Local_Radar(this)) {
		return(true);
	}

	if (ITType < ISOTILE_FIRST || ITType >= IsometricTileTypes.Count() || IsometricTileTypes[ITType]->IsAllowBurrowing) {
		if (Ramp == 0 && !IsUnderBridge && !WasUnderBridge) {
			if (Cell_Building() != NULL) {
				return(false);
			}
			if (Cell_Terrain() != NULL) {
				return(false);
			}
			return(true);
		}
	}
	return(false);
}


/// <summary>
/// Determines if the house specified has cloaked this cell.
/// </summary>
/// <returns>bool; Is the cell cloaked by that house?</returns>
bool CellClass::Is_Cloaked(HousesType house) const
{
	return((CloakedBy & (1 << house)) != 0);
}


/// <summary>
/// Determines if the house specified has a sensor covering this cell.
/// </summary>
/// <returns>bool; Is the cell sensed by that house?</returns>
bool CellClass::Is_Sensed(HousesType house) const
{
	return((SensedBy & (1 << house)) != 0);
}


/// <summary>
/// Marks this cell as cloaked by the house specified.
/// This routine is called as a cloaking field is laid down over the cell.
/// </summary>
void CellClass::Cloaked_By(HousesType house)
{
	CloakedBy |= (1 << house);
}


/// <summary>
/// Clears the cloak flag for the house specified.
/// This routine is called as a cloaking field is lifted from this cell.
/// </summary>
void CellClass::Uncloaked_By(HousesType house)
{
	CloakedBy &= ~(1 << house);
}


/// <summary>
/// Marks this cell as covered by a sensor of the house specified.
/// A sensed cell gives away any cloaked object standing on it to that house.
/// </summary>
void CellClass::Sensed_By(HousesType house)
{
	SensedBy |= (1 << house);
}


/// <summary>
/// Clears the sensor flag for the house specified.
/// This routine is called as a sensor field is lifted from this cell, letting cloaked
/// objects standing here hide from that house once more.
/// </summary>
void CellClass::Unsensed_By(HousesType house)
{
	SensedBy &= ~(1 << house);
}


/// <summary>
/// Places tiberium growth on this cell.
/// This routine is used by the tiberium growth and spread logic. A bare cell germinates a
/// fresh patch here; a cell that already carries the same type has its patch thickened
/// instead.
/// </summary>
/// <param name="tib">The type of tiberium to place.</param>
/// <param name="data">The growth stage to place, or to add to an existing patch.</param>
/// <returns>bool; Was tiberium planted or grown here?</returns>
bool CellClass::Place_Tiberium(TiberiumType tib, int data)
{
	TiberiumClass * tiberium = Tiberiums[tib];
	if (data < tiberium->FrameCount) {
		if (Can_Tiberium_Germinate(tiberium)) {
			if (Ramp != RAMP_NONE) {
				new OverlayClass(OverlayTypes[tiberium->Overlay->HeapID + tiberium->Variety + 2 * Ramp + (Random_Pick(0, 1) - 2)], Fetch_CellID());
			} else {
				new OverlayClass(OverlayTypes[tiberium->Overlay->HeapID + Random_Pick(0, 11)], Fetch_CellID());
			}
			tiberium->Queue_Growth(CellID);
			OverlayData = data;
			Register_For_Redraw();
			return(true);
		}

		if (Can_Tiberium_Grow()) {
			if (Tiberium_Type_Here() == tib) {
				OverlayData += data;
				OverlayData = std::min<int>(OverlayData, tiberium->FrameCount - 1);
				Rect rect = Union(Union(Cell_Render_Rect(), Overlay_Render_Rect()), Overlay_Shadow_Render_Rect());
				TacticalMap->Register_Dirty_Area(rect - TacticalRect.TopLeft);
				tiberium->Queue_Spread(CellID);
				return(true);
			}
		}
	}
	return(false);
}


/// <summary>
/// Kills whatever was traveling across a bridge that has just fallen.
/// This routine is called as a bridge span collapses. Foot units in the neighborhood that
/// were headed for the vanished deck are dragged down with it.
/// </summary>
void CellClass::On_Bridge_Collapse(void)
{
	if (BridgeFlag.Composite) {
		Coord coord = CellID.As_Coord();
		coord.Z = Map.Get_Height_GL(coord) + BRIDGE_LEPTON_HEIGHT;

		for (int x = -2; x < 3; x++) {
			for (int y = -2; y < 3; y++) {

				Cell cell = CellID + Cell(x, y);
				CellClass *cellptr = &Map[cell];

				if (cellptr != this) {
					ObjectClass *optr = NULL;
					ObjectClass *next = NULL;

					optr = cellptr->Cell_Occupier(true);
					while (optr != NULL) {
						next = optr->Next;
						if (optr->Is_Foot()) {
							FootClass *fptr = (FootClass *)optr;
							if (fptr->Locomotion->Is_Moving_Here(coord)) {
								int damage = fptr->Strength;
								fptr->Take_Damage(damage, 0, Rule->C4Warhead, NULL, true, true);
							}
						}
						optr = next;
					}

					optr = cellptr->Cell_Occupier(false);
					while (optr != NULL) {
						next = optr->Next;
						if (optr->Is_Foot()) {
							FootClass *fptr = (FootClass *)optr;
							if (fptr->Locomotion->Is_Moving_Here(coord)) {
								int damage = fptr->Strength;
								fptr->Take_Damage(damage, 0, Rule->C4Warhead, NULL, true, true);
							}
						}
						optr = next;
					}
				}
			}
		}
	}
}


/// <summary>
/// Determines if this cell is still hidden by the shroud.
/// </summary>
/// <returns>bool; Is the cell shrouded?</returns>
bool CellClass::Is_Shrouded(void) const
{
	return(Map.Is_Shrouded(Cell_Coord()));
}


/// <summary>
/// Determines if this cell lies under the fog of war.
/// </summary>
/// <returns>bool; Is the cell fogged?</returns>
bool CellClass::Is_Fogged(void) const
{
	return(Map.Is_Fogged(Cell_Coord()));
}


/// <summary>
/// Destroys anything that has no legal business being in this cell.
/// This routine is called after the ground under a cell changes. Occupiers that could no
/// longer enter the cell are killed outright, as are any nearby foot units that were in
/// the middle of moving into it.
/// </summary>
void CellClass::Kill_Illegal_Occupiers(void)
{
	ObjectClass * object = Cell_Occupier();
	ObjectClass *next = NULL;

	while (object) {
		next = object->Next;
		if (object->Can_Enter_Cell(this, FACING_NONE) == MOVE_NO) {
			int damage = object->Strength;
			object->Take_Damage(damage, 0, Rule->C4Warhead, NULL, true, true);
		}
		object = next;
	}

	Coord coord = Cell_Coord();
	for (int x = -2; x < 3; x++) {
		for (int y = -2; y < 3; y++) {

			CellClass * cellptr = &Map[CellID + Cell(x, y)];
			if (cellptr != this) {
				object = cellptr->Cell_Occupier();
				while (object) {
					if (object->Is_Foot()) {
						FootClass * foot = (FootClass *)object;
						if (foot->Locomotion->Is_Moving_Here(coord) && foot->Can_Enter_Cell(this, FACING_NONE) == MOVE_NO) {
							int damage = foot->Strength;
							foot->Take_Damage(damage, 0, Rule->C4Warhead, NULL, true, true);
						}
					}
					object = object->Next;
				}
			}
		}
	}
}


/// <summary>
/// Can a structure be placed on this cell?
/// This routine is used by the building placement logic. Sloped ground, a cell that is
/// already built upon, and land that does not support construction are all refused.
/// </summary>
/// <returns>bool; Can something be built here?</returns>
bool CellClass::Can_Build_Here(void) const
{
	if (Ramp) {
		return(false);
	}

	if (GameActive) {
		BuildingClass * building = Cell_Building();
		if (building) {
			return(false);
		}
	}

	return(Ground[Land].Build);
}


/// <summary>
/// Determines what kind of object this is.
/// This routine is the interface counterpart of Fetch_RTTI, and is used by outside code
/// that only ever sees the cell through its abstract interface.
/// </summary>
/// <returns>Returns with RTTI_CELL.</returns>
int CellClass::What_Am_I(void) const
{
	return(RTTI_CELL);
}


/// <summary>
/// Fetches the RTTI type of this object.
/// </summary>
/// <returns>Returns with RTTI_CELL.</returns>
RTTIType CellClass::Fetch_RTTI(void) const
{
	return(RTTI_CELL);
}
