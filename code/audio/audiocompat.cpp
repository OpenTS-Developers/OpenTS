/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "audio/audiocompat.h"

namespace {

// The old fade counted maintenance ticks at 40 Hz.
int const FADE_TICK_MS = 25;


AudioHandle From_Int(int handle)
{
	return(handle < 0 ? AudioHandle() : AudioHandle::From_Raw((uint32_t)handle));
}


int To_Int(AudioHandle handle)
{
	return(handle.Is_Null() ? INVALID_SAMPLE_HANDLE : (int)handle.Raw());
}


float Level(int volume)
{
	if (volume < 0) volume = 0;
	if (volume > 255) volume = 255;
	return((float)volume / 255.0f);
}

} // namespace


DSAudio::DSAudio(void) :
	Audio_Focus_Loss_Function(nullptr),
	StreamLowImpact(false),
	MasterVolume(255)
{
}


bool DSAudio::Init(void *, int, bool, int)
{
	MasterVolume = 255;
	return(AudioEngine.Init());
}


void DSAudio::End(void)
{
	AudioEngine.End();
}


int DSAudio::File_Stream_Sample(char const * filename, bool real_time_start)
{
	return(File_Stream_Sample_Vol(filename, 0xFF, real_time_start));
}


int DSAudio::File_Stream_Sample_Vol(char const * filename, int volume, bool)
{
	return(To_Int(AudioEngine.Open_Stream(filename, AUDIO_GROUP_MUSIC, Level(volume), false)));
}


void DSAudio::Sound_Callback(void)
{
	AudioEngine.Sound_Callback();
}


void DSAudio::Stop_Sample(int handle)
{
	AudioEngine.Events().Stop(From_Int(handle));
}


bool DSAudio::Sample_Status(int handle)
{
	return(AudioEngine.Events().Is_Playing(From_Int(handle)));
}


bool DSAudio::Is_Sample_Playing(void const * sample)
{
	return(AudioEngine.Is_Sample_Playing(sample));
}


void DSAudio::Stop_Sample_Playing(void const * sample)
{
	AudioEngine.Stop_Sample_Playing(sample);
}


int DSAudio::Play_Sample(void const * sample, int priority, int volume)
{
	if (volume <= 0) {
		return(INVALID_SAMPLE_HANDLE);
	}
	return(To_Int(AudioEngine.Play_Sample(sample, AUDIO_GROUP_SFX, Level(volume), priority)));
}


void DSAudio::Fade_Sample(int handle, int ticks)
{
	if (ticks <= 0) {
		AudioEngine.Events().Stop(From_Int(handle));
	} else {
		AudioEngine.Events().Fade(From_Int(handle), ticks * FADE_TICK_MS);
	}
}


bool DSAudio::Start_Primary_Sound_Buffer(bool)
{
	AudioEngine.Focus_Restore();
	return(AudioEngine.Is_Available());
}


void DSAudio::Stop_Primary_Sound_Buffer(void)
{
	AudioEngine.Focus_Loss();
}


void DSAudio::Set_Volume_All(int volume)
{
	if (volume < 0) volume = 0;
	if (volume > 255) volume = 255;
	MasterVolume = volume;
	AudioEngine.Set_Master_Gain(Level(volume), (int)(AUDIO_DUCK_RAMP_SECONDS * 1000.0f));
}


int DSAudio::Adjust_Volume_All(int percent)
{
	int previous = MasterVolume;
	Set_Volume_All(MasterVolume * percent / 100);
	return(previous);
}


void DSAudio::Set_Handle_Volume(int handle, int volume)
{
	AudioEngine.Events().Set_Volume(From_Int(handle), Level(volume));
}


void DSAudio::Set_Sample_Volume(void const * sample, int volume)
{
	AudioEngine.Set_Sample_Volume(sample, Level(volume));
}
