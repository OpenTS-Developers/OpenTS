/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/scenario/uiscenario.h"

#include "ui/rml/rmlsurface.h"
#include "ui/rml/rmlview.h"

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

#include <utility>


UIScenarioPresenterClass::UIScenarioPresenterClass(UIScenarioServiceClass & service, UIScenarioState state) :
	State(std::move(state)),
	Service(service)
{
}


void UIScenarioPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "select") {
		if (intent.Value >= 0 && intent.Value < (int)State.Entries.size() && intent.Value != State.Selected) {
			State.Selected = intent.Value;
			Service.Preview(State.Selected, State.Preview);
		}
	} else if (intent.Name == "ok") {
		Choice = UI_SCENARIO_ACCEPT;
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "random") {
		int scenario = Service.Random();
		Service.Read(State);
		if (scenario >= 0 && scenario < (int)State.Entries.size()) {
			State.Selected = scenario;
			Service.Preview(State.Selected, State.Preview);
		}
	} else if (intent.Name == "cancel") {
		Choice = UI_SCENARIO_CANCEL;
		Result = UI_RESULT_CANCELLED;
	}
}


void UIScenarioPresenterClass::Refresh(void)
{
}


namespace
{

class UIScenarioViewClass : public UIRmlViewClass
{
	public:
		explicit UIScenarioViewClass(UIScenarioPresenterClass & presenter) :
			UIRmlViewClass(presenter, "scenario.rml", "scenario"),
			Data(presenter),
			Shown(-1)
		{
		}

		virtual void Sync(void) override
		{
			Model.DirtyVariable("selected");
			Show_Preview();
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			Rml::StructHandle<UIScenarioEntry> entry = model.RegisterStruct<UIScenarioEntry>();
			if (!entry) {
				return(false);
			}
			entry.RegisterMember("label", &UIScenarioEntry::Label);

			UIScenarioState & state = Data.State;
			return(model.RegisterArray<std::vector<UIScenarioEntry>>()
				&& model.Bind("entries", &state.Entries)
				&& model.Bind("selected", &state.Selected));
		}

	private:
		void Show_Preview(void)
		{
			UIMapPreviewImage & preview = Data.State.Preview;
			if (preview.Generation == Shown || Document() == nullptr) {
				return;
			}

			UIRmlSurfaceElementClass * surface = rmlui_dynamic_cast<UIRmlSurfaceElementClass *>(Document()->GetElementById("preview"));
			if (surface == nullptr) {
				return;
			}

			Shown = preview.Generation;
			surface->Set_Image(preview.Width, preview.Height, preview.Pixels);
		}

		UIScenarioPresenterClass & Data;
		int Shown;
};

}


std::unique_ptr<UIViewClass> UI_Scenario_View(UIScenarioPresenterClass & presenter)
{
	return(std::make_unique<UIScenarioViewClass>(presenter));
}
