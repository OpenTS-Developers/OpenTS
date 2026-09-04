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
*     audio.c
*
* DESCRIPTION
*     Audio playback and timing.
*
* PROGRAMMER
*     Bill Randolph
*     Denzil E. Long, Jr.
*
* DATE
*     August 4, 1995
*
*
* HISTORY:
*     Modified for Win95 Direct Sound - Steve T 1/2/96 6:35AM
*
*----------------------------------------------------------------------------
*
* PUBLIC
*     VQA_StartTimerInt - Initialize system timer interrupt.
*     VQA_StopTimerInt  - Remove system timer interrupt.
*     VQA_SetTimer      - Resets current time to given tick value.
*     VQA_GetTime       - Return current time.
*     VQA_TimerMethod   - Get timer method being used.
*     VQA_OpenAudio     - Open sound system.
*     VQA_CloseAudio    - Close sound system
*     VQA_StartAudio    - Starts audio playback
*     VQA_StopAudio     - Stop audio playback.
*     CopyAudio         - Copy data from Audio Temp buf into Audio play buf.
*
* PRIVATE
*     TimerCallback - VQA timer event. (Called by HMI)
*     AutoDetect    - Auto detect the sound card.
*     AudioCallback - Sound system callback.
*
****************************************************************************/

#include	"vqaplayp.h"
#include	<stdio.h>
#include	<memory.h>
#include	"ahandle.h"
#include	"vqamem.h"


/*---------------------------------------------------------------------------
 * AUDIO DEFINITIONS
 *-------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
 * PROTOTYPES
 *-------------------------------------------------------------------------*/
/* Dummy functions used to mark the start/end address of the file. */
static void StartAddr(void);
static void EndAddr(void);

/*---------------------------------------------------------------------------
 * GLOBAL DATA
 *-------------------------------------------------------------------------*/
/* This is a dummy function that is used to mark the start of the module.
 * It is necessary for locking the memory the module occupies. This prevents
 * the virtual memory manager from swapping out this memory.
 */
static void StartAddr(void)
{
}

/****************************************************************************
*
* NAME
*     VQA_OpenAudio - Open sound system.
*
* SYNOPSIS
*     Error = VQA_OpenAudio(VQAHandleP)
*
*     long VQA_OpenAudio(VQAHandleP *);
*
* FUNCTION
*     Initialise the sound system. Create a direct sound object and the
*     direct sound primary sound buffer if they dont already exist.
*
* INPUTS
*     VQAHandleP - Pointer to private VQAHandle.
*
* RESULT
*     Error - 0 if successful, -1 if error.
*
****************************************************************************/

VQAErrorType VQA_OpenAudio(VQAHandleP *vqap)
{
	VQAAudio *audio;

	/* Dereference data memebers for quicker access. */
	audio = &vqap->Audio;

	if (audio->Buffer == NULL) {
		return(VQAERR_AUDIO);
	}

	audio->Block1 = 0;
	audio->Block2 = -1;
	audio->BufferPosition = 0;
	vqap->RepeatedBuffers = 0;

	AhandleInitParams params;
	params.SampleRate = vqap->SampleRate;
	params.Channels = vqap->Channels;
	params.BitsPerSample = vqap->BitsPerSample;
	params.Callback1 = VQA_AudioFillCallback;
	params.Callback2 = VQA_AudioDoneCallback;

	auto const opened = vqap->Config.AudioHandler((VQAHandle *)vqap, VQAAUDIO_OPEN, &params, sizeof(params));

	if (opened >= VQAERR_OK || opened == VQAERR_NONE) {

		/* Lock the memory occupied by this module. */
		if ((audio->Flags & VQAAUDF_MODLOCKED) == 0) {
			audio->Flags |= VQAAUDF_MODLOCKED;
		}
		return(VQAERR_NONE);
	}
	return((VQAErrorType)opened);
}


/****************************************************************************
*
* NAME
*     VQA_CloseAudio - Close sound system
*
* SYNOPSIS
*     VQA_CloseAudio()
*
*     void VQA_CloseAudio(void);
*
* FUNCTION
*     Removes VQA's involvement in the audio system.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
****************************************************************************/

