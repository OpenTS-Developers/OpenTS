/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "launch/launchmessages.h"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>


namespace Launch
{
	// Where in a request a fault lies, for a caller that reports it in the terms of another file.
	struct FaultSite {
		int Player = -1;        // a position in the player list, or -1
		std::string Field;      // a field of launch.proto, or "options." and an option's key; empty for none
	};

	class Game;
	class Machine;
}


/*
 * A launched game in the values every machine in it shares: the request form of launch.proto's
 * Game message, checked and typed. It reads no engine state, so a request can be judged before
 * anything is applied and without the game running.
 */
class Launch::Game
{
	public:

		// The most players a game takes, and the most start positions a map has.
		static constexpr int PLAYER_MAX = 8;
		static constexpr int START_MAX = 8;

		// Copies of the engine's counts and sizes; launcherapply.cpp checks each against the engine's.
		static constexpr int GAME_SPEED_COUNT = 7;
		static constexpr int DIFFICULTY_COUNT = 3;
		static constexpr int NAME_LENGTH_MAX = 63;

		// A player as the request gives it. Allies are positions in the player list, ascending.
		struct PlayerType {
			std::string Name;
			bool Human = false;
			bool Observer = false;
			int Country = 0;
			int Color = 0;
			int Start = 0;
			std::optional<int> AILevel;
			std::vector<int> Allies;
		};

		// One member per option key, set to the value a request gets when it leaves the key out.
		struct OptionsType {
			bool AlliesAllowed = false;
			bool AttackNeutralUnits = false;
			bool AutoDeployMCV = false;
			bool Bases = true;
			bool BridgeDestruction = true;
			bool BuildOffAlly = false;
			bool CoachMode = false;
			bool Crates = false;
			bool FogOfWar = false;
			bool HarvesterTruce = false;
			bool MCVRedeploy = true;
			bool MultiEngineer = false;
			bool PlayMovies = false;
			bool ScrapMetal = false;
			bool SharedControl = false;
			bool ShortGame = false;
			bool SurrenderOnDisconnect = true;
			int AIDifficulty = 1;
			int Credits = 10000;
			int TechLevel = 10;
			int UnitCount = 0;

			bool Firestorm = true;
			int GameSpeed = 0;

			int Campaign = -1;
			int CampaignPlayerDifficulty = 1;
			int CampaignComputerDifficulty = 1;
		};

		enum class OptionType {
			FLAG,
			NUMBER,
		};

		// Bits of OptionEntry::Kinds.
		static constexpr unsigned KIND_CAMPAIGN = 1u << 0;
		static constexpr unsigned KIND_SKIRMISH = 1u << 1;
		static constexpr unsigned KIND_MULTIPLAYER = 1u << 2;

		// An option the engine reads, the kinds of game it applies to, and the values it accepts.
		struct OptionEntry {
			char const * Key;
			OptionType Type;
			unsigned Kinds;
			std::variant<bool OptionsType::*, int OptionsType::*> Member;
			std::int64_t Lowest;
			std::int64_t Highest;
		};

		// Counts the rules, the campaign list, and a scenario give, passed in because only a running game has them.
		struct LimitsType {
			int Countries = 0;
			int Colors = 0;
			int Campaigns = 0;
			int GlobalFlags = 0;
		};

		static std::span<OptionEntry const> Option_Table(void);

		bool From_Message(Launch::Message::Game const & message, LimitsType const & limits, std::string & fault, Launch::FaultSite * site = nullptr);
		Launch::Message::Game To_Message(void) const;
		int Human_Count(void) const;

		Launch::GameKind Kind = Launch::GameKind::GAME_KIND_UNKNOWN;
		std::optional<std::uint32_t> Seed;
		std::vector<PlayerType> Players;
		OptionsType Options;
		bool Anonymous = false;
		// The scenario's global flags a campaign mission starts with set, ascending.
		std::vector<int> GlobalFlags;
};


/*
 * This machine's part of a game a launcher set up: who plays here, the map file to open, and how
 * to reach the other machines. None of it is shared or recorded in LNCH.
 */
class Launch::Machine
{
	public:

		// Bounds and defaults of the connection waits, in 16 ms ticks.
		static constexpr int TIMEOUT_MIN = 60;
		static constexpr int TIMEOUT_MAX = 36000;
		static constexpr int CONNECT_TIMEOUT_DEFAULT = 3600;
		static constexpr int RECONNECT_TIMEOUT_DEFAULT = 2400;

		struct PeerType {
			int Player = 0;
			std::string Address;
			int Port = 0;
		};

		struct TunnelType {
			std::string Address;
			int Port = 0;
		};

		struct NetworkType {
			int Port = 0;
			std::vector<PeerType> Peers;
			std::optional<TunnelType> Tunnel;
			int ConnectTimeout = CONNECT_TIMEOUT_DEFAULT;
			int ReconnectTimeout = RECONNECT_TIMEOUT_DEFAULT;
		};

		bool From_Request(Launch::Message::Play const & request, Launch::Game const & game, std::string & fault, Launch::FaultSite * site = nullptr);
		bool From_Load(Launch::Message::Load const & request, bool network, std::string & fault, Launch::FaultSite * site = nullptr);

		static int Timeout_Ticks(std::uint32_t milliseconds, int fallback);

		int LocalPlayer = 0;
		bool Host = false;
		std::string MapPath;
		std::string MapName;
		std::string GameMode;
		int AutosaveInterval = 0;
		int NextCampaignAutosave = 0;
		int NextSkirmishAutosave = 0;
		bool SkipScoreScreen = false;
		bool WriteStatistics = false;
		std::uint32_t GameId = 0;
		std::string MapHash;
		std::string DifficultyName;
		std::string LoadScreen;
		std::optional<int> LoadScreenX;
		std::optional<int> LoadScreenY;
		std::optional<NetworkType> Network;
};


namespace Launch
{
	bool Fault(std::string & fault, char const * format, ...);
	bool Is_Address(std::string const & text);
	bool Is_Tunnel_Number(int value);

	bool Judge_Request(RequestFault fault, Message::Request const & request, char const * conflict, std::string & refusal);

	// Why a scenario of a launched game ended, as the engine knows it.
	enum class ScenarioEnd {
		LEFT,               // the local player left, or the scenario stopped undecided
		WON,
		LOST,
		DESYNC,             // the machines' games differed, or an event arrived after its frame ran
		CONNECTION_LOST,    // the other machines stopped answering, or voted this machine out
		WAIT_CANCELLED,     // the local player stopped waiting for the other machines
		START_MISMATCH,     // the machines' start states differed
	};

	Message::ScenarioResult Scenario_Result(std::string const & map_path);
	void Close_Scenario(Message::ScenarioResult & entry, ScenarioEnd end, bool joined, bool observer, std::uint32_t frame);
	ResultCode Scenario_End_Code(ScenarioEnd end, bool joined);
	bool Local_Win(bool campaign, bool player_wins, bool local_defeated);
}
