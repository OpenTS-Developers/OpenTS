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

#include <functional>
#include <memory>
#include <string>
#include <vector>

class UIViewClass;


enum UIMenuKindType
{
	UI_MENU_MAIN,
	UI_MENU_MULTIPLAYER,
	UI_MENU_MULTIPLAYER_FIRESTORM,
	UI_MENU_GAME_TYPE,
};


struct UIMenuItemType
{
	std::string Label;
	int Choice = 0;
	bool Enabled = true;
};


struct UIMenuState
{
	UIMenuKindType Kind = UI_MENU_MAIN;
	std::string Title;

	bool Wide = true;
	std::vector<UIMenuItemType> Items;
	int Top = -1;
};


class UIMenuPresenterClass : public UIPresenterClass
{
	public:
		explicit UIMenuPresenterClass(UIMenuState state);
		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIMenuState State;

		int Choice = 0;
};


std::unique_ptr<UIViewClass> UI_Menu_View(UIMenuPresenterClass & presenter);

void UI_Menu_Place(UIMenuState & state);

int UI_Menu_Dialog(UIMenuState const & state, int nothing, std::function<bool(void)> const & hook = nullptr);
