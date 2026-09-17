/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The three network lobby screens: the game browser, and the setup as its host and as a
// guest. No window, control, RmlUi or engine type appears here.

#pragma once

#include "ui/uipreview.h"
#include "ui/uiscreen.h"

#include <memory>
#include <string>
#include <vector>

class UIViewClass;


// Which lobby the flow stands in. The engine maps its phase onto this, so nothing here
// needs the network headers.
enum UINetLobbyKind
{
	UI_NET_LOBBY_NONE,
	UI_NET_LOBBY_GAMES,
	UI_NET_LOBBY_HOST,
	UI_NET_LOBBY_GUEST
};


// What the lobby flow is answering: the two a screen gives, and the ones a packet gives when
// a guest is thrown out or the host starts the game under a guest's lobby.
enum UINetChoice
{
	UI_NET_NONE,
	UI_NET_CANCEL,
	UI_NET_GO,
	UI_NET_STARTED,
};


// The nine switches the host owns, named rather than numbered so that the service can answer
// each one without knowing the document's order.
enum UINetSwitch
{
	UI_NET_BASES,
	UI_NET_CRATES,
	UI_NET_SHORT_GAME,
	UI_NET_ALLIES,
	UI_NET_HARVESTER_TRUCE,
	UI_NET_FOG_OF_WAR,
	UI_NET_BRIDGES,
	UI_NET_ENGINEER,
	UI_NET_REDEPLOY
};


enum UINetSlider
{
	UI_NET_SPEED,
	UI_NET_AI_PLAYERS,
	UI_NET_AI_LEVEL,
	UI_NET_UNITS,
	UI_NET_TECH,
	UI_NET_CREDITS
};


// One row of a chooser: what it reads and the color the dropdown draws it in, which only the
// player colors carry.
struct UINetOption
{
	std::string Label;
	int Value = 0;
	std::string Color;
};


// One message as the lobby produced it. The document breaks it into lines itself, where the
// Win32 list box was handed lines already broken.
struct UINetChatLine
{
	std::string Text;
	std::string Color;
};


// One row of the player list. In the browser only the name is set; in a game the row also
// carries the color its name is drawn in, the side's emblem with the side's name as its
// tooltip, and the marker for the host or for a player who has accepted.
struct UINetPlayerRow
{
	std::string Name;
	std::string Color;
	std::string House;
	std::string Hint;
	std::string Mark;
	bool Selected = false;
};


// What a lobby screen shows. The service fills everything but Say and Selected, which belong
// to the screen for as long as it is open.
struct UINetLobbyState
{
	UINetLobbyKind Kind = UI_NET_LOBBY_NONE;
	bool Host = false;

	// The flow moved on without the player: the host started the game, or a packet threw the
	// guest back to the browser.
	bool Answered = false;

	std::string Handle;
	std::string Say;

	std::vector<UINetOption> Games;
	int Game = 0;

	std::vector<UINetPlayerRow> Players;
	std::vector<UINetChatLine> Chat;

	std::vector<UINetOption> Sides;
	int Side = 0;
	std::vector<UINetOption> Colors;
	int Color = 0;

	std::string MapName;
	UIMapPreviewImage Preview;

	bool Bases = true;
	bool Crates = true;
	bool ShortGame = false;
	bool Allies = true;
	bool HarvesterTruce = false;
	bool FogOfWar = false;
	bool Bridges = true;
	bool MultiEngineer = false;
	bool Redeploy = true;

	int GameSpeed = 0;
	int AIPlayers = 0;
	int AIPlayersMax = 7;
	int AILevel = 0;
	int UnitCount = 0;
	int UnitCountMin = 0;
	int UnitCountMax = 0;
	int TechLevel = 1;
	int TechLevelMax = 1;
	int Credits = 0;
	int CreditsMin = 0;
	int CreditsMax = 0;
	int CreditsStep = 1;

	// The guest may accept the host's settings, and may not accept them twice.
	bool CanAccept = false;
	// The host may start; refused while the game is too small or somebody has not accepted.
	bool CanGo = true;
};


// What the lobby screens ask of the game. The game supplies one that reaches the session and
// the packet queues; the test harness supplies one that records the calls.
class UINetLobbyServiceClass
{
	public:
		virtual ~UINetLobbyServiceClass(void) = default;

		// Copies what the lobby holds into the model, leaving the screen's fields alone.
		virtual void Read(UINetLobbyState & state) = 0;

		virtual void Set_Handle(char const * name) = 0;
		virtual void Select_Game(int index) = 0;
		virtual void Say(char const * text) = 0;
		virtual void Set_Side(int index) = 0;
		virtual void Set_Color(int index) = 0;
		virtual void Set_Switch(UINetSwitch which, bool on) = 0;
		virtual void Set_Slider(UINetSlider which, int value) = 0;
		virtual void Kick(std::vector<std::string> const & names) = 0;
		virtual void Accept(void) = 0;

		// Runs the map dialog over the host's setup, which stays open behind it.
		virtual void Pick_Map(void) = 0;

		// Asks to join the picked game; the answer arrives as a packet while the browser stays up.
		virtual void Join(void) = 0;

		// Opens the player's game, or refuses the name in a box over the browser.
		virtual void Host(void) = 0;

		// Whether the host may start; a refusal is printed into the chat.
		virtual bool Can_Start(void) = 0;
};


// Carries one lobby until the player answers or the flow moves on under it. The same presenter
// serves all three, because the three share their chat, their player list and their answers;
// which of them is open decides only what its document shows.
class UINetLobbyPresenterClass : public UIPresenterClass
{
	public:
		UINetLobbyPresenterClass(UINetLobbyServiceClass & service, UINetLobbyState state);

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UINetLobbyState State;
		UINetChoice Choice = UI_NET_NONE;

	private:
		void Toggle(UINetSwitch which, bool on);
		void Move(UINetSlider which, int value);
		int Reading(UINetSlider which) const;
		void Mark_Picked(void);

		UINetLobbyServiceClass & Service;
		UINetLobbyKind Opened;

		// Who is picked for the kick, by name: the list is rebuilt from the wire every pass,
		// so a row number would not survive it. The Win32 list saves its selection the same way.
		std::vector<std::string> Picked;
};


// The RmlUi views over a lobby presenter. The browser is netlobby.rml and the setup, as host
// or as guest, is netgame.rml. The presenter must outlive either.
std::unique_ptr<UIViewClass> UI_Net_Browser_View(UINetLobbyPresenterClass & presenter);
std::unique_ptr<UIViewClass> UI_Net_Setup_View(UINetLobbyPresenterClass & presenter);

// The game's lobby service, and the screen the network driver runs over it. Both live in
// uinetlobbydlg.cpp, which is the only part of the screen that knows the engine.
UINetLobbyServiceClass & UI_Net_Lobby_Service(void);
UINetChoice UI_Net_Lobby_Run(void);
