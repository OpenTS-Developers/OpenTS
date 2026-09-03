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

#include "aitrig.h"

#include "_rules.h"
#include "airctype.h"
#include "builtype.h"
#include "crc.h"
#include "findmake.h"
#include "globals.h"
#include "house.h"
#include "houstype.h"
#include "infatype.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "session.h"
#include "sun.h"
#include "swizzle.h"
#include "taskforc.h"
#include "techtype.h"
#include "trim.h"
#include "unittype.h"
#include "vector.h"

#include <algorithm>

char const *TRIG_DIFFICULTIES[DIFF_COUNT] = {
	"Easy",
	"Medium",
	"Hard"
};

char const * NONE_STRING = "<none>";
char const * ALL_STRING = "<all>";

char const * const AITriggerTypeClass::INI_NAME = "AITriggerTypes";
char const * const AITriggerTypeClass::INI_NAME_ENABLE = "AITriggerTypesEnable";


/// <summary>
/// Creates a blank AI trigger type of the name specified.
/// The trigger starts out disabled and with no teams or condition object attached, and
/// adds itself to the master list of trigger types.
/// </summary>
AITriggerTypeClass::AITriggerTypeClass(const char *name) :
	BASECLASS(name),
	Type(AIT_NONE),
	House(-1),
	CurWeight(1),
	MinWeight(1),
	MaxWeight(1),
	IsEnabledInEasy(true),
	IsEnabledInMedium(true),
	IsEnabledInHard(true),
	Scope(SCOPE_LOCAL),
	TrigHouse(AITRIG_HOUSE_NONE),
	IsEnabled(false),
	MultiSide(0),
	TechLevelNeeded(0),
	IsAvailableInSkirmish(false),
	IsForBaseDefense(false),
	ConditionObject(NULL),
	TeamTypeOne(NULL),
	TeamTypeTwo(NULL),
	TimesSucceded(0),
	TimesExecuted(0)
{
	memset(Params.Data, 0, sizeof(Params.Data));

	AITriggerTypes.Add(this);

}


/// <summary>
/// Removes this trigger type from the master trigger list.
/// </summary>
AITriggerTypeClass::~AITriggerTypeClass(void)
{
	AITriggerTypes.Delete(this);
}


/// <summary>
/// Fetches the class identifier of this object.
/// The save game system uses this identifier to work out which class to build when the
/// object is read back in.
/// </summary>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT STDMETHODCALLTYPE AITriggerTypeClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_AITriggerTypeClass;
	return(S_OK);
}


/// <summary>
/// Lists the members this trigger carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void AITriggerTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Type);
	stream.Serialize(Scope);
	stream.Serialize(TrigHouse);
	stream.Serialize(IsEnabled);
	stream.Serialize(House);
	stream.Serialize(MultiSide);
	stream.Serialize(TechLevelNeeded);
	stream.Serialize(CurWeight);
	stream.Serialize(MinWeight);
	stream.Serialize(MaxWeight);
	stream.Serialize(IsAvailableInSkirmish);
	stream.Serialize(IsForBaseDefense);
	stream.Serialize(IsEnabledInEasy);
	stream.Serialize(IsEnabledInMedium);
	stream.Serialize(IsEnabledInHard);
	stream.Serialize(ConditionObject);
	stream.Serialize(TeamTypeOne);
	stream.Serialize(TeamTypeTwo);

	/*
	 * Which alternative of the parameters is live depends on the condition, but both are
	 * plain scalars and neither holds a pointer, so the union travels as its raw image.
	 */
	stream.Serialize_Bytes(&Params, sizeof(Params));

	stream.Serialize(TimesSucceded);
	stream.Serialize(TimesExecuted);
}


/// <summary>
/// Adds this trigger's settings to the CRC calculation.
/// This routine is used by the network sync check to prove that every machine in the
/// game agrees about the trigger.
/// </summary>
void AITriggerTypeClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);

	crc(Type);
	crc(Scope);
	crc(TrigHouse);
	crc(House);
	crc(TechLevelNeeded);

	switch (Type) {
		case AIT_ENEMY_OWNS_X_COND_N:
		case AIT_HOUSE_OWNS_X_COND_N:
		case AIT_ENEMY_MONEY_COND_N:
			crc(Params.Number);
			crc(Params.Condition);
			break;
	}

	crc(CurWeight);
	crc(MinWeight);
	crc(MaxWeight);
	crc(IsAvailableInSkirmish);
	crc(IsEnabledInEasy);
	crc(IsEnabledInMedium);
	crc(IsEnabledInHard);
	crc(MultiSide);
	crc(IsForBaseDefense);
}

