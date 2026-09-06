/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "globals.h"
#include "object.h"
#include "vector.h"

#include "layer.hh"

template<class T> class DynamicVectorClass;
class ParticleSystemTypeClass;
class ObjectTypeClass;
class AbstractClass;
class ParticleClass;
class ParticleTypeClass;

class ParticleSystemClass : public ObjectClass
{
		typedef ObjectClass BASECLASS;

	public:
		ParticleSystemClass(ParticleSystemTypeClass const * system, Coord const & coord1, AbstractClass * = NULL, AbstractClass * = NULL, Coord const & coord2 = COORD_NONE);
		ParticleSystemClass(void);
		virtual ~ParticleSystemClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual void Delete_Me(void) override { IsMarkedForDeletion = true; }

		virtual void Compute_CRC(CRCEngine & crc) const override;

		virtual RTTIType Fetch_RTTI(void) const override;

		virtual void Detach(AbstractClass const * target, bool all = true) override;;

		virtual bool Is_Inactive(void) const override;

		void Sparks_To_Use_Random_Direction(void) { IsRandomSparkDirection = true; }

		AbstractClass *Source_Object(void) const { return(Source); }
		AbstractClass *Target_Object(void) const { return(Target); }

		virtual void Draw_It(Point2D const & point, Rect const & cliprect) const override;
		virtual ObjectTypeClass const * Class_Of(void) const override;
		virtual LayerType In_Which_Layer(void) const override;
		virtual void AI(void) override;

		ParticleClass * Spawn_Held_Particle(Coord const & coord1, Coord const & coord2);
		ParticleClass * Spawn_Held_Particle_Random(Coord const & coord1, Coord const & coord2, int v);
		ParticleClass * Spawn_Particle(ParticleTypeClass *type, Coord const & coord);
		void Clear_System(void);
		bool Delete_Particle(int index);

	private:
		void Gas_AI(void);
		void Railgun_AI(void);
		void Smoke_AI(void);
		void Spark_AI(void);
		void Fire_AI(void);
		void Web_AI(void);

	public:
		/*
		 * This points to the particle system type that this system is an instance of. It
		 * supplies the particle to hold, the behavior to run, and every limit the system
		 * works within.
		 */
		ParticleSystemTypeClass * Class;
	private:
		/*
		 * This is the vector from the source object's center to where this system was
		 * created, expressed in leptons. It is applied every frame so that a system
		 * attached to a moving object trails along with it.
		 */
		Coord CoordOffset;

		/*
		 * These are the particles the system is currently holding. The system tends them
		 * every frame and is not considered spent until the last of them has died off.
		 */
		DynamicVectorClass<ParticleClass *> SystemParticles;

		/*
		 * This is the coordinate the system aims its particles at, taken from the target
		 * object if one was given. It is the far end of a railgun beam and the point that
		 * gas and smoke are thrown toward.
		 */
		Coord SpawnCoord;

		/*
		 * This points to the object the system emanates from, if any. The system rides along
		 * with it and marks itself for deletion should the source ever go away.
		 */
		AbstractClass * Source;

		/*
		 * This points to whatever the system is aimed at. For a system attached to a techno
		 * source this is the source's own TarCom, so the effect follows the shooter's aim.
		 */
		AbstractClass * Target;
	public:
		/*
		 * This is the interval, in game frames, between rounds of particle spawning. It
		 * starts at the type's value and grows by the Slowdown amount each round, so the
		 * system thins out and finally quits once it passes the type's spawn cutoff.
		 */
		float SpawnFrames;

		/*
		 * This is the number of game frames this system has left to live. It counts down
		 * every frame, and the system marks itself for deletion on reaching zero however
		 * much spawning it had left to do.
		 */
		int Lifetime;

		/*
		 * This is the number of frames a spark system has left to throw sparks for. It counts
		 * down every frame and the system is marked for deletion when it runs out.
		 */
		int SparkSpawnFrames;

		/*
		 * This is the radius of the light a spark system casts. It wanders up and down by
		 * three each frame, between 17 and 41, which is what gives the light its flicker.
		 */
		int SparkRadius;

		/*
		 * If this system has stopped spawning and is waiting to die, then this flag will be
		 * true. It lingers until the last of its particles is gone so that they are not cut
		 * off in mid air.
		 */
		bool IsMarkedForDeletion;

		/*
		 * If sparks should fly out along a randomly chosen bias rather than the direction
		 * the type specifies, then this flag will be true. It is what makes two otherwise
		 * identical spark bursts look different.
		 */
		bool IsRandomSparkDirection;

};
