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

#include <cstdio>

struct IStream;
class SaveVersionInfo;

/*
**	SAVELOAD.CPP
*/
int Load_Misc_Values(IStream * stream);
int Save_Misc_Values(IStream * stream);
bool Get_Savefile_Info(char const * name, SaveVersionInfo * info);
bool Load_Game(const char *file_name);
bool Reconcile_Players(void);
bool Request_Save_Game(char const * file_name, char const * descr, bool quiet = false);
void Process_Pending_Save_Game(void);
void Autosave_Service(void);
void Request_Quick_Save(void);
void Quick_Save_Service(void);
void Post_Save_Notice(int text);
bool Multiplayer_Load_Is_Allowed(void);
bool Multiplayer_Load_Prompt(void);
bool Multiplayer_Load_Request(char const * file_name);
void Multiplayer_Load_Receive(char const * file_name);
bool Multiplayer_Load_Is_Pending(void);
bool Multiplayer_Load_Is_In_Progress(void);
void Multiplayer_Load_Unseat(int index);
void Process_Pending_Load_Game(void);
void Reset_Multiplayer_Save_State(void);
void Print_Heap_CRCs(FILE * fp);
void Disable_Multiplayer_Saving(void);
bool Is_Multiplayer_Saving_Allowed(void);

extern unsigned int ExpectedGameVersion;
