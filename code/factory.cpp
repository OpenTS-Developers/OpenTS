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

/* $Header: /CounterStrike/FACTORY.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FACTORY.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 12/26/94                                                     *
 *                                                                                             *
 *                  Last Update : May 22, 1995 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   FactoryClass::AI -- Process factory production logic.                                     *
 *   FactoryClass::Abandon -- Abandons current construction with money refunded.               *
 *   FactoryClass::Completed -- Clears factory object after a completed production process.    *
 *   FactoryClass::Completion -- Fetches the completion step for this factory.                 *
 *   FactoryClass::Cost_Per_Tick -- Breaks entire production cost into manageable chunks.      *
 *   FactoryClass::FactoryClass -- Default constructor for factory objects.                    *
 *   FactoryClass::Get_Object -- Fetches pointer to object being constructed.                  *
 *   FactoryClass::Get_Special_Item -- gets factory's spc prod item                            *
 *   FactoryClass::Has_Changed -- Checks to see if a production step has occurred?             *
 *   FactoryClass::Has_Completed -- Checks to see if object has completed production.          *
 *   FactoryClass::Set -- Assigns a factory to produce an object.                              *
 *   FactoryClass::Set -- Force factory to "produce" special object.                           *
 *   FactoryClass::Start -- Resumes production after suspension or creation.                   *
 *   FactoryClass::Suspend -- Temporarily stop production.                                     *
 *   FactoryClass::operator delete -- Returns a factory to the free factory pool.              *
 *   FactoryClass::operator new -- Allocates a factory object from the free factory pool.      *
 *   FactoryClass::~FactoryClass -- Default destructor for factory objects.                    *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "factory.h"

#include "_rules.h"
#include "building.h"
#include "crc.h"
#include "dbgprint.h"
#include "globals.h"
#include "house.h"
#include "rules.h"
#include "savestream.h"
#include "sun.h"
#include "swizzle.h"
#include "tracker.h"
#include "voc.h"

#include "super.hh"

#include <algorithm>

/***********************************************************************************************
 * FactoryClass::FactoryClass -- Default constructor for factory objects.                      *
 *                                                                                             *
 *    This brings the factory into a null state. It is called when a factory object is         *
 *    created.                                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
FactoryClass::FactoryClass(void) :
	BASECLASS(),
	QueuedObjects(),
	IsDifferent(false),
	Object(0),
	Balance(0),
	OriginalBalance(0),
	SpecialItem(SUPER_NONE),
	House(0),
	IsSuspended(false),
	IsOnHold(true)
{
	Create_ID();

	Factories.Add(this);

	Set_Rate(0);
	Set_Stage(0);

	ObjectPtrTracker.Add(this);
}


/***********************************************************************************************
 * FactoryClass::~FactoryClass -- Default destructor for factory objects.                      *
 *                                                                                             *
 *    This cleans up a factory object in preparation for deletion. If there is currently       *
 *    an object in production, it is abandoned and money is refunded.                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
FactoryClass::~FactoryClass(void)
{
	Detach_This_From_All(this, true);

	Factories.Delete(this);

	if (GameActive) {
		Abandon();
	}

	ObjectPtrTracker.Delete(this);
}


/***********************************************************************************************
 * FactoryClass::AI -- Process factory production logic.                                       *
 *                                                                                             *
 *    This routine should be called once per game tick. It handles the production process.     *
 *    As production proceeds, money is deducted from the owner object's house. When production *
 *    completes, the factory stop processing. A call to Abandon, Delete, or Completed is       *
 *    required after that point.                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *   01/04/1995 JLB : Uses exact installment payment method.                                   *
 *=============================================================================================*/
