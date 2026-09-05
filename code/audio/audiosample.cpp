/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "audio/audiosample.h"

#include "audio/audiodecode.h"

#include <cctype>
#include <cstring>
#include <string>


uint64_t const FNV_BASIS = 14695981039346656037ULL;
uint64_t const FNV_PRIME = 1099511628211ULL;
uint64_t const BLOB_KEY_BIT = 1ULL << 63;

char const * const AudioSampleCacheClass::FORMAT_EXTENSIONS[AudioSampleCacheClass::FORMAT_COUNT] = {
	".WAV", ".OGG", ".FLAC", ".MP3", ".AUD"
};


AudioSampleClass::AudioSampleClass(void) :
	Rate(0),
	Channels(0),
	Frames(0),
	PinCount(0),
	Key(0),
	LastUse(0)
{
}


AudioSampleCacheClass::AudioSampleCacheClass(size_t budget, size_t maxsample) :
	Budget(budget),
	MaxSample(maxsample),
	TotalBytes(0),
	Clock(0)
{
}


AudioSampleCacheClass::~AudioSampleCacheClass(void)
{
}


uint64_t AudioSampleCacheClass::Hash_Bytes(void const * data, size_t size, uint64_t seed)
{
	unsigned char const * bytes = (unsigned char const *)data;
	uint64_t hash = seed;
	for (size_t i = 0; i < size; i++) {
		hash ^= bytes[i];
		hash *= FNV_PRIME;
	}
	return(hash);
}


uint64_t AudioSampleCacheClass::Hash_Name(char const * basename)
{
	uint64_t hash = FNV_BASIS;
	for (char const * c = basename; *c != '\0'; c++) {
		hash ^= (unsigned char)std::toupper((unsigned char)*c);
		hash *= FNV_PRIME;
	}
	return(hash & ~BLOB_KEY_BIT);
}


AudioSampleClass * AudioSampleCacheClass::Find(uint64_t key)
{
	auto it = Table.find(key);
	if (it == Table.end()) {
		return(nullptr);
	}
	AudioSampleClass * sample = it->second.get();
	sample->PinCount++;
	sample->LastUse = ++Clock;
	return(sample);
}


AudioSampleClass * AudioSampleCacheClass::Insert(uint64_t key, std::unique_ptr<AudioSampleClass> sample)
{
	sample->Key = key;
	sample->PinCount = 1;
	sample->LastUse = ++Clock;
	TotalBytes += sample->Bytes();
	AudioSampleClass * raw = sample.get();
	Table[key] = std::move(sample);
	Evict_To_Budget();
	return(raw);
}


void AudioSampleCacheClass::Remove(uint64_t key)
{
	auto it = Table.find(key);
	if (it != Table.end()) {
		TotalBytes -= it->second->Bytes();
		Table.erase(it);
	}
}


void AudioSampleCacheClass::Evict_To_Budget(void)
{
	while (TotalBytes > Budget) {
		AudioSampleClass * oldest = nullptr;
		for (auto & entry : Table) {
			AudioSampleClass * sample = entry.second.get();
			if (sample->PinCount == 0 && (oldest == nullptr || sample->LastUse < oldest->LastUse)) {
				oldest = sample;
			}
		}
		if (oldest == nullptr) {
			break;
		}
		Remove(oldest->Key);
	}
}


std::unique_ptr<AudioSampleClass> AudioSampleCacheClass::Decode(void const * data, size_t size, bool audonly)
{
	std::unique_ptr<AudioSampleClass> sample(new (std::nothrow) AudioSampleClass());
	if (sample == nullptr) {
		return(nullptr);
	}

	AUDHeaderType header;
	if (Aud_Read_Header(data, size, header)) {
		unsigned capacity = Aud_Frame_Capacity(header);
		unsigned channels = Aud_Channels(header);
		if (capacity == 0 || (size_t)capacity * channels * sizeof(int16_t) > MaxSample) {
			return(nullptr);
		}
		sample->Pcm.reset(new (std::nothrow) int16_t[(size_t)capacity * channels]);
		if (sample->Pcm == nullptr) {
			return(nullptr);
		}
		AudioPcmFormat format;
		unsigned frames = Aud_Decode(data, size, sample->Pcm.get(), capacity, format);
		if (frames == 0) {
			return(nullptr);
		}
		sample->Rate = format.Rate;
		sample->Channels = format.Channels;
		sample->Frames = frames;
		return(sample);
	}

	if (audonly) {
		return(nullptr);
	}

	std::vector<int16_t> pcm;
	AudioPcmFormat format;
	if (!Audio_Decode_Other(data, size, pcm, format)) {
		return(nullptr);
	}
	if (pcm.size() * sizeof(int16_t) > MaxSample) {
		return(nullptr);
	}
	sample->Pcm.reset(new (std::nothrow) int16_t[pcm.size()]);
	if (sample->Pcm == nullptr) {
		return(nullptr);
	}
	std::memcpy(sample->Pcm.get(), pcm.data(), pcm.size() * sizeof(int16_t));
	sample->Rate = format.Rate;
	sample->Channels = format.Channels;
	sample->Frames = (unsigned)(pcm.size() / format.Channels);
	return(sample);
}


