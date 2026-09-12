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

#ifndef VQAFILE_H
#define VQAFILE_H
/****************************************************************************
*
*         C O N F I D E N T I A L -- W E S T W O O D  S T U D I O S
*
*----------------------------------------------------------------------------
*
* PROJECT
*     VQA player library. (32-Bit protected mode)
*
* FILE
*     vqafile.h
*
* DESCRIPTION
*     VQA file format definitions.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* DATE
*     April 10, 1995
*
****************************************************************************/

#include "iff.h"

#pragma pack(push,1)

/*---------------------------------------------------------------------------
 * STRUCTURE DEFINITIONS AND RELATED DEFINES.
 *-------------------------------------------------------------------------*/

/* VQAHeader: VQA movie description header. (VQHD)
 *
 * Version       - VQA version.
 * Flags         - Various flags. (See below)
 * ImageWidth    - Image width in pixels.
 * ImageHeight   - Image height in pixels.
 * BlockWidth    - Block width in pixels.
 * BlockHeight   - Block height in pixels.
 * Frames        - Total number of frames in the movie.
 * FPS           - Playback rate (Frame Per Second).
 * Groupsize     - Frame grouping size (frames per codebook).
 * Num1Colors    - Number of 1 color colors.
 * CBentries     - Number of codebook entries.
 * Xpos          - X position to draw frames. (-1 = Center)
 * Ypos          - Y position to draw frames. (-1 = Center)
 * MaxFramesize  - Size of largest frame.
 * SampleRate    - Sample rate of primary audio stream.
 * Channels      - Number of channels in primary audio stream.
 * BitsPerSample - Sample bit size in primary audio stream.
 * FutureUse     - Reserved for future expansion.
 */
typedef struct _VQAHeader {
	unsigned short Version;
	unsigned short Flags;
	unsigned short Frames;
	unsigned short ImageWidth;
	unsigned short ImageHeight;
	unsigned char  BlockWidth;
	unsigned char  BlockHeight;
	unsigned char  FPS;
	unsigned char  Groupsize;
	unsigned short Num1Colors;
	unsigned short CBentries;
	unsigned short Xpos;
	unsigned short Ypos;
	unsigned short MaxFramesize;
	unsigned short SampleRate;
	unsigned char  Channels;
	unsigned char  BitsPerSample;
	unsigned short AltSampleRate;
	unsigned char  AltChannels;
	unsigned char  AltBitsPerSample;
//	unsigned short FutureUse[5];
	unsigned char ColorMode;
	unsigned char Pad;

	/*
	 * Size of the largest codebook as it is stored in the file, which for a
	 * CBFZ/CBPZ movie is smaller than the expanded codebook. AllocBuffers
	 * rejects the movie if it exceeds the expanded size and substitutes the
	 * expanded size when it is zero, so an old movie that leaves it blank
	 * still allocates correctly.
	 */
	unsigned int MaxCBSize;

	/*
	 * Bytes of audio that must be loaded ahead of a seek target to prime the
	 * decoder. VQA_SeekGroup divides it by the per-frame audio size to decide
	 * how many frames early to start reading. When the movie carries no
	 * VQAHDF_SNDJUMP flag and this is zero, half a second is assumed.
	 */
	unsigned int AudioPreload;
} VQAHeader;
static_assert(sizeof(VQAHeader) == 42, "the VQHD chunk is 42 bytes on disk");

/* Version type. */
#define VQAHD_VER1 1
#define VQAHD_VER2 2
#define VQAHD_VER3 3

/* VQA header flag definitions */
#define VQAHDB_AUDIO    0 /* Audio track present. */
#define VQAHDB_ALTAUDIO 1 /* Alternate audio track present. */
#define VQAHDB_TRANS    2 /* Movie uses semitransparent blocks. */
#define VQAHDF_AUDIO    (1<<VQAHDB_AUDIO)
#define VQAHDF_ALTAUDIO (1<<VQAHDB_ALTAUDIO)
#define VQAHDF_TRANS    (1<<VQAHDB_TRANS)
#define VQAHDF_SNDJUMP  (1<<3)	/// Audio track can be repositioned mid-stream.
#define VQAHDF_4        (1<<4)


/* Frame information (FINF) chunk definitions
 *
 * The FINF chunk contains a longword (4 bytes) entry for each
 * frame in the movie. This entry is divided into two parts,
 * flags (4 bits) and offset (28 bits).
 *
 * BITS   NAME     DESCRIPTION
 * -----------------------------------------------------------
 * 31-28  Flags    4 bitwise boolean flags.
 * 27-0   Offset   Offset in WORDS from the start of the file.
 */
#define VQAFINB_KEY  31
#define VQAFINB_PAL  30
#define VQAFINB_SYNC 29
#define VQAFINB_28   28
#define VQAFINF_KEY  (1L<<VQAFINB_KEY)
#define VQAFINF_PAL  (1L<<VQAFINB_PAL)
#define VQAFINF_SYNC (1L<<VQAFINB_SYNC)
#define VQAFINF_28   (1L<<VQAFINB_28)

/* FINF related defines and macros. */
#define VQAFINF_OFFSET 0x0FFFFFFFL
#define VQAFINF_FLAGS  0xF0000000L
#define VQAFRAME_OFFSET(a) (((a & VQAFINF_OFFSET)<<1))

/* VQ vector pointer codes. */
#define VPC_ONE_SINGLE    0xF000 /* One single color block */
#define VPC_ONE_SEMITRANS 0xE000 /* One semitransparent block */
#define VPC_SHORT_DUMP    0xD000 /* Short dump of single color blocks */
#define VPC_LONG_DUMP     0xC000 /* Long dump of single color blocks */
#define VPC_SHORT_RUN     0xB000 /* Short run of single color blocks */
#define VPC_LONG_RUN      0xA000 /* Long run */

