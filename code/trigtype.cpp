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

/* $Header: /CounterStrike/TRIGTYPE.CPP 1     3/03/97 10:26a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TRIGTYPE.CPP                                                 *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 06/05/96                                                     *
 *                                                                                             *
 *                  Last Update : July 9, 1996 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   TriggerTypeClass::As_Target -- Convert this trigger type object into a target value.      *
 *   TriggerTypeClass::Attaches_To -- Determines what trigger can attach to.                   *
 *   TriggerTypeClass::Build_INI_Entry -- Construct the INI entry into the buffer specified.   *
 *   TriggerTypeClass::Description -- Build a text description of the trigger type.            *
 *   TriggerTypeClass::Detach -- Removes attachments to the target object specified.           *
 *   TriggerTypeClass::Draw_It -- Draws this trigger as if it were a line in a list box.       *
 *   TriggerTypeClass::Edit -- Edit the trigger type through the scenario editor.              *
 *   TriggerTypeClass::Fill_In -- fills in trigger from the given INI entry                    *
 *   TriggerTypeClass::From_Name -- Convert an ASCII name into a trigger type pointer.         *
 *   TriggerTypeClass::Init -- Initialize the trigger type object management system.           *
 *   TriggerTypeClass::Read_INI -- reads triggers from the INI file                            *
 *   TriggerTypeClass::TriggerTypeClass -- Constructor for trigger class object.               *
 *   TriggerTypeClass::Write_INI -- Stores all trigger types to the INI database specified.    *
 *   TriggerTypeClass::operator delete -- Returns a trigger type class object back to the pool *
 *   TriggerTypeClass::operator new -- Allocates a trigger type class object.                  *
 *   TriggerTypeClass::~TriggerTypeClass -- Deleting a trigger type deletes associated triggers*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "trigtype.h"

#include "_map.h"
#include "_rules.h"
#include "ccini.h"
#include "findmake.h"
#include "globals.h"
#include "house.h"
#include "houstype.h"
#include "incdec.h"
#include "savestream.h"
#include "sun.h"
#include "swizzle.h"
#include "taction.h"
#include "teamtype.h"
#include "tevent.h"
#include "tracker.h"

#include "persist.hh"

#include <cstdio>


char const * const TriggerTypeClass::INI_NAME = "Triggers";
char const * const TriggerTypeClass::INI_ACTION_NAME = "Actions";
char const * const TriggerTypeClass::INI_EVENT_NAME = "Events";


/***********************************************************************************************
 * TriggerTypeClass::TriggerTypeClass -- Constructor for trigger class object.                 *
 *                                                                                             *
 *    This is the normal constructor for a trigger object. The trigger starts with no team     *
 *    members, no mission, and default values for all settings.                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
TriggerTypeClass::TriggerTypeClass(char const * name) :
	BASECLASS(name),
	HeapID(-1),
	_IsEnabled(true),
	IsEnabledOnEasy(true),
	IsEnabledOnMedium(true),
	IsEnabledOnHard(true),
	IsToInherit(false),
	House(NULL),
	LinkedTo(NULL),
	FirstEvent(NULL),
	FirstAction(NULL)
{
	EventActionPtrTracker.Add(this);
	AbstractTypePtrTracker.Add(this);

	HeapID = TriggerTypes.Count();
	TriggerTypes.Add(this);
}


/***********************************************************************************************
 * TriggerTypeClass::~TriggerTypeClass -- Deleting a trigger type deletes associated triggers. *
 *                                                                                             *
 *    When a trigger type is deleted, then all triggers that refer to that type must also      *
 *    be deleted as well. There can be no 'orphan' triggers in existence.                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/09/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
TriggerTypeClass::~TriggerTypeClass(void)
{
	Detach_This_From_All(this, true);
	TriggerTypes.Delete(this);

	TActionClass * action = FirstAction;
	while (action != NULL) {
		TActionClass * next = action->Next;
		delete action;
		action = next;
	}

	TEventClass * event = FirstEvent;
	while (event != NULL) {
		TEventClass * next = event->Next;
		delete event;
		event = next;
	}

	AbstractTypePtrTracker.Delete(this);
	EventActionPtrTracker.Delete(this);
}


/// <summary>
/// Does this trigger watch for a vertical line crossing?
/// A trigger springs on any one of the events in its list. This routine picks the
/// vertical line crossing event out of that list.
/// </summary>
/// <returns>bool; Is one of this trigger's events a vertical line crossing?</returns>
bool TriggerTypeClass::Is_Cross_Vertical(void) const
{
	bool found = false;
	TEventClass * tevent = FirstEvent;
	while (tevent != NULL) {
		if (tevent->Event == TEVENT_CROSS_VERTICAL) {
			found = true;
			break;
		}
		tevent = tevent->Next;
	}
	return(found);
}


/// <summary>
/// Does this trigger watch for a horizontal line crossing?
/// A trigger springs on any one of the events in its list. This routine picks the
/// horizontal line crossing event out of that list.
/// </summary>
/// <returns>bool; Is one of this trigger's events a horizontal line crossing?</returns>
bool TriggerTypeClass::Is_Cross_Horizontal(void) const
{
	bool found = false;
	TEventClass * tevent = FirstEvent;
	while (tevent != NULL) {
		if (tevent->Event == TEVENT_CROSS_HORIZONTAL) {
			found = true;
			break;
		}
		tevent = tevent->Next;
	}
	return(found);
}


/// <summary>
/// Does this trigger watch for something entering its zone?
/// A trigger springs on any one of the events in its list. This routine picks the
/// enters zone event out of that list.
/// </summary>
/// <returns>bool; Is one of this trigger's events an enters zone event?</returns>
bool TriggerTypeClass::Is_Enters_Zone(void) const
{
	bool found = false;
	TEventClass * tevent = FirstEvent;
	while (tevent != NULL) {
		if (tevent->Event == TEVENT_ENTERS_ZONE) {
			found = true;
			break;
		}
		tevent = tevent->Next;
	}
	return(found);
}


/// <summary>
/// Does this trigger hold back the scenario victory?
/// The allow win action keeps the scenario from being won until every trigger that
/// carries it has sprung. This routine checks the trigger's action list for it.
/// </summary>
/// <returns>bool; Is one of this trigger's actions the allow win action?</returns>
bool TriggerTypeClass::Is_Allow_Win(void) const
{
	bool found = false;
	TActionClass * taction = FirstAction;
	while (taction != NULL) {
		if (taction->Action == TACTION_ALLOWWIN) {
			found = true;
			break;
		}
		taction = taction->Next;
	}
	return(found);
}


/// <summary>
/// Checks whether this trigger takes part at the specified difficulty.
/// </summary>
/// <param name="difficulty">The difficulty the scenario is being played at.</param>
/// <returns>Returns true if the trigger is active at that difficulty.</returns>
bool TriggerTypeClass::Is_Enabled_At(DiffType difficulty) const
{
	switch (difficulty) {
		case DIFF_EASY:
			return(IsEnabledOnEasy);

		case DIFF_NORMAL:
			return(IsEnabledOnMedium);

		case DIFF_HARD:
			return(IsEnabledOnHard);

		default:
			return(true);
	}
}


/// <summary>
/// Is this trigger tied to the specified global variable?
/// A trigger can watch for a global variable being set or cleared. This routine checks
/// the trigger's whole event list for either kind of watch on the variable given.
/// </summary>
/// <param name="global">The global variable number to check for.</param>
/// <returns>bool; Does one of this trigger's events watch that global variable?</returns>
bool TriggerTypeClass::Is_Linked_To_Global(int global) const
{
	bool found = false;
	TEventClass * event = FirstEvent;
	while (event != NULL) {
		if ((event->Event == TEVENT_GLOBAL_SET || event->Event == TEVENT_GLOBAL_CLEAR) && event->Data.Value == global) {
			found = true;
			break;
		}
		event = event->Next;
	}
	return(found);
}


/// <summary>
/// Is this trigger tied to the specified local variable?
/// A trigger can watch for a local variable being set or cleared. This routine checks
/// the trigger's whole event list for either kind of watch on the variable given.
/// </summary>
/// <param name="local">The local variable number to check for.</param>
/// <returns>bool; Does one of this trigger's events watch that local variable?</returns>
bool TriggerTypeClass::Is_Linked_To_Local(int local) const
{
	bool found = false;
	TEventClass * event = FirstEvent;
	while (event != NULL) {
		if ((event->Event == TEVENT_LOCAL_SET || event->Event == TEVENT_LOCAL_CLEAR) && event->Data.Value == local) {
			found = true;
			break;
		}
		event = event->Next;
	}
	return(found);
}


/***********************************************************************************************
 * TriggerTypeClass::Detach -- Removes attachments to the target object specified.             *
 *                                                                                             *
 *    When an object disappears from the game, it must be detached from all other objects that *
 *    may be referring to it. This routine will detach the specified target object from any    *
 *    references to it in this trigger type class.                                             *
 *                                                                                             *
 * INPUT:   target   -- The target object to be detached from this trigger type.               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/09/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TriggerTypeClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);

	if (LinkedTo != NULL && LinkedTo == target) {
		LinkedTo = LinkedTo->LinkedTo;
	}

	TActionClass * taction = FirstAction;
	while (taction != NULL) {
		taction->Detach(target);
		taction = taction->Next;
	}
	if (FirstAction == target) {
		FirstAction = FirstAction->Next;
	}

	TEventClass * tevent = FirstEvent;
	while (tevent != NULL) {
		tevent->Detach(target);
		tevent = tevent->Next;
	}
	if (FirstEvent == target) {
		FirstEvent = FirstEvent->Next;
	}

}


/***********************************************************************************************
 * TriggerTypeClass::From_Name -- Convert an ASCII name into a trigger type pointer.           *
 *                                                                                             *
 *    Given just an ASCII representation of the trigger type, this routine will return with    *
 *    a pointer to the trigger type it refers to. Typical use of this is when parsing          *
 *    scenario INI files.                                                                      *
 *                                                                                             *
 * INPUT:   name  -- Pointer to the name to use to identify the trigger type class object to   *
 *                   be looked up.                                                             *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the trigger type class object that matches the name      *
 *          specified. If no match could be found, then NULL is returned.                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/09/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
TriggerTypeClass * TriggerTypeClass::From_Name(char const * name)
{
	if (name != NULL) {
		for (int index = 0; index < TriggerTypes.Count(); index++) {
			if (stricmp(TriggerTypes[index]->IniName, name) == 0 || stricmp(TriggerTypes[index]->GivenName, name) == 0) {
				return(TriggerTypes[index]);
			}
		}
	}
	return(NULL);
}


static char const * PersistentName[3] = {
	"Volatile",
	"Semi-persistant",
	"Persistent"
};


/// <summary>
/// Converts a persistence name into a trigger persistence type.
/// The persistence decides whether a trigger dies after it springs, dies once all of
/// the objects it is attached to have sprung it, or lives on regardless.
/// </summary>
/// <returns>Returns with the persistence type of that name. A name that is not
/// recognized is taken to mean VOLATILE.</returns>
PersistentType Persistence_From_Name(char const * name)
{
	for (int index = 0; index < ARRAY_SIZE(PersistentName); index++) {
		if (stricmp(name, PersistentName[index]) == 0) {
			return(PersistentType(index));
		}
	}
	return(VOLATILE);
}


/// <summary>
/// Fetches the name of the specified trigger persistence.
/// This routine is used when a trigger is written back out to the scenario file.
/// </summary>
/// <returns>Returns with a pointer to the name of that persistence type.</returns>
char const * Name_From_Persistence(PersistentType pers)
{
	return(PersistentName[pers]);
}


/***********************************************************************************************
 * TriggerTypeClass::Attaches_To -- Determines what trigger can attach to.                     *
 *                                                                                             *
 *    This routine will examine the trigger events and return with a composit bitfield that    *
 *    indicates what this trigger can be attached to. This is used for trigger placement       *
 *    and logic processing.                                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with AttachType bitfield representing what this trigger can be attached    *
 *          to.                                                                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/30/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
AttachType TriggerTypeClass::Attaches_To(void) const
{
	AttachType attach = ATTACH_NONE;
	TEventClass * tevent = FirstEvent;
	while (tevent != NULL) {
		attach = AttachType(attach | ::Attaches_To(tevent->Event));
		tevent = tevent->Next;
	}
	TActionClass * taction = FirstAction;
	while (taction != NULL) {
		attach = AttachType(attach | ::Attaches_To(taction->Action));
		taction = taction->Next;
	}

	assert(LinkedTo != this);
	if (LinkedTo != NULL) {
		return(AttachType(attach | LinkedTo->Attaches_To()));
	}
	return(attach);
}


/***********************************************************************************************
 * TriggerTypeClass::Read_INI -- reads triggers from the INI file                              *
 *                                                                                             *
 *    INI entry format:                                                                        *
 *      Triggername = Eventname, Actionname, Data, Housename, TeamName, IsPersistant           *
 *                                                                                             *
 * This routine reads in the triggers & creates them. Then, other classes can                  *
 * get pointers to the triggers they're linked to.                                             *
 *                                                                                             *
 * The routine relies on the TeamTypeClasses already being loaded so it can resolve            *
 * references to teams in this function.                                                       *
 *                                                                                             *
 * Cell Trigger pointers & IsTrigger flags are set in DisplayClass::Read_INI(),                *
 * and cleared in the Map::Init() routine (which clears all cell objects to 0's).              *
 *                                                                                             *
 * Object's pointers are set in:                                                               *
 *      InfantryClass::Read_INI()                                                              *
 *      BuildingClass::Read_INI()                                                              *
 *      UnitClass::Read_INI()                                                                  *
 *      TerrainClass::Read_INI()                                                               *
 * The object trigger pointers are cleared in the ObjectClass constructor.                     *
 *                                                                                             *
 * The House's EMSListOf triggers is set in this routine, and cleared in the                   *
 * HouseClass::Init() routine.                                                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      buffer      buffer to hold the INI data                                                *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      This function must be called before any other class's Read_INI.                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/28/1994 BR : Created.                                                                  *
 *=============================================================================================*/
