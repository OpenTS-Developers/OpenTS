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
#include "foot.h"
#include "goptions.h"
#include "house.h"
#include "language\language.h"
#include "mouse.h"
#include "netsemantic.h"
#include "rules.h"
#include "saveload.h"
#include "scenario.h"
#include "session.h"
#include "stats.h"
#include "surface.h"
#include "techno.h"
#include "unit.h"
#include "unittype.h"

#include "color.hh"
#include "house.hh"
#include "ramp.hh"
#include "special.hh"

#include <cstdint>
#include <limits>


namespace {
	enum class EventRejectReason : unsigned int {
		InvalidType,
		InvalidOrigin,
		MissingOrigin,
		InvalidAllyHouse,
		InvalidAnimationType,
		InvalidAnimationOwner,
		InvalidProductionSelector,
		InvalidSuperWeapon,
		InvalidMission,
		InvalidGameSpeed,
		InvalidRemovedHouse,
		InvalidLatencyFudge,
		UnauthorizedSubject,
		UnauthorizedTiming,
		InvalidTimingArithmetic,
		InvalidTimingValues,
		UnschedulableTiming,
		InvalidNetworkReport,
		Count,
	};

	char const * const EventRejectReasonNames[] = {
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
	};

	static_assert(ARRAY_SIZE(EventRejectReasonNames) == (int)EventRejectReason::Count);
	unsigned int EventRejectCounts[(unsigned int)EventRejectReason::Count] = {};


