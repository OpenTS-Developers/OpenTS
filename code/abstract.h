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

/* $Header: /CounterStrike/ABSTRACT.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : ABSTRACT.H                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 01/26/95                                                     *
 *                                                                                             *
 *                  Last Update : January 26, 1995 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "coord.h"
#include "globals.h"
#include "stimer.h"

#include "house.hh"
#include "rtti.hh"

#include "persist.h"

class AbstractTypeClass;
class CRCEngine;
class SaveStreamClass;
class HouseClass;
class UnitClass;
class TagClass;
class ObjectClass;
class TechnoClass;
class InfantryClass;
class FootClass;
class CellClass;
class BuildingClass;
class BuildingTypeClass;
class AircraftClass;
class IsometricTileClass;
class MonoClass;

/*
**	This class is the base class for all game objects that have an existence on the
**	battlefield.
*/
class AbstractClass : public IPersistent
{
	public:

	protected:
		/*
		 * These carry the record a class describes through Serialize; the record is the
		 * swizzle identity followed by whatever members the class names. Load and Save
		 * call them, so a class only overrides those when something must happen before
		 * the members are read -- dropping a registration keyed by the identity the read
		 * is about to replace, say.
		 */
		HRESULT Save_Members(SaveStreamClass & stream, BOOL cleardirty);
		HRESULT Load_Members(SaveStreamClass & stream);

	public:

		/*
		**	This specifies the type of object and the unique ID number
		**	associated with it. The ID number happens to match the index into
		**	the object heap appropriate for this object type.
		*/
		__declspec(property(get = Fetch_RTTI)) RTTIType RTTI;
		int ID;

		/*
		 * If this object has changed since it was last written out, then this flag will be
		 * true. Save clears it on request.
		 */
		bool Dirty;

		/*
		**	Constructor & destructors.
		*/
		AbstractClass(void);
		virtual ~AbstractClass(void);


		virtual HRESULT Load(SaveStreamClass & stream) override;
		virtual HRESULT Save(SaveStreamClass & stream, BOOL cleardirty) override;

		virtual int What_Am_I(void) const;
		virtual int Fetch_ID(void) const;
		virtual void Create_ID(void);

		AbstractClass & operator = (const AbstractClass & that)
		{
			ID = that.ID;
			Dirty = that.Dirty;
			return(*this);
		}

		/*
		 * Lists this object's members for the save game. An implementation serializes its
		 * base class first and then names every member it owns in the order the header
		 * declares them, so that the same description serves saving and loading.
		 */
		virtual void Serialize(SaveStreamClass & stream);

		/*
		 * Restores whatever the record could not carry -- artwork fetched by name, tables
		 * shared with other objects, registrations that depend on the loaded identity.
		 * Load_Object calls this once the record has been checked, so an object never takes
		 * its place in the map or a side table while its record is still in doubt. An
		 * implementation chains to its base first and never touches the stream.
		 */
		virtual void Post_Load(void);

		virtual void Init(void);
		virtual void Detach(AbstractClass const * target, bool all = true);

		/*
		**	Query functions.
		*/
		virtual RTTIType Fetch_RTTI(void) const = 0;
		virtual void Compute_CRC(CRCEngine &) const;

		virtual HousesType Owner(void) const;
		virtual HouseClass * Owner_HouseClass(void) const;

		virtual int Fetch_Heap_ID(void) const;

		virtual bool Is_Inactive(void) const;

		bool Is_Techno(void) const;

		/*
		**	Coordinate query support functions.
		*/
		virtual Coord Center_Coord(void) const;
		virtual Coord Destination_Coord(void) const;

		virtual bool On_Ground(void) const;
		virtual bool In_Air(void) const;

		virtual Coord As_Coord(void) const;

		/*
		**	AI.
		*/
		virtual void AI(void);

		/*
		**	Scenario and debug support.
		*/
#ifdef _DEBUG
		virtual void Debug_Dump(MonoClass *mono) const {};
#endif

		/*
		 * Dynamic casts from AbstractClass to derived class.
		 *
		 * These must only be implemented in their respective modules!
		 */
		UnitClass * As_UnitClass(void);
		TagClass * As_TagClass(void);
		ObjectClass * As_ObjectClass(void);
		InfantryClass * As_InfantryClass(void);
		TechnoClass * As_TechnoClass(void);
		FootClass * As_FootClass(void);
		CellClass * As_CellClass(void);
		BuildingClass * As_BuildingClass(void);
		BuildingTypeClass * As_BuildingTypeClass(void);
		AircraftClass * As_AircraftClass(void);
		AbstractTypeClass * As_AbstractTypeClass(void);
		IsometricTileClass * As_IsometricTileClass(void);

		const UnitClass * As_UnitClass(void) const;
		const TagClass * As_TagClass(void) const;
		const ObjectClass * As_ObjectClass(void) const;
		const InfantryClass * As_InfantryClass(void) const;
		const TechnoClass * As_TechnoClass(void) const;
		const FootClass * As_FootClass(void) const;
		const CellClass * As_CellClass(void) const;
		const BuildingClass * As_BuildingClass(void) const;
		const BuildingTypeClass * As_BuildingTypeClass(void) const;
		const AircraftClass * As_AircraftClass(void) const;
		const AbstractTypeClass * As_AbstractTypeClass(void) const;
		const IsometricTileClass * As_IsometricTileClass(void) const;
};
