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

#include	"vqaplayp.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"vqamem.h"

extern VQAErrorType VQA_Set_DrawBuffer(VQAHandle *vqa, unsigned char *buffer, unsigned long width, unsigned long height, long xpos, long ypos);


_STATIC void VQA_BufferPerpareLoop(VQAHandleP *vqap, long * needs_start_cb, long * needs_end_cb);


/****************************************************************************
*
* NAME
*     AllocBuffers - Allocate VQA play buffers.
*
* SYNOPSIS
*     VQAData = AllocBuffers(Header, Config)
*
*     VQAData *AllocBuffers(VQAHeader *, VQAConfig *);
*
* FUNCTION
*     For those structures that contain buffer pointers (codebook nodes,
*     frame buffer nodes), enough memory is allocated for both the structure
*     and its associated buffers, then the buffer pointers are pointed to
*     the appropriate offset from the structure pointer.  This allows us
*     to perform only one malloc & free for each node.
*
*     Buffers allocated:
*       - vqa
*       - vqa->CBData (list)
*       - vqa->FrameData (list)
*       - vqa->Drawer.ImageBuf
*       - vqa->Audio.Buffer
*       - vqa->Audio.IsLoaded
*       - vqa->Foff
*
* INPUTS
*     Header - Pointer to VQAHeader structure.
*     Config - Pointer to VQA configuration structure.
*
* RESULT
*     VQAData - Pointer to initialized VQAData structure.
*
****************************************************************************/

