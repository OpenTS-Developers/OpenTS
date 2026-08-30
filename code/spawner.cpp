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
#include "ipxmgr.h"
#include "language\language.h"
#include "loaddlg.h"
#include "mplayer.h"
#include "msgbox.h"
#include "saveload.h"
#include "savever.h"
#include "scenario.h"
#include "session.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <winsock.h>


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
/// Puts one person's seat into the list the houses are created from, with the address the
/// machine playing it is reached on when the match is against other machines.
/// </summary>
/// <param name="index">Which seat, counted from zero.</param>
static void Spawner_Seat_Human(int index)
{
	SpawnerConfigClass::SlotType const & seat = SpawnConfig.Slots[index];

	NodeNameType * node = new NodeNameType;
	std::snprintf(node->Name, sizeof(node->Name), "%s",
		seat.Name.empty() ? "Player" : seat.Name.c_str());
	node->Player.House = seat.Country;
	node->Player.Color = seat.Color;
	node->Player.ProcessTime = -1;
	node->Player.SpawnChoice = seat.StartingPosition;
	node->Player.AlliesMask = Spawner_Allies_Mask(seat);

	/*
	 * Through a tunnel a machine is named by its tunnel number alone, carried where a port
	 * would go; reached directly, it is named by the address it answers on.
	 */
	if (SpawnConfig.TunnelPort != 0) {
		node->Address.Set_Address(0, htons((unsigned short)seat.Port));
	} else if (seat.Port > 0) {
		node->Address.Set_Address(inet_addr(seat.Address.c_str()), htons((unsigned short)seat.Port));
	}

	Session.Players.Add(node);
}


/// <summary>
/// Puts the people playing into the list the houses are created from. The game takes the
/// first entry of this list to be the player at this machine, so the local seat leads and
/// the rest follow in seat order; the houses take their own order from what the seats say
/// rather than from this list.
/// </summary>
static void Spawner_Seat_Humans(void)
{
	Spawner_Seat_Human(SpawnConfig.LocalSlot);

	for (int index = 0; index < SpawnConfig.HumanCount; index++) {
		if (index != SpawnConfig.LocalSlot) {
			Spawner_Seat_Human(index);
		}
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
	 *   ReconnectTimeout, ConnTimeout - how patiently to wait for a machine that has gone
	 *                                   quiet is part of the timing the game keeps for itself.
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
	 *   SaveGameName                  - read to decide what kind of launch this is, and to
	 *                                   name the saved game a resume restores.
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
/// Opens the network a game against other machines is played over. Through a tunnel every
/// machine is named by its tunnel number; otherwise each is reached at its own address,
/// and this machine listens where the file told the others to find it. The other players'
/// seats become the addresses a broadcast fans out to.
/// </summary>
/// <returns>bool; Is the network ready to carry the match?</returns>
static bool Spawner_Wire_Network(void)
{
	if (SpawnConfig.TunnelPort != 0) {
		Ipx.Configure_Tunnel(htons((unsigned short)SpawnConfig.TunnelId),
			inet_addr(SpawnConfig.TunnelAddress.c_str()), htons((unsigned short)SpawnConfig.TunnelPort));
	} else {
		Ipx.Configure_Direct_Peers((unsigned short)SpawnConfig.ListenPort);
	}

	// The local seat leads the player list, so everybody after it is another machine.
	for (int index = 1; index < Session.Players.Count(); index++) {
		Ipx.Add_Peer(Session.Players[index]->Address);
	}

	if (!Ipx.Init()) {
		return(Spawner_Refuse("The network could not be opened."));
	}

	return(true);
}


/// <summary>
/// Resumes the saved game a launch file names. The save carries the kind of game, the options
/// and the houses, and the expansion comes back with it rather than from the file. A game
/// played alone takes nothing else from the file; one against other machines takes its seats,
/// which name the same people at the addresses their machines answer on now.
/// </summary>
/// <param name="gameloaded">Set when the save loads, so the caller starts no scenario.</param>
/// <returns>bool; Is the saved game running?</returns>
static bool Spawner_Resume(bool & gameloaded)
{
	if (SpawnConfig.SaveGameName.empty()) {
		return(Spawner_Refuse("The file asks to resume a saved game without naming one."));
	}

	SaveVersionInfo info;
	if (!Get_Savefile_Info(SpawnConfig.SaveGameName.c_str(), &info)) {
		return(Spawner_Refuse("The saved game %s is missing or unreadable.", SpawnConfig.SaveGameName.c_str()));
	}

	if (info.Get_Internal_Version() != ExpectedGameVersion) {
		return(Spawner_Refuse("The saved game was made by another version of the game."));
	}

	/*
	 * A game the menu arranged over the local network is nothing a client launched, so no
	 * launch file describes the match such a save would resume.
	 */
	GameType type = (GameType)info.Get_Game_Type();
	if (type == GAME_IPX) {
		return(Spawner_Refuse("Resuming a game arranged over the local network is not supported."));
	}

	/*
	 * Against other machines the save restores the houses while the file seats the same
	 * people afresh, so the seats are judged and the network opened before the save is
	 * read, and the queue is told to shake hands again at the resumed frame.
	 */
	if (type == GAME_INTERNET) {
		std::string fault;
		if (!SpawnConfig.Is_Playable(HouseTypes.Count(), MAX_MPLAYER_COLORS, fault)) {
			return(Spawner_Refuse("%s", fault.c_str()));
		}

		Clear_Vector(&Session.Players);
		Clear_Vector(&Session.Computers);

		Spawner_Seat_Local();
		Spawner_Seat_Humans();

		if (!Spawner_Wire_Network()) {
			return(false);
		}

		Session.LoadGame = true;
	}

	if (!LoadOptionsClass().Load_File(SpawnConfig.SaveGameName.c_str())) {
		return(Spawner_Refuse("The saved game %s could not be loaded.", SpawnConfig.SaveGameName.c_str()));
	}

	if (type == GAME_INTERNET && !Reconcile_Players()) {
		return(Spawner_Refuse("The saved game and the file do not agree on who is playing."));
	}

	gameloaded = true;

	return(true);
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
/// Assembles the session a launch asks for, in place of what the skirmish or the lobby dialog
/// commits when a player presses OK.
/// </summary>
static void Spawner_Setup_Session(void)
{
	Session.Type = SpawnConfig.Launch_Type() == SpawnerConfigClass::LaunchType::Multiplayer
		? GAME_INTERNET : GAME_SKIRMISH;

	/*
	 * Against other machines the random numbers must fall the same way everywhere, and no
	 * lobby is there to hand a seed around, so the file's own is taken exactly as written.
	 */
	if (Session.Type == GAME_INTERNET) {
		Seed = SpawnConfig.Seed;
	}

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
			return(Spawner_Resume(gameloaded));

		case SpawnerConfigClass::LaunchType::Multiplayer:
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

		Spawner_Setup_Session();
	}

	DebugString("[Spawner] Launching %s with session identity %08x.\n",
		Scen->ScenarioName, SpawnConfig.Session_Identity_CRC());

	/*
	 * The network comes last, once the session it will carry is assembled whole.
	 */
	if (Session.Type == GAME_INTERNET && !Spawner_Wire_Network()) {
		return(false);
	}

	return(true);
}
