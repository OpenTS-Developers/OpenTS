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

#include "always.h"

#include "techtype.h"

#include "_map.h"
#include "_mixfile.h"
#include "_rules.h"
#include "animtype.h"
#include "building.h"
#include "builtype.h"
#include "bullet.h"
#include "bullettype.h"
#include "cell.h"
#include "combat.h"
#include "findmake.h"
#include "globals.h"
#include "ilocos.h"
#include "infatype.h"
#include "mixfile.h"
#include "psystype.h"
#include "rules.h"
#include "savestream.h"
#include "session.h"
#include "tracker.h"
#include "unittype.h"
#include "vanimtype.h"
#include "weapon.h"

#include "pip.hh"
#include "voc.hh"

#include <algorithm>
#include <new>


/***************************************************************************
**	These are the pointers to the special shape data that the units may need.
*/
void const * TechnoTypeClass::WakeShapes = NULL;


//**********************************************************************************************
// MODULE SEPARATION -- TechnoTypeClass member functions follow.
//**********************************************************************************************


/***********************************************************************************************
 * TechnoTypeClass::TechnoTypeClass -- Constructor for techno type objects.                    *
 *                                                                                             *
 *    This is the normal constructor for techno type objects. It is called in the process of   *
 *    constructing all the object type (constant) data for the various techno type objects.    *
 *                                                                                             *
 * INPUT:   see below...                                                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/19/1995 JLB : Created.                                                                 *
 *   05/11/1996 JLB : Moderated risk calc so range doesn't dominate.                           *
 *=============================================================================================*/
TechnoTypeClass::TechnoTypeClass(char const * ininame, SpeedType speed) :
	BASECLASS(ininame),
	IsDoubleOwned(false),
	IsInvisible(false),
	IsLeader(false),
	IsScanner(false),
	IsNominal(false),
	IsTurretEquipped(false),
	IsCrew(false),
	IsRepairable(true),
	IsRemappable(false),
	IsCloakable(false),
	IsSelfHealing(false),
	IsExploding(false),
	MZone(MZONE_NORMAL),
	ThreatRange(0),
	MaxPassengers(0),
	SightRange(0),
	Cost(0),
	Level(255),
	Prerequisite(),
	Risk(0),
	Reward(0),
	MaxSpeed(MPH_IMMOBILE),
	Speed(speed),
	MaxAmmo(-1),
	Ownable(0),
	CameoData(NULL),
	Rotation(0),
	ROT(0),
	Points(0),
	CollateralDamageCoefficient(.33f),
	WalkRate(1),
	SpecialThreatValue(0),
	MyEffectivenessCoefficient(0),
	TargetEffectivenessCoefficient(0),
	TargetSpecialThreatCoefficient(0),
	TargetStrengthCoefficient(0),
	TargetDistanceCoefficient(0),
	ThreatAvoidanceCoefficient(0),
	SlowdownDistance(500),
	DeaccelerationFactor(.002),
	AccelerationFactor(.03),
	CloakingSpeed(7),
	DebrisTypes(),
	DebrisMaximums(),
	Locomotor(CLSID_TeleportLocomotion),
	VoxelCenterY(0),
	VoxelCenterX(0),
	Weight(1),
	PhysicalSize(2),
	InitialMission(MISSION_HUNT),
	RollAngle(DEG_TO_RAD(30)),
	PitchSpeed(.25),
	PitchAngle(DEG_TO_RAD(20)),
	BuildLimit(0x7FFFFFFF),
	Category(CATEGORY_NONE),
	Unused2(0),
	DeployTime(0),
	FireAngle(8),
	PipScale(PIPSCALE_NONE),
	Dock(),
	DeploysInto(NULL),
	UndeploysInto(NULL),
	VoiceSelect(),
	VoiceMove(),
	VoiceAttack(),
	VoiceDie(),
	VoiceFeedback(),
	AuxSound1(VOC_NONE),
	AuxSound2(VOC_NONE),
	MaxDebris(0),
	FlightLevel(-1),
	IsAllowedToStartInMultiplayer(true),
	CameoFilename(""),
	TurretOffset(0),
	Explosion(),
	NaturalParticleSystem(NULL),
	NaturalParticleLocation(0,0,0),
	DamageParticleSystems(),
	DamageSmokeOffset(0,0,0),
	ShadowIndex(0),
	Capacity(0),
	TurretNotExportedOnGround(false),
	IsTypeImmune(false),
	IsDetectDisguise(false),
	IsMoveToShroud(true),
	IsTrainable(true),
	IsDamageSparks(true),
	IsTargetLaser(false),
	IsImmuneToVeins(false),
	IsTiberiumHeal(false),
	IsCloakStop(false),
	IsTrain(false),
	IsDropship(false),
	IsToProtect(false),
	IsDisableable(true),
	IsUnbuildable(false),
	IsRadarVisible(false),
	IsNoAutoFire(false),
	IsRadarEquipped(false),
	IsRegulated(false),
	IsManualReload(false),
	IsVisibleLoad(false),
	IsLightningRod(false),
	IsHunterSeeker(false),
	IsCrusher(false),
	IsTiltsWhenCrushes(true),
	IsSubterranean(false),
	IsAutoCrush(false),
	IsAccelerates(true),
	ZFudgeCliff(10),
	ZFudgeColumn(5),
	ZFudgeTunnel(10),
	ZFudgeBridge(0)
{
	IsSentient = true;

	DebrisTypes.Clear();
	DebrisMaximums.Clear();
	Dock.Clear();

	for (int i = 0; i < WEAPON_SLOT_COUNT; i++) {
		Weapons[i].Weapon = NULL;
		Weapons[i].BarrelLength = 0;
		Weapons[i].BarrelThickness = 0;
		Weapons[i].FireFLH = Point3D(0,0,0);
	}

	AbstractTypePtrTracker.Add(this);
	TechnoTypes.Add(this);
}


