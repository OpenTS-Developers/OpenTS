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

/* $Header: /CounterStrike/TARGET.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TARGET.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : September 10, 1993                                           *
 *                                                                                             *
 *                  Last Update : July 16, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   As_Aircraft -- Converts the target value into an aircraft pointer.                        *
 *   As_Animation -- Converts target value into animation pointer.                             *
 *   As_Building -- Converts a target value into a building object pointer.                    *
 *   As_Bullet -- Converts the target into a bullet pointer.                                   *
 *   As_Cell -- Converts a target value into a cell number.                                    *
 *   As_Coord -- Converts a target value into a coordinate value.                              *
 *   As_Infantry -- If the target is infantry, return a pointer to it.                         *
 *   As_Movement_Coord -- Fetches coordinate if trying to move to this target.                 *
 *   As_Object -- Converts a target value into an object pointer.                              *
 *   As_Target -- Converts a cell into a target value.                                         *
 *   As_Target -- Converts a coordinate into a target value.                                   *
 *   As_Team -- Converts a target number into a team pointer.                                  *
 *   As_TeamType -- Converts a target into a team type pointer.                                *
 *   As_Techno -- Converts a target value into a TechnoClass pointer.                          *
 *   As_TechnoType -- Convert the target number into a techno type class pointer.              *
 *   As_Trigger -- Converts specified target into a trigger pointer.                           *
 *   As_TriggerType -- Convert the specified target into a trigger type.                       *
 *   As_Unit -- Converts a target value into a unit pointer.                                   *
 *   As_Vessel -- Converts a target number into a vessel pointer.                              *
 *   TClass::TClass -- Constructor for target from object pointer.                             *
 *   TargetClass::As_Object -- Converts a target into an object pointer.                       *
 *   TargetClass::As_Techno -- Converts a target into a techno object pointer.                 *
 *   Target_Legal -- Determines if the specified target is legal.                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "target.h"

#include "_rtti.h"
#include "aircraft.h"
#include "anim.h"
#include "building.h"
#include "bullet.h"
#include "cell.h"
#include "house.h"
#include "index.h"
#include "infantry.h"
#include "tag.h"
#include "tagtype.h"
#include "team.h"
#include "teamtype.h"
#include "terrain.h"
#include "trigger.h"
#include "trigtype.h"
#include "unit.h"

IndexClass<int, AbstractClass *> TargetTracker;


/// <summary>
/// Creates a target that refers to a game object.
/// This routine is the usual way a target is made. A cell is recorded by its location so
/// that it survives being reallocated; everything else is recorded by its tracker ID. A
/// NULL pointer produces an empty target.
/// </summary>
/// <param name="ptr">Pointer to the object the target should refer to.</param>
TargetClass::TargetClass(AbstractClass const * ptr)
{
	if (ptr != NULL) {
		const CellClass *cellptr = ptr->As_CellClass();
		if (cellptr != NULL) {
			RTTI = RTTI_CELL;
			Cell cell = cellptr->CellID;
			ID = (cell.X + (1000 * cell.Y));
		} else {
			RTTI = RTTI_ABSTRACT;
			ID = ptr->Fetch_ID();
		}
	} else {
		RTTI = RTTI_NONE;
		ID = 0;
	}
}


/// <summary>
/// Creates a target that refers to a map cell.
/// A cell of CELL_NONE produces an empty target, so the caller need not screen out the
/// no-cell case before building one.
/// </summary>
/// <param name="cell">The cell to build the target from.</param>
TargetClass::TargetClass(Cell const & cell)
{
	if (cell == CELL_NONE) {
		RTTI = RTTI_NONE;
		ID = 0;
	} else {
		RTTI = RTTI_CELL;
		ID = (cell.X + (1000 * cell.Y));
	}
}


/// <summary>
/// Creates a target that refers to the cell under a coordinate.
/// Use this routine when a piece of logic has a location in hand but wants to aim at the
/// map rather than at whatever happens to be standing there.
/// </summary>
/// <param name="coord">The coordinate to build the target from.</param>
TargetClass::TargetClass(Coord const & coord)
{
	RTTI = RTTI_CELL;
	ID = ((coord.X / CELL_LEPTON_W) + (1000 * (coord.Y / CELL_LEPTON_H)));
}


/// <summary>
/// Converts a target into a type class pointer.
/// This routine is used when the target is expected to name one of the static type objects
/// rather than something on the map. If it doesn't, then NULL is returned.
/// </summary>
/// <returns>Returns with a pointer to the type object that this target represents, or NULL
/// if it doesn't represent one.</returns>
AbstractTypeClass * xTargetClass::As_TypeClass(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<AbstractTypeClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/// <summary>
/// Converts a target into a tag pointer.
/// This routine is used to convert the target object into a pointer to the tag it
/// represents. If it represents something else, then NULL is returned.
/// </summary>
/// <returns>Returns with a pointer to the tag that this target represents, or NULL if it
/// doesn't represent one.</returns>
TagClass * xTargetClass::As_Tag(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<TagClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/// <summary>
/// Converts a target into a tag type pointer.
/// This routine is used to convert the target object into a pointer to the tag type it
/// represents. If it represents something else, then NULL is returned.
/// </summary>
/// <returns>Returns with a pointer to the tag type that this target represents, or NULL if
/// it doesn't represent one.</returns>
TagTypeClass * xTargetClass::As_TagType(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<TagTypeClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/***********************************************************************************************
 * TargetClass::As_Abstract -- Converts a target into an abstract object pointer.              *
 *                                                                                             *
 *    If the target represents an object of some type, then this routine will return a         *
 *    pointer to the object. Otherwise it will return NULL.                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the object that this target represents or NULL if it     *
 *          doesn't represent a target.                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
AbstractClass * xTargetClass::As_Abstract(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<AbstractClass *>(TargetTracker[ID]));
	}

	if (RTTI == RTTI_CELL) {
		Cell cell(ID % 1000, ID / 1000);
		return(&Map[cell]);
	}
	return(NULL);
}


/***********************************************************************************************
 * TargetClass::As_Techno -- Converts a target into a techno object pointer.                   *
 *                                                                                             *
 *    This routine is used to convert the target object into a pointer to a techno class       *
 *    object. If the target doesn't specify a techno class object, then NULL is returned.      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the techno class object that this target represents or   *
 *          else it returns NULL.                                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
TechnoClass * xTargetClass::As_Techno(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(Dynamic_Cast<TechnoClass *>((ObjectClass *)TargetTracker[ID]));
	}
	return(NULL);
}


/***********************************************************************************************
 * TargetClass::As_Object -- Converts a target into an object pointer.                         *
 *                                                                                             *
 *    If the target represents an object of some type, then this routine will return a         *
 *    pointer to the object. Otherwise it will return NULL.                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the object that this target represents or NULL if it     *
 *          doesn't represent a target.                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectClass * xTargetClass::As_Object(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<ObjectClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/// <summary>
/// Converts a target into a foot object pointer.
/// This routine is used when only a mobile object will do. If the target represents a
/// building or something that isn't an object at all, then NULL is returned.
/// </summary>
/// <returns>Returns with a pointer to the mobile object that this target represents, or
/// NULL if it doesn't represent one.</returns>
FootClass * xTargetClass::As_Foot(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<FootClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/***********************************************************************************************
 * As_Trigger -- Converts specified target into a trigger pointer.                             *
 *                                                                                             *
 *    This routine will convert the specified target number into a trigger pointer.            *
 *                                                                                             *
 * INPUT:   target   -- The target number to convert.                                          *
 *                                                                                             *
 * OUTPUT:  Returns with the trigger pointer that the specified target number represents. If   *
 *          it doesn't represent a legal trigger object, then NULL is returned.                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
TriggerClass * xTargetClass::As_Trigger(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<TriggerClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/// <summary>
/// Converts a target into a house pointer.
/// This routine is used to convert the target object into a pointer to the house class
/// object it represents. If the target isn't a house, then NULL is returned.
/// </summary>
/// <returns>Returns with a pointer to the house that this target represents, or NULL if it
/// doesn't represent one.</returns>
HouseClass * xTargetClass::As_House(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<HouseClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/***********************************************************************************************
 * As_TechnoType -- Convert the target number into a techno type class pointer.                *
 *                                                                                             *
 *    This routine will conver the specified target number into a pointer to the techno        *
 *    type class that it represents.                                                           *
 *                                                                                             *
 * INPUT:   target   -- The target number to convert.                                          *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the TechnoTypeClass object that the target number        *
 *          represents. If it doesn't represent that kind of object, then NULL is returned.    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/16/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
TechnoTypeClass * xTargetClass::As_TechnoType(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<TechnoTypeClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/***********************************************************************************************
 * As_TriggerType -- Convert the specified target into a trigger type.                         *
 *                                                                                             *
 *    This routine will conver the target number into a pointer to the trigger type it         *
 *    represents.                                                                              *
 *                                                                                             *
 * INPUT:   target   -- The target value to convert into a trigger type pointer.               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the trigger type object that the specified target value  *
 *          represents. If it doesn't represent a trigger type, then NULL is returned.         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/16/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
TriggerTypeClass * xTargetClass::As_TriggerType(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<TriggerTypeClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/***********************************************************************************************
 * As_TeamType -- Converts a target into a team type pointer.                                  *
 *                                                                                             *
 *    This routine will convert the specified target number into a team type pointer.          *
 *                                                                                             *
 * INPUT:   target   -- The target number to convert.                                          *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the team type represented by the target number. If the   *
 *          target number doesn't represent a legal team type, then NULL is returned.          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
TeamTypeClass * xTargetClass::As_TeamType(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<TeamTypeClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/// <summary>
/// Converts a target into a terrain object pointer.
/// This routine is used to convert the target object into a pointer to a terrain class
/// object. If the target doesn't specify a terrain object, then NULL is returned.
/// </summary>
/// <returns>Returns with a pointer to the terrain object that this target represents, or
/// NULL if it doesn't represent one.</returns>
TerrainClass * xTargetClass::As_Terrain(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<TerrainClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/***********************************************************************************************
 * As_Bullet -- Converts the target into a bullet pointer.                                     *
 *                                                                                             *
 *    This routine will convert the specified target number into a bullet pointer.             *
 *                                                                                             *
 * INPUT:   target   -- The target number to convert.                                          *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the bullet it specifies. If the target doesn't refer to  *
 *          a legal bullet, then NULL is returned.                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
BulletClass * xTargetClass::As_Bullet(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<BulletClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/***********************************************************************************************
 * As_Animation -- Converts target value into animation pointer.                               *
 *                                                                                             *
 *    This routine will convert the specified target number into an animation pointer.         *
 *                                                                                             *
 * INPUT:   target   -- The target number to convert into an animation pointer.                *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the legal animation that this target represents. If it   *
 *          doesn't represent a legal animation, then NULL is returned.                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
AnimClass * xTargetClass::As_Anim(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<AnimClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/***********************************************************************************************
 * As_Team -- Converts a target number into a team pointer.                                    *
 *                                                                                             *
 *    This routine will convert the specified target number into a team pointer.               *
 *                                                                                             *
 * INPUT:   target   -- The target number to convert.                                          *
 *                                                                                             *
 * OUTPUT:  Returns with the team object that the specified target number represents. If it    *
 *          doesn't represent a legal team then NULL is returned.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
TeamClass * xTargetClass::As_Team(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<TeamClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/***********************************************************************************************
 * As_Infantry -- If the target is infantry, return a pointer to it.                           *
 *                                                                                             *
 *    This routine will translate the specified target value into an infantry pointer if the   *
 *    target actually represents an infantry object.                                           *
 *                                                                                             *
 * INPUT:   target   -- The target to convert to a pointer.                                    *
 *                                                                                             *
 * OUTPUT:  Returns a pointer to the infantry object that this target value represents. If     *
 *          the target doesn't represent an infantry object, then return NULL.                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
InfantryClass * xTargetClass::As_Infantry(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<InfantryClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/***********************************************************************************************
 * As_Unit -- Converts a target value into a unit pointer.                                     *
 *                                                                                             *
 *    This routine is used to convert the target value specified into a pointer to a unit      *
 *    object.                                                                                  *
 *                                                                                             *
 * INPUT:   target   -- The target value to convert into a unit pointer.                       *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the unit the target value represents or NULL if not      *
 *          a unit.                                                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/27/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
UnitClass * xTargetClass::As_Unit(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<UnitClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/***********************************************************************************************
 * As_Building -- Converts a target value into a building object pointer.                      *
 *                                                                                             *
 *    This routine is used to convert the target value specified into a building pointer.      *
 *                                                                                             *
 * INPUT:   target   -- The target value to convert from.                                      *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the building object that the target value represents.    *
 *          If it doesn't represent a building, then return NULL.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/27/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
BuildingClass * xTargetClass::As_Building(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<BuildingClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/***********************************************************************************************
 * As_Aircraft -- Converts the target value into an aircraft pointer.                          *
 *                                                                                             *
 *    This routine will convert the specified target value into an aircraft object pointer.    *
 *                                                                                             *
 * INPUT:   target   -- The target value to convert.                                           *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the aircraft that this target value represents. If the   *
 *          specified target value doesn't represent an aircraft, then NULL is returned.       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/27/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
AircraftClass * xTargetClass::As_Aircraft(void) const
{
	if (RTTI == RTTI_ABSTRACT) {
		return(dynamic_cast<AircraftClass *>(TargetTracker[ID]));
	}
	return(NULL);
}


/***********************************************************************************************
 * As_Cell -- Converts a target value into a cell number.                                      *
 *                                                                                             *
 *    This routine is used to convert the target value specified, into a cell value. This is   *
 *    necessary for find path and other procedures that need a cell value.                     *
 *                                                                                             *
 * INPUT:   target   -- The target value to convert to a cell value.                           *
 *                                                                                             *
 * OUTPUT:  Returns with the target value expressed as a cell location.                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/27/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
CellClass * xTargetClass::As_Cell(void) const
{
	if (RTTI == RTTI_CELL) {
		Cell cell(ID % 1000, ID / 1000);
		return(&Map[cell]);
	}
	return(NULL);
}
