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

#include "bullet.hh"
#include "scheme.hh"


class WeaponTypeClass;
class AnimTypeClass;
class HouseClass;
class Cell;


/***************************************************************************
**	Bullets and other projectiles need some specific information according
**	to their type.
*/
class BulletTypeClass : public ObjectTypeClass
{
		typedef ObjectTypeClass BASECLASS;

	public:

		/*
		 * If this projectile bursts in the air above its target rather than diving into it,
		 * then this flag will be true. An airburst holds its altitude to the end and ignores
		 * height when measuring the distance left, so a cluster carrier splits high up.
		 */
		bool IsAirburst;

		/*
		 * If this projectile drifts toward its target rather than plummeting, then this flag
		 * will be true. A floater falls under half the rule gravity, both in flight and when
		 * the firing routines work out the launch pitch and speed needed to reach the target.
		 */
		bool IsFloater;

		/*
		**	Does this bullet type fly over walls?
		*/
		bool IsHigh;

		/*
		 * If this homing projectile cruises at extra altitude, then this flag will be true. It
		 * holds ten terrain levels of clearance instead of five and does not begin its dive
		 * until within six cells of the target, so it clears tall hills on the way in.
		 */
		bool IsVeryHigh;

		/*
		**	Does this bullet need a shadow drawn under it?  Shadowed bullets
		**	use the Height value to offset their Y position.
		*/
		bool IsShadow;

		/*
		**	If this projectile is one that ballistically arcs from ground level, up into the air and
		**	then back to the ground, where it explodes. Typical uses of this are for grenades and
		**	artillery shells.
		*/
		bool IsArcing;

		/*
		**	Certain projectiles do not travel horizontally, but rather, vertically -- they drop
		**	from a height. Bombs fall into this category and will have this value set to
		**	true. Dropping projectiles do not calculate collision with terrain (such as walls).
		*/
		bool IsDropping;

		/*
		**	Is this projectile invisible?  Some bullets and weapon effects are not directly
		**	rendered. Small caliber bullets and flame thrower flames are treated like
		**	normal projectiles for damage purposes, but are displayed using custom
		**	rules.
		*/
		bool IsInvisible;

		/*
		**	Does this bullet explode when near the target?  Some bullets only explode if
		**	it actually hits the target. Some explode even if nearby.
		*/
		bool IsProximityArmed;

		/*
		**	Should fuel consumption be tracked for this projectile?  Rockets are the primary
		**	projectile with this characteristic, but even for bullets it should be checked so that
		**	bullets don't travel too far.
		*/
		bool IsFueled;

		/*
		**	Is this projectile without different facing visuals?  Most plain bullets do not change
		**	visual imagery if their facing changes. Rockets, on the other hand, are equipped with
		**	the full 32 facing imagery.
		*/
		bool IsFaceless;

		/*
		**	If this is a typically inaccurate projectile, then this flag will be true. Artillery
		**	is a prime example of this type.
		*/
		bool IsInaccurate;

		/*
		**	If this bullet can be fired on aircraft, then this flag will be true.
		*/
		bool IsAntiAircraft;

		/*
		**	If this bullet can fire upon ground targets, then this flag will be true.
		*/
		bool IsAntiGround;

		/*
		**	If this bullet should lose strength as it travels toward the target, then
		**	this flag will be true.
		*/
		bool IsDegenerate;

		/*
		 * If this ballistic projectile rebounds off the ground rather than detonating there,
		 * then this flag will be true. It explodes on striking a unit or after three bounces.
		 */
		bool IsBouncy;

		/*
		 * If this projectile's imagery is drawn with the animation palette rather than the
		 * normal one, then this flag will be true. Only the body is remapped, not the shadow.
		 */
		bool IsAnimPalette;

		/*
		 * If this projectile breaks up into smaller projectiles when it detonates, then this
		 * flag will be true. Rather than repeat its own warhead it fires Cluster bomblets of
		 * the AirburstWeapon straight down onto targets near the point of impact.
		 */
		bool IsSplits;

