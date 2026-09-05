/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "audio/audiomixer.h"

#include "audio/audiolevel.h"
#include "audio/audioring.h"
#include "audio/audiosample.h"
#include "audio/audiostream.h"

#include "miniaudio.h"

#include <cmath>
#include <cstring>

namespace {

unsigned const BLOCK = AUDIO_RENDER_BLOCK_FRAMES;

// Enough source frames for one output block at the highest rate ratio, plus the
// resampler's own lead.
unsigned const SCRATCH_FRAMES = (unsigned)(BLOCK * AUDIO_MAX_RATE_RATIO) + 32;

// After a source ends, this many blocks flush the resampler's tail.
unsigned const DRAIN_BLOCKS = 2;


struct VoiceClass {
	std::atomic<AudioVoiceState> State;
	uint32_t Generation;
	AudioGroupType Group;
	bool IsStream;
	AudioStreamClass * Stream;
	AudioSequenceClass const * Sequence;
	unsigned Segment;
	int CyclesLeft;
	bool EndAfterCycle;
	bool EndNow;
	bool Draining;
	unsigned DrainBlocks;
	unsigned Cursor;
	unsigned SourceRate;
	unsigned SourceChannels;
	float Pitch;
	AudioLevelClass Level;
	float Pan;
	float PanTarget;
	float PanRemaining;
	bool ResamplerReady;
	ma_linear_resampler Resampler;
	alignas(16) unsigned char Heap[AUDIO_RESAMPLER_HEAP_BYTES];

	VoiceClass(void) :
		State(AudioVoiceState::FREE),
		Generation(0),
		Group(AUDIO_GROUP_SFX),
		IsStream(false),
		Stream(nullptr),
		Sequence(nullptr),
		Segment(0),
		CyclesLeft(0),
		EndAfterCycle(false),
		EndNow(false),
		Draining(false),
		DrainBlocks(0),
		Cursor(0),
		SourceRate(0),
		SourceChannels(0),
		Pitch(1.0f),
		Pan(0.0f),
		PanTarget(0.0f),
		PanRemaining(0.0f),
		ResamplerReady(false)
	{
		std::memset(&Resampler, 0, sizeof(Resampler));
	}
};


struct GroupClass {
	AudioLevelClass Gain;
	AudioLevelClass Duck;
};


float Soft_Clip(float sample)
{
	float magnitude = std::fabs(sample);
	if (magnitude <= AUDIO_CLIP_THRESHOLD) {
		return(sample);
	}
	float room = 1.0f - AUDIO_CLIP_THRESHOLD;
	float shaped = AUDIO_CLIP_THRESHOLD + room * std::tanh((magnitude - AUDIO_CLIP_THRESHOLD) / room);
	return(sample < 0.0f ? -shaped : shaped);
}

} // namespace


struct AudioMixerClass::StateClass {
	VoiceClass Voices[AUDIO_MAX_VOICES];
	GroupClass Groups[AUDIO_GROUP_COUNT];
	AudioLevelClass Master;
	AudioLevelClass Pause;
	bool PauseObserved;
	bool Frozen;
	SpscRingClass<AudioCommand, AUDIO_COMMAND_QUEUE_SIZE> Commands;

	std::unique_ptr<float[]> Scratch;      // source frames as f32, SCRATCH_FRAMES x channels
	std::unique_ptr<int16_t[]> Staging;    // stream frames before conversion
	std::unique_ptr<float[]> Resampled;    // one block at the output rate, source channels
	unsigned Rate;

	StateClass(void) : PauseObserved(false), Frozen(false), Rate(0) {}

	void Apply(AudioCommand const & command, std::atomic<unsigned> & dropped);
	bool Start_Segment(VoiceClass & voice);
	bool Advance_Segment(VoiceClass & voice);
	bool Configure_Resampler(VoiceClass & voice, unsigned rate, unsigned channels);
	void Set_Pitch(VoiceClass & voice, float pitch);
	unsigned Fetch(VoiceClass & voice, unsigned need);
	void Finish(VoiceClass & voice);
	void Render_Voice(VoiceClass & voice, float * output, unsigned frames, float grouplevel, float master);
};


