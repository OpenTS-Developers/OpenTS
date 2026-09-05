/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The DirectSound-era interface over the engine, kept while the game's callers
// move to the event API. Integer handles are the raw handle value, with -1
// for none, so the callers' comparisons keep working.

#pragma once

#include "audio/audioengine.h"

#define INVALID_SAMPLE_HANDLE -1


class DSAudio
{
	public:
		DSAudio(void);

		// The arguments are ignored: the engine picks its own format.
		bool Init(void * window, int bits_per_sample, bool stereo, int rate);
		void End(void);

		int File_Stream_Sample(char const * filename, bool real_time_start = false);
		int File_Stream_Sample_Vol(char const * filename, int volume, bool real_time_start = false);
		void Sound_Callback(void);
		void Stop_Sample(int handle);
		bool Sample_Status(int handle);
		bool Is_Sample_Playing(void const * sample);
		void Stop_Sample_Playing(void const * sample);
		int Play_Sample(void const * sample, int priority = 0xFF, int volume = 0xFF);
		void Fade_Sample(int handle, int ticks);
		bool Start_Primary_Sound_Buffer(bool forced);
		void Stop_Primary_Sound_Buffer(void);
		void Set_Volume_All(int volume);
		int Adjust_Volume_All(int percent);
		void Set_Handle_Volume(int handle, int volume);
		void Set_Sample_Volume(void const * sample, int volume);

		// Assigned by the window code and never called: device loss is recovered
		// inside the engine.
		void (*Audio_Focus_Loss_Function)(void);

		// Read by nothing since the file streams moved to their own thread.
		bool StreamLowImpact;

	private:
		int MasterVolume;
};

extern DSAudio Audio;


inline bool Audio_Available(void)
{
	return(AudioEngine.Is_Available());
}
