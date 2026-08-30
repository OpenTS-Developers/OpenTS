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
enum
{
    _ADPCM_TYPE_1,
};

typedef struct _tagCOMPRESS_CHANNEL_INFO
{
    unsigned int dwSampleIndex;
    int dwPredicted;
    int dwDifference;
    short wCodeBuf;
    short wCode;
    short wStep;
    short wIndex;
} _COMPRESS_CHANNEL_INFO;

/* define compression structure */
typedef struct _tagCOMPRESS_INFO
{
    char* lpSource;
    char* lpDest;
    unsigned int dwCompSize;
    unsigned int dwUnCompSize;
    _COMPRESS_CHANNEL_INFO Channels[2];
    short wBitSize;
    short wChannels; // added BP for # of channels
} _SOS_COMPRESS_INFO;

/* compressed file type header */
typedef struct _tagCOMPRESS_HEADER
{
    unsigned int dwType;             // type of compression
    unsigned int dwCompressedSize;   // compressed file size
    unsigned int dwUnCompressedSize; // uncompressed file size
    unsigned int dwSourceBitSize;    // original bit size
    char szName[16];                 // file type, for error checking
} _SOS_COMPRESS_HEADER;

/* Prototypes */
extern "C" {
	void sosCODECInitStream(_SOS_COMPRESS_INFO *);
	unsigned int sosCODECDecompressData(_SOS_COMPRESS_INFO *, unsigned int);
	unsigned int sosCODECDecompressDataPlanar(_SOS_COMPRESS_INFO *, unsigned int);
}
