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

#include "psystype.h"

#include "ccini.h"
#include "findmake.h"
#include "globals.h"
#include "incdec.h"
#include "ptype.h"
#include "savestream.h"
#include "sun.h"
#include "tracker.h"
#include "warhead.h"


/// <summary>
/// Constructs a particle system type with default values.
/// The new type is given a heap ID and added to the master particle system type list, so
/// that the rules and the game code can refer to it by name from this point on.
/// </summary>
/// <param name="ininame">The INI section name that describes this particle system.</param>
ParticleSystemTypeClass::ParticleSystemTypeClass(char const * ininame) :
	BASECLASS(ininame),
	HoldsWhat(PARTICLE_NONE),
	BehavesLike(PSYS_BEHAVIOR_NONE),
	Lifetime(-1),
	SpawnDirection(0, 0, 0),
	Spawns(false),
	SpawnFrames(1),
	Slowdown(0.0),
	ParticleCap(50),
	SpawnRadius(0),
	SpawnCutoff(0.0),
	SpawnTranslucencyCutoff(0.0),
	ParticlesPerCoord(.1),
	SpiralDeltaPerCoord(.025),
	SpiralRadius(25.0),
	PositionPerturbationCoefficient(0.0),
	MovementPerturbationCoefficient(0.0),
	VelocityPerturbationCoefficient(0.0),
	SpawnSparkPercentage(0.0),
	SparkSpawnFrames(0),
	LightSize(0),
	LaserColor(),
	IsLaser(false),
	OneFrameLight(false)
{
	Create_ID();
	ParticleSystemTypes.Add(this);
	IsSentient = true;
}


/// <summary>
/// Destroys the particle system type.
/// Every object that still refers to this type is detached from it first, so that no
/// dangling reference survives the removal from the master list.
/// </summary>
ParticleSystemTypeClass::~ParticleSystemTypeClass(void)
{
	Detach_This_From_All(this);
	ParticleSystemTypes.Delete(this);
}


/// <summary>
/// Fetches this particle system type's data from the INI database.
/// This routine is called while the rules are being parsed. It lays the particle system
/// specific entries over whatever the base class has already picked up.
/// </summary>
/// <returns>bool; Was the data read?</returns>
bool ParticleSystemTypeClass::Read_INI(CCINIClass const & ini)
{
	char buffer[64];

	if (BASECLASS::Read_INI(ini)) {

		ini.Get_String(IniName, "HoldsWhat", "", buffer, sizeof(buffer));
		HoldsWhat = ParticleTypeClass::From_Name(buffer);

		Spawns = ini.Get_Bool(IniName, "Spawns", Spawns);
		SpawnFrames = ini.Get_Int(IniName, "SpawnFrames", SpawnFrames);
		ParticleCap = ini.Get_Int(IniName, "ParticleCap", ParticleCap);
		SpawnRadius = ini.Get_Int(IniName, "SpawnRadius", SpawnRadius);
		Slowdown = ini.Get_Float(IniName, "Slowdown", Slowdown);
		SpawnCutoff = ini.Get_Float(IniName, "SpawnCutoff", SpawnCutoff);
		SpawnTranslucencyCutoff = ini.Get_Float(IniName, "SpawnTranslucencyCutoff", SpawnTranslucencyCutoff);
		Lifetime = ini.Get_Int(IniName, "Lifetime", Lifetime);

		ini.Get_String(IniName, "BehavesLike", "", buffer, sizeof(buffer));
		BehavesLike = Particle_System_Behavior_From_Name(buffer);

		SpawnDirection = ini.Get_Vector(IniName, "SpawnDirection", SpawnDirection);
		ParticlesPerCoord = ini.Get_Float(IniName, "ParticlesPerCoord", ParticlesPerCoord);
		SpiralDeltaPerCoord = ini.Get_Float(IniName, "SpiralDeltaPerCoord", SpiralDeltaPerCoord);
		SpiralRadius = ini.Get_Float(IniName, "SpiralRadius", SpiralRadius);
		PositionPerturbationCoefficient = ini.Get_Float(IniName, "PositionPerturbationCoefficient", PositionPerturbationCoefficient);
		MovementPerturbationCoefficient = ini.Get_Float(IniName, "MovementPerturbationCoefficient", MovementPerturbationCoefficient);
		VelocityPerturbationCoefficient = ini.Get_Float(IniName, "VelocityPerturbationCoefficient", VelocityPerturbationCoefficient);
		IsLaser = ini.Get_Bool(IniName, "Laser", IsLaser);
		LaserColor = ini.Get_RGBClass(IniName, "LaserColor", LaserColor);
		SparkSpawnFrames = ini.Get_Int(IniName, "SparkSpawnFrames", SparkSpawnFrames);
		LightSize = ini.Get_Int(IniName, "LightSize", LightSize);
		OneFrameLight = ini.Get_Bool(IniName, "OneFrameLight", OneFrameLight);
		SpawnSparkPercentage = ini.Get_Float(IniName, "SpawnSparkPercentage", SpawnSparkPercentage);

		return(true);
	}
	return(false);
}


/// <summary>
/// Converts a particle system name into its type number.
/// This routine will create a new particle system type when the name is not already
/// known, so that the rules may refer to a system before it has been declared.
/// </summary>
/// <param name="name">The name of the particle system to convert.</param>
/// <returns>Returns with the type of the particle system. PARTSYS_NONE is returned when no
/// usable name was supplied.</returns>
ParticleSystemType ParticleSystemTypeClass::From_Name(char const * name)
{
	if (name != NULL && stricmp(name, "<none>") && strlen(name)) {
		for (int index = PARTSYS_FIRST; index < ParticleSystemTypes.Count(); index++) {
			if (stricmp(name, ParticleSystemTypes[index]->Name()) == 0) {
				return(ParticleSystemType(index));
			}
		}
		ParticleSystemTypeClass *ptr = new ParticleSystemTypeClass(name);
		return(ParticleSystemType(ParticleSystemTypes.ID(ptr)));

	}
	return(PARTSYS_NONE);
}


