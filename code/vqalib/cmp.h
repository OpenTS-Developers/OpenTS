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

#if defined(__WATCOMC__) || defined(_MSC_VER)
#pragma pack(push,1)
#endif

struct _VQA_SOS_COMPRESS_INFO

{
	long dwPredicted;
	short wIndex;
	long dwPredicted2;
	short wIndex2;
};

typedef _VQA_SOS_COMPRESS_INFO VQASOS;


extern "C" {
unsigned long __cdecl VQA_LCW_Uncompress(char const *source, char *dest, unsigned long length);
}

//#define VQA_LCW_Uncompress LCW_Uncompress

extern "C" {
long __cdecl AudioUnzap(void *source, void *dest, long);
}

extern "C" {
void __cdecl VQA_sosCODECInitStream(_VQA_SOS_COMPRESS_INFO *);
void __cdecl VQA_sosCODECDecompressData(void *src, void *dst, unsigned short wBitSize, unsigned short wChannels, unsigned long dwUnCompSize, _VQA_SOS_COMPRESS_INFO *sosinfo);
}

//#define VQA_sosCODECDecompressData sosCODECDecompressData

#if defined(__WATCOMC__) || defined(_MSC_VER)
#pragma pack(pop)
#endif

#endif //VQACMP_H
