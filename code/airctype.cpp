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

/* $Header: /CounterStrike/AADATA.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : AADATA.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : July 22, 1994                                                *
 *                                                                                             *
 *                  Last Update : July 9, 1996 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   AircraftTypeClass::AircraftTypeClass -- Constructor for aircraft objects.                 *
 *   AircraftTypeClass::As_Reference -- Given an aircraft type, find the matching type object. *
 *   AircraftTypeClass::Create_And_Place -- Creates and places aircraft using normal game syste*
 *   AircraftTypeClass::Create_One_Of -- Creates an aircraft object of the appropriate type.   *
 *   AircraftTypeClass::Dimensions -- Fetches the graphic dimensions of the aircraft type.     *
 *   AircraftTypeClass::Display -- Displays a generic version of the aircraft type.            *
 *   AircraftTypeClass::From_Name -- Converts an ASCII name into an aircraft type number.      *
 *   AircraftTypeClass::Init_Heap -- Initialize the aircraft type class heap.                  *
 *   AircraftTypeClass::Max_Pips -- Fetches the maximum number of pips allowed.                *
 *   AircraftTypeClass::Occupy_List -- Returns with occupation list for landed aircraft.       *
 *   AircraftTypeClass::One_Time -- Performs one time initialization of the aircraft type class*
 *   AircraftTypeClass::Overlap_List -- Determines the overlap list for a landed aircraft.     *
 *   AircraftTypeClass::Prep_For_Add -- Prepares the scenario editor for adding an aircraft obj*
 *   AircraftTypeClass::operator delete -- Returns aircraft type to special memory pool.       *
 *   AircraftTypeClass::operator new -- Allocates an aircraft type object from special pool.   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "airctype.h"

#include "_mixfile.h"
#include "_rules.h"
#include "aircraft.h"
#include "animtype.h"
#include "findmake.h"
#include "globals.h"
#include "incdec.h"
#include "mixfile.h"
#include "rules.h"
#include "savestream.h"
#include "sun.h"
#include "tracker.h"


void const * AircraftTypeClass::LRotorData = NULL;
void const * AircraftTypeClass::RRotorData = NULL;


/***********************************************************************************************
 * AircraftTypeClass::AircraftTypeClass -- Constructor for aircraft objects.                   *
 *                                                                                             *
 *    This is the constructor for the aircraft object.                                         *
 *                                                                                             *
 * INPUT:   see below...                                                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
AircraftTypeClass::AircraftTypeClass(char const* ininame) :
	BASECLASS(ininame, SPEED_WINGED),
	IsLandable(false),
	IsRotorEquipped(false),
	RotorCount(1),
	Rotor1Data(NULL),
	Rotor2Data(NULL),
	Rotor3Data(NULL),
	Rotor4Data(NULL),
	Rotor1TiltedData(NULL),
	Rotor2TiltedData(NULL),
	Rotor3TiltedData(NULL),
	Rotor4TiltedData(NULL),
	IsCarryall(false),
	HeapID(AIRCRAFT_NONE)
{
	Create_ID();

	Rotor1FLH = Point3D(0, 0, 0);
	Rotor2FLH = Point3D(0, 0, 0);
	Rotor3FLH = Point3D(0, 0, 0);
	Rotor4FLH = Point3D(0, 0, 0);

	AircraftTypes.Add(this);
	HeapID = AircraftType(AircraftTypes.ID(this));

	IsMoveToShroud = false;
	Rotation = 32;

	Rotor1File[0] = '\0';
	Rotor2File[0] = '\0';
	Rotor3File[0] = '\0';
	Rotor4File[0] = '\0';

	char(*tiltedFiles[4])[15 + 1] = {
		Rotor1TiltedFiles,
		Rotor2TiltedFiles,
		Rotor3TiltedFiles,
		Rotor4TiltedFiles
	};

	void const** tiltedData[4] = {
		Rotor1TiltedData,
		Rotor2TiltedData,
		Rotor3TiltedData,
		Rotor4TiltedData
	};

	for (int rotor = 0; rotor < 4; ++rotor) {
		for (int direction = 0; direction < 8; ++direction) {
			tiltedFiles[rotor][direction][0] = '\0';
			tiltedData[rotor][direction] = NULL;
		}
	}
}


/// <summary>
/// Destroys the aircraft type.
/// Any object still pointing at this type is told to forget it, and the type is removed
/// from the aircraft type heap so that nothing can find it again.
/// </summary>
AircraftTypeClass::~AircraftTypeClass(void)
{
	Detach_This_From_All(this, true);
	AircraftTypes.Delete(this);
}


/***********************************************************************************************
 * AircraftTypeClass::From_Name -- Converts an ASCII name into an aircraft type number.        *
 *                                                                                             *
 *    This routine is used to convert an ASCII representation of an aircraft into the          *
 *    matching aircraft type number. This is used by the scenario INI reader code.             *
 *                                                                                             *
 * INPUT:   name  -- Pointer to ASCII name to translate.                                       *
 *                                                                                             *
 * OUTPUT:  Returns the aircraft type number that matches the ASCII name provided. If no       *
 *          match could be found, then AIRCRAFT_NONE is returned.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
AircraftType AircraftTypeClass::From_Name(char const * name)
{
	if (name != NULL) {
		for (AircraftType classid = AIRCRAFT_FIRST; classid < AircraftTypes.Count(); classid++) {
			if (stricmp(AircraftTypes[classid]->Name(), name) == 0) {
				return(classid);
			}
		}
	}
	return(AIRCRAFT_NONE);
}


/***********************************************************************************************
 * AircraftTypeClass::One_Time -- Performs one time initialization of the aircraft type class. *
 *                                                                                             *
 *    This routine is used to perform the onetime initialization of the aircraft type. This    *
 *    includes primarily the shape and other graphic data loading.                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This goes to disk and also must only be called ONCE.                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftTypeClass::One_Time(void)
{
	LRotorData = MFCD::Retrieve("LROTOR.SHP");
	RRotorData = MFCD::Retrieve("RROTOR.SHP");
}


/***********************************************************************************************
 * AircraftTypeClass::Create_One_Of -- Creates an aircraft object of the appropriate type.     *
 *                                                                                             *
 *    This routine is used to create an aircraft object that matches the aircraft type. It     *
 *    serves as a shortcut to creating an object using the "new" operator and "if" checks.     *
 *                                                                                             *
 * INPUT:   house -- The house owner of the aircraft that is to be created.                    *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the aircraft created. If the aircraft could not be       *
 *          created, then a NULL is returned.                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectClass * AircraftTypeClass::Create_One_Of(HouseClass * house) const
{
	return(new AircraftClass(this, house));
}


/***********************************************************************************************
 * AircraftTypeClass::Occupy_List -- Returns with occupation list for landed aircraft.         *
 *                                                                                             *
 *    This determines the occupation list for the aircraft (if it was landed).                 *
 *                                                                                             *
 * INPUT:   placement   -- Is this for placement legality checking only? The normal condition  *
 *                         is for marking occupation flags.                                    *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to a cell offset occupation list for the aircraft.          *
 *                                                                                             *
 * WARNINGS:   This occupation list is only valid if the aircraft is landed.                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
Cell const * AircraftTypeClass::Occupy_List(bool) const
{
	static Cell const _list[] = {Cell(0,0), REFRESH_EOL};
	return(_list);
}


/***********************************************************************************************
 * AircraftTypeClass::Create_And_Place -- Creates and places aircraft using normal game system *
 *                                                                                             *
 *    This routine is used to create and place an aircraft through the normal game system.     *
 *    Since creation of aircraft in this fashion is prohibited, this routine does nothing.     *
 *                                                                                             *
 * INPUT:   na                                                                                 *
 *                                                                                             *
 * OUTPUT:  Always returns a failure code (false).                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/07/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool AircraftTypeClass::Create_And_Place(Cell const &, HouseClass *) const
{
	return(false);
}


/***********************************************************************************************
 * AircraftTypeClass::Dimensions -- Fetches the graphic dimensions of the aircraft type.       *
 *                                                                                             *
 *    This routine will fetch the pixel dimensions of this aircraft type. These dimensions     *
 *    are used to control map refresh and select box rendering.                                *
 *                                                                                             *
 * INPUT:   width    -- Reference to variable that will be filled in with aircraft width.      *
 *                                                                                             *
 *          height   -- Reference to variable that will be filled in with aircraft height.     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/07/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
Point3D AircraftTypeClass::Lepton_Dimensions(void) const
{
	return(Point3D(CELL_LEPTON_W, CELL_LEPTON_H, 200));
}


/***********************************************************************************************
 * AircraftTypeClass::Read_INI -- Fetches aircraft override values from the INI database.      *
 *                                                                                             *
 *    This routine will retrieve the override values for this aircraft type class object from  *
 *    the INI database specified.                                                              *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database to retrieve the data from.                  *
 *                                                                                             *
 * OUTPUT:  bool; Was the aircraft section for this type found and data retrieved from it?     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool AircraftTypeClass::Read_INI(CCINIClass const& ini)
{
	if (BASECLASS::Read_INI(ini)) {

		IsLandable = ini.Get_Bool(Name(), "Landable", IsLandable);
		IsCarryall = ini.Get_Bool(Name(), "Carryall", IsCarryall);
		IsRotorEquipped = ArtINI.Get_Bool(Graphic_Name(), "Rotors", IsRotorEquipped);
		RotorCount = ArtINI.Get_Int(Graphic_Name(), "RotorCount", RotorCount);
		Rotor1FLH = ArtINI.Get_Point(Graphic_Name(), "Rotor1FLH", Rotor1FLH);
		Rotor2FLH = ArtINI.Get_Point(Graphic_Name(), "Rotor2FLH", Rotor2FLH);
		Rotor3FLH = ArtINI.Get_Point(Graphic_Name(), "Rotor3FLH", Rotor3FLH);
		Rotor4FLH = ArtINI.Get_Point(Graphic_Name(), "Rotor4FLH", Rotor4FLH);
		ArtINI.Get_String(Graphic_Name(), "Rotor1Anim", "", Rotor1File, sizeof(Rotor1File));
		ArtINI.Get_String(Graphic_Name(), "Rotor2Anim", "", Rotor2File, sizeof(Rotor2File));
		ArtINI.Get_String(Graphic_Name(), "Rotor3Anim", "", Rotor3File, sizeof(Rotor3File));
		ArtINI.Get_String(Graphic_Name(), "Rotor4Anim", "", Rotor4File, sizeof(Rotor4File));

		/*
		** Read the directional rotor filenames for each rotor.
		*/
		{
			const int ROTOR_COUNT = 4;				// Max number of rotors
			const int ROTOR_DIRECTION_COUNT = 8;	// Number of directions expected in list
			const int ROTOR_FILENAME_LENGTH = 15;

			char(*tiltedFiles[ROTOR_COUNT])[ROTOR_FILENAME_LENGTH + 1] = {
				Rotor1TiltedFiles,
				Rotor2TiltedFiles,
				Rotor3TiltedFiles,
				Rotor4TiltedFiles
			};

			char tiltednames[
				ROTOR_DIRECTION_COUNT * ROTOR_FILENAME_LENGTH + (ROTOR_DIRECTION_COUNT - 1) + 1
			];

			for (int rotor = 0; rotor < ROTOR_COUNT; ++rotor) {
				for (int direction = 0; direction < ROTOR_DIRECTION_COUNT; ++direction) {
					tiltedFiles[rotor][direction][0] = '\0';
				}
				tiltednames[0] = '\0';

				int direction = 0;
				switch (rotor) {
					case 0:
						ArtINI.Get_String(
							Graphic_Name(), "Rotor1TiltedAnims", "", tiltednames, sizeof(tiltednames));
						break;
					case 1:
						ArtINI.Get_String(
							Graphic_Name(), "Rotor2TiltedAnims", "", tiltednames, sizeof(tiltednames));
						break;
					case 2:
						ArtINI.Get_String(
							Graphic_Name(), "Rotor3TiltedAnims", "", tiltednames, sizeof(tiltednames));
						break;
					case 3:
						ArtINI.Get_String(
							Graphic_Name(), "Rotor4TiltedAnims", "", tiltednames, sizeof(tiltednames));
						break;
				}

				char* filename = tiltednames;

				while (*filename != '\0' && direction < ROTOR_DIRECTION_COUNT) {
					char* comma = strchr(filename, ',');

					if (comma != NULL) {
						*comma = '\0';
					}

					size_t length = strlen(filename);

					if (length > 0 && length <= ROTOR_FILENAME_LENGTH) {
						strcpy(tiltedFiles[rotor][direction], filename);
					}

					direction++;

					if (comma == NULL) {
						break;
					}

					filename = comma + 1;
				}
			}
		}
		Load_Rotor_Assets();
		return(true);
	}
	return(false);
}


