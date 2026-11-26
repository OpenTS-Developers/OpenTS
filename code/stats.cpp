/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : WINSTUB.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Steve Tall                                                   *
 *                                                                                             *
 *                   Start Date : 05/29/1996                                                   *
 *                                                                                             *
 *                  Last Update : May 29th 1996 [ST]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Overview:                                                                                   *
 *  Internet game statistics to collect and upload to the server                               *
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int WestwoodOnline_PortNumber = 1234;

#include "always.h"

#include "addon.h"
#include "aircraft.h"
#include "airctype.h"
#include "building.h"
#include "builtype.h"
#include "dbgprint.h"
#include "gamedirs.h"
#include "getcpu.h"
#include "globals.h"
#include "house.h"
#include "infantry.h"
#include "infatype.h"
#include "misc.h"
#include "netshare.h"
#include "packet.h"
#include "rawfile.h"
#include "savemgr.h"
#include "savestream.h"
#include "scenario.h"
#include "session.h"
#include "stats.h"
#include "stimer.h"
#include "timer.h"
#include "unit.h"
#include "unittype.h"
#include "utracker.h"
#include "win.h"

#include <vector>

#define FIELD_GAME_ID							"IDNO"
#define FIELD_START_CREDITS						"CRED"
#define FIELD_BASES								"BASE"
#define FIELD_TIBERIUM							"TIBR"
#define FIELD_CRATES							"CRAT"
#define FIELD_AI_PLAYERS						"AIPL"
#define FIELD_CAPTURE_THE_FLAG					"FLAG"
#define FIELD_START_UNIT_COUNT					"UNIT"
#define FIELD_TECH_LEVEL						"TECH"
#define FIELD_SCENARIO							"SCEN"
#define FIELD_MAP_HASH							"HASH"
#define FIELD_START_TIME						"TIME"
#define FIELD_GAME_DURATION						"DURA"
#define FIELD_FRAME_RATE						"AFPS"
#define FIELD_SPEED_SETTING						"SPED"
#define FIELD_GAME_VERSION						"VERS"
#define FIELD_GAME_BUILD_DATE					"DATE"
#define FIELD_CPU_TYPE							"PROC"
#define FIELD_MEMORY							"MEMO"
#define FIELD_SHADOW_REGROWS					"SHAD"

#define FIELD_TOURNAMENT						"TRNY"

#define FIELD_GAME_SKU							"GSKU"

#define FIELD_STATS_PACKET_ID					"SPID"

#define FIELD_PLAYER_COUNT						"PLRS"
#define FIELD_WDT_TERRITORY						"WDTT"

#define FIELD_PINGS_RECEIVED					"PNGR"
#define FIELD_PINGS_SENT						"PNGS"

#define FIELD_GAME_FINISHED						"FINI"
#define FIELD_GAME_OUT_OF_SYNC					"OOSY"

// Note: These enums match those in the game results server code.
enum {
	COMPLETION_DISCONNECT = 1,
	COMPLETION_NO = 3,
	COMPLETION_QUIT = 4,
	COMPLETION_WON = 8,
	COMPLETION_LOST = 9,
};

/***************************************************************************
**	Internet specific globals
*/
bool								ConnectionLost;			//Flag that the connection to the other player was lost
bool								GameTimerInUse = false;
BasicTimerClass<SystemTimerClass>	GameTimer;
int								GameEndTime;
static int							ResumedGameTime = -1;	// Elapsed time a load left for the next match start.
void								*PacketLater = NULL;
bool								GameStatisticsPacketSent;

/*
 * These describe the match the results packet reports on. The online service that used
 * to name a game, a product and a tournament was retired, so nothing assigns them and
 * the packet goes out describing a match with no identity.
 */
int WestwoodOnline_StartTime = 0;
int WestwoodOnline_Tournament = 0;
int WestwoodOnline_GameID = 0;
int WestwoodOnline_GameSKU_TS = 0;
int WestwoodOnline_GameSKU_FS = 0;
int WestwoodOnline_GameSKU_WDT = 0;
char WestwoodOnline_UserName[16];
char WestwoodOnline_Clan1_Players[136];
char WestwoodOnline_Clan2_Players[136];
int g_PingsSent;
int g_PingsReceived;

