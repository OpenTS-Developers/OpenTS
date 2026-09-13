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
 *                     $Archive:: /Commando/Library/STIMER.H                                  $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

/****************************************************************************
**	Timer constants. These are used when setting the countdown timer.
**	Note that this is based upon a timer that ticks every 60th of a second.
*/
#define	TIMER_SECOND			60
#define	TIMER_MINUTE			(TIMER_SECOND*60)

#define	FADE_PALETTE_FAST		(TIMER_SECOND/8)
#define	FADE_PALETTE_MEDIUM	(TIMER_SECOND/4)
#define	FADE_PALETTE_SLOW		(TIMER_SECOND/2)

#define	TICKS_PER_SECOND		15
#define	TICKS_PER_MINUTE		(TICKS_PER_SECOND * 60)
#define	TICKS_PER_HOUR			(TICKS_PER_MINUTE * 60)

#define	GRAYFADETIME			(1 * TICKS_PER_SECOND)


class SystemTimerClass
{
	public:
		// The clock behind this counts from the start of the process, so a reading of it
		// means nothing in the process that loads it.
		static constexpr bool Reading_Survives_A_Save = false;

		int operator () (void) const;
		operator int (void) const;
};
