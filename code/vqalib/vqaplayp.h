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

#ifndef VQAPLAYP_H
#define VQAPLAYP_H
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
*     vqaplayp.h
*
* DESCRIPTION
*     VQAPlay private library definitions.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     Bill Randolph
*
* DATE
*     August 21, 1995
*
****************************************************************************/

#include "vqafile.h"
#include "vqaplay.h"
#include "cmp.h"

#define  NULL 0

#if 0
#define STATIC static
#else
#define STATIC
#endif

#if 1
#define _STATIC static
#else
#define _STATIC
#endif

/* VQABool - the type used for predicate returns and flag arguments throughout
 * the player. Compilers without a native bool pick up the int typedef in
 * vqaplay.h, so using this name keeps the affected declarations free of
 * per-compiler #ifs.
 */
typedef bool VQABool;

/*---------------------------------------------------------------------------
 * GENERAL CONSTANT DEFINITIONS
 *-------------------------------------------------------------------------*/

/* Internal library version. */
#define VQA_VERSION "5.0"
#define VQA_DATE    __DATE__ " " __TIME__

#if _MSC_VER >= 1200
#undef VQA_DATE
#define VQA_DATE "Nov 12 1999 13:58:22"
#endif

#define VQA_IDSTRING "VQA playback " VQA_VERSION " (" VQA_DATE ")"
#define VQA_REQUIRES "Support library " VQA_VERSION " or better."

extern char VerTag[];
extern char ReqTag[];

/* Block dimensions macro and identifiers. */
#define BLOCK_DIM(a,b) (((a&0xFF)<<8)|(b&0xFF))
#define BLOCK_2X2 BLOCK_DIM(2,2)
#define BLOCK_2X3 BLOCK_DIM(2,3)
#define BLOCK_4X2 BLOCK_DIM(4,2)
#define BLOCK_4X4 BLOCK_DIM(4,4)

/* Memory limits */
#define	VQA_MAX_CBBUFS    10 /* Maximum number of codebook buffers */
#define	VQA_MAX_FRAMEBUFS 30 /* Maximum number of frame buffers */

/* Special Constants */
#define	VQA_MASK_POINTER 0x8000 /* Pointer value to use for masking. */

#pragma pack(push,1)


/*---------------------------------------------------------------------------
 * STRUCTURES AND RELATED DEFINITIONS
 *-------------------------------------------------------------------------*/

/* ChunkHeader: IFF chunk identifier header.
 *
 * id   - 4 Byte chunk id.
 * size - Size of chunk.
 */
typedef struct _ChunkHeader {
	unsigned long id;
	unsigned long size;
} ChunkHeader;


/* ZAPHeader: ZAP audio compression header. NOTE: If the uncompressed size
 *            and the compressed size are equal then the audio frame is RAW
 *            (NOT COMPRESSED).
 *
 * UnCompSize - Uncompressed size in bytes.
 * CompSize   - Compressed size in bytes.
 */
typedef struct _ZAPHeader {
	unsigned short UnCompSize;
	unsigned short CompSize;
} ZAPHeader;

typedef struct _VQAClipper {
	unsigned long Width;
	unsigned long Height;
} VQAClipper;


/* VQACBNode: A circular list of codebook buffers, used by the load task.
 *            If the data is compressed, it is loaded into the end of the
 *            buffer and the compression flags is set. Otherwise the data
 *            is loaded into the start of the buffer.
 *            (Make sure this structure's size is always DWORD aligned.)
 *
 * Buffer   - Pointer to Codebook data.
 * Next     - Pointer to next VQACBNode in the codebook list.
 * Flags    - Used by the drawer to tell if certain operations have been
 *            performed on this codebook, such as downloading to VRAM,
 *            or pre-scaling it. This field is cleared by the Loader when a
 *            new codebook is loaded.
 * CBOffset - Offset into the buffer of the compressed data.
 */
typedef struct _VQACBNode {
	unsigned char     *Buffer;
	struct _VQACBNode *Next;
	struct _VQACBNode *Prev;
	unsigned long     Flags;
	unsigned long     CBOffset;
	int               CodebookSize;
} VQACBNode;

