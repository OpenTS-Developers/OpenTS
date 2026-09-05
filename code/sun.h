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

#ifdef INCLUDE_COM
#include "isun.h"
#endif
#include <cstring>

/// Everything from here on is the content of defines.h.

#include "ability.hh"
#include "do.hh"
#include "facing.hh"


/*
 * Undefine this for the demo build of the game
 * Must be first!
 */
//#define DEMO


#ifdef _DEBUG
#define _DEBUG_PRINT	/// Not to be confused with DebugString being omitted from builds -- the release binary still contains those strings.
#endif


/// Undefine this for the demo build of the game -- Must be first!
//#define _DEMO


/**********************************************************************
**	If this is defined, then the network code will be enabled.
*/
#define NETWORK
#define TIMING_FIX	1


/**********************************************************************
**	Defines for verifying free disk space
*/
#define	INIT_FREE_DISK_SPACE		8192
#define	SAVE_GAME_DISK_SPACE		 (INIT_FREE_DISK_SPACE - (1024*4096))		/// was 100000


/**********************************************************************
**	This is the size of the speech buffer. This value should be as large
**	as the largest speech sample, plus a few bytes for overhead
**	(16 bytes is sufficient).
*/


/**********************************************************************
**	Filenames of the data files it can create at run time.
*/
#define FAME_FILE_NAME			"HALLFAME.DAT"
#define NET_SAVE_FILE_NAME		"SAVEGAME.NET"
#define CONFIG_FILE_NAME		"SUN.INI"


/**********************************************************************
**	Map controls. The map is composed of square elements called 'cells'.
**	All larger elements are build upon these.
*/

// Size of the map in cells.
#define	MAP_CELL_W				512
#define	MAP_CELL_H				512
#define	MAP_CELL_TOTAL			(MAP_CELL_W*MAP_CELL_H)

#define	REFRESH_EOL				Cell(32767, 32767)	// This number ends a refresh/occupy offset list.

#define CELL_PIXEL_W			24
#define CELL_PIXEL_H			48
#define CELL_LEPTON_W			256
#define CELL_LEPTON_H			256
#define CELL_LEPTON				256

#define LEPTON_TO_PIXEL(lepton) ((lepton) / 7)
#define PIXEL_TO_LEPTON(pixel)  ((pixel) * 7)

#define	PIXEL_LEPTON_W			(CELL_LEPTON_W/CELL_PIXEL_W)
#define	PIXEL_LEPTON_H			(CELL_LEPTON_H/CELL_PIXEL_H)

#define BRIDGE_CELL_HEIGHT		4


/*
**	The map is broken down into regions of this specified dimensions.
*/
#define	REGION_WIDTH		4
#define	REGION_HEIGHT		4
#define	MAP_REGION_WIDTH	(((MAP_CELL_W + (REGION_WIDTH -1)) / REGION_WIDTH)+2)
#define	MAP_REGION_HEIGHT	(((MAP_CELL_H + (REGION_WIDTH -1)) / REGION_HEIGHT)+2)
#define  MAP_TOTAL_REGIONS	(MAP_REGION_WIDTH * MAP_REGION_HEIGHT)


// Save filename description.
#define	DESCRIP_MAX				44			// 40 chars + CR + LF + CTRL-Z + NULL

#define	CONQUER_PATH_MAX		24			// Number of cells to look ahead for movement.


/****************************************************************************
**	This is the max number of events supported on one frame.
*/
#define	MAX_EVENTS			64


#define MAX_LOG_LEVEL		10

// Maximum number of multi players possible.
#define	MAX_PLAYERS						8		// max # of players we can have

enum {
	MAX_TEAM_CLASSCOUNT=6,
	MAX_TEAM_MISSIONS=50
};


#define NORMAL_LIGHT 1000
