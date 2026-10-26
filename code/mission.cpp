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

/* $Header: /CounterStrike/MISSION.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : MISSION.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 23, 1994                                               *
 *                                                                                             *
 *                  Last Update : September 14, 1996 [JLB]                                     *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   MissionClass::AI -- Processes order script.                                               *
 *   MissionClass::Assign_Mission -- Give an order to a unit.                                  *
 *   MissionClass::Commence -- Start script with new order.                                    *
 *   MissionClass::Debug_Dump -- Dumps status values to mono screen.                           *
 *   MissionClass::Get_Mission -- Fetches the mission that this object is acting under.        *
 *   MissionClass::MissionClass -- Default constructor for the mission object type.            *
 *   MissionClass::Mission_???  -- Stub mission functions that do nothing.                     *
 *   MissionClass::Mission_From_Name -- Fetch order pointer from its name.                     *
 *   MissionClass::Mission_Name -- Converts a mission number into an ASCII string.             *
 *   MissionClass::Override_Mission -- temporarily overrides the units mission                 *
 *   MissionClass::Restore_Mission -- Restores overridden mission                              *
 *   MissionClass::Set_Mission -- Sets the mission to the specified value.                     *
 *   MissionClass::Is_Recruitable_Mission -- Determines if this mission is recruitable for a te*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "mission.h"

#include "_bench.h"
#include "_mission.h"
#include "bench.h"
#include "ccini.h"
#include "globals.h"
#include "incdec.h"
#include "mono.h"
#include "savestream.h"
#include "syncrechook.h"

#include "bench.hh"

#if defined(_MSC_VER)
#include <intrin.h>
#endif


/***********************************************************************************************
 * MissionClass::MissionClass -- Default constructor for the mission object type.              *
 *                                                                                             *
 *    This is the default constructor for the mission class object. It sets the mission        *
 *    handler into a default -- do nothing -- state.                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/23/1995 JLB : Created.                                                                 *
 *   03/01/1996 JLB : Uses initializer lists.                                                  *
 *=============================================================================================*/
MissionClass::MissionClass(void) :
	BASECLASS(),
	CurrentMission(MISSION_NONE),
	SuspendedMission(MISSION_NONE),
	MissionQueue(MISSION_NONE),
	Status(0),
	IsMissionUnloadStandby(false),
	Timer(0)
{
}


/***********************************************************************************************
 * MissionClass::Mission_???  -- Stub mission functions that do nothing.                       *
 *                                                                                             *
 *    These are the stub routines that handle the mission logic. They do nothing at this       *
 *    level. Derived classes will override these routine as necessary.                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before calling this mission        *
 *          handler again.                                                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int MissionClass::Do_MISSION_SLEEP(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_HARMLESS(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_AMBUSH(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_ATTACK(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_CAPTURE(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_GUARD(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_GUARD_AREA(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_HARVEST(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_HUNT(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_MOVE(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_RETREAT(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_RETURN(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_STOP(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_UNLOAD(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_ENTER(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_CONSTRUCTION(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_DECONSTRUCTION(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_REPAIR(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_MISSILE(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_OPEN(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_RESCUE(void) {return(TICKS_PER_SECOND*30);};
int MissionClass::Do_MISSION_PATROL(void) {return(TICKS_PER_SECOND*30);};


/***********************************************************************************************
 * MissionClass::Set_Mission -- Sets the mission to the specified value.                       *
 *                                                                                             *
 *    Use this routine to set the current mission for this object. This routine will blast     *
 *    over the current mission, bypassing the queue method. Call it when the mission needs     *
 *    to be changed immediately.                                                               *
 *                                                                                             *
 * INPUT:   mission  -- The mission to set to.                                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void MissionClass::Set_Mission(MissionType mission)
{
	CurrentMission = mission;
	MissionQueue = MISSION_NONE;
	IsMissionUnloadStandby = false;
}


/***********************************************************************************************
 * MissionClass::Get_Mission -- Fetches the mission that this object is acting under.          *
 *                                                                                             *
 *    Use this routine to fetch the mission that this object is CURRENTLY acting under. The    *
 *    mission queue may be filled with a imminent mission change, but this routine does not    *
 *    consider that. It only returns the CURRENT mission.                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the mission that this unit is currently following.                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
MissionType MissionClass::Get_Mission(void) const
{
	return(CurrentMission == MISSION_NONE ? MissionQueue : CurrentMission);
}


#ifdef _DEBUG
/***********************************************************************************************
 * MissionClass::Debug_Dump -- Dumps status values to mono screen.                             *
 *                                                                                             *
 *    This is a debugging function that dumps this class' status to the monochrome screen      *
 *    for review.                                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void MissionClass::Debug_Dump(MonoClass * mono) const
{
	mono->Set_Cursor(1, 9);mono->Printf("%-14s", MissionClass::Mission_Name(CurrentMission));
	mono->Set_Cursor(16, 9);mono->Printf("%-12s", MissionClass::Mission_Name(MissionQueue));
	mono->Set_Cursor(1, 7);mono->Printf("%3d", (int)Timer);
	mono->Set_Cursor(6, 7);mono->Printf("%2d", Status);

	BASECLASS::Debug_Dump(mono);
}
#endif


/***********************************************************************************************
 * MissionClass::AI -- Processes order script.                                                 *
 *                                                                                             *
 *    This routine will process the order script for as much time as                           *
 *    possible or until a script delay is detected. This routine should                        *
 *    be called for every unit once per game loop (if possible).                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/23/1994 JLB : Created.                                                                 *
 *   06/25/1995 JLB : Added new missions.                                                      *
 *=============================================================================================*/
