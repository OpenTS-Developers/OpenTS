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

/* $Header: /CounterStrike/TEAMTYPE.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TEAMTYPE.CPP                                                 *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 06/28/96                                                     *
 *                                                                                             *
 *                  Last Update : July 30, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   TeamMissionClass::Description -- Compose a text description of team mi                    *
 *   TeamMissionClass::Draw_It -- Draws a team mission list box entry.                         *
 *   TeamMission_Needs -- Determines what extra data is needed by team miss                    *
 *   TeamTypeClass::As_Pointer -- gets ptr for team type with given name                       *
 *   TeamTypeClass::Build_INI_Entry -- Builds the INI entry for this team type.                *
 *   TeamTypeClass::Create_One_Of -- Creates a team of this type.                              *
 *   TeamTypeClass::Description -- Builds a description of the team.                           *
 *   TeamTypeClass::Destroy_All_Of -- Destroy all teams of this type.                          *
 *   TeamTypeClass::Detach -- Detach the specified target from this team type.                 *
 *   TeamTypeClass::Draw_It -- Display the team type in a list box.                            *
 *   TeamTypeClass::Edit -- Edit the team type.                                                *
 *   TeamTypeClass::Fill_In -- fills in trigger from the given INI entry                       *
 *   TeamTypeClass::From_Name -- Converts a name into a team type pointer.                     *
 *   TeamTypeClass::Init -- pre-scenario initialization                                        *
 *   TeamTypeClass::Member_Description -- Builds a member description string                   *
 *   TeamTypeClass::Mission_From_Name -- returns mission for given name                        *
 *   TeamTypeClass::Name_From_Mission -- returns name for given mission                        *
 *   TeamTypeClass::Read_INI -- reads INI data                                                 *
 *   TeamTypeClass::Suggested_New_Team -- Suggests a new team to create.                       *
 *   TeamTypeClass::TeamTypeClass -- class constructor                                         *
 *   TeamTypeClass::Write_INI -- Write out the team types to the INI database.                 *
 *   TeamTypeClass::operator delete -- 'delete' operator                                       *
 *   TeamTypeClass::operator new -- 'new' operator                                             *
 *   TeamTypeClass::~TeamTypeClass -- class destructor                                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "teamtype.h"

#include "_rules.h"
#include "aitrig.h"
#include "ccini.h"
#include "ccrand.h"
#include "dbgprint.h"
#include "ddist.h"
#include "findmake.h"
#include "foot.h"
#include "globals.h"
#include "house.h"
#include "houstype.h"
#include "incdec.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "script.h"
#include "session.h"
#include "sun.h"
#include "swizzle.h"
#include "tagtype.h"
#include "taskforc.h"
#include "team.h"
#include "tracker.h"
#include "vector.h"
#include "waypoint.h"



/// <summary>
/// Converts an ASCII hexadecimal string into an integer.
/// Conversion stops at the first character that is not a hexadecimal digit, so trailing
/// text is harmless.
/// </summary>
/// <param name="string">The hexadecimal string to convert.</param>
/// <returns>Returns with the integer value of the leading hexadecimal digits.</returns>
int _ahtoi(const char *string)
{
	char *s = (char *)string;
	int integer = 0;
	while (isxdigit((unsigned char)s[0]))
	{
		integer *= 16; /// desired base is 16

		if (isdigit((unsigned char)s[0])) {
			integer += s[0] - '0';
		} else {
			integer += toupper((unsigned char)s[0]) - ('A' - 10);
		}
		s++;
	}

	return(integer);
}


/***************************************************************************
 * TeamTypeClass::TeamTypeClass -- class constructor                       *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/07/1994 BR : Created.                                              *
 *   11/22/1995 JLB : Uses initializer constructor method.                 *
 *=========================================================================*/
