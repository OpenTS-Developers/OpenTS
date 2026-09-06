/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#define INCLUDE_COM
#include "always.h"

#include "partsys.h"

#include "_map.h"
#include "_rtti.h"
#include "ccrand.h"
#include "cell.h"
#include "foot.h"
#include "globals.h"
#include "goptions.h"
#include "inline.h"
#include "laser.h"
#include "matrix3d.h"
#include "ovrlight.h"
#include "particle.h"
#include "psystype.h"
#include "ptype.h"
#include "savestream.h"
#include "scenario.h"
#include "sun.h"
#include "techno.h"
#include "tracker.h"
#include "vector.h"
#include "wave.h"

#include <algorithm>

extern short SineTable[];


/// <summary>
/// Creates a particle system of the specified type.
/// This routine adds the new system to the global particle system list and places
/// it on the map. The system aims its particles at the target object, or at the
/// spawn coordinate when no target was given. A system given a source object rides
/// along with it and inherits whatever that source is currently shooting at.
/// </summary>
/// <param name="target">The object the system should aim at, if any.</param>
/// <param name="source">The object the system is attached to, if any.</param>
/// <param name="spawn_coord">The coordinate to aim at when no target object was given.</param>
ParticleSystemClass::ParticleSystemClass(ParticleSystemTypeClass const * type, Coord const & coord, AbstractClass * target, AbstractClass * source, Coord const & spawn_coord) :
	BASECLASS(),
	Class((ParticleSystemTypeClass *)type),
	CoordOffset(0, 0, 0),
	SystemParticles(),
	SpawnCoord(COORD_NONE),
	Source(NULL),
	Target(NULL),
	SpawnFrames(Class->SpawnFrames),
	Lifetime(Class->Lifetime),
	SparkSpawnFrames(Class->SparkSpawnFrames),
	SparkRadius(29),
	IsMarkedForDeletion(false),
	IsRandomSparkDirection(false)
{
	Create_ID();
	ParticleSystems.Add(this);

	SystemParticles.Clear();

	if (target != NULL) {
		SpawnCoord = target->Center_Coord();
		if (Map[target->Center_Coord()].IsUnderBridge && target->RTTI == RTTI_CELL) {
			SpawnCoord.Z += BRIDGE_LEPTON_HEIGHT;
		}
	} else {
		SpawnCoord = spawn_coord;
	}

	Coord spawn;
	spawn.X = coord.X;
	spawn.Y = coord.Y;
	spawn.Z = coord.Z;

	if (source != NULL) {
		Source = source;
		TechnoClass * techno = Dynamic_Cast<TechnoClass *>(source);
		if (techno != NULL) {
			Target = techno->TarCom;
		}
	} else {
		Source = NULL;
		Target = target;
	}

	Unlimbo(spawn);

	if (Source != NULL) {
		CoordOffset = Coord(Position) - Source->Center_Coord();
	}

	ObjectPtrTracker.Add(this);
}


/// <summary>
/// Creates a particle system with no type attached.
/// The system adds itself to the global particle system list, but it has no type
/// to take its behavior from until one is assigned to it.
/// </summary>
ParticleSystemClass::ParticleSystemClass(void) :
	BASECLASS(),
	Class(NULL),
	SystemParticles(),
	SpawnCoord(COORD_NONE),
	Source(NULL),
	Target(NULL),
	SpawnFrames(0),
	Lifetime(0),
	SparkSpawnFrames(0),
	SparkRadius(0),
	IsMarkedForDeletion(false),
	IsRandomSparkDirection(false)
{
	SystemParticles.Clear();
	ParticleSystems.Add(this);
	ObjectPtrTracker.Add(this);
}


/// <summary>
/// Destroys this particle system.
/// Every particle the system is still holding dies along with it, and the system
/// removes itself from the global tracking lists.
/// </summary>
ParticleSystemClass::~ParticleSystemClass(void)
{
	Detach_This_From_All(this);
	Limbo();

	while (SystemParticles.Count()) {
		delete SystemParticles[0];
		ObjectsToDelete.Delete(SystemParticles[0]);
		SystemParticles.Delete(SystemParticles[0]);
	}

	Class = NULL;

	ParticleSystems.Delete(this);
	AbstractTypePtrTracker.Delete(this);
	ObjectPtrTracker.Delete(this);
}


