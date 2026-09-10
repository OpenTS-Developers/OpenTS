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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/stimer.cpp                             $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 12/09/01 6:42p                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "stimer.h"

#include "hostclock.h"
#include "win.h"

#ifdef _MSC_VER
#pragma warning (push,3)
#endif


#ifdef _MSC_VER
#pragma warning (pop)
#endif


/// <summary>
/// Fetches the current system timer value.
/// This routine is the clock source that the timer templates are built upon. It scales
/// the host clock down so that timers tick in game sized units rather than in
/// milliseconds.
/// </summary>
/// <returns>Returns with the current system time, expressed in timer ticks.</returns>
int SystemTimerClass::operator () (void) const
{
	return(Host_Milliseconds()/16);
}


/// <summary>
/// Converts the timer object into its current tick count.
/// This routine lets a SystemTimerClass be used wherever a long is expected, yielding
/// the same reading that the function call operator would.
/// </summary>
/// <returns>Returns with the current system time, expressed in timer ticks.</returns>
SystemTimerClass::operator int (void) const
{
	return(Host_Milliseconds()/16);
}
