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

/* $Header: /CounterStrike/REINF.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : REINF.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 24, 1994                                                 *
 *                                                                                             *
 *                  Last Update : July 26, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Create_Air_Reinforcement -- Creates air strike reinforcement                              *
 *   Create_Special_Reinforcement -- Ad hoc reinforcement handler.                             *
 *   Do_Reinforcements -- Create and place a reinforcement team.                               *
 *   _Consists_Only_Of_Infantry -- Determine if this group consists only of infantry.          *
 *   _Create_Group -- Create a group given team specification.                                 *
 *   _Pop_Group_Out_Of_Object -- Process popping the group out of the object.                  *
 *   _Who_Can_Pop_Out_Of -- Find a suitable host for these reinforcements.                     *
 *   _Need_To_Take -- Examines unit to determine if it should be confiscated.                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "reinf.h"

#include "_map.h"
#include "aircraft.h"
#include "airctype.h"
#include "building.h"
#include "cell.h"
#include "foot.h"
#include "globals.h"
#include "house.h"
#include "classids.h"
#include "incdec.h"
#include "inline.h"
#include "mouse.h"
#include "revent.h"
#include "script.h"
#include "taskforc.h"
#include "techno.h"
#include "unit.h"
#include "unittype.h"
#include "vox.h"


/***********************************************************************************************
 * _Pop_Group_Out_Of_Object -- Process popping the group out of the object.                    *
 *                                                                                             *
 *    This routine will cause the group to pop out of the object specified.                    *
 *                                                                                             *
 * INPUT:   group    -- Pointer to the first object in the group to be popped out.             *
 *                                                                                             *
 *          object   -- Pointer to the object that the group is to pop out of.                 *
 *                                                                                             *
 * OUTPUT:  bool; Was the group popped out of the specified object?                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static bool _Pop_Group_Out_Of_Object(FootClass * group, TechnoClass * object)
{
	assert(group != NULL && object != NULL);
	int quantity = 0;

	/*
	**	Take every infantry member of this group and detach it from the group list
	**	and then make it pop out of the candidate source.
	*/
	while (group != NULL) {
		TechnoClass * todo = group;
		group = (FootClass *)(ObjectClass *)group->Next;
		todo->Next = NULL;

		switch ((RTTIType)object->RTTI) {

			/*
			**	The infantry just walks out of a building.
			*/
			case RTTI_BUILDING:
				if (object->Exit_Object(todo) != 2) {
					delete todo;
				} else {
					object->Transmit_Message(RADIO_OVER_OUT);
					++quantity;
				}
				break;

			/*
			**	Infantry get attached to transport vehicles and then unload.
			*/
			case RTTI_UNIT:
			case RTTI_AIRCRAFT:
				object->Cargo.Attach((FootClass *)todo);
				todo->Hidden();
				object->Assign_Mission(MISSION_UNLOAD);
				++quantity;
				break;

			default:
				delete todo;
				break;
		}
	}

	return(quantity != 0);
}


