/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The bgfx views the presenter and the UI overlays draw into. Views render in ascending
// order, so the magnify pass must carry the lower id for the present pass to sample its
// output from this frame, and the overlays must follow the frame they sit on.

#pragma once


// Magnifies the frame when the pixel art filter needs an intermediate target.
const unsigned short VIEW_PRESCALE = 0;

// Draws the frame onto the window.
const unsigned short VIEW_PRESENT = 1;

// RmlUi documents.
const unsigned short VIEW_UI = 2;

// Developer overlays.
const unsigned short VIEW_DEV = 3;
