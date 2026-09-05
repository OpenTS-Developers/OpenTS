/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#pragma once

#include "house.hh"

#include <array>
#include <string>

class INIClass;


/*
 * What a client asked the game to launch. The file's spelling, defaults and shape belong to
 * the CnCNet client; reading cannot fail, and the result is judged at the launch.
 */
class SpawnerConfigClass
{
	public:

		// Counts changes to what the game makes of a launch file, never the file's own vocabulary.
		static constexpr int SCHEMA_VERSION = 2;

		// One seat per house a match may hold, and the fifty scenario flags the engine keeps.
		static constexpr int SLOT_COUNT = 8;
		static constexpr int GLOBAL_FLAG_COUNT = 50;

		// Resume overrides the rest: a saved game carries its own type, options and houses.
		enum class LaunchType {
			Skirmish,
			Campaign,
			Multiplayer,
			Resume,
		};

		// A file marks a seat human by writing a section for it; an unwritten one is a computer.
		enum class OccupancyType {
			Empty,
			Human,
			Computer,
		};

		/*
		 * A seat is held in the order the houses are created in, so its index is the house it
		 * becomes, which is what alliances and start positions name.
		 */
		struct SlotType {
			OccupancyType Occupancy = OccupancyType::Empty;
			std::string Name;
			int Color = -1;
			int Country = -1;
			int Handicap = -1;
			bool IsSpectator = false;
			int StartingPosition = -1;
			std::array<int, SLOT_COUNT> Alliances = {-1, -1, -1, -1, -1, -1, -1, -1};
			std::string Address = "0.0.0.0";
			int Port = 0;
		};

		void Read_INI(INIClass const & ini);
		LaunchType Launch_Type(void) const;
		int Session_Identity_CRC(void) const;

		// The rules' tables are handed in, so a reading can be judged without the game running.
		bool Is_Playable(int countries, int colors, std::string & fault) const;

		static int Playable_Handicap(int asked);

		// What kind of game to start.
		bool IsCampaign = false;
		bool IsHost = false;
		int CampaignID = -1;
		int Tournament = 0;
		int GameID = 0;

		// The scenario and the saved game.
		std::string ScenarioName = "spawnmap.ini";
		std::string MapName;
		bool LoadSaveGame = false;
		std::string SaveGameName;
		int AutoSaveInterval = 0;
		int NextCampaignAutoSave = 0;
		int NextSkirmishAutoSave = 0;

		// The options every house plays under.
		bool Bases = true;
		int Credits = 10000;
		bool BridgeDestroy = true;
		bool Crates = false;
		bool ShortGame = false;
		bool BuildOffAlly = false;
		int GameSpeed = 0;
		bool MultiEngineer = false;
		int UnitCount = 0;
		int AIPlayers = 0;
		int AIDifficulty = 1;
		bool AlliesAllowed = false;
		bool HarvesterTruce = false;
		bool FogOfWar = false;
		bool MCVRedeploy = true;
		int Seed = 0;
		int TechLevel = 10;
		bool Firestorm = true;
		int CampaignDifficulty = 1;
		int CampaignCDifficulty = 1;
		std::array<bool, GLOBAL_FLAG_COUNT> GlobalFlags = {};

		// Where the machines reach one another, settled by whatever service arranged the match.
		int TunnelId = 0;
		int ListenPort = 1234;
		std::string TunnelAddress = "0.0.0.0";
		int TunnelPort = 0;

		// How long this machine waits on another, in game ticks. A value outside the bounds is
		// brought within them.
		static constexpr int TIMEOUT_MIN = 60;
		static constexpr int TIMEOUT_MAX = 36000;
		int ConnTimeout = 3600;
		int ReconnectTimeout = 2400;

		// What a player is shown.
		bool QuickMatch = false;
		bool SkipScoreScreen = false;
		bool WriteStatistics = false;
		bool AINamesByDifficulty = false;
		bool CoachMode = false;
		bool AutoSurrender = true;
		bool AttackNeutralUnits = false;
		bool ScrapMetal = false;
		bool PlayMoviesInMultiplayer = false;
		std::string CustomLoadScreen;
		int CustomLoadScreenX = 0;
		int CustomLoadScreenY = 0;
		std::string DifficultyName;

		// The match's seats, and where in them the machine reading the file sits.
		std::array<SlotType, SLOT_COUNT> Slots;
		int HumanCount = 0;
		int LocalSlot = 0;

	private:

		void Read_Slots(INIClass const & ini);
};
