/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "wdtnet.h"

bool WDT_Setup_Game(bool first_time);
bool Request_WDT_Cycle(void);
WDTState * WDT_Get_New_State(void);
void WDT_Start_New_Campaign(WDTState *state);
WDTTerritory *WDT_Get_Territory(int index);
bool WDT_Is_Allowed(void);

/// Owning smart pointer; deletes the held object on destruction and reassignment.
template <class T>
class WDTPointer
{
public:
	WDTPointer(T *ptr = NULL) : Ptr(ptr) {}
	~WDTPointer(void)
	{
		if (Ptr != NULL) {
			delete Ptr;
			Ptr = NULL;
		}
	}

	const WDTPointer<T> & operator=(T * ptr)
	{
		if (Ptr != NULL) {
			delete Ptr;
		}
		Ptr = ptr;
		return(*this);
	}

	T *operator *(void) const
	{
		return(Ptr);
	}

	/*
	 * This is the object the pointer owns. Assigning over it or destroying the pointer
	 * deletes it, which is the whole point of the wrapper.
	 */
	T *Ptr;
};
