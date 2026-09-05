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

/* $Header: /CounterStrike/UNIT.CPP 1     3/03/97 10:26a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : UNIT.CPP                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : September 10, 1993                                           *
 *                                                                                             *
 *                  Last Update : November 3, 1996 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Recoil_Adjust -- Adjust pixel values in direction specified.                              *
 *   UnitClass::AI -- AI processing for the unit.                                              *
 *   UnitClass::APC_Close_Door -- Closes an APC door.                                          *
 *   UnitClass::APC_Open_Door -- Opens an APC door.                                            *
 *   UnitClass::Active_Click_With -- Intercepts the active click to see if deployment is possib*
 *   UnitClass::Active_Click_With -- Performs specified action on specified cell.              *
 *   UnitClass::Approach_Target -- Handles approaching the target in order to attack it.       *
 *   UnitClass::Assign_Destination -- Assign a destination to a unit.                          *
 *   UnitClass::Blocking_Object -- Determines how a object blocks a unit                       *
 *   UnitClass::Can_Enter_Cell -- Determines cell entry legality.                              *
 *   UnitClass::Can_Fire -- Determines if turret can fire upon target.                         *
 *   UnitClass::Click_With -- Handles player map clicking while this unit is selected.         *
 *   UnitClass::Credit_Load -- Fetch the full credit value of cargo carried.                   *
 *   UnitClass::Crew_Type -- Fetches the kind of crew that this object produces.               *
 *   UnitClass::Debug_Dump -- Displays the status of the unit to the mono monitor.             *
 *   UnitClass::Desired_Load_Dir -- Determines the best cell and facing for loading.           *
 *   UnitClass::Draw_It -- Draws a unit object.                                                *
 *   UnitClass::Edge_Of_World_AI -- Check for falling off the edge of the world.               *
 *   UnitClass::Enter_Idle_Mode -- Unit enters idle mode state.                                *
 *   UnitClass::Fire_Direction -- Determines the direction of firing.                          *
 *   UnitClass::Firing_AI -- Handle firing logic for this unit.                                *
 *   UnitClass::Flag_Attach -- Attaches a house flag to this unit.                             *
 *   UnitClass::Flag_Remove -- Removes the house flag from this unit.                          *
 *   UnitClass::Goto_Clear_Spot -- Finds a clear spot to deploy.                               *
 *   UnitClass::Goto_Tiberium -- Search for and head toward nearest available Tiberium patch.  *
 *   UnitClass::Greatest_Threat -- Fetches the greatest threat for this unit.                  *
 *   UnitClass::Harvesting -- Harvests tiberium at the current location.                       *
 *   UnitClass::Init -- Clears all units for scenario preparation.                             *
 *   UnitClass::Limbo -- Limbo this unit.                                                      *
 *   UnitClass::Mission_Guard -- Special guard mission override processor.                     *
 *   UnitClass::Mission_Guard_Area -- Guard area logic for units.                              *
 *   UnitClass::Mission_Harvest -- Handles the harvesting process used by harvesters.          *
 *   UnitClass::Mission_Hunt -- This is the AI process for aggressive enemy units.             *
 *   UnitClass::Mission_Move -- Handles special move mission overrides.                        *
 *   UnitClass::Mission_Repair -- Handles finding and proceeding on a repair mission.          *
 *   UnitClass::Mission_Unload -- Handles unloading cargo.                                     *
 *   UnitClass::Offload_Tiberium_Bail -- Offloads one Tiberium quantum from the object.        *
 *   UnitClass::Ok_To_Move -- Queries whether the vehicle can move.                            *
 *   UnitClass::Overlap_List -- Determines overlap list for units.                             *
 *   UnitClass::Overrun_Square -- Handles vehicle overrun of a cell.                           *
 *   UnitClass::Per_Cell_Process -- Performs operations necessary on a per cell basis.         *
 *   UnitClass::Pip_Count -- Fetches the number of pips to display on unit.                    *
 *   UnitClass::Random_Animate -- Handles random idle animation for the unit.                  *
 *   UnitClass::Read_INI -- Reads units from scenario INI file.                                *
 *   UnitClass::Receive_Message -- Handles receiving a radio message.                          *
 *   UnitClass::Reload_AI -- Perform reload logic for this unit.                               *
 *   UnitClass::Rotation_AI -- Process any turret or body rotation.                            *
 *   UnitClass::Scatter -- Causes the unit to scatter to a nearby location.                    *
 *   UnitClass::Set_Speed -- Initiate unit movement physics.                                   *
 *   UnitClass::Shape_Number -- Fetch the shape number to use for this unit.                   *
 *   UnitClass::Should_Crush_It -- Determines if this unit should crush an object.             *
 *   UnitClass::Sort_Y -- Give Y coordinate sort value for unit.                               *
 *   UnitClass::Start_Driver -- Starts driving and reserves destination cell.                  *
 *   UnitClass::Take_Damage -- Inflicts damage points on a unit.                               *
 *   UnitClass::Tiberium_Check -- Search for and head toward nearest available Tiberium patch. *
 *   UnitClass::Tiberium_Load -- Determine the Tiberium load as a percentage.                  *
 *   UnitClass::Try_To_Deploy -- The unit attempts to "deploy" at current location.            *
 *   UnitClass::UnitClass -- Constructor for units.                                            *
 *   UnitClass::Unlimbo -- Removes unit from stasis.                                           *
 *   UnitClass::What_Action -- Determines action to perform on specified cell.                 *
 *   UnitClass::What_Action -- Determines what action would occur if clicked on object.        *
 *   UnitClass::Write_INI -- Store the units to the INI database.                              *
 *   UnitClass::delete -- Deletion operator for units.                                         *
 *   UnitClass::new -- Allocate a unit slot and adjust access arrays.                          *
 *   UnitClass::~UnitClass -- Destructor for unit objects.                                     *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "unit.h"

#include "_bench.h"
#include "_convert.h"
#include "_map.h"
#include "_mixfile.h"
#include "_rect.h"
#include "_rtti.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "aircraft.h"
#include "airctype.h"
#include "anim.h"
#include "astar.h"
#include "bench.h"
#include "blit.h"
#include "building.h"
#include "builtype.h"
#include "bullet.h"
#include "bullettype.h"
#include "ccrand.h"
#include "cell.h"
#include "combat.h"
#include "conquer.h"
#include "draw.h"
#include "fog.h"
#include "house.h"
#include "houstype.h"
#include "ilocos.h"
#include "incdec.h"
#include "infantry.h"
#include "infatype.h"
#include "inline.h"
#include "ion.h"
#include "ipiggy.h"
#include "isotype.h"
#include "lightcon.h"
#include "mixfile.h"
#include "mono.h"
#include "overlay.h"
#include "overtype.h"
#include "partsys.h"
#include "revent.h"
#include "rules.h"
#include "savestream.h"
#include "scheme.h"
#include "session.h"
#include "shapeset.h"
#include "sun.h"
#include "swizzle.h"
#include "tactical.h"
#include "tag.h"
#include "tagtype.h"
#include "team.h"
#include "tiberium.h"
#include "tracker.h"
#include "tube.h"
#include "unittype.h"
#include "voc.h"
#include "vox.h"
#include "warhead.h"
#include "weapon.h"

#include "bench.hh"
#include "color.hh"
#include "tube.hh"

#include <algorithm>
#include <vector>

char const * const UnitClass::INI_NAME = "Units";

Rect UnitCompositeDirtyRect(RECT_NONE);


int UnitClass::Visceroid_Fire_List[FACING_COUNT] = { 100, 105, 110, 115, 120, 125, 90, 95 };


/*
**	This is the list of animation stages to use when the harvester
**	is to dump its load into the refinery. The offsets are based from the
**	start of the dump animation.
*/
int UnitClass::Harvester_Load_List[FACING_COUNT] = {4, 5, 6, 7, 0, 1, 2, 3};


/***********************************************************************************************
 * UnitClass::UnitClass -- Constructor for units.                                              *
 *                                                                                             *
 *    This constructor for units will initialize the unit into the game                        *
 *    system. It will be placed in all necessary tracking lists. The initial condition will    *
 *    be in a state of limbo.                                                                  *
 *                                                                                             *
 * INPUT:   classid  -- The type of unit to create.                                            *
 *                                                                                             *
 *          house -- The house owner of this unit.                                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/21/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
UnitClass::UnitClass(UnitTypeClass const * type, HouseClass * house) :
	BASECLASS(house),
	Class((UnitTypeClass *)type),
	Flagged(HOUSE_NONE),
	IsDumping(false),
	IsHarvesting(false),
	Reload(0),
	FiringSyncDelay(-1),
	VisceroidFacing(FACING_NONE),
	DeathCounter(-1),
	FollowingMe(NULL),
	IsFollowing(false),
	IsCompositingToEightBitSurface(false),
	Charge(0)
{
	Create_ID();
	Units.Add(this);
	SecondaryFacing.Set(PrimaryFacing.Current());

	if (Class != NULL) {
		Locomotion = ILocomotionPtr(Class->Locomotor, NULL, CLSCTX_ALL);
		Locomotion->Link_To_Object(this);
	}

	if (Class != NULL) {
		PrimaryFacing.Set_ROT(Class->ROT);
		SecondaryFacing.Set_ROT(Class->ROT);
		Ammo = Class->MaxAmmo;
		IsCloakable = Class->IsCloakable;
		Strength = Class->MaxStrength;
	}

	Reload = 0;

	if (Class != NULL) {
		Charge = Class->StartCharge;
	}

	if (House != NULL) {
		House->Tracking_Add(this);
	}

	Init();

	TargetTracker.Add_Index(Fetch_ID(), this);
}


/***********************************************************************************************
 * UnitClass::~UnitClass -- Destructor for unit objects.                                       *
 *                                                                                             *
 *    This destructor will lower the unit count for the owning house as well as inform any     *
 *    other units in communication, that this unit is about to leave reality.                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/15/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
UnitClass::~UnitClass(void)
{
	if (GameActive && Class != NULL) {

		if (House->Can_Build(Class, false, false) == -1) {
			House->IsRecalcNeeded = true;
		}

		/*
		**	Remove this member from any team it may be associated with. This must occur at the
		**	top most level of the inheritance hierarchy because it may call virtual functions.
		*/
		if (Team != NULL) {
			Team->Remove(this);
			Team = NULL;
		}

		House->Tracking_Remove(this);

		/*
		**	If there are any cargo members, delete them.
		*/
		while (Cargo.Is_Something_Attached()) {
			delete Cargo.Detach_Object();
		}

		Limbo();
	}

	Detach_This_From_All(this);
	Units.Delete(this);
	TargetTracker.Remove_Index(Fetch_ID());
	IsActive = false;
}


#ifdef _DEBUG
/***********************************************************************************************
 * UnitClass::Debug_Dump -- Displays the status of the unit to the mono monitor.               *
 *                                                                                             *
 *    This displays the current status of the unit class to the mono monitor. By this display  *
 *    bugs may be tracked down or prevented.                                                   *
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
void UnitClass::Debug_Dump(MonoClass * mono) const
{
	mono->Set_Cursor(0, 0);
	mono->Set_Cursor(47, 5);mono->Printf("%02X:%02X", SecondaryFacing.Current(), SecondaryFacing.Desired());

	mono->Set_Cursor(1, 11);mono->Printf("%3d", Storage.Get_Total_Amount());
	mono->Set_Cursor(7, 11);mono->Printf("%5d", Storage.Get_Total_Value());

	mono->Fill_Attrib(66, 13, 12, 1, IsDumping ? MonoClass::INVERSE : MonoClass::NORMAL);

	BASECLASS::Debug_Dump(mono);
}
#endif


/// <summary>
/// Handles a unit's passage through a tunnel.
/// This routine walks the vehicle along the segments of the tube it entered, following the
/// tunnel's slope, and hands it back to the map at the far mouth. If the exit cell is
/// already spoken for the unit waits underground, first shooing any idle occupier away.
/// </summary>
void UnitClass::Tunnel_AI(void)
{
	Coord coord;
	bool finished = Tubes[CurrentTube]->Dirs[CurrentTubeDir] == FACING_NONE;
	if (!finished) {
		int distance = Distance(LastTubeCoord);
		TubeClass * tube = Tubes[CurrentTube];
		Get_Coord();	/// Result discarded.

		DirType direction_temp;
		DirType direction = direction_temp.Direction(Center_Coord(), LastTubeCoord);
		int speed = (int)(Class->MaxSpeed * 1.5);

		Coord exit(tube->Exit);
		Coord enter(tube->Enter);
		int height_step = tube->Count;
		int enter_height = Map.Get_Height_GL(enter);
		height_step = (Map.Get_Height_GL(exit) - enter_height) / height_step;

		if (distance <= speed) {
			CurrentTubeDir++;
			FacingType tube_dir = tube->Dirs[CurrentTubeDir];
			finished = tube_dir == FACING_NONE;
			PositionCoord = LastTubeCoord;

			if (!finished) {
				coord = Adjacent_Cell(LastTubeCoord, tube_dir);
				Coord delta = Map[coord].Cell_Coord() - Map[LastTubeCoord].Cell_Coord();
				LastTubeCoord += Coord(delta.X, delta.Y, 0);
				LastTubeCoord.Z += height_step;

				DirType dir = DirType().Direction(Center_Coord(), LastTubeCoord);
				Coord new_coord = Move_Coord(PositionCoord, dir, speed - distance);
				double divisor;
				if (tube_dir % 2) {
					divisor = CELL_LEPTON_H * M_SQRT2;
				} else {
					divisor = CELL_LEPTON_H;
				}
				new_coord.Z += (speed - distance) / divisor * height_step;
				PositionCoord = new_coord;
				return;
			}
		} else {
			FacingType tube_dir = tube->Dirs[CurrentTubeDir];
			Coord new_coord = Move_Coord(Get_Coord(), direction, speed);
			double divisor;
			if (tube_dir % 2) {
				divisor = CELL_LEPTON_H * M_SQRT2;
			} else {
				divisor = CELL_LEPTON_H;
			}
			new_coord.Z += (speed) / divisor * height_step;
			PositionCoord = new_coord;
			return;
		}
	}

	if (finished) {
		CellClass * cellptr = &Map[Get_Coord()];
		ObjectClass * occupier = cellptr->Cell_Occupier();
		bool was_down = true;
		bool was_marked = true;

		while (occupier) {
			if (occupier == this) {
				was_marked = false;
			} else {
				if (occupier->RTTI == RTTI_UNIT || occupier->RTTI == RTTI_INFANTRY) {
					if (!((FootClass *)occupier)->Locomotion->Is_Moving()) {
						Coord scatter_coord(0, 0, 0);
						occupier->Scatter(scatter_coord, true, true);
					}
					was_down = false;
				}
			}
			occupier = occupier->Next;
		}

		if (was_down) {
			TubeClass * tube = Tubes[CurrentTube];
			Cell exit_cell = tube->Exit;
			Coord exit_coord(exit_cell, LastTubeCoord.Z);
			PositionCoord = exit_coord;
			CurrentTube = TUBE_NONE;

			if (was_marked) Mark(MARK_DOWN);

			if (NavCom == cellptr) {
				Scatter(Coord(0, 0, 0), true, true);
			}

			IsPlanningToLook = true;
			Per_Cell_Process(PCP_END);
			Set_Speed(1);

			TubeClass * tube_here = Map[Get_Coord()].Get_Tunnel();
			if (tube_here != NULL) {
				PrimaryFacing.Set(DirType(DirType(tube_here->EnterDir).As_Int() - DirType(FACING_135).As_Int() - 1).Snap_To_8());
			}
		} else {
			Set_Speed(0);
		}
	}
}


/***********************************************************************************************
 * UnitClass::AI -- AI processing for the unit.                                                *
 *                                                                                             *
 *    This routine will perform the AI processing necessary for the unit. These are non-       *
 *    graphic related operations.                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void UnitClass::AI(void)
{
	if (DeathCounter != -1) {
		if (DeathCounter++ >= Class->MaxDeathCounter) {
			Explode();
			Mark(MARK_UP);
			Delete_Me();
			return;
		}
	}

	if (Charge < Class->MaxCharge && !Is_Immobilized()) {
		Charge++;
	}

	if (CurrentTube >= TUBE_FIRST) {
		Tunnel_AI();
		Update_Radar_Position();
		return;
	}

	FiringSyncDelay = std::max(-1, FiringSyncDelay - 1);

	if (Class->DeploysInto == Rule->BuildConst[0]) {
		if (House->IsBaseBuilding && !House->Is_Human_Player()) {
			if (Session.Type != GAME_NORMAL && House->ConYards.Count() == 0) {
				if (CurrentMission != MISSION_HUNT && CurrentMission != MISSION_UNLOAD) {
					Assign_Mission(MISSION_HUNT);
				}
			}
		}
	}

	/*
	**	Act on new orders if the unit is at a good position to do so.
	*/
	if (Ready_To_Commence()) {
//		if (MissionQueue == MISSION_NONE) Enter_Idle_Mode();
		Commence();
	}
	BASECLASS::AI();

	if (IsSinking) {
		static Coord _sink_coords[] = {
			Coord( 0, -1, 0),
			Coord( 1, -1, 0),
			Coord( 1,  0, 0),
			Coord( 1,  1, 0),
			Coord( 0,  1, 0),
			Coord(-1,  1, 0),
			Coord(-1,  0, 0),
			Coord( 0,  0, 0)
		};

		Coord coord = PositionCoord;
		coord += _sink_coords[PrimaryFacing.Current().As_Dir8()] * 3;
		PositionCoord = coord - Coord(0, 0, 9);

		if (HeightAGL < -400) {
			Record_The_Kill(NULL);
			Delete_Me();
			return;
		}
	}

	if (!IsActive) {
		return;
	}

	/*
	**	Hack check to ensure that a harvester won't harvest if it is not harvesting.
	*/
	if (Mission != MISSION_HARVEST) {
		IsHarvesting = false;
	}

	/*
	**	Handle combat logic for this unit. It will determine if it has a target and
	**	if so, if conditions are favorable for firing. When conditions permit, the
	**	unit will fire upon its target.
	*/
	if (!Class->IsJellyfish) {
		Firing_AI();
	}

	/*
	**	Turret rotation processing. Handles rotating radar dish
	**	as well as conventional turrets if present. If no turret present, but
	**	it decides that the body should face its target, then body rotation
	**	would occur by this process as well.
	*/
	Rotation_AI();

	/*
	**	Delete this unit if it finds itself off the edge of the map and it is in
	**	guard or other static mission mode.
	*/
	if (Edge_Of_World_AI()) {
		return;
	}

	if (Class->IsSmallVisceroid || Class->IsLargeVisceroid) {
		Visceroid_AI();
	}

	if (Class->IsJellyfish)	{
		Jellyfish_AI();
	}

	if (Class->IsLimpetDrone && Fetch_Stage() >= 10) {
		Set_Stage(0);
	}

	/*
	**	Units will reload every so often if they are under the burden of
	**	being required to reload between shots.
	*/
	Reload_AI();

	/*
	**	Transporters require special logic handled here since there isn't a MISSION_WAIT_FOR_PASSENGERS
	**	mission that they can follow. Passenger loading is merely a part of their normal operation.
	*/
	if (Class->Max_Passengers() > 0) {

		/*
		**	Double check that there is a passenger that is trying to load or unload.
		**	If not, then close the door.
		*/
		if (!Door.Is_Door_Closed() && Mission != MISSION_UNLOAD && Transmit_Message(RADIO_TRYING_TO_LOAD) != RADIO_ROGER) {
			APC_Close_Door();
		}
	}

	/*
	**	Don't start a new mission unless the vehicle is in the center of
	**	a cell (not driving) and the door (if any) is closed.
	*/
	if (Ready_To_Commence()) {
		Commence();
	}

	/*
	**	A cloaked object that is carrying the flag will always shimmer.
	*/
	if (Cloak == CLOAKED && Flagged != HOUSE_NONE) {
		Do_Shimmer();
	}

	if ((unsigned)Frame % 16 == 0) {
		if ((CurrentMission == MISSION_GUARD_AREA || CurrentMission ==  MISSION_GUARD) && !House->Is_Human_Player() && MissionQueue == MISSION_NONE) {
			if (Strength < Class->MaxStrength && (ArchiveTarget == NULL || CurrentMission == MISSION_GUARD_AREA) && !Class->IsToHarvest && !Class->IsToVeinHarvest) {
				BuildingClass * repair = Find_Unit_Repair_Facility(House, this);
				if (repair != NULL) {
					if (ArchiveTarget == NULL) {
						ArchiveTarget = &Map[Get_Coord()];
					}
					Assign_Mission(MISSION_ENTER);
					Assign_Target(NULL);
					Assign_Destination(repair);
				}
			}
		}
	}
}


/***********************************************************************************************
 * UnitClass::Rotation_AI -- Process any turret or body rotation.                              *
 *                                                                                             *
 *    This routine will handle the rotation logic for the unit's turret (if it has one) as     *
 *    well as its normal body shape.                                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void UnitClass::Rotation_AI(void)
{
	if (TarCom != NULL && !IsRotating) {
		DirType dir = Direction(TarCom);

		if (Class->IsTurretEquipped) {
			SecondaryFacing.Set_Desired(dir);
		} else {

			/*
			**	Non turret equipped vehicles will rotate their body to face the target only
			**	if the vehicle isn't currently moving or facing the correct direction. This
			**	applies only to tracked vehicles. Wheeled vehicles never rotate to face the
			**	target, since they aren't maneuverable enough.
			*/
			if (Class->Speed == SPEED_TRACK && NavCom == NULL && !Locomotion->Is_Moving() && PrimaryFacing.Current() == dir) {
				PrimaryFacing.Set_Desired(dir);
			}
		}
	}

	if (Class->IsRadarEquipped) {
		SecondaryFacing.Set((Dir256)(SecondaryFacing.Current().As_Dir256() + DIR_STEP_32));
	} else {

		IsRotating = false;
		if (Class->IsTurretEquipped) {

			if (SecondaryFacing.Is_Rotating()) {

				/*
				**	If no further rotation is necessary, flag that the rotation
				**	has stopped.
				*/
				if (!Class->IsRadarEquipped) {
					IsRotating = SecondaryFacing.Is_Rotating();
				}
			} else {
				if (TarCom == NULL) {
					if (NavCom == NULL) {
						SecondaryFacing.Set_Desired(PrimaryFacing.Current());
					} else {
						SecondaryFacing.Set_Desired(Direction(NavCom));
					}
				}
			}
		}
	}
}


/***********************************************************************************************
 * UnitClass::Edge_Of_World_AI -- Check for falling off the edge of the world.                 *
 *                                                                                             *
 *    When a unit leaves the map it will be eliminated. This routine checks for this case      *
 *    and eliminates the unit accordingly.                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the unit eliminated by this routine?                                     *
 *                                                                                             *
 * WARNINGS:   Be sure to check for the return value and if 'true' abort any further processing*
 *             of the unit since it is dead. Only call this routine once per unit per          *
 *             game logic loop.                                                                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool UnitClass::Edge_Of_World_AI(void)
{
	if (Mission == MISSION_GUARD && !Map.In_Radar(Get_Coord()) && IsLocked) {
		if (Team != NULL) Team->IsLeaveMap = true;
		Stun();
		Delete_Me();
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * UnitClass::Reload_AI -- Perform reload logic for this unit.                                 *
 *                                                                                             *
 *    Some units require special reload logic. The V2 rocket launcher in particular. Perform   *
 *    this reload logic with this routine.                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only call this routine once per unit per game logic loop.                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void UnitClass::Reload_AI(void)
{
	if (Class->MaxAmmo != -1 && Class->IsNoFireWhileMoving && Ammo < Class->MaxAmmo) {
		if (Locomotion->Is_Moving()) {
			Reload = Reload + 1;
		} else {
			if (Reload == 0) {
				Ammo++;
				if (Ammo < Class->MaxAmmo) {
					Reload = TICKS_PER_SECOND*30;
				}
				Mark(MARK_CHANGE);
			}
		}
	}
}


/// <summary>
/// Should this unit deploy in order to attack?
/// This routine is used to decide whether a deployable vehicle such as the tick tank is
/// better off digging in before it opens fire. A computer controlled tick tank only
/// bothers when its target warrants it and there is somewhere legal to set down.
/// </summary>
/// <returns>bool; Should the unit deploy before firing?</returns>
bool UnitClass::Deploy_To_Fire(void) const
{
	if (Class->IsDeployToFire) {
		return(true);
	}

	if (Class->DeploysInto != NULL && Class->DeploysInto->IsTickTank) {
		if (!House->Is_Human_Player() && TarCom != NULL) {
			switch (TarCom->What_Am_I()) {
				case RTTI_INFANTRY:
					return(false);

				case RTTI_UNIT:
					if (Map[Get_Coord()].Can_Build_Here()) {
						return(true);
					}
					return(false);

				case RTTI_BUILDING:
					return(false);
			}
		}
	}
	return(false);
}


/***********************************************************************************************
 * UnitClass::Firing_AI -- Handle firing logic for this unit.                                  *
 *                                                                                             *
 *    This routine wil check for and perform any firing logic required of this unit.           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This should be called only once per unit per game logic loop.                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void UnitClass::Firing_AI(void)
{
	if (TarCom != NULL && PrimaryWeapon != NULL) {

		/*
		**	Determine which weapon can fire. First check for the primary weapon. If that weapon
		**	cannot fire, then check any secondary weapon. If neither weapon can fire, then the
		**	failure code returned is that from the primary weapon.
		*/
		int primary = What_Weapon_Should_I_Use(TarCom);
		FireErrorType ok = Can_Fire(TarCom, primary);
		WeaponTypeClass const * weapon = Get_Class_Weapon_Data(primary)->Weapon;

		if (weapon) {
			WarheadTypeClass const * warhead = weapon->WarheadPtr;
			if (warhead && warhead->IsWebby && TarCom->RTTI == RTTI_INFANTRY) {
				InfantryClass * inf = dynamic_cast<InfantryClass*>(TarCom);
				if (inf && inf->ProneStruggleTimer > warhead->WebDuration / 4) {
					Assign_Target(NULL);
					Assign_Mission(MISSION_GUARD);
					ok = FIRE_CANT;
				}
			}
		}

		if ((ok == FIRE_OK || ok == FIRE_FACING) && Deploy_To_Fire()) {
			Assign_Mission(MISSION_UNLOAD);
			return;
		}


		switch (ok) {
			case FIRE_ILLEGAL:
				if (Combat_Damage(primary) < 0) {
					ObjectClass * obj = dynamic_cast<ObjectClass*>(TarCom);
					if (obj == NULL || obj->RTTI != RTTI_UNIT) {
						Assign_Target(NULL);
					} else if (obj->HealthRatio >= Rule->ConditionGreen) {
						Assign_Target(NULL);
					}
				}
				break;

			case FIRE_OK:
				if (!Class->IsFireAnim) {
					IsFiring = false;
				}

				if (Class->IsLargeVisceroid || Class->IsSmallVisceroid) {
					Set_Stage(UnitClass::Visceroid_Fire_List[Direction(TarCom).As_Dir8()]);
					Set_Rate(5);
				}

				if (primary != 1) {
					WeaponTypeClass const * weap = Get_Class_Weapon_Data(primary)->Weapon;
					if (weap != NULL) {
						int burst = BurstIndex % weap->Burst;
						if (burst < 2) {
							if (Class->FiringSyncFrame[burst] != -1) {
								bool is_firing_frame;
								if (FiringSyncDelay == -1) {
									FiringSyncDelay = 2 * Class->FiringFrames - 1;
									is_firing_frame = true;
								} else {
									is_firing_frame = FiringSyncDelay == Class->FiringSyncFrame[burst];
								}
								if (!is_firing_frame) {
									break;
								}
							}
						}
					}
				}

				Fire_At(TarCom, primary);
				break;

			case FIRE_FACING:
				if (Class->IsLockTurret || !Class->IsTurretEquipped) {
					if (NavCom == NULL && !Locomotion->Is_Moving()) {
						PrimaryFacing.Set_Desired(Direction(TarCom));
						SecondaryFacing.Set_Desired(PrimaryFacing.Desired());
					}
				} else {
					SecondaryFacing.Set_Desired(Direction(TarCom));
				}
				break;

			case FIRE_CLOAKED:
				IsFiring = false;
				Do_Uncloak();
				break;

			case FIRE_RANGE:
			case FIRE_MUST_DEPLOY:
				IsFiring = false;
				Approach_Target();
				break;
		}
	}
}


