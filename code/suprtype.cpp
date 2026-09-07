/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "suprtype.h"

#include "_map.h"
#include "_mixfile.h"
#include "building.h"
#include "builtype.h"
#include "cell.h"
#include "findmake.h"
#include "globals.h"
#include "mixfile.h"
#include "mouse.h"
#include "savestream.h"
#include "sun.h"
#include "swizzle.h"
#include "weapon.h"


/// <summary>
/// Converts a super weapon behavior name into its type number.
/// The behavior name says which hard coded effect a super weapon delivers -- the ion
/// cannon, the multi missile, and so on -- rather than which rules section declared it.
/// </summary>
/// <returns>Returns with the super weapon type named. Otherwise, SUPER_NONE is
/// returned.</returns>
SuperWeaponType Special_From_Name(char const * name)
{
	static const char * _names[SUPER_COUNT] = {
		"MultiMissile",
		"EMPulse",
		"Firestorm",
		"IonCannon",
		"HunterSeeker",
		"ChemMissile",
		"DropPod"
	};

	for (int i = SUPER_FIRST; i < SUPER_COUNT; i++) {
		if (strcmpi(_names[i], name) == 0) {
			return(SuperWeaponType)(i);
		}
	}
	return(SUPER_NONE);
}


/// <summary>
/// Constructs a super weapon type of the rules name specified.
/// The new type is given a heap identifier and added to the super weapon type heap. Only
/// harmless defaults are installed here; Read_INI supplies the real characteristics.
/// </summary>
/// <param name="ininame">The rules section this super weapon is declared by.</param>
SuperWeaponTypeClass::SuperWeaponTypeClass(char const * ininame) :
	BASECLASS(ininame),
	VoxRecharge(VOX_NONE),
	VoxCharging(VOX_NONE),
	VoxImpatient(VOX_NONE),
	VoxSuspend(VOX_NONE),
	Type(SUPER_NONE),
	Weapon(NULL),
	RechargeTime(4500),
	CameoData(NULL),
	CameoSortOrder(0),
	Action(ACTION_NONE),
	AuxBuilding(NULL),
	SidebarImage(),
	UseChargeDrain(false),
	IsPowered(true),
	IsManualControl(false)
{
	Create_ID();
	HeapID = (SuperWeaponType)SuperWeaponTypes.Count();
	SuperWeaponTypes.Add(this);
	SidebarImage = IniName;
}


/// <summary>
/// Removes this super weapon type from the game.
/// The type is unlinked from the super weapon type heap so that nothing can reach it
/// afterwards.
/// </summary>
SuperWeaponTypeClass::~SuperWeaponTypeClass(void)
{
	SuperWeaponTypes.Delete(this);
}


ClassID SuperWeaponTypeClass::Class_ID(void) const
{
	return(ClassID_SuperWeaponTypeClass);
}


/// <summary>
/// Re-attaches the artwork this super weapon type names.
/// The sidebar cameo is fetched from the mix files again once the members have been
/// read, since artwork is never written to a save game.
/// </summary>
void SuperWeaponTypeClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	char fullname[_MAX_FNAME+_MAX_EXT];
	_makepath(fullname, NULL, NULL, SidebarImage, ".SHP");
	CameoData = (ShapeSet *)MFCD::Retrieve(fullname);
	if (CameoData == NULL) {
		CameoData = (ShapeSet *)MFCD::Retrieve("XXICON.SHP");
	}
}


/// <summary>
/// Lists the members this super weapon type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void SuperWeaponTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HeapID);
	stream.Serialize(Weapon);
	stream.Serialize(VoxRecharge);
	stream.Serialize(VoxCharging);
	stream.Serialize(VoxImpatient);
	stream.Serialize(VoxSuspend);
	stream.Serialize(RechargeTime);
	stream.Serialize(Type);
	// CameoData -- artwork, fetched from the mix files again as this loads.
	stream.Serialize(CameoSortOrder);
	stream.Serialize(Action);
	stream.Serialize(AuxBuilding);
	stream.Serialize(SidebarImage);
	stream.Serialize(UseChargeDrain);
	stream.Serialize(IsPowered);
	stream.Serialize(IsManualControl);
}


/// <summary>
/// Fetches the run time type of this object.
/// </summary>
/// <returns>Returns with RTTI_SUPERWEAPONTYPE.</returns>
RTTIType SuperWeaponTypeClass::Fetch_RTTI(void) const
{
	return(RTTI_SUPERWEAPONTYPE);
}


/// <summary>
/// Submits this super weapon type to the game state checksum.
/// This routine is used by the multiplayer sync check to prove that every machine is
/// playing with the same rules.
/// </summary>
void SuperWeaponTypeClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(HeapID);
	crc(Action);
	crc(VoxRecharge);
	crc(VoxCharging);
	crc(VoxImpatient);
	crc(VoxSuspend);
	crc(IsPowered);
	crc(RechargeTime);
	crc(Type);
	crc(UseChargeDrain);
	crc(IsManualControl);
}


/// <summary>
/// Fetches the heap identifier of this super weapon type.
/// </summary>
/// <returns>Returns with the index of this object within the super weapon type heap.</returns>
int SuperWeaponTypeClass::Fetch_Heap_ID(void) const
{
	return(HeapID);
}


