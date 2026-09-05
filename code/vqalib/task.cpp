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
*     task.c
*
* DESCRIPTION
*     Loading and drawing delegation
*
* PROGRAMMER
*     Bill Randolph
*     Denzil E. Long, Jr.
*
* DATE
*     July 25, 1995
*
*----------------------------------------------------------------------------
*
* PUBLIC
*     VQA_Alloc    - Allocate a VQAHandle to use.
*     VQA_Free     - Free a VQAHandle.
*     VQA_Init     - Initialize the VQAHandle IO.
*     VQA_Play     - Play the VQA movie.
*     VQA_SetStop  - Set the frame the player should stop on.
*     VQA_GetInfo  - Get VQA movie information.
*     VQA_GetStats - Get VQA movie statistics.
*     VQA_Version  - Get VQA library version number.
*     VQA_IDString - Get the VQA player library's ID string.
*
* PRIVATE
*     VQA_IO_Task        - Loader task for multitasking.
*     VQA_Rendering_Task - Drawer task for multitasking.
*     User_Update        - Page flip routine called by the task interrupt.
*
****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "vqaplay.h"
#include "vqaplayp.h"
#include "video.h"
#include "ahandle.h"
#include "lcw.h"
#include "vqadebug.h"


long VQA_ResetLastFrameNum(VQAHandle *vqa);
void VQA_DispatchFrameChunks(VQAHandleP *vqap, long frame);
void VQA_ResetCache(VQAHandleP *vqap);
long VQA_Configure_Buffer(VQAHandleP *vqap);
void VQA_SetTimer(VQAHandleP *vqap, long time);
unsigned long VQA_GetTime(VQAHandleP *vqap);
void VQA_StartAudio(VQAHandleP *vqap);
void VQA_StopAudio(VQAHandleP *vqap);
long VQA_LoadFrame(VQAHandleP *vqap, long flags);
long User_Update(VQAHandle *vqa);
long VQA_SetLoop(VQAHandle *vqa, int id, int iterations, int mode);
long VQA_SetLoop_Internal(VQAHandle *vqa, int start, int end, int iterations, int mode);
void VQA_Reset(VQAHandle *vqap);
long VQA_Configure_Drawer(VQAHandleP *vqap);
long VQA_NumFramesWithPalettes(VQAHandleP *vqap);

long VQA_ReloadPalette(VQAHandleP *vqap, long framenum, int force);
VQABool VQA_IsFrameStartOfLoop(VQAHandleP *vqap, long framenum);
long VQA_SeekGroup(VQAHandleP *vqap, long framenum, long groupsize, VQABool preloadaudio, VQABool reset_state, VQABool &skipcodebook);


/*---------------------------------------------------------------------------
 * PRIVATE DECLARATIONS
 *-------------------------------------------------------------------------*/

long PrimeBuffers(VQAHandle *vqa);

long Load_FINF(VQAHandleP *vqap, unsigned long iffsize);

long Load_CINF(VQAHandleP *vqap);
long Load_PINF(VQAHandleP *vqap);
long Load_LINF(VQAHandleP *vqap);
long Load_CLIP(VQAHandleP *vqap, unsigned long iffsize);
long Load_MFCI(VQAHandleP *vqap);
long Load_MSCI(VQAHandleP *vqap);

long __cdecl VQA_Memory_Handler(VQAHandle *vqa, long action, void *buffer, long nbytes);
long __cdecl Disk_VQA_Stream_Handler(VQAHandle *vqa, long action, void *buffer, long nbytes);

long VQA_LargestLoop(VQAHandleP *vqap, long);

extern void __cdecl UnVQ_Nop(uint8_t *codebook, uint8_t *pointers,
		uint8_t *buffer, size_t blocksperrow,
		size_t numrows, size_t bufwidth);

/****************************************************************************
*
* NAME
*     VQA_Open - Open a VQA file to play.
*
* SYNOPSIS
*     Error = VQA_Open(VQA, Name, Config)
*
*     long VQA_Open(VQAHandle *, char *, VQAConfig *);
*
* FUNCTION
*     - Open a VQA file for reading.
*     - Validate that it is an IFF file, of the VQA type.
*     - Read the VQA header.
*     - Open a VOC file for playback, if requested.
*     - Set the Loader's frame rate, if the caller's Config structure's
*       FrameRate is set to -1
*     - Set the Drawer's frame rate, if the caller's Config structure's
*       DrawRate is set to -1
*
* INPUTS
*     VQA    - Pointer to initialized handle. Obtained by VQA_Alloc().
*     Name   - Pointer to name of VQA file to open.
*     Config - Pointer to initialized VQA configuration structure.
*
* RESULT
*     Error - 0 if successful, or VQAERR_ error code.
*
****************************************************************************/

#define OPEN_VQHD     (1<<0)
#define OPEN_FINF     (1<<1)
#define OPEN_CAPTIONS (1<<2)
#define OPEN_EVA      (1<<3)

