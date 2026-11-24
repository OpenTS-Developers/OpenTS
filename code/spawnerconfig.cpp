/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "spawnerconfig.h"

#include "ini.h"
#include "launch/gamelaunch.h"
#include "launch/launchmessages.h"
#include "utf8.h"

#include "diff.hh"

#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <vector>


namespace {

using Launch::Fault;
using Launch::Is_Address;
using Launch::Is_Tunnel_Number;

// The match's options, which also describe this machine's own seat.
char const * const SETTINGS = "Settings";


/// <summary>Reads a string entry, or the fallback when it is empty.</summary>
std::string Read_Text(INIClass const & ini, char const * section, char const * entry, std::string const & fallback)
{
	std::string text = ini.Get_String(section, entry);
	return(text.empty() ? fallback : text);
}


/// <summary>
/// Brings a wait written in a launch file within the bounds the game accepts.
/// </summary>
int Clamp_Timeout(int ticks)
{
	return(std::clamp(ticks, Spawner::Config::TIMEOUT_MIN, Spawner::Config::TIMEOUT_MAX));
}


/// <summary>
/// Reads one of the eight numbered entries a section names its seats by.
/// </summary>
/// <returns>The value written for that seat.</returns>
int Read_Slot_Int(INIClass const & ini, char const * section, int slot, int fallback)
{
	std::string entry = "Multi" + std::to_string(slot + 1);
	return(ini.Get_Int(section, entry.c_str(), fallback));
}


std::string Seat_Section(int position)
{
	return(position == 0 ? SETTINGS : "Other" + std::to_string(position));
}


}


/// <summary>
/// Reads the match's seats and sorts them into the order their houses are created in, which
/// is the order everything naming a seat by position means. A seat is human because the file
/// wrote a section for it.
/// </summary>
void Spawner::Config::Read_Slots(INIClass const & ini)
{
	std::array<SlotType, SLOT_COUNT> staging;

	for (int index = 0; index < SLOT_COUNT; index++) {
		std::string section = Seat_Section(index);

		SlotType & slot = staging[index];
		slot.FilePosition = index;
		if (ini.Section_Present(section.c_str())) {
			slot.Occupancy = OccupancyType::Human;
			// A seat is judged and ordered by the name the game keeps, the same on every machine.
			std::string name = Read_Text(ini, section.c_str(), "Name", "");
			slot.Name = name.substr(0, UTF8::Boundary_Before(name.c_str(), HOUSE_NAME_MAX - 1));
			slot.Color = ini.Get_Int(section.c_str(), "Color", -1);
			slot.Country = ini.Get_Int(section.c_str(), "Side", -1);
			slot.Address = Read_Text(ini, section.c_str(), "Ip", slot.Address);
			slot.Port = ini.Get_Int(section.c_str(), "Port", 0);
		} else {
			slot.Color = Read_Slot_Int(ini, "HouseColors", index, -1);
			slot.Country = Read_Slot_Int(ini, "HouseCountries", index, -1);
			slot.Handicap = Read_Slot_Int(ini, "HouseHandicaps", index, -1);
		}
	}

	// Sorting by color makes a seat's index the house it becomes. Each machine writes itself
	// first, so a name breaks a color tie rather than file order.
	std::vector<int> humans;
	std::vector<int> rest;
	for (int index = 0; index < SLOT_COUNT; index++) {
		(staging[index].Occupancy == OccupancyType::Human ? humans : rest).push_back(index);
	}
	std::stable_sort(humans.begin(), humans.end(), [&staging](int left, int right) {
		if (staging[left].Color != staging[right].Color) {
			return(staging[left].Color < staging[right].Color);
		}
		return(_stricmp(staging[left].Name.c_str(), staging[right].Name.c_str()) < 0);
	});

	HumanCount = (int)humans.size();
	LocalSlot = 0;

	int filled = 0;
	for (int index : humans) {
		if (index == 0) {
			LocalSlot = filled;
		}
		Slots[filled++] = staging[index];
	}

	// A seat no section claimed is a computer player, up to AIPlayers of them.
	for (int index : rest) {
		SlotType & slot = Slots[filled];
		slot = staging[index];
		slot.Occupancy = (filled - HumanCount) < AIPlayers ? OccupancyType::Computer : OccupancyType::Empty;
		filled++;
	}

	// The alliance sections name seats in sorted order, so they are read after the sort.
	static char const * const _ordinals[SLOT_COUNT] = {
		"HouseAllyOne", "HouseAllyTwo", "HouseAllyThree", "HouseAllyFour",
		"HouseAllyFive", "HouseAllySix", "HouseAllySeven", "HouseAllyEight"
	};

	for (int index = 0; index < SLOT_COUNT; index++) {
		SlotType & slot = Slots[index];

		std::string entry = "Multi" + std::to_string(index + 1);
		slot.IsSpectator = ini.Get_Bool("IsSpectator", entry.c_str(), false);
		slot.StartingPosition = Read_Slot_Int(ini, "SpawnLocations", index, -1);

		// A start position out of range is left to the game, like no position at all.
		if (slot.StartingPosition < -1 || slot.StartingPosition >= SLOT_COUNT) {
			slot.StartingPosition = -1;
		}

		std::string section = "Multi" + std::to_string(index + 1) + "_Alliances";
		if (!ini.Section_Present(section.c_str())) {
			continue;
		}

		for (int ally = 0; ally < SLOT_COUNT; ally++) {
			slot.Alliances[ally] = ini.Get_Int(section.c_str(), _ordinals[ally], -1);
		}
	}
}


