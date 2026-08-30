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

/* define compression structure */
struct _SOS_COMPRESS_INFO {
	char       *lpSource;
	char       *lpDest;
	uint32_t      dwCompSize;
	uint32_t      dwUnCompSize;
	uint32_t      dwSampleIndex;
	int32_t       dwPredicted;
	int32_t       dwDifference;
	short         wCodeBuf;
	short         wCode;
	short         wStep;
	short         wIndex;

	uint32_t      dwSampleIndex2;   //added BP for channel 2
	int32_t       dwPredicted2;     //added BP for channel 2
	int32_t       dwDifference2;    //added BP for channel 2
	short         wCodeBuf2;        //added BP for channel 2
	short         wCode2;           //added BP for channel 2
	short         wStep2;           //added BP for channel 2
	short         wIndex2;          //added BP for channel 2
	short         wBitSize;
	short			  wChannels;		//added BP for # of channels
	};

/* compressed file type header */
struct _SOS_COMPRESS_HEADER {
	uint32_t      dwType;              // type of compression
	uint32_t      dwCompressedSize;    // compressed file size
	uint32_t      dwUnCompressedSize;  // uncompressed file size
	uint32_t      dwSourceBitSize;     // original bit size
	char          szName[16];          // file type, for error checking
	};

/* Prototypes */
extern "C" {
	void __cdecl sosCODECInitStream(_SOS_COMPRESS_INFO *);
	void __cdecl General_sosCODECInitStream(_SOS_COMPRESS_INFO *);
	uint32_t __cdecl sosCODECDecompressData(_SOS_COMPRESS_INFO *, uint32_t);
	uint32_t __cdecl General_sosCODECDecompressData(_SOS_COMPRESS_INFO *, uint32_t);
}
