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

/* $Header: /CounterStrike/BASE.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : BASE.CPP                                                     *
 *                                                                                             *
 *                   Programmer : Bill Randolph                                                *
 *                                                                                             *
 *                   Start Date : 03/27/95                                                     *
 *                                                                                             *
 *                  Last Update : July 30, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   BaseClass::Get_Building -- Returns ptr to the built building for the given node           *
 *   BaseClass::Get_Node -- Finds the node that matches the cell specified.                    *
 *   BaseClass::Get_Node -- Returns ptr to the node corresponding to given object              *
 *   BaseClass::Is_Built -- Tells if given item in the list has been built yet                 *
 *   BaseClass::Is_Node -- Tells if the given building is part of our base list                *
 *   BaseClass::Load -- loads from a saved game file                                           *
 *   BaseClass::Next_Buildable -- returns ptr to the next node that needs to be built          *
 *   BaseClass::Read_INI -- INI reading routine                                                *
 *   BaseClass::Save -- saves to a saved game file                                             *
 *   BaseClass::Write_INI -- Writes all the base information to the INI database.              *
 *   BaseNodeClass::operator != -- inequality operator                                         *
 *   BaseNodeClass::operator == -- equality operator                                           *
 *   BaseNodeClass::operator > -- greater-than operator                                        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "base.h"

#include "_map.h"
#include "building.h"
#include "builtype.h"
#include "cell.h"
#include "overtype.h"
#include "savestream.h"
#include "vector.h"

#include <cstdio>

/*
 * Unused, only ever nulled in HouseClass DTOR
 */
HouseClass *UnusedHouse = NULL;


char const * const BaseClass::INI_NAME = "Base";

/***********************************************************************************************
 * BaseNodeClass::operator == -- equality operator                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      node      node to test against                                                         *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      true = equal, false = not equal                                                        *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/24/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
bool BaseNodeClass::operator == (BaseNodeClass const & node)
{
	return(Type == node.Type && CellID == node.CellID);
}


/***********************************************************************************************
 * BaseNodeClass::operator != -- inequality operator                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      node      node to test against                                                         *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      comparison result                                                                      *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/24/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
bool BaseNodeClass::operator !=(BaseNodeClass const & node)
{
	return(!(*this == node));
}


/***********************************************************************************************
 * BaseNodeClass::operator > -- greater-than operator                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      node      node to test against                                                         *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      comparison result                                                                      *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/24/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
bool BaseNodeClass::operator > (BaseNodeClass const & )
{
	return(true);
}


/// <summary>
/// Creates an empty base.
/// This routine sets up a base that owns no nodes and belongs to no house. The scenario
/// reader fills in the node list and the owner as the map is parsed.
/// </summary>
BaseClass::BaseClass(void) :
	Nodes(),
	PercentBuilt(0),
	InnerCells(),
	OuterCells(),
	PlacementCenter(0,0),
	BaseAreaRect(0,0,0,0),
	LastBaseAreaRect(0,0,0,0),
	House(NULL)
{

}


/***********************************************************************************************
 * BaseClass::Is_Built -- Tells if given item in the list has been built yet                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      index      index into base list                                                        *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      true = yes, false = no                                                                 *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/24/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
bool BaseClass::Is_Built(int index) const
{
	if (Get_Building(index) != NULL) {
		return(true);
	}

	StructType build = Nodes[index].Type;
	if (build >= STRUCT_FIRST) {

		BuildingTypeClass const * btype = BuildingTypes[build];
		if (btype->IsWall) {

			CellClass & cellptr = Map[Nodes[index].CellID];
			if (cellptr.Overlay == btype->ToOverlay->HeapID) {
				return(true);
			}

			if (cellptr.Cell_Building() != NULL) {
				return(true);
			}
		}
	}

	return(false);
}


/***********************************************************************************************
 * BaseClass::Get_Building -- Returns ptr to the built building for the given node             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      obj      pointer to building to test                                                   *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      ptr to already-built building, NULL if none                                            *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/24/1995 BRR : Created.                                                                 *
 *   07/30/1996 JLB : Handle arbitrary overlapper list length.                                 *
 *=============================================================================================*/
