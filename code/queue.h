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

/* $Header: /CounterStrike/QUEUE.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : QUEUE.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 12/08/94                                                     *
 *                                                                                             *
 *                  Last Update : December 9, 1994 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   QueueClass<T,size>::Add -- Add object to queue.                                           *
 *   QueueClass<T,size>::First -- Fetches reference to first object in list.                   *
 *   QueueClass<T,size>::Init -- Initializes queue to empty state.                             *
 *   QueueClass<T,size>::Next -- Throws out the head of the line.                              *
 *   QueueClass<T,size>::operator[] -- Fetches reference to sub object in queue.               *
 *   QueueClass<T,size>::QueueClass -- Default constructor for QueueClass objects.             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "mission.h"
#include "target.h"

bool Queue_Mission(TargetClass whom, MissionType mission, TargetClass & target, TargetClass & destination);
bool Queue_Mission(TargetClass whom, MissionType mission, TargetClass & target, TargetClass & destination, SpeedType speed, MPHType maxspeed);
bool Queue_Options(void);
bool Queue_Exit(void);
void Queue_AI(void);
void Add_CRC(unsigned int *crc, unsigned int val);

void Wait_For_End_Of_Queue(void);

namespace NetGlobal { enum class DecodeError; }

NetGlobal::DecodeError Kick_Packet_Received(int kicker, int kickee);

void Forget_Kick_Player(int player);

extern BasicTimerClass<SystemTimerClass> SentFrameSyncTimer;
extern int SentFrameSyncCount;
