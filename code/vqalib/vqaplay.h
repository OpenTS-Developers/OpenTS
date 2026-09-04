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

#ifndef VQAPLAY_H
#define VQAPLAY_H

#include <cstdint>
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
*     vqaplay.h
*
* DESCRIPTION
*      VQAPlay library definitions.
*
* PROGRAMMER
*     Bill Randolph
*     Denzil E. Long, Jr.
*
* DATE
*     April 10, 1995
*
****************************************************************************/



class VQAClass;

/*---------------------------------------------------------------------------
 * CONDITIONAL COMPILATION FLAGS
 *-------------------------------------------------------------------------*/

// MEG - 11.28.95 - added for debug
//extern void Debug_Printf( char *format_string, ... );

#ifdef __WATCOMC__
#define VQASTANDALONE 0  /* Stand alone player */
#define VQAVOC_ON     0  /* Enable VOC file override */
#define	VQAMONO_ON    0  /* Mono display output enable/disable */
#define VQADIRECT_SOUND 0	/* Use windows direct sound system */
#define VQAAUDIO_ON   1  /* Audio playback enable/disable */
#define VQAVIDEO_ON   0  /* Video manager enable/disable */
#define VQAMCGA_ON    0  /* MCGA enable/disable */
#define VQAXMODE_ON   0  /* Xmode enable/disable */
#define VQAVESA_ON    0  /* VESA enable/disable */
#define	VQABLOCK_2X2  0  /* 2x2 block decode enable/disable */
#define	VQABLOCK_2X3  0  /* 2x2 block decode enable/disable */
#define	VQABLOCK_4X2  0  /* 4x2 block decode enable/disable */
#define	VQABLOCK_4X4  0  /* 4x4 block decode enable/disable */
#define VQAWOOFER_ON  0
#else
#define VQASTANDALONE 0  /* Stand alone player */
#define VQAVOC_ON     0  /* Enable VOC file override */
#define	VQAMONO_ON    0  /* Mono display output enable/disable */
#define VQADIRECT_SOUND 0	/* Use windows direct sound system */
#define VQAAUDIO_ON   1  /* Audio playback enable/disable */
#define VQAVIDEO_ON   0  /* Video manager enable/disable */
#define VQAMCGA_ON    0  /* MCGA enable/disable */
#define VQAXMODE_ON   0  /* Xmode enable/disable */
#define VQAVESA_ON    0  /* VESA enable/disable */
#define	VQABLOCK_2X2  0  /* 2x2 block decode enable/disable */
#define	VQABLOCK_2X3  0  /* 2x2 block decode enable/disable */
#define	VQABLOCK_4X2  0  /* 4x2 block decode enable/disable */
#define	VQABLOCK_4X4  0  /* 4x4 block decode enable/disable */
#define VQAWOOFER_ON  0
#endif


/*---------------------------------------------------------------------------
 * GENERAL CONSTANT DEFINITIONS
 *-------------------------------------------------------------------------*/

/* Playback modes. */
#define VQAMODE_RUN   0  /* Run the movie through the end. */
#define VQAMODE_WALK  1  /* Draw the next frame then return. */
#define VQAMODE_PAUSE 2  /* Suspend movie playback. */
#define VQAMODE_STOP  3  /* Stop the movie. */

/* Playback flags. */
#define VQAPLAYF_STEP     (1<<0) /* Advance one frame (sets VQADRWF_STEP) */
#define VQAPLAYF_REPAINT  (1<<1) /* Redraw rather than fail/skip; also draws while paused */

/* Playback timer methods */
#define VQA_TMETHOD_DEFAULT -1 /* Use default timer method. */
#define	VQA_TMETHOD_DOS      1 /* DOS timer method */
#define	VQA_TMETHOD_INT      2 /* Interrupt timer method */
#define	VQA_TMETHOD_AUDIO    3 /* Audio timer method */

#define	VQA_TIMETICKS 60 /* Clock ticks per second */

// Loop modes
#define VQALOOP_NORMAL	0
#define VQALOOP_JUMP	1
#define VQALOOP_2		2
#define VQALOOP_3		3

// Loop iterations
#define INFINITE_LOOP	-1

