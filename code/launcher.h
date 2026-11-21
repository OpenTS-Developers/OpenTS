/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#pragma once

namespace Launch
{
	bool Is_Game_Requested(void);
	bool Is_Active(void);
	bool Is_From_Spawn_File(void);
	int Session_Identity(void);

	// Whether this process skips the menus, for a launched game or a headless task.
	bool Replaces_Menu(void);

	// Called while the command line is parsed.
	void Request_Spawn_File(void);

	bool Prepare(bool & gameloaded);
	void Announce_Master(void);
}