		/*
		 * If this bullet may only be fired upon vehicles, then this flag will be true. It
		 * overrides the IsAntiAircraft and IsAntiGround flags when threats are evaluated.
		 */
		bool IsAntiVehicle;

		/*
		 * This is the number of separate detonations this projectile produces. An ordinary
		 * projectile applies its warhead this many times, scattering each blast a cell or two
		 * from the last. A splitting projectile instead releases this many bomblets.
		 */
		int Cluster;

		/*
		 * This is the weapon supplying the bomblets that a splitting projectile releases. Each
		 * bomblet carries that weapon's projectile, damage, warhead and range, not this one's.
		 */
		WeaponTypeClass *AirburstWeapon;

		/*
		 * This is the fraction of its speed that a bouncy projectile keeps when it rebounds
		 * off the slope it landed on. The rebound is figured in the slope's own frame of
		 * reference, so a projectile that lands on a ramp is thrown off downhill.
		 */
		double Elasticity;

		/*
		 * This is the rate at which a homing projectile gains speed toward its maximum,
		 * expressed in leptons per game frame. While it is still launching it gains one on
		 * alternate frames instead, so that it eases away from the launcher.
		 */
		int Acceleration;

		/*
		 * This is the color scheme index that a voxel projectile is remapped to when it is
		 * blitted to the screen. Projectiles drawn as shapes are not affected by this.
		 */
		int Color;

		/*
		 * This is the animation that trails behind this projectile in flight -- a missile's
		 * smoke trail. It is spawned every third game frame. If NULL, there is no trail.
		 */
		AnimTypeClass *Trailer;

		/*
		**	This is the rotation speed of the bullet. It only has practical value
		**	for those projectiles that performing homing action during flight -- such
		**	as with rockets. If the ROT is zero, then no homing is performed. Otherwise
		**	the projectile is considered to be a homing type.
		*/
		int ROT;

		/*
		 * This is the chance (0.0 - 1.0) that a bomblet released by a splitting projectile
		 * keeps the carrier's own target. Otherwise it picks at random from the objects
		 * standing near the point of impact, which spreads the cluster over the group.
		 */
		float RetargetAccuracy;

		/*
		**	Some projectiles have a built in arming distance that must elapse before the
		**	projectile may explode. If this value is non-zero, then this override is
		**	applied.
		*/
		int Arming;

		/*
		 * These control the looping animation this projectile's shape plays in flight -- the
		 * first and last frame of the loop, and how many game frames each is held for. If both
		 * frame numbers are zero, the shape is taken from the projectile's facing instead.
		 */
		unsigned char AnimLow;
		unsigned char AnimHigh;
		unsigned char AnimRate;

		/*
		**	If this bullet is of the tumbling type, then this is the modulo to factor
		**	into the game frame when determining what shape number to use for the
		**	imagery.
		*/
		int Tumble;

		//---------------------------------------------------------------------
		BulletTypeClass(char const * name = NULL);
		virtual ~BulletTypeClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		virtual void Detach(AbstractClass const * target, bool all = true) override;

		virtual RTTIType Fetch_RTTI(void) const override;

		virtual void Compute_CRC(CRCEngine & crc) const override;

		static BulletTypeClass *Find_Or_Make(const char * name);

		static BulletType From_Name(char const * name);
		static const char * Name_From(BulletType bullet);

		static void Init(TheaterType ) {};
		static void One_Time(void) {BASECLASS::One_Time();}

		virtual bool Read_INI(CCINIClass const & ini) override;

		virtual Coord const Coord_Fixup(Coord const & coord) const override;
		virtual bool Create_And_Place(Cell const & cell, HouseClass * house) const override {return(false);};
		virtual ObjectClass * Create_One_Of(HouseClass * house) const override {return(NULL);};
};
