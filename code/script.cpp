/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#define INCLUDE_COM
#include "always.h"

#include "script.h"

#include "_script.h"
#include "ccini.h"
#include "crc.h"
#include "findmake.h"
#include "globals.h"
#include "savestream.h"
#include "sun.h"
#include "swizzle.h"
#include "tracker.h"
#include "trim.h"
#include "vector.h"

#include <cstdio>

/// <summary>
/// Creates a running instance of the specified script type.
/// This routine is used when a team is formed and needs its own copy of the script's
/// execution state. The new script is registered with the master script list and starts
/// out positioned before the first mission of its type.
/// </summary>
/// <param name="type">Pointer to the script type that supplies the mission list.</param>
ScriptClass::ScriptClass(ScriptTypeClass *type):
	Unused1(0),
	Class(type),
	CurrentLineNumber(-1)
{
	Scripts.Add(this);
}


/// <summary>
/// Destroys the script.
/// This routine will sever any reference the game objects hold to this script before it
/// is dropped from the master script list.
/// </summary>
ScriptClass::~ScriptClass(void)
{
	Detach_This_From_All(this, true);
	Scripts.Delete(this);
}


/// <summary>
/// Adds this script's state to the game state checksum.
/// This routine is used by the multiplayer synchronization checker to prove that every
/// machine has its teams at the same point in the same script.
/// </summary>
/// <param name="crc">The checksum engine to submit the script state to.</param>
void ScriptClass::Compute_CRC(CRCEngine &crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(CurrentLineNumber);
}


/// <summary>
/// Fetches the mission the script is presently on.
/// This routine is used by the team logic to discover what orders it should be carrying
/// out at this moment.
/// </summary>
/// <returns>Returns with the current team mission. If the script has not been started, a
/// TMISSION_NONE mission is returned.</returns>
TeamMissionClass ScriptClass::Get_Current_Mission(void)
{
	if (CurrentLineNumber == -1) {
		return(TeamMissionClass(TMISSION_NONE, 0));
	}

	return(Class->MissionList[CurrentLineNumber]);
}


/// <summary>
/// Fetches the mission that follows the current one.
/// Use this routine to peek ahead in the script so that a team can prepare for what it
/// will be asked to do next.
/// </summary>
/// <returns>Returns with the next team mission. If the script has nothing further to ask
/// for, a TMISSION_NONE mission is returned.</returns>
TeamMissionClass ScriptClass::Get_Next_Mission(void)
{
	if (CurrentLineNumber + 1 >= Class->MissionCount) {
		return(TeamMissionClass(TMISSION_NONE, 0));
	}

	return(Class->MissionList[CurrentLineNumber + 1]);
}


/// <summary>
/// Halts execution of the script.
/// This routine will rewind the script to before its first mission, so that a team
/// holding it is no longer given orders until the script is started afresh.
/// </summary>
bool ScriptClass::Stop_Script(void)
{
	CurrentLineNumber = -1;
	return(true);
}


/// <summary>
/// Sets the script to the specified mission line.
/// Use this routine to jump a team's script to a particular line rather than letting it
/// advance in order.
/// </summary>
/// <param name="linenum">The mission line to make current.</param>
bool ScriptClass::Set_Line(int linenum)
{
	CurrentLineNumber = linenum;
	return(true);
}


/// <summary>
/// Advances the script to the mission that follows.
/// This routine is called by the team logic once the current mission has been satisfied
/// and the team is ready to be given its next orders.
/// </summary>
/// <returns>bool; Is there a mission waiting at the new line?</returns>
bool ScriptClass::Next_Mission(void)
{
	CurrentLineNumber++;
	return(Has_Missions_Remaining());
}


/// <summary>
/// Is the script sitting on a mission it can still perform?
/// This routine is used to tell whether the team owning this script has run off the end
/// of its orders and needs disbanding or reassigning.
/// </summary>
/// <returns>bool; Does the script still have a mission to hand out?</returns>
bool ScriptClass::Has_Missions_Remaining(void)
{
	return((unsigned)CurrentLineNumber < (unsigned)Class->MissionCount);
}


/// <summary>
/// Fetches the class identifier used to persist this script.
/// This routine is part of the IPersistent contract that the save game system relies
/// on to recreate objects when a game is loaded.
/// </summary>
/// <param name="retval">Pointer to the identifier to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT ScriptClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_ScriptClass;
	return(S_OK);
}


/// <summary>
/// Lists the members this script carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void ScriptClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Class);
	stream.Serialize(Unused1);
	stream.Serialize(CurrentLineNumber);
}


/// <summary>
/// Creates a script type of the specified name.
/// This routine is used as the scenario's script list is read in. The new type is added
/// to the master script type heap and takes its heap identifier from it.
/// </summary>
/// <param name="name">The INI name to give this script type.</param>
ScriptTypeClass::ScriptTypeClass(char const *name) :
	BASECLASS(name),
	Scope(SCOPE_LOCAL),
	MissionCount(0)
{
	ScriptTypes.Add(this);
	HeapID = ScriptTypes.ID(this);
}


/// <summary>
/// Destroys the script type.
/// This routine will sever any reference the game objects hold to this script type
/// before it is dropped from the master script type list.
/// </summary>
ScriptTypeClass::~ScriptTypeClass(void)
{
	Detach_This_From_All(this, true);
	ScriptTypes.Delete(this);
}


