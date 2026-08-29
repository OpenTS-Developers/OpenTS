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

#define INCLUDE_COM
#include "always.h"

#include "isotype.h"

#include "_alpha.h"
#include "_convert.h"
#include "_map.h"
#include "_mixfile.h"
#include "_palette.h"
#include "_rules.h"
#include "_surface.h"
#include "_theater.h"
#include "_zbuffer.h"
#include "abuffer.h"
#include "alphalighting.h"
#include "animtype.h"
#include "ccfile.h"
#include "ccini.h"
#include "cell.h"
#include "conquer.h"
#include "dbgprint.h"
#include "draw.h"
#include "dsurface.h"
#include "globals.h"
#include "isotile.h"
#include "lightcon.h"
#include "mixfile.h"
#include "palette.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "shapeset.h"
#include "sun.h"
#include "swizzle.h"
#include "tracker.h"
#include "vector.h"
#include "zbuffer.h"

#include <algorithm>
#include <limits>
#include <vector>

enum {
	ISO_WIDTH = 48,
	ISO_HEIGHT = 24,
	ISO_DRAW_WIDTH = ISO_WIDTH,
	ISO_DRAW_HEIGHT = ISO_HEIGHT-1,
};

/// Handy macro to shorten all the tileset checks
#define IS_SET_VALID(setname) (IsometricTileTypeClass::setname != ISOTILE_INVALID)
#define IS_TILE_IN_SET(setname, setsize) (ITType >= IsometricTileTypeClass::setname && ITType < IsometricTileTypeClass::setname + setsize)
#define IS_TILE_IN_VALID_SET(setname, setsize) (IS_SET_VALID(setname) && IS_TILE_IN_SET(setname, setsize))

/*
 * Z-depth shape data for the four straight ramp directions, loaded from
 * SLOP01Z through SLOP04Z for the current theater. Passed to Draw_Shape
 * as the z-shape file for depth-correct rendering on ramped cells,
 * indexed as SlopeZShapes[Ramp - 1].
 */
ShapeSet * SlopeZShapes[4];

void * IsometricTileTypeClass::CellShadowShapes;

extern void Clamp_Tile_RGB(int & r, int & g, int & b);


PaletteClass IsoTilePalette;

DynamicVectorClass<IsometricTileTypeClass::TileInsertType *> IsometricTileTypeClass::TileInsertTypes;

struct ShadowCasterInfo {
	int Frame;
	int Index;
	int XOffset;
	int YOffset;

	ShadowCasterInfo(int frame = 0, int index = 0, int x = 0, int y = 0) : Frame(frame), Index(index), XOffset(x), YOffset(y) {}
};


ShadowCasterInfo ShadowCasterCliffInfo[CLIFF_COUNT] = {
	/* Cliff 00 */ ShadowCasterInfo(),
	/* Cliff 01 */ ShadowCasterInfo(),
	/* Cliff 02 */ ShadowCasterInfo(),
	/* Cliff 03 */ ShadowCasterInfo(),
	/* Cliff 04 */ ShadowCasterInfo(),
	/* Cliff 05 */ ShadowCasterInfo(),
	/* Cliff 06 */ ShadowCasterInfo(),
	/* Cliff 07 */ ShadowCasterInfo(),
	/* Cliff 08 */ ShadowCasterInfo(),
	/* Cliff 09 */ ShadowCasterInfo(),
	/* Cliff 10 */ ShadowCasterInfo(),
	/* Cliff 11 */ ShadowCasterInfo(),
	/* Cliff 12 */ ShadowCasterInfo(),
	/* Cliff 13 */ ShadowCasterInfo(),
	/* Cliff 14 */ ShadowCasterInfo(),
	/* Cliff 15 */ ShadowCasterInfo(),
	/* Cliff 16 */ ShadowCasterInfo(),
	/* Cliff 17 */ ShadowCasterInfo(0,0,ISO_WIDTH/2,ISO_HEIGHT),
	/* Cliff 18 */ ShadowCasterInfo(0,0,ISO_WIDTH/2,ISO_HEIGHT),
	/* Cliff 19 */ ShadowCasterInfo(0,0,ISO_WIDTH/2,ISO_HEIGHT),
	/* Cliff 20 */ ShadowCasterInfo(1,0,ISO_WIDTH/2,ISO_HEIGHT),
	/* Cliff 21 */ ShadowCasterInfo(1,0,ISO_WIDTH/2,ISO_HEIGHT),
	/* Cliff 22 */ ShadowCasterInfo(2,1,ISO_WIDTH,ISO_HEIGHT/2),
	/* Cliff 23 */ ShadowCasterInfo(3,1,ISO_WIDTH,ISO_HEIGHT/2),
	/* Cliff 24 */ ShadowCasterInfo(4,1,ISO_WIDTH,ISO_HEIGHT/2),
	/* Cliff 25 */ ShadowCasterInfo(5,0,(ISO_WIDTH/2)*3,ISO_HEIGHT),
	/* Cliff 26 */ ShadowCasterInfo(6,0,ISO_WIDTH,ISO_HEIGHT/2),
	/* Cliff 27 */ ShadowCasterInfo(7,1,ISO_WIDTH/2,0),
	/* Cliff 28 */ ShadowCasterInfo(8,0,ISO_WIDTH,ISO_HEIGHT/2),
	/* Cliff 29 */ ShadowCasterInfo(9,0,ISO_WIDTH,ISO_HEIGHT/2),
	/* Cliff 30 */ ShadowCasterInfo(10,1,0,-(ISO_HEIGHT/2)),
	/* Cliff 31 */ ShadowCasterInfo(11,1,0,-(ISO_HEIGHT/2)),
	/* Cliff 32 */ ShadowCasterInfo(12,0,ISO_WIDTH,ISO_HEIGHT/2),
	/* Cliff 33 */ ShadowCasterInfo(),
	/* Cliff 34 */ ShadowCasterInfo(),
	/* Cliff 35 */ ShadowCasterInfo(),
	/* Cliff 36 */ ShadowCasterInfo(),
	/* Cliff 37 */ ShadowCasterInfo(),
	/* Cliff 38 */ ShadowCasterInfo(),
	/* Cliff 39 */ ShadowCasterInfo(),
};


ShadowCasterInfo ShadowCasterSlopeInfo[SLOPE_COUNT] = {
	/* Slope 00 */ ShadowCasterInfo(),
	/* Slope 01 */ ShadowCasterInfo(),
	/* Slope 02 */ ShadowCasterInfo(),
	/* Slope 03 */ ShadowCasterInfo(),
	/* Slope 04 */ ShadowCasterInfo(13,6,ISO_WIDTH,ISO_HEIGHT/2),
	/* Slope 05 */ ShadowCasterInfo(),
	/* Slope 06 */ ShadowCasterInfo(14,1,ISO_WIDTH,ISO_HEIGHT/2),
	/* Slope 07 */ ShadowCasterInfo(),
	/* Slope 08 */ ShadowCasterInfo(),
	/* Slope 09 */ ShadowCasterInfo(),
};


IsometricTileType IsometricTileTypeClass::RampStart;
IsometricTileType IsometricTileTypeClass::RampSmooth;
IsometricTileType IsometricTileTypeClass::MMRampBase;
IsometricTileType IsometricTileTypeClass::ClearTile;
IsometricTileType IsometricTileTypeClass::RoughTile;
IsometricTileType IsometricTileTypeClass::SandTile;
IsometricTileType IsometricTileTypeClass::GreenTile;
IsometricTileType IsometricTileTypeClass::PaveTile;
IsometricTileType IsometricTileTypeClass::MiscPaveTile;
IsometricTileType IsometricTileTypeClass::ClearToRoughLat;
IsometricTileType IsometricTileTypeClass::ClearToSandLat;
IsometricTileType IsometricTileTypeClass::ClearToGreenLat;
IsometricTileType IsometricTileTypeClass::ClearToPaveLat;
IsometricTileType IsometricTileTypeClass::HeightBase;
IsometricTileType IsometricTileTypeClass::BlackTile;
IsometricTileType IsometricTileTypeClass::BridgeSet;
IsometricTileType IsometricTileTypeClass::TrainBridgeSet;
IsometricTileType IsometricTileTypeClass::CliffSet;
IsometricTileType IsometricTileTypeClass::ShorePieces;
IsometricTileType IsometricTileTypeClass::WaterSet;
IsometricTileType IsometricTileTypeClass::Ice1Set;
IsometricTileType IsometricTileTypeClass::Ice2Set;
IsometricTileType IsometricTileTypeClass::Ice3Set;
IsometricTileType IsometricTileTypeClass::IceShoreSet;
IsometricTileType IsometricTileTypeClass::SlopeSetPieces;
IsometricTileType IsometricTileTypeClass::SlopeSetPieces2;
IsometricTileType IsometricTileTypeClass::MonorailSlopes;
IsometricTileType IsometricTileTypeClass::Tunnels;
IsometricTileType IsometricTileTypeClass::TrackTunnels;
IsometricTileType IsometricTileTypeClass::DirtTunnels;
IsometricTileType IsometricTileTypeClass::DirtTrackTunnels;
IsometricTileType IsometricTileTypeClass::WaterfallEast;
IsometricTileType IsometricTileTypeClass::WaterfallWest;
IsometricTileType IsometricTileTypeClass::WaterfallNorth;
IsometricTileType IsometricTileTypeClass::WaterfallSouth;
IsometricTileType IsometricTileTypeClass::CliffRamps;
IsometricTileType IsometricTileTypeClass::PavedRoads;
IsometricTileType IsometricTileTypeClass::PavedRoadEnds;
IsometricTileType IsometricTileTypeClass::Medians;
IsometricTileType IsometricTileTypeClass::RoughGround;
IsometricTileType IsometricTileTypeClass::DirtRoadJunction;
IsometricTileType IsometricTileTypeClass::DirtRoadCurve;
IsometricTileType IsometricTileTypeClass::DirtRoadStraight;
IsometricTileType IsometricTileTypeClass::DestroyableCliffs;
IsometricTileType IsometricTileTypeClass::WaterCaves;
IsometricTileType IsometricTileTypeClass::WaterCliffs;
IsometricTileType IsometricTileTypeClass::PavedRoadSlopes;
IsometricTileType IsometricTileTypeClass::DirtRoadSlopes;
IsometricTileType IsometricTileTypeClass::Rocks;
IsometricTileType IsometricTileTypeClass::CrystalTile;
IsometricTileType IsometricTileTypeClass::ClearToCrystalLat;
IsometricTileType IsometricTileTypeClass::CrystalCliff;
IsometricTileType IsometricTileTypeClass::SwampTile;
IsometricTileType IsometricTileTypeClass::WaterToSwampLat;
IsometricTileType IsometricTileTypeClass::BlueMoldTile;
IsometricTileType IsometricTileTypeClass::ClearToBlueMoldLat;
IsometricTileType IsometricTileTypeClass::BridgeTopLeft1;
IsometricTileType IsometricTileTypeClass::BridgeTopLeft2;
IsometricTileType IsometricTileTypeClass::BridgeBottomRight1;
IsometricTileType IsometricTileTypeClass::BridgeBottomRight2;
IsometricTileType IsometricTileTypeClass::BridgeTopRight1;
IsometricTileType IsometricTileTypeClass::BridgeTopRight2;
IsometricTileType IsometricTileTypeClass::BridgeBottomLeft1;
IsometricTileType IsometricTileTypeClass::BridgeBottomLeft2;
IsometricTileType IsometricTileTypeClass::BridgeMiddle1;
IsometricTileType IsometricTileTypeClass::BridgeMiddle2;

IsometricTileType IsometricTileTypeClass::ShadowCasterTiles[5];


/***********************************************************************************************
 * TemplateTypeClass::TemplateTypeClass -- Constructor for template type objects.              *
 *                                                                                             *
 *    This is the constructor for the template types.                                          *
 *                                                                                             *
 * INPUT:   see below...                                                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
IsometricTileTypeClass::IsometricTileTypeClass(IsometricTileType type, int unknown1, unsigned char unknown2, char const * ininame, bool skip_registration) :
	BASECLASS(ininame),
	HeapID(type),
	MarbleMadness(ISOTILE_NONE),
	NonMarbleMadness(ISOTILE_NONE),
	TileSetBaseID(0),
	PreviewTiles(),
	NextTileTypeInSet(NULL),
	ToSnowTheater(-1),
	ToTemperateTheater(-1),
	Anim(ANIM_NONE),
	Offset(Point2D(0,0)),
	AttachesTo(-1),
	ZAdjust(0),
	Unused1((unsigned char)unknown1),
	IsMorphable(false),
	IsShadowCaster(false),
	IsAllowToPlace(true),
	IsRequiredForRMG(false),
	Width(0),
	Height(0),
	Unused2(unknown2),
	NumTileTypesInSet(1),
	IsFileLoaded(false),
	//Filename(),
	IsAllowBurrowing(true),
	IsAllowTiberium(false),
	UseCount(0)
{
	AbstractTypePtrTracker.Add(this);
	if (!skip_registration) {
		IsometricTileTypes.Add(this);
	}

	GivenName = ininame;

	Filename[0] = '\0';

	IsStealthy = true;
	IsSelectable = false;
	IsLegalTarget = false;
	IsInsignificant = true;
	IsImmune = true;
	IsFootprint = false;
}


/// <summary>
/// Destroys this tile type and everything hanging off it.
/// The rest of the variation chain goes with it, along with any artwork and preview colors
/// it owns, and the tile type is unhooked from the type heaps so that nothing is left
/// pointing at it.
/// </summary>
IsometricTileTypeClass::~IsometricTileTypeClass(void)
{
	Detach_This_From_All(this, true);

	IsometricTileTypeClass * next = NextTileTypeInSet;

	if (next != NULL) {
		NextTileTypeInSet = NULL;
		delete next;
	}

	if (ImageData != NULL && IsFileLoaded) {
		delete [] (unsigned char *)ImageData;
	}

	ImageData = NULL;

	IsometricTileTypes.Delete(this);
	AbstractTypePtrTracker.Delete(this);

	for (int i = PreviewTiles.Count() - 1; i >= 0; i--) {
		if (PreviewTiles[i] != NULL) {
			delete [] PreviewTiles[i];
		}
		PreviewTiles.Delete_Index(i);
	}
}


/***********************************************************************************************
 * TemplateTypeClass::Land_Type -- Determines land type from template and icon number.         *
 *                                                                                             *
 *    This routine will convert the specified icon number into the appropriate land type. The  *
 *    land type can be determined from the embedded colors in the "control template" section   *
 *    of the original art file. This control information is encoded into the icon data file    *
 *    to be retrieved and interpreted as the program sees fit. The engine only recognizes      *
 *    the first 16 colors as control colors, so the control map color value serves as an       *
 *    index into a simple lookup table.                                                        *
 *                                                                                             *
 * INPUT:   icon  -- The icon number within this template that is to be examined and used      *
 *                   to determine the land type.                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the land type that corresponds to the icon number specified.          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/12/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
LandType IsometricTileTypeClass::Land_Type(int tile) const
{
	IsoTileSet const * tileset = (IsoTileSet const *)Get_Image_Data();

	if (tileset != NULL) {
		IsoTileRecord const * record = tileset->Fetch_Record_Pointer(tile);
		if (record != NULL) {
			static LandType _land[16] = {
				LAND_CLEAR,
				LAND_ICE,
				LAND_ICE,
				LAND_ICE,
				LAND_ICE,
				LAND_TUNNEL,
				LAND_RAILROAD,
				LAND_ROCK,
				LAND_ROCK,			// Rock
				LAND_WATER,
				LAND_BEACH,
				LAND_ROAD,
				LAND_ROAD,
				LAND_CLEAR,
				LAND_ROUGH,			// Rough
				LAND_ROCK,
			};

			return(_land[record->TileType]);
		}
	}
	return(LAND_CLEAR);
}


/// <summary>
/// Does this tile set hold the specified sub-tile?
/// </summary>
/// <param name="tile">The sub-tile index to look for.</param>
/// <param name="load">Should the artwork be loaded first if it is not resident?</param>
/// <returns>bool; Does the sub-tile exist?</returns>
bool IsometricTileTypeClass::Is_Tile_Index_Valid(int tile, bool load)
{
	if (load) {
		Load_Tile_Image();
	}

	IsoTileSet const * tileset = (IsoTileSet const *)Get_Image_Data();
	if (tileset != NULL && tile < (tileset->Tile_Count()) && tileset->Tiles[tile] != NULL) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Ensures that this tile type's artwork is resident.
/// </summary>
/// <returns>bool; Is the artwork loaded and ready to draw?</returns>
bool IsometricTileTypeClass::Load_Tile_Image(void)
{
	if (ImageData == NULL && IsFileLoaded) {
		Load_Tile_Data();
	}
	return(ImageData != NULL);
}


/// <summary>
/// Fetches this tile type's artwork.
/// The artwork is loaded on demand, so a tile type whose image was trimmed out of memory
/// will quietly reload itself the moment something asks to draw it.
/// </summary>
/// <returns>Returns with a pointer to the tile set data. Otherwise, NULL is returned if
/// the artwork could not be loaded.</returns>
void const * IsometricTileTypeClass::Get_Image_Data(void) const
{
	void const * data = BASECLASS::Get_Image_Data();
	if (data == NULL && IsFileLoaded) {
		((IsometricTileTypeClass *)this)->Load_Tile_Data();
		data = BASECLASS::Get_Image_Data();
	}
	return(data);
}


/***********************************************************************************************
 * TemplateTypeClass::From_Name -- Determine template from ASCII name.                         *
 *                                                                                             *
 *    This routine is used to determine the template number given only                         *
 *    an ASCII representation. The scenario loader uses this routine                           *
 *    to construct the map from the INI control file.                                          *
 *                                                                                             *
 * INPUT:   name  -- Pointer to the ASCII name of the template.                                *
 *                                                                                             *
 * OUTPUT:  Returns with the template number. If the name had no match,                        *
 *          then returns with TEMPLATE_NONE.                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
IsometricTileType IsometricTileTypeClass::From_Name(char const * name)
{
	if (name != NULL) {
		for (int index = ISOTILE_FIRST; index < IsometricTileTypes.Count(); index++) {
			if (stricmp(IsometricTileTypes[index]->IniName, name) == 0) {
				return(IsometricTileType(index));
			}
		}
	}
	return(ISOTILE_NONE);
}


/***********************************************************************************************
 * TemplateTypeClass::Occupy_List -- Determines occupation list.                               *
 *                                                                                             *
 *    This routine is used to examine the template map and build an                            *
 *    occupation list. This list is used to render a template cursor as                        *
 *    well as placement of icon numbers.                                                       *
 *                                                                                             *
 * INPUT:   placement   -- Is this for placement legality checking only? The normal condition  *
 *                         is for marking occupation flags.                                    *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the template occupation list.                            *
 *                                                                                             *
 * WARNINGS:   The return pointer is valid only until the next time that                       *
 *             this routine is called.                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1994 JLB : Created.                                                                 *
 *   12/12/1995 JLB : Optimized for direct access to iconset data.                             *
 *=============================================================================================*/