/// <summary>
/// Destroys the object type.
/// The type removes itself from the master type lists so that nothing can look it up
/// after it is gone.
/// </summary>
TechnoTypeClass::~TechnoTypeClass(void)
{
	AbstractTypePtrTracker.Delete(this);
	TechnoTypes.Delete(this);
}


/***********************************************************************************************
 * TechnoTypeClass::Raw_Cost -- Fetches the raw (base) cost of the object.                     *
 *                                                                                             *
 *    This routine is used to find the underlying cost for this object. The underlying cost    *
 *    does not include any free items that normally come with the object when purchased        *
 *    directly. Example: The raw cost of a refinery is the normal cost minus the cost of a     *
 *    harvester.                                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the credit cost of the base object type.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoTypeClass::Raw_Cost(void) const
{
	return(Cost);
}


/***********************************************************************************************
 * TechnoTypeClass::Get_Ownable -- Fetches the ownable bits for this object type.              *
 *                                                                                             *
 *    This routine will return the ownable bits for this object type. The ownable bits are     *
 *    a bitflag composite of the houses that can own (build) this object type.                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the ownable bits for this object type.                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoTypeClass::Get_Ownable(void) const
{
	if (IsDoubleOwned && Session.Type != GAME_NORMAL) {
		return(0x7FFFFFFF);
	}
	return(Ownable);
}


/***********************************************************************************************
 * TechnoTypeClass::Time_To_Build -- Fetches the time to build this object.                    *
 *                                                                                             *
 *    This routine will return the time it takes to construct this object. Usually the time    *
 *    to produce is directly related to cost.                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the time to produce this object type. The time is expressed in the    *
 *          form of game ticks.                                                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoTypeClass::Time_To_Build(void) const
{
	return(Cost * Rule->BuildSpeedBias * (TICKS_PER_MINUTE / 1000.));
}


/***********************************************************************************************
 * TechnoTypeClass::Cost_Of -- Fetches the cost of this object type.                           *
 *                                                                                             *
 *    This routine will return the cost to produce an object of this type.                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the cost to produce one object of this type.                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoTypeClass::Cost_Of(HouseClass * house) const
{
	if (house != NULL) {
		return(Raw_Cost() * house->CostBias);
	}
	return(Raw_Cost());
}


/***********************************************************************************************
 * TechnoTypeClass::Get_Cameo_Data -- Fetches the cameo image for this object type.            *
 *                                                                                             *
 *    This routine will fetch the cameo (sidebar small image) shape of this object type.       *
 *    If there is no cameo data available (typical for non-produceable units), then NULL will  *
 *    be returned.                                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the cameo data for this object type if present.          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void const * TechnoTypeClass::Get_Cameo_Data(void) const
{
	return(CameoData);
}


/***********************************************************************************************
 * TechnoTypeClass::Repair_Cost -- Fetches the cost to repair one step.                        *
 *                                                                                             *
 *    This routine will return the cost to repair one step. At the TechnoTypeClass level,      *
 *    this merely serves as a placeholder function. The derived classes will provide a         *
 *    functional version of this routine.                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the cost to repair one step.                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoTypeClass::Repair_Cost(void) const
{
	int cost = (Raw_Cost()/(MaxStrength/Rule->RepairStep)) * Rule->RepairPercent;
	return(std::max(cost, 1));
}


/***********************************************************************************************
 * TechnoTypeClass::Repair_Step -- Fetches the health to repair one step.                      *
 *                                                                                             *
 *    This routine merely serves as placeholder virtual function. The various type classes     *
 *    will override this routine to return the number of health points to repair in one        *
 *    "step".                                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of health points to repair in one step.                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoTypeClass::Repair_Step(void) const
{
	return(Rule->RepairStep);
}


/***********************************************************************************************
 * TechnoTypeClass::Is_Two_Shooter -- Determines if this object is a double shooter.           *
 *                                                                                             *
 *    Some objects fire two shots in quick succession. If this is true for this object, then   *
 *    a 'true' value will be returned from this routine.                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is this object a two shooter?                                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoTypeClass::Is_Two_Shooter(void) const
{
	WeaponTypeClass *pri = Get_Weapon(0)->Weapon;

	if (pri != NULL) {
		WeaponTypeClass *sec = Get_Weapon(1)->Weapon;

		if (pri == sec || pri->Burst > 1) {
				return(true);
		}

		if (sec != NULL && sec->Burst > 1) {
			return(true);
		}
	}
	return(false);
}


/***********************************************************************************************
 * _Scale_To_256 -- Scales a 1..100 number into a 1..255 number.                               *
 *                                                                                             *
 *    This is a helper routine that will take a decimal percentage number and convert it       *
 *    into a game based fixed point number.                                                    *
 *                                                                                             *
 * INPUT:   val   -- Decimal percent number to convert.                                        *
 *                                                                                             *
 * OUTPUT:  Returns with the decimal percent number converted to a game fixed point number.    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/17/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static inline int _Scale_To_256(int val)
{
	val = std::min(val, 100);
	val = std::max(val, 0);
	val = ((val * (MPH_LIGHT_SPEED + 1)) / 100);
	val = std::min<int>(val, MPH_LIGHT_SPEED);
	return(val);
}


/***********************************************************************************************
 * TechnoTypeClass::Read_INI -- Reads the techno type data from the INI database.              *
 *                                                                                             *
 *    Use this routine to fill in the data for this techno type class object from the          *
 *    database specified. Typical use of this is for the rules parsing.                        *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database that the information will be lifted from.   *
 *                                                                                             *
 * OUTPUT:  bool; Was the database used to extract information? A failure (false) response     *
 *                would mean that the database didn't contain a section that applies to this   *
 *                techno class object.                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoTypeClass::Read_INI(CCINIClass const & ini)
{
	if (BASECLASS::Read_INI(ini)) {

		IsTypeImmune = ini.Get_Bool(Name(), "TypeImmune", IsTypeImmune);
		IsDetectDisguise = ini.Get_Bool(Name(), "DetectDisguise", IsDetectDisguise);
		WalkRate = ini.Get_Int(Name(), "WalkRate", WalkRate);
		IsMoveToShroud = ini.Get_Bool(Name(), "MoveToShroud", IsMoveToShroud);
		IsTrain = ini.Get_Bool(Name(), "IsTrain", IsTrain);
		IsDoubleOwned = ini.Get_Bool(Name(), "DoubleOwned", IsDoubleOwned);
		ThreatRange = ini.Get_Lepton(Name(), "GuardRange", ThreatRange);

		if (RTTI == RTTI_INFANTRYTYPE) {
			/// This routine runs before InfantryTypeClass reads its own flags, so IsCyborg is
			/// still false on the first pass through the rules.
			if (((InfantryTypeClass *)this)->IsCyborg) {
				CollateralDamageCoefficient = 0.33f;
			} else {
				CollateralDamageCoefficient = 0.66f;
			}
		} else {
			CollateralDamageCoefficient = 1.0;
		}
		CollateralDamageCoefficient = ini.Get_Float(Name(), "CollateralDamageCoefficient", CollateralDamageCoefficient);

		if (strcmp(Name(), "GAFSDF") == 0 || strcmp(Name(), "GAWALL") == 0 || strcmp(Name(), "NAWALL") == 0) {
			ThreatRange = (CELL_LEPTON*5); /// 1280
		}

		IsExploding = ini.Get_Bool(Name(), "Explodes", IsExploding);
		if (stricmp(Name(), "E2") == 0) {
			IsExploding = true;
		}

		FlightLevel = ini.Get_Int(Name(), "FlightLevel", FlightLevel);
		IsDropship = ini.Get_Bool(Name(), "IsDropship", IsDropship);

		double pitch_angle = ini.Get_Float(Name(), "PitchAngle", -1);
		if (pitch_angle != -1) {
			PitchAngle = DEG_TO_RAD(pitch_angle);
		}
		double roll_angle = ini.Get_Float(Name(), "RollAngle", -1);
		if (roll_angle != -1) {
			RollAngle = DEG_TO_RAD(roll_angle);
		}

		PitchSpeed = ini.Get_Float(Name(), "PitchSpeed", PitchSpeed);
		Locomotor = ini.Get_CLSID(IniName, "Locomotor", Locomotor);
		CloakingSpeed = ini.Get_Int(Name(), "CloakingSpeed", CloakingSpeed);
		ThreatAvoidanceCoefficient = ini.Get_Float(Name(), "ThreatAvoidanceCoefficient", ThreatAvoidanceCoefficient);
		SlowdownDistance = ini.Get_Int(Name(), "SlowdownDistance", SlowdownDistance);
		DeaccelerationFactor = ini.Get_Float(Name(), "DeaccelerationFactor", DeaccelerationFactor);
		AccelerationFactor = ini.Get_Float(Name(), "AccelerationFactor", AccelerationFactor);
		Weight = ini.Get_Float(Name(), "Weight", Weight);
		PhysicalSize = ini.Get_Float(Name(), "PhysicalSize", PhysicalSize);
		MaxDebris = ini.Get_Int(Name(), "MaxDebris", MaxDebris);
		DebrisTypes = TGet_TypeList<VoxelAnimTypeClass>(ini, IniName, "DebrisTypes", DebrisTypes);
		DebrisMaximums = ini.Get_IntList(IniName, "DebrisMaximums", DebrisMaximums);
		Weapons[0].Weapon = TGet_Class(ini, Name(), "Primary", Weapons[0].Weapon);
		Weapons[1].Weapon = TGet_Class(ini, Name(), "Secondary", Weapons[1].Weapon);
		Weapons[2].Weapon = TGet_Class(ini, Name(), "Elite", Weapons[2].Weapon);
		VoiceMove = ini.Get_VocType_List(ini, IniName, "VoiceMove", VoiceMove);
		VoiceSelect = ini.Get_VocType_List(ini, IniName, "VoiceSelect", VoiceSelect);
		VoiceAttack = ini.Get_VocType_List(ini, IniName, "VoiceAttack", VoiceAttack);
		VoiceDie = ini.Get_VocType_List(ini, IniName, "VoiceDie", VoiceDie);
		VoiceFeedback = ini.Get_VocType_List(ini, IniName, "VoiceFeedback", VoiceFeedback);
		AuxSound1 = ini.Get_VocType(Name(), "AuxSound1", AuxSound1);
		AuxSound2 = ini.Get_VocType(Name(), "AuxSound2", AuxSound2);
		IsCloakStop = ini.Get_Bool(Name(), "CloakStop", IsCloakStop);
		Capacity = ini.Get_Int(Name(), "Storage", Capacity);
		BuildLimit = ini.Get_Int(Name(), "BuildLimit", BuildLimit);
		Category = ini.Get_CategoryType(Name(), "Category", Category);
		Dock = TGet_TypeList<BuildingTypeClass>(ini, Name(), "Dock", Dock);
		DeploysInto = TGet_Class(ini, Name(), "DeploysInto", DeploysInto);
		UndeploysInto = TGet_Class(ini, Name(), "UndeploysInto", UndeploysInto);
		IsLightningRod = ini.Get_Bool(Name(), "LightningRod", IsLightningRod);
		IsManualReload = ini.Get_Bool(Name(), "ManualReload", IsManualReload);
		IsRadarEquipped = ini.Get_Bool(Name(), "TurretSpins", IsRadarEquipped);
		IsTurretEquipped = ini.Get_Bool(Name(), "Turret", IsTurretEquipped);
		Explosion = TGet_TypeList<AnimTypeClass>(ini, Name(), "Explosion", Explosion);
		NaturalParticleSystem = TGet_Class(ini, Name(), "NaturalParticleSystem", NaturalParticleSystem);
		NaturalParticleLocation = ini.Get_Offset(Name(), "NaturalParticleLocation", NaturalParticleLocation);
		DamageParticleSystems = TGet_TypeList<ParticleSystemTypeClass>(ini, Name(), "DamageParticleSystems", DamageParticleSystems);
		DamageSmokeOffset = ini.Get_Offset(Name(), "DamageSmokeOffset", DamageSmokeOffset);
		IsNominal = ini.Get_Bool(Name(), "Nominal", IsNominal);
		IsCloakable = ini.Get_Bool(Name(), "Cloakable", IsCloakable);
		IsScanner = ini.Get_Bool(Name(), "Sensors", IsScanner);
		PipScale = ini.Get_PipScaleType(Name(), "PipScale", PipScale);
		Prerequisite = ini.Get_BuildingType_List(ini, IniName, "Prerequisite", Prerequisite);
		SightRange = ini.Get_Int(Name(), "Sight", SightRange);
		Level = ini.Get_Int(Name(), "TechLevel", Level);
		int maxspeed = ini.Get_Int(Name(), "Speed", -1);
		if (maxspeed != -1) {
			MaxSpeed = MPHType(_Scale_To_256(maxspeed));
		}
		Cost = ini.Get_Int(Name(), "Cost", Cost);
		if (strcmp(Name(), "GAFSDF") == 0 || strcmp(Name(), "GAWALL") == 0 || strcmp(Name(), "NAWALL") == 0) {
			Cost = 250;
		}
		MaxAmmo = ini.Get_Int(Name(), "Ammo", MaxAmmo);
		Reward = Points = ini.Get_Int(Name(), "Points", Points);
		Risk = ini.Get_Int(Name(), "ThreatPosed", Risk);
		Ownable = ini.Get_Owners(Name(), "Owner", Ownable);
		IsTrainable = ini.Get_Bool(Name(), "Trainable", IsTrainable);
		IsCrew = ini.Get_Bool(Name(), "Crewed", IsCrew);
		IsRepairable = ini.Get_Bool(Name(), "Repairable", IsRepairable);
		IsInvisible = ini.Get_Bool(Name(), "Invisible", IsInvisible);
		IsRadarVisible = ini.Get_Bool(Name(), "RadarVisible", IsRadarVisible);
		IsSelfHealing = ini.Get_Bool(Name(), "SelfHealing", IsSelfHealing);
		IsNoAutoFire = ini.Get_Bool(Name(), "NoAutoFire", IsNoAutoFire);
		ROT = ini.Get_Int(Name(), "ROT", ROT);
		MaxPassengers = ini.Get_Int(Name(), "Passengers", MaxPassengers);
		FireAngle = ini.Get_Int(Name(), "FireAngle", FireAngle);
		DeployTime = ini.Get_Float(Name(), "DeployTime", DeployTime);
		IsDisableable = ini.Get_Bool(Name(), "Disableable", IsDisableable);
		IsToProtect = ini.Get_Bool(Name(), "ToProtect", IsToProtect);
		IsTiberiumHeal = ini.Get_Bool(Name(), "TiberiumHeal", IsTiberiumHeal);
		IsImmuneToVeins = ini.Get_Bool(Name(), "ImmuneToVeins", IsImmuneToVeins);
		IsAllowedToStartInMultiplayer = ini.Get_Bool(Name(), "AllowedToStartInMultiplayer", IsAllowedToStartInMultiplayer);
		IsTargetLaser = ini.Get_Bool(Name(), "TargetLaser", IsTargetLaser);
		IsHunterSeeker = ini.Get_Bool(Name(), "HunterSeeker", IsHunterSeeker);
		IsCrusher = ini.Get_Bool(Name(), "Crusher", IsCrusher);
		IsAutoCrush = ini.Get_Bool(Name(), "AutoCrush", IsAutoCrush);
		IsTiltsWhenCrushes = ini.Get_Bool(Name(), "TiltsWhenCrushes", IsTiltsWhenCrushes);
		IsAccelerates = ini.Get_Bool(Name(), "Accelerates", IsAccelerates);
		ZFudgeCliff = ini.Get_Int(Name(), "ZFudgeCliff", ZFudgeCliff);
		ZFudgeColumn = ini.Get_Int(Name(), "ZFudgeColumn", ZFudgeColumn);
		ZFudgeTunnel = ini.Get_Int(Name(), "ZFudgeTunnel", ZFudgeTunnel);
		ZFudgeBridge = ini.Get_Int(Name(), "ZFudgeBridge", ZFudgeBridge);
		VeteranAbilities = ini.Get_Abilities(Name(), "VeteranAbilities", VeteranAbilities);
		EliteAbilities = ini.Get_Abilities(Name(), "EliteAbilities", EliteAbilities);
		MyEffectivenessCoefficient = ini.Get_Float(Name(), "MyEffectivenessCoefficient", MyEffectivenessCoefficient == 0 ? Rule->MyEffectivenessCoefficientDefault : MyEffectivenessCoefficient);
		TargetEffectivenessCoefficient = ini.Get_Float(Name(), "TargetEffectivenessCoefficient", TargetEffectivenessCoefficient == 0 ? Rule->TargetEffectivenessCoefficientDefault : TargetEffectivenessCoefficient);
		TargetSpecialThreatCoefficient = ini.Get_Float(Name(), "TargetSpecialThreatCoefficient", TargetSpecialThreatCoefficient == 0 ? Rule->TargetSpecialThreatCoefficientDefault : TargetSpecialThreatCoefficient);
		TargetStrengthCoefficient = ini.Get_Float(Name(), "TargetStrengthCoefficient", TargetStrengthCoefficient == 0 ? Rule->TargetStrengthCoefficientDefault : TargetStrengthCoefficient);
		TargetDistanceCoefficient = ini.Get_Float(Name(), "TargetDistanceCoefficient", TargetDistanceCoefficient == 0 ? Rule->TargetDistanceCoefficientDefault : TargetDistanceCoefficient);
		SpecialThreatValue = ini.Get_Float(Name(), "SpecialThreatValue", SpecialThreatValue);

		IsLeader = false;
		if (Weapons[0].Weapon != NULL && Weapons[0].Weapon->Attack > 0) {
			IsLeader = true;
		}

		char filename[512];
		_makepath(filename, NULL, NULL, Graphic_Name(), ".SHP");
		ImageData = MFCD::Retrieve(filename);

		FireAngle = ArtINI.Get_Angle(Graphic_Name(), "FireAngle", FireAngle);
		TurretOffset = ArtINI.Get_Int(Graphic_Name(), "TurretOffset", TurretOffset);
		Rotation = ArtINI.Get_Int(Graphic_Name(), "RotCount", Rotation);
		IsRemappable = ArtINI.Get_Bool(Graphic_Name(), "Remapable", IsRemappable);
		IsRegulated = ArtINI.Get_Bool(Graphic_Name(), "Normalized", IsRegulated);
		IsVisibleLoad = ArtINI.Get_Bool(Graphic_Name(), "VisibleLoad", IsVisibleLoad);
		ShadowIndex = ArtINI.Get_Int(Graphic_Name(), "ShadowIndex", ShadowIndex);

		TStringID<24> cameo;
		if (ArtINI.Get_String(Graphic_Name(), "Cameo", "", cameo) > 0) {
			CameoFilename = cameo;
			_makepath(filename, NULL, NULL, CameoFilename, ".SHP");
			CameoData = (ShapeSet const *)MFCD::Retrieve(filename);
		}
		if (CameoData == NULL) {
			CameoData = (ShapeSet const *)MFCD::Retrieve("XXICON.SHP");
		}

		Weapons[0].FireFLH = ArtINI.Get_Point(Graphic_Name(), "PrimaryFireFLH", Weapons[0].FireFLH);
		Weapons[0].BarrelLength = ArtINI.Get_Int(Graphic_Name(), "PBarrelLength", Weapons[0].BarrelLength);
		Weapons[0].BarrelThickness = ArtINI.Get_Int(Graphic_Name(), "PBarrelThickness", Weapons[0].BarrelThickness);
		Weapons[1].FireFLH = ArtINI.Get_Point(Graphic_Name(), "SecondaryFireFLH", Weapons[1].FireFLH);
		Weapons[1].BarrelLength = ArtINI.Get_Int(Graphic_Name(), "SBarrelLength", Weapons[1].BarrelLength);
		Weapons[1].BarrelThickness = ArtINI.Get_Int(Graphic_Name(), "SBarrelThickness", Weapons[1].BarrelThickness);
		Weapons[2].FireFLH = ArtINI.Get_Point(Graphic_Name(), "PrimaryFireFLH", Weapons[2].FireFLH);
		Weapons[2].BarrelLength = ArtINI.Get_Int(Graphic_Name(), "PBarrelLength", Weapons[2].BarrelLength);
		Weapons[2].BarrelThickness = ArtINI.Get_Int(Graphic_Name(), "PBarrelThickness", Weapons[2].BarrelThickness);

		TurretNotExportedOnGround = ArtINI.Get_Bool(Graphic_Name(), "TurretNotExportedOnGround", TurretNotExportedOnGround);

		/*
		**	Check to see what zone this object should recognize.
		*/
		MZone = ini.Get_MZoneType(Name(), "MovementZone", MZone);
		IsSubterranean = MZone == MZONE_SUBTERANNEAN;

		if (IsVoxel) {
			Fetch_Voxel_Image();
		} else if (IsTurretEquipped) {
			Fetch_Aux_Voxel_Image();
		}

		if (Voxel.VoxLib != NULL && !Voxel.VoxLib->Load_Failed()) {
			VoxelCenterY = Voxel.VoxLib->Get_Layer_Info(0, 0).YSize * 0.5f + 0.5;
			VoxelCenterX = Voxel.VoxLib->Get_Layer_Info(0, 0).XSize * 0.5f;
		}

		return(true);
	}
	return(false);
}


