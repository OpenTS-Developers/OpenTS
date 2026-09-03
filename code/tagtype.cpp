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

#include "tagtype.h"

#include "ahtoi.h"
#include "ccini.h"
#include "findmake.h"
#include "globals.h"
#include "savestream.h"
#include "sun.h"
#include "swizzle.h"
#include "tag.h"
#include "tracker.h"
#include "trigtype.h"
#include "vector.h"

char const * const TagTypeClass::INI_NAME = "Tags";

DynamicVectorClass<TagTypeClass *> TagTypes;


/// <summary>
/// Constructs a tag type of the name specified.
/// The new tag type is added to the master tag list and to the abstract type tracker,
/// so that the scenario reader and the save system can find it later.
/// </summary>
TagTypeClass::TagTypeClass(char const *name) :
	BASECLASS(name),
	HeapID(-1),
	Persistence(VOLATILE),
	FirstTrigger(NULL)
{
	GivenName = TStringID<48>(IniName);

	HeapID = TagTypes.Count();
	TagTypes.Add(this);
	AbstractTypePtrTracker.Add(this);
}


/// <summary>
/// Destroys this tag type.
/// This routine will delete the trigger types hanging off the tag as well as any tag
/// instances that were built from it, then take the tag type back out of the master
/// tag list.
/// </summary>
TagTypeClass::~TagTypeClass(void)
{
	Detach_This_From_All(this, true);

	TriggerTypeClass *trigtype = FirstTrigger;
	FirstTrigger = NULL;

	while (trigtype != NULL) {
		TriggerTypeClass *linked = trigtype->LinkedTo;
		trigtype->LinkedTo = NULL;
		delete trigtype;
		trigtype = linked;
	}

	for (int i = 0; i < Tags.Count(); i++) {
		if (Tags[i]->Class == this) {
			delete Tags[i];
			i--;
		}
	}

	AbstractTypePtrTracker.Delete(this);
	TagTypes.Delete(this);
}


/// <summary>
/// Adds a trigger type to this tag's trigger list.
/// The tag fires every trigger hanging off it, so this is how a designer gives one
/// tag more than one thing to do.
/// </summary>
/// <returns>bool; Was the trigger attached?</returns>
bool TagTypeClass::Link(TriggerTypeClass *trigger)
{
	if (trigger != NULL) {
		trigger->LinkedTo = FirstTrigger;
		FirstTrigger = trigger;
		return(true);
	}
	return(false);
}


/// <summary>
/// Removes a trigger type from this tag's trigger list.
/// This routine is used by the scenario editor when the designer breaks the tie
/// between a tag and one of its triggers.
/// </summary>
/// <returns>bool; Was the trigger found and unhooked?</returns>
bool TagTypeClass::Unlink(TriggerTypeClass *trigger)
{
	if (FirstTrigger == trigger) {
		FirstTrigger = trigger->LinkedTo;
		return(true);
	}
	TriggerTypeClass *trigtype = FirstTrigger;
	while (trigtype != NULL) {
		if (trigtype->LinkedTo == trigger) {
			trigtype->LinkedTo = trigger->LinkedTo;
			return(true);
		}
		trigtype = trigtype->LinkedTo;
	}
	return(false);
}


/// <summary>
/// Removes any reference this tag type holds to the object specified.
/// This routine is called when an object is being destroyed, so that the tag is not
/// left pointing at a corpse.
/// </summary>
void TagTypeClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);

	if (FirstTrigger == target) {
		FirstTrigger = FirstTrigger->LinkedTo;
	}
}


/// <summary>
/// Fetches the tag type that goes by the name specified.
/// Both the database name and the display name are considered, and the comparison
/// ignores case.
/// </summary>
/// <returns>Returns with a pointer to the tag type found, or NULL if there is no match.</returns>
TagTypeClass * TagTypeClass::From_Name(char const * name)
{
	if (name != NULL) {
		for (int index = 0; index < TagTypes.Count(); index++) {
			TagTypeClass *ttptr = TagTypes[index];
			if (stricmp(ttptr->IniName, name) == 0 || stricmp(ttptr->GivenName, name) == 0) {
				return(ttptr);
			}
		}
	}
	return(NULL);
}


