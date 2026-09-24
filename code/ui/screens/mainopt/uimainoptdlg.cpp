/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "_surface.h"
#include "_ui.h"
#include "audio/audioengine.h"
#include "surface.h"
#include "ui/screens/mainopt/uimainopt.h"
#include "ui/uienginehost.h"
#include "ui/uishell.h"
#include "ui/uiview.h"


void UI_Main_Options_State(UIMainOptionsState & state)
{
	state = UIMainOptionsState();
	state.SoundEnabled = AudioEngine.Is_Available();
	if (HiddenSurface != NULL) {
		state.Top = (HiddenSurface->Get_Height() - 400) / 2 + 147;
	}
}


UIMainOptionsChoice UI_Main_Options_Dialog(void)
{
	UIMainOptionsState state;
	UI_Main_Options_State(state);

	UIMainOptionsPresenterClass presenter(state);
	std::unique_ptr<UIViewClass> view = UI_Main_Options_View(presenter);

	if (UI_Run_Modal(*view) != UI_RESULT_ACCEPTED) {
		return(UI_MAIN_OPTIONS_LEAVE);
	}

	return(presenter.Choice);
}
