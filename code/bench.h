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

/* $Header: /CounterStrike/BENCH.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : BENCH.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 07/17/96                                                     *
 *                                                                                             *
 *                  Last Update : July 17, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "mpu.h"
#include "timer.h"

/*
**	This is a timer access object that will fetch the internal Pentium
**	clock value.
*/
class PentiumTimerClass
{
	public:
		// The cycle counter's origin is the processor's, not the game's.
		static constexpr bool Reading_Survives_A_Save = false;

		unsigned int operator () (void) const {unsigned int h;unsigned int l = Get_CPU_Clock(h);return((l >> 4) | (h << 28));}
		operator unsigned int (void) const {unsigned int h;unsigned int l = Get_CPU_Clock(h);return((l >> 4) | (h << 28));}
};


/*
**	A performance tracking tool object. It is used to track elapsed time. Unlike a simple clock, this
**	class will keep a running average of the duration. Typical use of this would be to benchmark some
**	process that occurs multiple times. By benchmarking an average time, inconsistencies in a particular
**	run can be overcome.
*/
class Benchmark
{
	public:
		Benchmark(void);

		void Begin(bool reset=false);
		void End(void);

		void Reset(void);
		unsigned int Value(void) const;
		unsigned int Count(void) const {return(TotalCount);}

		unsigned int Step(void);

	private:
		/*
		**	The maximum number of events to keep running average of. If
		**	events exceed this number, then older events drop off the
		**	accumulated time. This number needs to be as small as
		**	is reasonable. The larger this number gets, the less magnitude
		**	that the benchmark timer can handle. Example; At a value of
		**	256, the magnitude of the timer can only be 24 bits.
		*/
		enum {MAXIMUM_EVENT_COUNT=256};

		/*
		**	This is the timer the is used to clock the events.
		*/
		BasicTimerClass<PentiumTimerClass> Clock;

		/*
		**	The total time off all events tracked so far.
		*/
		unsigned int Average;

		/*
		**	The total number of events tracked so far.
		*/
		unsigned int Counter;

		/*
		**	Absolute total number of events (possibly greater than the
		**	number of events tracked in the average).
		*/
		unsigned int TotalCount;
};
