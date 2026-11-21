/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "always.h"

#include "launcherapply.h"

#include "addon.h"
#include "campaign.h"
#include "ccfile.h"
#include "enviro.h"
#include "event.h"
#include "globals.h"
#include "goptions.h"
#include "house.h"
#include "houstype.h"
#include "init.h"
#include "ipxmgr.h"
#include "loaddlg.h"
#include "mplayer.h"
#include "netshare.h"
#include "saveload.h"
#include "savemgr.h"
#include "savever.h"
#include "scenario.h"
#include "session.h"
#include "stats.h"
#include "stimer.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <winsock.h>


static_assert(HOUSE_NAME_MAX == MPLAYER_NAME_MAX,
	"a player is judged and ordered by the name the session carries");

static_assert(Launch::Game::GAME_SPEED_COUNT == OptionsClass::MAX_SPEED_SETTING,
	"a launch accepts the game speeds the options offer");

static_assert(Launch::Game::DIFFICULTY_COUNT == DIFF_COUNT,
	"a launch accepts the difficulties the game has");

static_assert(Launch::Game::NAME_LENGTH_MAX == HOUSE_NAME_MAX - 1,
	"a launch accepts a name a house can store");


std::optional<Launch::SetupType> Launch::Setup;


namespace {

using GameKind = Launch::GameKind;
using Launch::Fault;


unsigned Allies_Mask(Launch::Game::PlayerType const & player)
{
	unsigned mask = 0;
	for (int ally : player.Allies) {
		mask |= 1u << ally;
	}
	return(mask);
}


void Apply_Presentation(Launch::Machine const & machine)
{
	SaveManager.Autosave.Set_Interval(machine.AutosaveInterval);
	SaveManager.Autosave.Seed_Slots(machine.NextCampaignAutosave, machine.NextSkirmishAutosave);

	Session.SkipScoreScreen = machine.SkipScoreScreen;
	std::snprintf(Session.LoadScreen, sizeof(Session.LoadScreen), "%s", machine.LoadScreen.c_str());
	Session.LoadScreenX = machine.LoadScreenX.value_or(0);
	Session.LoadScreenY = machine.LoadScreenY.value_or(0);
	std::snprintf(Session.DifficultyName, sizeof(Session.DifficultyName), "%s", machine.DifficultyName.c_str());

	StatisticsSettings.IsWritten = machine.WriteStatistics;
	StatisticsSettings.MatchID = int(machine.GameId);
	StatisticsSettings.MapHash = machine.MapHash;
	StatisticsSettings.MapName = machine.MapName;
}


void Apply_Campaign(Launch::Game const & game, Launch::Machine const & machine)
{
	Launch::Game::OptionsType const & options = game.Options;

	Session.Type = GAME_NORMAL;
	Session.Options.GameSpeed = options.GameSpeed;
	Session.CampaignDifficulty = (DiffType)options.CampaignPlayerDifficulty;
	Session.CampaignCDifficulty = (DiffType)options.CampaignComputerDifficulty;
	CustomSeed = game.Seed;
	Scen->Campaign = (CampaignType)options.Campaign;

	// A launched campaign starts with only the global flags the launch lists.
	new (&Environment) EnvironmentClass;
	for (int flag : game.GlobalFlags) {
		Environment.Globals[flag] = true;
	}

	std::snprintf(Scen->ScenarioName, sizeof(Scen->ScenarioName), "%s", machine.MapPath.c_str());
}


void Apply_Match_Options(Launch::Game const & game, Launch::Machine const & machine)
{
	Launch::Game::OptionsType const & options = game.Options;

	Session.Options.Bases = options.Bases;
	Session.Options.Credits = options.Credits;
	Session.Options.BridgeDestruction = options.BridgeDestruction;
	Session.Options.Goodies = options.Crates;
	Session.Options.ShortGame = options.ShortGame;
	Session.Options.GameSpeed = options.GameSpeed;
	Session.Options.CrapEngineers = options.MultiEngineer;
	Session.Options.UnitCount = options.UnitCount;
	Session.Options.AIPlayers = int(game.Players.size()) - game.Human_Count();
	Session.Options.AIDifficulty = (DiffType)options.AIDifficulty;
	Session.Options.AlliesAllowed = options.AlliesAllowed;
	Session.Options.FogOfWar = options.FogOfWar;
	Session.Options.MCVRedeploy = options.MCVRedeploy;
	Session.Options.CoachMode = options.CoachMode;
	Session.Options.BuildOffAlly = options.BuildOffAlly;
	Session.Options.AutoDeployMCV = options.AutoDeployMCV;
	Session.Options.AttackNeutralUnits = options.AttackNeutralUnits;
	Session.Options.ScrapMetal = options.ScrapMetal;
	Session.Options.SharedControl = options.SharedControl;
	Session.Options.PlayMovies = options.PlayMovies;

	// Without SurrenderOnDisconnect, a departing player's base goes to the computer.
	Session.Options.AITakeover = !options.SurrenderOnDisconnect;

	// Only a network match applies the truce; a skirmish records it and ignores it.
	Session.Options.HarvTruce = options.HarvesterTruce;

	BuildLevel = options.TechLevel;

	if (machine.Network) {
		Session.ConnTimeout = machine.Network->ConnectTimeout;
		Session.ReconnectTimeout = machine.Network->ReconnectTimeout;
	}
}


NodeNameType * Player_Node(Launch::Game::PlayerType const & player)
{
	NodeNameType * node = new NodeNameType;
	std::snprintf(node->Name, sizeof(node->Name), "%s", player.Name.c_str());
	node->Player.House = player.Country;
	node->Player.Color = player.Color;
	node->Player.SpawnChoice = player.Start;
	node->Player.AlliesMask = Allies_Mask(player);
	return(node);
}


/// <summary>
/// Adds the human players, this machine's first, because the game takes the first entry to be
/// the local player.
/// </summary>
void Add_Human_Players(Launch::Game const & game, Launch::Machine const & machine)
{
	Launch::Game::PlayerType const & local = game.Players[machine.LocalPlayer];
	std::snprintf(Session.Handle, sizeof(Session.Handle), "%s", local.Name.c_str());
	Session.House = local.Country;
	Session.ColorIdx = local.Color;
	Session.PrefColor = Session.ColorIdx;

	auto add_human = [&](int index) {
		Launch::Game::PlayerType const & player = game.Players[index];
		NodeNameType * node = Player_Node(player);
		node->Player.ProcessTime = -1;
		node->Player.IsObserver = player.Observer;

		// Through a tunnel a machine is addressed by its tunnel number, written where the port goes.
		if (machine.Network) {
			Launch::Machine::NetworkType const & network = *machine.Network;
			if (index == machine.LocalPlayer) {
				node->Address.Set_Address(0, htons((unsigned short)network.Port));
			}
			for (Launch::Machine::PeerType const & peer : network.Peers) {
				if (peer.Player != index) {
					continue;
				}
				unsigned long ip = network.Tunnel ? 0 : inet_addr(peer.Address.c_str());
				node->Address.Set_Address(ip, htons((unsigned short)peer.Port));
			}
		}

		Session.Players.Add(node);
	};

	add_human(machine.LocalPlayer);
	for (int index = 0; index < int(game.Players.size()); index++) {
		if (game.Players[index].Human && index != machine.LocalPlayer) {
			add_human(index);
		}
	}

	// Observers are counted. The starting crate count and the network queue read this.
	Session.NumPlayers = game.Human_Count();
}


void Add_Computer_Players(Launch::Game const & game)
{
	for (Launch::Game::PlayerType const & player : game.Players) {
		if (player.Human) {
			continue;
		}

		NodeNameType * node = Player_Node(player);
		node->Player.Handicap = player.AILevel.value_or(-1);
		Session.Computers.Add(node);
	}
}


void Apply_Scenario(Launch::Machine const & machine)
{
	std::snprintf(Scen->ScenarioName, sizeof(Scen->ScenarioName), "%s", machine.MapPath.c_str());
	std::snprintf(Session.ScenarioFileName, sizeof(Session.ScenarioFileName), "%s", machine.MapPath.c_str());
	std::snprintf(Session.Options.ScenarioDescription, sizeof(Session.Options.ScenarioDescription),
		"%s", machine.MapName.c_str());

	Session.Options.ScenarioIndex = -1;
	Session.ScenarioFileLength = CCFileClass(Scen->ScenarioName).Size();
	Session.ScenarioIsOfficial = false;
	Session.ScenarioDigest[0] = '\0';
}


void Apply_Match(Launch::Game const & game, Launch::Machine const & machine)
{
	Session.Type = game.Kind == GameKind::GAME_KIND_MULTIPLAYER ? GAME_INTERNET : GAME_SKIRMISH;

	Clear_Vector(&Session.Players);
	Clear_Vector(&Session.Computers);

	Apply_Match_Options(game, machine);

	// A network game starts from the shared seed, which Init_Random does not replace.
	if (game.Kind == GameKind::GAME_KIND_MULTIPLAYER) {
		Seed = int(*game.Seed);
	} else {
		CustomSeed = game.Seed;
	}

	if (Session.Type == GAME_INTERNET) {
		Commit_Session_Specials();
	}

	Add_Human_Players(game, machine);
	Add_Computer_Players(game);
	Apply_Scenario(machine);
}


/// <summary>
/// Converts a load request and reads the game type of its save, or returns false with fault set
/// when the save cannot be resumed. Changes no game state.
/// </summary>
bool Take_Load(Launch::Message::Load const & request, Launch::Machine & machine, GameType & type, std::string & fault, Launch::FaultSite * site)
{
	Launch::Machine taken;
	if (!taken.From_Load(request, false, fault, site)) {
		return(false);
	}

	char const * const name = request.Save.c_str();
	SaveVersionInfo info;
	if (!Get_Savefile_Info(name, &info)) {
		return(Fault(fault, "The saved game %s is missing or cannot be read.", name));
	}
	if (info.Get_Internal_Version() != ExpectedGameVersion) {
		return(Fault(fault, "The saved game %s was made by a different version of the game.", name));
	}

	type = (GameType)info.Get_Game_Type();
	if (type == GAME_IPX) {
		return(Fault(fault, "The saved game %s is from a LAN game set up in the menu, which a launcher cannot resume.", name));
	}

	if (type == GAME_INTERNET) {
		if (!taken.From_Load(request, true, fault, site)) {
			return(false);
		}

		int const countries = HouseTypes.Count();
		int const colors = Session.Color_Count();
		for (std::size_t index = 0; index < request.Players.size(); index++) {
			Launch::Message::Player const & player = request.Players[index];
			if (site != nullptr) {
				site->Player = int(index);
				site->Field = (player.Country < 0 || player.Country >= countries) ? "country" : "color";
			}
			if (player.Country < 0 || player.Country >= countries) {
				return(Fault(fault, "Player %d has country %d; the rules have %d, numbered from 0.", int(index), player.Country, countries));
			}
			if (player.Color < 0 || player.Color >= colors) {
				return(Fault(fault, "Player %d has color %d; the rules have %d, numbered from 0.", int(index), player.Color, colors));
			}
		}
	}

	if (site != nullptr) {
		*site = Launch::FaultSite();
	}
	machine = std::move(taken);
	return(true);
}


/// <summary>
/// Returns the lowest house of a player who is not an observer, the same house on every
/// machine, or -1 when every player is an observer.
/// </summary>
int Lowest_Player_House(void)
{
	int lowest = -1;
	for (int index = 0; index < Session.Players.Count(); index++) {
		NodeNameType const * node = Session.Players[index];
		if (node != NULL && !node->Player.IsObserver && (lowest == -1 || node->Player.ID < lowest)) {
			lowest = node->Player.ID;
		}
	}
	return(lowest);
}

}


