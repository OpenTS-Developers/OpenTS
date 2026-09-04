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

/* $Header: /CounterStrike/TRIGGER.CPP 1     3/03/97 10:26a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TRIGGER.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 11/12/94                                                     *
 *                                                                                             *
 *                  Last Update : August 13, 1996 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Find_Or_Make -- Find or create a trigger of the type specified.                           *
 *   TriggerClass::As_Target -- Converts trigger to a target value                             *
 *   TriggerClass::Attaches_To -- Determines what trigger can attach to.                       *
 *   TriggerClass::Description -- Fetch a one line ASCII description of the trigger.           *
 *   TriggerClass::Detach -- Detach specified target from this trigger.                        *
 *   TriggerClass::Draw_It -- Draws this trigger as if it were part of a list box.             *
 *   TriggerClass::Init -- clears triggers for new scenario                                    *
 *   TriggerClass::Spring -- Spring the trigger (possibly).                                    *
 *   TriggerClass::TriggerClass -- constructor                                                 *
 *   TriggerClass::operator delete -- Returns a trigger to the special memory pool.            *
 *   TriggerClass::operator new -- 'new' operator                                              *
 *   TriggerClass::~TriggerClass -- Destructor for trigger objects.                            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "trigger.h"

#include "ccrand.h"
#include "crc.h"
#include "globals.h"
#include "house.h"
#include "houstype.h"
#include "savestream.h"
#include "scenario.h"
#include "sun.h"
#include "swizzle.h"
#include "taction.h"
#include "tevent.h"
#include "tracker.h"
#include "trigtype.h"
#include "vector.h"


/***********************************************************************************************
 * TriggerClass::TriggerClass -- constructor                                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/28/1994 BR : Created.                                                                  *
 *=============================================================================================*/
TriggerClass::TriggerClass(TriggerTypeClass * trigtype) :
	Class(trigtype),
	LinkedTo(NULL),
	IsTripped(0),
	IsToDelete(false),
	IsActive(true),
	Timer(0)
{
	AbstractTypePtrTracker.Add(this);
	TriggerPtrTracker.Add(this);
	Triggers.Add(this);

	if (Class != NULL)
	{
		Reset_All_Timed_Events();
		if (!Class->Is_Enabled()) {
			Disable();
		} else if (!Class->Is_Enabled_At(Scen->Difficulty)) {
			Disable();
		}
	}
}


