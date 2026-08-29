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
 * Either one unnamed means the game's own directory, which is where everything lived when
 * a game was one directory belonging to one person.
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
 * Where a file the game itself writes belongs, and where one should be read from. The two
 * differ while a player still has settings and saved games beside the executable: those
 * are read where they are, and written where they now belong.
 *
 * A file object built from one of these keeps the pointer it is given rather than copying
 * the name, so hold the string for as long as the file object lives.
 */
std::string User_File_Read_Name(char const * filename);
std::string User_File_Write_Name(char const * filename);

std::vector<std::string> Parse_Search_Folders(char const * list);
std::vector<std::string> Search_Files(char const * pattern);
