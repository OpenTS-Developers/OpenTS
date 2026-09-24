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
#include "init.h"
#include "surface.h"
#include "ui/screens/menu/uimenu.h"
#include "ui/uienginehost.h"
#include "ui/uishell.h"
#include "ui/uiview.h"


void UI_Menu_Place(UIMenuState & state)
{
	state.Top = (HiddenSurface != NULL) ? (HiddenSurface->Get_Height() - 400) / 2 + 147 : -1;
}


int UI_Menu_Dialog(UIMenuState const & state, int nothing, std::function<bool(void)> const & hook)
{
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

	return((result == UI_RESULT_ACCEPTED) ? presenter.Choice : nothing);
}
