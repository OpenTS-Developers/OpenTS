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
*        C O N F I D E N T I A L -- W E S T W O O D  S T U D I O S
*
*----------------------------------------------------------------------------
*
* PROJECT
*     VQA player library. (32-Bit protected mode)
*
* FILE
*     loader.c
*
* DESCRIPTION
*     Stream loading and pre-processing.
*
* PROGRAMMER
*     Bill Randolph
*     Denzil E. Long, Jr.
*
* DATE
*     August 21, 1995
*
*----------------------------------------------------------------------------
*
* PUBLIC
*     VQA_Open      - Open a VQA file to play.
*     VQA_Close     - Close an opened VQA file.
*     VQA_LoadFrame - Load the next video frame from the VQA data stream.
*     VQA_SeekFrame - Position the movie stream to the specified frame.
*
* PRIVATE
*     AllocBuffers  - Allocates the numerous VQA play buffers
*     FreeBuffers   - Frees the VQA play buffers
*     PrimeBuffers  - Pre-Load the internal buffers.
*     Load_FINF     - Loads the Frame Info Table.
*     Load_VQHD     - Loads a VQA Header.
*     Load_CBF0     - Loads a full, uncompressed codebook
*     Load_CBFZ     - Loads a full, compressed codebook
*     Load_CBP0     - Loads a partial uncompressed codebook
*     Load_CBPZ     - Loads a partial compressed codebook
*     Load_CPL0     - Loads an uncompressed palette
*     Load_CPLZ     - Loads a compressed palette
*     Load_VPT0     - Loads uncompressed pointers
*     Load_VPTZ     - Loads compressed pointers
*     Load_VQF      - Loads a VQ Frame chunk
*     Load_SND0     - Loads an uncompressed sound chunk
*     Load_SND1     - Loads a compressed sound chunk
*     Load_AudFrame - Loads blocks from separate audio file, if needed.
*
****************************************************************************/

#include "cmp.h"
#include <stdio.h>
#include <memory.h>
#include "vqaplayp.h"
#include "../lcw.h"

/*---------------------------------------------------------------------------
 * PRIVATE DECLARATIONS
 *-------------------------------------------------------------------------*/

long Load_FINF(VQAHandleP *vqap, unsigned long iffsize);
long Load_CINF(VQAHandleP *vqap);
long Load_PINF(VQAHandleP *vqap);
long Load_LINF(VQAHandleP *vqap);
long Load_CLIP(VQAHandleP *vqap, unsigned long iffsize);
long Load_MFCI(VQAHandleP *vqap);

long Load_MSCI(VQAHandleP *vqap);