void MissionClass::AI(void)
{
	BASECLASS::AI();

	if (!IsActive) {
		return;
	}

	/* 
	 * AircraftClass debug prints expose that mission functions were named Do_MISSION_X.
	 * This suggests that switches on enums like this used a macro taking the enum as input.
	 * 
	 * Simplifies invoking the right function and checking we aren't missing any case.
	 */
	#define INVOKE(e) case MISSION_ ## e: Timer = Do_MISSION_ ## e (); break;

	/*
	**	This is the script AI equivalent processing.
	*/
	BStart(BENCH_MISSION);
	if (Timer == 0 && Strength > 0) {
		switch (CurrentMission) {
			default:
				Timer = Do_MISSION_SLEEP();
				break;

			INVOKE(SLEEP);
			INVOKE(HARMLESS);
			INVOKE(ENTER);
			INVOKE(CONSTRUCTION);
			INVOKE(DECONSTRUCTION);
			INVOKE(ATTACK);
			INVOKE(RETREAT);
			INVOKE(HARVEST);
			INVOKE(GUARD_AREA);
			INVOKE(RETURN);
			INVOKE(STOP);
			INVOKE(AMBUSH);
			INVOKE(UNLOAD);
			INVOKE(REPAIR);
			INVOKE(OPEN);
			INVOKE(MISSILE);
			INVOKE(GUARD);
			INVOKE(CAPTURE);
			INVOKE(MOVE);
			INVOKE(HUNT);
			INVOKE(RESCUE);
			INVOKE(PATROL);

			case MISSION_STICKY:
				Timer = Do_MISSION_GUARD();
				break;

			case MISSION_SABOTAGE:
				Timer = Do_MISSION_CAPTURE();
				break;

			case MISSION_QMOVE:
				Timer = Do_MISSION_MOVE();
				break;
		}
	}
	BEnd(BENCH_MISSION);
}


/***********************************************************************************************
 * MissionClass::Commence -- Start script with new order.                                      *
 *                                                                                             *
 *    This routine will start script processing according to any queued                        *
 *    order it may have. If there is no queued order, then this routine                        *
 *    does nothing. Call this routine whenever the unit is in a good                           *
 *    position to change its order (such as when it is stopped).                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Did the mission actually change?                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/23/1994 JLB : Created.                                                                 *
 *   07/14/1994 JLB : Simplified.                                                              *
 *   06/17/1995 JLB : Returns success flag.                                                    *
 *=============================================================================================*/
