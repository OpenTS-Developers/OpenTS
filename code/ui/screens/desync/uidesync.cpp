/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/desync/uidesync.h"

#include "ui/rml/rmlview.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

#include <string>


UIDesyncPresenterClass::UIDesyncPresenterClass(UIDesyncServiceClass & service) :
	Service(service)
{
	Service.Read(State);
}


void UIDesyncPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "say") {
		// The box sends on a line break and keeps what was typed otherwise.
		State.Say = intent.Text;
		if (intent.Value != 0 && State.Say.size() > 0) {
			Service.Say(State.Say.c_str());
			State.Say.clear();
		}

	} else if (intent.Name == "load") {
		if (State.Host && State.LoadEnabled) {
			Choice = UI_DESYNC_LOAD;
			Result = UI_RESULT_ACCEPTED;
		}

	} else if (intent.Name == "continue") {
		if (State.Host && State.ContinueEnabled) {
			Choice = UI_DESYNC_CONTINUE;
			Result = UI_RESULT_ACCEPTED;
		}

	} else if (intent.Name == "quit") {
		if (State.QuitEnabled) {
			Choice = UI_DESYNC_QUIT;
			Result = UI_RESULT_ACCEPTED;
		}
	}
}


// The session settles the decision for everyone once one player has made it, so the screen
// closes on that as well as on a button, the way the dialog's loop ended.
void UIDesyncPresenterClass::Refresh(void)
{
	bool host = State.Host;
	Service.Read(State);

	if (!Result.has_value() && (Service.Settled() || State.Host != host)) {
		Result = UI_RESULT_ACCEPTED;
	}
}


namespace
{

class UIDesyncViewClass : public UIRmlViewClass
{
	public:
		explicit UIDesyncViewClass(UIDesyncPresenterClass & presenter) :
			UIRmlViewClass(presenter, "desync.rml", "desync"),
			Data(presenter)
		{
		}

		virtual void Sync(void) override
		{
			Model.DirtyVariable("players");
			Model.DirtyVariable("chat");
			Model.DirtyVariable("say");
			Model.DirtyVariable("loadenabled");
			Model.DirtyVariable("continueenabled");
			Model.DirtyVariable("quitenabled");
			Model.DirtyVariable("counting");
			Model.DirtyVariable("countdowntext");

			Rml::Element * bar = Document()->GetElementById("countdown-bar");
			if (bar != nullptr) {
				float width = Data.State.Countdown;
				width = (width < 0.0f) ? 0.0f : ((width > 1.0f) ? 1.0f : width);
				bar->SetProperty("width", std::to_string((int)(width * 100.0f)) + "%");
				if (!Data.State.CountdownColor.empty()) {
					bar->SetProperty("background-color", Data.State.CountdownColor);
				}
			}
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			Rml::StructHandle<UIDesyncPlayerRow> row = model.RegisterStruct<UIDesyncPlayerRow>();
			if (!row) {
				return(false);
			}
			row.RegisterMember("name", &UIDesyncPlayerRow::Name);
			row.RegisterMember("status", &UIDesyncPlayerRow::Status);
			row.RegisterMember("color", &UIDesyncPlayerRow::Color);
			row.RegisterMember("mark", &UIDesyncPlayerRow::Mark);

			UIDesyncState & state = Data.State;
			return(model.RegisterArray<std::vector<UIDesyncPlayerRow>>()
				&& model.RegisterArray<std::vector<std::string>>()
				&& model.Bind("host", &state.Host)
				&& model.Bind("players", &state.Players)
				&& model.Bind("chat", &state.Chat)
				&& model.Bind("say", &state.Say)
				&& model.Bind("loadenabled", &state.LoadEnabled)
				&& model.Bind("continueenabled", &state.ContinueEnabled)
				&& model.Bind("quitenabled", &state.QuitEnabled)
				&& model.Bind("counting", &state.Counting)
				&& model.Bind("countdowntext", &state.CountdownText));
		}

		// The master's template and everyone else's differ in their prose and their buttons,
		// so the document wears which one it is.
		virtual void Loaded(void) override
		{
			Document()->SetClass(Data.State.Host ? "host" : "wait", true);
		}

	private:
		UIDesyncPresenterClass & Data;
};

}


std::unique_ptr<UIViewClass> UI_Desync_View(UIDesyncPresenterClass & presenter)
{
	return(std::make_unique<UIDesyncViewClass>(presenter));
}
