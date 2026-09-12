/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ui/uiscreen.h"

#include <memory>
#include <string>
#include <vector>

class UIViewClass;


// The engine calls the sound options make. The game supplies one that reaches the options
// and the music player; the test harness supplies one that records the calls.
class UISoundServiceClass
{
	public:
		virtual ~UISoundServiceClass(void) = default;

		virtual void Set_Score_Volume(float volume, bool feedback) = 0;
		virtual void Set_Sound_Volume(float volume, bool feedback) = 0;
		virtual void Set_Voice_Volume(float volume, bool feedback) = 0;
		virtual void Set_Shuffle(bool on) = 0;
		virtual void Set_Repeat(bool on) = 0;
		virtual void Play(int theme) = 0;
		virtual void Stop(void) = 0;
};


// One row of the track list: its label as the dialog prints it and the theme it plays.
struct UISoundTrack
{
	std::string Label;
	int Theme = 0;
};


// What the dialog shows: the three volumes as slider levels of 0 to 10, the two playlist
// switches, whether the audio device is there, whether a game is running, the tracks the
// playlist allows, and the row of the one that is playing.
struct UISoundState
{
	int Score = 0;
	int Sound = 0;
	int Voice = 0;
	bool Shuffle = false;
	bool Repeat = false;
	bool Enabled = false;
	bool InGame = false;
	std::vector<UISoundTrack> Tracks;
	int Selected = -1;
};


// Applies each change as it arrives, the way the dialog always has: a slider level previews
// at once, shuffle and repeat switch each other off, play and stop reach the music player,
// and closing re-applies the levels without feedback. Nothing is reverted on close.
class UISoundPresenterClass : public UIPresenterClass
{
	public:
		enum {
			LEVELS = 10
		};

		UISoundPresenterClass(UISoundServiceClass & service, UISoundState state);

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		static int Level_Of(float volume);
		static float Volume_Of(int level);

		UISoundState State;

	private:
		UISoundServiceClass & Service;
};


// The RmlUi view over a sound presenter, bound to sound.rml. The presenter must outlive it.
std::unique_ptr<UIViewClass> UI_Sound_View(UISoundPresenterClass & presenter);

// The game's service and the state of the running game, shared by the Win32 dialog and the
// RmlUi view.
UISoundServiceClass & UI_Sound_Service(void);
void UI_Sound_State(UISoundState & state);

// Runs the sound options as an RmlUi screen. False means it could not run as one and the
// caller should open its Win32 dialog.
bool UI_Sound_Dialog(void);
