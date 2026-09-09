/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uisound.h"

#include <utility>


UISoundPresenterClass::UISoundPresenterClass(UISoundServiceClass & service, UISoundState state) :
	State(std::move(state)),
	Service(service)
{
}


static int Clamp_Level(int level)
{
	if (level < 0) {
		return(0);
	}
	if (level > UISoundPresenterClass::LEVELS) {
		return(UISoundPresenterClass::LEVELS);
	}
	return(level);
}


void UISoundPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "score") {
		State.Score = Clamp_Level(intent.Value);
		Service.Set_Score_Volume(Volume_Of(State.Score), true);

	} else if (intent.Name == "sound") {
		State.Sound = Clamp_Level(intent.Value);
		Service.Set_Sound_Volume(Volume_Of(State.Sound), true);

	} else if (intent.Name == "voice") {
		State.Voice = Clamp_Level(intent.Value);
		Service.Set_Voice_Volume(Volume_Of(State.Voice), true);

	} else if (intent.Name == "shuffle") {
		State.Shuffle = (intent.Value != 0);
		Service.Set_Shuffle(State.Shuffle);
		if (State.Shuffle) {
			State.Repeat = false;
			Service.Set_Repeat(false);
		}

	} else if (intent.Name == "repeat") {
		State.Repeat = (intent.Value != 0);
		Service.Set_Repeat(State.Repeat);
		if (State.Repeat) {
			State.Shuffle = false;
			Service.Set_Shuffle(false);
		}

	} else if (intent.Name == "select") {
		State.Selected = (intent.Value >= 0 && intent.Value < (int)State.Tracks.size()) ? intent.Value : -1;

	} else if (intent.Name == "play") {
		if (State.Selected >= 0 && State.Selected < (int)State.Tracks.size()) {
			Service.Play(State.Tracks[State.Selected].Theme);
		}

	} else if (intent.Name == "stop") {
		Service.Stop();

	} else if (intent.Name == "ok") {
		Service.Set_Score_Volume(Volume_Of(State.Score), false);
		Service.Set_Sound_Volume(Volume_Of(State.Sound), false);
		Service.Set_Voice_Volume(Volume_Of(State.Voice), false);
		Result = UI_RESULT_ACCEPTED;

	} else if (intent.Name == "cancel") {
		Result = UI_RESULT_ACCEPTED;
	}
}


void UISoundPresenterClass::Refresh(void)
{
}


int UISoundPresenterClass::Level_Of(float volume)
{
	return(Clamp_Level((int)(volume * (float)LEVELS + 0.5f)));
}


float UISoundPresenterClass::Volume_Of(int level)
{
	return((float)Clamp_Level(level) / (float)LEVELS);
}
