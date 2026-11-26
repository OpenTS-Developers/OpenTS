/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Exercises the deployment's configuration without the engine or any game data: what it
// supplies when there is no file, where the file is looked for and which copy wins, and what
// a written key changes. Every file this uses is one the harness makes itself.

#include <windows.h>

#include <cstdio>
#include <cstring>
#include <string>

#include "dbgprint.h"
#include "_deploymentconfig.h"
#include "deploymentconfig.h"
#include "rawfile.h"

#include "doctest/doctest.h"

namespace {

std::string Root;
char OriginalDirectory[MAX_PATH];

char const * const DefaultList = "INI,MIX,Maps";

// The file object keeps the name pointer it is handed rather than copying it, so the string
// it points into has to outlive it.
void Write_File(std::string const & path, char const * contents)
{
	RawFileClass file(path.c_str());

	file.Open(FileClass::WRITE);
	file.Write(contents, (int)strlen(contents));
	file.Close();
}


void Remove_File(std::string const & path)
{
	DeleteFile(path.c_str());
}


void Make_Directory(std::string const & path)
{
	CreateDirectory(path.c_str(), NULL);
}


/*
 * The harness works inside a tree of its own, with the current directory at its root so
 * that an unnamed directory means the root, as it means the game's own directory in play.
 */
bool Make_Root(void)
{
	char temp[MAX_PATH];
	if (GetTempPath(sizeof(temp), temp) == 0) {
		return(false);
	}

	char name[MAX_PATH];
	std::snprintf(name, sizeof(name), "%sopents-deploymentconfig-%lu", temp, GetCurrentProcessId());
	Root = name;

	Make_Directory(Root);
	Make_Directory(Root + "\\INI");
	Make_Directory(Root + "\\MIX");
	Make_Directory(Root + "\\Data");

	return(SetCurrentDirectory(Root.c_str()) != 0);
}


void Remove_Root(void)
{
	SetCurrentDirectory(OriginalDirectory);

	// The tree is shallow and entirely this harness's own, so it is removed by name.
	char command[MAX_PATH + 32];
	std::snprintf(command, sizeof(command), "cmd /c rd /s /q \"%s\"", Root.c_str());
	system(command);
}


bool Make_Root(void);
void Remove_Root(void);


// Each case works in a tree of its own, made fresh before it and removed after it.
struct RootFixture
{
	RootFixture(void)
	{
		GetCurrentDirectory(sizeof(OriginalDirectory), OriginalDirectory);
		REQUIRE_MESSAGE(Make_Root(), "the working directory is created");
	}

