/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "always.h"

#include "launcher.h"

#include "crc.h"
#include "dbgprint.h"
#include "globals.h"
#include "headless.h"
#include "house.h"
#include "init.h"
#include "language/language.h"
#include "launch/gamelaunch.h"
#include "launch/launchmessages.h"
#include "launcherapply.h"
#include "msgbox.h"
#include "scenario.h"
#include "session.h"
#include "spawner.h"

#include <cstdlib>
#include <optional>
#include <string>
#include <variant>


namespace {

// Raise when the engine plays the same game differently, so that sync reports from the two
// versions show different identities.
constexpr int SESSION_IDENTITY_VERSION = 3;

bool IsSpawnFileRequested = false;
Launch::Message::Request Request;
bool IsGameConsumed = false;
bool IsFromSpawnFile = false;
int SessionIdentity = 0;
bool IsHost = false;


/// <summary>
/// Returns the session identity of a game, the same on every machine given the same game.
/// </summary>
int Identity_Of(Launch::Game const & game)
{
	Launch::Bytes bytes;
	Launch::Encode(game.To_Message(), bytes);

	CRCEngine crc;
	crc(SESSION_IDENTITY_VERSION);
	crc(bytes.data(), int(bytes.size()));
	return(crc());
}


/// <summary>
/// Refuses the launch in a message box, once the game window exists.
/// </summary>
/// <returns>false, so a caller can refuse and return in one statement.</returns>
bool Refuse_In_Game(std::string const & text)
{
	DebugString("[Launch] Refusing the launch request: %s\n", text.c_str());
	WWMessageBox().Process(text.c_str(), TXT_OK);
	return(false);
}

}


/// <summary>
/// Returns whether the command line asks for a launched game in place of the menus: true for
/// -SPAWN from the start.
/// </summary>
bool Launch::Is_Game_Requested(void)
{
	return(IsSpawnFileRequested);
}


bool Launch::Replaces_Menu(void)
{
	return(Is_Game_Requested() || Headless::Is_Active());
}


/// <summary>
/// Returns whether the game being set up or played comes from SPAWN.INI or a play or load
/// request. True from the start of Launch::Prepare.
/// </summary>
bool Launch::Is_Active(void)
{
	return(IsGameConsumed);
}


/// <summary>
/// Returns whether SPAWN.INI was read for the game being set up or played, even when its
/// request was then refused.
/// </summary>
bool Launch::Is_From_Spawn_File(void)
{
	return(IsFromSpawnFile);
}


/// <summary>
/// Returns the session identity of the launched game, or 0 unless a play request from
/// SPAWN.INI started the game; a resume has none.
/// </summary>
int Launch::Session_Identity(void)
{
	return(SessionIdentity);
}


void Launch::Request_Spawn_File(void)
{
	IsSpawnFileRequested = true;
}


/// <summary>
/// Sets up the game SPAWN.INI describes, in place of the menu.
/// Returns false when the launch is refused, and on every call after the first, so the process
/// exits without showing the menu once that game ends.
/// </summary>
/// <param name="gameloaded">Set when the launch resumed a saved game.</param>
/// <returns>bool; Is a game ready to start?</returns>
bool Launch::Prepare(bool & gameloaded)
{
	if (!IsSpawnFileRequested || IsGameConsumed) {
		return(false);
	}
	IsGameConsumed = true;

	// A player's country is a position in the rules' country list, so the roster is read first.
	Prepare_Side_Roster();

	std::string fault;
	std::optional<Spawner::Config> spawn_file;
	bool const read = Spawner::Read_Request(Request, spawn_file, fault);
	IsFromSpawnFile = spawn_file.has_value();
	if (!read) {
		return(Refuse_In_Game(fault));
	}

	// A refusal of SPAWN.INI's request starts with the file's section and key.
	Launch::FaultSite site;
	auto const refuse = [&](void) {
		return(Refuse_In_Game(spawn_file->Describe_Fault(site, fault)));
	};

	if (Launch::Message::Load const * load = std::get_if<Launch::Message::Load>(&Request.Verb)) {
		if (!Launch::Judge_Load(*load, fault, &site)) {
			return(refuse());
		}
		if (!Launch::Apply_Load(*load, gameloaded, fault)) {
			return(Refuse_In_Game(fault));
		}
		IsHost = load->Session.Host;

		DebugString("[Launch] Resuming %s.\n", load->Save.c_str());
		return(true);
	}

	Launch::Message::Play const & play = std::get<Launch::Message::Play>(Request.Verb);
	Launch::Game game;
	Launch::Machine machine;
	if (!game.From_Message(play.Game, Launch::Limits(), fault, &site) || !machine.From_Request(play, game, fault, &site)) {
		return(refuse());
	}
	if (!Launch::Apply(game, machine, fault)) {
		return(Refuse_In_Game(fault));
	}
	IsHost = machine.Host;
	SessionIdentity = Identity_Of(game);

	DebugString("[Launch] Launching %s with session identity %08x.\n", Scen->ScenarioName, SessionIdentity);

	// The network opens last, once the session is fully set up.
	if (Session.Type == GAME_INTERNET && !Launch::Wire_Network(machine, fault)) {
		return(Refuse_In_Game(fault));
	}

	return(true);
}


/// <summary>
/// Sends the host announcement in a launched network game whose request made this machine the
/// host; otherwise does nothing.
/// </summary>
void Launch::Announce_Master(void)
{
	if (IsGameConsumed && IsHost && Session.Type == GAME_INTERNET) {
		Session.Announce_Master();
	}
}
