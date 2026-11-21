/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "doctest/doctest.h"
#include "launch/launchmessages.h"
#include "replaytest.h"

#include <string>
#include <variant>

using namespace Launch;


namespace {

Message::Request Request_With(std::uint32_t version, Verb verb)
{
	Message::Request request;
	request.Version = version;
	request.Verb = std::move(verb);
	request.ResultPath = "results/out.bin";
	request.RequestId = "req-7";
	return(request);
}


Message::Play Sample_Play(void)
{
	Message::Play play;
	play.Game.Kind = GameKind::GAME_KIND_MULTIPLAYER;
	play.Game.Seed = 0;
	Message::Player alpha;
	alpha.Name = "Alpha";
	alpha.Human = true;
	alpha.Color = 2;
	alpha.Allies = { 1 };
	Message::Player bravo;
	bravo.Name = "Bravo";
	bravo.Human = true;
	bravo.Start = 3;
	play.Game.Players = { alpha, bravo };
	Message::Option credits;
	credits.Key = "Credits";
	credits.Value = std::int64_t(5000);
	play.Game.Options = { credits };
	play.Game.Anonymous = true;

	play.MapPath = "maps/two.map";
	play.Session.LocalPlayer = 0;
	play.Session.Host = true;
	play.Session.AutosaveInterval = 900;
	play.Session.MapName = "Two";
	play.Session.LoadScreen.Image = "load.pcx";
	play.Session.LoadScreen.X = -4;

	Message::Network network;
	network.Port = -12;
	Message::Peer peer;
	peer.Player = 1;
	peer.Address = "10.0.0.2";
	peer.Port = 1235;
	network.Peers = { peer };
	Message::Tunnel tunnel;
	tunnel.Address = "203.0.113.5";
	tunnel.Port = 50000;
	network.Tunnel = tunnel;
	network.ConnectTimeout = 57600;
	play.Network = network;

	play.Recording.Enabled = false;
	Message::FileHash hash;
	hash.Path = "rules.ini";
	hash.Hash = "sha256:00";
	play.Recording.Files = { hash };

	Message::Fixture fixture;
	fixture.Headless = true;
	fixture.StopFrame = 0;
	fixture.CheckpointCheck = Message::CheckpointCheck{ 30, 60, {} };
	play.Fixture = fixture;
	return(play);
}


RequestFault Decode_Bytes(ByteVector const & bytes, Message::Request & request)
{
	return(Decode_Request(bytes, request));
}


void Check_Round_Trip(Message::Request const & request)
{
	Bytes const bytes = Encoded(request);
	Message::Request back;
	REQUIRE(Decode_Request(bytes, back) == RequestFault::NONE);
	CHECK(Hex(Encoded(back)) == Hex(bytes));
	CHECK(back.Verb.index() == request.Verb.index());
}

}


TEST_CASE("a launch request of version 0 or 2 is refused and still names its result path and id")
{
	for (std::uint32_t version : { 0u, 2u }) {
		CAPTURE(version);
		Message::Request request;
		REQUIRE(Decode_Request(Encoded(Request_With(version, Sample_Play())), request) == RequestFault::VERSION);
		CHECK(request.Version == version);
		CHECK(request.ResultPath == "results/out.bin");
		CHECK(request.RequestId == "req-7");
		CHECK(std::holds_alternative<std::monostate>(request.Verb));
	}
}


TEST_CASE("a launch request with no verb, or only a verb the engine does not know, is refused")
{
	Message::Request request;
	CHECK(Decode_Request(Encoded(Request_With(1, std::monostate())), request) == RequestFault::NO_VERB);
	CHECK(request.ResultPath == "results/out.bin");
	CHECK(request.RequestId == "req-7");

	// Field 10 is no verb this version defines.
	CHECK(Decode_Bytes(From_Hex("0801" "5200" "4a0178"), request) == RequestFault::NO_VERB);
	CHECK(request.RequestId == "x");
}


TEST_CASE("a launch request with a string that is not valid UTF-8 is refused")
{
	Message::Request request;

	// The map path of the play verb.
	CHECK(Decode_Bytes(From_Hex("0801" "1203" "1201ff" "3a0172" "4a0178"), request) == RequestFault::TEXT);
	CHECK(request.Version == 1);
	CHECK(request.ResultPath == "r");
	CHECK(request.RequestId == "x");

	// A player's name in the play verb's game.
	CHECK(Decode_Bytes(From_Hex("0801" "1207" "0a05" "1a03" "1201ff"), request) == RequestFault::TEXT);

	// The request id itself, which is then left empty.
	CHECK(Decode_Bytes(From_Hex("0801" "1200" "3a0172" "4a01ff"), request) == RequestFault::TEXT);
	CHECK(request.ResultPath == "r");
	CHECK(request.RequestId.empty());
}