/// <summary>
/// Reads every tag type from the INI database.
/// This routine will create a tag type for each entry in the tag section and announce
/// it to the pointer swizzler, so that the objects and cells that refer to the tag can
/// be hooked back up to it.
/// </summary>
void TagTypeClass::Read_All(CCINIClass const & ini)
{
	int len = ini.Entry_Count(INI_NAME);
	for (int index = 0; index < len; index++) {
		char const * entry = ini.Get_Entry(INI_NAME, index);
		assert(entry != NULL);
		if (ini.Get_String(INI_NAME, entry).empty()) {
			continue;
		}
		TagTypeClass * tag = Find_Or_Make(entry);
		assert(tag != NULL);

		Swizzle_Here_I_Am(ahtoi(entry), tag);
		tag->Read_INI(ini);
	}
}


/// <summary>
/// Writes every tag type out to the INI database.
/// This routine will first throw away the tag entries already in the database, so that
/// tags deleted since the last save do not linger.
/// </summary>
void TagTypeClass::Write_All(CCINIClass & ini)
{
	int numtypes = ini.Entry_Count(INI_NAME);
	int index;
	for (index = 0; index < numtypes; index++) {
		char const * entry = ini.Get_Entry(INI_NAME, index);
		char buffer[32];
		ini.Get_String(INI_NAME, entry, "", buffer, sizeof(buffer));
		ini.Clear(buffer);
	}
	ini.Clear(INI_NAME);

	for (index = 0; index < TagTypes.Count(); index++) {
		TagTypes[index]->Write_INI(ini);
	}
}


/// <summary>
/// Reads this tag type's data from the INI database.
/// This routine fetches the persistence, the display name, and the trigger type that
/// the tag fires.
/// </summary>
/// <returns>bool; Was an entry for this tag found in the database?</returns>
bool TagTypeClass::Read_INI(CCINIClass const & ini)
{
	char buffer[128];

	if (ini.Get_String(INI_NAME, IniName, "", buffer, sizeof(buffer))) {

		char * token = strtok(buffer, ",");

		Persistence = (PersistentType)atoi(buffer);

		token = strtok(NULL, ",");
		GivenName = TStringID<48>(token);

		token = strtok(NULL, ",");
		FirstTrigger = TriggerTypeClass::From_Name(token);

		return(true);
	}
	return(false);
}


/// <summary>
/// Writes this tag type out to the INI database.
/// The tag is stored as its persistence, its display name, and the trigger it is
/// linked to.
/// </summary>
bool TagTypeClass::Write_INI(CCINIClass & ini) const
{
	char buffer[128];

	if (FirstTrigger == NULL) {
		wsprintf(buffer, "%s,<none>", (char const *)GivenName);
		ini.Put_String(INI_NAME, IniName, buffer);
	} else {
		wsprintf(buffer, "%d,%s,%s", Persistence, (char const *)GivenName, (char const *)FirstTrigger->IniName);
		ini.Put_String(INI_NAME, IniName, buffer);
	}

	return(true);
}


/// <summary>
/// Fetches what this tag may be attached to.
/// This routine gathers the attachment types of every trigger hanging off the tag, so
/// that the scenario reader knows whether the tag belongs to a cell, an object, or the
/// house itself.
/// </summary>
/// <returns>Returns with the combined attachment flags of the attached triggers.</returns>
AttachType TagTypeClass::Attaches_To(void) const
{
	AttachType attach = ATTACH_NONE;
	TriggerTypeClass * trigger = FirstTrigger;
	while (trigger != NULL) {
		attach = AttachType(attach | trigger->Attaches_To());
		trigger = trigger->LinkedTo;
	}
	return(attach);
}