long VQA_Open(char const *filename, VQAConfig *_config, VQAHandle **handle)
{
	VQAHandle   *vqa;
	VQAHandleP  *vqap;
	VQAHeader   *header;
	VQAConfig   *config;
	ChunkHeader chunk;
//	long        max_frm_size;
//	long        i;
	bool        done;
//	char        *ptr;

	*handle = NULL;

	if (!_config) {
		return(VQAERR_NOCONFIG);
	}

	if (!_config->TimerCallback) {
		return(VQAERR_NOTIMER);
	}

	if (_config->RefreshRate == 0) {
		return(VQAERR_NORATE);
	}

	if (_config->MemoryHandler == NULL) {
		vqa = (VQAHandle *)VQA_Memory_Handler(NULL, VQAMEM_ALLOC, 0, sizeof(VQAHandleP));
		if (vqa != NULL) {
			VQA_Memory_Handler(vqa, VQAMEM_LOCK, vqa, sizeof(VQAHandleP));
		} else {
			return(VQAERR_NOMEM);
		}
	} else {
		vqa = (VQAHandle *)_config->MemoryHandler(NULL, VQAMEM_ALLOC, 0, sizeof(VQAHandleP));
		if (vqa != NULL) {
			_config->MemoryHandler(vqa, VQAMEM_LOCK, vqa, sizeof(VQAHandleP));
		} else {
			return(VQAERR_NOMEM);
		}

	}

	/* Dereference commonly used data members for quicker access. */
	vqap = (VQAHandleP *)vqa;

	memset(vqap, 0, sizeof(VQAHandleP));
	vqap->MemUsed = sizeof(VQAHandleP);

	/*-------------------------------------------------------------------------
	 * INITIALIZE THE PLAYERS CONFIGURATION
	 *-----------------------------------------------------------------------*/

	/* Use the clients configuration if they provided one. */
	if (_config != NULL) {
		memcpy(&vqap->Config, _config, sizeof(VQAConfig));
	} else {
		//VQA_DefaultConfig(&vqap->Config);
	}

	/* Use the internal configuration structure from now on. */
	config = &vqap->Config;

	if (config->MemoryHandler == NULL) {
		config->MemoryHandler = VQA_Memory_Handler;
	}

	if (config->StreamHandler == NULL) {
		config->StreamHandler = Disk_VQA_Stream_Handler;
	}

	header = &vqap->Header;

	/*-------------------------------------------------------------------------
	 * VERIFY VALIDITY OF VQA FILE.
	 *-----------------------------------------------------------------------*/

	/* Open the file. */
	if (config->StreamHandler(vqa, VQACMD_OPEN, (void *)filename, 0)) {
		config->MemoryHandler(vqa, VQAMEM_UNLOCK, vqa, sizeof(VQAHandleP));
		config->MemoryHandler(vqa, VQAMEM_FREE, vqa, NULL);
		return(VQAERR_OPEN);
	}

	int pos = strlen(filename) - 31;
	if (pos < 0) pos = 0;
	strcpy(vqap->Filename, filename + pos);

	/* Read the file ID & Size */
	if (config->StreamHandler(vqa, VQACMD_READ, &chunk, sizeof(chunk))) {
		VQA_Close(vqa);
		return(VQAERR_READ);
	}

	/* Verify an IFF FORM */
	if ((chunk.id != ID_FORM) || (chunk.size == 0)) {
		VQA_Close(vqa);
		return(VQAERR_NOTVQA);
	}

	/* Read in WVQA ID */
	if (config->StreamHandler(vqa, VQACMD_READ, &chunk, sizeof(chunk.id))) {
		VQA_Close(vqa);
		return(VQAERR_READ);
	}

	/* Verify VQA */
	if (chunk.id != ID_WVQA) {
		VQA_Close(vqa);
		return(VQAERR_NOTVQA);
	}

	/*-------------------------------------------------------------------------
	 * PROCESS THE PRE-FRAME CHUNKS (VQHD, CAP, FINF, ETC...)
	 *-----------------------------------------------------------------------*/
	done = 0;

	while (!done) {
		if (config->StreamHandler(vqa, VQACMD_READ, &chunk, sizeof(chunk))) {
			VQA_Close(vqa);
			return(VQAERR_READ);
		}

		chunk.size = REVERSE_LONG(chunk.size);

		switch (chunk.id) {

			/*---------------------------------------------------------------------
			 * READ IN THE VQA HEADER.
			 *-------------------------------------------------------------------*/
			case ID_VQHD:
				if (chunk.size != sizeof(VQAHeader)) {
					VQA_Close(vqa);
					return(VQAERR_NOTVQA);
				}

				/* Read the header data. */
				if (config->StreamHandler(vqa, VQACMD_READ, header, PADSIZE(chunk.size))) {
					VQA_Close(vqa);
					return(VQAERR_READ);
				}

				/*-------------------------------------------------------------------
				 * SETUP THE CONFIGURATION FROM THE HEADER.
				 *-----------------------------------------------------------------*/
				vqap->Version = header->Version;
				vqap->NumFrames = header->Frames;
				vqap->ImageWidth = header->ImageWidth;
				vqap->ImageHeight = header->ImageHeight;
				vqap->ColorMode = header->ColorMode;
				vqap->FrameRate = header->FPS;

				/* If Loaders frame rate is -1 then use the value from the header. */
				if (config->FrameRate == -1) {
					config->FrameRate = header->FPS;
				}

				/* If Drawers frame rate is -1 then use the value from the header,
				 * which will result in a "variable" frame rate.
				 */
				if (config->DrawRate == -1) {
					config->DrawRate = header->FPS;
				}

				/* Finally, if the DrawRate was set to -1 or 0 (ie MaxRate contained
				 * bogus values), set it to the header value.
				 */
				if ((config->DrawRate == -1) || (config->DrawRate == 0)) {
					config->DrawRate = header->FPS;
				}

				#if(VQAAUDIO_ON)
				/* If an alternate audio track is not available then turn it off.
				 * This enables the primary audio track to be played.
				 */
				if ((header->Version > VQAHD_VER1)
						&& !(header->Flags & VQAHDF_ALTAUDIO)) {
					config->OptionFlags &= ~VQAOPTF_ALTAUDIO;
				}
				#endif

				break;

			#if( VQACAPTIONS_ON )

			/*---------------------------------------------------------------------
			 * READ IN AND OPEN THE CAPTIONS STREAM.
			 *-------------------------------------------------------------------*/
			case ID_CAP0:
				if ((config->CapFont != NULL)
						&& (config->OptionFlags & VQAOPTF_CAPTIONS)) {

					short size = 0;

					/* Get uncompressed size of captions. */
					if (vqap->IOHandler(vqa, VQACMD_READ, &size, sizeof(short))) {
						VQA_Close(vqa);
						return(VQAERR_READ);
					}

					/* Allocate buffer for captions. */
					i = size + 50;

					if ((ptr = (char *)malloc(i)) == NULL) {
						VQA_Close(vqa);
						return(VQAERR_NOMEM);
					}

					/* Read in the captions chunk. */
					i -= PADSIZE(chunk.size);

					if (vqap->IOHandler(vqa, VQACMD_READ, (ptr + i),
							PADSIZE(chunk.size - sizeof(short)))) {

						free(ptr);
						VQA_Close(vqa);
						return(VQAERR_READ);
					}

					/* Decompress the captions. */
					LCW_Uncomp((ptr + i), ptr, size);
					vqap->Caption = OpenCaptions(ptr, config->CapFont);

					if (vqap->Caption == NULL) {
						VQA_Close(vqa);
						return(VQAERR_NOMEM);
					}

				} else {
					if (vqap->IOHandler(vqa, VQACMD_SEEK, (void *)SEEK_CUR,
							PADSIZE(chunk.size))) {
						VQA_Close(vqa);
						return(VQAERR_SEEK);
					}
				}
				break;

			case ID_EVA0:
				if ((config->EVAFont != NULL)
						&& (config->OptionFlags & VQAOPTF_EVA)) {

					short size = 0;

					/* Get uncompressed size of captions. */
					if (vqap->IOHandler(vqa, VQACMD_READ, &size, sizeof(short))) {
						VQA_Close(vqa);
						return(VQAERR_READ);
					}

					/* Allocate buffer for captions. */
					i = size + 50;

					if ((ptr = (char *)malloc(i)) == NULL) {
						VQA_Close(vqa);
						return(VQAERR_NOMEM);
					}

					/* Read in the captions chunk. */
					i -= PADSIZE(chunk.size);

					if (vqap->IOHandler(vqa, VQACMD_READ, (ptr + i),
							PADSIZE(chunk.size - sizeof(short)))) {
						free (ptr);
						VQA_Close(vqa);
						return(VQAERR_READ);
					}

					/* Decompress the captions. */
					LCW_Uncomp((ptr + i), ptr, size);
					vqap->EVA = OpenCaptions(ptr, config->EVAFont);

					if (vqap->EVA == NULL) {
						VQA_Close(vqa);
						return(VQAERR_NOMEM);
					}

				} else {
					if (vqap->IOHandler(vqa, VQACMD_SEEK, (void *)SEEK_CUR,
							PADSIZE(chunk.size))) {
						VQA_Close(vqa);
						return(VQAERR_SEEK);
					}
				}
				break;



			#endif

			case ID_CLIP:
				if (Load_CLIP(vqap, chunk.size) != VQAERR_NONE) {
					VQA_Close(vqa);
					return(VQAERR_READ);
				}
				break;

			case ID_LINF:
				if (Load_LINF(vqap) != VQAERR_NONE) {
					VQA_Close(vqa);
					return(VQAERR_READ);
				}
				break;

			case ID_MFCI:
				if (Load_MFCI(vqap) != VQAERR_NONE) {
					VQA_Close(vqa);
					return(VQAERR_READ);
				}
				break;

			case ID_CINF:
				if (Load_CINF(vqap) != VQAERR_NONE) {
					VQA_Close(vqa);
					return(VQAERR_READ);
				}
				break;

			case ID_PINF:
				if (Load_PINF(vqap) != VQAERR_NONE) {
					VQA_Close(vqa);
					return(VQAERR_READ);
				}
				break;

			/*---------------------------------------------------------------------
			 * READ FRAME INFORMATION
			 *-------------------------------------------------------------------*/
			case ID_FINF:

				/*-------------------------------------------------------------------
				 * ALLOCATE THE BUFFERS THAT WE NEED TO PLAY THE VQA.
				 *-----------------------------------------------------------------*/
				if (AllocBuffers(vqap) != VQAERR_NONE) {
					VQA_Close(vqa);
					return(VQAERR_NOMEM);
				}

				if (Load_FINF(vqap, chunk.size) != VQAERR_NONE) {
					VQA_Close(vqa);
					return(VQAERR_READ);
				}

				done = true;
				break;

			case ID_MSCI:
				if (Load_MSCI(vqap) != VQAERR_NONE) {
					VQA_Close(vqa);
					return(VQAERR_READ);
				}
				break;

			default:
				if (config->StreamHandler(vqa, VQACMD_SEEK, (void *)SEEK_CUR,
						PADSIZE(chunk.size))) {
					VQA_Close(vqa);
					return(VQAERR_SEEK);
				}
				break;
		}
	}

	/*-------------------------------------------------------------------------
	 * INITIALIZE THE VIDEO SYSTEM IF WE ARE REQUIRED TO HANDLE THAT.
	 *-----------------------------------------------------------------------*/
	if (header->ColorMode == 0 && vqap->PaletteInfo.Header.Count == 0) {
		vqap->PaletteInfo.Header.Count = (unsigned short)VQA_NumFramesWithPalettes(vqap);
	}


	//vqap->VBIBit = config->VBIBit;
	VQA_Reset((VQAHandle *)vqap);

	/* If the movie does not contain an audio track make sure we won't try
	 * to play one.
	 */
	if (((header->Flags & VQAHDF_AUDIO) == 0)) {
		config->OptionFlags &= (~VQAOPTF_AUDIO);
	}

	/*-------------------------------------------------------------------------
	 * INITIALIZE THE AUDIO PLAYBACK/TIMING SYSTEM.
	 *-----------------------------------------------------------------------*/
	#if(VQAAUDIO_ON)
	if (config->OptionFlags & VQAOPTF_AUDIO) {
		VQAAudio *audio;

		if (config->AudioHandler == NULL) {
			VQA_Close(vqa);
			return(VQAERR_NOAHANDL);
		}

		/* Dereference for quick access. */
		audio = &vqap->Audio;

		if (config->OptionFlags & VQAOPTF_ALTAUDIO) {
			vqap->SampleRate = header->AltSampleRate;
			vqap->Channels = header->AltChannels;
			vqap->BitsPerSample = header->AltBitsPerSample;
		} else {
			vqap->SampleRate = header->SampleRate;
			vqap->Channels = header->Channels;
			vqap->BitsPerSample = header->BitsPerSample;
		}

		/* Open HMI audio resource for playback. */
		if (VQA_OpenAudio(vqap) != VQAERR_NONE) {
			VQA_Close(vqa);
			return(VQAERR_AUDIO);
		}

		Lock_Audio_Handler();
		audio->Flags |= VQAAUDF_MEMLOCKED;

		/* Initialize ADPCM information structure for audio stream. */
		VQA_sosCODECInitStream(&audio->ADPCM_Info);


	} else {
		if (config->AudioHandler != NULL) {
			vqap->Config.AudioHandler(vqa, VQAAUDIO_INIT, NULL, NULL);
		}
	}
	#endif /* VQAAUDIO_ON */

	if (config->OptionFlags & VQAOPTF_LOOPS) {

		if ((config->OptionFlags & VQAOPTF_LOOPCACHE)) {
			VQALoopInfo *info = &vqap->LoopInfo;
			if (info->Header.Count > 0) {
				long cached;
				long size;
				if (config->StreamHandler(vqa, VQACMD_SIZE, &size, 0) != 0) {
					size = 0;
				}

				VQALoopCache *cache = &vqap->LoopCache;

				cache->Size = VQA_LargestLoop(vqap, size);
				config->MemoryHandler(vqa, VQAMEM_QUERYSIZE, &cached, 0);
				if (cached < 0) {
					if (size > 0) {
						cached = size;
					} else {
						cached = 0;
					}
				}
				if (cache->Size > cached) {
					cache->Size = cached;
				}

				if (cache->Size > 0) {
					void *ptr = (void *)config->MemoryHandler(vqa, VQAMEM_ALLOC, NULL, vqap->LoopCache.Size);
					cache->Ptr = (char *)ptr;
					if (ptr == NULL) {
						VQA_Close(vqa);
						return(VQAERR_NOMEM);
					}
					config->MemoryHandler(vqa, VQAMEM_LOCK, vqap->LoopCache.Ptr, vqap->LoopCache.Size);
					vqap->AltBufferFlags |= VQAABUFF_ALTLOOP;
					vqap->MemUsed += vqap->LoopCache.Size;
					VQA_ResetCache(vqap);
				}
			}
		}

		if (config->InitialLoopID >= 0) {
		#ifndef __WATCOMC__
			#pragma inline_depth(0)
		#endif
			if (VQA_SetLoop(vqa, config->InitialLoopID, config->InitialLoopIterations, 2) != VQAERR_NONE) {
				VQA_Close(vqa);
				return(VQAERR_SETLOOP);
			}
		#ifndef __WATCOMC__
			#pragma inline_depth()
		#endif
		} else {
			if (VQA_SetLoop_Internal(vqa, 0, vqap->StopFrame, 0, 0) != VQAERR_NONE) {
				VQA_Close(vqa);
				return(VQAERR_SETLOOP);
			}
		}


	} else {
		vqap->LoopEndFrameMode2 = vqap->StopFrame;
	}

	/* Init the Drawer's configuration */
	if (VQA_Configure_Drawer(vqap) != VQAERR_NONE) {
		return(VQAERR_OPEN);
	}

	/*-------------------------------------------------------------------------
	 * PRIME THE BUFFERS BY PRE-LOADING THEM WITH FRAME DATA.
	 *-----------------------------------------------------------------------*/
	if (!(config->OptionFlags & VQAOPTF_LOOPS) || config->InitialLoopID < 0) {
		if (PrimeBuffers(vqa) != VQAERR_NONE) {
			VQA_Close(vqa);
			return(VQAERR_READ);
		}
	}

	*handle = vqa;

	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     VQA_Close - Close an opened VQA file.
*
* SYNOPSIS
*     VQA_Close(VQA)
*
*     void VQA_Close(VQAHandle *);
*
* FUNCTION
*     Close the file that was opened with VQA_Open().
*
* INPUTS
*     VQA - Pointer VQAHandle to close.
*
* RESULT
*     NONE
*
****************************************************************************/

void VQA_Close(VQAHandle *vqa)
{
	VQAHandleP *vqap;
	VQAConfig *config;
	VQAAudio *audio;

	vqap = (VQAHandleP *)vqa;
	config = &vqap->Config;
	audio = &vqap->Audio;

	/* Shutdown audio/timing system. */
	#if(VQAAUDIO_ON)

	if (config->OptionFlags & VQAOPTF_AUDIO && vqap->Header.Flags & (VQAHDF_AUDIO|VQAHDF_ALTAUDIO)) {
		VQA_CloseAudio((VQAHandleP *)vqap);
		if (audio->Flags & VQAAUDF_MEMLOCKED) {
			Unlock_Audio_Handler();
			audio->Flags &= ~VQAAUDF_MEMLOCKED;
		}
	}
	#endif /* VQAAUDIO_ON */

	if (vqap->Flags & VQADATF_ALTIMG && vqap->AltImageBuf != NULL) {
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, vqap->AltImageBuf, vqap->AltImageWidth * vqap->AltImageHeight);
		config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, vqap->AltImageBuf, NULL);
	}

	FreeBuffers(vqap);
	config->StreamHandler((VQAHandle *)vqap, VQACMD_CLOSE, NULL, NULL);
	config->MemoryHandler((VQAHandle *)vqap, VQAMEM_UNLOCK, (VQAHandleP *)vqap, sizeof(VQAHandleP));
	config->MemoryHandler((VQAHandle *)vqap, VQAMEM_FREE, (VQAHandleP *)vqap, NULL);
}