void FactoryClass::AI(void)
{
	if (!IsSuspended && (Object != NULL || SpecialItem)) {
		for (int index = 0; index < 1; index++) {
			if (!Has_Completed() && Graphic_Logic() ) {
				IsDifferent = true;

				int cost = Cost_Per_Tick();

				cost = std::min(cost, Balance);

				/*
				**	Enough time has expired so that another production step can occur.
				**	If there is insufficient funds, then go back one production step and
				**	continue the countdown. The idea being that by the time the next
				**	production step occurs, there may be sufficient funds available.
				*/
				if (cost > House->Available_Money()) {
					Set_Stage(Fetch_Stage()-1);
				} else {
					House->Spend_Money(cost);
					Balance -= cost;
				}

				/*
				**	If the production has completed, then suspend further production.
				*/
				if (Fetch_Stage() == STEP_COUNT) {
					IsSuspended = true;
					Set_Rate(0);
					House->Spend_Money(Balance);
					Balance = 0;
				}
			}
		}
	}
}


/***********************************************************************************************
 * FactoryClass::Has_Changed -- Checks to see if a production step has occurred?               *
 *                                                                                             *
 *    Use this routine to determine if production has advanced at least one step. By using     *
 *    this function, intelligent rendering may be performed.                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Has the production process advanced one step since the last time this        *
 *                function was called?                                                         *
 *                                                                                             *
 * WARNINGS:   This function clears the changed status flag as a side effect.                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FactoryClass::Has_Changed(void)
{
	bool changed = IsDifferent;
	IsDifferent = false;
	return(changed);
}


/***********************************************************************************************
 * FactoryClass::Set -- Assigns a factory to produce an object.                                *
 *                                                                                             *
 *    This routine initializes a factory to produce the object specified. The desired object   *
 *    type is created and placed in suspended animation (limbo) until such time as production  *
 *    completes. Production is not actually started by this routine. An explicit call to       *
 *    Start() is required to begin production.                                                 *
 *                                                                                             *
 * INPUT:   object   -- Reference to the object type class that is to be produced.             *
 *                                                                                             *
 *          house    -- Reference to the owner of the object to be produced.                   *
 *                                                                                             *
 * OUTPUT:  bool; Was production successfully prepared for this factory object. Failure means  *
 *                that the object could not be created. This is catastrophic and in such       *
 *                cases, the factory object should be deleted.                                 *
 *                                                                                             *
 * WARNINGS:   Be sure to examine the return value from this function. Failure to initialize   *
 *             the factory means that the factory is useless and should be deleted.            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FactoryClass::Set(TechnoTypeClass const & object, HouseClass & house, bool resume)
{
	/*
	**	If there is any production currently in progress, abandon it.
	*/
	if (object.RTTI == RTTI_BUILDINGTYPE) {
		Abandon();
	}

	if (object.RTTI != RTTI_BUILDINGTYPE && (Is_Building() || QueuedObjects.Count() > 0 || Is_Suspended()) && !resume) {
		if (QueuedObjects.Count() < Rule->MaximumQueuedObjects && !House->Is_Build_Limited(&object)) {
			QueuedObjects.Add(&object);
			return(true);
		}
		if (house.Is_Player_Control()) {
			Sound_Effect(Rule->ScoldSound);
		}
		return(false);
	}

	/*
	**	Set up the factory for the new production process.
	*/
	IsDifferent = true;
	IsSuspended = true;
	Set_Rate(0);
	Set_Stage(0);

	/*
	**	Create an object of the type requested.
	*/
	Object = (TechnoClass *)object.Create_One_Of(&house);

	/*
	**	Buildings that are constructed, will default to rebuilding on so that
	**	repair can commence and base rebuilding can occur.
	*/
	if (!house.Is_Human_Player() && Object != NULL && Object->RTTI == RTTI_BUILDING) {
		((BuildingClass *)Object)->IsToRebuild = true;
	}

	if (Object) {
		House  = Object->House;
		Balance = object.Cost_Of(House);
		Object->PurchasePrice = Balance;
	}

	/*
	**	If all was set up successfully, then return true.
	*/
	return(Object != NULL);
}


