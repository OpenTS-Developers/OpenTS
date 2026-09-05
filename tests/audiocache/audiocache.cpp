/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Exercises the sample cache: named lookups try the modded formats before AUD
// and remember the answer, pinned samples survive the budget while unpinned
// ones leave oldest first, blobs are keyed by their whole contents so a
// refilled buffer misses, and a sample too large for the cache is refused.
// Needs no game data.

#include "audio/audiosample.h"
#include "audio/audiodecode.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>

namespace {

int Failures = 0;
int Checked = 0;


void Check(bool condition, char const * what)
{
	Checked++;
	if (!condition) {
		Failures++;
		std::printf("FAIL: %s\n", what);
	}
}


void Append(std::vector<uint8_t> & blob, void const * data, size_t size)
{
	uint8_t const * bytes = (uint8_t const *)data;
	blob.insert(blob.end(), bytes, bytes + size);
}


// A raw 16-bit mono AUD holding `frames` frames of the given fill value.
std::vector<uint8_t> Make_Aud(unsigned frames, int16_t fill, unsigned rate = 22050)
{
	AUDHeaderType header;
	header.Rate = (uint16_t)rate;
	header.Size = (int32_t)(frames * 2);
	header.UncompSize = header.Size;
	header.Flags = AUD_FLAG_16BIT;
	header.Compression = AUD_CODEC_PCM;
	std::vector<uint8_t> blob;
	Append(blob, &header, sizeof(header));
	for (unsigned i = 0; i < frames; i++) {
		Append(blob, &fill, sizeof(fill));
	}
	return(blob);
}


std::vector<uint8_t> Make_Wav(unsigned frames, int16_t fill, unsigned rate)
{
	std::vector<uint8_t> wav;
	uint32_t datasize = frames * 2;
	uint32_t riffsize = 36 + datasize;
	Append(wav, "RIFF", 4);
	Append(wav, &riffsize, 4);
	Append(wav, "WAVEfmt ", 8);
	uint32_t fmtsize = 16;
	uint16_t format = 1;
	uint16_t channels = 1;
	uint32_t byterate = rate * 2;
	uint16_t align = 2;
	uint16_t bits = 16;
	Append(wav, &fmtsize, 4);
	Append(wav, &format, 2);
	Append(wav, &channels, 2);
	Append(wav, &rate, 4);
	Append(wav, &byterate, 4);
	Append(wav, &align, 2);
	Append(wav, &bits, 2);
	Append(wav, "data", 4);
	Append(wav, &datasize, 4);
	for (unsigned i = 0; i < frames; i++) {
		Append(wav, &fill, sizeof(fill));
	}
	return(wav);
}


class MemoryReaderClass : public AudioAssetReaderClass
{
	public:
		std::map<std::string, std::vector<uint8_t>> Files;
		int Reads = 0;

