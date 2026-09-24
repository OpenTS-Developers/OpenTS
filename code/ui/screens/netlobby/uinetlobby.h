/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ui/uipreview.h"
#include "ui/uiscreen.h"

#include <memory>
#include <string>
#include <vector>

class UIViewClass;


enum UINetLobbyKind
{
	UI_NET_LOBBY_NONE,
	UI_NET_LOBBY_GAMES,
	UI_NET_LOBBY_HOST,
	UI_NET_LOBBY_GUEST
};


enum UINetChoice
{
	UI_NET_NONE,
	UI_NET_CANCEL,
	UI_NET_GO,
	UI_NET_STARTED,
};


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


struct UINetOption
{
	std::string Label;
	int Value = 0;
	std::string Color;
};


struct UINetChatLine
{
	std::string Text;
	std::string Color;
};


struct UINetPlayerRow
{
	std::string Name;
	std::string Color;
	std::string House;
	std::string Hint;
	std::string Mark;
	bool Selected = false;
};


struct UINetLobbyState
{
	UINetLobbyKind Kind = UI_NET_LOBBY_NONE;
	bool Host = false;

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

	bool CanAccept = false;
	bool CanGo = true;
};


class UINetLobbyServiceClass
{
	public:
		virtual ~UINetLobbyServiceClass(void) = default;
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
		virtual void Pick_Map(void) = 0;

		virtual void Join(void) = 0;
		virtual void Host(void) = 0;
		virtual bool Can_Start(void) = 0;
};


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

		std::vector<std::string> Picked;
};


std::unique_ptr<UIViewClass> UI_Net_Browser_View(UINetLobbyPresenterClass & presenter);
std::unique_ptr<UIViewClass> UI_Net_Setup_View(UINetLobbyPresenterClass & presenter);

UINetLobbyServiceClass & UI_Net_Lobby_Service(void);
UINetChoice UI_Net_Lobby_Run(void);