/* Error/Status conditions */
enum VQAErrorType : int32_t {
	VQAERR_OK       = 0,
	VQAERR_NONE     = -1,	/* No error */
	VQAERR_EOF      = -2,	/* Valid end of file */
	VQAERR_OPEN     = -3,	/* Unable to open */
	VQAERR_READ     = -4,	/* Read error */
	VQAERR_WRITE    = -5,	/* Write error */
	VQAERR_SEEK     = -6,	/* Seek error */
	VQAERR_NOTVQA   = -7,	/* Not a valid VQA file. */
	VQAERR_NOMEM    = -8,	/* Unable to allocate memory */
	VQAERR_NOBUFFER = -9,	/* No buffer avail for load/draw */
	VQAERR_NOT_TIME = -10,	/* Not time for frame yet */
	VQAERR_SLEEPING = -11,	/* Function is in a sleep state */
	VQAERR_VIDEO    = -12,	/* Video related error. */
	VQAERR_AUDIO    = -13,	/* Audio related error. */
	VQAERR_PAUSED   = -14,	/* In paused state. */
	VQAERR_NOTIMER  = -15,
	VQAERR_NORATE   = -16,
	VQAERR_NOCONFIG = -17,
	VQAERR_NOAUDSIZ = -18,
	VQAERR_AUDSYNC  = -19,
	VQAERR_BADBLOCK = -20,
	VQAERR_NOAHANDL = -21,
	VQAERR_SKIPDRAW = -22,
	VQAERR_SETLOOP  = -23,
	VQAERR_SETBUFFR = -24,
	VQAERR_FORCEDRW = -25
};

/* Event flags. */
#define VQAEVENT_0			0
#define VQAEVENT_1			1
#define VQAEVENT_PALETTE	2 /* New palette; buffer = palette, nbytes = size */
#define VQAEVENT_CODEBOOK	3 /* New full codebook; buffer = codebook, nbytes = size */
#define VQAEVENT_LOOPED		4 /* Loop repeated; buffer = frame number, nbytes = loop id */
#define VQAEVENT_LOOPJUMP	5 /* Jumped to another loop; same arguments as VQAEVENT_LOOPED */
#define VQAEVENT_LOCK		6 /* Lock the render target; the return value is the buffer to draw into */
#define VQAEVENT_UNLOCK		7 /* Unlock the render target */


/*---------------------------------------------------------------------------
 * STRUCTURES AND RELATED DEFINITIONS
 *-------------------------------------------------------------------------*/

 typedef struct _VQAHandle VQAHandle;

// UnVQ functions must be this type
typedef void  (__cdecl *UNVQ_FUNC)(unsigned char *codebook, unsigned char *pointers, unsigned char *buffer, unsigned long blocksperrow, unsigned long numrows, unsigned long bufwidth);

// Handlers must be this type
typedef long (__cdecl *VQA_H_FUNC)(VQAHandle *vqa, long action, void *buffer, long nbytes);

// draw callback must be this type
typedef long (__cdecl *VQA_DC_FUNC)(VQAHandle *vqa, long framenum);

// timer callback must be this type
typedef unsigned long (__cdecl *VQA_TC_FUNC)(VQAHandle *vqa);

// unused callback must be this type
typedef unsigned long (__cdecl *VQA_UC_FUNC)(VQAHandle *vqa);

/* VQAConfig: Player configuration structure
 *
 * DrawerCallback - User routine for Drawer to call each frame (NULL = none)
 * EventHandler   - User routine for notification to client of events.
 * NotifyFlags    - User specified events to be notified about.
 * Vmode          - Requested Video mode (May be promoted).
 * VBIBit         - Vertical blank bit polarity.
 * ImageBuf       - Pointer to caller's buffer for the Drawer to use as its
 *                  ImageBuf; NULL = player will allocate its own, if
 *                  VQACFGF_BUFFER is set in DrawFlags.
 * ImageWidth     - Width of Image buffer.
 * ImageHeight    - Height of Image buffer.
 * X1             - Draw window X coordinate (-1 = Center).
 * Y1             - Draw window Y coordinate (-1 = Center).
 * FrameRate      - Desired frames per second (-1 = use VQA header's value).
 * DrawRate       - Desired drawing frame rate; allows the Drawer to draw at
 *                  a separate rate from the Loader.
 * TimerMethod    - Timer method to use during playback.
 * DrawFlags      - Bits control various special drawing options. (See below)
 * OptionFlags    - Bits control various special misc options. (See below)
 * NumFrameBufs   - Desired number of frame buffers. (Default = 6)
 * NumCBBufs      - Desired number of codebook buffers. (Default = 3)
 * SoundObject		- Ptr to callers Direct Sound Object (Default =NULL)
 * PrimaryBufferPtr- Ptr to callers Primary Sound Buffer. (Default = NULL)
 * VocFile        - Name of VOC file to play instead of VQA audio track.
 * AudioBuf       - Pointer to audio buffer.
 * AudioBufSize   - Size of audio buffer. (Default = 32768)
 * AudioRate      - Audio data playback rate (-1 = use samplerate scaled
 *                  to the frame rate)
 * Volume         - Audio playback volume. (0x7FFF = max)
 * HMIBufSize     - Desired HMI buffer size. (Default = 2000)
 * DigiHandle     - Handle to an initialized sound driver. (-1 = none)
 * DigiCard       - HMI ID of card to use. (0 = none, -1 = auto-detect)
 * DigiPort       - Audio port address. (-1 = auto-detect)
 * DigiIRQ        - Audio IRQ. (-1 = auto-detect)
 * DigiDMA        - Audio DMA channel. (-1 = auto-detect)
 * Language       - Language identifier. (Not used)
 * CapFont        - Pointer to font to use for subtitle text captions.
 * EVAFont        - Pointer to font to use for E.V.A text cations. (For C&C)
 */
