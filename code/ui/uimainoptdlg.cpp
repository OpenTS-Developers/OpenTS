/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the options menu: the state it starts from and the entry the menu
// driver calls ahead of its Win32 dialog. The presenter and view live in uimainopt.cpp so
// that the test harness can drive them without the engine.

#include "ui/uimainopt.h"

#include "_surface.h"
#include "audio/audioengine.h"
#include "surface.h"
#include "ui/uirmlview.h"
#include "ui/uishell.h"


// The menu sits where the main menu's buttons were: a 400 pixel layout centred in the
// frame, with the buttons 147 pixels down it.
void UI_Main_Options_State(UIMainOptionsState & state)
{
	state = UIMainOptionsState();
	state.SoundEnabled = AudioEngine.Is_Available();
	if (HiddenSurface != NULL) {
		state.Top = (HiddenSurface->Get_Height() - 400) / 2 + 147;
	}
}


bool UI_Main_Options_Dialog(UIMainOptionsChoice & choice)
{
	choice = UI_MAIN_OPTIONS_LEAVE;

	if (UI_Legacy_Dialog_Visible()) {
		return(false);
	}

	UIMainOptionsState state;
	UI_Main_Options_State(state);

	UIMainOptionsPresenterClass presenter(state);
	std::unique_ptr<UIRmlViewClass> view = UI_Main_Options_View(presenter);

	UIResult result = UI_Run_Modal(*view);
	if (result == UI_RESULT_FAILED_TO_OPEN) {
		return(false);
	}

	choice = (result == UI_RESULT_ACCEPTED) ? presenter.Choice : UI_MAIN_OPTIONS_LEAVE;
	return(true);
}
