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

/* $Header: /CounterStrike/ABSTRACT.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : ABSTRACT.CPP                                                 *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 01/26/95                                                     *
 *                                                                                             *
 *                  Last Update : July 10, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   AbstractClass::Debug_Dump -- Display debug information to mono screen.                    *
 *   AbstractClass::Distance -- Determines distance to target.                                 *
 *   AbstractTypeClass::AbstractTypeClass -- Constructor for abstract type objects.            *
 *   AbstractTypeClass::Coord_Fixup -- Performs custom adjustments to location coordinate.     *
 *   AbstractTypeClass::Full_Name -- Returns the full name (number) of this object type.       *
 *   AbstractTypeClass::Get_Ownable -- Fetch the ownable bits for this object.                 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "abstract.h"

#include "_rtti.h"
#include "coord.h"
#include "crc.h"
#include "globals.h"
#include "savestream.h"
#include "scenario.h"
#include "swizzle.h"
#include "vector.h"

#include <cassert>

/// <summary>
/// Default constructor for abstract objects.
/// This routine puts the object into its neutral state: no unique ID assigned
/// yet, and nothing pending for the save system.
/// </summary>
AbstractClass::AbstractClass(void) :
	ID(-1),
	Dirty(false)
{
}


/// <summary>
/// Destructor for abstract objects.
/// </summary>
AbstractClass::~AbstractClass(void)
{
	/// empty
}


/// <summary>
/// Fetches the RTTI type of this object.
/// The engine carries its own run-time type scheme in place of the C++ feature.
/// Only instantiable classes report a type of their own; the intermediate layers
/// of the hierarchy report RTTI_NONE.
/// </summary>
/// <returns>Returns with the RTTI type constant for this object.</returns>
int AbstractClass::What_Am_I(void) const
{
	return(RTTI);
}


/// <summary>
/// Fetches this object's unique identifier.
/// </summary>
/// <returns>Returns with the unique ID, or -1 if none has been assigned yet.</returns>
int AbstractClass::Fetch_ID(void) const
{
	return(ID);
}


/// <summary>
/// Assigns this object a unique identifier.
/// The ID is drawn from the scenario's running counter, so objects created
/// before a scenario exists are all given an ID of zero.
/// </summary>
void AbstractClass::Create_ID(void)
{
	ID = (Scen == NULL ? 0 : Scen->Get_Unique_ID());
}


/// <summary>
/// Writes this object to the save stream.
/// </summary>
/// <param name="stream">The stream to write to.</param>
/// <param name="cleardirty">Should the object be marked clean once it has been written?</param>
/// <returns>bool; Was the record written whole?</returns>
bool AbstractClass::Save(SaveStreamClass & stream, bool cleardirty)
{
	return(Save_Members(stream, cleardirty));
}


/// <summary>
/// Reads this object back from the save stream.
/// </summary>
/// <param name="stream">The stream to read from.</param>
/// <returns>bool; Was the record read whole?</returns>
bool AbstractClass::Load(SaveStreamClass & stream)
{
	return(Load_Members(stream));
}


/// <summary>
/// Writes the members this object describes out to the save stream.
/// The object's swizzle identity goes out first, and the members follow
/// in the order Serialize names them.
/// </summary>
/// <param name="stream">The stream to write to.</param>
/// <param name="cleardirty">Should the object be marked clean once it has been written?</param>
/// <returns>bool; Was the record written whole?</returns>
bool AbstractClass::Save_Members(SaveStreamClass & stream, bool cleardirty)
{
	SwizzleIDType id = Swizzler.ID_Of(this);
	stream.Serialize(id);
	Serialize(stream);
	if (!stream.Was_Error() && cleardirty) {
		Dirty = false;
	}
	return(!stream.Was_Error());
}


/// <summary>
/// Reads the members this object describes back from the save stream.
/// The saved identity is handed to the swizzle system so that pointers elsewhere in the
/// save game can be remapped onto this object, and the members follow.
/// </summary>
/// <param name="stream">The stream to read from.</param>
/// <returns>bool; Was the record read whole?</returns>
bool AbstractClass::Load_Members(SaveStreamClass & stream)
{
	SwizzleIDType id = 0;
	stream.Serialize(id);
	if (stream.Was_Error()) {
		return(false);
	}
	Swizzle_Here_I_Am(id, this);

	// A nested record borrows the stream, so the owner's context is put back afterwards.
	char const * const outertype = stream.Context_Type();
	SwizzleIDType const outerid = stream.Context_ID();
	stream.Set_Context(typeid(*this).name(), id);
	Serialize(stream);
	stream.Set_Context(outertype, outerid);

	return(!stream.Was_Error());
}


/// <summary>
/// Restores the state a game object could not carry in its record.
/// The bare abstract object carries nothing of the sort, so there is nothing to do.
/// </summary>
void AbstractClass::Post_Load(void)
{
}


/// <summary>
/// Lists the members every game object carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void AbstractClass::Serialize(SaveStreamClass & stream)
{
	stream.Serialize(ID);
	stream.Serialize(Dirty);
}


/// <summary>
/// Folds this object's state into a running CRC.
/// The multiplayer sync check walks every object each frame and accumulates its
/// state; a CRC that differs between machines means the simulations have drifted
/// apart. Derived classes add their own state on top of this.
/// </summary>
/// <param name="crc">The running CRC to fold this object's state into.</param>
void AbstractClass::Compute_CRC(CRCEngine & crc) const
{
	assert(this != NULL);

	crc(ID);
	crc(Dirty);
}


/// <summary>
/// Determines if this object is inactive.
/// The bare abstract object is never a participant in the game, so it always
/// reports inactive. Objects that can be placed in the world override this.
/// </summary>
/// <returns>bool; Is the object inactive?</returns>
bool AbstractClass::Is_Inactive(void) const
{
	assert(this != NULL);

	return(true);
}


/// <summary>
/// Determines if this object is a techno object.
/// Techno objects are the ownable and targetable ones -- infantry, units,
/// aircraft and buildings.
/// </summary>
/// <returns>bool; Is this object derived from TechnoClass?</returns>
bool AbstractClass::Is_Techno(void) const
{
	return(::Dynamic_Cast<TechnoClass const *>(this) != NULL);
}


/// <summary>
/// Resets this object to its start of scenario state.
/// The bare abstract object carries no scenario state, so there is nothing to do.
/// </summary>
void AbstractClass::Init(void)
{
	assert(this != NULL);
}


/// <summary>
/// Severs any references this object holds to the target.
/// This routine is called on every object when one of them is about to be
/// destroyed, so that nothing is left pointing at the corpse.
/// </summary>
/// <param name="target">The object that is about to disappear.</param>
/// <param name="all">Should every reference be severed, including cargo and radio links?</param>
void AbstractClass::Detach(AbstractClass const * target, bool all)
{
	assert(this != NULL);

}


/// <summary>
/// Fetches the house that owns this object.
/// The bare abstract object belongs to nobody. Ownable objects override this.
/// </summary>
/// <returns>Returns with the owning house, or HOUSE_NONE when the object is unowned.</returns>
HousesType AbstractClass::Owner(void) const
{
	assert(this != NULL);

	return(HOUSE_NONE);
}


/// <summary>
/// Fetches the house object that owns this object.
/// </summary>
/// <returns>Returns with a pointer to the owning house, or NULL when the object
/// is unowned.</returns>
HouseClass *AbstractClass::Owner_HouseClass(void) const
{
	assert(this != NULL);

	return(NULL);
}


/// <summary>
/// Fetches this object's index within its heap.
/// </summary>
/// <returns>Returns with the heap index, or zero for an object that does not
/// live in a heap.</returns>
int AbstractClass::Fetch_Heap_ID(void) const
{
	assert(this != NULL);

	return(0);
}


/***********************************************************************************************
 * ObjectClass::Center_Coord -- Fetches the center coordinate for the object.                  *
 *                                                                                             *
 *    This routine will return the center coordinate for the object. The center coordinate is  *
 *    typically the coordinate recorded in the object structure. Exceptions to this include    *
 *    the trees and other terrain elements.                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the coordinate that is considered the center point of this object.         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
Coord AbstractClass::Center_Coord(void) const
{
	assert(this != NULL);

	return(COORD_NONE);
}


/// <summary>
/// Fetches the coordinate that others should move toward.
/// Objects with a distinguished approach point -- a building's entry cell, say --
/// override this. By default an object is approached at its center.
/// </summary>
/// <returns>Returns with the coordinate another object should head for.</returns>
Coord AbstractClass::Destination_Coord(void) const
{
	assert(this != NULL);

	return(Center_Coord());
}


/// <summary>
/// Determines if this object is resting on the ground.
/// </summary>
/// <returns>bool; Is the object on the ground?</returns>
bool AbstractClass::On_Ground(void) const
{
	return(false);
}


/// <summary>
/// Determines if this object is airborne.
/// </summary>
/// <returns>bool; Is the object in the air?</returns>
bool AbstractClass::In_Air(void) const
{
	return(false);
}


/// <summary>
/// Fetches this object's coordinate.
/// Defaults to the center coordinate; objects that are located by some other
/// reference point override this.
/// </summary>
/// <returns>Returns with the object's coordinate.</returns>
Coord AbstractClass::As_Coord(void) const
{
	assert(this != NULL);

	return(Center_Coord());
}


/// <summary>
/// Handles the per frame logic for this object.
/// This routine is called once per game frame for every active object. The bare
/// abstract object has no logic of its own.
/// </summary>
void AbstractClass::AI(void)
{
	assert(this != NULL);
}
