/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the version screen: the lines it shows and the entry the legacy dialog's
// wrapper calls. The presenter and view live in uiversion.cpp so that the test harness can
// drive them without the engine.

#include "ui/screens/version/uiversion.h"

#include "addon.h"
#include "data.h"
#include "getcpu.h"
#include "globals.h"
#include "language/language.h"
#include "ui/rml/rmlview.h"
#include "ui/uishell.h"
#include "version.h"

#include "opents_build.h"

#include <cstdio>
#include <utility>


void UI_Version_Lines(std::vector<std::string> & lines)
{
	char buffer[256];

	lines.clear();

	std::string title = Fetch_String(TXT_SHORT_TITLE);
	if (Addon_Installed(ADDON_FIRESTORM)) {
		title += ": ";
		title += Get_Addon_Title(ADDON_FIRESTORM);
	}
	lines.push_back(title);

	std::snprintf(buffer, sizeof(buffer), "Version %s", Version_Name());
	lines.push_back(buffer);

	std::snprintf(buffer, sizeof(buffer), "Internal Version %s", VerNum.Version_Name());
	lines.push_back(buffer);

#ifdef _DEBUG
	std::snprintf(buffer, sizeof(buffer), "Debug Build: %s - %s", OPENTS_BUILD_DESCRIPTION, OPENTS_COMMIT_DATE);
#else
	std::snprintf(buffer, sizeof(buffer), "Release Build: %s - %s", OPENTS_BUILD_DESCRIPTION, OPENTS_COMMIT_DATE);
#endif
	lines.push_back(buffer);

	int cpu_type = 5;
	char vendor[32];
	vendor[0] = '\0';
	Get_CPU_Type(cpu_type, vendor, sizeof(vendor) - 1);
	std::snprintf(buffer, sizeof(buffer), "CPU vendor: %s", vendor);
	lines.push_back(buffer);

	Get_Language_Version(buffer);
	lines.push_back(buffer);
}


bool UI_Version_Dialog(void)
{
	std::vector<std::string> lines;
	UI_Version_Lines(lines);

	UIVersionPresenterClass presenter(std::move(lines));
	std::unique_ptr<UIRmlViewClass> view = UI_Version_View(presenter);

	return(UI_Run_Modal(*view) != UI_RESULT_FAILED_TO_OPEN);
}
