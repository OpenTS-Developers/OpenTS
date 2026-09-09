/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uikeyboard.h"

#include "ui/uirmlview.h"

#include <algorithm>
#include <cctype>
#include <utility>


// The Win32 controls sorted their rows without regard to case, and the categories were told
// apart the same way.
static int Compare_Ignoring_Case(std::string const & a, std::string const & b)
{
	size_t count = std::min(a.size(), b.size());
	for (size_t index = 0; index < count; index++) {
		int ca = std::tolower((unsigned char)a[index]);
		int cb = std::tolower((unsigned char)b[index]);
		if (ca != cb) {
			return((ca < cb) ? -1 : 1);
		}
	}
	if (a.size() == b.size()) {
		return(0);
	}
	return((a.size() < b.size()) ? -1 : 1);
}


UIKeyboardPresenterClass::UIKeyboardPresenterClass(UIKeyboardServiceClass & service, UIKeyboardState state) :
	State(std::move(state)),
	Service(service)
{
	Reload();
}


void UIKeyboardPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "category") {
		Show_Category(intent.Value);

	} else if (intent.Name == "select") {
		State.Selected = (intent.Value >= 0 && intent.Value < (int)State.Commands.size()) ? intent.Value : -1;
		Show_Command();

	} else if (intent.Name == "capture") {
		State.Captured = intent.Value;
		Update_Capture();

	} else if (intent.Name == "assign") {
		if (State.Selected >= 0) {
			std::vector<UIHotkeyBinding> & bindings = State.Bindings;
			bindings.erase(std::remove_if(bindings.begin(), bindings.end(), [this](UIHotkeyBinding const & binding) {
				return(binding.Command == State.Selected || (State.Captured != 0 && binding.Key == State.Captured));
			}), bindings.end());
			if (State.Captured != 0) {
				bindings.push_back({ State.Captured, State.Selected });
			}
			Show_Command();
		}

	} else if (intent.Name == "reset") {
		if (Service.Confirm_Reset()) {
			Service.Reset(State.Bindings);
			Reload();
		}

	} else if (intent.Name == "ok") {
		Service.Save(State.Bindings);
		Result = UI_RESULT_ACCEPTED;

	} else if (intent.Name == "cancel") {
		Result = UI_RESULT_CANCELLED;
	}
}


void UIKeyboardPresenterClass::Refresh(void)
{
}


int UIKeyboardPresenterClass::Key_Of(int command) const
{
	for (UIHotkeyBinding const & binding : State.Bindings) {
		if (binding.Command == command) {
			return(binding.Key);
		}
	}
	return(0);
}


int UIKeyboardPresenterClass::Owner_Of(int key) const
{
	if (key == 0) {
		return(-1);
	}
	for (UIHotkeyBinding const & binding : State.Bindings) {
		if (binding.Key == key) {
			return(binding.Command);
		}
	}
	return(-1);
}


void UIKeyboardPresenterClass::Reload(void)
{
	State.Categories.clear();
	for (UIHotkeyCommand const & command : State.Commands) {
		bool known = false;
		for (std::string const & category : State.Categories) {
			if (Compare_Ignoring_Case(category, command.Category) == 0) {
				known = true;
			}
		}
		if (!known) {
			State.Categories.push_back(command.Category);
		}
	}
	std::sort(State.Categories.begin(), State.Categories.end(), [](std::string const & a, std::string const & b) {
		return(Compare_Ignoring_Case(a, b) < 0);
	});

	Show_Category(State.Categories.empty() ? -1 : 0);
}


void UIKeyboardPresenterClass::Show_Category(int index)
{
	State.Category = (index >= 0 && index < (int)State.Categories.size()) ? index : -1;

	State.Visible.clear();
	if (State.Category >= 0) {
		std::string const & category = State.Categories[State.Category];
		for (int command = 0; command < (int)State.Commands.size(); command++) {
			if (Compare_Ignoring_Case(State.Commands[command].Category, category) == 0) {
				State.Visible.push_back(command);
			}
		}
		std::stable_sort(State.Visible.begin(), State.Visible.end(), [this](int a, int b) {
			return(Compare_Ignoring_Case(State.Commands[a].Name, State.Commands[b].Name) < 0);
		});
	}

	State.Selected = -1;
	Show_Command();
}


void UIKeyboardPresenterClass::Show_Command(void)
{
	if (State.Selected >= 0) {
		State.Description = State.Commands[State.Selected].Description;
		State.Shortcut = Name_Of_Key(Key_Of(State.Selected));
	} else {
		State.Description.clear();
		State.Shortcut.clear();
	}

	State.Captured = 0;
	Update_Capture();
}


void UIKeyboardPresenterClass::Update_Capture(void)
{
	State.CapturedName = Name_Of_Key(State.Captured);

	int owner = Owner_Of(State.Captured);
	State.AssignedTo = (owner >= 0) ? State.Commands[owner].Name : std::string();
}


std::string UIKeyboardPresenterClass::Name_Of_Key(int key)
{
	if (key == 0) {
		return(std::string());
	}
	return(Service.Key_Name(key));
}
