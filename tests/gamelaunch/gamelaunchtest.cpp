/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Tests the rules a launch request is judged by, without the engine or game data: what is
// refused, what is accepted, and the bytes an accepted game encodes to.

#include "doctest/doctest.h"
#include "launch/gamelaunch.h"
#include "proto/wire.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

using Launch::GameKind;

namespace {

Launch::Game::LimitsType const LIMITS = { 2, 16, 4, 50 };


Launch::Message::Player Human(char const * name, int color)
{
	Launch::Message::Player player;
	player.Name = name;
	player.Human = true;
	player.Color = color;
	return(player);
}


Launch::Message::Player Computer(int color = -1)
{
	Launch::Message::Player player;
	player.Country = -1;
	player.Color = color;
	player.Start = -1;
	return(player);
}


Launch::Message::Option Flag(char const * key, bool value)
{
	Launch::Message::Option option;
	option.Key = key;
	option.Value = value;
	return(option);
}


Launch::Message::Option Number(char const * key, std::int64_t value)
{
	Launch::Message::Option option;
	option.Key = key;
	option.Value = value;
	return(option);
}


Launch::Message::Game Skirmish(void)
{
	Launch::Message::Game launch;
	launch.Kind = GameKind::GAME_KIND_SKIRMISH;
	launch.Players = { Human("Alpha", 3), Computer(), Computer(5) };
	return(launch);
}


Launch::Message::Game Match(void)
{
	Launch::Message::Game launch;
	launch.Kind = GameKind::GAME_KIND_MULTIPLAYER;
	launch.Seed = 0;
	launch.Players = { Human("Alpha", 1), Human("Bravo", 4), Computer() };
	return(launch);
}


Launch::Message::Game Campaign(void)
{
	Launch::Message::Game launch;
	launch.Kind = GameKind::GAME_KIND_CAMPAIGN;
	launch.Players = { Human("Commander", -1) };
	launch.Players[0].Country = -1;
	return(launch);
}


std::string Refusal(Launch::Message::Game const & launch)
{
	Launch::Game game;
	std::string fault;
	bool const taken = game.From_Message(launch, LIMITS, fault);
	CHECK_MESSAGE(!taken, "the launch was accepted");
	CHECK(!fault.empty());
	return(fault);
}


Launch::Game Accepted(Launch::Message::Game const & launch)
{
	Launch::Game game;
	std::string fault;
	bool const taken = game.From_Message(launch, LIMITS, fault);
	CHECK_MESSAGE(taken, fault);
	return(game);
}


Launch::Message::Play Play_Request(Launch::Message::Game const & launch)
{
	Launch::Message::Play request;
	request.Game = launch;
	request.MapPath = "maps/two.map";
	request.Session.LocalPlayer = 0;
	if (launch.Kind == GameKind::GAME_KIND_MULTIPLAYER) {
		Launch::Message::Network network;
		network.Port = 1234;
		Launch::Message::Peer peer;
		peer.Player = 1;
		peer.Address = "10.0.0.2";
		peer.Port = 1235;
		network.Peers = { peer };
		request.Network = network;
	}
	return(request);
}


std::string Machine_Refusal(Launch::Message::Play const & request)
{
	Launch::Game const game = Accepted(request.Game);
	Launch::Machine machine;
	std::string fault;
	bool const taken = machine.From_Request(request, game, fault);
	CHECK_MESSAGE(!taken, "the request was accepted");
	CHECK(!fault.empty());
	return(fault);
}

}


TEST_CASE("the option table is in ascending key order and names each key once")
{
	auto const table = Launch::Game::Option_Table();
	REQUIRE(table.size() == 26);
	for (std::size_t index = 1; index < table.size(); index++) {
		CHECK_MESSAGE(std::string(table[index - 1].Key) < std::string(table[index].Key), table[index].Key);
	}
}


