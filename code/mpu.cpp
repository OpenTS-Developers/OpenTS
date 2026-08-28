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
 *                     $Archive:: /Commando/Code/wwlib/mpu.cpp                                $*
 *                                                                                             *
 *                      $Author:: Denzil_l                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/23/01 5:07p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Get_CPU_Rate -- Fetch the rate of CPU ticks per second.                                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "mpu.h"

#include "win.h"

#include <intrin.h>
#include <math.h>

typedef union {
	LARGE_INTEGER LargeInt;
	struct QuadPart {
		unsigned int LowPart;
		unsigned int HighPart;
	} QuadPart;
} QuadValue;


/***********************************************************************************************
 * Get_CPU_Rate -- Fetch the rate of CPU ticks per second.                                     *
 *                                                                                             *
 *    This routine will query the CPU to determine how many clock per second it is.            *
 *                                                                                             *
 * INPUT:   high  -- Reference to the location that will be filled with the upper 32 bits      *
 *                   of the result.                                                            *
 *                                                                                             *
 * OUTPUT:  Returns with the lower 32 bits of the result.                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/20/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
unsigned int Get_CPU_Rate(unsigned int & high)
{
	union {
		LARGE_INTEGER LargeInt;
		struct {
			unsigned int LowPart;
			unsigned int HighPart;
		} QuadPart;
	} value;

	if (QueryPerformanceFrequency(&value.LargeInt)) {
		high = value.QuadPart.HighPart;
		return(value.QuadPart.LowPart);
	}
	high = 0;
	return(0);
}


/*
 * Based on code released by Intel
 * http://db.zmitac.aei.polsl.pl/Electronics_Firm_Docs/PENTIUMIII/pentium/cpuinfo.zip
 */

/*
**
** Cut and paste job from an intel example.
**
**
**
**
**
**
*/

// Max # of samplings to allow before giving up and returning current average.
#define MAX_TRIES			20
#define ROUND_THRESHOLD		6

// # of MHz to allow samplings to deviate from average of samplings.
#define TOLERANCE			1

static unsigned long TSC_Low;
static unsigned long TSC_High;

/// <summary>
/// Samples the processor's time stamp counter.
/// This routine executes the RDTSC opcode and stashes the two halves of the 64 bit cycle
/// count in the module's time stamp globals, where the timing code can pick them up.
/// </summary>
/// <remarks>Only call this routine on a processor that supports the RDTSC opcode.</remarks>
void RDTSC(void)
{
	unsigned long long const stamp = __rdtsc();

	TSC_Low = (unsigned long)(stamp & 0xFFFFFFFF);
	TSC_High = (unsigned long)(stamp >> 32);
}


