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

/* $Header: /CounterStrike/TEVENT.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TEVENT.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 11/28/95                                                     *
 *                                                                                             *
 *                  Last Update : July 29, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Attaches_To -- Determines what event can be attached to.                                  *
 *   EventChoiceClass::Draw_It -- Displays the event choice class as a text string.            *
 *   Event_From_Name -- retrieves EventType for given name                                     *
 *   Event_Needs -- Returns with what this event type needs for data.                          *
 *   Name_From_Event -- retrieves name for EventType                                           *
 *   TEventClass::Build_INI_Entry -- Builds the ini text for this event.                       *
 *   TEventClass::Read_INI -- Parses the INI text for this event's data.                       *
 *   TEventClass::Reset -- Reset the trigger for a subsequent "spring".                        *
 *   TEventClass::operator () -- Action operator to see if event is satisfied.                 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "tevent.h"

#include "building.h"
#include "builtype.h"
#include "globals.h"
#include "house.h"
#include "incdec.h"
#include "savestream.h"
#include "scenario.h"
#include "sun.h"
#include "swizzle.h"
#include "team.h"
#include "techno.h"
#include "tracker.h"
#include "vector.h"

DynamicVectorClass<TEventClass *> Events;

#ifdef _DEBUG
/*
**	This is the text name for all of the trigger events. These are used by the scenario editor
*/
/// Sourced from FA2 sources, adjusted for Tiberian Sun
static const struct {
	char const * Name;
	char const * Description;
} _EventText[TEVENT_COUNT] = {
	{"-No Event-","This is a null event. There is no need to ever use this in a real trigger."},
	{"Entered by...","Triggers when an infantry or vehicle enters the attached object (building) or cell. <THEM = House of entering unit>"},
	{"Spied upon","Detects when a spy has entered the attached building."},
	{"Thieved by...","Triggers when a thief steals money from the specified house."},
	{"Discovered by player","Detects when the attached object has been discovered by the player. Discovered means reavealed from under the shroud."},
	{"House Discovered...","Triggers when the specified house has any of its units or buildings discovered by the player."},
	{"Attacked by any house","Triggers when the attached unit is attacked in some manner. Incidental damage or friendly fire does not count."},
	{"Destroyed by any house","Triggers when the attached object has been destroyed. Destroyed by incidental damage or friendly fire doesn't count."},
	{"Any Event","When used alone, it will force the trigger to spring immediately."},
	{"Destroyed, Units, All...","Triggers when all units of the specified house have been destroyed. Typically used for end of game conditions."},
	{"Destroyed, Buildings, All...","Triggers when all buildings of the specified side have been destroyed. Typically used for end of game conditions."},
	{"Destroyed, All...","Triggers when all objects owned by the specified house have been destroyed. This is the normal (destroy everyone) trigger condition for end of game."},
	{"Credits exceed...","Triggers when the house (for this trigger) credit total exceeds this specified amount."},
	{"Elapsed Time...","Triggers when the elapsed time has expired. This time is initialized when the trigger is created. Timer is reset whenever trigger is sprung when trigger is 'persistant'. This is in 15 frame units."},
	{"Mission Timer Expired","Triggers when the global mission timer (as displayed on the screen) has reached zero."},
	{"Destroyed, Buildings, #...","Triggers when the number of buildings, owned by the trigger's specified house, have been destroyed."},
	{"Destroyed, Units, #...","Triggers when the number of units, owned by the trigger's specified house, have been destroyed."},
	{"No Factories left","Triggers when there are no factories left for the house specified in the trigger."},
	{"Civilians Evacuated","Triggers when civilians have been evacuated (left the map)."},
	{"Build Building Type...","When the trigger's house builds the building type specified, then this event will spring."},
	{"Build Unit Type...","When the trigger's house builds the unit type specified, then this event will spring."},
	{"Build Infantry Type...","When the trigger's house builds the infantry type specified, then this event will spring."},
	{"Build Aircraft Type...","When the trigger's house builds the aircraft type specified, then this event will spring."},
	{"Leaves map (team)...","Triggers when the specified team leaves the map. If the team is destroyed, it won't trigger. If all but one member is destroyed and that last member leaves the map, it WILL spring."},
	{"Zone Entry by...","Triggers when a unit of the specified house enters the same zone that this trigger is located in. This trigger must be located in a cell and only a cell. <THEM = House of entering unit>"},
	{"Crosses Horizontal Line...","Triggers when a unit of the specified house crosses the horizontal line as indicated by the location of this trigger. This trigger must be placed in a cell. <THEM = House of entering unit>"},
	{"Crosses Vertical Line...","Triggers when a unit of the specified house crosses the vertical line as indicated by the location of this trigger. This trigger must be placed in a cell. <THEM = House of entering unit>"},
	{"Global is set...","Triggers when the specifed global (named in Globals.INI) is turned on."},
	{"Global is clear...","Triggers when the specified global (named in Globals.INI) is turned off."},
	{"Destroyed by anything [not infiltrate]", "Triggers when attached object is destroyed, but not if it infiltrates a building/unit."},
	{"Power Low...","Triggers when the specified house's power falls below 100% level."},
	{"Bridge destroyed","Triggers when the attached bridge is destroyed. A bridge is considered destroyed when an impassable gap is created in the bridge."},
	{"Building exists...","Triggers when the building (owned by the house of this trigger) specified exists on the map. This works for buildings that are preexisting or constructed by deploying."},
	{"Selected by player", "Triggers when the unit is selected by the player.  Use in single-player only."},
	{"Comes near waypoint...", "Triggers when the object comes near the specified waypoint."},
	{"Enemy In Spotlight...", "Triggers when an enemy unit enters the spotlight cast by the attached building."},
	{"Local is set...", "Triggers when the specifed local is turned on."},
	{"Local is clear...", "Triggers when the specified local is turned off."},
	{"First damaged (combat only)", "Triggers when first suffering from combat damage from combat damage only."},
	{"Half health (combat only)", "Triggers when damaged to half health >from combat damage only."},
	{"Quarter health (combat only)", "Triggers when damaged to quarter health from combat damage only."},
	{"First damaged (any source)", "Triggers when first suffering from combat damage from any source."},
	{"Half health (any source)", "Triggers when damaged to half health >from any source."},
	{"Quarter health (any source)", "Triggers when damaged to quarter health from any source."},
	{"Attacked by (house)...", "When attacked by some unit of specified house."},
	{"Ambient light <= ...", "Triggers when the ambient light drops below a certain level.  Use numbers between 0 and 100."},
	{"Ambient light >= ...", "Triggers when the ambient light rises above a certain level.  Use numbers between 0 and 100."},
	{"Elapsed Scenario Time...", "When time has elapsed since start of scenario."},
	{"Destroyed by anything", "Triggers when destroyed by anything what-so-ever."},
	{"Pickup Crate", "When crate is picked up object the trigger is attached to."},
	{"Pickup Crate (any)", "When crate is picked up by any unit."},
	{"Random delay...", "Delays a random time between 50 and 150 percent of time specified."},
	{"Credits below...","Triggers when the house (for this trigger) credit total is below this specified amount."},
	{"Paralyzed", "Triggers when a object is paralyzed under EMP effect or web."},
	{"Enemy In Spotlight... (repeating)", "Triggers when an enemy unit enters the spotlight cast by the attached building. Unlike the other spotlight event, this one is re-tested every time the trigger is polled instead of staying satisfied once it has fired."},
	{"Limped", "Triggers when a object has been limped by a limpet drone."}
};
#endif