STATIC VQAErrorType AllocBuffers(VQAHandleP *vqap)
{
	long needs_start_cb;
	long needs_end_cb;
	VQACBNode    *cbnode;
	VQACBNode    *this_cb;
	VQAFrameNode *framenode;
	VQAFrameNode *this_frame;
	long         i;
	VQAConfig    *config;
	VQAHeader    *header;

	config = &vqap->Config;
	header = &vqap->Header;

	/* Check the configuration for valid values. */
	if (config->NumFrameBufs == 0) {
		return(VQAERR_NOMEM);
	}


	/*-------------------------------------------------------------------------
	 * INITIALIZE THE VQA DATA STRUCTURES.
	 *
	 * Pointers are set to NULL initially, and filled in as the buffers are
	 * allocated.  The Max buffer sizes are computed with 1K of padding,
	 * and'd with 0xFFFC to make the size divisible by 4, to ensure DWORD
	 * alignment.
	 *-----------------------------------------------------------------------*/

	/* Set maximum codebook size. */
	vqap->CBBufferSize = ALIGNUP(header->CBentries * header->BlockWidth * header->BlockHeight + 1, 4);

	if (header->ColorMode == 1 || header->ColorMode == 4) {
		vqap->CBBufferSize *= 2;
	}

	if (header->MaxCBSize > vqap->CBBufferSize) {
		return(VQAERR_OPEN);
	}

	if (header->MaxCBSize == 0) {
		header->MaxCBSize = vqap->CBBufferSize;
	}

	if (config->NumCBBufs <= 0) {
		long groupsize;
		if (vqap->CodebookInfo.Header.Count > 0) {
			groupsize = vqap->CodebookInfo.Header.Groupsize;
		} else {
			groupsize = header->Groupsize;
		}
		if (groupsize == vqap->NumFrames) {
			config->NumCBBufs = 1;
		} else {
			int newcbcount = 0;
			if (config->NumFrameBufs > 1) {
				newcbcount = (config->NumFrameBufs - 2) / groupsize + 1;
				if (config->OptionFlags & VQAOPTF_LOOPS) {
					if (vqap->LoopInfo.Header.Count == 0) {
						newcbcount++;
						newcbcount++;
					} else {
						VQA_BufferPerpareLoop(vqap, &needs_start_cb, &needs_end_cb);
						if (needs_start_cb != 0) {
							newcbcount++;
						}
						if (needs_end_cb != 0) {
							newcbcount++;
						}
					}
				}
			}
			config->NumCBBufs = newcbcount + 2;
		}
	}

	vqap->AltBufferFlags |= VQAABUFF_ALTCB;
	vqap->Max_CB_Size = ALIGNUP(header->MaxCBSize + 13, 4);

	vqap->AltCBBuffer = (void *)config->MemoryHandler((VQAHandle *)vqap, VQAMEM_ALLOC, NULL, vqap->CBBufferSize);

	if (vqap->AltCBBuffer == NULL) {
		FreeBuffers(vqap);
		return(VQAERR_NOMEM);
	}

	config->MemoryHandler((VQAHandle *)vqap, VQAMEM_LOCK, vqap->AltCBBuffer, vqap->CBBufferSize);
	vqap->MemUsed += vqap->CBBufferSize;


	/*-------------------------------------------------------------------------
	 * ALLOCATE THE CODEBOOK BUFFERS.
	 *-----------------------------------------------------------------------*/
	cbnode = NULL;
	this_cb = NULL;

	for (i = 0; i < config->NumCBBufs; i++) {

		/* Allocate a codebook node. */
		cbnode = (VQACBNode *)config->MemoryHandler((VQAHandle*)vqap, VQAMEM_ALLOC, NULL, (sizeof(VQACBNode) + vqap->Max_CB_Size));

		/* If failure then clean up and exit. */
		if (cbnode == NULL) {
			FreeBuffers(vqap);
			return(VQAERR_NOMEM);
		}

		/* Lock the buffer to prevent page swapping. */
		config->MemoryHandler((VQAHandle*)vqap, VQAMEM_LOCK, cbnode, (sizeof(VQACBNode) + vqap->Max_CB_Size));

		/* Keep count of the memory usage. */
		vqap->MemUsed += (long)(sizeof(VQACBNode) + vqap->Max_CB_Size);

		/* Initialize the node */
		memset(cbnode, 0, sizeof(VQACBNode));
		cbnode->Buffer = (unsigned char *)cbnode + sizeof(VQACBNode);

		/* Install the node */
		if (this_cb == NULL) {
			vqap->CBData = cbnode;
			this_cb = cbnode;
		} else {
			this_cb->Next = cbnode;
			cbnode->Prev = this_cb;
			this_cb = cbnode;
		}
	}

	/* Make the list circular */
	if (cbnode != NULL)
	{
		cbnode->Next = vqap->CBData;
		vqap->CBData->Prev = cbnode;
	}

	/* Install the Codebook list */
	vqap->Loader.CurCB = vqap->CBData;
	vqap->Loader.FullCB = vqap->CBData;

	vqap->Max_CB_Size -= 4;

	/* Set maximum palette size. */
	vqap->Max_Pal_Size = ALIGNUP(1024 - 4, 4);

	/* Set maximum vector pointers size. */
	vqap->PtrBufferSize = ALIGNUP((header->ImageWidth / header->BlockWidth)
			* (header->ImageHeight / header->BlockHeight)
			* sizeof(short) + 1, 4);

	/* Set maximum vector pointers size (from the largest frame). */
	{
		unsigned long maxptr;

		if (header->MaxFramesize == 0) {
			maxptr = vqap->PtrBufferSize;
		} else {
			maxptr = header->MaxFramesize;
			if (header->Version > 2) {
				maxptr <<= 8;
			}
		}

		vqap->AltBufferFlags |= VQAABUFF_ALTPTR;
		vqap->Max_Ptr_Size = ALIGNUP(maxptr + 5, 4);
	}

	vqap->AltPtrBuffer = (void *)config->MemoryHandler((VQAHandle *)vqap, VQAMEM_ALLOC, NULL, vqap->PtrBufferSize);


	if (vqap->AltPtrBuffer == NULL) {
		FreeBuffers(vqap);
		return(VQAERR_NOMEM);
	}

	config->MemoryHandler((VQAHandle *)vqap, VQAMEM_LOCK, vqap->AltPtrBuffer, vqap->PtrBufferSize);
	vqap->MemUsed += vqap->PtrBufferSize;

	/*-------------------------------------------------------------------------
	 * ALLOCATE THE FRAME BUFFERS.
	 *-----------------------------------------------------------------------*/
	framenode = NULL;
	this_frame = NULL;

	for (i = 0; i < config->NumFrameBufs; i++) {

		/* Allocate a pointer node */
		framenode = (VQAFrameNode *)config->MemoryHandler((VQAHandle*)vqap, VQAMEM_ALLOC, NULL, (sizeof(VQAFrameNode)
				+ vqap->Max_Ptr_Size + vqap->Max_Pal_Size));

		/* If failure then clean up and exit. */
		if (framenode == NULL) {
			FreeBuffers(vqap);
			return(VQAERR_NOMEM);
		}

		/* Lock the buffer to prevent page swapping. */
		config->MemoryHandler((VQAHandle*)vqap, VQAMEM_LOCK, framenode, sizeof(VQAFrameNode) + vqap->Max_Ptr_Size
				+ vqap->Max_Pal_Size);

		/* Keep count of the memory usage. */
		vqap->MemUsed += (long)(sizeof(VQAFrameNode) + vqap->Max_Ptr_Size
				+ vqap->Max_Pal_Size);

		/* Initialize the node */
		memset(framenode, 0, sizeof(VQAFrameNode));
		framenode->Pointers = (unsigned char *)framenode + sizeof(VQAFrameNode);
		framenode->Palette = (unsigned char *)framenode + sizeof(VQAFrameNode)
				+ vqap->Max_Ptr_Size;

		framenode->Codebook = NULL;

		/* Install the node */
		if (this_frame == NULL) {
			vqap->FrameData = framenode;
			this_frame = framenode;
		} else {
			this_frame->Next = framenode;
			framenode->Prev = this_frame;
			this_frame = framenode;
		}
	}

	/* Make the list circular */
	if (framenode != NULL)
	{
		framenode->Next = vqap->FrameData;
		vqap->FrameData->Prev = framenode;
	}

	/* Install the Frame Buffer list */
	vqap->Loader.CurFrame = vqap->FrameData;
	vqap->Drawer.CurFrame = vqap->FrameData;
	vqap->Flipper.CurFrame = vqap->FrameData;

	vqap->Max_Pal_Size -= 4;
	vqap->Max_Ptr_Size -= 4;

	/*-------------------------------------------------------------------------
	 * ALLOCATE AND INITIALIZE AUDIO BUFFERS AND STRUCTURES.
	 *-----------------------------------------------------------------------*/
	#if(VQAAUDIO_ON)
	if ((header->Flags & VQAHDF_AUDIO)
			&& (config->OptionFlags & VQAOPTF_AUDIO)) {

		/* Dereference audio structure for quick access. */
		VQAAudio *audio = &vqap->Audio;

		/* Version 1 VQA's only supported 22050 8 bit mono audio. */
		if (header->Version < VQAHD_VER2) {
			vqap->SampleRate = 22050U;
			vqap->Channels = 1;
			vqap->BitsPerSample = 8;
			audio->BytesPerSec = 22050U;
		} else {
			if ((config->OptionFlags & VQAOPTF_ALTAUDIO)
					&& (header->Flags & VQAHDF_ALTAUDIO)) {
				vqap->SampleRate = header->AltSampleRate;
				vqap->Channels = header->AltChannels;
				vqap->BitsPerSample = header->AltBitsPerSample;
			} else {
				vqap->SampleRate = header->SampleRate;
				vqap->Channels = header->Channels;
				vqap->BitsPerSample = header->BitsPerSample;
			}

			audio->BytesPerSec = ((vqap->SampleRate * vqap->Channels)
					* (vqap->BitsPerSample >> 3));
		}

		/* Adjust the HMI buffer to accomodate the amount of data. */
		#if(0)
		config->HMIBufSize *= (audio->SampleRate / 22050);
		config->HMIBufSize *= audio->Channels * (audio->BitsPerSample >> 3);
		#endif

		/* The default audio buffer size should be large enough to hold
		 * 1.5 seconds of data.
		 */
		if (config->AudioBufSize == -1) {

			/* Compute the number of HMI buffers that will completly fit into
			 * 1.5 seconds of audio data.
			 */
			i = ((audio->BytesPerSec+(audio->BytesPerSec/2))/config->HMIBufSize);
			config->AudioBufSize = (config->HMIBufSize * i);
		}

		/* Do not allocate anything if the audio buffer is zero length. */
		if (config->AudioBufSize > 0) {

			/* Allocate an audio buffer if the user did not provide one.
			 * Otherwise, use the user supplied buffer.
			 */
			if (config->AudioBuf == NULL) {
				audio->Buffer = (unsigned char *)config->MemoryHandler((VQAHandle*)vqap, VQAMEM_ALLOC, NULL, config->AudioBufSize);

				/* If failure then clean up and exit. */
				if (audio->Buffer == NULL) {
					FreeBuffers(vqap);
					return(VQAERR_NOMEM);
				}

				config->MemoryHandler((VQAHandle*)vqap, VQAMEM_LOCK, audio->Buffer, config->AudioBufSize);

				/* Add audio buffer size to memory usage. */
				vqap->MemUsed += config->AudioBufSize;
			} else {
				audio->Buffer = config->AudioBuf;
			}

			/* Allocate IsLoaded flags */
			audio->NumAudBlocks = (config->AudioBufSize / config->HMIBufSize);
			audio->IsLoaded = (bool *)config->MemoryHandler((VQAHandle*)vqap, VQAMEM_ALLOC, NULL, audio->NumAudBlocks * sizeof(*audio->IsLoaded));

			/* If failure then clean up and exit. */
			if (audio->IsLoaded == NULL) {
				FreeBuffers(vqap);
				return(VQAERR_NOMEM);
			}

			/* Lock to prevent page swapping. */
			config->MemoryHandler((VQAHandle*)vqap, VQAMEM_LOCK, audio->IsLoaded, audio->NumAudBlocks * sizeof(*audio->IsLoaded));

			/* Add IsLoaded flags array to memory usage. */
			vqap->MemUsed += (audio->NumAudBlocks * sizeof(*audio->IsLoaded));

			/* Initialize audio IsLoaded flags to false. */
			memset(audio->IsLoaded, 0, audio->NumAudBlocks * sizeof(*audio->IsLoaded));

			audio->BlockRepeats = (short *)config->MemoryHandler((VQAHandle*)vqap, VQAMEM_ALLOC, NULL, audio->NumAudBlocks * sizeof(*audio->BlockRepeats));

			if (audio->BlockRepeats == NULL) {
				FreeBuffers(vqap);
				return(VQAERR_NOMEM);
			}

			config->MemoryHandler((VQAHandle*)vqap, VQAMEM_LOCK, audio->BlockRepeats, audio->NumAudBlocks * sizeof(*audio->BlockRepeats));

			vqap->MemUsed += (audio->NumAudBlocks * sizeof(*audio->BlockRepeats));

			memset(audio->BlockRepeats, 0, audio->NumAudBlocks * sizeof(*audio->BlockRepeats));

			/* Allocate temporary staging buffer for the audio frames. */
			audio->TempBufSize = ((audio->BytesPerSec / header->FPS) * 2) + 100;
			audio->TempBuf = (unsigned char *)config->MemoryHandler((VQAHandle*)vqap, VQAMEM_ALLOC, NULL, audio->TempBufSize);

			if (audio->TempBuf == NULL) {
				FreeBuffers(vqap);
				return(VQAERR_NOMEM);
			}

			/* Lock to prevent page swapping. */
			config->MemoryHandler((VQAHandle*)vqap, VQAMEM_LOCK, audio->TempBuf, audio->TempBufSize);

			/* Add temporary buffer size to memory usage. */
			vqap->MemUsed += audio->TempBufSize;

			audio->HMIBuffer = (unsigned char *)config->MemoryHandler((VQAHandle*)vqap, VQAMEM_ALLOC, NULL, config->HMIBufSize);

			if (audio->HMIBuffer == NULL) {
				FreeBuffers(vqap);
				return(VQAERR_NOMEM);
			}

			config->MemoryHandler((VQAHandle*)vqap, VQAMEM_LOCK, audio->HMIBuffer, config->HMIBufSize);

			memset(audio->HMIBuffer, 0, config->HMIBufSize);

			vqap->MemUsed += config->HMIBufSize;

		} else {
			FreeBuffers(vqap);
			return(VQAERR_NOAUDSIZ);
		}
	}
	#endif /* VQAAUDIO_ON */

	/*-------------------------------------------------------------------------
	 * ALLOCATE THE FRAME INFORMATION TABLE IF REQUESTED.
	 *-----------------------------------------------------------------------*/
	vqap->Foff = (long *)config->MemoryHandler((VQAHandle *)vqap, VQAMEM_ALLOC, NULL, header->Frames * sizeof(*vqap->Foff));

	if (vqap->Foff == NULL) {
		FreeBuffers(vqap);
		return(VQAERR_NOMEM);
	}

	/* Lock to prevent page swapping. */
	config->MemoryHandler((VQAHandle *)vqap, VQAMEM_LOCK, vqap->Foff, header->Frames * sizeof(*vqap->Foff));

	/* Keep a running total of memory usage. */
	vqap->MemUsed += (header->Frames * sizeof(long));

	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     FreeBuffers - Free VQA play buffers.
*
* SYNOPSIS
*     FreeBuffers(VQAData, Config, Header)
*
*     void FreeBuffers(VQAData *, VQAConfig *, VQAHeader *);
*
* FUNCTION
*      Free the buffers allocated by AllocBuffers().
*
* INPUTS
*      VQAData - Pointer to VQAData structure.
*      Config  - Pointer to configuration structure.
*      Header  - Pointer to movie header structure.
*
* RESULT
*      NONE
*
****************************************************************************/

STATIC void FreeBuffers(VQAHandleP *vqap)
{
	VQACBNode    *cb_this,
	             *cb_next;
	VQAFrameNode *frame_this,
	             *frame_next;
	long         i;

	VQAAudio *audio;
	VQAConfig *config;
	VQALoader *loader;
	VQAHeader *header;

	config = &vqap->Config;
	header = &vqap->Header;
	loader = &vqap->Loader;
	audio = &vqap->Audio;

	///////////////////////////////////////////////////////////////////////////
	// free loop cache buffer
	///////////////////////////////////////////////////////////////////////////

	if (vqap->LoopCache.Ptr != NULL) {
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, vqap->LoopCache.Ptr, vqap->LoopCache.Size);
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, vqap->LoopCache.Ptr, NULL);
	}

	/*-------------------------------------------------------------------------
	 * FREE THE FRAME INFORMATION TABLE.
	 *-----------------------------------------------------------------------*/
	if (vqap->Foff) {
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, vqap->Foff, header->Frames * sizeof(*vqap->Foff));
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, vqap->Foff, NULL);
	}

	/*-------------------------------------------------------------------------
	 * FREE THE AUDIO BUFFERS.
	 *-----------------------------------------------------------------------*/
	#if(VQAAUDIO_ON)
	if (config->AudioBuf == NULL && audio->Buffer != NULL) {
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, audio->Buffer, config->AudioBufSize);
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, audio->Buffer, NULL);
	}

	/* Free the audio segments loaded flag array. */
	if (audio->IsLoaded) {
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, audio->IsLoaded, audio->NumAudBlocks * sizeof(*audio->IsLoaded));
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, audio->IsLoaded, NULL);
	}

	if (audio->BlockRepeats != NULL) {
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, audio->BlockRepeats, audio->NumAudBlocks * sizeof(*audio->BlockRepeats));
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, audio->BlockRepeats, NULL);
	}

	/* Free the temporary audio buffer. */
	if (audio->TempBuf != NULL) {
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, audio->TempBuf, audio->TempBufSize);
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, audio->TempBuf, NULL);
	}

	if (audio->HMIBuffer != NULL) {
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, audio->HMIBuffer, config->HMIBufSize);
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, audio->HMIBuffer, NULL);
	}

	#endif /* VQAAUDIO_ON */

	/*-------------------------------------------------------------------------
	 * FREE THE FRAME BUFFERS.
	 *-----------------------------------------------------------------------*/
	frame_this = vqap->FrameData;

	for (i = 0; i < config->NumFrameBufs; i++) {
		if (frame_this) {
			frame_next = frame_this->Next;

			vqap->Max_Pal_Size += 4;
			vqap->Max_Ptr_Size += 4;

			config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, frame_this, sizeof(VQAFrameNode) + vqap->Max_Ptr_Size
					+ vqap->Max_Pal_Size);
			config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, frame_this, NULL);
			frame_this = frame_next;
		} else {
			break;
		}
	}

	if (vqap->AltBufferFlags & VQAABUFF_ALTPTR) {
		if (vqap->AltPtrBuffer != NULL) {
			config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, vqap->AltPtrBuffer, vqap->PtrBufferSize);
			config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, vqap->AltPtrBuffer, NULL);
			vqap->AltPtrBuffer = NULL;
		}
	}

	/*-------------------------------------------------------------------------
	 * FREE THE CODEBOOK BUFFERS.
	 *-----------------------------------------------------------------------*/
	cb_this = vqap->CBData;

	for (i = 0; i < config->NumCBBufs; i++) {
		if (cb_this) {
			cb_next = cb_this->Next;

			vqap->Max_CB_Size += 4;

			config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, cb_this, sizeof(VQACBNode) + vqap->Max_CB_Size);
			config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, cb_this, NULL);
			cb_this = cb_next;
		} else {
			break;
		}
	}

	if (vqap->AltBufferFlags & VQAABUFF_ALTCB) {
		if (vqap->AltCBBuffer != NULL) {
			config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, vqap->AltCBBuffer, vqap->CBBufferSize);
			config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, vqap->AltCBBuffer, NULL);
			vqap->AltCBBuffer = NULL;
		}
	}

	/*-------------------------------------------------------------------------
	 * FREE THE VQA DATA STRUCTURES.
	 *-----------------------------------------------------------------------*/

	///////////////////////////////////////////////////////////////////////////
	// free pin data
	///////////////////////////////////////////////////////////////////////////

	if (vqap->PaletteInfo.Data != NULL) {
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, vqap->PaletteInfo.Data, vqap->PaletteInfo.Header.Count * sizeof(*vqap->PaletteInfo.Data));
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, vqap->PaletteInfo.Data, NULL);
	}

	///////////////////////////////////////////////////////////////////////////
	// free cin data
	///////////////////////////////////////////////////////////////////////////

	if (vqap->CodebookInfo.Data != NULL) {
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, vqap->CodebookInfo.Data, vqap->CodebookInfo.Header.Count * sizeof(*vqap->CodebookInfo.Data));
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, vqap->CodebookInfo.Data, NULL);
	}

	unsigned long count;
	unsigned long tablecount;
	unsigned long k;
	unsigned long staticcount;

	///////////////////////////////////////////////////////////////////////////
	// free msci data
	///////////////////////////////////////////////////////////////////////////
	tablecount = vqap->MSCInfo.Header.Count;
	for (k = 0; k < tablecount; k++) {
		count = vqap->MSCInfo.Data2[k].Count;
		VQAMSCInfo::DATA2::DATA *data = vqap->MSCInfo.Data2[k].Data;
		void *buf = data->Buffer;
		if (buf != NULL) {
			VQAMSCInfo::TABLE *table = &vqap->MSCInfo.Table[k];
			config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, buf, count * PADSIZE(table->EntrySize));
			config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, buf, NULL);
		}
		if (data != NULL) {
			config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, data, count * sizeof(*data));
			config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, data, NULL);
		}
	}

	if (vqap->MSCInfo.Data2 != NULL) {
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, vqap->MSCInfo.Data2, tablecount * sizeof(*vqap->MSCInfo.Data2));
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, vqap->MSCInfo.Data2, NULL);
	}

	if (vqap->MSCInfo.Table != NULL) {
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, vqap->MSCInfo.Table, tablecount * sizeof(*vqap->MSCInfo.Table));
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, vqap->MSCInfo.Table, NULL);
	}

	///////////////////////////////////////////////////////////////////////////
	// free mfci data
	///////////////////////////////////////////////////////////////////////////

	tablecount = vqap->MFCInfo.Header.Count;
	for (k = 0; k < tablecount; k++) {
		count = vqap->MFCInfo.Data2[k].Count;
		VQAMFCInfo::DATA2::DATA *data = vqap->MFCInfo.Data2[k].Data;
		void *buf = data->Buffer;
		if (buf != NULL) {
			VQAMFCInfo::TABLE *table = &vqap->MFCInfo.Table[k];
			config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, buf, count * PADSIZE(table->EntrySize));
			config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, buf, NULL);
		}
		if (data != NULL) {
			config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, data, count * sizeof(*data));
			config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, data, NULL);
		}
	}

	if (vqap->MFCInfo.Data2 != NULL) {
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, vqap->MFCInfo.Data2, tablecount * sizeof(*vqap->MFCInfo.Data2));
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, vqap->MFCInfo.Data2, NULL);
	}

	if (vqap->MFCInfo.Table != NULL) {
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, vqap->MFCInfo.Table, tablecount * sizeof(*vqap->MFCInfo.Table));
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, vqap->MFCInfo.Table, NULL);
	}

	staticcount = vqap->MFCInfo.Header.StaticCount;
	if (vqap->MFCInfo.StaticData != NULL) {
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, vqap->MFCInfo.StaticData, staticcount * sizeof(*vqap->MFCInfo.StaticData));
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, vqap->MFCInfo.StaticData, NULL);
	}

	///////////////////////////////////////////////////////////////////////////
	// free loop data
	///////////////////////////////////////////////////////////////////////////

	if (vqap->LoopInfo.Data != NULL) {
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, vqap->LoopInfo.Data, vqap->LoopInfo.Header.Count * sizeof(*vqap->LoopInfo.Data));
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, vqap->LoopInfo.Data, NULL);
	}
}