/// Spring?

/// <summary>
/// Determines if this trigger may be sprung right now.
/// This routine is called by the house AI as it looks for a trigger to run. Every gate
/// the trigger carries is examined -- its scope, the difficulty setting, the side and
/// house it is restricted to, the tech level it demands and the skirmish rules -- before
/// its own condition is tested. Finally the teams it would produce must still be
/// buildable and must not already be at their allowed count.
/// </summary>
/// <param name="house">The house that would spring this trigger.</param>
/// <param name="enemy">The house being weighed up as the target, or NULL if there is none.</param>
/// <param name="skip_base_defense">Should base defense triggers be passed over?</param>
/// <returns>bool; May this trigger be sprung?</returns>
bool AITriggerTypeClass::Process(HouseClass *house, HouseClass *enemy, bool skip_base_defense)
{
	bool basedefense;
	if ((TeamTypeOne != NULL && TeamTypeOne->IsBaseDefense) && (TeamTypeTwo == NULL || TeamTypeTwo->IsBaseDefense)) {
		basedefense = true;
	} else {
		basedefense = false;
	}

	if (TeamTypeOne == NULL) {
		return(false);
	}

	if (enemy == NULL || (Rule->UseMinDefenseRule && house->BaseDefenseTeamCount < Rule->MinimumAIDefensiveTeams[house->Difficulty])) {
		if (!basedefense) {
			return(false);
		}
	}

	if (basedefense && skip_base_defense) {
		return(false);
	}

	if (Scope == SCOPE_GLOBAL && Scen->IsIgnoreGlobalAITriggers == true) {
		return(false);
	}

	if (!IsEnabled) {
		return(false);
	}

	if (Session.Type != GAME_NORMAL && !IsAvailableInSkirmish) {
		return(false);
	}

	int diff;
	if (Session.Type == GAME_NORMAL) {

		diff = Scen->Difficulty;
		if (diff == DIFF_EASY) {
			if (!IsEnabledInEasy) {
				return(false);
			}
		} else if (diff == DIFF_NORMAL) {
			if (!IsEnabledInMedium) {
				return(false);
			}
		} else if (diff == DIFF_HARD) {
			if (!IsEnabledInHard) {
				return(false);
			}
		}

	} else {

		diff = house->Difficulty;
		if (diff == DIFF_EASY) {
			if (!IsEnabledInHard) {
				return(false);
			}
		} else if (diff == DIFF_NORMAL) {
			if (!IsEnabledInMedium) {
				return(false);
			}
		} else if (diff == DIFF_HARD) {
			if (!IsEnabledInEasy) {
				return(false);
			}
		}
	}

	if (Session.Type == GAME_NORMAL) {
		if (TrigHouse == AITRIG_HOUSE_NONE) {
			return(false);
		}
		if (TrigHouse != AITRIG_HOUSE_ALL && TrigHouse == AITRIG_HOUSE_INDEX && house->Class->House != House) {
			return(false);
		}
	}

	if (MultiSide == 1) {
		if (house->ActLike != HOUSE_GOOD) {
			return(false);
		}
	} else if (MultiSide == 2) {
		if (house->ActLike != HOUSE_BAD) {
			return(false);
		}
	}

	if (TechLevelNeeded > house->Control.TechLevel) {
		return(false);
	}

	bool res = false;
	if (enemy != NULL) {
		switch (Type) {
			case AIT_NONE:
				res = true;
				break;
			case AIT_ENEMY_OWNS_X_COND_N:
				res = Check_Enemy_Owns(house, enemy);
				break;
			case AIT_HOUSE_OWNS_X_COND_N:
				res = Check_House_Owns(house, enemy);
				break;
			case AIT_POWER_YELLOW:
				res = Check_Enemy_Yellow_Power(house, enemy);
				break;
			case AIT_POWER_RED:
				res = Check_Enemy_Red_Power(house, enemy);
				break;
			case AIT_ENEMY_MONEY_COND_N:
				res = Check_Enemy_Money(house, enemy);
				break;
		}
	} else {
		if (Type == AIT_HOUSE_OWNS_X_COND_N) {
			res = Check_House_Owns(house, enemy);
		} else {
			res = Type == AIT_NONE;
		}
	}

	if (!res) {
		return(false);
	}

	if (!house->Can_Create_Team(TeamTypeOne)) {
		return(false);
	}
	if (TeamTypeTwo != NULL && !house->Can_Create_Team(TeamTypeTwo)) {
		return(false);
	}

	if (TeamTypeOne && TeamTypeOne->MaxAllowed >= 0 && house->Owned_Team_Count(TeamTypeOne) >= TeamTypeOne->MaxAllowed) {
		return(false);
	}
	if (TeamTypeTwo && TeamTypeTwo->MaxAllowed >= 0 && house->Owned_Team_Count(TeamTypeTwo) >= TeamTypeTwo->MaxAllowed) {
		return(false);
	}

	return(true);
}


