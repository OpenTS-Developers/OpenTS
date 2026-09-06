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

#include "objtype.h"

#include "smudge.hh"
#include "theater.hh"

/****************************************************************************
**	This type elaborates the various "smudge" effects that can occur. Smudges are
**	those elements which are on top off all the ground icons, but below anything
**	that is "above" it. This includes scorch marks, craters, and infantry bodies.
**	Smudges, be definition, contain transparency. The are modifiers to underlying
**	terrain imagery.
*/
class SmudgeTypeClass : public ObjectTypeClass
{
		typedef ObjectTypeClass BASECLASS;

	public:
		/*
		**	What overlay is this.
		*/
		SmudgeType HeapID;

		/*
		**	Some smudges are larger than one cell. If this is the case, then
		**	these dimensions specify the number of cells wide and tall the
		**	smudge is.
		*/
		int Width;
		int Height;

		/*
		**	Is this smudge a crater type? If so, then a second crater can be added to
		**	this smudge so that a more cratered landscape results.
		*/
		bool IsCrater;

		/*
		 * Is this smudge a scorch mark? Only smudges flagged this way are considered when
		 * fire or an explosion needs to leave a burn upon the ground.
		 */
		bool IsScorch;

		//----------------------------------------------------------
		SmudgeTypeClass(char const * ininame = NULL);
		virtual ~SmudgeTypeClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_SMUDGETYPE);}
		virtual void Compute_CRC(CRCEngine &) const override;
		virtual int Fetch_Heap_ID(void) const override {return(HeapID);}

		static SmudgeType From_Name(char const * name);
		static void Init(TheaterType);
		static void One_Time(void) {BASECLASS::One_Time();}
		static void Prep_For_Add(void);

		static SmudgeTypeClass * Find_Or_Make(const char *name);

		virtual bool Read_INI(CCINIClass const & ini) override;

		virtual bool Create_And_Place(Cell const & cell, HouseClass * house) const override;
		virtual ObjectClass * Create_One_Of(HouseClass *) const override;
		virtual void Draw_It(Point2D const & point, Rect const & cliprect, int size, int z, Cell const & cell) const;

		bool Can_Place_Here(Cell const & origin, bool underbuildings) const;
		void Place(Cell const & origin) const;

		static bool Scorch_The_Ground(Coord const & coord, int width = 100, int height = 100, bool large = false);
		static bool Crater_The_Ground(Coord const & coord, int width = 100, int height = 100, bool large = false);
};