void VQA_CloseAudio(VQAHandleP *vqap)
{
	VQAAudio *audio;

	/* Dereference for quick access. */
	audio = &vqap->Audio;

	/*
	** If the audio is still playing then stop it
	*/
	if (audio->Flags & VQAAUDF_ISPLAYING) {
		VQA_StopAudio(vqap);
	}
	vqap->Config.AudioHandler((VQAHandle *)vqap, VQAAUDIO_CLOSE, NULL, NULL);

	/* Unlock the memory accupied by this module. */
	if ((audio->Flags & VQAAUDF_MODLOCKED) == 1) {
		audio->Flags &= ~VQAAUDF_MODLOCKED;
	}
}


/****************************************************************************
*
* NAME
*     VQA_StartAudio - Starts audio playback
*
* SYNOPSIS
*     Error = VQA_StartAudio(VQA)
*
*     long VQA_StartAudio(VQAHandleP *);
*
* FUNCTION
*     Start the audio playback for the movie.
*
* INPUTS
*     VQA - Pointer to private VQA handle.
*
* RESULT
*     Error - 0 if successful, or -1 error code.
*
****************************************************************************/

void VQA_StartAudio(VQAHandleP *vqap)
{
	VQAAudio *audio;

	/* Dereference commonly used data members for quicker access. */
	audio = &vqap->Audio;


	vqap->Config.AudioHandler((VQAHandle *)vqap, VQAAUDIO_START, NULL, NULL);
	audio->Flags |= VQAAUDF_ISPLAYING;
}


void VQA_PauseAudio(VQAHandleP *vqap)
{
	vqap->Config.AudioHandler((VQAHandle *)vqap, VQAAUDIO_PAUSE, NULL, NULL);
	vqap->Audio.Flags &= ~VQAAUDF_ISPLAYING;
}


/****************************************************************************
*
* NAME
*     VQA_StopAudio - Stop audio playback.
*
* SYNOPSIS
*     VQA_StopAudio(VQA)
*
*     void VQA_StopAudio(VQAHandleP *);
*
* FUNCTION
*     Halts the currently playing audio stream.
*
* INPUTS
*     VQA - Pointer to private VQAHandle.
*
* RESULT
*     NONE
*
****************************************************************************/

void VQA_StopAudio(VQAHandleP *vqap)
{

	/* Just return if not playing */
	vqap->Config.AudioHandler((VQAHandle *)vqap, VQAAUDIO_STOP, NULL, NULL);
	vqap->Audio.Flags &= ~VQAAUDF_ISPLAYING;
}


/****************************************************************************
*
* NAME
*     CopyAudio - Copy data from Audio Temp buffer into Audio play buffer.
*
* SYNOPSIS
*     Error = CopyAudio(VQA)
*
*     long CopyAudio(VQAHandleP *);
*
* FUNCTION
*     This routine just copies the data in the TempBuf into the correct
*     spots in the audio play buffer.  If there is no room available in the
*     audio play buffer, the routine returns VQAERR_SLEEPING, which will put
*     the whole Loader to "sleep" while it waits for a free buffer.
*
*     If there's no data in the TempBuf to copy, the routine just returns 0.
*
* INPUTS
*     VQA - Pointer to private VQAHandle structure.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long CopyAudio(VQAHandleP *vqap)
{
	VQAAudio  *audio;
	VQAConfig *config;
	VQALoader *loader;

	unsigned long startblock;
	unsigned long endblock;
	unsigned long len1,len2;
	unsigned long i;

	unsigned char *tempbuf;
	unsigned long tempbuflen;

	/* Dereference commonly used data members for quicker access. */
	audio = &vqap->Audio;
	config = &vqap->Config;
	loader = &vqap->Loader;

	/* If audio is disabled, or if we're playing from a VOC file, or if
	 * there's no Audio Buffer, or if there's no data to copy, just return 0
	 */
	#if(VQAVOC_ON && VQAAUDIO_ON)
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0) || (vqap->vocfh != -1)
			|| (audio->Buffer == NULL) || (audio->TempBufLen == 0)) {
	#else  /* VQAVOC_ON */
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0) || (audio->Buffer == NULL)
			|| (audio->TempBufLen == 0)) {
	#endif /* VQAVOC_ON */

		return(VQAERR_NONE);
	}

	tempbuf = audio->TempBuf + audio->BufferOffset;
	tempbuflen = audio->TempBufLen - audio->BufferOffset;

	/* Compute start & end blocks to copy into */
	startblock = (audio->AudBufPos / config->HMIBufSize);
	endblock = (audio->AudBufPos + tempbuflen) / config->HMIBufSize;

	if (endblock >= audio->NumAudBlocks) {
		endblock -= audio->NumAudBlocks;
	}

	/* If 'endblock' hasn't played yet, return VQAERR_SLEEPING */
	if (audio->IsLoaded[endblock] == 1) {
		loader->WaitsOnAudio++;
		return(VQAERR_SLEEPING);
	}

	/* Copy the data:
	 *
	 *  - If 'startblock' < 'endblock', copy the entire buffer
	 *  - Otherwise, fill to the end of the buffer with part of the data, then
	 *    copy the rest to the beginning of the buffer
	 */
	if (startblock <= endblock) {

		/* Copy data */
		memcpy((audio->Buffer + audio->AudBufPos), tempbuf,
				tempbuflen);

		/* Adjust current load position */
		audio->AudBufPos += tempbuflen;

		/* Mark buffer as empty */
		audio->TempBufLen = 0;

		audio->BufferOffset = 0;

		/* Set all blocks to loaded */
		for (i = startblock; i < endblock; i++) {
			audio->IsLoaded[i] = 1;
		}

	} else {

		/* Compute length of each piece */
		len1 = config->AudioBufSize - audio->AudBufPos;

		len2 = tempbuflen - len1;

		/* Copy 1st piece into end of Audio Buffer */
		memcpy((audio->Buffer + audio->AudBufPos), tempbuf, len1);

		/* Copy 2nd piece into start of Audio Buffer */
		memcpy(audio->Buffer, tempbuf + len1, len2);


		/* Adjust load position */
		audio->AudBufPos = len2;

		/* Mark buffer as empty */
		audio->TempBufLen = 0;

		audio->BufferOffset = 0;

		/* Set blocks to loaded */
		for (i = startblock; i < audio->NumAudBlocks; i++) {
			audio->IsLoaded[i] = 1;
		}

		for (i = 0; i < endblock; i++) {
			audio->IsLoaded[i] = 1;
		}
	}

	return(VQAERR_NONE);
}