AudioMixerClass::AudioMixerClass(void) :
	RenderOwner(0),
	PauseAll(false),
	Dropped(0),
	MixRate(0),
	MixChannels(0),
	Ready(false)
{
}


AudioMixerClass::~AudioMixerClass(void)
{
	Shutdown();
}


bool AudioMixerClass::Init(unsigned rate, unsigned channels)
{
	if (Ready || rate == 0 || channels != AUDIO_MIX_CHANNELS) {
		return(false);
	}

	// The voice heaps are fixed; refuse to run if this build's resampler needs more.
	ma_linear_resampler_config probe = ma_linear_resampler_config_init(ma_format_f32, 2, 48000, rate);
	probe.lpfOrder = AUDIO_LPF_ORDER;
	size_t heap = 0;
	if (ma_linear_resampler_get_heap_size(&probe, &heap) != MA_SUCCESS || heap > AUDIO_RESAMPLER_HEAP_BYTES) {
		return(false);
	}

	State.reset(new (std::nothrow) StateClass());
	if (State == nullptr) {
		return(false);
	}
	State->Rate = rate;
	State->Scratch.reset(new (std::nothrow) float[(size_t)SCRATCH_FRAMES * 2]);
	State->Staging.reset(new (std::nothrow) int16_t[(size_t)SCRATCH_FRAMES * 2]);
	State->Resampled.reset(new (std::nothrow) float[(size_t)BLOCK * 2]);
	if (State->Scratch == nullptr || State->Staging == nullptr || State->Resampled == nullptr) {
		State.reset();
		return(false);
	}

	MixRate = rate;
	MixChannels = channels;
	Ready = true;
	return(true);
}


void AudioMixerClass::Shutdown(void)
{
	if (!Ready) {
		return;
	}
	Ready = false;
	for (VoiceClass & voice : State->Voices) {
		if (voice.ResamplerReady) {
			ma_linear_resampler_uninit(&voice.Resampler, nullptr);
			voice.ResamplerReady = false;
		}
	}
	State.reset();
	MixRate = 0;
	MixChannels = 0;
}


bool AudioMixerClass::Allocate_Voice(unsigned slot)
{
	if (!Ready || slot >= AUDIO_MAX_VOICES) {
		return(false);
	}
	VoiceClass & voice = State->Voices[slot];
	AudioVoiceState state = voice.State.load(std::memory_order_acquire);
	if (state != AudioVoiceState::FREE) {
		return(false);
	}
	voice.State.store(AudioVoiceState::ALLOCATED, std::memory_order_release);
	return(true);
}


void AudioMixerClass::Free_Voice(unsigned slot)
{
	if (!Ready || slot >= AUDIO_MAX_VOICES) {
		return;
	}
	VoiceClass & voice = State->Voices[slot];
	AudioVoiceState state = voice.State.load(std::memory_order_acquire);
	if (state == AudioVoiceState::DONE || state == AudioVoiceState::ALLOCATED) {
		voice.State.store(AudioVoiceState::FREE, std::memory_order_release);
	}
}


AudioVoiceState AudioMixerClass::Voice_State(unsigned slot) const
{
	if (!Ready || slot >= AUDIO_MAX_VOICES) {
		return(AudioVoiceState::FREE);
	}
	return(State->Voices[slot].State.load(std::memory_order_acquire));
}


bool AudioMixerClass::Push(AudioCommand const & command)
{
	if (!Ready) {
		return(false);
	}
	return(State->Commands.Push(command));
}


void AudioMixerClass::Set_Pause_All(bool paused)
{
	PauseAll.store(paused, std::memory_order_release);
}


bool AudioMixerClass::Acquire_Render_Token(void)
{
	int expected = 0;
	return(RenderOwner.compare_exchange_strong(expected, 1, std::memory_order_acq_rel));
}


void AudioMixerClass::Release_Render_Token(void)
{
	RenderOwner.store(0, std::memory_order_release);
}


void AudioMixerClass::Render_Callback(void * context, float * output, unsigned frames)
{
	((AudioMixerClass *)context)->Render(output, frames);
}


