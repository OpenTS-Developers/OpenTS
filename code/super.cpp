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

/* $Header: /CounterStrike/SUPER.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SUPER.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 07/28/95                                                     *
 *                                                                                             *
 *                  Last Update : October 11, 1996 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   SuperClass::AI -- Process the super weapon AI.                                            *
 *   SuperClass::Anim_Stage -- Fetches the animation stage for this super weapon.              *
 *   SuperClass::Discharged -- Handles discharged action for special super weapon.             *
 *   SuperClass::Enable -- Enable this super special weapon.                                   *
 *   SuperClass::Forced_Charge -- Force the super weapon to full charge state.                 *
 *   SuperClass::Impatient_Click -- Called when player clicks on unfinished super weapon.      *
 *   SuperClass::Recharge -- Starts the special super weapon recharging.                       *
 *   SuperClass::Remove -- Removes super weapon availability.                                  *
 *   SuperClass::SuperClass -- Constructor for special super weapon objects.                   *
 *   SuperClass::Suspend -- Suspend the charging of the super weapon.                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "super.h"

#include "_map.h"
#include "_rules.h"
#include "_weapon.h"
#include "building.h"
#include "builtype.h"
#include "bullet.h"
#include "ccrand.h"
#include "cell.h"
#include "crc.h"
#include "data.h"
#include "globals.h"
#include "house.h"
#include "incdec.h"
#include "infantry.h"
#include "infatype.h"
#include "inline.h"
#include "ionblast.h"
#include "language/language.h"
#include "mouse.h"
#include "rules.h"
#include "savestream.h"
#include "sun.h"
#include "suprtype.h"
#include "swizzle.h"
#include "tracker.h"
#include "unit.h"
#include "vector.h"
#include "vox.h"
#include "weapon.h"

#include <algorithm>


/// <summary>
/// Default constructor for the super weapon objects.
/// This routine creates a blank super weapon with no type and no owning house, and adds
/// it to the global super weapon list.
/// </summary>
SuperClass::SuperClass(void) :
	Class(NULL),
	House(NULL),
	Control(0),
	NeedsBuilding(true),
	IsPresent(false),
	IsOneTime(false),
	IsReady(false),
	IsSuspended(false),
	ChargeDrainState(SUSPENDED)
{
	SuperWeapons.Add(this);
}


/***********************************************************************************************
 * SuperClass::SuperClass -- Constructor for special super weapon objects.                     *
 *                                                                                             *
 *    This is the constructor for the super weapons.                                           *
 *                                                                                             *
 * INPUT:   recharge    -- The recharge delay time (in game frames).                           *
 *                                                                                             *
 *          charging    -- Voice to announce that the weapon is charging.                      *
 *                                                                                             *
 *          ready       -- Voice to announce that the weapon is fully charged.                 *
 *                                                                                             *
 *          impatient   -- Voice to announce current charging state.                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
SuperClass::SuperClass(SuperWeaponTypeClass * type, HouseClass * owner) :
	Class(type),
	House(owner),
	Control(0),
	OldStage(-1),
	NeedsBuilding(true),
	IsPresent(false),
	IsOneTime(false),
	IsReady(false),
	IsSuspended(false),
	ChargeDrainState(SUSPENDED)
{
	Create_ID();
	SuperWeapons.Add(this);
	AbstractTypePtrTracker.Add(this);
	HousePtrTracker.Add(this);
}


/// <summary>
/// Destructor for the super weapon objects.
/// This routine unhooks the super weapon from the global super weapon list and from the
/// trackers that watch its weapon type and its owning house.
/// </summary>
SuperClass::~SuperClass(void)
{
	SuperWeapons.Delete(this);
	AbstractTypePtrTracker.Delete(this);
	HousePtrTracker.Delete(this);
}


/***********************************************************************************************
 * SuperClass::Suspend -- Suspend the charging of the super weapon.                            *
 *                                                                                             *
 *    This will temporarily put on hold the charging of the special weapon. This might be the  *
 *    result of insufficient power.                                                            *
 *                                                                                             *
 * INPUT:   on -- Should the weapon charging be suspended? Else, it will unsuspend.            *
 *                                                                                             *
 * OUTPUT:  Was the weapon suspend state changed?                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SuperClass::Suspend(bool on)
{
	if (IsPresent && !IsOneTime && on != IsSuspended && NeedsBuilding) {
		if (!on && !Class->IsManualControl) {
			Control.Start();
		} else {
			Control.Stop();
		}
		IsSuspended = on;
		if (Class->UseChargeDrain) {
			if (on) {
				ChargeDrainState = SUSPENDED;
				House->Deactivate_Firestorm();
			} else {
				ChargeDrainState = CHARGING;
				Control = Class->RechargeTime;
			}
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * SuperClass::Enable -- Enable this super special weapon.                                     *
 *                                                                                             *
 *    This routine is called when the special weapon needs to be activated. This is used for   *
 *    both the normal super weapons and the special one-time super weapons (from crates).      *
 *                                                                                             *
 * INPUT:   onetime  -- Is this a special one time super weapon?                               *
 *                                                                                             *
 *          player   -- Is this weapon for the player? If true, then there might be a voice    *
 *                      announcement of this weapon's availability.                            *
 *                                                                                             *
 *          quiet    -- Request that the weapon start in suspended state (quiet mode).         *
 *                                                                                             *
 * OUTPUT:  Was the special super weapon enabled? Failure might indicate that the weapon was   *
 *          already available.                                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SuperClass::Enable(bool onetime, bool player, bool quiet)
{
	if (!IsPresent) {
		IsPresent = true;
		IsOneTime = onetime;
		bool retval = false;
		if (Class->IsManualControl) {
			OldStage = -1;
			Control = Class->RechargeTime;
			Control.Stop();
		} else {
			retval = Recharge(player && !quiet);
		}
		if (IsOneTime) Forced_Charge(player);
		if (quiet) Suspend(true);
		return(retval);
	}
	return(false);
}


/***********************************************************************************************
 * SuperClass::Remove -- Removes super weapon availability.                                    *
 *                                                                                             *
 *    Call this routine when the super weapon should be removed because of some outside        *
 *    force. For one time special super weapons, they can never be removed except as the       *
 *    result of discharging them.                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Was the special weapon removed and disabled?                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SuperClass::Remove(void)
{
	if (IsPresent) {
		IsReady = false;
		IsPresent = false;
		if (Class->UseChargeDrain) {
			House->Deactivate_Firestorm();
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * SuperClass::Recharge -- Starts the special super weapon recharging.                         *
 *                                                                                             *
 *    This routine is called when the special weapon is allowed to recharge. Suspension will   *
 *    be disabled and the animation process will begin.                                        *
 *                                                                                             *
 * INPUT:   player   -- Is this for a player owned super weapon? If so, then a voice           *
 *                      announcement might be in order.                                        *
 *                                                                                             *
 * OUTPUT:  Was the super weapon begun charging up?                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SuperClass::Recharge(bool player)
{
	if (IsPresent && !IsReady && !IsSuspended) {
		OldStage = -1;
		Control.Start();
		Control = Class->RechargeTime;

		if (Class->UseChargeDrain) {
			ChargeDrainState = CHARGING;
		}

#ifdef _DEBUG
		if (Special.IsSpeedBuild) {
			Control = 1;
		}
#endif

		if (player) {
			Speak(Class->VoxCharging);
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * Superclass::Discharged -- Handles discharged action for special super weapon.               *
 *                                                                                             *
 *    This routine should be called when the special super weapon has been discharged. The     *
 *    weapon will either begin charging anew or will be removed entirely -- depends on the     *
 *    one time flag for the weapon.                                                            *
 *                                                                                             *
 * INPUT:   player   -- Is this special weapon for the player? If so, then there might be a    *
 *                      voice announcement.                                                    *
 *                                                                                             *
 * OUTPUT:  Should the sidebar be reprocessed because the special weapon has been eliminated?  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SuperClass::Discharged(bool player, Cell const & cell)
{
	if (Class->UseChargeDrain) {
		if (House->Is_Human_Player()) {
			if (ChargeDrainState == FIRESTORM_ON) {
				ChargeDrainState = READY;
				Control = Class->RechargeTime - Control.Value() / Rule->ChargeToDrainRatio;
				Deactivate_Firestorm(0, player);
				return(false);
			}
			if (ChargeDrainState == READY) {
				ChargeDrainState = FIRESTORM_ON;
				Control = (Class->RechargeTime - Control.Value()) * Rule->ChargeToDrainRatio;
				Place(cell, player);
				return(false);
			}
		} else {
			if (House->FirestormDefenseActivated) {
				Deactivate_Firestorm(0, player);
			} else {
				Place(cell, player);
			}
		}
		return(false);
	}

	if (Control.Is_Active() && IsPresent && IsReady) {
		Place(cell, player);
		IsReady = false;
		if (IsOneTime) {
			IsOneTime = false;
			return(Remove());
		} else {
			if (Class->IsManualControl) {
				Control = Class->RechargeTime;
				OldStage = -1;
				Control.Stop();
			} else {
				Recharge(player);
			}
		}
	}
	return(false);
}


/***********************************************************************************************
 * SuperClass::AI -- Process the super weapon AI.                                              *
 *                                                                                             *
 *    This routine will process the charge up AI for this super weapon object. If the weapon   *
 *    has advanced far enough to change any sidebar graphic that might represent it, then      *
 *    "true" will be returned. Use this return value to intelligently update the sidebar.      *
 *                                                                                             *
 * INPUT:   player   -- Is this for the player? If it is and the weapon is now fully charged,  *
 *                      then this fully charged state will be announced to the player.         *
 *                                                                                             *
 * OUTPUT:  Was the weapon's state changed such that a sidebar graphic update will be          *
 *          necessary?                                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SuperClass::AI(bool player)
{
	if (IsPresent && (!IsReady || Class->UseChargeDrain) && !IsSuspended) {
		if (!Control.Is_Active()) {
			if (OldStage != -1) {
				OldStage = -1;
				return(true);
			}
		} else {
			if (Control == 0) {
				if (Class->UseChargeDrain) {
					if (ChargeDrainState == FIRESTORM_ON) {
						ChargeDrainState = CHARGING;
						Deactivate_Firestorm(0, player);
						Control = Class->RechargeTime;
					} else {
						ChargeDrainState = READY;
						IsReady = true;
					}
					return(true);
				} else {
					IsReady = true;
					if (player) {
						Speak(Class->VoxRecharge);
					}
					return(true);
				}
			} else {
				if (Anim_Stage() != OldStage) {
					OldStage = Anim_Stage();
					return(true);
				}
			}
		}
	}
	return(false);
}


/***********************************************************************************************
 * SuperClass::Anim_Stage -- Fetches the animation stage for this super weapon.                *
 *                                                                                             *
 *    This will return the current animation stage for this super weapon. The value will be    *
 *    between zero (uncharged) to ANIMATION_STAGES (fully charged). Use this value to render   *
 *    the appropriate graphic on the sidebar.                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current animation stage for this special super weapon powerup.    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *   10/11/1996 JLB : Doesn't show complete until really complete.                             *
 *=============================================================================================*/
