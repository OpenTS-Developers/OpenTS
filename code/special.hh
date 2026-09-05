/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

#pragma once

/****************************************************************************
**	These are the types of dialogs that can pop up outside of the main loop,
**	an call the game in the background.
*/
enum SpecialDialogType {
	SDLG_NONE,
	SDLG_OPTIONS,
	SDLG_SURRENDER,
	SDLG_ABORT,
	SDLG_KEYBOARD,
	SDLG_SETTINGS,
	SDLG_SOUND,
	SDLG_SPECIAL,
	SDLG_QUICKLOAD,
	SDLG_LOAD,
};
