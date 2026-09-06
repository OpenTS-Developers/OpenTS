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

/* $Header: /CounterStrike/FACTORY.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FACTORY.H                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 12/26/94                                                     *
 *                                                                                             *
 *                  Last Update : December 26, 1994 [JLB]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "abstract.h"
#include "stage.h"
#include "vector.h"
#include "techno.h"

template<class T> class DynamicVectorClass;
class TechnoTypeClass;
class TechnoClass;

class FactoryClass : public AbstractClass, private StageClass
{
		friend class HouseClass;

		typedef AbstractClass BASECLASS;

	public:
		friend void Recalc_House_Factories(HouseClass *house);

	public:
		FactoryClass(void);
		~FactoryClass(void);

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual void Detach(AbstractClass const * target, bool all = true) override;

		virtual RTTIType Fetch_RTTI(void) const override { return(RTTI_FACTORY); };
		virtual void Compute_CRC(CRCEngine &) const override;

		bool Abandon(void);
		bool Completed(void);
		bool Has_Changed(void);
		bool Has_Completed(void);
		bool Is_Building(void) const {return(Fetch_Rate() != 0 && !IsSuspended);};
		bool Is_Suspended(void) const {return(Object != NULL && IsSuspended);}
		bool Set(TechnoTypeClass const & object, HouseClass & house, bool resume);
		bool Set(int const & type, HouseClass & house);
		bool Start(bool onhold);
		bool Suspend(bool onhold=true);
		int Completion(void);
		TechnoClass * Get_Object(void) const;
		int Get_Special_Item(void) const;
		void AI(void);
		void Set(TechnoClass & object);
		HouseClass * Get_House(void) {return(House);};
		char const * Name(void) {return("Factory");}

		int Build_Rate(void) const;

		void Resume_Queue(void);
		bool Remove_From_Queue(const TechnoTypeClass *type);
		int Total(const TechnoTypeClass *type);
		bool Is_Queued(const TechnoTypeClass *type);

		bool Has_Production_Target(void) const
		{
			if (Object != NULL) return(true);
			if (QueuedObjects.Count() > 0) return(true);
			return(false);
		}

		bool Is_Currently_Producing(const TechnoTypeClass * type) const
		{
			if (Object == NULL) return(false);
			if (Object->TClass == NULL) return(true);
			if (Object->TClass == type) return(true);
			return(false);
		}

		enum StepCountEnum {
			STEP_COUNT=54			// Number of steps to break production down into.
		};
	protected:

		int Cost_Per_Tick(void);

	private:
		/*
		 * These are the object types waiting to be built once the current production has
		 * finished, in the order they were ordered. Buildings are never queued, and the
		 * queue holds no more than MaximumQueuedObjects entries.
		 */
		DynamicVectorClass<const TechnoTypeClass *> QueuedObjects;

		/*
		**	This is the object that is being produced. It is held in a state of limbo while
		**	undergoing production. Since the object is created at the time production is
		**	started, it is always available when production completes.
		*/
		TechnoClass * Object;

		/*
		**	If the AI process detected that the production process has advanced far enough
		**	that a change in the building animation would occur, this flag will be true.
		**	Examination of this flag (through the Has_Changed function) allows intelligent
		**	updating of any production graphic.
		*/
		bool IsDifferent;

		/*
		**	This records the balance due on the current production item. This value will
		**	be reduced as production proceeds. It will reach zero the moment production has
		**	finished. Using this method ensures that the total production cost will be EXACT
		**	regardless of the number of installment payments that are made.
		*/
		int Balance;
		int OriginalBalance;

		/*
		**	If the factory is not producing an object and is instead producing
		**	a special item, then special item will be set.
		*/
		int SpecialItem;

		/*
		**	The factory has to be doing production for one house or another.
		**	The house pointer will point to whichever house it is being done
		**	for.
		*/
		HouseClass  * House;

		/*
		**	If production is temporarily suspended, then this flag will be true. A factory
		**	is suspended when it is first created, when production has completed, and when
		**	explicitly instructed to Suspend() production. Suspended production is not
		**	abandoned. It may be resumed with a call to Start().
		*/
		bool IsSuspended;

		/*
		 * Reflects if the factory is suspended because of being put on hold, as opposed to being done
		 * producing the object or being unable to produce it.
		 */
		bool IsOnHold;
};

void Recalc_House_Factories(HouseClass *house);
