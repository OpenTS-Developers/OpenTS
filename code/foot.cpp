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

/* $Header: /CounterStrike/FOOT.CPP 2     3/06/97 1:46p Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FOOT.CPP                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 22, 1994                                               *
 *                                                                                             *
 *                  Last Update : October 5, 1996 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   FootClass::AI -- Handle general movement AI.                                              *
 *   FootClass::Active_Click_With -- Initiates attack or move according to target clicked on.  *
 *   FootClass::Active_Click_With -- Performs action as a result of left mouse click.          *
 *   FootClass::Adjust_Dest -- Adjust candidate movement cell to account for formation.        *
 *   FootClass::Approach_Target -- Sets the navigation computer to approach target object.     *
 *   FootClass::Assign_Destination -- Assigns specified destination to NavCom.                 *
 *   FootClass::Basic_Path -- Finds the basic path for a ground object.                        *
 *   FootClass::Body_Facing -- Set the body rotation/facing.                                   *
 *   FootClass::Can_Demolish -- Checks to see if this object can be sold back.                 *
 *   FootClass::Can_Enter_Cell -- Checks to see if the object can enter cell specified.        *
 *   FootClass::Clear_Navigation_List -- Clears out the navigation queue.                      *
 *   FootClass::Death_Announcement -- Announces the death of a unit.                           *
 *   FootClass::Debug_Dump -- Displays the status of the FootClass to the mono monitor.        *
 *   FootClass::Detach -- Detaches a target from tracking systems.                             *
 *   FootClass::Detach_All -- Removes this object from the game system.                        *
 *   FootClass::Enters_Building -- When unit enters a building for some reason.                *
 *   FootClass::FootClass -- Normal constructor for the foot class object.                     *
 *   FootClass::Greatest_Threat -- Fetches the greatest threat to this object.                 *
 *   FootClass::Handle_Navigation_List -- Processes the navigation queue.                      *
 *   FootClass::Is_Allowed_To_Leave_Map -- Checks to see if it can leave the map and the game. *
 *   FootClass::Is_On_Priority_Mission -- Checks to see if this object should be given priority*
 *   FootClass::Is_Recruitable -- Determine if this object is recruitable as a team members.   *
 *   FootClass::Likely_Coord -- Fetches the coordinate the object will be at shortly.          *
 *   FootClass::Mark -- Unit interface to map rendering system.                                *
 *   FootClass::Mission_Attack -- AI for heading towards and firing upon target.               *
 *   FootClass::Mission_Capture -- Handles the capture mission.                                *
 *   FootClass::Mission_Enter -- Enter (cooperatively) mission handler.                        *
 *   FootClass::Mission_Guard_Area -- Causes unit to guard an area about twice weapon range.   *
 *   FootClass::Mission_Hunt -- Handles the default hunt order.                                *
 *   FootClass::Mission_Move -- AI process for moving a vehicle to its destination.            *
 *   FootClass::Mission_Retreat -- Handle reatreat from map mission for mobile objects.        *
 *   FootClass::Offload_Tiberium_Bail -- Fetches the Tiberium to offload per step.             *
 *   FootClass::Override_Mission -- temporarily overrides a units mission                      *
 *   FootClass::Per_Cell_Process -- Perform action based on once-per-cell condition.           *
 *   FootClass::Queue_Navigation_List -- Add a target to the objects navigation list.          *
 *   FootClass::Receive_Message -- Movement related radio messages are handled here.           *
 *   FootClass::Rescue_Mission -- Calls this unit to the rescue.                               *
 *   FootClass::Restore_Mission -- Restores an overridden mission                              *
 *   FootClass::Sell_Back -- Causes this object to be sold back.                               *
 *   FootClass::Set_Speed -- Initiate unit movement physics.                                   *
 *   FootClass::Sort_Y -- Determine the sort coordinate for foot class objects.                *
 *   FootClass::Start_Driver -- This starts the driver heading to the destination desired.     *
 *   FootClass::Stop_Driver -- This routine clears the driving state of the object.            *
 *   FootClass::Stun -- Prepares a ground travelling object for removal.                       *
 *   FootClass::Take_Damage -- Handles taking damage to this object.                           *
 *   FootClass::Unlimbo -- Unlimbos object and performs special fixups.                        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "foot.h"

#include "_astar.h"
#include "_bench.h"
#include "_convert.h"
#include "_keyboar.h"
#include "_map.h"
#include "_rect.h"
#include "_rtti.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "aircraft.h"
#include "anim.h"
#include "astar.h"
#include "bench.h"
#include "building.h"
#include "builtype.h"
#include "ccrand.h"
#include "cell.h"
#include "convert.h"
#include "ddist.h"
#include "goptions.h"
#include "house.h"
#include "incdec.h"
#include "infantry.h"
#include "infatype.h"
#include "inline.h"
#include "ion.h"
#include "ipiggy.h"
#include "loco.h"
#include "mono.h"
#include "partsys.h"
#include "revent.h"
#include "rules.h"
#include "savestream.h"
#include "session.h"
#include "swizzle.h"
#include "tactical.h"
#include "team.h"
#include "tracker.h"
#include "tube.h"
#include "unit.h"
#include "unittype.h"
#include "vein.h"
#include "voc.h"
#include "vox.h"
#include "waypoint.h"
#include "weapon.h"
#include "xsurface.h"

#include "bench.hh"
#include "color.hh"
#include "tube.hh"

#include <algorithm>
#include <cstring>
#include <iterator>


DynamicVectorClass<FootClass *> Feet;

/***********************************************************************************************
 * FootClass::FootClass -- Default constructor for foot class objects.                         *
 *                                                                                             *
 *    This is the default constructor for FootClass objects. It sets the foot class values to  *
 *    their default starting settings.                                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
FootClass::FootClass(HouseClass * house) :
	BASECLASS(house),
	IsToScatter(false),
	IsScanLimited(false),
	IsInitiated(false),
	IsNewNavCom(false),
	IsPlanningToLook(false),
	IsDeploying(false),
	IsFiring(false),
	IsRotating(false),
	IsUnloading(false),
	IsNavQueueLoop(false),
	IsScattering(false),
	IsIdle(false),
	IonBlastYDrawOffset(0),
	IsCrushing(false),
	IsOccupyingCell(true),
	IsToPathAroundBlockage(false),
	IsDroppedFromTeam(false),
	CurrentPath(PATH_NONE),
	WaypointOffsetCell(0,0),
	WaypointTargetCell(0,0),
	ThreatAvoidanceCoefficient(0),
	TotalFramesWalked(0),
	LastPathingCell(0,0),
	LastAdjacencyCell(0,0),
	LastTubeCoord(0,0,0),
	Speed(0),
	SpeedBias(1),
	NavCom(NULL),
	SuspendedNavCom(NULL),
	Team(0),
	Member(0),
	PatrolCell(0),
	PathDelay(0),
	TryTryAgain(PATH_RETRY),
	BaseAttackTimer(0),
	HeadToCoord(COORD_NONE),
	CurrentTube(-1),
	CurrentTubeDir(0),
	NextWaypoint(0)
{
	std::fill(std::begin(Path), std::end(Path), FACING_NONE);
	Feet.Add(this);
	TeamPtrTracker.Add(this);
}


/// <summary>
/// Destroys the object and unlinks it from the game.
/// This routine will resign the object from whatever team it belongs to and then remove it
/// from the global foot and team trackers.
/// </summary>
FootClass::~FootClass(void)
{
	if (Team != NULL && GameActive) {
		Team->Remove(this);
	}

	Feet.Delete(this);
	TeamPtrTracker.Delete(this);
};


#ifdef _DEBUG
/***********************************************************************************************
 * FootClass::Debug_Dump -- Displays the status of the FootClass to the mono monitor.          *
 *                                                                                             *
 *    This routine is used to output the current status of the foot class to the mono          *
 *    monitor. Through this display bugs may be tracked down or eliminated.                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/02/1994 JLB : Created.                                                                 *
 *   07/04/1995 JLB : Handles aircraft special case.                                           *
 *=============================================================================================*/
