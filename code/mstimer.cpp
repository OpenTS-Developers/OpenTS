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


/*
 * One request held for the life of the process gives every reading millisecond resolution,
 * so a timer can be made and dropped as often as the game likes without asking again.
 */
static struct MillisecondResolutionClass
{
	MillisecondResolutionClass(void) { timeBeginPeriod(1); }
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