/// <summary>
/// Constructor for the trigger event object.
/// The new event is registered with the event heap and the trackers, but watches for
/// nothing at all until the scenario reader fills in which event it is and what it
/// examines.
/// </summary>
TEventClass::TEventClass(void) :
	HeapID(-1),
	Next(NULL),
	Event(TEVENT_NONE),
	Team(NULL)
{
	Events.Add(this);
	HeapID = Events.ID(this);

	EventActionPtrTracker.Add(this);
	AbstractTypePtrTracker.Add(this);

	Data.Value = 0;
}


/// <summary>
/// Destructor for the trigger event object.
/// This routine will sever every reference the game still holds to this event before it
/// drops out of the event heap and the various trackers.
/// </summary>
TEventClass::~TEventClass(void)
{
	Detach_This_From_All(this, true);
	EventActionPtrTracker.Delete(this);
	AbstractTypePtrTracker.Delete(this);
	Events.Delete(this);
}


/***********************************************************************************************
 * TEventClass::operator () -- Action operator to see if event is satisfied.                   *
 *                                                                                             *
 *    This routine is called to see if the event has been satisfied. Typically, when the       *
 *    necessary trigger events have been satisfied, then the trigger is sprung. For some       *
 *    events, the act of calling this routine is tacit proof enough that the event has         *
 *    occurred. For most other events, the success condition must be explicitly checked.       *
 *                                                                                             *
 * INPUT:   event -- The event that has occurred according to the context from which this      *
 *                   routine was called. In the case of no specific event having occurred,     *
 *                   then TEVENT_ANY will be passed in.                                        *
 *                                                                                             *
 *          house -- The house that this event is tied to.                                     *
 *                                                                                             *
 *          object-- The object that this event might apply to. For object triggering          *
 *                   events, this will point to the perpetrator object.                        *
 *                                                                                             *
 *          forced-- If this event is forced by some outside action, this flag will be true.   *
 *                   Forcing only occurs as an explicit action from another trigger.           *
 *                                                                                             *
 * OUTPUT:  Was this event satisfied? A satisfied event will probably spring the trigger       *
 *          it is attached to.                                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TEventClass::operator () (TEventType event, HouseClass const * house, ObjectClass const * object, CDTimerClass<FrameTimerClass> & timer, bool & tripped, TechnoClass * source)
{
	/*
	**	Triggers based on the game's global environment such as time or
	**	global flags are triggered only when the appropriate condition
	**	is true.
	*/
	switch (Event) {
		case TEVENT_AMBIENT_LESS_THAN:
			return(Scen->CurrentAmbientLight <= Data.Value);
			break;

		case TEVENT_AMBIENT_GREATER_THAN:
			return(Scen->CurrentAmbientLight >= Data.Value);
			break;

		case TEVENT_GAME_TIME:
			if (Data.Value > (Frame / TICKS_PER_SECOND)) {
				return(false);
			} else {
				return(true);
			}
			break;

		case TEVENT_GLOBAL_SET: {
			bool value;
			Scen->Fetch_Global_Value(Data.Value, value);
			return(value);
		}

		case TEVENT_GLOBAL_CLEAR: {
			bool value;
			Scen->Fetch_Global_Value(Data.Value, value);
			return(!value);
		}

		case TEVENT_LOCAL_SET: {
			bool value;
			Scen->Fetch_Local_Value(Data.Value, value);
			return(value);
		}

		case TEVENT_LOCAL_CLEAR: {
			bool value;
			Scen->Fetch_Local_Value(Data.Value, value);
			return(!value);
		}

		case TEVENT_MISSION_TIMER_EXPIRED:
			if (!Scen->MissionTimer.Is_Active() || Scen->MissionTimer != 0) return(false);
			return(true);

		case TEVENT_TIME:
		case TEVENT_RANDOM_TIME:
			if (timer != 0) return(false);
			return(true);

	}

	/*
	**	Don't trigger this event if the parameters mean nothing. Typical of
	**	this would be for events related to time or other outside influences.
	*/
	if (Event == TEVENT_NONE) {
		return(false);
	}

	/*
	**	If this is not the event for this trigger, just return. This is only
	**	necessary to check for those trigger events that are presumed to be
	**	true just by the fact that this routine is called with the appropriate
	**	event identifier.
	*/
	if (Event != TEVENT_ATTACKED_BY &&
		Event != TEVENT_PLAYER_ENTERED &&
		Event != TEVENT_CROSS_HORIZONTAL &&
		Event != TEVENT_CROSS_VERTICAL &&
		Event != TEVENT_NEAR_WAYPOINT &&
		Event != TEVENT_ENTERS_ZONE &&
		Event != TEVENT_BUILD &&
		Event != TEVENT_BUILD_UNIT &&
		Event != TEVENT_BUILD_INFANTRY &&
		Event != TEVENT_BUILD_AIRCRAFT &&
		Is_Time_Based()) {
		if ((event != Event) || Debug_Map) {
			return(false);
		}
	}

	/*
	**	The cell entry trigger event is only tripped when an object of the
	**	matching ownership has entered the cell in question. All other
	**	conditions will not trigger the event.
	*/
	if (Event == TEVENT_PLAYER_ENTERED || Event == TEVENT_CROSS_HORIZONTAL || Event == TEVENT_CROSS_VERTICAL || Event == TEVENT_ENTERS_ZONE) {
		if (event != Event) return(false);
		if (!object) return(false);
		if (Data.House != HOUSE_NONE) {
			HouseClass * owner = House_From_HousesType(Data.House);
			if (owner == NULL || object->Owner() != owner->HeapID) return(false);
		}
		tripped = true;
		return(true);
	}
	else if (Event == TEVENT_NEAR_WAYPOINT) {
		if (event != Event) return(false);
		Coord waypoint_coord(Scen->Get_Waypoint_Coord(Data.Value));
		if (object->Distance(waypoint_coord) > (CELL_LEPTON_W*5)) {
			return(false);
		}
		return(true);
	}
	else if (Event == TEVENT_ATTACKED_BY) {
		if (event != Event) return(false);
		if (source == NULL || House_From_HousesType(Data.House) != source->House) {
			return(false);
		}
	}

	/*
	**	The following trigger events are not considered to have sprung
	**	merely by fact that this routine has been called. These trigger
	**	events must be verified manually by examining the house that
	**	they are assigned to.
	*/
	int index;
	if (house != NULL) {
		switch (Event) {
			/*
			**	Check to see if a team of the appropriate type has left the map.
			*/
			case TEVENT_LEAVES_MAP:
				for (index = 0; index < Teams.Count(); index++) {
					TeamClass * ptr = Teams[index];
					if (ptr->Class == Team && ptr->Is_Empty() && ptr->IsLeaveMap) {
						tripped = true;
						break;
					}
				}
				if (index == Teams.Count()) return(false);
				break;

			/*
			**	Credits must be equal or greater to the value specified.
			*/
			case TEVENT_CREDITS:
				if (((HouseClass *)house)->Available_Money() < Data.Value) return(false);
				break;

			case TEVENT_CREDITS_BELOW:
				if (((HouseClass *)house)->Available_Money() > Data.Value) return(false);
				break;

			/*
			**	Ensure that there are no more factories left.
			*/
			case TEVENT_NOFACTORIES:
				if (house->CurBuildings > 0) {
					for (int index = 0; index < Buildings.Count(); index++) {
						BuildingClass * ptr = Buildings[index];
						if (ptr != NULL && !ptr->IsInLimbo && ptr->House == house && ptr->Class->ToBuild != RTTI_NONE) {
							return(false);
						}
					}
				}
				break;

			/*
			**	A civilian must have been evacuated.
			*/
			case TEVENT_EVAC_CIVILIAN:
				if (!house->IsCivEvacuated) return(false);
				break;

			/*
			**	Verify that the structure has been built.
			*/
			case TEVENT_BUILDING_EXISTS:
				if (house->BQuantity.Value(Data.Structure) == 0) return(false);
				tripped = true;
				break;

			/*
			**	Verify that the structure has been built.
			*/
			case TEVENT_BUILD:
				if (house->JustBuiltStructure != Data.Structure) return(false);
				tripped = true;
				break;

			/*
			**	Verify that the unit has been built.
			*/
			case TEVENT_BUILD_UNIT:
				if (house->JustBuiltUnit != Data.Unit) return(false);
				tripped = true;
				break;

			/*
			**	Verify that the infantry has been built.
			*/
			case TEVENT_BUILD_INFANTRY:
				if (house->JustBuiltInfantry != Data.Infantry) return(false);
				tripped = true;
				break;

			/*
			**	Verify that the aircraft has been built.
			*/
			case TEVENT_BUILD_AIRCRAFT:
				if (house->JustBuiltAircraft != Data.Aircraft) return(false);
				tripped = true;
				break;

			/*
			**	Verify that the specified number of buildings have been destroyed.
			*/
			case TEVENT_NBUILDINGS_DESTROYED:
				if (house->BuildingsLost < Data.Value) return(false);
				break;

			/*
			**	Verify that the specified number of units have been destroyed.
			*/
			case TEVENT_NUNITS_DESTROYED:
				if (house->UnitsLost < Data.Value) return(false);
				break;

			default:
				break;
		}
	}

	house = House_From_HousesType(Data.House);
	if (house != NULL) {
		switch (Event) {
			case TEVENT_LOW_POWER:
				if (house->Power_Fraction() >= 1) return(false);
				break;

			case TEVENT_THIEVED:
				if (!house->IsThieved) return(false);
				break;

			/*
			**	Verify that the house has been discovered.
			*/
			case TEVENT_HOUSE_DISCOVERED:
				if (!house->IsDiscovered) return(false);
				break;

			/*
			**	Verify that all buildings have been destroyed.
			*/
			case TEVENT_BUILDINGS_DESTROYED:
				if (house->CurBuildings > 0) return(false);
				break;

			/*
			**	Verify that all units have been destroyed -- with some
			**	exceptions.
			*/
			case TEVENT_UNITS_DESTROYED:
				if (house->CurUnits > 0 || house->CurInfantry > 0) return(false);
				break;

			/*
			**	Verify that all buildings and units have been destroyed.
			*/
			case TEVENT_ALL_DESTROYED:
				if (house->CurBuildings > 0 || house->CurUnits > 0 || house->CurInfantry > 0) return(false);
				break;

			default:
				break;
		}
	}

	return(true);
}