AudioSampleClass * AudioSampleCacheClass::Acquire_Blob(void const * blob, size_t size)
{
	AUDHeaderType header;
	if (!Aud_Read_Header(blob, size, header)) {
		return(nullptr);
	}
	size_t bytes = Aud_Blob_Bytes(header);
	if (size != 0 && bytes > size) {
		bytes = size;
	}

	uintptr_t address = (uintptr_t)blob;
	uint64_t key = Hash_Bytes(&address, sizeof(address), FNV_BASIS);
	key = Hash_Bytes(blob, bytes, key) | BLOB_KEY_BIT;

	AudioSampleClass * sample = Find(key);
	if (sample != nullptr) {
		return(sample);
	}

	std::unique_ptr<AudioSampleClass> decoded = Decode(blob, bytes, true);
	if (decoded == nullptr) {
		return(nullptr);
	}
	return(Insert(key, std::move(decoded)));
}


AudioSampleClass * AudioSampleCacheClass::Acquire_Named(char const * basename, AudioAssetReaderClass & reader)
{
	if (basename == nullptr || basename[0] == '\0') {
		return(nullptr);
	}
	uint64_t key = Hash_Name(basename);

	AudioSampleClass * sample = Find(key);
	if (sample != nullptr) {
		return(sample);
	}

	int first = 0;
	int last = FORMAT_COUNT - 1;
	auto known = NameFormats.find(key);
	if (known != NameFormats.end()) {
		if (known->second < 0) {
			return(nullptr);
		}
		first = known->second;
		last = known->second;
	}

	std::vector<uint8_t> bytes;
	for (int format = first; format <= last; format++) {
		std::string filename(basename);
		filename += FORMAT_EXTENSIONS[format];
		if (!reader.Read(filename.c_str(), bytes) || bytes.empty()) {
			continue;
		}
		std::unique_ptr<AudioSampleClass> decoded = Decode(bytes.data(), bytes.size(), false);
		if (decoded == nullptr) {
			continue;
		}
		NameFormats[key] = format;
		return(Insert(key, std::move(decoded)));
	}

	NameFormats[key] = -1;
	return(nullptr);
}


void AudioSampleCacheClass::Release(AudioSampleClass * sample)
{
	if (sample != nullptr && sample->PinCount > 0) {
		sample->PinCount--;
		if (sample->PinCount == 0) {
			Evict_To_Budget();
		}
	}
}


void AudioSampleCacheClass::Invalidate_Blob(void const * blob)
{
	// Blob keys mix the address into the content hash, so the entries decoded from
	// this blob cannot be picked out; every unpinned blob entry goes. Blobs are
	// few and re-decode cheaply.
	(void)blob;
	std::vector<uint64_t> doomed;
	for (auto & entry : Table) {
		if ((entry.first & BLOB_KEY_BIT) != 0 && entry.second->PinCount == 0) {
			doomed.push_back(entry.first);
		}
	}
	for (uint64_t key : doomed) {
		Remove(key);
	}
}


unsigned AudioSampleCacheClass::Flush(void)
{
	std::vector<uint64_t> doomed;
	unsigned pinned = 0;
	for (auto & entry : Table) {
		if (entry.second->PinCount == 0) {
			doomed.push_back(entry.first);
		} else {
			pinned++;
		}
	}
	for (uint64_t key : doomed) {
		Remove(key);
	}
	NameFormats.clear();
	return(pinned);
}