/* VQACBNode flags */
#define	VQACBB_DOWNLOADED 0 /* Download codebook to VRAM (XMODE VRAM) */
#define VQACBB_CBCOMP     1 /* Codebook is compressed */
#define VQACBB_ALTPTR     2 /* Use alternative pointer buffer */
#define VQACBB_CBFULL     3 /* Codebook is full */
#define	VQACBF_DOWNLOADED (1<<VQACBB_DOWNLOADED)
#define	VQACBF_CBCOMP     (1<<VQACBB_CBCOMP)
#define	VQACBF_ALTPTR     (1<<VQACBB_ALTPTR)
#define	VQACBF_CBFULL     (1<<VQACBB_CBFULL)


/* VQAFrameNode: A circular list of frame buffers, filled in by the load
 *               task. If the data is compressed, it is loaded into the end
 *               of the buffer and the compress flag is set. Otherwise, it's
 *               loaded into the start of the buffer.
 *               (Make sure this structure's size is always DWORD aligned.)
 *
 * Pointers    - Pointer to the vector pointer data.
 * Codebook    - Pointer to VQACBNode list entry for this frame.
 * Palette     - Pointer to an array of palette colors (R,G,B).
 * Next        - Pointer to the next entry in the Frame Buffer List.
 * Flags       - Inter-process communication flags for this frame (see below)
 *               set by Loader, cleared by Flipper.
 * FrameNum    - Number of this frame in the animation.
 * PtrOffset   - Offset into buffer of the compressed vector pointer data.
 * PalOffset   - Offset into buffer of the compressed palette data.
 * PaletteSize - Size of the palette for this frame (in bytes).
 */
typedef struct _VQAFrameNode {
	unsigned char        *Pointers;
	VQACBNode            *Codebook;
	unsigned char        *Palette;
	struct _VQAFrameNode *Next;
	struct _VQAFrameNode *Prev;
	unsigned long        Flags;
	unsigned long        PrevFlags;
	long                 FrameNum;
	long                 PtrOffset;
	long                 PalOffset;
	long                 PaletteSize;
} VQAFrameNode;

/* FrameNode flags */
#define	VQAFRMB_LOADED  0 /* Frame loaded */
#define	VQAFRMB_KEY     1 /* Key Frame (must be drawn) */
#define	VQAFRMB_PALETTE 2 /* Palette needs set */
#define VQAFRMB_PALCOMP 3 /* Palette is compressed */
#define VQAFRMB_PTRCOMP 4 /* Vector pointer data is compressed */
#define VQAFRMB_RSDCOMP 5 /* Vector pointer data is RSD compressed */
#define VQAFRMB_ALTPTR  6 /* Use alternate pointer buffer */
#define VQAFRMB_HOLD    7 /* Frame is being held (redrawn) */
#define VQAFRMB_LOOPED  8 /* Frame is the first after a loop repeat */
#define VQAFRMB_LOOPJMP 9 /* Frame is the first after a jump to another loop */
#define VQAFRMB_CHUNKS 10 /* Frame has MFCI/MSCI chunks awaiting dispatch */
#define VQAFRMB_11     11
#define	VQAFRMF_LOADED  (1<<VQAFRMB_LOADED)
#define	VQAFRMF_KEY     (1<<VQAFRMB_KEY)
#define	VQAFRMF_PALETTE (1<<VQAFRMB_PALETTE)
#define	VQAFRMF_PALCOMP (1<<VQAFRMB_PALCOMP)
#define	VQAFRMF_PTRCOMP (1<<VQAFRMB_PTRCOMP)
#define VQAFRMF_RSDCOMP	(1<<VQAFRMB_RSDCOMP)
#define VQAFRMF_ALTPTR	(1<<VQAFRMB_ALTPTR)
#define VQAFRMF_HOLD	(1<<VQAFRMB_HOLD)	/// Set with VQADRWF_HOLD whenever the drawer redraws this frame instead of advancing.
#define VQAFRMF_LOOPED	(1<<VQAFRMB_LOOPED)	/// Loader wrapped to the loop start (endjump == -1); fires VQAEVENT_LOOPED.
#define VQAFRMF_LOOPJMP	(1<<VQAFRMB_LOOPJMP)	/// Loader jumped to a different loop (endjump != -1); fires VQAEVENT_LOOPJUMP.
#define VQAFRMF_CHUNKS	(1<<VQAFRMB_CHUNKS)	/// A custom chunk landed on this frame; VQA_DispatchFrameChunks delivers it.
#define VQAFRMF_11		(1<<VQAFRMB_11)	/// Set by User_Update when a paused forced redraw completes. Nothing reads it.


