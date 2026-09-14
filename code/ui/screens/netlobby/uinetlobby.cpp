/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/netlobby/uinetlobby.h"

#include "ui/rml/rmlsurface.h"
#include "ui/rml/rmlview.h"

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>
#include <utility>


UINetLobbyPresenterClass::UINetLobbyPresenterClass(UINetLobbyServiceClass & service, UINetLobbyState state) :
	State(std::move(state)),
	Service(service),
	Opened(State.Kind)
{
}


void UINetLobbyPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "handle") {
		State.Handle = intent.Text;
		Service.Set_Handle(State.Handle.c_str());
	} else if (intent.Name == "game") {
		if (intent.Value >= 0 && intent.Value < (int)State.Games.size() && intent.Value != State.Game) {
			State.Game = intent.Value;
			Service.Select_Game(State.Game);
		}
	} else if (intent.Name == "say") {
		State.Say = intent.Text;

		// The value says whether the line was ended. The Win32 dialog sends on the overflow
		// that ending it causes, and drops anything shorter than three characters.
		if (intent.Value != 0) {
			if (State.Say.size() > 2) {
				Service.Say(State.Say.c_str());
			}
			State.Say.clear();
		}
	} else if (intent.Name == "side") {
		if (intent.Value >= 0 && intent.Value < (int)State.Sides.size()) {
			State.Side = intent.Value;
			Service.Set_Side(State.Side);
		}
	} else if (intent.Name == "colour") {
		if (intent.Value >= 0 && intent.Value < (int)State.Colours.size()) {
			State.Colour = intent.Value;
			Service.Set_Colour(State.Colour);
		}
	} else if (intent.Name == "pick") {
		if (intent.Value >= 0 && intent.Value < (int)State.Players.size()) {
			std::string const & name = State.Players[intent.Value].Name;
			std::vector<std::string>::iterator found = std::find(Picked.begin(), Picked.end(), name);
			if (found != Picked.end()) {
				Picked.erase(found);
			} else {
				Picked.push_back(name);
			}
			Mark_Picked();
		}
	} else if (intent.Name == "kick") {
		if (!Picked.empty()) {
			Service.Kick(Picked);
			Picked.clear();
			Mark_Picked();
		}
	} else if (intent.Name == "accept") {
		if (State.CanAccept) {
			State.CanAccept = false;
			Service.Accept();
		}
	} else if (intent.Name == "bases") {
		Toggle(UI_NET_BASES, intent.Value != 0);
	} else if (intent.Name == "crates") {
		Toggle(UI_NET_CRATES, intent.Value != 0);
	} else if (intent.Name == "short") {
		Toggle(UI_NET_SHORT_GAME, intent.Value != 0);
	} else if (intent.Name == "allies") {
		Toggle(UI_NET_ALLIES, intent.Value != 0);
	} else if (intent.Name == "truce") {
		Toggle(UI_NET_HARVESTER_TRUCE, intent.Value != 0);
	} else if (intent.Name == "fog") {
		Toggle(UI_NET_FOG_OF_WAR, intent.Value != 0);
	} else if (intent.Name == "bridges") {
		Toggle(UI_NET_BRIDGES, intent.Value != 0);
	} else if (intent.Name == "engineer") {
		Toggle(UI_NET_ENGINEER, intent.Value != 0);
	} else if (intent.Name == "redeploy") {
		Toggle(UI_NET_REDEPLOY, intent.Value != 0);
	} else if (intent.Name == "speed") {
		Move(UI_NET_SPEED, intent.Value);
	} else if (intent.Name == "aiplayers") {
		Move(UI_NET_AI_PLAYERS, intent.Value);
	} else if (intent.Name == "ailevel") {
		Move(UI_NET_AI_LEVEL, intent.Value);
	} else if (intent.Name == "units") {
		Move(UI_NET_UNITS, intent.Value);
	} else if (intent.Name == "tech") {
		Move(UI_NET_TECH, intent.Value);
	} else if (intent.Name == "credits") {
		Move(UI_NET_CREDITS, intent.Value);
	} else if (intent.Name == "map") {
		Choice = UI_NET_PICK_MAP;
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "new") {
		Choice = UI_NET_NEW;
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "join") {
		Choice = UI_NET_JOIN;
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "go") {
		if (State.CanGo) {
			Choice = UI_NET_GO;
			Result = UI_RESULT_ACCEPTED;
		}
	} else if (intent.Name == "cancel") {
		Choice = UI_NET_CANCEL;
		Result = UI_RESULT_CANCELLED;
	}
}


