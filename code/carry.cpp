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

/* $Header: /CounterStrike/CARRY.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CARRY.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 02/26/96                                                     *
 *                                                                                             *
 *                  Last Update : May 10, 1996 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CarryoverClass::CarryoverClass -- Constructor for carry over objects.                     *
 *   CarryoverClass::Create -- Creates a carried over object.                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "carry.h"

#include "building.h"
#include "house.h"
#include "houstype.h"
#include "infantry.h"
#include "unit.h"

/***********************************************************************************************
 * CarryoverClass::CarryoverClass -- Constructor for carry over objects.                       *
 *                                                                                             *
 *    This is the constructor for a carry over object. Such an object is used to record the    *
 *    object that will be "carried over" into a new scenario at some future time.              *
 *                                                                                             *
 * INPUT:   techno   -- Pointer to the object that will be carried over.                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/10/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
CarryoverClass::CarryoverClass(TechnoClass * techno) :
	RTTI(RTTI_NONE),
	CellID(0,0),
	Strength(0),
	House(HOUSE_NONE)
{
	if (techno) {
		RTTI = techno->RTTI;

		switch (RTTI) {
			case RTTI_UNIT:
				Type.Unit = (UnitType)((UnitClass *)techno)->Techno_Type_Class()->Fetch_Heap_ID();
				break;

			case RTTI_BUILDING:
				Type.Building = (StructType)((BuildingClass *)techno)->Techno_Type_Class()->Fetch_Heap_ID();
				break;

			case RTTI_INFANTRY:
				Type.Infantry = (InfantryType)((InfantryClass *)techno)->Techno_Type_Class()->Fetch_Heap_ID();
				break;

			default:
				break;
		}

		House = techno->House->Class->House;
		Strength = techno->Strength;
		CellID = techno->Get_Cell();
	}
}


/***********************************************************************************************
 * CarryoverClass::Create -- Creates a carried over object.                                    *
 *                                                                                             *
 *    Use this routine to convert a carried over object into an actual object that will be     *
 *    placed on the map.                                                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the object successfully created and placed on the map?                   *
 *                                                                                             *
 * WARNINGS:   This routine might not place the object if the old map location was invalid     *
 *             or there are other barriers to the object's creation and placement.             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/10/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CarryoverClass::Create(void) const
{
	TechnoClass * techno = 0;

	HouseClass * house = House_From_HousesType(House);

	switch (RTTI) {
		case RTTI_UNIT:
			techno = new UnitClass(UnitTypes[Type.Unit], house);
			break;

		case RTTI_INFANTRY:
			techno = new InfantryClass(InfantryTypes[Type.Infantry], house);
			break;

		case RTTI_BUILDING:
			techno = new BuildingClass(BuildingTypes[Type.Building], house);
			break;
	}

	if (techno) {
		int oldscen = ScenarioInit;
		techno->Strength = Strength;
		if (RTTI == RTTI_INFANTRY) {
			ScenarioInit = 0;
		}
		techno->Unlimbo(CellID);
		if (RTTI == RTTI_INFANTRY) {
			ScenarioInit = oldscen;
		}
	}

	return(false);
}
