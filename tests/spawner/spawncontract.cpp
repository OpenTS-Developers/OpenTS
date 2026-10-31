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

#include <cstring>
#include <string>

#include "ini.h"
#include "spawnerconfig.h"
#include "xstraw.h"

#include "doctest/doctest.h"

namespace {

using LaunchType = SpawnerConfigClass::LaunchType;
using OccupancyType = SpawnerConfigClass::OccupancyType;


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


SpawnerConfigClass Read(char const * text, int length)
{
	INIClass ini;
	BufferStraw straw(text, length);
	ini.Load(straw);

	SpawnerConfigClass config;
	config.Read_INI(ini);
	return(config);
}


bool Judge(char const * text, int length, int countries, int colors, std::string & fault)
{
	SpawnerConfigClass config = Read(text, length);
	fault.clear();
	return(config.Is_Playable(countries, colors, fault));
}

}


// A file naming nothing at all still describes a playable game, because every key the client
// leaves out has a settled meaning.
TEST_CASE("a file naming nothing at all still describes a playable game")
{
	char const empty[] = "[Settings]\n";
	SpawnerConfigClass config = Read(empty, sizeof(empty) - 1);

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
	CHECK_MESSAGE(!config.Paranoid, "computer houses stay apart unless the file says otherwise");

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
	SpawnerConfigClass config = Read(off, sizeof(off) - 1);

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
	SpawnerConfigClass config = Read(written, sizeof(written) - 1);

	CHECK_MESSAGE(config.ConnTimeout == 1800, "a wait within the bounds is taken in ticks as written");
	CHECK_MESSAGE(config.ReconnectTimeout == 1400, "a wait within the bounds is taken in ticks as written");

	char const low[] =
		"[Settings]\n"
		"ConnTimeout=1\n"
		"ReconnectTimeout=-5\n";
	config = Read(low, sizeof(low) - 1);

	CHECK_MESSAGE(config.ConnTimeout == SpawnerConfigClass::TIMEOUT_MIN, "a wait shorter than the game can honor is raised to the floor");
	CHECK_MESSAGE(config.ReconnectTimeout == SpawnerConfigClass::TIMEOUT_MIN, "a wait shorter than the game can honor is raised to the floor");

	char const high[] =
		"[Settings]\n"
		"ConnTimeout=999999\n"
		"ReconnectTimeout=999999\n";
	config = Read(high, sizeof(high) - 1);

	CHECK_MESSAGE(config.ConnTimeout == SpawnerConfigClass::TIMEOUT_MAX, "a wait longer than the game will hold is lowered to the ceiling");
	CHECK_MESSAGE(config.ReconnectTimeout == SpawnerConfigClass::TIMEOUT_MAX, "a wait longer than the game will hold is lowered to the ceiling");
}


TEST_CASE("a departing player's base is destroyed unless the file asks for the computer to take it")
{
	char const off[] =
		"[Settings]\n"
		"AutoSurrender=No\n";
	SpawnerConfigClass config = Read(off, sizeof(off) - 1);

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
	SpawnerConfigClass config = Read(on, sizeof(on) - 1);

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
	SpawnerConfigClass config = Read(on, sizeof(on) - 1);

	CHECK_MESSAGE(config.AutoDeployMCV, "the base unit deploys itself when the file says so");

	char const spelled[] =
		"[Settings]\n"
		"AutoDeployMCV=True\n";
	config = Read(spelled, sizeof(spelled) - 1);

	CHECK_MESSAGE(config.AutoDeployMCV, "the option is read whichever way it spells yes");
}


TEST_CASE("computer houses stay apart when the file says so")
{
	char const off[] =
		"[Settings]\n"
		"Paranoid=no\n";
	SpawnerConfigClass config = Read(off, sizeof(off) - 1);
	CHECK_MESSAGE(!config.Paranoid, "Paranoid=no leaves the computer houses apart");
}


