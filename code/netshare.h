/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "dict.h"
#include "globals.h"
#include "preview.h"
#include "wstring.h"

class HouseClass;

int ODMessageBox(const char *text, int type, bool (*callback)(void), bool large = false);
int CALLBACK ODMessageBox_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

bool Set_Scenario_Info_From_Index(int index);
void Commit_Session_Specials(void);
void PregameSetup(void);
void Update_Network_Dialog_Preview(HWND win);
void Receive_Random_Map_Preview(void);
void Send_Preview_To_Guests(void);
int CountAliveTeams(HouseClass * house);

int RandomMapWaypointCount(int index);
int Scenario_Dialog(HWND hWndParent);

unsigned int Wstring_Hash(Wstring & string);


void __cdecl PMessagePrintf(int color, const char * fmt, ...);
void __cdecl SMessagePrintf(int color, const char * fmt, ...);

void _DrawMessage(int color, const char * msg, HWND window);
void _SetMessageString(HWND window,  const char * msg, int len, int color);

HWND GameoptWindow(void);

void PumpGameopts(bool, bool = false);
bool DecodePubGameopt(char * options, char * name);
void SendPublicGameopts(char const * options);
void SendPrivateGameopts(char const * player, char const * options);
void DisplayGameopts(HWND window, BOOL initialize);

void LBSaveSelections(HWND win, Dictionary<Wstring,bool> & lbdict);
void LBRestoreSelections(HWND win, Dictionary<Wstring,bool> & lbdict);

char * CalcRandomMapDigest(void);
int CreateRandomMap(void);

extern COLORREF PlayerColorTable[MAX_PLAYERS];

/*
 * These are the predefined colors that PMessagePrintf and SMessagePrintf display their
 * messages in.
 */
extern const COLORREF ColorSystem;
extern const COLORREF ColorUser;
extern const COLORREF ColorPriv;
extern const COLORREF ColorPrivAction;
extern const COLORREF ColorAction;
extern const COLORREF ColorOp;
extern const COLORREF ColorPaged;
extern const COLORREF ColorMe;
extern const COLORREF ColorNoJoin;


extern MapPreviewClass *MultiplayerMapPreview;

extern bool IsRandomMap;

extern int IsColorChangePending;