/***********************************************************************************************
 * _Need_To_Take -- Examines unit to determine if it should be confiscated.                    *
 *                                                                                             *
 *    The unit is examined and if the owning house needs to confiscate it, then this routine   *
 *    will return TRUE. In other cases, the unit should be left to its own devices.            *
 *                                                                                             *
 * INPUT:   unit  -- Pointer to the object to examine.                                         *
 *                                                                                             *
 * OUTPUT:  bool; Should the object be confiscated by the player so that it becomes one of     *
 *                his normal game objects?                                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool _Need_To_Take(AircraftClass const * air)
{
	return(false);
}


/***********************************************************************************************
 * _Create_Group -- Create a group given team specification.                                   *
 *                                                                                             *
 *    This routine will create all members of the group as specified by the team type.         *
 *                                                                                             *
 * INPUT:   teamtype -- Pointer to the team type that specifies what objects should be         *
 *                      created in this group.                                                 *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the first member of the group created.                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static FootClass * _Create_Group(TeamTypeClass const * teamtype)
{
	assert(teamtype != NULL);

	TeamClass * team = new TeamClass(teamtype, teamtype->House);
	if (team != NULL) {
		team->Force_Active();
	}

	bool hasunload = false;
	ScriptTypeClass * script = teamtype->Script;
	int count = script->MissionCount;
	for (int tm = 0; tm < count; tm++) {
		TeamMissionClass mission = script->MissionList[tm];
		if (mission.Mission == TMISSION_UNLOAD) {
			hasunload = true;
			break;
		}
	}

	bool has_air_transport = team->Has_Air_Transport();

	bool drop_pod = false;
	if (teamtype->IsDroppod) {
		if (teamtype->TaskForce->Has_Only_Infantry()) {
			drop_pod = true;
		}
	}

	/*
	**	Now that the official source for the reinforcement has been determined, the
	**	objects themselves must be created.
	*/
	FootClass * transport = NULL;
	FootClass * object = NULL;
	for (int index = 0; index < teamtype->TaskForce->ClassCount; index++) {
		TechnoTypeClass const * tclass = teamtype->TaskForce->Members[index].Class;

		for (int sub = 0; sub < teamtype->TaskForce->Members[index].Quantity; sub++) {
			ScenarioInit++;
			FootClass * temp = (FootClass *)tclass->Create_One_Of(teamtype->House);
			ScenarioInit--;

			if (temp != NULL) {

				if (tclass->Capacity > 0 && teamtype->IsFull) {
					temp->Storage.Increase_Amount(tclass->Capacity, 0);
				}

				switch (teamtype->VeteranLevel) {
					case 0:
						temp->Veterancy.Set_Dumbass(true);
						break;
					case 1:
						break;
					case 2:
						temp->Veterancy.Set_Veteran(true);
						break;
					case 3:
						temp->Veterancy.Set_Elite(true);
						break;
				}

				/*
				**	Add the member to the team.
				*/
				if (team != NULL) {
					ScenarioInit++;
					bool ok = team->Add(temp);
					ScenarioInit--;
					temp->IsInitiated = true;
				}

				bool transporter = false;
				if (tclass->Max_Passengers() > 0) {
					if (has_air_transport) {
						if (temp->RTTI == RTTI_AIRCRAFT) {
							transporter = true;
						} else {
							transporter = false;
						}
					} else {
						if (temp->RTTI == RTTI_UNIT) {
							transporter = true;
						} else {
							transporter = false;
						}
					}
				}

				/*
				**	Build the list of transporters and passengers.
				*/
				if (transporter) {
					/*
					**	Link to the list of transports.
					*/
					temp->Next = transport;
					transport = temp;
				} else {
					if (drop_pod) {
						temp->Link_DropPod();
					}

					/*
					**	Link to the list of normal objects.
					*/
					temp->Next = object;
					object = temp;
				}
			}
		}
	}

	/*
	**	If the group consists of transports and normal objects, then assign the normal
	**	objects to be passengers on the transport.
	*/
	if (transport != NULL && object != NULL) {
		transport->Cargo.Attach_Group(object);

		/*
		**	HACK ALERT! If the this team has an unload mission, then flag the transport
		**	as a loaner so that it will exit from the map when the unload process is
		**	complete, but only if the transport is an aircraft type.
		*/
		if (hasunload && transport->RTTI == RTTI_AIRCRAFT) {
			transport->IsALoaner = true;
		}
	}

	/*
	**	For JUST transport helicopters, consider the loaner a gift if there are
	**	no passengers.
	*/
	//if (transport != NULL && object == NULL && transport->What_Am_I() == RTTI_AIRCRAFT/* && *((AircraftClass *)transport) == AIRCRAFT_TRANSPORT*/) {
	//	transport->IsALoaner = false;
	//}

	if (transport == NULL && object == NULL) {
		if (team != NULL) delete team;
		return(NULL);
	}

	/*
	**	If this group consists only of non-transport object, then just return with a pointer
	**	to the first member of the group.
	*/
	if (transport == NULL) {
		return(object);
	}

	return(transport);
}


