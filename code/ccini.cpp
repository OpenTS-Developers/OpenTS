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

/* $Header: /CounterStrike/CCINI.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CCINI.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 05/24/96                                                     *
 *                                                                                             *
 *                  Last Update : November 1, 1996 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CCINIClass::Calculate_Message_Digest -- Calculate a message digest for the current databas*
 *   CCINIClass::Get_AnimType -- Fetch an animation type number from the INI database.         *
 *   CCINIClass::Get_ArmorType -- Fetches the armor type from the INI database.                *
 *   CCINIClass::Get_Buildings -- Fetch a building bitfield from the INI database.             *
 *   CCINIClass::Get_BulletType -- Fetch the bullet identifier from the INI database.          *
 *   CCINIClass::Get_CrateType -- Fetches a crate type value from the INI database.            *
 *   CCINIClass::Get_HousesType -- Fetch a house identifier from the INI database.             *
 *   CCINIClass::Get_Lepton -- Fetches a lepton value from the INI database.                   *
 *   CCINIClass::Get_MPHType -- Fetches the speed value as a number from 0 to 100.             *
 *   CCINIClass::Get_OverlayType -- Fetch the overlay identifier from the INI database.        *
 *   CCINIClass::Get_Owners -- Fetch the owners (list of house bits).                          *
 *   CCINIClass::Get_SourceType -- Fetch the source (edge) type from the INI database.         *
 *   CCINIClass::Get_TerrainType -- Fetch the terrain type identifier from the INI database.   *
 *   CCINIClass::Get_TheaterType -- Fetch the theater type from the INI database.              *
 *   CCINIClass::Get_ThemeType -- Fetch the theme identifier.                                  *
 *   CCINIClass::Get_TriggerType -- Fetch the trigger type identifier from the INI database.   *
 *   CCINIClass::Get_Unique_ID -- Fetch a unique identifier number for the INI file.           *
 *   CCINIClass::Get_VQType -- Fetch the VQ movie identifier from the INI database.            *
 *   CCINIClass::Get_VocType -- Fetch a voc (sound effect) from the INI database.              *
 *   CCINIClass::Get_WarheadType -- Fetch the warhead type from the INI database.              *
 *   CCINIClass::Get_WeaponType -- Fetches the weapon type from the INI database.              *
 *   CCINIClass::Invalidate_Message_Digest -- Flag message digest as being invalid.            *
 *   CCINIClass::Load -- Load the INI database from the data stream specified.                 *
 *   CCINIClass::Load -- Load the INI database from the file specified.                        *
 *   CCINIClass::Put_AnimType -- Stores the animation identifier to the INI database.          *
 *   CCINIClass::Put_ArmorType -- Store the armor type to the INI database.                    *
 *   CCINIClass::Put_Buildings -- Store a building list to the INI database.                   *
 *   CCINIClass::Put_BulletType -- Store the projectile identifier into the INI database.      *
 *   CCINIClass::Put_CrateType -- Stores the crate value in the section and entry specified.   *
 *   CCINIClass::Put_HousesType -- Store a house identifier to the INI database.               *
 *   CCINIClass::Put_Lepton -- Stores a lepton value to the INI database.                      *
 *   CCINIClass::Put_MPHType -- Stores the speed value to the section & entry specified.       *
 *   CCINIClass::Put_OverlayType -- Store the overlay identifier into the INI database.        *
 *   CCINIClass::Put_Owners -- Store the house bitfield to the INI database.                   *
 *   CCINIClass::Put_SourceType -- Store the source (edge) identifier to the INI database.     *
 *   CCINIClass::Put_TerrainType -- Store the terrain type number to the INI database.         *
 *   CCINIClass::Put_TheaterType -- Store the theater identifier to the INI database.          *
 *   CCINIClass::Put_ThemeType -- Store the theme identifier to the INI database.              *
 *   CCINIClass::Put_TriggerType -- Store the trigger identifier to the INI database.          *
 *   CCINIClass::Put_VQType -- Store the VQ movie identifier into the INI database.            *
 *   CCINIClass::Put_VocType -- Store a sound effect identifier into the INI database.         *
 *   CCINIClass::Put_WarheadType -- Stores the warhead identifier to the INI database.         *
 *   CCINIClass::Put_WeaponType -- Store the weapon identifier to the INI database.            *
 *   CCINIClass::Save -- Pipes the INI database to the pipe specified.                         *
 *   CCINIClass::Save -- Save the INI data to the file specified.                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "ccini.h"

#include "_rtti.h"
#include "_source.h"
#include "_theater.h"
#include "airctype.h"
#include "builtype.h"
#include "category.h"
#include "conquer.h"
#include "coord.h"
#include "globals.h"
#include "houstype.h"
#include "incdec.h"
#include "infatype.h"
#include "movie.h"
#include "scheme.h"
#include "shapipe.h"
#include "side.h"
#include "suprtype.h"
#include "target.h"
#include "theme.h"
#include "unittype.h"
#include "veteran.h"
#include "voc.h"
#include "vox.h"
#include "weapon.h"
#include "xpipe.h"
#include "xstraw.h"

#include <algorithm>


char const * const ActionName[ACTION_COUNT] = {
	"None",
	"Move",
	"NoMove",
	"Enter",
	"Self",
	"Attack",
	"Harvest",
	"Select",
	"ToggleSelect",
	"Capture",
	"Repair",
	"Sell",
	"SellUnit",
	"NoSell",
	"NoRepair",
	"Sabotage",
	"Tote",
	"DontUse2",
	"DontUse3",
	"Nuke",
	"DontUse4",
	"DontUse5",
	"DontUse6",
	"DontUse7",
	"DontUse8",
	"GuardArea",
	"Heal",
	"Damage",
	"GRepair",
	"NoDeploy",
	"NoEnter",
	"NoGRepair",
	"TogglePower",
	"NoTogglePower",
	"EnterTunnel",
	"NoEnterTunnel",
	"EMPulse",
	"IonCannon",
	"EMPulseRange",
	"ChemBomb",
	"PlaceWaypoint",
	"NoPlaceWaypoint",
	"EnterWaypointMode",
	"FollowWaypoint",
	"SelectWaypoint",
	"LoopWaypointPath",
	"DragWaypoint",
	"AttackWaypoint",
	"EnterWaypoint",
	"PatrolWaypoint",
	"DropPod",
	"Rally To Point",
	"Attack Support"
};


/***********************************************************************************************
 * CCINIClass::Load -- Load the INI database from the file specified.                          *
 *                                                                                             *
 *    This routine will load the database from the file specified in much the same manner      *
 *    that the INIClass load function works. However, this class will examine the message      *
 *    digest (if present) and compare it to the actual digest. If they differ, a special       *
 *    return value is used. This will allow verification of the integrity of the ini data.     *
 *                                                                                             *
 * INPUT:   file  -- Reference to the file that will be read from.                             *
 *                                                                                             *
 *          withdigest  -- Should a message digest be examined when loaded. If there is a      *
 *                         mismatch detected, then an error will be returned.                  *
 *                                                                                             *
 * OUTPUT:  If the file was not read, returns 0. If the file was read ok, returns 1. If the    *
 *          file was read ok, but the digest doesn't verify, returns 2.                        *
 *                                                                                             *
 * WARNINGS:   If no message digest was present in the INI file, then no verification can      *
 *             be performed.                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *   08/21/1996 JLB : Handles digest control.                                                  *
 *=============================================================================================*/
int CCINIClass::Load(FileClass & file, bool withdigest, bool loadcomments)
{
	FileStraw straw(file);
	return(Load_Verified(straw, withdigest, loadcomments, file.File_Name()));
}


/// <summary>
/// Loads the database from a straw and, when asked, checks the message digest it carries.
/// </summary>
/// <param name="source">The file name to give in diagnostics, or NULL when there is none.</param>
/// <returns>0 if nothing loaded, 1 if the database loaded, and 2 if a digest was asked for
/// but was missing or did not match.</returns>
int CCINIClass::Load_Verified(Straw & file, bool withdigest, bool loadcomments, char const * source)
{
	int ok = BASECLASS::Load(file, loadcomments, source);

	Invalidate_Message_Digest();
	if (ok && withdigest) {

		/*
		**	If a digest is present, fetch it.
		*/
		unsigned char digest[20];
		int len = Get_UUBlock("Digest", digest, sizeof(digest));
		if (len == 0) {
			return(2);
		}
		if (len > 0) {
			Clear("Digest");

			/*
			**	Calculate the message digest for the INI data that was read.
			*/
			Calculate_Message_Digest();

			/*
			**	If the message digests don't match, then return with the special error code.
			*/
			if (memcmp(digest, Digest, sizeof(digest)) != 0) {
				return(2);
			}
		}
	}
	return(ok);
}