Cell const * IsometricTileTypeClass::Occupy_List(bool placement) const
{
	static Cell _occupy[13*8+5+11];
	Cell	* ptr;

	IsoTileSet const * tileset = (IsoTileSet const *)Get_Image_Data();
	unsigned int const * map = (unsigned int const *)tileset->Tiles;

	ptr = &_occupy[0];
	for (int index = 0; index < Width * Height; index++) {
		if (*map++ != NULL) {
			*ptr++ = Cell(index % Width, index / Width);
		}
	}
	*ptr = REFRESH_EOL;

	return((Cell const *)&_occupy[0]);
}


/// <summary>
/// Fetches one of the variations of this tile type.
/// A tile type that has several visual variations keeps them on a chain, and this routine
/// walks along to the one asked for. The index wraps around the chain, so the caller may
/// hand in any value at all.
/// </summary>
/// <param name="index">The variation wanted.</param>
/// <returns>Returns with a pointer to the tile type for that variation.</returns>
IsometricTileTypeClass * IsometricTileTypeClass::Next_Tile_From_Set(int index)
{
	int num = NumTileTypesInSet;
	int i = index;
	if (i > num - 1) {
		i = i % num;
	}

	if (i == 0) {
		return(this);
	}

	IsometricTileTypeClass * next = this;
	do {
		next = next->NextTileTypeInSet;
	} while (--i != 0);
	return(next);
}


/// <summary>
/// Adjusts a tile type index for any inserted tile sets.
/// Extra tile sets can be spliced into the middle of the heap, which shifts everything
/// that follows them. Use this routine to bring an index that was recorded before the
/// insertion back into line with the heap as it now stands.
/// </summary>
/// <param name="current_type">The tile type index to adjust.</param>
/// <returns>Returns with the adjusted tile type index.</returns>
IsometricTileType IsometricTileTypeClass::Fixup_Tile_Type(IsometricTileType current_type)
{
	IsometricTileType new_type = current_type;
	if (current_type == ISOTILE_NONE) {
		return(new_type);
	}

	for (int i = 0; i < TileInsertTypes.Count(); i++) {
		TileInsertType *tileinsert = TileInsertTypes[i];
		if (tileinsert->Count > current_type) {
			break;
		}
		/*
		 * Apply the offset caused by this insert (shift tile ID forward).
		 */
		new_type += tileinsert->Offset;
	}
	return(new_type);
}


/// <summary>
/// Fetches a tile lighting converter for the specified tint.
/// This routine hands back the converter already built for that tint, making one if the
/// tint has not been asked for before. Cells share their converters this way instead of
/// each building its own set of remap tables.
/// </summary>
/// <param name="r">The red tint level of the lighting.</param>
/// <param name="g">The green tint level of the lighting.</param>
/// <param name="b">The blue tint level of the lighting.</param>
/// <returns>Returns with a pointer to the converter to draw with. Otherwise, NULL is
/// returned if there is no visible surface to build one for.</returns>
LightConvertClass * IsometricTileTypeClass::Find_Or_Make_Drawer(int r, int g, int b)
{
	LightConvertClass * drawer;

	if (VisibleSurface == NULL) {
		return(NULL);
	}

	if (r == NORMAL_LIGHT && g == NORMAL_LIGHT && b == NORMAL_LIGHT && TileDrawers.Count() != 0) {
		return(TileDrawers[0]);
	}

	Clamp_Tile_RGB(r,g,b);

	for (int i = 1; i < TileDrawers.Count(); i++) {
		drawer = TileDrawers[i];
		if (drawer->NormalRedTint == r && drawer->NormalGreenTint == g && drawer->NormalBlueTint == b) {
			return(drawer);
		}
	}

	drawer = new LightConvertClass(IsoTilePalette, GamePalette, *VisibleSurface, r, g, b, TileDrawers.Count() != 0);
	TileDrawers.Add(drawer);

	return(drawer);
}


/// This routine is a stub: no drawer is ever collected, and LightConvertClass::ReferenceCount
/// is maintained but never consulted.
/// The name is inferred from the two call sites, not attested.

/// <summary>
/// Frees the tile drawers that no cell is drawing through any more.
/// Drawers accumulate as changing light mints new tints, so this routine is used to hand the
/// unreferenced ones back. The sweep is spread across calls so that reclaiming a large pool
/// does not stall the game loop.
/// </summary>
/// <param name="time_budget_ms">The milliseconds this routine may spend before yielding.</param>
/// <param name="force">Should the whole pool be swept regardless of the budget?</param>
/// <returns>bool; Is there more left to free?</returns>
bool IsometricTileTypeClass::Free_Unused_Drawers(int time_budget_ms, bool force)
{
	return(false);
}


/// <summary>
/// Creates the default tile lighting converter.
/// This routine throws away the converters built for the previous theater and installs a
/// fresh untinted one in their place. Every cell is told to forget its cached converter
/// so that it will ask for a new one as it redraws.
/// </summary>
void IsometricTileTypeClass::Init_Drawers(void)
{
	for (int i = 0; i < TileDrawers.Count(); i++) {
		LightConvertClass *drawer = TileDrawers[i];
		delete drawer;
		TileDrawers.Delete_Index(i);
		i--;
	}

	TileDrawers.Add(new LightConvertClass(IsoTilePalette, GamePalette, *VisibleSurface));

	BlubCell.Drawer = NULL;
	BlubCell.Init_Drawer();

	Map.Reset_Iterator();
	CellClass * cptr = Map.Iterate();
	while (cptr != NULL) {
		cptr->Drawer = NULL;
		cptr = Map.Iterate();
	}
}