/// <summary>
/// Checks how many of the condition object the enemy has.
/// This routine serves the enemy owns trigger condition. The enemy's tally of the
/// trigger's condition object is measured against the trigger's number using whichever
/// comparison the trigger was given.
/// </summary>
/// <returns>bool; Does the enemy's count satisfy the condition?</returns>
bool AITriggerTypeClass::Check_Enemy_Owns(HouseClass *house, HouseClass *enemy)
{
	bool res = false;

	if (enemy != NULL) {
		int count;
		TechnoTypeClass *obj = ConditionObject;

		if (obj != NULL) {
			int id = obj->Fetch_Heap_ID();
			int hcount = 0;
			RTTIType rtti = obj->RTTI;
			switch (rtti) {
				case RTTI_BUILDINGTYPE:
					hcount = enemy->ABQuantity.Value(id);
					break;

				case RTTI_UNITTYPE:
					hcount = enemy->AUQuantity.Value(id);
					break;

				case RTTI_INFANTRYTYPE:
					hcount = enemy->AIQuantity.Value(id);
					break;

				case RTTI_AIRCRAFTTYPE:
					hcount = enemy->AAQuantity.Value(id);
					break;
			}
			count = hcount;
		} else {
			count = 0;
		}

		switch (Params.Condition) {
			case COND_LT:
				res = count < Params.Number;
				break;

			case COND_LE:
				res = count <= Params.Number;
				break;

			case COND_EQ:
				res = count == Params.Number;
				break;

			case COND_GE:
				res = count >= Params.Number;
				break;

			case COND_GT:
				res = count > Params.Number;
				break;

			case COND_NE:
				res = count != Params.Number;
				break;
		}
	}

	return(res);
}


/// <summary>
/// Checks how many of the condition object the owning house has.
/// This routine serves the house owns trigger condition. The house's tally of the
/// trigger's condition object is measured against the trigger's number using whichever
/// comparison the trigger was given.
/// </summary>
/// <returns>bool; Does the house's count satisfy the condition?</returns>
bool AITriggerTypeClass::Check_House_Owns(HouseClass *house, HouseClass *enemy)
{
	bool res = false;

	if (house != NULL) {
		int count;
		TechnoTypeClass *obj = ConditionObject;

		if (obj != NULL) {
			int id = obj->Fetch_Heap_ID();
			int hcount = 0;
			RTTIType rtti = obj->RTTI;
			switch (rtti) {
				case RTTI_BUILDINGTYPE:
					hcount = house->ABQuantity.Value(id);
					break;

				case RTTI_UNITTYPE:
					hcount = house->AUQuantity.Value(id);
					break;

				case RTTI_INFANTRYTYPE:
					hcount = house->AIQuantity.Value(id);
					break;

				case RTTI_AIRCRAFTTYPE:
					hcount = house->AAQuantity.Value(id);
					break;
			}
			count = hcount;
		} else {
			count = 0;
		}

		switch (Params.Condition) {
			case COND_LT:
				res = count < Params.Number;
				break;

			case COND_LE:
				res = count <= Params.Number;
				break;

			case COND_EQ:
				res = count == Params.Number;
				break;

			case COND_GE:
				res = count >= Params.Number;
				break;

			case COND_GT:
				res = count > Params.Number;
				break;

			case COND_NE:
				res = count != Params.Number;
				break;
		}
	}

	return(res);
}