/***********************************************************************************************
 * CCINIClass::Load -- Load the INI database from the data stream specified.                   *
 *                                                                                             *
 *    This will load the INI database and in the process, it will fetch and verify any         *
 *    message digest present.                                                                  *
 *                                                                                             *
 * INPUT:   straw -- The data stream to fetch the INI data from.                               *
 *                                                                                             *
 *          withdigest  -- Should a message digest be examined when loaded. If there is a      *
 *                         mismatch detected, then an error will be returned.                  *
 *                                                                                             *
 * OUTPUT:  bool; Was the database loaded ok? (hack: returns "2" if digest doesn't match).     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/10/1996 JLB : Created.                                                                 *
 *   08/21/1996 JLB : Handles message digest control.                                          *
 *=============================================================================================*/
int CCINIClass::Load(Straw & file, bool withdigest, bool loadcomments)
{
	return(Load_Verified(file, withdigest, loadcomments, NULL));
}


/***********************************************************************************************
 * CCINIClass::Save -- Save the INI data to the file specified.                                *
 *                                                                                             *
 *    This routine will save the INI data to the file. It will add a message digest so that    *
 *    validity check can be performed when the INI data is subsequently read.                  *
 *                                                                                             *
 * INPUT:   file  -- Reference to the file to write the INI data to.                           *
 *                                                                                             *
 *          withdigest  -- Should a message digest be generated and saved with the INI         *
 *                         data file?                                                          *
 *                                                                                             *
 * OUTPUT:  bool; Was the INI data saved?                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *   08/21/1996 JLB : Handles message digest control.                                          *
 *=============================================================================================*/
int CCINIClass::Save(FileClass & file, bool withdigest) const
{
	FilePipe pipe(file);
	return(Save(pipe, withdigest));
}


/***********************************************************************************************
 * CCINIClass::Save -- Pipes the INI database to the pipe specified.                           *
 *                                                                                             *
 *    This routine will pipe the INI data to the pipe segment specified. It is functionally    *
 *    the same as the save operation. A message digest is added to the output data so that     *
 *    validity check can occur during a subsequent read.                                       *
 *                                                                                             *
 * INPUT:   straw -- Reference to the pipe that will receive the output ini data stream.       *
 *                                                                                             *
 *          withdigest  -- Should a message digest be generated and saved with the INI         *
 *                         data file?                                                          *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes output to the pipe.                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *   08/21/1996 JLB : Handles message digest control.                                          *
 *=============================================================================================*/
int CCINIClass::Save(Pipe & pipe, bool withdigest) const
{
	if (!withdigest) {
		return(BASECLASS::Save(pipe));
	}

	/*
	**	Just in case these entries are present, clear them out.
	*/
	((CCINIClass *)this)->Clear("Digest");

	/*
	**	Calculate what the new digest should be.
	*/
	((CCINIClass *)this)->Calculate_Message_Digest();

	/*
	**	Store the actual digest into the INI database.
	*/
	((CCINIClass *)this)->Put_UUBlock("Digest", Digest, sizeof(Digest));

	/*
	**	Output the database to the pipe specified.
	*/
	int length = BASECLASS::Save(pipe);

	/*
	**	Remove the digest from the database. It shouldn't stick around as if it were real data
	**	since it isn't really part of the INI database proper.
	*/
	((CCINIClass *)this)->Clear("Digest");

	/*
	**	Finally, return with the total number of bytes send out the pipe.
	*/
	return(length);
}


/// <summary>
/// Converts a percentage into a value on a 0 to 255 scale.
/// This routine is used by the speed accessors, where an INI value of 100 means full speed.
/// Anything outside the percentage range is clamped rather than rejected.
/// </summary>
/// <param name="val">The percentage to convert.</param>
/// <returns>Returns with the scaled value, which will lie between 0 and 255.</returns>
static inline int _Scale_To_256(int val)
{
	val = std::min(val, 100);
	val = std::max(val, 0);
	val = ((val * (MPH_LIGHT_SPEED + 1)) / 100);
	val = std::min<int>(val, MPH_LIGHT_SPEED);
	return(val);
}


