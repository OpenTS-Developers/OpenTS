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

/* $Header: /CounterStrike/TRACKER.CPP 1     3/03/97 10:26a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TRACKER.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 06/14/96                                                     *
 *                                                                                             *
 *                  Last Update : June 14, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Detach_This_From_All -- Detaches this object from all others.                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "tracker.h"

#include "_logic.h"
#include "_map.h"
#include "_rules.h"
#include "_tactica.h"
#include "abstract.h"
#include "aircraft.h"
#include "building.h"
#include "infantry.h"
#include "object.h"
#include "partsys.h"
#include "rules.h"
#include "tactical.h"
#include "unit.h"
#include "vector.h"

#include <typeinfo>


DynamicVectorClass<AbstractClass *> ObjectPtrTracker;
DynamicVectorClass<AbstractClass *> AbstractTypePtrTracker;
DynamicVectorClass<AbstractClass *> TagPtrTracker;
DynamicVectorClass<AbstractClass *> TriggerPtrTracker;
DynamicVectorClass<AbstractClass *> AnimPtrTracker;
DynamicVectorClass<AbstractClass *> FactoryPtrTracker;
DynamicVectorClass<AbstractClass *> EventActionPtrTracker;
DynamicVectorClass<AbstractClass *> WaypointPathPtrTracker;
DynamicVectorClass<AbstractClass *> TeamPtrTracker;
DynamicVectorClass<AbstractClass *> HousePtrTracker;
DynamicVectorClass<AbstractClass *> NeuronPtrTracker;

DynamicVectorClass<AbstractClass *> ObjectsToDelete;


/***********************************************************************************************
 * Detach_This_From_All -- Detaches this object from all others.                               *
 *                                                                                             *
 *    This routine sweeps through all game objects and makes sure that it is no longer         *
 *    referenced by them. Typically, this is called in preparation for the object's death      *
 *    or limbo state.                                                                          *
 *                                                                                             *
 * INPUT:   target   -- This object expressed as a target number.                              *
 *                                                                                             *
 *          all      -- Is this object really in truly being removed from the game? The        *
 *                      answer would be false if the target was actually a stealth             *
 *                      tank that is cloaking. In such a case, the object should be removed    *
 *                      from all non-friendly tracking systems, but otherwise left alone.      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void Detach_This_From_All(AbstractClass const * target, bool all)
{
	int index = 0;

	if (Map.PendingObjectPtr == target) {
		Map.PendingObjectPtr = NULL;
	}

	if (Map.PendingObject == target) {
		Map.PendingObject = NULL;
		Map.Set_Cursor_Shape(NULL);
	}

	if (Map.HoverObject == target) {
		Map.HoverObject = NULL;
	}

	if (target->RTTI == RTTI_PARTICLESYSTEM && target == GasSystem) {
		GasSystem = NULL;
	}

	if (target->RTTI == RTTI_HOUSE) {
		for (index = 0; index < HousePtrTracker.Count(); index++) {
			HousePtrTracker[index]->Detach(target, all);
		}
		Logic.Detach(target, all);
		/*
		 * nothing else needs to be done
		 */
		return;
	}

	if (target->RTTI == RTTI_ANIM) {
		for (index = 0; index < AnimPtrTracker.Count(); index++) {
			AnimPtrTracker[index]->Detach(target, all);
		}
		/*
		 * nothing else needs to be done
		 */
		return;
	}

	if (target->As_ObjectClass() != NULL) {
		for (index = 0; index < ObjectPtrTracker.Count(); index++) {
			ObjectPtrTracker[index]->Detach(target, all);
		}
		if (TacticalMap != NULL) {
			TacticalMap->Detach(target, true);
		}
		Logic.Detach(target, all);
		/*
		 * nothing else needs to be done
		 */
		return;
	}

	if (target->As_AbstractTypeClass() != NULL) {
		for (index = 0; index < AbstractTypePtrTracker.Count(); index++) {
			AbstractTypePtrTracker[index]->Detach(target, all);
		}
		Rule->Detach(target, all);
		/*
		 * nothing else needs to be done
		 */
		return;
	}

	switch ((RTTIType)target->RTTI)
	{
		case RTTI_TAG:
			for (index = 0; index < TagPtrTracker.Count(); index++) {
				TagPtrTracker[index]->Detach(target, all);
			}
			Map.Detach(target, true);
			Logic.Detach(target, all);
			break;

		case RTTI_TRIGGER:
			for (index = 0; index < TriggerPtrTracker.Count(); index++) {
				TriggerPtrTracker[index]->Detach(target, all);
			}
			break;

		case RTTI_FACTORY:
			for (index = 0; index < FactoryPtrTracker.Count(); index++) {
				FactoryPtrTracker[index]->Detach(target, all);
			}
			Map.Detach(target, true);
			break;

		case RTTI_EVENT:
		case RTTI_ACTION:
			for (index = 0; index < EventActionPtrTracker.Count(); index++) {
				EventActionPtrTracker[index]->Detach(target, all);
			}
			break;

		case RTTI_WAYPOINT:
			for (index = 0; index < WaypointPathPtrTracker.Count(); index++) {
				WaypointPathPtrTracker[index]->Detach(target, all);
			}
			break;

		case RTTI_TEAM:
			for (index = 0; index < TeamPtrTracker.Count(); index++) {
				TeamPtrTracker[index]->Detach(target, all);
			}
			break;

		case RTTI_NEURON:
			for (index = 0; index < NeuronPtrTracker.Count(); index++) {
				NeuronPtrTracker[index]->Detach(target, all);
			}
			break;
	}
}


/// <summary>
/// Disposes of the objects that are waiting on deferred deletion list.
/// This routine is called by the main game loop once the logic has finished with the
/// objects it destroyed. Any object that has gone inactive is taken off the list and
/// released; one that is still active is left behind for a later pass.
/// </summary>
void Process_Deferred_Deletion(void)
{
	int index = 0;
	while (index < ObjectsToDelete.Count()) {
		AbstractClass *obj = ObjectsToDelete[index];
		if (obj->Is_Inactive()) {
			while (true) {
				if (!ObjectsToDelete.Delete(obj)) {
					break;
				}
			}
			if (typeid(BuildingClass) == typeid(*obj)
			 || typeid(UnitClass) == typeid(*obj)
			 || typeid(InfantryClass) == typeid(*obj)
			 || typeid(AircraftClass) == typeid(*obj)) {
				((ObjectClass *)obj)->IsActive = true;
			}
			delete obj;
		} else {
			++index;
		}
	}
}