/// <summary>
/// Draws the momentary light this particle system casts.
/// The particles themselves are rendered by the layer they belong to; only systems
/// whose type calls for a one frame light draw anything here. The light waxes and
/// wanes with how full of particles the system currently is.
/// </summary>
void ParticleSystemClass::Draw_It(Point2D const & point, Rect const & cliprect) const
{
	if (Class->OneFrameLight) {
		if (Class->LightSize > 0 && SystemParticles.Count() > 0) {
			float cap = (float)SystemParticles.Count() / (float)Class->ParticleCap;
			float fullness = std::max(0.4f, std::min(1.0f, cap));
			int spotsize = (int)(fullness * Class->LightSize);
			SpotLightClass * spotlight = new SpotLightClass(PositionCoord, spotsize);
			spotlight->Set_Radius(SparkRadius);
			spotlight->Draw_It();
			delete spotlight;
		}
	}
}


/// <summary>
/// Creates one of the particles this system is built to hold.
/// The kind of particle comes from this system's own type, so a system that holds
/// nothing will quietly create nothing.
/// </summary>
/// <param name="coord1">The coordinate to create the particle at.</param>
/// <param name="coord2">The coordinate the particle should head toward.</param>
/// <returns>Returns with a pointer to the particle created. Otherwise, NULL is returned.</returns>
ParticleClass * ParticleSystemClass::Spawn_Held_Particle(Coord const & coord1, Coord const & coord2)
{
	if (Class->HoldsWhat == PARTICLE_NONE) {
		return(NULL);
	}

	ParticleClass * particle = new ParticleClass(ParticleTypes[Class->HoldsWhat], coord1, coord2, this);
	if (particle != NULL) {
		SystemParticles.Add(particle);
		return(particle);
	}
	return(NULL);
}


/// <summary>
/// Creates a particle of the specified type within this system.
/// Unlike the held particle routines, the kind of particle is dictated by the
/// caller rather than by this system's own type.
/// </summary>
/// <returns>Returns with a pointer to the particle created. Otherwise, NULL is returned.</returns>
ParticleClass * ParticleSystemClass::Spawn_Particle(ParticleTypeClass * type, Coord const & coord)
{
	ParticleClass * particle = new ParticleClass(type, coord, COORD_NONE, NULL);
	if (particle != NULL) {
		SystemParticles.Add(particle);
		return(particle);
	}
	return(NULL);
}


/// <summary>
/// Creates a held particle and slips it in at a random place in the system.
/// This routine is used where the draw order of the newest particles should vary,
/// so that a stream does not always paint its freshest particle on top of the
/// others.
/// </summary>
/// <param name="coord1">The coordinate to create the particle at.</param>
/// <param name="coord2">The coordinate the particle should head toward.</param>
/// <param name="v">How many of the trailing particles the new one may be shuffled among.</param>
/// <returns>Returns with a pointer to the particle created. Otherwise, NULL is returned.</returns>
ParticleClass * ParticleSystemClass::Spawn_Held_Particle_Random(Coord const & coord1, Coord const & coord2, int v)
{
	if (Class->HoldsWhat == PARTICLE_NONE) {
		return(NULL);
	}

	ParticleClass * particle = new ParticleClass(ParticleTypes[Class->HoldsWhat], coord1, coord2, this);
	if (particle == NULL || v <= 0) {
		return(NULL);
	}

	SystemParticles.Add(particle);

	int active_count = SystemParticles.Count();
	ParticleClass **vector = &SystemParticles[0];

	int random_range = std::min(active_count, v);
	int random_offset = abs(Scen->RandomNumber) % random_range;

	int source_index = active_count - 2;
	int stop_index = active_count - random_offset - 2;
	int insert_position = active_count - random_offset;

	if (source_index > stop_index) {
		ParticleClass **source = &vector[source_index];
		int move_count = source_index - stop_index;
		do {
			source[1] = *source;
			source--;
		} while (--move_count != 0);
	}
	vector[insert_position - 1] = particle;
	return(particle);
}


/// <summary>
/// Removes the specified particle from this system.
/// </summary>
/// <param name="index">Index of the particle to be removed.</param>
/// <returns>bool; Was there a particle at that index to remove?</returns>
bool ParticleSystemClass::Delete_Particle(int index)
{
	if (index < SystemParticles.Count()) {
		SystemParticles[index]->Delete_Me();
		SystemParticles.Delete(SystemParticles[index]);
		return(true);
	}
	return(false);
}


