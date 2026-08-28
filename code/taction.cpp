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

/* $Header: /CounterStrike/TACTION.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : ACTION.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 11/28/95                                                     *
 *                                                                                             *
 *                  Last Update : July 17, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Action_From_Name -- retrieves ActionType for given name                                   *
 *   Action_Needs -- Figures out what data an action object needs.                             *
 *   Name_From_Action -- retrieves name for ActionType                                         *
 *   TActionClass::Build_INI_Entry -- Builds an INI entry for this trigger action.             *
 *   TActionClass::Code_Pointers -- Convert embedded pointers into a transportable format.     *
 *   TActionClass::Decode_Pointers -- Converts coded pointers into usable format.              *
 *   TActionClass::Detach -- Removes any attachment from associated action.                    *
 *   TActionClass::Read_INI -- Converts INI text into appropriate action data.                 *
 *   TActionClass::operator -- Performs the action that this object does.                      *
 *   ActionChoiceClass::Draw_It -- Display the action choice as part of a list box.            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "taction.h"

#include "_keyboar.h"
#include "_map.h"
#include "_rules.h"
#include "_tactica.h"
#include "_weapon.h"
#include "anim.h"
#include "blight.h"
#include "building.h"
#include "builtype.h"
#include "bullet.h"
#include "ccrand.h"
#include "cell.h"
#include "combat.h"
#include "empulse.h"
#include "foot.h"
#include "globals.h"
#include "house.h"
#include "houstype.h"
#include "incdec.h"
#include "infantry.h"
#include "ion.h"
#include "ionblast.h"
#include "map.h"
#include "movie.h"
#include "partsys.h"
#include "reinf.h"
#include "revent.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "session.h"
#include "sun.h"
#include "super.h"
#include "suprtype.h"
#include "swizzle.h"
#include "tactical.h"
#include "tag.h"
#include "tagtype.h"
#include "team.h"
#include "teamtype.h"
#include "tevent.h"
#include "theme.h"
#include "tracker.h"
#include "trigger.h"
#include "trigtype.h"
#include "vanim.h"
#include "vector.h"
#include "velocity.h"
#include "voc.h"
#include "vox.h"
#include "warhead.h"
#include "waypoint.h"
#include "weapon.h"

#include "draw.hh"
#include "need.hh"

#include <cstdio>

DynamicVectorClass<TActionClass *> Actions;

#ifdef _DEBUG
/// Sourced from FA2 sources, adjusted for Tiberian Sun
static const struct {
	char const * Name;
	char const * Description;
} _ActionText[TACTION_COUNT] = {
	{"-No Action-", "This is a null action. It will do nothing and is equivalent to not having an action at all. Why use it?"},
	{"Winner is...", "The winner will be forced to be the house specified. The game will end immediately. Typically, the player's house is specified."},
	{"Loser is...", "The loser will be force to be the house specified. The game will end immediately. Typically, the player's house is specified."},
	{"Production Begins...", "The computer's house (as specified) will begin production of units and structures."},
	{"Create Team...", "Creates a team of the type specified (owned by the house of this trigger). The team member are NOT automatically created however."},
	{"Destroy Team...", "Destroys all instances of the team type specified. The units in those existing teams will remain and be available for recruiting into other teams."},
	{"All to Hunt...", "Forces all units, of the house specified, into 'hunt' mode. They will seek out and destroy their enemies."},
	{"Reinforcement (team)...", "Create a reinforcement of the specified team. The members of the team WILL be created magically by this action."},
	{"Drop Zone Flare (waypoint)...", "Display a drop zone flair at the waypoint specified. The map will also be reaveald around that location."},
	{"Fire Sale...", "Cause all buildings of the specified house to be sold (for cash and prizes). Typically this is used in the final assault by the computer."},
	{"Play Movie...", "Displays the specified movie (full screen). The game is paused while this occurs and resumes normally after it completes."},
	{"Text Trigger...", "Display the text identified by the string file <label>."},
	{"Destroy Trigger...", "Destroy all current instances of the trigger type specified. This does not prevent future instances of that trigger >from being created."},
	{"Autocreate Begins...", "Initiates autocreate for the house specified. This will cause the computer's house to build autocreate teams as it sees fit."},
	{"Change House...", "Changes owning house to the one specified for attached objects."},
	{"Allow Win", "Removes one 'blockage' from allowing the player to win. The blockage number is equal the number of triggers created that have this action."},
	{"Reveal all map", "Reveals the entire map to the player."},
	{"Reveal around waypoint...", "Reveals a region of the map to the player around the waypoint specified."},
	{"Reveal zone of waypoint...", "Reveals all cells that share the same zone as the waypoing specified. This yields some wierd results. Use with caution."},
	{"Play sound effect...", "Plays the sound effect specified."},
	{"Play music theme...", "Plays the music theme specified."},
	{"Play speech...", "Plays the speech sound specified."},
	{"Force Trigger...", "Force all triggers of this specified type to spring regardless of what it's event flags may indicate."},
	{"Timer Start", "Start the global mission timer."},
	{"Timer Stop", "Stop the global mission timer."},
	{"Timer Extend...", "Extend the global mission timer by the time specified."},
	{"Timer Shorten...", "Short the global mission timer by the time specified. It can never be reduced below 'zero' time."},
	{"Timer Set...", "Set the global mission timer to the value specified."},
	{"Global Set...", "Set the global flag. Global flags are named in the file Globals.INI. Global flags can be either 'on/set/true' or 'off/clear/false'."},
	{"Global Clear...", "Clear the global flag. Global flags are named in the file Globals.INI. Global flags can either be 'on/set/true' or 'off/clear/false'."},
	{"Auto Base Building...", "Initialize the computer skirmish mode build control to either 'on' or 'off' state. When 'on', the computer takes over as if it were in skirmish mode. (gs make sure he has a con yard)"},
	{"Grow shroud one 'step'", "Increase the shroud darkness by one step (cell)."},
	{"Destroy attached building", "Destroy any buildings, bridges, or units that this trigger is attached to."},
	{"Add 1-time special weapon...", "Add a one-shot special weapon (as indicated) to the trigger's house."},
	{"Add repeating special weapon...", "Add a permanent special weapon (as indicated) to the trigger's house."},
	{"Preferred target...","Specify what the trigger's house should use as its preferred target when using special weapon attacks."},
	{"All change house...", "All objects of one house change ownership to specified house."},
	{"Make ally...", "Cause this trigger's house to ally with the house specified."},
	{"Make enemy...", "Cause this trigger's house to un-ally (declare war) with the house specified."},
	{"Change Zoom Level...", "Changes the zoom out level of the player's radar map.  Use 1 for normal view, 2 for zoomed out."},
	{"Resize Player View...", "Changes the player's viewing rectangle into the map.  Enter as:   x,y,w,h   where x,y gives the upper left corner and w,h give the width and height."},
	{"Play Anim At...", "Plays the specified anim in the specified cell."},
	{"Do Explosion At...", "Creates an explosion in the specified cell, using the specified warhead."},
	{"Meteor Impact At...", "Sends a meteor at the specified cell."},
	{"Ion Storm start...", "Starts an ion storm sequence to run for the specified number of game frames."},
	{"Ion Storm stop...", "End an Ion storm in progress."},
	{"Lock input", "Disables user input."},
	{"Unlock input", "Enables user input."},
	{"Center Camera at Waypoint...", "Moves the tactical view to a specified waypoint."},
	{"Zoom in", "Zooms the tactical map in."},
	{"Zoom out", "Zooms the tactical map out."},
	{"Reshroud Map", "Reshrouds the entire map."},
	{"Change Light Behavior", "Changes the way a building spotlight behaves. Attach this trigger to a building that casts a spotlight."},
	{"Enable Trigger", "Enables the target trigger."},
	{"Disable Trigger", "Disables the target trigger."},
	{"Create Radar Event", "Creates a radar event at the specified waypoint"},
	{"Local Set...", "Set the local flag. Local flags can be either 'on/set/true' or 'off/clear/false'."},
	{"Local Clear...", "Clear the local flag. Local flags can either be 'on/set/true' or 'off/clear/false'."},
	{"Meteor Shower At...", "Creates a meteor shower around the specified waypoint."},
	{"Reduce Tiberium At...", "Reduces Tiberium around the specified waypoint."},
	{"Sell building", "Sells the building attached to this trigger."},
	{"Turn off building", "Turn off building attached to this trigger."},
	{"Turn on building", "Turn on building attached to this trigger."},
	{"Apply 100 damage at...", "Applies 100 points of HE damage at location."},
	{"Light flash (small) at...", "Shows a small light flash at location."},
	{"Light flash (medium) at...", "Shows a medium light flash at location."},
	{"Light flash (large) at...", "Shows a large light flash at location."},
	{"Announce Win", "Announce that player has won."},
	{"Announce Lose", "Announce that player has lost."},
	{"Force end", "Force end of scenario."},
	{"Destroy Tag...", "Destroy tag and all attached triggers."},
	{"Set ambient step...", "Sets ambient light fade step value."},
	{"Set ambient rate...", "Sets ambient light fade rate."},
	{"Set ambient light...", "Fades ambient light to new lighting level."},
	{"AI triggers begin...", "Start AI triggers for specified house."},
	{"AI triggers stop...", "Stop AI triggers for specified house."},
	{"Ratio of AI trigger teams...", "AI percentage of teams created for AI triggers (100 = all for AI trigger teams, 0 = all for regular teams)"},
	{"Ratio of team aircraft...", "AI percentage of aircraft created for teams (100 = all for teams, 0 = all random)"},
	{"Ratio of team infantry...", "AI percentage of infantry created for teams (100 = all for teams, 0 = all random)"},
	{"Ratio of team units...", "AI percentage of units created for teams (100 = all for teams, 0 = all random)"},
	{"Reinforcement (team) [at waypoint]...", "Create reinforcement team at special waypoint location."},
	{"Wakeup self", "Breaks out of sleep or harmless mode so as to enter guard mode."},
	{"Wakeup all sleepers", "Breaks all units out of sleep mode."},
	{"Wakeup all harmless", "Breaks all out of harmless mode."},
	{"Wakeup group...", "Wakeup all units of specified group."},
	{"Vein growth...", "Control if veins grow or not."},
	{"Tiberium growth...", "Control if Tiberium grows or not."},
	{"Ice growth...", "Control if ice grows or not."},
	{"Particle Anim at...", "Show particle animation at location."},
	{"Remove Particle Anim at...", "Delete particle anims at specified location."},
	{"Lightning strike at...", "A single Ion Storm lightning strike."},
	{"Go Berzerk", "Attached object (cyborg) goes berzerk."},
	{"Activate Firestorm Defense", "Turns on a house's firestorm defense."},
	{"Deactivate Firestorm Defense", "Turns off a house's firestorm defense."},
	{"Ion-cannon strike...", "Fires Ion-Cannon at waypoint specified."},
	{"Nuke strike...", "Fires Nuke at waypoint specified from nearest edge."},
	{"Chem-missile strike...", "Fires Chemical missile at waypoint specified."},
	{"Toggle Train Cargo", "Toggles state of cargo train dropping crate."},
	{"Play Sound Effect (Random)...", "Plays sound effect at random waypoint."},
	{"Play Sound Effect At...", "Plays sound effect specified at waypoint specified. "},
	{"Play Ingame Movie...", "Displays the specified movie ingame. Player still has control of interface and units."},
	{"Flash Team...", "Flashes the specified team for the specified number of frames"},
	{"Disable Speech", "Disables EVA speech."},
	{"Enable Speech", "Enables EVA speech."},
	{"Set Group ID...", "Sets the group ID of the attached object."},
	{"Talk Bubble...", "Displays talk bubble over unit"}
};
#endif

enum ParamCodeType {
	PARAM_CODE_OTHER,
	PARAM_CODE_TEAM,
	PARAM_CODE_TRIGGER,
	PARAM_CODE_TAG,
	PARAM_CODE_TEAM_AND_TIME,
};


/// <summary>
/// Constructor for the trigger action object.
/// The new action is registered with the action heap and the trackers, but does nothing at
/// all until the scenario reader fills in which action it is and what it acts upon.
/// </summary>
TActionClass::TActionClass(void) :
	HeapID(-1),
	Next(NULL),
	Action(TACTION_NONE),
	Team(NULL),
	Trigger(NULL),
	Tag(NULL),
	TriggerRect(0,0,0,0),
	EffectLocation(0)
{
	Actions.Add(this);
	HeapID = Actions.ID(this);

	AbstractTypePtrTracker.Add(this);
	EventActionPtrTracker.Add(this);
	Data.Value = 0;
}


/// <summary>
/// Destructor for the trigger action object.
/// This routine will sever every reference the game still holds to this action before it
/// drops out of the action heap and the various trackers.
/// </summary>
TActionClass::~TActionClass(void)
{
	Detach_This_From_All(this, true);

	AbstractTypePtrTracker.Delete(this);
	EventActionPtrTracker.Delete(this);
	Actions.Delete(this);
}


/***********************************************************************************************
 * TActionClass::Detach -- Removes any attachment from associated action.                      *
 *                                                                                             *
 *    This routine will remove any action reference to the team type specified. This routine   *
 *    is called when the team type is being destroyed. All references to that team type must   *
 *    also be severed. This routine does that with respect to trigger actions.                 *
 *                                                                                             *
 * INPUT:   target-- The target object or type to remove from this taction object.             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/22/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TActionClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);

	if (Next == target) {
		Next = Next->Next;
	}
	if (Team == target) {
		Team = NULL;
	}
	if (Trigger == target) {
		Trigger = NULL;
	}
	if (Tag == target) {
		Tag = NULL;
	}
}


/***********************************************************************************************
 * TActionClass::Build_INI_Entry -- Builds an INI entry for this trigger action.               *
 *                                                                                             *
 *    This routine will build the text (INI entry) format for the data of this trigger         *
 *    action object. Typical use of this is when the INI file is being written.                *
 *                                                                                             *
 * INPUT:   ptr   -- Pointer to the location to build the INI text to. The buffer is presumed  *
 *                   to be big enough.                                                         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The buffer passed to this routine must be big enough to hold the largest        *
 *             text that will be created into it.                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/22/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TActionClass::Build_INI_Entry(char * ptr) const
{
	NeedType need = Action_Needs(Action);
	ParamCodeType code = PARAM_CODE_OTHER;
	int val = Data.Value;
	char const * str = NULL;

	if (Team != NULL && need == NEED_TEAM_AND_TIME) {
		code = PARAM_CODE_TEAM_AND_TIME;
		if (Team == NULL) {
			val = -1;
		} else {
			val = Team->HeapID;
			str = Team->IniName;
		}
	}
	if (Team != NULL && (need == NEED_TEAM || need == NEED_TEAM_AND_LOCATION)) {
		code = PARAM_CODE_TEAM;
		if (Team == NULL) {
			val = -1;
		} else {
			val = Team->HeapID;
			str = Team->IniName;
		}
	}
	else if (Trigger != NULL && need == NEED_TRIGGER) {
		code = PARAM_CODE_TRIGGER;
		if (Trigger == NULL) {
			val = -1;
		} else {
			val = Trigger->HeapID;
			str = Trigger->IniName;
		}
	}
	else if (Tag != NULL && need == NEED_TAG) {
		code = PARAM_CODE_TAG;
		if (Tag == NULL) {
			val = -1;
		} else {
			val = Tag->HeapID;
			str = Tag->IniName;
		}
	}

	ptr += strlen(ptr);
	if (code == PARAM_CODE_TEAM_AND_TIME) {
		sprintf(ptr, "%d,%d,%s,%d,%d,%d,%d,%d",
			Action,
			code,
			str,
			TriggerRect.X,
			TriggerRect.Y,
			TriggerRect.Width,
			TriggerRect.Height,
			Data.Value
			);
	}
	else if (code != PARAM_CODE_OTHER) {
		sprintf(ptr, "%d,%d,%s,%d,%d,%d,%d,%s",
			Action,
			code,
			str,
			TriggerRect.X,
			TriggerRect.Y,
			TriggerRect.Width,
			TriggerRect.Height,
			Waypoint_To_Name(EffectLocation)
			);
	}
	else {
		sprintf(ptr, "%d,%d,%d,%d,%d,%d,%d,%s",
			Action,
			code,
			val,
			TriggerRect.X,
			TriggerRect.Y,
			TriggerRect.Width,
			TriggerRect.Height,
			Waypoint_To_Name(EffectLocation)
			);
	}
}


/***********************************************************************************************
 * TActionClass::Read_INI -- Converts INI text into appropriate action data.                   *
 *                                                                                             *
 *    This routine will convert INI data into the right values within this trigger action      *
 *    object. Typical use of this routine is when the INI file is being read. It is the        *
 *    counterpart to the Build_INI_Entry function.                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/22/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TActionClass::Read_INI(void)
{
	Data.Value = 0;
	Action = TActionType(atoi(strtok(NULL, ",")));
	ParamCodeType code = (ParamCodeType)atoi(strtok(NULL, ","));
	char * text = strtok(NULL, ",");
	int val = atoi(text);
	switch (code)
	{
		case PARAM_CODE_OTHER:
			Data.Value = val;
			break;

		case PARAM_CODE_TEAM:
		case PARAM_CODE_TEAM_AND_TIME:
			if (val == -1) {
				Team = NULL;
			} else {
				if (strlen(text) < 3) {
					Team = TeamTypes[val];
				} else {
					Team = TeamTypeClass::Find_Or_Make(text);
				}
			}
			break;

		case PARAM_CODE_TRIGGER:
			if (val == -1) {
				Trigger = NULL;
			} else {
				if (strlen(text) < 3) {
					Trigger = TriggerTypes[val];
				} else {
					Trigger = TriggerTypeClass::Find_Or_Make(text);
				}
			}
			break;
		case PARAM_CODE_TAG:
			if (val == -1) {
				Tag = NULL;
			} else {
				if (strlen(text) < 3) {
					Tag = TagTypes[val];
				} else {
					Tag = TagTypeClass::Find_Or_Make(text);
				}
			}
			break;
	}

	TriggerRect.X = atoi(strtok(NULL, ","));
	TriggerRect.Y = atoi(strtok(NULL, ","));
	TriggerRect.Width = atoi(strtok(NULL, ","));
	TriggerRect.Height = atoi(strtok(NULL, ","));
	char *temp = strtok(NULL, ",");
	if (temp != NULL) {
		if (code == PARAM_CODE_TEAM_AND_TIME) {
			Data.Value = atoi(temp);
		}
		else {
			EffectLocation = Waypoint_From_Name(temp);
		}
	}
}


/***********************************************************************************************
 * TActionClass::operator -- Performs the action that this object does.                        *
 *                                                                                             *
 *    This routine is called when the action associated with this action object must be        *
 *    performed. Typically, this occurs when a trigger has "sprung" and now it must take       *
 *    effect. The action object is what carries out this effect.                               *
 *                                                                                             *
 * INPUT:   house -- The owner of this action. This information is necessary since some        *
 *                   actions depend on who the trigger was owned by.                           *
 *                                                                                             *
 *          object-- Pointer to the object that the springing trigger was attached to. If      *
 *                   this parameter is null, then the trigger wasn't attached to any object.   *
 *                                                                                             *
 *          id    -- Trigger ID (only if forced) otherwise -1.                                 *
 *                                                                                             *
 *          cell  -- The cell this trigger is attached to (if any).                            *
 *                                                                                             *
 * OUTPUT:  bool; Was this action able to perform what it needed to do? Failure could be       *
 *                because a reinforcement couldn't be generated, for example.                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/22/1996 JLB : Created.                                                                 *
 *   04/10/1996 JLB : Added the ID parameter.                                                  *
 *=============================================================================================*/