VQAErrorType VQA_Configure_Buffer(VQAHandleP *vqap)
{
	unsigned char *buf;
	int w;
	int h;
	VQAConfig *config;
	VQAHeader *header;

	header = &vqap->Header;
	config = &vqap->Config;

	if (config->ImageWidth == -1 || config->ImageHeight == -1) {
		w = header->ImageWidth;
		h = header->ImageHeight;
		if (config->DrawFlags & VQACFGF_OFFSET) {
			w += header->Xpos;
			h += header->Ypos;
		}
	} else {
		w = config->ImageWidth;
		h = config->ImageHeight;
	}


	/*-------------------------------------------------------------------------
	 * ALLOCATE THE IMAGE BUFFERS IF ONE IS NOT ALREADY PROVIDED.
	 *-----------------------------------------------------------------------*/
	if (config->DrawFlags & VQACFGF_BUFFER) {
		/* Allocate our own buffer. */
		if (config->ImageBuf == NULL) {
			int size = w * h;
			if (header->ColorMode == 1 || header->ColorMode == 4) {
				size *= 2;
			}
			/* If the allocation failed we must free up and exit. */
			buf = (unsigned char *)config->MemoryHandler(((VQAHandle *)vqap), VQAMEM_ALLOC, NULL, size);
			if (buf == NULL) {
				return(VQAERR_NOMEM);
			}
			vqap->MemUsed += size;

			/* Lock to prevent page swapping. */
			config->MemoryHandler(((VQAHandle *)vqap), VQAMEM_LOCK, buf, size);
			memset(buf, 0, size);

			/* Plugin image buffer information. */
			vqap->AltImageBuf = buf;
			vqap->AltImageWidth = w;
			vqap->AltImageHeight = h;
			vqap->Flags |= VQADATF_ALTIMG;
		} else {
			/* Use caller provided buffer */
			buf = config->ImageBuf;
		}
	} else {
		/* Use caller provided buffer */
		if (config->ImageBuf != NULL) {
			buf = config->ImageBuf;
		} else {
			return(VQAERR_NOBUFFER);
		}
	}

	VQA_Set_DrawBuffer((VQAHandle *)vqap, buf, w, h, config->X1, config->Y1);
	vqap->Flags |= VQADATF_BUFCONFIG;
	return(VQAERR_NONE);
}