/// <summary>
/// Removes every particle this system is holding.
/// </summary>
void ParticleSystemClass::Clear_System(void)
{
	for (int i = SystemParticles.Count() - 1; i >= 0; i--) {
		Delete_Particle(i);
	}
}


/// <summary>
/// Handles the per-frame logic for a gas particle system.
/// This routine drifts the system's particles along and, as each one expires,
/// spawns the successor particle its type calls for, so that the cloud can change
/// character as it ages.
/// </summary>
void ParticleSystemClass::Gas_AI(void)
{
	int i;

	for (i = SystemParticles.Count()-1; i >= 0; i--) {
		SystemParticles[i]->Behavior_AI();
	}

	for (i = SystemParticles.Count()-1; i >= 0; i--) {
		ParticleClass *spart = SystemParticles[i];
		if (spart->IsToDie) {
			if (spart->Class->NextParticle != PARTICLE_NONE) {
				Coord coord = spart->PositionCoord;
				ParticleTypeClass *next = ParticleTypes[spart->Class->NextParticle];
				Coord ncoord = coord + spart->Class->NextParticleOffset;
				ParticleClass *npart = new ParticleClass(next, ncoord);
				if (npart != NULL) {
					SystemParticles.Add(npart);
					npart->Speed = spart->Speed;
					npart->GasDrift = spart->GasDrift;
				}
			}
			spart->Delete_Me();
		} else {
			spart->Motion_AI();
		}
	}
}


/// <summary>
/// Handles the per-frame logic for a spark particle system.
/// While the system has spark frames left it throws out bursts of particles, each flung
/// along a randomly perturbed direction, either the type's spawn direction or one picked at
/// random for the burst. A light is cast on the first spark frame, and the spark radius
/// wanders from frame to frame so the shower appears to flicker. The system retires once the
/// spark frames run out.
/// </summary>
void ParticleSystemClass::Spark_AI(void)
{
	if (SparkSpawnFrames > 0) {

		if (SparkSpawnFrames == 1 || Random_Double(0.0, 1.0) <= Class->SpawnSparkPercentage) {

			int random = Scen->RandomNumber();
			int count = Class->ParticleCap / 2 + abs(random) % (Class->ParticleCap / 2);

			ParticleTypeClass * held = ParticleTypes[Class->HoldsWhat];

			int rand_a = Scen->RandomNumber();
			int rand_b = Scen->RandomNumber();

			Vector3 spark;
			spark.Z = (float)(Scen->RandomNumber() % held->ZVelocityRange);
			spark.X = (float)(rand_b % held->XVelocity);
			spark.Y = (float)(rand_a % held->YVelocity);

			while (count > 0) {
				ParticleClass * particle = Spawn_Held_Particle(PositionCoord, PositionCoord);

				ParticleTypeClass const * ptype = particle->Class;
				Vector3 & dir = particle->MovementDirection;

				particle->MovementDirection.X = (float)(Scen->RandomNumber() % ptype->XVelocity);
				particle->MovementDirection.Y = (float)(Scen->RandomNumber() % ptype->YVelocity);
				particle->MovementDirection.Z = (float)(ptype->MinZVelocity + abs(Scen->RandomNumber()) % ptype->ZVelocityRange);

				float speed = (float)dir.Length();
				if (IsRandomSparkDirection) {
					particle->MovementDirection = spark + particle->MovementDirection;
				} else {
					Vector3 & spawndir = (Vector3 &)Class->SpawnDirection;
					particle->MovementDirection = spawndir + particle->MovementDirection;
				}
				particle->MovementDirection = Normalize(particle->MovementDirection);
				particle->MovementDirection = speed * particle->MovementDirection;

				count--;
			}

			if (Options.DetailLevel == 2) {
				if (SparkSpawnFrames == Class->SparkSpawnFrames && Class->LightSize > 0 && !Class->OneFrameLight) {
					new SpotLightClass(PositionCoord, Class->LightSize);
				}
			}
		}

		SparkSpawnFrames--;
		if (SparkSpawnFrames <= 0) {
			IsMarkedForDeletion = true;
		}

		double chance = Random_Double(0.0, 1.0);
		if (chance < 0.3) {
			int radius = SparkRadius - 3;
			if (radius <= 17) {
				radius = 17;
			}
			SparkRadius = radius;
		} else if (chance < 0.6) {
			int radius = SparkRadius + 3;
			if (radius >= 41) {
				radius = 41;
			}
			SparkRadius = radius;
		}
	}

	for (int i = 0; i < SystemParticles.Count(); i++) {
		SystemParticles[i]->Behavior_AI();
	}

	for (int j = SystemParticles.Count() - 1; j >= 0; j--) {
		ParticleClass * particle = SystemParticles[j];
		if (particle->IsToDie) {
			particle->Delete_Me();
		}
	}
}


