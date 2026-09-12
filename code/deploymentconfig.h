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

		// The files the game reads its rules, artwork and text from. Whether the expansion is
		// installed at all is decided by looking for the rules expansion.
		std::string RulesFile = "RULES.INI";
		std::string RulesExpansionFile = "FIRESTRM.INI";
		std::string ArtFile = "ART.INI";
		std::string ArtExpansionFile = "ARTFS.INI";
		std::string AIFile = "AI.INI";
		std::string AIExpansionFile = "AIFS.INI";
		std::string SoundFile = "SOUND.INI";
		std::string SoundExpansionFile = "SOUND01.INI";
		std::string ThemeFile = "THEME.INI";
		std::string ThemeExpansionFile = "THEME01.INI";
		std::string BattleFile = "BATTLE.INI";
		std::string BattleExpansionFile = "BATTLEFS.INI";
		std::string LanguageRulesFile = "LANGRULE.INI";
		std::string LanguageRulesExpansionFile = "LANGFS.INI";
		std::string TutorialFile = "TUTORIAL.INI";
		std::string UIFile = "UI.INI";

		// The file a player's own settings are read from and written back to.
		std::string SettingsFile = "SUN.INI";

		// The palettes in force until a theater is loaded, which the theater roster cannot name
		// because they are read before the rules that declare it.
		std::string SchemePaletteFile = "UNITSNO.PAL";
		std::string GamePaletteFile = "TEMPERAT.PAL";

		void Read_INI(INIClass const & ini);

		/*
		 * Returns every setting to its default and reads the file from the directory named,
		 * empty or separator-terminated, or from its INI or MIX folder; false when there is none.
		 */
		bool Read_File(char const * directory);
};