/// <summary>
/// Handles the per frame logic for a visceroid.
/// This routine drives the creature's aimless wandering, sends a small visceroid off to
/// merge with any neighbor it stumbles across, and steers a wounded one onto Tiberium so
/// that it can heal.
/// </summary>
void UnitClass::Visceroid_AI(void)
{
	Cell cell = PositionCell;
	int stage = Fetch_Stage();

	if (stage >= 90) {
		if (stage % 5 == 4) {
			Set_Stage(TICKS_PER_SECOND * 2 * Random_Pick(0, 2));
			Set_Rate(1);
		}
		return;
	}

	if (stage % 30 == 29) {
		Set_Stage(TICKS_PER_SECOND * 2 * Random_Pick(0, 2));
	}
	Set_Rate(1);

	if (!Locomotion->Is_Moving()) {
		if (Class->IsSmallVisceroid) {
			FacingType facing = FACING_NW;
			for (int i = 0; i < FACING_COUNT; i++) {
				facing = FacingType(unsigned(facing + FACING_45) % FACING_COUNT);
				Cell adjacent = Adjacent_Cell(cell, facing);
				CellClass * cellptr = &Map[adjacent];
				ObjectClass * occupier = cellptr->Cell_Occupier();
				if (occupier != NULL && occupier->RTTI == RTTI_UNIT && ((UnitClass *)occupier)->Class->IsSmallVisceroid) {
					UnitClass * visceroid = (UnitClass *)occupier;
					if (visceroid->NavCom == NULL && visceroid->TarCom == NULL) {
						visceroid->Assign_Destination(this);
						VisceroidFacing = FACING_NONE;
					}
					return;
				}
			}
		}

		bool is_healing = false;
		if (HealthRatio < Rule->ConditionYellow) {
			if (Map[Center_Coord()].Land_Type() != LAND_TIBERIUM) {
				Goto_Tiberium(16, false);
				if (NavCom != NULL) {
					return;
				}
				VisceroidFacing = FACING_NONE;
			} else {
				VisceroidFacing = FACING_NONE;
				is_healing = true;
			}
		}

		if ((CurrentMission == MISSION_NONE || CurrentMission == MISSION_GUARD) && NavCom == NULL && TarCom == NULL) {
			FacingType dir;
			if (VisceroidFacing != FACING_NONE && Random_Pick(0, 2) != 0) {
				dir = VisceroidFacing;
			} else {
				dir = Random_Pick(FACING_FIRST, FacingType(FACING_COUNT - 1));
				VisceroidFacing = dir;
			}
			Cell adjacent = Adjacent_Cell(cell, dir);
			CellClass * cellptr = &Map[adjacent];
			if (Can_Enter_Cell(cellptr, dir, Get_Cell_Height(), &Map[cell]) == MOVE_OK && (!is_healing || cellptr->Land_Type() == LAND_TIBERIUM)) {
				Assign_Destination(cellptr);
			} else {
				VisceroidFacing = FACING_NONE;
			}
		}
	}
}


/// <summary>
/// Handles the per frame logic for a jellyfish.
/// This routine lets the jellyfish sting anything hostile that drifts into the cells
/// around it, and drives its animation between the idle and stinging states.
/// </summary>
void UnitClass::Jellyfish_AI(void)
{
	Cell pos = PositionCell;
	WeaponTypeClass const * weapon = Class->Get_Weapon(0)->Weapon;
	WarheadTypeClass const * warhead = weapon ? weapon->WarheadPtr : NULL;
	int stage = Fetch_Stage();

	bool time_to_attack = (stage % 8 == 0) || stage == 32;
	bool attacked = false;

	if (warhead != NULL && time_to_attack) {
		Cell cell = (Cell)Center_Coord();
		CellClass * cellptr = &Map[cell];
		for (int x = -1; x <= 1; x++) {
			for (int y = -1; y <= 1; y++) {
				Cell newcell = cell + Cell(x, y);
				CellClass * newcellptr = &Map[newcell];
				if (newcellptr != NULL) {
					Coord cell_crd = newcellptr->Cell_Coord();
					if (abs(cell_crd.Z - PositionCoord.Z) < 3.0 * CELL_LEPTON / 2.0) {
						ObjectClass * occupier = newcellptr->Cell_Occupier(newcellptr->IsUnderBridge && (!cellptr->IsUnderBridge || IsOnBridge));
						while (occupier != NULL) {
							ObjectClass * next = occupier->Next;
							if (occupier->IsActive && occupier->IsDown && !occupier->IsInLimbo && occupier->Is_Techno() && occupier != this) {
								TechnoClass * techno = dynamic_cast<TechnoClass *>(occupier);
								if (techno != NULL && techno != this && techno->Strength > 0) {
									UnitClass * unit = dynamic_cast<UnitClass *>(techno);
									bool visceroid = (unit != NULL && (unit->Class->IsSmallVisceroid || unit->Class->IsLargeVisceroid));

									bool invisible;
									if (unit == NULL) {
										BuildingClass * building = dynamic_cast<BuildingClass *>(techno);
										invisible = (building != NULL && (building->Class->IsInvisibleInGame));
									} else {
										invisible = false;
									}

									if (!visceroid && !invisible && !House->Is_Ally(techno)) {
										int damage = weapon->Attack * warhead->Modifier[techno->TClass->Armor];
										techno->Take_Damage(damage, 0, warhead, this);
										attacked = true;
									}
								}
							}
							occupier = next;
						}
					}
					if (attacked) {
						Sound_Effect((VocType)weapon->Sound.Pick(Scen->RandomNumber));
					}
				}
			}
		}
	}

	switch (stage - 1) {

		case 15:
			if (attacked) {
				Set_Stage(16);
				Just_Set_Rate(3);
			} else {
				Set_Stage(0);
				Just_Set_Rate(1);
			}
			break;

		case 31:
			if (!time_to_attack || attacked) {
				Set_Stage(16);
				Just_Set_Rate(3);
			}
			break;

		case 32:
			if (time_to_attack && attacked) {
				Set_Stage(16);
				Just_Set_Rate(3);
			} else {
				Set_Stage(0);
				Just_Set_Rate(1);
			}
			break;

		default:
			if (stage >= 32 || stage < 0) {
				if (attacked) {
					Set_Stage(16);
					Just_Set_Rate(3);
				} else {
					Set_Stage(0);
					Just_Set_Rate(1);
				}
			}
			break;
	}

	Locomotion->Is_Moving();
}


/***********************************************************************************************
 * UnitClass::Receive_Message -- Handles receiving a radio message.                            *
 *                                                                                             *
 *    This is the handler function for when a unit receives a radio                            *
 *    message. Typical use of this is when a unit unloads from a hover                         *
 *    class so that clearing of the transport is successful.                                   *
 *                                                                                             *
 * INPUT:   from     -- Pointer to the originator of the message.                              *
 *                                                                                             *
 *          message  -- The radio message received.                                            *
 *                                                                                             *
 *          param    -- Reference to an optional parameter the might be needed to return       *
 *                      information back to the originator of the message.                     *
 *                                                                                             *
 * OUTPUT:  Returns with the radio message response.                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/22/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
RadioMessageType UnitClass::Receive_Message(RadioClass * from, RadioMessageType message, int & param)
{
	switch (message) {

		case RADIO_WANT_RIDE:
			if (!Map[Destination_Coord()].IsUnderBridge || Is_Moving_Onto_Bridge()) {
				if (Mission == MISSION_UNLOAD) {
					return(RADIO_NEGATIVE);
				}
				if (IsTethered) {
					return(RADIO_NEGATIVE);
				}
				if (CurrentTube != TUBE_NONE) {
					return(RADIO_NEGATIVE);
				}
				return(RADIO_ROGER);
			}
			return(RADIO_NEGATIVE);

		case RADIO_HOLD_STILL:
			BASECLASS::Receive_Message(from, message, param);
			Assign_Destination(NULL);
			Assign_Target(NULL);
			Assign_Mission(MISSION_SLEEP);
			Clear_Navigation_List();
			if (!IsTethered || !In_Radio_Contact()) {
				Transmit_Message(RADIO_HELLO, from);
				Transmit_Message(RADIO_TETHER);
			}
			return(RADIO_ROGER);

		/*
		**	Asks if the passenger can load on this transport.
		*/
		case RADIO_CAN_LOAD:
			if (Class->Max_Passengers() == 0 || from == NULL || !House->Is_Ally(from)) return(RADIO_STATIC);
			if (Cargo.How_Many() < Class->Max_Passengers()) {
				Cell cell = PositionCell;
				CellClass * cellptr = &Map[cell];
				if (!cellptr->Is_Tile_With_Water() && !cellptr->Is_Tile_Shore()) {
					return(RADIO_ROGER);
				}
			}
			return(RADIO_NEGATIVE);

		/*
		**	The refinery has told this harvester that it should begin the backup procedure
		**	so that proper unloading may take place.
		*/
		case RADIO_BACKUP_NOW:
			BASECLASS::Receive_Message(from, message, param);
			if (!IsRotating && PrimaryFacing.Current() != DIR_E) {
				Locomotion->Do_Turn(DirType(DIR_E));
			} else {
				if (!Locomotion->Is_Moving()) {
					TechnoClass	* whom = Contact_With_Whom();
					if (IsTethered && whom != NULL) {
						if (whom->RTTI == RTTI_BUILDING && Mission == MISSION_ENTER) {
							if (Transmit_Message(RADIO_IM_IN, whom) == RADIO_ROGER) {
								//Transmit_Message(RADIO_UNLOADED, whom);
							}
						}
					}
				}
			}
			return(RADIO_ROGER);

		/*
		**	This message is sent by the passenger when it determines that it has
		**	entered the transport.
		*/
		case RADIO_IM_IN:
			if (Cargo.How_Many() == Class->Max_Passengers()) {
				APC_Close_Door();
			}
			return(RADIO_ATTACH);

		/*
		**	Docking maintenance message received. Check to see if new orders should be given
		**	to the impatient unit.
		*/
		case RADIO_DOCKING:

			/*
			**	If this transport is moving, then always abort the docking request.
			*/
			if (Locomotion->Is_Moving() || NavCom != NULL) {
				return(RADIO_NEGATIVE);
			}

			/*
			**	Check for the case of a docking message arriving from a unit that does not
			**	have formal radio contact established. This might be a unit that is standing
			**	by. If this transport is free to proceed with normal docking operation, then
			**	establish formal contact now. If the transport is completely full, then break
			**	off contact. In all other cases, just tell the pending unit to stand by.
			*/
			if (Contact_With_Whom() != from) {

				/*
				**	Can't ever load up so tell the passenger to bug off.
				*/
				if (Cargo.How_Many() >= Class->Max_Passengers()) {
					return(RADIO_NEGATIVE);
				}

				/*
				**	Establish contact and let the loading process proceed normally.
				*/
				if (!In_Radio_Contact()) {
					Transmit_Message(RADIO_HELLO, from);
				} else {

					/*
					**	This causes the potential passenger to think that all is ok and to
					**	hold on for a bit.
					*/
					//return(RADIO_ROGER);
				}
			}

			if (Class->Max_Passengers() > 0 && Cargo.How_Many() < Class->Max_Passengers()) {
				BASECLASS::Receive_Message(from, message, param);

				if (!Locomotion->Is_Moving() && !IsRotating && !IsTethered) {

					/*
					**	If the potential passenger needs someplace to go, then figure out a good
					**	spot and tell it to go.
					*/
					if (Transmit_Message(RADIO_NEED_TO_MOVE, from) == RADIO_ROGER) {

						/*
						**	Tell the potential passenger where it should go. If the passenger is
						**	already at the staging location, then tell it to move onto the transport
						**	directly.
						*/
						param = (int)this;
						if (Transmit_Message(RADIO_MOVE_HERE, param, from) != RADIO_ROGER) {
							Transmit_Message(RADIO_OVER_OUT, from);
						}

					}
				}
				return(RADIO_ROGER);
			}
			break;

		/*
		**	Something bad has happened to the object in contact with. Abort any coordinated
		**	activity with this object. Basically, ... run away! Run away!
		*/
		case RADIO_RUN_AWAY:
			if (Class->IsToHarvest || Class->IsToVeinHarvest) {
				if (IsDumping) {
					IsDumping = false;
					Scatter(COORD_NONE, true);
					Assign_Mission(MISSION_HARVEST);
					if (Ready_To_Commence()) {
						Commence();
					}
				}
			}
			break;


		/*
		**	When this message is received, it means that the other object
		**	has already turned its radio off. Turn this radio off as well.
		*/
		case RADIO_OVER_OUT:
			if (Mission == MISSION_RETURN) {
				Assign_Mission(MISSION_GUARD);
			}
			BASECLASS::Receive_Message(from, message, param);
			return(RADIO_ROGER);

	}
	return(BASECLASS::Receive_Message(from, message, param));
}


/***********************************************************************************************
 * UnitClass::Unlimbo -- Removes unit from stasis.                                             *
 *                                                                                             *
 *    This routine will place a unit into the game and out of its limbo                        *
 *    state. This occurs whenever a unit is unloaded from a transport.                         *
 *                                                                                             *
 * INPUT:   coord    -- The coordinate to make the unit appear.                                *
 *                                                                                             *
 *          dir      -- The initial facing to impart upon the unit.                            *
 *                                                                                             *
 * OUTPUT:  bool; Was the unit unlimboed successfully?  If the desired                         *
 *                coordinate is illegal, then this might very well return                      *
 *                false.                                                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/22/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool UnitClass::Unlimbo(Coord const & coord, Dir256 dir)
{
	if (BASECLASS::Unlimbo(coord, dir)) {

		SecondaryFacing.Set(dir);

		/*
		**	If it starts off the edge of the map, then it already starts cloaked.
		*/
		if (IsCloakable && !IsLocked) Cloak = CLOAKED;

		/*
		**	Units default to no special animation.
		*/
		if (Class->IsSmallVisceroid || Class->IsLargeVisceroid) {
			Set_Stage(Random_Pick(0, 29));
			Set_Rate(1);
		} else if (Class->IsJellyfish || Class->IsLimpetDrone) {
			Set_Stage(0);
			Set_Rate(1);
		} else {
			Set_Stage(0);
			Set_Rate(0);
		}

		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * UnitClass::Take_Damage -- Inflicts damage points on a unit.                                 *
 *                                                                                             *
 *    This routine will inflict the specified number of damage points on                       *
 *    the given unit. If the unit is destroyed, then this routine will                         *
 *    remove the unit cleanly from the game. The return value indicates                        *
 *    whether the unit was destroyed. This will allow appropriate death                        *
 *    animation or whatever.                                                                   *
 *                                                                                             *
 * INPUT:   damage-- The number of damage points to inflict.                                   *
 *                                                                                             *
 *          distance -- The distance from the damage center point to the object's center point.*
 *                                                                                             *
 *          warhead--The type of damage to inflict.                                            *
 *                                                                                             *
 *          source   -- Who is responsible for this damage?                                    *
 *                                                                                             *
 * OUTPUT:  Returns the result of the damage process. This can range from RESULT_NONE up to    *
 *          RESULT_DESTROYED.                                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/30/1991 JLB : Created.                                                                 *
 *   07/12/1991 JLB : Script initiated by unit destruction.                                    *
 *   04/15/1994 JLB : Converted to member function.                                            *
 *   04/16/1994 JLB : Warhead modifier.                                                        *
 *   06/03/1994 JLB : Added the source of the damage target value.                             *
 *   06/20/1994 JLB : Source is a base class pointer.                                          *
 *   11/22/1994 JLB : Shares base damage handler for techno objects.                           *
 *   06/30/1995 JLB : Lasers do maximum damage against gunboat.                                *
 *   08/16/1995 JLB : Harvester crushing doesn't occur on early missions.                      *
 *=============================================================================================*/
ResultType UnitClass::Take_Damage(int & damage, int distance, WarheadTypeClass const * warhead, TechnoClass * source, bool forced, bool no_crew)
{
	if (Scen->Special.IsHarvesterImmune) {
		if (Rule->HarvesterUnit.Is_In_List(Class)) {
			if (warhead != NULL && warhead->LimpetFactor <= 0.0) {
				return(RESULT_NONE);
			}
		}
	}

	ResultType res = RESULT_NONE;

	/*
	**	Remember if this object was selected. If it was and it gets destroyed and it has
	**	passengers that pop out, then the passengers will inherit the select state.
	*/
	bool select = (IsSelected && House->Is_Player_Control());

	/*
	**	In order for a this to be damaged, it must either be a unit
	**	with a crew or a sandworm.
	*/
	res = BASECLASS::Take_Damage(damage, distance, warhead, source, forced, no_crew);

	if (res == RESULT_ALREADY_DESTROYED) {
		return(res);
	}

	if (res == RESULT_DESTROYED) {
		if (Class->DeathFrames > 0) {
			if (DeathCounter == -1) {
				DeathCounter = 0;
			}
			Strength = 1;
			IsActive = true;
		} else {
			Death_Announcement(source);
			if (warhead == Rule->FirestormWarhead) {
				int count = 7 + abs(Scen->RandomNumber % 3);
				while (count > 0) {
					ParticleSystemClass * partsys = new ParticleSystemClass(Rule->DefaultFirestormExplosionSystem, Center_Coord(), NULL, this);
					partsys->Sparks_To_Use_Random_Direction();
					count--;
				} ;
			} else if (HeightAGL <= 10 && IsToExplode && Map[Get_Coord()].Land_Type() == LAND_WATER) {
				new AnimClass(Rule->Wake, PositionCoord);
				new AnimClass(Rule->SplashList[Rule->SplashList.Count() - 1], PositionCoord + Coord(0, 0, 5));
			} else {
				Explode();
			}

			if (Class->IsTrain) {
				if (IsFollowing) {
					for (int i = 0; i < Units.Count(); i++) {
						if (Units[i]->FollowingMe == this) {
							Units[i]->FollowingMe = NULL;
						}
					}
					IsFollowing = false;
				}
				UnitClass * tmp = FollowingMe;

				while (tmp != NULL) {
					if (tmp->NavCom != NULL) {
						tmp->Locomotion->Stop_Moving();
					}
					if (tmp == tmp->FollowingMe) {
						tmp->FollowingMe = NULL;
						break;
					}
					tmp = tmp->FollowingMe;
				}
			}

			Mark(MARK_UP);

			while (Cargo.Is_Something_Attached()) {
				FootClass * object = Cargo.Detach_Object();

				if (object == NULL) break;		// How can this happen?

				ScenarioInit++;
				object->IsOnBridge = IsOnBridge;

				/*
				**	Only infantry can run from a destroyed vehicle. Even then, it is not a sure
				**	thing.
				*/
				if (object->Is_Infantry() && !forced && !IsToExplode && object->Can_Enter_Cell(&Map[Get_Coord()]) == MOVE_OK && object->Unlimbo(PositionCoord, DIR_N)) {
					object->Scatter(COORD_NONE, true);
					if (select) object->Select();
				} else {
					object->Record_The_Kill(source);
					delete object;
				}

				ScenarioInit--;
			}

			/*
			**	Possibly have the crew member run away.
			*/
			bool make_crew = false;
			bool make_infiltrator = false;
			InfantryClass * i = NULL;

			if (EnteredByInfType == INFANTRY_NONE) {
				if (!no_crew && Class->IsCrew && Class->Max_Passengers() == 0) {
					if (Rule->CrewEscape > Random_Double(0.0, 1.0)) {
						if (EnteredByInfType != INFANTRY_NONE) {
							make_infiltrator = true;
						} else {
							make_crew = true;
						}
					}
				}
			} else {
				make_infiltrator = true;
			}

			if (make_infiltrator) {
				i = new InfantryClass(InfantryTypes[EnteredByInfType], House);
			}

			if (make_crew) {
				InfantryTypeClass const * crew = Crew_Type();
				if (crew) {
					i = new InfantryClass(crew, House);
				}
			}

			if (i != NULL) {
				i->IsOnBridge = IsOnBridge;
				if (i->Unlimbo(PositionCoord, DIR_N)) {
					i->Strength = Random_Pick(5, (int)i->Class->MaxStrength/2);
					i->Scatter(COORD_NONE, true);
					if (!House->Is_Human_Player()) {
						i->Assign_Mission(MISSION_HUNT);
					} else {
						i->Assign_Mission(MISSION_GUARD);
					}
					if (select) i->Select();
					if (Tag != NULL && Tag->Is_To_Inherit()) i->Attach_Tag(Tag);
				} else {
					delete i;
				}
			}

			/*
			**	If this is a truck, there is a possibility that a crate will drop out
			**	if the scenario so indicates and there is room.
			*/
			if (Class->IsCarriesCrate && (Scen->IsTruckCrate && !Class->IsTrain || Scen->IsTrainCargo && Class->IsTrain)) {
				Cell cell = Map.Nearby_Location(PositionCell, SPEED_TRACK, -1, MZONE_NORMAL, false, Point2D(1,1), true);
				if (cell != CELL_NONE) {
					new OverlayClass(Rule->WoodCrateImg, cell);
					Map[cell].Register_For_Redraw();
				}
			}

			/*
			**	Finally, delete the vehicle.
			*/
			Delete_Me();
		}

	} else {

		if (res != RESULT_NONE) {
			if (Class->IsToHarvest && House == PlayerPtr) {
				if (Submit_Radar_Event(RADAREVENT_HARVESTER_ATTACKED, Destination_Coord().As_Cell())) {
					Speak(VOX_HARVESTER_UNDER_ATTACK);
				}
			}
		}

		/*
		**	Try to crush anyone that fires on this unit if possible. The harvester
		**	typically is the only one that will qualify here.
		*/
		if (Team == NULL && source != NULL && !IsTethered && !House->Is_Ally(source) && !House->Is_Human_Player()) {

			/*
			**	Try to crush the attacker if it can be crushed by this unit and this unit is
			**	not equipped with a flame type weapon. If this unit has a weapon and the target
			**	is not very close, then fire on it instead. In easy mode, they never run over the
			**	player. In hard mode, they always do. In normal mode, they only overrun past
			**	mission #8.
			*/
			if (Should_Crush_It(source)) {
				Assign_Destination(source);
				Assign_Mission(MISSION_MOVE);
			} else {

				/*
				**	Try to return to base if possible.
				*/
				BuildingClass * building = NULL;
				if ((Class->IsToHarvest || Class->IsToVeinHarvest) && Pip_Count() > 0 && HealthRatio <= Rule->ConditionYellow) {

					/*
					**	Find nearby refinery and head to it?
					*/
					for (int i = 0; i < Class->Dock.Count(); i++) {
						building = Find_Docking_Bay(Class->Dock[i], false);
						if (building != NULL) {
							break;
						}
					}

					/*
					**	Since the refinery said it was ok to load, establish radio
					**	contact with the refinery and then await docking orders.
					*/
					if (building != NULL && Transmit_Message(RADIO_HELLO, building) == RADIO_ROGER) {
						Assign_Mission(MISSION_ENTER);
					}
				}
			}
		}
	}
	return(res);
}


/***********************************************************************************************
 * UnitClass::Active_Click_With -- Intercepts the active click to see if deployment is possible*
 *                                                                                             *
 *    This routine intercepts the active click operation. It check to see if this is a self    *
 *    deployment request (MCV's have this ability). If it is, then the object is initiated     *
 *    to self deploy. In the other cases, it passes the operation down to the lower            *
 *    classes for processing.                                                                  *
 *                                                                                             *
 * INPUT:   action   -- The action requested of the unit.                                      *
 *                                                                                             *
 *          object   -- The object that the mouse pointer is over.                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool UnitClass::Active_Click_With(ActionType action, ObjectClass * object, bool is_waypoint)
{
	if (action != What_Action(object, is_waypoint)) {
		action = What_Action(object, is_waypoint);
		switch (action) {
			case ACTION_SABOTAGE:
			case ACTION_CAPTURE:
				action = ACTION_ATTACK;
				break;

			case ACTION_ENTER:
				action = ACTION_MOVE;
				break;

			default:
				break;
		}
	}

	if (action == ACTION_HEAL || action == ACTION_GREPAIR) {
		action = ACTION_ATTACK;
	}

	/*
	**	Short circuit out if trying to tell a unit to "nomove" to itself. This bypass of the
	**	normal active click with logic prevents any disturbance to the vehicle's state. Without
	**	this bypass, a unit on a repair bay would stop repairing because it would break radio
	**	contact.
	*/
	if (object == this && action == ACTION_NOMOVE) {
		return(false);
	}

	return(BASECLASS::Active_Click_With(action, object, is_waypoint));
}


/***********************************************************************************************
 * UnitClass::Active_Click_With -- Performs specified action on specified cell.                *
 *                                                                                             *
 *    This routine is called when the mouse has been clicked over a cell and this unit must    *
 *    now respond. Notice that this is merely a placeholder function that exists because there *
 *    is another function of the same name that needs to be overloaded. C++ has scoping        *
 *    restrictions when there are two identically named functions that are overridden in       *
 *    different classes -- it handles it badly, hence the existence of this routine.           *
 *                                                                                             *
 * INPUT:   action   -- The action to perform on the cell specified.                           *
 *                                                                                             *
 *          cell     -- The cell that the action is to be performed on.                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool UnitClass::Active_Click_With(ActionType action, Cell const & cell, bool is_waypoint)
{
	return(BASECLASS::Active_Click_With(action, cell, is_waypoint));
}


/***********************************************************************************************
 * UnitClass::Enter_Idle_Mode -- Unit enters idle mode state.                                  *
 *                                                                                             *
 *    This routine is called when the unit completes one mission but does not have a clear     *
 *    follow up mission to perform. In such a case, the unit should enter a default idle       *
 *    state. This idle state varies depending on what the current internal computer            *
 *    settings of the unit is as well as what kind of unit it is.                              *
 *                                                                                             *
 * INPUT:   initial  -- Is this called when the unit just leaves a factory or is initially     *
 *                      or is initially placed on the map?                                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *   06/03/1994 JLB : Fixed to handle non-combat vehicles.                                     *
 *   06/18/1995 JLB : Allows a harvester to stop harvesting.                                   *
 *=============================================================================================*/
bool UnitClass::Enter_Idle_Mode(bool initial, bool resume_waypoint)
{
	MissionType	order = MISSION_GUARD;

	bool res = BASECLASS::Enter_Idle_Mode(initial, resume_waypoint);

	/*
	**	A movement mission without a NavCom would be pointless to have a radio contact since
	**	no radio coordination occurs on a just a simple movement mission.
	*/
	if (Mission == MISSION_MOVE && NavCom == NULL && PositionCoord == PositionCell.As_Coord()) {
		Transmit_Message(RADIO_OVER_OUT);
	}

	Handle_Navigation_List();
	if (NavCom != NULL) {
		order = MISSION_MOVE;
	} else {

		if (!Class->IsMobileEMP || Mission != MISSION_UNLOAD) {
			if (!Is_Weapon_Equipped()) {
				if (Class->IsToHarvest || Class->IsToVeinHarvest) {
					if (!In_Radio_Contact() && Mission != MISSION_HARVEST && MissionQueue != MISSION_HARVEST) {
						if (initial || !House->Is_Human_Player() || Map[Get_Coord()].Land_Type() == (Class->IsToHarvest ? LAND_TIBERIUM : LAND_WEEDS)) {
							order = MISSION_HARVEST;
						} else {
							order = MISSION_GUARD;
						}
						Assign_Target(NULL);
						Assign_Destination(NULL);
					} else {
						return(res);
					}
				} else {
					if (IsALoaner && Class->Max_Passengers() > 0 && Cargo.Is_Something_Attached() && Team == NULL) {
						order = MISSION_UNLOAD;
					} else {
						if (!IsDeploying && (CurrentMission != MISSION_UNLOAD || Class->DeploysInto == NULL) && CurrentMission != MISSION_GUARD_AREA) {
							order = MISSION_GUARD;
							Assign_Target(NULL);
							Assign_Destination(NULL);
						} else {
							return(res);
						}
					}
				}
			} else {

				if (Mission == MISSION_GUARD || Mission == MISSION_GUARD_AREA || (Mission != MISSION_NONE && (Current_Mission_Control().IsParalyzed || Current_Mission_Control().IsZombie))) {
					return(res);
				}

				if (House->IQ < Rule->IQGuardArea && !Has_Ability(ABILITY_GUARD_AREA) || Team != NULL) {
					order = MISSION_GUARD;
				} else {
					order = MISSION_GUARD_AREA;
				}
			}
		} else {
			return(res);
		}
	}
	if (CurrentMission != MISSION_PATROL && CurrentMission != MISSION_GUARD_AREA) {
		Assign_Mission(order);
	}
	return(res);
}