/// <summary>
/// Can this tag grant a victory?
/// This routine asks the triggers attached to the tag, since it is they that carry
/// the win action.
/// </summary>
/// <returns>bool; Is one of the attached triggers able to win the scenario?</returns>
bool TagTypeClass::Is_Allow_Win(void) const
{
	TriggerTypeClass * trigger = FirstTrigger;
	while (trigger != NULL) {
		if (trigger->Is_Allow_Win()) {
			return(true);
		}
		trigger = trigger->LinkedTo;
	}
	return(false);
}


/// <summary>
/// Does this tag watch for a horizontal line crossing?
/// This routine asks the triggers attached to the tag, since it is they that carry
/// the events.
/// </summary>
/// <returns>bool; Is one of the attached triggers a horizontal crossing trigger?</returns>
bool TagTypeClass::Is_Cross_Horizontal(void) const
{
	TriggerTypeClass * trigger = FirstTrigger;
	while (trigger != NULL) {
		if (trigger->Is_Cross_Horizontal()) {
			return(true);
		}
		trigger = trigger->LinkedTo;
	}
	return(false);
}


/// <summary>
/// Does this tag watch for a vertical line crossing?
/// This routine asks the triggers attached to the tag, since it is they that carry
/// the events.
/// </summary>
/// <returns>bool; Is one of the attached triggers a vertical crossing trigger?</returns>
bool TagTypeClass::Is_Cross_Vertical(void) const
{
	TriggerTypeClass * trigger = FirstTrigger;
	while (trigger != NULL) {
		if (trigger->Is_Cross_Vertical()) {
			return(true);
		}
		trigger = trigger->LinkedTo;
	}
	return(false);
}


/// <summary>
/// Does this tag watch for an object entering a zone?
/// This routine asks the triggers attached to the tag, since it is they that carry
/// the events.
/// </summary>
/// <returns>bool; Is one of the attached triggers an enters zone trigger?</returns>
bool TagTypeClass::Is_Enters_Zone(void) const
{
	TriggerTypeClass * trigger = FirstTrigger;
	while (trigger != NULL) {
		if (trigger->Is_Enters_Zone()) {
			return(true);
		}
		trigger = trigger->LinkedTo;
	}
	return(false);
}


/// <summary>
/// Is the specified trigger type attached to this tag?
/// </summary>
/// <returns>bool; Was the trigger found in this tag's trigger list?</returns>
bool TagTypeClass::Is_Linked(TriggerTypeClass *trigger) const
{
	TriggerTypeClass * trigtype = FirstTrigger;
	while (trigtype != NULL) {
		if (trigtype == trigger) {
			return(true);
		}
		trigtype = trigtype->LinkedTo;
	}
	return(false);
}


/// <summary>
/// Fetches the tag type of the name specified, creating it if need be.
/// This routine is used while reading a scenario, where a tag may be referred to
/// before the tag section itself has been processed.
/// </summary>
/// <returns>Returns with a pointer to the tag type, or NULL if one could not be made.</returns>
TagTypeClass * TagTypeClass::Find_Or_Make(char const * name)
{
	return(TFind_Or_Make<TagTypeClass>(name, TagTypes));
}


/// <summary>
/// Fetches the class identifier of this object.
/// This routine is used by the save and load system so that a tag type can be
/// recognized when it is read back out of a stream.
/// </summary>
/// <param name="retval">Pointer to the identifier to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT STDMETHODCALLTYPE TagTypeClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_TagTypeClass;
	return(S_OK);
}


/// <summary>
/// Adds this tag type's data to the CRC calculation.
/// This routine is used by the multiplayer sync checker to prove that every machine
/// holds the same tag types.
/// </summary>
void TagTypeClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);

	crc(Persistence);
}


/// <summary>
/// Lists the members this tag type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TagTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HeapID);
	stream.Serialize(Persistence);
	stream.Serialize(FirstTrigger);
}