/// <summary>
/// Is the enemy base running short of surplus power?
/// This routine serves the yellow power trigger condition, which lets the AI move
/// against an enemy that is close to browning out rather than waiting for the lights to
/// go out entirely.
/// </summary>
/// <returns>bool; Is the enemy's spare power below the yellow threshold?</returns>
bool AITriggerTypeClass::Check_Enemy_Yellow_Power(HouseClass *house, HouseClass *enemy)
{
	bool res = false;

	if (enemy != NULL) {
		res = enemy->Power_Output() - enemy->Power_Drain() < 100.0;
	}
	return(res);
}


/// <summary>
/// Is the enemy base in a power deficit?
/// This routine serves the red power trigger condition, which is how the AI notices that
/// an enemy's defenses are going offline and that now would be a fine time to visit.
/// </summary>
/// <returns>bool; Is the enemy drawing more power than it produces?</returns>
bool AITriggerTypeClass::Check_Enemy_Red_Power(HouseClass *house, HouseClass *enemy)
{
	bool res = false;

	if (enemy != NULL) {
		res = enemy->Power_Output() - enemy->Power_Drain() < 0.0;
	}
	return(res);
}


/// <summary>
/// Checks the enemy's cash reserves against this trigger's condition.
/// This routine serves the money style trigger condition, comparing what the enemy can
/// actually spend against the number the trigger was given.
/// </summary>
/// <returns>bool; Does the enemy's available money satisfy the condition?</returns>
bool AITriggerTypeClass::Check_Enemy_Money(HouseClass *house, HouseClass *enemy)
{
	bool res = false;

	if (enemy != NULL) {
		int money = enemy->Available_Money();
		switch (Params.Condition) {
			case COND_LT:
				res = money < Params.Number;
				break;

			case COND_LE:
				res = money <= Params.Number;
				break;

			case COND_EQ:
				res = money == Params.Number;
				break;

			case COND_GE:
				res = money >= Params.Number;
				break;

			case COND_GT:
				res = money > Params.Number;
				break;

			case COND_NE:
				res = money != Params.Number;
				break;

			default:
				break;
		}
	}
	return(res);
}


/// <summary>
/// Fetches every AI trigger of the scope specified from the INI database.
/// This routine is used to bring in the global trigger list from the rules and the
/// scenario's own local list. Global triggers are always enabled; local ones take their
/// enabled state from the companion enable section.
/// </summary>
/// <param name="scope">The trigger scope being read -- global rules or scenario local.</param>
void AITriggerTypeClass::Read_All(CCINIClass const & ini, INIScopeType scope)
{
	int i;
	int count;

	count = ini.Entry_Count(INI_NAME);

	for (i = 0; i < count; i++) {
		const char *entry = ini.Get_Entry(INI_NAME, i);

		AITriggerTypeClass *ptr = Find_Or_Make(entry);
		if (ptr != NULL) {
			ptr->Read_INI(ini);
			ptr->Scope = scope;
		}

		if (scope == SCOPE_GLOBAL) {
			ptr->IsEnabled = true;
		}
	}

	if (scope == SCOPE_LOCAL) {
		count = ini.Entry_Count(INI_NAME_ENABLE);
		for (i = 0; i < count; i++) {
			const char *entry = ini.Get_Entry(INI_NAME_ENABLE, i);

			for (int k = 0; k < AITriggerTypes.Count(); k++) {
				if (strcmpi((const char *)AITriggerTypes[k]->IniName, entry) == 0) {
					AITriggerTypeClass *ptr = AITriggerTypes[k];
					if (ptr != NULL) {
						if (ini.Get_Bool(INI_NAME_ENABLE, entry) || Session.Type != GAME_NORMAL) {
							ptr->IsEnabled = true;
						} else {
							ptr->IsEnabled = false;
						}
					}
					break;
				}
			}
		}
	}
}