TeamTypeClass::TeamTypeClass(char const * name) :
	BASECLASS(name),
	HeapID(-1),
	Group(-1),
	VeteranLevel(1),
	IsLoadable(false),
	IsFull(false),
	IsAnnoyance(false),
	IsGuardSlower(false),
	IsRecruiter(false),
	IsAutocreate(false),
	IsPrebuilt(false),
	IsReinforcable(false),
	IsWhiner(false),
	IsAggressive(false),
	IsLooseRecruit(false),
	IsSuicide(false),
	IsDroppod(false),
	IsDropship(false),
	OnTransOnly(false),
	RecruitPriority(7),
	MaxAllowed(0),
	Fear(0),
	House(NULL),
	TechLevel(0),
	Tag(NULL),
	Origin(-1),
	Number(0),
	Script(NULL),
	TaskForce(NULL),
	Scope(SCOPE_LOCAL),
	AvoidThreats(false),
	IsIonImmune(false),
	TransportsReturnOnUnload(false),
	AreTeamMembersRecruitable(true),
	IsBaseDefense(false),
	OnlyTargetHouseEnemy(false)
{
	TeamTypes.Add(this);

	HeapID = TeamTypes.ID(this);
}


/// <summary>
/// Destroys this team type.
/// Anything still pointing at this team type is detached from it first, and the team type
/// then removes itself from the master team type list.
/// </summary>
TeamTypeClass::~TeamTypeClass(void)
{
	Detach_This_From_All(this, true);
	TeamTypes.Delete(this);
}


#if _DEBUG
/***************************************************************************
 * TeamTypeClass::Mission_From_Name -- returns team mission for given name *
 *                                                                         *
 * INPUT:                                                                  *
 *      name         name to compare                                       *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      mission for that name                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/13/1994 BR : Created.                                              *
 *=========================================================================*/
TeamMissionType TeamTypeClass::Mission_From_Name(char const * name)
{
	if (name) {
		for (TeamMissionType order = TMISSION_FIRST; order < TMISSION_COUNT; order++) {
			if (stricmp(TMissions[order], name) == 0) {
				return(order);
			}
		}
	}

	return(TMISSION_NONE);
}


/***************************************************************************
 * TeamTypeClass::Name_From_Mission -- returns name for given mission      *
 *                                                                         *
 * INPUT:                                                                  *
 *      order      mission to get name for                                 *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      name of mission                                                    *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/13/1994 BR : Created.                                              *
 *=========================================================================*/
char const * TeamTypeClass::Name_From_Mission(TeamMissionType order)
{
	assert((unsigned)order < TMISSION_COUNT);

	return(TMissions[order]);
}
#endif


/***********************************************************************************************
 * TeamTypeClass::Create_One_Of -- Creates a team of this type.                                *
 *                                                                                             *
 *    Use this routine to create a team object from this team type.                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the newly created team object. If one could not be       *
 *          created, then NULL is returned.                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
TeamClass * TeamTypeClass::Create_One_Of(HouseClass * house) const
{
	DebugString("Create_One_Of...\n");
	if (house == NULL) {
		house = House;
	}
	// A type owned by nobody, such as a start position nobody holds, raises no team.
	if (house == NULL) {
		return(NULL);
	}
	if (ScenarioInit || MaxAllowed < 0 ||
		((Session.Type == GAME_NORMAL && Number < MaxAllowed) ||
		 (Session.Type != GAME_NORMAL && house->Owned_Team_Count(this) < MaxAllowed))) {

		DebugString("Creating a new team named '%s'.\n", (char const *)GivenName);
		return(new TeamClass(this, house));
	}
	return(NULL);
}


/***********************************************************************************************
 * TeamTypeClass::Destroy_All_Of -- Destroy all teams of this type.                            *
 *                                                                                             *
 *    This routine will destroy all teams of this type. Typical use of this is from a trigger  *
 *    event.                                                                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamTypeClass::Destroy_All_Of(void) const
{
	for (int index = 0; index < Teams.Count(); index++) {
		TeamClass * team = Teams[index];

		if (this == team->Class) {
			delete team;
			index--;
		}
	}
}


/***********************************************************************************************
 * TeamTypeClass::Suggested_New_Team -- Suggests a new team to create.                         *
 *                                                                                             *
 *    This routine will scan through the team types available and create teams of the          *
 *    type that can best utilize the existing unit mix.                                        *
 *                                                                                             *
 * INPUT:   house    -- Pointer to the house that this team is to be created for.              *
 *                                                                                             *
 *          atypes   -- A bit mask of the aircraft types available for this house.             *
 *                                                                                             *
 *          utypes   -- A bit mask of the unit types available for this house.                 *
 *                                                                                             *
 *          itypes   -- A bit mask of the infantry types available for this house.             *
 *                                                                                             *
 *          vtypes   -- A bit mask of the vessel types available for this house.               *
 *                                                                                             *
 *          alerted  -- Is this house alerted? If true, then the Autocreate teams will be      *
 *                      considered in the selection process.                                   *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the team type that should be created. If no team should  *
 *          be created, then it returns NULL.                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/13/1995 JLB : Created.                                                                 *
 *   07/21/1995 JLB : Will autocreate team even if no members in field.                        *
 *=============================================================================================*/
