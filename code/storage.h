/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once


class StorageClass
{
	public:
		StorageClass(void);
		~StorageClass(void) {};

		int Get_Total_Value(void) const;
		int Get_Total_Amount(void) const;
		int Get_Amount(int slot) const;
		int Increase_Amount(int amount, int slot);
		int Decrease_Amount(int amount, int slot);
		int First_Used_Slot(void) const;

		StorageClass operator+(StorageClass &that) const;
		StorageClass operator+=(StorageClass &that);
		StorageClass operator-(StorageClass &that) const;
		StorageClass operator-=(StorageClass &that);

		// Carries the stored amounts to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Values);
		}

	private:
		/*
		 * This is the amount held of each tiberium type, one slot per Tiberiums heap entry.
		 * Keeping them apart is what lets a refinery pay the right value for a mixed load.
		 */
		int Values[4];
};
