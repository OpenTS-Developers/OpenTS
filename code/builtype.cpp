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

/* $Header: /CounterStrike/BDATA.CPP 2     3/03/97 10:37p Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : BDATA.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : September 10, 1993                                           *
 *                                                                                             *
 *                  Last Update : October 2, 1996 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   BuildingTypeClass::As_Reference -- Fetches reference to the building type specified.      *
 *   BuildingTypeClass::Bib_And_Offset -- Determines the bib and appropriate cell offset.      *
 *   BuildingTypeClass::BuildingTypeClass -- This is the constructor for the building types.   *
 *   BuildingTypeClass::Coord_Fixup -- Adjusts coordinate to be legal for assignment.          *
 *   BuildingTypeClass::Cost_Of -- Fetches the cost of this building.                          *
 *   BuildingTypeClass::Create_And_Place -- Creates and places a building object onto the map. *
 *   BuildingTypeClass::Create_One_Of -- Creates a building of this type.                      *
 *   BuildingTypeClass::Dimensions -- Fetches the pixel dimensions of the building.            *
 *   BuildingTypeClass::Display -- Renders a generic view of building.                         *
 *   BuildingTypeClass::Flush_For_Placement -- Tries to clear placement area for this building *
 *   BuildingTypeClass::Full_Name -- Fetches the name to give this building.                   *
 *   BuildingTypeClass::Height -- Determines the height of the building in icons.              *
 *   BuildingTypeClass::Init -- Performs theater specific initialization.                      *
 *   BuildingTypeClass::Init_Anim -- Initialize an animation control for a building.           *
 *   BuildingTypeClass::Init_Heap -- Initialize the heap as necessary for the building type obj*
 *   BuildingTypeClass::Max_Pips -- Determines the maximum pips to display.                    *
 *   BuildingTypeClass::Occupy_List -- Fetches the occupy list for the building.               *
 *   BuildingTypeClass::One_Time -- Performs special one time action for buildings.            *
 *   BuildingTypeClass::Overlap_List -- Fetches the overlap list for the building.             *
 *   BuildingTypeClass::Prep_For_Add -- Prepares scenario editor for adding an object.         *
 *   BuildingTypeClass::Raw_Cost -- Fetches the raw (base) cost of this building type.         *
 *   BuildingTypeClass::Read_INI -- Fetch building type data from the INI database.            *
 *   BuildingTypeClass::Width -- Determines width of building in icons.                        *
 *   BuildingTypeClass::operator delete -- Deletes a building type object from the special heap*
 *   BuildingTypeClass::operator new -- Allocates a building type object from the special heap.*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "builtype.h"

#include "_map.h"
#include "_mixfile.h"
#include "_rules.h"
#include "_theater.h"
#include "airctype.h"
#include "bsurface.h"
#include "building.h"
#include "bullettype.h"
#include "ccfile.h"
#include "cell.h"
#include "data.h"
#include "findmake.h"
#include "foot.h"
#include "globals.h"
#include "house.h"
#include "incdec.h"
#include "isotype.h"
#include "mixfile.h"
#include "overtype.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "shapeset.h"
#include "sun.h"
#include "swizzle.h"
#include "tracker.h"
#include "unittype.h"
#include "warhead.h"
#include "weapon.h"

#include <algorithm>

void const * BuildingTypeClass::BuildingZShape;
void const * BuildingTypeClass::PowerOffShapes;
void const * BuildingTypeClass::WrenchShapes;

BSurface * CloakingSurface;

/// <summary>
/// Releases shape data that this type loaded through the file layer.
/// </summary>
static void Free_Demand_Loaded_Shape(void const *& data)
{
	delete [] (char *)data;
	data = NULL;
}

Cell const BuildingTypeClass::OccupyLists[BSIZE_COUNT][24] = {
	/* BSIZE_11,	*/ { Cell(0,0),REFRESH_EOL },
	/* BSIZE_21,	*/ { Cell(0,0),Cell(1,0),REFRESH_EOL },
	/* BSIZE_12,	*/ { Cell(0,0),Cell(0,1),REFRESH_EOL },
	/* BSIZE_22,	*/ { Cell(0,0),Cell(1,0),Cell(0,1),Cell(1,1),REFRESH_EOL },
	/* BSIZE_23,	*/ { Cell(0,0),Cell(1,0),Cell(0,1),Cell(1,1),Cell(0,2),Cell(1,2),REFRESH_EOL },
	/* BSIZE_32,	*/ { Cell(0,0),Cell(1,0),Cell(2,0),Cell(0,1),Cell(1,1),Cell(2,1),REFRESH_EOL },
	/* BSIZE_33,	*/ { Cell(0,0),Cell(1,0),Cell(2,0),Cell(0,1),Cell(1,1),Cell(2,1),Cell(0,2),Cell(1,2),Cell(2,2),REFRESH_EOL },
	/* BSIZE_35,	*/ { Cell(0,0),Cell(1,0),Cell(2,0),Cell(0,1),Cell(1,1),Cell(2,1),Cell(0,2),Cell(1,2),Cell(2,2),Cell(0,3),Cell(1,3),Cell(2,3),Cell(0,4),Cell(1,4),Cell(2,4),REFRESH_EOL },
	/* BSIZE_42,	*/ { Cell(0,0),Cell(1,0),Cell(2,0),Cell(3,0),Cell(0,1),Cell(1,1),Cell(2,1),Cell(3,1),REFRESH_EOL },
	/* BSIZE_33_REF	*/ { Cell(0,0),Cell(1,0),Cell(2,0),Cell(0,1),Cell(1,1),Cell(0,2),Cell(1,2),Cell(2,2),REFRESH_EOL },
	/* BSIZE_13,	*/ { Cell(0,0),Cell(0,1),Cell(0,2),REFRESH_EOL },
	/* BSIZE_31,	*/ { Cell(0,0),Cell(1,0),Cell(2,0),REFRESH_EOL },
	/* BSIZE_43,	*/ { Cell(0,0),Cell(1,0),Cell(2,0),Cell(3,0),Cell(0,1),Cell(1,1),Cell(2,1),Cell(3,1),Cell(0,2),Cell(1,2),Cell(2,2),Cell(3,2),REFRESH_EOL },
	/* BSIZE_14,	*/ { Cell(0,0),Cell(0,1),Cell(0,2),Cell(0,3),REFRESH_EOL },
	/* BSIZE_15,	*/ { Cell(0,0),Cell(0,1),Cell(0,2),Cell(0,3),Cell(0,4),REFRESH_EOL },
	/* BSIZE_26,	*/ { Cell(0,0),Cell(1,0),Cell(0,1),Cell(1,1),Cell(0,2),Cell(1,2),Cell(0,3),Cell(1,3),Cell(0,4),Cell(1,4),Cell(0,5),Cell(1,5),REFRESH_EOL },
	/* BSIZE_25,	*/ { Cell(0,0),Cell(1,0),Cell(0,1),Cell(1,1),Cell(0,2),Cell(1,2),Cell(0,3),Cell(1,3),Cell(0,4),Cell(1,4),REFRESH_EOL },
	/* BSIZE_53,	*/ { Cell(0,0),Cell(1,0),Cell(2,0),Cell(3,0),Cell(4,0),Cell(0,1),Cell(1,1),Cell(2,1),Cell(3,1),Cell(4,1),Cell(0,2),Cell(1,2),Cell(2,2),Cell(3,2),Cell(4,2),REFRESH_EOL },
	/* BSIZE_44,	*/ { Cell(0,0),Cell(1,0),Cell(2,0),Cell(3,0),Cell(0,1),Cell(1,1),Cell(2,1),Cell(3,1),Cell(0,2),Cell(1,2),Cell(2,2),Cell(3,2),Cell(0,3),Cell(1,3),Cell(2,3),Cell(3,3),REFRESH_EOL },
	/* BSIZE_34,	*/ { Cell(0,0),Cell(1,0),Cell(2,0),Cell(0,1),Cell(1,1),Cell(2,1),Cell(0,2),Cell(1,2),Cell(2,2),Cell(0,3),Cell(1,3),Cell(2,3),REFRESH_EOL },
	/* BSIZE_64,	*/ { Cell(0,0),Cell(1,0),Cell(2,0),Cell(3,0),Cell(4,0),Cell(5,0),Cell(0,1),Cell(1,1),Cell(2,1),Cell(3,1),Cell(4,1),Cell(5,1),Cell(0,2),Cell(1,2),Cell(2,2),Cell(3,2),Cell(4,2),Cell(5,2),Cell(1,3),Cell(2,3),Cell(3,3),REFRESH_EOL },
	/* BSIZE_00,	*/ { REFRESH_EOL }
};


Cell const BuildingTypeClass::ExitLists[BSIZE_COUNT][30] = {
	/* BSIZE_11,	*/	{ Cell(0,1),Cell(-1,1),Cell(1,1),Cell(-1,0),Cell(1,0),Cell(0,-1),Cell(-1,-1),Cell(1,-1),REFRESH_EOL },
	/* BSIZE_21,	*/	{ Cell(0,1),Cell(1,1),Cell(-1,1),Cell(2,1),Cell(-1,0),Cell(2,0),Cell(0,-1),Cell(1,-1),Cell(-1,-1),Cell(2,-1),REFRESH_EOL },
	/* BSIZE_12,	*/	{ Cell(0,2),Cell(1,2),Cell(-1,2),Cell(-1,1),Cell(1,1),Cell(-1,0),Cell(1,0),Cell(0,-1),Cell(-1,-1),Cell(1,-1),REFRESH_EOL },
	/* BSIZE_22,	*/	{ Cell(0,2),Cell(1,2),Cell(-1,2),Cell(2,2),Cell(-1,1),Cell(2,1),Cell(-1,0),Cell(2,0),Cell(0,-1),Cell(1,-1),Cell(-1,-1),Cell(2,-1),REFRESH_EOL },
	/* BSIZE_23,	*/	{ Cell(0,3),Cell(1,3),Cell(-1,3),Cell(2,3),Cell(-1,2),Cell(2,2),Cell(-1,1),Cell(2,1),Cell(-1,0),Cell(2,0),Cell(0,-1),Cell(1,-1),Cell(-1,-1),Cell(2,-1),REFRESH_EOL },
	/* BSIZE_32,	*/	{ Cell(0,2),Cell(1,2),Cell(2,2),Cell(-1,2),Cell(3,2),Cell(-1,1),Cell(3,1),Cell(-1,0),Cell(3,0),Cell(0,-1),Cell(1,-1),Cell(2,-1),Cell(-1,-1),Cell(3,-1),REFRESH_EOL },
	/* BSIZE_33,	*/	{ Cell(0,3),Cell(1,3),Cell(2,3),Cell(-1,3),Cell(3,3),Cell(-1,2),Cell(3,2),Cell(-1,1),Cell(3,1),Cell(-1,0),Cell(3,0),Cell(0,-1),Cell(1,-1),Cell(2,-1),Cell(-1,-1),Cell(3,-1),REFRESH_EOL },
	/* BSIZE_35,	*/	{ Cell(-1,-1),Cell(0,-1),Cell(1,-1),Cell(2,-1),Cell(3,-1),Cell(-1,0),Cell(3,0),Cell(-1,1),Cell(3,1),Cell(-1,2),Cell(3,2),Cell(-1,3),Cell(3,3),Cell(-1,4),Cell(3,4),Cell(-1,5),Cell(0,5),Cell(1,5),Cell(2,5),Cell(3,5),REFRESH_EOL },
	/* BSIZE_42,	*/	{ Cell(0,2),Cell(1,2),Cell(2,2),Cell(3,2),Cell(-1,2),Cell(4,2),Cell(-1,1),Cell(4,1),Cell(-1,0),Cell(4,0),Cell(0,-1),Cell(1,-1),Cell(2,-1),Cell(3,-1),Cell(-1,-1),Cell(4,-1),REFRESH_EOL },
	/* BSIZE_33_REF	*/	{ Cell(0,0),REFRESH_EOL },
	/* BSIZE_13,	*/	{ Cell(-1,-1),Cell(0,-1),Cell(1,-1),Cell(-1,0),Cell(1,0),Cell(-1,1),Cell(1,1),Cell(-1,2),Cell(1,2),Cell(-1,3),Cell(0,3),Cell(1,3),REFRESH_EOL },
	/* BSIZE_31,	*/	{ Cell(-1,-1),Cell(0,-1),Cell(1,-1),Cell(2,-1),Cell(3,-1),Cell(-1,0),Cell(3,0),Cell(-1,1),Cell(0,1),Cell(1,1),Cell(2,1),Cell(3,1),REFRESH_EOL },
	/* BSIZE_43,	*/	{ Cell(0,3),Cell(1,3),Cell(2,3),Cell(-1,3),Cell(3,3),Cell(-1,2),Cell(3,2),Cell(-1,1),Cell(3,1),Cell(-1,0),Cell(3,0),Cell(0,-1),Cell(1,-1),Cell(2,-1),Cell(-1,-1),Cell(3,-1),REFRESH_EOL },
	/* BSIZE_14,	*/	{ Cell(-1,-1),Cell(0,-1),Cell(1,-1),Cell(-1,0),Cell(1,0),Cell(-1,1),Cell(1,1),Cell(-1,2),Cell(1,2),Cell(-1,3),Cell(1,3),Cell(-1,4),Cell(0,4),Cell(1,4),REFRESH_EOL },
	/* BSIZE_15,	*/	{ Cell(-1,-1),Cell(0,-1),Cell(1,-1),Cell(-1,0),Cell(1,0),Cell(-1,1),Cell(1,1),Cell(-1,2),Cell(1,2),Cell(-1,3),Cell(1,3),Cell(-1,4),Cell(1,4),Cell(-1,5),Cell(0,5),Cell(1,5),REFRESH_EOL },
	/* BSIZE_26,	*/	{ Cell(-1,-1),Cell(0,-1),Cell(1,-1),Cell(2,-1),Cell(-1,0),Cell(2,0),Cell(-1,1),Cell(2,1),Cell(-1,2),Cell(2,2),Cell(-1,3),Cell(2,3),Cell(-1,5),Cell(2,5),Cell(-1,6),Cell(0,6),Cell(1,6),Cell(2,6),REFRESH_EOL },
	/* BSIZE_25,	*/	{ Cell(-1,-1),Cell(0,-1),Cell(1,-1),Cell(2,-1),Cell(-1,0),Cell(2,0),Cell(-1,1),Cell(2,1),Cell(-1,2),Cell(2,2),Cell(-1,3),Cell(2,3),Cell(-1,5),Cell(0,5),Cell(1,5),Cell(2,5),REFRESH_EOL },
	/* BSIZE_53,	*/	{ Cell(-1,-1),Cell(0,-1),Cell(1,-1),Cell(2,-1),Cell(3,-1),Cell(4,-1),Cell(5,-1),Cell(-1,0),Cell(5,0),Cell(-1,1),Cell(5,1),Cell(-1,2),Cell(5,2),Cell(-1,3),Cell(0,3),Cell(1,3),Cell(2,3),Cell(3,3),Cell(4,3),Cell(5,3),REFRESH_EOL },
	/* BSIZE_44,	*/	{ Cell(-1,-1),Cell(0,-1),Cell(1,-1),Cell(2,-1),Cell(3,-1),Cell(4,-1),Cell(-1,4),Cell(0,4),Cell(1,4),Cell(2,4),Cell(3,4),Cell(4,4),Cell(-1,0),Cell(-1,1),Cell(-1,2),Cell(-1,3),Cell(-1,4),Cell(4,0),Cell(4,1),Cell(4,2),Cell(4,3),Cell(4,4),REFRESH_EOL },
	/* BSIZE_34,	*/	{ Cell(0,4),Cell(1,4),Cell(2,4),Cell(-1,4),Cell(3,4),Cell(-1,2),Cell(3,2),Cell(-1,1),Cell(3,1),Cell(-1,0),Cell(3,0),Cell(0,-1),Cell(1,-1),Cell(2,-1),Cell(-1,-1),Cell(3,-1),REFRESH_EOL },
	/* BSIZE_64,	*/	{ Cell(2,-1),REFRESH_EOL },
	/* BSIZE_00,	*/	{ REFRESH_EOL }
};


