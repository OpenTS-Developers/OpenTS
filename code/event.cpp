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

/* $Header: /CounterStrike/EVENT.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : EVENT.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 12/09/94                                                     *
 *                                                                                             *
 *                  Last Update : November 10, 1995 [BRR]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   EventClass::EventClass -- Construct an id and cell based event.                           *
 *   EventClass::EventClass -- Construct simple target type event.                             *
 *   EventClass::EventClass -- Constructor for mission change events.                          *
 *   EventClass::EventClass -- Constructor for navigation computer events.                     *
 *   EventClass::EventClass -- Constructor for object types affecting cells event.             *
 *   EventClass::EventClass -- Constructor for sidebar build events.                           *
 *   EventClass::EventClass -- Constructs event to transfer special flags.                     *
 *   EventClass::EventClass -- Default constructor for event objects.                          *
 *   EventClass::EventClass -- Event for sequencing animations.                                *
 *   EventClass::EventClass -- Megamission assigned to unit.                                   *
 *   EventClass::Execute -- Execute a queued command.                                          *
 *   EventClass::EventClass -- construct a variable-sized event                                *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "event.h"

#include "_map.h"
#include "_rules.h"
#include "anim.h"
#include "building.h"
#include "cell.h"
#include "conquer.h"
#include "coord.h"
#include "data.h"
#include "dbgprint.h"
#include "display.h"
#include "factory.h"
#include "foot.h"
#include "goptions.h"
#include "house.h"
#include "ipxmgr.h"
#include "language/language.h"
#include "mouse.h"
#include "nettiming.h"
#include "overtype.h"
#include "rules.h"
#include "savemgr.h"
#include "scenario.h"
#include "session.h"
#include "stats.h"
#include "super.h"
#include "suprtype.h"
#include "surface.h"
#include "techno.h"
#include "unit.h"
#include "unittype.h"

#include "color.hh"
#include "house.hh"
#include "ramp.hh"
#include "special.hh"

#include <array>
#include <bitset>
#include <cstdint>
#include <limits>


namespace {
	using Reason = EventClass::RejectReason;

	char const * const RejectReasonNames[] = {
		"invalid type",
		"invalid origin",
		"missing origin",
		"invalid ally house",
		"invalid animation type",
		"invalid animation owner",
		"invalid production selector",
		"invalid super weapon",
		"invalid mission",
		"invalid game speed",
		"invalid removed house",
		"invalid latency fudge",
		"unauthorized subject",
		"unauthorized timing",
		"invalid timing arithmetic",
		"invalid timing values",
		"unschedulable timing",
		"invalid network report",
		"invalid abandon count",
		"event no client sends",
		"removal of a connected player",
		"placement of a non-building",
		"placement off the map",
		"placement away from the base",
		"placement under the shroud",
		"production without its requirements",
		"sale the sell cursor refuses",
		"repair the repair cursor refuses",
		"power toggle the power cursor refuses",
		"sale of a wall the sender does not own",
		"mission no player order gives",
		"order against a hidden object",
		"superweapon target out of range",
		"alliance the match does not allow",
		"invalid grant house",
		"control grant the match does not allow",
	};

	static_assert(ARRAY_SIZE(RejectReasonNames) == (int)Reason::Count);
	unsigned int RejectCounts[(unsigned int)Reason::Count] = {};

	// The reasons each player has already been named for during this match.
	std::array<std::bitset<(std::size_t)Reason::Count>, HOUSE_MAX> AnnouncedRejections;


	// An unmodified game cannot send these. Reasons left out can also follow a change to the
	// match between a player's click and the command running.
	bool Is_Announced(Reason reason)
	{
		switch (reason) {
			case Reason::InvalidAllyHouse:
			case Reason::InvalidAnimationType:
			case Reason::InvalidAnimationOwner:
			case Reason::InvalidProductionSelector:
			case Reason::InvalidSuperWeapon:
			case Reason::InvalidMission:
			case Reason::InvalidGameSpeed:
			case Reason::InvalidRemovedHouse:
			case Reason::InvalidLatencyFudge:
			case Reason::InvalidAbandonCount:
			case Reason::UnsentEvent:
			case Reason::PlacementNotBuilding:
			case Reason::PlacementOffMap:
			case Reason::PowerToggleNotAllowed:
			case Reason::WallNotOwned:
			case Reason::MissionNotAllowed:
			case Reason::AllianceNotAllowed:
			case Reason::InvalidGrantHouse:
				return(true);

			default:
				return(false);
		}
	}


	/// <summary>
	/// Tells every player in a network game who sent a command that an unmodified game cannot
	/// send. Each player is named at most once per reason in a match.
	/// </summary>
	void Announce_Rejection(Reason reason, int origin)
	{
		if (!Session.Is_Network_Game() || !Is_Announced(reason) || origin < 0 || origin >= HOUSE_MAX
			|| origin >= Houses.Count() || Houses[origin] == NULL) {
			return;
		}

		std::bitset<(std::size_t)Reason::Count> & announced = AnnouncedRejections[origin];
		if (announced.test((std::size_t)reason)) {
			return;
		}
		announced.set((std::size_t)reason);

		HouseClass const * house = Houses[origin];
		char const * format = Fetch_String(TXT_ILLEGAL_COMMAND);
		if (format == NULL || format[0] == '\0') {
			return;
		}

		char text[256];
		snprintf(text, sizeof(text), format, Session.Shown_Name(house).c_str());
		Session.Messages.Add_Message(NULL, 0, text, house->Scheme,
			TextPrintType(TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW), Rule->MessageDelay * TICKS_PER_MINUTE);
	}


	// Abandon_Production takes this in place of a heap index to mean whatever its factory
	// currently holds.
	constexpr int PRODUCTION_ID_NONE = -1;


	bool Production_Type_Is_Valid(RTTIType type)
	{
		switch (type) {
			case RTTI_UNIT:
			case RTTI_UNITTYPE:
			case RTTI_INFANTRY:
			case RTTI_INFANTRYTYPE:
			case RTTI_BUILDING:
			case RTTI_BUILDINGTYPE:
			case RTTI_AIRCRAFT:
			case RTTI_AIRCRAFTTYPE:
				return(true);

			default:
				return(false);
		}
	}


	/// <summary>Accepts a type and heap index that resolve to an object type, or the abandon sentinel with a buildable type.</summary>
	bool Production_Selector_Is_Valid(RTTIType type, int id, bool allow_sentinel)
	{
		if (allow_sentinel && id == PRODUCTION_ID_NONE) {
			return(Production_Type_Is_Valid(type));
		}
		return(Fetch_Techno_Type(type, id) != NULL);
	}
}


/// <summary>
/// Records that this event was refused, and in a network game names its sender to every
/// player when an unmodified game cannot send such an event.
/// </summary>
void EventClass::Reject(RejectReason reason, int detail) const
{
	Announce_Rejection(reason, ID);

	unsigned int const reason_index = (unsigned int)reason;
	unsigned int const count = ++RejectCounts[reason_index];
	if (count == 0 || (count & (count - 1)) != 0) {
		return;
	}

	DebugString("Rejected network event: %s, type %u, origin %d, detail %d (count %u)\n",
		RejectReasonNames[reason_index], (unsigned int)Type, ID, detail, count);
}


/// <summary>
/// Forgets which players have been named for refused events. Call once before each match starts.
/// </summary>
void EventClass::Reset_Announcements(void)
{
	for (std::bitset<(std::size_t)EventClass::RejectReason::Count> & announced : AnnouncedRejections) {
		announced.reset();
	}
}


/***********************************************************************************************
 * EventClass::EventClass -- Constructs event to transfer special flags.                       *
 *                                                                                             *
 *    This constructs an event that will transfer the special flags.                           *
 *                                                                                             *
 * INPUT:   data  -- The special flags to be transported to all linked computers.              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
EventClass::EventClass(int index, SpecialClass data) : EventClass()
{
	if (index >= 0) {
		ID = index;
		Type = SPECIAL;
		Data.Options.Data = data;
		Frame = ::Frame;
	} else {
		ID = -1;
		Type = EMPTY;
		Frame = ::Frame;
	}
}


/***********************************************************************************************
 * EventClass::EventClass -- Construct simple target type event.                               *
 *                                                                                             *
 *    This will construct a generic event that needs only a target parameter. The actual       *
 *    event and target values are specified as parameters.                                     *
 *                                                                                             *
 * INPUT:   type  -- The event type to construct.                                              *
 *                                                                                             *
 *          target-- The target value that this event is to apply to.                          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
EventClass::EventClass(int index, unsigned char type, TargetClass target) : EventClass()
{
#ifdef _DEBUG
	DebugString("Adding event %s\n", EventNames[type]);
#endif

	if (index >= 0) {
		ID = index;
		Type = type;
		Data.Target.Whom = target;
		Frame = ::Frame;
	} else {
		ID = -1;
		Type = EMPTY;
		Frame = ::Frame;
	}
}


/// <summary>
/// Constructs a simple cell type event.
/// This will construct a generic event that needs only a cell parameter. The actual
/// event and cell values are specified as parameters.
/// </summary>
/// <param name="index">The house index of the player originating this event.</param>
/// <param name="type">The event type to construct.</param>
/// <param name="cell">The cell that this event is to apply to.</param>
EventClass::EventClass(int index, unsigned char type, Cell const & cell) : EventClass()
{
	DebugString("Adding event %s\n", EventNames[type]);

	if (index >= 0) {
		ID = index;
		Type = type;
		Data.SellCell.Where = cell;
		Frame = ::Frame;
	} else {
		ID = -1;
		Type = EMPTY;
		Frame = ::Frame;
	}
}


/***********************************************************************************************
 * EventClass::EventClass -- Default constructor for event objects.                            *
 *                                                                                             *
 *    This constructs a simple event object that requires no parameters other than the         *
 *    type of event it is.                                                                     *
 *                                                                                             *
 * INPUT:   type  -- The type of event to construct.                                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/27/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
EventClass::EventClass(int index, unsigned char type) : EventClass()
{
	DebugString("Adding event %s\n", EventNames[type]);

	if (index >= 0) {
		ID = index;
		Type = type;
		Frame = ::Frame;
	} else {
		ID = -1;
		Type = EMPTY;
		Frame = ::Frame;
	}
}


/***********************************************************************************************
 * EventClass::EventClass -- Constructor for general-purpose-data events.                      *
 *                                                                                             *
 * INPUT:   type  -- The type of event to construct.                                           *
 *            val   -- data value                                                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/27/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
EventClass::EventClass(int index, unsigned char type, int val) : EventClass()
{
	DebugString("Adding event %s\n", EventNames[type]);
	if (index >= 0) {
		ID = index;
		Type = type;
		Data.General.Value = val;
		Frame = ::Frame;
	} else {
		ID = -1;
		Type = EMPTY;
		Frame = ::Frame;

	}
}


/***********************************************************************************************
 * EventClass::EventClass -- Constructor for navigation computer events.                       *
 *                                                                                             *
 *    Constructor for events that are used to assign the navigation computer.                  *
 *                                                                                             *
 * INPUT:   type     -- The type of event (this constructor can be used by other navigation    *
 *                      type events).                                                          *
 *                                                                                             *
 *          src      -- The object that the event should apply to.                             *
 *                                                                                             *
 *          dest     -- The destination (or target) that the event needs to complete.          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/27/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
EventClass::EventClass(int index, unsigned char type, TargetClass src, TargetClass dest) : EventClass()
{
	DebugString("Adding event %s\n", EventNames[type]);

	if (index >= 0) {
		ID = index;
		Type = type;
		Data.NavCom.Whom = src;
		Data.NavCom.Where = dest;
		Frame = ::Frame;
	} else {
		ID = -1;
		Type = EMPTY;
		Frame = ::Frame;
	}
}


/***********************************************************************************************
 * EventClass::EventClass -- Event for sequencing animations.                                  *
 *                                                                                             *
 *    This constructor is used for animations that must be created through the event system.   *
 *                                                                                             *
 * INPUT:   anim  -- The animation that will be created.                                       *
 *                                                                                             *
 *          coord -- The location where the animation is to be created.                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
EventClass::EventClass(int index, AnimType anim, HousesType owner, Coord const & coord) : EventClass()
{
	DebugString("Adding event ANIMATION\n");

	if (index >= 0) {
		ID = index;
		Type = ANIMATION;
		Data.Anim.What = anim;
		Data.Anim.Owner = owner;
		Data.Anim.Where = coord;
		Frame = ::Frame;
	} else {
		ID = -1;
		Type = EMPTY;
		Frame = ::Frame;
	}
}


/***********************************************************************************************
 * EventClass::EventClass -- Megamission assigned to unit.                                     *
 *                                                                                             *
 *    This is the event that is used to assign most missions to units. It combines both the    *
 *    mission and the target (navcom and tarcom).                                              *
 *                                                                                             *
 * INPUT:   src      -- The object that this mission is to apply to.                           *
 *                                                                                             *
 *          mission  -- The mission to assign to this object.                                  *
 *                                                                                             *
 *          target   -- The target to assign to this object's TarCom.                          *
 *                                                                                             *
 *          destination -- The destination to assign to this object's NavCom.                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
EventClass::EventClass(int index, TargetClass src, MissionType mission, TargetClass target, TargetClass destination) : EventClass()
{
	if (index >= 0) {
		ID = index;
		Type = MEGAMISSION;
		Data.MegaMission.Whom = src;
		Data.MegaMission.Mission = mission;
		Data.MegaMission.Target = target;
		Data.MegaMission.Destination = destination;
		Frame = ::Frame;
	} else {
		ID = -1;
		Type = EMPTY;
		Frame = ::Frame;
	}
}


/***********************************************************************************************
 * EventClass::EventClass -- Megamission assigned to unit.                                     *
 *                                                                                             *
 *    This is the event that is used to assign most missions to units. It combines both the    *
 *    mission and the target (navcom and tarcom).  This variation is used for formation moves. *
 *                                                                                             *
 * INPUT:   src      -- The object that this mission is to apply to.                           *
 *                                                                                             *
 *          mission  -- The mission to assign to this object.                                  *
 *                                                                                             *
 *          target   -- The target to assign to this object's TarCom.                          *
 *                                                                                             *
 *          destination -- The destination to assign to this object's NavCom.                  *
 *                                                                                             *
 *          speed    -- The formation override speed for this move.                            *
 *                                                                                             *
 *          maxspeed -- The formation override maximum speed for this move.                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
EventClass::EventClass(int index, TargetClass src, MissionType mission, TargetClass target, TargetClass destination, SpeedType speed, MPHType maxspeed) : EventClass()
{
	DebugString("Adding event MEGAMISSION_F\n");

	if (index >= 0) {
		ID = index;
		Type = MEGAMISSION_F;
		Data.MegaMission_F.Whom = src;
		Data.MegaMission_F.Mission = mission;
		Data.MegaMission_F.Target = TargetClass(target);
		Data.MegaMission_F.Destination = TargetClass(destination);
		Data.MegaMission_F.Speed = speed;
		Data.MegaMission_F.MaxSpeed = maxspeed;
		Frame = ::Frame;
	} else {
		ID = -1;
		Type = EMPTY;
		Frame = ::Frame;
	}
}


/***********************************************************************************************
 * EventClass::EventClass -- Constructor for sidebar build events.                             *
 *                                                                                             *
 *    This constructor is used for events that deal with an object type and an object ID.      *
 *    Typically, this is used exclusively by the sidebar.                                      *
 *                                                                                             *
 * INPUT:   type     -- The event type of this object.                                         *
 *                                                                                             *
 *          object   -- The object type number.                                                *
 *                                                                                             *
 *          id       -- The object sub-type number.                                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
EventClass::EventClass(int index, unsigned char type, RTTIType object, int id) : EventClass()
{
	DebugString("Adding event %s\n", EventNames[type]);

	if (index >= 0) {
		ID = index;
		Type = type;
		Data.Specific.Type = object;
		Data.Specific.ID = id;
		Frame = ::Frame;
	} else {
		ID = -1;
		Type = EMPTY;
		Frame = ::Frame;
	}
}


/// <summary>
/// Builds an event that abandons count items of one type, the queued copies first.
/// </summary>
EventClass::EventClass(int index, unsigned char type, RTTIType object, int id, int count) : EventClass()
{
	DebugString("Adding event %s\n", EventNames[type]);

	if (index >= 0) {
		ID = index;
		Type = type;
		Data.AbandonCount.Type = object;
		Data.AbandonCount.ID = id;
		Data.AbandonCount.Count = count;
		Frame = ::Frame;
	} else {
		ID = -1;
		Type = EMPTY;
		Frame = ::Frame;
	}
}


/***********************************************************************************************
 * EventClass::EventClass -- Constructor for object types affecting cells event.               *
 *                                                                                             *
 *    This constructor is used for those events that have an object type and associated cell.  *
 *    Typically, this is for building placement after construction has completed.              *
 *                                                                                             *
 * INPUT:   type     -- The event type for this object.                                        *
 *                                                                                             *
 *          object   -- The object type number (actual object is probably inferred from the    *
 *                      sidebar data).                                                         *
 *                                                                                             *
 *          cell     -- The cell location where this event is to occur.                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
EventClass::EventClass(int index, unsigned char type, RTTIType object, Cell const & cell) : EventClass()
{
	DebugString("Adding event %s\n", EventNames[type]);

	if (index >= 0) {
		ID = index;
		Type = type;
		Data.Place.Type = object;
		Data.Place.Where = cell;
		Frame = ::Frame;
	} else {
		ID = -1;
		Type = EMPTY;
		Frame = ::Frame;
	}
}


/***********************************************************************************************
 * EventClass::EventClass -- Construct an id and cell based event.                             *
 *                                                                                             *
 *    This constructor is used for those events that require an ID number and a cell location. *
 *                                                                                             *
 * INPUT:   type  -- The event type this will be.                                              *
 *                                                                                             *
 *          id    -- The arbitrary id number to assign.                                        *
 *                                                                                             *
 *          cell  -- The location for this event.                                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
EventClass::EventClass(int index, unsigned char type, int id, Cell const & cell) : EventClass()
{
	DebugString("Adding event %s\n", EventNames[type]);

	if (index >= 0) {
		ID = index;
		Type = type;
		Data.Special.ID 	= id;
		Data.Special.Where = cell;
		Frame = ::Frame;
	} else {
		ID = -1;
		Type = EMPTY;
		Frame = ::Frame;
	}
}


/***********************************************************************************************
 * EventClass::EventClass -- construct a variable-sized event                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      ptr      ptr to data associated with this event                                        *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/10/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
EventClass::EventClass(int index, unsigned char type, void * ptr, unsigned int size) : EventClass()
{
	DebugString("Adding event %s\n", EventNames[type]);

	if (index >= 0) {
		ID = index;
		Type = type;
		Data.Variable.Pointer = ptr;
		Data.Variable.Size = size;
		Frame = ::Frame;
	} else {
		ID = -1;
		Type = EMPTY;
		Frame = ::Frame;
	}
}


/***********************************************************************************************
 * EventClass::Execute -- Execute a queued command.                                            *
 *                                                                                             *
 *    This routine executes an event. The even must already have been confirmed by any         *
 *    remote machine before calling this routine.                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/27/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void EventClass::Execute(void)
{
	if (!Is_Admitted()) {
		return;
	}

	switch (Type) {
		case POWERON:
		case POWEROFF:      Execute_POWER(); break;
		case ARCHIVE:       Execute_ARCHIVE(); break;
		case ALLY:          Execute_ALLY(); break;
		case DESTRUCT:      Execute_DESTRUCT(); break;
		case SPECIAL:       Execute_SPECIAL(); break;
		case REPAIR:        Execute_REPAIR(); break;
		case SELL:          Execute_SELL(); break;
		case SELLCELL:      Execute_SELLCELL(); break;
		case ANIMATION:     Execute_ANIMATION(); break;
		case PLACE:         Execute_PLACE(); break;
		case PRODUCE:       Execute_PRODUCE(); break;
		case SUSPEND:       Execute_SUSPEND(); break;
		case ABANDON:       Execute_ABANDON(); break;
		case ABANDON_COUNT: Execute_ABANDON_COUNT(); break;
		case PRIMARY:       Execute_PRIMARY(); break;
		case MEGAMISSION_F:
		case MEGAMISSION:   Execute_MEGAMISSION(); break;
		case IDLE:          Execute_IDLE(); break;
		case DEPLOY:        Execute_DEPLOY(); break;
		case SCATTER:       Execute_SCATTER(); break;
		case SPECIAL_PLACE: Execute_SPECIAL_PLACE(); break;
		case EXIT:          Execute_EXIT(); break;
		case OPTIONS:       Execute_OPTIONS(); break;
		case GAMESPEED:     Execute_GAMESPEED(); break;
		case RESPONSE_TIME: Execute_RESPONSE_TIME(); break;
		case SAVEGAME:      Execute_SAVEGAME(); break;
		case ADDPLAYER:     Execute_ADDPLAYER(); break;
		case REMOVEPLAYER:  Execute_REMOVEPLAYER(); break;
		case LATENCYFUDGE:  Execute_LATENCYFUDGE(); break;
		case TIMING:        Execute_TIMING(); break;
		case PROCESS_TIME:  Execute_PROCESS_TIME(); break;
		case NETWORK_REPORT: Execute_NETWORK_REPORT(); break;
		case GRANT_CONTROL: Execute_GRANT_CONTROL(); break;
		case SELECTED:      Execute_SELECTED(); break;
		default:            break;
	}
}


/// <summary>
/// Applies the checks every event must pass before it runs: a known type, an existing sender
/// and, in a network game, the sender's ownership of the object it commands, or for an order
/// a shared control grant from its owner.
/// </summary>
bool EventClass::Is_Admitted(void) const
{
	if (Type == EMPTY || Type >= LAST_EVENT) {
		Reject(RejectReason::InvalidType, Type);
		return(false);
	}
	if (ID < 0 || ID >= Houses.Count()) {
		Reject(RejectReason::InvalidOrigin, ID);
		return(false);
	}
	if (Houses[ID] == NULL) {
		Reject(RejectReason::MissingOrigin, ID);
		return(false);
	}

	if (!Session.Is_Network_Game()) {
		return(true);
	}

	SubjectRule rule = SubjectRule::NONE;
	TechnoClass * subject = Subject(rule);
	if (rule != SubjectRule::NONE) {
		if (subject == NULL || !subject->IsActive || subject->Strength <= 0) {
			return(false);
		}
		HouseClass const * owner = subject->House;
		bool const allowed = owner != NULL
			&& (owner->HeapID == ID || (rule == SubjectRule::COMMANDER && owner->Grants_Control_To(Houses[ID])));
		if (!allowed) {
			Reject(RejectReason::UnauthorizedSubject, owner != NULL ? owner->HeapID : -1);
			return(false);
		}
	}

	// No client creates these, so in a network game any copy is forged.
	if (Type == SPECIAL || Type == ANIMATION || Type == PROCESS_TIME) {
		Reject(RejectReason::UnsentEvent, Type);
		return(false);
	}
	return(true);
}


/// <summary>
/// Returns the object a gated event commands, or NULL for any other event, and sets the rule
/// its sender must meet. Orders accept a shared control grant; choosing a primary factory,
/// repair, power and sale stay with the owner.
/// </summary>
TechnoClass * EventClass::Subject(SubjectRule & rule) const
{
	switch (Type) {
		case POWERON:
		case POWEROFF:
		case REPAIR:
		case PRIMARY:
		case SELL:
		case SELECTED:
			rule = SubjectRule::OWNER;
			return(Data.Target.Whom.As_Techno());

		case IDLE:
		case DEPLOY:
		case SCATTER:
			rule = SubjectRule::COMMANDER;
			return(Data.Target.Whom.As_Techno());

		case ARCHIVE:
			rule = SubjectRule::COMMANDER;
			return(Data.NavCom.Whom.As_Techno());

		case MEGAMISSION:
		case MEGAMISSION_F:
			rule = SubjectRule::COMMANDER;
			return(Data.MegaMission.Whom.As_Techno());

		default:
			rule = SubjectRule::NONE;
			return(NULL);
	}
}


void EventClass::Execute_POWER(void)
{
	BuildingClass * building = Data.Target.Whom.As_Building();
	if (building == NULL || !building->IsActive || building->IsInLimbo || building->Class->IsFirestormWall) {
		return;
	}
	if (Session.Is_Network_Game() && !building->Can_Toggle_Power()) {
		Reject(RejectReason::PowerToggleNotAllowed, building->Class->HeapID);
		return;
	}
	if (Type == POWERON) {
		building->Turn_On();
	} else {
		building->Turn_Off();
	}
}


void EventClass::Execute_ARCHIVE(void)
{
	TechnoClass * techno = Data.NavCom.Whom.As_Techno();
	if (techno && techno->IsActive) {
		techno->ArchiveTarget = Data.NavCom.Where.As_Abstract();
	}
}


void EventClass::Execute_ALLY(void)
{
	HouseClass * house = Houses[ID];
	int const index = Data.General.Value;
	if (index < 0 || index >= Houses.Count() || Houses[index] == NULL) {
		Reject(RejectReason::InvalidAllyHouse, index);
		return;
	}
	if (Session.Is_Network_Game() && !house->Can_Change_Alliance(Houses[index])) {
		Reject(RejectReason::AllianceNotAllowed, index);
		return;
	}
	if (house->Is_Ally(Houses[index])) {
		house->Make_Enemy((HousesType)index);
	} else {
		house->Make_Ally((HousesType)index);
	}
}


void EventClass::Execute_DESTRUCT(void)
{
	Houses[ID]->Flag_To_Die();
}


/// <summary>
/// Replaces the special game rules on every machine and tells the players they changed.
/// </summary>
void EventClass::Execute_SPECIAL(void)
{
	HouseClass * house = Houses[ID];
	Special = Data.Options.Data;
	Scen->Special = Data.Options.Data;

	char txt[80];
	sprintf(txt, Fetch_String(TXT_SPECIAL_WARNING), Session.Shown_Name(house).c_str());
	Session.Messages.Add_Message(NULL, 0, txt,
		house->Scheme,
		TextPrintType(TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW), 1200);
	Map.Flag_To_Redraw();
}


void EventClass::Execute_REPAIR(void)
{
	TechnoClass * techno = Data.Target.Whom.As_Techno();
	if (techno == NULL || !techno->IsActive) {
		return;
	}
	// Stopping a repair that is under way is always allowed.
	if (Session.Is_Network_Game() && (techno->RTTI != RTTI_BUILDING || (!((BuildingClass *)techno)->IsRepairing && !techno->Can_Repair()))) {
		Reject(RejectReason::RepairNotAllowed, techno->RTTI);
		return;
	}
	techno->Repair(-1);
}


void EventClass::Execute_SELL(void)
{
	TechnoClass * techno = Data.Target.Whom.As_Techno();
	if (techno == NULL || !techno->IsActive || techno->House->HeapID != ID) {
		return;
	}
	if (Session.Is_Network_Game()) {
		BuildingClass const * building = techno->RTTI == RTTI_BUILDING ? (BuildingClass const *)techno : NULL;
		// A move order to a building that packs up into a vehicle is sent as a sale.
		bool const packs_up = building != NULL && building->Class->UndeploysInto != NULL && building->Can_Redeploy();
		if (!techno->Can_Demolish() && !packs_up) {
			Reject(RejectReason::SaleNotAllowed, techno->RTTI);
			return;
		}
	}
	if (techno->RTTI == RTTI_BUILDING || ((techno->RTTI == RTTI_UNIT || techno->RTTI == RTTI_AIRCRAFT) && Map[techno->Center_Coord()].Cell_Building() != NULL)) {
		techno->Sell_Back(-1);
	}
}


void EventClass::Execute_SELLCELL(void)
{
	HouseClass * house = Houses[ID];
	Cell const cell(Data.SellCell.Where.X, Data.SellCell.Where.Y);
	// A cell without a wall passes, because selling it does nothing.
	bool const wall = Map.In_Radar(cell) && Map[cell].Overlay != OVERLAY_NONE && OverlayTypes[Map[cell].Overlay]->IsWall;
	if (Session.Is_Network_Game() && (!Map.In_Radar(cell) || (wall && !house->Can_Sell_Wall(cell)))) {
		Reject(RejectReason::WallNotOwned, (cell.X << 16) | (cell.Y & 0xFFFF));
		return;
	}
	house->Sell_Wall(cell, false);
}


void EventClass::Execute_ANIMATION(void)
{
	int const animation_type = (int)Data.Anim.What;
	int const owner = (int)Data.Anim.Owner;
	if (animation_type != ANIM_NONE && (animation_type < 0 || animation_type >= AnimTypes.Count() || AnimTypes[animation_type] == NULL)) {
		Reject(RejectReason::InvalidAnimationType, animation_type);
		return;
	}
	if (animation_type == ANIM_NONE && Rule->MoveFlash == NULL) {
		Reject(RejectReason::InvalidAnimationType, animation_type);
		return;
	}
	if (owner != HOUSE_NONE && (owner < 0 || owner >= Houses.Count() || Houses[owner] == NULL)) {
		Reject(RejectReason::InvalidAnimationOwner, owner);
		return;
	}

	Coord coord(Data.Anim.Where.X, Data.Anim.Where.Y);
	coord.Z = Map.Get_Height_GL(coord);
	if (Map[coord].IsUnderBridge) {
		coord.Z += BRIDGE_LEPTON_HEIGHT;
	}

	AnimClass * anim = NULL;
	if (Data.Anim.What == ANIM_NONE) {
		anim = new AnimClass(Rule->MoveFlash, coord);
	} else {
		anim = new AnimClass(AnimTypes[Data.Anim.What], coord);
	}
	if (anim) {
		if (owner != HOUSE_NONE && !Houses[owner]->Is_Player_Control()) {
			anim->Draw.IsInvisible = true;
		}
	}
}


/// <summary>
/// Places the object waiting in the sender's factory for the given type: a building on the
/// given cell, or anything at its factory's exit when the cell is CELL_NONE.
/// </summary>
void EventClass::Execute_PLACE(void)
{
	HouseClass * house = Houses[ID];
	Cell const cell(Data.Place.Where.X, Data.Place.Where.Y);
	FactoryClass * factory = house->Fetch_Factory(Data.Place.Type);
	TechnoClass const * object = factory != NULL && factory->Has_Completed() ? factory->Get_Object() : NULL;

	// A placement the house cannot make at all passes, because placing it does nothing.
	if (Session.Is_Network_Game() && object != NULL && cell != CELL_NONE && cell != Cell(-1, -1)) {
		RejectReason reason = RejectReason::Count;
		if (object->RTTI != RTTI_BUILDING) {
			reason = RejectReason::PlacementNotBuilding;
		} else if (!Map.In_Radar(cell)) {
			reason = RejectReason::PlacementOffMap;
		} else if (!DisplayClass::Is_Anchored_Placement(((BuildingClass const *)object)->Class, house, cell)) {
			reason = RejectReason::PlacementTooFar;
		} else if (!DisplayClass::Is_Unshrouded_Placement(((BuildingClass const *)object)->Class, house, cell)) {
			reason = RejectReason::PlacementShrouded;
		}
		if (reason != RejectReason::Count) {
			Reject(reason, (cell.X << 16) | (cell.Y & 0xFFFF));
			return;
		}
	}
	house->Place_Object(Data.Place.Type, cell);
}


void EventClass::Execute_PRODUCE(void)
{
	HouseClass * house = Houses[ID];
	if (!Production_Selector_Is_Valid(Data.Specific.Type, Data.Specific.ID, false)) {
		Reject(RejectReason::InvalidProductionSelector, Data.Specific.ID);
		return;
	}
	if (Session.Is_Network_Game() && !house->Can_Order_Production(Fetch_Techno_Type(Data.Specific.Type, Data.Specific.ID))) {
		Reject(RejectReason::ProductionNotAllowed, Data.Specific.ID);
		return;
	}
	house->Begin_Production(Data.Specific.Type, Data.Specific.ID);
}


void EventClass::Execute_SUSPEND(void)
{
	Houses[ID]->Suspend_Production(Data.Specific.Type);
}


void EventClass::Execute_ABANDON(void)
{
	if (!Production_Selector_Is_Valid(Data.Specific.Type, Data.Specific.ID, true)) {
		Reject(RejectReason::InvalidProductionSelector, Data.Specific.ID);
		return;
	}
	Houses[ID]->Abandon_Production(Data.Specific.Type, Data.Specific.ID);
}


// A count no factory could hold is refused rather than run.
void EventClass::Execute_ABANDON_COUNT(void)
{
	if (!Production_Selector_Is_Valid(Data.AbandonCount.Type, Data.AbandonCount.ID, true)) {
		Reject(RejectReason::InvalidProductionSelector, Data.AbandonCount.ID);
		return;
	}
	if (Data.AbandonCount.Count < 1 || Data.AbandonCount.Count > Rule->MaximumQueuedObjects + 1) {
		Reject(RejectReason::InvalidAbandonCount, Data.AbandonCount.Count);
		return;
	}
	for (int abandoned = 0; abandoned < Data.AbandonCount.Count; abandoned++) {
		Houses[ID]->Abandon_Production(Data.AbandonCount.Type, Data.AbandonCount.ID);
	}
}


void EventClass::Execute_PRIMARY(void)
{
	BuildingClass * building = Data.Target.Whom.As_Building();
	if (building && building->IsActive) {
		building->Toggle_Primary();
	}
}


/// <summary>
/// Gives an object a mission with its target and destination. Most player orders arrive
/// this way. A queued move appends the destination instead of replacing the route.
/// </summary>
void EventClass::Execute_MEGAMISSION(void)
{
	HouseClass * house = Houses[ID];
	if (Data.MegaMission.Mission != MISSION_NONE && (Data.MegaMission.Mission < 0 || Data.MegaMission.Mission >= MISSION_COUNT)) {
		Reject(RejectReason::InvalidMission, Data.MegaMission.Mission);
		return;
	}
	if (Session.Is_Network_Game()) {
		if (!Mission_Is_Player_Order(Data.MegaMission.Mission)) {
			Reject(RejectReason::MissionNotAllowed, Data.MegaMission.Mission);
			return;
		}
		ObjectClass const * target = Data.MegaMission.Target.As_Object();
		ObjectClass const * destination = Data.MegaMission.Destination.As_Object();
		if ((target != NULL && !house->Can_See_Target(target)) || (destination != NULL && !house->Can_See_Target(destination))) {
			Reject(RejectReason::TargetHidden, Data.MegaMission.Mission);
			return;
		}
	}

	TechnoClass * techno = Data.MegaMission.Whom.As_Techno();
	if (techno == NULL || !techno->IsActive || techno->Strength <= 0 || techno->IsInLimbo) {
		return;
	}

	/*
	**	Fetch a pointer to the object of the mission. If there is an error with
	**	this object, such as it is dead, then bail.
	*/
	ObjectClass * object = NULL;
	if (Data.MegaMission.Target.Is_Valid()) {
		object = Data.MegaMission.Target.As_Object();
		if (object != NULL && (!object->IsActive || object->Strength == 0 || object->IsInLimbo)) {
			return;
		}
	}

	/*
	**	If the destination target is invalid because the object is dead, then
	**	bail from processing this mega mission.
	*/
	if (Data.MegaMission.Destination.Is_Valid()) {
		object = Data.MegaMission.Destination.As_Object();
		if (object != NULL && (!object->IsActive || object->Strength == 0 || object->IsInLimbo)) {
			return;
		}
	}

	if (Data.MegaMission.Mission == MISSION_ENTER && techno->CurrentMission == MISSION_ENTER && techno->Is_Foot()) {
		AbstractClass * target = Data.MegaMission.Destination.As_Abstract();
		if (target != NULL && target->RTTI == RTTI_BUILDING && techno->Contact_With_Whom() == target) {
			return;
		}
	}

	/*
	**	Break any existing tether or team contact, since it is now invalid.
	*/
	if (!techno->IsTethered) {
		techno->Transmit_Message(RADIO_OVER_OUT);
	} else {
		TechnoClass * radio = techno->Contact_With_Whom();
		if (radio != NULL && radio->IsActive) {
			BuildingClass * building = dynamic_cast<BuildingClass *>(radio); /// Note, not As_BuildingClass(), that uses Abstract to Building, this is Techno to Building
			if (building != NULL && building->Class->IsRefinery) {
				techno->Transmit_Message(RADIO_OVER_OUT);
				techno->IsTethered = false;
			}
		}
	}
	if (techno->NearbyObject != NULL) {
		techno->NearbyObject = NULL;
	}
	if (techno->Is_Foot()) {
		if (((FootClass *)techno)->Team && Data.MegaMission.Mission != MISSION_UNLOAD) {
			((FootClass *)techno)->Team->Remove((FootClass *)techno);
		}
	}

	if (object != NULL) {
		if (PlayerPtr->Is_Ally(techno)) {
			object->Clicked_As_Target();
		}
	}

	/*
	**	Test to see if the navigation target should really be queued rather
	**	than assigned to the object. This would be the case if this is a
	**	special queued move mission and there is already a valid navigation
	**	target for this unit.
	*/
	bool q = (Data.MegaMission.Mission == MISSION_QMOVE);

	techno->Assign_Mission((MissionType)Data.MegaMission.Mission);

	FootClass * foot = dynamic_cast<FootClass *>(techno); /// Note, not As_FootClass(), that uses Abstract to Foot, this is Techno to Foot
	if (foot != NULL) {
		foot->SuspendedNavCom = NULL;
	}
	techno->SuspendedTarCom = NULL;

	/*
	**	Guard area mode is handled with care. The specified target is actually
	**	assigned as the location that should be guarded. In addition, the
	**	movement destination is immediately set to this new location.
	*/
	if (Data.MegaMission.Mission == MISSION_GUARD_AREA && techno->Is_Foot()) {
		techno->Assign_Target(NULL);
		techno->Assign_Destination(Data.MegaMission.Target.As_Abstract());
		techno->ArchiveTarget = Data.MegaMission.Target.As_Abstract();
	} else {
		if (foot != NULL) {
			foot->ArchiveTarget = NULL;
		}
		if (q && foot != NULL) {
			foot->Queue_Navigation_List(Data.MegaMission.Destination.As_Abstract());
		} else {
			if (foot != NULL) {
				foot->Clear_Navigation_List();
			}
			techno->Assign_Target(Data.MegaMission.Target.As_Abstract());
			techno->Assign_Destination(Data.MegaMission.Destination.As_Abstract());
		}
	}
}