void UINetLobbyPresenterClass::Toggle(UINetSwitch which, bool on)
{
	switch (which) {
		case UI_NET_BASES:
			State.Bases = on;

			// The Win32 dialog clears a short game when bases go, and sets bases when one is
			// asked for, because a short game is decided by what a player has left standing.
			if (!on) {
				State.ShortGame = false;
			}
			break;

		case UI_NET_SHORT_GAME:
			State.ShortGame = on;
			if (on) {
				State.Bases = true;
			}
			break;

		case UI_NET_CRATES: State.Crates = on; break;
		case UI_NET_ALLIES: State.Allies = on; break;
		case UI_NET_HARVESTER_TRUCE: State.HarvesterTruce = on; break;
		case UI_NET_FOG_OF_WAR: State.FogOfWar = on; break;
		case UI_NET_BRIDGES: State.Bridges = on; break;
		case UI_NET_ENGINEER: State.MultiEngineer = on; break;
		case UI_NET_REDEPLOY: State.Redeploy = on; break;
		default: return;
	}

	Service.Set_Switch(which, on);
	if (which == UI_NET_BASES || which == UI_NET_SHORT_GAME) {
		Service.Set_Switch(UI_NET_BASES, State.Bases);
		Service.Set_Switch(UI_NET_SHORT_GAME, State.ShortGame);
	}
}


// A reading is held to the bounds the rules gave it, because a document can be told to ask for
// anything and the game reads these straight into the session.
static int Clamp(int value, int low, int high)
{
	if (value < low) {
		return(low);
	}
	return(value > high ? high : value);
}


void UINetLobbyPresenterClass::Move(UINetSlider which, int value)
{
	int const before = Reading(which);

	switch (which) {
		case UI_NET_SPEED: State.GameSpeed = Clamp(value, 0, 6); break;
		case UI_NET_AI_PLAYERS: State.AIPlayers = Clamp(value, 0, State.AIPlayersMax); break;
		case UI_NET_AI_LEVEL: State.AILevel = Clamp(value, 0, 2); break;
		case UI_NET_UNITS: State.UnitCount = Clamp(value, State.UnitCountMin, State.UnitCountMax); break;
		case UI_NET_TECH: State.TechLevel = Clamp(value, 1, State.TechLevelMax); break;
		case UI_NET_CREDITS: State.Credits = Clamp(value, State.CreditsMin, State.CreditsMax); break;
		default: return;
	}

	// Every change here reaches the wire, so a track that has not actually moved must not
	// publish anything: laying the document out can raise a change of its own.
	if (Reading(which) != before) {
		Service.Set_Slider(which, Reading(which));
	}
}


int UINetLobbyPresenterClass::Reading(UINetSlider which) const
{
	switch (which) {
		case UI_NET_SPEED: return(State.GameSpeed);
		case UI_NET_AI_PLAYERS: return(State.AIPlayers);
		case UI_NET_AI_LEVEL: return(State.AILevel);
		case UI_NET_UNITS: return(State.UnitCount);
		case UI_NET_TECH: return(State.TechLevel);
		case UI_NET_CREDITS: return(State.Credits);
		default: return(0);
	}
}


void UINetLobbyPresenterClass::Refresh(void)
{
	Service.Read(State);

	// A packet can take the flow to another lobby or start the game while the screen is up,
	// which is the one thing a modal screen cannot do for itself.
	if (!Result.has_value() && (State.Kind != Opened || State.Answered)) {
		Result = UI_RESULT_ACCEPTED;
	}

	Mark_Picked();
}


