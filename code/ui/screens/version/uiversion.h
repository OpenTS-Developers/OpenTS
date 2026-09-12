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
#include <vector>

class UIViewClass;


// Shows fixed lines and closes. The lines are handed in, so the presenter needs no engine state.
class UIVersionPresenterClass : public UIPresenterClass
{
	public:
		explicit UIVersionPresenterClass(std::vector<std::string> lines);

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		std::vector<std::string> Lines;
};


// The RmlUi view over a version presenter, bound to version.rml. The presenter must outlive it.
std::unique_ptr<UIViewClass> UI_Version_View(UIVersionPresenterClass & presenter);

// The lines the version dialog shows, gathered from the running game.
void UI_Version_Lines(std::vector<std::string> & lines);

// Runs the version dialog as an RmlUi screen. False means it could not be prepared and the
// caller should open its legacy dialog.
bool UI_Version_Dialog(void);
