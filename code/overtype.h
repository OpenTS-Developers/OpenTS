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

#include "land.hh"
#include "overlay.hh"

class AnimTypeClass;


/****************************************************************************
**	This controls the overlay object types. These object types include walls
**	and concrete. They are always considered to be one icon in size and
**	are processed on an icon by icon basis. This is different from normal
**	templates which can be an arbitrary size. Other than this they are
**	mostly similar to normal templates but with some characteristics of
**	structures (they can be destroyed).
*/
class OverlayTypeClass: public ObjectTypeClass
{
		typedef ObjectTypeClass BASECLASS;

	public:
		/*
		**	What overlay is this.
		*/
		OverlayType HeapID;

		/*
		**	What type of ground does this make the cell it occupies?
		*/
		LandType Land;

		/*
		 * This points to the animation that plays in the cell this overlay occupies, for those
		 * overlays that are animated rather than a still shape. It is spawned as the overlay
		 * appears, and its artwork stands in for the overlay's own.
		 */
		AnimTypeClass *CellAnim;

		/*
		**	If this overlay is a wall, how many stages of destruction are there
		**	for this wall type? i.e. sandbags = 2, concrete = 4, etc.
		*/
		int DamageLevels;

		/*
		**	If this overlay is a wall, what amount of damage is necessary
		**	before the wall takes damage?
		*/
		int DamagePoints;

		/*
		**	Is this a wall type overlay?  Wall types change their shape
		**	depending on the existence of adjacent walls of the same type.
		*/
		bool IsWall;

		/*
		**	If this overlay is actually a wall and this wall type is tall enough that
		**	normal ground based straight line weapons will be blocked by it, then this
		**	flag will be true. Brick fences are typical of this type.
		*/
		bool IsHigh;

		/*
		**	If this overlay represents harvestable tiberium, then this flag
		**	will be true.
		*/
		bool IsTiberium;

		/*
		**	Is this a crate? If it is, then goodies may come out of it.
		*/
		bool IsCrate;

		/*
		 * If collecting this overlay should spring a crate pickup event, then this flag will
		 * be true. It is how a mission script notices that the player has taken the crate.
		 */
		bool IsCrateTrigger;

		/*
		 * If this overlay imposes its own land type on the cell rather than deferring to the
		 * terrain tile beneath it, then this flag will be true. Walls and railroads take
		 * precedence whatever this says.
		 */
		bool IsNoUseTileLandType;

		/*
		 * If this overlay is the body of a veinhole monster, then this flag will be true. The
		 * monster itself is a separate object, and this is what lets an explosion in the cell
		 * find it and pass the damage along.
		 */
		bool IsVeinholeMonster;

		/*
		 * If this overlay is a patch of veins, then this flag will be true. Veins spread from
		 * cell to cell of their own accord and their artwork is smoothed against whatever
		 * veins adjoin them.
		 */
		bool IsVeins;

		/*
		 * If this overlay's artwork should be fetched only when it is first needed, then this
		 * flag will be true. Theater initialization skips such overlays, which keeps rarely
		 * used art out of memory until something asks for it.
		 */
		bool DemandLoad;

		/*
		 * If this overlay detonates when it takes damage, then this flag will be true. The
		 * overlay is removed, an explosion is dealt to everything nearby, and any explosive
		 * overlay in an adjacent cell is set alight in turn.
		 */
		bool IsExplosive;

		/*
		 * If damage to this overlay can set off its neighbors, then this flag will be true. A
		 * shot into a tiberium field ripples out through the adjoining cells this way.
		 */
		bool IsChainReaction;

		/*
		 * If this overlay refuses to be displaced by another one, then this flag will be true.
		 * An overlay being placed into a cell it already occupies gives way instead, except
		 * while the scenario is still being laid out.
		 */
		bool IsOverrides;

		/*
		 * If this overlay lies flat on the ground, then this flag will be true. A flat overlay
		 * takes the ground's depth gradient, while an upright one is lifted half a cell and
		 * given a vertical gradient so that objects sort correctly against it.
		 */
		bool IsDrawFlat;

		/*
		 * If this overlay is a rock formation, then this flag will be true. Rocks skip the
		 * vertical lift given to other upright overlays, and a unit on a ramp beside one takes
		 * a depth adjustment of its own so that it does not sink into the rock.
		 */
		bool IsARock;

		//----------------------------------------------------------
		OverlayTypeClass(char const * ininame = NULL);
		~OverlayTypeClass(void);

		virtual ClassID Class_ID(void) const override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		static OverlayTypeClass * Find_Or_Make(char const * name);
		static OverlayType From_Name(char const * name);
		static void Init(TheaterType);
		static void Prep_For_Add(void);

		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_OVERLAYTYPE);}
		virtual void Compute_CRC(CRCEngine & crc) const override;
		virtual Coord const Coord_Fixup(Coord const & coord) const override;
		virtual bool Create_And_Place(Cell const & cell, HouseClass * house) const override;
		virtual ObjectClass * Create_One_Of(HouseClass *) const override;
		virtual Cell const * Occupy_List(bool placement=false) const override;
		virtual void Draw_It(Point2D const & point, Rect const & cliprect, int data) const;

		virtual bool Read_INI(CCINIClass const & ini) override;

		virtual void const * Get_Image_Data(void) const override;

		RGBClass Get_Radar_Color(int shape) const;
};