/***********************************************************************************************
 * TriggerClass::~TriggerClass -- Destructor for trigger objects.                              *
 *                                                                                             *
 *    This destructor will update the house blockage value if necessary. No other action need  *
 *    be performed on trigger destruction.                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
TriggerClass::~TriggerClass(void)
{
	Detach_This_From_All(this, true);
	Triggers.Delete(this);
	AbstractTypePtrTracker.Delete(this);
	TriggerPtrTracker.Delete(this);
}


/// <summary>
/// Does this trigger watch for a horizontal line being crossed?
/// The linked trigger is consulted as well, so a chain of linked triggers answers as one.
/// </summary>
/// <returns>bool; Is this a horizontal crossing trigger?</returns>
bool TriggerClass::Is_Cross_Horizontal(void) const
{
	if (LinkedTo != NULL) {
		return(Class->Is_Cross_Horizontal() || LinkedTo->Is_Cross_Horizontal());
	}
	return(Class->Is_Cross_Horizontal());
}


/// <summary>
/// Does this trigger watch for a vertical line being crossed?
/// The linked trigger is consulted as well, so a chain of linked triggers answers as one.
/// </summary>
/// <returns>bool; Is this a vertical crossing trigger?</returns>
bool TriggerClass::Is_Cross_Vertical(void) const
{
	if (LinkedTo != NULL) {
		return(Class->Is_Cross_Vertical() || LinkedTo->Is_Cross_Vertical());
	}
	return(Class->Is_Cross_Vertical());
}


/// <summary>
/// Does this trigger watch for something entering its zone?
/// The linked trigger is consulted as well, so a chain of linked triggers answers as one.
/// </summary>
/// <returns>bool; Is this an enters-zone trigger?</returns>
bool TriggerClass::Is_Enters_Zone(void) const
{
	if (LinkedTo != NULL) {
		return(Class->Is_Enters_Zone() || LinkedTo->Class->Is_Enters_Zone());
	}
	return(Class->Is_Enters_Zone());
}


/// <summary>
/// Must this trigger spring before the scenario can be won?
/// The victory check consults every allow-win trigger before it declares the mission a
/// success. The linked trigger is consulted as well, so a chain answers as one.
/// </summary>
/// <returns>bool; Is this an allow-win trigger?</returns>
bool TriggerClass::Is_Allow_Win(void) const
{
	if (LinkedTo != NULL) {
		return(Class->Is_Allow_Win() || LinkedTo->Class->Is_Allow_Win());
	}
	return(Class->Is_Allow_Win());
}


/// <summary>
/// Is this trigger controlled by the specified global variable?
/// The linked trigger is consulted as well, so a chain of linked triggers answers as one.
/// </summary>
/// <param name="global">Index of the global variable to check against.</param>
/// <returns>bool; Is this trigger tied to that global variable?</returns>
bool TriggerClass::Is_Linked_To_Global(int global) const
{
	if (LinkedTo != NULL) {
		return(Class->Is_Linked_To_Global(global) || LinkedTo->Class->Is_Linked_To_Global(global));
	}
	return(Class->Is_Linked_To_Global(global));
}


/// <summary>
/// Restarts the timed events of triggers tied to the specified global variable.
/// This routine is used when a global scenario variable changes state. The whole chain of
/// linked triggers is walked, since a linked trigger may be tied to the variable as well.
/// </summary>
/// <param name="global">Index of the global variable that changed.</param>
void TriggerClass::Reset_Global_Linked_Timed_Events(int global)
{
	TriggerClass * tp = this;
	while (tp != NULL) {
		if (tp->Class->Is_Linked_To_Global(global)) {
			tp->Reset_All_Timed_Events();
		}
		tp = tp->LinkedTo;
	}
}


/// <summary>
/// Restarts the timed events of triggers tied to the specified local variable.
/// This routine is used when a local scenario variable changes state. The whole chain of
/// linked triggers is walked, since a linked trigger may be tied to the variable as well.
/// </summary>
/// <param name="local">Index of the local variable that changed.</param>
void TriggerClass::Reset_Local_Linked_Timed_Events(int local)
{
	TriggerClass * tp = this;
	while (tp != NULL) {
		if (tp->Class->Is_Linked_To_Local(local)) {
			tp->Reset_All_Timed_Events();
		}
		tp = tp->LinkedTo;
	}
}


/// <summary>
/// Restarts this trigger's timed events.
/// This routine is used whenever a trigger is enabled or rearmed. Any time based event it
/// carries begins its countdown again from the delay the trigger type specifies.
/// </summary>
void TriggerClass::Reset_All_Timed_Events(void)
{

	int index = 0;
	TEventClass * event = Class->FirstEvent;
	while (event != NULL) {
		if (event->Event == TEVENT_TIME) {
			Timer = event->Data.Value * TICKS_PER_SECOND;
			Flag_Event_Untripped(index);
		}
		if (event->Event == TEVENT_RANDOM_TIME) {
			Timer = ((event->Data.Value/2) + Random_Pick(0, int(event->Data.Value))) * TICKS_PER_SECOND;
			Flag_Event_Untripped(index);
		}
		index++;
		event = event->Next;
	}
}


/***********************************************************************************************
 * TriggerClass::Spring -- Spring the trigger (possibly).                                      *
 *                                                                                             *
 *    This routine is called when a potential trigger even has occurred. The event is matched  *
 *    with the trigger event needed by this trigger. If the condition warrants, the trigger    *
 *    action is performed.                                                                     *
 *                                                                                             *
 * INPUT:   event    -- The event that is occurring.                                           *
 *                                                                                             *
 *          obj      -- If the trigger is attached to an object, this points to the object.    *
 *                                                                                             *
 *          cell     -- If the trigger is attached to a cell, this is the cell number.         *
 *                                                                                             *
 *          forced   -- Should the trigger be forced to execute regardless of the event?       *
 *                                                                                             *
 * OUTPUT:  bool; Was the trigger sprung?                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1996 JLB : Created.                                                                 *
 *   08/13/1996 JLB : Linked triggers supported.                                               *
 *=============================================================================================*/
