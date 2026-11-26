/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#pragma once

#include "launch/gamelaunch.h"

namespace Launch
{
	bool Is_Game_Requested(void);
	bool Is_Active(void);
	bool Is_From_Spawn_File(void);
	int Session_Identity(void);

	// Whether this process skips the menus, for a launched game or a headless task.
	bool Replaces_Menu(void);

	// Called while the command line is parsed.
	void Request_File(char const * path);
	void Request_Spawn_File(void);
	void Note_Switch(char const * name);

	// Call once the command line is parsed.
	bool Read_Request(void);

	bool Prepare(bool & gameloaded);
	void Announce_Master(void);

	// Called as each scenario of a launched game starts and ends.
	void Scenario_Started(void);
	void Scenario_Joined(void);
	void Scenario_Ended(ScenarioEnd end);

	char const * Refusal(void);
	bool Has_Result_File(void);
	void Write_Result(void);
}
