/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "mssfx.h"

#include "_mixfile.h"
#include "ccfile.h"
#include "data.h"
#include "dbgprint.h"
#include "audio/audioengine.h"
#include "mixfile.h"


/// <summary>
/// Creates a sound effect for the score screen.
/// This routine will fetch the sample out of the mixfiles and, failing that, load it from
/// disk into a buffer this object owns. Nothing is fetched at all when the audio system is
/// unavailable, leaving the effect silent.
/// </summary>
/// <param name="name">The name to identify this sound effect by.</param>
/// <param name="file_name">The name of the sample file to play.</param>
MSSfx::MSSfx(char const * name, char const * file_name) :
	Name(NULL),
	Sample(NULL),
	Loaded(false)
{
	if (AudioEngine.Is_Available()) {

		Name = strdup(name);
		Sample = (void *)MFCD::Retrieve(file_name);

		if (Sample == NULL) {
			CCFileClass file(file_name);
			Sample = Load_Alloc_Data(file);
			Loaded = true;
			file.Close();
			DebugString("ScoreScreen: Loaded %s\n", file_name);
		}
	}
}


/// <summary>
/// Stops the sound effect and frees it.
/// This routine will silence the sample first, since the buffer it is playing out of is
/// about to disappear from underneath the audio system.
/// </summary>
MSSfx::~MSSfx(void)
{
	if (Sample != NULL) {
		AudioEngine.Stop_Sample_Playing(Sample);
		AudioEngine.Release_Sample(Sample);
		if (Loaded == true) {
			delete Sample;
		}
	}

	if (Name != NULL) {
		free(Name);
	}
}


/// <summary>
/// Plays the sound effect.
/// The request is quietly ignored when the sample never loaded, so the caller does not
/// have to concern itself with whether the audio system was available.
/// </summary>
void MSSfx::Play(int volume)
{
	if (Sample != NULL) {
		AudioEngine.Play_Sample(Sample, AUDIO_GROUP_SFX, (float)volume / 255.0f, 255);
	}
}
