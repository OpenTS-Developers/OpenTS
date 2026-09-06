/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "scenfile.h"

#include <cstring>
#include <utility>


bool ScenarioFileClass::Matches(char const * name) const
{
	return(name != NULL && Is_Present() && _stricmp(FileName.c_str(), name) == 0);
}


void ScenarioFileClass::Assign(char const * name, std::vector<char> && bytes)
{
	if (name == NULL || bytes.empty()) {
		Clear();
		return;
	}

	FileName = name;
	Bytes = std::move(bytes);
}


void ScenarioFileClass::Clear(void)
{
	FileName.clear();
	Bytes.clear();
	Bytes.shrink_to_fit();
}
