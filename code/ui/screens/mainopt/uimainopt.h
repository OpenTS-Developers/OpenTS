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


// What the options menu answers with: the dialog to open next, or the way back to the menu.
enum UIMainOptionsChoice
{
	UI_MAIN_OPTIONS_LEAVE,
	UI_MAIN_OPTIONS_SETTINGS,
	UI_MAIN_OPTIONS_DISPLAY,
	UI_MAIN_OPTIONS_SOUND,
	UI_MAIN_OPTIONS_KEYBOARD,
};


// What the menu shows: whether the Sound button is live, and the top edge the menu sits at
// in the frame, or -1 to sit in the middle.
struct UIMainOptionsState
{
	bool SoundEnabled = false;
	int Top = -1;
};


// A leaf of buttons: each press closes the menu with its choice. Enter and Escape leave the
// way the Main Menu button does, and a Sound button without an audio device does nothing.
class UIMainOptionsPresenterClass : public UIPresenterClass
{
	public:
		explicit UIMainOptionsPresenterClass(UIMainOptionsState state);

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIMainOptionsState State;
		UIMainOptionsChoice Choice = UI_MAIN_OPTIONS_LEAVE;
};


// The RmlUi view over an options menu presenter, bound to mainopt.rml. The presenter must
// outlive it.
std::unique_ptr<UIViewClass> UI_Main_Options_View(UIMainOptionsPresenterClass & presenter);

// The state of the running game.
void UI_Main_Options_State(UIMainOptionsState & state);

// Runs the options menu as an RmlUi screen. False means it could not run as one and the
// caller should open its Win32 dialog; otherwise choice carries the player's pick.
bool UI_Main_Options_Dialog(UIMainOptionsChoice & choice);