void AudioMixerClass::Render(float * output, unsigned frames)
{
	if (!Ready || !Acquire_Render_Token()) {
		std::memset(output, 0, (size_t)frames * AUDIO_MIX_CHANNELS * sizeof(float));
		return;
	}

	StateClass & s = *State;

	AudioCommand command;
	while (s.Commands.Pop(command)) {
		s.Apply(command, Dropped);
	}

	bool pause = PauseAll.load(std::memory_order_acquire);
	if (pause != s.PauseObserved) {
		s.PauseObserved = pause;
		if (pause) {
			s.Pause.Adjust_Level(0.0f, AUDIO_PAUSE_RAMP_SECONDS);
		} else {
			s.Frozen = false;
			s.Pause.Restore_Level(AUDIO_PAUSE_RAMP_SECONDS);
		}
	}

	unsigned done = 0;
	while (done < frames) {
		unsigned count = frames - done;
		if (count > BLOCK) {
			count = BLOCK;
		}
		float * block = output + (size_t)done * AUDIO_MIX_CHANNELS;
		std::memset(block, 0, (size_t)count * AUDIO_MIX_CHANNELS * sizeof(float));

		float master = s.Master.Advance(count, s.Rate) * s.Pause.Advance(count, s.Rate);
		if (s.PauseObserved && s.Pause.Is_Settled()) {
			s.Frozen = true;
		}

		if (!s.Frozen) {
			float grouplevels[AUDIO_GROUP_COUNT];
			for (int g = 0; g < AUDIO_GROUP_COUNT; g++) {
				grouplevels[g] = s.Groups[g].Gain.Advance(count, s.Rate) * s.Groups[g].Duck.Advance(count, s.Rate);
			}
			for (VoiceClass & voice : s.Voices) {
				AudioVoiceState state = voice.State.load(std::memory_order_relaxed);
				if (state == AudioVoiceState::PLAYING || state == AudioVoiceState::STOPPING) {
					s.Render_Voice(voice, block, count, grouplevels[voice.Group], master);
				}
			}
			for (unsigned i = 0; i < count * AUDIO_MIX_CHANNELS; i++) {
				block[i] = Soft_Clip(block[i]);
			}
		}
		done += count;
	}

	Release_Render_Token();
}


