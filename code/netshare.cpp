/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "netshare.h"

#include "_ui.h"
#include "ui/screens/msgbox/uimsgbox.h"
#include "ui/screens/scenario/uiscenario.h"

#include "_rules.h"
#include "conquer.h"
#include "data.h"
#include "dbgprint.h"
#include "globals.h"
#include "goptions.h"
#include "ipxmgr.h"
#include "language/language.h"
#include "lzopipe.h"
#include "lzostraw.h"
#include "mapgen.h"
#include "msgbox.h"
#include "netdlg.h"
#include "netdlg2.h"
#include "newmenu.h"
#include "rules.h"
#include "scenario.h"
#include "sendfile.h"
#include "session.h"
#include "stimer.h"
#include "utf8.h"
#include "wdtnet.h"
#include "worlddom.h"
#include "wstring.h"
#include "xpipe.h"
#include "xstraw.h"

#include <algorithm>
#include <ctime>


const COLORREF ColorSystem     = RGB(255, 255, 255)|(255<<24);  /// 0xFFFFFFFF
const COLORREF ColorUser       = RGB(240, 240, 0);              /// 0x0000F0F0
const COLORREF ColorPriv       = RGB(128, 128, 255);            /// 0x00FF8080
const COLORREF ColorPrivAction = RGB(255, 0, 255);              /// 0x00FF00FF
const COLORREF ColorAction     = RGB(255, 80, 48);              /// 0x003050FF
const COLORREF ColorOp         = RGB(0, 255, 255);              /// 0x00FFFF00
const COLORREF ColorPaged      = RGB(255, 255, 255);            /// 0x00FFFFFF
const COLORREF ColorMe         = RGB(255, 255, 64);             /// 0x0040FFFF
const COLORREF ColorNoJoin     = RGB(128, 128, 128);            /// 0x00808080

/*
 * If the host is forcing a color reassignment, then this flag will be true. It is set when
 * the host accepts the game options and cleared once the assignment has been processed, and
 * it suppresses the "color in use" message that would otherwise appear.
 */
int IsColorChangePending;

COLORREF PlayerColorTable[MAX_PLAYERS] = {
	RGB(255, 223, 94),	/// 0x005EDFFF
	RGB(255, 26, 20),	/// 0x00141AFF
	RGB(39, 60, 179),	/// 0x00B33C27
	RGB(11, 148, 11),	/// 0x000B940B
	RGB(218, 137, 26),	/// 0x001A89DA
	RGB(20, 177, 255),	/// 0x00FFB114
	RGB(185, 20, 255),	/// 0x00FF14B9
	RGB(255, 70, 173)	/// 0x00AD46FF
};


bool IsRandomMap = true;

MapPreviewClass *MultiplayerMapPreview;


/// <summary>
/// Computes a hash value for a string.
/// This routine supplies the bucket value for the dictionaries that the multiplayer dialogs
/// key by name.
/// </summary>
/// <param name="string">The string to hash.</param>
/// <returns>Returns with the hash value of the string.</returns>
unsigned int Wstring_Hash(Wstring & string)
{
	unsigned int hash = 0;

	hash = string.length();

	for (unsigned int i = 0; i < string.length(); i++) {
		hash += *(string.get() + i);
		hash += i;
		hash = (hash << 8) ^ (hash >> 24);
	}
	return(hash);
}


static std::vector<NetChatLineType> _ChatLog;


std::vector<NetChatLineType> const & Net2_Chat_Log(void)
{
	return(_ChatLog);
}


/// <summary>
/// Empties the lobby chat. The dialog that showed it is going away with its messages.
/// </summary>
void Net2_Clear_Chat_Log(void)
{
	_ChatLog.clear();
}


/// <summary>
/// Adds a formatted chat message to the lobby's chat log, which is what shows it.
/// </summary>
/// <param name="color">Color to display the message in, or -1 for the default.</param>
/// <param name="fmt">Printf style format string for the message.</param>
void __cdecl PMessagePrintf(int color, const char * fmt, ...)
{
	va_list va;
	static char buffer[1024];
	memset(buffer, 0, sizeof(buffer));

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	buffer[UTF8::Boundary_Before(buffer, sizeof(buffer) - 1)] = '\0';

	if (_ChatLog.size() >= 500) {
		_ChatLog.erase(_ChatLog.begin());
	}
	_ChatLog.push_back(NetChatLineType{color, buffer});
}