bool TActionClass::operator() (HouseClass * house, ObjectClass * object, TriggerClass * trig, Cell const & cell)
{
	Cell waypoint;
	Coord coord;

	bool success = true;

	/*
	**	Ensure that the specified object is not actually dead. A dead object could
	**	be passed to this routine in the case of a multiple event trigger that
	**	had the first event kill the object.
	*/
	if (object != NULL && !object->IsActive) {
		object = NULL;
	}

	/* 
	 * AircraftClass debug prints expose that mission functions were named TAction_MISSION_X.
	 * This suggests that switches on enums like this used a macro taking the enum as input.
	 * 
	 * Simplifies invoking the right function and checking we aren't missing any case.
	 */
	#define INVOKE(e) case TACTION_ ## e: success = TAction_ ## e (house, object, trig, cell); break;

	switch (Action) {
		INVOKE(TOGGLE_TRAIN_CARGO);
		INVOKE(ION_CANNON);
		INVOKE(MULTI_MISSILE);
		INVOKE(CHEM_MISSILE);
		INVOKE(ACTIVATE_FIRESTORM);
		INVOKE(DEACTIVATE_FIRESTORM);
		INVOKE(GO_BERZERK);
		INVOKE(WAKEUP_SELF);
		INVOKE(VEIN_GROWTH);
		INVOKE(TIB_GROWTH);
		INVOKE(ICE_GROWTH);
		INVOKE(WAKEUP_ALL_SLEEP);
		INVOKE(WAKEUP_ALL_HARMLESS);
		INVOKE(WAKEUP_GROUP);
		INVOKE(ANNOUNCE_WIN);
		INVOKE(ANNOUNCE_LOSE);
		INVOKE(FORCE_END);
		INVOKE(DAMAGE);
		INVOKE(LIGHT_SMALL);
		INVOKE(LIGHT_MEDIUM);
		INVOKE(LIGHT_LARGE);
		INVOKE(SELL_ATTACHED);
		INVOKE(TURN_OFF_ATTACHED);
		INVOKE(TURN_ON_ATTACHED);
		INVOKE(CHANGE_HOUSE);
		INVOKE(ALL_CHANGE_HOUSE);
		INVOKE(TEXT_TRIGGER);
		INVOKE(MAKE_ALLY);
		INVOKE(MAKE_ENEMY);
		INVOKE(PREFERRED_TARGET);
		INVOKE(BASE_BUILDING);
		INVOKE(CREEP_SHADOW);
		INVOKE(SET_GLOBAL);
		INVOKE(CLEAR_GLOBAL);
		INVOKE(REDUCE_TIBERIUM);
		INVOKE(REVEAL_SOME);
		INVOKE(REVEAL_ZONE);
		INVOKE(REVEAL_ALL);
		INVOKE(START_TIMER);
		INVOKE(STOP_TIMER);
		INVOKE(ADD_TIMER);
		INVOKE(SUB_TIMER);
		INVOKE(SET_TIMER);
		INVOKE(PLAY_MOVIE);
		INVOKE(PLAY_SOUND);
		INVOKE(PLAY_SOUND_RANDOM);
		INVOKE(PLAY_SOUND_AT);
		INVOKE(PLAY_MUSIC);
		INVOKE(PLAY_SPEECH);
		INVOKE(1_SPECIAL);
		INVOKE(FULL_SPECIAL);
		INVOKE(DZ);
		INVOKE(WIN);
		INVOKE(LOSE);
		INVOKE(BEGIN_PRODUCTION);
		INVOKE(FIRE_SALE);
		INVOKE(AUTOCREATE);
		INVOKE(CREATE_TEAM);
		INVOKE(DESTROY_TEAM);
		INVOKE(REINFORCEMENTS);
		INVOKE(REINFORCEMENTS_SPECIAL);
		INVOKE(ALL_HUNT);
		INVOKE(DESTROY_OBJECT);
		INVOKE(CHANGE_ZOOM);
		INVOKE(RESIZE_PLAYER_VIEW);
		INVOKE(PLAY_ANIM);
		INVOKE(DO_EXPLOSION);
		INVOKE(METEOR_IMPACT);
		INVOKE(ION_STORM_START);
		INVOKE(ION_STORM_STOP);
		INVOKE(LOCK_INPUT);
		INVOKE(UNLOCK_INPUT);
		INVOKE(CENTER_VIEWPOINT);
		INVOKE(ZOOM_IN);
		INVOKE(ZOOM_OUT);
		INVOKE(RESHROUD);
		INVOKE(CHANGE_SPOTLIGHT_BEHAVIOR);
		INVOKE(DESTROY_TRIGGER);
		INVOKE(DESTROY_TAG);
		INVOKE(FORCE_TRIGGER);
		INVOKE(ENABLE_TRIGGER);
		INVOKE(DISABLE_TRIGGER);
		INVOKE(RADAR_EVENT);
		INVOKE(SET_LOCAL);
		INVOKE(CLEAR_LOCAL);
		INVOKE(METEOR_SHOWER);
		INVOKE(SET_AMBIENT_STEP);
		INVOKE(SET_AMBIENT_RATE);
		INVOKE(SET_AMBIENT_LIGHT);
		INVOKE(BEGIN_AI_TRIGGERS);
		INVOKE(STOP_AI_TRIGGERS);
		INVOKE(SET_AI_TRIGGER_TEAM_RATIO);
		INVOKE(SET_TEAM_AIRCRAFT_RATIO);
		INVOKE(SET_TEAM_INFANTRY_RATIO);
		INVOKE(SET_TEAM_UNIT_RATIO);
		INVOKE(PARTICLE_ANIM);
		INVOKE(REMOVE_PARTICLE_ANIM);
		INVOKE(ION_LIGHTNING_STRIKE);
		INVOKE(PLAY_INGAME_MOVIE);
		INVOKE(FLASH_TEAM);
		INVOKE(DISABLE_SPEECH);
		INVOKE(ENABLE_SPEECH);
		INVOKE(SET_GROUP_ID);
		INVOKE(TALK_BUBBLE);

		/*
		**	Do no action at all.
		*/
		case TACTION_NONE:
			break;

		default:
			break;
	}
	return(success);
}


