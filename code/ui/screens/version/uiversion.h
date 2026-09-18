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


class UIVersionPresenterClass : public UIPresenterClass
{
	public:
		explicit UIVersionPresenterClass(std::vector<std::string> lines);
		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		std::vector<std::string> Lines;
};


std::unique_ptr<UIViewClass> UI_Version_View(UIVersionPresenterClass & presenter);

void UI_Version_Lines(std::vector<std::string> & lines);

void UI_Version_Dialog(void);