/***********************************************************************************************
 * TEventClass::Build_INI_Entry -- Builds the ini text for this event.                         *
 *                                                                                             *
 *    This routine will build the ini text for this trigger event. The ini text is appended    *
 *    to the string buffer specified. This routine is used to build the complete trigger       *
 *    ini text for writing out to the INI scenario file.                                       *
 *                                                                                             *
 * INPUT:   ptr   -- Pointer to the string buffer that will hold the event INI data.           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TEventClass::Build_INI_Entry(char * ptr) const
{
	int code = 0;
	int val = Data.Value;
	NeedType need = Event_Needs(Event);
	if (Team != NULL) {
		code = 1;
		ptr += strlen(ptr);
		wsprintf(ptr, "%d,%d,%s", Event, code, (char const *)Team->IniName);
	} else {
		ptr += strlen(ptr);
		wsprintf(ptr, "%d,%d,%d", Event, code, val);
	}
}


/***********************************************************************************************
 * TEventClass::Read_INI -- Parses the INI text for this event's data.                         *
 *                                                                                             *
 *    This routine is used to parse the INI data line to fetch the event's data from it.       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void TEventClass::Read_INI(void)
{

	Data.Value = 0;
	Event = TEventType(atoi(strtok(NULL, ",")));
	int code = atoi(strtok(NULL, ","));
	char * text = strtok(NULL, ",");
	int val = atoi(text);

	switch (code) {
		case 0:
			Data.Value = val;
			break;

		case 1:
			Team = TeamTypeClass::From_Name(text);
			break;
	}
}


/***********************************************************************************************
 * Event_Needs -- Returns with what this event type needs for data.                            *
 *                                                                                             *
 *    This routine will examine the specified event type and return a code that indicates      *
 *    the type of data that must be supplied to this event. Some events require no data and    *
 *    these will return NEED_NONE.                                                             *
 *                                                                                             *
 * INPUT:   event -- The event type to examine.                                                *
 *                                                                                             *
 * OUTPUT:  Returns with the type of additional data that is needed by this event.             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
NeedType Event_Needs(TEventType event)
{
	switch (event) {
		case TEVENT_THIEVED:
		case TEVENT_PLAYER_ENTERED:
		case TEVENT_CROSS_HORIZONTAL:
		case TEVENT_CROSS_VERTICAL:
		case TEVENT_ENTERS_ZONE:
		case TEVENT_HOUSE_DISCOVERED:
		case TEVENT_BUILDINGS_DESTROYED:
		case TEVENT_UNITS_DESTROYED:
		case TEVENT_ALL_DESTROYED:
		case TEVENT_LOW_POWER:
		case TEVENT_ATTACKED_BY:
			return(NEED_HOUSE);

		case TEVENT_NUNITS_DESTROYED:
		case TEVENT_NBUILDINGS_DESTROYED:
		case TEVENT_CREDITS:
		case TEVENT_CREDITS_BELOW:
		case TEVENT_TIME:
		case TEVENT_RANDOM_TIME:
		case TEVENT_GAME_TIME:
		case TEVENT_AMBIENT_LESS_THAN:
		case TEVENT_AMBIENT_GREATER_THAN:
			return(NEED_NUMBER);

		case TEVENT_GLOBAL_SET:
		case TEVENT_GLOBAL_CLEAR:
			return(NEED_GLOBAL);

		case TEVENT_LOCAL_SET:
		case TEVENT_LOCAL_CLEAR:
			return(NEED_LOCAL);

		case TEVENT_BUILDING_EXISTS:
		case TEVENT_BUILD:
			return(NEED_STRUCTURE);

		case TEVENT_BUILD_UNIT:
			return(NEED_UNIT);

		case TEVENT_BUILD_INFANTRY:
			return(NEED_INFANTRY);

		case TEVENT_BUILD_AIRCRAFT:
			return(NEED_AIRCRAFT);

		case TEVENT_LEAVES_MAP:
			return(NEED_TEAM);

		case TEVENT_NEAR_WAYPOINT:
			return(NEED_WAYPOINT);

		default:
			break;
	}
	return(NEED_NONE);
}


#ifdef _DEBUG
/***********************************************************************************************
 * Event_From_Name -- retrieves EventType for given name                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      name      name to get event for                                                        *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      EventType for given name                                                               *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/1994 BR : Created.                                                                  *
 *=============================================================================================*/