TEST_CASE("a skirmish, a multiplayer game, and a campaign that keep the rules are accepted")
{
	Launch::Game const skirmish = Accepted(Skirmish());
	CHECK(skirmish.Human_Count() == 1);
	CHECK(skirmish.Players.size() == 3);
	CHECK(skirmish.Options.Credits == 10000);
	CHECK(skirmish.Options.SurrenderOnDisconnect);

	Launch::Game const match = Accepted(Match());
	CHECK(match.Human_Count() == 2);
	CHECK(match.Seed == std::optional<std::uint32_t>(0));

	// A campaign's country and color are ignored, so -1 is accepted there.
	Launch::Message::Game campaign = Campaign();
	campaign.Options = { Number("Campaign", 3), Number("GameSpeed", 6) };
	campaign.GlobalFlags = { 0, 49 };
	Launch::Game const mission = Accepted(campaign);
	CHECK(mission.Options.Campaign == 3);
	CHECK(mission.GlobalFlags == std::vector<int>{ 0, 49 });
	CHECK(mission.Options.GameSpeed == 6);
}


TEST_CASE("the kind of game must match the number of human players")
{
	Launch::Message::Game launch = Skirmish();
	launch.Kind = GameKind::GAME_KIND_UNKNOWN;
	Refusal(launch);

	launch = Skirmish();
	launch.Kind = GameKind::GAME_KIND_MULTIPLAYER;
	launch.Seed = 1;
	Refusal(launch);

	launch = Match();
	launch.Kind = GameKind::GAME_KIND_SKIRMISH;
	Refusal(launch);

	// An observer counts as a human player.
	launch = Skirmish();
	launch.Players.insert(launch.Players.begin() + 1, Human("Watcher", 4));
	launch.Players[1].Observer = true;
	Refusal(launch);

	launch = Skirmish();
	launch.Players = { Computer(), Computer() };
	Refusal(launch);

	launch = Campaign();
	launch.Players.push_back(Computer());
	Refusal(launch);
}


TEST_CASE("a multiplayer game needs a seed and takes 0 as one")
{
	Launch::Message::Game launch = Match();
	launch.Seed.reset();
	Refusal(launch);
	launch.Seed = 0;
	Accepted(launch);
}


TEST_CASE("a launch lists at most eight players")
{
	Launch::Message::Game launch = Skirmish();
	while (launch.Players.size() < 9) {
		launch.Players.push_back(Computer());
	}
	Refusal(launch);
	launch.Players.pop_back();
	Accepted(launch);
}


TEST_CASE("a player's name, observer flag, and ai_level are checked")
{
	Launch::Message::Game launch = Skirmish();
	launch.Players[0].Name.assign(64, 'n');
	Refusal(launch);
	launch.Players[0].Name.assign(63, 'n');
	Accepted(launch);

	launch = Skirmish();
	launch.Players[0].Name.clear();
	Refusal(launch);

	// A computer player may be unnamed.
	launch = Skirmish();
	CHECK(launch.Players[1].Name.empty());
	Accepted(launch);

	launch = Skirmish();
	launch.Players[1].Observer = true;
	Refusal(launch);

	launch = Skirmish();
	launch.Players[0].AILevel = 0;
	Refusal(launch);

	launch = Skirmish();
	launch.Players[1].AILevel = 2;
	Accepted(launch);
	launch.Players[1].AILevel = 3;
	Refusal(launch);
	launch.Players[1].AILevel = -1;
	Refusal(launch);
}


TEST_CASE("country, color, and start must be in range, and any player may ask for random")
{
	Launch::Message::Game launch = Skirmish();
	launch.Players[1].Country = 2;
	Refusal(launch);
	launch.Players[1].Country = 1;
	Accepted(launch);

	launch = Skirmish();
	launch.Players[1].Color = 16;
	Refusal(launch);

	launch = Skirmish();
	launch.Players[1].Start = 8;
	Refusal(launch);
	launch.Players[1].Start = -2;
	Refusal(launch);
	launch.Players[1].Start = 7;
	Accepted(launch);

	launch = Skirmish();
	launch.Players[0].Country = -1;
	launch.Players[0].Color = -1;
	Accepted(launch);
	launch.Players[0].Country = -2;
	Refusal(launch);
}