/***********************************************************************************************
 * FactoryClass::Set -- Fills a factory with an already completed object.                      *
 *                                                                                             *
 *    This routine is called when a produced object is in placement mode but then placement    *
 *    is suspended. The object must then return to the factory as if it were newly completed   *
 *    and awaiting removal.                                                                    *
 *                                                                                             *
 * INPUT:   object   -- The object to return to the factory.                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This will abandon any current object being produced at the factory in order     *
 *             to set the new object into it.                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void FactoryClass::Set(TechnoClass & object)
{
	Abandon();
	Object = &object;
	House  = Object->House;
	Balance = 0;
	Set_Rate(0);
	Set_Stage(STEP_COUNT);
	IsDifferent = true;
	IsSuspended = true;
}


/***********************************************************************************************
 * FactoryClass::Suspend -- Temporarily stop production.                                       *
 *                                                                                             *
 *    This routine will suspend production until a subsequent call to Start() or Abandon().    *
 *    Typical use of this function is when the player puts production on hold or when there    *
 *    is insufficient funds.                                                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was production actually stopped? A false return value indicates that the     *
 *                factory was empty or production was already stopped (or never started).      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FactoryClass::Suspend(bool onhold)
{
	if (!IsSuspended) {
		IsOnHold = onhold;
		IsSuspended = true;
		Set_Rate(0);
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * FactoryClass::Start -- Resumes production after suspension or creation.                     *
 *                                                                                             *
 *    This function will start the production process. It works for newly created factory      *
 *    objects, as well as if production had been suspended previously.                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was production started? A false return value means that the factory is       *
 *                empty or there is insufficient credits to begin production.                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FactoryClass::Start(bool onhold)
{
	if ((Object || SpecialItem) && IsSuspended && !Has_Completed()) {
			IsSuspended = false;
		Set_Rate(Build_Rate());
		if (House->Available_Money() >= Cost_Per_Tick()) {
			IsOnHold = true;
			if (onhold) {
				Suspend(true);
			}
			return(true);
		}
	}
	return(false);
}


/// <summary>
/// Fetches the production step delay for the current object.
/// This routine turns the object's build time into the pause between production steps,
/// so that the factory creeps forward a step at a time rather than all at once. An
/// idle factory is given the fastest rate, since there is nothing to slow down.
/// </summary>
/// <returns>Returns with the number of game frames between production steps.</returns>
int FactoryClass::Build_Rate(void) const
{
	int time = 0;

	if (Object) {
		time = Object->Time_To_Build();
	}

	time /= STEP_COUNT;
	time = std::clamp(time, 1, 255);

	return(time);
}


/***********************************************************************************************
 * FactoryClass::Abandon -- Abandons current construction with money refunded.                 *
 *                                                                                             *
 *    This routine is used when construction is to be abandoned and current money spend is     *
 *    to be refunded. This function effectively clears out this factory of all record of the   *
 *    producing object so that it may either be deleted or started anew with the Set()         *
 *    function.                                                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was an object actually abandoned? A false return value indicates that the    *
 *                factory was not producing any object.                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FactoryClass::Abandon(void)
{
	if (Object) {

		if (Object) {

			DebugString("Abandoning production of %s\n", (char const *)Object->Class_Of()->GivenName);

			/*
			**	Refund all money expended so far, back to the owner of the object under construction.
			*/
			int money = Object->Class_Of()->Cost_Of(Object->House);
			House->Refund_Money(money - Balance);
			Balance = 0;
		}
		if (SpecialItem) {
			SpecialItem = SUPER_NONE;
		}

		/*
		**	Set the factory back to the idle and empty state.
		*/
		Set_Rate(0);
		Set_Stage(0);
		IsSuspended = true;
		IsDifferent = true;

		if ( !House->Is_Human_Player()) {
			if (Object->RTTI == RTTI_INFANTRY) {
				House->BuildInfantry = INFANTRY_NONE;
			}
			if (Object->RTTI == RTTI_UNIT) {
				House->BuildUnit = UNIT_NONE;
			}
			if (Object->RTTI == RTTI_AIRCRAFT) {
				House->BuildAircraft = AIRCRAFT_NONE;
			}
			if (Object->RTTI == RTTI_BUILDING) {
				House->BuildStructure = STRUCT_NONE;
			}
		}

		/*
		**	Delete the object under construction.
		*/
		ScenarioInit++;
		delete Object;
		Object = NULL;
		ScenarioInit--;

		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * FactoryClass::Completion -- Fetches the completion step for this factory.                   *
 *                                                                                             *
 *    Use this routine to determine what animation (or completion step) the factory is         *
 *    currently on.                                                                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns a completion step number between 0 (uncompleted), to STEP_COUNT (completed)*
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int FactoryClass::Completion(void)
{
	return(Fetch_Stage());
}


