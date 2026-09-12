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

/* $Header: /CounterStrike/TDATA.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TDATA.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 2, 1994                                                  *
 *                                                                                             *
 *                  Last Update : July 19, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   TerrainTypeClass::As_Reference -- Fetches a reference to the terrain type object specified*
 *   TerrainTypeClass::Create_And_Place -- Creates and places terrain object on map.           *
 *   TerrainTypeClass::Create_On_Of -- Creates a terrain object from type.                     *
 *   TerrainTypeClass::Display -- Display a generic terrain object.                            *
 *   TerrainTypeClass::From_Name -- Convert name to terrain type.                              *
 *   TerrainTypeClass::Init -- Loads terrain object shape files.                               *
 *   TerrainTypeClass::Init_Heap -- Initialize the terrain object heap.                        *
 *   TerrainTypeClass::Occupy_List -- Returns with the occupy list for the terrain object type.*
 *   TerrainTypeClass::One_Time -- Performs any special one time processing for terrain types. *
 *   TerrainTypeClass::Overlap_List -- Fetches the overlap list for the terrain type.          *
 *   TerrainTypeClass::Prep_For_Add -- Prepares to add terrain object.                         *
 *   TerrainTypeClass::TerrainTypeClass -- The general constructor for the terrain type objects*
 *   TerrainTypeClass::operator delete -- Returns a terrain type object back to the mem pool.  *
 *   TerrainTypeClass::operator new -- Allocates a terrain type object from special pool.      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "terrtype.h"

#include "_map.h"
#include "_mixfile.h"
#include "_rules.h"
#include "_theater.h"
#include "findmake.h"
#include "globals.h"
#include "incdec.h"
#include "mixfile.h"
#include "rules.h"
#include "savestream.h"
#include "shapeset.h"
#include "sun.h"
#include "terrain.h"
#include "terrtype.h"
#include "tracker.h"

static Cell const _OccupyLists[BSIZE_COUNT][10] = {
	/* BSIZE_11,	*/	{ Cell(0, 0), REFRESH_EOL },
	/* BSIZE_21,	*/	{ Cell(0, 0), Cell(1, 0), REFRESH_EOL },
	/* BSIZE_12,	*/	{ Cell(0, 0), Cell(0, 1), REFRESH_EOL },
	/* BSIZE_22,	*/	{ Cell(0, 0), Cell(1, 0), Cell(0, 1), Cell(1, 1), REFRESH_EOL },
	/* BSIZE_23,	*/	{ Cell(0, 0), Cell(1, 0), Cell(0, 1), Cell(1, 1), Cell(0, 2), Cell(1, 2), REFRESH_EOL },
	/* BSIZE_32,	*/	{ Cell(0, 0), Cell(1, 0), Cell(2, 0), Cell(0, 1), Cell(1, 1), Cell(2, 1), REFRESH_EOL },
	/* BSIZE_33,	*/	{ Cell(0, 0), Cell(1, 0), Cell(2, 0), Cell(0, 1), Cell(1, 1), Cell(2, 1), Cell(0, 2), Cell(1, 2), Cell(2, 2), REFRESH_EOL },
	/* BSIZE_35,	*/	{ Cell(0, 0), Cell(1, 0), Cell(2, 0), Cell(3, 0), Cell(0, 1), Cell(1, 1), Cell(2, 1), Cell(3, 1), REFRESH_EOL },
	/* BSIZE_42,	*/	/// NOT SET
	/* BSIZE_33_REF	*/	/// NOT SET
	/* BSIZE_13,	*/	/// NOT SET
	/* BSIZE_31,	*/	/// NOT SET
	/* BSIZE_43,	*/	/// NOT SET
	/* BSIZE_14,	*/	/// NOT SET
	/* BSIZE_15,	*/	/// NOT SET
	/* BSIZE_26,	*/	/// NOT SET
	/* BSIZE_25,	*/	/// NOT SET
	/* BSIZE_53,	*/	/// NOT SET
	/* BSIZE_44,	*/	/// NOT SET
	/* BSIZE_34,	*/	/// NOT SET
	/* BSIZE_64,	*/	/// NOT SET
	/* BSIZE_00,	*/	/// NOT SET
};


