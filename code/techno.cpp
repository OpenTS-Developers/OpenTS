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

/* $Header: /CounterStrike/TECHNO.CPP 5     3/17/97 1:28a Steve_tall $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : BASE.CPP                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 8, 1994                                                  *
 *                                                                                             *
 *                  Last Update : November 1, 1996 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   TechnoClass::AI -- Handles AI processing for techno object.                               *
 *   TechnoClass::Anti_Air -- Determines the anti-aircraft strength of the object.             *
 *   TechnoClass::Anti_Armor -- Determines the anti-armor strength of the object.              *
 *   TechnoClass::Anti_Infantry -- Calculates the anti-infantry strength of this object.       *
 *   TechnoClass::Area_Modify -- Determine the area scan modifier for the cell.                *
 *   TechnoClass::Assign_Destination -- Assigns movement destination to the object.            *
 *   TechnoClass::Assign_Target -- Assigns the targeting computer with specified target.       *
 *   TechnoClass::Base_Is_Attacked -- Handle panic response to base being attacked.            *
 *   TechnoClass::Can_Fire -- Determines if this techno object can fire.                       *
 *   TechnoClass::Can_Player_Fire -- Determines if the player can give this object a fire order*
 *   TechnoClass::Can_Player_Move -- Determines if the object can move be moved by player.     *
 *   TechnoClass::Can_Repair -- Determines if the object can and should be repaired.           *
 *   TechnoClass::Can_Teleport_Here -- Checks cell to see if a valid teleport destination.     *
 *   TechnoClass::Captured -- Handles capturing this object.                                   *
 *   TechnoClass::Clicked_As_Target -- Sets the flash count for this techno object.            *
 *   TechnoClass::Cloaking_AI -- Perform the AI maintenance for a cloaking device.             *
 *   TechnoClass::Combat_Damage -- Fetch the amount of damage infliced by the specified weapon.*
 *   TechnoClass::Crew_Type -- Fetches the kind of crew this object contains.                  *
 *   TechnoClass::Debug_Dump -- Displays the base class data to the monochrome screen.         *
 *   TechnoClass::Desired_Load_Dir -- Fetches loading parameters for this object.              *
 *   TechnoClass::Detach -- Handles removal of target from tracking system.                    *
 *   TechnoClass::Do_Cloak -- Start the object into cloaking stage.                            *
 *   TechnoClass::Do_Shimmer -- Causes this object to shimmer if it is cloaked.                *
 *   TechnoClass::Do_Uncloak -- Cause the stealth tank to uncloak.                             *
 *   TechnoClass::Draw_It -- Draws the health bar (if necessary).                              *
 *   TechnoClass::Draw_Pips -- Draws the transport pips and other techno graphics.             *
 *   TechnoClass::Electric_Zap -- Fires electric zap at the target specified.                  *
 *   TechnoClass::Enter_Idle_Mode -- Object enters its default idle condition.                 *
 *   TechnoClass::Evaluate_Cell -- Determine the value and object of specified cell.           *
 *   TechnoClass::Evaluate_Just_Cell -- Evaluate a cell as a target by itself.                 *
 *   TechnoClass::Evaluate_Object -- Determines score value of specified object.               *
 *   TechnoClass::Exit_Object -- Causes specified object to leave this object.                 *
 *   TechnoClass::Find_Docking_Bay -- Searches for a close docking bay.                        *
 *   TechnoClass::Find_Exit_Cell -- Finds an appropriate exit cell for this object.            *
 *   TechnoClass::Fire_At -- Fires projectile at target specified.                             *
 *   TechnoClass::Fire_Coord -- Determine the coordinate where bullets appear.                 *
 *   TechnoClass::Fire_Direction -- Fetches the direction projectile fire will take.           *
 *   TechnoClass::Get_Ownable -- Fetches the ownable bits for this object.                     *
 *   TechnoClass::Greatest_Threat -- Determines best target given search criteria.             *
 *   TechnoClass::Hidden -- Returns the object back into the hidden state.                     *
 *   TechnoClass::How_Many_Survivors -- Determine the number of survivors to escape.           *
 *   TechnoClass::In_Range -- Determines if specified target is within weapon range.           *
 *   TechnoClass::In_Range -- Determines if specified target is within weapon range.           *
 *   TechnoClass::In_Range -- Determines if the specified coordinate is within range.          *
 *   TechnoClass::Is_Allowed_To_Recloak -- Can this object recloak?                            *
 *   TechnoClass::Is_Allowed_To_Retaliate -- Checks object to see if it can retaliate.         *
 *   TechnoClass::Is_In_Same_Zone -- Determine if specified cell is in same zone as object.    *
 *   TechnoClass::Is_Players_Army -- Determines if this object is part of the player's army.   *
 *   TechnoClass::Is_Ready_To_Cloak -- Determines if this object is ready to begin cloaking.   *
 *   TechnoClass::Is_Ready_To_Random_Animate -- Determines if the object should random animate.*
 *   TechnoClass::Is_Visible_On_Radar -- Is this object visible on player's radar screen?      *
 *   TechnoClass::Is_Weapon_Equipped -- Determines if this object has a combat weapon.         *
 *   TechnoClass::Kill_Cargo -- Destroys any cargo attached to this object.                    *
 *   TechnoClass::Look -- Performs a look around (map reveal) action.                          *
 *   TechnoClass::Mark -- Handles marking of techno objects.                                   *
 *   TechnoClass::Nearby_Location -- Radiates outward looking for clear cell nearby.           *
 *   TechnoClass::Owner -- Who is the owner of this object?                                    *
 *   TechnoClass::Per_Cell_Process -- Handles once-per-cell operations for techno type objects.*
 *   TechnoClass::Pip_Count -- Fetches the number of pips to display on this object.           *
 *   TechnoClass::Player_Assign_Mission -- Assigns a mission as result of player input.        *
 *   TechnoClass::Rearm_Delay -- Calculates the delay before firing can occur.                 *
 *   TechnoClass::Receive_Message -- Handles inbound message as appropriate.                   *
 *   TechnoClass::Record_The_Kill -- Records the death of this object.                         *
 *   TechnoClass::Refund_Amount -- Returns with the money to refund if this object is sold.    *
 *   TechnoClass::Remap_Table -- Fetches the appropriate remap table to use.                   *
 *   TechnoClass::Renovate -- Heal a building to maximum                                       *
 *   TechnoClass::Response_Attack -- Handles the voice response when given attack order.       *
 *   TechnoClass::Response_Move -- Handles the voice response to a movement request.           *
 *   TechnoClass::Response_Select -- Handles the voice response when selected.                 *
 *   TechnoClass::Revealed -- Handles revealing an object to the house specified.              *
 *   TechnoClass::Risk -- Fetches the risk associated with this object.                        *
 *   TechnoClass::Select -- Selects object and checks to see if can be selected.               *
 *   TechnoClass::Set_Mission -- Forced mission set (used by editor).                          *
 *   TechnoClass::Stun -- Prepares the object for removal from the game.                       *
 *   TechnoClass::Take_Damage -- Records damage assessed to this object.                       *
 *   TechnoClass::Target_Something_Nearby -- Handles finding and assigning a nearby target.    *
 *   TechnoClass::TechnoClass -- Constructor for techno type objects.                          *
 *   TechnoClass::Techno_Draw_Object -- General purpose draw object routine.                   *
 *   TechnoClass::Threat_Range -- Returns the range to scan based on threat control.           *
 *   TechnoClass::Tiberium_Load -- Fetches the current tiberium load percentage.               *
 *   TechnoClass::Time_To_Build -- Determines the time it would take to build this.            *
 *   TechnoClass::Unlimbo -- Performs unlimbo process for all techno type objects.             *
 *   TechnoClass::Value -- Fetches the target value for this object.                           *
 *   TechnoClass::Visual_Character -- Determine the visual character of the object.            *
 *   TechnoClass::Weapon_Range -- Determines the maximum range for the weapon.                 *
 *   TechnoClass::What_Action -- Determines action to perform if cell is clicked on.           *
 *   TechnoClass::What_Action -- Determines what action to perform if object is selected.      *
 *   TechnoClass::What_Weapon_Should_I_Use -- Determines what is the best weapon to use.       *
 *   TechnoTypeClass::Cost_Of -- Fetches the cost of this object type.                         *
 *   TechnoTypeClass::Get_Cameo_Data -- Fetches the cameo image for this object type.          *
 *   TechnoTypeClass::Get_Ownable -- Fetches the ownable bits for this object type.            *
 *   TechnoTypeClass::Is_Two_Shooter -- Determines if this object is a double shooter.         *
 *   TechnoTypeClass::Raw_Cost -- Fetches the raw (base) cost of the object.                   *
 *   TechnoTypeClass::Read_INI -- Reads the techno type data from the INI database.            *
 *   TechnoTypeClass::Repair_Cost -- Fetches the cost to repair one step.                      *
 *   TechnoTypeClass::Repair_Step -- Fetches the health to repair one step.                    *
 *   TechnoTypeClass::TechnoTypeClass -- Constructor for techno type objects.                  *
 *   TechnoTypeClass::Time_To_Build -- Fetches the time to build this object.                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "techno.h"

#include "_bench.h"
#include "_convert.h"
#include "_keyboar.h"
#include "_map.h"
#include "_rtti.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "_uicontrol.h"
#include "actionline.h"
#include "aircraft.h"
#include "airctype.h"
#include "anim.h"
#include "bench.h"
#include "blit.h"
#include "bsurface.h"
#include "building.h"
#include "builtype.h"
#include "bullet.h"
#include "bullettype.h"
#include "ccrand.h"
#include "cell.h"
#include "combat.h"
#include "data.h"
#include "dbgprint.h"
#include "dialog.h"
#include "draw.h"
#include "dsurface.h"
#include "fog.h"
#include "globals.h"
#include "goptions.h"
#include "house.h"
#include "houstype.h"
#include "infantry.h"
#include "infatype.h"
#include "inline.h"
#include "ion.h"
#include "isotile.h"
#include "isotype.h"
#include "language/language.h"
#include "laser.h"
#include "lightcon.h"
#include "mono.h"
#include "overtype.h"
#include "partsys.h"
#include "psystype.h"
#include "queue.h"
#include "revent.h"
#include "rules.h"
#include "savestream.h"
#include "scheme.h"
#include "session.h"
#include "shapeset.h"
#include "stimer.h"
#include "sun.h"
#include "swizzle.h"
#include "syncrechook.h"
#include "tactical.h"
#include "tag.h"
#include "team.h"
#include "techtype.h"
#include "tracker.h"
#include "uicontrol.h"
#include "unit.h"
#include "unittype.h"
#include "vanim.h"
#include "vein.h"
#include "voc.h"
#include "vox.h"
#include "voxdrsys.h"
#include "warhead.h"
#include "wave.h"
#include "weapon.h"

#include "bench.hh"
#include "tube.hh"

#include <algorithm>
#include <intrin.h>

CDTimerClass<FrameTimerClass> TechnoClass::ActionLineTimer;
bool TechnoClass::ActionLines = true;

TalkType TechnoClass::TalkBubbleType = TALK_NONE;
TechnoClass * TechnoClass::TalkBubbleOwner = NULL;
CDTimerClass<SystemTimerClass> TechnoClass::TalkBubbleTimer;


/***********************************************************************************************
 * TechnoClass::TechnoClass -- Default constructor for techno objects.                         *
 *                                                                                             *
 *    This default constructor for techno objects is used to reset all internal variables to   *
 *    their default state.                                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/09/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
TechnoClass::TechnoClass(HouseClass * house) :
	BASECLASS(),
	IsUseless(false),
	IsTickedOff(false),
	IsCloakable(false),
	IsLeader(false),
	IsALoaner(false),
	IsLocked(false),
	IsInRecoilState(false),
	IsTethered(false),
	IsOwnedByPlayer(false),
	IsDiscoveredByPlayer(false),
	IsDiscoveredByComputer(false),
	IsALemon(false),
	ArmorBias(1),
	FirepowerBias(1),
	IdleTimer(0),
	SpiedBy(0),
	ArchivedTarget(NULL),
	House(house),
	Cloak(UNCLOAKED),
	TarCom(NULL),
	SuspendedTarCom(NULL),
	PrimaryFacing(),
	SecondaryFacing(),
	Arm(0),
	Ammo(-1),
	PurchasePrice(0),
	ActLike(HOUSE_NONE),
	Cargo(),
	Veterancy(),
	RadarFlashTimer(),
	RadarPos(0,0),
	Group(-1),
	CloakingDevice(),
	CloakDelay(),
	PredatorOffset(0),
	PitchAngle(0),
	Wave(NULL),
	AngleRotatedSideways(0.0),
	AngleRotatedForwards(0.0),
	RockingSidewaysPerFrame(0.0),
	RockingForwardsPerFrame(0.0),
	EnteredByInfType(INFANTRY_NONE),
	Storage(),
	Door(),
	BarrelPitch(3),
	BurstIndex(0),
	IsBurstResetPending(false),
	BurstResetTimer(),
	TargetingLaserTimer(),
	SinkingYOffset(0),
	IsSinking(false),
	IsNeedingRescue(false),
	UnusedCooldown(0),
	Unused1(0),
	SightIncrease(0),
	IsTeamRecruitable(true),
	IsAutocreateRecruitable(true),
	IsRadarTracked(false),
	IsInTransport(false),
	IsRocking(false),
	IsOnPatrol(false),
	IsOnWaypointPatrol(false),
	NearbyObject(NULL),
	StunDuration(0),
	LimpetType(0)
{
	if (house != NULL) {
		ActLike = house->ActLike;
	}

	Technos.Add(this);
	HousePtrTracker.Add(this);

	for (int i = 0; i < ARRAY_SIZE(ParticleSystems); i++) {
		ParticleSystems[i] = NULL;
	}
	SoundRandomSeed = Scen->RandomNumber();
}


/// <summary>
/// Fetches the techno type class for this object.
/// Every ownable object's type derives from TechnoTypeClass, so this narrows the general type
/// pointer down to the one the combat and production logic works through.
/// </summary>
/// <returns>Returns with a pointer to this object's techno type class.</returns>
TechnoTypeClass const * TechnoClass::Techno_Type_Class(void) const
{
	return((TechnoTypeClass const *)Class_Of());
}


/// <summary>
/// Determines whether this object is allowed to scatter.
/// False while sleeping, sticky, or unloading, or if the type is a train.
/// </summary>
/// <returns>True if the object may scatter, false otherwise.</returns>
bool TechnoClass::Can_Scatter(void) const
{
	if (Mission != MISSION_SLEEP && Mission != MISSION_STICKY && Mission != MISSION_UNLOAD && !TClass->IsTrain) {
		return(true);
	}

	return(false);
}


/***************************************************************************
**	Which shape to use depending on which facing is controlled by these arrays.
*/
int const TechnoClass::BodyShape[FACING_COUNT * 4] = {28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,31,30,29};


/***********************************************************************************************
 * TechnoClass::Is_Players_Army -- Determines if this object is part of the player's army.     *
 *                                                                                             *
 *    The player's army is considered to be all those mobile units that can be selected        *
 *    and controlled by the player (they may or may not have weapons in the traditional        *
 *    sense).                                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is this object part of the player's selectable controllable army?            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/23/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoClass::Is_Players_Army(void) const
{
	/*
	**	An object that is dead (or about to be) is not considered part of
	**	the player's army.
	*/
	if (Strength <= 0) {
		return(false);
	}

	/*
	**	If the object is not yet on the map or is otherwise indisposed, then
	**	don't consider it.
	*/
	if (IsInLimbo || !IsLocked) {
		return(false);
	}

	/*
	**	Buildings, although sometimes serving a combat purpose, are not part
	**	of the player's army.
	*/
	if (RTTI == RTTI_BUILDING) {
		return(false);
	}

	/*
	**	If not discoverd by the player, then don't consider it part of the
	**	player's army (yet).
	*/
	if (!IsDiscoveredByPlayer) {
		return(false);
	}

	/*
	**	If not selectable, then not really part of the player's active army.
	*/
	if (!TClass->IsSelectable) {
		return(false);
	}

	/*
	**	If not under player control, then it isn't part of the player's army.
	*/
	if (!House->Is_Player_Control()) {
		return(false);
	}
	return(true);
}


/***********************************************************************************************
 * TechnoClass::What_Weapon_Should_I_Use -- Determines what is the best weapon to use.         *
 *                                                                                             *
 *    This routine will compare the weapons this object is equipped with verses the            *
 *    candidate target object. The best weapon to use against the target will be returned.     *
 *    Special emphasis is given to weapons that can fire on the target without requiring       *
 *    this object to move within range.                                                        *
 *                                                                                             *
 * INPUT:   target   -- The candidate target to determine which weapon is best against.        *
 *                                                                                             *
 * OUTPUT:  Returns with an identifier the specifies what weapon to use against the target.    *
 *          The return value will be "0" for the primary weapon and "1" for the secondary.     *
 *                                                                                             *
 * WARNINGS:   This routine is called very frequently. It should be as efficient as possible.  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/12/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoClass::What_Weapon_Should_I_Use(AbstractClass * target) const
{
	if (target == NULL) return(0);

	bool webby1 = false;
	bool webby2 = false;

	/*
	**	Fetch the armor of the candidate target object. Presume that if the target
	**	is not an object, then its armor is equivalent to wood. Who knows why?
	*/
	ArmorType armor = ARMOR_NONE;
	ObjectClass const * object = target->As_ObjectClass();
	if (object != NULL) {
		armor = object->Class_Of()->Armor;
	}

	/*
	**	Get the value of the primary weapon verses the candidate target. Increase the
	**	value of the weapon if it happens to be in range.
	*/
	int w1 = 0;
	FireErrorType ok = Can_Fire(target, 0);
	if (ok == FIRE_CANT || ok == FIRE_ILLEGAL || ok == FIRE_REARM) {
		w1 = 0;
	} else {
		WeaponTypeClass const * wptr = PrimaryWeapon;
		if (wptr != NULL) {
			if (wptr->WarheadPtr != NULL) {
				webby1 = wptr->WarheadPtr->IsWebby;
				w1 = wptr->WarheadPtr->Modifier[armor] * 1000;
			}
			if (In_Range(target, 0)) w1 *= 2;
		}
	}


	/*
	**	Calculate a similar value for the secondary weapon.
	*/
	int w2 = 0;
	ok = Can_Fire(target, 1);
	if (ok == FIRE_CANT || ok == FIRE_ILLEGAL || ok == FIRE_REARM) {
		w2 = 0;
	} else {
		WeaponTypeClass const * wptr = SecondaryWeapon;
		if (wptr != NULL) {
			if (wptr->WarheadPtr != NULL) {
				webby2 = wptr->WarheadPtr->IsWebby;
				w2 = wptr->WarheadPtr->Modifier[armor] * 1000;
			}
		}
		if (In_Range(target, 1)) w2 *= 2;
	}

	/*
	**	Return with the weapon identifier that should be used to fire upon the
	**	candidate target.
	*/
	if (!webby1 && !webby2) {
		if (w2 > w1) return(1);
		return(0);
	}

	bool is_web_target = false;
	if (object == NULL) {
		CellClass * cellptr = target->As_CellClass();
		if (cellptr != NULL) {
			is_web_target = true;
			if (cellptr->ITType == IsometricTileTypeClass::DestroyableCliffs || cellptr->ITType == IsometricTileTypeClass::DestroyableCliffs + DESTROYABLE_CLIFFS_COUNT - 1
				|| cellptr->IsUnderBridge || cellptr->Overlay >= OVERLAY_LOWBRIDGE_01 && cellptr->Overlay <= OVERLAY_LOWBRIDGE_26) {
				is_web_target = false;
			}
		}
	} else {
		if (object->Is_Foot()) {
			InfantryClass const * inf = dynamic_cast<InfantryClass const *>(object);
			if (inf != NULL && !inf->Is_Immobilized() && !inf->Class->IsWebImmune) {
				is_web_target = true;
			} else {
				is_web_target = false;
			}
		} else {
			//is_web_target = object->As_IsometricTileClass() != NULL;
			is_web_target = dynamic_cast<IsometricTileClass const *>(object) != NULL;
		}
	}

	return(is_web_target == webby2);
}


/***********************************************************************************************
 * TechnoClass::How_Many_Survivors -- Determine the number of survivors to escape.             *
 *                                                                                             *
 *    This routine will determine the number of survivors that should run from this object     *
 *    when it is destroyed.                                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the maximum number of survivors that should run from this object      *
 *          when the object gets destroyed.                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/04/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoClass::How_Many_Survivors(void) const
{
	if (TClass->IsCrew) {
		return(1);
	}
	return(0);
}


/***********************************************************************************************
 * TechnoClass::Combat_Damage -- Fetch the amount of damage infliced by the specified weapon.  *
 *                                                                                             *
 *    This routine will examine the specified weapon of this object and determine how much     *
 *    damage it could inflict -- best case.                                                    *
 *                                                                                             *
 * INPUT:   which -- Which weapon to consider. If -1 is specified, then the average of both    *
 *                   weapon types (if present) is returned.                                    *
 *                                                                                             *
 * OUTPUT:  Returns with the combat damage that could be inflicted by the specified weapon.    *
 *                                                                                             *
 * WARNINGS:   This routine could return a negative number if a medic is scanned.              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/16/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoClass::Combat_Damage(int which) const
{
	int divisor = 0;
	int value = 0;

	if (which == 0 || which == -1) {
		if (PrimaryWeapon != NULL) {
			value += PrimaryWeapon->Attack;
			value += PrimaryWeapon->AmbientDamage;
			divisor = 1;
		}
	}

	if (which == 1 || which == -1) {
		if (SecondaryWeapon != NULL) {
			value += SecondaryWeapon->Attack;
			value += SecondaryWeapon->AmbientDamage;
			divisor += 1;
		}
	}

	if (divisor > 1) {
		return(value / divisor);
	}
	return(value);
}


/***********************************************************************************************
 * TechnoClass::Fire_Coord -- Determine the coordinate where bullets appear.                   *
 *                                                                                             *
 *    This routine will determine the coordinate to use when this object fires. The coordinate *
 *    is the location where bullets appear (or fire effects appear) when the object fires      *
 *    its weapon.                                                                              *
 *                                                                                             *
 * INPUT:   which -- Which weapon is the coordinate to be calculated for? 0 means primary      *
 *                   weapon, 1 means secondary weapon.                                         *
 *                                                                                             *
 * OUTPUT:  Returns with the coordinate that any bullets fired from the specified weapon       *
 *          should appear.                                                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
Coord TechnoClass::Fire_Coord(int which) const
{
	TechnoTypeClass const * tclass = TClass;
	WeaponDataStruct const * weapon = Get_Class_Weapon_Data(which);

	int flhx = weapon->FireFLH.X;
	int flhy = weapon->FireFLH.Y;
	int flhz = weapon->FireFLH.Z;

	Matrix3D matrix;
	matrix.Make_Identity();
	matrix.Rotate_Z(Turret_Facing().As_Radian32());
	matrix.Translate(flhx + tclass->TurretOffset, flhy * (BurstIndex % 2 != 0 ? -1 : 1), flhz + weapon->BarrelThickness);
	matrix.Rotate_Y(-BarrelPitch.Current().As_Radian32());
	matrix.Translate(weapon->BarrelLength, 0, 0);

	Vector3 firevec = matrix * Vector3(0, 0, 0);
	Coord coord(firevec.X, -firevec.Y, firevec.Z);

	return(coord + Render_Coord());
}


/// <summary>
/// Fetches the coordinate that a weapon is mounted at.
/// This is where a projectile is created, and where a firing solution is measured from. The
/// barrel is deliberately not accounted for, so the coordinate holds still as the weapon
/// elevates -- unlike Fire_Coord, which follows the muzzle. A burst weapon alternates between
/// its left and right mounting on successive shots.
/// </summary>
/// <param name="which">Which of the object's weapons to fetch the mounting for.</param>
/// <returns>Returns with the coordinate the projectile should be created at.</returns>
Coord TechnoClass::Turret_Coord(int which) const
{
	TechnoTypeClass const * tclass = TClass;
	WeaponDataStruct const * weapon = Get_Class_Weapon_Data(which);

	int flhx = weapon->FireFLH.X;
	int flhy = weapon->FireFLH.Y;
	int flhz = weapon->FireFLH.Z;

	Matrix3D matrix;
	double angle = 0;
	FootClass const * foot = dynamic_cast<FootClass const *>(this);

	if (foot != NULL) {
		matrix = foot->Locomotion->Draw_Matrix(NULL);
		angle = PrimaryFacing.Current().As_Radian32();
	} else {
		matrix.Make_Identity();
	}

	matrix.Rotate_Z(Turret_Facing().As_Radian32() - angle);
	matrix.Translate(flhx + tclass->TurretOffset, flhy * (BurstIndex % 2 != 0 ? -1 : 1), flhz);

	Vector3 firevec = matrix * Vector3(0, 0, 0);
	Coord coord(firevec.X, -firevec.Y, firevec.Z);

	return(Render_Coord() + coord);
}


#ifdef _DEBUG
/***********************************************************************************************
 * TechnoClass::Debug_Dump -- Displays the base class data to the monochrome screen.           *
 *                                                                                             *
 *    This routine is used to dump the status of the object class to the monochrome screen.    *
 *    This display can be used to track down or prevent bugs.                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/02/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Debug_Dump(MonoClass * mono) const
{
	mono->Set_Cursor(10, 7);mono->Printf("%2d", Fetch_Rate());
	mono->Set_Cursor(14, 7);mono->Printf("%2d", Fetch_Stage());
	mono->Set_Cursor(49, 1);mono->Printf("%3d", Ammo);
	mono->Set_Cursor(71, 1);mono->Printf("$%4d", PurchasePrice);
	mono->Set_Cursor(54, 1);mono->Printf("%3d", (int)Arm);
	if (Cargo.Is_Something_Attached()) {
		mono->Set_Cursor(1, 5);mono->Printf("%08X", Cargo.Attached_Object());
	}
	if (TarCom != NULL) {
		mono->Set_Cursor(29, 3);mono->Printf("%08X", TarCom);
	}
	if (SuspendedTarCom != NULL) {
		mono->Set_Cursor(38, 3);mono->Printf("%08X", SuspendedTarCom);
	}
	if (ArchiveTarget != NULL) {
		mono->Set_Cursor(69, 5);mono->Printf("%08X", ArchiveTarget);
	}
	mono->Set_Cursor(47, 3);mono->Printf("%02X:%02X", PrimaryFacing.Current(), PrimaryFacing.Desired());
	mono->Set_Cursor(64, 1);mono->Printf("%d(%d)", Cloak, CloakingDevice.Fetch_Stage());

	mono->Fill_Attrib(14, 15, 12, 1, IsUseless ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(14, 16, 12, 1, IsTickedOff ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(14, 17, 12, 1, IsCloakable ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(27, 13, 12, 1, IsLeader ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(27, 14, 12, 1, IsALoaner ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(27, 15, 12, 1, IsLocked ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(27, 16, 12, 1, IsInRecoilState ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(27, 17, 12, 1, IsTethered ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(40, 13, 12, 1, IsOwnedByPlayer ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(40, 14, 12, 1, IsDiscoveredByPlayer ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(40, 15, 12, 1, IsDiscoveredByComputer ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(40, 16, 12, 1, IsALemon ? MonoClass::INVERSE : MonoClass::NORMAL);

	BASECLASS::Debug_Dump(mono);
}
#endif


/// <summary>
/// Initializes owner-related state for the object.
/// Sets IsOwnedByPlayer based on whether the object's house is the player's house.
/// </summary>
void TechnoClass::Init(void)
{
	if (House != NULL) {
		if (PlayerPtr == House) {
			IsOwnedByPlayer = true;
		} else {
			IsOwnedByPlayer = false;
		}
	}
}


/// <summary>
/// Destroys the techno object and unlinks it from the game.
/// The object gives up its house and drops out of the global techno lists that were holding
/// it.
/// </summary>
TechnoClass::~TechnoClass(void)
{
	House=0;
	Technos.Delete(this);
	HousePtrTracker.Delete(this);
}


/***********************************************************************************************
 * TechnoClass::Time_To_Build -- Determines the time it would take to build this.              *
 *                                                                                             *
 *    Use this routine to determine the amount of time it would take to build an object of     *
 *    this type.                                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the time it should take (unmodified by outside factors) to build      *
 *          this object. The time is expressed in game frames.                                 *
 *                                                                                             *
 * WARNINGS:   The time will usually be modified by power status and handicap rating for the   *
 *             owning house. The value returned is merely the raw unmodified time to build.    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *   09/27/1996 JLB : Takes into account the power availability.                               *
 *=============================================================================================*/
int TechnoClass::Time_To_Build(void) const
{
	int val = Class_Of()->Time_To_Build();

	val *= House->BuildSpeedBias;

	/*
	**	Adjust the time to build based on the power output of the owning house.
	*/
	double power = House->Power_Fraction();
	if (power > 1.0) power = 1.0;
	if (power < 1.0 && power > 0.75) power = 0.75;
	if (power < 0.5) power = 0.5;
	power = std::max(power, Rule->MinLowPowerProductionSpeed);
	val /= power;

	int divisor = House->Factory_Count(RTTI);
	if (divisor > 1 && Rule->MultipleFactory > 0) {
		val *= 1.0 / ((divisor - 1) * Rule->MultipleFactory);
	}
	if (RTTI == RTTI_BUILDING && ((BuildingClass *)this)->Class->IsWall) {
		val *= Rule->WallBuildSpeedCoefficient;
	}
	return(val);
}


/***********************************************************************************************
 * TechnoClass::Revealed -- Handles revealing an object to the house specified.                *
 *                                                                                             *
 *    When a unit moves out from under the shroud or when it gets delivered into already       *
 *    explored terrain, then it must be "revealed". Objects that are revealed may be           *
 *    announced to the player. The discovered bit updated accordingly.                         *
 *                                                                                             *
 * INPUT:   house -- The house that this object is revealed to.                                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/02/1994 JLB : Created.                                                                 *
 *   12/27/1994 JLB : Discovered trigger event processing.                                     *
 *=============================================================================================*/
