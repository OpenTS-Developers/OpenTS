/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the button menus over the title screen: where they sit and the entry
// each menu driver calls ahead of its Win32 dialog. The presenter and view live in
// uimenu.cpp so that the test harness can drive them without the engine.

#include "ui/screens/menu/uimenu.h"

#include "_surface.h"
#include "_ui.h"
#include "init.h"
#include "surface.h"
#include "ui/uienginehost.h"
#include "ui/uishell.h"
#include "ui/uiview.h"


// A menu sits where the title screen's buttons are: a 400 pixel layout centered in the
// frame, with the buttons 147 pixels down it.
void UI_Menu_Place(UIMenuState & state)
{
	state.Top = (HiddenSurface != NULL) ? (HiddenSurface->Get_Height() - 400) / 2 + 147 : -1;
}


bool UI_Menu_Dialog(UIMenuState const & state, int & choice, std::function<bool(void)> const & hook)
{
	choice = 0;

	if (UIShell.Legacy_Dialog_Visible()) {
		return(false);
	}

	UIMenuPresenterClass presenter(state);
	std::unique_ptr<UIViewClass> view = UI_Menu_View(presenter);

	UIResult result = UIShell.Run_Modal(*view, [&hook]() {
		bool ended = UI_Service_Game();
		Title_Screen_Restore();
		if (hook && hook()) {
			ended = true;
		}
		return(ended);
	});

	if (result == UI_RESULT_FAILED_TO_OPEN) {
		return(false);
	}

	choice = (result == UI_RESULT_ACCEPTED) ? presenter.Choice : 0;
	return(true);
}