void AudioMixerClass::StateClass::Apply(AudioCommand const & command, std::atomic<unsigned> & dropped)
{
	switch (command.Type) {
		case AudioCommandType::GROUP_SET_GAIN:
			if (command.Group < AUDIO_GROUP_COUNT) {
				Groups[command.Group].Gain.Set_Level(command.A, command.B);
			}
			return;

		case AudioCommandType::GROUP_SET_DUCK:
			if (command.Group < AUDIO_GROUP_COUNT) {
				Groups[command.Group].Duck.Set_Level(command.A, command.B);
			}
			return;

		case AudioCommandType::MASTER_SET_GAIN:
			Master.Set_Level(command.A, command.B);
			return;

		case AudioCommandType::STOP_ALL:
			for (VoiceClass & voice : Voices) {
				AudioVoiceState state = voice.State.load(std::memory_order_relaxed);
				if (state == AudioVoiceState::PLAYING || state == AudioVoiceState::PAUSED || state == AudioVoiceState::STOPPING) {
					voice.Level.Adjust_Level(0.0f, command.A > 0.0f ? command.A : AUDIO_STOP_RAMP_SECONDS);
					voice.State.store(AudioVoiceState::STOPPING, std::memory_order_relaxed);
				}
			}
			return;

		default:
			break;
	}

	if (command.Slot >= AUDIO_MAX_VOICES) {
		dropped.fetch_add(1, std::memory_order_relaxed);
		return;
	}
	VoiceClass & voice = Voices[command.Slot];
	AudioVoiceState state = voice.State.load(std::memory_order_acquire);

	if (command.Type == AudioCommandType::PLAY_SEQUENCE || command.Type == AudioCommandType::PLAY_STREAM) {
		if (state != AudioVoiceState::ALLOCATED || command.Ptr == nullptr || command.Group >= AUDIO_GROUP_COUNT) {
			dropped.fetch_add(1, std::memory_order_relaxed);
			return;
		}
		voice.Generation = command.Generation;
		voice.Group = (AudioGroupType)command.Group;
		voice.EndAfterCycle = false;
		voice.EndNow = false;
		voice.Draining = false;
		voice.DrainBlocks = 0;
		voice.Cursor = 0;
		voice.Level = AudioLevelClass();
		voice.Level.Set_Level(command.A, 0.0f);
		voice.Pan = command.B;
		voice.PanTarget = command.B;
		voice.PanRemaining = 0.0f;
		voice.Pitch = (command.C > 0.0f) ? command.C : 1.0f;

		bool started;
		if (command.Type == AudioCommandType::PLAY_STREAM) {
			voice.IsStream = true;
			voice.Stream = (AudioStreamClass *)command.Ptr;
			voice.Sequence = nullptr;
			started = Configure_Resampler(voice, voice.Stream->Rate(), voice.Stream->Channels());
		} else {
			voice.IsStream = false;
			voice.Stream = nullptr;
			voice.Sequence = (AudioSequenceClass const *)command.Ptr;
			voice.Segment = 0;
			voice.CyclesLeft = voice.Sequence->Cycles;
			started = voice.Sequence->Count > 0 && voice.Sequence->Count <= AUDIO_MAX_SEQUENCE
				&& voice.Sequence->LoopStart <= voice.Sequence->LoopEnd && voice.Sequence->LoopEnd <= voice.Sequence->Count
				&& Start_Segment(voice);
		}
		if (!started) {
			dropped.fetch_add(1, std::memory_order_relaxed);
			voice.State.store(AudioVoiceState::DONE, std::memory_order_release);
			return;
		}
		voice.State.store(AudioVoiceState::PLAYING, std::memory_order_release);
		return;
	}

	bool live = (state == AudioVoiceState::PLAYING || state == AudioVoiceState::PAUSED || state == AudioVoiceState::STOPPING);
	if (!live || voice.Generation != command.Generation) {
		dropped.fetch_add(1, std::memory_order_relaxed);
		return;
	}

	switch (command.Type) {
		case AudioCommandType::STOP:
			voice.Level.Adjust_Level(0.0f, command.A > 0.0f ? command.A : AUDIO_STOP_RAMP_SECONDS);
			voice.State.store(AudioVoiceState::STOPPING, std::memory_order_relaxed);
			break;

		case AudioCommandType::PAUSE:
			if (state == AudioVoiceState::PLAYING) {
				voice.State.store(AudioVoiceState::PAUSED, std::memory_order_relaxed);
			}
			break;

		case AudioCommandType::RESUME:
			if (state == AudioVoiceState::PAUSED) {
				voice.State.store(AudioVoiceState::PLAYING, std::memory_order_relaxed);
			}
			break;

		case AudioCommandType::SET_GAIN:
			voice.Level.Set_Level(command.A, command.B);
			break;

		case AudioCommandType::SET_PAN:
			voice.PanTarget = command.A;
			voice.PanRemaining = command.B;
			if (command.B <= 0.0f) {
				voice.Pan = command.A;
			}
			break;

		case AudioCommandType::SET_PITCH:
			Set_Pitch(voice, command.A);
			break;

		case AudioCommandType::END_SEQUENCE:
			if (voice.IsStream || voice.Sequence == nullptr) {
				voice.Level.Adjust_Level(0.0f, AUDIO_END_RAMP_SECONDS);
				voice.State.store(AudioVoiceState::STOPPING, std::memory_order_relaxed);
			} else if (command.Mode == (uint8_t)AudioEndMode::AFTER_CYCLE) {
				voice.EndAfterCycle = true;
			} else if (voice.Segment >= voice.Sequence->LoopEnd) {
				// Already in the decay, or there is none: just fade out.
				voice.Level.Adjust_Level(0.0f, AUDIO_END_RAMP_SECONDS);
				voice.State.store(AudioVoiceState::STOPPING, std::memory_order_relaxed);
			} else if (!voice.EndNow) {
				voice.EndNow = true;
				voice.Level.Adjust_Level(0.0f, AUDIO_END_RAMP_SECONDS);
			}
			break;

		default:
			dropped.fetch_add(1, std::memory_order_relaxed);
			break;
	}
}