int SuperClass::Anim_Stage(void) const
{
	if (IsPresent) {
		int stage;
		if (Class->UseChargeDrain) {
			if (ChargeDrainState == FIRESTORM_ON) {
				stage = ANIMATION_STAGES * (1-double(Class->RechargeTime*Rule->ChargeToDrainRatio-Control.Value()) / (Class->RechargeTime*Rule->ChargeToDrainRatio));
			} else {
				stage = ANIMATION_STAGES * (double(Class->RechargeTime-Control.Value()) / Class->RechargeTime);
			}
		} else {
			if (IsReady) {
				return(ANIMATION_STAGES);
			}
			stage = ANIMATION_STAGES * (double(Class->RechargeTime-Control.Value()) / Class->RechargeTime);
		}
		stage = std::min(stage, ANIMATION_STAGES-1);
		return(stage);
	}
	return(0);
}


/***********************************************************************************************
 * SuperClass::Impatient_Click -- Called when player clicks on unfinished super weapon.        *
 *                                                                                             *
 *    This routine is called when the player clicks on the super weapon icon on the sidebar    *
 *    when the super weapon is not ready yet. This results in a voice message feedback to the  *
 *    player.                                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void SuperClass::Impatient_Click(void) const
{
	if (!Control.Is_Active()) {
		Speak(Class->VoxSuspend);
	} else {
		Speak(Class->VoxImpatient);
	}
}


/***********************************************************************************************
 * SuperClass::Forced_Charge -- Force the super weapon to full charge state.                   *
 *                                                                                             *
 *    This routine will force the special weapon to full charge state. Call it when the weapon *
 *    needs to be instantly charged. The airstrike (when it first becomes available) is a      *
 *    good example.                                                                            *
 *                                                                                             *
 * INPUT:   player   -- Is this for the player? If true, then the full charge state will be    *
 *                      announced.                                                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void SuperClass::Forced_Charge(bool player)
{
	if (IsPresent) {
		IsReady = true;
		Control.Start();
		Control = 0;
//		IsSuspended = false;
		if (player) {
			Speak(Class->VoxRecharge);
		}
		if (Class->UseChargeDrain) {
			ChargeDrainState = READY;
		}
	}
}


/// <summary>
/// Does the super weapon need power in order to charge?
/// </summary>
/// <returns>bool; Is this super weapon dependent upon the owner's power supply?</returns>
bool SuperClass::Is_Powered(void) const
{
	return(Class->IsPowered);
}


/// <summary>
/// Fetches the status text for the super weapon.
/// This routine is used by the sidebar to caption the super weapon's cameo with a
/// description of what it is currently up to.
/// </summary>
/// <returns>Returns with a pointer to the text to display, or NULL if the weapon has
/// nothing worth saying.</returns>
char const * SuperClass::State_String(void) const
{
	if (!IsSuspended) {
		if (Class->UseChargeDrain) {
			switch (ChargeDrainState) {
				case CHARGING:
					return(Fetch_String(TXT_CHARGING));
				case READY:
					return(Fetch_String(TXT_READY));
				case FIRESTORM_ON:
					return(Fetch_String(TXT_FIRESTORM_ON));
				default:
					return(NULL);
			}
		}
		if (Class->Type == SUPER_HUNTER_SEEKER) {
			if (IsReady) {
				return(Fetch_String(TXT_RELEASE_THE_HOUNDS));
			} else {
				return(NULL);
			}
		}
		if (IsReady) {
			return(Fetch_String(TXT_READY));
		} else {
			return(NULL);
		}
	} else {
		return(Fetch_String(TXT_HOLD));
	}
}


/// <summary>
/// Can the super weapon be fired right now?
/// This routine is used to decide whether clicking the cameo should arm the targeting
/// cursor. A suspended weapon can never be fired, no matter how charged it is.
/// </summary>
/// <returns>bool; Is the super weapon ready to be used?</returns>
bool SuperClass::Can_Place(void) const
{
	if (!IsSuspended) {
		if (Class->UseChargeDrain) {
			return(ChargeDrainState != CHARGING ? true : false);
		}
		return(IsReady);
	}
	return(false);
}


/// <summary>
/// Unleashes the super weapon upon the cell specified.
/// This routine is called once the target has been chosen, either by the player
/// clicking on the map or by the computer deciding where to strike. Each kind of super
/// weapon does its own thing here -- some order a launch site to fire, others create
/// their effect outright, and the hunter seeker simply walks out of the nearest
/// suitable building.
/// </summary>
/// <param name="cell">The cell that was designated as the target.</param>
/// <param name="player">Is this for a player owned super weapon? If so, then the
/// targeting cursor gets cleared.</param>
void SuperClass::Place(Cell const & cell, bool player)
{
	switch (Class->Type) {
		case SUPER_DROP_PODS:
			Drop_Pods(cell);
			if (player) {
				Map.IsTargettingMode = SUPER_NONE;
			}
			House->IsRecalcNeeded = true;
			break;

		case SUPER_ION_CANNON:
			if (IsReady) {
				Coord coord = cell.As_Coord(LEVEL_LEPTON_H * Map[cell].Height);
				if (coord != COORD_NONE) {
					if (player) {
						Map.IsTargettingMode = SUPER_NONE;
					}
					House->IsRecalcNeeded = true;
					new IonBlastClass(coord);
				}
			}
			break;

		case SUPER_EM_PULSE:
			if (IsReady && (!IsOneTime || !IsPresent)) {
				int mindist = INT_MAX;
				BuildingClass * launchsite = NULL;
				CellClass * target = &Map[cell];

				/*
				 * Search for a suitable launch site for the EMP.
				 */
				for (int index = 0; index < Buildings.Count(); index++) {
					BuildingClass * b = Buildings[index];
					if (!b->IsInLimbo && b->Class->IsEMPulseCannon && b->House == House && b->Is_Powered_On()) {
						if (b->Distance(target) < mindist) {
							mindist = b->Distance(target);
							launchsite = b;
						}
					}
				}

				/*
				 * If a launch site was found, then proceed with the normal launch
				 * sequence.
				 */
				if (launchsite != NULL) {
					launchsite->Assign_Mission(MISSION_MISSILE);
					launchsite->Commence();
					House->EMPDest = cell;
				}
				if (player) {
					Map.IsTargettingMode = SUPER_NONE;
				}
				House->IsRecalcNeeded = true;
			}
			break;

		case SUPER_FIRESTORM:
			House->Activate_Firestorm();
			if (player) {
				House->IsRecalcNeeded = true;
				Map.Column[1].IsToRedraw = true;
			}
			break;

		case SUPER_MULTI_MISSILE:
		case SUPER_CHEM_MISSILE:
			if (IsReady) {
				if (IsOneTime && IsPresent) {
					Cell target = cell;
					Coord start = target.As_Coord();
					start.Z = Map.Get_Height_GL(start);
					if (Map[target].IsUnderBridge) {
						start.Z += BRIDGE_LEPTON_HEIGHT;
					}

					Cell closest = Map.Closest_Edge_Cell(start);

					WeaponTypeClass const * wtype;
					if (Class->Type == SUPER_CHEM_MISSILE) {
						wtype = Weapons[WeaponTypeClass::From_Name("ChemLauncher")];
					} else {
						wtype = Weapons[WeaponTypeClass::From_Name("MultiLauncher")];
					}

					BulletTypeClass const * btype = wtype->Bullet;
					BulletClass * bullet = Create_Bullet(btype, &Map[start], NULL, wtype->Attack, wtype->WarheadPtr, wtype->MaxSpeed, 100000, false);

					if (bullet) {
						TVelocity3D<double> velocity = TVelocity3D<double>(DIR_E, DIR_N, 100);
						bullet->Unlimbo(closest, velocity);
					}
					if (player) {
						Map.IsTargettingMode = SUPER_NONE;
					}
					House->IsRecalcNeeded = true;
				} else {
					for (int index = 0; index < BuildingTypes.Count(); index++) {
						BuildingTypeClass const * btype = BuildingTypes[index];
						if (btype->IsNukeSilo && (btype->SuperWeapon == Class->HeapID || btype->SuperWeapon2 == Class->HeapID)) {

							/*
							**	Search for a suitable launch site for this missile.
							*/
							BuildingClass * launchsite = House->Find_Building(StructType(index));

							/*
							**	If a launch site was found, then proceed with the normal launch
							**	sequence.
							*/
							if (launchsite) {
								launchsite->Assign_Mission(MISSION_MISSILE);
								launchsite->Commence();
								House->NukeDest = cell;
								launchsite->LastSuperWeaponIndex = Class->Type;
							}

							break;
						}
					}
					if (player) {
						Map.IsTargettingMode = SUPER_NONE;
					}
					House->IsRecalcNeeded = true;
				}
			}
			break;

		case SUPER_HUNTER_SEEKER: {
			BuildingClass * hsbuilding = NULL;
			for (int index = 0; index < Buildings.Count(); index++) {
				BuildingClass * b = Buildings[index];
				if (b->House == House) {
					for (int j = 0; j < Rule->HSBuilding.Count(); j++) {
						if (b->Class == Rule->HSBuilding[j]) {
							hsbuilding = b;
						}
					}
				}
			}

			if (hsbuilding) {
				Cell nearby = Map.Nearby_Location(hsbuilding->PositionCoord.As_Cell(), SPEED_FOOT);
				if (Map.In_Local_Radar(nearby)) {
					UnitClass * hs;
					if (House->ActLike == HOUSE_GOOD) {
						hs = new UnitClass(Rule->GDIHunterSeeker, House);
					} else {
						hs = new UnitClass(Rule->NodHunterSeeker, House);
					}
					if (!hs->Unlimbo(nearby, DIR_E)) {
						delete hs;
					} else {
						hs->Locomotion->Acquire_Hunter_Seeker_Target();
						hs->Assign_Mission(MISSION_ATTACK);
						hs->Commence();
					}
				}
			}
		}
		break;
	}
}


