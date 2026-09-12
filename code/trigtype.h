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

/* $Header: /CounterStrike/TRIGTYPE.H 1     3/03/97 10:26a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TRIGTYPE.H                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 06/05/96                                                     *
 *                                                                                             *
 *                  Last Update : June 5, 1996 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "abstype.h"

#include "attach.hh"
#include "dialog.hh"
#include "diff.hh"

class HouseClass;
class TActionClass;
class TEventClass;
class TriggerClass;
template<class T> class DynamicVectorClass;


class TriggerTypeClass : public AbstractTypeClass
{
		typedef AbstractTypeClass BASECLASS;

	public:
		TriggerTypeClass(char const * name = NULL);
		virtual ~TriggerTypeClass(void) override;

		static TriggerTypeClass * Find_Or_Make(char const * ininame = NULL);

		virtual ClassID Class_ID(void) const override;

		/*
		**	File I/O routines
		*/
		static void Read_All(CCINIClass const & ini);
		static void Write_All(CCINIClass & ini);
		virtual bool Read_INI(CCINIClass const & ini) override;
		virtual bool Write_INI(CCINIClass & ini) const override;

		/*
		**	Processing routines
		*/
		TriggerClass * Create_One_Of(void) const;
		void Destroy_All_Of(void) const;

		/*
		**	Utility routines
		*/
		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_TRIGGERTYPE);}
		virtual void Compute_CRC(CRCEngine & crc) const override;
		virtual int Fetch_Heap_ID(void) const override {return(HeapID);}

		virtual void Detach(AbstractClass const * target, bool all=true) override;

		bool Delete_Action(TActionClass const * taction);
		bool Delete_Event(TEventClass const * tevent);

		AttachType Attaches_To(void) const;
		static TriggerTypeClass * From_Name(char const * name);

		bool Is_To_Inherit(void) const {return(IsToInherit);}
		void Set_To_Inherit(bool b) {IsToInherit = b;}

		bool Is_Linked_To_Global(int global) const;
		bool Is_Linked_To_Local(int local) const;

		bool Is_Enters_Zone(void) const;
		bool Is_Cross_Horizontal(void) const;
		bool Is_Cross_Vertical(void) const;

		bool Is_Allow_Win(void) const;

		bool Is_Enabled_At(DiffType difficulty) const;

		bool Is_Enabled(void) const {return(_IsEnabled);}
		void Set_Enabled(bool val) {_IsEnabled = val;}
		__declspec(property(get=Is_Enabled, put=Set_Enabled)) bool IsEnabled;

	public:

		/*
		 * This is the index of this trigger type within the TriggerTypes vector, assigned as
		 * the type is created and returned by Fetch_Heap_ID.
		 */
		int HeapID;

	private:
		/*
		 * If triggers of this type start out able to spring, then this flag will be true. A
		 * disabled trigger stays attached to its tag but ignores its events until some
		 * action enables it. It is reached through the IsEnabled property.
		 */
		bool _IsEnabled;

	public:
		/*
		 * These specify the difficulty levels this trigger is active at. A trigger switched
		 * off for the scenario's difficulty is disabled as it is created, so it never
		 * springs.
		 */
		bool IsEnabledOnEasy;
		bool IsEnabledOnMedium;
		bool IsEnabledOnHard;

	private:

		/*
		 * If a tag carrying this trigger should be handed over when its object is replaced
		 * by another, then this flag will be true. Capturing a building or vehicle will pass
		 * the capturing infantry's tag along to the prize, and a destroyed vehicle will pass
		 * its tag to the crew that survives.
		 */
		bool IsToInherit;

	public:

		/*
		**	For house-specific events, this is the house for that event.
		*/
		HouseClass * House;

		/*
		 * This points to the next trigger type hanging off the same tag, or is NULL if this
		 * is the last of them. Springing the tag creates and fires a trigger for every type
		 * in the chain, so a single tag may carry several unrelated behaviors.
		 */
		TriggerTypeClass * LinkedTo;

		/*
		**	Each trigger must have an event which activates it. This is the event that is
		**	used to activate this trigger.
		*/
		TEventClass * FirstEvent;

		/*
		**	This is the action to perform when the trigger event occurs.
		*/
		TActionClass * FirstAction;

		/*
		 * These are the names of the map file sections that the trigger types, their
		 * actions and their events are stored under.
		 */
		static char const * const INI_NAME;
		static char const * const INI_ACTION_NAME;
		static char const * const INI_EVENT_NAME;
};

extern DynamicVectorClass<TriggerTypeClass *> TriggerTypes;