/// <summary>
/// Reads the tile control file for a theater.
/// This routine builds the isometric tile type heap from scratch: it loads the theater
/// palette, the cell shadow and slope depth shapes, and then creates a tile type for
/// every entry of every tile set named in the theater's INI file. Whatever theater was
/// loaded before is discarded first, so this is what a theater change goes through.
/// </summary>
/// <param name="theater">The theater to read the tile control file for.</param>
/// <param name="from_ccfile">Should the tile artwork be found as loose files and loaded on
/// demand, rather than pulled out of the mix files right away?</param>
void IsometricTileTypeClass::Read_Control_File(TheaterType theater, bool from_ccfile)
{
	int i;
	int j;
	int k;

	CCINIClass ini;
	IsometricTileTypeClass * tile = NULL;
	CDTimerClass<SystemTimerClass> callback_timer;
	int shadow_casters = 0;

	memset(ShadowCasterTiles, 0, sizeof(ShadowCasterTiles));

	CCFileClass cshadow("C_SHADOW.SHP");
	if (CellShadowShapes) {
		delete [] ((char *)CellShadowShapes);
		CellShadowShapes = NULL;
	}
	CellShadowShapes = (ShapeSet *)new char[cshadow.Size()];
	cshadow.Read(CellShadowShapes, cshadow.Size());

	if (TileInsertTypes.Count() != 0) {
		for (i = TileInsertTypes.Count() - 1; i >= 0; --i) {
			delete TileInsertTypes[i];
			TileInsertTypes.Delete_Index(i);
		}
	}

	for (i = 0; i < ARRAY_SIZE(SlopeZShapes); i++) {
		if (SlopeZShapes[i]) {
			delete [] ((char *)SlopeZShapes[i]);
			SlopeZShapes[i] = NULL;
		}
	}

	char slopzname[32];
	sprintf(slopzname, "SLOP01Z.%s", Theaters[theater].Suffix);
	CCFileClass slopz(slopzname);

	SlopeZShapes[0] = (ShapeSet *)new char[slopz.Size()];
	slopz.Read(SlopeZShapes[0], slopz.Size());

	slopzname[5] = '2';
	slopz.Set_Name(slopzname);
	SlopeZShapes[1] = (ShapeSet *)new char[slopz.Size()];
	slopz.Read(SlopeZShapes[1], slopz.Size());

	slopzname[5] = '3';
	slopz.Set_Name(slopzname);
	SlopeZShapes[2] = (ShapeSet *)new char[slopz.Size()];
	slopz.Read(SlopeZShapes[2], slopz.Size());

	slopzname[5] = '4';
	slopz.Set_Name(slopzname);
	SlopeZShapes[3] = (ShapeSet *)new char[slopz.Size()];
	slopz.Read(SlopeZShapes[3], slopz.Size());

	while (IsometricTileTypes.Count() != 0) {
		delete IsometricTileTypes[0];
	}

	char palname[32];
	sprintf(palname, "ISO%s.PAL", Theaters[theater].Suffix);
	CCFileClass palette(palname);
	if (palette.Is_Available()) {
		palette.Read(&IsoTilePalette, sizeof(IsoTilePalette));
		unsigned char * color = (unsigned char *)IsoTilePalette;
		for (i = 0; i < sizeof(PaletteClass); i++) {
			*color <<= 2;
			color++;
		}
	}
	IsometricTileTypeClass::Init_Drawers();

	char ininame[20];
	sprintf(ininame, "%s.INI", Theaters[theater].Root);
	CCFileClass inifile(ininame);
	ini.Load(inifile, false, false);

	int setid = 0;
	int heapid = 0;
	int tile_count = 0;

	IsometricTileType _RampStart = (IsometricTileType)ini.Get_Int("General", "RampBase", ISOTILE_INVALID);
	IsometricTileType _RampSmooth = (IsometricTileType)ini.Get_Int("General", "RampSmooth", ISOTILE_INVALID);
	IsometricTileType _MMRampBase = (IsometricTileType)ini.Get_Int("General", "MMRampBase", ISOTILE_INVALID);
	IsometricTileType _ClearTile = (IsometricTileType)ini.Get_Int("General", "ClearTile", ISOTILE_INVALID);
	IsometricTileType _RoughTile = (IsometricTileType)ini.Get_Int("General", "RoughTile", ISOTILE_INVALID);
	IsometricTileType _SandTile = (IsometricTileType)ini.Get_Int("General", "SandTile", ISOTILE_INVALID);
	IsometricTileType _GreenTile = (IsometricTileType)ini.Get_Int("General", "GreenTile", ISOTILE_INVALID);
	IsometricTileType _PaveTile = (IsometricTileType)ini.Get_Int("General", "PaveTile", ISOTILE_INVALID);
	IsometricTileType _MiscPaveTile = (IsometricTileType)ini.Get_Int("General", "MiscPaveTile", ISOTILE_INVALID);
	IsometricTileType _ClearToRoughLat = (IsometricTileType)ini.Get_Int("General", "ClearToRoughLat", ISOTILE_INVALID);
	IsometricTileType _ClearToSandLat = (IsometricTileType)ini.Get_Int("General", "ClearToSandLat", ISOTILE_INVALID);
	IsometricTileType _ClearToGreenLat = (IsometricTileType)ini.Get_Int("General", "ClearToGreenLat", ISOTILE_INVALID);
	IsometricTileType _ClearToPaveLat = (IsometricTileType)ini.Get_Int("General", "ClearToPaveLat", ISOTILE_INVALID);
	IsometricTileType _HeightBase = (IsometricTileType)ini.Get_Int("General", "HeightBase", ISOTILE_INVALID);
	IsometricTileType _BlackTile = (IsometricTileType)ini.Get_Int("General", "BlackTile", ISOTILE_INVALID);
	IsometricTileType _BridgeSet = (IsometricTileType)ini.Get_Int("General", "BridgeSet", ISOTILE_INVALID);
	IsometricTileType _TrainBridgeSet = (IsometricTileType)ini.Get_Int("General", "TrainBridgeSet", ISOTILE_INVALID);
	IsometricTileType _CliffSet = (IsometricTileType)ini.Get_Int("General", "CliffSet", ISOTILE_INVALID);
	IsometricTileType _ShorePieces = (IsometricTileType)ini.Get_Int("General", "ShorePieces", ISOTILE_INVALID);
	IsometricTileType _WaterSet = (IsometricTileType)ini.Get_Int("General", "WaterSet", ISOTILE_INVALID);
	IsometricTileType _Ice1Set = (IsometricTileType)ini.Get_Int("General", "Ice1Set", ISOTILE_INVALID);
	IsometricTileType _Ice2Set = (IsometricTileType)ini.Get_Int("General", "Ice2Set", ISOTILE_INVALID);
	IsometricTileType _Ice3Set = (IsometricTileType)ini.Get_Int("General", "Ice3Set", ISOTILE_INVALID);
	IsometricTileType _IceShoreSet = (IsometricTileType)ini.Get_Int("General", "IceShoreSet", ISOTILE_INVALID);
	IsometricTileType _SlopeSetPieces = (IsometricTileType)ini.Get_Int("General", "SlopeSetPieces", ISOTILE_INVALID);
	IsometricTileType _SlopeSetPieces2 = (IsometricTileType)ini.Get_Int("General", "SlopeSetPieces2", ISOTILE_INVALID);
	IsometricTileType _MonorailSlopes = (IsometricTileType)ini.Get_Int("General", "MonorailSlopes", ISOTILE_INVALID);
	IsometricTileType _Tunnels = (IsometricTileType)ini.Get_Int("General", "Tunnels", ISOTILE_INVALID);
	IsometricTileType _TrackTunnels = (IsometricTileType)ini.Get_Int("General", "TrackTunnels", ISOTILE_INVALID);
	IsometricTileType _DirtTunnels = (IsometricTileType)ini.Get_Int("General", "DirtTunnels", ISOTILE_INVALID);
	IsometricTileType _DirtTrackTunnels = (IsometricTileType)ini.Get_Int("General", "DirtTrackTunnels", ISOTILE_INVALID);
	IsometricTileType _WaterfallEast = (IsometricTileType)ini.Get_Int("General", "WaterfallEast", ISOTILE_INVALID);
	IsometricTileType _WaterfallWest = (IsometricTileType)ini.Get_Int("General", "WaterfallWest", ISOTILE_INVALID);
	IsometricTileType _WaterfallNorth = (IsometricTileType)ini.Get_Int("General", "WaterfallNorth", ISOTILE_INVALID);
	IsometricTileType _WaterfallSouth = (IsometricTileType)ini.Get_Int("General", "WaterfallSouth", ISOTILE_INVALID);
	IsometricTileType _CliffRamps = (IsometricTileType)ini.Get_Int("General", "CliffRamps", ISOTILE_INVALID);
	IsometricTileType _PavedRoads = (IsometricTileType)ini.Get_Int("General", "PavedRoads", ISOTILE_INVALID);
	IsometricTileType _PavedRoadEnds = (IsometricTileType)ini.Get_Int("General", "PavedRoadEnds", ISOTILE_INVALID);
	IsometricTileType _Medians = (IsometricTileType)ini.Get_Int("General", "Medians", ISOTILE_INVALID);
	IsometricTileType _RoughGround = (IsometricTileType)ini.Get_Int("General", "RoughGround", ISOTILE_INVALID);
	IsometricTileType _DirtRoadJunction = (IsometricTileType)ini.Get_Int("General", "DirtRoadJunction", ISOTILE_INVALID);
	IsometricTileType _DirtRoadCurve = (IsometricTileType)ini.Get_Int("General", "DirtRoadCurve", ISOTILE_INVALID);
	IsometricTileType _DirtRoadStraight = (IsometricTileType)ini.Get_Int("General", "DirtRoadStraight", ISOTILE_INVALID);
	IsometricTileType _DestroyableCliffs = (IsometricTileType)ini.Get_Int("General", "DestroyableCliffs", ISOTILE_INVALID_CLIFF);
	IsometricTileType _WaterCaves = (IsometricTileType)ini.Get_Int("General", "WaterCaves", ISOTILE_INVALID);
	IsometricTileType _WaterCliffs = (IsometricTileType)ini.Get_Int("General", "WaterCliffs", ISOTILE_INVALID);
	IsometricTileType _PavedRoadSlopes = (IsometricTileType)ini.Get_Int("General", "PavedRoadSlopes", ISOTILE_INVALID);
	IsometricTileType _DirtRoadSlopes = (IsometricTileType)ini.Get_Int("General", "DirtRoadSlopes", ISOTILE_INVALID);
	IsometricTileType _Rocks = (IsometricTileType)ini.Get_Int("General", "Rocks", ISOTILE_INVALID);
	IsometricTileType _CrystalTile = (IsometricTileType)ini.Get_Int("General", "CrystalTile", ISOTILE_INVALID);
	IsometricTileType _ClearToCrystalLat = (IsometricTileType)ini.Get_Int("General", "ClearToCrystalLat", ISOTILE_INVALID);
	IsometricTileType _CrystalCliff = (IsometricTileType)ini.Get_Int("General", "CrystalCliff", ISOTILE_INVALID);
	IsometricTileType _SwampTile = (IsometricTileType)ini.Get_Int("General", "SwampTile", ISOTILE_INVALID);
	IsometricTileType _WaterToSwampLat = (IsometricTileType)ini.Get_Int("General", "WaterToSwampLat", ISOTILE_INVALID);
	IsometricTileType _BlueMoldTile = (IsometricTileType)ini.Get_Int("General", "BlueMoldTile", ISOTILE_INVALID);
	IsometricTileType _ClearToBlueMoldLat = (IsometricTileType)ini.Get_Int("General", "ClearToBlueMoldLat", ISOTILE_INVALID);

	RampStart = ISOTILE_INVALID;
	RampSmooth = ISOTILE_INVALID;
	MMRampBase = ISOTILE_INVALID;
	ClearTile = ISOTILE_INVALID;
	RoughTile = ISOTILE_INVALID;
	SandTile = ISOTILE_INVALID;
	GreenTile = ISOTILE_INVALID;
	PaveTile = ISOTILE_INVALID;
	MiscPaveTile = ISOTILE_INVALID;
	ClearToRoughLat = ISOTILE_INVALID;
	ClearToSandLat = ISOTILE_INVALID;
	ClearToGreenLat = ISOTILE_INVALID;
	ClearToPaveLat = ISOTILE_INVALID;
	HeightBase = ISOTILE_INVALID;
	BlackTile = ISOTILE_INVALID;
	BridgeSet = ISOTILE_INVALID;
	TrainBridgeSet = ISOTILE_INVALID;
	CliffSet = ISOTILE_INVALID;
	ShorePieces = ISOTILE_INVALID;
	WaterSet = ISOTILE_INVALID;
	Ice1Set = ISOTILE_INVALID;
	Ice2Set = ISOTILE_INVALID;
	Ice3Set = ISOTILE_INVALID;
	IceShoreSet = ISOTILE_INVALID;
	SlopeSetPieces = ISOTILE_INVALID;
	SlopeSetPieces2 = ISOTILE_INVALID;
	MonorailSlopes = ISOTILE_INVALID;
	Tunnels = ISOTILE_INVALID;
	TrackTunnels = ISOTILE_INVALID;
	DirtTunnels = ISOTILE_INVALID;
	DirtTrackTunnels = ISOTILE_INVALID;
	WaterfallEast = ISOTILE_INVALID;
	WaterfallWest = ISOTILE_INVALID;
	WaterfallNorth = ISOTILE_INVALID;
	WaterfallSouth = ISOTILE_INVALID;
	CliffRamps = ISOTILE_INVALID;
	PavedRoads = ISOTILE_INVALID;
	PavedRoadEnds = ISOTILE_INVALID;
	Medians = ISOTILE_INVALID;
	RoughGround = ISOTILE_INVALID;
	DirtRoadJunction = ISOTILE_INVALID;
	DirtRoadCurve = ISOTILE_INVALID;
	DirtRoadStraight = ISOTILE_INVALID;
	DestroyableCliffs = ISOTILE_INVALID_CLIFF;
	WaterCaves = ISOTILE_INVALID;
	WaterCliffs = ISOTILE_INVALID;
	PavedRoadSlopes = ISOTILE_INVALID;
	DirtRoadSlopes = ISOTILE_INVALID;
	Rocks = ISOTILE_INVALID;
	CrystalTile = ISOTILE_INVALID;
	ClearToCrystalLat = ISOTILE_INVALID;
	CrystalCliff = ISOTILE_INVALID;
	SwampTile = ISOTILE_INVALID;
	WaterToSwampLat = ISOTILE_INVALID;
	BlueMoldTile = ISOTILE_INVALID;
	ClearToBlueMoldLat = ISOTILE_INVALID;

	BridgeTopLeft1 = (IsometricTileType)ini.Get_Int("General", "BridgeTopLeft1", ISOTILE_INVALID);
	BridgeTopLeft2 = (IsometricTileType)ini.Get_Int("General", "BridgeTopLeft2", ISOTILE_INVALID);
	BridgeBottomRight1 = (IsometricTileType)ini.Get_Int("General", "BridgeBottomRight1", ISOTILE_INVALID);
	BridgeBottomRight2 = (IsometricTileType)ini.Get_Int("General", "BridgeBottomRight2", ISOTILE_INVALID);
	BridgeTopRight1 = (IsometricTileType)ini.Get_Int("General", "BridgeTopRight1", ISOTILE_INVALID);
	BridgeTopRight2 = (IsometricTileType)ini.Get_Int("General", "BridgeTopRight2", ISOTILE_INVALID);
	BridgeBottomLeft1 = (IsometricTileType)ini.Get_Int("General", "BridgeBottomLeft1", ISOTILE_INVALID);
	BridgeBottomLeft2 = (IsometricTileType)ini.Get_Int("General", "BridgeBottomLeft2", ISOTILE_INVALID);
	BridgeMiddle1 = (IsometricTileType)ini.Get_Int("General", "BridgeMiddle1", ISOTILE_INVALID);
	BridgeMiddle2 = (IsometricTileType)ini.Get_Int("General", "BridgeMiddle2", ISOTILE_INVALID);

	struct TileSetRange {
		int BaseID;
		int Count;
	};
	std::vector<TileSetRange> tile_set_lookup;
	callback_timer = 4;

	while (true) {
		if (callback_timer == 0) {
			Call_Back();
			callback_timer = 4;
		}

		char section[64];
		sprintf(section, "TileSet%04d", setid);
		int tiles_in_set = ini.Get_Int(section, "TilesInSet", -1);
		if (tiles_in_set == -1) {
			break;
		}
		if (tiles_in_set < 0) {
			DebugString("Tile set section %s has invalid TilesInSet value %d; ending the contiguous tile-set load.\n",
				section, tiles_in_set);
			break;
		}

		int const current_set_base_id = heapid;
		tile_set_lookup.push_back({ current_set_base_id, tiles_in_set });

		if (setid == _RampStart) RampStart = (IsometricTileType)heapid;
		if (setid == _RampSmooth) RampSmooth = (IsometricTileType)heapid;
		if (setid == _MMRampBase) MMRampBase = (IsometricTileType)heapid;
		if (setid == _ClearTile) ClearTile = (IsometricTileType)heapid;
		if (setid == _RoughTile) RoughTile = (IsometricTileType)heapid;
		if (setid == _SandTile) SandTile = (IsometricTileType)heapid;
		if (setid == _GreenTile) GreenTile = (IsometricTileType)heapid;
		if (setid == _PaveTile) PaveTile = (IsometricTileType)heapid;
		if (setid == _MiscPaveTile) MiscPaveTile = (IsometricTileType)heapid;
		if (setid == _ClearToRoughLat) ClearToRoughLat = (IsometricTileType)heapid;
		if (setid == _ClearToSandLat) ClearToSandLat = (IsometricTileType)heapid;
		if (setid == _ClearToGreenLat) ClearToGreenLat = (IsometricTileType)heapid;
		if (setid == _ClearToPaveLat) ClearToPaveLat = (IsometricTileType)heapid;
		if (setid == _HeightBase) HeightBase = (IsometricTileType)heapid;
		if (setid == _BlackTile) BlackTile = (IsometricTileType)heapid;
		if (setid == _BridgeSet) BridgeSet = (IsometricTileType)heapid;
		if (setid == _TrainBridgeSet) TrainBridgeSet = (IsometricTileType)heapid;
		if (setid == _CliffSet) CliffSet = (IsometricTileType)heapid;
		if (setid == _ShorePieces) ShorePieces = (IsometricTileType)heapid;
		if (setid == _WaterSet) WaterSet = (IsometricTileType)heapid;
		if (setid == _Ice1Set) Ice1Set = (IsometricTileType)heapid;
		if (setid == _Ice2Set) Ice2Set = (IsometricTileType)heapid;
		if (setid == _Ice3Set) Ice3Set = (IsometricTileType)heapid;
		if (setid == _IceShoreSet) IceShoreSet = (IsometricTileType)heapid;
		if (setid == _SlopeSetPieces) SlopeSetPieces = (IsometricTileType)heapid;
		if (setid == _SlopeSetPieces2) SlopeSetPieces2 = (IsometricTileType)heapid;
		if (setid == _MonorailSlopes) MonorailSlopes = (IsometricTileType)heapid;
		if (setid == _DirtTunnels) DirtTunnels = (IsometricTileType)heapid;
		if (setid == _DirtTrackTunnels) DirtTrackTunnels = (IsometricTileType)heapid;
		if (setid == _Tunnels) Tunnels = (IsometricTileType)heapid;
		if (setid == _TrackTunnels) TrackTunnels = (IsometricTileType)heapid;
		if (setid == _WaterfallEast) WaterfallEast = (IsometricTileType)heapid;
		if (setid == _WaterfallWest) WaterfallWest = (IsometricTileType)heapid;
		if (setid == _WaterfallNorth) WaterfallNorth = (IsometricTileType)heapid;
		if (setid == _WaterfallSouth) WaterfallSouth = (IsometricTileType)heapid;
		if (setid == _CliffRamps) CliffRamps = (IsometricTileType)heapid;
		if (setid == _PavedRoads) PavedRoads = (IsometricTileType)heapid;
		if (setid == _PavedRoadEnds) PavedRoadEnds = (IsometricTileType)heapid;
		if (setid == _Medians) Medians = (IsometricTileType)heapid;
		if (setid == _RoughGround) RoughGround = (IsometricTileType)heapid;
		if (setid == _DirtRoadJunction) DirtRoadJunction = (IsometricTileType)heapid;
		if (setid == _DirtRoadCurve) DirtRoadCurve = (IsometricTileType)heapid;
		if (setid == _DirtRoadStraight) DirtRoadStraight = (IsometricTileType)heapid;
		if (setid == _DestroyableCliffs) DestroyableCliffs = (IsometricTileType)heapid;
		if (setid == _WaterCliffs) WaterCliffs = (IsometricTileType)heapid;
		if (setid == _WaterCaves) WaterCaves = (IsometricTileType)heapid;
		if (setid == _PavedRoadSlopes) PavedRoadSlopes = (IsometricTileType)heapid;
		if (setid == _DirtRoadSlopes) DirtRoadSlopes = (IsometricTileType)heapid;
		if (setid == _Rocks) Rocks = (IsometricTileType)heapid;
		if (setid == _CrystalTile) CrystalTile = (IsometricTileType)heapid;
		if (setid == _ClearToCrystalLat) ClearToCrystalLat = (IsometricTileType)heapid;
		if (setid == _CrystalCliff) CrystalCliff = (IsometricTileType)heapid;
		if (setid == _SwampTile) SwampTile = (IsometricTileType)heapid;
		if (setid == _WaterToSwampLat) WaterToSwampLat = (IsometricTileType)heapid;
		if (setid == _BlueMoldTile) BlueMoldTile = (IsometricTileType)heapid;
		if (setid == _ClearToBlueMoldLat) ClearToBlueMoldLat = (IsometricTileType)heapid;

		int last_tiles_in_set = ini.Get_Int(section, "LastTilesInSet", -1);
		if (last_tiles_in_set == -1 || last_tiles_in_set == tiles_in_set) {
			tile_count += tiles_in_set;
		} else {
			TileInsertType * insert = new TileInsertType;
			insert->Count = tile_count + last_tiles_in_set;
			insert->Offset = tiles_in_set - last_tiles_in_set;
			TileInsertTypes.Add(insert);
			tile_count += last_tiles_in_set;
		}

		char set_name[64];
		char file_name[64];
		ini.Get_String(section, "SetName", "No Name", set_name, sizeof(set_name));
		ini.Get_String(section, "FileName", "TILE", file_name, sizeof(file_name));
		IsometricTileType _MarbleMadness = (IsometricTileType)ini.Get_Int(section, "MarbleMadness", ISOTILE_NONE);
		IsometricTileType _NonMarbleMadness = (IsometricTileType)ini.Get_Int(section, "NonMarbleMadness", ISOTILE_NONE);
		bool _Morphable = ini.Get_Bool(section, "Morphable", false);
		bool _AllowToPlace = ini.Get_Bool(section, "AllowToPlace", true);
		bool _AllowBurrowing = ini.Get_Bool(section, "AllowBurrowing", true);
		bool _AllowTiberium = ini.Get_Bool(section, "AllowTiberium", false);
		bool _RequiredForRMG = ini.Get_Bool(section, "RequiredForRMG", false);
		IsometricTileType _ToSnowTheater = (IsometricTileType)ini.Get_Int(section, "ToSnowTheater", ISOTILE_INVALID);
		IsometricTileType _ToTemperateTheater = (IsometricTileType)ini.Get_Int(section, "ToTemperateTheater", ISOTILE_INVALID);

		bool is_shadow_caster = ini.Get_Bool(section, "ShadowCaster", false);
		if (is_shadow_caster) {
			ShadowCasterTiles[shadow_casters++] = (IsometricTileType)heapid;
		}
		int shadow_tiles = 0;
		if (is_shadow_caster) {
			shadow_tiles = ini.Get_Int(section, "ShadowTiles", shadow_tiles);
		}

		bool has_section = ini.Is_Present(set_name);

		for (i = 0; i < tiles_in_set; i++) {
			int tile_count = 0;

			IsometricTileTypeClass * head_tile = NULL;
			IsometricTileTypeClass * tail_tile = NULL;

			IsoTileSet * mixfile_set = NULL;
			bool found_image = false;

			do {
				char letter_suffix[2];
				char suffix[12];
				char tile_name[128];
				char given_name[128];

				if (tile_count != 0) {
					letter_suffix[0] = tile_count + 'a' - 1;
					letter_suffix[1] = 0;
				} else {
					letter_suffix[0] = 0;
				}

				sprintf(suffix, "%02d", i + 1);
				strcat(suffix, letter_suffix);
				strcpy(tile_name, file_name);
				strcat(tile_name, suffix);
				sprintf(given_name, "%.28s %02d", set_name, i + 1);

				int uninitialized_value = 0x00000003;

				if (tile_count == 0) {
					tile = new IsometricTileTypeClass((IsometricTileType)heapid++, uninitialized_value, 0, tile_name, false);

					tile->GivenName = given_name;
					tile->MarbleMadness = _MarbleMadness;
					tile->NonMarbleMadness = _NonMarbleMadness;
					tile->IsMorphable = _Morphable;
					tile->IsAllowToPlace = _AllowToPlace;
					tile->IsAllowBurrowing = _AllowBurrowing;
					tile->IsAllowTiberium = _AllowTiberium;
					tile->IsRequiredForRMG = _RequiredForRMG;
					tile->ImageData = NULL;
					tile->ToSnowTheater = _ToSnowTheater;
					tile->ToTemperateTheater = _ToTemperateTheater;
					if (is_shadow_caster && shadow_tiles) {
						tile->IsShadowCaster = is_shadow_caster;
					}
					if (from_ccfile) {
						tile->IsFileLoaded = true;
					}
					tile->TileSetBaseID = current_set_base_id;
					tile->PreviewTiles.Clear();
					if (has_section) {

						char entry[32];
						sprintf(entry, "Tile%02dAnim", i + 1);

						char anim_name[128];
						if (ini.Get_String(set_name, entry, (const char *)"", anim_name, sizeof(anim_name))) {

							AnimTypeClass * animtype = AnimTypeClass::Find_Or_Make(anim_name);
							if (animtype != NULL) {
								tile->Anim = (AnimType)animtype->Fetch_Heap_ID();
								sprintf(entry, "Tile%02dXOffset", i + 1);
								tile->Offset.X = ini.Get_Int(set_name, entry, tile->Offset.X);
								sprintf(entry, "Tile%02dYOffset", i + 1);
								tile->Offset.Y = ini.Get_Int(set_name, entry, tile->Offset.Y);
								sprintf(entry, "Tile%02dAttachesTo", i + 1);
								tile->AttachesTo = ini.Get_Int(set_name, entry, tile->AttachesTo);
								sprintf(entry, "Tile%02dZAdjust", i + 1);
								tile->ZAdjust = ini.Get_Int(set_name, entry, tile->ZAdjust);
							}
						}
					}
					head_tile = tile;
					tail_tile = tile;
				}

				char file_path[512];
				_makepath(file_path, NULL, NULL, tile_name, Theaters[theater].Suffix);

				mixfile_set = NULL;
				found_image = false;

				if (from_ccfile) {
					CCFileClass file(file_path);
					found_image = file.Is_Available();
					if (found_image && tile_count == 0) {
						strncpy(tile->Filename, file_path, sizeof(tile->Filename)-2);
					}
				} else {
					mixfile_set = (IsoTileSet *)MFCD::Retrieve(file_path);
					if (mixfile_set) {
						found_image = true;
					}
				}
				if (!found_image && _NonMarbleMadness) {
					if (theater == THEATER_TEMPERATE) {
						_makepath(file_path, NULL, NULL, tile_name, ".MMT");
					} else {
						_makepath(file_path, NULL, NULL, tile_name, ".MMS");
					}
					if (from_ccfile) {
						CCFileClass file(file_path);
						found_image = file.Is_Available();
						if (found_image && tile_count == 0) {
							strncpy(tile->Filename, file_path, sizeof(tile->Filename)-2);
						}
					} else {
						mixfile_set = (IsoTileSet *)MFCD::Retrieve(file_path);
						if (mixfile_set) {
							found_image = true;
						}
					}
				}

				if (tile_count != 0) {
					if (!mixfile_set && !found_image) {
						break;
					}
					tile = new IsometricTileTypeClass((IsometricTileType)(heapid - 1), uninitialized_value, 0, tile_name, true);

					tile->GivenName = given_name;
					tile->MarbleMadness = _MarbleMadness;
					tile->NonMarbleMadness = _NonMarbleMadness;
					tile->IsMorphable = _Morphable;
					tile->IsAllowToPlace = _AllowToPlace;
					tile->IsAllowBurrowing = _AllowBurrowing;
					tile->IsAllowTiberium = _AllowTiberium;
					tile->IsRequiredForRMG = _RequiredForRMG;
					if (from_ccfile) {
						tile->IsFileLoaded = true;
						strncpy(tile->Filename, file_path, sizeof(tile->Filename)-2);
					}
					if (is_shadow_caster && shadow_tiles) {
						tile->IsShadowCaster = is_shadow_caster;
					}
					tile->TileSetBaseID = current_set_base_id;
					tail_tile->NextTileTypeInSet = tile;
					tile->PreviewTiles.Clear();
					tail_tile = tile;
				}

				if (tile_count == 0 || mixfile_set) {
					tile->ImageData = mixfile_set;
				}

				if (mixfile_set != NULL) {
					tile->Width = (unsigned char)mixfile_set->MapWidth;
					tile->Height = (unsigned char)mixfile_set->MapHeight;
					for (j = 0; j < mixfile_set->Tile_Count(); j++) {
						IsoTileRecord ** record = &mixfile_set->Tiles[j];
						if (*record != 0 && (unsigned int)*record < (unsigned int)mixfile_set) {
							*record = (IsoTileRecord *)((unsigned int)mixfile_set + (unsigned int)*record);
						}
					}
					tile->Build_Preview_Tiles();
				} else {
					if (tile_count == 0) {
						tile->Width = 0;
						tile->Height = 0;
					}
				}
				tile_count++;
			} while (mixfile_set != NULL || found_image);

			if (tile_count > 1) {
				IsometricTileTypeClass * tile = head_tile;
				for (char t = tile_count; t > 1; t--) {
					tile->NumTileTypesInSet = t;
					tile = tile->NextTileTypeInSet;
				}
			}
		}
		setid++;
	}

	for (j = 0; j < IsometricTileTypes.Count(); ++j) {
		IsometricTileTypeClass * isotype = IsometricTileTypes[j];
		int offset = isotype->HeapID - isotype->TileSetBaseID;
		auto remap = [&](IsometricTileType & reference, char const * field) {
			if (reference == ISOTILE_NONE) {
				return;
			}

			int const referenced_set = static_cast<int>(reference);
			bool valid = referenced_set >= 0
				&& static_cast<std::size_t>(referenced_set) < tile_set_lookup.size();
			if (valid) {
				TileSetRange const & range = tile_set_lookup[referenced_set];
				valid = range.BaseID >= 0 && range.Count >= 0
					&& offset >= 0 && offset < range.Count
					&& range.BaseID <= std::numeric_limits<int>::max() - offset;
				if (valid) {
					reference = static_cast<IsometricTileType>(range.BaseID + offset);
				}
			}
			if (!valid) {
				DebugString("Tile type %d has invalid %s reference %d at offset %d; using ISOTILE_NONE.\n",
					static_cast<int>(isotype->HeapID), field, referenced_set, offset);
				reference = ISOTILE_NONE;
			}
		};

		remap(isotype->MarbleMadness, "MarbleMadness");
		remap(isotype->NonMarbleMadness, "NonMarbleMadness");
	}

	/// Remap ice tiles to water
	if (theater != THEATER_TEMPERATE && theater == THEATER_SNOW) {
		if (Ice1Set != ISOTILE_INVALID) {
			for (k = ICE_EDGE; k < ICE1_COUNT; k++) {
				IsoTileRecord * record = ((IsoTileSet *)IsometricTileTypes[Ice1Set + k]->Get_Image_Data())->Tiles[0];
				if (record) {
					record->TileType = 9;
				}
			}
		}
		if (Ice2Set != ISOTILE_INVALID) {
			for (k = ICE_EDGE; k < ICE2_COUNT; k++) {
				IsoTileRecord * record = (IsoTileRecord *)((IsoTileSet *)(IsometricTileTypes[Ice2Set + k])->Get_Image_Data())->Tiles[0];
				if (record) {
					record->TileType = 9;
				}
			}
		}
		if (Ice3Set != ISOTILE_INVALID) {
			for (k = ICE_EDGE; k < ICE3_COUNT; k++) {
				IsoTileRecord * record = (IsoTileRecord *)((IsoTileSet *)(IsometricTileTypes[Ice3Set + k])->Get_Image_Data())->Tiles[0];
				if (record) {
					record->TileType = 9;
				}
			}
		}
	}
}