BuildingClass * BaseClass::Get_Building(int index) const
{
	int i;

	/*
	**	Check the location on the map where this building should be; if it's
	**	there, return a pointer to it.
	*/
	Cell cell = Nodes[index].CellID;

	if (cell != Cell(0, 0)) {

		StructType build = Nodes[index].Type;
		if (build >= STRUCT_FIRST) {

			BuildingTypeClass const * btype = BuildingTypes[build];
			BuildingClass * bldg = Map[cell].Cell_Building();

			if (bldg != NULL && bldg->PositionCell == Nodes[index].CellID && bldg->House == House) {

				if (bldg->Class == btype) {
					return(bldg);
				}

				if (!btype->PowersUpBuilding.empty()) {

					int upgrades = 0;
					for (i = 0; i < ARRAY_SIZE(bldg->Upgrades); i++) {
						if (bldg->Upgrades[i] == btype) {
							upgrades++;
						}
					}

					for (i = 0; i < index; i++) {
						build = Nodes[i].Type;
						if (build >= STRUCT_FIRST && BuildingTypes[build] == btype) {
							if (Nodes[i].CellID == bldg->PositionCoord.As_Cell()) {
								upgrades--;
							}
						}
					}

					if (upgrades > 0) {
						return(bldg);
					}
					return(NULL);
				}
			}
		}
	}

	return(NULL);
}


/***********************************************************************************************
 * BaseClass::Is_Node -- Tells if the given building is part of our base list                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      obj      pointer to building to test                                                   *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      true = building is a node in the list, false = isn't                                   *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/24/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
bool BaseClass::Is_Node(BuildingClass const * obj)
{
	if (Get_Node(obj) != NULL) {
		return(true);
	} else {
		return(false);
	}
}


/***********************************************************************************************
 * BaseClass::Get_Node -- Returns ptr to the node corresponding to given object                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      obj      pointer to building to test                                                   *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      ptr to node                                                                            *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/24/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
BaseNodeClass * BaseClass::Get_Node(BuildingClass const * obj)
{
	if (obj->House == House) {
		for (int i = 0; i < Nodes.Count(); i++) {
			if (obj->Class->HeapID == Nodes[i].Type && obj->Get_Cell() == Nodes[i].CellID) {
				return(&Nodes[i]);
			}
		}
	}

	return(NULL);
}


/***********************************************************************************************
 * BaseClass::Get_Node -- Finds the node that matches the cell specified.                      *
 *                                                                                             *
 *    This routine is used to find a matching node the corresponds to the cell specified.      *
 *                                                                                             *
 * INPUT:   cell  -- The cell to use in finding a match.                                       *
 *                                                                                             *
 * OUTPUT:  Returns a pointer to the matching node if found. If not found, then NULL is        *
 *          returned.                                                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/12/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
BaseNodeClass * BaseClass::Get_Node(Cell const & cell)
{
	for (int index = 0; index < Nodes.Count(); index++) {
		if (cell == Nodes[index].CellID) {
			return(&Nodes[index]);
		}
	}
	return(NULL);
}


/***********************************************************************************************
 * BaseClass::Next_Buildable -- returns ptr to the next node that needs to be built            *
 *                                                                                             *
 * If 'type' is not NONE, returns ptr to the next "hole" in the list of the given type.        *
 * Otherwise, returns ptr to the next hole in the list of any type.                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      type      type of building to check for                                                *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      ptr to a BaseNodeClass, NULL if none                                                   *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/24/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
BaseNodeClass * BaseClass::Next_Buildable(StructType type)
{
	int index = Next_Buildable_Index(type);
	if (index != -1) {
		return(&Nodes[index]);
	}
	return(NULL);
}


/***********************************************************************************************
 * BaseClass::Next_Buildable -- returns ptr to the next node that needs to be built            *
 *                                                                                             *
 * If 'type' is not NONE, returns ptr to the next "hole" in the list of the given type.        *
 * Otherwise, returns ptr to the next hole in the list of any type.                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      type      type of building to check for                                                *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      ptr to a BaseNodeClass, NULL if none                                                   *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/24/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
int BaseClass::Next_Buildable_Index(StructType type)
{
	/*
	**	Loop through all node entries, returning a pointer to the first
	**	un-built one that matches the requested type.
	*/
	for (int i = 0; i < Nodes.Count(); i++) {

		/*
		**	For STRUCT_NONE, return the first hole found
		*/
		if (type == STRUCT_NONE) {
			if (!Is_Built(i)) {
				return(i);
			}

		} else {

			/*
			**	For a "real" building type, return the first hold for that type
			*/
			if (Nodes[i].Type==type && !Is_Built(i)) {
				return(i);
			}
		}
	}


// If no entry could be found, then create a fake one that will allow
// placement of the building. Make it static and reuse the next time this
// routine is called.

	return(-1);
}


