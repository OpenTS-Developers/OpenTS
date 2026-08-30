/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

#include "always.h"

#include <cstdint>

#include "soscomp.h"
#include "soscodec_tables.h"
#include "vqalib/cmp.h"

/*
 * Three decoders share this file, each replacing an assembly routine of the same name.
 *
 * sosCODECDecompressData is the fast path for 16 bit mono. It walks a difference table
 * indexed by the step index and the token together, and its stream state is that combined
 * index rather than a plain step index: wIndex holds the step index times 32, which is the
 * form the table wants and the form the assembly persisted between calls.
 *
 * General_sosCODECDecompressData handles every other shape. It computes the difference
 * arithmetically from the current step instead of reading a table, which is the same
 * arithmetic the table was built from.
 *
 * VQA_sosCODECDecompressData is the VQA flavour. It takes its shape through arguments,
 * carries only four values of state, and implements 16 bit only; anything else returns
 * having done nothing, exactly as the assembly did.
 */

namespace {

/// <summary>
/// Holds a running sample inside the range a 16 bit sample can carry.
/// </summary>
/// <param name="sample">The sample to bring back into range.</param>
/// <returns>int32_t; The sample, clamped.</returns>
inline int32_t Clamp_Sample(int32_t sample)
{
	if (sample > 32767) {
		return(32767);
	}

	if (sample < -32768) {
		return(-32768);
	}

	return(sample);
}


/// <summary>
/// Decodes 16 bit samples through the difference table, the form both the SOS fast path and
/// the VQA decoder use. Tokens come out of each source byte low half first.
/// </summary>
/// <param name="source">Compressed nybbles.</param>
/// <param name="dest">Where the samples go.</param>
/// <param name="samples">How many samples to produce.</param>
/// <param name="deststride">Distance in shorts between one sample and the next.</param>
/// <param name="predicted">Running sample, carried in and out.</param>
/// <param name="index">Step index times 32, carried in and out.</param>
void Decode_Table_16(unsigned char const * source, short * dest, int samples, int deststride, int32_t & predicted, unsigned short & index)
{
	int32_t sample = predicted;
	unsigned int slotbase = index;

	for (int i = 0; i < samples; i++) {
		unsigned int const byte = source[i >> 1];
		unsigned int const token = ((i & 1) == 0) ? (byte & 0x0F) : ((byte >> 4) & 0x0F);

		/*
		 * The index is a multiple of 32 and the token contributes at most 30, so the
		 * assembly's OR into the low byte is an addition that cannot carry.
		 */
		unsigned int const slot = (slotbase | (token * 2)) >> 1;

		sample += _SosDiffTable[slot];
		slotbase = _SosIndexTable[slot];
		sample = Clamp_Sample(sample);

		dest[i * deststride] = (short)sample;
	}

	predicted = sample;
	index = (unsigned short)slotbase;
}


/*
 * The general decoder keeps a separate copy of this state per channel, so the loop below
 * reaches it through pointers rather than naming the structure's fields twice.
 */
struct SosChannel {
	uint32_t * SampleIndex;
	short * CodeBuf;
	short * Code;
	int32_t * Predicted;
	int32_t * Difference;
	short * Index;
	short * Step;
};


/// <summary>
/// Decodes one channel the general way, computing each difference from the current step.
/// </summary>
/// <param name="channel">The stream state for this channel.</param>
/// <param name="source">Compressed nybbles.</param>
/// <param name="dest">Where the samples go.</param>
/// <param name="samples">How many samples to produce.</param>
/// <param name="sourcestride">Distance in bytes between one source byte and the next.</param>
/// <param name="deststride">Distance in bytes between one sample and the next.</param>
/// <param name="bits">8 or 16, the width of a written sample.</param>
void Decode_General(SosChannel const & channel, unsigned char const * source, unsigned char * dest, int samples, int sourcestride, int deststride, int bits)
{
	for (int i = 0; i < samples; i++) {

		/*
		 * A byte carries two tokens. Odd samples take the half already fetched, which is
		 * why the sample counter and the code buffer are stream state rather than locals.
		 */
		if ((*channel.SampleIndex & 1) != 0) {
			*channel.Code = (short)(((unsigned short)*channel.CodeBuf >> 4) & 0x0F);
		} else {
			*channel.CodeBuf = (short)(unsigned short)*source;
			source += sourcestride;
			*channel.Code = (short)(*channel.CodeBuf & 0x0F);
		}

		int const code = *channel.Code;
		int32_t const step = (int32_t)(unsigned short)*channel.Step;

		int32_t difference = 0;

		if ((code & 4) != 0) {
			difference += step;
		}

		if ((code & 2) != 0) {
			difference += step >> 1;
		}

		if ((code & 1) != 0) {
			difference += step >> 2;
		}

		difference += step >> 3;

		if ((code & 8) != 0) {
			difference = -difference;
		}

		*channel.Difference = difference;

		int32_t const sample = Clamp_Sample(*channel.Predicted + difference);
		*channel.Predicted = sample;

		if (bits == 16) {
			*(short *)dest = (short)sample;
		} else {

			/*
			 * An 8 bit stream carries the top half of the sample, biased to unsigned.
			 */
			*dest = (unsigned char)((((uint32_t)sample >> 8) & 0xFF) ^ 0x80);
		}

		dest += deststride;

		/*
		 * The assembly tests the index as unsigned, so an adjustment that takes it below
		 * zero shows up as a very large value and folds back to zero.
		 */
		unsigned short next = (unsigned short)((unsigned short)*channel.Index + (unsigned short)_SosIndexAdjust[code]);

		if (next >= 0x8000) {
			next = 0;
		} else if (next > 88) {
			next = 88;
		}

		*channel.Index = (short)next;
		*channel.Step = (short)_SosStepTable[next];

		(*channel.SampleIndex)++;
	}
}


SosChannel Left_Channel(_SOS_COMPRESS_INFO * info)
{
	SosChannel channel;
	channel.SampleIndex = &info->dwSampleIndex;
	channel.CodeBuf = &info->wCodeBuf;
	channel.Code = &info->wCode;
	channel.Predicted = &info->dwPredicted;
	channel.Difference = &info->dwDifference;
	channel.Index = &info->wIndex;
	channel.Step = &info->wStep;
	return(channel);
}


SosChannel Right_Channel(_SOS_COMPRESS_INFO * info)
{
	SosChannel channel;
	channel.SampleIndex = &info->dwSampleIndex2;
	channel.CodeBuf = &info->wCodeBuf2;
	channel.Code = &info->wCode2;
	channel.Predicted = &info->dwPredicted2;
	channel.Difference = &info->dwDifference2;
	channel.Index = &info->wIndex2;
	channel.Step = &info->wStep2;
	return(channel);
}

}	// namespace