/// <summary>
/// What kind of game this file asks for. A resume answers by itself, because the save carries
/// the type, the options and the houses.
/// </summary>
Spawner::Config::LaunchType Spawner::Config::Launch_Type(void) const
{
	if (LoadSaveGame) {
		return(LaunchType::Resume);
	}
	if (IsCampaign) {
		return(LaunchType::Campaign);
	}
	if (HumanCount > 1) {
		return(LaunchType::Multiplayer);
	}
	return(LaunchType::Skirmish);
}


/// <summary>
/// The difficulty a seat is played at. A client may ask for an easier opponent than the game
/// has, and any such request comes to the easiest one it does have.
/// </summary>
/// <returns>The difficulty to play the seat at, or -1 for the session default.</returns>
int Spawner::Config::Playable_Handicap(int asked)
{
	if (asked < 0) {
		return(-1);
	}
	// The rules' hardest table makes the easiest opponent, so an easier request lands there.
	if (asked > DIFF_HARD) {
		return(DIFF_HARD);
	}
	return(asked);
}


/// <summary>
/// Returns the name a computer player is shown under: its difficulty's name when the file asks
/// for names by difficulty, and otherwise empty, which leaves the name to the game.
/// </summary>
std::string Spawner::Config::Computer_Name(SlotType const & slot) const
{
	// Handicap 0 is the rules' easy table, which makes the hardest opponent.
	static char const * const _ai_names[DIFF_COUNT] = { "Hard AI", "Medium AI", "Easy AI" };

	if (!AINamesByDifficulty) {
		return(std::string());
	}

	// Session difficulty is numbered the opposite way round from a seat's handicap.
	int played = Playable_Handicap(slot.Handicap);
	if (played < 0) {
		played = DIFF_COUNT - 1 - AIDifficulty;
	}
	return(_ai_names[std::clamp(played, 0, DIFF_COUNT - 1)]);
}


void Spawner::Config::To_Session(Launch::Message::Session & session) const
{
	session.LocalPlayer = Launch_Type() == LaunchType::Campaign ? 0u : std::uint32_t(LocalSlot);
	session.Host = IsHost;

	// The game treats a negative interval or ring position as 0.
	session.AutosaveInterval = std::uint32_t(std::max(AutoSaveInterval, 0));
	session.NextCampaignAutosave = std::uint32_t(std::max(NextCampaignAutoSave, 0));
	session.NextSkirmishAutosave = std::uint32_t(std::max(NextSkirmishAutoSave, 0));

	session.SkipScoreScreen = SkipScoreScreen;
	session.WriteStatistics = WriteStatistics;
	session.GameId = std::uint32_t(GameID);
	session.MapHash = MapHash;
	session.MapName = MapName;
	session.DifficultyName = DifficultyName;
	session.LoadScreen.Image = CustomLoadScreen;

	// The game uses the position only when both coordinates are above 0, but a save keeps both,
	// so both are sent when either is set.
	if (CustomLoadScreenX != 0 || CustomLoadScreenY != 0) {
		session.LoadScreen.X = CustomLoadScreenX;
		session.LoadScreen.Y = CustomLoadScreenY;
	}
}