/// <summary>
/// Determines if this object type may legally be placed at the specified location.
/// Every square of the object's foundation is examined for obstacles -- buildings are
/// weighed against the house's own build restrictions, everything else merely has to fit.
/// A building that paves what it covers is more forgiving, since it only needs one square
/// of the foundation to be free.
/// </summary>
/// <param name="pos">The cell to anchor the object's foundation at.</param>
/// <param name="house">The house that wishes to do the placing.</param>
/// <returns>bool; May the object be placed at this location?</returns>
bool TechnoTypeClass::Legal_Placement(Cell const & pos, HouseClass * house) const
{
	if (pos == CELL_NONE) return(0);

	/*
	**	Normal buildings must check to see that every foundation square is free of
	**	obstacles. If this check passes for all foundation squares, only then does the
	**	routine return that it is legal to place.
	*/
	Cell const * offset = Occupy_List(true);
	bool build = (RTTI == RTTI_BUILDINGTYPE);
	bool any_blocked = false;
	bool any_clear = false;

	BuildingTypeClass * bptr = (BuildingTypeClass *)this;

	while (offset != NULL && *offset != REFRESH_EOL) {
		Cell cell = pos + *offset++;

		if (build) {
			if (!Map[cell].Is_Clear_To_Build(bptr->Speed, bptr, house)) {
				any_blocked = true;
			} else {
				any_clear = true;
			}
		} else {
			if (!Map[cell].Is_Clear_To_Move(Speed, false, false)) {
				any_blocked = true;
			} else {
				any_clear = true;
			}
		}
	}

	if (build && bptr->ToTile != NULL) {
		return(any_clear);
	}
	return(any_blocked == 0);
}


