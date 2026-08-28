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
 **   C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S  I N C  **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : MMX.ASM                                  *
 *                                                                         *
 *                   Programmer : Steve Tall                               *
 *                                                                         *
 *                   Start Date : May 19th, 1996                           *
 *                                                                         *
 *                  Last Update : May 19th 1996 [ST]                       *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "getcpu.h"
#include "misc.h"
#include "mpu.h"

#include <cstring>
#include <intrin.h>

extern "C" {

char UseCMOV = 0;
char HasCMOV = 0;
char UseMMX = 0;
char CPUType = 0;

/*
 * Filled in by Detect_MMX_Availability from CPUID leaf 0. The buffer holds the twelve
 * vendor characters, the separating space the original wrote after them, and the
 * terminator Get_CPU_Type copies up to.
 */
char VendorID[20] = "Not available";

}


/// <summary>
/// Detects MMX support and records the processor family and vendor.
/// This routine leaves the family in CPUType, which Detect_CMOV_Availability then reads, so
/// it has to run first. It also fills VendorID.
/// </summary>
/// <returns>bool; Is MMX technology available?</returns>
bool __cdecl Detect_MMX_Availability(void)
{
	int regs[4];

	/*
	 * The original probed the EFLAGS AC and ID bits here to separate a 386 from a 486 from a
	 * processor carrying CPUID. Every processor the current build supports carries CPUID, so
	 * the family comes straight from it and the 386 and 486 answers are unreachable.
	 */
	char cputype = 4;

	__cpuid(regs, 0);
	int const maxleaf = regs[0];

	std::memcpy(&VendorID[0], &regs[1], 4);
	std::memcpy(&VendorID[4], &regs[3], 4);
	std::memcpy(&VendorID[8], &regs[2], 4);
	VendorID[12] = ' ';
	VendorID[13] = '\0';

	if (maxleaf >= 1) {
		__cpuid(regs, 1);
		cputype = (char)((regs[0] & 0x0F00) >> 8);
	}

	CPUType = cputype;

	if (CPUType < 5) {
		UseMMX = 0;
		return(false);
	}

	__cpuid(regs, 1);

	if ((regs[3] & 0x00800000) == 0) {
		UseMMX = 0;
		return(false);
	}

	UseMMX = 1;
	return(true);
}


/// <summary>
/// Detects CMOV support, distinguishing a processor that has the instruction from one the
/// original considered worth using it on. Reads the family recorded by
/// Detect_MMX_Availability, so it must be called after it.
/// </summary>
/// <returns>bool; Does the processor have CMOV?</returns>
bool __cdecl Detect_CMOV_Availability(void)
{
	if (CPUType < 5) {
		UseCMOV = 0;
		HasCMOV = 0;
		return(false);
	}

	int regs[4];
	__cpuid(regs, 1);

	if ((regs[3] & 0x00008000) == 0) {
		UseCMOV = 0;
		HasCMOV = 0;
		return(false);
	}

	/*
	 * A Pentium that reports CMOV still runs the non-CMOV path; only a later family takes it.
	 */
	HasCMOV = 1;
	UseCMOV = (CPUType > 5) ? 1 : 0;
	return(true);
}


/// <summary>
/// Fetches the processor's clock accumulator, which advances every clock tick. The value is
/// 64 bits wide; the low half is returned and the high half stored through the reference.
/// </summary>
/// <param name="high">Receives the high half of the 64 bit clock value.</param>
/// <returns>unsigned int; The low half of the clock value.</returns>
unsigned int __cdecl Get_CPU_Clock(unsigned int & high)
{
	unsigned long long const stamp = __rdtsc();

	high = (unsigned int)(stamp >> 32);
	return((unsigned int)stamp);
}


/// <summary>
/// Reports how capable the processor is. The original distinguished a 386 that could not
/// toggle the EFLAGS AC bit, a 486 that could not toggle the ID bit, and anything carrying
/// CPUID. The supported build requires SSE2, so CPUID is always present.
/// </summary>
/// <returns>WORD; PROC_PENTIUM, meaning the processor carries CPUID.</returns>
WORD __cdecl Processor(void)
{
	return(PROC_PENTIUM);
}