/// <summary>
/// Switches the house's firestorm defense off.
/// The defense is toggled by firing the house's firestorm super weapon, so this
/// action does nothing when the wall is already down.
/// </summary>
bool TActionClass::TAction_DEACTIVATE_FIRESTORM(HouseClass * house, ObjectClass *, TriggerClass *, Cell const & )
{
	if (house->FirestormDefenseActivated) {
		int index = -1;
		for (int i = 0; i < house->SuperWeapon.Count(); i++) {
			SuperClass * super = house->SuperWeapon[i];
			if (super->Class->Type == SUPER_FIRESTORM) {
				index = i;
				break;
			}
		}

		if (index != -1) {
			house->Place_Special_Blast((SuperWeaponType)index, Cell(0, 0));
		}
	}
	return(true);
}


/// <summary>
/// Switches the house's firestorm defense on.
/// The defense is toggled by firing the house's firestorm super weapon, so this
/// action does nothing when the wall is already up.
/// </summary>
bool TActionClass::TAction_ACTIVATE_FIRESTORM(HouseClass * house, ObjectClass *, TriggerClass *, Cell const & )
{
	if (!house->FirestormDefenseActivated) {
		int index = -1;
		for (int i = 0; i < house->SuperWeapon.Count(); i++) {
			SuperClass * super = house->SuperWeapon[i];
			if (super->Class->Type == SUPER_FIRESTORM) {
				index = i;
				break;
			}
		}

		if (index != -1) {
			house->Place_Special_Blast((SuperWeaponType)index, Cell(0, 0));
		}
	}
	return(true);
}


/// <summary>
/// Calls down a lightning bolt at the effect waypoint.
/// </summary>
bool TActionClass::TAction_ION_LIGHTNING_STRIKE(HouseClass *, ObjectClass *, TriggerClass *, Cell const & )
{
	IonStormClass::Lightning_Bolt(Scen->Get_Waypoint_Cell(EffectLocation));
	return(true);
}


/// <summary>
/// Removes the particle systems at the effect waypoint.
/// </summary>
bool TActionClass::TAction_REMOVE_PARTICLE_ANIM(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Cell cell2 = Scen->Get_Waypoint_Cell(EffectLocation);

	for (int index = 0; index < ParticleSystems.Count(); index++) {
		ParticleSystemClass * psys = ParticleSystems[index];

		if (psys->IsActive && psys->PositionCell == cell2) {
			psys->Delete_Me();
		}
	}
	return(true);
}


/// <summary>
/// Creates a particle system at the effect waypoint.
/// The system is placed on the ground at the waypoint, and its type comes from
/// the trigger action's own data.
/// </summary>
bool TActionClass::TAction_PARTICLE_ANIM(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Cell cell2 = Scen->Get_Waypoint_Cell(EffectLocation);
	Coord coord = Coord(cell2);
	coord.Z = Map.Get_Height_GL(cell2);
	new ParticleSystemClass(ParticleSystemTypes[Data.PAnim], coord, NULL);
	return(true);
}


/// <summary>
/// Wakes the objects this trigger is attached to.
/// Sleeping and harmless objects carrying the trigger's tag are put back on
/// guard. Buildings are left as they are.
/// </summary>
bool TActionClass::TAction_WAKEUP_SELF(HouseClass * , ObjectClass * , TriggerClass * trigger, Cell const & )
{
	for (int index = 0; index < Technos.Count(); index++) {
		TechnoClass * techno = Technos[index];
		if (techno->RTTI != RTTI_BUILDING && techno->IsActive && techno->IsDown && techno->Tag != NULL && techno->Tag->Is_Trigger_Attached(trigger)) {
			if (techno->Mission == MISSION_SLEEP || techno->Mission == MISSION_HARMLESS) {
				techno->Assign_Mission(MISSION_GUARD);
			}
		}
	}
	return(true);
}


/// <summary>
/// Turns vein growth on or off for the scenario.
/// </summary>
bool TActionClass::TAction_VEIN_GROWTH(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Scen->IsVeinGrowth = Data.Bool;
	return(true);
}


/// <summary>
/// Turns tiberium growth on or off for the scenario.
/// </summary>
bool TActionClass::TAction_TIB_GROWTH(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Scen->IsTibGrowth = Data.Bool;
	return(true);
}


/// <summary>
/// Turns ice growth on or off for the scenario.
/// </summary>
bool TActionClass::TAction_ICE_GROWTH(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Scen->IsIceGrowth = Data.Bool;
	return(true);
}


/// <summary>
/// Wakes every sleeping unit the house does not own.
/// Used to rouse a dormant enemy force once the player trips the trigger.
/// </summary>
bool TActionClass::TAction_WAKEUP_ALL_SLEEP(HouseClass * house, ObjectClass * , TriggerClass * , Cell const & )
{
	for (int index = 0; index < Feet.Count(); index++) {
		FootClass * techno = Feet[index];
		if (techno->Mission == MISSION_SLEEP && techno->Strength > 0 && techno->IsActive && techno->IsDown && !techno->IsInLimbo && house != techno->House ) {
			techno->Assign_Mission(MISSION_GUARD);
		}
	}
	return(true);
}


/// <summary>
/// Wakes every harmless unit the house owns.
/// </summary>
bool TActionClass::TAction_WAKEUP_ALL_HARMLESS(HouseClass * house, ObjectClass * , TriggerClass * , Cell const & )
{
	for (int index = 0; index < Feet.Count(); index++) {
		FootClass * techno = Feet[index];
		if (techno->Mission == MISSION_HARMLESS && techno->Strength > 0 && techno->IsActive && techno->IsDown && !techno->IsInLimbo && techno->House == house) {
			techno->Assign_Mission(MISSION_GUARD);
		}
	}
	return(true);
}


/// <summary>
/// Wakes every dormant unit in the team group.
/// Sleeping and harmless units whose group matches the trigger action's own
/// data are put back on guard.
/// </summary>
bool TActionClass::TAction_WAKEUP_GROUP(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	for (int index = 0; index < Technos.Count(); index++) {
		FootClass * foot = Feet[index];
		if (foot->Group == Data.Value &&
				(foot->Mission == MISSION_SLEEP || foot->Mission == MISSION_HARMLESS) &&
				foot->Strength > 0 &&
				foot->IsActive &&
				foot->IsDown &&
				!foot->IsInLimbo) {
			foot->Assign_Mission(MISSION_GUARD);
		}
	}
	return(true);
}


/// <summary>
/// Flags the player as having won the scenario.
/// </summary>
bool TActionClass::TAction_ANNOUNCE_WIN(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	PlayerPtr->Flag_To_Win(true);
	return(true);
}


/// <summary>
/// Flags the player as having lost the scenario.
/// </summary>
bool TActionClass::TAction_ANNOUNCE_LOSE(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	PlayerPtr->Flag_To_Lose(true);
	return(true);
}


/// <summary>
/// Brings the scenario to an immediate end.
/// </summary>
bool TActionClass::TAction_FORCE_END(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	PlayerPtr->Flag_To_End();
	return(true);
}


/// <summary>
/// Detonates a cluster of explosions at the waypoint.
/// This routine handles the trigger action that demolishes a scripted target. Any bridge
/// spanning the waypoint is knocked down as well, and the tactical map is told to redraw
/// the wreckage.
/// </summary>
bool TActionClass::TAction_DAMAGE(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	int damage = 100;
	Explosion_Damage(Waypoint_As_Coord(), damage, NULL, Rule->C4Warhead);
	damage = 100;
	Explosion_Damage(Waypoint_As_Coord() + Coord(CELL_LEPTON_W/3, CELL_LEPTON_H/3, 0), damage, NULL, Rule->C4Warhead);
	damage = 100;
	Explosion_Damage(Waypoint_As_Coord() + Coord(-CELL_LEPTON_W/3, CELL_LEPTON_H/3, 0), damage, NULL, Rule->C4Warhead);
	damage = 100;
	Explosion_Damage(Waypoint_As_Coord() + Coord(CELL_LEPTON_W/3, -CELL_LEPTON_H/3, 0), damage, NULL, Rule->C4Warhead);
	damage = 100;
	Explosion_Damage(Waypoint_As_Coord() + Coord(-CELL_LEPTON_W/3, -CELL_LEPTON_H/3, 0), damage, NULL, Rule->C4Warhead);

	if (Scen->Is_Valid_Waypoint(Data.Value)) {
		Cell cell = Scen->Get_Waypoint_Cell(Data.Value);
		int tries=3;
		while (Map.Damage_Bridge(cell) == false && tries-- > 0) {};
		Point2D point;
		Coord coord = Coord(cell);
		coord.Z += BRIDGE_LEPTON_HEIGHT;
		TacticalMap->Coord_To_Pixel(coord, point);
		Rect dirty (point.X-128, point.Y-128, 256, 256);
		TacticalMap->Register_Dirty_Area(dirty);
	}

	return(true);
}


/// <summary>
/// Flashes a small lighting burst at the waypoint.
/// This routine handles the trigger action used to light a scripted explosion. Nothing is
/// harmed -- the flash is for the benefit of the audience.
/// </summary>
bool TActionClass::TAction_LIGHT_SMALL(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	int damage = 50;
	Combat_Lighting(Waypoint_As_Coord(), damage, Rule->C4Warhead);
	return(true);
}


