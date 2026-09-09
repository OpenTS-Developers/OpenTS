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

class UIRmlViewClass;


// One game command as the keyboard dialog lists it. Its index in the state is the command's
// index in the game's command list.
struct UIHotkeyCommand
{
	std::string Category;
	std::string Name;
	std::string Description;
};


// A key number, in the KEYBOARD.INI encoding, bound to a command.
struct UIHotkeyBinding
{
	int Key = 0;
	int Command = -1;
};


// One row of the command list: the command and its name.
struct UIHotkeyRow
{
	int Command = -1;
	std::string Name;
};


// The engine calls the keyboard dialog makes. The game supplies one over the hotkey table,
// the keyboard file and the message box; the test harness supplies one that records the calls.
class UIKeyboardServiceClass
{
	public:
		virtual ~UIKeyboardServiceClass(void) = default;

		virtual std::string Key_Name(int key) = 0;
		virtual bool Confirm_Reset(void) = 0;
		// Discards the player's keyboard file, reloads the game's table and returns it.
		virtual void Reset(std::vector<UIHotkeyBinding> & bindings) = 0;
		// Makes the table the game's and writes the keyboard file.
		virtual void Save(std::vector<UIHotkeyBinding> const & bindings) = 0;
};


// What the dialog shows: the commands, the table being edited, the categories and the
// commands of the open one, the selected command's description and shortcut, and the key in
// the capture control with the command that owns it.
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


// Edits a copy of the hotkey table. Assigning gives the captured key to the selected command
// and takes it from whichever command held it; an empty capture leaves the command unbound.
// Accepting saves the copy; cancelling drops it; a confirmed reset reloads it from the game.
class UIKeyboardPresenterClass : public UIPresenterClass
{
	public:
		UIKeyboardPresenterClass(UIKeyboardServiceClass & service, UIKeyboardState state);

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		// The key bound to a command, or 0; the command bound to a key, or -1.
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


// The RmlUi view over a keyboard presenter, bound to keyboard.rml. The presenter must outlive
// it.
std::unique_ptr<UIRmlViewClass> UI_Keyboard_View(UIKeyboardPresenterClass & presenter);

// The game's service and the state of the hotkey table, shared by the Win32 dialog and the
// RmlUi view.
UIKeyboardServiceClass & UI_Keyboard_Service(void);
void UI_Keyboard_State(UIKeyboardState & state);

// Runs the keyboard dialog as an RmlUi screen. False means it could not run as one and the
// caller should open its Win32 dialog.
bool UI_Keyboard_Dialog(void);