/***********************************************************************************************
 * UnitClass::Goto_Clear_Spot -- Finds a clear spot to deploy.                                 *
 *                                                                                             *
 *    This routine is used by the MCV to find a clear spot to deploy. If a clear spot          *
 *    is found, then the MCV will assign that location to its navigation computer. This only   *
 *    occurs if the MCV isn't already heading toward a spot.                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool;  Is the located at a spot where it can deploy?                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/27/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool UnitClass::Goto_Clear_Spot(void)
{
	if (!Class->DeploysInto) {
		return(true);
	}

	Mark(MARK_UP);
	if (NavCom == NULL && Class->DeploysInto->Legal_Placement(Adjacent_Cell(Center_Coord().As_Cell(), FACING_NW), NULL)) {
		Mark(MARK_DOWN);
		return(true);
	}

	if (NavCom == NULL) {
		/*
		**	This scan table is skewed to north scanning only. This should
		**	probably be converted to a more flexible method.
		*/
		static Cell _offsets[] = {
			Cell(+0, -1),
			Cell(+0, -2),
			Cell(+1, -2),
			Cell(-1, -2),
			Cell(+0, -3),
			Cell(+1, -3),
			Cell(-1, -3),
			Cell(+2, -3),
			Cell(-2, -3),
			Cell(+0, -4),
			Cell(+1, -4),
			Cell(-1, -4),
			Cell(+2, -4),
			Cell(-2, -4),
//BG: Added south scanning
			Cell(+0, +1),
			Cell(+0, +2),
			Cell(+1, +2),
			Cell(-1, +2),
			Cell(+0, +3),
			Cell(+1, +3),
			Cell(-1, +3),
			Cell(+2, +3),
			Cell(-2, +3),
			Cell(+0, +4),
			Cell(+1, +4),
			Cell(-1, +4),
			Cell(+2, +4),
			Cell(-2, +4),

//BG: Added some token east/west scanning
			Cell(-1, +0),Cell(-2, +0),Cell(-3, +0),Cell(-4, +0),

			Cell(+1, +0),Cell(+2, +0),Cell(+3, +0),Cell(+4, +0)
		};

		for (int i = 0; i < ARRAY_SIZE(_offsets); i++) {
			Cell	cell(Get_Cell()+_offsets[i]);
			Cell	check_cell = Adjacent_Cell(cell, FACING_NW);
			if (Class->DeploysInto->Legal_Placement(check_cell, NULL)) {
				Assign_Destination(&Map[cell]);
				break;
			}
		}
	}
	Mark(MARK_DOWN);

	/*
	**	If we couldn't find a destination to go to, let's try random movement
	**	to see if that brings us to a better spot.
	*/
	if (NavCom == NULL && !House->Is_Human_Player()) {
		Scatter(COORD_NONE);
	}

	return(false);
}


/***********************************************************************************************
 * UnitClass::Try_To_Deploy -- The unit attempts to "deploy" at current location.              *
 *                                                                                             *
 *    Certain units have the ability to deploy into a building. When this routine is called    *
 *    for one of those units, it will attempt to deploy at its current location. If the unit   *
 *    is in motion to a destination or it isn't one of the special units that can deploy or    *
 *    it isn't allowed to deploy at this location for some reason it won't deploy. In all      *
 *    other cases, it will begin to deploy and once it begins only a player abort action will  *
 *    stop it.                                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was deployment begun?                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool UnitClass::Try_To_Deploy(void)
{
	if (NavCom == NULL && !Locomotion->Is_Moving()) {
		if (Class->DeploysInto != NULL) {

			/*
			**	Determine if it is legal to deploy at this location. If not, tell the
			**	player.
			*/
			Mark(MARK_UP);
			Locomotion->Mark_All_Occupation_Bits(MARK_UP);
			Cell cell;
			Cell ncell = PositionCell;
			if (Class->DeploysInto->Is_Mobile_Deployer()) {
				cell = ncell;
			} else {
				cell = Adjacent_Cell(ncell, FACING_NW);
			}
			if (!Class->DeploysInto->Legal_Placement(cell, NULL)) {
				if (House->Is_Player_Control()) {
					Speak(VOX_DEPLOY);
				}
				if (!House->Is_Human_Player()) {
					Class->DeploysInto->Flush_For_Placement(cell, House);
				}
				Locomotion->Mark_All_Occupation_Bits(MARK_DOWN);
				Mark(MARK_DOWN);
				IsDeploying = false;
				return(false);
			}
			Locomotion->Mark_All_Occupation_Bits(MARK_DOWN);
			Mark(MARK_DOWN);

			/*
			**	If the unit is not facing the correct direction, then start it rotating
			**	toward the right facing, but still flag it as if it had deployed. This is
			**	because it will deploy as soon as it reaches the correct facing.
			*/
			Dir256 dfacing = Class->DeploysInto->Deploy_Facing();
			if (PrimaryFacing.Current().As_Dir256() != dfacing) {
				if (!Locomotion->Is_Moving_Now()) {
					Locomotion->Do_Turn(DirType(dfacing));
				}
				Transmit_Message(RADIO_OVER_OUT);
				IsDeploying = true;
				return(true);
			}

			bool selected = IsSelected;

			/*
			**	Since the unit is already facing the correct direction, actually do the
			**	deploy logic. If for some reason this cannot occur, then don't delete the
			**	unit, just mark it as not deploying.
			*/
			Mark(MARK_UP);
			Locomotion->Mark_All_Occupation_Bits(MARK_UP);
			BuildingClass * building = new BuildingClass(Class->DeploysInto, House);
			if (building != NULL) {
				building->Assign_Mission(MISSION_CONSTRUCTION);
				if (building->Unlimbo(cell)) {

					Transmit_Message(RADIO_OVER_OUT);

					for (int i = 0; i < Technos.Count(); i++) {
						TechnoClass * techno = Technos[i];
						if (techno->TarCom != NULL && techno->TarCom->RTTI == RTTI_UNIT && techno->TarCom == this) {
							if (techno->IsActive && techno != this && techno != building) {
								if ((building->Class->IsMobileWar || building->Class->IsConstructionYard) && techno->RTTI == RTTI_INFANTRY && ((InfantryTypeClass *)techno->TClass)->IsVehicleThief) {
									techno->Assign_Target(NULL);
								} else {
									techno->Assign_Target(building);
								}
							}
						}
					}

					building->ActLike = ActLike;
					building->Group = Group;
					building->Veterancy = Veterancy;
					building->LimpetType = LimpetType;
					building->LimpetSpeedFactor = LimpetSpeedFactor;

					/*
					**	Play the buildup sound for the player if this is the players
					**	MCV.
					*/
					if (building->House->Is_Player_Control()) {
						Sound_Effect(Rule->BuildingDrop, Center_Coord());
					} else {
						building->IsToRebuild = true;
						building->IsToRepair = true;
					}

					if (building->Class->IsArtillary || building->Class->IsTickTank) {
						building->BarrelPitch.Set(DIR_E);
					}

					building->Assign_Target(TarCom);

					/*
					**	Always reveal the construction yard to the player that owned the
					**	mobile construction vehicle.
					*/
					building->Revealed(House);

					building->IsReadyToCommence = true;

					/*
					**	When the MCV deploys, always consider production to have started
					**	for the owning house. This ensures that in multiplay, computer
					**	opponents will begin construction as soon as they start their
					**	base.
					*/
					if (!House->Is_Human_Player() && building->Class->IsConstructionYard && Session.Type != GAME_NORMAL) {
						Cell center = building->PositionCoord.As_Cell();
						House->Center = center;
						House->Begin_Construction(center);
						House->IsStarted = true;
						House->IsAITriggersOn = true;
						House->IsBaseBuilding = true;
					}

					/*
					**	Force the newly placed construction yard to be in the same strength
					**	ratio as the MCV that deployed into it.
					*/
					building->Strength = HealthRatio * building->Class->MaxStrength;
					building->Strength = std::max(building->Strength, 1);

					if (selected) building->Select();

					/*
					**	Force the MCV to drop any flag it was carrying.  This will also set
					**	the owner house's flag home cell (since the house's FlagHome is
					**	presumably 0 at this point).
					*/
					Stun();

					if (Tag != NULL) {
						building->Attach_Tag(Tag);
						Tag->AttachCount--;
						Tag = NULL;
					}

					Delete_Me();
					return(true);
				} else {

					/*
					**	Could not deploy the construction yard at this location! Just revert
					**	back to normal "just sitting there" mode and await further instructions.
					*/
					delete building;
				}
			}
			Locomotion->Mark_All_Occupation_Bits(MARK_DOWN);
			Mark(MARK_DOWN);
			IsDeploying = false;
		}
	}
	return(false);
}


/***********************************************************************************************
 * UnitClass::Per_Cell_Process -- Performs operations necessary on a per cell basis.           *
 *                                                                                             *
 *    This routine will perform the operations necessary that occur when a unit is at the      *
 *    center of a cell. These operations could entail deploying into a construction yard,      *
 *    radioing a transport unit, and looking around for the enemy.                             *
 *                                                                                             *
 * INPUT:   why   -- Specifies the circumstances under which this routine was called.          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/18/1994 JLB : Created.                                                                 *
 *   06/17/1995 JLB : Handles case when building says "NO!"                                    *
 *   06/30/1995 JLB : Gunboats head back and forth now.                                        *
 *=============================================================================================*/
void UnitClass::Per_Cell_Process(PCPType why)
{
	Cell	cell = PositionCell;

	if (why == PCP_END || why == PCP_ROTATION) {
		/*
		**	Check to see if this is merely the end of a rotation for the MCV as it is
		**	preparing to deploy. In this case, it should begin its deploy process.
		*/
		if (IsDeploying) {
			Try_To_Deploy();
			if (!IsActive) return;			// Unit no longer exists -- bail.
		}
	}

	BStart(BENCH_PCP);
	if (why == PCP_END) {

		if (Class->IsSmallVisceroid) {
			if (NavCom != NULL && NavCom->RTTI == RTTI_UNIT) {
				UnitClass * unit = static_cast<UnitClass*>(NavCom);
				if (unit->Class->IsSmallVisceroid) {
					if (Center_Coord().As_Cell() == NavCom->Center_Coord().As_Cell()) {
						unit->Class = const_cast<UnitTypeClass *>(Rule->LargeVisceroid);
						unit->Strength = unit->Class->MaxStrength;
						Map[unit->LastAdjacencyCell].Adjust_Threat(unit->Owner(), unit->Risk());
						Delete_Me();
						BEnd(BENCH_PCP);
						return;
					}
				}
			}
		}

		TechnoClass	* whom = Contact_With_Whom();
		if ((Mission == MISSION_ENTER || Mission == MISSION_PATROL) && whom != NULL) {
			Cell center = Center_Coord();
			Cell whom_center = whom->Center_Coord();
			if (Center_Coord().As_Cell() == whom->Center_Coord().As_Cell() && whom->RTTI == RTTI_BUILDING) {
				IPersistPtr persist(Locomotion);
				CLSID clsid;
				persist->GetClassID(&clsid);
				if (clsid == CLSID_HoverLocomotion && static_cast<BuildingClass *>(whom)->Class->IsCanUnitRepair && NavCom == NULL) {
					NavCom = whom;
				}
				if (whom == NavCom) {
					BASECLASS::Per_Cell_Process(PCP_END);
					Transmit_Message(RADIO_IM_IN);
					Locomotion->Power_Off();
					BEnd(BENCH_PCP);
					return;
				}
			}
		}

		/*
		**	If this is a unit that is driving onto a building then the unit must enter
		**	the building as the final step.
		*/
		if (IsTethered && whom != NULL) {
			if (whom->RTTI == RTTI_BUILDING && Mission == MISSION_ENTER) {
				if (whom == Map[cell - Cell(0, 1)].Cell_Building()) {
					switch (Transmit_Message(RADIO_IM_IN, whom)) {
						case RADIO_ROGER:
							break;

						case RADIO_ATTACH:
							break;

						default:
							Scatter(COORD_NONE, true);
							break;
					}
				}
			}
		}

		TechnoClass * techno = Contact_With_Whom();

		/*
		**	Unit entering a transport vehicle will break radio contact
		**	and attach itself to the transporter.
		*/
		if (Mission == MISSION_ENTER && techno && PositionCell == techno->PositionCell && techno == NavCom) {
			BASECLASS::Per_Cell_Process(PCP_END);
			if (Transmit_Message(RADIO_IM_IN) == RADIO_ATTACH) {
				Limbo();
				techno->Cargo.Attach(this);
				Hidden();
			}
			BEnd(BENCH_PCP);
			return;
		}

		/*
		**	When breaking away from a transport object or building, possibly
		**	scatter or otherwise begin normal unit operations.
		*/
		if (IsTethered && (Mission != MISSION_ENTER ||
				(Dynamic_Cast<TechnoClass *>(NavCom) != NULL && Contact_With_Whom() != NavCom)
				) &&
				Mission != MISSION_UNLOAD) {

			bool arrived = NavCom == NULL;
			if (Is_Target_Cell(NavCom)) {
				CellClass *cptr = static_cast<CellClass *>(NavCom);
				if (cptr->CellID == PositionCoord.As_Cell()) {
					arrived = true;
				}
			}

			/*
			**	Special hack check to make sure that even though it has moved one
			**	cell, if it is still on the building (e.g., service depot), have
			**	it scatter again.
			*/
			if (Map[(Coord const &)PositionCoord].Cell_Building() == NULL || arrived) {
				TechnoClass * contact = Contact_With_Whom();
				if (arrived || contact == NULL || contact->RTTI != RTTI_BUILDING || !static_cast<BuildingClass *>(contact)->Class->IsWeaponsFactory || Map[(Coord const &)PositionCoord].Cell_Building() != contact) {
					RadioMessageType response = Transmit_Message(RADIO_UNLOADED);
					if (response == RADIO_RUN_AWAY) {
						if (NavCom != NULL && NavCom != Get_Cell_Ptr()) {

							/*
							**	Special case hack to allow automatic transition to loading
							**	onto a transport (or other situation) if the destination
							**	so indicates.
							*/
							TechnoClass * techno = Dynamic_Cast<TechnoClass *>(NavCom);
							if (techno != NULL) {
								Transmit_Message(RADIO_DOCKING, techno);
							}
						} else {
							BuildingClass * building = dynamic_cast<BuildingClass *>(contact);
							if (!House->Is_Human_Player() && building != NULL && building->Class->IsWeaponsFactory) {
								Cell where = House->Where_To_Go(this);
								if (where != CELL_NONE) {
									Assign_Mission(MISSION_MOVE);
									Assign_Destination(&Map[where]);
									Commence();
									ArchiveTarget = &Map[where];
									Assign_Mission(MISSION_GUARD_AREA);
								} else {
									ArchiveTarget = NULL;
								}
							} else {
								AbstractClass * target = ArchiveTarget;
								if (target != NULL && target != NavCom) {
									Assign_Destination(target);
								} else {
									NavCom = NULL;
									Scatter(COORD_NONE, true);
								}
							}
						}

					} else {
						if (response != RADIO_NEGATIVE && (Class->IsToHarvest || Class->IsToVeinHarvest)) {
							if (ArchiveTarget != NULL) {
								Assign_Mission(MISSION_HARVEST);
								Assign_Destination(ArchiveTarget);
								ArchiveTarget = NULL;
							} else {

								/*
								**	Since there is no place to go, move away to clear
								**	the pad for another harvester.
								*/
								if (NavCom == NULL) {
									Scatter(COORD_NONE, true);
								}
							}
						}
					}
				}
			}

			if (Map[(Coord const &)PositionCoord].Cell_Building() && NavCom == NULL && NavQueue.Count() == 0 && RouteQueue.Count() == 0) {
				Scatter(COORD_NONE, true, true);
			}
		}

		/*
		**	If this is a loaner unit and is is off the edge of the
		**	map, then it gets eliminated. That is, unless it is carrying cargo. This means that
		**	it is probably carrying an incoming reinforcement and it should not be eliminated.
		*/
		if (Edge_Of_World_AI()) {
			BEnd(BENCH_PCP);
			return;
		}

		/*
		**	The unit performs looking around at this time. If the
		**	unit moved further than one square during the last track
		**	move, don't do an incremental look. Do a full look around
		**	instead.
		*/
		if (IsPlanningToLook) {
			IsPlanningToLook = false;
			Look(false);
		} else {
			Look(true);
		}

		bool broke_ice = false;
		if (Scen->Theater == THEATER_SNOW) {
			Map.DirtyIceCells.Clear();
			if (Class->Weight >= Rule->IceBreakingWeight) {
				broke_ice = Map.Break_Ice(&Map[(Coord const &)PositionCoord], this);
			} else if (Class->Weight >= Rule->IceCrackingWeight) {
				broke_ice = Map.Crack_Ice(&Map[(Coord const &)PositionCoord], this);
			}
			if (broke_ice) {
				IsSinking = true;
				Stun();
				Map.Recalc_Ice_Cells();
			}
		}

		/*
		**	Act on new orders if the unit is at a good position to do so.
		*/
		if (!IsDumping && Ready_To_Commence()) {
			Commence();
		}

		if (Class->IsToHarvest && Mission != MISSION_UNLOAD && CurrentMission != MISSION_ENTER && MissionQueue != MISSION_ENTER && !IsDumping) {
			RadioClass * radio = Contact_With_Whom();
			if (radio != NULL && radio->What_Am_I() == RTTI_BUILDING && static_cast<BuildingClass *>(radio)->Class->IsRefinery) {
				Transmit_Message(RADIO_OVER_OUT);
			}
		}

		if (Class->IsToVeinHarvest && Mission != MISSION_UNLOAD && CurrentMission != MISSION_ENTER && MissionQueue != MISSION_ENTER && !IsDumping) {
			RadioClass * radio = Contact_With_Whom();
			if (radio != NULL && radio->What_Am_I() == RTTI_BUILDING && static_cast<BuildingClass *>(radio)->Class->IsWeeder) {
				Transmit_Message(RADIO_OVER_OUT);
			}
		}

		/*
		**	Certain units require some setup time after they come to a halt.
		*/
		if (NavCom == NULL && Path[0] == FACING_NONE) {
			if (Class->IsNoFireWhileMoving) {
				Arm = Rearm_Delay(true)/4;
			}
		}

		/*
		**	If after all is said and done, the unit finishes its move on an impassable cell, then
		**	it must presume that it is in the case of a unit driving onto a bridge that blows up
		**	before the unit completes it's move. In such a case the unit should have been destroyed
		**	anyway, so blow it up now.
		*/
		CellClass * cellptr = &Map[(Coord const &)PositionCoord];
		LandType land = cellptr->Land_Type();
		if (!Locomotion->Is_Moving() && Can_Enter_Cell(cellptr) == MOVE_NO && (!IsOnBridge || !cellptr->IsUnderBridge) && !IsSinking) {
			new AnimClass(Combat_Anim(Strength, Rule->C4Warhead, land, PositionCoord), PositionCoord, 0, 1, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ZGRAD), Get_Explosion_Z(PositionCoord));
			int damage = Strength;
			Combat_Lighting(Center_Coord(), damage, Rule->C4Warhead, false);
			Take_Damage(damage, 0, Rule->C4Warhead, NULL, true);
			BEnd(BENCH_PCP);
			return;
		}
	}

	/*
	**	Destroy any crushable wall that is driven over by a tracked vehicle.
	*/
	CellClass * cellptr = &Map[cell];
	if ((Class->IsCrusher || Has_Ability(ABILITY_CRUSHER)) && cellptr->Overlay != OVERLAY_NONE) {
		OverlayTypeClass const * optr = OverlayTypes[cellptr->Overlay];

		if (optr->IsCrushable) {
			Sound_Effect(optr->CrushSound, Center_Coord());
			cellptr->Reduce_Wall(-1);
			IsCrushing = false;
			RockingForwardsPerFrame += 0.02f;
		}
	}

	/*
	**	Check to see if crushing of any unfortunate infantry is warranted.
	*/
	Overrun_Square(PositionCell, false);

	if (!IsActive) {
		BEnd(BENCH_PCP);
		return;
	}
	BASECLASS::Per_Cell_Process(why);
	BEnd(BENCH_PCP);
}


/// <summary>
/// Determines if this unit should be rendered, and marks it for redraw if so.
/// A vehicle still being disgorged by a building is held back until the building's door
/// says it may be seen, so that it does not appear before it has been let out.
/// </summary>
/// <param name="rect">The rectangle to record the unit's dirty area within.</param>
/// <param name="forced">Is this redraw forced by outside circumstances?</param>
/// <returns>bool; Was the unit rendered?</returns>
bool UnitClass::Render(Rect & rect, bool forced, bool extras_only) const
{
	if (IsTethered) {
		TechnoClass * radio = Contact_With_Whom();
		if (radio->RTTI == RTTI_BUILDING) {
			if (radio->Mission == MISSION_UNLOAD || radio->MissionQueue == MISSION_UNLOAD) {
				if (!radio->Door.Is_Ready_To_Close()) {
					return(false);
				}
			}
		}
	}
	/*
	 * Units have no extras pass (only buildings do), so always render the body.
	 */
	return(BASECLASS::Render(rect, forced, false));
}


/// <summary>
/// Transfers the composed unit image to the destination surface.
/// This routine is the last step of drawing a turreted vehicle. The image built up on the
/// scratch surface is blitted out with whatever cloaking or translucency effect the unit's
/// visual character calls for. A vehicle too tall to fit under a bridge goes across in two
/// pieces, so that its upper half sorts above the bridge deck.
/// </summary>
/// <param name="surface">The surface to blit the composed unit onto.</param>
/// <param name="drawpoint">The pixel position to center the unit upon.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
/// <param name="brightness">The lighting level to blit the unit at.</param>
void UnitClass::Unit_Blit_Voxel(Surface & surface, Point2D drawpoint, Rect cliprect, int brightness) const
{
	bool fudge = false;
	Point2D pt;
	pt.Y = (2 * (80 - UnitCompositeDirtyRect.Y) - UnitCompositeDirtyRect.Height) / 2;
	pt.X = (2 * (80 - UnitCompositeDirtyRect.X) - UnitCompositeDirtyRect.Width) / 2;

	if (Class->IsTooBigToFitUnderBridge) {
		if (Is_Z_Fudge_Bridge() && Get_Z_Fudge_Column() == 0) {
			fudge = true;
		} else {
			RadioClass * radio = Contact_With_Whom();
			if (NavCom != NULL) {
				if (radio != NULL && radio->RTTI == RTTI_BUILDING && ((BuildingClass *)radio)->Class->IsWeaponsFactory) {
					fudge = true;
				}
			}
		}
	}

	Blitter const * blitter = NULL;
	int predoffset = 0;

	switch (Visual_Character()) {

	case VISUAL_INDISTINCT:
		blitter = ColorSchemes[House->Scheme]->Converter->Blitter_From_Flags(ShapeFlags_Type(SHAPE_ALPHA | SHAPE_TRANSLUCENT25 | SHAPE_ZGRAD));
		break;

	case VISUAL_DARKEN:
		blitter = ColorSchemes[House->Scheme]->Converter->Blitter_From_Flags(ShapeFlags_Type(SHAPE_ALPHA | SHAPE_TRANSLUCENT50 | SHAPE_ZGRAD));
		break;

	case VISUAL_SHADOWY:
		blitter = ColorSchemes[House->Scheme]->Converter->Blitter_From_Flags(ShapeFlags_Type(SHAPE_ALPHA | SHAPE_TRANSLUCENT50 | SHAPE_ZGRAD));
		break;

	case VISUAL_RIPPLE: {
		int stage = CloakingDevice.Fetch_Stage();
		if (stage == 0) {
			blitter = ColorSchemes[House->Scheme]->Converter->Blitter_From_Flags(ShapeFlags_Type(SHAPE_ALPHA | SHAPE_TRANSLUCENT25 | SHAPE_PREDATOR | SHAPE_ZGRAD));
		} else if (stage == 1) {
			blitter = ColorSchemes[House->Scheme]->Converter->Blitter_From_Flags(ShapeFlags_Type(SHAPE_ALPHA | SHAPE_TRANSLUCENT50 | SHAPE_PREDATOR | SHAPE_ZGRAD));
		} else {
			blitter = ColorSchemes[House->Scheme]->Converter->Blitter_From_Flags(ShapeFlags_Type(SHAPE_ALPHA | SHAPE_TRANSLUCENT50 | SHAPE_PREDATOR | SHAPE_ZGRAD));
		}

		predoffset = Get_Predator_Offset();
		break;
	}

	case VISUAL_HIDDEN:
		break;

	default:
		blitter = ColorSchemes[House->Scheme]->Converter->Blitter_From_Flags(ShapeFlags_Type(SHAPE_ALPHA | SHAPE_ZGRAD));
		break;
	}

	int width = UnitCompositeDirtyRect.Width;
	Rect rect(drawpoint.X - width / 2 - pt.X,
		drawpoint.Y - UnitCompositeDirtyRect.Height / 2 - pt.Y,
		width,
		UnitCompositeDirtyRect.Height);

	if (IsSinking && SinkingYOffset == 0) {
		((UnitClass *)this)->Calculate_Sinking_Offset(rect.Y + cliprect.Y, UnitCompositeDirtyRect.Height);
	}

	if (fudge) {
		Rect source_rect;
		rect.Height = 32;
		source_rect = UnitCompositeDirtyRect;
		source_rect.Height = 32;
		Bit_Blit(surface, cliprect, rect, *LogicalSurface, LogicalSurface->Get_Rect(), source_rect, *blitter, Get_Z_Adjust() - (UnitCompositeDirtyRect.Height - 32) / 3, ZGRAD_GROUND, brightness, predoffset);
		rect.Y += 32;
		rect.Height = UnitCompositeDirtyRect.Height - 32;
		source_rect = Rect(UnitCompositeDirtyRect.X, UnitCompositeDirtyRect.Y + 32, UnitCompositeDirtyRect.Width, UnitCompositeDirtyRect.Height);
		source_rect.Height -= 32;
		Bit_Blit(surface, cliprect, rect, *LogicalSurface, LogicalSurface->Get_Rect(), source_rect, *blitter, Get_Z_Adjust(), ZGRAD_90DEG, brightness, predoffset);
	} else {
		Bit_Blit(surface, cliprect, rect, *LogicalSurface, LogicalSurface->Get_Rect(), UnitCompositeDirtyRect, *blitter, Get_Z_Adjust(), Get_Z_Gradient(), brightness, predoffset);
	}


}


