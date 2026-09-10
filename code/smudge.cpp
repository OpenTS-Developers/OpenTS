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

/* $Header: /CounterStrike/SMUDGE.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SMUDGE.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : August 9, 1994                                               *
 *                                                                                             *
 *                  Last Update : July 3, 1996 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   SmudgeClass::Disown -- Disowns (removes) a building bib piece.                            *
 *   SmudgeClass::Init -- Initialize the smudge tracking system.                               *
 *   SmudgeClass::Mark -- Marks a smudge down on the map.                                      *
 *   SmudgeClass::Read_INI -- Reads smudge data from an INI file.                              *
 *   SmudgeClass::SmudgeClass -- Constructor for smudge objects.                               *
 *   SmudgeClass::Write_INI -- Store all the smudge data to the INI database.                  *
 *   SmudgeClass::operator delete -- Deletes the smudge from the tracking system.              *
 *   SmudgeClass::operator new -- Creator of smudge objects.                                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "smudge.h"

#include "_map.h"
#include "ccini.h"
#include "cell.h"
#include "globals.h"
#include "savestream.h"
#include "sun.h"
#include "tracker.h"
#include "vector.h"

#include <cstdio>


HousesType SmudgeClass::ToOwn = HOUSE_NONE;

char const * const SmudgeClass::INI_NAME = "Smudge";


/***********************************************************************************************
 * SmudgeClass::SmudgeClass -- Constructor for smudge objects.                                 *
 *                                                                                             *
 *    This is the typical constructor for smudge objects. If the position to place the         *
 *    smudge is not given, then the smudge will be initialized in a limbo state. If the        *
 *    smudge is placed on the map, then this operation causes the smudge object itself to be   *
 *    deleted and special map values updated to reflect the presence of a smudge.              *
 *                                                                                             *
 * INPUT:   type  -- The type of smudge to construct.                                          *
 *                                                                                             *
 *          pos   -- The position to place the smudge. If -1, then the smudge is initialized   *
 *                   into a limbo state.                                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/01/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
SmudgeClass::SmudgeClass(SmudgeTypeClass const * type, Coord const & pos, HousesType house) :
	BASECLASS(),
	Class((SmudgeTypeClass *)type)
{
	Create_ID();
	Smudges.Add(this);
	if (pos != COORD_NONE) {
		ToOwn = house;
		if (!Unlimbo(pos)) {
			Delete_Me();
		} else {
			ToOwn = HOUSE_NONE;
		}
	}
}


/// <summary>
/// Destroys the smudge and removes it from the game.
/// This routine detaches anything that referred to this smudge, lifts it off the map, and
/// drops it from the global smudge list.
/// </summary>
SmudgeClass::~SmudgeClass(void)
{
	Detach_This_From_All(this);
	Smudges.Delete(this);
	if (GameActive) {
		Limbo();
	}
	Class = NULL;
}


/***********************************************************************************************
 * SmudgeClass::Mark -- Marks a smudge down on the map.                                        *
 *                                                                                             *
 *    This routine will place the smudge on the map. If the map cell allows.                   *
 *                                                                                             *
 * INPUT:   mark  -- The type of marking to perform. Only MARK_DOWN is supported.              *
 *                                                                                             *
 * OUTPUT:  bool; Was the smudge marked successfully? Failure occurs if the smudge isn't       *
 *                marked DOWN.                                                                 *
 *                                                                                             *
 * WARNINGS:   The smudge object is DELETED by this routine.                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/22/1994 JLB : Created.                                                                 *
 *   12/23/1994 JLB : Checks low level legality before proceeding.                             *
 *=============================================================================================*/
bool SmudgeClass::Mark(MarkType mark)
{
	if (BASECLASS::Mark(mark)) {
		if (mark == MARK_DOWN || mark == MARK_DOWN_FORCED) {
			Cell origin = (Cell)PositionCell;

			if (ScenarioInit > 0 || Debug_Map || Class->Can_Place_Here(origin, true)) {
				Class->Place(origin);
			}

			/*
			**	Whether it was successful in placing, or not, delete the smudge object. It isn't
			**	needed once the map has been updated with the proper smudge data. Fake this object
			**	as if it were never placed down!
			*/
			IsDown = false;
			Delete_Me();
			return(true);
		}
	}

	return(false);
}


