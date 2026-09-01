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

/* $Header: /CounterStrike/ODATA.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : ODATA.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 16, 1994                                                 *
 *                                                                                             *
 *                  Last Update : August 14, 1996 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   OverlayTypeClass::As_Reference -- Fetch a reference to the overlay type specified.        *
 *   OverlayTypeClass::Coord_Fixup -- Adjust the coord to be legal for assignment.             *
 *   OverlayTypeClass::Create_And_Place -- Creates and places a overlay object on the map.     *
 *   OverlayTypeClass::Create_One_Of -- Creates an object of this overlay type.                *
 *   OverlayTypeClass::Display -- Displays a generic representation of overlay.                *
 *   OverlayTypeClass::Draw_It -- Draws the overlay image at location specified.               *
 *   OverlayTypeClass::From_Name -- Determine overlay from ASCII name.                         *
 *   OverlayTypeClass::Init -- Initialize the overlay graphic data per theater.                *
 *   OverlayTypeClass::Init_Heap -- Initialize the overlay type class heap.                    *
 *   OverlayTypeClass::Occupy_List -- Determines occupation list.                              *
 *   OverlayTypeClass::One_Time -- Loads all the necessary general overlay shape data.         *
 *   OverlayTypeClass::OverlayTypeClass -- Constructor for overlay type objects.               *
 *   OverlayTypeClass::Prep_For_Add -- Prepares to add overlay to scenario.                    *
 *   OverlayTypeClass::Radar_Icon -- Gets a pointer to the radar icons                         *
 *   OverlayTypeClass::operator delete -- Returns an overlay type object back to the pool.     *
 *   OverlayTypeClass::operator new -- Allocate an overlay type class object from pool.        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "overtype.h"

#include "_convert.h"
#include "_map.h"
#include "_mixfile.h"
#include "_rect.h"
#include "_rules.h"
#include "_surface.h"
#include "_theater.h"
#include "animtype.h"
#include "ccfile.h"
#include "ccini.h"
#include "data.h"
#include "dbgprint.h"
#include "draw.h"
#include "findmake.h"
#include "init.h"
#include "mixfile.h"
#include "overlay.h"
#include "resource.h"
#include "savestream.h"
#include "scenario.h"
#include "shapeset.h"
#include "sun.h"
#include "swizzle.h"
#include "tracker.h"


/// <summary>
/// Releases shape data that this type loaded through the file layer.
/// </summary>
static void Free_Demand_Loaded_Shape(void const *& data)
{
	delete [] (char *)data;
	data = NULL;
}


/***********************************************************************************************
 * OverlayTypeClass::OverlayTypeClass -- Constructor for overlay type objects.                 *
 *                                                                                             *
 *    This is the constructor for the overlay types.                                           *
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
OverlayTypeClass::OverlayTypeClass(char const * ininame) :
	BASECLASS(ininame),
	HeapID(OVERLAY_NONE),
	Land(LAND_CLEAR),
	DamageLevels(1),
	DamagePoints(1),
	IsWall(false),
	IsHigh(false),
	IsTiberium(false),
	IsCrate(false),
	IsCrateTrigger(false),
	IsNoUseTileLandType(true),
	IsVeinholeMonster(false),
	IsVeins(false),
	IsExplosive(false),
	DemandLoad(false),
	IsChainReaction(false),
	IsOverrides(false),
	CellAnim(NULL),
	IsDrawFlat(true),
	IsARock(false)
{
	Create_ID();

	IsStealthy = true;
	IsSelectable = false;
	IsInsignificant = true;
	IsFootprint = false;

	OverlayTypes.Add(this);
	HeapID = OverlayType(OverlayTypes.ID(this));
}


/// <summary>
/// Destroys this overlay type object.
/// Any artwork that was demand loaded is released, anything still pointing at this type is
/// detached from it, and the type is dropped from the overlay type heap.
/// </summary>
OverlayTypeClass::~OverlayTypeClass(void)
{
	if (DemandLoad && ImageData != NULL) {
		Free_Demand_Loaded_Shape(ImageData);
	}
	Detach_This_From_All(this, true);
	OverlayTypes.Delete(this);
}


/***********************************************************************************************
 * OverlayTypeClass::From_Name -- Determine overlay from ASCII name.                           *
 *                                                                                             *
 *    This routine is used to determine the overlay number given only                          *
 *    an ASCII representation. The scenario loader uses this routine                           *
 *    to construct the map from the INI control file.                                          *
 *                                                                                             *
 * INPUT:   name  -- Pointer to the ASCII name of the overlay.                                 *
 *                                                                                             *
 * OUTPUT:  Returns with the overlay number. If the name had no match,                         *
 *          then returns with OVERLAY_NONE.                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
OverlayType OverlayTypeClass::From_Name(char const * name)
{
	if (name != NULL) {
		for (int index = 0; index < OverlayTypes.Count(); index++) {
			if (stricmp(OverlayTypes[index]->IniName, name) == 0) {
				return(OverlayType(index));
			}
		}
	}
	return(OVERLAY_NONE);
}


/***********************************************************************************************
 * OverlayTypeClass::Occupy_List -- Determines occupation list.                                *
 *                                                                                             *
 *    This routine is used to examine the overlay map and build an                             *
 *    occupation list. This list is used to render a overlay cursor as                         *
 *    well as placement of icon numbers.                                                       *
 *                                                                                             *
 * INPUT:   placement   -- Is this for placement legality checking only? The normal condition  *
 *                         is for marking occupation flags.                                    *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the overlay occupation list.                             *
 *                                                                                             *
 * WARNINGS:   The return pointer is valid only until the next time that                       *
 *             this routine is called.                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
Cell const * OverlayTypeClass::Occupy_List(bool) const
{
	static Cell const _simple[] = {Cell(0, 0), REFRESH_EOL};

	return(_simple);
}


/***********************************************************************************************
 * OverlayTypeClass::Create_And_Place -- Creates and places a overlay object on the map.       *
 *                                                                                             *
 *    This support routine is used by the scenario editor to add a overlay object to the map   *
 *    and to the game.                                                                         *
 *                                                                                             *
 * INPUT:   cell  -- The cell to place the overlay object.                                     *
 *                                                                                             *
 * OUTPUT:  bool; Was the overlay object placed successfully?                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool OverlayTypeClass::Create_And_Place(Cell const & cell, HouseClass * ) const
{
	if (new OverlayClass((OverlayTypeClass *)this, cell)) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * OverlayTypeClass::Create_One_Of -- Creates an object of this overlay type.                  *
 *                                                                                             *
 *    This routine will create an object of this type. For certain overlay objects, such       *
 *    as walls, it is actually created as a building. The "building" wall is converted into    *
 *    a overlay at the moment of placing down on the map.                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the appropriate object for this overlay type.            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectClass * OverlayTypeClass::Create_One_Of(HouseClass *) const
{
	return(new OverlayClass((OverlayTypeClass *)this, CELL_NONE));
}


/***********************************************************************************************
 * OverlayTypeClass::Draw_It -- Draws the overlay image at location specified.                 *
 *                                                                                             *
 *    This routine will draw the overlay shape at the coordinates specified. It is presumed    *
 *    that all the underlying layers have already been rendered by the time this routine is    *
 *    called.                                                                                  *
 *                                                                                             *
 * INPUT:   x, y  -- Coordinate (upper left) of cell where overlay image is to be drawn.       *
 *                                                                                             *
 *          data  -- Cell specific data that controls the imagery of the overlay.              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/12/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void OverlayTypeClass::Draw_It(Point2D const & point, Rect const & cliprect, int data) const
{
	Draw_Shape(*LogicalSurface, *TerrainDrawer, (ShapeSet const *)Get_Image_Data(), data, TacticalRect.Top_Left() + point + Point2D(ISO_TILE_PIXEL_W / 2, ISO_TILE_PIXEL_H / 2), cliprect, (ShapeFlags_Type)(SHAPE_CENTER|SHAPE_WIN_REL));
}


/***********************************************************************************************
 * OverlayTypeClass::Init -- Initialize the overlay graphic data per theater.                  *
 *                                                                                             *
 *    This routine will update the overlay graphic data according to the theater specified.    *
 *    It is typically called when the scenario is first loaded (theater change).               *
 *                                                                                             *
 * INPUT:   theater  -- The theater to load specific data for.                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/01/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void OverlayTypeClass::Init(TheaterType theater)
{
	for (int index = 0; index < OverlayTypes.Count(); index++) {
		OverlayTypeClass & overlay = *OverlayTypes[index];

		char fullname[_MAX_FNAME+_MAX_EXT];
		if (!overlay.DemandLoad) {
			if (overlay.IsTheater) {
				_makepath(fullname, NULL, NULL, overlay.GraphicName, Theaters[theater].Suffix);
				overlay.ImageData = MFCD::Retrieve(fullname);

			} else if (overlay.IsNewTheater) {
				_makepath(fullname, NULL, NULL, overlay.GraphicName, ".SHP");
				overlay.Theater_Naming_Convention(fullname, theater);
				overlay.ImageData = MFCD::Retrieve(fullname);
			}
		} else {
			if (overlay.IsTheater || overlay.IsNewTheater) {
				if (overlay.ImageData != NULL) {
					Free_Demand_Loaded_Shape(overlay.ImageData);
				}
			}
		}
	}
}


/// <summary>
/// Fetches this overlay type's settings from the rules.
/// This routine picks up what the overlay is made of and how it behaves -- whether it is a
/// wall, tiberium, a crate, or something that explodes when shot -- and it loads the artwork
/// for the type unless the art has been left for demand loading.
/// </summary>
/// <returns>bool; Was an entry for this overlay type found and read?</returns>
bool OverlayTypeClass::Read_INI(CCINIClass const & ini)
{
	char fullname[_MAX_FNAME+_MAX_EXT];

	if (!ini.Section_Present(IniName)) {
		return(false);
	}

	if (DemandLoad && ImageData != NULL) {
		Free_Demand_Loaded_Shape(ImageData);
	}

	if (BASECLASS::Read_INI(ini)) {
		Land = ini.Get_LandType(IniName, "Land", Land);
		DamagePoints = ini.Get_Int(IniName, "Strength", DamagePoints);
		IsWall = ini.Get_Bool(IniName, "Wall", IsWall);
		IsHigh = ini.Get_Bool(IniName, "High", IsHigh);
		IsTiberium = ini.Get_Bool(IniName, "Tiberium", IsTiberium);
		IsCrate = ini.Get_Bool(IniName, "Crate", IsCrate);
		IsCrateTrigger = ini.Get_Bool(IniName, "CrateTrigger", IsCrateTrigger);
		IsExplosive = ini.Get_Bool(IniName, "Explodes", IsExplosive);
		IsOverrides = ini.Get_Bool(IniName, "Overrides", IsOverrides);

		CellAnim = TGet_Class(ini, IniName, "CellAnim", CellAnim);

		DamageLevels = ArtINI.Get_Int(GraphicName, "DamageLevels", DamageLevels);
		DemandLoad = ArtINI.Get_Bool(GraphicName, "DemandLoad", DemandLoad);
		if (DemandLoad) {
			ImageData = NULL;
		}

		if (IsTiberium) {
			Armor = ARMOR_WOOD;
			if (Land == LAND_CLEAR) {
				Land = LAND_TIBERIUM;
			}
		}

		if (!IsTheater && !DemandLoad) {
			_makepath(fullname, NULL, NULL, GraphicName, ".SHP");
			ImageData = MFCD::Retrieve(fullname);
		}

		IsNoUseTileLandType = ini.Get_Bool(IniName, "NoUseTileLandType", IsNoUseTileLandType);
		IsVeinholeMonster = ini.Get_Bool(IniName, "IsVeinholeMonster", IsVeinholeMonster);
		IsVeins = ini.Get_Bool(IniName, "IsVeins", IsVeins);
		IsChainReaction = ini.Get_Bool(IniName, "ChainReaction", IsChainReaction);
		IsDrawFlat = ini.Get_Bool(IniName, "DrawFlat", IsDrawFlat);
		IsARock = ini.Get_Bool(IniName, "IsARock", IsARock);

		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * OverlayTypeClass::Coord_Fixup -- Adjust the coord to be legal for assignment.               *
 *                                                                                             *
 *    This will adjust the coordinate specified so that it will be of legal format to          *
 *    assign as the coordinate of an overlay. Overlays are always relative to the upper left   *
 *    corner of the cell, so this routine drops the fractional cell components.                *
 *                                                                                             *
 * INPUT:   coord -- The coordinate to fixup to be legal for assignment.                       *
 *                                                                                             *
 * OUTPUT:  Returns with a properly fixed up coordinate.                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/14/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
Coord const OverlayTypeClass::Coord_Fixup(Coord const & coord) const
{
	return(coord);
}


/// <summary>
/// Adds this overlay type to the game state checksum.
/// The network sync check uses this to prove that every machine in the game is working
/// from the same rules.
/// </summary>
void OverlayTypeClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);

	crc(HeapID);
	crc(Land);
	crc(DamageLevels);
	crc(DamagePoints);
	crc(IsWall);
	crc(IsHigh);
	crc(IsTiberium);
	crc(IsCrate);
	crc(IsCrateTrigger);
	crc(IsExplosive);
}


/// <summary>
/// Re-attaches the artwork this overlay type names.
/// The artwork is fetched again once the members have been read, since pointers into the
/// mix files do not survive a save. An overlay left for demand loading is not fetched here;
/// Get_Image_Data picks it up when something first asks to draw it.
/// </summary>
void OverlayTypeClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	Fetch_Voxel_Image();

	if (!DemandLoad) {
		Fetch_Normal_Image();
		char fullname[_MAX_FNAME+_MAX_EXT];
		if (IsTheater) {
			_makepath(fullname, NULL, NULL, GraphicName, Theaters[Scen->Theater].Suffix);
		} else {
			_makepath(fullname, NULL, NULL, GraphicName, ".SHP");
			Theater_Naming_Convention(fullname, Scen->Theater);
		}

		ImageData = MFCD::Retrieve(fullname);
	}
}


/// <summary>
/// Lists the members this overlay type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void OverlayTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HeapID);
	stream.Serialize(Land);
	stream.Serialize(CellAnim);
	stream.Serialize(DamageLevels);
	stream.Serialize(DamagePoints);
	stream.Serialize(IsWall);
	stream.Serialize(IsHigh);
	stream.Serialize(IsTiberium);
	stream.Serialize(IsCrate);
	stream.Serialize(IsCrateTrigger);
	stream.Serialize(IsNoUseTileLandType);
	stream.Serialize(IsVeinholeMonster);
	stream.Serialize(IsVeins);
	stream.Serialize(DemandLoad);
	stream.Serialize(IsExplosive);
	stream.Serialize(IsChainReaction);
	stream.Serialize(IsOverrides);
	stream.Serialize(IsDrawFlat);
	stream.Serialize(IsARock);
}


/// <summary>
/// Fetches the class identifier of this object.
/// The save system asks for this so that it knows which class to construct when the object
/// is read back out of a save file.
/// </summary>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT STDMETHODCALLTYPE OverlayTypeClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_OverlayTypeClass;
	return(S_OK);
}


/// <summary>
/// Fetches the overlay type of the specified name.
/// This routine is used while the rules are being read, where a type may be referred to
/// before its own section has been reached.
/// </summary>
/// <returns>Returns with a pointer to the overlay type, which is created and added to the
/// heap if no type of that name exists yet.</returns>
OverlayTypeClass * OverlayTypeClass::Find_Or_Make(char const * name)
{
	return(TFind_Or_Make<OverlayTypeClass>(name, OverlayTypes));
}


/***************************************************************************
 * OverlayTypeClass::Radar_Icon -- Gets a pointer to the radar icons       *
 *                                                                         *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   04/19/1995 PWG : Created.                                             *
 *=========================================================================*/
