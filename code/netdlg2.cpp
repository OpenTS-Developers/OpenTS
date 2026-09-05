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
#include "newmenu.h"
#include "ownrdraw.h"
#include "rules.h"
#include "scenario.h"
#include "sendfile.h"
#include "srfcache.h"
#include "stimer.h"
#include "timer.h"
#include "windlg.h"
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

BOOL CALLBACK MPlayer_Guest_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
BOOL CALLBACK MPlayer_Game_List_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
BOOL CALLBACK MPlayer_Host_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
bool Net2ReadyToGo(int load_game);

int CurGame;
int _netresponse;
JoinStateType JoinState;
bool Net2IsGameListActive = true;
bool Net2GameStarted = false;

int RulesID;
int ArtID;
int AIID;

int Net2_g_Col_Accept;
int Net2_g_Col_Name;
int Net2_g_Col_House;


/// <summary>
/// Fetches a player color that nobody else has claimed.
/// This routine is used when a player asks for a color, so that no two players in the
/// same game end up wearing the same one.
/// </summary>
/// <param name="reqcolor">The color the player would prefer to have.</param>
/// <param name="index">The player doing the asking, so that its own color is not counted
/// against it.</param>
/// <returns>Returns with the color the player should use.</returns>
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


void _Net2DisplayUsers(void);


/// <summary>
/// Redisplays the list of users.
/// This routine is the entry point the network code reaches for whenever a change to the
/// user list has to find its way onto the screen.
/// </summary>
void Net2DisplayUsers(void)
{
	_Net2DisplayUsers();
}

