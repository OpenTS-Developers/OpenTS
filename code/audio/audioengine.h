/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine the game talks to: owns the device, mixer, feeder, sample cache
// and event pool, hands out streams, and answers the handle methods. Every
// member runs on the game thread.

#pragma once

#include "audio/audiodefs.hh"
#include "audio/audioevent.h"
#include "audio/audiohandle.h"
#include "audio/audiomixer.h"
#include "audio/audiosample.h"
#include "audio/audiostream.h"

#include <chrono>
#include <memory>

class AudioDeviceClass;


class AudioEngineClass
{
	public:
		AudioEngineClass(void);
		~AudioEngineClass(void);

		AudioEngineClass(AudioEngineClass const &) = delete;
		AudioEngineClass & operator=(AudioEngineClass const &) = delete;

		// Opens the output device and starts the threads. Without a device every
		// other member is a no-op and Is_Available stays false. Both are idempotent
		// and End is safe from an emergency exit.
		bool Init(void);
		void End(void);
		bool Is_Available(void) const { return(Available); }

		AudioHandle Play_Event(AudioEventTypeClass const & type, AudioGroupType group, float volume, float pan, bool noattack = false);

		// An AUD in memory; its header supplies the size. The data is decoded into
		// the cache, so the caller may free it once the call returns.
		AudioHandle Play_Sample(void const * aud, AudioGroupType group, float volume, int priority = 255);

		// Plays a file through the game's file layer without loading it whole.
		AudioHandle Open_Stream(char const * filename, AudioGroupType group, float volume, bool loop);

		// Stops a stream and returns once its file is closed, so the archive it
		// came from may be released afterwards.
		void Stop_Stream(AudioHandle handle);

		// Screens that own their sample data address it by pointer.
		bool Is_Sample_Playing(void const * aud) const;
		void Stop_Sample_Playing(void const * aud);
		void Set_Sample_Volume(void const * aud, float volume);
		void Release_Sample(void const * aud);

		void Set_Group_Gain(AudioGroupType group, float gain);
		float Group_Gain(AudioGroupType group) const;
		void Set_Duck(AudioGroupType group, float level, int ms);
		void Set_Master_Gain(float gain, int ms = 0);
		float Master_Gain(void) const { return(MasterGain); }
		void Set_Channels(int channels);

		// Pauses the whole mix in place and resumes it; safe from any thread.
		void Focus_Loss(void);
		void Focus_Restore(void);

		void Stop_All(void);

		// Stops every stream and returns once their files are closed, so the
		// archives they read from may be freed afterwards.
		void Stop_All_Streams(void);

		// Once per game tick.
		void Sound_Callback(void);

		AudioEventPoolClass & Events(void) { return(Pool); }
		AudioEventPoolClass const & Events(void) const { return(Pool); }
		AudioSampleCacheClass & Samples(void) { return(Cache); }
		AudioMixerClass & Mixer_Ref(void) { return(Mixer); }
		AudioFeederClass & Feeder_Ref(void) { return(Feeder); }
		unsigned Now_Ms(void) const;

		// A stream slot for a producer the caller drives itself, such as the
		// movie sink. The caller attaches to the feeder, starts the stream
		// event, and gives the slot back once the event is finished.
		int Acquire_Stream_Slot(AudioStreamClass ** stream);
		void Release_Stream_Slot(int slot);

		// Frames of output the device holds beyond what the mixer has rendered.
		unsigned Device_Latency_Frames(void) const;

		// Services the pool until the event has let go of its voice, or ms pass.
		bool Wait_Finished(AudioHandle handle, int ms);

	private:
		struct StreamSlotClass {
			AudioStreamClass Stream;
			std::unique_ptr<AudioFileStreamProducerClass> Producer;
			AudioHandle Handle;
			bool InUse;
			bool External;

			StreamSlotClass(void) : InUse(false), External(false) {}
		};

		static int Random_Proc(int low, int high, void * context);
		int Free_Stream_Slot(void) const;
		void Reap_Streams(void);
		void Close_Stream(int slot);
		void Push_Level(AudioCommandType type, AudioGroupType group, float level, float ramp);

		std::unique_ptr<AudioDeviceClass> Device;
		AudioMixerClass Mixer;
		AudioFeederClass Feeder;
		AudioSampleCacheClass Cache;
		AudioEventPoolClass Pool;
		std::unique_ptr<AudioAssetReaderClass> Reader;
		std::unique_ptr<AudioClipProviderClass> Provider;
		StreamSlotClass Streams[AUDIO_MAX_STREAMS];
		float GroupGains[AUDIO_GROUP_COUNT];
		float MasterGain;
		std::chrono::steady_clock::time_point Epoch;
		unsigned LastDropped;
		unsigned LastDropReport;
		bool Available;
};

extern AudioEngineClass AudioEngine;
