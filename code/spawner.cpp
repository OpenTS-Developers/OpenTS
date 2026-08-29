/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "always.h"

#include "spawner.h"

#include "spawnerconfig.h"

#include "addon.h"
#include "campaign.h"
#include "ccfile.h"
#include "ccini.h"
#include "dbgprint.h"
#include "enviro.h"
#include "globals.h"
#include "goptions.h"
#include "houstype.h"
#include "init.h"
#include "language\language.h"
#include "mplayer.h"
#include "msgbox.h"
#include "scenario.h"
#include "session.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <string>


/*
 * A spawned launch happens at most once for the life of the process: the client that asked
 * for it watches for the process to exit, so a finished or refused spawn ends the program
 * rather than falling into the menu.
 */
static bool SpawnRequested = false;
static bool SpawnConsumed = false;
static SpawnerConfigClass SpawnConfig;


/// <summary>
/// Refuses the launch, telling the player why and leaving the reason in the log.
/// </summary>
/// <param name="fault">A printf style description of the fault.</param>
/// <returns>false, so a caller can refuse and return in one statement.</returns>
static bool Spawner_Refuse(char const * fault, ...)
{
	char buffer[256];

	va_list args;
	va_start(args, fault);
	std::vsnprintf(buffer, sizeof(buffer), fault, args);
	va_end(args);

	DebugString("[Spawner] Refusing to launch: %s\n", buffer);
	WWMessageBox().Process(buffer, TXT_OK);

	return(false);
}


/// <summary>
/// Folds a seat's alliance list into the bitfield the houses are allied by.
/// </summary>
/// <param name="seat">The seat whose alliances are wanted.</param>
/// <returns>One bit set per seat this one is allied with.</returns>
static unsigned Spawner_Allies_Mask(SpawnerConfigClass::SlotType const & seat)
{
	unsigned mask = 0;

	for (int ally : seat.Alliances) {
		if (ally >= 0 && ally < SpawnerConfigClass::SLOT_COUNT) {
			mask |= 1u << ally;
		}
	}

	return(mask);
}


/// <summary>
/// The difficulty one seat is played at, saying so when it is not the one asked for.
/// </summary>
/// <param name="index">Which seat, counted from zero.</param>
/// <param name="asked">The difficulty the launch file asked for.</param>
/// <returns>The difficulty to play the seat at, or -1 for the session default.</returns>
static int Spawner_Seat_Handicap(int index, int asked)
{
	int played = SpawnerConfigClass::Playable_Handicap(asked);

	if (played != asked) {
		DebugString("[Spawner] Seat %d asked for difficulty %d and is played at %d.\n",
			index + 1, asked, played);
	}

	return(played);
}


/// <summary>
/// Tells the session who is playing at this machine.
/// </summary>
static void Spawner_Seat_Local(void)
{
	SpawnerConfigClass::SlotType const & local = SpawnConfig.Slots[SpawnConfig.LocalSlot];

	std::snprintf(Session.Handle, sizeof(Session.Handle), "%s",
		local.Name.empty() ? "Player" : local.Name.c_str());
	Session.House = local.Country;
	Session.ColorIdx = local.Color;
	Session.PrefColor = Session.ColorIdx;
}


/// <summary>
/// Puts the people playing into the list the houses are created from.
/// </summary>
static void Spawner_Seat_Humans(void)
{
	for (int index = 0; index < SpawnConfig.HumanCount; index++) {
		SpawnerConfigClass::SlotType const & seat = SpawnConfig.Slots[index];

		NodeNameType * node = new NodeNameType;
		std::snprintf(node->Name, sizeof(node->Name), "%s",
			seat.Name.empty() ? "Player" : seat.Name.c_str());
		node->Player.House = seat.Country;
		node->Player.Color = seat.Color;
		node->Player.ProcessTime = -1;
		node->Player.SpawnChoice = seat.StartingPosition;
		node->Player.AlliesMask = Spawner_Allies_Mask(seat);
		Session.Players.Add(node);
	}

	Session.NumPlayers = SpawnConfig.HumanCount;
}