/// <summary>
/// Fetches the number of pips this object type displays.
/// The pip scale assigned in the rules decides both what the pips stand for -- ammo,
/// passengers, tiberium, power or charge -- and how many of them there can be.
/// </summary>
/// <returns>Returns with the maximum pip count, or zero if this object type shows none.</returns>
int TechnoTypeClass::Max_Pips(void) const
{
	switch (PipScale) {
		case PIPSCALE_POWER:
			return(10);

		case PIPSCALE_AMMO:
			return(std::min(MaxAmmo, 5));

		case PIPSCALE_TIBERIUM:
			return(5);

		case PIPSCALE_PASSENGERS:
			return(std::min(MaxPassengers, 5));

		case PIPSCALE_CHARGE:
			return(8);
	}
	return(0);
}


/***********************************************************************************************
 * TechnoClass::In_Range -- Determines if specified target is within weapon range.             *
 *                                                                                             *
 *    This routine is used to compare the distance to the specified target with the range      *
 *    of the weapon. If the target is outside of weapon range, then false is returned.         *
 *                                                                                             *
 * INPUT:   target   -- The target to check if it is within weapon range.                      *
 *                                                                                             *
 *          which    -- Which weapon to use in determining range. 0=primary, 1=secondary.      *
 *                                                                                             *
 * OUTPUT:  bool; Is the specified target within weapon range?                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/14/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoTypeClass::In_Range(Coord const & coord, AbstractClass * target, WeaponTypeClass * weapon) const
{
//	//assert(IsActive);

	bool is_within_ground_firing_angle;

	if (target != NULL && weapon != NULL) {
		int range = weapon->Range - CELL_LEPTON / 3;
		Coord tcoord = target->Center_Coord();
		int minrange = weapon->MinimumRange;

		if (minrange && (coord - tcoord).Length() < minrange) {
			return(false);
		}

		if (weapon->Bullet->IsArcing) {
			int dist = (Point2D(coord) - Point2D(tcoord)).Length();
			int height = tcoord.Z - coord.Z;
			double gravity = weapon->Bullet->IsFloater ? Get_Floater_Gravity() : Rule->Gravity;
			if (!Is_Projectile_Trajectory_Valid(weapon->MaxSpeed, dist, height, gravity) || (Map[tcoord].IsUnderBridge && tcoord.Z - coord.Z >= 3 * LEVEL_LEPTON_H)) {
				return(false);
			}
			Coord dist2d = Coord(coord.X - tcoord.X, coord.Y - tcoord.Y, 0);
			is_within_ground_firing_angle = coord.Z - tcoord.Z < dist2d.Length();
		} else {

			BuildingClass const * building = target->As_BuildingClass();
			if (building != NULL) {
				range += ((building->Class->Width() + building->Class->Height()) * (CELL_LEPTON_W / 4));
			}

			int cdist;
			if (RTTI != RTTI_AIRCRAFTTYPE) {
				cdist = (coord - tcoord).Length();
			} else {
				cdist = (Coord(Point2D(coord) - Point2D(tcoord), 0)).Length();
			}

			if (cdist > range) {
				return(false);
			}

			if (Map[coord].IsUnderBridge) {
				int height = BRIDGE_LEPTON_HEIGHT + Map.Get_Height_GL(coord);
				if (coord.Z < height && tcoord.Z >= height) {
					return(false);
				}
			}

			/*
			 * A ground weapon cannot hit a target that is too high above it -- the
			 * target must be no higher than the horizontal distance to it.
			 */
			Coord coord1 = coord;
			Coord coord2 = tcoord;
			if (!weapon->Bullet->IsAntiAircraft) {
				Point2D dist2d = Point2D(coord1 - coord2);
				if (tcoord.Z - coord.Z >= dist2d.Length()) {
					return(false);
				}
			}

			Point2D dist2d = Point2D(coord1 - coord2);
			is_within_ground_firing_angle = coord.Z - tcoord.Z < dist2d.Length();
		}

		if (!is_within_ground_firing_angle) {
			int hoffset = Map[coord].IsUnderBridge ? BRIDGE_LEPTON_HEIGHT : 0;
			if (coord.Z <= hoffset + Map.Get_Height_GL(coord)) {
				return(false);
			}
		}
		return(true);
	}

	return(false);
}


