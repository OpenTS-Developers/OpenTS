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

#include "always.h"

#include "netdlg2.h"

#include "ui/screens/netlobby/uinetlobby.h"

#include "_map.h"
#include "_rand.h"
#include "_rules.h"
#include "_timer.h"
#include "addon.h"
#include "conquer.h"
#include "data.h"
#include "dbgprint.h"
#include "globals.h"
#include "goptions.h"
#include "houstype.h"
#include "init.h"
#include "ipxmgr.h"
#include "language/language.h"
#include "mapgen.h"
#include "mplayer.h"
#include "msgbox.h"
#include "netdlg.h"
#include "netshare.h"
#include "nettiming.h"
#include "newmenu.h"
#include "rules.h"
#include "scenario.h"
#include "sendfile.h"
#include "srfcache.h"
#include "stimer.h"
#include "timer.h"
#include "utf8.h"
#include "winstub.h"
#include "wsproto.h"

#include <algorithm>


/*
******************************** Prototypes *********************************
*/
static int Request_To_Join(int join_index);
static void Unjoin_Game(int game_index);
static void Send_Join_Queries(int gamenow, int playernow, int chatnow, int init = 0);
static void Get_Join_Responses(void);

bool Net2ReadyToGo(int load_game);
void Net2ServiceGameList(void);

int CurGame;
int _netresponse;
JoinStateType JoinState;
char SerialNumber[23];
bool Net2IsGameListActive = true;
bool Net2GameStarted = false;

Net2LobbyPhaseType Net2LobbyPhase = NET2_LOBBY_NONE;

// The lobby dialog that is up, or null while a screen carries the phase instead.

// False once a screen has failed to open, so the flow falls back to the dialogs and stays there.

int RulesID;
int ArtID;
int AIID;

int Net2_g_Col_Accept;
int Net2_g_Col_Name;
int Net2_g_Col_House;


/// <summary>
/// Fills a side box with the multiplayable countries, each entry carrying its country index.
/// </summary>
int Net2FirstFreeColor(int reqcolor, int index)
{
	int color;
	while (1) {
		int taken = 0;
		color = reqcolor;
		for (int i = 0; i < Session.Players.Count(); i++) {
			if (i==index) {
				continue;
			}
			if (Session.Players[i]->Player.Color == reqcolor) {
				reqcolor++;
				taken=1;
			}
		}
		if (taken==0) {
			break;
		}
		reqcolor %= MAX_PLAYERS;
	}
	return(color);
}


int Net2Response(void)
{
	return(_netresponse);
}


int Net2CurrentGame(void)
{
	return(CurGame);
}


/// <summary>
/// Returns the country the side chooser's nth row stands for, or -1 for no such row.
/// </summary>
int Net2Country_At(int index)
{
	for (int country = 0; country < HouseTypes.Count(); country++) {
		if (!HouseTypes[country]->IsMultiplay) {
			continue;
		}
		if (index == 0) {
			return(country);
		}
		index--;
	}

	return(-1);
}


/// <summary>
/// Takes the name the player typed and tells the lobby about it.
/// </summary>
/// <remarks>A name too long for the session's buffer is cut to what fits, so the caller
/// should read Session.Handle back rather than trusting what it asked for.</remarks>
void Net2Set_Handle(char const * name)
{
	if (strcmp(name, Session.Handle) == 0) {
		return;
	}

	UTF8::Copy(Session.Handle, sizeof(Session.Handle), name);
	Send_Join_Queries(0, 0, 1, 0);
}


/// <summary>
/// Throws one player out of the game this machine is hosting.
/// </summary>
void Net2Kick(char const * name)
{
	if (strcmp(name, Session.Handle) == 0) {
		return;
	}

	for (int index = 0; index < Session.Players.Count(); index++) {
		if (strcmp(name, Session.Players[index]->Name) == 0) {
			memset(&Session.GPacket, 0, sizeof(Session.GPacket));
			Session.GPacket.Command = NET_REJECT_JOIN;
			Session.GPacket.Reject.Why = (int)REJECT_BY_OWNER;
			Ipx.Send_Global_Message(&Session.GPacket, 455, 1, &Session.Players[index]->Address);
			break;
		}
	}
}


/// <summary>
/// Moves the browser's highlight to another game and asks that game who is in it.
/// </summary>
void Net2Select_Game(int index)
{
	if (JoinState > JOIN_NOTHING || index < 0 || index >= Session.Games.Count()) {
		return;
	}

	int old = CurGame;
	CurGame = index;
	strcpy(Session.GameName, Session.Games[index]->Name);

	Clear_Vector(&Session.Players);
	if (old != CurGame) {
		Send_Join_Queries(1, 1, 1, 0);
	}
}


/// <summary>
/// Gives the host the color it asked for, or the nearest free one.
/// </summary>
/// <remarks>A color another player already wears is refused with a message, and the host
/// keeps the nearest color free of the one it had.</remarks>
void Net2Host_Take_Color(int color)
{
	int old_color = Session.ColorIdx;

	Session.ColorIdx = color;
	Session.PrefColor = color;

	int resolved = Net2FirstFreeColor(color, 0);
	if (resolved != color) {
		PMessagePrintf(ColorSystem, Fetch_String(TXT_COLOR_IN_USE));
		resolved = Net2FirstFreeColor(old_color, 0);
	}

	Session.ColorIdx = resolved;
	if (Session.Players.Count() > 0) {
		Session.Players[0]->Player.Color = resolved;
	}

	PumpGameopts(1, 0);
}


/// <summary>
/// Asks the host for a country and a color. The host answers by publishing what it gave.
/// </summary>
void Net2Request_House_And_Color(int house, int color)
{
	Session.PrefColor = color;

	char options[64];
	sprintf(options, "R%d,%d", house, color);
	SendPrivateGameopts(Session.GameName, options);
}


/// <summary>
/// Shows a line of chat here and sends it to everyone this machine is talking to.
/// </summary>
/// <remarks>Inside a game that is the players; out in the browser it is everyone the lobby
/// has heard from.</remarks>
void Net2Send_Chat(char const * text)
{
	PMessagePrintf(ColorMe, "[%s] %s", Session.Handle, text);

	GlobalPacketType gpacket;
	memset(&gpacket, 0, sizeof(gpacket));

	gpacket.Command = NET_MESSAGE;
	strcpy(gpacket.Name, Session.Handle);
	strcpy(gpacket.Message.Buf, text);
	gpacket.Message.Color = Session.ColorIdx;
	gpacket.Message.NameCRC = Compute_Name_CRC(Session.GameName);

	DynamicVectorClass<NodeNameType *> & who = (JoinState == JOIN_CONFIRMED) ? Session.Players : Session.Chat;
	for (int index = 1; index < who.Count(); index++) {
		Ipx.Send_Global_Message(&gpacket, sizeof(gpacket), 1, &who[index]->Address);
		Call_Back();
	}
}


// A random map keeps its picture in a file of its own; every other map carries one.
static void Net2Refresh_Preview(void)
{
	if (stricmp((char *)Session.Scenarios[Session.Options.ScenarioIndex] + DESCRIP_MAX, "RandMap.Sed") != 0) {
		Update_Network_Dialog_Preview();
		return;
	}

	delete MultiplayerMapPreview;
	MultiplayerMapPreview = new MapPreviewClass;
	MultiplayerMapPreview->Read_PCX_Preview("RandMap.img");
	if (MultiplayerMapPreview->Get_Preview_Surface() == NULL) {
		Update_Network_Dialog_Preview();
	}
}


/// <summary>
/// Runs the map dialog for the host and settles what it chose.
/// </summary>
/// <remarks>Cancelling puts the mission the host had back and tells the guests about it,
/// which is what the Win32 dialog did; accepting leaves the new one for the next publication
/// to carry.</remarks>
void Net2Pick_Map(HWND window)
{
	int old = Session.Options.ScenarioIndex;

	IsRandomMap = false;

	if (Scenario_Dialog(MainWindow) == IDCANCEL) {
		Session.Options.ScenarioIndex = old;
		Set_Scenario_Info_From_Index(Session.Options.ScenarioIndex);
		Update_Network_Dialog_Preview();
		IsRandomMap = true;
		Net2Refresh_Preview();
		PumpGameopts(1, 0);
	} else {
		if (!Set_Scenario_Info_From_Index(Session.Options.ScenarioIndex)) {
			Session.Options.ScenarioIndex = old;
		}
		IsRandomMap = true;
		Net2Refresh_Preview();
	}
}


/// <summary>
/// Runs one pass of the lobby: the network, the game, the window messages and the join
/// protocol.
/// </summary>
/// <returns>bool; Should the caller stop waiting? The lobby never ends the session, so this
/// is always false; it is shaped this way because a screen's runner asks it the question.
/// </returns>
bool Net2_Service_Lobby(void)
{
	Ipx.Service();
	Sleep(0);
	Call_Back();
	Ipx.Service();
	Title_Screen_Restore();

	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	Call_Back();
	if (_netresponse != 0) {
		return(false);
	}

	if (Net2LobbyPhase != NET2_LOBBY_NONE) {
		Send_Join_Queries(false, false, false, false);
		Get_Join_Responses();
		if (Net2LobbyPhase == NET2_LOBBY_HOST) {
			PumpGameopts(false);
		}
		Net2ServiceGameList();
	}

	return(false);
}


// What entering a lobby does to the session. The dialogs did this from WM_INITDIALOG, where a
// screen has no such message; both go through here, so the flow settles the same way whichever
// presentation the lobby is wearing.
static void Net2_Enter_Lobby(Net2LobbyPhaseType phase)
{
	switch (phase) {
		case NET2_LOBBY_GAME_LIST: {
			CurGame = 0;
			Net2IsGameListActive = 1;
			Session.Options.ScenarioDescription[0] = '\0';
			Session.ColorIdx = Session.PrefColor;

			Clear_Vector(&Session.Games);
			Clear_Vector(&Session.Players);
			Clear_Vector(&Session.Chat);

			NodeNameType * who = new NodeNameType;
			strcpy(who->Name, Session.Handle);
			who->Chat.LastTime = 0;
			who->Chat.LastChance = 0;
			who->Chat.Color = Session.GPacket.PlayerInfo.Color;
			Session.Chat.Add(who);

			// The browser's first row is the lobby itself rather than a game, and carries no name.
			NodeNameType * game = new NodeNameType;
			strcpy(game->Name, "");
			game->Game.IsOpen = 0;
			game->Game.LastTime = 0;
			Session.Games.Add(game);

			Send_Join_Queries(true, false, true, true);
			break;
		}

		case NET2_LOBBY_HOST:
			VerNum.Init_Clipping();

			srand(NonCriticalRandomNumber(1, 0x7FFF));
			Seed = rand();

			Set_Scenario_Info_From_Index(0);
			Session.Options.ScenarioIndex = 0;
			Update_Network_Dialog_Preview();

			if (Session.Players.Count() > 0) {
				Session.Players[0]->Player.House = Session.House;
			}
			PumpGameopts(1, 0);
			Net2Host_Take_Color(Session.ColorIdx);
			break;

		case NET2_LOBBY_GUEST: {
			int self = -1;
			for (int index = 0; index < Session.Players.Count(); index++) {
				if (strcmp(Session.Players[index]->Name, Session.Handle) == 0) {
					self = index;
				}
			}
			if (self != -1) {
				Session.Players[self]->Player.Status = 0;
			}

			Session.Options.ScenarioDescription[0] = '\0';
			break;
		}

		default:
			break;
	}
}