typedef struct _VQAConfig {
	VQA_H_FUNC			StreamHandler;
	VQA_H_FUNC 			MemoryHandler;
	VQA_H_FUNC 			EventHandler;
	VQA_DC_FUNC			DrawerCallback;
	VQA_TC_FUNC			TimerCallback;
	VQA_UC_FUNC 		UnusedCallback;
	VQA_H_FUNC			AudioHandler;
	unsigned char *ImageBuf;

	/*
	 * Width of the image buffer. When -1, the width recorded in the movie header
	 * is used instead.
	 */
	int					ImageWidth;

	/*
	 * Height of the image buffer. When -1, the height recorded in the movie
	 * header is used instead.
	 */
	int					ImageHeight;

	int					X1;
	int					Y1;
	long          FrameRate;
	long          DrawRate;
	int					RefreshRate;

	/// Unused
	int					field_3C;

	long          DrawFlags;
	long          OptionFlags;
	long          NumFrameBufs;
	long          NumCBBufs;

	/// Unused
	int					field_50;

	unsigned char *AudioBuf;
	long          AudioBufSize;
	long          HMIBufSize;
	int					AudioRate;
	int					Volume;

	/// Unused
	int					field_68;

	/*
	 * The VQAClass that owns this player. VQALib itself never touches it; the
	 * client stores itself here at construction and its own handlers read it
	 * back to recover the instance from a bare VQAHandle.
	 */
	VQAClass			*Owner;

	/*
	 * The loop to start playback in. When this is zero or greater, the loop is
	 * selected as the movie is opened. A negative value plays from the top.
	 */
	int					InitialLoopID;

	/*
	 * The number of times the loop named by InitialLoopID repeats.
	 */
	int					InitialLoopIterations;

	/*
	 * Scratch storage for the default disk stream handler, which keeps the open
	 * file's handle here for the life of the movie.
	 */
	int					StreamFileHandle;

	/// Unused
	int					field_7C;
	int					field_80;

	unsigned long		LatencyAdjustment;
} VQAConfig;

/* Drawer Configuration flags (DrawFlags) */
#define	VQACFGB_BUFFER   0 /* Buffer UnVQ enable */
#define	VQACFGB_NODRAW   1 /* Drawing disable */
#define VQACFGB_NOSKIP   2 /* Disable frame skipping. */
#define	VQACFGB_VRAMCB   3 /* XMode VRAM copy enable */
#define VQACFGB_ORIGIN   4 /* 0,0 origin position */
#define	VQACFGB_SCALEX2  6 /* Scale X2 enable (VESA 320x200 to 640x400) */
#define VQACFGB_WOOFER   7
#define	VQACFGF_BUFFER   (1<<VQACFGB_BUFFER)
#define	VQACFGF_NODRAW   (1<<VQACFGB_NODRAW)
#define	VQACFGF_NOSKIP   (1<<VQACFGB_NOSKIP)
#define	VQACFGF_OFFSET   (1<<3)
#define	VQACFGF_ORIGIN   (3<<VQACFGB_ORIGIN)
#define	VQACFGF_HALFSIZE (1<<6)	/// Draw at half size; selects the _HALF decoders.
#define	VQACFGF_NOTRANS  (1<<7)	/// Disable index-0 transparency; selects the _KEY decoders.
#define	VQACFGF_8        (1<<8)
#define	VQACFGF_9        (1<<9)
#define	VQACFGF_TOPLEFT  (0<<VQACFGB_ORIGIN)
#define	VQACFGF_TOPRIGHT (1<<VQACFGB_ORIGIN)
#define	VQACFGF_BOTRIGHT (2<<VQACFGB_ORIGIN)
#define	VQACFGF_BOTLEFT  (3<<VQACFGB_ORIGIN)
//#define	VQACFGF_SCALEX2  (1<<VQACFGB_SCALEX2)
//#define VQACFGF_WOOFER   (1<<VQACFGB_WOOFER)