/***********************************************************************************************
 * _Consists_Only_Of_Infantry -- Determine if this group consists only of infantry.            *
 *                                                                                             *
 *    Use this to determine if the specified group only contains infantry. Such a reinforcement*
 *    group is a candidate for popping out of a building or transport vehicle rather than      *
 *    driving/walking/sailing/flying onto the map under its own power.                         *
 *                                                                                             *
 * INPUT:   first -- Pointer to the first object in the group to examine.                      *
 *                                                                                             *
 * OUTPUT:  bool; Is the entire group composed of infantry type units?                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static bool _Consists_Only_Of_Infantry(FootClass const * first)
{
	while (first != NULL) {
		if (first->RTTI != RTTI_INFANTRY) {
			return(false);
		}
		first = (FootClass const *)((ObjectClass *)first->Next);
	}
	return(true);
}


/***********************************************************************************************
 * _Who_Can_Pop_Out_Of -- Find a suitable host for these reinforcements.                       *
 *                                                                                             *
 *    This routine is used to scan nearby locations to determine if there is a suitable host   *
 *    for these reinforcements to "pop out of" (apologies to Aliens). Typical hosts include    *
 *    buildings and transport vehicles (of any kind).                                          *
 *                                                                                             *
 * INPUT:   origin   -- The cell that should be scanned from. Only this location and immediate *
 *                      adjacent locations will be scanned.                                    *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to a suitable host. If none could be found then NULL is     *
 *          returned.                                                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static TechnoClass * _Who_Can_Pop_Out_Of(Cell origin)
{
	CellClass * cellptr = &Map[origin];
	TechnoClass * candidate = NULL;

	for (int f = FACING_NONE; f < FACING_COUNT; f++) {
		CellClass * ptr = cellptr;
		if (f != FACING_NONE) {
			ptr = &ptr->Adjacent_Cell(FacingType(f));
		}

		BuildingClass * building = ptr->Cell_Building();
		if (building && building->Strength > 0) {
			candidate = building;
		}

		UnitClass * unit = ptr->Cell_Unit();
		if (unit && unit->Strength && unit->Class->Max_Passengers() > 0) {
			return(unit);
		}
	}
	return(candidate);
}


/***********************************************************************************************
 * Do_Reinforcements -- Create and place a reinforcement team.                                 *
 *                                                                                             *
 *    This routine is called when a reinforcement team must be created and placed on the map.  *
 *    It will create all members of the team and place them at the location determined from    *
 *    the team composition. The reinforcement team should follow team orders until overridden  *
 *    by AI or player intervention.                                                            *
 *                                                                                             *
 * INPUT:   teamtype -- Pointer to the team type to create as a reinforcement.                 *
 *                                                                                             *
 * OUTPUT:  Was the reinforcement successfully created and placed?                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *   05/18/1995 JLB : Returns success or failure condition.                                    *
 *   06/19/1995 JLB : Announces reinforcements.                                                *
 *   02/15/1996 JLB : Recognizes team reinforcement location.                                  *
 *=============================================================================================*/
