/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "nettime.h"

#include <windows.h>


namespace NetTiming
{
	/// <summary>Reads the system's wrapping millisecond clock.</summary>
	Milliseconds SystemMillisecondClock::Now(void) const
	{
		return(static_cast<Milliseconds>(GetTickCount()));
	}


	/// <summary>Returns the process-wide network clock.</summary>
	MillisecondClock const & Default_Clock(void)
	{
		static SystemMillisecondClock clock;
		return(clock);
	}
}
