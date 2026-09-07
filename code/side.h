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
#include "typelist.h"

#include "side.hh"

class BuildingTypeClass;
class UnitTypeClass;


class SideClass : public AbstractTypeClass
{
		typedef AbstractTypeClass BASECLASS;

	public:
		SideClass(char const * ininame = NULL);
		virtual ~SideClass() override;

		virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID * retval) override;

		/*
		**	Query functions.
		*/
		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_SIDE);}
		virtual void Compute_CRC(CRCEngine &) const override;

		static SideType From_Name(char const * name);

		virtual bool Read_INI(CCINIClass const & ini) override;

	public:
		/*
		 * These are the house types that belong to this side, listed by heap index. The
		 * list is kept in step with each house type's own Side field.
		 */
		TypeList<int> Houses;

		/*
		 * The base building the computer does when it plays for this side, read from the
		 * section carrying the side's own name. The first two sides inherit the rules' GDI and
		 * Nod keys as each file sets them; a side naming no plant falls back to the role lists.
		 */
		BuildingTypeClass const * RegularPowerPlant;
		BuildingTypeClass const * AdvancedPowerPlant;
		BuildingTypeClass const * PowerTurbine;
		UnitTypeClass const * HunterSeeker;
		TypeList<BuildingTypeClass const *> AIWallTowers;
		double AIBaseDefenseCoefficient;
		double AIWallDefense;
		double AIWallDefenseCoefficient;
		bool IsAIBuildsWalls;
		int AIBaseDefensePlaceholders;
		bool IsAIBaseDefensesWithWalls;
};