/* Options Configuration (OptionFlags) */
#define	VQAOPTB_AUDIO    0 /* Audio enable. */
#define	VQAOPTB_STEP     1 /* Single step enable. */
#define	VQAOPTB_MONO     2 /* Mono output enable. */
#define VQAOPTB_PALOFF   3 /* Palette set disable. */
#define	VQAOPTB_SLOWPAL  4 /* Slow palette enable. */
#define VQAOPTB_HMIINIT  5 /* HMI already initialized by client. */
#define VQAOPTB_ALTAUDIO 6 /* Use alternate audio track. */
#define VQAOPTB_CAPTIONS 7 /* Show captions. */
#define VQAOPTB_EVA      8 /* Show EVA text (For C&C only) */
#define VQAOPTB_WAITFILL 9 /* Wait for the buffers to refill when the loader falls behind. */
#define	VQAOPTF_AUDIO    (1<<VQAOPTB_AUDIO)
#define VQAOPTF_STEP     (1<<VQAOPTB_STEP)
#define VQAOPTF_MONO     (1<<VQAOPTB_MONO)
#define VQAOPTF_PALOFF   (1<<VQAOPTB_PALOFF)
#define VQAOPTF_SLOWPAL  (1<<VQAOPTB_SLOWPAL)
#define VQAOPTF_LOOPS     (1<<5)	/// Enable the loop subsystem.
#define	VQAOPTF_ALTAUDIO (1<<VQAOPTB_ALTAUDIO)
#define VQAOPTF_7        (1<<7)
#define VQAOPTF_LOOPCACHE (1<<8)	/// Cache the largest loop at open time.
#define VQAOPTF_WAITFILL (1<<VQAOPTB_WAITFILL)



/* VQAHandle: VQA file handle. (Must be obtained by calling VQA_Alloc()
 *            and freed through VQA_Free(). This is the only legal way
 *            to obtain and dispose of a VQAHandle.
 *
 * VQAio - Something meaningful to the IO manager. (See DOCS)
 */
typedef struct _VQAHandle {
	unsigned long VQAio;
} VQAHandle;

// derives from AMIGA IFF handling https://wiki.amigaos.net/wiki/IFFParse_Library

/* Possible IO command values */
#define VQACMD_NOTHING  0
#define VQACMD_INIT     1 /* Prepare the IO for a session */
#define VQACMD_CLEANUP  2 /* Terminate IO session */
#define VQACMD_OPEN     3 /* Open file */
#define VQACMD_CLOSE    4 /* Close file */
#define VQACMD_READ     5 /* Read bytes */
#define VQACMD_WRITE    6 /* Write bytes */
#define VQACMD_SEEK     7 /* Seek */
#define VQACMD_SEEKPEEK 8
#define VQACMD_SIZE     9

#define VQAMEM_0		0
#define VQAMEM_1		1
#define VQAMEM_ALLOC	2
#define VQAMEM_FREE		3
#define VQAMEM_LOCK		4
#define VQAMEM_UNLOCK	5
#define VQAMEM_QUERYSIZE 6	/// Query the loop cache budget; -1 means no limit.

#define VQAAUDIO_0		0
#define VQAAUDIO_INIT	1
#define VQAAUDIO_OPEN	2
#define VQAAUDIO_CLOSE	3
#define VQAAUDIO_START	4
#define VQAAUDIO_LOAD	5
#define VQAAUDIO_PAUSE	6
#define VQAAUDIO_STOP	7
#define VQAAUDIO_PLAY	8

/*---------------------------------------------------------------------------
 * FUNCTION PROTOTYPES
 *-------------------------------------------------------------------------*/

/* Configuration routines. */
void VQA_INIConfig(VQAConfig *config);
void VQA_DefaultConfig(VQAConfig *config);

/* Handle manipulation routines. */
void VQA_Reset(VQAHandle *vqa);

//VQAHandle *VQA_Alloc(void);
//void VQA_Init(VQAHandle *, long (*)());
/* File routines. */
VQAErrorType VQA_Open(char const *, _VQAConfig *, VQAHandle **vqa);
void VQA_Free(VQAHandle *vqa);
void VQA_Close(VQAHandle *vqa);
long VQA_Play(VQAHandle *vqa, long, int flags);
long VQA_SeekFrame(VQAHandle *vqa, long framenum, long fromwhere);
long VQA_SetStop(VQAHandle *vqa, long stop);
VQAErrorType VQA_SetLoop(VQAHandle *vqa, int id, int iterations, int mode);
VQAErrorType VQA_SetLoop_Internal(VQAHandle *vqa, int start, int end, int iterations, int mode);

VQAErrorType VQA_SetUnVQ(VQAHandle *vqa, UNVQ_FUNC unvq1, UNVQ_FUNC unvq2);

VQAErrorType VQA_Set_DrawBuffer(VQAHandle *vqa, unsigned char *buffer, unsigned long width, unsigned long height, long xpos, long ypos);
VQAErrorType VQA_ResetLastFrameNum(VQAHandle *vqa);

/* Information/statistics access routines. */
VQAErrorType VQA_GetBlockInfo(VQAHandle *vqa, long & blockw, long & blockh, long & clrmode);

#endif /* VQAPLAY_H */

