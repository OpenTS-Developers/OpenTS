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

#include "dbgprint.h"
#include "getcpu.h"
#include "mpu.h"
#include "win.h"

#define PERIOD_RESOLUTION 1 /// Use 1-millisecond target resolution.

/// Microsoft's macros for widening a large integer into a double.
#define ULi2Double(x) ((double)((x).u.HighPart) * 4.294967296E9 + (double)((x).u.LowPart))
#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

/// The same conversion, but taking the two halves of the value separately.
#define LI_TO_DBL(dh, dl) ((double)((double)dh * 4.294967296E9 + (double)dl))


/// <summary>
/// Creates the millisecond timer and works out how to drive it.
/// This routine will ask the processor for its clock rate so that the cycle counter can be
/// scaled into milliseconds. Machines that will not report a rate fall back to the Windows
/// multimedia timer, whose resolution is raised to one millisecond for the life of the timer.
/// </summary>
MillisecondTimerClass::MillisecondTimerClass(void)
{
	unsigned int high = 0;
	Frequency = 1.0;
	unsigned int low = Get_CPU_Rate(high);

	if (low == 0 && high == 0) {
		timeBeginPeriod(PERIOD_RESOLUTION);

	} else {
		double dl = low;
		double dh = high;

		DebugString("MillisecondTimerClass low = %u, high = %u\n", low, high);

		Frequency = LI_TO_DBL(dh, dl) / 1000; // 1000 = rate.

	}
}


/// <summary>
/// Releases the millisecond timer.
/// If this timer had to raise the system timer resolution in order to work, the resolution
/// is dropped back here so that the rest of the system is not left paying for it.
/// </summary>
MillisecondTimerClass::~MillisecondTimerClass(void)
{
	if (Frequency != 1.0) {
		timeEndPeriod(PERIOD_RESOLUTION);
	}
}


/// <summary>
/// Fetches the current time, expressed in milliseconds.
/// This routine is used every time the timer is read. The processor's own cycle counter
/// supplies the value when the machine is new enough to have one, since it is both cheaper
/// and finer grained than the system timer. Otherwise the Windows multimedia timer is
/// consulted instead.
/// </summary>
/// <returns>Returns with the current time in milliseconds.</returns>
MillisecondTimerClass::operator double () const
{
	static int cpu_type = -1;

	if (cpu_type == -1) {
		Get_CPU_Type(cpu_type, NULL, 0);
	}
	/// On extremely old CPUs (80486 and older) the TSC and rdtsc instruction don't exist.
	if (Frequency != 1.0 && cpu_type > 4) {
		unsigned int high;
		unsigned int low;

		low = Get_CPU_Clock(high);
		double dl = low;
		double dh = high;

		return(LI_TO_DBL(dh, dl) / Frequency);
	}

	return(timeGetTime());
}