/// <summary>
/// Loads the tile artwork that the scenario needs.
/// This routine walks the map to find which tile types are actually in use, loads the
/// artwork for those, and frees the artwork for the rest. It is called once the map has
/// been read, so that the unused half of a theater does not sit in memory.
/// </summary>
/// <param name="skipiteration">Should the map scan be skipped and every tile type kept?</param>
/// <param name="isrand">Is this a random map, where the tiles the generator needs must be
/// kept even if nothing on the map uses them yet?</param>
void IsometricTileTypeClass::Load_Tiles(bool skipiteration, bool isrand)
{
	CDTimerClass<SystemTimerClass> timer = 0;
	IsometricTileTypeClass *ptr;

	if (!skipiteration) {
		Map.Reset_Iterator();
		CellClass *cptr = Map.Iterate();
		while (cptr != NULL) {
			IsometricTileType type = cptr->ITType;
			int index = 0;
			int num;

			if (type != ISOTILE_NONE && type < (IsometricTileType)IsometricTileTypes.Count()) {
				ptr = IsometricTileTypes[type];

				if (ptr->NumTileTypesInSet > 1) {
					num = ptr->NumTileTypesInSet;
					if (ptr->Is_Randomized(cptr->SubTile)) {
						index = cptr->IsBridgeDamaged;
					} else {
						num = ptr->NumTileTypesInSet;
						index = cptr->Clear_Icon(cptr->ITType, num);
					}
				}
			} else {
				ptr = IsometricTileTypes[TILE_CLEAR];
				num = ptr->NumTileTypesInSet;
				index = cptr->Clear_Icon(TILE_CLEAR, num);
			}
			ptr = ptr->Next_Tile_From_Set(index);
			ptr->UseCount++;
			cptr = Map.Iterate();
		}
	}

	int tile_count = 0;
	int bytes = 0;
	timer = 1;

	for (int i = 0; i < IsometricTileTypes.Count(); i++) {
		if (!timer) {
			Call_Back();
			timer = 4;
		}


		ptr = IsometricTileTypes[i];
		while (ptr != NULL) {
			if (ptr->IsFileLoaded && strlen(ptr->Filename)) {
				if (!skipiteration && ptr->UseCount == 0 && (!isrand || !ptr->IsRequiredForRMG)) {
					if (ptr->ImageData != NULL) {
						delete [] (unsigned char *)ptr->ImageData;
						ptr->ImageData = NULL;
					}
				} else {
					if (ptr->ImageData == NULL) {
						bytes += ptr->Load_Tile_Data();
						tile_count++;
					}
				}
			}
			ptr = ptr->NextTileTypeInSet;
		}
	}

	DebugString("Loaded %d isometric tiles consuming %d Kb\n", tile_count, bytes / 1024);
}


/// <summary>
/// Loads this tile type's artwork from its file.
/// This routine reads the tile set into memory, fixes up the record pointers it carries,
/// and rebuilds the preview colors from it. Any artwork already resident is thrown away
/// first, so a tile type never holds two copies.
/// </summary>
/// <returns>Returns with the number of bytes the artwork occupies.</returns>
int IsometricTileTypeClass::Load_Tile_Data(void)
{
	CCFileClass file(Filename);
	int size = file.Size();
	if (ImageData != NULL) {
		delete [] (unsigned char *)ImageData;
	}
	unsigned char * data = new unsigned char[size];
	ImageData = data;
	file.Read(data, size);

	IsoTileSet * tileset = (IsoTileSet *)ImageData;

	Height = ((unsigned char)tileset->Map_Height());
	Width = ((unsigned char)tileset->Map_Width());

	/// Fixup pointers to point to actual memory
	for (int i = 0; i < (Width * Height); i++) {

		if (tileset->Tiles[i] != NULL) {
			unsigned char * ptr = (unsigned char *)ImageData;
			/*
			 * Only fix up pointers that have not been converted already. The file
			 * has just been read fresh, so none of them ever have been.
			 */
			if ((void *)tileset->Tiles[i] < ptr) {
				tileset->Tiles[i] = (IsoTileRecord *)(ptr + (unsigned int)tileset->Tiles[i]);
			}
		}

	}

	Build_Preview_Tiles();

	return(size);
}


/// <summary>
/// Clears the usage count of every isometric tile type.
/// This routine is used before the map is scanned, so that the tile loader can tell which
/// tile types the scenario really needs and which may be dropped from memory.
/// </summary>
void IsometricTileTypeClass::Clear_Use_Counts(void)
{
	IsometricTileTypeClass * t;
	for (int i = 0; i < IsometricTileTypes.Count(); i++) {
		for (t = IsometricTileTypes[i]; t != NULL; t = t->NextTileTypeInSet) {
			t->UseCount = 0;
		}
	}
}


