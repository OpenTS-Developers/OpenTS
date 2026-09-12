/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "deploymentconfig.h"

#include "dbgprint.h"
#include "ini.h"
#include "rawfile.h"

static char const * const ConfigName = "OPENTS.INI";

/*
 * The folders the file itself is looked for in, relative to the data directory.
 */
static char const * const ConfigProbes[] = {"", "INI\\", "MIX\\"};


void DeploymentConfigClass::Read_INI(INIClass const & ini)
{
	SearchPaths = ini.Get_String("Paths", "SearchPaths", SearchPaths.c_str());
	CarryScenarioFile = ini.Get_Bool("Saves", "CarryScenarioFile", CarryScenarioFile);
	RulesFile = ini.Get_String("Files", "Rules", RulesFile.c_str());
	RulesExpansionFile = ini.Get_String("Files", "RulesExpansion", RulesExpansionFile.c_str());
	ArtFile = ini.Get_String("Files", "Art", ArtFile.c_str());
	ArtExpansionFile = ini.Get_String("Files", "ArtExpansion", ArtExpansionFile.c_str());
	AIFile = ini.Get_String("Files", "AI", AIFile.c_str());
	AIExpansionFile = ini.Get_String("Files", "AIExpansion", AIExpansionFile.c_str());
	SoundFile = ini.Get_String("Files", "Sound", SoundFile.c_str());
	SoundExpansionFile = ini.Get_String("Files", "SoundExpansion", SoundExpansionFile.c_str());
	ThemeFile = ini.Get_String("Files", "Theme", ThemeFile.c_str());
	ThemeExpansionFile = ini.Get_String("Files", "ThemeExpansion", ThemeExpansionFile.c_str());
	BattleFile = ini.Get_String("Files", "Battle", BattleFile.c_str());
	BattleExpansionFile = ini.Get_String("Files", "BattleExpansion", BattleExpansionFile.c_str());
	LanguageRulesFile = ini.Get_String("Files", "LanguageRules", LanguageRulesFile.c_str());
	LanguageRulesExpansionFile = ini.Get_String("Files", "LanguageRulesExpansion", LanguageRulesExpansionFile.c_str());
	TutorialFile = ini.Get_String("Files", "Tutorial", TutorialFile.c_str());
	UIFile = ini.Get_String("Files", "UI", UIFile.c_str());
	SettingsFile = ini.Get_String("Files", "Settings", SettingsFile.c_str());
	SchemePaletteFile = ini.Get_String("Palettes", "Scheme", SchemePaletteFile.c_str());
	GamePaletteFile = ini.Get_String("Palettes", "Game", GamePaletteFile.c_str());
}


/// <summary>
/// Reads the deployment's file. It is read from the disk rather than through the game's
/// file system, so a deployment cannot hide the description of its own layout inside an
/// archive.
/// </summary>
/// <returns>bool; Was a file found?</returns>
bool DeploymentConfigClass::Read_File(char const * directory)
{
	*this = DeploymentConfigClass();

	if (directory == NULL) {
		directory = "";
	}

	for (char const * probe : ConfigProbes) {
		std::string const name = std::string(directory) + probe + ConfigName;
		RawFileClass file(name.c_str());

		if (!file.Is_Available()) {
			continue;
		}

		INIClass ini;
		ini.Load(file);
		Read_INI(ini);

		DebugString("[DeploymentConfig] Read %s.\n", name.c_str());
		return(true);
	}

	return(false);
}