/****************************************************************************
*
* NAME
*     VQA_Play - Play the VQA movie.
*
* SYNOPSIS
*     Error = VQA_Play(VQA, Mode)
*
*     long VQA_Play(VQAHandle *, long);
*
* FUNCTION
*     Playback the movie associated with the specified VQAHandle.
*
* INPUTS
*     VQA  - Pointer to handle of movie to play.
*     Mode - Playback mode.
*              VQAMODE_RUN   - Run the movie until completion.
*              VQAMODE_WALK  - Walk the movie frame by frame.
*              VQAMODE_PAUSE - Pause the movie.
*              VQAMODE_STOP  - Stop the movie (Shutdown).
*
* RESULT
*     Error - 0 if successful, or error code.
*
****************************************************************************/

long VQA_Play(VQAHandle *vqa, long mode, int flags)
{
	VQAHandleP *vqabuf;
	VQAConfig *config;
	VQADrawer *drawer;
	VQAAudio  *audio;
	long      rc;
	long      curtime;
	#if(VQASTANDALONE)
	long      key;
	#endif

	rc = VQAERR_NONE;

	/* Dereference commonly used data members for quick access. */
	vqabuf = ((VQAHandleP *)vqa);
	drawer = &vqabuf->Drawer;
	config = &vqabuf->Config;

	audio = &vqabuf->Audio;

	/* One time player priming. */
	if (!(vqabuf->Flags & VQADATF_PRIMED)) {
		rc = VQA_ResetLastFrameNum(vqa);
		if (rc != VQAERR_NONE) {
			return(rc);
		}

		/* Init the Drawer's configuration */
		if (!(vqabuf->Flags & VQADATF_BUFCONFIG)) {
			VQA_Configure_Buffer((VQAHandleP *)vqa);
		}

		/* If audio enabled & loaded, start playing */
		#if(VQAAUDIO_ON)
		if ((config->OptionFlags & VQAOPTF_AUDIO) && vqabuf->Audio.IsLoaded[0]) {
			VQA_StartAudio((VQAHandleP *)vqabuf);
		}
		#endif

		/* Initialize the timer */
		curtime = ((drawer->CurFrame->FrameNum * VQA_TIMETICKS)
				/ config->DrawRate);

		VQA_SetTimer((VQAHandleP *)vqabuf, curtime);
		((VQAHandleP *)vqa)->StartTime = VQA_GetTime((VQAHandleP *)vqabuf);

		/* Set up the Mono screen */
		#if(VQAMONO_ON)
		if (config->OptionFlags & VQAOPTF_MONO) {
			VQA_InitMono((VQAHandleP *)vqabuf);
		}
		#endif

		/* Priming is complete. */
		vqabuf->Flags |= VQADATF_PRIMED;
	}

	int count;
	if (config->UnusedCallback != NULL) {
		count = config->UnusedCallback(vqa);
	} else {
		count = -1;
	}

	int lastcount = vqabuf->LastCallbackCount;
	if (lastcount == -1) {
		vqabuf->LastCallbackCount = count;
		vqabuf->MaxCallbackDelta = 0;
	} else {
		int delta = count - lastcount;
		vqabuf->LastCallbackCount = count;
		int maxdelta = vqabuf->MaxCallbackDelta;
		if (delta > maxdelta) {
			vqabuf->MaxCallbackDelta = delta;
		} else {
			vqabuf->MaxCallbackDelta = maxdelta;
		}
	}



	/* Main Player Loop */
	switch (mode) {
		case VQAMODE_PAUSE:
			if ((vqabuf->Flags & VQADATF_PAUSED) == 0) {
				vqabuf->Flags |= VQADATF_PAUSED;
				((VQAHandleP *)vqa)->EndTime = VQA_GetTime((VQAHandleP *)vqabuf);

				/* Stop the audio while the movie is paused. */
				#if(VQAAUDIO_ON)
				if (vqabuf->Audio.Flags & VQAAUDF_ISPLAYING) {
					VQA_PauseAudio((VQAHandleP *)vqabuf);
				}
				#endif

#ifdef _DEBUG // Observed that these were added in TS Patch 1.17, but we will gate them behind _DEBUG to be safe.
				DebugString("VQA_Play() - Pausing @ %ld\n", vqabuf->EndTime);
#endif
			}

			if ((flags & VQAPLAYF_REPAINT)) {
				drawer->Flags |= VQADRWF_REPAINT;
				drawer->Flags |= VQADRWF_FORCEDRAW;
				if ((rc = (*(vqabuf->Draw_Frame))(vqa)) == VQAERR_NONE) {
					if ((rc = User_Update(vqa)) != VQAERR_NONE) {
						vqabuf->Flags |= (VQADATF_DDONE|VQADATF_LDONE);
					} else {
						rc = drawer->LastFrameNum;
					}
				} else {
					if (rc == VQAERR_NOBUFFER && (vqabuf->Flags & VQADATF_LDONE)) {
						vqabuf->Flags |= (VQADATF_DDONE);
					}
				}
				drawer->Flags &= ~VQADRWF_STEP;
				drawer->Flags &= ~VQADRWF_REPAINT;
				drawer->Flags &= ~VQADRWF_FORCEDRAW;
			} else {
				rc = VQAERR_PAUSED;
			}

			break;

		case VQAMODE_RUN:
		case VQAMODE_WALK:
		default:

			/* Start up the movie if is it currently paused. */
			if (vqabuf->Flags & VQADATF_PAUSED) {
				vqabuf->Flags &= ~VQADATF_PAUSED;

				#if(VQAAUDIO_ON)
				/* Start the audio if it was previously on. */
				if (config->OptionFlags & VQAOPTF_AUDIO) {
					VQA_StartAudio((VQAHandleP *)vqabuf);
				}
				#endif

				VQA_SetTimer((VQAHandleP *)vqabuf, ((VQAHandleP *)vqa)->EndTime);

#ifdef _DEBUG // Observed that these were added in TS Patch 1.17, but we will gate them behind _DEBUG to be safe.
				DebugString("VQA_Play() - Restarting @ %ld\n", VQA_GetTime((VQAHandleP *)vqabuf));
#endif
			}

			/* Load, Draw, Load, Draw, Load, Draw ... */
			while ((vqabuf->Flags & (VQADATF_DDONE|VQADATF_LDONE))
					!= (VQADATF_DDONE|VQADATF_LDONE)) {

				/* Load a frame */
				if (!(vqabuf->Flags & VQADATF_LDONE)) {
					if ((rc = VQA_LoadFrame(vqabuf, 0)) == VQAERR_NONE) {
						((VQAHandleP *)vqa)->LoadedFrames++;
					}
					else {
						if ((rc != VQAERR_NOBUFFER) && (rc != VQAERR_SLEEPING)) {
							vqabuf->Flags |= VQADATF_LDONE;
							//rc = 0;
						}
					}

					if (config->OptionFlags & VQAOPTF_WAITFILL) {
						if (rc == VQAERR_NOBUFFER || (vqabuf->Flags & VQADATF_LDONE)) {
								if (vqabuf->Flags & VQADATF_AUDIOSYNC) {
#ifdef _DEBUG // Observed that these were added in TS Patch 1.17, but we will gate them behind _DEBUG to be safe.
									DebugString("VQA_Play() - Drawer waitfill\n");
#endif
									if (!(vqabuf->Flags & VQADATF_FRAMESTALL)) {
										vqabuf->Flags |= VQADATF_REFILLED;
									if (config->OptionFlags & VQAOPTF_AUDIO) {
#ifdef _DEBUG // Observed that these were added in TS Patch 1.17, but we will gate them behind _DEBUG to be safe.
										DebugString("VQA_Play() - Waitfill StartAudio\n");
#endif
										VQA_StartAudio(vqabuf);
									}
								}
							}
						} else if (!(vqabuf->Flags & VQADATF_LDONE)) {
							if ((vqabuf->Audio.Flags & VQAAUDF_ISSTARVED)) {
#ifdef _DEBUG // Observed that these were added in TS Patch 1.17, but we will gate them behind _DEBUG to be safe.
									DebugString("VQA_Play() - Loader waitfill\n");
#endif
								if ((vqabuf->Audio.Flags & VQAAUDF_ISPLAYING)) {
#ifdef _DEBUG // Observed that these were added in TS Patch 1.17, but we will gate them behind _DEBUG to be safe.
										DebugString("VQA_Play() - Waitfill PauseAudio\n");
#endif
									VQA_PauseAudio(vqabuf);
								}
								vqabuf->Flags |= VQADATF_AUDIOSYNC;
								audio->Flags &= ~VQAAUDF_ISSTARVED;

							}
 						}
					}
				}else{
					//VQAMovieDone++;
				}

				/* Draw a frame */
				if ((config->DrawFlags & VQACFGF_NODRAW) == 0) {




					if ((vqabuf->Config.OptionFlags & VQAOPTF_WAITFILL)) {
						if ((vqabuf->Flags & VQADATF_AUDIOSYNC)) {
							if ((vqabuf->Flags & VQADATF_REFILLED)) {
								vqabuf->Flags &= ~VQADATF_AUDIOSYNC;
								vqabuf->Flags &= ~VQADATF_REFILLED;
							} else if ((vqabuf->Flags & VQADATF_FRAMESTALL) == 0) {
								rc = VQAERR_NOT_TIME;
								break;
							}
						}
					}


					if ((flags & VQAPLAYF_STEP)) {
						drawer->Flags |= VQADRWF_STEP;
					} else if ((flags & VQAPLAYF_REPAINT)) {
						drawer->Flags |= VQADRWF_REPAINT;
					}

					if ((rc = ((VQAHandleP *)vqabuf)->Draw_Frame((VQAHandle *)vqabuf)) == VQAERR_NONE) {
						if (!(drawer->Flags & VQADRWF_HOLD)) {
							((VQAHandleP *)vqa)->DrawnFrames++;
						}
						//rc = drawer->LastFrameNum;

						#ifdef WIN32never
						curframe = drawer->CurFrame;
						pal = curframe->Palette;
						palsize = curframe->PaletteSize;
						slowpal = (config->OptionFlags & VQAOPTF_SLOWPAL) ? 1 : 0;
						if ((curframe->Flags & VQAFRMF_PALETTE)
								|| (drawer->Flags & VQADRWF_SETPAL)) {
							setpalette = TRUE;
						}else{
							setpalette = FALSE;
						}
						#endif

						if ((rc = User_Update((VQAHandle *)vqabuf)) != VQAERR_NONE) {
							vqabuf->Flags |= (VQADATF_DDONE|VQADATF_LDONE);
						} else {
							rc = drawer->LastFrameNum;
						}
						#ifdef WIN32never
						/*
						** Set the palette if neccessary
						*/
						if (setpalette) {
							SetPalette(pal, palsize, slowpal);
							curframe->Flags &= ~VQAFRMF_PALETTE;
							drawer->Flags &= ~VQADRWF_SETPAL;
 						}
						#endif	//WIN32

					}else {
//						if (rc==VQAERR_EOF) break;
						if ((rc == VQAERR_NOBUFFER) && (vqabuf->Flags & VQADATF_LDONE)) {
							vqabuf->Flags |= VQADATF_DDONE;
						}
					}
					drawer->Flags &= ~VQADRWF_STEP;
					drawer->Flags &= ~VQADRWF_REPAINT;
				} else {
					vqabuf->Flags |= VQADATF_DDONE;
					drawer->CurFrame->Flags = 0L;
					drawer->CurFrame = drawer->CurFrame->Next;
				}

				/* Update Mono output */
				#if(VQAMONO_ON)
				if (config->OptionFlags & VQAOPTF_MONO) {
					VQA_UpdateMono((VQAHandleP *)vqabuf);
				}
				#endif

				if (mode == VQAMODE_WALK) {
					break;
				}
				#if(VQASTANDALONE)
				else {

					/* Do single-stepping check. */
					if (config->OptionFlags & VQAOPTF_STEP) {
						while ((key = Check_Key()) == 0);
						Get_Key();

						/* Escape key still quits. */
						if (key == 27) {
							break;
						}
					}

					/* Check for ESC */
					if ((key = Check_Key()) != 0) {
						mode = VQAMODE_STOP;
						break;
					}
				}
				#endif
			}
			break;
	}

	/* If the movie is finished or we are requested to stop then shutdown. */
	if (((vqabuf->Flags & (VQADATF_DDONE|VQADATF_LDONE))
			== (VQADATF_DDONE|VQADATF_LDONE)) || (mode == VQAMODE_STOP)) {

		/* Record the end time; must be done before stopping audio, since we're
		 * getting the elapsed time from the audio DMA position.
		 */
		((VQAHandleP *)vqa)->EndTime = VQA_GetTime((VQAHandleP *)vqabuf);

		/* Stop audio, if it's playing. */
		#if(VQAAUDIO_ON)
		if (vqabuf->Audio.Flags & VQAAUDF_ISPLAYING) {
			VQA_StopAudio((VQAHandleP *)vqabuf);
		}
		#endif

		/* Movie is finished. */
		rc = VQAERR_EOF;
	}

	return(rc);
}


