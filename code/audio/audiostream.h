/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// A PCM stream between a producer and the mixer. The producer, on the feeder
// or the game thread, writes the ring; the mixer reads it. The ring's own
// counters give the producer the fill level and the movie player its clock.

#pragma once

#include "audio/audioring.h"

#include <atomic>
#include <cstdint>


class AudioStreamClass
{
	public:
		AudioStreamClass(void);

		AudioStreamClass(AudioStreamClass const &) = delete;
		AudioStreamClass & operator=(AudioStreamClass const &) = delete;

		// Allocates the ring; only while no voice reads the stream.
		bool Init(unsigned frames, unsigned channels, unsigned rate);

		// Empties the ring and clears the flags; only while no voice reads the stream.
		void Reset(void);

		unsigned Rate(void) const { return(RateValue); }
		unsigned Channels(void) const { return(Ring.Channels()); }

		// Frames the mixer has taken from the ring. Silence played during an
		// underrun is not counted, so this stays a position in the stream itself.
		uint32_t Frames_Consumed(void) const { return(Ring.Frames_Consumed()); }
		uint32_t Frames_Pushed(void) const { return(Ring.Frames_Pushed()); }

		PcmRingClass Ring;
		std::atomic<bool> EndOfInput;
		std::atomic<uint32_t> Underruns;

	private:
		unsigned RateValue;
};