/* VQALoader: Data needed exclusively by the Loader.
 *            (Make sure this structure's size is always DWORD aligned.)
 *
 * CurCB         - Pointer to the current codebook node to load data into.
 * FullCB        - Pointer to the last fully-loaded codebook node.
 * CurFrame      - Pointer to the current frame node to load data into.
 * NumPartialCB  - Number of partial codebooks accumulated.
 * PartialCBSize - Size of partial codebook (LCW'd or not), in bytes
 * CurFrameNum   - The number of the frame being loaded by the Loader.
 * LastCBFrame   - Last frame in the animation that contains a partial CB
 * LastFrameNum  - Number of the last loaded frame
 * WaitsOnDrawer - Number of wait states Loader hits waiting on the Drawer
 * WaitsOnAudio  - Number of wait states Loader hits waiting on HMI
 * FrameSize     - Size of the last frame in bytes.
 * MaxFrameSize  - Size of the largest frame in the animation.
 * CurChunkHdr   - Chunk header of the chunk currently being processed.
 */
typedef struct _VQALoader {
	VQACBNode    *CurCB;
	VQACBNode    *FullCB;
	VQACBNode    *PrevCB;
	VQAFrameNode *CurFrame;
	long         NumPartialCB;
	long         PartialCBSize;
	int          CBSize;
	long         CurFrameNum;
//	long         LastCBFrame;
	long         LastFrameNum;
	long         WaitsOnDrawer;
	long         WaitsOnAudio;
	long         FrameSize;
	long         MaxFrameSize;
	ChunkHeader  CurChunkHdr;
} VQALoader;


/* VQADrawer: Data needed exclusively by the Drawer.
 *            (Make sure this structure's size is always DWORD aligned.)
 *
 * CurFrame       - Pointer to the current frame to draw.
 * Flags          - Flags for the draw routines (IE: VQADRWF_SETPAL)
 * Display        - Pointer to DisplayInfo structure for active video mode.
 * ImageBuf       - Buffer to un-vq into, must be DWORD aligned.
 * ImageWidth     - Width of Image buffer (in pixels).
 * ImageHeight    - Height of Image buffer (in pixels).
 * X1,Y1,X2,Y2    - Coordinates of image corners (in pixels).
 * ScreenOffset   - Offset into screen memory, for centering small images.
 * CurPalSize     - Size of the current palette in bytes.
 * Palette_24     - Copy of the last-loaded palette
 * Palette_15     - 15-bit version of Palette_24, for 32K-color modes
 * BlocksPerRow   - # of VQ blocks per row for this resolution/block width.
 * NumRows        - # of rows of VQ blocks for this resolution/block height.
 * NumBlocks      - Total number of blocks in the image.
 * MaskStart      - Pointer index of start of mask rectangle.
 * MaskWidth      - Width of mask rectangle, in blocks.
 * MaskHeight     - Height of mask rectangle, in blocks.
 * LastTime       - The time when that last frame was drawn.
 * LastFrame      - The number of the last frame selected.
 * LastFrameNum   - Number of the last frame drawn.
 * DesiredFrame   - The number of the frame that should be drawn.
 * NumSkipped     - Number of frames skipped.
 * WaitsOnFlipper - Number of wait states Drawer hits waiting on the Flipper.
 * WaitsOnLoader  - Number of wait states Drawer hits waiting on the Loader.
 */
typedef struct _VQADrawer {
	VQAFrameNode  *CurFrame;
	unsigned long Flags;
//	DisplayInfo   *Display;
	unsigned char *ImageBuf;
	long          ImageWidth;
	long          ImageHeight;
	long          X1,Y1;//,X2,Y2;
	long          ScreenOffset;
	long          CurPalSize;
	unsigned char Palette_24[768];
	unsigned char Palette_15[512];
	long          BlocksPerRow;
	long          NumRows;
	long          NumBlocks;
//	long          MaskStart;
//	long          MaskWidth;
//	long          MaskHeight;
//	long          LastTime;
//	long          LastFrame;
	long          LastFrameNum;
	long          DesiredFrame;
//	long          NumSkipped;
//	long          WaitsOnFlipper;
	long          WaitsOnLoader;
} VQADrawer;

