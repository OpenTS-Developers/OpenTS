/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Loudness curve and ramped gain levels for the mixer.

#pragma once


// Maps a 0..1 level to a linear gain along the curve the DirectSound driver applied
// to its 0..255 volumes, so option and INI values keep their loudness. The power
// distributes over products, so master, group and voice levels may be shaped
// separately or once as a product.
float Audio_Perceptual_Gain(float level);
float Audio_Perceptual_Level(float gain);


// A gain that moves toward its target over time. The base level is what a caller
// sets; the adjust level is a temporary multiplier (a duck, a fade) kept apart so
// it can be undone without knowing the base. Both ramp linearly.
class AudioLevelClass
{
	public:
		AudioLevelClass(void);

		void Set_Level(float target, float seconds);
		void Adjust_Level(float multiplier, float seconds);
		void Restore_Level(float seconds);

		// Moves both ramps forward by the given frames and returns the gain in effect
		// at the end of them.
		float Advance(unsigned frames, unsigned rate);

		float Current(void) const { return(Base * Adjust); }
		float Target(void) const { return(BaseTarget * AdjustTarget); }
		bool Is_Settled(void) const { return(BaseRemaining <= 0.0f && AdjustRemaining <= 0.0f); }

	private:
		static void Step(float & value, float target, float & remaining, float seconds);

		float Base;
		float BaseTarget;
		float BaseRemaining;
		float Adjust;
		float AdjustTarget;
		float AdjustRemaining;
};
