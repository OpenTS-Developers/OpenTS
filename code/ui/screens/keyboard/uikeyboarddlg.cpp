/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the keyboard dialog: the service the presenter drives and the state it
// starts from. The presenter lives in uikeyboard.cpp so that the test harness can drive it
// against a recording service.

#include "ui/screens/keyboard/uikeyboard.h"

#include "_command.h"
#include "_ui.h"
#include "ccfile.h"
#include "ccini.h"
#include "cdfile.h"
#include "command.h"
#include "dbgprint.h"
#include "index.h"
#include "init.h"
#include "keyboard.h"
#include "language/language.h"
#include "msgbox.h"
#include "ownrdraw.h"
#include "ui/uienginehost.h"
#include "ui/uishell.h"
#include "ui/uiview.h"
#include "vector.h"


namespace
{

void Fetch_Bindings(std::vector<UIHotkeyBinding> & bindings)
{
	bindings.clear();

	for (int position = 0; position < HotkeyCommands.Count(); position++) {
		CommandClass const * command = HotkeyCommands.Fetch_By_Position(position);
		for (int index = 0; index < AllCommands.Count(); index++) {
			if (AllCommands[index] == command) {
				bindings.push_back({ HotkeyCommands.Fetch_ID_By_Position(position), index });
				break;
			}
		}
	}
}


class UIKeyboardEngineServiceClass : public UIKeyboardServiceClass
{
	public:
		virtual std::string Key_Name(int key) override
		{
			char buffer[128];
			Build_Hotkey_String((KeyNumType)key, buffer);
			return(buffer);
		}

		virtual bool Confirm_Reset(void) override
		{
			return(WWMessageBox()._Process(TXT_RESET_HOTKEYS, IDOK, TXT_YES, TXT_NO, TXT_NONE, false) == 0);
		}

		// Only the player's own file is discarded; the defaults a deployment ships are what
		// the reset falls back on.
		virtual void Reset(std::vector<UIHotkeyBinding> & bindings) override
		{
			DebugString("Deleting users KEYBOARD.INI\n");
			CCFileClass file("KEYBOARD.INI");
			file.Delete();
			Init_Hotkeys();
			Fetch_Bindings(bindings);
		}

		virtual void Save(std::vector<UIHotkeyBinding> const & bindings) override
		{
			HotkeyCommands.Clear();
			for (UIHotkeyBinding const & binding : bindings) {
				if (binding.Key != 0 && binding.Command >= 0 && binding.Command < AllCommands.Count()) {
					HotkeyCommands.Add_Index(binding.Key, AllCommands[binding.Command]);
				}
			}

			CCINIClass ini;
			ini.Clear();
			for (int position = 0; position < HotkeyCommands.Count(); position++) {
				CommandClass const * command = HotkeyCommands.Fetch_By_Position(position);
				ini.Put_Int("Hotkey", command->Get_Unique_Name(), HotkeyCommands.Fetch_ID_By_Position(position));
			}

			CDFileClass file("Keyboard.ini");
			ini.Save(file, false);
		}
};

UIKeyboardEngineServiceClass _Service;

}


UIKeyboardServiceClass & UI_Keyboard_Service(void)
{
	return(_Service);
}


void UI_Keyboard_State(UIKeyboardState & state)
{
	state = UIKeyboardState();

	for (int index = 0; index < AllCommands.Count(); index++) {
		CommandClass const * command = AllCommands[index];

		// Each string is copied before the next is asked for, because an accessor may answer
		// from storage that the next one reuses.
		UIHotkeyCommand entry;
		char const * category = command->Get_Category();
		entry.Category = (category != NULL) ? category : "";
		char const * name = command->Get_Display_Name();
		entry.Name = (name != NULL) ? name : "";
		char const * description = command->Get_Description();
		entry.Description = (description != NULL) ? description : "";
		state.Commands.push_back(entry);
	}

	Fetch_Bindings(state.Bindings);
}


bool UI_Keyboard_Dialog(void)
{
	if (UIShell.Legacy_Dialog_Visible()) {
		return(false);
	}

	UIKeyboardState state;
	UI_Keyboard_State(state);

	UIKeyboardPresenterClass presenter(UI_Keyboard_Service(), state);
	std::unique_ptr<UIViewClass> view = UI_Keyboard_View(presenter);

	return(UI_Run_Modal(*view) != UI_RESULT_FAILED_TO_OPEN);
}
