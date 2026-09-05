/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Decoding of Westwood AUD files and, through miniaudio, of the WAV, Ogg
// Vorbis, FLAC and MP3 files a mod may supply instead. Everything decodes to
// interleaved 16-bit PCM at the source's own rate; the mixer resamples.

#pragma once

#include "soscomp.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>


#define AUD_FLAG_STEREO 1
#define AUD_FLAG_16BIT 2

#pragma pack(push, 1)
struct AUDHeaderType {
	uint16_t Rate;
	int32_t Size;
	int32_t UncompSize;
	uint8_t Flags;
	uint8_t Compression;
};

struct AUDChunkHeaderType {
	uint16_t CompSize;
	uint16_t UncompSize;
	uint32_t Magic;
};
#pragma pack(pop)


enum AudCodecType : uint8_t {
	AUD_CODEC_PCM = 0,
	AUD_CODEC_WESTWOOD = 1,
	AUD_CODEC_SOS = 99
};

uint32_t const AUD_CHUNK_MAGIC = 0x0000DEAF;

// Chunk limits carried over from the DirectSound driver's staging area, which the
// shipped files were authored against.
unsigned const AUD_MAX_CHUNK_COMP_BYTES = 2048 + 50;
unsigned const AUD_MAX_CHUNK_UNCOMP_BYTES = AUD_MAX_CHUNK_COMP_BYTES * 4;


struct AudioPcmFormat {
	unsigned Rate;
	unsigned Channels;
};


// Reads and validates a header. A size of zero means the caller does not know
// how large the blob is and trusts the header.
bool Aud_Read_Header(void const * data, size_t size, AUDHeaderType & header);

// The rate the sample plays at. Rates between 20000 and 24000 play at 22050,
// as they always have.
unsigned Aud_Playback_Rate(AUDHeaderType const & header);

unsigned Aud_Channels(AUDHeaderType const & header);
unsigned Aud_Bits(AUDHeaderType const & header);
size_t Aud_Blob_Bytes(AUDHeaderType const & header);

// Upper bound on the frames the sample decodes to.
unsigned Aud_Frame_Capacity(AUDHeaderType const & header);


// Decodes one AUD's chunks in order, keeping the codec state between them, so
// a stream can decode a chunk at a time and a clip can decode all at once.
class AudChunkDecoderClass
{
	public:
		AudChunkDecoderClass(void);

		bool Init(AUDHeaderType const & header);
		void Rewind(void);

		unsigned Rate(void) const { return(Aud_Playback_Rate(Header)); }
		unsigned Channels(void) const { return(ChannelCount); }
		bool Is_Chunked(void) const { return(Header.Compression != AUD_CODEC_PCM); }

		// Decodes one chunk's payload into frames; returns the frames produced, or
		// zero for a chunk that is malformed or does not fit.
		unsigned Decode_Chunk(void const * payload, unsigned compsize, unsigned uncompsize, int16_t * output, unsigned capacity);

		// Converts raw (codec 0) sample bytes, which have no chunk framing.
		unsigned Convert_Raw(void const * data, size_t bytes, int16_t * output, unsigned capacity);

		unsigned Max_Chunk_Frames(void) const;

	private:
		unsigned Convert(void const * native, unsigned bytes, int16_t * output, unsigned capacity) const;

		AUDHeaderType Header;
		SosCompressInfo Sos;
		unsigned ChannelCount;
		unsigned BitSize;
		unsigned char Native[AUD_MAX_CHUNK_UNCOMP_BYTES];
};


// Decodes a whole AUD blob. Returns the frames produced, which may be fewer than
// the capacity when the data ends early or a chunk is bad, or zero on failure.
unsigned Aud_Decode(void const * data, size_t size, int16_t * output, unsigned capacity, AudioPcmFormat & format);

// The bounds-checked Westwood delta decoder (codec 1). Returns false if the
// compressed data does not produce exactly the requested bytes.
bool Aud_Decode_Westwood(void const * source, unsigned compsize, unsigned char * dest, unsigned uncompsize);

// Decodes any format miniaudio supports from memory. Returns false for data it
// does not recognise.
bool Audio_Decode_Other(void const * data, size_t size, std::vector<int16_t> & output, AudioPcmFormat & format);


// Sequential access to a file's bytes for the streaming decoders. The engine
// wraps the game's file layer; tests wrap memory.
class AudioByteSourceClass
{
	public:
		virtual ~AudioByteSourceClass(void) = default;

		virtual size_t Read(void * buffer, size_t bytes) = 0;
		virtual bool Seek(size_t position) = 0;
		virtual size_t Position(void) const = 0;
		virtual size_t Size(void) const = 0;
};


// Streams any format miniaudio supports from a byte source, a few frames at a
// time. The source must outlive the decoder.
class AudioOtherStreamDecoderClass
{
	public:
		AudioOtherStreamDecoderClass(void);
		~AudioOtherStreamDecoderClass(void);

		AudioOtherStreamDecoderClass(AudioOtherStreamDecoderClass const &) = delete;
		AudioOtherStreamDecoderClass & operator=(AudioOtherStreamDecoderClass const &) = delete;

		bool Open(AudioByteSourceClass & source);
		void Close(void);
		bool Is_Open(void) const { return(Data != nullptr); }

		unsigned Rate(void) const;
		unsigned Channels(void) const;

		// Returns the frames read; zero at the end of the data.
		unsigned Read(int16_t * output, unsigned frames);
		bool Rewind(void);

	private:
		struct DataClass;
		std::unique_ptr<DataClass> Data;
};
