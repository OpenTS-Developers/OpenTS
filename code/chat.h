/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#pragma once

#include "chat.hh"

class HouseClass;
class IPXAddressClass;
struct GlobalPacketType;


bool Chat_Can_Open(ChatScopeType scope);
bool Chat_Reaches(HouseClass const * from, HouseClass const * to, ChatScopeType scope);
bool Chat_Begin(ChatScopeType scope, int house = -1);
void Chat_Send(char const * text);
void Chat_Receive(GlobalPacketType const & packet, IPXAddressClass & from);
void Chat_Show(HouseClass const * sender, ChatScopeType scope, int target, char const * text);