/// <summary>
/// Shuts the firestorm defense back down.
/// This routine is called when the firestorm wall is toggled off or its charge runs
/// out. Any other kind of super weapon ignores the request.
/// </summary>
/// <param name="player">Is this for a player owned super weapon? If so, then the sidebar
/// will want redrawing.</param>
void SuperClass::Deactivate_Firestorm(int, bool player) const
{
	if (Class->Type == SUPER_FIRESTORM) {
		House->Deactivate_Firestorm();
		if (player) {
			House->IsRecalcNeeded = true;
			Map.Column[1].IsToRedraw = true;
		}
	}
}


/// <summary>
/// Is the super weapon building up toward its next use?
/// This routine is used by the sidebar to decide whether the charge-up clock should be
/// drawn over the cameo. A charge-drain weapon is always considered to be charging,
/// since it never simply sits at the ready.
/// </summary>
/// <returns>bool; Is the super weapon charging?</returns>
bool SuperClass::Is_Charging(void) const
{
	if (Class->UseChargeDrain) {
		return(true);
	}
	return(IsReady == false);
}


/// <summary>
/// Fetches the persistent class identifier for the super weapon.
/// </summary>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT STDMETHODCALLTYPE SuperClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_SuperWeaponClass;
	return(S_OK);
}


