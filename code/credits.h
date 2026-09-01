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

/* $Header: /CounterStrike/CREDITS.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CREDIT.H                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 19, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 19, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once


/****************************************************************************
**	The animating credit counter display is controlled by this class.
*/
class CreditClass {
	public:
		int Credits;		// Value of credits trying to update display to.

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		CreditClass(void);

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
		void Update(bool forced=false, bool redraw=false);

		void Graphic_Logic(bool forced=false);
		void AI(bool forced=false);

		// Carries the credit readout to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Credits);
			stream.Serialize(Current);
			stream.Serialize(IsUp);
			stream.Serialize(IsAudible);
			stream.Serialize(Countdown);

			// not saved: IsToRedraw -- a redraw flag; the load asks for a complete draw anyway.
		}

		int Current;		// Credit value currently displayed.

		bool IsToRedraw;
		bool IsUp;
		bool IsAudible;

	private:
		int	Countdown;		// Delay between ticks.
};
