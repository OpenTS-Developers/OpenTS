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

/* $Header: /CounterStrike/TEAM.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TEAM.CPP                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 12/11/94                                                     *
 *                                                                                             *
 *                  Last Update : August 27, 1996 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   TeamClass::AI -- Process team logic.                                                      *
 *   TeamClass::Add -- Adds specified object to team.                                          *
 *   TeamClass::Assign_Mission_Target -- Sets teams mission target and clears old target       *
 *   TeamClass::Calc_Center -- Determines average location of team members.                    *
 *   TeamClass::Can_Add -- Determines if the specified object can be added to team.            *
 *   TeamClass::Control -- Updates control on a member unit.                                   *
 *   TeamClass::Coordinate_Attack -- Handles coordinating a team attack.                       *
 *   TeamClass::Coordinate_Conscript -- Gives orders to new recruit.                           *
 *   TeamClass::Coordinate_Do -- Handles the team performing specified mission.                *
 *   TeamClass::Coordinate_Move -- Handles team movement coordination.                         *
 *   TeamClass::Coordinate_Regroup -- Handles team idling (regrouping).                        *
 *   TeamClass::Debug_Dump -- Displays debug information about the team.                       *
 *   TeamClass::Detach -- Removes specified target from team tracking.                         *
 *   TeamClass::Fetch_A_Leader -- Looks for a suitable leader member of the team.              *
 *   TeamClass::Has_Entered_Map -- Determines if the entire team has entered the map.          *
 *   TeamClass::Init -- Initializes the team objects for scenario preparation.                 *
 *   TeamClass::Is_A_Member -- Tests if a unit is a member of a team                           *
 *   TeamClass::Is_Leaving_Map -- Checks if team is in process of leaving the map              *
 *   TeamClass::Lagging_Units -- Finds and orders any lagging units to catch up.               *
 *   TeamClass::Recruit -- Attempts to recruit members to the team for the given index ID.     *
 *   TeamClass::Remove -- Removes the specified object from the team.                          *
 *   TeamClass::Scan_Limit -- Force all members of the team to have limited scan range.        *
 *   TeamClass::Suspend_Teams -- Suspends activity for low priority teams                      *
 *   TeamClass::TMision_Patrol -- Handles patrolling from one location to another.             *
 *   TeamClass::TMission_Attack -- Perform the team attack mission command.                    *
 *   TeamClass::TMission_Follow -- Perform the "follow friendlies" team command.               *
 *   TeamClass::TMission_Formation -- Process team formation change command.                   *
 *   TeamClass::TMission_Invulnerable -- Makes the entire team invulnerable for a period of tim*
 *   TeamClass::TMission_Load -- Tells the team to load onto the transport now.                *
 *   TeamClass::TMission_Loop -- Causes the team mission processor to jump to new location.    *
 *   TeamClass::TMission_Set_Global -- Performs a set global flag operation.                   *
 *   TeamClass::TMission_Spy -- Perform the team spy mission.                                  *
 *   TeamClass::TMission_Unload -- Tells the team to unload passengers now.                    *
 *   TeamClass::TeamClass -- Constructor for the team object type.                             *
 *   TeamClass::Took_Damage -- Informs the team when the team member takes damage.             *
 *   TeamClass::operator delete -- Deallocates a team object.                                  *
 *   TeamClass::operator new -- Allocates a team object.                                       *
 *   TeamClass::~TeamClass -- Team object destructor.                                          *
 *   _Is_It_Breathing -- Checks to see if unit is an active team member.                       *
 *   _Is_It_Playing -- Determines if unit is active and an initiated team member.              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "team.h"

#include "_map.h"
#include "_rtti.h"
#include "_rules.h"
#include "_tactica.h"
#include "aircraft.h"
#include "aitrig.h"
#include "anim.h"
#include "building.h"
#include "cell.h"
#include "crc.h"
#include "foot.h"
#include "globals.h"
#include "house.h"
#include "infantry.h"
#include "infatype.h"
#include "inline.h"
#include "ion.h"
#include "map.h"
#include "mission.h"
#include "mono.h"
#include "movie.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "script.h"
#include "session.h"
#include "sun.h"
#include "swizzle.h"
#include "tactical.h"
#include "tag.h"
#include "target.h"
#include "taskforc.h"
#include "teamtype.h"
#include "theme.h"
#include "tmission.h"
#include "tracker.h"
#include "tube.h"
#include "unit.h"
#include "unittype.h"
#include "voc.h"
#include "vox.h"

#include "target.hh"
#include "tube.hh"


BuildingClass *Pick_Building_With_Property(BuildingTypeClass *type, HouseClass *house, FootClass *unit, TargetPropertyType prop, bool only_enemy);


/***********************************************************************************************
 * _Is_It_Breathing -- Checks to see if unit is an active team member.                         *
 *                                                                                             *
 *    A unit could be a team member, but not be active. Such a case would occur when a         *
 *    reinforcement team is inside a transport. It could also occur if a unit is in the        *
 *    process of dying. Call this routine to ensure that the specified unit is a will and      *
 *    able participant in the team.                                                            *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the unit/infantry/aircraft that is to be checked.           *
 *                                                                                             *
 * OUTPUT:  bool; Is the specified unit active and able to be given commands by the team?      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/11/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static inline bool _Is_It_Breathing(FootClass const * object)
{
	/*
	**	If the object is not present or appears to be dead, then it
	**	certainly isn't an active member of the team.
	*/
	if (object == NULL || !object->IsActive || object->Strength == 0) return(false);

	/*
	**	If the object is in limbo, then it isn't an active team member either. However, if the
	**	scenario init flag is on, then it is probably a reinforcement issue or scenario
	**	creation situation. In such a case, the members are considered active because they need to
	**	be given special orders and treatment.
	*/
	if (!ScenarioInit && object->IsInLimbo) return(false);

	/*
	**	Nothing eliminated this object from being considered an active member of the team (i.e.,
	**	"breathing"), then return that it is ok.
	*/
	return(true);
}


/***********************************************************************************************
 * _Is_It_Playing -- Determines if unit is active and an initiated team member.                *
 *                                                                                             *
 *    Use this routine to determine if the specified unit is an active participant of the      *
 *    team. When a unit is first recruited to the team, it must travel to the team's location  *
 *    before it can become an active player. Call this routine to determine if the specified   *
 *    unit can be considered an active player.                                                 *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object that is to be checked to see if it is an         *
 *                      active player.                                                         *
 *                                                                                             *
 * OUTPUT:  bool; Is the specified unit an active, living, initiated member of the team?       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/11/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
static inline bool _Is_It_Playing(FootClass const * object)
{
	/*
	**	If the object is not active, then it certainly can be a participating member of the
	**	team.
	*/
	if (!_Is_It_Breathing(object)) return(false);

	/*
	**	Only members that have been "Initiated" are considered "playing" participants of the
	**	team. This results in the team members that are racing to regroup with the team (i.e.,
	**	not initiated), will continue to catch up to the team even while the initiated team members
	**	carry out their team specific orders.
	*/
	if (!object->IsInitiated && object->RTTI != RTTI_AIRCRAFT) return(false);

	/*
	**	If it reaches this point, then nothing appears to disqualify the specified object from
	**	being considered an active playing member of the team. In this case, return that
	**	information.
	*/
	return(true);
}


#ifdef _DEBUG
/***********************************************************************************************
 * TeamClass::Debug_Dump -- Displays debug information about the team.                         *
 *                                                                                             *
 *    This routine will display information about the team. This is useful for debugging       *
 *    purposes.                                                                                *
 *                                                                                             *
 * INPUT:   mono  -- Pointer to the monochrome screen that the debugging information will      *
 *                   be displayed on.                                                          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/11/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamClass::Debug_Dump(MonoClass * mono) const
{
	mono->Set_Cursor(1, 20);mono->Printf("%8.8s", Class->IniName.c_str());
	mono->Set_Cursor(10, 20);mono->Printf("%3d", Total);
	mono->Set_Cursor(17, 20);mono->Printf("%3d", Quantity[Class->ID]);
//	if (CurrentMission != -1) {
//		mono->Set_Cursor(1, 22);
//		mono->Printf("%-29s", Class->MissionList[CurrentMission].Description(CurrentMission));
//	}
//	mono->Set_Cursor(40, 20);mono->Printf("%-10s", FormationName[Formation]);
	mono->Set_Cursor(22, 20);mono->Printf("%08X", Zone);
	mono->Set_Cursor(31, 20);mono->Printf("%08X", Target);

	mono->Fill_Attrib(53, 20, 12, 1, IsUnderStrength ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(53, 21, 12, 1, IsFullStrength ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(53, 22, 12, 1, IsHasBeen ? MonoClass::INVERSE : MonoClass::NORMAL);

	mono->Fill_Attrib(66, 20, 12, 1, IsMoving ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(66, 21, 12, 1, IsForcedActive ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(66, 22, 12, 1, IsReforming ? MonoClass::INVERSE : MonoClass::NORMAL);
}
#endif


/***********************************************************************************************
 * TeamClass::TeamClass -- Constructor for the team object type.                               *
 *                                                                                             *
 *    This routine is called when the team object is created.                                  *
 *                                                                                             *
 * INPUT:   type  -- Pointer to the team type to make this team object from.                   *
 *                                                                                             *
 *          owner -- The owner of this team.                                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
TeamClass::TeamClass(TeamTypeClass const * type, HouseClass * owner, void * unknown) :
	BASECLASS(),
	Class((TeamTypeClass *)type),
	House(owner),
	IsForcedActive(false),
	IsHasBeen(false),
	IsFullStrength(false),
	IsUnderStrength(true),
	IsReforming(false),
	IsLagging(false),
	IsAltered(true),
	JustAltered(false),
	IsMoving(false),
	IsNextMission(true),
	IsLeaveMap(false),
	Suspended(false),
	Succeeded(false),
	Zone(NULL),
	ClosestMember(NULL),
	MissionTarget(NULL),
	Target(NULL),
	Total(0),
	Risk(0),
	SuspendTimer(0),
	TimeOut(0),
	Member(0),
	Script(NULL),
	HouseToScout(NULL),
	UnusedPtr1(unknown),
	CreationFrame(Frame),
	Tag(NULL),
	IsDeleteTypeWhenDone(false),
	IsRegrouping(false),
	NeedsReinforcement(false)
{
	//assert(Class);

	Teams.Add(this);
	AbstractTypePtrTracker.Add(this);
	TagPtrTracker.Add(this);
	ObjectPtrTracker.Add(this);
	NeuronPtrTracker.Add(this);

	memset(Quantity, 0, sizeof(Quantity));
	if (Class != NULL) {
		if (House != NULL && Class->IsBaseDefense) {
			House->BaseDefenseTeamCount++;

		}
		if (Class->Get_Origin() != CELL_NONE) {
			Zone = &Map[Class->Get_Origin()];
		}
		Class->Number++;

		/*
		**	If there is a trigger tightly associated with this team, then
		**	create an instance of that trigger and attach it to the team.
		*/
		if (Class->Tag) {
			Tag = new TagClass(Class->Tag);
		}

		Script = new ScriptClass(Class->Script);
	}
}


/***********************************************************************************************
 * TeamClass::~TeamClass -- Team object destructor.                                            *
 *                                                                                             *
 *    This routine is called when a team object is destroyed. It handles updating the total    *
 *    number count for this team object type.                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *   07/04/1996 JLB : Keeps trigger if trigger still attached to objects.                      *
 *=============================================================================================*/
TeamClass::~TeamClass(void)
{
	for (int i = 0; i < AITriggerTypes.Count(); i++) {
		AITriggerTypeClass *ptr = AITriggerTypes[i];
		if (ptr->Get_First_TeamType() == Class) {
			if (Succeeded) {
				ptr->Record_Success();
			} else {
				ptr->Record_Failure();
			}
		}
	}

	if (House != NULL && Class != NULL && Class->IsBaseDefense) {
		House->BaseDefenseTeamCount--;
	}

	if (GameActive && Class != NULL) {
		while (Member != NULL) {
			Remove(Member);
		}
		Class->Number--;

		/*
		**	When the team dies, any trigger associated with it, dies as well. This will only occur
		**	if there are no other objects linked to this trigger. Only player reinforcement
		**	members that broke off of the team earlier will have this occur.
		*/
		if (Tag != NULL) {
			if (Tag->AttachCount == 0) {
				Tag->Mark_To_Delete();
			}
			Tag = NULL;
		}
	}
	Detach_This_From_All(this);

	NeuronPtrTracker.Delete(this);
	ObjectPtrTracker.Delete(this);
	AbstractTypePtrTracker.Delete(this);
	TagPtrTracker.Delete(this);
	Teams.Delete(this);

	if (IsDeleteTypeWhenDone) {
		delete Class;
	}
}


/***************************************************************************
 * TeamClass::Assign_Mission_Target -- Sets mission target and clears old  *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/16/1995 PWG : Created.                                             *
 *=========================================================================*/
void TeamClass::Assign_Mission_Target(AbstractClass * new_target)
{
	if (new_target != MissionTarget) {
		/*
		**	First go through and find anyone who is currently targeting
		**	the old mission target and clear their Tarcom.
		*/
		FootClass * unit = Member;
		if (MissionTarget != NULL) {
			while (unit != NULL) {
				bool tar = (unit->TarCom == MissionTarget);
				bool nav = (unit->NavCom == MissionTarget);
				if (tar || nav) {

					/*
					**	If the unit was doing something related to the team mission
					**	then we kick him into guard mode so that he is easy to change
					**	missions for.
					*/
					unit->Assign_Mission(MISSION_GUARD);

					/*
					**	If the unit's tarcom is set to the old mission target, then
					**	clear it, so that it will be reset by whatever happens next.
					*/
					if (nav) {
						unit->Assign_Destination(NULL);
					}

					/*
					**	If the unit's navcom is set to the old mission target, then
					**	clear it, so that it will be reset by whatever happens next.
					*/
					if (tar) {
						unit->Assign_Target(NULL);
					}
				}
				unit = unit->Member;
			}
		}
	}

	/*
	**	If there is not currently an override on the current mission target
	**	then assign both MissionTarget and Target to the new target.  If
	**	there is an override, allow the team to keep fighting the override but
	**	make sure they pick up on the new mission when they are ready.
	*/
	if (Target == MissionTarget || Target == NULL) {
		MissionTarget = Target = new_target;
	} else {
		MissionTarget = new_target;
	}

	if (Is_Target_Cell(MissionTarget)) {
		if (!Map.In_Local_Radar((CellClass *)new_target)) {
			FootClass * unit = Member;
			IsLeaveMap = true;
			while (unit != NULL) {
				unit->Assign_Destination(NULL);
				unit = unit->Member;
			}
		} else {
			IsLeaveMap = false;
		}
	}
}