TEST_CASE("human players come first and ask for different colors")
{
	Launch::Message::Game launch = Match();
	std::swap(launch.Players[1], launch.Players[2]);
	Refusal(launch);

	// A player's house is its position, whatever order the colors come in.
	launch = Match();
	std::swap(launch.Players[0], launch.Players[1]);
	Accepted(launch);

	launch = Match();
	launch.Players[1].Color = launch.Players[0].Color;
	Refusal(launch);

	launch.Players[0].Color = -1;
	launch.Players[1].Color = -1;
	Accepted(launch);
}


TEST_CASE("allies are other players that are not observers, ascending, each once")
{
	Launch::Message::Game launch = Skirmish();
	launch.Players[0].Allies = { 1, 2 };
	Accepted(launch);

	launch.Players[0].Allies = { 2, 1 };
	Refusal(launch);
	launch.Players[0].Allies = { 1, 1 };
	Refusal(launch);
	launch.Players[0].Allies = { 0 };
	Refusal(launch);
	launch.Players[0].Allies = { 3 };
	Refusal(launch);

	launch = Match();
	launch.Players.insert(launch.Players.begin() + 2, Human("Watcher", 9));
	launch.Players[2].Observer = true;
	Accepted(launch);
	launch.Players[0].Allies = { 2 };
	Refusal(launch);
}


TEST_CASE("an option the engine does not read, of the wrong type or kind, or set twice is refused")
{
	Launch::Message::Game launch = Skirmish();
	launch.Options = { Flag("Crates", true), Number("Credits", 5000) };
	Launch::Game const game = Accepted(launch);
	CHECK(game.Options.Crates);
	CHECK(game.Options.Credits == 5000);

	launch.Options = { Number("AIPlayers", 2) };
	Refusal(launch);

	launch.Options = { Number("Crates", 1) };
	Refusal(launch);
	launch.Options = { Flag("Credits", true) };
	Refusal(launch);
	Launch::Message::Option text;
	text.Key = "Credits";
	text.Value = std::string("5000");
	launch.Options = { text };
	Refusal(launch);
	Launch::Message::Option unset;
	unset.Key = "Credits";
	launch.Options = { unset };
	Refusal(launch);

	launch.Options = { Number("Campaign", 0) };
	Refusal(launch);
	launch.Options.clear();
	launch.GlobalFlags = { 0 };
	Refusal(launch);

	launch = Campaign();
	launch.Options = { Flag("Crates", true) };
	Refusal(launch);
	launch.Options = { Number("Credits", 5000) };
	Refusal(launch);

	launch = Skirmish();
	launch.Options = { Flag("Bases", false), Flag("Bases", false) };
	Refusal(launch);
}


TEST_CASE("an option value outside what the engine accepts is refused")
{
	Launch::Message::Game launch = Skirmish();
	launch.Options = { Number("GameSpeed", 7) };
	Refusal(launch);
	launch.Options = { Number("AIDifficulty", 3) };
	Refusal(launch);
	launch.Options = { Number("Credits", std::int64_t(1) << 31) };
	Refusal(launch);

	launch = Campaign();
	launch.Options = { Number("Campaign", 4) };
	Refusal(launch);
	launch.Options = { Number("Campaign", -1) };
	Accepted(launch);
	launch.Options.clear();
	launch.GlobalFlags = { 50 };
	Refusal(launch);
	launch.GlobalFlags = { 3, 3 };
	Refusal(launch);
	launch.GlobalFlags = { 4, 3 };
	Refusal(launch);
	launch.GlobalFlags.clear();
	launch.Options = { Number("CampaignPlayerDifficulty", 3) };
	Refusal(launch);
}


TEST_CASE("an accepted launch encodes to the same bytes whatever order its options came in")
{
	Launch::Message::Game forward = Match();
	forward.Options = { Flag("Crates", true), Number("Credits", 7000), Flag("ShortGame", true), Number("GameSpeed", 2) };
	Launch::Message::Game backward = forward;
	std::reverse(backward.Options.begin(), backward.Options.end());

	Launch::Bytes const first = Launch::Encoded(Accepted(forward).To_Message());
	Launch::Bytes const second = Launch::Encoded(Accepted(backward).To_Message());
	CHECK(first == second);

	// Every option that applies is written, defaults included, in ascending key order.
	Launch::Message::Game const written = Accepted(forward).To_Message();
	CHECK(written.Options.size() == 23);
	for (std::size_t index = 1; index < written.Options.size(); index++) {
		CHECK(written.Options[index - 1].Key < written.Options[index].Key);
	}

	// What To_Message writes is accepted again and encodes to the same bytes.
	std::string fault;
	Launch::Game again;
	REQUIRE_MESSAGE(again.From_Message(written, LIMITS, fault), fault);
	CHECK(Launch::Encoded(again.To_Message()) == first);
}


