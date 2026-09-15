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


// What the abort question answers with.
enum UIAbortChoice
{
	UI_ABORT_CONTINUE,
	UI_ABORT_QUIT,
	UI_ABORT_RESTART,
};


// What the question shows: the middle button's caption, which is a surrender outside a
// campaign mission, and whether that button takes a press at all.
struct UIAbortState
{
	std::string RestartCaption;
	bool RestartEnabled = true;
};


// A leaf of three buttons: aborting and restarting each close the question with their choice,
// and cancelling, Enter and Escape all carry on playing, as the dialog's IDOK and IDCANCEL do.
class UIAbortPresenterClass : public UIPresenterClass
{
	public:
		explicit UIAbortPresenterClass(UIAbortState state);

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIAbortState State;
		UIAbortChoice Choice = UI_ABORT_CONTINUE;
};


// The RmlUi view over an abort presenter, bound to abort.rml. The presenter must outlive it.
std::unique_ptr<UIViewClass> UI_Abort_View(UIAbortPresenterClass & presenter);

// What the question shows in the running game.
void UI_Abort_State(UIAbortState & state);

// Runs the abort question, returning the player's answer. Backing out and a screen that could
// not open both carry on playing.
UIAbortChoice UI_Abort_Dialog(void);