	/// <summary>Records a rejected synchronized event.</summary>
	void Log_Event_Rejection(EventRejectReason reason, unsigned int type, int origin, int detail)
	{
		unsigned int const reason_index = (unsigned int)reason;
		unsigned int const count = ++EventRejectCounts[reason_index];
		if (count == 0 || (count & (count - 1)) != 0) {
			return;
		}

		DebugString("Rejected network event: %s, type %u, origin %d, detail %d (count %u)\n",
			EventRejectReasonNames[reason_index], type, origin, detail, count);
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


	/// <summary>Resolves the object controlled by an ownership-gated event.</summary>
	TechnoClass * Event_Subject(EventClass const & event, bool & requires_ownership)
	{
		requires_ownership = true;
		switch (event.Type) {
			case EventClass::POWERON:
			case EventClass::POWEROFF:
			case EventClass::REPAIR:
			case EventClass::PRIMARY:
			case EventClass::IDLE:
			case EventClass::DEPLOY:
			case EventClass::SCATTER:
			case EventClass::SELL:
				return(event.Data.Target.Whom.As_Techno());

			case EventClass::ARCHIVE:
				return(event.Data.NavCom.Whom.As_Techno());

			case EventClass::MEGAMISSION:
			case EventClass::MEGAMISSION_F:
				return(event.Data.MegaMission.Whom.As_Techno());

			default:
				requires_ownership = false;
				return(NULL);
		}
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
EventClass::EventClass(int index, SpecialClass data)
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
EventClass::EventClass(int index, unsigned char type, TargetClass target)
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
EventClass::EventClass(int index, unsigned char type, Cell const & cell)
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
EventClass::EventClass(int index, unsigned char type)
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
EventClass::EventClass(int index, unsigned char type, int val)
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
EventClass::EventClass(int index, unsigned char type, TargetClass src, TargetClass dest)
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
EventClass::EventClass(int index, AnimType anim, HousesType owner, Coord const & coord)
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
EventClass::EventClass(int index, TargetClass src, MissionType mission, TargetClass target, TargetClass destination)
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
EventClass::EventClass(int index, TargetClass src, MissionType mission, TargetClass target, TargetClass destination, SpeedType speed, MPHType maxspeed)
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
EventClass::EventClass(int index, unsigned char type, RTTIType object, int id)
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
EventClass::EventClass(int index, unsigned char type, RTTIType object, Cell const & cell)
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
EventClass::EventClass(int index, unsigned char type, int id, Cell const & cell)
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
EventClass::EventClass(int index, unsigned char type, void * ptr, unsigned int size)
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
	TechnoClass * techno = NULL;
	BuildingClass * building = NULL;
	AnimClass * anim = NULL;
	if (Type == EMPTY || Type >= LAST_EVENT) {
		Log_Event_Rejection(EventRejectReason::InvalidType, Type, -1, Type);
		return;
	}
	if (!NetSemantic::Index_Is_Valid(ID, Houses.Count())) {
		Log_Event_Rejection(EventRejectReason::InvalidOrigin, Type, ID, ID);
		return;
	}
	if (Houses[ID] == NULL) {
		Log_Event_Rejection(EventRejectReason::MissingOrigin, Type, ID, ID);
		return;
	}

	HouseClass * house = Houses[ID];
	if (Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) {
		bool requires_ownership = false;
		TechnoClass * subject = Event_Subject(*this, requires_ownership);
		if (requires_ownership) {
			if (subject == NULL || !subject->IsActive || subject->Strength <= 0) {
				return;
			}
			int const owner = subject->House != NULL ? subject->House->HeapID : -1;
			if (!NetSemantic::Subject_Owner_Is_Valid(ID, owner)) {
				Log_Event_Rejection(EventRejectReason::UnauthorizedSubject, Type, ID, owner);
				return;
			}
		}
	}
	HouseClass * hptr = NULL;
	const char *str = NULL;
//	Cell cell;
//	Coord coord;
	char txt[80];
	char msg[256];
//	bool formation = false;
	int i;
	int index;
//	RTTIType rt;

	//if (Debug_Print_Events) {
	//	printf("(%d) Executing %s ID:%d Frame:%d ",
	//		::Frame, EventNames[Type], ID, Frame);
	//}

	switch (Type) {

		case POWERON:
			building = Data.Target.Whom.As_Building();
			if (building && building->IsActive && !building->IsInLimbo && !building->Class->IsFirestormWall) {
				building->Turn_On();
			}
			break;

		case POWEROFF:
			building = Data.Target.Whom.As_Building();
			if (building && building->IsActive && !building->IsInLimbo && !building->Class->IsFirestormWall) {
				building->Turn_Off();
			}
			break;

		/*
		**	Update the archive target for this building.
		*/
		case ARCHIVE:
			techno = Data.NavCom.Whom.As_Techno();
			if (techno && techno->IsActive) {
				techno->ArchiveTarget = Data.NavCom.Where.As_Abstract();
			}
			break;

		/*
		**	Make or break alliance.
		*/
		case ALLY:
			index = Data.General.Value;
			if (!NetSemantic::Index_Is_Valid(index, Houses.Count()) || Houses[index] == NULL) {
				Log_Event_Rejection(EventRejectReason::InvalidAllyHouse, Type, ID, index);
				break;
			}
			hptr = Houses[index];
			if (house->Is_Ally(hptr)) {
				house->Make_Enemy((HousesType)index);
			} else {
				house->Make_Ally((HousesType)index);
			}
			break;

		/*
		**	Special self destruct action requested. This is active in the multiplayer mode.
		*/
		case DESTRUCT:
			house->Flag_To_Die();
			break;

		/*
		**	Update the special control flags. This is necessary so that in a multiplay
		**	game, all machines will agree on the rules. If these options change during
		**	game play, then all players are informed that options have changed.
		*/
		case SPECIAL:
			if (house != NULL) {
				Special = Data.Options.Data;
				Scen->Special = Data.Options.Data;

				sprintf(txt, Fetch_String(TXT_SPECIAL_WARNING), (char const *)house->IniName);
				Session.Messages.Add_Message(NULL, 0, txt,
					house->Scheme,
					TextPrintType(TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW), 1200);
				Map.Flag_To_Redraw();
			}
			break;

		/*
		**	Starts or stops repair on the specified object. This event is triggered by the
		**	player clicking the repair wrench on a building.
		*/
		case REPAIR:
			techno = Data.Target.Whom.As_Techno();
			if (techno && techno->IsActive) {
				techno->Repair(-1);
			}
			break;

		/*
		**	Tells a building/unit to sell. This event is triggered by the player clicking the
		**	sell animating cursor over the building or unit.
		*/
		case SELL:
			techno = Data.Target.Whom.As_Techno();
			if (techno && techno->IsActive && techno->House->HeapID == ID) {
				if (techno->RTTI == RTTI_BUILDING || ((techno->RTTI == RTTI_UNIT || techno->RTTI == RTTI_AIRCRAFT) && Map[techno->Center_Coord()].Cell_Building() != NULL)) {
					techno->Sell_Back(-1);
				}
			} else {
//				if (Is_Target_Cell(Data.Target.Whom)) {
//					Houses.Raw_Ptr(ID)->Sell_Wall(As_Cell(Data.Target.Whom));
//				}
			}
			break;

		/*
		**	Tells the wall at the specified location to sell off.
		*/
		case SELLCELL:
//			cell = Data.SellCell.Cell;
			house->Sell_Wall(Cell(Data.SellCell.Where.X, Data.SellCell.Where.Y), false);
			break;

		/*
		**	This even is used to trigger an animation that is generated as a direct
		**	result of player intervention.
		*/
		case ANIMATION:
		{
			int const animation_type = (int)Data.Anim.What;
			int const owner = (int)Data.Anim.Owner;
			if (!NetSemantic::Animation_Type_Is_Valid(animation_type, ANIM_NONE, AnimTypes.Count()) ||
				(animation_type != ANIM_NONE && AnimTypes[animation_type] == NULL)) {
				Log_Event_Rejection(EventRejectReason::InvalidAnimationType, Type, ID, animation_type);
				break;
			}
			if (!NetSemantic::Animation_Owner_Is_Valid(owner, HOUSE_NONE, Houses.Count()) ||
				(owner != HOUSE_NONE && Houses[owner] == NULL)) {
				Log_Event_Rejection(EventRejectReason::InvalidAnimationOwner, Type, ID, owner);
				break;
			}

			Coord coord(Data.Anim.Where.X, Data.Anim.Where.Y);
			coord.Z = Map.Get_Height_GL(coord);
			if (Map[coord].IsUnderBridge) {
				coord.Z += BRIDGE_LEPTON_HEIGHT;
			}
			if (Data.Anim.What == ANIM_NONE) {
				anim = new AnimClass(Rule->MoveFlash, coord);
			} else {
				anim = new AnimClass(AnimTypes[Data.Anim.What], coord);
			}
			if (anim) {
				if (owner != HOUSE_NONE && !Houses[owner]->Is_Player_Control()) {
					anim->Make_Invisible();
				}
			}
			break;
		}

		/*
		**	This event will place the specified object at the specified location.
		**	The event is used to place newly constructed buildings down on the map. The
		**	object type is specified. From this object type, the house can determine the
		**	exact factory and real object pointer to use.
		*/
		case PLACE:
			house->Place_Object(Data.Place.Type, Cell(Data.Place.Where.X, Data.Place.Where.Y));
			break;

		/*
		**	This event starts production of the specified object type. The house can
		**	determine from the type and ID value, what object to begin production on and
		**	what factory to use.
		*/
		case PRODUCE:
			if (!Production_Selector_Is_Valid(Data.Specific.Type, Data.Specific.ID, false)) {
				Log_Event_Rejection(EventRejectReason::InvalidProductionSelector, Type, ID, Data.Specific.ID);
				break;
			}
			house->Begin_Production(Data.Specific.Type, Data.Specific.ID);
			break;

		/*
		**	This event is generated when the player puts production on hold. From the
		**	object type, the factory can be inferred.
		*/
		case SUSPEND:
			house->Suspend_Production(Data.Specific.Type);
			break;

		/*
		**	This event is generated when the player cancels production of the specified
		**	object type. From the object type, the exact factory can be inferred.
		*/
		case ABANDON:
			if (!Production_Selector_Is_Valid(Data.Specific.Type, Data.Specific.ID, true)) {
				Log_Event_Rejection(EventRejectReason::InvalidProductionSelector, Type, ID, Data.Specific.ID);
				break;
			}
			house->Abandon_Production(Data.Specific.Type, Data.Specific.ID);
			break;

		/*
		**	Toggles the primary factory state of the specified building.
		*/
		case PRIMARY:
			{
				BuildingClass * building = Data.Target.Whom.As_Building();
				if (building && building->IsActive) {
					building->Toggle_Primary();
				}
			}
			break;

		/*
		**	This is the general purpose mission control event. Most player
		**	action routes through this event. It sets a unit's mission, TarCom,
		**	and NavCom to the values specified.
		*/
		case MEGAMISSION_F:
#if 0
			techno = Data.MegaMission_F.Whom.As_Techno();
			if (techno && techno->IsActive && techno->Is_Foot()) {
				((FootClass *)techno)->IsFormationMove = true;
				((FootClass *)techno)->FormationSpeed = Data.MegaMission_F.Speed;
				((FootClass *)techno)->FormationMaxSpeed = Data.MegaMission_F.MaxSpeed;
				FormMove = true;
				FormSpeed = Data.MegaMission_F.Speed;
				FormMaxSpeed = Data.MegaMission_F.MaxSpeed;
				formation = true;
			}
#endif
			// Fall thru to next case...

		case MEGAMISSION:
			if (!NetSemantic::Mission_Is_Valid(Data.MegaMission.Mission, MISSION_NONE, MISSION_COUNT)) {
				Log_Event_Rejection(EventRejectReason::InvalidMission, Type, ID, Data.MegaMission.Mission);
				break;
			}
			//if (Debug_Print_Events) {
			//	printf("Whom:%x Tgt:%x Dest:%x ",
			//		Data.MegaMission.Whom.As_TARGET(),
			//		Data.MegaMission.Target.As_TARGET(),
			//		Data.MegaMission.Destination.As_TARGET());
			//}
			techno = Data.MegaMission.Whom.As_Techno();
			if (techno != NULL && techno->IsActive && techno->Strength > 0 && !techno->IsInLimbo) {

				/*
				**	Fetch a pointer to the object of the mission. If there is an error with
				**	this object, such as it is dead, then bail.
				*/
				ObjectClass * object = NULL;
				if (Data.MegaMission.Target.Is_Valid()) {
					object = Data.MegaMission.Target.As_Object();
					if (object != NULL && (!object->IsActive || object->Strength == 0 || object->IsInLimbo)) {
						break;
//						object = NULL;
//						Data.MegaMission.Target.Invalidate();
					}
				}

				/*
				**	If the destination target is invalid because the object is dead, then
				**	bail from processing this mega mission.
				*/
				if (Data.MegaMission.Destination.Is_Valid()) {
					object = Data.MegaMission.Destination.As_Object();
					if (object != NULL && (!object->IsActive || object->Strength == 0 || object->IsInLimbo)) {
						break;
//						object = NULL;
//						Data.MegaMission.Destination.Invalidate();
					}
				}

				if (Data.MegaMission.Mission == MISSION_ENTER && techno->CurrentMission == MISSION_ENTER && techno->Is_Foot()) {
					AbstractClass * target = Data.MegaMission.Destination.As_Abstract();
					if (target != NULL && target->RTTI == RTTI_BUILDING && techno->Contact_With_Whom() == target) {
						break;
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
					//if (!formation) ((FootClass *)techno)->IsFormationMove = false;
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

#ifdef NEVER
				if ((techno->What_Am_I() == RTTI_UNIT || techno->What_Am_I() == RTTI_INFANTRY) &&
						Data.MegaMission.Mission == MISSION_GUARD_AREA) {

					((FootClass *)techno)->ArchiveTarget = Data.MegaMission.Destination;
				}
#endif
			}
			break;

		/*
		**	Request that the unit/infantry/aircraft go into idle mode.
		*/
		case IDLE:
			techno = Data.Target.Whom.As_Techno();
			if (techno != NULL && techno->IsActive && !techno->IsInLimbo && !techno->IsTethered) {

				if (techno->CurrentMission == MISSION_CONSTRUCTION || techno->CurrentMission == MISSION_DECONSTRUCTION) {
					break;
				}

				if (!techno->IsOnBridge && Map[(Coord const &)techno->PositionCoord].Ramp == RAMP_NONE && techno->Is_On_Elevation()) {
					break;
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
				if (techno->RTTI == RTTI_UNIT && ((UnitClass *)techno)->Class->IsToHarvest && techno->CurrentMission == MISSION_HARVEST) {
					techno->Assign_Mission(MISSION_GUARD);
					techno->Commence();
				}
			}
			break;

		case DEPLOY:
			techno = Data.Target.Whom.As_Techno();
			if (techno != NULL && techno->IsActive && !techno->IsInLimbo && !techno->IsTethered && techno->StunDuration == 0) {

				if (!techno->IsOnBridge) {
					if (!Map[(Coord const &)techno->PositionCoord].Ramp != 0 && techno->Is_On_Elevation()) {
						break;
					}
				}

				if (techno->CurrentMission == MISSION_CONSTRUCTION || techno->CurrentMission == MISSION_DECONSTRUCTION || techno->RTTI == RTTI_AIRCRAFT) {
					break;
				}

				bool should_unload = false;
				Cell cell = techno->PositionCell;
				if (cell == CELL_NONE) {
					should_unload = true;
				} else {
					building = Map[cell].Cell_Building();
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
			break;

		/*
		**	Request that the unit/infantry/aircraft scatter from its current location.
		*/
		case SCATTER:
			techno = Data.Target.Whom.As_Techno();
			if (techno != NULL && techno->Is_Foot() && techno->IsActive && !techno->IsInLimbo && !techno->IsTethered) {
				((FootClass *)techno)->IsScattering = true;
				techno->Scatter(COORD_NONE, true, false);
			}
			break;

		/*
		**	If we are placing down the ion cannon blast then lets take
		**	care of it.
		*/
		case SPECIAL_PLACE:
			if (!NetSemantic::Index_Is_Valid(Data.Special.ID, house->SuperWeapon.Count())) {
				Log_Event_Rejection(EventRejectReason::InvalidSuperWeapon, Type, ID, Data.Special.ID);
				break;
			}
			house->Place_Special_Blast((SuperWeaponType)Data.Special.ID, Cell(Data.Special.Where.X, Data.Special.Where.Y));
			break;

		/*
		**	Exit the game.
		**	Give parting message while palette is fading to black.
		*/
		case EXIT:
			//Theme.Queue_Song(THEME_NONE);
			//Stop_Speaking();
			//Speak(VOX_CONTROL_EXIT);
			//while (Is_Speaking()) {
			//	Call_Back();
			//}
			//GameActive = false;
			PlayerAborts = true;
			break;

		/*
		**	Process the options menu, unless we're playing back a recording.
		*/
		case OPTIONS:
			if (!Session.Play) {
				SpecialDialog = SDLG_OPTIONS;
			}
			break;

		/*
		**	Process the options Game Speed
		*/
		case GAMESPEED:
			if (!NetSemantic::Game_Speed_Is_Valid(Data.General.Value)) {
				Log_Event_Rejection(EventRejectReason::InvalidGameSpeed, Type, ID, Data.General.Value);
				break;
			}
			Options.GameSpeed = Data.General.Value;

			house = Houses[ID];
			if (house != PlayerPtr && house != NULL) {
				str = Fetch_String(TXT_PLAYER_CHANGED_SPEED);
				if (str != NULL && strlen(str) != 0) {
					sprintf(msg, str, house->IniName.c_str());
					Session.Messages.Add_Message(NULL, 0, msg, house->Scheme, TextPrintType(TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW), Rule->MessageDelay * TICKS_PER_MINUTE);
				}
			}
			break;

		/*
		**	Adjust connection timing for multiplayer games
		*/
		case RESPONSE_TIME:
		{
			int const master = Session.Master_Player_ID();
			if (!Session.Play && !NetSemantic::Timing_Authority_Is_Valid(ID, master)) {
				Log_Event_Rejection(EventRejectReason::UnauthorizedTiming, Type, ID, master);
				break;
			}
			if (!NetSemantic::Response_Time_Is_Valid(Data.FrameInfo.Delay, NETWORK_MIN_MAX_AHEAD, Session.FrameSendRate,
				Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP)) {
				Log_Event_Rejection(EventRejectReason::InvalidTimingValues, Type, ID, Data.FrameInfo.Delay);
				break;
			}
			Session.Apply_Network_Response_Time(Data.FrameInfo.Delay, Frame >= 0 ? static_cast<unsigned int>(Frame) : 0u);
			break;
		}

		/*
		**	Save a multiplayer game (this event is only generated in multiplayer mode)
		*/
		case SAVEGAME:
			Request_Save_Game(NET_SAVE_FILE_NAME, Fetch_String(TXT_MULTIPLAYER_GAME));
			break;

		/*
		**	Add a new player to the game:
		**	- Form a network connection to him
		**	- Add his name, ID, House etc to our list of players
		**	- Re-sort the ID array
		**	- Place his units on the map
		*/
		case ADDPLAYER:
			// int i;
			// printf("ADDPLAYER EVENT!\n");
			// for (i=0;i<Data.Variable.Size;i++) {
			// 	printf("%d\n", ((char *)Data.Variable.Pointer)[i]);
			// }
			if (ID != PlayerPtr->HeapID) {
				delete [] Data.Variable.Pointer;
			}
			break;

		case REMOVEPLAYER:
			index = Data.General.Value;
			if (!NetSemantic::Index_Is_Valid(index, Houses.Count()) || Houses[index] == NULL) {
				Log_Event_Rejection(EventRejectReason::InvalidRemovedHouse, Type, ID, index);
				break;
			}
			if (!Houses[index]->Is_Human_Player()) {
				break;
			}

			DebugString("Executing REMOVEPLAYER event. Frame is %d\n", ::Frame);
			Disable_Multiplayer_Saving();
			Session.Remove_Network_Timing_Player(index, Frame >= 0 ? static_cast<unsigned int>(Frame) : 0u);
			house = Houses[index];
			if (house->IsObserver) {
				break;
			}
			if (Session.Type == GAME_INTERNET && WestwoodOnline_Tournament) {
				house->Flag_To_Die();
			} else {
				if (house->Is_Human_Player()) {
					house->AI_Takeover();
				}
			}
			break;

		case LATENCYFUDGE:
			if (!NetSemantic::Latency_Fudge_Is_Valid(Data.General.Value)) {
				Log_Event_Rejection(EventRejectReason::InvalidLatencyFudge, Type, ID, Data.General.Value);
				break;
			}
			DebugString("Executing LATENCYFUDGE event. Frame is %d\n", ::Frame);
			Session.LatencyFudge = Data.General.Value;
			DebugString("LatencyFudge is %d\n", Session.LatencyFudge);

			house = Houses[ID];
			if (house != PlayerPtr && house != NULL) {
				str = Fetch_String(TXT_PLAYER_CHANGED_LATENCY);
				if (str != NULL && strlen(str) != 0) {
					sprintf(msg, str, house->IniName.c_str());
					Session.Messages.Add_Message(NULL, 0, msg, house->Scheme, TextPrintType(TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW), Rule->MessageDelay * TICKS_PER_MINUTE);
				}
			}
			break;

		//
		// This event tells all systems to use new timing values.  It's like
		// RESPONSE_TIME, only it works.  It's only used with the
		// COMM_MULTI_E_COMP protocol.
		//
		case TIMING:
		{
			int const master = Session.Master_Player_ID();
			if (!Session.Play && !NetSemantic::Timing_Authority_Is_Valid(ID, master)) {
				Log_Event_Rejection(EventRejectReason::UnauthorizedTiming, Type, ID, master);
				break;
			}

			if (Session.CommProtocol != COMM_PROTOCOL_MULTI_E_COMP || Frame < 0) {
				Log_Event_Rejection(EventRejectReason::InvalidTimingArithmetic, Type, ID, Frame);
				break;
			}
			if (!NetSemantic::Timing_Values_Are_Valid(Data.Timing.DesiredFrameRate, Data.Timing.MaxAhead, Data.Timing.FrameSendRate)) {
				Log_Event_Rejection(EventRejectReason::InvalidTimingValues, Type, ID, Data.Timing.MaxAhead);
				break;
			}

			NetTiming::TimingSettings const settings{Data.Timing.FrameSendRate, Data.Timing.MaxAhead};
			NetTiming::ConnectionQuality const old_quality = NetTiming::Connection_Quality_For_Settings(Session.Network_Timing_Target());
			unsigned int const old_frame_send_rate = Session.FrameSendRate;
			unsigned int const old_max_ahead = Session.MaxAhead;

			if (settings.MaxAhead > old_max_ahead || settings.FrameSendRate > old_frame_send_rate) {
				std::uint64_t const boundary = settings.FrameSendRate * ((static_cast<std::uint64_t>(Frame) + NetTiming::MAXIMUM_MAX_AHEAD
					+ settings.FrameSendRate - 1) / settings.FrameSendRate);
				if (boundary > static_cast<std::uint64_t>((std::numeric_limits<int>::max)())) {
					Log_Event_Rejection(EventRejectReason::InvalidTimingArithmetic, Type, ID, Frame);
					break;
				}
			}

			NetTiming::ScheduleResult const result = Session.Schedule_Network_Timing(settings, Data.Timing.DesiredFrameRate, static_cast<unsigned int>(Frame));
			if (result == NetTiming::ScheduleResult::Rejected) {
				Log_Event_Rejection(EventRejectReason::UnschedulableTiming, Type, ID, static_cast<int>(settings.MaxAhead));
				break;
			}

			DebugString("Network timing event at frame %d from player %d: %u/%u at %u fps %s\n", Frame, ID, settings.FrameSendRate, settings.MaxAhead,
				Data.Timing.DesiredFrameRate, result == NetTiming::ScheduleResult::Applied ? "applied" : "staged");
			NetTiming::ConnectionQuality const quality = NetTiming::Connection_Quality_For_Settings(settings);
			if (quality != old_quality) {
				char const * format = Fetch_String(TXT_CONNECTION_QUALITY_STATUS);
				char const * quality_name = Fetch_String(Network_Quality_Text_ID(quality));
				if (format != NULL && quality_name != NULL && format[0] != '\0' && quality_name[0] != '\0') {
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
			break;
		}

		//
		// This event tells all systems what the other systems' process
		// timing requirements are; it's used to compute a desired frame rate
		// for the game.
		//
		case PROCESS_TIME:
			for (i = 0; i < Session.Players.Count(); i++) {
				if (ID == Session.Players[i]->Player.ID) {
					Session.Players[i]->Player.ProcessTime = Data.ProcessTime.AverageTicks;
					break;
				}
			}
			break;

		case NETWORK_REPORT:
			// A recording started without a roster has nobody to attribute reports to.
			if ((Session.CommProtocol != COMM_PROTOCOL_MULTI_E_COMP || Frame < 0
				|| !Session.Record_Network_Report(ID, Data.NetworkReport.AverageProcessMilliseconds,
					Data.NetworkReport.WorstRoundTripMilliseconds, Data.NetworkReport.StallMilliseconds, static_cast<unsigned int>(Frame)))
				&& !Session.Play) {
				Log_Event_Rejection(EventRejectReason::InvalidNetworkReport, Type, ID, Data.NetworkReport.WorstRoundTripMilliseconds);
			} else if (!Session.Play) {
				DebugString("Network report at frame %d from player %d: process %u ms, RTT %d ms, longest wait %u ms\n", Frame, ID,
					(unsigned int)Data.NetworkReport.AverageProcessMilliseconds,
					Data.NetworkReport.WorstRoundTripMilliseconds == NETWORK_RTT_UNAVAILABLE ? -1 : (int)Data.NetworkReport.WorstRoundTripMilliseconds,
					(unsigned int)Data.NetworkReport.StallMilliseconds);
			}
			break;

		/*
		**	Default: do nothing.
		*/
		default:
			break;
	}

	//if (Debug_Print_Events) {
	//	printf("\n");
	//}

}
