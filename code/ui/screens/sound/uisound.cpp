/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/sound/uisound.h"

#include "ui/rml/rmlview.h"

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


// A slider reports its starting level as it is seeded, so a level that is already held plays
// no feedback.
void UISoundPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "score") {
		int level = Clamp_Level(intent.Value);
		if (level != State.Score) {
			State.Score = level;
			Service.Set_Score_Volume(Volume_Of(State.Score), true);
		}

	} else if (intent.Name == "sound") {
		int level = Clamp_Level(intent.Value);
		if (level != State.Sound) {
			State.Sound = level;
			Service.Set_Sound_Volume(Volume_Of(State.Sound), true);
		}

	} else if (intent.Name == "voice") {
		int level = Clamp_Level(intent.Value);
		if (level != State.Voice) {
			State.Voice = level;
			Service.Set_Voice_Volume(Volume_Of(State.Voice), true);
		}

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


namespace
{

class UISoundViewClass : public UIRmlViewClass
{
	public:
		explicit UISoundViewClass(UISoundPresenterClass & presenter) :
			UIRmlViewClass(presenter, "sound.rml", "sound"),
			Data(presenter)
		{
		}

		// The model is small, so every field is re-read after each drain.
		virtual void Sync(void) override
		{
			Model.DirtyAllVariables();
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			Rml::StructHandle<UISoundTrack> track = model.RegisterStruct<UISoundTrack>();
			if (!track) {
				return(false);
			}
			track.RegisterMember("label", &UISoundTrack::Label);
			track.RegisterMember("theme", &UISoundTrack::Theme);

			UISoundState & state = Data.State;
			return(model.RegisterArray<std::vector<UISoundTrack>>()
				&& model.Bind("score", &state.Score)
				&& model.Bind("sound", &state.Sound)
				&& model.Bind("voice", &state.Voice)
				&& model.Bind("shuffle", &state.Shuffle)
				&& model.Bind("repeat", &state.Repeat)
				&& model.Bind("enabled", &state.Enabled)
				&& model.Bind("ingame", &state.InGame)
				&& model.Bind("tracks", &state.Tracks)
				&& model.Bind("selected", &state.Selected));
		}

	private:
		UISoundPresenterClass & Data;
};

}


std::unique_ptr<UIRmlViewClass> UI_Sound_View(UISoundPresenterClass & presenter)
{
	return(std::make_unique<UISoundViewClass>(presenter));
}
