/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uimsgbox.h"

#include "ui/uirmlview.h"

#include <utility>


// The Win32 template puts the first button on the left, the third in the middle and the
// second on the right, and moves a lone first button into the middle.
UIMessageBoxPresenterClass::UIMessageBoxPresenterClass(std::string text, std::vector<std::string> captions, int defaultresponse) :
	Text(std::move(text)),
	Default(defaultresponse)
{
	static int const _slots[3][3] = {
		{ 1, 0, 0 },
		{ 0, 2, 0 },
		{ 0, 2, 1 },
	};

	std::vector<UIMessageButton> present;
	for (int index = 0; index < (int)captions.size() && index < 3; index++) {
		if (!captions[index].empty()) {
			UIMessageButton button;
			button.Caption = captions[index];
			button.Index = index;
			present.push_back(button);
		}
	}

	int count = (int)present.size();
	for (UIMessageButton & button : present) {
		button.Slot = _slots[count - 1][button.Index];
	}

	for (int slot = 0; slot < 3; slot++) {
		for (UIMessageButton const & button : present) {
			if (button.Slot == slot) {
				Buttons.push_back(button);
			}
		}
	}
}


void UIMessageBoxPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "button") {
		Choice = intent.Value;
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "ok") {
		Choice = Default;
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "cancel") {
		Choice = 1;
		Result = UI_RESULT_CANCELLED;
	}
}


void UIMessageBoxPresenterClass::Refresh(void)
{
}


namespace
{

class UIMessageBoxViewClass : public UIRmlViewClass
{
	public:
		explicit UIMessageBoxViewClass(UIMessageBoxPresenterClass & presenter) :
			UIRmlViewClass(presenter, "message.rml", "message"),
			Data(presenter)
		{
		}

		virtual void Sync(void) override
		{
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			Rml::StructHandle<UIMessageButton> button = model.RegisterStruct<UIMessageButton>();
			if (!button) {
				return(false);
			}
			button.RegisterMember("caption", &UIMessageButton::Caption);
			button.RegisterMember("index", &UIMessageButton::Index);
			button.RegisterMember("slot", &UIMessageButton::Slot);

			return(model.RegisterArray<std::vector<UIMessageButton>>() && model.Bind("text", &Data.Text) && model.Bind("buttons", &Data.Buttons));
		}

	private:
		UIMessageBoxPresenterClass & Data;
};

}


std::unique_ptr<UIRmlViewClass> UI_Message_Box_View(UIMessageBoxPresenterClass & presenter)
{
	return(std::make_unique<UIMessageBoxViewClass>(presenter));
}