int BuildingTypeClass::SizeWidth[BSIZE_COUNT] = {
	1,
	2,
	1,
	2,
	2,
	3,
	3,
	3,
	4,
	3,
	1,
	3,
	4,
	1,
	1,
	2,
	2,
	5,
	4,
	3,
	6,
	0
};


int BuildingTypeClass::SizeHeight[BSIZE_COUNT] = {
	1,
	1,
	2,
	2,
	3,
	2,
	3,
	5,
	2,
	3,
	3,
	1,
	3,
	4,
	5,
	6,
	5,
	3,
	4,
	4,
	4,
	0
};


/***********************************************************************************************
 * BuildingTypeClass::BuildingTypeClass -- This is the constructor for the building types.     *
 *                                                                                             *
 *    This is the constructor used to create the building types.                               *
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
BuildingTypeClass::BuildingTypeClass(char const * ininame) :
	BASECLASS(ininame, SPEED_FOOT),
	HeapID(STRUCT_NONE),
	OccupyList(NULL),
	BuildupData(NULL),
	HalfDamageSmokeLocation1(COORD_NONE),
	HalfDamageSmokeLocation2(COORD_NONE),
	GateCloseDelay(0.0),
	LightVisibility(5000),
	LightIntensity(0),
	LightRedTint(1000000),
	LightGreenTint(1000000),
	LightBlueTint(1000000),
	PrimaryFirePixelOffset(Point2D(0xFFFF, 0xFFFF)),
	SecondaryFirePixelOffset(Point2D(0xFFFF, 0xFFFF)),
	ToOverlay(NULL),
	ToTile(NULL),
	FreeUnit(NULL),
	FoundationFace(FACING_NONE),
	Adjacent(3),
	ToBuild(RTTI_NONE),
	ExitCoordinate(0, 0, 0),
	ExitList(NULL),
	StartFace(DIR_N),
	Power(0),
	Drain(0),
	Size(BSIZE_11),
	ZHeight(1),
	MidPoint(0),
	DoorStages(0),
	Upgrades(0),
	DeployingAnim(NULL),
	UnderDoorAnim(NULL),
	DoorAnim(NULL),
	SpecialZOverlay(NULL),
	SpecialZOverlayZAdjust(0),
	BibShape(NULL),
	NormalZAdjust(0),
	AntiAirValue(0),
	AntiArmorValue(0),
	AntiInfantryValue(0),
	ZShapePointMove(Point2D(0, 0)),
	DrawRect(RECT_NONE),
	ExtraLight(0),
	IsCanTogglePower(true),
	HasSpotlight(false),
	IsTemple(false),
	IsPlug(false),
	IsHoverPad(false),
	IsBase(true),
	IsBibbed(false),
	IsWall(false),
	IsCaptureable(false),
	IsPowered(false),
	IsUnsellable(false),
	IsRadar(false),
	IsHasChargeAnim(false),
	IsSiloDamage(false),
	IsCanUnitRepair(false),
	IsCanUnitReload(false),
	IsFlat(false),
	IsDockUnload(false),
	IsRecoilless(false),
	IsHasStupidGuardMode(true),
	IsBridgeRepairHut(false),
	IsGate(false),
	IsSAM(false),
	IsConstructionYard(false),
	IsNukeSilo(false),
	IsRefinery(false),
	IsWeeder(false),
	IsWeaponsFactory(false),
	IsLaserFencePost(false),
	IsLaserFence(false),
	IsFirestormWall(false),
	IsHospital(false),
	IsArmory(false),
	IsEMPulseCannon(false),
	IsTickTank(false),
	IsTurretAnimAVoxel(false),
	IsCloakGenerator(false),
	IsSensorArray(false),
	IsICBMLauncher(false),
	IsArtillary(false),
	IsHelipad(false),
	IsGDIBarracks(false),
	IsNODBarracks(false),
	SuperWeapon(SUPER_NONE),
	SuperWeapon2(SUPER_NONE),
	GateStages(9),
	PowersUpToLevel(-1),
	VoxelBarrelScale(1.0),
	TurretChargeAnimRate(3),
	StartPitch(DIR_E),
	IsLimpetMine(false),
	IsMobileWar(false),
	IsMobileStealth(false),
	IsJuggernaut(false),
	IsCoreDefender(false),
	IsBarrelAnimAVoxel(false),
	IsTurretAnimExclusive(false),
	IsDamagedDoor(false),
	IsInvisibleInGame(false),
	IsTerrainPalette(false),
	IsCanPlaceAnywhere(false),
	IsExtraDamageStage(true),
	CanAIBuildThis(false),
	IsBaseDefense(false),
	CloakRadiusInCells(20),
	IsDemandLoad(false),
	IsDemandLoadBuildup(false),
	IsFreeBuildup(false),
	IsThreatRatingNode(false)
{
	Create_ID();
	BuildingTypes.Add(this);
	HeapID = (StructType)BuildingTypes.ID(this);

	Init_Anim(BSTATE_CONSTRUCTION, 0, 1, 0);
	Init_Anim(BSTATE_IDLE, 0, 1, 0);
	Init_Anim(BSTATE_ACTIVE, 0, 1, 0);
	Init_Anim(BSTATE_AUX1, 0, 1, 0);
	Init_Anim(BSTATE_AUX2, 0, 1, 0);

	for (int i = 0; i < BANIM_COUNT; i++) {
		memset(AnimData[i].Anim, 0, sizeof(AnimData[i].Anim));
		memset(AnimData[i].AnimDamaged, 0, sizeof(AnimData[i].AnimDamaged));
		AnimData[i].Location = Point2D(0, 0);
		AnimData[i].ZAdjust = 0;
		AnimData[i].YSort = 0;
		AnimData[i].Powered = true;
		AnimData[i].PoweredLight = false;
	}

	BuildupFilename.clear();
	PowersUpBuilding.clear();
	TheaterImageFile[0] = '\0';
	VoxelBarrelFile[0] = '\0';
	IsTrainable = false;
}


/// <summary>
/// Destroys the building type and releases any art it demand loaded.
/// The type also detaches itself from everything that refers to it and drops out of the
/// building type heap.
/// </summary>
BuildingTypeClass::~BuildingTypeClass(void)
{
	if (IsDemandLoad && ImageData != NULL) {
		Free_Demand_Loaded_Shape(ImageData);
	}
	if (IsDemandLoadBuildup && BuildupData != NULL) {
		Free_Demand_Loaded_Shape(BuildupData);
	}
	Detach_This_From_All(this, true);
	BuildingTypes.Delete(this);
}


/***********************************************************************************************
 * BuildingTypeClass::One_Time -- Performs special one time action for buildings.              *
 *                                                                                             *
 *    This routine is used to do the one time action necessary to handle building type class   *
 *    objects. This entails loading of the building shapes and the brain file used by          *
 *    buildings.                                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine should only be called ONCE.                                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *   06/11/1994 JLB : Updated construction time and frame count logic.                         *
 *=============================================================================================*/
void BuildingTypeClass::One_Time(void)
{

}


/// <summary>
/// Converts a given name into a building type number.
/// This routine matches against the name the building is known by in the game, rather
/// than the name it is declared under in the rules.
/// </summary>
/// <param name="name">The given name of the building to search for.</param>
/// <returns>Returns with the building type that matches, or STRUCT_NONE if there is no
/// match.</returns>
StructType BuildingTypeClass::From_Given_Name(char const * name)
{
	if (name != NULL) {
		for (int classid = STRUCT_FIRST; classid < BuildingTypes.Count(); classid++) {
			if (stricmp(BuildingTypes[classid]->GivenName, name) == 0) {
				return((StructType)classid);
			}
		}
	}
	return(STRUCT_NONE);
}