		// Case-insensitive, like the game's file layer.
		bool Read(char const * filename, std::vector<uint8_t> & bytes) override
		{
			Reads++;
			std::string upper(filename);
			for (char & c : upper) {
				c = (char)std::toupper((unsigned char)c);
			}
			auto it = Files.find(upper);
			if (it == Files.end()) {
				return(false);
			}
			bytes = it->second;
			return(true);
		}
};


void Test_Named(void)
{
	MemoryReaderClass reader;
	reader.Files["BOTH.WAV"] = Make_Wav(50, 7, 44100);
	reader.Files["BOTH.AUD"] = Make_Aud(80, 9);
	reader.Files["ONLYAUD.AUD"] = Make_Aud(30, 3);
	reader.Files["BROKEN.AUD"] = std::vector<uint8_t>(5, 0);

	AudioSampleCacheClass cache;

	AudioSampleClass * both = cache.Acquire_Named("both", reader);
	Check(both != nullptr, "named sample with two formats loads");
	Check(both != nullptr && both->Frames == 50 && both->Rate == 44100, "WAV is preferred over AUD");
	Check(both != nullptr && both->Data()[0] == 7, "WAV samples arrive");
	Check(reader.Reads == 1, "the first extension that exists is the only read");

	AudioSampleClass * again = cache.Acquire_Named("BOTH", reader);
	Check(again == both, "a second acquire returns the cached sample");
	Check(both != nullptr && both->PinCount == 2, "each acquire pins");
	Check(reader.Reads == 1, "a cached sample is not re-read");

	reader.Reads = 0;
	AudioSampleClass * onlyaud = cache.Acquire_Named("onlyaud", reader);
	Check(onlyaud != nullptr && onlyaud->Frames == 30 && onlyaud->Data()[0] == 3, "AUD is found after the other extensions miss");
	Check(reader.Reads == 5, "every extension is probed once for an AUD-only sample");

	reader.Reads = 0;
	Check(cache.Acquire_Named("missing", reader) == nullptr, "a missing sample returns null");
	Check(reader.Reads == 5, "a missing sample is probed once");
	Check(cache.Acquire_Named("missing", reader) == nullptr, "a missing sample stays null");
	Check(reader.Reads == 5, "a missing sample is not probed again");

	reader.Reads = 0;
	Check(cache.Acquire_Named("broken", reader) == nullptr, "a sample that does not decode returns null");

	// After eviction, the remembered extension is the only one read.
	cache.Release(both);
	cache.Release(again);
	cache.Release(onlyaud);
	Check(cache.Flush() == 0, "flush with nothing pinned drops everything");
	Check(cache.Count() == 0 && cache.Bytes() == 0, "flush empties the cache");
	reader.Reads = 0;
	onlyaud = cache.Acquire_Named("onlyaud", reader);
	Check(onlyaud != nullptr, "a flushed sample reloads");
	Check(reader.Reads == 5, "a flush also forgets which extension a name resolved to");
	cache.Release(onlyaud);
	Check(cache.Acquire_Named("", reader) == nullptr, "an empty name returns null");
}


void Test_Budget(void)
{
	std::vector<uint8_t> a = Make_Aud(100, 1);
	std::vector<uint8_t> b = Make_Aud(100, 2);
	std::vector<uint8_t> c = Make_Aud(100, 3);
	std::vector<uint8_t> d = Make_Aud(100, 4);
	size_t each = 100 * sizeof(int16_t);

	AudioSampleCacheClass cache(each * 3, each * 10);

	AudioSampleClass * sa = cache.Acquire_Blob(a.data(), a.size());
	AudioSampleClass * sb = cache.Acquire_Blob(b.data(), b.size());
	AudioSampleClass * sc = cache.Acquire_Blob(c.data(), c.size());
	Check(sa != nullptr && sb != nullptr && sc != nullptr, "three samples fit the budget");
	Check(cache.Bytes() == each * 3, "bytes are accounted");

	uint64_t keya = sa->Key;
	uint64_t keyb = sb->Key;
	cache.Release(sa);
	cache.Release(sb);

	AudioSampleClass * sd = cache.Acquire_Blob(d.data(), d.size());
	Check(sd != nullptr, "a fourth sample loads over budget");
	Check(cache.Count() == 3, "one sample was evicted to make room");
	Check(!cache.Is_Cached(keya) && cache.Is_Cached(keyb), "the least recently used unpinned sample left first");

	cache.Release(sc);
	cache.Release(sd);

	// Pinned samples are never evicted, even when that leaves the cache over budget.
	AudioSampleClass * pa = cache.Acquire_Blob(a.data(), a.size());
	AudioSampleClass * pb = cache.Acquire_Blob(b.data(), b.size());
	AudioSampleClass * pc = cache.Acquire_Blob(c.data(), c.size());
	AudioSampleClass * pd = cache.Acquire_Blob(d.data(), d.size());
	Check(pa != nullptr && pb != nullptr && pc != nullptr && pd != nullptr, "pinned samples all load");
	Check(cache.Count() == 4 && cache.Bytes() == each * 4, "pinned samples stay over budget");
	Check(cache.Flush() == 4, "flush reports the pinned samples it kept");

	cache.Release(pa);
	Check(cache.Count() == 3, "releasing under pressure evicts at once");
	cache.Release(pb);
	cache.Release(pc);
	cache.Release(pd);
	Check(cache.Count() == 3, "releases below the budget evict nothing more");
}


void Test_Blobs(void)
{
	AudioSampleCacheClass cache;

	std::vector<uint8_t> first = Make_Aud(200, 5);
	std::vector<uint8_t> second = Make_Aud(200, 5);
	second[second.size() - 2] = 0x12;
	second[second.size() - 1] = 0x34;
	Check(std::memcmp(first.data(), second.data(), 64) == 0 && first.size() == second.size(), "the two blobs share size and prefix");

	AudioSampleClass * sfirst = cache.Acquire_Blob(first.data(), first.size());
	AudioSampleClass * ssecond = cache.Acquire_Blob(second.data(), second.size());
	Check(sfirst != nullptr && ssecond != nullptr && sfirst != ssecond, "blobs differing only in their tail get different entries");
	Check(ssecond != nullptr && ssecond->Data()[199] == 0x3412, "the differing tail is what decoded");

	AudioSampleClass * same = cache.Acquire_Blob(first.data(), first.size());
	Check(same == sfirst && sfirst->PinCount == 2, "the same blob hits its entry");

	// The same buffer refilled with a different sample must not replay the old one.
	std::vector<uint8_t> buffer = Make_Aud(150, 11);
	AudioSampleClass * before = cache.Acquire_Blob(buffer.data(), 0);
	Check(before != nullptr && before->Frames == 150 && before->Data()[0] == 11, "size zero trusts the header");
	cache.Release(before);
	std::vector<uint8_t> replacement = Make_Aud(150, 22);
	std::memcpy(buffer.data(), replacement.data(), replacement.size());
	AudioSampleClass * after = cache.Acquire_Blob(buffer.data(), 0);
	Check(after != nullptr && after != before && after->Data()[0] == 22, "a refilled buffer decodes the new contents");
	cache.Release(after);

	unsigned count = cache.Count();
	cache.Invalidate_Blob(buffer.data());
	Check(cache.Count() < count, "invalidating drops unpinned blob entries");
	Check(cache.Is_Cached(sfirst->Key), "a pinned entry survives invalidation");

	cache.Release(sfirst);
	cache.Release(same);
	cache.Release(ssecond);

	Check(cache.Acquire_Blob(nullptr, 0) == nullptr, "a null blob returns null");
	std::vector<uint8_t> junk(40, 0xEE);
	Check(cache.Acquire_Blob(junk.data(), junk.size()) == nullptr, "a blob without a valid header returns null");
	Check(cache.Acquire_Blob(first.data(), 20) == nullptr, "a blob smaller than its header claims returns null");
}


void Test_Limits(void)
{
	AudioSampleCacheClass cache(1u << 20, 100);
	std::vector<uint8_t> big = Make_Aud(100, 1);
	Check(cache.Acquire_Blob(big.data(), big.size()) == nullptr, "a sample above the per-sample cap is refused");
	std::vector<uint8_t> small = Make_Aud(40, 1);
	AudioSampleClass * sample = cache.Acquire_Blob(small.data(), small.size());
	Check(sample != nullptr, "a sample under the cap loads");
	cache.Release(sample);
	cache.Release(nullptr);
	Check(cache.Count() == 1, "releasing null is harmless");
}

} // namespace


int main(void)
{
	Test_Named();
	Test_Budget();
	Test_Blobs();
	Test_Limits();

	std::printf("audiocache: %d checks, %d failures\n", Checked, Failures);
	return(Failures == 0 ? 0 : 1);
}
