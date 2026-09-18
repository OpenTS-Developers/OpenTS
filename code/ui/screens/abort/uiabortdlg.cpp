/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/abort/uiabort.h"

#include "_ui.h"
#include "data.h"
#include "globals.h"
#include "house.h"
#include "language/language.h"
#include "session.h"
#include "ui/uienginehost.h"
#include "ui/uishell.h"
#include "ui/uiview.h"


void UI_Abort_State(UIAbortState & state)
{
	state = UIAbortState();

	if (Session.Type != GAME_NORMAL) {
		state.RestartCaption = Fetch_String(TXT_SURRENDER);
		if (PlayerPtr != NULL) {
			state.RestartEnabled = !(PlayerPtr->IsDefeated || PlayerPtr->IsToWin
				|| PlayerPtr->IsToLose || PlayerPtr->IsToDie);
		}
	} else {
		state.RestartCaption = "Restart";
	}
}


UIAbortChoice UI_Abort_Dialog(void)
{
	UIAbortState state;
	UI_Abort_State(state);

	UIAbortPresenterClass presenter(state);
	std::unique_ptr<UIViewClass> view = UI_Abort_View(presenter);

	if (UI_Run_Modal(*view) != UI_RESULT_ACCEPTED) {
		return(UI_ABORT_CONTINUE);
	}

	return(presenter.Choice);
}
