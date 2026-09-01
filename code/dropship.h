/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

class TechnoTypeClass;
class XSurface;
class ConvertClass;
class Blitter;


class DropshipLoadoutClass
{
	enum {
		MAX_ENTRIES = 5
	};

	public:
		DropshipLoadoutClass(void);
		~DropshipLoadoutClass(void);

		bool Add(TechnoTypeClass * ttype);
		bool Remove(int index);
		void Clear(void);
		TechnoTypeClass * Fetch(int index);

	private:
		friend void Dropship_Screen(void);
		static int Get_Allowable_Index(TechnoTypeClass * ttype);

	public:
		/// Unused
		int CreationFrame;
		int Unused1;
		int Unused2;
		bool UnusedBool1;

		/*
		 * This is the number of units currently loaded aboard the dropship.
		 */
		int EntryCount;

		/*
		 * These are the units the player has loaded aboard the dropship, in the order they
		 * were added. The list is kept gapless -- removing a unit closes up the slots behind
		 * it -- so the loadout screen can walk it straight through, and every slot from the
		 * count onwards is null, which is what lets the whole array travel to a save game.
		 */
		TechnoTypeClass * Entries[MAX_ENTRIES];

		/// Unused
		int TotalCost;

		// Carries this loadout to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(CreationFrame);
			stream.Serialize(Unused1);
			stream.Serialize(Unused2);
			stream.Serialize(UnusedBool1);
			stream.Serialize(EntryCount);
			stream.Serialize(Entries);
			stream.Serialize(TotalCost);
		}
};

void Dropship_Screen(void);
