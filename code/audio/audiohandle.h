/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The one handle type the game holds for anything it started: a sound effect
// event, a music, speech or movie stream, or a raw sample. It is a 4-byte value
// with a slot index and a generation stamp, so a copy kept in a game object
// stays harmless after the sound ends and the slot is reused.

#pragma once

#include <cstdint>

class AudioEventTypeClass;


class AudioHandle
{
	public:
		enum : uint32_t {
			INDEX_BITS = 8,
			GENERATION_BITS = 23,
			INDEX_MASK = (1u << INDEX_BITS) - 1,
			GENERATION_MASK = (1u << GENERATION_BITS) - 1
		};

		AudioHandle(void) = default;

		static AudioHandle Make(unsigned index, unsigned generation)
		{
			AudioHandle handle;
			handle.Value = (index & INDEX_MASK) | ((generation & GENERATION_MASK) << INDEX_BITS);
			return(handle);
		}

		// Generations never take the value zero, which is reserved for the null handle.
		static unsigned Next_Generation(unsigned generation)
		{
			generation = (generation + 1) & GENERATION_MASK;
			return(generation == 0 ? 1 : generation);
		}

		// Bit 31 is never set, so the raw value is non-negative as an int and -1 can
		// stand for "no handle" in code that still expects the old integer handles.
		static AudioHandle From_Raw(uint32_t raw)
		{
			AudioHandle handle;
			handle.Value = raw & 0x7FFFFFFFu;
			return(handle);
		}

		uint32_t Raw(void) const { return(Value); }
		bool Is_Null(void) const { return(Value == 0); }
		unsigned Index(void) const { return(Value & INDEX_MASK); }
		unsigned Generation(void) const { return((Value >> INDEX_BITS) & GENERATION_MASK); }

		void Clear(void) { Value = 0; }

		bool operator==(AudioHandle const & that) const { return(Value == that.Value); }
		bool operator!=(AudioHandle const & that) const { return(Value != that.Value); }

		// Answered by the audio engine. On a null or stale handle every one of these is
		// a no-op that returns false or null.
		bool Is_Valid(void) const;
		bool Is_Playing(void) const;
		AudioEventTypeClass const * Type(void) const;

		void Retarget(float volume, float pan);
		void Set_Volume(float volume);
		void Set_Pan(float pan);
		void Stop(void);
		void End(void);
		void End_Looping(void);
		void Fade(int ms);

	private:
		uint32_t Value = 0;
};