/// <summary>
/// Returns the country, color, campaign, and global flag counts a launch is checked against.
/// Call it after Prepare_Side_Roster and once Launch::Is_Active is true, because the country
/// and color counts depend on them.
/// </summary>
Launch::Game::LimitsType Launch::Limits(void)
{
	return(Launch::Game::LimitsType{ HouseTypes.Count(), Session.Color_Count(), Campaigns.Count(), int(std::size(Environment.Globals)) });
}


/// <summary>
/// Sets up the session, the scenario, and the options for a launched game, in place of the
/// menus, and keeps both parts in Launch::Setup. Both parts must come from the same request.
/// Does not open the network; call Launch::Wire_Network for a multiplayer game once this
/// succeeds.
/// </summary>
/// <param name="fault">Where to leave the sentence describing why the parts cannot be applied.</param>
/// <returns>bool; Was the game set up? On false nothing has changed.</returns>
bool Launch::Apply(Launch::Game const & game, Launch::Machine const & machine, std::string & fault)
{
	bool const campaign = game.Kind == GameKind::GAME_KIND_CAMPAIGN;
	bool const multiplayer = game.Kind == GameKind::GAME_KIND_MULTIPLAYER;

	if (!campaign && game.Kind != GameKind::GAME_KIND_SKIRMISH && !multiplayer) {
		fault = "The launch is not a campaign, a skirmish, or a multiplayer game.";
		return(false);
	}
	if (!campaign && (machine.LocalPlayer < 0 || machine.LocalPlayer >= int(game.Players.size()) || !game.Players[machine.LocalPlayer].Human)) {
		fault = "The local player is not a human player of the launch.";
		return(false);
	}
	if (multiplayer != machine.Network.has_value()) {
		fault = multiplayer ? "A multiplayer launch needs network." : "Only a multiplayer launch sets network.";
		return(false);
	}

	Apply_Presentation(machine);
	Session.QuickMatch = game.Anonymous;

	Disable_Addon(ADDON_ANY);
	if (game.Options.Firestorm) {
		Enable_Addon(ADDON_FIRESTORM);
		Set_Required_Addon(ADDON_FIRESTORM);
	}

	if (campaign) {
		Apply_Campaign(game, machine);
	} else {
		Apply_Match(game, machine);
	}

	Launch::Setup = Launch::SetupType{ game, machine };
	return(true);
}