/***********************************************************************************************
 * Struct_From_Name -- Find BData structure from its name.                                     *
 *                                                                                             *
 *    This routine will convert an ASCII name for a building class into                        *
 *    the actual building class it represents.                                                 *
 *                                                                                             *
 * INPUT:   name  -- ASCII representation of a building class.                                 *
 *                                                                                             *
 * OUTPUT:  Returns with the actual building class number that the string                      *
 *          represents.                                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/1992 JLB : Created.                                                                 *
 *   05/02/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
StructType BuildingTypeClass::From_Name(char const * name)
{
	if (name != NULL) {
		for (StructType classid = STRUCT_FIRST; classid < BuildingTypes.Count(); classid++) {
			if (stricmp(BuildingTypes[classid]->Name(), name) == 0) {
				return(classid);
			}
		}
	}
	return(STRUCT_NONE);
}


/***********************************************************************************************
 * BuildingTypeClass::Create_And_Place -- Creates and places a building object onto the map.   *
 *                                                                                             *
 *    This routine is used by the scenario editor to create and place buildings on the map.    *
 *                                                                                             *
 * INPUT:   cell     -- The cell that the building is to be placed upon.                       *
 *                                                                                             *
 *          house    -- The owner of the building.                                             *
 *                                                                                             *
 * OUTPUT:  bool; Was the building successfully created and placed on the map?                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool BuildingTypeClass::Create_And_Place(Cell const & cell, HouseClass * house) const
{
	BuildingClass * ptr;

	ptr = new BuildingClass(this, house);
	if (ptr != NULL) {
		return(ptr->Unlimbo(cell, DIR_N));
	}
	return(false);
}


/***********************************************************************************************
 * BuildingTypeClass::Create_One_Of -- Creates a building of this type.                        *
 *                                                                                             *
 *    This routine will create a building object of this type. The building object is in a     *
 *    limbo state. It is presumed that the building object will be unlimboed at the correct    *
 *    place and time. Typical use is when the building is created in a factory situation       *
 *    and will be placed on the map when construction completes.                               *
 *                                                                                             *
 * INPUT:   house -- Pointer to the house that is to be the owner of the building.             *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the building. If the building could not be created       *
 *          then a NULL is returned.                                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/07/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectClass * BuildingTypeClass::Create_One_Of(HouseClass * house) const
{
	return(new BuildingClass(this, house));
}


/***********************************************************************************************
 * BuildingTypeClass::Init_Anim -- Initialize an animation control for a building.             *
 *                                                                                             *
 *    This routine will initialize one animation control element for a                         *
 *    specified building. This modifies a "const" class and thus must                          *
 *    perform some strategic casting to get away with this.                                    *
 *                                                                                             *
 * INPUT:   state -- The animation state to apply these data values to.                        *
 *                                                                                             *
 *          start -- Starting frame for the building's animation.                              *
 *                                                                                             *
 *          count -- The number of frames in this animation.                                   *
 *                                                                                             *
 *          rate  -- The countdown timer between animation frames.                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void BuildingTypeClass::Init_Anim(BStateType state, int start, int count, int rate) const
{
	((int &)Anims[state].Start) = start;
	((int &)Anims[state].Count) = count;
	((int &)Anims[state].Rate) = rate;
}


/// <summary>
/// Fetches the depth shape shared by all buildings.
/// This routine loads the Z shape that buildings are rendered against and biases its
/// values into the range the depth buffer expects.
/// </summary>
void BuildingTypeClass::Fetch_Z_Data(void)
{
	if (BuildingZShape != NULL) {
		delete [] (char*) BuildingZShape;
		BuildingZShape = NULL;
	}

	int size = CCFileClass("BUILDNGZ.SHP").Size();
	BuildingZShape = new char[size];
	memcpy((void *)BuildingZShape, MFCD::Retrieve("BUILDNGZ.SHP"), size);

	ShapeSet * zshape = (ShapeSet *)BuildingZShape;
	char * data = (char *)zshape->Get_Data(0);
	int width = zshape->Get_Width();
	int height = zshape->Get_Height();

	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {
			if (data[h * width + w] != 0) {
				data[h * width + w] -= 39;
			}
		}
	}
}


/***********************************************************************************************
 * BuildingTypeClass::Init -- Performs theater specific initialization.                        *
 *                                                                                             *
 *    This routine is used to perform any initialization that is custom per theater.           *
 *    Typically, this is fetching the building shape data for those building types that have   *
 *    theater specific art.                                                                    *
 *                                                                                             *
 * INPUT:   theater  -- The theater to base this initialization on.                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void BuildingTypeClass::Init(TheaterType theater)
{
	Fetch_Z_Data();
	PowerOffShapes = MFCD::Retrieve("POWEROFF.SHP");
	WrenchShapes = MFCD::Retrieve("WRENCH.SHP");

	char fullname[_MAX_FNAME+_MAX_EXT];

	int maxcloak = 0;

	for (StructType sindex = STRUCT_FIRST; sindex < BuildingTypes.Count(); sindex++) {
		BuildingTypeClass * classptr = BuildingTypes[sindex];

		maxcloak = std::max<int>(maxcloak, classptr->CloakRadiusInCells);

		if (classptr->IsTheater) {

			if (!classptr->IsDemandLoad) {
				_makepath(fullname, NULL, NULL, classptr->Graphic_Name(), Theaters[theater].Suffix);
				classptr->ImageData = MFCD::Retrieve(fullname);
			} else {
				if (classptr->ImageData != NULL) {
					Free_Demand_Loaded_Shape(classptr->ImageData);
				}
			}

			/*
			**	Buildup data is probably theater specific as well. Fetch a pointer to the
			**	data at this time as well.
			*/
			if (!classptr->IsDemandLoadBuildup) {
				_makepath(fullname, NULL, NULL, classptr->BuildupFilename, Theaters[theater].Suffix);
				classptr->BuildupData = MFCD::Retrieve(fullname);
			} else {
				if (classptr->BuildupData != NULL) {
					Free_Demand_Loaded_Shape(classptr->BuildupData);
				}
			}

			if (classptr->BuildupData) {
				int timedelay = 1;
				int count = ((ShapeSet const *)classptr->BuildupData)->Get_Count();
				if (count != 0) {
					timedelay = (5 * TICKS_PER_SECOND) / count;
				}
				classptr->Init_Anim(BSTATE_CONSTRUCTION, 0, count, timedelay);
			}
		} else if (classptr->IsNewTheater) {
			if (classptr->IsDemandLoad) {
				if (classptr->ImageData != NULL) {
					Free_Demand_Loaded_Shape(classptr->ImageData);
				}
			}
			if (classptr->IsDemandLoadBuildup) {
				if (classptr->BuildupData != NULL) {
					Free_Demand_Loaded_Shape(classptr->BuildupData);
				}
			}
			classptr->Fetch_Building_Normal_Image(theater);
		}
	}

	if (maxcloak != 0) {
		delete CloakingSurface;
		CloakingSurface = new BSurface(maxcloak + 16, maxcloak + 16, 1);
	}
}


/***********************************************************************************************
 * BuildingTypeClass::Dimensions -- Fetches the pixel dimensions of the building.              *
 *                                                                                             *
 *    This routine will fetch the dimensions of the building (in pixels). These dimensions are *
 *    used to render the selection rectangle and the health bar.                               *
 *                                                                                             *
 * INPUT:   width    -- Reference to the pixel width (to be filled in).                        *
 *                                                                                             *
 *          height   -- Reference to the pixel height (to be filled in).                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
Point3D BuildingTypeClass::Pixel_Dimensions(void) const
{
	ShapeSet const * data = (ShapeSet const *)Get_Image_Data();
	int w;
	int h;
	if (data != NULL) {
		Rect r = data->Get_Rect(0);
		w = r.Width;
		h = r.Height;
	} else {
		w = 0;
		h = 0;
	}
	return(Point3D(w, w, h));
}


/***********************************************************************************************
 * BuildingTypeClass::Occupy_List -- Fetches the occupy list for the building.                 *
 *                                                                                             *
 *    Use this routine to fetch the occupy list pointer for the building. The occupy list is   *
 *    used to determine what cells the building occupies and thus precludes other buildings    *
 *    or objects from using.                                                                   *
 *                                                                                             *
 * INPUT:   placement   -- Is this for placement legality checking only? The normal condition  *
 *                         is for marking occupation flags.                                    *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to a cell offset list to be used to determine what cells    *
 *          this building occupies.                                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
Cell const * BuildingTypeClass::Occupy_List(bool placement) const
{
	static Cell const _templap[] = {REFRESH_EOL};
	if (OccupyList != NULL) {
		return(OccupyList);
	}
	return(_templap);
}


/***********************************************************************************************
 * BuildingTypeClass::Width -- Determines width of building in icons.                          *
 *                                                                                             *
 *    Use this routine to determine the width of the building type in icons.                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the building width in icons.                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int BuildingTypeClass::Width(void) const
{
	return(SizeWidth[Size]);
}


/***********************************************************************************************
 * BuildingTypeClass::Height -- Determines the height of the building in icons.                *
 *                                                                                             *
 *    Use this routine to find the height of the building in icons.                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the building height in icons.                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int BuildingTypeClass::Height(bool bib) const
{
	return(SizeHeight[Size] + ((bib && IsBibbed) ? 1 : 0));
}


/***********************************************************************************************
 * BuildingTypeClass::Max_Pips -- Determines the maximum pips to display.                      *
 *                                                                                             *
 *    Use this routine to determine the maximum number of pips to display on this building     *
 *    when it is rendered. Typically, this is the tiberium capacity divided by 100.            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of pips to display on this building when selected.         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int BuildingTypeClass::Max_Pips(void) const
{
	int maxpips = (Width() * ISO_TILE_PIXEL_W) / 8;

	switch (PipScale) {
		case PIPSCALE_TIBERIUM:
			if (IsWeeder) {
				return(std::clamp((int)(Rule->WeedCapacity), 0, maxpips));
			} else {
				return(std::clamp((int)(Capacity), 0, maxpips));
			}

		case PIPSCALE_POWER:
			return(maxpips);

		default:
			return(BASECLASS::Max_Pips());
	}
}


/***********************************************************************************************
 * BuildingTypeClass::Raw_Cost -- Fetches the raw (base) cost of this building type.           *
 *                                                                                             *
 *    This routine is used to fetch the real raw base cost of the building. The raw cost       *
 *    is the cost of the building less any free unit that would come with the building         *
 *    if it were built in the normal fashion. Specifically, the helicopter cost is subtracted  *
 *    from the helipad and the harvester cost is subtracted from the refinery. This cost       *
 *    is used for refunding.                                                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the raw (base) cost to build the building of this type.                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int BuildingTypeClass::Raw_Cost(void) const
{
	int cost = BASECLASS::Raw_Cost();

	if (this == Rule->PadAircraft[0]->Dock[0] && !Rule->IsSeparate) {
		cost -= (Rule->PadAircraft[0]->Raw_Cost() + Rule->PadAircraft[1]->Raw_Cost())/2;
	}
	if (FreeUnit != NULL) {
		cost -= FreeUnit->Raw_Cost();
		cost = std::max(cost, 0);
	}
	return(cost);
}


/***********************************************************************************************
 * BuildingTypeClass::Cost_Of -- Fetches the cost of this building.                            *
 *                                                                                             *
 *    This routine will fetch the cost to build the building of this type.                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the cost to produce this building.                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int BuildingTypeClass::Cost_Of(HouseClass * house) const
{
	int cost = BASECLASS::Cost_Of(house);

	if (this == Rule->PadAircraft[0]->Dock[0] && !Rule->IsSeparate) {
		cost += (Rule->PadAircraft[0]->Cost_Of(house) + Rule->PadAircraft[1]->Cost_Of(house))/2;
	}
	if (FreeUnit != NULL) {
		cost += FreeUnit->Cost_Of(house);
		cost = std::max(cost, 0);
	}
	return(cost);
}


/***********************************************************************************************
 * BuildingTypeClass::Flush_For_Placement -- Tries to clear placement area for this building t *
 *                                                                                             *
 *    This routine is called when a clear space for placement is desired at the cell location  *
 *    specified. Typical use of this routine is by the computer when it wants to build up      *
 *    its base.                                                                                *
 *                                                                                             *
 * INPUT:   cell  -- The cell that the building of this type would like to be placed down at.  *
 *                                                                                             *
 *          house -- Pointer to the house that want to clear the foundation zone.              *
 *                                                                                             *
 * OUTPUT:  Placement is temporarily blocked, please try again later?                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/27/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int BuildingTypeClass::Flush_For_Placement(Cell const & cell, HouseClass * house) const
{
	bool again = false;
	if (cell != CELL_NONE) {
		Cell const * list = Occupy_List(true);

		while (*list != REFRESH_EOL) {
			Cell newcell = cell + *list++;

			if (Map.In_Radar(newcell)) {
				CellClass & cptr = Map[newcell];
				if (cptr.Overlay != OVERLAY_NONE && (this != Rule->WallTower || cptr.Overlay != OVERLAY_BRICK_WALL)) {
					return(2);
				}
				ObjectClass * occupier = cptr.Cell_Occupier();
				if (occupier != NULL) {
					if (occupier->RTTI == RTTI_TERRAIN) {
						return(2);
					}
					TechnoClass * techno = dynamic_cast<TechnoClass *>(occupier); /// Note, not As_TechnoClass(), that uses Abstract to Techno, this is Object to Techno
					if (techno != NULL) {
						if (techno->RTTI == RTTI_BUILDING) {
							if (!((BuildingClass *)techno)->Can_Upgrade(this, house)) {
								return(2);
							}
						} else {
							if (techno->House->Is_Ally(house) && techno->Is_Foot()) {
								again = true;
								if (((FootClass *)techno)->NavCom == NULL || &Map[techno->Get_Coord()] == ((FootClass *)techno)->NavCom) {
									Map[newcell].Incoming(COORD_NONE, true, true);
								}
							} else {
								return(2);
							}
						}
					}
				}
			}
		}
	}
	if (again) {
		return(1);
	} else {
		return(0);
	}

}


/// <summary>
/// Fetches the shape data for this building.
/// This routine will load the art the first time it is asked for if this building type
/// was flagged to defer loading until it is really needed.
/// </summary>
/// <returns>Returns with a pointer to the shape data, or NULL if there is none.</returns>
void const * BuildingTypeClass::Get_Image_Data(void) const
{
	if (ImageData == NULL && IsDemandLoad) {
		if (TheaterImageFile[0] != '\0') {
			CCFileClass file(TheaterImageFile);
			(void *&)ImageData = Load_Alloc_Data(file);
		}
		return(ImageData);
	}
	return(ImageData);
}


/// <summary>
/// Fetches the shape art for this building.
/// This routine picks up the building's own shape along with all of its supporting art --
/// the construction animation, the deploy and door animations, the bib and the special Z
/// overlay -- and then fetches any voxel art the building needs as well.
/// </summary>
/// <param name="theater">The theater to fetch the art for.</param>
void BuildingTypeClass::Fetch_Building_Normal_Image(TheaterType theater)
{
	char fullname[MAX_PATH];
	char buffer[64];

	if (!IsDemandLoadBuildup) {
		if (!BuildupFilename.empty()) {

			/*
			**	Fetch the construction animation for this building.
			*/
			_makepath(fullname, NULL, NULL, BuildupFilename, ".SHP");
			Theater_Naming_Convention(fullname, theater);
			BuildupData = MFCD::Retrieve(fullname);
			if (BuildupData != NULL) {
				int timedelay = 1;
				int count = ((ShapeSet const *)BuildupData)->Get_Count()/2;
				if (IsGate) {
					count = GateStages + 1;
				}
				if (count > 0) {
					timedelay = (Rule->BuildupTime * TICKS_PER_MINUTE) / count;
				}
				Init_Anim(BSTATE_CONSTRUCTION, 0, count, timedelay);
			}
		}
	}

	ArtINI.Get_String(Graphic_Name(), "DeployingAnim", "", buffer, sizeof(buffer));
	if (strlen(buffer) != 0) {
		_makepath(fullname, NULL, NULL, buffer, ".SHP");
		ObjectTypeClass::Theater_Naming_Convention(fullname, theater);
		DeployingAnim = (ShapeSet const *)MFCD::Retrieve(fullname);
	}

	ArtINI.Get_String(Graphic_Name(), "DoorAnim", "", buffer, sizeof(buffer));
	if (strlen(buffer) != 0) {
		_makepath(fullname, NULL, NULL, buffer, ".SHP");
		ObjectTypeClass::Theater_Naming_Convention(fullname, theater);
		DoorAnim = (ShapeSet const *)MFCD::Retrieve(fullname);
	}

	ArtINI.Get_String(Graphic_Name(), "UnderDoorAnim", "", buffer, sizeof(buffer));
	if (strlen(buffer) != 0) {
		_makepath(fullname, NULL, NULL, buffer, ".SHP");
		ObjectTypeClass::Theater_Naming_Convention(fullname, theater);
		UnderDoorAnim = (ShapeSet const *)MFCD::Retrieve(fullname);
	}

	ArtINI.Get_String(Graphic_Name(), "SpecialZOverlay", "", buffer, sizeof(buffer));
	if (strlen(buffer) != 0) {
		_makepath(fullname, NULL, NULL, buffer, ".SHP");
		ObjectTypeClass::Theater_Naming_Convention(fullname, theater);
		SpecialZOverlay = (ShapeSet const *)MFCD::Retrieve(fullname);
	}

	ArtINI.Get_String(Graphic_Name(), "BibShape", "", buffer, sizeof(buffer));
	if (strlen(buffer) != 0) {
		_makepath(fullname, NULL, NULL, buffer, ".SHP");
		ObjectTypeClass::Theater_Naming_Convention(fullname, theater);
		BibShape = (ShapeSet const *)MFCD::Retrieve(fullname);
	}

	char ext[16];
	ArtINI.Get_String(Graphic_Name(), "Image", "", buffer, sizeof(buffer));
	if (!IsTheater || theater == THEATER_NONE) {
		strcpy(ext, ".SHP");
	} else {
		strcpy(ext, Theaters[theater].Suffix);
	}

	if (strlen(buffer)) {
		_makepath(fullname, NULL, NULL, buffer, ext);
	} else {
		_makepath(fullname, NULL, NULL, Graphic_Name(), ext);
	}

	ObjectTypeClass::Theater_Naming_Convention(fullname, theater);
	strncpy(TheaterImageFile, fullname, sizeof(TheaterImageFile) - 1);

	/*
	**	Fetch the normal game shape for this building.
	*/
	if (!IsDemandLoad) {
		ImageData = MFCD::Retrieve(fullname);
	}

	Fetch_Building_Voxel_Image();
}


