/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "audio/audiolevel.h"

#include <cmath>


// The DirectSound driver converted volume v in 0..255 to 3333.33 * log10(v / 255)
// hundredths of a decibel, which is the linear gain (v / 255) ^ (5 / 3).
float const PERCEPTUAL_EXPONENT = 5.0f / 3.0f;


float Audio_Perceptual_Gain(float level)
{
	if (level <= 0.0f) {
		return(0.0f);
	}
	if (level >= 1.0f) {
		return(1.0f);
	}
	return(powf(level, PERCEPTUAL_EXPONENT));
}


float Audio_Perceptual_Level(float gain)
{
	if (gain <= 0.0f) {
		return(0.0f);
	}
	if (gain >= 1.0f) {
		return(1.0f);
	}
	return(powf(gain, 1.0f / PERCEPTUAL_EXPONENT));
}


AudioLevelClass::AudioLevelClass(void) :
	Base(1.0f),
	BaseTarget(1.0f),
	BaseRemaining(0.0f),
	Adjust(1.0f),
	AdjustTarget(1.0f),
	AdjustRemaining(0.0f)
{
}


void AudioLevelClass::Set_Level(float target, float seconds)
{
	BaseTarget = target;
	if (seconds <= 0.0f || Base == target) {
		Base = target;
		BaseRemaining = 0.0f;
	} else {
		BaseRemaining = seconds;
	}
}


void AudioLevelClass::Adjust_Level(float multiplier, float seconds)
{
	AdjustTarget = multiplier;
	if (seconds <= 0.0f || Adjust == multiplier) {
		Adjust = multiplier;
		AdjustRemaining = 0.0f;
	} else {
		AdjustRemaining = seconds;
	}
}


void AudioLevelClass::Restore_Level(float seconds)
{
	Adjust_Level(1.0f, seconds);
}


float AudioLevelClass::Advance(unsigned frames, unsigned rate)
{
	float seconds = (rate == 0) ? 0.0f : (float)frames / (float)rate;
	Step(Base, BaseTarget, BaseRemaining, seconds);
	Step(Adjust, AdjustTarget, AdjustRemaining, seconds);
	return(Base * Adjust);
}


void AudioLevelClass::Step(float & value, float target, float & remaining, float seconds)
{
	if (remaining <= 0.0f) {
		value = target;
		return;
	}
	if (seconds >= remaining) {
		value = target;
		remaining = 0.0f;
		return;
	}
	value += (target - value) * (seconds / remaining);
	remaining -= seconds;
}
