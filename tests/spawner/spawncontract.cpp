/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins the launch file the CnCNet client writes, with no engine and no game data: unwritten
// keys, seat order, the reader's silent repairs, and the values two machines must agree on.

#include "doctest/doctest.h"
#include "ini.h"
#include "launch/gamelaunch.h"
#include "launch/launchmessages.h"
#include "spawnerconfig.h"
#include "xstraw.h"

#include <cstring>
#include <string>
#include <variant>

namespace {

using LaunchType = Spawner::Config::LaunchType;
using OccupancyType = Spawner::Config::OccupancyType;


// What the client writes to resume a saved campaign. It names almost nothing: the saved game
// carries the houses, the options and the kind of game they were played as.
char const _Resume[] =
	"[Settings]\n"
	"Scenario=spawnmap.ini\n"
	"SaveGameName=SAVEGAME.001\n"
	"LoadSaveGame=Yes\n"
	"SidebarHack=True\n"
	"CustomLoadScreen=Resources/l600s01.pcx\n"
	"Firestorm=No\n"
	"GameSpeed=1\n";


// What the client writes to start a campaign mission. It names no player and no color, since
// the map says who is playing.
char const _Campaign[] =
	"[Settings]\n"
	"Scenario=spawnmap.ini\n"
	"CampaignID=-1\n"
	"GameSpeed=1\n"
	"Firestorm=False\n"
	"CustomLoadScreen=Resources/l600s01.pcx\n"
	"IsSinglePlayer=Yes\n"
	"SidebarHack=True\n"
	"Side=0\n"
	"BuildOffAlly=False\n"
	"DifficultyModeHuman=0\n"
	"DifficultyModeComputer=2\n";


// What the client writes to start a game against computer players. The player is described
// in the settings, and the computer players are named by position.
char const _Skirmish[] =
	"[Settings]\n"
	"Scenario=spawnmap.ini\n"
	"Name=Commander\n"
	"Side=1\n"
	"Color=4\n"
	"Port=1234\n"
	"AIPlayers=2\n"
	"Credits=5000\n"
	"ShortGame=Yes\n"
	"Protocol=0\n"
	"NextSkirmishAutoSaveId=5\n"
	"\n"
	"[HouseColors]\n"
	"Multi2=2\n"
	"Multi3=7\n"
	"\n"
	"[HouseCountries]\n"
	"Multi2=0\n"
	"Multi3=1\n"
	"\n"
	"[HouseHandicaps]\n"
	"Multi2=2\n"
	"Multi3=0\n"
	"\n"
	"[SpawnLocations]\n"
	"Multi1=3\n"
	"Multi2=9\n"
	"Multi3=-2\n"
	"\n"
	"[Multi1_Alliances]\n"
	"HouseAllyOne=1\n";


// What the client writes for a game against other machines. The player who wrote it holds the
// higher color, so the sort puts that seat second.
char const _Network[] =
	"[Settings]\n"
	"Scenario=spawnmap.ini\n"
	"Name=Second\n"
	"Side=1\n"
	"Color=5\n"
	"Port=50000\n"
	"Host=No\n"
	"\n"
	"[Other1]\n"
	"Name=First\n"
	"Side=0\n"
	"Color=1\n"
	"Ip=10.0.0.7\n"
	"Port=50001\n"
	"\n"
	"[Tunnel]\n"
	"Ip=88.99.11.22\n"
	"Port=50010\n"
	"\n"
	"[IsSpectator]\n"
	"Multi1=Yes\n"
	"\n"
	"[Multi2_Alliances]\n"
	"HouseAllyOne=0\n";


Spawner::Config Read(char const * text, int length)
{
	INIClass ini;
	BufferStraw straw(text, length);
	ini.Load(straw);

	Spawner::Config config;
	config.Read_INI(ini);
	return(config);
}


// The play request a file gives; every file except a resume gives one.
Launch::Message::Play Play_Of(Spawner::Config const & config)
{
	Launch::Message::Request request;
	config.To_Request(request);
	REQUIRE(std::holds_alternative<Launch::Message::Play>(request.Verb));
	return(std::get<Launch::Message::Play>(request.Verb));
}


// The load request a resume gives, after it is encoded and decoded.
Launch::Message::Load Load_Of(Spawner::Config const & config)
{
	Launch::Message::Request request;
	config.To_Request(request);

	Launch::Bytes bytes;
	Launch::Encode(request, bytes);
	Launch::Message::Request back;
	REQUIRE(Launch::Decode_Request(bytes, back) == Launch::RequestFault::NONE);
	REQUIRE(std::holds_alternative<Launch::Message::Load>(back.Verb));
	return(std::get<Launch::Message::Load>(back.Verb));
}


Launch::Bytes Game_Bytes(Launch::Message::Play const & play)
{
	Launch::Bytes bytes;
	Launch::Encode(play.Game, bytes);
	return(bytes);
}


/// <summary>
/// Is the play request a file gives accepted after it is encoded and decoded? fault describes
/// the first refusal.
/// </summary>
bool Request_Is_Taken(Spawner::Config const & config, int countries, int colors, int campaigns, std::string & fault)
{
	Launch::Message::Request request;
	config.To_Request(request);
	if (!std::holds_alternative<Launch::Message::Play>(request.Verb)) {
		fault = "no play request";
		return(false);
	}

	Launch::Bytes bytes;
	Launch::Encode(request, bytes);
	Launch::Message::Request back;
	if (Launch::Decode_Request(bytes, back) != Launch::RequestFault::NONE) {
		fault = "the request does not decode";
		return(false);
	}
	Launch::Bytes again;
	Launch::Encode(back, again);
	if (again != bytes) {
		fault = "the request does not encode to the same bytes twice";
		return(false);
	}

	Launch::Message::Play const & play = std::get<Launch::Message::Play>(back.Verb);
	Launch::Game game;
	if (!game.From_Message(play.Game, Launch::Game::LimitsType{ countries, colors, campaigns, 50 }, fault)) {
		return(false);
	}
	Launch::Machine machine;
	return(machine.From_Request(play, game, fault));
}


/// <summary>
/// Does the game accept what a file describes? The file's checks come first, then the rules of
/// the request it gives; a resume with more than one human player is judged as a save from a
/// game against other machines. fault describes the first refusal in the file's terms.
/// </summary>
bool File_Is_Taken(Spawner::Config const & config, int countries, int colors, int campaigns, std::string & fault)
{
	fault.clear();
	if (!config.Is_Launchable(fault)) {
		return(false);
	}

	Launch::Message::Request request;
	config.To_Request(request);

	Launch::FaultSite site;
	bool taken = false;
	if (Launch::Message::Load const * load = std::get_if<Launch::Message::Load>(&request.Verb)) {
		Launch::Machine machine;
		taken = machine.From_Load(*load, config.HumanCount > 1, fault, &site);
	} else {
		Launch::Message::Play const & play = std::get<Launch::Message::Play>(request.Verb);
		Launch::Game game;
		Launch::Machine machine;
		taken = game.From_Message(play.Game, Launch::Game::LimitsType{ countries, colors, campaigns, 50 }, fault, &site)
			&& machine.From_Request(play, game, fault, &site);
	}

	if (!taken) {
		fault = config.Describe_Fault(site, fault);
	}
	return(taken);
}


bool Judge(char const * text, int length, int countries, int colors, std::string & fault)
{
	return(File_Is_Taken(Read(text, length), countries, colors, 1, fault));
}

}


// A file naming nothing at all still describes a playable game, because every key the client
// leaves out has a settled meaning.
TEST_CASE("a file naming nothing at all still describes a playable game")
{
	char const empty[] = "[Settings]\n";
	Spawner::Config config = Read(empty, sizeof(empty) - 1);

	CHECK_MESSAGE(config.Bases, "the unwritten keys keep the meaning clients expect");
	CHECK_MESSAGE(config.Credits == 10000, "the unwritten keys keep the meaning clients expect");
	CHECK_MESSAGE(config.MCVRedeploy, "the unwritten keys keep the meaning clients expect");
	CHECK_MESSAGE(config.TunnelId == 0, "one absent port names no tunnel and still listens");
	CHECK_MESSAGE(config.ListenPort == 1234, "one absent port names no tunnel and still listens");
	CHECK_MESSAGE(config.ScenarioName == "spawnmap.ini", "the scenario a client always writes is also the default");
	CHECK_MESSAGE(config.NextCampaignAutoSave == 0, "the first automatic save is numbered from zero");
	CHECK_MESSAGE(config.NextSkirmishAutoSave == 0, "the first automatic save is numbered from zero");
	CHECK_MESSAGE(config.AutoSaveInterval == 0, "an unwritten interval saves nothing automatically");
	CHECK_MESSAGE(config.ConnTimeout == 3600, "the unwritten waits are the ones every spawner has kept");
	CHECK_MESSAGE(config.ReconnectTimeout == 2400, "the unwritten waits are the ones every spawner has kept");
	CHECK_MESSAGE(config.AutoSurrender, "a departing player surrenders unless the file says otherwise");
	CHECK_MESSAGE(!config.BuildOffAlly, "nobody builds off an ally unless the file asks for it");
	CHECK_MESSAGE(!config.AutoDeployMCV, "a starting base unit waits to be deployed by hand");
	CHECK_MESSAGE(!config.AttackNeutralUnits, "a target scan passes over a neutral house unasked");

	bool any = false;
	for (bool flag : config.GlobalFlags) {
		any = any || flag;
	}
	CHECK_MESSAGE(!any, "no scenario flag is set unasked");
}