/****************************************************************************
*
* NAME
*     VQA_SeekFrame - Position the movie stream to the specified frame.
*
* SYNOPSIS
*     Frame = VQA_SeekFrame(VQA, Frame, FromWhere)
*
*     long VQA_SeekFrame(VQAHandle *, long, long);
*
* FUNCTION
*     This function sets the movie stream to the new frame specified by
*     the 'offset' parameter. 'FromWhere' is a symbolic constant that is used
*     to specify from where in the stream offset should be applied.
*
* INPUTS
*     VQA       - Pointer to VQAHandle of movie to seek into.
*     Frame     - Frame to seek to.
*     FromWhere - Relative position indicator.
*
* RESULT
*     Frame - New frame position or -1 if error.
*
****************************************************************************/
long VQA_SeekFrame(VQAHandle *vqa, long framenum, long fromwhere)
{
	VQAHandleP   *vqap;
	VQALoader    *loader;
	VQAHeader    *header;
	VQAFrameNode *frame;
	VQAConfig    *config;
	long         rc = VQAERR_NONE;

	#if(VQAAUDIO_ON)
	VQAAudio     *audio;
	long         audio_on;
 	#endif

	/* Dereference commonly used data members for quick access. */
	vqap = (VQAHandleP *)vqa;
	loader = &vqap->Loader;
	header = &vqap->Header;
	config = &vqap->Config;

	fromwhere = fromwhere;

	/* Get the current frame. */
	frame = loader->CurFrame;

	#if(VQAAUDIO_ON)
	audio = &vqap->Audio;

	/* Stop audio playback. */
	audio_on = (audio->Flags & VQAAUDF_ISPLAYING);
	if (audio_on) {
		VQA_StopAudio(vqap);
	}
	#endif

	/* Make sure the requested frame is valid and the frame information
	 * array is allocated before continuing.
	 */
	if ((framenum <= vqap->StopFrame) && (vqap->Foff != NULL)) {

		VQA_ResetCache(vqap);
		VQA_ResetLastFrameNum(vqa);

		bool has_palette = false;
		if (header->ColorMode == 0 && !VQA_IsFrameStartOfLoop(vqap, framenum)) {
			rc = VQA_ReloadPalette(vqap, framenum, true);
			if (rc == VQAERR_NONE) {
				if (frame->Flags & VQAFRMF_PALETTE) {
					has_palette = true;
				}
			}
		}

		/* Build the codebook for the frame we are seeking to. */
		if (rc == VQAERR_NONE) {
			loader->CurFrameNum = framenum;

			bool skipcodebook;
			bool preloadaudio = (((unsigned char)header->Flags) & VQAHDF_SNDJUMP) == 0;

			rc = VQA_SeekGroup(vqap, framenum, header->Groupsize, preloadaudio, 1, skipcodebook);

			/* If everything is okay, then re-prime the buffers. */
			if (rc == VQAERR_NONE) {

				loader->CurFrameNum = framenum;

				/* Mark all the frames except the current one as empty. */
				loader->CurFrame->Flags = 0;

				if (has_palette) {
					loader->CurFrame->Flags |= VQAFRMF_PALETTE;
				}

				frame = loader->CurFrame->Next;

				while (frame != loader->CurFrame) {
					frame->Flags = 0;
					frame = frame->Next;
				}

				/* Set the drawer to the current frame and the loader
				 * to the next.
				 */
				vqap->Drawer.CurFrame = loader->CurFrame;

				/* Prime the buffers for the new position. */
				rc = PrimeBuffers(vqa);

				/* An end of file is not considered and error. */
				if ((rc == VQAERR_NONE) || (rc == VQAERR_EOF)) {
					rc = framenum;
					VQA_SetTimer(vqap, VQA_TIMETICKS * framenum / config->DrawRate);
				}
			}
		}
		return(rc);
	} else {
		rc = VQAERR_SEEK;
	}
	return(rc);
}