/// <summary>
/// Flashes a medium lighting burst at the waypoint.
/// This routine handles the trigger action used to light a scripted explosion. Nothing is
/// harmed -- the flash is for the benefit of the audience.
/// </summary>
bool TActionClass::TAction_LIGHT_MEDIUM(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	int damage = 100;
	Combat_Lighting(Waypoint_As_Coord(), damage, Rule->C4Warhead);
	return(true);
}


/// <summary>
/// Flashes a large lighting burst at the waypoint.
/// This routine handles the trigger action used to light a scripted explosion. Nothing is
/// harmed -- the flash is for the benefit of the audience.
/// </summary>
bool TActionClass::TAction_LIGHT_LARGE(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	int damage = 300;
	Combat_Lighting(Waypoint_As_Coord(), damage, Rule->C4Warhead);
	return(true);
}


/// <summary>
/// Sells the buildings attached to the trigger.
/// This routine handles the trigger action that refunds the tagged structures back to
/// whoever owns them at the time.
/// </summary>
/// <returns>bool; Was any building sold?</returns>
bool TActionClass::TAction_SELL_ATTACHED(HouseClass * , ObjectClass * , TriggerClass * trig, Cell const & )
{
	bool success = false;

	for (int index = 0; index < Buildings.Count(); index++) {
		BuildingClass * building = Buildings[index];
		if (building->IsActive &&
				building->IsDown &&
				!building->IsInLimbo &&
				building->Tag != NULL &&
				building->Tag->Is_Trigger_Attached(trig)) {

			building->Sell_Back(1);
			success = true;
		}
	}
	return(success);
}


/// <summary>
/// Sends the infantry attached to the trigger berserk.
/// This routine handles the trigger action that drives the tagged infantry into a rampage
/// against whatever happens to be nearby.
/// </summary>
/// <returns>bool; Did any infantry go berserk?</returns>
bool TActionClass::TAction_GO_BERZERK(HouseClass * , ObjectClass * , TriggerClass * trig, Cell const & )
{
	bool success = false;

	for (int index = 0; index < Infantry.Count(); index++) {
		InfantryClass * infantry = Infantry[index];
		if (infantry->IsActive &&
				infantry->IsDown &&
				!infantry->IsInLimbo &&
				infantry->Tag != NULL &&
				infantry->Tag->Is_Trigger_Attached(trig)) {

			infantry->Berzerk();
			success = true;
		}
	}
	return(success);
}


/// <summary>
/// Turns off the buildings attached to the trigger.
/// This routine handles the trigger action that shuts tagged structures down so that they
/// stop functioning until something switches them back on.
/// </summary>
/// <returns>bool; Was any building turned off?</returns>
bool TActionClass::TAction_TURN_OFF_ATTACHED(HouseClass * , ObjectClass * , TriggerClass * trig, Cell const & )
{
	bool success = false;
	for (int index = 0; index < Buildings.Count(); index++) {
		BuildingClass * building = Buildings[index];
		if (building->IsActive &&
				building->IsDown &&
				!building->IsInLimbo &&
				building->Tag != NULL &&
				building->Tag->Is_Trigger_Attached(trig) &&
				building->IsOn) {

			building->Turn_Off();
			success = true;
		}
	}
	return(success);
}


/// <summary>
/// Turns on the buildings attached to the trigger.
/// This routine handles the trigger action that brings tagged structures back into service
/// after they have been switched off.
/// </summary>
/// <returns>bool; Was any building turned on?</returns>
bool TActionClass::TAction_TURN_ON_ATTACHED(HouseClass * , ObjectClass * , TriggerClass * trig, Cell const & )
{
	bool success = false;
	for (int index = 0; index < Buildings.Count(); index++) {
		BuildingClass * building = Buildings[index];
		if (building->IsActive && building->IsDown && !building->IsInLimbo && building->Tag != NULL && building->Tag->Is_Trigger_Attached(trig) && !building->IsOn) {
			building->Turn_On();
			success = true;
		}
	}
	return(success);
}


/// <summary>
/// Hands the objects attached to the trigger over to another house.
/// This routine handles the trigger action that captures the tagged units and buildings for
/// the house named in the action data.
/// </summary>
/// <returns>bool; Was anything captured?</returns>
bool TActionClass::TAction_CHANGE_HOUSE(HouseClass * , ObjectClass * , TriggerClass * trig, Cell const & )
{
	bool success = false;

	for (int index = 0; index < Technos.Count(); index++) {
		TechnoClass * techno = Technos[index];
		if (techno->IsActive &&
				techno->IsDown &&
				!techno->IsInLimbo &&
				techno->Tag != NULL &&
				techno->Tag->Is_Trigger_Attached(trig)) {

			techno->Captured(House_From_HousesType(Data.House));
			success = true;
		}
	}
	return(success);
}


/// <summary>
/// Assigns the triggering object to a numbered group.
/// This routine handles the trigger action that drops an object into one of the player's
/// team hotkey groups. Anything that is not a techno is quietly ignored.
/// </summary>
/// <param name="object">The object that sprung the trigger.</param>
bool TActionClass::TAction_SET_GROUP_ID(HouseClass * , ObjectClass * object, TriggerClass *, Cell const & )
{
	if (object && object->Is_Techno()) {
		((TechnoClass *)object)->Group = Data.Value;
	}
	return(true);
}


/// <summary>
/// Hands every object owned by a house over to another house.
/// This routine handles the trigger action that defects an entire side, units and buildings
/// alike, to the house named in the action data.
/// </summary>
/// <param name="house">The house whose forces are to change hands.</param>
/// <returns>bool; Was anything captured?</returns>
bool TActionClass::TAction_ALL_CHANGE_HOUSE(HouseClass * house, ObjectClass * , TriggerClass * trig, Cell const & )
{
	bool retval = false;

	HouseClass * hptr = House_From_HousesType(Data.House);

	for (int index = 0; index < Technos.Count(); index++) {
		if (Technos[index]->House == house) {
			Technos[index]->Captured(hptr);
			retval = true;
		}
	}

	return(retval);
}


/// <summary>
/// Displays a line of tutorial text to the player.
/// This routine handles the trigger action that posts a message onto the tactical map. The
/// message expires after the message delay specified in the rules.
/// </summary>
bool TActionClass::TAction_TEXT_TRIGGER(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	/*
	**	Display a text message overlayed onto the tactical map.
	*/
	Session.Messages.Add_Message(NULL, 0, TutorialText[Data.Value], 0, TextPrintType(TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW), Rule->MessageDelay * TICKS_PER_MINUTE);
	return(true);
}


/// <summary>
/// Makes two houses allies of one another.
/// This routine handles the trigger action that forms an alliance. Both houses turn
/// friendly, so the arrangement is mutual.
/// </summary>
bool TActionClass::TAction_MAKE_ALLY(HouseClass * house, ObjectClass * , TriggerClass * trig, Cell const & )
{
	if (Data.House != HOUSE_NONE) {
		HouseClass * house2 = House_From_HousesType(Data.House);
		house->Make_Ally(house2);
		house2->Make_Ally(house);
	}
	return(true);
}


/// <summary>
/// Makes two houses enemies of one another.
/// This routine handles the trigger action that breaks an alliance. Both houses turn
/// hostile, so the falling out is mutual.
/// </summary>
bool TActionClass::TAction_MAKE_ENEMY(HouseClass * house, ObjectClass * , TriggerClass * trig, Cell const & )
{
	if (Data.House != HOUSE_NONE) {
		HouseClass * house2 = House_From_HousesType(Data.House);
		house->Make_Enemy(house2);
		house2->Make_Enemy(house);
	}
	return(true);
}


/// <summary>
/// Sets the preferred target for the house.
/// This routine handles the trigger action that tells a computer house what sort of quarry
/// its attack teams should hunt for from now on.
/// </summary>
bool TActionClass::TAction_PREFERRED_TARGET(HouseClass * house, ObjectClass * , TriggerClass * , Cell const & )
{
	if (house != NULL) {
		house->PreferredTarget = Data.Quarry;
	}
	return(true);
}


/// <summary>
/// Turns base building on or off for the house.
/// This routine handles the trigger action that lets a computer house begin or abandon
/// expanding its base. When switched on, the base is re-centered on the house's
/// construction yard so that new structures cluster around it.
/// </summary>
bool TActionClass::TAction_BASE_BUILDING(HouseClass * house, ObjectClass * , TriggerClass * , Cell const & )
{
	if (Data.Bool) {
		house->IsBaseBuilding = true;
		if (house->ConYards.Count() > 0) {
			BuildingClass *building = house->ConYards[0];
			Cell base_cell = building->Get_Coord().As_Cell();
			house->Center = Coord(base_cell);
			house->Base.PlacementCenter = base_cell;
		}
	} else {
		house->IsBaseBuilding = false;
	}
	return(true);
}


/// <summary>
/// Creeps the shadow back in over the map.
/// This routine handles the trigger action that re-shrouds terrain the player is no longer
/// keeping an eye on.
/// </summary>
bool TActionClass::TAction_CREEP_SHADOW(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Map.Encroach_Shadow();
	return(true);
}


/// <summary>
/// Sets a scenario global variable.
/// This routine handles the trigger action that switches a global on. Mission designers use
/// these globals to chain one scenario event onto another.
/// </summary>
bool TActionClass::TAction_SET_GLOBAL(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Scen->Set_Global_To(Data.Value, true);
	return(true);
}


/// <summary>
/// Clears a scenario global variable.
/// This routine handles the trigger action that switches a global off so that any trigger
/// watching for it will stop firing.
/// </summary>
bool TActionClass::TAction_CLEAR_GLOBAL(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Scen->Set_Global_To(Data.Value, false);
	return(true);
}


/// <summary>
/// Reveals the map around a waypoint.
/// This routine handles the trigger action that uncovers a patch of terrain for the player.
/// The radius comes from the rules file, and nothing is revealed once the player has already
/// been granted full vision.
/// </summary>
bool TActionClass::TAction_REVEAL_SOME(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (!PlayerPtr->IsVisionary) {
		Cell waypoint = Scen->Get_Waypoint_Cell(Data.Value);
		int height = Map[waypoint].Height + ((Map[waypoint].IsUnderBridge || Map[waypoint].WasUnderBridge) ? BRIDGE_CELL_HEIGHT : 0);
		Map.Sight_From(Coord(waypoint - Cell(height/2, height/2)) + Coord(0, 0, height * LEVEL_LEPTON_H), Rule->RevealTriggerRadius, PlayerPtr, false ,false, false, true);
	}
	return(true);
}


/// <summary>
/// Reduces the tiberium growing around a waypoint.
/// This routine handles the trigger action that thins out a tiberium field, typically to
/// starve a harvesting opponent.
/// </summary>
bool TActionClass::TAction_REDUCE_TIBERIUM(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Cell cell1 = Scen->Get_Waypoint_Cell(Data.Value);
	Map.Area_Reduce_Tiberium(cell1);
	return(true);
}


/// <summary>
/// Reveals the movement zone that a waypoint sits in.
/// This routine handles the trigger action that uncovers a whole landmass at once, leaving
/// terrain the player cannot drive to still shrouded.
/// </summary>
bool TActionClass::TAction_REVEAL_ZONE(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (!PlayerPtr->IsVisionary) {
		int zone = Map.Get_Cell_Zone(Scen->Get_Waypoint_Cell(Data.Value), MZONE_CRUSHER);

		Map.Reset_Local_Iterator();
		CellClass *cellptr = Map.Local_Iterate();

		while (cellptr) {
			if (Map.Get_Cell_Zone(cellptr->CellID, MZONE_CRUSHER) == zone) {
				int height = cellptr->Height/2;
				Map.Sight_From(Coord(cellptr->CellID - Cell(height/2, height/2)) + Coord(0, 0, height * LEVEL_LEPTON_H), 2, PlayerPtr, false, false, false, true);
			}
			cellptr = Map.Local_Iterate();
		}

	}
	return(true);
}


