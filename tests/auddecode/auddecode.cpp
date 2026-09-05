/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Checks the AUD reader against the codecs it wraps: SOS chunks decode to what
// the codec produces when called directly with the same running state, the
// Westwood delta codec matches hand-built streams and refuses malformed ones,
// raw samples convert to 16-bit, the header rules hold, a bad chunk ends the
// sample cleanly, the chunk-at-a-time path equals the whole decode, and a WAV
// decodes through miniaudio. Needs no game data.

#include "audio/audiodecode.h"
#include "soscomp.h"

#include <cstdio>
#include <cstring>
#include <vector>

namespace {

int Failures = 0;
int Checked = 0;

unsigned int Seed = 12345;


void Check(bool condition, char const * what)
{
	Checked++;
	if (!condition) {
		Failures++;
		std::printf("FAIL: %s\n", what);
	}
}


unsigned int Next_Random(void)
{
	Seed = Seed * 1103515245u + 12345u;
	return(Seed >> 8);
}


void Append(std::vector<uint8_t> & blob, void const * data, size_t size)
{
	uint8_t const * bytes = (uint8_t const *)data;
	blob.insert(blob.end(), bytes, bytes + size);
}


std::vector<uint8_t> Make_Header(unsigned rate, unsigned flags, unsigned codec, int size, int uncompsize)
{
	AUDHeaderType header;
	header.Rate = (uint16_t)rate;
	header.Size = size;
	header.UncompSize = uncompsize;
	header.Flags = (uint8_t)flags;
	header.Compression = (uint8_t)codec;
	std::vector<uint8_t> blob;
	Append(blob, &header, sizeof(header));
	return(blob);
}


void Append_Chunk(std::vector<uint8_t> & blob, std::vector<uint8_t> const & payload, unsigned uncompsize, uint32_t magic = AUD_CHUNK_MAGIC)
{
	AUDChunkHeaderType chunk;
	chunk.CompSize = (uint16_t)payload.size();
	chunk.UncompSize = (uint16_t)uncompsize;
	chunk.Magic = magic;
	Append(blob, &chunk, sizeof(chunk));
	Append(blob, payload.data(), payload.size());
}


void Patch_Size(std::vector<uint8_t> & blob)
{
	int32_t size = (int32_t)(blob.size() - sizeof(AUDHeaderType));
	std::memcpy(&blob[2], &size, sizeof(size));
}


std::vector<uint8_t> Random_Bytes(unsigned count)
{
	std::vector<uint8_t> bytes(count);
	for (unsigned i = 0; i < count; i++) {
		bytes[i] = (uint8_t)(Next_Random() & 0xFF);
	}
	return(bytes);
}


int16_t Widen(unsigned char value)
{
	return((int16_t)(((int)value - 128) << 8));
}


// Decodes the same chunks straight through the codec, keeping one stream state
// across them, and converts the output the way the reader does.
std::vector<int16_t> Reference_Sos(std::vector<std::vector<uint8_t>> const & chunks, std::vector<unsigned> const & sizes, unsigned channels, unsigned bits, int totalcomp)
{
	SosCompressInfo info;
	std::memset(&info, 0, sizeof(info));
	info.ChannelCount = (short)channels;
	info.BitSize = (short)bits;
	info.CompSize = totalcomp;
	info.UnCompSize = totalcomp * (bits / 4);
	bool fast = (bits == 16 && channels == 1);
	if (fast) {
		sosCODECInitStream(&info);
	} else {
		General_sosCODECInitStream(&info);
	}

	std::vector<int16_t> out;
	std::vector<unsigned char> native(AUD_MAX_CHUNK_UNCOMP_BYTES);
	for (size_t i = 0; i < chunks.size(); i++) {
		info.Source = (char *)chunks[i].data();
		info.Dest = (char *)native.data();
		if (fast) {
			sosCODECDecompressData(&info, sizes[i]);
		} else {
			General_sosCODECDecompressData(&info, sizes[i]);
		}
		if (bits == 16) {
			int16_t const * samples = (int16_t const *)native.data();
			out.insert(out.end(), samples, samples + sizes[i] / 2);
		} else {
			for (unsigned b = 0; b < sizes[i]; b++) {
				out.push_back(Widen(native[b]));
			}
		}
	}
	return(out);
}


void Test_Sos(unsigned channels, unsigned bits, char const * label)
{
	unsigned ratio = bits / 4;
	std::vector<std::vector<uint8_t>> chunks;
	std::vector<unsigned> sizes;
	int totalcomp = 0;
	for (int i = 0; i < 6; i++) {
		unsigned comp = 256 + (Next_Random() % 400);
		comp -= comp % 4;
		chunks.push_back(Random_Bytes(comp));
		sizes.push_back(comp * ratio);
		totalcomp += comp;
	}

	std::vector<uint8_t> blob = Make_Header(22050, (channels == 2 ? AUD_FLAG_STEREO : 0) | (bits == 16 ? AUD_FLAG_16BIT : 0), AUD_CODEC_SOS, 0, 0);
	for (size_t i = 0; i < chunks.size(); i++) {
		Append_Chunk(blob, chunks[i], sizes[i]);
	}
	Patch_Size(blob);
	int32_t uncomp = (int32_t)(totalcomp * ratio);
	std::memcpy(&blob[6], &uncomp, sizeof(uncomp));

	std::vector<int16_t> expect = Reference_Sos(chunks, sizes, channels, bits, totalcomp);

	AUDHeaderType header;
	Check(Aud_Read_Header(blob.data(), blob.size(), header), label);
	unsigned expectframes = (unsigned)(expect.size() / channels);
	unsigned capacity = Aud_Frame_Capacity(header);
	Check(capacity >= expectframes, "frame capacity is an upper bound on the decoded size");
	Check(capacity <= expectframes + (unsigned)(chunks.size() * sizeof(AUDChunkHeaderType)) * 4, "frame capacity overshoots by no more than the chunk headers");

	std::vector<int16_t> out(capacity * channels + 16, (int16_t)0x5A5A);
	AudioPcmFormat format;
	unsigned frames = Aud_Decode(blob.data(), blob.size(), out.data(), capacity, format);
	Check(frames == expectframes, "every chunk decodes");
	Check(format.Rate == 22050 && format.Channels == channels, "format reported");
	Check(std::memcmp(out.data(), expect.data(), expect.size() * sizeof(int16_t)) == 0, "reader output equals the codec's own output");
	Check(out[expect.size()] == (int16_t)0x5A5A, "reader writes only the decoded frames");

	// A frame-sized decode must produce the same bytes chunk by chunk.
	AudChunkDecoderClass decoder;
	decoder.Init(header);
	std::vector<int16_t> piecewise;
	std::vector<int16_t> piece(decoder.Max_Chunk_Frames() * channels);
	for (size_t i = 0; i < chunks.size(); i++) {
		unsigned produced = decoder.Decode_Chunk(chunks[i].data(), (unsigned)chunks[i].size(), sizes[i], piece.data(), decoder.Max_Chunk_Frames());
		Check(produced == sizes[i] / (channels * bits / 8), "chunk decoder reports the chunk's frames");
		piecewise.insert(piecewise.end(), piece.begin(), piece.begin() + (size_t)produced * channels);
	}
	Check(piecewise == expect, "chunk-at-a-time decode equals the whole decode");

	// Rewinding restarts the codec so the first chunk decodes the same again.
	decoder.Rewind();
	unsigned produced = decoder.Decode_Chunk(chunks[0].data(), (unsigned)chunks[0].size(), sizes[0], piece.data(), decoder.Max_Chunk_Frames());
	Check(produced > 0 && std::memcmp(piece.data(), expect.data(), (size_t)produced * channels * sizeof(int16_t)) == 0, "rewind restarts the stream");
}


void Test_Sos_Uncompressed_Chunk(void)
{
	std::vector<uint8_t> payload = Random_Bytes(512);
	std::vector<uint8_t> blob = Make_Header(22050, AUD_FLAG_16BIT, AUD_CODEC_SOS, 0, 0);
	Append_Chunk(blob, payload, 512);
	Patch_Size(blob);

	std::vector<int16_t> out(512);
	AudioPcmFormat format;
	unsigned frames = Aud_Decode(blob.data(), blob.size(), out.data(), 512, format);
	Check(frames == 256, "uncompressed chunk yields its frames");
	Check(std::memcmp(out.data(), payload.data(), 512) == 0, "uncompressed chunk is copied as is");
}


void Test_Raw(void)
{
	std::vector<uint8_t> blob = Make_Header(11025, 0, AUD_CODEC_PCM, 0, 0);
	uint8_t samples[6] = {0, 128, 255, 64, 192, 1};
	Append(blob, samples, sizeof(samples));
	Patch_Size(blob);

	std::vector<int16_t> out(16, (int16_t)0x7777);
	AudioPcmFormat format;
	unsigned frames = Aud_Decode(blob.data(), blob.size(), out.data(), 16, format);
	Check(frames == 6, "raw 8-bit frames counted");
	Check(format.Rate == 11025 && format.Channels == 1, "raw 8-bit format");
	Check(out[0] == -32768 && out[1] == 0 && out[2] == 32512 && out[3] == -16384, "8-bit widens around silence");
	Check(out[6] == (int16_t)0x7777, "raw decode stays inside its frames");

	std::vector<uint8_t> stereo = Make_Header(44100, AUD_FLAG_STEREO | AUD_FLAG_16BIT, AUD_CODEC_PCM, 0, 0);
	int16_t pairs[8] = {1, -1, 2, -2, 3, -3, 4, -4};
	Append(stereo, pairs, sizeof(pairs));
	Patch_Size(stereo);
	frames = Aud_Decode(stereo.data(), stereo.size(), out.data(), 16, format);
	Check(frames == 4 && format.Channels == 2 && format.Rate == 44100, "raw 16-bit stereo frames");
	Check(std::memcmp(out.data(), pairs, sizeof(pairs)) == 0, "16-bit samples pass through");

	frames = Aud_Decode(stereo.data(), stereo.size(), out.data(), 2, format);
	Check(frames == 2, "raw decode clamps to the capacity");
}


void Test_Westwood(void)
{
	// silence x4, raw 10 20 30, four 2-bit deltas, two 4-bit deltas, one embedded 5-bit delta.
	uint8_t stream[] = {
		0xC3,
		0x82, 10, 20, 30,
		0x00, 0x4E,
		0x40, 0x8F,
		0xBE
	};
	unsigned char expect[] = {128, 128, 128, 128, 10, 20, 30, 30, 31, 29, 28, 36, 36, 34};
	unsigned char out[32];

	Check(Aud_Decode_Westwood(stream, sizeof(stream), out, sizeof(expect)), "hand-built delta stream decodes");
	Check(std::memcmp(out, expect, sizeof(expect)) == 0, "delta stream produces the expected samples");

	uint8_t clamp[] = {0x80, 0x01, 0x00, 0x00};
	unsigned char clampexpect[] = {1, 0, 0, 0, 0};
	Check(Aud_Decode_Westwood(clamp, sizeof(clamp), out, sizeof(clampexpect)), "clamping stream decodes");
	Check(std::memcmp(out, clampexpect, sizeof(clampexpect)) == 0, "deltas clamp at zero");

	Check(!Aud_Decode_Westwood(stream, sizeof(stream), out, sizeof(expect) + 1), "asking for more output than the stream holds fails");
	Check(!Aud_Decode_Westwood(stream, 3, out, sizeof(expect)), "a truncated stream fails");
	uint8_t overrun[] = {0xBF};
	Check(!Aud_Decode_Westwood(overrun, sizeof(overrun), out, 4), "a run past the output fails");

	// Through the reader, as a chunked codec 1 file.
	std::vector<uint8_t> blob = Make_Header(22050, 0, AUD_CODEC_WESTWOOD, 0, 0);
	Append_Chunk(blob, std::vector<uint8_t>(stream, stream + sizeof(stream)), sizeof(expect));
	Append_Chunk(blob, std::vector<uint8_t>(clamp, clamp + sizeof(clamp)), sizeof(clampexpect));
	Patch_Size(blob);
	int32_t uncomp = sizeof(expect) + sizeof(clampexpect);
	std::memcpy(&blob[6], &uncomp, sizeof(uncomp));

	std::vector<int16_t> pcm(64);
	AudioPcmFormat format;
	unsigned frames = Aud_Decode(blob.data(), blob.size(), pcm.data(), 64, format);
	Check(frames == (unsigned)uncomp, "codec 1 file decodes every chunk");
	bool match = true;
	for (unsigned i = 0; i < sizeof(expect); i++) {
		if (pcm[i] != Widen(expect[i])) match = false;
	}
	for (unsigned i = 0; i < sizeof(clampexpect); i++) {
		if (pcm[sizeof(expect) + i] != Widen(clampexpect[i])) match = false;
	}
	Check(match, "codec 1 samples widen like raw 8-bit ones");
}


void Test_Header_Rules(void)
{
	AUDHeaderType header;
	std::vector<uint8_t> blob = Make_Header(22000, AUD_FLAG_16BIT, AUD_CODEC_SOS, 100, 400);
	blob.resize(sizeof(AUDHeaderType) + 100);
	Check(Aud_Read_Header(blob.data(), blob.size(), header), "valid header reads");
	Check(Aud_Playback_Rate(header) == 22050, "rates just under 22050 play at 22050");
	header.Rate = 23999;
	Check(Aud_Playback_Rate(header) == 22050, "rates just over 22050 play at 22050");
	header.Rate = 44100;
	Check(Aud_Playback_Rate(header) == 44100, "44100 is left alone");
	header.Rate = 20000;
	Check(Aud_Playback_Rate(header) == 20000, "20000 is left alone");
	Check(Aud_Blob_Bytes(header) == sizeof(AUDHeaderType) + 100, "blob size follows the header");
	Check(Aud_Frame_Capacity(header) == 200, "SOS 16-bit capacity is four samples per byte");

	Check(!Aud_Read_Header(blob.data(), blob.size() - 1, header), "a blob shorter than its header claims is rejected");
	Check(Aud_Read_Header(blob.data(), 0, header), "size zero trusts the header");

	std::vector<uint8_t> bad = Make_Header(22050, AUD_FLAG_16BIT, 33, 100, 400);
	Check(!Aud_Read_Header(bad.data(), 0, header), "unknown codec is rejected");
	bad = Make_Header(22050, 4, AUD_CODEC_SOS, 100, 400);
	Check(!Aud_Read_Header(bad.data(), 0, header), "unknown flag bits are rejected");
	bad = Make_Header(22050, AUD_FLAG_16BIT, AUD_CODEC_WESTWOOD, 100, 400);
	Check(!Aud_Read_Header(bad.data(), 0, header), "16-bit delta files are rejected");
	bad = Make_Header(22050, 0, AUD_CODEC_PCM, 0, 0);
	Check(!Aud_Read_Header(bad.data(), 0, header), "an empty file is rejected");
	Check(!Aud_Read_Header(nullptr, 0, header), "null data is rejected");
	Check(!Aud_Read_Header(bad.data(), 4, header), "a fragment shorter than a header is rejected");
}


void Test_Bad_Chunks(void)
{
	std::vector<uint8_t> good = Random_Bytes(256);
	std::vector<uint8_t> blob = Make_Header(22050, AUD_FLAG_16BIT, AUD_CODEC_SOS, 0, 0);
	Append_Chunk(blob, good, 1024);
	Append_Chunk(blob, good, 1024);
	Append_Chunk(blob, good, 1024, 0xBEEF);
	Append_Chunk(blob, good, 1024);
	Patch_Size(blob);

	std::vector<int16_t> out(4096, (int16_t)0x1111);
	AudioPcmFormat format;
	unsigned frames = Aud_Decode(blob.data(), blob.size(), out.data(), 4096, format);
	Check(frames == 1024, "a bad magic ends the sample after the good chunks");
	Check(out[1024] == (int16_t)0x1111, "nothing is written past the good chunks");

	blob = Make_Header(22050, AUD_FLAG_16BIT, AUD_CODEC_SOS, 0, 0);
	Append_Chunk(blob, good, 1024);
	Append_Chunk(blob, good, AUD_MAX_CHUNK_UNCOMP_BYTES + 4);
	Patch_Size(blob);
	frames = Aud_Decode(blob.data(), blob.size(), out.data(), 4096, format);
	Check(frames == 512, "an oversize chunk ends the sample");

	std::vector<uint8_t> big = Random_Bytes(AUD_MAX_CHUNK_COMP_BYTES + 4);
	blob = Make_Header(22050, AUD_FLAG_16BIT, AUD_CODEC_SOS, 0, 0);
	Append_Chunk(blob, big, 2048);
	Patch_Size(blob);
	frames = Aud_Decode(blob.data(), blob.size(), out.data(), 4096, format);
	Check(frames == 0, "an oversize compressed chunk decodes nothing");

	blob = Make_Header(22050, AUD_FLAG_16BIT, AUD_CODEC_SOS, 0, 0);
	Append_Chunk(blob, good, 1024);
	Append_Chunk(blob, good, 1024);
	Patch_Size(blob);
	blob.resize(blob.size() - 100);
	frames = Aud_Decode(blob.data(), blob.size(), out.data(), 4096, format);
	Check(frames == 0, "a blob shorter than its header claims decodes nothing");

	// The header is honest about the blob, but the last chunk claims more than is left.
	blob = Make_Header(22050, AUD_FLAG_16BIT, AUD_CODEC_SOS, 0, 0);
	Append_Chunk(blob, good, 1024);
	size_t lastchunk = blob.size();
	Append_Chunk(blob, good, 1024);
	Patch_Size(blob);
	uint16_t claimed = 5000;
	std::memcpy(&blob[lastchunk], &claimed, sizeof(claimed));
	frames = Aud_Decode(blob.data(), blob.size(), out.data(), 4096, format);
	Check(frames == 512, "a chunk claiming more than the blob holds ends the sample");

	blob = Make_Header(22050, AUD_FLAG_16BIT, AUD_CODEC_SOS, 0, 0);
	Append_Chunk(blob, good, 1024);
	Append_Chunk(blob, good, 1024);
	Patch_Size(blob);
	frames = Aud_Decode(blob.data(), blob.size(), out.data(), 700, format);
	Check(frames == 512, "a chunk that does not fit the capacity is not started");
}


void Test_Wav(void)
{
	int16_t samples[100];
	for (int i = 0; i < 100; i++) {
		samples[i] = (int16_t)(i * 300 - 15000);
	}
	std::vector<uint8_t> wav;
	uint32_t datasize = sizeof(samples);
	uint32_t riffsize = 36 + datasize;
	Append(wav, "RIFF", 4);
	Append(wav, &riffsize, 4);
	Append(wav, "WAVEfmt ", 8);
	uint32_t fmtsize = 16;
	uint16_t format = 1;
	uint16_t channels = 1;
	uint32_t rate = 22050;
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
	Append(wav, samples, sizeof(samples));

	std::vector<int16_t> out;
	AudioPcmFormat info;
	Check(Audio_Decode_Other(wav.data(), wav.size(), out, info), "WAV decodes through miniaudio");
	Check(info.Rate == 22050 && info.Channels == 1, "WAV format reported");
	Check(out.size() == 100 && std::memcmp(out.data(), samples, sizeof(samples)) == 0, "WAV samples pass through");

	std::vector<uint8_t> junk = Random_Bytes(300);
	Check(!Audio_Decode_Other(junk.data(), junk.size(), out, info), "random bytes are not a known format");
}

} // namespace


int main(void)
{
	Test_Sos(1, 16, "SOS 16-bit mono header reads");
	Test_Sos(2, 16, "SOS 16-bit stereo header reads");
	Test_Sos(1, 8, "SOS 8-bit mono header reads");
	Test_Sos(2, 8, "SOS 8-bit stereo header reads");
	Test_Sos_Uncompressed_Chunk();
	Test_Raw();
	Test_Westwood();
	Test_Header_Rules();
	Test_Bad_Chunks();
	Test_Wav();

	std::printf("auddecode: %d checks, %d failures\n", Checked, Failures);
	return(Failures == 0 ? 0 : 1);
}
