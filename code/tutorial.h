/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <string>
#include <utility>
#include <vector>

class INIClass;

/*
 * The numbered lines a text trigger prints: those TUTORIAL.INI supplies, and those the
 * scenario being played puts in front of them.
 */
class TutorialTextClass
{
	public:
		void Read_Base(INIClass const & ini);
		void Read_Overrides(INIClass const & ini);
		void Clear_Overrides(void);
		char const * Fetch(int id) const;

		// Only the scenario's lines travel; the base set is read again as the game starts.
		template<typename StreamType>
		void Serialize(StreamType & stream) {stream.Serialize(Overrides);}

	private:
		using LineList = std::vector<std::pair<int, std::string>>;

		static void Read_Section(INIClass const & ini, LineList & into);
		static char const * Find(LineList const & lines, int id);

		LineList Base;
		LineList Overrides;
};

extern TutorialTextClass TutorialText;