bool TechnoClass::Revealed(HouseClass * house)
{
	if (house == PlayerPtr && IsDiscoveredByPlayer) return(false);
	if (house != PlayerPtr) {
		if (IsDiscoveredByComputer) return(false);
		IsDiscoveredByComputer = true;
	}

	if (BASECLASS::Revealed(house)) {

		/*
		**	An enemy object that is discovered will go into hunt mode if
		**	its current mission is to ambush.
		*/
		if (!House->Is_Human_Player() && Mission == MISSION_AMBUSH) {
			Assign_Mission(MISSION_HUNT);
		}

		if (house == PlayerPtr) {
			IsDiscoveredByPlayer = true;
			House->RecalcPower = true;
			House->RecalcRadar = true;

			if (!IsOwnedByPlayer) {

				/*
				**	If there is a trigger event associated with this object, then process
				**	it for discovery purposes.
				*/
				if (!ScenarioInit && Tag != NULL) {
					Tag->Spring(TEVENT_DISCOVERED, this);
				}

				/*
				**	Alert the enemy house to presence of the friendly side.
				*/
				House->IsDiscovered = true;
			}

			// Outside a campaign every object looks and Sight_From decides whose shroud
			// lifts, so an ally's placed structure reveals for the player; a campaign keeps
			// the look to the player's own objects so discovery does not chain through an
			// allied base.
			if (IsOwnedByPlayer || Session.Type != GAME_NORMAL) {
				Look();
			}
		} else {
			IsDiscoveredByComputer = true;
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * TechnoClass::Hidden -- Returns the object back into the hidden state.                       *
 *                                                                                             *
 *    This routine is called for every object that returns to the darkness shroud or when      *
 *    it gets destroyed. This also occurs when an object enters another (such as infantry      *
 *    entering a transport helicopter).                                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/02/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Hidden(void)
{
	if (!IsDiscoveredByPlayer) return;
	if (!House->Is_Human_Player()) {
		IsDiscoveredByPlayer = false;
	}
}


/***********************************************************************************************
 * TechnoClass::Mark -- Handles marking of techno objects.                                     *
 *                                                                                             *
 *    On the Techno-level, marking handles transmission of the redraw command to any object    *
 *    that it is 'connected' with. This only occurs during loading and unloading.              *
 *                                                                                             *
 * INPUT:   mark  -- The marking method. This routine just passes this on to base classes.     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoClass::Mark(MarkType mark)
{
	if (BASECLASS::Mark(mark)) {

		/*
		**	When redrawing an object, if there is another object tethered to this one,
		**	redraw it as well.
		*/
		if (IsTethered) {
			Transmit_Message(RADIO_REDRAW);
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * TechnoClass::Receive_Message -- Handles inbound message as appropriate.                     *
 *                                                                                             *
 *    This routine is used to handle inbound radio messages. It only handles those messages    *
 *    that deal with techno objects. Typically, these include loading and unloading.           *
 *                                                                                             *
 * INPUT:   from     -- Pointer to the originator of the radio message.                        *
 *                                                                                             *
 *          message  -- The inbound radio message.                                             *
 *                                                                                             *
 *          param    -- Reference to optional parameter that might be used to transfer         *
 *                      more information than is possible with the simple radio message        *
 *                      type.                                                                  *
 *                                                                                             *
 * OUTPUT:  Returns with the radio response.                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *   06/17/1995 JLB : Handles tether contact messages.                                         *
 *=============================================================================================*/
RadioMessageType TechnoClass::Receive_Message(RadioClass * from, RadioMessageType message, int & param)
{
	switch (message) {

		/*
		**	Just received instructions to attack the specified target.
		*/
		case RADIO_ATTACK_THIS:
			if (PrimaryWeapon != NULL) {
				Assign_Target((AbstractClass *)param);
				Assign_Mission(MISSION_ATTACK);
				return(RADIO_ROGER);
			}
			break;

		/*
		**	Establish a tethered connection to the object in radio contact.
		*/
		case RADIO_TETHER:
			if (!IsTethered) {
				IsTethered = true;
				Transmit_Message(RADIO_TETHER, from);
				return(RADIO_ROGER);
			}
			break;

		/*
		**	Break the tethered connection to the object in radio contact.
		*/
		case RADIO_UNTETHER:
			if (IsTethered) {
				IsTethered = false;
				Transmit_Message(RADIO_UNTETHER, from);
				return(RADIO_ROGER);
			}
			break;

		/*
		**	A tethered unit has reached it's destination. All is
		**	clear, so radio contact can be broken.
		*/
		case RADIO_UNLOADED:
			Transmit_Message(RADIO_UNTETHER, from);
			return(Transmit_Message(RADIO_OVER_OUT, from));

		/*
		**	When this message is received, it means that the other object
		**	has already turned its radio off. Turn this radio off as well.
		*/
		case RADIO_OVER_OUT:
			Transmit_Message(RADIO_UNTETHER, from);
			BASECLASS::Receive_Message(from, message, param);
			return(RADIO_ROGER);

		/*
		**	Request to be informed when unloaded. This message is transmitted
		**	by the transport unit to the transported unit as it is about to be
		**	unloaded. It is saying, "All is clear. Drive off now."
		*/
		case RADIO_UNLOAD:
		case RADIO_BACKUP_NOW:
		case RADIO_HOLD_STILL:
			Transmit_Message(RADIO_TETHER, from);
			BASECLASS::Receive_Message(from, message, param);
			return(RADIO_ROGER);

		/*
		**	Handle reloading one ammo point for this unit.
		*/
		case RADIO_RELOAD:
			if (Ammo == TClass->MaxAmmo) return(RADIO_NEGATIVE);
			Ammo++;
			return(RADIO_ROGER);

		/*
		**	Handle repair of this unit.
		*/
		case RADIO_REPAIR:
			LimpetType = 0;
			LimpetSpeedFactor = 0;

			PrimaryFacing.Set_ROT(TClass->ROT);
			SecondaryFacing.Set_ROT(TClass->ROT);

			/*
			**	If it's a mine layer, re-arm him if he's empty. This always takes precedence
			**	over repair, since this operation is free.
			*/
			if (TClass->IsManualReload && ((UnitClass *)this)->Ammo < ((UnitClass *)this)->Class->MaxAmmo) {
				((UnitClass *)this)->Ammo = ((UnitClass *)this)->Class->MaxAmmo;
				return(RADIO_NEGATIVE);
			}

			/*
			**	Determine if this unit can be repaired becaause it is under strength. If so, then
			**	proceed with the repair process.
			*/
			if (HealthRatio < Rule->ConditionGreen) {
				int cost = TClass->Repair_Cost();
				int step = TClass->Repair_Step();
				step = std::max(step, 1);

				/*
				**	If there is sufficient money to repair the unit one step, then do so.
				**	Otherwise return with a "can't complete" radio response.
				*/
				if (House->Available_Money() >= cost) {
					if (cost != 0) {
						House->Spend_Money(cost);
					}
					Strength += step;

					if (HealthRatio > Rule->ConditionYellow || HeightAGL < -10) {
						if (ParticleSystems[ATTACHED_PARTICLE_DAMAGE]) {
							ParticleSystems[ATTACHED_PARTICLE_DAMAGE]->Delete_Me();
						}
					}

					/*
					**	Return with either an all ok or mission accomplished radio message. This
					**	lets the repairing object know if it should abort the repair control process
					**	or continue it.
					*/
					if (HealthRatio < Rule->ConditionGreen) {
						return(RADIO_ROGER);
					} else {
						Strength = TClass->MaxStrength;
						return(RADIO_ALL_DONE);
					}
				} else {
					return(RADIO_CANT);
				}
			}
			return(RADIO_NEGATIVE);

		default:
			break;
	}
	return(BASECLASS::Receive_Message(from, message, param));
}


/// <summary>
/// Cloaks this object if it is standing somewhere that will hide it.
/// This routine is called as the object settles into cloaking cover. Everything that was
/// shooting at it is told to re-acquire it afterwards, so that an attacker which can still
/// sense the object does not simply lose track of it.
/// </summary>
void TechnoClass::Try_To_Cloak(void)
{
	int i;

	CellClass & cell = Map[Center_Coord().As_Cell()];
	if (cell.Is_Cloaked(House->HeapID) && Is_Ready_To_Cloak()) {
		DynamicVectorClass<TechnoClass *> targeting_me;
		for (i = Technos.Count() - 1; i >= 0; i--) {
			TechnoClass * tech = Technos[i];
			if (tech->TarCom == this) {
				if (Map[Center_Coord()].Is_Sensed(tech->House->HeapID) || tech->House == House) {
					targeting_me.Add(tech);
				}
			}
		}
		Do_Cloak();
		for (i = targeting_me.Count() - 1; i >= 0; i--) {
			targeting_me[i]->Assign_Target(this);
		}
	}
}


/***********************************************************************************************
 * TechnoClass::Per_Cell_Process -- Handles once-per-cell operations for techno type objects.  *
 *                                                                                             *
 *    This routine handles marking a game object as not a loaner. It is set only if the unit   *
 *    is not player owned and is on the regular map. This is necessary so that enemy objects   *
 *    can exist off-map but as soon as they move onto the map, are flagged so that can never   *
 *    leave it again.                                                                          *
 *                                                                                             *
 * INPUT:   why   -- Specifies the circumstances under which this routine was called.          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *   10/26/94   JLB : Handles scanner units.                                                   *
 *   12/27/1994 JLB : Checks for an processes any trigger in cell.                             *
 *=============================================================================================*/
void TechnoClass::Per_Cell_Process(PCPType why)
{
	if (why == PCP_END) {
		Cell cell = Center_Coord().As_Cell();

		Try_To_Cloak();

		if (Tag != NULL) {
			Tag->Spring(TEVENT_NEAR_WAYPOINT, this);
		}

		/*
		**	When enemy units enter the proper map area from off map, they are
		**	flagged so that they won't travel back off the map again.
		*/
		if (!IsLocked && Map.In_Local_Radar(cell)) {
	  		IsLocked = true;
		}

		/*
		**	If this object somehow moves into mapped terrain, but is not yet
		**	discovered, then flag it to be discovered.
		*/
		if (!IsDiscoveredByPlayer && Map[cell].IsVisible) {
			Revealed(PlayerPtr);
		}

		Map[cell].Trigger_Veins();
	}
}


/// <summary>
/// Draws the decorations that appear after the object's body.
/// A selected object, or a submerged one the player's sensors have picked up, gets its
/// selection box and its condition indicator drawn over it, plus its pips when the player
/// is allied to its owner or spying on that house. Without a selection, the object under
/// the mouse still shows its condition, and an allied or spied-on object still wears its
/// insignia. The talk bubble, when the object has something to say, is drawn whether the
/// object is selected or not.
/// </summary>
void TechnoClass::Draw_Post_Render(Point2D const & point, Rect const & cliprect) const
{
	bool sensed_underground = false;
	if (!IsSelected) {
		CellClass * cell = Get_Cell_Ptr();
		if (HeightAGL < -20 && cell->Is_Sensed(PlayerPtr->HeapID)) {
			sensed_underground = true;
		}
	}

	bool allied = House->Shares_View_With(PlayerPtr) || (SpiedBy & (1<<(PlayerPtr->Class->House)));

	if (IsSelected || sensed_underground) {

		UnitClass const * unit = (RTTI == RTTI_UNIT) ? dynamic_cast<UnitClass const *>(this) : NULL;
		if (RTTI == RTTI_BUILDING || (unit != NULL && unit->Class->IsCoreDefender)) {

			int color = WHITE;
			if (LimpetType > 0) {
				color = YELLOW;
			}
			if (HeightAGL < -4) {
				color = BLACK;
			}

			Coord dim = TClass->Lepton_Dimensions();
			int x = dim.X / 2;
			int y = dim.Y / 2;
			color = NormalDrawer->Convert_Pixel(color);
			Coord center = Center_Coord();

			if (RTTI != RTTI_INFANTRY) {
				Draw_Double_Selection_Bracket(center + Coord(x, y, 0), center + Coord(-x, y, 0), color);
				Draw_Double_Selection_Bracket(center + Coord(x, y, 0), center + Coord(x, -y, 0), color);
				Draw_Double_Selection_Bracket(center + Coord(-x, y, 0), center + Coord(-x, y, dim.Z), color);
				Draw_Double_Selection_Bracket(center + Coord(x, -y, 0), center + Coord(x, -y, dim.Z), color);
			}

			if (Strength > 0 && (House->Is_Ally(PlayerPtr) || Rule->IsHealthBar)) {
				Draw_Health_Bar_Old(point, cliprect);
			}

			if (RTTI != RTTI_INFANTRY) {
				Draw_Single_Selection_Bracket(center + Coord(x, y, 0), center + Coord(x, y, dim.Z), color);
				Draw_Single_Selection_Bracket(center + Coord(x, -y, dim.Z), center + Coord(x, y, dim.Z), color);
				Draw_Single_Selection_Bracket(center + Coord(-x, y, dim.Z), center + Coord(x, y, dim.Z), color);
			} else {
				Draw_Single_Selection_Bracket(center + Coord(-x, y, 0), center + Coord(x, y, 0), color);
				Draw_Single_Selection_Bracket(center + Coord(-x, y, 0), center + Coord(-x, y, dim.Z), color);
				Draw_Single_Selection_Bracket(center + Coord(x, -y, dim.Z), center + Coord(x, -y, 0), color);
				Draw_Single_Selection_Bracket(center + Coord(x, -y, dim.Z), center + Coord(-x, -y, dim.Z), color);
			}
		}

		Draw_Health_Bar(point, cliprect);
		if (allied) {
			Draw_Pips(Pip_Origin(point), point, cliprect);
		}

	} else {

		bool hovered = Map.HoverObject == this && Class_Of()->IsSelectable && !IsALoaner;

		if ((hovered || allied) && Is_Decoration_Visible()) {
			if (hovered) {
				Draw_Health_Bar(point, cliprect);
			}
			if (allied) {
				Draw_Insignia(Pip_Origin(point), point, cliprect);
			}
		}
	}

	if (TalkBubbleOwner == this && TalkBubbleType > 0 && (int)TalkBubbleTimer > 0) {
		Draw_Shape(*LogicalSurface, *NormalDrawer, (ShapeSet const *)ObjectTypeClass::TalkBubbleShapes, TalkBubbleType - 1, point, cliprect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL));
	}
}


/// <summary>
/// Draws one edge of a selection box as a pair of corner brackets.
/// Only the outer quarter at each end of the edge is drawn, so that a box assembled from
/// these is left open along its sides and does not fence in the object inside it.
/// </summary>
/// <param name="color">The color to draw the edge in. It must already be converted.</param>
void TechnoClass::Draw_Double_Selection_Bracket(Coord const & coord1, Coord const & coord2, int color)
{
	Coord c1 = (coord1 + coord1 + coord1 + coord2) / 4;
	Coord c2 = (coord2 + coord2 + coord2 + coord1) / 4;

	if (coord1.Z > c1.Z) {
		TacticalMap->Draw_3D_Line(coord1, c1, color, 0);
	} else {
		TacticalMap->Draw_3D_Line(c1, coord1, color, 0);
	}

	if (coord2.Z > c2.Z) {
		TacticalMap->Draw_3D_Line(coord2, c2, color, 0);
	} else {
		TacticalMap->Draw_3D_Line(c2, coord2, color, 0);
	}
}


/// <summary>
/// Static helper that draws only a single end-quarter tick of an edge (used for infantry).
/// Renders the outer one-quarter at the first endpoint only via the tactical map's 3D line routine.
/// </summary>
/// <param name="coord1">World coordinate of the endpoint where the tick is drawn.</param>
/// <param name="coord2">World coordinate of the far endpoint defining the edge direction.</param>
/// <param name="color">Converted pixel color used to draw the line.</param>
void TechnoClass::Draw_Single_Selection_Bracket(Coord const & coord1, Coord const & coord2, int color)
{
	Coord c1 = (coord1 + coord1 + coord1 + coord2) / 4;

	if (coord1.Z > c1.Z) {
		TacticalMap->Draw_3D_Line(coord1, c1, color, 0);
	} else {
		TacticalMap->Draw_3D_Line(c1, coord1, color, 0);
	}
}


/// <summary>
/// Draws the old style health bar.
/// Nothing is drawn. The routine survives only as the hook Draw_Post_Render still calls at
/// the point the older health bar used to occupy.
/// </summary>
void TechnoClass::Draw_Health_Bar_Old(Point2D const &, Rect const &) const
{
	// nothing
}


/// <summary>
/// Draws the rear portion of a selected building's three dimensional selection box.
/// Drawing these edges before the building body allows the body to sit between the rear
/// edges and the front edges drawn by Draw_Post_Render. The box turns yellow while a
/// limpet mine is attached and black while the building sits below ground.
/// </summary>
void TechnoClass::Draw_Pre_Render(Point2D const & point, Rect const & cliprect) const
{
	if (RTTI == RTTI_BUILDING && IsSelected && RTTI != RTTI_INFANTRY) {
		int color = WHITE;
		if (LimpetType > 0) {
			color = YELLOW;
		}
		if (HeightAGL < -4) {
			color = BLACK;
		}
		Coord dim = TClass->Lepton_Dimensions();
		dim.X /= 2;
		dim.Y /= 2;
		color = NormalDrawer->Convert_Pixel(color);
		Coord center = Center_Coord();
		Draw_Double_Selection_Bracket(center + Coord(-dim.X, -dim.Y, 0), center + Coord(-dim.X, -dim.Y, dim.Z), color);
		Draw_Double_Selection_Bracket(center + Coord(-dim.X, -dim.Y, 0), center + Coord(dim.X, -dim.Y, 0), color);
		Draw_Double_Selection_Bracket(center + Coord(-dim.X, -dim.Y, 0), center + Coord(-dim.X, dim.Y, 0), color);
		Draw_Double_Selection_Bracket(center + Coord(-dim.X, -dim.Y, dim.Z), center + Coord(-dim.X, dim.Y, dim.Z), color);
		Draw_Double_Selection_Bracket(center + Coord(-dim.X, -dim.Y, dim.Z), center + Coord(dim.X, -dim.Y, dim.Z), color);
	}
}


/// <summary>
/// Checks whether the player may be shown indicators drawn over this object.
/// This mirrors the entitlement the mouse applies when it resolves what it is pointing at, so
/// that an indicator drawn without a selection never reveals more than a selection would.
/// </summary>
bool TechnoClass::Is_Decoration_Visible(void) const
{
	if (!IsOwnedByPlayer) {
		if ((Cloak == CLOAKED && !Is_Sensed_By_Player()) || TClass->IsInvisible) {
			return(false);
		}
	}

	if (Map.Is_Shrouded(Center_Coord()) && MainWindow != NULL) {
		return(false);
	}

	if (RTTI == RTTI_BUILDING) {
		BuildingClass const * building = (BuildingClass const *)this;

		if (!IsOwnedByPlayer && ((building->TranslucencyLevel == 15 && !Is_Sensed_By_Player()) || building->Class->IsInvisibleInGame)) {
			return(false);
		}

		// Buildings reach the post render hook even while fogged, unlike the mobile objects the
		// renderer skips outright.
		if (Scen->Special.IsFogOfWar && building->IsFogged) {
			return(false);
		}
	}

	return(true);
}


/// <summary>
/// Returns the point this object's pips run from: the near corner of the footprint for a
/// building or core defender, and a fixed offset below the center for anything else.
/// </summary>
Point2D TechnoClass::Pip_Origin(Point2D const & point) const
{
	UnitClass const * unit = (RTTI == RTTI_UNIT) ? dynamic_cast<UnitClass const *>(this) : NULL;

	if (RTTI == RTTI_BUILDING || (unit != NULL && unit->Class->IsCoreDefender)) {
		Point3D dim = TClass->Lepton_Dimensions();
		Coord corner = dim - Point3D(dim.X / 2, dim.Y / 2, dim.Z / 2);
		corner.X = -corner.X;
		corner.Z = 0;
		return(point + TacticalMap->Coord_To_Pixel_Absolute(corner));
	}

	return(point + Point2D(-10, 10));
}


/// <summary>
/// Draws this object's condition indicator.
/// Buildings and core defenders get a pip bar laid along the near edge of their footprint;
/// everything else gets a row of health pips, framed by the familiar select box graphic
/// while the object is selected.
/// </summary>
void TechnoClass::Draw_Health_Bar(Point2D const & xpoint, Rect const & cliprect) const
{
	UnitClass const * unit = (RTTI == RTTI_UNIT) ? dynamic_cast<UnitClass const *>(this) : NULL;

	if (RTTI == RTTI_BUILDING || (unit != NULL && unit->Class->IsCoreDefender)) {

		/*
		 * Build the screen-space pip bar from the object's lepton dimensions.
		 */
		HeightAGL;

		Point3D dim = TClass->Lepton_Dimensions();

		Point3D half(dim.X / 2, dim.Y / 2, dim.Z / 2);

		/*
		 * The "dim - half" difference is computed for all three components and Z is
		 * then replaced with the full height, so the Z store below is dead.
		 */
		Coord coord = dim - half;
		coord.X = -coord.X;
		coord.Z = dim.Z;

		/*
		 * The X sign flips around the Y flips are redundant -- the paired negations
		 * are dead load/neg/neg/store sequences.
		 */
		Point2D p0 = TacticalMap->Coord_To_Pixel_Absolute(coord);
		coord.X = -coord.X;
		coord.Y = -coord.Y;
		coord.X = -coord.X;
		Point2D p1 = TacticalMap->Coord_To_Pixel_Absolute(coord);

		int barlen = (p0.Y - p1.Y) / 2;

		int n = (int)(HealthRatio * (double)barlen);
		if (n <= 1) {
			n = 1;
		}
		if (n >= barlen) {
			n = barlen;
		}

		int condcolor = 1;
		if (HealthRatio <= Rule->ConditionYellow) {
			condcolor = 2;
		}
		if (HealthRatio <= Rule->ConditionRed) {
			condcolor = 4;
		}

		int ybase = 2 - 2 * barlen;

		Point2D point;
		int index;

		int yoff = 0;
		int xoff = 0;
		for (index = 0; index < n; index++) {
			point.X = xpoint.X + p0.X + 4 * barlen + 3;
			point.Y = xpoint.Y + p0.Y + ybase + 2;
			point.X -= xoff;
			point.Y -= yoff;
			Draw_Shape(*LogicalSurface, *NormalDrawer, (ShapeSet const *)ObjectTypeClass::PipShapes, condcolor, point, cliprect, ShapeFlags_Type(SHAPE_WIN_REL|SHAPE_CENTER));
			xoff += 4;
			yoff -= 2;
		}

		yoff = -2 * n;
		xoff = 4 * n;
		for (index = n; index < barlen; index++) {
			point.X = xpoint.X + p0.X + 4 * barlen + 3;
			point.Y = xpoint.Y + p0.Y + ybase + 2;
			point.X -= xoff;
			point.Y -= yoff;
			Draw_Shape(*LogicalSurface, *NormalDrawer, (ShapeSet const *)ObjectTypeClass::PipShapes, 0, point, cliprect, ShapeFlags_Type(SHAPE_WIN_REL|SHAPE_CENTER));
			xoff += 4;
			yoff -= 2;
		}

	} else {

		bool powerup = false;
		if (ArmorBias > 1.0 || FirepowerBias > 1.0 || (Is_Foot() && ((FootClass *)this)->SpeedBias > 1.0)) {
			powerup = true;
		}

		Point2D offset;
		int health_bar_count;

		if (RTTI == RTTI_INFANTRY) {
			if (IsSelected) {
				Draw_Shape(*LogicalSurface, *NormalDrawer, (ShapeSet const *)ObjectTypeClass::SelectShapes, powerup ? 6 : 2, xpoint, cliprect, ShapeFlags_Type(SHAPE_ALPHA|SHAPE_WIN_REL|SHAPE_CENTER));
			}
			offset = Point2D(-5, -24);
			health_bar_count = 8;
		} else {
			if (IsSelected) {
				Draw_Shape(*LogicalSurface, *NormalDrawer, (ShapeSet const *)ObjectTypeClass::SelectShapes, (LimpetType != 0 ? 8 : 0) + (powerup ? 4 : 0) + 3, xpoint, cliprect, ShapeFlags_Type(SHAPE_ALPHA|SHAPE_WIN_REL|SHAPE_CENTER));
			}
			offset = Point2D(-15, -25);
			health_bar_count = 17;
		}

		int n = (int)(HealthRatio * (double)health_bar_count);
		if (n <= 1) {
			n = 1;
		}
		if (n >= health_bar_count) {
			n = health_bar_count;
		}

		int shapenum = 9;
		if (HealthRatio <= Rule->ConditionYellow) {
			shapenum = 10;
		}
		if (HealthRatio <= Rule->ConditionRed) {
			shapenum = 11;
		}

		Point2D point;
		for (int index = 0; index < n; index++) {
			point.X = xpoint.X + offset.X;
			point.Y = offset.Y + xpoint.Y;
			point.X += 2 * index;
			Draw_Shape(*LogicalSurface, *NormalDrawer, (ShapeSet const *)ObjectTypeClass::PipShapes, shapenum, point, cliprect, ShapeFlags_Type(SHAPE_WIN_REL|SHAPE_CENTER));
		}
	}
}


#if 0
/***********************************************************************************************
 * TechnoClass::Draw_It -- Draws the health bar (if necessary).                                *
 *                                                                                             *
 *    This routine will draw the common elements for techno type objects. This element is      *
 *    the health bar. The main game object has already been rendered by the time this          *
 *    routine is called.                                                                       *
 *                                                                                             *
 * INPUT:   x,y   -- The coordinate of the center of the unit.                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *   10/26/94   JLB : Knows about radar scanned cells.                                         *
 *   12/13/1994 JLB : Clips health bar against map edge.                                       *
 *   01/23/1995 JLB : Dynamic selected object rectangle.                                       *
 *=============================================================================================*/
void TechnoClass::Draw_It(int x, int y, int /*WindowNumberType*/ window) const
{
	//assert(IsActive);

	/*
	**	Tells the door logic that it has been drawn.
	*/
	((TechnoClass *)this)->Clear_Redraw_Flag();

	if (IsSelected) {
		GraphicViewPortClass draw_window(	LogicPage->Get_Graphic_Buffer(),
														WindowList[window][WINDOWX] + LogicPage->Get_XPos(),
														WindowList[window][WINDOWY] + LogicPage->Get_YPos(),
														WindowList[window][WINDOWWIDTH],
														WindowList[window][WINDOWHEIGHT]);


		/*
		**	The infantry select box should be a bit higher than normal.
		*/
		if (RTTI == RTTI_INFANTRY) {
			y -= 6;
		}

		if (RTTI == RTTI_BUILDING && ((BuildingTypeClass const &)Class_Of()).Type == STRUCT_BARRACKS) {
			y -= 5;
		}

		/*
		**	Fetch the dimensions of the object. These dimensions will be used to draw
		**	the selection box and the health bar.
		*/
		int width,height;
		Class_Of().Dimensions(width, height);

		if (Strength && (House->Is_Ally(PlayerPtr) || Rule.IsHealthBar)) {
			fixed	ratio = Health_Ratio();
			int	pwidth;		// Pixel width of bar interior.
			int	color;		// The color to give the interior of the bargraph.

			int xx = x-width/2;
			int yy = y-(height/2);

			/*
			**	Draw the outline of the bargraph.
			*/
			draw_window.Remap(xx+1, yy+1, width-1, 3-1, Map.FadingShade);
			draw_window.Draw_Rect(xx, yy, xx+width-1, yy+3, BLACK);

			/*
			**	Determine the width of the interior strength
			**	graph.
			*/
			pwidth = (width-2) * ratio;

			pwidth = std::clamp(pwidth, 1, width-2);

			color = LTGREEN;
			if (ratio <= Rule->ConditionYellow) {
				color = YELLOW;
			}
			if (ratio <= Rule->ConditionRed) {
				color = RED;
			}
			draw_window.Fill_Rect(xx+1, yy+1, xx+pwidth, yy+(3-1), color);
		}

		/*
		**	Draw the selected object graphic.
		*/
		if (IsSelected) {
			int lx = width/2;
			int ly = height/2;
			int dx = width/5;
			int dy = height/5;
			int fudge = (House->Is_Ally(PlayerPtr) || Rule.IsHealthBar) ? 4 : 0;
			if (What_Am_I() == RTTI_VESSEL) {
				lx = width / 2;
			}

			// Upper left corner.
			draw_window.Draw_Line(x-lx, fudge+y-ly, x-lx+dx, fudge+y-ly, WHITE);
			draw_window.Draw_Line(x-lx, fudge+y-ly, x-lx, fudge+y-ly+dy, WHITE);

			// Upper right corner.
			draw_window.Draw_Line(x+lx, fudge+y-ly, x+lx-dx, fudge+y-ly, WHITE);
			draw_window.Draw_Line(x+lx, fudge+y-ly, x+lx, fudge+y-ly+dy, WHITE);

			// Lower right corner.
			draw_window.Draw_Line(x+lx, y+ly, x+lx-dx, y+ly, WHITE);
			draw_window.Draw_Line(x+lx, y+ly, x+lx, y+ly-dy, WHITE);

			// Lower left corner.
			draw_window.Draw_Line(x-lx, y+ly, x-lx+dx, y+ly, WHITE);
			draw_window.Draw_Line(x-lx, y+ly, x-lx, y+ly-dy, WHITE);
			if (House->Is_Ally(PlayerPtr) || (SpiedBy & (1<<(PlayerPtr->Class->House)))) {
				Draw_Pips((x-lx)+5, y+ly-3, window);
			}
		}
	}
}
#endif


/// <summary>
/// Performs the limbo process for a techno object.
/// This routine handles the bookkeeping an owned object must undo as it leaves the map. It
/// drops out of its house's active tracking, hands back the threat it was contributing to
/// the cell it occupied, and stops being tracked on radar.
/// </summary>
/// <returns>bool; Was the object limboed?</returns>
bool TechnoClass::Limbo(void)
{
	if (IsRadarTracked) {
		Radar_Untrack();
	}

	if (!IsInLimbo) {
		House->Tracking_Active_Remove(this, false);
		int risk = Risk();
		HousesType owner = Owner();
		if (risk > 0) {
			FootClass * foot = dynamic_cast<FootClass *>(this);
			if (foot) {
				Map[foot->LastAdjacencyCell].Adjust_Threat(owner, -risk);
			} else {
				Get_Cell_Ptr()->Adjust_Threat(owner, -risk);
			}
		}
	}
	return(BASECLASS::Limbo());
}


/***********************************************************************************************
 * TechnoClass::Unlimbo -- Performs unlimbo process for all techno type objects.               *
 *                                                                                             *
 *    This routine handles the common operation between techno objects when they are           *
 *    unlimboed. This includes revealing the map.                                              *
 *                                                                                             *
 * INPUT:   coord    -- The coordinate to unlimbo object at.                                   *
 *                                                                                             *
 *          dir (optional) -- initial facing direction for this object                         *
 *                                                                                             *
 * OUTPUT:  bool; Was the unlimbo successful?                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/14/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoClass::Unlimbo(Coord const & coord, Dir256 dir)
{
	IsLocked = Map.In_Local_Radar(coord.As_Cell());

	if (BASECLASS::Unlimbo(coord, dir)) {

		if (!IsActive) {
			return(true);
		}

		House->Tracking_Active_Add(this, false);
		PrimaryFacing.Set(dir);
		BarrelPitch.Set(DIR_E << 8);
		Enter_Idle_Mode(true);
		if (Ready_To_Commence()) {
			Commence();
		}

		SightIncrease = Get_Sight_Bonus(coord);

		if (!Map.In_Local_Radar(coord.As_Cell())) {
			IsDiscoveredByPlayer = false;
		}

		int risk = Risk();
		HousesType owner = Owner();
		Get_Cell_Ptr()->Adjust_Threat(owner, risk);

		RadarPos = Map.Coord_To_Radar_Pixel(PositionCoord, true);

		if (ActLike == HOUSE_NONE) {
			ActLike = House->ActLike;
		}

		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the extra sight range earned by height.
/// An object raised off the ground can see further than one on it. This routine works out
/// how much further, and is used when the object is unlimboed onto the map.
/// </summary>
/// <param name="coord">The coordinate whose height the bonus is derived from.</param>
/// <returns>Returns with the amount to add to this object's sight range.</returns>
int TechnoClass::Get_Sight_Bonus(Coord const & coord)
{
	return(10 * (coord.Z / Rule->LeptonsPerSightIncrease));
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
bool TechnoClass::In_Range(AbstractClass * target, int which) const
{
	if (target == NULL) {
		return(true);
	}

	Coord coord = Center_Coord();
	if (In_Air()) {
		coord.Z = target->Center_Coord().Z;
	}

	return(TClass->In_Range(coord, target, Get_Class_Weapon_Data(which)->Weapon));
}


/***********************************************************************************************
 * TechnoClass::In_Range -- Determines if the specified coordinate is within range.            *
 *                                                                                             *
 *    Use this routine to determine if the specified coordinate is within weapon range.        *
 *                                                                                             *
 * INPUT:   coord    -- The coordinate to examine against the object to determine range.       *
 *                                                                                             *
 *          which    -- The weapon to consider when determining range. 0=primary, 1=secondary. *
 *                                                                                             *
 * OUTPUT:  bool; Is the weapon within range?                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/16/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoClass::In_Range(Coord const & coord, int which) const
{
	return(In_Range(&Map[coord], which));
}


/***********************************************************************************************
 * TechnoClass::Area_Modify -- Determine the area scan modifier for the cell.                  *
 *                                                                                             *
 *    This routine scans around the cell specified and if there are any friendly buildings     *
 *    nearby, the multiplier return value will be reduced. If there are no friendly buildings  *
 *    nearby, then the return value will be 1. It checks to see if the primary weapon is       *
 *    supposed to perform this scan and if so, the scan will be performed. Otherwise the       *
 *    default value is quickly returned.                                                       *
 *                                                                                             *
 * INPUT:   cell  -- The cell where the potential target lies. An area around this cell will   *
 *                   be scanned for friendly buildings.                                        *
 *                                                                                             *
 * OUTPUT:  Returns with the multiplier to be multiplied by the potential target score value.  *
 *          For less opportune targets, the multiplier fraction will be less than one. For     *
 *          all other cases, it will return the default value of 1.                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/23/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
double TechnoClass::Area_Modify(Cell const & cell) const
{
//	assert(PrimaryWeapon != NULL);
	if (PrimaryWeapon == NULL || !PrimaryWeapon->IsSupressed) return(1);

	int crange = Rule->SupressRadius / CELL_LEPTON;
	double odds = 1;

	for (int radius = 1; radius < crange; radius++) {

		/*
		**	Scan the top and bottom rows of the "box".
		*/
		for (int x = -radius; x <= radius; x++) {
			Cell newcell;

			if (cell.X + x < Map.MapRect.X) continue;
			if (cell.X + x >= (Map.MapRect.X+Map.MapRect.Width)) continue;

			if ((cell.Y - radius) >= Map.MapRect.Y) {
				newcell = cell + Cell(x, -radius);
				BuildingClass const * building = Map[newcell].Cell_Building();
				if (building != NULL && House->Is_Ally(building)) {
					odds /= 2;
				}
			}

			if ((cell.Y + radius) < (Map.MapRect.Y+Map.MapRect.Height)) {
				newcell = cell + Cell(x, radius);
				BuildingClass const * building = Map[newcell].Cell_Building();
				if (building != NULL && House->Is_Ally(building)) {
					odds /= 2;
				}
			}
		}

		/*
		**	Scan the left and right columns of the "box".
		*/
		for (int y = -(radius-1); y < radius; y++) {
			Cell newcell;

			if ((cell.Y + y) < Map.MapRect.Y) continue;
			if ((cell.Y + y) >= (Map.MapRect.Y+Map.MapRect.Height)) continue;

			if ((cell.X - radius) >= Map.MapRect.X) {
				newcell = cell + Cell(-radius, y);
				BuildingClass const * building = Map[newcell].Cell_Building();
				if (building != NULL && House->Is_Ally(building)) {
					odds /= 2;
				}
			}

			if ((cell.X + radius) < (Map.MapRect.X+Map.MapRect.Width)) {
				newcell = cell + Cell(radius, y);
				BuildingClass const * building = Map[newcell].Cell_Building();
				if (building != NULL && House->Is_Ally(building)) {
					odds /= 2;
				}
			}
		}
	}
	return(odds);
}


/***********************************************************************************************
 * TechnoClass::Evaluate_Object -- Determines score value of specified object.                 *
 *                                                                                             *
 *    This routine is used to determine the score value (value as a potential target) of the   *
 *    object specified. This routine will check the specified object for all the various       *
 *    legality checks that threat scanning requires. This is the main workhorse routine for    *
 *    target searching.                                                                        *
 *                                                                                             *
 * INPUT:   method   -- The threat method requested. This is a combined bitflag value that     *
 *                      not only specifies the kind of targets to consider, but how far away   *
 *                      they are allowed to be.                                                *
 *                                                                                             *
 *          mask     -- This is an RTTI mask to use for quickly eliminating object types.      *
 *                      The mask is created outside of this routine because this routine is    *
 *                      usually called from within a loop and this value is constant in that   *
 *                      context.                                                               *
 *                                                                                             *
 *          range    -- The range at which potential target objects are rejected.              *
 *                      0  = must be within weapon range.                                      *
 *                      >0 = must be within this lepton distance.                              *
 *                      <0 = range doesn't matter.                                             *
 *                                                                                             *
 *          object   -- Pointer to the object itself.                                          *
 *                                                                                             *
 *          value    -- Reference to the value variable that this routine will fill in. The    *
 *                      higher the value the more likely this object will be selected as best. *
 *                                                                                             *
 *          zone     -- The zone restriction if any. A -1 means no zone check required.        *
 *                                                                                             *
 * OUTPUT:  Did the target pass all legality checks? If this value is returned true, then the  *
 *          value parameter will be filled in correctly.                                       *
 *                                                                                             *
 * WARNINGS:   This routine is time consuming. Don't call unless necessary.                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/30/1995 JLB : Created.                                                                 *
 *   07/14/1995 JLB : Forces SAM site to not fire on landed aircraft.                          *
 *   09/22/1995 JLB : Zone checking enabled.                                                   *
 *   10/05/1995 JLB : Gives greater weight to designated enemy house targets.                  *
 *   02/16/1996 JLB : Added additional threat checks.                                          *
 *=============================================================================================*/
bool TechnoClass::Evaluate_Object(ThreatType method, int mask, int range, TechnoClass const * object, int & value, int zone, Coord const & coord) const
{
	assert(object != NULL);

	BStart(BENCH_EVAL_OBJECT);

	bool engineer = Is_Renovator();

	/*
	**	An object in limbo can never be a valid target.
	*/
	if (object == NULL || object->IsInLimbo || object->Strength == 0) {
		BEnd(BENCH_EVAL_OBJECT);
		return(false);
	}

	/*
	**	If the object is cloaked, then it isn't a legal target.
	*/
	if (object->Cloak == CLOAKED) {
		if (!Map[object->Center_Coord()].Is_Sensed(House->HeapID) && House != object->House) {
			BEnd(BENCH_EVAL_OBJECT);
			return(false);
		}
	}

	if (!object->IsLocked) {
		BEnd(BENCH_EVAL_OBJECT);
		return(false);
	}

	/*
	**	If the object is in a "harmless" state, then don't bother to consider it
	**	a threat.
	*/
	if (object->Current_Mission_Control().IsNoThreat) {
		BEnd(BENCH_EVAL_OBJECT);
		return(false);
	}

	if (object->HeightAGL < -20) {
		BEnd(BENCH_EVAL_OBJECT);
		return(false);
	}

	/*
	**	If the object is not within the desired zone, then ignore it, but only if
	**	zone checking is desired.
	*/
	Coord objectcoord = object->Center_Coord();
	if (zone != -1 && Map.Get_Cell_Zone(objectcoord.As_Cell(), TClass->MZone, object->IsOnBridge) != zone) {
		BEnd(BENCH_EVAL_OBJECT);
		return(false);
	}

	/*
	**	Friendly units are never considered a good target. Bail if this
	**	object is a friend.  Unless we're a medic, of course.  But then,
	**	only consider it a target if it's injured.
	*/
	if ((RTTI != RTTI_INFANTRY || !((InfantryClass *)this)->IsBerzerk) && House->Is_Ally(object)) {
		if (Combat_Damage() < 0 || engineer) {
			if (object->HealthRatio == Rule->ConditionGreen) {
				BEnd(BENCH_EVAL_OBJECT);
				return(false);
			}
			if (object->RTTI == RTTI_AIRCRAFT && RTTI == RTTI_UNIT) {
				if (object->HeightAGL > 0) {
					BEnd(BENCH_EVAL_OBJECT);
					return(false);
				}
				if (Map[object->Center_Coord()].Cell_Building()) {
					BEnd(BENCH_EVAL_OBJECT);
					return(false);
				}
			} else if (RTTI == RTTI_UNIT && !object->Considered_Vehicle()) {
				BEnd(BENCH_EVAL_OBJECT);
				return(false);
			}
		} else {
			BEnd(BENCH_EVAL_OBJECT);
			return(false);
		}
	}

	if (Scen->Special.IsHarvesterImmune) {
		if (Rule->HarvesterUnit.Is_In_List((UnitTypeClass const *)object->Class_Of())) {
			BEnd(BENCH_EVAL_OBJECT);
			return(false);
		}
	}

	/*
	**	If the object is further away than allowed, bail.
	*/
	int dist = Distance_To(object);
	if (range > 0 && dist > range) {
		BEnd(BENCH_EVAL_OBJECT);
		return(false);
	}

	if (range == 0) {
		if (!Is_Weapon_Equipped()) {
			if (dist > TClass->ThreatRange) {
				BEnd(BENCH_EVAL_OBJECT);
				return(false);
			}
		} else {
			int primary = What_Weapon_Should_I_Use((AbstractClass *)object);
			if (!In_Range((TechnoClass *)object, primary)) {
				BEnd(BENCH_EVAL_OBJECT);
				return(false);
			}
		}
	}

	/*
	**	If the object is not visible, then bail. Human controlled units
	**	are always considered to be visible.
	*/
	if (House->Is_Player_Control() && !object->IsOwnedByPlayer && !object->IsDiscoveredByPlayer && Session.Type == GAME_NORMAL && object->RTTI != RTTI_AIRCRAFT) {
		BEnd(BENCH_EVAL_OBJECT);
		return(false);
	}

	if (object->RTTI == RTTI_BUILDING && ((BuildingClass *)object)->Class->IsInvisibleInGame) {
		BEnd(BENCH_EVAL_OBJECT);
		return(false);
	}

	/*
	**	Quickly eliminate all unit types that are not allowed according to the mask
	**	value.
	*/
	RTTIType otype = object->RTTI;
	if (!((1 << otype) & mask) && (!(mask & 2) || !object->Considered_Vehicle())) {
		BEnd(BENCH_EVAL_OBJECT);
		return(false);		// Mask failure.
	}

	if (Session.Type != GAME_NORMAL && object->House->Class->IsMultiplayPassive) {
		BEnd(BENCH_EVAL_OBJECT);
		return(false);
	}

	/*
	**	Determine if the target is theoretically allowed to be a target. If
	**	not, then bail.
	*/
	TechnoTypeClass const * tclass = object->TClass;
	if (!tclass->IsLegalTarget) {
		BEnd(BENCH_EVAL_OBJECT);
		return(false);		// Legality failure.
	}

	if (tclass->IsTrain && RTTI == RTTI_INFANTRY && ((InfantryClass *)this)->Class->IsVehicleThief) {
		BEnd(BENCH_EVAL_OBJECT);
		return(false);
	}

	// A disguise defeats the scan unless this type, or the rules for a computer house, see through it.
	if (otype == RTTI_INFANTRY && ((InfantryTypeClass const *)tclass)->IsDisguised) {
		if (!Techno_Type_Class()->IsDetectDisguise && (!Rule->AIDetectDisguise || House->Is_Human_Player())) {
		BEnd(BENCH_EVAL_OBJECT);
		return(false);
	}
	}

	/*
	**	Special case so that SAM site doesn't fire on aircraft that are landed.
	*/
	if (PrimaryWeapon != NULL && !PrimaryWeapon->Bullet->IsAntiGround){
		if (((AircraftClass *)object)->Height == 0) {
			BEnd(BENCH_EVAL_OBJECT);
			return(false);
		}
	}

	/*
	**	If only allowed to attack civilians, then eliminate all other types.
	*/
	if (method & THREAT_CIVILIANS) {
		object->Owner_HouseClass(); /// The result is unused.
		BEnd(BENCH_EVAL_OBJECT);
		return(false);
	}

	/*
	**	If the scan is limited to capturable buildings only, then bail if the examined
	**	object isn't a capturable building.
	*/
	if ((method & THREAT_CAPTURE) && (otype != RTTI_BUILDING || !((BuildingTypeClass const *)tclass)->IsCaptureable)) {
		BEnd(BENCH_EVAL_OBJECT);
		return(false);
	}

	/*
	**	SPECIAL CASE: Friendly units won't automatically fire on buildings
	**	if the building is not aggressive. That is, unless it is part of a team. A team
	**	is allowed to pick any target it so chooses.
	*/
	if ((!Is_Foot() || !((FootClass *)this)->Team != NULL) &&
			House->Is_Human_Player() && !object->Considered_Vehicle() &&
			otype == RTTI_BUILDING && object->PrimaryWeapon == NULL) {

		if (!engineer) {
			BEnd(BENCH_EVAL_OBJECT);
			return(false);
		}
	}

	if (engineer) {
		if (object->RTTI != RTTI_BUILDING || (House->Is_Ally(object->House) && (object->HealthRatio > Rule->ConditionRed || !((BuildingClass *)object)->Class->Cost_Of(House)))) {
			BEnd(BENCH_EVAL_OBJECT);
			return(false);
		}
	}

	/*
	**	If the search is restricted to Tiberium processing objects, then
	**	perform the special qualification check now.
	*/
	if (method & THREAT_TIBERIUM) {
		if (tclass->Capacity == 0) {
			BEnd(BENCH_EVAL_OBJECT);
			return(false);
		}
	}

	CellClass * cell_this = &Map[Get_Coord()];
	CellClass * cell_that = &Map[object->Get_Coord()];
	if (cell_this->IsUnderBridge && cell_that->IsUnderBridge && IsOnBridge != object->IsOnBridge) {
		BEnd(BENCH_EVAL_OBJECT);
		return(false);
	}

	bool webbysecondary = false;
	WarheadTypeClass const * wh = NULL;
	WeaponTypeClass const * secondary = SecondaryWeapon;
	if (secondary != NULL) {
		wh = secondary->WarheadPtr;
		if (wh != NULL && wh->IsWebby) {
			webbysecondary = true;
		}
	}

	if (!webbysecondary) {
		WeaponTypeClass const * primary = PrimaryWeapon;
		if (primary != NULL) {
			wh = primary->WarheadPtr;
		} else {
			wh = NULL;
		}
	}

	if (wh && wh->IsWebby) {
		if (object->RTTI == RTTI_INFANTRY) {
			InfantryClass const * infantry = dynamic_cast<const InfantryClass *>(object);
			if (infantry != NULL && infantry->ProneStruggleTimer > wh->WebDuration / 4) {
				BEnd(BENCH_EVAL_OBJECT);
				return(false);
			}
		}
	}


	/*
	**	If this target value is better than the previously recorded best
	**	target value then record this target for possible return as the
	**	best.
	*/
	value = Target_Threat((TechnoClass *)object, coord);

	/*
	**	If the candidate object is owned by the designated enemy of this house, then
	**	give it a higher value. This will tend to gravitate attacks toward the main
	**	antagonist of this house.
	*/
	if (House->IsAllToHunt && House->Enemy != HOUSE_NONE && object->House != Houses[House->Enemy]) {
		value = 1;
	}

	/*
	**	If power plants are to be considered a greater threat, then increase
	**	their value here. Buildings that produce no power are not considered
	**	a threat.
	*/
	if ((method & THREAT_POWER) && otype == RTTI_BUILDING) {
		if (((BuildingTypeClass const *)tclass)->Power > 0) {
			value += ((BuildingTypeClass const *)tclass)->Power * 1000;
		} else {
			value = 0;
		}
	}

	/*
	**	If factories are to be considered a greater threat, then don't
	**	consider any non-factory building.
	*/
	if ((method & THREAT_FACTORIES) && otype == RTTI_BUILDING) {
		if (((BuildingTypeClass const *)tclass)->ToBuild == RTTI_NONE) {
			value = 0;
		}
	}

	/*
	**	If base defensive structures are to be considered a greater threat, then
	**	don't consider an unarmed building to be a threat.
	*/
	if (method & THREAT_BASE_DEFENSE) {
		if (object->PrimaryWeapon == NULL) {
			value = 0;
		}
	}

	/*
	**	Possibly cause a reduction of the target's value if it is nearby friendly
	**	structures and the primary weapon of this object is flagged for
	**	friendly fire supression special check logic.
	*/
	double areamod = Area_Modify(object->Center_Coord().As_Cell());
	if (areamod != 1) {
		value = areamod * value;
	}

	/*
	**	Lessen threat as a factor of distance.
	*/
	if (value) {
		value = std::max(1, value);
		BEnd(BENCH_EVAL_OBJECT);
		return(true);
	}
	value = 0;
	BEnd(BENCH_EVAL_OBJECT);
	return(false);
}


/***********************************************************************************************
 * TechnoClass::Evaluate_Cell -- Determine the value and object of specified cell.             *
 *                                                                                             *
 *    This routine will examine the specified cell and return with the potential target        *
 *    object it contains and the value of it. Use this routine when searching for threats.     *
 *                                                                                             *
 * INPUT:   method   -- The scan method to use for target searching.                           *
 *                                                                                             *
 *          mask     -- Prebuilt mask of object RTTI types acceptable for scanning.            *
 *                                                                                             *
 *          range    -- Scan range limit to use for elimination purposes. This ensures that    *
 *                      objects in the "corner" of a square scan get properly discarded.       *
 *                                                                                             *
 *          object   -- Pointer to object pointer to be filled in with the object at this      *
 *                      cell as a valid target.                                                *
 *                                                                                             *
 *          value    -- Reference to the value of the object in this cell. It will be set      *
 *                      according to the object's value.                                       *
 *                                                                                             *
 *          zone     -- The zone restriction if any. A -1 means no zone check required.        *
 *                                                                                             *
 * OUTPUT:  Was a valid potential target found in this cell?                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *   09/22/1995 JLB : Zone checking enabled.                                                   *
 *=============================================================================================*/
bool TechnoClass::Evaluate_Cell(ThreatType method, int mask, Cell const & cell, int range, TechnoClass const * * object, int & value, int zone) const
{
	BStart(BENCH_EVAL_CELL);

	*object = NULL;
	value = 0;

	/*
	**	Fetch the techno object from the cell. If there is no
	**	techno object there, then bail.
	*/
	CellClass * cellptr = &Map[cell];

	/*
	**	Don't consider for evaluation a cell that is not within the same zone. Only
	**	perform this check if zone checking is required.
	*/
	if (zone != -1 && Map.Get_Cell_Zone(cell, TClass->MZone, true) != zone) {
		BEnd(BENCH_EVAL_CELL);
		return(false);
	}

	ObjectClass const * tentative = cellptr->Cell_Occupier(true);
	TechnoClass * tech = NULL;
	if (tentative == NULL) {
		tentative = cellptr->Cell_Occupier(false);
	}

	while (tentative != NULL) {
		if (tentative != this) {
			tech = Dynamic_Cast<TechnoClass *>((ObjectClass *)tentative);
			if (tech) {
				if (Combat_Damage() < 0) {
					if (tech->HealthRatio < Rule->ConditionGreen && House->Is_Ally(tech)) break;
				} else {
					if (!House->Is_Ally(tech)
						|| (RTTI == RTTI_INFANTRY
							&& (((InfantryClass*)this)->IsBerzerk
							|| (((InfantryClass*)this)->Class->IsEngineer
								&& CurrentMission == MISSION_GUARD_AREA
								&& tech->HealthRatio <= Rule->ConditionRed
								&& tech->RTTI == RTTI_BUILDING
								&& ((BuildingClass*)tech)->Class->Cost_Of(tech->House) > 0)))) {

						break;
					}
				}
			}
		}
		tentative = tentative->Next;
	}

	if (tech == NULL) {
		BEnd(BENCH_EVAL_CELL);
		return(false);
	}
	*object = tech;

	bool result = Evaluate_Object(method, mask, range, tech, value);

	BEnd(BENCH_EVAL_CELL);
	return(result);
}


/***********************************************************************************************
 * TechnoClass::Evaluate_Just_Cell -- Evaluate a cell as a target by itself.                   *
 *                                                                                             *
 *    This will examine the cell (as if it contained no sentient objects) and determine a      *
 *    target value to assign to it. Typically, this is only useful for wall destroyable        *
 *    weapons when dealing with enemy walls.                                                   *
 *                                                                                             *
 * INPUT:   cell  -- The cell to examine and evaluate.                                         *
 *                                                                                             *
 * OUTPUT:  Returns with the target value to assign to this cell.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/10/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoClass::Evaluate_Just_Cell(Cell const & cell) const
{
	BStart(BENCH_EVAL_WALL);

	/*
	**	First, only computer objects are allowed to automatically scan for walls.
	*/
	if (House->Is_Human_Player()) {
		BEnd(BENCH_EVAL_WALL);
		return(0);
	}

	/*
	**	Even then, if the difficulty indicates that it shouldn't search for wall
	**	targets, then don't allow it to do so.
	*/
	if (!Rule->Diff[House->Difficulty].IsWallDestroyer) {
		BEnd(BENCH_EVAL_WALL);
		return(0);
	}

	/*
	**	Determine if, in fact, a wall is located at this cell location.
	*/
	CellClass const * cellptr = &Map[cell];
	if (cellptr->Overlay == OVERLAY_NONE || !OverlayTypes[cellptr->Overlay]->IsWall) {
		BEnd(BENCH_EVAL_WALL);
		return(0);
	}

	/*
	**	As a convenience to the target scanning logic, don't consider any wall to be
	**	a target if it isn't in range of the primary weapon.
	*/
	int primary = What_Weapon_Should_I_Use(&Map[cell]);
	if (!In_Range(cell, primary)) {
		BEnd(BENCH_EVAL_WALL);
		return(0);
	}

	/*
	**	See if the object has a weapon that can damage walls.
	*/
	if (PrimaryWeapon == NULL || PrimaryWeapon->WarheadPtr == NULL) {
		BEnd(BENCH_EVAL_WALL);
		return(0);
	}

	/*
	**	If the weapon cannot deal with ground based targets, then don't consider
	**	this a valid cell target.
	*/
	if (PrimaryWeapon->Bullet != NULL && !PrimaryWeapon->Bullet->IsAntiGround) {
		BEnd(BENCH_EVAL_WALL);
		return(0);
	}

	/*
	**	If the primary weapon cannot destroy a wall, then don't give the cell any
	**	value as a target.
	*/
	if (!PrimaryWeapon->WarheadPtr->IsWallDestroyer) {
		BEnd(BENCH_EVAL_WALL);
		return(0);
	}

	/*
	**	If this is a friendly wall, then don't attack it.
	*/
	if (cellptr->Owner == HOUSE_NONE || House->Is_Ally(Houses[cellptr->Owner])) {
		BEnd(BENCH_EVAL_WALL);
		return(0);
	}

	/*
	**	Since a wall was found, then return a value adjusted according to the range the wall
	**	is from the object. The greater the range, the lesser the value returned.
	*/
	BEnd(BENCH_EVAL_WALL);
	return(Weapon_Range(0) - Distance(cell));
}