void EventClass::Execute_IDLE(void)
{
	TechnoClass * techno = Data.Target.Whom.As_Techno();
	if (techno == NULL || !techno->IsActive || techno->IsInLimbo || techno->IsTethered) {
		return;
	}

	if (techno->CurrentMission == MISSION_CONSTRUCTION || techno->CurrentMission == MISSION_DECONSTRUCTION) {
		return;
	}

	if (!techno->IsOnBridge && Map[(Coord const &)techno->PositionCoord].Ramp == RAMP_NONE && techno->Is_On_Elevation()) {
		return;
	}

	if (techno->Is_Foot()) {
		((FootClass *)techno)->NavQueue.Clear();
		((FootClass *)techno)->Clear_Navigation_List();
		((FootClass *)techno)->CurrentPath = PATH_NONE;
		((FootClass *)techno)->NextWaypoint = 0;
		((FootClass *)techno)->WaypointOffsetCell = Cell(0, 0);
		((FootClass *)techno)->WaypointTargetCell = Cell(0, 0);
	}
	techno->Transmit_Message(RADIO_OVER_OUT);
	techno->Assign_Destination(NULL);
	techno->Assign_Target(NULL);
	if (techno->RTTI != RTTI_AIRCRAFT) {
		techno->Assign_Mission(MISSION_GUARD);
		techno->Commence();
	}
}


