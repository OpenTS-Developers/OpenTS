/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "vanimtype.h"

#include "animtype.h"
#include "ccfile.h"
#include "ccini.h"
#include "findmake.h"
#include "globals.h"
#include "psystype.h"
#include "savestream.h"
#include "stimer.h"
#include "sun.h"
#include "swizzle.h"
#include "techtype.h"
#include "tracker.h"
#include "vector.h"
#include "warhead.h"


/// <summary>
/// Creates a voxel animation type of the name specified.
/// The new type is added to the global voxel animation type list and starts out with
/// the manners of a harmless piece of flying debris -- it cannot be selected, targeted
/// or harmed, and it takes no part in the game logic.
/// </summary>
/// <param name="ininame">The INI section name that identifies this voxel animation.</param>
VoxelAnimTypeClass::VoxelAnimTypeClass(char const * ininame) :
	BASECLASS(ininame),
	IsNormalized(false),
	IsTranslucent(false),
	IsSharesSourceData(false),
	VoxelIndex(0),
	Duration(2 * TICKS_PER_SECOND),
	Elasticity(0.8f),
	MinAngularVelocity(0),
	/// Ten degrees per frame, converted with the truncated pi constant.
	MaxAngularVelocity(10.0 * (3.1415 / 180.0)),
	MinZVel(3.5),
	MaxZVel(5),
	MaxXYVel(15),
	IsMeteor(false),
	Spawns(NULL),
	SpawnCount(0),
	StartSound(VOC_NONE),
	BounceSound(VOC_NONE),
	ExpireSound(VOC_NONE),
	BounceAnim(NULL),
	ExpireAnim(NULL),
	TrailerAnim(NULL),
	Damage(0),
	DamageRadius(0),
	Warhead(NULL),
	AttachedSystem(NULL),
	IsTiberium(false)
{
	Create_ID();
	IsLegalTarget = false;
	IsSentient = true;
	IsStealthy = true;
	IsSelectable = true;
	IsInsignificant = true;
	IsImmune = true;
	IsFootprint = false;
	AbstractTypePtrTracker.Add(this);
	VoxelAnimTypes.Add(this);
}


/// <summary>
/// Removes this voxel animation type from the game.
/// Anything still referring to this type is detached from it first. A voxel image that
/// was borrowed from another object type is merely let go of, since it belongs to that
/// other type and must not be destroyed here.
/// </summary>
VoxelAnimTypeClass::~VoxelAnimTypeClass(void)
{
	Detach_This_From_All(this);
	AbstractTypePtrTracker.Delete(this);
	VoxelAnimTypes.Delete(this);
	if (IsSharesSourceData) {
		Voxel.VoxLib = NULL;
		Voxel.MotLib = NULL;
	}
}


