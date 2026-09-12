/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/keyboard/uikeyboard.h"

#include "ui/rml/rmlkeys.h"
#include "ui/rml/rmlview.h"

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

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
				State.Visible.push_back({ command, State.Commands[command].Name });
			}
		}
		std::stable_sort(State.Visible.begin(), State.Visible.end(), [](UIHotkeyRow const & a, UIHotkeyRow const & b) {
			return(Compare_Ignoring_Case(a.Name, b.Name) < 0);
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


namespace
{

class UIKeyboardViewClass : public UIRmlViewClass
{
	public:
		explicit UIKeyboardViewClass(UIKeyboardPresenterClass & presenter) :
			UIRmlViewClass(presenter, "keyboard.rml", "keyboard"),
			Data(presenter)
		{
		}

		// Selecting a command moves the focus to the capture element, as the Win32 dialog moved it
		// to its hotkey control.
		virtual void Sync(void) override
		{
			Model.DirtyAllVariables();

			if (Data.State.Selected != LastSelected) {
				LastSelected = Data.State.Selected;
				Rml::Element * capture = Capture();
				if (LastSelected >= 0 && capture != nullptr) {
					capture->Focus();
				}
			}
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			Rml::StructHandle<UIHotkeyRow> row = model.RegisterStruct<UIHotkeyRow>();
			if (!row) {
				return(false);
			}
			row.RegisterMember("command", &UIHotkeyRow::Command);
			row.RegisterMember("name", &UIHotkeyRow::Name);

			UIKeyboardState & state = Data.State;
			return(model.RegisterArray<std::vector<std::string>>()
				&& model.RegisterArray<std::vector<UIHotkeyRow>>()
				&& model.Bind("categories", &state.Categories)
				&& model.Bind("categoryindex", &state.Category)
				&& model.Bind("rows", &state.Visible)
				&& model.Bind("selected", &state.Selected)
				&& model.Bind("description", &state.Description)
				&& model.Bind("shortcut", &state.Shortcut)
				&& model.Bind("capturedname", &state.CapturedName)
				&& model.Bind("assignedto", &state.AssignedTo));
		}

		virtual void Loaded(void) override
		{
			Rml::Element * capture = Capture();
			if (capture != nullptr) {
				capture->AddEventListener(Rml::EventId::Keydown, this);
			}
		}

		// A key pressed in the capture element becomes the captured number. Enter, Escape and
		// Tab keep their dialog meaning and a modifier on its own captures nothing.
		virtual void ProcessEvent(Rml::Event & event) override
		{
			if (event.GetId() == Rml::EventId::Keydown && event.GetCurrentElement() != nullptr && event.GetCurrentElement() == Capture()) {
				Rml::Input::KeyIdentifier key = (Rml::Input::KeyIdentifier)event.GetParameter<int>("key_identifier", 0);
				if (key == Rml::Input::KI_RETURN || key == Rml::Input::KI_NUMPADENTER || key == Rml::Input::KI_ESCAPE || key == Rml::Input::KI_TAB) {
					return;
				}

				int number = UI_Key_Number(key, event.GetParameter<bool>("shift_key", false), event.GetParameter<bool>("ctrl_key", false), event.GetParameter<bool>("alt_key", false));
				if (number != 0) {
					Queue("capture", number);
				}
				event.StopPropagation();
				return;
			}

			UIRmlViewClass::ProcessEvent(event);
		}

	private:
		Rml::Element * Capture(void)
		{
			return((Document() != nullptr) ? Document()->GetElementById("capture") : nullptr);
		}

		UIKeyboardPresenterClass & Data;
		int LastSelected = -1;
};

}


std::unique_ptr<UIRmlViewClass> UI_Keyboard_View(UIKeyboardPresenterClass & presenter)
{
	return(std::make_unique<UIKeyboardViewClass>(presenter));
}
