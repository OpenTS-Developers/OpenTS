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

class INIClass;

/*
 * What a deployment asks of the game in its own OPENTS.INI, as against SUN.INI, which holds
 * a player's settings. Reading cannot fail: an unwritten key keeps its default.
 */
class DeploymentConfigClass
{
	public:
		// The folders its files are searched in, in the order written; a written list replaces this.
		std::string SearchPaths = "INI,MIX,Maps";

		// Whether a save carries the scenario file it was played from, which enlarges a save by half again.
		bool CarryScenarioFile = false;

		void Read_INI(INIClass const & ini);

		/*
		 * Returns every setting to its default and reads the file from the directory named,
		 * empty or separator-terminated, or from its INI or MIX folder; false when there is none.
		 */
		bool Read_File(char const * directory);
};