long VQA_SetUnVQ(VQAHandle *vqa, UNVQ_FUNC unvq1, UNVQ_FUNC unvq2)
{
	VQAHandleP *vqap = (VQAHandleP *)vqa;

	if (unvq1 != NULL) {
		vqap->UnVQ1 = unvq1;
	} else {
		vqap->UnVQ1 = UnVQ_Nop;
	}
	if (unvq2 != NULL) {
		vqap->UnVQ2 = unvq2;
	} else {
		vqap->UnVQ2 = UnVQ_Nop;
	}

	return(VQAERR_NONE);
}


/****************************************************************************
*
* NAME
*     VQA_Set_DrawBuffer - Set the buffer to draw the images to.
*
* SYNOPSIS
*     VQA_Set_DrawBuffer(VQA, Buffer, Width, Height, XPos, YPos)
*
*     void VQA_Set_DrawBuffer(VQAHandle *, unsigned char *,
*                             unsigned long, unsigned long, unsigned long,
*                             unsigned long);
*
* FUNCTION
*     Set the draw buffer to the buffer provided by the client.
*
* INPUTS
*     VQA    - Pointer to VQAHandle to set buffer for.
*     Buffer - Pointer to new image buffer.
*     Width  - Width of the buffer in pixels.
*     Height - Height of the buffer in pixels.
*     XPos   - X pixel position in buffer to draw image.
*     YPos   - Y pixel position in buffer to draw image.
*
* RESULT
*     NONE
*
****************************************************************************/

