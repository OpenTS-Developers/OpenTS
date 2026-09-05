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

#include "always.h"

#include "vqa.h"

#include "_keyboar.h"
#include "ccfile.h"
#include "dbgprint.h"
#include "globals.h"
#include "goptions.h"
#include "mixfile.h"
#include "movies.h"
#include "session.h"
#include "unvqtblc.h"
#include "vector.h"
#include "vqoption.h"
#include "win.h"

#include "special.hh"

// lib includes here.
#include "ahandle.h"

#include <unvq.h>
#include <vqaplay.h>
#include <vqaplayp.h>

DynamicVectorClass<VQHandle *> IngameVQ;

long __cdecl VQAMixFileHandler(VQAHandle * vqa, long action, void * buffer, long nbytes);
long __cdecl VQACCFileHandler(VQAHandle * vqa, long action, void * buffer, long nbytes);
long __cdecl VQAEventHandler(VQAHandle * vqa, long action, void * buffer, long nbytes);
long __cdecl VQAMemoryHandler(VQAHandle * vqa, long action, void * buffer, long nbytes);

bool VQA_Message_Handler(void)
{
	MSG msg;

	if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		if (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			return(false);
		}
	}
	return(true);
}


/***************************************************************************
 * VQAClass::VQAClass -- Constructor for VQAClass object.                  *
 *                                                                         *
 * INPUT:                                                                  *
 *   base_filename - vqa filename without the extension.                   *
 *   buffer - buffer to draw to.                                           *
 *   callback - pointer to function that will blit the drawn frames.       *
 *   media_source - either FROM_MEMORY or FROM_DISK.                       *
 *   id - more specific info about the vqa, like exact property location.  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY: See PVCS log                                                   *
 *=========================================================================*/
VQAClass::VQAClass(char const * filename, int flags, VQA_SURF_LOCK_CALLBACK surface_lock, VQA_SURF_UNLOCK_CALLBACK surface_unlock, VQA_SURF_DRAW_CALLBACK surface_draw, int frame_rate, int draw_rate)
{
	unsigned char buffer;

	// Initialize config options.
	VQA_DefaultConfig(&Config);

	Flags = flags;

	//-------------------------------------------------------------------------
	// Set up video config options.
	//-------------------------------------------------------------------------
	Config.Owner = this;
	Config.ImageBuf  = (unsigned char *)&buffer;
	Config.DrawFlags = 0;


	//
	// Set up draw options.
	//
	if (Get_Option(OPTION_NO_BUFFER) == false) {
		Config.DrawFlags |= VQACFGF_BUFFER;
	}

	if (Get_Option(OPTION_1) == false) {
		Config.DrawFlags |= VQACFGF_NOSKIP;
	}

	if (Get_Option(OPTION_10) == true) {
		Config.DrawFlags |= VQACFGF_HALFSIZE;
	}

	if (Get_Option(OPTION_12) == true) {
		Config.DrawFlags |= VQACFGF_NOTRANS;
	}

	Config.OptionFlags |= VQAOPTF_PALOFF;

	if ((Flags & VQACF_2)) {
		Config.OptionFlags |= VQAOPTF_LOOPS;
		if ((Flags & VQACF_4)) {
			Config.OptionFlags |= VQAOPTF_LOOPCACHE;
		}
	} else {
		Config.OptionFlags &= ~VQAOPTF_LOOPS;
	}

	if (Get_Option(OPTION_17) == true) {
		Config.OptionFlags |= VQAOPTF_MONO;
	} else {
		Config.OptionFlags &= ~VQAOPTF_MONO;
	}

	if (Get_Option(OPTION_NO_AUDIO) == true) {
		Config.OptionFlags &= ~VQAOPTF_AUDIO;
	} else {
		Config.OptionFlags |= VQAOPTF_AUDIO;
	}

	Config.ImageWidth = -1;
	Config.ImageHeight = -1;
	Config.X1 = 0;
	Config.Y1 = 0;

	//
	// Set up the drawer callback for the VQA.
	//
	Config.DrawerCallback = NULL;

	//
	// Set the load rate (it is misnamed FrameRate)
	//
	Config.FrameRate = frame_rate;

	//
	// Set the frame rate (DrawRate)
	//
	Config.DrawRate = draw_rate;

	if (Get_Option(OPTION_NO_AUDIO) == true) {
		Config.NumFrameBufs = 1;
	} else {
		Config.NumFrameBufs = 14;
	}

	Config.NumCBBufs = -1;

	if (Flags & VQACF_PLAY_FROM_MIXFILE) {
		Config.StreamHandler = VQAMixFileHandler;
	} else {
		Config.StreamHandler = VQACCFileHandler;
	}

	Config.MemoryHandler = VQAMemoryHandler;
	Config.EventHandler = VQAEventHandler;

	if (Get_Option(OPTION_SIMPLE_TIMER_CALLBACK) == true) {
		Config.TimerCallback = Simple_Timer_Callback_Audio_Handler;
		Config.RefreshRate = 60;
	} else {
		Config.TimerCallback = Timer_Callback_Audio_Handler;
		Config.RefreshRate = 60;
	}

	Config.AudioHandler = Stream_Audio_Handler;
	Config.HMIBufSize = 8192;

	//-------------------------------------------------------------------------
	// Initialize private class variables.
	//-------------------------------------------------------------------------

	// Initialize starting frame number to first frame of movie.
	CurrentFrame = 0;

	Event3Frame = 0;

	// Set total frames.
	TotalFrames = 0;

	// Save the base filename for this VQA instance.
	strcpy(Filename, filename);

	Width = 0;
	Height = 0;
	DrawBufferWidth = 0;
	DrawBufferHeight = 0;
	PrimaryColorMode = -1;
	SurfaceLockCallback = surface_lock;
	SurfaceUnlockCallback = surface_unlock;
	SurfaceDrawCallback = surface_draw;
	IsFileOpen = false;

	// Initially, vqa is not open.
	IsOpen = false;

	IsPaused = false;
	IsAdvanceReady = false;
}