void Spawner::Config::To_Network(Launch::Message::Network & network) const
{
	bool const tunnel = TunnelPort != 0;
	if (tunnel) {
		network.Port = TunnelId;
		network.Tunnel.emplace();
		network.Tunnel->Address = TunnelAddress;
		network.Tunnel->Port = std::uint32_t(TunnelPort);
	} else {
		network.Port = ListenPort;
	}

	for (int index = 0; index < HumanCount; index++) {
		if (index == LocalSlot) {
			continue;
		}
		Launch::Message::Peer peer;
		peer.Player = std::uint32_t(index);
		peer.Address = tunnel ? std::string() : Slots[index].Address;
		peer.Port = Slots[index].Port;
		network.Peers.push_back(std::move(peer));
	}

	// The request gives the waits in milliseconds, 16 to a tick.
	network.ConnectTimeout = std::uint32_t(Clamp_Timeout(ConnTimeout)) * 16;
	network.ReconnectTimeout = std::uint32_t(Clamp_Timeout(ReconnectTimeout)) * 16;
}


/// <summary>
/// Writes the launch request this file describes: a load request for a resume, and a play
/// request otherwise. Every machine's file for one match gives the same game. A load request
/// lists the human players and the network whenever the file has a human player, because only
/// the save tells whether the game was played against other machines.
/// </summary>
void Spawner::Config::To_Request(Launch::Message::Request & request) const
{
	request = Launch::Message::Request();
	request.Version = Launch::REQUEST_VERSION;

	LaunchType const type = Launch_Type();
	if (type == LaunchType::Resume) {
		Launch::Message::Load load;
		load.Save = SaveGameName;
		To_Session(load.Session);
		load.GameSpeed = GameSpeed;
		load.Anonymous = QuickMatch;

		if (HumanCount > 0) {
			for (int index = 0; index < HumanCount; index++) {
				SlotType const & slot = Slots[index];
				Launch::Message::Player player;
				player.Name = (slot.Name.empty() && HumanCount == 1) ? "Player" : slot.Name;
				player.Human = true;
				player.Observer = slot.IsSpectator;
				player.Country = slot.Country;
				player.Color = slot.Color;
				player.Start = slot.StartingPosition;
				load.Players.push_back(std::move(player));
			}
			To_Network(load.Network.emplace());
		}

		request.Verb = std::move(load);
		return;
	}

	Launch::Game game;
	game.Anonymous = QuickMatch;

	// A network match uses the file's seed, 0 included; a game played alone draws one for 0.
	if (type == LaunchType::Multiplayer || Seed != 0) {
		game.Seed = std::uint32_t(Seed);
	}

	Launch::Game::OptionsType & options = game.Options;
	options.AlliesAllowed = AlliesAllowed;
	options.AttackNeutralUnits = AttackNeutralUnits;
	options.AutoDeployMCV = AutoDeployMCV;
	options.Bases = Bases;
	options.BridgeDestruction = BridgeDestroy;
	options.BuildOffAlly = BuildOffAlly;
	options.CoachMode = CoachMode;
	options.Crates = Crates;
	options.FogOfWar = FogOfWar;
	options.HarvesterTruce = HarvesterTruce;
	options.MCVRedeploy = MCVRedeploy;
	options.MultiEngineer = MultiEngineer;
	options.PlayMovies = PlayMoviesInMultiplayer;
	options.ScrapMetal = ScrapMetal;
	options.SharedControl = SharedControl;
	options.ShortGame = ShortGame;
	options.SurrenderOnDisconnect = AutoSurrender;
	options.AIDifficulty = AIDifficulty;
	options.Credits = Credits;
	options.TechLevel = TechLevel;
	options.UnitCount = UnitCount;
	options.Firestorm = Firestorm;
	options.GameSpeed = GameSpeed;
	options.Campaign = CampaignID;
	options.CampaignPlayerDifficulty = CampaignDifficulty;
	options.CampaignComputerDifficulty = CampaignCDifficulty;

	if (type == LaunchType::Campaign) {
		game.Kind = Launch::GameKind::GAME_KIND_CAMPAIGN;
		for (int index = 0; index < GLOBAL_FLAG_COUNT; index++) {
			if (GlobalFlags[index]) {
				game.GlobalFlags.push_back(index);
			}
		}

		// The mission gives the player a house, so only the name is listed.
		Launch::Game::PlayerType player;
		player.Human = true;
		player.Name = HumanCount > 0 ? Slots[LocalSlot].Name : std::string();
		if (player.Name.empty()) {
			player.Name = "Player";
		}
		game.Players.push_back(std::move(player));
	} else {
		game.Kind = type == LaunchType::Multiplayer
			? Launch::GameKind::GAME_KIND_MULTIPLAYER : Launch::GameKind::GAME_KIND_SKIRMISH;

		// Read_Slots puts the occupied slots first, human players before computer players.
		int occupied = 0;
		while (occupied < SLOT_COUNT && Slots[occupied].Occupancy != OccupancyType::Empty) {
			occupied++;
		}

		for (int index = 0; index < occupied; index++) {
			SlotType const & slot = Slots[index];

			Launch::Game::PlayerType player;
			player.Human = slot.Occupancy == OccupancyType::Human;
			player.Observer = player.Human && slot.IsSpectator;
			player.Country = slot.Country;
			player.Color = slot.Color;
			player.Start = slot.StartingPosition;

			if (player.Human) {
				player.Name = (slot.Name.empty() && type == LaunchType::Skirmish) ? "Player" : slot.Name;
			} else {
				player.Name = Computer_Name(slot);
				int level = Playable_Handicap(slot.Handicap);
				if (level >= 0) {
					player.AILevel = level;
				}
			}

			// The game ignores an alliance with the player itself or with an observer, and every
			// alliance an observer lists.
			if (!player.Observer) {
				for (int ally : slot.Alliances) {
					if (ally < 0 || ally == index) {
						continue;
					}
					if (ally < occupied && Slots[ally].Occupancy == OccupancyType::Human && Slots[ally].IsSpectator) {
						continue;
					}
					player.Allies.push_back(ally);
				}
				std::sort(player.Allies.begin(), player.Allies.end());
				player.Allies.erase(std::unique(player.Allies.begin(), player.Allies.end()), player.Allies.end());
			}

			game.Players.push_back(std::move(player));
		}
	}

	Launch::Message::Play play;
	play.Game = game.To_Message();
	play.MapPath = ScenarioName;
	To_Session(play.Session);
	if (type == LaunchType::Multiplayer) {
		To_Network(play.Network.emplace());
	}

	request.Verb = std::move(play);
}