/// <summary>
/// Draws this unit from its voxel model.
/// This routine lays down the vehicle's shadow, its hull, and whatever turret and barrel
/// it carries. A turreted vehicle is composed on the scratch surface first so that the
/// pieces sort correctly against one another, then blitted to the screen in one piece.
/// </summary>
/// <param name="xdrawpoint">The pixel position to center the unit's model upon.</param>
/// <param name="xcliprect">The clipping rectangle to draw within.</param>
/// <param name="brightness">The lighting level to draw the unit at.</param>
void UnitClass::Unit_Draw_Voxel(Point2D xdrawpoint, Rect xcliprect, int brightness) const
{
	VoxelDataStruct *voxl;
	Point2D drawpoint = xdrawpoint;
	Point2D offset(0, 0);
	Matrix3D main_matrix;

	int frame = TotalFramesWalked / 2 % Class->Voxel.MotLib->Get_Frame_Count();

	switch (frame) {
		case 0:
		case 7:
		case 15:
			break;

		case 1:
		case 6:
		case 8:
		case 14:
			offset = Point2D(0, -1);
			break;

		default:
			offset = Point2D(0, -2);
			break;
	}

	Rect cliprect = xcliprect;
	int has_turret = false;
	ShapeFlags_Type flags = SHAPE_NORMAL;

	if (Class->IsTurretEquipped && Class->AuxVoxel.VoxLib != NULL || Class->AuxVoxel2.VoxLib != NULL &&  Class->AuxVoxel2.MotLib != NULL) {
		has_turret = true;
	}

	int key = 0;
	main_matrix = Locomotion->Shadow_Matrix(&key);

	if (!IsSinking) {
		Point2D shadow_drawpoint = xdrawpoint;
		if (Class->IsHunterSeeker) {
			Coord coord = PositionCoord;
			coord.Z = Map.Get_Height_GL(coord);
			TacticalMap->Coord_To_Pixel(coord, shadow_drawpoint);
			shadow_drawpoint += Locomotion->Shadow_Point();
		}

		if (IsInTransport) {
			shadow_drawpoint.Y -= 14;
		}

		int height = HeightAGL;
		CellClass * cellptr = &Map[Get_Coord()];

		if (IsInTransport) {
			if (!IsOnBridge && height >= BRIDGE_LEPTON_HEIGHT) {
				if (cellptr->IsUnderBridge && (cellptr->IsBridgeEastWest && cellptr->Adjacent_Cell(FACING_N).IsUnderBridge ||
					!cellptr->IsBridgeEastWest && cellptr->Adjacent_Cell(FACING_W).IsUnderBridge)) {
					shadow_drawpoint.Y -= 4 * LEVEL_PIXEL_H;
				}
			}
		}

		Draw_Voxel_Shadow(Class->Voxel, Class->ShadowIndex, key, &Class->ShadowVoxelIndex, xcliprect, shadow_drawpoint, Get_Isometric_View_Matrix() * main_matrix, true);
	}

	Surface * old_surface = LogicalSurface;
	if (has_turret) {
		LogicalSurface = EightBitSurface;
		drawpoint = Point2D(80, 80);
		flags = ShapeFlags_Type(SHAPE_ALPHA|SHAPE_ZGRAD);
		cliprect = LogicalSurface->Get_Rect();
		IsCompositingToEightBitSurface = true;
	}

	if (UseVoxelCache) {
		key = 0;
	} else {
		key = -1;
	}

	main_matrix = Locomotion->Draw_Matrix(&key);

	if (key != -1) {
		key = frame | (key << 5);
	}

	Rect rect = cliprect;
	if (!has_turret && SinkingYOffset > 0) {
		rect = Intersect(rect, Rect(0, 0, TacticalRect.Width, SinkingYOffset - TacticalMap->TacPixelY));
	}

	UnitCompositeDirtyRect = RECT_NONE;

	if (strcmp(Class->IniName, "APC") == 0 && Map[Get_Coord()].Land_Type() == LAND_WATER && !IsOnBridge && HeightAGL < LEVEL_LEPTON_H) {
		Draw_Voxel(Class->AuxVoxel, frame, -1, NULL, rect, drawpoint, Get_Isometric_View_Matrix() * main_matrix, brightness, flags);
	} else {
		Draw_Voxel(Class->Voxel, frame, key, &Class->VoxelIndex, rect, drawpoint, Get_Isometric_View_Matrix() * main_matrix, brightness, flags);
	}

	/*
	**	If there is a turret, then it must be rendered as well. This may include
	**	firing animation if required.
	*/
	if (Class->IsTurretEquipped && Class->AuxVoxel.VoxLib != NULL) {
		main_matrix.Translate_X(Class->TurretOffset / 8);
		main_matrix.Rotate_Z(SecondaryFacing.Current().As_Radian32() - PrimaryFacing.Current().As_Radian32());

		/*
		**	A recoiling turret moves "backward" one pixel.
		*/
		if (IsInRecoilState) {
			main_matrix.Translate_X(-2);
		}

		Matrix3D barrel_matrix = main_matrix;
		Vector3 vec2 = main_matrix.Get_Translation();
		barrel_matrix.Translate(-vec2);

		Vector3 flh = Vector3(-Get_Class_Weapon_Data(0)->FireFLH.X / 8, 0, -Get_Class_Weapon_Data(0)->FireFLH.Z / 8);
		barrel_matrix.Translate(-flh);
		barrel_matrix.Rotate_Y(-BarrelPitch.Current().As_Radian32());
		barrel_matrix.Translate(flh);
		barrel_matrix.Translate(vec2);

		bool draw_barrel;
		if (SecondaryFacing.Current().As_Dir4() <= 0 || SecondaryFacing.Current().As_Dir4() >= 3) {
			draw_barrel = false;
			voxl = &Class->AuxVoxel2;
			if (voxl->VoxLib != NULL && voxl->MotLib != NULL) {
				Draw_Voxel(Class->AuxVoxel2, 0, -1, &Class->VoxelIndex, cliprect, drawpoint + offset, Get_Isometric_View_Matrix() * barrel_matrix, brightness, flags);
			}
		} else {
			draw_barrel = true;
		}

		Draw_Voxel(Class->AuxVoxel, frame, -1, &Class->AuxVoxelIndex, cliprect, drawpoint + offset, Get_Isometric_View_Matrix() * main_matrix, brightness, flags);

		if (draw_barrel) {
			voxl = &Class->AuxVoxel2;
			if (voxl->VoxLib != NULL && voxl->MotLib != NULL) {
				Draw_Voxel(Class->AuxVoxel2, 0, -1, &Class->VoxelIndex, cliprect, drawpoint + offset, Get_Isometric_View_Matrix() * barrel_matrix, brightness, flags);
			}
		}
	} else {
		voxl = &Class->AuxVoxel2;
		if (voxl->VoxLib != NULL && voxl->MotLib != NULL) {
			main_matrix.Rotate_Y(-(BarrelPitch.Current().As_Radian32() - main_matrix.Get_Y_Rotation()));
			main_matrix.Translate(Get_Class_Weapon_Data(0)->FireFLH.X / 8, 0, Get_Class_Weapon_Data(0)->FireFLH.Z / 8);
			Draw_Voxel(Class->AuxVoxel2, frame, -1, &Class->VoxelIndex, cliprect, drawpoint, Get_Isometric_View_Matrix() * main_matrix, brightness, SHAPE_NORMAL);
		}
	}

	if (has_turret) {
		rect = xcliprect;
		if (SinkingYOffset > 0) {
			rect = Intersect(rect, Rect(0, 0, TacticalRect.Width, SinkingYOffset - TacticalMap->TacPixelY));
		}
		Unit_Blit_Voxel(*old_surface, xdrawpoint, rect, brightness);
		LogicalSurface->Fill_Rect(UnitCompositeDirtyRect, TBLACK);
		LogicalSurface = old_surface;
		IsCompositingToEightBitSurface = false;
	}

}


/// <summary>
/// Draws this unit from its shape artwork.
/// This routine handles the unit types that are drawn from shapes rather than voxels --
/// visceroids, jellyfish, limpet drones and the sprite based vehicles. A turreted vehicle
/// is composed on the scratch surface first so that its turret and voxel barrel can be
/// layered in the right order, then blitted to the destination in one piece.
/// </summary>
/// <param name="xdrawpoint">The pixel position to center the unit's artwork upon.</param>
/// <param name="xcliprect">The clipping rectangle to draw within.</param>
/// <param name="brightness">The lighting level to draw the unit at.</param>
void UnitClass::Unit_Draw_Shape(Point2D xdrawpoint, Rect xcliprect, int brightness) const
{
	VoxelDataStruct * voxl;
	int shapenum;                // Working shape number.
	ShapeSet const * shapefile;  // Working shape file pointer.

	/*
	**	Verify the legality of the unit class.
	*/
	shapefile = (ShapeSet const *)Get_Image_Data();
	bool barrel_above_turret = true;

	if (Class->IsSmallVisceroid || Class->IsLargeVisceroid) {
		if (VisceroidsAsSnoBees) {
			if (Class->IsSmallVisceroid) {
				shapefile = (ShapeSet const *)UnitTypeClass::SmallVisceroidShapes;
			} else {
				shapefile = (ShapeSet const *)UnitTypeClass::LargeVisceroidShapes;
			}
			shapenum = PrimaryFacing.Current().As_Dir8();
			switch (shapenum) {
				case FACING_N:
				case FACING_NE:
					shapenum = 2;
					break;
				case FACING_SE:
					shapenum = 4;
					break;
				case FACING_S:
				case FACING_SW:
					shapenum = 6;
					break;
				case FACING_NW:
					shapenum = 0;
					break;
				default:
					break;
			}
			shapenum += (((unsigned int)(Frame + (Fetch_ID() >> 1)) >> 3) & 1);
		} else {
			shapenum = Fetch_Stage();
			if (shapenum >= 90) {
				shapenum -= 90;
				shapefile = (ShapeSet const *)Class->AltImageData;
			}
		}
		Draw_Object(shapefile, shapenum, xdrawpoint, xcliprect, DIR_N, 256, 0, ZGRAD_90DEG, false, brightness);
		return;
	}

	if (Class->IsJellyfish || Class->IsLimpetDrone) {
		int zoff = 0;
		if (Class->IsJellyfish) {
			zoff = -TacticalMap->Z_Lepton_To_Pixel(PositionCoord.Z);
		}
		Draw_Object(shapefile, Fetch_Stage(), xdrawpoint, xcliprect, DIR_N, 256, zoff, ZGRAD_90DEG, false, brightness);
		return;
	}

	if (Class->Facings == FACING_COUNT) {
		shapenum = Facing_Add(PrimaryFacing.Current().Round_To_8(), FACING_45);
	} else {
		shapenum = 0;
	}

	if (Locomotion->Is_Moving()) {
		shapenum = Class->StartWalkFrame + shapenum * Class->WalkFrames + TotalFramesWalked % Class->WalkFrames;
	} else {
		if (FiringSyncDelay >= 0) {
			shapenum = Class->StartFiringFrame + FiringSyncDelay / 2 + shapenum * Class->FiringFrames;
		} else {
			if (DeathCounter >= 0) {
				int deathframe = DeathCounter / Class->DeathFrameRate;
				int StartDeathFrame = Class->StartDeathFrame;
				int lastframe = Class->DeathFrames - 1;
				if (deathframe >= lastframe) {
					deathframe = lastframe;
				}
				shapenum = deathframe + StartDeathFrame;
			} else if (IsOccupyingCell) {
				if (Class->StandingFrames == 0) {
					shapenum = Class->StartWalkFrame + shapenum * Class->WalkFrames;
				} else {
					shapenum = Class->StartStandFrame + shapenum * Class->StandingFrames;
				}
			}
		}
	}

	/*
	**	If there is a turret, then it must be rendered as well. This may include
	**	firing animation if required.
	*/
	if (Class->IsTurretEquipped) {

		Point2D drawpoint = xdrawpoint;
		if (IsInTransport) {
			drawpoint.Y -= 14;
		}

		/*
		**	Actually perform the draw. Overlay an optional shimmer effect as necessary.
		*/
		Draw_Shape(*LogicalSurface, *NormalDrawer, shapefile, shapenum + shapefile->Get_Count() / 2, drawpoint, xcliprect, ShapeFlags_Type(SHAPE_DARKEN | SHAPE_CENTER | SHAPE_WIN_REL | SHAPE_ALPHA | SHAPE_ZGRAD), NULL, Get_Z_Adjust() - 2);

		Surface * old_surface = LogicalSurface;
		LogicalSurface = EightBitSurface;
		Point2D pt = Point2D(80,80);

		Matrix3D nmtx;
		Rect srect = LogicalSurface->Get_Rect();
		IsCompositingToEightBitSurface = true;
		UnitCompositeDirtyRect = RECT_NONE;

		voxl = &Class->AuxVoxel2;
		if (voxl->VoxLib != NULL && voxl->MotLib != NULL) {
			int key = -1;
			Matrix3D mtx = Locomotion->Draw_Matrix(&key);
			mtx.Translate_X(Class->TurretOffset / 8);
			double sec = SecondaryFacing.Current().As_Radian32();
			double pri = PrimaryFacing.Current().As_Radian32();
			mtx.Rotate_Z(sec - pri);

			/*
			 * A recoiling turret moves "backward".
			 */
			if (IsInRecoilState) {
				mtx.Translate_X(-2);
			}

			nmtx = mtx;

			Vector3 trans = mtx.Get_Translation();
			nmtx.Translate(-trans);

			Vector3 flh = Vector3(Get_Class_Weapon_Data()->FireFLH.X / -8, 0, Get_Class_Weapon_Data()->FireFLH.Z / -8);
			nmtx.Translate(-flh);

			FacingClass face = BarrelPitch;
			if (PrimaryWeapon->Bullet->IsInvisible) {
				Dir256 dir256 = face.Current().As_Dir256();
				if (dir256 > DIR_E && dir256 <= DIR_S) {
					dir256 = (Dir256)((dir256 - DIR_E) / 3 + DIR_E);
				} else {
					dir256 = (Dir256)((DIR_E - dir256) / 3 + DIR_E);
				}
				face.Set(dir256);
			}

			DirType facing = face.Current();
			nmtx.Rotate_Y(-facing.As_Radian32());
			nmtx.Translate(flh);
			nmtx.Translate(trans);
			if (SecondaryFacing.Current().As_Dir4() > 0) {
				DirType dir = SecondaryFacing.Current();
				barrel_above_turret = true;
				if (dir.As_Dir4() >= 3) {
					barrel_above_turret = false;
				}
			} else {
				barrel_above_turret = false;
			}
		}

		/// Draw the main shape
		Draw_Object(shapefile, shapenum, pt, srect, DIR_N, 256, 0, ZGRAD_GROUND, 0, brightness, NULL, 0, Point2D(0, 0), ShapeFlags_Type(SHAPE_ALPHA|SHAPE_ZGRAD));

		/*
		 * The the voxel barrel below the turret at certain angles
		 */
		voxl = &Class->AuxVoxel2;
		if (voxl->VoxLib != NULL && voxl->MotLib != NULL && !barrel_above_turret) {
			Draw_Voxel(Class->AuxVoxel2, 0, -1, 0, srect, pt, Get_Isometric_View_Matrix() * nmtx, brightness, ShapeFlags_Type(SHAPE_ZGRAD|SHAPE_ALPHA));
		}

		Dir32 d = SecondaryFacing.Current().As_Dir32();
		Draw_Object(shapefile, ((d + 4) % 32U) + 8 * Class->WalkFrames, pt, srect, DIR_N, 256, 0, ZGRAD_GROUND, false, brightness, NULL, 0, Point2D(0, 0), ShapeFlags_Type(SHAPE_NOTRANS|SHAPE_ALPHA|SHAPE_ZGRAD));

		/*
		 * The the voxel barrel above the turret at other angles
		 */
		voxl = &Class->AuxVoxel2;
		if (voxl->VoxLib != NULL && voxl->MotLib != NULL && barrel_above_turret) {
			Draw_Voxel(Class->AuxVoxel2, 0, -1, 0, srect, pt, Get_Isometric_View_Matrix() * nmtx, brightness, ShapeFlags_Type(SHAPE_NOTRANS|SHAPE_ZGRAD|SHAPE_ALPHA));
		}

		int apparent_brightness = Apparent_Brightness(brightness);
		Unit_Blit_Voxel(*old_surface, xdrawpoint, xcliprect, apparent_brightness);
		LogicalSurface->Fill_Rect(UnitCompositeDirtyRect, 0);
		LogicalSurface = old_surface;
		IsCompositingToEightBitSurface = false;

	} else {
		static int _zadj = 0;
		Draw_Object(shapefile, shapenum, xdrawpoint, xcliprect, DIR_N, 256, _zadj, Get_Z_Gradient(), 0, brightness, NULL, 0, Point2D(0, 0), SHAPE_NORMAL);
	}
}


/***********************************************************************************************
 * UnitClass::Draw_It -- Draws a unit object.                                                  *
 *                                                                                             *
 *    This routine is the one that actually draws a unit object. It displays the unit          *
 *    according to its current state flags and centered at the location specified.             *
 *                                                                                             *
 * INPUT:   x,y   -- The X and Y coordinate of where to draw the unit.                         *
 *                                                                                             *
 *          window   -- The clipping window to use.                                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/20/1994 JLB : Created.                                                                 *
 *   06/27/1994 JLB : Takes a window parameter.                                                *
 *   08/15/1994 JLB : Removed infantry support.                                                *
 *   01/07/1995 JLB : Harvester animation support.                                             *
 *   07/08/1995 JLB : Uses general purpose draw routine.                                       *
 *=============================================================================================*/
void UnitClass::Draw_It(Point2D const & point, Rect const & cliprect) const
{
	static ShapeSet const * harvesting_shape = (ShapeSet const *)MFCD::Retrieve("HARVESTR.SHP");

	Point2D adjusted_point = point;
	adjusted_point.Y -= IonBlastYDrawOffset;
	Point2D raw_point = adjusted_point;
	if (IsInTransport) {
		adjusted_point.Y += 14;
	}

	Cell cell = PositionCell;

	/*
	**	If drawing of this unit is not explicitly prohibited, then proceed
	**	with the render process.
	*/
	if (Visual_Character(false, NULL) != VISUAL_HIDDEN) {

		TacticalMap->Add_To_Selectables((ObjectClass *)this, adjusted_point);
		int height = HeightAGL;
		CellClass * cptr = &Map[Get_Coord()];

		int brightness = 0;
			if (((!IsOnBridge && height >= BRIDGE_LEPTON_HEIGHT) || (IsOnBridge && height >= 0)) &&
				(cptr->IsUnderBridge &&
					(cptr->IsBridgeEastWest && cptr->Adjacent_Cell(FACING_N).IsUnderBridge ||
					!cptr->IsBridgeEastWest && cptr->Adjacent_Cell(FACING_W).IsUnderBridge))) {
			brightness = Map[cell].Brightness + (4 * (IonStormClass::Is_Ion_Storm_Active() ? Scen->IonLevelLight : Scen->LevelLight));
		} else {
			brightness = Map[cell].Brightness + (Map[cell].IsOvershadowed ? -500 : 0);
		}
		brightness += Rule->ExtraUnitLight;

		if (Class->IsToHarvest && IsHarvesting && !Locomotion->Is_Moving_Now()) {
			Point2D pixel;
			FacingType face = (FacingType)PrimaryFacing.Current().As_Dir8();
			TacticalMap->Coord_To_Pixel(Move_Coord(Render_Coord(), face, 150), pixel);
			Draw_Shape(*LogicalSurface, *AnimDrawer, harvesting_shape, (Frame + TotalFramesWalked) % 15, pixel, cliprect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_ALPHA|SHAPE_ZGRAD), NULL, Get_Z_Adjust() - 2, ZGRAD_GROUND, brightness);
		}

		UnitTypeClass * oldclass = Class;
		if (Class->IsToHarvest && IsDumping) {
			if (Rule->UnloadingHarvester != NULL) {
				((UnitClass *)this)->Class = (UnitTypeClass *)Rule->UnloadingHarvester;
			}
		}

		/*
		**	Actually perform the draw. Overlay an optional shimmer effect as necessary.
		*/
		if (CurrentTube == TUBE_NONE) {
			if (Class->IsVoxel) {
				if (Class->Voxel.VoxLib != NULL) {
					Unit_Draw_Voxel(adjusted_point, cliprect, brightness);
				}
			} else {
				Unit_Draw_Shape(adjusted_point, cliprect, brightness);
			}
		}

		((UnitClass *)this)->Class = oldclass;

		/*
		**	If this unit is carrying the flag, then draw that on top of everything else.
		*/
		if (Flagged != HOUSE_NONE) {
			Draw_Shape(*LogicalSurface, *ColorSchemes[Houses[Flagged]->Scheme]->Converter, (ShapeSet const *)MFCD::Retrieve("FLAGFLY.SHP"), Frame % 14, raw_point, cliprect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_ALPHA), NULL, 0, ZGRAD_GROUND, brightness);
		}

		BASECLASS::Draw_It(raw_point, cliprect);

		if (TargetingLaserTimer != 0) {
			Draw_Target_Laser();
		}
	}
}


/***********************************************************************************************
 * UnitClass::Harvesting -- Harvests tiberium at the current location.                         *
 *                                                                                             *
 *    This routine is used to by the harvester to harvest Tiberium at the current location.    *
 *    When harvesting is complete, this routine will return true.                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is harvesting complete?                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool UnitClass::Harvesting(void)
{
	CellClass * ptr = &Map[Get_Coord()];

	/*
	**	Keep waiting if still heading toward a spot to harvest.
	*/
	if (NavCom != NULL) return(true);

	bool harvest;
	if (Class->IsToHarvest) {
		harvest = (Tiberium_Load() < 1 && ptr->Land_Type() == LAND_TIBERIUM);
	} else {
		harvest = (Weed_Load() < 1 && ptr->Land_Type() == LAND_WEEDS && ptr->OverlayData >= OVERLAYDATA_FIRST_SOLID_VEIN);
	}

	if (harvest) {
		if (Class->IsToVeinHarvest) {

			if (Class->IsToVeinHarvest) {
				ptr->Reduce_Weed();
				Storage.Increase_Amount(1, 0);
				if (Weed_Load() < 1) {
					Storage.Increase_Amount(1, 0);
				}
				Set_Stage(0);
				Set_Rate(Rule->HarvesterLoadRate * 3);
			}

		} else {

			/*
			**	Lift some Tiberium from the ground. Try to lift a complete
			**	"level" of Tiberium. A level happens to be 6 steps. If there
			**	is a partial level, then lift that instead. Never lift more
			**	than the harvester can carry.
			*/
			TiberiumType tib = ptr->Tiberium_Type_Here();
			int reducer = std::min(1, Class->Capacity - Storage.Get_Total_Amount());
			reducer = ptr->Reduce_Tiberium(reducer);
			if (reducer <= 0) {
				return(false);
			}
			Storage.Increase_Amount(reducer, tib);
			Set_Stage(0);
			Set_Rate(Rule->HarvesterLoadRate);
		}

	} else {

		/*
		**	If the harvester is stopped on a non Tiberium field and the harvester
		**	isn't loaded with Tiberium, then no further action can be performed
		**	by this logic routine. Bail with a failure and thus cause a branch to
		**	a better suited logic processor.
		*/
		Set_Stage(0);
		Set_Rate(0);
		return(false);
	}

	return(true);
}


/***********************************************************************************************
 * UnitClass::Mission_Unload -- Handles unloading cargo.                                       *
 *                                                                                             *
 *    This is the AI control sequence for when a transport desires to unload its cargo and     *
 *    then exit the map.                                                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the delay before calling this routine again.                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int UnitClass::Do_MISSION_UNLOAD(void)
{
	enum {
		INITIAL_CHECK,
		MANEUVERING,
		OPENING_DOOR,
		UNLOADING,
		CLOSING_DOOR
	};

	FacingType	dir;
	Cell		cell;
	BuildingClass * building;

	if (Class->Max_Passengers() > 0) {
		/// derives from RA's UNIT_TRUCK
		switch (Status) {
			case INITIAL_CHECK:
				if (Locomotion->Is_Moving()) {
					return(10);
				}
				dir = Desired_Load_Dir(NULL, cell);
				if (Cargo.How_Many() && cell != CELL_NONE) {
					Locomotion->Do_Turn(DirType(dir));
					Status = MANEUVERING;
					return(1);
				} else {
					Assign_Mission(MISSION_GUARD);
				}
				break;

			case MANEUVERING:
				if (!IsRotating) {
					Status = UNLOADING;
					return(1);
				}
				break;

			case UNLOADING:
				if (Cargo.How_Many()) {
					FootClass * passenger = Cargo.Detach_Object();

					if (passenger != NULL) {
						/// The 8 grid wrap is deferred until each facing is tested.
						FacingType toface = (FacingType)(DirType((DirType(DIR_S)-DirType(DIR_STEP_256)).As_Int()) + PrimaryFacing.Current()).Round_To_8();
						FacingType nextface = toface;
						bool placed = false;
						Cell newcell;

						for (FacingType face = FACING_N; face < FACING_COUNT; face++, nextface++) {
							FacingType newface = Facing_Add(nextface, FACING_0);
							newcell = Adjacent_Cell(PositionCell, newface);

							if (passenger->Can_Enter_Cell(&Map[newcell], newface, Get_Cell_Height()) == MOVE_OK) {
								ScenarioInit++;
								Coord coord = newcell.As_Coord();
								coord = Map.Closest_Free_Spot(coord);
								if (Map[newcell].IsUnderBridge == false) {
									placed = passenger->Unlimbo(coord, DirType(newface).As_Dir256());
									ScenarioInit--;
									//placed = true;
									break;
								}
							}
						}


						if (placed) {
							passenger->Assign_Mission(MISSION_MOVE);
							passenger->Assign_Destination(&Map[newcell]);
							if (Team != NULL) {
								Team->Add(passenger);
							}
						} else {
							/*
							**	If the attached unit could NOT be deployed, then re-attach
							**	it and then bail out of this deploy process.
							*/
							Cargo.Attach(passenger);
							passenger->Hidden();
							Status = CLOSING_DOOR;
						}
					}
				} else {
					Status = CLOSING_DOOR;
				}
				break;

			/*
			**	Close APC door in preparation for normal operation.
			*/
			case CLOSING_DOOR:
				Assign_Mission(MISSION_GUARD);
				IsMissionUnloadStandby = true;
				break;
		}
		return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
	} else if (Class->IsToHarvest || Class->IsToVeinHarvest) {
		/// derives from RA's UNIT_HARVESTER
		if (!In_Radio_Contact()) {
			Enter_Idle_Mode();
			IsDumping = false;
			if (Locomotion->Is_Moving()) {
				Stop_Driver();
			}
			if (Ready_To_Commence()) {
				Commence();
			}
			return(1);
		}

		if (PrimaryFacing.Current().As_Dir256() != DIR_E) {
			if (!IsRotating) {
				Locomotion->Do_Turn(DirType(DIR_E));
			}
			return(5);
		}

		if (!IsDumping) {
			IsDumping = true;
			Set_Stage(0);
			Set_Rate(1);

			if (Class->IsToHarvest) {
				building = Map[Adjacent_Cell(PositionCell, FACING_W)].Cell_Building();
				if (building != NULL) {
					building->Begin_Anim(BANIM_PRE_PRODUCTION, false);
				}
			}

			Status = 3;
		} else {
			switch (Status) {
				case 4:
					if (Class->IsToVeinHarvest) {
						IsDumping = false;
						if (NavCom != NULL && MissionQueue != MISSION_NONE && MissionQueue != MISSION_HARVEST) {
							if (Locomotion->Is_Moving()) {
								Stop_Driver();
							}
							if (Ready_To_Commence()) {
								Commence();
							}
						} else {
							Assign_Mission(MISSION_HARVEST);
							if (Ready_To_Commence()) {
								if (In_Radio_Contact()) {
									Transmit_Message(RADIO_OVER_OUT);
								}
								Commence();
							}
						}
					} else {
						building = Map[Adjacent_Cell(PositionCell, FACING_W)].Cell_Building();
						bool active = false;
						if (building != NULL && building->Class->IsRefinery) {
							active = building->Anim_Active(BANIM_PRODUCTION);
						}
						if (!active) {
							IsDumping = false;
							if (NavCom != NULL && MissionQueue != MISSION_NONE && MissionQueue != MISSION_HARVEST) {
								if (Locomotion->Is_Moving()) {
									Stop_Driver();
								}
								if (Ready_To_Commence()) {
									Commence();
								}
							} else {
								Assign_Mission(MISSION_HARVEST);
								if (Ready_To_Commence()) {
									if (In_Radio_Contact()) {
										Transmit_Message(RADIO_OVER_OUT);
									}
									Commence();
								}
							}
						} else {
							return(1);
						}
					}
					break;

				case 3:
					building = Map[Adjacent_Cell(PositionCell, FACING_W)].Cell_Building();
					if (Fetch_Stage() >= Rule->HarvesterDumpRate * TICKS_PER_MINUTE) {
						bool dumped = false;
						int slot = Storage.First_Used_Slot();
						if (slot != -1) {
							int amount = Storage.Decrease_Amount(1, slot);
							if (amount > 0) {
								dumped = true;
								if (Class->IsToVeinHarvest) {
									House->Harvested_Weed(amount, slot);
								} else {
									House->Harvested(amount, (TiberiumType)slot);
								}
								Set_Stage(0);
							}
						}
						if (!dumped) {
							if (building != NULL && building->Class->IsRefinery) {
								building->Begin_Anim(BANIM_PRODUCTION, false);
							}
							Status = 4;
						}
					}
					if (NavCom != NULL) {
						if (MissionQueue != MISSION_NONE && MissionQueue != MISSION_HARVEST) {
							if (building != NULL && building->Class->IsRefinery) {
								building->Begin_Anim(BANIM_PRODUCTION, false);
							}
							Status = 4;
						}
					}
					return(1);

				default:
					break;
			}
		}
		return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
	} else if (Class->DeploysInto != NULL) {
		/// derives from RA's UNIT_MCV
		switch (Status) {
			case 0:
				Transmit_Message(RADIO_OVER_OUT);
				Path[0] = FACING_NONE;
				Status = 1;
				// fall through

			case 1:
				if (!Locomotion->Is_Moving()) {
					Try_To_Deploy();
					if (IsActive) {
						if (IsDeploying) {
							Status = 2;
						} else {
							if (!House->Is_Human_Player() && Session.Type != GAME_NORMAL) {
								Assign_Mission(MISSION_HUNT);
							} else {
								Assign_Mission(MISSION_GUARD);
							}
						}
					}
				} else if (MissionQueue != MISSION_NONE && MissionQueue != MISSION_UNLOAD) {
					Commence();
				}
				break;

			case 2:
				if (!IsDeploying) {
					Enter_Idle_Mode();
					Commence();
				} else if (!Try_To_Deploy()) {
					if (NavCom != NULL) {
						IsDeploying = false;
					}
				}
				break;
		}
		//return(1);
		return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
	} else {
		if (Class->IsMobileEMP && !Is_Immobilized()) {
			EMPulse_Blast();
			Assign_Mission(MISSION_GUARD);
		}
	}

	return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
}


