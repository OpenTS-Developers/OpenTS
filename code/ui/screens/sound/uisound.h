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


struct UISoundTrack
{
	std::string Label;
	int Theme = 0;
};


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


std::unique_ptr<UIViewClass> UI_Sound_View(UISoundPresenterClass & presenter);

UISoundServiceClass & UI_Sound_Service(void);
void UI_Sound_State(UISoundState & state);

void UI_Sound_Dialog(void);
