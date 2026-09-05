/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "audio/audiodecode.h"

#include "miniaudio.h"

#include <cstring>


bool Aud_Read_Header(void const * data, size_t size, AUDHeaderType & header)
{
	if (data == nullptr || (size != 0 && size < sizeof(AUDHeaderType))) {
		return(false);
	}
	std::memcpy(&header, data, sizeof(header));

	if (header.Rate == 0 || header.Size <= 0) {
		return(false);
	}
	if ((header.Flags & ~(AUD_FLAG_STEREO | AUD_FLAG_16BIT)) != 0) {
		return(false);
	}
	if (header.Compression != AUD_CODEC_PCM && header.Compression != AUD_CODEC_WESTWOOD && header.Compression != AUD_CODEC_SOS) {
		return(false);
	}
	// The delta codec only ever produced 8-bit output.
	if (header.Compression == AUD_CODEC_WESTWOOD && (header.Flags & AUD_FLAG_16BIT) != 0) {
		return(false);
	}
	if (size != 0 && Aud_Blob_Bytes(header) > size) {
		return(false);
	}
	return(true);
}


unsigned Aud_Playback_Rate(AUDHeaderType const & header)
{
	if (header.Rate < 24000 && header.Rate > 20000) {
		return(22050);
	}
	return(header.Rate);
}


unsigned Aud_Channels(AUDHeaderType const & header)
{
	return((header.Flags & AUD_FLAG_STEREO) ? 2 : 1);
}


unsigned Aud_Bits(AUDHeaderType const & header)
{
	return((header.Flags & AUD_FLAG_16BIT) ? 16 : 8);
}


size_t Aud_Blob_Bytes(AUDHeaderType const & header)
{
	return(sizeof(AUDHeaderType) + (size_t)header.Size);
}


unsigned Aud_Frame_Capacity(AUDHeaderType const & header)
{
	size_t bytes;
	switch (header.Compression) {
		case AUD_CODEC_SOS:
			// Four bits per sample for 16-bit data, two for 8-bit; the header's own
			// figure is used when it is at least that large.
			bytes = (size_t)header.Size * (Aud_Bits(header) / 4);
			if (header.UncompSize > 0 && (size_t)header.UncompSize > bytes) {
				bytes = (size_t)header.UncompSize;
			}
			break;

		case AUD_CODEC_WESTWOOD:
			bytes = (header.UncompSize > 0) ? (size_t)header.UncompSize : (size_t)header.Size * 4;
			break;

		default:
			bytes = (size_t)header.Size;
			break;
	}
	size_t framebytes = (size_t)Aud_Channels(header) * (Aud_Bits(header) / 8);
	return((unsigned)(bytes / framebytes));
}


AudChunkDecoderClass::AudChunkDecoderClass(void) :
	ChannelCount(0),
	BitSize(0)
{
	std::memset(&Header, 0, sizeof(Header));
	std::memset(&Sos, 0, sizeof(Sos));
}


bool AudChunkDecoderClass::Init(AUDHeaderType const & header)
{
	Header = header;
	ChannelCount = Aud_Channels(header);
	BitSize = Aud_Bits(header);
	Rewind();
	return(true);
}


void AudChunkDecoderClass::Rewind(void)
{
	if (Header.Compression == AUD_CODEC_SOS) {
		std::memset(&Sos, 0, sizeof(Sos));
		Sos.ChannelCount = (short)ChannelCount;
		Sos.BitSize = (short)BitSize;
		Sos.CompSize = (uint32_t)Header.Size;
		Sos.UnCompSize = (uint32_t)Header.Size * (BitSize / 4);
		if (BitSize == 16 && ChannelCount == 1) {
			sosCODECInitStream(&Sos);
		} else {
			General_sosCODECInitStream(&Sos);
		}
	}
}


unsigned AudChunkDecoderClass::Max_Chunk_Frames(void) const
{
	return(AUD_MAX_CHUNK_UNCOMP_BYTES / (ChannelCount * (BitSize / 8)));
}