bool Do_Reinforcements(TeamTypeClass const * teamtype, WAYPOINT wp)
{
	assert(teamtype != 0);

	bool delete_when_done = false;

	/*
	**	perform some preliminary checks for validity.
	*/
	if (!teamtype || !teamtype->TaskForce->ClassCount || teamtype->House == NULL) return(false);

	AircraftTypeClass const * dshp = AircraftTypes[AircraftTypeClass::From_Name("DSHP")];
	if (teamtype->TaskForce->ClassCount == 1 && teamtype->TaskForce->Members[0].Class == dshp && teamtype->House->CurrentDropship < 3) {

		if (!PlayerPtr->DropshipLoadouts[PlayerPtr->CurrentDropship].EntryCount) {
			return(NULL);
		}

		TaskForceClass * dropship_taskforce = new TaskForceClass;
		dropship_taskforce->Members[0] = EnlistedMemberClass(1, dshp);

		int classcount;
		for (classcount = 1; classcount <= PlayerPtr->DropshipLoadouts[PlayerPtr->CurrentDropship].EntryCount; classcount++) {
			TechnoTypeClass const * obj = teamtype->House->DropshipLoadouts[teamtype->House->CurrentDropship].Fetch(classcount - 1);
			dropship_taskforce->Members[classcount] = EnlistedMemberClass(1, obj);
		}
		dropship_taskforce->ClassCount = classcount;

		TeamTypeClass * dropship_teamtype = new TeamTypeClass;
		static char name[24];
		strcpy(name, dropship_teamtype->IniName);
		memcpy(dropship_teamtype, teamtype, sizeof(TeamTypeClass));
		dropship_teamtype->TaskForce = dropship_taskforce;
		dropship_teamtype->IniName = TStringID<24>(name);
		dropship_teamtype->GivenName = TStringID<48>(name);

		dropship_teamtype->IsDropship = true;
		teamtype->House->CurrentDropship++;
		teamtype = dropship_teamtype;
		delete_when_done = true;
	}

	/*
	**	HACK ALERT!
	**	Give this team an attack waypoint mission that will attack the waypoint location of this
	**	team if there are no team missions previously assigned.
	*/
	ScriptTypeClass * script = teamtype->Script;
	int count = script->MissionCount;
	if (script->MissionCount == 0) {
		TeamMissionClass tmission(TMISSION_ATT_WAYPT, 0);
		if (count < MAX_TEAM_MISSIONS) {
			script->MissionList[count] = tmission;
			script->MissionCount++;
		}
	}

	bool drop_pod = false;
	if (teamtype->IsDroppod) {
		if (teamtype->TaskForce->Has_Only_Infantry()) {
			drop_pod = true;
		}
	}

	FootClass * object = _Create_Group(teamtype);

	/*
	**	Bail on this reinforcement if no reinforcements could be created.
	**	This is probably because the object maximum was reached.
	*/
	if (!object) {
		return(false);
	}

	object->Team->IsDeleteTypeWhenDone = delete_when_done;

	Cell origin = teamtype->Get_Origin();
	if (wp != -1) {
		origin = Scen->Get_Waypoint_Cell(wp);
	}

	/*
	**	Special case code to handle infantry types that run from a building. This presumes
	**	that infantry are never a transport (which is safe to do).
	*/
	if (!drop_pod && origin != CELL_NONE && _Consists_Only_Of_Infantry(object)) {

		/*
		**	Search for an object that these infantry can pop out of.
		*/
		TechnoClass * candidate = _Who_Can_Pop_Out_Of(origin);

		if (candidate != NULL) {
			return(_Pop_Group_Out_Of_Object(object, candidate));
		}
	}

	/*
	**	Announce when the reinforcements have arrived.
	*/
	if (Create_Reinforcement(teamtype, object, origin, wp != -1) && teamtype->House->Is_Ally(PlayerPtr)) {
		LastRadarEventCell = origin;
		Speak(VOX_REINFORCEMENTS);
	}

	return(true);
}


/// <summary>
/// Can the whole reinforcement chain travel by tunnel?
/// This routine is used by the reinforcement handler to decide whether a group should
/// burrow into the map rather than arrive above ground. A single member that lacks the
/// tunnel locomotor disqualifies the entire chain.
/// </summary>
/// <param name="object">The first object of the reinforcement chain to examine.</param>
/// <returns>bool; Can every object in the chain burrow?</returns>
inline bool _Can_Burrow(FootClass * object)
{
	while (object != NULL) {
		TechnoTypeClass const * tclass = object->TClass;
		if (tclass->Locomotor != ClassID_TunnelLocomotion) {
			return(false);
		}
		object = (FootClass *)object->Next;
	}
	return(true);
}


