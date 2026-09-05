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
#include "savemgr.h"
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
#include "language/language.h"
#include "loaddlg.h"
#include "mplayer.h"
#include "netshare.h"
#include "msgbox.h"
#include "saveload.h"
#include "savever.h"
#include "scenario.h"
#include "session.h"
#include "stimer.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <winsock.h>


static_assert(HOUSE_NAME_MAX == MPLAYER_NAME_MAX,
	"a seat is judged and ordered by the name the session carries");

/*
 * Only the first launch runs. A later call answers false, so the process exits rather than
 * falling into the menu.
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
/// Converts a seat's alliance list into the bit mask a node carries.
/// </summary>
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
/// The difficulty a seat is played at, logged when it differs from the one asked for.
/// </summary>
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
/// Adds one human seat to the list the houses are created from.
/// </summary>
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
	node->Player.IsObserver = seat.IsSpectator;

	// Through a tunnel a machine is addressed by its tunnel number, written where the port goes.
	if (SpawnConfig.TunnelPort != 0) {
		node->Address.Set_Address(0, htons((unsigned short)seat.Port));
	} else if (seat.Port > 0) {
		node->Address.Set_Address(inet_addr(seat.Address.c_str()), htons((unsigned short)seat.Port));
	}

	Session.Players.Add(node);
}


/// <summary>
/// Adds the human seats, this machine's first, because the game takes the first entry to be
/// the local player.
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
/// Adds the computer seats the client named to the list the houses are created from.
/// </summary>
static void Spawner_Seat_Computers(void)
{
	// Handicap 0 is the rules' easy table, which makes the hardest opponent.
	static char const * const _ai_names[DIFF_COUNT] = { "Hard AI", "Medium AI", "Easy AI" };

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

		// Session difficulty is numbered the opposite way round from a seat's handicap.
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
/// Copies the launch file's match options into the session and the game options.
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
	Session.Options.CoachMode = SpawnConfig.CoachMode;

	/*
	 * Only a match against other machines commits this to the simulation; a skirmish never does.
	 */
	Session.Options.HarvTruce = SpawnConfig.HarvesterTruce;

	// These live outside the session's option block.
	Options.GameSpeed = SpawnConfig.GameSpeed;
	BuildLevel = SpawnConfig.TechLevel;
	Session.PlayMovies = SpawnConfig.PlayMoviesInMultiplayer;

	// Init_Random uses this for a game played alone, and draws its own seed when it is zero.
	CustomSeed = SpawnConfig.Seed;

	/*
	 * Read, not honored. Every field the reader carries is bound above, consulted when a launch
	 * is refused, or listed here, so a new field forces a decision. A contract test enforces it.
	 *
	 *   MapName                       - shown while loading; bound with the scenario below.
	 *   IsCampaign, LoadSaveGame,
	 *   SaveGameName                  - read to decide the kind of launch and name the save.
	 *   Tournament, GameID,
	 *   WriteStatistics               - naming a match and reporting how it went.
	 *   BuildOffAlly, AttackNeutralUnits,
	 *   ScrapMetal, AutoSurrender,
	 *   ContinueWithoutHumans         - options the game has no setting of its own for yet.
	 *   QuickMatch, SkipScoreScreen,
	 *   CustomLoadScreen,
	 *   CustomLoadScreenX,
	 *   CustomLoadScreenY,
	 *   DifficultyName                - what a player is shown around the match.
	 */
}


/// <summary>
/// Hands the game the automatic-save schedule and ring positions the launch file names. A
/// resumed save overrides the positions when it loads.
/// </summary>
static void Spawner_Bind_Autosave(void)
{
	SaveManager.Autosave.Set_Interval(SpawnConfig.AutoSaveInterval);
	SaveManager.Autosave.Seed_Slots(SpawnConfig.NextCampaignAutoSave, SpawnConfig.NextSkirmishAutoSave);
}


/// <summary>
/// Names this machine the host when its launch file says so. The other seats learn it from
/// the announcement once the connections exist, and the lowest seat stands in until then.
/// </summary>
static void Spawner_Bind_Master(void)
{
	if (SpawnConfig.IsHost && Session.Players.Count() > 0) {
		Session.Adopt_Master(Session.Players[0]->Player.ID, Session.Players[0]->Name);
	}
}


/// <summary>
/// Sends the host announcement when this machine's launch file made it the host: once the
/// connections exist, and again after an in-place load.
/// </summary>
void Spawner_Announce_Master(void)
{
	if (Spawner_Is_Active() && SpawnConfig.IsHost && Session.Type == GAME_INTERNET) {
		Session.Announce_Master();
	}
}


/// <summary>
/// Names the scenario the session plays, in place of the menu's map list.
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
/// Opens the network a match against other machines is played over, through the tunnel the
/// file names or straight to the address each seat carries.
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

	// The local seat is first, so every seat after it is another machine.
	for (int index = 1; index < Session.Players.Count(); index++) {
		Ipx.Add_Peer(Session.Players[index]->Address);
	}

	if (!Ipx.Init()) {
		return(Spawner_Refuse("The network could not be opened."));
	}

	// Loading-screen progress goes out before the in-game timing is set, and the manager's
	// default 33 ms retry floods a slow link.
	Ipx.Set_Timing(TIMER_SECOND, (unsigned int)-1, 10 * TIMER_SECOND);

	return(true);
}