void EventClass::Execute_DEPLOY(void)
{
	TechnoClass * techno = Data.Target.Whom.As_Techno();
	if (techno == NULL || !techno->IsActive || techno->IsInLimbo || techno->IsTethered || techno->StunDuration != 0) {
		return;
	}

	if (!techno->IsOnBridge) {
		if (!Map[(Coord const &)techno->PositionCoord].Ramp != 0 && techno->Is_On_Elevation()) {
			return;
		}
	}

	if (techno->CurrentMission == MISSION_CONSTRUCTION || techno->CurrentMission == MISSION_DECONSTRUCTION || techno->RTTI == RTTI_AIRCRAFT) {
		return;
	}

	bool should_unload = false;
	Cell cell = techno->PositionCell;
	if (cell == CELL_NONE) {
		should_unload = true;
	} else {
		BuildingClass * building = Map[cell].Cell_Building();
		if (building == NULL || !building->Class->IsWeaponsFactory) {
			should_unload = true;
		}
	}

	if (should_unload) {
		techno->Transmit_Message(RADIO_OVER_OUT);
		techno->Assign_Destination(NULL);
		techno->Assign_Target(NULL);
		techno->Assign_Mission(MISSION_UNLOAD);
	}
}


void EventClass::Execute_SCATTER(void)
{
	TechnoClass * techno = Data.Target.Whom.As_Techno();
	if (techno != NULL && techno->Is_Foot() && techno->IsActive && !techno->IsInLimbo && !techno->IsTethered) {
		((FootClass *)techno)->IsScattering = true;
		techno->Scatter(COORD_NONE, true, false);
	}
}


