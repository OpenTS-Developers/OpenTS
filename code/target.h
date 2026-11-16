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

/* $Header: /CounterStrike/TARGET.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TARGET.H                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 25, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 25, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once


#include "abstract.h"

#include "rtti.hh"


inline bool Is_Target_Team(const AbstractClass * target) {return(target != NULL && target->RTTI == RTTI_TEAM);}
inline bool Is_Target_TeamType(const AbstractClass * target) {return(target != NULL && target->RTTI == RTTI_TEAMTYPE);}
inline bool Is_Target_Trigger(const AbstractClass * target) {return(target != NULL && target->RTTI == RTTI_TRIGGER);}
inline bool Is_Target_TriggerType(const AbstractClass * target) {return(target != NULL && target->RTTI == RTTI_TRIGGERTYPE);}
inline bool Is_Target_Infantry(const AbstractClass * target) {return(target != NULL && target->RTTI == RTTI_INFANTRY);}
inline bool Is_Target_Bullet(const AbstractClass * target) {return(target != NULL && target->RTTI == RTTI_BULLET);}
inline bool Is_Target_Terrain(const AbstractClass * target) {return(target != NULL && target->RTTI == RTTI_TERRAIN);}
inline bool Is_Target_Cell(const AbstractClass * target) {return(target != NULL && target->RTTI ==  RTTI_CELL);}
inline bool Is_Target_Unit(const AbstractClass * target) {return(target != NULL && target->RTTI == RTTI_UNIT);}
inline bool Is_Target_Building(const AbstractClass * target) {return(target != NULL && target->RTTI == RTTI_BUILDING);}
inline bool Is_Target_IsoTile(const AbstractClass * target) {return(target != NULL && target->RTTI == RTTI_ISOTILE);}
inline bool Is_Target_Aircraft(const AbstractClass * target) {return(target != NULL && target->RTTI == RTTI_AIRCRAFT);}
inline bool Is_Target_Animation(const AbstractClass * target) {return(target != NULL && target->RTTI == RTTI_ANIM);}
inline bool Is_Target_Tag(const AbstractClass * target) {return(target != NULL && target->RTTI == RTTI_TAG);}

class UnitClass;
class BuildingClass;
class TechnoClass;
class TerrainClass;
class ObjectClass;
class InfantryClass;
class BulletClass;
class TriggerClass;
class TeamClass;
class TeamTypeClass;
class AnimClass;
class AircraftClass;
class CellClass;
class TriggerTypeClass;
class AbstractTypeClass;
class TagClass;
class TagTypeClass;
class FootClass;
class HouseClass;
class TechnoTypeClass;
class Cell;
class Coord;
class CCINIClass;

#pragma pack(push,1)
/*
**	Must not have a constructor since Watcom cannot handle a class that has a constructor if
**	that class object is in a union. Don't use this class for normal purposes. Use the TargetClass
**	instead. The xTargetClass is only used in one module for a special reason -- keep it that way.
*/
class xTargetClass
{
	friend class CCINIClass;
	protected:

		/*
		 * This specifies what kind of thing the target names, and so how the ID is to be
		 * read -- RTTI_ABSTRACT for a game object, RTTI_CELL for a map cell, or RTTI_NONE
		 * for an empty target.
		 */
		int RTTI;

		/*
		 * This is the value that identifies the target within its kind -- the tracker ID for
		 * an object, or the location packed as X + 1000 * Y for a cell. An object is recorded
		 * by ID rather than by pointer so the target survives the object being reallocated.
		 */
		int ID;

	public:

		// conversion operator to RTTIType
		operator RTTIType (void) const {return(RTTIType(RTTI));}

		// comparison operator
		bool operator == (xTargetClass & tgt) {return(RTTI == tgt.RTTI && ID == tgt.ID);}

		unsigned Value(void) const {return(ID);};

		void Invalidate(void) {RTTI = RTTI_NONE; ID = 0;}
		bool Is_Valid(void) const {return(RTTI != RTTI_NONE);}

		int Encode(void)
		{
			int value;
			if (RTTI == RTTI_NONE) {
				value = -1;
			} else {
				value = ((RTTI << 24) | (ID & 0x00FFFFFF));
			}
			return(value);
		}

		xTargetClass Decode(int value)
		{
			Invalidate();

			RTTI = (value >> 24) & 0xFF;
			ID = value   & 0x00FFFFFF;

			return(*this);
		}

		AbstractTypeClass * As_TypeClass(void) const;
		AbstractClass * As_Abstract(void) const;
		TechnoClass * As_Techno(void) const;
		ObjectClass * As_Object(void) const;
		CellClass * As_Cell(void) const;

		/*
		**	Helper routines to combine testing for, and fetching a pointer to, the
		**	type of object indicated.
		*/
		TriggerTypeClass * As_TriggerType(void) const;
		TeamTypeClass * As_TeamType(void) const;
		TerrainClass * As_Terrain(void) const;
		BulletClass * As_Bullet(void) const;
		AnimClass * As_Anim(void) const;
		TeamClass * As_Team(void) const;
		InfantryClass * As_Infantry(void) const;
		UnitClass * As_Unit(void) const;
		BuildingClass * As_Building(void) const;
		AircraftClass * As_Aircraft(void) const;

		TagClass * As_Tag(void) const;
		TagTypeClass * As_TagType(void) const;
		FootClass * As_Foot(void) const;
		TriggerClass * As_Trigger(void) const;
		HouseClass * As_House(void) const;
		TechnoTypeClass * As_TechnoType(void) const;
};

/*
**	This class only serves as a wrapper to the xTargetClass. This class must not define any members except
**	for the constructors. This is because the xTargetClass is used in a union and this target object is
**	used as its initializer. If this class had any extra members they would not be properly copied and
**	communicated to the other machines in a network/modem game. Combining this class with xTargetClass would
**	be more efficient, but Watcom doesn't allow class objects that have a constructor to be part of a union [even
**	if the class object has a default constructor!].
*/
class TargetClass : public xTargetClass
{
	public:

		TargetClass(void) {Invalidate();}

		TargetClass(Cell const &cell);
		TargetClass(Coord const &coord);

		TargetClass(AbstractClass const * ptr);
};
#pragma pack(pop)

template<class INDEX, class T> class IndexClass;

extern IndexClass<int, AbstractClass *> TargetTracker;