/// <summary>
/// Searches for a substring, ignoring case.
/// The string being searched is uppercased in place, so it does not survive the call
/// unchanged. The substring is matched as supplied and must already be uppercase.
/// </summary>
/// <param name="string">String to search; uppercased in place.</param>
/// <param name="substring">Uppercase substring to search for.</param>
/// <returns>Returns with a pointer to the substring within the string. Otherwise, NULL is
/// returned.</returns>
static char * Upr_Strstr(char * string, char const * substring)
{
	return(strstr(_strupr(string), substring));
}


/// <summary>
/// Fetches the voxel art for this building's turret and barrel.
/// This routine is used by those buildings that draw their turret or barrel as a voxel
/// rather than as an animation. The matching motion data is loaded alongside each model.
/// </summary>
void BuildingTypeClass::Fetch_Building_Voxel_Image(void)
{
	char buffer[128];
	char fullname[256];

	if (IsTurretAnimAVoxel || IsBarrelAnimAVoxel) {
		strcpy(buffer, AnimData[BANIM_TURRET].Anim);
		_makepath(fullname, NULL, NULL, buffer, ".VXL");

		if (Upr_Strstr(&fullname[4], "TUR")) {
			CCFileClass vxl(fullname);
			if (vxl.Is_Available()) {
				delete AuxVoxel.VoxLib;
				AuxVoxel.VoxLib = new VoxelLibrary(vxl);

				_makepath(fullname, NULL, NULL, buffer, ".HVA");
				CCFileClass hva(fullname);
				delete AuxVoxel.MotLib;
				AuxVoxel.MotLib = new MotionLibrary(hva);

				if (AuxVoxel.MotLib != NULL && !AuxVoxel.MotLib->Load_Failed()) {
					AuxVoxel.MotLib->Scale(AuxVoxel.VoxLib->Get_Layer_Info(0, 0).Scale);
				}
			}

			if (char * nameroot = Upr_Strstr(&buffer[4], "TUR")) {
				*nameroot++ = 'B';
				*nameroot++ = 'A';
				*nameroot++ = 'R';
				*nameroot++ = 'L';
				*nameroot++ = '\0';
			}
		} else if (IsBarrelAnimAVoxel) {
			strncpy(buffer, VoxelBarrelFile, sizeof(buffer));
		}

		_makepath(fullname, NULL, NULL, buffer, ".VXL");
		CCFileClass turvxl(fullname);
		if (turvxl.Is_Available()) {
			delete AuxVoxel2.VoxLib;
			AuxVoxel2.VoxLib = new VoxelLibrary(turvxl);

			_makepath(fullname, NULL, NULL, buffer, ".HVA");
			CCFileClass turhva(fullname);
			delete AuxVoxel2.MotLib;
			AuxVoxel2.MotLib = new MotionLibrary(turhva);

			if (AuxVoxel2.MotLib != NULL && !AuxVoxel2.MotLib->Load_Failed()) {
				AuxVoxel2.MotLib->Scale(AuxVoxel2.VoxLib->Get_Layer_Info(0, 0).Scale);
			}
		}
	}
}


