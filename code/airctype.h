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

#pragma once

#include "techtype.h"

#include "aircraft.hh"


/****************************************************************************
**	The various aircraft types are controlled by object types of
**	this class.
*/
class AircraftTypeClass : public TechnoTypeClass
{
		typedef TechnoTypeClass BASECLASS;

	public:

		/*
		**	This is the kind of aircraft identifier number.
		*/
		AircraftType HeapID;

		/*
		 * Can this aircraft lift a vehicle and ferry it elsewhere? A carryall offers the
		 * "tote" action over a friendly vehicle and carries its cargo slung underneath.
		 */
		bool IsCarryall;

		/*
		**	Does this aircraft have a rotor blade (helicopter) type propulsion?
		*/
		bool IsRotorEquipped;	// Is a rotor attached?

		/*
		**	Is there a custom rotor animation stage set for each facing of the aircraft?
		*/
		bool IsRotorCustom;	// Custom rotor sets for each facing?

		/*
		**	Can this aircraft land?  If it can land it is presumed to be controllable by the player.
		*/
		bool IsLandable;

		AircraftTypeClass(char const * ininame = NULL);
		virtual ~AircraftTypeClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_AIRCRAFTTYPE);}
		virtual void Compute_CRC(CRCEngine & crc) const override;
		virtual int Fetch_Heap_ID(void) const override {return(HeapID);}

		static AircraftTypeClass * Find_Or_Make(char const * name);
		static AircraftType From_Name(char const * name);
		static void Init(TheaterType ) {};
		static void One_Time(void);

		virtual bool Read_INI(CCINIClass const & ini) override;
		virtual Point3D Lepton_Dimensions(void) const override;
		virtual bool Create_And_Place(Cell const & cell, HouseClass * house = NULL) const override;
		virtual ObjectClass * Create_One_Of(HouseClass * house) const override;
		virtual Cell const * Occupy_List(bool placement=false) const override;

		static void const * LRotorData;
		static void const * RRotorData;
};
