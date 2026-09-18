/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

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


bool UI_Network_Message_Box(char const * text, UINetworkMessageButtons buttons, bool (*idle)(void))
{
	if (text == NULL || text[0] == '\0') {
		return(false);
	}

	std::vector<std::string> captions;
	int accepted = 0;

	if (buttons == UI_NETWORK_MESSAGE_YES_NO) {
		captions.push_back("No");
		captions.push_back("Yes");
		accepted = 1;
	} else if (buttons == UI_NETWORK_MESSAGE_OK_CANCEL) {
		captions.push_back("OK");
		captions.push_back("Cancel");
	} else {
		captions.push_back("OK");
	}
	captions.resize(3);

	UIMessageBoxPresenterClass presenter(text, std::move(captions), 0);
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
		return(false);
	}

	return(presenter.Choice == accepted);
}