/// <summary>
/// Handles the per-frame logic for a smoke particle system.
/// The system rides along with whatever it is attached to, unless that is a building, ages
/// its particles, and replaces each expiring one with a pair of successors thrown out to
/// either side, so that the column spreads as it rises. Fresh particles are emitted on the
/// spawn interval, which lengthens as the system tires until it finally retires.
/// </summary>
void ParticleSystemClass::Smoke_AI(void)
{
	int i;

	if (Source->As_ObjectClass() != NULL && Source->What_Am_I() != RTTI_BUILDING) {
		Set_Coord(Source->Center_Coord() + CoordOffset);
	}

	for (i = 0; i < SystemParticles.Count(); i++) {
		SystemParticles[i]->Behavior_AI();
	}

	for (i = SystemParticles.Count() - 1; i >= 0; i--) {
		ParticleClass *spart = SystemParticles[i];
		if (spart->IsToDie) {
			if (spart->Class->NextParticle != PARTICLE_NONE) {
				Coord coord = spart->Get_Coord();
				int x = coord.X;
				int y = coord.Y;
				int z = coord.Z;

				int xspread = spart->Class->Radius >> 3;
				int xrand = Scen->RandomNumber() % xspread;
				if (xrand <= 0) {
					xrand = xrand - xspread;
				} else {
					xrand = xspread + xrand;
				}

				ParticleTypeClass const *type = spart->Class;
				int yspread = type->Radius >> 3;
				int yrand = Scen->RandomNumber() % yspread;
				if (yrand <= 0) {
					yrand = yrand - yspread;
				} else {
					yrand = yspread + yrand;
				}

				ParticleTypeClass *next = ParticleTypes[type->NextParticle];

				Coord ncoord;
				ncoord.Y = y + yrand;
				ncoord.Z = z;
				ncoord.X = x + xrand;
				ParticleClass *npart = new ParticleClass(next, ncoord);
				if (npart != NULL) {
					SystemParticles.Add(npart);
					if (npart != NULL) {
						npart->Speed = spart->Speed;
						npart->Translucency = spart->Translucency + (Scen->RandomNumber() % 6 != 0 ? 25 : 0);
					}
				}

				Coord pcoord;
				pcoord.X = x - xrand;
				pcoord.Y = y - yrand;
				pcoord.Z = z;
				ParticleClass *ppart = new ParticleClass(next, pcoord);
				if (ppart != NULL) {
					SystemParticles.Add(ppart);
					ppart->Speed = spart->Speed;
					ppart->Translucency = spart->Translucency + (Scen->RandomNumber() % 6 != 0 ? 25 : 0);
				}
			}
			spart->Delete_Me();
		} else {
			SystemParticles[i]->Motion_AI();
		}
	}

	if (!IsMarkedForDeletion && IsActive && (Frame % (int)SpawnFrames) == 0) {
		FootClass *foot = Source->As_FootClass();
		if (foot == NULL || foot->CurrentTube < 0) {
			int xrand = Scen->RandomNumber();
			int yrand = Scen->RandomNumber();
			int range = Class->SpawnRadius + 1;
			Coord scoord = Get_Coord() + Coord(xrand % range, yrand % range, 10);

			if (Class->HoldsWhat != PARTICLE_NONE) {
				ParticleClass *npart = new ParticleClass(ParticleTypes[Class->HoldsWhat], scoord, SpawnCoord, this);
				if (npart != NULL) {
					SystemParticles.Add(npart);
					if (SpawnFrames > Class->SpawnTranslucencyCutoff) {
						npart->Translucency += 25;
					}
					npart->Speed = npart->Speed - (SpawnFrames - Class->SpawnFrames) * 0.35f;
					if (npart->Speed < 2.0) {
						npart->Speed = 2.0;
					}
				}
			}
		}
	}

	SpawnFrames = Class->Slowdown + SpawnFrames;
	if (SpawnFrames > Class->SpawnCutoff) {
		IsMarkedForDeletion = true;
	}
}