long __cdecl VQA_AudioFillCallback(VQAHandleP *vqap)
{
	VQAAudio *audio;
	VQAConfig *config;

	audio = &vqap->Audio;
	config = &vqap->Config;

	long size = config->HMIBufSize;

	if (audio->Flags & VQAAUDF_ISDONE) {
		return(0);
	}

	long pos = audio->BufferPosition;
	unsigned long block = audio->Block1;

	if (config->OptionFlags & VQAOPTF_WAITFILL) {
		long nblock = block + 1;
		if ((unsigned)nblock >= audio->NumAudBlocks) {
			nblock = 0;
		}
		if (!(audio->Flags & VQAAUDF_ISENDOFFILE)) {
			if (audio->IsLoaded[nblock] == 0) {
				audio->Flags |= VQAAUDF_ISSTARVED;
			}
		}
	}

	bool repeating = false;
	if (audio->IsLoaded[block] == 1) {
		if (audio->Block2 == -1) {
			audio->Block2 = block;
			audio->PlayPosition = pos;
		}
		audio->BlockRepeats[block] = 0;
		block++;
		long npos = pos + size;
		if (npos >= config->AudioBufSize) {
			npos = 0;
			block = 0;
		}
		audio->BufferPosition = npos;
		audio->Block1 = block;
	} else {

		if (audio->Flags & VQAAUDF_ISENDOFFILE) {
			audio->Flags |= VQAAUDF_ISDONE;

			if ((unsigned)pos < audio->AudBufPos) {
				size = audio->AudBufPos - pos;
			} else {
				size = 0;
			}

		} else {
			audio->Flags |= VQAAUDF_ISREPEATING;
			repeating = true;

			vqap->RepeatedBuffers++;

			pos -= size;

			block--;

			if (pos < 0) {
				pos = config->AudioBufSize - size;
				block = audio->NumAudBlocks - 1;
			}

			audio->BlockRepeats[block]++;
		}

	}

	if (size > 0) {
		if (repeating == true) {
			config->AudioHandler((VQAHandle *)vqap, VQAAUDIO_LOAD, audio->HMIBuffer, size);
		} else {
			config->AudioHandler((VQAHandle *)vqap, VQAAUDIO_LOAD, audio->Buffer + pos, size);
		}
	}
	return(size);
}