SUGGESTED_TEAM_LIST TeamTypeClass::Suggested_New_Team(HouseClass * house, bool)
{
	int i;
	bool skip_base_defense;

	SUGGESTED_TEAM_LIST best;
	HouseClass *enemy = NULL;

	if (house->Enemy != HOUSE_NONE) {
		enemy = Houses[house->Enemy];
	}
	if (Random_Pick(1, 100) <= house->RatioAITriggerTeam && house->IsAITriggersOn) {
		int team_count = 0;
		int defense_team_count = 0;
		for (i = 0; i < Teams.Count(); i++) {
			if (Teams[i]->House == house) {
				team_count++;
				if (Teams[i]->Class->IsBaseDefense) {
					defense_team_count++;
				}
			}
		}

		skip_base_defense = false;
		if (team_count < Rule->TotalAITeamCap[house->Difficulty] || defense_team_count < team_count / 2) {
			if (defense_team_count > Rule->MaximumAIDefensiveTeams[house->Difficulty]) {
				skip_base_defense = true;
			}
		} else {
			TeamClass *team = NULL;
			int frame = 0x7FFFFFFF;
			for (i = 0; i < Teams.Count(); i++) {
				TeamClass *t = Teams[i];
				if (t->House == house) {
					if (t->Class->IsBaseDefense) {
						if (t->CreationFrame < frame) {
							team = t;
							frame = t->CreationFrame;
						}
					}
				}
			}
			if (team != NULL) {
				team_count--;
				skip_base_defense = true;
				delete team;
			}
		}

		if (team_count < Rule->TotalAITeamCap[house->Difficulty]) {
			DiscreteDistributionClass<AITriggerTypeClass> trigdist;
			for (i = 0; i < AITriggerTypes.Count(); i++) {
				if (AITriggerTypes[i] != NULL) {
					AITriggerTypeClass *trig = AITriggerTypes[i];
					if (trig->Process(house, enemy, skip_base_defense) == true) {
						trigdist.Add(trig, trig->Get_Current_Weight());
					}
				}
			}
			AITriggerTypeClass *trig = trigdist.Sample();
			if (trig != NULL) {
				if (trig->Get_First_TeamType() != NULL) {
					best.Add(trig->Get_First_TeamType());
				}
			}
			if (trig != NULL) {
				if (trig->Get_Second_TeamType() != NULL) {
					best.Add(trig->Get_Second_TeamType());
				}
			}
		}
	}

	for (i = 0; i < Teams.Count(); i++) {
		if (Teams[i]->House == house && (Teams[i]->IsReforming || !Teams[i]->IsMoving)) {
			int j = 0;
			if (j < best.Count()) {
				TeamTypeClass *cls = Teams[i]->Class;
				while (true) {
					if (cls == best[j]) {
						best.Clear();
						return(best);
					}
					j++;
					if (j >= best.Count()) {
						break;
					}
				}
			}
		}
	}

	for (i = 0; i < best.Count(); i++) {
		TeamTypeClass *ttype = (TeamTypeClass *)best[i];
		ttype->IsAutocreate = true;
	}

	return(best);
}