/// <summary>
/// Checks the values that translating the file into a launch request changes or drops: the
/// number of computer players, a slot's difficulty, and an alliance entry.
/// </summary>
/// <param name="fault">Where to leave the sentence describing the first fault found.</param>
bool Spawner::Config::Is_Launchable(std::string & fault) const
{
	LaunchType const kind = Launch_Type();
	if (kind == LaunchType::Resume || kind == LaunchType::Campaign) {
		return(true);
	}

	int const free_seats = SLOT_COUNT - HumanCount;
	if (AIPlayers < 0 || AIPlayers > free_seats) {
		return(Fault(fault, "SPAWN.INI [Settings] AIPlayers: %d is outside 0 to %d, the free seats.", AIPlayers, free_seats));
	}

	for (int index = 0; index < SLOT_COUNT; index++) {
		SlotType const & slot = Slots[index];
		if (slot.Occupancy == OccupancyType::Empty) {
			continue;
		}

		if (slot.Handicap < -1 || slot.Handicap > 6) {
			return(Fault(fault, "SPAWN.INI [HouseHandicaps] Multi%d: %d is outside -1 to 6.", slot.FilePosition + 1, slot.Handicap));
		}

		for (int ally : slot.Alliances) {
			if (ally < -1) {
				return(Fault(fault, "SPAWN.INI [Multi%d_Alliances]: %d is not a seat; use 0 to %d, or -1 for none.", index + 1, ally, SLOT_COUNT - 1));
			}
		}
	}

	return(true);
}