/// <summary>
/// Puts the computer players the client seated into the list the houses are created from.
/// </summary>
static void Spawner_Seat_Computers(void)
{
	static char const * const _ai_names[DIFF_COUNT] = { "Easy AI", "Medium AI", "Hard AI" };

	for (int index = SpawnConfig.HumanCount; index < SpawnerConfigClass::SLOT_COUNT; index++) {
		SpawnerConfigClass::SlotType const & seat = SpawnConfig.Slots[index];
		if (seat.Occupancy != SpawnerConfigClass::OccupancyType::Computer) {
			continue;
		}

		NodeNameType * node = new NodeNameType;
		node->Player.House = seat.Country;
		node->Player.Color = seat.Color;
		node->Player.Handicap = Spawner_Seat_Handicap(index, seat.Handicap);
		node->Player.SpawnChoice = seat.StartingPosition;
		node->Player.AlliesMask = Spawner_Allies_Mask(seat);

		/*
		 * A computer player is named for the difficulty it is actually played at, which is
		 * the one its seat asked for, or else the one the session gives every computer.
		 */
		if (SpawnConfig.AINamesByDifficulty) {
			int played = node->Player.Handicap >= 0
				? node->Player.Handicap
				: (DIFF_COUNT - 1 - SpawnConfig.AIDifficulty);
			std::snprintf(node->Name, sizeof(node->Name), "%s",
				_ai_names[std::clamp(played, 0, DIFF_COUNT - 1)]);
		}

		Session.Computers.Add(node);
	}
}


/// <summary>
/// Tells the session what every house plays under.
/// The order below follows the launch file's own, so that what the game takes from a launch
/// can be read against what the file carries.
/// </summary>
static void Spawner_Bind_Options(void)
{
	Session.Options.Bases = SpawnConfig.Bases;
	Session.Options.Credits = SpawnConfig.Credits;
	Session.Options.BridgeDestruction = SpawnConfig.BridgeDestroy;
	Session.Options.Goodies = SpawnConfig.Crates;
	Session.Options.ShortGame = SpawnConfig.ShortGame;
	Session.Options.GameSpeed = SpawnConfig.GameSpeed;
	Session.Options.CrapEngineers = SpawnConfig.MultiEngineer;
	Session.Options.UnitCount = SpawnConfig.UnitCount;
	Session.Options.AIPlayers = SpawnConfig.AIPlayers;
	Session.Options.AIDifficulty = (DiffType)SpawnConfig.AIDifficulty;
	Session.Options.AlliesAllowed = SpawnConfig.AlliesAllowed;
	Session.Options.FogOfWar = SpawnConfig.FogOfWar;
	Session.Options.MCVRedeploy = SpawnConfig.MCVRedeploy;

	/*
	 * A skirmish never reaches the pregame setup that hands this to the simulation, so the
	 * session records what was asked for while the map's own setting still decides it.
	 */
	Session.Options.HarvTruce = SpawnConfig.HarvesterTruce;

	/*
	 * These two are options as much as anything above, but they live outside the block the
	 * session keeps them in; the menu's own commit sets both the same way.
	 */
	Options.GameSpeed = SpawnConfig.GameSpeed;
	BuildLevel = SpawnConfig.TechLevel;

	/*
	 * The seed is left where a launch option leaves it, since the random numbers are only
	 * settled once the session type is known. A file naming no seed leaves it to the clock.
	 */
	CustomSeed = SpawnConfig.Seed;

	/*
	 * Read, not honored. Every field the reader carries is either bound above, consumed to
	 * refuse a launch, or named here with the reason, so that adding a field to the reader
	 * forces a decision rather than a silent omission. A field named here is not a defect:
	 * the launch file is the client's vocabulary, and much of it describes machinery this
	 * game does not have yet.
	 *
	 *   IsHost, Tournament, GameID    - the client's own bookkeeping of the match.
	 *   MapName                       - shown while loading; bound with the scenario below.
	 *   MapHash                       - the client checks that the machines hold one map.
	 *   AutoSaveInterval,
	 *   NextCampaignAutoSave,
	 *   NextSkirmishAutoSave          - saving by itself is not wired up.
	 *   BuildOffAlly                  - the game has no such option to give it to.
	 *   ReconnectTimeout, ConnTimeout,
	 *   TunnelId, ListenPort,
	 *   TunnelAddress, TunnelPort,
	 *   Slots[].Address, Slots[].Port - where machines reach one another; a skirmish reaches
	 *                                   none of them.
	 *   QuickMatch, SkipScoreScreen,
	 *   WriteStatistics, CoachMode,
	 *   AutoSurrender, AttackNeutralUnits,
	 *   ScrapMetal, ContinueWithoutHumans,
	 *   PlayMoviesInMultiplayer       - behaviors no part of this game offers yet.
	 *   CustomLoadScreen,
	 *   CustomLoadScreenX,
	 *   CustomLoadScreenY             - the loading backdrop belongs to the campaign path.
	 *   DifficultyName                - shown, never played by.
	 *   IsCampaign, LoadSaveGame,
	 *   SaveGameName                  - read to decide what kind of launch this is, which is
	 *                                   how the two this game cannot start are refused.
	 *   Slots[].IsSpectator           - read to refuse a launch.
	 *
	 * The timing keys a client writes are not read at all: the game keeps its own.
	 */
}