/// <summary>
/// Checks that a load request keeps its rules and names a save of this version from a kind of
/// game a launcher can resume. Changes no game state.
/// </summary>
/// <param name="fault">Where to leave the sentence describing why the save cannot be resumed.</param>
bool Launch::Judge_Load(Launch::Message::Load const & request, std::string & fault, Launch::FaultSite * site)
{
	Launch::Machine machine;
	GameType type = GAME_NORMAL;
	return(Take_Load(request, machine, type, fault, site));
}


/// <summary>
/// Resumes the saved game a load request names, in place of the menus. For a save from a game
/// against other machines, it opens the network before the load and gives each player of the
/// request the saved house with the same name.
/// </summary>
/// <param name="gameloaded">Set when the save loads, so the caller starts no scenario.</param>
/// <param name="fault">Where to leave the sentence describing why the game was not resumed.</param>
/// <returns>bool; Is the saved game running?</returns>
bool Launch::Apply_Load(Launch::Message::Load const & request, bool & gameloaded, std::string & fault)
{
	Launch::Machine machine;
	GameType type = GAME_NORMAL;
	if (!Take_Load(request, machine, type, fault, nullptr)) {
		return(false);
	}

	Apply_Presentation(machine);
	Session.QuickMatch = request.Anonymous;

	char const * const name = request.Save.c_str();
	if (type == GAME_INTERNET) {
		Launch::Game humans;
		for (Launch::Message::Player const & player : request.Players) {
			Launch::Game::PlayerType human;
			human.Name = player.Name;
			human.Human = true;
			human.Observer = player.Observer;
			human.Country = player.Country;
			human.Color = player.Color;
			human.Start = player.Start;
			humans.Players.push_back(std::move(human));
		}

		Clear_Vector(&Session.Players);
		Clear_Vector(&Session.Computers);
		Add_Human_Players(humans, machine);

		if (!Launch::Wire_Network(machine, fault)) {
			return(false);
		}
		Session.LoadGame = true;
	}

	if (!LoadOptionsClass().Load_File(name)) {
		return(Fault(fault, "The saved game %s could not be loaded.", name));
	}

	if (type == GAME_INTERNET && !Reconcile_Players()) {
		return(Fault(fault, "The players do not match the human players of the saved game %s.", name));
	}

	// The save's options apply, except this machine's waits and the game speed.
	if (machine.Network) {
		Session.ConnTimeout = machine.Network->ConnectTimeout;
		Session.ReconnectTimeout = machine.Network->ReconnectTimeout;
	}

	// Every machine starts at the saved speed, and the player with the lowest house changes it
	// for everyone through the speed order.
	if (type == GAME_INTERNET) {
		Session.Options.GameSpeed = std::clamp(Session.Options.GameSpeed, 0, OptionsClass::MAX_SPEED_SETTING - 1);
		if (request.GameSpeed && *request.GameSpeed != Session.Options.GameSpeed && PlayerPtr->HeapID == Lowest_Player_House()) {
			OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::GAMESPEED, *request.GameSpeed));
		}
	} else if (request.GameSpeed) {
		Session.Options.GameSpeed = *request.GameSpeed;
	}

	gameloaded = true;
	return(true);
}