/// <summary>
/// Returns text prefixed with SPAWN.INI and, when site points to one, its section and key.
/// </summary>
/// <param name="site">Where the request's check found the fault.</param>
/// <param name="text">The sentence the check wrote.</param>
std::string Spawner::Config::Describe_Fault(Launch::FaultSite const & site, std::string const & text) const
{
	static constexpr struct { char const * Field; char const * Key; } _settings[] = {
		{ "seed", "Seed" },
		{ "map_path", "Scenario" },
		{ "save", "SaveGameName" },
		{ "game_speed", "GameSpeed" },
		{ "network.port", "Port" },
	};

	// The launch option keys that SPAWN.INI spells differently; the rest are spelled alike.
	static constexpr struct { char const * Option; char const * Key; } _renamed[] = {
		{ "BridgeDestruction", "BridgeDestroy" },
		{ "Campaign", "CampaignID" },
		{ "CampaignComputerDifficulty", "DifficultyModeComputer" },
		{ "CampaignPlayerDifficulty", "DifficultyModeHuman" },
		{ "PlayMovies", "PlayMoviesInMultiplayer" },
		{ "SurrenderOnDisconnect", "AutoSurrender" },
	};

	std::string where;
	if (site.Field.rfind("options.", 0) == 0) {
		std::string key = site.Field.substr(std::strlen("options."));
		for (auto const & renamed : _renamed) {
			if (key == renamed.Option) {
				key = renamed.Key;
			}
		}
		where = "[Settings] " + key;
	} else if (site.Field == "network.tunnel.address") {
		where = "[Tunnel] Ip";
	} else if (site.Field == "network.tunnel.port") {
		where = "[Tunnel] Port";
	} else if (site.Field == "global_flags") {
		where = "[GlobalFlags]";
	} else if (site.Player >= 0 && site.Player < SLOT_COUNT) {
		// A campaign request lists only the local player, whatever slot the file gave them.
		int const index = Launch_Type() == LaunchType::Campaign ? LocalSlot : site.Player;
		SlotType const & slot = Slots[index];
		bool const human = slot.Occupancy == OccupancyType::Human;
		std::string const section = "[" + Seat_Section(slot.FilePosition) + "] ";
		std::string const entry = " Multi" + std::to_string(slot.FilePosition + 1);
		std::string const slot_entry = "Multi" + std::to_string(index + 1);

		if (site.Field == "name") {
			where = section + "Name";
		} else if (site.Field == "country") {
			where = human ? section + "Side" : "[HouseCountries]" + entry;
		} else if (site.Field == "color") {
			where = human ? section + "Color" : "[HouseColors]" + entry;
		} else if (site.Field == "ai_level") {
			where = "[HouseHandicaps]" + entry;
		} else if (site.Field == "observer") {
			where = "[IsSpectator] " + slot_entry;
		} else if (site.Field == "start") {
			where = "[SpawnLocations] " + slot_entry;
		} else if (site.Field == "allies") {
			where = "[" + slot_entry + "_Alliances]";
		} else if (site.Field == "peer.port") {
			where = section + "Port";
		} else if (site.Field == "peer.address") {
			where = section + "Ip";
		}
	} else {
		for (auto const & setting : _settings) {
			if (site.Field == setting.Field) {
				where = std::string("[Settings] ") + setting.Key;
			}
		}
	}

	return(where.empty() ? "SPAWN.INI: " + text : "SPAWN.INI " + where + ": " + text);
}