long VQA_Set_DrawBuffer(VQAHandle *vqa, unsigned char *buffer, unsigned long width, unsigned long height, long xpos, long ypos)
{
	long origin;
	VQAHeader *header;
	VQADrawer *drawer;
	VQAConfig *config;

	header = &((VQAHandleP *)vqa)->Header;
	drawer = &((VQAHandleP *)vqa)->Drawer;
	config = &((VQAHandleP *)vqa)->Config;

	origin = (config->DrawFlags & VQACFGF_ORIGIN);

	if (width == 0) {
		width = header->ImageWidth;
	}

	if (height == 0) {
		height = header->ImageHeight;
	}


	if (xpos < 0 && ypos < 0) {
		if (config->DrawFlags & VQACFGF_OFFSET) {
			if (config->DrawFlags & VQACFGF_HALFSIZE) {
				drawer->X1 = (header->Xpos / 2);
				drawer->Y1 = (header->Ypos / 2);
			} else {
				drawer->X1 = header->Xpos;
				drawer->Y1 = header->Ypos;
			}
		} else {
			if (config->DrawFlags & VQACFGF_HALFSIZE) {
				drawer->X1 = (width - (header->ImageWidth / 2)) / 2;
				drawer->Y1 = (height - (header->ImageHeight / 2)) / 2;
			} else {
				drawer->X1 = (width - header->ImageWidth) / 2;
				drawer->Y1 = (height - header->ImageHeight) / 2;
			}
		}
	} else {
		if (config->DrawFlags & VQACFGF_HALFSIZE) {
			xpos /= 2;
			ypos /= 2;
		}

		switch (origin) {
			default:
			case VQACFGF_TOPLEFT:
				drawer->X1 = xpos;
				drawer->Y1 = ypos;
				break;

			case VQACFGF_BOTLEFT:
				drawer->X1 = xpos;
				drawer->Y1 = (height - ypos);
				break;

			case VQACFGF_BOTRIGHT:
				drawer->X1 = (width - xpos);
				drawer->Y1 = (height - ypos);
				break;

			//case VQACFGF_TOPRIGHT: // unimplemented
			//	break;
		}
	}

	if (drawer->X1 >= 0 && drawer->Y1 >= 0 && (unsigned)drawer->X1 < width && (unsigned)drawer->Y1 < height) {
		long x = drawer->X1;
		long y = drawer->Y1;

		if (header->ColorMode == 1 || header->ColorMode == 4) {
			x *= 2;
			y *= 2;
		}

		drawer->ImageWidth = width;
		drawer->ImageHeight = height;
		drawer->ScreenOffset = ((width * y) + x);

		if (buffer != NULL) {
			drawer->ImageBuf = buffer;
			((VQAHandleP *)vqa)->ImageBuf = buffer;
		}

		((VQAHandleP *)vqa)->Flags |= VQADATF_BUFCONFIG;
		return(VQAERR_NONE);
	}

	return(VQAERR_SETBUFFR);
}