/***********************************************************************************************
 * TechnoClass::Greatest_Threat -- Determines best target given search criteria.               *
 *                                                                                             *
 *    This routine will scan game objects looking for the best target. It is used by the       *
 *    general target searching processes. The type of target scan to perform is controlled     *
 *    by the method control parameter.                                                         *
 *                                                                                             *
 * INPUT:   method   -- The method control parameter is used to control the type of target     *
 *                      scan performed. It consists of a series of bit flags (see ThreatType)  *
 *                      that are combined to form the target scan desired.                     *
 *                                                                                             *
 * OUTPUT:  Returns the target value of a suitable target. If no target was found then the     *
 *          value TARGET_NONE is returned.                                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/14/1994 JLB : Created.                                                                 *
 *   06/20/1995 JLB : Greatly optimized scan method.                                           *
 *   09/22/1995 JLB : Takes into account the zone (if necessary).                              *
 *   05/30/1996 JLB : Tighter elimination mask checking.                                       *
 *=============================================================================================*/
AbstractClass * TechnoClass::Greatest_Threat(ThreatType method, Coord const & coord, bool onlyenemy) const
{
	BStart(BENCH_GREATEST_THREAT);

	ObjectClass const * bestobject = NULL;
	int bestval = -1;
	int zone = -1;

	TargetScan++;

	if (TClass->IsNoAutoFire && House->Is_Human_Player()) {
		BEnd(BENCH_GREATEST_THREAT);
		return(NULL);
	}

	/*
	**	Determine the zone that the target must be in. For aircraft and gunboats, they
	**	ignore zones since they either can fly over any zone or are designed to fire into
	**	other zones. If scanning for targets that are within range, then zone checking need
	**	not be performed -- range checking is much more thorough and effective.
	*/
	if (!(method & THREAT_RANGE) &&
		RTTI != RTTI_BUILDING &&
		RTTI != RTTI_AIRCRAFT) {

		zone = Map.Get_Cell_Zone(Center_Coord().As_Cell(), TClass->MZone, true);
	}

	/*
	**	Hack for dogs, 'cause they can only consider infantrymen to be a
	**	threat.  Medics also.
	*/
	if (RTTI == RTTI_INFANTRY) {
		if (Combat_Damage() < 0) {
			method = ThreatType(THREAT_INFANTRY|THREAT_ALLIES|(method & (THREAT_RANGE|THREAT_AREA)));
		} else if (((InfantryClass *)this)->Class->IsEngineer) {
			method = ThreatType(method & ~(THREAT_INFANTRY|THREAT_VEHICLES));
		}
	} else if (RTTI == RTTI_UNIT) {
		if (Combat_Damage() < 0) {
			method = ThreatType(THREAT_VEHICLES|THREAT_ALLIES|(method & (THREAT_RANGE|THREAT_AREA)));
		}
	}

	/*
	**	Build a quick elimination mask. If the RTTI of the object doesn't
	**	qualify with this mask, then we KNOW that it shouldn't be considered.
	*/
	int mask = 0;
	if (method & THREAT_CIVILIANS) mask |= ((1 << RTTI_BUILDING)|(1 << RTTI_INFANTRY)|(1 << RTTI_UNIT));
	if (method & THREAT_AIR) mask |= (1 << RTTI_AIRCRAFT);
	if (method & (THREAT_BUILDINGS|THREAT_FACTORIES|THREAT_POWER|THREAT_BASE_DEFENSE|THREAT_TIBERIUM|THREAT_CAPTURE)) mask |= (1 << RTTI_BUILDING);
	if (method & THREAT_INFANTRY) mask |= (1 << RTTI_INFANTRY);
	if (method & (THREAT_VEHICLES|THREAT_TIBERIUM)) mask |= (1 << RTTI_UNIT);

	/*
	**	Limit area target scans use a method where the actual map cells are
	**	examined for occupants. The occupant is then examined in turn. The
	**	best target within the area is returned as a target.
	*/
	if (method & (THREAT_AREA|THREAT_RANGE)) {
		int range = 0;
		if (method & THREAT_RANGE) {
			range = Threat_Range(0);
		} else {
			if (method & THREAT_AREA) {
				if (CurrentMission == MISSION_PATROL) {
					range = Threat_Range(2);
				} else {
					range = Threat_Range(1);
				}
			}
		}


		/*
		**	BG: Miserable hack to get the stupid doctor to actually do area
		**	guarding.
		*/
		if (Combat_Damage() < 0 && CurrentMission == MISSION_GUARD) {
			range = CELL_LEPTON * 2;
		}

		int crange = range / CELL_LEPTON;
		if (range == 0) {
			crange = std::max(Weapon_Range(0), Weapon_Range(1)) / CELL_LEPTON;
			crange++;
		}
		Cell cell = coord.As_Cell();

		/*
		**	If aircraft are a legal target, then scan through all of them at this time.
		**	Scanning by cell is not possible for aircraft since they are not recorded
		**	at the cell level.
		*/
		if (method & THREAT_AIR) {
			int _mask = mask|(1 << RTTI_INFANTRY);
			int value;
			int index;
			for (index = 0; index < Map.Layer[LAYER_TOP].Count(); index++) {
				TechnoClass * object = (TechnoClass *)Map.Layer[LAYER_TOP][index];

				if (object->RTTI == RTTI_AIRCRAFT || (object->RTTI == RTTI_INFANTRY && ((InfantryClass *)object)->Class->IsJumpJet)) {
					value = 0;
					if ((!(method & THREAT_ALLIES) || House->Is_Ally(object->House)) && object->IsDown &&
						object->In_Which_Layer() != LAYER_GROUND && (!onlyenemy || object->House->HeapID == House->Enemy) &&
						Evaluate_Object(method, _mask, range, object, value)) {

						if (value > bestval) {
							bestobject = object;
							bestval = value;
						}
					}
				}
			}

			for (index = 0; index < Map.Layer[LAYER_AIR].Count(); index++) {
				TechnoClass * object = (TechnoClass *)Map.Layer[LAYER_AIR][index];

				if (object->RTTI == RTTI_INFANTRY) {
					value = 0;
					if ((!onlyenemy || object->House->HeapID == House->Enemy) &&
						(!(method & THREAT_ALLIES) || House->Is_Ally(object->House)) && object->IsDown &&
						object->In_Which_Layer() != LAYER_GROUND &&
						Evaluate_Object(method, _mask, range, object, value)) {

						if (value > bestval) {
							bestobject = object;
							bestval = value;
						}
					}
				}
			}
		}

		/*
		**	When scanning the ground, always consider landed aircraft as a valid
		**	potential target. This is only true if vehicles are considered a
		**	valid target. A landed aircraft is considered a vehicle.
		*/
		if (method & THREAT_VEHICLES) {
			mask |= (1 << RTTI_AIRCRAFT);
		}

		/*
		**	Radiate outward from the object's location, looking for the best
		**	target.
		*/
		Cell bestcell(0, 0);
		int bestcellvalue = 0;
		TechnoClass const * object;
		int value;

		for (int radius = 0; radius < crange; radius++) {

			/*
			**	Scan the top and bottom rows of the "box".
			*/
			for (int x = -radius; x <= radius; x++) {
				Cell newcell = cell + Cell(x, -radius);
				if (Map.In_Radar(newcell)) {
					if (Evaluate_Cell(method, mask, newcell, range, &object, value, zone)) {
						if (object != NULL && (!onlyenemy || object->House->HeapID == House->Enemy) &&
						(!(method & THREAT_ALLIES) || House->Is_Ally(object->House))) {
							if (bestval < value) {
								bestobject = object;
							}
						}
					}
					if (bestobject == NULL) {
						value = Evaluate_Just_Cell(newcell);
						if (bestcellvalue < value) {
							bestcellvalue = value;
							bestcell = newcell;
						}
					}
				}

				newcell = cell + Cell(x, radius);
				if (Map.In_Radar(newcell)) {
					if (Evaluate_Cell(method, mask, newcell, range, &object, value, zone)) {
						if (object != NULL && (!onlyenemy || object->House->HeapID == House->Enemy) &&
						(!(method & THREAT_ALLIES) || House->Is_Ally(object->House))) {
							if (bestval < value) {
								bestobject = object;
							}
						}
					}
					if (bestobject == NULL) {
						value = Evaluate_Just_Cell(newcell);
						if (bestcellvalue < value) {
							bestcellvalue = value;
							bestcell = newcell;
						}
					}
				}
			}

			/*
			**	Scan the left and right columns of the "box".
			*/
			for (int y = -(radius-1); y < radius; y++) {
				Cell newcell = cell + Cell(-radius, y);

				if (Map.In_Radar(newcell)) {
					if (Evaluate_Cell(method, mask, newcell, range, &object, value, zone)) {
						if (object != NULL && (!onlyenemy || object->House->HeapID == House->Enemy) &&
						(!(method & THREAT_ALLIES) || House->Is_Ally(object->House))) {
							if (bestval < value) {
								bestobject = object;
							}
						}
					}
					if (bestobject == NULL) {
						value = Evaluate_Just_Cell(newcell);
						if (bestcellvalue < value) {
							bestcellvalue = value;
							bestcell = newcell;
						}
					}
				}

				newcell = cell + Cell(radius, y);
				if (Map.In_Radar(newcell)) {
					if (Evaluate_Cell(method, mask, newcell, range, &object, value, zone)) {
						if (object != NULL && (!onlyenemy || object->House->HeapID == House->Enemy) &&
						(!(method & THREAT_ALLIES) || House->Is_Ally(object->House))) {
							if (bestval < value) {
								bestobject = object;
							}
						}
					}
					if (bestobject == NULL) {
						value = Evaluate_Just_Cell(newcell);
						if (bestcellvalue < value) {
							bestcellvalue = value;
							bestcell = newcell;
						}
					}
				}
			}

			/*
			**	Bail early if a target has already been found and the range is at
			**	one of the breaking points (i.e., normal range or range * 2).
			*/
			if (bestobject != NULL) {
				if (radius == crange/4) {
					return((ObjectClass *)bestobject);
				}
				if (radius == crange/2) {
					return((ObjectClass *)bestobject);
				}
			}
			if (bestcell != CELL_NONE) {
				return(&Map[bestcell]);
			}
		}

	} else {
		/*
		**	A full map scan was requested. First scan through aircraft. The top map layer
		**	is NOT scanned since that layer will probably contain more bullets and animations
		**	than aircraft.
		*/
		int index;
		if (mask & (1L << RTTI_AIRCRAFT)) {
			for (index = 0; index < Aircraft.Count(); index++) {
				TechnoClass * object = Aircraft[index];

				int value = 0;
				if ((!onlyenemy || object->House->HeapID == House->Enemy) &&
					(!(method & THREAT_ALLIES) || House->Is_Ally(object->House)) &&
					Evaluate_Object(method, mask, -1, object, value)) {

					if (value > bestval) {
						bestobject = object;
						bestval = value;
					}
				}
			}
		}

		/*
		**	When scanning the ground, always consider landed aircraft as a valid
		**	potential target. This is only true if vehicles are considered a
		**	valid target. A landed aircraft is considered a vehicle.
		*/
		if (method & THREAT_VEHICLES) {
			mask |= (1 << RTTI_AIRCRAFT);
		}

		/*
		**	Now scan through the entire ground layer. This is painful, but what other
		**	choice is there?
		*/
		for (index = 0; index < Technos.Count(); index++) {
				TechnoClass * object = Technos[index];

				int value = 0;
				if (object->Layer == LAYER_GROUND &&
					(!onlyenemy || object->House->HeapID == House->Enemy) &&
					(!(method & THREAT_ALLIES) || House->Is_Ally(object->House)) &&
					Evaluate_Object(method, mask, -1, object, value, zone, coord)) {

					if (value > bestval) {
						bestobject = object;
						bestval = value;
					}
				}
			}
	}

	BEnd(BENCH_GREATEST_THREAT);

	/*
	**	If a good target object was found, then return with the target value
	**	of it.
	*/
	return((ObjectClass *)bestobject);
}


/***********************************************************************************************
 * TechnoClass::Owner -- Who is the owner of this object?                                      *
 *                                                                                             *
 *    Use this routine to examine this object and return who the owner is.                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the house number of the owner of this object.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/09/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
HousesType TechnoClass::Owner(void) const
{
	return(House->HeapID);
}


/// <summary>
/// Returns the house that owns this techno object.
/// </summary>
/// <returns>HouseClass *; pointer to the owning house.</returns>
HouseClass * TechnoClass::Owner_HouseClass(void) const
{
	return(House);
}


/***********************************************************************************************
 * TechnoClass::Clicked_As_Target -- Sets the flash count for this techno object.              *
 *                                                                                             *
 *    Use this routine to set the flash count for the object. This flash count is the number   *
 *    of times the object will "flash". Typically it is called as a result of the player       *
 *    clicking on this object in order to make it the target of a move or attack.              *
 *                                                                                             *
 * INPUT:   count -- The number of times the object should flash.                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/09/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Clicked_As_Target(int count)
{
	FlashCount = count;
}


/// <summary>
/// Determines whether this object's voxel library is loaded and usable.
/// Returns false if the voxel library is NULL or failed to load.
/// </summary>
/// <returns>bool; is the voxel library loaded successfully?</returns>
bool TechnoClass::Is_Voxel_Loaded(void) const
{
	if (TClass->Voxel.VoxLib == NULL) {
		return(false);
	}
	if (TClass->Voxel.VoxLib->Load_Failed()) {
		return(false);
	}
	return(true);
}


/***********************************************************************************************
 * TechnoClass::AI -- Handles AI processing for techno object.                                 *
 *                                                                                             *
 *    This routine handles AI processing for techno objects. Typically, this merely dispatches *
 *    to the appropriate AI routines for the base classes.                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Make sure that this routine is only called ONCE per game tick.                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/09/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::AI(void)
{
	if (Is_Voxel_Loaded()) {
		Rocking_AI();
		if (!IsActive) {
			return;
		}
	}

	if (!House->Is_Human_Player() && TarCom != NULL && House->Is_Ally(TarCom)) {
		if (RTTI != RTTI_AIRCRAFT && (RTTI != RTTI_INFANTRY || !((InfantryClass *)this)->Class->IsEngineer)) {
			Assign_Target(NULL);
		}
	}

	/*
	**	Handle recoil recovery here.
	*/
	if (IsInRecoilState) {
		IsInRecoilState = false;
	}

	if (TarCom != NULL && PrimaryWeapon != NULL) {
		BarrelPitch.Set_Desired(Barrel_Pitch(TarCom));
		if (RTTI != RTTI_BUILDING || !PrimaryWeapon->IsLaser) {
			Coord coord = Predict_Target_Coord() - Turret_Coord();
			WeaponDataStruct const * wdata = Get_Class_Weapon_Data(0);
			WeaponTypeClass const * weapon = wdata->Weapon;
			double gravity = Rule->Gravity;
			BulletTypeClass const * bullet = weapon->Bullet;
			if (bullet != NULL && bullet->IsFloater) {
				gravity = Get_Floater_Gravity();
			}
			DirType dir;
			if (Calculate_Projectile_Pitch(Should_Use_High_Arc(0), weapon->MaxSpeed, std::sqrt(coord.X * coord.X + coord.Y * coord.Y), coord.Z, gravity, dir)) {
				BarrelPitch.Set_Desired(dir);
			} else {
				BarrelPitch.Set_Desired(DirType(DIR_E) - DirType(Dir256(TClass->FireAngle)));
			}
		}
	} else {
		if (RTTI != RTTI_BUILDING && Mission != MISSION_UNLOAD) {
			BarrelPitch.Set_Desired(DirType(DIR_E) - DirType(Dir256(TClass->FireAngle)));
		}
	}

	Door.AI();

	if (UnusedCooldown) {
		UnusedCooldown--;
	}

	Cargo.AI();

	if (IsBurstResetPending && BurstResetTimer == 0) {
		IsBurstResetPending = false;
		if (TarCom == NULL) {
			BurstIndex = 0;
		}
	}

	BASECLASS::AI();

	if (!IsActive) return;

	/*
	**	If this is a vehicle that heals itself (e.g., Mammoth Tank), then it will perform
	**	the heal logic here.
	*/
	if (Should_Self_Heal_Now()) {
		Strength++;
		if (HealthRatio > Rule->ConditionYellow || HeightAGL < -10) {
			if (ParticleSystems[ATTACHED_PARTICLE_DAMAGE] != NULL) {
				ParticleSystems[ATTACHED_PARTICLE_DAMAGE]->Delete_Me();
			}
		}
	}

	/*
	**	Cloaking device processing.
	*/
	Cloaking_AI();

	if (Cloak == UNCLOAKED) {
		if (Map[Center_Coord()].Is_Cloaked(House->HeapID)) {
			Try_To_Cloak();
		}
	}

	if (Cloak == CLOAKED) {
		if (!Map[Center_Coord()].Is_Cloaked(House->HeapID)) {
			Try_To_Cloak();
		}
	}

	/*
	**	If for some strange reason, the computer is firing upon itself, then
	**	tell it not to.
	*/
	if (Session.Type == GAME_NORMAL && !House->Is_Player_Control() && TarCom != NULL) {
		if (House->Is_Ally(TarCom)) {
			if (Combat_Damage() > 0) {
				Assign_Target(NULL);
			}
		} else if (Combat_Damage() < 0) {
			Assign_Target(NULL);
		}
	}

	if (TarCom != NULL && Session.Type != GAME_NORMAL && House->IsHuman) {
		if (Combat_Damage() < 0 && !House->Is_Ally(TarCom)) {
			Assign_Target(NULL);
		}
	}

	if (TarCom != NULL && TarCom->RTTI == RTTI_AIRCRAFT) {
		AircraftClass * tarcom = (AircraftClass *)TarCom;
		if (RTTI == RTTI_UNIT && Combat_Damage() < 0 && (tarcom->HeightAGL > 0 || Map[tarcom->Get_Coord()].Cell_Building() != NULL)) {
			Assign_Target(NULL);
		}
	}

	/*
	**	Perform a maintenance check to see that if somehow this object is trying to fire
	**	upon an object it can never hit (because it can't reach it), then abort the tarcom
	*/
	if (RTTI != RTTI_AIRCRAFT && TarCom != NULL && !Is_In_Team()) {
		FootClass * foot = NULL;
		if (Is_Foot()) foot = (FootClass *)this;
		if (foot == NULL || foot->NavCom == NULL) {
			if (!Is_In_Same_Zone_As((ObjectClass *)TarCom)) {
				if (Is_Foot()) {
					foot->Approach_Target();
				}
				if (foot == NULL || foot->NavCom == NULL) {
					int primary = What_Weapon_Should_I_Use(TarCom);
					if (!In_Range(TarCom, primary)) {
						Assign_Target(NULL);
					}
				}
			}
		}
	}

	/*
	**	Update the animation timer system. If the animation stage
	**	changes, then flag the object to be redrawn as well as determine
	**	if the current animation process needs to change.
	*/
	if (RTTI != RTTI_BUILDING) {
		StageClass::Graphic_Logic();
	}

	/*
	**	If the object is flashing and a change of flash state has occurred, then mark the
	**	object to be redrawn.
	*/
	unsigned int oldflash = FlashCount;
	bool was_flash_on = ((oldflash / 2) % 2) == 1;

	if (FlasherClass::Process()) {
		Mark(MARK_CHANGE);
	}

	if (oldflash != 0 && oldflash != FlashCount && RTTI == RTTI_BUILDING) {
		if (was_flash_on != (((FlashCount / 2) % 2) == 1)) {
			TacticalMap->Register_Dirty_Area(Get_Render_Rect(), false);
			((BuildingClass *)this)->Update_Anim_Appearance();
		}
	}

	TechnoTypeClass const * tclass = TClass;
	if (tclass->IsDamageSparks && HealthRatio < Rule->ConditionYellow && HeightAGL > -10) {
		DynamicVectorClass<ParticleSystemTypeClass const *> sparks;
		for (int i = 0; i < tclass->DamageParticleSystems.Count(); i++) {
			if (tclass->DamageParticleSystems[i]->BehavesLike == PSYS_BEHAVIOR_SPARK) {
				sparks.Add(tclass->DamageParticleSystems[i]);
			}
		}

		if (ParticleSystems[ATTACHED_PARTICLE_SPARK] == NULL && sparks.Count() > 0) {
			double probability = HealthRatio < Rule->ConditionRed ? Rule->ConditionRedSparkingProbability : Rule->ConditionYellowSparkingProbability;
			if (Random_Double(0.0, 1.0) < probability) {
				ParticleSystems[ATTACHED_PARTICLE_SPARK] = new ParticleSystemClass(sparks[Random_Pick(0, sparks.Count() - 1)], Center_Coord() + TClass->DamageSmokeOffset, NULL, this);
			}
		}
	}

	Update_Radar_Position();

	if (StunDuration > 0) {
		StunDuration--;
		if (StunDuration == 0) {
			if (RTTI == RTTI_BUILDING) {
				BuildingClass * building = (BuildingClass *)this;
				if (building != NULL && !building->Class->IsInvisibleInGame) {
					building->Power_On();
					if (building->Class->IsRadar) {
						building->House->RecalcRadar = true;
					}
				}
				for (int i = 0; i < Anims.Count(); i++) {
					AnimClass * anim = Anims[i];
					if (anim != NULL && anim->xObject == this && anim->Class == Rule->EMPulseSparkles) {
						anim->Loops = 0;
					}
				}
			} else if (Is_Foot()) {
				TechnoClass * foot = (TechnoClass *)this;
				if (((FootClass *)foot)->Locomotion != NULL) {
					((FootClass *)foot)->Locomotion->Power_On();
				}
				UnitClass * unit = dynamic_cast<UnitClass *>(foot);
				if (unit != NULL && unit->Mission != MISSION_UNLOAD) {
					if (Rule->HarvesterUnit.Is_In_List(unit->Class)) {
						unit->Assign_Destination(NULL);
						unit->Assign_Mission(MISSION_HARVEST);
					}
				}
				for (int i = 0; i < Anims.Count(); i++) {
					AnimClass * anim = Anims[i];
					if (anim != NULL && anim->xObject == this && anim->Class == Rule->EMPulseSparkles) {
						anim->Loops = 0;
					}
				}
			}
		}
	}
}


/***********************************************************************************************
 * TechnoClass::Cloaking_AI -- Perform the AI maintenance for a cloaking device.               *
 *                                                                                             *
 *    This routine handles the cloaking device logic for this object. It will handle the       *
 *    transition effects as the object cloaks or decloaks. It will also try to start an        *
 *    object to cloak if possible.                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/09/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Cloaking_AI(bool)
{
	/*
	**	If this object is uncloaked, but it can be cloaked and it thinks that it
	**	is a good time do so, then begin cloaking.
	*/
	if (Cloak == UNCLOAKED) {
		if ((Is_Allowed_To_Recloak() && !Is_Immobilized()) || Has_Ability(ABILITY_CLOAK)) {
			RadioClass * radio = Contact_With_Whom();
			if (radio == NULL || !(radio->RTTI == RTTI_BUILDING && ((BuildingClass *)radio)->Class->IsWeaponsFactory)) {
				CloakingDevice.Graphic_Logic();
				if (Is_Ready_To_Cloak()) {
					if (HealthRatio > Rule->ConditionRed) {
						Do_Cloak();
					} else {
						if (Percent_Chance(4)) {
							Do_Cloak();
						}
					}
				}
			}
		}
	} else {
		CloakingDevice.Graphic_Logic();
		if (CloakingDevice.Fetch_Stage() < 0) {
			CloakingDevice.Set_Stage(0);
		}
		switch (Cloak) {
			/*
			**	Handle the uncloaking process. Always mark to redraw
			**	the object and when cloaking is complete, stabilize into
			**	the normal uncloaked state.
			*/
			case UNCLOAKING:
			{
				Mark(MARK_CHANGE);
				switch (Visual_Character(true)) {
					case VISUAL_NORMAL:
						CloakingDevice.Set_Rate(0);
						CloakingDevice.Set_Stage(0);	// re-start the stage counter
						Cloak = UNCLOAKED;
						CloakDelay = Rule->CloakDelay * TICKS_PER_MINUTE;
						Mark(MARK_CHANGE);
						break;

					case VISUAL_INDISTINCT:
						if (Is_Ready_To_Cloak()) {
							Do_Cloak(true);
						}
				}
				break;
			}
			/*
			**	Handle the cloaking process. Always mark to redraw the object
			**	and when the cloaking process is complete, stabilize into the
			**	normal cloaked state.
			*/
			case CLOAKING:
				Mark(MARK_CHANGE);
				if (!CloakingDevice.Fetch_Rate()) {
					CloakingDevice.Set_Rate(1);
				}
				switch (Visual_Character(true)) {
					/*
					**	If badly damaged, then it can never fully cloak.
					*/
					case VISUAL_DARKEN:
						if (HealthRatio <= Rule->ConditionRed && Percent_Chance(10)) {
							Do_Uncloak(true);
						}
						break;
					case VISUAL_SHADOWY:
					case VISUAL_HIDDEN:
						Cloak = CLOAKED;
						CloakingDevice.Set_Rate(0);
						CloakingDevice.Set_Stage(0);
						Mark(MARK_CHANGE);
						/*
						**	Special check to ensure that if the unit is carrying a captured
						**	flag, it will never fully cloak.
						*/
						if (RTTI == RTTI_UNIT && ((UnitClass *)this)->Flagged != HOUSE_NONE) {
							Do_Shimmer();
						} else {
							int i;
							DynamicVectorClass<TechnoClass *> targeting_me;
							for (i = Technos.Count() - 1; i >= 0; i--) {
								TechnoClass * tech = Technos[i];
								if (tech->TarCom == this) {
									if (Map[Center_Coord()].Is_Sensed(tech->House->HeapID) || tech->House == House) {
										targeting_me.Add(tech);
									}
								}
							}
							Detach_All(false);
							for (i = targeting_me.Count() - 1; i >= 0; i--) {
								targeting_me[i]->Assign_Target(this);
							}
						}
						/*
						**	A computer controlled unit will try to scatter if possible so
						**	that it will be much harder to locate.
						*/
						if (RTTI == RTTI_UNIT && !House->Is_Human_Player()) {
							Scatter(COORD_NONE, true);
						}
						break;
				}
				break;
			/*
			**	A cloaked object will always be redrawn if it is owned by the
			**	player. This ensures that the shimmering effect will animate.
			*/
			case CLOAKED:
				if (Should_Uncloak()) {
					Do_Uncloak();
				}
				break;
		}
	}
}


/// <summary>
/// Should this object drop its cloak?
/// An object that has lost the right to stay hidden -- because it may no longer recloak, is
/// not a cloaking type at all, or has been immobilized -- must decloak, unless it carries
/// the cloak ability outright or is standing in cloaking cover.
/// </summary>
/// <returns>bool; Should the object uncloak now?</returns>
bool TechnoClass::Should_Uncloak(void) const
{
	bool cloaked = Map[Center_Coord().As_Cell()].Is_Cloaked(House->HeapID);
	if (!(Is_Allowed_To_Recloak() || IsCloakable) || Is_Immobilized()) {
		if (Has_Ability(ABILITY_CLOAK)) return(false);
		if (!cloaked) return(true);
	}
	return(false);
}