void FootClass::Debug_Dump(MonoClass * mono) const
{
	mono->Fill_Attrib(53, 13, 12, 1, IsInitiated ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(1, 18, 12, 1, IsPlanningToLook ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(53, 14, 12, 1, IsDeploying ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(53, 15, 12, 1, IsFiring ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(53, 16, 12, 1, IsRotating ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(53, 18, 12, 1, IsUnloading ? MonoClass::INVERSE : MonoClass::NORMAL);

	mono->Set_Cursor(45, 1);mono->Printf("%02X", Speed);
	if (NavCom) {
		mono->Set_Cursor(29, 5);
		mono->Printf("%08X", NavCom);
	}
	if (SuspendedNavCom) {
		mono->Set_Cursor(38, 5);
		mono->Printf("%08X", SuspendedNavCom);
	}

	if (Team) Team->Debug_Dump(mono);
	if (Group != 255) {
		mono->Set_Cursor(59, 1);mono->Printf("%d", Group);
	}

	static char	const * _p2c[9] = {"-","0","1","2","3","4","5","6","7"};
	for (int index = 0; index < std::min(12, ARRAY_SIZE(Path)); index++) {
		mono->Set_Cursor(54+index, 3);
		mono->Printf("%s", _p2c[((abs((int)Path[index]+1)) % ARRAY_SIZE(_p2c))]);
	}
	mono->Set_Cursor(72, 3);mono->Printf("%4d", (int)PathDelay);
	mono->Set_Cursor(67, 3);mono->Printf("%3d", TryTryAgain);
	if (HeadToCoord != COORD_NONE) {
		mono->Set_Cursor(54, 5);mono->Printf("%5d,%5d,%3d", HeadToCoord.X, HeadToCoord.Y, HeadToCoord.Z);
	}

	BASECLASS::Debug_Dump(mono);
}
#endif


/***********************************************************************************************
 * FootClass::Set_Speed -- Initiate unit movement physics.                                     *
 *                                                                                             *
 *    This routine is used to set a unit's velocity control structure.                         *
 *    The game will then process the unit's movement during the momentum                       *
 *    physics calculation.                                                                     *
 *                                                                                             *
 * INPUT:   unit  -- Pointer to the unit to alter.                                             *
 *                                                                                             *
 *          speed -- Throttle setting (0=stop, 255=full throttle).                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/07/1992 JLB : Created.                                                                 *
 *   09/24/1993 JLB : Revised for faster speed.                                                *
 *   04/02/1994 JLB : Revised for new system.                                                  *
 *   04/15/1994 JLB : Converted to member function.                                            *
 *   07/21/1994 JLB : Simplified.                                                              *
 *=============================================================================================*/
void FootClass::Set_Speed(double speed)
{
	if (speed >= 1.0) {
		speed = 1.0;
	} else if (speed <= 0.0) {
		speed = 0.0;
	}

	Speed = speed;
}


/***********************************************************************************************
 * FootClass::Mark -- Unit interface to map rendering system.                                  *
 *                                                                                             *
 *    This routine is the interface function for units as they relate to                       *
 *    the map rendering system. Whenever a unit's imagery changes, this                        *
 *    function is called.                                                                      *
 *                                                                                             *
 * INPUT:   mark  -- Type of image change (MARK_UP, _DOWN, _CHANGE)                            *
 *             MARK_UP  -- Unit is removed.                                                    *
 *             MARK_CHANGE -- Unit alters image but doesn't move.                              *
 *             MARK_DOWN -- Unit is overlaid onto existing icons.                              *
 *                                                                                             *
 * OUTPUT:  bool; Did the marking operation succeed? Failure could be the result of marking    *
 *                down when it is already down, or visa versa.                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/14/1991 JLB : Created.                                                                 *
 *   04/15/1994 JLB : Converted to member function.                                            *
 *   12/23/1994 JLB : Performs low level check before processing.                              *
 *=============================================================================================*/
bool FootClass::Mark(MarkType mark)
{
	assert(this != NULL);

	if (mark == MARK_CHANGE) {
		return(true);
	}

	if (BASECLASS::Mark(mark)) {
		if (In_Which_Layer() == LAYER_GROUND) {

			Cell cell = Get_Cell();

			/*
			**	Inform the map of the refresh, occupation, and overlap
			**	request.
			*/
			switch (mark) {
				case MARK_UP:
					Map.Pick_Up(cell, this);
					break;

				case MARK_DOWN:
				case MARK_DOWN_FORCED:
					Map.Place_Down(cell, this);
					break;
			}
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Determines if the specified location shares this object's destination zone.
/// This routine is consulted before an object commits to a journey, so that it does not set
/// off toward somewhere its locomotor could never actually walk or drive to. An object with
/// no movement zone restriction always answers yes.
/// </summary>
/// <param name="coord">The location to compare against this object's destination.</param>
/// <returns>bool; Are the two locations in the same movement zone?</returns>
bool FootClass::Is_In_Same_Zone(Coord const & coord) const
{
	MZoneType zone = TClass->MZone;
	if (zone == MZONE_NONE) {
		return(true);
	}

	Cell cell1 = coord.As_Cell();

	if (cell1 == CELL_NONE) {
		return(false);
	}

	Cell cell2 = Destination_Coord().As_Cell();

	return(Map.Is_Same_Cell_Zone(cell2, cell1, zone, ((ObjectClass *)this)->Is_Moving_Onto_Bridge(), Map[cell1].IsUnderBridge, Is_Allowed_To_Leave_Map()));
	return(0);
}


/***********************************************************************************************
 * FootClass::Basic_Path -- Finds the basic path for a ground object.                          *
 *                                                                                             *
 *    This is a common routine used by both infantry and other ground travelling units. It     *
 *    will fill in the unit's basic path to the NavCom destination.                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was a path found? A failure to find a path means either the target cannot    *
 *                be found or the terrain prohibits the unit's movement.                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FootClass::Basic_Path(Cell cell, int path_offset, int avoidance)
{
	/// simplifies movement zone types
	static const MZoneType _simple_mzone[MZONE_COUNT] = {
		MZONE_NORMAL,
		MZONE_NORMAL,
		MZONE_NORMAL,
		MZONE_AMPHIBIOUS,
		MZONE_AMPHIBIOUS,
		MZONE_AMPHIBIOUS,
		MZONE_SUBTERANNEAN,
		MZONE_INFANTRY,
		MZONE_INFANTRY,
		MZONE_FLYER,
	};

	PathStruct		* path;			// Pointer to path control structure.
	int 			skip_path = false;

	if (path_offset == 0) {
		Path[0] = FACING_NONE;
	}

	if (!Is_In_Same_Zone(cell)) {
		Path[0] = FACING_NONE;
		return(false);
	}

	/*
	**	When the navigation computer is set to a location that is impassible, then
	**	find a nearby cell that can be entered and try to head toward that instead.
	**	EXCEPT when that cell is very close -- then just bail.
	*/
	int dist = Distance(cell);
	int checkdist = Team != NULL ? Rule->StrayDistance : Rule->CloseEnoughDistance;
	int maxdist = 0;

	bool istrain = TClass->IsTrain;

	MoveType move = Can_Enter_Cell(&Map[cell]);

	if (move == MOVE_TEMP && dist > checkdist && !istrain) {

		MZoneType mzone = _simple_mzone[TClass->MZone];
		bool check = TClass->IsSubterranean && Map[cell].Can_Burrow_Here() ? true : false;

		Cell nearby = Map.Nearby_Location(cell, TClass->Speed, Map.Get_Cell_Zone(PositionCell, mzone, IsOnBridge), mzone, IsOnBridge, Point2D(1, 1), false, true, check, true, PositionCell);
		maxdist = std::max(abs(nearby.X - cell.X), abs(nearby.Y - cell.Y));

		if (nearby != CELL_NONE && ::Distance(Coord(cell), Coord(nearby)) < dist) {

			int adist = Search.Test_Cell_Walk(nearby, cell, this, Map[nearby].IsUnderBridge, Map[cell].IsUnderBridge, check ? MZONE_NORMAL : MZONE_NONE);
			if (adist <= maxdist + 6) {
				Assign_Destination(&Map[nearby]);
				cell = nearby;
			}
		}
	}

	if (move == MOVE_NO && !istrain) {

		if (Map[cell].Cell_Building() != NULL) {

			MZoneType mzone = _simple_mzone[TClass->MZone];
			bool check = TClass->IsSubterranean && Map[cell].Can_Burrow_Here() ? true : false;

			Cell nearby = Map.Nearby_Location(cell, TClass->Speed, Map.Get_Cell_Zone(PositionCell, mzone, IsOnBridge), mzone, IsOnBridge, Point2D(1, 1), false, true, check, true, PositionCell);

			Assign_Destination(&Map[nearby]);
			cell = nearby;
		}
	}

	if (!skip_path) {
		Mark(MARK_UP);
		if (path_offset == 0) {
			Path[0] = FACING_NONE;		// Probably not necessary, but...
		}

		/*
		**	Try to find a path to the destination. If a failure occurs, then keep trying
		**	with greater determination until either a complete failure occurs, or a decent
		**	path was found.
		*/
		bool found = false;		// Found a best path yet?
		PathStruct path1;
		FacingType workpath[200*10];	// Staging area for path list.
		MoveType maxtype = MOVE_OK;

		path = Find_Path(cell, &workpath[0], ARRAY_SIZE(workpath), maxtype, path_offset, avoidance);

		if (path && path->Cost) {
			memcpy(&path1, path, sizeof(path1));
			found = true;
		}

		/*
		**	If a good path was found, then record it in the object's path
		**	list.
		*/
		if (found) {
			Fixup_Path(&path1);
			memcpy(&Path[path_offset], &workpath[0], std::min(path->Length, ARRAY_SIZE(Path) - path_offset) * sizeof(Path[0]));
		}

		Mark(MARK_DOWN);

		PathDelay = 0;

		if (RTTI == RTTI_UNIT && Path[0] != FACING_NONE) {
			UnitClass * unit = (UnitClass *)this;
			UnitClass * follower = unit->FollowingMe;
			int length = 0;
			if (path != NULL) {
				length = path->Length - 2;
			}
			Cell ncell = cell;
			if (follower != NULL && !unit->IsFollowing) {
				while (follower != NULL) {
					CellClass *cptr = &Map[ncell];
					ncell = cptr->Adjacent_Cell(FacingType((workpath[length] + FACING_180) % FACING_COUNT)).Fetch_CellID();
					AbstractClass *nav = follower->NavCom;
					if (nav == NULL || nav->Destination_Coord().As_Cell() != ncell) {
						follower->Assign_Mission(MISSION_MOVE);
						follower->Assign_Destination(&Map[ncell]);
						follower->Basic_Path(ncell);
					}
					follower = follower->FollowingMe;
				}
			}
		}

		if (path != NULL) {
			LastPathingCell = PositionCell;
			return(true);
		}

		PathDelay = Rule->PathDelay * TICKS_PER_MINUTE;
	}

	/*
	**	If a basic path couldn't be determined, then abort the navigation process.
	*/
	Stop_Driver();

	Cell mycell = PositionCoord;
	maxdist = std::max(abs(mycell.X - cell.X), abs(mycell.Y - cell.Y));
	if (maxdist > 1 || (!IsOnBridge && Map[cell].IsUnderBridge)) {
		if (Team != NULL) {
			Locomotion->Lock();
			Team->Remove(this);
			Locomotion->Unlock();
		}

		Assign_Destination(NULL);
		Assign_Target(NULL);

		if (!House->Is_Human_Player()) {
			Assign_Mission(MISSION_GUARD_AREA);
			if (Session.Type != GAME_NORMAL) {
				Cell where = House->Where_To_Go(this);
				if (where != CELL_NONE) {
					Assign_Destination(&Map[where]);
				}
			}
		} else {
			Assign_Mission(MISSION_GUARD);
		}
	}

	return(false);
}

/// <summary>
/// Removes completed steps from the front of the active movement path.
/// </summary>
void FootClass::Advance_Path(int count)
{
	if (count <= 0) {
		return;
	}

	int const advance = std::min(count, ARRAY_SIZE(Path));
	std::memmove(Path, Path + advance, (ARRAY_SIZE(Path) - advance) * sizeof(Path[0]));
	std::fill(std::end(Path) - advance, std::end(Path), FACING_NONE);
}



/***********************************************************************************************
 * FootClass::Mission_Move -- AI process for moving a vehicle to its destination.              *
 *                                                                                             *
 *    This simple AI script handles moving the vehicle to its desired destination. Since       *
 *    simple movement is handled directly by the engine, this routine merely waits until       *
 *    the unit has reached its destination, and then causes the unit to enter idle mode.       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the delay before calling this routine again.                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1994 JLB : Created.                                                                 *
 *   10/02/1996 JLB : Player controlled or human owned units don't scan for targets.           *
 *=============================================================================================*/
int FootClass::Do_MISSION_MOVE(void)
{
	if (NavCom == NULL && !Locomotion->Is_Moving() && MissionQueue == MISSION_NONE) {
		Enter_Idle_Mode();
		return(1);
	}

	if (TarCom == NULL && !House->Is_Human_Player() && (Team == NULL || !Team->Class->IsSuicide)) {
		Target_Something_Nearby(Get_Coord(), THREAT_RANGE);
	}

	return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
}


/// <summary>
/// Handles the patrol mission state machine. The unit scans for threats while
/// traveling, breaks off to engage any threat it can reach, then returns to
/// its patrol point or resumes its archived travel destination.
/// </summary>
/// <returns>The delay in game frames before this mission should be processed again.</returns>
int FootClass::Do_MISSION_PATROL(void)
{
	int trange = Threat_Range(2);
	IsOnPatrol = true;
	int dist = (trange / CELL_LEPTON_W) + 6;

	enum {
		PATROL_FIND_TARGET,		/// scan for a target and take up the patrol
		PATROL_ENGAGE_TARGET,	/// fighting the target; approach until it is gone
		PATROL_RETURN,			/// heading back; re-scan from the patrol cell
		PATROL_RESET			/// wait until moving again before scanning anew
	};

	switch (Status) {

		case PATROL_RESET:
			if (NavCom != NULL) {
				Status = PATROL_FIND_TARGET;
				return(1);
			}
			break;

		case PATROL_FIND_TARGET: {
			AbstractClass * target = Greatest_Threat(THREAT_AREA, Get_Coord(), false);
			ObjectClass * optr = target->As_ObjectClass();
			int primary = What_Weapon_Should_I_Use(optr);

			if (optr != NULL && (In_Range(optr, primary) || Search.Test_Cell_Walk(Cell(Destination_Coord()), Cell(optr->Destination_Coord()), this, Is_Moving_Onto_Bridge(), optr->Is_Moving_Onto_Bridge(), MZONE_NONE) < dist)) {

				/*
				 * Take up patrol at the cell the target occupies. If that cell is
				 * tucked under a bridge and this unit isn't crossing onto the
				 * bridge, find a nearby clear cell instead.
				 */
				PatrolCell = Get_Target_Cell_Ptr();
				if (PatrolCell->IsUnderBridge && !Is_Moving_Onto_Bridge()) {
					Cell cell = Map.Nearby_Location(PatrolCell->CellID, TClass->Speed, Map.Get_Cell_Zone(PatrolCell->CellID, TClass->MZone, false), TClass->MZone, false, Point2D(1,1), false, true, false, false);
					if (cell != CELL_NONE) {
						PatrolCell = &Map[cell];
					} else {
						PatrolCell = NULL;
						return(60);
					}
				}

				/*
				 * Remember where to resume travel once the threat is dealt with.
				 */
				if (ArchiveTarget == NULL) {
					if (RouteQueue.Count() > 0) {
						ArchiveTarget = RouteQueue[RouteQueue.Count()-1];
					} else if (NavCom != NULL) {
						ArchiveTarget = NavCom;
					} else {
						ArchiveTarget = PatrolCell;
					}
				}

				Assign_Destination(NULL);
				Assign_Target(target);
				Status = PATROL_ENGAGE_TARGET;

			} else if (NavCom == NULL || !Locomotion->Is_Moving()) {
				Enter_Idle_Mode();
				Status = PATROL_RESET;
				return(1);
			}
			break;
		}

		case PATROL_ENGAGE_TARGET: {
			ObjectClass * optr = TarCom->As_ObjectClass();
			int primary = What_Weapon_Should_I_Use(optr);

			/*
			 * Engineers shouldn't try to recapture an allied building that is
			 * still healthy enough.
			 */
			if (Is_Renovator() && optr != NULL && optr->RTTI == RTTI_BUILDING && House->Is_Ally(((TechnoClass *)optr)->House) && optr->HealthRatio > Rule->ConditionRed) {
				Assign_Target(NULL);
			} else if (optr != NULL && (In_Range(optr, primary) || Search.Test_Cell_Walk(PatrolCell->CellID, Cell(optr->Destination_Coord()), this, PatrolCell->IsUnderBridge, optr->Is_Moving_Onto_Bridge(), MZONE_NONE) < dist)) {
				Approach_Target();
				if (TarCom != NULL) {
					break;
				}
				Assign_Destination(ArchiveTarget);
				ArchiveTarget = NULL;
				PatrolCell = NULL;
				Status = PATROL_FIND_TARGET;
				return(45);
			}

			/*
			 * The target is gone or out of reach. Scan for another threat from
			 * the patrol cell before giving up the engagement.
			 */
			if (TarCom == NULL) {
				AbstractClass * target = Greatest_Threat(THREAT_AREA, PatrolCell->Center_Coord(), false);
				ObjectClass * scanptr = target->As_ObjectClass();
				int scanweapon = What_Weapon_Should_I_Use(scanptr);
				if (scanptr != NULL && (In_Range(scanptr, scanweapon) || Search.Test_Cell_Walk(PatrolCell->CellID, Cell(scanptr->Destination_Coord()), this, PatrolCell->IsUnderBridge, scanptr->Is_Moving_Onto_Bridge(), MZONE_NONE) < dist)) {
					Assign_Target(target);
					Assign_Destination(NULL);
					return(1);
				}
			}

			/*
			 * Head for whichever is closer; the patrol cell or the archived
			 * travel destination.
			 */
			Status = PATROL_RETURN;
			Assign_Target(NULL);
			if (ArchiveTarget != NULL) {
				CellClass * tcell = Get_Target_Cell_Ptr();
				bool onbridge = Is_Moving_Onto_Bridge();
				CellClass * acell = &Map[ArchiveTarget->Center_Coord()];
				int patrolwalk = Search.Test_Cell_Walk(tcell->CellID, PatrolCell->CellID, this, onbridge, PatrolCell->IsUnderBridge, MZONE_NONE);
				if (patrolwalk < Search.Test_Cell_Walk(tcell->CellID, acell->CellID, this, onbridge, acell->IsUnderBridge, MZONE_NONE)) {
					Assign_Destination(PatrolCell);
				} else {
					Assign_Destination(ArchiveTarget);
				}
				return(1);
			}
			Status = PATROL_FIND_TARGET;
			Enter_Idle_Mode();
			Status = PATROL_RESET;
			return(1);
		}

		case PATROL_RETURN: {
			AbstractClass * target = Greatest_Threat(THREAT_AREA, PatrolCell->Center_Coord(), false);
			ObjectClass * optr = target->As_ObjectClass();
			int primary = What_Weapon_Should_I_Use(optr);

			if (optr != NULL && (In_Range(optr, primary) || Search.Test_Cell_Walk(PatrolCell->CellID, Cell(optr->Destination_Coord()), this, PatrolCell->IsUnderBridge, optr->Is_Moving_Onto_Bridge(), MZONE_NONE) < dist)) {
				Assign_Target(target);
				Assign_Destination(NULL);
				Status = PATROL_ENGAGE_TARGET;
				break;
			}

			if (NavCom == NULL) {
				Assign_Target(NULL);
				if (ArchiveTarget != NULL) {
					Assign_Destination(ArchiveTarget);
					ArchiveTarget = NULL;
					Status = PATROL_FIND_TARGET;
					PatrolCell = NULL;
				} else {
					Status = PATROL_FIND_TARGET;
					Enter_Idle_Mode();
					Status = PATROL_RESET;
					return(1);
				}
			}
			break;
		}

		default:
			break;
	}

	return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
}


/***********************************************************************************************
 * FootClass::Mission_Capture -- Handles the capture mission.                                  *
 *                                                                                             *
 *    Capture missions are nearly the same as normal movement missions. The only difference    *
 *    is that the final destination is handled in a special way so that it is not marked as    *
 *    impassable. This allows the object (usually infantry) the ability to walk onto the       *
 *    object and thus capture it.                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game ticks to delay before calling this routine.        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int FootClass::Do_MISSION_CAPTURE(void)
{
	InfantryClass * inf = dynamic_cast<InfantryClass *>(this);

	/*
	**	If there is a valid TarCom but the NavCom isn't set, then set the NavCom accordingly.
	*/
	if (Is_Target_Building(TarCom) && NavCom == NULL && inf != NULL && (inf->Class->IsBomber || inf->Has_Ability(ABILITY_C4) || inf->Class->IsEngineer)) {
		Assign_Destination(TarCom);
	}

	if (Mission == MISSION_SABOTAGE) {
		BuildingClass * building = dynamic_cast<BuildingClass *>(NavCom);
		if (building != NULL && !building->Class->IsRepairable) {
			Assign_Target(NULL);
			Assign_Destination(NULL);
			Enter_Idle_Mode();
			return(1);
		}
	}

	if (NavCom == NULL /*&& !In_Radio_Contact()*/) {
		Enter_Idle_Mode();
		if (Map[Center_Coord()].Cell_Building()) {
			Scatter(COORD_NONE, true);
		}
	}

	if (TarCom == NULL && !House->Is_Human_Player()) {
		Assign_Target(NULL);
		Assign_Destination(NULL);
		Assign_Mission(MISSION_HUNT);
	}

	return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
}


/***********************************************************************************************
 * FootClass::Mission_Attack -- AI for heading towards and firing upon target.                 *
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
int FootClass::Do_MISSION_ATTACK(void)
{
	if (TarCom != NULL) {
		Approach_Target();
	} else {
		Enter_Idle_Mode();
	}
	return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
}


/***********************************************************************************************
 * FootClass::Mission_Guard -- Handles the AI for guarding in place.                           *
 *                                                                                             *
 *    Units that are performing stationary guard duty use this AI process. They will sit       *
 *    still and target any enemies that get within range.                                      *
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
int FootClass::Do_MISSION_GUARD(void)
{
	bool renovator = Is_Renovator();
	if (!renovator && (TarCom == NULL || RTTI != RTTI_AIRCRAFT || House->Is_Human_Player())) {
		if (!Target_Something_Nearby(PositionCoord, THREAT_RANGE)) {
			Random_Animate();
		}
	}

	int dtime = Current_Mission_Control().Normal_Delay();

	InfantryClass * inf = dynamic_cast<InfantryClass *>(this);
	if (inf != NULL) {

		/*
		**	If this is a bomber type infantry and the current target is a building, then go into
		**	sabotage mode if not already.
		*/
		if (!House->Is_Human_Player() && (inf->Class->IsBomber || inf->Has_Ability(ABILITY_C4)) && Mission != MISSION_SABOTAGE) {
			BuildingClass * building = dynamic_cast<BuildingClass *>(TarCom);
			if (building != NULL && building->Class->IsRepairable) {
				Assign_Mission(MISSION_SABOTAGE);
			}
		}
	}

	return((Arm != 0) ? (int)Arm : (dtime+Random_Pick(0, 2)));
}


/***********************************************************************************************
 * FootClass::Mission_Hunt -- Handles the default hunt order.                                  *
 *                                                                                             *
 *    This routine is the default hunt order for game objects. It handles searching for a      *
 *    nearby object and heading toward it. The act of targeting will cause it to attack        *
 *    the target it selects.                                                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the game tick delay before calling this routine again.                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int FootClass::Do_MISSION_HUNT(void)
{
	if (!Target_Something_Nearby(PositionCoord, THREAT_NORMAL)) {
		Random_Animate();
	} else {
		InfantryClass * infantry = RTTI == RTTI_INFANTRY ? (InfantryClass *)this : NULL;
		if (infantry != NULL && infantry->Class->IsEngineer && !infantry->Class->IsBomber && !infantry->Has_Ability(ABILITY_C4)) {
			Assign_Destination(TarCom);
			Assign_Mission(MISSION_CAPTURE);
			if (Ready_To_Commence()) {
				Commence();
			}
		} else {
			BuildingClass * building = dynamic_cast<BuildingClass *>(TarCom);
			if (infantry != NULL && (infantry->Class->IsBomber || infantry->Has_Ability(ABILITY_C4)) && building != NULL && building->Class->IsRepairable) {
				Assign_Destination(TarCom);
				Assign_Mission(MISSION_SABOTAGE);
				if (Ready_To_Commence()) {
					Commence();
				}
			} else if (infantry != NULL && infantry->Class->IsVehicleThief) {
				Assign_Destination(TarCom);
				Assign_Mission(MISSION_CAPTURE);
				if (Ready_To_Commence()) {
					Commence();
				}
			} else {
				Approach_Target();
			}
		}
	}
	return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
}


/***********************************************************************************************
 * FootClass::Stop_Driver -- This routine clears the driving state of the object.              *
 *                                                                                             *
 *    This is the counterpart routine to the Start_Driver function. It clears the driving      *
 *    status flags and destination coordinate record.                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was driving stopped?                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *   12/12/1994 JLB : Greatly simplified.                                                      *
 *=============================================================================================*/
bool FootClass::Stop_Driver(void)
{
	Locomotion->Stop_Moving();
	return(false);
}


/***********************************************************************************************
 * FootClass::Start_Driver -- This starts the driver heading to the destination desired.       *
 *                                                                                             *
 *    Before a unit can move it must be started by this routine. This routine handles          *
 *    reserving the cell and setting the driving flag.                                         *
 *                                                                                             *
 * INPUT:   headto   -- The coordinate of the immediate drive destination. This is one cell    *
 *                      away from the unit's current location.                                 *
 *                                                                                             *
 * OUTPUT:  bool; Was driving initiated?                                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *   12/12/1994 JLB : Uses simple spot index finder.                                           *
 *=============================================================================================*/
bool FootClass::Start_Driver(Coord & headto)
{
	Locomotion->Move_To(headto);
	return(Locomotion->Is_Moving() != 0);
}


/***********************************************************************************************
 * FootClass::Stun -- Prepares a ground travelling object for removal.                         *
 *                                                                                             *
 *    This routine clears the units' navigation computer in preparation for removal from the   *
 *    game. This is probably called as a result of unit destruction in combat. Clearing the    *
 *    navigation computer ensures that the normal AI process won't start it moving again while *
 *    the object is undergoing any death animations.                                           *
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
void FootClass::Stun(void)
{
	Assign_Destination(NULL);
	Path[0] = FACING_NONE;
	Stop_Driver();
	BASECLASS::Stun();
}


/***********************************************************************************************
 * FootClass::Approach_Target -- Sets the navigation computer to approach target object.       *
 *                                                                                             *
 *    This routine will set the navigation computer to approach the target indicated by the    *
 *    targeting computer. It is through this function that the unit nears the target so        *
 *    that weapon firing may occur.                                                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *   12/13/1994 JLB : Made part of TechnoClass.                                                *
 *   12/22/1994 JLB : Enhanced search algorithm.                                               *
 *   05/20/1995 JLB : Always approaches if the object is off the map.                          *
 *=============================================================================================*/
void FootClass::Approach_Target(void)
{
	if (RTTI == RTTI_UNIT && ((UnitClass *)this)->Class->IsJellyfish) {
		return;
	}

	/*
	**	Determine that if there is an existing target it is still legal
	**	and within range.
	*/
	if (TarCom != NULL) {
		int primary = What_Weapon_Should_I_Use(TarCom);

		/*
		**	If the target is too far away then head toward it.
		*/
		int maxrange = Weapon_Range(primary);
		bool inrange = In_Range(TarCom, primary);

		/*
		 * A unit on a sticky mission that finds its target out of range will
		 * just forget about the target rather than give chase.
		 */
		if (!inrange && CurrentMission == MISSION_STICKY) {
			Assign_Target(NULL);
			Assign_Destination(NULL);
			return;
		}

		/*
		 * A unit that must deploy to fire, and is sitting on ground that it
		 * cannot deploy upon, must restrict the search to deployable ground
		 * within a reasonable distance of the target.
		 */
		bool checkbuild = false;
		if (RTTI == RTTI_UNIT && ((UnitClass *)this)->Class->IsDeployToFire) {
			if (!Map[(Coord const &)PositionCoord].Can_Build_Here()) {
				checkbuild = true;
				int dist = Distance(TarCom) + 2 * CELL_LEPTON;
				if (maxrange >= dist) {
					maxrange = dist;
				}
			}
		}

		/*
		 * Flying objects need no path validation when looking for a location
		 * to attack from.
		 */
		bool flyer = (RTTI == RTTI_AIRCRAFT);

		CLSID clsid;
		IPersistPtr persist(Locomotion);
		persist->GetClassID(&clsid);
		if (clsid == CLSID_JumpjetLocomotion) {
			flyer = true;
		}

		/*
		 * If the unit is not heading somewhere on the ground, and the weapon
		 * cannot already bear on the target, then scan for a better location.
		 */
		if ((NavCom == NULL || In_Air()) && (!inrange || !IsLocked || checkbuild)) {

			/*
			 * If a queued route exists, then advance to the next location on it.
			 */
			if (RouteQueue.Count() > 0) {
				Assign_Destination(RouteQueue[0], false);
				RouteQueue.Delete_Index(0);
				return;
			}

			/*
			 * When attacking a destroyable cliff, retarget to the closest passable
			 * cell so that the cliff can actually be approached.
			 */
			if (TarCom->RTTI == RTTI_CELL) {
				CellClass * cellptr = (CellClass *)TarCom;
				if (cellptr->Is_Tile_Destroyable_Cliff()) {
					Cell cell = Map.Closest_Passable_Cell(cellptr->CellID, Destination_Coord().As_Cell());
					Assign_Target(&Map[cell]);
				}
			}

			/*
			**	If the object that we are attacking is a building adjust the unit's
			**	max range so that people can stand far away from the buildings and
			**	hit them.
			*/
			if (TarCom->RTTI == RTTI_BUILDING) {
				BuildingClass * obj = (BuildingClass *)TarCom;
				if (obj != NULL) {
					maxrange += ((obj->Class->Width() + obj->Class->Height()) * (CELL_LEPTON / 4));
				}
			}

			/*
			**	Adjust the max range of an infantry unit for where he is standing
			**	in the room.
			*/
			double adjust;
			if (RTTI == RTTI_INFANTRY) {
				adjust = (double)maxrange - 281.6;
			} else {
				adjust = (double)maxrange - 179.2;
			}
			maxrange = (int)adjust;
			maxrange = (maxrange <= 0) ? 0 : maxrange;
			if (maxrange > 0 && maxrange <= 255) {
				maxrange = 255;
			}

			Coord tcoord = TarCom->Center_Coord();
			Coord trycoord;
			Cell tcell = tcoord.As_Cell();
			Cell trycell = tcell;

			Dir256 dir = DirType().Direction(tcoord, Center_Coord()).As_Dir256();

			/*
			 * Determine if the target is sitting upon a bridge.
			 */
			bool tarbridge = false;
			if (TarCom->RTTI == RTTI_CELL) {
				tarbridge = ((CellClass *)TarCom)->IsUnderBridge;
			} else {
				ObjectClass * obj = dynamic_cast<ObjectClass *>(TarCom);
				if (obj != NULL) {
					tarbridge = obj->IsOnBridge;
				}
			}

			MZoneType mzone = TClass->MZone;
			bool found = false;

			/*
			**	Sweep through the cells between the target and the unit, looking for
			**	a cell that the unit can enter but which is also within weapon range
			**	of the target. If after a reasonable search, no appropriate cell could
			**	be found, then the target will be assigned as the movement destination
			**	and let "the chips fall where they may."
			*/
			for (int range = maxrange; (double)range > CELL_LEPTON / 1.25; range -= CELL_LEPTON) {
				static int _angles[] = {
					DIR_MIN,
					DIR_STEP_32, -DIR_STEP_32,
					DIR_STEP_16, -DIR_STEP_16,
					DIR_STEP_16 + DIR_STEP_32, -(DIR_STEP_16 + DIR_STEP_32),
					DIR_STEP_8, -DIR_STEP_8,
					DIR_STEP_8 + DIR_STEP_16, -(DIR_STEP_8 + DIR_STEP_16),
					DIR_STEP_4, -DIR_STEP_4
				};

				for (int index = 0; index < ARRAY_SIZE(_angles); index++) {
					trycoord = Coord(Move_Coord(tcoord, DirType((Dir256)(dir + _angles[index])).Snap_To_256(), range).As_Cell(), 0);

					if (checkbuild && !Map[trycoord].Can_Build_Here()) continue;

					trycoord.Z = Map.Get_Height_GL(trycoord);
					if (Map[trycoord].IsUnderBridge) {
						trycoord.Z += BRIDGE_LEPTON_HEIGHT;
					}

					if (TClass->In_Range(trycoord, TarCom, Get_Class_Weapon_Data(primary)->Weapon)) {
						trycell = trycoord.As_Cell();
						if (Map.In_Local_Radar(trycell) && Map[trycell].Is_Clear_To_Move(TClass->Speed, false, false, Map.Get_Cell_Zone(Destination_Coord().As_Cell(), mzone, IsOnBridge), mzone)) {

							int maxdist = std::max(abs(trycell.X - tcell.X), abs(trycell.Y - tcell.Y));
							CellClass * cellptr = &Map[trycell];

							if (flyer) {
								found = true;
								break;
							}

							if (Search.Test_Cell_Walk(trycell, tcell, this, cellptr->IsUnderBridge, tarbridge, MZONE_NONE) <= maxdist + 8) {
								found = true;
								break;
							}

							Cell destcell = Destination_Coord().As_Cell();
							int destdist = std::max(abs(destcell.X - trycell.X), abs(destcell.Y - trycell.Y));
							if (Search.Test_Cell_Walk(destcell, trycell, this, Is_Moving_Onto_Bridge(), cellptr->IsUnderBridge, MZONE_NONE) <= destdist + 8) {
								found = true;
								break;
							}
						}
					}
				}
				if (found) break;
			}

			/*
			**	If a suitable intermediate location was found, then head toward it.
			**	Otherwise, head toward the enemy unit directly.
			*/
			if (found) {
				Assign_Destination(&Map[trycell]);
			} else if (TClass->IsHunterSeeker || (RTTI == RTTI_INFANTRY && ((InfantryClass *)this)->Class->IsVehicleThief)) {
				Assign_Destination(TarCom);
			} else {
				trycell = Map.Nearby_Location(trycell,
					TClass->Speed,
					Map.Get_Cell_Zone(Destination_Coord().As_Cell(), mzone, Is_Moving_Onto_Bridge()),
					mzone,
					Map[trycell].IsUnderBridge,
					Point2D(1, 1),
					false,
					true,
					false,
					false);
				Assign_Target(NULL);

				if (trycell != CELL_NONE) {
					Assign_Destination(&Map[trycell]);
				} else {
					Assign_Destination(NULL);
				}
			}
		}
	}
}


/***********************************************************************************************
 * FootClass::Mission_Guard_Area -- Causes unit to guard an area about twice weapon range.     *
 *                                                                                             *
 *    This mission routine causes the unit to scan for targets out to twice its weapon range   *
 *    from the home point. If a target was found, then it will be attacked. The unit will      *
 *    chase the target until it gets up to to its weapon range from the home position.         *
 *    In that case, it will return to home position and start scanning for another target.     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with time delay before calling this routine again.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/23/1994 JLB : Created.                                                                 *
 *   07/27/1995 JLB : Greatly simplified.                                                      *
 *=============================================================================================*/
int FootClass::Do_MISSION_GUARD_AREA(void)
{
	if (!House->Is_Human_Player()) {
		/*
		**	The navigation queue only needs to be processed if there is
		**	currently no navigation target for this object.
		*/
		if (NavQueue.Count() > 0 && NavCom == NULL && NavQueue[0] == ArchiveTarget) {
			AbstractClass * target = NavQueue[0];

			/*
			**	Check to see if the navigation queue even exists and
			**	has at least one valid entry. If it does, then process it by
			**	assigning the object's NavCom to the first entry on the list.
			*/
			if (target != NULL) {
				Assign_Destination(target);
				NavQueue.Delete_Index(0);

				/*
				**	If the navigation queue is to loop (indefinately), then append the
				**	target value from the first part to the end of the queue.
				*/
				if (IsNavQueueLoop) {
					NavQueue.Add(target);
				}
			}
		}
	}

	if (Is_Target_Cell(ArchiveTarget)) {
		BuildingClass * building = Map[ArchiveTarget->Center_Coord()].Cell_Building();
		if (building != NULL && House->Is_Ally(building->House) && !House->Is_Human_Player() && (RTTI != RTTI_UNIT || !static_cast<UnitClass *>(this)->Class->IsToHarvest)) {
			Cell nearby = Map.Nearby_Location((Cell)ArchiveTarget->Center_Coord(), SPEED_TRACK, Map.Get_Cell_Zone((Cell)PositionCoord));
			if (nearby != CELL_NONE) {
				ArchiveTarget = &Map[nearby];
			}
		}
	}

	if (RTTI == RTTI_UNIT && static_cast<UnitClass *>(this)->Class->IsToHarvest) {
		Assign_Mission(MISSION_HARVEST);
		Commence();
		return(1+Random_Pick(1, 10));
	}

	/*
	**	Ensure that the archive target is valid.
	*/
	if (ArchiveTarget == NULL && MissionQueue == MISSION_NONE) {
		ArchiveTarget = &Map[(Coord const &)PositionCoord];
	}

	/*
	**	If this is a bomber type infantry and the current target is a building, then go into
	**	sabotage mode if not already.
	*/
	InfantryClass * infantry = As_InfantryClass();
	BuildingClass * building = dynamic_cast<BuildingClass *>(TarCom);
	if (!House->Is_Human_Player() && infantry != NULL && (infantry->Class->IsBomber || infantry->Has_Ability(ABILITY_C4)) && Mission != MISSION_SABOTAGE && building != NULL && building->Class->IsRepairable) {
		Assign_Mission(MISSION_SABOTAGE);
		return(1);
	}

	/*
	**	Make sure that the unit has not strayed too far from the home position.
	**	If it has, then race back to it.
	*/
	int maxrange = (int)(Threat_Range(1) * 0.75);

	if (ArchiveTarget != NULL) {
		if (!IsFiring && NavCom == NULL && Distance(ArchiveTarget) > maxrange) {
			Assign_Target(NULL);
			Assign_Destination(ArchiveTarget);
		}

		if (TarCom == NULL) {
			Target_Something_Nearby(ArchiveTarget->Center_Coord(), THREAT_AREA);
			if (TarCom != NULL) {
				return(1);
			}
			Random_Animate();
		} else {
			Approach_Target();
		}
	}

	int dtime = Current_Mission_Control().Normal_Delay();
	if (RTTI == RTTI_AIRCRAFT) {
		dtime *= 2;
	}
	return(dtime + Random_Pick(1, 5));
}


/***********************************************************************************************
 * FootClass::Unlimbo -- Unlimbos object and performs special fixups.                          *
 *                                                                                             *
 *    This routine will make sure that the home position for the foot class object gets        *
 *    reset. This is necessary since the home position may change depending on the unit's      *
 *    transition between limbo and non-limbo condition.                                        *
 *                                                                                             *
 * INPUT:   coord    -- The coordinate to unlimbo the unit at.                                 *
 *                                                                                             *
 *          dir      -- The initial direction to give the unit.                                *
 *                                                                                             *
 * OUTPUT:  bool; Was the unit unlimboed successfully?                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/23/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FootClass::Unlimbo(Coord const & coord, Dir256 dir)
{
	/*
	**	Try to unlimbo the unit.
	*/
	if (BASECLASS::Unlimbo(coord, dir)) {

		Locomotion->Unlimbo();

		bool off = false;
		if (IonStormClass::Is_Ion_Storm_Active()) {
			if (Locomotion->Is_Ion_Sensitive()) {
				off = true;
			}
		}

		if (off) {
			Locomotion->Power_Off();
		} else {
			Locomotion->Power_On();
		}

		/*
		**	Mobile units are always revealed to the house that owns them.
		*/
		Revealed(House);

		/*
		**	Start in a still (non-moving) state.
		*/
		Path[0] = FACING_NONE;

		Cell cell = PositionCell;
		for (int face = FACING_FIRST; face < FACING_COUNT; face++) {
			Cell c = Adjacent_Cell(cell, FacingType(face));
			CellClass *cptr = &Map[c];
			cptr->AdjacentObjectCount++;
		}

		if (!In_Air()) {
			LastAdjacencyCell = cell;
		}

		double avoidance = TClass->ThreatAvoidanceCoefficient;
		ThreatAvoidanceCoefficient = avoidance;
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * FootClass::Take_Damage -- Handles taking damage to this object.                             *
 *                                                                                             *
 *    This routine intercepts the damage assigned to this object and if this object is         *
 *    a member of a team, it informs the team that the damage has occurred. The team may       *
 *    change it's priority or action based on this event.                                      *
 *                                                                                             *
 * INPUT:   damage      -- The damage points inflicted on the unit.                            *
 *                                                                                             *
 *          distance    -- The distance from the point of damage to the unit itself.           *
 *                                                                                             *
 *          warhead     -- The type of damage that is inflicted.                               *
 *                                                                                             *
 *          source      -- The perpetrator of the damage. By knowing who caused the damage,    *
 *                         the team know's who to "get even with".                             *
 *                                                                                             *
 * OUTPUT:  Returns with the result type of the damage.                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/30/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
ResultType FootClass::Take_Damage(int & damage, int distance, WarheadTypeClass const * warhead, TechnoClass * source, bool forced, bool no_crew)
{
	ResultType result = BASECLASS::Take_Damage(damage, distance, warhead, source, forced, no_crew);

	if (result == RESULT_ALREADY_DESTROYED) {
		return(result);
	}

	if (result != RESULT_NONE && Team) {

		Team->Took_Damage(this, result, source);

	} else {

		if (result != RESULT_DESTROYED && result != RESULT_NONE) {

			/*
			**	Determine if the target that is currently being attacked has a weapon that can
			**	do harm to a ground based unit. This information is needed so that an appropriate
			**	response will occur when damage is taken.
			*/
//			bool tweap = false;
//			if (As_Techno(TarCom)) {
//				tweap = (As_Techno(TarCom)->TClass->PrimaryWeapon != NULL);
//			}

			if (Team != NULL && Team->Class->IsWhiner && !House->Is_Human_Player()) {
				if (source == NULL) {
					return(result);
				}
				Base_Is_Attacked(source);
			}

			/*
			**	This ensures that if a unit is in sticky mode, then it will snap out of
			**	it when it takes damage.
			*/
			if (source != NULL && Current_Mission_Control().IsNoThreat && !Current_Mission_Control().IsZombie) {
				Enter_Idle_Mode();
			}

			#if 0
			/*
			**	If this object is not part of a team and it can retaliate for the damage, then have
			**	it try to do so. This prevents it from just sitting there and taking damage.
			*/
			if (Is_Allowed_To_Retaliate(source)) {

				int primary = What_Weapon_Should_I_Use(source->As_Target());
				if (In_Range(source, primary) || !House->IsHuman) {
					Assign_Target(source->As_Target());
				}

				if (Mission == MISSION_AMBUSH) {
					Assign_Mission(MISSION_HUNT);
				}

				/*
				**	Simple retaliation cannot occur because the source of the damage
				**	is too far away. If scatter logic is enabled, then scatter now.
				*/
				if (TarCom == NULL && NavCom == NULL && Rule->IsScatter) {
					Scatter(COORD_NONE, true);
				}

			} else {

				/*
				**	If this object isn't doing anything important, then scatter.
				*/
				if (Current_Mission_Control().IsScatter && !IsTethered && !IsDriving && TarCom == NULL && NavCom == NULL && RTTI != RTTI_AIRCRAFT && RTTI != RTTI_VESSEL) {
					if (!House->IsHuman || Rule->IsScatter) {
						Scatter(COORD_NONE, true);
					}
				}
			}
			#endif
		}
	}
	return(result);
}


/***********************************************************************************************
 * FootClass::Active_Click_With -- Initiates attack or move according to target clicked on.    *
 *                                                                                             *
 *    At this level, the object is known to have the ability to attack or move to the          *
 *    target specified (in theory). Perform the attack or move as indicated.                   *
 *                                                                                             *
 * INPUT:   target   -- The target clicked upon that will precipitate action.                  *
 *                                                                                             *
 * OUTPUT:  Returns with the type of action performed.                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/06/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FootClass::Active_Click_With(ActionType action, ObjectClass * object, bool is_waypoint)
{
	assert(object != NULL);

	switch (action) {
		case ACTION_GUARD_AREA:
			if (Can_Player_Fire() && Can_Player_Move()) {
				if (((RTTI == RTTI_INFANTRY &&
						((InfantryClass *)this)->Class->IsBomber) || Has_Ability(ABILITY_C4)) &&
						object->RTTI == RTTI_BUILDING &&
						((BuildingClass *)object)->Class->IsRepairable &&
						!House->Is_Ally(object)) {

					Player_Assign_Mission(MISSION_SABOTAGE, NULL, object);
				} else {
					Player_Assign_Mission(MISSION_GUARD_AREA, object);
				}
				return(true);
			}
			break;

		case ACTION_SELF:
			Player_Assign_Mission(MISSION_UNLOAD);
			return(true);

		case ACTION_ATTACK:
			if (Can_Player_Fire()) {
				Player_Assign_Mission(MISSION_ATTACK, object);
				return(true);
			}
			break;

		case ACTION_ENTER:
			if (Can_Player_Move() && object && Dynamic_Cast<TechnoClass *>(object) /*&& !((RadioClass *)object)->In_Radio_Contact()*/) {
				Player_Assign_Mission(MISSION_ENTER, NULL, object);
				return(true);
			}
			break;

		case ACTION_CAPTURE:
			if (Can_Player_Move()) {
				Player_Assign_Mission(MISSION_CAPTURE, NULL, object);
				return(true);
			}
			break;

		case ACTION_SABOTAGE:
			if (Can_Player_Move() && object->RTTI == RTTI_BUILDING && ((BuildingClass *)object)->Class->IsRepairable) {
				Player_Assign_Mission(MISSION_SABOTAGE, NULL, object);
				return(true);
			}
			break;

		case ACTION_NOMOVE:
			if (Map.Is_Shrouded(object->Center_Coord()) && !TClass->IsMoveToShroud) {
				return(false);
			}
			/// intentional fallthrough
		case ACTION_MOVE:
			if (Can_Player_Move()) {

				AbstractClass * targ = object;

				Coord center = object->Center_Coord();
				if (center.Z <= Map.Get_Height_GL(center)) {
					center.Z = Map.Get_Height_GL(center);
				}

				if (((TClass->IsSubterranean && !Rule->IsShroudedSubteranneanMovesAllowed) || RTTI == RTTI_AIRCRAFT) && Map.Is_Shrouded(center)) return(true);

				MZoneType mzone = TClass->MZone;
				Cell cell = object->Destination_Coord().As_Cell();
				bool in_radar = Map.In_Local_Radar(cell);
				CellClass * cptr = &Map[Destination_Coord()];

				if (mzone == MZONE_SUBTERANNEAN) mzone = MZONE_NORMAL;
				else if (mzone == MZONE_FLYER) mzone = MZONE_INFANTRY;

				bool moveanywhere = TClass->IsSubterranean || (RTTI == RTTI_INFANTRY && ((InfantryClass*)this)->Class->IsJumpJet) || RTTI == RTTI_AIRCRAFT;

				bool ontobridge = Is_Moving_Onto_Bridge();
				Coord coord = cell.As_Coord();
				coord.Z = Map.Get_Height_GL(coord);
				bool shrouded = Map.Is_Shrouded(coord);
				bool forcemove = !is_waypoint && (Keyboard->Down(Options.KeyForceMove1) || Keyboard->Down(Options.KeyForceMove2));

				if (moveanywhere && action == ACTION_NOMOVE) {
					Cell nearby = Map.Nearby_Location(cell, TClass->Speed, -1, mzone, Map[cell].IsUnderBridge, Point2D(1, 1), false, true, TClass->IsSubterranean && HeightAGL < 0, true);
					if (nearby != CELL_NONE) {
						targ = &Map[nearby];
					}
				} else {
					/*
					**	If the destination object is not the same zone, then pick a nearby location.
					*/
					if (!in_radar || !moveanywhere && (shrouded || action == ACTION_NOMOVE || forcemove && RTTI == RTTI_INFANTRY || !Map.Is_Same_Cell_Zone(cptr->CellID, cell, mzone, ontobridge, Map[cell].IsUnderBridge, false))) {
						Cell nearby = Map.Nearby_Location(cell, TClass->Speed, Map.Get_Cell_Zone(cptr->CellID, mzone, ontobridge), mzone, Map[cell].IsUnderBridge, Point2D(1, 1), false, true, false, true);
						if (nearby != CELL_NONE) {
							targ = &Map[nearby];
						}
					}
				}

				if (targ != NULL) {
					Player_Assign_Mission(MISSION_MOVE, NULL, targ);
				}
				return(true);
			}
			break;

		case ACTION_PATROL_WAYPOINT:
			return(false);

		case ACTION_ATTACK_SUPPORT: {
				WeaponDataStruct const * wdata = Get_Class_Weapon_Data(0);
				bool heals = (wdata != 0 && wdata->Weapon != 0 && wdata->Weapon->Attack < 0);
				Player_Assign_Mission(heals ? MISSION_GUARD_AREA : MISSION_GUARD, this, NULL);
			}
			return(true);

		default:
			return(true);
	}
	return(false);
}


/***********************************************************************************************
 * FootClass::Active_Click_With -- Performs action as a result of left mouse click.            *
 *                                                                                             *
 *    This routine performs the action requested when the left mouse button was clicked over   *
 *    a cell. Typically, this is just a move command.                                          *
 *                                                                                             *
 * INPUT:   action   -- The predetermined action that should occur.                            *
 *                                                                                             *
 *          cell     -- The cell number that the action should occur at.                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FootClass::Active_Click_With(ActionType action, Cell const & cell, bool is_waypoint)
{
	action = What_Action(cell, false, is_waypoint);

	if (!Map.In_Local_Radar(cell) && action != ACTION_NOMOVE) {
		return(false);
	}

	switch (action) {
		case ACTION_PATROL_WAYPOINT:
			if (Can_Player_Move()) {
				Cell moveto = Move_Order(cell, false);
				Player_Assign_Mission(MISSION_PATROL, NULL, &Map[moveto]);
				break;
			}
			return(false);

		case ACTION_GUARD_AREA:
			if ((Can_Player_Fire() || Is_Renovator()) && Can_Player_Move()) {
				Player_Assign_Mission(MISSION_GUARD_AREA, &Map[cell]);
				break;
			}
			return(false);

		case ACTION_HARVEST:
			Player_Assign_Mission(MISSION_HARVEST, NULL, &Map[cell]);
			break;

		case ACTION_MOVE:
			if (AllowVoice) {
				Coord coord = cell;
				coord.Z = Map.Get_Height_GL(coord);
				if (Map[coord].IsUnderBridge) {
					coord.Z += BRIDGE_LEPTON_HEIGHT;
				}
				int scenid = Scen->UniqueID;
				if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
					Scen->UniqueID = -3;
					((AnimTypeClass *)Rule->MoveFlash)->YSortAdjust = -5000;
				}
				AnimClass * moveflash = new AnimClass(Rule->MoveFlash, coord);
				if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
					Scen->UniqueID = scenid;
					Anims.Delete(moveflash);
					MoveFlashes.Add(moveflash);
				}
			}
			// Fall into next case.

		case ACTION_NOMOVE: {
			Cell moveto = Move_Order(cell, false);
			if (moveto != CELL_NONE) {
				Player_Assign_Mission(MISSION_MOVE, NULL, &Map[moveto]);
				break;
			}
		}
		return(false);

		case ACTION_ATTACK: {
			VeinholeMonsterClass * veinhole = VeinholeMonsterClass::Get_Monster_At(cell);
			if (veinhole != NULL && !veinhole->IsDead && veinhole->Strength > 0) {
				Player_Assign_Mission(MISSION_ATTACK, veinhole);
				break;
			} else {
				Player_Assign_Mission(MISSION_ATTACK, &Map[cell]);
				break;
			}
		}
		return(false);

		/*
		**	Engineer attempting to capture bridge to repair it
		*/
		case ACTION_CAPTURE:
			if (Can_Player_Move()) {
				Player_Assign_Mission(MISSION_CAPTURE, NULL, &Map[cell]);
				break;
			}
			return(false);

		case ACTION_ENTER:
			if (Can_Player_Move()) {
				Player_Assign_Mission(MISSION_ENTER, NULL, &Map[cell]);
				break;
			}
			return(false);

		case ACTION_SABOTAGE:
			Player_Assign_Mission(MISSION_SABOTAGE, NULL, &Map[cell] );
			break;

		case ACTION_ENTER_TUNNEL: {
			TubeClass * tunnel = Map[cell].Get_Tunnel();
			TubeClass * exittunnel = Map[(Cell)tunnel->Exit].Get_Tunnel();
			Cell exit = Adjacent_Cell((Cell)tunnel->Exit, Facing_Sub(exittunnel->EnterDir, FACING_180));
			Player_Assign_Mission(MISSION_MOVE, NULL, &Map[exit]);
			break;
		}
	}
	return(true);
}