void UINetLobbyPresenterClass::Mark_Picked(void)
{
	for (UINetPlayerRow & row : State.Players) {
		row.Selected = std::find(Picked.begin(), Picked.end(), row.Name) != Picked.end();
	}
}


namespace
{

// Everything both documents show. Each view binds the members its own document names.
class UINetLobbyViewClass : public UIRmlViewClass
{
	public:
		UINetLobbyViewClass(UINetLobbyPresenterClass & presenter, char const * document) :
			UIRmlViewClass(presenter, document, "netlobby"),
			Data(presenter),
			Shown(-1)
		{
		}

		// Coming back from the map dialog, the lobby is shown whole rather than opened.
		virtual void Loaded(void) override
		{
			if (!Data.State.Reveal && Document() != nullptr) {
				Document()->SetAttribute("reveal", Rml::String("none"));
			}
		}

	protected:
		bool Bind_Shared(Rml::DataModelConstructor & model)
		{
			Rml::StructHandle<UINetOption> option = model.RegisterStruct<UINetOption>();
			Rml::StructHandle<UINetChatLine> line = model.RegisterStruct<UINetChatLine>();
			Rml::StructHandle<UINetPlayerRow> row = model.RegisterStruct<UINetPlayerRow>();
			if (!option || !line || !row) {
				return(false);
			}

			option.RegisterMember("label", &UINetOption::Label);
			option.RegisterMember("value", &UINetOption::Value);
			option.RegisterMember("colour", &UINetOption::Colour);
			line.RegisterMember("text", &UINetChatLine::Text);
			line.RegisterMember("colour", &UINetChatLine::Colour);
			row.RegisterMember("name", &UINetPlayerRow::Name);
			row.RegisterMember("colour", &UINetPlayerRow::Colour);
			row.RegisterMember("house", &UINetPlayerRow::House);
			row.RegisterMember("hint", &UINetPlayerRow::Hint);
			row.RegisterMember("mark", &UINetPlayerRow::Mark);
			row.RegisterMember("selected", &UINetPlayerRow::Selected);

			UINetLobbyState & state = Data.State;
			return(model.RegisterArray<std::vector<UINetOption>>()
				&& model.RegisterArray<std::vector<UINetChatLine>>()
				&& model.RegisterArray<std::vector<UINetPlayerRow>>()
				&& model.Bind("players", &state.Players)
				&& model.Bind("chat", &state.Chat)
				&& model.Bind("say", &state.Say));
		}

		void Sync_Shared(void)
		{
			Model.DirtyVariable("players");
			Model.DirtyVariable("chat");

			// The line the player is typing is pushed back so that sending one empties the box.
			Model.DirtyVariable("say");

			Show_Newest_Chat();
		}

		// The list box the layer drew forced its newest line into view on every rebuild, so the
		// log cannot be scrolled back in either presentation.
		void Show_Newest_Chat(void)
		{
			Rml::Element * chat = (Document() != nullptr) ? Document()->GetElementById("chat") : nullptr;
			if (chat != nullptr) {
				chat->SetScrollTop(chat->GetScrollHeight());
			}
		}

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

		UINetLobbyPresenterClass & Data;

	private:
		int Shown;
};


// The browser: the games on the wire, everyone chatting, and the name they see.
class UINetBrowserViewClass : public UINetLobbyViewClass
{
	public:
		explicit UINetBrowserViewClass(UINetLobbyPresenterClass & presenter) :
			UINetLobbyViewClass(presenter, "netlobby.rml")
		{
		}