TEventType Event_From_Name (char const * name)
{
	if (name) {
		for (TEventType i = TEVENT_NONE; i < TEVENT_COUNT; ++i) {
			if (!stricmp(name, _EventText[i].Name)) {
				return(i);
			}
		}
	}

	return(TEVENT_NONE);
}


/***********************************************************************************************
 * Name_From_Event -- retrieves name for EventType                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      event      EventType to get name for                                                   *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      name for EventType                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/1994 BR : Created.                                                                  *
 *=============================================================================================*/
char const * Name_From_Event(TEventType event)
{
	return(_EventText[event].Name);
}
#endif


/***********************************************************************************************
 * Attaches_To -- Determines what event can be attached to.                                    *
 *                                                                                             *
 *    This routine is used to determine what this event type can be attached to in the game.   *
 *    Some events are specifically tied to cells or game object. This routine will indicate    *
 *    this requirement.                                                                        *
 *                                                                                             *
 * INPUT:   event -- The event type to examine.                                                *
 *                                                                                             *
 * OUTPUT:  Returns with the attachable characteristics for this event type. These             *
 *          characteristics are represented by a composite bit field. Some events can be       *
 *          attached to multiple objects.                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
AttachType Attaches_To(TEventType event)
{
	AttachType attach = ATTACH_NONE;

	switch (event) {
		case TEVENT_CROSS_HORIZONTAL:
		case TEVENT_CROSS_VERTICAL:
		case TEVENT_ENTERS_ZONE:
		case TEVENT_PLAYER_ENTERED:
		case TEVENT_ANY:
		case TEVENT_DISCOVERED:
		case TEVENT_NONE:
		case TEVENT_BRIDGE_DESTROYED:
			attach = AttachType(attach | ATTACH_CELL);
			break;

		default:
			break;
	}

	switch (event) {
		case TEVENT_FIRST_DAMAGED_ANY:
		case TEVENT_ENTER_YELLOW_ANY:
		case TEVENT_ENTER_RED_ANY:
		case TEVENT_FIRST_DAMAGED:
		case TEVENT_ENTER_YELLOW:
		case TEVENT_ENTER_RED:
		case TEVENT_SPIED:
		case TEVENT_PLAYER_ENTERED:
		case TEVENT_DISCOVERED:
		case TEVENT_DESTROYED:
		case TEVENT_DESTROYED_ANY:
		case TEVENT_DESTROYED_ANY_X:
		case TEVENT_ATTACKED:
		case TEVENT_ATTACKED_BY:
		case TEVENT_ANY:
		case TEVENT_NONE:
		case TEVENT_SELECTED:
		case TEVENT_NEAR_WAYPOINT:
		case TEVENT_ENEMY_IN_SPOTLIGHT:
		case TEVENT_PICKUP_CRATE:
		case TEVENT_PARALYZED:
		case TEVENT_ENEMY_IN_SPOTLIGHT_REPEATING:
		case TEVENT_LIMPED:
			attach = AttachType(attach | ATTACH_OBJECT);
			break;

		default:
			break;
	}

	switch (event) {
		case TEVENT_ENTERS_ZONE:
		case TEVENT_ANY:
			attach = AttachType(attach | ATTACH_MAP);
			break;

		default:
			break;
	}

	switch (event) {
		case TEVENT_LOW_POWER:
		case TEVENT_EVAC_CIVILIAN:
		case TEVENT_BUILDING_EXISTS:
		case TEVENT_BUILD:
		case TEVENT_BUILD_UNIT:
		case TEVENT_BUILD_INFANTRY:
		case TEVENT_BUILD_AIRCRAFT:
		case TEVENT_NOFACTORIES:
		case TEVENT_BUILDINGS_DESTROYED:
		case TEVENT_NBUILDINGS_DESTROYED:
		case TEVENT_UNITS_DESTROYED:
		case TEVENT_NUNITS_DESTROYED:
		case TEVENT_ALL_DESTROYED:
		case TEVENT_HOUSE_DISCOVERED:
		case TEVENT_CREDITS:
		case TEVENT_CREDITS_BELOW:
		case TEVENT_THIEVED:
		case TEVENT_ANY:
			attach = AttachType(attach | ATTACH_HOUSE);
			break;

		default:
			break;
	}

	switch (event) {
		case TEVENT_GAME_TIME:
		case TEVENT_TIME:
		case TEVENT_RANDOM_TIME:
		case TEVENT_GLOBAL_SET:
		case TEVENT_GLOBAL_CLEAR:
		case TEVENT_LOCAL_SET:
		case TEVENT_LOCAL_CLEAR:
		case TEVENT_MISSION_TIMER_EXPIRED:
		case TEVENT_ANY:
		case TEVENT_AMBIENT_LESS_THAN:
		case TEVENT_AMBIENT_GREATER_THAN:
		case TEVENT_LEAVES_MAP:
		case TEVENT_PICKUP_CRATE_ANY:
			attach = AttachType(attach | ATTACH_GENERAL);
			break;

		default:
			break;
	}

	return(attach);
}


/// <summary>
/// Severs any references this event holds to the target.
/// This routine is called when an object or type is about to be destroyed. Every reference
/// the event keeps to it must be dropped, and the event list is closed up around a link
/// that is going away.
/// </summary>
/// <param name="target">The object or type that is about to disappear.</param>
/// <param name="all">Should every reference be severed?</param>
void TEventClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);

	if (Next == target) {
		Next = Next->Next;
	}
	if (Team == target) {
		Team = NULL;
	}
}


/// <summary>
/// Submits this event's state to the game CRC.
/// This routine is used by the network synchronization check, so the objects this event
/// hangs off are submitted by identifier rather than by pointer.
/// </summary>
/// <param name="crc">The CRC accumulator to submit this event's data to.</param>
void TEventClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);

	if (Next != NULL) crc(Next->Fetch_ID());
	crc(Event);
	if (Team != NULL) crc(Team->Fetch_ID());
	crc((int)Data.Value);
}


/// <summary>
/// Fetches the class identifier of this object.
/// This routine is used by the persistence machinery to recognize what kind of object it
/// is about to load back.
/// </summary>
/// <param name="retval">Pointer to the identifier to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT TEventClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_EventClass;
	return(S_OK);
}


/// <summary>
/// Lists the members this event carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TEventClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HeapID);
	stream.Serialize(Next);
	stream.Serialize(Event);
	stream.Serialize(Team);

	/*
	 * Which alternative of the data is live depends on the event, but every one of them is
	 * a plain scalar and none holds a pointer, so the union travels as its raw image.
	 */
	stream.Serialize_Bytes(&Data, sizeof(Data));
}