/***********************************************************************************************
 * TeamClass::AI -- Process team logic.                                                        *
 *                                                                                             *
 *    General purpose team logic is handled by this routine. It should be called once per      *
 *    active team per game tick. This routine handles recruitment and assigning orders to      *
 *    member units.                                                                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/29/1994 JLB : Created.                                                                 *
 *   01/06/1995 JLB : Choreographed gesture.                                                   *
 *=============================================================================================*/
void TeamClass::AI(void)
{
	/*
	**	If the team has been suspended then we need to check if it's time for
	**	us to reactivate the team.  If not, no team logic will be processed
	**	for this team.
	*/
	if (Suspended) {
		if (SuspendTimer != 0) {
			return;
		}
		Suspended = false;
	}

	/*
	**	If this team senses that its composition has been altered, then it should
	**	recalculate the under strength and full strength flags.
	*/
	if (IsAltered) {
		if (!Recalc_Strength()) {
			return;
		}
	}

	/*
	**	If the team is under strength, then flag it to regroup.
	*/
	if (IsMoving && IsUnderStrength) {
		Regroup();
	}

	/*
	**	Flag this team into action when it gets to full strength. Human owned teams are only
	**	used for reinforcement purposes -- always consider them at full strength.
	*/
	if (!IsMoving && (IsFullStrength || IsForcedActive)) {
		Flag_Into_Action();
	}

	/*
	**	If the team is moving or if there is no center position for
	**	the team, then the center position must be recalculated.
	*/
	if (IsReforming || IsMoving || Zone == NULL || ClosestMember == NULL) {
		Calc_Center(Zone, ClosestMember);
	}

	/*
	**	Try to recruit members if there is room to do so for this team.
	**	Only try to recruit members for a non player controlled team.
	*/
	if ((!IsMoving || (!IsFullStrength && Class->IsReinforcable)) && (!House->Is_Human_Player() || !IsHasBeen)) {
		for (int index = 0; index < Class->TaskForce->ClassCount; index++) {
			if (Quantity[index] < Class->TaskForce->Members[index].Quantity) {
				Recruit(index);
			}
		}
	}

	/*
	**	If there are no members of the team and the team has reached
	**	full strength at one time, then delete the team.
	*/
	if (Member == NULL && (IsHasBeen || Session.Type != GAME_NORMAL && Frame - CreationFrame > Rule->DissolveUnfilledTeamDelay)) {

		/*
		**	If this team had no members (i.e., the team object wasn't terminated by some
		**	outside means), then pass through the logic triggers to see if one that
		**	depends on this team leaving the map should be sprung.
		*/
		if (IsLeaveMap) {
			for (int index = 0; index < LogicTags.Count(); index++) {
				TagClass * trig = LogicTags[index];
				if (trig->Spring(TEVENT_LEAVES_MAP)) {
					index--;
					if (LogicTags.Count() == 0) break;
				}
			}
		}
		delete this;
		return;
	}

	/*
	**	If the mission should be advanced to the next entry, then do so at
	**	this time. Various events may cause the mission to advance, but it
	**	all boils down to the following change-mission code.
	*/
	if (IsMoving && !IsReforming && !IsUnderStrength) {
		bool is_next = false;
		if (IsNextMission) {
			is_next = true;
			IsNextMission = false;
			Script->Next_Mission();
			FootClass * techno = Member;
			while (techno) {
				techno->ArchiveTarget = NULL;
				techno = techno->Member;
			}

			if (Script->Has_Missions_Remaining()) {
				Assign_Mission_Target(NULL);
				Target = NULL;
			} else {
				delete this;
				return;
			}
		}

		/*
		**	Perform mission of the team. This depends on the mission list.
		*/
		if (!is_next) {
			/*
			**	If the current Target has been dealt with but the mission target
			**	has not, then the current target needs to be reset to the mission
			**	target.
			*/
			if (Target == NULL) {
				Target = MissionTarget;
			}
		}

		/*
		**	If the current mission is one that times out, then check for
		**	this case. If it has timed out then advance to the next
		**	mission in the list or disband the team.
		*/
		TeamMissionClass mission = Script->Get_Current_Mission();

		/* 
		 * AircraftClass debug prints expose that mission functions were named Do_MISSION_X.
		 * This suggests that switches on enums like this used a macro taking the enum as input.
		 * 
		 * Simplifies invoking the right function and checking we aren't missing any case.
		 */
		#define INVOKE(e) case TMISSION_ ## e: TMission_ ## e (&mission, is_next); break;

		switch (mission.Mission) {
			INVOKE(UNLOAD_TRUCK);
			INVOKE(LOAD_TRUCK);
			INVOKE(CLEAR_GLOBAL);
			INVOKE(DESTROY_MEMBERS);
			INVOKE(GOTO_SHROUD);
			INVOKE(MOVECELL);
			INVOKE(MOVE);
			INVOKE(ATT_WAYPT);
			INVOKE(PATROL);
			INVOKE(SPY);
			INVOKE(SCATTER);
			INVOKE(CHANGE_HOUSE);
			INVOKE(TEAMCHANGE);
			INVOKE(SCRIPT);
			INVOKE(ATTACK);
			INVOKE(LOAD);
			INVOKE(DEPLOY);
			INVOKE(GUARD);
			INVOKE(DO);
			INVOKE(SET_GLOBAL);
			INVOKE(HOUND_DOG);
			INVOKE(PANIC);
			INVOKE(BERZERK);
			INVOKE(IDLE_ANIM);
			INVOKE(LOOP);
			INVOKE(WIN);
			INVOKE(LOSE);
			INVOKE(PLAY_SPEECH);
			INVOKE(PLAY_SOUND);
			INVOKE(PLAY_MOVIE);
			INVOKE(PLAY_MUSIC);
			INVOKE(REDUCE_TIBERIUM);
			INVOKE(BEGIN_PRODUCTION);
			INVOKE(FIRE_SALE);
			INVOKE(SELF_DESTRUCT);
			INVOKE(ION_STORM_START);
			INVOKE(ION_STORM_END);
			INVOKE(CENTER_VIEWPOINT);
			INVOKE(RESHROUD);
			INVOKE(REVEAL);
			INVOKE(SET_LOCAL);
			INVOKE(CLEAR_LOCAL);
			INVOKE(UNPANIC);
			INVOKE(FORCE_FACING);
			INVOKE(FULLY_LOADED);
			INVOKE(ATTACK_BUILDING_WITH_PROPERTY);
			INVOKE(MOVETO_BUILDING_WITH_PROPERTY);
			INVOKE(SCOUT);
			INVOKE(UNLOAD);
			INVOKE(SUCCESS);
			INVOKE(FLASH);
			INVOKE(PLAY_ANIM);
			INVOKE(TALK_BUBBLE);
		}

	} else {

		if (IsMoving) {
			IsReforming = !Coordinate_Regroup();
		} else {
			Coordinate_Move();
		}
	}
}


/// <summary>
/// Kicks the team into action.
/// This routine is called once the team is fit to begin its mission list. The members are
/// marked as initiated and the current script is halted, so that the team will move on to
/// its next mission.
/// </summary>
void TeamClass::Flag_Into_Action(void)
{
	IsMoving = true;
	IsHasBeen = true;
	IsUnderStrength = false;

	FootClass * member = Member;
	while (member != NULL) {
		if (IsReforming || IsForcedActive) {
			member->IsInitiated = true;
		}
		member = member->Member;
	}

	Script->Stop_Script();
	IsNextMission = true;
//	IsForcedActive = false;
}


/// <summary>
/// Sends the team off to regroup at a friendly structure.
/// This routine is used when the team has been knocked about badly enough that it should
/// pull back for a while. A repair facility is the first preference, but any harmless
/// friendly building will serve.
/// </summary>
void TeamClass::Regroup(void)
{
	IsMoving = false;
	Script->Stop_Script();
	if (Total > 0) {
		Calc_Center(Zone, ClosestMember);

		/*
		**	When a team is badly damaged and needs to regroup it should
		**	pick a friendly building to go and regroup at.  Its first preference
		**	should be somewhere near repair factory.  If it cannot find a repair
		**	factory then it should pick another structure that is friendly to
		**	its side.
		*/
		Cell dest = Zone->Center_Coord().As_Cell();
		int max	= 0x7FFFFFFF;

		for (int index = 0; index < Buildings.Count(); index++) {
			BuildingClass * b = Buildings[index];

			if (b != NULL && !b->IsInLimbo && b->House == House && b->PrimaryWeapon == NULL) {
				Cell cell = b->Center_Coord().As_Cell();
				int dist = b->Distance(Zone->Center_Coord()) * (Map.Cell_Threat(cell, *House) + 1);

				if (b->Class->IsCanUnitRepair) {
					dist /= 2;
				}
				if (dist < max) {
					cell = Fetch_A_Leader()->Safety_Point(Zone->Center_Coord().As_Cell(), cell, 2, 4);
					if (cell != CELL_NONE) {
						max = dist;
						dest = cell;
					}
				}
			}
		}

		// Should calculate a regroup location.
		Target = &Map[dest];
		Coordinate_Move();
	} else {
		Zone = NULL;
	}
}


/// <summary>
/// Recalculates the strength condition of the team.
/// This routine is used after the roster changes in order to work out whether the team is at
/// full strength, under strength, or in want of reinforcement. A team that has already seen
/// action and has since lost every member is disbanded here.
/// </summary>
/// <returns>bool; Does the team still exist?</returns>
/// <remarks>The team is deleted when this routine returns false -- do not touch it
/// afterward.</remarks>
bool TeamClass::Recalc_Strength(void)
{
	bool old_under = IsUnderStrength;
	int desired = Class->TaskForce->Required_Object_Count();

	if (Total > 0) {
		IsFullStrength = (Total == desired);
		if (IsFullStrength) {
			IsHasBeen = true;
		}

		/*
		**	Reinforceable teams will revert (or snap out of) the under strength
		**	mode when the members transition the magic 1/3 strength threshold.
		*/
		if (Class->IsReinforcable) {
			if (desired > 2) {
				IsUnderStrength = (Total <= desired / 3);
			} else {
				IsUnderStrength = (Total < desired);
			}
		} else {

			/*
			**	Teams that are not flagged as reinforceable are never considered under
			**	strength if the team has already started its main mission. This
			**	ensures that once the team has started, it won't dally to pick up
			**	new members.
			*/
			IsUnderStrength = !IsHasBeen;
		}

		if (Class->IsGuardSlower) {
			if (IsUnderStrength) {
				NeedsReinforcement = false;
			} else {
				NeedsReinforcement = true;
			}
		}

		IsAltered = JustAltered = false;
	} else {
		NeedsReinforcement = false;
		IsUnderStrength = true;
		IsFullStrength = false;
		Zone = NULL;

		/*
		**	A team that exists on the player's side is automatically destroyed
		**	when there are no team members left. This team was created as a
		**	result of reinforcement logic and no longer needs to exist when there
		**	are no more team members.
		*/
		if (IsHasBeen) {

			/*
			**	If this team had no members (i.e., the team object wasn't terminated by some
			**	outside means), then pass through the logic triggers to see if one that
			**	depends on this team leaving the map should be sprung.
			*/
			if (IsLeaveMap) {
				for (int index = LogicTags.Count() - 1; index >= 0; index--) {
					TagClass * tag = LogicTags[index];
					if (tag->Spring(TEVENT_LEAVES_MAP)) {
						if (LogicTags.Count() == 0) break;
					}
				}
			}
			delete this;
			return(false);
		}
	}

	/*
	**	If the team has gone from under strength to no longer under
	**	strength than the team needs to reform.
	*/
	if (old_under != IsUnderStrength) {
		IsReforming = true;
	}
	return(true);
}


/***********************************************************************************************
 * TeamClass::Add -- Adds specified object to team.                                            *
 *                                                                                             *
 *    Use this routine to add the specified object to the team. The object is checked to make  *
 *    sure that it can be assigned to the team. If it can't, then the object will be left      *
 *    alone and false will be returned.                                                        *
 *                                                                                             *
 * INPUT:   obj      -- Pointer to the object that is to be assigned to this team.             *
 *                                                                                             *
 * OUTPUT:  bool; Was the unit added to the team?                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/29/1994 JLB : Created.                                                                 *
 *   01/02/1995 JLB : Initiation flag setup.                                                   *
 *   08/06/1995 JLB : Allows member stealing from lesser priority teams.                       *
 *=============================================================================================*/
bool TeamClass::Add(FootClass * obj)
{
	if (!obj) return(false);

	int typeindex;
	if (!Can_Add(obj, typeindex)) return(false);

	/*
	**	All is ok to add the object to the team, but if the object is already part of
	**	another team, then it must be removed from that team first.
	*/
	if (obj->Team != NULL) {
		obj->Team->Remove(obj);
	}

	/*
	**	Actually add the object to the team.
	*/
	Quantity[typeindex]++;
	obj->IsInitiated = (Member == NULL);
	obj->Member = Member;
	Member = obj;
	obj->Team = this;
	obj->Group = Class->Get_Group();

	/*
	**	If a common trigger is designated for this team type, then attach the
	**	trigger to this team member.
	*/
	if (Tag != NULL && (!Class->OnTransOnly || obj->TClass->MaxPassengers > 0)) {
		obj->Attach_Tag(Tag);
	}

	Total++;
	Risk += obj->Risk();
	if (Zone == NULL) {
		Calc_Center(Zone, ClosestMember);
	}

	/*
	**	Return with success, since the object was added to the team.
	*/
	IsAltered = JustAltered = true;
	obj->IsAutocreateRecruitable = Class->AreTeamMembersRecruitable;
	return(true);
}


