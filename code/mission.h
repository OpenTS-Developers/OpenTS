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

/* $Header: /CounterStrike/MISSION.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : MISSION.H                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 23, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 23, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "ccini.h"
#include "ftimer.h"
#include "object.h"
#include "timer.h"

class MissionControlClass;

/****************************************************************************
**	This handles order assignment and tracking. The order is used to guide
**	overall AI processing.
*/
class MissionClass : public ObjectClass
{
		typedef ObjectClass BASECLASS;

	public:

		/*
		**	This the tactical strategy to use. It is used by the unit script. This
		**	is a general guide for unit AI processing.
		*/
		MissionType CurrentMission;
		MissionType SuspendedMission;

		/*
		**	The order queue is used for orders that should take effect when the vehicle
		**	has reached the center point of a cell. The queued order number is +1 when stored here
		**	so that 0 will indicated there is no queued order.
		*/
		MissionType MissionQueue;

		int Status;

		bool IsMissionUnloadStandby;

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		MissionClass(void);
		virtual ~MissionClass(void) override {};

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
#ifdef _DEBUG
		void  Debug_Dump(MonoClass *mono) const;
#endif

		void Shorten_Mission_Timer(void) {Timer = 0;}
		virtual MissionType Get_Mission(void) const override;
		virtual void  Assign_Mission(MissionType mission) override;
		virtual bool Commence(void);
		virtual bool Ready_To_Commence(void) {return(true);};
		virtual void AI(void) override;
		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Compute_CRC(CRCEngine &) const override;

		/*
		**	Support functions.
		*/
		virtual int Do_MISSION_SLEEP(void);
		virtual int Do_MISSION_HARMLESS(void);
		virtual int Do_MISSION_AMBUSH(void);
		virtual int Do_MISSION_ATTACK(void);
		virtual int Do_MISSION_CAPTURE(void);
		virtual int Do_MISSION_GUARD(void);
		virtual int Do_MISSION_GUARD_AREA(void);
		virtual int Do_MISSION_HARVEST(void);
		virtual int Do_MISSION_HUNT(void);
		virtual int Do_MISSION_MOVE(void);
		virtual int Do_MISSION_RETREAT(void);
		virtual int Do_MISSION_RETURN(void);
		virtual int Do_MISSION_STOP(void);
		virtual int Do_MISSION_UNLOAD(void);
		virtual int Do_MISSION_ENTER(void);
		virtual int Do_MISSION_CONSTRUCTION(void);
		virtual int Do_MISSION_DECONSTRUCTION(void);
		virtual int Do_MISSION_REPAIR(void);
		virtual int Do_MISSION_MISSILE(void);
		virtual int Do_MISSION_OPEN(void);
		virtual int Do_MISSION_RESCUE(void);
		virtual int Do_MISSION_PATROL(void);

		virtual void Set_Mission(MissionType mission);
		static bool Is_Recruitable_Mission(MissionType mission);

		static char const *  Mission_Name(MissionType order);
		static MissionType  Mission_From_Name(char const *name);
		virtual void  Override_Mission(MissionType mission, AbstractClass *, AbstractClass *);
		virtual bool Restore_Mission(void);
		MissionControlClass const & Current_Mission_Control(void) const;
		virtual bool Has_Suspended_Mission(void) const;

	private:

		/*
		**	This the thread processing timer. When this value counts down to zero, then
		**	more script processing may occur.
		*/
		CDTimerClass<FrameTimerClass> Timer;
};


/****************************************************************************
**	This is the mission control (pun) that controls how each mission behaves
**	when it comes to interacting with the game world. Example; some
**	missions allow the object to scatter from threats, while others require
**	the object to remain in place. This kind of characteristics are specfied
**	by this class.
*/
class MissionControlClass
{
	public:
		MissionControlClass(void);

		bool Read_INI(CCINIClass const & ini);
		int Normal_Delay(void) const {return(int(TICKS_PER_MINUTE * Rate));}
		int AA_Delay(void) const {return(int(TICKS_PER_MINUTE * AARate));}

		/*
		**	This is the mission identifier that this mission represents.
		*/
		MissionType Mission;

		char const * Name(void) const;

		/*
		**	If the object should not be considered a threat when it
		**	comes to target scanning, then this will be true.
		*/
		bool IsNoThreat;

		/*
		**	If objects in this mission should avoid targeting the enemy and
		**	also avoid responding to the enemy, then this will be true.
		*/
		bool IsZombie;

		/*
		**	An ojbect that can be recruited into a team must be on a mission
		**	of this type.
		*/
		bool IsRecruitable;

		/*
		**	If the object can behave normally except that it cannot
		**	move to another location, then this flag will be true.
		*/
		bool IsParalyzed;

		/*
		**	If an object on this mission is damaged, it is allowed to
		**	retaliate?
		*/
		bool IsRetaliate;

		/*
		**	Is the object allowed to scatter from immediate threats?
		*/
		bool IsScatter;

		/*
		**	This specifies the time to delay between calls to the mission handler for those cases
		**	where the delay could be indefinate. The exception would be when timing is critical.
		**	Typical use of this would be to regulate the delay between mundane mission processing
		**	in order to achieve less game overhead.
		*/
		double Rate;

		/*
		**	Anti-Aircraft buildings (and units) in guard or guard area mode will use this override
		**	delay interval instead of the normal "Rate" value.
		*/
		double AARate;

		// Carries the mission controls to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Mission);
			stream.Serialize(IsNoThreat);
			stream.Serialize(IsZombie);
			stream.Serialize(IsRecruitable);
			stream.Serialize(IsParalyzed);
			stream.Serialize(IsRetaliate);
			stream.Serialize(IsScatter);
			stream.Serialize(Rate);
			stream.Serialize(AARate);
		}
};
