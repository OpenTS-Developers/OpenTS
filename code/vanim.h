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

#include "bounce.h"
#include "object.h"

class VoxelAnimTypeClass;
class ParticleSystemClass;
class HouseClass;
class Coord;

class VoxelAnimClass : public ObjectClass, public BounceClass
{
		typedef ObjectClass BASECLASS;

	public:
		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		VoxelAnimClass(void);
		VoxelAnimClass(VoxelAnimTypeClass const * type, Coord const & coord, HouseClass * house = NULL);
		virtual ~VoxelAnimClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine & crc) const override;

		virtual bool Render(Rect &cliprect, bool forced, bool extras_only) const override;
		virtual void Draw_It(Point2D const &point, Rect const &cliprect) const override;
		virtual void AI(void) override;
		virtual LayerType In_Which_Layer(void) const override;
		virtual ObjectTypeClass const * Class_Of(void) const override;
		virtual Cell const * Occupy_List(bool placement=false) const override;

		static void Init_Clear(void);

		void Make_Invisible(void) {IsInvisible = true;}
		void Make_Visible(void) {IsInvisible = false;}

	public:
		/// Unused
		int Unused1;

		/*
		 * This points to the type class that describes this voxel animation -- the voxel to
		 * draw, how it tumbles, and what it does when its life runs out.
		 */
		VoxelAnimTypeClass * Class;

		/*
		 * This is the particle system that rides along with this animation, created at birth
		 * if the type asks for one and destroyed with the animation. It is what gives a
		 * tumbling wreck its trail of smoke.
		 */
		ParticleSystemClass * AttachedParticleSys;

		/*
		 * This is the house whose color scheme the voxel is drawn in, normally the owner of
		 * whatever threw this animation into the air. If NULL, then the animation is drawn
		 * in its own colors.
		 */
		HouseClass * House;

		/*
		 * If this animation is to be removed at the start of its next update, then this flag
		 * will be true. It bypasses the expiration entirely -- the animation simply disappears
		 * rather than exploding, splashing, or spawning anything.
		 */
		bool IsToDie;

		/*
		 * If this animation is to be run but not drawn, then this flag will be true. The
		 * voxel is skipped while the animation goes on bouncing and aging as usual.
		 */
		bool IsInvisible;

		/*
		 * This is the number of game frames left before the animation expires, started
		 * from the type's Duration and counted down by the AI. The bounce logic zeroes it
		 * when the voxel settles or splashes into water, so a wreck expires where it lands.
		 */
		int ECCounter;
};
