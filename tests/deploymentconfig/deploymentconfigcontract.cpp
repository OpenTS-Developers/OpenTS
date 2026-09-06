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

#include "_deploymentconfig.h"
#include "deploymentconfig.h"
#include "rawfile.h"

namespace {

int Failures = 0;

std::string Root;
char OriginalDirectory[MAX_PATH];

char const * const DefaultList = "INI,MIX,Maps";


void Check(bool condition, char const * what)
{
	std::printf("%-62s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


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


void Test_Defaults(void)
{
	DeploymentConfigClass config;

	Check(config.SearchPaths == DefaultList, "with no file the INI, MIX and Maps folders are searched");
	Check(DeploymentConfig.SearchPaths == DefaultList, "the game's own copy starts from the same defaults");

	Check(!config.Read_File(""), "with no file there is nothing to read");
	Check(config.SearchPaths == DefaultList, "and the default stands");
}


void Test_Search_Paths(void)
{
	DeploymentConfigClass config;

	Write_File(Root + "\\OPENTS.INI", "[Paths]\nSearchPaths=Data,More\n");

	Check(config.Read_File(""), "a file beside the game is read");
	Check(config.SearchPaths == "Data,More", "the folders it names replace the default");

	/*
	 * The reader passes over an entry with nothing after the equals sign, so a written key
	 * cannot empty the list; naming only the game's own directory is how a deployment asks
	 * for no other folder.
	 */
	Write_File(Root + "\\OPENTS.INI", "[Paths]\nSearchPaths=\n");

	Check(config.Read_File(""), "a file with an empty list is still read");
	Check(config.SearchPaths == DefaultList, "and the empty list leaves the default in force");

	Remove_File(Root + "\\OPENTS.INI");
}


void Test_Where_The_File_Is_Looked_For(void)
{
	DeploymentConfigClass config;

	Write_File(Root + "\\MIX\\OPENTS.INI", "[Paths]\nSearchPaths=FromMix\n");
	config.Read_File("");
	Check(config.SearchPaths == "FromMix", "a file in the MIX folder is found");

	Write_File(Root + "\\INI\\OPENTS.INI", "[Paths]\nSearchPaths=FromIni\n");
	config.Read_File("");
	Check(config.SearchPaths == "FromIni", "a file in the INI folder is read ahead of one in MIX");

	Write_File(Root + "\\OPENTS.INI", "[Paths]\nSearchPaths=Beside\n");
	config.Read_File("");
	Check(config.SearchPaths == "Beside", "a file beside the game is read ahead of both");

	Remove_File(Root + "\\OPENTS.INI");
	Remove_File(Root + "\\INI\\OPENTS.INI");
	Remove_File(Root + "\\MIX\\OPENTS.INI");
}


void Test_The_Directory_Named(void)
{
	DeploymentConfigClass config;
	std::string const data = Root + "\\Data\\";

	Write_File(data + "OPENTS.INI", "[Paths]\nSearchPaths=Sorted\n");

	Check(config.Read_File(data.c_str()), "the file is read from the directory named");
	Check(config.SearchPaths == "Sorted", "and what it names is taken from there");

	Remove_File(data + "OPENTS.INI");

	Check(!config.Read_File(data.c_str()), "a file beside the game is not read for a directory named");
}


void Test_A_Read_Starts_Over(void)
{
	DeploymentConfigClass config;

	Write_File(Root + "\\OPENTS.INI", "[Paths]\nSearchPaths=Data\n");
	config.Read_File("");
	Remove_File(Root + "\\OPENTS.INI");

	Check(!config.Read_File(""), "with the file gone there is nothing to read");
	Check(config.SearchPaths == DefaultList, "and every setting returns to its default");
}


void Test_Carry_Scenario_File(void)
{
	DeploymentConfigClass config;

	Check(!config.CarryScenarioFile, "with no file a save carries no scenario file");

	Write_File(Root + "\\OPENTS.INI", "[Paths]\nSearchPaths=Data\n");
	config.Read_File("");
	Check(!config.CarryScenarioFile, "a file that does not ask for it leaves it off");

	Write_File(Root + "\\OPENTS.INI", "[Saves]\nCarryScenarioFile=yes\n");
	config.Read_File("");
	Check(config.CarryScenarioFile, "a file asking for it turns it on");

	Write_File(Root + "\\OPENTS.INI", "[Saves]\nCarryScenarioFile=no\n");
	config.Read_File("");
	Check(!config.CarryScenarioFile, "a file refusing it turns it off again");

	Write_File(Root + "\\OPENTS.INI", "[Saves]\nCarryScenarioFile=yes\n");
	config.Read_File("");
	Remove_File(Root + "\\OPENTS.INI");
	config.Read_File("");
	Check(!config.CarryScenarioFile, "with the file gone it is off");
}

}


int main(void)
{
	GetCurrentDirectory(sizeof(OriginalDirectory), OriginalDirectory);

	if (!Make_Root()) {
		std::printf("could not create the working directory\n");
		return(1);
	}

	std::printf("Working in %s\n\n", Root.c_str());

	Test_Defaults();
	Test_Search_Paths();
	Test_Where_The_File_Is_Looked_For();
	Test_The_Directory_Named();
	Test_A_Read_Starts_Over();
	Test_Carry_Scenario_File();

	Remove_Root();

	std::printf("\n%s\n", Failures == 0 ? "All checks passed." : "There were failures.");
	return(Failures == 0 ? 0 : 1);
}