/// <summary>
/// Determines if this event fires at a moment in time.
/// A temporal event is announced to the trigger as it happens rather than being a lasting
/// condition that can be examined at leisure. The trigger springing logic uses this to tell
/// the two kinds apart, since only a momentary event may be recorded as having occurred.
/// </summary>
/// <returns>bool; Is this event a momentary occurrence?</returns>
bool TEventClass::Is_Time_Based(void) const
{
	switch (Event) {
		case TEVENT_PICKUP_CRATE:
		case TEVENT_PICKUP_CRATE_ANY:
		case TEVENT_PLAYER_ENTERED:
		case TEVENT_SPIED:
		case TEVENT_THIEVED:
		case TEVENT_DISCOVERED:
		case TEVENT_ATTACKED:
		case TEVENT_ATTACKED_BY:
		case TEVENT_DESTROYED:
		case TEVENT_DESTROYED_ANY:
		case TEVENT_DESTROYED_ANY_X:
		case TEVENT_EVAC_CIVILIAN:
		case TEVENT_BUILD:
		case TEVENT_BUILD_UNIT:
		case TEVENT_BUILD_INFANTRY:
		case TEVENT_BUILD_AIRCRAFT:
		case TEVENT_LEAVES_MAP:
		case TEVENT_ENTERS_ZONE:
		case TEVENT_CROSS_HORIZONTAL:
		case TEVENT_CROSS_VERTICAL:
		case TEVENT_SELECTED:
		case TEVENT_NEAR_WAYPOINT:
		case TEVENT_ENEMY_IN_SPOTLIGHT:
		case TEVENT_FIRST_DAMAGED:
		case TEVENT_ENTER_YELLOW:
		case TEVENT_ENTER_RED:
		case TEVENT_FIRST_DAMAGED_ANY:
		case TEVENT_ENTER_YELLOW_ANY:
		case TEVENT_ENTER_RED_ANY:
		case TEVENT_BRIDGE_DESTROYED:
		case TEVENT_PARALYZED:
		case TEVENT_ENEMY_IN_SPOTLIGHT_REPEATING:
		case TEVENT_LIMPED:
			return(true);

	}
	return(false);
}


/// <summary>
/// Determines if a satisfied event can be remembered.
/// The trigger logic will only mark a temporal event off as sprung when this routine says
/// it may. The events that answer no are re-examined every time the trigger is polled,
/// since the circumstance that satisfied them can just as easily lapse.
/// </summary>
/// <returns>bool; Can this event be marked as sprung once it has fired?</returns>
bool TEventClass::Is_To_Flag_As_Tripped(void) const
{
	switch (Event) {
		case TEVENT_ATTACKED:
		case TEVENT_ATTACKED_BY:
		case TEVENT_PLAYER_ENTERED:
		case TEVENT_PARALYZED:
		case TEVENT_ENEMY_IN_SPOTLIGHT_REPEATING:
			return(false);
	}
	return(true);
}