/// <summary>
/// Reveals the entire map to the player.
/// This routine handles the trigger action that lifts the shroud everywhere. The player is
/// marked as visionary so that the later reveal actions know there is nothing left to
/// uncover.
/// </summary>
bool TActionClass::TAction_REVEAL_ALL(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (!PlayerPtr->IsVisionary) {
		PlayerPtr->IsVisionary = true;

		Map.Reset_Iterator();
		CellClass *cellptr = Map.Iterate();

		while (cellptr) {
			Map.Map_Cell(cellptr->CellID, PlayerPtr);
			cellptr = Map.Iterate();
		}
		Map.Flag_To_Redraw(GS_REDRAW_TACTICAL);
	}
	return(true);
}


/// <summary>
/// Starts the mission countdown timer.
/// This routine handles the trigger action that sets a timed objective running. The player
/// is told over the EVA channel and the tab is refreshed so the clock appears.
/// </summary>
bool TActionClass::TAction_START_TIMER(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (!Scen->MissionTimer.Is_Active()) {
		Speak(VOX_TIMER_STARTED);
		Scen->MissionTimer.Start();
		Map.Redraw_Tab();
	}
	return(true);
}


/// <summary>
/// Stops the mission countdown timer.
/// This routine handles the trigger action that calls off a timed objective. The player is
/// told over the EVA channel and the tab is refreshed so the clock disappears.
/// </summary>
bool TActionClass::TAction_STOP_TIMER(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (Scen->MissionTimer.Is_Active()) {
		Speak(VOX_TIMER_STOPPED);
		Scen->MissionTimer.Stop();
		Map.Redraw_Tab();
	}
	return(true);
}


/// <summary>
/// Adds seconds to the mission timer.
/// This routine is used to give the player a reprieve on a timed objective.
/// </summary>
bool TActionClass::TAction_ADD_TIMER(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Scen->MissionTimer = Scen->MissionTimer + Data.Value * TICKS_PER_SECOND;
	Map.Redraw_Tab();
	return(true);
}


/// <summary>
/// Takes seconds off the mission timer.
/// The timer will not run past zero, so a generous subtraction merely expires it.
/// </summary>
bool TActionClass::TAction_SUB_TIMER(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (Scen->MissionTimer <= Data.Value * TICKS_PER_SECOND) {
		Scen->MissionTimer = 0;
	} else {
		Scen->MissionTimer = Scen->MissionTimer - Data.Value * TICKS_PER_SECOND;
	}
	Map.Redraw_Tab();
	return(true);
}


/// <summary>
/// Starts the mission timer running.
/// The timer is set to the number of seconds the action calls for and EVA announces
/// the countdown to the player.
/// </summary>
bool TActionClass::TAction_SET_TIMER(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Speak(VOX_TIMER_STARTED);
	Scen->MissionTimer = Data.Value * TICKS_PER_SECOND;
	Scen->MissionTimer.Start();
	Map.Redraw_Tab();
	return(true);
}


/// <summary>
/// Plays a full screen movie.
/// The mission is interrupted while the movie runs, and no music plays over it.
/// </summary>
bool TActionClass::TAction_PLAY_MOVIE(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Play_Movie(Data.Movie, THEME_NONE, true);
	return(true);
}


/// <summary>
/// Plays a movie without leaving the battlefield.
/// This routine is used for the small in-game transmissions, which play over the
/// mission rather than interrupting it the way PLAY_MOVIE does.
/// </summary>
bool TActionClass::TAction_PLAY_INGAME_MOVIE(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Play_Ingame_Movie(Data.Movie);
	return(true);
}


/// <summary>
/// Plays a sound effect.
/// The sound has no position on the map, so the player hears it wherever the view
/// happens to be.
/// </summary>
bool TActionClass::TAction_PLAY_SOUND(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Sound_Effect(Data.Sound);
	return(true);
}


/// <summary>
/// Plays a sound effect at a randomly chosen waypoint.
/// This routine is used to make a noise somewhere out on the map without the designer
/// having to say where.
/// </summary>
bool TActionClass::TAction_PLAY_SOUND_RANDOM(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Cell list[100];
	int count = 0;

	for (int index = 0; index < WAYPT_COUNT; index++) {
		if (Scen->Is_Valid_Waypoint(index)) {
			list[count++] = Scen->Get_Waypoint_Cell(index);
			if (count == sizeof(list)) break;
		}
	}

	Sound_Effect(Data.Sound, Coord(list[Random_Pick(0, count-1)]));
	return(true);
}


/// <summary>
/// Plays a sound effect at the trigger's waypoint.
/// The sound is positioned on the map, so the player hears it only while the view is
/// somewhere near the waypoint.
/// </summary>
bool TActionClass::TAction_PLAY_SOUND_AT(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Sound_Effect(Data.Sound, Scen->Get_Waypoint_Coord(EffectLocation));
	return(true);
}


/// <summary>
/// Queues up a music theme.
/// The new theme takes over once whatever is currently playing has finished.
/// </summary>
bool TActionClass::TAction_PLAY_MUSIC(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Theme.Queue_Song(Data.Theme);
	return(true);
}


/// <summary>
/// Plays an EVA speech line.
/// </summary>
bool TActionClass::TAction_PLAY_SPEECH(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Speak(Data.Speech);
	return(true);
}


/// <summary>
/// Grants a house a single shot of a super weapon.
/// This routine is used to hand out a one-off special weapon, adding its button to
/// the sidebar if the local player is the one being favored.
/// </summary>
bool TActionClass::TAction_1_SPECIAL(HouseClass * house, ObjectClass * , TriggerClass * , Cell const & )
{
	house->SuperWeapon[Data.Special]->Enable(true, false);

	if (PlayerPtr == house) {
		Map.Add(RTTI_SPECIAL, Data.Special);
		Map.Column[1].Flag_To_Redraw();
	}
	return(true);
}


/// <summary>
/// Grants a house a super weapon outright.
/// This routine differs from 1_SPECIAL in that the weapon keeps recharging for as
/// long as the house lives, and it no longer needs the building that normally
/// provides it. The recipient's sidebar gains the button for it.
/// </summary>
bool TActionClass::TAction_FULL_SPECIAL(HouseClass * house, ObjectClass * , TriggerClass * , Cell const & )
{
	house->SuperWeapon[Data.Special]->Enable(false, false);
	house->SuperWeapon[Data.Special]->NeedsBuilding = false;

	if (PlayerPtr == house) {
		Map.Add(RTTI_SPECIAL, Data.Special);
		Map.Column[1].Flag_To_Redraw();
	}
	return(true);
}


/// <summary>
/// Drops a flare on the trigger's waypoint.
/// This routine marks a drop zone so the player knows where reinforcements are about
/// to arrive. The flare is inert and sits on top of any bridge over the cell.
/// </summary>
bool TActionClass::TAction_DZ(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Cell waypoint = Scen->Get_Waypoint_Cell(EffectLocation);
	Coord coord = Coord(waypoint);
	coord.Z = Map.Get_Height_GL(coord);
	if ( Map[waypoint].IsUnderBridge || Map[waypoint].WasUnderBridge ) {
		coord.Z += BRIDGE_LEPTON_HEIGHT;
	}

	AnimClass * anim = new AnimClass(Rule->FlareAnim, coord);
	anim->IsInert = true;
	return(true);
}


/// <summary>
/// Declares the mission won for a house.
/// The player wins if the action names their own house, and loses if it names
/// somebody else's.
/// </summary>
bool TActionClass::TAction_WIN(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (Data.House == PlayerPtr->Class->House) {
		PlayerPtr->Flag_To_Win();
	} else {
		PlayerPtr->Flag_To_Lose();
	}
	return(true);
}


/// <summary>
/// Declares the mission lost for a house.
/// The player loses if the action names their own house, and wins if it names
/// somebody else's.
/// </summary>
bool TActionClass::TAction_LOSE(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (Data.House != PlayerPtr->Class->House) {
		PlayerPtr->Flag_To_Win();
	} else {
		PlayerPtr->Flag_To_Lose();
	}
	return(true);
}


/// <summary>
/// Allows a house to start building.
/// This routine is used to hold a computer house idle until the mission is ready for
/// it to start turning out units and structures.
/// </summary>
bool TActionClass::TAction_BEGIN_PRODUCTION(HouseClass * , ObjectClass * , TriggerClass * trig, Cell const & )
{
	if (Data.House != HOUSE_NONE) {
		House_From_HousesType(Data.House)->Begin_Production();
	}
	return(true);
}


/// <summary>
/// Orders a house to sell off everything it owns.
/// The house is put into its endgame state, where it liquidates its base and throws
/// whatever that buys at the enemy.
/// </summary>
bool TActionClass::TAction_FIRE_SALE(HouseClass * , ObjectClass * , TriggerClass * trig, Cell const & )
{
	if (Data.House != HOUSE_NONE) {
		House_From_HousesType(Data.House)->State = STATE_ENDGAME;
	}
	return(true);
}


/// <summary>
/// Puts a house on alert.
/// An alerted house will build and dispatch its autocreate teams, which is how a
/// computer opponent is woken up part way through a mission.
/// </summary>
bool TActionClass::TAction_AUTOCREATE(HouseClass * , ObjectClass * , TriggerClass * trig, Cell const & )
{
	if (Data.House != HOUSE_NONE) {
		House_From_HousesType(Data.House)->IsAlerted = true;
	}
	return(true);
}


/// <summary>
/// Creates one team of the trigger's team type.
/// The team is assembled as if the scenario were still initializing, so its members
/// are gathered without the usual production fanfare.
/// </summary>
bool TActionClass::TAction_CREATE_TEAM(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	ScenarioInit++;
	if (Team != NULL) {
		Team->Create_One_Of();
	}
	ScenarioInit--;
	return(true);
}


/// <summary>
/// Disbands every team of the trigger's team type.
/// The members themselves survive; only the teams that were holding them together
/// are broken up.
/// </summary>
bool TActionClass::TAction_DESTROY_TEAM(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (Team != NULL) {
		Team->Destroy_All_Of();
	}
	return(true);
}


/// <summary>
/// Flashes the members of the trigger's team.
/// This routine is used to point the player at a particular team, usually alongside a
/// line of briefing text that refers to it.
/// </summary>
bool TActionClass::TAction_FLASH_TEAM(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (Team != NULL) {
		Team->Flash(Data.Value);
	}
	return(true);
}


/// <summary>
/// Silences the EVA speech channel.
/// This routine is used to keep EVA from talking over a scripted sequence.
/// </summary>
bool TActionClass::TAction_DISABLE_SPEECH(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Set_Speech_State(false);
	return(true);
}


/// <summary>
/// Allows EVA to speak again.
/// This routine undoes DISABLE_SPEECH once the sequence it was covering is over.
/// </summary>
bool TActionClass::TAction_ENABLE_SPEECH(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Set_Speech_State(true);
	return(true);
}


/// <summary>
/// Hangs a talk bubble over a member of the trigger's team.
/// This routine is used to put words in a character's mouth during a scripted
/// conversation. A trigger with no team clears the current talker instead.
/// </summary>
bool TActionClass::TAction_TALK_BUBBLE(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (Team != NULL) {
		TeamClass * team = Team->Find_First_Of_Type();

		if (team != NULL) {
			TechnoClass * talker = team->Get_Member();
			TechnoClass::Set_Talker(talker, (TalkType)Data.Value);
		}
	} else {
		TechnoClass::Set_Talker(NULL);
	}

	return(true);
}


