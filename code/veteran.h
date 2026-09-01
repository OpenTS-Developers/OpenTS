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

/* $Header: /CounterStrike/CREW.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CREW.H                                                       *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 23, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 23, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once


#include "ability.hh"

/*
 * This class handles hero tracking.
 */
class VeterancyClass
{
	public:
		/*
		**	Constructors, Destructors, and overloaded operators.
		*/
		VeterancyClass(void);
		~VeterancyClass(void);

		void Made_A_Kill(int cost, int value);

		/*
		**	Query functions.
		*/
		bool Is_Dumbass(void) const;
		bool Is_Rookie(void) const;
		bool Is_Veteran(void) const;
		bool Is_Elite(void) const;

		/*
		 * Modifies the value based on the current XP level (unused).
		 */
		double Modify(double value);

		/*
		 * Sets the current XP level.
		 */
		void Set_Dumbass(bool set);
		void Set_Rookie(bool set);
		void Set_Veteran(bool set);
		void Set_Elite(bool set);

		/*
		 * I/O.
		 */
		int To_Integer(void) const;
		void From_Integer(int value);

		// Carries the accumulated experience to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Experience);
		}

	//private:

		/*
		 * This keeps track of the amount of experience the unit has accumulated.
		 * When it reaches a certain point, the unit improves.
		 */
		double Experience;
};

AbilityType Ability_From_Name(const char *name);