/// <summary>
/// Reads what the client asked the game to launch. Reading cannot fail: an unwritten key has
/// a settled meaning, an unusable value keeps it, and an unknown key is passed over.
/// </summary>
void Spawner::Config::Read_INI(INIClass const & ini)
{
	IsCampaign = ini.Get_Bool(SETTINGS, "IsSinglePlayer", IsCampaign);
	IsHost = ini.Get_Bool(SETTINGS, "Host", IsHost);
	CampaignID = ini.Get_Int(SETTINGS, "CampaignID", CampaignID);

	ScenarioName = Read_Text(ini, SETTINGS, "Scenario", ScenarioName);
	MapName = Read_Text(ini, SETTINGS, "UIMapName", MapName);

	LoadSaveGame = ini.Get_Bool(SETTINGS, "LoadSaveGame", LoadSaveGame);

	// Saves are opened by name in the saved-games folder, so a path is cut to its last part.
	SaveGameName = std::filesystem::path(Read_Text(ini, SETTINGS, "SaveGameName", SaveGameName)).filename().string();

	AutoSaveInterval = ini.Get_Int(SETTINGS, "AutoSaveGame", AutoSaveInterval);

	// The client counts automatic saves from one; the game counts from zero.
	NextCampaignAutoSave = ini.Get_Int(SETTINGS, "NextSPAutoSaveId", 1) - 1;
	NextSkirmishAutoSave = ini.Get_Int(SETTINGS, "NextSkirmishAutoSaveId", 1) - 1;

	Bases = ini.Get_Bool(SETTINGS, "Bases", Bases);
	Credits = ini.Get_Int(SETTINGS, "Credits", Credits);
	BridgeDestroy = ini.Get_Bool(SETTINGS, "BridgeDestroy", BridgeDestroy);
	Crates = ini.Get_Bool(SETTINGS, "Crates", Crates);
	ShortGame = ini.Get_Bool(SETTINGS, "ShortGame", ShortGame);
	BuildOffAlly = ini.Get_Bool(SETTINGS, "BuildOffAlly", BuildOffAlly);
	GameSpeed = ini.Get_Int(SETTINGS, "GameSpeed", GameSpeed);
	MultiEngineer = ini.Get_Bool(SETTINGS, "MultiEngineer", MultiEngineer);
	UnitCount = ini.Get_Int(SETTINGS, "UnitCount", UnitCount);
	AIPlayers = ini.Get_Int(SETTINGS, "AIPlayers", AIPlayers);
	AIDifficulty = ini.Get_Int(SETTINGS, "AIDifficulty", AIDifficulty);
	AlliesAllowed = ini.Get_Bool(SETTINGS, "AlliesAllowed", AlliesAllowed);
	HarvesterTruce = ini.Get_Bool(SETTINGS, "HarvesterTruce", HarvesterTruce);
	FogOfWar = ini.Get_Bool(SETTINGS, "FogOfWar", FogOfWar);
	MCVRedeploy = ini.Get_Bool(SETTINGS, "MCVRedeploy", MCVRedeploy);
	AutoDeployMCV = ini.Get_Bool(SETTINGS, "AutoDeployMCV", AutoDeployMCV);
	Seed = ini.Get_Int(SETTINGS, "Seed", Seed);
	TechLevel = ini.Get_Int(SETTINGS, "TechLevel", TechLevel);
	Firestorm = ini.Get_Bool(SETTINGS, "Firestorm", Firestorm);
	CampaignDifficulty = ini.Get_Int(SETTINGS, "DifficultyModeHuman", CampaignDifficulty);
	CampaignCDifficulty = ini.Get_Int(SETTINGS, "DifficultyModeComputer", CampaignCDifficulty);

	// One key serves twice: the port the game listens on, and this machine's tunnel number.
	TunnelId = ini.Get_Int(SETTINGS, "Port", TunnelId);
	ListenPort = ini.Get_Int(SETTINGS, "Port", ListenPort);
	TunnelAddress = Read_Text(ini, "Tunnel", "Ip", TunnelAddress);
	TunnelPort = ini.Get_Int("Tunnel", "Port", TunnelPort);

	ConnTimeout = Clamp_Timeout(ini.Get_Int(SETTINGS, "ConnTimeout", ConnTimeout));
	ReconnectTimeout = Clamp_Timeout(ini.Get_Int(SETTINGS, "ReconnectTimeout", ReconnectTimeout));

	QuickMatch = ini.Get_Bool(SETTINGS, "QuickMatch", QuickMatch);
	SkipScoreScreen = ini.Get_Bool(SETTINGS, "SkipScoreScreen", SkipScoreScreen);
	AINamesByDifficulty = ini.Get_Bool(SETTINGS, "DifficultyBasedAINames", AINamesByDifficulty);
	CoachMode = ini.Get_Bool(SETTINGS, "CoachMode", CoachMode);
	AutoSurrender = ini.Get_Bool(SETTINGS, "AutoSurrender", AutoSurrender);
	AttackNeutralUnits = ini.Get_Bool(SETTINGS, "AttackNeutralUnits", AttackNeutralUnits);
	ScrapMetal = ini.Get_Bool(SETTINGS, "ScrapMetal", ScrapMetal);
	SharedControl = ini.Get_Bool(SETTINGS, "SharedControl", SharedControl);
	PlayMoviesInMultiplayer = ini.Get_Bool(SETTINGS, "PlayMoviesInMultiplayer", PlayMoviesInMultiplayer);
	CustomLoadScreen = Read_Text(ini, SETTINGS, "CustomLoadScreen", CustomLoadScreen);
	DifficultyName = Read_Text(ini, SETTINGS, "DifficultyName", DifficultyName);

	WriteStatistics = ini.Get_Bool(SETTINGS, "WriteStatistics", WriteStatistics);
	GameID = ini.Get_Int(SETTINGS, "GameID", GameID);
	MapHash = Read_Text(ini, SETTINGS, "MapHash", MapHash);

	std::string position = Read_Text(ini, SETTINGS, "CustomLoadScreenPos", "");
	if (!position.empty()) {
		int x = 0;
		int y = 0;
		if (std::sscanf(position.c_str(), "%d,%d", &x, &y) == 2) {
			CustomLoadScreenX = x;
			CustomLoadScreenY = y;
		}
	}

	for (int index = 0; index < GLOBAL_FLAG_COUNT; index++) {
		std::string entry = "GlobalFlag" + std::to_string(index);
		GlobalFlags[index] = ini.Get_Bool("GlobalFlags", entry.c_str(), false);
	}

	Read_Slots(ini);
}