/// <summary>
/// Delivers the trigger's team as reinforcements.
/// The team arrives by whatever means its team type specifies, so this covers
/// everything from a truck driving on to a dropship landing.
/// </summary>
/// <returns>bool; Were the reinforcements delivered?</returns>
bool TActionClass::TAction_REINFORCEMENTS(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (Team != NULL) {
		return(Do_Reinforcements(Team));
	}
	return(false);
}


/// <summary>
/// Delivers the trigger's team as reinforcements at a chosen waypoint.
/// This routine differs from REINFORCEMENTS in that the team arrives at the
/// waypoint the action names rather than the one its own team type calls for.
/// </summary>
/// <returns>bool; Were the reinforcements delivered?</returns>
bool TActionClass::TAction_REINFORCEMENTS_SPECIAL(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (Team != NULL && EffectLocation != -1) {
		return(Do_Reinforcements(Team, EffectLocation));
	}
	return(false);
}


/// <summary>
/// Sends every unit of a house off to hunt.
/// This routine is used to throw a house at the player all at once, typically as a
/// last ditch reaction to it losing its base.
/// </summary>
bool TActionClass::TAction_ALL_HUNT(HouseClass * , ObjectClass * , TriggerClass * trig, Cell const & )
{
	House_From_HousesType(Data.House)->All_To_Hunt();
	return(true);
}


/// <summary>
/// Destroys every object attached to this trigger.
/// The objects are killed by damage rather than simply removed, so they die with the
/// explosions and death animations the player expects. When the trigger names a cell,
/// any bridge spanning it is dropped as well.
/// </summary>
/// <param name="trig">The trigger whose attached objects are to be destroyed.</param>
/// <param name="cell">The cell holding the bridge to drop, or CELL_NONE for none.</param>
/// <returns>bool; Was at least one attached object destroyed?</returns>
bool TActionClass::TAction_DESTROY_OBJECT(HouseClass * , ObjectClass * , TriggerClass * trig, Cell const & cell)
{
	bool any = false;
	bool process = true;
	while (process) {
		process = false;
		for (int index = 0; index < Technos.Count(); index++) {
			TechnoClass * techno = Technos[index];
			if (techno->Strength > 0 && techno->IsActive && techno->IsDown && !techno->IsInLimbo && techno->Tag != NULL && techno->Tag->Is_Trigger_Attached(trig)) {
				int damage = techno->Strength;
				techno->Take_Damage(damage, 0, Rule->C4Warhead, 0, true);
				any = true;
				process = true;
				index--;
			}
		}
	}

	if (cell != CELL_NONE) {
		int tries = 3;
		while (Map.Damage_Bridge(cell) == false && tries-- > 0) {};
		Point2D point;
		Coord coord = Coord(cell);
		coord.Z += BRIDGE_LEPTON_HEIGHT;
		TacticalMap->Coord_To_Pixel(coord, point);
		Rect dirty (point.X-128, point.Y-128, 256, 256);
		TacticalMap->Register_Dirty_Area (dirty);
	}
	return(any);
}


/// <summary>
/// Handles the change zoom trigger action.
/// The action is still accepted from a scenario, but it no longer has any effect on
/// the tactical view.
/// </summary>
bool TActionClass::TAction_CHANGE_ZOOM(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	return(true);
}


/// <summary>
/// Resizes the playable area of the map.
/// This routine is used to open up (or close down) part of the map part way through a
/// mission. The cells, movement zones, radar and every building's radar position are
/// brought back into step with the new bounds, and EVA tells the player that new
/// terrain is available.
/// </summary>
bool TActionClass::TAction_RESIZE_PLAYER_VIEW(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Map.LocalRect = TriggerRect;
	Map.Reset_Radar();

	Map.Reset_Iterator();
	CellClass *cptr = Map.Iterate();

	while (cptr) {
		cptr->Recalc_Attributes();
		cptr = Map.Iterate();
	}

	Map.Zone_Reset();
	Map.Reset_All_Subzones();

	Map.Complete_Radar_Refresh();

	for (int i = Buildings.Count() - 1; i >= 0; i--) {
		Buildings[i]->Update_Radar_Position(true);
	}

	Speak(VOX_NEW_TERRAIN);
	return(true);
}


/// <summary>
/// Plays an animation at the trigger's waypoint.
/// The animation is inert, so it is pure decoration -- whatever happens to be standing
/// there takes no harm from it.
/// </summary>
bool TActionClass::TAction_PLAY_ANIM(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Cell waypoint = Scen->Get_Waypoint_Cell(EffectLocation);
	Coord coord = Coord (waypoint);
	coord.Z = Map.Get_Height_GL(coord);
	if ( Map[waypoint].IsUnderBridge || Map[waypoint].WasUnderBridge ) {
		coord.Z += BRIDGE_LEPTON_HEIGHT;
	}
	AnimClass * anim = new AnimClass(AnimTypes[Data.Anim], coord);
	anim->IsInert = true;
	return(true);
}


/// <summary>
/// Detonates a weapon at the trigger's waypoint.
/// This routine is used to stage a scripted explosion. The weapon's warhead supplies
/// the damage, the combat animation and the lighting flash, and an EM pulse weapon
/// throws its pulse as well.
/// </summary>
bool TActionClass::TAction_DO_EXPLOSION(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Cell waypoint = Scen->Get_Waypoint_Cell(EffectLocation);
	Coord coord = Coord (waypoint);
	coord.Z = Map.Get_Height_GL(coord);
	if ( Map[waypoint].IsUnderBridge || Map[waypoint].WasUnderBridge ) {
		coord.Z += BRIDGE_LEPTON_HEIGHT;
	}
	WeaponType weapon = Data.Weapon;
	int damage = Weapons[weapon]->Attack;

	WeaponTypeClass * ww = Weapons[weapon];
	AnimTypeClass const * aptr = Combat_Anim(damage, ww->WarheadPtr, Map[coord].Land_Type(), coord);

	if (aptr) {
		new AnimClass(aptr, coord, 0, 1, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ZGRAD), Get_Explosion_Z(coord));
	}

	Combat_Lighting(coord, damage, Weapons[weapon]->WarheadPtr);
	Explosion_Damage(coord, damage, NULL, Weapons[weapon]->WarheadPtr);
	if (stricmp(ww->IniName, "empulseweapon") == 0) {
		new EMPulseClass(coord.As_Cell(), ww->WarheadPtr->SpreadFactor, damage, NULL);
	}
	return(true);
}


/// <summary>
/// Drops a meteor onto the trigger's waypoint.
/// The meteor lands at ground level, or on top of a bridge if one covers the cell,
/// and EVA warns the player that a meteor storm is under way.
/// </summary>
bool TActionClass::TAction_METEOR_IMPACT(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Cell waypoint = Scen->Get_Waypoint_Cell(EffectLocation);
	Coord coord = Coord (waypoint);
	coord.Z = Map.Get_Height_GL(coord);
	if ( Map[waypoint].IsUnderBridge || Map[waypoint].WasUnderBridge ) {
		coord.Z += BRIDGE_LEPTON_HEIGHT;
	}
	new VoxelAnimClass(VoxelAnimTypes[Data.VAnim], coord, NULL);

	Speak(VOX_METEOR_STORM);
	return(true);
}


/// <summary>
/// Brews up an ion storm over the battlefield.
/// The storm lasts for the number of seconds the action specifies, and the lightning
/// holds off for the warning period the rules call for.
/// </summary>
/// <returns>bool; Was the storm started? A storm already raging blocks a new one.</returns>
bool TActionClass::TAction_ION_STORM_START(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (IonStormClass::Is_Ion_Storm_Active()) {
		return(false);
	}
	IonStormClass::Ion_Storm_Begin(TICKS_PER_SECOND * Data.Value, TICKS_PER_SECOND * Rule->LightningDeferment);
	return(true);
}


/// <summary>
/// Brings an ion storm to an end.
/// </summary>
/// <returns>bool; Was there a storm in progress to stop?</returns>
bool TActionClass::TAction_ION_STORM_STOP(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (!IonStormClass::Is_Ion_Storm_Active()) {
		return(false);
	}

	IonStormClass::Ion_Storm_End();
	return(true);
}


/// <summary>
/// Takes control of the game away from the player.
/// This routine is used ahead of a scripted sequence so that the player cannot
/// interfere with it. Any pending build placement or special mouse mode is cleared
/// first, and EVA announces that control has been taken.
/// </summary>
bool TActionClass::TAction_LOCK_INPUT(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Speak(VOX_CONTROL_ESTABLISHED);

	Map.PendingObjectPtr = NULL;
	Map.PendingObject = NULL;
	Map.PendingHouse = HOUSE_NONE;
	Map.Set_Cursor_Shape(NULL);
	Map.Repair_Mode_Control(0);
	Map.Sell_Mode_Control(0);
	Map.Power_Mode_Control(0);
	Map.Waypoint_Mode_Control(0);

	Lock_Scenario_Input();

	return(true);
}


/// <summary>
/// Returns control of the game to the player.
/// This routine undoes LOCK_INPUT once a scripted sequence has run its course.
/// </summary>
bool TActionClass::TAction_UNLOCK_INPUT(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Unlock_Scenario_Input();
	return(true);
}


/// <summary>
/// Scrolls the tactical view to the trigger's waypoint.
/// This routine is used by scripted sequences to draw the player's attention somewhere.
/// The view travels there at the speed the action specifies rather than jumping.
/// </summary>
bool TActionClass::TAction_CENTER_VIEWPOINT(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Cell waypoint = Scen->Get_Waypoint_Cell(EffectLocation);
	Coord coord = Coord (waypoint);
	coord.Z = Map.Get_Height_GL(coord);

	if (Map[waypoint].IsUnderBridge || Map[waypoint].WasUnderBridge) {
		coord.Z += BRIDGE_LEPTON_HEIGHT;
	}

	TacticalMap->Setup_Trigger_Scroll(coord, Data.Speed);
	return(true);
}


/// <summary>
/// Zooms the tactical view in on the battlefield.
/// This routine is used by scripted sequences to push the camera in close. Input is
/// locked out and the mouse hidden, since the player is only meant to watch.
/// </summary>
bool TActionClass::TAction_ZOOM_IN(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	TacticalMap->ZoomFactor = Rule->ZoomInFactor;
	IgnoreInput = true;
	// Keyboard->Clear(); /// should do this too?? ZOOM_OUT does...

	Hide_Mouse();
	Map.Abort_Drag_Select();
	Map.Flag_To_Redraw();
	Map.Render();

	Sleep(1000);
	return(true);
}


/// <summary>
/// Returns the tactical view to its normal zoom level.
/// This routine reverses ZOOM_IN, bringing the mouse back and handing input control
/// back to whatever the scenario had set.
/// </summary>
bool TActionClass::TAction_ZOOM_OUT(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	TacticalMap->ZoomFactor = 1;
	Keyboard->Clear();
	IgnoreInput = Scen->IsInputLocked;

	Show_Mouse();
	Map.Flag_To_Redraw();
	Map.Render();

	return(true);
}


/// <summary>
/// Shrouds the entire map once more.
/// This routine is used to hide the map from the player again, as if it had never been
/// explored at all.
/// </summary>
bool TActionClass::TAction_RESHROUD(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Map.Shroud_The_Map();
	return(true);
}