// A client writes the interval only when it wants automatic saves, in frames, and hands back -1
// for a loaded save that was not one; the ring is what starts that over.
TEST_CASE("a client writes the interval only when it wants automatic saves, in frames")
{
	char const off[] =
		"[Settings]\n"
		"AutoSaveGame=0\n"
		"NextSPAutoSaveId=-1\n";
	Spawner::Config config = Read(off, sizeof(off) - 1);

	CHECK_MESSAGE(config.AutoSaveInterval == 0, "an interval of zero saves nothing automatically");
	CHECK_MESSAGE(config.NextCampaignAutoSave == -2, "a loaded save that was no autosave is handed on as read");

	char const on[] =
		"[Settings]\n"
		"AutoSaveGame=10800\n";
	config = Read(on, sizeof(on) - 1);

	CHECK_MESSAGE(config.AutoSaveInterval == 10800, "the interval is read in frames as written");
}


// The waits are written in ticks, and a value the game cannot wait for is brought within the
// bounds rather than refusing a match that is otherwise playable.
TEST_CASE("the waits are written in ticks")
{
	char const written[] =
		"[Settings]\n"
		"ConnTimeout=1800\n"
		"ReconnectTimeout=1400\n";
	Spawner::Config config = Read(written, sizeof(written) - 1);

	CHECK_MESSAGE(config.ConnTimeout == 1800, "a wait within the bounds is taken in ticks as written");
	CHECK_MESSAGE(config.ReconnectTimeout == 1400, "a wait within the bounds is taken in ticks as written");

	char const low[] =
		"[Settings]\n"
		"ConnTimeout=1\n"
		"ReconnectTimeout=-5\n";
	config = Read(low, sizeof(low) - 1);

	CHECK_MESSAGE(config.ConnTimeout == Spawner::Config::TIMEOUT_MIN, "a wait shorter than the game can honor is raised to the floor");
	CHECK_MESSAGE(config.ReconnectTimeout == Spawner::Config::TIMEOUT_MIN, "a wait shorter than the game can honor is raised to the floor");

	char const high[] =
		"[Settings]\n"
		"ConnTimeout=999999\n"
		"ReconnectTimeout=999999\n";
	config = Read(high, sizeof(high) - 1);

	CHECK_MESSAGE(config.ConnTimeout == Spawner::Config::TIMEOUT_MAX, "a wait longer than the game will hold is lowered to the ceiling");
	CHECK_MESSAGE(config.ReconnectTimeout == Spawner::Config::TIMEOUT_MAX, "a wait longer than the game will hold is lowered to the ceiling");
}


TEST_CASE("a departing player's base is destroyed unless the file asks for the computer to take it")
{
	char const off[] =
		"[Settings]\n"
		"AutoSurrender=No\n";
	Spawner::Config config = Read(off, sizeof(off) - 1);

	CHECK_MESSAGE(!config.AutoSurrender, "the computer takes the seat when the file says so");

	char const spelled[] =
		"[Settings]\n"
		"AutoSurrender=False\n";
	config = Read(spelled, sizeof(spelled) - 1);

	CHECK_MESSAGE(!config.AutoSurrender, "a forced option is read whichever way it spells no");
}


TEST_CASE("building next to an ally's base is asked for by the file alone")
{
	char const on[] =
		"[Settings]\n"
		"BuildOffAlly=Yes\n";
	Spawner::Config config = Read(on, sizeof(on) - 1);

	CHECK_MESSAGE(config.BuildOffAlly, "an ally's base anchors a placement when the file says so");

	char const spelled[] =
		"[Settings]\n"
		"BuildOffAlly=True\n";
	config = Read(spelled, sizeof(spelled) - 1);

	CHECK_MESSAGE(config.BuildOffAlly, "the option is read whichever way it spells yes");
}


TEST_CASE("deploying the starting base unit is asked for by the file alone")
{
	char const on[] =
		"[Settings]\n"
		"AutoDeployMCV=Yes\n";
	Spawner::Config config = Read(on, sizeof(on) - 1);

	CHECK_MESSAGE(config.AutoDeployMCV, "the base unit deploys itself when the file says so");

	char const spelled[] =
		"[Settings]\n"
		"AutoDeployMCV=True\n";
	config = Read(spelled, sizeof(spelled) - 1);

	CHECK_MESSAGE(config.AutoDeployMCV, "the option is read whichever way it spells yes");
}


TEST_CASE("acquiring a neutral house's objects is asked for by the file alone")
{
	char const on[] =
		"[Settings]\n"
		"AttackNeutralUnits=Yes\n";
	Spawner::Config config = Read(on, sizeof(on) - 1);

	CHECK_MESSAGE(config.AttackNeutralUnits, "a neutral house is scanned when the file says so");

	char const spelled[] =
		"[Settings]\n"
		"AttackNeutralUnits=True\n";
	config = Read(spelled, sizeof(spelled) - 1);

	CHECK_MESSAGE(config.AttackNeutralUnits, "the option is read whichever way it spells yes");
}


// Resuming a saved game asks for almost nothing; the save answers for the rest, so the reading
// alone decides the kind of launch.
TEST_CASE("resuming a saved game asks for almost nothing")
{
	Spawner::Config config = Read(_Resume, sizeof(_Resume) - 1);

	CHECK_MESSAGE(config.Launch_Type() == LaunchType::Resume, "resuming a save is what the file asks for");
	CHECK_MESSAGE(config.SaveGameName == "SAVEGAME.001", "the saved game is named");
	CHECK_MESSAGE(!config.Firestorm, "the expansion is left out when the file says so");
}


// A saved game is opened by name in the game's own folder, so a name written with a path is
// reduced to the name without ceremony.
TEST_CASE("a saved game is opened by name in the game's own folder")
{
	char const traversal[] =
		"[Settings]\n"
		"LoadSaveGame=Yes\n"
		"SaveGameName=..\\..\\Windows\\SAVEGAME.001\n";
	Spawner::Config config = Read(traversal, sizeof(traversal) - 1);

	CHECK_MESSAGE(config.SaveGameName == "SAVEGAME.001", "only the name of the saved game is read");

	char const forward[] =
		"[Settings]\n"
		"SaveGameName=saves/deep/SAVEGAME.002\n";
	config = Read(forward, sizeof(forward) - 1);

	CHECK_MESSAGE(config.SaveGameName == "SAVEGAME.002", "a forward slash hides nothing either");

	char const drive[] =
		"[Settings]\n"
		"SaveGameName=C:SAVEGAME.003\n";
	config = Read(drive, sizeof(drive) - 1);

	CHECK_MESSAGE(config.SaveGameName == "SAVEGAME.003", "a drive letter is not part of the name");
}


// A campaign takes its houses from the map, so a file naming no player is complete.
TEST_CASE("a campaign takes its houses from the map")
{
	Spawner::Config config = Read(_Campaign, sizeof(_Campaign) - 1);

	CHECK_MESSAGE(config.Launch_Type() == LaunchType::Campaign, "a single player game is a campaign");
	CHECK_MESSAGE(config.CampaignDifficulty == 0, "the two difficulties are read apart");
	CHECK_MESSAGE(config.CampaignCDifficulty == 2, "the two difficulties are read apart");
	CHECK_MESSAGE(config.CampaignID == -1, "a mission outside any campaign says so");
}


// The seats end up in the order the houses are created in, which is what everything naming a
// seat by position afterwards means.
TEST_CASE("the seats end up in the order the houses are created in")
{
	Spawner::Config config = Read(_Skirmish, sizeof(_Skirmish) - 1);

	CHECK_MESSAGE(config.Launch_Type() == LaunchType::Skirmish, "one player and computers is a skirmish");
	CHECK_MESSAGE(config.HumanCount == 1, "the only player holds the first seat");
	CHECK_MESSAGE(config.LocalSlot == 0, "the only player holds the first seat");
	CHECK_MESSAGE(config.Slots[0].Occupancy == OccupancyType::Human, "the player is read from the settings themselves");
	CHECK_MESSAGE(config.Slots[0].Name == "Commander", "the player is read from the settings themselves");
	CHECK_MESSAGE(config.Slots[0].Color == 4, "the player is read from the settings themselves");
	CHECK_MESSAGE(config.Slots[0].Country == 1, "the player is read from the settings themselves");
	CHECK_MESSAGE(config.Slots[1].Occupancy == OccupancyType::Computer, "a seat no section claimed is a computer player named by position");
	CHECK_MESSAGE(config.Slots[1].Color == 2, "a seat no section claimed is a computer player named by position");
	CHECK_MESSAGE(config.Slots[1].Country == 0, "a seat no section claimed is a computer player named by position");
	CHECK_MESSAGE(config.Slots[1].Handicap == 2, "a seat no section claimed is a computer player named by position");
	CHECK_MESSAGE(config.Slots[2].Occupancy == OccupancyType::Computer, "the second computer player follows the first");
	CHECK_MESSAGE(config.Slots[2].Color == 7, "the second computer player follows the first");
	CHECK_MESSAGE(config.Slots[3].Occupancy == OccupancyType::Empty, "no more seats are filled than are asked for");
	CHECK_MESSAGE(config.Slots[0].StartingPosition == 3, "a start position is read for the seat that asked");
	CHECK_MESSAGE(config.Slots[1].StartingPosition == -1, "a start position the map cannot hold becomes the game's own choice");
	CHECK_MESSAGE(config.Slots[2].StartingPosition == -1, "a position below the game's own choice is that choice");
	CHECK_MESSAGE(config.Slots[0].Alliances[0] == 1, "an alliance is read as the seat it names");
	CHECK_MESSAGE(config.NextSkirmishAutoSave == 4, "a client's save numbering is shifted to the game's");
}


