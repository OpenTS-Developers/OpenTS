/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once


enum UIInputOwner
{
	UI_INPUT_NONE,
	UI_INPUT_GAME,
	UI_INPUT_RML,
	UI_INPUT_IMGUI,
	UI_INPUT_SUPPRESSED
};


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