TEST_CASE("a campaign launch writes the options of a campaign")
{
	Launch::Message::Game const written = Accepted(Campaign()).To_Message();
	std::vector<std::string> keys;
	for (Launch::Message::Option const & option : written.Options) {
		keys.push_back(option.Key);
	}
	CHECK(keys == std::vector<std::string>{ "Campaign", "CampaignComputerDifficulty", "CampaignPlayerDifficulty", "Firestorm", "GameSpeed" });
}


TEST_CASE("this machine's part of a play request is checked against the shared part")
{
	Launch::Game const game = Accepted(Match());
	Launch::Message::Play request = Play_Request(Match());
	request.Session.LocalPlayer = 1;
	request.Network->Peers[0].Player = 0;
	request.Network->ConnectTimeout = 30000;
	request.Session.LoadScreen.X = 12;
	Launch::Machine machine;
	std::string fault;
	REQUIRE_MESSAGE(machine.From_Request(request, game, fault), fault);
	CHECK(machine.LocalPlayer == 1);
	REQUIRE(machine.Network.has_value());
	CHECK(machine.Network->ConnectTimeout == 1875);
	CHECK(machine.Network->ReconnectTimeout == Launch::Machine::RECONNECT_TIMEOUT_DEFAULT);
	CHECK(machine.LoadScreenX == std::optional<int>(12));
	CHECK(!machine.LoadScreenY.has_value());

	request = Play_Request(Match());
	request.Fixture = Launch::Message::Fixture();
	Machine_Refusal(request);

	request = Play_Request(Match());
	request.MapPath.clear();
	Machine_Refusal(request);

	request = Play_Request(Match());
	request.Session.LocalPlayer.reset();
	Machine_Refusal(request);
	request.Session.LocalPlayer = 2;
	Machine_Refusal(request);

	request = Play_Request(Match());
	request.Network.reset();
	Machine_Refusal(request);

	request = Play_Request(Skirmish());
	request.Network = Launch::Message::Network();
	Machine_Refusal(request);

	request = Play_Request(Campaign());
	request.Session.LocalPlayer = 1;
	Machine_Refusal(request);
}


TEST_CASE("the network part names every other human player once and a usable address")
{
	Launch::Message::Play request = Play_Request(Match());
	request.Network->Peers.clear();
	Machine_Refusal(request);

	request = Play_Request(Match());
	request.Network->Peers.push_back(request.Network->Peers[0]);
	Machine_Refusal(request);

	request = Play_Request(Match());
	request.Network->Peers[0].Player = 2;
	Machine_Refusal(request);

	request = Play_Request(Match());
	request.Network->Peers[0].Address = "0.0.0.0";
	Machine_Refusal(request);

	request = Play_Request(Match());
	request.Network->Port = 0;
	Machine_Refusal(request);

	// Through a tunnel a port is a tunnel number, which may be negative, and addresses are ignored.
	request = Play_Request(Match());
	request.Network->Tunnel = Launch::Message::Tunnel{ "203.0.113.5", 50000, {} };
	request.Network->Port = -7;
	request.Network->Peers[0].Port = -9;
	request.Network->Peers[0].Address.clear();
	Launch::Game const game = Accepted(Match());
	Launch::Machine machine;
	std::string fault;
	CHECK_MESSAGE(machine.From_Request(request, game, fault), fault);

	request.Network->Tunnel->Port = 0;
	Machine_Refusal(request);
	request.Network->Tunnel = Launch::Message::Tunnel{ "", 50000, {} };
	Machine_Refusal(request);
}


