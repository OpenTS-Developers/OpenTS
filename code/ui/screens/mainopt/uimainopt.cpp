/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/mainopt/uimainopt.h"

#include "ui/rml/rmlview.h"

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

#include <string>
#include <utility>


UIMainOptionsPresenterClass::UIMainOptionsPresenterClass(UIMainOptionsState state) :
	State(std::move(state))
{
}


void UIMainOptionsPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "settings") {
		Choice = UI_MAIN_OPTIONS_SETTINGS;
		Result = UI_RESULT_ACCEPTED;

	} else if (intent.Name == "display") {
		Choice = UI_MAIN_OPTIONS_DISPLAY;
		Result = UI_RESULT_ACCEPTED;

	} else if (intent.Name == "sound") {
		if (State.SoundEnabled) {
			Choice = UI_MAIN_OPTIONS_SOUND;
			Result = UI_RESULT_ACCEPTED;
		}

	} else if (intent.Name == "keyboard") {
		Choice = UI_MAIN_OPTIONS_KEYBOARD;
		Result = UI_RESULT_ACCEPTED;

	} else if (intent.Name == "ok") {
		Choice = UI_MAIN_OPTIONS_LEAVE;
		Result = UI_RESULT_ACCEPTED;

	} else if (intent.Name == "cancel") {
		Choice = UI_MAIN_OPTIONS_LEAVE;
		Result = UI_RESULT_CANCELLED;
	}
}


void UIMainOptionsPresenterClass::Refresh(void)
{
}


namespace
{

class UIMainOptionsViewClass : public UIRmlViewClass
{
	public:
		explicit UIMainOptionsViewClass(UIMainOptionsPresenterClass & presenter) :
			UIRmlViewClass(presenter, "mainopt.rml", "mainopt"),
			Data(presenter)
		{
		}

		virtual void Sync(void) override
		{
			Model.DirtyAllVariables();
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			return(model.Bind("soundenabled", &Data.State.SoundEnabled));
		}

		// The menu sits where the main menu's buttons were, so the panel takes that edge over
		// the centred position the style sheet gives it.
		virtual void Loaded(void) override
		{
			Rml::Element * panel = Document()->GetElementById("panel");
			if (panel != nullptr && Data.State.Top >= 0) {
				panel->SetProperty("top", std::to_string(Data.State.Top) + "dp");
				panel->SetProperty("margin-top", "0dp");
			}
		}

	private:
		UIMainOptionsPresenterClass & Data;
};

}


std::unique_ptr<UIRmlViewClass> UI_Main_Options_View(UIMainOptionsPresenterClass & presenter)
{
	return(std::make_unique<UIMainOptionsViewClass>(presenter));
}