unsigned AudChunkDecoderClass::Decode_Chunk(void const * payload, unsigned compsize, unsigned uncompsize, int16_t * output, unsigned capacity)
{
	if (ChannelCount == 0 || payload == nullptr || uncompsize == 0) {
		return(0);
	}
	unsigned framebytes = ChannelCount * (BitSize / 8);
	if (uncompsize > AUD_MAX_CHUNK_UNCOMP_BYTES || uncompsize % framebytes != 0 || uncompsize / framebytes > capacity) {
		return(0);
	}

	switch (Header.Compression) {
		case AUD_CODEC_SOS:
			if (compsize == uncompsize) {
				return(Convert(payload, uncompsize, output, capacity));
			}
			if (compsize > AUD_MAX_CHUNK_COMP_BYTES) {
				return(0);
			}
			Sos.Source = (char *)payload;
			Sos.Dest = (char *)Native;
			if (BitSize == 16 && ChannelCount == 1) {
				sosCODECDecompressData(&Sos, uncompsize);
			} else {
				General_sosCODECDecompressData(&Sos, uncompsize);
			}
			return(Convert(Native, uncompsize, output, capacity));

		case AUD_CODEC_WESTWOOD:
			if (compsize == uncompsize) {
				return(Convert(payload, uncompsize, output, capacity));
			}
			if (!Aud_Decode_Westwood(payload, compsize, Native, uncompsize)) {
				return(0);
			}
			return(Convert(Native, uncompsize, output, capacity));

		default:
			return(0);
	}
}


unsigned AudChunkDecoderClass::Convert_Raw(void const * data, size_t bytes, int16_t * output, unsigned capacity)
{
	if (ChannelCount == 0 || data == nullptr) {
		return(0);
	}
	unsigned framebytes = ChannelCount * (BitSize / 8);
	size_t frames = bytes / framebytes;
	if (frames > capacity) {
		frames = capacity;
	}
	return(Convert(data, (unsigned)(frames * framebytes), output, capacity));
}


unsigned AudChunkDecoderClass::Convert(void const * native, unsigned bytes, int16_t * output, unsigned capacity) const
{
	unsigned framebytes = ChannelCount * (BitSize / 8);
	unsigned frames = bytes / framebytes;
	if (frames > capacity) {
		frames = capacity;
	}
	unsigned samples = frames * ChannelCount;
	if (BitSize == 16) {
		std::memcpy(output, native, (size_t)samples * sizeof(int16_t));
	} else {
		unsigned char const * source = (unsigned char const *)native;
		for (unsigned i = 0; i < samples; i++) {
			output[i] = (int16_t)(((int)source[i] - 128) << 8);
		}
	}
	return(frames);
}


unsigned Aud_Decode(void const * data, size_t size, int16_t * output, unsigned capacity, AudioPcmFormat & format)
{
	AUDHeaderType header;
	if (!Aud_Read_Header(data, size, header) || output == nullptr || capacity == 0) {
		return(0);
	}

	AudChunkDecoderClass decoder;
	decoder.Init(header);
	format.Rate = decoder.Rate();
	format.Channels = decoder.Channels();

	unsigned char const * cursor = (unsigned char const *)data + sizeof(AUDHeaderType);
	size_t remaining = (size_t)header.Size;
	if (size != 0 && size - sizeof(AUDHeaderType) < remaining) {
		remaining = size - sizeof(AUDHeaderType);
	}

	if (!decoder.Is_Chunked()) {
		return(decoder.Convert_Raw(cursor, remaining, output, capacity));
	}

	unsigned frames = 0;
	while (remaining >= sizeof(AUDChunkHeaderType) && frames < capacity) {
		AUDChunkHeaderType chunk;
		std::memcpy(&chunk, cursor, sizeof(chunk));
		cursor += sizeof(chunk);
		remaining -= sizeof(chunk);

		if (chunk.Magic != AUD_CHUNK_MAGIC || chunk.CompSize > remaining) {
			break;
		}
		unsigned produced = decoder.Decode_Chunk(cursor, chunk.CompSize, chunk.UncompSize, output + (size_t)frames * format.Channels, capacity - frames);
		if (produced == 0) {
			break;
		}
		frames += produced;
		cursor += chunk.CompSize;
		remaining -= chunk.CompSize;
	}
	return(frames);
}