TEST_CASE("a wait is converted to 16 ms ticks and kept within the game's bounds")
{
	CHECK(Launch::Machine::Timeout_Ticks(0, 3600) == 3600);
	CHECK(Launch::Machine::Timeout_Ticks(57600, 3600) == 3600);
	CHECK(Launch::Machine::Timeout_Ticks(57615, 3600) == 3600);
	CHECK(Launch::Machine::Timeout_Ticks(1, 3600) == 60);
	CHECK(Launch::Machine::Timeout_Ticks(4000000000u, 3600) == 36000);
}


namespace {

Launch::Message::Request Request_Of(Launch::Verb verb)
{
	Launch::Message::Request request;
	request.Version = Launch::REQUEST_VERSION;
	request.Verb = std::move(verb);
	request.ResultPath = "result.bin";
	request.RequestId = "match-17";
	return(request);
}


std::string Request_Refusal(Launch::RequestFault fault, Launch::Message::Request const & request, char const * conflict = nullptr)
{
	std::string refusal;
	CHECK_MESSAGE(!Launch::Judge_Request(fault, request, conflict, refusal), "the request was accepted");
	return(refusal);
}

}


TEST_CASE("a play request, a load request and a capture request are run")
{
	std::string refusal = "untouched";
	CHECK_MESSAGE(Launch::Judge_Request(Launch::RequestFault::NONE, Request_Of(Play_Request(Skirmish())), nullptr, refusal), refusal);
	CHECK(refusal.empty());

	Launch::Message::Load load;
	load.Save = "SAVEGAME.001";
	CHECK_MESSAGE(Launch::Judge_Request(Launch::RequestFault::NONE, Request_Of(load), nullptr, refusal), refusal);

	Launch::Message::Capture shorthand;
	shorthand.MapPath = "maps/two.map";
	shorthand.OutPath = "two.png";
	CHECK_MESSAGE(Launch::Judge_Request(Launch::RequestFault::NONE, Request_Of(shorthand), nullptr, refusal), refusal);

	Launch::Message::Capture described;
	described.DescriptionPath = "capture.ini";
	CHECK_MESSAGE(Launch::Judge_Request(Launch::RequestFault::NONE, Request_Of(described), nullptr, refusal), refusal);
}


TEST_CASE("a request that did not decode is refused for its fault")
{
	Launch::Message::Request request = Request_Of(Play_Request(Skirmish()));
	CHECK(Request_Refusal(Launch::RequestFault::MALFORMED, request) == "The launch request is not a file this version can read.");
	CHECK(Request_Refusal(Launch::RequestFault::TEXT, request) == "The launch request has text that is not UTF-8.");
	CHECK(Request_Refusal(Launch::RequestFault::NO_VERB, request) == "The launch request asks for nothing this version can do.");

	request.Version = 2;
	CHECK(Request_Refusal(Launch::RequestFault::VERSION, request) == "The launch request has version 2; this version runs version 1.");
}


TEST_CASE("a request given with another switch that starts or changes a game is refused")
{
	Launch::Message::Request const request = Request_Of(Play_Request(Skirmish()));
	for (char const * name : { "-SPAWN", "-CAPTUREMAP=", "-CAPTUREOUT=", "-CAPTURE=", "-TIME=", "-SEED" }) {
		CHECK(Request_Refusal(Launch::RequestFault::NONE, request, name) == std::string("-LAUNCH= cannot be combined with ") + name + ".");
	}
}


TEST_CASE("the verbs this version cannot run are refused")
{
	CHECK(Request_Refusal(Launch::RequestFault::NONE, Request_Of(Launch::Message::Replay())) == "This version cannot play a replay.");
	CHECK(Request_Refusal(Launch::RequestFault::NONE, Request_Of(Launch::Message::Verify())) == "This version cannot verify a replay.");

	Launch::Message::Play fixture = Play_Request(Skirmish());
	fixture.Fixture.emplace();
	CHECK(Request_Refusal(Launch::RequestFault::NONE, Request_Of(fixture)) == "The play request sets fixture, which this version does not run.");
}


