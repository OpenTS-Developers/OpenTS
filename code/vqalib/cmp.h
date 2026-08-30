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

#ifndef VQACMP_H
#define VQACMP_H

#pragma once

#include "../soscomp.h"

#if defined(__WATCOMC__) || defined(_MSC_VER)
#pragma pack(push,1)
#endif

extern "C" {
unsigned long __cdecl VQA_LCW_Uncompress(char const *source, char *dest, unsigned long length);
}

//#define VQA_LCW_Uncompress LCW_Uncompress

extern "C" {
long __cdecl AudioUnzap(void *source, void *dest, long);
}

/* VQA's SND2 chunks carry a stereo stream as two consecutive per-channel
   blocks rather than interleaved nybbles, unlike sosCODECDecompressData's
   AUD-derived layout. */
extern "C" {
unsigned int sosCODECDecompressDataPlanar(_SOS_COMPRESS_INFO *, unsigned int);
}

#if defined(__WATCOMC__) || defined(_MSC_VER)
#pragma pack(pop)
#endif

#endif //VQACMP_H
