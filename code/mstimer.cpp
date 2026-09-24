/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "mstimer.h"

#include "win.h"


// Windows 10 SDKs such as 10.0.19041 lack this Windows 11 flag.
#ifndef PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION
#define PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION 0x4
#endif


// One request for the life of the process gives every timer millisecond resolution. Without
// the opt-out, Windows 11 drops it while the window is minimized and each sleep lasts about 16 ms.
static struct MillisecondResolutionClass
{
	MillisecondResolutionClass(void)
	{
		PROCESS_POWER_THROTTLING_STATE state = {};
		state.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
		state.ControlMask = PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION;
		state.StateMask = 0;
		SetProcessInformation(GetCurrentProcess(), ProcessPowerThrottling, &state, sizeof(state));
		timeBeginPeriod(1);
	}
	~MillisecondResolutionClass(void) { timeEndPeriod(1); }
} MillisecondResolution;


/// <summary>
/// Fetches the current millisecond reading of the system clock.
/// This is the sampling routine that the timer templates call whenever they need to
/// know how much time has passed.
/// </summary>
/// <returns>Returns with the number of milliseconds elapsed since Windows started.</returns>
int MillisecondSystemTimerClass::operator () (void) const
{
	return(timeGetTime());
}


/// <summary>
/// Converts the timer into its current millisecond reading.
/// This routine lets the timer object be used wherever a plain time value is expected.
/// </summary>
/// <returns>Returns with the number of milliseconds elapsed since Windows started.</returns>
MillisecondSystemTimerClass::operator int (void) const
{
	return(timeGetTime());
}