/***********************************************************************************************
 * TeamClass::Can_Add -- Determines if the specified object can be added to team.              *
 *                                                                                             *
 *    This routine will examine the team and determine if the specified object can be          *
 *    properly added to this team. This is a security check to filter out those objects that   *
 *    should not be added because of conflicting priorities or other restrictions.             *
 *                                                                                             *
 * INPUT:   obj      -- Pointer to the candidate object that is being checked for legal        *
 *                      adding to this team.                                                   *
 *                                                                                             *
 *          typeindex-- The class index number (according to the team type's class array) that *
 *                      the candidate object is classified as. The routine processes much      *
 *                      faster if you can provide this information, but if you don't, the      *
 *                      routine will figure it out.                                            *
 *                                                                                             *
 * OUTPUT:  bool; Can the specified object be added to this team?                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/27/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TeamClass::Can_Add(FootClass * obj, int & typeindex) const
{
	/*
	**	Trying to add the team member to itself is an error condition.
	*/
	if (this == obj->Team) {
		return(false);
	}

	/*
	**	The object must be active, a member of this house. A special dispensation is given to
	**	units that are in radio contact. It is presumed that they are very busy and should
	**	not be disturbed.
	*/
	if (!_Is_It_Breathing(obj) || obj->In_Radio_Contact() || obj->House != House) {
		return(false);
	}

	/*
	**	If the object is doing some mission that precludes it from joining
	**	a team then don't add it.
	*/
	if (obj->Mission != MISSION_NONE && !MissionClass::Is_Recruitable_Mission(obj->Mission)) {
		return(false);
	}

	if (!obj->IsTeamRecruitable && !Class->IsAutocreate) {
		return(false);
	}

	if (!obj->IsAutocreateRecruitable && Class->IsAutocreate) {
		return(false);
	}

	/*
	**	If this object is part of another team, then check to make sure that it
	**	is permitted to leave the other team in order to join this one. If not,
	**	then no further processing is allowed -- bail.
	*/
	if (obj->Team != NULL && (obj->Team->Class->RecruitPriority >= Class->RecruitPriority)) {
		return(false);
	}

	/*
	**	Aircraft that have no ammo for their weapons cannot be recruited into a team.
	*/
	if (obj->RTTI == RTTI_AIRCRAFT && obj->PrimaryWeapon != NULL && !obj->Ammo) {
		return(false);
	}

	/*
	**	Search for the exact member index that the candidate object matches.
	**	If no match could be found, then adding the object to the team cannot
	**	occur.
	*/
	for (typeindex = 0; typeindex < Class->TaskForce->ClassCount; typeindex++) {
		if (Class->TaskForce->Members[typeindex].Class == obj->Class_Of()) {
			break;
		}
	}

	TaskForceClass *tptr = Class->TaskForce;
	if (typeindex == tptr->ClassCount) {
		return(false);
	}

	/*
	**	If the team is already full of this type, then adding the object is not allowed.
	**	Return with a failure flag in this case.
	*/
	if (Quantity[typeindex] >= tptr->Members[typeindex].Quantity) {
		return(false);
	}
	return(true);
}


/***********************************************************************************************
 * TeamClass::Remove -- Removes the specified object from the team.                            *
 *                                                                                             *
 *    Use this routine to remove an object from a team. Objects removed from the team are      *
 *    then available to be recruited by other teams, or even by the same team at a later time. *
 *                                                                                             *
 * INPUT:   obj      -- Pointer to the object that is to be removed from this team.            *
 *                                                                                             *
 *          typeindex-- Optional index of where this object type is specified in the type      *
 *                      type class. This parameter can be omitted. It only serves to make      *
 *                      the removal process faster.                                            *
 *                                                                                             *
 * OUTPUT:  bool; Was the object removed from this team?                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/29/1994 JLB : Created.                                                                 *
 *   01/02/1995 JLB : Initiation tracking and team captain selection.                          *
 *=============================================================================================*/
bool TeamClass::Remove(FootClass * obj, int typeindex)
{
	obj->IsDroppedFromTeam = true;

	/*
	**	Make sure that the object is in fact a member of this team. If not, then it can't
	**	be removed. Return success because the end result is the same.
	*/
	if (this != obj->Team) {
		return(true);
	}

	/*
	**	Detach the common trigger for this team type. Only current and active members of the
	**	team have that trigger attached. The exception is for player team members that
	**	get removed from a reinforcement team.
	*/
	if (obj->Tag == Tag) {
		HouseClass *hptr = obj->House;
		if (hptr != NULL && !hptr->Is_Human_Player()) {
			obj->Attach_Tag(NULL);
		}
	}

	/*
	**	If the proper team index was not provided, then find it in the type type class. The
	**	team type class will not be set if the appropriate type could not be found
	**	for this object. This indicates that the object was illegally added. Continue to
	**	process however, since removing this object from the team is a good idea.
	*/
	if (typeindex == -1 && Class != NULL && Class->TaskForce != NULL) {
		for (typeindex = 0; typeindex < Class->TaskForce->ClassCount; typeindex++) {
			if (Class->TaskForce->Members[typeindex].Class == obj->Class_Of()) {
				break;
			}
		}
	}

	/*
	**	Decrement the counter for the team class. There is now one less of this object type.
	*/
	if (Class != NULL && Class->TaskForce != NULL && (unsigned)typeindex < (unsigned)Class->TaskForce->ClassCount) {
		Quantity[typeindex]--;
	}

	/*
	**	Actually remove the object from the team. Scan through the team members
	**	looking for the one that matches the one specified. If it is found, it
	**	is unlinked from the member chain. During this scan, a check is made to
	**	ensure that at least one remaining member is still initiated. If not, then
	**	a new team captain must be chosen.
	*/
	bool initiated = false;
	FootClass * prev = 0;
	FootClass * curr = Member;
	bool found = false;
	while (curr != NULL && (!found || !initiated)) {
		if (curr == obj) {
			if (prev != NULL) {
				prev->Member = curr->Member;
			} else {
				Member = curr->Member;
			}
			FootClass * temp = curr->Member;
			curr->Member = 0;
			curr->Team = 0;
			curr->SuspendedMission = MISSION_NONE;
			curr->SuspendedNavCom = NULL;
			curr->SuspendedTarCom = NULL;
			curr = temp;
			Total--;
			found = true;
			Risk -= obj->Risk();
			continue;
		}

		/*
		**	If this (remaining) member is initiated, then keep a record of this.
		*/
		//initiated |= curr->IsInitiated;
		if (initiated) {
			initiated = true;
		} else {
			initiated = false;
			if (curr->IsInitiated) {
				initiated = true;
			}
		}

		prev = curr;
		curr = curr->Member;
	}

	if (obj->Team != NULL) {
		obj->Team = NULL;
	}

	/*
	**	A unit that breaks off of a team will enter idle mode.
	*/
	if (GameActive && obj->IsActive && !obj->IsInLimbo) {
		obj->Enter_Idle_Mode();
	}

	/*
	**	If, after removing the team member, there are no initiated members left
	**	in the team, then just make the first remaining member of the team the
	**	team captain. Mark the center location of the team as invalid so that
	**	it will be centered around the captain.
	*/
	if (!initiated && Member != NULL) {
		Member->IsInitiated = true;
		Zone = NULL;
	}

	/*
	**	Must record that the team composition has changed. At the next opportunity,
	**	the team members will be counted and appropriate AI adjustments made.
	*/
	IsAltered = JustAltered = true;
	return(true);
}


/***********************************************************************************************
 * TeamClass::Recruit -- Attempts to recruit members to the team for the given index ID.       *
 *                                                                                             *
 *    This routine will take the given index ID and scan for available objects of that type    *
 *    to recruit to the team. Recruiting will continue until that object type has either       *
 *    been exhausted or if the team's requirement for that type has been filled.               *
 *                                                                                             *
 * INPUT:   typeindex   -- The index for the object type to recruit. The index is used to      *
 *                         look into the type type's array of object types that make up this   *
 *                         team.                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of objects added to this team.                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/29/1994 JLB : Created.                                                                 *
 *   04/10/1995 JLB : Scans for units too.                                                     *
 *=============================================================================================*/
int TeamClass::Recruit(int typeindex)
{
	Coord center = Zone != NULL ? Zone->Center_Coord() : COORD_NONE;

	if (Class->Get_Origin() != CELL_NONE) {
		center = Class->Get_Origin().As_Coord();
	}

	int added = 0;				// Total number added to team.

	/*
	**	Quick check to see if recruiting is really allowed for this index or not.
	*/
	if (Class->TaskForce->Members[typeindex].Quantity > Quantity[typeindex]) {
		int group = Class->Get_Group();
		switch ((RTTIType)Class->TaskForce->Members[typeindex].Class->RTTI) {

			/*
			**	For infantry objects, sweep through the infantry in the game looking for
			**	ones owned by the house that owns the team. When found, try to add.
			*/
			case RTTI_INFANTRYTYPE:
			case RTTI_INFANTRY:
				{
					InfantryClass * best = 0;
					int bestdist = -1;

					for (int index = 0; index < Infantry.Count(); index++) {
						InfantryClass * infantry = Infantry[index];
						if (group == -2 || infantry->Group == group || Class->IsRecruiter) {
							int d = infantry->Relative_Distance(center);

							if (infantry->Group != group) {
								d += 50 * CELL_LEPTON;
							}

							if ((d < bestdist || bestdist == -1) && Can_Add(infantry, typeindex)) {
								best = infantry;
								bestdist = d;
							}
						}
					}

					if (best) {
						best->Assign_Target(NULL);
						Add(best);
						added++;
					}
				}
				break;

			case RTTI_AIRCRAFTTYPE:
			case RTTI_AIRCRAFT:
				{
					AircraftClass * best = 0;
					int bestdist = -1;

					for (int index = 0; index < Aircraft.Count(); index++) {
						AircraftClass * aircraft = Aircraft[index];
						if (group == -2 || aircraft->Group == group || Class->IsRecruiter) {
							int d = aircraft->Relative_Distance(center);

							if (aircraft->Group != group) {
								d += 50 * CELL_LEPTON;
							}

							if ((d < bestdist || bestdist == -1) && Can_Add(aircraft, typeindex)) {
								best = aircraft;
								bestdist = d;
							}
						}
					}

					if (best) {
						best->Assign_Target(NULL);
						Add(best);
						added++;
					}
				}
				break;

			case RTTI_UNITTYPE:
			case RTTI_UNIT:
				{
					UnitClass * best = 0;
					int bestdist = -1;

					for (int index = 0; index < Units.Count(); index++) {
						UnitClass * unit = Units[index];
						if (group == -2 || unit->Group == group || Class->IsRecruiter) {
							int d = unit->Relative_Distance(center);

							if (unit->Group != group) {
								d += 12800;
							}

							if (unit->House == House && unit->Class == Class->TaskForce->Members[typeindex].Class) {

								if ((d < bestdist || bestdist == -1) && Can_Add(unit, typeindex)) {
									best = unit;
									bestdist = d;
								}

							}
						}
					}

					if (best) {
						best->Assign_Target(NULL);
						Add(best);
						added++;

						/*
						**	If a transport is added to the team, the occupants
						**	are added by default.
						*/
						FootClass * f = best->Cargo.Attached_Object();
						while (f) {
							Add(f);
							f = (FootClass *)(ObjectClass *)f->Next;
						}
					}
				}
				break;
		}
	}
	return(added);
}


/***********************************************************************************************
 * TeamClass::Detach -- Removes specified target from team tracking.                           *
 *                                                                                             *
 *    When a target object is about to be removed from the game (e.g., it was killed), then    *
 *    any team that is looking at that target must abort from that target.                     *
 *                                                                                             *
 * INPUT:   target   -- The target object that is going to be removed from the game.           *
 *                                                                                             *
 *          all      -- Is the target going away for good as opposed to just cloaking/hiding?  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/29/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamClass::Detach(AbstractClass const * target, bool all)
{
	if (target == Tag) {
		Tag = NULL;
	}

	if (target == Member) {
		if (all) {
			Member = Member->Member;
		}
	}

	if (target == House) {
		House = NULL;
	}

	if (target == (AbstractClass *)Script) {
		Script = NULL;
	}

	/*
	**	If the target to detach matches the target of this team, then remove
	**	the target from this team's Tar/Nav com and let the chips fall
	**	where they may.
	*/
	if (Target == target) {
		Target = NULL;
	}
	if (MissionTarget == target) {
		MissionTarget = NULL;
	}

	if (Zone == target) {
		Zone = NULL;
	}

	if (Class == (TeamTypeClass *)target) {
		Class = NULL;
	}

	if (ClosestMember == (TeamTypeClass *)target) {
		ClosestMember = NULL;
	}

	if ((AbstractClass *)UnusedPtr1 == target) {
		UnusedPtr1 = NULL;
	}

	if (HouseToScout == target) {
		HouseToScout = NULL;
	}
}