/// <summary>
/// Fetches this super weapon's characteristics from the rules.
/// This routine picks up the weapon, the announcement voices, the recharge delay, and
/// the sidebar cameo artwork. A missing cameo falls back on the generic icon so that the
/// sidebar always has something to draw.
/// </summary>
/// <returns>bool; Was the database entry found and read?</returns>
bool SuperWeaponTypeClass::Read_INI(CCINIClass const & ini)
{
	if (BASECLASS::Read_INI(ini)) {

		Weapon = TGet_Class(ini, IniName, "WeaponType", Weapon);

		VoxRecharge = ini.Get_VoxType(IniName, "RechargeVoice", VoxRecharge);
		VoxCharging = ini.Get_VoxType(IniName, "ChargingVoice", VoxCharging);
		VoxImpatient = ini.Get_VoxType(IniName, "ImpatientVoice", VoxImpatient);
		VoxSuspend = ini.Get_VoxType(IniName, "SuspendVoice", VoxSuspend);

		Action = ini.Get_ActionType(IniName, "Action", Action);
		IsPowered = ini.Get_Bool(IniName, "IsPowered", IsPowered);

		char buffer[40];
		ini.Get_String(IniName, "Type", "", buffer, sizeof(buffer));
		if (strlen(buffer) != 0) {
			SuperWeaponType type = Special_From_Name(buffer);
			if (type != SUPER_NONE) Type = type;
		}

		AuxBuilding = TGet_Class(ini, IniName, "AuxBuilding", AuxBuilding);
		UseChargeDrain = ini.Get_Bool(IniName, "UseChargeDrain", UseChargeDrain);
		IsManualControl = ini.Get_Bool(IniName, "ManualControl", IsManualControl);
		CameoSortOrder = ini.Get_Int(IniName, "CameoSortOrder", CameoSortOrder);

		float recharge = ini.Get_Float(IniName, "RechargeTime");
		if (recharge != 0.0) {
			RechargeTime = recharge * TICKS_PER_MINUTE;
		}

		ini.Get_String(IniName, "SidebarImage", SidebarImage);

		char fullname[_MAX_FNAME+_MAX_EXT];
		_makepath(fullname, NULL, NULL, SidebarImage, ".SHP");
		CameoData = (ShapeSet *)MFCD::Retrieve(fullname);
		if (CameoData == NULL) {
			CameoData = (ShapeSet *)MFCD::Retrieve("XXICON.SHP");
		}

		return(true);
	}
	return(false);
}


/// <summary>
/// Converts a rules name into a super weapon type number.
/// </summary>
/// <returns>Returns with the super weapon type that bears the name. Otherwise, SUPER_NONE
/// is returned.</returns>
SuperWeaponType SuperWeaponTypeClass::From_Name(char const * name)
{
	if (name != NULL) {
		for (int classid = SUPER_FIRST; classid < SuperWeaponTypes.Count(); classid++) {
			if (stricmp(SuperWeaponTypes[classid]->IniName, name) == 0) {
				return(SuperWeaponType)classid;
			}
		}
	}
	return(SUPER_NONE);
}


/// <summary>
/// Fetches the super weapon type that performs the action specified.
/// This routine is used to work back from a mouse action to the super weapon that
/// offers it.
/// </summary>
/// <returns>Returns with a pointer to the matching super weapon type. Otherwise, NULL is
/// returned.</returns>
SuperWeaponTypeClass * SuperWeaponTypeClass::From_Action(ActionType action)
{
	for (int classid = SUPER_FIRST; classid < SuperWeaponTypes.Count(); classid++) {
		if (SuperWeaponTypes[classid]->Action == action) {
			return(SuperWeaponTypes[classid]);
		}
	}
	return(NULL);
}


/// <summary>
/// Fetches the super weapon type of the name specified, creating it if need be.
/// This routine is used while the rules are being parsed so that a super weapon may be
/// referred to before its own section has been reached.
/// </summary>
/// <returns>Returns with a pointer to the super weapon type object.</returns>
SuperWeaponTypeClass * SuperWeaponTypeClass::Find_Or_Make(const char *name)
{
	return(TFind_Or_Make<SuperWeaponTypeClass>(name, SuperWeaponTypes));
}


/// <summary>
/// Determines the action to perform when this super weapon is aimed at a target.
/// Most super weapons simply report the action their rules entry names. The EM pulse is
/// the exception, since it must be delivered by a powered pulse cannon -- this routine
/// tells the cursor logic whether the player has one that can reach the target.
/// </summary>
/// <param name="object">The object being targeted, or NULL if the cell itself is the
/// target.</param>
/// <returns>Returns with the action to perform, which the caller turns into a mouse
/// shape.</returns>
ActionType SuperWeaponTypeClass::What_Action(Cell const & cell, ObjectClass * object)
{
	if (Type != SUPER_EM_PULSE) {
		return(Action);
	}

	AbstractClass * target;
	Cell target_cell;

	if (object != NULL) {
		target = object;
		target_cell = object->PositionCell;
	} else {
		target = &Map[cell];
		target_cell = cell;
	}

	BuildingClass * cannon = NULL;
	int mindist = INT_MAX;

	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass * building = Buildings[i];
		if (building->Class->IsEMPulseCannon && building->House == PlayerPtr && building->Is_Powered_On()) {
			if (building->Distance(target) < mindist) {
				mindist = building->Distance(target);
				cannon = building;
			}
		}
	}

	if (cannon == NULL) {
		return(ACTION_EMPULSE_RANGE);
	}

	WeaponTypeClass *weap = cannon->Get_Class_Weapon_Data()->Weapon;
	int range = weap->Range / CELL_LEPTON;
	Cell cannon_cell = cannon->PositionCell;
	Cell dist(target_cell - cannon_cell);
	if (dist.X * dist.X + dist.Y * dist.Y < range * range) {
		return(ACTION_EMPULSE);
	}

	return(ACTION_EMPULSE_RANGE);
}