/// <summary>
/// Redisplays the list of users.
/// Out in the lobby this is the roster of everyone chatting. Within a game it becomes the
/// player list, each row carrying the player's color, house emblem, and ready marker. The
/// selection and the scroll position survive the rebuild.
/// </summary>
void _Net2DisplayUsers(void)
{
	int i;
	int color;
	char hname[128];
	char info[128];
	Surface * surf = NULL;

	HWND win=WS_Top_Window();

	HWND userwin=GetDlgItem(win,IDC_USERS);

	if (win==NULL || userwin==NULL) {
		return;
	}

	OwnerDraw::CellData thecell;

	int topindex=SendDlgItemMessage(win, IDC_USERS, LB_GETTOPINDEX, 0, 0);

	SendDlgItemMessage(win, IDC_USERS, OD_DISABLEPAINT, 0, TRUE);

	Dictionary<Wstring,bool> lbdict(Wstring_Hash);
	LBSaveSelections(userwin, lbdict);

	SendDlgItemMessage(win, IDC_USERS, LB_RESETCONTENT, NULL, NULL);

	if (CurGame == 0) {
		SendDlgItemMessage(win, IDC_USERS, LB_INSERTSTRING, (WPARAM)0, (LPARAM)Session.Handle);

		for (i = 1; i < Session.Chat.Count(); i++) {
			SendDlgItemMessage(win, IDC_USERS, LB_INSERTSTRING, (WPARAM)i, (LPARAM)Session.Chat[i]->Name);
		}

	} else {

		for (i = 0; i < Session.Players.Count(); i++) {
			int type = 0;

			if (!strcmp(Session.Players[i]->Name, Session.GameName)) {
				Session.Players[i]->Player.Status = 1;
				type = 2;
			} else if (Session.Players[i]->Player.Status != 0) {
				type = 1;
			}

			sprintf(info, "%s", Session.Players[i]->Name);

			if (Session.Players[i]->Player.House == HOUSE_GOOD) {
				sprintf(hname, "%s", Fetch_String(TXT_GDI));
				surf = SurfaceCache.GetSurface("gdii.pcx");
			} else {
				sprintf(hname, "%s", Fetch_String(TXT_NOD));
				surf = SurfaceCache.GetSurface("nodi.pcx");
			}

			SendDlgItemMessage(win, IDC_USERS, LB_INSERTSTRING, (WPARAM) -1, (LPARAM)info);

			color = PlayerColorTable[Session.Players[i]->Player.Color];

			thecell.type = OwnerDraw::CellData::PRIMARY;
			thecell.color = color;
			thecell.hint.set("");
			SendDlgItemMessage(win, IDC_USERS, OD_SETCELL, MAKEWPARAM(Net2_g_Col_Name,i),(LPARAM)&thecell);

			thecell.type = OwnerDraw::CellData::SURFACE;
			thecell.hint.set(hname);
			thecell.surf = surf;
			SendDlgItemMessage(win, IDC_USERS, OD_SETCELL, MAKEWPARAM(Net2_g_Col_House,i),(LPARAM)&thecell);

			thecell.hint.set("");
			thecell.type = OwnerDraw::CellData::SURFACE;
			if (type == 2) {
				thecell.surf=SurfaceCache.GetSurface("wolhost.pcx");
			} else if (type != 0) {
				thecell.surf=SurfaceCache.GetSurface("wolacpt.pcx");
			} else {
				thecell.type = OwnerDraw::CellData::INVALID;
			}
			SendDlgItemMessage(win, IDC_USERS, OD_SETCELL, MAKEWPARAM(Net2_g_Col_Accept,i),(LPARAM)&thecell);
		}

	}

	LBRestoreSelections(userwin, lbdict);
	SendDlgItemMessage(win, IDC_USERS, LB_SETTOPINDEX, (WPARAM)topindex, 0);
	SendDlgItemMessage(win, IDC_USERS, OD_DISABLEPAINT, 0, 0);
	InvalidateRect(userwin, NULL, 0);
	UpdateWindow(userwin);
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

				Net2DisplayGameList();
				if (i <= CurGame) {
					Net2DisplayUsers();
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
			Net2DisplayUsers();
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
/// Redisplays the list of games available to join.
/// The lobby heads the list and the games other hosts are advertising follow it. The
/// current selection is clamped and re-applied, since a game can vanish out from under
/// the player at any moment.
/// </summary>
void Net2DisplayGameList(void)
{
	char buffer[80];

	HWND window = WS_Top_Window();

	int count = Session.Games.Count();
	if (CurGame >= count) {
		CurGame = count - 1;
		Send_Join_Queries(0, 1, 0, 0);
	}

	if (CurGame < 0) {
		CurGame = 0;
	}

	int top = SendDlgItemMessage(window, IDC_GAMELIST, LB_GETTOPINDEX, 0, 0);

	SendDlgItemMessage(window, IDC_GAMELIST, OD_DISABLEPAINT, 0, 1);
	SendDlgItemMessage(window, IDC_GAMELIST, LB_RESETCONTENT, 0, 0);
	SendDlgItemMessage(window, IDC_GAMELIST, LB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_LOBBY));

	for (int i = 1; i < Session.Games.Count(); i++) {
		NodeNameType *node = Session.Games[i];
		if (node->Game.IsOpen) {
			sprintf(buffer, Fetch_String(TXT_THATGUYS_GAME), node);
		} else {
			sprintf(buffer, Fetch_String(TXT_THATGUYS_GAME_BRACKET), node);
		}
		SendDlgItemMessage(window, IDC_GAMELIST, LB_INSERTSTRING, -1, (LPARAM)buffer);
	}

	int idx = CurGame;
	SendDlgItemMessage(window, IDC_GAMELIST, LB_SETCURSEL, idx, 0);
	SendDlgItemMessage(window, IDC_GAMELIST, LB_SETTOPINDEX, top, 0);
	SendDlgItemMessage(window, IDC_GAMELIST, OD_DISABLEPAINT, 0, 0);

	HWND handle = GetDlgItem(window, IDC_GAMELIST);
	InvalidateRect(handle, NULL, FALSE);
	UpdateWindow(handle);
}


/// <summary>
/// Encodes the current game options into a text string.
/// This routine builds the blob the host broadcasts to the guests, so that every machine
/// agrees on the rules, the scenario, and who is playing what.
/// </summary>
/// <param name="out">Buffer to build the encoded option string within.</param>
/// <remarks>Be sure the destination buffer is big enough for the options and an entry for
/// every player in the game.</remarks>
void Net2EncodeGameopt(char *out)
{
	static char useroptions[512];

	sprintf(out,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
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

	memset(useroptions, 0, sizeof(useroptions));

	for (int i = 0; i < Session.Players.Count(); i++) {
		sprintf(useroptions + strlen(useroptions), "%s,%d,%d,", Session.Players[i]->Name, Session.Players[i]->Player.House,
				Session.Players[i]->Player.Color);
	}

	strcat(out, useroptions);
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
	Net2DisplayUsers();
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
		Net2DisplayUsers();
	}

	if (offset == 0) {
		HWND win=WS_Find_Dialog(IDD_MPLAYER_HOST);
		Session.PrefColor = color;
		if (win == NULL) {
			win = WS_Find_Dialog(IDD_MPLAYER_GUEST);
		}
		if ((SendDlgItemMessage(win,IDC_YOURCOLOR,CB_GETCURSEL,0,0) != color) &&
			(SendDlgItemMessage(win,IDC_YOURCOLOR,CB_GETDROPPEDSTATE,0,0) == FALSE)) {
			SendDlgItemMessage(win,IDC_YOURCOLOR,CB_SETCURSEL,color,0);
		}
	}

	if (offset == 0) {
		Session.ColorIdx=color;
		Session.House=house;
	}

	return(retval);
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
bool Net2Remote_Connect(void)
{
	RulesID = RulesClass::Get_Rule_Unique_ID();
	RulesClass::Load_Art_INI();
	ArtID = RulesClass::Get_Art_Unique_ID();
	AIID = RulesClass::Get_AI_Unique_ID();

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

	OwnerDraw::Register_Control_Classes();

	HWND game_list_dialog = WS_Create_Dialog(ProgramInstance, IDD_MPLAYER_GAME_LIST, MainWindow, MPlayer_Game_List_Dialog_Proc, FALSE);
	Center_Window_Within_Window(game_list_dialog);
	OwnerDraw::Subclass_Dialog(game_list_dialog, 0);
	ShowWindow(game_list_dialog, SW_SHOWNORMAL);
	Net2DisplayUsers();

	_netresponse = 0;
	CurGame = 0;
	JoinState = JOIN_NOTHING;
	Net2IsGameListActive = true;

	//------------------------------------------------------------------------
	// Keep looping until something useful happens.
	//------------------------------------------------------------------------
	while (true) {
		//.....................................................................
		// Pop up the network Join/New dialog
		//.....................................................................
		while (_netresponse == 0) {
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
				break;
			}

			if (WS_Top_Window()) {
				Send_Join_Queries(false, false, false, false);
				Get_Join_Responses();
				if (WS_Top_Window_ID() == IDD_MPLAYER_HOST) {
					PumpGameopts(false);
				}
				Net2ServiceGameList();
			}
		}

		//.....................................................................
		//	-1 = user selected Cancel
		//.....................................................................
		if (_netresponse == IDCANCEL) {
			Session.Write_MultiPlayer_Settings();
			if (WS_Top_Window_ID() == IDD_MPLAYER_GAME_LIST) {
				if (JoinState > JOIN_NOTHING) {
					Unjoin_Game(CurGame);
					Ipx.Service();
				}
				WS_Destroy_Dialog(NULL, 0);
				Clear_Vector(&Session.Players);
				Clear_Vector(&Session.Games);
				Clear_Vector(&Session.Chat);
				Session.NetOpen = false;
				Ipx.Service();
				return(false);
			}

			if (WS_Top_Window_ID() == IDD_MPLAYER_HOST) {
				Unjoin_Game(CurGame);
				JoinState = JOIN_NOTHING;
				WS_Destroy_Dialog(WS_Top_Window(), 0);
				game_list_dialog = WS_Create_Dialog(ProgramInstance, IDD_MPLAYER_GAME_LIST, MainWindow, MPlayer_Game_List_Dialog_Proc, 0);
				Center_Window_Within_Window(game_list_dialog);
				OwnerDraw::Subclass_Dialog(game_list_dialog, 0);
				ShowWindow(game_list_dialog, SW_SHOWNORMAL);
				Send_Join_Queries(false, false, true, false);
			}

			if (WS_Top_Window_ID() == IDD_MPLAYER_GUEST) {
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
				WS_Destroy_Dialog(0, 0);
				_netresponse = 0;
				CurGame = 0;
				Clear_Vector(&Session.Players);
				game_list_dialog = WS_Create_Dialog(ProgramInstance, IDD_MPLAYER_GAME_LIST, MainWindow, MPlayer_Game_List_Dialog_Proc, 0);
				Center_Window_Within_Window(game_list_dialog);
				OwnerDraw::Subclass_Dialog(game_list_dialog, 0);
				ShowWindow(game_list_dialog, SW_SHOWNORMAL);
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

				WS_Destroy_Dialog(NULL, NULL);
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
				HWND host_dialog = WS_Create_Dialog(ProgramInstance, IDD_MPLAYER_HOST, MainWindow, MPlayer_Host_Dialog_Proc, 0);
				Center_Window_Within_Window(host_dialog);
				OwnerDraw::Subclass_Dialog(host_dialog, 0);
				SendMessage(host_dialog, OD_SETTOP, 0, 1);
				ShowWindow(host_dialog, SW_SHOWNORMAL);
			}
		}

		if (_netresponse != 1 || WS_Top_Window_ID() != IDD_MPLAYER_GUEST) {
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
						EnableWindow(GetDlgItem(WS_Top_Window(), IDC_GO), TRUE);
					}

					if (_netresponse == IDC_GO) {
						for (int i = 0; i < Session.Players.Count(); i++) {
							if (Session.Players[i]->Player.Status == 0) {
								PMessagePrintf(-1, Fetch_String(TXT_ACCEPTFIRST));
								_netresponse = 0;
								EnableWindow(GetDlgItem(WS_Top_Window(), IDC_GO), TRUE);
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
			while (WS_Destroy_Dialog(NULL, 0) == true) {}
			_netresponse = 0;

			PregameSetup();

			//.....................................................................
			// Compute frame delay value for packet transmissions:
			// - Divide global channel's response time by 8 (2 to convert to 1-way
			//	  value, 4 more to convert from ticks to frames)
			//.....................................................................
			Session.LatencyFudge = 0;
			Session.PrecalcMaxAhead = 0;
			Session.PrecalcDesiredFrameRate = 0;
			Session.FrameSendRate = 3;
			if (Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP) {
				Session.MaxAhead = std::max<unsigned int>(((((Ipx.Global_Response_Time() / 8) + (Session.FrameSendRate - 1)) / Session.FrameSendRate) * Session.FrameSendRate), NETWORK_MIN_MAX_AHEAD * 3);
			} else {
				Session.MaxAhead = std::max(((int)Ipx.Global_Response_Time() / 8), NETWORK_MIN_MAX_AHEAD);
			}

			break;
		}

		int waypoints = RandomMapWaypointCount(Session.Options.ScenarioIndex);
		if (waypoints < SendDlgItemMessage(game_list_dialog, IDC_AIPLAYERS, TBM_GETPOS, 0, 0) + Session.Players.Count()) {
			PMessagePrintf(-1, Fetch_String(TXT_SCENARIO_TOO_SMALL));
			EnableWindow(GetDlgItem(WS_Top_Window(), IDC_GO), TRUE);
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

				//.....................................................................
				// Compute frame delay value for packet transmissions:
				// - Divide global channel's response time by 8 (2 to convert to 1-way
				//	  value, 4 more to convert from ticks to frames)
				//.....................................................................
				Session.FrameSendRate = 3;
				Session.LatencyFudge = 0;
				Session.PrecalcMaxAhead = 0;
				Session.PrecalcDesiredFrameRate = 0;
				if (Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP) {
					Session.MaxAhead = std::max<unsigned int>(((((Ipx.Global_Response_Time() / 8) + (Session.FrameSendRate - 1)) / Session.FrameSendRate) * Session.FrameSendRate), NETWORK_MIN_MAX_AHEAD * 3);
				} else {
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
				WS_Destroy_Dialog(NULL, 0);
				break;
			}
		}
	}

	Session.NetOpen = false;
	Session.Write_MultiPlayer_Settings();
	return(true);

} /* end of Remote_Connect */


/// <summary>
/// Handles the multiplayer game list dialog.
/// This is the lobby a player lands in before hosting or joining anything. It keeps the
/// game and user lists current, carries the lobby chat, and records which button was
/// pressed so that the driver loop knows whether to move on to the host or guest dialog.
/// </summary>
/// <returns>Returns with TRUE if the message was consumed by this dialog.</returns>
BOOL CALLBACK MPlayer_Game_List_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message) {

	case WM_INITDIALOG: {
		CurGame = 0;
		Net2IsGameListActive = 1;

		SendDlgItemMessage(window, IDC_YOURNAME, EM_SETLIMITTEXT, 16, 0);
		SetWindowText(GetDlgItem(window, IDC_YOURNAME), Session.Handle);

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

		NodeNameType * game = new NodeNameType;
		strcpy(game->Name, "");
		game->Game.IsOpen = 0;
		game->Game.LastTime = 0;
		Session.Games.Add(game);

		Send_Join_Queries(true, false, true, true);
		return(0);
	}

	case WM_COMMAND: {
		switch (LOWORD(wparam)) {

		case IDC_YOURNAME: {
			char name_buf[64];

			SendDlgItemMessage(window, IDC_YOURNAME, WM_GETTEXT, 63, (LPARAM)name_buf);

			if (strcmp(name_buf, Session.Handle)) {
				strcpy(Session.Handle, name_buf);
				Send_Join_Queries(0, 0, 1, 0);
				_Net2DisplayUsers();
			}

			return(0);
		}

		case IDCANCEL: {
			_netresponse = IDCANCEL;
			return(0);
		}

		case IDC_GAMELIST_NEW: {
			_netresponse = IDC_GAMELIST_NEW;
			return(0);
		}

		case IDC_INPUT: {
			char text[260];

			SendDlgItemMessage(window, IDC_INPUT, WM_GETTEXT, 256, (LPARAM)text);

			int len = strlen(text);

			if (HIWORD(wparam) == EN_MAXTEXT) {
				SendDlgItemMessage(window, IDC_INPUT, WM_SETTEXT, 0, (LPARAM) "");

				if (len > 2) {

					PMessagePrintf(ColorMe, "[%s] %s", Session.Handle, text);

					GlobalPacketType gpacket;
					memset(&gpacket, 0, sizeof(gpacket));

					gpacket.Command = NET_MESSAGE;
					strcpy(gpacket.Name, Session.Handle);
					strcpy(gpacket.Message.Buf, text);
					gpacket.Message.Color = Session.ColorIdx;
					gpacket.Message.NameCRC = Compute_Name_CRC(Session.GameName);

					if (JoinState == JOIN_CONFIRMED) {
						for (int i = 1; i < Session.Players.Count(); ++i) {
							Ipx.Send_Global_Message(&gpacket, sizeof(gpacket), 1, &Session.Players[i]->Address);
							Call_Back();
						}
					} else {
						for (int i = 1; i < Session.Chat.Count(); ++i) {
							Ipx.Send_Global_Message(&gpacket, sizeof(gpacket), 1, &Session.Chat[i]->Address);
							Call_Back();
						}
					}
				}
			}

			return(0);
		}

		case IDC_YOURCOLOR: {
			if (HIWORD(wparam) == LBN_SELCHANGE) {
				Session.ColorIdx = SendDlgItemMessage(window, IDC_YOURCOLOR, LB_GETCURSEL, 0, 0);
			}
			return(0);
		}

		case IDC_GAMELIST: {

			if (JoinState > JOIN_NOTHING) {
				return(0);
			}

			int old_game = CurGame;
			LRESULT sel = SendDlgItemMessage(window, IDC_GAMELIST, LB_GETCURSEL, 0, 0);

			if (sel >= 0 && Net2IsGameListActive) {
				CurGame = sel;
				strcpy(Session.GameName, Session.Games[sel]->Name);
			}

			if (HIWORD(wparam) == LBN_SELCHANGE) {
				Clear_Vector(&Session.Players);

				if (old_game != CurGame) {
					Send_Join_Queries(1, 1, 1, 0);
				}

				_Net2DisplayUsers();
				return(0);
			}

			if (HIWORD(wparam) == LBN_DBLCLK) {
				_netresponse = IDC_GAMELIST_JOIN;
				return(0);
			}

			return(0);
		}

		case IDC_GAMELIST_JOIN: {
			_netresponse = IDC_GAMELIST_JOIN;
			return(0);
		}
		}

		return(0);
	}

	case OD_SUBCLASSED: {
		Net2DisplayGameList();
		_Net2DisplayUsers();
		OwnerDraw::Draw_Dialog_Back(window);
		return(0);
	}

	case WM_DRAWITEM:
		OwnerDraw::Draw_Item((DRAWITEMSTRUCT *)lparam);
		return(1);

	case WM_PAINT:
		OwnerDraw::Draw_Dialog_Back(window);
		ValidateRect(window, NULL);
		return(0);

	case WM_ERASEBKGND:
		return(1);
	}

	return(0);
}


/// <summary>
/// Handles the multiplayer host dialog.
/// This is the setup dialog belonging to the player who created the game. It owns the
/// game option controls, the scenario picker, and the player list along with the means to
/// kick somebody out of it -- and finally the button that starts the match.
/// </summary>
/// <returns>Returns with TRUE if the message was consumed by this dialog.</returns>
BOOL CALLBACK MPlayer_Host_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	/*
	 * ------------------------------------------------------------------------
	 * When 'Net2GameStarted' is set the game is in progress; this whole
	 * preliminary dispatch is skipped. Option checkbox and slider changes
	 * are applied here first, and then the message falls through to the
	 * full dispatch.
	 * ------------------------------------------------------------------------
	 */
	if (!Net2GameStarted) {

		switch (message) {

		case WM_COMMAND:

			switch (LOWORD(wparam)) {

			/*
			 * ................................................................
			 * Bases. Turning bases off also forces the "short game" option off.
			 * ................................................................
			 */
			case IDC_BASES:
				Session.Options.Bases = false;
				if (SendDlgItemMessage(window, IDC_BASES, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					Session.Options.Bases = true;
				} else if (!Session.Options.Bases) {
					Session.Options.ShortGame = false;
					SendDlgItemMessage(window, IDC_SHORT_GAME, BM_SETCHECK, 0, 0);
				}
				break;

			/*
			 * ................................................................
			 * Redeploy MCV.
			 * ................................................................
			 */
			case IDC_REDEPLOY_MCV:
				Session.Options.MCVRedeploy = false;
				if (SendDlgItemMessage(window, IDC_REDEPLOY_MCV, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					Session.Options.MCVRedeploy = true;
				}
				break;

			/*
			 * ................................................................
			 * Crates / goodies.
			 * ................................................................
			 */
			case IDC_CRATES:
				Session.Options.Goodies = false;
				if (SendDlgItemMessage(window, IDC_CRATES, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					Session.Options.Goodies = true;
				}
				break;

			/*
			 * ................................................................
			 * Short game. Turning the short game on also forces bases on.
			 * ................................................................
			 */
			case IDC_SHORT_GAME:
				Session.Options.ShortGame = false;
				if (SendDlgItemMessage(window, IDC_SHORT_GAME, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					Session.Options.ShortGame = true;
					if (!Session.Options.Bases) {
						Session.Options.Bases = true;
						SendDlgItemMessage(window, IDC_BASES, BM_SETCHECK, 1, 0);
					}
				}
				break;

			/*
			 * ................................................................
			 * Multiplayer engineers.
			 * ................................................................
			 */
			case IDC_MULTI_ENGINEER:
				Session.Options.CrapEngineers = false;
				if (SendDlgItemMessage(window, IDC_MULTI_ENGINEER, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					Session.Options.CrapEngineers = true;
				}
				break;

			/*
			 * ................................................................
			 * Bridge destruction.
			 * ................................................................
			 */
			case IDC_BRIDGE_DESTROY:
				Session.Options.BridgeDestruction = false;
				if (SendDlgItemMessage(window, IDC_BRIDGE_DESTROY, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					Session.Options.BridgeDestruction = true;
				}
				break;

			/*
			 * ................................................................
			 * Allies allowed.
			 * ................................................................
			 */
			case IDC_ALLIES:
				Session.Options.AlliesAllowed = false;
				if (SendDlgItemMessage(window, IDC_ALLIES, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					Session.Options.AlliesAllowed = true;
				}
				break;

			/*
			 * ................................................................
			 * Harvester truce.
			 * ................................................................
			 */
			case IDC_HARVTRUCE:
				Session.Options.HarvTruce = false;
				if (SendDlgItemMessage(window, IDC_HARVTRUCE, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					Session.Options.HarvTruce = true;
				}
				break;

			/*
			 * ................................................................
			 * Fog of war.
			 * ................................................................
			 */
			case IDC_FOG_OF_WAR:
				Session.Options.FogOfWar = false;
				if (SendDlgItemMessage(window, IDC_FOG_OF_WAR, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					Session.Options.FogOfWar = true;
				}
				break;
			}
			break;

		/*
		 * ....................................................................
		 * The option sliders are read back per-control here; the values are
		 * all unconditionally re-read by the main WM_HSCROLL/WM_VSCROLL
		 * handler below.
		 * ....................................................................
		 */
		case WM_HSCROLL:
			if (GetDlgItem(window, IDC_AILEVEL_SLIDER) == (HWND)lparam) {
				Session.Options.AIDifficulty = (DiffType)SendDlgItemMessage(window, IDC_AILEVEL_SLIDER, TBM_GETPOS, 0, 0);
			}
			if (GetDlgItem(window, IDC_AIPLAYERS) == (HWND)lparam) {
				Session.Options.AIPlayers = SendDlgItemMessage(window, IDC_AIPLAYERS, TBM_GETPOS, 0, 0);
			}
			if (GetDlgItem(window, IDC_UNITCOUNT) == (HWND)lparam) {
				Session.Options.UnitCount = SendDlgItemMessage(window, IDC_UNITCOUNT, TBM_GETPOS, 0, 0);
			}
			if (GetDlgItem(window, IDC_TECHLEVEL) == (HWND)lparam) {
				BuildLevel = SendDlgItemMessage(window, IDC_TECHLEVEL, TBM_GETPOS, 0, 0);
			}
			if (GetDlgItem(window, IDC_CREDITS) == (HWND)lparam) {
				Session.Options.Credits = SendDlgItemMessage(window, IDC_CREDITS, TBM_GETPOS, 0, 0);
			}
			if (GetDlgItem(window, IDC_GAME_SPEED_SLIDER) == (HWND)lparam) {
				Session.Options.GameSpeed = 6 - SendDlgItemMessage(window, IDC_GAME_SPEED_SLIDER, TBM_GETPOS, 0, 0);
			}
			break;

		/*
		 * ....................................................................
		 * Refresh the game option controls.
		 * ....................................................................
		 */
		case WM_INITDIALOG:
		case OD_SUBCLASSED:
			DisplayGameopts(window, 1);
			break;
		}
	}

	switch (message) {

	case WM_DESTROY:
		return(0);

	case WM_DRAWITEM:
		OwnerDraw::Draw_Item((DRAWITEMSTRUCT *)lparam);

	case WM_ERASEBKGND:
		return(TRUE);

	case WM_PAINT:
		OwnerDraw::Draw_Dialog_Back(window);
		if (MultiplayerMapPreview != NULL) {
			MultiplayerMapPreview->Blit_Preview(window);
		}
		ValidateRect(window, NULL);
		return(0);

	case WM_INITDIALOG: {
		VerNum.Init_Clipping();

		srand(NonCriticalRandomNumber(1, 0x7FFF));
		Seed = rand();

		Set_Scenario_Info_From_Index(0);
		Session.Options.ScenarioIndex = 0;

		Center_Window_Within_Window(window);

		SendDlgItemMessage(window, IDC_YOURSIDE, CB_RESETCONTENT, 0, 0);

		for (int i = 0; i < HouseTypes.Count(); ++i) {
			HouseTypeClass * house = HouseTypes[i];
			if (house->IsMultiplay) {
				SendDlgItemMessage(window, IDC_YOURSIDE, CB_INSERTSTRING, -1, (LPARAM)(char const *)house->GivenName);
			}
		}

		SendDlgItemMessage(window, IDC_YOURSIDE, CB_SETCURSEL, Session.House, 0);

		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_RESETCONTENT, 0, 0);

		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_GOLD));
		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_RED));
		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_BLUE));
		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_GREEN));
		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_ORANGE));
		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_SKY_BLUE));
		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_PURPLE));
		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_INSERTSTRING, -1, (LPARAM)Fetch_String(TXT_PINK));

		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_SETCURSEL, Session.ColorIdx, 0);

		SendDlgItemMessage(window, IDC_AILEVEL_SLIDER, LB_RESETCONTENT, 0, 0);

		for (int j = 0; j < Session.Scenarios.Count(); ++j) {
			SendDlgItemMessage(window, IDC_AILEVEL_SLIDER, LB_INSERTSTRING, -1, (LPARAM)Session.Scenarios[j]);
		}

		SendDlgItemMessage(window, IDC_SCENARIONAME, WM_SETTEXT, 0, (LPARAM)Session.Options.ScenarioDescription);

		Update_Network_Dialog_Preview(window);

		SendMessage(window, WM_COMMAND, MAKEWPARAM(IDC_YOURSIDE, CBN_SELCHANGE), (LPARAM)GetDlgItem(window, IDC_YOURSIDE));
		SendMessage(window, WM_COMMAND, MAKEWPARAM(IDC_YOURCOLOR, CBN_SELCHANGE), (LPARAM)GetDlgItem(window, IDC_YOURCOLOR));

		return(0);
	}

	case WM_HSCROLL:
	case WM_VSCROLL: {
		if (Net2GameStarted) return(0);

		Session.Options.UnitCount = SendDlgItemMessage(window, IDC_UNITCOUNT, TBM_GETPOS, 0, 0);
		BuildLevel = SendDlgItemMessage(window, IDC_TECHLEVEL, TBM_GETPOS, 0, 0);
		Session.Options.Credits = SendDlgItemMessage(window, IDC_CREDITS, TBM_GETPOS, 0, 0);
		Session.Options.AIPlayers = SendDlgItemMessage(window, IDC_AIPLAYERS, TBM_GETPOS, 0, 0);
		Session.Options.AIDifficulty = (DiffType)SendDlgItemMessage(window, IDC_AILEVEL_SLIDER, TBM_GETPOS, 0, 0);
		Session.Options.GameSpeed = 6 - SendDlgItemMessage(window, IDC_GAME_SPEED_SLIDER, TBM_GETPOS, 0, 0);

		return(0);
	}

	case WM_COMMAND: {
		switch (LOWORD(wparam)) {

		case IDC_YOURSIDE:
			if (HIWORD(wparam) == CBN_SELCHANGE && !Net2GameStarted) {
				Session.House = SendDlgItemMessage(window, IDC_YOURSIDE, CB_GETCURSEL, 0, 0);
				Session.Players[0]->Player.House = Session.House;

				PumpGameopts(1, 0);
				_Net2DisplayUsers();
			}
			return(0);

		case IDC_INPUT:
		{
			char text[260];
			SendDlgItemMessage(window, IDC_INPUT, WM_GETTEXT, 256, (LPARAM)text);

			int len = strlen(text);

			if (HIWORD(wparam) != EN_MAXTEXT) return(0);

			SendDlgItemMessage(window, IDC_INPUT, WM_SETTEXT, 0, (LPARAM)"");

			if (len <= 2) return(0);

			PMessagePrintf(ColorMe, "[%s] %s", Session.Handle, text);

			GlobalPacketType gpacket;
			memset(&gpacket, 0, sizeof(gpacket));

			gpacket.Command = NET_MESSAGE;
			strcpy(gpacket.Name, Session.Handle);
			strcpy(gpacket.Message.Buf, text);
			gpacket.Message.Color = Session.ColorIdx;
			gpacket.Message.NameCRC = Compute_Name_CRC(Session.GameName);


			if (JoinState == JOIN_CONFIRMED) {
				for (int i = 1; i < Session.Players.Count(); ++i) {
					Ipx.Send_Global_Message(&gpacket, sizeof(gpacket), 1, &Session.Players[i]->Address);
					Call_Back();
				}
			} else {
				for (int i = 1; i < Session.Chat.Count(); ++i) {
					Ipx.Send_Global_Message(&gpacket, sizeof(gpacket), 1, &Session.Chat[i]->Address);
					Call_Back();
				}
			}

			return(0);
		}

		case IDCANCEL: {
			if (Net2GameStarted) return(0);

			if (MultiplayerMapPreview != NULL) {
				delete MultiplayerMapPreview;
				MultiplayerMapPreview = NULL;
			}

			_netresponse = IDCANCEL;
			return(0);
		}

		/*
		 * ....................................................................
		 * The user picked a color. Resolve it against the colors already in
		 * use, skipping our own slot (index 0). If the chosen color is taken,
		 * bump forward (mod 8) until a free color is found; if that changed
		 * the color from what the user wanted, warn and re-resolve starting
		 * from our previous color.
		 * ....................................................................
		 */
		case IDC_YOURCOLOR:
			if (HIWORD(wparam) == CBN_SELCHANGE && !Net2GameStarted) {

				int old_color = Session.ColorIdx;

				Session.ColorIdx = SendDlgItemMessage(window, IDC_YOURCOLOR, CB_GETCURSEL, 0, 0);
				Session.PrefColor = Session.ColorIdx;

				int newcolor;
				int found;
				int color = Session.ColorIdx;
				for (;;) {
					int count = 0;
					newcolor = color;
					found = FALSE;

					while (count < Session.Players.Count()) {
						if (count != 0 && Session.Players[count]->Player.Color == color) {
							color++;
							found = TRUE;
						}
						count++;
					}

					if (!found) break;

					color %= MAX_MPLAYER_COLORS;
				}

				int resolved = newcolor;
				if (newcolor != Session.ColorIdx) {
					PMessagePrintf(ColorSystem, Fetch_String(TXT_COLOR_IN_USE));

					color = old_color;
					for (;;) {
						int count = 0;
						old_color = color;
						found = FALSE;

						while (count < Session.Players.Count()) {
							if (count != 0 && Session.Players[count]->Player.Color == color) {
								color++;
								found = TRUE;
							}
							count++;
						}

						if (!found) break;

						color %= MAX_MPLAYER_COLORS;
					}

					resolved = old_color;
				}

				Session.ColorIdx = resolved;
				Session.Players[0]->Player.Color = resolved;

				SendDlgItemMessage(window, IDC_YOURCOLOR, CB_SETCURSEL, Session.ColorIdx, 0);

				_Net2DisplayUsers();
				PumpGameopts(1, 0);
			}
			return(0);

		case IDC_CRATES:
			if (Net2GameStarted) return(0);
			Session.Options.Goodies = false;
			if (SendDlgItemMessage(window, IDC_CRATES, BM_GETCHECK, 0, 0) != BST_CHECKED) return(0);
			Session.Options.Goodies = true;
			return(0);

		case IDC_BASES:
			if (Net2GameStarted) return(0);
			Session.Options.Bases = false;
			if (SendDlgItemMessage(window, IDC_BASES, BM_GETCHECK, 0, 0) != BST_CHECKED) return(0);
			Session.Options.Bases = true;
			return(0);

		case IDC_SHORT_GAME:
			if (Net2GameStarted) return(0);
			Session.Options.ShortGame = false;
			if (SendDlgItemMessage(window, IDC_SHORT_GAME, BM_GETCHECK, 0, 0) != BST_CHECKED) return(0);
			Session.Options.ShortGame = true;
			return(0);

		/*
		 * ....................................................................
		 * Toggle the "go" (start game) flag. Disable the button and signal
		 * the driver loop to begin the game.
		 * ....................................................................
		 */
		case IDC_GO:
			EnableWindow(GetDlgItem(window, IDC_GO), FALSE);
			Net2GameStarted = true;
			_netresponse = IDC_GO;
			return(0);

		/*
		 * ....................................................................
		 * Kick the selected players from the game. The host list-box (on the
		 * host dialog) holds the player rows; capture the selected names into
		 * a dictionary, then for each name (other than our own) find the
		 * matching player and send a NET_REJECT_JOIN kick packet.
		 * ....................................................................
		 */
		case IDC_KICK:
		{
			HWND userwin = GetDlgItem(WS_Find_Dialog(IDD_MPLAYER_HOST), IDC_USERS);

			Wstring name;
			Dictionary<Wstring,bool> lbdict(Wstring_Hash);

			LBSaveSelections(userwin, lbdict);

			while (lbdict.getEntries()) {
				bool value;
				lbdict.removeAny(name, value);

				if (strcmp(name.get(), Session.Handle)) {
					int index = -1;
					for (int i = 0; i < Session.Players.Count(); ++i) {
						if (!strcmp(name.get(), Session.Players[i]->Name)) {
							index = i;
							break;
						}
					}

					if (index != -1) {
						memset(&Session.GPacket, 0, sizeof(Session.GPacket));
						Session.GPacket.Command = NET_REJECT_JOIN;
						Session.GPacket.Reject.Why = (int)REJECT_BY_OWNER;
						Ipx.Send_Global_Message(&Session.GPacket, sizeof(Session.GPacket), 1, &Session.Players[index]->Address);
					}
				}
			}

			SendMessage(userwin, LB_SELITEMRANGE, 0, MAKELPARAM(0, -1));
			_Net2DisplayUsers();
			return(0);
		}

		/*
		 * ....................................................................
		 * Pick a different scenario. If "RandMap.Sed" is chosen, rebuild the
		 * map preview from "RandMap.img".
		 * ....................................................................
		 */
		case IDC_MULTIMAP: {
			if (Net2GameStarted) return(0);

			int old = Session.Options.ScenarioIndex;

			ShowWindow(window, SW_HIDE);
			IsRandomMap = false;

			if (Scenario_Dialog(MainWindow) == 2) {
				Session.Options.ScenarioIndex = old;
				Set_Scenario_Info_From_Index(Session.Options.ScenarioIndex);
				Update_Network_Dialog_Preview(window);
				IsRandomMap = true;
				ShowWindow(window, SW_SHOW);

				if (!stricmp((char *)Session.Scenarios[Session.Options.ScenarioIndex] + DESCRIP_MAX, "RandMap.Sed")) {
					if (MultiplayerMapPreview != NULL) {
						delete MultiplayerMapPreview;
					}
					MultiplayerMapPreview = new MapPreviewClass;
					MultiplayerMapPreview->Read_PCX_Preview("RandMap.img");
					if (MultiplayerMapPreview->Get_Preview_Surface() == NULL) {
						Update_Network_Dialog_Preview(window);
					}
					InvalidateRect(window, NULL, FALSE);
				} else {
					Update_Network_Dialog_Preview(window);
				}

				PumpGameopts(1, 0);
				InvalidateRect(window, NULL, FALSE);
			} else {
				if (!Set_Scenario_Info_From_Index(Session.Options.ScenarioIndex)) {
					Session.Options.ScenarioIndex = old;
				}
				IsRandomMap = true;
				ShowWindow(window, SW_SHOW);

				SendDlgItemMessage(window, IDC_SCENARIONAME, WM_SETTEXT, 0, (LPARAM)Session.Options.ScenarioDescription);

				if (!stricmp((char *)Session.Scenarios[Session.Options.ScenarioIndex] + DESCRIP_MAX, "RandMap.Sed")) {
					if (MultiplayerMapPreview != NULL) {
						delete MultiplayerMapPreview;
					}
					MultiplayerMapPreview = new MapPreviewClass;
					MultiplayerMapPreview->Read_PCX_Preview("RandMap.img");
					if (MultiplayerMapPreview->Get_Preview_Surface() == NULL) {
						Update_Network_Dialog_Preview(window);
					}
					InvalidateRect(window, NULL, FALSE);
				} else {
					Update_Network_Dialog_Preview(window);
				}
			}

			return(0);
		}

		case IDC_BRIDGE_DESTROY:
			if (Net2GameStarted) return(0);
			Session.Options.BridgeDestruction = false;
			if (SendDlgItemMessage(window, IDC_BRIDGE_DESTROY, BM_GETCHECK, 0, 0) != BST_CHECKED) return(0);
			Session.Options.BridgeDestruction = true;
			return(0);

		case IDC_MULTI_ENGINEER:
			if (Net2GameStarted) return(0);
			Session.Options.CrapEngineers = false;
			if (SendDlgItemMessage(window, IDC_MULTI_ENGINEER, BM_GETCHECK, 0, 0) != BST_CHECKED) return(0);
			Session.Options.CrapEngineers = true;
			return(0);

		default:
			return(0);
		}
	}

	case OD_SUBCLASSED: {
		Net2_g_Col_Accept = 5;
		Net2_g_Col_Name = 45;
		Net2_g_Col_House = 25;

		SendDlgItemMessage(window, IDC_USERS, OD_ADDCOLUMN, 0, Net2_g_Col_Name);
		SendDlgItemMessage(window, IDC_USERS, OD_ADDCOLUMN, 0, Net2_g_Col_House);
		SendDlgItemMessage(window, IDC_USERS, OD_ADDCOLUMN, 0, Net2_g_Col_Accept);
		SendDlgItemMessage(window, IDC_USERS, OD_TOOLTIPS, 0, 1);

		for (int i = 0; i < ARRAY_SIZE(PlayerColorTable); i++) {
			SendDlgItemMessage(window, IDC_YOURCOLOR, OD_SETCOLOR, i, (LPARAM)PlayerColorTable[i]);
		}

		SendDlgItemMessage(window, IDC_KICK, OD_TOOLTIPS, 0, 1);
		SendDlgItemMessage(window, IDC_KICK, OD_SETIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("woukick.pcx"));
		SendDlgItemMessage(window, IDC_KICK, OD_SETALTIMAGE, 0, (LPARAM)SurfaceCache.GetSurface("wodkick.pcx"));

		_Net2DisplayUsers();
		Net2DisplayGameList();
		return(0);
	}

	case OD_GETTIPTEXT: {
		HWND ctrl = GetDlgItem(window, wparam);
		GetWindowText(ctrl, (LPSTR)lparam, 127);
		return(0);
	}
	}

	return(0);
}


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
		if ((WS_Top_Window_ID() != IDD_MPLAYER_HOST) || gamenow) {
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
	bool display_users = false;
	bool display_games = false;
	HWND dialog;
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
			if (WS_Top_Window_ID() == IDD_MPLAYER_GUEST) {
				Receive_Random_Map_Preview();
			}
			continue;
		}

		if (Session.GPacket.Command==NET_REQ_PREVIEW) {
			if (WS_Top_Window_ID() == IDD_MPLAYER_HOST) {
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
						display_games = true;
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
								wsprintf(txt,Fetch_String(TXT_S_FORMED_NEW_GAME),
									Session.GPacket.Name);
								Sound_Effect(Rule->GameForming);
							}
							else {
								wsprintf(txt,Fetch_String(TXT_GAME_NOW_IN_PROGRESS),
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
				strcpy(who->Name, Session.GPacket.Name);
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
					wsprintf(txt,Fetch_String(TXT_S_FORMED_NEW_GAME),
						Session.GPacket.Name);
					PMessagePrintf(ColorSystem, txt);
					Sound_Effect(Rule->GameForming);
				}

				display_users = true;
				display_games = true;
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
				strcpy(who->Name, Session.GPacket.Name);
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
						EnableWindow(GetDlgItem(GameoptWindow(), IDC_ACCEPT), TRUE);
					}
				}

				DebugString("New Player");
				display_users = true;

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
				WS_Destroy_Dialog(0, 0);
				_netresponse = 0;
				dialog = WS_Create_Dialog(ProgramInstance, IDD_MPLAYER_GUEST, MainWindow, MPlayer_Guest_Dialog_Proc, FALSE);
				Center_Window_Within_Window(dialog);
				OwnerDraw::Subclass_Dialog(dialog, 0);
				ShowWindow(dialog, SW_SHOWNORMAL);
				display_users = true;

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
				display_users = true;
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
				if (item) {
					ODMessageBox(item, 0, Net2Callback, 0);
				}
				if ( WS_Top_Window_ID() != IDD_MPLAYER_GAME_LIST ) {
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
						DecodePubGameopt(Session.GPacket.Message.Buf, Session.GPacket.Name);
					}
					break;
				}
			}
			continue;
		}

		if (Session.GPacket.Command==NET_PRIV_GAMEOPT) {
			char *opts = strdup(Session.GPacket.Message.Buf + 1);
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
							display_users = true;
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
						if (WS_Top_Window_ID() != IDD_MPLAYER_GAME_LIST && WS_Top_Window_ID() == IDD_MPLAYER_GUEST) {
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
					display_games = true;
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
						EnableWindow(GetDlgItem(GameoptWindow(), IDC_ACCEPT), TRUE);
					}
				}
			}

			display_users = true;
			continue;
		}

		//------------------------------------------------------------------------
		// NET_GO: The game's owner is signalling us to start playing.
		//------------------------------------------------------------------------
		else if (Session.GPacket.Command==NET_GO || Session.GPacket.Command==NET_LOADGAME) {
			if ( JoinState==JOIN_CONFIRMED) {
				Session.MaxAhead = Session.GPacket.ResponseTime.OneWay;
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
				strcpy (who->Name, Session.GPacket.Name);
				who->Address = Session.GAddress;
				who->Chat.LastTime = TickCount;
				who->Chat.LastChance = 0;
				who->Chat.Color = Session.GPacket.Chat.Color;
				Session.Chat.Add (who);
			}

			if (found) {
				if (CurGame == 0) {
					display_users = true;
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
					Sound_Effect(Rule->IncomingMessage, 1.0, 0);
				}
			}
			//.....................................................................
			// Otherwise, we're in the chat room; display any old message.
			//.....................................................................
			else {
				PMessagePrintf(ColorUser, "[%s] %s", Session.GPacket.Name, Session.GPacket.Message.Buf);
				Sound_Effect(Rule->IncomingMessage, 1.0, 0);
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
				strcpy(who->Name, Session.GPacket.Name);
				who->Address = Session.GAddress;
				who->Player.House = Session.GPacket.PlayerInfo.House;

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
				display_users = true;
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

	if (display_users) {
		Net2DisplayUsers();
	}
	if (display_games) {
		Net2DisplayGameList();
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


/// <summary>
/// Handles the multiplayer guest dialog.
/// This is the setup dialog a player works in after joining somebody else's game. The
/// guest picks a side and a color here, chats with the rest of the players, and tells the
/// host when it is happy for the game to begin.
/// </summary>
/// <returns>Returns with TRUE if the message was consumed by this dialog.</returns>
BOOL CALLBACK MPlayer_Guest_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message) {

	case WM_INITDIALOG: {
		SendDlgItemMessage(window, IDC_YOURSIDE, CB_RESETCONTENT, 0, 0);

		int i;
		for (i = 0; i < HouseTypes.Count(); i++) {
			HouseTypeClass * house = HouseTypes[i];
			if (house->IsMultiplay) {
				SendDlgItemMessage(window, IDC_YOURSIDE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)(char const *)house->GivenName);
			}
		}

		SendDlgItemMessage(window, IDC_YOURSIDE, CB_SETCURSEL, Session.House, 0);

		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_RESETCONTENT, 0, 0);

		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_GOLD));
		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_RED));
		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_BLUE));
		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_GREEN));
		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_ORANGE));
		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_SKY_BLUE));
		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_PURPLE));
		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Fetch_String(TXT_PINK));

		SendDlgItemMessage(window, IDC_YOURCOLOR, CB_SETCURSEL, Session.ColorIdx, 0);

		EnableWindow(GetDlgItem(window, IDC_ACCEPT), FALSE);

		int self_index = -1;
		for (i = 0; i < Session.Players.Count(); ++i) {
			if (!strcmp(Session.Players[i]->Name, Session.Handle)) {
				self_index = i;
			}
		}

		if (self_index != -1) {
			Session.Players[self_index]->Player.Status = 0;
		}

		_Net2DisplayUsers();
		Session.Options.ScenarioDescription[0] = '\0';

		return(0);
	}

	case WM_COMMAND: {
		switch (LOWORD(wparam)) {

		case IDC_ACCEPT: {
			Session.Players[0]->Player.Status = 1;

			char dest[64];
			sprintf(dest, "A1");
			SendPublicGameopts(dest);

			EnableWindow(GetDlgItem(window, IDC_ACCEPT), FALSE);
			InvalidateRect(GetDlgItem(window, IDC_ACCEPT), NULL, FALSE);

			_Net2DisplayUsers();
			return(0);
		}

		case IDC_YOURSIDE:
		case IDC_YOURCOLOR: {
			if (HIWORD(wparam) == CBN_SELCHANGE) {
				LRESULT color = SendDlgItemMessage(window, IDC_YOURCOLOR, CB_GETCURSEL, 0, 0);

				LRESULT house = SendDlgItemMessage(window, IDC_YOURSIDE, CB_GETCURSEL, 0, 0);

				Session.PrefColor = color;

				char dest[64];
				sprintf(dest, "R%d,%d", house, color);
				SendPrivateGameopts(Session.GameName, dest);
			}
			return(0);
		}

		case IDCANCEL: {
			if (!Net2GameStarted) {
				_netresponse = IDCANCEL;
			}
			return(0);
		}

		case IDC_INPUT: {
			char text[260];

			SendDlgItemMessage(window, IDC_INPUT, WM_GETTEXT, 256, (LPARAM)text);

			int len = strlen(text);
			if (HIWORD(wparam) == EN_MAXTEXT) {
				SendDlgItemMessage(window, IDC_INPUT, WM_SETTEXT, 0, (LPARAM)"");

				if (len > 2) {
					PMessagePrintf(ColorMe, "[%s] %s", Session.Handle, text);

					GlobalPacketType gpacket;
					memset(&gpacket, 0, sizeof(gpacket));

					gpacket.Command = NET_MESSAGE;
					strcpy(gpacket.Name, Session.Handle);
					strcpy(gpacket.Message.Buf, text);
					gpacket.Message.Color = Session.ColorIdx;
					gpacket.Message.NameCRC = Compute_Name_CRC(Session.GameName);

					if (JoinState == JOIN_CONFIRMED) {
						for (int i = 1; i < Session.Players.Count(); ++i) {
							Ipx.Send_Global_Message(&gpacket, sizeof(gpacket), 1, &Session.Players[i]->Address);
							Call_Back();
						}
					} else {
						for (int i = 1; i < Session.Chat.Count(); ++i) {
							Ipx.Send_Global_Message(&gpacket, sizeof(gpacket), 1, &Session.Chat[i]->Address);
							Call_Back();
						}
					}
				}
			}

			return(0);
		}
		}

		return(0);
	}

	case OD_SUBCLASSED: {
		Net2_g_Col_Accept = 5;
		Net2_g_Col_Name = 45;
		Net2_g_Col_House = 25;

		SendDlgItemMessage(window, IDC_USERS, OD_ADDCOLUMN, 0, 45);
		SendDlgItemMessage(window, IDC_USERS, OD_ADDCOLUMN, 0, Net2_g_Col_House);
		SendDlgItemMessage(window, IDC_USERS, OD_ADDCOLUMN, 0, Net2_g_Col_Accept);
		SendDlgItemMessage(window, IDC_USERS, OD_TOOLTIPS, 0, 1);

		for (int i = 0; i < ARRAY_SIZE(PlayerColorTable); i++) {
			SendDlgItemMessage(window, IDC_YOURCOLOR, OD_SETCOLOR, i, (LPARAM)PlayerColorTable[i]);
		}

		_Net2DisplayUsers();
		Net2DisplayGameList();
		DisplayGameopts(window, 1);

		HWND combo = GetDlgItem(window, IDC_YOURSIDE);
		SendMessage(window, WM_COMMAND, MAKEWPARAM(IDC_YOURSIDE, CBN_SELCHANGE), (LPARAM)combo);

		return(0);
	}

	case WM_DRAWITEM:
		OwnerDraw::Draw_Item((DRAWITEMSTRUCT *)lparam);
		return(1);

	case WM_DESTROY: {
		if (MultiplayerMapPreview != NULL) {
			delete MultiplayerMapPreview;
			MultiplayerMapPreview = 0;
		}
		return(0);
	}

	case WM_PAINT: {
		OwnerDraw::Draw_Dialog_Back(window);

		if (MultiplayerMapPreview) {
			MultiplayerMapPreview->Blit_Preview(window);
		}

		ValidateRect(window, NULL);
		return(0);
	}

	case WM_ERASEBKGND:
		return(1);
	}

	return(0);
}