/// <summary>
/// Puts the object into its idle state now that it has run out of orders.
/// This routine will end any piggyback locomotion and then look around for further work --
/// a queued route destination, a waypoint path to resume, or an archived target to head back
/// to -- before finally bringing the object to a halt.
/// </summary>
/// <param name="resume_waypoint">Should an interrupted waypoint path be resumed?</param>
/// <returns>bool; Was other work taken up instead of going idle?</returns>
bool FootClass::Enter_Idle_Mode(bool, bool resume_waypoint)
{
	if (!IsIdle) {
		IsIdle = true;

		if (IsToScatter) {
			IsToScatter = false;
			Scatter(COORD_NONE, true);
		}

		bool was_piggybacking = false;
		IPiggybackPtr piggy(Locomotion);
		if (piggy != NULL && piggy->Is_Ok_To_End()) {
			piggy->End_Piggyback(&Locomotion);
			was_piggybacking = true;
		}

		if (RouteQueue.Count() > 0) {
			Assign_Destination(RouteQueue[0], false);
			RouteQueue.Delete_Index(0);
			return(true);
		}

		if (CurrentMission != MISSION_PATROL || Status == 0) {
			if (CurrentPath != PATH_NONE && resume_waypoint) {
				WaypointClass * wp = PlayerPtr->Paths[CurrentPath]->Get_Waypoint(NextWaypoint);
				WaypointClass * next_wp = PlayerPtr->Paths[CurrentPath]->Get_Next_Waypoint(wp);
				Execute_Waypoint_Path(next_wp);
			}
		}

		if (was_piggybacking) {
			return(true);
		}

		if (RTTI == RTTI_INFANTRY) {
			AbstractClass *target = ArchiveTarget;
			if (target != NULL) {
				if (CurrentMission != MISSION_GUARD_AREA) {
					Assign_Mission(MISSION_MOVE);
					ArchiveTarget = NULL;
				}
				Assign_Destination(target, true);
			}
		}

		Set_Speed(0);
	}

	return(false);
}