/// <summary>
/// Records the phase the lobby has moved into and settles the session for it.
/// </summary>
/// <remarks>The driver opens the document for the phase, because a packet handler can change
/// the phase while a screen is up and no screen can be raised from inside another one's
/// pass.</remarks>
void Net2_Show_Lobby(Net2LobbyPhaseType phase)
{
	Net2_Clear_Chat_Log();

	Net2LobbyPhase = phase;
	Net2_Enter_Lobby(phase);
}


/// <summary>
/// Leaves the flow in no phase.
/// </summary>
void Net2_Close_Lobby(void)
{
	Net2LobbyPhase = NET2_LOBBY_NONE;
}


/// <summary>
/// Services the network while something else is waiting.
/// This routine is handed to the parts of the game that must block for a while -- the
/// message boxes and the file transfers -- so that the game keeps ticking over and the
/// join protocol keeps answering while they do.
/// </summary>
/// <returns>bool; Should the caller abandon what it is waiting on?</returns>
bool Net2Callback(void)
{
	Call_Back();
	Get_Join_Responses();
	return(false);
}


/***********************************************************************************************
 * Init_Network -- initializes network stuff                                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      true = Initialization OK, false = error                                                *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *=============================================================================================*/
bool Net2Init_Network (void)
{
	assert ( PacketTransport != NULL );

	//------------------------------------------------------------------------
	// This call allocates all necessary queue buffers and commands the
	// transport to start listening on the Global Channel.
	//------------------------------------------------------------------------
	return(Ipx.Init() != 0);

}	/* end of Init_Network */


/// <summary>
/// Ages out the games and chatters that have gone quiet.
/// This routine is called regularly while the lobby is up. A host that has stopped
/// advertising has its game dropped from the list, and a silent chatter is prodded once
/// before it too is forgotten.
/// </summary>
void Net2ServiceGameList(void)
{
	int i;

	for (i = 1; i < Session.Games.Count(); i++) {
		if (strcmp(Session.Games[i]->Name, Session.GameName) != 0) {
			if (TickCount - Session.Games[i]->Game.LastTime > 5 * TIMER_SECOND) {
				DebugString("Game '%s' has timed out\n", Session.Games[i]->Name);
				delete Session.Games[i];
				Session.Games.Delete(Session.Games[i]);

						if (i <= CurGame) {
									Send_Join_Queries(0, 1, 0, 0);
				}
			}
		}
	}

	for (i = 1; i < Session.Chat.Count(); i++) {
		if (Session.Chat[i]->Chat.LastTime == 0) {
			Session.Chat[i]->Chat.LastTime = TickCount;
		}
		if (TickCount - Session.Chat[i]->Chat.LastTime > 6 * TIMER_SECOND) {
			delete Session.Chat[i];
			Session.Chat.Delete(Session.Chat[i]);
				} else if (TickCount - Session.Chat[i]->Chat.LastTime > 5 * TIMER_SECOND &&
			Session.Chat[i]->Chat.LastChance == 0) {
			GlobalPacketType packet;
			memset (&packet, 0, sizeof(GlobalPacketType));
			strcpy(packet.Name, Session.Handle);
			packet.Command = NET_CHAT_REQUEST;
			Ipx.Send_Global_Message (&packet, sizeof(GlobalPacketType), 0, &(Session.Chat[i]->Address));
			Call_Back();
			Session.Chat[i]->Chat.LastChance = 1;
		}
	}
}


/// <summary>
/// Encodes the current game options into a text string.
/// This routine builds the blob the host broadcasts to the guests, so that every machine
/// agrees on the rules, the scenario, and who is playing what.
/// </summary>
/// <param name="out">Buffer to build the encoded option string within.</param>
/// <remarks>Be sure the destination buffer is big enough for the options and an entry for
/// every player in the game.</remarks>
void Net2EncodeGameopt(char *out, int size)
{
	int length = snprintf(out, size, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
		"%s,%d,%d,%s,%s:",
		Session.Options.UnitCount,
		BuildLevel,
		Session.Options.Credits,
		Session.Options.FogOfWar,
		Session.Options.BridgeDestruction,
		Session.Options.Goodies,
		Session.Options.MCVRedeploy,
		Session.Options.AlliesAllowed,
		Session.Options.HarvTruce,
		Session.Options.Bases,
		Session.Options.CTF,
		Seed,
		Options.GameSpeed,
		Session.Options.AIPlayers,
		Session.Options.AIDifficulty,
		Session.Options.ShortGame,
		Session.Options.CrapEngineers,
		Session.Options.GameSpeed,
		Session.Options.ScenarioDescription,
		Session.ScenarioIsOfficial,
		Session.ScenarioFileLength,
		Session.ScenarioFileName,
		Session.ScenarioDigest);
	if (length < 0 || length >= size) {
		DebugString("Game options do not fit the packet\n");
		return;
	}

	for (int i = 0; i < Session.Players.Count(); i++) {
		int written = snprintf(out + length, size - length, "%s,%d,%d,", Session.Players[i]->Name, Session.Players[i]->Player.House,
				Session.Players[i]->Player.Color);
		if (written < 0 || written >= size - length) {
			DebugString("Game options do not fit the packet for player %d\n", i);
			out[length] = '\0';
			return;
		}
		length += written;
	}
}


/// <summary>
/// Sets the accept status of a player.
/// The player list is redisplayed afterwards, so that the ready markers beside the names
/// keep up with what the other machines have reported.
/// </summary>
/// <param name="who">The name of the player to adjust. NULL means this machine's own
/// player.</param>
/// <param name="status">The accept status to record against the player.</param>
void Net2SetAccept(char * who, int status)
{
	if (who == NULL) {
		who = Session.Handle;
	}

	int offset = -1;

	for (int i = 0; i < Session.Players.Count(); i++) {
		if (strcmp(Session.Players[i]->Name, who) == 0) {
			offset = i;
		}
	}

	if (offset != -1) {
		Session.Players[offset]->Player.Status = status;
	}
}


/// <summary>
/// Fetches the accept status of a player.
/// </summary>
/// <param name="who">The name of the player to examine. NULL means this machine's own
/// player.</param>
/// <returns>Returns with the player's accept status, or -1 if no such player is in the
/// game.</returns>
int Net2GetAccept(char * who)
{
	if (who == NULL) {
		who = Session.Handle;
	}

	int offset = -1;

	for (int i = 0; i < Session.Players.Count(); i++) {
		if (strcmp(Session.Players[i]->Name, who) == 0) {
			offset = i;
		}
	}

	if (offset != -1) {
		return (Session.Players[offset]->Player.Status);
	}
	return (-1);
}


/// <summary>
/// Sets the house and color of a player in the game.
/// This routine is called as the option packets arrive from the other machines. The
/// player list is redisplayed, and when it is this machine's own entry that changed, the
/// color combo box is nudged along to agree with it.
/// </summary>
/// <param name="who">The name of the player to adjust.</param>
/// <returns>Returns with TRUE if the player's house actually changed.</returns>
int Net2SetHouseAndColor(char *who, int house, int color)
{
	int offset = -1;
	int retval = 0;

	for (int i = 0; i < Session.Players.Count(); i++) {
		if (strcmp(Session.Players[i]->Name, who) == 0) {
			offset = i;
		}
	}

	if (offset != -1) {
		if (Session.Players[offset]->Player.House != house) {
			retval = TRUE;
		}

		Session.Players[offset]->Player.House = house;
		Session.Players[offset]->Player.Color = color;
		}

	if (offset == 0) {
		Session.PrefColor = color;
		Session.ColorIdx=color;
		Session.House=house;
	}

	return(retval);
}


/// <summary>
/// Fetches the serial number recorded in the registry.
/// A key that is missing, or that cannot be opened, simply leaves the buffer as it was
/// found -- the caller is expected to have primed it with something harmless.
/// </summary>
/// <param name="serial">Buffer to fill in with the serial number found.</param>
/// <param name="reg_key">The registry key, beneath the local machine hive, to read from.</param>
/// <remarks>Be sure the buffer is big enough to hold an entire encrypted serial number.</remarks>
static void Get_Serial_From_Registry(char * serial, char const * reg_key)
{
	if (reg_key && strlen(reg_key) != 0) {
		HKEY rKey;
		char keyname[256];
		strcpy(keyname, reg_key);
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyname, 0, KEY_READ, &rKey) == ERROR_SUCCESS) {
			DWORD type;
			DWORD sizeOfBuffer = ENCRYPTION_STRING_LENGTH;
			RegQueryValueEx(rKey, "Serial", NULL, &type, (BYTE *)serial, &sizeOfBuffer);
			RegCloseKey(rKey);
		}
	}
	serial[SERIAL_MAX-1] = 0;
}


/// <summary>
/// Fetches the decrypted serial number of this installation.
/// The scrambled serial is pulled out of the registry and then unpicked with the key file
/// that shipped alongside the game. This routine is used to identify the player to the
/// online service.
/// </summary>
/// <param name="buffer">Buffer to fill in with the decrypted serial number.</param>
/// <returns>bool; Was the serial number recovered? Failure means the key file was not
/// available.</returns>
/// <remarks>Be sure the destination buffer is big enough to hold an entire serial number.</remarks>
bool Decrypt_Serial(char * buffer)
{
	char serial[ENCRYPTION_STRING_LENGTH];

	bool encrypt = false;

	memset(serial, '0', SERIAL_MAX-1);
	serial[SERIAL_MAX-1] = 0;
	strcpy(buffer, serial);
	memset(serial, 0, sizeof(serial));

	Get_Serial_From_Registry(serial, "SOFTWARE\\Westwood\\Tiberian Sun");

	strcpy(buffer, serial);

	int sign = encrypt ? 1 : -1;

	int number;
	int temp;
	int pos = 0;

	FILE *in = fopen("woldata.key", "r");

	if (in == NULL) {
		return(false);
	}

	while ((number = fgetc(in)) != EOF) {
		temp = serial[pos] - '0';
		temp %= 10;
		number *= sign;
		temp += number;
		temp += 1000;
		temp %= 10;
		temp += '0';
		serial[pos] = temp;

		pos++;
		if (pos == (int)strlen(serial)) {
			pos = 0;
		}
	}

	fclose(in);

	strcpy(buffer, serial);

	return(true);
}


/***********************************************************************************************
 * Remote_Connect -- handles connecting this user to others                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      true = connections established; false = not                                            *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *=============================================================================================*/