// The player who wrote the file is not always the first house, and the game has to know which
// seat is his once the sorting is done.
TEST_CASE("the player who wrote the file is not always the first house")
{
	Spawner::Config config = Read(_Network, sizeof(_Network) - 1);

	CHECK_MESSAGE(config.HumanCount == 2, "a written section is what makes a seat a player");
	CHECK_MESSAGE(config.Slots[0].Name == "First", "the players are sorted into the order their houses are created in");
	CHECK_MESSAGE(config.Slots[1].Name == "Second", "the players are sorted into the order their houses are created in");
	CHECK_MESSAGE(config.LocalSlot == 1, "this machine knows which of the seats is its own");
	CHECK_MESSAGE(config.Slots[0].Address == "10.0.0.7", "the other machine is read with the address it answers on");
	CHECK_MESSAGE(config.Slots[0].Port == 50001, "the other machine is read with the address it answers on");
	CHECK_MESSAGE(config.TunnelId == 50000, "one written port both listens and names this machine to a tunnel");
	CHECK_MESSAGE(config.ListenPort == 50000, "one written port both listens and names this machine to a tunnel");
	CHECK_MESSAGE(config.TunnelAddress == "88.99.11.22", "the tunnel is read from its own section");
	CHECK_MESSAGE(config.TunnelPort == 50010, "the tunnel is read from its own section");
	CHECK_MESSAGE(config.Launch_Type() == LaunchType::Multiplayer, "two players is a game against another machine");
	CHECK_MESSAGE(config.Slots[0].IsSpectator, "a watcher is named by the seat order the houses take");
	CHECK_MESSAGE(!config.Slots[1].IsSpectator, "a watcher is named by the seat order the houses take");
	CHECK_MESSAGE(config.Slots[1].Alliances[0] == 0, "an alliance section names the sorted seat too");
}