/***********************************************************************************************
 * FootClass::Per_Cell_Process -- Perform action based on once-per-cell condition.             *
 *                                                                                             *
 *    This routine is called as this object moves from cell to cell. When the center of the    *
 *    cell is reached, check to see if any trigger should be sprung. For moving units, reduce  *
 *    the path to the distance to the target. This forces path recalculation in an effort to   *
 *    avoid units passing each other.                                                          *
 *                                                                                             *
 * INPUT:   why   -- Specifies the circumstances under which this routine was called.          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *   07/08/1995 JLB : Handles generic enter trigger event.                                     *
 *   07/16/1995 JLB : If next to a scanner and cloaked, then shimmer.                          *
 *=============================================================================================*/
void FootClass::Per_Cell_Process(PCPType why)
{
	Cell cell;

	if (why == PCP_END) {

		IsScattering = false;

		/*
		**	Clear any unloading flag if necessary.
		*/
		IsUnloading = false;

		if (LastAdjacencyCell != Cell(0, 0)) {
			if (Map.Cell_Region(LastAdjacencyCell) != Map.Cell_Region(PositionCell)) {
				int risk = Risk();
				HousesType owner = Owner();
				Get_Cell_Ptr()->Adjust_Threat(owner, risk);
				Map[LastAdjacencyCell].Adjust_Threat(owner, -risk);
			}
			int face;
			for (face = FACING_FIRST; face < FACING_COUNT; face++) {
				Cell c = Adjacent_Cell(LastAdjacencyCell, (FacingType)face);
				CellClass *cptr = &Map[c];
				cptr->AdjacentObjectCount--;
			}
			LastAdjacencyCell = PositionCell;
			for (face = FACING_FIRST; face < FACING_COUNT; face++) {
				Cell c = Adjacent_Cell(LastAdjacencyCell, (FacingType)face);
				CellClass *cptr = &Map[c];
				cptr->AdjacentObjectCount++;
			}
		}

		/*
		**	If adjacent to an enemy techno that has the ability to reveal a sub,
		**	then shimmer the cloaked object.
		*/
		if (Cloak == CLOAKED) {
			for (FacingType face = FACING_N; face < FACING_COUNT; face++) {
				cell = Adjacent_Cell(PositionCell, (FacingType)face);

				if (Map.In_Local_Radar(cell)) {
					TechnoClass const * techno = Map[cell].Cell_Techno();

					if (techno && !techno->House->Is_Ally(this) && (techno->TClass->IsScanner || techno->Has_Ability(ABILITY_SENSORS))) {
						Do_Shimmer();
						break;
					}
				}
			}
		}

		/*
		**	Shorten the path if the target is now within weapon range of this
		**	unit and this unit is on an attack type mission.
		*/
		int primary = What_Weapon_Should_I_Use(TarCom);
		bool inrange = false;
		FootClass const * foot = dynamic_cast<FootClass *>(TarCom);
		if (foot != NULL) {
			inrange = In_Range(foot->Likely_Coord(), primary);
		}

		if ((Mission == MISSION_RESCUE || Mission == MISSION_GUARD_AREA || Mission == MISSION_ATTACK || Mission == MISSION_HUNT) && inrange && RouteQueue.Count() == 0) {
			Assign_Destination(NULL);
			Path[0] = FACING_NONE;
		}

		cell = PositionCell;
		CellClass * cellptr = &Map[cell];

		/*
		**	Trigger event associated with the player entering the cell.
		*/
		if (Cloak != CLOAKED) {
			TagClass * tag = cellptr->Tag;
			int x = cell.X;
			int y = cell.Y;
			if (((!cellptr->IsUnderBridge && !cellptr->WasUnderBridge) || IsOnBridge) && tag != NULL) {
				tag->Spring(TEVENT_PLAYER_ENTERED, this, PositionCell);
			}

			/*
			**	Check for horizontal trigger crossing.
			*/
			if (cellptr->IsHorizontalLine) {
				for (int index = 0; index < Map.MapRect.Width; index++) {
					cellptr = &Map[Cell(index+Map.MapRect.X, y)];
					tag = cellptr->Tag;
					if (tag != NULL) {
						if (tag->Is_Cross_Horizontal()) {
							tag->Spring(TEVENT_CROSS_HORIZONTAL, this, PositionCell);
						}
					}
				}
			}

			/*
			**	Check for vertical trigger crossing.
			*/
			if (cellptr->IsVerticalLine) {
				for (int index = 0; index < Map.MapRect.Height; index++) {
					tag = Map[Cell(x, index+Map.MapRect.Y)].Tag;
					if (tag != NULL) {
						if (tag->Is_Cross_Vertical()) {
							tag->Spring(TEVENT_CROSS_VERTICAL, this, PositionCell);
						}
					}
				}
			}

			/*
			**	Check for zone entry trigger events.
			*/
			for (MapTriggerID = 0; MapTriggerID < MapTags.Count(); MapTriggerID++) {
				tag = MapTags[MapTriggerID];
				if (tag != NULL) {
					if (tag->Is_Enters_Zone()) {
						if (Map.Is_Same_Cell_Zone(tag->Get_Position(), Destination_Coord().As_Cell(), TClass->MZone, Map[tag->Get_Position()].IsUnderBridge, Is_Moving_Onto_Bridge())) {
							tag->Spring(TEVENT_ENTERS_ZONE, this, Destination_Coord().As_Cell());
						}
					}
				}
			}
		}

		BuildingClass * building = cellptr->Cell_Building();
		if (building != NULL) {
			if (building->Class->IsLaserFence && building->LaserFenceFrame < 8) {
				RTTIType rtti = RTTI;
				if (rtti > RTTI_NONE && (rtti <= RTTI_AIRCRAFT || rtti == RTTI_INFANTRY) && Strength > 0) {
					Take_Damage(Strength, 0, Rule->C4Warhead, building, true, true);
				}
			}
		}

		if (IsOnBridge && !Map[(Coord const &)PositionCoord].IsUnderBridge) {
			Fall_From_Height();
		}

		/*
		**	If any of these triggers cause this unit to be destroyed, then
		**	stop all further processing for this unit.
		*/
		if (!IsActive) return;

		if (!Map.In_Local_Radar((Cell const &)PositionCell) && Should_Delete_Off_Map()) {
			if (RTTI == RTTI_UNIT && TClass->IsHunterSeeker && TarCom != NULL) {
				TechnoClass * techno = Dynamic_Cast<TechnoClass *>(TarCom);
				WeaponTypeClass * weap = PrimaryWeapon;
				WarheadTypeClass const * wh = weap->WarheadPtr;
				int damage = weap->Attack;
				techno->Take_Damage(damage, 0, wh, this, true, true);
			}
			Delete_Me();
			return;
		}

		if (IsSelected && !House->Is_Player_Control() && Map.Is_Shrouded(PositionCoord)) {
			Unselect();
		}
	}

	if (CurrentPath != PATH_NONE) {
		WaypointPathClass * path = PlayerPtr->Paths[CurrentPath];
		WaypointClass * next_waypoint = path->Get_Waypoint(NextWaypoint);
		if (next_waypoint != NULL) {
			Cell wp_cell = next_waypoint->Location;
			if (Map.DraggedWaypoint == next_waypoint) {
				wp_cell = Map.DraggedWaypointCoord;
			}
			if (WaypointOffsetCell + wp_cell != WaypointTargetCell) {
				Execute_Waypoint_Path(next_waypoint);
			}
		} else {
			CurrentPath = PATH_NONE;
			NextWaypoint = NULL;
			WaypointOffsetCell = Cell(0, 0);
			WaypointTargetCell = Cell(0, 0);
		}
	}

	BASECLASS::Per_Cell_Process(why);
}


/***************************************************************************
 * FootClass::Override_Mission -- temporarily overrides a units mission    *
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
void FootClass::Override_Mission(MissionType mission, AbstractClass * tarcom, AbstractClass * navcom)
{
	SuspendedNavCom = NavCom;
	BASECLASS::Override_Mission(mission, tarcom, navcom);

	Assign_Destination(navcom);
}


/***************************************************************************
 * FootClass::Restore_Mission -- Restores an overridden mission            *
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
bool FootClass::Restore_Mission(void)
{
	if (BASECLASS::Restore_Mission()) {
		Assign_Destination(SuspendedNavCom);
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * FootClass::Receive_Message -- Movement related radio messages are handled here.             *
 *                                                                                             *
 *    This routine handles radio message that are related to movement. These are used for      *
 *    complex coordinated maneuvers.                                                           *
 *                                                                                             *
 * INPUT:   from     -- Pointer to the originator of this radio message.                       *
 *                                                                                             *
 *          message  -- The radio message that is being received.                              *
 *                                                                                             *
 *          param    -- The optional parameter (could be a movement destination).              *
 *                                                                                             *
 * OUTPUT:  Returns with the radio response appropriate to the message received. Usually the   *
 *          response is RADIO_ROGER.                                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/14/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
RadioMessageType FootClass::Receive_Message(RadioClass * from, RadioMessageType message, int & param)
{
	BuildingClass const * building = NULL;
	ObjectClass *object = NULL;

	switch (message) {

		/*
		**	Answers if this object is located on top of a service depot.
		*/
		case RADIO_ON_DEPOT:
			building = Map[Center_Coord()].Cell_Building();
			if (building == (RadioClass const *)from) {
				return(RADIO_ROGER);
			}
			return(RADIO_NEGATIVE);

		/*
		**	Intercept the repair request and if this object is moving, then no repair
		**	is possible.
		*/
		case RADIO_REPAIR:
			if (NavCom != NULL) return(RADIO_NEGATIVE);
			break;

		/*
		**	Something bad has happened to the object in contact with. Abort any coordinated
		**	activity with this object. Basically, ... run away! Run away!
		*/
		case RADIO_RUN_AWAY:
			if (In_Radio_Contact()) {
				if (NavCom == Contact_With_Whom()) {
					Assign_Destination(NULL);
				}
			}
			if (Mission == MISSION_SLEEP) {
				Assign_Mission(MISSION_GUARD);
				if (Ready_To_Commence()) {
					Commence();
				}
			}
			if (Mission == MISSION_ENTER) {
				Assign_Mission(MISSION_GUARD);
			}
			if (!IsRotating && NavCom == NULL) {
				Scatter(COORD_NONE, true, true);
			}
			break;

		/*
		**	Checks to see if this unit needs to move somewhere. If it is already in motion,
		**	then it doesn't need further movement instructions.
		*/
		case RADIO_NEED_TO_MOVE:
			param = (int)NavCom;
			if (NavCom == NULL || !Locomotion->Is_Moving()) {
				return(RADIO_ROGER);
			}
			return(RADIO_NEGATIVE);

		/*
		**	Radio request to move to location specified. Typically this is used
		**	for complex loading and unloading missions.
		*/
		case RADIO_MOVE_HERE:
			object = (ObjectClass *)param;
			{
				if (object != NULL && PositionCell == Cell(object->Center_Coord())) {
						return(RADIO_YEA_NOW_WHAT);
				} else {
					if (Mission == MISSION_GUARD && MissionQueue == MISSION_NONE) {
						Assign_Mission(MISSION_MOVE);
					}
					if (MissionQueue == MISSION_ENTER && Ready_To_Commence()) {
						Commence();
					}
					Assign_Destination((ObjectClass *)param);
					Shorten_Mission_Timer();
				}
			}
			return(RADIO_ROGER);

		/*
		**	Requests if this unit is trying to cooperatively load up. Typically, this occurs
		**	for passengers and when vehicles need to be repaired.
		*/
		case RADIO_TRYING_TO_LOAD:
			if (Mission == MISSION_ENTER || MissionQueue == MISSION_ENTER) {
				BASECLASS::Receive_Message(from, message, param);
				return(RADIO_ROGER);
			}
			break;
	}
	return(BASECLASS::Receive_Message(from, message, param));
}


/***********************************************************************************************
 * FootClass::Mission_Enter -- Enter (cooperatively) mission handler.                          *
 *                                                                                             *
 *    This mission handler will cooperatively coordinate the object to maneuver into the       *
 *    object it is in radio contact with. This is used by infantry when they wish to load      *
 *    into an APC as well as by vehicles when they wish to enter a repair facility.            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the number of game ticks before this routine should be called again.       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/15/1995 JLB : Created.                                                                 *
 *   09/22/1995 JLB : Modified to handle the "on hold" condition.                              *
 *=============================================================================================*/