/// <summary>
/// Brings a chain of reinforcement objects onto the map.
/// This routine is used by the reinforcement handlers to deliver a group, arriving either
/// by drop pod, by burrowing in underground, or by driving onto the map from the owning
/// house's edge. Any object that cannot be given a legal arrival cell is destroyed rather
/// than left in limbo.
/// </summary>
/// <param name="object">The first object of the reinforcement chain to deliver.</param>
/// <param name="xcell">The cell the reinforcement should arrive at or near.</param>
/// <param name="atwaypoint">Should the objects appear at the cell rather than travel in
/// from the map edge?</param>
/// <returns>bool; Was at least one object placed onto the map?</returns>
bool Create_Reinforcement(TeamTypeClass const * teamtype, FootClass * object, Cell const & xcell, bool atwaypoint)
{
	bool drop_pod = false;
	if (teamtype->IsDroppod) {
		if (teamtype->TaskForce->Has_Only_Infantry()) {
			drop_pod = true;
		}
	}

	bool burrow = _Can_Burrow(object);

	FacingType eface = FACING_N;	// Facing to enter map.

	Cell cell = xcell;

	if (!drop_pod && !burrow && !atwaypoint) {

		/*
		**	The reinforcements must be delivered the old fashioned way -- by moving onto the
		**	map using their own power. First order of business is to determine where they
		**	should arrive from.
		*/
		SourceType source = teamtype->House->Control.Edge;
		if (source == SOURCE_NONE) {
			source = SOURCE_NORTH;
		}

		eface = (FacingType)(source << 1);
		cell = Map.Calculated_Cell(source, cell, CELL_NONE, object->TClass->Speed);
	} else {
		cell = Map.Nearby_Location(cell, object->TClass->Speed);
	}

	Cell newcell = cell;

	FootClass * o = (FootClass *)object->Next;
	object->Next = NULL;
	bool okvoice = false;
	if (newcell != CELL_NONE) {
		while (newcell != CELL_NONE && object != NULL) {
			DirType desiredfacing(eface);
			if (object->RTTI == RTTI_AIRCRAFT) {
				desiredfacing = DirType(DirType(eface).As_Int() - DirType(FACING_135).As_Int() - 1).Snap_To_8();
			}

			ScenarioInit++;

			/*
			**	Pick the location where the reinforcements appear and then place
			**	them there.
			*/
			bool placed = false;

			if (drop_pod) {
				object->PositionCoord = newcell;
				object->Assign_Destination(&Map[newcell]);
				object->Locomotion->Move_To(newcell.As_Coord());
				object->Look();
				placed = true;
			} else if (burrow) {
				Coord coord = Map[newcell].Cell_Coord() - Coord(0, 0, 400);
				placed = object->Unlimbo(coord, desiredfacing.As_Dir256());
				if (placed) {
					object->PositionCoord += Coord(0, 0, -CELL_LEPTON - object->Height);
					object->Assign_Destination(&Map[newcell]);
					object->Set_Speed(1);
					object->Locomotion->Move_To(newcell.As_Coord());
				}
			} else {
				placed = object->Unlimbo(Map[newcell].Cell_Coord(), desiredfacing.As_Dir256());
			}

			if (placed) {
				okvoice = true;

				/*
				**	If this object is part of a team, then the mission for this
				**	object will be guard. The team handler will assign the proper
				**	mission that it should follow.
				*/
				if (object->RTTI != RTTI_AIRCRAFT) {
					object->Assign_Mission(MISSION_GUARD);
					object->Commence();
				}

				if (drop_pod) {

					/*
					**	Could not unlimbo at location specified so find an adjacent location that it can
					**	be unlimboed at. If this fails, then abort the whole placement process.
					*/
					FacingType adj;
					for (adj = FACING_N; adj < FACING_COUNT; adj++) {
						Cell trycell = (Cell)Adjacent_Cell(newcell, adj);
						if (Map.In_Radar(trycell)) {
							newcell = trycell;
							break;
						}
					}
					if (adj >= FACING_COUNT) {
						newcell = CELL_NONE;
					}
				}

			} else {

				/*
				**	Could not unlimbo at location specified so find an adjacent location that it can
				**	be unlimboed at. If this fails, then abort the whole placement process.
				*/
				FacingType adj;
				for (adj = FACING_N; adj < FACING_COUNT; adj++) {
					Cell trycell = (Cell)Adjacent_Cell(newcell, adj);
					if (!Map.In_Radar(trycell) && object->Can_Enter_Cell(&Map[trycell], adj) == MOVE_OK) {
						newcell = trycell;
						break;
					}
				}

				/*
				 * An adjacent location was found, so retry placing this same
				 * object there on the next pass through the loop.
				 */
				if (adj >= FACING_COUNT) {
					newcell = CELL_NONE;
				} else if (newcell != CELL_NONE) {
					ScenarioInit--;
					continue;
				}

				/*
				 * No usable location could be found, so abort placement of this
				 * object entirely.
				 */
				delete object;
			}

			ScenarioInit--;

			object = o;
			if (object != NULL) {
				o = (FootClass *)(ObjectClass *)object->Next;
				object->Next = NULL;
			}
		}

	/*
	 * This delete only fires when no valid arrival cell was ever found (the
	 * loop above never ran). When the loop runs and later aborts by setting
	 * newcell to CELL_NONE, the object it has advanced to is intentionally
	 * left for the trailing teardown -- do not hoist this into an
	 * unconditional post-loop delete.
	 */
	} else if (object != NULL) {
		delete object;
	}

	/*
	**	If there are still objects that could not be placed, then delete them.
	*/
	if (o != NULL) {
		while (o != NULL) {
			FootClass * old = o;
			o = (FootClass *)(ObjectClass *)o->Next;
			old->Next = 0;

			delete old;
		}
	}

	return(okvoice);
}