/// <summary>
/// Changes the sweep behavior of the attached spotlights.
/// Every live building that carries a spotlight and shares this trigger's tag picks up
/// the new behavior, so a scenario can have the searchlights start hunting on cue.
/// </summary>
/// <returns>bool; Was at least one spotlight changed?</returns>
bool TActionClass::TAction_CHANGE_SPOTLIGHT_BEHAVIOR(HouseClass * , ObjectClass * object, TriggerClass * trig, Cell const & )
{
	bool spotlight_event = false;

	if (trig != NULL && trig->Class != NULL) {
		TEventClass* tevent = trig->Class->FirstEvent;

		while (tevent != NULL) {
			if (tevent->Event == TEVENT_ENEMY_IN_SPOTLIGHT || tevent->Event == TEVENT_ENEMY_IN_SPOTLIGHT_REPEATING) {

				spotlight_event = true;
				break;
			}

			tevent = tevent->Next;
		}
	}

	if (Data.LightBehavior == LIGHT_BEHAVIOR_FOLLOW && spotlight_event) {
		if (object == NULL || object->RTTI != RTTI_BUILDING) {
			return(false);
		}

		BuildingClass* building = (BuildingClass*)object;

		if (building->Strength > 0 &&
				building->IsActive &&
				building->IsDown &&
				!building->IsInLimbo &&
				building->Class->HasSpotlight &&
				building->BuildingLight != NULL &&
				building->BuildingLight->IsDetectionSource) {

			building->BuildingLight->Set_Behavior_Type(LIGHT_BEHAVIOR_FOLLOW);

			return(true);
		}

		return(false);
	}

	bool success = false;
	for (int index = 0; index < Buildings.Count(); index++) {
		BuildingClass * ptr = Buildings[index];
		if (ptr->Strength > 0 &&
				ptr->IsActive &&
				ptr->IsDown &&
				!ptr->IsInLimbo &&
				ptr->Class->HasSpotlight &&
				ptr->BuildingLight != NULL &&
				ptr->Tag != NULL &&
				ptr->Tag->Is_Trigger_Attached(trig)) {
			ptr->BuildingLight->Set_Behavior_Type(Data.LightBehavior);
			success = true;
		}
	}
	return(success);
}


/// <summary>
/// Destroys every trigger of the type this action names.
/// Unlike disabling, this is permanent -- the triggers are gone for the rest of the
/// scenario and no later action can bring them back.
/// </summary>
bool TActionClass::TAction_DESTROY_TRIGGER(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (Trigger != NULL) {
		for (int index = Triggers.Count() - 1; index >= 0; index--) {
			if (Triggers[index]->Class == Trigger) {
				Triggers[index]->Mark_To_Delete();
			}
		}
	}
	return(true);
}


/// <summary>
/// Destroys every tag of the type this action names.
/// Whatever those tags were attached to loses its link to the trigger, so the trigger can
/// no longer be sprung by that object or cell.
/// </summary>
bool TActionClass::TAction_DESTROY_TAG(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (Tag != NULL) {
		for (int index = Tags.Count() - 1; index >= 0; index--) {
			if (Tags[index]->Class == Tag) {
				Tags[index]->Mark_To_Delete();
			}
		}
	}
	return(true);
}


/// <summary>
/// Springs every trigger of the type this action names.
/// The triggers fire whether or not their own events have occurred, which lets a
/// scenario set off consequences on its own schedule.
/// </summary>
bool TActionClass::TAction_FORCE_TRIGGER(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (Trigger != NULL) {
		for (int index = 0; index < Triggers.Count(); index++) {
			if (Triggers[index]->Class == Trigger) {
				Triggers[index]->Spring(NULL, CELL_NONE);
			}
		}
	}
	return(true);
}


/// <summary>
/// Enables every trigger of the type this action names.
/// This is the counterpart of the disable action, and is how a scenario arms triggers
/// that were left dormant when the mission began.
/// </summary>
bool TActionClass::TAction_ENABLE_TRIGGER(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (Trigger != NULL && Trigger->Is_Enabled_At(Scen->Difficulty)) {
		for (int index = 0; index < Triggers.Count(); index++) {
			if (Triggers[index]->Class == Trigger) {
				Triggers[index]->Enable();
			}
		}
	}
	return(true);
}


/// <summary>
/// Disables every trigger of the type this action names.
/// A disabled trigger stops watching for its events, so it cannot spring until something
/// enables it again.
/// </summary>
bool TActionClass::TAction_DISABLE_TRIGGER(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	if (Trigger != NULL) {
		for (int index = 0; index < Triggers.Count(); index++) {
			if (Triggers[index]->Class == Trigger) {
				Triggers[index]->Disable();
			}
		}
	}
	return(true);
}


/// <summary>
/// Submits a radar event at the action's waypoint.
/// This routine is used to draw the player's attention to a place on the map with the
/// usual radar ping.
/// </summary>
bool TActionClass::TAction_RADAR_EVENT(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Submit_Radar_Event(Data.RadarEvent, Scen->Get_Waypoint_Cell(EffectLocation));
	return(true);
}


/// <summary>
/// Sets one of the scenario's local variables.
/// Local variables are the scenario's own flags, and triggers watch them to chain one
/// piece of scripting onto another.
/// </summary>
bool TActionClass::TAction_SET_LOCAL(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Scen->Set_Local_To(Data.Value, true);
	return(true);
}


/// <summary>
/// Clears one of the scenario's local variables.
/// Local variables are the scenario's own flags, and triggers watch them to chain one
/// piece of scripting onto another.
/// </summary>
bool TActionClass::TAction_CLEAR_LOCAL(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Scen->Set_Local_To(Data.Value, false);
	return(true);
}


/// <summary>
/// Rains a shower of meteors down around the action's waypoint.
/// The meteors are scattered about the waypoint in a mixture of sizes, and the player is
/// warned that the storm has begun.
/// </summary>
bool TActionClass::TAction_METEOR_SHOWER(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	static const int _scale = 70;
	static int _counts[SHOWER_COUNT] = {1, 5, 9, 15};

	Cell waypoint = Scen->Get_Waypoint_Cell(EffectLocation);
	Coord coord = Coord (waypoint);
	coord.Z = Map.Get_Height_GL(coord);
	if ( Map[waypoint].IsUnderBridge || Map[waypoint].WasUnderBridge ) {
		coord.Z += BRIDGE_LEPTON_HEIGHT;
	}
	int count = _counts[Data.MeteorShower] + abs(Scen->RandomNumber) % 3;
	AnimType met_large = Anim_From_Name("METLARGE");
	AnimType met_small = Anim_From_Name("METSMALL");

	int distance = count * _scale;
	for (int i = 0; i < count; i++) {
		Coord meteor_coord = coord + Coord(Scen->RandomNumber % distance, Scen->RandomNumber % distance);
		meteor_coord.Z = Map.Get_Height_GL(meteor_coord);

		new AnimClass(AnimTypes[abs(Scen->RandomNumber) % 2 ? met_large : met_small], meteor_coord);
	}

	Speak(VOX_METEOR_STORM);
	return(true);
}


/// <summary>
/// Sets how far the ambient light moves per adjustment.
/// Together with the change rate, this controls how leisurely the map fades from one
/// lighting level to the next.
/// </summary>
bool TActionClass::TAction_SET_AMBIENT_STEP(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Rule->AmbientLightChangeStep = (double)Data.Float;
	return(true);
}


/// <summary>
/// Sets how often the ambient light level is adjusted.
/// Together with the step size, this controls how leisurely the map fades from one
/// lighting level to the next.
/// </summary>
bool TActionClass::TAction_SET_AMBIENT_RATE(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Rule->AmbientLightChangeRate = (double)Data.Float;
	return(true);
}


/// <summary>
/// Sets the ambient light level of the scenario.
/// The map fades toward the new level right away, unless an ion storm is running -- the
/// storm owns the lighting while it lasts, and the new level takes hold when it clears.
/// </summary>
bool TActionClass::TAction_SET_AMBIENT_LIGHT(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Scen->AmbientLight = Data.Value;
	if ( !IonStormClass::Is_Ion_Storm_Active() ) {
		Scen->DesiredAmbientLight = Scen->AmbientLight;
	}
	return(true);
}


/// <summary>
/// Switches on AI trigger processing for a house.
/// This routine is how a scenario holds an AI opponent's team building back until the
/// script decides the mission is ready for it.
/// </summary>
bool TActionClass::TAction_BEGIN_AI_TRIGGERS(HouseClass * , ObjectClass * , TriggerClass * trig, Cell const & )
{
	if (Data.House != HOUSE_NONE) {
		House_From_HousesType(Data.House)->IsAITriggersOn = true;
	}
	return(true);
}


/// <summary>
/// Switches off AI trigger processing for a house.
/// The house named by the action stops evaluating its AI triggers, so it will raise no
/// further teams from them until the matching start action runs.
/// </summary>
bool TActionClass::TAction_STOP_AI_TRIGGERS(HouseClass * , ObjectClass * , TriggerClass * trig, Cell const & )
{
	if (Data.House != HOUSE_NONE) {
		House_From_HousesType(Data.House)->IsAITriggersOn = false;
	}
	return(true);
}


/// <summary>
/// Sets the house's share of effort spent on AI trigger teams.
/// This governs how much of the house's production the AI is willing to divert into the
/// teams its AI triggers ask for.
/// </summary>
bool TActionClass::TAction_SET_AI_TRIGGER_TEAM_RATIO(HouseClass * house, ObjectClass * , TriggerClass * , Cell const & )
{
	if (house != NULL) {
		house->RatioAITriggerTeam = Data.Value;
	}
	return(true);
}


/// <summary>
/// Sets the house's aircraft share of its team building effort.
/// The house's AI weighs this against its infantry and unit shares when it decides what
/// to produce for the teams it wants to assemble.
/// </summary>
bool TActionClass::TAction_SET_TEAM_AIRCRAFT_RATIO(HouseClass * house, ObjectClass * , TriggerClass * , Cell const & )
{
	if (house != NULL) {
		house->RatioTeamAircraft = Data.Value;
	}
	return(true);
}


/// <summary>
/// Sets the house's infantry share of its team building effort.
/// The house's AI weighs this against its aircraft and unit shares when it decides what
/// to produce for the teams it wants to assemble.
/// </summary>
bool TActionClass::TAction_SET_TEAM_INFANTRY_RATIO(HouseClass * house, ObjectClass * , TriggerClass * , Cell const & )
{
	if (house != NULL) {
		house->RatioTeamInfantry = Data.Value;
	}
	return(true);
}


/// <summary>
/// Sets the house's unit share of its team building effort.
/// The house's AI weighs this against its aircraft and infantry shares when it decides
/// what to produce for the teams it wants to assemble.
/// </summary>
bool TActionClass::TAction_SET_TEAM_UNIT_RATIO(HouseClass * house, ObjectClass * , TriggerClass * , Cell const & )
{
	if (house != NULL) {
		house->RatioTeamUnits = Data.Value;
	}
	return(true);
}


/// <summary>
/// Fires an ion cannon blast at the action's waypoint.
/// The blast is the same one the super weapon delivers, so the scenario gets the full
/// strike without a satellite having to be charged.
/// </summary>
bool TActionClass::TAction_ION_CANNON(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Cell waypoint = Scen->Get_Waypoint_Cell(EffectLocation);
	Coord coord = Coord (waypoint);
	coord.Z = Map.Get_Height_GL(coord);

	new IonBlastClass(coord);
	return(true);
}