StatisticsSettingsType StatisticsSettings;

static int Get_Unit_Count(int const *buf, int count);


// The counts go out as big-endian 32-bit values, without the zero entries at the end.
static void Add_Counts_Field(PacketClass & stats, char const * id, int const * counts, int count)
{
	int used = Get_Unit_Count(counts, count);
	std::vector<unsigned int> data(used);
	for (int index = 0; index < used; index++) {
		data[index] = htonl((unsigned int)counts[index]);
	}
	stats.Add_Field(id, (void *)data.data(), used * (int)sizeof(unsigned int));
}


static void Add_Counts_Field(PacketClass & stats, char const * id, UnitTrackerClass & tracker)
{
	Add_Counts_Field(stats, id, tracker.Get_All_Totals(), tracker.Get_Unit_Count());
}


// Replaces any earlier file, so a client never reads the tail of an older match.
static void Write_Statistics_File(void const * packet, int size)
{
	// The file object keeps the name pointer, so the path must outlive it.
	std::string const path = User_File_Write_Name("stats.dmp");
	RawFileClass file(path.c_str());
	bool const written = file.Open(FileClass::WRITE) && file.Write(packet, size) == size;
	file.Close();

	if (written) {
		DebugString("Wrote %d bytes of game results to %s\n", size, path.c_str());
	} else {
		DebugString("The game results could not be written to %s\n", path.c_str());
	}
}

/***********************************************************************************************
 * Send_Statistics_To_Server -- sends internet game statistics to the Westeood server          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    5/29/96 12:38PM ST : Created                                                             *
 *=============================================================================================*/