/// <summary>
/// Adds the aircraft type to a running checksum.
/// This routine is used by the network code to prove that every machine in a multiplayer
/// game agrees about the rules. Anything that differs between machines must be folded in
/// here or a desynchronization will go unnoticed.
/// </summary>
/// <param name="crc">The checksum engine to submit the type's data to.</param>
void AircraftTypeClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);

	crc(IsRotorEquipped);
	crc(RotorCount);
	crc(IsLandable);
	crc(HeapID);
}


/// <summary>
/// Re-attaches the artwork this aircraft type names.
/// Artwork is never written to a save game, so the voxel and shape images the aircraft
/// draws with are fetched again once the members have been read.
/// </summary>
void AircraftTypeClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	Fetch_Voxel_Image();
	Fetch_Normal_Image();
	Load_Rotor_Assets();
}


/// <summary>
/// Lists the members this aircraft type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void AircraftTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HeapID);
	stream.Serialize(IsCarryall);
	stream.Serialize(IsRotorEquipped);
	stream.Serialize(RotorCount);
	stream.Serialize(Rotor1FLH);
	stream.Serialize(Rotor2FLH);
	stream.Serialize(Rotor3FLH);
	stream.Serialize(Rotor4FLH);
	stream.Serialize(Rotor1File);
	stream.Serialize(Rotor2File);
	stream.Serialize(Rotor3File);
	stream.Serialize(Rotor4File);
	stream.Serialize(Rotor1TiltedFiles);
	stream.Serialize(Rotor2TiltedFiles);
	stream.Serialize(Rotor3TiltedFiles);
	stream.Serialize(Rotor4TiltedFiles);
	stream.Serialize(IsLandable);
}