/***********************************************************************************************
 * Create_Special_Reinforcement -- Ad hoc reinforcement handler.                               *
 *                                                                                             *
 *    Use this routine to bring on a reinforcement that hasn't been anticipated by the trigger *
 *    system. An example of this would be replacement harvesters or airfield ordered units.    *
 *    The appropriate transport is created (if necessary) and a mission is assigned such that  *
 *    the object will legally bring itself onto the playing field.                             *
 *                                                                                             *
 * INPUT:   house    -- The owner of this reinforcement.                                       *
 *                                                                                             *
 *          type     -- The object to bring on.                                                *
 *                                                                                             *
 *          another  -- This is reserved for the transport class in those cases where the      *
 *                      transport MUST be forced to a specific type.                           *
 *                                                                                             *
 *          mission  -- The mission to assign this reinforcement team.                         *
 *                                                                                             *
 *          argument -- Optional team mission argument (usually a waypoint).                   *
 *                                                                                             *
 * OUTPUT:  Was the special reinforcement created without error?                               *
 *                                                                                             *
 * WARNINGS:   This routine will fail if a team type cannot be created.                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/04/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool Create_Special_Reinforcement(HouseClass * house, TechnoTypeClass const * type, TechnoTypeClass const * another, TeamMissionType mission, int argument)
{
	/// nothing in TS
#if 0
	assert(house != 0);
	assert(type != 0);

	if (house && type) {
		TeamTypeClass * team = new TeamTypeClass();

		if (team) {

			/*
			**	If there is no overridden mission assign to this special reinforcement, then
			**	we must assign something. If not, the reinforcement will just sit at the edge
			**	of the map.
			*/
			if (!another && mission == TMISSION_NONE) {
				mission = TMISSION_MOVECELL;
				argument = Map.Calculated_Cell(house->Control.Edge);
			}

			/*
			**	Fill in the team characteristics.
			*/
			team->IniName = "TEMP";
			team->IsReinforcable = false;
			team->IsTransient = true;
			team->ClassCount = 1;
			team->Members[0].Class = type;
			team->Members[0].Quantity = 1;
			team->MissionCount = 1;
			if (mission == TMISSION_NONE) {
				team->MissionList[0].Mission	= TMISSION_UNLOAD;
				team->MissionList[0].Data.Value = WAYPT_REINF;
			} else {
				team->MissionList[0].Mission	= mission;
				team->MissionList[0].Data.Value = argument;
			}
			team->House = house->Class->House;
			if (another) {
				team->ClassCount++;
				team->Members[1].Class = another;
				team->Members[1].Quantity = 1;
			}

			bool ok = Do_Reinforcements(team);
			if (!ok) delete team;
			return(ok);
		}
	}