VQAClass::~VQAClass(void)
{
	Hicolor_Clear_Table();
}


long VQAClass::Cache_VQA(unsigned long bytes_to_cache)
{
	if (!IsFileOpen) {
		IsFileOpen = FileHandle.Open(Filename, FileClass::READ) != 0;
		if (!IsFileOpen) {
			return(CACHE_OPEN_FILE_ERROR);
		}
		int size = FileHandle.Size();
		Cache.file_size = size;
		Cache.file_buffer_pos = 0;
		Cache.file_offset = 0;
		Cache.file_buffer = (unsigned char *)malloc(size);
		if (Cache.file_buffer == NULL) {
			FileHandle.Close();
			IsFileOpen = false;
			return(CACHE_FAILED_MEM_ALLOC);
		}
	}

	if (bytes_to_cache == CACHE_REST_OF_FILE) {
		bytes_to_cache = Cache.file_size - Cache.file_offset;
	} else {
		if (Cache.file_offset + bytes_to_cache > Cache.file_size) {
			bytes_to_cache = Cache.file_size - Cache.file_offset;
		}
	}

	if ((unsigned)FileHandle.Read(Cache.file_buffer + Cache.file_offset, bytes_to_cache) != bytes_to_cache) {
		FileHandle.Close();
		IsFileOpen = false;
		Cache.file_buffer_pos = 0;
		Cache.file_offset = 0;
		return(CACHE_READ_ERROR);
	}

	Cache.file_offset += bytes_to_cache;
	if (Cache.file_offset >= Cache.file_size) {
		FileHandle.Close();
		IsFileOpen = false;
		return(CACHE_EOF);
	}
	return(CACHE_NO_ERROR);
}


long VQAClass::CacheHandler(long action, void * buffer, long nbytes)
{
	//unsigned long pos;
	long rc = 1;

	switch (action) {
		case VQACMD_INIT:
		//	rc = 0;
		//	break;
		case VQACMD_CLEANUP:
		//	rc = 0;
		//	break;
		case VQACMD_OPEN:
		//	rc = 0;
		//	break;
		case VQACMD_CLOSE:
			rc = 0;
			break;

		case VQACMD_READ:
			if (Cache.file_buffer_pos + nbytes > Cache.file_size) {
				rc = 1;
				return(rc);
			} else {
				memcpy(buffer, Cache.file_buffer + Cache.file_buffer_pos, nbytes);
				Cache.file_buffer_pos += nbytes;
				rc = 0;
			}
			break;

		case VQACMD_SEEK:
			switch ((int)buffer) {
				case 1:
					Cache.file_buffer_pos += nbytes;
					rc = 0;
					break;
				case 0:
					Cache.file_buffer_pos = nbytes;
					rc = 0;
					break;
				case 2:
					Cache.file_buffer_pos = Cache.file_size - nbytes - 1;
					rc = 0;
					break;

				default:
					rc = 1;
					break;
			}
			break;


		default:
			rc = 1;
			break;
	}
	return(rc);
}