TEST_CASE("acquiring a neutral house's objects is asked for by the file alone")
{
	char const on[] =
		"[Settings]\n"
		"AttackNeutralUnits=Yes\n";
	SpawnerConfigClass config = Read(on, sizeof(on) - 1);

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
	SpawnerConfigClass config = Read(_Resume, sizeof(_Resume) - 1);

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
	SpawnerConfigClass config = Read(traversal, sizeof(traversal) - 1);

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
	SpawnerConfigClass config = Read(_Campaign, sizeof(_Campaign) - 1);

	CHECK_MESSAGE(config.Launch_Type() == LaunchType::Campaign, "a single player game is a campaign");
	CHECK_MESSAGE(config.CampaignDifficulty == 0, "the two difficulties are read apart");
	CHECK_MESSAGE(config.CampaignCDifficulty == 2, "the two difficulties are read apart");
	CHECK_MESSAGE(config.CampaignID == -1, "a mission outside any campaign says so");
}


// The seats end up in the order the houses are created in, which is what everything naming a
// seat by position afterwards means.
TEST_CASE("the seats end up in the order the houses are created in")
{
	SpawnerConfigClass config = Read(_Skirmish, sizeof(_Skirmish) - 1);

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
	SpawnerConfigClass config = Read(_Network, sizeof(_Network) - 1);

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

	SpawnerConfigClass a = Read(view_a, sizeof(view_a) - 1);
	SpawnerConfigClass b = Read(view_b, sizeof(view_b) - 1);
	CHECK_MESSAGE(a.Slots[0].Name == "Alpha", "a color tie seats the match identically on every machine");
	CHECK_MESSAGE(b.Slots[0].Name == "Alpha", "a color tie seats the match identically on every machine");
	CHECK_MESSAGE(a.LocalSlot == 0, "each machine still knows which of the tied seats is its own");
	CHECK_MESSAGE(b.LocalSlot == 1, "each machine still knows which of the tied seats is its own");
	CHECK_MESSAGE(a.Session_Identity_CRC() == b.Session_Identity_CRC(), "the tied match carries one identity on both machines");
}


