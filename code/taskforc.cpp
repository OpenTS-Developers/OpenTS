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

#define INCLUDE_COM
#include "always.h"

#include "taskforc.h"

#include "ahtoi.h"
#include "ccini.h"
#include "findmake.h"
#include "globals.h"
#include "savestream.h"
#include "sun.h"
#include "swizzle.h"
#include "techtype.h"
#include "tracker.h"
#include "trim.h"
#include "vector.h"

#include <cstdio>


/// <summary>
/// Creates a task force with the specified name.
/// The new force joins the global task force list and starts out empty, waiting for its
/// member list to arrive from the INI database.
/// </summary>
/// <param name="name">The INI name to give the new task force.</param>
TaskForceClass::TaskForceClass(char const *name) :
	BASECLASS(name),
	Group(-1),
	ClassCount(0),
	Scope(SCOPE_LOCAL)
{
	TaskForces.Add(this);

	for (int i = 0; i < MAX_TEAM_CLASSCOUNT; i++) {
		Members[i] = EnlistedMemberClass(0,NULL);
	}
}


/// <summary>
/// Destroys the task force.
/// Anything still pointing at this force is detached from it before the force drops out
/// of the global list.
/// </summary>
TaskForceClass::~TaskForceClass(void)
{
	Detach_This_From_All(this, true);
	TaskForces.Delete(this);
}


/// <summary>
/// Creates a copy of the specified task force.
/// </summary>
TaskForceClass::TaskForceClass(TaskForceClass const & that) :
	BASECLASS(that)
{
	Group = that.Group;
	ClassCount = that.ClassCount;
	Scope = that.Scope;

	for (int i = 0; i < ClassCount; i++) {
		Members[i] = that.Members[i];
	}
}


/// <summary>
/// Copies the specified task force over this one.
/// </summary>
/// <returns>Returns with this task force, now holding the same members as the source.</returns>
TaskForceClass TaskForceClass::operator=(TaskForceClass const & that)
{
	if (this != &that) {
		Group = that.Group;
		ClassCount = that.ClassCount;
		Scope = that.Scope;

		for (int i = 0; i < ClassCount; i++) {
			Members[i] = that.Members[i];
		}
	}
	return(*this);
}


/// <summary>
/// Fetches the number of objects needed to fill out this task force.
/// This routine is used by the team code to know how many recruits must be gathered
/// before the team can be considered complete.
/// </summary>
/// <returns>Returns with the total quantity called for by all members of the force.</returns>
int TaskForceClass::Required_Object_Count(void) const
{
	int desired = 0;
	/*
	**	Figure out the total number of objects that this team type requires.
	*/
	for (int index = 0; index < ClassCount; index++) {
		desired += Members[index].Quantity;
	}
	assert(desired != 0);

	return(desired);
}


/// <summary>
/// Fetches the task force that carries the specified INI name.
/// </summary>
/// <returns>Returns with a pointer to the task force, or NULL if there is no match.</returns>
TaskForceClass * TaskForceClass::From_Name(char const * name)
{
	if (name != NULL) {
		for (int index = 0; index < TaskForces.Count(); index++) {
			if (stricmp(name, TaskForces[index]->IniName) == 0) {
				return(TaskForces[index]);
			}
		}
	}
	return(NULL);
}


/// <summary>
/// Fetches the task force that carries the specified given name.
/// This is the human readable name the designer typed into the editor, as opposed to the
/// INI name the scenario file refers to the force by.
/// </summary>
/// <returns>Returns with a pointer to the task force, or NULL if there is no match.</returns>
TaskForceClass * TaskForceClass::From_Given_Name(char const * name)
{
	if (name != NULL) {
		for (int index = 0; index < TaskForces.Count(); index++) {
			if (stricmp(name, TaskForces[index]->GivenName) == 0) {
				return(TaskForces[index]);
			}
		}
	}
	return(NULL);
}


/// <summary>
/// Reads all of the task forces out of the INI database.
/// This routine is used when loading the rules or a scenario. Each force listed is
/// created if it does not exist yet, registered with the swizzler so that teams can be
/// pointed at it, and then told to read its own section.
/// </summary>
/// <param name="scope">The scope to assign to every task force read in.</param>
void TaskForceClass::Read_All(CCINIClass const & ini, INIScopeType scope)
{
	int len = ini.Entry_Count("TaskForces");
	for (int index = 0; index < len; index++) {
		char const * entry = ini.Get_Entry("TaskForces", index);
		assert(entry != NULL);
		char name[24];
		if (ini.Get_String("TaskForces", entry, "", name, sizeof(name)) <= 0) {
			continue;
		}

		TaskForceClass * tforce = Find_Or_Make(name);
		assert(tforce != NULL);
		Swizzle_Here_I_Am(ahtoi(name), tforce);

		tforce->Read_INI(ini);
		tforce->Scope = scope;
	}
}


