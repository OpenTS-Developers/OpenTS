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

/* $Header: /CounterStrike/NETDLG.CPP 13    10/13/97 2:20p Steve_t $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : NETDLG.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Bill Randolph                                                *
 *                                                                                             *
 *                   Start Date : January 23, 1995                                             *
 *                                                                                             *
 *                  Last Update : December 12, 1995 [BRR]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 * These routines establish & maintain peer-to-peer connections between this system            *
 * and all others in the game.  Each system finds out the IPX address of the others,           *
 * and forms a direct connection (IPXConnectionClass) to that system.  Systems are             *
 * found out via broadcast queries.  Every system broadcasts its queries, and every            *
 * system replies to queries it receives.  At the point when the game owner signals            *
 * 'OK', every system must know about all the other systems in the game.                       *
 *                                                                                             *
 * How Bridges are handled:                                                                    *
 * Currently, bridges are handled by specifying the destination IPX address of the             *
 * "server" (game owner's system) on the command-line.  This address is used to                *
 * derive a broadcast address to that destination network, and this system's queries           *
 * are broadcast over its network & the server's network; replies to the queries come          *
 * with each system's IPX address attached, so once we have the address, we can form           *
 * a connection with any system on the bridged net.                                            *
 *                                                                                             *
 * The flaw in this plan is that we can only cross one bridge.  If there are 3 nets            *
 * bridged (A, B, & C), and the server is on net B, and we're on net A, our broadcasts         *
 * will reach nets A & B, but not C.  The way to circumvent this (if it becomes a problem)     *
 * would be to have the server tell us what other systems are in its game, not each            *
 * individual player's system.  Thus, each system would find out about all the other systems   *
 * by interacting with the game's owner system (this would be more involved than what          *
 * I'm doing here).                                                                            *
 *                                                                                             *
 * Here's a list of all the different packets sent over the Global Channel:                    *
 *                                                                                             *
 * NET_QUERY_GAME                                                                              *
 *                   (no other data)                                                           *
 * NET_ANSWER_GAME                                                                             *
 *                   Name:             game owner's name                                       *
 *                   GameInfo:         game's version & open state                             *
 * NET_QUERY_PLAYER                                                                            *
 *                   Name:             name of game we want players to respond for             *
 * NET_ANSWER_PLAYER                                                                           *
 *                   Name:             player's name                                           *
 *                   PlayerInfo:       info about player                                       *
 * NET_CHAT_ANNOUNCE                                                                           *
 *                   Chat:             unique id of the local node, so I can tell              *
 *                                     if this chat announcement is from myself                *
 * NET_QUERY_JOIN                                                                              *
 *                   Name:             name of player wanting to join                          *
 *                   PlayerInfo:       player's requested house, color, & version range        *
 * NET_CONFIRM_JOIN                                                                            *
 *                   PlayerInfo:       approves player's house & color                         *
 * NET_REJECT_JOIN                                                                             *
 *                   Reject.Why:       tells why we got rejected                               *
 * NET_GAME_OPTIONS                                                                            *
 *                   ScenarioInfo:     info about scenario                                     *
 * NET_SIGN_OFF                                                                                *
 *                   Name:             name of player signing off                              *
 * NET_PING                                                                                    *
 *                   (no other data)                                                           *
 * NET_GO                                                                                      *
 *                   Delay:            value of one-way response time, in frames               *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Init_Network -- initializes network stuff                                                 *
 *   Shutdown_Network -- shuts down network stuff                                              *
 *   Process_Global_Packet -- responds to remote queries                                       *
 *   Destroy_Connection -- destroys the given connection                                       *
 *   Remote_Connect -- handles connecting this user to others                                  *
 *   Net_Join_Dialog -- lets user join an existing game, or start a new one                    *
 *   Request_To_Join -- Sends a JOIN request packet to game owner                              *
 *   Unjoin_Game -- Cancels joining a game                                                     *
 *   Send_Join_Queries -- sends queries for the Join Dialog                                    *
 *   Get_Join_Responses -- sends queries for the Join Dialog                                   *
 *   Net_New_Dialog -- lets user start a new game                                              *
 *   Get_NewGame_Responses -- processes packets for New Game dialog                            *
 *   Compute_Name_CRC -- computes CRC from char string                                         *
 *   Net_Reconnect_Dialog -- Draws/updates the network reconnect dialog                        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
//   Warning - Most disgusting cpp file of all time. ajw

#include "always.h"

#include "netdlg.h"

#include "_map.h"
#include "_rules.h"
#include "data.h"
#include "dbgprint.h"
#include "globals.h"
#include "houstype.h"
#include "ipxmgr.h"
#include "language/language.h"
#include "list.h"
#include "netglobal.h"
#include "queue.h"
#include "rules.h"
#include "saveload.h"
#include "session.h"
#include "stats.h"
#include "stimer.h"
#include "timer.h"
#include "wsproto.h"

#include <cstdio>
#include <ctime>

class ListClass;
class ColorListClass;

//#define SHOW_MONO		0			// Replaced with _DEBUG
//#define OLDWAY			1

/*
******************************** Prototypes *********************************
*/
static int Net_Join_Dialog(void);
static int Request_To_Join (char *playername, int join_index,
	HousesType house, int color);
