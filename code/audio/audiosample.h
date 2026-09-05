/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Decoded samples and the cache that owns them. Everything here runs on the game
// thread; the mixer only reads the PCM of samples the game has pinned for it.

#pragma once

#include "audio/audiodefs.hh"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>


class AudioSampleClass
{
	public:
		AudioSampleClass(void);

		unsigned Rate;
		unsigned Channels;
		unsigned Frames;
		std::unique_ptr<int16_t[]> Pcm;

		size_t Bytes(void) const { return((size_t)Frames * Channels * sizeof(int16_t)); }
		int16_t const * Data(void) const { return(Pcm.get()); }

		// Number of holders that may still hand the sample to the mixer. Only an
		// unpinned sample can be evicted.
		int PinCount;

		uint64_t Key;
		uint32_t LastUse;
};


// Supplies file contents to the cache. The engine's reader goes through the
// game's file layer; tests supply memory.
class AudioAssetReaderClass
{
	public:
		virtual ~AudioAssetReaderClass(void) = default;
		virtual bool Read(char const * filename, std::vector<uint8_t> & bytes) = 0;
};


class AudioSampleCacheClass
{
	public:
		explicit AudioSampleCacheClass(size_t budget = AUDIO_CACHE_BUDGET_BYTES, size_t maxsample = AUDIO_CACHE_MAX_SAMPLE_BYTES);
		~AudioSampleCacheClass(void);

		AudioSampleCacheClass(AudioSampleCacheClass const &) = delete;
		AudioSampleCacheClass & operator=(AudioSampleCacheClass const &) = delete;

		// Both return a pinned sample, or null when the data cannot be decoded or is
		// too large for the cache. A blob is an AUD in memory; its size may be zero
		// when only the header knows it. The blob's contents are hashed, so a buffer
		// refilled with a different sample misses rather than replaying the old one.
		AudioSampleClass * Acquire_Blob(void const * blob, size_t size);
		AudioSampleClass * Acquire_Named(char const * basename, AudioAssetReaderClass & reader);

		void Release(AudioSampleClass * sample);

		// Forgets every entry decoded from the blob; call before the buffer is freed.
		void Invalidate_Blob(void const * blob);

		// Drops every unpinned entry and returns how many pinned ones remain.
		unsigned Flush(void);

		size_t Bytes(void) const { return(TotalBytes); }
		unsigned Count(void) const { return((unsigned)Table.size()); }
		bool Is_Cached(uint64_t key) const { return(Table.find(key) != Table.end()); }

		static uint64_t Hash_Bytes(void const * data, size_t size, uint64_t seed);
		static uint64_t Hash_Name(char const * basename);

		// The extensions tried for a named sample, in order of preference, so a loose
		// modded file wins over an archived one.
		static int const FORMAT_COUNT = 5;
		static char const * const FORMAT_EXTENSIONS[FORMAT_COUNT];

	private:
		AudioSampleClass * Find(uint64_t key);
		AudioSampleClass * Insert(uint64_t key, std::unique_ptr<AudioSampleClass> sample);
		std::unique_ptr<AudioSampleClass> Decode(void const * data, size_t size, bool audonly);
		void Evict_To_Budget(void);
		void Remove(uint64_t key);

		std::unordered_map<uint64_t, std::unique_ptr<AudioSampleClass>> Table;

		// Which extension a name resolved to last time, or -1 when none was found, so
		// a re-load after eviction and a missing sound both skip the probing.
		std::unordered_map<uint64_t, int> NameFormats;

		size_t Budget;
		size_t MaxSample;
		size_t TotalBytes;
		uint32_t Clock;
};