/***********************************************************************************************
 * TeamClass::Calc_Center -- Determines average location of team members.                      *
 *                                                                                             *
 *    Use this routine to calculate the "center" location of the team. This is the average     *
 *    position of all members of the team. Using this center value it is possible to tell      *
 *    if a team member is too far away and where to head to in order to group up.              *
 *                                                                                             *
 * INPUT:   center   -- Average center target location of the team. Only initiated members     *
 *                      will be considered.                                                    *
 *                                                                                             *
 *          close_member--Location (as target) of the unit that is closest to the team's       *
 *                      target.                                                                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/29/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamClass::Calc_Center(AbstractClass *& center, AbstractClass *& close_member) const
{
	/*
	**	Presume there is no center. This will be confirmed in the following scanning
	**	operation.
	*/
	close_member = NULL;
	center = NULL;

	FootClass * team_member = Member;		// Working team member pointer.

	/*
	**	If there are no members of the team, then there can be no center point of the team.
	*/
	if (team_member == NULL) return;

	/*
	**	If the team is supposed to follow a nearby friendly unit, then the
	**	team's "center" will actually be that unit. Otherwise, calculated the
	**	average center location for the team.
	*/
	if (Script->Get_Current_Mission().Mission == TMISSION_HOUND_DOG) {

		/*
		**	First pick a member of the team. The closest friendly object to that member
		**	will be picked.
		*/
		if (!team_member) return;

		FootClass * closest = NULL; // Current closest friendly object.
		int distance = -1;          // Record of last closest distance calc.

		/*
		**	Scan through all vehicles.
		*/
		for (int unit_index = 0; unit_index < Units.Count(); unit_index++) {
			FootClass * trial_unit = Units[unit_index];

			if (_Is_It_Breathing(trial_unit) && House != trial_unit->House && trial_unit->House->Is_Ally(House) && this != trial_unit->Team) {
				//int trial_distance = team_member->Distance(trial_unit);
				Coord crd = team_member->Center_Coord() - trial_unit->Center_Coord();
				int trial_distance = (crd).Length();

				if (distance == -1 || trial_distance < distance) {
					distance = trial_distance;
					closest = trial_unit;
				}
			}
		}

		/*
		**	Scan through all infantry.
		*/
		for (int infantry_index = 0; infantry_index < Infantry.Count(); infantry_index++) {
			FootClass * trial_infantry = Infantry[infantry_index];

			if (_Is_It_Breathing(trial_infantry) && House != trial_infantry->House && trial_infantry->House->Is_Ally(House) && this != trial_infantry->Team) {
				int trial_distance = team_member->Distance_To(trial_infantry);

				if (distance == -1 || trial_distance < distance) {
					distance = trial_distance;
					closest = trial_infantry;
				}
			}
		}

		/*
		**	Set the center location as actually the friendly object that is closest. If there
		**	is no friendly object, then don't set any center location at all.
		*/
		if (closest) {
			if (closest->CurrentTube >= TUBE_FIRST) {
				Cell exit = Tubes[closest->CurrentTube]->Exit;
				center = &Map[exit];
			} else {
				center = closest;
			}
			close_member = Member;
		}

	} else {

		int	x = 0;                              // Accumulated X coordinate.
		int	y = 0;                              // Accumulated Y coordinate.
		int   dist = 0;                             // Closest recorded distance to team target.
		int	quantity = 0;                           // Number of team members counted.
		FootClass * closest = 0;                    // Closest member to target.

		/*
		**	Scan through all team members and accumulate the X and Y component of their
		**	location. Only team members that are active will be considered. Also keep
		**	track of the team member that is closest to the team's target.
		*/
		while (team_member != NULL) {
			if (_Is_It_Playing(team_member) && team_member->IsLocked) {

				/*
				**	Accumulate X and Y components of qualified team members.
				*/
				x += team_member->PositionCoord.X;
				y += team_member->PositionCoord.Y;
				quantity++;

				if (Class->IsGuardSlower && team_member->Is_Considered_Slow()) {
					x += team_member->PositionCoord.X;
					y += team_member->PositionCoord.Y;
					quantity++;
				}

				/*
				**	Keep a record of the team member that is nearest to the team's
				**	target.
				*/
				int try_dist = team_member->Relative_Distance(Target);
				if (!dist || try_dist < dist) {
					dist = try_dist;
					closest = team_member;
				}
			}

			team_member = team_member->Member;
		}

		/*
		**	If there were any qualifying members, then the team's center point can be
		**	determined.
		*/
		if (quantity) {
			x /= quantity;
			y /= quantity;
			Coord coord = Coord((int)x, (int)y);
			center = &Map[coord];

			/*
			**	If the center location is impassable, then just pick the location of
			**	one of the team members.
			*/
			if (closest->Can_Enter_Cell((CellClass *)center) == MOVE_OK) {
				center = closest;
			}
		}

		/*
		**	Record the position of the closest member to the team's target and
		**	that will be used as the regroup point.
		*/
		if (closest != NULL) {
			close_member = closest;
		}
	}
}


/***********************************************************************************************
 * TeamClass::Took_Damage -- Informs the team when the team member takes damage.               *
 *                                                                                             *
 *    This routine is used when a team member takes damage. Usually the team will react in     *
 *    some fashion to the attack. This reaction can range from running away to assigning this  *
 *    new target as the team's target.                                                         *
 *                                                                                             *
 * INPUT:   obj      -- The team member that was damaged.                                      *
 *                                                                                             *
 *          result   -- The severity of the damage taken.                                      *
 *                                                                                             *
 *          source   -- The perpetrator of the damage.                                         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/29/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamClass::Took_Damage(FootClass * , ResultType result, TechnoClass * source)
{
	if ((source != NULL) && (result != RESULT_NONE) && (!Class->IsSuicide)) {
		if (!IsMoving) {

			// TCTCTC
			// Should run to a better hiding place or disband into a group of hunting units.
			Zone = NULL;
			IsRegrouping = true;
			IsReforming = true;

		} else {
			/*
			** Respond to the attack, but not if we're an aircraft or a LST.
			*/
			if (source && !House->Is_Ally(source) && !Is_A_Member(source) && Member && Member->RTTI != RTTI_AIRCRAFT && Member->PrimaryWeapon != NULL) {
				if (Target != source) {

					if (Class->IsAnnoyance) {
						IsRegrouping = true;
						IsReforming = true;
						Zone = NULL;
					}

					/*
					**	Don't change target if the team's target is one that can fire as well. There is
					**	no point in endlessly shuffling between targets that have firepower.
					*/
					if (Target != NULL) {
						TechnoClass * techno = Dynamic_Cast<TechnoClass *>(Target);

						if (techno != NULL && techno->PrimaryWeapon != NULL) {
							if (Zone == NULL || techno->In_Range(Zone->Center_Coord())) {
								return;
							}
						}
					}

					/*
					**	Don't change target to aggressor if the aggressor cannot normally be attacked.
					*/
					if (source->RTTI == RTTI_AIRCRAFT) {
						return;
					}

					Target = source;

					FootClass * unit = Member;
					while (unit != NULL) {
						if (_Is_It_Playing(unit)) {
							unit->Assign_Target(NULL);
							unit->Assign_Destination(NULL);
						}
						unit = unit->Member;
					}
				}
			}
		}
	}
}


/***********************************************************************************************
 * TeamClass::Coordinate_Attack -- Handles coordinating a team attack.                         *
 *                                                                                             *
 *    This function is called when the team knows what it should attack. This routine will     *
 *    give the necessary orders to the members of the team.                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/06/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamClass::Coordinate_Attack(void)
{
	if (Target == NULL) {
		Target = MissionTarget;
	}

	/*
	**	Check if they're attacking a cell.  If the contents of the cell are
	**	a bridge or a building/unit/techno, then it's a valid target.  Otherwise,
	**	the target is invalid. This only applies to non-aircraft teams. An aircraft team
	**	can "attack" an empty cell and this is perfectly ok (paratrooper drop and parabombs
	**	are prime examples).
	*/
	if (Is_Target_Cell(Target) && Member != NULL && Fetch_A_Leader()->RTTI != RTTI_AIRCRAFT) {
		CellClass *cellptr = dynamic_cast<CellClass*>(Target);
		if (cellptr->Cell_Object()) {
			Target = cellptr->Cell_Object();
		}
	}

	if (Target == NULL) {
		IsNextMission = true;

	} else {

		TeamMissionClass mission = Script->Get_Current_Mission();
		bool has_attacker = false;
		FootClass * unit = Member;
		while (unit != NULL) {

			Coordinate_Conscript(unit);

			if (_Is_It_Playing(unit)) {
				if (mission.Mission == TMISSION_SPY && unit->RTTI == RTTI_INFANTRY && ((InfantryClass *)unit)->Class->IsCapture) {
					unit->Assign_Mission(MISSION_CAPTURE);
					unit->Assign_Target(Target);
				} else {
					if (unit->Mission != MISSION_ATTACK && unit->Mission != MISSION_ENTER && unit->Mission != MISSION_CAPTURE && (unit->Mission != MISSION_UNLOAD || !unit->Deploy_To_Fire())) {
						unit->Transmit_Message(RADIO_OVER_OUT);
						unit->Assign_Mission(MISSION_ATTACK);
						unit->Assign_Target(NULL);
						unit->Assign_Destination(NULL);
					}
				}

				if (unit->TarCom != Target && unit->TarCom == NULL) {
					unit->Assign_Target(Target);
				}

				if (unit->RTTI != RTTI_AIRCRAFT || unit->PrimaryWeapon == NULL || unit->Ammo > 0) {
					has_attacker = true;
				}
			}

			unit = unit->Member;
		}
		if (!has_attacker) {
			IsNextMission = true;
		}
	}
}


/***********************************************************************************************
 * TeamClass::Coordinate_Regroup -- Handles team idling (regrouping).                          *
 *                                                                                             *
 *    This routine is called when the team must delay at its current location. Team members    *
 *    are grouped together by this function. It is called when the team needs to sit and       *
 *    wait.                                                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Has the team completely regrouped?                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/06/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TeamClass::Coordinate_Regroup(void)
{
	FootClass * unit   = Member;
	bool retval = true;

	/*
	**	Regroup default logic.
	*/
	while (unit != NULL) {

		Coordinate_Conscript(unit);

		if (_Is_It_Playing(unit)) {

			if (unit->Distance(Zone) > Rule->StrayDistance && (unit->Mission != MISSION_GUARD_AREA || unit->TarCom == NULL)) {
				if (unit->NavCom == NULL) {
// TCTCTC
//				if (unit->NavCom == NULL || ::Distance(unit->NavCom, Zone) > Rule->StrayDistance) {
					unit->Assign_Mission(MISSION_MOVE);
					unit->Assign_Destination(Zone);

					retval = false;
					unit->Assign_Mission(MISSION_MOVE);
					Cell dest = unit->Adjust_Dest(Zone->Center_Coord().As_Cell());
					unit->Assign_Destination(&Map[dest]);
				}
			} else {

				/*
				**	The team is regrouping, so just sit here and wait.
				*/
				if (unit->Mission != MISSION_GUARD_AREA) {
					unit->Assign_Mission(MISSION_GUARD);
					unit->Assign_Destination(NULL);
				}
			}

		}

		unit = unit->Member;
	}
	if (retval == true) {
		IsRegrouping = false;
	}
	return(retval);
}


/***********************************************************************************************
 * TeamClass::Coordinate_Move -- Handles team movement coordination.                           *
 *                                                                                             *
 *    This routine is called when the team must move to a new location. Movement and grouping  *
 *    commands associated with this task are initiated here.                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/06/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamClass::Coordinate_Move(void)
{
	FootClass * unit = Member;
	bool finished = true;
	bool found = false;

	if (unit == NULL) return;

	if (Target == NULL) {
		Target = MissionTarget;
	}

	if (Target != NULL) {

		if (Class->TransportsReturnOnUnload) {
			FootClass * obj = unit;
			while (obj != NULL) {
				if (!obj) {
					break;
				}
				if (obj->TClass->Max_Passengers() > 0 && obj->ArchiveTarget == NULL) {
					obj->ArchiveTarget = &Map[obj->Get_Coord()];
				}
				obj = obj->Member;
			}

		}

		if (!Lagging_Units()) {

			while (unit != NULL) {

				/*
				**	Tell the unit, if necessary, that it should regroup
				**	with the main team location. If the unit is regrouping, then
				**	the team should continue NOT qualify as fully reaching the desired
				**	location.
				*/
				if (Coordinate_Conscript(unit)) {
					finished = false;
				}

				if (unit->Mission == MISSION_UNLOAD || unit->MissionQueue == MISSION_UNLOAD) {
					finished = false;
				}

				if (_Is_It_Playing(unit) && unit->Mission != MISSION_UNLOAD && unit->MissionQueue != MISSION_UNLOAD) {
					int stray = Rule->StrayDistance;
					if (unit->RTTI == RTTI_AIRCRAFT) {
						stray *= 3;
					}

					found = true;

					int dist = unit->Distance(Target);

					if (dist > stray ||
						(unit->HeightAGL < 0 &&
						Script->Get_Next_Mission().Mission != TMISSION_MOVE) ||
						(unit->RTTI == RTTI_AIRCRAFT &&
//						(unit->In_Which_Layer() == LAYER_TOP &&
						((AircraftClass *)unit)->Height > 0 &&
						&Map[unit->Center_Coord()] != Target &&
						Script->Get_Next_Mission().Mission != TMISSION_MOVE)) {

						if (!Class->IsAggressive || unit->TarCom == NULL) {
							if (unit->Mission != MISSION_MOVE) {
								unit->Assign_Mission(MISSION_MOVE);
								if (unit->Ready_To_Commence()) {
									unit->Commence();
								}
							}

							if (unit->NavCom == NULL) {
								unit->Assign_Destination(Target);
							}
							finished = false;
							goto UGH;
						}

					} else {
						if (unit->Mission == MISSION_MOVE && (unit->NavCom == NULL || unit->Distance(unit->NavCom) <= Rule->CloseEnoughDistance && !unit->Locomotion->Is_Moving())) {
							if (unit->TarCom == NULL) {
								unit->Assign_Destination(NULL);
								unit->Enter_Idle_Mode();
							}
						}

						UGH:

						/*
						**	If any member still has a valid NavCom then consider this
						**	movement mission to still be in progress. This will ensure
						**	that the members come to a complete stop before the next
						**	mission commences. Without this, the team will prematurely
						**	start on the next mission even when all members aren't yet
						**	in their proper spot.
						*/
						if (unit->NavCom != NULL) {
							finished = false;
						}
					}
				}

				unit = unit->Member;
			}
		} else {
			finished = false;
		}
	}

	/*
	**	If there are no initiated members to this team, then it certainly
	**	could not have managed to move to the target destination.
	*/
	if (!found) {
		finished = false;
	}

	/*
	**	If all the team members are close enough to the desired destination, then
	**	move to the next mission.
	*/
	if (finished && IsMoving) {
		IsNextMission = true;
	}
}


