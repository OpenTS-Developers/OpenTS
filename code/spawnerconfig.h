/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#pragma once

#include <array>
#include <string>

class INIClass;


/*
 * What a client asked the game to launch.
 *
 * The file this is read from belongs to the CnCNet client, not to the game: its spelling, its
 * defaults and its shape are fixed by what that client already writes. This class is the game's
 * own reading of it, so that nothing downstream has to parse anything, and so that the
 * values a match's outcome depends upon can be told apart from the ones only shown to a
 * player. Reading cannot fail; whether what was read describes a playable game is judged
 * where the launch is attempted, against the tables the game has loaded by then.
 */
class SpawnerConfigClass
{
	public:

		/*
		 * The version of this reading. It counts changes to what the game makes of a launch
		 * file, never changes to the file's own vocabulary, which belongs to the client.
		 */
		static constexpr int SCHEMA_VERSION = 1;

		/*
		 * A match holds this many seats, one per house it may hold. The scenario flags are
		 * the fifty the engine keeps; a client allowing more writes ones the game passes over.
		 */
		static constexpr int SLOT_COUNT = 8;
		static constexpr int GLOBAL_FLAG_COUNT = 50;

		/*
		 * What kind of game the file asks for. Resume overrides the rest: a saved game
		 * carries its own type, options and houses, so nothing else in the file decides them.
		 */
		enum class LaunchType {
			Skirmish,
			Campaign,
			Multiplayer,
			Resume,
		};

		/*
		 * Who occupies a seat. A launch file marks a seat human by writing a section for it,
		 * so an unwritten section is what makes a seat a computer player or nothing at all.
		 */
		enum class OccupancyType {
			Empty,
			Human,
			Computer,
		};

		/*
		 * One seat of the match. The seats are held in the order the houses are created in --
		 * humans first by ascending color, then computer players -- so that a seat's index is
		 * the index of the house it becomes, which is what alliances and start positions are
		 * named by.
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
			int Port = -1;
		};

		void Read_INI(INIClass const & ini);
		LaunchType Launch_Type(void) const;
		int Session_Identity_CRC(void) const;

		/*
		 * What kind of game to start.
		 */
		bool IsCampaign = false;
		bool IsHost = false;
		int CampaignID = -1;
		int Tournament = 0;
		int GameID = 0;

		/*
		 * The scenario and the saved game.
		 */
		std::string ScenarioName = "spawnmap.ini";
		std::string MapName;
		std::string MapHash;
		bool LoadSaveGame = false;
		std::string SaveGameName;
		int AutoSaveInterval = 10800;
		int NextCampaignAutoSave = 0;
		int NextSkirmishAutoSave = 0;

		/*
		 * The options every house plays under.
		 */
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

		/*
		 * Where the machines reach one another. A launcher settles these with the service
		 * that arranged the match, so they are the one part of the network it decides; the
		 * timing the machines keep is the game's own and is not read from a launch file.
		 */
		int ReconnectTimeout = 2400;
		int ConnTimeout = 3600;
		int TunnelId = 0;
		int ListenPort = 1234;
		std::string TunnelAddress = "0.0.0.0";
		int TunnelPort = 0;

		/*
		 * What a player is shown.
		 */
		bool QuickMatch = false;
		bool SkipScoreScreen = false;
		bool WriteStatistics = false;
		bool AINamesByDifficulty = false;
		bool CoachMode = false;
		bool AutoSurrender = true;
		bool AttackNeutralUnits = false;
		bool ScrapMetal = false;
		bool ContinueWithoutHumans = false;
		bool PlayMoviesInMultiplayer = false;
		std::string CustomLoadScreen;
		int CustomLoadScreenX = 0;
		int CustomLoadScreenY = 0;
		std::string DifficultyName;

		/*
		 * The match's seats, and where in them the machine reading the file sits.
		 */
		std::array<SlotType, SLOT_COUNT> Slots;
		int HumanCount = 0;
		int LocalSlot = 0;

	private:

		void Read_Slots(INIClass const & ini);
};