/// <summary>
/// Launches a cluster missile at the action's waypoint.
/// The missile is flown in from the nearest map edge just as the multi missile super
/// weapon does, so the player sees the usual approach before the impact.
/// </summary>
/// <returns>bool; Was the missile launched?</returns>
bool TActionClass::TAction_MULTI_MISSILE(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Cell waypoint = Scen->Get_Waypoint_Cell(EffectLocation);
	Coord coord = Coord (waypoint);
	coord.Z = Map.Get_Height_GL(coord);
	if (Map[waypoint].IsUnderBridge) {
		coord.Z += BRIDGE_LEPTON_HEIGHT;
	}

	Cell closest = Map.Closest_Edge_Cell(coord.As_Cell());

	WeaponTypeClass const * weapon_type = Weapons[WeaponTypeClass::From_Name("MultiLauncher")];
	BulletTypeClass const * bullet_type = weapon_type->Bullet;
	BulletClass * bullet = Create_Bullet(bullet_type, &Map[coord], NULL, weapon_type->Attack, weapon_type->WarheadPtr, 50, weapon_type->ProjectileRange, false);

	if (bullet != NULL) {
		TVelocity3D<double> velocity(DIR_E, DIR_N, 100);
		bullet->Unlimbo(closest, velocity);
		return(true);
	}

	return(false);
}


/// <summary>
/// Launches a chemical missile at the action's waypoint.
/// The missile is flown in from the nearest map edge just as the chemical missile super
/// weapon does, so the player sees the usual approach before the impact.
/// </summary>
/// <returns>bool; Was the missile launched?</returns>
bool TActionClass::TAction_CHEM_MISSILE(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Cell waypoint = Scen->Get_Waypoint_Cell(EffectLocation);
	Coord coord = Coord (waypoint);
	coord.Z = Map.Get_Height_GL(coord);
	if (Map[waypoint].IsUnderBridge) {
		coord.Z += BRIDGE_LEPTON_HEIGHT;
	}

	Cell closest = Map.Closest_Edge_Cell(coord.As_Cell());

	WeaponTypeClass const * weapon_type = Weapons[WeaponTypeClass::From_Name("ChemLauncher")];
	BulletTypeClass const * bullet_type = weapon_type->Bullet;
	BulletClass * bullet = Create_Bullet(bullet_type, &Map[coord], NULL, weapon_type->Attack, weapon_type->WarheadPtr, 20, weapon_type->ProjectileRange, false);

	if (bullet != NULL) {
		TVelocity3D<double> velocity(DIR_E, DIR_N, 100);
		bullet->Unlimbo(closest, velocity);
		return(true);
	}

	return(false);
}


/// <summary>
/// Toggles whether the scenario's trains carry cargo.
/// </summary>
bool TActionClass::TAction_TOGGLE_TRAIN_CARGO(HouseClass * , ObjectClass * , TriggerClass * , Cell const & )
{
	Scen->IsTrainCargo = (Scen->IsTrainCargo == false);
	return(true);
}

#ifdef _DEBUG
/***********************************************************************************************
 * Action_From_Name -- retrieves ActionType for given name                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      name         name to get ActionType for                                                *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      ActionType for given name                                                              *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/1994 BR : Created.                                                                  *
 *=============================================================================================*/
TActionType Action_From_Name (char const * name)
{
	if (name == NULL) {
		return(TACTION_NONE);
	}

	for (TActionType i = TACTION_NONE; i < TACTION_COUNT; ++i) {
		if (!stricmp(name, _ActionText[i].Name)) {
			return(i);
		}
	}

	return(TACTION_NONE);
}

/***********************************************************************************************
 * Name_From_Action -- retrieves name for ActionType                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      action      ActionType to get name for                                                 *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      name of ActionType                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/1994 BR : Created.                                                                  *
 *=============================================================================================*/
char const * Name_From_Action(TActionType action)
{
	return(_ActionText[action].Name);
}
#endif

/***********************************************************************************************
 * Action_Needs -- Figures out what data an action object needs.                               *
 *                                                                                             *
 *    Use this routine to determine what extra data is needed for the specified action. This   *
 *    data will be prompted for in the scenario editor.                                        *
 *                                                                                             *
 * INPUT:   action   -- The action that is to be queried.                                      *
 *                                                                                             *
 * OUTPUT:  Returns with the data type (enumeration) needed for this action type.              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/22/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
NeedType Action_Needs(TActionType action)
{
	switch (action) {
		case TACTION_1_SPECIAL:
		case TACTION_FULL_SPECIAL:
			return(NEED_SPECIAL);

		case TACTION_FIRE_SALE:
		case TACTION_WIN:
		case TACTION_LOSE:
		case TACTION_ALL_HUNT:
		case TACTION_BEGIN_PRODUCTION:
		case TACTION_AUTOCREATE:
		case TACTION_MAKE_ALLY:
		case TACTION_MAKE_ENEMY:
		case TACTION_CHANGE_HOUSE:
		case TACTION_ALL_CHANGE_HOUSE:
		case TACTION_BEGIN_AI_TRIGGERS:
		case TACTION_STOP_AI_TRIGGERS:
			return(NEED_HOUSE);

		case TACTION_VEIN_GROWTH:
		case TACTION_TIB_GROWTH:
		case TACTION_ICE_GROWTH:
		case TACTION_BASE_BUILDING:
			return(NEED_BOOL);

		case TACTION_CREATE_TEAM:
		case TACTION_DESTROY_TEAM:
		case TACTION_REINFORCEMENTS:
		case TACTION_TALK_BUBBLE:
			return(NEED_TEAM);

		case TACTION_FLASH_TEAM:
			return(NEED_TEAM_AND_TIME);

		case TACTION_REINFORCEMENTS_SPECIAL:
			return(NEED_TEAM_AND_LOCATION);

		case TACTION_PARTICLE_ANIM:
			return(NEED_PARTICLE_AND_LOCATION);

		case TACTION_FORCE_TRIGGER:
		case TACTION_DESTROY_TRIGGER:
		case TACTION_ENABLE_TRIGGER:
		case TACTION_DISABLE_TRIGGER:
			return(NEED_TRIGGER);

		case TACTION_DZ:
		case TACTION_DAMAGE:
		case TACTION_LIGHT_SMALL:
		case TACTION_LIGHT_MEDIUM:
		case TACTION_LIGHT_LARGE:
		case TACTION_REDUCE_TIBERIUM:
		case TACTION_REVEAL_SOME:
		case TACTION_REVEAL_ZONE:
		case TACTION_REMOVE_PARTICLE_ANIM:
		case TACTION_ION_LIGHTNING_STRIKE:
		case TACTION_ION_CANNON:
		case TACTION_MULTI_MISSILE:
		case TACTION_CHEM_MISSILE:
			return(NEED_WAYPOINT);

		case TACTION_PLAY_MUSIC:
			return(NEED_THEME);

		case TACTION_PLAY_MOVIE:
		case TACTION_PLAY_INGAME_MOVIE:
			return(NEED_MOVIE);

		case TACTION_PLAY_SOUND:
		case TACTION_PLAY_SOUND_RANDOM:
			return(NEED_SOUND);

		case TACTION_PLAY_SPEECH:
			return(NEED_SPEECH);

		case TACTION_DESTROY_TAG:
			return(NEED_TAG);

		case TACTION_WAKEUP_GROUP:
		case TACTION_ADD_TIMER:
		case TACTION_SUB_TIMER:
		case TACTION_SET_TIMER:
		case TACTION_TEXT_TRIGGER:
		case TACTION_SET_AMBIENT_LIGHT:
		case TACTION_SET_AI_TRIGGER_TEAM_RATIO:
		case TACTION_SET_TEAM_AIRCRAFT_RATIO:
		case TACTION_SET_TEAM_INFANTRY_RATIO:
		case TACTION_SET_TEAM_UNIT_RATIO:
		case TACTION_SET_GROUP_ID:
			return(NEED_NUMBER);

		case TACTION_SET_GLOBAL:
		case TACTION_CLEAR_GLOBAL:
			return(NEED_GLOBAL);

		case TACTION_SET_LOCAL:
		case TACTION_CLEAR_LOCAL:
			return(NEED_LOCAL);

		case TACTION_PREFERRED_TARGET:
			return(NEED_QUARRY);

		case TACTION_CHANGE_ZOOM:
			return(NEED_NUMBER);

		case TACTION_RESIZE_PLAYER_VIEW:
			return(NEED_RECT);

		case TACTION_PLAY_ANIM:
			return(NEED_ANIM_AND_LOCATION);

		case TACTION_PLAY_SOUND_AT:
			return(NEED_SOUND_AND_LOCATION);

		case TACTION_DO_EXPLOSION:
			return(NEED_WEAPON_AND_LOCATION);

		case TACTION_METEOR_IMPACT:
			return(NEED_METEOR_AND_LOCATION);

		case TACTION_CENTER_VIEWPOINT:
			return(NEED_SPEED_AND_LOCATION);

		case TACTION_ION_STORM_START:
			return(NEED_NUMBER);

		case TACTION_CHANGE_SPOTLIGHT_BEHAVIOR:
			return(NEED_LIGHT_BEHAVIOR);

		case TACTION_RADAR_EVENT:
			return(NEED_EVENT_AND_LOCATION);

		case TACTION_METEOR_SHOWER:
			return(NEED_SHOWER_AND_LOCATION);

		case TACTION_SET_AMBIENT_STEP:
		case TACTION_SET_AMBIENT_RATE:
			return(NEED_FLOAT);

		default:
			break;
	}
	return(NEED_NONE);
}


/// <summary>
/// Fetches the class identifier of this object.
/// This routine is used by the persistence machinery to recognize what kind of object it
/// is about to load back.
/// </summary>
/// <param name="retval">Pointer to the identifier to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT STDMETHODCALLTYPE TActionClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_ActionClass;
	return(S_OK);
}


/// <summary>
/// Lists the members this action carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TActionClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HeapID);
	stream.Serialize(Next);
	stream.Serialize(Action);
	stream.Serialize(Team);
	stream.Serialize(TriggerRect);
	stream.Serialize(EffectLocation);
	stream.Serialize(Tag);
	stream.Serialize(Trigger);

	/*
	 * Which alternative of the data is live depends on the action, but every one of them
	 * is a plain scalar and none holds a pointer, so the union travels as its raw image.
	 */
	stream.Serialize_Bytes(&Data, sizeof(Data));
}


/// <summary>
/// Submits this action's state to the game CRC.
/// This routine is used by the network synchronization check, so only the data that must
/// agree between machines is folded in.
/// </summary>
/// <param name="crc">The CRC accumulator to submit this action's data to.</param>
void TActionClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);

	if (Next != NULL) crc(Next->Fetch_ID());
	crc(Action);
	if (Team != NULL) crc(Team->Fetch_ID());
	if (Tag != NULL) crc(Tag->Fetch_ID());
	if (Trigger != NULL) crc(Trigger->Fetch_ID());
	crc((int)Data.Value);
}


/// <summary>
/// Determines what an action must be attached to.
/// This routine is used by the map editor and by the trigger logic to tell which actions
/// only make sense when the trigger is wired to an object.
/// </summary>
/// <param name="event">The action to examine.</param>
/// <returns>Returns with the attachment requirement of the action, or ATTACH_NONE if it can
/// stand on its own.</returns>
AttachType Attaches_To(TActionType event)
{
	AttachType attach = ATTACH_NONE;

	switch (event) {
		case TACTION_DESTROY_OBJECT:
		case TACTION_SELL_ATTACHED:
		case TACTION_TURN_OFF_ATTACHED:
		case TACTION_TURN_ON_ATTACHED:
		case TACTION_CHANGE_HOUSE:
		case TACTION_GO_BERZERK:
		case TACTION_SET_GROUP_ID:
			attach = AttachType(attach | ATTACH_OBJECT);
			break;

		default:
			break;
	}
	return(attach);
}


/// <summary>
/// Fetches the coordinate of the waypoint this action refers to.
/// This routine is used by the actions that need somewhere on the map to happen at,
/// such as the missile and lighting effects.
/// </summary>
/// <returns>Returns with the coordinate of the action's waypoint.</returns>
Coord TActionClass::Waypoint_As_Coord(void)
{
	return(Scen->Get_Waypoint_Coord(Data.Value));
}