int FootClass::Do_MISSION_ENTER(void)
{
	/*
	**	Find out who to coordinate with. If in radio contact, then this the transporter is
	**	defined. If not in radio contact, then try the archive target value to see if that
	**	is suitable.
	*/
	TechnoClass * contact = Contact_With_Whom();
	if (contact == NULL) {
		contact = Dynamic_Cast<TechnoClass *>(ArchiveTarget);
	}

	/*
	**	If in contact, then let the transporter handle the movement coordination.
	*/
	if (contact != NULL) {

		/*
		**	If the transport says to "bug off", then abort the enter mission. The transport may
		**	likely say all is 'ok' with the "RADIO ROGER", then try again later.
		*/
		if (Transmit_Message(RADIO_DOCKING, contact) != RADIO_ROGER && !IsTethered) {
			Transmit_Message(RADIO_OVER_OUT);
			Enter_Idle_Mode();
		} else {
			if (NavCom == NULL && RouteQueue.Count() > 0 ) {
				IPiggybackPtr piggy(Locomotion);
				if (piggy != NULL && piggy->Is_Ok_To_End()) {
					piggy->End_Piggyback(&Locomotion);
				}
				if (RouteQueue.Count() > 0) {
					Assign_Destination(RouteQueue[0], false);
					RouteQueue.Delete_Index(0);
				}
			}
		}

	} else {

		/*
		**	Since there is no potential object to enter, then abort this
		**	mission with some default standby mission.
		*/
		if (!Move_To_Object_Nearby()) {
			if (NavCom == NULL || NavCom->RTTI != RTTI_UNIT && NavCom->RTTI != RTTI_AIRCRAFT) {
				Enter_Idle_Mode();
			}
		}
		Commence();
	}

	return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
}


/***********************************************************************************************
 * FootClass::Assign_Destination -- Assigns specified destination to NavCom.                   *
 *                                                                                             *
 *    This routine will assign the specified target to the navigation computer. No legality    *
 *    checks are performed.                                                                    *
 *                                                                                             *
 * INPUT:   target   -- The target value to assign to the navigation computer.                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void FootClass::Assign_Destination(AbstractClass * target, bool)
{
	NavCom = target;

	if (NavCom != NULL) {
		if (ParticleSystems[ATTACHED_PARTICLE_FIRE] != NULL) {
			ParticleSystems[ATTACHED_PARTICLE_FIRE]->Delete_Me();
			ParticleSystems[ATTACHED_PARTICLE_FIRE] = NULL;
		}

		CLSID locoid;
		IPersistPtr persist(Locomotion);
		persist->GetClassID(&locoid);

		if (locoid == CLSID_HoverLocomotion && PathDelay == 0) {
			PathDelay = 1;
		}

		Locomotion->Move_To(NavCom->Destination_Coord());

	} else if (RTTI != RTTI_AIRCRAFT || (CurrentMission != MISSION_ATTACK && MissionQueue != MISSION_ATTACK) || TarCom == NULL) {
		Locomotion->Stop_Moving();
		NavCom = NULL;
	}

	/*
	**	Presume that the easiest path is tried first. As the findpath proceeds, when
	**	a failure occurs, this threshhold will be increased until path failure
	**	cannot be prevent. At this point, all movement should cease.
	*/
	IsToPathAroundBlockage = false;
	BlockagePathDelay = Rule->BlockagePathDelay;
	PathDelay = 0;
}


/***********************************************************************************************
 * FootClass::Detach_All -- Removes this object from the game system.                          *
 *                                                                                             *
 *    This routine will remove this object from the game system. This routine is called when   *
 *    this object is about to be deleted. All other objects should no longer reference this    *
 *    object in that case.                                                                     *
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
void FootClass::Detach_All(bool all)
{
	if (all) {
		if (Team && !ScenarioInit) {
			Team->Remove(this);
		}
	}

	if (!all) {
		if (In_Radio_Contact() && !House->Is_Ally(Contact_With_Whom())) {
			Transmit_Message(RADIO_OVER_OUT);
		}
	} else {
		Transmit_Message(RADIO_OVER_OUT);
	}

	BASECLASS::Detach_All(all);
}


/***********************************************************************************************
 * FootClass::Rescue_Mission -- Calls this unit to the rescue.                                 *
 *                                                                                             *
 *    This routine is called when the house determines that it should attack the specified     *
 *    target. This routine will determine if it can attack the target specified and if so,     *
 *    the amount of power it can throw at it. This returned power value is used to allow       *
 *    intelligent distribution of retaliation.                                                 *
 *                                                                                             *
 * INPUT:   target   -- The target that this object just might be assigned to attack and thus  *
 *                      how much power it can bring to bear should be returned.                *
 *                                                                                             *
 * OUTPUT:  Returns with the amount of power that this object can bring to bear against the    *
 *          potential target specified.                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int FootClass::Rescue_Mission(AbstractClass * tarcom)
{
	/*
	**	If the target specified is not legal, then it cannot be attacked. Always return
	**	zero in this case.
	*/
	if (tarcom == NULL) return(0);

	/*
	**	If the unit is already assigned to destroy the tarcom then we need
	**	to return a negative value which tells the computer to lower the
	**	desired threat rating.
	*/
	if (TarCom == tarcom) {
		return(-Risk());
	}

	/*
	**	If the unit is currently attacking a target that has a weapon then we
	**	cannot abandon it as it will destroy us if we return to base.
	*/
	if (TarCom != NULL) {
		TechnoClass * techno = Dynamic_Cast<TechnoClass *>(TarCom);
		if (techno != NULL && techno->Is_Weapon_Equipped()) {
			return(0);
		}
	}

	/*
	**	If the unit is in a harvest mission or is currently attacking
	**	something, or is not very effective, then it will be of no help
	**	at all.
	*/
	bool basedefense = Team != NULL && Team->Class->IsBaseDefense;
	if ((Team != NULL && !basedefense) || Mission == MISSION_HARVEST || !Risk()) {
		return(0);
	}

	/*
	**	Find the distance to the target modified by the range.  If the
	**	the distance is 0, then things are ok.
	*/
	int dist = Distance(tarcom) - Weapon_Range(0);
	int threat = Risk() * 1024;
	int speed = -1;
	if (dist > 0) {

		/*
		**	Next we need to figure out how fast the unit moves because this
		**	decreases the distance penalty.
		*/
		speed = std::max((unsigned)Get_Max_Speed(), (unsigned)1);

		int ratio = (speed > 0) ? std::max(dist / speed, 1) : 1;

		/*
		**	Finally modify the threat by the distance the unit is away.
		*/
		threat = std::max(threat/ratio, 1);
	}
	return(threat);
}


/***********************************************************************************************
 * FootClass::Death_Announcement -- Announces the death of a unit.                             *
 *                                                                                             *
 *    This routine is called when a unit (infantry, vehicle, or aircraft) is destroyed.        *
 *                                                                                             *
 * INPUT:   source   -- The perpetrator of this death.                                         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/01/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void FootClass::Death_Announcement(TechnoClass const * ) const
{
	if (IsOwnedByPlayer) {
		LastRadarEventCell = Destination_Coord().As_Cell();
		Speak(VOX_UNIT_LOST);
	}
}


/***********************************************************************************************
 * FootClass::Greatest_Threat -- Fetches the greatest threat to this object.                   *
 *                                                                                             *
 *    This routine will return with the greatest threat (best target) for this object. For     *
 *    movable ground object, they won't automatically return ANY target if this object is      *
 *    cloaked. Otherwise, cloaking is relatively useless.                                      *
 *                                                                                             *
 * INPUT:   method   -- The request method (bit flags) to use when scanning for a target.      *
 *                                                                                             *
 * OUTPUT:  Returns with the best target to attack. If there is no target that qualifies, then *
 *          TARGET_NONE is returned.                                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *   07/10/1996 JLB : Handles scan range limitation.                                           *
 *=============================================================================================*/
AbstractClass * FootClass::Greatest_Threat(ThreatType method, Coord const & coord, bool onlyenemy) const
{
	/*
	**	If the scan is forced to be limited, then limit the scan now.
	*/
	if (IsScanLimited) {
		method = ThreatType(method & ~THREAT_AREA);
		method = ThreatType(method | THREAT_RANGE);
	}

	/*
	**	If this object can cloak, then it won't select a target automatically.
	*/
	if (House->Is_Human_Player() && (Is_Allowed_To_Recloak() || Has_Ability(ABILITY_CLOAK)) && Mission == MISSION_GUARD) {
		return(NULL);
	}

	if (!(method & (THREAT_INFANTRY|THREAT_VEHICLES|THREAT_BUILDINGS|THREAT_TIBERIUM|THREAT_CIVILIANS|THREAT_POWER|THREAT_FACTORIES|THREAT_BASE_DEFENSE))) {
		method = ThreatType(method | THREAT_GROUND);
	}

	/*
	**	Perform the search for the target.
	*/
	AbstractClass * target = BASECLASS::Greatest_Threat(method, coord, onlyenemy);

	/*
	**	If no target could be located and this object is under scan range
	**	restrictions, then this restriction must be lifted now.
	*/
	if (IsScanLimited && target == NULL) {
		const_cast<FootClass*>(this)->IsScanLimited = false;
	}

	/*
	**	Return with final target found.
	*/
	return(target);
}


/***********************************************************************************************
 * FootClass::Detach -- Detaches a target from tracking systems.                               *
 *                                                                                             *
 *    This routine will detach the specified target from the tracking systems of this object.  *
 *    It will be removed from the navigation computer and any queued mission record.           *
 *                                                                                             *
 * INPUT:   target   -- The target to be removed from this object.                             *
 *                                                                                             *
 *          all      -- Is the unit really about to be eliminated? If this is true then even   *
 *                      friendly contact (i.e., radio) must be eliminated.                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1995 JLB : Created.                                                                 *
 *   07/24/1996 JLB : Removes target from NavQueue list.                                       *
 *=============================================================================================*/
void FootClass::Detach(AbstractClass const * target, bool all)
{
	int index;

	BASECLASS::Detach(target, all);

	if (Team == target) {
		Team = NULL;
	}

	if (Member == target && all) {
		if (target != NULL) {
			Member = Member->Member;
		}
	}

	if (ArchiveTarget == target) {
		ArchiveTarget = NULL;
	}

	if (SuspendedNavCom == target) {
		SuspendedNavCom = NULL;
	}

	/*
	**	If the navigation computer is assigned to the target, then the navigation
	**	computer must be cleared.
	*/
	if (NavCom == target) {
		CellClass * cptr;
		if (all || !target->Is_Techno() || (cptr = &Map[target->Center_Coord()], !cptr->Is_Sensed(House->HeapID))) {
			NavCom = NULL;
		}
		//Restore_Mission();
	}

	/*
	**	Remove the target from the NavQueue list as well.
	*/
	for (index = 0; index < NavQueue.Count(); index++) {
		if (NavQueue[index] == target) {
			NavQueue.Delete_Index(index);
			index--;
		}
	}

	for (index = 0; index < RouteQueue.Count(); index++) {
		if (RouteQueue[index] == target) {
			RouteQueue.Delete_Index(index);
			index--;
		}
	}
}


/***********************************************************************************************
 * FootClass::Offload_Tiberium_Bail -- Fetches the Tiberium to offload per step.               *
 *                                                                                             *
 *    This routine is called when a packet/package/bail of Tiberium needs to be offloaded      *
 *    from the object. This function is overridden for those objects that can contain          *
 *    Tiberium.                                                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of credits offloaded from the object.                      *
 *                                                                                             *
 * WARNINGS:   This routine must be called multiple times in order to completely offload the   *
 *             Tiberium. When this routine return 0, all Tiberium has been offloaded.          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int FootClass::Offload_Tiberium_Bail(void)
{
	return(0);
}


/***********************************************************************************************
 * FootClass::Can_Enter_Cell -- Checks to see if the object can enter cell specified.          *
 *                                                                                             *
 *    This routine examines the specified cell to see if the object can enter it. This         *
 *    function is to be overridden for objects that could have the possibility of not being    *
 *    allowed to enter the cell. Typical objects at the FootClass level always return          *
 *    MOVE_OK.                                                                                 *
 *                                                                                             *
 * INPUT:   cell     -- The cell to examine.                                                   *
 *                                                                                             *
 *          facing   -- The direction that this cell might be entered from.                    *
 *                                                                                             *
 * OUTPUT:  Returns with the move check result type. This will be MOVE_OK if there is not      *
 *          blockage. There are various other values that represent other blockage types.      *
 *          The value returned will indicated the most severe reason why entry into the cell   *
 *          is blocked.                                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
MoveType FootClass::Can_Enter_Cell(CellClass const * cell, FacingType, int cell_height, CellClass const *, bool use_locomotor_enter_check) const
{
	if (Locomotion != NULL && use_locomotor_enter_check) {
		Cell cellnum = cell->CellID;
		return(Locomotion->Can_Enter_Cell(cellnum));
	}
	return(MOVE_OK);
}


/// <summary>
/// Determines if this object can step between two adjacent cells.
/// This routine is used by the Can_Enter_Cell logic to police elevation changes. The step is
/// refused unless the two cells are level, joined by a ramp, or joined by a bridge that this
/// object is permitted to travel over or under.
/// </summary>
/// <param name="current_cell">The cell being stepped out of.</param>
/// <param name="facing">The direction of travel, or FACING_NONE to merely resolve the
/// height.</param>
/// <param name="cell_height">The elevation to travel at. This is filled in with the bridge deck
/// height when the cell turns out to be spanned by one.</param>
/// <param name="onto_bridge">Set to true when the step climbs onto a bridge deck.</param>
/// <param name="adjacent_cell">The cell being stepped into, or NULL to derive it from the
/// facing.</param>
/// <returns>Returns with MOVE_OK if the step is legal, or MOVE_NO if the elevation change
/// forbids it.</returns>
MoveType FootClass::Can_Reach(CellClass const * current_cell, FacingType facing, int & cell_height, bool & onto_bridge, CellClass const * adjacent_cell) const
{
	if (adjacent_cell == NULL) {
		adjacent_cell = &Map[Adjacent_Cell(current_cell->CellID, Facing_Sub(facing, FACING_180))];
	}

	if (facing != FACING_NONE) {
		if (adjacent_cell == NULL || current_cell == NULL) {
			return(MOVE_OK);
		}

		if (cell_height == -1 && adjacent_cell->IsUnderBridge) {
			cell_height = adjacent_cell->Height + BRIDGE_CELL_HEIGHT;
			if (!current_cell->IsBridgeTraversable) {
				return(MOVE_NO);
			}
		}

		int height_difference = adjacent_cell->IsUnderBridge ? adjacent_cell->Height : cell_height;
		int current_height = current_cell->Height;

		switch (abs(height_difference - current_height)) {
			case 0:
				if ((!current_cell->IsUnderBridge || !current_cell->IsBridgeTraversable || !adjacent_cell->IsUnderBridge) && cell_height != -1 && cell_height != current_height) {
					return(MOVE_NO);
				}
				break;

			case 1:
				if ((height_difference - current_height) > 0) {
					if (current_cell->Ramp == 0) {
						return(MOVE_NO);
					}
				} else if (adjacent_cell->Ramp == 0) {
					return(MOVE_NO);
				}
				break;

			case BRIDGE_CELL_HEIGHT:
				if (adjacent_cell->Height == current_height - BRIDGE_CELL_HEIGHT) {
					if (cell_height != current_height) return(MOVE_NO);
					if (!adjacent_cell->IsUnderBridge) return(MOVE_NO);
				}
				if (current_height != adjacent_cell->Height - BRIDGE_CELL_HEIGHT) return(MOVE_OK);
				if (!current_cell->IsUnderBridge) return(MOVE_NO);
				if (!current_cell->IsBridgeTraversable) return(MOVE_NO);
				onto_bridge = true;
				break;

			default:
				return(MOVE_NO);
		}
	} else {
		if (cell_height == -1 && current_cell->IsUnderBridge) {
			cell_height = current_cell->Height + BRIDGE_CELL_HEIGHT;
		}
	}
	return(MOVE_OK);
}


/***********************************************************************************************
 * FootClass::Can_Demolish -- Checks to see if this object can be sold back.                   *
 *                                                                                             *
 *    This routine determines if it is legal to sell the object back. A foot class object can  *
 *    only be sold back if it is sitting on a repair bay.                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Was the object successfully sold back?                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FootClass::Can_Demolish(void) const
{
	switch ((RTTIType)RTTI) {
		case RTTI_UNIT:
		case RTTI_AIRCRAFT:
			if (In_Radio_Contact() &&
				Contact_With_Whom()->RTTI == RTTI_BUILDING &&
				((BuildingClass *)Contact_With_Whom())->Class->IsCanUnitRepair &&
				Distance(Contact_With_Whom()->Center_Coord()) < CELL_LEPTON / 2) {

				return(true);
			}
			break;

		default:
			break;
	}
	return(BASECLASS::Can_Demolish());
}


/***********************************************************************************************
 * FootClass::Sell_Back -- Causes this object to be sold back.                                 *
 *                                                                                             *
 *    When an object is sold back, a certain amount of money is refunded to the owner and then *
 *    the object is removed from the game system.                                              *
 *                                                                                             *
 * INPUT:   control  -- The action to perform. The only supported action is "1", which means   *
 *                      to sell back.                                                          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void FootClass::Sell_Back(int control)
{
	if (control != 0) {
		if (House->Is_Player_Control()) {
			Speak(VOX_UNIT_SOLD);
			Sound_Effect(Rule->SellSound);
		}
		House->Refund_Money(Refund_Amount());
		Stun();
		Limbo();
		Delete_Me();
	}
}


/***********************************************************************************************
 * FootClass::Likely_Coord -- Fetches the coordinate the object will be at shortly.            *
 *                                                                                             *
 *    This routine comes in handy when determining where a travelling object will be at        *
 *    when considering the amount of time it would take for a normal unit to travel one cell.  *
 *    Using this information, an intelligent "approach target" logic can be employed.          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the coordinate the object is at or soon will be.                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
Coord FootClass::Likely_Coord(void) const
{
	return(Target_Coord());
}


/***********************************************************************************************
 * FootClass::Adjust_Dest -- Adjust candidate movement cell to account for formation.          *
 *                                                                                             *
 *    This routine modify the specified cell if the unit is part of a formation. The           *
 *    adjustment will take into consideration the formation relative offset from the           *
 *    (presumed) center cell specified.                                                        *
 *                                                                                             *
 * INPUT:   cell  -- The cell to presume as the desired center point of the formation.         *
 *                                                                                             *
 * OUTPUT:  Returns with the cell that should be used as the actual destination. If this       *
 *          object is part of a formation, then the cell location will be appropriately        *
 *          adjusted.                                                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/11/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
Cell FootClass::Adjust_Dest(Cell const & cell) const
{
	return(cell);
}


/***********************************************************************************************
 * FootClass::Handle_Navigation_List -- Processes the navigation queue.                        *
 *                                                                                             *
 *    This routine will process the navigation queue. If the queue is present and valid and    *
 *    there is currently no navigation target assigned to this object, then the first entry    *
 *    of the queue will be assigned. The remaining entries will move down. If the queue is     *
 *    to be processed as a circular list, then the first entry is appended to the end.         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine might end up assigning a movement destination.                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void FootClass::Handle_Navigation_List(void)
{
	/*
	**	The navigation queue only needs to be processed if there is
	**	currently no navigation target for this object.
	*/
	if (NavCom == NULL && NavQueue.Count()) {
		AbstractClass * target = NavQueue[0];

		/*
		**	Check to see if the navigation queue even exists and
		**	has at least one valid entry. If it does, then process it by
		**	assigning the object's NavCom to the first entry on the list.
		*/
		if (target != NULL) {
			Assign_Destination(target);
			NavQueue.Delete_Index(0);

			/*
			**	If the navigation queue is to loop (indefinately), then append the
			**	target value from the first part to the end of the queue.
			*/
			if (IsNavQueueLoop) {
				NavQueue.Add(target);
			}
		}
	}
}