/// <summary>
/// Writes every AI trigger of the scope specified to the INI database.
/// This routine clears the old trigger section before recording the current triggers.
/// For local scope it also records each trigger's enabled state, since that is a
/// scenario level decision rather than a rules one.
/// </summary>
/// <param name="scope">The trigger scope to write -- global rules or scenario local.</param>
void AITriggerTypeClass::Write_All(CCINIClass & ini, INIScopeType scope)
{
	int i;
	char buf[32];

	int count = ini.Entry_Count(INI_NAME);

	for (i = 0; i < count; i++) {
		const char *entry = ini.Get_Entry(INI_NAME, i);
		ini.Get_String(INI_NAME, entry, "", buf, sizeof(buf));
		ini.Clear(buf);
	}

	ini.Clear(INI_NAME);

	for (i = 0; i < AITriggerTypes.Count(); i++) {
		AITriggerTypeClass *ptr = AITriggerTypes[i];
		if (ptr->Scope == scope) {
			ptr->Write_INI(ini);
		}
	}

	if (scope == SCOPE_LOCAL) {
		ini.Clear(INI_NAME_ENABLE);
		for (i = 0; i < AITriggerTypes.Count(); i++) {
			ini.Put_Bool(INI_NAME_ENABLE, (const char *)AITriggerTypes[i]->IniName, AITriggerTypes[i]->IsEnabled);
		}
	}
}


/// <summary>
/// Fetches this trigger's settings from the INI database.
/// This routine parses the comma separated line that the AI trigger section uses,
/// resolving the team, house, and condition object names into pointers. The tech level
/// the trigger demands is raised to cover whatever its teams need to be built.
/// </summary>
/// <returns>bool; Was an entry for this trigger found and parsed?</returns>
bool AITriggerTypeClass::Read_INI(CCINIClass const & ini)
{
	unsigned int i;
	char *paramstr;
	char *tok;
	char objname[24];
	char teamname[24];

	std::string line = ini.Get_String(INI_NAME, (const char *)IniName);
	if (!line.empty()) {

		tok = strtok(line.data(), ",");
		if (tok == NULL) {
			return(false);
		}

		GivenName = TStringID<48>(tok);

		tok = strtok(NULL, ",");
		if (tok == NULL) {
			return(false);
		}
		strncpy(objname, tok, sizeof(objname));
		objname[sizeof(objname)-1] = 0;
		strtrim(objname);
		TeamTypeOne = NULL;
		if (strcmpi(objname, NONE_STRING) != 0) {
			TeamTypeOne = TeamTypeClass::From_Name(objname);
		}

		tok = strtok(NULL, ",");
		if (tok == NULL) {
			return(false);
		}
		strncpy(objname, tok, sizeof(objname));
		objname[sizeof(objname)-1] = 0;
		strtrim(objname);
		TrigHouse = AITRIG_HOUSE_NONE;
		House = HOUSE_NONE;

		if (strcmpi(objname, ALL_STRING) == 0) {
			TrigHouse = AITRIG_HOUSE_ALL;
		} else if (strcmpi(objname, NONE_STRING) != 0) {
			House = HouseTypeClass::From_Name(objname);
			if (House != HOUSE_NONE) {
				TrigHouse = AITRIG_HOUSE_INDEX;
			}
		}

		tok = strtok(NULL, ",");
		if (tok == NULL) {
			return(false);
		}

		TechLevelNeeded = 0;
		tok = strtok(NULL, ",");
		if (tok == NULL) {
			return(false);
		}

		Type = (AITriggerEnum)atoi(tok);
		tok = strtok(NULL, ",");
		if (tok == NULL) {
			return(false);
		}

		strncpy(objname, tok, sizeof(objname));
		objname[sizeof(objname)-1] = 0;
		strtrim(objname);

		int type = -1;
		TechnoTypeClass * conobj = NULL;

		if (type == -1) {
			type = (int)InfantryTypeClass::From_Name(objname);
			if (type != -1) {
				conobj = InfantryTypes[type];
			}
		}

		if (type == -1) {
			type = (int)UnitTypeClass::From_Name(objname);
			if (type != -1) {
				conobj = UnitTypes[type];
			}
		}

		if (type == -1) {
			type = (int)AircraftTypeClass::From_Name(objname);
			if (type != -1) {
				conobj = AircraftTypes[type];
			}
		}

		if (type == -1) {
			type = (int)BuildingTypeClass::From_Name(objname);
			if (type != -1) {
				conobj = BuildingTypes[type];
			}
		}

		ConditionObject = conobj;

		tok = strtok(NULL, ",");
		if (tok == NULL) {
			return(false);
		}
		paramstr = tok;
		{
			char tmp;
			char *endptr;
			char nptr[] = "00";
			endptr = 0;
			i = 0;
			while (*paramstr != '\0') {
				while (isspace(*paramstr)) {
					paramstr++;
				}
				nptr[0] = *paramstr;
				paramstr++;
				tmp = *paramstr;
				if (tmp) {
					nptr[1] = tmp;
					paramstr++;
				} else {
					nptr[1] = '\0';
				}
				if (i >= ARRAY_SIZE(Params.Data)) {
					break;
				}

				Params.Data[i] = strtol(nptr, &endptr, 16);
				i++;
			}
		}

		tok = strtok(NULL, ",");
		if (tok != NULL) {
			CurWeight = (unsigned int)atof(tok);
		}

		tok = strtok(NULL, ",");
		if (tok != NULL) {
			MinWeight = (unsigned int)atof(tok);
		}

		tok = strtok(NULL, ",");
		if (tok != NULL) {
			MaxWeight = (unsigned int)atof(tok);
		}

		tok = strtok(NULL, ",");
		if (tok != NULL) {
			IsAvailableInSkirmish = atoi(tok) != 0;
		}

		tok = strtok(NULL, ",");

		tok = strtok(NULL, ",");
		if (tok != NULL) {
			MultiSide = atoi(tok);
		}

		tok = strtok(NULL, ",");
		if (tok != NULL) {
			IsForBaseDefense = atoi(tok) != 0;
		}

		tok = strtok(NULL, ",");
		if (tok != NULL) {
			strncpy(teamname, tok, sizeof(teamname));
			teamname[sizeof(teamname)-1] = 0;
			strtrim(teamname);
			TeamTypeTwo = NULL;
			if (strcmpi(teamname, NONE_STRING) != 0) {
				TeamTypeTwo = TeamTypeClass::From_Name(teamname);
			}
		}

		tok = strtok(NULL, ",");
		if (tok != NULL) {
			IsEnabledInEasy = atoi(tok) != 0;
		}

		tok = strtok(NULL, ",");
		if (tok != NULL) {
			IsEnabledInMedium = atoi(tok) != 0;
		}

		tok = strtok(NULL, ",");
		if (tok != NULL) {
			IsEnabledInHard = atoi(tok) != 0;
		}

		if (TeamTypeOne != NULL) {
			TechLevelNeeded = std::max(TechLevelNeeded, TeamTypeOne->TaskForce->Needed_Tech_Level());
		}

		if (TeamTypeTwo != NULL) {
			TechLevelNeeded = std::max(TechLevelNeeded, TeamTypeTwo->TaskForce->Needed_Tech_Level());
		}

		return(true);
	}
	return(false);
}


