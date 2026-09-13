/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The schedule a dialog opens on. The Win32 dialogs painted a screen in full and then let
// it out through a band widening from the middle, 24 pixels a frame, over frames that fall
// due sooner as the band widens. Nothing here draws or reads a clock, so a harness runs the
// whole curve without either.

#pragma once


// How wide the band is for the pass at `elapsed` milliseconds, in the units `full` is given
// in, when the last pass drew it `shown` wide; nothing shown yet is the first pass. `scale`
// is how many of those units the frame draws to one of the original's pixels, so the reveal
// covers the same fraction of the screen and takes the same time however large the window
// is. A result at or past `full` means the screen is open.
//
// A pass opens the band by one step at most, however late it comes: the original drew every
// band in turn and only slept while it was ahead of its schedule, so a machine that cannot
// keep up draws the reveal out rather than skipping to the end of it.
float UI_Reveal_Width(float full, float scale, int elapsed, float shown);