/***********************************************************************************************
 * BuildingTypeClass::Read_INI -- Fetch building type data from the INI database.              *
 *                                                                                             *
 *    This routine will fetch the building type class data from the INI database file.         *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database that will be examined.                      *
 *                                                                                             *
 * OUTPUT:  bool; Was the building entry found and the data extracted?                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool BuildingTypeClass::Read_INI(CCINIClass const & ini)
{
	char buffer[128];

	if (!ini.Section_Present(IniName)) {
		return(false);
	}

	if (IsDemandLoad && ImageData != NULL) {
		Free_Demand_Loaded_Shape(ImageData);
	}
	if (IsDemandLoadBuildup && BuildupData != NULL) {
		Free_Demand_Loaded_Shape(BuildupData);
	}

	if (BASECLASS::Read_INI(ini)) {

		HasSpotlight = ini.Get_Bool(Name(), "HasSpotlight", HasSpotlight);
		HalfDamageSmokeLocation1 = ini.Get_Point(Name(), "HalfDamageSmokeLocation1", HalfDamageSmokeLocation1);
		HalfDamageSmokeLocation2 = ini.Get_Point(Name(), "HalfDamageSmokeLocation2", HalfDamageSmokeLocation2);
		IsRadar = ini.Get_Bool(Name(), "Radar", IsRadar);
		Speed = ini.Get_Bool(Name(), "WaterBound", Speed == SPEED_FLOAT) ? SPEED_FLOAT : SPEED_NONE;
		Adjacent = ini.Get_Int(Name(), "Adjacent", Adjacent);
		IsCaptureable = ini.Get_Bool(Name(), "Capturable", IsCaptureable);
		IsPowered = ini.Get_Bool(Name(), "Powered", IsPowered);
		IsBibbed = ini.Get_Bool(Name(), "Bib", IsBibbed);
		IsUnsellable = ini.Get_Bool(Name(), "Unsellable", IsUnsellable);

		IsBase = ini.Get_Bool(Name(), "BaseNormal", IsBase);
		if (!stricmp(Name(), "NAFNCE") || !stricmp(Name(), "NAPOST")) {
			IsBase = false;
		}

		IsWall = ini.Get_Bool(Name(), "Wall", IsWall);
		IsWeeder = ini.Get_Bool(Name(), "Weeder", IsWeeder);
		IsHelipad = ini.Get_Bool(Name(), "Helipad", IsHelipad);
		IsLimpetMine = ini.Get_Bool(Name(), "IsLimpetMine", IsLimpetMine);
		IsMobileWar = ini.Get_Bool(Name(), "IsMobileWar", IsMobileWar);
		IsMobileStealth = ini.Get_Bool(Name(), "IsMobileStealth", IsMobileStealth);
		IsJuggernaut = ini.Get_Bool(Name(), "IsJuggernaut", IsJuggernaut);
		IsCoreDefender = ini.Get_Bool(Name(), "IsCoreDefender", IsCoreDefender);

		IsBarrelAnimAVoxel = ini.Get_Bool(Name(), "BarrelAnimIsVoxel", IsBarrelAnimAVoxel);
		ini.Get_String(Name(), "VoxelBarrelFile", VoxelBarrelFile, VoxelBarrelFile, sizeof(VoxelBarrelFile));
		VoxelBarrelOffsetToPitchPivotPoint = ini.Get_Point(Name(), "VoxelBarrelOffsetToPitchPivotPoint", VoxelBarrelOffsetToPitchPivotPoint);
		VoxelBarrelOffsetToRotatePivotPoint = ini.Get_Point(Name(), "VoxelBarrelOffsetToRotatePivotPoint", VoxelBarrelOffsetToRotatePivotPoint);
		VoxelBarrelOffsetToBuildingPivotPoint = ini.Get_Point(Name(), "VoxelBarrelOffsetToBuildingPivotPoint", VoxelBarrelOffsetToBuildingPivotPoint);
		VoxelBarrelOffsetToBarrelEnd = ini.Get_Point(Name(), "VoxelBarrelOffsetToBarrelEnd", VoxelBarrelOffsetToBarrelEnd);
		StartFace = Dir256(ini.Get_Int(Name(), "StartFacing", StartFace >> 5) << 5);
		StartPitch = Dir256(ini.Get_Int(Name(), "StartPitch", StartPitch >> 5) << 5);
		VoxelBarrelScale = ini.Get_Float(Name(), "VoxelBarrelScale", VoxelBarrelScale);
		TurretChargeAnimRate = ini.Get_Int(Name(), "TurretChargeAnimRate", TurretChargeAnimRate);
		IsTurretAnimExclusive = ini.Get_Bool(Name(), "TurretAnimIsExclusive", IsTurretAnimExclusive);

		AuxSound1 = ini.Get_VocType(Name(), "DeploySound", AuxSound1);
		AuxSound2 = ini.Get_VocType(Name(), "UndeploySound", AuxSound2);
		ToBuild = ini.Get_RTTIType(Name(), "Factory", ToBuild);
		FreeUnit = TGet_Class(ini, Name(), "FreeUnit", FreeUnit);
		IsHoverPad = ini.Get_Bool(Name(), "HoverPad", IsHoverPad);
		IsTemple = ini.Get_Bool(Name(), "IsTemple", IsTemple);
		IsPlug = ini.Get_Bool(Name(), "IsPlug", IsPlug);
		IsCanTogglePower = ini.Get_Bool(Name(), "TogglePower", IsCanTogglePower);
		IsCanUnitRepair = ini.Get_Bool(Name(), "UnitRepair", IsCanUnitRepair);
		IsCanUnitReload = ini.Get_Bool(Name(), "UnitReload", IsCanUnitReload);
		IsDockUnload = ini.Get_Bool(Name(), "DockUnload", IsDockUnload);
		IsGate = ini.Get_Bool(Name(), "Gate", IsGate);
		IsSAM = ini.Get_Bool(Name(), "SAM", IsSAM);
		IsConstructionYard = ini.Get_Bool(Name(), "ConstructionYard", IsConstructionYard);
		IsNukeSilo = ini.Get_Bool(Name(), "NukeSilo", IsNukeSilo);
		IsRefinery = ini.Get_Bool(Name(), "Refinery", IsRefinery);
		IsWeaponsFactory = ini.Get_Bool(Name(), "WeaponsFactory", IsWeaponsFactory);
		IsLaserFencePost = ini.Get_Bool(Name(), "LaserFencePost", IsLaserFencePost);
		IsLaserFence = ini.Get_Bool(Name(), "LaserFence", IsLaserFence);
		IsFirestormWall = ini.Get_Bool(Name(), "FirestormWall", IsFirestormWall);
		IsHospital = ini.Get_Bool(Name(), "Hospital", IsHospital);
		IsArmory = ini.Get_Bool(Name(), "Armory", IsArmory);
		IsGDIBarracks = ini.Get_Bool(Name(), "GDIBarracks", IsGDIBarracks);
		IsNODBarracks = ini.Get_Bool(Name(), "NODBarracks", IsNODBarracks);
		IsEMPulseCannon = ini.Get_Bool(Name(), "EMPulseCannon", IsEMPulseCannon);
		IsTickTank = ini.Get_Bool(Name(), "TickTank", IsTickTank);
		SuperWeapon = ini.Get_SuperWeaponType(Name(), "SuperWeapon", SuperWeapon);
		SuperWeapon2 = ini.Get_SuperWeaponType(Name(), "SuperWeapon2", SuperWeapon2);
		IsCloakGenerator = ini.Get_Bool(Name(), "CloakGenerator", IsCloakGenerator);
		IsSensorArray = ini.Get_Bool(Name(), "SensorArray", IsSensorArray);
		CloakRadiusInCells = ini.Get_Int(Name(), "CloakRadiusInCells", CloakRadiusInCells);

		LightVisibility = ini.Get_Int(Name(), "LightVisibility", LightVisibility);
		LightIntensity = (ini.Get_Float(Name(), "LightIntensity", LightIntensity / NORMAL_LIGHT) * (double)NORMAL_LIGHT) + 0.1;
		LightRedTint = (ini.Get_Float(Name(), "LightRedTint", LightRedTint / NORMAL_LIGHT) * (double)NORMAL_LIGHT) + 0.1;
		LightGreenTint = (ini.Get_Float(Name(), "LightGreenTint", LightGreenTint / NORMAL_LIGHT) * (double)NORMAL_LIGHT) + 0.1;
		LightBlueTint = (ini.Get_Float(Name(), "LightBlueTint", LightBlueTint / NORMAL_LIGHT) * (double)NORMAL_LIGHT) + 0.1;
		GateCloseDelay = ini.Get_Float(Name(), "GateCloseDelay", GateCloseDelay);

		IsInvisibleInGame = ini.Get_Bool(Name(), "InvisibleInGame", IsInvisibleInGame);
		if (IsInvisibleInGame) {
			IsInvisible = true;
			IsRadarVisible = false;
		}

		ini.Get_String(Name(), "PowersUpBuilding", PowersUpBuilding);
		PowersUpToLevel = ini.Get_Int(Name(), "PowersUpToLevel", PowersUpToLevel);

		IsBridgeRepairHut = ini.Get_Bool(Name(), "BridgeRepairHut", IsBridgeRepairHut);
		IsHasStupidGuardMode = ini.Get_Bool(Name(), "HasStupidGuardMode", IsHasStupidGuardMode);
		IsCanPlaceAnywhere = ini.Get_Bool(Name(), "PlaceAnywhere", IsCanPlaceAnywhere);
		IsICBMLauncher = ini.Get_Bool(Name(), "ICBMLauncher", IsICBMLauncher);
		IsArtillary = ini.Get_Bool(Name(), "Artillary", IsArtillary);

		ExitCoordinate = ini.Get_Point(Name(), "ExitCoord", Point3D(ExitCoordinate));

		CanAIBuildThis = ini.Get_Bool(Name(), "AIBuildThis", CanAIBuildThis);
		IsBaseDefense = ini.Get_Bool(Name(), "IsBaseDefense", IsBaseDefense);
		IsThreatRatingNode = ini.Get_Bool(Name(), "IsThreatRatingNode", IsThreatRatingNode);
		Rotation = IsTurretEquipped ? 32 : 1;

		Power = ini.Get_Int(Name(), "Power", (Power > 0) ? Power : -Drain);
		if (Power < 0) {
			Drain = -Power;
			Power = 0;
		} else {
			Drain = 0;
		}

		ZHeight = ArtINI.Get_Int(Graphic_Name(), "Height", ZHeight);
		IsRecoilless = ArtINI.Get_Bool(Graphic_Name(), "Recoilless", IsRecoilless);
		IsFlat = ArtINI.Get_Bool(Graphic_Name(), "Flat", IsFlat);
		IsSiloDamage = ArtINI.Get_Bool(Graphic_Name(), "SiloDamage", IsSiloDamage);
		IsHasChargeAnim = ArtINI.Get_Bool(Graphic_Name(), "ChargeAnim", IsHasChargeAnim);
		ToOverlay = TGet_Class(ArtINI, Graphic_Name(), "ToOverlay", ToOverlay);

		Size = ArtINI.Get_BSizeType(Graphic_Name(), "Foundation", Size);
		BSizeType size = ArtINI.Get_BSizeType(Name(), "Foundation", Size);
		if (size != BSIZE_FIRST) {
			Size = size;
		}

		MidPoint = ArtINI.Get_Int(Graphic_Name(), "MidPoint", MidPoint);
		DoorStages = ArtINI.Get_Int(Graphic_Name(), "DoorStages", DoorStages);
		IsDamagedDoor = ArtINI.Get_Bool(Graphic_Name(), "DamagedDoor", IsDamagedDoor);
		IsTerrainPalette = ArtINI.Get_Bool(Graphic_Name(), "TerrainPalette", IsTerrainPalette);
		GateStages = ArtINI.Get_Int(Graphic_Name(), "GateStages", GateStages);
		PrimaryFirePixelOffset = ArtINI.Get_Point(Graphic_Name(), "PrimaryFirePixelOffset", PrimaryFirePixelOffset);
		SecondaryFirePixelOffset = ArtINI.Get_Point(Graphic_Name(), "SecondaryFirePixelOffset", SecondaryFirePixelOffset);
		IsExtraDamageStage = ArtINI.Get_Bool(Graphic_Name(), "ExtraDamageStage", IsExtraDamageStage);
		SpecialZOverlayZAdjust = ArtINI.Get_Int(Graphic_Name(), "SpecialZOverlayZAdjust", SpecialZOverlayZAdjust);
		NormalZAdjust = ArtINI.Get_Int(Graphic_Name(), "NormalZAdjust", NormalZAdjust);
		ZShapePointMove = ArtINI.Get_Point(Graphic_Name(), "ZShapePointMove", ZShapePointMove);
		ExtraLight = ArtINI.Get_Int(Graphic_Name(), "ExtraLight", ExtraLight);
		IsDemandLoad = ArtINI.Get_Bool(Graphic_Name(), "DemandLoad", IsDemandLoad);
		IsDemandLoadBuildup = ArtINI.Get_Bool(Graphic_Name(), "DemandLoadBuildup", IsDemandLoadBuildup);
		IsFreeBuildup = ArtINI.Get_Bool(Graphic_Name(), "FreeBuildup", IsFreeBuildup);
		if (IsDemandLoad) {
			ImageData = NULL;
		}
		if (IsDemandLoadBuildup) {
			BuildupData = NULL;
		}

		OccupyList = OccupyLists[Size];
		ExitList = ExitLists[Size];

		/*
		 * Graphics.
		 */
		ArtINI.Get_String(Graphic_Name(), "Buildup", BuildupFilename);

		BuildingTypeClass::Fetch_Building_Normal_Image(Scen->Theater);

		char defvalue[64];
		char animcontrol[64];

		/*
		 * Animation controls.
		 */
		sprintf(defvalue, "%d,%d,%d", Anims[BSTATE_IDLE].Start, Anims[BSTATE_IDLE].Count, Anims[BSTATE_IDLE].Rate);
		if (ArtINI.Get_String(Graphic_Name(), "AnimIdle", defvalue, animcontrol, sizeof(animcontrol)) > 0) {
			sscanf(animcontrol, "%d,%d,%d", &Anims[BSTATE_IDLE].Start, &Anims[BSTATE_IDLE].Count, &Anims[BSTATE_IDLE].Rate);
		}

		sprintf(defvalue, "%d,%d,%d", Anims[BSTATE_ACTIVE].Start, Anims[BSTATE_ACTIVE].Count, Anims[BSTATE_ACTIVE].Rate);
		if (ArtINI.Get_String(Graphic_Name(), "AnimActive", defvalue, animcontrol, sizeof(animcontrol)) > 0) {
			sscanf(animcontrol, "%d,%d,%d", &Anims[BSTATE_ACTIVE].Start, &Anims[BSTATE_ACTIVE].Count, &Anims[BSTATE_ACTIVE].Rate);
		}

		sprintf(defvalue, "%d,%d,%d", Anims[BSTATE_AUX1].Start, Anims[BSTATE_AUX1].Count, Anims[BSTATE_AUX1].Rate);
		if (ArtINI.Get_String(Graphic_Name(), "AnimAux1", defvalue, animcontrol, sizeof(animcontrol)) > 0) {
			sscanf(animcontrol, "%d,%d,%d", &Anims[BSTATE_AUX1].Start, &Anims[BSTATE_AUX1].Count, &Anims[BSTATE_AUX1].Rate);
		}

		sprintf(defvalue, "%d,%d,%d", Anims[BSTATE_AUX2].Start, Anims[BSTATE_AUX2].Count, Anims[BSTATE_AUX2].Rate);
		if (ArtINI.Get_String(Graphic_Name(), "AnimAux2", defvalue, animcontrol, sizeof(animcontrol)) > 0) {
			sscanf(animcontrol, "%d,%d,%d", &Anims[BSTATE_AUX2].Start, &Anims[BSTATE_AUX2].Count, &Anims[BSTATE_AUX2].Rate);
		}

		/// ActiveAnim
		ArtINI.Get_String(Graphic_Name(), "ActiveAnim", "", buffer, sizeof(((AnimDataType *)0)->Anim));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_ACTIVE_ONE].Anim, buffer);
		}

		ArtINI.Get_String(Graphic_Name(), "ActiveAnimDamaged", "", buffer, sizeof(((AnimDataType *)0)->AnimDamaged));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_ACTIVE_ONE].AnimDamaged, buffer);
		}

		if (!strlen(AnimData[BANIM_ACTIVE_ONE].AnimDamaged)) {
			strcpy(AnimData[BANIM_ACTIVE_ONE].AnimDamaged, AnimData[BANIM_ACTIVE_ONE].Anim);
		}

		if (strlen(AnimData[BANIM_ACTIVE_ONE].Anim) || strlen(AnimData[BANIM_ACTIVE_ONE].AnimDamaged)) {
			AnimData[BANIM_ACTIVE_ONE].Location.X = ArtINI.Get_Int(Graphic_Name(), "ActiveAnimX", AnimData[BANIM_ACTIVE_ONE].Location.X);
			AnimData[BANIM_ACTIVE_ONE].Location.Y = ArtINI.Get_Int(Graphic_Name(), "ActiveAnimY", AnimData[BANIM_ACTIVE_ONE].Location.Y);
			AnimData[BANIM_ACTIVE_ONE].ZAdjust = ArtINI.Get_Int(Graphic_Name(), "ActiveAnimZAdjust", AnimData[BANIM_ACTIVE_ONE].ZAdjust);
			AnimData[BANIM_ACTIVE_ONE].YSort = ArtINI.Get_Int(Graphic_Name(), "ActiveAnimYSort", AnimData[BANIM_ACTIVE_ONE].YSort);
			AnimData[BANIM_ACTIVE_ONE].Powered = ArtINI.Get_Bool(Graphic_Name(), "ActiveAnimPowered", AnimData[BANIM_ACTIVE_ONE].Powered);
			AnimData[BANIM_ACTIVE_ONE].PoweredLight = ArtINI.Get_Bool(Graphic_Name(), "ActiveAnimPoweredLight", AnimData[BANIM_ACTIVE_ONE].PoweredLight);
		}

		/// ActiveAnimTwo
		ArtINI.Get_String(Graphic_Name(), "ActiveAnimTwo", "", buffer, sizeof(((AnimDataType *)0)->Anim));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_ACTIVE_TWO].Anim, buffer);
		}

		ArtINI.Get_String(Graphic_Name(), "ActiveAnimTwoDamaged", "", buffer, sizeof(((AnimDataType *)0)->AnimDamaged));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_ACTIVE_TWO].AnimDamaged, buffer);
		}

		if (!strlen(AnimData[BANIM_ACTIVE_TWO].AnimDamaged)) {
			strcpy(AnimData[BANIM_ACTIVE_TWO].AnimDamaged, AnimData[BANIM_ACTIVE_TWO].Anim);
		}

		if (strlen(AnimData[BANIM_ACTIVE_TWO].Anim) || strlen(AnimData[BANIM_ACTIVE_TWO].AnimDamaged)) {
			AnimData[BANIM_ACTIVE_TWO].Location.X = ArtINI.Get_Int(Graphic_Name(), "ActiveAnimTwoX", AnimData[BANIM_ACTIVE_TWO].Location.X);
			AnimData[BANIM_ACTIVE_TWO].Location.Y = ArtINI.Get_Int(Graphic_Name(), "ActiveAnimTwoY", AnimData[BANIM_ACTIVE_TWO].Location.Y);
			AnimData[BANIM_ACTIVE_TWO].ZAdjust = ArtINI.Get_Int(Graphic_Name(), "ActiveAnimTwoZAdjust", AnimData[BANIM_ACTIVE_TWO].ZAdjust);
			AnimData[BANIM_ACTIVE_TWO].YSort = ArtINI.Get_Int(Graphic_Name(), "ActiveAnimTwoYSort", AnimData[BANIM_ACTIVE_TWO].YSort);
			AnimData[BANIM_ACTIVE_TWO].Powered = ArtINI.Get_Bool(Graphic_Name(), "ActiveAnimTwoPowered", AnimData[BANIM_ACTIVE_TWO].Powered);
			AnimData[BANIM_ACTIVE_TWO].PoweredLight = ArtINI.Get_Bool(Graphic_Name(), "ActiveAnimTwoPoweredLight", AnimData[BANIM_ACTIVE_TWO].PoweredLight);
		}

		/// ActiveAnimThree
		ArtINI.Get_String(Graphic_Name(), "ActiveAnimThree", "", buffer, sizeof(((AnimDataType *)0)->Anim));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_ACTIVE_THREE].Anim, buffer);
		}

		ArtINI.Get_String(Graphic_Name(), "ActiveAnimThreeDamaged", "", buffer, sizeof(((AnimDataType *)0)->AnimDamaged));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_ACTIVE_THREE].AnimDamaged, buffer);
		}

		if (!strlen(AnimData[BANIM_ACTIVE_THREE].AnimDamaged)) {
			strcpy(AnimData[BANIM_ACTIVE_THREE].AnimDamaged, AnimData[BANIM_ACTIVE_THREE].Anim);
		}

		if (strlen(AnimData[BANIM_ACTIVE_THREE].Anim) || strlen(AnimData[BANIM_ACTIVE_THREE].AnimDamaged)) {
			AnimData[BANIM_ACTIVE_THREE].Location.X = ArtINI.Get_Int(Graphic_Name(), "ActiveAnimThreeX", AnimData[BANIM_ACTIVE_THREE].Location.X);
			AnimData[BANIM_ACTIVE_THREE].Location.Y = ArtINI.Get_Int(Graphic_Name(), "ActiveAnimThreeY", AnimData[BANIM_ACTIVE_THREE].Location.Y);
			AnimData[BANIM_ACTIVE_THREE].ZAdjust = ArtINI.Get_Int(Graphic_Name(), "ActiveAnimThreeZAdjust", AnimData[BANIM_ACTIVE_THREE].ZAdjust);
			AnimData[BANIM_ACTIVE_THREE].YSort = ArtINI.Get_Int(Graphic_Name(), "ActiveAnimThreeYSort", AnimData[BANIM_ACTIVE_THREE].YSort);
			AnimData[BANIM_ACTIVE_THREE].Powered = ArtINI.Get_Bool(Graphic_Name(), "ActiveAnimThreePowered", AnimData[BANIM_ACTIVE_THREE].Powered);
			AnimData[BANIM_ACTIVE_THREE].PoweredLight = ArtINI.Get_Bool(Graphic_Name(), "ActiveAnimThreePoweredLight", AnimData[BANIM_ACTIVE_THREE].PoweredLight);
		}

		/// ActiveAnimFour
		ArtINI.Get_String(Graphic_Name(), "ActiveAnimFour", "", buffer, sizeof(((AnimDataType *)0)->Anim));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_ACTIVE_FOUR].Anim, buffer);
		}

		ArtINI.Get_String(Graphic_Name(), "ActiveAnimFourDamaged", "", buffer, sizeof(((AnimDataType *)0)->AnimDamaged));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_ACTIVE_FOUR].AnimDamaged, buffer);
		}

		if (!strlen(AnimData[BANIM_ACTIVE_FOUR].AnimDamaged)) {
			strcpy(AnimData[BANIM_ACTIVE_FOUR].AnimDamaged, AnimData[BANIM_ACTIVE_FOUR].Anim);
		}

		if (strlen(AnimData[BANIM_ACTIVE_FOUR].Anim) || strlen(AnimData[BANIM_ACTIVE_FOUR].AnimDamaged)) {
			AnimData[BANIM_ACTIVE_FOUR].Location.X = ArtINI.Get_Int(Graphic_Name(), "ActiveAnimFourX", AnimData[BANIM_ACTIVE_FOUR].Location.X);
			AnimData[BANIM_ACTIVE_FOUR].Location.Y = ArtINI.Get_Int(Graphic_Name(), "ActiveAnimFourY", AnimData[BANIM_ACTIVE_FOUR].Location.Y);
			AnimData[BANIM_ACTIVE_FOUR].ZAdjust = ArtINI.Get_Int(Graphic_Name(), "ActiveAnimFourZAdjust", AnimData[BANIM_ACTIVE_FOUR].ZAdjust);
			AnimData[BANIM_ACTIVE_FOUR].YSort = ArtINI.Get_Int(Graphic_Name(), "ActiveAnimFourYSort", AnimData[BANIM_ACTIVE_FOUR].YSort);
			AnimData[BANIM_ACTIVE_FOUR].Powered = ArtINI.Get_Bool(Graphic_Name(), "ActiveAnimFourPowered", AnimData[BANIM_ACTIVE_FOUR].Powered);
			AnimData[BANIM_ACTIVE_FOUR].PoweredLight = ArtINI.Get_Bool(Graphic_Name(), "ActiveAnimFourPoweredLight", AnimData[BANIM_ACTIVE_FOUR].PoweredLight);
		}

		/// SpecialAnim
		ArtINI.Get_String(Graphic_Name(), "SpecialAnim", "", buffer, sizeof(((AnimDataType *)0)->Anim));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_SPECIAL_ONE].Anim, buffer);
		}

		ArtINI.Get_String(Graphic_Name(), "SpecialAnimDamaged", "", buffer, sizeof(((AnimDataType *)0)->AnimDamaged));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_SPECIAL_ONE].AnimDamaged, buffer);
		}

		if (!strlen(AnimData[BANIM_SPECIAL_ONE].AnimDamaged)) {
			strcpy(AnimData[BANIM_SPECIAL_ONE].AnimDamaged, AnimData[BANIM_SPECIAL_ONE].Anim);
		}

		if (strlen(AnimData[BANIM_SPECIAL_ONE].Anim) || strlen(AnimData[BANIM_SPECIAL_ONE].AnimDamaged)) {
			AnimData[BANIM_SPECIAL_ONE].Location.X = ArtINI.Get_Int(Name(), "SpecialAnimX", AnimData[BANIM_SPECIAL_ONE].Location.X);
			AnimData[BANIM_SPECIAL_ONE].Location.Y = ArtINI.Get_Int(Name(), "SpecialAnimY", AnimData[BANIM_SPECIAL_ONE].Location.Y);
			AnimData[BANIM_SPECIAL_ONE].ZAdjust = ArtINI.Get_Int(Name(), "SpecialAnimZAdjust", AnimData[BANIM_SPECIAL_ONE].ZAdjust);
			AnimData[BANIM_SPECIAL_ONE].YSort = ArtINI.Get_Int(Name(), "SpecialAnimYSort", AnimData[BANIM_SPECIAL_ONE].YSort);
			AnimData[BANIM_SPECIAL_ONE].Powered = ArtINI.Get_Bool(Name(), "SpecialAnimPowered", AnimData[BANIM_SPECIAL_ONE].Powered);
			AnimData[BANIM_SPECIAL_ONE].PoweredLight = ArtINI.Get_Bool(Name(), "SpecialAnimPoweredLight", AnimData[BANIM_SPECIAL_ONE].PoweredLight);
		}

		/// SpecialAnimTwo
		ArtINI.Get_String(Graphic_Name(), "SpecialAnimTwo", "", buffer, sizeof(((AnimDataType *)0)->Anim));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_SPECIAL_TWO].Anim, buffer);
		}

		ArtINI.Get_String(Graphic_Name(), "SpecialAnimTwoDamaged", "", buffer, sizeof(((AnimDataType *)0)->AnimDamaged));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_SPECIAL_TWO].AnimDamaged, buffer);
		}

		if (!strlen(AnimData[BANIM_SPECIAL_TWO].AnimDamaged)) {
			strcpy(AnimData[BANIM_SPECIAL_TWO].AnimDamaged, AnimData[BANIM_SPECIAL_TWO].Anim);
		}

		if (strlen(AnimData[BANIM_SPECIAL_TWO].Anim) || strlen(AnimData[BANIM_SPECIAL_TWO].AnimDamaged)) {
			AnimData[BANIM_SPECIAL_TWO].Location.X = ArtINI.Get_Int(Name(), "SpecialAnimTwoX", AnimData[BANIM_SPECIAL_TWO].Location.X);
			AnimData[BANIM_SPECIAL_TWO].Location.Y = ArtINI.Get_Int(Name(), "SpecialAnimTwoY", AnimData[BANIM_SPECIAL_TWO].Location.Y);
			AnimData[BANIM_SPECIAL_TWO].ZAdjust = ArtINI.Get_Int(Name(), "SpecialAnimTwoZAdjust", AnimData[BANIM_SPECIAL_TWO].ZAdjust);
			AnimData[BANIM_SPECIAL_TWO].YSort = ArtINI.Get_Int(Name(), "SpecialAnimTwoYSort", AnimData[BANIM_SPECIAL_TWO].YSort);
			AnimData[BANIM_SPECIAL_TWO].Powered = ArtINI.Get_Bool(Name(), "SpecialAnimTwoPowered", AnimData[BANIM_SPECIAL_TWO].Powered);
			AnimData[BANIM_SPECIAL_TWO].PoweredLight = ArtINI.Get_Bool(Name(), "SpecialAnimTwoPoweredLight", AnimData[BANIM_SPECIAL_TWO].PoweredLight);
		}

		/// SpecialAnimThree
		ArtINI.Get_String(Graphic_Name(), "SpecialAnimThree", "", buffer, sizeof(((AnimDataType *)0)->Anim));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_SPECIAL_THREE].Anim, buffer);
		}

		ArtINI.Get_String(Graphic_Name(), "SpecialAnimThreeDamaged", "", buffer, sizeof(((AnimDataType *)0)->AnimDamaged));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_SPECIAL_THREE].AnimDamaged, buffer);
		}

		if (!strlen(AnimData[BANIM_SPECIAL_THREE].AnimDamaged)) {
			strcpy(AnimData[BANIM_SPECIAL_THREE].AnimDamaged, AnimData[BANIM_SPECIAL_THREE].Anim);
		}

		if (strlen(AnimData[BANIM_SPECIAL_THREE].Anim) || strlen(AnimData[BANIM_SPECIAL_THREE].AnimDamaged)) {
			AnimData[BANIM_SPECIAL_THREE].Location.X = ArtINI.Get_Int(Name(), "SpecialAnimThreeX", AnimData[BANIM_SPECIAL_THREE].Location.X);
			AnimData[BANIM_SPECIAL_THREE].Location.Y = ArtINI.Get_Int(Name(), "SpecialAnimThreeY", AnimData[BANIM_SPECIAL_THREE].Location.Y);
			AnimData[BANIM_SPECIAL_THREE].ZAdjust = ArtINI.Get_Int(Name(), "SpecialAnimThreeZAdjust", AnimData[BANIM_SPECIAL_THREE].ZAdjust);
			AnimData[BANIM_SPECIAL_THREE].YSort = ArtINI.Get_Int(Name(), "SpecialAnimThreeYSort", AnimData[BANIM_SPECIAL_THREE].YSort);
			AnimData[BANIM_SPECIAL_THREE].Powered = ArtINI.Get_Bool(Name(), "SpecialAnimThreePowered", AnimData[BANIM_SPECIAL_THREE].Powered);
			AnimData[BANIM_SPECIAL_THREE].PoweredLight = ArtINI.Get_Bool(Name(), "SpecialAnimThreePoweredLight", AnimData[BANIM_SPECIAL_THREE].PoweredLight);
		}

		/// ProductionAnim
		ArtINI.Get_String(Graphic_Name(), "ProductionAnim", "", buffer, sizeof(((AnimDataType *)0)->Anim));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_PRODUCTION].Anim, buffer);
		}

		ArtINI.Get_String(Graphic_Name(), "ProductionAnimDamaged", "", buffer, sizeof(((AnimDataType *)0)->AnimDamaged));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_PRODUCTION].AnimDamaged, buffer);
		}

		if (!strlen(AnimData[BANIM_PRODUCTION].AnimDamaged)) {
			strcpy(AnimData[BANIM_PRODUCTION].AnimDamaged, AnimData[BANIM_PRODUCTION].Anim);
		}

		if (strlen(AnimData[BANIM_PRODUCTION].Anim) || strlen(AnimData[BANIM_PRODUCTION].AnimDamaged)) {
			AnimData[BANIM_PRODUCTION].Location.X = ArtINI.Get_Int(Name(), "ProductionAnimX", AnimData[BANIM_PRODUCTION].Location.X);
			AnimData[BANIM_PRODUCTION].Location.Y = ArtINI.Get_Int(Name(), "ProductionAnimY", AnimData[BANIM_PRODUCTION].Location.Y);
			AnimData[BANIM_PRODUCTION].ZAdjust = ArtINI.Get_Int(Name(), "ProductionAnimZAdjust", AnimData[BANIM_PRODUCTION].ZAdjust);
			AnimData[BANIM_PRODUCTION].YSort = ArtINI.Get_Int(Name(), "ProductionAnimYSort", AnimData[BANIM_PRODUCTION].YSort);
		}

		/// PreProductionAnim
		ArtINI.Get_String(Graphic_Name(), "PreProductionAnim", "", buffer, sizeof(((AnimDataType *)0)->Anim));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_PRE_PRODUCTION].Anim, buffer);
		}

		ArtINI.Get_String(Graphic_Name(), "PreProductionAnimDamaged", "", buffer, sizeof(((AnimDataType *)0)->AnimDamaged));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_PRE_PRODUCTION].AnimDamaged, buffer);
		}

		if (!strlen(AnimData[BANIM_PRE_PRODUCTION].AnimDamaged)) {
			strcpy(AnimData[BANIM_PRE_PRODUCTION].AnimDamaged, AnimData[BANIM_PRE_PRODUCTION].Anim);
		}

		if (strlen(AnimData[BANIM_PRE_PRODUCTION].Anim) || strlen(AnimData[BANIM_PRE_PRODUCTION].AnimDamaged)) {
			AnimData[BANIM_PRE_PRODUCTION].Location.X = ArtINI.Get_Int(Name(), "PreProductionAnimX", AnimData[BANIM_PRE_PRODUCTION].Location.X);
			AnimData[BANIM_PRE_PRODUCTION].Location.Y = ArtINI.Get_Int(Name(), "PreProductionAnimY", AnimData[BANIM_PRE_PRODUCTION].Location.Y);
			AnimData[BANIM_PRE_PRODUCTION].ZAdjust = ArtINI.Get_Int(Name(), "PreProductionAnimZAdjust", AnimData[BANIM_PRE_PRODUCTION].ZAdjust);
			AnimData[BANIM_PRE_PRODUCTION].YSort = ArtINI.Get_Int(Name(), "PreProductionAnimYSort", AnimData[BANIM_PRE_PRODUCTION].YSort);
		}

		/// TurretAnim
		ini.Get_String(Name(), "TurretAnim", "", buffer, sizeof(((AnimDataType *)0)->Anim));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_TURRET].Anim, buffer);
		}

		ini.Get_String(Name(), "TurretAnimDamaged", "", buffer, sizeof(((AnimDataType *)0)->AnimDamaged));
		if (strlen(buffer)) {
			strcpy(AnimData[BANIM_TURRET].AnimDamaged, buffer);
		}

		if (!strlen(AnimData[BANIM_TURRET].AnimDamaged)) {
			strcpy(AnimData[BANIM_TURRET].AnimDamaged, AnimData[BANIM_TURRET].Anim);
		}

		AnimData[BANIM_TURRET].Location.X = ini.Get_Int(Name(), "TurretAnimX", AnimData[BANIM_TURRET].Location.X);
		AnimData[BANIM_TURRET].Location.Y = ini.Get_Int(Name(), "TurretAnimY", AnimData[BANIM_TURRET].Location.Y);
		AnimData[BANIM_TURRET].ZAdjust = ini.Get_Int(Name(), "TurretAnimZAdjust", AnimData[BANIM_TURRET].ZAdjust);
		AnimData[BANIM_TURRET].YSort = ini.Get_Int(Name(), "TurretAnimYSort", AnimData[BANIM_TURRET].YSort);
		IsTurretAnimAVoxel = ini.Get_Bool(Name(), "TurretAnimIsVoxel", IsTurretAnimAVoxel);

		char anim[64];
		char damagedanim[64];
		char locxx[64];
		char locyy[64];
		char loczz[64];
		char ysort[64];

		/// Building upgrades
		Upgrades = ini.Get_Int(Name(), "Upgrades", Upgrades);
		for (int i = 0; i < Upgrades; i++) {
			sprintf(anim, "PowerUp%01dAnim", i + 1);
			sprintf(damagedanim, "PowerUp%01dDamagedAnim", i + 1);
			sprintf(locxx, "PowerUp%01dLocXX", i + 1);
			sprintf(locyy, "PowerUp%01dLocYY", i + 1);
			sprintf(loczz, "PowerUp%01dLocZZ", i + 1);
			sprintf(ysort, "PowerUp%01dYSort", i + 1);

			ArtINI.Get_String(Graphic_Name(), anim, "", buffer, sizeof(((AnimDataType *)0)->Anim));
			if (strlen(buffer)) {
				strcpy(AnimData[i].Anim, buffer);
			}

			ArtINI.Get_String(Graphic_Name(), damagedanim, "", buffer, sizeof(((AnimDataType *)0)->AnimDamaged));
			if (strlen(buffer)) {
				strcpy(AnimData[i].AnimDamaged, buffer);
			}

			AnimData[i].Location.X = ArtINI.Get_Int(Graphic_Name(), locxx, AnimData[i].Location.X);
			AnimData[i].Location.Y = ArtINI.Get_Int(Graphic_Name(), locyy, AnimData[i].Location.Y);
			AnimData[i].ZAdjust = ArtINI.Get_Int(Graphic_Name(), loczz, AnimData[i].ZAdjust);
			AnimData[i].YSort = ArtINI.Get_Int(Graphic_Name(), ysort, AnimData[i].YSort);
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * BuildingTypeClass::Coord_Fixup -- Adjusts coordinate to be legal for assignment.            *
 *                                                                                             *
 *    This routine will adjust the specified coordinate so that it will be legal for assignment*
 *    to this building. All buildings are given a coordinate that is in the upper left corner  *
 *    of a cell. This routine will drop the fractional component of the coordinate.            *
 *                                                                                             *
 * INPUT:   coord -- The coordinate to fixup into a legal to assign value.                     *
 *                                                                                             *
 * OUTPUT:  Returns with a coordinate that can be assigned to the building.                    *
 *                                                                                             *
 * WARNINGS:   The coordinate is not examined to see if the cell is legal for placing the      *
 *             building. It merely adjusts the coordinate so that is legal at first glance.    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/14/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
Coord const BuildingTypeClass::Coord_Fixup(Coord const & coord) const
{
	Coord crd = coord;
	crd.Z = Map.Get_Height_GL(crd);
	return(crd);
}


/// <summary>
/// Can this building legally be placed at the location specified?
/// A building type flagged as placeable anywhere bypasses the usual terrain and
/// adjacency restrictions.
/// </summary>
/// <param name="pos">The cell to test placement at.</param>
/// <param name="house">The house that wishes to place the building.</param>
/// <returns>bool; Is the placement legal?</returns>
bool BuildingTypeClass::Legal_Placement(Cell const & pos, HouseClass * house) const
{
	if (IsCanPlaceAnywhere) {
		return(true);
	}
	return(BASECLASS::Legal_Placement(pos, house));
}


/// <summary>
/// Fetches the dimensions of this building in leptons.
/// </summary>
/// <returns>Returns with the width, height and vertical extent of the building.</returns>
Point3D BuildingTypeClass::Lepton_Dimensions(void) const
{
	return(Point3D(Width() * CELL_LEPTON_W, Height() * CELL_LEPTON_H, 40 * (ZHeight * 5)));
}


/// <summary>
/// Submits this building type to the CRC engine specified.
/// This routine is used by the multiplayer sync check to prove that every machine is
/// playing by an identical set of building rules.
/// </summary>
/// <param name="crc">The CRC engine to submit this object's data to.</param>
void BuildingTypeClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(GateCloseDelay);
	crc(LightVisibility);
	crc(LightIntensity);
	crc(LightRedTint);
	crc(LightGreenTint);
	crc(LightBlueTint);
	crc((char const *)BuildupFilename);
	crc((char const *)PowersUpBuilding);
	crc(Adjacent);
	crc(ToBuild);
	crc(Power);
	crc(Drain);
	crc(Size);
	crc(ZHeight);
	crc(MidPoint);
	crc(DoorStages);
	crc(&Anims, sizeof(Anims));
	crc(&AnimData, sizeof(AnimData));
	crc(Upgrades);
	crc(SpecialZOverlayZAdjust);
	crc(IsHoverPad);
	crc(IsPlug);
	crc(IsTemple);
	crc(HasSpotlight);
	crc(IsBase);
	crc(IsBibbed);
	crc(IsWall);
	crc(IsCaptureable);
	crc(IsPowered);
	crc(IsUnsellable);
	crc(IsRadar);
	crc(IsHasChargeAnim);
	crc(IsSiloDamage);
	crc(IsCanUnitRepair);
	crc(IsCanUnitReload);
	crc(IsFlat);
	crc(IsDockUnload);
	crc(IsRecoilless);
	crc(IsBridgeRepairHut);
	crc(IsGate);
	crc(IsSAM);
	crc(IsConstructionYard);
	crc(IsNukeSilo);
	crc(IsRefinery);
	crc(IsWeaponsFactory);
	crc(IsLaserFencePost);
	crc(IsLaserFence);
	crc(IsFirestormWall);
	crc(IsHelipad);
	crc(VoxelBarrelFile);
	crc(VoxelBarrelScale);
	crc(&VoxelBarrelOffsetToPitchPivotPoint, sizeof(VoxelBarrelOffsetToPitchPivotPoint));
	crc(&VoxelBarrelOffsetToRotatePivotPoint, sizeof(VoxelBarrelOffsetToRotatePivotPoint));
	crc(&VoxelBarrelOffsetToBuildingPivotPoint, sizeof(VoxelBarrelOffsetToBuildingPivotPoint));
	crc(&VoxelBarrelOffsetToBarrelEnd, sizeof(VoxelBarrelOffsetToBarrelEnd));
	crc(StartPitch);
	crc(IsLimpetMine);
	crc(IsMobileWar);
	crc(IsMobileStealth);
	crc(IsJuggernaut);
	crc(IsCoreDefender);
	crc(IsBarrelAnimAVoxel);
	crc(TurretChargeAnimRate);
	crc(IsTurretAnimExclusive);
	crc(IsExtraDamageStage);
	crc(IsHospital);
	crc(IsArmory);
	crc(GateStages);
	crc(PowersUpToLevel);
	crc(IsDamagedDoor);
	crc(IsInvisibleInGame);
	crc(IsTerrainPalette);
	crc(IsTurretAnimAVoxel);
}


