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

#include "always.h"

#include "objtype.h"

#include "sun.h"
#include "_mixfile.h"
#include "_rules.h"
#include "_theater.h"
#include "aircraft.h"
#include "airctype.h"
#include "building.h"
#include "builtype.h"
#include "ccfile.h"
#include "coord.h"
#include "globals.h"
#include "house.h"
#include "mixfile.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "shapeset.h"
#include "unittype.h"
#include "vector.h"
#include "voc.h"

#include <algorithm>

/*
**	Selected objects have a special marking box around them. This is the shapes that are
**	used for this purpose.
*/
void const * ObjectTypeClass::SelectShapes = 0;

void const * ObjectTypeClass::PipShapes = 0;
void const * ObjectTypeClass::Pip2Shapes = 0;

void const * ObjectTypeClass::TalkBubbleShapes = 0;

DynamicVectorClass<ObjectTypeClass *> ObjectTypeClass::ObjectTypes;


/***********************************************************************************************
 * ObjectTypeClass::ObjectTypeClass -- Normal constructor for object type class objects.       *
 *                                                                                             *
 *    This is the base constructor that is used when constructing the object type classes.     *
 *    Every tangible game piece type calls this constructor for the ObjectTypeClass. This      *
 *    class holds static information that is common to objects in general.                     *
 *                                                                                             *
 * INPUT:   see below...                                                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectTypeClass::ObjectTypeClass(char const * ininame) :
	BASECLASS(ininame),
	MaxSize(0),
	CrushSound(VOC_NONE),
	GraphicName(),
	AlphaGraphicName(),
	IsTheater(false),
	IsCrushable(false),
	IsStealthy(false),
	IsSelectable(true),
	IsLegalTarget(true),
	IsInsignificant(false),
	IsImmune(false),
	IsSentient(false),
	IsFootprint(true),
	IsVoxel(false),
	IsNewTheater(false),
	IsHasRadialIndicator(false),
	IsIgnoresFirestorm(false),
	RadialColor(0,0,0),
	Armor(ARMOR_NONE),
	MaxStrength(0),
	ImageData(0),
	AlphaImageData(0)
{
	GraphicName = IniName;

	AlphaGraphicName = "";

	ObjectTypes.Add(this);
}


/// <summary>
/// Destructor for the object type object.
/// This routine will detach the object type from the master list of object types, so that
/// no further code can find it by name or by index.
/// </summary>
ObjectTypeClass::~ObjectTypeClass(void)
{
	ObjectTypes.Delete(this);
}


/***********************************************************************************************
 * ObjectTypeClass::Max_Pips -- Fetches the maximum pips allowed for this object.              *
 *                                                                                             *
 *    This routine will return the maximum number of pips that can be displayed for this       *
 *    object. When dealing with generic objects, this value is always zero.                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of pip boxes (empty or otherwise) to display.              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ObjectTypeClass::Max_Pips(void) const
{
	return(0);
}


/***********************************************************************************************
 * ObjectTypeClass::Dimensions -- Gets the dimensions of the object in pixels.                 *
 *                                                                                             *
 *    This routine will fetch the dimensions of this object expressed as pixels width and      *
 *    pixels height. This information can be used to intelligently update the clipping         *
 *    rectangles.                                                                              *
 *                                                                                             *
 * INPUT:   width    -- Reference to the width variable that will be filled in.                *
 *                                                                                             *
 *          height   -- Reference to the height variable that will be filled in.               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
Point3D ObjectTypeClass::Pixel_Dimensions(void) const
{
	return(Point3D(10, 10, 10));
}


/***********************************************************************************************
 * ObjectTypeClass::Dimensions -- Gets the dimensions of the object in leptons.                *
 *                                                                                             *
 *    This routine will fetch the dimensions of this object expressed as leptons width and     *
 *    leptons height. This information can be used to intelligently update the clipping        *
 *    rectangles.                                                                              *
 *                                                                                             *
 * INPUT:   width    -- Reference to the width variable that will be filled in.                *
 *                                                                                             *
 *          height   -- Reference to the height variable that will be filled in.               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
Point3D ObjectTypeClass::Lepton_Dimensions(void) const
{
	return(Point3D(CELL_LEPTON_W, CELL_LEPTON_H, 200));
}


/***********************************************************************************************
 * ObjectTypeClass::Cost_Of -- Returns the cost to buy this unit.                              *
 *                                                                                             *
 *    This routine will return the cost to purchase this unit. This routine is expected to be  *
 *    overridden by the objects that can actually be purchased. All other object types can     *
 *    simply return zero since this value won't be used.                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the cost of the object.                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ObjectTypeClass::Cost_Of(HouseClass * house) const
{
	return(0);
}


/***********************************************************************************************
 * ObjectTypeClass::Time_To_Build -- Fetches the time to construct this object.                *
 *                                                                                             *
 *    This routine will fetch the time in takes to construct this object. Objects that can     *
 *    be constructed will override this routine in order to return a useful value.             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the time units (arbitrary) that it takes to construct this object.    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ObjectTypeClass::Time_To_Build(void) const
{
	return(0);
}


/***********************************************************************************************
 * ObjectTypeClass::Get_Cameo_Data -- Fetches pointer to cameo data for this object type.      *
 *                                                                                             *
 *    This routine will return with the cameo data pointer for this object type. It is         *
 *    expected that objects that can appear on the sidebar will override this routine in order *
 *    to provide proper cameo data pointer.                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the cameo shape data.                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void const * ObjectTypeClass::Get_Cameo_Data(void) const
{
	return(NULL);
}


/***********************************************************************************************
 * ObjectTypeClass::Occupy_List -- Returns with simple occupation list for object.             *
 *                                                                                             *
 *    This routine returns a pointer to a simple occupation list for this object. Since at     *
 *    this tier of the object class chain, the exact shape of the object is indeterminate,     *
 *    this function merely returns a single cell occupation list. This actually works for      *
 *    most vehicles.                                                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns a pointer to a simple occupation list.                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
Cell const * ObjectTypeClass::Occupy_List(bool) const
{
	static Cell const _list[] = {Cell(0, 0), REFRESH_EOL};
	return(_list);
}


/***********************************************************************************************
 * ObjectTypeClass::One_Time -- Handles one time processing for object types.                  *
 *                                                                                             *
 *    This routine is used to handle the once per game processing required for object types.   *
 *    This consists of loading any data and initializing any data tables the game requires.    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine goes to disk.                                                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/01/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void ObjectTypeClass::One_Time(void)
{
	SelectShapes = MFCD::Retrieve("SELECT.SHP");
	PipShapes = MFCD::Retrieve("PIPS.SHP");
	Pip2Shapes = MFCD::Retrieve("PIPS2.SHP");
	TalkBubbleShapes = MFCD::Retrieve("TALKBUBL.SHP");
}


/***********************************************************************************************
 * ObjectTypeClass::Who_Can_Build_Me -- Determine what building can build this object type.    *
 *                                                                                             *
 *    This routine will scan through all available factory buildings and determine which       *
 *    is capable of building this object type. The scan can be controlled to scan for only     *
 *    factory buildings that are free to produce now or those that could produce this          *
 *    object type if conditions permit.                                                        *
 *                                                                                             *
 * INPUT:   intheory -- Should the general (when conditions permit) case be examined to see    *
 *                      if a building could build this object type "in theory" even though it  *
 *                      might currently be otherwise occupied?                                 *
 *                                                                                             *
 *          legal    -- Check for building prerequisite and technology level rules? Usually    *
 *                      this would be 'true' for human controlled requests and 'false' for     *
 *                      the computer. This is because the computer is usually not under        *
 *                      the normal restrictions that the player is under.                      *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the building that can produce the object of this         *
 *          type. If no suitable factory building could be found, then NULL is returned.       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
BuildingClass * ObjectTypeClass::Who_Can_Build_Me(bool intheory, bool needsnopower, bool legal, HouseClass * house) const
{
	BuildingClass * freebuilding = NULL;
	BuildingClass * anybuilding = NULL;
	int ownable = Get_Ownable();

	for (int index = 0; index < Buildings.Count(); index++) {
		BuildingClass * building = Buildings[index];
		assert(building != NULL);

		if (!building->IsInLimbo &&
			building->House == house &&
			building->Class->ToBuild == RTTI &&
			(!needsnopower || building->IsOn) &&
			building->Mission != MISSION_DECONSTRUCTION && building->MissionQueue != MISSION_DECONSTRUCTION &&
			(!legal || building->House->Can_Build(this, true, true) > 0) &&
			(building->Class->Get_Ownable() & ownable) &&
			(!Rule->BuildConst.Is_In_List(building->Class) || Rule->IsMultiMCV || (building->ActLike != HOUSE_NONE && ((1L << building->ActLike) & ownable) != 0))) {

			/*
			**	HACK ALERT: Helipads can build aircraft and airstrips can build
			**	fixed wing craft only.
			*/
			if (intheory || !building->In_Radio_Contact() || RTTI != RTTI_AIRCRAFTTYPE) {
				if (building->IsLeader) return(building);
				freebuilding = building;
			} else {
				AircraftTypeClass * air = (AircraftTypeClass *)this;
				if (air->RTTI == RTTI_AIRCRAFTTYPE) {
					anybuilding = building;
				}
			}
		}
	}

	if (freebuilding != NULL) {
		return(freebuilding);
	}

	return(anybuilding);
}


