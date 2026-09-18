/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uireveal.h"


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

	float step = (float)UI_REVEAL_STEP * scale;
	int next = shown <= 0.0f ? 0 : (int)(shown / step + 0.5f);

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