_STATIC long Load_VQF(VQAHandleP *vqap, unsigned long frame_iffsize, char flags);
_STATIC long Load_CBF0(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_CBFZ(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_CBP0(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_CBPZ(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_CPL0(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_CPLZ(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_VPT0(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_VPTZ(VQAHandleP *vqap, unsigned long iffsize);

#if(VQAAUDIO_ON)
_STATIC long Load_SND0(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_SND1(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_SND2(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_SN2J(VQAHandleP *vqap, unsigned long iffsize);
#endif

_STATIC long Load_LINH(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_LIND(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_CINH(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_CIND(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_PINH(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_PIND(VQAHandleP *vqap, unsigned long iffsize);

_STATIC long Load_MSCH(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_MSCT(VQAHandleP *vqap, unsigned long iffsize);

_STATIC long Load_MFCH(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_MFCD(VQAHandleP *vqap, unsigned long iffsize);
_STATIC long Load_MFCT(VQAHandleP *vqap, unsigned long iffsize);

_STATIC long VQA_GetCodebookSize(VQAHandleP *vqap, long framenum);
_STATIC long VQA_MFCIPrepare(VQAHandleP *vqap, unsigned long index);
_STATIC long VQA_MFCICalcCount(VQAHandleP *vqap, unsigned long chunkid, long count, unsigned long value);
_STATIC VQABool VQA_MFCISpansLoop(VQAHandleP *vqap, unsigned long chunkid, unsigned long value);

_STATIC long VQA_MSCIPrepare(VQAHandleP *vqap, unsigned long index);
_STATIC long VQA_MSCIReadData(VQAHandleP *vqap, unsigned long iffsize, long index);

_STATIC long VQA_MFCIIndexFromChunkID(VQAHandleP *vqap, unsigned long chunkid);
_STATIC long VQA_MFCIReadData(VQAHandleP *vqap, unsigned long iffsize, long index);
_STATIC long VQA_MSCIIndexFromChunkID(VQAHandleP *vqap, unsigned long chunkid);

_STATIC long VQA_GetPaletteFrameRange(VQAHandleP *vqap, long framenum, long * first_frame, long * last_frame);
_STATIC long VQA_SeekLoop(VQAHandleP *vqap, long framenum, long flags);
_STATIC long VQA_GetCodebookFrameRange(VQAHandleP *vqap, long framenum, long *first_frame, long *last_frame);
VQABool VQA_IsFrameStartOfLoop(VQAHandleP *vqap, long framenum);
long VQA_ReloadPalette(VQAHandleP *vqap, long framenum, int force);
_STATIC long VQA_LoadLoop(VQAHandleP *vqap, long framenum);

long __cdecl Memory_VQA_Stream_Handler(VQAHandle *vqa, long action, void *buffer, long nbytes);
_STATIC long VQA_LoadFrame_Internal(VQAHandleP *vqap, long flags);

long VQA_SeekGroup(VQAHandleP *vqap, long framenum, long groupsize, VQABool preloadaudio, VQABool reset_state, VQABool &skipcodebook);


/****************************************************************************
*
* NAME
*     VQA_LoadFrame - Load the next video frame from the VQA data stream.
*
* SYNOPSIS
*     Error = VQA_LoadFrame(VQA)
*
*     long VQA_LoadFrame(VQAHandle *);
*
* FUNCTION
*     The codebook is split up such that the last frame of every group gets
*     a new, complete codebook, ready for the next group.  The first codebook
*     in the VQA is a full codebook, and goes with the first frame's data.
*     Partial codebooks are stored per frame after that, and they add up to
*     a full codebook just before the first frame for the next group is read.
*
*     (Currently, this routine can read either the older non-frame-grouped
*     VQA file format, or the new frame-chunk format.  For the older format,
*     it's assumed that the last chunk in a frame is the pointer data.)
*
*     This routine also does a sort of "cooperative multitasking".  If the
*     Loader hits a "wait state" where it has to wait on the audio to finish
*     playing before it can continue to load, it sets a "sleep" flag and
*     just returns.  The sleep flag is checked on entry to see if it needs
*     to jump to the proper execution point. This may improve performance on
*     some platforms, but it also allows the Loader to be called regardless
*     of the size of the buffers; if the buffers fill up or the audio fails
*     to play, the Loader won't just get stuck.
*
* INPUTS
*     VQA - Pointer to VQAHandle structure.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/
long VQA_LoadFrame(VQAHandleP *vqap, long flags)
{
	VQAConfig *config;
	VQALoader *loader;
	VQALoopCache *cache;

	int frame;
	int loop_bytes;
	int scan_frame;
	int tocache;
	VQA_H_FUNC handler;
	unsigned int rc;
	int val4;
	int fsize;
	VQA_H_FUNC oldhandler;
	bool restore_handler;

	oldhandler = NULL;
	long foffset = 0;

	config = &vqap->Config;
	loader = &vqap->Loader;
	long *foff = vqap->Foff;
	frame = loader->CurFrameNum;


	restore_handler = false;

	if ((config->OptionFlags & VQAOPTF_LOOPS) && (vqap->AltBufferFlags & VQAABUFF_ALTLOOP) && vqap->LoopID >= 0) {

		if (frame < vqap->NumFrames) {
			foffset = VQAFRAME_OFFSET(foff[frame]);
		}

		cache = &vqap->LoopCache;
		if (frame == vqap->LoopStartFrame0 && vqap->LoopID != cache->ID) {
			if (frame != cache->Min) {
				cache->Buffer = (char *)foffset;
				cache->Bytes = 0;
				cache->Offset = 0;
			}
			cache->ID = vqap->LoopID;
			cache->Min = vqap->LoopStartFrame0;
			cache->Max = -1;
			loop_bytes = 0;

			scan_frame = vqap->LoopStartFrame0;
			for (;scan_frame <= vqap->LoopEndFrameMode2; scan_frame++) {

				if (scan_frame < vqap->NumFrames - 1) {
					loop_bytes += VQAFRAME_OFFSET(foff[scan_frame+1]) - VQAFRAME_OFFSET(foff[scan_frame]);
				} else {
					if (config->StreamHandler((VQAHandle *)vqap, VQACMD_SIZE, &fsize, 0)) {
						cache->Max = scan_frame - 1;
						break;
					}
					loop_bytes += fsize - VQAFRAME_OFFSET(foff[vqap->LoopEndFrameMode2]);
				}
				if (loop_bytes >= cache->Size) {
					cache->Max = scan_frame - 1;
					break;
				}


			}

			if (loop_bytes < cache->Size && cache->Max == -1) {
				cache->Max = vqap->LoopEndFrameMode2;
			}
		}
		if ( frame >= cache->Min && frame <= cache->Max )
		{
			if (foffset == (long)cache->Buffer + cache->Bytes) {
				if (frame < vqap->NumFrames - 1) {
					tocache = VQAFRAME_OFFSET(foff[frame + 1]) - foffset;
				} else {
					if (!config->StreamHandler((VQAHandle *)vqap, VQACMD_SIZE, &fsize, 0) ) {
						tocache = fsize - foffset;
					} else {
						tocache = 0;
					}
				}

				if (tocache > 0) {
					if (config->StreamHandler != Memory_VQA_Stream_Handler) {
						config->StreamHandler((VQAHandle *)vqap, VQACMD_SEEK, NULL, foffset);
					}
					if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, &cache->Ptr[cache->Offset], tocache)) {
						return -4;
					}
					cache->Bytes += tocache;
				}
			}

			if ( foffset < (long)cache->Buffer + cache->Bytes )
			{
				restore_handler = true;
				cache->Offset = foffset - (long)cache->Buffer;
				oldhandler = config->StreamHandler;
				config->StreamHandler = Memory_VQA_Stream_Handler;
			}
		}
	}


	if (VQA_IsFrameStartOfLoop(vqap, frame)) {
		if ((flags & VQALOADF_NOLFR) == 0) {
			vqap->Loader.FullCB = vqap->Loader.CurCB;
		}
	}

	rc = VQA_LoadFrame_Internal(vqap, flags);

	if (restore_handler == true) {
		config->StreamHandler = oldhandler;
	}

	return rc;
}


/* On-disk ADPCM stream-state record carried by the SN2J chunk. The fields are
 * packed (no padding) to match the 12-byte on-disk layout.
 */
#pragma pack(push,1)
struct VQASN2J {
	short index;
	long  predicted;
	short index2;
	long  predicted2;
};
#pragma pack(pop)


long VQA_LoadFrame_Internal(VQAHandleP *vqap, long flags)
{
	VQAConfig     *config;
	VQALoader     *loader;
	VQADrawer     *drawer;
	VQAFrameNode  *curframe;
	ChunkHeader   *chunk;
	static unsigned long iffsize;

	VQABool      frame_loaded = 0;
	VQABool      loop_loaded = 0;


	long          rc;

	/* Dereference commonly used data members for quicker access. */
	config = &vqap->Config;
	loader = &vqap->Loader;
	drawer = &vqap->Drawer;
	curframe = loader->CurFrame;
	chunk = &loader->CurChunkHdr;

	/*-------------------------------------------------------------------------
	 * LOOP TRANSITION. If we've run off the end of the current loop, rewind to
	 * the loop start and (possibly) jump to a new end point.
	 *-----------------------------------------------------------------------*/
	if (config->OptionFlags & VQAOPTF_LOOPS) {
		if (vqap->LoopIterations > 0 || vqap->LoopIterations == -1) {
			if (loader->CurFrameNum > vqap->LoopEndFrameMode2) {
				long endjump = vqap->LoopEndFrameJump;
				long end2 = vqap->LoopEndFrameMode2;

				/* Stash StopFrame in the (dead past this point) flags
				 * parameter while we rewind. */
				flags = vqap->StopFrame;

				if (endjump != -1) {
					vqap->LoopEndFrameMode2 = endjump;
					vqap->LoopEndFrameJump = -1;
					vqap->StopFrame = endjump;
				}

				/* Rewind to the loop start, reseeding loader state. */
				if (loader->CurFrameNum != vqap->LoopStartFrame0) {
					loader->CurFrameNum = vqap->LoopStartFrame0;
					rc = VQA_LoadLoop(vqap, vqap->LoopStartFrame0);
				} else {
					rc = VQAERR_NONE;
				}

				if (rc == VQAERR_NONE || rc == VQAERR_SLEEPING) {

					vqap->Flags |= VQADATF_LOOPED;

					if (endjump == -1) {
						curframe->Flags |= VQAFRMF_LOOPED;
						if (vqap->LoopIterations != -1) {
							vqap->LoopIterations--;
						}
					} else {
						curframe->Flags |= VQAFRMF_LOOPJMP;
						vqap->LoopEndFrame2 = end2;
						vqap->LoopIterations = vqap->LoopIterationsJump;
						vqap->LoopIterationsJump = -1;
						vqap->Flags |= VQADATF_LOOPJMP;
					}

				} else {
					vqap->LoopEndFrameMode2 = end2;
					vqap->LoopEndFrameJump = endjump;
					vqap->StopFrame = flags;
					loader->CurFrameNum = vqap->LoopEndFrameMode2 + 1;
				}
				return(rc);
			}
		}
	}

	/*-------------------------------------------------------------------------
	 * EOF AUDIO TAIL. We've already read past the StopFrame; service one last
	 * audio copy and report end-of-file.
	 *-----------------------------------------------------------------------*/
	if (loader->CurFrameNum > vqap->StopFrame) {
		if (!(flags & VQALOADF_NOSND) && (config->OptionFlags & VQAOPTF_AUDIO) && !(config->OptionFlags & VQAOPTF_ALTAUDIO)) {
			if (CopyAudio(vqap) == VQAERR_SLEEPING) {
				vqap->Flags |= VQADATF_LSLEEP;
				return(VQAERR_SLEEPING);
			}
			vqap->Flags &= (~VQADATF_LSLEEP);
			vqap->Audio.Flags |= VQAAUDF_ISENDOFFILE;
		}
		return(VQAERR_EOF);
	}

	/*-------------------------------------------------------------------------
	 * THE MAIN LOADER LOOP
	 *-----------------------------------------------------------------------*/

	/* Set when the current chunk's data was skipped (must seek past it)
	 * rather than read from the stream.
	 */
	VQABool skip;

	/* If no buffer is available for loading then return. This allows the
	 * drawer to service one of the buffers more readily. (We'll wait for one
	 * to free up).
	 */
	if ((curframe->Flags & VQAFRMF_LOADED) && !(curframe->Flags & VQAFRMF_11)) {
		loader->WaitsOnDrawer++;
		return(VQAERR_NOBUFFER);
	}

	/* If we're not sleeping, initialize */
	if (!(vqap->Flags & VQADATF_LSLEEP)) {
		loader->FrameSize = 0;

		/* Initialize the codebook ptr for the frame we're about to load:
		 * (This frame's codebook is the last full codebook; we have to init it
		 * now, because if we're on the last frame in a group, we'll get a new
		 * FullCB pointer.)
		 */
		curframe->Codebook = loader->FullCB;
	}

	while (frame_loaded == 0) {

		/* Read new chunk, only if we're not sleeping */
		if (!(vqap->Flags & VQADATF_LSLEEP)) {

			/* Read chunk ID */
			if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, chunk, 8)) {
				return(VQAERR_EOF);
			}

			iffsize = REVERSE_LONG(chunk->size);
			loader->FrameSize += iffsize;
		}

		/* Assume the chunk data will be skipped rather than read; the
		 * pointer chunk handlers clear this when they consume the data.
		 */
		skip = true;

		/* Handle each chunk type */
		switch (chunk->id) {

			/* VQ Normal Frame */
			case ID_VQFR:
				if ((flags & (VQALOADF_NOPAL|VQALOADF_NOPTR|VQALOADF_NOPCB|VQALOADF_NOFCB)) != (VQALOADF_NOPAL|VQALOADF_NOPTR|VQALOADF_NOPCB|VQALOADF_NOFCB)) {
					if (Load_VQF(vqap, iffsize, flags) != VQAERR_NONE) {
						return(VQAERR_READ);
					}
					skip = false;
				}
				frame_loaded = true;
				break;

			/* VQ Key Frame */
			case ID_VQFK:
				if ((flags & (VQALOADF_NOPAL|VQALOADF_NOPTR|VQALOADF_NOPCB|VQALOADF_NOFCB)) != (VQALOADF_NOPAL|VQALOADF_NOPTR|VQALOADF_NOPCB|VQALOADF_NOFCB)) {
					if (Load_VQF(vqap, iffsize, flags) != VQAERR_NONE) {
						return(VQAERR_READ);
					}
					curframe->Flags |= VQAFRMF_KEY;
					skip = false;
				}
				frame_loaded = true;
				break;

			/* VQ loop frame container. */
			case ID_VQFL:
				if ((flags & VQALOADF_NOLFR) == 0) {
					if (Load_VQF(vqap, iffsize, flags) != VQAERR_NONE) {
						return(VQAERR_READ);
					}
					skip = false;
					loop_loaded = true;
					if ((vqap->Header.Flags & VQAHDF_4) && flags & VQALOADF_NOPTR) {
						frame_loaded = true;
					}
				}
				break;


			/* Full uncompressed codebook */
			case ID_CBF0:
				if ((flags & VQALOADF_NOFCB) == 0) {
					if (Load_CBF0(vqap, iffsize) != VQAERR_NONE) {
						return(VQAERR_READ);
					}
					skip = false;
				}
				break;

			/* Full compressed codebook */
			case ID_CBFZ:
				if ((flags & VQALOADF_NOFCB) == 0) {
					if (Load_CBFZ(vqap, iffsize) != VQAERR_NONE) {
						return(VQAERR_READ);
					}
					skip = false;
				}
				break;

			/* Partial uncompressed codebook */
			case ID_CBP0:
				if ((flags & VQALOADF_NOPCB) == 0) {
					if (Load_CBP0(vqap, iffsize) != VQAERR_NONE) {
						return(VQAERR_READ);
					}
					skip = false;
				}
				break;

			/* Partial compressed codebook */
			case ID_CBPZ:
				if ((flags & VQALOADF_NOPCB) == 0) {
					if (Load_CBPZ(vqap, iffsize) != VQAERR_NONE) {
						return(VQAERR_READ);
					}
					skip = false;
				}
				break;

			/* Uncompressed palette */
			case ID_CPL0:
				if ((flags & VQALOADF_NOPAL) == 0) {
					if (Load_CPL0(vqap, iffsize) != VQAERR_NONE) {
						return(VQAERR_READ);
					}

					/* Flag this frame as having a palette. */
					curframe->Flags |= VQAFRMF_PALETTE;
					skip = false;
				}
				break;

			/* Compressed palette */
			case ID_CPLZ:
				if ((flags & VQALOADF_NOPAL) == 0) {
					if (Load_CPLZ(vqap, iffsize) != VQAERR_NONE) {
						return(VQAERR_READ);
					}
					skip = false;

					/* Flag this frame as having a palette. */
					curframe->Flags |= VQAFRMF_PALETTE;
				}
				break;

			/* Uncompressed pointer data */
			case ID_VPT0:
			case ID_VPTD:
				if ((flags & VQALOADF_NOPTR) == 0) {
					if (Load_VPT0(vqap, iffsize) != VQAERR_NONE) {
						return(VQAERR_READ);
					}
					skip = false;
				}
				frame_loaded = true;
				break;

			case ID_VPKZ:
				if ((flags & VQALOADF_NOPTR) == 0) {
					if (Load_VPTZ(vqap, iffsize) != VQAERR_NONE) {
						return(VQAERR_READ);
					}
					skip = false;

					curframe->Flags |= VQAFRMF_KEY;
				}
				frame_loaded = true;
				break;

			/* Pointer data Key (Must draw) */
			case ID_VPTK:
				if ((flags & VQALOADF_NOPTR) == 0) {
					if (Load_VPT0(vqap, iffsize) != VQAERR_NONE) {
						return(VQAERR_READ);
					}
					skip = false;

					curframe->Flags |= VQAFRMF_KEY;
				}
				frame_loaded = true;
				break;

			/* Pointers RSD compressed. */
			case ID_VPTR:
				if ((flags & VQALOADF_NOPTR) == 0) {
					if (Load_VPT0(vqap, iffsize) != VQAERR_NONE) {
						return(VQAERR_READ);
					}
					skip = false;

					curframe->Flags |= VQAFRMF_RSDCOMP;
				}
				frame_loaded = true;
				break;

			/* Pointers RSD, lcw compressed. */
			case ID_VPRZ:
				if ((flags & VQALOADF_NOPTR) == 0) {
					if (Load_VPTZ(vqap, iffsize) != VQAERR_NONE) {
						return(VQAERR_READ);
					}
					skip = false;

					curframe->Flags |= VQAFRMF_RSDCOMP;
				}
				frame_loaded = true;
				break;

			/* Compressed pointer data */
			case ID_VPTZ:
			case ID_VPDZ:
				if ((flags & VQALOADF_NOPTR) == 0) {
					if (Load_VPTZ(vqap, iffsize) != VQAERR_NONE) {
						return(VQAERR_READ);
					}
					skip = false;
				}
				frame_loaded = true;
				break;

			#if(VQAAUDIO_ON)
			/* Uncompressed audio frame.
			 *
			 *  - Make sure the sound load buffer (Audio.TempBuf) is empty; if not
			 *    go into a sleep state.
			 *  - Load the data into TempBuf.
			 */
			case ID_SND0:
				if (!(flags & VQALOADF_NOSND)) {
					if ((config->OptionFlags & VQAOPTF_AUDIO) && !(config->OptionFlags & VQAOPTF_ALTAUDIO)) {

						/* Move the last audio frame to the play buffer. */
						if (CopyAudio(vqap) == VQAERR_SLEEPING) {
							vqap->Flags |= VQADATF_LSLEEP;
							return(VQAERR_SLEEPING);
						} else {
							vqap->Flags &= (~VQADATF_LSLEEP);
						}

						/* Load an uncompressed audio frame. */
						if (Load_SND0(vqap, iffsize) != VQAERR_NONE) {
							return(VQAERR_READ);
						}
						skip = false;
					}
				}
				break;

			case ID_SNA0:
				if (!(flags & VQALOADF_NOSND)) {
					if ((config->OptionFlags & VQAOPTF_AUDIO) && (config->OptionFlags & VQAOPTF_ALTAUDIO)) {

						/* Move the last audio frame to the play buffer. */
						if (CopyAudio(vqap) == VQAERR_SLEEPING) {
							vqap->Flags |= VQADATF_LSLEEP;
							return(VQAERR_SLEEPING);
						} else {
							vqap->Flags &= (~VQADATF_LSLEEP);
						}

						/* Load an uncompressed audio frame. */
						if (Load_SND0(vqap, iffsize) != VQAERR_NONE) {
							return(VQAERR_READ);
						}
						skip = false;
					}
				}
				break;

			/* Compressed audio frame.
			 *
			 *  - Make sure the sound load buffer (Audio.TempBuf) is empty; if not
			 *    go into a sleep state.
			 *  - Load the data into TempBuf.
			 */
			case ID_SND1:
				if (!(flags & VQALOADF_NOSND)) {
					if ((config->OptionFlags & VQAOPTF_AUDIO) && !(config->OptionFlags & VQAOPTF_ALTAUDIO)) {

						/* Move the last audio frame to the play buffer. */
						if (CopyAudio(vqap) == VQAERR_SLEEPING) {
							vqap->Flags |= VQADATF_LSLEEP;
							return(VQAERR_SLEEPING);
						} else {
							vqap->Flags &= (~VQADATF_LSLEEP);
						}

						/* Load a compressed audio frame. */
						if (Load_SND1(vqap, iffsize) != VQAERR_NONE) {
							return(VQAERR_READ);
						}
						skip = false;
					}
				}
				break;

			case ID_SNA1:
				if ((flags & VQALOADF_NOSND) == 0) {

					if ((config->OptionFlags & VQAOPTF_AUDIO) && (config->OptionFlags & VQAOPTF_ALTAUDIO)) {

						/* Move the last audio frame to the play buffer. */
						if (CopyAudio(vqap) == VQAERR_SLEEPING) {
							vqap->Flags |= VQADATF_LSLEEP;
							return(VQAERR_SLEEPING);
						} else {
							vqap->Flags &= (~VQADATF_LSLEEP);
						}

						/* Load a compressed audio frame. */
						if (Load_SND1(vqap, iffsize) != VQAERR_NONE) {
							return(VQAERR_READ);
						}
						skip = false;
					}
				}
				break;

			/* HMI ADPCM compressed audio frame.
			 *
			 *  - Make sure the sound load buffer (Audio.TempBuf) is empty; if not
			 *    go into a sleep state.
			 *  - Load the data into TempBuf.
			 */
			case ID_SND2:
				if (!(flags & VQALOADF_NOSND)) {

					if ((config->OptionFlags & VQAOPTF_AUDIO) && !(config->OptionFlags & VQAOPTF_ALTAUDIO)) {

						/* Move the last audio frame to the play buffer. */
						if (CopyAudio(vqap) == VQAERR_SLEEPING) {
							vqap->Flags |= VQADATF_LSLEEP;
							return(VQAERR_SLEEPING);
						} else {
							vqap->Flags &= (~VQADATF_LSLEEP);
						}

						/* Load a compressed audio frame. */
						if (Load_SND2(vqap, iffsize) != VQAERR_NONE) {
							return(VQAERR_READ);
						}
						skip = false;
					}
				}
				break;

			case ID_SNA2:
				if (!(flags & VQALOADF_NOSND)) {
					if ((config->OptionFlags & VQAOPTF_AUDIO) && (config->OptionFlags & VQAOPTF_ALTAUDIO)) {

						/* Move the last audio frame to the play buffer. */
						if (CopyAudio(vqap) == VQAERR_SLEEPING) {
							vqap->Flags |= VQADATF_LSLEEP;
							return(VQAERR_SLEEPING);
						} else {
							vqap->Flags &= (~VQADATF_LSLEEP);
						}

						/* Load a compressed audio frame. */
						if (Load_SND2(vqap, iffsize) != VQAERR_NONE) {
							return(VQAERR_READ);
						}
						skip = false;
					}
				}
				break;

			/* HMI ADPCM stream-state jump chunk. */
			case ID_SN2J:
				if (!(flags & VQALOADF_NOSND)) {
					if ((config->OptionFlags & VQAOPTF_AUDIO) && !(config->OptionFlags & VQAOPTF_ALTAUDIO)) {
						if (Load_SN2J(vqap, iffsize) != VQAERR_NONE) {
							return(VQAERR_READ);
						}
						skip = false;
					}
				}
				break;
			#endif

			/* Skip any unknown chunks. Try the multi-frame and multi-stream
			 * chunk tables first.
			 */
			default: {
				long index = VQA_MFCIIndexFromChunkID(vqap, chunk->id);
				if (index >= 0) {
					if (VQA_MFCIReadData(vqap, iffsize, index) != VQAERR_NONE) {
						return(VQAERR_READ);
					}
					skip = false;
					break;
				}

				index = VQA_MSCIIndexFromChunkID(vqap, chunk->id);
				if (index >= 0) {
					if (VQA_MSCIReadData(vqap, iffsize, index) != VQAERR_NONE) {
						return(VQAERR_READ);
					}
					skip = false;
					break;
				}

				break;
			}
			}

		/* A pointer chunk has been dealt with; the frame is done. If the
		 * chunk data was suppressed (NOPTR) rather than read, fall into
		 * the generic skip to seek past it first.
		 */

		/* fall through to the generic skip. */
		if (skip == true) {
			if (config->StreamHandler((VQAHandle *)vqap, VQACMD_SEEK, (void *)SEEK_CUR, PADSIZE(iffsize))) {
				return(VQAERR_SEEK);
			}
		}
	}

	/* Update maximum frame size stat. */
	if ((loader->CurFrameNum > 0) && (loader->FrameSize > loader->MaxFrameSize)) {
		loader->MaxFrameSize = loader->FrameSize;
	}

	/*-------------------------------------------------------------------------
	 * SET UP THE FRAME FOR DRAWING.
	 *-----------------------------------------------------------------------*/


	/* If this is a loop-start frame, re-seed the full codebook pointer. */
	if (VQA_IsFrameStartOfLoop(vqap, loader->CurFrameNum)
			&& !(flags & VQALOADF_NOLFR) && !loop_loaded) {
		loader->FullCB = loader->CurCB->Prev;
		curframe->Codebook = loader->FullCB;
	}


	if (!(flags & VQALOADF_NOPTR)) {

		/* Set the frame # */
		curframe->FrameNum = loader->CurFrameNum;

		/* Update data for mono output */
		loader->LastFrameNum = loader->CurFrameNum;

		loader->PrevCB = curframe->Codebook;

		/* Loader is finished with this frame; tell Drawer to draw it */
		curframe->Flags |= VQAFRMF_LOADED;
		loader->CurFrame = curframe->Next;
	}

	loader->CurFrameNum++;
	return(VQAERR_NONE);
}


long PrimeBuffers(VQAHandle *vqa)
{
	VQAHandleP *vqap;
	VQAConfig *config;
	long rc;
	long i;

	vqap = ((VQAHandleP *)vqa);
	config = &((VQAHandleP *)vqa)->Config;

	for (i = 0; i < config->NumFrameBufs; i++) {
		if ((rc = VQA_LoadFrame(((VQAHandleP *)vqa), VQALOADF_NOLFR)) == VQAERR_NONE) {
			((VQAHandleP *)vqa)->LoadedFrames++;
		}
		else if ((rc != VQAERR_NOBUFFER) && (rc != VQAERR_SLEEPING)) {
			return(rc);
		}
	}

	return(VQAERR_NONE);
}


/// <summary>
/// Fetches the multi-frame chunk table entry that handles the given chunk ID.
/// </summary>
/// <returns>Returns with the index of the matching table entry, or -1 if the chunk is not a multi-frame chunk.</returns>
long VQA_MFCIIndexFromChunkID(VQAHandleP *vqap, unsigned long chunkid)
{
	VQAMFCInfo::TABLE *table = vqap->MFCInfo.Table;
	unsigned long count = vqap->MFCInfo.Header.Count;
	for (unsigned long i = 0; i < count; i++) {
		if (chunkid == table[i].ChunkID) {
			return(i);
		}
	}
	return(-1);
}


/// <summary>
/// Reads a multi-frame chunk's payload into the next slot of its ring buffer.
/// The slot is stamped with the chunk size and the current frame number, and the
/// frame being loaded is flagged as carrying chunk data so that the data is handed
/// to the event handler when the frame is drawn.
/// </summary>
/// <param name="iffsize">Size of IFF chunk.</param>
/// <param name="index">Index of the multi-frame chunk table entry to read into.</param>
/// <returns>Returns with VQAERR_NONE if successful, or a VQAERR_??? error code.</returns>
long VQA_MFCIReadData(VQAHandleP *vqap, unsigned long iffsize, long index)
{
	void *buf;
	VQALoader *loader;
	VQAMFCInfo::DATA2 *data2;
	VQAConfig *config;
	unsigned long padsize;
	unsigned long write_index;


	loader = &vqap->Loader;
	config = &vqap->Config;

	padsize = PADSIZE(iffsize);

	data2 = &vqap->MFCInfo.Data2[index];

	write_index = data2->WriteIndex;

	buf = data2->Data[write_index].Buffer;
	data2->Data[write_index].Size = iffsize;
	data2->Data[write_index].Frame = loader->CurFrameNum;
	write_index++;
	if (write_index >= data2->Count) {
		write_index = 0;
	}
	data2->WriteIndex = write_index;

	if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, buf, padsize)) {
		return(VQAERR_READ);
	}

	loader->CurFrame->Flags |= VQAFRMF_CHUNKS;

	return(VQAERR_NONE);
}


/// <summary>
/// Fetches the multi-stream chunk table entry that handles the given chunk ID.
/// </summary>
/// <returns>Returns with the index of the matching table entry, or -1 if the chunk is not a multi-stream chunk.</returns>
long VQA_MSCIIndexFromChunkID(VQAHandleP *vqap, unsigned long chunkid)
{
	VQAMSCInfo *info = &vqap->MSCInfo;
	VQAMSCInfo::HEADER *infohdr = &info->Header;
	VQAMSCInfo::TABLE *table = info->Table;
	unsigned long count = infohdr->Count;
	for (unsigned long i = 0; i < count; i++) {
		if (chunkid == table[i].ChunkID) {
			return(i);
		}
	}
	return(-1);
}


/// <summary>
/// Reads a multi-stream chunk's payload into the next slot of its ring buffer.
/// This is the multi-stream counterpart of VQA_MFCIReadData. The slot is stamped with
/// the chunk size and the current frame number, and the frame being loaded is flagged
/// as carrying chunk data so that the data is handed to the event handler when the
/// frame is drawn.
/// </summary>
/// <param name="iffsize">Size of IFF chunk.</param>
/// <param name="index">Index of the multi-stream chunk table entry to read into.</param>
/// <returns>Returns with VQAERR_NONE if successful, or a VQAERR_??? error code.</returns>
long VQA_MSCIReadData(VQAHandleP *vqap, unsigned long iffsize, long index)
{
	VQAConfig *config;
	VQALoader *loader;
	unsigned long padsize;

	VQAMSCInfo::DATA2 *data2;
	unsigned long write_index;
	void *buf;

	VQAMSCInfo *info = &vqap->MSCInfo;

	loader = &vqap->Loader;
	config = &vqap->Config;

	padsize = PADSIZE(iffsize);

	data2 = &info->Data2[index];

	write_index = data2->WriteIndex;

	buf = data2->Data[write_index].Buffer;
	data2->Data[write_index].Size = iffsize;
	data2->Data[write_index].Frame = loader->CurFrameNum;
	write_index++;
	if (write_index >= data2->Count) {
		write_index = 0;
	}
	data2->WriteIndex = write_index;

	if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, buf, padsize)) {
		return(VQAERR_READ);
	}

	loader->CurFrame->Flags |= VQAFRMF_CHUNKS;

	return(VQAERR_NONE);
}


/// <summary>
/// Determines if a frame is the first frame of a loop.
/// The loader uses this to decide when its codebook and palette state must be
/// rebuilt, since playback can arrive at a loop start from anywhere in the movie.
/// </summary>
/// <returns>Returns with true if the frame begins a loop.</returns>
VQABool VQA_IsFrameStartOfLoop(VQAHandleP *vqap, long framenum)
{
	int i;
	int count;
	VQALoopInfo::DATA *data;

	if (framenum == 0) {
		return(true);
	}

	count = vqap->LoopInfo.Header.Count;

	if (count > 0 && ((vqap->LoopInfo.Header.Flags & VQALOOPF_DATAVALID) || (vqap->LoopInfo.Header.Flags & VQALOOPF_2))) {
		data = vqap->LoopInfo.Data;
		for (i = 0; i < count; i++) {
			if (framenum == data[i].StartFrame) {
				return(true);
			}
		}
	}

	return(false);
}


long VQA_SeekLoop(VQAHandleP *vqap, long framenum, long flags)
{
	long rc = VQAERR_NONE;
	VQAConfig *config;
	VQALoopCache *cache;
	bool needs_seek = false;
	long *foff;

	cache = &vqap->LoopCache;
	foff = vqap->Foff;

	if ((vqap->AltBufferFlags & VQAABUFF_ALTLOOP) && framenum == cache->Min && cache->Bytes != 0) {
		if ((long)(unsigned char *)cache->Buffer + cache->Bytes <= (long)(unsigned char *)VQAFRAME_OFFSET(foff[vqap->LoopEndFrameMode2])) {
			needs_seek = true;
		}
		cache->Offset = 0;
	} else if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_SEEK, 0, VQAFRAME_OFFSET(foff[framenum])) != 0) {
		return(VQAERR_SEEK);
	}

	if (framenum > 0 && (!(flags & VQALOADF_NOFCB) || !(vqap->Header.Flags & VQAHDF_4))) {
		rc = VQA_LoadFrame(vqap, (flags | VQALOADF_NOSND | VQALOADF_NOPTR));
	}

	if (rc == VQAERR_NONE) {
		if (needs_seek && vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_SEEKPEEK, 0, long((int)cache->Buffer + cache->Bytes)) != 0) {
			return(VQAERR_SEEK);
		}
	}

	return(rc);
}


/// <summary>
/// Fetches the range of frames over which a single palette remains in effect.
/// The movie records which of its frames carry a palette chunk. This routine finds
/// the entry covering the requested frame and reports the inclusive frame span that
/// the palette governs.
/// </summary>
/// <param name="framenum">The frame to locate.</param>
/// <param name="first_frame">Receives the first frame of the span. May be NULL.</param>
/// <param name="last_frame">Receives the last frame of the span. May be NULL.</param>
/// <returns>Returns with the index of the palette entry covering the frame.</returns>
long VQA_GetPaletteFrameRange(VQAHandleP *vqap, long framenum, long * first_frame, long * last_frame)
{
	VQAPaletteInfo::DATA *data = vqap->PaletteInfo.Data;
	int count = vqap->PaletteInfo.Header.Count;
	unsigned int idx1;
	unsigned int idx2;
	long cur_frame;
	long prev_frame;
	int findex = 0;
	bool found = false;

	for (int i = 1; i < count; i++) {
		cur_frame = data[i].Frame;
		prev_frame = data[i - 1].Frame;
		if (framenum >= prev_frame) {
			if (framenum < cur_frame) {
				found = true;
				break;
			}
		}
		findex++;
	}

	if (!found) {
		findex = count - 1;
	}

	if (first_frame != NULL) {
		*first_frame = data[findex].Frame;
	}

	if (last_frame != NULL) {
		if (findex < count - 1) {
			*last_frame = data[findex + 1].Frame - 1;
		} else {
			*last_frame = vqap->NumFrames;
		}
	}
	return(findex);
}


/// <summary>
/// Reloads the palette that applies to the given frame.
/// The palette in effect at the target frame is located and loaded on its own, so
/// that none of the pointer, codebook or sound data of the frame carrying it is
/// disturbed. A forced reload also resets the loader's codebook state first, so that
/// the palette is rebuilt from the start of the movie rather than carried forward
/// from the current position.
/// </summary>
/// <param name="framenum">The frame whose palette is required.</param>
/// <param name="force">Non-zero to force a full reload rather than reloading only when the palette has changed.</param>
long VQA_ReloadPalette(VQAHandleP *vqap, long framenum, int force)
{
	VQALoader *loader;
	VQAFrameNode *curframe;
	VQAConfig *config;

	long rc = -1;

	loader = &vqap->Loader;
	curframe = loader->CurFrame;
	config = &vqap->Config;


	long pal_frame;
	long last_pal_frame;
	int i;

	if ( framenum > vqap->StopFrame || vqap->Foff == NULL) {
		return VQAERR_SEEK;
	}

	if ( vqap->PaletteInfo.Data != 0 )
	{
		//pal_frame = -1;
		VQA_GetPaletteFrameRange(vqap, framenum, &pal_frame, NULL);
		if (!force) {
			//last_pal_frame = -1;
			VQA_GetPaletteFrameRange(vqap, loader->LastFrameNum, &last_pal_frame, 0);
			if (pal_frame == last_pal_frame) {
				pal_frame = -1;
			}
		}
	} else {
		pal_frame = -1;
		i = framenum;
		for (; i >= 0; i--) {
			if (vqap->Foff[i] & VQAFINF_PAL) {
				pal_frame = i;
				break;
			}
		}

		if (!force) {
			last_pal_frame = -1;
			i = loader->LastFrameNum;
			for (; i >= 0; i--) {
				if (vqap->Foff[i] & VQAFINF_PAL) {
					last_pal_frame = i;
					break;
				}
			}

			if (pal_frame == last_pal_frame) {
				pal_frame = -1;
			}
			}
	}



	if (pal_frame >= 0 ) {
		if (!config->StreamHandler((VQAHandle *)vqap, VQACMD_SEEK, (void *)SEEK_SET, VQAFRAME_OFFSET(vqap->Foff[pal_frame]))) {

			if (force) {
				loader->NumPartialCB = 0;
				loader->PartialCBSize = 0;
				loader->FullCB = vqap->CBData;
				loader->CurCB = vqap->CBData;
				loader->CurFrameNum = 0;
				curframe->Flags = 0;
			}
			rc = VQA_LoadFrame(vqap, VQALOADF_NOSND | VQALOADF_NOPTR | VQALOADF_NOPCB | VQALOADF_NOFCB);
		} else {
			rc = VQAERR_SEEK;
		}
	}
	return rc;
}


/// <summary>
/// Positions the loader at the frame group that contains the given frame.
/// The codebook group covering the target frame is located, then the frames leading
/// up to the target are loaded so that their partial codebooks accumulate into a
/// complete one. Audio may be pre-rolled ahead of the target so that the sound buffer
/// already holds data by the time playback resumes.
/// </summary>
/// <param name="framenum">The frame to seek to.</param>
/// <param name="groupsize">Number of frames per codebook group, or zero if the movie carries a codebook table instead.</param>
/// <param name="preloadaudio">Should audio be loaded ahead of the target frame?</param>
/// <param name="reset_state">Should the loader and audio state be reset before seeking?</param>
/// <param name="skipcodebook">Set true if the full codebook is already valid, so that the caller may skip codebook assembly.</param>
/// <returns>Returns with VQAERR_NONE if successful, or a VQAERR_??? error code.</returns>
long VQA_SeekGroup(VQAHandleP *vqap, long framenum, long groupsize, VQABool preloadaudio, VQABool reset_state, VQABool &skipcodebook)
{
	//VQAHandleP   *vqap;
	VQALoader    *loader;
	VQADrawer    *drawer;
	VQAHeader    *header;
	VQAFrameNode *frame;
	VQACBNode    *curcb;
	VQAConfig    *config;
	long         group;
	long         i;

	#if(VQAAUDIO_ON)
	VQAAudio     *audio;;
	#endif

	int audpreload;
	int bytes_per_frame;
	int preload_frames;
	int covered_bytes;
	unsigned int rc;
	bool bool2;
	int loadflags;
	long group_end;
	long group_start;
#define VQA_GROUP_START group_start
#define VQA_GROUP_END group_end
	int group_index;
	int last_group_index;
	VQABool cb_valid;
	long audio_start;

	/* Dereference commonly used data members for quick access. */
	//vqap = (VQAHandleP *)vqa;
	loader = &vqap->Loader;
	drawer = &vqap->Drawer;
	header = &vqap->Header;
	config = &vqap->Config;

	#if(VQAAUDIO_ON)
	audio = &vqap->Audio;
	#endif

	group_index = 0;


	frame = drawer->CurFrame;
	if (vqap->Foff) {

		group = framenum;
		if (config->NumCBBufs > 1) {
			cb_valid = 0;
			if (groupsize > 0) {
				/* Compute the starting group frame of the requested frame. */
				group_index = framenum / groupsize;
				VQA_GROUP_START = group_index * groupsize;


				/* The codebook for the group we want to goto is found in the previous
				 * group, with the exception of the very first group.
				 */
				if (VQA_GROUP_START >= groupsize) {
					VQA_GROUP_START -= groupsize;
				}
				if (group_index == 0) {
					VQA_GROUP_END = -1;
				} else {
					VQA_GROUP_END = VQA_GROUP_START + groupsize - 1;
				}
				last_group_index = loader->LastFrameNum / groupsize;
			} else {
				group_index = VQA_GetCodebookFrameRange(vqap, framenum, &VQA_GROUP_START, &VQA_GROUP_END);
				last_group_index = VQA_GetCodebookFrameRange(vqap, loader->LastFrameNum, 0, 0);
			}
			group = VQA_GROUP_START;


			if (!reset_state) {
				if (last_group_index == group_index) {
					group = VQA_GROUP_END + 1;
					loader->FullCB = loader->PrevCB;
					loader->CurCB = loader->FullCB->Next;
					cb_valid = 1;
				}
				else if (loader->NumPartialCB == 0) {
					bool dont_set_cur = false;
					curcb = loader->FullCB;
					while (true) {
						if (curcb == frame->Codebook && (frame->Flags & VQAFRMF_LOADED) != 0) {
							dont_set_cur = true;
							break;
						}
						frame = frame->Next;
						if (drawer->CurFrame == frame) {
							break;
						}
					}
					if (!dont_set_cur) {
						loader->CurCB = curcb;
					}
				}
			} else {
				loader->FullCB = vqap->CBData;
				loader->CurCB = vqap->CBData;
			}

			/* Fool the loader into thinking this frame is empty. */
			loader->NumPartialCB = 0;
			loader->PartialCBSize = 0;
		} else {
			cb_valid = 1;
			VQA_GROUP_START = framenum;
			VQA_GROUP_END = framenum;
		}

		skipcodebook = cb_valid;

		/* Throw away any audio frames that were loaded. */
		#if(VQAAUDIO_ON)
			if ((config->OptionFlags & VQAOPTF_AUDIO)
					&& (audio->Buffer != NULL)) {
			int bytes_per_sec = ((vqap->SampleRate * vqap->Channels) * (vqap->BitsPerSample >> 3));
			bytes_per_frame = bytes_per_sec / config->FrameRate;
			audpreload = header->AudioPreload;
			if ((header->Flags & VQAHDF_SNDJUMP) == 0 && audpreload == 0)
			{
				audpreload = (long)(bytes_per_sec / 2);
			}

			if ( preloadaudio && audpreload > 0 )
			{
				preload_frames = audpreload / bytes_per_frame;
				covered_bytes = bytes_per_frame * preload_frames;
				if ( audpreload > covered_bytes )
				{
					++preload_frames;
				}
				audio_start = framenum - preload_frames;
				if (audio_start < 0) {
					audio_start = 0;
					audio->BufferOffset = framenum * bytes_per_frame;


				}
				else
				{
					audio->BufferOffset = bytes_per_frame + covered_bytes - audpreload;
				}

				if ( audio_start < group )
				{
					group = audio_start;
				}
			}
			else
			{
				audio_start = framenum;
				audio->BufferOffset = 0;
			}

			if ( reset_state )
			{
				/* Throw away any audio frames that were loaded. */
				memset(audio->IsLoaded, 0, audio->NumAudBlocks * sizeof(*audio->IsLoaded));
				memset(audio->Buffer, 0, config->AudioBufSize);
				audio->TempBufLen = 0;

				/* Position the audio buffer to 1/2 second. */
				if (audio_start == 0) {
					audio->AudBufPos = 0;
				} else {
					audio->AudBufPos = audpreload;
					/* Mark 1/2 second of the audio buffer as loaded. */
					int blocks = int(audio->AudBufPos / config->HMIBufSize);
					for (i = 0; i < blocks; i++) {
						audio->IsLoaded[i] = 1;
					}
				}
			}

		}
		else
		{
			preloadaudio = 0;
			audio_start = framenum;
			audio->BufferOffset = 0;
		}
		#endif


		if (VQA_IsFrameStartOfLoop(vqap, framenum)) {
			if (reset_state) {
				loader->CurFrame->Flags = 0;
			}

			if (cb_valid) {
				loadflags = VQALOADF_NOFCB;
			} else {
				loadflags = 0;
				loader->FullCB = loader->CurCB;
			}
			rc = VQA_SeekLoop(vqap, framenum, loadflags);
			if ( reset_state )
			{
				if (rc == VQAERR_NOBUFFER || rc == VQAERR_SLEEPING) {
					rc = -1;
				}
			}

			return(rc);
		}

		/* Seek to the start of the group containing the partial codebooks for
		 * the target frame.
		 */
		if (!config->StreamHandler((VQAHandle *)vqap, VQACMD_SEEK, (void *)SEEK_SET, VQAFRAME_OFFSET(vqap->Foff[group]))) {
			loader->CurFrameNum = group;
			rc = -1;

			loadflags = (VQALOADF_NOPAL|VQALOADF_NOPTR);
			if (group_index) {
				loadflags |= VQALOADF_NOFCB;
			}
			if (!preloadaudio) {
				loadflags |= VQALOADF_NOSND;
			}


			/* Load frames up to the target frame collecting partial codebooks
			 * along the way.
			 */
			long loadframe = loader->CurFrameNum;
			for (i = 0; i < (framenum - group); i++) {

				if (reset_state) {
					loader->CurFrame->Flags &= ~VQAFRMF_LOADED;
				}

				if (preloadaudio) {
					if (reset_state) {
						if (loadframe == audio_start) {
							audio->TempBufLen = 0;
						}
					}

					if (loadframe < audio_start) {
						loadflags |= VQALOADF_NOSND;
					} else {
						loadflags &= ~VQALOADF_NOSND;
					}
				}

				if (cb_valid && loadframe <= VQA_GROUP_END) {
					loadflags |= VQALOADF_NOPCB;
				} else {
					loadflags &= ~VQALOADF_NOPCB;
				}

				/* Load the frame. */
				rc = VQA_LoadFrame(vqap, loadflags);
				if (rc != VQAERR_NONE) {
					if (!reset_state || ((rc != VQAERR_NOBUFFER) && (rc != VQAERR_SLEEPING))) {
						break;
					} else {
						rc = VQAERR_NONE;
					}
				}
				loadframe++;
			}
		} else {
			return(VQAERR_SEEK);
		}
		return(rc);
	}
	return(VQAERR_SEEK);
#undef VQA_GROUP_START
#undef VQA_GROUP_END
}


/// <summary>
/// Rewinds the loader to the start frame of a loop.
/// The palette and codebook state are rebuilt for the loop start frame before its
/// data is loaded, so that playback resumes cleanly however far into the movie the
/// loop was entered. The rewind is refused when the frame buffer is still busy, or
/// when the stream cannot be seeked.
/// </summary>
/// <param name="framenum">The loop start frame to rewind to.</param>
/// <returns>Returns with VQAERR_NONE if successful, or a VQAERR_??? error code.</returns>
_STATIC long VQA_LoadLoop(VQAHandleP *vqap, long framenum)
{
	VQALoader *loader;
	VQAHeader *header;
	VQAConfig *config;


	long rc = VQAERR_NONE;
	VQABool skipcodebook = false;

	loader = &vqap->Loader;
	header = &vqap->Header;
	config = &vqap->Config;

	if (loader->CurFrame->Flags & VQAFRMF_LOADED) {
		return(VQAERR_NOBUFFER);
	}

	if (framenum >= vqap->StopFrame || vqap->Foff == NULL) {
		return(VQAERR_SEEK);
	}

	if ((config->OptionFlags & VQAOPTF_AUDIO) && !(vqap->Header.Flags & VQAHDF_SNDJUMP)) {
		return(VQAERR_SEEK);
	}

	if (header->ColorMode == 0 && !VQA_IsFrameStartOfLoop(vqap, framenum)) {
		rc = VQA_ReloadPalette(vqap, framenum, 1);
	}

	if (rc == VQAERR_NONE) {
		rc = VQA_SeekGroup(vqap, framenum, header->Groupsize, false, false, skipcodebook);
	}

	loader->CurFrameNum = framenum;
	if (rc == VQAERR_NONE) {

		long flags = VQALOADF_NOLFR;
		if (skipcodebook) {
			flags |= VQALOADF_NOFCB;
		}

		rc = VQA_LoadFrame(vqap, flags);

	}
	return(rc);
}


/****************************************************************************
*
* NAME
*     Load_VQF - Loads a VQ Frame chunk.
*
* SYNOPSIS
*     Error = Load_VQF(VQA, Iffsize)
*
*     long Load_VQF(VQAHandleP *, unsigned long);
*
* FUNCTION
*     The VQ Frame Chunk contains a set of other chunks (codebooks,
*     palettes, pointers).  This routine reads the frame's chunk size,
*     then loops until it's read that many bytes.
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long Load_VQF(VQAHandleP *vqap, unsigned long frame_iffsize, char flags)
{
	VQAFrameNode  *curframe;
	ChunkHeader   *chunk;
	unsigned long iffsize;
	unsigned long framesize;
	unsigned long bytes_loaded = 0;
	VQADrawer     *drawer;
	VQAConfig     *config;
	int skip;

	/* Dereference commonly used data members for quicker access. */
	curframe = vqap->Loader.CurFrame;
	drawer = &(vqap->Drawer);
	chunk = &vqap->Loader.CurChunkHdr;
	config = &(vqap->Config);
	framesize = PADSIZE(frame_iffsize);

	/*-------------------------------------------------------------------------
	 * FRAME LOADING LOOP.
	 *-----------------------------------------------------------------------*/
	while (bytes_loaded < framesize) {

		/* Read chunk ID */
		if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, chunk, sizeof(*chunk))) {
			return(VQAERR_EOF);
		}

		iffsize = REVERSE_LONG(chunk->size);
		bytes_loaded += sizeof(*chunk);
		bytes_loaded += PADSIZE(iffsize);

		skip = true;

		/* Handle each chunk type */
		switch (chunk->id) {

			/* Full uncompressed codebook */
			case ID_CBF0:
				if (flags & VQALOADF_NOFCB) {
					break;
				}
				if (Load_CBF0(vqap, iffsize) != VQAERR_NONE) {
					return(VQAERR_READ);
				}

				skip = false;
				break;

			/* Full compressed codebook */
			case ID_CBFZ:
				if (flags & VQALOADF_NOFCB) {
					break;
				}
				if (Load_CBFZ(vqap, iffsize) != VQAERR_NONE) {
					return(VQAERR_READ);
				}

				skip = false;
				break;

			/* Partial uncompressed codebook */
			case ID_CBP0:
				if (flags & VQALOADF_NOPCB) {
					break;
				}
				if (Load_CBP0(vqap, iffsize) != VQAERR_NONE) {
					return(VQAERR_READ);
				}

				skip = false;
				break;

			/* Partial compressed codebook */
			case ID_CBPZ:
				if (flags & VQALOADF_NOPCB) {
					break;
				}
				if (Load_CBPZ(vqap, iffsize) != VQAERR_NONE) {
					return(VQAERR_READ);
				}

				skip = false;
				break;

			/* Uncompressed palette */
			case ID_CPL0:
				if (flags & VQALOADF_NOPAL) {
					break;
				}
				if (Load_CPL0(vqap, iffsize) != VQAERR_NONE) {
					return(VQAERR_READ);
				}

				#if 0
				/* If this is the first occurance of a palette then store it now.
				 * This functionality is needed for Monopoly!
				 */
				if (drawer->CurPalSize == 0) {
					memcpy(drawer->Palette_24,curframe->Palette,curframe->PaletteSize);
					drawer->CurPalSize = curframe->PaletteSize;
				}
				#endif

				/* Flag this frame as having a palette. */
				curframe->Flags |= VQAFRMF_PALETTE;

				skip = false;
				break;

			/* Compressed palette */
			case ID_CPLZ:
				if (flags & VQALOADF_NOPAL) {
					break;
				}
				if (Load_CPLZ(vqap, iffsize) != VQAERR_NONE) {
					return(VQAERR_READ);
				}

				/* Flag this frame as having a palette. */
				curframe->Flags |= VQAFRMF_PALETTE;

				skip = false;
				break;



			/* Uncompressed pointer data */
			case ID_VPT0:
			case ID_VPTD:
				if (flags & VQALOADF_NOPTR) {
					break;
				}
				if (Load_VPT0(vqap, iffsize) != VQAERR_NONE) {
					return(VQAERR_READ);
				}

				skip = false;
				break;

			/* Compressed pointer data */
			case ID_VPTZ:
			case ID_VPDZ:
			//case ID_VPTD:
				if (flags & VQALOADF_NOPTR) {
					break;
				}
				if (Load_VPTZ(vqap, iffsize) != VQAERR_NONE) {
					return(VQAERR_READ);
				}

				skip = false;
				break;

			/* Compressed pointer data */
			case ID_VPTK:
				if (flags & VQALOADF_NOPTR) {
					break;
				}
				if (Load_VPT0(vqap, iffsize) != VQAERR_NONE) {
					return(VQAERR_READ);
				}

				skip = false;
				/* Flag this frame as being key. */
				curframe->Flags |= VQAFRMF_KEY;

				break;

			// new chunks

			case ID_VPTR:
				if (flags & VQALOADF_NOPTR) {
					break;
				}
				if (Load_VPT0(vqap, iffsize) != VQAERR_NONE) {
					return(VQAERR_READ);
				}

				skip = false;
				curframe->Flags |= VQAFRMF_RSDCOMP;

				break;

			case ID_VPKZ:
				if (flags & VQALOADF_NOPTR) {
					break;
				}
				if (Load_VPTZ(vqap, iffsize) != VQAERR_NONE) {
					return(VQAERR_READ);
				}

				skip = false;
				curframe->Flags |= VQAFRMF_KEY;

				break;

			case ID_VPRZ:
				if (flags & VQALOADF_NOPTR) {
					break;
				}
				if (Load_VPTZ(vqap, iffsize) != VQAERR_NONE) {
					return(VQAERR_READ);
				}

				skip = false;
				curframe->Flags |= VQAFRMF_RSDCOMP;

				break;

			/* An unknown chunk in the video frame is an error. */
			default:
				return(VQAERR_READ);
		}

		/* Skip any unknown chunks. */
		if (skip == true) {
			if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_SEEK, (void *)1, PADSIZE(iffsize))) {
				return(VQAERR_SEEK);
			}
		}
	}

	return(VQAERR_NONE);
}


long Load_CLIP(VQAHandleP *vqap, unsigned long iffsize)
{
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, &vqap->Clipper, PADSIZE(iffsize))) {
		return(VQAERR_READ);
	}
	return(VQAERR_NONE);
}


long Load_LINF(VQAHandleP *vqap)
{
	unsigned long iffsize;
	ChunkHeader chunk;
	VQAConfig *config;

	config = &vqap->Config;

	if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, &chunk, sizeof(chunk))) {
		return(VQAERR_EOF);
	}

	if (chunk.id != ID_LINH) {
		return(VQAERR_READ);
	}

	iffsize = REVERSE_LONG(chunk.size);
	if (Load_LINH(vqap, iffsize) != VQAERR_NONE) {
		return(VQAERR_READ);
	}

	if (vqap->LoopInfo.Header.Count > 0) {

		long size = (unsigned short)vqap->LoopInfo.Header.Count * sizeof(VQALoopInfo::DATA);
		vqap->LoopInfo.Data = (VQALoopInfo::DATA *)config->MemoryHandler((VQAHandle *)vqap, VQAMEM_ALLOC, 0, size);
		if (vqap->LoopInfo.Data == NULL) {
			return(VQAERR_NOMEM);
		}

		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_LOCK, vqap->LoopInfo.Data, size);
		vqap->MemUsed += size;

		if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, &chunk, sizeof(chunk))) {
			return(VQAERR_EOF);
		}

		if (chunk.id != ID_LIND) {
			return(VQAERR_READ);
		}

		size = REVERSE_LONG(chunk.size);
		if (Load_LIND(vqap, size) != VQAERR_NONE) {
			return(VQAERR_READ);
		}
	}

	return(VQAERR_NONE);
}


