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

/* $Header: /CounterStrike/TEAM.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TEAM.H                                                       *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 12/11/94                                                     *
 *                                                                                             *
 *                  Last Update : December 11, 1994 [JLB]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "abstract.h"
#include "ftimer.h"
#include "timer.h"
#include "tmission.h"
#include "vector.h"

#include "result.hh"

class TeamTypeClass;
class ScriptClass;
class TechnoClass;
class TeamTypeClass;
class HouseClass;
class FootClass;
class TagClass;
class TechnoTypeClass;
class MonoClass;

typedef DynamicVectorClass<TechnoTypeClass const *> TEAM_MEMBER_LIST;


/*
**	Units are only allowed to stray a certain distance away from their
**	team.  When they exceed this distance, some sort of fixup must be
**	done.
*/
#define STRAY_DISTANCE		2

class TeamClass : public AbstractClass
{
		typedef AbstractClass BASECLASS;

	public:
		/*
		**	This specifies the type of team this is.
		*/
		TeamTypeClass * Class;

		/*
		 * This points to the team's working copy of the mission list it follows, built from
		 * the script specified by its type. The team steps through it one mission at a time
		 * and disbands when the missions run out.
		 */
		ScriptClass * Script;

		/*
		**	This specifies the owner of this team.
		*/
		HouseClass * House;

		/*
		 * If the team is on a scouting mission, then this points to the house whose base it
		 * has been sent to look at. It is cleared, and the house marked as scouted, once the
		 * team's leader completes the trip, so the mission can pick the next unscouted house.
		 */
		HouseClass * HouseToScout;

		/*
		**	A team will have a center point. This is the point used to determine if
		**	any member of the team is "too far" from the team and must return. This
		**	center point is usually calculated as the average position of all the
		**	team members.
		*/
		AbstractClass * Zone;

		/*
		**	This is the target value of the team member that is closest to the
		**	destination of the team. The implied location serves as the
		**	regroup point for the unit as it is moving.
		*/
		AbstractClass * ClosestMember;

		/*
		**	This is the target of the team. Typically, it is a unit or structure, but
		**	for the case of teams with a movement mission, it might represent a
		**	destination cell.
		*/
		AbstractClass * MissionTarget;
		AbstractClass * Target;

		/// Unused
		void * UnusedPtr1;

		/*
		**	This is the total number of members in this team.
		*/
		int Total;

		/*
		**	This is the teams combined risk value
		*/
		int Risk;

		/*
		 * This is the game frame that the team was created on. A team that never fills up is
		 * dissolved once DissolveUnfilledTeamDelay frames have elapsed, and when a house is
		 * over its team cap it is the oldest base defense team that gets disbanded.
		 */
		int CreationFrame;

		/*
		**	Points to the first member in the list of members for this team.
		*/
		FootClass * Member;

		/*
		**	Some missions will time out. This is the timer that keeps track of the
		**	time to transition between missions.
		*/
		CDTimerClass<FrameTimerClass> TimeOut;

		/*
		**	This is the amount of time the team is suspended for.
		*/
		CDTimerClass<FrameTimerClass> SuspendTimer;

		/*
		 * If the team's type specifies a tag to be shared by the whole team, then this
		 * points to the instance of it created for this team. Members are attached to the
		 * tag as they are recruited, so that a trigger can watch the team as a whole.
		 */
		TagClass * Tag;

		/*
		 * If this team owns its own type -- as it does when the reinforcement logic
		 * manufactures a throwaway team type for a dropship loadout -- then this flag will
		 * be true and the type is deleted along with the team.
		 */
		bool IsDeleteTypeWhenDone;

		/*
		 * If the team has been shaken out of its mission and told to regroup, then this flag
		 * will be true. It is cleared once every member is back at the team's center point.
		 */
		bool IsRegrouping;

		/*
		 * For a team whose type has IsGuardSlower set, this records whether the team is
		 * above its under strength threshold, and is cleared once the last member is gone.
		 */
		bool NeedsReinforcement;

		/*
		**	This flag forces the team into active state regardless of whether it
		**	is understrength or not.
		*/
		bool IsForcedActive;

		/*
		**	This flag is set to true when the team initiates into active mode. The
		**	flag is never cleared. By examining this flag, it is possible to determine
		**	if the team has ever launched into active mode.
		*/
		bool IsHasBeen;