/***********************************************************************************************
 * SmudgeClass::Disown -- Disowns (removes) a building bib piece.                              *
 *                                                                                             *
 *    This routine is used when a building is removed from the game. If there was any bib      *
 *    attached, this routine will be called to disown the cells and remove the bib artwork.    *
 *                                                                                             *
 * INPUT:   cell  -- The origin cell for this bib removal.                                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This is actually working on a temporary bib object. It is created for the sole  *
 *             purpose of calling this routine. It will be deleted immediately afterward.      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/04/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void SmudgeClass::Disown(Cell const & cell)
{
}


/***********************************************************************************************
 * SmudgeClass::Read_INI -- Reads smudge data from an INI file.                                *
 *                                                                                             *
 *    This routine is used by the scenario loader to read the smudge data in an INI file and   *
 *    create the appropriate smudge objects on the map.                                        *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the INI file staging buffer.                                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/01/1994 JLB : Created.                                                                 *
 *   07/24/1995 JLB : Sets the smudge data value as well.                                      *
 *=============================================================================================*/
void SmudgeClass::Read_INI(CCINIClass const & ini)
{
	char	buf[128];	// Working string staging buffer.

	int len = ini.Entry_Count(INI_NAME);
	for (int index = 0; index < len; index++) {
		char const * entry = ini.Get_Entry(INI_NAME, index);
		SmudgeType	smudge;		// Smudge type.

		ini.Get_String(INI_NAME, entry, NULL, buf, sizeof(buf));
		smudge = SmudgeTypeClass::From_Name(strtok(buf, ","));
		if (smudge != SMUDGE_NONE) {
			char * ptr = strtok(NULL, ",");
			if (ptr != NULL) {

				int x;
				int y;
				x = atoi(ptr);
				ptr = strtok(NULL, ",");
				y = atoi(ptr);
				Cell cell(x, y);

				ptr = strtok(NULL, ",");

				if (ptr == NULL || atoi(ptr) == 0) {
					new SmudgeClass(SmudgeTypes[smudge], cell);
				}
			}
		}
	}
}


/***********************************************************************************************
 * SmudgeClass::Write_INI -- Store all the smudge data to the INI database.                    *
 *                                                                                             *
 *    This routine will output all the smudge data to the INI database.                        *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database object.                                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void SmudgeClass::Write_INI(CCINIClass & ini)
{
	/*
	**	First, clear out all existing template data from the ini file.
	*/
	ini.Clear(INI_NAME);

	/*
	**	Find all templates and write them to the file.
	*/
	int index = 0;
	for (int y = 0; y < MAP_CELL_H; y++) {
		for (int x = 0; x < MAP_CELL_W; x++) {
			CellClass * ptr;

			ptr = &Map[Cell(x, y)];
			if (ptr->Smudge != SMUDGE_NONE && ptr->SmudgeData == 0) {
				SmudgeTypeClass const * stype = SmudgeTypes[ptr->Smudge];
				char	uname[10];
				char	buf[127];

				sprintf(uname, "%d", index);
				sprintf(buf, "%s,%d,%d,%d", (char const *)stype->IniName, x, y, ptr->SmudgeData);
				ini.Put_String(INI_NAME, uname, buf);
				index++;
			}
		}
	}
}


/// <summary>
/// Lists the members this smudge carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void SmudgeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Class);
}


/// <summary>
/// Fetches the type class this smudge was created from.
/// </summary>
/// <returns>Returns with a pointer to the smudge type object.</returns>
ObjectTypeClass const * SmudgeClass::Class_Of(void) const
{
	return(Class);
}


/// <summary>
/// Fetches the run time type of this object.
/// </summary>
/// <returns>Returns with RTTI_SMUDGE.</returns>
RTTIType SmudgeClass::Fetch_RTTI(void) const
{
	return(RTTI_SMUDGE);
}


ClassID SmudgeClass::Class_ID(void) const
{
	return(ClassID_SmudgeClass);
}