/***********************************************************************************************
 * TeamClass::Lagging_Units -- Finds and orders any lagging units to catch up.                 *
 *                                                                                             *
 *    This routine will examine the team and find any lagging units. The units are then        *
 *    ordered to catch up to the team member that is closest to the team's destination. This   *
 *    routine will not do anything unless lagging members are suspected. This fact is          *
 *    indicated by setting the IsLagging flag. The flag is set by some outside agent.          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Be sure to set IsLagging for the team if a lagging member is suspected.         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/01/1995 PWG : Created.                                                                 *
 *   04/11/1996 JLB : Modified.                                                                *
 *=============================================================================================*/
bool TeamClass::Lagging_Units(void)
{
	FootClass * unit = Member;
	bool lag = false;

	/*
	**	If the IsLagging bit is not set, then obviously there are no lagging
	**	units.
	*/
	if (!IsLagging) return(false);

	/*
	**	Scan through all of the units, searching for units who are having
	**	trouble keeping up with the pack.
	*/
	while (unit != NULL) {

		if (_Is_It_Playing(unit)) {
			int stray = Rule->StrayDistance;
			if (unit->RTTI ==  RTTI_AIRCRAFT) {
				stray *= 3;
			}
			if (Class->IsGuardSlower && !unit->Is_Considered_Slow()) {
				stray /= 3;
			}

			/*
			**	If we find a unit who has fallen too far away from the center of
			**	the pack, then we need to order that unit to catch up with the
			**	first unit.
			*/
			if (unit->Distance_To(ClosestMember) > stray) {
				if (unit->NavCom == NULL) {
					unit->Assign_Mission(MISSION_MOVE);
					unit->Assign_Destination(ClosestMember);
				}
				lag = true;

			} else {
				/*
				**	We need to order all of the other units to hold their
				**	position until all lagging units catch up.
				*/
				if (unit->Mission != MISSION_GUARD) {
					unit->Assign_Mission(MISSION_GUARD);
					unit->Assign_Destination(NULL);
				}
			}
		}
		unit = unit->Member;
	}

	/*
	**	Once we have handled the loop we know whether there are any lagging
	**	units or not.
	*/
	IsLagging = lag;
	return(lag);
}


/***********************************************************************************************
 * TeamClass::Coordinate_Conscript -- Gives orders to new recruit.                             *
 *                                                                                             *
 *    This routine will give the movement orders to the conscript so that it will group        *
 *    with the other members of the team.                                                      *
 *                                                                                             *
 * INPUT:   unit  -- Pointer to the conscript unit.                                            *
 *                                                                                             *
 * OUTPUT:  bool; Is the unit still scurrying to reach the team's current location?            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/06/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TeamClass::Coordinate_Conscript(FootClass * unit)
{
	if (_Is_It_Breathing(unit) && !unit->IsInitiated) {
		if (unit->Distance(Zone) > Rule->StrayDistance) {
			if (unit->NavCom == NULL) {
				unit->Assign_Mission(MISSION_MOVE);
				unit->Assign_Target(NULL);
				unit->Assign_Destination(Zone);
			}
			return(true);

		} else {

			/*
			**	This unit has gotten close enough to the team center so that it is
			**	now considered initiated. An initiated unit is considered when calculating
			**	the center of the team.
			*/
			unit->IsInitiated = true;
		}
	}
	return(false);
}


/***************************************************************************
 * TeamClass::Is_A_Member -- Tests if a unit is a member of a team         *
 *                                                                         *
 * INPUT:      none                                                        *
 *                                                                         *
 * OUTPUT:     none                                                        *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/16/1995 PWG : Created.                                             *
 *=========================================================================*/
bool TeamClass::Is_A_Member(void const * who) const
{
	FootClass * unit = Member;
	while (unit != NULL) {
		if (unit == who) {
			return(true);
		}
		unit = unit->Member;
	}
	return(false);
}


/***************************************************************************
 * TeamClass::Suspend_Teams -- Suspends activity for low priority teams    *
 *                                                                         *
 * INPUT:   int priority - determines what is considered low priority.     *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   06/19/1995 PWG : Created.                                             *
 *=========================================================================*/
void TeamClass::Suspend_Teams(int priority, HouseClass const * house)
{
	for (int index = 0; index < Teams.Count(); index++) {
		TeamClass * team = Teams[index];

		/*
		**	If a team is below the "survival priority level", then it gets
		**	destroyed. The team members are then free to be reassigned.
		*/
		if (team != NULL && house == team->House && team->Class->RecruitPriority < priority) {
			FootClass * unit = team->Member;
			while (team->Member) {
				team->Remove(team->Member);
			}
			team->IsAltered = team->JustAltered = true;
			team->SuspendTimer = Rule->SuspendDelay * TICKS_PER_MINUTE;
			team->Suspended = true;
		}
	}
}


/***********************************************************************************************
 * TeamClass::Is_Leaving_Map -- Checks if team is in process of leaving the map                *
 *                                                                                             *
 *    This routine is used to see if the team is leaving the map. A team that is leaving the   *
 *    map gives implicit permission for its members to leave the map.                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is this team trying to leave the map?                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TeamClass::Is_Leaving_Map(void) const
{
	if (IsMoving && Script->Has_Missions_Remaining()) {
		TeamMissionClass const mission = Script->Get_Current_Mission();

		if (mission.Mission == TMISSION_MOVE && !Map.In_Local_Radar(Scen->Get_Waypoint_Cell(mission.Data.Value))) {
			return(true);
		}
	}
	return(false);
}


/***********************************************************************************************
 * TeamClass::Has_Entered_Map -- Determines if the entire team has entered the map.            *
 *                                                                                             *
 *    This will examine all team members and only if all of them have entered the map, will    *
 *    it return true. This routine is used to recognize the case of a team that has been       *
 *    generated off map and one that has already entered game play. This knowledge can lead    *
 *    to more intelligent behavior regarding team and member disposition.                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Have all members of this team entered the map?                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TeamClass::Has_Entered_Map(void) const
{
	bool ok = true;
	FootClass * foot = Member;
	while (foot != NULL) {
		if (!foot->IsLocked) {
			ok = false;
			break;
		}
		foot = (FootClass *)foot->Member;
	}
	return(ok);
}


/***********************************************************************************************
 * TeamClass::Scan_Limit -- Force all members of the team to have limited scan range.          *
 *                                                                                             *
 *    This routine is used when one of the team members cannot get within range of the team's  *
 *    target. In such a case, the team must be assigned a new target and all members of that   *
 *    team must recognize that a restricted target scan is required. This is done by clearing  *
 *    out the team's target so that it will be forced to search for a new one. Also, since the *
 *    members are flagged for short scanning, whichever team member is picked to scan for a    *
 *    target will scan for one that is within range.                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The team will reassign its target as a result of this routine.                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamClass::Scan_Limit(void)
{
	Assign_Mission_Target(NULL);
	FootClass * foot = Member;
	while (foot != NULL) {
		foot->Assign_Target(NULL);
		foot->IsScanLimited = true;
		foot = foot->Member;
	}
}


/***********************************************************************************************
 * TeamClass::Fetch_A_Leader -- Looks for a suitable leader member of the team.                *
 *                                                                                             *
 *    This will scan through the team members looking for one that is suitable as a leader     *
 *    type. A team can sometimes contain limboed or unarmed members. These members are not     *
 *    suitable for leadership roles.                                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a suitable leader type unit.                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/27/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
FootClass * TeamClass::Fetch_A_Leader(void) const
{
	FootClass * leader = Member;

	/*
	**	Scan through the team members trying to find one that is an active member and
	**	is equipped with a weapon.
	*/
	while (leader != NULL) {
		if (_Is_It_Playing(leader) /*&& leader->Is_Weapon_Equipped()*/) break;
		leader = leader->Member;
	}

	/*
	**	If no suitable leader was found, then just return with the first conveniently
	**	accessable team member. This presumes that some member is better than no member
	**	at all.
	*/
	if (leader == NULL) {
		leader = Member;
	}

	return(leader);
}


/// <summary>
/// Lists the members this team carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TeamClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Class);
	stream.Serialize(Script);
	stream.Serialize(House);
	stream.Serialize(HouseToScout);
	stream.Serialize(Zone);
	stream.Serialize(ClosestMember);
	stream.Serialize(MissionTarget);
	stream.Serialize(Target);
	// UnusedPtr1 -- untyped, so it carries no swizzle identity, and nothing reads it.
	stream.Serialize(Total);
	stream.Serialize(Risk);
	stream.Serialize(CreationFrame);
	stream.Serialize(Member);
	stream.Serialize(TimeOut);
	stream.Serialize(SuspendTimer);
	stream.Serialize(Tag);
	stream.Serialize(IsDeleteTypeWhenDone);
	stream.Serialize(IsRegrouping);
	stream.Serialize(NeedsReinforcement);
	stream.Serialize(IsForcedActive);
	stream.Serialize(IsHasBeen);
	stream.Serialize(IsFullStrength);
	stream.Serialize(IsUnderStrength);
	stream.Serialize(IsReforming);
	stream.Serialize(IsLagging);
	stream.Serialize(IsAltered);
	stream.Serialize(JustAltered);
	stream.Serialize(IsMoving);
	stream.Serialize(IsNextMission);
	stream.Serialize(IsLeaveMap);
	stream.Serialize(Suspended);
	stream.Serialize(Succeeded);
	stream.Serialize(Quantity);
}


/// <summary>
/// Fetches the class identifier of this object.
/// This routine is part of the persistence contract and is what allows the save game loader
/// to recognize a team when it reads one back in.
/// </summary>
/// <param name="retval">Pointer to the identifier to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT TeamClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_TeamClass;
	return(S_OK);
}


/// <summary>
/// Adds the state of this team to the running game checksum.
/// This routine is used by the network code to detect when two machines have drifted out of
/// sync with one another.
/// </summary>
void TeamClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);

	crc(Class->Fetch_ID());

	crc(House->Fetch_ID());

	if (ClosestMember != NULL) {
		crc(ClosestMember->Fetch_ID());
	}

	crc(Total);
	crc(Risk);

	if (Member != NULL) {
		crc(Member->Fetch_ID());
	}

	crc((int)TimeOut);
	crc((int)SuspendTimer);

	if (Tag != NULL) {
		crc(Tag->Fetch_ID());
	}

	crc(IsForcedActive);
	crc(IsHasBeen);
	crc(IsFullStrength);
	crc(IsUnderStrength);
	crc(IsReforming);
	crc(IsLagging);
	crc(IsAltered);
	crc(JustAltered);
	crc(IsMoving);
	crc(IsNextMission);
	crc(IsLeaveMap);
	crc(Suspended);

	crc(Quantity, sizeof(Quantity));
}


/// <summary>
/// Handles the move to shroud team mission.
/// This routine sends the team toward the nearest shrouded cell, so that it can go and
/// uncover a piece of the map.
/// </summary>
/// <param name="first_time">Is this the first time this mission has been processed?</param>
void TeamClass::TMission_GOTO_SHROUD(TeamMissionClass * mission, bool first_time)
{
	if (first_time) {
		FootClass *unit = (FootClass *)Zone;
		if (unit == NULL) {
			unit = Member;
		}
		Assign_Mission_Target(Map.Find_Nearby_Shroud(unit));
	}
	Coordinate_Move();
}


/// <summary>
/// Handles the move to cell team mission.
/// This routine sends the team to the scripted map cell.
/// </summary>
/// <param name="first_time">Is this the first time this mission has been processed?</param>
void TeamClass::TMission_MOVECELL(TeamMissionClass * mission, bool first_time)
{
	if (first_time) {
		Cell cell;
		cell.X = mission->Data.Value % 1000;
		cell.Y = mission->Data.Value / 1000;
		if (Map.In_Radar(cell)) {
		Assign_Mission_Target(&Map[cell]);
	}
	}
	Coordinate_Move();
}


/// <summary>
/// Handles the move to waypoint team mission.
/// This routine sends the team to the scripted waypoint, settling for a nearby cell when the
/// leader cannot reach the waypoint itself.
/// </summary>
/// <param name="first_time">Is this the first time this mission has been processed?</param>
void TeamClass::TMission_MOVE(TeamMissionClass * mission, bool first_time)
{
	if (first_time) {
		FootClass * leader = Fetch_A_Leader();
		Cell movecell = Scen->Get_Waypoint_Cell(mission->Data.Value);
		Assign_Mission_Target(&Map[movecell]);
		if (!Is_Leaving_Map()) {
			if (leader->Can_Enter_Cell(&Map[movecell], FACING_NONE, -1, NULL, false) != MOVE_OK) {
				movecell = Map.Nearby_Location(movecell, leader->TClass->Speed);
			}
		}
		if (movecell != CELL_NONE) {
			Assign_Mission_Target(&Map[movecell]);
		} else {
			Assign_Mission_Target(NULL);
		}

	}
	Coordinate_Move();
}


/// <summary>
/// Handles the attack waypoint team mission.
/// This routine points the team at whatever sits on the scripted waypoint and then
/// coordinates the attack. The mission is abandoned when there is nothing there worth
/// shooting at, or the team has run dry of ammunition.
/// </summary>
/// <param name="first_time">Is this the first time this mission has been processed?</param>
void TeamClass::TMission_ATT_WAYPT(TeamMissionClass * mission, bool first_time)
{
	if (first_time) {
		CellClass * trgt = (CellClass *)Scen->Get_Waypoint_Target(mission->Data.Value);
		if (Is_Target_Cell(trgt)) {
			ObjectClass * otrgt = trgt->Cell_Object(Point2D(0,0),trgt->IsUnderBridge);
			if (otrgt) {
				trgt = (CellClass *)otrgt;
			}
		}
		Assign_Mission_Target(trgt);
	}

	if (MissionTarget && Ammo_Check()) {
		Coordinate_Attack();
	} else {
		Assign_Mission_Target(NULL);
		IsNextMission = true;
	}
}