/// <summary>
/// Counts the human teams still in the game.
/// A group of allied players counts as one team. This routine is used to recognize the point
/// where only one side is left standing, which is why the house being examined is left out
/// of the tally.
/// </summary>
/// <param name="house">The house to leave out of the count.</param>
/// <returns>Returns with the number of opposing teams still alive.</returns>
int CountAliveTeams(HouseClass * house)
{
	int count = 0;
	for (int i = 0; i < Houses.Count(); i++) {
		HouseClass * house1 = Houses[i];
		if (house1 != NULL && !house1->IsDefeated && house1->IsHuman && house1 != house) {
			bool has_ally = false;
			for (int j = 0; j < Houses.Count(); j++) {
				if (j != i) {
					HouseClass * house2 = Houses[j];
					if (house2 != NULL && !house2->IsDefeated && house2->IsHuman) {
						if (house2 != house && house1->Is_Ally(Houses[j]) && house2->Is_Ally(house1)) {
							if (j > i && !has_ally) {
								count++;
							}
							has_ally = true;
						}
					}
				}
			}
			if (!has_ally) {
				count++;
			}
		}
	}
	return(count);
}


/// <summary>
/// Puts a message box up over the lobby and waits for an answer. The supplied callback is
/// polled while it is on screen so that network traffic keeps flowing.
/// </summary>
void Net2EncodeGameopt(char *out, int size);


/// <summary>
/// Broadcasts the host's game options if they need broadcasting.
/// This routine is called from the game setup dialog's idle loop. It only puts a packet on
/// the wire when an option has actually changed or when the caller insists, so that dragging
/// a slider about does not flood the other players.
/// </summary>
/// <param name="force">Should the options be sent even though nothing has changed?</param>
/// <param name="now">Should the send happen immediately rather than waiting for the next
/// opportunity?</param>
void PumpGameopts(bool force, bool now)
{
	static int _last_pump_time = 0;
	static bool _need_to_pump = false;

	static int _last_unit_count = -1;
	static int _last_tech_level = -1;
	static int _last_credits = -1;
	static int _last_ai_players = -1;
	static int _last_ai_difficulty = 1;
	static int _last_game_speed = -1;
	static bool _last_mcv_redeploy = false;
	static bool _last_allies_allowed = false;
	static bool _last_harvester_truce = false;
	static bool _last_capture_the_flag = false;
	static bool _last_fog_of_war = false;
	static bool _last_bases = false;
	static bool _last_bridge_destruction = false;
	static bool _last_goodies = false;
	static bool _last_short_game = false;
	static bool _last_crap_engineers = false;

	static char _last_scenario_description[DESCRIP_MAX];
	static char _last_scenario_file_name[14];
	static char _last_scenario_digest[33];
	static unsigned int _last_scenario_file_length = -1;
	static bool _last_scenario_is_official = true;

	bool do_pump = false;
	bool it_is_time = false;

	/// if we didn't pump this second, we can consider pumping
	if (time(NULL) - _last_pump_time != 0) {
		it_is_time = true;
	}

	/// if it isn't time to pump, but we were told to pump, remember to do
	/// so when time comes, regardless of if anything changed
	if (!it_is_time) {
		if (force) {
			_need_to_pump = true;
		}
	}

	/// if it is time to pump, or we were told to pump, do so
	if (it_is_time || now) {
		if (_need_to_pump) {
			force = true;
		}
		_need_to_pump = false;

		if (time(NULL) - _last_pump_time >= 2) {
			do_pump = true;
		}

		if (force == true) {
			do_pump = true;
		}

		if (_last_unit_count != Session.Options.UnitCount) do_pump = true;
		if (_last_tech_level != BuildLevel) do_pump = true;
		if (_last_credits != Session.Options.Credits) do_pump = true;
		if (_last_ai_players != Session.Options.AIPlayers) do_pump = true;
		if (_last_bases != Session.Options.Bases) do_pump = true;
		if (_last_bridge_destruction != Session.Options.BridgeDestruction) do_pump = true;
		if (_last_goodies != Session.Options.Goodies) do_pump = true;
		if (_last_short_game != Session.Options.ShortGame) do_pump = true;
		if (_last_game_speed != Session.Options.GameSpeed) do_pump = true;
		if (_last_crap_engineers != Session.Options.CrapEngineers) do_pump = true;
		if (_last_mcv_redeploy != Session.Options.MCVRedeploy) do_pump = true;
		if (_last_allies_allowed != Session.Options.AlliesAllowed) do_pump = true;
		if (_last_harvester_truce != Session.Options.HarvTruce) do_pump = true;
		if (_last_capture_the_flag != Session.Options.CTF) do_pump = true;
		if (_last_fog_of_war != Session.Options.FogOfWar) do_pump = true;
		if (_last_ai_difficulty != Session.Options.AIDifficulty) do_pump = true;

		_last_scenario_description[43] = 0;
		if (stricmp(_last_scenario_description, Session.Options.ScenarioDescription)) do_pump = true;

		_last_scenario_file_name[13] = 0;
		if (stricmp(_last_scenario_file_name, Session.ScenarioFileName)) do_pump = true;

		_last_scenario_digest[32] = 0;
		if (stricmp(_last_scenario_digest, Session.ScenarioDigest)) do_pump = true;

		if (_last_scenario_file_length != Session.ScenarioFileLength) do_pump = true;
		if (_last_scenario_is_official != Session.ScenarioIsOfficial) do_pump = true;

		if (do_pump) {
			_last_pump_time = time(0);
			_last_unit_count = Session.Options.UnitCount;
			_last_tech_level = BuildLevel;
			_last_credits = Session.Options.Credits;
			_last_ai_players = Session.Options.AIPlayers;
			_last_ai_difficulty = Session.Options.AIDifficulty;
			_last_bases = Session.Options.Bases;
			_last_bridge_destruction = Session.Options.BridgeDestruction;
			_last_goodies = Session.Options.Goodies;
			_last_mcv_redeploy = Session.Options.MCVRedeploy;
			_last_short_game = Session.Options.ShortGame;
			_last_game_speed = Session.Options.GameSpeed;
			_last_crap_engineers = Session.Options.CrapEngineers;
			_last_allies_allowed = Session.Options.AlliesAllowed;
			_last_harvester_truce = Session.Options.HarvTruce;
			_last_capture_the_flag = Session.Options.CTF;
			_last_fog_of_war = Session.Options.FogOfWar;
			strcpy(_last_scenario_description, Session.Options.ScenarioDescription);
			strcpy(_last_scenario_digest, Session.ScenarioDigest);
			strcpy(_last_scenario_file_name, Session.ScenarioFileName);
			_last_scenario_file_length = Session.ScenarioFileLength;
			_last_scenario_is_official = Session.ScenarioIsOfficial;

			char buffer[MAX_GAMEOPT_LENGTH];
			memset(buffer, '\0', sizeof(buffer));

			Net2EncodeGameopt(buffer, sizeof(buffer));

			SendPublicGameopts(buffer);
		}
	}
}