/// <summary>
/// Tells the session which scenario is being played, in place of the map list the menu picks
/// from, which a client-launched game never shows.
/// </summary>
static void Spawner_Bind_Scenario(void)
{
	std::snprintf(Scen->ScenarioName, sizeof(Scen->ScenarioName), "%s", SpawnConfig.ScenarioName.c_str());
	std::snprintf(Session.ScenarioFileName, sizeof(Session.ScenarioFileName), "%s", SpawnConfig.ScenarioName.c_str());
	std::snprintf(Session.Options.ScenarioDescription, sizeof(Session.Options.ScenarioDescription),
		"%s", SpawnConfig.MapName.c_str());

	Session.Options.ScenarioIndex = -1;
	Session.ScenarioFileLength = CCFileClass(Scen->ScenarioName).Size();
	Session.ScenarioIsOfficial = false;
	Session.ScenarioDigest[0] = '\0';
}


/// <summary>
/// Assembles the campaign mission a launch asks for: the mission, the handicap pair, and the
/// scenario flags a client carries over from an earlier mission.
/// </summary>
/// <returns>bool; Can the campaign the file describes be played?</returns>
static bool Spawner_Setup_Campaign(void)
{
	if (SpawnConfig.CampaignDifficulty < 0 || SpawnConfig.CampaignDifficulty >= DIFF_COUNT ||
		SpawnConfig.CampaignCDifficulty < 0 || SpawnConfig.CampaignCDifficulty >= DIFF_COUNT) {
		return(Spawner_Refuse("A campaign is played at difficulty 0, 1 or 2, and the file says %d and %d.",
			SpawnConfig.CampaignDifficulty, SpawnConfig.CampaignCDifficulty));
	}

	if (SpawnConfig.CampaignID < -1 || SpawnConfig.CampaignID >= Campaigns.Count()) {
		return(Spawner_Refuse("The file names campaign %d, and there are %d.",
			SpawnConfig.CampaignID, Campaigns.Count()));
	}

	Session.Type = GAME_NORMAL;
	Session.CampaignDifficulty = (DiffType)SpawnConfig.CampaignDifficulty;
	Session.CampaignCDifficulty = (DiffType)SpawnConfig.CampaignCDifficulty;
	Scen->Campaign = (CampaignType)SpawnConfig.CampaignID;

	/*
	 * The flags are left where a mission carries them over from the one before it, since a
	 * fresh launch has nothing else of its own to carry.
	 */
	new (&Environment) EnvironmentClass;
	for (int index = 0; index < SpawnerConfigClass::GLOBAL_FLAG_COUNT; index++) {
		Environment.Globals[index] = SpawnConfig.GlobalFlags[index];
	}

	std::snprintf(Scen->ScenarioName, sizeof(Scen->ScenarioName), "%s", SpawnConfig.ScenarioName.c_str());

	return(true);
}