/* Drawer flags */
#define	VQADRWB_SETPAL 0  /* Set palette */
#define	VQADRWF_SETPAL (1<<VQADRWB_SETPAL)
#define VQADRWF_STEP      (1 << 1)
#define VQADRWF_REPAINT   (1 << 2)	/// Hold the current frame instead of returning an error.
#define VQADRWF_HOLD      (1 << 3)	/// Stay on the current frame.
#define VQADRWF_FORCEDRAW (1 << 4)	/// One-shot redraw of the current frame while paused.


/* VQAFlipper: Data needed exclusively by the page-flipper.
 *             (Make sure this structure's size is always DWORD aligned.)
 *
 * CurFrame     - Pointer to current flipper frame.
 * LastFrameNum - Number of last flipped frame
 * pad          - DWORD alignment padding.
 */
typedef struct _VQAFlipper {
	VQAFrameNode *CurFrame;
	long         LastFrameNum;
} VQAFlipper;


/* VQAAudio: Data needed exclusively by audio playback.
 *           (Make sure this structure's size is always DWORD aligned.)
 *
 * Buffer         - Pointer to the audio buffer.
 * AudBufPos      - Current audio buffer position, for copying data in buffer.
 * IsLoaded       - Inter-process communication flag:
 *                  0 = is loadable, 1 = is not. Loader sets it when it
 *                  loads, audio callback clears it when it plays one.
 * NumAudBlocks   - Number of HMI blocks in the audio buffer.
 * CurBlock       - Current audio block
 * NextBlock      - Next audio block
 * TempBuf        - Pointer to temp buffer for loading/decompressing audio
 *                  data.
 * TempBufLen     - Number of bytes loaded into temp buffer.
 * TempBufSize    - Size of temp buffer in bytes.
 * Flags          - Various audio flags. (See below)
 * PlayPosition   - HMI's current buffer position.
 * SamplesPlayed  - Total samples played.
 * NumSkipped     - Count of buffers missed.
 * SampleRate     - Recorded sampling rate of the track.
 * Channels       - Number of channels in the track.
 * BitsPerSample  - Bit resolution size of sample (8,16)
 * BytesPerSec    - Recorded data transfer for one second.
 * ADPCM_Info     - ADPCM decompression information structure.
 * DigiHandle     - HMI digital device handle.
 * SampleHandle   - HMI sample handle.
 * DigiTimer      - HMI digital fill handler timer handle.
 * sSOSSampleData - HMI sample structure.
 * DigiCaps       - HMI sound card digital capabilities.
 * DigiHardware   - HMI sound card digital hardware settings.
 * sSOSInitDriver - HMI driver initialization structure.
 * TimerHandle		- Handle to Windows multi-media timer
 * SoundTimerHandle- Handle to extra Windows mm timer req. for direct sound
 * SecondaryBufferPtr - Pointer to out direct sound secondary buffer
 * DSBuffFormat		- WAVEFORMATEX structure for direct sound
 * BufferDesc		- Description structure for setting up direct sound buffers
 * SecondaryBufferSize - length in bytes of our secondary sound buffer
 * EndLastAudioChunk - Offset into secondary buffer of the end of the last chunk of audio copied in
 * ChunksMovedToAudioBuffer - Total number of HMIBufSize chunks moved into the secondary buffer
 * LastChunkPosition - Offset position of last chunk copied to secondary buffer
 * CreatedSoundObject - True if we had to create our own direct sound object
 * CreatedSoundBuffer - True if we had to create out own direct sound primary buffer
 */
typedef struct _VQAAudio {
	unsigned char      *Buffer;
	unsigned long      AudBufPos;
	bool               *IsLoaded;
	short              *BlockRepeats;
	unsigned long      NumAudBlocks;
	unsigned long      Block1;
	unsigned long      Block2;
	unsigned char      *TempBuf;
	unsigned long      BufferOffset;
	unsigned long      TempBufLen;
	unsigned long      TempBufSize;
	void              *HMIBuffer;
	unsigned long      Flags;
	unsigned long      PlayPosition;
	unsigned long      BufferPosition;

	/// Unused
	int                field_3C;

	unsigned long      BytesPerSec;
	VQASOS             ADPCM_Info;
} VQAAudio;