// What a lobby screen answered with, as the responses the Win32 dialogs set.
static void Net2Apply_Choice(UINetChoice choice)
{
	switch (choice) {
		case UI_NET_CANCEL:
			_netresponse = IDCANCEL;
			break;

		case UI_NET_JOIN:
			_netresponse = IDC_GAMELIST_JOIN;
			break;

		case UI_NET_NEW:
			_netresponse = IDC_GAMELIST_NEW;
			break;

		case UI_NET_GO:
			Net2GameStarted = true;
			_netresponse = IDC_GO;
			break;

		default:
			break;
	}
}


bool Net2Remote_Connect(void)
{
	RulesID = RulesClass::Get_Rule_Unique_ID();
	RulesClass::Load_Art_INI();
	ArtID = RulesClass::Get_Art_Unique_ID();
	AIID = RulesClass::Get_AI_Unique_ID();
	Decrypt_Serial(SerialNumber);

	//------------------------------------------------------------------------
	//	Init network timing parameters; these values should work for both a
	// "real" network, and a simulated modem network (ie Kali)
	//------------------------------------------------------------------------
	Ipx.Set_Timing (TIMER_SECOND / 2,    // retry 2 times per second
					-1,                  // ignore max retries
					10 * TIMER_SECOND);  // give up after 10 seconds


	//------------------------------------------------------------------------
	// The game is now "open" for joining.  Close it as soon as we exit this
	// routine.
	//------------------------------------------------------------------------
	Session.NetOpen = true;

	//------------------------------------------------------------------------
	// Save the original value of the NetStealth flag, so we can turn stealth
	//	off for now (during this portion of the dialogs, we must show ourselves)
	//------------------------------------------------------------------------
	Session.NetStealth = false;

	//------------------------------------------------------------------------
	// Init my game name to 0-length, since I haven't joined any game yet.
	//------------------------------------------------------------------------
	Session.GameName[0] = '\0';

	Net2GameStarted = false;

	Net2_Show_Lobby(NET2_LOBBY_GAME_LIST);

	_netresponse = 0;
	CurGame = 0;
	JoinState = JOIN_NOTHING;
	Net2IsGameListActive = true;

	//------------------------------------------------------------------------
	// Keep looping until something useful happens.
	//------------------------------------------------------------------------
	while (true) {
		//.....................................................................
		// Wait for the lobby that is up to be answered, or for a packet to move
		// the flow on under it.
		//.....................................................................
		UINetChoice choice = UI_Net_Lobby_Run();

		Net2Apply_Choice(choice);

		//.....................................................................
		//	-1 = user selected Cancel
		//.....................................................................
		if (_netresponse == IDCANCEL) {
			Session.Write_MultiPlayer_Settings();
			if (Net2LobbyPhase == NET2_LOBBY_GAME_LIST) {
				if (JoinState > JOIN_NOTHING) {
					Unjoin_Game(CurGame);
					Ipx.Service();
				}
				Net2_Close_Lobby();
				Clear_Vector(&Session.Players);
				Clear_Vector(&Session.Games);
				Clear_Vector(&Session.Chat);
				Session.NetOpen = false;
				Ipx.Service();
				return(false);
			}

			if (Net2LobbyPhase == NET2_LOBBY_HOST) {
				Unjoin_Game(CurGame);
				JoinState = JOIN_NOTHING;
				Net2_Show_Lobby(NET2_LOBBY_GAME_LIST);
				Send_Join_Queries(false, false, true, false);
			}

			if (Net2LobbyPhase == NET2_LOBBY_GUEST) {
				//...............................................................
				// If we're joined to a game, make extra sure the other players in
				//	that game know I'm exiting; send my SIGN_OFF as an ack-required
				// packet.  Don't send this to myself (index 0).
				//...............................................................
				if (JoinState == JOIN_CONFIRMED) {
					Unjoin_Game(CurGame);
					JoinState = JOIN_NOTHING;
				} else {
					//...............................................................
					// If I'm not joined to a game, send a SIGN_OFF to all players
					// in my Chat vector (but not to myself, index 0)
					//...............................................................
					GlobalPacketType gpacket;
					memset(&gpacket, 0, sizeof(gpacket));
					gpacket.Command = NET_SIGN_OFF;
					strcpy(gpacket.Name, Session.Handle);
					for (int i = 1; i < Session.Chat.Count(); i++) {
						Ipx.Send_Global_Message(&gpacket, sizeof(gpacket), 1, &Session.Chat[i]->Address);
						Call_Back();
					}

					//............................................................
					// Now broadcast a SIGN_OFF just to be thorough
					//............................................................
					Ipx.Send_Global_Message(&gpacket, sizeof(gpacket), 0, 0);
				}

				//.....................................................................
				// Wait for all the ACK's to come in.
				//.....................................................................
				CDTimerClass<SystemTimerClass> timeout = TIMER_SECOND * 10;
				while (Ipx.Global_Num_Send() > 0) {
					if (Ipx.Service() == 0) {
						break;
					}
					if (timeout == 0) {
						break;
					}
					Call_Back();
				}

				Session.GameName[0] = '\0';
				JoinState = JOIN_NOTHING;
				Net2_Close_Lobby();
				_netresponse = 0;
				CurGame = 0;
				Clear_Vector(&Session.Players);
				Net2_Show_Lobby(NET2_LOBBY_GAME_LIST);
			}
		}

		//.....................................................................
		//	0 = user has joined an existing game; save values & return
		//.....................................................................
		if (_netresponse == IDC_GAMELIST_JOIN) {
			Session.NetStealth = false;
			Session.Write_MultiPlayer_Settings();

			if (Request_To_Join(CurGame)) {
				JoinState = JOIN_WAIT_CONFIRM;
			}
		}

		//.....................................................................
		//	1 = user requests New Network Game
		//.....................................................................
		if (_netresponse == IDC_GAMELIST_NEW) { /// Net_New_Dialog maybe?

			bool ok = true;

			//...............................................................
			// Force user to enter a name
			//...............................................................
			if (strlen(Session.Handle) < 1) {
				ODMessageBox(Fetch_String(TXT_NAME_BLANK), 0, Net2Callback);
				ok = false;
			}

			//...............................................................
			// Ensure name is unique
			//...............................................................
			for (int i = 0; i < Session.Games.Count(); i++) {
				if (ok && !strcmp(Session.Games[i]->Name, Session.Handle)) {
					ODMessageBox(Fetch_String(TXT_GAMENAME_MUSTBE_UNIQUE), 0, Net2Callback);
					ok = false;
					break;
				}
			}

			if (ok) {
				//...............................................................
				// Save player & game name
				//...............................................................
				strcpy(Session.GameName, Session.Handle);

				Session.NetOpen = true;
				Session.NetStealth = false;
				Session.Options.ScenarioIndex = 0;
				Session.PlayingAgainstVersion = VerNum.Version_Number();
				Set_Scenario_Info_From_Index(Session.Options.ScenarioIndex);

				Net2_Close_Lobby();
				_netresponse = 0;

				//------------------------------------------------------------------------
				// Clear the list of players
				//------------------------------------------------------------------------
				Clear_Vector(&Session.Players);

				//------------------------------------------------------------------------
				// Add myself to the list, and to the Players vector.
				//------------------------------------------------------------------------
				NodeNameType * who = new NodeNameType;
				strcpy(who->Name, Session.Handle);
				strcpy(who->Player.Serial, SerialNumber);
				who->Player.House = Session.House;
				who->Player.Color = Session.ColorIdx;
				Session.Players.Add(who);

				JoinState = JOIN_CONFIRMED;

				NodeNameType * game = new NodeNameType;
				strcpy(game->Name, Session.Handle);
				game->Address = Session.GAddress;
				game->Game.IsOpen = true;
				game->Game.LastTime = TickCount;
				game->Game.Addon = Addon_Enabled(ADDON_FIRESTORM);
				Session.Games.Add(game);

				CurGame = Session.Games.Count() - 1;

				//..................................................................
				//	Pop up the New Network Game dialog; if user selects OK, return
				//	'true'; otherwise, return to the Join Dialog.
				//..................................................................
				Net2_Show_Lobby(NET2_LOBBY_HOST);
			}
		}

		if (_netresponse != 1 || Net2LobbyPhase != NET2_LOBBY_GUEST) {
			if (_netresponse == IDC_GO) {
				Net2GameStarted = 0;
				Session.Write_MultiPlayer_Settings();
				if (_netresponse == IDC_GO) {

					//...............................................................
					//	If there are at least 2 players, go ahead & play; error otherwise
					//...............................................................
					if (Session.Players.Count() == 1) {
						PMessagePrintf(-1, Fetch_String(TXT_ONLY_ONE));
						_netresponse = 0;
					}

					if (_netresponse == IDC_GO) {
						for (int i = 0; i < Session.Players.Count(); i++) {
							if (Session.Players[i]->Player.Status == 0) {
								PMessagePrintf(-1, Fetch_String(TXT_ACCEPTFIRST));
								_netresponse = 0;
										break;
							}
						}
					}
				}
			}
		} else {

			/*
			 * The guest accepted the host's "go" -- tear down the dialogs, run
			 * the pregame setup, compute the packet timing and leave the loop.
			 */
			Net2_Close_Lobby();
			_netresponse = 0;

			PregameSetup();

			// A compressed game starts at the fixed bootstrap rung and measures from there.
			if (Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP) {
				NetTiming::TimingSettings const initial = NetTiming::Settings_For_Rung(NetTiming::INITIAL_TIMING_RUNG);
				Session.FrameSendRate = initial.FrameSendRate;
				Session.MaxAhead = initial.MaxAhead;
			} else {
				Session.FrameSendRate = DEFAULT_FRAME_SEND_RATE;
				Session.MaxAhead = std::max(((int)Ipx.Global_Response_Time() / 8), NETWORK_MIN_MAX_AHEAD);
			}

			break;
		}

		// The computer players belong in this sum, but the original reads their slider from
		// the game list window, which never carried one, so the term has always been zero.
		int waypoints = RandomMapWaypointCount(Session.Options.ScenarioIndex);
		if (waypoints < Session.Players.Count()) {
			PMessagePrintf(-1, Fetch_String(TXT_SCENARIO_TOO_SMALL));
			_netresponse = 0;
		} else {
			if (_netresponse != IDC_GO) {

				/*
				 * Not the GO button -- there is nothing to do this pass, so reset
				 * the response and fall back through the main message loop.
				 */
				_netresponse = 0;

			} else {
				Net2GameStarted = 1;

				PumpGameopts(true, true);

				if (MultiplayerMapPreview != NULL) {
					delete MultiplayerMapPreview;
					MultiplayerMapPreview = NULL;
				}

				PregameSetup();

				// A compressed game starts at the fixed bootstrap rung and measures from there.
				if (Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP) {
					NetTiming::TimingSettings const initial = NetTiming::Settings_For_Rung(NetTiming::INITIAL_TIMING_RUNG);
					Session.FrameSendRate = initial.FrameSendRate;
					Session.MaxAhead = initial.MaxAhead;
				} else {
					Session.FrameSendRate = DEFAULT_FRAME_SEND_RATE;
					Session.MaxAhead = std::max(((int)Ipx.Global_Response_Time() / 8), NETWORK_MIN_MAX_AHEAD);
				}

				Ipx.Set_Timing(std::max<unsigned>(TIMER_SECOND / 2, (unsigned int)Ipx.Global_Response_Time() + 2), (unsigned int)-1, 10 * TIMER_SECOND);

				//.....................................................................
				// Send all players the NET_GO packet.  Wait until all ACK's have been
				// received.
				//.....................................................................
				GlobalPacketType gpacket;
				memset(&gpacket, 0, sizeof(gpacket));
				gpacket.Command = NET_GO;
				gpacket.ResponseTime.OneWay = Session.MaxAhead;
				for (int i = 1; i < Session.Players.Count(); i++) {
					Ipx.Send_Global_Message(&gpacket, sizeof(gpacket), 1, &(Session.Players[i]->Address));
				}

				//.....................................................................
				// Wait for all the ACK's to come in.
				//.....................................................................
				CDTimerClass<SystemTimerClass> timeout = TIMER_SECOND * 20;
				while (Ipx.Global_Num_Send() > 0 && !timeout) {
					Call_Back();
				}

				/*
				** Wait for the go responses from each player in case someone needs the scenario
				** file to be sent.
				*/
				int responses[MAX_PLAYERS];
				memset(responses, 0, sizeof(responses));
				int num_responses = 0;
				bool send_scenario = false;
				DebugString("About to wait for 'GO' response.\n");
				CDTimerClass<SystemTimerClass> response_timer;    // timeout timer for waiting for responses
				response_timer = TIMER_SECOND * 10;               // Wait for 10 seconds. If we dont hear by then assume someone crashed

				do {
					Call_Back();
					int retcode = Ipx.Get_Global_Message(&Session.GPacket, sizeof(Session.GPacket), &Session.GPacketlen, &Session.GAddress, &Session.GProductID);
					if (retcode && Session.GProductID == IPXGlobalConnClass::COMMAND_AND_CONQUER2) {
						for (int i = 1; i < Session.Players.Count(); i++) {
							if (Session.Players[i]->Address == Session.GAddress) {
								if (!responses[i]) {
									if (Session.GPacket.Command == NET_REQ_SCENARIO) {
										DebugString("Received REQ_SCENARIO packet.\n");
										responses[i] = Session.GPacket.Command;
										send_scenario = true;
										num_responses++;
									}
									if (Session.GPacket.Command == NET_READY_TO_GO) {
										DebugString("Received READY_TO_GO packet.\n");
										responses[i] = Session.GPacket.Command;
										num_responses++;
									}
								}
							}
						}
					}
				} while (num_responses < Session.Players.Count() - 1 && response_timer);

				/*
				** If one of the machines requested that the scenario be sent then send it.
				*/
				if (send_scenario) {
					memset(Session.ScenarioRequests, 0, sizeof(Session.ScenarioRequests));
					Session.RequestCount = 0;
					for (int i = 1; i < Session.Players.Count(); i++) {
						if (responses[i] == NET_REQ_SCENARIO) {
							Session.ScenarioRequests[Session.RequestCount++] = i;
						}
					}
					Send_Remote_File(Scen->ScenarioName, false, true);
				}

				//------------------------------------------------------------------------
				// Init network timing values, using previous response times as a measure
				// of what our retry delta & timeout should be.
				//------------------------------------------------------------------------
				Ipx.Set_Timing(std::max<unsigned>(Ipx.Global_Response_Time() + 2, TIMER_SECOND / 2), (unsigned int)-1, std::max<unsigned>(2 * TIMER_SECOND, Ipx.Global_Response_Time() * 8));

				//------------------------------------------------------------------------
				// Restore screen
				//------------------------------------------------------------------------
				Hide_Mouse();
				Draw_Menu_Background();
				Show_Mouse();
				Net2_Close_Lobby();
				break;
			}
		}
	}

	Session.NetOpen = false;
	Session.Write_MultiPlayer_Settings();
	return(true);

} /* end of Remote_Connect */


