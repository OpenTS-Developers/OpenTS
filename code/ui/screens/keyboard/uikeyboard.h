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


struct UIHotkeyCommand
{
	std::string Category;
	std::string Name;
	std::string Description;
};


struct UIHotkeyBinding
{
	int Key = 0;
	int Command = -1;
};


struct UIHotkeyRow
{
	int Command = -1;
	std::string Name;
};


class UIKeyboardServiceClass
{
	public:
		virtual ~UIKeyboardServiceClass(void) = default;
		virtual std::string Key_Name(int key) = 0;
		virtual bool Confirm_Reset(void) = 0;
		virtual void Reset(std::vector<UIHotkeyBinding> & bindings) = 0;
		virtual void Save(std::vector<UIHotkeyBinding> const & bindings) = 0;
};


struct UIKeyboardState
{
	std::vector<UIHotkeyCommand> Commands;
	std::vector<UIHotkeyBinding> Bindings;
	std::vector<std::string> Categories;
	int Category = -1;
	std::vector<UIHotkeyRow> Visible;
	int Selected = -1;
	std::string Description;
	std::string Shortcut;
	int Captured = 0;
	std::string CapturedName;
	std::string AssignedTo;
};


class UIKeyboardPresenterClass : public UIPresenterClass
{
	public:
		UIKeyboardPresenterClass(UIKeyboardServiceClass & service, UIKeyboardState state);
		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;
		int Key_Of(int command) const;
		int Owner_Of(int key) const;

		UIKeyboardState State;

	private:
		void Reload(void);
		void Show_Category(int index);
		void Show_Command(void);
		void Update_Capture(void);
		std::string Name_Of_Key(int key);

		UIKeyboardServiceClass & Service;
};


std::unique_ptr<UIViewClass> UI_Keyboard_View(UIKeyboardPresenterClass & presenter);

UIKeyboardServiceClass & UI_Keyboard_Service(void);
void UI_Keyboard_State(UIKeyboardState & state);

void UI_Keyboard_Dialog(void);