/// <summary>
/// Fetches the pixel dimensions of a sub-tile.
/// The height reported takes in any extra artwork that hangs above the tile diamond, so
/// the caller gets the full extent of what will be drawn.
/// </summary>
/// <param name="tile">The sub-tile index within this tile set.</param>
/// <param name="width">Receives the tile width in pixels.</param>
/// <param name="height">Receives the tile height in pixels.</param>
/// <returns>bool; Were the dimensions available?</returns>
bool IsometricTileTypeClass::Get_Tile_Pixel_Dimensions(int tile, int & width, int & height)
{
	IsoTileSet const * tileset = (IsoTileSet const *)Get_Image_Data();
	if (tileset != NULL) {
		IsoTileRecord const * record = tileset->Fetch_Record_Pointer(tile);
		width = tileset->Pixel_Width();
		height = tileset->Pixel_Height();
		if (record != NULL) {
			if ((record->IsHasExtraData) != 0) {
				height += (record->Y - record->ExtraY);
			}
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the ramp type of a sub-tile.
/// The ramp type tells the movement and drawing code which way the ground slopes beneath
/// the tile.
/// </summary>
/// <param name="tile">The sub-tile index within this tile set.</param>
/// <returns>Returns with the ramp type, or zero if the ground there is flat.</returns>
int IsometricTileTypeClass::Ramp_Type(int tile) const
{
	IsoTileSet const * tileset = (IsoTileSet const *)Get_Image_Data();

	if (tileset != NULL) {
		IsoTileRecord const * record = tileset->Fetch_Record_Pointer(tile);
		if (record != NULL) {
			return(record->RampType);
		}
	}
	return(0);
}


/// <summary>
/// Is this sub-tile flagged as randomized?
/// A randomized tile takes its variation at random rather than from the usual clear
/// terrain pattern, so that large open areas do not repeat.
/// </summary>
/// <param name="tile">The sub-tile index within this tile set.</param>
/// <returns>bool; Is the sub-tile randomized?</returns>
bool IsometricTileTypeClass::Is_Randomized(int tile) const
{
	IsoTileSet const * tileset = (IsoTileSet const *)Get_Image_Data();

	if (tileset != NULL) {
		IsoTileRecord const * record = tileset->Fetch_Record_Pointer(tile);
		if (record != NULL) {
			return(record->IsRandomized);
		}
	}
	return(0);
}


/// <summary>
/// Draws the shadow that this tile casts.
/// Cliff and slope tiles have a shadow shape associated with them, taken from the shared
/// cell shadow shape file and nudged so that it falls away from the tile. A tile with no
/// shadow of its own draws nothing.
/// </summary>
/// <param name="index">The sub-tile index within this tile set.</param>
/// <param name="surf">The surface to draw the shadow onto.</param>
/// <param name="point">The tactical pixel position of the tile.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
/// <param name="offset">The depth offset to draw the shadow at.</param>
void IsometricTileTypeClass::Draw_Shadow_Caster(int index, Surface * surf, Point2D point, Rect cliprect, int offset)
{
	int x;
	int y;
	int frame;
	int caster_index;

	int tiletype = HeapID;

	bool flat = false;

	int pieces;
	if (tiletype >= SlopeSetPieces && tiletype < SlopeSetPieces + ARRAY_SIZE(ShadowCasterSlopeInfo)) {
		pieces = SlopeSetPieces;
		flat = true;
	} else {
		if (tiletype >= SlopeSetPieces2 && tiletype < SlopeSetPieces2 + ARRAY_SIZE(ShadowCasterSlopeInfo)) {
			pieces = SlopeSetPieces2;
			flat = true;
		}
	}

	bool can_draw = false;

	if (flat && tiletype - pieces != -1) {
		caster_index = tiletype - pieces;

		if (index == ShadowCasterSlopeInfo[caster_index].Index) {

			frame = ShadowCasterSlopeInfo[caster_index].Frame;

			if (frame != 0) {
				x = (ISO_TILE_PIXEL_W / 2) + ShadowCasterSlopeInfo[caster_index].XOffset;
				y = (ISO_TILE_PIXEL_H / 2) + ShadowCasterSlopeInfo[caster_index].YOffset;
				can_draw = true;
			}
		}
	} else if (IsShadowCaster) {

		for (int i = 0; i < ARRAY_SIZE(ShadowCasterTiles); i++) {
			caster_index = tiletype - ShadowCasterTiles[i];

			if (caster_index >= 0 && caster_index < ARRAY_SIZE(ShadowCasterCliffInfo)) {
				if (index == ShadowCasterCliffInfo[caster_index].Index) {
					frame = ShadowCasterCliffInfo[caster_index].Frame;

					if (frame != 0) {
						x = (ISO_TILE_PIXEL_W / 2) + ShadowCasterCliffInfo[caster_index].XOffset;
						y = (ISO_TILE_PIXEL_H / 2) + ShadowCasterCliffInfo[caster_index].YOffset;
						can_draw = true;
					}
				}
				break;
			}
			if (i == 4) {
				can_draw = false;
				break;
			}
		}
	}

	if (can_draw) {
		point = Point2D((TacticalRect.X + x + point.X) - cliprect.X, (TacticalRect.Y + y + point.Y) - cliprect.Y);
		Draw_Shape(
			*surf,
			*TileDrawers[0],
			(const ShapeSet *)CellShadowShapes,
			frame - 1,
			point,
			cliprect,
			ShapeFlags_Type(SHAPE_ZWRITE|SHAPE_WIN_REL|SHAPE_CENTER|SHAPE_DARKEN),
			0,
			offset);
	}
}


/// <summary>
/// Fetches the list of cells that this tile casts its shadow over.
/// This routine is used by the redraw logic so that the cells darkened by a cliff shadow
/// are refreshed along with the cliff itself.
/// </summary>
/// <returns>Returns with a pointer to the cell offset list, terminated by REFRESH_EOL.
/// Otherwise, NULL is returned if this tile casts no shadow.</returns>
Cell const * IsometricTileTypeClass::Shadow_Caster_List(void) const
{
	if (IsShadowCaster) {
		static const Cell _list[CLIFF_COUNT][6] = {
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ Cell(0,-1), Cell(0,-1), Cell(-1,-2), CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ Cell(0,-1), Cell(0,-1), Cell(-1,-2), CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ Cell(-1,-3), Cell(0,-3), CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ Cell(-1,-3), Cell(0,-3), CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ Cell(-1,-3), Cell(0,-3), CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ Cell(0,-3), CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ Cell(0,-3), Cell(1,-3), Cell(-1,-3), Cell(-2,-3), CELL_NONE, REFRESH_EOL },
			{ Cell(0,-3), Cell(-1,-3), Cell(-2,-3), Cell(-3,-2), CELL_NONE, REFRESH_EOL },
			{ Cell(0,-3), Cell(-1,-3), Cell(-2,-3), CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ Cell(0,-3), Cell(-1,-3), Cell(-2,-3), CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ Cell(-2,-3), CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ Cell(-2,-3), CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ Cell(0,-3), CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
			{ CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, CELL_NONE, REFRESH_EOL },
		};

		for (int i = 0; i < ARRAY_SIZE(ShadowCasterTiles); i++) {
			int idx = HeapID - ShadowCasterTiles[i];
			if (idx >= 0 && HeapID - ShadowCasterTiles[i] < CLIFF_COUNT) {
				return(_list[idx]);
			}
		}
	}
	return(NULL);
}

/// Scratch state for the iso-tile rasterizer, the C++ loop in
/// IsometricTileTypeClass::Draw_Tile. The field names below describe how that loop uses them.
///
/// The rasterizer walks the iso diamond one scanline at a time using three precomputed span
/// tables (RowSrcOffset/RowStartCol/RowRunLength), reading a palette index per pixel, looking
/// up a per-pixel light level from the alpha buffer (LightRemap), translating to 16bpp
/// (PixelTranslate) and depth-testing against the Z buffer.
/// Row tables (advance ISO_DRAW_WIDTH entries per scanline):
/// RowSrcOffset   per-row offset into the tile image / depth data
/// RowStartCol    per-row left column of the diamond span
/// RowRunLength   per-row pixel count of the diamond span
/// Image sources:
/// ImageBase      base of the tile's palette-index pixels (record + 1)
/// DepthBase      base of the tile's per-pixel depth bytes
/// SrcPixel       running source palette-index pointer
/// SrcDepth       running source depth-byte pointer
///     BaseDepth      depth value added to each source depth byte (from drawpoint.Y/height)
/// Destinations (running + per-row start):
/// DestPtr/RowDest    surface, DepthPtr/RowDepth  Z buffer, AlphaPtr/RowAlpha  alpha buffer
///   Strides: SurfacePitch (bytes), DepthWidth, AlphaWidth (u16 elements)
///   Lookups: LightRemap (alpha->light), PixelTranslate (palette+light -> 16bpp)
/// Clip/span: ClipLeft, ClipTop, SpanWidth, SpanHeight
/// Fill constants: FillDepth (depth-only / fill passes), FogColor (shroud fog), HalfbrightMask
#pragma pack(push, 1)
struct IsoBlitState {
	int SrcPixel;
	int SrcDepth;
	unsigned short *PixelTranslate;
	unsigned short *DepthPtr;
	unsigned short *AlphaPtr;
	unsigned short *DestPtr;
	unsigned short *LightRemap;
	unsigned short *RowSrcOffset;
	unsigned char *RowStartCol;
	unsigned char *RowRunLength;
	unsigned char *ImageBase;
	unsigned char *DepthBase;
	unsigned short *RowDest;
	unsigned short *RowDepth;
	unsigned short *RowAlpha;
	signed int SpanWidth;
	signed int SpanHeight;
	unsigned int ClipLeft;
	unsigned int ClipTop;
	unsigned int BaseDepth;
	unsigned int DepthWidth;
	unsigned int AlphaWidth;
	unsigned int SurfacePitch;
	unsigned int ImageRowStep;
	unsigned int FillDepth;
	unsigned int FogColor;
	unsigned short HalfbrightMask;
};
#pragma pack(pop)


IsoBlitState IsoDrawData;
unsigned short _iso_row_offsets[ISO_DRAW_WIDTH*ISO_DRAW_HEIGHT];
unsigned char _iso_start_cols[ISO_DRAW_WIDTH*ISO_DRAW_HEIGHT];
/*
 * The run-length span table is indexed [right_clip_shift][row*ISO_DRAW_WIDTH + col]. The one-time span
 * builder fills it from the last row (shift (ISO_DRAW_WIDTH-1)) downward; the blit reads the row selected by
 * the tile's right-edge clip (shift (ISO_DRAW_WIDTH-1) == unclipped).
 */
unsigned char _iso_run_lengths[ISO_DRAW_WIDTH][ISO_DRAW_WIDTH*ISO_DRAW_HEIGHT];
char IsoSpanTablesBuilt;


/// <summary>
/// Draws a single isometric terrain tile.
/// This is the low level tile blitter that the map renderer relies on for every terrain
/// cell it lays down. The tile is clipped, lit, and optionally depth tested against the
/// Z buffer as it is drawn. The callers are CellClass::Draw_It for a normal tile and
/// CellClass::Wipe_Depth for the depth only pass.
/// </summary>
/// <param name="drawer">The lighting converter that supplies the palette tables.</param>
/// <param name="subtile">The sub-tile image index within the template set.</param>
/// <param name="surface">The destination surface to draw upon.</param>
/// <param name="x">The destination pixel X of the tile diamond.</param>
/// <param name="y">The destination pixel Y of the tile diamond.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
/// <param name="height">The cell height level, which biases the tile's base depth.</param>
/// <param name="brightness">The lighting level to remap the tile pixels with.</param>
/// <param name="use_z">Should the tile be depth tested and the Z buffer updated?</param>
/// <param name="cell_variation">The variant to draw from this tile's set. Zero draws
/// this tile.</param>
/// <param name="fill">Should the footprint be stamped solid with the fog color instead?</param>
/// <param name="depth_only">Should only the Z buffer be written?</param>
/// <param name="fog">Should the footprint be darkened as fog instead of drawn?</param>
/// <param name="fog_color">The tint to apply for the fill and fog passes.</param>
/// <remarks>
/// At most one of fill, depth_only and fog may be set on any one call. With none of them
/// set, the tile is drawn normally.
/// </remarks>
void IsometricTileTypeClass::Draw_Tile(LightConvertClass * drawer, int subtile, Surface & surface, int x, int y, Rect cliprect, int height, int brightness, bool use_z, int cell_variation, bool fill, bool depth_only, bool fog, signed int fog_color) const
{
	static int const _iso_row_bases[ISO_HEIGHT] = {
		0, 4, 12, 24, 40, 60, 84, 112, 144, 180, 220, 264, 312, 356, 396, 432, 464, 492, 516, 536, 552, 564, 572, 576
	};

	/// The table is spelled with literal 0x20 (space) and 0xDB (box) characters, written as
	/// escapes here so that the file stays plain ASCII.
	#define __ "\x20"
	#define XX "\xDB"

	static const unsigned char _tilemask[] = {
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __
		__ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __
		__ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __
		__ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __
		XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX
		__ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __
		__ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __
		__ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __
		__ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
	};

	#undef __
	#undef XX

	LightConvertClass * drawtest = drawer;
	IsometricTileTypeClass const * tileptr;
	tileptr = this;
	if (drawtest != NULL) {
		/*
		 * If a random variation was requested, walk the tile-set list to the
		 * requested variation and draw that tile instead of this one.
		 */
		while (true) {
			IsoDrawData.HalfbrightMask = DSurface::Get_Halfbright_Mask();
			if (cell_variation == 0) {
				break;
			}
			int num = tileptr->NumTileTypesInSet;
			if (cell_variation > num - 1) {
				cell_variation %= num;
			}
			if (cell_variation == 0) {
				break;
			}
			IsometricTileTypeClass const * next = tileptr;
			do {
				next = next->NextTileTypeInSet;
			} while (--cell_variation != 0);
			if (next == tileptr) {
				break;
			}
			cell_variation = 0;
			tileptr = next;
		}
		Point2D work(x, y);
		bool clipped_out = false;
		AlphaLightingRemapClass * aremap = AlphaLightingRemapInit.Init(drawer->IntensityLevels);
		IsoDrawData.LightRemap = aremap->Get_Table(brightness);
		IsoDrawData.PixelTranslate = (unsigned short *)drawer->IntensityTranslator;

		/*
		 * One-time build of the iso-diamond span tables. For each of the ISO_DRAW_HEIGHT
		 * scanlines of the ISO_DRAW_WIDTH-wide tile diamond record the left column of the
		 * solid span (_iso_start_cols), the per-pixel source offset (_iso_row_offsets)
		 * and, for every possible right-edge clip, the run length (_iso_run_lengths).
		 * Driven by the diamond bitmap in _tilemask.
		 */
		if (!IsoSpanTablesBuilt) {
			int cell_offset = 0;
			const int * lut = _iso_row_bases;
			int loop_pos = 0;
			do {
				int span_count;

				/*
				 * Find the first non-space column of this mask row and the
				 * length of the solid span that follows it.
				 */
				int first_col = 0;
				while (first_col < ISO_DRAW_WIDTH && _tilemask[cell_offset + first_col] == 32) {
					++first_col;
				}
				span_count = 1;
				if (first_col + 1 < ISO_DRAW_WIDTH) {
					int scan = first_col + 1;
					const unsigned char * mask = &_tilemask[cell_offset + 1 + first_col];
					do {
						if (*mask == 32) {
							break;
						}
						++span_count;
						++scan;
						++mask;
					} while (scan < ISO_DRAW_WIDTH);
				}
				if (first_col >= ISO_DRAW_WIDTH) {
					first_col = 0;
					span_count = 0;
				}
				{
					int col = 0;
					unsigned short * shortp = (unsigned short *)&((char *)_iso_row_offsets)[loop_pos];
					int base = *lut;
					int startcol = first_col;
					do {
						_iso_start_cols[cell_offset + col] = startcol < 0 ? 0 : startcol;
						*shortp++ = (unsigned short)(base + std::max(0, col - first_col));
						++col;
						--startcol;
					} while (col < ISO_DRAW_WIDTH);
				}
				{
					unsigned char * runp = &_iso_run_lengths[ISO_DRAW_WIDTH-1][cell_offset];
					int left_clip = span_count + first_col - ISO_DRAW_WIDTH;
					for (int rowcount = ISO_DRAW_WIDTH; rowcount != 0; --rowcount) {
						for (int i = 0; i < ISO_DRAW_WIDTH; ++i) {
							int run = span_count - std::max(0, i - first_col) - std::max(0, left_clip);
							runp[i] = std::max(0, run);
						}
						runp -= (ISO_DRAW_WIDTH*ISO_DRAW_HEIGHT);
						++left_clip;
					}
				}
				cell_offset += ISO_DRAW_WIDTH;
				loop_pos += (ISO_DRAW_WIDTH * 2);
				++lut;
			} while (loop_pos < ((ISO_DRAW_WIDTH*ISO_DRAW_HEIGHT) *2));
			IsoSpanTablesBuilt = 1;
		}

		if (surface.Bytes_Per_Pixel() == 2) {
			const IsoTileSet * set = (const IsoTileSet *)tileptr->Get_Image_Data();
			if (set != NULL) {
				if (use_z) {
					IsoDrawData.BaseDepth = (unsigned short)(LOWORD(DepthBuffer->Bounds.Y) + LOWORD(DepthBuffer->ScrollOffset) - y - LOWORD(set->Height));
					IsoDrawData.BaseDepth = (int)&((unsigned char *)IsoDrawData.BaseDepth)[height * set->Height / -2];
				}

				int spanw = ISO_DRAW_WIDTH;
				int spanh = ISO_DRAW_HEIGHT;
				const IsoTileRecord * record = set->Fetch_Record_Pointer(subtile);
				if (record == NULL) {
					AlphaLightingRemapInit.Deinit(aremap);
					return;
				}

				IsoDrawData.SpanWidth = spanw;
				IsoDrawData.SpanHeight = spanh;
				IsoDrawData.ClipLeft = 0;
				IsoDrawData.ClipTop = 0;
				IsoDrawData.RowSrcOffset = _iso_row_offsets;
				IsoDrawData.RowStartCol = (unsigned char *)_iso_start_cols;
				IsoDrawData.RowRunLength = (unsigned char *)_iso_run_lengths;

				/*
				 * Clip the ISO_DRAW_WIDTHxISO_DRAW_HEIGHT tile diamond against the clip rectangle, adjusting
				 * the span dimensions and the left/top source offsets. If the tile
				 * clips away entirely, skip straight to the extra-image pass.
				 */

				/// Initial left/top clipping uses the original draw coordinates.
				if (x < cliprect.X) {
					spanw = x - cliprect.X + IsoDrawData.SpanWidth;
					IsoDrawData.ClipLeft = cliprect.X - x;
					IsoDrawData.SpanWidth = spanw;
					work.X = cliprect.X;
					if (spanw <= 0) {
						clipped_out = true;
					}
				}
				if (y < cliprect.Y) {
					spanh = y - cliprect.Y + IsoDrawData.SpanHeight;
					IsoDrawData.ClipTop = cliprect.Y - y;
					IsoDrawData.SpanHeight = spanh;
					work.Y = cliprect.Y;
					if (spanh <= 0) {
						clipped_out = true;
					}
				}

				int right_clip = ISO_DRAW_WIDTH-1;
				int lock_x = work.X;
				int clip_right = cliprect.Width + cliprect.X;
				if (work.X + spanw > clip_right) {
					int overhang = spanw - cliprect.Width - cliprect.X + work.X;
					spanw -= overhang;
					right_clip -= overhang;
					IsoDrawData.SpanWidth = spanw;
					if (spanw <= 0) {
						clipped_out = true;
					}
				}
				int clip_bottom = cliprect.Y + cliprect.Height;
				if (IsoDrawData.SpanHeight + work.Y > clip_bottom) {
					IsoDrawData.SpanHeight = cliprect.Height - work.Y + cliprect.Y;
					if (IsoDrawData.SpanHeight <= 0) {
						clipped_out = true;
					}
				}

				bool skip_extra = false;
				if (!clipped_out) {
					{
						int clip_x = IsoDrawData.ClipLeft;
						if (clip_x > ISO_DRAW_WIDTH-1) {
							clip_x = ISO_DRAW_WIDTH-1;
							IsoDrawData.ClipLeft = clip_x;
						}
						int clip_y = IsoDrawData.ClipTop;
						if (clip_y > ISO_DRAW_HEIGHT-1) {
							clip_y = ISO_DRAW_HEIGHT-1;
							IsoDrawData.ClipTop = clip_y;
						}
						int table_off = clip_x + ISO_DRAW_WIDTH * clip_y;
						IsoDrawData.RowSrcOffset = &_iso_row_offsets[table_off];
						IsoDrawData.RowStartCol = (unsigned char *)&_iso_start_cols[table_off];
						IsoDrawData.RowRunLength = (unsigned char *)&_iso_run_lengths[std::max(0, right_clip)][table_off];
					}
					IsoDrawData.SurfacePitch = surface.Stride();
					IsoDrawData.DestPtr = (unsigned short *)surface.Lock(Point2D(lock_x, work.Y));
					if (IsoDrawData.DestPtr == NULL) {
						skip_extra = true;
					} else {
						unsigned short * zrow;
						unsigned short * arow;

						if (!record->IsHasZData) {
							use_z = 0;
						} else if (use_z) {
							IsoDrawData.DepthPtr = (unsigned short *)DepthBuffer->Get_Buffer_Offset(Point2D(lock_x, work.Y - TacticalRect.Y));
							IsoDrawData.DepthWidth = DepthBuffer->BufferWidth;
						}
						arow = (unsigned short *)AlphaBuffer->Get_Buffer_Offset(Point2D(lock_x, work.Y - TacticalRect.Y));
						IsoDrawData.AlphaPtr = arow;
						IsoDrawData.AlphaWidth = AlphaBuffer->Get_Buffer_Width();
						IsoDrawData.SrcPixel = (int)(record + 1);
						drawer = (LightConvertClass *)IsoDrawData.SrcPixel;
						if (fill || fog) {
							IsoDrawData.FillDepth = 0;
						} else {
							IsoDrawData.FillDepth = 0xFFFF;
						}
						IsoDrawData.FogColor = IsoDrawData.HalfbrightMask & (fog_color >> 1);
						if (use_z && record->IsHasZData) {
							IsoDrawData.SrcDepth = (int)record + record->ZDataOffset;
						}
						IsoDrawData.ImageBase = (unsigned char *)(record + 1);
						IsoDrawData.DepthBase = (unsigned char *)IsoDrawData.SrcDepth;

						if (use_z) {
							zrow = IsoDrawData.DepthPtr;
							if ((unsigned int)&IsoDrawData.DepthPtr[IsoDrawData.SpanWidth + 2 + IsoDrawData.SpanHeight * DepthBuffer->BufferWidth] >= DepthBuffer->Get_Buffer_End()) {
								if (fill) {

									/*
									 * Fill pass: stamp FillDepth into the Z buffer and FogColor
									 * into the surface across the whole span.
									 */
									unsigned short * destrow = IsoDrawData.DestPtr;
									for (int row = 0; row < IsoDrawData.SpanHeight; ++row) {
										int start = *IsoDrawData.RowStartCol * 2;
										IsoDrawData.RowDest = (unsigned short *)destrow;
										IsoDrawData.RowDepth = zrow;
										IsoDrawData.DestPtr = (unsigned short *)((char *)destrow + start);
										IsoDrawData.DepthPtr = (unsigned short *)((char *)zrow + start);
										IsoDrawData.DepthPtr = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)IsoDrawData.DepthPtr);
										int run = *IsoDrawData.RowRunLength;
										if (run > 0) {
											do {
												*IsoDrawData.DepthPtr = IsoDrawData.FillDepth;
												*IsoDrawData.DestPtr = (unsigned short)IsoDrawData.FogColor;
												++IsoDrawData.DestPtr;
												++IsoDrawData.DepthPtr;
												IsoDrawData.DepthPtr = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)IsoDrawData.DepthPtr);
												--run;
											} while (run != 0);
										}
										destrow = (unsigned short *)((char *)IsoDrawData.RowDest + IsoDrawData.SurfacePitch);
										IsoDrawData.DestPtr = destrow;
										zrow = &IsoDrawData.RowDepth[IsoDrawData.DepthWidth];
										IsoDrawData.DepthPtr = zrow;
										IsoDrawData.RowStartCol += ISO_DRAW_WIDTH;
										IsoDrawData.RowRunLength += ISO_DRAW_WIDTH;
									}
								} else if (depth_only) {

									/*
									 * Depth-only fill: stamp FillDepth across the span without
									 * touching the surface (occludes without drawing).
									 */
									for (int row = 0; row < IsoDrawData.SpanHeight; ++row) {
										IsoDrawData.RowDepth = zrow;
										IsoDrawData.DepthPtr = &zrow[*IsoDrawData.RowStartCol];
										IsoDrawData.DepthPtr = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)IsoDrawData.DepthPtr);
										int run = *IsoDrawData.RowRunLength;
										if (run > 0) {
											do {
												*IsoDrawData.DepthPtr = IsoDrawData.FillDepth;
												++IsoDrawData.DepthPtr;
												IsoDrawData.DepthPtr = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)IsoDrawData.DepthPtr);
												--run;
											} while (run != 0);
										}
										zrow = &IsoDrawData.RowDepth[IsoDrawData.DepthWidth];
										IsoDrawData.RowStartCol += ISO_DRAW_WIDTH;
										IsoDrawData.RowRunLength += ISO_DRAW_WIDTH;
									}
								} else if (fog) {
									if (!Rule->IsBlendedFog) {

										/*
										 * Checkerboard fog: as above but only every other pixel of
										 * the span is darkened, so the fog reads as a dither.
										 */
										unsigned char * destrow = (unsigned char *)IsoDrawData.DestPtr;
										for (int row = 0; row < IsoDrawData.SpanHeight; ++row) {
											unsigned short rowoff = *IsoDrawData.RowSrcOffset;
											IsoDrawData.SrcPixel = (int)IsoDrawData.ImageBase + rowoff;
											int start = *IsoDrawData.RowStartCol * 2;
											IsoDrawData.RowDest = (unsigned short *)destrow;
											IsoDrawData.RowDepth = zrow;
											IsoDrawData.DestPtr = (unsigned short *)&destrow[start];
											IsoDrawData.DepthPtr = (unsigned short *)((char *)zrow + start);
											IsoDrawData.DepthPtr = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)IsoDrawData.DepthPtr);
											int checker = ((unsigned char *)IsoDrawData.SrcPixel - (unsigned char *)drawer + row + IsoDrawData.ClipTop) & 1;
											int run = *IsoDrawData.RowRunLength;
											if (run > 0) {
												do {
													if (checker) {
														*IsoDrawData.DestPtr = (unsigned short)IsoDrawData.FogColor;
													}
													*IsoDrawData.DepthPtr = 0;
													checker = checker == 0;
													++IsoDrawData.SrcPixel;
													++IsoDrawData.DestPtr;
													++IsoDrawData.DepthPtr;
													IsoDrawData.DepthPtr = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)IsoDrawData.DepthPtr);
													--run;
												} while (run != 0);
											}
											destrow = (unsigned char *)((char *)IsoDrawData.RowDest + IsoDrawData.SurfacePitch);
											IsoDrawData.DestPtr = (unsigned short *)destrow;
											zrow = &IsoDrawData.RowDepth[IsoDrawData.DepthWidth];
											IsoDrawData.DepthPtr = zrow;
											IsoDrawData.RowStartCol += ISO_DRAW_WIDTH;
											IsoDrawData.RowRunLength += ISO_DRAW_WIDTH;
										}
									} else {

										/*
										 * Blended fog: where the tile pixel wins the depth test,
										 * darken the surface pixel and clear its depth to zero.
										 */
										unsigned char * destrow = (unsigned char *)IsoDrawData.DestPtr;
										for (int row = 0; row < IsoDrawData.SpanHeight; ++row) {
											IsoDrawData.SrcPixel = (int)IsoDrawData.ImageBase + *IsoDrawData.RowSrcOffset;
											int start = *IsoDrawData.RowStartCol * 2;
											IsoDrawData.RowDest = (unsigned short *)destrow;
											IsoDrawData.RowDepth = zrow;
											IsoDrawData.DestPtr = (unsigned short *)&destrow[start];
											IsoDrawData.DepthPtr = (unsigned short *)((char *)zrow + start);
											IsoDrawData.DepthPtr = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)IsoDrawData.DepthPtr);
											int run = *IsoDrawData.RowRunLength;
											if (run > 0) {
												do {
													if (*IsoDrawData.DepthPtr > (int)IsoDrawData.FillDepth) {
														*IsoDrawData.DestPtr = (unsigned short)IsoDrawData.FogColor + (IsoDrawData.HalfbrightMask & (*IsoDrawData.DestPtr >> 1));
													}
													*IsoDrawData.DepthPtr = 0;
													++IsoDrawData.SrcPixel;
													++IsoDrawData.DestPtr;
													++IsoDrawData.DepthPtr;
													IsoDrawData.DepthPtr = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)IsoDrawData.DepthPtr);
													--run;
												} while (run != 0);
											}
											destrow = (unsigned char *)((char *)IsoDrawData.RowDest + IsoDrawData.SurfacePitch);
											IsoDrawData.DestPtr = (unsigned short *)destrow;
											zrow = &IsoDrawData.RowDepth[IsoDrawData.DepthWidth];
											IsoDrawData.DepthPtr = zrow;
											IsoDrawData.RowStartCol += ISO_DRAW_WIDTH;
											IsoDrawData.RowRunLength += ISO_DRAW_WIDTH;
										}
									}
								} else {

									/*
									 * Default depth-buffered draw: depth-test each tile pixel and,
									 * if it wins, write its lit color and update the Z buffer.
									 * Both the Z and alpha cursors wrap at their buffer ends.
									 */
									unsigned char * destrow = (unsigned char *)IsoDrawData.DestPtr;
									for (int row = 0; row < IsoDrawData.SpanHeight; ++row) {
										unsigned short rowoff = *IsoDrawData.RowSrcOffset;
										IsoDrawData.SrcPixel = (int)IsoDrawData.ImageBase + rowoff;
										IsoDrawData.SrcDepth = (int)IsoDrawData.DepthBase + rowoff;
										int startcol = *IsoDrawData.RowStartCol * 2;
										IsoDrawData.RowDest = (unsigned short *)destrow;
										IsoDrawData.RowDepth = zrow;
										IsoDrawData.DepthPtr = (unsigned short *)((char *)zrow + startcol);
										IsoDrawData.DestPtr = (unsigned short *)((char *)destrow + startcol);
										IsoDrawData.DepthPtr = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)IsoDrawData.DepthPtr);
										IsoDrawData.RowAlpha = arow;
										IsoDrawData.AlphaPtr = (unsigned short *)((char *)arow + startcol);
										IsoDrawData.AlphaPtr = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)IsoDrawData.AlphaPtr);
										int run = *IsoDrawData.RowRunLength;
										if (run > 0) {
											do {
												unsigned short pixeldepth = (unsigned short)IsoDrawData.BaseDepth + *(unsigned char *)IsoDrawData.SrcDepth;
												if (*IsoDrawData.DepthPtr >= pixeldepth) {
													*IsoDrawData.DepthPtr = pixeldepth;
													*IsoDrawData.DestPtr = IsoDrawData.PixelTranslate[*(unsigned char *)IsoDrawData.SrcPixel | IsoDrawData.LightRemap[*IsoDrawData.AlphaPtr]];
												}
												++IsoDrawData.SrcDepth;
												++IsoDrawData.DepthPtr;
												++IsoDrawData.SrcPixel;
												++IsoDrawData.DestPtr;
												IsoDrawData.DepthPtr = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)IsoDrawData.DepthPtr);
												++IsoDrawData.AlphaPtr;
												IsoDrawData.AlphaPtr = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)IsoDrawData.AlphaPtr);
												--run;
											} while (run != 0);
										}
										destrow = (unsigned char *)((char *)IsoDrawData.RowDest + IsoDrawData.SurfacePitch);
										IsoDrawData.DestPtr = (unsigned short *)destrow;
										zrow = &IsoDrawData.RowDepth[IsoDrawData.DepthWidth];
										IsoDrawData.DepthPtr = zrow;
										arow = &IsoDrawData.RowAlpha[IsoDrawData.AlphaWidth];
										IsoDrawData.AlphaPtr = arow;
										IsoDrawData.RowSrcOffset += ISO_DRAW_WIDTH;
										IsoDrawData.RowStartCol += ISO_DRAW_WIDTH;
										IsoDrawData.RowRunLength += ISO_DRAW_WIDTH;
									}
								}
							} else {
								if (fill) {
									unsigned char * destrow = (unsigned char *)IsoDrawData.DestPtr;
									for (int row = 0; row < IsoDrawData.SpanHeight; ++row) {
										int start = *IsoDrawData.RowStartCol * 2;
										IsoDrawData.RowDest = (unsigned short *)destrow;
										IsoDrawData.RowDepth = zrow;
										IsoDrawData.DepthPtr = (unsigned short *)((char *)zrow + start);
										IsoDrawData.DestPtr = (unsigned short *)&destrow[start];
										int run = *IsoDrawData.RowRunLength;
										if (run > 0) {
											do {
												*IsoDrawData.DepthPtr = IsoDrawData.FillDepth;
												*IsoDrawData.DestPtr = (unsigned short)IsoDrawData.FogColor;
												++IsoDrawData.DepthPtr;
												++IsoDrawData.DestPtr;
												--run;
											} while (run != 0);
										}
										destrow = (unsigned char *)((char *)IsoDrawData.RowDest + IsoDrawData.SurfacePitch);
										IsoDrawData.DestPtr = (unsigned short *)destrow;
										zrow = &IsoDrawData.RowDepth[IsoDrawData.DepthWidth];
										IsoDrawData.DepthPtr = zrow;
										IsoDrawData.RowStartCol += ISO_DRAW_WIDTH;
										IsoDrawData.RowRunLength += ISO_DRAW_WIDTH;
									}
								} else if (depth_only) {
									for (int row = 0; row < IsoDrawData.SpanHeight; ++row) {
										IsoDrawData.RowDepth = zrow;
										IsoDrawData.DepthPtr = &zrow[*IsoDrawData.RowStartCol];
										int run = *IsoDrawData.RowRunLength;
										if (run > 0) {
											do {
												*IsoDrawData.DepthPtr = IsoDrawData.FillDepth;
												++IsoDrawData.DepthPtr;
												--run;
											} while (run != 0);
										}
										zrow = &IsoDrawData.RowDepth[IsoDrawData.DepthWidth];
										IsoDrawData.RowStartCol += ISO_DRAW_WIDTH;
										IsoDrawData.RowRunLength += ISO_DRAW_WIDTH;
									}
								} else if (fog) {
									if (!Rule->IsBlendedFog) {
										unsigned char * destrow = (unsigned char *)IsoDrawData.DestPtr;
										for (int row = 0; row < IsoDrawData.SpanHeight; ++row) {
											unsigned short rowoff = *IsoDrawData.RowSrcOffset;
											IsoDrawData.SrcPixel = (int)IsoDrawData.ImageBase + rowoff;
											int start = *IsoDrawData.RowStartCol;
											IsoDrawData.RowDest = (unsigned short *)destrow;
											IsoDrawData.RowDepth = zrow;
											IsoDrawData.DestPtr = (unsigned short *)&destrow[start * 2];
											IsoDrawData.DepthPtr = &zrow[start];
											int run = *IsoDrawData.RowRunLength;
											int checker = (unsigned char *)IsoDrawData.SrcPixel - (unsigned char *)drawer;
											checker += row;
											checker += IsoDrawData.ClipTop;
											checker &= 1;
											if (run > 0) {
												do {
													if (checker) {
														*IsoDrawData.DestPtr = (unsigned short)IsoDrawData.FogColor;
													}
													*IsoDrawData.DepthPtr = 0;
													checker = checker == 0;
													++IsoDrawData.SrcPixel;
													++IsoDrawData.DestPtr;
													++IsoDrawData.DepthPtr;
													--run;
												} while (run != 0);
											}
											destrow = (unsigned char *)((char *)IsoDrawData.RowDest + IsoDrawData.SurfacePitch);
											IsoDrawData.DestPtr = (unsigned short *)destrow;
											zrow = &IsoDrawData.RowDepth[IsoDrawData.DepthWidth];
											IsoDrawData.DepthPtr = zrow;
											IsoDrawData.RowStartCol += ISO_DRAW_WIDTH;
											IsoDrawData.RowRunLength += ISO_DRAW_WIDTH;
										}
									} else {
										unsigned char * destrow = (unsigned char *)IsoDrawData.DestPtr;
										for (int row = 0; row < IsoDrawData.SpanHeight; ++row) {
											IsoDrawData.SrcPixel = (int)IsoDrawData.ImageBase + *IsoDrawData.RowSrcOffset;
											int start = *IsoDrawData.RowStartCol;
											IsoDrawData.RowDest = (unsigned short *)destrow;
											IsoDrawData.RowDepth = zrow;
											IsoDrawData.DestPtr = (unsigned short *)&destrow[start * 2];
											IsoDrawData.DepthPtr = &zrow[start];
											int run = *IsoDrawData.RowRunLength;
											if (run > 0) {
												do {
													if (*IsoDrawData.DepthPtr > (int)IsoDrawData.FillDepth) {
														*IsoDrawData.DestPtr = (unsigned short)IsoDrawData.FogColor + (IsoDrawData.HalfbrightMask & (*IsoDrawData.DestPtr >> 1));
													}
													*IsoDrawData.DepthPtr = 0;
													++IsoDrawData.SrcPixel;
													++IsoDrawData.DestPtr;
													++IsoDrawData.DepthPtr;
													--run;
												} while (run != 0);
											}
											destrow = (unsigned char *)((char *)IsoDrawData.RowDest + IsoDrawData.SurfacePitch);
											IsoDrawData.DestPtr = (unsigned short *)destrow;
											zrow = &IsoDrawData.RowDepth[IsoDrawData.DepthWidth];
											IsoDrawData.DepthPtr = zrow;
											IsoDrawData.RowStartCol += ISO_DRAW_WIDTH;
											IsoDrawData.RowRunLength += ISO_DRAW_WIDTH;
										}
									}
								} else {
									unsigned char * destrow = (unsigned char *)IsoDrawData.DestPtr;
									for (int row = 0; row < IsoDrawData.SpanHeight; ++row) {
										unsigned short rowoff = *IsoDrawData.RowSrcOffset;
										IsoDrawData.SrcPixel = (int)IsoDrawData.ImageBase + rowoff;
										IsoDrawData.SrcDepth = (int)IsoDrawData.DepthBase + rowoff;
										int start = *IsoDrawData.RowStartCol * 2;
										IsoDrawData.RowDest = (unsigned short *)destrow;
										IsoDrawData.RowDepth = zrow;
										IsoDrawData.RowAlpha = arow;
										IsoDrawData.DestPtr = (unsigned short *)&destrow[start];
										IsoDrawData.DepthPtr = (unsigned short *)((char *)zrow + start);
										IsoDrawData.AlphaPtr = (unsigned short *)((char *)arow + start);
										int run = *IsoDrawData.RowRunLength;
										if (run > 0) {
											do {
												unsigned short pixeldepth = (unsigned short)IsoDrawData.BaseDepth + *(unsigned char *)IsoDrawData.SrcDepth;
												if (*IsoDrawData.DepthPtr >= pixeldepth) {
													*IsoDrawData.DepthPtr = pixeldepth;
													*IsoDrawData.DestPtr = IsoDrawData.PixelTranslate[*(unsigned char *)IsoDrawData.SrcPixel | IsoDrawData.LightRemap[*IsoDrawData.AlphaPtr]];
												}
												++IsoDrawData.SrcDepth;
												++IsoDrawData.DepthPtr;
												++IsoDrawData.AlphaPtr;
												++IsoDrawData.SrcPixel;
												++IsoDrawData.DestPtr;
												--run;
											} while (run != 0);
										}
										destrow = (unsigned char *)((char *)IsoDrawData.RowDest + IsoDrawData.SurfacePitch);
										IsoDrawData.DestPtr = (unsigned short *)destrow;
										zrow = &IsoDrawData.RowDepth[IsoDrawData.DepthWidth];
										IsoDrawData.DepthPtr = zrow;
										arow = &IsoDrawData.RowAlpha[IsoDrawData.AlphaWidth];
										IsoDrawData.AlphaPtr = arow;
										IsoDrawData.RowSrcOffset += ISO_DRAW_WIDTH;
										IsoDrawData.RowStartCol += ISO_DRAW_WIDTH;
										IsoDrawData.RowRunLength += ISO_DRAW_WIDTH;
									}
								}
							}
						} else {
							if ((unsigned int)&arow[IsoDrawData.SpanWidth + 2 + IsoDrawData.SpanHeight * AlphaBuffer->Get_Buffer_Width()] >= AlphaBuffer->Get_Buffer_End()) {
								unsigned char * destrow = (unsigned char *)IsoDrawData.DestPtr;
								for (int row = 0; row < IsoDrawData.SpanHeight; ++row) {
									IsoDrawData.SrcPixel = (int)IsoDrawData.ImageBase + *IsoDrawData.RowSrcOffset;
									int startcol = *IsoDrawData.RowStartCol * 2;
									IsoDrawData.RowDest = (unsigned short *)destrow;
									IsoDrawData.RowAlpha = arow;
									IsoDrawData.DestPtr = (unsigned short *)&destrow[startcol];
									IsoDrawData.AlphaPtr = (unsigned short *)((char *)arow + startcol);
									IsoDrawData.AlphaPtr = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)IsoDrawData.AlphaPtr);
									int run = *IsoDrawData.RowRunLength;
									if (run > 0) {
										do {
											*IsoDrawData.DestPtr = IsoDrawData.PixelTranslate[*(unsigned char *)IsoDrawData.SrcPixel | IsoDrawData.LightRemap[*IsoDrawData.AlphaPtr]];
											++IsoDrawData.DestPtr;
											++IsoDrawData.SrcPixel;
											++IsoDrawData.AlphaPtr;
											IsoDrawData.AlphaPtr = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)IsoDrawData.AlphaPtr);
											--run;
										} while (run != 0);
									}
									destrow = (unsigned char *)((char *)IsoDrawData.RowDest + IsoDrawData.SurfacePitch);
									IsoDrawData.DestPtr = (unsigned short *)destrow;
									arow = &IsoDrawData.RowAlpha[IsoDrawData.AlphaWidth];
									IsoDrawData.AlphaPtr = arow;
									IsoDrawData.RowSrcOffset += ISO_DRAW_WIDTH;
									IsoDrawData.RowStartCol += ISO_DRAW_WIDTH;
									IsoDrawData.RowRunLength += ISO_DRAW_WIDTH;
								}
							} else {
								unsigned char * destrow = (unsigned char *)IsoDrawData.DestPtr;
								for (int row = 0; row < IsoDrawData.SpanHeight; ++row) {
									IsoDrawData.SrcPixel = (int)IsoDrawData.ImageBase + *IsoDrawData.RowSrcOffset;
									int startcol = *IsoDrawData.RowStartCol * 2;
									IsoDrawData.RowDest = (unsigned short *)destrow;
									IsoDrawData.RowAlpha = arow;
									IsoDrawData.DestPtr = (unsigned short *)&destrow[startcol];
									IsoDrawData.AlphaPtr = (unsigned short *)((char *)arow + startcol);
									int run = *IsoDrawData.RowRunLength;
									if (run > 0) {
										do {
											*IsoDrawData.DestPtr = IsoDrawData.PixelTranslate[*(unsigned char *)IsoDrawData.SrcPixel | IsoDrawData.LightRemap[*IsoDrawData.AlphaPtr]];
											++IsoDrawData.DestPtr;
											++IsoDrawData.SrcPixel;
											++IsoDrawData.AlphaPtr;
											--run;
										} while (run != 0);
									}
									destrow = (unsigned char *)((char *)IsoDrawData.RowDest + IsoDrawData.SurfacePitch);
									IsoDrawData.RowSrcOffset += ISO_DRAW_WIDTH;
									arow = &IsoDrawData.RowAlpha[IsoDrawData.AlphaWidth];
									IsoDrawData.DestPtr = (unsigned short *)destrow;
									IsoDrawData.AlphaPtr = arow;
									IsoDrawData.RowStartCol += ISO_DRAW_WIDTH;
									IsoDrawData.RowRunLength += ISO_DRAW_WIDTH;
								}
							}
						}
						surface.Unlock();
					}
				}

				if (!skip_extra) {
					if (record->IsHasExtraData && !fill && !depth_only && !fog) {
						int ex = record->ExtraX - record->X + x;
						int ey = record->ExtraY - record->Y + y;
						IsoDrawData.SpanWidth = record->ExtraWidth;
						IsoDrawData.SpanHeight = record->ExtraHeight;
						IsoDrawData.ClipLeft = 0;
						IsoDrawData.ClipTop = 0;
						bool extra_ok = true;

						if (ex < cliprect.X) {
							IsoDrawData.SpanWidth = ex - cliprect.X + record->ExtraWidth;
							IsoDrawData.ClipLeft = cliprect.X - ex;
							ex = cliprect.X;
							if (IsoDrawData.SpanWidth <= 0) {
								extra_ok = false;
							}
						}
						if (extra_ok && ey < cliprect.Y) {
							IsoDrawData.SpanHeight = ey - cliprect.Y + IsoDrawData.SpanHeight;
							IsoDrawData.ClipTop = cliprect.Y - ey;
							ey = cliprect.Y;
							if (IsoDrawData.SpanHeight <= 0) {
								extra_ok = false;
							}
						}
						if (extra_ok && (int)IsoDrawData.SpanWidth + ex > clip_right) {
							IsoDrawData.SpanWidth = cliprect.Width - ex + cliprect.X;
							if (IsoDrawData.SpanWidth <= 0) {
								extra_ok = false;
							}
						}
						if (extra_ok && IsoDrawData.SpanHeight + ey > clip_bottom) {
							IsoDrawData.SpanHeight = cliprect.Height - ey + cliprect.Y;
							if (IsoDrawData.SpanHeight <= 0) {
								extra_ok = false;
							}
						}
						if (extra_ok) {
							IsoDrawData.ImageRowStep = record->ExtraWidth - IsoDrawData.SpanWidth;
							IsoDrawData.SurfacePitch = surface.Stride() - IsoDrawData.SpanWidth * surface.Bytes_Per_Pixel();
							if (use_z) {
								IsoDrawData.DepthPtr = (unsigned short *)DepthBuffer->Get_Buffer_Offset(Point2D(ex, ey - TacticalRect.Y));
								IsoDrawData.DepthWidth = DepthBuffer->BufferWidth - IsoDrawData.SpanWidth;
							}
							IsoDrawData.AlphaPtr = (unsigned short *)AlphaBuffer->Get_Buffer_Offset(Point2D(ex, ey - TacticalRect.Y));
							IsoDrawData.AlphaWidth = AlphaBuffer->Get_Buffer_Width() - IsoDrawData.SpanWidth;
							IsoDrawData.DestPtr = (unsigned short *)surface.Lock(Point2D(ex, ey));
							if (IsoDrawData.DestPtr != NULL) {
								IsoDrawData.SrcPixel = (int)record + record->ExtraOffset + IsoDrawData.ClipLeft;
								IsoDrawData.SrcPixel = IsoDrawData.ClipTop * record->ExtraWidth + IsoDrawData.SrcPixel;
								if (use_z) {
									IsoDrawData.SrcDepth = (int)record + record->ExtraZOffset + IsoDrawData.ClipLeft;
									IsoDrawData.SrcDepth = IsoDrawData.ClipTop * record->ExtraWidth + IsoDrawData.SrcDepth;
									if ((unsigned int)&IsoDrawData.DepthPtr[IsoDrawData.SpanWidth + 2 + IsoDrawData.SpanHeight * DepthBuffer->BufferWidth] >= DepthBuffer->Get_Buffer_End()) {

										/*
										 * Depth-tested extra image, Z buffer wrapping.
										 */
										for (int row = 0; row < IsoDrawData.SpanHeight; ++row) {
											for (int col = 0; col < (int)IsoDrawData.SpanWidth; ++col) {
												if (*(unsigned char *)IsoDrawData.SrcPixel) {
													if (*IsoDrawData.DepthPtr >= (unsigned short)((unsigned short)IsoDrawData.BaseDepth + *(unsigned char *)IsoDrawData.SrcDepth)) {
														*IsoDrawData.DestPtr = IsoDrawData.PixelTranslate[*(unsigned char *)IsoDrawData.SrcPixel | IsoDrawData.LightRemap[*IsoDrawData.AlphaPtr]];
														*IsoDrawData.DepthPtr = (unsigned short)IsoDrawData.BaseDepth + *(unsigned char *)IsoDrawData.SrcDepth;
													}
												}
												++IsoDrawData.DestPtr;
												++IsoDrawData.SrcPixel;
												++IsoDrawData.SrcDepth;
												++IsoDrawData.DepthPtr;
												IsoDrawData.DepthPtr = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)IsoDrawData.DepthPtr);
												++IsoDrawData.AlphaPtr;
												IsoDrawData.AlphaPtr = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)IsoDrawData.AlphaPtr);
											}
											IsoDrawData.DestPtr = (unsigned short *)((char *)IsoDrawData.DestPtr + IsoDrawData.SurfacePitch);
											IsoDrawData.SrcPixel += IsoDrawData.ImageRowStep;
											IsoDrawData.SrcDepth += IsoDrawData.ImageRowStep;
											IsoDrawData.DepthPtr = &IsoDrawData.DepthPtr[IsoDrawData.DepthWidth];
											IsoDrawData.DepthPtr = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)IsoDrawData.DepthPtr);
											IsoDrawData.AlphaPtr = &IsoDrawData.AlphaPtr[IsoDrawData.AlphaWidth];
											IsoDrawData.AlphaPtr = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)IsoDrawData.AlphaPtr);
										}
									} else {

										/*
										 * Depth-tested extra image, no wrapping.
										 */
										for (int row = 0; row < IsoDrawData.SpanHeight; ++row) {
											for (int col = 0; col < (int)IsoDrawData.SpanWidth; ++col) {
												if (*(unsigned char *)IsoDrawData.SrcPixel) {
													if (*IsoDrawData.DepthPtr >= (unsigned short)((unsigned short)IsoDrawData.BaseDepth + *(unsigned char *)IsoDrawData.SrcDepth)) {
														*IsoDrawData.DestPtr = IsoDrawData.PixelTranslate[*(unsigned char *)IsoDrawData.SrcPixel | IsoDrawData.LightRemap[*IsoDrawData.AlphaPtr]];
														*IsoDrawData.DepthPtr = (unsigned short)IsoDrawData.BaseDepth + *(unsigned char *)IsoDrawData.SrcDepth;
													}
												}
												++IsoDrawData.DestPtr;
												++IsoDrawData.SrcPixel;
												++IsoDrawData.SrcDepth;
												++IsoDrawData.DepthPtr;
												++IsoDrawData.AlphaPtr;
											}
											IsoDrawData.DestPtr = (unsigned short *)((char *)IsoDrawData.DestPtr + IsoDrawData.SurfacePitch);
											IsoDrawData.SrcPixel += IsoDrawData.ImageRowStep;
											IsoDrawData.SrcDepth += IsoDrawData.ImageRowStep;
											IsoDrawData.DepthPtr = &IsoDrawData.DepthPtr[IsoDrawData.DepthWidth];
											IsoDrawData.AlphaPtr = &IsoDrawData.AlphaPtr[IsoDrawData.AlphaWidth];
										}
									}
								} else if ((unsigned int)&IsoDrawData.AlphaPtr[IsoDrawData.SpanWidth + 2 + IsoDrawData.SpanHeight * AlphaBuffer->Get_Buffer_Width()] >= AlphaBuffer->Get_Buffer_End()) {

									/*
									 * Plain extra image (no depth), alpha buffer wrapping.
									 */
									for (int row = 0; row < IsoDrawData.SpanHeight; ++row) {
										for (int col = 0; col < (int)IsoDrawData.SpanWidth; ++col) {
											if (*(unsigned char *)IsoDrawData.SrcPixel) {
												*IsoDrawData.DestPtr = IsoDrawData.PixelTranslate[*(unsigned char *)IsoDrawData.SrcPixel | IsoDrawData.LightRemap[*IsoDrawData.AlphaPtr]];
											}
											++IsoDrawData.DestPtr;
											++IsoDrawData.AlphaPtr;
											IsoDrawData.AlphaPtr = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)IsoDrawData.AlphaPtr);
											++IsoDrawData.SrcPixel;
										}
										IsoDrawData.DestPtr = (unsigned short *)((char *)IsoDrawData.DestPtr + IsoDrawData.SurfacePitch);
										IsoDrawData.AlphaPtr = &IsoDrawData.AlphaPtr[IsoDrawData.AlphaWidth];
										IsoDrawData.AlphaPtr = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)IsoDrawData.AlphaPtr);
										IsoDrawData.SrcPixel += IsoDrawData.ImageRowStep;
									}
								} else {

									/*
									 * Plain extra image (no depth), no wrapping.
									 */
									for (int row = 0; row < IsoDrawData.SpanHeight; ++row) {
										for (int col = 0; col < (int)IsoDrawData.SpanWidth; ++col) {
											if (*(unsigned char *)IsoDrawData.SrcPixel) {
												*IsoDrawData.DestPtr = IsoDrawData.PixelTranslate[*(unsigned char *)IsoDrawData.SrcPixel | IsoDrawData.LightRemap[*IsoDrawData.AlphaPtr]];
											}
											++IsoDrawData.DestPtr;
											++IsoDrawData.SrcPixel;
											++IsoDrawData.AlphaPtr;
										}
										IsoDrawData.AlphaPtr = &IsoDrawData.AlphaPtr[IsoDrawData.AlphaWidth];
										IsoDrawData.DestPtr = (unsigned short *)((char *)IsoDrawData.DestPtr + IsoDrawData.SurfacePitch);
										IsoDrawData.SrcPixel += IsoDrawData.ImageRowStep;
									}
								}
								surface.Unlock();
							}
						}
					}
				}
			}
		}
		AlphaLightingRemapInit.Deinit(aremap);
	}
}