/***********************************************************************************************
 * UnitClass::Mission_Harvest -- Handles the harvesting process used by harvesters.            *
 *                                                                                             *
 *    This is the AI process used by harvesters when they are doing their harvesting action.   *
 *    This entails searching for nearby Tiberium, heading there, harvesting, and then          *
 *    returning to a refinery for unloading.                                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the delay before calling this routine again.                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1994 JLB : Created.                                                                 *
 *   06/21/1995 JLB : Force guard mode if no Tiberium found.                                   *
 *   09/28/1995 JLB : Aborts harvesting if there are no more refineries.                       *
 *=============================================================================================*/
int UnitClass::Do_MISSION_HARVEST(void)
{
	int i;

	enum {
		LOOKING,
		HARVESTING,
		FINDHOME,
		HEADINGHOME,
		GOINGTOIDLE,
	};

	/*
	**	A non-harvesting type unit will just sit still if it is given the harvest mission. This
	**	allows combat units to act "brain dead".
	*/
	if (!Class->IsToHarvest && !Class->IsToVeinHarvest) return(TICKS_PER_SECOND*30);

	if (Class->Dock.Count() == 0 && !House->Is_Human_Player()) {
		Assign_Mission(MISSION_GUARD);
		return(1);
	}

	/*
	**	If there are no more refineries, then drop into guard mode.
	*/
	for (i = 0; i < Class->Dock.Count(); i++) {

			if (House->BQuantity.Value(Class->Dock[i]->HeapID) <= 0) {
				continue;
			}

	switch (Status) {

		/*
		**	Go and find a Tiberium field to harvest.
		*/
		case LOOKING:
			if (Class->IsToVeinHarvest) {
				if (Weed_Load() == 1) {
					Status = FINDHOME;
					return(1);
				}
			}

			/*
			**	When full of tiberium, just skip to finding a free refinery
			**	to unload at.
			*/
			else if (Tiberium_Load() >= 1) {
				Status = FINDHOME;
				return(1);
			}

			{

				/*
				**	Look for ore where we last found some - mine the same patch
				*/
				bool hastarget = true;
				if (ArchiveTarget != NULL) {
					Assign_Destination(ArchiveTarget);
					ArchiveTarget = 0;
					hastarget = false;
				}
				IsHarvesting = false;
				bool ok = false;
				if (Class->IsToVeinHarvest) {
					ok = Goto_Weed(Rule->TiberiumLongScan / CELL_LEPTON_W);
				} else {
					ok = Goto_Tiberium(Rule->TiberiumLongScan / CELL_LEPTON_W, hastarget);
				}
				if (ok) {
					IsHarvesting = true;
					Set_Rate(2);
					Set_Stage(0);
					Status = HARVESTING;
					return(1);
				} else {

					/*
					**	If the harvester isn't on Tiberium and it is not heading toward Tiberium, then
					**	force it to go into guard mode. This will prevent the harvester from repeatedly
					**	searching for Tiberium.
					*/
					if (NavCom == NULL) {

						/*
						**	If the archive target is legal, then head there since it is presumed
						**	that the archive target points to the last place it harvested at. This might
						**	solve the case where the harvester gets stuck and can't find Tiberium just because
						**	it is greater than 32 squares away.
						*/
						if (ArchiveTarget != NULL) {
							Assign_Destination(ArchiveTarget);
						} else {
							Status = GOINGTOIDLE;
							IsUseless = true;
							if (Class->IsToHarvest) {
								House->IsTiberiumShort = true;
							}
							return(TICKS_PER_SECOND*7);
						}
					} else {
						IsUseless = false;
					}
				}
			}
			break;

		/*
		**	Harvest at current location until full or Tiberium exhausted.
		*/
		case HARVESTING:
			if (Fetch_Rate() == 0) {
				Set_Stage(0);
				Set_Rate(Rule->HarvesterLoadRate);
			}

			if (Fetch_Stage() < ARRAY_SIZE(Harvester_Load_List)+1) return(1);
			if (!Harvesting()) {
				IsHarvesting = false;
				if ((Class->IsToHarvest && Tiberium_Load() == 1) || (Class->IsToVeinHarvest && Weed_Load() == 1)) {
					Status = FINDHOME;
					Cell cell;
					if (Class->IsToHarvest) {
						cell = Search_For_Tiberium(Rule->TiberiumShortScan / CELL_LEPTON_W);
					} else {
						cell = Search_For_Weed(Rule->TiberiumShortScan / CELL_LEPTON_W);
					}
					if (cell != CELL_NONE) {
						ArchiveTarget = &Map[cell];
					} else {
						ArchiveTarget = NULL;
					}
				} else {
					bool ok;
					if (Class->IsToHarvest) {
						ok = Goto_Tiberium(Rule->TiberiumShortScan / CELL_LEPTON_W);
					} else {
						ok = Goto_Weed(Rule->TiberiumShortScan / CELL_LEPTON_W);
					}
					if (!ok && NavCom == NULL)	{
						ArchiveTarget = NULL;
						Status = FINDHOME;
					} else {
						Status = HARVESTING;
						IsHarvesting = true;
					}
				}
				return(1);
			}
			return(1);
//			return(TICKS_PER_SECOND*Rule->OreDumpRate);

		/*
		**	Find and head to refinery.
		*/
		case FINDHOME:
			if (NavCom == NULL) {

				/*
				**	Find nearby refinery and head to it?
				*/
				BuildingClass * nearest = NULL;

				for (i = 0; i < Class->Dock.Count(); i++) {
					nearest = Find_Docking_Bay(Class->Dock[i], false);
					if (nearest != NULL) {
						break;
					}
				}

				/*
				**	Since the refinery said it was ok to load, establish radio
				**	contact with the refinery and then await docking orders.
				*/
				if (nearest != NULL && Transmit_Message(RADIO_HELLO, nearest) == RADIO_ROGER) {
					Status = HEADINGHOME;
///					if (nearest->House == PlayerPtr && (PlayerPtr->Capacity - PlayerPtr->Tiberium) < 300 && PlayerPtr->Capacity > 500 && (PlayerPtr->ActiveBScan & (STRUCTF_REFINERY | STRUCTF_CONST))) {
///						Speak(VOX_NEED_MO_CAPACITY);
///					}
				} else {
					ScenarioInit++;
					nearest = NULL;
					for (i = 0; i < Class->Dock.Count(); i++) {
						nearest = Find_Docking_Bay(Class->Dock[i], false);
						if (nearest != NULL) {
							break;
						}
					}
					ScenarioInit--;
					if (nearest != NULL) {
						if (Distance_To(nearest) > 3 * CELL_LEPTON) {
							Cell cell = Cell(nearest->Get_Coord());
							Cell nearby = Map.Nearby_Location(Cell(nearest->Get_Coord()), SPEED_WHEEL, Map.Get_Cell_Zone(cell, Class->MZone), Class->MZone, false, Point2D(1, 1), false, true, false, false);
							if (nearby != CELL_NONE) {
								Assign_Destination(&Map[nearby]);
							} else {
								Assign_Destination(NULL);
							}
						}
					}
				}
			}
			break;

		/*
		**	In communication with refinery so that it will successfully dock and
		**	unload. If, for some reason, radio contact was lost, then hunt for
		**	another refinery to unload at.
		*/
		case HEADINGHOME:
			Assign_Mission(MISSION_ENTER);
			return(1);

		/*
		**	The harvester has nothing to do. There is no Tiberium nearby and
		**	no where to go.
		*/
		case GOINGTOIDLE:
			if (IsUseless) {
				if (House->BQuantity.Value(Rule->RepairBay->HeapID) > 0) {
					Assign_Mission(MISSION_REPAIR);
				} else {
					Assign_Mission(MISSION_HUNT);
				}
			}
			BuildingClass *bptr = Map[(Coord const &)PositionCoord].Cell_Building();
			if (bptr != NULL) {
				if (bptr->Class->IsRefinery || bptr->Class->IsWeeder) {
					Assign_Destination(&Map[Nearby_Location(bptr)]);
				}
			}
			Assign_Mission(MISSION_GUARD);
			break;

	}
	return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));

	}

	Assign_Mission(MISSION_GUARD);
	return(1);
}


/***********************************************************************************************
 * UnitClass::Mission_Hunt -- This is the AI process for aggressive enemy units.               *
 *                                                                                             *
 *    Computer controlled units must be intelligent enough to find enemies as well as to       *
 *    attack them. This AI process will handle both the simple attack process as well as the   *
 *    scanning for enemy units to attack.                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the delay before calling this routine again.                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int UnitClass::Do_MISSION_HUNT(void)
{
	if (Class->DeploysInto != NULL && (Class->DeploysInto == Rule->BuildConst[0] || TarCom != NULL || House->Is_Human_Player())) {
		enum {
			FIND_SPOT,
			WAITING
		};

		switch (Status) {

			/*
			**	This stage handles locating a convenient spot, rotating to face the correct
			**	direction and then commencing the deployment operation.
			*/
			case FIND_SPOT:
				if (Goto_Clear_Spot()) {
					if (Try_To_Deploy()) {
						Status = WAITING;
					}
				}
				break;

			/*
			**	This stage watchdogs the deployment operation and if for some reason, the deployment
			**	is aborted (the IsDeploying flag becomes false), then it reverts back to hunting for
			**	a convenient spot to deploy.
			*/
			case WAITING:
				if (!IsDeploying) {
					Status = FIND_SPOT;
				}
				break;
		}
	} else {

		return(BASECLASS::Do_MISSION_HUNT());
	}
	return(Current_Mission_Control().Normal_Delay()+Random_Pick(0, 2));
}


/***********************************************************************************************
 * UnitClass::Can_Enter_Cell -- Determines cell entry legality.                                *
 *                                                                                             *
 *    Use this routine to determine if the unit can enter the cell                             *
 *    specified and given the direction of entry specified. Typically,                         *
 *    this is used when determining unit travel path.                                          *
 *                                                                                             *
 * INPUT:   cell     -- The cell to examine.                                                   *
 *                                                                                             *
 *          facing   -- The facing that the unit would enter the specified                     *
 *                      cell. If this value is -1, then don't consider                         *
 *                      facing when performing the check.                                      *
 *                                                                                             *
 * OUTPUT:  Returns the reason why it couldn't enter the cell or MOVE_OK if movement is        *
 *          allowed.                                                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/07/1992 JLB : Created.                                                                 *
 *   04/16/1994 JLB : Converted to member function.                                            *
 *   07/04/1995 JLB : Allowed to drive on building trying to enter it.                         *
 *=============================================================================================*/
MoveType UnitClass::Can_Enter_Cell(CellClass const * cellptr, FacingType dir, int cell_height, CellClass const *cellptr2, bool use_locomotor_enter_check) const
{
	bool bridge = cellptr->IsUnderBridge && (cell_height == -1 || abs(cell_height - cellptr->Height) > 1);
	unsigned char flags = cellptr->Flag.Composite;
	bool vehicle = cellptr->Flag.Occupy.Vehicle;
	HousesType inftype = cellptr->InfType;
	TubeClass *tunnel = cellptr->Get_Tunnel();

	if (Class->MovementRestrictedTo != LAND_NONE) {
		if (cellptr->Land_Type() == LAND_TUNNEL) {
			IsometricTileTypeClass * itptr = IsometricTileTypes[cellptr->ITType];
			if (itptr->Width == 5 && itptr->Height == 3 || itptr->Width == 4 && itptr->Height == 3) {
				if (cellptr->SubTile != 2) {
					return(MOVE_NO);
				}
			} else if ((itptr->Width == 3 && itptr->Height == 4 || itptr->Width == 3 && itptr->Height == 5) && cellptr->SubTile != 6) {
				return(MOVE_NO);
			}
		}
		if (cellptr->Land_Type() != Class->MovementRestrictedTo && cellptr->Land_Type() != LAND_TUNNEL) {
			if (cellptr->Overlay < OVERLAY_RAIL_BRIDGE1 || cellptr->Overlay > OVERLAY_RAIL_BRIDGE2 || cell_height == cellptr->Height) {
				return(MOVE_NO);
			}
		}
	}

	if (dir == FACING_COUNT) {
		if (tunnel != NULL) {
			Cell exit = tunnel->Exit;
			if (exit != Cell(0, 0)) {
				return(MOVE_OK);
			}
		}
		return(MOVE_NO);
	}

	if (tunnel != NULL) {
		int dist = abs(dir - tunnel->EnterDir);
		if (dist > 2 && dist < 6 && dir != FACING_NONE) {
			return(MOVE_NO);
		}
	}

	/*
	 * Check the tunnel of the cell entered from (the cell in the opposite
	 * direction) so that the unit can't drive through a tunnel sideways.
	 */
	TubeClass *backtunnel = cellptr->Adjacent_Cell(Facing_Sub(dir, FACING_180)).Get_Tunnel();
	if (backtunnel != NULL) {
		int dist = abs(Facing_Sub(dir, FACING_180) - backtunnel->EnterDir);
		if (dist > FACING_90 && dist < FACING_270 && dir != FACING_NONE) {
			return(MOVE_NO);
		}
	}

	MoveType retval = Can_Reach(cellptr, dir, cell_height, bridge, cellptr2);

	if (retval == MOVE_NO) {
		return(MOVE_NO);
	}

	if (cell_height != -1 && cellptr->IsUnderBridge && cell_height == cellptr->Height + BRIDGE_CELL_HEIGHT) {
		flags = cellptr->BridgeFlag.Composite;
		vehicle = cellptr->BridgeFlag.Occupy.Vehicle;
		inftype = cellptr->BridgeInfType;
	}

	/*
	**	Moving off the edge of the map is not allowed unless
	**	this is a loaner vehicle.
	*/
	if (!ScenarioInit && !Map.In_Local_Radar(cellptr) && !Is_Allowed_To_Leave_Map() && IsLocked) {
		return(MOVE_NO);
	}

	retval = BASECLASS::Can_Enter_Cell(cellptr, dir, cell_height, cellptr2, use_locomotor_enter_check);

	if (retval == MOVE_NO) {
		return(MOVE_NO);
	}


	/*
	**	Certain vehicles can drive over walls. Check for this case and
	**	and return the appropriate flag. Other units treat walls as impassable.
	*/
	if (cellptr->Overlay != OVERLAY_NONE) {
		OverlayTypeClass const * optr = OverlayTypes[cellptr->Overlay];

		if (optr->IsCrate && !House->Is_Human_Player() && Session.Type == GAME_NORMAL) {
			return(MOVE_NO);
		}

		if (optr->IsWall) {

			/*
			**	If the blocking wall is crushable (and not owned by this player or one of this players
			**	allies, then record that it is crushable and let the normal logic take over. The end
			**	result should cause this unit to consider the cell passable.
			*/
			if (optr->IsCrushable && (Class->IsCrusher || Has_Ability(ABILITY_CRUSHER))) {
				if (House->Is_Ally(cellptr->Owner)) {
					if (retval < MOVE_FRIENDLY_DESTROYABLE) retval = MOVE_FRIENDLY_DESTROYABLE;
				}
			} else {
				if (Is_Weapon_Equipped()) {
					WarheadTypeClass const * whead = PrimaryWeapon->WarheadPtr;

					if (whead->IsWallDestroyer || (whead->IsWoodDestroyer && optr->Armor == ARMOR_WOOD)) {
						if (House->Is_Ally(cellptr->Owner)) {
							if (retval < MOVE_FRIENDLY_DESTROYABLE) retval = MOVE_FRIENDLY_DESTROYABLE;
						} else {
							if (retval < MOVE_DESTROYABLE) retval = MOVE_DESTROYABLE;
						}
					} else {
						return(MOVE_NO);
					}
				} else {
					return(MOVE_NO);
				}
			}
		}
	}

	/*
	**	Loop through all of the objects in the square setting a bit
	**	for how they affect movement.
	*/
	bool crushable = false;
	ObjectClass * obj = cellptr->Cell_Occupier(bridge);
	while (obj != NULL) {

		if (const_cast<UnitClass *>(this) != obj) {

			/*
			**	Always allow entry if trying to move on a cell with
			**	authorization from the occupier.
			*/
			if (obj == Contact_With_Whom() && (IsTethered || (obj->RTTI == RTTI_BUILDING && ((BuildingClass *)obj)->Class->IsCanUnitRepair))) {
				return(MOVE_OK);
			}

			if (Class->IsSmallVisceroid && obj->RTTI == RTTI_UNIT && ((UnitClass *)obj)->Class->IsSmallVisceroid) {
				return(MOVE_OK);
			}

			if (obj->RTTI == RTTI_BUILDING) {
				BuildingClass *bptr = (BuildingClass *)obj;

				if (bptr->Class->IsGate) {
					if (!bptr->Is_Gate_Open()) {
						if (bptr->House->Is_Ally(House)) {
							if (retval < MOVE_CLOSED_GATE) retval = MOVE_CLOSED_GATE;
						} else {
							if (!Is_Weapon_Equipped()) {
								return(MOVE_NO);
							}
							if (retval < MOVE_DESTROYABLE) retval = MOVE_DESTROYABLE;
						}
					}
					obj = obj->Next;
					continue;
				}

				/*
				 * A unit can drive onto the building it is trying to enter (repair
				 * bay) if that building is in the cell it is entering from.
				 */
				if (dir != FACING_NONE && bptr->Class->IsCanUnitRepair && Map[Adjacent_Cell(cellptr->CellID, Facing_Sub(dir, FACING_180))].Cell_Building() == bptr) {
					obj = obj->Next;
					continue;
				}

				/*
				 * Various special-purpose buildings do not block movement.
				 */
				if (bptr->Class->IsInvisibleInGame) {
					obj = obj->Next;
					continue;
				}
				if (bptr->Class->IsLimpetMine) {
					obj = obj->Next;
					continue;
				}
				if (bptr->Class->IsLaserFence && (bptr->LaserFenceFrame == 12 || bptr->LaserFenceFrame == 8)) {
					obj = obj->Next;
					continue;
				}
				if (bptr->Class->IsFirestormWall) {
					if (bptr->House->FirestormDefenseActivated) {
						return(MOVE_NO);
					}
					obj = obj->Next;
					continue;
				}

				/*
				 * The bib of a refinery or weeder is a legal destination for a
				 * harvester whose house is mutually allied and that is heading for the dock.
				 */
				if (bptr->Class->IsBibbed) {
					if (Map[Adjacent_Cell(cellptr->CellID, FACING_E)].Cell_Building() != bptr) {
						obj = obj->Next;
						continue;
					}
					if (Class->IsToHarvest && bptr->Class->IsRefinery &&
						House->Is_Ally(bptr->House) && bptr->House->Is_Ally(House)) {
						Cell bcell = bptr->PositionCell;
						if (cellptr->CellID == bcell + Cell(2, 1)) {
							obj = obj->Next;
							continue;
						}
					}
					if (Class->IsToVeinHarvest && bptr->Class->IsWeeder &&
						House->Is_Ally(bptr->House) && bptr->House->Is_Ally(House)) {
						Cell bcell = bptr->PositionCell;
						if (cellptr->CellID == bcell + Cell(2, 1)) {
							obj = obj->Next;
							continue;
						}
					}
				}
			}

			/*
			**	Special check to allow entry into the sea transport this vehicle
			**	is trying to enter.
			*/
			if (Mission == MISSION_ENTER && obj == NavCom && IsTethered) {
				return(MOVE_OK);
			}

			/*
			**	Guard area should not allow the guarding unit to enter the cell with the
			**	guarded unit.
			*/
			if (Mission == MISSION_GUARD_AREA && ArchiveTarget == obj) {
				return(MOVE_NO);
			}

			FootClass *foot = dynamic_cast<FootClass *>(obj);

			bool is_moving = foot != NULL &&
					(foot->NavCom || foot->PrimaryFacing.Is_Rotating() || foot->Locomotion->Is_Moving());

			if (House->Is_Ally(obj)) {
				if (is_moving) {
					int face = PrimaryFacing.Current().As_Dir8();
					DirType d = ((FootClass const *)obj)->PrimaryFacing.Current().Right_180();
					int techface = d.As_Dir8();
					DirType direction = ::Direction(PositionCoord, ((ObjectClass *)obj)->PositionCoord);
					if (face == techface && Distance(((ObjectClass *)obj)->Center_Coord()) <= 2 * CELL_LEPTON - 1 && direction.As_Dir8() == face) {
						return(MOVE_NO);
					}
					if ((foot->IsOccupyingCell && foot->RTTI != RTTI_INFANTRY) || foot->Locomotion->Will_Jump_Tracks()) {
						if (retval < MOVE_MOVING_BLOCK) retval = MOVE_MOVING_BLOCK;
					}
				} else {

					/*
					 * An allied object that is not moving blocks the cell outright
					 * if it is a building; otherwise it only temporarily blocks
					 * the cell.
					 */
					if (obj->RTTI == RTTI_BUILDING) return(MOVE_NO);

					if (!Class->IsJellyfish) {
						if (retval < MOVE_TEMP) retval = MOVE_TEMP;
					}
				}
			} else {

				/*
				**	Cloaked enemy objects are not considered if this is a Find_Path()
				**	call.
				*/
				TechnoClass * tech = Dynamic_Cast<TechnoClass *>(obj);
				if (tech == NULL || tech->Cloak != CLOAKED) {

					/*
					**	If this unit can crush infantry, and there is an enemy infantry in the
					**	cell, don't consider the cell impassible. This is true even if the unit
					**	doesn't contain a legitimate weapon.
					*/
					bool crusher = Class->IsCrusher || Has_Ability(ABILITY_CRUSHER);
					if (!crusher || !obj->Class_Of()->IsCrushable) {

						/*
						 * Any non-allied blockage is considered impassable if the unit
						 * is not equipped with a weapon (unless it is a train).
						 */
						if (PrimaryWeapon == NULL && !Class->IsTrain) return(MOVE_NO);

						/*
						**	Some kinds of terrain are considered destroyable if the unit is equipped
						**	with the weapon that can destroy it. Otherwise, the terrain is considered
						**	impassable.
						*/
						RTTIType rtti = obj->RTTI;
						switch (rtti) {
							case RTTI_TERRAIN: {

								WeaponTypeClass const * weapon = Class->Get_Weapon(What_Weapon_Should_I_Use(obj))->Weapon;
								if (weapon != NULL && weapon->WarheadPtr != NULL &&
									weapon->WarheadPtr->IsWoodDestroyer && !obj->Class_Of()->IsImmune) {

									if (retval < MOVE_DESTROYABLE) retval = MOVE_DESTROYABLE;
								} else {
									return(MOVE_NO);
								}
								break;
							}

							case RTTI_BUILDING: {

								if (((BuildingClass *)obj)->Class->IsBridgeRepairHut) {
									return(MOVE_NO);
								}
								if (retval < MOVE_DESTROYABLE) retval = MOVE_DESTROYABLE;
								break;
							}

							default:
								if (retval < MOVE_DESTROYABLE) retval = MOVE_DESTROYABLE;
								break;
						}
					} else {
						if (!House->Is_Ally(obj)) crushable = true;
					}
				} else {
					if (retval < MOVE_CLOAK) retval = MOVE_CLOAK;
				}
			}
		} else {

			/*
			 * Ignore this unit itself; clear its own vehicle reservation from
			 * the cell flags so it isn't considered a blockage.
			 */
			vehicle = false;
			flags &= ~0x20;
		}

		/*
		**	Move to next object in chain.
		*/
		obj = obj->Next;
	}

	/*
	**	If the cell is out and out impassable because of underlying terrain, then
	**	return this immutable fact.
	*/
	if (!bridge && Ground[cellptr->Land_Type()].Cost[Class->Speed] == 0) {
		return(MOVE_NO);
	}

	/*
	**	If some allied object has reserved the cell, then consider the cell
	**	as blocked by a moving object.
	*/
	if (retval == MOVE_OK && !crushable && (flags & 0x3F) != 0 && !Class->IsJellyfish) {

		/*
		**	If reserved by a vehicle, then consider this blocked terrain.
		*/
		if (vehicle) {
			retval = MOVE_MOVING_BLOCK;
		} else {
			if (inftype != HOUSE_NONE && House->Is_Ally(inftype)) {
				retval = MOVE_MOVING_BLOCK;
			} else {

				/*
				**	Enemy infantry have reserved the cell. If this unit can crush
				**	infantry, consider the cell passable. If not, then consider the
				**	cell destroyable if it has a weapon. If neither case applies, then
				**	this vehicle should avoid the cell altogether.
				*/
				if (!Class->IsCrusher && !Has_Ability(ABILITY_CRUSHER)) {
					if (Class->IsTrain || (PrimaryWeapon != NULL && PrimaryWeapon->Bullet->IsAntiGround)) {
						retval = MOVE_DESTROYABLE;
					} else {
						return(MOVE_NO);
					}
				}
			}
		}
	}

	/*
	**	If its ok to move into the cell because we can crush whats in the cell, then
	**	make sure no one else is already moving into the cell to crush something.
	*/
	if (retval == MOVE_OK && crushable && vehicle) {

		/*
		**	However, if the cell is occupied by a crushable vehicle, then we can
		**	never be sure if some other friendly vehicle is also trying to crush
		**	the cell at the same time. In the case of a crushable vehicle in the
		**	cell, then allow entry.
		*/
		if (!cellptr->Cell_Unit() || !cellptr->Cell_Unit()->Class->IsCrushable) {
			return(MOVE_MOVING_BLOCK);
		}
	}

	/*
	**	Return with the most severe reason why this cell would be impassable.
	*/
	return(retval);
}


