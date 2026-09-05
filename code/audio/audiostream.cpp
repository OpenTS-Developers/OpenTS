/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "audio/audiostream.h"


AudioStreamClass::AudioStreamClass(void) :
	EndOfInput(false),
	Underruns(0),
	RateValue(0)
{
}


bool AudioStreamClass::Init(unsigned frames, unsigned channels, unsigned rate)
{
	if (rate == 0 || !Ring.Init(frames, channels)) {
		return(false);
	}
	RateValue = rate;
	Reset();
	return(true);
}


void AudioStreamClass::Reset(void)
{
	Ring.Reset();
	EndOfInput.store(false, std::memory_order_release);
	Underruns.store(0, std::memory_order_release);
}