/***************************************************************************
 * VQACLASS::OPEN_AND_LOAD_BUFFERS -- Opens VQA file and fills frame buffrs*
 *                                                                         *
 * INPUT:                                                                  *
 *    none                                                                 *
 *                                                                         *
 * OUTPUT:                                                                 *
 *    TRUE if successful                                                   *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   See PVCS log                                                          *
 *=========================================================================*/
bool VQAClass::Open_And_Load_Buffers(void)
{

	//
	// Open the VQA file, allocate the buffers for this VQ instance, and pre-
	// load the buffers.
	//
	if (VQA_Open(Filename, &Config, &Handle) != VQAERR_NONE) {
		IsOpen = false;
		return(false);
	}

	//
	// Get frame information about the VQA.
	//
	VQAHandleP *vqap = (VQAHandleP *)Handle;
	TotalFrames = vqap->NumFrames;
	DesiredColorMode = vqap->ColorMode;
	Width = vqap->ImageWidth;
	Height = vqap->ImageHeight;

	IsOpen = true;

	return(true);
}


int VQAClass::Get_VQA_Version(void) const
{
	if (Handle != NULL) {
		return((VQAHandleP *)Handle)->Version;
	}
	return(-1);
}


int VQAClass::Set_VQA_Volume(int vol)
{
	int old = Config.Volume;
	if (vol != -1) {
		Config.Volume = vol;
	}
	return(old);
}


bool VQAClass::Set_Loop(int loop_id, int iterations)
{
	if (iterations < 0) {
		iterations = -1;
	}
	long rc = VQA_SetLoop(Handle, loop_id, iterations, 1);
	if (rc == VQAERR_NONE) {
		return(true);
	}
	return(false);
}


bool VQAClass::Set_Loop(int start, int end, int iterations)
{
	if (start < 0 || start >= TotalFrames) {
		start = 0;
	}

	if ( end < 0 || end >= TotalFrames) {
		end = TotalFrames - 1;
	}

	if (iterations < 0) {
		iterations = -1;
	}

	long rc = VQA_SetLoop_Internal(Handle, start, end, iterations, 0);
	if (rc == VQAERR_NONE) {
		return(true);
	}
	return(false);
}


/***************************************************************************
 * VQACLASS::SEEK_TO_FRAME -- Performs file seek to specified frame.       *
 *                                                                         *
 * INPUT:                                                                  *
 *    frame - frame number to seek to.                                     *
 *                                                                         *
 * OUTPUT:                                                                 *
 *    none                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   See PVCS log                                                          *
 *=========================================================================*/
void VQAClass::Seek_To_Frame(int frame)
{
	if (frame == (int)CurrentFrame) {
		return;
	}

	if (VQA_SeekFrame(Handle, frame, SEEK_SET) != VQAERR_SEEK) {
		CurrentFrame = frame;
	}
}


bool VQAClass::Seek_To_Stored_Frame(void)
{
	if (IsOpen) {
		if (VQA_SeekFrame(Handle, Event3Frame, 0) != VQAERR_SEEK) {
			CurrentFrame = Event3Frame;
			return(true);
		}
		return(false);
	}
	return(true);
}


/***************************************************************************
 * Play_VQA - Plays from the current frame of the VQA up to and including  *
 *      the last frame specified.                                          *
 *                                                                         *
 * INPUT: INT last_frame - last frame to be displayed                      *
 *                                                                         *
 * OUTPUT: NONE                                                            *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   02/20/1995  MG : Created.                                             *
 *=========================================================================*/