/***********************************************************************************************
 * UnitClass::What_Action -- Determines what action would occur if clicked on object.          *
 *                                                                                             *
 *    Use this function to determine what action would likely occur if the specified object    *
 *    were clicked on while this unit was selected as current. This function controls, not     *
 *    only the action to perform, but indirectly controls the cursor shape to use as well.     *
 *                                                                                             *
 * INPUT:   object   -- The object that to check for against "this" object.                    *
 *                                                                                             *
 * OUTPUT:  Returns with the default action to perform. If no clear action can be determined,  *
 *          then ACTION_NONE is returned.                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/11/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ActionType UnitClass::What_Action(ObjectClass const * object, bool disallow_force) const
{
	ActionType action = BASECLASS::What_Action(object, disallow_force);

	/*
	**	If the unit doesn't have a weapon, but can crush the object, then consider
	**	the object as a movable location.
	*/
	if (action == ACTION_ATTACK && !Can_Player_Fire()) {
		if ((Class->IsCrusher || Has_Ability(ABILITY_CRUSHER)) && object->Class_Of()->IsCrushable) {
			action = ACTION_MOVE;
		} else {
			action = ACTION_SELECT;
		}
	}

	/*
	**	Don't allow special deploy action unless there is something to deploy.
	*/
	if (action == ACTION_SELF) {
		if (Class->DeploysInto != NULL) {

			Cell cell = Center_Coord().As_Cell();
			if (Class->DeploysInto == Rule->BuildConst[0]) {
				cell = Adjacent_Cell(cell, FACING_NW);
			} else {
				bool hasfactory = false;
				for (int index = 0; index < Rule->BuildWeapons.Count(); index++) {
					if (Class->DeploysInto == Rule->BuildWeapons[index]) {
						cell = Adjacent_Cell(cell, FACING_NW);
						break;
					}
				}

			}

			/*
			**	The MCV will get the no-deploy cursor if it couldn't
			**	deploy at its current location.
			*/
			((UnitClass &)(*this)).Mark(MARK_UP);
			Locomotion->Mark_All_Occupation_Bits(MARK_UP);
			if (!Class->DeploysInto->Legal_Placement(cell)) {
				action = ACTION_NO_DEPLOY;
			}
			Locomotion->Mark_All_Occupation_Bits(MARK_DOWN);
			((UnitClass &)(*this)).Mark(MARK_DOWN);
		} else {

			/*
			**	All other units can "deploy" their passengers if they in-fact have
			**	passengers and are a transport vehicle. Otherwise, they cannot
			**	perform any self action.
			*/
			if (Class->Max_Passengers() > 0) {
				if (Cargo.How_Many() == 0) {
					action = ACTION_NO_DEPLOY;
				} else {
					bool can_deploy = false;
					if (IsOnBridge) {
						can_deploy = true;
					} else {
						CellClass * cellptr = &Map[(Coord const &)PositionCoord];
						ObjectClass * object = Cargo.Attached_Object();
						if (object == NULL || Ground[cellptr->Land_Type()].Cost[object->TClass->Speed] >= 0.01) {
							can_deploy = true;
						}
					}
					if (!can_deploy) {
						action = ACTION_NO_DEPLOY;
					}
				}
			} else {
				if (Class->IsMobileEMP) {
					if (Charge < Class->MaxCharge) {
						action = ACTION_NO_DEPLOY;
					}
				} else {
					action = ACTION_NONE;
				}
			}
		}
	}

	if (action == ACTION_SELF && !Can_Deploy_Now()) {
		action = ACTION_NO_DEPLOY;
	}

	TechnoClass const * techno = Dynamic_Cast<TechnoClass const *>((AbstractClass const *)object);

	/*
	**	Special return to friendly refinery action.
	*/
	if (House->Is_Player_Control() && techno != NULL && techno->House->Is_Ally(this) && House->Is_Ally(techno->House)) {
		if (object->RTTI == RTTI_BUILDING && ((UnitClass *)this)->Transmit_Message(RADIO_CAN_LOAD, (TechnoClass*)object) == RADIO_ROGER) {
			action = ACTION_ENTER;
		}
	}

	/*
	**	Special return to friendly repair factory action.
	*/
	if (House->Is_Player_Control() && action == ACTION_SELECT && object->RTTI == RTTI_BUILDING) {
		BuildingClass * building = (BuildingClass *)object;
		if (building != NULL && building->House->Is_Ally(this) && House->Is_Ally(building->House)) {
			if (building->Class->IsCanUnitRepair && ((UnitClass *)this)->Transmit_Message(RADIO_CAN_LOAD, building) == RADIO_ROGER && !building->In_Radio_Contact() && !building->Cargo.Is_Something_Attached()) {
				action = ACTION_MOVE;
			}
		}
	}

	if (Combat_Damage() < 0 && House->Is_Player_Control()) {
		if (House->Is_Ally(object)) {
			if (object->Considered_Vehicle() && object != this && object->Not_Underground()) {
				if ( object->RTTI != RTTI_AIRCRAFT || Map[object->Center_Coord()].Cell_Building() == NULL) {
					if (object->HealthRatio < Rule->ConditionGreen) {
						action = ACTION_GREPAIR;
					}
				}
			} else if ( object->RTTI != RTTI_BUILDING ) {
				action = ACTION_SELECT;
			}
		} else {
			action = ACTION_ATTACK_SUPPORT;
		}
	}

	if (action == ACTION_ATTACK) {
		if (!Is_Weapon_Equipped()) {
			action = ACTION_ATTACK_SUPPORT;
		}
	}

	/*
	**	If it doesn't know what to do with the object, then just
	**	say it can't move there.
	*/
	if (action == ACTION_NONE && House->Is_Player_Control()) {
		action = ACTION_NOMOVE;
	}

	if (action == ACTION_ATTACK && !Can_Attack_Now()) {
		action = ACTION_NONE;
	}


	return(action);
}


/***********************************************************************************************
 * UnitClass::What_Action -- Determines action to perform on specified cell.                   *
 *                                                                                             *
 *    This routine will determine what action to perform if the mouse were clicked over the    *
 *    cell specified. At the unit level, only the harvester is checked for. The lower          *
 *    classes determine the regular action response.                                           *
 *                                                                                             *
 * INPUT:   cell  -- The cell that the mouse might be clicked on.                              *
 *                                                                                             *
 * OUTPUT:  Returns with the action type that this unit will perform if the mouse were         *
 *          clicked of the cell specified.                                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ActionType UnitClass::What_Action(Cell const & cell, bool check_fog, bool disallow_force) const
{
	if (!House->Is_Player_Control()) {
		return(ACTION_NONE);
	}

	CellClass * cellptr = &Map[cell];

	ActionType action = BASECLASS::What_Action(cell, check_fog, disallow_force);

	if (Combat_Damage() < 0 && House->Is_Player_Control() && action == ACTION_ATTACK) {
		action = ACTION_ATTACK_SUPPORT;
	}

	OverlayType overlay = OVERLAY_NONE;
	if (check_fog && Scen->Special.IsFogOfWar && cellptr->FoggedObjects != NULL) {
		for (int i = 0; i < cellptr->FoggedObjects->Count(); i++) {
			FoggedObjectClass * fogged = (*cellptr->FoggedObjects)[i];
			if (fogged->CanDraw && fogged->RTTI == RTTI_OVERLAY) {
				overlay = fogged->Overlay;
			}
		}
	}

	Coord coord = cell;
	coord.Z = Map.Get_Height_GL(coord);
	if (Map[coord].IsUnderBridge) {
		coord.Z += BRIDGE_LEPTON_HEIGHT;
	}

	if (!Map.Is_Shrouded(coord)) {
		if (action == ACTION_MOVE) {
			if ((cellptr->Land_Type() == LAND_TIBERIUM || overlay != OVERLAY_NONE && OverlayTypes[overlay]->IsTiberium && check_fog) && Class->IsToHarvest) {
				return(ACTION_HARVEST);
			}
			if ((cellptr->Land_Type() == LAND_WEEDS || overlay == OVERLAY_VEINS && cellptr->OverlayData >= OVERLAYDATA_FIRST_SOLID_VEIN && check_fog) && Class->IsToVeinHarvest) {
				return(ACTION_HARVEST);
			}
		}
	}

	if (action == ACTION_MOVE) {
		if (Map[cell].Has_Tunnel()) {
			if (Map[cell].Can_Enter_Tunnel((FootClass *)this)) {
				return(ACTION_ENTER_TUNNEL);
			}
			return(ACTION_NO_ENTER_TUNNEL);
		}
	} else {
		if (action == ACTION_ATTACK && !Can_Attack_Now()) {
			action = ACTION_NONE;
		}
	}

	return(action);
}


/***********************************************************************************************
 * UnitClass::Mission_Guard -- Special guard mission override processor.                       *
 *                                                                                             *
 *    Handles the guard mission for the unit. If the IQ is high enough and the unit is         *
 *    a harvester, it will begin to harvest automatically. An MCV might autodeploy.            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the time delay before this command is executed again.                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *   05/08/1995 JLB : Fixes gunboat problems.                                                  *
 *=============================================================================================*/
int UnitClass::Do_MISSION_GUARD(void)
{
	bool needs_dock = true;
	if (/*House->IsBaseBuilding &&*/ !House->Is_Human_Player() && (Class->IsToHarvest || Class->IsToVeinHarvest)) {
		for (int i = 0; i < Class->Dock.Count(); i++) {
			if (House->BQuantity.Value(Class->Dock[i]->HeapID) > 0) {
				needs_dock = false;
				break;
			}
		}
	}

	if (needs_dock || (Class->IsToHarvest && House->IsTiberiumShort)) {
		if (Class->DeploysInto == Rule->BuildConst[0] && House->IsBaseBuilding && !House->Is_Human_Player()) {
			Assign_Mission(MISSION_UNLOAD);
			return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
		}
		if (Class->IsToVeinHarvest && IsDroppedFromTeam) {
			IsDroppedFromTeam = false;
			Assign_Mission(MISSION_HARVEST);
		}
	} else {
		Assign_Mission(MISSION_HARVEST);
		return(1);
//		return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
	}
	return(BASECLASS::Do_MISSION_GUARD());
}


/***********************************************************************************************
 * UnitClass::Mission_Move -- Handles special move mission overrides.                          *
 *                                                                                             *
 *    This routine intercepts the normal move mission and if a gunboat is being processed,     *
 *    changes its mission to hunt. This is an attempt to keep the gunboat on the hunt mission  *
 *    regardless of what the player did.                                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the number of ticks before this routine should be called again.            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/09/1995 JLB : Created.                                                                 *
 *   09/28/1995 JLB : Harvester stick in guard mode if no more refineries.                     *
 *=============================================================================================*/
int UnitClass::Do_MISSION_MOVE(void)
{
	IsHarvesting = false;

	/*
	**	Always make sure that that transport door is closed if the vehicle is moving.
	*/
	if (!Door.Is_Door_Closed()) {
		APC_Close_Door();
	}

	return(BASECLASS::Do_MISSION_MOVE());
}


/// <summary>
/// Handles the patrol mission for a vehicle.
/// This routine sees the transport door shut before handing the mission over to the
/// normal foot object patrol logic.
/// </summary>
/// <returns>The delay in game frames before this mission should be processed again.</returns>
int UnitClass::Do_MISSION_PATROL(void)
{
	IsHarvesting = false;

	/*
	**	Always make sure that that transport door is closed if the vehicle is moving.
	*/
	if (!Door.Is_Door_Closed()) {
		APC_Close_Door();
	}

	return(BASECLASS::Do_MISSION_PATROL());
}

/***********************************************************************************************
 * UnitClass::Desired_Load_Dir -- Determines the best cell and facing for loading.             *
 *                                                                                             *
 *    This routine examines the unit and adjacent cells in order to find the best facing       *
 *    for the transport and best staging cell for the potential passengers. This location is   *
 *    modified by adjacent cell passability and direction of the potential passenger.          *
 *                                                                                             *
 * INPUT:   passenger   -- Pointer to the potential passenger.                                 *
 *                                                                                             *
 *          moveto      -- Reference to the cell number that specifies where the potential     *
 *                         passenger should move to first.                                     *
 *                                                                                             *
 * OUTPUT:  Returns with the direction the transport should face before opening the transport  *
 *          door.                                                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
FacingType UnitClass::Desired_Load_Dir(ObjectClass * passenger, Cell & moveto) const
{
	/*
	**	Determine the ideal facing that provides the least resistance. This would be the direction
	**	of the potential passenger or the current transport facing if it is going to unload.
	*/
	FacingType face = FACING_N;
	FacingType faceto;
	if (passenger != NULL) {
		faceto = (FacingType)Direction(passenger).As_Dir256();
	} else {
		faceto = (FacingType)(PrimaryFacing.Current().Right_180()).As_Dir256();
	}
	int desired = (int)(signed char)faceto;

	/*
	**	Sweep through the adjacent cells in order to find the best candidate.
	*/
	FacingType bestdir = FACING_N;
	int bestval = -1;
	for (; face < FACING_COUNT; face++) {
		int value = 0;
		Cell cellnum = Adjacent_Cell(PositionCell, face);

		/*
		**	Base the initial value of the potential cell according to whether the passenger is
		**	allowed to enter the cell. If it can't, then give such a negative value to the
		**	cell so that it is prevented from ever choosing that cell for load/unload.
		*/
		if (passenger != NULL) {
			value = (passenger->Can_Enter_Cell(&Map[cellnum], face, Get_Cell_Height()) == MOVE_OK || passenger->PositionCell == cellnum) ? 128 : -128;
		} else {
			CellClass * cell = &Map[Cell(cellnum)];
			if (Ground[cell->Land_Type()].Cost[SPEED_FOOT] == 0 || cell->Flag.Occupy.Building || cell->Flag.Occupy.Vehicle || cell->Flag.Occupy.Monolith || (cell->Flag.Composite & 0x01F) == 0x01F) {
				value = -128;
			} else {
				if (cell->Cell_Techno() && !House->Is_Ally(cell->Cell_Techno())) {
					value = -128;
				} else {
					value = 128;
				}
			}
		}

		/*
		**	Give more weight to the cells that require the least rotation of the transport or the
		**	least roundabout movement for the potential passenger.
		*/
		value -= (int)abs((int)(signed char)DirType(face).As_Dir256() - desired);
		if (face == FACING_S) {
			value -= 100;
		}
//		if (face == FACING_SW || face == FACING_SE) value += 64;

		/*
		**	If the value for the potential cell is greater than the last recorded potential
		**	value, then record this cell as the best candidate.
		*/
		if (bestval == -1 || value > bestval) {
			bestval = value;
			bestdir = face;
		}
	}

	/*
	**	If a suitable direction was found, then return with the direction value.
	*/
	FacingType facing = FACING_S;
	moveto = CELL_NONE;
	if (bestval > 0) {
		moveto = Adjacent_Cell(PositionCell, bestdir);
		facing = (FacingType)Harvester_Load_List[bestdir];
	}

	if (Class->IsTrain) {
		facing = PrimaryFacing.Current().As_Dir8();
	}

	return(facing);
}


/***********************************************************************************************
 * UnitClass::Flag_Attach -- Attaches a house flag to this unit.                               *
 *                                                                                             *
 *    This routine will attach a house flag to this unit.                                      *
 *                                                                                             *
 * INPUT:   house -- The house that is having its flag attached to it.                         *
 *                                                                                             *
 * OUTPUT:  Was the house flag successfully attached to this unit?                             *
 *                                                                                             *
 * WARNINGS:   A unit can only carry one flag at a time. This might be a reason for failure    *
 *             of this routine.                                                                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool UnitClass::Flag_Attach(HousesType house)
{
	if (house != HOUSE_NONE && Flagged == HOUSE_NONE) {
		Flagged = house;
		Mark(MARK_CHANGE);
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * UnitClass::Flag_Remove -- Removes the house flag from this unit.                            *
 *                                                                                             *
 *    This routine will remove the house flag that is attached to this unit.                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Was the flag successfully removed?                                                 *
 *                                                                                             *
 * WARNINGS:   This routine doesn't put the flag into a new location. That operation must      *
 *             be performed or else the house flag will cease to exist.                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool UnitClass::Flag_Remove(void)
{
	if (Flagged != HOUSE_NONE) {
		Flagged = HOUSE_NONE;
		Mark(MARK_CHANGE);
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * UnitClass::Pip_Count -- Fetches the number of pips to display on unit.                      *
 *                                                                                             *
 *    This routine is used to fetch the number of "fullness" pips to display on the unit.      *
 *    This will either be the number of passengers or the percentage full (in 1/5ths) of       *
 *    a harvester.                                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of pips to draw on this unit.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int UnitClass::Pip_Count(void) const
{
	switch (Class->PipScale) {
		case PIPSCALE_TIBERIUM:
			return((int)(Storage.Get_Total_Amount() / (double)Class->Capacity * Class->Max_Pips() + 0.5));

		case PIPSCALE_CHARGE:
			return((int)((Charge / (double)Class->MaxCharge) * Class->Max_Pips() + 0.5));

		default:
			return(BASECLASS::Pip_Count());
	}
}


/***********************************************************************************************
 * UnitClass::APC_Close_Door -- Closes an APC door.                                            *
 *                                                                                             *
 *    This routine will initiate closing of the APC door.                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void UnitClass::APC_Close_Door(void)
{
	Door.Close_Door(Class->DeployTime);
}


/***********************************************************************************************
 * UnitClass::APC_Open_Door -- Opens an APC door.                                              *
 *                                                                                             *
 *    This routine will initiate opening of the APC door.                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void UnitClass::APC_Open_Door(void)
{
	if (!Locomotion->Is_Moving() && !IsRotating) {
		Door.Open_Door(Class->DeployTime);
	}
}


/***********************************************************************************************
 * UnitClass::Crew_Type -- Fetches the kind of crew that this object produces.                 *
 *                                                                                             *
 *    When a unit is destroyed, a crew member might be generated. This routine will return     *
 *    with the infantry type to produce for this unit. This routine will be called for every   *
 *    survivor that is generated.                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a suggested infantry type to generate as a survivor from this unit.   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
InfantryTypeClass const * UnitClass::Crew_Type(void) const
{
	return(BASECLASS::Crew_Type());
}


/***********************************************************************************************
 * UnitClass::Mission_Repair -- Handles finding and proceeding on a repair mission.            *
 *                                                                                             *
 *    This mission handler will look for a repair facility. If one is found then contact       *
 *    is established and then the normal Mission_Enter logic is performed. The repair facility *
 *    will take over the actual repair coordination process.                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the number of game frames to delay before calling this routine again.      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int UnitClass::Do_MISSION_REPAIR(void)
{
	BuildingClass * nearest = Find_Docking_Bay(Rule->RepairBay, true);

	IsHarvesting = false;

	/*
	**	If there is no available repair facility, then check to see if there
	**	are any repair facilities at all. If not, then enter this unit
	**	into idle state.
	*/
	if (nearest == NULL) {
		if (!(House->BQuantity.Value(Rule->RepairBay->HeapID))) {
			Enter_Idle_Mode();
		}
	} else {

		/*
		**	Try to establish radio contact with the repair facility. If contact
		**	was established, then proceed with normal enter mission, which handles
		**	the repair process.
		*/
		if (Transmit_Message(RADIO_HELLO, nearest) == RADIO_ROGER) {
			Assign_Mission(MISSION_ENTER);
			return(1);
		}
	}

	/*
	**	If no action could be performed at this time, then wait
	**	around for a bit before trying again.
	*/
	return(Current_Mission_Control().Normal_Delay());
}


/***********************************************************************************************
 * UnitClass::Fire_Direction -- Determines the direction of firing.                            *
 *                                                                                             *
 *    This routine will return with the facing that a projectile will travel if it was         *
 *    fired at this instant. The facing should match the turret facing for those units         *
 *    equipped with a turret. If the unit doesn't have a turret, then it will be the facing    *
 *    of the body.                                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the default firing direction for a projectile.                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
DirType UnitClass::Fire_Direction(void) const
{
	if (Class->IsTurretEquipped) {
		return(SecondaryFacing.Current());
	}

	return(BASECLASS::Fire_Direction());
}


/***********************************************************************************************
 * UnitClass::Can_Fire -- Determines if turret can fire upon target.                           *
 *                                                                                             *
 *    This routine determines if the turret can fire upon the target                           *
 *    specified.                                                                               *
 *                                                                                             *
 * INPUT:   target   -- The target to fire upon.                                               *
 *                                                                                             *
 *          which    -- Which weapon to use to determine legality to fire. 0=primary,          *
 *                      1=secondary.                                                           *
 *                                                                                             *
 * OUTPUT:  Returns the fire status type that indicates if firing is allowed and if not, why.  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/26/1994 JLB : Created.                                                                 *
 *   06/01/1994 JLB : Returns reason why it can't fire.                                        *
 *=============================================================================================*/
FireErrorType UnitClass::Can_Fire(AbstractClass * target, int which) const
{
	FireErrorType	fire = BASECLASS::Can_Fire(target, which);

	if (fire == FIRE_OK || fire == FIRE_FACING) {
		bool buildable = Map[(Coord const &)PositionCoord].Can_Build_Here();
		if (Class->IsDeployToFire && Deploy_To_Fire() && !buildable) {
			return(FIRE_MUST_DEPLOY);
		}
	}

	if (fire == FIRE_OK) {
		if (IsTethered && Contact_With_Whom()->RTTI == RTTI_BUILDING) {
			return(FIRE_CANT);
		}

		WeaponTypeClass const * weapon = Get_Class_Weapon_Data(which)->Weapon;

		if (Combat_Damage() < 0) {
			TechnoClass const * techno = Dynamic_Cast<TechnoClass const *>((AbstractClass const *)target);
			if (techno == NULL || !techno->Considered_Vehicle() || techno->HealthRatio >= Rule->ConditionGreen) {
				return(FIRE_ILLEGAL);
			}
		}

		/*
		**	If this unit cannot fire while moving, then bail.
		*/
		if ((Class->IsNoFireWhileMoving /*!Class->IsTurretEquipped || Class->IsLockTurret*/) && NavCom != NULL) {
			return(FIRE_MOVING);
		}

		if ((weapon->UseSparkParticles || weapon->UseFireParticles) && NavCom != NULL) {
			return(FIRE_MOVING);
		}

		/*
		**	If the turret is rotating and the projectile isn't a homing type, then
		**	firing must be delayed until the rotation stops.
		*/
		if (!IsFiring && IsRotating && weapon->Bullet->ROT == 0) {
			return(FIRE_ROTATING);
		}

		if (!Class->IsLargeVisceroid && !Class->IsSmallVisceroid && !Class->IsJellyfish) {
			DirType turret;
			/*
			**	Determine if the turret facing isn't too far off of facing the target.
			*/
			if (Class->IsTurretEquipped) {
				turret = SecondaryFacing.Current();
			} else {
				turret = PrimaryFacing.Current();
			}
			DirType rot = (weapon->Bullet->ROT ? DIR_STEP_16 : DIR_STEP_32);
			if (rot < (turret - Direction(target))) {
				return(FIRE_FACING);
			} else {
				//return(BASECLASS::Can_Fire(target, which));
			}

		}

		FireErrorType locomotion_fire = Locomotion->Can_Fire();
		if (locomotion_fire != FIRE_OK) {
			return(locomotion_fire);
		}
	}
	return(fire);
}


/***********************************************************************************************
 * UnitClass::Fire_At -- Try to fire upon the target specified.                                *
 *                                                                                             *
 *    This routine is the auto-fire logic for the turret. It will check                        *
 *    to see if firing is technically legal given the specified target.                        *
 *    If it is legal to fire, it does so. It is safe to call this routine                      *
 *    every game tick.                                                                         *
 *                                                                                             *
 * INPUT:   target   -- The target to fire upon.                                               *
 *                                                                                             *
 *          which    -- Which weapon to use when firing. 0=primary, 1=secondary.               *
 *                                                                                             *
 * OUTPUT:  bool; Did firing occur?                                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
BulletClass * UnitClass::Fire_At(AbstractClass * target, int which)
{
	BulletClass * bullet = NULL;
	WeaponTypeClass const * weap = Get_Class_Weapon_Data(which)->Weapon;
	if (weap == NULL) return(NULL);

	bool sync = false;
	if (which != 1) {
		int burst = BurstIndex % weap->Burst;
		int frame = burst < 2 ? Class->FiringSyncFrame[burst] : -1;
		sync = FiringSyncDelay != -1 && frame != -1;
	}

	if (Can_Fire(target, which) == FIRE_OK) {
		bullet = BASECLASS::Fire_At(target, which);

		if (bullet != NULL) {

			/*
			**	Possible reload timer set.
			*/
			if (Class->MaxAmmo > 0 && !Class->IsManualReload && Reload == 0) {
				Reload = TICKS_PER_SECOND * 30;
			}

			if (!sync && Class->FiringFrames > 0) {
				FiringSyncDelay = 2 * Class->FiringFrames - 1;
			}
		}
	}

	return(bullet);
}


/***********************************************************************************************
 * UnitClass::Class_Of -- Fetches a reference to the class type for this object.               *
 *                                                                                             *
 *    This routine will fetch a reference to the TypeClass of this object.                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with reference to the type class of this object.                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectTypeClass const * UnitClass::Class_Of(void) const
{
	return(Class);
}


/***********************************************************************************************
 * UnitClass::Tiberium_Load -- Determine the Tiberium load as a percentage.                    *
 *                                                                                             *
 *    Use this routine to determine what the Tiberium load is (as a floating point percentage).*
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current "fullness" rating for the object.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/17/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
double UnitClass::Tiberium_Load(void) const
{
	if (Class->IsToHarvest) {
		return(Storage.Get_Total_Amount() / (double)Class->Capacity);
	}
	return(0.0);
}


/***********************************************************************************************
 * UnitClass::Approach_Target -- Handles approaching the target in order to attack it.         *
 *                                                                                             *
 *    This routine will check to see if the target is infantry and it can be overrun. It will  *
 *    try to overrun the infantry rather than attack it. This only applies to computer         *
 *    controlled vehicles. If it isn't the infantry overrun case, then it falls into the       *
 *    base class for normal (complex) approach algorithm.                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/17/1995 JLB : Created.                                                                 *
 *   07/12/1995 JLB : Flamethrower tanks don't overrun -- their weapon is better.              *
 *=============================================================================================*/
void UnitClass::Approach_Target(void)
{
	/*
	**	Only if there is a legal target should the approach check occur.
	*/
	if (!House->Is_Human_Player() && TarCom != NULL && NavCom == NULL) {

		/*
		**	Special case:
		**	If this is for a unit that can crush infantry, and the target is
		**	infantry, AND the infantry is pretty darn close, then just try
		**	to drive over the infantry instead of firing on it.
		*/
		TechnoClass * target = Dynamic_Cast<TechnoClass *>(TarCom);
		if ((Class->IsCrusher || Has_Ability(ABILITY_CRUSHER)) && Distance(TarCom) < Rule->CrushDistance && target && ((TechnoTypeClass const *)(target->Class_Of()))->IsCrushable && (Class->IsAutoCrush || !House->Is_Human_Player())) {
			Assign_Destination(TarCom);
			return;
		}
	}

	/*
	**	In the other cases, uses the more complex "get to just within weapon range"
	**	algorithm.
	*/
	BASECLASS::Approach_Target();
}


