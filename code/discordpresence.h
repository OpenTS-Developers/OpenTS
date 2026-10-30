/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once


// Shows the menus. A repeat while the menus are already shown keeps the elapsed time.
void Discord_Presence_Show_Menu(void);

// Shows the scenario that just started or was just loaded. Each call starts the clock again.
void Discord_Presence_Show_Match(void);

// Closes the local Discord connection. Safe when presence never started.
void Discord_Presence_Shutdown(void);
