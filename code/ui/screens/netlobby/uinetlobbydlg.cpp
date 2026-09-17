/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the network lobbies: what the wire holds, what the player's answers do to
// the session, and the pass the game runs under the screen.

#include "ui/screens/netlobby/uinetlobby.h"

#include "_rules.h"
#include "_ui.h"
#include "conquer.h"
#include "data.h"
#include "globals.h"
#include "houstype.h"
#include "ipxmgr.h"
#include "language/language.h"
#include "netdlg.h"
#include "netdlg2.h"
#include "netshare.h"
#include "rules.h"
#include "session.h"
#include "ui/uienginehost.h"
#include "ui/uipreview.h"
#include "ui/uishell.h"
#include "ui/uiview.h"
#include "win.h"

#include <cstdio>
#include <cstring>
#include <utility>


// The least a network game can be played for, as the dialog's constant has it.
static int const UI_NET_MIN_MONEY = 2500;

// What a list row is drawn in when nothing has colored it, which is what the list box left
// a row at. A document is told this rather than nothing, because a style of nothing is an
// error to the toolkit and it complains about one on every pass.
static char const UI_NET_PLAIN[] = "#70ff00";

// The colors a player may wear, in the order the dialogs list them.
static int const UI_NET_COLORS[] = {
	TXT_GOLD, TXT_RED, TXT_BLUE, TXT_GREEN, TXT_ORANGE, TXT_SKY_BLUE, TXT_PURPLE, TXT_PINK
};


static UINetLobbyKind Kind_Of(Net2LobbyPhaseType phase)
{
	switch (phase) {
		case NET2_LOBBY_GAME_LIST: return(UI_NET_LOBBY_GAMES);
		case NET2_LOBBY_HOST: return(UI_NET_LOBBY_HOST);
		case NET2_LOBBY_GUEST: return(UI_NET_LOBBY_GUEST);
		default: return(UI_NET_LOBBY_NONE);
	}
}


namespace
{

// Reaches the session, the scenario list and the packet queues. Every method does what the
// matching Win32 handler does, in the same order and with the same packets.
class UINetLobbyEngineServiceClass : public UINetLobbyServiceClass
{
	public:
		virtual void Read(UINetLobbyState & state) override;

