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

/* $Header: /CounterStrike/BULLET.H 2     3/06/97 1:46p Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : BULLET.H                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 23, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 23, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "facing.h"
#include "fuse.h"
#include "object.h"
#include "velocity.h"

#include "draw.hh"


class BulletTypeClass;
class TechnoClass;
class WeaponTypeClass;
struct VoxelDataStruct;
class Matrix3D;


class BulletClass : public ObjectClass
{
		typedef ObjectClass BASECLASS;

	public:

	public:

		/*
		**	This specifies exactly what kind of bullet this is. All of the static attributes
		**	for this bullet is located in the BulletTypeClass pointed to by this variable.
		*/
		BulletTypeClass * Class;

	private:
		/*
		**	Records who sent this "present" so that an appropriate "thank you" can
		**	be returned.
		*/
		TechnoClass * Payback;

	public:

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		BulletClass(void);
		virtual ~BulletClass(void) override;

		virtual ClassID Class_ID(void) const override;

		virtual void Serialize(SaveStreamClass & stream) override;

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
		virtual RTTIType Fetch_RTTI(void) const override;

		void Set_Bullet_Data(BulletTypeClass const *type, AbstractClass *target, TechnoClass *payback, int strength, WarheadTypeClass const *warhead, int max_speed, int range, bool bright);
		bool Is_Forced_To_Explode(Coord & coord) const;
		void Bullet_Explodes(bool forced);
		virtual int Shape_Number(void) const;
		virtual LayerType In_Which_Layer(void) const override;
		virtual void Assign_Target(AbstractClass * target);
		virtual bool Unlimbo(Coord const & coord, TVelocity3D<double> const & velocity);
		virtual ObjectTypeClass const * Class_Of(void) const override;
		virtual void Detach(AbstractClass const * target, bool all = true) override;
		virtual void Draw_It(Point2D const & point, Rect const & cliprect) const override;
		virtual bool Mark(MarkType mark=MARK_CHANGE) override;
		virtual void AI(void) override;
		virtual Cell const * Occupy_List(bool = false) const override;
		bool Is_Homing(void) const;
		void Draw_Voxel(VoxelDataStruct const & voxeldata, Matrix3D const & transform, Point2D const & drawpoint, Rect const & cliprect, int frame, ShapeFlags_Type flags, int brightness) const;
		void Detonate(Coord const & coord);

		/*
		**	If this bullet is forced to be inaccurate because of some outside means. A tank
		**	firing while moving is a good example.
		*/
		bool IsInaccurate;

	private:
		/*
		 * This is the fuse that decides when a homing projectile has arrived. It is armed
		 * with the impact point as the projectile is launched and trips once the projectile
		 * stops closing on it.
		 */
		FuseClass Fuse;

		/*
		 * If the explosion of this projectile should light up the terrain around it, then
		 * this flag will be true. It comes from the weapon that fired the projectile.
		 */
		bool IsBright;

	public:
		/*
		 * This is the distance the projectile covers each game frame. Gravity, homing and
		 * bouncing all work by adjusting it, and the direction it points is the direction
		 * the projectile is drawn facing.
		 */
		TVelocity3D<double> Velocity;

		/*
		 * The number of times this (arcing) projectile has struck a surface. When it
		 * reaches the bounce limit, the projectile is forced to detonate.
		 */
		int BounceCount;

		/// Unused
		bool field_A4;

		/*
		 * Set while a homing projectile is still in its launch phase: acceleration is
		 * ramped gently, no turning toward the target is performed and the closure
		 * stall detector is inactive. Cleared once the projectile reaches cruise
		 * speed (fast projectiles skip the launch phase entirely).
		 */
		bool IsLaunching;

		/*
		**	This is the target of the projectile. It is especially significant for those projectiles
		**	that home in on a target.
		*/
		AbstractClass * TarCom;

		/*
		**	The speed of this projectile.
		*/
		int MaxSpeed;

		/*
		 * Warm-up counter for the closure stall detector; counts the frames that
		 * SmoothedClosure has plainly accumulated before it switches over to the
		 * moving average form.
		 */
		int ClosureSamples;

		/*
		 * Smoothed measure of how quickly a homing projectile is closing on its
		 * target (roughly the per-frame closure in leptons scaled by the warm-up
		 * length). When it decays to nearly nothing, the projectile has stalled
		 * beside its target and is forced to detonate.
		 */
		double SmoothedClosure;

		/*
		**	The warhead of this projectile.
		*/
		WarheadTypeClass * Warhead;

		/*
		 * This is the frame of the projectile's shape that is currently displayed. It cycles
		 * between the low and high animation frames of the projectile type.
		 */
		unsigned char AnimFrame;

		/*
		 * This counts down the game frames until the next animation frame is shown. When it
		 * reaches zero it is reloaded from the animation rate of the projectile type.
		 */
		unsigned char AnimRate;

		/*
		 * This is how much farther the projectile may travel, expressed in leptons. It only
		 * applies to a fueled projectile, which is forced to detonate when it runs out.
		 */
		int Range;

	public:
		friend BulletClass *Create_Bullet(BulletTypeClass const *type, TechnoClass *target, TechnoClass *payback, int strength, WarheadTypeClass const *warhead, int max_speed, int range, bool bright);
};


BulletClass * Create_Bullet(BulletTypeClass const *type, AbstractClass *target, TechnoClass *payback, int strength, WarheadTypeClass const *warhead, int max_speed, int range, bool bright);
