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

/***************************************************************************
 **     C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S       **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : VQAVIEW                                  *
 *                                                                         *
 *                    File Name : GAMETIME.CPP                             *
 *                                                                         *
 *                   Programmer : Michael Grayford                         *
 *                                                                         *
 *                   Start Date :                                          *
 *                                                                         *
 *                  Last Update : Nov 22, 1995   [MG]                      *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

//==========================================================================
// INCLUDES
//==========================================================================

#include "always.h"

#include "gametime.h"

#include "hostclock.h"
#include "win.h"

//==========================================================================
// PUBLIC DATA
//==========================================================================

GameTimeClass Game_Time;

/***************************************************************************
 * GameTimeClass - Constructor function for GameTimeClass                  *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/22/1995  MG : Created.                                             *
 *=========================================================================*/
GameTimeClass::GameTimeClass( void )
{
	game_start_time = Host_Milliseconds();
}


/***************************************************************************
 * Get_Time - returns the time in ms elapsed since game was started        *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      unsigned long - time in milliseconds since game was started        *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/22/1995  MG : Created.                                             *
 *=========================================================================*/
unsigned int GameTimeClass::Get_Time( void )
{
	unsigned int curr_windows_time;
	unsigned int game_time;

	curr_windows_time = Host_Milliseconds();
	if ( curr_windows_time <= game_start_time ) {
		// Handles the case if the windows time wraps while playing the game.
		game_time = MAX_ULONG - game_start_time + curr_windows_time;
	}
	else {
		game_time = curr_windows_time - game_start_time;
	}
	return( game_time );
}


/***************************************************************************
 * Get_Game_Time - returns the time in ms elapsed since game was started   *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      unsigned long - time in milliseconds since game was started        *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/22/1995  MG : Created.                                             *
 *=========================================================================*/
unsigned int Get_Game_Time( void )
{
	return( Game_Time.Get_Time() );
}


/// <summary>
/// Fetches the elapsed game time in movie timer ticks.
/// This routine is the timer source handed to the VQA player, which paces its frames
/// and its audio against a sixty tick per second clock rather than milliseconds.
/// </summary>
/// <returns>Returns with the time elapsed since the game started, in sixtieths of a
/// second.</returns>
unsigned int Get_Game_Time_50( void )
{
	return( 3 * Game_Time.Get_Time() / 50 );
}
