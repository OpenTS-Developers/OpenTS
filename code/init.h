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

#include "stimer.h"

#include "side.hh"
#include "theater.hh"
#include "theme.hh"

template<class T> class TRect;
typedef TRect<int> Rect;
class Surface;
class ObjectClass;

bool Parse_Command_Line(int argc, char * argv[]);

int Init_Game(int argc, char * argv[]);
bool Select_Game(bool fade);

int Main_Menu(unsigned int timeout);

bool Allocate_Surfaces(const Rect & hidden_rect, const Rect & composite_rect, const Rect & tile_rect, const Rect & sidebar_rect, bool hidden_first = false);

bool Init_Hotkeys(void);
void Execute_Command(char const * name);

#define ATTRACT_MODE_TIMEOUT	TIMER_MINUTE

void Title_Screen_Restore(bool force=false);

void Init_Campaigns(void);

void Prepare_Side_Roster(void);

void Delete_All_Objects(void);

void Init_Theater(TheaterType theater);
bool Prep_For_Side(SideType side);
bool Prep_Speech_For_Side(SideType side);

void Anim_Init(void);

void Load_Title_Page(const char * name, bool visible);

ThemeType Fetch_Main_Menu_Theme(void);
ThemeType Fetch_Map_Select_Theme(void);

void Draw_Version_Text(Surface * surface);

ObjectClass * Best_Selected_Object(void);
int New_Main_Menu(void);
