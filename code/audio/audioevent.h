/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Sound events: the unit the game controls. An event plays one sound type's
// sequence of clips on a mixer voice, or a stream, or one raw clip. The pool
// owns every event, enforces each type's Limit and the effects budget, steals
// voices by priority, and answers the handles. Everything here runs on the
// game thread.

#pragma once

#include "audio/audiodefs.hh"
#include "audio/audiohandle.h"
#include "audio/audiovoice.h"

#include <cstdint>

class AudioMixerClass;
class AudioSampleClass;
class AudioStreamClass;


// A sound type as the INI layer fills it in. Plain data apart from the two
// runtime counters.
class AudioEventTypeClass
{
	public:
		AudioEventTypeClass(void);

		char Name[32];
		char Sounds[AUDIO_MAX_SOUNDS][32];
		unsigned SoundCount;

		int Priority;             // 0..255
		float Volume;             // 0..1
		float MinVolume;          // 0..1
		int Range;                // cells
		int Limit;                // simultaneous events of this type; 0 = unlimited
		int Loop;                 // body cycles with LOOP; 0 = until ended
		int DelayMin;             // milliseconds
		int DelayMax;
		int FShiftMin;            // percent of pitch
		int FShiftMax;
		int VShiftMin;            // percent of volume
		int VShiftMax;
		int AttackCount;
		int DecayCount;
		unsigned Type;            // SoundTypeFlag bits
		unsigned Control;         // SoundControlFlag bits
		AudioGroupType Group;

		mutable int LiveCount;
		mutable unsigned SequentialIndex;

		bool Never_Ends(void) const { return((Control & SOUND_CONTROL_LOOP) != 0 && Loop == 0); }
		bool Has_Delay(void) const { return(DelayMax > 0); }
		unsigned Body_Start(void) const;
		unsigned Body_Count(void) const;
};


// Hands the pool pinned clips for a type's sounds and takes them back when the
// event ends. The engine's provider is the sample cache.
class AudioClipProviderClass
{
	public:
		virtual ~AudioClipProviderClass(void) = default;

		virtual AudioSampleClass * Acquire(AudioEventTypeClass const & type, unsigned sound) = 0;
		virtual void Release(AudioSampleClass * clip) = 0;
};


// Inclusive random pick. The engine installs the non-critical generator so
// audio never touches the simulation's.
typedef int (*AudioRandomProc)(int low, int high, void * context);


enum AudioEventState {
	AUDIO_EVENT_FREE,
	AUDIO_EVENT_PENDING,      // waiting for a delay, or queued for a voice
	AUDIO_EVENT_PLAYING,      // a voice is running its sequence
	AUDIO_EVENT_GAP,          // between cycles of a delayed loop
	AUDIO_EVENT_DONE
};


class AudioEventPoolClass
{
	public:
		AudioEventPoolClass(void);
		~AudioEventPoolClass(void);

		AudioEventPoolClass(AudioEventPoolClass const &) = delete;
		AudioEventPoolClass & operator=(AudioEventPoolClass const &) = delete;

		bool Init(AudioMixerClass * mixer, AudioClipProviderClass * clips);
		void Shutdown(void);

		void Set_Random(AudioRandomProc proc, void * context);

		// Voices the SFX and SYSTEM groups may hold at once; streams are outside it.
		void Set_Budget(int channels);
		int Budget(void) const { return(BudgetValue); }

		// Starting returns a null handle when the type has no playable sound, the
		// type's Limit refuses the request, or no voice can be taken. A refused
		// request with Control=QUEUE waits instead.
		AudioHandle Start(AudioEventTypeClass const & type, AudioGroupType group, float volume, float pan, bool noattack = false);

		// A raw clip the caller already acquired from the cache; the pool releases it.
		AudioHandle Start_Sample(AudioSampleClass * clip, AudioGroupType group, float volume, int priority, void const * tag);

		// A stream another part of the engine keeps filled. Never stolen.
		AudioHandle Start_Stream(AudioStreamClass * stream, AudioGroupType group, float volume, float pan);

		// Once per game tick, with a millisecond clock. Reaps finished voices,
		// starts delayed cycles and queued events, and applies the budget.
		void Service(unsigned now);

		bool Is_Valid(AudioHandle handle) const;
		bool Is_Playing(AudioHandle handle) const;
		AudioEventTypeClass const * Type(AudioHandle handle) const;
		AudioEventState State(AudioHandle handle) const;

		void Retarget(AudioHandle handle, float volume, float pan);
		void Set_Volume(AudioHandle handle, float volume);
		void Set_Pan(AudioHandle handle, float pan);
		void Stop(AudioHandle handle);
		void End(AudioHandle handle);
		void End_Looping(AudioHandle handle);
		void Fade(AudioHandle handle, int ms);
		void Pause(AudioHandle handle);
		void Resume(AudioHandle handle);

		// Screens that own their sample data address it by pointer.
		bool Is_Tag_Playing(void const * tag) const;
		void Stop_Tag(void const * tag);

		void Stop_All(void);
		void Stop_Group(AudioGroupType group, int fadems);

		unsigned Live_Count(void) const;
		unsigned Live_Count(AudioEventTypeClass const & type) const;
		unsigned Effects_In_Use(void) const;

		// The sequence an event's voice is running, for tests.
		bool Get_Sequence(AudioHandle handle, AudioSequenceClass & sequence) const;

	private:
		struct EventClass;

		EventClass * Lookup(AudioHandle handle);
		EventClass const * Lookup(AudioHandle handle) const;
		EventClass * Allocate(void);
		int Random(int low, int high);
		float Current_Level(EventClass const & event) const;
		void Push_Level(EventClass & event, float ramp);
		void Push_Pan(EventClass & event, float ramp);
		bool Enforce_Limit(EventClass & event);
		bool Acquire_Voice(EventClass & event);
		bool Issue(EventClass & event, bool attack, bool body, bool decay);
		bool Try_Start(EventClass & event);
		void Release_Voice(EventClass & event);
		void Release_Pins(EventClass & event);
		void Kill(EventClass & event, float fade);
		void Finish(EventClass & event);
		AudioSampleClass * Pin(EventClass & event, unsigned sound);

		EventClass * Events;
		uint32_t VoiceGenerations[AUDIO_MAX_VOICES];
		AudioMixerClass * Mixer;
		AudioClipProviderClass * Clips;
		AudioRandomProc RandomProc;
		void * RandomContext;
		unsigned Seed;
		int BudgetValue;
		unsigned LastNow;
		unsigned StartOrder;
		bool Ready;
};