bool AudioMixerClass::StateClass::Configure_Resampler(VoiceClass & voice, unsigned rate, unsigned channels)
{
	if (rate == 0 || (channels != 1 && channels != 2)) {
		return(false);
	}
	if (voice.ResamplerReady && voice.SourceChannels == channels) {
		if (voice.SourceRate != rate) {
			ma_linear_resampler_set_rate(&voice.Resampler, rate, Rate);
		}
	} else {
		if (voice.ResamplerReady) {
			ma_linear_resampler_uninit(&voice.Resampler, nullptr);
			voice.ResamplerReady = false;
		}
		ma_linear_resampler_config config = ma_linear_resampler_config_init(ma_format_f32, channels, rate, Rate);
		config.lpfOrder = AUDIO_LPF_ORDER;
		if (ma_linear_resampler_init_preallocated(&config, voice.Heap, &voice.Resampler) != MA_SUCCESS) {
			return(false);
		}
		voice.ResamplerReady = true;
	}
	voice.SourceRate = rate;
	voice.SourceChannels = channels;
	Set_Pitch(voice, voice.Pitch);
	return(true);
}


void AudioMixerClass::StateClass::Set_Pitch(VoiceClass & voice, float pitch)
{
	if (pitch <= 0.0f) {
		pitch = 1.0f;
	}
	float ratio = ((float)voice.SourceRate / (float)Rate) * pitch;
	if (ratio > AUDIO_MAX_RATE_RATIO) {
		pitch *= AUDIO_MAX_RATE_RATIO / ratio;
		ratio = AUDIO_MAX_RATE_RATIO;
	}
	voice.Pitch = pitch;
	if (voice.ResamplerReady) {
		ma_linear_resampler_set_rate_ratio(&voice.Resampler, ratio);
	}
}


bool AudioMixerClass::StateClass::Start_Segment(VoiceClass & voice)
{
	AudioSequenceClass const & sequence = *voice.Sequence;
	while (voice.Segment < sequence.Count) {
		AudioSampleClass const * clip = sequence.Clips[voice.Segment];
		if (clip != nullptr && clip->Frames > 0 && clip->Data() != nullptr) {
			voice.Cursor = 0;
			return(Configure_Resampler(voice, clip->Rate, clip->Channels));
		}
		voice.Segment++;
	}
	return(false);
}


// Moves to the segment after the current one, repeating the body when cycles
// remain. Returns false when the sequence is over.
bool AudioMixerClass::StateClass::Advance_Segment(VoiceClass & voice)
{
	AudioSequenceClass const & sequence = *voice.Sequence;
	unsigned segment = voice.Segment;

	if (segment + 1 < sequence.LoopStart) {
		voice.Segment = segment + 1;
	} else if (segment < sequence.LoopEnd) {
		if (segment + 1 < sequence.LoopEnd) {
			voice.Segment = segment + 1;
		} else {
			bool repeat = false;
			if (!voice.EndAfterCycle) {
				if (voice.CyclesLeft < 0) {
					repeat = true;
				} else if (voice.CyclesLeft > 1) {
					voice.CyclesLeft--;
					repeat = true;
				}
			}
			voice.Segment = repeat ? sequence.LoopStart : sequence.LoopEnd;
		}
	} else {
		voice.Segment = segment + 1;
	}

	if (voice.Segment >= sequence.Count) {
		return(false);
	}
	return(Start_Segment(voice));
}