/***********************************************************************************************
 * TechnoClass::Is_Ready_To_Cloak -- Determines if this object is ready to begin cloaking.     *
 *                                                                                             *
 *    This routine will examine this object and determine if it can and is ready and able      *
 *    to begin cloaking. It will also check to make sure it appears to be a good time to cloak *
 *    as well.                                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is this unit ready and able to start cloaking?                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/09/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoClass::Is_Ready_To_Cloak(void) const
{
	/*
	**	If the object cannot recloak, then it certainly is not allowed to start.
	*/
	if (!Is_Allowed_To_Recloak() && !Has_Ability(ABILITY_CLOAK)) {
		if (!Map[Center_Coord().As_Cell()].Is_Cloaked(House->HeapID) && !IsCloakable) {
			return(false);
		}
	}

	/*
	**	If it is already cloaked or in the process of cloaking, then it can't start cloaking.
	*/
	if (Cloak == CLOAKED) {
		return(false);
	}

	/*
	**	If the object is currently rearming, then don't begin to recloak.
	*/
	if (Arm != 0) {
		return(false);
	}

	/*
	**	If it seems like this object is about to fire on a target, then don't begin
	**	cloaking either.
	*/
	if (TarCom != NULL && In_Range(TarCom)) {
		return(false);
	}

	/*
	**	Recloaking can only begin if the cloaking device is not already operating.
	*/
	if (RTTI != RTTI_BUILDING && CloakingDevice.Fetch_Stage() != 0) {
		return(false);
	}

	/*
	**	If the arbitrary cloak delay value is still counting down, then don't
	**	allow recloaking just yet.
	*/
	if (CloakDelay != 0) {
		return(false);
	}

	/*
	**	All tests passed, so this object is allowed to begin cloaking.
	*/
	return(true);
}


/***********************************************************************************************
 * TechnoClass::Select -- Selects object and checks to see if can be selected.                 *
 *                                                                                             *
 *    This function checks to see if this techno object can be selected. If it can, then it    *
 *    is selected.                                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/11/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoClass::Select(void)
{
	if (!IsDiscoveredByPlayer && !House->Is_Player_Control() && MainWindow) {
		return(false);
	}

	if (BASECLASS::Select()) {

		/*
		**	Speak a confirmation of selection.
		*/
		if (House->Is_Player_Control() && AllowVoice) {
			if (Tag != NULL) {
				Tag->Spring(TEVENT_SELECTED, this);
			}
			Response_Select();
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * TechnoClass::Can_Fire -- Determines if this techno object can fire.                         *
 *                                                                                             *
 *    This performs a simple check to make sure that this techno object can fire. At this      *
 *    level, the only thing checked for is the rearming delay.                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the fire legality control code.                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/23/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
FireErrorType TechnoClass::Can_Fire(AbstractClass * target, int which) const
{
	/*
	**	Don't allow firing if the target is illegal.
	*/
	if (target == NULL) {
		return(FIRE_ILLEGAL);
	}

	TechnoClass const * techno = Dynamic_Cast<TechnoClass const *>((AbstractClass const *)target);
	CellClass * cellptr = &Map[target->Center_Coord()];
	WeaponTypeClass const * weapon;
	WeaponTypeClass const * other_weapon;
	bool check_rearm;

	/*
	**	If the object is completely cloaked, then you can't fire on it.
	*/
	if (techno != NULL && techno->Visual_Character(true, House) == VISUAL_HIDDEN
		&& !cellptr->Is_Sensed(House->HeapID) && techno->House != House
		&& (Combat_Damage() > 0 || !techno->House->Is_Ally(House)))	{

		goto CANT_FIRE;
	}

	/*
	**	A falling object is too busy falling to fire.
	*/
	if (IsFalling) {
		goto CANT_FIRE;
	}

	if (Is_Immobilized()) {
		if (RTTI != RTTI_UNIT) {
			goto CANT_FIRE;
		}
		if (!((UnitClass*)this)->Class->IsLargeVisceroid && !((UnitClass*)this)->Class->IsSmallVisceroid) {
			goto CANT_FIRE;
		}
	}

	/*
	**	If there is no weapon, then firing is not allowed.
	*/
	weapon = Get_Class_Weapon_Data(which)->Weapon;
	if (weapon == NULL) {
		goto CANT_FIRE;
	}

	if (weapon->IsIonSensitive && IonStormClass::Is_Ion_Storm_Active()) {
		goto CANT_FIRE;
	}

	other_weapon = Get_Class_Weapon_Data(which == 0 ? 1 : 0)->Weapon;
	if (other_weapon != NULL) {
		if ((other_weapon->UseFireParticles && ParticleSystems[ATTACHED_PARTICLE_FIRE])
			|| (other_weapon->IsRailgun && ParticleSystems[ATTACHED_PARTICLE_RAILGUN])
			|| (other_weapon->UseSparkParticles && ParticleSystems[ATTACHED_PARTICLE_SPARK])
			|| (other_weapon->IsSonic && Wave)) {

			goto CANT_FIRE;
		}
	}

	/*
	**	Can only fire anti-aircraft weapons against aircraft unless the aircraft is
	**	sitting on the ground.
	*/
	if (target->In_Air() && !weapon->Bullet->IsAntiAircraft) {
		goto CANT_FIRE;
	}

	/*
	**	If the object is on the ground, then don't allow firing if it can't fire upon ground objects.
	*/
	if (target->On_Ground() && !weapon->Bullet->IsAntiGround) {
		goto CANT_FIRE;
	}

	check_rearm = true;
	if (which != 1 && RTTI == RTTI_UNIT) {
		UnitClass const * unit = dynamic_cast<UnitClass const *>(this);
		if (unit != NULL) {
			int burst = unit->BurstIndex % weapon->Burst;
			if (burst < 2) {
				if (unit->Class->FiringSyncFrame[burst] != -1 && unit->FiringSyncDelay != -1) {
					if (unit->FiringSyncDelay != unit->Class->FiringSyncFrame[burst]) {
						return(FIRE_REARM);
					}

					check_rearm = false;
				}
			}
		}

	}

	/*
	**	Don't allow firing if still rearming.
	*/
	if (check_rearm && Arm != 0) return(FIRE_REARM);

	if (weapon->UseFireParticles && ParticleSystems[ATTACHED_PARTICLE_FIRE]) return(FIRE_REARM);
	if (weapon->IsRailgun && ParticleSystems[ATTACHED_PARTICLE_RAILGUN]) return(FIRE_REARM);
	if (weapon->UseSparkParticles && ParticleSystems[ATTACHED_PARTICLE_SPARK]) return(FIRE_REARM);
	if (weapon->IsSonic && Wave) return(FIRE_REARM);

	/*
	**	The target must be within range in order to allow firing.
	*/
	if (!In_Range(target, which)) {
		return(FIRE_RANGE);
	}

	/*
	**	If there is no ammo left, then it can't fire.
	*/
	if (!Ammo) {
		return(FIRE_AMMO);
	}

	/*
	**	If cloaked, then firing is disabled.
	*/
	if (Cloak != UNCLOAKED) {
		if (RTTI != RTTI_AIRCRAFT || Cloak == CLOAKED) {
			return(FIRE_CLOAKED);
		}
	}

	if (TClass->IsHunterSeeker) {
		return(FIRE_RANGE);
	}
	return(FIRE_OK);

CANT_FIRE:
	return(FIRE_CANT);
}


/***********************************************************************************************
 * TechnoClass::Stun -- Prepares the object for removal from the game.                         *
 *                                                                                             *
 *    This routine handles cleaning up this techno object from the game system so that when    *
 *    it is subsequently removed, it doesn't leave any loose ends.                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/23/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Stun(void)
{
	Assign_Target(NULL);
	Assign_Destination(NULL);
	Transmit_Message(RADIO_OVER_OUT);
	Detach_All();
	Unselect();
}


/***********************************************************************************************
 * TechnoClass::Assign_Target -- Assigns the targeting computer with specified target.         *
 *                                                                                             *
 *    Use this routine to set the targeting computer for this object. It checks to make sure   *
 *    that targeting of itself is prohibited.                                                  *
 *                                                                                             *
 * INPUT:   target   -- The target for this object to attack.                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/23/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Assign_Target(AbstractClass * target)
{
	// Infantry record their own assignment before calling here, and buildings are not recorded.
	RTTIType const rtti = Fetch_RTTI();
	if (rtti != RTTI_INFANTRY && rtti != RTTI_BUILDING) {
		Sync_Record_Target(*this, target, (unsigned)(uintptr_t)_ReturnAddress());
	}

	AbstractClass * old_target = TarCom;

	if (target == TarCom) return;

	if (target == NULL) {
		target = NULL;
	} else {

		/*
		**	Prevent targeting of self.
		*/
		if (target == this) {
			target = &Map[(Coord const &)PositionCoord];
		} else {

			/*
			**	Make sure that the target is not already dead.
			*/
			ObjectClass * object = As_Object(target);
			if (object != NULL && (object->IsActive == false || object->Strength == 0)) {
				target = NULL;
			}
		}
	}

	/*
	**	Set the unit's targeting computer.
	*/
	TarCom = target;

	if (target != NULL) {
		IsBurstResetPending = false;
		BurstResetTimer = 0;
	} else if (old_target != NULL) {
		IsBurstResetPending = false;
		BurstResetTimer = 0;

		if (BurstIndex != 0) {
			int which = What_Weapon_Should_I_Use(old_target);
			WeaponTypeClass const * weapon = Get_Class_Weapon_Data(which)->Weapon;
			if (weapon != NULL && weapon->Burst > 1) {
				int old_burst_index = BurstIndex;
				BurstIndex = weapon->Burst;
				BurstResetTimer = Rearm_Delay(which);
				BurstIndex = old_burst_index;
				IsBurstResetPending = true;
			}
		}

		if (!IsBurstResetPending) {
		BurstIndex = 0;
	}
	} else if (!IsBurstResetPending) {
		BurstIndex = 0;
		BurstResetTimer = 0;
	}

	if (ParticleSystems[ATTACHED_PARTICLE_FIRE]) {
		if (!In_Range(target)) {
			ParticleSystems[ATTACHED_PARTICLE_FIRE]->Delete_Me();
			ParticleSystems[ATTACHED_PARTICLE_FIRE] = NULL;
		}
	}
}


/***********************************************************************************************
 * TechnoClass::Rearm_Delay -- Calculates the delay before firing can occur.                   *
 *                                                                                             *
 *    This function calculates the delay between shots. It determines this from the standard   *
 *    rate of fire (ROF) of the base class and modifies it according to game speed and         *
 *    whether this is the first or second shot. All single shot attackers consider their       *
 *    shots to be "second" since the second shot is the one handled normally. The first shot   *
 *    usually gets assigned a much shorter delay time before the next shot can fire.           *
 *                                                                                             *
 * INPUT:   second   -- bool; Is this the second of a two shot salvo?                          *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before the next shot may fire.     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoClass::Rearm_Delay(int which) const
{
	if (RTTI == RTTI_BUILDING && Ammo > 1) {
		return(1);
	}

	WeaponTypeClass const * weapon = Get_Class_Weapon_Data(which)->Weapon;
	if (weapon == NULL) {
		return(1);
	}

	if (weapon->IsSonic
		|| (weapon->UseSparkParticles && ParticleSystems[ATTACHED_PARTICLE_SPARK])
		|| (weapon->UseFireParticles && ParticleSystems[ATTACHED_PARTICLE_FIRE])
		|| (weapon->IsRailgun && ParticleSystems[ATTACHED_PARTICLE_RAILGUN])) {

		return(weapon->ROF);
	}

	if (BurstIndex < weapon->Burst) {
		int delay = -1;
		if (BurstIndex > 0 && BurstIndex <= 4) {
			delay = weapon->BurstDelay[BurstIndex - 1];
		}
		if (delay == -1) delay = Random_Pick(3, 5);
		return(delay);

	} else {
		int delay = weapon->ROF * House->ROFBias + Random_Pick(0, 2);

		if (Has_Ability(ABILITY_ROF)) {
			delay = (1.0 / (Rule->VeteranROF + 1.0)) * delay;
		}
		return(delay);
	}

	return(3);
}


/***********************************************************************************************
 * TechnoClass::Laser_Zap -- Fires laser zap at the target specified.                          *
 *                                                                                             *
 *    This routine is used to fire a laser zap at the target specified.                        *
 *                                                                                             *
 * INPUT:   target   -- The target to fire the zap at.                                         *
 *                                                                                             *
 *          which    -- Which weapon is this zap associated with (0=primary, 1=secondary).     *
 *                                                                                             *
 *          window      -- The clipping window to use when rendering.                          *
 *                                                                                             *
 *          source_coord   -- The coordinate that the zap is to originate from. This is an     *
 *                            override value and if not specifide, the normal fire coordinate  *
 *                            is used.                                                         *
 *                                                                                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 BWG : Created.                                                                 *
 *   09/30/1996 JLB : Uses standard facing conversion and distance routines.                   *
 *=============================================================================================*/
void TechnoClass::Laser_Zap(AbstractClass * target, int which, WeaponTypeClass const * weapon, Coord const & source_coord)
{
	Coord source;
	Coord dest;
	int zadjust = 0;
	int duration = weapon->LaserDuration;

	if (source_coord != COORD_NONE) {
		source = source_coord;
	} else {
		source = Fire_Coord(which);
		if (source.Y != Render_Coord().Y) {
			Point2D p1 = TacticalMap->Coord_To_Pixel_Absolute(source);
			Point2D p2 = TacticalMap->Coord_To_Pixel_Absolute(Render_Coord());
			zadjust = p1.Y - p2.Y;
		}
	}

	ObjectClass *optr = target->As_ObjectClass();
	if (optr != NULL) {
		dest = optr->Target_Coord();
	} else {
		dest = target->Center_Coord();
	}

	if (RTTI == RTTI_BUILDING) {
		((BuildingClass *)this)->IsCharging = false;
	}

	new LaserDrawClass(source, dest, zadjust, true, weapon->LaserInnerColor, weapon->LaserOuterColor, weapon->LaserOuterSpread, duration, false, false, 1.0, 0.0);
	new WaveClass(source, dest, this, weapon->IsBigLaser ? WAVE_BIG_LASER : WAVE_LASER, (TechnoClass *)target);
}


/// <summary>
/// Fetches the barrel elevation needed to hit a target.
/// This routine is used when aiming a weapon that lobs its shot, solving the ballistic arc
/// to where the target is predicted to be by the time the shot arrives. When no arc will
/// reach, a fixed fallback pitch is used rather than aiming flat.
/// </summary>
/// <param name="target">The target to aim at. If NULL, the resting fire angle is used.</param>
/// <returns>Returns with the pitch to set the barrel to.</returns>
DirType TechnoClass::Barrel_Pitch(AbstractClass * target) const
{
	DirType pitch(Dir256(TClass->FireAngle));
	if (target != NULL) {
		Coord predicted = Predict_Target_Coord();
		Coord coord = predicted - Turret_Coord();
		WeaponTypeClass const * weapon = PrimaryWeapon;
		double gravity = Rule->Gravity;
		BulletTypeClass const * bullet = weapon->Bullet;
		if (bullet != NULL && bullet->IsFloater) {
			gravity = Get_Floater_Gravity();
		}
		if (!Calculate_Projectile_Pitch(Should_Use_High_Arc(0), weapon->MaxSpeed, std::sqrt(pow(coord.X, 2) + pow(coord.Y, 2)), coord.Z, gravity, pitch)) {
			pitch = DIR_NW;
		}
	}
	return(pitch);
}


/***********************************************************************************************
 * TechnoClass::Fire_At -- Fires projectile at target specified.                               *
 *                                                                                             *
 *    This is the main projectile firing code. Buildings, units, and infantry route fire       *
 *    requests through this function.                                                          *
 *                                                                                             *
 * INPUT:   target   -- The target that the projectile is to be fired at.                      *
 *                                                                                             *
 *          which    -- Which weapon to fire.                                                  *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the projectile object that was fired. If no projectile   *
 *          could be created or there was some other illegality detected, the return value     *
 *          will be NULL.                                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *   07/03/1995 JLB : Moving platforms fire inaccurate projectiles.                            *
 *   02/22/1996 JLB : Handles camera "weapon" case.                                            *
 *=============================================================================================*/
BulletClass * TechnoClass::Fire_At(AbstractClass * target, int which)
{
	BulletClass * bullet;            // Projectile.
	DirType dir;                     // The facing to impart upon the projectile.
	Coord target_coord;              // Coordinate of the target.
	Coord fire_coord;                // Coordinate of firing position.
	ObjectClass * object;
	WeaponTypeClass const * weapon = Get_Class_Weapon_Data(which)->Weapon;

	/*
	**	If this object doesn't have a weapon, then it is obvious that firing
	**	cannot ever succeed. Return with failure flag.
	*/
	if (weapon == NULL) return(NULL);

	BulletTypeClass const & btype = *weapon->Bullet;

	/*
	**	Perform a quick legality check to see if firing can occur.
	*/
	if (Debug_Map || target == NULL) {
		return(NULL);
	}

	if (weapon->WarheadPtr != NULL && weapon->WarheadPtr->LimpetFactor > 0 && target->Is_Techno() == true) {
		TechnoClass * techno = (TechnoClass *)target;
		if ((techno->LimpetType & 1 << House->HeapID) == 0) {
			techno->LimpetType |= 1 << House->HeapID;
			techno->LimpetSpeedFactor = (double)(100 - weapon->WarheadPtr->LimpetFactor) / 100.0;
			PrimaryFacing.Set_ROT((int)((double)TClass->ROT * techno->LimpetSpeedFactor));
			SecondaryFacing.Set_ROT((int)((double)TClass->ROT * techno->LimpetSpeedFactor));
			if (weapon->Sound.Count() > 0) {
				Sound_Effect((VocType)weapon->Sound.Pick(SoundRandomSeed));
			}
			if (techno->Tag != NULL) {
				techno->Tag->Spring(TEVENT_LIMPED, techno);
			}
			DebugString("Limped %s\n", (char const *)techno->TClass->IniName);
			Delete_Me();
		}
		return(NULL);
	}

	if ((weapon->UseFireParticles && ParticleSystems[ATTACHED_PARTICLE_FIRE])
		|| (weapon->IsRailgun && ParticleSystems[ATTACHED_PARTICLE_RAILGUN])
		|| (weapon->UseSparkParticles && ParticleSystems[ATTACHED_PARTICLE_SPARK])
		|| (weapon->IsSonic && Wave)) {

		return(NULL);
	}

	/*
	**	Fetch the target coordinate for the target specified.
	*/
	object = target->As_ObjectClass();
	if (object != NULL) {
		target_coord = object->Target_Coord();
	} else {
		target_coord = target->As_Coord();
	}

	BarrelPitch.Set_Desired(Barrel_Pitch(target));

	/*
	**	Get the location where the projectile should appear.
	*/
	fire_coord = Turret_Coord(which);

	/*
	**	Create the projectile. Then process any special operations that
	**	need to be performed according to the style of projectile
	**	created.
	*/
	int firepower = weapon->Attack;
	if (weapon->IsSonic || weapon->UseFireParticles) {
		firepower = 0;
	}
	if (firepower > 0) {
		firepower = (int)(House->FirepowerBias * FirepowerBias * weapon->Attack);
		if (Has_Ability(ABILITY_FIREPOWER)) {
			firepower = (int)((Rule->VeteranCombat + 1.0) * firepower);
		}
	}

	int max_speed = weapon->MaxSpeed;
	bullet = Create_Bullet(weapon->Bullet, target, this, firepower, weapon->WarheadPtr, max_speed, weapon->ProjectileRange, weapon->IsBright);

	if (bullet != NULL) {
		bullet->Limbo();

		/*
		**	If this is firing from a moving platform, then the projectile is inaccurate.
		*/
		if (Is_Foot() && ((FootClass const *)this)->Locomotion->Is_Moving()) {
			bullet->IsInaccurate = true;
		}

		Coord turret_coord = Turret_Coord(which);
		Coord displacement = Predict_Target_Coord() - turret_coord;

		if (bullet->Class->IsInaccurate && bullet->Class->IsArcing) {
			int spread = Scen->RandomNumber(Rule->BallisticScatter / 2, Rule->BallisticScatter);
			DirType dir(Random_Double(0.0, M_PI * 2));
			displacement = Move_Coord(displacement, dir, spread);
		}

		/*
		**	If the projectile is a homing type (such as a missile), then it will
		**	launch in the direction the turret is facing, NOT necessarily the same
		**	direction as the target.
		*/
		if (btype.ROT != 0 || btype.IsDropping) {
			dir = Fire_Direction();
			if (btype.IsDropping) {
				fire_coord = Center_Coord();
			}
		} else {
			dir = std::atan2((double)-displacement.Y, (double)displacement.X);
		}

		if (max_speed > displacement.Length() / 2) {
			max_speed = displacement.Length() / 2;
		}

		if (weapon->Bullet->ROT > 0) {
			bullet->MaxSpeed = weapon->MaxSpeed;
			max_speed = 1;
		}

		TVelocity3D<double> velocity;
		velocity.Set(0.0, 0.0, 0.0);
		velocity.Set_Yaw(dir);
		velocity.Set_Speed((double)max_speed);

		DirType pitch((DirType(DIR_E)-DirType(DIR_STEP_256)).As_Int());
		bool valid_arc = true;
		if (bullet->Class->IsArcing) {
			double gravity = Rule->Gravity;
			if (bullet->Class->IsFloater) {
				gravity = Get_Floater_Gravity();
			}

			int planar = Point2D(displacement).Length();
			valid_arc = Calculate_Projectile_Pitch(Should_Use_High_Arc(which), max_speed, planar, displacement.Z, gravity, pitch);
		} else if (bullet->Class->IsVoxel) {
			pitch = 0;
		} else {
			int abs_z = abs(displacement.Z);
			if (abs_z > 200) {
				double y = 20.0;
				BuildingClass * building = TarCom->As_BuildingClass();
				if (building != NULL) {
					displacement.Z = 200 * building->Class->ZHeight - turret_coord.Z;
					abs_z = abs((displacement.Z));
					if ((double)abs_z < y) {
						y = 0.0;
					}
				}

				double planar = Point2D(displacement).Length();
				if (planar < 0.05) {
					planar = 0.05;
				}
				double angle = std::atan2f((double)abs_z - y, planar);
				if (displacement.Z < 0) {
					angle = -angle;
				}
				pitch = angle;
			}
		}

		velocity.Set_Pitch(pitch);

		if (valid_arc) {
			if (!bullet->Unlimbo(turret_coord, velocity)) {
				bullet->Release();
				bullet = NULL;
			} else {

				if (bullet->Class->IsInvisible && object != NULL && object->IsOnBridge) {
					bullet->IsOnBridge = true;
				}

				if (Is_Turret_Equipped()) {
					IsInRecoilState = true;
				}

				if (Get_Class_Weapon_Data(which)->BarrelLength > 0 && !weapon->IsLaser) {
					if (weapon->Bullet == NULL || !weapon->Bullet->IsInvisible) {
						bullet->AI();
						if (bullet->IsActive) {
							bullet->AI();
						}
					}
				}

				if (weapon->UseFireParticles && ParticleSystems[ATTACHED_PARTICLE_FIRE] == NULL) {
					ParticleSystems[ATTACHED_PARTICLE_FIRE] = new ParticleSystemClass(weapon->AttachedParticleSystem, Fire_Coord(which), target, this);
				}
				if (weapon->UseSparkParticles && ParticleSystems[ATTACHED_PARTICLE_SPARK] == NULL) {
					ParticleSystems[ATTACHED_PARTICLE_SPARK] = new ParticleSystemClass(weapon->AttachedParticleSystem, Fire_Coord(which), target, this);
				}
				if (weapon->IsRailgun && ParticleSystems[ATTACHED_PARTICLE_RAILGUN] == NULL) {
					Coord start = Fire_Coord(which);
					Coord end = Railgun_Beam_Damage(start, target, (WeaponTypeClass *)weapon);
					ParticleSystems[ATTACHED_PARTICLE_RAILGUN] = new ParticleSystemClass(weapon->AttachedParticleSystem, start, NULL, this, end);
				}

				BurstIndex++;
				Arm = Rearm_Delay(which);
				BurstIndex %= weapon->Burst;

				/*
				**	Perform any animation effect for this weapon.
				*/
				AnimTypeClass const * a = NULL;
				if (weapon->Anim.Count() == 8) {
					a = weapon->Anim[(Fire_Direction().As_Dir8() + FACING_45) % FACING_COUNT];
				} else if (weapon->Anim.Count() > 0) {
					a = weapon->Anim[0];
				}

				/*
				**	Play any sound effect tied to this weapon type.
				*/
				if (weapon->Sound.Count() > 0) {
					Sound_Effect((VocType)weapon->Sound.Pick(SoundRandomSeed), fire_coord);
				}

				/*
				**	If there is a special firing animation, then create and attach it
				**	now.
				*/
				if (a != NULL) {
					Coord anim_coord = Fire_Coord(which);
					AnimClass * anim = new AnimClass(a, anim_coord);
					if (RTTI == RTTI_BUILDING) {
						int zadjust = (anim_coord.Y - Render_Coord().Y) / -4;
						anim->ZAdjust = std::min(zadjust, 0);
					}
					if (anim != NULL && RTTI != RTTI_BUILDING) {
						anim->Attach_To(this);
					}
				}

				if (weapon->IsSonic) {
					Wave = new WaveClass(Fire_Coord(which), target_coord, this, WAVE_SONIC, (TechnoClass *)target);
				}

				if (TClass->IsTargetLaser && House->Is_Player_Control()) {
					TargetingLaserTimer = UIControls.TargetLaserTime;
				}

				/*
				 * Laser zap animation.
				 */
				if (weapon->IsLaser) {
					BuildingClass * building = dynamic_cast<BuildingClass *>(this);
					Laser_Zap(target, which, PrimaryWeapon, COORD_NONE);
					if (building != NULL) {
						building->BuildingStage.Set_Stage(0);
						building->BuildingStage.Set_Rate(0);
					}
					if (Ammo <= 1 && RTTI == RTTI_BUILDING) {
						BuildingClass * discharge = dynamic_cast<BuildingClass *>(this);
						if (discharge != NULL) {
							discharge->Discharge_Turret();
						}
					}
				}

				/*
				**	Reduce ammunition for this object.
				*/
				Reduce_Ammunition();

				/*
				**	Firing will in all likelihood, require the unit to be redrawn. Flag it to be
				**	redrawn here.
				*/
				Mark(MARK_CHANGE);

				/*
				**	If a projectile was fired from a unit that is hidden in the darkness,
				**	reveal that unit and a little area around it.
				**	For multiplayer games, only reveal the unit if the target is the
				**	local player.
				*/
				if ((!IsOwnedByPlayer && !IsDiscoveredByPlayer) || ((Map.Is_Shrouded(Center_Coord()) || Map.Is_Fogged(Center_Coord())) && (RTTI != RTTI_AIRCRAFT || !IsOwnedByPlayer))) {
					ObjectClass * obj = target->As_ObjectClass();
					if (obj != NULL) {
						HouseClass * tgt_owner = obj->Owner_HouseClass();

						if (tgt_owner != NULL && tgt_owner->Is_Player_Control()) {
							Map.Sight_From(Center_Coord(), 2, tgt_owner);
						}
					}
				}
			}
		} else {
			bullet->Release();
			bullet = NULL;
		}
	}

	return(bullet);
}


/// <summary>
/// Draws the targeting laser line from this object's firing coordinate to its current
/// target, styled by UI.INI.
/// </summary>
void TechnoClass::Draw_Target_Laser(void) const
{
	if (TarCom != NULL) {
		Draw_Action_Line_Segment(*LogicalSurface, Turret_Coord(0), Predict_Target_Coord(), UIControls.Target_Laser_Style(), 2, 1, 0);
	}
}


/***********************************************************************************************
 * TechnoClass::Player_Assign_Mission -- Assigns a mission as result of player input.          *
 *                                                                                             *
 *    This routine is called when the mission for an object needs to change as a result of     *
 *    player input. The basic operation would be to queue the event and let the action         *
 *    occur at the frame dictated by the queuing system. However, if a voice response is       *
 *    indicated, then perform it at this time. This will give a greater illusion of            *
 *    immediate response.                                                                      *
 *                                                                                             *
 * INPUT:   mission     -- The mission order to assign to this object.                         *
 *                                                                                             *
 *          target      -- The target of this object. This will be used for combat and attack. *
 *                                                                                             *
 *          destination -- The movement destination for this object.                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/22/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Player_Assign_Mission(MissionType mission, AbstractClass * target, AbstractClass * destination)
{
	if (AllowVoice) {
		if (mission == MISSION_ATTACK) {
			Response_Attack();
		} else {
			Response_Move();
		}
	}

	/*
	**	Cooerce the movement mission into a queued movement mission if the ALT key was
	**	held down.
	*/
	if (mission == MISSION_MOVE && (Keyboard->Down(Options.KeyQueueMove1) || Keyboard->Down(Options.KeyQueueMove2))) {
		mission = MISSION_QMOVE;
	}

	TargetClass dest(destination);
	TargetClass targ(target);
	Queue_Mission(TargetClass(this), mission, targ, dest);
}


/***********************************************************************************************
 * TechnoClass::What_Action -- Determines what action to perform if object is selected.        *
 *                                                                                             *
 *    This routine will examine the object specified and return with the action that will      *
 *    be performed if the mouse button were clicked over the object.                           *
 *                                                                                             *
 * INPUT:   object   -- The object that the mouse button might be clicked on.                  *
 *                                                                                             *
 * OUTPUT:  Returns with the action that will be performed if the object was clicked on.       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *   03/21/1995 JLB : Special target control for trees.                                        *
 *=============================================================================================*/
ActionType TechnoClass::What_Action(ObjectClass const * object, bool disallow_force) const
{
	if (object != NULL) {

		/*
		**	Return the ACTION_SELF flag if clicking on itself. However, if this
		**	object cannot do anything special with itself, then just return with
		**	the no action flag.
		*/
		if (object == this && CurrentObject.Count() == 1 && House->Is_Player_Control()) {
			return(ACTION_SELF);
		}

		if (object == this && CurrentObject.Count() == 1 && House->Is_Ally(PlayerPtr) && Session.Type == GAME_NORMAL) {
			UnitClass const * unit = dynamic_cast<UnitClass const *>(this);
			if (unit != NULL && unit->Class->Max_Passengers() > 0 && unit->Cargo.How_Many() > 0) {
				return(ACTION_SELF);
			}
			AircraftClass const * airc = dynamic_cast<AircraftClass const *>(this);
			if (airc != NULL && airc->Passenger) {
				return(ACTION_SELF);
			}
		}

		bool altdown = !disallow_force && (Keyboard->Down(Options.KeyForceMove1) || Keyboard->Down(Options.KeyForceMove2));
		bool ctrldown = !disallow_force && (Keyboard->Down(Options.KeyForceAttack1) || Keyboard->Down(Options.KeyForceAttack2));
		bool shiftdown = !disallow_force && (Keyboard->Down(Options.KeySelect1) || Keyboard->Down(Options.KeySelect2));

		/*
		**	Special guard area mission is possible if both the control and the
		**	alt keys are held down.
		*/
		if (House->Is_Player_Control() && ctrldown && altdown && Can_Player_Move()) {
			return(ACTION_GUARD_AREA);
		}

		/*
		**	Special override to force a move regardless of what is occupying the location.
		*/
		if (altdown) {
			if (House->Is_Player_Control() && Can_Player_Move()) {
				return(ACTION_MOVE);
			}
		}

		/*
		**	Override so that toggled select state can be performed while the <SHIFT> key
		**	is held down.
		*/
		if (shiftdown) {
			if (House->Is_Player_Control() && !IsALoaner) {
				return(ACTION_TOGGLE_SELECT);
			}
		}

		/*
		**	If firing is possible and legal, then return this action potential.
		*/
		TechnoTypeClass const * ttype = TClass;
		if (object->Not_Underground() && House->Is_Player_Control() && (ctrldown || !House->Is_Ally(object)) && (ctrldown || object->Class_Of()->IsLegalTarget || (Rule->IsTreeTarget && object->RTTI == RTTI_TERRAIN))) {

			if (Is_Weapon_Equipped() ||
					(RTTI == RTTI_INFANTRY &&
					(/// ((InfantryTypeClass const *)ttype)->IsBomber ||
					((InfantryTypeClass const *)ttype)->IsCapture)
					)) {

				int primary = What_Weapon_Should_I_Use((ObjectClass *)object);
				if (Can_Player_Move() || In_Range((ObjectClass *)object, primary)) {
					if (In_Range((ObjectClass *)object, primary) || (RTTI == RTTI_INFANTRY && ((InfantryClass *)this)->Class->IsCapture && object->RTTI == RTTI_BUILDING && ((BuildingClass *)object)->Class->IsCaptureable)) {
						return(ACTION_ATTACK);
					} else {
						if (!Can_Player_Move()) {
							return(ACTION_NONE);
						} else {
							return(ACTION_ATTACK);
						}
					}
				}
			}
		}

		/*
		**	Possibly try to select the specified object, if that is warranted.
		*/
		if (!Is_Weapon_Equipped() || !House->Is_Player_Control() || object->Owner() == Owner()) {
			if ((!IsALoaner || !IsOwnedByPlayer) && object->Class_Of()->IsSelectable && !object->IsSelected) {
				return(ACTION_SELECT);
			}
			return(ACTION_NONE);
		}
	}
	return(ACTION_NONE);
}


/***********************************************************************************************
 * TechnoClass::What_Action -- Determines action to perform if cell is clicked on.             *
 *                                                                                             *
 *    Use this routine to determine what action will be performed if the specified cell        *
 *    is clicked on. Usually this action is either a ACTION_MOVE or ACTION_NOMOVE. The action  *
 *    nomove is used to perform special case checking for nearby cells if in fact the mouse    *
 *    is clicked over the cell.                                                                *
 *                                                                                             *
 * INPUT:   cell  -- The cell to check for being clicked over.                                 *
 *                                                                                             *
 * OUTPUT:  Returns with the action that will occur if the cell is clicked on.                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *   07/10/1995 JLB : Force fire for buildings is explicitly disabled.                         *
 *=============================================================================================*/
ActionType TechnoClass::What_Action(Cell const & cell, bool check_fog, bool disallow_force) const
{
	CellClass const * cellptr = &Map[cell];
	OverlayTypeClass const * optr = NULL;

	bool ctrldown = !disallow_force && (Keyboard->Down(Options.KeyForceAttack1) || Keyboard->Down(Options.KeyForceAttack2));
	bool shiftdown = !disallow_force && (Keyboard->Down(Options.KeySelect1) || Keyboard->Down(Options.KeySelect2));
	bool altdown = !disallow_force && (Keyboard->Down(Options.KeyForceMove1) || Keyboard->Down(Options.KeyForceMove2));

	/*
	 * When fog of war is enabled and the fogged imagery is being examined, then
	 * scan the cell's list of fogged objects for an overlay or a targetable
	 * enemy building. This lets the player target what they can still see under
	 * the fog rather than what is actually present.
	 */
	OverlayType overlay = OVERLAY_NONE;
	bool legal = false;
	if (Scen->Special.IsFogOfWar && check_fog) {
		if (cellptr->FoggedObjects != NULL) {
			for (int index = 0; index < cellptr->FoggedObjects->Count(); index++) {
				FoggedObjectClass * fogged = (*cellptr->FoggedObjects)[index];
				if (fogged->CanDraw) {
					if (fogged->RTTI == RTTI_OVERLAY) {
						overlay = fogged->Overlay;
					} else if (fogged->RTTI == RTTI_BUILDING && (fogged->House == NULL || !PlayerPtr->Is_Ally(fogged->House)) && (fogged->Get_Head_Record_Object_Type() == NULL || fogged->Get_Head_Record_Object_Type()->IsLegalTarget)) {
						legal = true;
					}
				}
			}

			if (overlay != OVERLAY_NONE) {
				optr = OverlayTypes[overlay];
			}
		}
	}

	if (optr == NULL && cellptr->Overlay != OVERLAY_NONE) {
		optr = OverlayTypes[cellptr->Overlay];
	}

	/*
	 * Special guard area or waypoint patrol mission is possible if both the
	 * control and the alt keys are held down, or if this object is already on a
	 * waypoint patrol.
	 */
	bool renovator = Is_Renovator();
	if (House->Is_Player_Control() && ((ctrldown && altdown) || IsOnWaypointPatrol) && Can_Player_Move() && (Can_Player_Fire() || renovator)) {
		if (PlayerPtr->Waypoint_At(cell) != NULL || IsOnWaypointPatrol) {
			return(ACTION_PATROL_WAYPOINT);
		}
		return(ACTION_GUARD_AREA);
	}

	/*
	**	If firing is possible and legal, then return this action potential.
	*/
	if (House->Is_Player_Control() && Get_Class_Weapon_Data(0)->Weapon != NULL) {

		bool destroyable = Map[cell].Is_Tile_Destroyable_Cliff();
		if (destroyable) {
			if (Get_Class_Weapon_Data(0)->Weapon->WarheadPtr->IsFire) {
				destroyable = false;
			}
		}

		if (ctrldown || (optr != NULL && optr->IsLegalTarget) || destroyable || legal) {
			WarheadTypeClass const * warhead = Get_Class_Weapon_Data(0)->Weapon->WarheadPtr;
			bool isally = (cellptr->Owner != HOUSE_NONE) && !Houses[cellptr->Owner]->Is_Ally(House);

			VeinholeMonsterClass * monster = VeinholeMonsterClass::Get_Monster_At(cell);
			bool isvein = (monster != NULL && !monster->IsDead && monster->Strength > 0);

			if (ctrldown || optr == NULL || isvein || (isally && optr->IsWall && (warhead->IsWallDestroyer || (warhead->IsWoodDestroyer && optr->Armor == ARMOR_WOOD)))) {
				int primary = What_Weapon_Should_I_Use(&Map[cell]);
				if (Can_Player_Move()) {
					return(ACTION_ATTACK);
				}
				if (In_Range(Coord(cell, 0), primary)) {
					return(ACTION_ATTACK);
				}
			}
		}
	}

	/*
	**	If the object can enter the cell specified, then allow
	**	movement to it.
	*/
	if (House->Is_Player_Control() && (Can_Player_Move() || Is_Move_Override())) {

		if (!Map.In_Local_Radar(cell, true)) {
			return(ACTION_NOMOVE);
		}

		if (shiftdown) {
			return(ACTION_MOVE);
		}

		if (Is_Move_Override()) {
			if (altdown) {
				return(ACTION_RALLY_TO_POINT);
			}
			if (!Can_Player_Move()) {
				return(ACTION_NONE);
			}
		}

		if (Can_Player_Move()) {
			if (Can_Enter_Cell(&Map[cell], FACING_NONE, -1, 0, true) <= MOVE_CLOAK) {
				return(ACTION_MOVE);
			}
			if (check_fog) {
				return(ACTION_MOVE);
			}
			if (Techno_Type_Class()->IsSubterranean) {
				if (Can_Enter_Cell(&Map[cell], FACING_NONE, -1, 0, false) <= MOVE_CLOAK) {
					return(ACTION_MOVE);
				}
			}
		}
		return(ACTION_NOMOVE);
	}

	return(ACTION_NONE);
}


