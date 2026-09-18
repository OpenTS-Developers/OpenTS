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

/*
**	Timer objects that fetch the appropriate timer value according to
**	the type of timer they are.
*/
extern int Frame;
class FrameTimerClass
{
	public:
		// The frame counter travels in the save, so a reading of it means the same thing in
		// the process that loads it and must not be rebased.
		static constexpr bool Reading_Survives_A_Save = true;

		int operator () (void) const {return(Frame);};
		operator int (void) const {return(Frame);};
};