void EventClass::Execute_SPECIAL_PLACE(void)
{
	HouseClass * house = Houses[ID];
	Cell const cell(Data.Special.Where.X, Data.Special.Where.Y);
	if (Data.Special.ID < 0 || Data.Special.ID >= house->SuperWeapon.Count()) {
		Reject(RejectReason::InvalidSuperWeapon, Data.Special.ID);
		return;
	}
	// Only a click on the in-range cursor sends a strike for a weapon with Action=EMPulse.
	SuperClass const * super = house->SuperWeapon[Data.Special.ID];
	if (Session.Is_Network_Game() && super->Class->Action == ACTION_EMPULSE && !super->Is_In_Range(cell)) {
		Reject(RejectReason::SuperWeaponOutOfRange, Data.Special.ID);
		return;
	}
	house->Place_Special_Blast((SuperWeaponType)Data.Special.ID, cell);
}


void EventClass::Execute_EXIT(void)
{
	PlayerAborts = true;
}


void EventClass::Execute_OPTIONS(void)
{
	SpecialDialog = SDLG_OPTIONS;
}


void EventClass::Execute_GAMESPEED(void)
{
	if (Data.General.Value < 0 || Data.General.Value >= OptionsClass::MAX_SPEED_SETTING) {
		Reject(RejectReason::InvalidGameSpeed, Data.General.Value);
		return;
	}
	Session.Options.GameSpeed = Data.General.Value;
	Announce_Setting_Change(TXT_PLAYER_CHANGED_SPEED);
}