/***********************************************************************************************
 * TeamClass::TMision_Patrol -- Handles patrolling from one location to another.               *
 *                                                                                             *
 *    A patrolling team will move to the designated waypoint, but along the way it will        *
 *    periodically scan for nearby enemies. If an enemy is found, the patrol mission turns     *
 *    into an attack mission until the target is destroyed -- after which it resumes its       *
 *    patrol duties.                                                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the delay before the next call to this routine is needed.             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/12/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamClass::TMission_PATROL(TeamMissionClass * mission, bool first_time)
{
	CellClass *trgt = (CellClass *)Scen->Get_Waypoint_Target(mission->Data.Value);
	if (first_time) {
		Assign_Mission_Target(trgt);
	}

	/*
	**	Reassign the movement destination if the target has been prematurely
	**	cleared (probably because the object has been destroyed).
	*/
	if (Target == NULL) {
		TeamMissionClass mission = Script->Get_Current_Mission();
		if ((unsigned)mission.Data.Value < WAYPT_COUNT) {
			Assign_Mission_Target(trgt);
		}
	}

	/*
	**	Every so often, scan for a nearby enemy.
	*/
	if (Frame % int(Rule->PatrolTime * TICKS_PER_MINUTE) == 0) {
		FootClass * leader = Fetch_A_Leader();
		if (leader != NULL) {
			AbstractClass * target = leader->Greatest_Threat(ThreatType(THREAT_NORMAL|THREAT_RANGE), leader->PositionCoord, false);

			if (target != NULL) {
				Assign_Mission_Target(target);
			} else if (Target != trgt) {
				Assign_Mission_Target(NULL);
			}
		}
	}

	/*
	**	If the mission target looks like it should be attacked, then do so, otherwise
	**	treat it as a movement destination.
	*/
	if (dynamic_cast<ObjectClass *>(Target) != NULL) {
		Coordinate_Attack();
	} else {
		Coordinate_Move();
	}
}