		virtual void Sync(void) override
		{
			Sync_Shared();
			Model.DirtyVariable("games");
			Model.DirtyVariable("game");
			Model.DirtyVariable("handle");
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			UINetLobbyState & state = Data.State;
			return(Bind_Shared(model)
				&& model.Bind("games", &state.Games)
				&& model.Bind("game", &state.Game)
				&& model.Bind("handle", &state.Handle));
		}
};


// The setup, as its host and as a guest. The guest's document is the host's with the options
// turned off, which is how the two templates differ.
class UINetSetupViewClass : public UINetLobbyViewClass
{
	public:
		explicit UINetSetupViewClass(UINetLobbyPresenterClass & presenter) :
			UINetLobbyViewClass(presenter, "netgame.rml")
		{
		}

		virtual void Loaded(void) override
		{
			UINetLobbyViewClass::Loaded();
			if (Document() != nullptr) {
				Document()->SetClass("host", Data.State.Kind == UI_NET_LOBBY_HOST);
				Document()->SetClass("guest", Data.State.Kind != UI_NET_LOBBY_HOST);
			}
		}

		virtual void Sync(void) override
		{
			Sync_Shared();
			Model.DirtyVariable("host");
			Model.DirtyVariable("side");
			Model.DirtyVariable("colour");
			Model.DirtyVariable("mapname");
			Model.DirtyVariable("bases");
			Model.DirtyVariable("crates");
			Model.DirtyVariable("shortgame");
			Model.DirtyVariable("allies");
			Model.DirtyVariable("truce");
			Model.DirtyVariable("fog");
			Model.DirtyVariable("bridges");
			Model.DirtyVariable("engineer");
			Model.DirtyVariable("redeploy");
			Model.DirtyVariable("speed");
			Model.DirtyVariable("aiplayers");
			Model.DirtyVariable("ailevel");
			Model.DirtyVariable("units");
			Model.DirtyVariable("tech");
			Model.DirtyVariable("credits");
			Model.DirtyVariable("canaccept");
			Model.DirtyVariable("cango");

			Show_Preview();
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			UINetLobbyState & state = Data.State;
			return(Bind_Shared(model)
				&& model.Bind("host", &state.Host)
				&& model.Bind("sides", &state.Sides)
				&& model.Bind("side", &state.Side)
				&& model.Bind("colours", &state.Colours)
				&& model.Bind("colour", &state.Colour)
				&& model.Bind("mapname", &state.MapName)
				&& model.Bind("bases", &state.Bases)
				&& model.Bind("crates", &state.Crates)
				&& model.Bind("shortgame", &state.ShortGame)
				&& model.Bind("allies", &state.Allies)
				&& model.Bind("truce", &state.HarvesterTruce)
				&& model.Bind("fog", &state.FogOfWar)
				&& model.Bind("bridges", &state.Bridges)
				&& model.Bind("engineer", &state.MultiEngineer)
				&& model.Bind("redeploy", &state.Redeploy)
				&& model.Bind("speed", &state.GameSpeed)
				&& model.Bind("aiplayers", &state.AIPlayers)
				&& model.Bind("aiplayersmax", &state.AIPlayersMax)
				&& model.Bind("ailevel", &state.AILevel)
				&& model.Bind("units", &state.UnitCount)
				&& model.Bind("unitsmin", &state.UnitCountMin)
				&& model.Bind("unitsmax", &state.UnitCountMax)
				&& model.Bind("tech", &state.TechLevel)
				&& model.Bind("techmax", &state.TechLevelMax)
				&& model.Bind("credits", &state.Credits)
				&& model.Bind("creditsmin", &state.CreditsMin)
				&& model.Bind("creditsmax", &state.CreditsMax)
				&& model.Bind("creditsstep", &state.CreditsStep)
				&& model.Bind("canaccept", &state.CanAccept)
				&& model.Bind("cango", &state.CanGo));
		}
};

}


std::unique_ptr<UIViewClass> UI_Net_Browser_View(UINetLobbyPresenterClass & presenter)
{
	return(std::make_unique<UINetBrowserViewClass>(presenter));
}


std::unique_ptr<UIViewClass> UI_Net_Setup_View(UINetLobbyPresenterClass & presenter)
{
	return(std::make_unique<UINetSetupViewClass>(presenter));
}