void EventClass::Execute_RESPONSE_TIME(void)
{
	int const master = Session.Master_Player_ID();
	if (!NetTiming::Timing_Authority_Is_Valid(ID, master)) {
		Reject(RejectReason::UnauthorizedTiming, master);
		return;
	}
	if (!NetTiming::Response_Time_Is_Valid(Data.FrameInfo.Delay, NETWORK_MIN_MAX_AHEAD, Session.FrameSendRate,
		Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP)) {
		Reject(RejectReason::InvalidTimingValues, Data.FrameInfo.Delay);
		return;
	}
	Session.Apply_Network_Response_Time(Data.FrameInfo.Delay, Frame >= 0 ? static_cast<unsigned int>(Frame) : 0u);
}


void EventClass::Execute_SAVEGAME(void)
{
	SaveManager.Request_Multiplayer_Save(Fetch_String(TXT_MULTIPLAYER_GAME), false,
		SaveManagerClass::NoticeType::Requested);
}


// Every machine but the sender's owns a copy of the joining player's data.
void EventClass::Execute_ADDPLAYER(void)
{
	if (ID != PlayerPtr->HeapID) {
		delete [] Data.Variable.Pointer;
	}
}


/// <summary>
/// Records one player's report that another has left. The reported player is removed once
/// more than half of the other remaining players have reported it.
/// </summary>
void EventClass::Execute_REMOVEPLAYER(void)
{
	HouseClass * house = Houses[ID];
	int const index = Data.General.Value;
	if (index < 0 || index >= Houses.Count() || Houses[index] == NULL) {
		Reject(RejectReason::InvalidRemovedHouse, index);
		return;
	}
	HouseClass * target = Houses[index];
	if (!target->Is_Human_Player() || Session.Is_Player_Removed(target) || target == house) {
		return;
	}

	// The reported player ignores a sender it is still connected to, so a forged report puts
	// this machine out of sync instead of removing it.
	if (target == PlayerPtr && Ipx.Connection_Index(ID) != -1) {
		Reject(RejectReason::LiveRemoval, index);
		return;
	}

	DebugString("Executing REMOVEPLAYER event from player %d for player %d. Frame is %d\n", ID, index, ::Frame);
	Session.Report_Departure(house, target, Frame);
}