/// <summary>
/// Handles the per-frame logic for a railgun particle system.
/// The first pass lays a spiral of particles along the line from the firer to the
/// target, jittering each one so that the trail looks ragged, and draws the
/// accompanying laser beam if the type calls for one. After that the system merely
/// ages its particles until the last of them has faded.
/// </summary>
void ParticleSystemClass::Railgun_AI(void)
{
	int i;

	if (!IsMarkedForDeletion && !SystemParticles.Count()) {
		Coord pos = Get_Coord();
		int dx = SpawnCoord.X - pos.X;
		int dy = SpawnCoord.Y - pos.Y;
		int dz = SpawnCoord.Z - pos.Z;

		double dz_double = (double)dz;
		double dy_double = (double)dy;
		double dx_double = (double)dx;
		int total_dist = (int)std::sqrt(dx_double * dx_double + dy_double * dy_double + dz_double * dz_double);
		int horiz_dist = (int)std::sqrt(dy_double * dy_double + dx_double * dx_double);

		int clamped_z = dz;
		int clamped_x = dx;

		if (clamped_z >= total_dist) clamped_z = total_dist;
		if (clamped_z <= -total_dist) clamped_z = -total_dist;

		if (clamped_x >= horiz_dist) clamped_x = horiz_dist;
		if (clamped_x <= -horiz_dist) clamped_x = -horiz_dist;

		int num_particles = (int)((double)total_dist * Class->ParticlesPerCoord);
		float dist_f = (float)total_dist;

		float elevation = (float)std::asin((double)clamped_z / (double)dist_f);
		float azimuth = (float)std::acos((double)clamped_x / (double)horiz_dist);

		if (dy < 0) {
			azimuth = -azimuth;
		}

		Matrix3D mat;
		mat.Make_Identity();
		mat.Rotate_Z(azimuth);
		mat.Rotate_X(elevation);

		float velocity_accum = 0.0f;
		i = 0;
		if (num_particles > 0) {
			float num_particles_f = (float)num_particles;
			do {
				float frac = (float)i / num_particles_f;
				double angle = dist_f * frac * Class->SpiralDeltaPerCoord;

				double sine = std::sin(angle);
				double cosine = std::cos(angle);
				Vector3 spiral(0.0f, cosine, sine);
				spiral = mat * spiral;
				Vector3 scaled = spiral * (float)Class->SpiralRadius;
				Vector3 tmp = Vector3(Random_Double(-0.5, 0.5) * Class->PositionPerturbationCoefficient, Random_Double(-0.5, 0.5) * Class->PositionPerturbationCoefficient, Random_Double(-0.5, 0.5) * Class->PositionPerturbationCoefficient);

				scaled += tmp;
				Coord ic((int)scaled.X, (int)scaled.Y, (int)scaled.Z);
				Coord particle_pos = ic + Lerp(PositionCoord, SpawnCoord, frac);

				ParticleClass * particle = Spawn_Held_Particle(particle_pos, particle_pos);

				particle->MovementDirection = spiral;
				particle->MovementDirection += Vector3(Random_Double(-0.5, 0.5) * Class->MovementPerturbationCoefficient, Random_Double(-0.5, 0.5) * Class->MovementPerturbationCoefficient, Random_Double(-0.5, 0.5) * Class->MovementPerturbationCoefficient);
				particle->MovementDirection = Normalize(particle->MovementDirection);

				double vp = (Random_Double(-0.5, 0.5) + velocity_accum) * (Class->VelocityPerturbationCoefficient * 0.5);
				velocity_accum = (float)std::max(std::min(vp, Class->VelocityPerturbationCoefficient), -Class->MovementPerturbationCoefficient);
				particle->Speed = velocity_accum + particle->Class->Velocity;
				i++;
			} while (i < num_particles);
		}

		if (Class->IsLaser) {
			new LaserDrawClass(PositionCoord, SpawnCoord, 0, true, Class->LaserColor, RGBClass(0, 0, 0), RGBClass(0, 0, 0), 10, false, true, 0.5f, 0.0f);
		}

		IsMarkedForDeletion = true;
	}

	for (i = 0; i < SystemParticles.Count(); i++) {
		SystemParticles[i]->Behavior_AI();
	}

	for (int j = SystemParticles.Count() - 1; j >= 0; j--) {
		ParticleClass * particle = SystemParticles[j];
		if (particle->IsToDie) {
			particle->Delete_Me();
		}
	}
}