		virtual void Set_Handle(char const * name) override;
		virtual void Select_Game(int index) override;
		virtual void Say(char const * text) override;
		virtual void Set_Side(int index) override;
		virtual void Set_Color(int index) override;
		virtual void Set_Switch(UINetSwitch which, bool on) override;
		virtual void Set_Slider(UINetSlider which, int value) override;
		virtual void Kick(std::vector<std::string> const & names) override;
		virtual void Accept(void) override;
		virtual void Pick_Map(void) override;
		virtual void Join(void) override;
		virtual void Host(void) override;
		virtual bool Can_Start(void) override;
};


void UINetLobbyEngineServiceClass::Read(UINetLobbyState & state)
{
	state.Kind = Kind_Of(Net2LobbyPhase);
	state.Host = state.Kind == UI_NET_LOBBY_HOST;
	state.Answered = Net2Response() != UI_NET_NONE;
	state.Handle = Session.Handle;

	// The browser lists the lobby and then every game being advertised, as the layer named them.
	state.Games.clear();
	for (int index = 0; index < Session.Games.Count(); index++) {
		char buffer[80];
		if (index == 0) {
			std::snprintf(buffer, sizeof(buffer), "%s", Fetch_String(TXT_LOBBY));
		} else {
			NodeNameType * node = Session.Games[index];
			std::snprintf(buffer, sizeof(buffer), node->Game.IsOpen ? Fetch_String(TXT_THATGUYS_GAME) : Fetch_String(TXT_THATGUYS_GAME_BRACKET), node->Name);
		}

		UINetOption option;
		option.Label = buffer;
		state.Games.push_back(option);
	}

	// The current selection is clamped and re-applied, since a game can vanish out from
	// under the player at any moment.
	if (Net2CurrentGame() >= (int)state.Games.size()) {
		state.Game = (int)state.Games.size() - 1;
	} else {
		state.Game = Net2CurrentGame() < 0 ? 0 : Net2CurrentGame();
	}

	// Out in the lobby the list is everyone chatting; inside a game it is the players, each
	// row carrying its color, its side's emblem and the marker for the host or for an
	// accepted player, which is what the columned list box drew.
	state.Players.clear();
	if (state.Kind == UI_NET_LOBBY_GAMES && Net2CurrentGame() == 0) {
		UINetPlayerRow row;
		row.Name = Session.Handle;
		row.Color = UI_NET_PLAIN;
		state.Players.push_back(row);

		for (int index = 1; index < Session.Chat.Count(); index++) {
			UINetPlayerRow other;
			other.Name = Session.Chat[index]->Name;
			other.Color = UI_NET_PLAIN;
			state.Players.push_back(other);
		}
	} else {
		for (int index = 0; index < Session.Players.Count(); index++) {
			NodeNameType * who = Session.Players[index];

			UINetPlayerRow row;
			row.Name = who->Name;
			row.Color = UI_Color_Text(PlayerColorTable[who->Player.Color]);

			// Only two emblems ship, so every side past the first borrows the second's.
			int country = who->Player.House;
			SideType side = (country >= HOUSE_FIRST && country < HouseTypes.Count()) ? HouseTypes[country]->Side : SIDE_NONE;
			if (side == SIDE_GDI) {
				row.House = "gdii.pcx";
				row.Hint = Fetch_String(TXT_GDI);
			} else {
				row.House = "nodi.pcx";
				row.Hint = (side == SIDE_NOD || side == SIDE_NONE) ? Fetch_String(TXT_NOD) : (char const *)HouseTypes[country]->GivenName;
			}

			// The host counts as having accepted its own game. The Win32 dialogs settle this
			// while drawing the row, and the rest of the flow reads it back, so it is settled
			// here for the same reason.
			if (std::strcmp(who->Name, Session.GameName) == 0) {
				who->Player.Status = 1;
				row.Mark = "wolhost.pcx";
			} else if (who->Player.Status != 0) {
				row.Mark = "wolacpt.pcx";
			}

			state.Players.push_back(row);
		}
	}

	state.Chat.clear();
	for (NetChatLineType const & line : Net2_Chat_Log()) {
		UINetChatLine copy;
		copy.Text = line.Text;
		copy.Color = (line.Color == -1) ? UI_NET_PLAIN : UI_Color_Text((COLORREF)line.Color);
		state.Chat.push_back(copy);
	}

	state.Sides.clear();
	state.Side = 0;
	for (int index = 0; index < HouseTypes.Count(); index++) {
		HouseTypeClass * house = HouseTypes[index];
		if (!house->IsMultiplay) {
			continue;
		}

		UINetOption option;
		option.Label = (char const *)house->GivenName;
		option.Value = index;
		if (index == Session.House) {
			state.Side = (int)state.Sides.size();
		}
		state.Sides.push_back(option);
	}

	int const palette = (int)(sizeof(UI_NET_COLORS) / sizeof(UI_NET_COLORS[0]));
	state.Colors.clear();
	for (int index = 0; index < MAX_MPLAYER_COLORS && index < palette; index++) {
		UINetOption option;
		option.Label = Fetch_String(UI_NET_COLORS[index]);
		option.Value = index;
		option.Color = UI_Color_Text(PlayerColorTable[index]);
		state.Colors.push_back(option);
	}
	state.Color = (Session.ColorIdx >= 0 && Session.ColorIdx < (int)state.Colors.size()) ? Session.ColorIdx : 0;

	state.MapName = Session.Options.ScenarioDescription;
	UI_Map_Preview_Image(state.Preview);

	state.Bases = Session.Options.Bases;
	state.Crates = Session.Options.Goodies;
	state.ShortGame = Session.Options.ShortGame;
	state.Allies = Session.Options.AlliesAllowed;
	state.HarvesterTruce = Session.Options.HarvTruce;
	state.FogOfWar = Session.Options.FogOfWar;
	state.Bridges = Session.Options.BridgeDestruction;
	state.MultiEngineer = Session.Options.CrapEngineers;
	state.Redeploy = Session.Options.MCVRedeploy;

	// The track counts the other way from the option it sets: the fastest game is its far end.
	state.GameSpeed = 6 - Session.Options.GameSpeed;
	// The bounds are the network dialog's own, which are not the skirmish setup's.
	state.AIPlayers = Session.Options.AIPlayers;
	state.AIPlayersMax = 6;
	state.AILevel = Session.Options.AIDifficulty;
	state.UnitCountMin = 1;
	state.UnitCountMax = 10;
	state.UnitCount = Session.Options.UnitCount;
	state.TechLevelMax = MPLAYER_BUILD_LEVEL_MAX;
	state.TechLevel = BuildLevel;
	state.CreditsMin = UI_NET_MIN_MONEY;
	state.CreditsMax = Rule->MPMaxMoney;
	state.CreditsStep = 100;
	state.Credits = Session.Options.Credits;

	// The guest may accept until the host changes something, which clears every acceptance.
	state.CanAccept = state.Kind == UI_NET_LOBBY_GUEST && Session.Players.Count() > 0 && Session.Players[0]->Player.Status == 0;
	state.CanGo = true;
}


void UINetLobbyEngineServiceClass::Set_Handle(char const * name)
{
	Net2Set_Handle(name);
}


void UINetLobbyEngineServiceClass::Select_Game(int index)
{
	Net2Select_Game(index);
}


void UINetLobbyEngineServiceClass::Say(char const * text)
{
	Net2Send_Chat(text);
}


void UINetLobbyEngineServiceClass::Set_Side(int index)
{
	int country = Net2Country_At(index);
	if (country < 0) {
		return;
	}

	if (Net2LobbyPhase == NET2_LOBBY_HOST) {
		Session.House = country;
		if (Session.Players.Count() > 0) {
			Session.Players[0]->Player.House = Session.House;
		}
		PumpGameopts(1, 0);
	} else {
		Session.House = country;
		Net2Request_House_And_Color(country, Session.ColorIdx);
	}
}


void UINetLobbyEngineServiceClass::Set_Color(int index)
{
	if (index < 0 || index >= MAX_MPLAYER_COLORS) {
		return;
	}

	if (Net2LobbyPhase == NET2_LOBBY_HOST) {
		Net2Host_Take_Color(index);
	} else {
		Session.PrefColor = index;
		Net2Request_House_And_Color(Session.House, index);
	}
}


void UINetLobbyEngineServiceClass::Set_Switch(UINetSwitch which, bool on)
{
	switch (which) {
		case UI_NET_BASES: Session.Options.Bases = on; break;
		case UI_NET_CRATES: Session.Options.Goodies = on; break;
		case UI_NET_SHORT_GAME: Session.Options.ShortGame = on; break;
		case UI_NET_ALLIES: Session.Options.AlliesAllowed = on; break;
		case UI_NET_HARVESTER_TRUCE: Session.Options.HarvTruce = on; break;
		case UI_NET_FOG_OF_WAR: Session.Options.FogOfWar = on; break;
		case UI_NET_BRIDGES: Session.Options.BridgeDestruction = on; break;
		case UI_NET_ENGINEER: Session.Options.CrapEngineers = on; break;
		case UI_NET_REDEPLOY: Session.Options.MCVRedeploy = on; break;
		default: return;
	}

	PumpGameopts(1, 0);
}


void UINetLobbyEngineServiceClass::Set_Slider(UINetSlider which, int value)
{
	switch (which) {
		case UI_NET_SPEED: Session.Options.GameSpeed = 6 - value; break;
		case UI_NET_AI_PLAYERS: Session.Options.AIPlayers = value; break;
		case UI_NET_AI_LEVEL: Session.Options.AIDifficulty = (DiffType)value; break;
		case UI_NET_UNITS: Session.Options.UnitCount = value; break;
		case UI_NET_TECH: BuildLevel = value; break;
		case UI_NET_CREDITS: Session.Options.Credits = value; break;
		default: return;
	}

	PumpGameopts(1, 0);
}


void UINetLobbyEngineServiceClass::Kick(std::vector<std::string> const & names)
{
	for (std::string const & name : names) {
		Net2Kick(name.c_str());
	}

}


void UINetLobbyEngineServiceClass::Accept(void)
{
	if (Session.Players.Count() == 0) {
		return;
	}

	Session.Players[0]->Player.Status = 1;
	SendPublicGameopts("A1");
}


void UINetLobbyEngineServiceClass::Pick_Map(void)
{
	Net2Pick_Map();
}


void UINetLobbyEngineServiceClass::Join(void)
{
	Net2Join_Game();
}


void UINetLobbyEngineServiceClass::Host(void)
{
	Net2Host_Game();
}


bool UINetLobbyEngineServiceClass::Can_Start(void)
{
	return(Net2Can_Start());
}

}