int VQAClass::Play_VQA(int last_frame_to_play, bool nobreakout)
{
	long errval;

	bool brokeout = false;
	bool sleeping = false;

	if (last_frame_to_play < 0 || last_frame_to_play >= TotalFrames) {
		last_frame_to_play = TotalFrames - 1;
	}
	VQA_SetStop(Handle, last_frame_to_play);

	if (PrimaryColorMode != -1) {
		long blockw;
		long blockh;
		long cmode;
		VQA_GetBlockInfo(Handle, blockw, blockh, cmode);
		if (cmode == 1) {
			Hicolor_Init_Table(PrimaryColorMode);
			if (Get_Option(OPTION_ALTERNATE_UNVQ)) {
				VQA_SetUnVQ(Handle, UnVQ1_C1_TABLE_ALT, UnVQ2_4x2_Table);
			} else {
				VQA_SetUnVQ(Handle, UnVQ1_C1_TABLE, UnVQ2_4x4_Table);
			}
		} else if (cmode == 4) {
			Hicolor_Init_Table(PrimaryColorMode);
			if (Get_Option(OPTION_ALTERNATE_UNVQ)) {
				VQA_SetUnVQ(Handle, UnVQ1_4x4_Table, UnVQ1_4x2_Table);
			}
		}
	}

	//
	// Play frames until the last desired frame is drawn or breakout occurs.
	//
	errval = 0;

	while (errval >= VQAERR_OK || errval == VQAERR_NOT_TIME || errval == VQAERR_NOBUFFER || errval == VQAERR_SLEEPING || errval == VQAERR_NONE) {
		if (brokeout) {
			break;
		}

		//
		// Check for Windows Messages.
		//
		VQA_Message_Handler();

		if (sleeping == true) {
			if (!GameInFocus) {
				Sleep((1000/30));
				continue;
			} else {
				sleeping = false;
				IsPaused = false;
				DebugString("Movie is awake\n");
			}
		}

		if (GameInFocus == true && !IsPaused) {

			//
			// Maybe draw another frame.
			//
			errval = VQA_Play(Handle, VQAMODE_WALK, 0);

			if (errval >= VQAERR_OK) {
				if (SurfaceDrawCallback) {
					SurfaceDrawCallback();
				}
			}

			// If we actually played a frame update the current frame number.
			if (errval >= VQAERR_OK) {
				CurrentFrame = errval;
				CurrentFrame++;
			}
		} else {
			if (sleeping == false) {
				sleeping = true;
				if (IsOpen && !IsPaused) {
					VQA_Play(Handle, VQAMODE_PAUSE, 0);
				}
				IsPaused = true;
				DebugString("Movie is sleeping\n");
			}
		}

		if (!nobreakout && Keyboard->Check() && Keyboard->Get() == (KN_ESC|WWKEY_RLS_BIT)) {
			brokeout = true;
		}
	}

	if (brokeout) {
		return(VQA_PLAY_USER_BREAK);
	}
	return(VQA_PLAY_NO_ERROR);
}


/***************************************************************************
 * Play_VQA_Frame - Plays the specified frame, seeking to it if necessary. *
 *                                                                         *
 * INPUT: INT frame_number                                                 *
 *                                                                         *
 * OUTPUT: NONE                                                            *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   02/20/1995  MG : Created.                                             *
 *=========================================================================*/
void VQAClass::Play_VQA_Frame(int frame_number)
{
	if (frame_number == PLAY_LAST_FRAME) {
		frame_number = TotalFrames - 1;
	}

	//
	// If not currently at the desired frame, seek to it.
	//
	if ((unsigned)CurrentFrame != (unsigned)frame_number) {
		Seek_To_Frame(frame_number);
		CurrentFrame = frame_number;
	}

	//
	// Display the frame.
	//
	Play_VQA(frame_number, 0);

	//
	// If at end of movie, reset to start of movie.
	//
	if ((unsigned)CurrentFrame >= (unsigned)TotalFrames) {
		CurrentFrame = 0;
	}
}


