/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "always.h"

#include "chat.h"

#include "_map.h"
#include "_rules.h"
#include "data.h"
#include "dbgprint.h"
#include "desyncdlg.h"
#include "globals.h"
#include "house.h"
#include "ipxmgr.h"
#include "language/language.h"
#include "rules.h"
#include "session.h"
#include "stimer.h"
#include "utf8.h"

#include "dialog.hh"

#include <cstdio>
#include <cstring>


/// <summary>
/// Whether the player at this machine may open a message of this scope.
/// </summary>
bool Chat_Can_Open(ChatScopeType scope)
{
	switch (scope) {
		case ChatScopeType::Everyone:
			return(true);

		case ChatScopeType::Allies:
		case ChatScopeType::Player:
			return(!Session.ObiWan);

		case ChatScopeType::Observers:
			return(PlayerPtr->IsObserver);
	}
	return(false);
}


/// <summary>
/// Whether a message of this scope from one house is delivered to another. The sender asks it
/// for each connection and the receiver asks it again, so delivery never rests on the sender.
/// </summary>
bool Chat_Reaches(HouseClass const * from, HouseClass const * to, ChatScopeType scope)
{
	switch (scope) {
		case ChatScopeType::Everyone:
		case ChatScopeType::Player:
			return(true);

		case ChatScopeType::Allies:
			return(from->Is_Ally(to));

		case ChatScopeType::Observers:
			return(from->IsObserver && to->IsObserver);
	}
	return(false);
}


/// <summary>
/// Opens the message edit for a scope, addressed to one house for a private message.
/// </summary>
/// <returns>bool; Did the edit open?</returns>
bool Chat_Begin(ChatScopeType scope, int house)
{
	if (Session.Type != GAME_IPX && Session.Type != GAME_INTERNET) {
		return(false);
	}
	if (Session.Messages.Is_Edit() || !Chat_Can_Open(scope)) {
		return(false);
	}

	char prefix[80 + MAX_MESSAGE_LENGTH + 32];
	switch (scope) {
		case ChatScopeType::Everyone:
			std::snprintf(prefix, sizeof(prefix), "%s", Fetch_String(TXT_TO_ALL));
			break;

		case ChatScopeType::Allies:
			std::snprintf(prefix, sizeof(prefix), "%s", Fetch_String(TXT_TO_TEAM));
			break;

		case ChatScopeType::Observers:
			std::snprintf(prefix, sizeof(prefix), "%s", Fetch_String(TXT_TO_OBSERVERS));
			break;

		case ChatScopeType::Player:
			if (house < 0 || Ipx.Connection_Address(house) == NULL) {
				return(false);
			}
			std::snprintf(prefix, sizeof(prefix), Fetch_String(TXT_TO), Session.Shown_Name(house, Ipx.Connection_Name(house)).c_str());
			break;
	}

	Session.MessageScope = scope;
	Session.MessageTarget = -1;
	Session.MessageAddress = IPXAddressClass();
	if (scope == ChatScopeType::Player) {
		Session.MessageTarget = house;
		Session.MessageAddress = *Ipx.Connection_Address(house);
	}

	Session.Messages.Add_Edit(PlayerPtr->Scheme,
		TextPrintType(TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW), prefix, 0, -1);
	Map.Flag_To_Redraw();
	return(true);
}