/// <summary>
/// Assembles the session a skirmish launch asks for, in place of what the skirmish dialog
/// commits when a player presses OK.
/// </summary>
static void Spawner_Setup_Skirmish(void)
{
	Session.Type = GAME_SKIRMISH;

	Clear_Vector(&Session.Players);
	Clear_Vector(&Session.Computers);

	Spawner_Bind_Options();
	Spawner_Seat_Local();
	Spawner_Seat_Humans();
	Spawner_Seat_Computers();
	Spawner_Bind_Scenario();
}


/// <summary>
/// Notes that a client asked the game to launch what its file describes.
/// </summary>
void Spawner_Request(void)
{
	SpawnRequested = true;
}


/// <summary>
/// Did a client ask the game to launch what its file describes?
/// </summary>
/// <returns>bool; Was a launch requested on the command line?</returns>
bool Spawner_Is_Requested(void)
{
	return(SpawnRequested);
}


/// <summary>
/// Is the game being played the one a launch file described?
/// This answers for the game itself rather than for the command line, so a path that must
/// leave a client's choices alone can tell that a launch file made them.
/// </summary>
/// <returns>bool; Was the game assembled from a launch file?</returns>
bool Spawner_Is_Active(void)
{
	return(SpawnConsumed);
}


/// <summary>
/// Reads the launch file and assembles the game it describes.
/// This stands in place of the menu, and answers false once the game it launched has ended,
/// so that the process leaves rather than showing a menu the client never meant to show.
/// </summary>
/// <param name="gameloaded">Set when the launch resumed a saved game.</param>
/// <returns>bool; Is a game ready to start?</returns>
bool Spawner_Prepare(bool & gameloaded)
{
	(void)gameloaded;

	if (SpawnConsumed) {
		return(false);
	}

	CCFileClass file("SPAWN.INI");
	if (!file.Is_Available()) {
		return(Spawner_Refuse("SPAWN.INI is missing, and it says what to launch."));
	}

	CCINIClass ini;
	ini.Load(file, false);
	SpawnConfig.Read_INI(ini);

	/*
	 * A launch is spent as soon as it is read, so that a refusal ends the process the same
	 * way a finished game does.
	 */
	SpawnConsumed = true;

	/*
	 * The countries a seat may name are the rules', so the roster is read before the seats
	 * are judged, as the menu paths setting up a game do.
	 */
	Prepare_Side_Roster();

	switch (SpawnConfig.Launch_Type()) {
		case SpawnerConfigClass::LaunchType::Resume:
			return(Spawner_Refuse("Resuming a saved game from a launch file is not supported yet."));

		case SpawnerConfigClass::LaunchType::Multiplayer:
			return(Spawner_Refuse("Launching a game against other machines is not supported yet."));

		case SpawnerConfigClass::LaunchType::Campaign:
		case SpawnerConfigClass::LaunchType::Skirmish:
			break;
	}

	Disable_Addon(ADDON_ANY);
	if (SpawnConfig.Firestorm) {
		Enable_Addon(ADDON_FIRESTORM);
		Set_Required_Addon(ADDON_FIRESTORM);
	}

	if (SpawnConfig.Launch_Type() == SpawnerConfigClass::LaunchType::Campaign) {
		if (!Spawner_Setup_Campaign()) {
			return(false);
		}
	} else {
		std::string fault;
		if (!SpawnConfig.Is_Playable(HouseTypes.Count(), MAX_MPLAYER_COLORS, fault)) {
			return(Spawner_Refuse("%s", fault.c_str()));
		}

		Spawner_Setup_Skirmish();
	}

	DebugString("[Spawner] Launching %s with session identity %08x.\n",
		Scen->ScenarioName, SpawnConfig.Session_Identity_CRC());

	return(true);
}