ClassID AircraftTypeClass::Class_ID(void) const
{
	return(ClassID_AircraftTypeClass);
}


/// <summary>
/// Fetches the aircraft type of the name specified, creating it if need be.
/// This routine is used while parsing the rules and the scenario so that a type may be
/// referred to before its own section has been read. A type made this way is added to the
/// aircraft type heap and filled in later.
/// </summary>
/// <param name="name">The INI name of the aircraft type to look for.</param>
/// <returns>Returns with a pointer to the aircraft type, or NULL for "none".</returns>
AircraftTypeClass * AircraftTypeClass::Find_Or_Make(char const * name)
{
	return(TFind_Or_Make<AircraftTypeClass>(name, AircraftTypes));
}


/// <summary>
/// Gets the SHP data for each of the rotor filenames specified by the Rotor#Anim and
/// Rotor#TiltedAnims keys. These files aren't initialized through anims so need to be
/// loaded specially. Because they aren't anims, they also need to be restored on game load,
/// so this function is used both when reading the INI and when reloading a savegame.
/// </summary>
void AircraftTypeClass::Load_Rotor_Assets(void)
{
	/*
	** Load the ordinary custom rotor SHPs.
	*/
	char const* rotorFiles[] = {
		Rotor1File,
		Rotor2File,
		Rotor3File,
		Rotor4File
	};

	void const** rotorData[] = {
		&Rotor1Data,
		&Rotor2Data,
		&Rotor3Data,
		&Rotor4Data
	};

	for (int i = 0; i < 4; ++i) {
		*rotorData[i] = NULL;

		if (rotorFiles[i][0] != '\0') {
			char fullname[MAX_PATH];
			_makepath(fullname, NULL, NULL, rotorFiles[i], ".SHP");
			*rotorData[i] = MFCD::Retrieve(fullname);
		}
	}

	/*
	** Load the directional rotor SHPs from the stored filenames.
	*/
	char(*tiltedFiles[4])[15 + 1] = {
		Rotor1TiltedFiles,
		Rotor2TiltedFiles,
		Rotor3TiltedFiles,
		Rotor4TiltedFiles
	};

	void const** tiltedData[4] = {
		Rotor1TiltedData,
		Rotor2TiltedData,
		Rotor3TiltedData,
		Rotor4TiltedData
	};

	for (int rotor = 0; rotor < 4; ++rotor) {
		for (int direction = 0; direction < 8; ++direction) {
			tiltedData[rotor][direction] = NULL;

			if (tiltedFiles[rotor][direction][0] != '\0') {
				char fullname[MAX_PATH];

				_makepath(
					fullname,
					NULL,
					NULL,
					tiltedFiles[rotor][direction],
					".SHP"
				);

				tiltedData[rotor][direction] = MFCD::Retrieve(fullname);
			}
		}
	}
}
