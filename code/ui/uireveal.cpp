/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uireveal.h"


// 12 pixels a side a frame, so 24 across; the frame period starts at 40 milliseconds and
// tightens by the band's share of the half width as it opens.
static const int UI_REVEAL_STEP = 24;
static const int UI_REVEAL_PERIOD = 40;
static const int UI_REVEAL_TIGHTEN = 240;


float UI_Reveal_Width(float full, float scale, int elapsed, float shown)
{
	if (scale <= 0.0f) {
		scale = 1.0f;
	}

	int half = (int)(full / scale / 2.0f);
	if (full <= 0.0f || half <= 0 || elapsed < 0) {
		return(full);
	}

	// The original draws a slice before it sleeps at all, so a dialog is one step open the
	// moment it appears rather than showing nothing, and each pass after that adds one more.
	float step = (float)UI_REVEAL_STEP * scale;
	int next = shown <= 0.0f ? 0 : (int)(shown / step + 0.5f);

	// The period shrinks faster than the frame count grows, so a later frame's moment can
	// fall before an earlier one's. The original drew its frames in order and only slept
	// while it was ahead, so a moment already gone never pulls the reveal backwards: what
	// governs is the latest moment reached so far.
	int frame = 0;
	int deadline = 0;

	while (frame < next && UI_REVEAL_STEP * frame < half * 2) {
		int period = UI_REVEAL_PERIOD - (UI_REVEAL_TIGHTEN * frame) / half;
		if (period < 1) {
			period = 1;
		}

		int due = (frame + 1) * period;
		if (due > deadline) {
			deadline = due;
		}
		if (deadline > elapsed) {
			break;
		}
		frame++;
	}

	float width = (float)(UI_REVEAL_STEP * (frame + 1)) * scale;
	return(width >= full ? full : width);
}
