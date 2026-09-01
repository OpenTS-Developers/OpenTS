/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>

/*
 * The directories the game keeps its files in. The data directory holds what a deployment
 * ships and is never written to. The user directory holds what a player's game writes.
 * Either one unnamed means the game's own directory.
 */

void Set_Data_Directory(char const * path);
void Set_User_Directory(char const * path);

bool Apply_Game_Directories(void);
void Init_Search_Folders(void);

/*
 * What stopped the directories being used, for whoever has a window to say it in.
 */
char const * Game_Directory_Error(void);

/*
 * Where a file the game itself writes belongs. The file classes place their own files, so
 * this is for the few things that never reach them: structured storage, and the directory
 * searches and disk queries the game makes of Windows directly.
 */
std::string User_File_Write_Name(char const * filename);

/*
 * Where the player's saved games are. They are never searched for: the folder is named
 * outright wherever a saved game is opened, listed or removed.
 */
std::string Saved_Game_Name(char const * filename);

std::vector<std::string> Parse_Search_Folders(char const * list);
std::vector<std::string> Search_Files(char const * pattern);