/// <summary>
/// Sends the typed line to whoever the open edit addressed and shows it at this machine.
/// </summary>
void Chat_Send(char const * text)
{
	ChatScopeType scope = Session.MessageScope;

	std::memset(&Session.GPacket, 0, sizeof(Session.GPacket));
	Session.GPacket.Command = NET_MESSAGE;
	std::strcpy(Session.GPacket.Name, Session.Players[0]->Name);
	Session.GPacket.Message.Color = Session.ColorIdx;
	// Nothing in a match reads the game-name CRC; the lobby keeps its own.
	Session.GPacket.Message.NameCRC = 0;
	Session.GPacket.Message.Scope = scope;
	UTF8::Copy(Session.GPacket.Message.Buf, sizeof(Session.GPacket.Message.Buf), text);

	if (scope == ChatScopeType::Player) {
		Ipx.Send_Global_Message(&Session.GPacket, sizeof(GlobalPacketType), 1, &Session.MessageAddress);
		Ipx.Service();
	} else {
		for (int index = 0; index < Ipx.Num_Connections(); index++) {
			int id = Ipx.Connection_ID(index);
			if (Chat_Reaches(PlayerPtr, Houses[id], scope)) {
				Ipx.Send_Global_Message(&Session.GPacket, sizeof(GlobalPacketType), 1, Ipx.Connection_Address(id));
				Ipx.Service();
			}
		}
	}

	int target = -1;
	if (scope == ChatScopeType::Player) {
		target = Session.MessageTarget;
	}
	Chat_Show(PlayerPtr, scope, target, Session.GPacket.Message.Buf);
	std::strcpy(Session.LastMessage, Session.GPacket.Message.Buf);
}


/// <summary>
/// Shows a line another machine sent, if it came from a seat of the match and is for this house.
/// </summary>
void Chat_Receive(GlobalPacketType const & packet, IPXAddressClass & from)
{
	HouseClass const * sender = NULL;
	for (int index = 0; index < Ipx.Num_Connections(); index++) {
		int id = Ipx.Connection_ID(index);
		if (from == *Ipx.Connection_Address(id)) {
			sender = Houses[id];
			break;
		}
	}

	if (sender == NULL) {
		DebugString("Chat: dropped a line from an address that holds no seat\n");
		return;
	}

	if (!Chat_Reaches(sender, PlayerPtr, packet.Message.Scope)) {
		DebugString("Chat: dropped a line from %s that is not for this house\n", (char const *)sender->IniName);
		return;
	}

	Chat_Show(sender, packet.Message.Scope, PlayerPtr->HeapID, packet.Message.Buf);
	std::strcpy(Session.LastMessage, packet.Message.Buf);
}


/// <summary>
/// Adds a line to the message list in the sender's color, tagged with its scope. The target
/// is the house a private message is for.
/// </summary>
void Chat_Show(HouseClass const * sender, ChatScopeType scope, int target, char const * text)
{
	char tag[MPLAYER_NAME_MAX + 16];
	switch (scope) {
		case ChatScopeType::Allies:
			std::snprintf(tag, sizeof(tag), "%s", Fetch_String(TXT_CHAT_TO_TEAM));
			break;

		case ChatScopeType::Observers:
			std::snprintf(tag, sizeof(tag), "%s", Fetch_String(TXT_CHAT_TO_OBSERVERS));
			break;

		case ChatScopeType::Player:
			std::snprintf(tag, sizeof(tag), Fetch_String(TXT_CHAT_TO_PLAYER),
				target >= 0 && target < Houses.Count() ? Session.Shown_Name(Houses[target]).c_str() : "");
			break;

		default:
			tag[0] = '\0';
			break;
	}

	char name[HOUSE_NAME_MAX + sizeof(tag) + 4];
	if (tag[0] != '\0') {
		std::snprintf(name, sizeof(name), Fetch_String(TXT_CHAT_TAGGED), Session.Shown_Name(sender).c_str(), tag);
	} else {
		std::snprintf(name, sizeof(name), "%s", Session.Shown_Name(sender).c_str());
	}

	int timeout = int(Rule->MessageDelay * TICKS_PER_MINUTE);
	if (!Session.Messages.Concat_Message(name, sender->HeapID, text, timeout)) {
		Session.Messages.Add_Message(name, sender->HeapID, text, sender->Scheme,
			TextPrintType(TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW), timeout);
	}
	DesyncDialog.Notify_Chat(name, text);
	Map.Flag_To_Redraw(GS_REDRAW_ALL);
}