/***********************************************************************************************
 * TechnoClass::Can_Player_Move -- Determines if the object can move be moved by player.       *
 *                                                                                             *
 *    Use this routine to determine whether a movement order can be given to this object.      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Can this object be given a movement order by the player?                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoClass::Can_Player_Move(void) const
{
	if (House->Is_Player_Control() && !Is_Immobilized()) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Determines whether this object is currently able to attack.
/// For units, returns false if it must deploy to fire while in a tube, or if it is immobilized.
/// </summary>
/// <returns>bool; can the object attack right now?</returns>
bool TechnoClass::Can_Attack_Now(void) const
{
	UnitClass const * unit = dynamic_cast<UnitClass const *>(this);
	if (unit != NULL) {
		if ((unit->CurrentTube >= TUBE_FIRST && unit->Deploy_To_Fire()) || unit->Is_Immobilized()) {
			return(false);
		}
	}
	return(true);
}


/// <summary>
/// Can this object deploy where it stands?
/// This routine is consulted before a deploy or undeploy order is accepted. A vehicle that
/// cannot move, that has nothing to deploy into, or that is riding a tunnel is refused, as
/// is a transport standing under a bridge. A building that is allowed to undeploy anywhere
/// overrides all of that, but nothing at all may deploy against a tunnel mouth.
/// </summary>
/// <returns>bool; Can the object deploy now?</returns>
bool TechnoClass::Can_Deploy_Now(void) const
{
	UnitClass const * unit = dynamic_cast<UnitClass const *>(this);
	bool blocked = false;

	if (unit != NULL) {
		blocked = unit->Is_Immobilized() || unit->CurrentTube >= TUBE_FIRST;
		if (unit->Class->DeploysInto == NULL && unit->Class->Max_Passengers() == 0 && !unit->Class->IsMobileEMP) {
			blocked = true;
		}
		if (unit->Class->Max_Passengers() > 0) {
			if (unit->IsOnBridge) {
				blocked = true;
			}
		}
		if (unit->Class->Max_Passengers() > 0) {
			Cell cell = PositionCell;
			CellClass * cellptr = &Map[cell];
			if (cellptr != NULL) {
				CellClass * cellptr_s = &Map[Adjacent_Cell(cell, FACING_S)];
				CellClass * cellptr_n = &Map[Adjacent_Cell(cell, FACING_N)];
				CellClass * cellptr_e = &Map[Adjacent_Cell(cell, FACING_E)];
				CellClass * cellptr_w = &Map[Adjacent_Cell(cell, FACING_W)];

				if (cellptr->IsUnderBridge ||
					cellptr_s != NULL && cellptr_s->IsUnderBridge ||
					cellptr_w != NULL && cellptr_w->IsUnderBridge ||
					cellptr_e != NULL && cellptr_e->IsUnderBridge ||
					cellptr_n != NULL && cellptr_n->IsUnderBridge) {

					blocked = true;
				}
			}
		}
	} else {
		blocked = Is_Immobilized();
		if (TClass->Max_Passengers() == 0) {
			blocked = true;
		}
	}

	BuildingClass const * building = dynamic_cast<BuildingClass const *>(this);
	if (building != NULL && building->Class->Can_Always_Undeploy()) {
		blocked = false;
	}

	Cell cell = PositionCell;
	CellClass * cellptr = &Map[cell];
	if ((cellptr != NULL && cellptr->Is_Near_Tunnel_NW()) || blocked) {
		return(false);
	}
	return(true);
}


/***********************************************************************************************
 * TechnoClass::Can_Player_Fire -- Determines if the player can give this object a fire order. *
 *                                                                                             *
 *    Call this routine to determine if this object can be given a fire order by the player.   *
 *    Such objects will affect the mouse cursor accordingly -- usually causes the targeting    *
 *    cursor to appear.                                                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Can this object be given firing orders by the player?                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoClass::Can_Player_Fire(void) const
{
	if (House->Is_Player_Control() && PrimaryWeapon != NULL && !Is_Immobilized()) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Determines whether this object is immobilized.
/// Returns true while the stun duration is greater than zero.
/// </summary>
/// <returns>bool; is the object currently immobilized?</returns>
bool TechnoClass::Is_Immobilized(void) const
{
	return(StunDuration > 0);
}


/***********************************************************************************************
 * TechnoClass::Is_Weapon_Equipped -- Determines if this object has a combat weapon.           *
 *                                                                                             *
 *    Use this routine to determine if this object is equipped with a combat weapon. Such      *
 *    determination is used by the AI system to gauge the threat potential of the object.      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is this object equipped with a combat weapon?                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoClass::Is_Weapon_Equipped(void) const
{
	return(PrimaryWeapon != NULL);
}


/***********************************************************************************************
 * TechnoClass::Can_Repair -- Determines if the object can and should be repaired.             *
 *                                                                                             *
 *    Use this routine to determine if the specified object is a candidate for repair. In      *
 *    order to qualify, the object must be allowed to be repaired (in theory) and it must      *
 *    be below full strength. If these conditions are met, then it can be repaired.            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; May this unit be repaired? A return value of false may mean that the object  *
 *                is not allowed to be repaired, or it might be full strength already.         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoClass::Can_Repair(void) const
{
	/*
	**	Temporary hack to disable repair cursor over non-buildings.
	*/
	if (RTTI != RTTI_BUILDING) {
		return(false);
	}
	return(TClass->IsRepairable && Strength != Class_Of()->MaxStrength || LimpetType);
}


/***********************************************************************************************
 * TechnoClass::Weapon_Range -- Determines the maximum range for the weapon.                   *
 *                                                                                             *
 *    Use this routine to determine the maximum range for the weapon indicated.                *
 *                                                                                             *
 * INPUT:   which -- Which weapon to use when determining the range. 0=primary, 1=secondary.   *
 *                                                                                             *
 * OUTPUT:  Returns with the range of the weapon (in leptons).                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoClass::Weapon_Range(int which) const
{
	assert((unsigned)which < 2);

	WeaponTypeClass const * weapon = Get_Class_Weapon_Data(which)->Weapon;

	if (weapon != NULL) {
		return(weapon->Range);
	}
	return(0);
}

/***************************************************************************
 * TechnoClass::Override_Mission -- temporarily overrides a units mission  *
 *                                                                         *
 *                                                                         *
 *                                                                         *
 * INPUT:      MissionType mission - the mission we want to override       *
 *               TARGET      tarcom  - the new target we want to override  *
 *               TARGET      navcom  - the new navigation point to override*
 *                                                                         *
 * OUTPUT:      none                                                       *
 *                                                                         *
 * WARNINGS:   If a mission is already overridden, the current mission is  *
 *               just re-assigned.                                         *
 *                                                                         *
 * HISTORY:                                                                *
 *   04/28/1995 PWG : Created.                                             *
 *=========================================================================*/
void TechnoClass::Override_Mission(MissionType mission, AbstractClass * tarcom, AbstractClass * navcom)
{
	// Foot units record their own override before calling here.
	if (Fetch_RTTI() == RTTI_BUILDING) {
		Sync_Record_Mission(*this, CurrentMission, mission, SYNC_MISSION_OVERRIDE, (unsigned)(uintptr_t)_ReturnAddress());
	}

	SuspendedTarCom = TarCom;
	BASECLASS::Override_Mission(mission, tarcom, navcom);
	Assign_Target(tarcom);
}


/***************************************************************************
 * TechnoClass::Restore_Mission -- Restores an overridden mission          *
 *                                                                         *
 * INPUT:      none                                                        *
 *                                                                         *
 * OUTPUT:     none                                                        *
 *                                                                         *
 * WARNINGS:   none                                                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   04/28/1995 PWG : Created.                                             *
 *=========================================================================*/
bool TechnoClass::Restore_Mission(void)
{
	if (BASECLASS::Restore_Mission()) {
		Assign_Target(SuspendedTarCom);
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * TechnoClass::Renovate -- Heal a building to maximum                                         *
 *                                                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/15/1996 BWG : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Renovate(void)
{
	Mark(MARK_CHANGE);
	Strength = TClass->MaxStrength;
	if (RTTI == RTTI_BUILDING) {
		((BuildingClass *)this)->Repair(0);
		((BuildingClass *)this)->Set_Anim_Damage_State(HealthRatio <= Rule->ConditionYellow);
	}
}


/***********************************************************************************************
 * TechnoClass::Captured -- Handles capturing this object.                                     *
 *                                                                                             *
 *    This routine is called when this object gets captured by the house specified. It handles *
 *    removing this object from any targeting computers and then changes the ownership of      *
 *    the object to the new house.                                                             *
 *                                                                                             *
 * INPUT:   newowner -- Pointer to the house that is now the new owner.                        *
 *                                                                                             *
 * OUTPUT:  Was the object captured? Failure would mean that it is already under control of    *
 *          the house specified.                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *   09/29/1995 JLB : Keeps track of quantity records.                                         *
 *=============================================================================================*/
bool TechnoClass::Captured(HouseClass * newowner)
{
	if (newowner != House) {

		Assign_Target(NULL);
		Assign_Destination(NULL);

		/*
		**	Capture attempt springs any "entered" trigger. The entered trigger
		**	occurs first since there may be a special trigger attached to this
		**	object that flags a capture as a win and a destroy as a loss. This
		**	order is necessary because the object is recorded as a kill as well.
		*/
		if (Tag != NULL) {
			Tag->Spring(TEVENT_PLAYER_ENTERED, this);
		}

		House->Tracking_Active_Remove(this, false);

		/*
		**	Record this as a kill.
		*/
		Record_The_Kill(NULL);

		/*
		**	Special kill record logic for capture process.
		*/
		newowner->PointTotal += TClass->Cost_Of(House);
		House->Tracking_Remove(this);
		newowner->Tracking_Add(this);
		switch ((RTTIType)RTTI) {
			case RTTI_BUILDING:
				newowner->BuildingsKilled[Owner()]++;
				break;

			case RTTI_AIRCRAFT:
			case RTTI_INFANTRY:
			case RTTI_UNIT:
				newowner->UnitsKilled[Owner()]++;
				break;

			default:
				break;
		}
		House->WhoLastHurtMe = newowner->Class->House;

		if (!IsInLimbo) {
			Mark(MARK_UP);
		}

		/*
		**	Remove from targeting computers.
		*/
		Detach_All(false);

		if (!IsInLimbo) {
			Mark(MARK_DOWN_FORCED);
		}

		if (!IsInLimbo) {
			CellClass * cptr = !Is_Foot() ? &Map[Get_Coord()] : &Map[((FootClass *)this)->LastAdjacencyCell];
			cptr->Adjust_Threat(House->HeapID, -Risk());
			cptr->Adjust_Threat(newowner->HeapID, Risk());
		}

		/*
		**	Change ownership now.
		*/
		House = newowner;
		IsOwnedByPlayer = (House == PlayerPtr);

		newowner->Tracking_Active_Add(this, true);

		if (!IsInLimbo) {
			if (RTTI != RTTI_BUILDING || CurrentMission != MISSION_UNLOAD || !((BuildingClass *)this)->Class->IsWeaponsFactory) {
				RadioClass * radio = Contact_With_Whom();
				if (radio == NULL || radio->RTTI != RTTI_BUILDING || !((BuildingClass *)radio)->Class->IsWeaponsFactory) {
					Enter_Idle_Mode();
				}
			}
			Radar_Untrack();
			Radar_Track();
		}

		return(true);
	}
	return(false);
}


/// <summary>
/// Reassigns this object to a new owning house.
/// Updates the player-ownership flag based on whether the new owner is the local player.
/// </summary>
/// <param name="newowner">The house that will now own this object.</param>
void TechnoClass::Set_Owner(HouseClass * newowner)
{
	House = newowner;
	IsOwnedByPlayer = newowner == PlayerPtr;
}


/***********************************************************************************************
 * TechnoClass::Take_Damage -- Records damage assessed to this object.                         *
 *                                                                                             *
 *    This routine is called when this object has taken damage. It handles recording whether   *
 *    this object has been destroyed. If it has, then mark the appropriate kill records as     *
 *    necessary.                                                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/20/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ResultType TechnoClass::Take_Damage(int & damage, int distance, WarheadTypeClass const * warhead, TechnoClass * source, bool forced, bool no_crew)
{
	bool negative = damage < 0;

	/*
	 * If not a forced damage condition, adjust damage according to the house and
	 * object armor bias, veterancy armor bonus, and type-immunity.
	 */
	if (!forced && damage > 0) {
		damage = (int)(1.0 / (House->ArmorBias * ArmorBias) * (double)damage);

		if (Has_Ability(ABILITY_STRONGER)) {
			damage = (int)(1.0 / (Rule->VeteranArmor + 1.0) * (double)damage);
		}

		if (damage < 1) {
			damage = 1;
		}

		/*
		 * Type-immune objects take no damage from another object of the same type
		 * owned by the same house.
		 */
		if (source != NULL) {
			if (TClass->IsTypeImmune) {
				if (TClass == source->TClass && House == source->House) {
					return(RESULT_NONE);
				}
			}
		}
	}

	/*
	 * Negative damage (healing) resets the limpet drain effect and restores the
	 * normal rate of turn for the turret and body.
	 */
	if (negative == true) {
		LimpetType = 0;
		LimpetSpeedFactor = 0.0;
		PrimaryFacing.Set_ROT(TClass->ROT);
		SecondaryFacing.Set_ROT(TClass->ROT);
	}

	ResultType result = (ResultType)ObjectClass::Take_Damage(damage, distance, warhead, source, forced, no_crew);

	/*
	 * Inform the owning house of the anger level this damage produced.
	 */
	if (source != NULL) {
		House->Add_Anger((int)((double)TClass->Raw_Cost() * ((double)damage / (double)(int)TClass->MaxStrength)), source->House);
	}

	if (result == RESULT_ALREADY_DESTROYED) {
		return(RESULT_ALREADY_DESTROYED);
	}

	/*
	 * Any damage that was actually registered flashes this object on the radar.
	 */
	if (result != RESULT_NONE) {
		RadarFlashTimer = Rule->RadarCombatFlashTime;
	}

	/*
	 * If the object's strength has reached zero it is destroyed regardless of the
	 * result reported by the base class.
	 */
	if (Strength == 0) {
		result = RESULT_DESTROYED;
	}

	switch (result) {
		case RESULT_DESTROYED:

			/*
			 * Play the death voice response.
			 */
			if (TClass->VoiceDie.Count() > 0) {
				VocType voc = (VocType)TClass->VoiceDie.Pick(NonCriticalRandomNumber());
				Sound_Effect(voc, Get_Coord());
			}

			Transmit_Message(RADIO_OVER_OUT);
			Stun();

			/*
			 * Tiberium-healing objects spew tiberium into the adjacent cells when destroyed.
			 */
			if (TClass->IsTiberiumHeal) {
				static FacingType _heal_facing[] = {FACING_NONE, FACING_N, FACING_E, FACING_S, FACING_W};

				Cell center = Center_Coord().As_Cell();
				for (int index = 0; index < ARRAY_SIZE(_heal_facing); index++) {
					Cell cell = Adjacent_Cell(center, _heal_facing[index]);
					CellClass * cellptr = &Map[cell];
					cellptr->Place_Tiberium(TIBERIUM_RIPARIUS, Scen->RandomNumber(0, 2));
				}
			}

			/*
			 * Clean up the primary attached particle system.
			 */
			if (ParticleSystems[0] != NULL) {
				ParticleSystems[0]->Delete_Me();
				ParticleSystems[0] = NULL;
			}

			/*
			 * If destroyed while in/near the water and flagged to explode, bail out so the
			 * splash logic (handled elsewhere) is not stomped by debris.
			 */
			if (HeightAGL <= 10) {
				if (IsToExplode) {
					if (Map[Get_Coord()].Land_Type() == LAND_WATER) {
						break;
					}
				}
			}

			/*
			 * Spawn destruction debris -- either the explicit voxel debris list (with
			 * per-type maximums) or generic metallic debris animations.
			 */
			if (TClass->MaxDebris > 0) {
				if (TClass->DebrisTypes.Count() > 0) {
					int remaining = TClass->MaxDebris;
					for (int index = 0; remaining > 0; index++) {
						if (index >= TClass->DebrisTypes.Count()) {
							break;
						}

						int count = abs(Scen->RandomNumber) % (TClass->DebrisMaximums[index] + 1);
						if (count >= remaining) {
							count = remaining;
						}
						for (int j = 0; j < count; j++) {
							new VoxelAnimClass(TClass->DebrisTypes[index], Center_Coord(), House);
						}
						remaining -= count;
					}
				} else {
					int count = Scen->RandomNumber(0, TClass->MaxDebris);
					for (int index = 0; index < count; index++) {
						new AnimClass(Rule->MetallicDebris[Scen->RandomNumber(0, Rule->MetallicDebris.Count() - 1)], Center_Coord() + Coord(0, 0, 20), 0, 1, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL), 0);
					}
				}
			}

			/*
			 * Determine whether this object performs a violent collateral explosion on death.
			 * Either the type is flagged exploding, or a veteran/elite has the explodes ability.
			 */
			if (!TClass->IsExploding && !Has_Ability(ABILITY_EXPLODES)) {
				break;
			}

			{
				/*
				**	The warhead to use is based on the weapon this object is equipped with.
				*/
				WarheadTypeClass const * wh = NULL;
				if (Get_Class_Weapon_Data(0)->Weapon != NULL) {
					wh = Get_Class_Weapon_Data(0)->Weapon->WarheadPtr;
				}

				int colat_damage = Get_Collateral_Damage();
				AnimTypeClass const * anim = Combat_Anim(colat_damage, wh, Map[Center_Coord()].Land_Type(), Get_Coord());
				Combat_Lighting(Center_Coord(), colat_damage, wh, false);
				if (anim != NULL) {
					new AnimClass(anim, Center_Coord(), 0, 1, ShapeFlags_Type(SHAPE_ZGRAD|SHAPE_WIN_REL|SHAPE_CENTER), Get_Explosion_Z(Center_Coord()));
				}

				int radius = (int)((double)(colat_damage / 100) / Rule->ExplosionSpread * CELL_LEPTON);
				if (radius >= 3 * CELL_LEPTON) {
					radius = 3 * CELL_LEPTON;
				} else if (radius <= 1) {
					radius = 1;
				}

				int cells = radius / CELL_LEPTON;
				if (cells < 1) {
					cells = 1;
				}
				Wide_Area_Damage(Center_Coord(), radius, colat_damage * cells, source, wh);

				/*
				 * A destroyed harvester scatters its tiberium cargo into the surrounding cells.
				 */
				if (Storage.Get_Total_Amount() <= 0 || Fetch_RTTI() == RTTI_BUILDING || Scen->Special.IsHarvesterImmune) {
					break;
				}

				static FacingType _scatter_facing[] = {FACING_NONE, FACING_E, FACING_NW, FACING_NE, FACING_S, FACING_SE, FACING_N, FACING_SW, FACING_W};

				int amount = (int)((double)Storage.Get_Total_Amount() / (double)TClass->Capacity * 9.0);
				Cell center = Center_Coord().As_Cell();
				for (int index = 0; index < ARRAY_SIZE(_scatter_facing); index++) {
					Cell cell = Adjacent_Cell(center, _scatter_facing[index]);
					CellClass * cellptr = &Map[cell];
					cellptr->Place_Tiberium(TIBERIUM_RIPARIUS, Scen->RandomNumber(0, 2));
					amount--;
					if (amount <= 0) {
						break;
					}
				}
				break;
			}

		case RESULT_HALF: {

			/*
			 * Half-strength transition may trigger a feedback voice response.
			 */
			TechnoTypeClass const * ttype = TClass;
			if (ttype->VoiceFeedback.Count() > 0 && Scen->RandomNumber(0, 99) < 30) {
				VocType voc = (VocType)ttype->VoiceFeedback.Pick(Scen->RandomNumber());
				Sound_Effect(voc, Center_Coord());
			}
			break;
		}

		case RESULT_NONE:
		case RESULT_MAJOR:
			break;

		default:

			/*
			 * Protected or rescue-needing AI objects notify their base when attacked.
			 */
			if ((TClass->IsToProtect || IsNeedingRescue) && !House->Is_Human_Player()) {
				if (source != NULL) {
					Base_Is_Attacked(source);
				}
			}
			break;
	}

	if (result != RESULT_DESTROYED) {
		/*
		**	If some damage was received and this object is cloaked, shimmer
		**	the cloak a bit.
		*/
		if (source != NULL) {
			if (!House->Is_Ally(source)) {
				IsTickedOff = true;
			}
		}
		Do_Shimmer();

		if (HealthRatio > Rule->ConditionYellow) {
			if (ParticleSystems[3] != NULL) {
				ParticleSystems[3]->Delete_Me();
			}
		} else {
			if (result == RESULT_HALF || result == RESULT_MAJOR) {
				DynamicVectorClass<ParticleSystemTypeClass const *> systems;
				systems.Set_Growth_Step(10);

				for (int index = TClass->DamageParticleSystems.Count() - 1; index >= 0; index--) {
					if (TClass->DamageParticleSystems[index]->Behaves_Like() == PSYS_BEHAVIOR_SMOKE) {
						systems.Add(TClass->DamageParticleSystems[index]);
					}
				}

				if (ParticleSystems[3] == NULL && systems.Count() > 0 && HeightAGL > -10) {
					Coord spawn = TClass->DamageSmokeOffset;
					ParticleSystems[3] = new ParticleSystemClass(systems[Scen->RandomNumber(0, systems.Count() - 1)], Get_Coord() + spawn, NULL, this, COORD_NONE);
				}
			}
		}

		if (!negative) {
			AbstractClass * target = source;

			if (Is_Allowed_To_Retaliate(source, warhead)) {

				if (source == NULL) {
					if (warhead->IsVeinhole) {
						target = VeinholeMonsterClass::Get_Vein_Owner_At(Destination_Coord().As_Cell());
					} else {
						target = NULL;
					}
				}

				if (target != NULL) {
					int which = What_Weapon_Should_I_Use(target);
					bool retaliate = In_Range(target, which);
					if (!retaliate) {
						if (House->Is_Human_Player()) {
							retaliate = ((double)(int)Distance(target->Center_Coord()) <= ((double)TClass->SightRange + 0.5) * CELL_LEPTON);
						} else {
							retaliate = true;
						}
					}
					if (retaliate) {
						Override_Mission(MISSION_ATTACK, target, NULL);
					}
				}

				FootClass * foot = dynamic_cast<FootClass *>(this);
				if (foot != NULL) {
					if (TarCom == NULL && foot->NavCom == NULL) {
						if (Rule->IsScatter || Has_Ability(ABILITY_SCATTER)) {
							Scatter(COORD_NONE, true, false);
						}
					}
				}

			} else {

				FootClass * foot = dynamic_cast<FootClass *>(this);
				if (foot != NULL) {
					if (Current_Mission_Control().IsScatter) {
						if (!IsTethered) {
							if (!foot->Locomotion->Is_Moving() && TarCom == NULL && foot->NavCom == NULL && Fetch_RTTI() != RTTI_AIRCRAFT) {
								if (!House->Is_Human_Player() || Rule->IsScatter || Has_Ability(ABILITY_SCATTER)) {
									Scatter(COORD_NONE, true, false);
								}
							}
						}
					}
				}
			}
		}
	}
	return(result);
}


/***********************************************************************************************
 * TechnoClass::Record_The_Kill -- Records the death of this object.                           *
 *                                                                                             *
 *    This routine is used to record the death of this object. It will handle updating the     *
 *    owner house with the kill record as well as springing any trigger events associated with *
 *    this object's death.                                                                     *
 *                                                                                             *
 * INPUT:   source   -- Pointer to the source of this object's death (if there is a source).   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *   08/23/1995 JLB : Building loss is only counted if it received damage.                     *
 *=============================================================================================*/
void TechnoClass::Record_The_Kill(TechnoClass * source)
{
	int total_recorded = 0;

	int points = TClass->Cost_Of(House);

	/*
	**	Handle any trigger event associated with this object.
	*/
	if (IsActive && Tag && source) Tag->Spring(TEVENT_ATTACKED, this);

	if (IsActive && Tag && source) Tag->Spring(TEVENT_DISCOVERED, this);

	if (IsActive && RTTI != RTTI_UNIT) {

		if (IsActive && Tag && source) Tag->Spring(TEVENT_DESTROYED, this);

		if (IsActive && Tag) Tag->Spring(TEVENT_DESTROYED_ANY, this);

		if (IsActive && Tag) Tag->Spring(TEVENT_DESTROYED_ANY_X, this);
	}

	if (source != NULL) {
		if (source->TClass->IsTrainable && !House->Is_Ally(source)) {
			source->Veterancy.Made_A_Kill(source->TClass->Cost_Of(House), points);
		}

		House->WhoLastHurtMe = source->Owner();

		/*
		**	Add up the score for killing this unit
		*/
		source->House->PointTotal += points;
	}

	switch ((RTTIType)RTTI) {
		case RTTI_BUILDING:
			{
				if (!TClass->IsInsignificant) {
					if (((BuildingClass *)this)->WhoLastHurtMe != HOUSE_NONE) {
						House->BuildingsLost++;
					}

					if (source != NULL) {
						if (Session.Type == GAME_INTERNET) {
							source->House->DestroyedBuildings->Increment_Unit_Total(((BuildingClass*)this)->Class->HeapID);
						}
						source->House->BuildingsKilled[Owner()]++;
					}

					/*
					**	If the map is displaying the multiplayer player names & their
					** # of kills, tell it to redraw.
					*/
					if (Map.Is_Player_Names()) {
						Map.Redraw_Radar(false);
					}
				}
			}
			break;

		case RTTI_AIRCRAFT:
			if (source != NULL && Session.Type == GAME_INTERNET) {
				source->House->DestroyedAircraft->Increment_Unit_Total(((AircraftClass*)this)->Class->HeapID);
				total_recorded++;
			}
			//Fall through.....
		case RTTI_INFANTRY:
			if (source != NULL && !total_recorded && Session.Type == GAME_INTERNET) {
				source->House->DestroyedInfantry->Increment_Unit_Total(((InfantryClass*)this)->Class->HeapID);
				total_recorded++;
			}
			//Fall through.....
		case RTTI_UNIT:
			if (source != NULL && !total_recorded && Session.Type == GAME_INTERNET) {
				source->House->DestroyedUnits->Increment_Unit_Total(((UnitClass*)this)->Class->HeapID);
				total_recorded++;
			}


			House->UnitsLost++;
			if (source != NULL) source->House->UnitsKilled[Owner()]++;

			/*
			**	If the map is displaying the multiplayer player names & their
			** # of kills, tell it to redraw.
			*/
			if (Map.Is_Player_Names()) {
				Map.Redraw_Radar(false);
			}
			break;

		default:
			break;
	}

	/*
	**	Since we lost an object, we lose the associated points as well.
	*/
	//House->PointTotal -= points;
}


/***********************************************************************************************
 * TechnoClass::Nearby_Location -- Radiates outward looking for clear cell nearby.             *
 *                                                                                             *
 *    This routine is used to find a nearby location from center of this object. It can lean   *
 *    toward finding a location closest to an optional object.                                 *
 *                                                                                             *
 * INPUT:   object   -- Optional object that the finding algorithm will try to find a close    *
 *                      spot to.                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the cell that is closest to this object.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1995 JLB : Created.                                                                 *
 *   09/28/1995 JLB : Uses map scan function.                                                  *
 *=============================================================================================*/
Cell TechnoClass::Nearby_Location(TechnoClass const * techno) const
{
	SpeedType speed = TClass->Speed;
	if (speed == SPEED_WINGED) {
		speed = SPEED_TRACK;
	}

	Cell cell;
	if (techno != NULL) {
		cell = techno->Center_Coord().As_Cell();
	} else {
		cell = Center_Coord().As_Cell();
	}

	MZoneType mzone = TClass->MZone;
	int zone = mzone != MZONE_NONE ? Map.Get_Cell_Zone(cell, mzone, IsOnBridge) : -1;
	return(Map.Nearby_Location(cell, speed, zone, mzone, IsOnBridge));
}


/***********************************************************************************************
 * TechnoClass::Do_Uncloak -- Cause the stealth tank to uncloak.                               *
 *                                                                                             *
 *    This routine will start the stealth tank to uncloak.                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Do_Uncloak(bool silent)
{
	CloakDelay = Rule->CloakDelay * TICKS_PER_MINUTE;

	if (Cloak == CLOAKED || Cloak == CLOAKING) {
		Cloak = UNCLOAKING;
		CloakingDevice.Set_Stage(Rule->CloakingStages - 1);
		CloakingDevice.Set_Rate(TClass->CloakingSpeed);
		CloakingDevice.Set_Step(-1);
		if (!silent) {
			Sound_Effect(Rule->CloakSound, PositionCoord);
		}
	}
}


/***********************************************************************************************
 * TechnoClass::Do_Cloak -- Start the object into cloaking stage.                              *
 *                                                                                             *
 *    This routine will start the object into its cloaking state.                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Do_Cloak(bool silent)
{
	if (Cloak == UNCLOAKED || Cloak == UNCLOAKING) {
		Detach_All(false);
		Cloak = CLOAKING;
		CloakingDevice.Set_Stage(0);
		CloakingDevice.Set_Rate(TClass->CloakingSpeed);
		CloakingDevice.Set_Step(1);
		if (!silent) {
			Sound_Effect(Rule->CloakSound, PositionCoord);
		}
	}
}


/***********************************************************************************************
 * TechnoClass::Do_Shimmer -- Causes this object to shimmer if it is cloaked.                  *
 *                                                                                             *
 *    This routine is called when this object should shimmer. If the object is cloaked, then   *
 *    a shimmering effect (partial decloak) occurs. For objects that are not cloaked, no       *
 *    effect occurs.                                                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Do_Shimmer(void)
{
	Do_Uncloak();
}


/***********************************************************************************************
 * TechnoClass::Visual_Character -- Determine the visual character of the object.              *
 *                                                                                             *
 *    This routine will determine how this object should be drawn. Typically, this is the      *
 *    unmodified visible state, but cloaked objects have a different character.                *
 *                                                                                             *
 * INPUT:   raw   -- Should the check be based on the unmodified cloak condition of the        *
 *                   object? If false, then an object owned by the player will never become    *
 *                   completely invisible.                                                     *
 *                                                                                             *
 * OUTPUT:  Returns with the visual character to use when displaying this object.              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/07/1995 JLB : Created.                                                                 *
 *   05/27/1996 JLB : Knows about invisible objects.                                           *
 *=============================================================================================*/
VisualType TechnoClass::Visual_Character(bool raw, HouseClass const * house) const
{
	if (TClass->IsInvisible && IsOwnedByPlayer) return(VISUAL_NORMAL);
	if (TClass->IsInvisible && !IsOwnedByPlayer && !Debug_Map) return(VISUAL_HIDDEN);

	/*
	**	When uncloaked or in map editor mode, always draw the object normally.
	*/
	if (Cloak == UNCLOAKED || Debug_Map || RTTI == RTTI_BUILDING) return(VISUAL_NORMAL);

	/*
	**	A cloaked unit will not be visible at all unless it is owned
	**	by the player.
	*/
	if (Cloak == CLOAKED) {
		if (raw && house != NULL && Map[Get_Coord().As_Cell()].Is_Sensed(house->HeapID)) return(VISUAL_SHADOWY);
		if (!raw && !MainWindow) return(VISUAL_SHADOWY);
		if (!raw && IsOwnedByPlayer) return(VISUAL_SHADOWY);
		if (!raw && Is_Sensed_By_Player()) return(VISUAL_SHADOWY);
		if (!raw && (Session.Type != GAME_NORMAL && House != NULL && PlayerPtr != NULL && PlayerPtr->Shares_View_With(House) && House->Shares_View_With(PlayerPtr))) return(VISUAL_SHADOWY);
		return(VISUAL_HIDDEN);
	}

	int stage = CloakingDevice.Fetch_Stage();
	if (stage <= 0) {
		return(VISUAL_NORMAL);
	}

	stage = double(stage) / Rule->CloakingStages * 256;

	if (stage < 0x0040) return(VISUAL_INDISTINCT);
	if (stage < 0x0080) return(VISUAL_DARKEN);
	if (stage < 0x00C0) return(VISUAL_SHADOWY);
	if (!raw && IsOwnedByPlayer) return(VISUAL_SHADOWY);
	if (stage < 0x00FF) return(VISUAL_RIPPLE);
	return(VISUAL_HIDDEN);
}


/// <summary>
/// Is this object under a bridge?
/// An object counts as under a bridge when its own cell is covered, or when it stands close
/// enough alongside one that the deck still passes over it. This routine feeds the draw
/// depth calculation, so that the object sorts beneath the deck. An object riding the
/// bridge itself never counts.
/// </summary>
/// <returns>bool; Is the object under a bridge?</returns>
bool TechnoClass::Is_Z_Fudge_Bridge(void) const
{
	Cell cell = Get_Cell();
	CellClass * cptr = &Map[cell];

	if (cptr != NULL && !IsOnBridge) {
		CellClass * south = &Map[Adjacent_Cell(cell, FACING_S)];
		CellClass * north = &Map[Adjacent_Cell(cell, FACING_N)];
		CellClass * east = &Map[Adjacent_Cell(cell, FACING_E)];
		CellClass * west = &Map[Adjacent_Cell(cell, FACING_W)];

		if (cptr->IsUnderBridge ||
			(south != NULL && south->IsUnderBridge && south->IsBridgeEastWest) ||
			(west != NULL && west->IsUnderBridge && !west->IsBridgeEastWest) ||
			(east != NULL && east->IsUnderBridge && !east->IsBridgeEastWest) ||
			(north != NULL && north->IsUnderBridge && north->IsBridgeEastWest)) {

			return(true);
		}
	}

	return(false);
}


/// <summary>
/// Was this object under a bridge?
/// This is the previous frame companion to Is_Z_Fudge_Bridge, and the bridge column depth
/// fudge consults the two together. An object riding the bridge itself never counts.
/// </summary>
/// <returns>bool; Was the object under a bridge?</returns>
bool TechnoClass::Was_Z_Fudge_Bridge(void) const
{
	Cell cell = Get_Cell();
	CellClass * cptr = &Map[cell];

	if (cptr != NULL && !IsOnBridge) {
		CellClass * south = &Map[Adjacent_Cell(cell, FACING_S)];
		CellClass * north = &Map[Adjacent_Cell(cell, FACING_N)];
		CellClass * east = &Map[Adjacent_Cell(cell, FACING_E)];
		CellClass * west = &Map[Adjacent_Cell(cell, FACING_W)];

		if (cptr->WasUnderBridge ||
			(south != NULL && south->WasUnderBridge && south->IsBridgeEastWest) ||
			(west != NULL && west->WasUnderBridge && !west->IsBridgeEastWest) ||
			(east != NULL && east->WasUnderBridge && !east->IsBridgeEastWest) ||
			(north != NULL && north->WasUnderBridge && north->IsBridgeEastWest)) {

			return(true);
		}
	}

	return(false);
}


/// <summary>
/// Determines how far a bridge column should bias this object's depth.
/// This routine feeds the draw depth calculation for an object that is, or has just been,
/// under a bridge, so that it sorts behind the support columns rather than in front of
/// them.
/// </summary>
/// <returns>Returns with the column fudge strength, or zero if no column is in the way.</returns>
int TechnoClass::Get_Z_Fudge_Column(void) const
{
	Cell cell = Get_Cell();
	int fudge = 0;
	CellClass * cptr = &Map[cell];

	if (cptr != NULL && (Is_Z_Fudge_Bridge() || Was_Z_Fudge_Bridge())) {
		CellClass * south = &Map[Adjacent_Cell(cell, FACING_S)];
		CellClass * east = &Map[Adjacent_Cell(cell, FACING_E)];
		CellClass * south_east = &Map[Adjacent_Cell(cell, FACING_SE)];

		if (south != NULL && east != NULL && south_east != NULL) {
			if (south->ITType != TILE_NONE && south->ITType != ISOTILE_NONE_LEGACY) {
				int tile = south->ITType - IsometricTileTypeClass::BridgeSet + 1;
				if (tile >= 7 && tile <= BRIDGE_COUNT) {
					fudge = 1;
				}
			}
			if (east->ITType != TILE_NONE && east->ITType != ISOTILE_NONE_LEGACY) {
				int tile = east->ITType - IsometricTileTypeClass::BridgeSet + 1;
				if (tile >= 7 && tile <= BRIDGE_COUNT) {
					fudge = 1;
				}
			}
			if (south_east->ITType != TILE_NONE && south_east->ITType != ISOTILE_NONE_LEGACY) {
				int tile = south_east->ITType - IsometricTileTypeClass::BridgeSet + 1;
				if (tile >= 7 && tile <= BRIDGE_COUNT) {
					fudge++;
				}
			}
		}
	}

	return(fudge);
}