/***********************************************************************************************
 * TerrainTypeClass::TerrainTypeClass -- The general constructor for the terrain type objects. *
 *                                                                                             *
 *    This is the constructor for terrain type objects. It is only used to construct the       *
 *    static (constant) terrain type objects.                                                  *
 *                                                                                             *
 * INPUT:   see below..                                                                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
TerrainTypeClass::TerrainTypeClass(char const * ininame) :
	BASECLASS(ininame),
	HeapID(TERRAIN_NONE),
	Foundation(BSIZE_FIRST),
	RadarColor(0,0,0),
	TemperateOccupationBits(7),
	SnowOccupationBits(7),
	AnimationRate(0),
	AnimationProbability(0),
	YDrawFudge(0),
	TiberiumToSpawn(0),
	IsWaterBased(false),
	IsTiberiumSpawn(false),
	IsFlammable(false),
	IsAnimated(false),
	IsVeinhole(false),
	Occupy(NULL)

{
	Create_ID();
	IsSentient = true;
	IsStealthy = true;
	IsSelectable = false;
	IsLegalTarget = false;
	IsInsignificant = true;

	TerrainTypes.Add(this);

	HeapID = (TerrainType)TerrainTypes.ID(this);

	MaxStrength = -1;
	Armor = ARMOR_WOOD;
}


/// <summary>
/// Removes this terrain type from the game.
/// This routine will detach the type from everything that refers to it before it
/// drops out of the terrain type heap.
/// </summary>
TerrainTypeClass::~TerrainTypeClass(void)
{
	Detach_This_From_All(this, true);
	TerrainTypes.Delete(this);
}


/***********************************************************************************************
 * TerrainTypeClass::Init -- Loads terrain object shape files.                                 *
 *                                                                                             *
 *    This routine is used to load up the terrain object shape files.                          *
 *    The shape files loaded depends on theater.                                               *
 *                                                                                             *
 * INPUT:   theater  -- The theater to load the terrain shape data for.                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/16/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TerrainTypeClass::Init(TheaterType theater)
{
	if (Debug_Map || theater != LastTheater) {

		for (TerrainType index = TERRAIN_FIRST; index < TerrainTypes.Count(); index++) {
			TerrainTypeClass * terrain = TerrainTypes[index];
			char	fullname[_MAX_FNAME+_MAX_EXT];

			if (terrain->IsTheater) {

				/*
				 * Clear any existing shape pointer.
				 */
				terrain->ImageData = NULL;

				/*
				**	Load in the appropriate object shape data.
				*/
				_makepath(fullname, NULL, NULL, terrain->Name(), TheaterClass::As_Reference(theater).Suffix);
				terrain->ImageData = (ShapeSet const *)MFCD::Retrieve(fullname);

			}
		}
	}
}