TEST_CASE("a capture names a map and a picture, or a description alone")
{
	Launch::Message::Capture both;
	both.DescriptionPath = "capture.ini";
	both.OutPath = "two.png";
	CHECK(Request_Refusal(Launch::RequestFault::NONE, Request_Of(both)) == "The capture request sets description_path together with map_path or out_path.");

	Launch::Message::Capture half;
	half.MapPath = "maps/two.map";
	CHECK(Request_Refusal(Launch::RequestFault::NONE, Request_Of(half)) == "The capture request needs map_path and out_path, or description_path alone.");
	CHECK(Request_Refusal(Launch::RequestFault::NONE, Request_Of(Launch::Message::Capture())) == "The capture request needs map_path and out_path, or description_path alone.");
}


namespace {

Launch::Message::ScenarioResult Closed(Launch::ScenarioEnd end, bool joined, bool observer)
{
	Launch::Message::ScenarioResult entry = Launch::Scenario_Result("TSGM01EA.MAP");
	Launch::Close_Scenario(entry, end, joined, observer, 420);
	return(entry);
}

}


TEST_CASE("a scenario's outcome is the local player's")
{
	using Launch::GameOutcome;
	using Launch::ScenarioEnd;

	CHECK(Closed(ScenarioEnd::WON, true, false).Outcome == GameOutcome::GAME_OUTCOME_WON);
	CHECK(Closed(ScenarioEnd::LOST, true, false).Outcome == GameOutcome::GAME_OUTCOME_LOST);
	CHECK(Closed(ScenarioEnd::LEFT, true, false).Outcome == GameOutcome::GAME_OUTCOME_ABORTED);
	CHECK(Closed(ScenarioEnd::WON, false, false).Completed);
	CHECK(Closed(ScenarioEnd::LOST, false, false).Completed);
	CHECK(!Closed(ScenarioEnd::LEFT, false, false).Completed);
	CHECK(Closed(ScenarioEnd::LEFT, false, false).LastFrame == std::optional<std::uint32_t>(420));

	CHECK(Closed(ScenarioEnd::WON, true, true).Outcome == GameOutcome::GAME_OUTCOME_UNKNOWN);
	CHECK(Closed(ScenarioEnd::LOST, true, true).Outcome == GameOutcome::GAME_OUTCOME_UNKNOWN);
	CHECK(Closed(ScenarioEnd::LEFT, true, true).Outcome == GameOutcome::GAME_OUTCOME_UNKNOWN);
	CHECK(Closed(ScenarioEnd::WON, true, true).Completed);
}


// The machines' start wait decides whether a failure ends a running game or one that never
// started; a desync, a lost connection, and a game that never started apply to an observer too.
TEST_CASE("a network failure ends a scenario uncompleted, before or after its start")
{
	using Launch::GameOutcome;
	using Launch::ScenarioEnd;

	for (bool const observer : { false, true }) {
		CAPTURE(observer);
		CHECK(Closed(ScenarioEnd::DESYNC, true, observer).Outcome == GameOutcome::GAME_OUTCOME_DESYNC);
		CHECK(Closed(ScenarioEnd::CONNECTION_LOST, true, observer).Outcome == GameOutcome::GAME_OUTCOME_CONNECTION_LOST);
		CHECK(Closed(ScenarioEnd::START_MISMATCH, true, observer).Outcome == GameOutcome::GAME_OUTCOME_DESYNC);

		for (ScenarioEnd const end : { ScenarioEnd::CONNECTION_LOST, ScenarioEnd::WAIT_CANCELLED, ScenarioEnd::START_MISMATCH }) {
			Launch::Message::ScenarioResult const entry = Closed(end, false, observer);
			CHECK(entry.Outcome == GameOutcome::GAME_OUTCOME_NOT_STARTED);
			CHECK_MESSAGE(!entry.LastFrame.has_value(), "a game that never started has no last frame");
		}

		for (ScenarioEnd const end : { ScenarioEnd::DESYNC, ScenarioEnd::CONNECTION_LOST, ScenarioEnd::WAIT_CANCELLED, ScenarioEnd::START_MISMATCH }) {
			for (bool const joined : { false, true }) {
				CHECK(!Closed(end, joined, observer).Completed);
			}
		}
		CHECK(Closed(ScenarioEnd::DESYNC, true, observer).LastFrame == std::optional<std::uint32_t>(420));
	}

	CHECK(Closed(ScenarioEnd::WAIT_CANCELLED, true, false).Outcome == GameOutcome::GAME_OUTCOME_ABORTED);
	CHECK(Closed(ScenarioEnd::WAIT_CANCELLED, true, true).Outcome == GameOutcome::GAME_OUTCOME_UNKNOWN);
}


