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

/* $Header: /CounterStrike/GETCPU.CPP 1     3/03/97 10:24a Joe_bostic $*/
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : GETCPU                                                       *
 *                                                                                             *
 *                    File Name : GETCPU.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Steve Tall                                                   *
 *                                                                                             *
 *                   Start Date : 6/26/96                                                      *
 *                                                                                             *
 *                  Last Update : June 26th 1996 [ST]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Overview:                                                                                   *
 *   Example of interface to assembly language code to find CPU type                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 * Functions:                                                                                  *
 *   Get_CPU_Type -- interface to ASM detection code                                           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "getcpu.h"

#include <cstdio>
#include <cstring>

#include <intrin.h>

/***********************************************************************************************
 * Get_CPU_Type -- Find out what kind of CPU we are running on                                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    int   - reference to cpu type                                                     *
 *           bool  - reference to mmx availability flag                                        *
 *           char* - ptr to buffer to receive chip vendor info                                 *
 *           int   - length of above buffer                                                    *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    6/26/96 10:15AM ST : Created                                                             *
 *=============================================================================================*/

extern "C" {

char UseCMOV = 1;
char HasCMOV = 1;
char UseMMX = 1;
char CPUType = 0;

/*
 * Filled in by Detect_MMX_Availability from CPUID leaf 0. The buffer holds the twelve
 * vendor characters, the separating space the original wrote after them, and the
 * terminator Get_CPU_Type copies up to.
 */
char VendorID[20] = "Not available";

}


/// <summary>
/// Records the processor family in CPUType and the vendor in VendorID, and reports MMX
/// support. The supported minimum hardware (SSE2, so a Pentium 4 or Athlon 64 onward) always
/// has MMX, so this always sets UseMMX and returns true rather than reading the CPUID
/// feature bit.
/// </summary>
/// <returns>bool; always true on the supported minimum hardware.</returns>

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

	UseMMX = 1;
	return(true);
}


void Get_CPU_Type(int & cpu_type, bool & mmx, char * vendor_id, int vendor_id_length)
{
	/*
	**	Call the asm CPU detection code
	*/
	mmx = Detect_MMX_Availability();

	/*
	**	Return the promised results
	*/
	cpu_type = (int)CPUType;

	if (vendor_id != NULL) {
		strncpy(vendor_id, VendorID, vendor_id_length);
	}
}