/// <summary>
/// Writes this trigger out to the INI database.
/// This routine flattens the trigger into the single comma separated line that the AI
/// trigger section uses. Any team, house, or condition object that is not specified is
/// recorded as the none placeholder so that the line can be read back unambiguously.
/// </summary>
/// <returns>bool; Was the trigger recorded in the INI database?</returns>
bool AITriggerTypeClass::Write_INI(CCINIClass & ini) const
{
	char paramstr[INIClass::MAX_LINE_LENGTH];
	char buf[INIClass::MAX_LINE_LENGTH];

	const char *ininame1 = NONE_STRING;
	const char *ininame2 = NONE_STRING;
	const char *housname = NONE_STRING;
	const char *condname = NONE_STRING;

	if (TeamTypeOne != NULL) {
		ininame1 = (const char *)TeamTypeOne->IniName;
	}

	if (TeamTypeTwo != NULL) {
		ininame2 = (const char *)TeamTypeTwo->IniName;
	}

	switch (TrigHouse) {
		case AITRIG_HOUSE_INDEX:
			if (House != HOUSE_NONE) {
				housname = (const char *)HouseTypes[House]->IniName;
			}
			break;

		case AITRIG_HOUSE_ALL: {
			housname = ALL_STRING;
			break;
		}
	}

	if (ConditionObject != NULL) {
		condname = (const char *)ConditionObject->IniName;
	}

	unsigned int i = 0;
	char *ptr = paramstr;
	while (i < ARRAY_SIZE(Params.Data)) {
		sprintf(ptr, "%02x", (unsigned char)Params.Data[i]);
		i++;
		ptr += 2;
	}
	*ptr = 0;

	sprintf(
		buf,
		"%s,%s,%s,%d,%d,%s,%s,%lf,%lf,%lf,%d,%d,%d,%d,%s,%d,%d,%d",
		(const char *)GivenName,
		ininame1,
		housname,
		TechLevelNeeded,
		Type,
		condname,
		paramstr,
		CurWeight,
		MinWeight,
		MaxWeight,
		IsAvailableInSkirmish != 0,
		0,
		MultiSide,
		IsForBaseDefense != 0,
		ininame2,
		IsEnabledInEasy != 0,
		IsEnabledInMedium != 0,
		IsEnabledInHard != 0);

	return(ini.Put_String(INI_NAME, IniName, buf));
}


