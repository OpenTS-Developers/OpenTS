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


// Where a game tooltip may sit, in overlay pixels.
struct UIGameTipPlacement
{
	bool Beside = false;	// beside the control, rather than under the pointer
	bool Upright = false;	// the control is taller than it is wide
	float PointerX = 0.0f;
	float PointerY = 0.0f;
	float Below = 0.0f;	// how far under the pointer the box starts
	float ControlLeft = 0.0f;
	float ControlTop = 0.0f;
	float ControlRight = 0.0f;
	float ControlBottom = 0.0f;
	float Left = 0.0f;	// the edges the box stays inside
	float Top = 0.0f;
	float Right = 0.0f;
	float Bottom = 0.0f;
};


class UIGameTipPresenterClass : public UIPresenterClass
{
	public:
		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		std::string Text;
		UIGameTipPlacement Placement;
		bool Wanted = false;
};


std::string UI_Game_Tip_Text(std::string const & text);
void UI_Place_Game_Tip(UIGameTipPlacement const & placement, float width, float height, float & x, float & y);
std::unique_ptr<UIViewClass> UI_Game_Tip_View(UIGameTipPresenterClass & presenter);