long Load_LINH(VQAHandleP *vqap, unsigned long iffsize)
{
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, &vqap->LoopInfo.Header, PADSIZE(iffsize))) {
		return(VQAERR_READ);
	}
	return(VQAERR_NONE);
}


long Load_LIND(VQAHandleP *vqap, unsigned long iffsize)
{
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, vqap->LoopInfo.Data, PADSIZE(iffsize))) {
		return(VQAERR_READ);
	}
	return(VQAERR_NONE);
}


long Load_MFCI(VQAHandleP *vqap)
{
	ChunkHeader chunk;
	VQAConfig *config;
	long size;
	unsigned long count;
	unsigned long iffsize;

	config = &vqap->Config;

	if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, &chunk, sizeof(chunk))) {
		return(VQAERR_EOF);
	}

	if (chunk.id != ID_MFCH) {
		return(VQAERR_READ);
	}

	iffsize = REVERSE_LONG(chunk.size);
	if (Load_MFCH(vqap, iffsize) != VQAERR_NONE) {
		return(VQAERR_READ);
	}

	count = vqap->MFCInfo.Header.StaticCount;
	if (count > 0) {

		size = count * sizeof(VQAMFCInfo::DATA);
		vqap->MFCInfo.StaticData = (VQAMFCInfo::DATA *)config->MemoryHandler((VQAHandle *)vqap, VQAMEM_ALLOC, 0, size);
		if (vqap->MFCInfo.StaticData == NULL) {
			return(VQAERR_NOMEM);
		}

		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_LOCK, vqap->MFCInfo.StaticData, size);
		vqap->MemUsed += size;

		if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, &chunk, sizeof(chunk))) {
			return(VQAERR_EOF);
		}

		if (chunk.id != ID_MFCD) {
			return(VQAERR_READ);
		}

		size = REVERSE_LONG(chunk.size);
		if (Load_MFCD(vqap, size) != VQAERR_NONE) {
			return(VQAERR_READ);
		}
	}

	count = vqap->MFCInfo.Header.Count;
	if (count > 0) {

		size = count * sizeof(VQAMFCInfo::TABLE);
		vqap->MFCInfo.Table = (VQAMFCInfo::TABLE *)config->MemoryHandler((VQAHandle *)vqap, VQAMEM_ALLOC, 0, size);
		if (vqap->MFCInfo.Table == NULL) {
			return(VQAERR_NOMEM);
		}

		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_LOCK, vqap->MFCInfo.Table, size);
		vqap->MemUsed += size;

		if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, &chunk, sizeof(chunk))) {
			return(VQAERR_EOF);
		}

		if (chunk.id != ID_MFCT) {
			return(VQAERR_READ);
		}

		size = REVERSE_LONG(chunk.size);
		if (Load_MFCT(vqap, size) != VQAERR_NONE) {
			return(VQAERR_READ);
		}
	}

	count = vqap->MFCInfo.Header.Count;
	if (count > 0) {
		size = count * sizeof(VQAMFCInfo::DATA2);
		vqap->MFCInfo.Data2 = (VQAMFCInfo::DATA2 *)config->MemoryHandler((VQAHandle *)vqap, VQAMEM_ALLOC, 0, size);
		if (vqap->MFCInfo.Data2 == NULL) {
			return(VQAERR_NOMEM);
		}

		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_LOCK, vqap->MFCInfo.Data2, size);
		vqap->MemUsed += size;

		for (unsigned long i = 0; i < count; i++) {
			long rc = VQA_MFCIPrepare(vqap, i);
			if (rc != VQAERR_NONE) {
				return(rc);
			}
		}
	}

	return(VQAERR_NONE);
}