static void Unjoin_Game(char *namebuf,JoinStateType joinstate,
	ListClass *gamelist, ColorListClass *playerlist, int game_index,
	int goto_lobby, int msg_x, int msg_y, int msg_h, int send_x, int send_y,
	int msg_len);
static void Send_Join_Queries(int curgame, JoinStateType joinstate,
	int gamenow, int playernow, int chatnow, char *myname, int init = 0);
static JoinEventType Get_Join_Responses(JoinStateType *joinstate,
	ListClass *gamelist, ColorListClass *playerlist, int join_index,
	char *my_name, RejectType *why);
static int Net_New_Dialog(void);
static JoinEventType Get_NewGame_Responses(ColorListClass *playerlist,
	int *color_used);


#define PCOLOR_BROWN	PCOLOR_GREY


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
bool Init_Network (void)
{
	assert ( PacketTransport != NULL );

	//------------------------------------------------------------------------
	// This call allocates all necessary queue buffers and commands the
	// transport to start listening on the Global Channel.
	//------------------------------------------------------------------------
	return(Ipx.Init() != 0);

}	/* end of Init_Network */


/***********************************************************************************************
 * Shutdown_Network -- shuts down network stuff                                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *=============================================================================================*/
void Shutdown_Network (void)
{
	//------------------------------------------------------------------------
	// If I was in a game, I'm not now, so clear the game name
	//------------------------------------------------------------------------
	Session.GameName[0] = 0;

	Ipx.Shutdown();

}	/* end of Shutdown_Network */


/***********************************************************************************************
 * Destroy_Connection -- destroys the given connection                                         *
 *                                                                                             *
 * Call this routine when a connection goes bad, or another player signs off.                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      id         connection ID to destroy; this should be the HousesType of the player       *
 *             on this connection                                                              *
 *      error      0 = user signed off; 1 = connection error; otherwise, no error is shown.    *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/22/1995 BR : Created.                                                                  *
 *=============================================================================================*/
