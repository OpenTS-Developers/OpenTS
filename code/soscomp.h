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

/****************************************************************************
*
*  File              : soscomp.h
*  Date Created      : 6/1/94
*  Description       :
*
*  Programmer(s)     : Nick Skrepetos
*  Last Modification : 10/1/94 - 11:37:9 AM
*  Additional Notes  : Modified by Denzil E. Long, Jr.
*
*****************************************************************************
*            Copyright (c) 1994,  HMI, Inc.  All Rights Reserved            *
****************************************************************************/
#pragma once

#include <cstdint>

/* compression types */
enum {
	_ADPCM_TYPE_1,
	};

struct SosChannel {
	uint32_t SampleIndex;
	short CodeBuf;
	short Code;
	int32_t Predicted;
	int32_t Difference;
	short Index;
	short Step;
};

/* define compression structure */
struct SosCompressInfo {
	char *     Source;
	char *     Dest;
	uint32_t   CompSize;
	uint32_t   UnCompSize;
	SosChannel Channels[2];
	short      BitSize;
	short      ChannelCount;		//added BP for # of channels
};

/* Prototypes */
extern "C" {
	void __cdecl sosCODECInitStream(SosCompressInfo *);
	void __cdecl General_sosCODECInitStream(SosCompressInfo *);
	uint32_t __cdecl sosCODECDecompressData(SosCompressInfo *, uint32_t);
	uint32_t __cdecl General_sosCODECDecompressData(SosCompressInfo *, uint32_t);
}