long Load_MFCH(VQAHandleP *vqap, unsigned long iffsize)
{
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, &vqap->MFCInfo.Header, PADSIZE(iffsize))) {
		return(VQAERR_READ);
	}
	return(VQAERR_NONE);
}


long Load_MFCD(VQAHandleP *vqap, unsigned long iffsize)
{
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, vqap->MFCInfo.StaticData, PADSIZE(iffsize))) {
		return(VQAERR_READ);
	}
	return(VQAERR_NONE);
}


long Load_MFCT(VQAHandleP *vqap, unsigned long iffsize)
{
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, vqap->MFCInfo.Table, PADSIZE(iffsize))) {
		return(VQAERR_READ);
	}
	return(VQAERR_NONE);
}


long Load_MSCI(VQAHandleP *vqap)
{
	ChunkHeader chunk;
	VQAConfig *config;
	long size;
	unsigned long count;
	unsigned long iffsize;

	config = &vqap->Config;

	if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, &chunk, sizeof(chunk))) {
		return(VQAERR_EOF);
	}

	if (chunk.id != ID_MSCH) {
		return(VQAERR_READ);
	}

	iffsize = REVERSE_LONG(chunk.size);
	if (Load_MSCH(vqap, iffsize) != VQAERR_NONE) {
		return(VQAERR_READ);
	}

	VQAMSCInfo::HEADER *infohdr = &vqap->MSCInfo.Header;

	count = infohdr->Count;
	if (count > 0) {

		size = count * sizeof(VQAMSCInfo::TABLE);
		vqap->MSCInfo.Table = (VQAMSCInfo::TABLE *)config->MemoryHandler((VQAHandle *)vqap, VQAMEM_ALLOC, 0, size);
		if (vqap->MSCInfo.Table == NULL) {
			return(VQAERR_NOMEM);
		}

		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_LOCK, vqap->MSCInfo.Table, size);
		vqap->MemUsed += size;

		if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, &chunk, sizeof(chunk))) {
			return(VQAERR_EOF);
		}

		if (chunk.id != ID_MSCT) {
			return(VQAERR_READ);
		}

		size = REVERSE_LONG(chunk.size);
		if (Load_MSCT(vqap, size) != VQAERR_NONE) {
			return(VQAERR_READ);
		}
	}

	count = infohdr->Count;
	if (count > 0) {
		size = count * sizeof(VQAMSCInfo::DATA2);
		vqap->MSCInfo.Data2 = (VQAMSCInfo::DATA2 *)config->MemoryHandler((VQAHandle *)vqap, VQAMEM_ALLOC, 0, size);
		if (vqap->MSCInfo.Data2 == NULL) {
			return(VQAERR_NOMEM);
		}

		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_LOCK, vqap->MSCInfo.Data2, size);
		vqap->MemUsed += size;

		for (unsigned long i = 0; i < count; i++) {
			long rc = VQA_MSCIPrepare(vqap, i);
			if (rc != VQAERR_NONE) {
				return(rc);
			}
		}
	}

	return(VQAERR_NONE);
}