/// <summary>
/// Re-attaches the artwork and shared tables this building type names.
/// The art is fetched again once the members have been read, and the footprint and exit
/// lists are re-attached from the shared tables that match this building's size.
/// </summary>
void BuildingTypeClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	Fetch_Building_Voxel_Image();
	if (IsDemandLoad) {
		ImageData = NULL;
	} else {
		Fetch_Normal_Image();
	}

	ToTile = NULL;
	OccupyList = OccupyLists[Size];
	ExitList = ExitLists[Size];

	Fetch_Building_Voxel_Image();
	Fetch_Building_Normal_Image(Scen->Theater);
}


/// <summary>
/// Lists the members this building type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void BuildingTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HeapID);
	// OccupyList -- a shared footprint table, re-attached by size as this loads.
	// BuildupData -- artwork, fetched from the mix files again as this loads.
	stream.Serialize(HalfDamageSmokeLocation1);
	stream.Serialize(HalfDamageSmokeLocation2);
	stream.Serialize(GateCloseDelay);
	stream.Serialize(LightVisibility);
	stream.Serialize(LightIntensity);
	stream.Serialize(LightRedTint);
	stream.Serialize(LightGreenTint);
	stream.Serialize(LightBlueTint);
	stream.Serialize(PrimaryFirePixelOffset);
	stream.Serialize(SecondaryFirePixelOffset);
	stream.Serialize(ToOverlay);
	// ToTile -- named by the rules, and resolved once the tile types are in place.
	stream.Serialize(BuildupFilename);
	stream.Serialize(PowersUpBuilding);
	stream.Serialize(FreeUnit);
	stream.Serialize(FoundationFace);
	stream.Serialize(Adjacent);
	stream.Serialize(ToBuild);
	stream.Serialize(ExitCoordinate);
	// ExitList -- a shared exit table, re-attached by size as this loads.
	stream.Serialize(StartFace);
	stream.Serialize(Power);
	stream.Serialize(Drain);
	stream.Serialize(Size);
	stream.Serialize(ZHeight);
	stream.Serialize(MidPoint);
	stream.Serialize(DoorStages);
	stream.Serialize(Anims);
	stream.Serialize(AnimData);
	stream.Serialize(Upgrades);
	// DeployingAnim -- artwork, fetched from the mix files again as this loads.
	// UnderDoorAnim
	// DoorAnim
	// SpecialZOverlay
	stream.Serialize(SpecialZOverlayZAdjust);
	// BibShape -- artwork, fetched from the mix files again as this loads.
	stream.Serialize(NormalZAdjust);
	stream.Serialize(AntiAirValue);
	stream.Serialize(AntiArmorValue);
	stream.Serialize(AntiInfantryValue);
	stream.Serialize(ZShapePointMove);
	stream.Serialize(DrawRect);
	stream.Serialize(ExtraLight);
	stream.Serialize(IsCanTogglePower);
	stream.Serialize(HasSpotlight);
	stream.Serialize(IsTemple);
	stream.Serialize(IsPlug);
	stream.Serialize(IsHoverPad);
	stream.Serialize(IsBase);
	stream.Serialize(IsBibbed);
	stream.Serialize(IsWall);
	stream.Serialize(IsCaptureable);
	stream.Serialize(IsPowered);
	stream.Serialize(IsUnsellable);
	stream.Serialize(IsRadar);
	stream.Serialize(IsHasChargeAnim);
	stream.Serialize(IsSiloDamage);
	stream.Serialize(IsCanUnitRepair);
	stream.Serialize(IsCanUnitReload);
	stream.Serialize(IsFlat);
	stream.Serialize(IsDockUnload);
	stream.Serialize(IsRecoilless);
	stream.Serialize(IsHasStupidGuardMode);
	stream.Serialize(IsBridgeRepairHut);
	stream.Serialize(IsGate);
	stream.Serialize(IsSAM);
	stream.Serialize(IsConstructionYard);
	stream.Serialize(IsNukeSilo);
	stream.Serialize(IsRefinery);
	stream.Serialize(IsWeeder);
	stream.Serialize(IsWeaponsFactory);
	stream.Serialize(IsLaserFencePost);
	stream.Serialize(IsLaserFence);
	stream.Serialize(IsFirestormWall);
	stream.Serialize(IsHospital);
	stream.Serialize(IsArmory);
	stream.Serialize(IsEMPulseCannon);
	stream.Serialize(IsTickTank);
	stream.Serialize(IsTurretAnimAVoxel);
	stream.Serialize(IsCloakGenerator);
	stream.Serialize(IsSensorArray);
	stream.Serialize(IsICBMLauncher);
	stream.Serialize(IsArtillary);
	stream.Serialize(IsHelipad);
	stream.Serialize(IsGDIBarracks);
	stream.Serialize(IsNODBarracks);
	stream.Serialize(SuperWeapon);
	stream.Serialize(SuperWeapon2);
	stream.Serialize(GateStages);
	stream.Serialize(PowersUpToLevel);
	stream.Serialize(VoxelBarrelFile);
	stream.Serialize(VoxelBarrelScale);
	stream.Serialize(VoxelBarrelOffsetToPitchPivotPoint);
	stream.Serialize(VoxelBarrelOffsetToRotatePivotPoint);
	stream.Serialize(VoxelBarrelOffsetToBuildingPivotPoint);
	stream.Serialize(VoxelBarrelOffsetToBarrelEnd);
	stream.Serialize(TurretChargeAnimRate);
	stream.Serialize(StartPitch);
	stream.Serialize(IsLimpetMine);
	stream.Serialize(IsMobileWar);
	stream.Serialize(IsMobileStealth);
	stream.Serialize(IsJuggernaut);
	stream.Serialize(IsCoreDefender);
	stream.Serialize(IsBarrelAnimAVoxel);
	stream.Serialize(IsTurretAnimExclusive);
	stream.Serialize(UnusedBTypeBool1);
	stream.Serialize(IsDamagedDoor);
	stream.Serialize(IsInvisibleInGame);
	stream.Serialize(IsTerrainPalette);
	stream.Serialize(IsCanPlaceAnywhere);
	stream.Serialize(IsExtraDamageStage);
	stream.Serialize(CanAIBuildThis);
	stream.Serialize(IsBaseDefense);
	stream.Serialize(CloakRadiusInCells);
	stream.Serialize(IsDemandLoad);
	stream.Serialize(IsDemandLoadBuildup);
	stream.Serialize(IsFreeBuildup);
	stream.Serialize(IsThreatRatingNode);
	stream.Serialize(TheaterImageFile);
}