/// <summary>
/// Re-attaches the artwork this techno type names.
/// Artwork is never written to a save game, so the shape and the sidebar cameo are
/// re-fetched from the mix files once the members have been read.
/// </summary>
void TechnoTypeClass::Post_Load(void)
{
	BASECLASS::Post_Load();

		char buffer[256];
		char fname[_MAX_PATH];

		_makepath(fname, NULL, NULL, (const char *)GraphicName, ".SHP");
		ImageData = MFCD::Retrieve(fname);

		ArtINI.Get_String((const char *)GraphicName, "Cameo", "XXICON", buffer, sizeof(buffer));
		if (stricmp(buffer, "XXICON") == 0) {
			ArtINI.Get_String((const char *)IniName, "Cameo", "XXICON", buffer, sizeof(buffer));
		}
		_makepath(fname, NULL, NULL, buffer, ".SHP");
		CameoData = (const ShapeSet *)MFCD::Retrieve(fname);
}


/// <summary>
/// Lists the members every techno type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TechnoTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(CollateralDamageCoefficient);
	stream.Serialize(Unused1);
	stream.Serialize(WalkRate);
	stream.Serialize(VeteranAbilities);
	stream.Serialize(EliteAbilities);
	stream.Serialize(SpecialThreatValue);
	stream.Serialize(MyEffectivenessCoefficient);
	stream.Serialize(TargetEffectivenessCoefficient);
	stream.Serialize(TargetSpecialThreatCoefficient);
	stream.Serialize(TargetStrengthCoefficient);
	stream.Serialize(TargetDistanceCoefficient);
	stream.Serialize(ThreatAvoidanceCoefficient);
	stream.Serialize(SlowdownDistance);
	stream.Serialize(DeaccelerationFactor);
	stream.Serialize(AccelerationFactor);
	stream.Serialize(CloakingSpeed);
	stream.Serialize(DebrisTypes);
	stream.Serialize(DebrisMaximums);

	/*
	 * A class identifier is a plain sixteen byte value from the Windows SDK with no member
	 * of its own to describe, so it travels as its raw image.
	 */
	stream.Serialize_Bytes(&Locomotor, sizeof(Locomotor));

	stream.Serialize(VoxelCenterY);
	stream.Serialize(VoxelCenterX);
	stream.Serialize(Weight);
	stream.Serialize(PhysicalSize);
	stream.Serialize(InitialMission);
	stream.Serialize(RollAngle);
	stream.Serialize(PitchSpeed);
	stream.Serialize(PitchAngle);
	stream.Serialize(BuildLimit);
	stream.Serialize(Category);
	stream.Serialize(Unused2);
	stream.Serialize(DeployTime);
	stream.Serialize(FireAngle);
	stream.Serialize(PipScale);
	stream.Serialize(Dock);
	stream.Serialize(DeploysInto);
	stream.Serialize(UndeploysInto);
	stream.Serialize(VoiceSelect);
	stream.Serialize(VoiceMove);
	stream.Serialize(VoiceAttack);
	stream.Serialize(VoiceDie);
	stream.Serialize(VoiceFeedback);
	stream.Serialize(AuxSound1);
	stream.Serialize(AuxSound2);
	stream.Serialize(MZone);
	stream.Serialize(ThreatRange);
	stream.Serialize(MaxDebris);
	stream.Serialize(MaxPassengers);
	stream.Serialize(SightRange);
	stream.Serialize(Cost);
	stream.Serialize(FlightLevel);
	stream.Serialize(Level);
	stream.Serialize(Prerequisite);
	stream.Serialize(Risk);
	stream.Serialize(Reward);
	stream.Serialize(MaxSpeed);
	stream.Serialize(Speed);
	stream.Serialize(MaxAmmo);
	stream.Serialize(Ownable);
	stream.Serialize(IsAllowedToStartInMultiplayer);
	stream.Serialize(CameoFilename);
	// CameoData -- artwork, fetched from the mix files again as this loads.
	stream.Serialize(Rotation);
	stream.Serialize(ROT);
	stream.Serialize(TurretOffset);
	stream.Serialize(Points);
	stream.Serialize(Explosion);
	stream.Serialize(NaturalParticleSystem);
	stream.Serialize(NaturalParticleLocation);
	stream.Serialize(DamageParticleSystems);
	stream.Serialize(DamageSmokeOffset);
	stream.Serialize(ShadowIndex);
	stream.Serialize(Capacity);
	stream.Serialize(TurretNotExportedOnGround);
	stream.Serialize(Weapons);
	stream.Serialize(IsTypeImmune);
	stream.Serialize(IsDetectDisguise);
	stream.Serialize(IsMoveToShroud);
	stream.Serialize(IsTrainable);
	stream.Serialize(IsDamageSparks);
	stream.Serialize(IsTargetLaser);
	stream.Serialize(IsImmuneToVeins);
	stream.Serialize(IsTiberiumHeal);
	stream.Serialize(IsCloakStop);
	stream.Serialize(IsTrain);
	stream.Serialize(IsDropship);
	stream.Serialize(IsToProtect);
	stream.Serialize(IsDisableable);
	stream.Serialize(IsUnbuildable);
	stream.Serialize(IsDoubleOwned);
	stream.Serialize(IsInvisible);
	stream.Serialize(IsRadarVisible);
	stream.Serialize(IsLeader);
	stream.Serialize(IsScanner);
	stream.Serialize(IsNominal);
	stream.Serialize(IsTurretEquipped);
	stream.Serialize(IsRepairable);
	stream.Serialize(IsCrew);
	stream.Serialize(IsRemappable);
	stream.Serialize(IsCloakable);
	stream.Serialize(IsSelfHealing);
	stream.Serialize(IsExploding);
	stream.Serialize(IsNoAutoFire);
	stream.Serialize(IsRadarEquipped);
	stream.Serialize(IsRegulated);
	stream.Serialize(IsManualReload);
	stream.Serialize(IsVisibleLoad);
	stream.Serialize(IsLightningRod);
	stream.Serialize(IsHunterSeeker);
	stream.Serialize(IsCrusher);
	stream.Serialize(IsTiltsWhenCrushes);
	stream.Serialize(IsSubterranean);
	stream.Serialize(IsAutoCrush);
	stream.Serialize(IsAccelerates);
	stream.Serialize(ZFudgeCliff);
	stream.Serialize(ZFudgeColumn);
	stream.Serialize(ZFudgeTunnel);
	stream.Serialize(ZFudgeBridge);
}


