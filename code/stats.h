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

/*************************************************************
**	Internet specific externs
*/
extern bool	ConnectionLost;
extern void *PacketLater;
extern int WestwoodOnline_PortNumber;
extern bool GameStatisticsPacketSent;

// These describe the match the results packet reports on. Nothing supplies them since
// the online service that once did was retired, so they hold their initial values and
// the packet reports a match with no identity.
extern int WestwoodOnline_StartTime;
extern int WestwoodOnline_Tournament;
extern int WestwoodOnline_GameID;
extern int WestwoodOnline_GameSKU_TS;
extern int WestwoodOnline_GameSKU_FS;
extern int WestwoodOnline_GameSKU_WDT;
extern char WestwoodOnline_LoginName[];
extern char WestwoodOnline_UserName[];
extern char WestwoodOnline_Clan1_Players[];
extern char WestwoodOnline_Clan2_Players[];
extern int g_PingsSent;
extern int g_PingsReceived;

void	Register_Game_Start_Time(void);
void	Register_Game_End_Time(void);
void	Send_Statistics_Packet(void);
