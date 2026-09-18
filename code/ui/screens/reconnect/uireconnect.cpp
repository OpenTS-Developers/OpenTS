/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/reconnect/uireconnect.h"

#include "ui/rml/rmlview.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

#include <string>


void UIReconnectPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "cancel") {
		Cancelled = true;

	} else if (intent.Name == "kick") {
		if (intent.Value >= 0 && (std::size_t)intent.Value < State.Players.size()) {
			Kick = intent.Value;
		}
	}
}


void UIReconnectPresenterClass::Refresh(void)
{
}


int UIReconnectPresenterClass::Take_Kick_Vote(void)
{
	int const taken = Kick;
	Kick = -1;
	return(taken);
}


namespace
{

class UIReconnectViewClass : public UIRmlViewClass
{
	public:
		explicit UIReconnectViewClass(UIReconnectPresenterClass & presenter) :
			UIRmlViewClass(presenter, "reconnect.rml", "reconnect"),
			Data(presenter)
		{
		}

		virtual void Sync(void) override
		{
			Model.DirtyVariable("players");
			Model.DirtyVariable("messages");
			Model.DirtyVariable("timeremaining");

			Rml::ElementDocument * document = Document();
			if (document == nullptr) {
				return;
			}

			for (std::size_t index = 0; index < Data.State.Players.size(); index++) {
				UIReconnectPlayer const & player = Data.State.Players[index];
				Rml::Element * bar = document->GetElementById("bar-" + std::to_string(index));
				if (bar == nullptr) {
					continue;
				}

				float filled = player.Filled;
				filled = (filled < 0.0f) ? 0.0f : ((filled > 1.0f) ? 1.0f : filled);
				bar->SetProperty("width", std::to_string((int)(filled * 100.0f)) + "%");
				if (!player.Color.empty()) {
					bar->SetProperty("background-color", player.Color);
				}
			}
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			Rml::StructHandle<UIReconnectPlayer> player = model.RegisterStruct<UIReconnectPlayer>();
			if (!player) {
				return(false);
			}
			player.RegisterMember("name", &UIReconnectPlayer::Name);

			return(model.RegisterArray<std::vector<UIReconnectPlayer>>()
				&& model.RegisterArray<std::vector<std::string>>()
				&& model.Bind("players", &Data.State.Players)
				&& model.Bind("messages", &Data.State.Messages)
				&& model.Bind("timeremaining", &Data.State.TimeRemaining));
		}

	private:
		UIReconnectPresenterClass & Data;
};

}


std::unique_ptr<UIViewClass> UI_Reconnect_View(UIReconnectPresenterClass & presenter)
{
	return(std::make_unique<UIReconnectViewClass>(presenter));
}
