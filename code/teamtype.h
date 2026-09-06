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

/* $Header: /CounterStrike/TEAMTYPE.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TEAMTYPE.H                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 07/02/96                                                     *
 *                                                                                             *
 *                  Last Update : July 2, 1996 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "abstype.h"
#include "ccini.h"
#include "typelist.h"
#include "types.h"

#include "dialog.hh"
#include "tmission.hh"

/*
**	Forward declarations.
*/
class TechnoTypeClass;
class TriggerTypeClass;
class TeamClass;
class HouseClass;
class ScriptTypeClass;
class TaskForceClass;
class TagTypeClass;
class TeamTypeClass;

typedef TypeList<TeamTypeClass const *> SUGGESTED_TEAM_LIST;

/*
**	TeamTypeClass declaration
*/
class TeamTypeClass : public AbstractTypeClass
{
		typedef AbstractTypeClass BASECLASS;

	public:
		/*
		**	Constructor/Destructor
		*/
		TeamTypeClass(char const * name = NULL);
		virtual ~TeamTypeClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		static TeamTypeClass * Find_Or_Make(char const * ininame = NULL);

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual void Compute_CRC(CRCEngine & crc) const override;
		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_TEAMTYPE);}
		virtual int Fetch_Heap_ID(void) const override {return(HeapID);};

		/*
		**	File I/O routines
		*/
		static void Read_All(CCINIClass const & ini, INIScopeType scope);
		static void Write_All(CCINIClass & ini, INIScopeType scope);
		virtual bool Read_INI(CCINIClass const & ini) override;
		virtual bool Write_INI(CCINIClass & ini) const override;
		/*
		**	Processing routines
		*/
		TeamClass * Create_One_Of(HouseClass *house=NULL) const;
		void Destroy_All_Of(void) const;
		void Detach(AbstractClass const * target, bool all=true);

		/*
		**	Utility routines
		*/
		static char const * Name_From_Mission(TeamMissionType order);
		static TeamMissionType Mission_From_Name(char const *name);
		static SUGGESTED_TEAM_LIST Suggested_New_Team(HouseClass * house, bool);
		static TeamTypeClass * From_Name(char const * name);
#ifdef _DEBUG
		char const * Member_Description(void) const;
		char const * Description(void) const;
		operator const char * (void) const {return(Description());};