bool MissionClass::Commence(void)
{
	if (MissionQueue != MISSION_NONE) {
		CurrentMission = MissionQueue;
		MissionQueue = MISSION_NONE;

		/*
		**	Force immediate state machine processing at the first state machine state value.
		*/
		Timer = 0;
		Status = 0;
		IsMissionUnloadStandby = false;
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * MissionClass::Assign_Mission -- Give an order to a unit.                                    *
 *                                                                                             *
 *    This routine will assign the specified mission to the mission queue for this object.     *
 *    The actual mission logic will then be performed at the first available and legal         *
 *    opportunity.                                                                             *
 *                                                                                             *
 * INPUT:   order -- Mission to give the unit.                                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/04/1991 JLB : Created.                                                                 *
 *   04/15/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
void MissionClass::Assign_Mission(MissionType order)
{
	Sync_Record_Mission(*this, CurrentMission, order, SYNC_MISSION_ASSIGN, (unsigned)(uintptr_t)_ReturnAddress());

	if (CurrentMission == MISSION_DECONSTRUCTION) return;
	/*
	**	Ensure that a MISSION_QMOVE is translated into a MISSION_MOVE.
	*/
	if (order == MISSION_QMOVE) order = MISSION_MOVE;

	if (order != MISSION_NONE && (CurrentMission != order || MissionQueue != order && MissionQueue != MISSION_NONE)) {
		MissionQueue = order;
		IsMissionUnloadStandby = false;
	}
}


/***********************************************************************************************
 * MissionClass::Override_Mission -- temporarily overrides the units mission                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:      MissionType mission - the mission we want to override                           *
 *               TARGET      tarcom  - the new target we want to override                      *
 *               TARGET      navcom  - the new navigation point to override                    *
 *                                                                                             *
 * OUTPUT:      none                                                                           *
 *                                                                                             *
 * WARNINGS:   If a mission is already overridden, the current mission is                      *
 *               just re-assigned.                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/28/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
void MissionClass::Override_Mission(MissionType mission, AbstractClass *, AbstractClass *)
{
	if (CurrentMission == MISSION_DECONSTRUCTION) return;

	if (MissionQueue != MISSION_NONE) {
		SuspendedMission = MissionQueue;
	} else {
		SuspendedMission = CurrentMission;
	}

	CurrentMission = mission;
	IsMissionUnloadStandby = false;
}


/***********************************************************************************************
 * MissionClass::Restore_Mission -- Restores overridden mission                                *
 *                                                                                             *
 * INPUT:      none                                                                            *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/28/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
bool MissionClass::Restore_Mission(void)
{
	if (SuspendedMission != MISSION_NONE) {
		CurrentMission = SuspendedMission;
	 	SuspendedMission= MISSION_NONE;
		IsMissionUnloadStandby = false;
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * MissionClass::Is_Recruitable_Mission -- Determines if this mission is recruitable for a tea *
 *                                                                                             *
 *    Some missions preclude recruitment into a team. This routine will examine the mission    *
 *    specified and if not allowed for a team, it will return false.                           *
 *                                                                                             *
 * INPUT:   mission  -- The mission type to examine.                                           *
 *                                                                                             *
 * OUTPUT:  bool; Is an object following this mission allowed to be recruited into a team?     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/14/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool MissionClass::Is_Recruitable_Mission(MissionType mission)
{
	if (mission == MISSION_NONE) {
		return(true);
	}
	return(MissionControl[mission].IsRecruitable);
}


/// <summary>
/// Constructor for the mission control record.
/// This routine establishes the default behavior for a mission. The rules file is free to
/// override any of these settings when the mission's own section is processed.
/// </summary>
MissionControlClass::MissionControlClass(void) :
	Mission(MISSION_NONE),
	IsNoThreat(false),
	IsZombie(false),
	IsRecruitable(true),
	IsParalyzed(false),
	IsRetaliate(true),
	IsScatter(true),
	Rate(.016),
	AARate(.016)
{
}


/// <summary>
/// Fetches the name of the mission this record controls.
/// The name doubles as the rules file section that the settings are read from.
/// </summary>
/// <returns>Returns with a pointer to the mission name. A placeholder name is returned if
/// no mission has been assigned to this record.</returns>
char const * MissionControlClass::Name(void) const
{
	if (Mission == MISSION_NONE) {
		return("<none>");
	}
	return(Missions[Mission]);
}


/// <summary>
/// Fetches the mission behavior settings from the rules file.
/// Every mission has a section of its own in the rules, named after the mission. When that
/// section is missing, the defaults established by the constructor are left in place.
/// </summary>
/// <returns>bool; Was a rules section for this mission found and processed?</returns>
bool MissionControlClass::Read_INI(CCINIClass const & ini)
{
	if (ini.Is_Present(Name())) {
		IsNoThreat = ini.Get_Bool(Name(), "NoThreat", IsNoThreat);
		IsZombie = ini.Get_Bool(Name(), "Zombie", IsZombie);
		IsRecruitable = ini.Get_Bool(Name(), "Recruitable", IsRecruitable);
		IsParalyzed = ini.Get_Bool(Name(), "Paralyzed", IsParalyzed);
		IsRetaliate = ini.Get_Bool(Name(), "Retaliate", IsRetaliate);
		IsScatter = ini.Get_Bool(Name(), "Scatter", IsScatter);
		Rate = ini.Get_Float(Name(), "Rate", Rate);
		AARate = ini.Get_Float(Name(), "AARate", 0);
		if (AARate == 0) {
			AARate = Rate;
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * MissionClass::Mission_From_Name -- Fetch order pointer from its name.                       *
 *                                                                                             *
 *    This routine is used to convert an ASCII order name into the actual                      *
 *    order number it represents. Typically, this is used when processing                      *
 *    a scenario INI file.                                                                     *
 *                                                                                             *
 * INPUT:   name  -- The ASCII order name to process.                                          *
 *                                                                                             *
 * OUTPUT:  Returns with the actual order number that the ASCII name                           *
 *          represents.                                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/1992 JLB : Created.                                                                 *
 *   04/22/1994 JLB : Converted to static member function.                                     *
 *=============================================================================================*/
MissionType MissionClass::Mission_From_Name(char const * name)
{
	MissionType	order;

	if (name) {
		for (order = MISSION_FIRST; order < MISSION_COUNT; order++) {
			if (stricmp(Missions[order], name) == 0) {
				return(order);
			}
		}
	}
	return(MISSION_NONE);
}


/***********************************************************************************************
 * MissionClass::Mission_Name -- Converts a mission number into an ASCII string.               *
 *                                                                                             *
 *    Use this routine to convert a mission number into the ASCII string that represents       *
 *    it. Typical use of this is when generating an INI file.                                  *
 *                                                                                             *
 * INPUT:   mission  -- The mission number to convert.                                         *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the ASCII string that represents the mission type.       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
char const * MissionClass::Mission_Name(MissionType mission)
{
	return(mission != MISSION_NONE ? Missions[mission] : "<none>");
}


/// <summary>
/// Lists the members this mission handler carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void MissionClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(CurrentMission);
	stream.Serialize(SuspendedMission);
	stream.Serialize(MissionQueue);
	stream.Serialize(Status);
	stream.Serialize(IsMissionUnloadStandby);
	stream.Serialize(Timer);
}


/// <summary>
/// Submits the mission state of this object to the checksum engine.
/// This routine is one link in the game state checksum chain that the network code uses to
/// detect a multiplayer desynchronization.
/// </summary>
void MissionClass::Compute_CRC(CRCEngine &crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(CurrentMission);
	crc(SuspendedMission);
	crc(MissionQueue);
	crc(Status);
	crc((int)Timer);
}


/// <summary>
/// Fetches the mission control record for the current mission.
/// The mission control record holds the rule settings -- threat, recruitability, processing
/// rate and the like -- that govern how this object conducts itself while on this mission.
/// </summary>
/// <returns>Returns with a reference to the mission control record governing the current
/// mission.</returns>
MissionControlClass const & MissionClass::Current_Mission_Control(void) const
{
	return(MissionControl[CurrentMission]);
}


/// <summary>
/// Determines if this object has a mission held in reserve.
/// A mission becomes suspended when a temporary override takes control of the object. Use
/// this routine to find out whether there is anything to fall back to when the override
/// runs its course.
/// </summary>
/// <returns>bool; Is there a suspended mission waiting to be restored?</returns>
bool MissionClass::Has_Suspended_Mission(void) const
{
	return(SuspendedMission != MISSION_NONE);
}
