/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "discordformat.h"

#include <cstdio>
#include <cstring>
#include <string>


namespace {

constexpr int Field_Limit = 128;


void Write_LE32(unsigned char * dest, unsigned value)
{
	dest[0] = (unsigned char)(value & 0xFFu);
	dest[1] = (unsigned char)((value >> 8) & 0xFFu);
	dest[2] = (unsigned char)((value >> 16) & 0xFFu);
	dest[3] = (unsigned char)((value >> 24) & 0xFFu);
}


unsigned Read_LE32(unsigned char const * src)
{
	return (unsigned)src[0]
		| ((unsigned)src[1] << 8)
		| ((unsigned)src[2] << 16)
		| ((unsigned)src[3] << 24);
}


int Write_Frame(int opcode, char const * payload, int payload_size, unsigned char * dest, int dest_size)
{
	if (dest == nullptr || payload_size < 0 || dest_size < 8 + payload_size) {
		return(0);
	}
	if (payload_size > 0 && payload == nullptr) {
		return(0);
	}
	Write_LE32(dest, (unsigned)opcode);
	Write_LE32(dest + 4, (unsigned)payload_size);
	if (payload_size > 0) {
		std::memcpy(dest + 8, payload, (size_t)payload_size);
	}
	return(8 + payload_size);
}


void Copy_Limited(char * dest, int capacity, char const * text)
{
	if (capacity <= 0) {
		return;
	}
	if (text == nullptr) {
		text = "";
	}
	int count = 0;
	while (text[count] != '\0' && count < capacity - 1) {
		dest[count] = text[count];
		count++;
	}
	bool truncated = text[count] != '\0';
	if (truncated) {
		while (count > 0 && ((unsigned char)dest[count - 1] & 0xC0) == 0x80) {
			count--;
		}
		if (count > 0 && ((unsigned char)dest[count - 1] & 0x80) != 0) {
			count--;
		}
	}
	dest[count] = '\0';
}


void Append_Quoted(std::string & out, char const * text)
{
	out.push_back('"');
	if (text == nullptr) {
		text = "";
	}
	while (*text != '\0') {
		unsigned char c = (unsigned char)*text++;
		if (c == '"' || c == '\\') {
			out.push_back('\\');
			out.push_back((char)c);
		} else if (c < 0x20) {
			char hex[8];
			std::snprintf(hex, sizeof(hex), "\\u%04x", c);
			out += hex;
		} else {
			out.push_back((char)c);
		}
	}
	out.push_back('"');
}


char const * Mode_Label(int mode)
{
	switch (mode) {
		case DISCORD_PRESENCE_SKIRMISH:
			return("Skirmish");
		case DISCORD_PRESENCE_CAMPAIGN:
			return("Campaign");
		case DISCORD_PRESENCE_MULTIPLAYER:
			return("Multiplayer");
		default:
			return("In menus");
	}
}


bool Named(char const * text)
{
	return(text != nullptr && text[0] != '\0');
}


void Format_Details(char * dest, int capacity, DiscordPresenceFields const & fields)
{
	if (fields.Mode == DISCORD_PRESENCE_MENU || !Named(fields.Title) || !Named(fields.Kind)) {
		Copy_Limited(dest, capacity, Mode_Label(fields.Mode));
		return;
	}
	char combined[Field_Limit + 64];
	std::snprintf(combined, sizeof(combined), "%s (%s)", fields.Title, fields.Kind);
	Copy_Limited(dest, capacity, combined);
}


void Format_State(char * dest, int capacity, DiscordPresenceFields const & fields)
{
	if (fields.Mode == DISCORD_PRESENCE_MENU) {
		if (capacity > 0) {
			dest[0] = '\0';
		}
		return;
	}
	bool has_faction = Named(fields.Faction);
	bool has_map = Named(fields.Map);
	if (has_faction && has_map) {
		char combined[Field_Limit * 2 + 8];
		std::snprintf(combined, sizeof(combined), "%s - %s", fields.Faction, fields.Map);
		Copy_Limited(dest, capacity, combined);
	} else if (has_faction) {
		Copy_Limited(dest, capacity, fields.Faction);
	} else if (has_map) {
		Copy_Limited(dest, capacity, fields.Map);
	} else if (capacity > 0) {
		dest[0] = '\0';
	}
}

}


