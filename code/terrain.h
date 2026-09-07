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

/* $Header: /CounterStrike/TERRAIN.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TERRAIN.H                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 29, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 29, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "object.h"
#include "stage.h"
#include "terrtype.h"


/****************************************************************************
**	Each type of terrain has certain pieces of static information associated
**	with it. This class elaborates this data.
*/
class TerrainClass : public ObjectClass, public StageClass
{
		typedef ObjectClass BASECLASS;

	public:
		/*
		**	This points to the constant terrain data (for this type) that gives this
		**	terrain object its character.
		*/
		TerrainTypeClass * Class;

		/*
		**	Constructor for terrain object class.
		*/
		TerrainClass(void);
		TerrainClass(TerrainTypeClass const * type, Cell const & cell);
		virtual ~TerrainClass(void) override;

		virtual ClassID Class_ID(void) const override;
		virtual HRESULT Load(SaveStreamClass & stream) override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		/*
		**	Terrain specific support functions.
		*/
		void Start_To_Crumble(void);
		bool Is_Animating(void) const;

		/*
		**	Query functions.
		*/
		virtual ObjectTypeClass const * Class_Of(void) const override;
		virtual void Detach(AbstractClass const * target, bool all = true) override;
		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine &) const override;

		/*
		**	Object entry and exit from the game system.
		*/
		virtual bool Unlimbo(Coord const & coord, Dir256 dir=DIR_N) override;
		virtual bool Limbo(void) override;
		virtual MoveType Can_Enter_Cell(CellClass const * cell, FacingType dir = FACING_NONE, int cell_height = -1, CellClass const * = 0, bool = true) const override;

		/*
		**	Display and rendering support functionality. Supports imagery and how
		**	object interacts with the map and thus indirectly controls rendering.
		*/
		virtual bool Render(Rect &, bool forced, bool extras_only) const override;
		virtual void Draw_It(Point2D const & point, Rect const & cliprect) const override;
		virtual void Editor_Draw_It(Point2D const & point, Rect const & cliprect) const override;
		virtual bool Mark(MarkType mark=MARK_CHANGE) override;
		virtual Rect Get_Render_Rect(void) override;

		/*
		**	Combat related.
		*/
		virtual void Fire_Out(void) override;
		virtual bool Catch_Fire(void) override;
		virtual ResultType Take_Damage(int & damage, int distance, WarheadTypeClass const * warhead, TechnoClass * source=0, bool forced=false, bool=false) override;
		virtual void Set_Occupy_Bit(Coord const & coord) override;
		virtual void Clear_Occupy_Bit(Coord const & coord) override;

		int Occupation_Bits(void) const;

		/*
		**	AI.
		*/
		virtual void AI(void) override;

		/*
		**	Scenario and debug support.
		*/
#ifdef _DEBUG
		virtual void Debug_Dump(MonoClass *mono) const override;
#endif

		/*
		**	File I/O.
		*/
		static void Read_INI(CCINIClass const & ini);
		static void Write_INI(CCINIClass & ini);

	private:

		/*
		**	If this terrain object is on fire, then this flag will be true.
		*/
		bool IsOnFire;

		/*
		**	Is this a terrain object that undergoes crumbling animation and it is
		**	in fact crumbling at this time?
		*/
		bool IsCrumbling;

		/// Unused
		int Unused1;
		int Unused2;

		/*
		 * This is the position of this terrain object in absolute tactical pixel space,
		 * computed once when the object is unlimboed and biased by the terrain type's
		 * YDrawFudge. Terrain never moves, so the render rectangle is built by subtracting
		 * the current scroll position from it rather than converting the coordinate again.
		 */
		Point2D RenderPixelPos;

		/*
		 * This is the name of the scenario INI section that lists the terrain objects to
		 * be placed upon the map when the scenario begins.
		 */
		static char const * const INI_NAME;
};
