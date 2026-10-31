/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstdint>


// What the profile is showing. A menu has no map or side.
enum DiscordPresenceMode {
	DISCORD_PRESENCE_MENU = 0,
	DISCORD_PRESENCE_SKIRMISH,
	DISCORD_PRESENCE_CAMPAIGN,
	DISCORD_PRESENCE_MULTIPLAYER
};


// The fields of one activity. A null map, faction, title, or kind is an empty one.
// A menu, or an activity with no title or kind, uses the mode label alone.
// A StartedAt of zero omits the elapsed-time clock.
struct DiscordPresenceFields {
	int Mode;
	char const * Title;
	char const * Kind;
	char const * Map;
	char const * Faction;
	std::int64_t StartedAt;
};


// Writes the side list for the houses given, such as "2v2", "2v1v1", or "1v1v1".
// allies[i] bit j means house i counts house j as an ally. A side is mutual.
// Fewer than two houses, or one side among them, writes an empty string.
void Discord_Presence_Match_Label(char * dest, int capacity, std::uint64_t const * allies, int count);


// Discord's IPC opcodes. A frame is a little-endian opcode, a little-endian
// length, and then that many payload bytes.
enum {
	DISCORD_IPC_HANDSHAKE = 0,
	DISCORD_IPC_FRAME = 1,
	DISCORD_IPC_CLOSE = 2,
	DISCORD_IPC_PING = 3,
	DISCORD_IPC_PONG = 4
};


// Each call writes one IPC frame and returns how many bytes that was.
// A destination that cannot hold the frame returns zero and is left untouched.
int Discord_Presence_Handshake_Frame(std::uint64_t application_id, unsigned char * dest, int dest_size);
int Discord_Presence_Activity_Frame(DiscordPresenceFields const & fields, unsigned nonce, long pid, unsigned char * dest, int dest_size);
int Discord_Presence_Pong_Frame(unsigned char const * payload, int payload_size, unsigned char * dest, int dest_size);

int Discord_Presence_Frame_Opcode(unsigned char const * frame, int size);
int Discord_Presence_Frame_Length(unsigned char const * frame, int size);