/// <summary>
/// Fetches the voxel artwork for this object type.
/// This routine will load the voxel model and the motion data that animates it, along
/// with the turret, barrel, or weapon models that belong with it. If any piece of the set
/// is missing or refuses to load, the whole set is thrown away so that the object type is
/// left with no voxel artwork at all.
/// </summary>
void ObjectTypeClass::Fetch_Voxel_Image(void)
{
	char buffer[256];
	char name[260];

	bool failed = false;

	_makepath(name, 0, 0, (const char *)GraphicName, ".VXL");
	CCFileClass vxl(name);

	if (vxl.Is_Available()) {
		delete Voxel.VoxLib;
		Voxel.VoxLib = new VoxelLibrary(vxl);
		if (Voxel.VoxLib == NULL || Voxel.VoxLib->Load_Failed()) {
			failed = true;
		}
		_makepath(name, 0, 0, (const char *)GraphicName, ".HVA");
		CCFileClass hva(name);
		//if (hva.Is_Available()) {
			delete Voxel.MotLib;
			Voxel.MotLib = new MotionLibrary(hva);
			if (Voxel.MotLib == NULL || Voxel.MotLib->Load_Failed()) {
				failed = true;
			} else {
				Voxel.MotLib->Scale(Voxel.VoxLib->Get_Layer_Info(0, 0).Scale);
			}
		//}
	} else {
		failed = true;
	}

	UnitTypeClass *utype = (UnitTypeClass *)this;

	if (utype->RTTI != RTTI_UNITTYPE || utype->IsTurretEquipped) {
		sprintf(buffer, "%sTUR", (const char *)utype->GraphicName);
		_makepath(name, 0, 0, buffer, ".VXL");
		CCFileClass tvxl(name);

		if (tvxl.Is_Available()) {
			delete utype->AuxVoxel.VoxLib;
			utype->AuxVoxel.VoxLib = new VoxelLibrary(tvxl);

			if (AuxVoxel.VoxLib == NULL || AuxVoxel.VoxLib->Load_Failed()) {
				failed = true;
			}

			sprintf(buffer, "%sTUR", (const char *)utype->GraphicName);
			_makepath(name, 0, 0, buffer, ".HVA");
			CCFileClass thva(name);

			//if (thva.Is_Available()) {
				delete utype->AuxVoxel.MotLib;
				utype->AuxVoxel.MotLib = new MotionLibrary(thva);
				if (AuxVoxel.MotLib == NULL || AuxVoxel.MotLib->Load_Failed()) {
					failed = true;
				} else {
					AuxVoxel.MotLib->Scale(AuxVoxel.VoxLib->Get_Layer_Info(0, 0).Scale);
				}
			//}
		}
	} else if (strcmp((const char *)utype->IniName, "APC") == 0) {
		sprintf(buffer, "%sW", (const char *)utype->GraphicName);
		_makepath(name, 0, 0, buffer, ".VXL");
		CCFileClass wvxl(name);

		if (wvxl.Is_Available()) {
			delete utype->AuxVoxel.VoxLib;
			utype->AuxVoxel.VoxLib = new VoxelLibrary(wvxl);

			if (AuxVoxel.VoxLib == NULL || AuxVoxel.VoxLib->Load_Failed()) {
				failed = true;
			}
			sprintf(buffer, "%sW", (const char *)utype->GraphicName);
			_makepath(name, 0, 0, buffer, ".HVA");
			CCFileClass whva(name);

			//if (whva.Is_Available()) {
				delete utype->AuxVoxel.MotLib;
				utype->AuxVoxel.MotLib = new MotionLibrary(whva);
				if (AuxVoxel.MotLib == NULL || AuxVoxel.MotLib->Load_Failed()) {
					failed = true;
				} else {
					AuxVoxel.MotLib->Scale(AuxVoxel.VoxLib->Get_Layer_Info(0, 0).Scale);
				}
			//}
		}
	}

	if (utype->RTTI != RTTI_UNITTYPE || utype->IsTurretEquipped) {
		sprintf(buffer, "%sBARL", (const char *)utype->GraphicName);
		_makepath(name, 0, 0, buffer, ".VXL");
		CCFileClass bvxl(name);

		if (bvxl.Is_Available()) {
			delete utype->AuxVoxel2.VoxLib;
			utype->AuxVoxel2.VoxLib = new VoxelLibrary(bvxl);

			if (AuxVoxel2.VoxLib == NULL || AuxVoxel2.VoxLib->Load_Failed()) {
				failed = true;
			}

			sprintf(buffer, "%sBARL", (const char *)utype->GraphicName);
			_makepath(name, 0, 0, buffer, ".HVA");
			CCFileClass bhva(name);

			//if (bhva.Is_Available()) {
				delete utype->AuxVoxel2.MotLib;
				utype->AuxVoxel2.MotLib = new MotionLibrary(bhva);
				if (AuxVoxel2.MotLib == NULL || AuxVoxel2.MotLib->Load_Failed()) {
					failed = true;
				} else {
					AuxVoxel2.MotLib->Scale(AuxVoxel2.VoxLib->Get_Layer_Info(0, 0).Scale);
				}
			//}
		}
	}
	if (!failed) {
		int largest = Voxel.VoxLib->Get_Layer_Info(0, 0).XSize;
		for (int i = 0; i < (int)Voxel.VoxLib->Get_Layer_Count(); i++) {
			if (largest <= Voxel.VoxLib->Get_Layer_Info(i, 0).XSize) {
				largest = Voxel.VoxLib->Get_Layer_Info(i, 0).XSize;
			}
			if (largest <= Voxel.VoxLib->Get_Layer_Info(i, 0).YSize) {
				largest = Voxel.VoxLib->Get_Layer_Info(i, 0).YSize;
			}
			if (largest <= Voxel.VoxLib->Get_Layer_Info(i, 0).ZSize) {
				largest = Voxel.VoxLib->Get_Layer_Info(i, 0).ZSize;
			}
		}
		MaxSize = std::max(largest, 8);
		Clear_Voxel_Index();
	} else {
		delete Voxel.VoxLib;
		delete Voxel.MotLib;
		delete AuxVoxel.VoxLib;
		delete AuxVoxel.MotLib;
		Voxel.VoxLib = NULL;
		Voxel.MotLib = NULL;
		AuxVoxel.VoxLib = NULL;
		AuxVoxel.MotLib = NULL;
	}
}