bool TriggerTypeClass::Read_INI(CCINIClass const & ini)
{
	std::string line = ini.Get_String(INI_NAME, IniName);

	if (!line.empty()) {
		char * token = strtok(line.data(), ",");
		HousesType house = stricmp(token, "<none>") == 0 ? HOUSE_FIRST : HouseTypeClass::From_Name(token);
		if (house == HOUSE_NONE || House_From_HousesType(house) == NULL) return(false);
		House = HouseTypes[house];
		token = strtok(NULL, ",");
		LinkedTo = NULL;
		if (stricmp(token, "<none>") != 0) {
			LinkedTo = From_Name(token);
		}
		GivenName = strtok(NULL, ",");

		token = strtok(NULL, ",");
		if (token != NULL && atoi(token) == 0) {
			IsEnabled = true;
		} else {
			IsEnabled = false;
		}

		token = strtok(NULL, ",");
		if (token != NULL) {
			IsEnabledOnEasy = atoi(token) != 0;
		}

		token = strtok(NULL, ",");
		if (token != NULL) {
			IsEnabledOnMedium = atoi(token) != 0;
		}

		token = strtok(NULL, ",");
		if (token != NULL) {
			IsEnabledOnHard = atoi(token) != 0;
		}

		token = strtok(NULL, ",");
		if (token != NULL && atoi(token) != 0) {
			Set_To_Inherit(true);
		}

		line = ini.Get_String(INI_EVENT_NAME, IniName);
		if (!line.empty()) {
			int count = atoi(strtok(line.data(), ","));
			while (count) {
				TEventClass * tevent = new TEventClass;
				tevent->Read_INI();
				tevent->Next = FirstEvent;
				FirstEvent = tevent;
				count--;
			}
		}

		line = ini.Get_String(INI_ACTION_NAME, IniName);
		if (!line.empty()) {
			int count = atoi(strtok(line.data(), ","));
			FirstAction = NULL;
			TActionClass * last = NULL;

			while (count) {
				TActionClass * taction = new TActionClass;
				taction->Read_INI();
				if (FirstAction == NULL) {
					FirstAction = taction;
					last = FirstAction;
				} else {
					last->Next = taction;
					last = taction;
				}
				count--;
			}
		}

		return(true);
	}
	return(false);
}