/// <summary>
/// Fetches the class identifier of this object.
/// This routine is used by the save game system so that it knows what kind of object to
/// construct when the stream is read back in.
/// </summary>
/// <param name="retval">Pointer to the class ID to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT STDMETHODCALLTYPE BuildingTypeClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_BuildingTypeClass;
	return(S_OK);
}


/// <summary>
/// Fetches the building type of the name specified, creating it if necessary.
/// </summary>
/// <param name="name">The name of the building type to find.</param>
/// <returns>Returns with a pointer to the building type, newly created if it did not
/// already exist.</returns>
BuildingTypeClass * BuildingTypeClass::Find_Or_Make(char const * name)
{
	return(TFind_Or_Make<BuildingTypeClass>(name, BuildingTypes));
}


/// <summary>
/// Performs the post load game fixups for the building types.
/// This routine sizes the shared cloaking surface so that it can serve the greediest
/// cloak radius any building asks for, and resolves the tile each building leaves behind.
/// </summary>
void BuildingTypeClass::Post_Load_Game(void)
{
	int maxcloak = 0;

	for (int sindex = 0; sindex < BuildingTypes.Count(); sindex++) {
		BuildingTypeClass * classptr = BuildingTypes[sindex];
		maxcloak = std::max<int>(maxcloak, classptr->CloakRadiusInCells);
	}

	if (maxcloak != 0) {
		delete CloakingSurface;
		CloakingSurface = new BSurface(maxcloak + 16, maxcloak + 16, 1);
	}

	Post_Read_Tile_Fixup();
}


