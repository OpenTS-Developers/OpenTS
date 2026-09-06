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

/* $Header: /CounterStrike/WARHEAD.H 1     3/03/97 10:26a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : WARHEAD.H                                                    *
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
#include "typelist.h"

#include "armor.hh"

class AnimTypeClass;
class ParticleSystemTypeClass;

/**********************************************************************
**	Each of the warhead types has specific characteristics. This structure
**	holds these characteristics.
*/
class WarheadTypeClass : public AbstractTypeClass
{
		typedef AbstractTypeClass BASECLASS;

	public:
		WarheadTypeClass(char const * ininame = NULL);
		virtual ~WarheadTypeClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual void Detach(AbstractClass const * target, bool all = true) override;

		virtual RTTIType Fetch_RTTI(void) const override;

		virtual void Compute_CRC(CRCEngine &) const override;

		bool Read_INI(CCINIClass const & ini);

		static WarheadTypeClass *Find_Or_Make(const char *name);
		static WarheadTypeClass *From_Name(char const * name);

		/*
		 * This is the chance, per point of damage inflicted, that the blast will crater the
		 * ground beneath it. The damage must first exceed the DeformThreshhold before the
		 * roll is made at all.
		 */
		double Deform;

		/*
		**	The warhead damage is reduced depending on the the type of armor the
		**	defender has. This table is what gives weapons their "character".
		*/
		double Modifier[ARMOR_COUNT];

		/*
		 * This is the fraction of the damage that reaches an infantryman who is lying prone,
		 * where one means full damage. A prone infantryman still always takes at least one
		 * point of damage, however small the fraction.
		 */
		double ProneDamage;

		/*
		 * Damage at or below this amount will never crater the ground, however lucky the
		 * Deform roll would have been.
		 */
		int DeformThreshhold;

		/*
		**	Which explosion set to use for warhead impact.
		*/
		TypeList<AnimTypeClass const *> ExplosionSet;

		/*
		**	This specifies the infantry death animation to use if the infantry dies as
		**	a result of a warhead of this type.
		*/
		int InfantryDeath;

		/*
		**	This value control how damage from this warhead type will reduce
		**	over distance. The larger the number, the less the damage is reduced
		**	the further the distance from the source of the damage.
		*/
		int SpreadFactor;

		/*
		 * This is how long, expressed in game frames, an infantryman caught by a "webby"
		 * warhead stays paralyzed and struggling.
		 */
		int WebDuration;

		/*
		 * The web duration is randomly adjusted by up to this many frames in either
		 * direction, so that a group of webbed infantry does not break free all at once.
		 */
		int WebDurationVariation;

		/*
		 * This is the radius, expressed in cells, of the area that a "webby" warhead covers
		 * with web when it detonates.
		 */
		int WebRadius;

		/*
		 * This is the percentage of its speed that a limpet drone robs from the object it
		 * attaches to (0 - 100), slowing its body and turret rotation by the same amount.
		 * A value of zero means this warhead carries no limpet at all.
		 */
		unsigned int LimpetFactor;

		/*
		 * Pointer to the particle system this warhead releases where it detonates. A "webby"
		 * warhead spawns one in every cell it webs. If NULL, then no particles are released.
		 */
		ParticleSystemTypeClass *Particle;

		/*
		**	If this warhead type can destroy walls, then this flag will be true.
		*/
		bool IsWallDestroyer;

		/*
		 * If this warhead entangles infantry in a web rather than damaging them, then this
		 * flag will be true. Everything caught within the WebRadius is paralyzed and left
		 * struggling for the duration, and takes no damage at all.
		 */
		bool IsWebby;

		/*
		**	If this warhead can destroy wooden walls, then this flag will be true.
		*/
		bool IsWoodDestroyer;

		/*
		**	Does this warhead damage tiberium?
		*/
		bool IsTiberiumDestroyer;

		/*
		**	Only effective against infantry?
		*/
		bool IsOrganic;

		/*
		 * If this warhead sets what it hits alight, then this flag will be true. A terrain
		 * object struck by it catches fire, and a building shows pieces of flame as it
		 * drops through a damage level.
		 */
		bool IsSparky;

		/*
		 * If this warhead burns rather than blasts, then this flag will be true. Fire melts
		 * the ice it lands on, but it cannot bring down a destroyable cliff, so the attack
		 * cursor will not offer one as a target.
		 */
		bool IsFire;

		/*
		 * If this warhead throws up a splash when it strikes water, then this flag will be
		 * true. Such a blast picks its animation from the splash list in the rules rather
		 * than from its own explosion set.
		 */
		bool IsConventional;

		/*
		 * If this warhead rocks the vehicles caught in the blast, then this flag will be
		 * true. Everything within three cells is tipped by the explosion, with the force
		 * scaled by the damage -- a weak enough blast rocks nothing at all.
		 */
		bool IsRocker;

		/*
		 * If this warhead lights up the terrain around the blast, then this flag will be
		 * true. The explosion drops a spotlight whose size grows with the damage.
		 */
		bool IsBright;

		/*
		 * If this warhead detonates as an electromagnetic pulse rather than as a blast, then
		 * this flag will be true. Its impact animation is also picked at random rather than
		 * chosen according to the strength of the damage.
		 */
		bool IsEMEffect;

		/*
		 * If this warhead is what the veins and the veinhole monster attack with, then this
		 * flag will be true. Such damage arrives with no source object, so a unit hurt by it
		 * retaliates against the veinhole that owns the veins it is standing in.
		 */
		bool IsVeinhole;

};