/* Long run codes. */
#define LRC_SEMITRANS 0xC000 /* Long run of semitransparent blocks. */
#define LRC_SINGLE    0x8000 /* Long run of single color blocks. */

/* Defines used for Run-Skip-Dump compression. */
#define MIN_SHORT_RUN_LENGTH  2
#define MAX_SHORT_RUN_LENGTH  15
#define MIN_LONG_RUN_LENGTH   2
#define MAX_LONG_RUN_LENGTH   4095
#define MIN_SHORT_DUMP_LENGTH 3
#define MAX_SHORT_DUMP_LENGTH 15
#define MIN_LONG_DUMP_LENGTH  2
#define MAX_LONG_DUMP_LENGTH  4095

#define WORD_HI_BIT 0x8000

/*---------------------------------------------------------------------------
 * VQA FILE CHUNK ID DEFINITIONS.
 *-------------------------------------------------------------------------*/

#define ID_WVQA MAKE_ID('W','V','Q','A') /* Westwood VQ Animation form. */
#define ID_VQHD MAKE_ID('V','Q','H','D') /* VQ header. */
#define ID_NAME MAKE_ID('N','A','M','E') /* Name string. */
#define ID_FINF MAKE_ID('F','I','N','F') /* Frame information. */
#define ID_VQFR MAKE_ID('V','Q','F','R') /* VQ frame container. */
#define ID_VQFK MAKE_ID('V','Q','F','K') /* VQ key frame container. */
#define ID_VQFL MAKE_ID('V','Q','F','L') /* VQ loop frame container. */
#define ID_CBF0 MAKE_ID('C','B','F','0') /* Full codebook. */
#define ID_CBFZ MAKE_ID('C','B','F','Z') /* Full codebook (compressed). */
#define ID_CBP0 MAKE_ID('C','B','P','0') /* Partial codebook. */
#define ID_CBPZ MAKE_ID('C','B','P','Z') /* Partial codebook (compressed). */
#define ID_VPT0 MAKE_ID('V','P','T','0') /* Vector pointers. */
#define ID_VPTZ MAKE_ID('V','P','T','Z') /* Vector pointers (compressed). */
#define ID_VPTK MAKE_ID('V','P','T','K') /* Vector pointers (Delta Key). */
#define ID_VPTD MAKE_ID('V','P','T','D') /* Vector pointers (Delta). */
#define ID_VPTR MAKE_ID('V','P','T','R') /* Pointers RSD compressed. */
#define ID_VPRZ MAKE_ID('V','P','R','Z') /* Pointers RSD, lcw compressed. */
#define ID_CPL0 MAKE_ID('C','P','L','0') /* Color palette. */
#define ID_CPLZ MAKE_ID('C','P','L','Z') /* Color palette (compressed). */
#define ID_SND0 MAKE_ID('S','N','D','0') /* Sound */
#define ID_SND1 MAKE_ID('S','N','D','1') /* Sound (Zap compressed). */
#define ID_SND2 MAKE_ID('S','N','D','2') /* Sound (ADPCM compressed). */
#define ID_SNDZ MAKE_ID('S','N','D','Z') /* Sound (LCW compression). */

#define ID_SNA0 MAKE_ID('S','N','A','0') /* Sound */
#define ID_SNA1 MAKE_ID('S','N','A','1') /* Sound (Zap compressed). */
#define ID_SNA2 MAKE_ID('S','N','A','2') /* Sound (ADPCM compressed). */
#define ID_SNAZ MAKE_ID('S','N','A','Z') /* Sound (LCW compression). */
#define ID_SN2J MAKE_ID('S','N','2','J') /* ADPCM stream-state jump chunk. */

#define ID_CAP0 MAKE_ID('C','A','P','0') /* Caption text */
#define ID_EVA0 MAKE_ID('E','V','A','0') /* EVA text */

#define ID_CINF MAKE_ID('C','I','N','F') // Codebook INFo
#define ID_CINH MAKE_ID('C','I','N','H') // Codebook INfo Header
#define ID_CIND MAKE_ID('C','I','N','D') // Codebook INfo Data
#define ID_PINF MAKE_ID('P','I','N','F') // Palette INFo
#define ID_PINH MAKE_ID('P','I','N','H') // Palette INfo Header
#define ID_PIND MAKE_ID('P','I','N','D') // Palette INfo Data
#define ID_LINF MAKE_ID('L','I','N','F') // Loop INFo
#define ID_LINH MAKE_ID('L','I','N','H') // Loop INfo Header
#define ID_LIND MAKE_ID('L','I','N','D') // Loop INfo Data

#define ID_CLIP MAKE_ID('C','L','I','P')

#define ID_MSCI MAKE_ID('M','S','C','I')
#define ID_MSCH MAKE_ID('M','S','C','H')
#define ID_MSCD MAKE_ID('M','S','C','D')
#define ID_MSCT MAKE_ID('M','S','C','T')
#define ID_MFCI MAKE_ID('M','F','C','I')
#define ID_MFCH MAKE_ID('M','F','C','H')
#define ID_MFCD MAKE_ID('M','F','C','D')
#define ID_MFCT MAKE_ID('M','F','C','T')

#define ID_VPKZ MAKE_ID('V','P','K','Z')
#define ID_VPDZ MAKE_ID('V','P','D','Z')

#pragma pack(pop)

#endif /* VQAFILE_H */

