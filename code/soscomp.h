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

/* compression types */
enum {
	_ADPCM_TYPE_1,
	};

/* define compression structure */
struct _SOS_COMPRESS_INFO {
	char       *lpSource;
	char       *lpDest;
	unsigned long dwCompSize;
	unsigned long dwUnCompSize;
	unsigned long dwSampleIndex;
	long          dwPredicted;
	long          dwDifference;
	short         wCodeBuf;
	short         wCode;
	short         wStep;
	short         wIndex;

	unsigned long dwSampleIndex2;   //added BP for channel 2
	long          dwPredicted2;     //added BP for channel 2
	long          dwDifference2;    //added BP for channel 2
	short         wCodeBuf2;        //added BP for channel 2
	short         wCode2;           //added BP for channel 2
	short         wStep2;           //added BP for channel 2
	short         wIndex2;          //added BP for channel 2
	short         wBitSize;
	short			  wChannels;		//added BP for # of channels
	};

/* compressed file type header */
struct _SOS_COMPRESS_HEADER {
	unsigned long dwType;              // type of compression
	unsigned long dwCompressedSize;    // compressed file size
	unsigned long dwUnCompressedSize;  // uncompressed file size
	unsigned long dwSourceBitSize;     // original bit size
	char          szName[16];          // file type, for error checking
	};

/* Prototypes */
extern "C" {
	void __cdecl sosCODECInitStream(_SOS_COMPRESS_INFO *);
	void __cdecl General_sosCODECInitStream(_SOS_COMPRESS_INFO *);
	unsigned long __cdecl sosCODECDecompressData(_SOS_COMPRESS_INFO *, unsigned long);
	unsigned long __cdecl General_sosCODECDecompressData(_SOS_COMPRESS_INFO *, unsigned long);
}
