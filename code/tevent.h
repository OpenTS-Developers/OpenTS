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

/* $Header: /CounterStrike/TEVENT.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TEVENT.H                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 11/28/95                                                     *
 *                                                                                             *
 *                  Last Update : November 28, 1995 [JLB]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "abstract.h"
#include "ftimer.h"
#include "timer.h"
#include "types.h"

#include "aircraft.hh"
#include "attach.hh"
#include "dialog.hh"
#include "infantry.hh"
#include "need.hh"
#include "struct.hh"
#include "tevent.hh"
#include "unit.hh"

template<class T> class DynamicVectorClass;
class TeamTypeClass;
class TechnoClass;

TEventType Event_From_Name(char const * name);
NeedType Event_Needs(TEventType event);
#ifdef _DEBUG
char const * Name_From_Event(TEventType event);
#endif


/*
**	This elaborates the information necessary to trigger
**	an event.
*/
class TEventClass : public AbstractClass
{
		typedef AbstractClass BASECLASS;

	public:

		/*
		 * This is the index of this event within the Events vector, assigned as the event
		 * is created and returned by Fetch_Heap_ID.
		 */
		int HeapID;

		/*
		 * This points to the next event of the trigger this one belongs to. A trigger chains
		 * its events off FirstEvent and examines all of them when deciding whether to spring.
		 */
		TEventClass * Next;

		/*
		**	This is the event that will controls how this event gets triggered.
		*/
		TEventType Event;

		/*
		**	If this event needs to reference a team type, then this is the pointer
		**	to the team type object. This must be separated from the following
		**	union because Watcom compiler won't allow a class that has a
		**	constructor to be declared in a union.
		*/
		TeamTypeClass const * Team;

		union {
			StructType				Structure;	// Used for structure type checking.
			UnitType				Unit;		// Used for unit type checking.
			InfantryType			Infantry;	// Used for infantry type checking.
			AircraftType			Aircraft;	// Used for aircraft type checking.
			HousesType				House;		// Used for house specific events.
			int	Value;						// Used for other events that need data.
		} Data;

		TEventClass(void);
		virtual ~TEventClass(void) override;

		virtual ClassID Class_ID(void) const override;

		virtual void Serialize(SaveStreamClass & stream) override;

		void Read_INI(void);
		void Build_INI_Entry(char * buffer) const;
		virtual void Compute_CRC(CRCEngine & crc) const override;

		virtual void Detach(AbstractClass const * target, bool all=true) override;
		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_EVENT);}
		virtual int Fetch_Heap_ID(void) const override {return(HeapID);};

		bool Is_Time_Based(void) const;
		bool Is_To_Flag_As_Tripped(void) const;

		bool operator () (TEventType event, HouseClass const * house, ObjectClass const * object, CDTimerClass<FrameTimerClass> & timer, bool & tripped, TechnoClass * source);

		bool operator == (TEventClass const & rvalue) const {return(memcmp(this, &rvalue, sizeof(*this)) == 0);}
		bool operator != (TEventClass const & rvalue) const {return(!(*this == rvalue));}
};

extern DynamicVectorClass<TEventClass *> Events;

AttachType Attaches_To(TEventType event);