/// <summary>
/// Fetches this voxel animation type's data from the INI database.
/// This routine reads the bouncing, spawning, sound and damage settings that give the
/// voxel animation its character. The voxel image itself either comes from a .VXL file
/// of its own, or is borrowed from the body, turret or barrel of some other object type.
/// </summary>
/// <returns>bool; Was the data read from the database?</returns>
bool VoxelAnimTypeClass::Read_INI(CCINIClass const & ini)
{
	if (BASECLASS::Read_INI(ini)) {

		IsNormalized = ini.Get_Bool(IniName, "Normalized", IsNormalized);
		IsTranslucent = ini.Get_Bool(IniName, "Translucent", IsTranslucent);
		IsTiberium = ini.Get_Bool(IniName, "IsTiberium", IsTiberium);
		IsMeteor = ini.Get_Bool(IniName, "IsMeteor", IsMeteor);
		Elasticity = ini.Get_Float(IniName, "Elasticity", Elasticity);

		double min_angular_vel = ini.Get_Float(IniName, "MinAngularVelocity", -1.0);
		if (min_angular_vel != 0.0) {
			MinAngularVelocity = min_angular_vel * (3.1415 / 180.0);
		}

		double max_angular_vel = ini.Get_Float(IniName, "MaxAngularVelocity", -1.0);
		if (max_angular_vel != 0.0) {
			MaxAngularVelocity = max_angular_vel * (3.1415 / 180.0);
		}

		Duration = ini.Get_Int(IniName, "Duration", Duration);
		MinZVel = ini.Get_Float(IniName, "MinZVel", MinZVel);
		MaxZVel = ini.Get_Float(IniName, "MaxZVel", MaxZVel);
		MaxXYVel = ini.Get_Float(IniName, "MaxXYVel", MaxXYVel);

		Spawns = TGet_Class(ini, IniName, "Spawns", Spawns);
		SpawnCount = ini.Get_Int(IniName, "SpawnCount", SpawnCount);

		bool share_body_data = ini.Get_Bool(IniName, "ShareBodyData", false);
		bool share_turret_data = ini.Get_Bool(IniName, "ShareTurretData", false);
		bool share_barrel_data = ini.Get_Bool(IniName, "ShareBarrelData", false);

		TargetClass share_source;

		VoxelIndex = ini.Get_Int(IniName, "VoxelIndex", VoxelIndex);

		StartSound = ini.Get_VocType(IniName, "StartSound", StartSound);
		BounceSound = ini.Get_VocType(IniName, "BounceSound", BounceSound);
		ExpireSound = ini.Get_VocType(IniName, "ExpireSound", ExpireSound);

		BounceAnim = TGet_Class(ini, IniName, "BounceAnim", BounceAnim);
		ExpireAnim = TGet_Class(ini, IniName, "ExpireAnim", ExpireAnim);
		TrailerAnim = TGet_Class(ini, IniName, "TrailerAnim", TrailerAnim);

		Damage = ini.Get_Int(IniName, "Damage", Damage);
		DamageRadius = ini.Get_Int(IniName, "DamageRadius", DamageRadius);

		Warhead = TGet_Class(ini, IniName, "Warhead", Warhead);
		AttachedSystem = TGet_Class(ini, IniName, "AttachedSystem", AttachedSystem);

		IsSharesSourceData = share_body_data || share_barrel_data || share_turret_data;

		if (IsSharesSourceData) {
			share_source = ini.Get_Target(IniName, "ShareSource", TargetClass());
			TechnoTypeClass const * source = share_source.As_TechnoType();
			if (source) {
				if (share_body_data) {
					Voxel = source->Voxel;
				} else if (share_turret_data) {
					Voxel = source->AuxVoxel;
				} else if (share_barrel_data) {
					Voxel = source->AuxVoxel2;
				}
			} else {
				Voxel.VoxLib = NULL;
				Voxel.MotLib = NULL;
			}
		} else {
			char path[512];
			_makepath(path, NULL, NULL, GraphicName, ".VXL");
			CCFileClass file(path);
			if (Voxel.VoxLib) {
				delete Voxel.VoxLib;
			}
			Voxel.VoxLib = new VoxelLibrary(file, false);
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the INI name of the voxel animation type specified.
/// </summary>
/// <returns>Returns with a pointer to the name, or an empty string if the type is not
/// a legal one.</returns>
char const * VoxelAnimTypeClass::Name_From(VoxelAnimType anim)
{
	if ((unsigned)anim > (unsigned)VoxelAnimTypes.Count()) return("");

	return(VoxelAnimTypes[anim]->IniName);
}


/// <summary>
/// Converts an ASCII name into a voxel animation type number.
/// This routine is used when the rules or a map refer to a voxel animation by name.
/// The comparison ignores case.
/// </summary>
/// <param name="name">The INI name of the voxel animation to search for.</param>
/// <returns>Returns with the type matched, or VANIM_NONE if the name is not known.</returns>
VoxelAnimType VoxelAnimTypeClass::From_Name(char const * name)
{
	if (name != NULL) {
		for (int index = VANIM_FIRST; index < VoxelAnimTypes.Count(); index++) {
			if (stricmp(VoxelAnimTypes[index]->IniName, name) == 0) {
				return(VoxelAnimType(index));
			}
		}
	}
	return(VANIM_NONE);
}


/// <summary>
/// Submits this voxel animation type's data to the CRC accumulator.
/// This routine is used by the multiplayer sync checking, which verifies that every
/// machine in the game agrees on the rules data it was handed.
/// </summary>
void VoxelAnimTypeClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(IsNormalized);
	crc(IsTranslucent);
	crc(IsSharesSourceData);
	crc(VoxelIndex);
	crc(Duration);
	crc(Elasticity);
	crc(MinAngularVelocity);
	crc(MaxAngularVelocity);
	crc(MinZVel);
	crc(MaxZVel);
	crc(MaxXYVel);
	crc(IsMeteor);
	crc(SpawnCount);
	crc(StartSound);
	crc(BounceSound);
	crc(ExpireSound);
	crc(Damage);
	crc(DamageRadius);
	crc(IsTiberium);
}


ClassID VoxelAnimTypeClass::Class_ID(void) const
{
	return(ClassID_VoxelAnimTypeClass);
}


/// <summary>
/// Re-attaches the artwork this voxel animation type names.
/// The voxel image is fetched again once the members have been read.
/// </summary>
void VoxelAnimTypeClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	Fetch_Voxel_Image();
}


/// <summary>
/// Lists the members this voxel animation type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void VoxelAnimTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(IsNormalized);
	stream.Serialize(IsTranslucent);
	stream.Serialize(IsSharesSourceData);
	stream.Serialize(VoxelIndex);
	stream.Serialize(Duration);
	stream.Serialize(Elasticity);
	stream.Serialize(MinAngularVelocity);
	stream.Serialize(MaxAngularVelocity);
	stream.Serialize(MinZVel);
	stream.Serialize(MaxZVel);
	stream.Serialize(MaxXYVel);
	stream.Serialize(IsMeteor);
	stream.Serialize(Spawns);
	stream.Serialize(SpawnCount);
	stream.Serialize(StartSound);
	stream.Serialize(BounceSound);
	stream.Serialize(ExpireSound);
	stream.Serialize(BounceAnim);
	stream.Serialize(ExpireAnim);
	stream.Serialize(TrailerAnim);
	stream.Serialize(Damage);
	stream.Serialize(DamageRadius);
	stream.Serialize(Warhead);
	stream.Serialize(AttachedSystem);
	stream.Serialize(IsTiberium);
}


/// <summary>
/// Removes the specified object from this voxel animation type.
/// This routine is called when an object is about to leave the game. Every reference
/// this type holds to it must be severed so that nothing is left pointing at it.
/// </summary>
void VoxelAnimTypeClass::Detach(AbstractClass const * target, bool all)
{
	if (Warhead == target) {
		Warhead = NULL;
	}
	if (BounceAnim == target) {
		BounceAnim = NULL;
	}
	if (ExpireAnim == target) {
		ExpireAnim = NULL;
	}
	if (TrailerAnim == target) {
		TrailerAnim = NULL;
	}
	if (Spawns == target) {
		Spawns = NULL;
	}
	if (AttachedSystem == target) {
		AttachedSystem = NULL;
	}
}


/// <summary>
/// Fetches the voxel animation type of the name specified, creating it if need be.
/// This routine is used when parsing the rules and the maps, where a reference may name
/// a voxel animation that has not been declared yet.
/// </summary>
/// <returns>Returns with a pointer to the voxel animation type of that name.</returns>
VoxelAnimTypeClass * VoxelAnimTypeClass::Find_Or_Make(char const * name)
{
	return(TFind_Or_Make<VoxelAnimTypeClass>(name, VoxelAnimTypes));
}
