/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Checks the processor detection in getcpu.cpp against CPUID read directly here. The
// detection used to be hand-written assembly, so the point is to confirm the C++ reports the
// same family and vendor the instruction does, and that MMX and CMOV are reported available
// unconditionally, as required by the supported minimum hardware. Needs no game data.

#include <windows.h>

#include <cstdio>
#include <cstring>
#include <intrin.h>

#include "getcpu.h"
#include "mpu.h"

namespace {

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-52s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


/*
 * The detection reads the base family field only, as the assembly did. An extended family is
 * deliberately not folded in, so this reference computes the value the same narrow way.
 */
int Reference_Family(void)
{
	int regs[4];
	__cpuid(regs, 1);
	return((regs[0] & 0x0F00) >> 8);
}


int Reference_Feature_Edx(void)
{
	int regs[4];
	__cpuid(regs, 1);
	return(regs[3]);
}

}	// namespace


int main(void)
{
	int regs[4];
	__cpuid(regs, 0);

	char vendor[16];
	std::memcpy(&vendor[0], &regs[1], 4);
	std::memcpy(&vendor[4], &regs[3], 4);
	std::memcpy(&vendor[8], &regs[2], 4);
	vendor[12] = '\0';

	int const maxleaf = regs[0];
	Check(maxleaf >= 1, "CPUID reports leaf 1");

	int const family = Reference_Family();
	int const edx = Reference_Feature_Edx();

	std::printf("Reported vendor '%s', family %d, feature EDX %08X\n\n", vendor, family, (unsigned int)edx);

	int cpu_type = -1;
	bool mmx = false;
	char reported[64];
	std::memset(reported, 0, sizeof(reported));

	Get_CPU_Type(cpu_type, mmx, reported, sizeof(reported) - 1);

	Check(cpu_type == family, "Get_CPU_Type family matches CPUID");
	Check(CPUType == (char)family, "CPUType global matches CPUID");

	/*
	 * Detect_MMX_Availability writes the twelve vendor characters and then a space, so the
	 * buffer Get_CPU_Type copies out is the vendor followed by that separator.
	 */
	char expected[16];
	std::memcpy(expected, vendor, 12);
	expected[12] = ' ';
	expected[13] = '\0';
	Check(std::strcmp(reported, expected) == 0, "Vendor string matches CPUID");

	/*
	 * The supported minimum hardware (SSE2, so a Pentium 4 or Athlon 64 onward) always carries
	 * MMX and CMOV, so detection reports both available unconditionally rather than reading
	 * the CPUID feature bits.
	 */
	Check(mmx, "Get_CPU_Type reports MMX available");
	Check(UseMMX != 0, "UseMMX global reports available");
	Check(HasCMOV != 0, "HasCMOV global reports available");
	Check(UseCMOV != 0, "UseCMOV global reports available");

	/*
	 * The clock accumulator only ever counts up, so a later read cannot be the smaller of
	 * the two once both halves are put back together.
	 */
	unsigned int high1 = 0;
	unsigned int const low1 = Get_CPU_Clock(high1);
	unsigned int high2 = 0;
	unsigned int const low2 = Get_CPU_Clock(high2);

	unsigned long long const stamp1 = ((unsigned long long)high1 << 32) | low1;
	unsigned long long const stamp2 = ((unsigned long long)high2 << 32) | low2;

	Check(stamp1 != 0, "Get_CPU_Clock returns a running count");
	Check(stamp2 >= stamp1, "Get_CPU_Clock advances");

	std::printf("\n%s\n", Failures == 0 ? "All checks passed." : "Some checks FAILED.");
	return(Failures == 0 ? 0 : 1);
}