	~RootFixture(void)
	{
		Remove_Root();
	}
};


TEST_CASE_FIXTURE(RootFixture, "with no file the INI, MIX and Maps folders are searched and there is nothing to read")
{
	DeploymentConfigClass config;

	CHECK_MESSAGE(config.SearchPaths == DefaultList, "with no file the INI, MIX and Maps folders are searched");
	CHECK_MESSAGE(DeploymentConfig.SearchPaths == DefaultList, "the game's own copy starts from the same defaults");

	CHECK_MESSAGE(!config.Read_File(""), "with no file there is nothing to read");
	CHECK_MESSAGE(config.SearchPaths == DefaultList, "and the default stands");
}


TEST_CASE_FIXTURE(RootFixture, "a file beside the game is read, and the folders it names replace the default")
{
	Debug_Init(0, (const char **)NULL);
	DeploymentConfigClass config;

	Write_File(Root + "\\OPENTS.INI", "[Paths]\nSearchPaths=Data,More\n");

	CHECK_MESSAGE(config.Read_File(""), "a file beside the game is read");
	CHECK_MESSAGE(config.SearchPaths == "Data,More", "the folders it names replace the default");

	/*
	 * The reader passes over an entry with nothing after the equals sign, so a written key
	 * cannot empty the list; naming only the game's own directory is how a deployment asks
	 * for no other folder.
	 */
	Write_File(Root + "\\OPENTS.INI", "[Paths]\nSearchPaths=\n");

	CHECK_MESSAGE(config.Read_File(""), "a file with an empty list is still read");
	CHECK_MESSAGE(config.SearchPaths == DefaultList, "and the empty list leaves the default in force");

	Remove_File(Root + "\\OPENTS.INI");
}


TEST_CASE_FIXTURE(RootFixture, "the file is looked for beside the game, then in INI, then in MIX")
{
	Debug_Init(0, (const char **)NULL);
	DeploymentConfigClass config;

	Write_File(Root + "\\MIX\\OPENTS.INI", "[Paths]\nSearchPaths=FromMix\n");
	config.Read_File("");
	CHECK_MESSAGE(config.SearchPaths == "FromMix", "a file in the MIX folder is found");

	Write_File(Root + "\\INI\\OPENTS.INI", "[Paths]\nSearchPaths=FromIni\n");
	config.Read_File("");
	CHECK_MESSAGE(config.SearchPaths == "FromIni", "a file in the INI folder is read ahead of one in MIX");

	Write_File(Root + "\\OPENTS.INI", "[Paths]\nSearchPaths=Beside\n");
	config.Read_File("");
	CHECK_MESSAGE(config.SearchPaths == "Beside", "a file beside the game is read ahead of both");

	Remove_File(Root + "\\OPENTS.INI");
	Remove_File(Root + "\\INI\\OPENTS.INI");
	Remove_File(Root + "\\MIX\\OPENTS.INI");
}


TEST_CASE_FIXTURE(RootFixture, "the file is read from the directory named, and what it names is taken from there")
{
	Debug_Init(0, (const char **)NULL);
	DeploymentConfigClass config;
	std::string const data = Root + "\\Data\\";

	Write_File(data + "OPENTS.INI", "[Paths]\nSearchPaths=Sorted\n");

	CHECK_MESSAGE(config.Read_File(data.c_str()), "the file is read from the directory named");
	CHECK_MESSAGE(config.SearchPaths == "Sorted", "and what it names is taken from there");

	Remove_File(data + "OPENTS.INI");

	CHECK_MESSAGE(!config.Read_File(data.c_str()), "a file beside the game is not read for a directory named");
}


TEST_CASE_FIXTURE(RootFixture, "a read starts over, so with the file gone every setting returns to its default")
{
	Debug_Init(0, (const char **)NULL);
	DeploymentConfigClass config;

	Write_File(Root + "\\OPENTS.INI", "[Paths]\nSearchPaths=Data\n");
	config.Read_File("");
	Remove_File(Root + "\\OPENTS.INI");

	CHECK_MESSAGE(!config.Read_File(""), "with the file gone there is nothing to read");
	CHECK_MESSAGE(config.SearchPaths == DefaultList, "and every setting returns to its default");
}


TEST_CASE_FIXTURE(RootFixture, "a save carries the scenario file only when the file asks for it")
{
	Debug_Init(0, (const char **)NULL);
	DeploymentConfigClass config;

	CHECK_MESSAGE(!config.CarryScenarioFile, "with no file a save carries no scenario file");

	Write_File(Root + "\\OPENTS.INI", "[Paths]\nSearchPaths=Data\n");
	config.Read_File("");
	CHECK_MESSAGE(!config.CarryScenarioFile, "a file that does not ask for it leaves it off");

	Write_File(Root + "\\OPENTS.INI", "[Saves]\nCarryScenarioFile=yes\n");
	config.Read_File("");
	CHECK_MESSAGE(config.CarryScenarioFile, "a file asking for it turns it on");

	Write_File(Root + "\\OPENTS.INI", "[Saves]\nCarryScenarioFile=no\n");
	config.Read_File("");
	CHECK_MESSAGE(!config.CarryScenarioFile, "a file refusing it turns it off again");

	Write_File(Root + "\\OPENTS.INI", "[Saves]\nCarryScenarioFile=yes\n");
	config.Read_File("");
	Remove_File(Root + "\\OPENTS.INI");
	config.Read_File("");
	CHECK_MESSAGE(!config.CarryScenarioFile, "with the file gone it is off");
}


TEST_CASE_FIXTURE(RootFixture, "with no file the rules, multiplayer rules and settings come from their shipped files")
{
	Debug_Init(0, (const char **)NULL);
	DeploymentConfigClass config;

	CHECK_MESSAGE(config.RulesFile == "RULES.INI", "with no file the rules come from RULES.INI");
	CHECK_MESSAGE(config.MultiplayerRulesFile == "MPLAYER.INI", "and the multiplayer rules from MPLAYER.INI");
	CHECK_MESSAGE(config.SettingsFile == "SUN.INI", "and a player's settings from SUN.INI");

	Write_File(Root + "\\OPENTS.INI", "[Paths]\nSearchPaths=Data\n");
	config.Read_File("");
	CHECK_MESSAGE(config.ArtFile == "ART.INI", "a file that names none of them leaves the defaults");

	Write_File(Root + "\\OPENTS.INI",
			"[Files]\nRules=dtarules.ini\nArt=dtaart.ini\nAI=dtaai.ini\nSound=dtasound.ini\n"
			"Theme=dtatheme.ini\nBattle=dtabattle.ini\nLanguageRules=dtalang.ini\n"
			"MultiplayerRules=dtamplayer.ini\n"
			"Tutorial=dtatutorial.ini\nUI=dtaui.ini\nSettings=Settings.ini\n");
	config.Read_File("");
	CHECK_MESSAGE(config.RulesFile == "dtarules.ini", "a name it writes for the rules is taken");
	CHECK_MESSAGE(config.ArtFile == "dtaart.ini", "and for the artwork");
	CHECK_MESSAGE(config.AIFile == "dtaai.ini", "and for the AI");
	CHECK_MESSAGE(config.SoundFile == "dtasound.ini", "and for the sounds");
	CHECK_MESSAGE(config.ThemeFile == "dtatheme.ini", "and for the music");
	CHECK_MESSAGE(config.BattleFile == "dtabattle.ini", "and for the campaigns");
	CHECK_MESSAGE(config.LanguageRulesFile == "dtalang.ini", "and for the translated rules");
	CHECK_MESSAGE(config.MultiplayerRulesFile == "dtamplayer.ini", "and for the multiplayer rules");
	CHECK_MESSAGE(config.TutorialFile == "dtatutorial.ini", "and for the tutorial text");
	CHECK_MESSAGE(config.UIFile == "dtaui.ini", "and for the interface");
	CHECK_MESSAGE(config.SettingsFile == "Settings.ini", "and for a player's settings");
	CHECK_MESSAGE(config.ArtExpansionFile == "ARTFS.INI", "an expansion file it leaves alone keeps its name");

	Remove_File(Root + "\\OPENTS.INI");
	config.Read_File("");
	CHECK_MESSAGE(config.RulesFile == "RULES.INI", "with the file gone the names return to the defaults");
	CHECK_MESSAGE(config.SettingsFile == "SUN.INI", "every one of them");
}


TEST_CASE_FIXTURE(RootFixture, "the expansion rules have their shipped names, and a name the file writes is taken")
{
	Debug_Init(0, (const char **)NULL);
	DeploymentConfigClass config;

	CHECK_MESSAGE(config.RulesExpansionFile == "FIRESTRM.INI", "with no file the expansion rules are FIRESTRM.INI");
	CHECK_MESSAGE(config.MultiplayerRulesExpansionFile == "MPLAYERFS.INI", "and the expansion multiplayer rules are MPLAYERFS.INI");

	Write_File(Root + "\\OPENTS.INI",
			"[Files]\nRulesExpansion=fsrules.ini\nArtExpansion=fsart.ini\nAIExpansion=fsai.ini\n"
			"SoundExpansion=fssound.ini\nThemeExpansion=fstheme.ini\nBattleExpansion=fsbattle.ini\n"
			"LanguageRulesExpansion=fslang.ini\nMultiplayerRulesExpansion=fsmplayer.ini\n");
	config.Read_File("");
	CHECK_MESSAGE(config.RulesExpansionFile == "fsrules.ini", "a name it writes for the expansion rules is taken");
	CHECK_MESSAGE(config.ArtExpansionFile == "fsart.ini", "and for the expansion artwork");
	CHECK_MESSAGE(config.AIExpansionFile == "fsai.ini", "and for the expansion AI");
	CHECK_MESSAGE(config.SoundExpansionFile == "fssound.ini", "and for the expansion sounds");
	CHECK_MESSAGE(config.ThemeExpansionFile == "fstheme.ini", "and for the expansion music");
	CHECK_MESSAGE(config.BattleExpansionFile == "fsbattle.ini", "and for the expansion campaigns");
	CHECK_MESSAGE(config.LanguageRulesExpansionFile == "fslang.ini", "and for the translated expansion rules");
	CHECK_MESSAGE(config.MultiplayerRulesExpansionFile == "fsmplayer.ini", "and for the expansion multiplayer rules");
	CHECK_MESSAGE(config.RulesFile == "RULES.INI", "while the base files stand where it names none of them");

	Remove_File(Root + "\\OPENTS.INI");
	config.Read_File("");
	CHECK_MESSAGE(config.RulesExpansionFile == "FIRESTRM.INI", "with the file gone they return to the defaults");
}


TEST_CASE_FIXTURE(RootFixture, "the scheme and starting palettes have their shipped names, and names the file writes are taken")
{
	Debug_Init(0, (const char **)NULL);
	DeploymentConfigClass config;

	CHECK_MESSAGE(config.SchemePaletteFile == "UNITSNO.PAL", "with no file the scheme palette is UNITSNO.PAL");
	CHECK_MESSAGE(config.GamePaletteFile == "TEMPERAT.PAL", "and the starting palette TEMPERAT.PAL");

	Write_File(Root + "\\OPENTS.INI", "[Palettes]\nScheme=UNITTEM.PAL\nGame=DESERT.PAL\n");
	config.Read_File("");
	CHECK_MESSAGE(config.SchemePaletteFile == "UNITTEM.PAL", "the names it writes are taken");
	CHECK_MESSAGE(config.GamePaletteFile == "DESERT.PAL", "both of them");

	Write_File(Root + "\\OPENTS.INI", "[Palettes]\nScheme=UNITTEM.PAL\n");
	config.Read_File("");
	CHECK_MESSAGE(config.GamePaletteFile == "TEMPERAT.PAL", "and one named alone leaves the other at its default");

	Remove_File(Root + "\\OPENTS.INI");
	config.Read_File("");
	CHECK_MESSAGE(config.SchemePaletteFile == "UNITSNO.PAL", "with the file gone both return to the defaults");
	CHECK_MESSAGE(config.GamePaletteFile == "TEMPERAT.PAL", "as they stand in Tiberian Sun");
}

}