/// <summary>
/// Converts a particle system type into its name.
/// </summary>
/// <returns>Returns with the name of the particle system type. An invalid type yields the
/// placeholder name used for "no system".</returns>
const char * ParticleSystemTypeClass::Name_From(ParticleSystemType partsys)
{
	if ((unsigned)partsys < (unsigned)ParticleSystemTypes.Count()) {
		return(ParticleSystemTypes[partsys]->Name());
	}
	return("<none>");
}


/// <summary>
/// Submits this particle system type to the CRC calculation.
/// This routine is used by the multiplayer sync check so that players running with
/// different rules are caught rather than left to drift apart.
/// </summary>
/// <param name="crc">The CRC engine to submit this object's data to.</param>
void ParticleSystemTypeClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);

	crc(HoldsWhat);
	crc(Spawns);
	crc(SpawnFrames);
	crc(Slowdown);
	crc(ParticleCap);
	crc(SpawnRadius);
	crc(SpawnCutoff);
	crc(SpawnTranslucencyCutoff);
	crc(BehavesLike);
	crc(Lifetime);
}


/// <summary>
/// Fetches the class identifier of this object.
/// This routine is part of the persistence interface. The save system records the class
/// ID so that the right kind of object can be manufactured when the game is reloaded.
/// </summary>
/// <param name="retval">Pointer to the class ID to be filled in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT ParticleSystemTypeClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_ParticleSystemTypeClass;
	return(S_OK);
}


/// <summary>
/// Lists the members this particle system type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void ParticleSystemTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HoldsWhat);
	stream.Serialize(Spawns);
	stream.Serialize(SpawnFrames);
	stream.Serialize(Slowdown);
	stream.Serialize(ParticleCap);
	stream.Serialize(SpawnRadius);
	stream.Serialize(SpawnCutoff);
	stream.Serialize(SpawnTranslucencyCutoff);
	stream.Serialize(BehavesLike);
	stream.Serialize(Lifetime);
	stream.Serialize(SpawnDirection);
	stream.Serialize(ParticlesPerCoord);
	stream.Serialize(SpiralDeltaPerCoord);
	stream.Serialize(SpiralRadius);
	stream.Serialize(PositionPerturbationCoefficient);
	stream.Serialize(MovementPerturbationCoefficient);
	stream.Serialize(VelocityPerturbationCoefficient);
	stream.Serialize(SpawnSparkPercentage);
	stream.Serialize(SparkSpawnFrames);
	stream.Serialize(LightSize);
	stream.Serialize(LaserColor);
	stream.Serialize(IsLaser);
	stream.Serialize(OneFrameLight);
}


/// <summary>
/// Converts a behavior name into its particle system behavior type.
/// This routine is used when the BehavesLike entry is read from the rules.
/// </summary>
/// <param name="name">The behavior name to convert.</param>
/// <returns>Returns with the behavior that matches the name. An unrecognized name yields
/// PSYS_BEHAVIOR_NONE.</returns>
ParticleSystemBehaviorType Particle_System_Behavior_From_Name(char const * name)
{
	static char const * _BehaviorNames[PSYS_BEHAVIOR_COUNT] = {
		"Smoke",
		"Gas",
		"Fire",
		"Spark",
		"Railgun",
		"Web",
		"WeakGas"
	};

	for (ParticleSystemBehaviorType behavior = PSYS_BEHAVIOR_FIRST; behavior < PSYS_BEHAVIOR_COUNT; behavior++) {
		if (!strcmpi(name, _BehaviorNames[behavior])) {
			return(behavior);
		}
	}
	return(PSYS_BEHAVIOR_NONE);
}


/// <summary>
/// Fetches the particle system type of the name specified, creating it if need be.
/// This routine is used while the rules are being parsed so that one section may refer
/// to a particle system whose own section has not been reached yet.
/// </summary>
/// <param name="name">The INI name of the particle system type to find.</param>
/// <returns>Returns with a pointer to the type found or created.</returns>
ParticleSystemTypeClass * ParticleSystemTypeClass::Find_Or_Make(char const * name)
{
	return(TFind_Or_Make<ParticleSystemTypeClass>(name, ParticleSystemTypes));
}


/// <summary>
/// Fetches the RTTI type of this object.
/// </summary>
/// <returns>Returns with RTTI_PARTICLESYSTEMTYPE.</returns>
RTTIType ParticleSystemTypeClass::Fetch_RTTI(void) const
{
	return(RTTI_PARTICLESYSTEMTYPE);
}


/// <summary>
/// Creates and places a particle system upon the map.
/// Particle systems are never placed by the scenario or by the map editor's placement
/// logic, so this routine always refuses the request.
/// </summary>
/// <returns>bool; Was the object placed?</returns>
bool ParticleSystemTypeClass::Create_And_Place(Cell const & , HouseClass * house) const
{
	return(false);
}


/// <summary>
/// Creates a particle system object of this type.
/// Particle systems are spawned by whatever owns them rather than through the generic
/// object type interface, so this routine never manufactures one.
/// </summary>
/// <returns>Returns with a pointer to the object created, which is always NULL.</returns>
ObjectClass * ParticleSystemTypeClass::Create_One_Of(HouseClass *) const
{
	return(NULL);
}
