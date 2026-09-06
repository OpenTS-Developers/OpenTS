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
#include "partsys.hh"


class ParticleSystemTypeClass : public ObjectTypeClass
{
		typedef ObjectTypeClass BASECLASS;

	public:
		ParticleSystemTypeClass(char const * ininame = NULL);
		virtual ~ParticleSystemTypeClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine & crc) const override;

		virtual bool Read_INI(CCINIClass const & ini) override;
		virtual bool Create_And_Place(Cell const & , HouseClass * house = NULL) const override;
		virtual ObjectClass * Create_One_Of(HouseClass *) const override;

		static ParticleSystemType From_Name(char const * name);
		static char const * Name_From(ParticleSystemType partsys);
		static ParticleSystemTypeClass * Find_Or_Make(char const * name);

		ParticleSystemBehaviorType Behaves_Like(void) const { return(BehavesLike); }

	public:
		/*
		 * This is the type of particle the system fills itself with. If PARTICLE_NONE, then
		 * the system spawns nothing at all.
		 */
		ParticleType HoldsWhat;

		/// Unused
		bool Spawns;

		/*
		 * This is the number of game frames between particle spawns. A smoke system takes
		 * this as its starting interval and then lets Slowdown stretch it as the plume ages.
		 */
		int SpawnFrames;

		/*
		 * This is the amount added to a smoke system's spawn interval every frame, so that
		 * the plume thins out as it dies down rather than pouring at a constant rate.
		 */
		float Slowdown;

		/*
		 * This is the number of particles the system is built to hold at once. A spark burst
		 * spawns between half of it and all of it, and a one frame light brightens as it fills.
		 */
		int ParticleCap;

		/*
		 * This is the radius, expressed in leptons, that a smoke system scatters its new
		 * particles within, so that the plume does not rise from a single point.
		 */
		int SpawnRadius;

		/*
		 * Once a smoke system's spawn interval has been stretched past this, the system stops
		 * emitting and is retired as soon as the last of its particles has died.
		 */
		float SpawnCutoff;

		/*
		 * Once a smoke system's spawn interval has been stretched past this, new particles
		 * are created a step more translucent, so that the plume fades as it thins.
		 */
		float SpawnTranslucencyCutoff;

		/*
		 * This specifies which of the system logic routines drives this type -- smoke, gas,
		 * fire, spark, railgun or web. It decides how the system emits and aims its particles.
		 */
		ParticleSystemBehaviorType BehavesLike;

		/*
		 * This is how long the system lives, expressed in game frames. If -1, then the system
		 * runs until its own behavior retires it.
		 */
		int Lifetime;

		/*
		 * This is the direction a spark burst is thrown in, added to each spark's own random
		 * velocity before rescaling. It is ignored when the sparks fly in random directions.
		 */
		TPoint3D<float> SpawnDirection;

		/*
		 * This is how densely a railgun trace lays its particles along the beam, expressed as
		 * particles per lepton of the distance from the firer to the target.
		 */
		double ParticlesPerCoord;

		/*
		 * This is how far the railgun trace's spiral turns per lepton of beam length,
		 * expressed in radians. The larger the value, the tighter the corkscrew is wound.
		 */
		double SpiralDeltaPerCoord;

		/*
		 * This is the radius of the corkscrew a railgun trace winds around its beam,
		 * expressed in leptons.
		 */
		double SpiralRadius;

		/*
		 * This is how far each railgun particle is randomly displaced from its place on the
		 * spiral, so that the trace looks ragged rather than machined.
		 */
		double PositionPerturbationCoefficient;

		/*
		 * This is how far each railgun particle's travel direction is randomly deflected
		 * from the spiral it was laid on, so that the trace frays as it fades.
		 */
		double MovementPerturbationCoefficient;

		/*
		 * This is how far a railgun particle's speed may wander from its own type's velocity.
		 * The wander carries over from one particle to the next, so that the trace ripples.
		 */
		double VelocityPerturbationCoefficient;

		/*
		 * This is the chance (0 - 1) that a spark system emits a burst on any given frame.
		 * Its final frame always bursts, so that a shy system is never wasted entirely.
		 */
		double SpawnSparkPercentage;

		/*
		 * This is how many game frames a spark system goes on bursting for. The system is
		 * retired once they have run out.
		 */
		int SparkSpawnFrames;

		/*
		 * This specifies how strong a light the system casts, which scales the brightness
		 * the spotlight is drawn at. If zero, then the system lights nothing around it.
		 */
		int LightSize;

		/*
		 * This is the color of the laser beam drawn along a railgun trace.
		 */
		RGBClass LaserColor;

		/*
		 * If a railgun trace should draw a laser beam from the firer to the target alongside
		 * its spiral of particles, then this flag will be true.
		 */
		bool IsLaser;

		/*
		 * If this flag is true, then the system's light is drawn afresh every frame, waxing
		 * and waning with how full of particles the system is, rather than being cast once.
		 */
		bool OneFrameLight;
};

ParticleSystemBehaviorType Particle_System_Behavior_From_Name(char const * name);