/// <summary>
/// Determines whether a tunnel mouth should bias this object's depth.
/// This routine feeds the draw depth calculation, so that an object standing at a tunnel
/// entrance sorts behind the tunnel art instead of in front of it. An object riding a
/// bridge is never fudged this way.
/// </summary>
/// <returns>Returns with one when the tunnel fudge applies, otherwise zero.</returns>
int TechnoClass::Get_Z_Fudge_Tunnel(void) const
{
	int fudge = 0;
	Cell cell = PositionCell;

	if (!IsOnBridge) {
		CellClass * north = &Map[Adjacent_Cell(cell, DIR_N)];
		CellClass * west = &Map[Adjacent_Cell(cell, DIR_W)];
		Cell tuncell = CELL_NONE;
		CellClass * own = &Map[cell];

		if (north != NULL && west != NULL) {
			if (own->Has_Tunnel()) {
				tuncell = own->Fetch_CellID();
			} else if (north->Has_Tunnel()) {
				tuncell = north->Fetch_CellID();
			} else if (west->Has_Tunnel()) {
				tuncell = west->Fetch_CellID();
			}

			if (tuncell != CELL_NONE) {
				north = &Map[Adjacent_Cell(tuncell, DIR_N)];
				west = &Map[Adjacent_Cell(tuncell, DIR_W)];

				if (north != NULL && west != NULL) {
					north = &Map[Adjacent_Cell(north->Fetch_CellID(), DIR_N)];
					west = &Map[Adjacent_Cell(west->Fetch_CellID(), DIR_W)];

					if (north != NULL && west != NULL) {
						if (west->Has_Tunnel() || north->Has_Tunnel()) {
							fudge = 1;
						}
					}
				}
			}
		}
	}

	return(fudge);
}


/// <summary>
/// Determines how far a cliff should bias this object's depth.
/// This routine feeds the draw depth calculation, so that an object standing at the foot of
/// a cliff sorts behind it rather than through it. An object riding a bridge is never
/// fudged this way.
/// </summary>
/// <returns>Returns with the cliff fudge strength, or zero if no cliff is in the way.</returns>
int TechnoClass::Get_Z_Fudge_Cliff(void) const
{
	int fudge = 0;
	Cell cell = Get_Cell();
	CellClass * cptr = &Map[cell];

	if (!IsOnBridge) {
		CellClass * south_east = &Map[Adjacent_Cell(Get_Cell(), FACING_SE)];
		if (south_east != NULL) {
			if (south_east->Height - cptr->Height >= 4) {
				fudge = 2;
			}
			south_east = &Map[Adjacent_Cell(south_east->Fetch_CellID(), FACING_SE)];
			if (south_east != NULL) {
				if (south_east->Height - cptr->Height >= 4) {
					fudge = 1;
				}
			}
		}
	}

	return(fudge);
}


/// <summary>
/// Fetches this object's draw depth adjustment.
/// This routine biases the object's depth so that it sorts sensibly against whatever it is
/// standing on or next to -- a harvester backed into a refinery, a vehicle on or beside a
/// ramp, a rock or a tall tile in the way. FootClass::Get_Z_Adjust combines this with the
/// locomotor's own bias and with any bridge, tunnel or cliff fudge that applies.
/// </summary>
/// <returns>Returns with the amount to bias this object's depth by when it is drawn.</returns>
int TechnoClass::Get_Z_Adjust(void) const
{
	static int _techno_zadj_ramp1 = -1;
	static int _techno_zadj_ramp2 = -1;
	static int _techno_zadj_rock = 3;

	CellClass *cptrs[3];
	int width;
	int height;

	int z = -TacticalMap->Z_Lepton_To_Pixel(Get_Height());
	int zadjust2 = z;
	int zadjust = z;
	Cell cell = PositionCell;
	CellClass *cptr = &Map[cell];
	UnitClass *unit = (UnitClass *)this;

	if (unit->RTTI == RTTI_UNIT) {
		if (unit->IsTethered) {
			BuildingClass *building = (BuildingClass *)unit->Contact_With_Whom();
			if (building->RTTI == RTTI_BUILDING && building->Mission == MISSION_UNLOAD) {
				return(zadjust2 - 3);
			}
		}
		if (unit->Class->IsToHarvest) {
			BuildingClass *building = Get_Cell_Ptr()->Cell_Building();
			if (building != NULL && building->Class->IsRefinery) {
				return(zadjust2 - 14);
			}
		}
	}

	if (cptr->Ramp) {
		CellClass *cell_s = &Map[Adjacent_Cell(cell, FACING_S)];
		CellClass *cell_e = &Map[Adjacent_Cell(cell, FACING_E)];
		CellClass *cell_se = &Map[Adjacent_Cell(cell, FACING_SE)];

		if (cell_s != NULL && cell_e != NULL && cell_se != NULL) {

			if (cell_s->Overlay != OVERLAY_NONE) {
				if (OverlayTypes[cell_s->Overlay]->IsARock) {
					return(_techno_zadj_rock + zadjust2 - 1);
				}
			}
			else if (cell_e->Overlay != OVERLAY_NONE) {
				if (OverlayTypes[cell_e->Overlay]->IsARock) {
					return(_techno_zadj_rock + zadjust2 - 1);
				}
			}
			else if (cell_se->Overlay != OVERLAY_NONE) {
				if (OverlayTypes[cell_se->Overlay]->IsARock) {
					return(_techno_zadj_rock + zadjust2 - 1);
				}
			}
		}
	}

	CellClass *ahead_right = &Map[Adjacent_Cell(cell, Facing_Add(PrimaryFacing.Current().As_Dir8(), FACING_45))];
	CellClass *ahead_left = &Map[Adjacent_Cell(cell, Facing_Sub(PrimaryFacing.Current().As_Dir8(), FACING_45))];

	CellClass *ahead = &Map[Adjacent_Cell(cell, Facing_Add(PrimaryFacing.Current().As_Dir8(), FACING_0))];
	CellClass *behind = &Map[Adjacent_Cell(cell, Facing_Sub(PrimaryFacing.Current().As_Dir8(), FACING_180))];

	CellClass *behind_right = &Map[Adjacent_Cell(cell, Facing_Add(PrimaryFacing.Current().As_Dir8(), FACING_135))];
	CellClass *behind_left = &Map[Adjacent_Cell(cell, Facing_Sub(PrimaryFacing.Current().As_Dir8(), FACING_135))];

	if (ahead_right != NULL && ahead_left != NULL && ahead != NULL) {

		if (ahead_right->Ramp == 0 && ahead_left->Ramp == 0 && ahead->Ramp == 0) {

			if (behind != NULL && behind_right != NULL && behind_left != NULL) {

				if (behind->Ramp == 0 && behind_right->Ramp == 0 && behind_left->Ramp == 0) {

					if ( cptr->Ramp )
					{
						return(zadjust + _techno_zadj_ramp1);
					}

					if (PrimaryFacing.Current().As_Dir8() == FACING_W || PrimaryFacing.Current().As_Dir8() == FACING_N) {
						cptrs[0] = behind;
						cptrs[1] = behind_right;
						cptrs[2] = behind_left;
					}
					else if (PrimaryFacing.Current().As_Dir8() == FACING_S || PrimaryFacing.Current().As_Dir8() == FACING_E) {
						cptrs[0] = ahead_right;
						cptrs[1] = ahead_left;
						cptrs[2] = ahead;
					} else {
						return(zadjust - 1);
					}

					if (!cptr->IsOvershadowed) {
						for (int i = 0; i < ARRAY_SIZE(cptrs); i++) {
							if (cptrs[i]->IsOvershadowed) {
								return(zadjust - 1);
							}
						}

						for (int j = 0; j < ARRAY_SIZE(cptrs); j++) {
							IsometricTileType ittype = cptrs[j]->ITType;

							int subtile;
							IsometricTileTypeClass *iptr;

							if (ittype == ISOTILE_NONE || ittype == ISOTILE_NONE_LEGACY) {
								subtile = 0;
								iptr = IsometricTileTypes[TILE_CLEAR];
							} else {
								iptr = IsometricTileTypes[ittype];
								subtile = cptrs[j]->SubTile;
							}

							iptr->Get_Tile_Pixel_Dimensions(subtile, width, height);
							if (height > 36) {
								return(zadjust - 2);
							}
						}
					}
					//return zadjust - 1;
				} else {
					return(_techno_zadj_ramp2 + zadjust);
				}
			}
		} else {
			return(_techno_zadj_ramp2 + zadjust);
		}
	}

	return(zadjust - 1);

	#if 0
	index = 0;
	for ( i = cptrs; ; ++i )
	{
		ittype = (*i)->ITType;
		if ( ittype == ISOTILE_NONE || ittype == TILE_UNKNOWN_BRIDGE_INDEX )
		{
			subtile = 0;
			iptr = IsometricTileTypes.Vector[TILE_CLEAR];
		}
		else
		{
			iptr = IsometricTileTypes.Vector[ittype];
			subtile = (*i)->SubTile;
		}
		IsometricTileTypeClass::Get_Tile_Pixel_Dimensions(iptr, subtile, &width, &height);
		if ( height > 36 )
		{
			break;
		}
		if ( ++index >= 3 )
		{
			return(zadjust - 1);
		}
	}
	#endif
	return(zadjust - 2);
}


/***********************************************************************************************
 * TechnoClass::Techno_Draw_Object -- General purpose draw object routine.                     *
 *                                                                                             *
 *    This routine is used to draw the object. It will handle any remapping or cloaking        *
 *    effects required. This logic is isolated here since all techno object share the same     *
 *    render logic when it comes to remapping and cloaking.                                    *
 *                                                                                             *
 * INPUT:   shapefile   -- Pointer to the shape file that the shape will be drawn from.        *
 *                                                                                             *
 *          shapenum    -- The shape number of the object in the file to use.                  *
 *                                                                                             *
 *          x,y         -- Center pixel coordinate to use for rendering this object.           *
 *                                                                                             *
 *          window      -- The clipping window to use when rendering.                          *
 *                                                                                             *
 *          rotation    -- The rotation of the object.                                         *
 *                                                                                             *
 *          scale       -- The scaling factor to use (24.8 fixed point).                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *   01/11/1996 JLB : Added rotation and scaling.                                              *
 *=============================================================================================*/
void TechnoClass::Techno_Draw_Object(ShapeSet const * shapefile, int shapenum, Point2D const & xdrawpoint, Rect const & rect, Dir256 rotation, int scale, int xzadjust, ZGradientType zgrad, bool zwrite, int brightness, ShapeSet const * zshapefile, int zshapenum, Point2D zoff, ShapeFlags_Type negflags) const
{
	if (shapefile != NULL) {
		bool shadow = DrawShapeShadows;
		VisualType visual = Visual_Character();

		ShapeFlags_Type flags = SHAPE_NORMAL;
		if (negflags == SHAPE_NORMAL) {
			bool visible = false;
			bool shadowy = true;
			switch (visual) {
				case VISUAL_DARKEN:
				case VISUAL_SHADOWY:
					visible = true;
					shadowy = false;
					break;

				case VISUAL_HIDDEN:
					return;

				case VISUAL_RIPPLE:
					visible = true;
					if (CloakingDevice.Fetch_Stage()) {
						shadowy = false;
					}
					break;

				case VISUAL_INDISTINCT:
					visible = true;
					break;
			}
			if (visible) {
				if (shadowy) {
					flags = ShapeFlags_Type(flags|SHAPE_TRANSLUCENT25);
				} else {
					flags = ShapeFlags_Type(flags|SHAPE_TRANSLUCENT50);
				}
			}
		}

		ConvertClass * converter;
		if (RTTI == RTTI_BUILDING && ((BuildingClass *)this)->Class->IsTerrainPalette) {
			CellClass * cptr = &Map[Center_Coord().As_Cell()];
			converter = cptr->Drawer;
			if (converter == NULL) {
				cptr->Init_Drawer();
				converter = cptr->Drawer;
				if (converter == NULL) return;
			}
			brightness = cptr->TileBrightness;
		} else {
			if (RTTI == RTTI_UNIT && ((UnitClass const *)this)->IsCompositingToEightBitSurface) {
				converter = EightBitDrawer;
			} else {
				converter = ColorSchemes[House->Scheme]->Converter;
			}
		}

		Point2D drawpoint = xdrawpoint;
		int zadjust = xzadjust;

		switch ((RTTIType)RTTI) {
			case RTTI_UNIT:
				if (((UnitClass *)this)->Class->IsSmallVisceroid || ((UnitClass *)this)->Class->IsLargeVisceroid) {
					shadow = false;
				}
				if (HeightAGL == 0) {
					zadjust += Get_Z_Adjust();
				} else {
					zadjust -= TacticalMap->Z_Lepton_To_Pixel(Height);
				}
				break;

			case RTTI_AIRCRAFT:
				drawpoint.Y -= TacticalMap->Z_Lepton_To_Pixel(Height);
				zadjust -= TacticalMap->Z_Lepton_To_Pixel(Height);
				break;

			case RTTI_INFANTRY:
				if (HeightAGL == 0) {
					zadjust += Get_Z_Adjust();
				} else {
					zadjust -= TacticalMap->Z_Lepton_To_Pixel(Height);
					shadow = false;
				}
				break;
		}

		/*
		**	If they're viewing a spy, and the spy belongs to some other house,
		**	make it look like an infantryman from our house
		*/
		if (RTTI == RTTI_INFANTRY) {
			if (!IsOwnedByPlayer) {
				if (((InfantryClass *)this)->Class->IsDisguised) {
					converter = ColorSchemes[PlayerPtr->Scheme]->Converter;
				}
			}
		}

		if (zgrad != ZGRAD_NONE) flags = ShapeFlags_Type(flags|SHAPE_ZGRAD);
		if (zwrite) flags = ShapeFlags_Type(flags|SHAPE_ZWRITE);
		flags = ShapeFlags_Type(flags|SHAPE_ALPHA);

		if (RTTI == RTTI_UNIT && ((UnitClass const *)this)->IsCompositingToEightBitSurface) {
			flags = ShapeFlags_Type(flags|SHAPE_NOTRANS);
			shadow = false;
		}

		flags = ShapeFlags_Type(flags & ~negflags);

		if (RTTI == RTTI_UNIT && ((UnitClass const *)this)->IsCompositingToEightBitSurface) {
			Rect shaperect = shapefile->Get_Rect(shapenum);
			Rect drawrect;
			drawrect.X = shaperect.X + drawpoint.X - shapefile->Get_Width() / 2;
			drawrect.Y = shaperect.Y + drawpoint.Y - shapefile->Get_Height() / 2;
			drawrect.Width = shaperect.Width;
			drawrect.Height = shaperect.Height;
			UnitCompositeDirtyRect = Union(UnitCompositeDirtyRect, drawrect);
		}

		brightness = Apparent_Brightness(brightness);

		switch (visual) {
			case VISUAL_NORMAL:
				Draw_Shape(*LogicalSurface, *converter, shapefile, shapenum, drawpoint, rect, ShapeFlags_Type(flags|SHAPE_CENTER|SHAPE_WIN_REL), NULL, zadjust - 2, zgrad, brightness, zshapefile, zshapenum, zoff);
				if (shadow) {
					zadjust = -2 - TacticalMap->Z_Lepton_To_Pixel(Height);
					if (IsInTransport) {
						drawpoint.Y -= 14;
					}
					Draw_Shape(*LogicalSurface, *converter, shapefile, shapenum + shapefile->Get_Count() / 2, drawpoint, rect, ShapeFlags_Type(flags|SHAPE_DARKEN|SHAPE_CENTER|SHAPE_WIN_REL), NULL, zadjust - 2);
				}
				break;

			case VISUAL_INDISTINCT:
				Draw_Shape(*LogicalSurface, *converter, shapefile, shapenum, drawpoint, rect, ShapeFlags_Type(flags|SHAPE_CENTER|SHAPE_WIN_REL), NULL, zadjust - 2, zgrad, brightness, zshapefile, zshapenum, zoff);
				if (DrawShapeShadows) {
					zadjust = -2 - TacticalMap->Z_Lepton_To_Pixel(Height);
					flags = ShapeFlags_Type(flags & ~SHAPE_TRANSLUCENT75);
					if (IsInTransport) {
						drawpoint.Y -= 14;
					}
					Draw_Shape(*LogicalSurface, *converter, shapefile, shapenum + shapefile->Get_Count() / 2, drawpoint, rect, ShapeFlags_Type(flags|SHAPE_DARKEN|SHAPE_CENTER|SHAPE_WIN_REL), NULL, zadjust - 2);
				}
				break;

			case VISUAL_DARKEN:
			case VISUAL_RIPPLE:
				Draw_Shape(*LogicalSurface, *converter, shapefile, shapenum, drawpoint, rect, ShapeFlags_Type(flags|SHAPE_CENTER|SHAPE_WIN_REL), NULL, zadjust - 2, zgrad, brightness, zshapefile, zshapenum, zoff);
				break;

			case VISUAL_SHADOWY:
				Draw_Shape(*LogicalSurface, *converter, shapefile, shapenum, drawpoint, rect, ShapeFlags_Type(flags|SHAPE_CENTER|SHAPE_WIN_REL), NULL, zadjust - 2, zgrad, brightness, zshapefile, zshapenum, zoff);
				break;
		}
	}
}


/// <summary>
/// Draws this object's voxel image.
/// This routine picks the shape flags that suit the object's current visual character --
/// a cloaked object ripples, a shrouded one fades -- and then draws from the voxel index
/// cache when the caller offers a key, rendering and caching the image if it is not there
/// yet. An object that is hidden outright draws nothing.
/// </summary>
/// <param name="voxeldata">The voxel and motion libraries to render from.</param>
/// <param name="key">The cache slot to draw from, or -1 to render without caching.</param>
/// <param name="cache">The voxel index cache to look the image up in and store it back to.</param>
/// <param name="negflags">Shape flags to suppress. Pass SHAPE_NORMAL to let the visual
/// character choose the flags.</param>
void TechnoClass::Draw_Voxel(VoxelDataStruct const & voxeldata, int frame, int key, VoxelIndexClass * cache, Rect const & xcliprect, Point2D const & point, Matrix3D const & matrix, int brightness, ShapeFlags_Type negflags) const
{
	ShapeFlags_Type flags = SHAPE_ZGRAD;
	if (negflags == SHAPE_NORMAL) {
		bool visible = false;
		bool shadowy = true;
		bool predator = false;
		switch (Visual_Character()) {
			case VISUAL_RIPPLE:
				{
					int stage = CloakingDevice.Fetch_Stage();
					visible = true;
					predator = true;
					if (stage == 0) {
						flags = ShapeFlags_Type(SHAPE_TRANSLUCENT25|SHAPE_PREDATOR|SHAPE_ZGRAD);
					} else if (stage == 1) {
						flags = ShapeFlags_Type(SHAPE_TRANSLUCENT50|SHAPE_PREDATOR|SHAPE_ZGRAD);
					} else {
						flags = ShapeFlags_Type(SHAPE_TRANSLUCENT50|SHAPE_PREDATOR|SHAPE_ZGRAD);
					}
				}
				break;

			case VISUAL_INDISTINCT:
				visible = true;
				break;

			case VISUAL_DARKEN:
			case VISUAL_SHADOWY:
				visible = true;
				shadowy = false;
				break;

			case VISUAL_HIDDEN:
				return;
		}
		if (visible) {
			if (predator) {
			} else {
				if (shadowy) {
					flags = ShapeFlags_Type(SHAPE_TRANSLUCENT25|SHAPE_ZGRAD);
				} else {
					flags = ShapeFlags_Type(SHAPE_TRANSLUCENT50|SHAPE_ZGRAD);
				}
			}
		}
	}

	flags = ShapeFlags_Type(~negflags & (flags|SHAPE_ALPHA));

	Rect cliprect = xcliprect;

	if (RTTI == RTTI_UNIT && ((UnitClass const *)this)->IsCompositingToEightBitSurface && key != -1) {
		flags = ShapeFlags_Type(flags|SHAPE_NOTRANS);
	}

	brightness = Apparent_Brightness(brightness);

	if (key != -1) {
		StaticBufferClass::Entry * entry = (*cache)[key];
		if (entry != NULL) {
			Techno_Blit_Voxel(*entry, point, cliprect, flags, brightness);
			return;
		}
	}

	SurfaceRegion region = Techno_Render_Voxel_Object(voxeldata, matrix, point, cliprect, frame, flags, brightness);
	UnitCompositeDirtyRect = Union(UnitCompositeDirtyRect, Rect(region.Point.X + point.X, region.Point.Y + point.Y, region.Bounds.Width, region.Bounds.Height));

	if (key == -1) return;

	StaticBufferClass::Entry * data = VoxelStaticBuffer.Add(*VoxelDrawSystem::Get_Surface(), region);
	if (data == NULL || !cache->Add_Index(key, data)) {
		Class_Of()->Clear_Voxel_Indexes();
		if (data == NULL) {
			data = VoxelStaticBuffer.Add(*VoxelDrawSystem::Get_Surface(), region);
		}
		cache->Add_Index(key, data);
	}
}


/// <summary>
/// Draws the voxel shadow for this object, honoring the cache key when present.
/// Skips drawing entirely if cloaked, mid-flash, or the voxel library failed to load.
/// Renders and caches the shadow when uncached, then redraws from cache.
/// </summary>
/// <param name="voxeldata">Voxel/motion library data for the object.</param>
/// <param name="layer_index">Voxel layer to render the shadow for.</param>
/// <param name="key">Cache key; -1 means do not use the cache.</param>
/// <param name="cache">Voxel index cache for the shadow entry.</param>
/// <param name="cliprect">Clipping rectangle for the shadow blit.</param>
/// <param name="point">Screen position to draw the shadow at.</param>
/// <param name="matrix">Transform matrix for the shadow.</param>
/// <param name="force_cache">Force rendering even when a cache key is supplied.</param>
void TechnoClass::Techno_Draw_Voxel_Shadow(VoxelDataStruct const & voxeldata, int layer_index, int key, VoxelIndexClass * cache, Rect const & cliprect, Point2D const & point, Matrix3D const & matrix, bool force_cache) const
{
	if (Cloak != UNCLOAKED || voxeldata.VoxLib->Load_Failed()) {
		return;
	}

	if (key != -1) {
		StaticBufferClass::Entry * entry = (*cache)[key];
		if (entry != NULL) {
			Techno_Blit_Voxel(*entry, point, cliprect, ShapeFlags_Type(SHAPE_DARKEN|SHAPE_ZGRAD), NORMAL_LIGHT);
			return;
		}
	}

	if (key == -1 || force_cache) {
		SurfaceRegion region = Techno_Render_Voxel_Shadow(voxeldata, matrix, point, cliprect, layer_index, ShapeFlags_Type(SHAPE_DARKEN|SHAPE_ZGRAD), key != -1);
		if (key != -1) {
			bool added = true;
			StaticBufferClass::Entry * data = VoxelStaticBuffer.Add(*VoxelDrawSystem::Get_Surface(), region);
			if (data == NULL || !cache->Add_Index(key, data)) {
				added = false;
				Class_Of()->Clear_Voxel_Indexes();
				if (data == NULL) {
					data = VoxelStaticBuffer.Add(*VoxelDrawSystem::Get_Surface(), region);
				}
			}
			if (!added) {
				cache->Add_Index(key, data);
			}

			Techno_Draw_Voxel_Shadow(voxeldata, layer_index, key, cache, cliprect, point, matrix, false);
		}
	}
}


/// <summary>
/// Renders a voxel object and blits the result to the screen.
/// This is the low level voxel routine that Draw_Voxel falls back on when there is no
/// cached image to use. The owning house's color scheme, the sinking clip and the predator
/// offset are all applied here, and the region returned is what the caller hands on to the
/// voxel cache.
/// </summary>
/// <param name="voxeldata">The voxel and motion libraries to render from.</param>
/// <returns>Returns with the surface region the object was rendered into.</returns>
SurfaceRegion TechnoClass::Techno_Render_Voxel_Object(VoxelDataStruct const & voxeldata, Matrix3D const & matrix, Point2D const & point, Rect const & xcliprect, int frame, ShapeFlags_Type flags, int brightness) const
{
	Matrix3D mtx = matrix;
	Matrix3D mtx2 = matrix;

	VoxelLibrary * voxlib = voxeldata.VoxLib;
	MotionLibrary * motlib = voxeldata.MotLib;

	VoxelDrawSystem::Precalculate_Light(voxlib, 0, 0, mtx2, VoxelLightSource);
	VoxelDrawSystem::Reset();

	for (unsigned layer = 0; layer < voxlib->Get_Layer_Count(); layer++) {
		if (motlib != NULL) {
			Matrix3D mtx3 = motlib->Get_Layer_Matrix(layer, frame);
			mtx = mtx2 * mtx3;
		}
		VoxelDrawSystem::Prep_For_Object(voxlib, layer, 0, VoxelCameraMatrix * mtx);
	}

	SurfaceRegion region = VoxelDrawSystem::Render();
	Rect cliprect = xcliprect;

	if (LogicalSurface != EightBitSurface) {
		if (SinkingYOffset > 0) {
			cliprect = Intersect(cliprect, Rect(0, 0, TacticalRect.Width, SinkingYOffset - TacticalMap->TacPixelY));
		} else if (IsSinking) {
			((TechnoClass *)this)->Calculate_Sinking_Offset(region.Bounds.Height, point.Y);
		}
	}

	flags = ShapeFlags_Type(flags & ~SHAPE_REMAP);
	ConvertClass * converter = NULL;
	if (RTTI == RTTI_UNIT && ((UnitClass const *)this)->IsCompositingToEightBitSurface) {
		converter = EightBitDrawer;
	} else {
		converter = ColorSchemes[House->Scheme]->Converter;
	}

	int predoffset = 0;
	if (flags & SHAPE_PREDATOR) {
		predoffset = Get_Predator_Offset();
	}

	Blit_Block(*LogicalSurface, *converter, *VoxelDrawSystem::Get_Surface(), region.Bounds, region.Point + point, cliprect, NULL, converter->Blitter_From_Flags(flags), Get_Z_Adjust(), Get_Z_Gradient(), brightness, predoffset);

	return(region);
}


/// <summary>
/// Draws the shadow cast by one layer of a voxel object.
/// This routine serves the voxel drawing pass, which works through an object's layers and
/// asks each of them in turn for its shadow. A caller that is filling the voxel cache takes
/// the rendered region away instead and does its own drawing from it later.
/// </summary>
/// <param name="voxeldata">The voxel and motion libraries for this object.</param>
/// <param name="point">The screen position to draw the shadow at.</param>
/// <param name="layer_index">Which of the object's layers to cast the shadow for.</param>
/// <param name="cached">Is the caller keeping the shadow rather than drawing it now?</param>
/// <returns>Returns with the surface region the shadow was rendered into.</returns>
SurfaceRegion TechnoClass::Techno_Render_Voxel_Shadow(VoxelDataStruct const & voxeldata, Matrix3D const & matrix, Point2D const & point, Rect const & cliprect, int layer_index, ShapeFlags_Type flags, bool cached) const
{
	Matrix3D mtx = voxeldata.MotLib->Get_Layer_Matrix(layer_index, 0);
	Matrix3D mtx2 = matrix * mtx;
	Matrix3D mtx3;
	mtx3.Make_Identity();

	VoxelDrawSystem::Reset();
	VoxelDrawSystem::Prep_For_Shadow(voxeldata.VoxLib, layer_index, 0, VoxelCameraMatrix * mtx3, mtx2, VoxelShadowLightVector);
	SurfaceRegion region = VoxelDrawSystem::Render();

	if (!cached) {
		Rect r = VoxelDrawSystem::Get_Surface()->Get_Rect(); /// The result is unused.
		Blit_Block(*LogicalSurface, *NormalDrawer, *VoxelDrawSystem::Get_Surface(), region.Bounds, region.Point + point, cliprect, NULL, NormalDrawer->Blitter_From_Flags(flags), Get_Z_Adjust());
	}

	return(region);
}


/// <summary>
/// Draws a voxel image that was rendered earlier and kept.
/// This is the cheap path for an object whose appearance has not changed since the last
/// time it was drawn -- the rendering was paid for once and this routine merely puts the
/// finished image back on the screen.
/// </summary>
/// <param name="entry">The cached voxel image to draw.</param>
/// <param name="point">The screen position to draw it at.</param>
/// <param name="brightness">The brightness to draw the image with.</param>
void TechnoClass::Techno_Blit_Voxel(StaticBufferClass::Entry const & entry, Point2D const & point, Rect const & cliprect, ShapeFlags_Type flags, int brightness) const
{
	BSurface bsurface(entry.Width, entry.Height, 1, entry.Data);
	flags = ShapeFlags_Type(flags & ~SHAPE_REMAP);

	ConvertClass * converter = NULL;
	if (RTTI == RTTI_UNIT && ((UnitClass const *)this)->IsCompositingToEightBitSurface) {
		converter = EightBitDrawer;
	} else {
		converter = ColorSchemes[House->Scheme]->Converter;
	}

	RLEBlitter const * blitter = converter->RLEBlitter_From_Flags(flags);

	int predoffset = 0;
	if (flags & SHAPE_PREDATOR) {
		predoffset = Get_Predator_Offset();
	}

	if (RTTI == RTTI_UNIT) {
		Rect r = Rect(Point2D(entry.X, entry.Y) + point, entry.Width, entry.Height);
		UnitCompositeDirtyRect = Union(UnitCompositeDirtyRect, r);
	}

	if (blitter != NULL) {
		RLE_Blit(*LogicalSurface, cliprect, Rect(Point2D(entry.X, entry.Y) + point, entry.Width, entry.Height), bsurface, bsurface.Get_Rect(), bsurface.Get_Rect(), *blitter, Get_Z_Adjust(), Get_Z_Gradient(), brightness, predoffset, NULL, Point2D());
	}
}


/// <summary>
/// Determines where a sinking object should be clipped off.
/// This routine works out the screen row below which the object is no longer to be drawn,
/// so that it appears to slide beneath the surface rather than simply vanish. A few facings
/// need the waterline nudged up before they look right.
/// </summary>
/// <param name="height">Pixel height of the object's drawn image.</param>
/// <param name="y">The screen row the object is being drawn at.</param>
void TechnoClass::Calculate_Sinking_Offset(short height, int y)
{
	SinkingYOffset = TacticalMap->TacPixelY + height + y - 10;
	FacingType dir = PrimaryFacing.Current().As_Dir8();
	if (dir == FACING_N || dir == FACING_W) {
		SinkingYOffset -= 8;
	} else if (dir == FACING_NW) {
		SinkingYOffset -= 12;
	}
}


/***********************************************************************************************
 * TechnoClass::Detach -- Handles removal of target from tracking system.                      *
 *                                                                                             *
 *    This routine is called when the specified object is about to be removed from the game    *
 *    system. The target object is removed from any tracking computers that this object may    *
 *    have.                                                                                    *
 *                                                                                             *
 * INPUT:   target   -- The target object (as a target value) that is being removed from the   *
 *                      game.                                                                  *
 *                                                                                             *
 *          all      -- Is the target about to die? A false value might indicate that the      *
 *                      object is merely cloaking. In such a case, radio contact will not      *
 *                      be affected.                                                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);

	if (all) {
		Cargo.Detach((FootClass *)target);
	}

	bool clear_target = true;

	if (!all && target->Is_Techno()) {
		CellClass * cptr = &Map[target->Center_Coord()];
		if (cptr->Is_Sensed(House->HeapID)) {
			clear_target = false;
		}
	}

	/*
	**	If the targeting computer is assigned to the target, then the targeting
	**	computer must be cleared.
	*/
	if (TarCom == target && clear_target && (all || target->Owner_HouseClass() != House)) {
		Assign_Target(NULL);
		if (Has_Suspended_Mission()) {
			Restore_Mission();
			if (RTTI == RTTI_AIRCRAFT && CurrentMission == MISSION_PATROL) {
				Status = 0;
				((AircraftClass *)this)->IsLockedStraight = false;
			}
		}
	}

	if (SuspendedTarCom == target && clear_target) {
		SuspendedTarCom = NULL;
	}

	if (House == target) {
		House = NULL;
	}

	for (int i = 0; i < ATTACHED_PARTICLE_COUNT; i++) {
		if (ParticleSystems[i] == target) {
			ParticleSystems[i] = NULL;
		}
	}

	if (Wave == target) {
		Wave = NULL;
	}

	if (all) {
		if (NearbyObject == target) {
			NearbyObject = NULL;
		}
		if (ArchiveTarget == target) {
			ArchiveTarget = NULL;
		}
	}
}


/***********************************************************************************************
 * TechnoClass::Kill_Cargo -- Destroys any cargo attached to this object.                      *
 *                                                                                             *
 *    This routine handles the destruction of any cargo this object may contain. Typical of    *
 *    this would be when a transport helicopter gets destroyed.                                *
 *                                                                                             *
 * INPUT:   source   -- The source of the destruction of the cargo.                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Kill_Cargo(TechnoClass * source)
{
	while (Cargo.Is_Something_Attached()) {
		Cargo.Attached_Object()->Remove_From_Team();
		FootClass * foot = Cargo.Detach_Object();
		if (foot != NULL) {
			foot->Record_The_Kill(source);
			delete foot;
		}
	}
}


/***********************************************************************************************
 * TechnoClass::Crew_Type -- Fetches the kind of crew this object contains.                    *
 *                                                                                             *
 *    This routine is called when generating survivors to this object. This routine returns    *
 *    the type of survivor to generate.                                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the infantry type of a survivor.                                           *
 *                                                                                             *
 * WARNINGS:   This routine is designed to be called repeatedly. Once for each survivor to     *
 *             generate.                                                                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
InfantryTypeClass const * TechnoClass::Crew_Type(void) const
{
	/*
	**	If this object contains no crew, then there can be no
	**	crew inside, duh... return this news.
	*/
	if (!TClass->IsCrew) {
		return(NULL);
	}

	/*
	**	The normal infantry survivor is the standard issue
	**	minigunner. Certain buildings, especially neutral ones, tend to have
	**	civilians exit them instead.
	*/
	InfantryTypeClass const * infantry = Rule->Crew;
	if (House->Class->Side != SIDE_NONE) {
		if (Is_Weapon_Equipped() && Percent_Chance(15)) {
			infantry = Rule->Technician;
		}
	} else {
		infantry = Rule->Technician;
	}
	return(infantry);
}


/***********************************************************************************************
 * TechnoClass::Value -- Fetches the target value for this object.                             *
 *                                                                                             *
 *    This routine is used to fetch the target value for this object. The greater the value    *
 *    returned, the better this object is as a target.                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the target value for this object.                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *   08/16/1995 JLB : Adjusted for early mission lame-out.                                     *
 *=============================================================================================*/
int TechnoClass::Value(void) const
{
	int value = 0;

	/*
	**	In early missions, contents of transports are not figured
	**	into the total value.
	*/
	if (Rule->Diff[House->Difficulty].IsContentScan || House->IQ >= Rule->IQContentScan) {
		if (Cargo.Is_Something_Attached()) {
			FootClass * object = Cargo.Attached_Object();

			while (object != NULL) {
				value += object->Value();
				object = (FootClass *)(ObjectClass *)object->Next;
			}
		}
	}

	return(Risk() + TClass->Reward + value);
}


/***********************************************************************************************
 * TechnoClass::Threat_Range -- Returns the range to scan based on threat control.             *
 *                                                                                             *
 *    This routine will return the range to scan based on the control value specified. The     *
 *    value returned by this routine is typically used when scanning for enemies.              *
 *                                                                                             *
 * INPUT:   control  -- The range control parameter.                                           *
 *                      0  = Use weapon range (zero is returned in this special case).         *
 *                      -1 = Scan without range restrictions (-1 is returned in this case).    *
 *                      1  = Scan up to twice weapon range.                                    *
 *                                                                                             *
 * OUTPUT:  Returns with a range (or special value) that can be used in the threat scan        *
 *          process. If zero is returned, then always check threat against In_Range(). If      *
 *          -1 is returned, then no range limitation restriction exists.                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoClass::Threat_Range(int control) const
{
	/*
	**	Threat range means nothing if scanning the whole map. In such a case, just
	**	return with the same control flag specified.
	*/
	if (control == -1) return(-1);

	/*
	**	If simple guard range is requested, then return "0" since
	**	this is a special control value that is calculated as the object's
	**	weapon range.
	*/
	if (control == 0) {
		/*
		**	For normal guard mode or for area guard mode, use the override
		**	threat range value as specified by the object's type class.
		*/
		bool is_renovator = Is_Renovator();
		if (TClass->ThreatRange != 0 && !is_renovator) {
			return(TClass->ThreatRange);
		}
		return(0);
	}

	/*
	**	Area guard range is specified, so figure twice the weapon range of the
	**	longest range weapon this object is equipped with.
	*/
	int range = TClass->ThreatRange;
	if (range == 0) {
		range = std::max(Weapon_Range(0), Weapon_Range(1));
	}

	range *= 2;
	if (control == 2) {
		range = std::clamp(range, 7 * CELL_LEPTON, 16 * CELL_LEPTON);
	} else {
		range = std::clamp(range, 0, 16 * CELL_LEPTON);
	}

	return(range);
}


/***********************************************************************************************
 * TechnoClass::Is_In_Same_Zone -- Determine if specified cell is in same zone as object.      *
 *                                                                                             *
 *    This will examine the specified cell to determine if it is in the same zone as this      *
 *    object's location.                                                                       *
 *                                                                                             *
 * INPUT:   cell  -- The cell that is to be checked against this object's current location.    *
 *                                                                                             *
 * OUTPUT:  bool; Is the specified cell in the same zone as this object is?                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoClass::Is_In_Same_Zone(Coord const & coord) const
{
	MZoneType zone = TClass->MZone;

	if (zone != MZONE_NONE) {
		if (coord != COORD_NONE) {
			Cell cell1 = coord.As_Cell();
			Cell cell2 = Destination_Coord().As_Cell();

			return(Map.Is_Same_Cell_Zone(cell2, cell1, zone, ((ObjectClass *)this)->Is_Moving_Onto_Bridge(), Map[cell1].IsUnderBridge, Is_Allowed_To_Leave_Map()));
		}
		/// FootClass::Is_In_Same_Zone reports false for a coordinate it cannot resolve;
		/// this version falls through and reports the same zone instead.
	}
	return(true);
}


/***********************************************************************************************
 * TechnoClass::Base_Is_Attacked -- Handle panic response to base being attacked.              *
 *                                                                                             *
 *    This routine is called when the base is being attacked. It will pull units off of the    *
 *    field and send them back to defend the base. This routine will make taking an enemy      *
 *    base much more difficult.                                                                *
 *                                                                                             *
 * INPUT:   enemy -- Pointer to the enemy object that did the damage on the base.              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine can drastically affect the game play. The computer will probably   *
 *             call off its attacks as a result.                                               *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Commented.                                                               *
 *   10/15/1996 JLB : Alternates between guard area and attack.                                *
 *   11/01/1996 JLB : Allow recruit of guard area units in multiplay.                          *
 *=============================================================================================*/