void Send_Statistics_Packet(void)
{
//	DebugString( "Stats: Send_Statistics_Packet() called.\n" );
#ifndef INTERNET_OFF // Denzil 5/4/98

	PacketClass		stats;
	unsigned int 	playercount = 0;
	unsigned int 	statsid = 0;
	HouseClass		*player;
	static int		packet_size;
	int				index;
	unsigned int	i;
	void			*packet;
	unsigned int	temp;

	static char	field_player_handle[5] 				= { "NAM?" };
	static char	field_player_team[5] 				= { "SID?" };
	static char	field_player_t_team[5] 				= { "TID?" };
	static char	field_player_color[5]				= { "COL?" };
	static char field_player_credits[5]				= { "CRD?" };
	static char field_player_units_left[5]			= { "UNL?" };
	static char field_player_infantry_left[5]		= { "INL?" };
	static char field_player_planes_left[5]			= { "PLL?" };
	static char field_player_buildings_left[5]		= { "BLL?" };
	static char field_player_units_bought[5] 		= { "UNB?" };
	static char field_player_infantry_bought[5] 	= { "INB?" };
	static char field_player_planes_bought[5]		= { "PLB?" };
	static char field_player_buildings_bought[5]	= { "BLB?" };
	static char field_player_units_killed[5] 		= { "UNK?" };
	static char field_player_infantry_killed[5] 	= { "INK?" };
	static char field_player_planes_killed[5]		= { "PLK?" };
	static char field_player_buildings_killed[5]	= { "BLK?" };
	static char field_player_buildings_captured[5]	= { "BLC?" };
	static char field_player_crates_found[5]		= { "CRA?" };
	static char field_player_harvested[5]			= { "HRV?" };
	static char field_player_completion[5]			= { "CMP?" };
	static char field_player_ip[5]					= { "IPA?" };
	static char field_player_clan[5]				= { "CID?" };
	static char field_player_lost_connection[5]		= { "LCN?" };
	static char field_player_start[5]				= { "BSP?" };

	DebugString("Building game results.  SawCompletion=%d\n", Session.SawGameCompletion);

	// The identifier that tells one match from another, assigned by whoever set the match up.
	stats.Add_Field(FIELD_GAME_ID, (unsigned int)StatisticsSettings.MatchID);

	/*
	**	Product SKU
	*/
	if (Session.IsWDT) {
		stats.Add_Field(FIELD_GAME_SKU, (unsigned int)WestwoodOnline_GameSKU_WDT);
	} else if (Addon_Installed(ADDON_FIRESTORM) && Addon_Enabled(ADDON_FIRESTORM)) {
		stats.Add_Field(FIELD_GAME_SKU, (unsigned int)WestwoodOnline_GameSKU_FS);
	} else {
		stats.Add_Field(FIELD_GAME_SKU, (unsigned int)WestwoodOnline_GameSKU_TS);
	}

	/*
	**	Whether or not this was a tournament game.
	*/
	stats.Add_Field(FIELD_TOURNAMENT, (unsigned int)WestwoodOnline_Tournament);

	/*
	**
	*/
	stats.Add_Field(FIELD_GAME_OUT_OF_SYNC, (unsigned char)(Session.OutOfSync == true));

	/*
	**
	*/
	stats.Add_Field(FIELD_GAME_FINISHED, (unsigned char)(Session.SawGameCompletion == true));


	/*
	**	Game duration (seconds).
	*/
	stats.Add_Field (FIELD_GAME_DURATION, (unsigned int) GameEndTime/60);

	/*
	**	Start credits.
	*/
	stats.Add_Field(FIELD_START_CREDITS, (unsigned int)Session.Options.Credits);

	/*
	**	Bases (On/Off)
	*/
	stats.Add_Field(FIELD_BASES, (unsigned char)(Session.Options.Bases != false));

	/*
	**	Tiberium (On/Off)
	*/
	stats.Add_Field(FIELD_TIBERIUM, (unsigned char)(Session.Options.BridgeDestruction != false));

	/*
	**	Crates (On/Off)
	*/
	stats.Add_Field(FIELD_CRATES, (unsigned char)(Session.Options.Goodies != false));

	/*
	**	AI Players (On/Off)
	*/
	stats.Add_Field(FIELD_AI_PLAYERS, (unsigned int)Session.Options.AIPlayers);

	/*
	**	Shadow regrowth enabled
	*/
	stats.Add_Field(FIELD_SHADOW_REGROWS, (unsigned char)(Special.IsShadowGrow));

	/*
	**	Capture the flag mode (On/Off)
	*/
	stats.Add_Field(FIELD_CAPTURE_THE_FLAG, (unsigned char)(Special.IsCaptureTheFlag));

	/*
	**	Start unit count
	*/
	stats.Add_Field(FIELD_START_UNIT_COUNT, (unsigned int)Session.Options.UnitCount);

	/*
	**	Tech level.
	*/
	stats.Add_Field(FIELD_TECH_LEVEL, (unsigned int)BuildLevel);

	/*
	**	Scenario
	*/
	if (!StatisticsSettings.MapName.empty()) {
		stats.Add_Field(FIELD_SCENARIO, StatisticsSettings.MapName.c_str());
	} else {
		stats.Add_Field(FIELD_SCENARIO, Scen->Description);
	}

	/*
	 * The map's identity in the client's map list.
	 */
	if (!StatisticsSettings.MapHash.empty()) {
		stats.Add_Field(FIELD_MAP_HASH, StatisticsSettings.MapHash.c_str());
	}

	/*
	 * Pings sent
	 */
	stats.Add_Field(FIELD_PINGS_SENT, (unsigned int)g_PingsSent);

	/*
	 * Pings received
	 */
	stats.Add_Field(FIELD_PINGS_RECEIVED, (unsigned int)g_PingsReceived);

	/*
	 * WDT territory
	 */
	if (Session.IsWDT) {
		stats.Add_Field(FIELD_WDT_TERRITORY, (unsigned int)Session.WDTTerritory);
	}

	/*
	**
	*/
	HouseClass *houses[MAX_PLAYERS];
	for (int h = 0; h < Houses.Count(); h++) {
		HouseClass *ptr = Houses[h];
		if (ptr->IsHuman && !ptr->IsObserver) {
			houses[playercount] = ptr;
			playercount++;
		}
	}

	/*
	**
	*/
	for (i = 0; i < playercount; i++) {
		for (unsigned int j = 0; j < (playercount - i) - 1; j++) {
			HouseClass *ptr1 = houses[j+1];
			HouseClass *ptr2 = houses[j];
			if (strcmpi(ptr2->IniName, ptr1->IniName) > 0) {
				houses[j] = ptr1;
				houses[j+1] = ptr2;
			}
		}
	}

	/*
	**
	*/
	for (i = 1; i < playercount; i++) {
		HouseClass *ptr = houses[i];
		if (strcmpi(ptr->IniName, WestwoodOnline_UserName) == 0) {
			HouseClass *tmp = houses[0];
			houses[0] = ptr;
			houses[i] = tmp;
		}
	}

	/*
	**
	*/
	for (i = 0; i < playercount; i++) {
		if (houses[i] == PlayerPtr) {
			statsid = i;
			break;
		}
	}

	/*
	**	Number of players
	*/
	stats.Add_Field(FIELD_PLAYER_COUNT, (unsigned int)playercount);

	/*
	 * Statistics packet ID
	 */
	stats.Add_Field(FIELD_STATS_PACKET_ID, (unsigned int)statsid);

	/*
	 * Game completion status.
	 */
	int completion[MAX_PLAYERS];
	for (i = 0; i < playercount; i++) {
		if (!Session.SawGameCompletion) {
			completion[i] = (1 << COMPLETION_NO);
		} else {
			HouseClass * hptr = houses[i];
			if (hptr->LostConnection) {
				completion[i] = (1 << COMPLETION_DISCONNECT);
			} else if (hptr->IsGiverUpper || hptr->IsResigner) {
				completion[i] = (1 << COMPLETION_LOST) | (1 << COMPLETION_QUIT);
			} else if (hptr->IsDefeated) {
				completion[i] = (1 << COMPLETION_LOST);
			} else {
				completion[i] = (1 << COMPLETION_WON);
			}
		}
	}

	/*
	**	Game start time (GMT or Pacific?)
	*/
	stats.Add_Field (FIELD_START_TIME, (int) WestwoodOnline_StartTime);

	/*
	**	Avg. frame rate.
	*/
	int divisor = GameEndTime / 60;
	if (divisor != 0) {
		stats.Add_Field (FIELD_FRAME_RATE, (unsigned int) Frame / (GameEndTime/60) );
	} else {
		stats.Add_Field (FIELD_FRAME_RATE, (unsigned int)0);
	}

	/*
	**	CPU type
	*/
	stats.Add_Field (FIELD_CPU_TYPE, (char)CPUType);

	/*
	**	Memory
	*/
	MEMORYSTATUS	mem_info;
	mem_info.dwLength=sizeof(mem_info);
	GlobalMemoryStatus(&mem_info);
	stats.Add_Field (FIELD_MEMORY, (int)mem_info.dwTotalPhys);

	/*
	**	Game speed setting.
	*/
	stats.Add_Field (FIELD_SPEED_SETTING, (char)Session.Options.GameSpeed);

	/*
	 * Game version/build date
	 */
	char	version[128];
	snprintf(version, sizeof(version), "V%s", VerNum.Version_Name() );
	stats.Add_Field (FIELD_GAME_VERSION, (char*)version);

	char path_to_exe[280];
	FILETIME write_time;		//File time is 64 bits

	GetModuleFileName (ProgramInstance, path_to_exe, sizeof(path_to_exe));

	// The packet carries each half byte-swapped with the low half still leading.
	WIN32_FILE_ATTRIBUTE_DATA attributes;

	if (GetFileAttributesEx (path_to_exe, GetFileExInfoStandard, &attributes)) {
		write_time = attributes.ftLastWriteTime;
		write_time.dwLowDateTime = htonl (write_time.dwLowDateTime);
		write_time.dwHighDateTime = htonl (write_time.dwHighDateTime);
		stats.Add_Field (FIELD_GAME_BUILD_DATE, (void*)&write_time, sizeof (write_time));
	}

	/*
	**	Build the player specific statistics
	**
	*/
	for (unsigned house = 0 ; house < playercount; house++){
		player = houses[(HousesType) (house)];

		/*
		**	Player handle.
		*/
		field_player_handle[3] = '0' + (char)house;
		stats.Add_Field (field_player_handle, player->IniName.c_str());

		/*
		 * Player IP Address.
		 */
		temp = player->IPAddress;
		field_player_ip[3] = '0' + (char)house;
		stats.Add_Field (field_player_ip, (unsigned int)(temp));

		/*
		 * Player clan.
		 */
		temp = player->SquadID;
		field_player_clan[3] = '0' + (char)house;
		stats.Add_Field (field_player_clan, (unsigned int)(temp));

		/*
		**	Player team. (NOD or GDI)
		*/
		field_player_team[3] = '0' + (char)house;
		stats.Add_Field (field_player_team, (unsigned int)player->ActLike);

		/*
		 * Player team.
		 */
		field_player_t_team[3] = '0' + (char)house;
		temp = house;
		if (WestwoodOnline_Tournament == 2) {
			temp = player->SquadID;
		}
		stats.Add_Field (field_player_t_team, (unsigned int)(temp));

		/*
		 * Player completion.
		 */
		field_player_completion[3] = '0' + (char)house;
		stats.Add_Field (field_player_completion, (unsigned int)(completion[house]));

		/*
		 * LCN
		 */
		field_player_lost_connection[3] = '0' + (char)house;
		bool lcn = false;
		if (player->LostConnection && !player->IsResigner) {
			lcn = true;
		}
		stats.Add_Field (field_player_lost_connection, (unsigned char)(lcn));

		/*
		**	Player color
		*/
		field_player_color[3] = '0' + (char)house;
		stats.Add_Field (field_player_color, (unsigned int) (player->Scheme));

		/*
		 * Player start position.
		 */
		field_player_start[3] = '0' + (char)house;
		stats.Add_Field (field_player_start, (int)player->SpawnWaypoint);

		/*
		**	Player end credits.
		*/
		field_player_credits[3] = '0' + (char)house;
		stats.Add_Field (field_player_credits, (int)player->Available_Money());

		/*
		**	Number of each unit/building type built
		*/
		field_player_infantry_bought[3] = '0' + (char)house;
		field_player_units_bought[3] = '0' + (char)house;
		field_player_planes_bought[3] = '0' + (char)house;
		field_player_buildings_bought[3] = '0' + (char)house;

		Add_Counts_Field(stats, field_player_infantry_bought, *player->InfantryTotals);
		Add_Counts_Field(stats, field_player_units_bought, *player->UnitTotals);
		Add_Counts_Field(stats, field_player_planes_bought, *player->AircraftTotals);
		Add_Counts_Field(stats, field_player_buildings_bought, *player->BuildingTotals);

		/*
		**	Number of units remaining to player
		*/
		std::vector<int> units_left(UnitTypes.Count());
		std::vector<int> infantry_left(InfantryTypes.Count());
		std::vector<int> aircraft_left(AircraftTypes.Count());
		std::vector<int> buildings_left(BuildingTypes.Count());

		for (index = 0; index < Units.Count(); index++) {
			UnitClass const * unit = Units[index];
			if (player == unit->House) {
				units_left[unit->Class->HeapID]++;
			}
		}

		for (index = 0; index < Infantry.Count(); index++) {
			InfantryClass const * infantry = Infantry[index];
			if (player == infantry->House && !infantry->Class->IsCivilian) {
				infantry_left[infantry->Class->HeapID]++;
			}
		}

		for (index = 0; index < Aircraft.Count(); index++) {
			AircraftClass const * aircraft = Aircraft[index];
			if (player == aircraft->House) {
				aircraft_left[aircraft->Class->HeapID]++;
			}
		}

		for (index = 0; index < Buildings.Count(); index++) {
			BuildingClass const * building = Buildings[index];
			if (player == building->House) {
				buildings_left[building->Class->HeapID]++;
			}
		}

		field_player_infantry_left[3] = '0' + (char)house;
		field_player_units_left[3] = '0' + (char)house;
		field_player_planes_left[3] = '0' + (char)house;
		field_player_buildings_left[3] = '0' + (char)house;

		Add_Counts_Field(stats, field_player_infantry_left, infantry_left.data(), (int)infantry_left.size());
		Add_Counts_Field(stats, field_player_units_left, units_left.data(), (int)units_left.size());
		Add_Counts_Field(stats, field_player_planes_left, aircraft_left.data(), (int)aircraft_left.size());
		Add_Counts_Field(stats, field_player_buildings_left, buildings_left.data(), (int)buildings_left.size());

		/*
		**	Number of enemy units/buildings of each type destroyed.
		*/
		field_player_infantry_killed[3] = '0' + (char)house;
		field_player_units_killed[3] = '0' + (char)house;
		field_player_planes_killed[3] = '0' + (char)house;
		field_player_buildings_killed[3] = '0' + (char)house;

		Add_Counts_Field(stats, field_player_infantry_killed, *player->DestroyedInfantry);
		Add_Counts_Field(stats, field_player_units_killed, *player->DestroyedUnits);
		Add_Counts_Field(stats, field_player_planes_killed, *player->DestroyedAircraft);
		Add_Counts_Field(stats, field_player_buildings_killed, *player->DestroyedBuildings);

		/*
		**	Number and type of enemy buildings captured
		*/
		field_player_buildings_captured[3] = '0' + (char)house;
		Add_Counts_Field(stats, field_player_buildings_captured, *player->CapturedBuildings);

		/*
		**	Number of crates discovered and their contents
		*/
		field_player_crates_found[3] = '0' + (char)house;
		Add_Counts_Field(stats, field_player_crates_found, *player->TotalCrates);

		/*
		**	Amount of tiberium turned into credits
		*/
		field_player_harvested[3] = '0' + (char)house;
		stats.Add_Field (field_player_harvested, (unsigned int) player->HarvestedCredits);
	}

	/*
	**	Create the comms packet to be sent
	*/
	packet = stats.Create_Comms_Packet(packet_size);

	// The header holds the size in 16 bits, so a larger packet could not be read back.
	if (packet_size > 0xFFFF) {
		DebugString("The %d byte game results packet is too large to write.\n", packet_size);
	} else if (StatisticsSettings.IsWritten && PlayerPtr != NULL && !PlayerPtr->IsObserver) {
		Write_Statistics_File(packet, packet_size);
	} else {
		DebugString("Built a %d byte game results packet that nothing asked to be written.\n", packet_size);
	}

	/*
	**	Tidy up
	*/
	delete [] packet;

	GameStatisticsPacketSent = true;
#endif // INTERNET_OFF
}