/// <summary>
/// Sends an encoded game options string to every player.
/// </summary>
/// <param name="options">The encoded option string to send.</param>
void SendPublicGameopts(char const * options)
{
	GlobalPacketType packet;
	memset(&packet, 0, sizeof(packet));
	packet.Command = NET_PUB_GAMEOPT;
	strcpy(packet.Name, Session.Handle);
	UTF8::Copy(packet.Options.Buf, sizeof(packet.Options.Buf), options);
	packet.Options.Color = Session.ColorIdx;
	packet.Options.NameCRC = Compute_Name_CRC(Session.GameName);
	for (int i = 1; i < Session.Players.Count(); i++) {
		DebugString("Sending public game options to %s\n", Session.Players[i]->Name);
		Ipx.Send_Global_Message(&packet, sizeof(packet), 1, &Session.Players[i]->Address);
		Call_Back();
	}
}


/// <summary>
/// Sends an encoded game options string to one player.
/// </summary>
/// <param name="player">Name of the player to send the options to.</param>
/// <param name="options">The encoded option string to send.</param>
void SendPrivateGameopts(char const * player, char const * options)
{
	memset(&Session.GPacket, 0, sizeof(Session.GPacket));
	Session.GPacket.Command = NET_PRIV_GAMEOPT;
	strcpy(Session.GPacket.Name, Session.Handle);
	UTF8::Copy(Session.GPacket.Options.Buf, sizeof(Session.GPacket.Options.Buf), options);
	Session.GPacket.Options.Color = Session.ColorIdx;
	Session.GPacket.Options.NameCRC = Compute_Name_CRC(Session.GameName);
	for (int i = 1; i < Session.Players.Count(); i++) {
		if (stricmp(Session.Players[i]->Name, player) == 0) {
			DebugString("Sending private game options to %s\n", Session.Players[i]->Name);
			Ipx.Send_Global_Message(&Session.GPacket, sizeof(Session.GPacket), 1, &Session.Players[i]->Address);
			Call_Back();
		}
	}
}


