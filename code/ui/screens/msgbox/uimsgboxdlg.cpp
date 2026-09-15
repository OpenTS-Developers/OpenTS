/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the message box screen: the entry WWMessageBox::Process calls. The
// presenter and view live in uimsgbox.cpp so that the test harness can drive them without
// the engine.

#include "ui/screens/msgbox/uimsgbox.h"

#include "_ui.h"
#include "ui/uienginehost.h"
#include "ui/uishell.h"
#include "ui/uiview.h"
#include "win.h"

#include <utility>


int UI_Message_Box(char const * text, int defaultresponse, char const * b1, char const * b2, char const * b3)
{
	std::vector<std::string> captions;
	captions.push_back((b1 != NULL) ? b1 : "");
	captions.push_back((b2 != NULL) ? b2 : "");
	captions.push_back((b3 != NULL) ? b3 : "");

	UIMessageBoxPresenterClass presenter((text != NULL) ? text : "", std::move(captions), defaultresponse);

	// The Win32 box with no buttons is ended as soon as it is shown and answers 0.
	if (presenter.Button_Count() == 0) {
		return(0);
	}

	std::unique_ptr<UIViewClass> view = UI_Message_Box_View(presenter);
	UIResult result = UI_Run_Modal(*view);

	if (result == UI_RESULT_SESSION_ENDED || result == UI_RESULT_FAILED_TO_OPEN) {
		return(-1);
	}

	return(presenter.Choice);
}


int UI_Network_Message_Box(char const * text, int type, bool (*idle)(void))
{
	// The Win32 templates carry these captions as literals rather than string table entries,
	// so the box reads the same in every language; the screen matches what it replaces.
	std::vector<std::string> captions;
	int ids[2] = { IDOK, IDCANCEL };

	if (type == MB_YESNO) {
		captions.push_back("No");
		captions.push_back("Yes");
		ids[0] = IDNO;
		ids[1] = IDYES;
	} else if (type == MB_OKCANCEL) {
		captions.push_back("OK");
		captions.push_back("Cancel");
	} else {
		captions.push_back("OK");
	}
	captions.resize(3);

	UIMessageBoxPresenterClass presenter((text != NULL) ? text : "", std::move(captions), 0);
	presenter.Network = true;

	std::unique_ptr<UIViewClass> view = UI_Message_Box_View(presenter);

	UIResult result = UIShell.Run_Modal(*view, [idle](void) {
		bool ended = UI_Service_Game();
		if (idle != NULL && idle()) {
			ended = true;
		}
		return(ended);
	});

	if (result == UI_RESULT_SESSION_ENDED || result == UI_RESULT_FAILED_TO_OPEN) {
		return(IDCANCEL);
	}

	if (presenter.Choice >= 0 && presenter.Choice < 2) {
		return(ids[presenter.Choice]);
	}
	return(IDCANCEL);
}