bool VQAClass::Advance_Frame(bool & finished)
{
	static bool _state = false;
	finished = false;

	if (!IsAdvanceReady) {
		VQA_SetStop(Handle, TotalFrames);
		if (PrimaryColorMode != -1) {
			long blockw;
			long blockh;
			long cmode;
			VQA_GetBlockInfo(Handle, blockw, blockh, cmode);
			if (cmode == 1 || cmode == 4) {
				Hicolor_Init_Table(PrimaryColorMode);
				if (cmode == 1) {
					VQA_SetUnVQ(Handle, UnVQ1_C1_TABLE, UnVQ2_4x4_Table);
				}
			}
		}
		DebugString("VQAClass::Advance_Frame() - Ready\n");
		IsAdvanceReady = true;
	}

	if (IsPaused == true) {
		if (!Session.Play && SpecialDialog == SDLG_NONE && GameInFocus == true) {
			IsPaused = false;
			DebugString("VQAClass::Advance_Frame() - Unpause\n");
		}
	}

	if (GameInFocus == true && !IsPaused) {
		int res = VQA_Play(Handle, VQAMODE_WALK, 0);
		if (res >= VQAERR_OK) {
			if (SurfaceDrawCallback) {
				SurfaceDrawCallback();
			}
			CurrentFrame = res + 1;
			return(true);
		}

		if (res >= VQAERR_SLEEPING && (res <= VQAERR_NOT_TIME || res == VQAERR_NONE)) {
			finished = false;
		} else {
			finished = true;
		}

		return(false);
	}

	if (!_state) {
		Pause_VQA();
		_state = true;
		DebugString("VQAClass::Advance_Frame() - Pause\n");
	}
	return(false);
}


/***************************************************************************
 * Pause_VQA - Pauses a VQA in order to freeze the VQA timers.             *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   03/29/1995  MG : Created.                                             *
 *=========================================================================*/
void VQAClass::Pause_VQA(void)
{
	if (IsOpen) {
		if (!IsPaused) {
			VQA_Play(Handle, VQAMODE_PAUSE, 0);
		}
	}
	IsPaused = true;
}


void VQAClass::Resume_VQA(void)
{
	IsPaused = false;
}


/***************************************************************************
 * VQACLASS::CLOSE_AND_FREE_VQA -- Closes vqa, frees instance, frees cache *
 *                                                                         *
 * INPUT:                                                                  *
 *   none                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *   none                                                                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   See PVCS log.                                                         *
 *=========================================================================*/
void VQAClass::Close_And_Free_VQA(void)
{
	if (IsOpen) {
		VQA_Close(Handle);
	}
	//VQA_Free(Handle);
}


void VQAClass::Reset_VQA(void)
{
	VQA_Reset(Handle);
}


int VQAClass::Get_Desired_Color_Mode(void)
{
	return(DesiredColorMode);
}


bool VQAClass::Set_Draw_Buffer(void * buffer, int buffer_width, int buffer_height, int x_offset, int y_offset)
{
	if (buffer_width == -1) {
		buffer_width = DrawBufferWidth;
	} else {
		DrawBufferWidth = buffer_width;
	}
	if (buffer_height == -1) {
		buffer_height = DrawBufferHeight;
	} else {
		DrawBufferHeight = buffer_height;
	}
	if (x_offset == -1) {
		x_offset = 0;
	}
	if (y_offset == -1) {
		y_offset = 0;
	}
	long rc = VQA_Set_DrawBuffer(Handle, (unsigned char *)buffer, buffer_width, buffer_height, x_offset, y_offset);
	if (rc == VQAERR_NONE) {
		return(true);
	}
	return(false);
}


void VQAClass::Set_Primary_Color_Mode(int mode)
{
	PrimaryColorMode = mode;
}


void VQAClass::Handle_Codebook_Event(void *buf, unsigned int num)
{
	if (PrimaryColorMode != -1) {
		Hicolor_Translate(buf, num >> 1);
	}
	Event3Frame = CurrentFrame;
}


int VQAClass::Get_VQA_Width(void) const
{
	return(Width);
}


int VQAClass::Get_VQA_Height(void) const
{
	return(Height);
}


void *VQAClass::Handle_Lock_Event(void)
{
	return(SurfaceLockCallback());
}


bool VQAClass::Handle_Unlock_Event(void)
{
	return(SurfaceUnlockCallback());
}