UINetLobbyServiceClass & UI_Net_Lobby_Service(void)
{
	static UINetLobbyEngineServiceClass service;
	return(service);
}


/// <summary>
/// Shows the lobby the flow is standing in, until the player answers or a packet moves the
/// flow on under it.
/// </summary>
/// <returns>Go or cancel. Nothing comes back when the flow moved on under the screen, or
/// from a flow standing in no phase, which the lobby is serviced through instead; a screen
/// that could not open answers as a cancel does, which backs the flow out.</returns>
UINetChoice UI_Net_Lobby_Run(void)
{
	UINetLobbyState state;
	UI_Net_Lobby_Service().Read(state);

	// A packet can leave the flow between phases; there is no screen to run then, so the pass
	// the screen's runner would have made is made here.
	if (state.Kind == UI_NET_LOBBY_NONE) {
		Net2_Service_Lobby();
		return(UI_NET_NONE);
	}

	UINetLobbyPresenterClass presenter(UI_Net_Lobby_Service(), std::move(state));
	std::unique_ptr<UIViewClass> view = (presenter.State.Kind == UI_NET_LOBBY_GAMES)
		? UI_Net_Browser_View(presenter)
		: UI_Net_Setup_View(presenter);

	if (UIShell.Run_Modal(*view, Net2_Service_Lobby) == UI_RESULT_FAILED_TO_OPEN) {
		return(UI_NET_CANCEL);
	}

	return(presenter.Choice);
}