/***********************************************************************************************
 * FactoryClass::Has_Completed -- Checks to see if object has completed production.            *
 *                                                                                             *
 *    Use this routine to examine the factory object in order to determine if the associated   *
 *    object has completed production and is awaiting placement.                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is the associated object to the factory completed and ready for placement?   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FactoryClass::Has_Completed(void)
{
	if (Object && Fetch_Stage() == STEP_COUNT) {
		return(true);
	}
	if (SpecialItem != SUPER_NONE && Fetch_Stage() == STEP_COUNT) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * FactoryClass::Get_Object -- Fetches pointer to object being constructed.                    *
 *                                                                                             *
 *    This routine gets the pointer to the currently constructing object.                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the object undergoing construction.                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
TechnoClass * FactoryClass::Get_Object(void) const
{
	return(Object);
}


/***************************************************************************
 * FactoryClass::Get_Special_Item -- gets factory spc prod item            *
 *                                                                         *
 * INPUT:      none                                                        *
 *                                                                         *
 * OUTPUT:     int the item the factory is currently working on            *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/05/1995 PWG : Created.                                             *
 *=========================================================================*/
int FactoryClass::Get_Special_Item(void) const
{
	return(SpecialItem);
}


/***********************************************************************************************
 * FactoryClass::Cost_Per_Tick -- Breaks entire production cost into manageable chunks.        *
 *                                                                                             *
 *    Use this routine to determine the cost per game "tick" to produce the object.            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the number of credits necessary to advance production one game tick.       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int FactoryClass::Cost_Per_Tick(void)
{
	if (Object) {
		int steps = STEP_COUNT - Fetch_Stage();
		if (steps) {
			return(Balance / steps);
		}
		return(Balance);
	}
	return(0);
}


/***********************************************************************************************
 * FactoryClass::Completed -- Clears factory object after a completed production process.      *
 *                                                                                             *
 *    This routine is called after production completes, and the object produced has been      *
 *    placed into the game. It resets the factory for deletion or starting of new production.  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Did any resetting occur? Failure is the result of the factory not having     *
 *                any completed object. An immediate second call to this routine will also     *
 *                yield false.                                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FactoryClass::Completed(void)
{
	if (Object && Fetch_Stage() == STEP_COUNT) {
		Object = NULL;
		IsSuspended = true;
		IsDifferent = true;
		Set_Stage(0);
		Set_Rate(0);
		return(true);
	}

	if (SpecialItem && Fetch_Stage() == STEP_COUNT) {
		SpecialItem = SUPER_NONE;
		IsSuspended = true;
		IsDifferent = true;
		Set_Stage(0);
		Set_Rate(0);
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the class identifier for a factory.
/// This routine is part of the persistence interface. The save game loader uses the
/// identifier to know what kind of object to create before handing it the stream.
/// </summary>
/// <returns>Returns with S_OK, or E_POINTER if no return location was supplied.</returns>
HRESULT FactoryClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_FactoryClass;
	return(S_OK);
}


/// <summary>
/// Lists the members this factory carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void FactoryClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);
	StageClass::Serialize(stream);

	stream.Serialize(QueuedObjects);
	stream.Serialize(Object);
	stream.Serialize(IsDifferent);
	stream.Serialize(Balance);
	stream.Serialize(OriginalBalance);
	stream.Serialize(SpecialItem);
	stream.Serialize(House);
	stream.Serialize(IsSuspended);
	stream.Serialize(IsOnHold);
}


/// <summary>
/// Submits this factory's state to the game checksum.
/// This routine is used by the multiplayer sync check to prove that every machine
/// agrees on what each factory is building and how far along it has got. It also
/// reports the same values to the debug log so that a desync can be read by eye.
/// </summary>
void FactoryClass::Compute_CRC(CRCEngine &crc) const
{
	BASECLASS::Compute_CRC(crc);

	crc(IsSuspended);
	crc(IsDifferent);
	crc(Balance);
	crc(OriginalBalance);

	if (Object != NULL) crc(Object->Fetch_ID());
	crc(SpecialItem);

	crc(House->Fetch_ID());
}


/// <summary>
/// Removes any reference this factory holds to the specified object.
/// This routine is called when an object is about to be destroyed, so that the factory
/// is not left pointing at the thing it was building.
/// </summary>
void FactoryClass::Detach(AbstractClass const * target, bool all)
{
	if (target == (AbstractClass *)Object) {
		Object = NULL;
	}
}


/// <summary>
/// Starts the next queued object building.
/// Use this routine when the factory has fallen idle. The object at the head of the
/// queue is handed back to the owning house, which begins production of it as though
/// the player had just clicked on it.
/// </summary>
void FactoryClass::Resume_Queue(void)
{
	if (QueuedObjects.Count()) {
		if (Object == NULL && (!Fetch_Rate() || IsSuspended)) {
			const TechnoTypeClass *object = QueuedObjects[0];
			QueuedObjects.Delete_Index(0);
			int id = object->Fetch_Heap_ID();
			if (id >= 0) {
				House->Begin_Production(object->RTTI, id, true);
			}
		}
	}
}


/// <summary>
/// Removes an object type from the production queue.
/// Only the queue is disturbed by this routine. Whatever is under construction keeps
/// building, even if it happens to be of the same type.
/// </summary>
/// <returns>bool; Was a queued object of that type found and removed?</returns>
bool FactoryClass::Remove_From_Queue(const TechnoTypeClass *type)
{
	return(QueuedObjects.Delete(type));
}


/// <summary>
/// Counts how many of an object type this factory has pending.
/// This routine tallies the object under construction together with any copies still
/// waiting in the queue, so that the sidebar can display the outstanding count.
/// </summary>
/// <returns>Returns with the number of objects of that type still pending.</returns>
int FactoryClass::Total(const TechnoTypeClass *type)
{
	int total = 0;
	if (Object != NULL) {
		if (Object->TClass == type) {
			total++;
		}
	}

	for (int i = 0; i < QueuedObjects.Count(); i++) {
		if (QueuedObjects[i] == type) {
			total++;
		}
	}
	return(total);
}


/// <summary>
/// Is the specified object type waiting in the production queue?
/// The object currently under construction is not considered to be queued.
/// </summary>
/// <returns>bool; Is that object type sitting in the queue?</returns>
bool FactoryClass::Is_Queued(const TechnoTypeClass *type)
{
	for (int i = 0; i < QueuedObjects.Count(); i++) {
		if (type == QueuedObjects[i]){
			return(true);
		}
	}
	return(false);
}


/// <summary>
/// Refreshes the production rate of a house's factories.
/// This routine is used after something has changed how fast a house can build, so that
/// every factory it owns picks up the new rate without losing its current progress.
/// </summary>
void Recalc_House_Factories(HouseClass *house)
{
	for (int i = 0; i < Factories.Count(); i++) {
		FactoryClass *fptr = Factories[i];
		if (fptr->Get_House() == house) {
			int rate = fptr->Build_Rate();
			fptr->Adjust_Rate(rate);
		}
	}
}