/// <summary>
/// Fetches the turret and barrel voxels for this object type.
/// This routine is used to attach the auxiliary artwork to a unit that already has its
/// main voxel loaded. Object types that carry no turret have nothing to fetch and are
/// left alone.
/// </summary>
void ObjectTypeClass::Fetch_Aux_Voxel_Image(void)
{
	char buffer[256];
	char name[260];

	if (RTTI == RTTI_UNITTYPE) {
		UnitTypeClass *utype = (UnitTypeClass *)this;

		if (utype->Get_Image_Data() != NULL && utype->IsTurretEquipped) {
			sprintf(buffer, "%sTUR", (const char *)utype->GraphicName);
			_makepath(name, 0, 0, buffer, ".VXL");
			CCFileClass tvxl(name);

			if (tvxl.Is_Available()) {
				delete utype->AuxVoxel.VoxLib;
				utype->AuxVoxel.VoxLib = new VoxelLibrary(tvxl);

				if (utype->AuxVoxel.VoxLib && !utype->AuxVoxel.VoxLib->Load_Failed()) {

					sprintf(buffer, "%sTUR", (const char *)utype->GraphicName);
					_makepath(name, 0, 0, buffer, ".HVA");
					CCFileClass thva(name);

					//if (thva.Is_Available()) {
						delete utype->AuxVoxel.MotLib;
						utype->AuxVoxel.MotLib = new MotionLibrary(thva);
						utype->AuxVoxel.MotLib->Scale(utype->AuxVoxel.VoxLib->Get_Layer_Info(0, 0).Scale);
					//}
				}
			}
			sprintf(buffer, "%sBARL", (const char *)utype->GraphicName);
			_makepath(name, 0, 0, buffer, ".VXL");
			CCFileClass bvxl(name);

			if (bvxl.Is_Available()) {
				delete utype->AuxVoxel2.VoxLib;
				utype->AuxVoxel2.VoxLib = new VoxelLibrary(bvxl);

				if (utype->AuxVoxel2.VoxLib && !utype->AuxVoxel2.VoxLib->Load_Failed()) {

					sprintf(buffer, "%sBARL", (const char *)utype->GraphicName);
					_makepath(name, 0, 0, buffer, ".HVA");
					CCFileClass bhva(name);

					//if (bhva.Is_Available()) {
						delete utype->AuxVoxel2.MotLib;
						utype->AuxVoxel2.MotLib = new MotionLibrary(bhva);
						utype->AuxVoxel2.MotLib->Scale(utype->AuxVoxel2.VoxLib->Get_Layer_Info(0, 0).Scale);
					//}
				}
			}
		}
	}
}