long Load_MSCH(VQAHandleP *vqap, unsigned long iffsize)
{
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, &vqap->MSCInfo.Header, PADSIZE(iffsize))) {
		return(VQAERR_READ);
	}
	return(VQAERR_NONE);
}


long Load_MSCT(VQAHandleP *vqap, unsigned long iffsize)
{
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, vqap->MSCInfo.Table, PADSIZE(iffsize))) {
		return(VQAERR_READ);
	}
	return(VQAERR_NONE);
}


/// <summary>
/// Determines if a multi-frame chunk's key frames are spread across a loop more
/// widely than a single cycle of its ring buffer.
/// This is a capacity probe. When it reports true the chunk needs one extra buffer
/// slot, otherwise a key frame could be overwritten before the loop replays it.
/// </summary>
/// <param name="chunkid">The chunk type to examine.</param>
/// <param name="value">The number of frames covered by one cycle of the ring buffer.</param>
/// <returns>Returns with true if an extra buffer slot is required.</returns>
VQABool VQA_MFCISpansLoop(VQAHandleP *vqap, unsigned long chunkid, unsigned long value)
{
	unsigned int lcount;
	unsigned int scount;
	VQALoopInfo::DATA *ldata;
	VQAMFCInfo::DATA *sdata;

	lcount = vqap->LoopInfo.Header.Count;
	ldata = vqap->LoopInfo.Data;
	scount = vqap->MFCInfo.Header.StaticCount;
	sdata = vqap->MFCInfo.StaticData;

	int delta_to_end = 0;
	int delta_to_start = 0;

	for (unsigned int lindex = 0; lindex < lcount; lindex++) {

		delta_to_start = -1;

		for (unsigned int sindex = 0; sindex < scount; sindex++) {

			if (sdata[sindex].ChunkID == chunkid) {
				unsigned int frame = sdata[sindex].KeyFrame;

				unsigned int start = ldata[lindex].StartFrame;

				if (frame >= start) {
					unsigned int end = ldata[lindex].EndFrame;
					if (frame > end) {
						break;
					}

					if (delta_to_start < 0) {
						delta_to_start = frame - start;
						delta_to_end = end - frame;
					} else {
						delta_to_end = end - frame;
					}
				}
			}
		}
		if (delta_to_start > 0 && delta_to_start + delta_to_end + 1 < value) {
			return 1;
		}
	}
	return 0;
}


