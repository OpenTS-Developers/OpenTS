/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "spawnerconfig.h"

#include "crc.h"
#include "diff.hh"
#include "ini.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <vector>


namespace {

/*
 * The section a launch file keeps the match's settings in. It doubles as the section the
 * machine reading the file describes itself in, so the first seat is read from here.
 */
char const * const SETTINGS = "Settings";


/// <summary>
/// Reads a string entry.
/// </summary>
/// <returns>The value written, or the fallback.</returns>
std::string Read_Text(INIClass const & ini, char const * section, char const * entry, std::string const & fallback)
{
	char buffer[512];
	if (ini.Get_String(section, entry, "", buffer, sizeof(buffer)) == 0) {
		return(fallback);
	}
	return(buffer);
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


/// <summary>
/// Reads a dotted address, so that a seat naming an unreachable machine is refused where
/// every other fault is. The game's own resolver is not reachable from here.
/// </summary>
/// <returns>bool; Is this four numbers between 0 and 255?</returns>
bool Is_Address(std::string const & text)
{
	unsigned quad[4] = {};
	char tail = '\0';

	if (std::sscanf(text.c_str(), "%u.%u.%u.%u%c", &quad[0], &quad[1], &quad[2], &quad[3], &tail) != 4) {
		return(false);
	}

	for (unsigned part : quad) {
		if (part > 255) {
			return(false);
		}
	}

	return(quad[0] != 0 || quad[1] != 0 || quad[2] != 0 || quad[3] != 0);
}


/// <summary>
/// Names the fault that refuses a launch.
/// </summary>
/// <param name="format">A printf style description of the fault.</param>
/// <returns>false, so a caller can name a fault and refuse in one statement.</returns>
bool Fault(std::string & fault, char const * format, ...)
{
	char buffer[256];

	va_list args;
	va_start(args, format);
	std::vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	fault = buffer;
	return(false);
}

}


/// <summary>
/// Reads the match's seats. A seat is human because the file wrote a section for it, and the
/// seats are then sorted into the order their houses will be created in, because everything
/// naming a seat by position afterwards means that order.
/// </summary>
void SpawnerConfigClass::Read_Slots(INIClass const & ini)
{
	std::array<SlotType, SLOT_COUNT> staging;

	for (int index = 0; index < SLOT_COUNT; index++) {
		std::string section = index == 0 ? SETTINGS : "Other" + std::to_string(index);

		SlotType & slot = staging[index];
		if (ini.Section_Present(section.c_str())) {
			slot.Occupancy = OccupancyType::Human;
			slot.Name = Read_Text(ini, section.c_str(), "Name", "").substr(0, NAME_KEPT);
			slot.Color = ini.Get_Int(section.c_str(), "Color", -1);
			slot.Country = ini.Get_Int(section.c_str(), "Side", -1);
			slot.Address = Read_Text(ini, section.c_str(), "Ip", slot.Address);
			slot.Port = ini.Get_Int(section.c_str(), "Port", -1);
		} else {
			slot.Color = Read_Slot_Int(ini, "HouseColors", index, -1);
			slot.Country = Read_Slot_Int(ini, "HouseCountries", index, -1);
			slot.Handicap = Read_Slot_Int(ini, "HouseHandicaps", index, -1);
		}
	}

	/*
	 * Sorting by color makes a seat's index the house it becomes. Every machine writes its
	 * own file with itself first, so a name breaks a color tie rather than file order.
	 */
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

	/*
	 * What the file wrote for a seat no section claimed describes a computer player, and the
	 * options say how many of those are playing.
	 */
	for (int index : rest) {
		SlotType & slot = Slots[filled];
		slot = staging[index];
		slot.Occupancy = (filled - HumanCount) < AIPlayers ? OccupancyType::Computer : OccupancyType::Empty;
		filled++;
	}

	/*
	 * These name their seats by the sorted order, so they are read once the sorting is done.
	 */
	static char const * const _ordinals[SLOT_COUNT] = {
		"HouseAllyOne", "HouseAllyTwo", "HouseAllyThree", "HouseAllyFour",
		"HouseAllyFive", "HouseAllySix", "HouseAllySeven", "HouseAllyEight"
	};

	for (int index = 0; index < SLOT_COUNT; index++) {
		SlotType & slot = Slots[index];

		std::string entry = "Multi" + std::to_string(index + 1);
		slot.IsSpectator = ini.Get_Bool("IsSpectator", entry.c_str(), false);
		slot.StartingPosition = Read_Slot_Int(ini, "SpawnLocations", index, -1);

		/*
		 * A start position the map cannot hold is one the game picks instead, which is what
		 * a file asking for no particular position already means.
		 */
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
/// What kind of game this file asks for. Resuming a saved game answers by itself, since the
/// save carries the type, the options and the houses.
/// </summary>
SpawnerConfigClass::LaunchType SpawnerConfigClass::Launch_Type(void) const
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
/// The identity of the match this file asks for. It gathers every value the course of the
/// match depends upon and nothing merely shown, so two machines handed the same match agree.
/// The version leads: one file under two readings is not one match.
/// </summary>
int SpawnerConfigClass::Session_Identity_CRC(void) const
{
	CRCEngine crc;

	crc(SCHEMA_VERSION);

	crc(ScenarioName.c_str());
	crc(IsCampaign);
	crc(CampaignID);
	crc(CampaignDifficulty);
	crc(CampaignCDifficulty);
	crc(LoadSaveGame);
	crc(SaveGameName.c_str());

	crc(Bases);
	crc(Credits);
	crc(BridgeDestroy);
	crc(Crates);
	crc(ShortGame);
	crc(BuildOffAlly);
	crc(GameSpeed);
	crc(MultiEngineer);
	crc(UnitCount);
	crc(AIPlayers);
	crc(AIDifficulty);
	crc(AlliesAllowed);
	crc(HarvesterTruce);
	crc(FogOfWar);
	crc(MCVRedeploy);
	crc(Seed);
	crc(TechLevel);
	crc(Firestorm);
	crc(AttackNeutralUnits);
	crc(ScrapMetal);

	for (bool flag : GlobalFlags) {
		crc(flag);
	}

	for (SlotType const & slot : Slots) {
		crc(static_cast<int>(slot.Occupancy));
		crc(slot.Color);
		crc(slot.Country);
		crc(slot.Handicap);
		crc(slot.IsSpectator);
		crc(slot.StartingPosition);

		for (int ally : slot.Alliances) {
			crc(ally);
		}
	}

	return(crc());
}


/// <summary>
/// The difficulty a seat is played at. A client may offer more easy settings than the game
/// holds, and any easier request comes to the easiest one it has.
/// </summary>
/// <returns>The difficulty to play the seat at, or -1 for the session default.</returns>
int SpawnerConfigClass::Playable_Handicap(int asked)
{
	if (asked < 0) {
		return(-1);
	}
	if (asked > DIFF_HARD) {
		return(DIFF_EASY);
	}
	return(asked);
}


/// <summary>
/// Judges whether this reading describes a game that can be played. The countries and colors
/// are handed in because they are the rules', settled only once the game has loaded them.
/// </summary>
/// <param name="fault">Where to leave the sentence describing the first fault found.</param>
/// <returns>bool; Can the game this file describes be played?</returns>
bool SpawnerConfigClass::Is_Playable(int countries, int colors, std::string & fault) const
{
	/*
	 * A resumed match against other machines is seated from the file like any other, so it
	 * is held to the same rules; its kind is the save's rather than the file's.
	 */
	LaunchType kind = Launch_Type();
	bool multiplayer = kind == LaunchType::Multiplayer ||
		(kind == LaunchType::Resume && HumanCount > 1);

	if (kind != LaunchType::Campaign && HumanCount == 0) {
		return(Fault(fault, "The file seats nobody at this machine."));
	}

	if (AIDifficulty < 0 || AIDifficulty >= DIFF_COUNT) {
		return(Fault(fault, "The file plays the computer at difficulty %d, and there are %d.",
			AIDifficulty, DIFF_COUNT));
	}

	int free_seats = SLOT_COUNT - HumanCount;
	if (AIPlayers < 0 || AIPlayers > free_seats) {
		return(Fault(fault, "The file asks for %d computer players, and %d seats are left.",
			AIPlayers, free_seats));
	}

	for (int index = 0; index < SLOT_COUNT; index++) {
		SlotType const & slot = Slots[index];
		if (slot.Occupancy == OccupancyType::Empty) {
			continue;
		}

		bool human = slot.Occupancy == OccupancyType::Human;

		// A computer seat may leave its country and color to the game; a person's seat names both.
		if ((human || slot.Country != -1) && (slot.Country < 0 || slot.Country >= countries)) {
			return(Fault(fault, "Seat %d is given country %d, and there are %d to choose from.",
				index + 1, slot.Country, countries));
		}

		// A computer seat may share a color; only a color with no scheme refuses outright.
		if ((human || slot.Color != -1) && (slot.Color < 0 || slot.Color >= colors)) {
			return(Fault(fault, "Seat %d is given color %d, and there are %d to choose from.",
				index + 1, slot.Color, colors));
		}

		if (slot.Handicap < -1 || slot.Handicap > 6) {
			return(Fault(fault, "Seat %d is given difficulty %d, which names none.",
				index + 1, slot.Handicap));
		}

		for (int ally : slot.Alliances) {
			if (ally < -1 || ally >= SLOT_COUNT ||
				(ally >= 0 && Slots[ally].Occupancy == OccupancyType::Empty)) {
				return(Fault(fault, "Seat %d is allied to seat %d, which the match does not hold.",
					index + 1, ally + 1));
			}
		}

		if (slot.IsSpectator) {
			return(Fault(fault, "Seat %d watches rather than plays, which this game cannot yet do.",
				index + 1));
		}

		/*
		 * The seats are ordered by color, and the client keys what it writes for each of them
		 * by an order of its own that no other machine can rebuild. Two people of one color
		 * would therefore take each other's start position and alliances, so a match against
		 * other machines gives every person a color and a name of their own.
		 */
		if (human && multiplayer) {
			if (slot.Name.empty()) {
				return(Fault(fault, "Seat %d is played by somebody the file does not name.", index + 1));
			}

			for (int other = 0; other < index; other++) {
				if (Slots[other].Occupancy != OccupancyType::Human) {
					continue;
				}

				if (_stricmp(Slots[other].Name.c_str(), slot.Name.c_str()) == 0) {
					return(Fault(fault, "Seats %d and %d are both played by %s.",
						other + 1, index + 1, slot.Name.c_str()));
				}

				if (Slots[other].Color == slot.Color) {
					return(Fault(fault, "Seats %d and %d are both given color %d.",
						other + 1, index + 1, slot.Color));
				}
			}

			/*
			 * Through a tunnel a machine is named by the number carried where its port would
			 * go, so every seat but this one needs that number either way.
			 */
			if (index != LocalSlot) {
				if (slot.Port < 1 || slot.Port > 65535) {
					return(Fault(fault, "Seat %d is reached on port %d, which names no machine.",
						index + 1, slot.Port));
				}

				if (TunnelPort == 0 && !Is_Address(slot.Address)) {
					return(Fault(fault, "Seat %d is reached at %s, which names no machine.",
						index + 1, slot.Address.c_str()));
				}
			}
		}
	}

	return(true);
}


/// <summary>
/// Reads what the CnCNet client asked the game to launch. Reading cannot fail: an unwritten
/// key has a settled meaning, a nonsense value keeps it, and an unknown key is passed over.
/// </summary>
void SpawnerConfigClass::Read_INI(INIClass const & ini)
{
	IsCampaign = ini.Get_Bool(SETTINGS, "IsSinglePlayer", IsCampaign);
	IsHost = ini.Get_Bool(SETTINGS, "Host", IsHost);
	CampaignID = ini.Get_Int(SETTINGS, "CampaignID", CampaignID);
	Tournament = ini.Get_Int(SETTINGS, "Tournament", Tournament);
	GameID = ini.Get_Int(SETTINGS, "GameID", GameID);

	ScenarioName = Read_Text(ini, SETTINGS, "Scenario", ScenarioName);
	MapName = Read_Text(ini, SETTINGS, "UIMapName", MapName);

	LoadSaveGame = ini.Get_Bool(SETTINGS, "LoadSaveGame", LoadSaveGame);

	/*
	 * A saved game is opened by name in the game's own folder, so a name written with a
	 * path is reduced to its last element.
	 */
	SaveGameName = std::filesystem::path(Read_Text(ini, SETTINGS, "SaveGameName", SaveGameName)).filename().string();

	AutoSaveInterval = ini.Get_Int(SETTINGS, "AutoSaveGame", AutoSaveInterval);

	/*
	 * The client counts its automatic saves from one, while the game numbers them from zero.
	 */
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
	Seed = ini.Get_Int(SETTINGS, "Seed", Seed);
	TechLevel = ini.Get_Int(SETTINGS, "TechLevel", TechLevel);
	Firestorm = ini.Get_Bool(SETTINGS, "Firestorm", Firestorm);
	CampaignDifficulty = ini.Get_Int(SETTINGS, "DifficultyModeHuman", CampaignDifficulty);
	CampaignCDifficulty = ini.Get_Int(SETTINGS, "DifficultyModeComputer", CampaignCDifficulty);

	/*
	 * One key carries the port twice: a machine listens on it and a tunnel names the machine
	 * by it. Absent, a tunnel has no name while the game still has a port to listen on.
	 */
	TunnelId = ini.Get_Int(SETTINGS, "Port", TunnelId);
	ListenPort = ini.Get_Int(SETTINGS, "Port", ListenPort);
	TunnelAddress = Read_Text(ini, "Tunnel", "Ip", TunnelAddress);
	TunnelPort = ini.Get_Int("Tunnel", "Port", TunnelPort);

	QuickMatch = ini.Get_Bool(SETTINGS, "QuickMatch", QuickMatch);
	SkipScoreScreen = ini.Get_Bool(SETTINGS, "SkipScoreScreen", SkipScoreScreen);
	WriteStatistics = ini.Get_Bool(SETTINGS, "WriteStatistics", WriteStatistics);
	AINamesByDifficulty = ini.Get_Bool(SETTINGS, "DifficultyBasedAINames", AINamesByDifficulty);
	CoachMode = ini.Get_Bool(SETTINGS, "CoachMode", CoachMode);
	AutoSurrender = ini.Get_Bool(SETTINGS, "AutoSurrender", AutoSurrender);
	AttackNeutralUnits = ini.Get_Bool(SETTINGS, "AttackNeutralUnits", AttackNeutralUnits);
	ScrapMetal = ini.Get_Bool(SETTINGS, "ScrapMetal", ScrapMetal);
	ContinueWithoutHumans = ini.Get_Bool(SETTINGS, "ContinueWithoutHumans", ContinueWithoutHumans);
	PlayMoviesInMultiplayer = ini.Get_Bool(SETTINGS, "PlayMoviesInMultiplayer", PlayMoviesInMultiplayer);
	CustomLoadScreen = Read_Text(ini, SETTINGS, "CustomLoadScreen", CustomLoadScreen);
	DifficultyName = Read_Text(ini, SETTINGS, "DifficultyName", DifficultyName);

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