/// <summary>
/// Fetches the shape artwork for this object type.
/// This routine will work out the artwork name that suits the current theater and then
/// attach the matching shape data to the object type. A type whose artwork is not present
/// is left with no image, which the drawing code copes with.
/// </summary>
void ObjectTypeClass::Fetch_Normal_Image(void)
{
	char fullname[260];
	_makepath(fullname, NULL, NULL, GraphicName, ".SHP");

	if (IsTheater) {
		_makepath(fullname, NULL, NULL, GraphicName, TheaterClass::As_Reference(Scen->Theater).Suffix);
	} else if (IsNewTheater) {
		Theater_Naming_Convention(fullname, Scen->Theater);
	}

	ShapeSet const * image = (ShapeSet const *)MFCD::Retrieve(fullname);
	if (image) {
		ImageData = image;
		int maxsize = std::max(image->Get_Width(), image->Get_Height());
		maxsize = std::max(maxsize, 8);
		MaxSize = maxsize;
	}
}


/// <summary>
/// Fetches the object type's data from the INI database.
/// This routine will read the attributes common to every object -- armor, strength,
/// crushability, and the targeting flags -- from the rules database, and the artwork
/// settings from the art database. Types that are not voxels have their shape artwork
/// attached as part of the process.
/// </summary>
/// <returns>bool; Was the object type's section found and read?</returns>
bool ObjectTypeClass::Read_INI(CCINIClass const & ini)
{
	char path[_MAX_PATH];

	if (BASECLASS::Read_INI(ini)) {

		ini.Get_String(IniName, "Image", GraphicName);
		ini.Get_String(IniName, "AlphaImage", AlphaGraphicName);

		CrushSound = ini.Get_VocType(IniName, "CrushSound", CrushSound);

		IsCrushable = ini.Get_Bool(IniName, "Crushable", IsCrushable);
		IsStealthy = ini.Get_Bool(IniName, "RadarInvisible", IsStealthy);
		IsSelectable = ini.Get_Bool(IniName, "Selectable", IsSelectable);
		IsLegalTarget = ini.Get_Bool(IniName, "LegalTarget", IsLegalTarget);
		Armor = ini.Get_ArmorType(IniName, "Armor", Armor);
		MaxStrength = ini.Get_Int(IniName, "Strength", MaxStrength);
		IsImmune = ini.Get_Bool(IniName, "Immune", IsImmune);
		IsInsignificant = ini.Get_Bool(IniName, "Insignificant", IsInsignificant);
		IsHasRadialIndicator = ini.Get_Bool(IniName, "HasRadialIndicator", IsHasRadialIndicator);

		RadialColor = ini.Get_RGBClass(IniName, "RadialColor", RadialColor);

		IsIgnoresFirestorm = ini.Get_Bool(IniName, "IgnoresFirestorm", IsIgnoresFirestorm);
		IsTheater = ArtINI.Get_Bool(GraphicName, "Theater", IsTheater);
		IsNewTheater = ArtINI.Get_Bool(GraphicName, "NewTheater", IsNewTheater);

		IsVoxel = ArtINI.Get_Bool(GraphicName, "Voxel", IsVoxel);
		if (!IsVoxel) {
			Fetch_Normal_Image();
		}

		if (!AlphaGraphicName.empty()) {
			_makepath(path, NULL, NULL, AlphaGraphicName, ".SHP");
			AlphaImageData = MixFileClass::Retrieve(path);
		}

		return(true);
	}
	return(false);
}


