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

bool Init_Network (void);
void Shutdown_Network (void);
bool Remote_Connect (void);
void Destroy_Connection(int id, int error);
void Sign_Off_Match(void);
unsigned int Compute_Name_CRC(char *name);
void Net_Reconnect_Dialog(int reconn, int fresh, int oldest_index, unsigned int timeval);


//---------------------------------------------------------------------------
// The possible states of the join-game dialog
//---------------------------------------------------------------------------
enum JoinStateType {
	JOIN_REJECTED = -1,     // we've been rejected
	JOIN_NOTHING,           // we're not trying to join a game
	JOIN_WAIT_CONFIRM,      // we're asking to join, & waiting for confirmation
	JOIN_CONFIRMED,         // we've been confirmed
	JOIN_GAME_START,        // the game we've joined is starting
	JOIN_GAME_START_LOAD,   // the game we've joined is starting; load saved game
};

//---------------------------------------------------------------------------
//	The possible return codes from Get_Join_Responses()
//---------------------------------------------------------------------------
enum JoinEventType {
	EV_NONE,            // nothing happened
	EV_STATE_CHANGE,    // Join dialog is in a new state
	EV_NEW_GAME,        // a new game formed, or is now open
	EV_NEW_PLAYER,      // a new player was detected
	EV_PLAYER_SIGNOFF,  // a player has signed off
	EV_GAME_SIGNOFF,    // a gamed owner has signed off
	EV_GAME_OPTIONS,    // a game options packet was received
	EV_MESSAGE,         // a message was received
};


//---------------------------------------------------------------------------
// The possible reasons we're rejected from joining a game
//---------------------------------------------------------------------------
enum RejectType {
	REJECT_DUPLICATE_NAME,          // player's name is a duplicate
	REJECT_GAME_FULL,               // game is full
	REJECT_VERSION_TOO_OLD,         // joiner's version is too old
	REJECT_VERSION_TOO_NEW,         // joiner's version is too new
	REJECT_BY_OWNER,                // game owner clicked "reject"
	REJECT_DISBANDED,               // game was disbanded
	REJECT_MISMATCH,                // "rules.ini" file mismatch.
	REJECT_DUPLICATE_SERIAL,        /// player's serial is a duplicate
};