#endif
	return(false);
}


/***********************************************************************************************
 * Create_Air_Reinforcement -- Creates air strike reinforcement                                *
 *                                                                                             *
 *    This routine is used to launch an airstrike. It will create the necessary aircraft and   *
 *    assign them to attack the target specified. This routine bypasses the normal             *
 *    reinforcement logic since it doesn't need the sophistication of unloading and following  *
 *    team mission lists.                                                                      *
 *                                                                                             *
 * INPUT:   house    -- The perpetrator of this air strike.                                    *
 *                                                                                             *
 *          air      -- The type of aircraft to make up this airstrike.                        *
 *                                                                                             *
 *          number   -- The number of aircraft in this airstrike.                              *
 *                                                                                             *
 *          mission  -- The mission to assign the aircraft.                                    *
 *                                                                                             *
 *          tarcom   -- The target to assign these aircraft.                                   *
 *                                                                                             *
 *          navcom   -- The navigation target to assign (if necessary).                        *
 *                                                                                             *
 * OUTPUT:  Returns the number of aircraft created for this airstrike.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/04/1995 JLB : Commented.                                                               *
 *=============================================================================================*/
int Create_Air_Reinforcement(HouseClass * house, AircraftType air, int number, MissionType mission, AbstractClass * tarcom, AbstractClass * navcom, InfantryType passenger)
{
	assert(house != 0);
	assert((unsigned)air < AircraftTypes.Count());
	assert(number != 0);
	assert((unsigned)mission < MISSION_COUNT);
	/*
	**	Get a pointer to the class of the object that we are going to create.
	*/
	TechnoTypeClass const * type = (TechnoTypeClass *)AircraftTypes[air];

	/*
	**	Loop through the number of objects we are supposed to create and
	**	create and place them on the map.
	*/
	int sub;
	for (sub = 0; sub < number; sub++) {

		/*
		**	Create one of the required objects.  If this fails we could have
		**	a real problem.
		*/
		ScenarioInit++;
		TechnoClass * obj = (TechnoClass *)type->Create_One_Of(house);
		ScenarioInit--;
		if (!obj) return(sub);

		/*
		**	Flying objects always have the IsALoaner bit set.
		*/
		obj->IsALoaner = true;

		/*
		**	Find a cell for the object to come in on.  This is stolen from the
		**	the code that handles a SOURCE_AIR in the normal logic.
		*/
		SourceType source = house->Control.Edge;
		switch (source) {
			case SOURCE_NORTH:
			case SOURCE_EAST:
			case SOURCE_SOUTH:
			case SOURCE_WEST:
				break;

			default:
				source = SOURCE_NORTH;
				break;
		}
		Cell newcell = Map.Calculated_Cell(source, CELL_NONE, CELL_NONE, SPEED_WINGED);

		/*
		**	Try and place the object onto the map.
		*/
		ScenarioInit++;
		int placed = obj->Unlimbo(newcell, DIR_N);
		ScenarioInit--;
		if (placed) {

			/*
			**	If we succeeded in placing the obj onto the map then
			**	now we need to give it a mission and destination.
			*/
			obj->Assign_Mission(mission);

			/*
			**	If a navcom was specified then set it.
			*/
			if (navcom != NULL) {
				obj->Assign_Destination(navcom);
			}

			/*
			**	If a tarcom was specified then set it.
			*/
			if (tarcom != NULL) {
				obj->Assign_Target(tarcom);
			}

			/*
			**	Assign generic passenger value here. This value is used to determine
			**	if this aircraft should drop parachute reinforcements.
			*/
			if (obj->RTTI == RTTI_AIRCRAFT) {
				AircraftClass * aircraft = (AircraftClass *)obj;
				if (passenger != INFANTRY_NONE) {
					aircraft->Passenger = true;
				}
			}

			/*
			**	Start the object into action.
			*/
			obj->Commence();
		} else {
			delete obj;
			sub--;
			return(sub);
		}
	}
	return(sub);
}