/***********************************************************************************************
 * DriveClass::Overrun_Square -- Handles vehicle overrun of a cell.                            *
 *                                                                                             *
 *    This routine is called when a vehicle enters a square or when it is about to enter a     *
 *    square (controlled by parameter). When a vehicle that can crush infantry enters a        *
 *    cell that contains infantry, then the infantry will be destroyed (regardless of          *
 *    affiliation). When a vehicle threatens to overrun a square, all occupying infantry       *
 *    will attempt to get out of the way.                                                      *
 *                                                                                             *
 * INPUT:   cell     -- The cell that is, or soon will be, entered by a vehicle.               *
 *                                                                                             *
 *          threaten -- Don't kill, but just threaten to enter the cell.                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void UnitClass::Overrun_Square(Cell const & cell, bool threaten)
{
	CellClass * cellptr = &Map[cell];
	bool isbridge = false;

	if (cellptr->IsUnderBridge) {
		isbridge = IsOnBridge || (Map[Center_Coord().As_Cell()].Height == (cellptr->Height + BRIDGE_CELL_HEIGHT));
	}

	if (Class->IsCrusher || Has_Ability(ABILITY_CRUSHER)) {
		if (threaten) {

			/*
			**	If the cell contains infantry, then they will panic when a vehicle tries
			**	drive over them. Have the infantry run away instead.
			*/
			if (isbridge) {

				/*
				**	Scattering is controlled by the game difficulty level.
				*/
				if (cellptr->BridgeFlag.Composite & 0x1F) {
					cellptr->Incoming(COORD_NONE, true, false, true);
				}

			} else {

				/*
				**	Scattering is controlled by the game difficulty level.
				*/
				if (cellptr->Flag.Composite & 0x1F) {
					cellptr->Incoming(COORD_NONE, true, false, false);
				}
			}
		} else {
			ObjectClass * object = cellptr->Cell_Occupier(isbridge);
			int crushed = false;
			while (object != NULL) {
				if (object->Class_Of()->IsCrushable && (!House->Is_Ally(object) || Class->IsTrain) && Relative_Distance(object->Center_Coord()) < CELL_LEPTON*64) {

					/*
					**	If we're running over infantry, let's see if the infantry we're
					**	squashing is a thief trying to capture us.  If so, let him succeed.
					*/
					if (object->RTTI == RTTI_INFANTRY && ((InfantryClass *)object)->Class->IsVehicleThief && ((InfantryClass *)object)->NavCom == this && !Class->IsTrain) {
						ObjectClass * next = object->Next;
						IsOwnedByPlayer = ((InfantryClass *)object)->IsOwnedByPlayer;
						Detach_All(false);
						Captured(((InfantryClass *)object)->House);
						object->Delete_Me();
						object = next;
					} else {

						ObjectClass * next = object->Next;
						crushed = true;

						/*
						**	Record credit for the kill(s)
						*/
						Sound_Effect(object->Class_Of()->CrushSound, PositionCoord);
						object->Record_The_Kill(this);
						object->Mark(MARK_UP);
						object->Limbo();
						object->Delete_Me();

						object = next;
					}
				} else {
					object = object->Next;
				}
			}
			if (crushed) Do_Uncloak();
		}
	}
}


/***********************************************************************************************
 * UnitClass::Assign_Destination -- Assign a destination to a unit.                            *
 *                                                                                             *
 *    This will assign the specified destination to the unit. It is presumed that doing is     *
 *    is all that is needed in order to cause the unit to move to the specified destination.   *
 *                                                                                             *
 * INPUT:   target   -- The target (location) to move to.                                      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/09/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void UnitClass::Assign_Destination(AbstractClass * target, bool immediate)
{
	/*
	**	Abort early if there is anything wrong with the parameters
	**	or the unit already is assigned the specified destination.
	*/
	if (target == NavCom) return;

	/*
	**	Transport vehicles must tell all passengers that are about to load, that they
	**	cannot proceed. This is accomplished with a radio message to this effect.
	*/
	if (In_Radio_Contact() && Class->Max_Passengers() > 0 && Contact_With_Whom()->Fetch_RTTI() == RTTI_INFANTRY) {
		Transmit_Message(RADIO_OVER_OUT);
	}

	/*
	 * If tethered to a carryall while sleeping, untether before moving.
	 */
	if (target != NULL && In_Radio_Contact() && Mission == MISSION_SLEEP && IsTethered && Contact_With_Whom()->RTTI == RTTI_AIRCRAFT) {
		AircraftClass * a = (AircraftClass *)Contact_With_Whom();
		if (a->Class->IsCarryall) {
			a->Assign_Destination(NULL);
			Transmit_Message(RADIO_UNTETHER);
			Transmit_Message(RADIO_OVER_OUT);
		}
	}

	BuildingClass * b = dynamic_cast<BuildingClass *>(target);

	/*
	**	Handle entry logic here.
	*/
	if (Mission == MISSION_ENTER || MissionQueue == MISSION_ENTER) {

		/*
		**	If not already in radio contact (presumed with the transport), then
		**	either try to establish contact if allowed, or just move close and
		**	wait until radio contact can be established.
		*/
		if (!In_Radio_Contact()) {
			if (b != NULL) {

				/*
				**	Determine if the transport is already in radio contact. If so, then just move
				**	toward the transport and try to establish contact at a later time.
				*/
				if (b->In_Radio_Contact()) {
					ArchiveTarget = target;

					/*
					**	HACK ALERT: The repair bay is counting on the assignment of the NavCom by this routine.
					**	The refinery must NOT have the navcom assigned by this routine.
					*/
					if (b->Class->IsCanUnitRepair) {
						target = NULL;
						NearbyObject = b;
					}
				} else {
					AbstractClass * oldnav = NavCom;
					if (Transmit_Message(RADIO_DOCKING, b) != RADIO_ROGER) {
						Transmit_Message(RADIO_OVER_OUT);
						if (b->Class->IsCanUnitRepair) {
							ArchiveTarget = target;
						}
					} else {
						if (b->Class->IsDockUnload && NavCom != oldnav) {
							target = NavCom;
						}
					}
				}
			} else {
				TechnoClass * techno = Dynamic_Cast<TechnoClass *>(target);
				if (techno != NULL) {

					/*
					**	Determine if the transport is already in radio contact. If so, then just move
					**	toward the transport and try to establish contact at a later time.
					*/
					if (techno->In_Radio_Contact()) {
						ArchiveTarget = target;
					} else {
						if (Transmit_Message(RADIO_HELLO, techno) == RADIO_ROGER) {
							if (Transmit_Message(RADIO_DOCKING) == RADIO_ROGER) {
								return;
							}
							Transmit_Message(RADIO_OVER_OUT);
						}
					}
				}
			}
		} else {
			Path[0] = FACING_NONE;
		}
	} else {
		Path[0] = FACING_NONE;
	}

	bool useplan = false;

	/*
	 * Subterranean units prepend their navigation target onto the queue and
	 * re-target the nearest reachable cell when driving rather than burrowing.
	 */
	if (target != NULL && Class->IsSubterranean && Locomotion->Is_Moving()) {
		IPersistPtr persist(Locomotion);
		CLSID clsid;
		persist->GetClassID(&clsid);
		if (clsid == CLSID_DriveLocomotion) {
			NavQueue.Add_Head(target);
			RouteQueue.Clear();
			CellClass * tcell = Get_Target_Cell_Ptr();
			target = tcell;
			immediate = false;
			if (tcell != NULL && tcell->IsUnderBridge && !Is_Moving_Onto_Bridge()) {
				Cell nearby = Map.Nearby_Location(tcell->CellID, SPEED_TRACK, Map.Get_Cell_Zone(tcell->CellID, Class->MZone, false), Class->MZone, false, Point2D(1, 1), false, true, false, false);
				if (nearby != CELL_NONE) {
					target = &Map[nearby];
				}
			}
		}
	}

	bool needsswap = false;

	if (target != NULL) {
		bool surface = false;

		/*
		 * A subterranean unit that is told to move immediately must decide whether
		 * it can burrow straight to the destination or has to plan a surface route.
		 */
		if (Class->IsSubterranean && immediate) {
			bool isbridge = Is_Moving_Onto_Bridge();

			Cell from = Destination_Coord().As_Cell();
			bool canburrowfrom = Map[from].Can_Burrow_Here();

			Cell to = target->Center_Coord().As_Cell();
			CellClass * tocell = &Map[to];
			bool isunderbridgeto = tocell->IsUnderBridge;
			bool canburrowto = tocell->Can_Burrow_Here();
			int height = Get_Height_AGL();

			if (isbridge) {
				if (isunderbridgeto) {
					int z1 = Map.Zone_Connection_Index(from, 3, 0);
					if (z1 == Map.Zone_Connection_Index(to, 3, 0) && z1 != -1) {
						if (!Map.ZoneConnections[z1].IsPassable) {
							Cell d1 = Map.Get_Zone_Connection_Destination(from, from);
							Cell d2 = Map.Get_Zone_Connection_Destination(to, to);
							if (d1.Distance_To(d2) <= 2) {
								needsswap = true;
							}
						} else {
							needsswap = true;
						}
					}
				}
				if (!needsswap) {
					useplan = true;
				}
			} else if (isunderbridgeto) {
				useplan = true;
			}

			if (((!canburrowfrom && height >= 0) || !canburrowto) && !needsswap || useplan) {
				target = Plan_Route(*target);
				surface = true;
			}
		}

		/*
		 * The non-subterranean (and the no-plan subterranean) immediate path clears
		 * the route queue before surfacing a tunneler that is on its way up.
		 */
		if (!surface) {
			if (immediate) {
				RouteQueue.Clear();
				surface = true;
			}
		}

		/*
		 * Surfacing tunneler keeps its current cell at the route front.
		 */
		if (surface) {
			if (Locomotion->Is_Surfacing()) {
				RouteQueue.Add_Head(target);
				target = &Map[Get_Coord()];
			}
		}

		/*
		 * If the locomotor is a tunnel locomotor and the unit is on the ground,
		 * surface the tunneler by swapping the tunnel locomotor for a drive one.
		 * (Mirrors BuildingClass weapons-factory exit, building.cpp:6236-6251.)
		 */
		if (target != NULL && !Locomotion->Is_Moving()) {
			IPersistPtr persist(Locomotion);
			CLSID clsid;
			persist->GetClassID(&clsid);
			if (clsid == CLSID_TunnelLocomotion && Get_Height_AGL() == 0) {
				Coord tc = target->Center_Coord();
				int gl = Map.Get_Height_GL(tc);
				if (tc.Z < gl) tc.Z = gl;

				bool doswap = needsswap;
				if (!doswap) {
					if (!Is_Moving_Onto_Bridge()
						&& !Map[target->Center_Coord()].IsUnderBridge
						&& Map[Destination_Coord()].Can_Burrow_Here()
						&& Map[target->Center_Coord()].Can_Burrow_Here()) {
						Cell to = target->Center_Coord().As_Cell();
						if (!Is_Route_Broken(Get_Target_Cell(), to)) {
							doswap = true;
						}
					} else {
						doswap = true;
					}
				}

				if (doswap) {
					IPiggybackPtr piggy(Locomotion);
					if (piggy != NULL && piggy->Is_Piggybacking()) {
						piggy->End_Piggyback(&Locomotion);
					}
					ILocomotionPtr walk(CLSID_DriveLocomotion);
					walk->Link_To_Object(this);
					piggy = IPiggybackPtr(walk);
					if (piggy != NULL) {
						piggy->Begin_Piggyback(Locomotion);
						Locomotion = walk;
						Locomotion->Force_New_Slope(Map[Get_Coord()].Ramp);
					}
				}
			}
		}
	} else {
		RouteQueue.Clear();
	}

	/*
	**	If the player clicked on a friendly repair facility and the repair
	**	facility is currently not involved with some other unit (radio or unloading).
	*/
	if (b != NULL && b->Class->IsCanUnitRepair) {
		if (b->In_Radio_Contact() && b->Contact_With_Whom() != this) {
			ArchiveTarget = target;
		} else {

			/*
			**	Establish radio contact protocol. If the facility responds correctly,
			**	then remain in radio contact and proceed toward the desired destination.
			*/
			if (Transmit_Message(RADIO_HELLO, b) == RADIO_ROGER) {
				if (Transmit_Message(RADIO_DOCKING) == RADIO_ROGER) {
					BASECLASS::Assign_Destination(target, immediate);
					Path[0] = FACING_NONE;
					return;
				}

				/*
				**	Failure to establish a docking relationship with the refinery.
				**	Bail & await further instructions.
				*/
				Transmit_Message(RADIO_OVER_OUT);
				target = NULL;
			}
		}
	}

	/*
	 * A weapons factory that this unit is exiting wants the unit driven to its
	 * exit cell before anything else; queue the real destination behind it.
	 */
	BuildingClass * wf = (BuildingClass *)Contact_With_Whom();
	if (wf != NULL && wf->Fetch_RTTI() == RTTI_BUILDING && wf->Class->IsWeaponsFactory) {
		Cell exitcell = wf->Get_Coord().As_Cell() + Cell(3, 1);
		if (target != &Map[exitcell]) {
			NavQueue.Clear();
			RouteQueue.Clear();
			if (target != NULL) {
				Queue_Navigation_List(target);
			}
			return;
		}
	}

	/*
	 * Harvesters and weeders automatically enter their dock building when the
	 * clicked target is that building (and the houses are allied).
	 */
	if (b != NULL && Class->Dock.Count() > 0 && b->Class == (BuildingTypeClass *)Class->Dock[0] &&
		Fetch_RTTI() == RTTI_UNIT && (Class->IsToHarvest || Class->IsToVeinHarvest) &&
		Mission != MISSION_UNLOAD && b->House->Is_Ally(House) && House->Is_Ally(b->House)) {

		if (Contact_With_Whom() != b && !b->In_Radio_Contact() && Transmit_Message(RADIO_HELLO, b) == RADIO_ROGER &&
			Mission != MISSION_ENTER && Mission != MISSION_HARVEST) {
			Assign_Mission(MISSION_ENTER);
			target = NULL;
		} else {
			if (target != NULL && target->Fetch_RTTI() == RTTI_BUILDING && Contact_With_Whom() != target) {
				Cell nearby = Map.Nearby_Location(b->Get_Coord().As_Cell(), SPEED_WHEEL, Map.Get_Cell_Zone(b->Get_Coord().As_Cell(), Class->MZone, false), Class->MZone, false, Point2D(1, 1), false, true, false, false);
				if (nearby != CELL_NONE) {
					target = &Map[nearby];
				} else {
					target = NULL;
				}
			}
		}
	}

	/*
	 * If the locomotor is unpowered and we are not in an ion storm, power it
	 * back on when an adjacent friendly repair facility occupies our cell.
	 */
	if (!Locomotion->Is_Powered() && !IonStormClass::Is_Ion_Storm_Active()) {
		CellClass * cellptr = Get_Cell_Ptr();
		if (!cellptr->IsUnderBridge) {
			ObjectClass * occupier = cellptr->Cell_Occupier();
			while (occupier != NULL) {
				if (occupier != this && occupier->Fetch_RTTI() == RTTI_BUILDING && ((BuildingClass *)occupier)->Class->IsCanUnitRepair) {
					Locomotion->Power_On();
					break;
				}
				occupier = occupier->Next;
			}
		}
	}

	BASECLASS::Assign_Destination(target, immediate);
}


/***********************************************************************************************
 * UnitClass::Greatest_Threat -- Fetches the greatest threat for this unit.                    *
 *                                                                                             *
 *    This routine will search the map looking for a good target to attack. It takes into      *
 *    consideration the type of weapon it is equipped with.                                    *
 *                                                                                             *
 * INPUT:   threat   -- The threat type to search for.                                         *
 *                                                                                             *
 * OUTPUT:  Returns with a target value of the target that this unit should pursue. If there   *
 *          is no suitable target, then TARGET_NONE is returned.                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/09/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
AbstractClass * UnitClass::Greatest_Threat(ThreatType threat, Coord const & coord, bool onlyenemy) const
{
	if (House->Is_Human_Player() && Class->IsDeployToFire) {
		return(NULL);
	}
	if (!(threat & (THREAT_INFANTRY|THREAT_VEHICLES|THREAT_BUILDINGS|THREAT_TIBERIUM|THREAT_CIVILIANS|THREAT_POWER|THREAT_FACTORIES|THREAT_BASE_DEFENSE))) {
		if (PrimaryWeapon != NULL) {
			threat = ThreatType(threat | PrimaryWeapon->Allowed_Threats());
		}
		if (SecondaryWeapon != NULL) {
			threat = ThreatType(threat | SecondaryWeapon->Allowed_Threats());
		}
	}

	return(BASECLASS::Greatest_Threat(threat, coord, onlyenemy));
}


/***********************************************************************************************
 * UnitClass::Read_INI -- Reads units from scenario INI file.                                  *
 *                                                                                             *
 *    This routine is used to read all the starting units from the                             *
 *    scenario control INI file. The units are created and placed on the                       *
 *    map by this routine.                                                                     *
 *                                                                                             *
 *    INI entry format:                                                                        *
 *      Housename, Typename, Strength, Coord, Facingnum, Missionname, Triggername              *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the loaded scenario INI file.                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void UnitClass::Read_INI(CCINIClass const & ini)
{
	UnitClass	* unit;         // Working unit pointer.
	UnitType		classid;    // Unit class.
	char			buf[128];
	int len = ini.Entry_Count(INI_NAME);
	std::vector<int> followers(len, -1);
	std::vector<UnitClass *> source_units(len, nullptr);

	for (int index = 0; index < len; index++) {
		char const * entry = ini.Get_Entry(INI_NAME, index);

		ini.Get_String(INI_NAME, entry, NULL, buf, sizeof(buf));

		HouseClass * inhousep = House_From_Name(strtok(buf, ","));
		if (inhousep != NULL) {
			classid = UnitTypeClass::From_Name(strtok(NULL, ","));

			if (classid != UNIT_NONE) {

				unit = new UnitClass(UnitTypes[classid], inhousep);
				if (unit != NULL) {

					/*
					**	Read the raw data.
					*/
					int strength = atoi(strtok(NULL, ","));

					Cell cell;
					Coord coord;
					if (NewINIFormat >= 4) {
						unsigned short x = atoi(strtok(NULL, ","));
						unsigned short y = atoi(strtok(NULL, ","));
						cell = Cell(x,y);
					} else {
						int c = atoi(strtok(NULL, ","));
						cell = Cell(c);
					}

					coord = cell;

					Dir256 dir = (Dir256)atoi(strtok(NULL, ","));
					MissionType mission = MissionClass::Mission_From_Name(strtok(NULL, ","));

					TagTypeClass * tp = TagTypeClass::From_Name(strtok(NULL,","));
					if (tp != NULL) {
						TagClass * tt = Find_Or_Make(tp);
						if (tt != NULL) {
							unit->Attach_Tag(tt);
						}
					}

					char * token = strtok(NULL, ",");
					if (token != NULL) {
						unit->Veterancy.From_Integer(atoi(token));
					}

					token = strtok(NULL, ",");
					if (token != NULL) {
						unit->Group = atoi(token);
					}

					token = strtok(NULL, ",");
					if (token != NULL) {
						unit->IsOnBridge = atoi(token) != 0;
						if (unit->IsOnBridge) {
							coord.Z = Map.Get_Height_GL(coord) + BRIDGE_LEPTON_HEIGHT;
						}
					}

					token = strtok(NULL, ",");
					if (token != NULL) {
						followers[index] = atoi(token);
					}

					token = strtok(NULL, ",");
					if (token != NULL) {
						unit->IsTeamRecruitable = atoi(token) != 0;
					}

					token = strtok(NULL, ",");
					if (token != NULL) {
						unit->IsAutocreateRecruitable = atoi(token) != 0;
					}

					if (unit->Unlimbo(coord, dir)) {
						unit->Strength = unit->Class->MaxStrength * (double)strength / 256.0;
						if (unit->Strength > unit->Class->MaxStrength-3) unit->Strength = unit->Class->MaxStrength;
						if (unit->Strength == 0) unit->Strength = 1;
						if (Session.Type == GAME_NORMAL || unit->House->Is_Human_Player()) {
							unit->Assign_Mission(mission);
							if (unit->Ready_To_Commence()) {
								unit->Commence();
							}
						} else {
							unit->Enter_Idle_Mode();
						}
						source_units[index] = unit;

					} else {

						/*
						**	If the unit could not be unlimboed, then this is a catastrophic error
						**	condition. Delete the unit.
						*/
						delete unit;
					}
				}
			}
		}
	}

	for (int index = 0; index < len; index++) {
		UnitClass * unit = source_units[index];
		if (unit != NULL) {
			int followerid = followers[index];
			if (followerid >= 0 && followerid < len && source_units[followerid] != NULL) {
				unit->FollowingMe = source_units[followerid];
				source_units[followerid]->IsFollowing = true;
			} else {
				unit->FollowingMe = NULL;
			}
		}
	}
}


/***********************************************************************************************
 * UnitClass::Write_INI -- Store the units to the INI database.                                *
 *                                                                                             *
 *    This routine will store all the unit data to the INI database.                           *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database object to store to.                         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void UnitClass::Write_INI(CCINIClass & ini)
{
	/*
	**	First, clear out all existing unit data from the ini file.
	*/
	ini.Clear(INI_NAME);

	/*
	**	Write the unit data out.
	*/
	for (int index = 0; index < Units.Count(); index++) {
		UnitClass * unit = Units[index];
		if (unit != NULL && !unit->IsInLimbo && unit->IsActive) {
			char	uname[10];
			char	buf[128];

			sprintf(uname, "%d", index);
			sprintf(buf, "%s,%s,%d,%d,%d,%d,%s,%s,%d,%d,%d,%d,%d,%d",
				(char const *)unit->House->Class->IniName,
				(char const *)unit->Class->IniName,
				(int)(unit->HealthRatio*256),
				unit->PositionCell.X,
				unit->PositionCell.Y,
				unit->PrimaryFacing.Current().As_Dir256(),
				MissionClass::Mission_Name(unit->Mission),
				(unit->Tag != NULL && unit->Tag->Class != NULL) ? (char const *)unit->Tag->Class->IniName : "None",
				unit->Veterancy.To_Integer(),
				unit->Group,
				unit->IsOnBridge,
				(unit->FollowingMe != NULL) ? Units.ID(unit->FollowingMe) : -1,
				unit->IsTeamRecruitable,
				unit->IsAutocreateRecruitable
				);
			ini.Put_String(INI_NAME, uname, buf);
		}
	}
}


/***********************************************************************************************
 * UnitClass::Credit_Load -- Fetch the full credit value of cargo carried.                     *
 *                                                                                             *
 *    This will determine the value of the cargo carried (limited to considering only gold     *
 *    and gems) and return that value. Use this to determine how 'valuable' a harvester is.    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the credit value of the cargo load of this unit (harvester).          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int UnitClass::Credit_Load(void) const
{
	return(Storage.Get_Total_Value());
}


/***********************************************************************************************
 * UnitClass::Should_Crush_It -- Determines if this unit should crush an object.               *
 *                                                                                             *
 *    Call this routine to determine if this unit should crush the object specified. The       *
 *    test for crushable action depends on proximity and ability of the unit. If a unit        *
 *    should crush the object, then it should be given a movement order to enter the cell      *
 *    where the object is located.                                                             *
 *                                                                                             *
 * INPUT:   it -- The object to see if it should be crushed.                                   *
 *                                                                                             *
 * OUTPUT:  bool; Should "it" be crushed by this unit?                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool UnitClass::Should_Crush_It(TechnoClass const * it) const
{
	/*
	**	If this unit cannot crush anything or the candidate object cannot be crushed,
	**	then it obviously should not try to crush it -- return negative answer.
	*/
	if (!(Class->IsCrusher || Has_Ability(ABILITY_CRUSHER)) || it == NULL || !it->TClass->IsCrushable) return(false);

	/*
	**	Objects that are far away should really be fired upon rather than crushed.
	*/
	if (Distance_To(it) > Rule->CrushDistance) return(false);

	/*
	**	Human controlled units don't automatically crush. Neither do computer controlled ones
	**	if they are at difficult setting.
	*/
	if ((House->Is_Human_Player() && !Rule->IsAutoCrush && !Class->IsAutoCrush) || House->Difficulty == DIFF_HARD) return(false);

	/*
	**	If the house IQ indicates that crushing should not be allowed, then don't
	**	suggest that crushing be done.
	*/
	if (!House->Is_Human_Player() && House->IQ < Rule->IQCrush) return(false);

	/*
	**	Don't allow crushing of spies by computer-controlled vehicles.
	*/
	if (it->RTTI == RTTI_INFANTRY && ((InfantryClass *)it)->Class->IsDisguised) {
		return(false);
	}

	return(true);
}


/***********************************************************************************************
 * UnitClass::Scatter -- Causes the unit to scatter to a nearby location.                      *
 *                                                                                             *
 *    This scatter logic will actually look for a nearby location rather than an adjacent      *
 *    free location. This is necessary because sometimes a unit is required to scatter more    *
 *    than one cell. A vehicle on a service depot is a prime example.                          *
 *                                                                                             *
 * INPUT:   threat   -- The coordinate that a potential threat resides. If this is a non       *
 *                      threat related scatter, then this parameter will be zero.              *
 *                                                                                             *
 *          forced   -- Should the scatter be performed even if it would be otherwise          *
 *                      inconvenient?                                                          *
 *                                                                                             *
 *          nokidding-- Should the scatter be performed even if it would otherwise be          *
 *                      illegal?                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void UnitClass::Scatter(Coord const & threat, bool forced, bool nokidding)
{
	if (!Can_Scatter()) return;

	/*
	**	Certain missions prevent scattering regardless of whether it would be
	**	a good idea or not.
	*/
	if (!Current_Mission_Control().IsScatter && !forced) return;

	if (PrimaryFacing.Is_Rotating()) return;

	if (NavCom != NULL && !nokidding) return;

	if (threat == COORD_NONE) {
		Cell nearby = Map.Nearby_Location(Destination_Coord().As_Cell(), Class->Speed, -1, MZONE_NORMAL, IsOnBridge, Point2D(1,1), false, true);
		if (nearby != CELL_NONE) {
			Assign_Destination(&Map[nearby]);
		}
		return;
	}

	/*
	**	Certain missions prevent scattering regardless of whether it would be
	**	a good idea or not.
	*/
	if (Current_Mission_Control().IsParalyzed) return;

	if ((Fetch_RTTI() != RTTI_UNIT || !((UnitClass *)this)->IsDumping) && (NavCom == NULL || (nokidding && !IsRotating))) {
		if (TarCom == NULL || forced || Random_Pick(1, 4) == 1) {
			FacingType	toface;

			if (threat != COORD_NONE) {
				toface = Dir_Facing(::Direction(threat, PositionCoord));
				toface = Facing_Add(toface, Random_Pick(FACING_0, FACING_90)-FACING_45);
			} else {
				toface = (PrimaryFacing.Current().As_Dir8());
				toface = Facing_Add(toface, Random_Pick(FACING_0, FACING_90)-FACING_45);
			}

			Cell altcell(0, 0);
			Cell newcell(0, 0);
			Cell destcell(Destination_Coord());

			CellClass *cellptr = &Map[destcell];

			int z = cellptr->Height + Is_Moving_Onto_Bridge() ? BRIDGE_CELL_HEIGHT : 0;
			Coord destcoord = Destination_Coord();
			destcoord.Z = z * LEVEL_LEPTON_H;

			FacingType face;
			for (face = FACING_N; face < FACING_COUNT; face++) {
				FacingType newface = Facing_Add(toface, face);
				Cell checkcell = Adjacent_Cell(destcell, newface);
				CellClass *cptr = &Map[checkcell];

				if (Map.In_Local_Radar(checkcell) && Can_Enter_Cell(cptr, newface, Get_Cell_Height()) == MOVE_OK) {
					if (altcell == CELL_NONE) altcell = checkcell;
					if (CELL_NONE == Cell(0,0)) {
						Coord checkcoord = checkcell.As_Coord();
						checkcoord.Z = destcoord.Z;
						if (checkcell == TacticalMap->Coord_To_Cell(checkcoord) && !Map[checkcell].IsUnderBridge) {
							newcell = checkcell;
							break;
						}
					}
				}
			}

			Cell destination = newcell;
			if (destination == CELL_NONE) {
				destination = altcell;
			}

			if (destination != CELL_NONE) {
				Assign_Mission(MISSION_MOVE);
				Assign_Destination(&Map[destination]);
			}
		}
	}
}


