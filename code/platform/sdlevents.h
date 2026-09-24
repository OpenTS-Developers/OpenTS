/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "platform/windowevent.hh"

#include <vector>

union SDL_Event;


// Appends the window events an SDL event stands for, which is none for an event the game
// does not use. Positions are scaled by the window's pixel density into client pixels. Set
// altgr while the Right Alt held was pressed as AltGr.
void Window_Events_From_SDL(SDL_Event const & sdlevent, float pixeldensity, bool altgr, std::vector<WindowEvent> & events);