void EventClass::Execute_LATENCYFUDGE(void)
{
	if (Data.General.Value < 0 || Data.General.Value > 3) {
		Reject(RejectReason::InvalidLatencyFudge, Data.General.Value);
		return;
	}
	DebugString("Executing LATENCYFUDGE event. Frame is %d\n", ::Frame);
	Session.LatencyFudge = Data.General.Value;
	DebugString("LatencyFudge is %d\n", Session.LatencyFudge);
	Announce_Setting_Change(TXT_PLAYER_CHANGED_LATENCY);
}


/// <summary>
/// Schedules new frame rate, send period and look-ahead values from the master. Only the
/// compressed protocol sends this event.
/// </summary>
void EventClass::Execute_TIMING(void)
{
	HouseClass * house = Houses[ID];
	int const master = Session.Master_Player_ID();
	if (!NetTiming::Timing_Authority_Is_Valid(ID, master)) {
		Reject(RejectReason::UnauthorizedTiming, master);
		return;
	}

	if (Session.CommProtocol != COMM_PROTOCOL_MULTI_E_COMP || Frame < 0) {
		Reject(RejectReason::InvalidTimingArithmetic, Frame);
		return;
	}
	if (!NetTiming::Timing_Values_Are_Valid(Data.Timing.DesiredFrameRate, Data.Timing.MaxAhead, Data.Timing.FrameSendRate)) {
		Reject(RejectReason::InvalidTimingValues, Data.Timing.MaxAhead);
		return;
	}

	NetTiming::TimingSettings const settings{Data.Timing.FrameSendRate, Data.Timing.MaxAhead};
	NetTiming::ConnectionQuality const old_quality = NetTiming::Connection_Quality_For_Settings(Session.Network_Timing_Target());
	unsigned int const old_frame_send_rate = Session.FrameSendRate;
	unsigned int const old_max_ahead = Session.MaxAhead;

	if (settings.MaxAhead > old_max_ahead || settings.FrameSendRate > old_frame_send_rate) {
		std::uint64_t const boundary = settings.FrameSendRate * ((static_cast<std::uint64_t>(Frame) + NetTiming::MAXIMUM_MAX_AHEAD
			+ settings.FrameSendRate - 1) / settings.FrameSendRate);
		if (boundary > static_cast<std::uint64_t>((std::numeric_limits<int>::max)())) {
			Reject(RejectReason::InvalidTimingArithmetic, Frame);
			return;
		}
	}

	NetTiming::ScheduleResult const result = Session.Schedule_Network_Timing(settings, Data.Timing.DesiredFrameRate, static_cast<unsigned int>(Frame));
	if (result == NetTiming::ScheduleResult::Rejected) {
		Reject(RejectReason::UnschedulableTiming, static_cast<int>(settings.MaxAhead));
		return;
	}

	DebugString("Network timing event at frame %d from player %d: %u/%u at %u fps %s\n", Frame, ID, settings.FrameSendRate, settings.MaxAhead,
		Data.Timing.DesiredFrameRate, result == NetTiming::ScheduleResult::Applied ? "applied" : "staged");
	NetTiming::ConnectionQuality const quality = NetTiming::Connection_Quality_For_Settings(settings);
	if (quality != old_quality) {
		char const * format = Fetch_String(TXT_CONNECTION_QUALITY_STATUS);
		char const * quality_name = Fetch_String(Network_Quality_Text_ID(quality));
		if (format != NULL && quality_name != NULL && format[0] != '\0' && quality_name[0] != '\0') {
			char msg[256];
			snprintf(msg, sizeof(msg), format, quality_name);
			Session.Messages.Add_Message(NULL, 0, msg, house->Scheme,
				TextPrintType(TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW), Rule->MessageDelay * TICKS_PER_MINUTE);
		}
	}

#if (TIMING_FIX)
	//
	// If MaxAhead is about to increase, we're vulnerable to a Packet-
	// Received-Too-Late error, if any system generates an event after
	// this TIMING event, but before it executes.  So, record the
	// period of vulnerability's frame start & end values, so we
	// can reschedule these events to execute after it's over.
	//
	if (result == NetTiming::ScheduleResult::Applied && (Session.MaxAhead > old_max_ahead || Session.FrameSendRate > old_frame_send_rate)) {
		std::uint64_t const boundary = Session.FrameSendRate * ((static_cast<std::uint64_t>(Frame) + Session.MaxAhead
			+ Session.FrameSendRate - 1) / Session.FrameSendRate);
		NewMaxAheadFrame1 = Frame;
		NewMaxAheadFrame2 = static_cast<int>(boundary);
	} else {
		NewMaxAheadFrame1 = 0;
		NewMaxAheadFrame2 = 0;
	}
#endif
}


