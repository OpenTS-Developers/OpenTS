/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "milsectmr.h"

#include <chrono>


/// <summary>
/// Fetches the current time in milliseconds, with whatever resolution the steady clock
/// offers. The origin is this process, so readings are only meaningful against one another.
/// </summary>
MillisecondTimerClass::operator double () const
{
	using namespace std::chrono;

	static steady_clock::time_point const started = steady_clock::now();
	return(duration<double, std::milli>(steady_clock::now() - started).count());
}