/// <summary>
/// Converts an artwork name to the current theater's spelling.
/// Only a name whose second letter is already some theater's image letter follows this
/// convention, so the civilian artwork that carries NewTheater= in error is left alone.
/// </summary>
/// <param name="name">The artwork name to adjust in place.</param>
void ObjectTypeClass::Theater_Naming_Convention(char * name, TheaterType theater) const
{
	char letter = TheaterClass::As_Reference(theater).ImageLetter;

	if (letter == '\0' || name[0] == '\0' || name[1] == '\0') {
		return;
	}

	for (int index = 0; index < Theaters.Count(); index++) {
		if (toupper((unsigned char)name[1]) == toupper((unsigned char)Theaters[index]->ImageLetter)) {
			name[1] = letter;
			return;
		}
	}
}


/// <summary>
/// Lists the members every object type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void ObjectTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(RadialColor);
	stream.Serialize(Armor);
	stream.Serialize(MaxStrength);
	// ImageData -- artwork, loaded on demand.
	// AlphaImageData -- artwork, fetched again by name in Post_Load.
	// Voxel -- the voxel model and its motion data, loaded on demand.
	// AuxVoxel
	// AuxVoxel2
	stream.Serialize(MaxSize);
	stream.Serialize(CrushSound);
	stream.Serialize(GraphicName);
	stream.Serialize(AlphaGraphicName);
	stream.Serialize(IsTheater);
	stream.Serialize(IsCrushable);
	stream.Serialize(IsStealthy);
	stream.Serialize(IsSelectable);
	stream.Serialize(IsLegalTarget);
	stream.Serialize(IsInsignificant);
	stream.Serialize(IsImmune);
	stream.Serialize(IsSentient);
	stream.Serialize(IsFootprint);
	stream.Serialize(IsVoxel);
	stream.Serialize(IsNewTheater);
	stream.Serialize(IsHasRadialIndicator);
	stream.Serialize(IsIgnoresFirestorm);
	// VoxelIndex -- caches rendered from the voxel artwork, built again as it is drawn.
	// AuxVoxelIndex
	// ShadowVoxelIndex
	// AuxVoxel2Index
}


