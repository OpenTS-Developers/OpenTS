/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 Vanilla-Conquer contributors
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/
#include "soscomp.h"
#include <string.h>
#include <assert.h>

// index table for stepping into step table.
static const short wCODECIndexTab[16] = {-1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8};

static const short wCODECStepTab[89] = {
    7,    8,     9,     10,    11,    12,    13,    14,    16,    17,    19,    21,    23,    25,   28,
    31,   34,    37,    41,    45,    50,    55,    60,    66,    73,    80,    88,    97,    107,  118,
    130,  143,   157,   173,   190,   209,   230,   253,   279,   307,   337,   371,   408,   449,  494,
    544,  598,   658,   724,   796,   876,   963,   1060,  1166,  1282,  1411,  1552,  1707,  1878, 2066,
    2272, 2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,  5894,  6484,  7132,  7845, 8630,
    9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767};

#ifndef clamp
#define clamp(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#endif

extern "C"
void sosCODECInitStream(_SOS_COMPRESS_INFO* stream)
{
    for (int i = 0; i < 2; i++) {
        stream->Channels[i].wCode = 0;
        stream->Channels[i].wCodeBuf = 0;
        stream->Channels[i].wIndex = 0;
        stream->Channels[i].wStep = wCODECStepTab[0];
        stream->Channels[i].dwPredicted = 0;
        stream->Channels[i].dwSampleIndex = 0;
    }
}

/* Number of possible wIndex.  Comes from the fact that:
 *
 *   next_index = clamp(next_index, 0, 88);
 *
 * which means 0 <= index <= 88, hence 89 indexes.
 */
#define NUM_INDEXES 89

/* Number of possible nybbles.  Comes from the fact that:
 *
 *   next_nybble = wCodeBuf & 0xF
 *
 * which means 0 <= next_nybble <= 15, hence 16 possibilites.
 */
#define NUM_NYBBLES 16

/* Define a dynamic programming table mapping all possible indexes and nybbles
 * into their next value.  Pack things together into a struct so a cache miss
 * will retrieve both next index and diff value.
 *
 * This table should consume ~12kb, which is quite small.
 *
 */
static struct
{
    int diff;
    short index;
} SosDecompTable[NUM_INDEXES][NUM_NYBBLES];

/* Flag if above table was initialized.  */
static bool SosDecompTableGenerated = false;

/* Generate decompression table for samples.  Precompute every possible value
 * of dwDifference and Channels[0].wIndex based on every possible  combination
 * of index and nybble values.  */
void sosCODECGenerateDecompressTable(void)
{
    short index, nybble;
    int diff;

    for (index = 0; index < NUM_INDEXES; index++) {
        short step = wCODECStepTab[index];
        for (nybble = 0; nybble < NUM_NYBBLES; nybble++) {
            diff = step >> 3;

            if ((nybble & 4) != 0) {
                diff += step;
            }

            if ((nybble & 2) != 0) {
                diff += step >> 1;
            }

            if ((nybble & 1) != 0) {
                diff += step >> 2;
            }

            if ((nybble & 8) != 0) {
                diff = -diff;
            }

            short next_index = index + wCODECIndexTab[nybble & 0x7];
            next_index = clamp(next_index, 0, 88);

            SosDecompTable[index][nybble].diff = diff;
            SosDecompTable[index][nybble].index = next_index;
        }
    }
}

/* Template version of sosCODECDecompressData which generates a single version
   for 8-bits and 16-bits, and for the interleaved (AUD) and planar (VQA) source
   layouts.  This instructs the compiler to avoid generating a branch in the
   deepest loop.  */
template <bool BITS_8, bool PLANAR> static unsigned sosCODECDecompressDataTemplate(_SOS_COMPRESS_INFO* stream, unsigned bytes)
{
    unsigned full_length = bytes;
    int num_channels = stream->wChannels;

    /* 'bytes' is the total decompressed size across every channel; each
       channel decodes its own share, and each loop iteration below produces
       two of its samples.  */
    bytes = (BITS_8 ? (bytes / 2) : (bytes / 4)) / num_channels;

    /* Quickly return if we are not going to write anything.  */
    if (bytes == 0) {
        return full_length;
    }

    int channel = 0;

    unsigned char* src = (unsigned char*)stream->lpSource;
    short* dst = (short*)(stream->lpDest);
    do {
        short index = stream->Channels[channel].wIndex;
        int sample = stream->Channels[channel].dwPredicted;

        int j = 0;
        do {
            unsigned char codebuf = *src;
            src += PLANAR ? 1 : num_channels;

            /* First step: case dwSampleIndex is even (unrolled).  */
            char current_nybble = codebuf & 0xF;

            sample += SosDecompTable[index][current_nybble].diff;
            sample = clamp(sample, -32768, 32767);

            if (BITS_8) {
                *dst = ((sample & 0xFF00) >> 8) ^ 0x80;
                dst = (short*)((char*)(dst) + num_channels);
            } else {
                *dst = sample;
                dst += num_channels;
            }

            index = SosDecompTable[index][current_nybble].index;

            /* Second step: case dwSampleIndex is odd (unrolled).  */
            current_nybble = codebuf >> 4;
            sample += SosDecompTable[index][current_nybble].diff;
            sample = clamp(sample, -32768, 32767);

            if (BITS_8) {
                *dst = ((sample & 0xFF00) >> 8) ^ 0x80;
                dst = (short*)((char*)(dst) + num_channels);
            } else {
                *dst = sample;
                dst += num_channels;
            }

            index = SosDecompTable[index][current_nybble].index;
        } while (++j < bytes);

        /* Write back the important stuff from the loop back to the struct.  */
        stream->Channels[channel].dwPredicted = sample;
        stream->Channels[channel].wIndex = index;

        /* In case of stereo we also need to update the src and dst pointers
         before proceeding to the next iteration..  */
        src = PLANAR ? ((unsigned char*)stream->lpSource + bytes) : ((unsigned char*)stream->lpSource + 1);
        if (BITS_8) {
            dst = (short*)(stream->lpDest + 1);
        } else {
            dst = (short*)(stream->lpDest) + 1;
        }
    } while (++channel < num_channels);

    return full_length;
}

//
// decompress data from a 4:1 ADPCM compressed file.  the number of
// bytes decompressed is returned.
//
//
extern "C"
unsigned int sosCODECDecompressData(_SOS_COMPRESS_INFO* stream, unsigned int bytes)
{
    if (SosDecompTableGenerated == false) {
        sosCODECGenerateDecompressTable();
        SosDecompTableGenerated = true;
    }

    if (stream->wBitSize == 16) {
        return sosCODECDecompressDataTemplate<false, false>(stream, bytes);
    }
#if 0 // No video or audio sample with this option?
    else {
        return sosCODECDecompressDataTemplate<true, false>(stream, bytes);
    }
#endif
    assert(0 && "Unreachable");
    return 0;
}

/* VQA's SND2 chunks carry a stereo stream as two consecutive per-channel
   blocks rather than interleaved nybbles, so it needs its own entry point
   into the shared decoder.  */
extern "C"
unsigned int sosCODECDecompressDataPlanar(_SOS_COMPRESS_INFO* stream, unsigned int bytes)
{
    if (SosDecompTableGenerated == false) {
        sosCODECGenerateDecompressTable();
        SosDecompTableGenerated = true;
    }

    if (stream->wBitSize != 16) {
        return 0;
    }

    return sosCODECDecompressDataTemplate<false, true>(stream, bytes);
}