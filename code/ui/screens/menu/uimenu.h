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


// Which of the game's button menus is being shown. They differ only in their size and the
// buttons they carry, so one document covers them and a class chooses the shape.
enum UIMenuKindType
{
	UI_MENU_MAIN,
	UI_MENU_MULTIPLAYER,
	UI_MENU_MULTIPLAYER_FIRESTORM,
	UI_MENU_GAME_TYPE,
};


// One button: what it says, what the caller is told when it is pressed, and whether it can
// be pressed at all. A button that cannot is drawn but does nothing, as the dialog left the
// ones leading to the online service.
struct UIMenuItemType
{
	std::string Label;
	int Choice = 0;
	bool Enabled = true;
};


// A menu of buttons over the title screen: which one it is, the caption above the buttons
// where the template carries one, and the top edge it sits at in the frame, or -1 for the
// middle.
struct UIMenuState
{
	UIMenuKindType Kind = UI_MENU_MAIN;
	std::string Title;

	// Whether a button's run is wider than the tile it is drawn from, which decides whether
	// the tile repeats or is sampled from its middle.
	bool Wide = true;
	std::vector<UIMenuItemType> Items;
	int Top = -1;
};


// A leaf: each button closes the menu with its own answer, and Escape closes it with none.
class UIMenuPresenterClass : public UIPresenterClass
{
	public:
		explicit UIMenuPresenterClass(UIMenuState state);

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIMenuState State;

		// What the button the player pressed answers with, or zero while none has been.
		int Choice = 0;
};


// The RmlUi view over a menu presenter, bound to menu.rml. The presenter must outlive it.
std::unique_ptr<UIViewClass> UI_Menu_View(UIMenuPresenterClass & presenter);

// The top edge a menu sits at over the title screen.
void UI_Menu_Place(UIMenuState & state);

// Runs a menu over the title screen, which is restored each pass as the dialog's loop
// restored it. The hook, where one is given, runs after that: it is where the main menu reads
// the keys it answers to, and it ends the menu by returning true. Returns the value the
// pressed button carries, or the caller's "nothing" when the player backed out or the
// screen could not open.
int UI_Menu_Dialog(UIMenuState const & state, int nothing, std::function<bool(void)> const & hook = nullptr);