/// <summary>
/// Reads all trigger types from the INI database specified.
/// Every name is created before any body is read so forward links can resolve. Definitions
/// whose owner is not live in the session are then discarded and the survivors renumbered.
/// </summary>
/// <param name="ini">The INI database to read the trigger types from.</param>
void TriggerTypeClass::Read_All(CCINIClass const & ini)
{
	int len = ini.Entry_Count(INI_NAME);
	for (int index = 0; index < len; index++) {
		char const * entry = ini.Get_Entry(INI_NAME, index);
		assert(entry != NULL);

		TriggerTypeClass * trigger = Find_Or_Make(entry);
		assert(trigger != NULL);
	}

	for (int index = 0; index < len; index++) {
		char const * entry = ini.Get_Entry(INI_NAME, index);
		TriggerTypeClass * trigger = From_Name(entry);
		if (trigger != NULL && !trigger->Read_INI(ini)) {
			delete trigger;
		}
	}

	for (int index = 0; index < TriggerTypes.Count(); index++) {
		TriggerTypes[index]->HeapID = index;
	}
}


/***********************************************************************************************
 * TriggerTypeClass::Write_INI -- Stores all trigger types to the INI database specified.      *
 *                                                                                             *
 *    This routine will write out all trigger type objects to the INI database. Any existing   *
 *    trigger types in the database will be cleared out.                                       *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database to have the trigger types added.            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/09/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TriggerTypeClass::Write_INI(CCINIClass & ini) const
{
	char buffer[INIClass::MAX_LINE_LENGTH];

	sprintf(buffer, "%s,%s,%s,%d,%d,%d,%d,%d",
		(House != NULL) ? (char const *)House->IniName : "<none>",
		(LinkedTo != NULL) ? (char const *)LinkedTo->IniName : "<none>",
		(char const *)GivenName,
		IsEnabled ? 0 : 1,
		IsEnabledOnEasy ? 1 : 0,
		IsEnabledOnMedium ? 1 : 0,
		IsEnabledOnHard ? 1 : 0,
		IsToInherit ? 1 : 0);

	ini.Put_String(INI_NAME, IniName, buffer);

	TEventClass const * tevent = FirstEvent;
	int count = 0;
	while (tevent != NULL) {
		count++;
		tevent = tevent->Next;
	}
	sprintf(buffer, "%d", count);

	tevent = FirstEvent;
	while (tevent != NULL) {
		strcat(buffer, ",");
		tevent->Build_INI_Entry(buffer);
		tevent = tevent->Next;
	}
	ini.Put_String(INI_EVENT_NAME, IniName, buffer);

	TActionClass const * taction = FirstAction;
	count = 0;
	while (taction != NULL) {
		count++;
		taction = taction->Next;
	}
	sprintf(buffer, "%d", count);

	taction = FirstAction;
	while (taction != NULL) {
		strcat(buffer, ",");
		taction->Build_INI_Entry(buffer);
		taction = taction->Next;
	}
	ini.Put_String(INI_ACTION_NAME, IniName, buffer);
	return(true);
}


/// <summary>
/// Stores all trigger types to the INI database specified.
/// Whatever triggers, events and actions the database already holds are cleared out
/// first, so that what remains describes only the triggers that exist right now.
/// </summary>
/// <param name="ini">The INI database to write the trigger types to.</param>
void TriggerTypeClass::Write_All(CCINIClass & ini)
{
	int index;

	/*
	**	Now write all the trigger data out
	*/
	int numtypes = ini.Entry_Count(INI_NAME);
	for (index = 0; index < numtypes; index++) {
		char const * entry = ini.Get_Entry(INI_NAME, index);
		char buffer[32];
		ini.Get_String(INI_NAME, entry, "", buffer, sizeof(buffer));
		ini.Clear(buffer);
	}
	ini.Clear(INI_NAME);

	numtypes = ini.Entry_Count(INI_EVENT_NAME);
	for (index = 0; index < numtypes; index++) {
		char const * entry = ini.Get_Entry(INI_EVENT_NAME, index);
		char buffer[32];
		ini.Get_String(INI_NAME, entry, "", buffer, sizeof(buffer));
		ini.Clear(buffer);
	}
	ini.Clear(INI_EVENT_NAME);

	numtypes = ini.Entry_Count(INI_ACTION_NAME);
	for (index = 0; index < numtypes; index++) {
		char const * entry = ini.Get_Entry(INI_ACTION_NAME, index);
		char buffer[32];
		ini.Get_String(INI_NAME, entry, "", buffer, sizeof(buffer));
		ini.Clear(buffer);
	}
	ini.Clear(INI_ACTION_NAME);

	for (index = 0; index < TriggerTypes.Count(); index++) {
		TriggerTypes[index]->Write_INI(ini);
	}
}


