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

#include <cstdio>
#include <cstring>
#include <string>

#include "ini.h"
#include "spawnerconfig.h"
#include "xstraw.h"

namespace {

using LaunchType = SpawnerConfigClass::LaunchType;
using OccupancyType = SpawnerConfigClass::OccupancyType;

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-64s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


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


int main(void)
{
	/*
	 * A file naming nothing at all still describes a playable game, because every key the
	 * client leaves out has a settled meaning.
	 */
	{
		char const empty[] = "[Settings]\n";
		SpawnerConfigClass config = Read(empty, sizeof(empty) - 1);

		Check(config.Bases && config.Credits == 10000 && config.MCVRedeploy,
			"the unwritten keys keep the meaning clients expect");
		Check(config.TunnelId == 0 && config.ListenPort == 1234,
			"one absent port names no tunnel and still listens");
		Check(config.ScenarioName == "spawnmap.ini",
			"the scenario a client always writes is also the default");
		Check(config.NextCampaignAutoSave == 0 && config.NextSkirmishAutoSave == 0,
			"the first automatic save is numbered from zero");

		bool any = false;
		for (bool flag : config.GlobalFlags) {
			any = any || flag;
		}
		Check(!any, "no scenario flag is set unasked");
	}

	/*
	 * Resuming a saved game asks for almost nothing; the save answers for the rest, so the
	 * reading alone decides the kind of launch.
	 */
	{
		SpawnerConfigClass config = Read(_Resume, sizeof(_Resume) - 1);

		Check(config.Launch_Type() == LaunchType::Resume, "resuming a save is what the file asks for");
		Check(config.SaveGameName == "SAVEGAME.001", "the saved game is named");
		Check(!config.Firestorm, "the expansion is left out when the file says so");
	}

	/*
	 * A saved game is opened by name in the game's own folder, so a name written with a path
	 * is reduced to the name without ceremony.
	 */
	{
		char const traversal[] =
			"[Settings]\n"
			"LoadSaveGame=Yes\n"
			"SaveGameName=..\\..\\Windows\\SAVEGAME.001\n";
		SpawnerConfigClass config = Read(traversal, sizeof(traversal) - 1);

		Check(config.SaveGameName == "SAVEGAME.001", "only the name of the saved game is read");

		char const forward[] =
			"[Settings]\n"
			"SaveGameName=saves/deep/SAVEGAME.002\n";
		config = Read(forward, sizeof(forward) - 1);

		Check(config.SaveGameName == "SAVEGAME.002", "a forward slash hides nothing either");

		char const drive[] =
			"[Settings]\n"
			"SaveGameName=C:SAVEGAME.003\n";
		config = Read(drive, sizeof(drive) - 1);

		Check(config.SaveGameName == "SAVEGAME.003", "a drive letter is not part of the name");
	}

	/*
	 * A campaign takes its houses from the map, so a file naming no player is complete.
	 */
	{
		SpawnerConfigClass config = Read(_Campaign, sizeof(_Campaign) - 1);

		Check(config.Launch_Type() == LaunchType::Campaign, "a single player game is a campaign");
		Check(config.CampaignDifficulty == 0 && config.CampaignCDifficulty == 2,
			"the two difficulties are read apart");
		Check(config.CampaignID == -1, "a mission outside any campaign says so");
	}

	/*
	 * The seats end up in the order the houses are created in, which is what everything
	 * naming a seat by position afterwards means.
	 */
	{
		SpawnerConfigClass config = Read(_Skirmish, sizeof(_Skirmish) - 1);

		Check(config.Launch_Type() == LaunchType::Skirmish, "one player and computers is a skirmish");
		Check(config.HumanCount == 1 && config.LocalSlot == 0, "the only player holds the first seat");
		Check(config.Slots[0].Occupancy == OccupancyType::Human &&
			config.Slots[0].Name == "Commander" &&
			config.Slots[0].Color == 4 && config.Slots[0].Country == 1,
			"the player is read from the settings themselves");
		Check(config.Slots[1].Occupancy == OccupancyType::Computer && config.Slots[1].Color == 2 &&
			config.Slots[1].Country == 0 && config.Slots[1].Handicap == 2,
			"a seat no section claimed is a computer player named by position");
		Check(config.Slots[2].Occupancy == OccupancyType::Computer && config.Slots[2].Color == 7,
			"the second computer player follows the first");
		Check(config.Slots[3].Occupancy == OccupancyType::Empty, "no more seats are filled than are asked for");
		Check(config.Slots[0].StartingPosition == 3, "a start position is read for the seat that asked");
		Check(config.Slots[1].StartingPosition == -1,
			"a start position the map cannot hold becomes the game's own choice");
		Check(config.Slots[2].StartingPosition == -1,
			"a position below the game's own choice is that choice");
		Check(config.Slots[0].Alliances[0] == 1, "an alliance is read as the seat it names");
		Check(config.NextSkirmishAutoSave == 4, "a client's save numbering is shifted to the game's");
	}

	/*
	 * The player who wrote the file is not always the first house, and the game has to know
	 * which seat is his once the sorting is done.
	 */
	{
		SpawnerConfigClass config = Read(_Network, sizeof(_Network) - 1);

		Check(config.HumanCount == 2, "a written section is what makes a seat a player");
		Check(config.Slots[0].Name == "First" && config.Slots[1].Name == "Second",
			"the players are sorted into the order their houses are created in");
		Check(config.LocalSlot == 1, "this machine knows which of the seats is its own");
		Check(config.Slots[0].Address == "10.0.0.7" && config.Slots[0].Port == 50001,
			"the other machine is read with the address it answers on");
		Check(config.TunnelId == 50000 && config.ListenPort == 50000,
			"one written port both listens and names this machine to a tunnel");
		Check(config.TunnelAddress == "88.99.11.22" && config.TunnelPort == 50010,
			"the tunnel is read from its own section");
		Check(config.Launch_Type() == LaunchType::Multiplayer,
			"two players is a game against another machine");
		Check(config.Slots[0].IsSpectator && !config.Slots[1].IsSpectator,
			"a watcher is named by the seat order the houses take");
		Check(config.Slots[1].Alliances[0] == 0,
			"an alliance section names the sorted seat too");
	}

	/*
	 * Every machine writes its own file with itself first, so a color tie must be broken by what
	 * the seats say rather than by file order.
	 */
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
		Check(a.Slots[0].Name == "Alpha" && b.Slots[0].Name == "Alpha",
			"a color tie seats the match identically on every machine");
		Check(a.LocalSlot == 0 && b.LocalSlot == 1,
			"each machine still knows which of the tied seats is its own");
		Check(a.Session_Identity_CRC() == b.Session_Identity_CRC(),
			"the tied match carries one identity on both machines");
	}