#endif
		TeamClass *Find_First_Of_Type(void);

		int Get_Group(void) const;
		Cell Get_Origin(void) const;

		bool Can_Recruit(FootClass * foot, HouseClass * house) const;
		void Flash(int);

		/*
		 * This is this team type's index within the master TeamTypes list, assigned when the
		 * type is created. It is how a team type is named in a save game or a network packet.
		 */
		int HeapID;

		/*
		 * This is the player group number that members of this team are stamped with, and the
		 * group this team recruits from. If -1, the group of the team's task force is used
		 * instead; if -2, any group will do.
		 */
		int Group;

		/*
		 * This specifies the experience the crews of this team are created with -- zero for a
		 * dumb crew, one for ordinary, two for veteran and three for elite.
		 */
		int VeteranLevel;

		/*
		 * If the player may order infantry aboard the transports of this team, then this flag
		 * will be true. Otherwise the enter cursor is refused over any member of the team.
		 */
		bool IsLoadable;

		/*
		 * If the members of this team should arrive with their cargo holds full, then this
		 * flag will be true. Only a member with a storage capacity is affected.
		 */
		bool IsFull;

		/*
		 * If this team should regroup when one of its members is fired upon, then this flag
		 * will be true. The team gathers up and re-forms before pressing on with its mission.
		 */
		bool IsAnnoyance;

		/*
		 * If this team should arrange itself around the members that Is_Considered_Slow
		 * answers true for, then this flag will be true. Such a team weights its center
		 * toward those members and keeps a tighter formation around them.
		 */
		bool IsGuardSlower;

		/*
		 * If this team may recruit any suitable object regardless of the group it belongs to,
		 * then this flag will be true. Otherwise only its own group is considered.
		 */
		bool IsRecruiter;

		/*
		**	Is this team type allowed to be created automatically by the computer
		**	when the appropriate trigger indicates?
		*/
		bool IsAutocreate;

		/*
		**	This flag tells the computer that it should build members to fill
		**	a team of this type regardless of whether there actually is a team
		**	of this type active.
		*/
		bool IsPrebuilt;

		/*
		**	If this team should allow recruitment of new members, then this flag
		**	will be true. A false value results in a team that fights until it
		**	is dead. This is similar to IsSuicide, but they will defend themselves.
		*/
		bool IsReinforcable;

		/*
		 * If this team should raise the alarm when one of its members is hurt, then this flag
		 * will be true. The owning house reacts as though its base were under attack.
		 */
		bool IsWhiner;

		/*
		 * If a member of this team should keep firing at a target of opportunity rather than
		 * break off and follow the team to its mission target, then this flag will be true.
		 */
		bool IsAggressive;

		/// Unused
		bool IsLooseRecruit;

		/*
		**	If Suicide, the team won't stop until it achieves its mission or it's
		**	dead
		*/
		bool IsSuicide;

		/*
		 * If a team of this type should arrive by drop pod, then this flag will be true. Only
		 * a team whose task force is entirely infantry can actually make the trip.
		 */
		bool IsDroppod;

		/*
		 * If this team type was manufactured to deliver the player's dropship loadout rather
		 * than read from the scenario, then this flag will be true.
		 */
		bool IsDropship;

		/*
		 * If the team's Tag should be attached only to those members that can carry
		 * passengers, then this flag will be true. Otherwise every member gets the tag.
		 */
		bool OnTransOnly;

		/*
		**	Priority given the team for recruiting purposes; higher priority means
		**	it can steal members from other teams (scale: 0 - 15)
		*/
		int RecruitPriority;

		/*
		**	Max # of this type of team allowed at one time
		*/
		int MaxAllowed;

		/*
		**	Fear level of this team
		*/
		int Fear;

		/*
		**	House the team belongs to
		*/
		HouseClass * House;

		/// Unused
		int TechLevel;

		/*
		**	Trigger to assign to each object as it joins this team.
		*/
		TagTypeClass * Tag;

		/*
		**	This is the waypoint origin to use when creating this team or
		**	when bringing the team on as a reinforcement.
		*/
		WAYPOINT Origin;

		/*
		**	This records the number of teams of this type that are currently
		**	active.
		*/
		int Number;

		/*
		 * Pointer to the script that drives teams of this type -- the list of missions they
		 * work their way through once created.
		 */
		ScriptTypeClass * Script;

		/*
		 * Pointer to the task force that specifies which object types a team of this type is
		 * built from and how many of each it wants.
		 */
		TaskForceClass * TaskForce;

		/*
		 * This records whether this team type came from the scenario's INI or from the global
		 * rules, so that each one is written back out to the file it was read from.
		 */
		INIScopeType Scope;

		/*
		 * If the members of this team should steer around danger rather than through it, then
		 * this flag will be true. It overrides each member's own ThreatAvoidanceCoefficient.
		 */
		bool AvoidThreats;

		/*
		 * If the members of this team should shrug off the ion storm, then this flag will be
		 * true. It spares them both the storm's lightning strikes and its warhead damage.
		 */
		bool IsIonImmune;

		/*
		 * If the transports of this team should return to the cell they arrived at once their
		 * cargo is unloaded, then this flag will be true. The cell is kept in ArchiveTarget.
		 */
		bool TransportsReturnOnUnload;

		/*
		 * If the members of this team may be poached by autocreate teams forming later, then
		 * this flag will be true. It is stamped onto each object as it joins the team.
		 */
		bool AreTeamMembersRecruitable;

		/*
		 * If this team exists to defend its owner's base, then this flag will be true. Such
		 * teams count against the house's defensive team cap, and their members stay
		 * available to answer a call for help from elsewhere in the base.
		 */
		bool IsBaseDefense;

		/*
		 * If this team should only ever target the objects of its owner's designated
		 * enemy house, then this flag will be true. Otherwise anything hostile is fair game.
		 */
		bool OnlyTargetHouseEnemy;
};