/// <summary>
/// Fetches the list of cells that objects leave this building through.
/// </summary>
/// <returns>Returns with a pointer to the exit cell list that suits this building's
/// size.</returns>
Cell const * BuildingTypeClass::Exit_List(void) const
{
	BSizeType size = Size;
	if (size == BSIZE_33_REF) {
		size = BSIZE_33;
	}
	return(ExitLists[size]);
}


/// <summary>
/// Fetches the rectangle that this building draws within.
/// This routine is used by the render logic to know how much screen area the building can
/// cover. Every piece of its art is taken into account -- the normal shapes, the
/// construction animation and the bib.
/// </summary>
/// <returns>Returns with the draw rectangle, or RECT_NONE if the building has no art.</returns>
Rect BuildingTypeClass::Get_Draw_Rect(void)
{
	int i;

	if (DrawRect != RECT_NONE) {
		return(DrawRect);
	}

	ShapeSet const * shape = (ShapeSet const *)Get_Image_Data();
	if (shape == NULL) {
		return(RECT_NONE);
	}

	int frames = shape->Get_Count();
	Rect rect = RECT_NONE;
	int width = shape->Get_Width();
	int height = shape->Get_Height();

	for (i = 0; i < frames; i++) {
		rect = Union(rect, shape->Get_Rect(i));
	}

	bool free_buildup = (BuildupData == NULL);
	ShapeSet const * buildup = (ShapeSet const *)Get_Buildup_Data();

	if (buildup != NULL) {
		frames = buildup->Get_Count();
		for (i = 0; i < frames; i++) {
			rect = Union(rect, buildup->Get_Rect(i));
		}
		if (free_buildup) {
			Free_Buildup_Data();
		}
	}

	if (BibShape != NULL) {
		Rect bibrect = BibShape->Get_Rect(0);
		rect = Union(rect, bibrect);
	}

	int x = rect.X - width / 2;
	int y = rect.Y - height / 2;
	DrawRect = Rect(x, y, rect.Width, rect.Height);

	return(DrawRect);
}


/// <summary>
/// Fetches the largest power drain in the list of building types.
/// </summary>
/// <param name="list">The list of building types to examine.</param>
/// <returns>Returns with the greatest drain found, or zero if the list holds none.</returns>
int BuildingTypeClass::Get_Max_Drain(DynamicVectorClass<BuildingTypeClass *> const & list)
{
	int maxdrain = 0;
	for (int i = 0; i < list.Count(); i++) {
		if (list[i]->Drain > maxdrain) {
			maxdrain = list[i]->Drain;
		}
	}
	return(maxdrain);
}


/// <summary>
/// Fetches the construction animation shapes for this building.
/// This routine will load the buildup art the first time it is asked for if this building
/// type was flagged to defer loading, and it prepares the construction animation timing
/// to suit whatever art it finds.
/// </summary>
/// <returns>Returns with a pointer to the buildup shape data, or NULL if there is
/// none.</returns>
void const * BuildingTypeClass::Get_Buildup_Data(void) const
{
	char fullname[MAX_PATH];

	if (BuildupData == NULL && IsDemandLoadBuildup) {
		if (!BuildupFilename.empty()) {
			_makepath(fullname, NULL, NULL, BuildupFilename, ".SHP");
			Theater_Naming_Convention(fullname, Scen->Theater);
			CCFileClass file(fullname);
			ShapeSet * data = (ShapeSet *)Load_Alloc_Data(file);
			((void const *&)BuildupData) = data;
			if (BuildupData != NULL) {
				int timedelay = 1;
				int count = ((ShapeSet const *)BuildupData)->Get_Count()/2;
				if (IsGate) {
					count = GateStages + 1;
				}
				if (count > 0) {
					timedelay = (Rule->BuildupTime * TICKS_PER_MINUTE) / count;
				}
				Init_Anim(BSTATE_CONSTRUCTION, 0, count, timedelay);
			}
		}
		return(BuildupData);
	}
	return(BuildupData);
}


/// <summary>
/// Frees the construction animation shapes for this building.
/// Only those building types that are flagged as worth discarding will actually give
/// their buildup art back.
/// </summary>
void BuildingTypeClass::Free_Buildup_Data(void)
{
	if (IsFreeBuildup && IsDemandLoadBuildup) {
		if (BuildupData != NULL) {
			Free_Demand_Loaded_Shape(BuildupData);
		}
	}
}


/// <summary>
/// Calculates the threat values for this base defense.
/// This routine rates how dangerous the structure is to aircraft, armor and infantry so
/// that the computer can weigh its base defenses against the enemy it expects to face.
/// </summary>
void BuildingTypeClass::Calculate_Base_Defense_Values(void)
{
	if (IsBaseDefense) {
		WeaponTypeClass * weapon = Get_Weapon(0)->Weapon;

		if (weapon != NULL) {
			int damage = weapon->Attack / (weapon->ROF * 0.025);

			if (weapon->Bullet->IsAntiAircraft) {
				AntiAirValue = std::min((double)Rule->MaximumBaseDefenseValue, damage * weapon->WarheadPtr->Modifier[ARMOR_STEEL]);
			}

			if (weapon->Bullet->IsAntiGround) {
				AntiArmorValue = std::min((double)Rule->MaximumBaseDefenseValue, damage * weapon->WarheadPtr->Modifier[ARMOR_STEEL]);
				AntiInfantryValue = std::min((double)Rule->MaximumBaseDefenseValue, damage * weapon->WarheadPtr->Modifier[ARMOR_NONE]);
			}
		}
	}
}


/// <summary>
/// Resolves the tile that each building type leaves behind.
/// This routine must wait until the isometric tile types have been loaded, since the
/// rule names a tile that does not exist before then.
/// </summary>
void BuildingTypeClass::Post_Read_Tile_Fixup(void)
{
	char buffer[60];

	for (int i = 0; i < BuildingTypes.Count(); i++) {
		BuildingTypeClass * building = BuildingTypes[i];
		buffer[0] = '\0';
		RuleINI->Get_String(building->Name(), "ToTile", NULL, buffer, sizeof(buffer));
		IsometricTileType ittype = IsometricTileTypeClass::From_Name(buffer);
		if (ittype >= ISOTILE_FIRST && ittype < IsometricTileTypes.Count()) {
			building->ToTile = IsometricTileTypes[ittype];
		}
	}
}


/// <summary>
/// Can this building undeploy regardless of circumstances?
/// This routine spots the building types that are allowed to pack themselves up even
/// where the usual undeploy restrictions would forbid it.
/// </summary>
/// <returns>bool; Is undeploying always permitted for this building?</returns>
bool BuildingTypeClass::Can_Always_Undeploy(void) const
{
	return(IsLimpetMine || IsMobileWar);
}


/// <summary>
/// Is this building the deployed form of a mobile unit?
/// </summary>
/// <returns>bool; Can this building undeploy back into a unit?</returns>
bool BuildingTypeClass::Is_Mobile_Deployer(void) const
{
	return(IsSensorArray || IsTickTank || IsICBMLauncher || IsArtillary || IsMobileStealth || IsJuggernaut || IsCoreDefender || IsLimpetMine);
}


/// <summary>
/// Fetches the facing this building takes when it deploys.
/// This routine is used when a mobile deployer converts between its unit and building
/// forms so that the two share a sensible orientation.
/// </summary>
/// <returns>Returns with the direction the building faces once deployed.</returns>
Dir256 BuildingTypeClass::Deploy_Facing(void) const
{
	if (!Is_Mobile_Deployer()) {
		return(DIR_S);
	}
	if (IsArtillary) {
		return(DIR_N);
	}
	if (IsSensorArray || IsTickTank || IsICBMLauncher) {
		return(DIR_E);
	}
	return(DIR_S);
}


/// <summary>
/// Fetches the RTTI type of this object.
/// </summary>
/// <returns>Returns with RTTI_BUILDINGTYPE.</returns>
RTTIType BuildingTypeClass::Fetch_RTTI(void) const
{
	return(RTTI_BUILDINGTYPE);
}


/// <summary>
/// Fetches the heap identifier of this building type.
/// </summary>
/// <returns>Returns with the index of this type within the building type heap.</returns>
int BuildingTypeClass::Fetch_Heap_ID(void) const
{
	return(HeapID);
}
