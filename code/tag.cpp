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

#include "tag.h"

#include "_map.h"
#include "cell.h"
#include "crc.h"
#include "globals.h"
#include "house.h"
#include "houstype.h"
#include "savestream.h"
#include "sun.h"
#include "swizzle.h"
#include "tagtype.h"
#include "tracker.h"
#include "trigger.h"
#include "trigtype.h"
#include "vector.h"

DynamicVectorClass<TagClass *> Tags;


/// <summary>
/// Creates a tag from the specified tag type.
/// The new tag joins the global tag list and builds a trigger for every trigger type
/// that its tag type names, so it is ready to spring the moment it is attached to
/// something.
/// </summary>
/// <param name="type">The tag type that this tag is built from.</param>
TagClass::TagClass(TagTypeClass *type) :
	BASECLASS(),
	Class(type),
	Trigger(NULL),
	AttachCount(0),
	CellID(CELL_NONE),
	IsToDie(false),
	IsCurrentlySprung(false)
{
	Tags.Add(this);
	AbstractTypePtrTracker.Add(this);
	TriggerPtrTracker.Add(this);

	if (type != NULL) {
		TriggerTypeClass *tt = type->FirstTrigger;
		while (tt != NULL) {

			TriggerClass *trigger = new TriggerClass(tt);

			trigger->LinkedTo = Trigger;
			Trigger = trigger;

			tt = tt->LinkedTo;
		}
	}
}


/// <summary>
/// Destroys this tag and severs everything that pointed at it.
/// The triggers hanging off this tag are queued for destruction, the tag is pulled from
/// the global tag list, and if it was one of the allow-win tags then its house is given
/// back the victory it was holding up.
/// </summary>
TagClass::~TagClass(void)
{
	Detach_This_From_All(this, true);

	if (GameActive && ScenarioActive &&
		Class != NULL && (Class->Attaches_To() & ATTACH_GENERAL) != 0) {
		if (LogicTriggerID >= LogicTags.ID(this)) {
			LogicTriggerID--;
			if (LogicTriggerID < 0 && LogicTags.Count() == 0) {
				LogicTriggerID = 0;
			}
		}
	}

	if (GameActive && ScenarioActive &&
		Class != NULL && (Class->Attaches_To() & ATTACH_MAP) != 0) {
		if (MapTriggerID >= MapTags.ID(this)) {
			MapTriggerID--;
			if (MapTriggerID < 0 && MapTags.Count() == 0) {
				MapTriggerID = 0;
			}
		}
	}

	if (GameActive && ScenarioActive &&
		Trigger != NULL && Trigger->Class != NULL && Trigger->Class->House != NULL && Class->Is_Allow_Win()) {
		HouseClass *hptr = House_From_HousesType(Trigger->Class->House->House);

		if (hptr->Blockage) hptr->Blockage--;
		hptr->BorrowedTime = TICKS_PER_SECOND*4;
	}
	//////////////////////////////////////////////////

	TriggerClass *trigger = Trigger;
	while (trigger != NULL) {

		ObjectsToDelete.Add(trigger);

		trigger = trigger->LinkedTo;
	}

	AbstractTypePtrTracker.Delete(this);
	TriggerPtrTracker.Delete(this);
	Tags.Delete(this);

	Detach_This_From_All(this, true);
}


/// <summary>
/// Condemns this tag to destruction.
/// The tag is not deleted on the spot; it is queued so that it goes away at a point in
/// the game logic where nothing is still walking the tag list.
/// </summary>
void TagClass::Mark_To_Delete(void)
{
	IsToDie = true;
	ObjectsToDelete.Add(this);
}


/// <summary>
/// Has this tag been condemned?
/// </summary>
/// <returns>bool; Is this tag waiting to be destroyed?</returns>
bool TagClass::Is_Marked_To_Delete(void) const
{
	return(IsToDie);
}


/// <summary>
/// Fetches the tag of the specified type, creating one if need be.
/// Use this routine when something needs a tag of a given type and does not care whether
/// the scenario has already brought one into existence.
/// </summary>
/// <param name="type">The tag type that the tag should be built from.</param>
/// <returns>Returns with a pointer to the tag of that type. It is created if none
/// existed.</returns>
TagClass * Find_Or_Make(TagTypeClass *type)
{
	for (int index = 0; index < Tags.Count(); index++) {
		TagClass *tag = Tags[index];
		if (tag->Class == type) {
			return(tag);
		}
	}

	return(new TagClass(type));
}


/// <summary>
/// Fetches the map cell that this tag rides on.
/// </summary>
/// <returns>Returns with the cell the tag is attached to. CELL_NONE means it is not on
/// the map.</returns>
Cell TagClass::Get_Position(void) const
{
	return(CellID);
}