/// <summary>
/// Fetches the trigger type of the name specified, creating it if need be.
/// This routine is used by the INI reader so that a trigger may be mentioned before it
/// has been declared.
/// </summary>
/// <returns>Returns with a pointer to the trigger type found or created.</returns>
AITriggerTypeClass * AITriggerTypeClass::Find_Or_Make(char const * name)
{
	return(TFind_Or_Make<AITriggerTypeClass>(name, AITriggerTypes));
}


/// <summary>
/// Sets the object type this trigger's condition counts.
/// The owns and money style trigger conditions test the tally of this object type
/// against the trigger's own number.
/// </summary>
void AITriggerTypeClass::Set_Condition_Object(TechnoTypeClass *obj)
{
	ConditionObject = obj;
}


/// <summary>
/// Sets the first team this trigger will create.
/// A trigger with no first team can never spring, so this is the one team a usable
/// trigger must be given.
/// </summary>
void AITriggerTypeClass::Set_First_TeamType(TeamTypeClass *team)
{
	TeamTypeOne = team;
}


/// <summary>
/// Sets the second team this trigger will create.
/// The second team is optional; a trigger with none set will only ever build its first
/// team.
/// </summary>
void AITriggerTypeClass::Set_Second_TeamType(TeamTypeClass *team)
{
	TeamTypeTwo = team;
}


/// <summary>
/// Records a successful run of this trigger.
/// This routine is called by the house AI once a trigger it sprang has paid off. The
/// trigger's weight is moved by the rules' success delta and by its track record so far,
/// then held within the trigger's own minimum and maximum, so that triggers that keep
/// working become more likely to be picked again.
/// </summary>
void AITriggerTypeClass::Record_Success(void)
{
	double weight = 0;
	if (TimesExecuted > 0) {
		weight = (double)TimesExecuted * ((double)TimesSucceded / (double)TimesExecuted - 0.5);
		if (weight < 0.0) {
			weight = 0.0;
		}
	}
	CurWeight = CurWeight + weight + Rule->AITriggerSuccessWeightDelta;

	if (CurWeight < MinWeight) {
		CurWeight = MinWeight;
	}

	if (CurWeight > MaxWeight) {
		CurWeight = MaxWeight;
	}

	TimesSucceded++;
	TimesExecuted++;
}


/// <summary>
/// Records a failed run of this trigger.
/// This routine is called by the house AI once a trigger it sprang has come to nothing.
/// The trigger's weight is moved by the rules' failure delta and by its track record so
/// far, then held within the trigger's own minimum and maximum, so that unproductive
/// triggers become less likely to be picked again.
/// </summary>
void AITriggerTypeClass::Record_Failure(void)
{
	double weight = 0;
	if (TimesExecuted > 0) {
		weight = (double)TimesExecuted * ((double)TimesSucceded / (double)TimesExecuted - 0.5) * Rule->AITriggerTrackRecordCoefficient;
		if (weight > 0.0) {
			weight = 0.0;
		}
	}
	CurWeight = CurWeight + weight + Rule->AITriggerFailureWeightDelta;

	if (CurWeight < MinWeight) {
		CurWeight = MinWeight;
	}

	if (CurWeight > MaxWeight) {
		CurWeight = MaxWeight;
	}

	TimesExecuted++;
}