/***********************************************************************************************
 * TeamClass::TMission_Spy -- Perform the team spy mission.                                    *
 *                                                                                             *
 *    This will give the team a spy mission to the location specified. It is presumed that     *
 *    the location of the team mission actually resides under the building to be spied. If     *
 *    no building exists at the location, then the spy operation is presumed to be a mere      *
 *    move operation.                                                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the delay before the next team logic operation should occur.          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamClass::TMission_SPY(TeamMissionClass * mission, bool first_time)
{
	if (first_time) {
		Assign_Mission_Target(Scen->Get_Waypoint_Target(mission->Data.Value));
	}

	if (Is_Target_Cell(MissionTarget)) {
		CellClass * cellptr = MissionTarget->As_CellClass();

		ObjectClass * bldg = cellptr->Cell_Object();
		if (bldg != NULL) {
			Assign_Mission_Target(bldg);
			Coordinate_Attack();
		}
	} else {
		if (MissionTarget == NULL) {
			Assign_Mission_Target(NULL);
			IsNextMission = true;
		} else {
			Coordinate_Attack();
		}
	}
}


/// <summary>
/// Handles the scatter team mission.
/// This routine sends every member of the team scattering to a nearby cell.
/// </summary>
void TeamClass::TMission_SCATTER(TeamMissionClass * mission, bool)
{
	FootClass *unit = Member;
	while (unit != NULL) {
		unit->Scatter(COORD_NONE, true);
		unit = unit->Member;
	}
	IsNextMission = true;
}


/// <summary>
/// Handles the change house team mission.
/// This routine hands every member of the team over to the scripted house, just as though
/// they had been captured.
/// </summary>
void TeamClass::TMission_CHANGE_HOUSE(TeamMissionClass * mission, bool)
{
	HouseClass * newowner = House_From_HousesType(mission->Data.House);
	if (newowner != NULL) {
		FootClass *unit = Member;
		while (unit != NULL) {
			FootClass *next = unit->Member;
			unit->Captured(newowner);
			unit = next;
		}
	}
	IsNextMission = true;
}


/// <summary>
/// Handles the change team team mission.
/// This routine creates a team of the scripted type, hands every member over to it, and then
/// disbands this team.
/// </summary>
/// <remarks>The team does not survive this mission -- the caller must not touch it
/// afterward.</remarks>
void TeamClass::TMission_TEAMCHANGE(TeamMissionClass * mission, bool)
{
	TeamClass *team = new TeamClass(TeamTypes[mission->Data.Value], House);

	FootClass *unit = Member;
	while (unit != NULL) {
		this->Remove(unit);
		team->Add(unit);
		unit = Member;
	}
	delete this;
}


/// <summary>
/// Handles the change script team mission.
/// This routine hands the team a fresh script of the scripted type, discarding whatever it
/// was working from before.
/// </summary>
void TeamClass::TMission_SCRIPT(TeamMissionClass * mission, bool)
{
	if (Script != NULL) {
		delete Script;
	}

	Script = new ScriptClass(ScriptTypes[mission->Data.Value]);
	Script->Stop_Script();
}


/***********************************************************************************************
 * TeamClass::TMission_Attack -- Perform the team attack mission command.                      *
 *                                                                                             *
 *    This will tell the team to attack the quarry specified in the team command. If the team  *
 *    already has a target, this it is presumed that this target take precidence and it won't  *
 *    be changed.                                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the delay before the next team logic operation should occur.          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamClass::TMission_ATTACK(TeamMissionClass * mission, bool)
{
	if (MissionTarget == NULL && Member != NULL) {

		/*
		**	Pick a team leader that has a weapon. Only in the case of no
		**	team members having any weapons, will a member without a weapon
		**	be chosen.
		*/
		FootClass const * candidate = Fetch_A_Leader();

		/*
		**	Have the team leader pick what the next team target will be.
		*/
		switch (mission->Data.Quarry) {
			case QUARRY_ANYTHING:
				Assign_Mission_Target(candidate->Greatest_Threat(THREAT_NORMAL, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
				break;

			case QUARRY_BUILDINGS:
				Assign_Mission_Target(candidate->Greatest_Threat(THREAT_BUILDINGS, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
				break;

			case QUARRY_HARVESTERS:
				Assign_Mission_Target(candidate->Greatest_Threat(THREAT_TIBERIUM, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
				break;

			case QUARRY_INFANTRY:
				Assign_Mission_Target(candidate->Greatest_Threat(THREAT_INFANTRY, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
				break;

			case QUARRY_VEHICLES:
				Assign_Mission_Target(candidate->Greatest_Threat(THREAT_VEHICLES, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
				break;

			case QUARRY_FACTORIES:
				Assign_Mission_Target(candidate->Greatest_Threat(THREAT_FACTORIES, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
				break;

			case QUARRY_DEFENSE:
				Assign_Mission_Target(candidate->Greatest_Threat(THREAT_BASE_DEFENSE, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
				break;

			case QUARRY_THREAT:
				Assign_Mission_Target(candidate->Greatest_Threat(THREAT_NORMAL, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
				break;

			case QUARRY_POWER:
				Assign_Mission_Target(candidate->Greatest_Threat(THREAT_POWER, candidate->PositionCoord, Class->OnlyTargetHouseEnemy));
				break;

			default:
				break;
		}
		if (MissionTarget == NULL || !Ammo_Check()) IsNextMission = true;
	}
	if (MissionTarget == NULL || !Ammo_Check()) IsNextMission = true;

	Coordinate_Attack();
}


/***********************************************************************************************
 * TeamClass::TMission_Load -- Tells the team to load onto the transport now.                  *
 *                                                                                             *
 *    This routine tells all non-transport units in the team to climb onto the transport in the*
 *    team.  Note the transport must be a member of this team.                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/28/1996 BWG : Created.                                                                 *
 *=============================================================================================*/
void TeamClass::TMission_LOAD(TeamMissionClass * mission, bool)
{
	FootClass * unit = Member;
	FootClass * trans = NULL;

	/*
	 * First figure out whether our transport should be an aircraft or a vehicle.
	 */
	bool transportisaircraft = Has_Air_Transport();

	/*
	** Locate the transport in the team, if there is one.  There should
	** only be one transport in the team.
	*/
	while (unit != NULL) {
		if (unit->Techno_Type_Class()->Max_Passengers() > 0 && unit->Cargo.How_Many() < unit->Techno_Type_Class()->Max_Passengers()) {
			if ((!transportisaircraft && unit->Fetch_RTTI() == RTTI_UNIT) || (transportisaircraft && unit->Fetch_RTTI() == RTTI_AIRCRAFT)) {
				trans = unit;
				break;
			}
		}
		unit = unit->Member;
	}

	/*
	**	In the case of no transport available, then consider the mission complete
	**	since it can never complete otherwise.
	*/
	if (trans == NULL) {
		IsNextMission = true;
		return;
	}

	/*
	**	If the transport is already in radio contact, then this means that
	**	it is in the process of loading. During this time, don't bother to assign
	**	the enter mission to the other team members.
	*/
	if (trans->In_Radio_Contact()) {
		return;
	}

	/*
	**	Find a member to assign the entry logic for.
	*/
	bool finished = true;
	unit = Member;	// re-point at the first member of the team again.
	while (unit != NULL && Total > 1) {
		Coordinate_Conscript(unit);

		/*
		**	Only assign the mission if the unit is not the transport.
		*/
		if (_Is_It_Playing(unit) && unit != trans) {
			finished = false;
			if (unit->Mission != MISSION_ENTER) {
				unit->Assign_Mission(MISSION_ENTER);
				unit->Assign_Target(NULL);
				unit->Assign_Destination(trans);
				break;
			}
		}

		unit = unit->Member;
	}

	if (finished) {
		IsNextMission = true;
	}
}


/// <summary>
/// Handles the deploy team mission.
/// This routine orders every member that can deploy into a structure to do so, clearing the
/// ground for the building first if need be. The mission only completes once the team has
/// nothing left to deploy.
/// </summary>
void TeamClass::TMission_DEPLOY(TeamMissionClass * mission, bool)
{
	FootClass * obj = Member;
	bool finished = true;

	while (obj != NULL) {

		Coordinate_Conscript(obj);

		if (_Is_It_Playing(obj)) {

			if (obj->RTTI == RTTI_UNIT) {
				UnitClass * unit = (UnitClass *)obj;
				if (unit->Class->DeploysInto != NULL) {
					finished = false;
					if (unit->Mission != MISSION_UNLOAD) {
						unit->Mark(MARK_UP);
						if (!unit->Class->DeploysInto->Legal_Placement(unit->PositionCell, NULL)) {
							unit->Class->DeploysInto->Flush_For_Placement(unit->PositionCell, unit->House);
							unit->Mark(MARK_DOWN);
						} else {
							unit->Mark(MARK_DOWN);
							unit->Assign_Destination(NULL);
							unit->Assign_Target(NULL);
							unit->Assign_Mission(MISSION_UNLOAD);
						}

					}
				}
			}
		}

		obj = obj->Member;
	}

	if (finished) {
		IsNextMission = true;
	}
}


/// <summary>
/// Handles the guard team mission.
/// This routine holds the team in place for the scripted number of seconds, keeping it
/// regrouped while it waits.
/// </summary>
/// <param name="first_time">Is this the first time this mission has been processed?</param>
void TeamClass::TMission_GUARD(TeamMissionClass * mission, bool first_time)
{
	if (first_time) {
		TimeOut = TICKS_PER_SECOND * mission->Data.Value;
	}

	Coordinate_Regroup();

	/*
	**	Check for mission time out condition. If the mission does in fact time out, then
	**	flag it so that the team mission list will advance.
	*/
	if (TimeOut == 0) {
		IsNextMission = true;
	}
}


/***********************************************************************************************
 * TeamClass::Coordinate_Do -- Handles the team performing specified mission.                  *
 *                                                                                             *
 *    This will assign the specified mission to the team. If there are team members that are   *
 *    too far away from the center of the team, then they will be told to move to the team's   *
 *    location.                                                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This only works if the special mission the team members are to perform does not *
 *             require extra parameters. The ATTACK and MOVE missions are particularly bad.    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/11/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamClass::TMission_DO(TeamMissionClass * mission, bool)
{
	FootClass * unit = Member;
	MissionType do_mission = mission->Data.Mission;

	/*
	**	For each unit either head it back to the team center or give it the main
	**	team mission order as appropriate.
	*/
	while (unit != NULL) {

		Coordinate_Conscript(unit);

		if (_Is_It_Playing(unit)) {

			if (unit->TarCom == NULL && unit->NavCom == NULL && unit->Distance(Zone) > Rule->StrayDistance * 2 && do_mission != MISSION_GUARD_AREA) {

				/*
				**	Only if the unit isn't already heading to regroup with the team, will it
				**	be given orders to do so.
				*/
				unit->Assign_Mission(MISSION_MOVE);
				unit->Assign_Destination(Zone);
				unit->Assign_Mission(MISSION_MOVE);
				Cell dest = unit->Adjust_Dest(Zone->Center_Coord().As_Cell());
				unit->Assign_Destination(&Map[dest]);

			} else {

				/*
				**	The team is regrouping, so just sit here and wait.
				*/
				if (unit->TarCom == NULL && unit->NavCom == NULL && unit->Mission != do_mission && (do_mission != MISSION_GUARD || unit->Mission != MISSION_UNLOAD)) {
					unit->ArchiveTarget = NULL;
					unit->Assign_Mission(do_mission);
					unit->Assign_Target(NULL);
					unit->Assign_Destination(NULL);
				}
			}

		}

		unit = unit->Member;
	}
}


/***********************************************************************************************
 * TeamClass::TMission_Set_Global -- Performs a set global flag operation.                     *
 *                                                                                             *
 *    This routine is used by the team to set a global variable but otherwise perform no       *
 *    visible effect on the team. By using this routine, sophisticated trigger dependencies    *
 *    can be implemented.                                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the delay before the next team logic operation should occur.          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamClass::TMission_SET_GLOBAL(TeamMissionClass * mission, bool)
{
	Scen->Set_Global_To(mission->Data.Value, true);
	IsNextMission = true;
}


/// <summary>
/// Handles the clear global flag team mission.
/// This routine clears a scenario global variable but otherwise has no visible effect on the
/// team. It is the counterpart of the set global flag mission.
/// </summary>
void TeamClass::TMission_CLEAR_GLOBAL(TeamMissionClass * mission, bool)
{
	Scen->Set_Global_To(mission->Data.Value, false);
	IsNextMission = true;
}


/// <summary>
/// Handles the set local flag team mission.
/// This routine sets a scenario local variable but otherwise has no visible effect on the
/// team. Sophisticated trigger dependencies can be built with it.
/// </summary>
void TeamClass::TMission_SET_LOCAL(TeamMissionClass * mission, bool)
{
	Scen->Set_Local_To(mission->Data.Value, true);
	IsNextMission = true;
}


/// <summary>
/// Handles the clear local flag team mission.
/// This routine clears a scenario local variable but otherwise has no visible effect on the
/// team. Sophisticated trigger dependencies can be built with it.
/// </summary>
void TeamClass::TMission_CLEAR_LOCAL(TeamMissionClass * mission, bool)
{
	Scen->Set_Local_To(mission->Data.Value, false);
	IsNextMission = true;
}


/***********************************************************************************************
 * TeamClass::TMission_Follow -- Perform the "follow friendlies" team command.                 *
 *                                                                                             *
 *    This will cause the team members to search out and follow the nearest friendly object.   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the delay before the next team logic operation should be performed.   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamClass::TMission_HOUND_DOG(TeamMissionClass * mission, bool)
{
	Calc_Center(Zone, ClosestMember);
	Target = Zone;
	Coordinate_Move();

	TechnoClass *trgt = Dynamic_Cast<TechnoClass *>(Zone);
	if (trgt != NULL) {
		FootClass *unit = Member;
		while (unit != NULL) {
			if (_Is_It_Playing(unit)) {
				if (unit->TarCom != NULL && unit->TarCom != trgt->TarCom) {
					unit->Assign_Target(NULL);
				}
				if (trgt->TarCom != NULL) {
					if (!unit->Locomotion->Is_Moving() && unit->Is_Weapon_Equipped() && unit->Mission == MISSION_GUARD) {
						unit->Assign_Mission(MISSION_ATTACK);
						unit->Assign_Target(trgt->TarCom);
					}
				}
			}
			unit = unit->Member;

		}
		//IsNextMission = true;
	}
}


/// <summary>
/// Handles the unpanic team mission.
/// This routine calms every member of the team back down after a panic mission.
/// </summary>
void TeamClass::TMission_UNPANIC(TeamMissionClass * mission, bool)
{
	FootClass *unit = Member;
	while (unit != NULL) {
		unit->Stop_Fear();
		unit = unit->Member;
	}
	IsNextMission = true;
}


/// <summary>
/// Handles the force facing team mission.
/// This routine turns every member of the team toward the scripted direction. The mission
/// only completes once none of the members are still on the move.
/// </summary>
void TeamClass::TMission_FORCE_FACING(TeamMissionClass * mission, bool)
{
	FootClass *unit = Member;
	bool is_complete = true;
	while (unit != NULL) {
		if (unit->Locomotion->Is_Moving()) {
			is_complete = false;
		} else {
			if (unit->PrimaryFacing.Current() != mission->Data.Facing) {
				unit->Locomotion->Do_Turn(DirType(mission->Data.Facing));
			}
		}
		unit = unit->Member;
	}
	if (is_complete) {
		IsNextMission = true;
	}

}


/// <summary>
/// Handles the panic team mission.
/// This routine frightens every member of the team into running for cover.
/// </summary>
void TeamClass::TMission_PANIC(TeamMissionClass * mission, bool)
{
	FootClass *unit = Member;
	while (unit != NULL) {
		unit->Start_Fear();
		unit = unit->Member;
	}
	IsNextMission = true;
}


/// <summary>
/// Handles the berzerk team mission.
/// This routine sends every member of the team berzerk, so that they will lash out at
/// whatever they happen to find.
/// </summary>
void TeamClass::TMission_BERZERK(TeamMissionClass * mission, bool)
{
	FootClass *unit = Member;
	while (unit != NULL) {
		unit->Berzerk();
		unit = unit->Member;
	}
	IsNextMission = true;
}


/// <summary>
/// Handles the idle animation team mission.
/// This routine has every member of the team perform the scripted idle animation.
/// </summary>
void TeamClass::TMission_IDLE_ANIM(TeamMissionClass * mission, bool)
{
	FootClass *unit = Member;
	while (unit != NULL) {
		unit->Do_Idle(mission->Data.Value);
		unit = unit->Member;
	}
	IsNextMission = true;
}


/***********************************************************************************************
 * TeamClass::TMission_Loop -- Causes the team mission processor to jump to new location.      *
 *                                                                                             *
 *    This is equivalent to a jump or goto command. It will alter the team command processing  *
 *    such that it will continue processing at the command number specified.                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the delay before the next team logic operation should be performed.   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamClass::TMission_LOOP(TeamMissionClass * mission, bool)
{
	Script->Set_Line(mission->Data.Value-2);
	IsNextMission = true;
}


/// <summary>
/// Handles the win team mission.
/// This routine flags the human player as having won the scenario.
/// </summary>
void TeamClass::TMission_WIN(TeamMissionClass * mission, bool)
{
	PlayerPtr->Flag_To_Win();
	IsNextMission = true;
}


/// <summary>
/// Handles the lose team mission.
/// This routine flags the human player as having lost the scenario.
/// </summary>
void TeamClass::TMission_LOSE(TeamMissionClass * mission, bool)
{
	PlayerPtr->Flag_To_Lose();
	IsNextMission = true;
}


/// <summary>
/// Handles the play speech team mission.
/// This routine has the announcer speak the scripted line.
/// </summary>
void TeamClass::TMission_PLAY_SPEECH(TeamMissionClass * mission, bool)
{
	Speak(mission->Data.Speech);
	IsNextMission = true;
}


/// <summary>
/// Handles the play sound team mission.
/// This routine plays the scripted sound effect.
/// </summary>
void TeamClass::TMission_PLAY_SOUND(TeamMissionClass * mission, bool)
{
	Sound_Effect(mission->Data.Sound);
	IsNextMission = true;
}


/// <summary>
/// Handles the play movie team mission.
/// This routine plays the scripted movie before the game resumes.
/// </summary>
void TeamClass::TMission_PLAY_MOVIE(TeamMissionClass * mission, bool)
{
	Play_Movie(mission->Data.Movie, THEME_NONE, true);
	IsNextMission = true;
}


/// <summary>
/// Handles the play music team mission.
/// This routine queues up the scripted theme so that it plays once the current one is done.
/// </summary>
void TeamClass::TMission_PLAY_MUSIC(TeamMissionClass * mission, bool)
{
	Theme.Queue_Song(mission->Data.Theme);
	IsNextMission = true;
}


/// <summary>
/// Handles the reduce tiberium team mission.
/// This routine thins out the tiberium growing around each member of the team.
/// </summary>
void TeamClass::TMission_REDUCE_TIBERIUM(TeamMissionClass * mission, bool)
{
	FootClass *unit = Member;
	while (unit != NULL) {
		Cell cell = unit->Center_Coord().As_Cell();
		Map.Area_Reduce_Tiberium(cell);
		unit = unit->Member;
	}
	IsNextMission = true;
}


/// <summary>
/// Handles the begin production team mission.
/// This routine wakes up the team type's house so that it starts building for itself.
/// </summary>
void TeamClass::TMission_BEGIN_PRODUCTION(TeamMissionClass * mission, bool)
{
	if (Class->House != NULL) {
		Class->House->IsStarted = true;
	}
	IsNextMission = true;
}


/// <summary>
/// Handles the fire sale team mission.
/// This routine puts the team type's house into its endgame state, so that it sells off its
/// base and throws everything it has left at the enemy.
/// </summary>
void TeamClass::TMission_FIRE_SALE(TeamMissionClass * mission, bool)
{
	if (Class->House != NULL) {
		Class->House->State = STATE_ENDGAME;
	}
	IsNextMission = true;
}


/// <summary>
/// Handles the self destruct team mission.
/// This routine kills off the members of the team by applying lethal damage to them, so that
/// the usual death animations and trigger events occur.
/// </summary>
void TeamClass::TMission_SELF_DESTRUCT(TeamMissionClass * mission, bool)
{
	bool doit = true;
	while (doit) {
		doit = false;

		FootClass *unit = Member;
		while (unit != NULL) {
			if (unit->Strength > 0 && unit->IsActive && unit->IsDown && !unit->IsInLimbo) {
				int damage = unit->Strength;
				unit->Take_Damage(damage, 0, Rule->C4Warhead, NULL, true);
				doit = true;
			}
			unit = unit->Member;
		}
	}
	IsNextMission = true;
}


/// <summary>
/// Handles the destroy members team mission.
/// This routine quietly takes every member of the team out of the game. Unlike the self
/// destruct mission, nothing is damaged and nothing explodes.
/// </summary>
void TeamClass::TMission_DESTROY_MEMBERS(TeamMissionClass * mission, bool)
{
	FootClass *unit = Member;
	while (unit != NULL) {
		FootClass *next = unit->Member;
		unit->Limbo();
		unit->Delete_Me();
		unit = next;
	}
	IsNextMission = true;
}


/// <summary>
/// Handles the ion storm start team mission.
/// This routine kicks off an ion storm of the scripted duration, unless one is already under
/// way.
/// </summary>
void TeamClass::TMission_ION_STORM_START(TeamMissionClass * mission, bool)
{
	if (!IonStormClass::Is_Ion_Storm_Active()) {
		IonStormClass::Ion_Storm_Begin(mission->Data.Value, TICKS_PER_SECOND * Rule->LightningDeferment);
	}
	IsNextMission = true;
}


/// <summary>
/// Handles the ion storm end team mission.
/// This routine shuts down the ion storm if one happens to be raging.
/// </summary>
void TeamClass::TMission_ION_STORM_END(TeamMissionClass * mission, bool)
{
	if (IonStormClass::Is_Ion_Storm_Active()) {
		IonStormClass::Ion_Storm_End();
	}
	IsNextMission = true;
}


/// <summary>
/// Handles the center viewpoint team mission.
/// This routine scrolls the tactical map over to the team's current zone at the scripted
/// speed, allowing for any bridge the team happens to be standing on.
/// </summary>
void TeamClass::TMission_CENTER_VIEWPOINT(TeamMissionClass * mission, bool)
{
	Coord coord = Zone->Center_Coord();
	coord.Z = Map.Get_Height_GL(coord);

	if (Map[coord].IsUnderBridge || Map[coord].WasUnderBridge) {
		coord.Z += BRIDGE_LEPTON_HEIGHT;
	}

	TacticalMap->Setup_Trigger_Scroll(coord, mission->Data.Speed);
	IsNextMission = true;
}


/// <summary>
/// Handles the reshroud map team mission.
/// This routine draws the shroud back over the entire map.
/// </summary>
void TeamClass::TMission_RESHROUD(TeamMissionClass * mission, bool)
{
	Map.Shroud_The_Map();
	IsNextMission = true;
}


/// <summary>
/// Handles the reveal map team mission.
/// This routine lifts the shroud from the entire map.
/// </summary>
void TeamClass::TMission_REVEAL(TeamMissionClass * mission, bool)
{
	Map.Reveal_The_Map();
	IsNextMission = true;
}


/// <summary>
/// Handles the fully loaded team mission.
/// This routine holds the script in place until every transport in the team is carrying as
/// many passengers as it can hold.
/// </summary>
void TeamClass::TMission_FULLY_LOADED(TeamMissionClass * mission, bool)
{
	FootClass *unit = Member;
	while (unit != NULL) {
		if (unit->TClass->Max_Passengers() > unit->Cargo.How_Many()) {
			return;
		}
		unit = unit->Member;
	}
	IsNextMission = true;
}


/// <summary>
/// Handles the unload truck team mission.
/// This routine swaps every loaded truck in the team for its empty counterpart, so that it
/// appears to have dropped off its cargo.
/// </summary>
void TeamClass::TMission_UNLOAD_TRUCK(TeamMissionClass * mission, bool)
{
	FootClass *unit = Member;
	while (unit != NULL) {
		if (unit->RTTI == RTTI_UNIT) {
			if (!strcmpi(unit->TClass->IniName, "TRUCKB")) {
				((UnitClass *)unit)->Class = UnitTypes[UnitTypeClass::From_Name("TRUCKA")];
			}
		}
		unit = unit->Member;
	}
	IsNextMission = true;
}


/// <summary>
/// Handles the load truck team mission.
/// This routine swaps every empty truck in the team for its loaded counterpart, so that the
/// cargo it is carrying becomes visible.
/// </summary>
void TeamClass::TMission_LOAD_TRUCK(TeamMissionClass * mission, bool)
{
	FootClass *unit = Member;
	while (unit != NULL) {
		if (unit->RTTI == RTTI_UNIT) {
			if (!strcmpi(unit->TClass->IniName, "TRUCKA")) {
				((UnitClass *)unit)->Class = UnitTypes[UnitTypeClass::From_Name("TRUCKB")];
			}
		}
		unit = unit->Member;
	}
	IsNextMission = true;
}


/// <summary>
/// Handles the attack building with property team mission.
/// This routine picks the enemy building that best matches the scripted property and then
/// turns the team loose on it.
/// </summary>
void TeamClass::TMission_ATTACK_BUILDING_WITH_PROPERTY(TeamMissionClass * mission, bool)
{
	if (MissionTarget == NULL) {
		FootClass *unit = Member;
		if (unit != NULL) {
			HouseClass * eptr = NULL;
			TargetPropertyType prop = TargetPropertyType((unsigned short)mission->Data.Prop);
			int type = mission->Data.Type;
			BuildingTypeClass *btype = BuildingTypes[type];

			if (unit->House->Enemy != HOUSE_NONE) {
				eptr = Houses[unit->House->Enemy];
			}

			BuildingClass * bptr = Pick_Building_With_Property(btype, eptr, unit, prop, Class->OnlyTargetHouseEnemy);
			if (bptr != NULL) {
				Assign_Mission_Target(bptr);
			}
			if (MissionTarget == NULL) {
				IsNextMission = true;
			}
		}
	}
	Coordinate_Attack();
}


/// <summary>
/// Handles the move to building with property team mission.
/// This routine picks the enemy building that best matches the scripted property and then
/// sends the team to a cell alongside it.
/// </summary>
void TeamClass::TMission_MOVETO_BUILDING_WITH_PROPERTY(TeamMissionClass * mission, bool)
{
	if (MissionTarget == NULL) {
		FootClass *unit = Member;
		if (unit != NULL) {
			HouseClass * eptr = NULL;
			TargetPropertyType prop = TargetPropertyType((unsigned short)mission->Data.Prop);
			int type = mission->Data.Type;
			BuildingTypeClass *btype = BuildingTypes[type];

			if (unit->House->Enemy != HOUSE_NONE) {
				eptr = Houses[unit->House->Enemy];
			}

			BuildingClass * bptr = Pick_Building_With_Property(btype, eptr, unit, prop, Class->OnlyTargetHouseEnemy);
			if (bptr != NULL) {
				MZoneType mzone = unit->TClass->MZone;
				Cell cell = bptr->PositionCoord.As_Cell();
				Cell newcell = Map.Nearby_Location(cell, unit->TClass->Speed, Map.Get_Cell_Zone(cell, mzone, unit->IsOnBridge), mzone, false, Point2D(3,3));
				if (newcell != CELL_NONE) {
					Assign_Mission_Target(&Map[newcell]);
				} else {
					Assign_Mission_Target(NULL);
				}
			}
			if (MissionTarget == NULL) {
				IsNextMission = true;
			}
		}
	}
	Coordinate_Move();
}


/// <summary>
/// Handles the scout team mission.
/// This routine sends the team toward a house the owner has not yet laid eyes on, and marks
/// that house as scouted once the trip is over. The mission only completes when there is
/// nobody left worth scouting.
/// </summary>
void TeamClass::TMission_SCOUT(TeamMissionClass * mission, bool)
{
	int i;

	bool should_continue = true;
	if (HouseToScout == NULL) {

		int scouted_count = 0;
		for (i = 0; i < House->ScoutNodes.Count(); i++) {
			if (!House->ScoutNodes[i].IsScouted) {
				scouted_count++;
			}
		}

		if (scouted_count > 0) {

			int picked = Random_Pick(0, scouted_count - 1);

			for (i = 0; i < House->ScoutNodes.Count(); i++) {
				if (!House->ScoutNodes[i].IsScouted) {
					if (picked == 0) {
						HouseToScout = House->ScoutNodes[i].House;
						break;
					} else {
						picked--;
					}
				}
			}

			DynamicVectorClass<BuildingClass *> targets;

			for (i = 0; i < Buildings.Count(); i++) {
				if (HouseToScout == Buildings[i]->House) {
					targets.Add(Buildings[i]);
				}
			}

			if (targets.Count() > 0) {

				int picked = Random_Pick(0, targets.Count() - 1);
				BuildingClass * target = targets[picked];

				FootClass * leader = Fetch_A_Leader();
				if (leader != NULL) {
					Cell cell = Map.Nearby_Location(target->PositionCoord.As_Cell(), leader->TClass->Speed);
					if (cell != CELL_NONE) {
						Assign_Mission_Target(&Map[cell]);
					} else {
						Assign_Mission_Target(NULL);
					}
				}
			} else {
				House->Mark_Scouted(HouseToScout);
				HouseToScout = NULL;
			}
		} else {
			should_continue = false;
		}
	} else {
		FootClass * leader = Fetch_A_Leader();
		if  (leader != NULL) {
			if (leader->CurrentMission != MISSION_MOVE && leader->MissionQueue != MISSION_MOVE) {
				House->Mark_Scouted(HouseToScout);
				HouseToScout = NULL;
			}
		}
	}

	Coordinate_Move();

	if (should_continue) {
		IsNextMission = false;
	} else {
		IsNextMission = true;
	}
}


/// <summary>
/// Finds the building of a type that best satisfies a targeting property.
/// This routine is used by the building-with-property team missions to decide what the team
/// should head for. A building belonging to the preferred house always wins over any other
/// legal candidate.
/// </summary>
/// <param name="type">The building type to search for.</param>
/// <param name="house">The house whose buildings are preferred.</param>
/// <param name="unit">The team member the search is being performed on behalf of.</param>
/// <param name="prop">The property that decides which candidate is the best one.</param>
/// <param name="only_enemy">Should the search fail rather than settle for a building outside
/// the preferred house?</param>
/// <returns>Returns with a pointer to the building picked. Otherwise, NULL is returned.</returns>
BuildingClass *Pick_Building_With_Property(BuildingTypeClass *type, HouseClass *house, FootClass *unit, TargetPropertyType prop, bool only_enemy)
{
	int best_same_dist = -1;
	BuildingClass *best_same_ptr = NULL;
	int best_dist = -1;
	BuildingClass *best_ptr = NULL;

	for (int index = 0; index < Buildings.Count(); index++) {

		BuildingClass *ptr = Buildings[index];

		HouseClass *hptr = ptr->House;

		bool same_house = hptr == house;

		if (ptr->Class == type && (same_house || !unit->House->Is_Ally(ptr->House))) {

			int dist = -1;

			switch (prop) {
				case TPROPERTY_LEAST_THREAT:
					dist = INT_MAX - Map.Cell_Threat(ptr->Center_Coord().As_Cell(), *unit->House);
					break;

				case TPROPERTY_GREATEST_THREAT:
					dist = Map.Cell_Threat(ptr->Center_Coord().As_Cell(), *unit->House);
					break;

				case TPROPERTY_NEAREST:
					dist = INT_MAX - ptr->Get_Coord().Distance_To(unit->Get_Coord());
					break;

				case TPROPERTY_FARTHEST:
					dist = ptr->Get_Coord().Distance_To(unit->Get_Coord());
					break;

			}

			if (dist > best_same_dist && same_house) {
				best_same_ptr = ptr;
				best_same_dist = dist;
			}
			if (dist > best_dist) {
				best_ptr = ptr;
				best_dist = dist;
			}
		}
	}

	if (best_same_ptr) {
		return(best_same_ptr);
	}

	if (!only_enemy) {
		return(best_ptr);
	}

	return(NULL);
}


/***********************************************************************************************
 * TeamClass::TMission_Unload -- Tells the team to unload passengers now.                      *
 *                                                                                             *
 *    This routine tells all transport vehicles to unload passengers now.                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/14/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamClass::TMission_UNLOAD(TeamMissionClass * mission, bool)
{
	FootClass * unit = Member;
	bool finished = true;

	while (unit != NULL) {

		FootClass *next = unit->Member;

		Coordinate_Conscript(unit);

		if (_Is_It_Playing(unit)) {
			/*
			**	Only assign the mission if the unit is carrying a passenger, OR
			**	if the unit is a minelayer, with mines in it, and the cell it's
			**	on doesn't have a building (read: mine) in it already.
			*/
			if (unit->Cargo.Is_Something_Attached()) {
				/*
				**	Passenger-carrying vehicles will always return false until
				**	they've unloaded all passengers.
				*/
				finished = false;

				/*
				**	The check for a building is located here because the mine layer may have
				**	already unloaded the mine but is still in the process of retracting
				**	the mine layer. During this time, it should not be considered to have
				**	finished its unload mission.
				*/
				if (Map[unit->Center_Coord()].Cell_Building() == NULL && unit->Mission != MISSION_UNLOAD) {
					unit->Assign_Destination(NULL);
					unit->Assign_Target(NULL);
					unit->Assign_Mission(MISSION_UNLOAD);
					finished = false;
				}

			}
		}

		unit = next;
	}

	if (finished) {
		unit = Member;
		int unload = mission->Data.Value;
		bool transportisaircraft = Has_Air_Transport();
		while (unit != NULL) {
			FootClass *next = unit->Member;

			if (unit->TClass->Max_Passengers() == 0 || (transportisaircraft && unit->RTTI != RTTI_AIRCRAFT)) {
				if (unload == 1 || unload == 3) {
					Remove(unit);
					unit->Assign_Target(NULL);
					unit->Assign_Destination(NULL);
				}
			} else if (unit->TClass->Max_Passengers() > 0) {
				if (Class->TransportsReturnOnUnload) {
					Remove(unit);
					unit->Assign_Target(NULL);
					unit->Assign_Destination(unit->ArchiveTarget);
					unit->Assign_Mission(MISSION_MOVE);
					if (unit->Ready_To_Commence()) {
						unit->Commence();
					}
					unit->ArchiveTarget = NULL;
				}
				else if (unload == 3 || unload == 2) {

					Remove(unit);
					unit->Assign_Target(NULL);
					unit->Assign_Destination(NULL);
				}
			}

			unit = next;
		}

		IsNextMission = true;
	}
}


/// <summary>
/// Handles the success team mission.
/// This routine marks the team as having succeeded, so that any trigger watching for team
/// success will spring.
/// </summary>
void TeamClass::TMission_SUCCESS(TeamMissionClass * mission, bool)
{
	IsNextMission = true;
	Succeeded = true;
}


/// <summary>
/// Does the team's task force call for an air transport?
/// This routine is used by the unload logic to tell whether the team's passengers are meant
/// to arrive by air rather than by ground transport.
/// </summary>
/// <returns>bool; Does the task force include a passenger carrying aircraft?</returns>
bool TeamClass::Has_Air_Transport(void) const
{
	for (int i = 0; i < Class->TaskForce->ClassCount; i++) {
		TechnoTypeClass const * techtype = Class->TaskForce->Members[i].Class;
		if (techtype->RTTI == RTTI_AIRCRAFTTYPE && techtype->MaxPassengers > 0) {
			return(true);
		}
	}
	return(false);
}


/// <summary>
/// Fetches the member types the team is still missing.
/// This routine builds the task force's full roster and then strikes off every type that is
/// already present, leaving what recruitment or production must still supply.
/// </summary>
/// <param name="list">The list to fill in with the outstanding member types.</param>
void TeamClass::Team_Members(TEAM_MEMBER_LIST & list)
{
	for (int i = 0; i < Class->TaskForce->ClassCount; i++) {
		TechnoTypeClass const * techtype = Class->TaskForce->Members[i].Class;
		int count = Class->TaskForce->Members[i].Quantity;
		while (count > 0) {
			list.Add(techtype);
			count--;
		}
	}

	FootClass *unit = Member;
	while (unit != NULL) {
		list.Delete(unit->TClass);
		unit = unit->Member;
	}
}


/// <summary>
/// Handles the flash team mission.
/// This routine flashes the members of the team for the scripted count.
/// </summary>
/// <param name="first_time">Is this the first time this mission has been processed?</param>
void TeamClass::TMission_FLASH(TeamMissionClass * mission, bool first_time)
{
	if (first_time) {
		Flash(mission->Data.Value);
		IsNextMission = true;
	}
}


/// <summary>
/// Flashes every member of the team.
/// This routine is used by the scripted flash mission to draw the player's attention to the
/// team.
/// </summary>
/// <param name="count">The number of times each member should flash.</param>
void TeamClass::Flash(int count)
{
	FootClass *unit = Member;
	while (unit != NULL) {
		unit->FlashCount = count;
		unit = unit->Member;
	}
}


/// <summary>
/// Handles the play animation team mission.
/// This routine attaches the scripted animation to every member of the team, so that it
/// travels along with them.
/// </summary>
/// <param name="first_time">Is this the first time this mission has been processed?</param>
void TeamClass::TMission_PLAY_ANIM(TeamMissionClass * mission, bool first_time)
{
	int type = (int)mission->Data.AType;
	if (first_time && type >= 0 && type < AnimTypes.Count()) {
		FootClass *unit = Member;
		int loops = (unsigned short)mission->Data.ALoops;
		AnimTypeClass *atype = AnimTypes[type];
		while (unit != NULL) {
			AnimClass * anim = new AnimClass(atype, unit->Center_Coord(), 0, loops);
			if (anim != NULL) {
				anim->Attach_To(unit);
			}
			unit = unit->Member;
		}
	}
	IsNextMission = true;

}


/// <summary>
/// Handles the talk bubble team mission.
/// This routine puts the scripted talk bubble over the team's lead member.
/// </summary>
/// <param name="first_time">Is this the first time this mission has been processed?</param>
void TeamClass::TMission_TALK_BUBBLE(TeamMissionClass * mission, bool first_time)
{
	if (first_time) {
		TechnoClass::Set_Talker(Member, (TalkType)mission->Data.Value);
		IsNextMission = true;
	}
}


/// <summary>
/// Does the team still have a member that is able to shoot?
/// This routine is used to decide whether the team must break off and rearm. A member that
/// never uses ammunition always counts as ready.
/// </summary>
/// <returns>bool; Is at least one member of the team still armed?</returns>
bool TeamClass::Ammo_Check(void) const
{
	bool r = false;

	FootClass *unit = Member;
	while (unit != NULL) {
		if (unit->TClass->MaxAmmo <= 0 || unit->Ammo > 0) {
			r = true;
			break;
		}
		unit = unit->Member;
	}
	return(r);
}


/// <summary>
/// Fetches the RTTI type of this object.
/// </summary>
/// <returns>Returns with the RTTI identifier that marks this object as a team.</returns>
RTTIType TeamClass::Fetch_RTTI(void) const
{
	return(RTTI_TEAM);
}
