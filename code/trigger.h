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

/* $Header: /CounterStrike/TRIGGER.H 1     3/03/97 10:26a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TRIGGER.H                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 11/12/94                                                     *
 *                                                                                             *
 *                  Last Update : November 12, 1994 [JLB]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "abstract.h"
#include "ftimer.h"
#include "timer.h"

#include "dialog.hh"
#include "tevent.hh"

class ObjectClass;
class TriggerTypeClass;
template<class T> class DynamicVectorClass;
class TechnoClass;

class TriggerClass : public AbstractClass
{
		typedef AbstractClass BASECLASS;

	public:

		TriggerTypeClass * Class;

		/*
		 * This points to the next trigger hanging off the same tag, or is NULL if this is the
		 * last of them. A tag makes one trigger per trigger type it carries and examines them
		 * all as a chain.
		 */
		TriggerClass * LinkedTo;

		/*
		**	Constructor/Destructor
		*/
		TriggerClass(TriggerTypeClass * trigtype=NULL);
		virtual ~TriggerClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		/*
		**	Processing routines
		*/
		bool Spring(ObjectClass * object=0, Cell cell=CELL_NONE);
		bool Should_Spring(TEventType event=TEVENT_ANY, ObjectClass * object=0, bool forced = false, bool persistent = false, TechnoClass * source=NULL);
		virtual void Detach(AbstractClass const * target, bool all=true) override;


		/*
		**	Utility routines
		*/
		virtual void Compute_CRC(CRCEngine & crc) const override;

		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_TRIGGER);}

		void Enable(void);
		void Disable(void);
		void Set_Enabled(bool val) {if (val) Enable(); else Disable();}
		bool Is_Enabled(void) const {return(IsActive);}
		__declspec(property(get=Is_Enabled, put=Set_Enabled)) bool IsEnabled;

		void Flag_Event_Tripped(int event);
		void Flag_Event_Untripped(int event);
		bool Is_Event_Tripped(int event);

		bool Is_Linked_To_Global(int global) const;
		void Reset_Global_Linked_Timed_Events(int global);
		void Reset_Local_Linked_Timed_Events(int local);
		void Reset_All_Timed_Events(void);

		void Mark_To_Delete(void);
		bool Is_Marked_To_Delete(void) const;
		
		bool Is_Enters_Zone(void) const;
		bool Is_Cross_Horizontal(void) const;
		bool Is_Cross_Vertical(void) const;

		bool Is_Allow_Win(void) const;

	private:
		/*
		 * If this trigger has been marked for deletion, then this flag will be true. It stops
		 * springing at once but is not destroyed until the deferred deletion list is processed, so
		 * that it cannot vanish out from under the logic still walking it.
		 */
		bool IsToDelete;

		/*
		**	Timer based events require a special timer control handler.
		*/
		CDTimerClass<FrameTimerClass> Timer;

		/*
		**	If this event has been triggered by something that is temporal, then
		**	this flag will be set to true so that subsequent trigger examination
		**	will return a successful event trigger flag. Typical use of this is
		**	for when objects of a specific type are built.
		*/
		int IsTripped;

		/*
		**	If this trigger object is active, then this flag will be true. Trigger
		**	objects that are not active are either not yet created or have been
		**	deleted after fulfilling their action.
		*/
		bool IsActive;
};

extern DynamicVectorClass<TriggerClass *> Triggers;

TriggerClass * Find_Or_Make(TriggerTypeClass * trigtype);
