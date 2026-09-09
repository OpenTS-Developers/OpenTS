/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the sound options: the service the presenter drives and the state it
// starts from. The presenter lives in uisound.cpp so that the test harness can drive it
// against a recording service.

#include "ui/uisound.h"

#include "audio/audioengine.h"
#include "globals.h"
#include "goptions.h"
#include "incdec.h"
#include "theme.h"

#include <cstdio>


namespace
{

class UISoundEngineServiceClass : public UISoundServiceClass
{
	public:
		virtual void Set_Score_Volume(float volume, bool feedback) override
		{
			Options.Set_Score_Volume(volume, feedback);
		}

		virtual void Set_Sound_Volume(float volume, bool feedback) override
		{
			Options.Set_Sound_Volume(volume, feedback);
		}

		virtual void Set_Voice_Volume(float volume, bool feedback) override
		{
			Options.Set_Voice_Volume(volume, feedback);
		}

		virtual void Set_Shuffle(bool on) override
		{
			Options.Set_Shuffle(on);
		}

		virtual void Set_Repeat(bool on) override
		{
			Options.Set_Repeat(on);
		}

		virtual void Play(int theme) override
		{
			Theme.Stop();
			Theme.Queue_Song((ThemeType)theme);
		}

		virtual void Stop(void) override
		{
			Theme.Queue_Song(THEME_QUIET);
		}
};

UISoundEngineServiceClass _Service;

}


UISoundServiceClass & UI_Sound_Service(void)
{
	return(_Service);
}


// The track list depends on the scenario and the player's house, so it is built only in game.
void UI_Sound_State(UISoundState & state)
{
	state = UISoundState();

	state.Score = UISoundPresenterClass::Level_Of(Options.ScoreVolume);
	state.Sound = UISoundPresenterClass::Level_Of(Options.SoundVolume);
	state.Voice = UISoundPresenterClass::Level_Of(Options.VoiceVolume);
	state.Shuffle = Options.IsScoreShuffle;
	state.Repeat = Options.IsScoreRepeat;
	state.Enabled = AudioEngine.Is_Available();
	state.InGame = GameActive;

	if (!GameActive) {
		return;
	}

	int visible = 1;
	for (ThemeType index = THEME_FIRST; index < Theme.Max_Themes(); index++) {
		if (!Theme.Is_Allowed(index)) {
			continue;
		}

		int length = Theme.Track_Length(index);
		char const * fullname = Theme.Full_Name(index);
		char buffer[100];
		std::snprintf(buffer, sizeof(buffer), "%02d - %s [%d:%02d]", visible, (fullname != NULL) ? fullname : "", length / 60, length % 60);
		visible++;

		UISoundTrack track;
		track.Label = buffer;
		track.Theme = index;
		if (Theme.What_Is_Playing() == index) {
			state.Selected = (int)state.Tracks.size();
		}
		state.Tracks.push_back(track);
	}

	if (state.Selected < 0 && !state.Tracks.empty()) {
		state.Selected = 0;
	}
}
