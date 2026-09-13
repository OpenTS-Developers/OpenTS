/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Definitions the shell's input routing and the toolkit adapters share.

#pragma once


// Who a press belongs to for as long as it is held. SUPPRESSED is input the shell once
// owned or has cancelled: its release is swallowed rather than handed to the game.
enum UIInputOwner
{
	UI_INPUT_NONE,
	UI_INPUT_GAME,
	UI_INPUT_RML,
	UI_INPUT_IMGUI,
	UI_INPUT_SUPPRESSED
};


// The pointer shape a document asks for.
enum UICursor
{
	UI_CURSOR_ARROW,
	UI_CURSOR_TEXT,
	UI_CURSOR_HAND,
	UI_CURSOR_RESIZE_NS,
	UI_CURSOR_RESIZE_EW,
	UI_CURSOR_RESIZE_NESW,
	UI_CURSOR_RESIZE_NWSE,
	UI_CURSOR_MOVE,
	UI_CURSOR_UNAVAILABLE
};