long VQAClass::CCFileHandler(long action, void * buffer, long nbytes)
{
	long error = 1;
	unsigned char tmp;

	/*
	**	Perform the action specified by the stream command.
	*/
	switch (action) {

		/*
		**	VQACMD_OPEN asks that you open your stream for access.
		*/
		case VQACMD_OPEN:
			IsFileOpen = FileHandle.Open((char *)buffer, FileClass::READ) != 0;
			error = IsFileOpen == false;
			break;

		/*
		**	VQACMD_READ means read NBytes from the stream and place it in the
		**	memory pointed to by Buffer.
		**
		**	Any error code returned will be remapped by VQA library into
		**	VQAERR_READ.
		*/
		case VQACMD_READ:
			if (FileHandle.Read(buffer, nbytes) != nbytes) {
				error = 1;
			} else {
				error = 0;
			}
			break;

		/*
		**	VQACMD_WRITE is analogous to VQACMD_READ.
		**
		**	Writing is not allowed to the VQA file, VQA library will remap the
		**	error into VQAERR_WRITE.
		*/
		case VQACMD_WRITE:
			error = 1;
			break;

		/*
		**	VQACMD_SEEK asks that you perform a seek relative to the current
		**	position. NBytes is a signed number, indicating seek direction
		** (positive for forward, negative for backward). Buffer has no meaning
		**	here.
		**
		**	Any error code returned will be remapped by VQA library into
		**	VQAERR_SEEK.
		*/
		case VQACMD_SEEK:
			error = (FileHandle.Seek(nbytes, (int)buffer) == 0);
			break;

		case VQACMD_SEEKPEEK:
			if (nbytes > 0) {
				error = FileHandle.Seek(nbytes - sizeof(tmp), (int)buffer) == 0;
				if (error == 0) {
					error = FileHandle.Read(&tmp, sizeof(tmp)) != sizeof(tmp);
				}
			} else {
				error = FileHandle.Seek(nbytes, (int)buffer) == 0;
				if (error == 0) {
					error = FileHandle.Read(&tmp, sizeof(tmp)) != sizeof(tmp);
					if (error == 0) {
						error = FileHandle.Seek(-((int)sizeof(tmp)), SEEK_CUR) == 0;
					}
				}
			}
			break;

		case VQACMD_SIZE:
			*((int *)buffer) = FileHandle.Size();
			error = 0;
			break;

		case VQACMD_CLOSE:
			FileHandle.Close();
			IsFileOpen = false;
			error = 0;
			break;

		/*
		**	VQACMD_INIT means to prepare your stream for reading. This is used for
		**	certain streams that can't be read immediately upon opening, and need
		**	further preparation. This operation is allowed to fail; the error code
		**	will be returned directly to the client.
		*/
		case VQACMD_INIT:

		/*
		**	IFFCMD_CLEANUP means to terminate the transaction with the associated
		**	stream. This is used for streams that can't simply be closed. This
		**	operation is not allowed to fail; any error returned will be ignored.
		*/
		case VQACMD_CLEANUP:
			error = 0;
			break;

		default:
			//error = 1;
			break;
	}

	return(error);
}


long __cdecl VQACCFileHandler(VQAHandle * vqa, long action, void * buffer, long nbytes)
{
	VQAHandleP *vqap = (VQAHandleP *)vqa;
	VQAConfig *config = &vqap->Config;
	VQAClass *_this = config->Owner;

	return(_this->CCFileHandler(action, buffer, nbytes));
}