/// <summary>
/// Fetches the number of ring buffer slots a multi-frame chunk requires.
/// Enough slots are needed to cover every buffered frame, plus one more when the
/// chunk's key frames span a loop more widely than a single cycle of the buffer.
/// </summary>
/// <param name="chunkid">The chunk type to size the buffer for.</param>
/// <param name="count">The number of frames the buffer must cover.</param>
/// <param name="value">The number of frames covered by one cycle of the ring buffer.</param>
/// <returns>Returns with the number of slots required, or zero if the chunk is disabled.</returns>
long VQA_MFCICalcCount(VQAHandleP *vqap, unsigned long chunkid, long count, unsigned long value)
{
	if (value == 0) {
		return(0);
	}
	long ret = ((count - 1) / value) + 1;
	if (VQA_MFCISpansLoop(vqap, chunkid, value)) {
		ret++;
	}
	return(ret);
}


/// <summary>
/// Allocates the ring buffer that holds the loaded data of one multi-frame chunk.
/// A ring of slot descriptors and one contiguous pool of slot storage are allocated
/// through the memory handler, and the pool is then parcelled out a buffer per slot.
/// Every slot starts out marked as empty.
/// </summary>
/// <param name="index">Index of the multi-frame chunk table entry to prepare.</param>
/// <returns>Returns with VQAERR_NONE if successful, or VQAERR_NOMEM if a buffer could not be allocated.</returns>
long VQA_MFCIPrepare(VQAHandleP *vqap, unsigned long index)
{
	VQAConfig *config;
	unsigned long size;

	config = &vqap->Config;

	int count = config->NumFrameBufs;

	VQAMFCInfo *info = &vqap->MFCInfo;
	VQAMFCInfo::DATA2 *data2s = info->Data2;
	VQAMFCInfo::TABLE *table = info->Table;

	int tsize = table[index].EntrySize;

	count = VQA_MFCICalcCount(vqap, table[index].ChunkID, count, table[index].FrameInterval);

	if (count > 0) {

		data2s[index].Count = count;
		data2s[index].WriteIndex = 0;

		size = sizeof(data2s[index]) * count;
		VQAMFCInfo::DATA2::DATA *data = (VQAMFCInfo::DATA2::DATA *)config->MemoryHandler((VQAHandle *)vqap, VQAMEM_ALLOC, NULL, size);
		if (data == NULL) {
			return(VQAERR_NOMEM);
		}

		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_LOCK, data, size);
		vqap->MemUsed += size;

		data2s[index].Data = data;

		size = count * PADSIZE(tsize);
		unsigned char *entries = (unsigned char *)config->MemoryHandler((VQAHandle *)vqap, VQAMEM_ALLOC, NULL, size);
		if (entries == NULL)  {
			return(VQAERR_NOMEM);
		}

		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_LOCK, entries, size);
		vqap->MemUsed += size;

		for (int i = 0; i < count; i++) {
			data2s[index].Data[i].Buffer = (char *)entries;
			data2s[index].Data[i].Frame = -1;
			entries += tsize;
		}
	}
	return(VQAERR_NONE);
}


/// <summary>
/// Allocates the ring buffer that holds the loaded data of one multi-stream chunk.
/// This is the multi-stream counterpart of VQA_MFCIPrepare. One slot is allocated per
/// frame buffer; stream data arrives with every frame, so no allowance is needed for
/// the spread of key frames across a loop.
/// </summary>
/// <param name="index">Index of the multi-stream chunk table entry to prepare.</param>
/// <returns>Returns with VQAERR_NONE if successful, or VQAERR_NOMEM if a buffer could not be allocated.</returns>
long VQA_MSCIPrepare(VQAHandleP *vqap, unsigned long index)
{
	VQAConfig *config;
	unsigned long size;

	config = &vqap->Config;

	int count = config->NumFrameBufs;

	VQAMSCInfo *info = &vqap->MSCInfo;
	VQAMSCInfo::DATA2 *data2s = info->Data2;
	VQAMSCInfo::TABLE *table = info->Table;

	int tsize = table[index].EntrySize;

	if (count > 0) {

		data2s[index].Count = count;
		data2s[index].WriteIndex = 0;

		size = sizeof(data2s[index]) * count;
		VQAMSCInfo::DATA2::DATA *data = (VQAMSCInfo::DATA2::DATA *)config->MemoryHandler((VQAHandle *)vqap, VQAMEM_ALLOC, NULL, size);
		if (data == NULL) {
			return VQAERR_NOMEM;
		}

		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_LOCK, data, size);
		vqap->MemUsed += size;

		data2s[index].Data = data;

		size = PADSIZE(tsize) * count;
		char *entries = (char *)config->MemoryHandler((VQAHandle *)vqap, VQAMEM_ALLOC, NULL, size);
		if (entries == NULL)  {
			return VQAERR_NOMEM;
		}

		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_LOCK, entries, size);
		vqap->MemUsed += size;

		for (int i = 0; i < count; i++) {
			data2s[index].Data[i].Buffer = entries;
			data2s[index].Data[i].Frame = -1;
			entries += tsize;
		}
	}
	return VQAERR_NONE;
}

/// <summary>
/// Fetches the size of the next codebook to be assembled after the given frame.
/// Movies that do not use a fixed codebook group size carry a table of codebook sizes
/// instead. The loader needs the size up front, so that it can tell when the partial
/// codebooks it is accumulating add up to a complete one.
/// </summary>
/// <param name="framenum">The frame to search forward from.</param>
/// <returns>Returns with the size in bytes of the next codebook, or zero if there is none.</returns>
long VQA_GetCodebookSize(VQAHandleP *vqap, long framenum)
{
	VQACodebookInfo *info = &vqap->CodebookInfo;
	VQACodebookInfo::DATA *data = info->Data;
	int count = info->Header.Count;
	bool found = false;
	int findex;

	for (int i = 0; i < count; i++) {
		if (data[i].Frame > framenum) {
			findex = i;
			found = true;
			break;
		}
	}

	if (found == false) {
		return(0);
	}
	return(data[findex].Size);
}


/// <summary>
/// Fetches the range of frames over which a single codebook remains in effect.
/// This is the codebook counterpart of VQA_GetPaletteFrameRange, and applies to
/// movies that carry a codebook table rather than a fixed codebook group size.
/// </summary>
/// <param name="framenum">The frame to locate.</param>
/// <param name="first_frame">Receives the first frame of the span. May be NULL.</param>
/// <param name="last_frame">Receives the last frame of the span. May be NULL.</param>
/// <returns>Returns with the index of the codebook entry covering the frame.</returns>
long VQA_GetCodebookFrameRange(VQAHandleP *vqap, long framenum, long *first_frame, long *last_frame)
{
	VQACodebookInfo::DATA *data = vqap->CodebookInfo.Data;
	int count = vqap->CodebookInfo.Header.Count;
	unsigned int start_index;
	unsigned int end_index;
	long cur_frame;
	long prev_frame;
	int findex = 0;
	bool found = false;

	for (int i = 1; i < count; i++) {
		cur_frame = data[i].Frame;
		prev_frame = data[i - 1].Frame;
		if (framenum >= prev_frame) {
			if (framenum < cur_frame) {
				found = true;
				break;
			}
		}
		findex++;
	}

	if (!found) {
		findex = count - 1;
	}

	if (findex == 0) {
		start_index = 0;
		end_index = 0;
	} else {
		start_index = findex - 1;
		end_index = findex;
	}

	if (first_frame != NULL) {
		*first_frame = data[start_index].Frame;
	}

	if (last_frame != NULL) {
		*last_frame = data[end_index].Frame - 1;
	}
	return(findex);
}


long Load_CINF(VQAHandleP *vqap)
{
	int groupsize;
	long size;
	unsigned long iffsize;
	ChunkHeader chunk;
	VQAConfig *config;

	config = &vqap->Config;

	if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, &chunk, sizeof(chunk))) {
		return(VQAERR_EOF);
	}

	if (chunk.id != ID_CINH) {
		return(VQAERR_READ);
	}

	iffsize = REVERSE_LONG(chunk.size);
	if (Load_CINH(vqap, iffsize) != VQAERR_NONE) {
		return(VQAERR_READ);
	}

	groupsize = 0;
	groupsize = vqap->Header.Groupsize;
	if (groupsize == 0 && vqap->CodebookInfo.Header.Count > 0) {

		size = vqap->CodebookInfo.Header.Count * sizeof(VQACodebookInfo::DATA);
		vqap->CodebookInfo.Data = (VQACodebookInfo::DATA *)config->MemoryHandler((VQAHandle *)vqap, VQAMEM_ALLOC, 0, size);
		if (vqap->CodebookInfo.Data == NULL) {
			return(VQAERR_NOMEM);
		}

		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_LOCK, vqap->CodebookInfo.Data, size);
		vqap->MemUsed += size;

		if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, &chunk, sizeof(chunk))) {
			return(VQAERR_EOF);
		}

		if (chunk.id != ID_CIND) {
			return(VQAERR_READ);
		}

		iffsize = REVERSE_LONG(chunk.size);
		if (Load_CIND(vqap, iffsize) != VQAERR_NONE) {
			return(VQAERR_READ);
		}
	}

	return(VQAERR_NONE);
}


long Load_CINH(VQAHandleP *vqap, unsigned long iffsize)
{
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, &vqap->CodebookInfo.Header, PADSIZE(iffsize))) {
		return(VQAERR_READ);
	}
	return(VQAERR_NONE);
}


long Load_CIND(VQAHandleP *vqap, unsigned long iffsize)
{
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, vqap->CodebookInfo.Data, PADSIZE(iffsize))) {
		return(VQAERR_READ);
	}
	return(VQAERR_NONE);
}