void TechnoClass::Base_Is_Attacked(TechnoClass const * enemy)
{
	FootClass * defender[6];
	memset(defender, '\0', sizeof(defender));

	int value[ARRAY_SIZE(defender)];
	memset(value, '\0', sizeof(value));

	int count = 0;
	int weakest = 0;
	int desired = enemy->Risk() * Rule->ComputerBaseDefenseResponse;
	int risktotal = 0;

	/*
	**	Humans have to deal with their own base is attacked problems.
	*/
	if (enemy == NULL || House->Is_Ally(enemy) || House->Is_Human_Player()) {
		return;
	}

	/*
	**	Don't overreact if this building can defend itself.
	*/
	if (Session.Type == GAME_NORMAL && Is_Weapon_Equipped()) return;

	/*
	**	If the enemy is not an infantry or a unit there is not much we can
	**	do about it.
	*/
	if (enemy->RTTI != RTTI_INFANTRY && enemy->RTTI != RTTI_UNIT) {
		return;
	}

	/*
	**	If we are a certain type of building, such as a barrel or land mine,
	**	ignore the attack.
	*/
	if (TClass->IsInsignificant) {
		return;
	}

	/*
	**	If the threat has already been dealt with then we don't need to do
	**	any work. Check for that here.
	*/
	if (enemy->Is_Foot() && ((FootClass *)enemy)->BaseAttackTimer != 0) {
		return;
	}

	/*
	**	We will need units to defend our base.  We need to suspend teams until
	**	the situation has been dealt with.
	*/
	TeamClass::Suspend_Teams(Rule->SuspendPriority, House);

	/*
	**	Loop through the infantry looking for those who are capable of going
	**	on a rescue mission.
	*/
	int index;
	for (index = 0; index < Infantry.Count() && desired > 0; index++) {
	 	InfantryClass * infantry = Infantry[index];
		if (infantry != NULL && infantry->IsActive && infantry->Owner() == Owner()) {

			/*
			**	Never recruit sticky guard units to defend a base.
			*/
			bool basedefense = infantry->Team != NULL && infantry->Team->Class->IsBaseDefense;
			if (infantry->Team != NULL && !basedefense) {
				continue;
			}

			if (!infantry->IsTeamRecruitable || !infantry->Is_Weapon_Equipped() || !infantry->IsAutocreateRecruitable ||
					(!infantry->Current_Mission_Control().IsRecruitable && Session.Type == GAME_NORMAL)) continue;

			/*
			**	Don't allow a response if it doesn't have a weapon that will affect the
			**	enemy object.
			*/
			if (infantry->Get_Class_Weapon_Data(0)->Weapon->WarheadPtr->Modifier[enemy->TClass->Armor] == 0) {
				continue;
			}

			/*
			**	Don't try to help if the building is on another planet.
			*/
			if (!Map.Is_Same_Cell_Zone(infantry->Destination_Coord().As_Cell(), Destination_Coord().As_Cell(), infantry->Class->MZone, infantry->Is_Moving_Onto_Bridge(), false, false)) continue;

			/*
			**	Find the amount of threat that this unit can apply to the
			**	enemy.
			*/
			int threat = infantry->Rescue_Mission((AbstractClass *)enemy);

			/*
			**	If it can't apply any threat then do just skip it and do not
			**	add it to the list.
			*/
			if (!threat) {
				continue;
			}

			/*
			**	Greatly increase the threat value if this unit is already assigned to protect
			**	the target.
			*/
			if (ArchiveTarget == this) {
				threat *= 100;
			}

			/*
			**	If the value returned is negative then this unit is already
			**	assigned to fighting the enemy, so subtract its value from
			**	the enemy's desired value.
			*/
			if (threat < 0) {
				desired += threat;
				continue;
			}

			if (count < ARRAY_SIZE(defender)) {
				defender[count] = infantry;
				value[count] = threat;
				count++;
				continue;
			}

			if (threat > weakest) {
				int newweakest = threat;

				for (int lp = 0; lp < count; lp++) {
					if (value[lp] == weakest) {
						value[lp] = threat;
						defender[lp] = (FootClass *) infantry;
						continue;
					}
					if (value[lp] < newweakest) {
						newweakest = value[lp];
					}
				}
				weakest = newweakest;
			}
		}
	}

	/*
	**	Loop through the units looking for those who are capable of going
	**	on a rescue mission.
	*/
	for (index = 0; index < Units.Count() && desired > 0; index++) {
	 	UnitClass * unit = Units[index];
		if (unit != NULL && unit->IsActive && unit->Owner() == Owner()) {

			/*
			**	Never recruit sticky guard units to defend a base.
			*/
			bool basedefense = unit->Team != NULL && unit->Team->Class->IsBaseDefense;
			if (unit->Team != NULL && !basedefense) {
				continue;
			}

			if (!unit->IsTeamRecruitable || !unit->Is_Weapon_Equipped() || !unit->IsAutocreateRecruitable ||
					(!unit->Current_Mission_Control().IsRecruitable && Session.Type == GAME_NORMAL)) continue;

			/*
			**	Don't allow a response if it doesn't have a weapon that will affect the
			**	enemy object.
			*/
			if (unit->Get_Class_Weapon_Data(0)->Weapon->WarheadPtr->Modifier[enemy->TClass->Armor] == 0) {
				continue;
			}

			/*
			**	Don't try to help if the building is on another planet.
			*/
			if (!Map.Is_Same_Cell_Zone(unit->Destination_Coord().As_Cell(), Destination_Coord().As_Cell(), unit->Class->MZone, unit->Is_Moving_Onto_Bridge(), false, false)) continue;

			/*
			**	Find the amount of threat that this unit can apply to the
			**	enemy.
			*/
			int threat = unit->Rescue_Mission((AbstractClass *)enemy);

			/*
			**	If it can't apply any threat then do just skip it and do not
			**	add it to the list.
			*/
			if (!threat) {
				continue;
			}

			/*
			**	Greatly increase the threat value if this unit is already assigned to protect
			**	the target.
			*/
			if (threat > 0 && ArchiveTarget == this) {
				threat *= 10;
			}

			/*
			**	If the value returned is negative then this unit is already
			**	assigned to fighting the enemy, so subtract its value from
			**	the enemy's desired value.
			*/
			if (threat < 0) {
				desired += threat;
				continue;
			}

			if (count < ARRAY_SIZE(defender)) {
				defender[count] = unit;
				value[count] = threat;
				count++;
				continue;
			}
			if (threat > weakest) {
				int newweakest = threat;

				for (int lp = 0; lp < count; lp ++) {
					if (value[lp] == weakest) {
						value[lp] = threat;
						defender[lp] = (FootClass *) unit;
						continue;
					}
					if (value[lp] < newweakest) {
//					if (value[count] < newweakest) {
						newweakest = value[lp];
					}
				}
				weakest = newweakest;
			}
		}
	}

	if (desired > 0) {

		/*
		**	Sort the defenders by value, this doesn't take very long and will
		**	help the closest defenders to respond.
		*/
		int lp;
		for (lp = 0; lp < count - 1; lp ++) {
			for (int lp2 = lp + 1; lp2 < count; lp2++) {
				if (value[lp] < value[lp2]) {

					value[lp] 	^= value[lp2];
					value[lp2]	^= value[lp];
					value[lp]	^= value[lp2];

					FootClass *temp;
					temp				= defender[lp];
					defender[lp]	= defender[lp2];
					defender[lp2]  = temp;
				}
			}
		}

		for (lp = 0; lp < count; lp ++) {
			bool basedefense = defender[lp]->Team != NULL && defender[lp]->Team->Class->IsBaseDefense;
			if (Percent_Chance(66) && !basedefense) {
				defender[lp]->Assign_Mission(MISSION_RESCUE);
				defender[lp]->ArchiveTarget = this;
			} else {
				defender[lp]->Assign_Mission(MISSION_GUARD_AREA);
				defender[lp]->ArchiveTarget = this;
			}
			defender[lp]->Assign_Target((AbstractClass *)enemy);
			risktotal += defender[lp]->Risk();
			if (risktotal > desired) {
				break;
			}
		}
	}

	if (risktotal > desired && enemy->Is_Foot()) {
		((FootClass *)enemy)->BaseAttackTimer = TICKS_PER_MINUTE * Rule->BaseDefenseDelay;
	}
}


/***********************************************************************************************
 * TechnoClass::Is_Allowed_To_Retaliate -- Checks object to see if it can retaliate.           *
 *                                                                                             *
 *    This routine is called when this object has suffered some damage and it needs to know    *
 *    if it should fight back. The object that caused the damage is specifed as a parameter.   *
 *                                                                                             *
 * INPUT:   source   -- The points to the object that was the source of the damage applied     *
 *                      to this object.                                                        *
 *                                                                                             *
 * OUTPUT:  bool; Should retaliation occur?                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/19/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoClass::Is_Allowed_To_Retaliate(TechnoClass const * source, WarheadTypeClass const * warhead) const
{
	if (House->Is_Human_Player() && TarCom != NULL) return(false);

	bool has_navqueue = Is_Foot() && ((FootClass const *)this)->NavCom != NULL;
	if (warhead != NULL && warhead->IsVeinhole && (!has_navqueue || !House->Is_Human_Player())) {
		return(true);
	}

	/*
	**	If there is no source of the damage, then retaliation cannot occur.
	*/
	if (source == NULL) return(false);

	/*
	**	If the mission precludes retaliation, then don't retaliate.
	*/
	if (!Current_Mission_Control().IsRetaliate) return(false);

	/*
	**	If the source of the damage is an ally, then retaliation shouldn't
	**	occur either.
	*/
	if (House->Is_Ally(source)) return(false);

	/*
	**	Only objects that have a damaging weapon are allowed to retaliate.
	*/
	if (Combat_Damage() <= 0 || !Is_Weapon_Equipped()) return(false);

	/*
	**	If this is not equipped with a weapon that can attack the molester, then
	**	don't allow retaliation.
	*/
	TechnoTypeClass const * ttype = TClass;
	int which = What_Weapon_Should_I_Use((AbstractClass *)source);
	WeaponDataStruct const * wdata = Get_Class_Weapon_Data(which);
	if (wdata->Weapon->WarheadPtr != NULL &&
		wdata->Weapon->WarheadPtr->Modifier[source->TClass->Armor] == 0) {
			return(false);
	}

	/*
	**	Don't allow retaliation if it isn't equipped with a weapon that can deal with the threat.
	*/
	if (source->RTTI == RTTI_AIRCRAFT && !wdata->Weapon->Bullet->IsAntiAircraft) return(false);

	/*
	**	Tanya is not allowed to retaliate against buildings in the normal sense while in guard mode. That
	**	is, unless it is owned by the computer. Normally, Tanya can't do anything substantial to a building
	**	except to blow it up.
	*/
	if (House->Is_Human_Player() && source->RTTI == RTTI_BUILDING &&
			(RTTI == RTTI_INFANTRY && ((InfantryTypeClass const *)ttype)->IsBomber || Has_Ability(ABILITY_C4))) {
		return(false);
	}

	if (House->Is_Human_Player() && RTTI == RTTI_UNIT) {
		BuildingTypeClass const * deploys_into = ((UnitClass const *)this)->Class->DeploysInto;
		if (deploys_into != NULL && deploys_into->IsArtillary) {
			return(false);
		}
	}

	/*
	**	If a human house is not allowed to retaliate automatically, then don't
	*/
	if (House->Is_Human_Player() && !Rule->IsSmartDefense && RTTI != RTTI_BUILDING) {
		if (CurrentMission != MISSION_GUARD_AREA && CurrentMission != MISSION_GUARD && CurrentMission != MISSION_PATROL) {
			return(false);
		}
	}

	/*
	**	If this object is part of a team that prevents retaliation then don't allow retaliation.
	*/
	if (Is_Foot() && ((FootClass *)this)->Team != NULL && ((FootClass *)this)->Team->Class->IsSuicide) {
		return(false);
	}

	/*
	**	Compare potential threat of the current target and the potential new target. Don't retaliate
	**	if it is currently attacking the greater threat.
	*/
	if (!House->Is_Human_Player() && TarCom != NULL && TarCom->As_ObjectClass() != NULL
			&& Target_Threat((TechnoClass *)TarCom) > Target_Threat((TechnoClass *)source)) {
		return(false);
	}

	/*
	**	All checks passed, so return that retaliation is allowed.
	*/
	return(true);
}


/***********************************************************************************************
 * TechnoClass::Get_Ownable -- Fetches the ownable bits for this object.                       *
 *                                                                                             *
 *    This routine will return the ownable bits for this object. The ownable bits represent    *
 *    the houses that are allowed to own this object.                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the ownable bits for this object.                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoClass::Get_Ownable(void) const
{
	return(TClass->Get_Ownable());
}


/***********************************************************************************************
 * TechnoClass::Risk -- Fetches the risk associated with this object.                          *
 *                                                                                             *
 *    This routine is called when the risk value for this object needs to be determined.       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the risk value for this object.                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoClass::Risk(void) const
{
	TechnoTypeClass const *ttptr = TClass;
	if (ttptr != NULL) {
		return(ttptr->Risk);
	}

	return(0);
}


/***********************************************************************************************
 * TechnoClass::Tiberium_Load -- Fetches the current tiberium load percentage.                 *
 *                                                                                             *
 *    This routine will return the current Tiberium load (expressed as a fixed point fraction) *
 *    that this object currently contains. Typical implementor of this function would be       *
 *    the harvester. Any object that can return a non-zero value should derive from this       *
 *    function in order to return the appropriate value.                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current Tiberium load expressed as a fixed point number.          *
 *          0x0000   = empty                                                                   *
 *          0x0080   = half full                                                               *
 *          0x0100   = full                                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
double TechnoClass::Tiberium_Load(void) const
{
	return(0.0);
}


/***********************************************************************************************
 * TechnoClass::Desired_Load_Dir -- Fetches loading parameters for this object.                *
 *                                                                                             *
 *    This routine is called when an object desires to load up on this object. The object      *
 *    desiring to load is specified. The cell that the loading object should move to is        *
 *    determined. The direction that this object should face is also calculated. This routine  *
 *    will be overridden by those objects that can actually load up passengers.                *
 *                                                                                             *
 * INPUT:   object   -- The object that is desiring to load up.                                *
 *                                                                                             *
 *          moveto   -- Reference to the cell that the loading object should move to before    *
 *                      the final load process occurs (this value will be filled in).          *
 *                                                                                             *
 * OUTPUT:  Returns with the direction that the transport object should face.                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
FacingType TechnoClass::Desired_Load_Dir(ObjectClass * , Cell & moveto) const
{
	moveto = CELL_NONE;
	return(FACING_N);
}


/***********************************************************************************************
 * TechnoClass::Pip_Count -- Fetches the number of pips to display on this object.             *
 *                                                                                             *
 *    This routine will return the number of pips to display on this object when the object    *
 *    is selected. The default condition is to return no pips at all. This routine is          *
 *    derived for those objects that can have pips.                                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of pips to display on this object when selected.           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoClass::Pip_Count(void) const
{
	bool valid = false;
	double maximum, current;
	switch (TClass->PipScale) {

		case PIPSCALE_PASSENGERS:
			current = Cargo.How_Many();
			maximum = TClass->Max_Passengers();
			valid = true;
			break;

		case PIPSCALE_AMMO:
			current = Ammo;
			maximum = TClass->MaxAmmo;
			valid = true;
			break;

		default:
			return(0);
	}

	if (valid) {
		int retval = TClass->Max_Pips() * (current / maximum) + 0.5;
		if (!retval && current > 0) retval = 1;
		return(retval);
	}

	return(0);
}


/***********************************************************************************************
 * TechnoClass::Fire_Direction -- Fetches the direction projectile fire will take.             *
 *                                                                                             *
 *    This routine will fetch the direction that a fired projectile will take. This is         *
 *    usually the facing of the object's weapon. This routine will be derived for the objects  *
 *    that have their weapon barrel facing a different direction than the body.                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the direction a fired projectile will take.                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
DirType TechnoClass::Fire_Direction(void) const
{
	return(Turret_Facing());
}


/***********************************************************************************************
 * TechnoClass::Response_Select -- Handles the voice response when selected.                   *
 *                                                                                             *
 *    This routine is called when a voice response to a select action is desired. This routine *
 *    should be overridden for any object that actually has a voice response.                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine can generate an audio response.                                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Response_Select(void)
{
	if (AllowVoice && TClass->VoiceSelect.Count()) {
		Sound_Effect((VocType)TClass->VoiceSelect.Pick(NonCriticalRandomNumber));
	}
}


/***********************************************************************************************
 * TechnoClass::Response_Move -- Handles the voice response to a movement request.             *
 *                                                                                             *
 *    This routine is called when a voice response to a movement order is desired. This        *
 *    routine should be overridden for any object that actually has a voice response.          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This can generate an audio response.                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Response_Move(void)
{
	if (AllowVoice && TClass->VoiceMove.Count()) {
		Sound_Effect((VocType)TClass->VoiceMove.Pick(NonCriticalRandomNumber));
	}
}


/***********************************************************************************************
 * TechnoClass::Response_Attack -- Handles the voice response when given attack order.         *
 *                                                                                             *
 *    This routine is called when a voice response to an attack order is desired. This routine *
 *    should be overridden for any object that actually have a voice response.                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This can generate an audio response.                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Response_Attack(void)
{
	if (AllowVoice && TClass->VoiceAttack.Count()) {
		Sound_Effect((VocType)TClass->VoiceAttack.Pick(NonCriticalRandomNumber));
	}
}


/***********************************************************************************************
 * TechnoClass::Target_Something_Nearby -- Handles finding and assigning a nearby target.      *
 *                                                                                             *
 *    This routine will search for a nearby target and assign it to this object's TarCom.      *
 *    The method to use when scanning for a target is controlled by the parameter passed.      *
 *                                                                                             *
 * INPUT:   threat   -- The threat control parameter used to control the range searched. The   *
 *                      only values recognized are THREAT_RANGE and THREAT_AREA.               *
 *                                                                                             *
 * OUTPUT:  Was a suitable target acquired and assigned to the TarCom?                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoClass::Target_Something_Nearby(Coord const & coord, ThreatType threat)
{
	threat = ThreatType(threat & (THREAT_RANGE|THREAT_AREA));

	/*
	**	Determine that if there is an existing target it is still legal
	**	and within range.
	*/
	if (TarCom != NULL) {
		if ((threat & THREAT_RANGE)) {
			int primary = What_Weapon_Should_I_Use(TarCom);
			if (!In_Range(TarCom, primary)) {
				Assign_Target(NULL);
			}
		}
	}

	/*
	**	If there is no target, then try to find one and assign it as
	**	the target for this unit.
	*/
	if (TarCom == NULL) {
		Assign_Target(Greatest_Threat(threat, coord, false));
	}

	/*
	**	Return with answer to question: Does this unit now have a target?
	*/
	return(TarCom != NULL);
}


/***********************************************************************************************
 * TechnoClass::Exit_Object -- Causes specified object to leave this object.                   *
 *                                                                                             *
 *    This routine is called when there is an attached object that should detach and leave     *
 *    this object. Typical of this would be the refinery and APC.                              *
 *                                                                                             *
 * INPUT:   object   -- The object that is trying to leave this object.                        *
 *                                                                                             *
 * OUTPUT:  Was the object successfully launched from this object? Failure might indicate that *
 *          there is insufficient room to detach the specified object.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoClass::Exit_Object(TechnoClass *)
{
	return(0);
}


/***********************************************************************************************
 * TechnoClass::Is_Ready_To_Random_Animate -- Determines if the object should random animate.  *
 *                                                                                             *
 *    This will examine this object to determine if it is time and ready to perform some       *
 *    kind of random animation.                                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is it time to perform an random animation?                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/19/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoClass::Is_Ready_To_Random_Animate(void) const
{
	return(IdleTimer == 0);
}


/***********************************************************************************************
 * TechnoClass::Assign_Destination -- Assigns movement destination to the object.              *
 *                                                                                             *
 *    This routine is called when the object needs to have a new movement destination          *
 *    assigned. This routine must be overridden since at this level, movement is not allowed.  *
 *                                                                                             *
 * INPUT:   destination -- The destination to assign to this object.                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Assign_Destination(AbstractClass *, bool)
{
}


/***********************************************************************************************
 * TechnoClass::Enter_Idle_Mode -- Object enters its default idle condition.                   *
 *                                                                                             *
 *    This routine is called when the object should intelligently revert to an idle state.     *
 *    Typically this routine is called after some mission has completed. This routine must     *
 *    be overridden by the various object types. It is located at this level merely to provide *
 *    a virtual function entry point.                                                          *
 *                                                                                             *
 * INPUT:   initial  -- Is this called when the unit just leaves a factory or is initially     *
 *                      or is initially placed on the map?                                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoClass::Enter_Idle_Mode(bool, bool)
{
	return(false);
}


/// <summary>
/// Draws the marks that stand for what the object is rather than what state it is in: the
/// cross worn by a healer and the insignia of a veteran or an elite. These are drawn whether
/// or not the object is selected, so the caller is responsible for testing that the player is
/// allied to its owner and may see it.
/// </summary>
/// <param name="bottomleft">The point the cargo pips would run from.</param>
/// <param name="center">The point the insignia is placed beside.</param>
void TechnoClass::Draw_Insignia(Point2D const & bottomleft, Point2D const & center, Rect const & rect) const
{
	ShapeSet const * pips1 = (ShapeSet const *)Class_Of()->PipShapes;

	// A weapon that averages negative damage is what marks the object out as a healer.
	if (RTTI == RTTI_INFANTRY && Combat_Damage() < 0) {
		Point2D xy = bottomleft + Point2D(-5, 0);
		Draw_Shape(*LogicalSurface, *NormalDrawer, pips1, PIP_MEDIC, xy+Point2D(0,-8), rect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL));
	}

	PipEnum veterancy_shape = PIP_NONE;
	if (Veterancy.Is_Veteran()) {
		veterancy_shape = PIP_VETERAN;
	}
	if (Veterancy.Is_Elite()) {
		veterancy_shape = PIP_ELITE;
	}
	if (Veterancy.Is_Dumbass()) {
		veterancy_shape = PIP_COUNT;
	}
	if (veterancy_shape != PIP_NONE) {
		Point2D drawpoint = center + Point2D(5, 2);
		if (RTTI != RTTI_INFANTRY) {
			drawpoint += Point2D(5, 4);
		}
		Draw_Shape(*LogicalSurface, *NormalDrawer, pips1, veterancy_shape, drawpoint, rect, ShapeFlags_Type(SHAPE_WIN_REL|SHAPE_CENTER));
	}
}


/***********************************************************************************************
 * TechnoClass::Draw_Pips -- Draws the transport pips and other techno graphics.               *
 *                                                                                             *
 *    This routine is used to render the small transportation pip (occupant feedback graphic)  *
 *    used for transporter object. It will also display if the techno object is "primary"      *
 *    if necessary.                                                                            *
 *                                                                                             *
 * INPUT:   x,y   -- The pixel coordinate for the center of the first pip. Subsequent pips     *
 *                   are drawn rightward.                                                      *
 *                                                                                             *
 *          window-- The window that pip clipping is relative to.                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/08/1995 JLB : Created.                                                                 *
 *   10/06/1995 JLB : Displays the team group number.                                          *
 *   09/10/1996 JLB : Medic hack for red pip.                                                  *
 *=============================================================================================*/
void TechnoClass::Draw_Pips(Point2D const & bottomleft, Point2D const & center, Rect const & rect) const
{
	Point2D offset(4, 2);
	Point2D xy = bottomleft + Point2D(6, -1);

	ShapeSet const * pip_shapes = (ShapeSet const *)Class_Of()->PipShapes;
	ShapeSet const * pips2 = (ShapeSet const *)Class_Of()->Pip2Shapes;

	if (RTTI != RTTI_BUILDING) {
		xy = bottomleft + Point2D(-5, 0);
		offset = Point2D(4, 0);
		pip_shapes = pips2;
	}

	/*
	**	Transporter type objects have a different graphic representation for the pips. The
	**	pip color represents the type of occupant.
	*/
	if (TClass->Max_Passengers() > 0) {
		ObjectClass const * object = Cargo.Attached_Object();
		for (int index = 0; index < Class_Of()->Max_Pips(); index++) {
			PipEnum pip = PIP_EMPTY;

			if (object != NULL) {
				pip = PIP_GREEN;
				if (object->RTTI == RTTI_INFANTRY) {
					pip = ((InfantryClass *)object)->Class->Pip;
				}
				object = object->Next;
			}
			Draw_Shape(*LogicalSurface, *NormalDrawer, pip_shapes, pip, xy + offset * index, rect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL));
		}

	} else {

		/*
		**	Display number of how many attached objects there are. This is also used
		**	to display the fullness rating for a harvester.
		*/
		int pips = Pip_Count();

		/*
		**	Check if it's a harvester, to show the right type of pips for the
		**	various minerals it could have harvested.
		*/
		if (RTTI == RTTI_UNIT && TClass->PipScale == PIPSCALE_TIBERIUM) {

			int iron = Storage.Get_Amount(0);
			int nickel = Storage.Get_Total_Amount() - iron;
			int greenpips   = TClass->Max_Pips() * (double(iron) / TClass->Capacity) + 0.5;
			int bluepips  = TClass->Max_Pips() * (double(nickel) / TClass->Capacity) + 0.5;

			for (int index = 0; index < Class_Of()->Max_Pips(); index++) {
				int shape = PIP_EMPTY;
				if (bluepips > 0) {
					shape = PIP_BLUE;
					bluepips--;
				} else if (greenpips > 0) {
					shape = PIP_GREEN;
					greenpips--;
				}
				Draw_Shape(*LogicalSurface, *NormalDrawer, pip_shapes, shape, xy + offset * index, rect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL));
			}
		}

		/*
		**
		*/
		else if (TClass->PipScale == PIPSCALE_AMMO) {
			int _pips = pips;
			for (int index = 0; index < Class_Of()->Max_Pips(); index++) {
				if (_pips > 0) {
					_pips--;
					Draw_Shape(*LogicalSurface, *NormalDrawer, pips2, 6, xy + offset * index - Point2D(0, 3), rect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL));
				}
			}

		}
		/*
		**
		*/
		else if (RTTI == RTTI_BUILDING && TClass->PipScale == PIPSCALE_TIBERIUM) {
			int _pips = pips;
			for (int index = 0; index < Class_Of()->Max_Pips(); index++) {
				int shape = PIP_EMPTY;
				if (_pips > 0) {
					shape = PIP_GREEN;
					_pips--;
				}
				Draw_Shape(*LogicalSurface, *NormalDrawer, pip_shapes, shape, xy + offset * index, rect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL));
			}
		}

		/*
		**
		*/
		else if (TClass->PipScale == PIPSCALE_CHARGE) {
			for (int index = 0; index < Class_Of()->Max_Pips(); index++) {
				Draw_Shape(*LogicalSurface, *NormalDrawer, pip_shapes, index < pips ? 1 : 0, xy + offset * index, rect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL));
			}
		}
	}

	Draw_Insignia(bottomleft, center, rect);

	/*
	**	Display whether this unit is a leader unit or not.
	*/
	if (RTTI != RTTI_BUILDING) {
		Draw_Text_Overlay(bottomleft + Point2D(-10, 10), bottomleft, rect);
	}

	/*
	**	Display what group this unit belongs to. This corresponds to the team
	**	number assigned with the <CTRL> key.
	*/
	if (Group >= 0 && Group < 10) {
		int yval = -1;
		int group = Group+1;

		if (Class_Of()->Max_Pips()) yval -= 4;
		if (group == 10) group = 0;

		char group_text[12];
		sprintf(group_text, "%d", group < 10 ? group : 0);

		Plain_Text_Print(group_text, *LogicalSurface, rect, bottomleft + Point2D(-4, yval-3), WHITE, TBLACK, TextPrintType(TPF_FULLSHADOW|TPF_EFNT), 0, 1);
	}
}


/// <summary>
/// Draws the text that is overlaid on top of the object.
/// This routine handles the power output and drain figures shown against a building that
/// generates power, and the "Primary" tag worn by a leader object. The tag is abbreviated
/// when the building is too narrow to carry the whole word.
/// </summary>
/// <param name="point2">The point the text is centered on.</param>
void TechnoClass::Draw_Text_Overlay(Point2D const & point1, Point2D const & point2, Rect const & cliprect) const
{
	if (RTTI == RTTI_BUILDING && ((BuildingClass*)this)->Class->Power > 0) {
		char buffer[128];
		sprintf(buffer, Fetch_String(TXT_POWER_DRAIN), House->Power_Output(), House->Power_Drain());
		Plain_Text_Print(buffer, *LogicalSurface, cliprect, point2, WHITE, TBLACK, TextPrintType(TPF_CENTER|TPF_FULLSHADOW|TPF_EFNT), 0, 1);
	}

	/*
	**	Display whether this unit is a leader unit or not.
	*/
	if (IsLeader) {
		int pritext = TXT_PRIMARY;

		if (RTTI == RTTI_BUILDING) {
			if (((BuildingClass *)this)->Class->Width() == 1) {
				pritext = TXT_PRI;
			}
		}
		Plain_Text_Print(pritext, *LogicalSurface, cliprect, point2, WHITE, TBLACK, TextPrintType(TPF_CENTER|TPF_FULLSHADOW|TPF_EFNT), 0, 1);
	}
}


/***********************************************************************************************
 * TechnoClass::Find_Docking_Bay -- Searches for a close docking bay.                          *
 *                                                                                             *
 *    This routine will be used to find a building that can serve as a docking bay. The        *
 *    closest building that qualifies will be returned. If no building could be found then     *
 *    return with NULL.                                                                        *
 *                                                                                             *
 * INPUT:   b  -- The structure type to look for.                                              *
 *                                                                                             *
 *          friendly -- Allow searching for allied buildings as well.                          *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the building that can serve as the best docking bay.     *
 *                                                                                             *
 * WARNINGS:   This routine might return NULL even if there are buildings of the specified     *
 *             type available. This is the case when the building(s) are currently busy and    *
 *             cannot serve as a docking bay at the moment.                                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1995 JLB : Created.                                                                 *
 *   08/13/1995 JLB : Recognizes the "IsLeader" method of building preference.                 *
 *=============================================================================================*/
BuildingClass * TechnoClass::Find_Docking_Bay(BuildingTypeClass const * b, bool friendly, bool evenoccupied) const
{
	BuildingClass * best = 0;

	/*
	**	First check to see if there are ANY buildings of the specified
	**	type in this house's inventory. If not, then don't bother to scan
	**	for one.
	*/
	if (House->BQuantity.Value(b->HeapID) != 0) {
		int bestval = -1;

		/*
		**	Loop through all the buildings and find the one that matches the specification
		**	and is willing to dock with this object.
		*/
		for (int index = 0; index < Buildings.Count(); index++) {
			BuildingClass * building = Buildings[index];

			/*
			**	Check to see if the building qualifies (preliminary scan).
			*/
			if (building != NULL &&
				(friendly ? building->House->Is_Ally(this) : building->House == House) &&
				!building->IsInLimbo &&
				building->Class == b &&
				(!evenoccupied || !building->In_Radio_Contact()) &&
				(RTTI == RTTI_AIRCRAFT || Map.Is_Same_Cell_Zone(Destination_Coord().As_Cell(), building->Center_Coord().As_Cell(), TClass->MZone, Is_Moving_Onto_Bridge(), false, false)) &&
				((TechnoClass *)this)->Transmit_Message(RADIO_CAN_LOAD, building) == RADIO_ROGER) {

				/*
				**	If the building qualifies and this building is better than the
				**	last qualifying building (as rated by distance), then record
				**	this building and keep scanning.
				*/
				int dist = Relative_Distance(building);
				if (bestval == -1 || dist < bestval || building->IsLeader) {
					best = building;
					bestval = dist;
				}
			}
		}
	}
	return(best);
}