/*
 * These are the unit-step tables used to nudge a fire particle's spawn point sideways,
 * indexed by facing in the order N, NE, E, SE, S, SW, W, NW. Only Fire_AI reads them.
 */
static int _FireStepX[FACING_COUNT] = { 1,  1,  0, -1, -1, -1,  0,  1 };
static int _FireStepY[FACING_COUNT] = { 0,  1,  1,  1,  0, -1, -1, -1 };


/// <summary>
/// Handles the per-frame logic for a fire stream particle system.
/// The stream follows the firer's aim while its turret is turning, and each fresh
/// particle is launched toward a point that swings in and out along the line of fire, so
/// that the jet appears to pulse.
/// </summary>
void ParticleSystemClass::Fire_AI(void)
{
	int i;

	for (i = SystemParticles.Count()-1; i >= 0; i--) {
		ParticleClass *spart = SystemParticles[i];
		spart->Behavior_AI();
		spart->Motion_AI();
	}

	for (i = SystemParticles.Count()-1; i >= 0; i--) {
		ParticleClass *spart = SystemParticles[i];
		if (spart->IsToDie) {
			spart->Delete_Me();
		}
	}

	bool reaimed = false;
	TechnoClass *source = Dynamic_Cast<TechnoClass *>(Source);
	if (source != NULL) {
		if (source->IsActive) {
			if (source->TarCom != NULL && source->PrimaryFacing.Is_Rotating()) {
				Coord cc = source->TarCom->Center_Coord();
				Coord delta = cc - source->Get_Coord();
				int dist = (int)std::sqrt((double)delta.X * (double)delta.X + (double)delta.Y * (double)delta.Y + (double)delta.Z * (double)delta.Z);
				SpawnCoord = Move_Coord(source->Get_Coord(), source->PrimaryFacing.Current(), dist);
				Set_Coord(source->Fire_Coord(0));
				reaimed = true;
			}
		}
		if (!source->IsActive) {
			Delete_Me();
		}
	} else {
		Delete_Me();
	}

	if (!IsMarkedForDeletion && (!(Frame % Class->SpawnFrames) || (!(Frame % 3) && reaimed))) {
		int index = Frame % 500;

		Coord pos = Get_Coord();
		int dx = pos.X - SpawnCoord.X;
		int dz = pos.Z - SpawnCoord.Z;
		int dy = pos.Y - SpawnCoord.Y;
		int distsq = dx * dx + dy * dy + dz * dz;

		int divisor;
		if ((float)std::sqrt((double)distsq) < 200.0f) {
			divisor = 3;
		} else {
			divisor = 1;
		}

		int offset = (int)((double)(12 * SineTable[index]) / ((float)divisor * 3.0f));

		FacingType face = Facing_Between_Points(Get_Coord(), SpawnCoord);

		Coord spawn = Coord(SpawnCoord.X + offset * _FireStepX[face], SpawnCoord.Y + offset * _FireStepY[face], SpawnCoord.Z);

		Spawn_Held_Particle_Random(Get_Coord() + Coord(0, 0, 1), spawn, 4);
	}
}


/// <summary>
/// Handles the per-frame logic for a web particle system.
/// This routine drifts the system's particles along and, as each one expires,
/// spawns the successor particle its type calls for in its place.
/// </summary>
void ParticleSystemClass::Web_AI(void)
{
	int i;

	for (i = SystemParticles.Count()-1; i >= 0; i--) {
		SystemParticles[i]->Behavior_AI();
	}

	for (i = SystemParticles.Count()-1; i >= 0; i--) {
		ParticleClass *spart = SystemParticles[i];
		if (spart->IsToDie) {
			if (spart->Class->NextParticle != PARTICLE_NONE) {
				Coord coord = spart->PositionCoord;
				ParticleTypeClass *next = ParticleTypes[spart->Class->NextParticle];
				Coord ncoord = coord + spart->Class->NextParticleOffset;
				ParticleClass *npart = new ParticleClass(next, ncoord);
				if (npart != NULL) {
					SystemParticles.Add(npart);
					npart->Speed = spart->Speed;
					npart->GasDrift = spart->GasDrift;
				}
			}
			spart->Delete_Me();
		} else {
			spart->Motion_AI();
		}
	}
}


