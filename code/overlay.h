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

/* $Header: /CounterStrike/OVERLAY.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : OVERLAY.H                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 17, 1994                                                 *
 *                                                                                             *
 *                  Last Update : May 17, 1994   [JLB]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "isun.h"
#include "object.h"

#include "overlay.hh"
#include "tiberium.hh"


class CCINIClass;
class OverlayTypeClass;

/******************************************************************************
**	This class controls the overlay object. Overlay objects function congruously
**	to carpet on a floor. They have no depth, but merely control the icon to be rendered
**	as the cell's bottom most layer.
*/
class OverlayClass : public ObjectClass
{
		typedef ObjectClass BASECLASS;

	public:
		/*
		**	This is a pointer to the overlay object's class.
		*/
		OverlayTypeClass * Class;

		/*-------------------------------------------------------------------
		**	Constructors and destructors.
		*/
		OverlayClass(OverlayTypeClass const * ttype, Cell const & pos = CELL_NONE, HousesType = HOUSE_NONE);
		virtual ~OverlayClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override {if (retval == NULL) return(E_POINTER);*retval = CLSID_OverlayClass;return(S_OK);}

		virtual void Serialize(SaveStreamClass & stream) override;

		/*
		**	File I/O.
		*/
		static void Read_INI(CCINIClass const & ini);
		static void Write_INI(CCINIClass & ini);
		static void Post_Read_Vein_Fixups(void);

		/*
		**	Virtual support functionality.
		*/
		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_OVERLAY);}
		virtual bool Mark(MarkType) override;
		virtual ObjectTypeClass const * Class_Of(void) const override;
		virtual void Draw_It(Point2D const &, Rect const &) const override {}
		virtual void Editor_Draw_It(Point2D const & point, Rect const & cliprect) const override;

	private:
		/*
		**	This is used to control the marking process of the overlay. If this is
		**	set to a valid house number, then the cell that the overlay is marked down
		**	upon will be flagged as being owned by the specified house.
		*/
		static HousesType ToOwn;

		/*
		 * This is the name of the scenario section that lists the overlay layer cell by
		 * cell. The layer itself travels in the compressed OverlayPack section, so this
		 * one survives only to be cleared away when a scenario is saved.
		 */
		static char const * const INI_NAME;
};

Point2D Overlay_Draw_Offset(OverlayType overlay);
TiberiumType Which_Tiberium_Type(OverlayType overlay);