/***********************************************************************************************
 * FootClass::Queue_Navigation_List -- Add a target to the objects navigation list.            *
 *                                                                                             *
 *    This routine will append the destination target to the object's NavQueue list. After     *
 *    doing so, if the object is not doing anything important, then it will be started on      *
 *    that destination. This is functionally the same as Assign_Destination, but it stores     *
 *    the target to the NavQueue first.                                                        *
 *                                                                                             *
 * INPUT:   target   -- The movement target destination to append the queue.                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The queue is of finite size and any queue requests that would exceed that size  *
 *             are ignored. If there are no queue entries pending and the unit is not          *
 *             otherwise occupied, then the queue target might be carried directly into the    *
 *             NavCom.                                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void FootClass::Queue_Navigation_List(AbstractClass * target)
{
	if (target != NULL) {
		int count = NavQueue.Count();
		/*
		**	If the target is this object itself, then this indicates that the
		**	queue list is to be processed as a loop. Otherwise, just tack the
		**	navigation target to the end of the list.
		*/
		if (target == this && count > 0) {
			IsNavQueueLoop = true;
		} else {
			if (count == 0) {
				IsNavQueueLoop = false;
			}
			NavQueue.Add(target);
		}

		/*
		**	If this object isn't doing anything, then start acting on the
		**	navigation queue now.
		*/
		if (NavCom == NULL && Mission == MISSION_GUARD) {
			TechnoClass *tptr = Contact_With_Whom();
			if (tptr == NULL || tptr->RTTI != RTTI_BUILDING || !((BuildingClass*)tptr)->Class->IsWeaponsFactory) {
				Enter_Idle_Mode();
			}
		}
	}
}


/***********************************************************************************************
 * FootClass::Clear_Navigation_List -- Clears out the navigation queue.                        *
 *                                                                                             *
 *    This routine will clear out any values in the navigation queue. This is the preferred    *
 *    way of aborting a navigation queue for a unit. If the unit is already travelling, it     *
 *    won't be interrupted by this routine.                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This will clear the navigation list but not the navigation computer. Thus a     *
 *             unit will still travel to its current immediate destination.                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void FootClass::Clear_Navigation_List(void)
{
	NavQueue.Clear();
}


/***********************************************************************************************
 * FootClass::Is_Allowed_To_Leave_Map -- Checks to see if it can leave the map and the game.   *
 *                                                                                             *
 *    This routine will determine if this object has permission to leave the map and thus      *
 *    leave the game. Typical objects with this permission are transports used to drop of      *
 *    reinforcements.                                                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Does this object have permission to travel off the map edge and leave the    *
 *                game?                                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FootClass::Is_Allowed_To_Leave_Map(void) const
{
	/*
	**	If the unit hasn't entered the map yet, then don't allow leave the game.
	*/
	if (!IsLocked) return(false);

	/*
	**	A unit that isn't marked as a loaner is a gift to the player. Such objects can never
	**	leave the map unless they are part of a team that gives it special permision.
	*/
	if (!TClass->IsTrain && !IsALoaner && Mission != MISSION_RETREAT && (Team == NULL || !Team->Is_Leaving_Map())) return(false);

	return(true);
}


/***********************************************************************************************
 * FootClass::Is_Recruitable -- Determine if this object is recruitable as a team members.     *
 *                                                                                             *
 *    This will examine this object to determine if it is suitable as a team recruit. Some     *
 *    objects are disqualified if they are otherwise premptively occupied.                     *
 *                                                                                             *
 * INPUT:   house -- Pointer to the house that is trying to recruit this object.               *
 *                                                                                             *
 * OUTPUT:  bool; Is this object suitable for recruitment by a team.                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/14/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FootClass::Is_Recruitable(HouseClass const * house) const
{
	/*
	**	If not of the correct house presuasion, then recruitment is not allowed.
	*/
	if (house != NULL && house != House) {
		return(false);
	}

	/*
	**	If the object is not a playing member of the game, then don't consider it available.
	*/
	if (IsInLimbo) {
		return(false);
	}

	/*
	**	If it is already part of another team, then it is not available for
	**	general recruitment.
	*/
	if (Team != NULL) {
		return(false);
	}

	/*
	**	If it is currently in a mission the precludes recruitment into a team, then
	**	return with this information.
	*/
	if (!Is_Recruitable_Mission(Mission)) {
		return(false);
	}

	if (!IsTeamRecruitable) {
		return(false);
	}

	/*
	**	It was not disqualified for general team recruitment, so return that
	**	it is available.
	*/
	return(true);
}


/***********************************************************************************************
 * FootClass::Is_On_Priority_Mission -- Checks to see if this object should be given priority. *
 *                                                                                             *
 *    Some objects are on an important mission that must succeed. If the object is on such     *
 *    a mission, then it will be more aggressive in its movement action.                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is this object on a priority mission?                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FootClass::Is_On_Priority_Mission(void) const
{
	if (Mission == MISSION_ENTER) return(true);
	return(false);
}


/***********************************************************************************************
 * FootClass::Mission_Retreat -- Handle reatreat from map mission for mobile objects.          *
 *                                                                                             *
 *    This will try to make this mobile object leave the map. It does this by assigning a      *
 *    movement destination that is located off the edge of the map.                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before calling this routine        *
 *          again.                                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int FootClass::Do_MISSION_RETREAT(void)
{
	enum {
		FIND_EDGE,
		TRAVELLING
	};

	switch (Status) {

		/*
		**	Find a suitable edge to travel to and then assign destination there.
		*/
		case FIND_EDGE:
			if (NavCom != NULL) {
				Status = TRAVELLING;
			} else {

				Cell cell(0,0);

				/*
				**	If this is part of a team, then pick the edge where the team as likely
				**	entered from.
				*/
				if (Team != NULL && Team->Class->Get_Origin() != CELL_NONE) {
					cell = Map.Calculated_Cell(House->Control.Edge, Team->Class->Get_Origin(), Center_Coord().As_Cell(), TClass->Speed);
				}

				/*
				**	If an edge hasn't been found, then try to find one that is not based on any
				**	team information.
				*/
				if (cell == CELL_NONE) {
					cell = Map.Calculated_Cell(House->Control.Edge, CELL_NONE, Center_Coord().As_Cell(), TClass->Speed);
				}

				assert(cell == Cell(0,0));		// An edge cell must be found!

				Assign_Destination(&Map[cell]);
				Status = TRAVELLING;
			}
			break;

		/*
		**	While travelling, monitor that all is proceeding according to plan.
		*/
		case TRAVELLING:
			if (NavCom == NULL) {
				Status = FIND_EDGE;
			}
			break;
	}

	return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
}


/// <summary>
/// Determines the visual character of this object.
/// The locomotor gets first say, so that a submerging or burrowing locomotion can hide the
/// object. Failing that, the normal cloaking rules decide the matter.
/// </summary>
/// <param name="raw">Should the check be based on the unmodified cloak condition?</param>
/// <returns>Returns with the visual character to use when displaying this object.</returns>
VisualType FootClass::Visual_Character(bool raw, HouseClass const * house) const
{
	VisualType visual = VISUAL_NORMAL;

	if (Locomotion != NULL) {
		visual = Locomotion->Visual_Character(raw);
	}
	if (visual == VISUAL_NORMAL) {
		visual = BASECLASS::Visual_Character(raw, house);
	}

	return(visual);
}


/***********************************************************************************************
 * FootClass::AI -- Handle general movement AI.                                                *
 *                                                                                             *
 *    This basically just sees if this object is within weapon range of the target and if      *
 *    so, it will stop movement so that firing may commence. This prevents the occasional      *
 *    case of an attacker driving right up to the defender before firing.                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/17/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void FootClass::AI(void)
{
	BASECLASS::AI();

	if (IsActive) {
		IsIdle = false;

		if (TClass->IsTiberiumHeal || Has_Ability(ABILITY_TIBERIUM_HEAL)) {
			if (Strength > 0 && HealthRatio < Rule->ConditionGreen) {
				if (Map[Center_Coord()].Land_Type() == LAND_TIBERIUM) {
					if ((Frame % int(Rule->TiberiumHeal * TICKS_PER_MINUTE)) == 0) {
						int step = TClass->Repair_Step();
						step = std::max(step, 1);
						Strength += step;
						if (HealthRatio > Rule->ConditionGreen) {
							Strength = Techno_Type_Class()->MaxStrength;
						}
					}
				}
			}
		}

		if (Locomotion != NULL && !IsSinking && !IsFalling) {
			Locomotion->Process();
			if (!IsActive) {
				return;
			}
			if (Locomotion->Is_Moving_Now() && (Frame % TClass->WalkRate) == 0) {
				TotalFramesWalked++;
			}
		}


		if ((Frame & 63) == 63 && NavCom == NULL && !IsOnBridge && Get_Cell_Ptr()->Ramp == 0 && Is_On_Elevation() && HeightAGL == 0) {
			Scatter(Coord(0,0,0), true);
		}

		IPiggybackPtr piggy(Locomotion);
		if (piggy != NULL) {
			if (piggy->Is_Ok_To_End()) {
				piggy->End_Piggyback(&Locomotion);
			}
		}

		IonBlastYDrawOffset = 0;

		Enter_Object_Nearby();
	}

// FootClass::Per_Cell_Process does this function already.
#ifdef OBSOLETE
	if (IsActive) {
		if (!IsScattering && !IsTethered && !IsInLimbo && RTTI != RTTI_AIRCRAFT && TarCom != NULL && In_Range(TarCom)) {
			Assign_Destination(NULL);
		}
	}
#endif
}


/// <summary>
/// Draws the voxel body of this object.
/// This override shifts the draw position by whatever offset the locomotor asks for, so that
/// a bobbing or lurching locomotion carries the artwork along with it.
/// </summary>
void FootClass::Draw_Voxel(VoxelDataStruct const & voxeldata, int frame, int key, VoxelIndexClass * cache, Rect const & cliprect, Point2D const & point, Matrix3D const & matrix, int brightness, ShapeFlags_Type flags) const
{
	Matrix3D mtx = matrix;
	Point2D pt = point;

	if (Locomotion != NULL) {
		pt = Point2D(Locomotion->Draw_Point()) + point;
	}

	BASECLASS::Draw_Voxel(voxeldata, frame, key, cache, cliprect, pt, mtx, brightness, flags);
}


/// <summary>
/// Fetches the depth buffer bias to render this object with.
/// The locomotor's own adjustment is combined with the fudge factor for whatever the object
/// happens to be passing -- a column, a tunnel, a cliff or a bridge -- so that it sorts in
/// front of or behind the terrain as the type data asks.
/// </summary>
/// <returns>Returns with the amount to bias this object's depth by when it is drawn.</returns>
int FootClass::Get_Z_Adjust(void) const
{
	int adjust = 0;
	if (Locomotion != NULL) {
		adjust = Locomotion->Z_Adjust();
	}

	int column = TClass->ZFudgeColumn * Get_Z_Fudge_Column();
	int tunnel = TClass->ZFudgeTunnel * Get_Z_Fudge_Tunnel();
	int cliff = TClass->ZFudgeCliff * Get_Z_Fudge_Cliff();
	int bridge = 0;
	if (Is_Z_Fudge_Bridge()) {
		bridge = TClass->ZFudgeBridge;
	}

	int max_fudge = std::max(column, tunnel);
	max_fudge = std::max(max_fudge, cliff);
	max_fudge = std::max(max_fudge, bridge);

	adjust += BASECLASS::Get_Z_Adjust();
	adjust += max_fudge;
	return(adjust);
}


/// <summary>
/// Fetches the depth gradient to render this object with.
/// The locomotor decides, so that a climbing or diving locomotion can lay its object into the
/// depth buffer at the appropriate slope. Objects without a locomotor stand upright.
/// </summary>
/// <returns>Returns with the Z gradient to use, which is ZGRAD_90DEG by default.</returns>
ZGradientType FootClass::Get_Z_Gradient(void) const
{
	if (Locomotion != NULL) {
		return(Locomotion->Z_Gradient());
	}
	return(ZGRAD_90DEG);
}


/// <summary>
/// Draws the ground shadow for this object's voxel body.
/// The locomotor has the final say over whether a shadow is appropriate at all -- a burrowed
/// or submerged object casts none -- and over where the shadow should be offset to.
/// </summary>
void FootClass::Draw_Voxel_Shadow(VoxelDataStruct const & voxeldata, int layer_index, int key, VoxelIndexClass * cache, Rect const & cliprect, Point2D const & point, Matrix3D const & matrix, bool force_cache) const
{
	if (Locomotion != NULL && Locomotion->Is_To_Have_Shadow() == (boolean)true) {
		Point2D drawpoint = point;
		if (Locomotion != NULL) {
			drawpoint = Point2D(Locomotion->Shadow_Point()) + point;
		}
		Techno_Draw_Voxel_Shadow(voxeldata, layer_index, key, cache, cliprect, drawpoint, matrix, force_cache);
	}
}


/// <summary>
/// Fetches the speed this object should be traveling at.
/// This is the type's maximum speed after the owner house's ground speed bias, any veteran
/// speed ability, and the object's current throttle have all been taken into account. A unit
/// that is carrying the flag is deliberately hobbled.
/// </summary>
/// <returns>Returns with the current speed of the object.</returns>
int FootClass::Current_Speed(void)
{
	int speed = Get_Max_Speed() * House->GroundspeedBias * SpeedBias;
	if (Has_Ability(ABILITY_FASTER)) {
		speed *= (1 + Rule->VeteranSpeed);
	}
	speed *= Speed;
	if (RTTI == RTTI_UNIT && ((UnitClass *)this)->Flagged != HOUSE_NONE) {
		speed /= 2;
	}
	return(speed);
}


/// <summary>
/// Draws this object.
/// Foot objects are rendered by their derived classes, each of which knows whether it is a
/// shape or a voxel, so this override deliberately does nothing.
/// </summary>
void FootClass::Draw_It(Point2D const &, Rect const &) const
{
	// Do nothing.
}


/***********************************************************************************************
 * DriveClass::Limbo -- Prepares vehicle and then limbos it.                                   *
 *                                                                                             *
 *    This routine removes the occupation bits for the vehicle and also handles cleaning up    *
 *    any vehicle reservation bits. After this, it then proceeds with limboing the unit.       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the vehicle limboed?                                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/22/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FootClass::Limbo(void)
{
	if (!IsInLimbo) {
		Cell cell = LastAdjacencyCell;
		for (FacingType face = FACING_FIRST; face < FACING_COUNT; face++) {
			Cell newcell = Adjacent_Cell(cell, face);
			CellClass * cptr = &Map[newcell];
			cptr->AdjacentObjectCount--;
		}
		Stop_Driver();
		if (Locomotion != NULL) {
			Locomotion->Mark_All_Occupation_Bits(0);
		}
	}
	return(BASECLASS::Limbo());
}


/// <summary>
/// Lists the members every moving object carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void FootClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(CurrentPath);
	stream.Serialize(WaypointOffsetCell);
	stream.Serialize(WaypointTargetCell);
	stream.Serialize(ThreatAvoidanceCoefficient);
	stream.Serialize(TotalFramesWalked);
	stream.Serialize(LastPathingCell);
	stream.Serialize(LastAdjacencyCell);
	stream.Serialize(LastTubeCoord);
	stream.Serialize(Speed);
	stream.Serialize(SpeedBias);
	stream.Serialize(RouteQueue);
	stream.Serialize(NavCom);
	stream.Serialize(SuspendedNavCom);
	stream.Serialize(NavQueue);
	stream.Serialize(Team);
	stream.Serialize(Member);
	stream.Serialize(PatrolCell);
	stream.Serialize(Path);
	stream.Serialize(PathDelay);
	stream.Serialize(TryTryAgain);
	stream.Serialize(BaseAttackTimer);
	stream.Serialize(BlockagePathDelay);

	/*
	 * The locomotor is a COM sub-object rather than a member, so it persists itself onto
	 * the raw stream through OLE. The one being replaced is released first, since loading
	 * hands back a fresh interface pointer rather than filling this one in.
	 */
	if (stream.Is_Saving()) {
		IPersistStreamPtr persist(Locomotion);
		OleSaveToStream(persist, stream.Get_Stream());
	} else {
		if (Locomotion != NULL) {
			((ILocomotion *)Locomotion)->Release();
		}
		Locomotion.Detach();
		OleLoadFromStream(stream.Get_Stream(), IID_ILocomotion, (LPVOID *)&Locomotion);
	}

	stream.Serialize(HeadToCoord);
	stream.Serialize(CurrentTube);
	stream.Serialize(CurrentTubeDir);
	stream.Serialize(NextWaypoint);
	stream.Serialize(IsToScatter);
	stream.Serialize(IsScanLimited);
	stream.Serialize(IsInitiated);
	stream.Serialize(IsNewNavCom);
	stream.Serialize(IsPlanningToLook);
	stream.Serialize(IsDeploying);
	stream.Serialize(IsFiring);
	stream.Serialize(IsRotating);
	stream.Serialize(IsUnloading);
	stream.Serialize(IsNavQueueLoop);
	stream.Serialize(IsScattering);
	stream.Serialize(IsIdle);
	stream.Serialize(IonBlastYDrawOffset);
	stream.Serialize(IsCrushing);
	stream.Serialize(IsOccupyingCell);
	stream.Serialize(IsToPathAroundBlockage);
	stream.Serialize(IsDroppedFromTeam);
}


/// <summary>
/// Fetches the display layer this object belongs in.
/// The locomotor makes this decision, since it is the part that knows whether the object is
/// currently on the ground, in the air, or under it.
/// </summary>
/// <returns>Returns with the layer this object should be rendered in.</returns>
LayerType FootClass::In_Which_Layer(void) const
{
	return(Locomotion->In_Which_Layer());
}


/// <summary>
/// Sets this object's location on the map.
/// This routine will lift the object off the map before moving it and set it back down
/// afterward, so that the cells it is recorded as occupying stay correct.
/// </summary>
void FootClass::Set_Coord(Coord const & coord)
{
	if (IsDown) {
		Mark(MARK_UP);
		BASECLASS::Set_Coord(coord);
		Mark(MARK_DOWN);
	} else {
		BASECLASS::Set_Coord(coord);
	}
}


/// <summary>
/// Attaches a drop pod locomotor to this object.
/// This routine piggybacks a ballistic locomotor onto whatever the object was using, so
/// that it can be dropped from orbit. The original locomotor takes over again once the
/// descent is finished.
/// </summary>
void FootClass::Link_DropPod(void)
{
	ILocomotionPtr locomotion = Locomotion;
	ILocomotionPtr ballistic(CLSID_BallisticLocomotion);
	ballistic->Link_To_Object(this);
	IPiggybackPtr piggy(ballistic);
	piggy->Begin_Piggyback(locomotion);
	Locomotion = ballistic;

}