void EventClass::Execute_PROCESS_TIME(void)
{
	for (int i = 0; i < Session.Players.Count(); i++) {
		if (ID == Session.Players[i]->Player.ID) {
			Session.Players[i]->Player.ProcessTime = Data.ProcessTime.AverageTicks;
			break;
		}
	}
}


void EventClass::Execute_NETWORK_REPORT(void)
{
	if (Session.CommProtocol != COMM_PROTOCOL_MULTI_E_COMP || Frame < 0
		|| !Session.Record_Network_Report(ID, Data.NetworkReport.AverageProcessMilliseconds,
			Data.NetworkReport.WorstRoundTripMilliseconds, Data.NetworkReport.StallMilliseconds, static_cast<unsigned int>(Frame))) {
		Reject(RejectReason::InvalidNetworkReport, Data.NetworkReport.WorstRoundTripMilliseconds);
	} else {
		DebugString("Network report at frame %d from player %d: process %u ms, RTT %d ms, longest wait %u ms\n", Frame, ID,
			(unsigned int)Data.NetworkReport.AverageProcessMilliseconds,
			Data.NetworkReport.WorstRoundTripMilliseconds == NETWORK_RTT_UNAVAILABLE ? -1 : (int)Data.NetworkReport.WorstRoundTripMilliseconds,
			(unsigned int)Data.NetworkReport.StallMilliseconds);
	}
}


