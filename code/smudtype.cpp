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

/* $Header: /CounterStrike/SDATA.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SDATA.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : August 9, 1994                                               *
 *                                                                                             *
 *                  Last Update : July 9, 1996 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   SmudgeTypeClass::As_Reference -- Fetches a reference to the smudge type specified.        *
 *   SmudgeTypeClass::Create_And_Place -- Creates and places on map, a smudge object.          *
 *   SmudgeTypeClass::Create_One_Of -- Creates a smudge object of this type.                   *
 *   SmudgeTypeClass::Display -- Draws a generic version of this smudge type.                  *
 *   SmudgeTypeClass::Draw_It -- Renders the smudge image at the coordinate specified.         *
 *   SmudgeTypeClass::From_Name -- Converts an ASCII name into a smudge type.                  *
 *   SmudgeTypeClass::Init -- Performs theater specific initializations.                       *
 *   SmudgeTypeClass::Init_Heap -- Initialize the smudge type class object heap.               *
 *   SmudgeTypeClass::One_Time -- Performs one-time initialization                             *
 *   SmudgeTypeClass::Prep_For_Add -- Prepares the scenario editor for adding a smudge object. *
 *   SmudgeTypeClass::SmudgeTypeClass -- Constructor for smudge type objects.                  *
 *   SmudgeTypeClass::operator delete -- Returns a smudge type class object to the pool.       *
 *   SmudgeTypeClass::operator new -- Allocate a smudge type object from the memory pool.      *
 *   SmudgetypeClass::Occupy_List -- Determines occupation list for smudge object.             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "smudtype.h"

#include "_map.h"
#include "_mixfile.h"
#include "_surface.h"
#include "_tactica.h"
#include "_theater.h"
#include "ccini.h"
#include "ccrand.h"
#include "cell.h"
#include "draw.h"
#include "findmake.h"
#include "globals.h"
#include "isotype.h"
#include "lightcon.h"
#include "mixfile.h"
#include "mouse.h"
#include "savestream.h"
#include "scenario.h"
#include "smudge.h"
#include "sun.h"
#include "tactical.h"
#include "tracker.h"


/***********************************************************************************************
 * SmudgeTypeClass::SmudgeTypeClass -- Constructor for smudge type objects.                    *
 *                                                                                             *
 *    This constructor is used to create the smudge type objects. These type objects contain   *
 *    static information about the various smudge types supported in the game.                 *
 *                                                                                             *
 * INPUT:   see below...                                                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/12/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
SmudgeTypeClass::SmudgeTypeClass(char const * ini) :
	BASECLASS(ini),
	HeapID(SMUDGE_NONE),
	Width(1),
	Height(1),
	IsCrater(false),
	IsScorch(false)
{
	Create_ID();

	IsStealthy = true;
	IsSelectable = false;
	IsLegalTarget = false;
	IsInsignificant = true;
	IsImmune = true;
	IsFootprint = false;

	SmudgeTypes.Add(this);
	HeapID = (SmudgeType)SmudgeTypes.ID(this);
}


/// <summary>
/// Destroys this smudge type object.
/// Anything still pointing at this type is detached from it before the type is dropped from
/// the smudge type heap.
/// </summary>
SmudgeTypeClass::~SmudgeTypeClass(void)
{
	Detach_This_From_All(this, true);
	SmudgeTypes.Delete(this);
}


/***********************************************************************************************
 * SmudgeTypeClass::From_Name -- Converts an ASCII name into a smudge type.                    *
 *                                                                                             *
 *    This converts an ASCII name into a smudge type number. This is typically necessary       *
 *    when processing scenario INI files and not used otherwise.                               *
 *                                                                                             *
 * INPUT:   name  -- Pointer to the name to convert.                                           *
 *                                                                                             *
 * OUTPUT:  Returns with the SmudgeType number that matches the name supplied. If no match     *
 *          was found, then SMUDGE_NONE is returned.                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/12/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
SmudgeType SmudgeTypeClass::From_Name(char const * name)
{
	if (name != NULL) {
		for (int index = SMUDGE_FIRST; index < SmudgeTypes.Count(); index++) {
			if (stricmp(SmudgeTypes[index]->Name(), name) == 0) {
				return(SmudgeType(index));
			}
		}
	}
	return(SMUDGE_NONE);
}


/***********************************************************************************************
 * SmudgeTypeClass::Init -- Performs theater specific initializations.                         *
 *                                                                                             *
 *    Smudge object imagery varies between theaters. This routine will load the appropriate    *
 *    imagery for the theater specified.                                                       *
 *                                                                                             *
 * INPUT:   theater  -- The theater to prepare for.                                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/12/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void SmudgeTypeClass::Init(TheaterType theater)
{
	if (Debug_Map || theater != LastTheater) {
		for (int index = SMUDGE_FIRST; index < SmudgeTypes.Count(); index++) {
			SmudgeTypeClass * smudge = SmudgeTypes[index];
			char fullname[_MAX_FNAME+_MAX_EXT];	// Fully constructed smudge data set name.
			if (smudge->IsTheater) {
				_makepath(fullname, NULL, NULL, smudge->Name(), TheaterClass::As_Reference(theater).Suffix);
				smudge->ImageData = (ShapeSet const *)MFCD::Retrieve(fullname);
			}
		}
	}
}


/***********************************************************************************************
 * SmudgeTypeClass::Create_And_Place -- Creates and places on map, a smudge object.            *
 *                                                                                             *
 *    This routine will, in one motion, create a smudge object and place it upon the map.      *
 *    Since placing a smudge on the map will destroy the object, this routine will leave the   *
 *    smudge object count unchanged. Typically, this routine is used by the scenario editor    *
 *    for creating smudges and placing them on the map.                                        *
 *                                                                                             *
 * INPUT:   cell  -- The cell to place the smudge object.                                      *
 *                                                                                             *
 * OUTPUT:  bool; Was the placement successful?                                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/12/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SmudgeTypeClass::Create_And_Place(Cell const & cell, HouseClass * house) const
{
	if (new SmudgeClass(this, cell)) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * SmudgeTypeClass::Create_One_Of -- Creates a smudge object of this type.                     *
 *                                                                                             *
 *    This routine will create a smudge object of the appropriate type. Smudge objects are     *
 *    transitory in nature. They exist only from the point of creation until they are given    *
 *    a spot on the map to reside. At that time the map data is updated and the smudge         *
 *    object is destroyed.                                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to a created smudge object. If none could be created, then  *
 *          NULL is returned.                                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/12/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectClass * SmudgeTypeClass::Create_One_Of(HouseClass *) const
{
	return(new SmudgeClass(this));
}


/***********************************************************************************************
 * SmudgeTypeClass::Draw_It -- Renders the smudge image at the coordinate specified.           *
 *                                                                                             *
 *    This routine will draw the smudge overlay image at the coordinate (upper left)           *
 *    specified. The underlying terrain icon is presumed to have already been rendered.        *
 *                                                                                             *
 * INPUT:   x,y   -- Coordinate of the upper left corner of icon to render the smudge object.  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/12/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void SmudgeTypeClass::Draw_It(Point2D const & point, Rect const & cliprect, int size, int z, Cell const & cell) const
{
	ShapeSet const * ptr = (ShapeSet const *)Get_Image_Data();
	if (ptr != NULL) {
		Point2D drawpoint = point;
		if (size != 0) {
			int width = size % Width;
			int height = size / Width;
			drawpoint.Y -= LEVEL_PIXEL_H * (height + width);
			drawpoint.X -= ISO_TILE_PIXEL_W * (width - height) / 2;
		}
		CellClass * cellptr = &Map[cell];
		int brightness = cellptr->TileBrightness;
		Draw_Shape(*LogicalSurface, *cellptr->Drawer, ptr, 0, drawpoint, cliprect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA), NULL, -1 - TacticalMap->Z_Lepton_To_Pixel(z), ZGRAD_GROUND, brightness);
	}
}


/// <summary>
/// Fetches this smudge type's settings from the rules.
/// This routine picks up the size of the smudge and whether it counts as a crater or a
/// scorch, and it loads the artwork for the type as well.
/// </summary>
/// <returns>bool; Was an entry for this smudge type found and read?</returns>
bool SmudgeTypeClass::Read_INI(CCINIClass const & ini)
{
	if (BASECLASS::Read_INI(ini)) {
		IsCrater = ini.Get_Bool(Name(), "Crater", IsCrater);
		IsScorch = ini.Get_Bool(Name(), "Burn", IsScorch);
		Width = ini.Get_Int(Name(), "Width", Width);
		Height = ini.Get_Int(Name(), "Height", Height);

		char fullname[_MAX_FNAME+_MAX_EXT];
		if (!IsTheater) {
			_makepath(fullname, NULL, NULL, (char const *)Graphic_Name(), ".SHP");
			ImageData = (ShapeSet const *)MFCD::Retrieve(fullname);
		} else {
			_makepath(fullname, NULL, NULL, (char const *)Graphic_Name(), TheaterClass::As_Reference(Scen->Theater).Suffix);
			ImageData = (ShapeSet const *)MFCD::Retrieve(fullname);
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Adds this smudge type to the game state checksum.
/// The network sync check uses this to prove that every machine in the game is working
/// from the same rules.
/// </summary>
void SmudgeTypeClass::Compute_CRC(CRCEngine &crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(HeapID);
	crc(IsCrater);
	crc(IsScorch);
	crc(Width);
	crc(Height);
}


/// <summary>
/// Re-attaches the artwork this smudge type names.
/// The artwork is fetched again once the members have been read, since pointers into the
/// mix files do not survive a save.
/// </summary>
void SmudgeTypeClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	Fetch_Voxel_Image();
	Fetch_Normal_Image();
}


/// <summary>
/// Lists the members this smudge type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void SmudgeTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HeapID);
	stream.Serialize(Width);
	stream.Serialize(Height);
	stream.Serialize(IsCrater);
	stream.Serialize(IsScorch);
}


/// <summary>
/// Fetches the class identifier of this object.
/// The save system asks for this so that it knows which class to construct when the object
/// is read back out of a save file.
/// </summary>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT SmudgeTypeClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_SmudgeTypeClass;
	return(S_OK);
}


/// <summary>
/// Fetches the smudge type of the specified name.
/// This routine is used while the rules are being read, where a type may be referred to
/// before its own section has been reached.
/// </summary>
/// <returns>Returns with a pointer to the smudge type, which is created and added to the
/// heap if no type of that name exists yet.</returns>
SmudgeTypeClass * SmudgeTypeClass::Find_Or_Make(const char *name)
{
	return(TFind_Or_Make<SmudgeTypeClass>(name, SmudgeTypes));
}


/// <summary>
/// Scorches the ground at the specified location.
/// This routine is called when fire or an explosion should leave a burn mark on the
/// terrain. A scorch that suits the location and the size of the blast is picked at random
/// from the scorch types the rules declare.
/// </summary>
/// <param name="width">The width of the blast, in pixels.</param>
/// <param name="height">The height of the blast, in pixels.</param>
/// <param name="large">Should only the multiple cell scorches be considered?</param>
bool SmudgeTypeClass::Scorch_The_Ground(Coord const & coord, int width, int height, bool large)
{
	int i;

	if (coord.As_Cell() == CELL_NONE) {
		return(false);
	}

	DynamicVectorClass<SmudgeTypeClass const *> list;
	for (i = 0; i < SmudgeTypes.Count(); i++) {
		SmudgeTypeClass const * smudge = SmudgeTypes[i];
		if (smudge->IsScorch) {
			if (smudge->Can_Place_Here(coord.As_Cell(), large)) {
				list.Add(smudge);
			}
		}
	}

	DynamicVectorClass<SmudgeTypeClass const *> largelist;
	for (i = 0; i < list.Count(); i++) {
		SmudgeTypeClass const * scorch = list[i];
		if (!large) {
			if (scorch->Width == 1 && scorch->Height == 1 || width > CELL_PIXEL_H && height > (CELL_PIXEL_W * 5 / 3)) {
				largelist.Add(scorch);
			}
		} else {
			if (scorch->Width > 1 && scorch->Height > 1) {
				largelist.Add(scorch);
			}
		}
	}

	if (largelist.Count() > 0) {
		new SmudgeClass(largelist[Random_Pick(0, largelist.Count() - 1)], coord);
	} else if (list.Count() > 0) {
		new SmudgeClass(list[Random_Pick(0, list.Count() - 1)], coord);
	}

	return(false);
}


/// <summary>
/// Craters the ground at the specified location.
/// This routine is called when an explosion should leave a permanent mark on the terrain.
/// A crater that suits the location and the size of the blast is picked at random from the
/// crater types the rules declare.
/// </summary>
/// <param name="width">The width of the blast, in pixels.</param>
/// <param name="height">The height of the blast, in pixels.</param>
/// <param name="large">Should only the multiple cell craters be considered?</param>
bool SmudgeTypeClass::Crater_The_Ground(Coord const & coord, int width, int height, bool large)
{
	int i;

	if (coord.As_Cell() == CELL_NONE) {
		return(false);
	}

	DynamicVectorClass<SmudgeTypeClass const *> list;
	for (i = 0; i < SmudgeTypes.Count(); i++) {
		SmudgeTypeClass const * smudge = SmudgeTypes[i];
		if (smudge->IsCrater) {
			if (smudge->Can_Place_Here(coord.As_Cell(), large)) {
				list.Add(smudge);
			}
		}
	}

	DynamicVectorClass<SmudgeTypeClass const *> largelist;
	for (i = 0; i < list.Count(); i++) {
		SmudgeTypeClass const * scorch = list[i];
		if (!large) {
			if (scorch->Width == 1 && scorch->Height == 1 || width > CELL_PIXEL_H && height > (CELL_PIXEL_W * 5 / 3)) {
				largelist.Add(scorch);
			}
		} else {
			if (scorch->Width > 1 && scorch->Height > 1) {
				largelist.Add(scorch);
			}
		}
	}

	if (largelist.Count() > 0) {
		new SmudgeClass(largelist[Random_Pick(0, largelist.Count() - 1)], coord);
	} else if (list.Count() > 0) {
		new SmudgeClass(list[Random_Pick(0, list.Count() - 1)], coord);
	}

	return(false);
}


/// <summary>
/// Can this smudge type be laid down at the specified location?
/// Every cell the smudge would cover must be flat unmarked ground of a kind that will take
/// a smudge, though the radar bounds are tested against the origin cell alone. This routine
/// is used to filter the candidate smudges before one is picked.
/// </summary>
/// <param name="origin">The upper left cell of the area the smudge would cover.</param>
/// <param name="underbuildings">Should a cell that a building sits on still count as
/// clear?</param>
/// <returns>bool; Is the location suitable for this smudge?</returns>
bool SmudgeTypeClass::Can_Place_Here(Cell const & origin, bool underbuildings) const
{
	for (int h = 0; h < Height; h++) {
		for (int w = 0; w < Width; w++) {
			CellClass * cell = &Map[origin + Cell(w, h)];
			if (!Map.In_Radar(origin)) {
				return(false);
			}
			if (cell->Ramp != 0) {
				return(false);
			}
			if (cell->Smudge != SMUDGE_NONE) {
				return(false);
			}
			if (cell->Overlay != OVERLAY_NONE) {
				return(false);
			}
			if (!underbuildings && cell->Cell_Building() != NULL) {
				return(false);
			}
			IsometricTileType ittype = cell->ITType;
			if (cell->ITType < ISOTILE_FIRST || cell->ITType >= IsometricTileTypes.Count()) {
				ittype = ISOTILE_FIRST;
			}
			if (!IsometricTileTypes[ittype]->IsMorphable) {
				return(false);
			}
		}
	}
	return(true);
}


/// <summary>
/// Stamps this smudge type onto the map.
/// This routine lays the smudge across the whole area it covers and flags the affected
/// cells so that anything overlapping them is redrawn.
/// </summary>
/// <param name="origin">The upper left cell of the area the smudge will cover.</param>
/// <remarks>Check the location with Can_Place_Here first -- this routine overwrites
/// whatever it finds.</remarks>
void SmudgeTypeClass::Place(Cell const & origin) const
{
	for (int h = 0; h < Height; h++) {
		for (int w = 0; w < Width; w++) {
			Cell newcell = origin + Cell(w, h);
			CellClass * cell = &Map[newcell];
			cell->Smudge = HeapID;
			cell->SmudgeData = w + (h*Width);

			/*
			**	Flag everything that might be overlapping this cell to redraw itself.
			*/
			cell->Register_For_Redraw();
		}
	}
}