/// <summary>
/// Writes all of the task forces of the specified scope out to the INI database.
/// The task force list and every section it points to are cleared first, so that the
/// database ends up describing the forces that exist now rather than the ones that used
/// to.
/// </summary>
/// <param name="scope">Only task forces of this scope are written out.</param>
void TaskForceClass::Write_All(CCINIClass & ini, INIScopeType scope)
{
	char buffer[32];
	int index;

	int numtypes = ini.Entry_Count("TaskForces");
	for (index = 0; index < numtypes; index++) {
		char const * entry = ini.Get_Entry("TaskForces", index);
		ini.Get_String("TaskForces", entry, "", buffer, sizeof(buffer));
		ini.Clear(buffer);
	}
	ini.Clear("TaskForces");

	int i = 0;
	for (index = 0; index < TaskForces.Count(); index++) {
		TaskForceClass *tforce = TaskForces[index];
		if (tforce->Scope == scope) {
			sprintf(buffer, "%d", i++);
			strtrim(buffer);
			ini.Put_String("TaskForces", buffer, (char const *)tforce->IniName);
			tforce->Write_INI(ini);
		}
	}
}


/// <summary>
/// Reads this task force from the INI database.
/// The member list is rebuilt from the numbered entries in the force's own section. A
/// member that names an object type the game does not recognize is quietly dropped.
/// </summary>
/// <returns>bool; Was the task force read in?</returns>
bool TaskForceClass::Read_INI(CCINIClass const & ini)
{
	if (BASECLASS::Read_INI(ini)) {
		char buf[128];
		char entry[32];

		ClassCount = 0;

		for (int index = 0; index < MAX_TEAM_CLASSCOUNT; index++) {
			sprintf(entry, "%d", index);

			if (ini.Get_String(Name(), entry, "", buf, sizeof(buf)) > 0) {
				Members[ClassCount] = EnlistedMemberClass(buf);
				if (Members[ClassCount].Class != NULL) {
					ClassCount++;
				}
			}
		}
		Group = ini.Get_Int(IniName, "Group", Group);
		return(true);
	}
	return(false);
}


/// <summary>
/// Writes this task force out to the INI database.
/// Every member slot is dealt with, and the slots this force does not use are cleared,
/// so that no leftover member from an earlier version of the force survives.
/// </summary>
/// <returns>bool; Was the task force written out?</returns>
bool TaskForceClass::Write_INI(CCINIClass & ini) const
{
	if (BASECLASS::Write_INI(ini)) {
		char entry[32];

		for (int index = 0; index < MAX_TEAM_CLASSCOUNT; index++) {
			EnlistedMemberClass enlisted;

			sprintf(entry, "%d", index);
			if (index < ClassCount) {
				ini.Put_String(Name(), entry, Members[index].Build_INI_Entry());
			} else {
				ini.Clear(Name(), entry);
			}
		}
		ini.Put_Int(IniName, "Group", Group, 0);
		return(true);
	}
	return(false);
}


/// <summary>
/// Is this task force made up of nothing but infantry?
/// This routine is used by the team code when it must decide whether the force can go
/// where only foot soldiers are able to follow.
/// </summary>
/// <returns>bool; Are all of the members infantry?</returns>
bool TaskForceClass::Has_Only_Infantry(void) const
{
	for (int i = 0; i < ClassCount; i++) {
		if (Members[i].Class->RTTI != RTTI_INFANTRYTYPE) {
			return(false);
		}
	}
	return(true);
}


/// <summary>
/// Fetches the task force of the specified name, creating it if need be.
/// This routine is used while reading a scenario, so that a task force can be referred
/// to before its own section has been reached.
/// </summary>
/// <returns>Returns with a pointer to the task force found or created.</returns>
TaskForceClass * TaskForceClass::Find_Or_Make(char const * name)
{
	return(TFind_Or_Make<TaskForceClass>(name, TaskForces));
}


/// <summary>
/// Lists the members this task force carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TaskForceClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Group);
	stream.Serialize(ClassCount);
	stream.Serialize(Scope);
	stream.Serialize(Members);
}


/// <summary>
/// Fetches the class identifier of this object.
/// This routine is used by the save game code so that an object of the right kind can
/// be created when the game is loaded back in.
/// </summary>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT TaskForceClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_TaskForceClass;
	return(S_OK);
}


/// <summary>
/// Adds the state of this task force to the running game checksum.
/// This routine is used by the multiplayer sync checking code.
/// </summary>
void TaskForceClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);

	crc(Group);
	crc(ClassCount);
}


/// <summary>
/// Fetches the tech level needed before this task force can be fielded.
/// This routine is used by the team building logic to tell whether a house is advanced
/// enough to raise the whole force. A member that cannot be built by normal means pushes
/// the requirement out past anything a house will ever reach.
/// </summary>
/// <returns>Returns with the highest tech level demanded by any member of the force.</returns>
int TaskForceClass::Needed_Tech_Level(void) const
{
	int needed = 0;

	for (int i = 0; i < ClassCount; i++) {
		int level = Members[i].Class->Level;
		if (level > needed) {
			needed = level;
		} else {
			if (level == -1) {
				needed = 11;
			}
		}
	}
	return(needed);
}
