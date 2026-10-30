/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The activity text and the IPC frame layout, with no Discord client and no game data.

#include "discordformat.h"

#include "doctest/doctest.h"

#include <cstring>
#include <string>


namespace {

int Payload_Length(unsigned char const * frame, int size)
{
	int length = Discord_Presence_Frame_Length(frame, size);
	if (length < 0 || size < 8 + length) {
		return(-1);
	}
	return(length);
}


std::string Payload(unsigned char const * frame, int size)
{
	int length = Payload_Length(frame, size);
	if (length < 0) {
		return("");
	}
	return(std::string((char const *)frame + 8, (size_t)length));
}


int Brace_Balance(std::string const & json)
{
	int depth = 0;
	bool quoted = false;
	bool escaped = false;
	for (char c : json) {
		if (quoted) {
			if (escaped) {
				escaped = false;
			} else if (c == '\\') {
				escaped = true;
			} else if (c == '"') {
				quoted = false;
			}
			continue;
		}
		if (c == '"') {
			quoted = true;
		} else if (c == '{') {
			depth++;
		} else if (c == '}') {
			depth--;
			if (depth < 0) {
				return(depth);
			}
		}
	}
	return(quoted ? -1 : depth);
}

}


TEST_CASE("the handshake names the application")
{
	unsigned char frame[80];
	int size = Discord_Presence_Handshake_Frame(123456789012345678ULL, frame, (int)sizeof(frame));
	CHECK(size > 8);
	CHECK(Discord_Presence_Frame_Opcode(frame, size) == DISCORD_IPC_HANDSHAKE);
	CHECK(Discord_Presence_Frame_Length(frame, size) == size - 8);
	std::string payload = Payload(frame, size);
	CHECK(payload == "{\"v\":1,\"client_id\":\"123456789012345678\"}");
	CHECK(Brace_Balance(payload) == 0);
}


TEST_CASE("a menu names the menus and the logo")
{
	DiscordPresenceFields fields = {};
	fields.Mode = DISCORD_PRESENCE_MENU;
	fields.Map = "Should Not Show";
	fields.Faction = "GDI";
	fields.AICount = 4;
	fields.StartedAt = 1'700'000'000;
	unsigned char frame[1024];
	int size = Discord_Presence_Activity_Frame(fields, 7, 42, frame, (int)sizeof(frame));
	std::string payload = Payload(frame, size);
	CHECK(Discord_Presence_Frame_Opcode(frame, size) == DISCORD_IPC_FRAME);
	CHECK(payload.find("\"details\":\"In menus\"") != std::string::npos);
	CHECK(payload.find("Should Not Show") == std::string::npos);
	CHECK(payload.find("\"state\"") == std::string::npos);
	CHECK(payload.find("\"start\":1700000000") != std::string::npos);
	CHECK(payload.find("\"large_image\":\"logo\"") != std::string::npos);
	CHECK(payload.find("\"large_text\":\"siliconsun\"") != std::string::npos);
	CHECK(payload.find("\"pid\":42") != std::string::npos);
	CHECK(payload.find("\"nonce\":\"7\"") != std::string::npos);
	CHECK(payload.find("\"type\":0") != std::string::npos);
	CHECK(Brace_Balance(payload) == 0);
}


TEST_CASE("a skirmish names the map, the side, and the AI count")
{
	DiscordPresenceFields fields = {};
	fields.Mode = DISCORD_PRESENCE_SKIRMISH;
	fields.Map = "Reinforcements";
	fields.Faction = "GDI";
	fields.AICount = 3;
	fields.StartedAt = 50;
	unsigned char frame[1024];
	int size = Discord_Presence_Activity_Frame(fields, 1, 9, frame, (int)sizeof(frame));
	std::string payload = Payload(frame, size);
	CHECK(payload.find("\"details\":\"Skirmish: Reinforcements\"") != std::string::npos);
	CHECK(payload.find("\"state\":\"GDI, 3 AI\"") != std::string::npos);
	CHECK(Brace_Balance(payload) == 0);
}


TEST_CASE("a campaign names the map and the side")
{
	DiscordPresenceFields fields = {};
	fields.Mode = DISCORD_PRESENCE_CAMPAIGN;
	fields.Map = "Reinforce Phoenix Base";
	fields.Faction = "Nod";
	fields.AICount = -1;
	fields.StartedAt = 50;
	unsigned char frame[1024];
	int size = Discord_Presence_Activity_Frame(fields, 1, 9, frame, (int)sizeof(frame));
	std::string payload = Payload(frame, size);
	CHECK(payload.find("\"details\":\"Campaign: Reinforce Phoenix Base\"") != std::string::npos);
	CHECK(payload.find("\"state\":\"Nod\"") != std::string::npos);
	CHECK(payload.find(" AI") == std::string::npos);
	CHECK(Brace_Balance(payload) == 0);
}


TEST_CASE("multiplayer keeps the AI count off the profile")
{
	DiscordPresenceFields fields = {};
	fields.Mode = DISCORD_PRESENCE_MULTIPLAYER;
	fields.Map = "Arctic";
	fields.Faction = "GDI";
	fields.AICount = -1;
	fields.StartedAt = 0;
	unsigned char frame[1024];
	int size = Discord_Presence_Activity_Frame(fields, 1, 9, frame, (int)sizeof(frame));
	std::string payload = Payload(frame, size);
	CHECK(payload.find("\"details\":\"Multiplayer: Arctic\"") != std::string::npos);
	CHECK(payload.find("\"state\":\"GDI\"") != std::string::npos);
	CHECK(payload.find("timestamps") == std::string::npos);
	CHECK(Brace_Balance(payload) == 0);
}


TEST_CASE("a quote in the map name stays inside its string")
{
	DiscordPresenceFields fields = {};
	fields.Mode = DISCORD_PRESENCE_SKIRMISH;
	fields.Map = "Say \"Hi\"\\now";
	fields.Faction = "";
	fields.AICount = 1;
	fields.StartedAt = 0;
	unsigned char frame[1024];
	int size = Discord_Presence_Activity_Frame(fields, 1, 9, frame, (int)sizeof(frame));
	std::string payload = Payload(frame, size);
	CHECK(payload.find("Say \\\"Hi\\\"\\\\now") != std::string::npos);
	CHECK(payload.find("\"state\":\"1 AI\"") != std::string::npos);
	CHECK(Brace_Balance(payload) == 0);
}


TEST_CASE("a pong repeats the ping and a short buffer writes nothing")
{
	unsigned char ping[] = { 'p', 'i', 'n', 'g' };
	unsigned char frame[32];
	int size = Discord_Presence_Pong_Frame(ping, 4, frame, (int)sizeof(frame));
	CHECK(Discord_Presence_Frame_Opcode(frame, size) == DISCORD_IPC_PONG);
	CHECK(Discord_Presence_Frame_Length(frame, size) == 4);
	CHECK(std::memcmp(frame + 8, ping, 4) == 0);
	CHECK(Discord_Presence_Pong_Frame(ping, 4, frame, 8) == 0);
	CHECK(Discord_Presence_Handshake_Frame(1, frame, 8) == 0);
}
