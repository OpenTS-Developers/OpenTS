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

class UIViewClass;


enum UIMainOptionsChoice
{
	UI_MAIN_OPTIONS_LEAVE,
	UI_MAIN_OPTIONS_SETTINGS,
	UI_MAIN_OPTIONS_DISPLAY,
	UI_MAIN_OPTIONS_SOUND,
	UI_MAIN_OPTIONS_KEYBOARD,
};


struct UIMainOptionsState
{
	bool SoundEnabled = false;
	int Top = -1;
};


class UIMainOptionsPresenterClass : public UIPresenterClass
{
	public:
		explicit UIMainOptionsPresenterClass(UIMainOptionsState state);
		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIMainOptionsState State;
		UIMainOptionsChoice Choice = UI_MAIN_OPTIONS_LEAVE;
};


std::unique_ptr<UIViewClass> UI_Main_Options_View(UIMainOptionsPresenterClass & presenter);

void UI_Main_Options_State(UIMainOptionsState & state);

UIMainOptionsChoice UI_Main_Options_Dialog(void);
