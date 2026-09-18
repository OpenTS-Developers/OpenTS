/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

// Milliseconds since the first reading, which is all any caller compares.
unsigned int System_Milliseconds(void);

class MillisecondSystemTimerClass
{
	public:
		// The clock behind this counts from the start of the process, so a reading of it
		// means nothing in the process that loads it.
		static constexpr bool Reading_Survives_A_Save = false;

		int operator () (void) const;
		operator int (void) const;
};