	/*
	 * Two machines handed the same match agree on its identity, and a difference in what
	 * either of them merely displays cannot move it.
	 */
	{
		SpawnerConfigClass one = Read(_Skirmish, sizeof(_Skirmish) - 1);
		SpawnerConfigClass two = Read(_Skirmish, sizeof(_Skirmish) - 1);

		Check(one.Session_Identity_CRC() == two.Session_Identity_CRC(),
			"the same match is given the same identity twice");

		two.MapName = "A Map By Another Name";
		two.DifficultyName = "Gentle";
		two.Slots[0].Name = "Somebody Else";
		Check(one.Session_Identity_CRC() == two.Session_Identity_CRC(),
			"what a player is shown is left out of the identity");

		two.Credits = one.Credits + 1;
		Check(one.Session_Identity_CRC() != two.Session_Identity_CRC(),
			"a value the match is played by moves the identity");

		SpawnerConfigClass three = Read(_Skirmish, sizeof(_Skirmish) - 1);
		three.Slots[1].Country = one.Slots[1].Country + 1;
		Check(one.Session_Identity_CRC() != three.Session_Identity_CRC(),
			"a computer player's country moves the identity");

		SpawnerConfigClass four = Read(_Skirmish, sizeof(_Skirmish) - 1);
		four.GlobalFlags[49] = !four.GlobalFlags[49];
		Check(one.Session_Identity_CRC() != four.Session_Identity_CRC(),
			"a scenario flag moves the identity");

		/*
		 * A resume is a match of its own: the saved game decides everything the fields above
		 * would otherwise have decided, so which save is being resumed is part of the identity.
		 */
		SpawnerConfigClass five = Read(_Skirmish, sizeof(_Skirmish) - 1);
		five.LoadSaveGame = !five.LoadSaveGame;
		Check(one.Session_Identity_CRC() != five.Session_Identity_CRC(),
			"resuming a save rather than starting one moves the identity");

		SpawnerConfigClass six = Read(_Resume, sizeof(_Resume) - 1);
		SpawnerConfigClass seven = Read(_Resume, sizeof(_Resume) - 1);
		seven.SaveGameName = "SAVEGAME.002";
		Check(six.Session_Identity_CRC() != seven.Session_Identity_CRC(),
			"resuming another saved game moves the identity");

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
		Check(eight.Session_Identity_CRC() == nine.Session_Identity_CRC(),
			"where the machines reach one another is left out of the identity");
	}

