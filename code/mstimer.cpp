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

#include <chrono>


/// <summary>
/// Returns the milliseconds elapsed since the first call. The origin is this process, not
/// the machine, so readings are only meaningful against one another.
/// </summary>
unsigned int System_Milliseconds(void)
{
	using namespace std::chrono;

	static steady_clock::time_point const started = steady_clock::now();
	return((unsigned int)duration_cast<milliseconds>(steady_clock::now() - started).count());
}


/// <summary>
/// Fetches the current millisecond reading. This is the sampling routine that the timer
/// templates call whenever they need to know how much time has passed.
/// </summary>
int MillisecondSystemTimerClass::operator () (void) const
{
	return((int)System_Milliseconds());
}


/// <summary>
/// Converts the timer into its current millisecond reading, so that it can be used wherever
/// a plain time value is expected.
/// </summary>
MillisecondSystemTimerClass::operator int (void) const
{
	return((int)System_Milliseconds());
}