/* Audio flags. */
#define VQAAUDB_DIGIINIT	0  /* HMI digital driver initialized (2 bits) */
#define VQAAUDB_TIMERINIT	2  /* HMI timer system initialized (2 bits) */
#define VQAAUDB_HMITIMER	4  /* HMI timer callback initialized (2 bits) */
#define VQAAUDB_ISPLAYING	6  /* Audio playing flag. */
#define VQAAUDB_ISREPEATING	7  /* Audio repeating flag. */
#define VQAAUDB_MEMLOCKED	30 /* Audio memory page locked. */
#define VQAAUDB_MODLOCKED	31 /* Audio module page locked. */

#define VQAAUDF_ISPLAYING	(1<<VQAAUDB_ISPLAYING)
#define VQAAUDF_ISREPEATING	(1<<VQAAUDB_ISREPEATING)
#define VQAAUDF_ISENDOFFILE	(1<<8)
#define VQAAUDF_ISDONE		(1<<9)	/// Track has drained; CopyAudio returns 0 from here.
#define VQAAUDF_ISSTARVED	(1<<10)	/// Loader is falling behind under VQAOPTF_WAITFILL.
#define VQAAUDF_MEMLOCKED	(1<<VQAAUDB_MEMLOCKED)
#define VQAAUDF_MODLOCKED	(1<<VQAAUDB_MODLOCKED)

// flags that used to apply to VQAData Flags member in the old version
/* VQAData flags */
#define VQADATB_LSLEEP      0 /* Loader sleep state. */
#define VQADATB_DDONE       1 /* Drawer done flag. (0 = done) */
#define VQADATB_LDONE       2 /* Loader done flag. (0 = done) */
#define VQADATB_PRIMED      3 /* Buffers are primed. */
#define VQADATB_PAUSED      6 /* The player is paused. */
#define VQADATF_LSLEEP      (1<<VQADATB_LSLEEP)
#define VQADATF_DDONE       (1<<VQADATB_DDONE)
#define VQADATF_LDONE       (1<<VQADATB_LDONE)
#define VQADATF_PRIMED      (1<<VQADATB_PRIMED)
#define VQADATF_BUFCONFIG   (1<<4)
#define VQADATF_ALTIMG      (1<<5)	// Uses alternative image buffer.
#define VQADATF_PAUSED      (1<<VQADATB_PAUSED)
#define VQADATF_LOOPED      (1<<7)	/// Loader wrapped at a loop end; drawer has not caught up.
#define VQADATF_LOOPJMP     (1<<8)	/// The pending wrap is a jump to another loop.
#define VQADATF_AUDIOSYNC   (1<<9)
#define VQADATF_REFILLED    (1<<10)	/// Waitfill refill finished; the drawer may resume.
#define VQADATF_FRAMESTALL  (1<<11)

// AltBufferFlags
#define VQAABUFF_ALTCB		(1<<0)	// use alternative codebook buffer
#define VQAABUFF_ALTPTR		(1<<1)	// use alternative frame pointer buffer
#define VQAABUFF_ALTLOOP	(1<<2)	// use alternative loop buffer

// Draw_Frame and Page_Flip functions must be this type
typedef VQAErrorType (*VQAD_FUNC)(VQAHandle *vqa);
typedef VQAErrorType (*VQAP_FUNC)(VQAHandle *vqa);

struct VQA_Array_Data {
	void			**Ptr;
	char			pad[0x8];
};

struct VQALoopInfo {
	struct HEADER {
		unsigned short Count;
		unsigned long Flags;
		unsigned short Pad;
	};
	HEADER Header;

	struct DATA {
		unsigned short StartFrame;
		unsigned short EndFrame;
	};
	DATA *Data;
};

#define VQALOOPF_DATAVALID  (1<<0)
#define VQALOOPF_2          (1<<1)


struct VQAPaletteInfo {
	struct HEADER {
		unsigned short Count;
		unsigned long Flags;
		unsigned short Pad;
	};
	HEADER Header;

	struct DATA {
		unsigned short Frame;
	};
	DATA *Data;
};

struct VQACodebookInfo {
	struct HEADER {
		unsigned short Count;
		unsigned short Groupsize;
		unsigned short Flags;
		unsigned short Pad;
	};
	HEADER Header;

	struct DATA {
		unsigned short Frame;
		int     Size;
	};
	DATA *Data;
};


