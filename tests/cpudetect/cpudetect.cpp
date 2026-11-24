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
// same family and vendor the instruction does. Needs no game data.

#include <windows.h>

#include <cstring>
#include <float.h>
#include <intrin.h>
#include <math.h>
#include <string>

#include "getcpu.h"
#include "mpu.h"

#include "doctest/doctest.h"

namespace {

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


std::string Reference_Vendor(void)
{
	int regs[4];
	__cpuid(regs, 0);

	char vendor[13];
	std::memcpy(&vendor[0], &regs[1], 4);
	std::memcpy(&vendor[4], &regs[3], 4);
	std::memcpy(&vendor[8], &regs[2], 4);
	vendor[12] = '\0';
	return(vendor);
}

}	// namespace


TEST_CASE("CPUID reports leaf 1")
{
	int regs[4];
	__cpuid(regs, 0);
	CHECK(regs[0] >= 1);
}


TEST_CASE("the processor detection reports the family and vendor CPUID does")
{
	int const family = Reference_Family();
	std::string const vendor = Reference_Vendor();
	INFO("vendor '", vendor, "', family ", family);

	int cpu_type = -1;
	char reported[64];
	std::memset(reported, 0, sizeof(reported));
	Get_CPU_Type(cpu_type, reported, sizeof(reported) - 1);

	CHECK_MESSAGE(cpu_type == family, "Get_CPU_Type reports the family");
	CHECK_MESSAGE(CPUType == (char)family, "the CPUType global holds the family");

	/*
	 * CPU_Id writes the twelve vendor characters and then a space, so the buffer
	 * Get_CPU_Type copies out is the vendor followed by that separator.
	 */
	CHECK_MESSAGE(std::string(reported) == vendor + " ", "the vendor string is the vendor and a space");
}


TEST_CASE("resetting the floating-point state undoes a changed rounding, denormal and exception setting")
{
#if defined(_M_IX86)
	unsigned int const mask = _MCW_EM | _MCW_RC | _MCW_PC | _MCW_DN;
	unsigned int const changed = _RC_CHOP | _PC_24 | _DN_FLUSH | (_MCW_EM & ~_EM_ZERODIVIDE);
#else
	unsigned int const mask = _MCW_EM | _MCW_RC | _MCW_DN;
	unsigned int const changed = _RC_CHOP | _DN_FLUSH | (_MCW_EM & ~_EM_ZERODIVIDE);
#endif

	// A pending status flag would trap once its exception is unmasked.
	_clearfp();

	unsigned int word = 0;
	REQUIRE(_controlfp_s(&word, changed, mask) == 0);
	CHECK((word & mask) == changed);

	Reset_Float_State();

	REQUIRE(_controlfp_s(&word, 0, 0) == 0);
	CHECK((word & mask) == (_CW_DEFAULT & mask));

#if defined(_M_X64)
	CHECK(_get_FMA3_enable() == 0);
#endif
}


/*
 * The clock accumulator only ever counts up, so a later read cannot be the smaller of the two
 * once both halves are put back together.
 */
TEST_CASE("the processor clock returns a running count that advances")
{
	unsigned int high1 = 0;
	unsigned int const low1 = Get_CPU_Clock(high1);
	unsigned int high2 = 0;
	unsigned int const low2 = Get_CPU_Clock(high2);

	unsigned long long const stamp1 = ((unsigned long long)high1 << 32) | low1;
	unsigned long long const stamp2 = ((unsigned long long)high2 << 32) | low2;

	CHECK(stamp1 != 0u);
	CHECK(stamp2 >= stamp1);
}