// The delta codes and their tables are the ones the VQA player's AudioUnzap
// uses; this copy checks every read and write against the chunk bounds.
bool Aud_Decode_Westwood(void const * source, unsigned compsize, unsigned char * dest, unsigned uncompsize)
{
	static signed char const DELTA_2BIT[4] = {-2, -1, 0, 1};
	static signed char const DELTA_4BIT[16] = {-9, -8, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 8};
	enum { CODE_2BIT, CODE_4BIT, CODE_RAW, CODE_SILENCE };

	unsigned char const * s = (unsigned char const *)source;
	unsigned in = 0;
	unsigned out = 0;
	int previous = 0x80;

	auto emit = [&](int value) {
		if (value < 0) value = 0;
		if (value > 255) value = 255;
		previous = value;
		dest[out++] = (unsigned char)value;
	};

	while (out < uncompsize) {
		if (in >= compsize) {
			return(false);
		}
		unsigned char code = s[in++];
		unsigned counter = (code & 0x3F) + 1;

		switch ((code >> 6) & 0x03) {
			case CODE_RAW:
				if ((counter - 1) & 0x20) {
					int delta = (int)((counter - 1) & 0x1F);
					if (delta & 0x10) delta -= 0x20;
					emit(previous + delta);
				} else {
					if (in + counter > compsize || out + counter > uncompsize) {
						return(false);
					}
					std::memcpy(dest + out, s + in, counter);
					in += counter;
					out += counter;
					previous = dest[out - 1];
				}
				break;

			case CODE_4BIT:
				if (in + counter > compsize || out + counter * 2 > uncompsize) {
					return(false);
				}
				while (counter--) {
					unsigned char delta = s[in++];
					emit(previous + DELTA_4BIT[delta & 0x0F]);
					emit(previous + DELTA_4BIT[(delta >> 4) & 0x0F]);
				}
				break;

			case CODE_2BIT:
				if (in + counter > compsize || out + counter * 4 > uncompsize) {
					return(false);
				}
				while (counter--) {
					unsigned char delta = s[in++];
					emit(previous + DELTA_2BIT[delta & 0x03]);
					emit(previous + DELTA_2BIT[(delta >> 2) & 0x03]);
					emit(previous + DELTA_2BIT[(delta >> 4) & 0x03]);
					emit(previous + DELTA_2BIT[(delta >> 6) & 0x03]);
				}
				break;

			default:
				if (out + counter > uncompsize) {
					return(false);
				}
				std::memset(dest + out, previous, counter);
				out += counter;
				break;
		}
	}
	return(out == uncompsize);
}


bool Audio_Decode_Other(void const * data, size_t size, std::vector<int16_t> & output, AudioPcmFormat & format)
{
	output.clear();
	if (data == nullptr || size == 0) {
		return(false);
	}

	ma_decoder_config config = ma_decoder_config_init(ma_format_s16, 0, 0);
	ma_decoder decoder;
	if (ma_decoder_init_memory(data, size, &config, &decoder) != MA_SUCCESS) {
		return(false);
	}

	ma_format outformat;
	ma_uint32 channels = 0;
	ma_uint32 rate = 0;
	if (ma_decoder_get_data_format(&decoder, &outformat, &channels, &rate, nullptr, 0) != MA_SUCCESS || channels == 0 || channels > 2 || rate == 0) {
		ma_decoder_uninit(&decoder);
		return(false);
	}

	// Vorbis reports no length, so everything is read in pieces.
	unsigned const PIECE = 4096;
	std::vector<int16_t> piece((size_t)PIECE * channels);
	for (;;) {
		ma_uint64 read = 0;
		ma_result result = ma_decoder_read_pcm_frames(&decoder, piece.data(), PIECE, &read);
		if (read > 0) {
			output.insert(output.end(), piece.begin(), piece.begin() + (size_t)read * channels);
		}
		if (result != MA_SUCCESS || read < PIECE) {
			break;
		}
	}
	ma_decoder_uninit(&decoder);

	format.Rate = rate;
	format.Channels = channels;
	return(!output.empty());
}


