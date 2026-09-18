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

#pragma once

#include "win.h"

#include "ui/screens/netlobby/uinetlobby.h"

struct GlobalPacketType;
class IPXAddressClass;


enum Net2LobbyPhaseType
{
	NET2_LOBBY_NONE,
	NET2_LOBBY_GAME_LIST,
	NET2_LOBBY_HOST,
	NET2_LOBBY_GUEST
};

extern Net2LobbyPhaseType Net2LobbyPhase;

void Net2_Show_Lobby(Net2LobbyPhaseType phase);
void Net2_Close_Lobby(void);

UINetChoice Net2Response(void);
int Net2CurrentGame(void);

bool Net2_Service_Lobby(void);

int Net2Country_At(int index);

void Net2Select_Game(int index);
void Net2Host_Take_Color(int color);
void Net2Request_House_And_Color(int house, int color);
void Net2Send_Chat(char const * text);
void Net2Set_Handle(char const * name);
void Net2Kick(char const * name);
void Net2Pick_Map(void);
void Net2Join_Game(void);
void Net2Host_Game(void);
bool Net2Can_Start(void);

int Net2FirstFreeColor(int reqcolor, int index);
bool Net2Callback(void);
bool Net2Init_Network(void);
void Net2EncodeGameopt(char *out);
void Net2SetAccept(char *who, int status);
int Net2GetAccept(char *who);
int Net2SetHouseAndColor(char *who, int house, int color);
bool Decrypt_Serial(char *buffer);
bool Net2Remote_Connect(void);
bool Process_Global_Packet(GlobalPacketType *packet, IPXAddressClass *address);
