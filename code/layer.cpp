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

/* $Header: /CounterStrike/LAYER.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : LAYER.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 31, 1994                                                 *
 *                                                                                             *
 *                  Last Update : March 10, 1995 [JLB]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   LayerClass::Sort -- Perform an incremental sort pass on the layer's objects.              *
 *   LayerClass::Sorted_Add -- Adds object in sorted order to layer.                           *
 *   LayerClass::Submit -- Adds an object to a layer list.                                     *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "layer.h"

#include "savestream.h"


/***********************************************************************************************
 * LayerClass::Submit -- Adds an object to a layer list.                                       *
 *                                                                                             *
 *    This routine is used to add an object to the layer list. If the list is full, then the   *
 *    object is not added.                                                                     *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object to add.                                          *
 *                                                                                             *
 * OUTPUT:  bool; Was the object added successfully?                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *   05/31/1994 JLB : Allows sorted insert.                                                    *
 *   01/02/1995 JLB : Fixed to work with EMSListOf template.                                   *
 *=============================================================================================*/
bool LayerClass::Submit(ObjectClass const * object, bool sort)
{
	/*
	**	Add the object to the layer. Either at the end (if "sort" is false) or at the
	**	appropriately sorted position.
	*/
	if (sort) {
		return((Sorted_Add(object)) != false);
	}
	return(Add((ObjectClass *)object) != false);
}


/***********************************************************************************************
 * LayerClass::Sort -- Handles sorting the objects in the layer.                               *
 *                                                                                             *
 *    This routine is used if the layer objects must be sorted and sorting is to occur now.    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Don't call this routine too often since it does take a bit of time to           *
 *             execute. It is a single pass binary sort and thus isn't horribly slow,          *
 *             but it does take some time.                                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *   03/10/1995 JLB : Uses comparison operator.                                                *
 *=============================================================================================*/
void LayerClass::Sort(void)
{
	for (int index = 0; index < Count()-1; index++) {
		ObjectClass *o1 = (*this)[index+1];
		ObjectClass *o2 = (*this)[index];
		int t1 = o1->Sort_Y();
		int t2 = o2->Sort_Y();
		if (t1 < t2) {
			(*this)[index+1] = o2;
			(*this)[index] = o1;
		}
	}
}


/***********************************************************************************************
 * DynamicVectorClass<T>::Sorted_Add -- Adds object in sorted order to vector.                 *
 *                                                                                             *
 *    Use this routine to add an object to the vector but it will be inserted in sorted        *
 *    order. This depends on the ">" operator being defined for the vector object.             *
 *                                                                                             *
 * INPUT:   object   -- Reference to the object that will be added to the vector.              *
 *                                                                                             *
 * OUTPUT:  bool; Was the object added to the vector successfully?                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int LayerClass::Sorted_Add(ObjectClass const * const object)
{
	if (!Grow()) {
		return(false);
	}

	/*
	**	There is room for the new object now. Add it to the right sorted position.
	*/
	int index;
	for (index = 0; index < ActiveCount; index++) {
		if ((*(*this)[index]) > (*object)) {
			break;
		}
	}

	/*
	**	Make room if the insertion spot is not at the end of the vector.
	*/
	for (int i = ActiveCount-1; i >= index; i--) {
		(*this)[i+1] = (*this)[i];
	}
	(*this)[index] = (ObjectClass *)object;
	ActiveCount++;
	return(true);
}


/// <summary>
/// Saves the layer to the data stream.
/// This routine is called as part of the persistent save process. The objects themselves
/// are written by their own owners -- only the layer's object pointers are recorded here,
/// to be swizzled back into real addresses when the game is loaded.
/// </summary>
/// <returns>Returns with S_OK if the layer was written. Otherwise, the failure code from
/// the stream is returned.</returns>
HRESULT LayerClass::Save(SaveStreamClass & stream)
{

	DynamicVectorClass<ObjectClass *>::Serialize(stream);
	return(stream.Result());
}


/// <summary>
/// Loads the layer from the data stream.
/// This routine is the counterpart to Save and is called while the save game is being
/// reconstructed. Whatever the layer was holding is discarded and the object pointers are
/// read back, so they do not become usable until the swizzle pass has run.
/// </summary>
/// <returns>Returns with S_OK if the layer was read. Otherwise, the failure code from the
/// stream is returned.</returns>
HRESULT LayerClass::Load(SaveStreamClass & stream)
{

	stream.Set_Context("LayerClass");
	DynamicVectorClass<ObjectClass *>::Serialize(stream);
	return(stream.Result());
}
