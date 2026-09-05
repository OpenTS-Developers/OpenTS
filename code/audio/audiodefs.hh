/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Enumerations and constants shared by the audio engine and the game layer that
// configures it.

#pragma once

#include <cstddef>
#include <cstdint>


// Mixing groups. Each has its own gain and duck level. The option sliders drive
// SFX, SPEECH, MUSIC and MOVIE; SYSTEM plays at unity.
enum AudioGroupType {
	AUDIO_GROUP_SFX,
	AUDIO_GROUP_SPEECH,
	AUDIO_GROUP_MUSIC,
	AUDIO_GROUP_MOVIE,
	AUDIO_GROUP_SYSTEM,

	AUDIO_GROUP_COUNT
};


// A voice belongs to the game thread while FREE, ALLOCATED or DONE and to the
// device thread while PLAYING, PAUSED or STOPPING.
enum class AudioVoiceState : uint8_t {
	FREE,
	ALLOCATED,
	PLAYING,
	PAUSED,
	STOPPING,
	DONE
};


enum class AudioCommandType : uint8_t {
	PLAY_SEQUENCE,
	PLAY_STREAM,
	END_SEQUENCE,
	STOP,
	PAUSE,
	RESUME,
	SET_GAIN,
	SET_PAN,
	SET_PITCH,
	GROUP_SET_GAIN,
	GROUP_SET_DUCK,
	MASTER_SET_GAIN,
	STOP_ALL,
	PAUSE_ALL,
	RESUME_ALL
};


// How END_SEQUENCE leaves the body before the decay plays.
enum class AudioEndMode : uint8_t {
	NOW,
	AFTER_CYCLE
};


// Type= flags. The bit values follow Yuri's Revenge so its documentation applies.
enum SoundTypeFlag : unsigned {
	SOUND_TYPE_NORMAL = 0x0000,
	SOUND_TYPE_VIOLENT = 0x0001,
	SOUND_TYPE_MOVEMENT = 0x0002,
	SOUND_TYPE_QUIET = 0x0004,
	SOUND_TYPE_LOUD = 0x0008,
	SOUND_TYPE_GLOBAL = 0x0010,
	SOUND_TYPE_SCREEN = 0x0020,
	SOUND_TYPE_LOCAL = 0x0040,
	SOUND_TYPE_PLAYER = 0x0080,
	SOUND_TYPE_NOISE_SHY = 0x0100,
	SOUND_TYPE_GUN_SHY = 0x0200,
	SOUND_TYPE_UNSHROUD = 0x0400,
	SOUND_TYPE_SHROUD = 0x0800,
	SOUND_TYPE_AMBIENT = 0x1000,
	SOUND_TYPE_HIDDEN = 0x2000
};


// Control= flags. Yuri's Revenge bit values plus the two Vinifera additions.
enum SoundControlFlag : unsigned {
	SOUND_CONTROL_NONE = 0x0000,
	SOUND_CONTROL_LOOP = 0x0001,
	SOUND_CONTROL_RANDOM = 0x0002,
	SOUND_CONTROL_ALL = 0x0004,
	SOUND_CONTROL_PREDELAY = 0x0008,
	SOUND_CONTROL_INTERRUPT = 0x0010,
	SOUND_CONTROL_ATTACK = 0x0020,
	SOUND_CONTROL_DECAY = 0x0040,
	SOUND_CONTROL_AMBIENT = 0x0080,
	SOUND_CONTROL_SEQUENTIAL = 0x0100,
	SOUND_CONTROL_QUEUE = 0x0200
};


// Sizes.
int const AUDIO_MIX_RATE = 48000;
int const AUDIO_MIX_CHANNELS = 2;
int const AUDIO_MAX_VOICES = 64;
int const AUDIO_MAX_EVENTS = 128;
int const AUDIO_MAX_STREAMS = 4;
int const AUDIO_MAX_SOUNDS = 32;
int const AUDIO_MAX_SEQUENCE = AUDIO_MAX_SOUNDS + 2;
int const AUDIO_COMMAND_QUEUE_SIZE = 512;
int const AUDIO_RENDER_BLOCK_FRAMES = 64;
int const AUDIO_MAX_RENDER_FRAMES = 2048;
int const AUDIO_PERIOD_MS = 10;
int const AUDIO_PERIODS = 3;

// Timing.
float const AUDIO_STOP_RAMP_SECONDS = 0.005f;
float const AUDIO_RETARGET_RAMP_SECONDS = 0.05f;
float const AUDIO_END_RAMP_SECONDS = 0.25f;
float const AUDIO_DUCK_RAMP_SECONDS = 0.25f;
float const AUDIO_ADJUST_RAMP_SECONDS = 1.0f;
int const AUDIO_FEEDER_PERIOD_MS = 16;
int const AUDIO_DEVICE_RETRY_MS = 2000;
int const AUDIO_DEVICE_REOPEN_MS = 10000;
float const AUDIO_FILE_STREAM_SECONDS = 5.0f;
float const AUDIO_FILE_STREAM_LOW_WATER_SECONDS = 2.0f;
int const AUDIO_MOVIE_RING_BLOCKS = 4;

// Memory.
size_t const AUDIO_CACHE_BUDGET_BYTES = 64u * 1024u * 1024u;
size_t const AUDIO_CACHE_MAX_SAMPLE_BYTES = 8u * 1024u * 1024u;

// Mixing.
float const AUDIO_CLIP_THRESHOLD = 0.9f;