// Every machine writes its own file with itself first, so a color tie must be broken by what
// the seats say rather than by file order.
TEST_CASE("every machine writes its own file with itself first")
{
	char const view_a[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"\n"
		"[Other1]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=3\n"
		"Ip=10.0.0.9\n"
		"Port=50002\n";
	char const view_b[] =
		"[Settings]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=3\n"
		"\n"
		"[Other1]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"Ip=10.0.0.8\n"
		"Port=50001\n";

	Spawner::Config a = Read(view_a, sizeof(view_a) - 1);
	Spawner::Config b = Read(view_b, sizeof(view_b) - 1);
	CHECK_MESSAGE(a.Slots[0].Name == "Alpha", "a color tie seats the match identically on every machine");
	CHECK_MESSAGE(b.Slots[0].Name == "Alpha", "a color tie seats the match identically on every machine");
	CHECK_MESSAGE(a.LocalSlot == 0, "each machine still knows which of the tied seats is its own");
	CHECK_MESSAGE(b.LocalSlot == 1, "each machine still knows which of the tied seats is its own");
	CHECK_MESSAGE(Game_Bytes(Play_Of(a)) == Game_Bytes(Play_Of(b)), "the tied match gives one game, and one identity, on both machines");
}


/*
 * The session identity is a CRC of the game a file gives, so two machines handed the same match
 * get the same identity, and a value that only one machine has or that is only displayed cannot
 * change it.
 */
TEST_CASE("two machines handed the same match give the same game")
{
	Spawner::Config one = Read(_Skirmish, sizeof(_Skirmish) - 1);
	Spawner::Config two = Read(_Skirmish, sizeof(_Skirmish) - 1);

	CHECK_MESSAGE(Game_Bytes(Play_Of(one)) == Game_Bytes(Play_Of(two)), "the same match gives the same game twice");

	two.ScenarioName = "another.map";
	two.MapName = "A Map By Another Name";
	two.DifficultyName = "Gentle";
	two.SkipScoreScreen = !two.SkipScoreScreen;
	two.CustomLoadScreen = "Resources/l600s02.pcx";
	CHECK_MESSAGE(Game_Bytes(Play_Of(one)) == Game_Bytes(Play_Of(two)), "the map file and what a player is shown are left out of the game");

	two.Credits = one.Credits + 1;
	CHECK_MESSAGE(Game_Bytes(Play_Of(one)) != Game_Bytes(Play_Of(two)), "a value the match is played by changes the game");

	Spawner::Config named = Read(_Skirmish, sizeof(_Skirmish) - 1);
	named.Slots[0].Name = "Somebody Else";
	CHECK_MESSAGE(Game_Bytes(Play_Of(one)) != Game_Bytes(Play_Of(named)), "a player's name is part of the game");

	Spawner::Config anonymous = Read(_Skirmish, sizeof(_Skirmish) - 1);
	anonymous.QuickMatch = !anonymous.QuickMatch;
	CHECK_MESSAGE(Game_Bytes(Play_Of(one)) != Game_Bytes(Play_Of(anonymous)), "showing players by number is part of the game");

	Spawner::Config three = Read(_Skirmish, sizeof(_Skirmish) - 1);
	three.Slots[1].Country = one.Slots[1].Country + 1;
	CHECK_MESSAGE(Game_Bytes(Play_Of(one)) != Game_Bytes(Play_Of(three)), "a computer player's country changes the game");

	Spawner::Config surrender = Read(_Skirmish, sizeof(_Skirmish) - 1);
	surrender.AutoSurrender = !surrender.AutoSurrender;
	CHECK_MESSAGE(Game_Bytes(Play_Of(one)) != Game_Bytes(Play_Of(surrender)), "what becomes of a departing player's base changes the game");

	Spawner::Config ally = Read(_Skirmish, sizeof(_Skirmish) - 1);
	ally.BuildOffAlly = !ally.BuildOffAlly;
	CHECK_MESSAGE(Game_Bytes(Play_Of(one)) != Game_Bytes(Play_Of(ally)), "building next to an ally changes the game");

	Spawner::Config deploy = Read(_Skirmish, sizeof(_Skirmish) - 1);
	deploy.AutoDeployMCV = !deploy.AutoDeployMCV;
	CHECK_MESSAGE(Game_Bytes(Play_Of(one)) != Game_Bytes(Play_Of(deploy)), "deploying the starting base unit changes the game");

	Spawner::Config neutrals = Read(_Skirmish, sizeof(_Skirmish) - 1);
	neutrals.AttackNeutralUnits = !neutrals.AttackNeutralUnits;
	CHECK_MESSAGE(Game_Bytes(Play_Of(one)) != Game_Bytes(Play_Of(neutrals)), "scanning a neutral house changes the game");

	Spawner::Config waits = Read(_Skirmish, sizeof(_Skirmish) - 1);
	waits.ConnTimeout = one.ConnTimeout + 60;
	waits.ReconnectTimeout = one.ReconnectTimeout + 60;
	CHECK_MESSAGE(Game_Bytes(Play_Of(one)) == Game_Bytes(Play_Of(waits)), "how long a machine waits is its own, so it is left out of the game");

	Spawner::Config ten = Read(_Skirmish, sizeof(_Skirmish) - 1);
	ten.CoachMode = !ten.CoachMode;
	CHECK_MESSAGE(Game_Bytes(Play_Of(one)) != Game_Bytes(Play_Of(ten)), "coach mode changes the game");

	Spawner::Config eleven = Read(_Skirmish, sizeof(_Skirmish) - 1);
	eleven.PlayMoviesInMultiplayer = !eleven.PlayMoviesInMultiplayer;
	CHECK_MESSAGE(Game_Bytes(Play_Of(one)) != Game_Bytes(Play_Of(eleven)), "whether movies play changes the game");

	Spawner::Config campaign = Read(_Campaign, sizeof(_Campaign) - 1);
	Spawner::Config flagged = Read(_Campaign, sizeof(_Campaign) - 1);
	flagged.GlobalFlags[49] = !flagged.GlobalFlags[49];
	CHECK_MESSAGE(Game_Bytes(Play_Of(campaign)) != Game_Bytes(Play_Of(flagged)), "a campaign's scenario flag changes the game");

	/*
	 * Where the machines reach one another carries the match rather than shaping it, and each
	 * machine writes its own view, so it is left out as well.
	 */
	Spawner::Config eight = Read(_Network, sizeof(_Network) - 1);
	Spawner::Config nine = Read(_Network, sizeof(_Network) - 1);
	nine.TunnelAddress = "203.0.113.9";
	nine.TunnelPort = 50010;
	nine.ListenPort = 60000;
	nine.Slots[0].Address = "10.0.0.8";
	nine.Slots[0].Port = 50003;
	nine.IsHost = !nine.IsHost;
	CHECK_MESSAGE(Game_Bytes(Play_Of(eight)) == Game_Bytes(Play_Of(nine)), "where the machines reach one another is left out of the game");
}


// The timing the machines keep is the game's own, and a key the game does not know is passed
// over, so neither can change the game a file gives.
TEST_CASE("the timing the machines keep is the game's own")
{
	char const plain[] =
		"[Settings]\n"
		"Credits=7000\n"
		"Seed=42\n";
	char const noisy[] =
		"[Settings]\n"
		"Credits=7000\n"
		"Seed=42\n"
		"Protocol=2\n"
		"FrameSendRate=3\n"
		"MaxAhead=100\n"
		"PreCalcMaxAhead=1\n"
		"MaxLatencyLevel=2\n"
		"SomeFutureClientKey=1\n";

	Spawner::Config a = Read(plain, sizeof(plain) - 1);
	Spawner::Config b = Read(noisy, sizeof(noisy) - 1);
	CHECK_MESSAGE(Game_Bytes(Play_Of(a)) == Game_Bytes(Play_Of(b)), "timing and unknown keys cannot change the game");
}


// The scenario flags are named by their number, and a load screen position is taken whole or
// not at all.
TEST_CASE("the scenario flags are named by their number")
{
	char const flags[] =
		"[Settings]\n"
		"CustomLoadScreenPos=317,401\n"
		"\n"
		"[GlobalFlags]\n"
		"GlobalFlag0=yes\n"
		"GlobalFlag49=yes\n";
	Spawner::Config config = Read(flags, sizeof(flags) - 1);

	CHECK_MESSAGE(config.GlobalFlags[0], "a scenario flag is read by its number");
	CHECK_MESSAGE(config.GlobalFlags[49], "a scenario flag is read by its number");

	bool between = false;
	for (int index = 1; index < 49; index++) {
		between = between || config.GlobalFlags[index];
	}
	CHECK_MESSAGE(!between, "no flag is set by a neighbor's spelling");
	CHECK_MESSAGE(config.CustomLoadScreenX == 317, "the load screen position is read whole");
	CHECK_MESSAGE(config.CustomLoadScreenY == 401, "the load screen position is read whole");

	char const malformed[] =
		"[Settings]\n"
		"CustomLoadScreenPos=oops\n";
	config = Read(malformed, sizeof(malformed) - 1);

	CHECK_MESSAGE(config.CustomLoadScreenX == 0, "a position the reader cannot make sense of is no position");
	CHECK_MESSAGE(config.CustomLoadScreenY == 0, "a position the reader cannot make sense of is no position");

	char const half[] =
		"[Settings]\n"
		"CustomLoadScreenPos=12\n";
	config = Read(half, sizeof(half) - 1);

	CHECK_MESSAGE(config.CustomLoadScreenX == 0, "half a position is no position either");
	CHECK_MESSAGE(config.CustomLoadScreenY == 0, "half a position is no position either");
}


// What a player is shown is taken as the client wrote it, and an unwritten key leaves the game
// showing what it would have shown by itself.
TEST_CASE("what a player is shown is taken as the client wrote it")
{
	char const shown[] =
		"[Settings]\n"
		"SkipScoreScreen=Yes\n"
		"CustomLoadScreen=Resources/l600s01.pcx\n"
		"DifficultyName=Gentle\n"
		"QuickMatch=Yes\n";
	Spawner::Config config = Read(shown, sizeof(shown) - 1);

	CHECK_MESSAGE(config.SkipScoreScreen, "a match may ask that its score screen be passed over");
	CHECK_MESSAGE(config.CustomLoadScreen == "Resources/l600s01.pcx", "the load screen is named exactly as the client wrote it");
	CHECK_MESSAGE(config.DifficultyName == "Gentle", "a campaign may be played under a difficulty name of its own");
	CHECK_MESSAGE(config.QuickMatch, "a match may ask that its players be shown by number");

	char const silent[] = "[Settings]\n";
	config = Read(silent, sizeof(silent) - 1);

	CHECK_MESSAGE(!config.SkipScoreScreen, "an unwritten key still shows the score screen");
	CHECK_MESSAGE(!config.QuickMatch, "and still shows each player by name");
	CHECK_MESSAGE(config.CustomLoadScreen.empty(), "the game's own load screen and difficulty names stand unasked");
	CHECK_MESSAGE(config.DifficultyName.empty(), "the game's own load screen and difficulty names stand unasked");
}


// The results file is written only on request, and what it reports about the match is taken as
// the client wrote it without making the match a different one.
TEST_CASE("the results file is written only on request")
{
	char const results[] =
		"[Settings]\n"
		"WriteStatistics=Yes\n"
		"GameID=-123456789\n"
		"MapHash=0123456789abcdef0123456789abcdef01234567\n"
		"UIMapName=Tiny Tunnels\n";
	Spawner::Config config = Read(results, sizeof(results) - 1);

	CHECK_MESSAGE(config.WriteStatistics, "a client may ask for the results file");
	CHECK_MESSAGE(config.GameID == -123456789, "the match number is read as the client wrote it");
	CHECK_MESSAGE(config.MapHash == "0123456789abcdef0123456789abcdef01234567", "the map hash is read as the client wrote it");

	char const silent[] = "[Settings]\n";
	Spawner::Config plain = Read(silent, sizeof(silent) - 1);
	CHECK_MESSAGE(!plain.WriteStatistics, "an unwritten key writes no results file");
	CHECK_MESSAGE(plain.GameID == 0, "and reports no match number or map hash");
	CHECK_MESSAGE(plain.MapHash.empty(), "and reports no match number or map hash");

	plain.WriteStatistics = config.WriteStatistics;
	plain.GameID = config.GameID;
	plain.MapHash = config.MapHash;
	Spawner::Config bare = Read(silent, sizeof(silent) - 1);
	CHECK_MESSAGE(Game_Bytes(Play_Of(plain)) == Game_Bytes(Play_Of(bare)), "what the results file reports is left out of the game");
}


// Reading a launch file cannot fail, so whether what it describes can be played is judged
// separately, against the tables the game has loaded by the time it launches.
TEST_CASE("reading a launch file cannot fail")
{
	std::string fault;

	CHECK_MESSAGE(Judge(_Skirmish, sizeof(_Skirmish) - 1, 2, 8, fault), "a match the loaded rules can hold is played");

	char const crowded[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"AIPlayers=8\n";
	CHECK_MESSAGE(!Judge(crowded, sizeof(crowded) - 1, 2, 8, fault), "more computer players than seats names both counts");
	CHECK_MESSAGE(fault.find("8") != std::string::npos, "more computer players than seats names both counts");
	CHECK_MESSAGE(fault.find("7") != std::string::npos, "more computer players than seats names both counts");

	char const negative[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"AIPlayers=-1\n";
	CHECK_MESSAGE(!Judge(negative, sizeof(negative) - 1, 2, 8, fault), "fewer than no computer players is refused");

	char const nameless_country[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Color=0\n";
	CHECK_MESSAGE(Judge(nameless_country, sizeof(nameless_country) - 1, 2, 8, fault), "a person who names no country is given one");

	char const nameless_color[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n";
	CHECK_MESSAGE(Judge(nameless_color, sizeof(nameless_color) - 1, 2, 8, fault), "a person who names no color is given one");

	char const drawn_computer[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"AIPlayers=1\n"
		"\n"
		"[HouseColors]\n"
		"Multi2=-1\n"
		"\n"
		"[HouseCountries]\n"
		"Multi2=-1\n";
	CHECK_MESSAGE(Judge(drawn_computer, sizeof(drawn_computer) - 1, 2, 8, fault), "a computer seat may leave its country and color to the game");

	char const past_countries[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=2\n"
		"Color=0\n";
	CHECK_MESSAGE(!Judge(past_countries, sizeof(past_countries) - 1, 2, 8, fault), "a country the rules did not declare is refused under the key that set it");
	CHECK_MESSAGE(fault.find("SPAWN.INI [Settings] Side: ") != std::string::npos, "a country the rules did not declare is refused under the key that set it");

	char const sorted_past_countries[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=9\n"
		"Color=5\n"
		"\n"
		"[Other1]\n"
		"Name=Bravo\n"
		"Side=0\n"
		"Color=0\n"
		"Ip=10.0.0.9\n"
		"Port=50002\n";
	CHECK_MESSAGE(!Judge(sorted_past_countries, sizeof(sorted_past_countries) - 1, 2, 8, fault), "a refusal names the person's own section after house order has moved them");
	CHECK_MESSAGE(fault.find("SPAWN.INI [Settings] Side: ") != std::string::npos, "a refusal names the person's own section after house order has moved them");

	char const past_colors[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=8\n";
	CHECK_MESSAGE(!Judge(past_colors, sizeof(past_colors) - 1, 2, 8, fault), "a color the game has no scheme for is refused");

	char const shared_color[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"\n"
		"[Other1]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=3\n"
		"Ip=10.0.0.9\n"
		"Port=50002\n";
	CHECK_MESSAGE(!Judge(shared_color, sizeof(shared_color) - 1, 2, 8, fault), "two people of one color are refused against other machines");

	char const shared_with_computer[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=3\n"
		"AIPlayers=1\n"
		"\n"
		"[HouseColors]\n"
		"Multi2=3\n";
	CHECK_MESSAGE(Judge(shared_with_computer, sizeof(shared_with_computer) - 1, 2, 8, fault), "a computer player may take the color its opponent plays");

	char const past_difficulty[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"AIPlayers=1\n"
		"\n"
		"[HouseHandicaps]\n"
		"Multi2=7\n";
	CHECK_MESSAGE(!Judge(past_difficulty, sizeof(past_difficulty) - 1, 2, 8, fault), "a difficulty naming none is refused under the entry that set it");
	CHECK_MESSAGE(fault.find("SPAWN.INI [HouseHandicaps] Multi2: 7") != std::string::npos, "a difficulty naming none is refused under the entry that set it");

	char const easy_difficulty[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"AIPlayers=1\n"
		"\n"
		"[HouseHandicaps]\n"
		"Multi2=6\n";
	CHECK_MESSAGE(Judge(easy_difficulty, sizeof(easy_difficulty) - 1, 2, 8, fault), "a difficulty easier than the game holds is played, not refused");

	CHECK_MESSAGE(Spawner::Config::Playable_Handicap(-1) == -1, "an easier opponent than the game has comes to the easiest opponent it has");
	CHECK_MESSAGE(Spawner::Config::Playable_Handicap(0) == 0, "an easier opponent than the game has comes to the easiest opponent it has");
	CHECK_MESSAGE(Spawner::Config::Playable_Handicap(2) == 2, "an easier opponent than the game has comes to the easiest opponent it has");
	CHECK_MESSAGE(Spawner::Config::Playable_Handicap(3) == 2, "an easier opponent than the game has comes to the easiest opponent it has");
	CHECK_MESSAGE(Spawner::Config::Playable_Handicap(6) == 2, "an easier opponent than the game has comes to the easiest opponent it has");

	char const two_machines[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"\n"
		"[Other1]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=5\n"
		"Ip=10.0.0.9\n"
		"Port=50002\n";
	CHECK_MESSAGE(Judge(two_machines, sizeof(two_machines) - 1, 2, 8, fault), "a match against another machine with everybody named is played");

	char const nameless_machine[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"\n"
		"[Other1]\n"
		"Side=1\n"
		"Color=5\n"
		"Ip=10.0.0.9\n"
		"Port=50002\n";
	CHECK_MESSAGE(!Judge(nameless_machine, sizeof(nameless_machine) - 1, 2, 8, fault), "a person the file leaves unnamed is refused against other machines");

	char const one_name[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"\n"
		"[Other1]\n"
		"Name=alpha\n"
		"Side=1\n"
		"Color=5\n"
		"Ip=10.0.0.9\n"
		"Port=50002\n";
	CHECK_MESSAGE(!Judge(one_name, sizeof(one_name) - 1, 2, 8, fault), "two people under one name are refused however either is spelled");
	CHECK_MESSAGE(fault.find("SPAWN.INI [Other1] Name: ") == 0, fault);

	char const alone[] =
		"[Settings]\n"
		"Side=0\n"
		"Color=0\n";
	CHECK_MESSAGE(Judge(alone, sizeof(alone) - 1, 2, 8, fault), "somebody playing alone need not be named");

	char const nobody[] =
		"[GlobalFlags]\n"
		"GlobalFlag0=yes\n";
	CHECK_MESSAGE(!Judge(nobody, sizeof(nobody) - 1, 2, 8, fault), "a file seating nobody at this machine is refused");

	char const past_ai_difficulty[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"AIDifficulty=7\n";
	CHECK_MESSAGE(!Judge(past_ai_difficulty, sizeof(past_ai_difficulty) - 1, 2, 8, fault), "a computer difficulty the game does not have is refused");

	char const unreachable[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"\n"
		"[Other1]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=5\n"
		"Ip=10.0.0.9\n";
	CHECK_MESSAGE(!Judge(unreachable, sizeof(unreachable) - 1, 2, 8, fault), "a machine the file gives no port is refused");

	char const nowhere[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"\n"
		"[Other1]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=5\n"
		"Ip=10.0.0.\n"
		"Port=50002\n";
	CHECK_MESSAGE(!Judge(nowhere, sizeof(nowhere) - 1, 2, 8, fault), "an address naming no machine is refused");

	char const tunnelled[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"Port=50000\n"
		"\n"
		"[Other1]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=5\n"
		"Port=50002\n"
		"\n"
		"[Tunnel]\n"
		"Ip=88.99.11.22\n"
		"Port=50010\n";
	CHECK_MESSAGE(Judge(tunnelled, sizeof(tunnelled) - 1, 2, 8, fault), "a tunnelled machine is named by its number rather than an address");

	char const client_tunnel[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"Port=-14\n"
		"\n"
		"[Other1]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=5\n"
		"Ip=0.0.0.0\n"
		"Port=-3695\n"
		"\n"
		"[Tunnel]\n"
		"Ip=88.99.11.22\n"
		"Port=50000\n";
	CHECK_MESSAGE(Judge(client_tunnel, sizeof(client_tunnel) - 1, 2, 8, fault), "the client's negative tunnel numbers are accepted");

	char const zero_tunnel_number[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"Port=-14\n"
		"\n"
		"[Other1]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=5\n"
		"Port=0\n"
		"\n"
		"[Tunnel]\n"
		"Ip=88.99.11.22\n"
		"Port=50000\n";
	CHECK_MESSAGE(!Judge(zero_tunnel_number, sizeof(zero_tunnel_number) - 1, 2, 8, fault), "a seat the tunnel knows as zero is refused");

	char const portless_tunnel[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"Port=-14\n"
		"\n"
		"[Other1]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=5\n"
		"\n"
		"[Tunnel]\n"
		"Ip=88.99.11.22\n"
		"Port=50000\n";
	CHECK_MESSAGE(!Judge(portless_tunnel, sizeof(portless_tunnel) - 1, 2, 8, fault), "a seat the tunnel is given no number for is refused");

	/*
	 * A client that runs the tunnel beside the game points the match at the loopback
	 * address, so that is an address like any other.
	 */
	char const bridged[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"Port=48000\n"
		"\n"
		"[Other1]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=5\n"
		"Port=47999\n"
		"\n"
		"[Tunnel]\n"
		"Ip=127.0.0.1\n"
		"Port=48000\n";
	CHECK_MESSAGE(Judge(bridged, sizeof(bridged) - 1, 2, 8, fault), "a tunnel beside the game is reached like any other");

	char const strange_tunnel[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"Port=50000\n"
		"\n"
		"[Other1]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=5\n"
		"Port=50002\n"
		"\n"
		"[Tunnel]\n"
		"Ip=88.99.11\n"
		"Port=50010\n";
	CHECK_MESSAGE(!Judge(strange_tunnel, sizeof(strange_tunnel) - 1, 2, 8, fault), "a tunnel at an address naming no machine is refused");

	char const wide_tunnel_port[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"Port=50000\n"
		"\n"
		"[Other1]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=5\n"
		"Port=50002\n"
		"\n"
		"[Tunnel]\n"
		"Ip=88.99.11.22\n"
		"Port=65536\n";
	CHECK_MESSAGE(!Judge(wide_tunnel_port, sizeof(wide_tunnel_port) - 1, 2, 8, fault), "a tunnel port too wide to carry is refused rather than cut down");

	char const wide_tunnel_number[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"Port=70000\n"
		"\n"
		"[Other1]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=5\n"
		"Port=50002\n"
		"\n"
		"[Tunnel]\n"
		"Ip=88.99.11.22\n"
		"Port=50010\n";
	CHECK_MESSAGE(!Judge(wide_tunnel_number, sizeof(wide_tunnel_number) - 1, 2, 8, fault), "a tunnel number too wide to carry is refused rather than cut down");

	char const wide_listen_port[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"Port=70000\n"
		"\n"
		"[Other1]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=5\n"
		"Ip=10.0.0.9\n"
		"Port=50002\n";
	CHECK_MESSAGE(!Judge(wide_listen_port, sizeof(wide_listen_port) - 1, 2, 8, fault), "a listening port too wide to bind is refused rather than cut down");

	char const one_kept_name[] =
		"[Settings]\n"
		"Name=CommanderAlphaOmegaCommanderAlphaOmegaCommanderAlphaOmegaCommanX\n"
		"Side=0\n"
		"Color=3\n"
		"\n"
		"[Other1]\n"
		"Name=CommanderAlphaOmegaCommanderAlphaOmegaCommanderAlphaOmegaCommanY\n"
		"Side=1\n"
		"Color=5\n"
		"Ip=10.0.0.9\n"
		"Port=50002\n";
	CHECK_MESSAGE(!Judge(one_kept_name, sizeof(one_kept_name) - 1, 2, 8, fault), "two names the game keeps as one are refused");

	char const unheld_ally[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"\n"
		"[Multi1_Alliances]\n"
		"HouseAllyOne=5\n";
	CHECK_MESSAGE(!Judge(unheld_ally, sizeof(unheld_ally) - 1, 2, 8, fault), "an alliance with a seat nobody occupies is refused");

	char const past_seats[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"\n"
		"[Multi1_Alliances]\n"
		"HouseAllyOne=8\n";
	CHECK_MESSAGE(!Judge(past_seats, sizeof(past_seats) - 1, 2, 8, fault), "an alliance with a seat the match does not hold is refused");

	char const watcher[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"AIPlayers=1\n"
		"\n"
		"[IsSpectator]\n"
		"Multi1=Yes\n";
	CHECK_MESSAGE(Judge(watcher, sizeof(watcher) - 1, 2, 8, fault), "a seat that watches a computer match is accepted");

	CHECK_MESSAGE(Judge(_Network, sizeof(_Network) - 1, 2, 8, fault), "a seat that watches another machine play is accepted");

	char const watchers_alone[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"\n"
		"[IsSpectator]\n"
		"Multi1=Yes\n";
	CHECK_MESSAGE(Judge(watchers_alone, sizeof(watchers_alone) - 1, 2, 8, fault), "a match of watchers alone is accepted");

	CHECK_MESSAGE(!Judge(_Skirmish, sizeof(_Skirmish) - 1, 0, 8, fault), "a match is refused rather than read against countries the rules never declared");
}


namespace {

Launch::Message::Option const * Option_Of(Launch::Message::Game const & launch, char const * key)
{
	for (Launch::Message::Option const & option : launch.Options) {
		if (option.Key == key) {
			return(&option);
		}
	}
	return(nullptr);
}

}


// Each machine writes its own file with itself first, and the launch both files give is the
// same to the byte; only the local player and the peers differ.
TEST_CASE("every machine's file for one match gives the same launch")
{
	char const view_a[] =
		"[Settings]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"Port=50001\n"
		"Seed=77\n"
		"AIPlayers=1\n"
		"Credits=7000\n"
		"Host=Yes\n"
		"\n"
		"[Other1]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=5\n"
		"Ip=10.0.0.9\n"
		"Port=50002\n"
		"\n"
		"[HouseColors]\n"
		"Multi3=6\n"
		"\n"
		"[HouseCountries]\n"
		"Multi3=1\n"
		"\n"
		"[HouseHandicaps]\n"
		"Multi3=1\n"
		"\n"
		"[SpawnLocations]\n"
		"Multi1=2\n"
		"Multi2=0\n"
		"Multi3=1\n"
		"\n"
		"[Multi1_Alliances]\n"
		"HouseAllyOne=2\n"
		"\n"
		"[Multi3_Alliances]\n"
		"HouseAllyOne=0\n";
	char const view_b[] =
		"[Settings]\n"
		"Name=Bravo\n"
		"Side=1\n"
		"Color=5\n"
		"Port=50002\n"
		"Seed=77\n"
		"AIPlayers=1\n"
		"Credits=7000\n"
		"\n"
		"[Other1]\n"
		"Name=Alpha\n"
		"Side=0\n"
		"Color=3\n"
		"Ip=10.0.0.8\n"
		"Port=50001\n"
		"\n"
		"[HouseColors]\n"
		"Multi3=6\n"
		"\n"
		"[HouseCountries]\n"
		"Multi3=1\n"
		"\n"
		"[HouseHandicaps]\n"
		"Multi3=1\n"
		"\n"
		"[SpawnLocations]\n"
		"Multi1=2\n"
		"Multi2=0\n"
		"Multi3=1\n"
		"\n"
		"[Multi1_Alliances]\n"
		"HouseAllyOne=2\n"
		"\n"
		"[Multi3_Alliances]\n"
		"HouseAllyOne=0\n";

	Spawner::Config a = Read(view_a, sizeof(view_a) - 1);
	Spawner::Config b = Read(view_b, sizeof(view_b) - 1);
	Launch::Message::Play play_a = Play_Of(a);
	Launch::Message::Play play_b = Play_Of(b);

	CHECK_MESSAGE(Game_Bytes(play_a) == Game_Bytes(play_b), "both machines give the same launch, byte for byte");
	CHECK_MESSAGE(play_a.Game.Kind == Launch::GameKind::GAME_KIND_MULTIPLAYER, "two people make a network match");
	CHECK_MESSAGE(play_a.Game.Seed == std::optional<std::uint32_t>(77), "the file's seed is the match's");

	REQUIRE(play_a.Game.Players.size() == 3);
	CHECK_MESSAGE(play_a.Game.Players[0].Name == "Alpha", "the people come first, by color");
	CHECK_MESSAGE(play_a.Game.Players[1].Name == "Bravo", "the people come first, by color");
	CHECK_MESSAGE(!play_a.Game.Players[2].Human, "the computer player follows the people");
	CHECK_MESSAGE(play_a.Game.Players[2].Color == 6, "the computer player keeps the color its file position asks for");
	CHECK_MESSAGE(play_a.Game.Players[2].AILevel == std::optional<std::int32_t>(1), "the computer player keeps its difficulty");
	CHECK_MESSAGE(play_a.Game.Players[0].Start == 2, "a start position stays with the sorted slot");
	CHECK_MESSAGE(play_a.Game.Players[0].Allies == std::vector<std::uint32_t>{ 2 }, "an alliance names the sorted slot");
	CHECK_MESSAGE(play_a.Game.Players[2].Allies == std::vector<std::uint32_t>{ 0 }, "an alliance names the sorted slot");

	CHECK_MESSAGE(play_a.Session.LocalPlayer == std::optional<std::uint32_t>(0), "each machine knows its own player");
	CHECK_MESSAGE(play_b.Session.LocalPlayer == std::optional<std::uint32_t>(1), "each machine knows its own player");
	CHECK_MESSAGE(play_a.Session.Host, "the host is this machine's own setting");
	CHECK_MESSAGE(!play_b.Session.Host, "the host is this machine's own setting");

	REQUIRE(play_a.Network.has_value());
	REQUIRE(play_b.Network.has_value());
	CHECK_MESSAGE(play_a.Network->Port == 50001, "without a tunnel the port is the one this machine listens on");
	CHECK_MESSAGE(!play_a.Network->Tunnel.has_value(), "no tunnel is named when the file gives none");
	REQUIRE(play_a.Network->Peers.size() == 1);
	CHECK_MESSAGE(play_a.Network->Peers[0].Player == 1, "the peer is the other person, by position");
	CHECK_MESSAGE(play_a.Network->Peers[0].Address == "10.0.0.9", "the peer is reached at the address the file gives");
	CHECK_MESSAGE(play_a.Network->Peers[0].Port == 50002, "the peer is reached at the address the file gives");
	REQUIRE(play_b.Network->Peers.size() == 1);
	CHECK_MESSAGE(play_b.Network->Peers[0].Player == 0, "the peer is the other person, by position");

	std::string fault;
	CHECK_MESSAGE(Request_Is_Taken(a, 2, 8, 1, fault), fault);
	CHECK_MESSAGE(Request_Is_Taken(b, 2, 8, 1, fault), fault);
}


// A skirmish lists the players in the order their houses are created in, with the reader's
// repairs, and a computer player's difficulty is the one the game plays it at.
TEST_CASE("a launch lists the players the game plays")
{
	Spawner::Config config = Read(_Skirmish, sizeof(_Skirmish) - 1);
	Launch::Message::Play play = Play_Of(config);
	Launch::Message::Game const & launch = play.Game;

	CHECK_MESSAGE(launch.Kind == Launch::GameKind::GAME_KIND_SKIRMISH, "one person and computers is a skirmish");
	CHECK_MESSAGE(!play.Network.has_value(), "a skirmish reaches no other machine");
	CHECK_MESSAGE(play.MapPath == "spawnmap.ini", "the map is the scenario the file names");
	CHECK_MESSAGE(play.Session.NextSkirmishAutosave == 4, "the save numbering is the game's");

	REQUIRE(launch.Players.size() == 3);
	CHECK_MESSAGE(launch.Players[0].Human, "the person comes first");
	CHECK_MESSAGE(launch.Players[0].Name == "Commander", "the person keeps the name the file gives");
	CHECK_MESSAGE(launch.Players[0].Country == 1, "the person keeps the country and color the file gives");
	CHECK_MESSAGE(launch.Players[0].Color == 4, "the person keeps the country and color the file gives");
	CHECK_MESSAGE(launch.Players[0].Start == 3, "a start position the map holds is kept");
	CHECK_MESSAGE(launch.Players[1].Start == -1, "a start position the map cannot hold is the game's choice");
	CHECK_MESSAGE(launch.Players[2].Start == -1, "a start position below the game's choice is that choice");
	CHECK_MESSAGE(!launch.Players[0].AILevel.has_value(), "a person has no computer difficulty");
	CHECK_MESSAGE(launch.Players[1].AILevel == std::optional<std::int32_t>(2), "a computer player keeps its difficulty");
	CHECK_MESSAGE(launch.Players[2].AILevel == std::optional<std::int32_t>(0), "a computer player keeps its difficulty");
	CHECK_MESSAGE(launch.Players[1].Name.empty(), "a computer player's name is left to the game unless the file asks");
	CHECK_MESSAGE(launch.Players[0].Allies == std::vector<std::uint32_t>{ 1 }, "an alliance is kept");

	Launch::Message::Option const * credits = Option_Of(launch, "Credits");
	REQUIRE(credits != nullptr);
	CHECK_MESSAGE(std::get<std::int64_t>(credits->Value) == 5000, "an option is written as the file gives it");
	Launch::Message::Option const * shortgame = Option_Of(launch, "ShortGame");
	REQUIRE(shortgame != nullptr);
	CHECK_MESSAGE(std::get<bool>(shortgame->Value), "an option is written as the file gives it");
	CHECK_MESSAGE(Option_Of(launch, "Campaign") == nullptr, "a campaign option is left out of a skirmish");

	char const named[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"AIPlayers=3\n"
		"AIDifficulty=0\n"
		"DifficultyBasedAINames=Yes\n"
		"\n"
		"[HouseHandicaps]\n"
		"Multi2=6\n"
		"Multi3=1\n";
	Launch::Message::Play by_difficulty = Play_Of(Read(named, sizeof(named) - 1));
	REQUIRE(by_difficulty.Game.Players.size() == 4);
	CHECK_MESSAGE(by_difficulty.Game.Players[1].Name == "Easy AI", "an easier difficulty than the game has is named for the easiest");
	CHECK_MESSAGE(by_difficulty.Game.Players[1].AILevel == std::optional<std::int32_t>(2), "an easier difficulty than the game has is played as the easiest");
	CHECK_MESSAGE(by_difficulty.Game.Players[2].Name == "Medium AI", "a computer player is named for its difficulty");
	CHECK_MESSAGE(by_difficulty.Game.Players[3].Name == "Easy AI", "a computer player without a difficulty is named for AIDifficulty, numbered the other way");
	CHECK_MESSAGE(!by_difficulty.Game.Players[3].AILevel.has_value(), "a computer player without a difficulty plays at AIDifficulty");

	char const nameless[] =
		"[Settings]\n"
		"Side=0\n"
		"Color=0\n";
	CHECK_MESSAGE(Play_Of(Read(nameless, sizeof(nameless) - 1)).Game.Players[0].Name == "Player", "a person the file leaves unnamed is shown as Player");
}


// The game ignores an alliance with the player itself, with an observer, and every alliance an
// observer lists, so the launch leaves them out and lists the rest once each, ascending.
TEST_CASE("a launch lists only the alliances the game makes")
{
	char const tangled[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"AIPlayers=2\n"
		"\n"
		"[Multi1_Alliances]\n"
		"HouseAllyOne=2\n"
		"HouseAllyTwo=0\n"
		"HouseAllyThree=1\n"
		"HouseAllyFour=2\n";
	Launch::Message::Play play = Play_Of(Read(tangled, sizeof(tangled) - 1));
	CHECK_MESSAGE(play.Game.Players[0].Allies == std::vector<std::uint32_t>({ 1, 2 }), "the alliances are listed once each, ascending, without the player itself");

	Launch::Message::Play network = Play_Of(Read(_Network, sizeof(_Network) - 1));
	REQUIRE(network.Game.Players.size() == 2);
	CHECK_MESSAGE(network.Game.Players[0].Observer, "a watcher is listed as an observer");
	CHECK_MESSAGE(network.Game.Players[1].Allies.empty(), "an alliance with an observer is left out");

	char const watching_ally[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"AIPlayers=1\n"
		"\n"
		"[IsSpectator]\n"
		"Multi1=Yes\n"
		"\n"
		"[Multi1_Alliances]\n"
		"HouseAllyOne=1\n";
	Launch::Message::Play watcher = Play_Of(Read(watching_ally, sizeof(watching_ally) - 1));
	CHECK_MESSAGE(watcher.Game.Players[0].Observer, "a watcher is listed as an observer");
	CHECK_MESSAGE(watcher.Game.Players[0].Allies.empty(), "an observer's own alliances are left out");
}


// A network match reaches the others through the tunnel the file gives, and each wait is
// given in milliseconds that convert back to the same ticks.
TEST_CASE("a network launch reaches the other machines as the file says")
{
	char const waits[] =
		"[Settings]\n"
		"Name=Second\n"
		"Side=1\n"
		"Color=5\n"
		"Port=50000\n"
		"ConnTimeout=1800\n"
		"ReconnectTimeout=1\n"
		"\n"
		"[Other1]\n"
		"Name=First\n"
		"Side=0\n"
		"Color=1\n"
		"Port=-3695\n"
		"\n"
		"[Tunnel]\n"
		"Ip=88.99.11.22\n"
		"Port=50010\n";
	Launch::Message::Play play = Play_Of(Read(waits, sizeof(waits) - 1));

	REQUIRE(play.Network.has_value());
	Launch::Message::Network const & network = *play.Network;
	CHECK_MESSAGE(network.Port == 50000, "through a tunnel the port is this machine's tunnel number");
	REQUIRE(network.Tunnel.has_value());
	CHECK_MESSAGE(network.Tunnel->Address == "88.99.11.22", "the tunnel is the one the file names");
	CHECK_MESSAGE(network.Tunnel->Port == 50010, "the tunnel is the one the file names");
	REQUIRE(network.Peers.size() == 1);
	CHECK_MESSAGE(network.Peers[0].Player == 0, "the other person is reached by position");
	CHECK_MESSAGE(network.Peers[0].Port == -3695, "through a tunnel a peer is reached by its tunnel number");
	CHECK_MESSAGE(network.Peers[0].Address.empty(), "through a tunnel a peer needs no address");

	CHECK_MESSAGE(network.ConnectTimeout == 1800u * 16, "a wait is given in milliseconds");
	CHECK_MESSAGE(Launch::Machine::Timeout_Ticks(network.ConnectTimeout, 0) == 1800, "the milliseconds come back to the ticks the file gives");
	CHECK_MESSAGE(Launch::Machine::Timeout_Ticks(network.ReconnectTimeout, 0) == Spawner::Config::TIMEOUT_MIN, "a wait the reader raised stays raised");
	CHECK_MESSAGE(play.Game.Seed == std::optional<std::uint32_t>(0), "a network match starts from seed 0 when the file gives no seed");
}


// A game played alone draws its seed unless the file gives one other than 0; a network match
// always uses the file's seed, and a negative seed keeps its 32 bits.
TEST_CASE("a launch carries the seed the game would start from")
{
	char const unseeded[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n";
	CHECK_MESSAGE(!Play_Of(Read(unseeded, sizeof(unseeded) - 1)).Game.Seed.has_value(), "a game alone with seed 0 draws one");

	char const seeded[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"Seed=42\n";
	CHECK_MESSAGE(Play_Of(Read(seeded, sizeof(seeded) - 1)).Game.Seed == std::optional<std::uint32_t>(42), "a game alone keeps a seed other than 0");

	Spawner::Config negative = Read(_Network, sizeof(_Network) - 1);
	negative.Seed = -1;
	CHECK_MESSAGE(Play_Of(negative).Game.Seed == std::optional<std::uint32_t>(0xFFFFFFFFu), "a negative seed keeps its bits");
}


// A campaign lists only the local player and the options a mission reads; the file's values
// for this machine go in the session.
TEST_CASE("a campaign launch lists the person playing and the mission's options")
{
	Spawner::Config config = Read(_Campaign, sizeof(_Campaign) - 1);
	Launch::Message::Play play = Play_Of(config);
	Launch::Message::Game const & launch = play.Game;

	CHECK_MESSAGE(launch.Kind == Launch::GameKind::GAME_KIND_CAMPAIGN, "a single player game is a campaign");
	REQUIRE(launch.Players.size() == 1);
	CHECK_MESSAGE(launch.Players[0].Human, "the one player is the person playing");
	CHECK_MESSAGE(launch.Players[0].Name == "Player", "a person the file leaves unnamed is shown as Player");
	CHECK_MESSAGE(play.Session.LocalPlayer == std::optional<std::uint32_t>(0), "the person playing is the first player");
	CHECK_MESSAGE(play.Session.LoadScreen.Image == "Resources/l600s01.pcx", "the load screen is this machine's");

	Launch::Message::Option const * human = Option_Of(launch, "CampaignPlayerDifficulty");
	Launch::Message::Option const * computer = Option_Of(launch, "CampaignComputerDifficulty");
	Launch::Message::Option const * campaign = Option_Of(launch, "Campaign");
	Launch::Message::Option const * firestorm = Option_Of(launch, "Firestorm");
	REQUIRE(human != nullptr);
	REQUIRE(computer != nullptr);
	REQUIRE(campaign != nullptr);
	REQUIRE(firestorm != nullptr);
	CHECK_MESSAGE(std::get<std::int64_t>(human->Value) == 0, "the two difficulties are kept apart");
	CHECK_MESSAGE(std::get<std::int64_t>(computer->Value) == 2, "the two difficulties are kept apart");
	CHECK_MESSAGE(std::get<std::int64_t>(campaign->Value) == -1, "a mission outside any campaign says so");
	CHECK_MESSAGE(!std::get<bool>(firestorm->Value), "the expansion is left out when the file says so");
	CHECK_MESSAGE(Option_Of(launch, "Credits") == nullptr, "a skirmish option is left out of a campaign");

	std::string fault;
	CHECK_MESSAGE(Request_Is_Taken(config, 2, 8, 0, fault), fault);
}


// A resume gives a load request. Only the save tells whether the game was played against other
// machines, so the request lists the file's human players.
TEST_CASE("a resume gives a load request")
{
	Spawner::Config config = Read(_Resume, sizeof(_Resume) - 1);
	Launch::Message::Load load = Load_Of(config);

	CHECK_MESSAGE(load.Save == "SAVEGAME.001", "the save is the one the file names");
	CHECK_MESSAGE(load.GameSpeed == std::optional<std::int32_t>(1), "the file's speed goes with the load");
	CHECK_MESSAGE(load.Session.LoadScreen.Image == "Resources/l600s01.pcx", "what the player is shown goes with the load");
	REQUIRE_MESSAGE(load.Players.size() == 1, "the [Settings] player is listed although the save may not need it");
	CHECK_MESSAGE(load.Players[0].Name == "Player", "an unnamed player is listed under the name the game gives it");
	CHECK_MESSAGE(load.Network.has_value(), "the network goes with the player");

	Launch::Machine machine;
	std::string fault;
	CHECK_MESSAGE(machine.From_Load(load, false, fault), fault);
	CHECK_MESSAGE(machine.MapPath == "SAVEGAME.001", "the machine part keeps the save");
	CHECK_MESSAGE(!machine.Network.has_value(), "a single-player save reads no network");

	char const nameless[] =
		"[Settings]\n"
		"LoadSaveGame=Yes\n";
	CHECK_MESSAGE(!File_Is_Taken(Read(nameless, sizeof(nameless) - 1), 2, 8, 1, fault), "a resume needs a save");
	CHECK_MESSAGE(fault.find("SPAWN.INI [Settings] SaveGameName: ") == 0, fault);
}


// A network resume lists the human players who rejoin, this machine's slot as the local player,
// and how to reach the others, as a network match does.
TEST_CASE("a network resume gives the players and the network")
{
	char const resume[] =
		"[Settings]\n"
		"LoadSaveGame=Yes\n"
		"SaveGameName=SVGM_000.NET\n"
		"Name=Second\n"
		"Side=1\n"
		"Color=5\n"
		"Port=50000\n"
		"Host=Yes\n"
		"GameSpeed=2\n"
		"ConnTimeout=1000\n"
		"\n"
		"[Other1]\n"
		"Name=First\n"
		"Side=0\n"
		"Color=1\n"
		"Ip=10.0.0.7\n"
		"Port=50001\n"
		"\n"
		"[IsSpectator]\n"
		"Multi1=Yes\n";
	Spawner::Config config = Read(resume, sizeof(resume) - 1);
	std::string fault;
	REQUIRE_MESSAGE(File_Is_Taken(config, 2, 8, 1, fault), fault);

	Launch::Message::Load load = Load_Of(config);
	REQUIRE(load.Players.size() == 2);
	REQUIRE(load.Session.LocalPlayer.has_value());
	Launch::Message::Player const & local = load.Players[*load.Session.LocalPlayer];
	CHECK_MESSAGE(local.Name == "Second", "the local player is this machine's slot");
	CHECK_MESSAGE(local.Human, "every listed player is a person");
	CHECK_MESSAGE(load.Session.Host, "the host is this machine's");
	CHECK_MESSAGE(load.GameSpeed == std::optional<std::int32_t>(2), "the file's speed goes with the load");

	int observers = 0;
	for (Launch::Message::Player const & player : load.Players) {
		observers += player.Observer ? 1 : 0;
	}
	CHECK_MESSAGE(observers == 1, "an observer rejoins as an observer");

	REQUIRE(load.Network.has_value());
	CHECK_MESSAGE(load.Network->Port == 50000, "this machine listens where the file says");
	REQUIRE(load.Network->Peers.size() == 1);
	Launch::Message::Peer const & peer = load.Network->Peers[0];
	CHECK_MESSAGE(load.Players[peer.Player].Name == "First", "the peer is the other slot");
	CHECK_MESSAGE(peer.Address == "10.0.0.7", "the peer is reached where the file says");
	CHECK_MESSAGE(peer.Port == 50001, "the peer is reached where the file says");

	Launch::Machine machine;
	REQUIRE_MESSAGE(machine.From_Load(load, true, fault), fault);
	REQUIRE(machine.Network.has_value());
	CHECK_MESSAGE(machine.LocalPlayer == int(*load.Session.LocalPlayer), "the machine part keeps the local player");
	CHECK_MESSAGE(machine.Network->ConnectTimeout == 1000, "the wait comes back in the file's ticks");
	CHECK_MESSAGE(machine.Network->Peers.size() == 1, "the peer is kept");

	// The game matches a player to a saved house by name, so two players cannot share one.
	Launch::Message::Load twins = load;
	twins.Players[1 - *load.Session.LocalPlayer].Name = "SECOND";
	CHECK_MESSAGE(!machine.From_Load(twins, true, fault), "two players with one name are refused");
	CHECK_MESSAGE(fault == "Players 0 and 1 have the same name.", fault);

	Launch::Message::Load folder = load;
	folder.Save = "..\\SVGM_000.NET";
	CHECK_MESSAGE(!machine.From_Load(folder, false, fault), "a save with a folder part is refused");
}


// The session takes this machine's values as the game uses them: a negative autosave interval
// or ring position is 0, and a load screen position is kept whenever either coordinate is set.
TEST_CASE("a launch's session keeps what the game does with this machine's values")
{
	char const shown[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"AutoSaveGame=-5\n"
		"NextSPAutoSaveId=-1\n"
		"NextSkirmishAutoSaveId=3\n"
		"CustomLoadScreen=Resources/l600s01.pcx\n"
		"CustomLoadScreenPos=317,401\n"
		"WriteStatistics=Yes\n"
		"GameID=-123456789\n"
		"MapHash=0123456789abcdef0123456789abcdef01234567\n"
		"UIMapName=Tiny Tunnels\n"
		"DifficultyName=Gentle\n"
		"SkipScoreScreen=Yes\n"
		"QuickMatch=Yes\n";
	Launch::Message::Play play = Play_Of(Read(shown, sizeof(shown) - 1));
	Launch::Message::Session const & session = play.Session;

	CHECK_MESSAGE(session.AutosaveInterval == 0, "a negative interval saves nothing automatically");
	CHECK_MESSAGE(session.NextCampaignAutosave == 0, "a negative save position is the first");
	CHECK_MESSAGE(session.NextSkirmishAutosave == 2, "the client's save numbering is shifted to the game's");
	CHECK_MESSAGE(session.LoadScreen.X == std::optional<std::int32_t>(317), "a load screen position is kept whole");
	CHECK_MESSAGE(session.LoadScreen.Y == std::optional<std::int32_t>(401), "a load screen position is kept whole");
	CHECK_MESSAGE(session.WriteStatistics, "the results file is this machine's");
	CHECK_MESSAGE(session.GameId == std::uint32_t(-123456789), "the match number keeps its bits");
	CHECK_MESSAGE(session.MapHash == "0123456789abcdef0123456789abcdef01234567", "the map hash is as the client wrote it");
	CHECK_MESSAGE(session.MapName == "Tiny Tunnels", "the map name is as the client wrote it");
	CHECK_MESSAGE(session.DifficultyName == "Gentle", "the difficulty name is as the client wrote it");
	CHECK_MESSAGE(session.SkipScoreScreen, "passing over the score screen is this machine's");
	CHECK_MESSAGE(play.Game.Anonymous, "a match that shows players by number says so in the launch");

	char const unplaced[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"CustomLoadScreen=Resources/l600s01.pcx\n"
		"CustomLoadScreenPos=317,0\n";
	Launch::Message::Play bare = Play_Of(Read(unplaced, sizeof(unplaced) - 1));
	CHECK_MESSAGE(bare.Session.LoadScreen.X == std::optional<std::int32_t>(317), "a position the game does not use is still kept with the scenario");
	CHECK_MESSAGE(bare.Session.LoadScreen.Y == std::optional<std::int32_t>(0), "a position the game does not use is still kept with the scenario");

	char const none[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n";
	Launch::Message::Play absent = Play_Of(Read(none, sizeof(none) - 1));
	CHECK_MESSAGE(!absent.Session.LoadScreen.X.has_value(), "no position is left to the game");
	CHECK_MESSAGE(!absent.Session.LoadScreen.Y.has_value(), "no position is left to the game");
}


// The request gets the speed as the file wrote it, and a speed the options do not offer is
// refused with the file's section and key in front.
TEST_CASE("a speed outside the options is refused as the file wrote it")
{
	char const fast[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n"
		"Color=0\n"
		"GameSpeed=9\n";
	Spawner::Config config = Read(fast, sizeof(fast) - 1);

	std::string fault;
	CHECK_MESSAGE(!File_Is_Taken(config, 2, 8, 1, fault), "a speed of 9 is refused");
	CHECK_MESSAGE(fault.find("SPAWN.INI [Settings] GameSpeed: ") == 0, fault);

	Launch::Message::Play play = Play_Of(config);
	Launch::Message::Option const * speed = Option_Of(play.Game, "GameSpeed");
	REQUIRE(speed != nullptr);
	CHECK_MESSAGE(std::get<std::int64_t>(speed->Value) == 9, "the translator does not clamp the speed");

	char const resume[] =
		"[Settings]\n"
		"LoadSaveGame=Yes\n"
		"SaveGameName=SAVEGAME.001\n"
		"GameSpeed=-1\n";
	CHECK_MESSAGE(!File_Is_Taken(Read(resume, sizeof(resume) - 1), 2, 8, 1, fault), "a resume's speed is judged too");
	CHECK_MESSAGE(fault.find("SPAWN.INI [Settings] GameSpeed: ") == 0, fault);
}


// A campaign is refused for its difficulty pair and its campaign number, with the file's section
// and key in front.
TEST_CASE("a campaign's numbers are refused as the file wrote them")
{
	char const hard[] =
		"[Settings]\n"
		"IsSinglePlayer=Yes\n"
		"DifficultyModeHuman=3\n";
	std::string fault;
	CHECK_MESSAGE(!File_Is_Taken(Read(hard, sizeof(hard) - 1), 2, 8, 4, fault), "a difficulty past the hardest is refused");
	CHECK_MESSAGE(fault.find("SPAWN.INI [Settings] DifficultyModeHuman: ") == 0, fault);

	char const unknown[] =
		"[Settings]\n"
		"IsSinglePlayer=Yes\n"
		"CampaignID=4\n";
	CHECK_MESSAGE(!File_Is_Taken(Read(unknown, sizeof(unknown) - 1), 2, 8, 4, fault), "a campaign the list lacks is refused");
	CHECK_MESSAGE(fault.find("SPAWN.INI [Settings] CampaignID: ") == 0, fault);

	CHECK_MESSAGE(File_Is_Taken(Read(_Campaign, sizeof(_Campaign) - 1), 2, 8, 0, fault), fault);
	CHECK_MESSAGE(File_Is_Taken(Read(_Resume, sizeof(_Resume) - 1), 2, 8, 0, fault), fault);
	CHECK_MESSAGE(File_Is_Taken(Read(_Skirmish, sizeof(_Skirmish) - 1), 2, 8, 0, fault), fault);
}


// A client may write a name in the Windows code page rather than in UTF-8. The INI reader turns
// such a line into UTF-8 as it loads the file, so the name launches as it always has, and an
// encoded request built from it decodes without a fault.
TEST_CASE("a name that is not UTF-8 still launches")
{
	char const latin[] =
		"[Settings]\n"
		"Name=\xC9mile\n"
		"Side=0\n"
		"Color=0\n";
	Spawner::Config config = Read(latin, sizeof(latin) - 1);
	CHECK_MESSAGE(config.Slots[0].Name == "\xC3\x89mile", "the reader turns a Windows-1252 name into UTF-8");

	std::string fault;
	CHECK_MESSAGE(File_Is_Taken(config, 2, 8, 1, fault), fault);

	Launch::Message::Request request;
	config.To_Request(request);
	Launch::Message::Play const & play = std::get<Launch::Message::Play>(request.Verb);

	Launch::Game game;
	REQUIRE_MESSAGE(game.From_Message(play.Game, Launch::Game::LimitsType{ 2, 8, 1, 50 }, fault), fault);
	Launch::Machine machine;
	REQUIRE_MESSAGE(machine.From_Request(play, game, fault), fault);
	CHECK_MESSAGE(game.Players[0].Name == config.Slots[0].Name, "the player keeps the name the reader gave");

	Launch::Bytes bytes;
	Launch::Encode(request, bytes);
	Launch::Message::Request back;
	CHECK_MESSAGE(Launch::Decode_Request(bytes, back) == Launch::RequestFault::NONE, "the request is valid UTF-8");
}