bool TriggerClass::Should_Spring(TEventType event, ObjectClass * object, bool forced, bool persistent, TechnoClass * source)
{
	if (!IsEnabled || Is_Marked_To_Delete()) {
		return(false);
	}

	bool all_sprung = true;
	if (!forced) {
		TEventClass * tevent = Class->FirstEvent;
		int index = 0;
		while (tevent != NULL) {
			bool event_tripped = Is_Event_Tripped(index);

			/*
			** A latched event only helps a persistent trigger when another event
			** still needs to be satisfied. Keep this exception scoped to Event 35
			** rather than changing latch behavior for other trigger events.
			*/
			if (event_tripped &&
				tevent->Event == TEVENT_ENEMY_IN_SPOTLIGHT &&
				index == 0 &&
				tevent->Next == NULL &&
				event != TEVENT_ENEMY_IN_SPOTLIGHT) {
				event_tripped = false;
			}

			if (event_tripped || tevent->operator()(event, House_From_HousesType((HousesType)Class->House->HeapID), object, Timer, persistent, source)) {
				if (persistent) {
					if (tevent->Is_Time_Based() && tevent->Is_To_Flag_As_Tripped()) {
						Flag_Event_Tripped(index);
					}
				}
			} else {
				all_sprung = false;
			}
			index++;
			tevent = tevent->Next;
		}
	}

	if (all_sprung && persistent) {
		Reset_All_Timed_Events();
	}

	return(all_sprung);
}


/// <summary>
/// Performs this trigger's actions.
/// This routine is called once the trigger's events have all been satisfied. Every action
/// attached to the trigger type is given its chance to run.
/// </summary>
/// <param name="object">The object the trigger is attached to, if it is attached to one.</param>
/// <param name="cell">The cell the trigger is attached to, if it is attached to one.</param>
/// <returns>bool; Did any of the actions actually do something?</returns>
bool TriggerClass::Spring(ObjectClass * object, Cell cell)
{
	if (!IsEnabled || Is_Marked_To_Delete()) {
		return(false);
	}

	bool done = false;
	TActionClass * taction = Class->FirstAction;
	while (taction != NULL) {
		if (taction->operator()(House_From_HousesType((HousesType)Class->House->HeapID), object, this, cell)) {
			done = true;
		}
		taction = taction->Next;
	}

	return(done);
}


/***********************************************************************************************
 * Find_Or_Make -- Find or create a trigger of the type specified.                             *
 *                                                                                             *
 *    This routine is used when, given a trigger type, an actual trigger object is needed.     *
 *    In this case, an existing trigger of the correct type must be located, or a trigger      *
 *    object must be created. In either case, this routine will return a trigger object that   *
 *    corresponds to the trigger type class specified.                                         *
 *                                                                                             *
 * INPUT:   trigtype -- Pointer to the trigger type to find (or create) a matching trigger     *
 *                      object.                                                                *
 *                                                                                             *
 * OUTPUT:  Returns a pointer to a matching trigger object. If no more triggers could be       *
 *          allocated and no matching trigger could be found, then this routine will return    *
 *          NULL (a very rare case).                                                           *
 *                                                                                             *
 * WARNINGS:   This routine could return NULL.                                                 *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/09/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
TriggerClass * Find_Or_Make(TriggerTypeClass * trigtype)
{
	if (trigtype == NULL) return(NULL);

	for (int index = 0; index < Triggers.Count(); index++) {
		if (trigtype == Triggers[index]->Class) {
			return(Triggers[index]);
		}
	}

	/*
	**	No trigger was found, so make one.
	*/
	TriggerClass * trig = new TriggerClass(trigtype);
	return(trig);
}