/// <summary>
/// Is this tag watching for a vertical line crossing?
/// </summary>
/// <returns>bool; Was the tag type declared as a vertical crossing tag?</returns>
bool TagClass::Is_Cross_Vertical(void) const
{
	return(Class->Is_Cross_Vertical());
}


/// <summary>
/// Is this tag watching for a horizontal line crossing?
/// </summary>
/// <returns>bool; Was the tag type declared as a horizontal crossing tag?</returns>
bool TagClass::Is_Cross_Horizontal(void) const
{
	return(Class->Is_Cross_Horizontal());
}


/// <summary>
/// Is this tag watching for something entering its zone?
/// </summary>
/// <returns>bool; Was the tag type declared as an enters-zone tag?</returns>
bool TagClass::Is_Enters_Zone(void) const
{
	return(Class->Is_Enters_Zone());
}


/// <summary>
/// Is this one of the tags that gates the player's victory?
/// A house cannot declare victory while any of its allow-win tags is still waiting to
/// be sprung.
/// </summary>
/// <returns>bool; Does this tag hold up its house from winning?</returns>
bool TagClass::Is_Allow_Win(void) const
{
	return(Class->Is_Allow_Win());
}


/// <summary>
/// Is the specified trigger hanging off this tag?
/// </summary>
/// <returns>bool; Is the trigger a member of this tag's chain?</returns>
bool TagClass::Is_Trigger_Attached(TriggerClass *trigger) const
{
	TriggerClass * trigptr = Trigger;
	while (trigptr != NULL) {
		if (trigptr == trigger) {
			return(true);
		}
		trigptr = trigptr->LinkedTo;
	}
	return(false);
}


/// <summary>
/// Springs this tag in response to a game event.
/// Every trigger hanging off this tag is offered the event and any that recognize it
/// will fire their actions. The tag type's persistence decides what becomes of the tag
/// afterwards -- it may detach itself from whatever it was riding on and queue itself
/// for destruction, or it may stay behind to fire again.
/// </summary>
/// <param name="event">The event that has occurred.</param>
/// <param name="object">The object the event happened to. This may be NULL.</param>
/// <param name="cell">The cell the event happened at, or CELL_NONE if it was not cell
/// based.</param>
/// <param name="forced">Should the triggers fire without regard to their own event tests?</param>
/// <param name="source">The object that caused the event, if there was one.</param>
/// <returns>bool; Did any attached trigger fire?</returns>
/// <remarks>A tag will refuse to spring while it is already in the middle of springing.</remarks>
bool TagClass::Spring(TEventType event, ObjectClass * object, Cell cell, bool forced, TechnoClass *source)
{
	bool res = false; /// was handled
	bool die = false; /// to delete
	bool det = false; /// to detach

	if (IsCurrentlySprung) {
		return(false);
	}

	TriggerClass *trigger = Trigger;

	IsCurrentlySprung = true;

	while (trigger != NULL) {
		if (trigger->Should_Spring(event, object, forced, Class->Persistence == PERSISTENT, source)) {
			switch (Class->Persistence) {

				case VOLATILE:
					trigger->Spring(object, cell);
					trigger->Mark_To_Delete();
					res = true;
					die = true;
					det = true;
					break;

				case SEMIPERSISTENT:
					if (AttachCount == 1) {
						trigger->Spring(object, cell);
						trigger->Mark_To_Delete();
						die = true;
						res = true;
						break;
					}
					det = true;
					break;

				case PERSISTENT:
					trigger->Spring(object, cell);
					res = true;
					break;

				default:
					break;
			}
		}
		trigger = trigger->LinkedTo;
	}

	IsCurrentlySprung = false;

	if (det) {
		if (object != NULL && object->Tag == this) {
			object->Attach_Tag(NULL);
		}
		if (cell != CELL_NONE) {
			Map[cell].Attach_Tag(NULL);
		}
	}

	if (die) {
		Detach_This_From_All(this, true);
		ObjectsToDelete.Add(this);
	}

	return(res);
}


/// <summary>
/// Records the map cell that this tag rides on.
/// </summary>
/// <param name="cell">The cell the tag is attached to, or CELL_NONE if it rides on an
/// object.</param>
void TagClass::Set_Position(Cell cell)
{
	CellID = cell;
}


/// <summary>
/// Destroys every tag in the game.
/// This routine is called when a scenario is torn down so that no tag from the previous
/// mission survives into the next one.
/// </summary>
void TagClass::Delete_All(void)
{
	while (Tags.Count() > 0) {
		delete Tags[0];
	}
}


/// <summary>
/// Restarts this tag's elapsed time events tied to a global flag.
/// </summary>
/// <param name="global">The global flag that just changed.</param>
void TagClass::Timer_Global_Reset(int global)
{
	Trigger->Reset_Global_Linked_Timed_Events(global);
}