/// <summary>
/// Removes an action from this trigger.
/// The action is unlinked from the trigger's action list and destroyed. Handing over an
/// action that this trigger does not own is not an error -- the request is simply
/// refused.
/// </summary>
/// <param name="taction">Pointer to the action to remove. It is deleted if found.</param>
/// <returns>bool; Was the action found and removed?</returns>
bool TriggerTypeClass::Delete_Action(TActionClass const * taction)
{
	assert(taction != NULL);

	if (FirstAction == taction) {
		FirstAction = taction->Next;
		delete taction;
		return(true);
	}

	TActionClass * tptr = FirstAction;
	while (tptr != NULL) {
		if (tptr->Next == taction) {
			tptr->Next = tptr->Next->Next;
			delete taction;
			return(true);
		}
		tptr = tptr->Next;
	}
	return(false);
}

/// <summary>
/// Removes an event from this trigger.
/// The event is unlinked from the trigger's event list and destroyed. Handing over an
/// event that this trigger does not own is not an error -- the request is simply
/// refused.
/// </summary>
/// <param name="tevent">Pointer to the event to remove. It is deleted if found.</param>
/// <returns>bool; Was the event found and removed?</returns>
bool TriggerTypeClass::Delete_Event(TEventClass const * tevent)
{
	assert(tevent != NULL);

	if (FirstEvent == tevent) {
		FirstEvent = tevent->Next;
		delete tevent;
		return(true);
	}

	TEventClass * tptr = FirstEvent;
	while (tptr != NULL) {
		if (tptr->Next == tevent) {
			tptr->Next = tptr->Next->Next;
			delete tevent;
			return(true);
		}
		tptr = tptr->Next;
	}
	return(false);
}