/***********************************************************************************************
 * TriggerClass::Detach -- Detach specified target from this trigger.                          *
 *                                                                                             *
 *    This routine is called when the specified trigger MUST be detached from all references   *
 *    to it. The only reference maintained by a trigger is the reference to the trigger        *
 *    type class it is modeled after.                                                          *
 *                                                                                             *
 * INPUT:   target   -- The target identifier to remove all attachments to.                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   You must never detach the trigger type class from a trigger. Such a process     *
 *             will leave the trigger orphan and in a 'crash the game immediately if used'     *
 *             state. As such, this routine will throw an assertion if this is tried.          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/09/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TriggerClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);

	if (Class == target) {
		Class = NULL;
	}
	if (LinkedTo == target) {
		LinkedTo = LinkedTo->LinkedTo;
	}
}


/// <summary>
/// Records that the specified event has been satisfied.
/// This routine is used by the persistent trigger logic so that an event which has already
/// occurred does not have to occur a second time.
/// </summary>
/// <param name="event">Index of the event that was satisfied.</param>
void TriggerClass::Flag_Event_Tripped(int event)
{
	IsTripped |= (1 << event);
}


/// <summary>
/// Clears the sprung record for the specified event.
/// This routine is used when a trigger is rearmed, so that the event must be satisfied
/// all over again before the trigger will spring.
/// </summary>
/// <param name="event">Index of the event to rearm.</param>
void TriggerClass::Flag_Event_Untripped(int event)
{
	IsTripped &= ~(1 << event);
}


/// <summary>
/// Has the specified event of this trigger already been satisfied?
/// A persistent trigger remembers which of its events have fired, so that the ones still
/// outstanding can be waited on without re-testing those already met.
/// </summary>
/// <param name="event">Index of the event to examine.</param>
/// <returns>bool; Has the event already sprung?</returns>
bool TriggerClass::Is_Event_Tripped(int event)
{
	return((IsTripped & (1 << event)) != 0);
}


/// <summary>
/// Marks this trigger for deletion.
/// The trigger stops springing right away, but it is not destroyed until the deferred deletion
/// list is processed. This keeps a trigger from vanishing out from under the logic that
/// is still walking it.
/// </summary>
void TriggerClass::Mark_To_Delete(void)
{
	IsToDelete = true;
	ObjectsToDelete.Add(this);
}


/// <summary>
/// Has this trigger been marked for deletion?
/// </summary>
/// <returns>bool; Is this trigger waiting on deferred deletion list?</returns>
bool TriggerClass::Is_Marked_To_Delete(void) const
{
	return(IsToDelete);
}


/// <summary>
/// Adds this trigger's state to the CRC calculation.
/// This routine is used by the network sync check to prove that every machine in the game
/// agrees about the triggers currently in play.
/// </summary>
/// <param name="crc">The CRC engine to feed this trigger's values into.</param>
void TriggerClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);

	crc(Class->Fetch_ID());
	if (LinkedTo != NULL) crc(LinkedTo->Fetch_ID());

	crc(IsToDelete);
	crc(IsActive);

	crc((int)Timer);
	crc(IsTripped);
}


/// <summary>
/// Fetches the class identifier of this object.
/// This routine is part of the persistence support. The save game system uses the
/// identifier to work out what kind of object to build when the stream is read back in.
/// </summary>
/// <param name="retval">Pointer to the identifier to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT STDMETHODCALLTYPE TriggerClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_TriggerClass;
	return(S_OK);
}


/// <summary>
/// Lists the members this trigger carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TriggerClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Class);
	stream.Serialize(LinkedTo);
	stream.Serialize(IsToDelete);
	stream.Serialize(Timer);
	stream.Serialize(IsTripped);
	stream.Serialize(IsActive);
}


/// <summary>
/// Enables this trigger.
/// The trigger's timed events are restarted as it comes on, so a trigger switched on part
/// way through a scenario counts its delays from that moment rather than from the start.
/// </summary>
void TriggerClass::Enable(void)
{
	IsActive = true;
	Reset_All_Timed_Events();
}


/// <summary>
/// Disables this trigger.
/// A disabled trigger ignores every event that comes its way. Use this routine when a
/// trigger must be held in reserve until the scenario decides to switch it on.
/// </summary>
void TriggerClass::Disable(void)
{
	IsActive = false;
}