TEST_CASE("only a start mismatch before every machine joined gives the launch a code")
{
	using Launch::ResultCode;
	using Launch::ScenarioEnd;

	CHECK(Launch::Scenario_End_Code(ScenarioEnd::START_MISMATCH, false) == ResultCode::RESULT_CODE_START_MISMATCH);
	CHECK(Launch::Scenario_End_Code(ScenarioEnd::START_MISMATCH, true) == ResultCode::RESULT_CODE_UNKNOWN);
	for (ScenarioEnd const end : { ScenarioEnd::LEFT, ScenarioEnd::WON, ScenarioEnd::LOST, ScenarioEnd::DESYNC, ScenarioEnd::CONNECTION_LOST, ScenarioEnd::WAIT_CANCELLED }) {
		CHECK(Launch::Scenario_End_Code(end, false) == ResultCode::RESULT_CODE_UNKNOWN);
	}
}


// Defeating the last opponent flags the defeated house to win, which sets PlayerLoses on the
// winner's machine, so a match is judged by whether the local house was defeated.
TEST_CASE("a match is won by the player whose house was not defeated")
{
	CHECK(Launch::Local_Win(false, false, false));
	CHECK(Launch::Local_Win(false, true, false));
	CHECK_FALSE(Launch::Local_Win(false, true, true));
	CHECK_FALSE(Launch::Local_Win(false, false, true));

	CHECK(Launch::Local_Win(true, true, false));
	CHECK_FALSE(Launch::Local_Win(true, false, false));
	CHECK(Launch::Local_Win(true, true, true));
}


// The result repeats the request id and lists each scenario as not recorded, because recording
// is off, with no replay path.
TEST_CASE("a launch result encodes the scenarios a launch started")
{
	Launch::Message::Result result;
	result.RequestId = "match-17";
	result.Code = Launch::ResultCode::RESULT_CODE_OK;

	Launch::Message::ScenarioResult first = Launch::Scenario_Result("TSGM01EA.MAP");
	Launch::Close_Scenario(first, Launch::ScenarioEnd::WON, true, false, 9000);
	result.Scenarios.push_back(first);
	result.Scenarios.push_back(Launch::Scenario_Result("TSGM02EA.MAP"));

	CHECK(first.NotRecorded == Launch::NotRecorded::NOT_RECORDED_DISABLED);
	CHECK(first.ReplayPath.empty());

	Launch::Bytes bytes;
	Launch::Encode(result, bytes);
	Launch::Message::Result back;
	REQUIRE(Launch::Decode(bytes, back));

	CHECK(back.RequestId == "match-17");
	CHECK(back.Code == Launch::ResultCode::RESULT_CODE_OK);
	REQUIRE(back.Scenarios.size() == 2);
	CHECK(back.Scenarios[0].MapPath == "TSGM01EA.MAP");
	CHECK(back.Scenarios[0].Outcome == Launch::GameOutcome::GAME_OUTCOME_WON);
	CHECK(back.Scenarios[0].Completed);
	CHECK(back.Scenarios[0].LastFrame == std::optional<std::uint32_t>(9000));
	CHECK(back.Scenarios[0].NotRecorded == Launch::NotRecorded::NOT_RECORDED_DISABLED);
	CHECK(back.Scenarios[0].ReplayPath.empty());
	CHECK(back.Scenarios[1].Outcome == Launch::GameOutcome::GAME_OUTCOME_UNKNOWN);
	CHECK(!back.Scenarios[1].Completed);
	CHECK(!back.Scenarios[1].LastFrame.has_value());

	Launch::Bytes again;
	Launch::Encode(back, again);
	CHECK_MESSAGE(again == bytes, "the result encodes to the same bytes twice");
}