/// <summary>
/// Adds this object type's rule data to a running checksum.
/// This routine is used by the multiplayer sync check to prove that every machine in the
/// game is playing by the same rules.
/// </summary>
void TechnoTypeClass::Compute_CRC(class CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(ThreatAvoidanceCoefficient);
	crc(SlowdownDistance);
	crc(DeaccelerationFactor);
	crc(AccelerationFactor);
	crc(CloakingSpeed);
	crc(DebrisTypes.Count());
	crc(Dock.Count());
	crc(DebrisMaximums.Count());
	crc((char*)&Locomotor, sizeof(Locomotor));
	crc(VoxelCenterY);
	crc(VoxelCenterX);
	crc(Weight);
	crc(PhysicalSize);
	crc(InitialMission);
	crc(IsImmuneToVeins);
	crc(IsTiberiumHeal);
	crc(IsTargetLaser);
	crc(RollAngle);
	crc(PitchAngle);
	crc(PitchSpeed);
	crc(FlightLevel);
	crc(BuildLimit);
	crc(Category);
	crc(DeployTime);
	crc(FireAngle);
	crc(PipScale);
	crc(VoiceSelect.Count());
	crc(VoiceMove.Count());
	crc(VoiceAttack.Count());
	crc(VoiceDie.Count());
	crc(VoiceFeedback.Count());
	crc(DamageParticleSystems.Count());
	crc(MZone);
	crc(ThreatRange);
	crc(MaxDebris);
	crc(MaxPassengers);
	crc(SightRange);
	crc(Cost);
	crc(Level);
	crc(Prerequisite.Count());
	crc(Risk);
	crc(Reward);
	crc(MaxSpeed);
	crc(Speed);
	crc(MaxAmmo);
	crc((int)Ownable);
	crc(Rotation);
	crc(ROT);
	crc(TurretOffset);
	crc(Points);
	crc(Explosion.Count());
	crc(ShadowIndex);
	crc(Capacity);
	crc(IsTrain);
	crc(IsDetectDisguise);
	crc(IsDropship);
	crc(IsToProtect);
	crc(IsDisableable);
	crc(IsUnbuildable);
	crc(IsDoubleOwned);
	crc(IsInvisible);
	crc(IsRadarVisible);
	crc(IsLeader);
	crc(IsScanner);
	crc(IsNominal);
	crc(IsTurretEquipped);
	crc(IsRepairable);
	crc(IsCrew);
	crc(IsRemappable);
	crc(IsCloakable);
	crc(IsSelfHealing);
	crc(IsExploding);
	crc(IsNoAutoFire);
	crc(IsRadarEquipped);
	crc(IsRegulated);
	crc(IsManualReload);
	crc(IsVisibleLoad);
	crc(IsLightningRod);
}