/// <summary>
/// Resumes the saved game a launch file names. The save carries the game and its houses,
/// while a match against other machines takes its seats and their addresses from the file.
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

	// A client never arranges a local network game, so no launch file describes one.
	GameType type = (GameType)info.Get_Game_Type();
	if (type == GAME_IPX) {
		return(Spawner_Refuse("Resuming a game arranged over the local network is not supported."));
	}

	/*
	 * The file seats the same people again, so the network opens before the load and the queue
	 * synchronizes at the resumed frame.
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
		Spawner_Bind_Master();

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

	/*
	 * A save carries the options it was played under, but game speed and whether movies play
	 * are the player's own.
	 */
	Options.GameSpeed = SpawnConfig.GameSpeed;
	Session.PlayMovies = SpawnConfig.PlayMoviesInMultiplayer;

	gameloaded = true;

	return(true);
}


/// <summary>
/// Assembles the campaign mission a launch asks for: the mission, its difficulty pair, and
/// the scenario flags carried over from an earlier mission.
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
	Options.GameSpeed = SpawnConfig.GameSpeed;
	Session.CampaignDifficulty = (DiffType)SpawnConfig.CampaignDifficulty;
	Session.CampaignCDifficulty = (DiffType)SpawnConfig.CampaignCDifficulty;
	Scen->Campaign = (CampaignType)SpawnConfig.CampaignID;

	// A fresh launch carries nothing over, so the file's flags replace an earlier mission's.
	new (&Environment) EnvironmentClass;
	for (int index = 0; index < SpawnerConfigClass::GLOBAL_FLAG_COUNT; index++) {
		Environment.Globals[index] = SpawnConfig.GlobalFlags[index];
	}

	std::snprintf(Scen->ScenarioName, sizeof(Scen->ScenarioName), "%s", SpawnConfig.ScenarioName.c_str());

	return(true);
}


/// <summary>
/// Assembles the session a launch asks for, in place of what a setup dialog commits.
/// </summary>
static void Spawner_Setup_Session(void)
{
	Session.Type = SpawnConfig.Launch_Type() == SpawnerConfigClass::LaunchType::Multiplayer
		? GAME_INTERNET : GAME_SKIRMISH;

	// Every machine must draw alike, and no lobby is there to share a seed.
	if (Session.Type == GAME_INTERNET) {
		Seed = SpawnConfig.Seed;
	}

	Clear_Vector(&Session.Players);
	Clear_Vector(&Session.Computers);

	Spawner_Bind_Options();

	if (Session.Type == GAME_INTERNET) {
		Commit_Session_Specials();
	}

	Spawner_Seat_Local();
	Spawner_Seat_Humans();
	Spawner_Seat_Computers();
	Spawner_Bind_Master();
	Spawner_Bind_Scenario();
}


/// <summary>
/// Records that a client asked the game to launch what SPAWN.INI describes.
/// </summary>
void Spawner_Request(void)
{
	SpawnRequested = true;
}


/// <summary>
/// Did a client ask the game to launch what its file describes?
/// </summary>
bool Spawner_Is_Requested(void)
{
	return(SpawnRequested);
}


/// <summary>
/// Is the running game the one a launch file described? Asked by code that must leave a
/// client's choices alone.
/// </summary>
bool Spawner_Is_Active(void)
{
	return(SpawnConsumed);
}


/// <summary>
/// Returns the launch file's session identity digest, which two peers handed the same match
/// share, or 0 when the game was not launched from a spawn file.
/// </summary>
int Spawner_Session_Identity(void)
{
	return(Spawner_Is_Active() ? SpawnConfig.Session_Identity_CRC() : 0);
}


/// <summary>
/// Reads the launch file and assembles the game it describes, in place of the menu. Answers
/// false once that game has ended, so the process exits rather than showing one.
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

	SpawnConsumed = true;

	Spawner_Bind_Autosave();

	/*
	 * Every kind of launch is played at this speed, so it is checked before the kinds part.
	 */
	if (SpawnConfig.GameSpeed < 0 || SpawnConfig.GameSpeed >= OptionsClass::MAX_SPEED_SETTING) {
		return(Spawner_Refuse("The file asks for game speed %d, and the game has 0 through %d.",
			SpawnConfig.GameSpeed, OptionsClass::MAX_SPEED_SETTING - 1));
	}

	// A seat names its country by the rules' own numbering, so the roster is read first.
	Prepare_Side_Roster();

	if (SpawnConfig.Launch_Type() == SpawnerConfigClass::LaunchType::Resume) {
		return(Spawner_Resume(gameloaded));
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

	// The network comes last, once the session it carries is fully assembled.
	if (Session.Type == GAME_INTERNET && !Spawner_Wire_Network()) {
		return(false);
	}

	return(true);
}
