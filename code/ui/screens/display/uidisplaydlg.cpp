/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the display options: the service the presenter drives and the state it
// starts from. The presenters live in uidisplay.cpp so that the test harness can drive them
// against a recording service and a hand-driven clock.

#include "ui/screens/display/uidisplay.h"

#include "globals.h"
#include "goptions.h"
#include "ui/rml/rmlview.h"
#include "ui/uishell.h"
#include "video.h"

#include <cstdio>


namespace
{

enum {
	MIN_WIDTH = 640,
	MIN_HEIGHT = 400,
	MAX_WIDTH = 4096,
	MAX_HEIGHT = 4096
};


class UIDisplayEngineServiceClass : public UIDisplayServiceClass
{
	public:
		virtual void Set_Stretch_Movies(bool on) override
		{
			Options.StretchMovies = on;
		}
};

UIDisplayEngineServiceClass _Service;

}


UIDisplayServiceClass & UI_Display_Service(void)
{
	return(_Service);
}


void UI_Display_State(UIDisplayState & state)
{
	state = UIDisplayState();
	state.StretchMovies = Options.StretchMovies;

	int * modes = EnumDisplayModes(MIN_WIDTH, MIN_HEIGHT, MAX_WIDTH, MAX_HEIGHT);
	if (modes == NULL) {
		return;
	}

	for (int * entry = modes; *entry != 0; entry += 2) {
		UIDisplayMode mode;
		mode.Width = entry[0];
		mode.Height = entry[1];

		char buffer[64];
		std::snprintf(buffer, sizeof(buffer), "%d x %d", mode.Width, mode.Height);
		mode.Label = buffer;

		if (mode.Width == Options.ScreenWidth && mode.Height == Options.ScreenHeight) {
			state.Selected = (int)state.Modes.size();
		}
		state.Modes.push_back(mode);
	}

	delete [] modes;
}


bool UI_Display_Dialog(std::optional<UIDisplayMode> & picked)
{
	picked.reset();

	if (UI_Legacy_Dialog_Visible()) {
		return(false);
	}

	UIDisplayState state;
	UI_Display_State(state);

	UIDisplayPresenterClass presenter(UI_Display_Service(), state);
	std::unique_ptr<UIRmlViewClass> view = UI_Display_View(presenter);

	if (UI_Run_Modal(*view) == UI_RESULT_FAILED_TO_OPEN) {
		return(false);
	}

	picked = presenter.Picked;
	return(true);
}


bool UI_Confirm_Mode_Dialog(bool & kept)
{
	kept = false;

	if (UI_Legacy_Dialog_Visible()) {
		return(false);
	}

	UIConfirmModePresenterClass presenter(UI_Clock());
	std::unique_ptr<UIRmlViewClass> view = UI_Confirm_Mode_View(presenter);

	UIResult result = UI_Run_Modal(*view);
	if (result == UI_RESULT_FAILED_TO_OPEN) {
		return(false);
	}

	kept = (result == UI_RESULT_ACCEPTED);
	return(true);
}