/// <summary>
/// Writes this script type's mission list to the INI database.
/// This routine is used as a scenario is saved. Any mission slot this script does not
/// use is cleared out, so that no stale lines are left behind from an earlier version of
/// the script.
/// </summary>
/// <param name="ini">The INI database to write the mission list to.</param>
/// <returns>bool; Was the script type written out?</returns>
bool ScriptTypeClass::Write_INI(CCINIClass & ini) const
{
	if (BASECLASS::Write_INI(ini)) {
		char buf[128];
		char entry[16];

		for (int index = 0; index < MAX_TEAM_MISSIONS; index++) {
			TeamMissionClass tmission;

			sprintf(entry, "%d", index);
			strtrim(entry);
			if (index < MissionCount) {
				MissionList[index].Build_INI_Entry(buf);
				ini.Put_String(Name(), entry, buf);
			} else {
				ini.Clear(Name(), entry);
			}
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Reads this script type's mission list from the INI database.
/// This routine is used as a scenario is loaded. The numbered entries in the script's own
/// section supply the missions the teams assigned this script will carry out.
/// </summary>
/// <param name="ini">The INI database to read the mission list from.</param>
/// <returns>bool; Was the script type read in?</returns>
bool ScriptTypeClass::Read_INI(CCINIClass const & ini)
{
	if (BASECLASS::Read_INI(ini)) {
		char buf[128];
		char entry[16];

		MissionCount = 0;

		for (int index = 0; index < MAX_TEAM_MISSIONS; index++) {
			TeamMissionClass tmission;

			sprintf(entry, "%d", index);
			strtrim(entry);

			if (ini.Get_String(Name(), entry, "", buf, sizeof(buf)) > 0) {
				tmission.Fill_In(buf);
				if (MissionCount < MAX_TEAM_MISSIONS) {
					MissionList[MissionCount] = tmission;
					MissionCount++;
				}
			}
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Reads every script type declared in the INI database.
/// This routine is used to bring in the scripts that a scenario or the rules file
/// declares. Each script named in the list is created if it does not already exist,
/// filled in, and tagged with the supplied scope.
/// </summary>
/// <param name="ini">The INI database to read the script types from.</param>
/// <param name="scope">The scope to assign to every script type read in.</param>
void ScriptTypeClass::Read_All(CCINIClass const & ini, INIScopeType scope)
{
	int len = ini.Entry_Count("ScriptTypes");
	for (int index = 0; index < len; index++) {
		char const * entry = ini.Get_Entry("ScriptTypes", index);
		assert(entry != NULL);
		char name[24];
		if (ini.Get_String("ScriptTypes", entry, "", name, sizeof(name)) <= 0) {
			continue;
		}

		ScriptTypeClass * script = Find_Or_Make(name);
		assert(script != NULL);
		script->Read_INI(ini);
		script->Scope = scope;
	}
}


/// <summary>
/// Writes every script type out to the INI database.
/// This is the counterpart to Read_All, and would be used when a scenario is saved back
/// out.
/// </summary>
void ScriptTypeClass::Write_All(CCINIClass & ini, INIScopeType scope)
{
	/// unimplemented
}


/// <summary>
/// Finds the script type of the specified name.
/// This routine will match against either the INI name or the name the scenario designer
/// gave the script, so either spelling will locate it.
/// </summary>
/// <param name="name">The name of the script type to search for.</param>
/// <returns>Returns with a pointer to the script type found. Otherwise, NULL is
/// returned.</returns>
ScriptTypeClass * ScriptTypeClass::From_Name(char const * name)
{
	if (name != NULL) {
		for (int index = 0; index < ScriptTypes.Count(); index++) {
			if (stricmp(name, ScriptTypes[index]->IniName) == 0 || stricmp(name, ScriptTypes[index]->GivenName) == 0) {
				return(ScriptTypes[index]);
			}
		}
	}
	return(NULL);
}


/// <summary>
/// Finds the script type carrying the specified designer name.
/// Unlike From_Name, this routine will only consider the name the scenario designer gave
/// the script, never its INI name.
/// </summary>
/// <param name="name">The given name of the script type to search for.</param>
/// <returns>Returns with a pointer to the script type found. Otherwise, NULL is
/// returned.</returns>
ScriptTypeClass * ScriptTypeClass::From_Given_Name(char const * name)
{
	if (name != NULL) {
		for (int index = 0; index < ScriptTypes.Count(); index++) {
			if (stricmp(name, ScriptTypes[index]->GivenName) == 0) {
				return(ScriptTypes[index]);
			}
		}
	}
	return(NULL);
}


/// <summary>
/// Fetches the script type of the specified name, creating it if necessary.
/// This routine is used while a scenario is being read, where a script can be referred
/// to before its own section has been reached.
/// </summary>
/// <param name="name">The name of the script type to find or create.</param>
/// <returns>Returns with a pointer to the script type.</returns>
ScriptTypeClass * ScriptTypeClass::Find_Or_Make(char const * name)
{
	return(TFind_Or_Make<ScriptTypeClass>(name, ScriptTypes));
}


/// <summary>
/// Fetches the class identifier used to persist this script type.
/// This routine is part of the IPersistent contract that the save game system relies
/// on to recreate objects when a game is loaded.
/// </summary>
/// <param name="retval">Pointer to the identifier to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT ScriptTypeClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_ScriptTypeClass;
	return(S_OK);
}


/// <summary>
/// Lists the members this script type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void ScriptTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HeapID);
	stream.Serialize(Scope);
	stream.Serialize(MissionCount);
	stream.Serialize(MissionList);
}


/// <summary>
/// Adds this script type's state to the game state checksum.
/// This routine is used by the multiplayer synchronization checker to prove that every
/// machine is running with the same script definitions.
/// </summary>
/// <param name="crc">The checksum engine to submit the script type state to.</param>
void ScriptTypeClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);

	crc(MissionCount);
}