void Discord_Presence_Match_Label(char * dest, int capacity, std::uint64_t const * allies, int count)
{
	if (dest == nullptr || capacity <= 0) {
		return;
	}
	dest[0] = '\0';
	if (allies == nullptr || count < 2 || count > 64) {
		return;
	}

	int parent[64];
	for (int index = 0; index < count; index++) {
		parent[index] = index;
	}
	auto find = [&](int index) {
		while (parent[index] != index) {
			parent[index] = parent[parent[index]];
			index = parent[index];
		}
		return(index);
	};

	for (int i = 0; i < count; i++) {
		for (int j = i + 1; j < count; j++) {
			bool mutual = (allies[i] & (1ULL << j)) != 0 && (allies[j] & (1ULL << i)) != 0;
			if (!mutual) {
				continue;
			}
			int a = find(i);
			int b = find(j);
			if (a != b) {
				parent[b] = a;
			}
		}
	}

	int size[64] = {};
	int teams = 0;
	for (int index = 0; index < count; index++) {
		int root = find(index);
		if (size[root] == 0) {
			teams++;
		}
		size[root]++;
	}
	if (teams <= 1) {
		return;
	}

	int sizes[64];
	int n = 0;
	for (int index = 0; index < count; index++) {
		if (size[index] > 0) {
			sizes[n++] = size[index];
		}
	}
	for (int i = 1; i < n; i++) {
		int value = sizes[i];
		int j = i;
		while (j > 0 && sizes[j - 1] < value) {
			sizes[j] = sizes[j - 1];
			j--;
		}
		sizes[j] = value;
	}

	char combined[96];
	int used = 0;
	combined[0] = '\0';
	for (int i = 0; i < n; i++) {
		int wrote = std::snprintf(combined + used, sizeof(combined) - (size_t)used, i == 0 ? "%d" : "v%d", sizes[i]);
		if (wrote < 0 || used + wrote >= (int)sizeof(combined)) {
			break;
		}
		used += wrote;
	}
	Copy_Limited(dest, capacity, combined);
}


int Discord_Presence_Frame_Opcode(unsigned char const * frame, int size)
{
	if (frame == nullptr || size < 4) {
		return(-1);
	}
	return((int)Read_LE32(frame));
}


int Discord_Presence_Frame_Length(unsigned char const * frame, int size)
{
	if (frame == nullptr || size < 8) {
		return(-1);
	}
	return((int)Read_LE32(frame + 4));
}


int Discord_Presence_Handshake_Frame(std::uint64_t application_id, unsigned char * dest, int dest_size)
{
	char payload[64];
	int length = std::snprintf(payload, sizeof(payload), "{\"v\":1,\"client_id\":\"%llu\"}",
		(unsigned long long)application_id);
	if (length <= 0 || length >= (int)sizeof(payload)) {
		return(0);
	}
	return(Write_Frame(DISCORD_IPC_HANDSHAKE, payload, length, dest, dest_size));
}


int Discord_Presence_Activity_Frame(DiscordPresenceFields const & fields, unsigned nonce, long pid, unsigned char * dest, int dest_size)
{
	char details[Field_Limit + 1];
	char state[Field_Limit + 1];
	Format_Details(details, (int)sizeof(details), fields);
	Format_State(state, (int)sizeof(state), fields);

	std::string payload = "{\"cmd\":\"SET_ACTIVITY\",\"nonce\":\"";
	payload += std::to_string(nonce);
	payload += "\",\"args\":{\"pid\":";
	payload += std::to_string(pid);
	payload += ",\"activity\":{\"type\":0,\"details\":";
	Append_Quoted(payload, details);
	if (state[0] != '\0') {
		payload += ",\"state\":";
		Append_Quoted(payload, state);
	}
	if (fields.StartedAt > 0) {
		payload += ",\"timestamps\":{\"start\":";
		payload += std::to_string(fields.StartedAt);
		payload += '}';
	}
	payload += ",\"assets\":{\"large_image\":\"logo\",\"large_text\":\"siliconsun\"}}}}";
	return(Write_Frame(DISCORD_IPC_FRAME, payload.data(), (int)payload.size(), dest, dest_size));
}


int Discord_Presence_Pong_Frame(unsigned char const * payload, int payload_size, unsigned char * dest, int dest_size)
{
	if (payload_size < 0 || (payload_size > 0 && payload == nullptr)) {
		return(0);
	}
	return(Write_Frame(DISCORD_IPC_PONG, (char const *)payload, payload_size, dest, dest_size));
}