/// <summary>
/// Decodes a public game options string sent by the host.
/// This routine applies the host's settings to the local session and refreshes the game
/// options dialog to suit. When something material has changed, everybody's accept flag is
/// cleared so that the players have to agree to the new terms. A player whose preferred
/// color has been taken is quietly moved to the one the host assigned.
/// </summary>
/// <param name="options">The encoded option string as received.</param>
/// <param name="name">Name of the player the options came from.</param>
/// <returns>bool; Did the scenario change?</returns>
bool DecodePubGameopt(char * options, char * name)
{
	static int _last_unit_count = -1;
	static int _last_tech_level = -1;
	static int _last_credits = -1;
	static int _last_ai_players = -1;
	static int _last_ai_difficulty = 1;
	static int _last_game_speed = -1;
	static bool _last_mcv_redeploy = false;
	static bool _last_allies_allowed = false;
	static bool _last_harvester_truce = false;
	static bool _last_capture_the_flag = false;
	static bool _last_fog_of_war = false;
	static bool _last_bases = false;
	static bool _last_bridge_destruction = false;
	static bool _last_goodies = false;
	static bool _last_short_game = false;
	static bool _last_crap_engineers = false;

	bool do_decode = false;

	char *string = strdup(options);
	char *token = string;
	if (token[0] == 'A') {
		int status = atol(&token[1]);
		Net2SetAccept(name, status);
		return(false);
	}

	DebugString("Decoding game options %s\n", options);

	token = strtok(token, ",");
	if (token != NULL) {
		Session.Options.UnitCount = atol(token);
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		BuildLevel = atol(token);
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		Session.Options.Credits = atol(token);
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		Session.Options.FogOfWar = atol(token) != false;
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		Session.Options.BridgeDestruction = atoi(token) != false;
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		Session.Options.Goodies = atoi(token) != false;
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		Session.Options.MCVRedeploy = atoi(token) != false;
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		Session.Options.AlliesAllowed = atoi(token) != false;
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		Session.Options.HarvTruce = atoi(token) != false;
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		Session.Options.Bases = atoi(token) != false;
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		Session.Options.CTF = atoi(token) != false;
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		Seed = atol(token);
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		Options.GameSpeed = atol(token);
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		Session.Options.AIPlayers = atol(token);
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		Session.Options.AIDifficulty = (DiffType)atol(token);
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		Session.Options.ShortGame = atol(token) != false;;
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		Session.Options.CrapEngineers = atol(token) != false;;
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		Session.Options.GameSpeed = atol(token);
	}

	bool same_scenario = true;
	char * scenario_description = strtok(NULL, ",");

	token = strtok(NULL, ",");
	bool old_official = Session.ScenarioIsOfficial;
	if (token != NULL) {
		Session.ScenarioIsOfficial = atoi(token) != false;;
	}

	token = strtok(NULL, ",");
	unsigned int old_scenario_file_length = Session.ScenarioFileLength;
	if (token != NULL) {
		Session.ScenarioFileLength = atoi(token);
	}

	token = strtok(NULL, ",");
	if (token != NULL) {
		if (stricmp(Session.ScenarioFileName, token) != 0) {
			same_scenario = false;
		}
		strncpy(Session.ScenarioFileName, token, sizeof(Session.ScenarioFileName));
		strcpy(Scen->ScenarioName, Session.ScenarioFileName);
	}

	char * digest = strtok(NULL, ":");
	if (digest != NULL) {
		if (strcmp(Session.ScenarioDigest, digest) != 0) {
			same_scenario = false;
		}
		strncpy(Session.ScenarioDigest, digest, sizeof(Session.ScenarioDigest)-1);
	}

	if (!same_scenario || strlen(Session.Options.ScenarioDescription) == 0) {
		same_scenario = false;
		bool found = false;
		for (int i = 0; i < Session.Scenarios.Count(); i++) {
			MultiMission * scenario = Session.Scenarios[i];
			if (stricmp(Session.ScenarioFileName, scenario->Get_Filename()) == 0 && stricmp(Session.ScenarioFileName, RANDOM_MAP_FILE_NAME) != 0) {
				strcpy(Session.Options.ScenarioDescription, scenario->Description());
				found = true;
				break;
			}
		}
		if (!found && scenario_description != NULL) {
			strcpy(Session.Options.ScenarioDescription, scenario_description);
		}
		if (stricmp(Session.ScenarioFileName, RANDOM_MAP_FILE_NAME) == 0) {
			strcpy(Session.Options.ScenarioDescription, Fetch_String(TXT_RANDOM_MAP_DESCRIPTION));
		}
	}

	Scen->Scenario = -1;
	Frame = 0;
	Session.CommProtocol = DEFAULT_COMM_PROTOCOL;

	if (old_official != Session.ScenarioIsOfficial || old_scenario_file_length != Session.ScenarioFileLength) {
		same_scenario = false;
	}

	if (!same_scenario) {
		DebugString("Not same scenario...");
		Update_Network_Dialog_Preview();
	}

	if (digest == NULL) {
		PMessagePrintf(-1, "Gameopt parse failed");
	}

	for (int i = 0; i < MAX_PLAYERS; i++) {
		token = strtok(NULL, ",:");
		if (token == NULL) break;
		char * handle = token;

		token = strtok(NULL, ",:");
		if (token == NULL) break;
		int house = atol(token);

		token = strtok(NULL, ",:");
		if (token == NULL) break;
		int color = atol(token);

		if (stricmp(handle, Session.Handle) == 0 && color != Session.PrefColor && !IsColorChangePending) {
			PMessagePrintf(-1, Fetch_String(TXT_COLOR_IN_USE));
			Session.PrefColor = color;
		}

		if (stricmp(handle, Session.Handle) == 0) {
			IsColorChangePending = false;
		}

		if (Net2SetHouseAndColor(handle, house, color)) {
			do_decode = true;
		}
	}

	free(string);

	if (_last_unit_count != Session.Options.UnitCount) do_decode = true;
	if (_last_tech_level != BuildLevel) do_decode = true;
	if (_last_credits != Session.Options.Credits) do_decode = true;
	if (_last_ai_players != Session.Options.AIPlayers) do_decode = true;
	if (_last_ai_difficulty != Session.Options.AIDifficulty) do_decode = true;
	if (_last_bases != Session.Options.Bases) do_decode = true;
	if (_last_bridge_destruction != Session.Options.BridgeDestruction) do_decode = true;
	if (_last_goodies != Session.Options.Goodies) do_decode = true;
	if (_last_mcv_redeploy != Session.Options.MCVRedeploy) do_decode = true;
	if (_last_allies_allowed != Session.Options.AlliesAllowed) do_decode = true;
	if (_last_harvester_truce != Session.Options.HarvTruce) do_decode = true;
	if (_last_capture_the_flag != Session.Options.CTF) do_decode = true;
	if (_last_fog_of_war != Session.Options.FogOfWar) do_decode = true;
	if (_last_short_game != Session.Options.ShortGame) do_decode = true;
	if (_last_game_speed != Session.Options.GameSpeed) do_decode = true;
	if (_last_crap_engineers != Session.Options.CrapEngineers) do_decode = true;

	if (!same_scenario || do_decode) {
		int accept = Net2GetAccept(NULL);

		if (accept == 1) {
			Net2SetAccept(NULL, 0);

			PMessagePrintf(-1, Fetch_String(TXT_HOST_CHANGED_OPTIONS));

			char buffer[64];
			sprintf(buffer, "A0");
			SendPublicGameopts(buffer);
		}
	}


	_last_unit_count = Session.Options.UnitCount;
	_last_tech_level = BuildLevel;
	_last_credits = Session.Options.Credits;
	_last_ai_players = Session.Options.AIPlayers;
	_last_ai_difficulty = Session.Options.AIDifficulty;
	_last_allies_allowed = Session.Options.AlliesAllowed;
	_last_bridge_destruction = Session.Options.BridgeDestruction;
	_last_fog_of_war = Session.Options.FogOfWar;
	_last_goodies = Session.Options.Goodies;
	_last_bases = Session.Options.Bases;
	_last_crap_engineers = Session.Options.CrapEngineers;
	_last_harvester_truce = Session.Options.HarvTruce;
	_last_mcv_redeploy = Session.Options.MCVRedeploy;
	_last_short_game = Session.Options.ShortGame;
	_last_game_speed = Session.Options.GameSpeed;
	_last_capture_the_flag = Session.Options.CTF;

	if (!same_scenario) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the number of starting positions a scenario offers.
/// This routine is used to check that there is somewhere to put every player before a map is
/// allowed into a multiplayer game.
/// </summary>
/// <param name="index">Index into the multiplayer scenario list.</param>
/// <returns>Returns with the number of start positions on the map. Zero is returned if the
/// scenario could not be read.</returns>
int RandomMapWaypointCount(int index)
{
	char wp[32];
	if (index >= 0 && index < Session.Scenarios.Count()) {
		CCFileClass file(Session.Scenarios[index]->Get_Filename());

		INIClass ini;
		if (!ini.Load(file)) {
			return(0);
		}

		int count = 0;

		for (int i = 0; i < MAX_PLAYERS; i++) {
			sprintf(wp, "%d", i);
			if (ini.Get_Int("Waypoints", wp, -1) != -1) {
				count++;
			}
		}

		if (count == 0) {
			count = ini.Get_Int("RandomMap", "NumPlayers", 0);
		}

		return(count);
	}
	return(0);
}


/// <summary>
/// Lets the host choose the scenario for the game.
/// </summary>
/// <returns>True when the player settled on a map.</returns>
bool Scenario_Dialog(void)
{
	return(UI_Scenario_Dialog());
}
/// <summary>
/// Puts the options every machine agreed on into the globals the simulation reads, so a match
/// against other machines is played under one set of rules however it was set up.
/// </summary>
void Commit_Session_Specials(void)
{
	Special.IsHarvesterImmune = Session.Options.HarvTruce;
	Special.IsDestroyBridges = Session.Options.BridgeDestruction;
	Special.IsScrapMetal = Session.Options.ScrapMetal;
	Special.IsTGrowth = true;
	Special.IsTSpread = true;
	Special.Apply_To_Game();
}


/// <summary>
/// Performs the last setup step before a multiplayer game begins.
/// This routine copies the agreed session options into the globals the game logic actually
/// reads, so that every machine starts the scenario with the same rules in force.
/// </summary>
void PregameSetup(void)
{
	Scen->Scenario = Session.Options.ScenarioIndex;
	Frame = 0;
	Session.NumPlayers = Session.Players.Count();
	DebugString("Pregame setup for %d players.\n", Session.NumPlayers);
	Options.GameSpeed = Session.Options.GameSpeed;
	Session.CommProtocol = DEFAULT_COMM_PROTOCOL;
	Commit_Session_Specials();
}


/// <summary>
/// Updates the map preview shown in a network game dialog.
/// This routine is called whenever the selected scenario changes. A guest that does not have
/// the scenario locally asks the host for a preview instead of building one, so the picture
/// may not appear until that download arrives.
/// </summary>
/// <returns>bool; Was a preview built? A guest still waiting on one from the host, or a
/// scenario this machine does not hold, leaves none.</returns>
bool Update_Network_Dialog_Preview(void)
{
	delete MultiplayerMapPreview;
	MultiplayerMapPreview = NULL;

	switch (Session.Type) {
		case GAME_IPX:
			if (Net2LobbyPhase == NET2_LOBBY_GUEST && !Find_Local_Scenario(Session.ScenarioFileName, Session.ScenarioFileLength, Session.ScenarioDigest, Session.ScenarioIsOfficial)) {
				GlobalPacketType packet;
				memset(&packet, 0, sizeof(packet));
				packet.Command = NET_REQ_PREVIEW;
				while (true) {
					if (Ipx.Send_Global_Message(&packet, sizeof(packet), 1, &Session.HostAddress)) {
						break;
					}
					Call_Back();
				}
				while (Ipx.Global_Num_Send() != 0) {
					Call_Back();
				}
				return(false);
			}
			break;
	}

	bool unavailable = CCFileClass(Session.ScenarioFileName).Is_Available() == false;
	if (unavailable) {
		if (MultiplayerMapPreview != NULL) {
			delete MultiplayerMapPreview;
			MultiplayerMapPreview = NULL;
		}
		return(false);
	}

	if (MultiplayerMapPreview != NULL) {
		delete MultiplayerMapPreview;
		MultiplayerMapPreview = NULL;
	}
	MultiplayerMapPreview = new MapPreviewClass;
	if (MultiplayerMapPreview == NULL) {
		return(false);
	}

	MultiplayerMapPreview->Read_INI_Preview(Session.ScenarioFileName);
	return(true);
}



/// <summary>
/// Receives the random map preview image from the host.
/// This is the guest side of the preview handshake. The host is told that this machine is
/// ready, the compressed preview file is downloaded, and the decompressed image becomes the
/// preview shown in the multiplayer dialog.
/// </summary>
void Receive_Random_Map_Preview(void)
{
	Ipx.Set_Timing(50, -1, 5000);
	DebugString("Starting map preview download\n");

	GlobalPacketType packet;
	memset(&packet, 0, sizeof(packet));
	packet.Command = NET_PREVIEW_ACK;
	DebugString("Sending preview mode acks\n");
	while (true) {
		if (Ipx.Send_Global_Message(&packet, sizeof(packet), 1, &Session.HostAddress)) {
			break;
		}
		Call_Back();
	}
	while (Ipx.Global_Num_Send() != 0) {
		Call_Back();
	}
	DebugString("Preview mode acks sent\n");

	Session.GAddress = Session.HostAddress;
	DebugString("Calling Get_File_From_Host to receive the file download\n");
	char preview_name[256];
	bool got_file = Get_File_From_Host(preview_name, false);
	if (!got_file) {
		DebugString("got_file is false. Download failed\n");
		Ipx.Set_Timing(TIMER_SECOND / 2, -1, 10 * TIMER_SECOND);
		return;
	}

	DebugString("Loading the compressed preview image\n");
	CDFileClass file(preview_name);
	int size = file.Size();
	char * buffer = new char[size];
	file.Read(buffer, size);
	int preview_size = ((int *)buffer)[0];

	DebugString("Decompressing the preview image\n");

	BufferStraw bstraw(&((int *)buffer)[1], size);
	LZOStraw lzostraw(LZOStraw::DECOMPRESS);
	lzostraw.Get_From(&bstraw);
	char * preview = new char[2 * preview_size];
	lzostraw.Get(preview, preview_size);

	DebugString("Creating the new preview surface\n");
	if (MultiplayerMapPreview) {
		delete MultiplayerMapPreview;
	}
	MultiplayerMapPreview = new MapPreviewClass;
	MultiplayerMapPreview->Create_Preview_Surface(preview);

	DebugString("Cleaning up the temporary decompression buffers\n");
	delete [] preview;
	delete [] buffer;

	Ipx.Set_Timing(TIMER_SECOND / 2, -1, 10 * TIMER_SECOND);
}


/// <summary>
/// Sets the session's scenario information from the scenario list.
/// This routine is used whenever the host picks a different map. If the scenario file is not
/// on the hard drive, the player is asked for the disk that holds it, and the selection
/// fails if that disk cannot be made available.
/// </summary>
/// <param name="index">Index into the multiplayer scenario list, or -1 to clear the
/// selection.</param>
/// <returns>bool; Was the scenario information set?</returns>
bool Set_Scenario_Info_From_Index(int index)
{
	if (index == -1) {
		Session.ScenarioFileName[0] = '\0';
		Session.ScenarioDigest[0] = '\0';
		Session.ScenarioFileLength = 0;
		Session.ScenarioIsOfficial = 0;
		Session.Options.ScenarioDescription[0] = '\0';
		return(false);
	}

	bool unavailable = CCFileClass(Session.Scenarios[index]->Get_Filename()).Is_Available() == false;

	if (unavailable) {
		WWMessageBox().Process(TXT_UNABLE_READ_SCENARIO, TXT_OK);
		return(false);
	}

	strcpy(Session.Options.ScenarioDescription, Session.Scenarios[index]->Description());
	strcpy(Session.ScenarioDigest, (Session.Scenarios[index])->Get_Digest());
	strcpy(Session.ScenarioFileName, (Session.Scenarios[index])->Get_Filename());
	strcpy(Scen->ScenarioName, Session.ScenarioFileName);
	Session.ScenarioIsOfficial = Session.Scenarios[index]->Get_Official();
	Session.ScenarioFileLength = CCFileClass(Session.ScenarioFileName).Size();
	Session.ScenarioFileLength = CCFileClass(Session.ScenarioFileName).Size();
	return(true);
}


/// <summary>
/// Sends the random map preview image to the guests.
/// Only a generated map needs this -- an ordinary scenario's preview is built from the map
/// file the guest already holds. The guests are put into preview receive mode and, once they
/// have all answered, the compressed image is handed to the remote file sender.
/// </summary>
/// <remarks>Only the game host should call this routine.</remarks>
void Send_Preview_To_Guests(void)
{
	CDTimerClass<SystemTimerClass> response_timer;

	if (MultiplayerMapPreview != NULL && stricmp(Session.ScenarioFileName, RANDOM_MAP_FILE_NAME) == 0 && Session.Players.Count() > 1) {
		DebugString("Starting map preview upload\n");

		GlobalPacketType packet;
		memset(&packet, 0, sizeof(packet));
		packet.Command = NET_PREVIEW_MODE;
		strcpy(packet.Name, Session.Handle);

		Ipx.Set_Timing(50, -1, 5000);
		response_timer = TIMER_SECOND * 20;

		for (int i = 1; i < Session.Players.Count(); i++) {
			Ipx.Send_Global_Message(&packet, sizeof(packet), 1, &Session.Players[i]->Address);
			Call_Back();
		}

		while (Ipx.Global_Num_Send() != 0 && response_timer) {
			Call_Back();
		}

		if (response_timer) {
			IPXAddressClass sender_address;
			bool responses[MAX_PLAYERS];
			memset(responses, 0, sizeof(responses));
			int num_responses = 0;

			response_timer = TIMER_SECOND * 30;
			DebugString("Waiting for %d players to signal ready to receive\n", Session.Players.Count() - 1);

			while (num_responses < Session.Players.Count() - 1 && response_timer) {

				Call_Back();

				GlobalPacketType response = {};
				int length = 455;
				unsigned short product_id;

				if (Ipx.Get_Global_Message(&response, sizeof(response), &length, &sender_address, &product_id)) {
					if (response.Command == NET_PREVIEW_ACK) {
						for (int j = 1; j < Session.Players.Count(); j++) {
							if (sender_address == Session.Players[j]->Address) {
								if (!responses[j]) {
									responses[j] = true;
									num_responses++;
								}
								break;
							}
						}
					}
				}
			}

			if (num_responses == Session.Players.Count() - 1) {
				if (MultiplayerMapPreview != NULL) {
					DebugString("Creating low-res preview image\n");

					int size = 0;
					unsigned * preview = MultiplayerMapPreview->Create_Paletted_Preview(64, size);
					DebugString("Preview size is %d bytes\n", size);

					unsigned * buffer = new unsigned[size];
					BufferPipe bpipe(buffer, size * sizeof(unsigned));

					LZOPipe lzopipe(LZOPipe::COMPRESS);
					lzopipe.Put_To(&bpipe);
					int comp_size = lzopipe.Put(preview, size);
					comp_size += lzopipe.End();

					DebugString("Compressed preview image is %d bytes\n", comp_size);

					CDFileClass file("Preview.bin");
					if (file.Is_Available()) {
						file.Delete();
					}

					file.Open(RawFileClass::WRITE);
					file.Write(&size, sizeof(size));
					file.Write(buffer, comp_size);
					file.Close();
					delete [] buffer;

					DebugString("Calling Send_Remote_File to send the preview\n");
					Send_Remote_File("Preview.bin", true, false);
					Ipx.Set_Timing(TIMER_SECOND / 2, -1, 10 * TIMER_SECOND);
				}
			}
		}
	}
}


/// <summary>
/// Fetches a digest string for the current random map seed.
/// A generated map has no file to checksum, so this routine builds the identity string that
/// stands in for one. It is what lets the multiplayer scenario checks tell one generated map
/// from another. The map description is deliberately left out -- renaming a map does not
/// make it a different map.
/// </summary>
/// <param name="digest">Buffer to fill in with the digest string.</param>
/// <param name="bufsize">Size of the destination buffer.</param>
void CalcRandomMapDigest(char * digest, int bufsize)
{
	unsigned char * data;
	char description[sizeof(RandomMapGen.SeedData.MapDescription)];
	int size;
	unsigned char *bytes;
	unsigned int val;
	unsigned int hibit;
	unsigned int checksum = 0;

	memcpy(description, RandomMapGen.SeedData.MapDescription, sizeof(description));

	/// Hash all of SeedData except UseTransitions
	data = (unsigned char *)&RandomMapGen.SeedData.Biome;
	size = (sizeof(RandomMapGen.SeedData) - offsetof(MapSeedClass, Biome) - sizeof(RandomMapGen.SeedData.UseTransitions));
	memset(RandomMapGen.SeedData.MapDescription, '\0', sizeof(description));

	while (size > 0) {
		hibit = checksum >> 31;
		if (size >= 4) {
			val = (*(unsigned int *)data);
			checksum <<= 1;
			checksum += val;
			checksum += hibit;
			data += sizeof(unsigned int);
			size -= sizeof(unsigned int);
		} else {
			val = 0;
			bytes = data;
			while (size) {
				val <<= 8;
				val |= (*bytes++);
				size--;
			}
			checksum <<= 1;
			checksum += val;
			checksum += hibit;
		}
	}

	snprintf(digest, bufsize, "%08X", checksum);
	memcpy(RandomMapGen.SeedData.MapDescription, description, sizeof(description));
}


/// <summary>
/// Creates a new random map for a multiplayer game.
/// This routine prompts the host with the random map generator, saves the resulting seed out
/// under the random map file name, and makes sure the map has an entry in the scenario list
/// so that it can be picked like any other map.
/// </summary>
/// <returns>Returns with the scenario list index of the random map. Zero is returned if the
/// player backed out of the generator.</returns>
int CreateRandomMap(void)
{
	int result;
	if (Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) {
		result = Do_Random_Map_Dialog(Net2Callback);
	} else {
		result = Do_Random_Map_Dialog(MapGen_Call_Back);
	}

	if (result != IDOK) {
		return(0);
	}

	IsRandomMap = true;
	RandomMapGen.SeedData.Save(RANDOM_MAP_FILE_NAME);

	delete MultiplayerMapPreview;
	MultiplayerMapPreview = new MapPreviewClass;
	MultiplayerMapPreview->Read_PCX_Preview("RandMap.img");

	int index;
	bool found = false;
	for (index = 0; index < Session.Scenarios.Count(); index++) {
		if (stricmp(Session.Scenarios[index]->Get_Filename(), RANDOM_MAP_FILE_NAME) == 0) {
			found = true;
			break;
		}
	}

	if (!found) {
		char digest[RANDOM_MAP_DIGEST_SIZE];
		CalcRandomMapDigest(digest, sizeof(digest));
		MultiMission * scenario = new MultiMission(RANDOM_MAP_FILE_NAME, RandomMapGen.SeedData.MapDescription, digest);
		Session.Scenarios.Add(scenario);
	} else {
		Session.Scenarios[index]->Set_Description(RandomMapGen.SeedData.MapDescription);
	}

	return(index);
}