/// <summary>
/// Determines how much of a unit tracker's totals is worth sending.
/// This routine is used by the statistics packet builder to trim the unused tail off a
/// tracker's totals, so that only the meaningful head of the array is transmitted to the
/// server.
/// </summary>
/// <param name="buf">The tracker totals to examine.</param>
/// <param name="count">The number of entries held in the tracker totals.</param>
/// <returns>Returns with the entry count up to and including the last non-zero total.
/// Zero is returned if the tracker recorded nothing at all.</returns>
static int Get_Unit_Count(int const *buf, int count)
{
	int n = 0;
	for (int i = 0; i < count; i++) {
		if (buf[i] != 0) {
			n = i + 1;
		}
	}
	return(n);
}


/// <summary>
/// Starts the game duration timer as the game begins. A match resumed from a save starts from
/// the time the save recorded.
/// </summary>
void Register_Game_Start_Time(void)
{
	GameTimer = ResumedGameTime >= 0 ? ResumedGameTime : 0;
	ResumedGameTime = -1;
	GameTimerInUse = true;
}


/// <summary>
/// Stops the game duration timer.
/// This routine is called as the game ends and records how long the game ran, so that the
/// statistics packet can report the duration and the average frame rate.
/// </summary>
void Register_Game_End_Time(void)
{
	GameEndTime = GameTimer;
	GameTimerInUse = false;
}


/// <summary>
/// Saves or restores how long the match has run, so that a resumed match reports its whole
/// duration. A load in place of a running match sets the timer at once; any other load of an
/// internet game hands the time to the next Register_Game_Start_Time.
/// </summary>
void Serialize_Game_Time(SaveStreamClass & stream)
{
	int elapsed = GameTimerInUse ? (int)GameTimer : 0;
	stream.Serialize(elapsed);

	if (stream.Is_Loading() && !stream.Was_Error()) {
		if (SaveManager.Multiplayer_Load_Is_In_Progress()) {
			GameTimer = elapsed;
		} else if (Session.Type == GAME_INTERNET) {
			ResumedGameTime = elapsed;
		}
	}
}