struct VQAMFCInfo {
	struct HEADER {
		unsigned long StaticCount;
		unsigned long Count;

		/// Unused
		int     field_8;
		int     field_C;
	};
	HEADER Header;

	struct DATA {
		/*
		 * The frame this static entry applies to. It is compared against a loop's
		 * start and end frames to decide whether the entry falls inside that loop.
		 */
		int     KeyFrame;

		unsigned long ChunkID;
		char    Pad[0x10];
	};
	DATA *StaticData;

	struct TABLE {
		unsigned long ChunkID;

		/*
		 * The period of this chunk type, in frames. It divides the frame buffer
		 * count to size the ring of entries; zero disables the entry entirely.
		 */
		int     FrameInterval;

		/*
		 * The size in bytes of one entry's buffer. The ring's entries are carved
		 * out of a single allocation holding this many bytes per slot.
		 */
		int     EntrySize;

		/// Unused
		int     field_C;
	};
	TABLE *Table;

	struct DATA2 {
		unsigned long Count;

		/*
		 * The ring buffer write cursor. It picks the slot the next chunk of this
		 * type is stored into, wrapping back to zero at Count.
		 */
		int     WriteIndex;

		struct DATA {
			char    *Buffer;
			unsigned long Size;
			long Frame;
		};
		DATA *Data;
	};
	DATA2 *Data2;
};

struct VQAMSCInfo {
	struct HEADER {
		unsigned long Count;

		/// Unused
		int     field_4;
	};
	HEADER Header;

	struct TABLE {
		unsigned long ChunkID;

		/*
		 * The size in bytes of one entry's buffer. Note that this sits one slot
		 * earlier than the equivalent field of the MFCI table.
		 */
		int     EntrySize;

		/// Unused
		int     field_8;
		int     field_C;
	};
	TABLE *Table;

	struct DATA2 {
		unsigned long Count;

		/*
		 * The ring buffer write cursor. It picks the slot the next chunk of this
		 * type is stored into, wrapping back to zero at Count.
		 */
		int     WriteIndex;

		struct DATA {
			char    *Buffer;
			unsigned long Size;
			long Frame;
		};
		DATA *Data;
	};
	DATA2 *Data2;
};


struct VQALoopCache {
	char			*Ptr;
	int				Size;
	char			*Buffer;
	int				Bytes;
	int				Offset;
	int				Min;
	int				Max;
	int				ID;
};

/* VQAHandleP: Private VQA file handle. Must be obtained by calling
 *             VQA_Alloc() and freed through VQA_Free(). This is the only
 *             legal way to obtain and dispose of a VQAHandle.
 *
 * VQAio     - Something meaningful to the IO manager. (See DOCS)
 * IOHandler - IO handler callback.
 * VQABuf    - Pointer to internal data buffers.
 * Config    - Configuration structure.
 * Header    - VQA header structure.
 * vocfh     - Override audiotrack file handle.
 */
