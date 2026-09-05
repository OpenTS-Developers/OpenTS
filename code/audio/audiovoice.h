/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The data the game thread hands the mixer: a command, and the sequence of
// clips a voice plays. Both are plain data; the mixer never writes to a
// sequence and the game never changes one while a voice is playing it.

#pragma once

#include "audio/audiodefs.hh"

#include <cstdint>

class AudioSampleClass;


// The clips one voice plays in order: attack segments, the body segments that
// repeat, then decay segments. A one-shot has one body segment and one cycle.
struct AudioSequenceClass {
	AudioSampleClass const * Clips[AUDIO_MAX_SEQUENCE];
	unsigned Count;
	unsigned LoopStart;   // first body segment
	unsigned LoopEnd;     // one past the last body segment
	int Cycles;           // body cycles to play; -1 repeats until ended
};


struct AudioCommand {
	AudioCommandType Type;
	uint8_t Group;
	uint8_t Slot;
	uint8_t Mode;         // AudioEndMode for END_SEQUENCE
	uint32_t Generation;  // must match the voice's for every command after its play
	float A;              // level, pan, pitch, or seconds, by command
	float B;              // pan or ramp seconds
	float C;              // pitch
	void const * Ptr;     // AudioSequenceClass const * or AudioStreamClass *
};