void Destroy_Connection(int id, int error)
{
	int i;
	HouseClass *housep;
	char txt[80];

	housep = Houses[(HousesType)id];

	//------------------------------------------------------------------------
	// Do nothing if the house isn't human.
	//------------------------------------------------------------------------
	if (!housep || !housep->IsHuman)
		return;

	Disable_Multiplayer_Saving();

	if (Debug_Print_Events) {
		DebugString("Destroying connection for house %d (%s)\n",
			id,(char const *)housep->IniName);
	}

	//------------------------------------------------------------------------
	// Create a message to display to the user
	//------------------------------------------------------------------------
	txt[0] = '\0';
	if (error==1) {
		sprintf(txt,Fetch_String(TXT_CONNECTION_LOST), housep->IniName.c_str());
	} else if (error==0) {
		sprintf(txt,Fetch_String(TXT_LEFT_GAME), housep->IniName.c_str());
	}

	if (strlen(txt)) {
		Session.Messages.Add_Message (NULL,0, txt, housep->Class->Scheme, TextPrintType(TPF_6PT_GRAD|TPF_FULLSHADOW|TPF_USE_GRAD_PAL), int(Rule->MessageDelay * TICKS_PER_MINUTE));
		Map.Flag_To_Redraw();
	}

	//------------------------------------------------------------------------
	// Remove this player from the Players vector
	//------------------------------------------------------------------------
	for (i = 0; i < Session.Players.Count(); i++) {
		if (!_stricmp(Session.Players[i]->Name, housep->IniName)) {
			delete Session.Players[i];
			Session.Players.Delete(Session.Players[i]);
			break;
		}
	}

	if (Session.Type == GAME_INTERNET && ! stricmp(Session.MasterPlayerName, housep->IniName)) {
		Session.MasterPlayerID = -1;
		Session.MasterPlayerName[0] = '\0';
	}

	//------------------------------------------------------------------------
	// Delete the IPX connection
	//------------------------------------------------------------------------
	Ipx.Delete_Connection(id);

	// Every survivor reports the departure; execution makes later copies no-ops.
	if (PlayerPtr != NULL) {
		OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::REMOVEPLAYER, id));
	}

	Session.NumPlayers--;

	//------------------------------------------------------------------------
	// If we're the last player left, tell the user.
	//------------------------------------------------------------------------
	if (Session.NumPlayers == 1) {
		sprintf(txt,"%s",Fetch_String(TXT_JUST_YOU_AND_ME));
		Session.Messages.Add_Message (NULL, 0, txt, housep->Class->Scheme,
			TextPrintType(TPF_6PT_GRAD|TPF_FULLSHADOW|TPF_USE_GRAD_PAL), int(Rule->MessageDelay * TICKS_PER_MINUTE));
		Map.Flag_To_Redraw();
	}
}	/* end of Destroy_Connection */


/***************************************************************************
 * Compute_Name_CRC -- computes CRC from char string                       *
 *                                                                         *
 * INPUT:                                                                  *
 *      name      string to create CRC for                                 *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      CRC                                                                *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   06/29/1995 BRR : Created.                                             *
 *=========================================================================*/
unsigned int Compute_Name_CRC(char *name)
{
	char buf[80];
	unsigned int crc = 0L;
	int i;

	strcpy (buf, name);
	_strupr (buf);

	for (i = 0; (unsigned)i < strlen(buf); i++) {
		Add_CRC (&crc, (unsigned int)buf[i]);
	}

	return(crc);

}	/* end of Compute_Name_CRC */


/************************************************************************** *
 * Clear_Listbox -- clears the given list box                               *
 *                                                                          *
 * This routine assumes the items in the given list box are character       *
 * buffers; it deletes each item in the list, then clears the list.         *
 *                                                                          *
 * INPUT:                                                                   *
 *      list         ptr to listbox                                         *
 *                                                                          *
 * OUTPUT:                                                                  *
 *      none.                                                               *
 *                                                                          *
 * WARNINGS:                                                                *
 *      none.                                                               *
 *                                                                          *
 * HISTORY:                                                                 *
 *   11/29/1995 BRR : Created.                                              *
 *=========================================================================*/
void Clear_Listbox(ListClass * list)
{
	char * item;

	//------------------------------------------------------------------------
	// Clear the list box
	//------------------------------------------------------------------------
	while (list->Count()) {
		item = (char *)(list->Get_Item(0));
		list->Remove_Item(item);
		delete [] item;
	}
	list->Flag_To_Redraw();
}	// end of Clear_Listbox


/// <summary>
/// Tells every seat this machine is leaving the match and waits, briefly, for the word to go
/// out. Used where the game ends on this machine alone: quitting the out-of-sync dialog, and
/// a multiplayer load that failed here.
/// </summary>
void Sign_Off_Match(void)
{
	if (Session.Players.Count() == 0) {
		return;
	}

	GlobalPacketType packet;
	NetGlobal::Initialize_Packet(packet, NET_SIGN_OFF);
	std::snprintf(packet.Name, sizeof(packet.Name), "%s", Session.Players[0]->Name);

	for (int index = 1; index < Session.Players.Count(); index++) {
		Ipx.Send_Global_Message(&packet, sizeof(packet), 1, &Session.Players[index]->Address);
		Ipx.Service();
	}

	CDTimerClass<SystemTimerClass> timer = TIMER_SECOND * 2;
	while (Ipx.Global_Num_Send() > 0 && timer > 0) {
		Ipx.Service();
	}
}