/// <summary>
/// Re-attaches the artwork this object type names.
/// Whatever artwork the type was holding is discarded first, and then the alpha shape
/// is fetched again by name. The voxel libraries are deliberately left empty, since
/// they are fetched again on demand.
/// </summary>
void ObjectTypeClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	Clear_Voxel_Index();

	delete Voxel.VoxLib;
	Voxel.VoxLib = NULL;

	delete Voxel.MotLib;
	Voxel.MotLib = NULL;

	delete AuxVoxel.VoxLib;
	AuxVoxel.VoxLib = NULL;

	delete AuxVoxel.MotLib;
	AuxVoxel.MotLib = NULL;

	delete AuxVoxel2.VoxLib;
	AuxVoxel2.VoxLib = NULL;

	delete AuxVoxel2.MotLib;
	AuxVoxel2.MotLib = NULL;

	if (!AlphaGraphicName.empty()) {
		char filename[_MAX_FNAME + _MAX_EXT];
		_makepath(filename, NULL, NULL, AlphaGraphicName, ".SHP");
		AlphaImageData = MFCD::Retrieve(filename);
	}
}


/***********************************************************************************************
 * ObjectTypeClass::From_Name -- Converts an ASCII name into an object type pointer.           *
 *                                                                                             *
 *    This routine is used to convert an ASCII representation of an object into the            *
 *    matching object type pointer. This is used by the scenario INI reader code.              *
 *                                                                                             *
 * INPUT:   name  -- Pointer to ASCII name to translate.                                       *
 *                                                                                             *
 * OUTPUT:  Returns the object type poiner that matches the ASCII name provided. If no         *
 *          match could be found, then NULL is returned.                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectTypeClass const * ObjectTypeClass::From_Name(char const * name)
{
	for (int classid = 0; classid < ObjectTypes.Count(); classid++) {
		if (stricmp(ObjectTypes[classid]->IniName, name) == 0) {
			return(ObjectTypes[classid]);
		}
	}
	return(NULL);
}


/// <summary>
/// Discards the cached voxel renderings of every object type.
/// This routine is used by the voxel drawing code when the shared render buffer can no
/// longer take another entry. Throwing away every type's cached images frees the buffer
/// so that drawing can carry on.
/// </summary>
void ObjectTypeClass::Clear_Voxel_Indexes(void)
{
	for (int classid = 0; classid < ObjectTypes.Count(); classid++) {
		ObjectTypes[classid]->Clear_Voxel_Index();
	}

	VoxelStaticBuffer.Reset();
}