RGBClass OverlayTypeClass::Get_Radar_Color(int shape) const
{
	ShapeSet const *image = (const ShapeSet *) Get_Image_Data();

	if (image == NULL && CellAnim != NULL) {
		image = (const ShapeSet *)CellAnim->Get_Image_Data();
	}

	if (image != NULL) {
		if ((HeapID >= OVERLAY_TIBERIUM2_01 && HeapID <= OVERLAY_TIBERIUM2_12) ||
			(HeapID >= OVERLAY_TIBERIUM3_01 && HeapID <= OVERLAY_TIBERIUM3_12)) {

			RGBClass color = image->Get_Color(shape);
			/// Tiberium's art is red
			return(RGBClass (color.Get_Red(), color.Get_Blue(), color.Get_Green()));
		}
		return(image->Get_Color(shape));
	}

	return(RGBClass(0, 0, 0));
}


/// <summary>
/// Fetches the shape data to draw this overlay type with.
/// An overlay type marked for demand loading does without its artwork until something first
/// asks to draw it. This routine fetches the shape file at that moment and holds on to it
/// for every later request.
/// </summary>
/// <returns>Returns with a pointer to the shape data for this overlay type.</returns>
void const * OverlayTypeClass::Get_Image_Data(void) const
{
	char fullname[_MAX_FNAME+_MAX_EXT];

	if (ImageData != NULL || DemandLoad == false) {
		return(ImageData);
	}

	OverlayTypeClass * _this = (OverlayTypeClass *)this;

	DebugString("Demand loading image for %s\n", (char const *)GivenName);
	if (IsTheater) {
		_makepath(fullname, NULL, NULL, GraphicName, Theaters[Scen->Theater].Suffix);
	} else {
		_makepath(fullname, NULL, NULL, GraphicName, ".SHP");
		if (IsNewTheater) {
			_this->Theater_Naming_Convention( fullname, Scen->Theater);
		}
	}

	CCFileClass file (fullname);

	_this->ImageData = Load_Alloc_Data(file);

	return(ImageData);
}
