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

/* $Header: /CounterStrike/SMUDGE.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SMUDGE.H                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : August 9, 1994                                               *
 *                                                                                             *
 *                  Last Update : August 9, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "globals.h"
#include "object.h"
#include "smudtype.h"


/******************************************************************************
**	This is the transitory form for smudges. They exist as independent objects
**	only in the transition stage from creation to placement upon the map. Once
**	they are placed on the map, they merely become 'smudges' in the cell data. This
**	object is then destroyed.
*/
class SmudgeClass : public ObjectClass
{
		typedef ObjectClass BASECLASS;

	public:
		/*
		**	This is a pointer to the template object's class.
		*/
		SmudgeTypeClass * Class;

		/*-------------------------------------------------------------------
		**	Constructors and destructors.
		*/
		SmudgeClass(SmudgeTypeClass const * type, Coord const & pos = COORD_NONE, HousesType = HOUSE_NONE);
		virtual ~SmudgeClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override;

		/*
		**	File I/O.
		*/
		static void Read_INI(CCINIClass const & ini);
		static void Write_INI(CCINIClass & ini);

		virtual ObjectTypeClass const * Class_Of(void) const override;
		virtual bool Mark(MarkType) override;
		virtual void Draw_It(Point2D const & point, Rect const & cliprect) const override {};

		void Disown(Cell const & cell);

	private:

		static HousesType ToOwn;

		/*
		 * This is the name of the scenario section that the map's smudges are listed in.
		 * Each entry names a smudge type and the cell that it was laid down upon.
		 */
		static char const * const INI_NAME;
};