// A load request is judged without its save: the save name, the speed, and, for a save played
// against other machines, the players and how to reach them.
TEST_CASE("a load request keeps the rules of launch.proto")
{
	Launch::Message::Load load;
	load.Save = "SVGM_000.NET";
	load.Session.LocalPlayer = 1;
	load.Session.Host = true;
	load.Session.AutosaveInterval = 900;
	load.GameSpeed = 3;
	load.Players.resize(2);
	load.Players[0].Name = "First";
	load.Players[1].Name = "Second";
	load.Network.emplace();
	load.Network->Port = 50000;
	load.Network->Peers.push_back(Launch::Message::Peer{ 0, "10.0.0.7", 50001 });

	Launch::Machine machine;
	std::string fault;
	REQUIRE_MESSAGE(machine.From_Load(load, true, fault), fault);
	CHECK(machine.MapPath == "SVGM_000.NET");
	CHECK(machine.LocalPlayer == 1);
	CHECK(machine.Host);
	CHECK(machine.AutosaveInterval == 900);
	REQUIRE(machine.Network.has_value());
	CHECK(machine.Network->ConnectTimeout == Launch::Machine::CONNECT_TIMEOUT_DEFAULT);

	Launch::Machine alone;
	REQUIRE_MESSAGE(alone.From_Load(load, false, fault), fault);
	CHECK_MESSAGE(!alone.Network.has_value(), "a single-player save reads no network");
	CHECK_MESSAGE(alone.LocalPlayer == 0, "a single-player save reads no local player");

	Launch::Message::Load fast = load;
	fast.GameSpeed = 7;
	CHECK(!machine.From_Load(fast, false, fault));
	CHECK(fault == "The load request sets game_speed 7, outside 0 to 6.");

	Launch::Message::Load unnamed = load;
	unnamed.Save.clear();
	CHECK(!machine.From_Load(unnamed, false, fault));
	CHECK(fault == "No saved game is given.");

	Launch::Message::Load stranger = load;
	stranger.Session.LocalPlayer = 2;
	CHECK(!machine.From_Load(stranger, true, fault));
	CHECK(fault == "local_player must be one of the load request's players.");

	Launch::Message::Load unreached = load;
	unreached.Network->Peers.clear();
	CHECK(!machine.From_Load(unreached, true, fault));
	CHECK(fault == "No peer is given for player 0.");

	Launch::Message::Load offline = load;
	offline.Network.reset();
	CHECK(!machine.From_Load(offline, true, fault));
	CHECK(machine.MapPath == "SVGM_000.NET");
}


// A crash handler adds the report folder to bytes encoded before the crash, and the launcher
// reads them as one result.
TEST_CASE("a crash result is the result so far with the report folder added")
{
	Launch::Message::Result result;
	result.RequestId = "match-18";
	Launch::Message::ScenarioResult first = Launch::Scenario_Result("TSGM01EA.MAP");
	Launch::Close_Scenario(first, Launch::ScenarioEnd::WON, true, false, 9000);
	result.Scenarios.push_back(first);
	result.Scenarios.push_back(Launch::Scenario_Result("TSGM02EA.MAP"));

	Launch::Bytes bytes;
	Launch::Encode(Launch::Crash_Result(result), bytes);

	std::string const folder = "C:\\Games\\OpenTS\\Exceptions\\exception-20261121-101500-42";
	std::size_t length = bytes.size();
	bytes.resize(length + 256);
	REQUIRE(ProtoWire::Put_Text_Field(bytes, length, Launch::RESULT_CRASH_PATH_FIELD, folder));
	bytes.resize(length);

	Launch::Message::Result back;
	REQUIRE(Launch::Decode(bytes, back));
	CHECK(back.RequestId == "match-18");
	CHECK(back.Code == Launch::ResultCode::RESULT_CODE_CRASHED);
	CHECK(back.Message == "The game crashed.");
	CHECK(back.CrashPath == folder);
	REQUIRE(back.Scenarios.size() == 2);
	CHECK(back.Scenarios[0].Outcome == Launch::GameOutcome::GAME_OUTCOME_WON);
	CHECK_MESSAGE(back.Scenarios[1].Outcome == Launch::GameOutcome::GAME_OUTCOME_UNKNOWN, "the scenario that crashed has no outcome");
	CHECK_MESSAGE(!back.Scenarios[1].LastFrame.has_value(), "the scenario that crashed gives no last frame");
}