TEST_CASE("a launch request that is not a valid encoding, or is too large, is refused")
{
	Message::Request request;

	// The play field runs past the end; the version before it is still read.
	CHECK(Decode_Bytes(From_Hex("0801" "4a0178" "1205"), request) == RequestFault::MALFORMED);
	CHECK(request.Version == 1);
	CHECK(request.RequestId == "x");

	// A group wire type.
	CHECK(Decode_Bytes(From_Hex("0801" "1200" "1b"), request) == RequestFault::MALFORMED);

	// A malformed body inside the verb.
	CHECK(Decode_Bytes(From_Hex("0801" "1202" "0a05"), request) == RequestFault::MALFORMED);

	Message::Request large = Request_With(1, Sample_Play());
	large.Channel.assign(REQUEST_MAX_SIZE, 'c');
	CHECK(Decode_Request(Encoded(large), request) == RequestFault::MALFORMED);
	CHECK(request.RequestId == "req-7");
}


TEST_CASE("when a launch request sets two verbs the last one counts")
{
	Message::Request request;
	REQUIRE(Decode_Bytes(From_Hex("0801" "1200" "2a030a0161"), request) == RequestFault::NONE);
	REQUIRE(std::holds_alternative<Message::Verify>(request.Verb));
	CHECK(std::get<Message::Verify>(request.Verb).File == "a");

	REQUIRE(Decode_Bytes(From_Hex("0801" "2a030a0161" "1200"), request) == RequestFault::NONE);
	CHECK(std::holds_alternative<Message::Play>(request.Verb));
}


TEST_CASE("a verb that encodes empty is still written, and an empty network or tunnel is left out")
{
	CHECK(Hex(Encoded(Request_With(1, Message::Verify()))) == Hex(From_Hex("0801" "2a00" "3a0f" "726573756c74732f6f75742e62696e" "4a05" "7265712d37")));

	Message::Play play;
	play.Network = Message::Network();
	play.Fixture = Message::Fixture();
	CHECK(Hex(Encoded(play)).empty());

	Message::Network network;
	network.Tunnel = Message::Tunnel();
	CHECK(Hex(Encoded(network)).empty());
}


TEST_CASE("every verb survives an encode and a decode byte for byte")
{
	Check_Round_Trip(Request_With(1, Sample_Play()));

	Message::Load load;
	load.Save = "SAVE0001.SAV";
	load.Session.LocalPlayer = 1;
	load.Network = Message::Network();
	load.Network->Port = 1234;
	Message::Player player;
	player.Name = "Alpha";
	player.Human = true;
	load.Players = { player, player };
	load.GameSpeed = 0;
	Check_Round_Trip(Request_With(1, load));

	Message::Replay replay;
	replay.File = "Replays/a.cncr";
	replay.WatchPlayer = 0;
	replay.FreeView = true;
	replay.LoadScreen.Y = 7;
	Check_Round_Trip(Request_With(1, replay));

	Message::Verify verify;
	verify.File = "a.cncr";
	verify.OutPath = "b.cncr";
	Check_Round_Trip(Request_With(1, verify));

	Message::Capture capture;
	capture.MapPath = "maps/a.map";
	capture.OutPath = "a.png";
	Check_Round_Trip(Request_With(1, capture));

	// A field no version defines is kept in place.
	Message::Request unknown;
	REQUIRE(Decode_Bytes(From_Hex("0801" "1200" "4000" "c00601"), unknown) == RequestFault::NONE);
	CHECK(Hex(Encoded(unknown)) == "080112004000c00601");
}


TEST_CASE("a launch result survives an encode and a decode byte for byte")
{
	Message::Result result;
	result.RequestId = "req-7";
	result.Code = ResultCode::RESULT_CODE_START_MISMATCH;
	result.Message = "The start states differ.";
	Message::ScenarioResult scenario;
	scenario.MapPath = "maps/a.map";
	scenario.Outcome = GameOutcome::GAME_OUTCOME_NOT_STARTED;
	scenario.LastFrame = 0;
	scenario.NotRecorded = NotRecorded::NOT_RECORDED_RESUMED;
	result.Scenarios = { scenario, Message::ScenarioResult() };
	result.Unverified = true;
	result.MismatchFrame = 899;
	result.CrashPath = "C:/crash";

	Bytes const bytes = Encoded(result);
	Message::Result back;
	REQUIRE(Decode(bytes, back));
	CHECK(Hex(Encoded(back)) == Hex(bytes));
	REQUIRE(back.Scenarios.size() == 2);
	CHECK(back.Scenarios[0].LastFrame == std::optional<std::uint32_t>(0));
	CHECK(back.Code == ResultCode::RESULT_CODE_START_MISMATCH);
}