/// <summary>
/// Fetches a sub-tile's artwork as a plain rectangular image.
/// This routine unpacks the diamond shaped tile data, along with any extra artwork
/// attached to it, into a buffer the caller supplies. It is used where the tile is wanted
/// as an ordinary image rather than as a shape to blit.
/// </summary>
/// <param name="tilenum">The sub-tile index within this tile set.</param>
/// <param name="buffer">Pointer to the destination buffer to unpack the tile into.</param>
/// <param name="width">The width of the destination buffer in pixels.</param>
/// <param name="height">The height of the destination buffer in pixels.</param>
/// <returns>bool; Was the tile unpacked?</returns>
/// <remarks>The destination buffer is cleared first, so the caller need not do so.</remarks>
bool IsometricTileTypeClass::Get_Tile_Image(int tilenum, unsigned char **buffer, int width, int height)
{
	/// The table is spelled with literal 0x20 (space) and 0xDB (box) characters, written as
	/// escapes here so that the file stays plain ASCII.
	#define __ "\x20"
	#define XX "\xDB"

	static unsigned char _tilemask[] = {
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __
		__ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __
		__ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __
		__ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __
		XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX
		__ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __
		__ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __
		__ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __
		__ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ XX XX XX XX __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
		__ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
	};

	#undef __
	#undef XX

	int tilew = 0;
	int tileh = 0;
	memset(*buffer, 0, width * height);
	IsoTileSet *isotile = (IsoTileSet *)Get_Image_Data();

	if (isotile != NULL) {
		tilenum = tilenum % isotile->Tile_Count();
		const IsoTileRecord *record = ((const IsoTileRecord *)isotile->Fetch_Record_Pointer_Unsafe(tilenum));

		if (record != NULL) {
			Get_Tile_Pixel_Dimensions(tilenum, tilew, tileh);

			if (tilew <= width && tileh <= height) {

				unsigned char *buf = *buffer;
				unsigned char *end = &(*buffer)[width * height];

				if (record->IsHasExtraData) {
					buf += width * (record->Y - record->ExtraY);
					if (buf >= end) {
						return(false);
					}
				}

				unsigned char *mask = _tilemask;
				unsigned char *image = (unsigned char *)(record + 1);

				for (int py = 0; py < isotile->Pixel_Height(); py++) {
					for (int px = 0; px < isotile->Pixel_Width(); px++) {

						if (*mask++ != '\x20') {
							*buf = *image;
							image++;
						}

						buf++;
						if (buf >= end) {
							return(false);
						}
					}

					buf += width - isotile->Pixel_Width();
					if (buf >= end) {
						return(false);
					}
				}

				if (record->IsHasExtraData) {

					buf = &(*buffer)[record->ExtraX - record->X];
					if (buf >= end) {
						return(false);
					}

					unsigned char *extraimage = (unsigned char *)record + record->ExtraOffset;

					for (int ey = 0; ey < record->ExtraHeight; ey++) {
						for (int ex = 0; ex < record->ExtraWidth; ex++) {
							if (*extraimage != 0) {
								*buf = *extraimage;
							}
							buf++;
							extraimage++;
							if (buf >= end) {
								return(false);
							}

						}

						buf += width - record->ExtraWidth;
						if (buf >= end) {
							return(false);
						}

					}
				}
				return(true);
			}
		}
		return(false);
	}

	return(true);
}