/***********************************************************************************************
 * TechnoClass::Find_Exit_Cell -- Finds an appropriate exit cell for this object.              *
 *                                                                                             *
 *    This routine is called when an object would like to exit from this (presumed) transport. *
 *    A suitable cell should be returned by this routine. The specified object will probably   *
 *    be unloaded at that cell.                                                                *
 *                                                                                             *
 * INPUT:   techno   -- Pointer to the object that would like to unload. This is used to       *
 *                      determine suitability for placement.                                   *
 *                                                                                             *
 * OUTPUT:  Returns with the cell that is recommended for object exit.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/12/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
Cell TechnoClass::Find_Exit_Cell(TechnoClass const *) const
{
	return(Docking_Coord().As_Cell());
}


/***********************************************************************************************
 * TechnoClass::Refund_Amount -- Returns with the money to refund if this object is sold.      *
 *                                                                                             *
 *    This routine is used by the selling back mechanism in order to credit the owning house   *
 *    with some refund credits. The value returned is the credits to refund to the owner.      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the credits to refund to the owner.                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoClass::Refund_Amount(void) const
{
	int cost = TClass->Cost_Of(House);

	if (House->Is_Human_Player()) {
		cost = cost * Rule->RefundPercent;
	}
	return(cost);
}


/***********************************************************************************************
 * TechnoClass::Anti_Air -- Determines the anti-aircraft strength of the object.               *
 *                                                                                             *
 *    This routine will calculate and return the anti-aircraft strength of this object.        *
 *    Typical users of this strength value is the base defense expert system AI.               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the anti-aircraft defense value of this object. The value returned    *
 *          is an abstract number to be used for relative comparisons only.                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoClass::Anti_Air(void) const
{
	if (Is_Weapon_Equipped()) {

		WeaponTypeClass const * weapon = PrimaryWeapon;
		BulletTypeClass const * bullet = weapon->Bullet;
		WarheadTypeClass const * warhead = weapon->WarheadPtr;

		if (bullet->IsAntiAircraft) {
			int value = ((weapon->Attack * warhead->Modifier[ARMOR_ALUMINUM]) * weapon->Range) / weapon->ROF;

			if (TClass->Is_Two_Shooter()) {
				value *= 2;
			}
			return(value/50);
		}
	}
	return(0);
}


/***********************************************************************************************
 * TechnoClass::Anti_Armor -- Determines the anti-armor strength of the object.                *
 *                                                                                             *
 *    This routine is used to examine and calculate the anti-armor strength of this object.    *
 *    Typical user user of this would be the expert system base defense AI.                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the relative anti-armor combat value for this object. The value       *
 *          is abstract and is only to be used in relative comparisons.                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoClass::Anti_Armor(void) const
{
	if (Is_Weapon_Equipped()) {
		if (!PrimaryWeapon->Bullet->IsAntiGround) return(0);

		WeaponTypeClass const * weapon = PrimaryWeapon;
		BulletTypeClass const * bullet = weapon->Bullet;
		WarheadTypeClass const * warhead = weapon->WarheadPtr;
		int mrange = std::min(weapon->Range, 4 * CELL_LEPTON);

		int value = ((weapon->Attack * warhead->Modifier[ARMOR_STEEL]) * mrange * warhead->SpreadFactor) / weapon->ROF;
		if (TClass->Is_Two_Shooter()) {
			value *= 2;
		}
		if (bullet->IsInaccurate) {
			value /= 2;
		}
		return(value/50);
	}
	return(0);
}


/***********************************************************************************************
 * TechnoClass::Anti_Infantry -- Calculates the anti-infantry strength of this object.         *
 *                                                                                             *
 *    This routine is used to determine the anti-infantry strength of this object. The         *
 *    typical user of this routine is the expert system base defense AI.                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the anti-infantry strength of this object. The value returned is      *
 *          abstract and should only be used for relative comparisons.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int TechnoClass::Anti_Infantry(void) const
{
	if (Is_Weapon_Equipped()) {
		if (!PrimaryWeapon->Bullet->IsAntiGround) return(0);

		WeaponTypeClass const * weapon = PrimaryWeapon;
		BulletTypeClass const * bullet = weapon->Bullet;
		WarheadTypeClass const * warhead = weapon->WarheadPtr;
		int mrange = std::min(weapon->Range, 4 * CELL_LEPTON);

		int value = ((weapon->Attack * warhead->Modifier[ARMOR_NONE]) * mrange * warhead->SpreadFactor) / weapon->ROF;
		if (TClass->Is_Two_Shooter()) {
			value *= 2;
		}
		if (bullet->IsInaccurate) {
			value /= 2;
		}
		return(value/50);
	}
	return(0);
}


/***********************************************************************************************
 * TechnoClass::Look -- Performs a look around (map reveal) action.                            *
 *                                                                                             *
 *    This routine will reveal the map around this object.                                     *
 *                                                                                             *
 * INPUT:   incremental -- This parameter can enable a more efficient map reveal logic.        *
 *                         If it is absolutely known that the object has only moved one        *
 *                         cell from its previous location that it performed a Look() at,      *
 *                         then set this parameter to TRUE. It will only perform the look      *
 *                         check on the perimeter cells.                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine is slow, try to call it only when necessary.                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/14/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TechnoClass::Look(bool incremental, bool dontmap)
{
	assert(!IsInLimbo);

	if (IsLocked && (!House->Class->IsMultiplayPassive || Session.Type == GAME_NORMAL)) {
		int sight_increase = 10 * (Get_Coord().Z / Rule->LeptonsPerSightIncrease);
		if (sight_increase > SightIncrease) {
			incremental = false;
		}
		SightIncrease = sight_increase;

		int sight_range = TClass->SightRange * (SightIncrease * 0.01 + 1.0);
		if (Has_Ability(ABILITY_SIGHT) && Rule->VeteranSight != 0.0) {
			sight_range *= Rule->VeteranSight + 1;
		}

		if (sight_range) {
			HouseClass * house = House;
			if (((1 << PlayerPtr->HeapID) & LimpetType) != 0) {
				house = PlayerPtr;
			}
			Map.Sight_From(PositionCoord, sight_range, house, incremental, dontmap);
		}
	}
}


/// <summary>
/// Rocks the object in reaction to a nearby jolt.
/// This routine is called when something goes off near a voxel object, and it works out
/// how hard and in which direction the object should tip. A heavy object shrugs the jolt
/// off, and one far enough away is not disturbed at all.
/// </summary>
/// <param name="coord">Where the jolt came from.</param>
/// <param name="force">How violent the jolt was.</param>
void TechnoClass::Rock(Coord const & coord, float force)
{
	if (TClass->Voxel.VoxLib != NULL && !TClass->Voxel.VoxLib->Load_Failed()) {
		Coord pos = PositionCoord;
		Vector3 vec1;
		vec1.X = pos.X - coord.X;
		vec1.Y = coord.Y - pos.Y;
		vec1.Z = pos.Z - coord.Z;
		Vector3 vec2 = vec1;

		float theta = PrimaryFacing.Current().As_Radian();
		double sangle = std::sin(theta);
		double cangle = std::cos(theta);

		float dist1 = vec1.Length();

		float scale = (float)((0.04f - (double)dist1 * 0.000025f) * force / TClass->Weight);

		if (fabs(dist1) >= 0.00002 && scale >= 0.01f) {
			if (scale > 0.05f) {
				scale = 0.05f;
			}

			vec2.Z = 0.0;
			vec1 = Normalize(vec2);

			dist1 = (float)(vec1.X * cangle + vec1.Y * sangle);
			vec2.X = (float)-(vec1.Z * sangle);
			vec2.Y = (float)+(vec1.Z * cangle);
			vec2.Z = (float)((vec1.X * sangle) - (vec1.Y * cangle));
			float dist2 = vec2.Length();
			// This flips the sign of dist2, so it decides which way the object
			// rocks, and that reaches the multiplayer checksum.
			if (fabs(cangle * dist1 - sangle * dist2 - vec1.X) > 0.0002 || fabs(cangle * dist2 + sangle * dist1 - vec1.Y) > 0.0002) {
				dist2 = -dist2;
			}
			RockingForwardsPerFrame = (float)((double)dist1 * scale / 2.0);
			RockingSidewaysPerFrame = (float)-((double)dist2 * scale);
		}
	}
}


/// <summary>
/// Handles the rocking of the object for one frame.
/// This routine tips the object a little further along whatever rocking motion it was
/// given, holds it back from tipping over entirely, and then eases it toward level again
/// so the disturbance dies away. An object that is sinking ignores all of this and simply
/// keels forward.
/// </summary>
void TechnoClass::Rocking_AI(void)
{
	bool sideways_was_positive = AngleRotatedSideways > 0.00002;
	bool sideways_was_negative = AngleRotatedSideways < -0.00002;

	if (IsSinking) {
		if (AngleRotatedForwards < (float)DEG_TO_RAD(45)) {
			AngleRotatedForwards += 0.03;
		}
		return;
	}

	if (IsRocking) {
		AngleRotatedForwards += RockingForwardsPerFrame;
		AngleRotatedSideways += RockingSidewaysPerFrame;
		return;
	}

	if (RockingSidewaysPerFrame != 0.0f) {

		AngleRotatedSideways += RockingSidewaysPerFrame;

		float angle = (float)DEG_TO_RAD(45);

		if (AngleRotatedSideways > angle) {
			AngleRotatedSideways = angle;
			RockingSidewaysPerFrame = 0.0f;
		} else if (AngleRotatedSideways < -angle) {
			AngleRotatedSideways = -angle;
			RockingSidewaysPerFrame = 0.0f;
		}

		if (sideways_was_positive) {
			if (RockingSidewaysPerFrame > 0.0f) {
				RockingSidewaysPerFrame -= 0.002f;
			} else {
				RockingSidewaysPerFrame -= 0.0050000002374872565;
			}
		} else {
			if (RockingSidewaysPerFrame < 0.0f) {
				RockingSidewaysPerFrame += 0.002f;
			} else {
				RockingSidewaysPerFrame += 0.0050000002374872565;
			}
		}

	} else {
		AngleRotatedSideways = 0.0f;
	}

	if ((sideways_was_positive && AngleRotatedSideways < 0.00002) || (sideways_was_negative && AngleRotatedSideways > -0.00002)) {
		RockingSidewaysPerFrame = 0.0f;
		AngleRotatedSideways = 0.0f;
	}

	bool forwards_was_positive = AngleRotatedForwards > 0.00002;
	bool forwards_was_negative = AngleRotatedForwards < -0.00002;

	if (RockingForwardsPerFrame != 0.0f) {

		AngleRotatedForwards += RockingForwardsPerFrame;

		float angle = (float)DEG_TO_RAD(45);

		FootClass *foot = dynamic_cast<FootClass *>(this);
		if (foot != NULL && ((FootClass *)this)->IsCrushing) {
			angle = (float)DEG_TO_RAD(18);
		}

		if (AngleRotatedForwards > angle) {
			AngleRotatedForwards = angle;
			RockingForwardsPerFrame = 0.0f;
		} else if (AngleRotatedForwards < -angle) {
			AngleRotatedForwards = -angle;
			RockingForwardsPerFrame = 0.0f;
		}

		if (forwards_was_positive) {
			if (RockingForwardsPerFrame > 0.0f) {
				RockingForwardsPerFrame -= 0.002f;
			} else {
				RockingForwardsPerFrame -= 0.0050000002374872565;
			}
		} else {
			if (RockingForwardsPerFrame < 0.0f) {
				RockingForwardsPerFrame += 0.002f;
			} else {
				RockingForwardsPerFrame += 0.0050000002374872565;
			}
		}
	} else {
		AngleRotatedForwards = 0.0f;
	}

	if ((forwards_was_positive && AngleRotatedForwards < 0.00002) || (forwards_was_negative && AngleRotatedForwards > -0.00002)) {
		RockingForwardsPerFrame = 0.0f;
		AngleRotatedForwards = 0.0f;
	}
}


/// <summary>
/// Fetches the point to aim at in order to lead the target.
/// Firing at where a moving target stands is a good way to miss it. This routine guesses
/// where the target will have got to by the time the shot arrives, so that the weapon can
/// be aimed ahead of it instead. A target that is standing still needs no such courtesy.
/// </summary>
/// <returns>Returns with the coordinate to aim at. If there is no target at all, COORD_NONE
/// is returned.</returns>
Coord TechnoClass::Predict_Target_Coord(void) const
{
	Coord coord = COORD_NONE;

	if (TarCom != NULL) {
		coord = TarCom->As_Coord();
		if (TarCom != NULL && TarCom->RTTI == RTTI_UNIT) {
			UnitClass * unit = (UnitClass *)TarCom;
			if (unit != NULL && unit->Locomotion->Is_Moving()) {
				int speed = unit->Current_Speed();
				int distance = Distance(TarCom);
				WeaponTypeClass const * weapon = PrimaryWeapon;
				if (weapon != NULL) {
					int travel = (distance / (weapon->MaxSpeed * 0.9) * speed);
					double dir = unit->PrimaryFacing.Current().As_Radian();
					coord.Y -= std::sin(dir) * travel;
					coord.X += std::cos(dir) * travel;
				}
			}
		}
	}
	return(coord);
}


/// <summary>
/// Returns a per-object cloaking/predator animation offset.
/// Computed as (object ID + PredatorOffset) modulo 400.
/// </summary>
/// <returns>Predator effect offset in the range 0-399.</returns>
int TechnoClass::Get_Predator_Offset(void) const
{
	return((Fetch_ID() + (int)PredatorOffset) % 400);
}


/// <summary>
/// Should this object mend a little of its damage now?
/// Only objects that can heal themselves qualify, whether by their nature or by the grace
/// of veterancy, and only while they are hurt badly enough to be in the yellow. The healing
/// is doled out on a slow tick, so this routine says no far more often than it says yes.
/// </summary>
/// <returns>bool; Should the object self heal this frame?</returns>
bool TechnoClass::Should_Self_Heal_Now(void) const
{
	if (!TClass->IsSelfHealing) {
		if (!Has_Ability(ABILITY_SELF_HEAL)) {
			return(false);
		}
	}
	if ((Frame % (int)(Rule->RepairRate * TICKS_PER_MINUTE)) != 0) {
		return(false);
	}
	return(HealthRatio > Rule->ConditionYellow ? false : true);
}


/// <summary>
/// Lists the members every techno object carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TechnoClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);
	FlasherClass::Serialize(stream);
	StageClass::Serialize(stream);

	stream.Serialize(ActLike);
	stream.Serialize(Cargo);
	stream.Serialize(Veterancy);
	stream.Serialize(ArmorBias);
	stream.Serialize(FirepowerBias);
	stream.Serialize(IdleTimer);
	stream.Serialize(RadarFlashTimer);
	stream.Serialize(RadarPos);
	stream.Serialize(SpiedBy);
	stream.Serialize(Group);
	stream.Serialize(ArchivedTarget);
	stream.Serialize(House);
	stream.Serialize(Cloak);
	stream.Serialize(CloakingDevice);
	stream.Serialize(CloakDelay);
	stream.Serialize(PredatorOffset);
	stream.Serialize(TarCom);
	stream.Serialize(SuspendedTarCom);
	stream.Serialize(PitchAngle);
	stream.Serialize(Arm);
	stream.Serialize(Ammo);
	stream.Serialize(PurchasePrice);
	stream.Serialize(ParticleSystems);
	stream.Serialize(Wave);
	stream.Serialize(AngleRotatedSideways);
	stream.Serialize(AngleRotatedForwards);
	stream.Serialize(RockingSidewaysPerFrame);
	stream.Serialize(RockingForwardsPerFrame);
	stream.Serialize(EnteredByInfType);
	stream.Serialize(Storage);
	stream.Serialize(Door);
	stream.Serialize(BarrelPitch);
	stream.Serialize(PrimaryFacing);
	stream.Serialize(SecondaryFacing);
	stream.Serialize(BurstIndex);
	stream.Serialize(IsBurstResetPending);
	stream.Serialize(BurstResetTimer);
	stream.Serialize(TargetingLaserTimer);
	stream.Serialize(SoundRandomSeed);
	stream.Serialize(SinkingYOffset);
	stream.Serialize(IsSinking);
	stream.Serialize(IsNeedingRescue);
	stream.Serialize(IsUseless);
	stream.Serialize(IsTickedOff);
	stream.Serialize(IsCloakable);
	stream.Serialize(IsLeader);
	stream.Serialize(IsALoaner);
	stream.Serialize(IsLocked);
	stream.Serialize(IsInRecoilState);
	stream.Serialize(IsTethered);
	stream.Serialize(IsOwnedByPlayer);
	stream.Serialize(IsDiscoveredByPlayer);
	stream.Serialize(IsDiscoveredByComputer);
	stream.Serialize(IsALemon);
	stream.Serialize(UnusedCooldown);
	stream.Serialize(Unused1);
	stream.Serialize(SightIncrease);
	stream.Serialize(IsTeamRecruitable);
	stream.Serialize(IsAutocreateRecruitable);
	stream.Serialize(IsRadarTracked);
	stream.Serialize(IsInTransport);
	stream.Serialize(IsRocking);
	stream.Serialize(IsOnPatrol);
	stream.Serialize(IsOnWaypointPatrol);
	stream.Serialize(NearbyObject);
	stream.Serialize(StunDuration);
	stream.Serialize(LimpetType);
	stream.Serialize(LimpetSpeedFactor);
	// ActionLineTimer -- static, shared by every object rather than owned by one.
	// ActionLines
	// TalkBubbleType
	// TalkBubbleOwner
	// TalkBubbleTimer
	// BodyShape
}


/// <summary>
/// Adds this object's state to the running game CRC.
/// This routine serves the multiplayer sync check. Every value that must agree between the
/// machines playing the game is folded in, so that a desynchronization is caught on the
/// frame it happens rather than long afterwards.
/// </summary>
void TechnoClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(Group);
	crc(ActLike);
	crc(ArmorBias);
	crc(FirepowerBias);
	crc((int)IdleTimer);
	crc((int)SpiedBy);
	crc(Cloak);
	crc((int)CloakDelay);
	crc(PredatorOffset);
	crc(PitchAngle);
	crc((int)Arm);
	crc(Ammo);
	crc(PurchasePrice);
	crc(BurstIndex);
	crc(IsBurstResetPending);
	crc((int)BurstResetTimer);
	crc(AngleRotatedSideways);
	crc(AngleRotatedForwards);
	crc(RockingSidewaysPerFrame);
	crc(RockingForwardsPerFrame);
	crc(SinkingYOffset);
	crc(IsSinking);
	crc(IsNeedingRescue);
	crc(IsUseless);
	crc(IsTickedOff);
	crc(IsCloakable);
	crc(IsLeader);
	crc(IsALoaner);
	crc(IsLocked);
	crc(IsInRecoilState);
	crc(IsTethered);
	if (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH) {
		crc(IsOwnedByPlayer);
		crc(IsDiscoveredByPlayer);
		crc(IsDiscoveredByComputer);
	}
	crc(IsALemon);
	crc(StunDuration);
	crc(UnusedCooldown);
	crc(Unused1);
	crc(SightIncrease);
	crc((int)LimpetType);
}


/***********************************************************************************************
 * TechnoClass::Is_Allowed_To_Recloak -- Can this object recloak?                              *
 *                                                                                             *
 *    Determine is this object can recloak now and returns that info. Usually the answer is    *
 *    yes, but it can be overridden be derived classes.                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Can this object recloak now? (presumes it has the ability to cloak)          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TechnoClass::Is_Allowed_To_Recloak(void) const
{
	if (IsCloakable) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Sets the object's archived (saved/secondary) target.
/// </summary>
/// <param name="target">Object to store as the archived target.</param>
void TechnoClass::Assign_Archive_Target(AbstractClass * target)
{
	ArchivedTarget = target;
}


/// <summary>
/// Is this object standing on raised ground?
/// An object up a hill or on a bridge is drawn higher up the screen than its own cell, so
/// it appears to be standing in a different cell than the one it really occupies.
/// </summary>
/// <returns>bool; Is the object on raised ground?</returns>
bool TechnoClass::Is_On_Elevation(void) const
{
	Cell cell = TacticalMap->Coord_To_Cell(Get_Coord());
	return(cell != Get_Cell());
}


/// <summary>
/// Returns the object's current weed (tiberium weed) load.
/// Base implementation always returns 0; overridden by derived classes that carry weed.
/// </summary>
/// <returns>Current weed load (always 0.0 in this base implementation).</returns>
double TechnoClass::Weed_Load(void) const
{
	return(0.0);
}


/// <summary>
/// Damages everything a railgun beam passes through.
/// A railgun does not merely hit what it was aimed at -- it punches a line clean through
/// the world, and anything standing close enough to that line takes the weapon's ambient
/// damage as well. The beam is stopped by the first ground high enough to be in its way,
/// and it may bring a fragile cliff down where it strikes.
/// </summary>
/// <param name="coord">Where the beam is fired from.</param>
/// <param name="abstract">What is being fired at. The beam ends at its center and it always
/// takes damage, whether or not the beam actually passed over it.</param>
/// <param name="weapon">The weapon supplying the ambient damage and the warhead.</param>
/// <returns>Returns with the point the beam came to rest at -- the blocking ground, or the
/// target's own center if nothing got in the way.</returns>
Coord TechnoClass::Railgun_Beam_Damage(Coord & coord, AbstractClass *abstract, WeaponTypeClass *weapon)
{
	Coord center = abstract->Center_Coord();

	bool target_found = false;

	Cell final_cell = coord.As_Cell();
	Vector3 start(coord.X, coord.Y, coord.Z);
	Vector3 end(center.X, center.Y, center.Z);

	Coord final_impact = coord;

	Vector3 delta = end - start;
	DynamicVectorClass<ObjectClass *> targets;

	for (int i = 0; i < RAILGUN_STEPS; i++) {
		final_impact = Lerp(coord, center, i * (1.0f / (RAILGUN_STEPS - 1)));

		if (final_impact.As_Cell() != final_cell) {
			final_cell = final_impact.As_Cell();
			CellClass *cptr = &Map[final_cell];

			TechnoClass *optr = (TechnoClass *)cptr->Cell_Occupier(cptr->IsUnderBridge && final_impact.Z >= LEVEL_LEPTON_H * (cptr->Height + BRIDGE_CELL_HEIGHT));

			while (optr != NULL) {
				if (optr != this) {
					if (optr == abstract) {
						target_found = true;
					}
					if (optr->Is_Techno()) {
						float distance = 0.0f;
						if (optr->RTTI != RTTI_BUILDING) {
							/*
							 * Measure the perpendicular distance from the object to the beam
							 * line: cross the offset to the object with the beam direction,
							 * cross the result back with the beam direction to get a vector
							 * perpendicular to the beam, then project the offset onto it
							 * and normalize.
							 */
							Coord obj_coord = optr->PositionCoord;
							Vector3 obj(obj_coord.X, obj_coord.Y, obj_coord.Z);
							Vector3 off = obj - start;
							Vector3 cross = Vector3::Cross_Product(off, delta);
							Vector3 perp = Vector3::Cross_Product(cross, delta);
							float dot = perp * off;
							distance = fabs(dot / perp.Length());
						}

						if (distance < Rule->RailgunDamageRadius || optr == abstract) {
							if (targets.Count() == 0 || targets.ID(optr) == -1) {
								targets.Add(optr);
							}
						}
					}
				}
				optr = (TechnoClass *)optr->Next;
			}

			/// We hit a cell higher than the impact point, halt gathering targets.
			if (Map.Get_Height_GL(final_impact) > final_impact.Z) {
				/// If the impact was a cell and contains a collapseable cliff, try to collapse it.
				if (Map[final_impact].Is_Tile_Destroyable_Cliff()) {
					if (Percent_Chance(Rule->CollapseChance)) {
						Map.Collapse_Cliff(&Map[final_impact]);
					}
				}
				return(final_impact);
			}
		}
	}

	CellClass *cptr = (CellClass *)abstract;

	if (!target_found) {
		ObjectClass *obj = dynamic_cast<ObjectClass *>(abstract);

		if (obj != NULL) {
			if (targets.Count() == 0 || targets.ID(obj) == -1) {
				targets.Add(obj);
			}
		}
	}

	for (int j = 0; j < targets.Count(); j++) {
		int damage = weapon->AmbientDamage;
		targets[j]->Take_Damage(damage, 0, weapon->WarheadPtr, this);
	}

	/// If the impact was a cell and contains a collapseable cliff, try to collapse it.
	if (cptr->RTTI == RTTI_CELL && cptr->Is_Tile_Destroyable_Cliff()) {
		if (Percent_Chance(Rule->CollapseChance)) {
			Map.Collapse_Cliff(cptr);
		}
	}

	return(center);
}


/// <summary>
/// Adds this object to the radar's tracking list.
/// This routine is used so that the radar can tell what is standing at any given radar
/// pixel without having to search every object in the game.
/// </summary>
void TechnoClass::Radar_Track(void)
{
	Map.Radar_Track(this, RadarPos);
	IsRadarTracked = true;
}


/// <summary>
/// Removes this object from the radar's tracking list.
/// </summary>
void TechnoClass::Radar_Untrack(void)
{
	Map.Radar_Untrack(this, RadarPos);
	IsRadarTracked = false;
}


/// <summary>
/// Draws this object's radar pixel on the map at its radar position.
/// </summary>
void TechnoClass::Plot_On_Radar(void) const
{
	Map.Radar_Pixel(RadarPos);
}


/// <summary>
/// Scores how worthwhile it would be to attack the given target.
/// This routine is the heart of automatic target selection. It weighs the harm the target
/// could do to this object against the harm this object could do to it, how badly hurt the
/// target already is, whether it belongs to the house's declared enemy, and how far out of
/// range the shot would be. Whether the careful per-type weights or the crude house-wide
/// ones are used depends on the house having a threat rating node.
/// </summary>
/// <param name="target">The candidate target to weigh up.</param>
/// <param name="firing_coord">The position the shot would be taken from. Pass COORD_NONE to
/// measure the range from this object's own center.</param>
/// <returns>Returns with the threat score. A target with no type at all scores zero, and
/// every real target scores above that.</returns>
double TechnoClass::Target_Threat(TechnoClass * target, Coord const & firing_coord) const
{
	double target_effectiveness_coefficient;
	double target_special_threat_coefficient;
	double my_effectiveness_coefficient;
	double target_strength_coefficient;
	double target_distance_coefficient;

	TechnoTypeClass const * ttype = TClass;

	if (target->Class_Of() == NULL) {
		return(0);
	}

	if (House->IsThreatRatingNodeActive) {
		my_effectiveness_coefficient = ttype->MyEffectivenessCoefficient;
		target_effectiveness_coefficient = ttype->TargetEffectivenessCoefficient;
		target_special_threat_coefficient = ttype->TargetSpecialThreatCoefficient;
		target_strength_coefficient = ttype->TargetStrengthCoefficient;
		target_distance_coefficient = ttype->TargetDistanceCoefficient;
	} else {
		my_effectiveness_coefficient = Rule->DumbMyEffectivenessCoefficient;
		target_effectiveness_coefficient = Rule->DumbTargetEffectivenessCoefficient;
		target_special_threat_coefficient = Rule->DumbTargetSpecialThreatCoefficient;
		target_strength_coefficient = Rule->DumbTargetStrengthCoefficient;
		target_distance_coefficient = Rule->DumbTargetDistanceCoefficient;
	}

	int my_weapon_index = What_Weapon_Should_I_Use(target);
	WeaponTypeClass const * my_weapon = Get_Class_Weapon_Data(my_weapon_index)->Weapon;
	double threat = 0.0;
	RTTIType target_rtti = target->RTTI;

	if (target_rtti == RTTI_BUILDING || target_rtti == RTTI_INFANTRY || target_rtti == RTTI_UNIT || target_rtti == RTTI_AIRCRAFT) {
		if (target != NULL) {
			int target_weapon_index = target->What_Weapon_Should_I_Use((AbstractClass *)this);
			WeaponTypeClass const * target_weapon = target->Get_Class_Weapon_Data(target_weapon_index)->Weapon;
			WarheadTypeClass const * target_warhead = target_weapon != NULL ? target_weapon->WarheadPtr : NULL;
			if (target_warhead != NULL) {
				if (target->TarCom == (AbstractClass *)this) {
					threat = -target_effectiveness_coefficient * target_warhead->Modifier[ttype->Armor];
				} else {
					threat = target_effectiveness_coefficient * target_warhead->Modifier[ttype->Armor];
				}
			}

			threat += target_special_threat_coefficient * target->TClass->SpecialThreatValue;

			if (House->Enemy != HOUSE_NONE && House->Enemy == target->House->HeapID) {
				threat += Rule->EnemyHouseThreatBonus;
			}
		}
	}

	if (my_weapon && my_weapon->WarheadPtr) {
		threat += my_effectiveness_coefficient * my_weapon->WarheadPtr->Modifier[target->Class_Of()->Armor];
	}

	threat += target->HealthRatio * target_strength_coefficient;

	int range = my_weapon != NULL ? my_weapon->Range : TClass->ThreatRange;
	range /= CELL_LEPTON;

	int dist;
	if (firing_coord != COORD_NONE) {
		dist = firing_coord.Distance_To(target->Center_Coord());
	} else {
		dist = Center_Coord().Distance_To(target->Center_Coord()) / CELL_LEPTON;
	}

	threat += std::max(0, dist - range) * target_distance_coefficient;

	return(threat + 100000.0);
}


/// <summary>
/// Checks whether this veteran or elite unit has the given ability.
/// Veterans use VeteranAbilities; elites use either VeteranAbilities or EliteAbilities.
/// </summary>
/// <param name="ability">The ability to test for.</param>
/// <returns>True if the unit's veterancy grants the ability, false otherwise.</returns>
bool TechnoClass::Has_Ability(AbilityType ability) const
{
	if (Veterancy.Is_Veteran() || Veterancy.Is_Elite()) {
		TechnoTypeClass const * ttype = TClass;
		if (Veterancy.Is_Veteran() && ttype->VeteranAbilities[ability]) {
			return(true);
		}
		if (Veterancy.Is_Elite() && (ttype->VeteranAbilities[ability] || ttype->EliteAbilities[ability])) {
			return(true);
		}
	}
	return(false);
}


/// <summary>
/// Resets the action line display timer to 25.
/// </summary>
void TechnoClass::Reset_Action_Line_Timer(void)
{
	ActionLineTimer = 25;
}


/// <summary>
/// Sets whether action lines are drawn for this object.
/// </summary>
/// <param name="on">True to enable action lines, false to disable.</param>
void TechnoClass::Set_Action_Lines(bool on)
{
	ActionLines = on;
}


/// <summary>
/// Fetches the brightness this object should be drawn at.
/// This routine is what makes a flashing object flash. While the flash is on, the object
/// is drawn at the opposite extreme of its usual brightness, so a dark object glares and
/// a bright one goes dim.
/// </summary>
/// <param name="brightness">The brightness the object would normally be drawn at.</param>
/// <returns>Returns with the brightness to actually draw with.</returns>
int TechnoClass::Apparent_Brightness(int brightness) const
{
	if (((FlashCount / 2) % 2) == 1) {
		return(brightness > 1500 ? 500 : 2000);
	} else {
		return(brightness);
	}
}


/// <summary>
/// Should this object show up on the radar?
/// This routine gathers up every reason an object might stay hidden -- shroud, fog, cloak,
/// being underground, or simply never having been discovered -- and decides whether the
/// radar may draw it. An object the player has merely sensed rather than seen also reports
/// how it gave itself away, so that the radar can announce it.
/// </summary>
/// <param name="detected">Set to how the object was detected when it is only showing
/// because the player sensed it.</param>
/// <returns>bool; Should the object appear on the radar?</returns>
bool TechnoClass::Is_Radar_Visible(DetectedType & detected) const
{
	TechnoTypeClass const * ttype = TClass;
	if (!ttype->IsInvisible) {
		if (ttype->IsRadarVisible) {
			return(true);
		}

		if (House->Is_Player_Control()) {
			return(IsDiscoveredByPlayer ? true : false);
		}

		int height = HeightAGL;
		bool ability_radar_invisible = Has_Ability(ABILITY_RADAR_INVISIBLE);
		bool is_shrouded = Map.Is_Shrouded(Get_Coord()) && MainWindow;
		bool is_fogged = Scen->Special.IsFogOfWar && Map.Is_Fogged(Get_Coord());

		if (!is_fogged && Cloak != CLOAKED && height >= -20 && !ability_radar_invisible && !is_shrouded) {
			return(true);
		}

		if (!Is_Sensed_By_Player()) {
			return(false);
		}

		if (!PlayerPtr->Shares_View_With(House) && !ability_radar_invisible && !is_fogged && !is_shrouded && !IsSinking) {
			detected = (height < -20) ? DETECTED_SUBTERRANEAN : DETECTED_CLOAKED;
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Checks whether the cell at this object's center is sensed by the local player.
/// </summary>
/// <returns>True if the player senses the object's cell, false if there is no player.</returns>
bool TechnoClass::Is_Sensed_By_Player(void) const
{
	if (PlayerPtr != NULL) {
		// A player given the whole map senses every cell, so hidden objects show as their owners see them.
		if (Session.ObiWan) {
			return(true);
		}
		CellClass * cptr = &Map[Center_Coord()];
		return(cptr->Is_Sensed(PlayerPtr->HeapID));
	}
	return(false);
}


/// <summary>
/// Checks whether the cell at this object's center is sensed by the given house.
/// </summary>
/// <param name="house">The house to test sensing for.</param>
/// <returns>True if the house senses the object's cell, false if house is NULL.</returns>
bool TechnoClass::Is_Sensed_By_House(HouseClass const * house) const
{
	if (house != NULL) {
		CellClass * cptr = &Map[Center_Coord()];
		return(cptr->Is_Sensed(house->HeapID));
	}
	return(false);
}


/// <summary>
/// Scrubs a departing target from everything that was aiming at it.
/// This routine is called when a target is about to cease to exist. Anything attacking it
/// falls back to whatever mission it was on beforehand, and any team that was built around
/// it is left without one, rather than being left holding a pointer to something that is
/// no longer there.
/// </summary>
/// <param name="target">The target that is going away.</param>
void TechnoClass::Remove_Target(AbstractClass * target)
{
	for (int i = Technos.Count() - 1; i >= 0; i--) {
		TechnoClass * techno = Technos[i];
		if (techno->TarCom == target) {
			techno->Restore_Mission();
			if (techno->Fetch_RTTI() == RTTI_AIRCRAFT && techno->CurrentMission == MISSION_PATROL) {
				techno->Status = 0;
				((AircraftClass*)techno)->IsLockedStraight = false;
			}
			if (techno->TarCom == target) {
				techno->Assign_Target(NULL);
			}
		}
	}
	for (int j = Teams.Count() - 1; j >= 0; j--) {
		TeamClass * team = Teams[j];
		if (team->MissionTarget == target) {
			team->MissionTarget = NULL;
		}
		if (team->Target == target) {
			team->Target = NULL;
		}
	}
}


/// <summary>
/// Should this weapon lob its shot rather than fire flat?
/// Some weapons always arc their shots. The rest will arc anyway when the target is high
/// enough overhead that a flat shot would only bury itself in the hillside between them.
/// </summary>
/// <param name="which">Which of this object's weapons is being fired.</param>
/// <returns>bool; Should the shot be lobbed? An object with no weapon in that slot answers
/// yes.</returns>
bool TechnoClass::Should_Use_High_Arc(int which) const
{
	WeaponTypeClass const * weapon = Get_Class_Weapon_Data(which)->Weapon;
	if (weapon == NULL) {
		return(true);
	}

	bool result = weapon->IsLobber;

	if (!result) {
		if (TarCom != NULL) {
			int dz = TarCom->Center_Coord().Z - Get_Coord().Z;
			if (dz > 0) {
				if (Point2D(Get_Coord()).Distance_To(Point2D(TarCom->Center_Coord())) < dz) {
					result = true;
				}
			}
		}
	}

	return(result);
}


/// <summary>
/// Decrements the ammunition counter by one if any ammo remains.
/// </summary>
void TechnoClass::Reduce_Ammunition(void)
{
	if (Ammo > 0) {
		Ammo--;
	}
}


/// <summary>
/// Fetches the damage this object does to its surroundings when it dies.
/// The figure is a fraction of the object's own toughness, so a heavier object makes a
/// correspondingly bigger mess of whatever was standing next to it.
/// </summary>
/// <returns>Returns with the collateral damage figure.</returns>
int TechnoClass::Get_Collateral_Damage(void) const
{
	return(int((float)TClass->MaxStrength * TClass->CollateralDamageCoefficient));
}


/// <summary>
/// Removes the damage smoke when the object no longer warrants it.
/// This routine is called as the object's condition is re-examined. A repaired object
/// stops smoking, and so does one that has gone below ground where nobody can see it.
/// </summary>
void TechnoClass::Remove_Damage_Particle(void)
{
	if (HealthRatio > Rule->ConditionYellow || HeightAGL < -10) {
		if (ParticleSystems[ATTACHED_PARTICLE_DAMAGE]) {
			ParticleSystems[ATTACHED_PARTICLE_DAMAGE]->Delete_Me();
		}
	}
}


/// <summary>
/// Sends this object into the building it was waiting to enter.
/// A unit that is sent to a repair bay, or an aircraft sent to a helipad, remembers the
/// building it wants and asks again once it has finished moving. This routine makes that
/// second attempt. An aircraft that only needs rearming settles for docking instead, and
/// a building that has been destroyed in the meantime is simply forgotten.
/// </summary>
/// <returns>bool; Was the enter mission assigned?</returns>
bool TechnoClass::Enter_Object_Nearby(void)
{
	if (NearbyObject != NULL) {
		if (NearbyObject->IsActive) {
			TechnoClass * techno = NearbyObject->As_TechnoClass();
			if (techno != NULL) {
				if (Transmit_Message(RADIO_HELLO, techno) == RADIO_ROGER) {
					Assign_Mission(MISSION_ENTER);
					Assign_Destination(techno);
					NearbyObject = NULL;
					return(true);
				}
				if (techno->RTTI == RTTI_BUILDING && RTTI == RTTI_AIRCRAFT) {
					if (((BuildingClass*)techno)->Class->IsCanUnitReload && !((BuildingClass*)techno)->Class->IsCanUnitRepair) {
						Transmit_Message(RADIO_DOCKING, techno);
						return(false);
					}
				}
			}
		} else {
			NearbyObject = NULL;
		}
	}
	return(false);
}


/// <summary>
/// Assigns a MISSION_MOVE toward a cell near the cached nearby object.
/// Clears the nearby object reference if it is no longer active.
/// </summary>
/// <returns>True if a move mission was assigned; false otherwise.</returns>
bool TechnoClass::Move_To_Object_Nearby(void)
{
	if (NearbyObject != NULL) {
		if (NearbyObject->IsActive) {
			TechnoClass * techno = NearbyObject->As_TechnoClass();
			if (techno != NULL) {
				Assign_Destination(&Map[Nearby_Location(techno)]);
				Assign_Mission(MISSION_MOVE);
				return(true);
			}
		} else {
			NearbyObject = NULL;
		}
	}
	return(false);
}


/// <summary>
/// Scatters any infantry that were heading into this object.
/// The base object never receives infantry, so the routine does nothing here. Objects
/// that infantry can be sent into override it.
/// </summary>
void TechnoClass::Scatter_Incoming_Infantry(void) const
{

}


/// <summary>
/// Keeps this object's blip in the right place on the radar.
/// This routine is called as the object moves about. It works out where the object now
/// belongs on the radar and moves its tracking entry to suit, announcing the first
/// sighting of a cloaked or subterranean enemy as it does so. An object that has come
/// out from under the shroud is marked as discovered while it is at it.
/// </summary>
/// <param name="force_update">Should the radar position be recomputed even for a building?</param>
void TechnoClass::Update_Radar_Position(bool force_update)
{
	if (IsInLimbo) {
		if (IsRadarTracked) {
			Radar_Untrack();
		}
		return;
	}

	if (!IsDiscoveredByPlayer && Session.Type == GAME_NORMAL) {
		IsDiscoveredByPlayer = !Map.Is_Shrouded(Center_Coord());
	}

	Point2D point;
	if (RTTI != RTTI_BUILDING || force_update) {
		point = Map.Coord_To_Radar_Pixel(PositionCoord, false);
	} else {
		point = RadarPos;
	}

	DetectedType detected = DETECTED_NONE;
	bool visible = Is_Radar_Visible(detected);
	Rect radar_rect = Map.RadarSurface->Get_Rect();

	if (detected != DETECTED_NONE) {
		if (visible && (!Is_Foot() || ((FootClass *)this)->CurrentTube < TUBE_FIRST)) {
			if (Submit_Radar_Event(RADAREVENT_ENEMY_SENSED, Get_Coord().As_Cell())) {
				if (detected == DETECTED_CLOAKED) {
					Speak(VOX_CLOAKED_DETECTED);
				} else if (detected == DETECTED_SUBTERRANEAN) {
					Speak(VOX_SUBTERRANEAN_DETECTED);
				}
			}
		}
	}

	if (!radar_rect.Is_Point_Within(point)) {
		if (visible) {
			visible = Map.In_Local_Radar(Get_Target_Cell_Ptr());
			if (visible) {
				point = Map.Coord_To_Radar_Pixel(Get_Coord(), true);
			}
		}
	}

	if (IsRadarTracked) {
		if (point != RadarPos || !visible) {
			Radar_Untrack();
		}
	}

	RadarPos = point;

	if (!IsRadarTracked && visible) {
		Radar_Track();
	}

	if (House == PlayerPtr && RadarFlashTimer > 0) {
		if (((int)RadarFlashTimer) % Rule->FlashFrameTime == 0) {
			Plot_On_Radar();
		}
	}
}


/// <summary>
/// Returns the maximum speed from the techno type class, or 0 if the type is NULL.
/// </summary>
/// <returns>The type's MaxSpeed, or 0 when no type is present.</returns>
int TechnoClass::Get_Max_Speed(void) const
{
	TechnoTypeClass const * ttype = TClass;
	if (ttype) {
		return(ttype->MaxSpeed);
	}
	return(0);
}


/// <summary>
/// Sets the object that is speaking through the talk bubble.
/// This routine is used by trigger actions and team missions to put a bubble over a
/// character for a while and to take it away again afterwards. Giving the bubble to
/// someone also lifts the shroud around them, so the player can see who is talking.
/// </summary>
/// <param name="techno">The object doing the talking. Pass NULL to clear the bubble.</param>
/// <param name="bubble">The bubble to display. TALK_NONE clears it.</param>
void TechnoClass::Set_Talker(TechnoClass * techno, TalkType bubble)
{
	if (techno != NULL && bubble != TALK_NONE) {
		TalkBubbleType = bubble;
		TalkBubbleOwner = techno;
		TalkBubbleTimer = Rule->TalkBubbleTime;
		Map.Sight_From(techno->Center_Coord(), 2, PlayerPtr);
	} else {
		TalkBubbleType = TALK_NONE;
		TalkBubbleOwner = NULL;
		TalkBubbleTimer = 0;
	}
}
