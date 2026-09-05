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

#pragma once

//==========================================================================
// INCLUDES
//==========================================================================

#include "ccfile.h"

#include <vqaplay.h>

//==========================================================================
// PUBLIC FUNCTIONS
//==========================================================================

//==========================================================================
// PRIVATE DEFINES
//==========================================================================

//--------------------------------------------------------------------------
// GENERAL DEFINES
//--------------------------------------------------------------------------
typedef void * (*VQA_SURF_LOCK_CALLBACK) (void);
typedef bool (*VQA_SURF_UNLOCK_CALLBACK) (void);
typedef void (*VQA_SURF_DRAW_CALLBACK) (void);

#define SIZE_OF_PALETTE				256

// CODES for Play_VQA_Frame:
#define PLAY_LAST_FRAME			-1

// Error codes returned by VQA_INIT:
#define VQA_INIT_NO_ERROR				-1
#define VQA_INIT_FAILED_MEM_POOL_ALLOC	-2
#define VQA_INIT_FAILED_SCRATCH_ALLOC	-3

// Codes for Cache_VQA:
#define CACHE_REST_OF_FILE		0

// Error codes returned by CACHE_VQA
#define CACHE_NO_ERROR			-1
#define CACHE_EOF				-2
#define CACHE_FAILED_MEM_ALLOC	-3
#define CACHE_OPEN_FILE_ERROR	-4
#define CACHE_READ_ERROR		-5

// Error codes returned by Play_VQA:
#define VQA_PLAY_NO_ERROR		0
#define VQA_PLAY_USER_BREAK		1

#define VECTOR_FORMAT_OFFSETS	1
#define VECTOR_FORMAT_INDEXES	2
#define MAX_CODEBOOK_OFFSET		32766

enum {
	VQACF_PLAY_FROM_MIXFILE = 1 << 0,
	VQACF_2 = 1 << 1,
	VQACF_4 = 1 << 2,
};

//==========================================================================
// PUBLIC DATA
//==========================================================================

//==========================================================================
// CLASSES
//==========================================================================


//==========================================================================
// TYPES
//==========================================================================

struct VQACacheHeader {
	unsigned long	file_size;
	unsigned long	file_offset;
	unsigned long	file_buffer_pos;
	unsigned char	*file_buffer;
};

class VQAClass
{
	private:
		CCFileClass FileHandle;
		bool IsFileOpen;
		char Filename[MAX_PATH];
		VQAConfig Config;
		VQAHandle * Handle;
		int CurrentFrame;
		int TotalFrames;
		int DesiredColorMode;
		int PrimaryColorMode;
		bool IsOpen;
		int Width;
		int Height;
		int DrawBufferWidth;
		int DrawBufferHeight;
		int Flags;
		VQACacheHeader Cache;
		VQA_SURF_LOCK_CALLBACK SurfaceLockCallback;
		VQA_SURF_UNLOCK_CALLBACK SurfaceUnlockCallback;
		VQA_SURF_DRAW_CALLBACK SurfaceDrawCallback;
		int Event3Frame;
		bool IsPaused;
		bool IsAdvanceReady;

	public:
		VQAClass(char const * filename, int flags, VQA_SURF_LOCK_CALLBACK surface_lock, VQA_SURF_UNLOCK_CALLBACK surface_unlock, VQA_SURF_DRAW_CALLBACK surface_draw, int frame_rate = -1, int draw_rate = -1);
		~VQAClass (void);

		bool Open_And_Load_Buffers(void);
		bool Set_Loop(int loop_id, int iterations);
		bool Set_Loop(int start, int end, int iterations);
		void Seek_To_Frame(int frame);
		int Play_VQA(int last_frame_to_play, bool breakout);
		void Play_VQA_Frame(int frame_number);
		bool Advance_Frame(bool &);
		bool Redraw_Paused_Frame(void);
		void Pause_VQA(void);
		void Resume_VQA(void);
		void Close_And_Free_VQA(void);
		void Reset_VQA(void);
		bool Set_Draw_Buffer(void * buffer, int buffer_width, int buffer_height, int x_offset = 0, int y_offset = 0);
		long Cache_VQA(unsigned long bytes_to_cache);

		int Get_Desired_Color_Mode(void);
		void Set_Primary_Color_Mode(int mode);

		int Get_VQA_Width(void) const;
		int Get_VQA_Height(void) const;
		int Get_VQA_Version(void) const;

		int Set_VQA_Volume(int vol);

		bool Seek_To_Stored_Frame(void);

		void Handle_Codebook_Event(void *buf, unsigned int num);
		void *Handle_Lock_Event(void);
		bool Handle_Unlock_Event(void);

		bool Is_Paused(void) const { return(IsPaused); }

		long CCFileHandler(long action, void * buffer, long nbytes);
		long MixFileHandler(long action, void * buffer, long nbytes);
		long CacheHandler(long action, void * buffer, long nbytes);

		/*=========================================================================*/
		/* Private functions.                                                      */
		/*=========================================================================*/
	private:

}; /* VQAClass */

bool VQA_Message_Handler(void);