/// <summary>
/// Converts a map position into isometric screen pixels.
/// </summary>
/// <param name="x">The map X position to convert.</param>
/// <param name="y">The map Y position to convert.</param>
/// <param name="xx">Receives the screen pixel X position.</param>
/// <param name="yy">Receives the screen pixel Y position.</param>
void World_To_Screen(int x, int y, int & xx, int & yy)
{
	xx = x * ISO_TILE_PIXEL_W / 2;
	yy = x * ISO_TILE_PIXEL_H / 2;
	xx += y * ISO_TILE_PIXEL_W / -2;
	yy += y * ISO_TILE_PIXEL_H / 2;
}


/// <summary>
/// Fetches the vertical draw offset of a sub-tile.
/// A tile that carries extra artwork reaches up above its own diamond, and this routine
/// reports how far. Plain tiles have no offset at all.
/// </summary>
/// <param name="tile">The sub-tile index within this tile set.</param>
/// <returns>Returns with the vertical offset in pixels, or zero if the tile is a plain
/// diamond.</returns>
int IsometricTileTypeClass::Get_Y_Offset(int tile)
{
	IsoTileSet const * tileset = (IsoTileSet const *)Get_Image_Data();
	if (tileset != NULL && tile < tileset->Tile_Count()) {
		IsoTileRecord const * record = tileset->Tiles[tile];
		if (record != NULL) {
			if ((record->IsHasExtraData) != 0) {
				return(record->ExtraY - record->Y);
			}
		}
	}

	return(0);
}