/// <summary>
/// Handles the per-frame logic for this particle system.
/// This routine hands the system over to the update handler for whatever behavior
/// its type calls for, ages the system, and retires it once its lifetime has run
/// out and the last of its particles is gone.
/// </summary>
void ParticleSystemClass::AI(void)
{
	switch (Class->BehavesLike) {
		case PSYS_BEHAVIOR_SMOKE:
			Smoke_AI();
			break;
		case PSYS_BEHAVIOR_GAS:
			Gas_AI();
			break;
		case PSYS_BEHAVIOR_WEAKGAS:
			Gas_AI();
			break;
		case PSYS_BEHAVIOR_FIRE:
			Fire_AI();
			break;
		case PSYS_BEHAVIOR_SPARK:
			Spark_AI();
			break;
		case PSYS_BEHAVIOR_RAILGUN:
			Railgun_AI();
			break;
		case PSYS_BEHAVIOR_WEB:
			Web_AI();
			break;
	}

	Lifetime--;
	if (Lifetime == 0) {
		Delete_Me();
	}
	if (IsActive && (IsMarkedForDeletion && SystemParticles.Count() == 0)) {
		Limbo();
		IsActive = false;
		ObjectsToDelete.Add(this);
	}
}


/// <summary>
/// Has this particle system finished its work?
/// A system is spent once it has been marked for deletion and the last of the
/// particles it was holding has died off.
/// </summary>
/// <returns>bool; Is the particle system spent?</returns>
bool ParticleSystemClass::Is_Inactive(void) const
{
	return(IsMarkedForDeletion && SystemParticles.Count() == 0);
}


/// <summary>
/// Determines which display layer this system is rendered in.
/// </summary>
/// <returns>Returns with the layer that this object should be drawn in.</returns>
LayerType ParticleSystemClass::In_Which_Layer(void) const
{
	return(LAYER_GROUND);
}


/// <summary>
/// Removes any reference this particle system has to the specified object.
/// This routine is called when an object is about to disappear, so that nothing
/// is left pointing at it. Losing the source object the system was attached to
/// also marks the system itself for deletion.
/// </summary>
/// <param name="target">The object that is going away.</param>
/// <param name="all">Is the object going away for good, as opposed to merely cloaking?</param>
void ParticleSystemClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);
	SystemParticles.Delete((ParticleClass *)target);
	if (target == Class) {
		Class = NULL;
	}
	if (target == Target) {
		Target = NULL;
	}
	if (target == Source) {
		IsMarkedForDeletion = true;
		Source = NULL;
	}
}


/// <summary>
/// Lists the members this particle system carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void ParticleSystemClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Class);
	stream.Serialize(CoordOffset);
	stream.Serialize(SystemParticles);
	stream.Serialize(SpawnCoord);
	stream.Serialize(Source);
	stream.Serialize(Target);
	stream.Serialize(SpawnFrames);
	stream.Serialize(Lifetime);
	stream.Serialize(SparkSpawnFrames);
	stream.Serialize(SparkRadius);
	stream.Serialize(IsMarkedForDeletion);
	stream.Serialize(IsRandomSparkDirection);
}


/// <summary>
/// Adds this particle system's state to the running game checksum.
/// This routine is used by the multiplayer sync check to notice when two machines
/// have drifted out of step with each other.
/// </summary>
/// <param name="crc">The checksum engine to submit this object's data to.</param>
void ParticleSystemClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	for (int i = 0; i < SystemParticles.Count(); i++) {
		crc(SystemParticles[i]->Fetch_ID());
	}
	if (Source != NULL) {
		crc(Source->Fetch_ID());
	}
	if (Target != NULL) {
		crc(Target->Fetch_ID());
	}
	crc(SpawnFrames);
	crc(Lifetime);
}


/// <summary>
/// Fetches the class identifier for this object.
/// This routine is part of the persistence support. The save process records the
/// identifier so that the load process knows what kind of object to build.
/// </summary>
/// <param name="retval">Pointer to the class identifier to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT ParticleSystemClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_ParticleSystemClass;
	return(S_OK);
}


/// <summary>
/// Fetches the RTTI type of this object.
/// </summary>
/// <returns>Returns with RTTI_PARTICLESYSTEM.</returns>
RTTIType ParticleSystemClass::Fetch_RTTI(void) const
{
	return(RTTI_PARTICLESYSTEM);
}


/// <summary>
/// Fetches the type object this particle system was created from.
/// </summary>
/// <returns>Returns with a pointer to the particle system type class object.</returns>
ObjectTypeClass const * ParticleSystemClass::Class_Of(void) const
{
	return(Class);
}
