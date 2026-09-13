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
#include <RmlUi/Core/Context.h>
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

		// Nothing this screen binds changes once it is open, so the first read is the last.
		// The wallpaper is placed here rather than when the document loads, because where it
		// goes depends on where the dialog landed and nothing is laid out until then.
		virtual void Sync(void) override
		{
			Place_Wallpaper();
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			return(model.Bind("soundenabled", &Data.State.SoundEnabled));
		}

		// The wallpaper belongs to the screen rather than to the dialog: it is one picture
		// centred on the frame, and a dialog shows the part of it lying behind. The style
		// sheet hangs it off the middle of the dialog, which is the middle of the frame only
		// while the dialog is centred, so a dialog sitting anywhere else pushes it back by
		// however far it is off centre. The picture is sized in dp, so it is placed in dp.
		void Place_Wallpaper(void)
		{
			Rml::Element * wallpaper = Document() != nullptr ? Document()->GetElementById("wallpaper-art") : nullptr;
			Rml::Element * dialog = Document() != nullptr ? Document()->GetElementById("reveal") : nullptr;
			Rml::Context * context = Document() != nullptr ? Document()->GetContext() : nullptr;
			if (wallpaper == nullptr || dialog == nullptr || context == nullptr) {
				return;
			}

			float ratio = context->GetDensityIndependentPixelRatio();
			if (ratio <= 0.0f) {
				ratio = 1.0f;
			}

			float middle = (dialog->GetAbsoluteOffset(Rml::BoxArea::Border).y + dialog->GetBox().GetSize().y * 0.5f) / ratio;
			int offset = (int)(-200.0f + (float)context->GetDimensions().y * 0.5f / ratio - middle);
			if (offset == Placed) {
				return;
			}

			Placed = offset;
			wallpaper->SetProperty("margin-top", std::to_string(offset) + "dp");
		}

		// The menu sits where the main menu's buttons were, so the dialog takes that edge
		// over the centered position the style sheet gives it.
		virtual void Loaded(void) override
		{
			Rml::Element * dialog = Document()->GetElementById("reveal");
			if (dialog == nullptr || Data.State.Top < 0) {
				return;
			}

			dialog->SetProperty("top", std::to_string(Data.State.Top) + "dp");
			dialog->SetProperty("margin-top", "0dp");
		}

	private:
		UIMainOptionsPresenterClass & Data;

		// Where the wallpaper was last put, so it is only moved when it has to be.
		int Placed = 0x7FFFFFFF;
};

}


std::unique_ptr<UIViewClass> UI_Main_Options_View(UIMainOptionsPresenterClass & presenter)
{
	return(std::make_unique<UIMainOptionsViewClass>(presenter));
}
