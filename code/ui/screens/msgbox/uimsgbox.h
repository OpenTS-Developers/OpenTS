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


struct UIMessageButton
{
	std::string Caption;
	int Index = 0;
	int Slot = 0;
};


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
		bool Network = false;
		int Choice = -1;
};


std::unique_ptr<UIViewClass> UI_Message_Box_View(UIMessageBoxPresenterClass & presenter);

int UI_Message_Box(char const * text, int defaultresponse, char const * b1, char const * b2, char const * b3);

enum UINetworkMessageButtons
{
	UI_NETWORK_MESSAGE_OK,
	UI_NETWORK_MESSAGE_OK_CANCEL,
	UI_NETWORK_MESSAGE_YES_NO,
};

bool UI_Network_Message_Box(char const * text, UINetworkMessageButtons buttons, bool (*idle)(void));