/***********************************************************************************************
 * CCINIClass::Get_Lepton -- Fetches a lepton value from the INI database.                     *
 *                                                                                             *
 *    This routine will fetch the lepton value as if it were expressed as cells. Example;      *
 *    a value of 1 would mean 256 in leptons.                                                  *
 *                                                                                             *
 * INPUT:   section  -- The section identifier to look under.                                  *
 *                                                                                             *
 *          entry    -- The entry identifier to find.                                          *
 *                                                                                             *
 *          defvalue -- The default value to use if the specified section and entry could      *
 *                      not be located.                                                        *
 *                                                                                             *
 * OUTPUT:  Returns with the lepton value of the section & entry specified. If not found, then *
 *          the default value is returned.                                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
LEPTON CCINIClass::Get_Lepton(char const * section, char const * entry, LEPTON defvalue) const
{
	double result = Get_Float(section, entry, -1.0);
	if (result == -1.0) return(defvalue);
	return(result * CELL_LEPTON_W);
}


/***********************************************************************************************
 * CCINIClass::Put_Lepton -- Stores a lepton value to the INI database.                        *
 *                                                                                             *
 *    This routine will store the lepton value as if it were expressed in cells. Example;      *
 *    A lepton of 128 will be stored as ".5".                                                  *
 *                                                                                             *
 * INPUT:   section  -- The section identifier to store the value under.                       *
 *                                                                                             *
 *          entry    -- The entry to store the lepton value at.                                *
 *                                                                                             *
 *          value    -- The lepton value to store.                                             *
 *                                                                                             *
 * OUTPUT:  bool; Was the lepton value stored?                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CCINIClass::Put_Lepton(char const * section, char const * entry, LEPTON value)
{
	return(Put_Float(section, entry, value * (1.0 / CELL_LEPTON_W)));
}


/// <summary>
/// Stores an angle to the INI database.
/// The angle arrives on the 256 step direction scale the game works in, but is recorded in
/// degrees so that the file stays readable.
/// </summary>
/// <returns>bool; Was the angle stored?</returns>
bool CCINIClass::Put_Angle(char const * section, char const * entry, int value)
{
	char buffer[32];

	sprintf(buffer, "%d", 360 * value / 256);
	return(Put_String(section, entry, buffer));
}


/// <summary>
/// Fetches an angle from the INI database.
/// The angle is expressed in degrees for the convenience of whoever edits the file, and is
/// converted into the 256 step direction scale the game works in.
/// </summary>
/// <returns>Returns with the angle found, or the default value if the entry is
/// absent.</returns>
int CCINIClass::Get_Angle(char const * section, char const * entry, int defvalue) const
{
	char buffer[32];

	if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
		return(atoi(buffer) * 256 / 360);
	}
	return(defvalue);
}


/// <summary>
/// Fetches a cell location from the INI database.
/// The cell is expressed as an "x,y" pair.
/// </summary>
/// <returns>Returns with the cell found, or the default cell if the entry is absent.</returns>
Cell CCINIClass::Get_Cell(char const * section, char const * entry, Cell const & defvalue) const
{
	char buffer[64];

	if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
		int x, y;
		sscanf(buffer, "%d,%d", &x, &y);
		return(Cell(x, y));
	}
	return(defvalue);
}


/// <summary>
/// Stores a cell location to the INI database.
/// The cell is recorded as an "x,y" pair.
/// </summary>
/// <returns>bool; Was the cell stored?</returns>
bool CCINIClass::Put_Cell(char const * section, char const * entry, Cell const & value)
{
	char buffer[64];
	sprintf(buffer, "%d,%d", value.X, value.Y);
	return(Put_String(section, entry, buffer));
}


/***********************************************************************************************
 * CCINIClass::Get_MPHType -- Fetches the speed value as a number from 0 to 100.               *
 *                                                                                             *
 *    This routine will fetch the speed value as if it were expressed as a number from 0       *
 *    to 100. The value of 100 would translate into a speed of 256 leptons per game frame.     *
 *                                                                                             *
 * INPUT:   section  -- The section identifier to search for the entry under.                  *
 *                                                                                             *
 *          entry    -- The entry identifier to find.                                          *
 *                                                                                             *
 *          defvalue -- The default speed value to use if the entry could not be located.      *
 *                                                                                             *
 * OUTPUT:  Returns with the speed value. If no entry could be found, then the default value   *
 *          will be returned.                                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
MPHType CCINIClass::Get_MPHType(char const * section, char const * entry, MPHType defvalue) const
{
	int val = Get_Int(section, entry, -1);
	if (val == -1) {
		return(defvalue);
	}
	return(MPHType(_Scale_To_256(val)));
}


/***********************************************************************************************
 * CCINIClass::Put_MPHType -- Stores the speed value to the section & entry specified.         *
 *                                                                                             *
 *    Use this routine to store a speed value into the INI database. The number stored will    *
 *    be in a 0..100 format. A speed of 256 leptons per tick would be stored as 100.           *
 *                                                                                             *
 * INPUT:   section  -- The section identifier to store the entry under.                       *
 *                                                                                             *
 *          entry    -- The entry identifier to store the speed value to.                      *
 *                                                                                             *
 *          value    -- The speed value to store.                                              *
 *                                                                                             *
 * OUTPUT:  bool; Was the speed value stored?                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CCINIClass::Put_MPHType(char const * section, char const * entry, MPHType value)
{
	return(Put_Int(section, entry, ((int)value * 100) / (MPH_LIGHT_SPEED + 1)));
}


static struct
{
	char const * Name;
	PipEnum Type;
} _pips[6] = {
	{ "empty", PIP_EMPTY },
	{ "green", PIP_GREEN },
	{ "yellow", PIP_YELLOW },
	{ "white", PIP_WHITE },
	{ "red", PIP_RED },
	{ "blue", PIP_BLUE }
};


/// <summary>
/// Fetches a pip color from the INI database.
/// This is the color of the little markers drawn under a selected object.
/// </summary>
/// <returns>Returns with the pip color that matches the name recorded, or PIP_GREEN if the
/// name is not recognized.</returns>
PipEnum CCINIClass::Get_PipEnum(char const * section, char const * entry, PipEnum defvalue) const
{
	char buffer[32];

	Get_String(section, entry, _pips[defvalue].Name, buffer, sizeof(buffer));
	for (int index = 0; index < ARRAY_SIZE(_pips); index++) {
		if (!strcmpi(buffer, _pips[index].Name)) {
			return(_pips[index].Type);
		}
	}
	return(PIP_GREEN);
}


/// <summary>
/// Stores a pip color to the INI database.
/// </summary>
/// <returns>bool; Was the pip color stored? A value of PIP_EMPTY is never stored.</returns>
bool CCINIClass::Put_PipEnum(char const * section, char const * entry, PipEnum value)
{
	if (value != PIP_EMPTY) {
		return(Put_String(section, entry, _pips[value].Name));
	}
	return(false);
}


static struct
{
	char const * Name;
	PipScaleType Index;
} _pipscales[5] = {
	{ "Ammo", PIPSCALE_AMMO },
	{ "Tiberium", PIPSCALE_TIBERIUM },
	{ "Passengers", PIPSCALE_PASSENGERS },
	{ "Power", PIPSCALE_POWER },
	{ "Charge", PIPSCALE_CHARGE }
};


/// <summary>
/// Fetches a pip scale from the INI database.
/// The pip scale decides which quantity the pips drawn under a selected object are
/// counting -- ammo, tiberium, passengers, power or charge.
/// </summary>
/// <returns>Returns with the pip scale that matches the name recorded, or PIPSCALE_NONE if
/// the name is not recognized. An absent entry yields the default.</returns>
PipScaleType CCINIClass::Get_PipScaleType(char const * section, char const * entry, PipScaleType defvalue) const
{
	char buffer[32];

	if (Get_String(section, entry, "", buffer, sizeof(buffer)) > 0) {
		for (int index = 0; index < ARRAY_SIZE(_pipscales); index++) {
			if (!strcmpi(buffer, _pipscales[index].Name)) {
				return(_pipscales[index].Index);
			}
		}
		return(PIPSCALE_NONE);
	}
	return(defvalue);
}


/// <summary>
/// Stores a pip scale to the INI database.
/// </summary>
/// <returns>bool; Was the pip scale stored? A value of PIPSCALE_NONE is never
/// stored.</returns>
bool CCINIClass::Put_PipScaleType(char const * section, char const * entry, PipScaleType value)
{
	if (value != PIPSCALE_NONE) {
		return(Put_String(section, entry, _pipscales[value].Name));
	}
	return(false);
}


/// <summary>
/// Fetches an object category from the INI database.
/// The category is what the AI production logic uses to tell a soldier from a transport or
/// a power plant.
/// </summary>
/// <returns>Returns with the category that matches the name recorded, or CATEGORY_NONE if
/// the name is not recognized.</returns>
CategoryType CCINIClass::Get_CategoryType(char const * section, char const * entry, CategoryType defvalue) const
{
	char buffer[32];

	Get_String(section, entry, Name_From_Category(defvalue), buffer, sizeof(buffer));
	return(Category_From_Name(buffer));
}


/// <summary>
/// Stores an object category to the INI database.
/// </summary>
/// <returns>bool; Was the category stored? A value of CATEGORY_SOLDIER is never
/// stored.</returns>
bool CCINIClass::Put_CategoryType(char const * section, char const * entry, CategoryType value)
{
	if (value != CATEGORY_SOLDIER) {
		return(Put_String(section, entry, Name_From_Category(value)));
	}
	return(false);
}


/// <summary>
/// Fetches a target value from the INI database.
/// The target is expressed as a hexadecimal number.
/// </summary>
/// <returns>Returns with the target found, or the default target if the entry is
/// absent.</returns>
TargetStruct CCINIClass::Get_xTarget(char const * section, char const * entry, TargetStruct defvalue) const
{
	TargetStruct t;
	t.Value = Get_Hex(section, entry, defvalue.Value);
	return(t);
}


/// <summary>
/// Stores a target value to the INI database.
/// The target is recorded as a hexadecimal number.
/// </summary>
/// <returns>bool; Was the target stored?</returns>
bool CCINIClass::Put_xTarget(char const * section, char const * entry, TargetStruct value)
{
	return(Put_Hex(section, entry, value.Value));
}


/// <summary>
/// Fetches a color scheme from the INI database.
/// The scheme is recorded by name and matched against the loaded color schemes. Only a
/// scheme that carries more than one intensity level can be selected this way.
/// </summary>
/// <returns>Returns with the index of the matching color scheme, or the default index if no
/// scheme answers to the name.</returns>
int CCINIClass::Get_Scheme_Index(char const * section, char const * entry, int defvalue) const
{
	char buffer[32];

	Get_String(section, entry, ColorSchemes[defvalue]->Name, buffer, sizeof(buffer));
	for (int index = 0; index < ColorSchemes.Count(); index++) {
		if (!strcmpi(ColorSchemes[index]->Name, buffer) && ColorSchemes[index]->IntensityLevels != 1) {
			return(index);
		}
	}
	return(defvalue);
}


/// <summary>
/// Stores a color scheme to the INI database.
/// The scheme is recorded by name rather than by index, so the entry survives the scheme
/// list being reordered.
/// </summary>
/// <returns>bool; Was the color scheme stored?</returns>
bool CCINIClass::Put_Scheme_Index(char const * section, char const * entry, int value)
{
	return(Put_String(section, entry, ColorSchemes[value]->Name));
}


/// <summary>
/// Fetches an RGB color from the INI database.
/// The color is expressed as a "red,green,blue" triplet.
/// </summary>
/// <returns>Returns with the color found. If the entry is absent, or does not hold three
/// numbers, the default color is returned.</returns>
RGBClass CCINIClass::Get_RGBClass(char const * section, char const * entry, RGBClass const & defvalue) const
{
	int values[3];

	if (Read_Numbers(section, entry, values, 3) == INIReadResult::Parsed) {
		return(RGBClass(values[0], values[1], values[2]));
	}
	return(defvalue);
}


/// <summary>
/// Stores an RGB color to the INI database.
/// The color is recorded as a "red,green,blue" triplet.
/// </summary>
/// <returns>bool; Was the color stored?</returns>
bool CCINIClass::Put_RGBClass(char const * section, char const * entry, RGBClass const & value)
{
	char buffer[64];

	sprintf(buffer, "%d,%d,%d", value.Get_Red(), value.Get_Green(), value.Get_Blue());
	return(Put_String(section, entry, buffer));
}


/// <summary>
/// Fetches an HSV color from the INI database.
/// The color is expressed as a "hue,saturation,value" triplet.
/// </summary>
/// <returns>Returns with the color found. If the entry is absent, or does not hold three
/// numbers, the default color is returned.</returns>
HSVClass CCINIClass::Get_HSVClass(char const * section, char const * entry, HSVClass const & defvalue) const
{
	int values[3];

	if (Read_Numbers(section, entry, values, 3) == INIReadResult::Parsed) {
		return(HSVClass(values[0], values[1], values[2]));
	}
	return(defvalue);
}


/// <summary>
/// Stores an HSV color to the INI database.
/// The color is recorded as a "hue,saturation,value" triplet.
/// </summary>
/// <returns>bool; Was the color stored?</returns>
bool CCINIClass::Put_HSVClass(char const * section, char const * entry, HSVClass const & value)
{
	char buffer[64];

	sprintf(buffer, "%d,%d,%d", value.Get_Hue(), value.Get_Saturation(), value.Get_Value());
	return(Put_String(section, entry, buffer));
}


static struct
{
	char const * Name;
	BSizeType Type;
} _foundations[] = {
	{ "1x1", BSIZE_11 },
	{ "2x1", BSIZE_21 },
	{ "1x2", BSIZE_12 },
	{ "2x2", BSIZE_22 },
	{ "2x3", BSIZE_23 },
	{ "3x2", BSIZE_32 },
	{ "3x3", BSIZE_33 },
	{ "3x5", BSIZE_35 },
	{ "4x2", BSIZE_42 },
	{ "3x3Refinery", BSIZE_33_REF },
	{ "1x3", BSIZE_13 },
	{ "3x1", BSIZE_31 },
	{ "4x3", BSIZE_43 },
	{ "1x4", BSIZE_14 },
	{ "1x5", BSIZE_15 },
	{ "2x6", BSIZE_26 },
	{ "2x5", BSIZE_25 },
	{ "5x3", BSIZE_53 },
	{ "4x4", BSIZE_44 },
	{ "3x4", BSIZE_34 },
	{ "6x4", BSIZE_64 },
	{ "0x0", BSIZE_00 }
};


/// <summary>
/// Fetches a building foundation size from the INI database.
/// The size is recorded by its dimension name, such as "2x3". Building types use this to
/// know how much of the map they will occupy.
/// </summary>
/// <returns>Returns with the foundation size that matches the name recorded, or the smallest
/// foundation if the name is not recognized.</returns>
BSizeType CCINIClass::Get_BSizeType(char const * section, char const * entry, BSizeType defvalue) const
{
	char buffer[32];

	Get_String(section, entry, _foundations[defvalue].Name, buffer, sizeof(buffer));
	for (int index = 0; index < ARRAY_SIZE(_foundations); index++) {
		if (!strcmpi(buffer, _foundations[index].Name)) {
			return(_foundations[index].Type);
		}
	}
	return(BSIZE_FIRST);
}


/// <summary>
/// Stores a building foundation size to the INI database.
/// The size is recorded by its dimension name, such as "2x3".
/// </summary>
/// <returns>bool; Was the foundation size stored? A value of BSIZE_NONE is never
/// stored.</returns>
bool CCINIClass::Put_BSizeType(char const * section, char const * entry, BSizeType value)
{
	if (value != BSIZE_NONE) {
		return(Put_String(section, entry, _foundations[value].Name));
	}
	return(false);
}


const char * _mzones[MZONE_COUNT] = {
	"Normal",
	"Crusher",
	"Destroyer",
	"AmphibiousDestroyer",
	"AmphibiousCrusher",
	"Amphibious",
	"Subterannean",
	"Infantry",
	"InfantryDestroyer",
	"Fly"
};


/// <summary>
/// Fetches a movement zone identifier from the INI database.
/// This is the movement class that the pathfinding and zone logic use to decide what
/// terrain an object may travel over.
/// </summary>
/// <returns>Returns with the movement zone that matches the name recorded, or MZONE_NONE if
/// the name is not recognized.</returns>
MZoneType CCINIClass::Get_MZoneType(char const * section, char const * entry, MZoneType defvalue) const
{
	char buffer[32];

	Get_String(section, entry, _mzones[defvalue], buffer, sizeof(buffer));
	for (int index = 0; index < ARRAY_SIZE(_mzones); index++) {
		if (!strcmpi(buffer, _mzones[index])) {
			return(MZoneType)(index);
		}
	}
	return(MZONE_NONE);
}


/// <summary>
/// Stores a movement zone identifier to the INI database.
/// </summary>
/// <returns>bool; Was the movement zone stored? A value of MZONE_NONE is never
/// stored.</returns>
bool CCINIClass::Put_MZoneType(char const * section, char const * entry, MZoneType value)
{
	if (value != MZONE_NONE) {
		return(Put_String(section, entry, _mzones[value]));
	}
	return(false);
}


/// <summary>
/// Fetches a trigger action identifier from the INI database.
/// </summary>
/// <returns>Returns with the action that matches the name recorded, or ACTION_NONE if the
/// name is not recognized.</returns>
ActionType CCINIClass::Get_ActionType(char const * section, char const * entry, ActionType defvalue) const
{
	char buffer[32];

	Get_String(section, entry, ActionName[defvalue], buffer, sizeof(buffer));
	for (int index = 0; index < ARRAY_SIZE(ActionName); index++) {
		if (!strcmpi(buffer, ActionName[index])) {
			return(ActionType)(index);
		}
	}
	return(ACTION_NONE);
}


/// <summary>
/// Fetches a super weapon identifier from the INI database.
/// </summary>
/// <returns>Returns with the super weapon that matches the name recorded, or SUPER_NONE if
/// the name is not recognized.</returns>
SuperWeaponType CCINIClass::Get_SuperWeaponType(char const * section, char const * entry, SuperWeaponType defvalue) const
{
	char buffer[32];
	char emptystr[28];

	char const * defname;
	if (defvalue != SUPER_NONE) {
		defname = SuperWeaponTypes[defvalue]->IniName;
	}
	else {
		emptystr[0] = '\0';
		defname = emptystr;
	}
	Get_String(section, entry, defname, buffer, sizeof(buffer));
	return(SuperWeaponTypeClass::From_Name(buffer));
}


/// <summary>
/// Fetches a speech identifier from the INI database.
/// </summary>
/// <returns>Returns with the speech identifier that matches the name recorded, or VOX_NONE
/// if the name is not recognized.</returns>
VoxType CCINIClass::Get_VoxType(char const * section, char const * entry, VoxType defvalue) const
{
	char buffer[32];

	Get_String(section, entry, defvalue != VOX_NONE ? Speech[defvalue] : NULL, buffer, sizeof(buffer));
	for (int index = 0; index < ARRAY_SIZE(Speech); index++) {
		if (!strcmpi(buffer, Speech[index])) {
			return(VoxType)(index);
		}
	}
	return(VOX_NONE);
}


/// <summary>
/// Fetches an object category identifier from the INI database.
/// This routine is used where a rules entry names the kind of object it refers to, rather
/// than an individual object type.
/// </summary>
/// <returns>Returns with the object category that matches the name recorded.</returns>
RTTIType CCINIClass::Get_RTTIType(char const * section, char const * entry, RTTIType defvalue) const
{
	char buffer[32];

	Get_String(section, entry, Name_From_RTTI(defvalue), buffer, sizeof(buffer));
	return(RTTI_From_Name(buffer));
}


/// <summary>
/// Stores an object category identifier to the INI database.
/// </summary>
/// <returns>bool; Was the object category stored?</returns>
bool CCINIClass::Put_RTTIType(char const * section, char const * entry, RTTIType value)
{
	return(Put_String(section, entry, Name_From_RTTI(value)));
}


/***********************************************************************************************
 * CCINIClass::Get_Owners -- Fetch the owners (list of house bits).                            *
 *                                                                                             *
 *    Use this to fetch a house bit array value from the INI database. This value will be      *
 *    various bit positions set (1 << house#) for each house specified in the database.        *
 *    Houses can be specified in series by the house name separated by commas or by the        *
 *    special group names of "soviet", and "allies" to cover the houses that are members of    *
 *    these groups.                                                                            *
 *                                                                                             *
 * INPUT:   section  -- The section identifier to search for the entry under.                  *
 *                                                                                             *
 *          entry    -- The entry identifier to search for.                                    *
 *                                                                                             *
 *          defvalue -- The default house bitfield to use if the entry could not be found.     *
 *                                                                                             *
 * OUTPUT:  Returns with the house bitfield value. If the entry could not be found, then the   *
 *          default value is returned.                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int CCINIClass::Get_Owners(char const * section, char const * entry, int defvalue) const
{
	int ownable = defvalue;

	std::string value = Get_String(section, entry);
	if (!value.empty()) {

		ownable = 0;
		char * name = strtok(value.data(), ",");

		while (name) {
			ownable |= Owner_From_Name(name);
			name = strtok(NULL, ",");
		}
	}
	return(ownable);
}


/***********************************************************************************************
 * CCINIClass::Put_Owners -- Store the house bitfield to the INI database.                     *
 *                                                                                             *
 *    Use this routine to store the house bitfield data into the database. The bitfield format *
 *    matches the format used by the Get_Owners function. Example; if both England and         *
 *    Spain were specified in the bitfield, the entry would be stored as "England,Spain".      *
 *                                                                                             *
 * INPUT:   section  -- The section identifier to store the entry under.                       *
 *                                                                                             *
 *          entry    -- The entry identifier that is assigned the value.                       *
 *                                                                                             *
 *          value    -- The value to assign to the entry.                                      *
 *                                                                                             *
 * OUTPUT:  bool; Was the entry stored in the INI database?                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CCINIClass::Put_Owners(char const * section, char const * entry, int value)
{
	char buffer[128];

	buffer[0] = '\0';

	if (value == 0) {
		return(true);
	}

	for (HousesType house = HOUSE_FIRST; house < HouseTypes.Count(); house++) {
		HouseTypeClass * type = HouseTypes[house];
		if ((value & (1 << type->House)) != 0) {
			if (buffer[0] != '\0') {
				strcat(buffer, ",");
			}
			strcat(buffer, type->Name());
		}
	}

	if (buffer[0] != '\0') {
		return(Put_String(section, entry, buffer));
	}
	return(true);
}


/***********************************************************************************************
 * CCINIClass::Get_ArmorType -- Fetches the armor type from the INI database.                  *
 *                                                                                             *
 *    This routine will fetch the armor type from the database.                                *
 *                                                                                             *
 * INPUT:   section  -- Identifier for the section to search for the entry under.              *
 *                                                                                             *
 *          entry    -- Th identifier for the entry to search for.                             *
 *                                                                                             *
 *          defvalue -- The default value to use if the entry could not be located.            *
 *                                                                                             *
 * OUTPUT:  Returns with the armor type specified in the INI database. If it could not be      *
 *          found, then the default value is returned instead.                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
ArmorType CCINIClass::Get_ArmorType(char const * section, char const * entry, ArmorType defvalue) const
{
	char buffer[128];

	Get_String(section, entry, ArmorName[defvalue], buffer, sizeof(buffer));
	return(Armor_From_Name(buffer));
}


/***********************************************************************************************
 * CCINIClass::Put_ArmorType -- Store the armor type to the INI database.                      *
 *                                                                                             *
 *    Use this routine to store the specified armor type to the INI database.                  *
 *                                                                                             *
 * INPUT:   section  -- The section identifier to store the entry under.                       *
 *                                                                                             *
 *          entry    -- The entry to store the value at.                                       *
 *                                                                                             *
 *          value    -- The value to store in the database.                                    *
 *                                                                                             *
 * OUTPUT:  bool; Was the entry stored in the database?                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CCINIClass::Put_ArmorType(char const * section, char const * entry, ArmorType value)
{
	return(Put_String(section, entry, ArmorName[value]));
}


/***********************************************************************************************
 * CCINIClass::Put_VocType -- Store a sound effect identifier into the INI database.           *
 *                                                                                             *
 *    Use this routine to store a voc identifier (stored a the text name of the sound) into    *
 *    the INI database.                                                                        *
 *                                                                                             *
 * INPUT:   section  -- Identifier for the section to store the entry under.                   *
 *                                                                                             *
 *          entry    -- The entry to assign the value to.                                      *
 *                                                                                             *
 *          value    -- The sound effect to store to the entry.                                *
 *                                                                                             *
 * OUTPUT:  bool; Was the sound effect entry stored?                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CCINIClass::Put_VocType(char const * section, char const * entry, VocType value)
{
	if (value == VOC_NONE) {
		return(Put_String(section, entry, "<none>"));
	}
	return(Put_String(section, entry, Voc_Name(value)));
}


/// <summary>
/// Fetches a land type from the INI database.
/// </summary>
/// <returns>Returns with the land type named by the entry. If the entry could not be found,
/// then the default value is returned.</returns>
LandType CCINIClass::Get_LandType(char const * section, char const * entry, LandType defvalue) const
{
	char buffer[128];

	if (Get_String(section, entry, Name_From_Land(defvalue), buffer, sizeof(buffer))) {
		return(Land_From_Name(buffer));
	}
	return(defvalue);
}


/// <summary>
/// Stores a land type to the INI database.
/// </summary>
/// <returns>bool; Was the land type stored?</returns>
bool CCINIClass::Put_LandType(char const * section, char const * entry, LandType value)
{
	return(Put_String(section, entry, Name_From_Land(value)));
}


/***********************************************************************************************
 * CCINIClass::Get_HousesType -- Fetch a house identifier from the INI database.               *
 *                                                                                             *
 *    Use this routine to fetch an individual house identifier from the INI database. This is  *
 *    somewhat similar to the Get_Owners function but is limited to a single house.            *
 *                                                                                             *
 * INPUT:   section  -- Identifier for the section to search for the entry under.              *
 *                                                                                             *
 *          entry    -- Identifier for the entry to search for.                                *
 *                                                                                             *
 *          defvalue -- The default value to use if the entry could not be located.            *
 *                                                                                             *
 * OUTPUT:  Returns with the house identifier if it was found. If not found, then the default  *
 *          value is returned.                                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
HousesType CCINIClass::Get_HousesType(char const * section, char const * entry, HousesType defvalue) const
{
	char buffer[128];

	if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
		HousesType house = HouseTypeClass::From_Name(buffer);
		if (house == HOUSE_NONE) {
			HouseTypeClass * type = new HouseTypeClass(buffer);
			return((HousesType)HouseTypes.ID(type));
		}
		return(house);
	}
	return(defvalue);
}


/***********************************************************************************************
 * CCINIClass::Put_HousesType -- Store a house identifier to the INI database.                 *
 *                                                                                             *
 *    Use this routine to store the specified house identifier to the INI database.            *
 *                                                                                             *
 * INPUT:   section  -- The section to store the entry under.                                  *
 *                                                                                             *
 *          entry    -- Identifier for the entry to search for.                                *
 *                                                                                             *
 *          value    -- The house identifier to store in the database.                         *
 *                                                                                             *
 * OUTPUT:  bool; Was the house identifier stored?                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CCINIClass::Put_HousesType(char const * section, char const * entry, HousesType value)
{
	char const * name = "<none>";
	if (value != HOUSE_NONE) {
		name = HouseTypes[value]->Name();
	}

	return(Put_String(section, entry, name));
}


/// <summary>
/// Fetches a side identifier from the INI database.
/// This routine is used while the rules are being read, where a side may well be named
/// before it has been declared.
/// </summary>
/// <returns>Returns with the side identifier that matches the name recorded. If the entry
/// could not be found, then the default value is returned.</returns>
/// <remarks>A name that is not already known will add a new side to the side list.</remarks>
SideType CCINIClass::Get_Side(char const * section, char const * entry, SideType defvalue) const
{
	char buffer[128];

	if (Get_String(section, entry, "", buffer, sizeof(buffer)) && strcmpi(buffer, "<none>")) {
		SideType side = SideClass::From_Name(buffer);
		if (side == SIDE_NONE) {
			SideClass * type = new SideClass(buffer);
			return((SideType)Sides.ID(type));
		}
		return(side);
	}
	return(defvalue);
}


/// <summary>
/// Stores a side identifier to the INI database.
/// The side is recorded by name. A side identifier of SIDE_NONE is recorded as no side at all.
/// </summary>
/// <returns>bool; Was the side identifier stored?</returns>
bool CCINIClass::Put_Side(char const * section, char const * entry, SideType value)
{
	char const * name = "<none>";
	if (value != SIDE_NONE) {
		name = Sides[value]->Name();
	}
	return(Put_String(section, entry, name));
}


/***********************************************************************************************
 * CCINIClass::Get_VQType -- Fetch the VQ movie identifier from the INI database.              *
 *                                                                                             *
 *    Fetches the VQ movie name (identifier) from the INI database.                            *
 *                                                                                             *
 * INPUT:   section  -- Identifier for the section to search for the entry under.              *
 *                                                                                             *
 *          entry    -- Identifier for the entry to search for.                                *
 *                                                                                             *
 *          defvalue -- The default value to use if the entry could not be located.            *
 *                                                                                             *
 * OUTPUT:  Returns with the VQ movie identifier found. If the entry could not be located,     *
 *          then the default value is returned.                                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
VQType CCINIClass::Get_VQType(char const * section, char const * entry, VQType defvalue) const
{
	char buffer[128];

	if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
		VQType vq = VQ_From_Name(buffer);
		if (vq != VQ_NONE) {
			return(vq);
		}
	}
	return(defvalue);
}


/***********************************************************************************************
 * CCINIClass::Put_VQType -- Store the VQ movie identifier into the INI database.              *
 *                                                                                             *
 *    Use this routine to store the VQ movie identifier into the INI database.                 *
 *                                                                                             *
 * INPUT:   section  -- The section to store the entry under.                                  *
 *                                                                                             *
 *          entry    -- Identifier for the entry to store.                                     *
 *                                                                                             *
 *          value    -- The VQ movie identifier to store to the INI database.                  *
 *                                                                                             *
 * OUTPUT:  bool; Was the VQ identifier stored?                                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CCINIClass::Put_VQType(char const * section, char const * entry, VQType value)
{
	if ((unsigned)value >= (unsigned)Movies.Count()) {
		return(Put_String(section, entry, "<none>"));
	}
	return(Put_String(section, entry, Movies[value]));
}


/***********************************************************************************************
 * CCINIClass::Get_TheaterType -- Fetch the theater type from the INI database.                *
 *                                                                                             *
 *    This will fetch the theater identifier from the INI database.                            *
 *                                                                                             *
 * INPUT:   section  -- Identifier for the section to search for the entry under.              *
 *                                                                                             *
 *          entry    -- Identifier for the entry to search for.                                *
 *                                                                                             *
 *          defvalue -- The default value to use if the entry could not be located.            *
 *                                                                                             *
 * OUTPUT:  Returns with the theater type found. If the entry could not be found, then the     *
 *          default value is returned.                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
TheaterType CCINIClass::Get_TheaterType(char const * section, char const * entry, TheaterType defvalue) const
{
	char buffer[128];

	if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
		return(Theater_From_Name(buffer));
	}
	return(defvalue);
}


/***********************************************************************************************
 * CCINIClass::Put_TheaterType -- Store the theater identifier to the INI database.            *
 *                                                                                             *
 *    Use this routine to store the theater name to the INI database.                          *
 *                                                                                             *
 * INPUT:   section  -- The section to store the entry under.                                  *
 *                                                                                             *
 *          entry    -- Identifier for the entry to store.                                     *
 *                                                                                             *
 *          value    -- The theater identifier to store.                                       *
 *                                                                                             *
 * OUTPUT:  bool; Was the theater identifier stored?                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CCINIClass::Put_TheaterType(char const * section, char const * entry, TheaterType value)
{
	return(Put_String(section, entry, Theaters[value].Name));
}


/***********************************************************************************************
 * CCINIClass::Get_ThemeType -- Fetch the theme identifier.                                    *
 *                                                                                             *
 *    This routine will fetch the theme identifier from the INI database.                      *
 *                                                                                             *
 * INPUT:   section  -- The section to search for the entry under.                             *
 *                                                                                             *
 *          entry    -- Identifier of the entry to search for.                                 *
 *                                                                                             *
 *          defvalue -- The default theme identifier to return if the entry could not be found.*
 *                                                                                             *
 * OUTPUT:  Returns with the theme identifier if it was found. If not found, then the default  *
 *          value is returned instead.                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
ThemeType CCINIClass::Get_ThemeType(char const * section, char const * entry, ThemeType defvalue) const
{
	char buffer[128];

	if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
		return(Theme.From_Name(buffer));
	}
	return(defvalue);
}


/***********************************************************************************************
 * CCINIClass::Put_ThemeType -- Store the theme identifier to the INI database.                *
 *                                                                                             *
 *    This routine will store the specified theme identifier to the INI database.              *
 *                                                                                             *
 * INPUT:   section  -- Identifier for the section to store the entry under.                   *
 *                                                                                             *
 *          entry    -- Identifier for the entry to store the value to.                        *
 *                                                                                             *
 *          value    -- The theme identifier to store.                                         *
 *                                                                                             *
 * OUTPUT:  bool; Was the theme identifier stored.                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CCINIClass::Put_ThemeType(char const * section, char const * entry, ThemeType value)
{
	return(Put_String(section, entry, Theme.Base_Name(value)));
}


/***********************************************************************************************
 * CCINIClass::Get_SourceType -- Fetch the source (edge) type from the INI database.           *
 *                                                                                             *
 *    This routine will fetch the source (reinforcement edge) identifier from the INI          *
 *    database.                                                                                *
 *                                                                                             *
 * INPUT:   section  -- Identifier for the section that the entry will be searched under.      *
 *                                                                                             *
 *          entry    -- Identifier for the entry that will be searched for.                    *
 *                                                                                             *
 *          defvalue -- The default value to return if the entry could not be located.         *
 *                                                                                             *
 * OUTPUT:  Returns with the source type of the entry if found. If not found, then the         *
 *          default value is returned.                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
SourceType CCINIClass::Get_SourceType(char const * section, char const * entry, SourceType defvalue) const
{
	char buffer[128];

	if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
		return(Source_From_Name(buffer));
	}
	return(defvalue);
}


/***********************************************************************************************
 * CCINIClass::Put_SourceType -- Store the source (edge) identifier to the INI database.       *
 *                                                                                             *
 *    This will store the source type (reinforcement edge) to the INI database.                *
 *                                                                                             *
 * INPUT:   section  -- The section to store the entry under.                                  *
 *                                                                                             *
 *          entry    -- Identifier of the entry to store the source identifier to.             *
 *                                                                                             *
 *          value    -- The source (edge) value to store.                                      *
 *                                                                                             *
 * OUTPUT:  bool; Was the source identifier stored?                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CCINIClass::Put_SourceType(char const * section, char const * entry, SourceType value)
{
	return(Put_String(section, entry, SourceName[value]));
}


/***********************************************************************************************
 * CCINIClass::Get_CrateType -- Fetches a crate type value from the INI database.              *
 *                                                                                             *
 *    This will return with the crate type specified in the INI database.                      *
 *                                                                                             *
 * INPUT:   section  -- Identifier for the section to search under.                            *
 *                                                                                             *
 *          entry    -- The entry to find the matching crate value for.                        *
 *                                                                                             *
 *          defvalue -- The default crate value to return if the entry could not be found.     *
 *                                                                                             *
 * OUTPUT:  Returns with the crate type identified with the specified entry. If the entry      *
 *          could not be located, then the default value is returned.                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
CrateType CCINIClass::Get_CrateType(char const * section, char const * entry, CrateType defvalue) const
{
	char buffer[128];

	if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
		return(Crate_From_Name(buffer));
	}
	return(defvalue);
}


/***********************************************************************************************
 * CCINIClass::Put_CrateType -- Stores the crate value in the section and entry specified.     *
 *                                                                                             *
 *    This will store the specified crate value to the section and entry specified.            *
 *                                                                                             *
 * INPUT:   section  -- The section identifier to store the entry under.                       *
 *                                                                                             *
 *          entry    -- The entry identifier to store the crate value with.                    *
 *                                                                                             *
 *          value    -- The crate value to store.                                              *
 *                                                                                             *
 * OUTPUT:  bool; Was the crate value stored to the INI database?                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CCINIClass::Put_CrateType(char const * section, char const * entry, CrateType value)
{
	return(Put_String(section, entry, CrateNames[value]));
}


/***********************************************************************************************
 * CCINIClass::Get_Buildings -- Fetch a building bitfield from the INI database.               *
 *                                                                                             *
 *    This routing will fetch the a list of buildings from the INI database. The buildings     *
 *    are expressed as a comma separated list of building identifiers. The return value is     *
 *    a composite of bits that represent these buildings -- one bit per building type.         *
 *                                                                                             *
 * INPUT:   section  -- The section to search for the entry under.                             *
 *                                                                                             *
 *          entry    -- The entry to fetch the building list from.                             *
 *                                                                                             *
 *          defvalue -- The default value to return if the section and entry could not be      *
 *                      located.                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the building list (as a bitfield). If the entry could not be          *
 *          found, the the default value is returned instead.                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/11/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int CCINIClass::Get_Buildings(char const * section, char const * entry, int defvalue) const
{
	int pre;

	std::string value = Get_String(section, entry);
	if (!value.empty()) {

		pre = 0;
		char * token = strtok(value.data(), ",");
		while (token != NULL && *token != '\0') {

			if (!strcmpi(token, "POWER")) {
				pre |= 1L << (32 - 1);
			}
			if (!strcmpi(token, "FACTORY")) {
				pre |= 1L << (32 - 2);
			}
			if (!strcmpi(token, "BARRACKS")) {
				pre |= 1L << (32 - 3);
			}
			if (!strcmpi(token, "RADAR")) {
				pre |= 1L << (32 - 4);
			}
			StructType building = BuildingTypeClass::From_Name(token);
			if (building != STRUCT_NONE) {
				pre |= (1L << building);
			}
			token = strtok(NULL, ",");
		}
	} else {
		pre = defvalue;
	}

	return(pre);
}


/***********************************************************************************************
 * CCINIClass::Put_Buildings -- Store a building list to the INI database.                     *
 *                                                                                             *
 *    This will store a list of buildings to the INI database. The buildings are listed by     *
 *    their identifier names separated by commas.                                              *
 *                                                                                             *
 * INPUT:   section  -- The identifier for the section to store the entry under.               *
 *                                                                                             *
 *          entry    -- The entry to store the building list to.                               *
 *                                                                                             *
 *          value    -- A list of buildings (in the form of a bit field -- one bit per         *
 *                      building type).                                                        *
 *                                                                                             *
 * OUTPUT:  Was the building list stored to the INI file?                                      *
 *                                                                                             *
 * WARNINGS:   This is limited to the buildings that can be expressed in a bitfield long.      *
 *             Which means, there can be only a maximum of 32 building types listed and        *
 *             even then, the total line length generated must not exceed 128 bytes.           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/11/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CCINIClass::Put_Buildings(char const * section, char const * entry, int value)
{
	char buffer[128] = "";
	int maxi = (32 < BuildingTypes.Count()) ? 32 : BuildingTypes.Count();

	for (StructType index = STRUCT_FIRST; index < maxi; index++) {
		if ((value & (1L << index)) != 0) {

			if (buffer[0] != '\0') {
				strcat(buffer, ",");
			}
			strcat(buffer, BuildingTypes[index]->IniName);
		}
	}

	return(Put_String(section, entry, buffer));
}


/// <summary>
/// Stores a sound effect list to the INI database.
/// The sounds are recorded as a comma separated list of sound effect names.
/// </summary>
/// <returns>bool; Was the sound effect list stored?</returns>
bool CCINIClass::Put_VocType_List(char const * section, char const * entry, TypeList<int> const value)
{
	char buffer[128] = "";
	for (int index = 0; index < value.Count(); index++) {
		if (buffer[0]) {
			strcat(buffer, ",");
		}
		strcat(buffer, Voc_Name((VocType)value[index]));
	}
	return(Put_String(section, entry, buffer));
}


/// <summary>
/// Fetches a list of numbers from the INI database.
/// The numbers are expressed in the database as a comma separated list.
/// </summary>
/// <returns>Returns with the list of numbers specified. If the entry could not be found,
/// then the default value is returned.</returns>
TypeList<int> CCINIClass::Get_IntList(const char * section, const char * entry, TypeList<int> defvalue) const
{
	std::string value = Get_String(section, entry);

	if (!value.empty()) {
		TypeList<int> list;
		char * token = strtok(value.data(), ",");
		while (token != NULL && *token != '\0') {
			list.Add(atoi(token));
			token = strtok(NULL, ",");
		}
		return(list);
	}
	return(defvalue);
}


/// <summary>
/// Stores a list of numbers to the INI database.
/// The numbers are recorded as a comma separated list.
/// </summary>
/// <returns>bool; Was the number list stored?</returns>
bool CCINIClass::Put_IntList(char const * section, char const * entry, TypeList<int> const & value)
{
	char buffer[MAX_LINE_LENGTH] = "";
	char number[12];

	for (int index = 0; index < value.Count(); index++) {
		if (buffer[0]) {
			strcat(buffer, ",");
		}
		sprintf(number, "%d", value[index]);
		strcat(buffer, number);
	}
	return(Put_String(section, entry, buffer));
}


/// <summary>
/// Fetches a target list from the INI database.
/// This routine will read a comma separated list of infantry and unit names, recording
/// each one as an encoded target. Any name that matches neither is ignored.
/// </summary>
/// <returns>Returns with the list of encoded targets specified. If the entry could not be
/// found, then the default value is returned.</returns>
TypeList<int> CCINIClass::Get_Target_List(const char * section, const char * entry, TypeList<int> defvalue) const
{
	std::string value = Get_String(section, entry);

	if (!value.empty()) {
		TypeList<int> list;
		char * token = strtok(value.data(), ",");
		while (token != NULL && *token != '\0') {
			TargetClass trgt;
			InfantryType inf = InfantryTypeClass::From_Name(token);
			if (inf != INFANTRY_NONE) {
				trgt = TargetClass(::InfantryTypes[inf]);
			} else {
				UnitType unit = UnitTypeClass::From_Name(token);
				if (unit != UNIT_NONE) {
					trgt = TargetClass(::UnitTypes[unit]);
				}
			}
			if (trgt.Is_Valid()) {
				list.Add(trgt.Encode());
			}
			token = strtok(NULL, ",");
		}
		return(list);
	}
	return(defvalue);
}


/// <summary>
/// Stores a target list to the INI database.
/// The targets are recorded as a comma separated list of the object types they refer to.
/// </summary>
/// <returns>bool; Was the target list stored?</returns>
bool CCINIClass::Put_Target_List(const char * section, const char * entry, TypeList<int> const & value)
{
	char buffer[MAX_LINE_LENGTH] = "";

	for (int index = 0; index < value.Count(); index++) {
		if (buffer[0]) {
			strcat(buffer, ",");
		}
		TargetClass target;
		target.Decode(value[index]);
		TechnoTypeClass *ttype = target.As_TechnoType();
		strcat(buffer, ttype->Name());
	}
	return(Put_String(section, entry, buffer));
}


/// <summary>
/// Fetches a three dimensional vector from the INI database.
/// The vector is expressed in the database as comma separated X, Y, and Z values.
/// </summary>
/// <returns>Returns with the vector specified. If the entry could not be found, or does not
/// hold three numbers, then the default value is returned.</returns>
TPoint3D<float> CCINIClass::Get_Vector(char const * section, char const * entry, TPoint3D<float> const & defvalue) const
{
	return(Get_Point(section, entry, defvalue));
}


/// <summary>
/// Fetches a three dimensional offset from the INI database.
/// The offset is expressed in the database as comma separated X, Y, and Z values.
/// </summary>
/// <returns>Returns with the offset specified. If the entry could not be found, or does not
/// hold three numbers, then the default value is returned.</returns>
TPoint3D<int> CCINIClass::Get_Offset(char const * section, char const * entry, TPoint3D<int> const & defvalue) const
{
	return(Get_Point(section, entry, defvalue));
}


/// <summary>
/// Fetches a techno type list from the INI database.
/// This routine will read a comma separated list of object names. Any name that does not
/// match a known techno type is ignored.
/// </summary>
/// <returns>Returns with the list of techno types specified. If the entry could not be
/// found, then the default value is returned.</returns>
TypeList<TechnoTypeClass *> CCINIClass::Get_TechnoType_List(const char * section, const char * entry, TypeList<TechnoTypeClass *> defvalue) const
{
	std::string value = Get_String(section, entry);

	if (!value.empty()) {
		TypeList<TechnoTypeClass *> list;
		char * token = strtok(value.data(), ",");
		while (token != NULL && *token != '\0') {
			for (int index = 0; index < TechnoTypes.Count(); index++) {
				if (!strcmpi(token, TechnoTypes[index]->Name())) {
					list.Add(TechnoTypes[index]);
					break;
				}
			}
			token = strtok(NULL, ",");
		}
		return(list);
	}
	return(defvalue);
}


/// <summary>
/// Stores a techno type list to the INI database.
/// The types are recorded as a comma separated list of object names.
/// </summary>
/// <returns>bool; Was the techno type list stored?</returns>
bool CCINIClass::Put_TechnoType_List(char const * section, char const * entry, TypeList<TechnoTypeClass *> const & value)
{
	char buffer[MAX_LINE_LENGTH] = "";

	for (int index = 0; index < value.Count(); index++) {
		if (buffer[0]) {
			strcat(buffer, ",");
		}
		strcat(buffer, value[index]->Name());
	}
	return(Put_String(section, entry, buffer));
}


/// <summary>
/// Fetches a house list from the INI database.
/// This routine will read a comma separated list of house names. A side may be named in
/// place of a house, in which case every house belonging to that side is added to the list.
/// </summary>
/// <returns>Returns with the list of house identifiers specified. If the entry could not be
/// found, then the default value is returned.</returns>
TypeList<int> CCINIClass::Get_House_List(const char * section, const char * entry, TypeList<int> defvalue) const
{
	std::string value = Get_String(section, entry);

	if (!value.empty()) {
		TypeList<int> list;
		char * token = strtok(value.data(), ",");
		while (token != NULL && *token != '\0') {
			int house = (int)HouseTypeClass::From_Name(token);
			if (house != HOUSE_NONE) {
				list.Add(house);
			}
			else {
				SideType side = SideClass::From_Name(token);
				if (side != SIDE_NONE) {
					SideClass * otherside = Sides[side];
					for (int index = 0; index < otherside->Houses.Count(); index++) {
						list.Add(otherside->Houses[index]);
					}
				}
			}
			token = strtok(NULL, ",");
		}
		return(list);
	}
	return(defvalue);
}


/// <summary>
/// Stores a house list to the INI database.
/// The houses are recorded as a comma separated list of house names.
/// </summary>
/// <returns>bool; Was the house list stored?</returns>
bool CCINIClass::Put_House_List(char const * section, char const * entry, TypeList<HousesType> value)
{
	char buffer[128] = "";

	for (int index = 0; index < value.Count(); index++) {
		if (buffer[0]) {
			strcat(buffer, ",");
		}
		strcat(buffer, HouseTypes[value[index]]->Name());
	}
	return(Put_String(section, entry, buffer));
}


/// <summary>
/// Fetches a list of colors from the INI database.
/// This routine will read a series of parenthesized red, green, and blue triplets. Any
/// triplet that is not complete is discarded.
/// </summary>
/// <returns>Returns with the list of colors specified. If the entry could not be found,
/// then the default value is returned.</returns>
TypeList<RGBClass> CCINIClass::Get_RGBClass_List(const char * section, const char * entry, TypeList<RGBClass> defvalue) const
{
	std::string value = Get_String(section, entry);

	if (!value.empty()) {
		TypeList<RGBClass> list;
		char * token = strtok(value.data(), ",");
		while (token != NULL && *token != '\0') {

			RGBClass c(0,0,0);
			bool valid = true;

			if (token && *token) {
				if (*token == '(') token++;
				c.Set_Red(atoi(token));
			} else {
				valid = false;
			}

			token = strtok(NULL, ",");
			if (token && *token) {
				c.Set_Green(atoi(token));
			} else {
				valid = false;
			}

			token = strtok(NULL, ",");
			if (token && *token) {
				if (token[strlen(token) - 1] == ')') token[strlen(token) - 1] = '\0';
				c.Set_Blue(atoi(token));
			} else {
				valid = false;
			}

			if (valid) {
				list.Add(c);
			}

			token = strtok(NULL, ",");
		}
		return(list);
	}
	return(defvalue);
}


/***********************************************************************************************
 * CCINIClass::Get_Unique_ID -- Fetch a unique identifier number for the INI file.             *
 *                                                                                             *
 *    This is a shorthand version of the message digest. It calculates the ID number from the  *
 *    message digest itself.                                                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a 32 bit unique identifier number for the INI database.               *
 *                                                                                             *
 * WARNINGS:   Since the return value is only 32 bits, it is much less secure than the         *
 *             complete message digest.                                                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/01/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int CCINIClass::Get_Unique_ID(void) const
{
	if (!IsDigestPresent) {
		((CCINIClass *)this)->Calculate_Message_Digest();
	}

	return(CRCEngine()(&Digest[0], sizeof(Digest)));
}


/***********************************************************************************************
 * CCINIClass::Calculate_Message_Digest -- Calculate a message digest for the current database *
 *                                                                                             *
 *    This will calculate a new message digest according to the current state of the INI       *
 *    database.                                                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   If the database is changed in any fashion, this message digest will be rendered *
 *             obsolete.                                                                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/01/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void CCINIClass::Calculate_Message_Digest(void)
{
	/*
	**	Calculate the message digest for the INI data that was read.
	*/
	SHAPipe sha;
	BASECLASS::Save(sha);
	sha.Result(Digest);
	IsDigestPresent = true;
}