/***********************************************************************************************
 * TemplateTypeClass::Create_And_Place -- Creates and places a template object on the map.     *
 *                                                                                             *
 *    This support routine is used by the scenario editor to add a template object to the map  *
 *    and to the game.                                                                         *
 *                                                                                             *
 * INPUT:   cell  -- The cell to place the template object.                                    *
 *                                                                                             *
 * OUTPUT:  bool; Was the template object placed successfully?                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool IsometricTileTypeClass::Create_And_Place(Cell const & cell, HouseClass * house) const
{
	if (new IsometricTileClass(HeapID, cell)) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * TemplateTypeClass::Create_One_Of -- Creates an object of this template type.                *
 *                                                                                             *
 *    This routine will create an object of this type. For certain template objects, such      *
 *    as walls, it is actually created as a building. The "building" wall is converted into    *
 *    a template at the moment of placing down on the map.                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the appropriate object for this template type.           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectClass * IsometricTileTypeClass::Create_One_Of(HouseClass * house) const
{
	((IsometricTileTypeClass *)this)->Load_Tile_Image();
	return(new IsometricTileClass(HeapID, CELL_NONE));
}


/// <summary>
/// Forgets the cached cliff shadow and slope depth shapes.
/// The shapes themselves are left alone; only the pointers to them are dropped.
/// </summary>
void IsometricTileTypeClass::Clear_Z_Shapes(void)
{
	CellShadowShapes = NULL;
	SlopeZShapes[0] = NULL;
	SlopeZShapes[1] = NULL;
	SlopeZShapes[2] = NULL;
	SlopeZShapes[3] = NULL;
}


/// <summary>
/// Adjusts a coordinate to the legal position for this object type.
/// A tile covers its whole cell, so any coordinate within it is as good as another and
/// the coordinate is handed straight back.
/// </summary>
/// <returns>Returns with the coordinate to use.</returns>
Coord const IsometricTileTypeClass::Coord_Fixup(Coord const & coord) const
{
	return(coord);
}


/// <summary>
/// Submits this tile type to the running game state checksum.
/// This routine is used by the multiplayer sync check to prove that every machine agrees
/// about the terrain.
/// </summary>
/// <param name="crc">The checksum engine to submit the object data to.</param>
void IsometricTileTypeClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(HeapID);
	crc(MarbleMadness);
	crc(NonMarbleMadness);
	crc(TileSetBaseID);
	crc(PreviewTiles.Count());
	if (NextTileTypeInSet != NULL) {
		crc(NextTileTypeInSet->Fetch_ID());
	}
	crc(ToSnowTheater);
	crc(ToTemperateTheater);
	crc(AttachesTo);
	crc(ZAdjust);
	crc(Unused1);
	crc(IsMorphable);
	crc(IsShadowCaster);
	crc(Width);
	crc(Height);
	crc(Unused2);
	crc(NumTileTypesInSet);
}


/// <summary>
/// Rebuilds the presentation state this tile type carries.
/// The lighting converters and the preview colors do not survive a save game, so they are
/// discarded here and rebuilt from the artwork once the object has been read.
/// </summary>
void IsometricTileTypeClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	for (int i = 0; i < TileDrawers.Count(); i++) {
		delete TileDrawers[i];
		TileDrawers[i] = NULL;
	}

	Build_Preview_Tiles();
}


/// <summary>
/// Lists the members this tile type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void IsometricTileTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HeapID);
	stream.Serialize(MarbleMadness);
	stream.Serialize(NonMarbleMadness);
	stream.Serialize(TileSetBaseID);
	// PreviewTiles -- radar and preview colors, baked again as this loads.
	stream.Serialize(NextTileTypeInSet);
	stream.Serialize(ToSnowTheater);
	stream.Serialize(ToTemperateTheater);
	stream.Serialize(Anim);
	stream.Serialize(Offset);
	stream.Serialize(AttachesTo);
	stream.Serialize(ZAdjust);
	stream.Serialize(Unused1);
	stream.Serialize(IsMorphable);
	stream.Serialize(IsShadowCaster);
	stream.Serialize(IsAllowToPlace);
	stream.Serialize(IsRequiredForRMG);
	stream.Serialize(Width);
	stream.Serialize(Height);
	stream.Serialize(Unused2);
	stream.Serialize(NumTileTypesInSet);
	stream.Serialize(IsFileLoaded);
	stream.Serialize(Filename);
	stream.Serialize(IsAllowBurrowing);
	stream.Serialize(IsAllowTiberium);
	stream.Serialize(UseCount);
}


/// <summary>
/// Fetches the class identifier that this tile type persists under.
/// </summary>
/// <param name="retval">Receives the class identifier.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT STDMETHODCALLTYPE IsometricTileTypeClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_IsometricTileTypeClass;
	return(S_OK);
}


/// <summary>
/// Removes any reference this tile type has to the specified object.
/// This routine is called when an object is about to disappear, so that the tile type is
/// not left pointing at something that no longer exists.
/// </summary>
/// <param name="target">The object that is going away.</param>
void IsometricTileTypeClass::Detach(AbstractClass const * target, bool all)
{
	if (NextTileTypeInSet == target) {
		NextTileTypeInSet = NULL;
	}

	if (Anim != ANIM_NONE && AnimTypes[Anim] == target) {
		Anim = ANIM_NONE;
	}
}


/// <summary>
/// Fetches a single preview color of a sub-tile at the specified lighting level.
/// </summary>
/// <param name="tile">The sub-tile index within this tile set.</param>
/// <param name="level">The lighting level to fetch the preview color for.</param>
/// <param name="use_high_color">Should the high color be fetched rather than the low one?</param>
/// <returns>Returns with the 16bpp preview color.</returns>
unsigned short IsometricTileTypeClass::Preview_Tile_Color(int tile, int level, bool use_high_color)
{
	return(PreviewTiles[tile][(use_high_color ? 1 : 0) + level * 2]);
}


/// <summary>
/// Fetches the preview colors of a sub-tile at the specified lighting level.
/// The tile artwork is loaded on demand, so this routine is safe to call on a tile type
/// that has been trimmed out of memory.
/// </summary>
/// <param name="tile">The sub-tile index within this tile set.</param>
/// <param name="level">The lighting level to fetch the preview colors for.</param>
/// <returns>Returns with a pointer to the low and high preview colors for that level.</returns>
unsigned short *IsometricTileTypeClass::Preview_Tile(int tile, int level)
{
	Load_Tile_Image();
	return(&PreviewTiles[tile][level * 2]);
}


/// <summary>
/// Builds the preview colors for every sub-tile in this set.
/// This routine bakes the low and high control colors of each tile record into a small
/// ramp of 16bpp colors, one pair per lighting level, so that the radar and the map
/// preview can draw terrain without touching the tile artwork itself. Any ramps built
/// previously are discarded first.
/// </summary>
void IsometricTileTypeClass::Build_Preview_Tiles(void)
{
	int i;
	int j;

	IsoTileSet const * tileset = (IsoTileSet const *)ImageData;

	for (i = 0; i < PreviewTiles.Count(); i++) {
		if (PreviewTiles[i] != NULL) {
			delete [] PreviewTiles[i];
		}
	}

	PreviewTiles.Clear();

	for (i = 0; i < tileset->Tile_Count(); i++) {
		if (tileset->Tiles[i] != NULL) {
			unsigned short * buffer = new unsigned short[24 + 2];
			IsoTileRecord const * record = tileset->Tiles[i];

			PreviewTiles.Add(buffer);

			RGBClass low(record->LowColor.Red, record->LowColor.Green, record->LowColor.Blue);
			RGBClass high(record->HighColor.Red, record->HighColor.Green, record->HighColor.Blue);

			static float const _brightness_factor = 1.4f;

			RGBClass alow;
			alow.Set(low, _brightness_factor);

			RGBClass ahigh;
			ahigh.Set(high, _brightness_factor);

			unsigned short * ptr = buffer;

			for (j = 0; j <= 12; j++) {
				RGBClass tmp = RGBClass().Lerp(low, alow, float((double)j / 12.0f));
				ptr[0] = DSurface::Build_Hicolor_Pixel(tmp.Get_Red(), tmp.Get_Green(), tmp.Get_Blue());

				tmp = RGBClass().Lerp(high, ahigh, float((double)j / 12.0f));
				ptr[1] = DSurface::Build_Hicolor_Pixel(tmp.Get_Red(), tmp.Get_Green(), tmp.Get_Blue());

				ptr += 2;
			}
		} else {
			PreviewTiles.Add(NULL);
		}
	}
}


/// <summary>
/// Picks one of the three ice tile sets at random.
/// This routine is used when ice is laid down so that a frozen stretch of map does not
/// end up paved with a single monotonous pattern.
/// </summary>
/// <returns>Returns with the tile set that was picked.</returns>
IsometricTileType Pick_Ice_Tile_Set(void)
{
	int r = Scen->RandomNumber(0,2);
	if (r == 0) {
		return(IsometricTileTypeClass::Ice1Set);
	}
	if (r == 1) {
		return(IsometricTileTypeClass::Ice2Set);
	}
	return(IsometricTileTypeClass::Ice3Set);
}
