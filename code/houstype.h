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

#include "abstype.h"

#include "house.hh"
#include "side.hh"

/**********************************************************************
**	Each house has certain unalienable characteristics. This structure
**	elaborates these.
*/
class HouseTypeClass : public AbstractTypeClass
{
		typedef AbstractTypeClass BASECLASS;

	public:

		/*
		**	This is the house number (enum). This is a unique identification
		**	number for the house.
		*/
		HousesType HeapID;
		HousesType House;

		/*
		 * This is the side this house fights for, expressed as an index into the Sides list.
		 * The house is added to that side's roster as the rules are read, and the side then
		 * decides such things as which music it may play. If SIDE_NONE, then it belongs to
		 * no side.
		 */
		SideType Side;

		/*
		**	This points to the default remap table for this house.
		*/
		int Scheme;

		/*
		**	This controls the various general adjustments to the house owned
		**	unit and building ratings. The default value for these ratings is
		**	a fixed point number of 1.0.
		*/
		double FirepowerBias;
		double GroundspeedBias;
		double AirspeedBias;
		double ArmorBias;
		double ROFBias;
		double CostBias;
		double BuildSpeedBias;

		/*
		**	This is the filename suffix to use when creating a house specific
		**	file name. It is three characters long.
		*/
		char Suffix[4];

		/*
		**	This is a unique ASCII character used when constructing filenames. It
		**	serves a similar purpose as the "Suffix" element, but is only one
		**	character long.
		*/
		char Prefix;

		/*
		 * If this house may be picked as a country by a player setting up a multiplayer or
		 * skirmish game, then this flag will be true. It is what keeps the story only houses
		 * out of the country lists offered in the lobby.
		 */
		bool IsMultiplay;

		/*
		 * If this house takes no part in the multiplayer contest, then this flag will be
		 * true. A passive house is not treated as an opponent -- it is left out of the
		 * victory test and the score screen, and the computer players will not hunt for it.
		 */
		bool IsMultiplayPassive;

		/*
		 * If a wall may be credited to this house, then this flag will be true. A wall
		 * section belongs to whichever wall owning house has a building nearest it, so a
		 * house without this flag never inherits the walls around it.
		 */
		bool IsWallOwner;

		/// Unused
		bool IsSmartAI;

		//------------------------------------------------------------------------
		HouseTypeClass(char const * ininame = NULL);
		virtual ~HouseTypeClass() override;

		virtual ClassID Class_ID(void) const override;


		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine &) const override;
		virtual int Fetch_Heap_ID(void) const override;

		static HousesType From_Name(char const * name);
		static void One_Time(void);
		static HouseTypeClass * Find_Or_Make(char const * ininame);

		virtual bool Read_INI(CCINIClass const & ini) override;
};