/// <summary>
/// Determines the speed of the processor in megahertz.
/// This routine races the processor's time stamp counter against the Win32 high resolution
/// performance counter. The measurement is repeated until successive readings agree, so
/// that other activity on the machine cannot skew the result.
/// </summary>
/// <returns>Returns with the processor speed in megahertz. Zero is returned if the machine
/// has no high resolution performance counter.</returns>
/// <remarks>Only call this routine on a processor that supports the RDTSC opcode.</remarks>
int Get_RDTSC_CPU_Speed(void)
{
	LARGE_INTEGER t0,t1;
	unsigned int	freq=0;						// Most current freq. calc.
	unsigned int	freq2=0;						// 2nd most current freq. calc.
	unsigned int	freq3=0;						// 3rd most current freq. calc.
	unsigned int	total;						// Sum of previous three freq. calc.
	int	tries=0;						// Number of times a calculation has been
												// made on this call
	unsigned int	total_cycles=0, cycles;	// Clock cycles elapsed during test
	unsigned int	stamp0, stamp1;			// Time Stamp for beginning and end of test
	unsigned int	total_ticks=0, ticks;	// Microseconds elapsed during test
// unsigned int	current = 0;				// Elapsed time during loop
	LARGE_INTEGER count_freq;			// Hi-Res Performance Counter frequency


	if ( !QueryPerformanceFrequency(&count_freq) ) return(0);


#if 0 ///Change in Ren
	HANDLE process = GetCurrentProcess();
	DWORD processPri = GetPriorityClass(process);
	SetPriorityClass(process, REALTIME_PRIORITY_CLASS);

	HANDLE thread = GetCurrentThread();
	int threadPri = GetThreadPriority(thread);
	SetThreadPriority(thread, THREAD_PRIORITY_TIME_CRITICAL);
#endif

	/*
	** On processors supporting the TSC opcode, compare elapsed time on the
	** High-Resolution Counter with elapsed cycles on the Time Stamp Counter.
	*/

	do	{
		/*
		** This do loop runs up to 20 times or until the average of the previous
		** three calculated frequencies is within 1 MHz of each of the individual
		** calculated frequencies.   This resampling increases the accuracy of the
		** results since outside factors could affect this calculation.
		*/

		tries++;								// Increment number of times sampled
												//   on this call to cpuspeed

		freq3 = freq2;						// Shift frequencies back to make
		freq2 = freq;						//   room for new frequency measurement

		/*
		** Get high-resolution performance counter time
		*/
		QueryPerformanceCounter(&t0);

		t1.LowPart = t0.LowPart;		// Set Initial time
		t1.HighPart = t0.HighPart;

		/*
		** Loop until 50 ticks have passed since last read of hi-res counter.
		** This accounts for overhead later.
		*/
		while ( (unsigned int)t1.LowPart - (unsigned int)t0.LowPart<50) {
			QueryPerformanceCounter(&t1);
		}

		stamp0 = (unsigned long)(__rdtsc() & 0xFFFFFFFF);

		t0.LowPart = t1.LowPart;		// Reset Initial Time
		t0.HighPart = t1.HighPart;

		/*
		** Loop until 1000 ticks have passed since last read of hi-res counter.
		** This allows for elapsed time for sampling.
		*/
		while ( (unsigned int)t1.LowPart - (unsigned int)t0.LowPart < 1000 ) {
			QueryPerformanceCounter(&t1);
		}

		stamp1 = (unsigned long)(__rdtsc() & 0xFFFFFFFF);


		cycles = stamp1 - stamp0;					// # of cycles passed between reads

		ticks = ((unsigned int)t1.LowPart - (unsigned int)t0.LowPart);
		ticks = ticks * 100000;						// Convert ticks to hundred
															//   thousandths of a tick
		ticks = (unsigned int)(ticks / (count_freq.LowPart / 10));
															// Hundred Thousandths of a
															//   Ticks / ( 10 ticks/second )
															//   = microseconds (us)
		total_ticks += ticks;
		total_cycles += cycles;
		if ( (ticks % count_freq.LowPart) > (count_freq.LowPart/2) ) ticks++;		// Round up if necessary

		freq = cycles/ticks;							// MHz = cycles / us

		if ( cycles%ticks > ticks/2 ) freq++;	// Round up if necessary

		total = ( freq + freq2 + freq3 );		// Total last three frequency calcs

	} while ( (tries < 3 ) || (tries < 20) && ((abs((int)(3 * freq -total)) > 3*TOLERANCE )|| (abs((int)(3 * freq2-total)) > 3*TOLERANCE )|| (abs((int)(3 * freq3-total)) > 3*TOLERANCE )));

#if 0 ///Change in Ren
	SetThreadPriority(thread, threadPri);
	SetPriorityClass(process, processPri);
#endif

	/*
	** Try one more significant digit.
	*/
	freq3 = ( total_cycles * 10 ) / total_ticks;
	freq2 = ( total_cycles * 100 ) / total_ticks;

	if ( freq2 - (freq3 * 10) >= ROUND_THRESHOLD ) freq3++;

	int norm_freq = total_cycles / total_ticks;

	freq = norm_freq * 10;
	if ( (freq3 - freq) >= ROUND_THRESHOLD ) norm_freq++;

	return (norm_freq);

}


