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

/* $Header: /CounterStrike/WEAPON.H 1     3/03/97 10:26a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : WEAPON.H                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 05/17/96                                                     *
 *                                                                                             *
 *                  Last Update : May 17, 1996 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "abstype.h"
#include "rgb.h"
#include "typelist.h"

#include "armor.hh"
#include "mph.hh"
#include "threat.hh"
#include "weapon.hh"

class ParticleSystemTypeClass;
class AnimTypeClass;
class WarheadTypeClass;
class BulletTypeClass;


/**********************************************************************
**	This is the constant data associated with a weapon. Some objects
**	can have multiple weapons and this class is used to isolate and
**	specify this data in a convenient and selfcontained way.
*/
class WeaponTypeClass : public AbstractTypeClass
{
		typedef AbstractTypeClass BASECLASS;

	public:
		WeaponTypeClass(char const * ininame = NULL);
		~WeaponTypeClass(void);

		virtual ClassID Class_ID(void) const override;

		static WeaponType From_Name(char const * name);

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_WEAPONTYPE);}

		virtual void Compute_CRC(CRCEngine &) const override;

		static WeaponTypeClass *Find_Or_Make(const char * name);

		char const * Name(void) const {return(IniName);}
		bool Read_INI(CCINIClass const & ini);
		ThreatType Allowed_Threats(void) const;
		bool Is_Wall_Destroyer(void) const;

		void Init_Max_Speed(void);

		/*
		 * This is the damage inflicted upon everything the weapon's beam or wave passes
		 * over, as opposed to the Attack damage its projectile delivers at the target
		 * itself. Only the sonic and railgun weapons wash their path this way.
		 */
		int AmbientDamage;

		/*
		**	This is the number of shots this weapon first (in rapid succession).
		**	The normal value is 1, but for the case of two shooter weapons such as
		**	the double barreled gun turrets of the Mammoth tank, this value will be
		**	set to 2.
		*/
		int Burst;

		/*
		**	This is the unit class of the projectile fired. A subset of the unit types
		**	represent projectiles. It is one of these classes that is specified here.
		**	If this object does not fire anything, then this value will be BULLET_NONE.
		*/
		BulletTypeClass const * Bullet;

		/*
		**	This is the damage (explosive load) to be assigned to the projectile that
		**	this object fires. For the rare healing weapon, this value is negative.
		*/
		int Attack;

		/*
		**	Speed of the projectile launched.
		*/
		MPHType MaxSpeed;

		/*
		**	Warhead to attach to the projectile.
		*/
		WarheadTypeClass const * WarheadPtr;

		/*
		**	Objects that fire (which can be buildings as well) will fire at a
		**	frequency controlled by this value. This value serves as a count
		**	down timer between shots. The smaller the value, the faster the
		**	rate of fire.
		*/
		int ROF;

		/*
		**	When this object fires, the range at which it's projectiles travel is
		**	controlled by this value. The value represents the number of cells the
		**	projectile will travel. Objects outside of this range will not be fired
		**	upon (in normal circumstances).
		*/
		LEPTON Range;

		/*
		 * This is the distance the projectile is fueled for, expressed in leptons. A fueled
		 * projectile burns this down as it flies and detonates when it runs out, so that a
		 * missile cannot chase an evading target forever.
		 */
		LEPTON ProjectileRange;

		/*
		 * These are the delays, in game frames, between the successive shots of a burst. The
		 * entry used is chosen by which shot of the burst has just been fired, and an entry
		 * of -1 leaves that gap to a short random delay instead.
		 */
		int BurstDelay[4];

		/*
		 * This is the closest a target may be before this weapon refuses to fire upon it,
		 * expressed in leptons. If zero, then the weapon has no minimum range.
		 */
		LEPTON MinimumRange;

		/*
		**	This is the typical sound generated when firing.
		*/
		TypeList<int> Sound;

		/*
		**	This is the animation to display at the firing coordinate.
		*/
		TypeList<AnimTypeClass const *> Anim;

		/*
		 * This points to the particle system type this weapon spawns when it fires. The
		 * flame, spark and railgun weapons carry their effect -- and their damage -- in that
		 * system rather than in a projectile.
		 */
		ParticleSystemTypeClass *AttachedParticleSystem;

		/*
		 * This is the color of the bright core of the laser beam this weapon draws.
		 */
		RGBClass LaserInnerColor;

		/*
		 * This is the color of the glow drawn around that core. If it is black, then no
		 * outer glow is drawn at all.
		 */
		RGBClass LaserOuterColor;

		/*
		 * This is how far each color channel of the outer glow may wander from
		 * LaserOuterColor. A fresh offset is picked every frame, so the glow shimmers
		 * instead of sitting at one flat color.
		 */
		RGBClass LaserOuterSpread;

		/*
		 * If this weapon attacks with a stream of fire particles rather than a projectile,
		 * then this flag will be true. The firer must stand still to use it, and cannot fire
		 * again until the particle system it spawned has burned itself out.
		 */
		bool UseFireParticles;

		/*
		 * If this weapon attacks with a spray of sparks rather than a projectile, then this
		 * flag will be true. Like the fire weapon, it can only be used standing still and
		 * only once the previous spray has finished.
		 */
		bool UseSparkParticles;

		/*
		 * If this weapon fires a railgun beam, then this flag will be true. The beam damages
		 * everything standing along its line at the instant it is fired, and is drawn as a
		 * particle system that must expire before the weapon can be fired again.
		 */
		bool IsRailgun;

		/*
		 * If this weapon always lobs its shot in a high arc, then this flag will be true.
		 * Other weapons resort to an arc only when the target sits high enough overhead that
		 * a flat shot would bury itself in the ground between.
		 */
		bool IsLobber;

		/*
		 * If the explosion of this weapon's projectile should light up the terrain around
		 * it, then this flag will be true.
		 */
		bool IsBright;

		/*
		 * This is the number of game frames that the laser beam remains drawn for. The beam
		 * fades toward the end of that time rather than simply vanishing.
		 */
		char LaserDuration;

		/*
		 * If the glow that accompanies the laser beam should be the wider of the two sizes,
		 * then this flag will be true.
		 */
		bool IsBigLaser;

		/*
		 * If this weapon attacks with a sonic wave rather than a projectile, then this flag
		 * will be true. The wave rolls out to the target damaging everything it crosses, and
		 * the firer cannot fire again until it has arrived.
		 */
		bool IsSonic;

		/*
		**	Increase the weapon speed if the target is flying.
		*/
		bool IsTurboBoosted;

		/*
		**	If potential targets of this weapon should be scanned for
		**	nearby friendly structures and if found, firing upon the target
		**	would be discouraged, then this flag will be true.
		*/
		bool IsSupressed;

		/*
		**	If this weapon is equipped with a camera that reveals the
		**	area around the firer, then this flag will be true.
		*/
		bool IsCamera;

		/*
		**	If this weapon requires charging before it can fire, then this
		**	flag is true. In actuality, this only applies to the Tesla coil
		**	which has specific charging animation. The normal rate of fire
		**	value suffices for all other cases.
		*/
		bool IsElectric;

		/*
		 * If this weapon zaps its target with a laser beam, then this flag will be true. The
		 * colors and lifetime of that beam are given by the Laser fields above.
		 */
		bool IsLaser;

		/*
		 * If this weapon cannot be fired during an ion storm, then this flag will be true.
		 */
		bool IsIonSensitive;
};

ArmorType Armor_From_Name(char const * name);