/// <summary>
/// Opens the network a match against other machines is played over, through the tunnel the
/// launch gives or straight to each machine's address. Call it after Launch::Apply has filled
/// the player list.
/// </summary>
/// <param name="fault">Where to leave the sentence describing why the network did not open.</param>
/// <returns>bool; Is the network open?</returns>
bool Launch::Wire_Network(Launch::Machine const & machine, std::string & fault)
{
	if (!machine.Network) {
		fault = "Only a multiplayer launch opens a network.";
		return(false);
	}

	Launch::Machine::NetworkType const & network = *machine.Network;
	if (network.Tunnel) {
		Ipx.Configure_Tunnel(htons((unsigned short)network.Port),
			inet_addr(network.Tunnel->Address.c_str()), htons((unsigned short)network.Tunnel->Port));
	} else {
		Ipx.Configure_Direct_Peers((unsigned short)network.Port);
	}

	// The local player is first, so every player after it is another machine.
	for (int index = 1; index < Session.Players.Count(); index++) {
		Ipx.Add_Peer(Session.Players[index]->Address);
	}

	if (!Ipx.Init()) {
		fault = "The game could not open its network connection.";
		return(false);
	}

	// Loading-screen progress goes out before the in-game timing is set, and the manager's
	// default 33 ms retry floods a slow link.
	Ipx.Set_Timing(TIMER_SECOND, (unsigned int)-1, 10 * TIMER_SECOND);

	return(true);
}