/***********************************************************************************************
 * TeamTypeClass::From_Name -- Converts a name into a team type pointer.                       *
 *                                                                                             *
 *    This routine is used to convert an ASCII name of a team type into the corresponding      *
 *    team type pointer.                                                                       *
 *                                                                                             *
 * INPUT:   name  -- Pointer to the ASCII name of the team type.                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the team type that this ASCII name represents. If there  *
 *          is no match, the NULL is returned.                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/26/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
TeamTypeClass * TeamTypeClass::From_Name(char const * name)
{
	if (name) {
		for (int index = 0; index < TeamTypes.Count(); index++) {
			if (stricmp(name, TeamTypes[index]->IniName) == 0 || stricmp(name, TeamTypes[index]->GivenName) == 0) {
				return(TeamTypes[index]);
			}
		}
	}
	return(0);
}



#ifdef _DEBUG
/***********************************************************************************************
 * TeamTypeClass::Member_Description -- Builds a member description string.                    *
 *                                                                                             *
 *    This routine will build a team member description string. The string will be composed    *
 *    of the team member type and quantity. As many team member types will be listed that      *
 *    can fit within a reasonable size.                                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the text string that contains a description of the team  *
 *          type members.                                                                      *
 *                                                                                             *
 * WARNINGS:   The return string may be truncated if necessary to fit within reasonable size   *
 *             limits.                                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
char const * TeamTypeClass::Member_Description(void) const
{
	static char buffer[128];

	buffer[0] = '\0';

	if (TaskForce == NULL) return(buffer);

	/*
	**	Fill in class & count for all classes
	*/
	for (int index = 0; index < TaskForce->ClassCount; index++) {
		char txt[10];

		strcat(buffer, TaskForce->Members[index].Class->IniName);
		strcat(buffer, ":");

		sprintf(txt, "%d", TaskForce->Members[index].Quantity);
		strcat(buffer, txt);

		if (index < TaskForce->ClassCount-1) {
			strcat(buffer, ",");
		}
	}

	if (strlen(buffer) > 25) {
		strcpy(&buffer[25-3], "...");
	}

	return(buffer);
}


/***********************************************************************************************
 * TeamTypeClass::Description -- Builds a description of the team.                             *
 *                                                                                             *
 *    This routine will build a brief description of the team type. This description is used   *
 *    in the team type list.                                                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the composed text string that represents the team type.               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
char const * TeamTypeClass::Description(void) const
{
	static char _buffer[128];
	char extra = ' ';
	char loc[3];

	loc[0] = loc[1] = loc[2] = 0;
	if (IsAutocreate) extra = '*';
	if (Origin > -1) {
//	if (Origin != -1) {
		if (Origin < 26) {
			loc[0] = 'A' + Origin;
		} else {
			loc[0] = Origin / 26 + 'A'-1;
			loc[1] = Origin % 26 + 'A';
		}
	}

	sprintf(_buffer, "%s\t%s\t%c%s\t%d\t%s", (char const *)IniName, House->Class->Suffix, extra, loc, Script ? Script->MissionCount : 0, Member_Description());
	return(_buffer);
}
#endif


/***********************************************************************************************
 * TeamTypeClass::Detach -- Detach the specified target from this team type.                   *
 *                                                                                             *
 *    This routine is called when some object is about to be removed from the game system and  *
 *    all references to it must be severed. This will check to see if the specified object     *
 *    is a trigger that this team refers to. If so, then the reference will be cleared.        *
 *                                                                                             *
 * INPUT:   target   -- The target object to remove references to.                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TeamTypeClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);

	if (Tag == target) {
		Tag = NULL;
	}
	if (House == target) {
		House = NULL;
	}
	if (TaskForce == target) {
		TaskForce = NULL;
	}
	if (Script == target) {
		Script = NULL;
	}
}


/***************************************************************************
 * TeamTypeClass::Read_INI -- reads INI data                               *
 *                                                                         *
 * INI entry format:                                                       *
 *      TeamName = Housename,Roundabout,Learning,Suicide,Spy,Mercenary,    *
 *       RecruitPriority,MaxAllowed,InitNum,Fear,                          *
 *       ClassCount,Class:Num,Class:Num,...,                               *
 *       MissionCount,Mission:Arg,Mission:Arg,Mission:Arg,...              *
 *                                                                         *
 * INPUT:                                                                  *
 *      buffer      buffer to hold the INI data                            *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/07/1994 BR : Created.                                              *
 *   02/01/1995 BR : No del team if no classes (editor needs empty teams!) *
 *=========================================================================*/