long VQA_SetLoop(VQAHandle *vqa, int id, int iterations, int mode)
{
	VQAHandleP *vqap = (VQAHandleP *)vqa;
	VQAConfig *config = &vqap->Config;
	VQALoopInfo * info = &vqap->LoopInfo;

	if (id >= vqap->LoopInfo.Header.Count || id < 0) {
		return VQAERR_SETLOOP;
	}

	if ((config->OptionFlags & VQAOPTF_AUDIO) && vqap->LoopInfo.Header.Count == 0) {
		return(VQAERR_SETLOOP);
	}

	VQALoopInfo::DATA *data = &vqap->LoopInfo.Data[id];
	int start = data->StartFrame;
	int end = data->EndFrame;
	vqap->LoopID = id;
	long rc = VQA_SetLoop_Internal(vqa, start, end, iterations, mode);
	if (rc == VQAERR_NONE) {
		vqap->LoopID = id;
	}
	return(rc);
}


/// <summary>
/// Establishes the loop the player is to repeat.
/// This is the lower layer beneath VQA_SetLoop, and is also what installs the
/// default loop spanning the whole movie. Where the player is already inside a
/// loop, the new bounds are staged and do not take effect until the current loop
/// is crossed, so that the change is not visible mid-cycle. A VQALOOP_2 loop is
/// entered at once by seeking to its start frame.
/// </summary>
/// <param name="start">First frame of the loop.</param>
/// <param name="end">Last frame of the loop.</param>
/// <param name="iterations">Number of times to repeat the loop. A negative value loops forever.</param>
/// <param name="mode">One of the VQALOOP_ modes.</param>
/// <returns>Returns with VQAERR_NONE, or VQAERR_SETLOOP if the loop could not be set.</returns>
long VQA_SetLoop_Internal(VQAHandle *vqa, int start, int end, int iterations, int mode)
{
	VQAHandleP *vqap = (VQAHandleP *)vqa;

	long rc = VQAERR_NONE;

	if (start < vqap->NumFrames && end < vqap->NumFrames && start < end && mode >= VQALOOP_NORMAL && mode < VQALOOP_3) {

		if (iterations < 0) {
			iterations = INFINITE_LOOP;
		}

		if (vqap->LoopIterations == 0 && mode == VQALOOP_JUMP) {
			mode = VQALOOP_2;
		}
		if (mode != VQALOOP_2) {
			if (vqap->Flags & VQADATF_LOOPED) {
				if (mode == VQALOOP_NORMAL) {
					vqap->LoopStartFrame1 = start;
					vqap->LoopEndFrameNormal = end;
					end = vqap->LoopEndFrameMode2;
				} else {
					vqap->LoopStartFrame1 = start;
					start = vqap->LoopStartFrame0;
					vqap->LoopEndFrameNormal = vqap->LoopEndFrameMode2;
				}
			} else {
				vqap->LoopStartFrame2 = start;
				vqap->LoopEndFrame2 = end;
			}
		}
		vqap->LoopStartFrame0 = start;
		if (mode == VQALOOP_JUMP) {
			vqap->LoopIterationsJump = iterations;
			vqap->LoopEndFrameJump = end;
		} else {
			vqap->LoopIterations = iterations;
			if (mode == VQALOOP_2) {
				vqap->LoopEndFrameMode2 = end;
				vqap->StopFrame = end;
				if (VQA_SeekFrame((VQAHandle *)vqa, start, 0) >= VQAERR_OK) {
					rc = VQAERR_NONE;
				} else {
					rc = VQAERR_SETLOOP;
				}
			}
		}
	} else {
		rc = VQAERR_SETLOOP;
	}

	return(rc);
}


int VQA_GetLoopCount(VQAHandleP *vqap)
{
	return(vqap->LoopInfo.Header.Count);
}


/****************************************************************************
*
* NAME
*     VQA_Reset - Reset the VQAHandle.
*
* SYNOPSIS
*     VQA_Reset(VQA)
*
*     void VQA_Reset(VQAHandle *);
*
* FUNCTION
*
* INPUTS
*     VQA - VQAHandle to reset.
*
* RESULT
*     NONE
*
****************************************************************************/

void VQA_Reset(VQAHandle *vqa)
{
	VQAHandleP *vqap;
	VQAAudio *audio;
	VQAHeader *header;

	/* Dereference data members for quick access */
	vqap = (VQAHandleP *)vqa;
	audio = &vqap->Audio;
	header = &vqap->Header;


	int stop;
	int frames = header->Frames;

	vqap->LoadedFrames = 0;
	vqap->DrawnFrames = 0;
	stop = frames - 1;
	vqap->StopFrame = stop;
	vqap->LoopEndFrameMode2 = stop;
	vqap->LoopEndFrame2 = stop;

	unsigned long audio_flags = audio->Flags & ~(VQAAUDF_ISREPEATING|VQAAUDF_ISENDOFFILE|VQAAUDF_ISDONE|VQAAUDF_ISSTARVED);//~1920;

	vqap->StartTime = 0;
	audio->Flags = audio_flags;
	vqap->EndTime = 0;
	vqap->Flags = 0;
	vqap->LoopIterations = 0;
	vqap->LoopID = -1;
	vqap->LoopStartFrame0 = -1;
	vqap->LoopEndFrameJump = -1;

	vqap->LoopIterationsJump = -1;

	vqap->LoopStartFrame1 = -1;
	vqap->LoopEndFrameNormal = -1;
	vqap->LoopStartFrame2 = -1;

	vqap->LastCallbackCount = -1;
	vqap->MaxCallbackDelta = 0;

	audio->Block1 = 0;
	audio->Block2 = -1;
	audio->BufferPosition = 0;

	vqap->RepeatedBuffers = 0;

	VQA_ResetCache(vqap);

}


void VQA_ResetCache(VQAHandleP *vqap)
{
	vqap->LoopCache.ID = -1;
	vqap->LoopCache.Min = -1;
	vqap->LoopCache.Max = -1;
}



