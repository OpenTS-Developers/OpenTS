/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "objtype.h"
#include "rgb.h"
#include "typelist.h"

#include "particle.hh"


class WarheadTypeClass;
class ParticleTypeClass;


class ParticleTypeClass : public ObjectTypeClass
{
		typedef ObjectTypeClass BASECLASS;

	public:
		ParticleTypeClass(char const * ininame = NULL);
		virtual ~ParticleTypeClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine & crc) const override;
		virtual void Detach(AbstractClass const * target, bool all = true) override;

		virtual bool Read_INI(CCINIClass const & ini) override;
		virtual bool Create_And_Place(Cell const & , HouseClass * house = NULL) const override;
		virtual ObjectClass * Create_One_Of(HouseClass *) const override;

		static ParticleType From_Name(char const * name);
		static const char * Name_From(ParticleType particle);
		static ParticleTypeClass * Find_Or_Make(char const * name);

	public:
		/*
		 * This is the offset from the dying particle's position at which its NextParticle
		 * successor is created, so the follow-on can be nudged clear of the original.
		 */
		Point3D NextParticleOffset;

		/*
		 * These are the ranges the horizontal components of a spark's initial velocity are
		 * randomly picked from. Neither may be zero, since both are used as a divisor.
		 */
		int XVelocity;
		int YVelocity;

		/*
		 * These specify the vertical component of a spark's initial velocity -- a random
		 * amount up to the range is added to the minimum. The range may not be zero.
		 */
		int MinZVelocity;
		int ZVelocityRange;

		/*
		 * This specifies how quickly the particle blends from one entry of its ColorList to
		 * the next. A small random amount is added each frame so no two fade in step.
		 */
		double ColorSpeed;

		/*
		 * This is the list of colors a spark or railgun trace blends through as it ages.
		 * Such particles have no artwork -- each is drawn as one pixel of the blended color.
		 */
		TypeList<RGBClass> ColorList;

		/*
		 * These are the two ends of the range a particle's starting color is picked from,
		 * randomly interpolated at creation. If both are black, then the first entry of the
		 * ColorList is used instead.
		 */
		RGBClass StartColor1;
		RGBClass StartColor2;

		/*
		 * This is the number of game frames between damage applications, so that a flame
		 * scorches what it passes over at a steady rate rather than every frame.
		 */
		int MaxDC;

		/*
		 * This is the base lifetime of the particle, expressed in game frames. A random
		 * extra amount is added at creation so a cloud does not all expire at once.
		 */
		int MaxEC;

		/*
		 * Pointer to the warhead the particle's damage is delivered through. It decides how
		 * much of the Damage each kind of target actually suffers.
		 */
		WarheadTypeClass * Warhead;

		/*
		 * This is the amount of damage the particle inflicts on whatever shares its cell,
		 * applied through its Warhead. If zero, then the particle is harmless.
		 */
		int Damage;

		/// Unused
		int StartFrame;
		int NumLoopFrames;

		/*
		 * This is the translucency the particle is drawn with when it is created, as a
		 * percentage. Only the 25, 50 and 75 levels are recognized, at the highest detail.
		 */
		int Translucency;

		/*
		 * This specifies how strongly the prevailing wind carries the particle sideways --
		 * a gas cloud drifts further per frame, a smoke puff drifts more often. If zero,
		 * then the wind is ignored.
		 */
		int WindEffect;

		/*
		 * This is the speed a particle of this type is created with, before the spawning
		 * system's own perturbation is applied.
		 */
		float Velocity;

		/*
		 * This is the amount the particle's speed is reduced by each game frame, so that a
		 * flame or a puff slows as it travels. A flame that slows to a stop dies.
		 */
		float Deacc;

		/*
		 * This is the radius that a dying smoke puff scatters its two successor particles
		 * within, so that a plume spreads out as it rises rather than staying a column.
		 */
		int Radius;

		/*
		 * If this flag is true, then the particle dies as soon as its animation reaches the
		 * EndStateAI, rather than looping until its lifetime runs out.
		 */
		bool DeleteOnStateLimit;

		/*
		 * This is the last animation state of the particle's sequence. Reaching it kills the
		 * particle if DeleteOnStateLimit is set, and otherwise wraps the animation around.
		 */
		char EndStateAI;

		/*
		 * This is the animation state a newly created particle of this type starts at.
		 */
		char StartStateAI;

		/*
		 * This is the number of game frames between advances of the particle's animation
		 * state. It is jittered per particle so a cloud does not animate in lockstep.
		 */
		char StateAIAdvance;

		/*
		 * This is the last animation state at which a damaging particle still inflicts its
		 * damage, so a dying flame stops scorching what it drifts over. For a normalized
		 * particle it also decides how many states the flight is divided into.
		 */
		char FinalDamageState;

		/*
		 * These are the animation states at which a flame fades to 25 and 50 percent
		 * translucency, so that it thins out as it burns down. If 255, then it never fades.
		 */
		unsigned char Translucent25State;
		unsigned char Translucent50State;

		/*
		 * If this flag is true, then the particle's animation rate is scaled at creation so
		 * that the sequence runs out just as the particle arrives at its target.
		 */
		bool IsNormalized;

		/*
		 * This is the particle type spawned in this particle's place when it expires. It
		 * lets a cloud change character as it ages. If PARTICLE_NONE, nothing follows it.
		 */
		ParticleType NextParticle;

		/*
		 * This specifies which of the particle logic routines drives this type -- gas,
		 * smoke, fire, spark, railgun or web. It decides how the particle moves and draws.
		 */
		ParticleBehaviorType BehavesLike;
};

ParticleBehaviorType Particle_Behavior_From_Name(char const * name);
