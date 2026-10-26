/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "spawnerconfig.h"
#include "win.h"

#include "crc.h"
#include "diff.hh"
#include "ini.h"
#include "utf8.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <vector>


namespace {

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
	return(std::clamp(ticks, SpawnerConfigClass::TIMEOUT_MIN, SpawnerConfigClass::TIMEOUT_MAX));
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


/// <summary>Is this a dotted IPv4 address other than 0.0.0.0?</summary>
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


/// <summary>Accepts a tunnel number as the client writes it: nonzero, within sixteen bits either side of zero.</summary>
static bool Is_Tunnel_Number(int value)
{
	return(value != 0 && value >= -65535 && value <= 65535);
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
/// Reads the match's seats and sorts them into the order their houses are created in, which
/// is the order everything naming a seat by position means. A seat is human because the file
/// wrote a section for it.
/// </summary>
void SpawnerConfigClass::Read_Slots(INIClass const & ini)
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
/// The identity of the match this file asks for. It covers every value the course of the
/// match depends on and nothing merely displayed, so two machines handed the same match
/// agree. The version comes first: one file read two ways is not one match.
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
	crc(AutoDeployMCV);
	crc(Seed);
	crc(TechLevel);
	crc(Firestorm);
	crc(AttackNeutralUnits);
	crc(ScrapMetal);
	crc(SharedControl);
	crc(CoachMode);
	crc(PlayMoviesInMultiplayer);

	crc(AutoSurrender);

	// ConnTimeout and ReconnectTimeout are each machine's own, so they are left out on purpose.

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
/// The difficulty a seat is played at. A client may ask for an easier opponent than the game
/// has, and any such request comes to the easiest one it does have.
/// </summary>
/// <returns>The difficulty to play the seat at, or -1 for the session default.</returns>
int SpawnerConfigClass::Playable_Handicap(int asked)
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
/// Judges whether this reading describes a game that can be played. The country and color
/// counts are passed in because they come from the rules, which only a running game holds.
/// </summary>
/// <param name="fault">Where to leave the sentence describing the first fault found.</param>
/// <returns>bool; Can the game this file describes be played?</returns>
bool SpawnerConfigClass::Is_Playable(int countries, int colors, std::string & fault) const
{
	// A resumed network match is seated from the file, so the same rules hold.
	LaunchType kind = Launch_Type();
	bool multiplayer = kind == LaunchType::Multiplayer ||
		(kind == LaunchType::Resume && HumanCount > 1);

	if (kind != LaunchType::Campaign && HumanCount == 0) {
		return(Fault(fault, "SPAWN.INI has no [Settings] or [OtherN] section, so it seats nobody."));
	}

	if (AIDifficulty < 0 || AIDifficulty >= DIFF_COUNT) {
		return(Fault(fault, "SPAWN.INI sets AIDifficulty=%d; it must be 0 to %d.",
			AIDifficulty, DIFF_COUNT - 1));
	}

	int free_seats = SLOT_COUNT - HumanCount;
	if (AIPlayers < 0 || AIPlayers > free_seats) {
		return(Fault(fault, "SPAWN.INI sets AIPlayers=%d; it must be 0 to %d, the seats no person holds.",
			AIPlayers, free_seats));
	}

	// These reach the network as 16-bit values. A version 2 tunnel numbers machines across the
	// whole signed 16-bit range, so about half of its numbers arrive negative.
	if (multiplayer) {
		if (TunnelPort != 0) {
			if (TunnelPort < 1 || TunnelPort > 65535) {
				return(Fault(fault, "SPAWN.INI sets [Tunnel] Port=%d; it must be 1 to 65535, or 0 for no tunnel.",
					TunnelPort));
			}

			if (!Is_Address(TunnelAddress)) {
				return(Fault(fault, "SPAWN.INI sets [Tunnel] Ip=%s; it must be a dotted IPv4 address other than 0.0.0.0.",
					TunnelAddress.c_str()));
			}

			if (!Is_Tunnel_Number(TunnelId)) {
				return(Fault(fault, "SPAWN.INI sets [Settings] Port=%d; through a tunnel it must be a nonzero number from -65535 to 65535.",
					TunnelId));
			}
		} else if (ListenPort < 1 || ListenPort > 65535) {
			return(Fault(fault, "SPAWN.INI sets [Settings] Port=%d; it must be 1 to 65535.", ListenPort));
		}
	}

	for (int index = 0; index < SLOT_COUNT; index++) {
		SlotType const & slot = Slots[index];
		if (slot.Occupancy == OccupancyType::Empty) {
			continue;
		}

		bool human = slot.Occupancy == OccupancyType::Human;

		std::string section = Seat_Section(slot.FilePosition);
		int entry = slot.FilePosition + 1;

		// A computer seat may leave its country and color to the game; a person's seat names both.
		if ((human || slot.Country != -1) && (slot.Country < 0 || slot.Country >= countries)) {
			if (human) {
				return(Fault(fault, "SPAWN.INI sets [%s] Side=%d, but the rules have %d countries, numbered from 0.",
					section.c_str(), slot.Country, countries));
			}
			return(Fault(fault, "SPAWN.INI sets [HouseCountries] Multi%d=%d, but the rules have %d countries, numbered from 0; -1 lets the game choose.",
				entry, slot.Country, countries));
		}

		// A computer seat may leave its color to the game; only a color the rules lack refuses.
		if ((human || slot.Color != -1) && (slot.Color < 0 || slot.Color >= colors)) {
			if (human) {
				return(Fault(fault, "SPAWN.INI sets [%s] Color=%d, but the rules have %d colors, numbered from 0.",
					section.c_str(), slot.Color, colors));
			}
			return(Fault(fault, "SPAWN.INI sets [HouseColors] Multi%d=%d, but the rules have %d colors, numbered from 0; -1 lets the game choose.",
				entry, slot.Color, colors));
		}

		if (slot.Handicap < -1 || slot.Handicap > 6) {
			return(Fault(fault, "SPAWN.INI sets [HouseHandicaps] Multi%d=%d; it must be -1 to 6.",
				entry, slot.Handicap));
		}

		for (int ally : slot.Alliances) {
			if (ally < -1 || ally >= SLOT_COUNT) {
				return(Fault(fault, "[Multi%d_Alliances] in SPAWN.INI names seat %d; seats are numbered 0 to %d.",
					index + 1, ally, SLOT_COUNT - 1));
			}
			if (ally >= 0 && Slots[ally].Occupancy == OccupancyType::Empty) {
				return(Fault(fault, "[Multi%d_Alliances] in SPAWN.INI names seat %d, which nobody holds.",
					index + 1, ally));
			}
		}

		// The client's seat order cannot be rebuilt when two people share a name or a color, so
		// they would swap start positions and alliances.
		if (human && multiplayer) {
			if (slot.Name.empty()) {
				return(Fault(fault, "SPAWN.INI gives [%s] no Name; a network game needs every player named.",
					section.c_str()));
			}

			for (int other = 0; other < index; other++) {
				if (Slots[other].Occupancy != OccupancyType::Human) {
					continue;
				}

				std::string other_section = Seat_Section(Slots[other].FilePosition);

				if (_stricmp(Slots[other].Name.c_str(), slot.Name.c_str()) == 0) {
					return(Fault(fault, "SPAWN.INI gives [%s] and [%s] the same Name, %s; a network game needs a different name for each player.",
						other_section.c_str(), section.c_str(), slot.Name.c_str()));
				}

				if (Slots[other].Color == slot.Color) {
					return(Fault(fault, "SPAWN.INI gives [%s] and [%s] the same Color, %d; a network game needs a different color for each player.",
						other_section.c_str(), section.c_str(), slot.Color));
				}
			}

			// Through a tunnel the port carries the tunnel number, so every other seat needs one.
			if (index != LocalSlot) {
				if (TunnelPort != 0 && !Is_Tunnel_Number(slot.Port)) {
					return(Fault(fault, "SPAWN.INI sets [%s] Port=%d; through a tunnel it must be a nonzero number from -65535 to 65535.",
						section.c_str(), slot.Port));
				}

				if (TunnelPort == 0 && (slot.Port < 1 || slot.Port > 65535)) {
					return(Fault(fault, "SPAWN.INI sets [%s] Port=%d; it must be 1 to 65535.",
						section.c_str(), slot.Port));
				}

				if (TunnelPort == 0 && !Is_Address(slot.Address)) {
					return(Fault(fault, "SPAWN.INI sets [%s] Ip=%s; it must be a dotted IPv4 address other than 0.0.0.0.",
						section.c_str(), slot.Address.c_str()));
				}
			}
		}
	}

	return(true);
}


/// <summary>
/// Reads what the client asked the game to launch. Reading cannot fail: an unwritten key has
/// a settled meaning, an unusable value keeps it, and an unknown key is passed over.
/// </summary>
void SpawnerConfigClass::Read_INI(INIClass const & ini)
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
