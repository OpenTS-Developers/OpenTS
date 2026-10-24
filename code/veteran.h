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
#include <functional>

typedef enum VeterancyRankType {
	RANK_DUMBASS,
	RANK_ROOKIE,
	RANK_VETERAN,
	RANK_ELITE,

	RANK_NONE = -1,
} VeterancyRankType;


/*
 * This class handles hero tracking.
 */
class VeterancyClass
{
	public:
		// Event callbacks.
		using PromotedCallback = std::function<void(bool quiet)>;

		/*
		**	Constructors, Destructors, and overloaded operators.
		*/
		explicit VeterancyClass(PromotedCallback callback);
		~VeterancyClass(void);

		void Made_A_Kill(int cost, int value);

		/*
		**	Query functions.
		*/
		bool Is_Dumbass(void) const;
		bool Is_Rookie(void) const;
		bool Is_Veteran(void) const;
		bool Is_Elite(void) const;
		double Get_Experience(void) const;
		VeterancyRankType Get_Rank() const;

		/*
		 * Sets the current XP level.
		 */
		void Set_Dumbass();
		void Set_Rookie();
		void Set_Veteran(bool notify_player = true);
		void Set_Elite(bool notify_player = true);
		void Set_Experience(double value, bool notify_player = true);

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

	private:
		/*
		 * This keeps track of the amount of experience the unit has accumulated.
		 * When it reaches a certain point, the unit improves.
		 */
		double Experience;

		// Callback function for promotion events.
		PromotedCallback On_Unit_Promoted_Callback;		
};

AbilityType Ability_From_Name(const char *name);
