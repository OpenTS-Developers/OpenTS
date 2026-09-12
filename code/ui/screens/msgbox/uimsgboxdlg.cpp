/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the message box screen: the entry WWMessageBox::Process calls ahead of
// its Win32 box. The presenter and view live in uimsgbox.cpp so that the test harness can
// drive them without the engine.

#include "ui/screens/msgbox/uimsgbox.h"

#include "ui/rml/rmlview.h"
#include "ui/uishell.h"

#include <utility>


bool UI_Message_Box(char const * text, int defaultresponse, char const * b1, char const * b2, char const * b3, int & choice)
{
	std::vector<std::string> captions;
	captions.push_back((b1 != NULL) ? b1 : "");
	captions.push_back((b2 != NULL) ? b2 : "");
	captions.push_back((b3 != NULL) ? b3 : "");

	UIMessageBoxPresenterClass presenter((text != NULL) ? text : "", std::move(captions), defaultresponse);

	// The Win32 box with no buttons is ended as soon as it is shown and answers 0.
	if (presenter.Button_Count() == 0) {
		choice = 0;
		return(true);
	}

	// A visible Win32 dialog takes the mouse before a document can, so a box over one stays Win32.
	if (UI_Legacy_Dialog_Visible()) {
		return(false);
	}

	std::unique_ptr<UIRmlViewClass> view = UI_Message_Box_View(presenter);
	UIResult result = UI_Run_Modal(*view);

	if (result == UI_RESULT_FAILED_TO_OPEN) {
		return(false);
	}

	choice = (result == UI_RESULT_SESSION_ENDED) ? -1 : presenter.Choice;
	return(true);
}