// Fills Scratch with the next source frames as f32. Returns how many were real;
// the rest are zero.
unsigned AudioMixerClass::StateClass::Fetch(VoiceClass & voice, unsigned need)
{
	float * scratch = Scratch.get();
	unsigned channels = voice.SourceChannels;
	unsigned filled = 0;

	if (voice.IsStream) {
		unsigned got = voice.Stream->Ring.Read(Staging.get(), need);
		for (unsigned i = 0; i < got * channels; i++) {
			scratch[i] = (float)Staging[i] * (1.0f / 32768.0f);
		}
		filled = got;
		if (got < need) {
			if (voice.Stream->EndOfInput.load(std::memory_order_acquire) && voice.Stream->Ring.Available_Read() == 0) {
				if (!voice.Draining) {
					voice.Draining = true;
					voice.DrainBlocks = DRAIN_BLOCKS;
				}
			} else {
				voice.Stream->Underruns.fetch_add(1, std::memory_order_relaxed);
			}
		}
	} else {
		while (filled < need && !voice.Draining) {
			AudioSampleClass const * clip = voice.Sequence->Clips[voice.Segment];
			unsigned available = clip->Frames - voice.Cursor;
			unsigned take = need - filled;
			if (take > available) {
				take = available;
			}
			int16_t const * source = clip->Data() + (size_t)voice.Cursor * channels;
			for (unsigned i = 0; i < take * channels; i++) {
				scratch[(size_t)filled * channels + i] = (float)source[i] * (1.0f / 32768.0f);
			}
			voice.Cursor += take;
			filled += take;

			if (voice.Cursor >= clip->Frames) {
				unsigned rate = voice.SourceRate;
				if (!Advance_Segment(voice)) {
					voice.Draining = true;
					voice.DrainBlocks = DRAIN_BLOCKS;
				} else if (voice.SourceChannels != channels || voice.SourceRate != rate) {
					// The next clip has another format; the resampler was just
					// reconfigured for it, so this block ends here.
					break;
				}
			}
		}
	}

	if (filled < need) {
		std::memset(scratch + (size_t)filled * channels, 0, (size_t)(need - filled) * channels * sizeof(float));
	}
	return(filled);
}


void AudioMixerClass::StateClass::Finish(VoiceClass & voice)
{
	voice.Sequence = nullptr;
	voice.Stream = nullptr;
	voice.Draining = false;
	voice.State.store(AudioVoiceState::DONE, std::memory_order_release);
}


void AudioMixerClass::StateClass::Render_Voice(VoiceClass & voice, float * output, unsigned frames, float grouplevel, float master)
{
	float level = voice.Level.Advance(frames, Rate);

	if (voice.State.load(std::memory_order_relaxed) == AudioVoiceState::STOPPING && voice.Level.Is_Settled()) {
		Finish(voice);
		return;
	}

	if (voice.EndNow && voice.Level.Is_Settled()) {
		voice.EndNow = false;
		voice.Level.Restore_Level(0.0f);
		level = voice.Level.Current();
		voice.Segment = voice.Sequence->LoopEnd;
		if (voice.Segment >= voice.Sequence->Count || !Start_Segment(voice)) {
			Finish(voice);
			return;
		}
	}

	float gain = Audio_Perceptual_Gain(level * grouplevel * master);

	if (voice.PanRemaining > 0.0f) {
		float seconds = (float)frames / (float)Rate;
		if (seconds >= voice.PanRemaining) {
			voice.Pan = voice.PanTarget;
			voice.PanRemaining = 0.0f;
		} else {
			voice.Pan += (voice.PanTarget - voice.Pan) * (seconds / voice.PanRemaining);
			voice.PanRemaining -= seconds;
		}
	}
	float left = gain * ((1.0f - voice.Pan) < 1.0f ? (1.0f - voice.Pan) : 1.0f);
	float right = gain * ((1.0f + voice.Pan) < 1.0f ? (1.0f + voice.Pan) : 1.0f);

	ma_uint64 need = 0;
	ma_linear_resampler_get_required_input_frame_count(&voice.Resampler, frames, &need);
	if (need > SCRATCH_FRAMES) {
		need = SCRATCH_FRAMES;
	}

	unsigned channels = voice.SourceChannels;
	Fetch(voice, (unsigned)need);

	ma_uint64 in = need;
	ma_uint64 out = frames;
	ma_linear_resampler_process_pcm_frames(&voice.Resampler, Scratch.get(), &in, Resampled.get(), &out);
	if (out < frames) {
		std::memset(Resampled.get() + (size_t)out * channels, 0, (size_t)(frames - out) * channels * sizeof(float));
	}

	float const * resampled = Resampled.get();
	if (channels == 1) {
		for (unsigned i = 0; i < frames; i++) {
			output[i * 2] += resampled[i] * left;
			output[i * 2 + 1] += resampled[i] * right;
		}
	} else {
		for (unsigned i = 0; i < frames; i++) {
			output[i * 2] += resampled[i * 2] * left;
			output[i * 2 + 1] += resampled[i * 2 + 1] * right;
		}
	}

	if (voice.Draining) {
		if (voice.DrainBlocks == 0) {
			Finish(voice);
		} else {
			voice.DrainBlocks--;
		}
	}
}