/***************************************************************************
 * Request_To_Join -- Sends a JOIN request packet to game owner            *
 *                                                                         *
 * Regardless of the return code, the Join Dialog will need to be redrawn  *
 * after calling this routine.                                             *
 *                                                                         *
 * INPUT:                                                                  *
 *      playername      player's name                                      *
 *      join_index      index of game we're joining                        *
 *      house            requested house                                   *
 *      color            requested color                                   *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = Packet sent, 0 = wasn't                                        *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *=========================================================================*/
static int Request_To_Join(int join_index)
{
	//------------------------------------------------------------------------
	// Validate join_index
	//------------------------------------------------------------------------
	if (CurGame < 1) {
		PMessagePrintf(ColorSystem, Fetch_String(TXT_MUST_SELECT_GAME));
		Sound_Effect(Rule->SystemError);
		return(false);
	}
	if ( (Session.Games.Count()<=1) || CurGame > Session.Games.Count()) {
		PMessagePrintf(ColorSystem, Fetch_String(TXT_NOTHING_TO_JOIN));
		Sound_Effect(Rule->SystemError);
		return(false);
	}

	//------------------------------------------------------------------------
	// Force user to enter a name
	//------------------------------------------------------------------------
	if (strlen(Session.Handle)==0) {
		PMessagePrintf(ColorSystem, Fetch_String(TXT_NAME_ERROR));
		Sound_Effect(Rule->SystemError);
		return(false);
	}

	//------------------------------------------------------------------------
	// The game must be open
	//------------------------------------------------------------------------
	if (!Session.Games[CurGame]->Game.IsOpen) {
		PMessagePrintf(ColorSystem, Fetch_String(TXT_GAME_IS_CLOSED));
		Sound_Effect(Rule->SystemError);
		return(false);
	}

	if (Session.Games[CurGame]->Game.Addon == ADDON_FIRESTORM && Addon_Enabled(ADDON_FIRESTORM) == false) {
		if (Addon_Installed(ADDON_FIRESTORM) != true) {
			PMessagePrintf(ColorSystem, Fetch_String(TXT_FIRESTORM_REQUIRED));
			Sound_Effect(Rule->SystemError);
			return(false);
		} else {
			PMessagePrintf(ColorSystem, Fetch_String(TXT_FIRESTORM_MUST_ENABLE));
			Sound_Effect(Rule->SystemError);
			return(false);
		}
	}

	if (Session.Games[CurGame]->Game.Addon == ADDON_BASE_GAME && Addon_Enabled(ADDON_FIRESTORM) == true) {
		PMessagePrintf(ColorSystem, Fetch_String(TXT_FIRESTORM_NO_JOIN_TS));
		Sound_Effect(Rule->SystemError);
		return(false);
	}

	//------------------------------------------------------------------------
	// Send packet to game's owner
	//------------------------------------------------------------------------
	memset (&Session.GPacket, 0, sizeof(GlobalPacketType));

	Session.GPacket.Command = NET_QUERY_JOIN;
	strcpy (Session.GPacket.Name, Session.Handle);
	strcpy (Session.GPacket.Serial, SerialNumber);
	Session.GPacket.PlayerInfo.House = Session.House;
	Session.GPacket.PlayerInfo.Color = Session.ColorIdx;
	Session.GPacket.PlayerInfo.MinVersion = VerNum.Min_Version();
	Session.GPacket.PlayerInfo.MaxVersion = VerNum.Max_Version();
	Session.GPacket.PlayerInfo.CheatCheck = RulesID;
	Session.GPacket.PlayerInfo.ArtCheatCheck = ArtID;
	Session.GPacket.PlayerInfo.AICheatCheck = AIID;
	Session.GPacket.PlayerInfo.BuildNumber = Build_Number();

	DebugString("RulesID = %lX\n", RulesID);
	DebugString("ArtID = %lX\n", ArtID);
	DebugString("AIID = %lX\n", AIID);
	DebugString("BuildNumber = %ld\n", Build_Number());
	DebugString("RuleINI ID = %lX\n", RuleINI->Get_Unique_ID());
	DebugString("FSRuleINI = %lX\n", FSRuleINI.Get_Unique_ID());

	Ipx.Send_Global_Message(&Session.GPacket, sizeof(GlobalPacketType), 1, &(Session.Games[CurGame]->Address));

	return(true);

} /* end of Request_To_Join */


/***************************************************************************
 * Unjoin_Game -- Cancels joining a game                                   *
 *                                                                         *
 * INPUT:                                                                  *
 *      namebuf         current player name                                *
 *      joinstate      current join state                                  *
 *      gamelist         ListBox of game names                             *
 *      playerlist      ListBox of player names                            *
 *      game_index      index in 'gamelist' of game we're leaving          *
 *      goto_lobby      true = we're going to the lobby                    *
 *      msg_x            message system x-coord                            *
 *      msg_y            message system y-coord                            *
 *      msg_h            message system char height                        *
 *      send_x         message system send x-coord                         *
 *      send_y         message system send y-coord                         *
 *      msg_len         message system max msg length                      *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   12/12/1995 BRR : Created.                                             *
 *=========================================================================*/
static void Unjoin_Game(int game_index)
{
	int i;
	GlobalPacketType packet;

	//------------------------------------------------------------------------
	// Fill in a SIGN_OFF packet
	//------------------------------------------------------------------------
	memset (&packet, 0, sizeof(GlobalPacketType));
	packet.Command = NET_SIGN_OFF;
	strcpy(packet.Name,Session.Handle);

	//------------------------------------------------------------------------
	// If we're joined to a game, make extra sure the other players in
	//	that game know I'm exiting; send my SIGN_OFF as an ack-required
	// packet.  Don't send this to myself (index 0).
	//------------------------------------------------------------------------
	for (i = 1; i < Session.Players.Count(); i++) {
		Ipx.Send_Global_Message (&packet, sizeof(packet), 1,
			&(Session.Players[i]->Address));
		Call_Back();
	}

	if (JoinState == JOIN_WAIT_CONFIRM || JoinState == JOIN_CONFIRMED) {
		if (!Session.Games[game_index]->Address.Is_Broadcast()) {
			Ipx.Send_Global_Message (&packet, sizeof(packet), 1,
				&(Session.Games[game_index]->Address));
		}
	}
#if 0
	//------------------------------------------------------------------------
	// Re-init the message system to its new larger size
	//------------------------------------------------------------------------
	Session.Messages.Init (msg_x + 1, msg_y + 1, 14,
		msg_len, msg_h, send_x + 1, send_y + 1, 1,
		20, msg_len - 5);
	Session.Messages.Add_Edit((Session.ColorIdx == PCOLOR_DIALOG_BLUE) ?
											PCOLOR_REALLY_BLUE : Session.ColorIdx,
											TPF_TEXT, NULL, '_');
#endif

	Ipx.Send_Global_Message (&packet, sizeof(packet), 0,NULL);

	//------------------------------------------------------------------------
	// Remove myself from the player list, and reset my game name
	//------------------------------------------------------------------------
	Clear_Vector (&Session.Players);

	Session.GameName[0] = 0;

#if 0
	//------------------------------------------------------------------------
	// Highlight "Lobby" on the Game list, Announce I'm ready to chat
	//------------------------------------------------------------------------
	if (goto_lobby) {
		gamelist->Set_Selected_Index(0);
		Send_Join_Queries (game_index, joinstate, 0, 0, 1, namebuf);
	}
#endif
}	// end of Unjoin_Game