long __cdecl VQA_AudioDoneCallback(VQAHandleP *vqap, unsigned long buffer)
{
	VQAConfig *config;
	VQAAudio *audio;
	unsigned long  block;

	audio = &vqap->Audio;
	config = &vqap->Config;

	if ((void *)buffer == audio->Buffer + audio->PlayPosition || (void *)buffer == audio->HMIBuffer) {

		block = audio->Block2;

		if (block != -1) {

			if (!audio->BlockRepeats[block]) {
				/* Update this block's status to loadable (0) */
				audio->IsLoaded[block] = 0;

				block++;

				if (block >= audio->NumAudBlocks) {
					block = 0;
				}

				if (audio->IsLoaded[block] == 1) {
					/* Update position within audio buffer */
					audio->PlayPosition += config->HMIBufSize;

					if (audio->PlayPosition >= (unsigned)config->AudioBufSize) {
						audio->PlayPosition = 0;
					}
					audio->Block2 = block;
				} else {
					audio->Block2 = -1;
				}

			} else {
				audio->BlockRepeats[block]--;
			}
			return(VQAERR_NONE);
		}
		return(VQAERR_BADBLOCK);
	}
	return(VQAERR_AUDSYNC);
}


/* Dummy function used to mark the beginning address of the file. */
static void EndAddr(void)
{
}

enum SCodeType {
	CODE_2BIT,				// Bit packed 2 bit delta.
	CODE_4BIT,				// Nibble packed 4 bit delta.
	CODE_RAW,				// Raw sample.
	CODE_SILENCE			// Run of silence.
};
signed int _2bitdecode[4] = {-2, -1, 0, 1};
signed int _4bitdecode[16] = {-9, -8, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 8};

extern "C" size_t AudioUnzap(void * source, void * dest, size_t size)
{
	unsigned int	previous = 0x0080;
	signed char		*s = (signed char *)source;
	unsigned char	*d = (unsigned char *)dest;
	size_t incount = 0;

	/*
	**	Uncompress the source data until the buffer is filled.
	*/
	while (size > 0) {
		signed char code;		// Compression code.
		int counter;

		code = *s++;
		counter = (code & 0x3F) + 1;
		incount++;

		switch ((code >> 6) & 0x03) {
			case CODE_RAW:

				/*
				**	The "raw" code could actually contain an embedded 5 bit delta.
				**	If this is the case then this is a self contained code.  Extract
				**	and process the delta.
				*/
				if ((counter-1) & 0x20) {
					counter = (counter-1) & 0x1F;
					if (counter & 0x10) counter |= 0xFFE0;
					previous = *d++ = previous + counter;
					size--;

				} else {

					/*
					**	Normal run of raw samples.
					*/
					memcpy(d, s, counter);
					incount += counter;
					size -= counter;
					d += counter-1;
					s += counter;
					previous = *d++;
				}
				break;

			case CODE_4BIT:
				while (counter) {
					int delta;

					delta = *s++;
					incount++;

					previous += (signed)_4bitdecode[delta & 0x0F];
					if (((signed)previous) < 0) previous = 0;
					if (((signed)previous) > 255) previous = 255;
					*d++ = previous;
					size--;

					previous += (signed)_4bitdecode[(delta >> 4) & 0x0F];
					if (((signed)previous) < 0) previous = 0;
					if (((signed)previous) > 255) previous = 255;
					*d++ = previous;
					size--;

					counter--;
				}
				break;

			case CODE_2BIT:
				while (counter) {
					int delta;

					delta = *s++;
					incount++;

					previous += (signed)_2bitdecode[delta & 0x03];
					if (((signed)previous) < 0) previous = 0;
					if (((signed)previous) > 255) previous = 255;
					*d++ = previous;
					size--;

					previous += (signed)_2bitdecode[(delta >> 2) & 0x03];
					if (((signed)previous) < 0) previous = 0;
					if (((signed)previous) > 255) previous = 255;
					*d++ = previous;
					size--;

					previous += (signed)_2bitdecode[(delta >> 4) & 0x03];
					if (((signed)previous) < 0) previous = 0;
					if (((signed)previous) > 255) previous = 255;
					*d++ = previous;
					size--;

					previous += (signed)_2bitdecode[(delta >> 6) & 0x03];
					if (((signed)previous) < 0) previous = 0;
					if (((signed)previous) > 255) previous = 255;
					*d++ = previous;
					size--;

					counter--;
				}
				break;

			default:
			case CODE_SILENCE:
				memset(d, previous, counter);
				d += counter;
				size -= counter;
				break;
		}
	}
	return (incount);
}