/***********************************************************************************************
 * TerrainTypeClass::From_Name -- Convert name to terrain type.                                *
 *                                                                                             *
 *    This routine is used to convert a text name into the matching                            *
 *    terrain type number. This is used during scenario initialization.                        *
 *                                                                                             *
 * INPUT:   name  -- The name to convert.                                                      *
 *                                                                                             *
 * OUTPUT:  Returns the TerrainType that matches the name specified. If                        *
 *          no match was found, then TERRAIN_NONE is returned.                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/16/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
TerrainType TerrainTypeClass::From_Name(char const * name)
{
	TerrainType	index;

	if (name != NULL) {
		for (index = TERRAIN_FIRST; index < TerrainTypes.Count(); index++) {
			if (stricmp(name, TerrainTypes[index]->Name()) == 0) {
				return(index);
			}
		}
	}
	return(TERRAIN_NONE);
}


/***********************************************************************************************
 * TerrainTypeClass::Create_And_Place -- Creates and places terrain object on map.             *
 *                                                                                             *
 *    This support routine is used by the scenario editor to add a terrain object to the map.  *
 *                                                                                             *
 * INPUT:   cell  -- The cell to place the terrain object in.                                  *
 *                                                                                             *
 * OUTPUT:  bool; Was the placement successful?                                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TerrainTypeClass::Create_And_Place(Cell const & cell, HouseClass *) const
{
	if (new TerrainClass(this, cell)) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * TerrainTypeClass::Create_One_Of -- Creates a terrain object from type.                      *
 *                                                                                             *
 *    This is used to create a terrain object by using the terrain type as a guide. This       *
 *    routine is typically used by the scenario editor in order to place a terrain object      *
 *    onto the map.                                                                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the created terrain object or NULL if one couldn't be    *
 *          created.                                                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectClass * TerrainTypeClass::Create_One_Of(HouseClass *) const
{
	return(new TerrainClass(this, CELL_NONE));
}


/***********************************************************************************************
 * TerrainTypeClass::Occupy_List -- Returns with the occupy list for the terrain object type.  *
 *                                                                                             *
 *    This routine will return with the occupy list for the terrain object type. If there is   *
 *    no occupy list for this terrain object type, then a special zero length occupy list      *
 *    pointer is returned.                                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the terrain object's occupy list. A zero length list is  *
 *          returned in the case of no occupy list.                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/20/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
Cell const * TerrainTypeClass::Occupy_List(bool ) const
{
	if (Occupy != NULL) return(Occupy);

	static Cell const _simple[1] = {
		REFRESH_EOL
	};
	return(&_simple[0]);
}


/// <summary>
/// Fetches the terrain type data from the INI database specified.
/// This routine reads the rules that make a terrain object what it is -- whether it is a
/// veinhole, whether it must sit on water, whether it burns or spawns tiberium -- along
/// with the artwork, foundation and animation settings it draws with.
/// </summary>
/// <returns>bool; Was the terrain type data read?</returns>
bool TerrainTypeClass::Read_INI(CCINIClass const & ini)
{
	if (BASECLASS::Read_INI(ini)) {

		if (MaxStrength == -1) {
			MaxStrength = Rule->TreeStrength;
		}

		IsVeinhole = ini.Get_Bool(Name(), "IsVeinhole", IsVeinhole);
		if (IsVeinhole) {
			IsSentient = false;
			IsLegalTarget = true;
		}

		IsWaterBased = ini.Get_Bool(Name(), "WaterBound", IsWaterBased);
		IsTiberiumSpawn = ini.Get_Bool(Name(), "SpawnsTiberium", IsTiberiumSpawn);
		IsFlammable = ini.Get_Bool(Name(), "IsFlammable", IsFlammable);

		Foundation = ArtINI.Get_BSizeType(Graphic_Name(), "Foundation", Foundation);
		Occupy = _OccupyLists[Foundation];

		if (!IsTheater) {
			char filename[512];
			_makepath(filename, 0, 0, Graphic_Name(), ".SHP");
			ImageData = (ShapeSet const *)MFCD::Retrieve(filename);
		}

		ShapeSet const * image = (ShapeSet const *)Get_Image_Data();
		if (image) {
			RadarColor = image->Get_Color(0);
		}

		IsAnimated = ini.Get_Bool(Name(), "IsAnimated", IsAnimated);
		AnimationRate = ini.Get_Int(Name(), "AnimationRate", AnimationRate);
		AnimationProbability = ini.Get_Float(Name(), "AnimationProbability", AnimationProbability);

		TemperateOccupationBits = ini.Get_Int(IniName, "TemperateOccupationBits", TemperateOccupationBits);
		SnowOccupationBits = ini.Get_Int(IniName, "SnowOccupationBits", SnowOccupationBits);
		YDrawFudge = ini.Get_Int(IniName, "YDrawFudge", YDrawFudge);
		TiberiumToSpawn = ini.Get_Int(IniName, "TiberiumToSpawn", TiberiumToSpawn);

		return(true);
	}
	return(false);
}


/// <summary>
/// Adjusts the coordinate so that the object will rest on the ground.
/// This routine is used when a terrain object is placed, so that it never ends up
/// buried in the terrain at that location.
/// </summary>
/// <returns>Returns with the coordinate raised to ground level if it was below it.</returns>
Coord const TerrainTypeClass::Coord_Fixup(Coord const & coord) const
{
	Coord tmp = coord;
	if (tmp.Z < Map.Get_Height_GL(tmp)) {
		tmp.Z = Map.Get_Height_GL(tmp);
	}
	return(tmp);
}


/// <summary>
/// Adds this terrain type's data to the CRC engine specified.
/// This routine is used by the multiplayer sync checking logic to prove that every
/// machine in the game loaded the same terrain rules.
/// </summary>
void TerrainTypeClass::Compute_CRC(CRCEngine &crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(HeapID);
	crc(IsWaterBased);
	crc(IsTiberiumSpawn);
	crc(IsFlammable);
	crc(Foundation);
	crc(IsAnimated);
	crc(AnimationRate);
	crc(AnimationProbability);
}


/// <summary>
/// Re-attaches the artwork and occupy list this terrain type names.
/// The artwork is fetched again once the members have been read, and the occupy list is
/// picked back out of the static table that the restored foundation selects.
/// </summary>
void TerrainTypeClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	Fetch_Voxel_Image();
	Fetch_Normal_Image();
	Occupy = _OccupyLists[Foundation];
}


/// <summary>
/// Lists the members this terrain type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TerrainTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HeapID);
	stream.Serialize(Foundation);
	stream.Serialize(RadarColor);
	stream.Serialize(AnimationRate);
	stream.Serialize(AnimationProbability);
	stream.Serialize(YDrawFudge);
	stream.Serialize(TiberiumToSpawn);
	stream.Serialize(TemperateOccupationBits);
	stream.Serialize(SnowOccupationBits);
	stream.Serialize(IsWaterBased);
	stream.Serialize(IsTiberiumSpawn);
	stream.Serialize(IsFlammable);
	stream.Serialize(IsAnimated);
	stream.Serialize(IsVeinhole);
	// Occupy -- points into a static table, picked again from Foundation as this loads.
}


ClassID TerrainTypeClass::Class_ID(void) const
{
	return(ClassID_TerrainTypeClass);
}


/// <summary>
/// Fetches the terrain type of the name specified, creating it if necessary.
/// This routine is used while the rules are being parsed, so that a terrain type may be
/// referred to before its own section has been reached.
/// </summary>
/// <returns>Returns with a pointer to the terrain type found or created.</returns>
TerrainTypeClass * TerrainTypeClass::Find_Or_Make(const char *name)
{
	return(TFind_Or_Make<TerrainTypeClass>(name, TerrainTypes));
}


/// <summary>
/// Fetches the RTTI type of this object.
/// </summary>
/// <returns>Returns with RTTI_TERRAINTYPE.</returns>
RTTIType TerrainTypeClass::Fetch_RTTI(void) const
{
	return(RTTI_TERRAINTYPE);
}


/// <summary>
/// Fetches the heap identifier of this terrain type.
/// </summary>
/// <returns>Returns with the index of this type within the terrain type heap.</returns>
int TerrainTypeClass::Fetch_Heap_ID(void) const
{
	return(HeapID);
}