bool TeamTypeClass::Read_INI(CCINIClass const & ini)
{
	if (BASECLASS::Read_INI(ini)) {
		HousesType house = HOUSE_NONE;
		if (House != NULL) {
			house = House->Class->House;
		}
		house = ini.Get_HousesType(IniName, "House", house);
		if (house != HOUSE_NONE) {
			House = House_From_HousesType(house);
		}

		VeteranLevel = ini.Get_Int(IniName, "VeteranLevel", VeteranLevel);
		IsLoadable = ini.Get_Bool(IniName, "Loadable", IsLoadable);
		IsFull = ini.Get_Bool(IniName, "Full", IsFull);
		IsAnnoyance = ini.Get_Bool(IniName, "Annoyance", IsAnnoyance);
		IsGuardSlower = ini.Get_Bool(IniName, "GuardSlower", IsGuardSlower);
		IsAutocreate = ini.Get_Bool(IniName, "Autocreate", IsAutocreate);
		IsPrebuilt = ini.Get_Bool(IniName, "Prebuild", IsPrebuilt);
		IsReinforcable = ini.Get_Bool(IniName, "Reinforce", IsReinforcable);
		IsDroppod = ini.Get_Bool(IniName, "Droppod", IsDroppod);
		IsRecruiter = ini.Get_Bool(IniName, "Recruiter", IsRecruiter);
		IsWhiner = ini.Get_Bool(IniName, "Whiner", IsWhiner);
		IsSuicide = ini.Get_Bool(IniName, "Suicide", IsSuicide);
		IsLooseRecruit = ini.Get_Bool(IniName, "LooseRecruit", IsLooseRecruit);
		IsAggressive = ini.Get_Bool(IniName, "Aggressive", IsAggressive);
		OnTransOnly = ini.Get_Bool(IniName, "OnTransOnly", OnTransOnly);
		RecruitPriority = ini.Get_Int(IniName, "Priority", RecruitPriority);
		MaxAllowed = ini.Get_Int(IniName, "Max", MaxAllowed);
		TechLevel = ini.Get_Int(IniName, "TechLevel", TechLevel);

		/*
		**	Fetch the trigger ID.
		*/
		Tag = TGet_Class(ini, IniName, "Tag", Tag);

		Group = ini.Get_Int(IniName, "Group", Group);
		AvoidThreats = ini.Get_Bool(IniName, "AvoidThreats", AvoidThreats);
		IsIonImmune = ini.Get_Bool(IniName, "IonImmune", IsIonImmune);
		IsBaseDefense = ini.Get_Bool(IniName, "IsBaseDefense", IsBaseDefense);
		OnlyTargetHouseEnemy = ini.Get_Bool(IniName, "OnlyTargetHouseEnemy", OnlyTargetHouseEnemy);
		TransportsReturnOnUnload = ini.Get_Bool(IniName, "TransportsReturnOnUnload", TransportsReturnOnUnload);
		AreTeamMembersRecruitable = ini.Get_Bool(IniName, "AreTeamMembersRecruitable", AreTeamMembersRecruitable);

		char waypt[64];
		if (ini.Get_String(IniName, "Waypoint", Waypoint_To_Name(Origin), waypt, sizeof(waypt)) > 0) {
			Origin = Waypoint_From_Name(waypt);
		}

		Script = TGet_Class(ini, IniName, "Script", Script);
		TaskForce = TGet_Class(ini, IniName, "TaskForce", TaskForce);

		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * TeamTypeClass::Write_INI -- Write out the team types to the INI database.                   *
 *                                                                                             *
 *    This routine will take all team types and write them out to the INI database specified.  *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database that will hold al the teams.                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   All preexisting team data in the database will be erased by this routine.       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TeamTypeClass::Write_INI(CCINIClass & ini) const
{
	if (BASECLASS::Write_INI(ini)) {
		HousesType house = HOUSE_NONE;
		if (House != NULL) {
			house = House->Class->House;
		}

		/*
		**	Output the general data for this team type.
		*/
		ini.Put_Int(IniName, "VeteranLevel", VeteranLevel);
		ini.Put_Bool(IniName, "Loadable", IsLoadable);
		ini.Put_Bool(IniName, "Full", IsFull);
		ini.Put_Bool(IniName, "Annoyance", IsAnnoyance);
		ini.Put_Bool(IniName, "GuardSlower", IsGuardSlower);
		ini.Put_HousesType(IniName, "House", house);
		ini.Put_Bool(IniName, "Recruiter", IsRecruiter);
		ini.Put_Bool(IniName, "Autocreate", IsAutocreate);
		ini.Put_Bool(IniName, "Prebuild", IsPrebuilt);
		ini.Put_Bool(IniName, "Reinforce", IsReinforcable);
		ini.Put_Bool(IniName, "Droppod", IsDroppod);
		ini.Put_Bool(IniName, "Whiner", IsWhiner);
		ini.Put_Bool(IniName, "LooseRecruit", IsLooseRecruit);
		ini.Put_Bool(IniName, "Aggressive", IsAggressive);
		ini.Put_Bool(IniName, "Suicide", IsSuicide);
		ini.Put_Int(IniName, "Priority", RecruitPriority);
		ini.Put_Int(IniName, "Max", MaxAllowed);
		ini.Put_Int(IniName, "TechLevel", TechLevel);
		ini.Put_Int(IniName, "Group", Group);
		ini.Put_Bool(IniName, "OnTransOnly", OnTransOnly);
		ini.Put_Bool(IniName, "AvoidThreats", AvoidThreats);
		ini.Put_Bool(IniName, "IonImmune", IsIonImmune);
		ini.Put_Bool(IniName, "TransportsReturnOnUnload", TransportsReturnOnUnload);
		ini.Put_Bool(IniName, "AreTeamMembersRecruitable", AreTeamMembersRecruitable);
		ini.Put_Bool(IniName, "IsBaseDefense", IsBaseDefense);
		ini.Put_Bool(IniName, "OnlyTargetHouseEnemy", OnlyTargetHouseEnemy);

		if (Tag != NULL) {
			ini.Put_String(IniName, "Tag", Tag->IniName);
		}

		ini.Put_String(IniName, "Waypoint", Waypoint_To_Name(Origin));

		/*
		**	Record the # of missions, and each mission name & argument value.
		*/
		if (Script != NULL) {
			ini.Put_String(IniName, "Script", Script->IniName);
		}

		/*
		**	For every class in the team, record the class's name & desired count
		*/
		if (TaskForce != NULL) {
			ini.Put_String(IniName, "TaskForce", TaskForce->IniName);
		}

		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the group number this team type belongs to.
/// A team type that has no group of its own falls back on the group its task force belongs
/// to, so scripts may set the group in either place.
/// </summary>
/// <returns>Returns with the group number this team type belongs to.</returns>
int TeamTypeClass::Get_Group(void) const
{
	if (Group != -2 && Group == -1) {
		if (TaskForce != NULL) {
			return(TaskForce->Group);
		}
	}
	return(Group);
}


/// <summary>
/// Fetches the cell that teams of this type start life at.
/// The origin is stored as a waypoint, so this routine resolves it against the scenario's
/// waypoint list before handing it back.
/// </summary>
/// <returns>Returns with the origin cell, or CELL_NONE if this team type has no origin
/// assigned.</returns>
Cell TeamTypeClass::Get_Origin(void) const
{
	if (Origin == -1) {
		return(CELL_NONE);
	}
	return(Scen->Get_Waypoint_Cell(Origin));
}


/// <summary>
/// Fetches the team type of the name given, creating it if need be.
/// </summary>
/// <param name="name">The name of the team type wanted.</param>
/// <returns>Returns with a pointer to the team type, freshly created if it did not exist
/// already.</returns>
TeamTypeClass * TeamTypeClass::Find_Or_Make(char const * name)
{
	return(TFind_Or_Make<TeamTypeClass>(name, TeamTypes));
}


/// <summary>
/// Reads every team type listed in the INI database.
/// Each team type named is created if it does not exist already and is then told to read its
/// own data. Use this routine when loading a scenario.
/// </summary>
/// <param name="ini">The INI database to read the team types from.</param>
/// <param name="scope">The scope to tag each team type read with.</param>
void TeamTypeClass::Read_All(CCINIClass const & ini, INIScopeType scope)
{
	char buffer[32];

	int len = ini.Entry_Count("TeamTypes");
	for (int index = 0; index < len; index++) {
		char const * entry = ini.Get_Entry("TeamTypes", index);
		assert(entry != NULL);
		ini.Get_String("TeamTypes", entry, "", buffer, sizeof(buffer));
		TeamTypeClass * ttptr = Find_Or_Make(buffer);
		assert(ttptr != NULL);
		ttptr->Read_INI(ini);
		ttptr->Scope = scope;
	}
}


/// <summary>
/// Writes every team type of the scope specified out to the INI database.
/// Any team types the database already holds are cleared out first, so that stale entries
/// left over from an earlier save do not survive. Use this routine when saving a scenario.
/// </summary>
/// <param name="ini">The INI database to write the team types to.</param>
/// <param name="scope">Only team types of this scope are written.</param>
void TeamTypeClass::Write_All(CCINIClass & ini, INIScopeType scope)
{
	char buffer[32];
	int index;

	int numtypes = ini.Entry_Count("TeamTypes");
	for (index = 0; index < numtypes; index++) {
		char const * entry = ini.Get_Entry("TeamTypes", index);
		ini.Get_String("TeamTypes", entry, "", buffer, sizeof(buffer));
		ini.Clear(buffer);
	}
	ini.Clear("TeamTypes");

	int i = 0;
	for (index = 0; index < TeamTypes.Count(); index++) {
		if (TeamTypes[index]->Scope == scope) {
			sprintf(buffer, "%d", i++);
			ini.Put_String("TeamTypes", buffer, (char const *)TeamTypes[index]->IniName);
			TeamTypes[index]->Write_INI(ini);
		}
	}
}


/// <summary>
/// Lists the members this team type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TeamTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HeapID);
	stream.Serialize(Group);
	stream.Serialize(VeteranLevel);
	stream.Serialize(IsLoadable);
	stream.Serialize(IsFull);
	stream.Serialize(IsAnnoyance);
	stream.Serialize(IsGuardSlower);
	stream.Serialize(IsRecruiter);
	stream.Serialize(IsAutocreate);
	stream.Serialize(IsPrebuilt);
	stream.Serialize(IsReinforcable);
	stream.Serialize(IsWhiner);
	stream.Serialize(IsAggressive);
	stream.Serialize(IsLooseRecruit);
	stream.Serialize(IsSuicide);
	stream.Serialize(IsDroppod);
	stream.Serialize(IsDropship);
	stream.Serialize(OnTransOnly);
	stream.Serialize(RecruitPriority);
	stream.Serialize(MaxAllowed);
	stream.Serialize(Fear);
	stream.Serialize(House);
	stream.Serialize(TechLevel);
	stream.Serialize(Tag);
	stream.Serialize(Origin);
	stream.Serialize(Number);
	stream.Serialize(Script);
	stream.Serialize(TaskForce);
	stream.Serialize(Scope);
	stream.Serialize(AvoidThreats);
	stream.Serialize(IsIonImmune);
	stream.Serialize(TransportsReturnOnUnload);
	stream.Serialize(AreTeamMembersRecruitable);
	stream.Serialize(IsBaseDefense);
	stream.Serialize(OnlyTargetHouseEnemy);
}


/// <summary>
/// Fetches the class identifier of this object.
/// This is part of the persistence contract that the save and load code leans on to
/// recognize what it is reading back.
/// </summary>
/// <returns>Returns with S_OK and the class identifier filled in, or E_POINTER if no
/// destination was supplied.</returns>
HRESULT TeamTypeClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_TeamTypeClass;
	return(S_OK);
}


/// <summary>
/// Submits this team type's data to the CRC accumulator.
/// This routine is used by the network code when comparing game state between machines in
/// search of a desynchronization.
/// </summary>
/// <param name="crc">The accumulator to submit this team type's data to.</param>
void TeamTypeClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(Group);
	crc(IsRecruiter);
	crc(IsAutocreate);
	crc(IsPrebuilt);
	crc(IsReinforcable);
	crc(IsWhiner);
	crc(IsAggressive);
	crc(IsLooseRecruit);
	crc(IsSuicide);
	crc(IsDroppod);
	crc(RecruitPriority);
	crc(MaxAllowed);
	crc(Fear);

	if (House != NULL) {
		crc((RTTIType)House->RTTI);
	}
	if (House != NULL) {
		crc(House->Fetch_ID());
	}

	crc(TechLevel);

	if (Tag != NULL) {
		crc(Tag->Fetch_ID());
	}

	crc(Origin);
	crc(Number);

	if (Script != NULL) {
		crc(Script->Fetch_ID());
	}

	if (TaskForce != NULL) {
		crc(TaskForce->Fetch_ID());
	}
}


/// <summary>
/// Can the object specified be recruited into a team of this type?
/// This routine is used by the team building logic to weed out objects that are dead, busy,
/// owned by somebody else, or already serving a team of equal or better priority.
/// </summary>
/// <param name="foot">The object being considered for recruitment.</param>
/// <param name="house">The house the team is being assembled for.</param>
/// <returns>bool; Can the object be recruited?</returns>
bool TeamTypeClass::Can_Recruit(FootClass * foot, HouseClass * house) const
{
	if (foot->Team != NULL && this == foot->Team->Class) {
		return(false);
	}

	if (foot->Strength <= 0 || !foot->IsActive || foot->IsInLimbo) {
		return(false);
	}

	if (foot->In_Radio_Contact()) {
		return(false);
	}

	if (foot->House != house) {
		return(false);
	}

	if (foot->Mission != MISSION_NONE && !MissionClass::Is_Recruitable_Mission(foot->Mission)) {
		return(false);
	}

	if ((!foot->IsTeamRecruitable && !IsAutocreate) || (!foot->IsAutocreateRecruitable && IsAutocreate)) {
		return(false);
	}

	if (foot->Team != NULL && foot->Team->Class->RecruitPriority >= RecruitPriority) {
		return(false);
	}

	if (foot->RTTI == RTTI_AIRCRAFT && foot->PrimaryWeapon != NULL && foot->Ammo == NULL) {
		return(false);
	}

	return(true);
}


/// <summary>
/// Flashes every team that was built from this team type.
/// This routine is used by the scenario editor so that the designer can see at a glance
/// which teams on the map came from the team type under examination.
/// </summary>
/// <param name="frames">The number of game frames the teams should flash for.</param>
void TeamTypeClass::Flash(int frames)
{
	for (int index = 0; index < Teams.Count(); index++) {
		TeamClass * team = Teams[index];
		if (team->Class == this) {
			team->Flash(frames);
		}
	}
}


/// <summary>
/// Fetches the first live team that was built from this team type.
/// Use this routine when all that is to hand is the team type but the caller needs to reach
/// an actual team on the map.
/// </summary>
/// <returns>Returns with a pointer to the first team of this type. Otherwise, NULL is
/// returned.</returns>
TeamClass * TeamTypeClass::Find_First_Of_Type(void)
{
	for (int index = 0; index < Teams.Count(); index++) {
		TeamClass *team = Teams[index];
		if (team->Class == this) {
			return(team);
		}
	}
	return(NULL);
}