/***********************************************************************************************
 * BaseClass::Read_INI -- INI reading routine                                                  *
 *                                                                                             *
 * INI entry format:                                                                           *
 *      BLDG=COORDINATE                                                                        *
 *      BLDG=COORDINATE                                                                        *
 *        ...                                                                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      buffer      pointer to loaded INI file                                                 *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      This routines assumes there is only one base defined for the scenario.                 *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/24/1995 BRR : Created.                                                                 *
 *   02/20/1996 JLB : Fixed to know what house to build base from.                             *
 *=============================================================================================*/
void BaseClass::Read_INI(CCINIClass const & ini, char const * hname)
{
	char buf[128];
	char uname[12];
	BaseNodeClass node;						// node to add to list

	PercentBuilt = ini.Get_Int(hname, "PercentBuilt", PercentBuilt);

	/*
	**	Read the number of buildings that will go into the base node list
	*/
	int count = ini.Get_Int(hname, "NodeCount", 0);

	/*
	**	Read each entry in turn, in the same order they were written out.
	*/
	for (int i = 0; i < count; i++) {

		/*
		**	Get an INI entry
		*/
		sprintf(uname,"%03d",i);
		ini.Get_String(hname, uname, NULL, buf, sizeof(buf));

		/*
		**	Set the node's building type
		*/
		if (buf[0] == '-') {
			node.Type = (StructType)atoi(strtok(buf, ","));
		} else {
			node.Type = BuildingTypeClass::From_Name(strtok(buf,","));
		}

		/*
		**	Read & set the node's coordinate
		*/
		Cell cell;
		cell.X = atoi(strtok(NULL,","));
		cell.Y = atoi(strtok(NULL,","));
		node.CellID = cell;

		/*
		**	Add this node to the Base's list
		*/
		Nodes.Add(node);
	}
}


/***********************************************************************************************
 * BaseClass::Write_INI -- Writes all the base information to the INI database.                *
 *                                                                                             *
 *    Use this routine to write all prebuild base information to the INI database specified.   *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database to store the data to.                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   If there was any preexisting prebuild base data in the database, it will be     *
 *             be erased by this routine.                                                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void BaseClass::Write_INI(CCINIClass & ini, char const * hname)
{
	ini.Put_Int(hname, "PercentBuilt", PercentBuilt);

	/*
	**	Save the # of buildings in the Nodes list.  This is essential because
	**	they must be read in the same order they were created, so "000" must be
	**	read first, etc.
	*/
	ini.Put_Int(hname, "NodeCount", Nodes.Count());

	/*
	**	Write each entry into the INI
	*/
	for (int i = 0; i < Nodes.Count(); i++) {
		char buf[128];
		char uname[12];

		sprintf(uname,"%03d",i);
		StructType type = Nodes[i].Type;
		if (Nodes[i].Type >= STRUCT_FIRST) {
			sprintf(buf,"%s,%d,%d",
			(char const *)BuildingTypes[Nodes[i].Type]->IniName,
			Nodes[i].CellID.X,
			Nodes[i].CellID.Y);
		} else {
			sprintf(buf,"%d,%d,%d",
			Nodes[i].Type,
			Nodes[i].CellID.X,
			Nodes[i].CellID.Y);
		}

		ini.Put_String(hname, uname, buf);
	}
}

/// <summary>
/// Reads the base back in from a save game.
/// </summary>
/// <returns>Returns with the result reported by the stream read.</returns>
void BaseClass::Serialize(SaveStreamClass & stream)
{
	stream.Serialize(Nodes);
	stream.Serialize(PercentBuilt);
	stream.Serialize(InnerCells);
	stream.Serialize(OuterCells);
	stream.Serialize(PlacementCenter);
	stream.Serialize(BaseAreaRect);
	stream.Serialize(LastBaseAreaRect);
	stream.Serialize(House);
	// INI_NAME -- a constant shared by every base rather than owned by one.
}


/// <summary>
/// Adds the base to the running game state checksum.
/// This routine is used by the multiplayer sync check. Only the node list is submitted,
/// since that is what distinguishes one house's base plan from another.
/// </summary>
void BaseClass::Compute_CRC(CRCEngine & crc) const
{
	crc(Nodes.Count());
	for (int i = 0; i < Nodes.Count(); i++) {
		crc(Nodes[i].Type);
		crc(Nodes[i].CellID.X);
		crc(Nodes[i].CellID.Y);
	}
}