long VQAClass::MixFileHandler(long action, void * buffer, long nbytes)
{
	long error = 1;
	unsigned char tmp;

	/*
	**	Perform the action specified by the stream command.
	*/
	switch (action) {

		/*
		**	VQACMD_OPEN asks that you open your stream for access.
		*/
		case VQACMD_OPEN: {
			int offset = 0;
			MixFileClass * mixfile = NULL;
			if (MixFileClass::Offset(Filename, NULL, &mixfile, &offset)) {
				IsFileOpen = FileHandle.Open(mixfile->Filename, FileClass::READ) != 0;
				error = FileHandle.Seek(offset, SEEK_CUR) == 0;
			} else {
				error = 1;
			}
			break;
		}

		/*
		**	VQACMD_READ means read NBytes from the stream and place it in the
		**	memory pointed to by Buffer.
		**
		**	Any error code returned will be remapped by VQA library into
		**	VQAERR_READ.
		*/
		case VQACMD_READ:
			if (FileHandle.Read(buffer, nbytes) != nbytes) {
				error = 1;
			} else {
				error = 0;
			}
			break;

		/*
		**	VQACMD_WRITE is analogous to VQACMD_READ.
		**
		**	Writing is not allowed to the VQA file, VQA library will remap the
		**	error into VQAERR_WRITE.
		*/
		case VQACMD_WRITE:
			error = 1;
			break;

		/*
		**	VQACMD_SEEK asks that you perform a seek relative to the current
		**	position. NBytes is a signed number, indicating seek direction
		** (positive for forward, negative for backward). Buffer has no meaning
		**	here.
		**
		**	Any error code returned will be remapped by VQA library into
		**	VQAERR_SEEK.
		*/
		case VQACMD_SEEK:
			error = (FileHandle.Seek(nbytes, (int)buffer) == 0);
			break;

		case VQACMD_SEEKPEEK:
			if (nbytes > 0) {
				error = FileHandle.Seek(nbytes - sizeof(tmp), (int)buffer) == 0;
				if (error == 0) {
					error = FileHandle.Read(&tmp, sizeof(tmp)) != sizeof(tmp);
				}
			} else {
				error = FileHandle.Seek(nbytes, (int)buffer) == 0;
				if (error == 0) {
					error = FileHandle.Read(&tmp, sizeof(tmp)) != sizeof(tmp);
					if (error == 0) {
						error = FileHandle.Seek(-((int)sizeof(tmp)), SEEK_CUR) == 0;
					}
				}
			}
			break;

		case VQACMD_SIZE:
			*((int *)buffer) = FileHandle.Size();
			error = 0;
			break;

		case VQACMD_CLOSE:
			FileHandle.Close();
			IsFileOpen = false;
			error = 0;
			break;

		/*
		**	VQACMD_INIT means to prepare your stream for reading. This is used for
		**	certain streams that can't be read immediately upon opening, and need
		**	further preparation. This operation is allowed to fail; the error code
		**	will be returned directly to the client.
		*/
		case VQACMD_INIT:

		/*
		**	IFFCMD_CLEANUP means to terminate the transaction with the associated
		**	stream. This is used for streams that can't simply be closed. This
		**	operation is not allowed to fail; any error returned will be ignored.
		*/
		case VQACMD_CLEANUP:
			error = 0;
			break;

		default:
			//error = 1;
			break;
	}

	return(error);
}


long __cdecl VQAMixFileHandler(VQAHandle * vqa, long action, void * buffer, long nbytes)
{
	VQAHandleP *vqap = (VQAHandleP *)vqa;
	VQAConfig *config = &vqap->Config;
	VQAClass *_this = config->Owner;

	return(_this->MixFileHandler(action, buffer, nbytes));
}


long /*__cdecl*/ VQACacheHandler(VQAHandle * vqa, long action, void * buffer, long nbytes)
{
	VQAHandleP *vqap = (VQAHandleP *)vqa;
	VQAConfig *config = &vqap->Config;
	VQAClass *_this = config->Owner;

	return(_this->CacheHandler(action, buffer, nbytes));
}


long __cdecl VQAMemoryHandler(VQAHandle * vqa, long action, void * buffer, long nbytes)
{
	long error = 0;

	switch (action) {

		case VQAMEM_ALLOC:
			error = (int)malloc(nbytes);
			break;

		case VQAMEM_FREE:
			free(buffer);
			error = 0;
			break;

		case VQAMEM_LOCK:
		case VQAMEM_UNLOCK:
			error = (int)buffer;
			break;

		default:
			error = 0;
			break;

		case VQAMEM_QUERYSIZE:
			*((int *)buffer) = -1;
			error = 0;
			break;


	}
	return(error);
}


static void VQAScalePalette(unsigned char *palette)
{
	//
	// The palette is 6-bit per pixel, so shift all the
	// values to make them 8-bits per pixel.
	//
	for (int i = 0; i < SIZE_OF_PALETTE; i++) {
		palette[i] <<= 2;
	}
}