	/*
	 * The timing the machines keep is the game's own, and a key the game does not know is
	 * passed over, so neither can move a match's identity.
	 */
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
		Check(a.Session_Identity_CRC() == b.Session_Identity_CRC(),
			"timing and unknown keys cannot move a match's identity");
	}

	/*
	 * The scenario flags are named by their number, and a load screen position is taken
	 * whole or not at all.
	 */
	{
		char const flags[] =
			"[Settings]\n"
			"CustomLoadScreenPos=317,401\n"
			"\n"
			"[GlobalFlags]\n"
			"GlobalFlag0=yes\n"
			"GlobalFlag49=yes\n";
		SpawnerConfigClass config = Read(flags, sizeof(flags) - 1);

		Check(config.GlobalFlags[0] && config.GlobalFlags[49],
			"a scenario flag is read by its number");

		bool between = false;
		for (int index = 1; index < 49; index++) {
			between = between || config.GlobalFlags[index];
		}
		Check(!between, "no flag is set by a neighbor's spelling");
		Check(config.CustomLoadScreenX == 317 && config.CustomLoadScreenY == 401,
			"the load screen position is read whole");

		char const malformed[] =
			"[Settings]\n"
			"CustomLoadScreenPos=oops\n";
		config = Read(malformed, sizeof(malformed) - 1);

		Check(config.CustomLoadScreenX == 0 && config.CustomLoadScreenY == 0,
			"a position the reader cannot make sense of is no position");

		char const half[] =
			"[Settings]\n"
			"CustomLoadScreenPos=12\n";
		config = Read(half, sizeof(half) - 1);

		Check(config.CustomLoadScreenX == 0 && config.CustomLoadScreenY == 0,
			"half a position is no position either");
	}

	/*
	 * Reading a launch file cannot fail, so whether what it describes can be played is
	 * judged separately, against the tables the game has loaded by the time it launches.
	 */
	{
		std::string fault;

		Check(Judge(_Skirmish, sizeof(_Skirmish) - 1, 2, 8, fault),
			"a match the loaded rules can hold is played");

		char const crowded[] =
			"[Settings]\n"
			"Name=Commander\n"
			"Side=0\n"
			"Color=0\n"
			"AIPlayers=8\n";
		Check(!Judge(crowded, sizeof(crowded) - 1, 2, 8, fault) &&
			fault.find("8") != std::string::npos && fault.find("7") != std::string::npos,
			"more computer players than seats names both counts");

		char const negative[] =
			"[Settings]\n"
			"Name=Commander\n"
			"Side=0\n"
			"Color=0\n"
			"AIPlayers=-1\n";
		Check(!Judge(negative, sizeof(negative) - 1, 2, 8, fault),
			"fewer than no computer players is refused");

		char const nameless_country[] =
			"[Settings]\n"
			"Name=Commander\n"
			"Color=0\n";
		Check(!Judge(nameless_country, sizeof(nameless_country) - 1, 2, 8, fault),
			"a person's country is never the game's to draw");

		char const nameless_color[] =
			"[Settings]\n"
			"Name=Commander\n"
			"Side=0\n";
		Check(!Judge(nameless_color, sizeof(nameless_color) - 1, 2, 8, fault),
			"a person's color is never the game's to draw either");

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
		Check(Judge(drawn_computer, sizeof(drawn_computer) - 1, 2, 8, fault),
			"a computer seat may leave its country and color to the game");

		char const past_countries[] =
			"[Settings]\n"
			"Name=Commander\n"
			"Side=2\n"
			"Color=0\n";
		Check(!Judge(past_countries, sizeof(past_countries) - 1, 2, 8, fault),
			"a country the rules did not declare is refused");

		char const past_colors[] =
			"[Settings]\n"
			"Name=Commander\n"
			"Side=0\n"
			"Color=8\n";
		Check(!Judge(past_colors, sizeof(past_colors) - 1, 2, 8, fault),
			"a color the game has no scheme for is refused");

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
		Check(!Judge(shared_color, sizeof(shared_color) - 1, 2, 8, fault),
			"two people of one color are refused against other machines");

		char const shared_with_computer[] =
			"[Settings]\n"
			"Name=Commander\n"
			"Side=0\n"
			"Color=3\n"
			"AIPlayers=1\n"
			"\n"
			"[HouseColors]\n"
			"Multi2=3\n";
		Check(Judge(shared_with_computer, sizeof(shared_with_computer) - 1, 2, 8, fault),
			"a computer player may take the color its opponent plays");

		char const past_difficulty[] =
			"[Settings]\n"
			"Name=Commander\n"
			"Side=0\n"
			"Color=0\n"
			"AIPlayers=1\n"
			"\n"
			"[HouseHandicaps]\n"
			"Multi2=7\n";
		Check(!Judge(past_difficulty, sizeof(past_difficulty) - 1, 2, 8, fault),
			"a difficulty naming none is refused");

		char const easy_difficulty[] =
			"[Settings]\n"
			"Name=Commander\n"
			"Side=0\n"
			"Color=0\n"
			"AIPlayers=1\n"
			"\n"
			"[HouseHandicaps]\n"
			"Multi2=6\n";
		Check(Judge(easy_difficulty, sizeof(easy_difficulty) - 1, 2, 8, fault),
			"a difficulty easier than the game holds is played, not refused");

		Check(SpawnerConfigClass::Playable_Handicap(-1) == -1 && SpawnerConfigClass::Playable_Handicap(0) == 0 &&
			SpawnerConfigClass::Playable_Handicap(2) == 2 && SpawnerConfigClass::Playable_Handicap(3) == 2 &&
			SpawnerConfigClass::Playable_Handicap(6) == 2,
			"an easier opponent than the game has comes to the easiest opponent it has");

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
		Check(Judge(two_machines, sizeof(two_machines) - 1, 2, 8, fault),
			"a match against another machine with everybody named is played");

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
		Check(!Judge(nameless_machine, sizeof(nameless_machine) - 1, 2, 8, fault),
			"a person the file leaves unnamed is refused against other machines");

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
		Check(!Judge(one_name, sizeof(one_name) - 1, 2, 8, fault) &&
			fault.find("1") != std::string::npos && fault.find("2") != std::string::npos,
			"two people under one name are refused however either is spelled");

		char const alone[] =
			"[Settings]\n"
			"Side=0\n"
			"Color=0\n";
		Check(Judge(alone, sizeof(alone) - 1, 2, 8, fault),
			"somebody playing alone need not be named");

		char const nobody[] =
			"[GlobalFlags]\n"
			"GlobalFlag0=yes\n";
		Check(!Judge(nobody, sizeof(nobody) - 1, 2, 8, fault),
			"a file seating nobody at this machine is refused");

		char const past_ai_difficulty[] =
			"[Settings]\n"
			"Name=Commander\n"
			"Side=0\n"
			"Color=0\n"
			"AIDifficulty=7\n";
		Check(!Judge(past_ai_difficulty, sizeof(past_ai_difficulty) - 1, 2, 8, fault),
			"a computer difficulty the game does not have is refused");

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
		Check(!Judge(unreachable, sizeof(unreachable) - 1, 2, 8, fault),
			"a machine the file gives no port is refused");

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
		Check(!Judge(nowhere, sizeof(nowhere) - 1, 2, 8, fault),
			"an address naming no machine is refused");

		char const tunnelled[] =
			"[Settings]\n"
			"Name=Alpha\n"
			"Side=0\n"
			"Color=3\n"
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
		Check(Judge(tunnelled, sizeof(tunnelled) - 1, 2, 8, fault),
			"a tunnelled machine is named by its number rather than an address");

		char const one_kept_name[] =
			"[Settings]\n"
			"Name=CommanderAlphaOmegaX\n"
			"Side=0\n"
			"Color=3\n"
			"\n"
			"[Other1]\n"
			"Name=CommanderAlphaOmegaY\n"
			"Side=1\n"
			"Color=5\n"
			"Ip=10.0.0.9\n"
			"Port=50002\n";
		Check(!Judge(one_kept_name, sizeof(one_kept_name) - 1, 2, 8, fault),
			"two names the game keeps as one are refused");

		char const unheld_ally[] =
			"[Settings]\n"
			"Name=Commander\n"
			"Side=0\n"
			"Color=0\n"
			"\n"
			"[Multi1_Alliances]\n"
			"HouseAllyOne=5\n";
		Check(!Judge(unheld_ally, sizeof(unheld_ally) - 1, 2, 8, fault),
			"an alliance with a seat nobody occupies is refused");

		char const past_seats[] =
			"[Settings]\n"
			"Name=Commander\n"
			"Side=0\n"
			"Color=0\n"
			"\n"
			"[Multi1_Alliances]\n"
			"HouseAllyOne=8\n";
		Check(!Judge(past_seats, sizeof(past_seats) - 1, 2, 8, fault),
			"an alliance with a seat the match does not hold is refused");

		char const watcher[] =
			"[Settings]\n"
			"Name=Commander\n"
			"Side=0\n"
			"Color=0\n"
			"\n"
			"[IsSpectator]\n"
			"Multi1=Yes\n";
		Check(!Judge(watcher, sizeof(watcher) - 1, 2, 8, fault),
			"a seat that watches rather than plays is refused");

		Check(!Judge(_Skirmish, sizeof(_Skirmish) - 1, 0, 8, fault),
			"a match is refused rather than read against countries the rules never declared");
	}

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