/***********************************************************************************************
 * Send_Join_Queries -- sends queries for the Join Dialog                                      *
 *                                                                                             *
 * This routine [re]sends the queries related to the Join Dialog:                              *
 * - NET_QUERY_GAME                                                                            *
 * - NET_QUERY_PLAYER for the game currently selected (if there is one)                        *
 *                                                                                             *
 * The queries are "staggered" in time so they aren't all sent at once; otherwise, we'd        *
 * be inundated with reply packets & we'd miss some (even though the replies will require      *
 * ACK's).                                                                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      curgame      index of currently-selected game; -1 = none                               *
 *      joinstate   our current joinstate                                                      *
 *      gamenow      if 1, will immediately send the game query                                *
 *      playernow   if 1, will immediately send the player query for currently-selected game   *
 *      chatnow      if 1, will immediately send the chat announcement                         *
 *      myname      user's name                                                                *
 *      init         initialize the timers                                                     *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *   04/15/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
static void Send_Join_Queries(int gamenow, int playernow, int chatnow, int init)
{
	GlobalPacketType packet = {};

	//........................................................................
	// These values control the timeouts for sending various types of packets;
	// they're designed such that they'll rarely occur simultaneously.
	//........................................................................
	enum {
		GAME_QUERY_TIME = 2 * TIMER_SECOND,
		PLAYER_QUERY_TIME = 35,
		CHAT_ANNOUNCE_TIME = 83,
	};
	static CDTimerClass<SystemTimerClass> game_timer;   // time between NET_QUERY_GAME's
	static CDTimerClass<SystemTimerClass> player_timer; // time between NET_QUERY_PLAYERS's
	static CDTimerClass<SystemTimerClass> chat_timer;   // time between NET_CHAT_ANNOUNCE's


	//------------------------------------------------------------------------
	// Initialize timers
	//------------------------------------------------------------------------
	if (init) {
		game_timer = GAME_QUERY_TIME;
		player_timer = PLAYER_QUERY_TIME;
		chat_timer = CHAT_ANNOUNCE_TIME;
	}

	//------------------------------------------------------------------------
	// Send the game-name query if the time has expired, or we're told to do
	// it right now
	//------------------------------------------------------------------------
	if (!game_timer || gamenow) {

		game_timer = GAME_QUERY_TIME;
		if ((Net2LobbyPhase != NET2_LOBBY_HOST) || gamenow) {
			memset (&packet, 0, sizeof(GlobalPacketType));

			packet.Command = NET_QUERY_GAME;

			strcpy (packet.Name, Session.Handle);

			Ipx.Send_Global_Message (&packet,
				sizeof(GlobalPacketType), 0, NULL);
		}
	}

	//------------------------------------------------------------------------
	// Send the player query for the game currently clicked on, if the time has
	// expired and there is a currently-selected game, or we're told to do it
	// right now
	//------------------------------------------------------------------------
	if ( ((CurGame > 0) && (CurGame < Session.Games.Count()) &&
		!player_timer) || playernow) {

		player_timer = PLAYER_QUERY_TIME;

		memset (&packet, 0, sizeof(GlobalPacketType));

		packet.Command = NET_QUERY_PLAYER;
		strcpy (packet.Name, Session.Games[CurGame]->Name);

		Ipx.Send_Global_Message (&packet,
			sizeof(GlobalPacketType), 0, NULL);
	}

	//------------------------------------------------------------------------
	// Send the chat announcement
	//------------------------------------------------------------------------
	if ((!chat_timer && JoinState<JOIN_CONFIRMED) || chatnow) {

		chat_timer = CHAT_ANNOUNCE_TIME;

		memset (&packet, 0, sizeof(GlobalPacketType));

		packet.Command = NET_CHAT_ANNOUNCE;
		strcpy (packet.Name, Session.Handle);
		packet.Chat.ID = Session.UniqueID;
		packet.Chat.Color = Session.ColorIdx;

		Ipx.Send_Global_Message (&packet,
			sizeof(GlobalPacketType), 0, NULL);
	}

}	/* end of Send_Join_Queries */


/***********************************************************************************************
 * Process_Global_Packet -- responds to remote queries                                         *
 *                                                                                             *
 * The only commands from other systems this routine responds to are NET_QUERY_GAME            *
 * and NET_QUERY_PLAYER.  The other commands are too context-specific to be able               *
 * to handle here, such as joining the game or signing off; but this routine handles           *
 * the majority of the program's needs.                                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      packet      ptr to packet to process                                                   *
 *      address      source address of sender                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      true = packet was processed, false = wasn't                                            *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      Session.GameName must have been filled in before this function can be called.          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/15/1995 BR : Created.                                                                  *
 *=============================================================================================*/
bool Process_Global_Packet(GlobalPacketType *packet, IPXAddressClass *address)
{
	GlobalPacketType mypacket = {};
#if 0
	//------------------------------------------------------------------------
	// If our Players vector is empty, just return.
	//------------------------------------------------------------------------
	if (Session.Players.Count()==0) {
		return(true);
	}
#endif
	//------------------------------------------------------------------------
	// Another system asking what game this is
	//------------------------------------------------------------------------
	if (packet->Command==NET_QUERY_GAME && Session.NetStealth==0) {

		//.....................................................................
		// If the game is closed, let every player respond, and let the sender of
		// the query sort it all out.  This way, if the game's host exits the game,
		// the game still shows up on other players' dialogs.
		// If the game is open, only the game owner may respond.
		//.....................................................................
		if (strlen(Session.GameName) > 0 &&
			(!Session.NetOpen || !strcmp(Session.Handle,Session.GameName)) &&
			(!strcmp(Session.Handle,Session.GameName) || ScenarioActive)) {

			memset (&mypacket, 0, sizeof(GlobalPacketType));

			mypacket.Command = NET_ANSWER_GAME;
			strcpy(mypacket.Name, Session.GameName);
			mypacket.GameInfo.IsOpen = Session.NetOpen;
			mypacket.GameInfo.IsFirestorm = Addon_Enabled(ADDON_FIRESTORM);

			Ipx.Send_Global_Message (&mypacket, sizeof(GlobalPacketType), 1,
				address);
		}
		return(true);
	}

	//------------------------------------------------------------------------
	// Another system asking what player I am
	//------------------------------------------------------------------------
	else if (packet->Command==NET_QUERY_PLAYER &&
		!strcmp (packet->Name, Session.GameName) &&
			(strlen(Session.GameName) > 0) && Session.NetStealth==0 && JoinState >= JOIN_CONFIRMED) {

		memset (&mypacket, 0, sizeof(GlobalPacketType));		// changed DRD 9/26

		mypacket.Command = NET_ANSWER_PLAYER;
		strcpy(mypacket.Name, Session.Handle);
		strcpy(mypacket.Serial, SerialNumber);
		mypacket.PlayerInfo.House = Session.House;
		mypacket.PlayerInfo.Color = Session.ColorIdx;
		mypacket.PlayerInfo.NameCRC = Compute_Name_CRC(Session.GameName);

		Ipx.Send_Global_Message (&mypacket, sizeof(GlobalPacketType), 1, address);
		return(true);
	}

	return(false);

}	/* end of Process_Global_Packet */


