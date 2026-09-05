/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The mixer. A fixed pool of voices reads clips and streams, resamples them
// to the output rate, applies the voice, group and master levels and the pan,
// and sums them. Render is a pure function of the commands received and the
// frames asked for, so it runs the same on a device thread, on the feeder
// thread while a device is being recovered, and in a test.

#pragma once

#include "audio/audiodefs.hh"
#include "audio/audiovoice.h"

#include <atomic>
#include <memory>


class AudioMixerClass
{
	public:
		AudioMixerClass(void);
		~AudioMixerClass(void);

		AudioMixerClass(AudioMixerClass const &) = delete;
		AudioMixerClass & operator=(AudioMixerClass const &) = delete;

		// Allocates every buffer the render thread will use. Game thread only.
		bool Init(unsigned rate, unsigned channels);
		void Shutdown(void);
		bool Is_Ready(void) const { return(Ready); }

		unsigned Rate(void) const { return(MixRate); }
		unsigned Channels(void) const { return(MixChannels); }

		// Game thread. A voice must be ALLOCATED before its play command is pushed;
		// the game marks it so, fills the sequence, then pushes. Push fails when the
		// ring is full, and the caller must then treat the play as refused.
		bool Allocate_Voice(unsigned slot);
		void Free_Voice(unsigned slot);
		AudioVoiceState Voice_State(unsigned slot) const;
		bool Push(AudioCommand const & command);
		unsigned Dropped_Commands(void) const { return(Dropped.load(std::memory_order_relaxed)); }

		// Any thread. Ramps everything to silence over a few milliseconds and then
		// stops advancing voices, so positions are kept for the resume.
		void Set_Pause_All(bool paused);

		// Render thread. Whoever holds the render token renders; a second caller
		// writes silence for its block instead, so two threads can never mix at once.
		void Render(float * output, unsigned frames);
		static void Render_Callback(void * context, float * output, unsigned frames);

		// Only for a test that wants to stand in for a second renderer.
		bool Acquire_Render_Token(void);
		void Release_Render_Token(void);

	private:
		struct StateClass;
		friend struct StateClass;

		std::unique_ptr<StateClass> State;
		std::atomic<int> RenderOwner;
		std::atomic<bool> PauseAll;
		std::atomic<unsigned> Dropped;
		unsigned MixRate;
		unsigned MixChannels;
		bool Ready;
};