long __cdecl VQAEventHandler(VQAHandle * vqa, long action, void * buffer, long nbytes)
{
	VQAHandleP *vqap = (VQAHandleP *)vqa;
	VQAConfig *config = &vqap->Config;
	VQAClass *_this = config->Owner;

	long error = 0;

	switch (action) {
		case VQAEVENT_PALETTE:
			VQAScalePalette((unsigned char *)buffer);
			error = 0;
			break;

		case VQAEVENT_CODEBOOK:
			_this->Handle_Codebook_Event(buffer, nbytes);
			error = 0;
			break;

		case VQAEVENT_LOCK:
			error = (int)_this->Handle_Lock_Event();
			break;

		case VQAEVENT_UNLOCK:
			_this->Handle_Unlock_Event();
			error = 0;
			break;

		default:
			error = 0;
			break;
	}

	return(error);
}


int vqanoop1(void)
{
	return(0);
}


void vqanoop2(void)
{
	return;
}


void vqanoop3(void)
{
	return;
}

#if (0)
/***********************************************************************************************
 * MixFileHandler -- Handles VQ file access.                                                   *
 *                                                                                             *
 *    This routine is called from the VQ player when it needs to access the source file. By    *
 *    using this routine it is possible to virtualize the file system.                         *
 *                                                                                             *
 * INPUT:   vqa   -- Pointer to the VQA handle for this animation.                             *
 *                                                                                             *
 *          action-- The requested action to perform.                                          *
 *                                                                                             *
 *          buffer-- Optional buffer pointer as needed by the type of action.                  *
 *                                                                                             *
 *          nbytes-- The number of bytes (if needed) for this operation.                       *
 *                                                                                             *
 * OUTPUT:  Returns a value consistent with the action requested.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/04/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
long MixFileHandler(VQAHandle * vqa, long action, void * buffer, long nbytes)
{
	CCFileClass * file;
	long        error;

	file = (CCFileClass *)vqa->VQAio;

	/*
	**	Perform the action specified by the stream command.
	*/
	switch (action) {

		/*
		**	VQACMD_READ means read NBytes from the stream and place it in the
		**	memory pointed to by Buffer.
		**
		**	Any error code returned will be remapped by VQA library into
		**	VQAERR_READ.
		*/
		case VQACMD_READ:
			error = (FileHandle.Read(buffer, (unsigned short)nbytes) != (unsigned short)nbytes);
			break;

		/*
		**	VQACMD_WRITE is analogous to VQACMD_READ.
		**
		**	Writing is not allowed to the VQA file, VQA library will remap the
		**	error into VQAERR_WRITE.
		*/
		case VQACMD_WRITE:
			error = 1;
			break;

		/*
		**	VQACMD_SEEK asks that you perform a seek relative to the current
		**	position. NBytes is a signed number, indicating seek direction
		** (positive for forward, negative for backward). Buffer has no meaning
		**	here.
		**
		**	Any error code returned will be remapped by VQA library into
		**	VQAERR_SEEK.
		*/
		case VQACMD_SEEK:
			error = (FileHandle.Seek(nbytes, SEEK_CUR) == -1);
			break;

		/*
		**	VQACMD_OPEN asks that you open your stream for access.
		*/
		case VQACMD_OPEN:
			file = new CCFileClass((char *)buffer);

			if (file != NULL && FileHandle.Is_Available()) {
				error = FileHandle.Open((char *)buffer, FileClass::READ);

				if (error != -1) {
					vqa->VQAio = (unsigned long)file;
					error = 0;
				} else {
					delete file;
					file = 0;
					error = 1;
				}
			} else {
				error = 1;
			}
			break;

		case VQACMD_CLOSE:
			FileHandle.Close();
			delete file;
			file = 0;
			vqa->VQAio = 0;
			error = 0;
			break;

		/*
		**	VQACMD_INIT means to prepare your stream for reading. This is used for
		**	certain streams that can't be read immediately upon opening, and need
		**	further preparation. This operation is allowed to fail; the error code
		**	will be returned directly to the client.
		*/
		case VQACMD_INIT:

		/*
		**	IFFCMD_CLEANUP means to terminate the transaction with the associated
		**	stream. This is used for streams that can't simply be closed. This
		**	operation is not allowed to fail; any error returned will be ignored.
		*/
		case VQACMD_CLEANUP:
			error = 0;
			break;

		default:
			error = 0;
			break;
	}

	return(error);
}
#endif
