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

/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : UTRACKER.CPP                             *
 *                                                                         *
 *                   Programmer : Steve Tall                               *
 *                                                                         *
 *                   Start Date : June 3rd, 1996                           *
 *                                                                         *
 *                  Last Update : June 7th, 1996 [ST]                      *
 *                                                                         *
 *-------------------------------------------------------------------------*
 *  The UnitTracker class exists to track the various statistics           *
 *   required for internet games.                                          *
 *                                                                         *
 *                                                                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *
 *                                                                         *
 *  Functions:                                                             *
 *                                                                         *
 *                                                                         *
 *                                                                         *
 *                                                                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "always.h"

#include "utracker.h"

#include "savestream.h"

#include <cstring>
#if defined(_WIN32)
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif


/***********************************************************************************************
 * UTC::UnitTrackerClass -- Class constructor                                                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Number of unit types to reserve space for                                         *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    6/7/96 0:10AM ST : Created                                                               *
 *=============================================================================================*/
UnitTrackerClass::UnitTrackerClass (int unit_count)
{
	UnitTotals = new int [unit_count]; // Allocate memory for the unit totals
	UnitCount = unit_count;             // Keep a record of how many unit entries there are
	InNetworkFormat = 0;                // The unit entries are in host format
	Clear_Unit_Total();                 // Clear each entry
}


/***********************************************************************************************
 * UTC::~UnitTrackerClass -- Class destructor                                                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    6/7/96 0:10AM ST : Created                                                               *
 *=============================================================================================*/
UnitTrackerClass::~UnitTrackerClass (void)
{
	delete [] UnitTotals;
}


/***********************************************************************************************
 * UTC::Increment_Unit_Total -- Increment the total for the specefied unit                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Unit number                                                                       *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    6/7/96 0:12AM ST : Created                                                               *
 *=============================================================================================*/
void UnitTrackerClass::Increment_Unit_Total(int unit_type)
{
	if (unit_type < UnitCount) {
		UnitTotals[unit_type]++;
	}
}


/***********************************************************************************************
 * UTC::Decrement_Unit_Total -- Decrement the total for the specefied unit                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Unit number                                                                       *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    6/7/96 0:13AM ST : Created                                                               *
 *=============================================================================================*/
void UnitTrackerClass::Decrement_Unit_Total(int unit_type)
{
	if (unit_type < UnitCount) {
		UnitTotals[unit_type]--;
	}
}


/***********************************************************************************************
 * UTC::Get_All_Totals -- Returns a pointer to the start of the unit totals list               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to unit totals list                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    6/7/96 0:13AM ST : Created                                                               *
 *=============================================================================================*/
int *UnitTrackerClass::Get_All_Totals (void)
{
	return(UnitTotals);
}


/***********************************************************************************************
 * UTC::Clear_Unit_Total -- Clear out all the unit totals                                      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    6/7/96 0:14AM ST : Created                                                               *
 *=============================================================================================*/
void UnitTrackerClass::Clear_Unit_Total (void)
{
	memset (UnitTotals, 0, UnitCount * sizeof(int) );
}


/***********************************************************************************************
 * UTC::To_Network_Format -- Changes all unit totals to network format for the internet        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    6/7/96 0:15AM ST : Created                                                               *
 *=============================================================================================*/
void UnitTrackerClass::To_Network_Format (void)
{
	if (!InNetworkFormat){
		for (int i=0 ; i<UnitCount ; i++){
			UnitTotals[i] = htonl (UnitTotals[i]);
		}
	}
	InNetworkFormat = 1;		// Flag that data is now in network format
}


/***********************************************************************************************
 * UTC::To_PC_Format -- Changes all unit totals to PC format from network format               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    6/7/96 0:15AM ST : Created                                                               *
 *=============================================================================================*/
void UnitTrackerClass::To_PC_Format (void)
{
	if (InNetworkFormat){
		for (int i=0 ; i<UnitCount ; i++){
			UnitTotals[i] = ntohl (UnitTotals[i]);
		}
	}
	InNetworkFormat = 0;		// Flag that data is now in PC format
}


/// <summary>
/// Saves or restores the totals in host order. A load takes the number of entries from the
/// save, since a house is rebuilt before the type lists its trackers are sized from.
/// </summary>
void UnitTrackerClass::Serialize(SaveStreamClass & stream)
{
	To_PC_Format();

	int count = UnitCount;
	stream.Serialize(count);
	if (stream.Is_Loading()) {
		if (!stream.Fits(count, sizeof(int))) {
			return;
		}
		if (count != UnitCount) {
			delete [] UnitTotals;
			UnitTotals = new int [count];
			UnitCount = count;
			Clear_Unit_Total();
		}
	}

	stream.Serialize_Bytes(UnitTotals, UnitCount * (int)sizeof(int));
}