/// <summary>
/// Starts a compression stream, clearing the running sample and step index for both channels.
/// </summary>
/// <param name="info">The stream to initialize.</param>
void __cdecl sosCODECInitStream(_SOS_COMPRESS_INFO * info)
{
	info->wIndex = 0;
	info->dwPredicted = 0;
	info->wIndex2 = 0;
	info->dwPredicted2 = 0;
}


/// <summary>
/// Decompresses 4:1 ADPCM, the specialized path for 16 bit mono. Anything else is left alone,
/// as it was in the assembly, where only this one shape was ever implemented.
/// </summary>
/// <param name="info">Stream state, source and destination.</param>
/// <param name="bytes">How many bytes of samples to produce.</param>
/// <returns>uint32_t; The byte count asked for, or zero if the shape is not handled.</returns>
uint32_t __cdecl sosCODECDecompressData(_SOS_COMPRESS_INFO * info, uint32_t bytes)
{
	if (info->wBitSize != 16 || info->wChannels != 1) {
		return(0);
	}

	unsigned short index = (unsigned short)info->wIndex;

	Decode_Table_16((unsigned char const *)info->lpSource, (short *)info->lpDest, (int)(bytes / 2), 1, info->dwPredicted, index);

	info->wIndex = (short)index;
	return(bytes);
}


