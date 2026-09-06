/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// PCM streams between a producer and the mixer, the producers that fill them,
// and the feeder thread that runs the producers and recovers a lost device.
// The mixer reads a stream's ring; the producer writes it from the feeder
// thread. The ring's own counters give the producer the fill level and the
// movie player its clock.

#pragma once

#include "audio/audiodecode.h"
#include "audio/audiodefs.hh"
#include "audio/audioring.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

class AudioDeviceClass;
class AudioMixerClass;


class AudioStreamClass
{
	public:
		AudioStreamClass(void) = default;

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
		std::atomic<bool> EndOfInput{false};
		std::atomic<uint32_t> Underruns{0};

	private:
		unsigned RateValue = 0;
};


// Fills a stream's ring from the feeder thread.
class AudioStreamProducerClass
{
	public:
		virtual ~AudioStreamProducerClass(void) = default;

		// Writes what fits. Returns false once the input is exhausted, after which
		// the feeder marks the stream's end.
		virtual bool Fill(AudioStreamClass & stream) = 0;
		virtual void Close(void) = 0;

		// The smallest ring the producer can make progress in.
		virtual unsigned Min_Ring_Frames(void) const = 0;
};


// Streams an AUD, or any format miniaudio decodes, from a byte source, a chunk
// at a time. Music, speech and voice-overs use this.
class AudioFileStreamProducerClass : public AudioStreamProducerClass
{
	public:
		AudioFileStreamProducerClass(void) = default;
		~AudioFileStreamProducerClass(void);

		// Takes ownership of the source and reads its format. Returns false when the
		// data is neither an AUD nor a format miniaudio decodes.
		bool Open(std::unique_ptr<AudioByteSourceClass> source, bool loop);

		unsigned Rate(void) const { return(RateValue); }
		unsigned Channels(void) const { return(ChannelCount); }

		bool Fill(AudioStreamClass & stream) override;
		void Close(void) override;
		unsigned Min_Ring_Frames(void) const override;

	private:
		bool Fill_Aud(AudioStreamClass & stream);
		bool Fill_Other(AudioStreamClass & stream);
		bool Rewind(void);

		std::unique_ptr<AudioByteSourceClass> Source;
		bool Loop = false;
		bool IsAud = false;
		bool Ended = false;
		AUDHeaderType Header = {};
		AudChunkDecoderClass Aud;
		size_t DataStart = 0;
		size_t DataRemaining = 0;
		AudioOtherStreamDecoderClass Other;
		std::vector<uint8_t> Compressed;
		std::vector<int16_t> Pcm;
		unsigned RateValue = 0;
		unsigned ChannelCount = 0;
};


// A stream another part of the engine pushes PCM into. The movie player uses it.
class AudioPushStreamProducerClass : public AudioStreamProducerClass
{
	public:
		AudioPushStreamProducerClass(void) = default;

		bool Open(AudioStreamClass & stream);

		// Producer thread only. Converts 8-bit samples on the way in. Returns the
		// frames written, which is less than offered when the ring is full.
		unsigned Push(void const * pcm, unsigned bytes, bool eightbit);
		unsigned Free_Frames(void) const;

		// Tells the mixer the input is complete once the ring drains.
		void Mark_End(void);

		bool Fill(AudioStreamClass & stream) override;
		void Close(void) override;
		unsigned Min_Ring_Frames(void) const override { return(1); }

	private:
		AudioStreamClass * Stream = nullptr;
		std::vector<int16_t> Staging;
};


// One thread that keeps every attached stream filled and, when the device stops
// on its own, renders into the void and restarts it, so audio and the movie
// clock keep moving whatever loop the game thread is in.
class AudioFeederClass
{
	public:
		AudioFeederClass(void) = default;
		~AudioFeederClass(void);

		AudioFeederClass(AudioFeederClass const &) = delete;
		AudioFeederClass & operator=(AudioFeederClass const &) = delete;

		// Setup records what the passes work on; the device may be null when there is
		// nothing to recover. Start runs the passes on their own thread.
		bool Setup(AudioMixerClass * mixer, AudioDeviceClass * device);
		bool Start(void);
		void Stop(void);
		bool Is_Running(void) const { return(Running.load(std::memory_order_acquire)); }

		// Game thread. The producer is serviced from the next period on and stays
		// owned by the caller; Detach returns once the feeder has closed it, so the
		// caller may then delete it. A ring too small for the producer is refused.
		bool Attach(unsigned slot, AudioStreamClass * stream, AudioStreamProducerClass * producer);
		void Detach(unsigned slot);
		bool Is_Attached(unsigned slot) const;

		bool Is_Recovering(void) const { return(Recovering.load(std::memory_order_acquire)); }
		unsigned Passes(void) const { return(PassCount.load(std::memory_order_relaxed)); }

		// One pass of the feeder's work, for a test or a caller without the thread.
		void Service(void);

		// How often a lost device is restarted and, failing that, reopened. Set
		// before Start; a test shortens them.
		void Set_Recovery_Timing(unsigned retry_ms, unsigned reopen_ms);

	private:
		enum SlotState { SLOT_IDLE, SLOT_ACTIVE, SLOT_CLOSING };

		struct SlotClass {
			std::atomic<int> State{SLOT_IDLE};
			AudioStreamClass * Stream = nullptr;
			AudioStreamProducerClass * Producer = nullptr;
		};

		void Run(void);
		void Service_Streams(void);
		void Service_Device(void);

		SlotClass Slots[AUDIO_MAX_STREAMS];
		std::thread Thread;
		std::atomic<bool> Exit{false};
		std::atomic<bool> Running{false};
		std::atomic<bool> Recovering{false};
		std::atomic<unsigned> PassCount{0};
		AudioMixerClass * Mixer = nullptr;
		AudioDeviceClass * Device = nullptr;
		std::unique_ptr<float[]> Scratch;
		unsigned RetryMs = AUDIO_DEVICE_RETRY_MS;
		unsigned ReopenMs = AUDIO_DEVICE_REOPEN_MS;
		std::chrono::steady_clock::time_point LastPump;
		std::chrono::steady_clock::time_point LostSince;
		std::chrono::steady_clock::time_point LastRetry;
};