/// <summary>
/// Restarts this tag's elapsed time events tied to a local variable.
/// </summary>
/// <param name="local">The local variable that just changed.</param>
void TagClass::Timer_Local_Reset(int local)
{
	Trigger->Reset_Local_Linked_Timed_Events(local);
}


/// <summary>
/// Adds a trigger to this tag's trigger chain.
/// </summary>
/// <param name="trigger">The trigger to attach to this tag.</param>
void TagClass::Link(TriggerClass *trigger)
{
	trigger->LinkedTo = Trigger;
	Trigger = trigger;
}


/// <summary>
/// Removes a trigger from this tag's trigger chain.
/// The trigger itself is left intact -- only its membership in this tag is severed.
/// </summary>
/// <param name="trigger">The trigger to detach from this tag.</param>
/// <returns>bool; Was the trigger found and unlinked?</returns>
bool TagClass::Unlink(TriggerClass *trigger)
{
	if (trigger != NULL) {
		if (Trigger == trigger) {
			Trigger = trigger->LinkedTo;
			return(true);
		}
		TriggerClass *trigptr = Trigger;
		while (trigptr != NULL) {
			if (trigptr->LinkedTo == trigger) {
				trigptr->LinkedTo = trigger->LinkedTo;
				return(true);
			}
			trigptr = trigptr->LinkedTo;
		}
	}
	return(false);
}


/// <summary>
/// Removes all references to the object specified.
/// This routine is called when an object is leaving the game. Should the tag lose the
/// tag type it was built from, it marks itself to die as well, since a tag with no type
/// has nothing left to do.
/// </summary>
/// <param name="target">The object that is going away.</param>
/// <param name="all">Is the target disappearing for good rather than merely hiding?</param>
void TagClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);

	if (Class == target) {
		Class = NULL;
	}

	if (Trigger == target) {
		Trigger = Trigger->LinkedTo;
	}

	if (Class == NULL) {
		Mark_To_Delete();
	}
}


/// <summary>
/// Fetches the class identifier that this tag persists under.
/// </summary>
/// <param name="retval">Pointer to the identifier to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT STDMETHODCALLTYPE TagClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_TagClass;
	return(S_OK);
}


/// <summary>
/// Submits this tag's state to the game CRC.
/// This routine is used by the network sync check to prove that every machine agrees
/// about the tags currently in play.
/// </summary>
/// <param name="crc">The CRC engine to feed this tag's values into.</param>
void TagClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);

	crc(Class->Fetch_ID());
	if (Trigger != NULL) crc(Trigger->Fetch_ID());

	crc(IsToDie);
}


/// <summary>
/// Lists the members this tag carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TagClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Class);
	stream.Serialize(Trigger);
	stream.Serialize(AttachCount);
	stream.Serialize(CellID);
	stream.Serialize(IsToDie);
	stream.Serialize(IsCurrentlySprung);
}


/// <summary>
/// Determines if this tag may be handed over to another object.
/// A tag is transferable if any of the triggers hanging off it was marked as such by
/// its trigger type. Such a tag will be passed along to the new owner when an object
/// is captured, and to the crew that survives a destroyed vehicle, rather than being
/// lost with the object it started out on.
/// </summary>
/// <returns>bool; May this tag be handed over to another object?</returns>
bool TagClass::Is_To_Inherit(void) const
{
	TriggerClass *trigger = Trigger;
	while (trigger != NULL) {
		if (trigger->Class->Is_To_Inherit()) {
			return(true);
		}
		trigger = trigger->LinkedTo;
	}
	return(false);
}


/// <summary>
/// Restarts the elapsed time events tied to a global flag.
/// This routine is called when a global flag changes value, so that every tag in the
/// scenario gets a chance to restart the timers that were waiting on it.
/// </summary>
/// <param name="global">The global flag that just changed.</param>
void TagClass::All_Timer_Global_Reset(int global)
{
	for (int index = 0; index < Tags.Count(); index++) {
		Tags[index]->Timer_Global_Reset(global);
	}
}


/// <summary>
/// Restarts the elapsed time events tied to a local variable.
/// This routine is called when a local variable changes value, so that every tag in the
/// scenario gets a chance to restart the timers that were waiting on it.
/// </summary>
/// <param name="local">The local variable that just changed.</param>
void TagClass::All_Timer_Local_Reset(int local)
{
	for (int index = 0; index < Tags.Count(); index++) {
		Tags[index]->Timer_Local_Reset(local);
	}
}


/// <summary>
/// Is this the only tag built from its tag type?
/// This routine is used when something needs to know whether the tag type it is looking
/// at has been instantiated more than once during the scenario.
/// </summary>
/// <returns>bool; Is this the sole tag of its type?</returns>
bool TagClass::Is_One_Of_A_Kind(void)
{
	for (int i = 0; i < Tags.Count(); i++) {
		TagClass *tag = Tags[i];
		if (tag != this && tag->Class == Class) {
			return(false);
		}
	}
	return(true);
}