long Load_PINF(VQAHandleP *vqap)
{
	unsigned long iffsize;
	ChunkHeader chunk;
	VQAConfig *config;

	config = &vqap->Config;

	if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, &chunk, sizeof(chunk))) {
		return(VQAERR_EOF);
	}

	if (chunk.id != ID_PINH) {
		return(VQAERR_READ);
	}

	iffsize = REVERSE_LONG(chunk.size);
	if (Load_PINH(vqap, iffsize) != VQAERR_NONE) {
		return(VQAERR_READ);
	}

	if (vqap->PaletteInfo.Header.Count > 0) {

		long size = vqap->PaletteInfo.Header.Count * sizeof(VQAPaletteInfo::DATA);
		vqap->PaletteInfo.Data = (VQAPaletteInfo::DATA *)config->MemoryHandler((VQAHandle *)vqap, VQAMEM_ALLOC, 0, size);
		if (vqap->PaletteInfo.Data == NULL) {
			return(VQAERR_NOMEM);
		}

		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_LOCK, vqap->PaletteInfo.Data, size);
		vqap->MemUsed += size;

		if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, &chunk, sizeof(chunk))) {
			return(VQAERR_EOF);
		}

		if (chunk.id != ID_PIND) {
			return(VQAERR_READ);
		}

		iffsize = REVERSE_LONG(chunk.size);
		if (Load_PIND(vqap, iffsize) != VQAERR_NONE) {
			return(VQAERR_READ);
		}
	}

	return(VQAERR_NONE);
}


long Load_PINH(VQAHandleP *vqap, unsigned long iffsize)
{
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, &vqap->PaletteInfo.Header, PADSIZE(iffsize))) {
		return(VQAERR_READ);
	}
	return(VQAERR_NONE);
}


long Load_PIND(VQAHandleP *vqap, unsigned long iffsize)
{
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, vqap->PaletteInfo.Data, PADSIZE(iffsize))) {
		return(VQAERR_READ);
	}
	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     Load_FINF - Load Frame Info chunk.
*
* SYNOPSIS
*     Error = Load_FINF(VQA, Iffsize)
*
*     long Load_FINF(VQAHandleP *, unsigned long);
*
* FUNCTION
*     Load FINF chunk if buffer available, otherwise skip it.
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long Load_FINF(VQAHandleP *vqap, unsigned long iffsize)
{
	/* Dereference commonly used data members for quicker access. */
	/* Load the frame information table if we need to, otherwise we will
	 * skip it.
	 */
	if (vqap->Foff != NULL) {
		if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, vqap->Foff,
				PADSIZE(iffsize))) {

			return(VQAERR_READ);
		}
	} else {
		if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_SEEK, (void *)SEEK_CUR,
				PADSIZE(iffsize))) {
			return(VQAERR_SEEK);
		}
	}

	return(VQAERR_NONE);
}


#if 0
/****************************************************************************
*
* NAME
*     Load_VQHD - Load VQA header chunk.
*
* SYNOPSIS
*     Error = Load_VQHD(VQA, Iffsize)
*
*     long Load_VQHD(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

static long Load_VQHD(VQAHandleP *vqap, unsigned long iffsize)
{
	/* Read the header */
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, &vqap->Header,
			PADSIZE(iffsize))) {

		return (VQAERR_READ);
	}

	/* Reconfigure the Drawer for the new settings */
	VQA_Configure_Drawer(vqap);

	return (0);
}
#endif


/****************************************************************************
*
* NAME
*     Load_CBF0 - Load full uncompressed codebook.
*
* SYNOPSIS
*     Error = Load_CBF0(VQA, Iffsize)
*
*     long Load_CBF0(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long Load_CBF0(VQAHandleP *vqap, unsigned long iffsize)
{
	VQALoader *loader;
	VQACBNode *curcb;

	/* Dereference commonly used data members for quicker access. */
	loader = &vqap->Loader;
	curcb = loader->CurCB;

	/* Read into the start of the buffer */
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, curcb->Buffer,
			PADSIZE(iffsize))) {

		return(VQAERR_READ);
	}

	/* Reset the partial codebook counter. */
	loader->NumPartialCB = 0;

	/* Flag this codebook as uncompressed. */
	//curcb->Flags &= (~VQACBF_CBCOMP);
	curcb->Flags = 0;
	curcb->CBOffset = 0;
	curcb->CodebookSize = iffsize;
	curcb->Flags |= VQACBF_CBFULL;

	/* Clock pointers to next CB Buffer. */
	loader->FullCB = curcb;
	///loader->FullCB->Flags &= (~VQACBF_DOWNLOADED);
	loader->CurCB = curcb->Next;

	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     Load_CBFZ - Load full compressed codebook.
*
* SYNOPSIS
*     Error = Load_CBFZ(VQA, Iffsize)
*
*     long Load_CBFZ(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long Load_CBFZ(VQAHandleP *vqap, unsigned long iffsize)
{
	VQALoader     *loader;
	VQACBNode     *curcb;
	void          *buffer;
	unsigned long padsize;
	unsigned long lcwoffset;

	/* Dereference commonly used data members for quicker access. */
	loader = &vqap->Loader;
	curcb = loader->CurCB;
	padsize = PADSIZE(iffsize);

	/* Load the codebook into the end of the buffer. */
	lcwoffset = vqap->Max_CB_Size - padsize;
	buffer = curcb->Buffer + lcwoffset;

	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, buffer, padsize)) {
		return(VQAERR_READ);
	}

	/* Reset the partial codebook counter. */
	loader->NumPartialCB = 0;

	curcb->Flags = 0;

	/* Flag this codebook as compressed */
	curcb->Flags |= VQACBF_CBFULL|VQACBF_CBCOMP;
	curcb->CBOffset = lcwoffset;

	/* Clock pointers to next CB Buffer */
	loader->FullCB = curcb;
	loader->CurCB = curcb->Next;

	if (vqap->Header.Version < VQAHD_VER3) {
		((unsigned char *)buffer)[iffsize + 1] = 0x80;
	}

	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     Load_CBP0 - Load partial uncompressed codebook.
*
* SYNOPSIS
*     Error = Load_CBP0(VQA, Iffsize)
*
*     long Load_CBP0(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQA_??? error code.
*
****************************************************************************/

long Load_CBP0(VQAHandleP *vqap, unsigned long iffsize)
{
	VQALoader *loader;
	VQACBNode *curcb;
	VQAHeader *header;
	void      *buffer;
	int cbfull;

	/* Dereference commonly used data members for quicker access. */
	loader = &vqap->Loader;
	curcb = loader->CurCB;
	header = &vqap->Header;

	/*-------------------------------------------------------------------------
	 * ASSEMBLY PARTIAL CODEBOOKS.
	 *-----------------------------------------------------------------------*/

	if (header->Groupsize == 0 && loader->PartialCBSize == 0) {
		loader->CBSize = VQA_GetCodebookSize(vqap, loader->CurFrameNum);
	}

	/* Read the partial codebook into the next position in the buffer. */
	buffer = curcb->Buffer + loader->PartialCBSize;
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, buffer,
			PADSIZE(iffsize))) {

		return(VQAERR_READ);
	}

	/* Accumulate the partial codebook values. */
	loader->PartialCBSize += iffsize;
	loader->NumPartialCB++;
	cbfull = false;

	/*-------------------------------------------------------------------------
	 * PROCESS FULL CODEBOOK.
	 *-----------------------------------------------------------------------*/
	if (header->Groupsize != 0) {
		if (loader->NumPartialCB == vqap->Header.Groupsize) {
			cbfull = true;
		}
	} else {
		if (loader->PartialCBSize == loader->CBSize) {
			cbfull = true;
		} else {
			if (loader->PartialCBSize > loader->CBSize) {
				return(VQAERR_READ);
			}
		}
	}

	if (cbfull) {
		curcb->CodebookSize = loader->PartialCBSize;

		/* Reset the codebook accumulator values */
		loader->NumPartialCB = 0;
		loader->PartialCBSize = 0;

		curcb->Flags = 0;

		curcb->CBOffset = 0;

		curcb->Flags |= VQACBF_CBFULL;

		/* Go to the next codebook buffer */
		loader->FullCB = curcb;
		loader->CurCB = curcb->Next;
	}

	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     Load_CBPZ - Load partial compressed codebook.
*
* SYNOPSIS
*     Error = Load_CBPZ(VQA, Iffsize)
*
*     long Load_CBPZ(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long Load_CBPZ(VQAHandleP *vqap, unsigned long iffsize)
{
	VQAConfig     *config;
	VQALoader     *loader;
	VQACBNode     *curcb;
	void          *buffer;
	unsigned long padsize;
	VQAHeader     *header;
	long          groupsize;
	int cbfull;

	/* Dereference commonly used data members for quicker access */
	padsize = PADSIZE(iffsize);
	config = &vqap->Config;
	loader = &vqap->Loader;
	header = &vqap->Header;
	groupsize = header->Groupsize;
	curcb = loader->CurCB;

	/* Attempt to compute the LCW offset into the codebook buffer by
	 * multiplying the size of this chunk by the # frames/group, and adding
	 * a small fudge factor on, then subtracting that from the CB buffer size.
	 */
	if (loader->PartialCBSize == 0) {
		if (groupsize != 0) {
			loader->CBSize = PADSIZE(groupsize) + groupsize * padsize;
		} else {
			loader->CBSize = VQA_GetCodebookSize(vqap, loader->CurFrameNum);
		}
		if (!(vqap->AltBufferFlags & VQAABUFF_ALTCB)) {
			curcb->CBOffset = vqap->Max_CB_Size - loader->CBSize;
		} else {
			curcb->CBOffset = 0;
		}
	}

	/*-------------------------------------------------------------------------
	 * ASSEMBLE PARTIAL CODEBOOKS.
	 *-----------------------------------------------------------------------*/

	/* Read the partial codebook into the next position in the buffer. */
	buffer = ((curcb->Buffer + curcb->CBOffset) + loader->PartialCBSize);

	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, buffer, padsize)) {
		return(VQAERR_READ);
	}

	/* Accumulate partial codebook values */
	loader->PartialCBSize += iffsize;
	loader->NumPartialCB++;
	cbfull = false;

	/*-------------------------------------------------------------------------
	 * PROCESS FULL CODEBOOK.
	 *-----------------------------------------------------------------------*/
	if (groupsize != 0) {
		if (loader->NumPartialCB == groupsize) {
			cbfull = true;
		}
	} else {
		if (loader->PartialCBSize == loader->CBSize) {
			cbfull = true;
		} else {
			if (loader->PartialCBSize > loader->CBSize) {
				return(VQAERR_READ);
			}
		}
	}

	if (cbfull) {

		/* Reset the codebook accumulator values. */
		loader->NumPartialCB = 0;
		loader->PartialCBSize = 0;

		curcb->Flags = 0;

		/* Flag this codebook as compressed. */
		curcb->Flags |= VQACBF_CBFULL|VQACBF_CBCOMP;

		/* Go to the next codebook buffer */
		loader->FullCB = curcb;
		loader->CurCB = curcb->Next;

		if (header->Version < VQAHD_VER3) {
			curcb->Buffer[loader->CBSize + curcb->CBOffset + 1] = 0x80;
		}
	}

	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     Load_CPL0 - Load an uncompressed palette.
*
* SYNOPSIS
*     Error = Load_CPL0(VQA, Iffsize)
*
*     long Load_CPL0(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long Load_CPL0(VQAHandleP *vqap, unsigned long iffsize)
{
	VQADrawer *drawer;
	VQAFrameNode *curframe;

	/* Dereference commonly used data members for quicker access. */
	drawer = &vqap->Drawer;
	curframe = vqap->Loader.CurFrame;

	/* Read the palette into the palette buffer */
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, curframe->Palette,
			PADSIZE(iffsize))) {

		return(VQAERR_READ);
	}

	/* Flag the palette as uncompressed. */
	curframe->Flags &= ~VQAFRMF_PALCOMP;
	curframe->PalOffset = 0;
	curframe->PaletteSize = iffsize;

	if (vqap->PaletteInfo.Header.Count == 1) {
		memcpy(drawer->Palette_24, curframe->Palette, curframe->PaletteSize);
		drawer->CurPalSize = curframe->PaletteSize;
		drawer->Flags |= VQADRWF_SETPAL;
	}

	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     Load_CPLZ - Load compressed palette.
*
* SYNOPSIS
*     Error = Load_CPLZ(VQA, Iffsize)
*
*     long Load_CPLZ(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long Load_CPLZ(VQAHandleP *vqap, unsigned long iffsize)
{
	VQADrawer *drawer;
	VQAFrameNode  *curframe;
	void          *buffer;
	unsigned long padsize;
	unsigned long lcwoffset;

	/* Dereference commonly used data members for quicker access. */
	curframe = vqap->Loader.CurFrame;
	padsize = PADSIZE(iffsize);
	drawer = &vqap->Drawer;

 	/* Read the palette into the end of the palette buffer. */
	lcwoffset = vqap->Max_Pal_Size - padsize;
	buffer = curframe->Palette + lcwoffset;

	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, buffer, padsize)) {
		return(VQAERR_READ);
	}

	/* Flag this palette as compressed. */
	curframe->Flags |= VQAFRMF_PALCOMP;
	curframe->PalOffset = lcwoffset;
	curframe->PaletteSize = iffsize;

	if (vqap->Header.Version < VQAHD_VER3) {
		((unsigned char *)buffer)[iffsize + 1] = 0x80;
	}

	if (vqap->PaletteInfo.Header.Count == 1) {
		curframe->PaletteSize = LCW_Uncomp((char *)(curframe->Palette + curframe->PalOffset), (char *)curframe->Palette, vqap->Max_Pal_Size);
		curframe->Flags &= ~VQAFRMF_PALCOMP;
		memcpy(drawer->Palette_24, curframe->Palette, curframe->PaletteSize);
		drawer->CurPalSize = curframe->PaletteSize;
		drawer->Flags |= VQADRWF_SETPAL;
	}

	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     Load_VPT0 - Load uncompressed pointers.