/***********************************************************************************************
 * CCINIClass::Invalidate_Message_Digest -- Flag message digest as being invalid.              *
 *                                                                                             *
 *    This flags the message digest as being invalid so that it will be recalculated when      *
 *    needed.                                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/01/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void CCINIClass::Invalidate_Message_Digest(void)
{
	IsDigestPresent = false;
}


/// <summary>
/// Fetches a target from the INI database.
/// This routine will search the infantry, unit, aircraft, and building type lists for an
/// object type bearing the name recorded in the database.
/// </summary>
/// <returns>Returns with a target for the object type found. If no type matches the name,
/// then the default value is returned.</returns>
TargetClass CCINIClass::Get_Target(char const * section, char const * entry, TargetClass const & defvalue) const
{
	char buffer[128];

	Get_String(section, entry, defvalue.As_TechnoType() == NULL ? "<none>" : defvalue.As_TechnoType()->Name(), buffer, sizeof(buffer));

	int type = -1;
	AbstractClass * target = NULL;

	if (type == -1) {
		type = (int)InfantryTypeClass::From_Name(buffer);
		if (type != -1) {
			target = InfantryTypes[type];
		}
	}

	if (type == -1) {
		type = (int)UnitTypeClass::From_Name(buffer);
		if (type != -1) {
			target = UnitTypes[type];
		}
	}

	if (type == -1) {
		type = (int)AircraftTypeClass::From_Name(buffer);
		if (type != -1) {
			target = AircraftTypes[type];
		}
	}

	if (type == -1) {
		type = (int)BuildingTypeClass::From_Name(buffer);
		if (type != -1) {
			target = BuildingTypes[type];
		}
	}

	if (target == NULL) {
		return(defvalue);
	}

	return(TargetClass(target));
}


/// <summary>
/// Stores a target to the INI database.
/// The target is recorded by the name of the object type it refers to. A target that does
/// not resolve to an object type is recorded as having none.
/// </summary>
/// <returns>bool; Was the target stored?</returns>
bool CCINIClass::Put_Target(char const * section, char const * entry, TargetClass const & value)
{
	TechnoTypeClass *ttype = value.As_TechnoType();
	if (ttype != NULL) {
		return(Put_String(section, entry, ttype->Name()));
	}
	return(Put_String(section, entry, "<none>"));
}


/// <summary>
/// Fetches a speed type from the INI database.
/// </summary>
/// <returns>Returns with the speed type named by the entry. If the entry could not be
/// found, then the default value is returned.</returns>
SpeedType CCINIClass::Get_SpeedType(char const * section, char const * entry, SpeedType defvalue) const
{
	char buffer[128];

	if (Get_String(section, entry, Name_From_Speed(defvalue), buffer, sizeof(buffer))) {
		return(Speed_From_Name(buffer));
	}
	return(defvalue);
}


/// <summary>
/// Stores a speed type to the INI database.
/// </summary>
/// <returns>bool; Was the speed type stored?</returns>
bool CCINIClass::Put_SpeedType(char const * section, char const * entry, SpeedType value)
{
	return(Put_String(section, entry, Name_From_Speed(value)));
}


/// <summary>
/// Fetches a building type list from the INI database.
/// This routine will read a comma separated list of building names. The special group
/// names, such as "POWER" and "BARRACKS", are recognized and stored as the matching group
/// identifier rather than as an individual building type.
/// </summary>
/// <param name="ini">The INI database to fetch the entry from.</param>
/// <returns>Returns with the list of building types specified. If the entry could not be
/// found, then the default value is returned.</returns>
TypeList<int> CCINIClass::Get_BuildingType_List(CCINIClass const & ini, char const * section, char const * entry, TypeList<int> defvalue)
{
	std::string value = ini.Get_String(section, entry);

	if (!value.empty()) {
		TypeList<int> list;
		char * token = strtok(value.data(), ",");
		while (token != NULL && *token != '\0') {

			bool isgroup = false;
			if (!strcmpi(token, "POWER")) {
				list.Add(STRUCT_G_POWER);
				isgroup = true;
			}
			if (!strcmpi(token, "FACTORY")) {
				list.Add(STRUCT_G_FACTORY);
				isgroup = true;
			}
			if (!strcmpi(token, "BARRACKS")) {
				list.Add(STRUCT_G_BARRACKS);
				isgroup = true;
			}
			if (!strcmpi(token, "RADAR")) {
				list.Add(STRUCT_G_RADAR);
				isgroup = true;
			}
			if (!strcmpi(token, "TECH")) {
				list.Add(STRUCT_G_TECH);
				isgroup = true;
			}
			if (!strcmpi(token, "GDIFACTORY")) {
				list.Add(STRUCT_G_GDIFACTORY);
				isgroup = true;
			}
			if (!strcmpi(token, "NODFACTORY")) {
				list.Add(STRUCT_G_NODFACTORY);
				isgroup = true;
			}
			if (!isgroup) {
				int building = BuildingTypeClass::From_Name(token);
				if (building != STRUCT_NONE) {
					list.Add(building);
				}
			}

			token = strtok(NULL, ",");
		}
		return(list);
	}

	return(defvalue);
}


/// <summary>
/// Fetches a veterancy ability set from the INI database.
/// This routine will read a comma separated list of ability names, as used by the elite
/// ability lists of the object types. Any name that is not recognized is ignored.
/// </summary>
/// <returns>Returns with the abilities specified. If the entry could not be found, then the
/// default value is returned.</returns>
AbilityFlagsType CCINIClass::Get_Abilities(char const * section, char const * entry, AbilityFlagsType const & defvalue) const
{
	std::string value = Get_String(section, entry);

	if (!value.empty()) {
		AbilityFlagsType abilities;
		char * token = strtok(value.data(), ",");
		while (token != NULL && *token != '\0') {
			AbilityType ability = Ability_From_Name(token);
			if (ability != ABILITY_NONE) {
				abilities[ability] = true;
			}
			token = strtok(NULL, ",");
		}
		return(abilities);
	}
	return(defvalue);
}