/****************************************************************************
*
* NAME
*     VQA_SetStop - Set the frame the player should stop on.
*
* SYNOPSIS
*     OldStop = VQA_SetStop(VQA, Frame)
*
*     long = VQA_SetStop(VQAHandle *, long);
*
* FUNCTION
*     Set the frame that the player should stop on. This function will only
*     work on movies that are already open.
*
* INPUTS
*     VQA   - VQAHandle of movie to set the stop frame for.
*     Frame - Frame number to stop on.
*
* RESULT
*     OldStop - Previous stop frame. (-1 = invalid stop frame)
*
****************************************************************************/

long VQA_SetStop(VQAHandle *vqa, long stop)
{
	VQAHandleP *vqap = (VQAHandleP *)vqa;
	long      oldstop = -1;
	VQAHeader *header;

	/* Get a local pointer to the header. */
	header = &vqap->Header;

	if ((stop > 0) && (header->Frames >= stop)) {
		oldstop = vqap->StopFrame;
		vqap->StopFrame = (unsigned short)stop;
	}

	return(oldstop);
}


/****************************************************************************
*
* NAME
*     VQA_Version - Get VQA library version number.
*
* SYNOPSIS
*     Version = VQA_Version()
*
*     char *VQA_Version(void);
*
* FUNCTION
*     Return the version of the VQA player library.
*
* INPUTS
*     NONE
*
* RESULT
*     Version - Pointer to version number string.
*
****************************************************************************/

char const *VQA_Version(void)
{
	return(VQA_VERSION);
}


long VQA_GetClipping(VQAHandleP *vqap, int & clipw, long & cliph)
{
	if (vqap->Clipper.Width > 0) {
		clipw = vqap->Clipper.Width;
		cliph = vqap->Clipper.Height;
	} else {
		clipw = vqap->ImageWidth;
		cliph = vqap->ImageHeight;
	}
	return(VQAERR_NONE);
}


long VQA_GetBlockInfo(VQAHandle *vqa, long & blockw, long & blockh, long & clrmode)
{
	VQAHandleP *vqap = (VQAHandleP *)vqa;
	VQAHeader *header;

	header = &vqap->Header;

	blockw = header->BlockWidth;
	blockh = header->BlockHeight;
	clrmode = header->ColorMode;
	return(VQAERR_NONE);
}

#if MSC_VER
/// hack to force MSVC to keep this data
#pragma comment(linker, "/include:?VerTag@@3PADA")
#pragma comment(linker, "/include:?ReqTag@@3PADA")
#endif

/****************************************************************************
*
* NAME
*     VQA_IDString - Get the VQA player library's ID string.
*
* SYNOPSIS
*     IDString = VQA_IDString()
*
*     char *VQA_IDString(void);
*
* FUNCTION
*     Return the ID string of this VQA player library.
*
* INPUTS
*     NONE
*
* RESULT
*     IDString - Pointer to ID string.
*
****************************************************************************/

char const *VQA_IDString(void)
{
	return(VQA_IDSTRING);
}


/****************************************************************************
*
* NAME
*     User_Update - Page flip routine called by the task interrupt.
*
* SYNOPSIS
*     User_Update(VQA)
*
*     long User_Update(VQAHandle *);
*
* FUNCTION
*
* INPUTS
*     VQA - Handle of VQA movie.
*
* RESULT
*     NONE
*
****************************************************************************/

long User_Update(VQAHandle *vqa)
{
	VQAHandleP *vqap;
	VQAFrameNode *curframe;
	VQADrawer *drawer;
	VQAConfig *config;
	long    rc = 0;

	/* Dereference data members for quicker access. */
	vqap = (VQAHandleP *)vqa;
	drawer = &vqap->Drawer;
	config = &vqap->Config;
	curframe = vqap->Flipper.CurFrame;

	if (config->EventHandler != NULL) {
		if (curframe->Flags & VQAFRMF_LOOPED) {
			config->EventHandler((VQAHandle *)vqap, VQAEVENT_LOOPED, (void *)curframe->FrameNum, vqap->LoopID);
			curframe->Flags &= ~VQAFRMF_LOOPED;
		}
		if (curframe->Flags & VQAFRMF_LOOPJMP) {
			config->EventHandler((VQAHandle *)vqap, VQAEVENT_LOOPJUMP, (void *)curframe->FrameNum, vqap->LoopID);
			curframe->Flags &= ~VQAFRMF_LOOPJMP;
		}
		if (curframe->Flags & VQAFRMF_CHUNKS) {
			VQA_DispatchFrameChunks((VQAHandleP *)vqap, curframe->FrameNum);
			curframe->Flags &= ~VQAFRMF_CHUNKS;
		}
	}

	/* Invoke the page flip routine */
	rc = vqap->Page_Flip((VQAHandle *)vqap);

	/* Update data for mono output */
	vqap->Flipper.LastFrameNum = curframe->FrameNum;

	/* Mark the frame as loadable */
	if (!(drawer->Flags & VQADRWF_HOLD)) {
		curframe->PrevFlags = curframe->Flags;
		curframe->Flags = 0;
	} else {
		if (drawer->Flags & VQADRWF_FORCEDRAW) {
			curframe->Flags |= VQAFRMF_11;
			drawer->Flags &= ~VQADRWF_FORCEDRAW;
		}
		drawer->Flags &= ~VQADRWF_HOLD;
	}

	return(rc);
}


long VQA_NumFramesWithPalettes(VQAHandleP *vqap)
{
	long num = 0;
	for (int i = 0; i < vqap->NumFrames; i++) {
		if (vqap->Foff[i] & VQAFINF_PAL) {
			num++;
		}
	}
	return(num);
}


long VQA_GetXYPos(VQAHandleP *vqap, int & x, long & y)
{
	VQAHeader *header;

	header = &vqap->Header;

	x = header->Xpos;
	y = header->Ypos;

	return(VQAERR_NONE);
}


/// <summary>
/// Fetches the size of the largest loop in the movie.
/// Each loop is measured from the frame offset table, so the result is how much of
/// the file the loop occupies. This routine is used to size the loop cache, so that
/// any single loop can be held in memory and replayed without going back to the
/// stream.
/// </summary>
/// <param name="streamsize">Total size of the stream in bytes, or 0 if it is not known.</param>
/// <returns>Returns with the size in bytes of the largest loop.</returns>
long VQA_LargestLoop(VQAHandleP *vqap, long streamsize)
{
	VQALoopInfo::HEADER *infohdr = &vqap->LoopInfo.Header;
	long largest = 0;

	int count = infohdr->Count;
	for (int i = 0; i < count; i++) {
		VQALoopInfo::DATA *data = &vqap->LoopInfo.Data[i];
		int start = data->StartFrame;
		int stop = data->EndFrame;

		long start_offset;
		long end_offset;

		long loopsize = 0;
		if (stop != vqap->NumFrames - 1) {
			end_offset = VQAFRAME_OFFSET(vqap->Foff[stop + 1]);
			start_offset = VQAFRAME_OFFSET(vqap->Foff[start]);
			loopsize = end_offset - start_offset;
		} else if (streamsize > 0) {
			end_offset = streamsize;
			start_offset = VQAFRAME_OFFSET(vqap->Foff[start]);
			loopsize = end_offset - start_offset;
		} else {
			end_offset = VQAFRAME_OFFSET(vqap->Foff[stop]);
			start_offset = VQAFRAME_OFFSET(vqap->Foff[start]);
			loopsize = end_offset - start_offset;
		}
		if (loopsize > largest) {
			largest = loopsize;
		}
	}
	return(largest);
}



