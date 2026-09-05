/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Holds the loudness curve to the values the DirectSound driver produced for its
// 0..255 volumes, and checks that ramped levels move linearly, settle exactly on
// their targets, and keep the base and adjust ramps apart. Needs no game data.

#include "audio/audiolevel.h"

#include <cmath>
#include <cstdio>

namespace {

int Failures = 0;
int Checked = 0;


void Check(bool condition, char const * what)
{
	Checked++;
	if (!condition) {
		Failures++;
		std::printf("FAIL: %s\n", what);
	}
}


bool Near(float value, float expect, float tolerance)
{
	return(std::fabs(value - expect) <= tolerance);
}


void Test_Curve(void)
{
	// (v / 255) ^ (5 / 3) for the volumes the old driver was fed.
	Check(Near(Audio_Perceptual_Gain(255.0f / 255.0f), 1.0f, 0.0001f), "255 is unity");
	Check(Near(Audio_Perceptual_Gain(128.0f / 255.0f), 0.317f, 0.001f), "128 is 0.317");
	Check(Near(Audio_Perceptual_Gain(64.0f / 255.0f), 0.100f, 0.001f), "64 is 0.100");
	Check(Near(Audio_Perceptual_Gain(0.0f), 0.0f, 0.0f), "0 is silence");
	Check(Audio_Perceptual_Gain(-0.5f) == 0.0f, "negative clamps to silence");
	Check(Audio_Perceptual_Gain(2.0f) == 1.0f, "above one clamps to unity");

	// The curve is a power, so shaping a product equals the product of shaped factors.
	float product = Audio_Perceptual_Gain(0.7f * 0.5f);
	float factors = Audio_Perceptual_Gain(0.7f) * Audio_Perceptual_Gain(0.5f);
	Check(Near(product, factors, 0.0001f), "curve distributes over products");

	for (int v = 0; v <= 255; v += 5) {
		float level = (float)v / 255.0f;
		Check(Near(Audio_Perceptual_Level(Audio_Perceptual_Gain(level)), level, 0.0005f), "inverse round-trips");
	}

	bool monotonic = true;
	float last = -1.0f;
	for (int v = 0; v <= 255; v++) {
		float gain = Audio_Perceptual_Gain((float)v / 255.0f);
		if (gain < last) monotonic = false;
		last = gain;
	}
	Check(monotonic, "curve is monotonic");
}


void Test_Set_Ramp(void)
{
	AudioLevelClass level;
	Check(level.Current() == 1.0f, "new level is unity");
	Check(level.Is_Settled(), "new level is settled");

	level.Set_Level(0.25f, 0.0f);
	Check(level.Current() == 0.25f, "zero-second set snaps");
	Check(level.Is_Settled(), "snap is settled");

	// One second ramp from 0.25 to 1.0, advanced in 64-frame blocks at 48 kHz.
	level.Set_Level(1.0f, 1.0f);
	Check(!level.Is_Settled(), "ramp is not settled while running");
	float previous = level.Current();
	bool increasing = true;
	unsigned blocks = 0;
	while (!level.Is_Settled() && blocks < 100000) {
		float now = level.Advance(64, 48000);
		if (now < previous) increasing = false;
		previous = now;
		blocks++;
	}
	Check(increasing, "set ramp rises monotonically");
	Check(level.Current() == 1.0f, "set ramp lands exactly on its target");
	Check(Near((float)blocks * 64.0f / 48000.0f, 1.0f, 0.002f), "set ramp takes the requested time");

	// Halfway check: a fresh ramp reaches the midpoint after half the time.
	level.Set_Level(0.0f, 2.0f);
	level.Advance(48000, 48000);
	Check(Near(level.Current(), 0.5f, 0.002f), "linear ramp is half done at half time");

	// Retargeting mid-ramp starts from the current value.
	level.Set_Level(1.0f, 1.0f);
	Check(Near(level.Current(), 0.5f, 0.002f), "retarget keeps the current value");
	level.Advance(48000, 48000);
	Check(level.Current() == 1.0f, "retargeted ramp lands on the new target");
}


void Test_Adjust_Ramp(void)
{
	AudioLevelClass level;
	level.Set_Level(0.8f, 0.0f);

	level.Adjust_Level(0.5f, 0.25f);
	Check(Near(level.Current(), 0.8f, 0.0001f), "adjust starts from unity");
	level.Advance(6000, 48000);
	Check(Near(level.Current(), 0.8f * 0.75f, 0.002f), "adjust ramps toward the multiplier");
	level.Advance(6000, 48000);
	Check(Near(level.Current(), 0.4f, 0.0005f), "adjust reaches the multiplier");
	Check(level.Is_Settled(), "adjust settles");

	// The base can change underneath the adjust without disturbing it.
	level.Set_Level(0.4f, 0.0f);
	Check(Near(level.Current(), 0.2f, 0.0005f), "base and adjust multiply");

	level.Restore_Level(0.0f);
	Check(Near(level.Current(), 0.4f, 0.0005f), "restore undoes the adjust");

	level.Restore_Level(1.0f);
	Check(level.Is_Settled(), "restore to the current multiplier is a no-op");
	level.Set_Level(0.4f, 1.0f);
	Check(level.Is_Settled(), "set to the current level is a no-op");
}

} // namespace


int main(void)
{
	Test_Curve();
	Test_Set_Ramp();
	Test_Adjust_Ramp();

	std::printf("audiolevel: %d checks, %d failures\n", Checked, Failures);
	return(Failures == 0 ? 0 : 1);
}
