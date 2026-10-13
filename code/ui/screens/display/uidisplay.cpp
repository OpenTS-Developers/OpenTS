/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/display/uidisplay.h"

#include "ui/rml/rmlview.h"
#include "uiscalelayout.h"

#include <algorithm>
#include <string>
#include <utility>


UIDisplayPresenterClass::UIDisplayPresenterClass(UIDisplayServiceClass & service, UIDisplayState state) :
	State(std::move(state)),
	Service(service),
	Initial(State.Selected)
{
	Fit_UI_Scale();
}


// Holds the interface size within what the selected resolution fits and names it.
void UIDisplayPresenterClass::Fit_UI_Scale(void)
{
	int width = State.ScreenWidth;
	int height = State.ScreenHeight;
	if (State.Selected >= 0 && State.Selected < (int)State.Modes.size()) {
		width = State.Modes[State.Selected].Width;
		height = State.Modes[State.Selected].Height;
	}

	State.UIScaleMax = UI_Scale_For(width, height, UI_SCALE_MAX);
	State.UIScale = std::clamp(State.UIScale, 0, State.UIScaleMax);

	if (State.UIScale == 0) {
		State.UIScaleLabel = "Auto (" + std::to_string(UI_Scale_For(width, height, 0)) + "x)";
	} else {
		State.UIScaleLabel = std::to_string(State.UIScale) + "x";
	}
}


void UIDisplayPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "select") {
		State.Selected = (intent.Value >= 0 && intent.Value < (int)State.Modes.size()) ? intent.Value : -1;
		Fit_UI_Scale();

	} else if (intent.Name == "stretch") {
		State.StretchMovies = (intent.Value != 0);

	} else if (intent.Name == "systemcursor") {
		State.SystemCursor = (intent.Value != 0);

	} else if (intent.Name == "uiscale") {
		State.UIScale = intent.Value;
		Fit_UI_Scale();

	} else if (intent.Name == "ok") {
		Service.Set_Stretch_Movies(State.StretchMovies);
		Service.Set_System_Cursor(State.SystemCursor);
		Service.Set_UI_Scale(State.UIScale);
		if (State.Selected >= 0 && State.Selected != Initial) {
			Picked = State.Modes[State.Selected];
		}
		Result = UI_RESULT_ACCEPTED;

	} else if (intent.Name == "cancel") {
		Result = UI_RESULT_CANCELLED;
	}
}


void UIDisplayPresenterClass::Refresh(void)
{
}


UIConfirmModePresenterClass::UIConfirmModePresenterClass(UIClockClass & clock, int timeout) :
	Seconds((timeout + 999) / 1000),
	Clock(clock),
	Timeout(timeout)
{
}


void UIConfirmModePresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "ok") {
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "cancel") {
		Result = UI_RESULT_CANCELLED;
	}
}


void UIConfirmModePresenterClass::Refresh(void)
{
	int now = Clock.Milliseconds();
	if (!Deadline.has_value()) {
		Deadline = now + Timeout;
	}

	int left = *Deadline - now;
	Seconds = (left > 0) ? (left + 999) / 1000 : 0;

	if (left <= 0 && !Result.has_value()) {
		TimedOut = true;
		Result = UI_RESULT_CANCELLED;
	}
}


namespace
{

class UIDisplayViewClass : public UIRmlViewClass
{
	public:
		explicit UIDisplayViewClass(UIDisplayPresenterClass & presenter) :
			UIRmlViewClass(presenter, "display.rml", "display"),
			Data(presenter)
		{
		}

		virtual void Sync(void) override
		{
			Model.DirtyVariable("selected");
			Model.DirtyVariable("stretch");
			Model.DirtyVariable("systemcursor");
			Model.DirtyVariable("uiscale");
			Model.DirtyVariable("uiscalemax");
			Model.DirtyVariable("uiscalelabel");
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			Rml::StructHandle<UIDisplayMode> mode = model.RegisterStruct<UIDisplayMode>();
			if (!mode) {
				return(false);
			}
			mode.RegisterMember("label", &UIDisplayMode::Label);
			mode.RegisterMember("width", &UIDisplayMode::Width);
			mode.RegisterMember("height", &UIDisplayMode::Height);

			UIDisplayState & state = Data.State;
			return(model.RegisterArray<std::vector<UIDisplayMode>>()
				&& model.Bind("modes", &state.Modes)
				&& model.Bind("selected", &state.Selected)
				&& model.Bind("stretch", &state.StretchMovies)
				&& model.Bind("systemcursor", &state.SystemCursor)
				&& model.Bind("uiscale", &state.UIScale)
				&& model.Bind("uiscalemax", &state.UIScaleMax)
				&& model.Bind("uiscalelabel", &state.UIScaleLabel));
		}

	private:
		UIDisplayPresenterClass & Data;
};


class UIConfirmModeViewClass : public UIRmlViewClass
{
	public:
		explicit UIConfirmModeViewClass(UIConfirmModePresenterClass & presenter) :
			UIRmlViewClass(presenter, "confirm.rml", "confirm"),
			Data(presenter)
		{
		}

		virtual void Sync(void) override
		{
			Model.DirtyVariable("seconds");
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			return(model.Bind("seconds", &Data.Seconds));
		}

	private:
		UIConfirmModePresenterClass & Data;
};

}


std::unique_ptr<UIViewClass> UI_Display_View(UIDisplayPresenterClass & presenter)
{
	return(std::make_unique<UIDisplayViewClass>(presenter));
}


std::unique_ptr<UIViewClass> UI_Confirm_Mode_View(UIConfirmModePresenterClass & presenter)
{
	return(std::make_unique<UIConfirmModeViewClass>(presenter));
}
