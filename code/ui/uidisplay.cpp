/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uidisplay.h"

#include "ui/uirmlview.h"

#include <utility>


UIDisplayPresenterClass::UIDisplayPresenterClass(UIDisplayServiceClass & service, UIDisplayState state) :
	State(std::move(state)),
	Service(service),
	Initial(State.Selected)
{
}


void UIDisplayPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "select") {
		State.Selected = (intent.Value >= 0 && intent.Value < (int)State.Modes.size()) ? intent.Value : -1;

	} else if (intent.Name == "stretch") {
		State.StretchMovies = (intent.Value != 0);

	} else if (intent.Name == "ok") {
		Service.Set_Stretch_Movies(State.StretchMovies);
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

		// The model is small, so every field is re-read after each drain.
		virtual void Sync(void) override
		{
			Model.DirtyAllVariables();
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
				&& model.Bind("stretch", &state.StretchMovies));
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
			Model.DirtyAllVariables();
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


std::unique_ptr<UIRmlViewClass> UI_Display_View(UIDisplayPresenterClass & presenter)
{
	return(std::make_unique<UIDisplayViewClass>(presenter));
}


std::unique_ptr<UIRmlViewClass> UI_Confirm_Mode_View(UIConfirmModePresenterClass & presenter)
{
	return(std::make_unique<UIConfirmModeViewClass>(presenter));
}