		/*
		**	If the team is full strength, then this flag is true. A full strength
		**	team will not try to recruit members.
		*/
		bool IsFullStrength;

		/*
		**	A team that is below half strength has this flag true. It means that the
		**	the team should hide back at the owner's base and try to recruit
		**	members.
		*/
		bool IsUnderStrength;

		/*
		**	If a team is not understrength but is not yet full strength, then
		**	the team is regrouping.  If this flag is set and the team becomes
		**	full strength, the all members of the team will become initiated
		**	and this flag will be reset.
		*/
		bool IsReforming;

		/*
		**	This bit should be set if a team is determined to have lagging
		**	units in its formation.
		*/
		bool IsLagging;

		/*
		**	If a team member was removed or added, then this flag will be set to true. The
		**	team system uses this flag to tell whether it should recalculate the team
		**	under strength or full strength flags. This process does not need to occur
		**	EVERY time a unit added or deleted from a team, just every so often if the
		**	team has been changed.
		*/
		bool IsAltered;
		bool JustAltered;

		/*
		**	If the team is working on it's primary mission (it is past the build up stage)
		**	then this flag will be true. The transition between "moving" and "stationary"
		**	stages usually requires some action on the team's part.
		*/
		bool IsMoving;

		/*
		**	When the team determines that the next mission should be advanced to, it will
		**	set this flag to true. Mission advance will either change the behavior of the
		**	team or cause it to disband.
		*/
		bool IsNextMission;

		/*
		**	If at least one member of this team successfully left the map, then this
		**	flag will be set. At the time the team is terminated, if this flag is true, then
		**	if there are any triggers that depend upon this team leaving, they will be
		**	sprung.
		*/
		bool IsLeaveMap;

		/*
		**	Records whether the team is suspended from production.
		*/
		bool Suspended;

		/*
		 * If the team got as far as its success mission, then this flag will be true. When
		 * the team is destroyed, the AI trigger type that spawned it records a success or a
		 * failure accordingly, which is how the AI learns which teams are worth repeating.
		 */
		bool Succeeded;

		//------------------------------------------------------------
		/*
		 * Constructors, destructors, and persistence.
		 */
		TeamClass(TeamTypeClass const * team=0, HouseClass * owner=0, void * = NULL);
		virtual ~TeamClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine & crc) const override;

		/*
		 * Membership.
		 */
		bool Add(FootClass *);
		bool Remove(FootClass *, int typeindex=-1);
		bool Can_Add(FootClass * obj, int & typeindex) const;
		bool Is_Empty(void) const {return(Member == NULL);}
		FootClass * Get_Member(void) {return(Member);}
		void Team_Members(TEAM_MEMBER_LIST & list);
		bool Recalc_Strength(void);

		/*
		 * Per frame processing and teardown.
		 */
		void AI(void);
		void Detach(AbstractClass const * target, bool all);
		void Took_Damage(FootClass * obj, ResultType result, TechnoClass * source);

		/*
		 * Team state and orders.
		 */
		void Force_Active(void) {IsForcedActive = true;IsUnderStrength=false;};
		void Assign_Mission_Target(AbstractClass * new_target);
		void Flag_Into_Action(void);
		void Regroup(void);
		void Scan_Limit(void);
		void Flash(int count);
		bool Has_Entered_Map(void) const;
		bool Is_Leaving_Map(void) const;
		bool Has_Air_Transport(void) const;

		static void Suspend_Teams(int priority, HouseClass const * house);
#ifdef _DEBUG
		void Debug_Dump(MonoClass * mono) const;
#endif

	private:

		/*
		 * The team mission handlers, one for each TeamMissionType. They are declared in
		 * the order the enumeration gives them, and the commented out entries mark the
		 * missions that carry no handler of their own.
		 */
		void TMission_ATTACK(TeamMissionClass * mission, bool);
		void TMission_ATT_WAYPT(TeamMissionClass * mission, bool);
//		void TMission_FORMATION(TeamMissionClass * mission, bool);
		void TMission_BERZERK(TeamMissionClass * mission, bool);
		void TMission_MOVE(TeamMissionClass * mission, bool);
		void TMission_MOVECELL(TeamMissionClass * mission, bool);
		void TMission_GUARD(TeamMissionClass * mission, bool);
		void TMission_LOOP(TeamMissionClass * mission, bool);
