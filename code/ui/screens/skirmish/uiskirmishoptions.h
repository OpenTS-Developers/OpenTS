/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ui/screens/skirmish/uiskirmish.h"
#include "ui/uiscreen.h"

#include <memory>

class UIViewClass;


// Edits the game rules and economy settings of a skirmish setup in place. The state's other
// fields are left untouched.
class UISkirmishOptionsPresenterClass : public UIPresenterClass
{
	public:
		explicit UISkirmishOptionsPresenterClass(UISkirmishState & state);
		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UISkirmishState & State;
		bool Accepted = false;
};


std::unique_ptr<UIViewClass> UI_Skirmish_Options_View(UISkirmishOptionsPresenterClass & presenter);
