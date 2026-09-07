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

/* $Header: /CounterStrike/CARGO.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CARGO.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 23, 1994                                               *
 *                                                                                             *
 *                  Last Update : 10/31/94 [JLB]                                               *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CargoClass::Attach -- Add unit to cargo hold.                                             *
 *   CargoClass::Attached_Object -- Determine attached unit pointer.                           *
 *   CargoClass::Debug_Dump -- Displays the cargo value to the monochrome screen.              *
 *   CargoClass::Detach_Object -- Removes a unit from the cargo hold.                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "cargo.h"

#include "foot.h"
#include "mono.h"


#ifdef _DEBUG
/***********************************************************************************************
 * CargoClass::Debug_Dump -- Displays the cargo value to the monochrome screen.                *
 *                                                                                             *
 *    This routine is used to dump the current cargo value to the monochrome monitor.          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/02/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void CargoClass::Debug_Dump(MonoClass * mono) const
{
	if (How_Many()) {
		mono->Set_Cursor(63, 3);
		mono->Printf("(%d)%04X", How_Many(), Attached_Object());
	}
}
#endif


/***********************************************************************************************
 * CargoClass::Attach_Group -- Add a chained group of units to a cargo hold.                   *
 *                                                                                             *
 *    This routine will add the specified unit to the cargo hold. The                          *
 *    unit will chain to any existing units in the hold. The chaining is                       *
 *    in a LIFO order.                                                                         *
 *                                                                                             *
 * INPUT:   object-- Pointer to the object to attach to the cargo hold.                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/23/1994 JLB : Created.                                                                 *
 *   10/31/94   JLB : Handles chained objects.                                                 *
 *=============================================================================================*/
void CargoClass::Attach_Group(FootClass * object)
{
	/*
	**	If there is no object, then no action is necessary.
	*/
	if (object == NULL) return;

	object->Limbo();

	/*
	**	Attach any existing cargo hold object to the end of the list as indicated by the
	**	object pointer passed into this routine. This is necessary because several objects may
	**	be attached at one time or several objects may be attached as a result of several calls
	**	to this routine. Either case must be handled properly.
	*/
	ObjectClass * o = NULL;
	if (object->Next && object->Next->Is_Foot()) {
		o = object->Next;
		while (o != NULL) {
			if (o->Next == NULL) break;
			if (!o->Next->Is_Foot()) {
				o = NULL;
				break;
			}
			o = o->Next;
			if (o == NULL) break;
		}
	}
	if (o != NULL) {
		o->Next = CargoHold;
	} else {
		object->Next = CargoHold;
	}

	/*
	**	Finally, assign the object pointer as the first object attached to this cargo hold.
	*/
	CargoHold = object;
	Quantity = 0;
	object = CargoHold;
	while (object != NULL) {
		Quantity++;
		if (object->Next == (void*)NULL) break;
		if (!object->Next->Is_Foot()) break;
		object = (FootClass *)(ObjectClass *)object->Next;
	}
}


/***********************************************************************************************
 * CargoClass::Detach_Object -- Removes a unit from the cargo hold.                            *
 *                                                                                             *
 *    This routine will take a unit from the cargo hold and extract it.                        *
 *    The unit extracted is the last unit added to the hold. If there                          *
 *    is no unit in the hold or the occupant is not a unit, then NULL is                       *
 *    returned.                                                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the unit that has been extracted.                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/23/1994 JLB : Created.                                                                 *
 *   06/07/1994 JLB : Handles generic object types.                                            *
 *=============================================================================================*/
FootClass * CargoClass::Detach_Object(void)
{
	TechnoClass * unit = Attached_Object();

	if (unit != NULL) {
		CargoHold = (FootClass *)(ObjectClass *)unit->Next;
		unit->Next = 0;
		Quantity--;
	}
	return((FootClass *)unit);
}


/***********************************************************************************************
 * CargoClass::Attached_Object -- Determine attached unit pointer.                             *
 *                                                                                             *
 *    This routine will return with a pointer to the attached unit if one                      *
 *    is present. One would need to know this if this is a transport                           *
 *    unit and it needs to unload.                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns a pointer to the attached unit. If there is no                             *
 *          attached unit, then return NULL.                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/07/1992 JLB : Created.                                                                 *
 *   06/07/1994 JLB : Handles generic object types.                                            *
 *=============================================================================================*/
FootClass * CargoClass::Attached_Object(void) const
{
	if (Is_Something_Attached()) {
		return(CargoHold);
	}
	return(NULL);
}


/// <summary>
/// Takes one object aboard, ignoring whatever its Next points at. Next doubles as the cell
/// occupier chain, so following it would drag whatever shares the passenger's cell into the
/// hold, the transport included. Attach_Group takes a deliberately chained group.
/// </summary>
/// <param name="object">The passenger to take aboard.</param>
void CargoClass::Attach(FootClass * object)
{
	if (object == NULL) return;

	object->Limbo();
	object->Next = CargoHold;
	CargoHold = object;
	Quantity++;
}


/// <summary>
/// Returns the combined Size of every passenger aboard. A hold whose passengers all carry
/// the default Size of one reports the same figure as How_Many.
/// </summary>
int CargoClass::Total_Size(void) const
{
	int size = 0;

	ObjectClass * object = CargoHold;
	while (object != NULL) {
		size += object->TClass->Size;
		if (object->Next == NULL || !object->Next->Is_Foot()) break;
		object = object->Next;
	}

	return(size);
}


/// <summary>
/// Removes a specific object from the cargo hold.
/// This routine will unlink the object from the passenger list wherever it happens to sit
/// in the chain. Use Detach_Object when unloading passengers in the normal order; use this
/// routine when one particular passenger must leave out of turn.
/// </summary>
/// <param name="object">The passenger to remove from the cargo hold.</param>
void CargoClass::Detach(FootClass * object)
{
	if (object != NULL && CargoHold != NULL) {
		if (CargoHold == object) {
			CargoHold = (FootClass *)CargoHold->Next;
			object->Next = NULL;
			Quantity--;
		} else {
			FootClass * o = CargoHold;
			if (o->Next != NULL) {
				while (o->Next != NULL && object != o->Next) {
					o = (FootClass*)o->Next;
					if (o->Next == NULL) return;
				}
				o->Next = o->Next->Next;
				object->Next = NULL;
				Quantity--;
			}
		}
	}
}