//		void TMission_ATTACKTARCOM(TeamMissionClass * mission, bool);
		void TMission_WIN(TeamMissionClass * mission, bool);
		void TMission_UNLOAD(TeamMissionClass * mission, bool);
		void TMission_DEPLOY(TeamMissionClass * mission, bool);
		void TMission_HOUND_DOG(TeamMissionClass * mission, bool);
		void TMission_DO(TeamMissionClass * mission, bool);
		void TMission_SET_GLOBAL(TeamMissionClass * mission, bool);
//		void TMission_INVULNERABLE(TeamMissionClass * mission, bool);
		void TMission_IDLE_ANIM(TeamMissionClass * mission, bool);
		void TMission_LOAD(TeamMissionClass * mission, bool);
		void TMission_SPY(TeamMissionClass * mission, bool);
		void TMission_PATROL(TeamMissionClass * mission, bool);
		void TMission_SCRIPT(TeamMissionClass * mission, bool);
		void TMission_TEAMCHANGE(TeamMissionClass * mission, bool);
		void TMission_PANIC(TeamMissionClass * mission, bool);
		void TMission_CHANGE_HOUSE(TeamMissionClass * mission, bool);
		void TMission_SCATTER(TeamMissionClass * mission, bool);
		void TMission_GOTO_SHROUD(TeamMissionClass * mission, bool);
		void TMission_LOSE(TeamMissionClass * mission, bool);
		void TMission_PLAY_SPEECH(TeamMissionClass * mission, bool);
		void TMission_PLAY_SOUND(TeamMissionClass * mission, bool);
		void TMission_PLAY_MOVIE(TeamMissionClass * mission, bool);
		void TMission_PLAY_MUSIC(TeamMissionClass * mission, bool);
		void TMission_REDUCE_TIBERIUM(TeamMissionClass * mission, bool);
		void TMission_BEGIN_PRODUCTION(TeamMissionClass * mission, bool);
		void TMission_FIRE_SALE(TeamMissionClass * mission, bool);
		void TMission_SELF_DESTRUCT(TeamMissionClass * mission, bool);
		void TMission_ION_STORM_START(TeamMissionClass * mission, bool);
		void TMission_ION_STORM_END(TeamMissionClass * mission, bool);
		void TMission_CENTER_VIEWPOINT(TeamMissionClass * mission, bool);
		void TMission_RESHROUD(TeamMissionClass * mission, bool);
		void TMission_REVEAL(TeamMissionClass * mission, bool);
		void TMission_DESTROY_MEMBERS(TeamMissionClass * mission, bool);
		void TMission_CLEAR_GLOBAL(TeamMissionClass * mission, bool);
		void TMission_SET_LOCAL(TeamMissionClass * mission, bool);
		void TMission_CLEAR_LOCAL(TeamMissionClass * mission, bool);
		void TMission_UNPANIC(TeamMissionClass * mission, bool);
		void TMission_FORCE_FACING(TeamMissionClass * mission, bool);
		void TMission_FULLY_LOADED(TeamMissionClass * mission, bool);
		void TMission_UNLOAD_TRUCK(TeamMissionClass * mission, bool);
		void TMission_LOAD_TRUCK(TeamMissionClass * mission, bool);
		void TMission_ATTACK_BUILDING_WITH_PROPERTY(TeamMissionClass * mission, bool);
		void TMission_MOVETO_BUILDING_WITH_PROPERTY(TeamMissionClass * mission, bool);
		void TMission_SCOUT(TeamMissionClass * mission, bool);
		void TMission_SUCCESS(TeamMissionClass * mission, bool);
		void TMission_FLASH(TeamMissionClass * mission, bool);
		void TMission_PLAY_ANIM(TeamMissionClass * mission, bool);
		void TMission_TALK_BUBBLE(TeamMissionClass * mission, bool);

		/*
		 * Keeping the members moving and fighting together.
		 */
		int Recruit(int typeindex);
		bool Is_A_Member(void const * who) const;
		bool Coordinate_Regroup(void);
		void Coordinate_Attack(void);
		void Coordinate_Move(void);
		bool Coordinate_Conscript(FootClass * unit);
		void Calc_Center(AbstractClass *& center, AbstractClass *& obj_center) const;
		bool Lagging_Units(void);
		FootClass * Fetch_A_Leader(void) const;
		bool Ammo_Check(void) const;

	public:
		int Quantity[MAX_TEAM_CLASSCOUNT];
};