// Two machines handed the same match agree on its identity, and a difference in what either of
// them merely displays cannot move it.
TEST_CASE("two machines handed the same match agree on its identity")
{
	SpawnerConfigClass one = Read(_Skirmish, sizeof(_Skirmish) - 1);
	SpawnerConfigClass two = Read(_Skirmish, sizeof(_Skirmish) - 1);

	CHECK_MESSAGE(one.Session_Identity_CRC() == two.Session_Identity_CRC(), "the same match is given the same identity twice");

	two.MapName = "A Map By Another Name";
	two.DifficultyName = "Gentle";
	two.SkipScoreScreen = !two.SkipScoreScreen;
	two.QuickMatch = !two.QuickMatch;
	two.CustomLoadScreen = "Resources/l600s02.pcx";
	two.Slots[0].Name = "Somebody Else";
	CHECK_MESSAGE(one.Session_Identity_CRC() == two.Session_Identity_CRC(), "what a player is shown is left out of the identity");

	two.Credits = one.Credits + 1;
	CHECK_MESSAGE(one.Session_Identity_CRC() != two.Session_Identity_CRC(), "a value the match is played by moves the identity");

	SpawnerConfigClass three = Read(_Skirmish, sizeof(_Skirmish) - 1);
	three.Slots[1].Country = one.Slots[1].Country + 1;
	CHECK_MESSAGE(one.Session_Identity_CRC() != three.Session_Identity_CRC(), "a computer player's country moves the identity");

	SpawnerConfigClass four = Read(_Skirmish, sizeof(_Skirmish) - 1);
	four.GlobalFlags[49] = !four.GlobalFlags[49];
	CHECK_MESSAGE(one.Session_Identity_CRC() != four.Session_Identity_CRC(), "a scenario flag moves the identity");

	SpawnerConfigClass surrender = Read(_Skirmish, sizeof(_Skirmish) - 1);
	surrender.AutoSurrender = !surrender.AutoSurrender;
	CHECK_MESSAGE(one.Session_Identity_CRC() != surrender.Session_Identity_CRC(), "what becomes of a departing player's base moves the identity");

	SpawnerConfigClass ally = Read(_Skirmish, sizeof(_Skirmish) - 1);
	ally.BuildOffAlly = !ally.BuildOffAlly;
	CHECK_MESSAGE(one.Session_Identity_CRC() != ally.Session_Identity_CRC(), "building next to an ally moves the identity");

	SpawnerConfigClass deploy = Read(_Skirmish, sizeof(_Skirmish) - 1);
	deploy.AutoDeployMCV = !deploy.AutoDeployMCV;
	CHECK_MESSAGE(one.Session_Identity_CRC() != deploy.Session_Identity_CRC(), "deploying the starting base unit moves the identity");

	SpawnerConfigClass neutrals = Read(_Skirmish, sizeof(_Skirmish) - 1);
	neutrals.AttackNeutralUnits = !neutrals.AttackNeutralUnits;
	CHECK_MESSAGE(one.Session_Identity_CRC() != neutrals.Session_Identity_CRC(), "scanning a neutral house moves the identity");

	SpawnerConfigClass paranoid = Read(_Skirmish, sizeof(_Skirmish) - 1);
	paranoid.Paranoid = !paranoid.Paranoid;
	CHECK_MESSAGE(one.Session_Identity_CRC() != paranoid.Session_Identity_CRC(), "computer houses closing ranks moves the identity");

	SpawnerConfigClass waits = Read(_Skirmish, sizeof(_Skirmish) - 1);
	waits.ConnTimeout = one.ConnTimeout + 60;
	waits.ReconnectTimeout = one.ReconnectTimeout + 60;
	CHECK_MESSAGE(one.Session_Identity_CRC() == waits.Session_Identity_CRC(), "how long a machine waits is its own, so it is left out of the identity");

	SpawnerConfigClass ten = Read(_Skirmish, sizeof(_Skirmish) - 1);
	ten.CoachMode = !ten.CoachMode;
	CHECK_MESSAGE(one.Session_Identity_CRC() != ten.Session_Identity_CRC(), "coach mode moves the identity");

	SpawnerConfigClass eleven = Read(_Skirmish, sizeof(_Skirmish) - 1);
	eleven.PlayMoviesInMultiplayer = !eleven.PlayMoviesInMultiplayer;
	CHECK_MESSAGE(one.Session_Identity_CRC() != eleven.Session_Identity_CRC(), "whether movies play moves the identity");

	/*
	 * A resume is a match of its own: the saved game decides everything the fields above
	 * would otherwise have decided, so which save is being resumed is part of the identity.
	 */
	SpawnerConfigClass five = Read(_Skirmish, sizeof(_Skirmish) - 1);
	five.LoadSaveGame = !five.LoadSaveGame;
	CHECK_MESSAGE(one.Session_Identity_CRC() != five.Session_Identity_CRC(), "resuming a save rather than starting one moves the identity");

	SpawnerConfigClass six = Read(_Resume, sizeof(_Resume) - 1);
	SpawnerConfigClass seven = Read(_Resume, sizeof(_Resume) - 1);
	seven.SaveGameName = "SAVEGAME.002";
	CHECK_MESSAGE(six.Session_Identity_CRC() != seven.Session_Identity_CRC(), "resuming another saved game moves the identity");

	/*
	 * Where the machines reach one another carries the match rather than shaping it, and each
	 * machine writes its own view, so it is left out as well.
	 */
	SpawnerConfigClass eight = Read(_Network, sizeof(_Network) - 1);
	SpawnerConfigClass nine = Read(_Network, sizeof(_Network) - 1);
	nine.TunnelAddress = "203.0.113.9";
	nine.TunnelPort = 50010;
	nine.ListenPort = 60000;
	nine.Slots[0].Address = "10.0.0.8";
	nine.Slots[0].Port = 50003;
	CHECK_MESSAGE(eight.Session_Identity_CRC() == nine.Session_Identity_CRC(), "where the machines reach one another is left out of the identity");
}