/// <summary>
/// Fetches the weapon data for one of this object type's weapon slots.
/// An object type that was never given an elite weapon quietly serves up its primary
/// instead, so a veteran object may ask for its elite armament without checking first.
/// </summary>
/// <param name="which">The weapon slot desired.</param>
/// <returns>Returns with a pointer to the weapon data for that slot.</returns>
WeaponDataStruct const * TechnoTypeClass::Get_Weapon(int which) const
{
	if (which == 2 && Weapons[which].Weapon == NULL) {
		which = 0;
	}
	return(&Weapons[which]);
}


/// <summary>
/// Sets one of this object type's weapon slots.
/// </summary>
/// <param name="which">The weapon slot to fill in.</param>
void TechnoTypeClass::Set_Weapon(WeaponDataStruct const & weapon, int which)
{
	Weapons[which] = weapon;
}


/// <summary>
/// Fetches the altitude that this object type travels at.
/// An object type that does not name a flight level of its own falls back on the global
/// rule, so most flying objects share one cruising height.
/// </summary>
/// <returns>Returns with the height above ground that this object type should fly at.</returns>
int TechnoTypeClass::Flight_Level(void) const
{
	if (FlightLevel == -1) {
		return(Rule->FlightLevel);
	}
	return(FlightLevel);
}