/***********************************************************************************************
 * UnitClass::Limbo -- Limbo this unit.                                                        *
 *                                                                                             *
 *    This will cause the unit to go into a limbo state. If it was carrying a flag, then       *
 *    the flag will be dropped where the unit is at.                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was this unit limboed?                                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/08/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool UnitClass::Limbo(void)
{
	if (BASECLASS::Limbo()) {
		if (Flagged != HOUSE_NONE) {
			Houses[Flagged]->Flag_Attach(Get_Cell());
			Flagged = HOUSE_NONE;
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * UnitClass::Mission_Guard_Area -- Guard area logic for units.                                *
 *                                                                                             *
 *    This logic is similar to normal guard area except that APCs owned by the computer will   *
 *    try to load up with nearby infantry. This will give the computer some fake intelligence  *
 *    when playing in skirmish mode.                                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the delay to use before calling this routine again.                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int UnitClass::Do_MISSION_GUARD_AREA(void)
{
	return(BASECLASS::Do_MISSION_GUARD_AREA());
}


/// <summary>
/// Is this unit free to start moving?
/// A transport must have its door settled shut before it may drive off; one that is open
/// or still animating holds the vehicle where it is.
/// </summary>
/// <returns>bool; May the unit begin moving?</returns>
bool UnitClass::Is_Ready_To_Move(void) const
{
	if (Door.Is_Ready_To_Open()) {
		return(true);
	}

	return(false);
}


/// <summary>
/// Marks the cell at the coordinate specified as occupied by this unit.
/// A vehicle riding a bridge claims the bridge deck rather than the ground below it.
/// </summary>
/// <param name="coord">The coordinate of the cell to occupy.</param>
void UnitClass::Set_Occupy_Bit(Coord const & coord)
{
	CellClass &cell = Map[coord];
	if (Map.Get_Height_GL(coord) + BRIDGE_LEPTON_HEIGHT <= coord.Z && cell.IsUnderBridge) {
		cell.BridgeFlag.Occupy.Vehicle = true;
	} else {
		cell.Flag.Occupy.Vehicle = true;
	}
}


/// <summary>
/// Releases this unit's claim on the cell at the coordinate specified.
/// A vehicle riding a bridge gives up the bridge deck rather than the ground below it.
/// </summary>
/// <param name="coord">The coordinate of the cell to release.</param>
void UnitClass::Clear_Occupy_Bit(Coord const & coord)
{
	CellClass &cell = Map[coord];
	if (Map.Get_Height_GL(coord) + BRIDGE_LEPTON_HEIGHT <= coord.Z && cell.IsUnderBridge) {
		cell.BridgeFlag.Occupy.Vehicle = false;
	} else {
		cell.Flag.Occupy.Vehicle = false;
	}
}


/// <summary>
/// May this unit be handed its next order now?
/// The mission system consults this routine before starting a queued mission. A vehicle
/// that is still rolling, still unloading, or still being disgorged by a war factory is
/// left to finish what it is doing.
/// </summary>
/// <returns>bool; Is the unit ready to begin its next mission?</returns>
bool UnitClass::Ready_To_Commence(void)
{
	if (CurrentMission == MISSION_STICKY || CurrentMission == MISSION_RESCUE) {
		return(false);
	}

	if (IsDumping) {
		return(false);
	}

	if (MissionQueue != MISSION_ENTER) {
		if (Locomotion->Is_Moving_Now() && HeightAGL >= 0 &&
			Mission != MISSION_GUARD && (Mission != MISSION_ATTACK || TarCom != NULL) && !IsMissionUnloadStandby) {

			return(false);
		}
	}

	if (!Door.Is_Door_Closed()) {
		return(false);
	}

	RadioClass * radio = Contact_With_Whom();
	if (radio != NULL) {
		if (radio->RTTI == RTTI_BUILDING && ((BuildingClass *)radio)->Class->IsWeaponsFactory && MissionQueue != MISSION_MOVE) {
			return(false);
		}
	} else {
		BuildingClass * building = Map[Get_Coord()].Cell_Building();
		if (building != NULL && building->Class->IsWeaponsFactory && (PositionCoord.As_Cell() - building->PositionCoord.As_Cell() == Cell(0, 1))) {
			return(false);
		}
	}

	return(true);
}


/// <summary>
/// Reads this unit back in from a save game stream.
/// The unit is withdrawn from the target tracker under the identity it is carrying now,
/// since the one it is about to be given is the one it was saved with. Post_Load enters it
/// again once that identity has arrived.
/// </summary>
/// <param name="stream">The stream to read this unit from.</param>
/// <returns>Returns with S_OK if the unit was read successfully.</returns>
HRESULT STDMETHODCALLTYPE UnitClass::Load(IStream *stream)
{
	TargetTracker.Remove_Index(Fetch_ID());
	return(BASECLASS::Load(stream));
}


/// <summary>
/// Lists the members this unit carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void UnitClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(FiringSyncDelay);
	stream.Serialize(Reload);
	stream.Serialize(Class);
	stream.Serialize(FollowingMe);
	stream.Serialize(Flagged);
	stream.Serialize(IsFollowing);
	stream.Serialize(IsDumping);
	stream.Serialize(IsHarvesting);
	stream.Serialize(IsCompositingToEightBitSurface);
	stream.Serialize(VisceroidFacing);
	stream.Serialize(Charge);
	stream.Serialize(DeathCounter);
	stream.Serialize(Unused1);
}


/// <summary>
/// Enters this unit in the target tracker under the identity it was saved with.
/// </summary>
void UnitClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	TargetTracker.Add_Index(Fetch_ID(), this);
}


/// <summary>
/// Adds this unit's state to the running game state checksum.
/// The network code compares these checksums between machines in order to detect a game
/// that has fallen out of sync.
/// </summary>
/// <param name="crc">The checksum engine to submit this unit's state to.</param>
void UnitClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);

	crc((int)Reload);
	crc(Class->Fetch_RTTI());
	crc(Class->Fetch_ID());
	if (FollowingMe != NULL) {
		crc(FollowingMe->Fetch_ID());
	}
	crc(Flagged);
	crc(IsFollowing);
	crc(IsDumping);
	crc(IsHarvesting);
	crc(IsToScatter);
}


/// <summary>
/// Removes all references this unit holds to the object specified.
/// This routine is called when an object is about to disappear, so that no dangling
/// pointer to it survives anywhere in the game.
/// </summary>
/// <param name="target">The object that is going away.</param>
/// <param name="all">Should even the passive references be severed?</param>
void UnitClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);
	if (FollowingMe == target) {
		FollowingMe = NULL;
	}
	if (Class == target) {
		Class = NULL;
	}
}


/// <summary>
/// Records the destruction of this unit.
/// This routine springs whatever destruction events are hanging off the unit's tag before
/// handing the kill on for the usual scoring bookkeeping.
/// </summary>
/// <param name="source">The object responsible for the kill, or NULL if there was none.</param>
void UnitClass::Record_The_Kill(TechnoClass * source)
{
	if (Tag != NULL) {
		if (EnteredByInfType == INFANTRY_NONE || !Tag->Is_To_Inherit()) {
			if (source != NULL) {
				Tag->Spring(TEVENT_DESTROYED, this);
			}
			if (Tag != NULL) {
				Tag->Spring(TEVENT_DESTROYED_ANY, this);
			}
			if (Tag != NULL) {
				Tag->Spring(TEVENT_DESTROYED_ANY_X, this);
			}
		}
	}
	BASECLASS::Record_The_Kill(source);
}


/***********************************************************************************************
 * UnitClass::Mission_Attack -- AI for heading towards and firing upon target.                 *
 *                                                                                             *
 *    This AI routine handles heading to within range of the target and then firing upon       *
 *    it until it is destroyed. If the target is destroyed, then the unit will change          *
 *    missions to match its "idle mode" of operation (usually guarding).                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the delay before calling this routine again.                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int UnitClass::Do_MISSION_ATTACK(void)
{
	return(BASECLASS::Do_MISSION_ATTACK());
}


/***********************************************************************************************
 * UnitClass::Weed_Load -- Determine the Weed load as a percentage.                            *
 *                                                                                             *
 *    Use this routine to determine what the Weed load is (as a floating point percentage).    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current "fullness" rating for the object.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/17/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
double UnitClass::Weed_Load(void) const
{
	if (Class->IsToVeinHarvest) {
		return((double)Storage.Get_Total_Amount() / Class->Capacity);
	}
	return(0.0);
}


/// <summary>
/// Fetches the maximum speed this unit may travel at.
/// A limpet drone clamped to the vehicle drags its top speed down; otherwise this is the
/// normal speed for any foot object.
/// </summary>
int UnitClass::Get_Max_Speed(void) const
{
	int speed = BASECLASS::Get_Max_Speed();
	if (LimpetType) {
		speed = (int)(speed * LimpetSpeedFactor);
	}
	return(speed);
}


/// <summary>
/// Plans a route from the unit's destination cell to the given target. When the journey
/// crosses movement zones (under-bridge destinations, burrowable start or end cells,
/// bridge approaches), the intermediate travel legs (bridge entry/exit cells, zone
/// connection cells, nearest reachable cells) are queued in RouteQueue ahead of the
/// target itself. The first leg is then popped and returned as the immediate destination.
/// </summary>
/// <param name="object">The desired final movement target.</param>
/// <returns>The first leg of the planned route, or NULL if no route is possible.</returns>
AbstractClass * UnitClass::Plan_Route(AbstractClass const & object) const
{
	DynamicVectorClass<AbstractClass *> & route = ((UnitClass *)this)->RouteQueue;
	route.Clear();

	bool onbridge = Is_Moving_Onto_Bridge();

	Cell from = Destination_Coord().As_Cell();

	bool canburrowfrom = Map[from].Can_Burrow_Here() || Get_Height_AGL() < 0;

	AbstractClass * dest = (AbstractClass *)&object;
	Cell to = dest->Center_Coord().As_Cell();

	CellClass * toptr = &Map[to];
	bool isunderbridgeto = toptr->IsUnderBridge;
	bool canburrowto = toptr->Can_Burrow_Here();

	if (onbridge) {
		if (isunderbridgeto) {
			Cell frombridge = Map.Get_Zone_Connection_Destination(from, to);
			Cell tobridge = Map.Get_Zone_Connection_Destination(to, to);

			if (Is_Route_Broken(frombridge, tobridge)) {
				Cell fromspot = Map.Nearby_Location(frombridge, SPEED_TRACK, Map.Get_Cell_Zone(frombridge), MZONE_NORMAL, false, Point2D(1, 1), false, true, true);
				Cell tospot = Map.Nearby_Location(tobridge, SPEED_TRACK, Map.Get_Cell_Zone(frombridge), MZONE_NORMAL, false, Point2D(1, 1), false, true, true);

				if (fromspot != CELL_NONE && tospot != CELL_NONE) {
					route.Add(&Map[fromspot]);
					route.Add(&Map[tospot]);

				} else if (Map.Is_Same_Cell_Zone(from, to, MZONE_NORMAL, true, true, false)) {
					if (frombridge != from) {
						route.Add(&Map[frombridge]);
					}
					route.Add(&Map[tobridge]);

				} else if (fromspot != CELL_NONE) {
					tospot = Map.Nearby_Location(to, SPEED_TRACK, -1, MZONE_NORMAL, true, Point2D(1, 1), false, false, true);
					if (tospot != CELL_NONE && fromspot != tospot) {
						route.Add(&Map[fromspot]);
						dest = &Map[tospot];
					} else {
						Cell fallback = Map.Nearby_Location(to, SPEED_TRACK, Map.Get_Cell_Zone(from, MZONE_NORMAL, true), MZONE_NORMAL, true, Point2D(1, 1), false, true, false);
						if (fallback == CELL_NONE || Get_Height_AGL() < 0) {
							return(NULL);
						}
						dest = &Map[fallback];
					}

				} else {
					Cell fallback = Map.Nearby_Location(to, SPEED_TRACK, Map.Get_Cell_Zone(from, MZONE_NORMAL, true), MZONE_NORMAL, true, Point2D(1, 1), false, true, false);
					if (fallback == CELL_NONE || Get_Height_AGL() < 0) {
						return(NULL);
					}
					dest = &Map[fallback];
				}

			} else {
				if (frombridge != from) {
					route.Add(&Map[frombridge]);
				}
				route.Add(&Map[tobridge]);
			}

		} else if (canburrowto) {
			Cell frombridge = Map.Get_Zone_Connection_Destination(from, to);

			if (Is_Route_Broken(frombridge, to)) {
				Cell fromspot = Map.Nearby_Location(frombridge, SPEED_TRACK, Map.Get_Cell_Zone(frombridge), MZONE_NORMAL, false, Point2D(1, 1), false, true, true);

				if (fromspot != CELL_NONE) {
					route.Add(&Map[fromspot]);

				} else if (Map.Is_Same_Cell_Zone(from, to, MZONE_NORMAL, true, false, false)) {
					if (frombridge != from) {
						route.Add(&Map[frombridge]);
					}

				} else {
					Cell fallback = Map.Nearby_Location(to, SPEED_TRACK, Map.Get_Cell_Zone(from, MZONE_NORMAL, true), MZONE_NORMAL, true, Point2D(1, 1), false, true, false);
					if (fallback == CELL_NONE || Get_Height_AGL() < 0) {
						return(NULL);
					}
					dest = &Map[fallback];
				}

			} else if (frombridge != from) {
				route.Add(&Map[frombridge]);
			}

		} else {
			Cell frombridge = Map.Get_Zone_Connection_Destination(from, to);

			if (Is_Route_Broken(frombridge, to)) {
				Cell fromspot = Map.Nearby_Location(frombridge, SPEED_TRACK, Map.Get_Cell_Zone(frombridge), MZONE_NORMAL, false, Point2D(1, 1), false, true, true);
				Cell tospot = Map.Nearby_Location(to, SPEED_TRACK, Map.Get_Cell_Zone(to), MZONE_NORMAL, false, Point2D(1, 1), false, true, true);

				if (fromspot != CELL_NONE && tospot != CELL_NONE) {
					route.Add(&Map[fromspot]);
					route.Add(&Map[tospot]);

				} else if (Map.Is_Same_Cell_Zone(from, to, MZONE_NORMAL, true, false, false)) {
					if (frombridge != from) {
						route.Add(&Map[frombridge]);
					}

				} else if (fromspot != CELL_NONE) {
					Cell altdest = Map.Nearby_Location(to, SPEED_TRACK, -1, MZONE_NORMAL, true, Point2D(1, 1), false, true, true);
					if (altdest != CELL_NONE && altdest != fromspot) {
						route.Add(&Map[fromspot]);
						dest = &Map[altdest];
					} else {
						Cell fallback = Map.Nearby_Location(to, SPEED_TRACK, Map.Get_Cell_Zone(from, MZONE_NORMAL, true), MZONE_NORMAL, true, Point2D(1, 1), false, true, false);
						if (fallback == CELL_NONE || Get_Height_AGL() < 0) {
							return(NULL);
						}
						dest = &Map[fallback];
					}

				} else {
					Cell fallback = Map.Nearby_Location(to, SPEED_TRACK, Map.Get_Cell_Zone(from, MZONE_NORMAL, true), MZONE_NORMAL, true, Point2D(1, 1), false, true, false);
					if (fallback == CELL_NONE || Get_Height_AGL() < 0) {
						return(NULL);
					}
					dest = &Map[fallback];
				}

			} else if (frombridge != from) {
				route.Add(&Map[frombridge]);
			}
		}

	} else if (canburrowfrom) {
		if (isunderbridgeto) {
			Cell tobridge = Map.Get_Zone_Connection_Destination(to, to);

			if (Is_Route_Broken(from, tobridge)) {
				Cell tospot = Map.Nearby_Location(tobridge, SPEED_TRACK, Map.Get_Cell_Zone(tobridge), MZONE_NORMAL, false, Point2D(1, 1), false, true, true);

				if (tospot != CELL_NONE) {
					route.Add(&Map[tospot]);

				} else if (Map.Is_Same_Cell_Zone(from, to, MZONE_NORMAL, false, true, false)) {
					if (tobridge != from) {
						route.Add(&Map[tobridge]);
					}

				} else {
					Cell altdest = Map.Nearby_Location(to, SPEED_TRACK, -1, MZONE_NORMAL, false, Point2D(1, 1), false, false, true);
					if (altdest != CELL_NONE) {
						dest = &Map[altdest];
					} else {
						Cell fallback = Map.Nearby_Location(to, SPEED_TRACK, Map.Get_Cell_Zone(from), MZONE_NORMAL, false, Point2D(1, 1), false, true, false);
						if (fallback == CELL_NONE || Get_Height_AGL() < 0) {
							return(NULL);
						}
						dest = &Map[fallback];
					}
				}

			} else if (tobridge != from) {
				route.Add(&Map[tobridge]);
			}

		} else if (!canburrowto && Is_Route_Broken(from, to)) {
			Cell tospot = Map.Nearby_Location(to, SPEED_TRACK, Map.Get_Cell_Zone(to), MZONE_NORMAL, false, Point2D(1, 1), false, true, true);

			if (tospot != CELL_NONE) {
				route.Add(&Map[tospot]);

			} else if (!Map.Is_Same_Cell_Zone(from, to, MZONE_NORMAL, true, true, false)) {
				Cell altdest = Map.Nearby_Location(to, SPEED_TRACK, -1, MZONE_NORMAL, false, Point2D(1, 1), false, false, true);
				if (altdest != CELL_NONE) {
					dest = &Map[altdest];
				} else {
					Cell fallback = Map.Nearby_Location(to, SPEED_TRACK, Map.Get_Cell_Zone(from), MZONE_NORMAL, false, Point2D(1, 1), false, false, false);
					if (fallback == CELL_NONE || Get_Height_AGL() < 0) {
						return(NULL);
					}
					dest = &Map[fallback];
				}
			}
		}

	} else if (isunderbridgeto) {
		Cell tobridge = Map.Get_Zone_Connection_Destination(to, to);

		if (Is_Route_Broken(from, tobridge)) {
			Cell fromspot = Map.Nearby_Location(from, SPEED_TRACK, Map.Get_Cell_Zone(from), MZONE_NORMAL, false, Point2D(1, 1), false, true, true);
			Cell tospot = Map.Nearby_Location(tobridge, SPEED_TRACK, Map.Get_Cell_Zone(tobridge), MZONE_NORMAL, false, Point2D(1, 1), false, true, true);

			if (fromspot != CELL_NONE && tospot != CELL_NONE) {
				route.Add(&Map[fromspot]);
				route.Add(&Map[tospot]);

			} else if (Map.Is_Same_Cell_Zone(from, to, MZONE_NORMAL, false, true, false)) {
				if (tobridge != from) {
					route.Add(&Map[tobridge]);
				}

			} else if (fromspot != CELL_NONE) {
				Cell altdest = Map.Nearby_Location(to, SPEED_TRACK, -1, MZONE_NORMAL, false, Point2D(1, 1), false, false, true);
				if (altdest != CELL_NONE && fromspot != altdest) {
					route.Add(&Map[fromspot]);
					dest = &Map[altdest];
				} else {
					Cell fallback = Map.Nearby_Location(to, SPEED_TRACK, Map.Get_Cell_Zone(from), MZONE_NORMAL, false, Point2D(1, 1), false, false, false);
					if (fallback == CELL_NONE || Get_Height_AGL() < 0) {
						return(NULL);
					}
					dest = &Map[fallback];
				}

			} else {
				Cell fallback = Map.Nearby_Location(to, SPEED_TRACK, Map.Get_Cell_Zone(from), MZONE_NORMAL, false, Point2D(1, 1), false, false, false);
				if (fallback == CELL_NONE || Get_Height_AGL() < 0) {
					return(NULL);
				}
				dest = &Map[fallback];
			}

		} else if (tobridge != from) {
			route.Add(&Map[tobridge]);
		}

	} else if (canburrowto) {
		if (Is_Route_Broken(from, to)) {
			Cell fromspot = Map.Nearby_Location(from, SPEED_TRACK, Map.Get_Cell_Zone(from), MZONE_NORMAL, false, Point2D(1, 1), false, true, true);

			if (fromspot != CELL_NONE) {
				route.Add(&Map[fromspot]);

			} else if (!Map.Is_Same_Cell_Zone(from, to, MZONE_NORMAL, false, false, false)) {
				Cell fallback = Map.Nearby_Location(to, SPEED_TRACK, Map.Get_Cell_Zone(from), MZONE_NORMAL, false, Point2D(1, 1), false, false, false);
				if (fallback == CELL_NONE || Get_Height_AGL() < 0) {
					return(NULL);
				}
				dest = &Map[fallback];
			}
		}

	} else {
		if (Is_Route_Broken(from, to)) {
			Cell fromspot = Map.Nearby_Location(from, SPEED_TRACK, Map.Get_Cell_Zone(from), MZONE_NORMAL, false, Point2D(1, 1), false, true, true);
			Cell tospot = Map.Nearby_Location(to, SPEED_TRACK, Map.Get_Cell_Zone(to), MZONE_NORMAL, false, Point2D(1, 1), false, true, true);

			if (fromspot != CELL_NONE && tospot != CELL_NONE) {
				route.Add(&Map[fromspot]);
				route.Add(&Map[tospot]);

			} else if (!Map.Is_Same_Cell_Zone(from, to, MZONE_NORMAL, true, true, false)) {
				if (fromspot != CELL_NONE) {
					Cell altdest = Map.Nearby_Location(to, SPEED_TRACK, -1, MZONE_NORMAL, false, Point2D(1, 1), false, false, true);
					if (altdest != CELL_NONE && fromspot != altdest) {
						route.Add(&Map[fromspot]);
						dest = &Map[altdest];
					} else {
						Cell fallback = Map.Nearby_Location(to, SPEED_TRACK, Map.Get_Cell_Zone(from), MZONE_NORMAL, false, Point2D(1, 1), false, false, false);
						if (fallback == CELL_NONE || Get_Height_AGL() < 0) {
							return(NULL);
						}
						dest = &Map[fallback];
					}
				} else {
					Cell fallback = Map.Nearby_Location(to, SPEED_TRACK, Map.Get_Cell_Zone(from), MZONE_NORMAL, false, Point2D(1, 1), false, false, false);
					if (fallback == CELL_NONE || Get_Height_AGL() < 0) {
						return(NULL);
					}
					dest = &Map[fallback];
				}
			}
		}
	}

	route.Add(dest);
	AbstractClass * next = RouteQueue[0];
	route.Delete_Index(0);
	return(next);
}


/// <summary>
/// Determines whether a clean direct route exists between two cells, or whether route
/// planning must insert intermediate travel legs. The route is considered broken if the
/// cells are in different movement zones, if the unit is underground, if distant cells
/// fall outside the local radar, or if a trial path walk between them fails. Identical
/// and adjacent cell pairs are never considered broken.
/// </summary>
/// <param name="from">Starting cell.</param>
/// <param name="to">Ending cell.</param>
/// <returns>true if there is no clean direct route between the cells.</returns>
bool UnitClass::Is_Route_Broken(Cell const & from, Cell const & to) const
{
	if (HeightAGL >= 0 && Map.Is_Same_Cell_Zone(from, to, MZONE_NORMAL, 0, 0, 0)) {
		if (from == to) {
			return(false);
		}
		int dist = std::max(abs(to.X - from.X), abs(to.Y - from.Y));
		if (dist == 1) {
			return(false);
		}
		if (dist >= 12 || !Map.In_Local_Radar(from) || !Map.In_Local_Radar(to)) {
			return(true);
		}
		return(Search.Test_Cell_Walk(from, to, this, false, Map[to].IsUnderBridge, MZONE_NORMAL) > 15);
	}
	return(true);
}


/// <summary>
/// Changes ownership of this unit to the house specified.
/// A vehicle that has another following it takes its follower along, so that a captured
/// pair does not end up split between two houses.
/// </summary>
/// <param name="house">The house that is to take ownership of this unit.</param>
/// <returns>bool; Did ownership actually change hands?</returns>
bool UnitClass::Captured(HouseClass * house)
{
	if (house != House) {
		UnitClass * follower = FollowingMe;
		IsFollowing = NULL;

		if (follower != NULL) {
			follower->Captured(house);
			FollowingMe = follower;
			follower->IsFollowing = true;
		}

		return(BASECLASS::Captured(house));
	}
	return(false);
}


/// <summary>
/// Is this object treated as a vehicle by the rest of the game?
/// Some unit types -- visceroids and their kin -- are units only as a convenience and
/// should not be counted or handled as vehicles.
/// </summary>
/// <returns>bool; Should this object be considered a vehicle?</returns>
bool UnitClass::Considered_Vehicle(void) const
{
	return(!Class->IsNonVehicle);
}


/// <summary>
/// Discharges this unit's stored EM pulse charge.
/// This routine is used by the mobile EM pulse cannon once it has built up a full charge.
/// The pulse goes off at the vehicle's own position and the charge is spent.
/// </summary>
void UnitClass::EMPulse_Blast(void)
{
	if (!Is_Immobilized() && Charge >= Class->MaxCharge) {
		WeaponTypeClass const * weapon = WeaponTypeClass::Find_Or_Make("MobileEMPulseWeapon");
		if (weapon != NULL && weapon->Bullet != NULL && weapon->WarheadPtr != NULL) {
			CellClass * cptr = &Map[Center_Coord().As_Cell()];
			BulletClass * bullet = Create_Bullet(weapon->Bullet, cptr, this, weapon->Attack, weapon->WarheadPtr, 1234, weapon->ProjectileRange, weapon->IsBright);
			if (bullet != NULL) {
				bullet->Set_Coord(PositionCoord);
				bullet->Bullet_Explodes(true);
				bullet->Delete_Me();
			}
		}
		Charge = 0;
	}
}


/// <summary>
/// Creates the explosion that accompanies this unit's destruction.
/// A loaded harvester adds a blast in proportion to the Tiberium it was carrying, and a
/// sufficiently sturdy vehicle will rock the screen on its way out.
/// </summary>
void UnitClass::Explode(void)
{
	if (Class->Explosion.Count() > 0) {
		AnimTypeClass const * anim = Class->Explosion.Pick(Scen->RandomNumber);

		/*
		**	SSM launchers will really explode big if they are carrying
		**	missiles at the time of the explosion.
		*/
		if (Class->IsExploding || Has_Ability(ABILITY_EXPLODES)) {
			if (Class->MaxAmmo == -1 || Ammo > 0) {
				anim = Class->Explosion[Class->Explosion.Count() - 1];
			}
		}

		new AnimClass(anim, PositionCoord);

		/*
		**	Harvesters explode with a force equal to the amount of
		**	Tiberium they are carrying.
		*/
		if (Storage.Get_Total_Amount() > 0 && Rule->IsExplosiveHarvester && !Scen->Special.IsHarvesterImmune) {
			int power = 0;
			for (int i = 0; i < Tiberiums.Count(); i++) {
				power += Storage.Get_Amount(i) * Tiberiums[i]->Power;
			}
			Wide_Area_Damage(PositionCoord, CELL_LEPTON * 1.5, power, this, Rule->C4Warhead);
		}

		/*
		**	Very strong units that have an explosion will also rock the
		**	screen when they are destroyed.
		*/
		if (Class->MaxStrength > Rule->ShakeScreen) {
			int shakes = Class->MaxStrength / (Rule->ShakeScreen / 2) + 3;
			shakes = std::min(shakes, 6);
			Shake_The_Screen(shakes);
		}
	}
}


/// <summary>
/// Is this unit unable to move?
/// A vehicle that is playing out its death throes is immobilized on top of all the usual
/// reasons that apply to any foot object.
/// </summary>
/// <returns>bool; Is the unit prevented from moving?</returns>
bool UnitClass::Is_Immobilized(void) const
{
	return(DeathCounter != -1 || BASECLASS::Is_Immobilized());
}


/// <summary>
/// Fetches the class identifier used by the save game persistence system.
/// </summary>
/// <param name="retval">Pointer to the buffer to fill in with the class identifier.</param>
/// <returns>Returns with S_OK, or E_POINTER if no buffer was supplied.</returns>
HRESULT STDMETHODCALLTYPE UnitClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_UnitClass;
	return(S_OK);
}


/// <summary>
/// Fetches the RTTI type of this object.
/// </summary>
/// <returns>Returns with RTTI_UNIT.</returns>
RTTIType UnitClass::Fetch_RTTI(void) const
{
	return(RTTI_UNIT);
}


/// <summary>
/// Fetches the displayable name of this unit.
/// </summary>
/// <returns>Returns with a pointer to the name to present to the player.</returns>
char const * UnitClass::Full_Name(void) const
{
	return(Class->GivenName);
}


/// <summary>
/// Fetches the direction this unit's weapon is pointing.
/// This routine returns the turret facing for a turreted vehicle and the body facing for
/// everything else, so a caller need not know which kind of unit it is dealing with.
/// </summary>
/// <returns>Returns with the direction the unit is currently aiming.</returns>
DirType UnitClass::Turret_Facing(void) const
{
	if (Class->IsTurretEquipped) {
		return(SecondaryFacing.Current());
	} else {
		return(PrimaryFacing.Current());
	}
}
