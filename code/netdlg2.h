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

struct GlobalPacketType;
class IPXAddressClass;

int Net2FirstFreeColor(int reqcolor, int index);
bool Net2Callback(void);
void Net2DisplayUsers(void);
bool Net2Init_Network(void);
void Net2EncodeGameopt(char *out);
void Net2SetAccept(char *who, int status);
int Net2GetAccept(char *who);
int Net2SetHouseAndColor(char *who, int house, int color);
bool Net2Remote_Connect(void);
bool Process_Global_Packet(GlobalPacketType *packet, IPXAddressClass *address);
void Net2DisplayGameList(void);