struct AudioOtherStreamDecoderClass::DataClass {
	ma_decoder Decoder;
	AudioByteSourceClass * Source;
	unsigned Rate;
	unsigned Channels;

	static ma_result Read_Proc(ma_decoder * decoder, void * buffer, size_t bytes, size_t * read)
	{
		DataClass * data = (DataClass *)decoder->pUserData;
		*read = data->Source->Read(buffer, bytes);
		return(*read == 0 && bytes > 0 ? MA_AT_END : MA_SUCCESS);
	}

	static ma_result Seek_Proc(ma_decoder * decoder, ma_int64 offset, ma_seek_origin origin)
	{
		DataClass * data = (DataClass *)decoder->pUserData;
		ma_int64 target;
		switch (origin) {
			case ma_seek_origin_start:
				target = offset;
				break;
			case ma_seek_origin_current:
				target = (ma_int64)data->Source->Position() + offset;
				break;
			default:
				target = (ma_int64)data->Source->Size() + offset;
				break;
		}
		if (target < 0 || (size_t)target > data->Source->Size()) {
			return(MA_BAD_SEEK);
		}
		return(data->Source->Seek((size_t)target) ? MA_SUCCESS : MA_BAD_SEEK);
	}
};


AudioOtherStreamDecoderClass::AudioOtherStreamDecoderClass(void)
{
}


AudioOtherStreamDecoderClass::~AudioOtherStreamDecoderClass(void)
{
	Close();
}


bool AudioOtherStreamDecoderClass::Open(AudioByteSourceClass & source)
{
	Close();
	std::unique_ptr<DataClass> data(new (std::nothrow) DataClass());
	if (data == nullptr) {
		return(false);
	}
	data->Source = &source;
	data->Rate = 0;
	data->Channels = 0;

	ma_decoder_config config = ma_decoder_config_init(ma_format_s16, 0, 0);
	if (ma_decoder_init(DataClass::Read_Proc, DataClass::Seek_Proc, data.get(), &config, &data->Decoder) != MA_SUCCESS) {
		return(false);
	}

	ma_format format;
	ma_uint32 channels = 0;
	ma_uint32 rate = 0;
	if (ma_decoder_get_data_format(&data->Decoder, &format, &channels, &rate, nullptr, 0) != MA_SUCCESS || channels == 0 || channels > 2 || rate == 0) {
		ma_decoder_uninit(&data->Decoder);
		return(false);
	}
	data->Rate = rate;
	data->Channels = channels;
	Data = std::move(data);
	return(true);
}


void AudioOtherStreamDecoderClass::Close(void)
{
	if (Data != nullptr) {
		ma_decoder_uninit(&Data->Decoder);
		Data.reset();
	}
}


unsigned AudioOtherStreamDecoderClass::Rate(void) const
{
	return(Data != nullptr ? Data->Rate : 0);
}


unsigned AudioOtherStreamDecoderClass::Channels(void) const
{
	return(Data != nullptr ? Data->Channels : 0);
}


unsigned AudioOtherStreamDecoderClass::Read(int16_t * output, unsigned frames)
{
	if (Data == nullptr || frames == 0) {
		return(0);
	}
	ma_uint64 read = 0;
	ma_decoder_read_pcm_frames(&Data->Decoder, output, frames, &read);
	return((unsigned)read);
}


bool AudioOtherStreamDecoderClass::Rewind(void)
{
	return(Data != nullptr && ma_decoder_seek_to_pcm_frame(&Data->Decoder, 0) == MA_SUCCESS);
}