long __cdecl VQA_Memory_Handler(VQAHandle *vqa, long action, void *buffer, long nbytes)
{
	long error = 0;

	switch (action) {
		default:
			break;

		case VQAMEM_1:
			break;

		case VQAMEM_ALLOC:
			error = (long)malloc(nbytes);
			break;

		case VQAMEM_FREE:
			free(buffer);
			break;

		case VQAMEM_LOCK:
			error = (long)buffer;
			break;

		case VQAMEM_UNLOCK:
			error = (long)buffer;
			break;

		case VQAMEM_QUERYSIZE:
			*((long *)buffer) = -1;
			break;

	}
	return(error);
}


/// <summary>
/// Determines whether any loop boundary falls off a codebook boundary.
/// This routine is used while sizing the codebook ring, so that extra slots can be
/// reserved when a mid-loop seek will need a codebook of its own. A codebook boundary
/// is a multiple of the group size, or -- for a movie that has no fixed group size --
/// any frame listed in the codebook information table.
/// </summary>
/// <param name="needs_start_cb">Set to 1 if a loop start needs an extra codebook buffer.</param>
/// <param name="needs_end_cb">Set to 1 if a loop end needs an extra codebook buffer.</param>
void VQA_BufferPerpareLoop(VQAHandleP *vqap, long * needs_start_cb, long * needs_end_cb)
{
	VQALoopInfo::DATA *ldata;
	VQACodebookInfo::DATA *cdata;
	int groupsize;
	int lcount;
	int ccount;
	int lindex;
	int cindex;

	lcount = vqap->LoopInfo.Header.Count;
	ldata = vqap->LoopInfo.Data;

	groupsize = vqap->Header.Groupsize;
	if (groupsize > 0) {
		*needs_start_cb = 0;
		*needs_end_cb = 0;

		for (lindex = 0; lindex < lcount; lindex++) {
			if ((ldata[lindex].StartFrame % groupsize)) {
				*needs_start_cb = 1;
				break;
			}
		}

		for (lindex = 0; lindex < lcount; lindex++) {
			if ((ldata[lindex].EndFrame % groupsize)) {
				*needs_end_cb = 1;
				break;
			}

		}
	} else {
		*needs_start_cb = 1;
		*needs_end_cb = 1;
		ccount = vqap->CodebookInfo.Header.Count;
		cdata = vqap->CodebookInfo.Data;

		for (lindex = 0; lindex < lcount; lindex++) {
			if (*needs_start_cb) {
				for (cindex = 0; cindex < ccount; cindex++) {
					if (ldata[lindex].StartFrame == cdata[cindex].Frame) {
						*needs_start_cb = 0;
						break;
					}
				}
			}

			if (*needs_end_cb) {
				for (cindex = 0; cindex < ccount; cindex++) {
					if (ldata[lindex].EndFrame == cdata[cindex].Frame) {
						*needs_end_cb = 0;
						break;
					}
				}
			}

			if (!*needs_start_cb && !*needs_end_cb) {
				break;
			}
		}
	}
}