// The timing the machines keep is the game's own, and a key the game does not know is passed
// over, so neither can move a match's identity.
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

	SpawnerConfigClass a = Read(plain, sizeof(plain) - 1);
	SpawnerConfigClass b = Read(noisy, sizeof(noisy) - 1);
	CHECK_MESSAGE(a.Session_Identity_CRC() == b.Session_Identity_CRC(), "timing and unknown keys cannot move a match's identity");
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
	SpawnerConfigClass config = Read(flags, sizeof(flags) - 1);

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
	SpawnerConfigClass config = Read(shown, sizeof(shown) - 1);

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
	SpawnerConfigClass config = Read(results, sizeof(results) - 1);

	CHECK_MESSAGE(config.WriteStatistics, "a client may ask for the results file");
	CHECK_MESSAGE(config.GameID == -123456789, "the match number is read as the client wrote it");
	CHECK_MESSAGE(config.MapHash == "0123456789abcdef0123456789abcdef01234567", "the map hash is read as the client wrote it");

	char const silent[] = "[Settings]\n";
	SpawnerConfigClass plain = Read(silent, sizeof(silent) - 1);
	CHECK_MESSAGE(!plain.WriteStatistics, "an unwritten key writes no results file");
	CHECK_MESSAGE(plain.GameID == 0, "and reports no match number or map hash");
	CHECK_MESSAGE(plain.MapHash.empty(), "and reports no match number or map hash");

	plain.WriteStatistics = config.WriteStatistics;
	plain.GameID = config.GameID;
	plain.MapHash = config.MapHash;
	SpawnerConfigClass bare = Read(silent, sizeof(silent) - 1);
	CHECK_MESSAGE(plain.Session_Identity_CRC() == bare.Session_Identity_CRC(), "what the results file reports is left out of the identity");
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
	CHECK_MESSAGE(!Judge(nameless_country, sizeof(nameless_country) - 1, 2, 8, fault), "a person's country is never the game's to draw");

	char const nameless_color[] =
		"[Settings]\n"
		"Name=Commander\n"
		"Side=0\n";
	CHECK_MESSAGE(!Judge(nameless_color, sizeof(nameless_color) - 1, 2, 8, fault), "a person's color is never the game's to draw either");

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
	CHECK_MESSAGE(fault.find("[Settings] Side=2") != std::string::npos, "a country the rules did not declare is refused under the key that set it");

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
	CHECK_MESSAGE(fault.find("[Settings] Side=9") != std::string::npos, "a refusal names the person's own section after house order has moved them");

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
	CHECK_MESSAGE(fault.find("[HouseHandicaps] Multi2=7") != std::string::npos, "a difficulty naming none is refused under the entry that set it");

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

	CHECK_MESSAGE(SpawnerConfigClass::Playable_Handicap(-1) == -1, "an easier opponent than the game has comes to the easiest opponent it has");
	CHECK_MESSAGE(SpawnerConfigClass::Playable_Handicap(0) == 0, "an easier opponent than the game has comes to the easiest opponent it has");
	CHECK_MESSAGE(SpawnerConfigClass::Playable_Handicap(2) == 2, "an easier opponent than the game has comes to the easiest opponent it has");
	CHECK_MESSAGE(SpawnerConfigClass::Playable_Handicap(3) == 2, "an easier opponent than the game has comes to the easiest opponent it has");
	CHECK_MESSAGE(SpawnerConfigClass::Playable_Handicap(6) == 2, "an easier opponent than the game has comes to the easiest opponent it has");

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
	CHECK_MESSAGE(!Judge(one_name, sizeof(one_name) - 1, 2, 8, fault), "two people under one name are refused however either is spelled, naming both sections");
	CHECK_MESSAGE(fault.find("[Settings]") != std::string::npos, "two people under one name are refused however either is spelled, naming both sections");
	CHECK_MESSAGE(fault.find("[Other1]") != std::string::npos, "two people under one name are refused however either is spelled, naming both sections");

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
