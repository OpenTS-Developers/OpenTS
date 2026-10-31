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
#include <cctype>
#include <string>
#include <utility>


UIScenarioPresenterClass::UIScenarioPresenterClass(UIScenarioServiceClass & service, UIScenarioState state) :
	State(std::move(state)),
	Service(service)
{
}


namespace
{

bool Contains_Fold(std::string const & text, std::string const & query)
{
	if (query.empty()) {
		return(true);
	}

	std::string lower;
	lower.reserve(text.size());
	for (char c : text) {
		lower.push_back((char)std::tolower((unsigned char)c));
	}

	std::string needle;
	needle.reserve(query.size());
	for (char c : query) {
		needle.push_back((char)std::tolower((unsigned char)c));
	}

	return(lower.find(needle) != std::string::npos);
}

}


void UIScenarioPresenterClass::Select(int index)
{
	State.Selected = index;
	State.Meta.clear();
	for (UIScenarioEntry const & entry : State.All) {
		if (entry.Index == index) {
			State.Meta = entry.Meta;
			break;
		}
	}

	Service.Preview(index, State.Preview);
}


void UIScenarioPresenterClass::Apply_Filter(void)
{
	State.Entries.clear();
	for (UIScenarioEntry const & entry : State.All) {
		if (!Contains_Fold(entry.Label, State.Query)) {
			continue;
		}
		if (State.Player != 0 && entry.Players != State.Player) {
			continue;
		}
		if (State.Theater != 0 && State.Theater <= (int)State.TheaterNames.size()
			&& entry.Theater != State.TheaterNames[State.Theater - 1]) {
			continue;
		}
		if (State.Source == 1 && !entry.Official) {
			continue;
		}
		if (State.Source == 2 && entry.Official) {
			continue;
		}
		State.Entries.push_back(entry);
	}

	if (State.Selected < 0) {
		return;
	}

	for (UIScenarioEntry const & entry : State.Entries) {
		if (entry.Index == State.Selected) {
			return;
		}
	}

	State.Selected = -1;
	State.Meta.clear();
	State.Preview = UIMapPreviewImage();
}


void UIScenarioPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "select") {
		for (UIScenarioEntry const & entry : State.Entries) {
			if (entry.Index == intent.Value && intent.Value != State.Selected) {
				Select(intent.Value);
				break;
			}
		}
	} else if (intent.Name == "find") {
		State.Query = intent.Text;
		Apply_Filter();
	} else if (intent.Name == "players") {
		State.Player = intent.Value;
		Apply_Filter();
	} else if (intent.Name == "theater") {
		State.Theater = intent.Value;
		Apply_Filter();
	} else if (intent.Name == "source") {
		State.Source = intent.Value;
		Apply_Filter();
	} else if (intent.Name == "ok") {
		if (State.Selected >= 0) {
			Choice = UI_SCENARIO_ACCEPT;
			Result = UI_RESULT_ACCEPTED;
		}
	} else if (intent.Name == "random") {
		int scenario = Service.Random();
		Service.Read(State);
		if (scenario >= 0 && scenario < (int)State.All.size()) {
			Select(scenario);
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
			Model.DirtyVariable("entries");
			Model.DirtyVariable("selected");
			Model.DirtyVariable("meta");
			Model.DirtyVariable("query");
			Model.DirtyVariable("player");
			Model.DirtyVariable("theater");
			Model.DirtyVariable("source");

			Show_Preview();
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			Rml::StructHandle<UIScenarioEntry> entry = model.RegisterStruct<UIScenarioEntry>();
			Rml::StructHandle<UIScenarioOption> option = model.RegisterStruct<UIScenarioOption>();
			if (!entry || !option) {
				return(false);
			}
			entry.RegisterMember("label", &UIScenarioEntry::Label);
			entry.RegisterMember("index", &UIScenarioEntry::Index);

			option.RegisterMember("label", &UIScenarioOption::Label);
			option.RegisterMember("value", &UIScenarioOption::Value);

			UIScenarioState & state = Data.State;
			return(model.RegisterArray<std::vector<UIScenarioEntry>>()
				&& model.RegisterArray<std::vector<UIScenarioOption>>()
				&& model.Bind("entries", &state.Entries)
				&& model.Bind("selected", &state.Selected)
				&& model.Bind("meta", &state.Meta)
				&& model.Bind("query", &state.Query)
				&& model.Bind("players", &state.Players)
				&& model.Bind("player", &state.Player)
				&& model.Bind("theaters", &state.Theaters)
				&& model.Bind("theater", &state.Theater)
				&& model.Bind("sources", &state.Sources)
				&& model.Bind("source", &state.Source));
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