/// <summary>
/// Lists the members this super weapon carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void SuperClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Class);
	stream.Serialize(House);
	stream.Serialize(Control);
	stream.Serialize(NeedsBuilding);
	stream.Serialize(IsPresent);
	stream.Serialize(IsOneTime);
	stream.Serialize(IsReady);
	stream.Serialize(IsSuspended);
	stream.Serialize(OldStage);
	stream.Serialize(ChargeDrainState);
}


/// <summary>
/// Removes any reference to the object specified.
/// This routine is called when an object is about to be destroyed, so that the super
/// weapon does not keep pointing at the type or the house that is going away.
/// </summary>
/// <param name="target">The object that is about to disappear.</param>
void SuperClass::Detach(AbstractClass const * target, bool all)
{
	if (target == House) {
		House = NULL;
	}
	if (target == Class) {
		Class = NULL;
	}
}


/// <summary>
/// Submits the super weapon's state to the game CRC.
/// This routine is used by the multiplayer synchronization check to prove that every
/// machine agrees about the charge state of this super weapon.
/// </summary>
void SuperClass::Compute_CRC(CRCEngine &crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(IsPresent);
	crc(IsOneTime);
	crc(IsReady);
	crc(OldStage);
	crc(ChargeDrainState);
	crc(NeedsBuilding);
}