/***********************************************************************************************
 * Get_Join_Responses -- sends queries for the Join Dialog                                     *
 *                                                                                             *
 * This routine polls the Global Channel to see if there are any incoming packets;             *
 * if so, it processes them.  This routine can change the state of the Join Dialog, or         *
 * the contents of the list boxes, based on what the packet is.                                *
 *                                                                                             *
 * The list boxes are passed in as pointers; they can't be made globals, because they          *
 * can't be constructed, because they require shape pointers to the arrow buttons, and         *
 * the mix files won't have been initialized when the global variables' constructors are       *
 * called.                                                                                     *
 *                                                                                             *
 * This routine sets the globals                                                               *
 *      Session.House               (from NET_CONFIRM_JOIN)                                    *
 *      Session.ColorIdx            (from NET_CONFIRM_JOIN)                                    *
 *      Session.Options.Bases      (from NET_GAME_OPTIONS)                                     *
 *      Session.Options.Tiberium   (from NET_GAME_OPTIONS)                                     *
 *      Session.Options.Goodies      (from NET_GAME_OPTIONS)                                   *
 *      Session.Options.Ghosts      (from NET_GAME_OPTIONS)                                    *
 *      ScenarioIdx            (from NET_GAME_OPTIONS; -1 = scenario not found)                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      joinstate      current state of Join Dialog                                            *
 *      gamelist         list box containing game names                                        *
 *      playerlist      list box containing player names for the currently-selected game       *
 *      join_index      index of the game we've joined or are asking to join                   *
 *      my_name         name of local system                                                   *
 *      why            ptr: filled in with reason for rejection from a game                    *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      Event that occurred                                                                    *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *   04/15/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
static void Get_Join_Responses(void)
{
	int rc;
	NodeNameType *who;				// node to add to Games or Players
	int i;
	int found;
	RejectType why;
	char txt[80];
	int resend;
	unsigned int version;              // version # to use

	//------------------------------------------------------------------------
	// If there is no incoming packet, just return
	//------------------------------------------------------------------------
	for (Call_Back(); (rc = Ipx.Get_Global_Message (&Session.GPacket, sizeof(Session.GPacket),
		&Session.GPacketlen, &Session.GAddress, &Session.GProductID)) != 0; Call_Back()) {
		if (Session.GProductID != IPXGlobalConnClass::COMMAND_AND_CONQUER2) {
			continue;
		}

		//------------------------------------------------------------------------
		//	If we're joined in a game, handle the packet in a standard way; otherwise,
		// don't answer standard queries.
		//------------------------------------------------------------------------
		if (Process_Global_Packet(&Session.GPacket,&Session.GAddress)!=0) {
			continue;
		}

		if (Session.GPacket.Command==NET_PREVIEW_MODE) {
			if (Net2LobbyPhase == NET2_LOBBY_GUEST) {
				Receive_Random_Map_Preview();
			}
			continue;
		}

		if (Session.GPacket.Command==NET_REQ_PREVIEW) {
			if (Net2LobbyPhase == NET2_LOBBY_HOST) {
				Send_Preview_To_Guests();
			}
			continue;
		}
		//------------------------------------------------------------------------
		// NET_ANSWER_GAME:  Another system is answering our GAME query, so add that
		// system to our list box if it's new.
		//------------------------------------------------------------------------
		if (Session.GPacket.Command==NET_ANSWER_GAME) {

			//.....................................................................
			// See if this name is unique
			//.....................................................................
			found = 0;
			for (i = 1; i < Session.Games.Count(); i++) {
				if (!strcmp(Session.Games[i]->Name, Session.GPacket.Name)) {
					found = 1;

					//...............................................................
					// If name was found, update the node's time stamp & IsOpen flag.
					//...............................................................
					Session.Games[i]->Game.LastTime = TickCount;
					if (Session.Games[i]->Game.IsOpen != Session.GPacket.GameInfo.IsOpen) {
						Session.Games[i]->Game.IsOpen = Session.GPacket.GameInfo.IsOpen;

						//............................................................
						// If this game has gone from closed to open, copy the
						// responder's address into our Game slot, since the guy
						// responding to this must be game owner.
						//............................................................
						if (Session.Games[i]->Game.IsOpen) {
							Session.Games[i]->Address = Session.GAddress;
						}

						//............................................................
						// If we're in chat mode, print a message that the state of
						// this game has changed.
						//............................................................
						if (JoinState < JOIN_CONFIRMED) {
							if (Session.Games[i]->Game.IsOpen) {
								snprintf(txt, sizeof(txt), Fetch_String(TXT_S_FORMED_NEW_GAME),
									Session.GPacket.Name);
								Sound_Effect(Rule->GameForming);
							}
							else {
								snprintf(txt, sizeof(txt), Fetch_String(TXT_GAME_NOW_IN_PROGRESS),
									Session.GPacket.Name);
								Sound_Effect(Rule->GameClosed);
							}
							PMessagePrintf(ColorSystem, txt);
						}
					}
					break;
				}
			}

			//.....................................................................
			//	name not found (or addresses are different); add it to 'Games'
			//.....................................................................
			if (found==0) {

				//..................................................................
				// Create a new node structure, fill it in, add it to 'Games'
				//..................................................................
				who = new NodeNameType;
				UTF8::Copy(who->Name, sizeof(who->Name), Session.GPacket.Name);
				who->Address = Session.GAddress;
				who->Game.IsOpen = Session.GPacket.GameInfo.IsOpen;
				who->Game.Addon = Session.GPacket.GameInfo.IsFirestorm;
				who->Game.LastTime = TickCount;
				Session.Games.Add (who);

				//..................................................................
				// If this player's in the Chat vector, remove him from there
				//..................................................................
				for (i = 1; i < Session.Chat.Count(); i++) {
					if (Session.Chat[i]->Address==Session.GAddress) {
						delete Session.Chat[i];
						Session.Chat.Delete(Session.Chat[i]);
						break;
					}
				}

				//..................................................................
				// If this game is open, display a message stating that it's
				// now available.
				//..................................................................
				if (Session.GPacket.GameInfo.IsOpen && JoinState < JOIN_CONFIRMED) {
					snprintf(txt, sizeof(txt), Fetch_String(TXT_S_FORMED_NEW_GAME),
						Session.GPacket.Name);
					PMessagePrintf(ColorSystem, txt);
					Sound_Effect(Rule->GameForming);
				}

			}
			continue;
		}

		//------------------------------------------------------------------------
		// NET_ANSWER_PLAYER: Another system is answering our PLAYER query, so add
		// it to our player list box & the Player Vector if it's new
		//------------------------------------------------------------------------
		if (Session.GPacket.Command==NET_ANSWER_PLAYER) {
			//.....................................................................
			// See if this name is unique
			//.....................................................................
			found = 0;
			for (i = 0; i < Session.Players.Count(); i++) {

				//..................................................................
				// If the address is already present, re-copy their name, color &
				// house into the existing entry, in case they've changed it without
				//	our knowledge; set the 'found' flag so we won't create a new entry.
				//..................................................................
				if (Session.Players[i]->Address==Session.GAddress) {
					found = 1;
					break;
				}
			}

			//.....................................................................
			// Don't add this player if he's not part of the game that's selected.
			//.....................................................................
			if (Session.Games.Count() && Session.GPacket.PlayerInfo.NameCRC !=
				Compute_Name_CRC(Session.GameName)) {
				found = 1;
			}

			//.....................................................................
			// Don't add this player if it's myself.  (We must check the name
			// since the address of myself in 'Players' won't be valid.)
			//.....................................................................
			if (!strcmp (Session.Handle,Session.GPacket.Name)) {
				found = 1;
			}

			//.....................................................................
			//	name not found, or address didn't match; add to player list box
			// & Players Vector
			//.....................................................................
			if (found==0) {
				//..................................................................
				// Create & add a node to the Vector
				//..................................................................
				who = new NodeNameType;
				UTF8::Copy(who->Name, sizeof(who->Name), Session.GPacket.Name);
				strcpy(who->Player.Serial, Session.GPacket.Serial);
				who->Address = Session.GAddress;
				who->Player.House = Session.GPacket.PlayerInfo.House;
				who->Player.Color = Session.GPacket.PlayerInfo.Color;
				Session.Players.Add (who);

				//..................................................................
				// If this player's in the Chat vector, remove him from there
				//..................................................................
				for (i = 1; i < Session.Chat.Count(); i++) {
					if (Session.Chat[i]->Address==Session.GAddress) {
						delete Session.Chat[i];
						Session.Chat.Delete(Session.Chat[i]);
						break;
					}
				}

				for (i = 0; i < Session.Players.Count(); i++) {
					NodeNameType * player = Session.Players[i];
					if (strcmp(player->Name,Session.GameName) && player->Player.Status != 0) {
						player->Player.Status = 0;
					}
				}

				DebugString("New Player");

				//..................................................................
				// If this player has joined our game, play a special sound.
				//..................................................................
				if (JoinState>=JOIN_CONFIRMED) {
					Sound_Effect(Rule->PlayerJoined);
				}
			}
			continue;
		}

		//------------------------------------------------------------------------
		//	NET_CONFIRM_JOIN: The game owner has confirmed our JOIN query; mark us
		// as being confirmed, and start answering queries from other systems
		//------------------------------------------------------------------------
		if (Session.GPacket.Command==NET_CONFIRM_JOIN) {
			if ( JoinState != JOIN_CONFIRMED) {
				JoinState = JOIN_CONFIRMED;
				strcpy (Session.GameName, Session.GPacket.Name);
				Session.House = Session.GPacket.PlayerInfo.House;
				Session.ColorIdx = Session.GPacket.PlayerInfo.Color;

				//...............................................................
				// Clear the player list, then add myself to the list.
				//...............................................................
				Clear_Vector(&Session.Players);

				who = new NodeNameType;
				strcpy(who->Name, Session.Handle);
				who->Player.House = Session.House;
				who->Player.Color = Session.ColorIdx;
				Session.Players.Add (who);

				Net2IsGameListActive = false;
				_netresponse = 0;
				Net2_Show_Lobby(NET2_LOBBY_GUEST);

				Send_Join_Queries(1, 1, 1, 0);
			}
			continue;
		}

		//------------------------------------------------------------------------
		//	NET_REJECT_JOIN: The game owner has turned down our JOIN query; restore
		// the dialog state to its first pop-up state.
		//------------------------------------------------------------------------
		if (Session.GPacket.Command==NET_REJECT_JOIN) {
			why = REJECT_DUPLICATE_NAME;
			//.....................................................................
			// If we're confirmed in a game, broadcast a sign-off to tell all other
			// systems that I'm no longer a part of any game; this way, I'll be
			// properly removed from their dialogs.
			//.....................................................................
			if ( JoinState == JOIN_CONFIRMED) {
				GlobalPacketType packet;

				memset (&packet, 0, sizeof(GlobalPacketType));
				packet.Command = NET_SIGN_OFF;
				strcpy (packet.Name,Session.Handle);

				for (i = 1; i < Session.Players.Count(); i++) {
					Ipx.Send_Global_Message (&packet,
						sizeof(GlobalPacketType), 1,
						&(Session.Players[i]->Address));
					Call_Back();
				}

				Ipx.Send_Global_Message (&packet,
					sizeof (GlobalPacketType), 0, NULL);

				while (Ipx.Global_Num_Send() > 0 && Ipx.Service() != 0) {
					Call_Back();
				}

				Session.GameName[0] = 0;

				//..................................................................
				// remove myself from the player list
				//..................................................................

				Clear_Vector(&Session.Players);
				Clear_Vector(&Session.Chat);
				CurGame = 0;
				Net2IsGameListActive = true;
				JoinState = JOIN_REJECTED;
				why = REJECT_BY_OWNER;
			}
			//.....................................................................
			// If we're waiting for confirmation & got rejected, tell the user why
			//.....................................................................
			else if (JoinState == JOIN_WAIT_CONFIRM) {
				why = (RejectType)Session.GPacket.Reject.Why;
				JoinState = JOIN_REJECTED;
			}
			//..................................................................
			// If we've been rejected, clear any messages we may have been
			// typing, add a message stating why we were rejected, and send a
			// chat announcement.
			//..................................................................
			if (JoinState == JOIN_REJECTED) {
				PMessagePrintf(ColorSystem, Fetch_String(TXT_REQUEST_DENIED));
				Sound_Effect(Rule->SystemError);

				char *item = NULL;
				if (why==REJECT_DUPLICATE_NAME) {
					item = (char *)Fetch_String(TXT_NAME_MUSTBE_UNIQUE);
				}
				else if (why==REJECT_GAME_FULL) {
					item = (char *)Fetch_String(TXT_GAME_FULL);
				}
				else if (why==REJECT_VERSION_TOO_OLD) {
					item = (char *)Fetch_String(TXT_YOURGAME_OUTDATED);
				}
				else if (why==REJECT_VERSION_TOO_NEW) {
					item = (char *)Fetch_String(TXT_DESTGAME_OUTDATED);
				}
				else if (why==REJECT_MISMATCH) {
					item = (char *)Fetch_String(TXT_MISMATCH);
				}
				else if (why==REJECT_DISBANDED) {
					item = (char *)Fetch_String(TXT_GAME_CANCELLED);
				}
				else if (why==REJECT_DUPLICATE_SERIAL) {
					item = (char *)Fetch_String(TXT_SERIAL_DUP);
				}
				if (item) {
					ODMessageBox(item, 0, Net2Callback, 0);
				}
				if (Net2LobbyPhase != NET2_LOBBY_GAME_LIST) {
					_netresponse = IDCANCEL;
				}
				Send_Join_Queries (0, 0, 1, 0);
			}
			continue;
		}

		if (Session.GPacket.Command==NET_PUB_GAMEOPT) {
			for (i = 0; i < Session.Players.Count(); i++) {
				if (!strcmp(Session.Players[i]->Name,Session.GPacket.Name)) {
					if (i != -1 && !Net2GameStarted) {
						Session.HostAddress = Session.GAddress;
						DecodePubGameopt(Session.GPacket.Options.Buf, Session.GPacket.Name);
					}
					break;
				}
			}
			continue;
		}

		if (Session.GPacket.Command==NET_PRIV_GAMEOPT) {
			char *opts = strdup(Session.GPacket.Options.Buf + 1);
			for (i = 1; i < Session.Players.Count(); i++) {
				if (!strcmp(Session.Players[i]->Name,Session.GPacket.Name)) {

					if (i != -1 && Net2GameStarted != 1) {
						int oldhouse = Session.Players[i]->Player.House;
						int oldcolor = Session.Players[i]->Player.Color;
						int newcolor = 0;
						int newhouse = 0;
						char * tok;

						tok = strtok(opts, ",");
						if (tok) {
							newhouse = atol(tok);
							Session.Players[i]->Player.House = newhouse;
						}

						tok = strtok(NULL, ",");
						if (tok) {
							int reqcolor = atol(tok);
							newcolor = Net2FirstFreeColor(reqcolor, i);
							if (newcolor != reqcolor) {
								newcolor = Net2FirstFreeColor(oldcolor, i);
							}
							Session.Players[i]->Player.Color = newcolor;
						}
						if (oldcolor != newcolor || oldhouse != newhouse) {
							PumpGameopts(true, 0);
						}
					}
					break;
				}
			}
			free(opts);
			continue;
		}

		//------------------------------------------------------------------------
		// NET_GAME_OPTIONS: The game owner has changed the game options & is
		// sending us the new values.
		//------------------------------------------------------------------------
		if (Session.GPacket.Command==NET_GAME_OPTIONS) {
			/// Nothing in TS
			continue;
		}

		//------------------------------------------------------------------------
		// NET_SIGN_OFF: Another system is signing off: search for that system in
		// both the game list & player list, & remove it if found
		//------------------------------------------------------------------------
		if (Session.GPacket.Command==NET_SIGN_OFF) {
			//.....................................................................
			// Remove this name from the list of games
			//.....................................................................
			for (i = 1; i < Session.Games.Count(); i++) {
				if (!strcmp(Session.Games[i]->Name, Session.GPacket.Name) &&
					Session.Games[i]->Address==Session.GAddress) {

					//...............................................................
					// If the system signing off is the currently-selected list
					// item, clear the player list since that game is no longer
					// forming.
					//...............................................................
					if (i==CurGame) {
						Clear_Vector (&Session.Players);
						if (Net2LobbyPhase == NET2_LOBBY_GUEST) {
							_netresponse = 2;
						}
					}

					//...............................................................
					// If the system signing off was the owner of our game, mark
					// ourselves as rejected
					//...............................................................
					if ( JoinState > JOIN_NOTHING && i==CurGame) {
						JoinState = JOIN_REJECTED;
						why = REJECT_DISBANDED;
					}

					//...............................................................
					// Set my return code
					//...............................................................
					//if (retcode == EV_NONE) {
					//	if (i <= CurGame) {
					//		retcode = EV_GAME_SIGNOFF;
					//	}
					//	else {
					//		retcode = EV_PLAYER_SIGNOFF;
					//	}
					//}

					//...............................................................
					// Remove game name from game list
					//...............................................................
					delete Session.Games[i];
					Session.Games.Delete(Session.Games[i]);

					if (CurGame > i) {
						CurGame--;
					}
					Send_Join_Queries(0, 1, 1, 0);
				}
			}

			bool player_removed = false;
			//.....................................................................
			// Remove this name from the list of players
			//.....................................................................
			for (i = 0; i < Session.Players.Count(); i++) {

				//..................................................................
				//	Name found; remove it
				//..................................................................
				if (Session.Players[i]->Address==Session.GAddress) {

					delete Session.Players[i];
					Session.Players.Delete(Session.Players[i]);

					player_removed = true;

					//...............................................................
					// If this player has left our game, play a special sound.
					//...............................................................
					if (JoinState>=JOIN_CONFIRMED) {
						Sound_Effect(Rule->PlayerLeft);
					}

					//if (retcode == EV_NONE) {
					//	retcode = EV_PLAYER_SIGNOFF;
					//}
				}
			}

			//.....................................................................
			// Remove this name from the chat list
			//.....................................................................
			for (i = 1; i < Session.Chat.Count(); i++) {

				//..................................................................
				//	Name found; remove it
				//..................................................................
				if (Session.Chat[i]->Address==Session.GAddress) {

					delete Session.Chat[i];
					Session.Chat.Delete(Session.Chat[i]);

					//if (retcode == EV_NONE) {
					//	retcode = EV_PLAYER_SIGNOFF;
					//}
				}
			}

			if (player_removed) {
				for (i = 0; i < Session.Players.Count(); i++) {
					NodeNameType * player = Session.Players[i];
					if (strcmp(player->Name,Session.GameName) && player->Player.Status != 0) {
						player->Player.Status = 0;
					}
				}
			}

			continue;
		}

		//------------------------------------------------------------------------
		// NET_GO: The game's owner is signalling us to start playing.
		//------------------------------------------------------------------------
		else if (Session.GPacket.Command==NET_GO || Session.GPacket.Command==NET_LOADGAME) {
			if ( JoinState==JOIN_CONFIRMED) {
				if (Session.GPacket.Command == NET_GO && Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP) {
					int const max_ahead = Session.GPacket.ResponseTime.OneWay;
					if (max_ahead < 0) {
						continue;
					}

					NetTiming::TimingSettings const initial = NetTiming::Settings_For_Rung(NetTiming::INITIAL_TIMING_RUNG);
					NetTiming::TimingSettings const received{initial.FrameSendRate, static_cast<unsigned int>(max_ahead)};
					if (!NetTiming::Timing_Settings_Are_Valid(received) || received != initial) {
						continue;
					}

					Session.FrameSendRate = received.FrameSendRate;
					Session.MaxAhead = received.MaxAhead;
				} else {
					Session.MaxAhead = Session.GPacket.ResponseTime.OneWay;
				}
				Session.HostAddress = Session.GAddress;
				Session.NumPlayers = Session.Players.Count();
				_netresponse = IDOK;
				if (Session.GPacket.Command==NET_GO) {
					JoinState = JOIN_GAME_START;
					if (!Net2ReadyToGo(0)) {
						_netresponse = 2;
						Net2GameStarted = false;
					} else {
						Net2GameStarted = true;
					}
				} else if (Session.GPacket.Command==NET_LOADGAME) {
					JoinState = JOIN_GAME_START_LOAD;
					Net2ReadyToGo(1);
				}
			}
			continue;
		}

		//------------------------------------------------------------------------
		// NET_CHAT_ANNOUNCE: Someone is ready to chat; add them to our list, if
		// they aren't already on it, and it's not myself.
		//------------------------------------------------------------------------
		if (Session.GPacket.Command==NET_CHAT_ANNOUNCE) {
			found = 0;
			//.....................................................................
			// If this packet is from myself, don't add it to the list
			//.....................................................................
			if (Session.GPacket.Chat.ID == Session.UniqueID) {
				found = 1;
				continue;
			}
			//.....................................................................
			// Otherwise, see if we already have this address stored in our list
			// If so, update that node's time values & name (in case the user
			// changed it), and return.
			//.....................................................................
			else {
				for (i = 0; i < Session.Chat.Count(); i++) {
					if (Session.Chat[i]->Address==Session.GAddress) {
						strcpy (Session.Chat[i]->Name, Session.GPacket.Name);
						Session.Chat[i]->Chat.LastTime = TickCount;
						Session.Chat[i]->Chat.LastChance = 0;
						Session.Chat[i]->Chat.Color = Session.GPacket.Chat.Color;
						found = 1;
						break;
					}
				}
			}
			//.....................................................................
			// Add a new node to the list
			//.....................................................................
			if (!found) {
				who = new NodeNameType;
				UTF8::Copy(who->Name, sizeof(who->Name), Session.GPacket.Name);
				who->Address = Session.GAddress;
				who->Chat.LastTime = TickCount;
				who->Chat.LastChance = 0;
				who->Chat.Color = Session.GPacket.Chat.Color;
				Session.Chat.Add (who);
			}

			if (found) {
				if (CurGame == 0) {
				}
			}

			continue;
		}

		//------------------------------------------------------------------------
		// NET_CHAT_REQUEST: Someone is requesting a CHAT_ANNOUNCE from us; send
		// one to him directly.
		//------------------------------------------------------------------------
		if (Session.GPacket.Command==NET_CHAT_REQUEST) {
			if (JoinState != JOIN_WAIT_CONFIRM && JoinState != JOIN_CONFIRMED) {
				GlobalPacketType packet;

				memset (&packet, 0, sizeof(GlobalPacketType));

				packet.Command = NET_CHAT_ANNOUNCE;
				strcpy(packet.Name, Session.Handle);
				packet.Chat.ID = Session.UniqueID;
				packet.Chat.Color = Session.ColorIdx;

				Ipx.Send_Global_Message (&packet,
					sizeof(GlobalPacketType), 1, &Session.GAddress);

				Call_Back();
			}
			continue;
		}

		//------------------------------------------------------------------------
		// NET_MESSAGE: Someone is sending us a message
		//------------------------------------------------------------------------
		if (Session.GPacket.Command==NET_MESSAGE) {
			//.....................................................................
			// If we're in a game, the sender must be in our game.
			//.....................................................................
			if ( JoinState==JOIN_CONFIRMED) {
				if (Session.GPacket.Message.NameCRC ==
					Compute_Name_CRC(Session.GameName)) {
					PMessagePrintf(ColorUser, "[%s] %s", Session.GPacket.Name, Session.GPacket.Message.Buf);
					Sound_Effect(Rule->IncomingMessage);
				}
			}
			//.....................................................................
			// Otherwise, we're in the chat room; display any old message.
			//.....................................................................
			else {
				PMessagePrintf(ColorUser, "[%s] %s", Session.GPacket.Name, Session.GPacket.Message.Buf);
				Sound_Effect(Rule->IncomingMessage);
			}
			continue;
		}

		//------------------------------------------------------------------------
		// NET_QUERY_JOIN:
		//------------------------------------------------------------------------
		if (Session.GPacket.Command==NET_QUERY_JOIN) {
			GlobalPacketType packet = {};

			if (!Session.Players.Count()) {
				memset (&packet, 0, sizeof(GlobalPacketType));
				packet.Command = NET_REJECT_JOIN;
				packet.Reject.Why = (int)REJECT_DISBANDED;
				Ipx.Send_Global_Message (&packet, sizeof (GlobalPacketType),
					1, &Session.GAddress);
				continue;
			}

			//.....................................................................
			// See if this name is unique:
			// - If the name matches, but the address is different, reject this player
			// - If the name & address match, this packet must be a re-send of a
			//	  previous request; in this case, do nothing.  The other player must have
			//   received my CONFIRM_JOIN packet (since it was sent with an ACK
			//   required), so we can ignore this resend.
			//.....................................................................
			found = 0;
			resend = 0;
			for (i = 1; i < Session.Players.Count(); i++) {
				if (!strcmp(Session.Players[i]->Name,Session.GPacket.Name)) {
					if (Session.Players[i]->Address != Session.GAddress) {
						found = 1;
					}
					else {
						resend = 1;
					}
					break;
				}
			}
			//.....................................................................
			// If his name is the same as mine, treat it like a duplicate name
			//.....................................................................
			if (!strcmp (Session.Players[0]->Name, Session.GPacket.Name)) {
				found = 1;
			}

			//.....................................................................
			// Reject if name is a duplicate
			//.....................................................................
			if (found) {
				memset (&packet, 0, sizeof(GlobalPacketType));
				packet.Command = NET_REJECT_JOIN;
				packet.Reject.Why = (int)REJECT_DUPLICATE_NAME;
				Ipx.Send_Global_Message (&packet, sizeof (GlobalPacketType),
					1, &Session.GAddress);
				continue;
			}

			//.....................................................................
			// Reject if there are too many players
			//.....................................................................
			else if ( (Session.Players.Count() >= Session.MaxPlayers) && !resend) {
				memset (&packet, 0, sizeof(GlobalPacketType));
				packet.Command = NET_REJECT_JOIN;
				packet.Reject.Why = (int)REJECT_GAME_FULL;
				Ipx.Send_Global_Message (&packet, sizeof (GlobalPacketType),
					1, &Session.GAddress);
				continue;
			}

			bool match = true;
			if (!resend) {
				DebugString("RulesID = %lX\n", RulesID);
				DebugString("ArtID = %lX\n", ArtID);
				DebugString("AIID = %lX\n", AIID);
				DebugString("BuildNumber = %ld\n", Build_Number());
				DebugString("RuleINI ID = %lX\n", RuleINI->Get_Unique_ID());
				DebugString("FSRuleINI = %lX\n", FSRuleINI.Get_Unique_ID());
				if (Session.GPacket.PlayerInfo.CheatCheck != RulesID) { match = false; }
				if (Session.GPacket.PlayerInfo.AICheatCheck != RulesClass::Get_AI_Unique_ID()) { match = false; }
				if (Session.GPacket.PlayerInfo.ArtCheatCheck != ArtID) { match = false; }
				if (Session.GPacket.PlayerInfo.BuildNumber != Build_Number()) { match = false; }
			}
			int dups = 0;
			for (i = 0; i < Session.Players.Count(); i++) {
				if (!strcmp(Session.Players[i]->Player.Serial, Session.GPacket.Serial)) {
					dups++;
				}
			}

			if (dups >= 2 && !resend) {
				memset (&packet, 0, sizeof(GlobalPacketType));
				packet.Command = NET_REJECT_JOIN;
				packet.Reject.Why = (int)REJECT_DUPLICATE_SERIAL;
				Ipx.Send_Global_Message (&packet, sizeof (GlobalPacketType), 1, &Session.GAddress);
				continue;
			}

			/*
			**	Don't allow joining if the rules.ini file doesn't appear to match.
			*/
			if (!match) {
				memset (&packet, 0, sizeof(GlobalPacketType));
				packet.Command = NET_REJECT_JOIN;
				packet.Reject.Why = (int)REJECT_MISMATCH;
				Ipx.Send_Global_Message (&packet, sizeof (GlobalPacketType), 1, &Session.GAddress);
				continue;
			}

			//.....................................................................
			// If this packet is NOT a resend, accept the player.  Grant him the
			// requested color if possible.
			//.....................................................................
			if (!resend) {
				//..................................................................
				// Check the player's version range against our own, to see if
				// there's an overlap region
				//..................................................................
				version = VerNum.Clip_Version (Session.GPacket.PlayerInfo.MinVersion,
					Session.GPacket.PlayerInfo.MaxVersion);
				Session.PlayingAgainstVersion = version;

				//..................................................................
				// Reject player if his version is too old
				//..................................................................
				if (version == 0) {
					memset (&packet, 0, sizeof(GlobalPacketType));
					packet.Command = NET_REJECT_JOIN;
					packet.Reject.Why = (int)REJECT_VERSION_TOO_OLD;
					Ipx.Send_Global_Message (&packet, sizeof (GlobalPacketType),
						1, &Session.GAddress);
					continue;
				}

				//..................................................................
				// Reject player if his version is too new
				//..................................................................
				if (version == 0xffffffff) {
					memset (&packet, 0, sizeof(GlobalPacketType));
					packet.Command = NET_REJECT_JOIN;
					packet.Reject.Why = (int)REJECT_VERSION_TOO_NEW;
					Ipx.Send_Global_Message (&packet, sizeof (GlobalPacketType),
						1, &Session.GAddress);
					continue;
				}

				//..................................................................
				// If the player is accepted, our mutually-accepted version may be
				// different; set the CommProtocol accordingly.
				//..................................................................
				Session.CommProtocol = VerNum.Version_Protocol(version);

				//..................................................................
				// Add node to the Vector list
				//..................................................................
				who = new NodeNameType;
				UTF8::Copy(who->Name, sizeof(who->Name), Session.GPacket.Name);
				who->Address = Session.GAddress;
				who->Player.House = Session.GPacket.PlayerInfo.House;
				strcpy(who->Player.Serial, Session.GPacket.Serial);

				//..................................................................
				//	Set player's color; if requested color isn't used, give it to him;
				// otherwise, give him the 1st available color.  Mark the color we
				// give him as used.
				//..................................................................
				int oldcolor = who->Player.Color;
				int newcolor = Net2FirstFreeColor(Session.GPacket.PlayerInfo.Color, -1);
				if (newcolor != Session.GPacket.PlayerInfo.Color) {
					newcolor = Net2FirstFreeColor(oldcolor, -1);
				}
				who->Player.Color = newcolor;
				Session.Players.Add (who);

				for (i = 0; i < Session.Players.Count(); i++) {
					if (strcmp(Session.Players[i]->Name, Session.GameName)) {
						Session.Players[i]->Player.Status = 0;
					}
				}

				//..................................................................
				// Send a confirmation packet
				//..................................................................
				memset (&packet, 0, sizeof(GlobalPacketType));

				packet.Command = NET_CONFIRM_JOIN;
				strcpy(packet.Name,Session.Handle);
				packet.PlayerInfo.House = who->Player.House;
				packet.PlayerInfo.Color = who->Player.Color;

				Ipx.Send_Global_Message (&packet, sizeof (GlobalPacketType),
					1, &Session.GAddress);

				//..................................................................
				// Play a special sound.
				//..................................................................
				Sound_Effect(Rule->PlayerJoined);

				PumpGameopts(true);
			}
		}

		//------------------------------------------------------------------------
		// NET_PING: Someone is pinging me to get a response time measure (will only
		//	happen after I've joined a game).  Do nothing; the IPX Manager will handle
		// sending an ACK, and updating the response time measurements.
		//------------------------------------------------------------------------
		else if (Session.GPacket.Command==NET_PING) {
		}

		//------------------------------------------------------------------------
		// Default case: nothing happened.  (This case will be hit every time I
		//	receive my own NET_QUERY_GAME or NET_QUERY_PLAYER packets.)
		//------------------------------------------------------------------------
		else {
		}
	}

}	/* end of Get_Join_Responses */


