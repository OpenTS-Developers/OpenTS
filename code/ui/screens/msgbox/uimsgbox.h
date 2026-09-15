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


// One button of a message box in its display position: slot 0 is the left, 1 the middle and
// 2 the right one. Index is the legacy button number the box returns for it.
struct UIMessageButton
{
	std::string Caption;
	int Index = 0;
	int Slot = 0;
};


// Shows a message with up to three buttons and reports which one answered. The captions
// arrive in the legacy order, first to third, and an empty caption leaves its button out.
class UIMessageBoxPresenterClass : public UIPresenterClass
{
	public:
		UIMessageBoxPresenterClass(std::string text, std::vector<std::string> captions, int defaultresponse);

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		int Button_Count(void) const { return((int)Buttons.size()); }

		std::string Text;
		std::vector<UIMessageButton> Buttons;
		int Default;
		// The network box is its own shape, wider and taller than the menu's.
		bool Network = false;
		// The legacy return value: the button number, or -1 while nothing has answered.
		int Choice = -1;
};


// The RmlUi view over a message box presenter, bound to message.rml. The presenter must
// outlive it.
std::unique_ptr<UIViewClass> UI_Message_Box_View(UIMessageBoxPresenterClass & presenter);

// Runs a message box, returning the number of the button pressed, or -1 when the session ended
// or the screen could not open.
int UI_Message_Box(char const * text, int defaultresponse, char const * b1, char const * b2, char const * b3);

// Runs the network message box, polling the caller's idle routine each pass so that its answers
// keep flowing while the box is up. The type is the MB_ layout the Win32 box took, and the
// answer comes back as the control id that gave it, as ODMessageBox reported it.
int UI_Network_Message_Box(char const * text, int type, bool (*idle)(void));