/// <summary>
/// Delivers a squad of elite infantry by drop pod.
/// This routine is used by the drop pod super weapon to land a handful of veteran
/// soldiers on the map. Each pod aims for a cell adjacent to the one before it, so the
/// squad arrives as a cluster rather than stacked on one spot, and any soldier that
/// cannot find room to land is quietly thrown away.
/// </summary>
/// <param name="cell">The cell to begin dropping the squad around.</param>
void SuperClass::Drop_Pods(Cell const & cell) const
{
	Cell working_cell = cell;
	int count = Random_Pick(Rule->DropPodInfantryMinimum, Rule->DropPodInfantryMaximum);
	InfantryType i1 = InfantryTypeClass::From_Name("E1");
	InfantryTypeClass const * inftype = InfantryTypes[i1];
	InfantryType i2 = InfantryTypeClass::From_Name("E2");
	InfantryTypeClass const * altinftype = InfantryTypes[i2];

	int toplace = count;
	int attempts = 3 * count;

	while (toplace && attempts--) {
		InfantryClass * inf = (InfantryClass *)(((Random_Double(0.0, 1.0) < 0.5) ? inftype : altinftype)->Create_One_Of(House));

		Cell nearby = Map.Nearby_Location(working_cell, SPEED_FOOT, -1, MZONE_INFANTRY, false, Point2D(1,1), false, false, false, false);
		if (inf->Can_Enter_Cell(&Map[nearby]) == MOVE_OK) {
			inf->Veterancy.Set_Elite(true);
			inf->Link_DropPod();
			inf->PositionCoord = nearby;
			inf->Assign_Destination(&Map[nearby]);
			inf->Locomotion->Move_To(Coord(nearby));
			if (!inf->IsInLimbo) {
				inf->Look();
				inf->Assign_Mission(MISSION_GUARD_AREA);
				inf->Commence();
				toplace--;
			}

			FacingType start_dir = Random_Pick(FACING_N, FACING_NW);

			for (int offset = 0; offset < FACING_COUNT; offset++) {
				FacingType dir = (FacingType)((start_dir + offset) % FACING_COUNT);
				if (inf->Can_Enter_Cell(&Map[Adjacent_Cell(nearby, dir)]) == MOVE_OK) {
					working_cell = Adjacent_Cell(nearby, dir);
					break;
				}
			}

			if (inf->IsInLimbo) {
				inf->Delete_Me();
			}
		} else {
			inf->Delete_Me();
		}
	}
}