*
* SYNOPSIS
*     Error = Load_VPT0(VQA, Iffsize)
*
*     long Load_VPT0(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long Load_VPT0(VQAHandleP *vqap, unsigned long iffsize)
{
	VQAFrameNode *curframe;

	/* Dereference commonly used data members for quicker access. */
	curframe = vqap->Loader.CurFrame;

	/* Read the pointers into start of the pointer buffer. */
	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, curframe->Pointers,
			PADSIZE(iffsize))) {

		return(VQAERR_READ);
	}

	/* Flag this frame as uncompressed */
	curframe->Flags &= ~VQAFRMF_PTRCOMP;
	curframe->PtrOffset = 0;

	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     Load_VPTZ - Load compressed pointers.
*
* SYNOPSIS
*     Error = Load_VPTZ(VQA, Iffsize)
*
*     long Load_VPTZ(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long Load_VPTZ(VQAHandleP *vqap, unsigned long iffsize)
{
	VQAFrameNode  *curframe;
	void          *buffer;
	unsigned long padsize;
	unsigned long lcwoffset;

	/* Dereference commonly used data members for quicker access. */
	curframe = vqap->Loader.CurFrame;
	padsize = PADSIZE(iffsize);
	lcwoffset = vqap->Max_Ptr_Size - padsize;

	/* Read the pointers into end of the pointer buffer. */
	buffer = curframe->Pointers + lcwoffset;

	if (vqap->Config.StreamHandler((VQAHandle *)vqap, VQACMD_READ, buffer, padsize)) {
		return(VQAERR_READ);
	}

	/* Flag this frame as compressed. */
	curframe->Flags |= VQAFRMF_PTRCOMP;
	curframe->PtrOffset = lcwoffset;

	if (vqap->Header.Version < VQAHD_VER3) {
		((unsigned char *)buffer)[iffsize + 1] = 0x80;
	}

	return(VQAERR_NONE);
}


#if(VQAAUDIO_ON)
/****************************************************************************
*
* NAME
*     Load_SND0 - Load uncompressed sound chunk.
*
* SYNOPSIS
*     Error = Load_SND0(VQA, Iffsize)
*
*     long Load_SND0(VQAHandleP *, unsigned long);
*
* FUNCTION
*     This routine normally loads the chunk into the TempBuf, unless the
*     chunk is larger than the temp buffer size, in which case it puts it
*     directly into the audio buffer itself.  This assumes that the only
*     such chunk will be the first audio chunk!
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long Load_SND0(VQAHandleP *vqap, unsigned long iffsize)
{
	VQAAudio      *audio;
	VQAConfig     *config;
	unsigned long padsize;
	unsigned long i;

	/* Dereference commonly used data members for quicker access. */
	audio = &vqap->Audio;
	config = &vqap->Config;
	padsize = PADSIZE(iffsize);

	/* If sound is disabled, or if we're playing from a VOC file, or if
	 * there's no Audio Buffer, just skip the chunk.
	 */
	#if(VQAVOC_ON && VQAAUDIO_ON)
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0)
			|| (vqap->vocfh != -1) || (audio->Buffer == NULL)) {
	#else  /* VQAVOC_ON */
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0)
			|| (audio->Buffer == NULL)) {
	#endif /* VQAVOC_ON */

		if (config->StreamHandler((VQAHandle *)vqap, VQACMD_SEEK, (void *)SEEK_CUR,
				padsize)) {
			return(VQAERR_SEEK);
		} else {
			return(VQAERR_NONE);
		}
	}

	/* Read large startup chunk directly into AudioBuf */
	if ((padsize > audio->TempBufSize) && (audio->AudBufPos == 0)) {
		if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, audio->Buffer,
				padsize)) {

			return(VQAERR_READ);
		}

		audio->AudBufPos += iffsize;

		/* Flag the audio frame flags as loaded for the initial audio frame. */
		for (i = 0; i < (iffsize / config->HMIBufSize); i++) {
			audio->IsLoaded[i] = 1;
		}

		return(VQAERR_NONE);
	} else {

		/*  Read data into TempBuf */
		if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, audio->TempBuf,
				padsize)) {

			return(VQAERR_READ);
		}
	}

	/* Set the TempBufLen */
	audio->TempBufLen = iffsize;

	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     Load_SND1 - Load compressed sound chunk.
*
* SYNOPSIS
*     Error = Load_SND1(VQA, Iffsize)
*
*     long Load_SND1(VQAHandleP *, unsigned long);
*
* FUNCTION
*     This routine normally loads the chunk into the TempBuf, unless the
*     chunk is larger than the temp buffer size, in which case it puts it
*     directly into the audio buffer itself.  This assumes that the only
*     such chunk will be the first audio chunk!
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long Load_SND1(VQAHandleP *vqap, unsigned long iffsize)
{
	VQAAudio      *audio;
	VQAConfig     *config;
	unsigned char *loadbuf;
	unsigned long padsize;
	ZAPHeader     zap;
	long          i;

	/* Dereference commonly used data members for quicker access. */
	audio = &vqap->Audio;
	config = &vqap->Config;
	padsize = PADSIZE(iffsize);

	/* If sound is disabled, or if we're playing from a VOC file, or if
	 * there's no Audio Buffer, just skip the chunk
	 */
	#if(VQAVOC_ON && VQAAUDIO_ON)
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0)
			|| (vqap->vocfh != -1) || (audio->Buffer == NULL)) {
	#else  /* VQAVOC_ON */
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0)
			|| (audio->Buffer == NULL)) {
	#endif /* VQAVOC_ON */

		if (config->StreamHandler((VQAHandle *)vqap, VQACMD_SEEK, (void *)SEEK_CUR,
				padsize)) {
			return(VQAERR_SEEK);
		} else {
			return(VQAERR_NONE);
		}
	}

	/* Read the ZAP audio frame header. */
	if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, &zap,
			sizeof(ZAPHeader))) {

		return(VQAERR_READ);
	}

	/* Adjust chunk size */
	padsize -= sizeof(ZAPHeader);

	/* Read large startup chunk directly into AudioBuf */
	if ((zap.UnCompSize > audio->TempBufSize) && (audio->AudBufPos == 0)) {

		/* Load RAW uncompressed data. */
		if (zap.UnCompSize == zap.CompSize) {
			if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, audio->Buffer,
					padsize)) {
				return(VQAERR_READ);
			}
		} else {

			/* Load compressed data into the end of the buffer. */
			loadbuf = (audio->Buffer + config->AudioBufSize) - padsize;

			if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, loadbuf,
					padsize)) {

				return(VQAERR_READ);
			}

			/* Uncompress the audio frame. */
			AudioUnzap(loadbuf, audio->Buffer, zap.UnCompSize);
		}

		/* Set buffer positions & flags */
		audio->AudBufPos += zap.UnCompSize;

		for (i = 0; i < (zap.UnCompSize / config->HMIBufSize); i++) {
			audio->IsLoaded[i] = 1;
		}

		return(VQAERR_NONE);
	}

	/* Load an audio frame. */
	if (zap.UnCompSize == zap.CompSize) {

		/* If the frame is uncompressed the load it in directly. */
		if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, audio->TempBuf,
				padsize)) {

			return(VQAERR_READ);
		}
	} else {

		/* Load the audio frame into the end of the buffer. */
		loadbuf = ((audio->TempBuf + audio->TempBufSize) - padsize);

		if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, loadbuf, padsize)) {
			return(VQAERR_READ);
		}

		/* Uncompress the audio frame. */
		AudioUnzap(loadbuf, audio->TempBuf, zap.UnCompSize);
	}

	/* Set the TempBufLen */
	audio->TempBufLen = zap.UnCompSize;

	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     Load_SND2 - Load ADPCM compressed sound chunk.
*
* SYNOPSIS
*     Error = Load_SND2(VQA, Iffsize)
*
*     long Load_SND2(VQAHandleP *, unsigned long);
*
* FUNCTION
*     This routine normally loads the chunk into the TempBuf, unless the
*     chunk is larger than the temp buffer size, in which case it puts it
*     directly into the audio buffer itself.  This assumes that the only
*     such chunk will be the first audio chunk!
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long Load_SND2(VQAHandleP *vqap, unsigned long iffsize)
{
	VQAAudio      *audio;
	VQAConfig     *config;
	unsigned char *loadbuf;
	unsigned long padsize;
	unsigned long uncomp_size;
	long          i;

	/* Dereference commonly used data members for quicker access. */
	audio = &vqap->Audio;
	config = &vqap->Config;
	padsize = PADSIZE(iffsize);

	/* If sound is disabled, or if we're playing from a VOC file, or if
	 * there's no Audio Buffer, just skip the chunk
	 */
	#if(VQAVOC_ON && VQAAUDIO_ON)
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0)
			|| (vqap->vocfh != -1) || (audio->Buffer == NULL)) {
	#else  /* VQAVOC_ON */
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0)
			|| (audio->Buffer == NULL)) {
	#endif /* VQAVOC_ON */

		if (config->StreamHandler((VQAHandle *)vqap, VQACMD_SEEK, (void *)SEEK_CUR,
				padsize)) {
			return(VQAERR_SEEK);
		} else {
			return(VQAERR_NONE);
		}
	}

	uncomp_size = iffsize * (vqap->BitsPerSample / 4);

	/* Read large startup chunk directly into AudioBuf */
	if ((uncomp_size > audio->TempBufSize) && (audio->AudBufPos == 0)) {

		/* Load compressed data into the end of the buffer. */
		loadbuf = (audio->Buffer + config->AudioBufSize) - padsize;

		if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, loadbuf, padsize)) {
			return(VQAERR_READ);
		}

		/* Uncompress the audio frame. */
		VQA_sosCODECDecompressData(loadbuf, (char *)audio->Buffer, vqap->BitsPerSample, vqap->Channels, uncomp_size, &audio->ADPCM_Info);

		if (audio->BufferOffset) {
			uncomp_size -= audio->BufferOffset;
			memcpy(audio->Buffer, audio->Buffer + audio->BufferOffset, uncomp_size);
			audio->BufferOffset = 0;
		}

		/* Set buffer positions & flags */
		audio->AudBufPos += uncomp_size;

		for (i = 0; i < (uncomp_size / config->HMIBufSize); i++) {
			audio->IsLoaded[i] = true;
		}

		return(VQAERR_NONE);
	}

	/* Load an audio frame. */
	loadbuf = ((audio->TempBuf + audio->TempBufSize) - padsize);

	if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, loadbuf, padsize)) {
		return(VQAERR_READ);
	}

	/* Uncompress the audio frame. */
	VQA_sosCODECDecompressData(loadbuf, (char *)audio->TempBuf, vqap->BitsPerSample, vqap->Channels, uncomp_size, &audio->ADPCM_Info);

	/* Set the TempBufLen */
	audio->TempBufLen = uncomp_size;

	return(VQAERR_NONE);
}


long Load_SN2J(VQAHandleP *vqap, unsigned long iffsize)
{
	unsigned long padsize;
	VQAConfig     *config;
	VQAAudio      *audio;

	/* Dereference commonly used data members for quicker access. */
	audio = &vqap->Audio;
	config = &vqap->Config;

	padsize = PADSIZE(iffsize);

	#pragma pack(push,1)
	struct SNJ2Struct {
		unsigned short wIndex;
		unsigned int dwPredicted;
		unsigned short wIndex2;
		unsigned int dwPredicted2;
	} data;
	#pragma pack(pop)

	#if(VQAVOC_ON && VQAAUDIO_ON)
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0)
			|| (vqap->vocfh != -1) || (audio->Buffer == NULL)) {
	#else  /* VQAVOC_ON */
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0)
			|| (audio->Buffer == NULL)) {
	#endif /* VQAVOC_ON */

		if (config->StreamHandler((VQAHandle *)vqap, VQACMD_SEEK, (void *)SEEK_CUR,
				padsize)) {
			return(VQAERR_SEEK);
		}
		return(VQAERR_NONE);
	}

	if (padsize <= sizeof(data)) {
		if (config->StreamHandler((VQAHandle *)vqap, VQACMD_READ, &data,
			padsize)) {
			return(VQAERR_READ);
		}
	} else {
		return(VQAERR_READ);
	}

	audio->ADPCM_Info.dwPredicted = data.dwPredicted;
	audio->ADPCM_Info.wIndex = data.wIndex;
	if (vqap->Channels == 2) {
		audio->ADPCM_Info.dwPredicted2 = data.dwPredicted2;
		audio->ADPCM_Info.wIndex2 = data.wIndex2;
	}

	return(VQAERR_NONE);
}
#endif /* VQAAUDIO_ON */