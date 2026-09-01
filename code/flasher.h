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

/* $Header: /CounterStrike/FLASHER.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FLASHER.H                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 28, 1994                                                 *
 *                                                                                             *
 *                  Last Update : May 28, 1994   [JLB]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once


class MonoClass;

class FlasherClass {
	public:
		/*
		**	When this object is targeted, it will flash a number of times. This is the
		**	flash control number. It counts down to zero and then stops. Odd values
		**	cause the object to be rendered in a lighter color.
		*/
		unsigned FlashCount;

		/*
		**	When an object is targeted, it flashes several times to give visual feedback
		**	to the player. Every other game "frame", this flag is true until the flashing
		**	is determined to be completed.
		*/
		bool IsBlushing;

		FlasherClass(void) {FlashCount = 0; IsBlushing = false;};
		~FlasherClass(void) {};

		// Carries the flash state to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(FlashCount);
			stream.Serialize(IsBlushing);
		}

#ifdef _DEBUG
		void Debug_Dump(MonoClass *mono) const;
#endif
		bool Process(void);
};