/// <summary>
/// Fetches the trigger type of the specified name, creating it if necessary.
/// This routine is used while the scenario is being read, so that a trigger can be
/// referred to before its own declaration has been processed.
/// </summary>
/// <returns>Returns with a pointer to the trigger type that goes by that name.</returns>
TriggerTypeClass * TriggerTypeClass::Find_Or_Make(char const * name)
{
	return(TFind_Or_Make<TriggerTypeClass>(name, TriggerTypes));
}


/// <summary>
/// Adds this trigger type's data to a running checksum.
/// A trigger is defined by the objects it hangs off, so the owning house, the linked
/// trigger and the heads of the event and action lists are submitted by identifier
/// rather than by pointer.
/// </summary>
/// <param name="crc">The checksum engine to submit the data to.</param>
void TriggerTypeClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);

	crc(House->Fetch_ID());
	if (LinkedTo != NULL) crc(LinkedTo->Fetch_ID());
	if (FirstEvent != NULL) crc(FirstEvent->Fetch_ID());
	if (FirstAction != NULL) crc(FirstAction->Fetch_ID());
}


/// <summary>
/// Fetches the class identifier of this object.
/// The save game system uses this identifier to know which kind of object to build
/// when the stream is read back in.
/// </summary>
/// <param name="retval">Pointer to the location to store the class identifier.</param>
/// <returns>Returns with S_OK, or E_POINTER if no storage location was supplied.</returns>
HRESULT STDMETHODCALLTYPE TriggerTypeClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_TriggerTypeClass;
	return(S_OK);
}


/// <summary>
/// Lists the members this trigger type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TriggerTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HeapID);
	stream.Serialize(_IsEnabled);
	stream.Serialize(IsEnabledOnEasy);
	stream.Serialize(IsEnabledOnMedium);
	stream.Serialize(IsEnabledOnHard);
	stream.Serialize(IsToInherit);
	stream.Serialize(House);
	stream.Serialize(LinkedTo);
	stream.Serialize(FirstEvent);
	stream.Serialize(FirstAction);
}
