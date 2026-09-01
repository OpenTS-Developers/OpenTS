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

#pragma once

void Get_CPU_Type(int & cpu_type, bool & mmx, char * vendor_id = 0, int vendor_id_length = 0);

extern "C" {
	bool __cdecl Detect_MMX_Availability(void);

	extern char CPUType;
	extern char VendorID[];

	/*
	 * Fixed true rather than probed: the supported minimum hardware (SSE2, so a Pentium 4 or
	 * Athlon 64 onward) always has CMOV and MMX.
	 */
	extern char UseCMOV;
	extern char HasCMOV;
	extern char UseMMX;
}

// Processor family constants. Get_CPU_Type reports the CPUID base family through its
// cpu_type parameter; callers compare it against these. PROC_80386 and PROC_80486 are
// unreachable on the supported minimum hardware, which always carries CPUID.
#define	PROC_80386			0
#define	PROC_80486			1
#define	PROC_PENTIUM		2
#define	PROC_PENTIUM_PRO	5

enum CPUType {
	CPU_UNKNOWN = -1,

	CPU_486,
	CPU_PENTIUM,

	CPU_PPRO,
	CPU_PII,
	CPU_PIII,
	CPU_CELERON,
	CPU_PIV,

	CPU_AMD,
	CPU_CYRIX,
	CPU_COUNT,
};