/// <summary>
/// Starts a compression stream for the general decoder.
/// </summary>
/// <param name="info">The stream to initialize.</param>
void __cdecl General_sosCODECInitStream(_SOS_COMPRESS_INFO * info)
{
	info->wIndex = 0;
	info->wStep = (short)_SosStepTable[0];
	info->dwPredicted = 0;
	info->dwSampleIndex = 0;
	info->wIndex2 = 0;
	info->wStep2 = (short)_SosStepTable[0];
	info->dwPredicted2 = 0;
	info->dwSampleIndex2 = 0;
}


/// <summary>
/// Decompresses 4:1 ADPCM for any bit size and channel count.
///
/// The assembly counted down and only then tested for zero, so a request for nothing, or an
/// odd number of samples on the stereo path, never reached the test and ran away through both
/// buffers. Those requests now decode nothing instead.
/// </summary>
/// <param name="info">Stream state, source and destination.</param>
/// <param name="bytes">How many bytes of samples to produce.</param>
/// <returns>uint32_t; The byte count asked for.</returns>
uint32_t __cdecl General_sosCODECDecompressData(_SOS_COMPRESS_INFO * info, uint32_t bytes)
{
	info->dwSampleIndex = 0;
	info->dwSampleIndex2 = 0;

	int const bits = info->wBitSize;
	int const samples = (bits == 16) ? (int)(bytes / 2) : (int)bytes;

	if (samples <= 0) {
		return(bytes);
	}

	unsigned char const * source = (unsigned char const *)info->lpSource;
	unsigned char * dest = (unsigned char *)info->lpDest;

	if (info->wChannels == 2) {
		if ((samples % 2) != 0) {
			return(bytes);
		}

		int const perchannel = samples / 2;
		int const sampled = (bits == 16) ? 4 : 2;

		Decode_General(Left_Channel(info), source, dest, perchannel, 2, sampled, bits);
		Decode_General(Right_Channel(info), source + 1, dest + (bits == 16 ? 2 : 1), perchannel, 2, sampled, bits);
	} else {
		Decode_General(Left_Channel(info), source, dest, samples, 1, (bits == 16) ? 2 : 1, bits);
	}

	return(bytes);
}


/// <summary>
/// Starts a VQA compression stream.
/// </summary>
/// <param name="info">The stream to initialize.</param>
void __cdecl VQA_sosCODECInitStream(_VQA_SOS_COMPRESS_INFO * info)
{
	info->wIndex = 0;
	info->dwPredicted = 0;
	info->wIndex2 = 0;
	info->dwPredicted2 = 0;
}


/// <summary>
/// Decompresses 4:1 ADPCM for VQA audio. Only 16 bit is implemented, mono and stereo; an 8 bit
/// request does nothing, which is what the assembly did.
///
/// A stereo stream carries its two channels as consecutive halves of the source rather than
/// interleaved, and writes them interleaved into the destination.
/// </summary>
/// <param name="src">Compressed nybbles.</param>
/// <param name="dst">Where the samples go.</param>
/// <param name="bits">Sample width; only 16 is handled.</param>
/// <param name="channels">1 or 2.</param>
/// <param name="bytes">How many bytes of samples to produce.</param>
/// <param name="info">Stream state carried between calls.</param>
void __cdecl VQA_sosCODECDecompressData(void * src, void * dst, unsigned short bits, unsigned short channels, uint32_t bytes, _VQA_SOS_COMPRESS_INFO * info)
{
	if (bits != 16) {
		return;
	}

	unsigned char const * source = (unsigned char const *)src;
	short * dest = (short *)dst;

	if (channels == 2) {
		int const perchannel = (int)(bytes / 4);
		unsigned short index = (unsigned short)info->wIndex;
		unsigned short index2 = (unsigned short)info->wIndex2;

		Decode_Table_16(source, dest, perchannel, 2, info->dwPredicted, index);
		Decode_Table_16(source + (bytes >> 3), dest + 1, perchannel, 2, info->dwPredicted2, index2);

		info->wIndex = (short)index;
		info->wIndex2 = (short)index2;
		return;
	}

	if (channels != 1) {
		return;
	}

	unsigned short index = (unsigned short)info->wIndex;

	Decode_Table_16(source, dest, (int)(bytes / 2), 1, info->dwPredicted, index);

	info->wIndex = (short)index;
}