// Tells the other players that the sender changed a setting that affects everyone.
void EventClass::Announce_Setting_Change(int text) const
{
	HouseClass * house = Houses[ID];
	if (house == PlayerPtr) {
		return;
	}

	char const * str = Fetch_String(text);
	if (str != NULL && strlen(str) != 0) {
		char msg[256];
		sprintf(msg, str, Session.Shown_Name(house).c_str());
		Session.Messages.Add_Message(NULL, 0, msg, house->Scheme, TextPrintType(TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW), Rule->MessageDelay * TICKS_PER_MINUTE);
	}
}


/// <summary>
/// Grants the house the event names control of the sender's units, or withdraws the grant,
/// and tells both players. Outside a network game the event is refused, since no honest game
/// sends it there.
/// </summary>
void EventClass::Execute_GRANT_CONTROL(void)
{
	HouseClass * house = Houses[ID];
	int const index = Data.General.Value;
	if (index < 0 || index >= Houses.Count() || Houses[index] == NULL) {
		Reject(RejectReason::InvalidGrantHouse, index);
		return;
	}
	HouseClass * grantee = Houses[index];
	if (!house->Can_Grant_Control(grantee)) {
		Reject(RejectReason::ControlGrantNotAllowed, index);
		return;
	}

	bool const granted = house->Toggle_Control_Grant(grantee);
	if (PlayerPtr == house || PlayerPtr == grantee) {
		char msg[256];
		snprintf(msg, sizeof(msg), Fetch_String(granted ? TXT_CONTROL_GRANTED : TXT_CONTROL_WITHDRAWN),
			Session.Shown_Name(house).c_str(), Session.Shown_Name(grantee).c_str());
		Session.Messages.Add_Message(NULL, 0, msg, house->Scheme, TextPrintType(TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW), Rule->MessageDelay * TICKS_PER_MINUTE);
	}
}


/// <summary>
/// Springs the "selected by player" event on the tag of the object the sender selected. Nothing
/// happens when the object is gone, has no tag, or no longer belongs to the sender.
/// </summary>
void EventClass::Execute_SELECTED(void)
{
	TechnoClass * techno = Data.Target.Whom.As_Techno();
	if (techno == NULL || !techno->IsActive || techno->Strength <= 0 || techno->House != Houses[ID]) {
		return;
	}
	if (techno->Tag != NULL) {
		techno->Tag->Spring(TEVENT_SELECTED, techno);
	}
}
