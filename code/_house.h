/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

class HouseClass;


struct AngerStruct
{
	AngerStruct(void) : House(NULL), Level(0) {}
	AngerStruct(HouseClass * house) : House(house), Level(0) {}
	AngerStruct(AngerStruct const &that) : House(that.House) { Level = that.Level; }

	bool operator==(const AngerStruct &that) const { return(House == that.House && Level == that.Level); }
	bool operator!=(const AngerStruct &that) const { return(House != that.House || Level != that.Level); }

	/*
	 * Pointer to the house that this record of anger is aimed at.
	 */
	HouseClass * House;

	/*
	 * This is how much anger has built up against that house. The angriest surviving non-ally
	 * becomes this house's Enemy, and every level decays slowly as time passes.
	 */
	int Level;

	// Carries this anger record to or from a save game.
	template<typename S>
	void Serialize(S & stream)
	{
		stream.Serialize(House);
		stream.Serialize(Level);
	}
};


struct ScoutStruct
{
	ScoutStruct(void) : House(NULL), IsScouted(false) {}
	ScoutStruct(HouseClass * house) : House(house), IsScouted(false) {}

	bool operator==(const ScoutStruct &that) const { return(House == that.House && IsScouted == that.IsScouted); }
	bool operator!=(const ScoutStruct &that) const { return(House != that.House || IsScouted != that.IsScouted); }

	/*
	 * Pointer to the house that this scouting record refers to.
	 */
	HouseClass * House;

	/*
	 * If that house has been found, then this flag will be true. A scout team only heads for
	 * houses that have not been laid eyes on yet.
	 */
	bool IsScouted;

	// Carries this scouting record to or from a save game.
	template<typename S>
	void Serialize(S & stream)
	{
		stream.Serialize(House);
		stream.Serialize(IsScouted);
	}
};