/// <summary>
/// Handles an idle object that may be sitting on a repair dock.
/// This routine is called when the object has run out of orders. It drops the object into
/// its idle mode, but withholds the automatic advance to the next waypoint when the object
/// is deliberately entering a building that can repair it.
/// </summary>
/// <returns>bool; Did the object pick up further orders instead of falling idle?</returns>
bool FootClass::Is_Docked_For_Repair(void)
{
	if (NavCom == NULL && TarCom == NULL) {
		TechnoClass * whom = Contact_With_Whom();
		BuildingClass * bptr = NULL;
		if (whom != NULL && whom->RTTI == RTTI_BUILDING) {
			bptr = (BuildingClass *)whom;
		}
		bool going_to_repair = (CurrentMission == MISSION_ENTER && bptr != NULL && bptr->Class->IsCanUnitRepair);
		return(Enter_Idle_Mode(false, going_to_repair == false));
	}
	return(false);
}


/// <summary>
/// Stops whatever animation goes with this object's movement.
/// Derived classes override this routine to halt their walking or turning artwork when the
/// object comes to rest. At this level there is nothing to stop.
/// </summary>
void FootClass::Stop_Movement_Animation(void)
{
	//nothing
}


/// <summary>
/// Can this object reach where the other object is headed?
/// This routine is a convenience for the zone test. It compares against the other object's
/// destination rather than where it happens to be standing at this instant.
/// </summary>
/// <returns>bool; Are the two in the same movement zone?</returns>
bool FootClass::Is_In_Same_Zone_As(ObjectClass const * object) const
{
	return(Is_In_Same_Zone(object->Destination_Coord().As_Cell()));
}


/// <summary>
/// Adds this object's movement state to the consistency check.
/// This routine is used by the network code to prove that every machine agrees about where
/// this object is going and what it is doing on the way.
/// </summary>
void FootClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(ThreatAvoidanceCoefficient);
	crc(LastPathingCell.Length());
	crc(LastTubeCoord.Length());
	crc(Speed);
	crc(SpeedBias);
	if (NavCom != NULL) crc(NavCom->Fetch_ID());
	if (SuspendedNavCom != NULL) crc(SuspendedNavCom->Fetch_ID());
	if (Team != NULL) crc(Team->Fetch_ID());
	if (Member != NULL) crc(Member->Fetch_ID());
	crc((int)PathDelay);
	crc(TryTryAgain);
	crc((int)BaseAttackTimer);
	crc(HeadToCoord.Length());
	crc(CurrentTube);
	crc(CurrentTubeDir);
	crc(IsScanLimited);
	crc(IsInitiated);
	crc(IsNewNavCom);
	crc(IsPlanningToLook);
	crc(IsDeploying);
	crc(IsFiring);
	crc(IsRotating);
	crc(IsUnloading);
	crc(IsNavQueueLoop);
	crc(IsScattering);
	crc(IsIdle);
	crc(IonBlastYDrawOffset);
}


/// <summary>
/// Can this object cloak itself again?
/// Some objects are forbidden from cloaking while they are still under way, so they must
/// come to a halt before they can disappear.
/// </summary>
/// <returns>bool; Is the object allowed to recloak?</returns>
bool FootClass::Is_Allowed_To_Recloak(void) const
{
	if (BASECLASS::Is_Allowed_To_Recloak()) {
		if (!TClass->IsCloakStop || !Locomotion->Is_Moving_Now()) {
			return(true);
		}
	}
	return(false);
}


/// <summary>
/// Fetches the location this object is traveling toward.
/// An object making its way through a tunnel reports the tunnel exit, since that is where
/// it will surface. An object with nowhere to be reports its own center.
/// </summary>
/// <returns>Returns with the coordinate being headed for.</returns>
Coord FootClass::Destination_Coord(void) const
{
	if (CurrentTube >= TUBE_FIRST) {
		Cell exit = Tubes[CurrentTube]->Exit;
		return(exit.As_Coord());
	} else {
		Coord tmp = Locomotion->Head_To_Coord();
		Coord head_to = tmp;
		if (head_to != COORD_NONE) {
			return(head_to);
		}
		return(Center_Coord());
	}
}


/// <summary>
/// Handles this object changing hands.
/// This routine remembers where the object was standing at the moment it was captured, so
/// that its adjacency tracking remains valid under the new owner.
/// </summary>
/// <returns>bool; Was the object captured?</returns>
bool FootClass::Captured(HouseClass * newowner)
{
	if (BASECLASS::Captured(newowner)) {
		if (!In_Air()) {
			LastAdjacencyCell = PositionCell;
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Is this object considered slow?
/// Slow is meant in the slow-witted sense. A team that has IsGuardSlower set will tweak
/// itself to better guard the members that answer true -- typically the unarmed, the
/// unpromotable, and anything that would rather deploy than fight.
/// </summary>
/// <returns>bool; Is the object considered slow?</returns>
bool FootClass::Is_Considered_Slow(void)
{
	/*
	 * Slow is used in slow-witted sense here.
	 * When a unit that is considered "slow" is part of a team which has IsGuardSlower set,
	 * the team gets tweaked to better guard this unit.
	 */

	if (Get_Class_Weapon_Data(0) == NULL) {
		return(true);
	}
	if (TClass->Level == -1) {
		return(true);
	}
	if (TClass->DeploysInto != NULL) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Handles this object being unable to make any progress.
/// This routine gives a hunting object the chance to forget its orders and look for
/// something else to do, rather than grinding away against whatever is in its path.
/// </summary>
void FootClass::On_Movement_Blocked(void)
{
	if (Mission == MISSION_HUNT) {
		Assign_Target(NULL);
		Assign_Destination(NULL);
	}
}


/// <summary>
/// Draws the line that shows this object's current order.
/// This routine marks the two ends of the order -- the firing point and its target, or the
/// object and its destination -- and joins them with a line clipped to the tactical map.
/// The line only appears for a short while after the order is given.
/// </summary>
void FootClass::Draw_Action_Line(void) const
{
	if (!TarCom && !NavCom) {
		return;
	}

	if (ActionLineTimer.Value() > 0) {

		Coord start_coord;
		Coord end_coord;
		ColorType color;

		if (TarCom != NULL) {
			start_coord = Turret_Coord();
			end_coord = Predict_Target_Coord();
			color = RED;
		} else {
			start_coord = PositionCoord;
			if (RouteQueue.Count() == 0) {
				end_coord = NavCom->Center_Coord();
			} else {
				end_coord = RouteQueue[RouteQueue.Count() - 1]->Center_Coord();
			}
			color = GREEN;
			if (Map.In_Radar(end_coord.As_Cell()) && Map[end_coord].IsUnderBridge) {
				end_coord.Z = BRIDGE_LEPTON_HEIGHT + Map.Get_Height_GL(end_coord);
			}
		}

		Point2D start_point;
		Point2D end_point;

		TacticalMap->Coord_To_Pixel(start_coord, start_point);
		TacticalMap->Coord_To_Pixel(end_coord, end_point);

		start_point.Y += TacticalRect.Y;
		end_point.Y += TacticalRect.Y;

		CompositeSurface->Fill_Rect(Intersect(TacticalRect, Rect(start_point - Point2D(2, 2), 3, 3)), NormalDrawer->Convert_Pixel(color));
		CompositeSurface->Fill_Rect(Intersect(TacticalRect, Rect(end_point - Point2D(2, 2), 3, 3)), NormalDrawer->Convert_Pixel(color));

		Rect tacticalr = TacticalRect;
		if (Clip_Line_To_Rect(start_point, end_point, tacticalr)) {
			CompositeSurface->Draw_Line(start_point, end_point, NormalDrawer->Convert_Pixel(color));
		}
	}
}


/// <summary>
/// Fetches how strongly this object should steer around danger.
/// This routine is used by the path finder when weighting the threat of the regions it
/// passes through. A team that has been told to avoid threats forces its members to the
/// maximum avoidance.
/// </summary>
/// <returns>Returns with the avoidance coefficient, where zero ignores threats entirely.</returns>
double FootClass::Threat_Avoidance_Value(void) const
{
	if (Team != NULL && Team->Class->AvoidThreats) {
		return(1.0);
	}
	return(ThreatAvoidanceCoefficient);
}


/// <summary>
/// Should this object be quietly removed once it wanders off the map?
/// This routine is consulted as objects reach the edge of the world. A team member only
/// qualifies once its team is actually leaving, and a train is spared while its destination
/// is still within the local view.
/// </summary>
/// <returns>bool; Should the object be deleted?</returns>
bool FootClass::Should_Delete_Off_Map(void)
{
	if (!IsLocked) {
		return(false);
	}
	if (Team != NULL && !Team->Is_Leaving_Map()) {
		return(false);
	}
	if (TClass->IsTrain && NavCom != NULL && Map.In_Local_Radar(NavCom->Center_Coord())) {
		return(false);
	}
	return(true);
}


/// <summary>
/// Assigns this object to one of the player's waypoint paths.
/// This routine is used when a recorded path is handed to the object, and also to clear
/// that assignment again.
/// </summary>
/// <param name="path">The path to follow, or PATH_NONE to clear the assignment.</param>
/// <param name="index">The waypoint within the path to head for first.</param>
void FootClass::Set_Waypoint_Path(PathType path, char index)
{
	if (path != PATH_NONE) {
		CurrentPath = path;
		NextWaypoint = index;
		WaypointClass *wpc = PlayerPtr->Paths[path]->Get_Waypoint(index);
		WaypointTargetCell = wpc->Location.As_Cell();
	} else {
		CurrentPath = PATH_NONE;
		NextWaypoint = 0;
		WaypointTargetCell = Cell(0,0);
	}
	WaypointOffsetCell = Cell(0,0);
}


/// <summary>
/// Carries out the order recorded at a waypoint.
/// This routine performs the same click the player would have performed on the waypoint's
/// cell, shifting the order to a nearby reachable cell when the waypoint itself is occupied
/// or cannot be walked to directly. A waypoint that cannot be acted upon at all abandons
/// the path.
/// </summary>
/// <param name="waypoint">The waypoint to act upon. A NULL waypoint abandons the path.</param>
void FootClass::Execute_Waypoint_Path(WaypointClass * waypoint)
{
	bool action_succeeded = false;

	if (IsOnPatrol) {
		IsOnWaypointPatrol = true;
	}

	if (waypoint != NULL) {
		PlayerPtr->Fetch_Waypoint_Data(waypoint, CurrentPath, NextWaypoint);

		Coord waypoint_coord = waypoint->Location;
		ObjectClass * occupying_object = Map[waypoint_coord].Cell_Occupier();
		TechnoClass * occupying_techno = Dynamic_Cast<TechnoClass *>(occupying_object);

		WaypointTargetCell = waypoint_coord;

		bool was_voice_allowed = AllowVoice;
		AllowVoice = false;

		Cell waypoint_cell = waypoint_coord;

		ActionType waypoint_action = ACTION_NONE;
		Cell waypoint_offset;
		Cell adjusted_waypoint;
		CellClass * adjusted_cell = NULL;
		CellClass * waypoint_map_cell = NULL;
		ActionType adjusted_action = ACTION_NONE;
		int waypoint_zone = -1;
		int adjusted_zone = -1;
		int destination_zone = -1;
		MoveType entry_result = MOVE_OK;
		int walk_distance_limit = 0;
		bool is_direct_waypoint = true;
		bool should_click_occupier = false;

		if (occupying_techno != NULL) {
			BuildingClass * building;
			bool needs_repair;
			bool needs_reload;
			bool is_allied_building;

			ActionType occupier_action = What_Action(occupying_object, true);

			if (PlayerPtr->Is_Ally(occupying_techno->House)) {
				if (occupying_techno->RTTI == RTTI_BUILDING) {
					building = (BuildingClass *)occupying_techno;

					needs_repair = (building->Class->IsCanUnitRepair &&
						Strength < (int)TClass->MaxStrength &&
						RTTI != RTTI_INFANTRY);

					needs_reload = (building->Class->IsCanUnitReload &&
						Ammo < TClass->MaxAmmo &&
						Ammo != -1);

					is_allied_building = building->House->Is_Ally(PlayerPtr);

					if ((needs_repair || needs_reload) &&
						is_allied_building &&
						occupier_action != ACTION_SELECT &&
						occupier_action != ACTION_TOGGLE_SELECT) {
						should_click_occupier = true;
					}
				}
			} else if (occupier_action != ACTION_TOGGLE_SELECT &&
					   occupier_action != ACTION_SELECT &&
					   occupier_action != ACTION_MOVE &&
					   occupier_action != ACTION_NOMOVE) {
				should_click_occupier = true;
			}

			if (!should_click_occupier) {
				/*
				 * The waypoint cell is occupied but the occupier wasn't clicked on.
				 * Try to shift the waypoint to a reachable nearby cell, then carry
				 * on with the normal waypoint processing.
				 */
				Cell nearby_cell = Map.Nearby_Location(
					waypoint_cell,
					TClass->Speed,
					Map.Get_Cell_Zone(waypoint_cell, TClass->MZone, Map[waypoint_cell].IsUnderBridge),
					TClass->MZone,
					false,
					Point2D(1, 1),
					false,
					true,
					false,
					true,
					Cell(0, 0));

				if (nearby_cell != CELL_NONE) {
					WaypointOffsetCell = nearby_cell - waypoint_cell;
					is_direct_waypoint = false;
				}
			}
		}

		if (should_click_occupier) {
			ActionType click_action = What_Action(occupying_object, true);
			action_succeeded = Active_Click_With(click_action, occupying_object, true);
			WaypointOffsetCell = Cell(0, 0);
		} else {

			waypoint_action = What_Action(waypoint_cell, false, true);
			waypoint_offset = WaypointOffsetCell;
			adjusted_waypoint = waypoint_cell + waypoint_offset;
			adjusted_cell = &Map[adjusted_waypoint];
			waypoint_map_cell = &Map[waypoint_cell];
			adjusted_action = What_Action(adjusted_waypoint, false, true);

			waypoint_zone = Map.Get_Cell_Zone(waypoint_cell, TClass->MZone, waypoint_map_cell->IsUnderBridge);
			adjusted_zone = Map.Get_Cell_Zone(adjusted_waypoint, TClass->MZone, adjusted_cell->IsUnderBridge);
			destination_zone = Map.Get_Cell_Zone(Destination_Coord().As_Cell(), TClass->MZone, Is_Moving_Onto_Bridge());
			entry_result = Can_Enter_Cell(adjusted_cell, FACING_NONE,
				adjusted_cell->Height + (BRIDGE_CELL_HEIGHT * adjusted_cell->IsUnderBridge), 0, true);

			walk_distance_limit = std::max(abs((int)waypoint_offset.X), abs((int)waypoint_offset.Y));
			walk_distance_limit = std::min(5, walk_distance_limit + 3);

			if (waypoint_action != adjusted_action && is_direct_waypoint ||
				entry_result != MOVE_OK ||
				adjusted_zone != waypoint_zone ||
				destination_zone != adjusted_zone ||
				is_direct_waypoint && Search.Test_Cell_Walk(adjusted_waypoint, waypoint_cell, this,
					adjusted_cell->IsUnderBridge, waypoint_map_cell->IsUnderBridge, MZONE_NONE) > walk_distance_limit) {
				action_succeeded = Active_Click_With(waypoint_action, waypoint_cell, true);
				WaypointOffsetCell = Cell(0, 0);
			} else {
				action_succeeded = Active_Click_With(waypoint_action, adjusted_waypoint, true);
				WaypointTargetCell = adjusted_waypoint;
			}
		}

		if (was_voice_allowed) {
			AllowVoice = true;
		}
	}

	if (!action_succeeded) {
		CurrentPath = PATH_NONE;
		NextWaypoint = 0;
		WaypointOffsetCell = Cell(0, 0);
		WaypointTargetCell = Cell(0, 0);
	}

	IsOnWaypointPatrol = false;
}


/***********************************************************************************************
 * FootClass::Tiberium_Check -- Search for and head toward nearest available Tiberium patch.   *
 *                                                                                             *
 *    This routine is used to move a harvester to a place where it can load up with            *
 *    Tiberium. It will return true only if it can start harvesting. Otherwise, it sets        *
 *    the navigation computer toward the nearest Tiberium and lets the unit head there         *
 *    automatically.                                                                           *
 *                                                                                             *
 * INPUT:   center   -- Reference to the center of the radius scan.                            *
 *                                                                                             *
 *          x,y      -- Relative offset from the center cell to perform the check upon.        *
 *                                                                                             *
 * OUTPUT:  bool; Is it located directly over a Tiberium patch?                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FootClass::Tiberium_Check(Cell & center)
{
	/*
	**	If the specified offset from the origin will cause it
	**	to spill past the map edge, then abort this cell check.
	*/
	if (!Map.In_Local_Radar(center)) return(false);

	if ((Session.Type != GAME_NORMAL || (!IsOwnedByPlayer || !Map.Is_Shrouded(center.As_Coord(Map.Get_Height_GL(center)))))) {
		if (!Map.Is_Same_Cell_Zone(Destination_Coord().As_Cell(), center, TClass->MZone, Is_Moving_Onto_Bridge(), false, false)) return(false);
		CellClass * cptr = &Map[center];
		if (!Can_Enter_Cell(cptr) && cptr->Land_Type() == LAND_TIBERIUM) {
			return(true);
		}
	}
	return(false);
}


/***********************************************************************************************
 * FootClass::Goto_Tiberium -- Searches for and heads toward tiberium.                         *
 *                                                                                             *
 *    This routine will cause the unit to search for and head toward nearby Tiberium. When     *
 *    the Tiberium is reached, then this routine should not be called again until such time    *
 *    as additional harvesting is required. When this routine returns false, then it should    *
 *    be called again until such time as it returns true.                                      *
 *                                                                                             *
 * INPUT:   rad = size of ring to search                                                       *
 *                                                                                             *
 * OUTPUT:  Has the unit reached Tiberium and harvesting should begin?                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/22/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FootClass::Goto_Tiberium(int rad, bool allow_weighted)
{
	if (NavCom == NULL) {
		Cell newcell = Search_For_Tiberium(rad, allow_weighted);
		if (newcell != CELL_NONE) {
			if (newcell == Center_Coord().As_Cell()) {
				return(true);
			}
			Assign_Destination(&Map[newcell]);
		}
	}
	return(false);
}


/// <summary>
/// Finds the most valuable Tiberium patch within reach.
/// This routine is used by the harvest mission to pick somewhere to go. A computer
/// controlled harvester in a skirmish or multiplayer game defers to the weighted search
/// instead, so that the house's harvesters do not all pile onto one patch.
/// </summary>
/// <param name="rad">The limit of the search, expressed in cells from this object.</param>
/// <param name="allow_weighted">Is the weighted search allowed to be used?</param>
/// <returns>Returns with the cell of the Tiberium found. Otherwise, CELL_NONE is
/// returned.</returns>
Cell FootClass::Search_For_Tiberium(int rad, bool allow_weighted)
{
	Cell bestcell = CELL_NONE;

	if (!House->Is_Human_Player() && RTTI == RTTI_UNIT && ((UnitClass *)this)->Class->IsToHarvest && allow_weighted && Session.Type != GAME_NORMAL) {
		return(Search_For_Tiberium_Weighted(rad));
	}

	Cell center = Center_Coord().As_Cell();
	if (Map[center].Land_Type() == LAND_TIBERIUM) {
		return(center);
	} else {

		/*
		**	Perform a ring search outward from the center.
		*/
		Cell cell(0,0);
		int bestvalue = -1;
		for (int radius = 1; radius < rad; radius++) {
			for (int x = -radius; x <= radius; x++) {
				cell = center + Cell(x, -radius);
				if (Tiberium_Check(cell)) {
					int value = Map[cell].Tiberium_Value();
					if (value > bestvalue) {
						bestvalue = value;
						bestcell = cell;
					}
				}

				cell = center + Cell(x, +radius);
				if (Tiberium_Check(cell)) {
					int value = Map[cell].Tiberium_Value();
					if (value > bestvalue) {
						bestvalue = value;
						bestcell = cell;
					}
				}

				cell = center + Cell(-radius, x);
				if (Tiberium_Check(cell)) {
					int value = Map[cell].Tiberium_Value();
					if (value > bestvalue) {
						bestvalue = value;
						bestcell = cell;
					}
				}

				cell = center + Cell(+radius, x);
				if (Tiberium_Check(cell)) {
					int value = Map[cell].Tiberium_Value();
					if (value > bestvalue) {
						bestvalue = value;
						bestcell = cell;
					}
				}
			}
			if (bestvalue != -1) break;
		}
	}
	return(bestcell);
}


/// <summary>
/// Finds a Tiberium patch, weighted by richness and by distance.
/// This routine picks at random from the reachable patches, favoring the rich ones close to
/// home. It is used in place of the plain search so that a house's harvesters spread out
/// across the field instead of all converging on the same patch.
/// </summary>
/// <param name="rad">The limit of the search, expressed in cells from this object.</param>
/// <returns>Returns with the cell of the Tiberium chosen. Otherwise, CELL_NONE is
/// returned.</returns>
Cell FootClass::Search_For_Tiberium_Weighted(int rad)
{
	DiscreteDistributionClass<CellClass> celldist;

	Cell center = Center_Coord().As_Cell();
	if (Map[center].Land_Type() == LAND_TIBERIUM) {
		return(center);
	}

	int numharv = House->AUQuantity.Value(Rule->HarvesterUnit[0]->HeapID);
	if (numharv < 1) {
		numharv = 1;
	}

	double weight;
	/*
	**	Perform a ring search outward from the center.
	*/
	for (int radius = 1; radius < rad; radius++) {
		int ringspan = radius * 2;
		int share = ringspan / numharv;
		bool northrun = false;
		bool southrun = false;
		bool westrun = false;
		bool eastrun = false;
		int divisor = 1;
		if (share >= 1) {
			divisor = share;
		}
		double scale = 1.0 / (double)divisor;
		for (int x = -radius; x <= radius; x++) {
			Cell cell;
			cell = Cell(x, -radius) + center;
			if (Tiberium_Check(cell)) {
				if (!northrun) {
					CellClass *cptr = &Map[cell];
					if (cptr->Tiberium_Value() * scale < 1.0) {
						weight = 1.0;
					} else {
						weight = cptr->Tiberium_Value() * scale;
					}
					int intweight = (int)weight;
					northrun = true;
					celldist.Add(cptr, intweight);
				}
			} else {
				northrun = false;
			}

			cell = Cell(x, +radius) + center;
			if (Tiberium_Check(cell)) {
				if (!southrun) {
					CellClass *cptr = &Map[cell];
					if (cptr->Tiberium_Value() * scale < 1.0) {
						weight = 1.0;
					} else {
						weight = cptr->Tiberium_Value() * scale;
					}
					int intweight = (int)weight;
					southrun = true;
					celldist.Add(cptr, intweight);
				}
			} else {
				southrun = false;
			}

			cell = Cell(-radius, x) + center;
			if (Tiberium_Check(cell)) {
				if (!westrun) {
					CellClass *cptr = &Map[cell];
					if (cptr->Tiberium_Value() * scale < 1.0) {
						weight = 1.0;
					} else {
						weight = cptr->Tiberium_Value() * scale;
					}
					int intweight = (int)weight;
					westrun = true;
					celldist.Add(cptr, intweight);
				}
			} else {
				westrun = false;
			}

			cell = Cell(+radius, x) + center;
			if (Tiberium_Check(cell)) {
				if (!eastrun) {
					CellClass *cptr = &Map[cell];
					celldist.Add(cptr, cptr->Tiberium_Value() * scale < 1.0 ? 1.0 : cptr->Tiberium_Value() * scale);
					eastrun = true;
				}
			} else {
				eastrun = false;
			}
		}
	}

	CellClass *cellptr = celldist.Sample();
	if (cellptr != NULL) {
		return(cellptr->Fetch_CellID());
	}
	return(CELL_NONE);
}


/// <summary>
/// Finds the nearest harvestable weed patch.
/// This routine is used by the weed harvesting logic to pick somewhere to go. Only patches
/// the harvester can actually reach are considered.
/// </summary>
/// <param name="rad">The limit of the search, expressed in cells from this object.</param>
/// <returns>Returns with the cell of the weed found. Otherwise, CELL_NONE is returned.</returns>
Cell FootClass::Search_For_Weed(int rad)
{
	Cell bestcell = CELL_NONE;
	Cell center = Center_Coord().As_Cell();
	if (Weed_Check(center, 0, 0)) {
		return(center);
	} else {

		/*
		**	Perform a ring search outward from the center.
		*/
		Cell cell(0,0);
		for (int radius = 1; radius < rad; radius++) {
			for (int x = -radius; x <= radius; x++) {
				cell = center;
				if (Weed_Check(cell, x, -radius)) {
					bestcell = cell;
				}

				cell = center;
				if (Weed_Check(cell, x, +radius)) {
					bestcell = cell;
				}

				cell = center;
				if (Weed_Check(cell, -radius, x)) {
					bestcell = cell;
				}

				cell = center;
				if (Weed_Check(cell, +radius, x)) {
					bestcell = cell;
				}
			}
			if (bestcell != CELL_NONE) break;
		}
	}
	return(bestcell);
}


/***********************************************************************************************
 * FootClass::Weed_Check -- Search for and head toward nearest available Weed patch.           *
 *                                                                                             *
 *    This routine is used to move a harvester to a place where it can load up with            *
 *    Weed. It will return true only if it can start harvesting. Otherwise, it sets            *
 *    the navigation computer toward the nearest Weed and lets the unit head there             *
 *    automatically.                                                                           *
 *                                                                                             *
 * INPUT:   center   -- Reference to the center of the radius scan.                            *
 *                                                                                             *
 *          x,y      -- Relative offset from the center cell to perform the check upon.        *
 *                                                                                             *
 * OUTPUT:  bool; Is it located directly over a Weed patch?                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FootClass::Weed_Check(Cell & center, int x, int y)
{
	Cell cell = center + Cell(x, y);

	/*
	**	If the specified offset from the origin will cause it
	**	to spill past the map edge, then abort this cell check.
	*/
	if (!Map.In_Local_Radar(cell)) return(false);

	/*
	 * Pass the probed cell back through the center reference. The
	 * caller reads it back to learn which cell satisfied the check.
	 */
	center = cell;

	if ((Session.Type != GAME_NORMAL || (!IsOwnedByPlayer || !Map.Is_Shrouded(center.As_Coord(Map.Get_Height_GL(center)))))) {
		if (!Map.Is_Same_Cell_Zone(Destination_Coord().As_Cell(), center, TClass->MZone, Is_Moving_Onto_Bridge(), false, false)) return(false);
		CellClass * cptr = &Map[center];
		if (!Can_Enter_Cell(cptr) && cptr->Land_Type() == LAND_WEEDS && cptr->OverlayData >= OVERLAYDATA_FIRST_SOLID_VEIN) {
			return(true);
		}
	}
	return(false);
}