typedef struct _VQAHandleP {
	unsigned short	Version;
	unsigned short	ImageWidth;
	unsigned short	ImageHeight;

	/*
	 * Reserved space between the image dimensions and the image buffer pointer.
	 * Nothing reads it.
	 */
	short			field_6;

	void *			ImageBuf;
	unsigned short	ColorMode;
	unsigned short	FrameRate;
	long			NumFrames;
	int				LoadedFrames;
	int				DrawnFrames;
	int				SkippedFrames;
	int				StartTime;
	int				EndTime;

	/*
	 * The value the unused callback returned on the previous tick. It starts at
	 * -1 and is refreshed on every tick, so that the change since the last tick
	 * can be measured.
	 */
	int				LastCallbackCount;

	/*
	 * The largest change ever seen between two successive unused callback values,
	 * latched as a worst case statistic.
	 */
	int				MaxCallbackDelta;

	int				MemUsed;
	int				AudioHandleIndex;
	int				RepeatedBuffers;
	unsigned short	SampleRate;
	unsigned char	Channels;
	unsigned char	BitsPerSample;
	VQAConfig     Config;

	/*
	 * The name of the movie being played, trimmed to its last 31 characters. It
	 * is kept for display purposes only.
	 */
	char			Filename[32];

	VQAHeader     Header;
	VQAClipper		Clipper;

	/*
	 * Reserved space between the clipper and the stop frame. Nothing reads it.
	 */
	short			field_11A;

	int				StopFrame;
	int				LoopID;
	int				LoopIterations;
	int				LoopStartFrame0;
	int				LoopEndFrameMode2;
	int				LoopEndFrameJump;
	int				LoopIterationsJump;
	int				LoopEndFrameNormal;
	int				LoopStartFrame1;
	int				LoopEndFrame2;
	int				LoopStartFrame2;
	int				TickOffset;

	/// Unused
	int				field_14C;

	unsigned long	Flags;
	unsigned long	AltBufferFlags;
	void *			AltImageBuf;
	int				AltImageWidth;
	int				AltImageHeight;
	VQALoader		Loader;
	VQADrawer		Drawer;
	VQAFlipper		Flipper;
	VQAFrameNode	*FrameData;
	void *			AltPtrBuffer;
	VQACBNode		*CBData;
	void *			AltCBBuffer;
	VQALoopCache	LoopCache;
	VQALoopInfo		LoopInfo;
	VQAMFCInfo		MFCInfo;
	VQAMSCInfo		MSCInfo;
	VQACodebookInfo CodebookInfo;
	VQAPaletteInfo	PaletteInfo;
	long			*Foff;
	long			Max_CB_Size;
	long			Max_Ptr_Size;
	long			Max_Pal_Size;
	int				CBBufferSize;
	int				PtrBufferSize;
	VQAD_FUNC		Draw_Frame;
	VQAP_FUNC		Page_Flip;
	UNVQ_FUNC		UnVQ1;
	UNVQ_FUNC		UnVQ2;
	#if(VQAAUDIO_ON)
	VQAAudio		Audio;
	#endif
} VQAHandleP;
#pragma pack(pop)


/* Load function flags. */
#define VQALOADB_NOPAL	0 /* Don't load palette */
#define VQALOADB_NOSND	1 /* Don't load sound */
#define VQALOADB_NOPTR	2 /* Don't load pointers */
#define VQALOADB_NOPCB	3 /* Don't load codebook */
#define VQALOADB_NOFCB	4 /* Don't load full codebook */
#define VQALOADB_NOLFR	5 /* Don't load loop frame */

#define VQALOADF_NOPAL		(1<<VQALOADB_NOPAL)
#define VQALOADF_NOSND		(1<<VQALOADB_NOSND)
#define VQALOADF_NOPTR		(1<<VQALOADB_NOPTR)
#define VQALOADF_NOPCB		(1<<VQALOADB_NOPCB)
#define VQALOADF_NOFCB		(1<<VQALOADB_NOFCB)
#define VQALOADF_NOLFR		(1<<VQALOADB_NOLFR)

/*---------------------------------------------------------------------------
 * FUNCTION PROTOTYPES
 *-------------------------------------------------------------------------*/

/* Loader/Drawer system. */
VQAErrorType VQA_LoadFrame(VQAHandle *vqa);
VQAErrorType VQA_Configure_Drawer(VQAHandleP *vqap);
long User_Update(VQAHandle *vqa);

/* Timer system. */
void VQA_SetTimer(VQAHandleP *vqap, long time);
void VQA_StepTimer(VQAHandleP *vqap, long step);
unsigned long VQA_GetTime(VQAHandleP *vqap);
unsigned long VQA_GetMovieTime(VQAHandle *vqa);

/* Audio system. */
#if(VQAAUDIO_ON)
VQAErrorType VQA_OpenAudio(VQAHandleP *vqap);
void VQA_CloseAudio(VQAHandleP *vqap);
void VQA_StartAudio(VQAHandleP *vqap);
void VQA_PauseAudio(VQAHandleP *vqap);
void VQA_StopAudio(VQAHandleP *vqap);
long CopyAudio(VQAHandleP *vqap);
long __cdecl VQA_AudioFillCallback(VQAHandleP *vqap);
long __cdecl VQA_AudioDoneCallback(VQAHandleP *vqap, unsigned long);
#endif

/* Debugging system. */
void VQA_InitMono(VQAHandleP *vqap);
void VQA_UpdateMono(VQAHandleP *vqap);

VQAErrorType AllocBuffers(VQAHandleP *vqap);
void FreeBuffers(VQAHandleP *vqap);

#endif /* VQAPLAYP_H */