/// The load_game argument separates NET_GO from NET_LOADGAME, but both cases are treated
/// alike here, so the argument is vestigial.

/// <summary>
/// Determines if this machine is ready for the game to begin.
/// The scenario the host has chosen is located, and fetched from the host when this
/// machine does not already have it. A scenario that cannot be had at all is fatal to the
/// join -- the player signs off rather than sit in a game it could never play. The
/// network timing is primed from the measured response times before returning.
/// </summary>
/// <returns>bool; Is this machine ready to go?</returns>
bool Net2ReadyToGo(int load_game)
{
	GlobalPacketType packet = {};
	int i;

	Ipx.Set_Timing(Ipx.Global_Response_Time() + 2 > 30 ? Ipx.Global_Response_Time () + 2 : 30, (unsigned int) -1, 1000);

	/*
	**	If the scenario that the host wants to play doesnt exist locally then we
	**	need to request that it is sent. If we can identify the scenario locally then
	**	we need to fix up the file name so we load the right one.
	*/
	if (Find_Local_Scenario (Session.ScenarioFileName, Session.ScenarioFileLength, Session.ScenarioDigest, Session.ScenarioIsOfficial) == true) {
		DebugString("Found local scenario, file name is %s\n", Session.ScenarioFileName);

		/*
		** We have the scenario. Tell the host that I am ready to go.
		*/
		memset (&packet, 0, sizeof (packet));
		packet.Command = NET_READY_TO_GO;
		Ipx.Send_Global_Message (&packet, sizeof (packet), 1, &Session.HostAddress);
		while (Ipx.Global_Num_Send() > 0 && Ipx.Service() != 0) {
			Call_Back();
		}

	} else {
		//.....................................................................
		// If the other guys are playing a scenario I don't have (sniff), I can't
		// play.  Try to bail gracefully.
		//.....................................................................
		DebugString("Failed to find local scenario, file name is %s\n", Session.ScenarioFileName);

		if ((Session.ScenarioIsOfficial && strcmpi(Session.ScenarioFileName, RANDOM_MAP_FILE_NAME) != 0) || !Get_File_From_Host(Session.ScenarioFileName, true)) {
			Session.Options.ScenarioIndex = -1;
			WWMessageBox().Process(TXT_UNABLE_PLAY_WAAUGH, TXT_OK);
			memset (&packet, 0, sizeof(packet));

			packet.Command = NET_SIGN_OFF;
			strcpy (packet.Name, Session.Handle);

			//..................................................................
			// Don't send myself the message.
			//..................................................................
			for (i = 1; i < Session.Chat.Count(); i++) {
				Ipx.Send_Global_Message (&packet, sizeof(packet), 1, &(Session.Chat[i]->Address));
				Call_Back();
			}

			Ipx.Send_Global_Message(&packet, sizeof (packet), 0, NULL);
			while (Ipx.Global_Num_Send() > 0 && Ipx.Service() != 0) {
				Call_Back();
			}
			return(false);
		}
	}

	//---------------------------------------------------------------------
	// Prepare to load the scenario.
	//---------------------------------------------------------------------
	strcpy(Scen->ScenarioName, Session.ScenarioFileName);
	int retrydelta = Ipx.Global_Response_Time();
	if (retrydelta < 20) {
		DebugString("IPX.Global_Response_Time() == %d. Adjusting to 20\n", retrydelta);
		retrydelta = 20;
	}

	//------------------------------------------------------------------------
	// Init network timing values, using previous response times as a measure
	// of what our retry delta & timeout should be.
	//------------------------------------------------------------------------
	Ipx.Set_Timing(retrydelta, (unsigned int) -1, std::max(2 * TIMER_SECOND, (int)retrydelta * 8));
	Ipx.Set_External_Timing(TIMER_SECOND, -1, 10 * TIMER_SECOND);

	return(true);
}