/***********************************************************************************************
 * FootClass::Goto_Weed -- Searches for and heads toward weed.                                 *
 *                                                                                             *
 *    This routine will cause the unit to search for and head toward nearby Weed. When         *
 *    the Weed is reached, then this routine should not be called again until such time        *
 *    as additional harvesting is required. When this routine returns false, then it should    *
 *    be called again until such time as it returns true.                                      *
 *                                                                                             *
 * INPUT:   rad = size of ring to search                                                       *
 *                                                                                             *
 * OUTPUT:  Has the unit reached Weed and harvesting should begin?                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/22/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FootClass::Goto_Weed(int rad)
{
	if (NavCom == NULL) {
		Cell newcell = Search_For_Weed(rad);
		if (newcell != CELL_NONE) {
			if (newcell == Center_Coord().As_Cell()) {
				return(true);
			}
			Assign_Destination(&Map[newcell]);
		}
	}
	return(false);
}


/// <summary>
/// Is this object about to step onto a bridge?
/// An object traveling through a tunnel is never considered to be heading onto a bridge,
/// since its destination is underground rather than on the deck.
/// </summary>
/// <returns>bool; Is the object moving onto a bridge?</returns>
bool FootClass::Is_Moving_Onto_Bridge(void) const
{
	if (CurrentTube >= 0) {
		return(false);
	}
	return(BASECLASS::Is_Moving_Onto_Bridge());
}

/***********************************************************************************************
 * FootClass::Is_LZ_Clear -- Determines if landing zone is free for landing.                   *
 *                                                                                             *
 *    This routine examines the landing zone (as specified by the target parameter) in order   *
 *    to determine if it is free to be landed upon. Call this routine when it is necessary     *
 *    to double check this. Typically this occurs right before a helicopter lands and also     *
 *    when determining the landing zone in the first place.                                    *
 *                                                                                             *
 * INPUT:   target   -- The target that is the "landing zone".                                 *
 *                                                                                             *
 * OUTPUT:  bool; Is the landing zone clear for landing?                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/31/94   JLB : Created.                                                                 *
 *=============================================================================================*/
bool FootClass::Is_LZ_Clear(AbstractClass * target) const
{
	if (target == NULL) return(false);
	Cell cell = target->Center_Coord().As_Cell();
	if (!Map.In_Local_Radar(cell)) return(false);

	/*
	**	If the requested landing location is occupied, then only consider that location
	**	legal if the occupying object is in radio contact with the aircraft. This presumes that
	**	the two objects know what they are doing.
	*/
	ObjectClass const * object = Map[cell].Cell_Object();
	if (object) {
		if (this == object) return(true);

		if (Contact_With_Whom() == object) {
			return(true);
		}
		return(false);
	}

	if (!Map[cell].Is_Clear_To_Move(SPEED_TRACK, false, false)) return(false);

	for (int i = 0; i < Aircraft.Count(); i++) {
		AircraftClass * aptr = Aircraft[i];
		if (aptr->IsActive && !aptr->IsInLimbo && aptr->NavCom == target && aptr != this) {
			return(false);
		}
	}

	return(true);
}


/// <summary>
/// Determines what action to perform if this object is ordered onto the cell.
/// This routine adds the shroud restriction to the base class decision. A shrouded cell can
/// only be moved toward, and only if this object is allowed to travel into the shroud at
/// all. Patrol waypoint orders survive the restriction unchanged.
/// </summary>
/// <returns>Returns with the action to perform.</returns>
ActionType FootClass::What_Action(Cell const & cell, bool check_fog, bool disallow_force) const
{
	ActionType action = BASECLASS::What_Action(cell, check_fog, disallow_force);

	Coord coord = cell;
	coord.Z = Map.Get_Height_GL(coord);
	if (Map[coord].IsUnderBridge) {
		coord.Z += BRIDGE_LEPTON_HEIGHT;
	}

	if (Map.Is_Shrouded(coord) && action != ACTION_NONE) {
		if (TClass->IsMoveToShroud && Map.In_Local_Radar(cell)) {
			if (action != ACTION_PATROL_WAYPOINT) {
				action = ACTION_MOVE;
			}
		} else {
			action = ACTION_NOMOVE;
		}
	}

	return(action);
}


/// <summary>
/// Determines what action to perform if this object is ordered onto the target.
/// This routine adds the shroud restriction to the base class decision. A target hidden
/// under the shroud can only be moved toward, and only if this object is allowed to travel
/// into the shroud at all.
/// </summary>
/// <returns>Returns with the action to perform.</returns>
ActionType FootClass::What_Action(ObjectClass const * target, bool disallow_force) const
{
	ActionType action = BASECLASS::What_Action(target, disallow_force);

	Coord coord = target->Center_Coord();
	coord.Z = Map.Get_Height_GL(coord);
	if (Map[coord].IsUnderBridge) {
		coord.Z += BRIDGE_LEPTON_HEIGHT;
	}

	if (Map.Is_Shrouded(coord) && action != ACTION_NONE && Session.Type == GAME_NORMAL) {
		if (TClass->IsMoveToShroud) {
			action = ACTION_MOVE;
		} else {
			action = ACTION_NOMOVE;
		}
	}

	return(action);
}


/// <summary>
/// Handles the rescue mission state machine.
/// This routine will have the object engage any threat close to the spot the mission began
/// at, then head for whatever destination its house nominates. Once it arrives, it settles
/// into a guard area mission.
/// </summary>
/// <returns>The delay in game frames before this mission should be processed again.</returns>
int FootClass::Do_MISSION_RESCUE(void)
{
	enum {
		ENGAGE_THREATS,		/// clear any threat near the spot the mission began at
		TRAVELLING			/// heading for the destination the house nominated
	};

	switch (Status) {

		case ENGAGE_THREATS:
			if (TarCom != NULL) {
				Approach_Target();
			} else {
				if (ArchiveTarget == NULL) {
					ArchiveTarget = &Map[(Coord const &)PositionCoord];
				}

				IsScanLimited = false;

				ObjectClass * threat = (ObjectClass *)Greatest_Threat(THREAT_NORMAL, ArchiveTarget->Center_Coord(), false);
				if (threat != NULL) {
					if (threat->Distance(ArchiveTarget->Center_Coord()) < Threat_Range(1) * 1.5) {
						Assign_Target(threat);
					}
				}

				if (TarCom != NULL) {
					return(1);
				} else {
					Status = TRAVELLING;
					Cell where = House->Where_To_Go(this);
					if (where != CELL_NONE) {
						Assign_Destination(&Map[where]);
					} else {
						Assign_Destination(NULL);
					}
					ArchiveTarget = NULL;
				}
			}
			break;

		case TRAVELLING:
			if (NavCom == NULL) {
				ArchiveTarget = NULL;
				Assign_Mission(MISSION_GUARD_AREA);
				Commence();
			}
			break;
	}

	return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
}


/// <summary>
/// Fetches the cell this object should actually be sent to.
/// This routine will find the closest reachable cell to the one ordered, so that clicking
/// on an impassable or shrouded location still sends the object as near as it can
/// reasonably get. An object that is not allowed to move into shroud refuses the order
/// outright.
/// </summary>
/// <param name="where">The cell that was ordered as the destination.</param>
/// <param name="consider_fog">Should the fog of war be considered when deciding the action?</param>
/// <returns>Returns with the cell to head toward, or CELL_NONE if the order is refused.</returns>
Cell FootClass::Move_Order(Cell const & where, bool consider_fog)
{
	/*
	**	Find the closest same-zoned cell to where the unit currently is.
	**	This will allow the unit to come as close to the destination cell
	**	as is reasonably possible, when clicking on an impassable cell
	**	(as is likely when clicking in the shroud.)  It looks for the
	**	nearest cell using an expanding-radius box, and ignores cells
	**	off the edge of the map.
	*/

	ActionType action = What_Action(where, consider_fog, false);
	bool inradar = Map.In_Local_Radar(where);

	Coord coord = where;
	coord.Z = Map.Get_Height_GL(coord);
	if (Map[coord].IsUnderBridge) {
		coord.Z += BRIDGE_LEPTON_HEIGHT;
	}

	if (TClass->IsMoveToShroud || !Map.Is_Shrouded(coord)) {

		Cell cell = where;
		CellClass * cptr = &Map[Destination_Coord()];

		MZoneType mzone = TClass->MZone;
		if (mzone == MZONE_SUBTERANNEAN) {
			mzone = MZONE_NORMAL;
		} else if (mzone == MZONE_FLYER && IonStormClass::Is_Ion_Storm_Active()) {
			mzone = MZONE_INFANTRY;
		}

		bool moveanywhere = (TClass->IsSubterranean || (RTTI == RTTI_INFANTRY && ((InfantryClass *)this)->Class->IsJumpJet && !IonStormClass::Is_Ion_Storm_Active()) || RTTI == RTTI_AIRCRAFT) ? true : false;
		bool ontobridge = Is_Moving_Onto_Bridge();

		Coord coord2 = where.As_Coord();
		coord2.Z = Map.Get_Height_GL(coord2);
		if (Map[coord2].IsUnderBridge) {
			coord2.Z += BRIDGE_LEPTON_HEIGHT;
		}

		bool shroud = Map.Is_Shrouded(coord2);

		if (moveanywhere && action == ACTION_NOMOVE) {
			cell = Map.Nearby_Location(where, TClass->Speed, -1, mzone, Map[where].IsUnderBridge, Point2D(1, 1), false, true, TClass->IsSubterranean && HeightAGL < 0);
		} else if (!inradar || shroud || (!moveanywhere && !Map.Is_Same_Cell_Zone(cptr->CellID, cell, mzone, ontobridge, Map[where].IsUnderBridge, false))) {
			cell = Map.Nearby_Location(where, TClass->Speed, Map.Get_Cell_Zone(cptr->CellID, mzone, ontobridge), mzone, Map[where].IsUnderBridge, Point2D(1, 1), false, true, false, true);
		}

		if (cell != CELL_NONE) {
			return(cell);
		}
	}

	return(CELL_NONE);
}


/// <summary>
/// Sends this object on to the next leg of its waypoint path.
/// This routine is used once the current waypoint has been satisfied. If the object is not
/// following a path at all, then it does nothing.
/// </summary>
void FootClass::Advance_Waypoint_Path(void)
{
	if (CurrentPath != PATH_NONE) {
		WaypointClass * wp = PlayerPtr->Paths[CurrentPath]->Get_Waypoint(NextWaypoint);
		WaypointClass * next_wp = PlayerPtr->Paths[CurrentPath]->Get_Next_Waypoint(wp);
		Execute_Waypoint_Path(next_wp);
	}
}


/// <summary>
/// Removes this object from the game.
/// This routine will detach the object from whatever team it belongs to before letting the
/// base class perform the actual deletion, so that no team is left holding a dead member.
/// </summary>
void FootClass::Delete_Me(void)
{
	if (Team != NULL) {
		Team->Remove(this);
	}
	BASECLASS::Delete_Me();
}


/// <summary>
/// Determines if this object is airborne.
/// A hover driven object is never considered to be in the air, even though it rides above
/// the terrain rather than on it.
/// </summary>
/// <returns>bool; Is the object in the air?</returns>
bool FootClass::In_Air(void) const
{
	IPersistPtr loco(Locomotion);

	CLSID clsid;
	loco->GetClassID(&clsid);

	if (clsid == CLSID_HoverLocomotion) {
		return(false);
	}

	return(BASECLASS::In_Air());
}


/// <summary>
/// Determines if this object is resting on the ground.
/// A hover driven object floats a short distance above the terrain, but as long as it is
/// placed down on the map it still counts as being on the ground.
/// </summary>
/// <returns>bool; Is the object on the ground?</returns>
bool FootClass::On_Ground(void) const
{
	if (BASECLASS::On_Ground()) {
		return(true);
	}
	IPersistPtr loco(Locomotion);

	CLSID clsid;
	loco->GetClassID(&clsid);

	return(IsDown && clsid == CLSID_HoverLocomotion);
}
