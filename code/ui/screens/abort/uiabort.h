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
#include <string>

class UIViewClass;


enum UIAbortChoice
{
	UI_ABORT_CONTINUE,
	UI_ABORT_QUIT,
	UI_ABORT_RESTART,
};


struct UIAbortState
{
	std::string RestartCaption;
	bool RestartEnabled = true;
};


class UIAbortPresenterClass : public UIPresenterClass
{
	public:
		explicit UIAbortPresenterClass(UIAbortState state);
		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIAbortState State;
		UIAbortChoice Choice = UI_ABORT_CONTINUE;
};


std::unique_ptr<UIViewClass> UI_Abort_View(UIAbortPresenterClass & presenter);

void UI_Abort_State(UIAbortState & state);

UIAbortChoice UI_Abort_Dialog(void);
