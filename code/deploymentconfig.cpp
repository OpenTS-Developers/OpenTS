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
