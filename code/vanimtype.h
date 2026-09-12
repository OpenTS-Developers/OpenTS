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

#include "vanim.hh"

class AnimTypeClass;
class WarheadTypeClass;
class ParticleSystemTypeClass;

class VoxelAnimTypeClass : public ObjectTypeClass
{
		typedef ObjectTypeClass BASECLASS;

	public:
		/*-----------------------------------------------------------------------------------
		**	Constructor & destructors.
		*/
		VoxelAnimTypeClass(char const * ininame = NULL);
		~VoxelAnimTypeClass(void);

		virtual ClassID Class_ID(void) const override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		static VoxelAnimTypeClass * Find_Or_Make(char const * name);
		static VoxelAnimType From_Name(char const * name);
		static char const * Name_From(VoxelAnimType anim);

		virtual void Detach(AbstractClass const * target, bool all = true) override;

		virtual void Compute_CRC(CRCEngine & crc) const override;
		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_VOXELANIMTYPE);}

		virtual bool Read_INI(CCINIClass const & ini) override;

		virtual bool Create_And_Place(Cell const & cell, HouseClass * house) const override {return(false);}
		virtual ObjectClass * Create_One_Of(HouseClass *) const override {return(NULL);}

	public:
		/// Unused
		bool IsNormalized;

		/*
		 * If this animation should be blended into the scene at 50% translucency rather
		 * than drawn solid, then this flag will be true.
		 */
		bool IsTranslucent;

		/*
		 * If this animation borrows its voxel image from the body, turret or barrel of
		 * another object type, then this flag will be true. The borrowed image belongs to
		 * that other type, so it is merely let go of when this type is destroyed.
		 */
		bool IsSharesSourceData;

		/*
		 * This specifies which layer of the voxel file is drawn for this animation. A
		 * voxel file may hold more than one section and only one of them is wanted here.
		 */
		int VoxelIndex;

		/*
		 * This is how long the animation is allowed to live, expressed in game frames.
		 * When the time runs out it performs its impact effects and removes itself.
		 */
		int Duration;

		/*
		 * This is how bouncy the animation is -- the fraction of its speed that it keeps
		 * when it rebounds off the terrain.
		 */
		double Elasticity;

		/*
		 * These bound the tumbling rate the animation is created with, which is picked at
		 * random between them. They are expressed in radians per frame, although the INI
		 * supplies degrees.
		 */
		double MinAngularVelocity;
		double MaxAngularVelocity;

		/*
		 * These bound the vertical speed the animation is thrown into the air with, a
		 * speed being picked at random between them. A meteor always starts at the minimum.
		 */
		double MinZVel;
		double MaxZVel;

		/*
		 * This is the fastest the animation may be thrown horizontally. Both the X and Y
		 * components are picked at random between plus and minus this value.
		 */
		double MaxXYVel;

		/*
		 * If this animation behaves as a falling meteor, then this flag will be true. A
		 * meteor starts back along its flight path so that it appears to fall in from the
		 * distance, and it gouges a crater into the terrain where it lands.
		 */
		bool IsMeteor;

		/*
		 * This is the voxel animation that this one breaks apart into when a meteor
		 * strikes the ground.
		 */
		VoxelAnimTypeClass const * Spawns;

		/*
		 * This is the upper limit on how many child animations the impact spawns. The
		 * count is the sum of two random picks in that range, so middling counts are the
		 * most likely.
		 */
		int SpawnCount;

		/*
		 * These are the sound effects that mark the events in the animation's life.
		 */
		VocType StartSound;		/// When it is created.
		VocType BounceSound;	/// Each time it strikes the ground.
		VocType ExpireSound;	/// When its lifetime runs out.

		/*
		 * This is the animation played wherever the debris strikes the ground, such as a
		 * puff of dust.
		 */
		AnimTypeClass const * BounceAnim;

		/*
		 * This is the animation played when the debris comes to the end of its life on
		 * solid ground -- typically the explosion that goes with the impact damage.
		 */
		AnimTypeClass const * ExpireAnim;

		/*
		 * This is the animation left behind the debris while it is in the air, such as a
		 * trail of smoke.
		 */
		AnimTypeClass const * TrailerAnim;

		/*
		 * This is the damage the debris deals -- to whatever it lands on as it bounces,
		 * and as a blast at the point where it finally expires.
		 */
		int Damage;

		/*
		 * This is how close an object must be to a bounce for the debris to hurt it,
		 * expressed in leptons.
		 */
		int DamageRadius;

		/*
		 * This is the warhead the debris deals its damage with. It decides what the
		 * damage is effective against and what mark is left behind.
		 */
		WarheadTypeClass const * Warhead;

		/*
		 * This is the particle system attached to the debris when it is created, so that
		 * something such as a plume of smoke follows it through the air.
		 */
		ParticleSystemTypeClass const * AttachedSystem;

		/*
		 * If this debris is a chunk of tiberium, then this flag will be true. It is drawn
		 * with the tiberium colors and seeds tiberium into the cells it lands on.
		 */
		bool IsTiberium;
};

char const * VoxelAnim_Name(VoxelAnimType anim);
