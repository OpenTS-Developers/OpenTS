/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "doctest/doctest.h"
#include "replay/replaymessages.h"
#include "replaytest.h"

#include "replay/replayfields.hh"

#include <random>

using namespace Replay;


namespace {

RuleBreak Broke(RuleError error, std::string_view message, std::uint32_t field)
{
	return(RuleBreak{ error, message, field });
}

template<typename T>
std::string Encoded_Hex(T const & message)
{
	return(Hex(Encoded(message)));
}


// Decodes bytes and encodes the result again, which must give the same bytes back.
template<typename T>
T Round_Trip(ByteVector const & bytes)
{
	T message;
	REQUIRE(Decode(bytes, message));
	CHECK(Hex(Encoded(message)) == Hex(bytes));
	return(message);
}


UnknownField Unknown_Field(std::uint32_t number, char const * hex)
{
	UnknownField field;
	field.Number = number;
	field.Encoded = From_Hex(hex);
	return(field);
}


Message::Player Sample_Player(void)
{
	Message::Player player;
	player.House = 1;
	player.Name = "A";
	player.Human = true;
	player.Country = -1;
	player.CountryName = "GDI";
	player.Color = 3;
	player.Start = -1;
	player.Allies = { 2, 3 };
	player.ResolvedCountry = 0;
	player.ResolvedColor = 3;
	player.ResolvedStart = 2;
	return(player);
}

char const * const SAMPLE_PLAYER_HEX =
	"0801" "120141" "1801" "2801" "3203474449" "3806" "4001" "52020203" "5800" "6006" "6804";

}


TEST_CASE("a message whose fields are all zero encodes to no bytes")
{
	CHECK(Encoded(Message::RecordingInfo()).empty());
	CHECK(Encoded(Message::Launch()).empty());
	CHECK(Encoded(Message::Player()).empty());
	CHECK(Encoded(Message::Frame()).empty());
	CHECK(Encoded(Message::Event()).empty());
	CHECK(Encoded(Message::ViewState()).empty());
	CHECK(Encoded(Message::HouseSample()).empty());
	CHECK(Encoded(Message::Checkpoint()).empty());
	CHECK(Encoded(Message::Statistics()).empty());
	CHECK(Encoded(Message::HouseStatistics()).empty());
	CHECK(Encoded(Message::RecordingSummary()).empty());
}


TEST_CASE("a player encodes in field order with optional zeros and packed allies")
{
	CHECK(Encoded_Hex(Sample_Player()) == Hex(From_Hex(SAMPLE_PLAYER_HEX)));
	Message::Player const back = Round_Trip<Message::Player>(From_Hex(SAMPLE_PLAYER_HEX));
	CHECK(back.Country == -1);
	CHECK(back.Allies == std::vector<std::uint32_t>{ 2, 3 });
	CHECK(back.ResolvedCountry == 0);
	CHECK(!back.AILevel.has_value());
}


TEST_CASE("the launch bytes are pinned, oneof zeros and unknown fields included")
{
	Message::Launch launch;
	launch.Kind = GameKind::GAME_KIND_SKIRMISH;
	launch.Seed = 0;
	launch.Players.push_back(Sample_Player());

	Message::GameOption crates;
	crates.Key = "Crates";
	crates.Value = false;
	Message::GameOption credits;
	credits.Key = "Credits";
	credits.Value = std::int64_t(0);
	Message::GameOption mode;
	mode.Key = "Mode";
	mode.Value = std::string();
	launch.Options = { crates, credits, mode };
	launch.Anonymous = true;

	// Kept out of order on purpose: the encoder sorts unknown fields among the known ones.
	launch.Unknown.push_back(Unknown_Field(100, "a00605"));
	launch.Unknown.push_back(Unknown_Field(8, "4001"));

	std::string const expected = Hex(From_Hex(std::string(
		"0802"
		"1500000000"
		"1a1c") + SAMPLE_PLAYER_HEX +
		"220a" "0a06437261746573" "1800"
		"220b" "0a0743726564697473" "1000"
		"2208" "0a044d6f6465" "2200"
		"2801"
		"4001"
		"a00605"));
	CHECK(Encoded_Hex(launch) == expected);
	CHECK(Check_Order(launch) == RuleError::NONE);

	Message::Launch const back = Round_Trip<Message::Launch>(From_Hex(expected));
	CHECK(back.Seed == 0u);
	REQUIRE(back.Options.size() == 3);
	CHECK(std::get<bool>(back.Options[0].Value) == false);
	CHECK(std::get<std::int64_t>(back.Options[1].Value) == 0);
	CHECK(std::get<std::string>(back.Options[2].Value).empty());
	REQUIRE(back.Unknown.size() == 2);
	CHECK(back.Unknown[0].Number == 8);
	CHECK(back.Unknown[1].Number == 100);
}


TEST_CASE("a frame encodes events, view, samples, and defeats canonically")
{
	Message::Event event;
	event.Type = 5;
	event.House = 1;
	event.Due = -2;
	event.Payload = { 0x01, 0x02 };
	event.Role = EventRole::EVENT_ROLE_CHAT_PROOF;
	event.SealedPayload = Replay::Bytes();

	Message::HouseSample sample;
	sample.Credits = 0;

	Message::Frame frame;
	frame.Hash = 0;
	frame.Events.push_back(event);
	frame.View.Camera = Message::Point();
	frame.FrameMicros = 16666;
	frame.Samples.push_back(sample);
	frame.Defeated = { 2, 5 };

	std::string const expected = Hex(From_Hex(
		"1500000000"
		"1a0e" "0805" "1001" "1803" "22020102" "3001" "3a00"
		"22020a00"
		"309a8201"
		"3a021000"
		"52020205"));
	CHECK(Encoded_Hex(frame) == expected);

	Message::Frame const back = Round_Trip<Message::Frame>(From_Hex(expected));
	REQUIRE(back.Events.size() == 1);
	CHECK(back.Events[0].Due == -2);
	CHECK(back.Events[0].SealedPayload.has_value());
	CHECK(back.View.Camera.has_value());
	CHECK(!back.View.Selection.has_value());
	CHECK(back.Hash == 0u);
}


TEST_CASE("income kinds and income encode canonically, an income entry's private fields included")
{
	Message::IncomeKind kind;
	kind.Number = 2;
	kind.Id = "Crate";
	Message::RecordingInfo info;
	info.IncomeKinds.push_back(kind);

	std::string const info_hex = Hex(From_Hex("7a09" "0802" "1205" "4372617465"));
	CHECK(Encoded_Hex(info) == info_hex);
	Message::RecordingInfo const info_back = Round_Trip<Message::RecordingInfo>(From_Hex(info_hex));
	REQUIRE(info_back.IncomeKinds.size() == 1);
	CHECK(info_back.IncomeKinds[0].Number == 2);
	CHECK(info_back.IncomeKinds[0].Id == "Crate");

	Message::Income income;
	income.House = 1;
	income.Kind = 2;
	income.Amount = 25;
	income.Unknown.push_back(Unknown_Field(150, "b0092a"));

	Message::Frame frame;
	frame.Income.push_back(income);

	std::string const frame_hex = Hex(From_Hex("4209" "0801" "1002" "1832" "b0092a"));
	CHECK(Encoded_Hex(frame) == frame_hex);
	Message::Frame const frame_back = Round_Trip<Message::Frame>(From_Hex(frame_hex));
	REQUIRE(frame_back.Income.size() == 1);
	CHECK(frame_back.Income[0].Kind == 2);
	CHECK(frame_back.Income[0].Amount == 25);
	REQUIRE(frame_back.Income[0].Unknown.size() == 1);
	CHECK(frame_back.Income[0].Unknown[0].Number == 150);
}


TEST_CASE("a set optional message field with an empty encoding is written with length zero")
{
	Message::ViewState view;
	view.Selection = Message::Selection();
	CHECK(Encoded_Hex(view) == "1200");

	Message::Frame frame;
	frame.View = view;
	CHECK(Encoded_Hex(frame) == "22021200");

	Message::HouseStatistics house;
	house.Final = Message::HouseSample();
	CHECK_MESSAGE(Encoded(house).empty(), "a plain message field with an empty encoding is left out");
	house.Final.Credits = 0;
	CHECK(Encoded_Hex(house) == "4a021000");
}


TEST_CASE("a house's crate counts encode after its kills")
{
	Message::HouseKills kills;
	kills.House = 2;
	kills.Units = 3;
	Message::CrateCount crate;
	crate.Id = "Money";
	crate.Count = 4;

	Message::HouseStatistics house;
	house.House = 1;
	house.Kills.push_back(kills);
	house.Crates.push_back(crate);

	std::string const expected = Hex(From_Hex("0801" "5a04" "0802" "1003" "6209" "0a05" "4d6f6e6579" "1004"));
	CHECK(Encoded_Hex(house) == expected);
	Message::HouseStatistics const back = Round_Trip<Message::HouseStatistics>(From_Hex(expected));
	REQUIRE(back.Crates.size() == 1);
	CHECK(back.Crates[0].Id == "Money");
	CHECK(back.Crates[0].Count == 4);
}


TEST_CASE("a checkpoint at frame 0 has no frame field")
{
	Message::Checkpoint checkpoint;
	checkpoint.Crc32 = 0x12345678;
	checkpoint.State = { 0xAA };
	CHECK(Encoded_Hex(checkpoint) == "1578563412" "1a01aa");

	checkpoint.Frame = 900;
	CHECK(Encoded_Hex(checkpoint) == "088407" "1578563412" "1a01aa");
}


TEST_CASE("a summary keeps a last frame of zero and lists its seals")
{
	Message::Seal seal;
	seal.Through = 899;
	seal.Hash.assign(32, 0x11);

	Message::RecordingSummary summary;
	summary.LastFrame = 0;
	summary.Completed = true;
	summary.Seals.push_back(seal);
	summary.GameMicros = 0;

	std::string const expected = "0800" "2001" "2a25" "088307" "1220" + std::string(64, '1') + "3800";
	CHECK(Encoded_Hex(summary) == expected);
	Message::RecordingSummary const back = Round_Trip<Message::RecordingSummary>(From_Hex(expected));
	CHECK(back.LastFrame == 0u);
	CHECK(!back.ElapsedMicros.has_value());
	CHECK(back.GameMicros == 0u);
}


TEST_CASE("a non-repeated field that appears twice takes its last value")
{
	Message::Player player;
	REQUIRE(Decode(From_Hex("0801" "0802"), player));
	CHECK(player.House == 2);

	// The second view replaces the first whole.
	Message::Frame frame;
	REQUIRE(Decode(From_Hex("22040a020802" "22021200"), frame));
	CHECK(!frame.View.Camera.has_value());
	CHECK(frame.View.Selection.has_value());

	Message::GameOption option;
	REQUIRE(Decode(From_Hex("1002" "1801"), option));
	CHECK_MESSAGE(std::get<bool>(option.Value) == true, "the last oneof member counts");
}


TEST_CASE("repeated numeric fields decode packed and unpacked")
{
	Message::Player player;
	REQUIRE(Decode(From_Hex("5003" "52020405" "5006"), player));
	CHECK(player.Allies == std::vector<std::uint32_t>{ 3, 4, 5, 6 });
	CHECK_MESSAGE(Encoded_Hex(player) == "520403040506", "the writer packs them");
}


TEST_CASE("unknown fields and known fields of another wire type are kept byte for byte")
{
	ByteVector const bytes = From_Hex("0d01000000" "1801" "a2060568656c6c6f" "e8070a");
	Message::Player player;
	REQUIRE(Decode(bytes, player));
	CHECK(player.House == 0);
	CHECK(player.Human);
	REQUIRE(player.Unknown.size() == 3);
	CHECK(player.Unknown[0].Number == 1);
	CHECK(player.Unknown[1].Number == 100);
	CHECK(player.Unknown[2].Number == 125);
	CHECK(Encoded_Hex(player) == Hex(bytes));
}


TEST_CASE("malformed messages are refused")
{
	Message::Frame frame;
	CHECK_MESSAGE(!Decode(From_Hex("0b"), frame), "a group start");
	CHECK_MESSAGE(!Decode(From_Hex("0801" "0c"), frame), "a group end");
	CHECK_MESSAGE(!Decode(From_Hex("1a05" "0801"), frame), "a length past the end");
	CHECK_MESSAGE(!Decode(From_Hex("1a02" "0b00"), frame), "a malformed event inside the frame");
	CHECK_MESSAGE(!Decode(From_Hex("5202" "0280"), frame), "a packed list that ends inside a varint");
	CHECK_MESSAGE(!Decode(From_Hex("0001"), frame), "field number zero");

	Message::Launch launch;
	CHECK_MESSAGE(!Decode(From_Hex("1a03" "520502"), launch), "a malformed player");
}

TEST_CASE("a string that is not valid UTF-8 makes its message malformed")
{
	Message::RecordingInfo info;
	REQUIRE(Decode(From_Hex("4a03" "e298ba"), info));
	CHECK(info.MapName == "\xE2\x98\xBA");
	CHECK_MESSAGE(!Decode(From_Hex("4a01" "ff"), info), "a byte that never starts a character");
	CHECK_MESSAGE(!Decode(From_Hex("4a02" "c0af"), info), "an overlong form");
	CHECK_MESSAGE(!Decode(From_Hex("4a03" "eda080"), info), "a surrogate");
	CHECK_MESSAGE(!Decode(From_Hex("4a04" "f4908080"), info), "a code point past U+10FFFF");
	CHECK_MESSAGE(!Decode(From_Hex("4a02" "e298"), info), "a character cut short");

	Message::Launch launch;
	CHECK_MESSAGE(!Decode(From_Hex("1a04" "1202" "c328"), launch), "a player name");

	Message::Frame frame;
	CHECK_MESSAGE(Decode(From_Hex("1a03" "2201" "ff"), frame), "bytes fields are not text");
}


namespace {

class Randomizer
{
	public:
		explicit Randomizer(unsigned seed) : Engine(seed) {}

		std::uint32_t Number(std::uint32_t limit)
		{
			return(std::uniform_int_distribution<std::uint32_t>(0, limit)(Engine));
		}

		bool Coin(void)
		{
			return(Number(1) == 1);
		}

		// Small values are common in replays and zero is left out of the encoding, so both are
		// drawn often.
		std::int64_t Signed(void)
		{
			switch (Number(3)) {
				case 0: return(0);
				case 1: return(std::int64_t(Number(300)) - 150);
				case 2: return(std::int64_t(Engine()) << 32 | Engine());
				default: return(-std::int64_t(Number(0x7FFFFFFF)));
			}
		}

		Replay::Bytes Blob(std::size_t limit)
		{
			Replay::Bytes bytes(Number(std::uint32_t(limit)));
			for (std::uint8_t & byte : bytes) {
				byte = std::uint8_t(Engine());
			}
			return(bytes);
		}

		std::mt19937 Engine;
};


Message::Frame Random_Frame(Randomizer & random)
{
	Message::Frame frame;
	frame.Skipped = random.Coin() ? 0 : random.Number(2000);
	if (random.Coin()) {
		frame.Hash = random.Number(0xFFFFFFFF);
	}
	for (std::uint32_t i = random.Number(4); i > 0; i--) {
		Message::Event event;
		event.Type = random.Number(60);
		event.House = random.Number(8);
		event.Due = std::int32_t(random.Signed());
		event.Payload = random.Blob(40);
		event.Local = random.Coin();
		event.Role = EventRole(random.Number(1));
		if (random.Number(4) == 0) {
			event.SealedPayload = random.Blob(40);
		}
		frame.Events.push_back(event);
	}
	if (random.Coin()) {
		Message::Point point;
		point.X = std::int32_t(random.Signed());
		point.Y = std::int32_t(random.Signed());
		frame.View.Camera = point;
	}
	if (random.Number(3) == 0) {
		Message::Selection selection;
		for (std::uint32_t i = random.Number(5); i > 0; i--) {
			selection.Ids.push_back(random.Number(0xFFFFFFFF));
		}
		frame.View.Selection = selection;
	}
	if (random.Number(5) == 0) {
		Message::PlayerMessage message;
		message.House = random.Number(8);
		message.Sender = "Sender";
		message.Text = "text";
		message.Color = std::int32_t(random.Signed());
		message.Scope = MessageScope(random.Number(4));
		message.Recipients = { 1, 2 };
		message.Salt = random.Blob(16);
		frame.Messages.push_back(message);
	}
	if (random.Coin()) {
		frame.FrameMicros = random.Number(100000);
	}
	if (random.Number(4) == 0) {
		Message::HouseSample sample;
		sample.House = random.Number(8);
		sample.Credits = random.Signed();
		if (random.Coin()) {
			sample.PowerDrain = std::int32_t(random.Signed());
		}
		sample.Vehicles = random.Number(100);
		sample.Defeated = random.Coin();
		frame.Samples.push_back(sample);
	}
	if (random.Number(4) == 0) {
		Message::Income income;
		income.House = random.Number(8);
		income.Kind = random.Number(0xFFFFFFFF);
		income.Amount = random.Signed();
		if (random.Coin()) {
			income.Unknown.push_back(Unknown_Field(150, "b0092a"));
		}
		frame.Income.push_back(income);
	}
	if (random.Number(5) == 0) {
		frame.Defeated.push_back(random.Number(8));
	}
	if (random.Number(6) == 0) {
		frame.Unknown.push_back(Unknown_Field(101, "a80601"));
	}
	return(frame);
}

}


TEST_CASE("random frames survive an encode and decode unchanged")
{
	Randomizer random(20261116);
	for (int i = 0; i < 2000; i++) {
		Message::Frame const frame = Random_Frame(random);
		Replay::Bytes const bytes = Encoded(frame);
		Message::Frame back;
		REQUIRE(Decode(bytes, back));
		CHECK_MESSAGE(Encoded(back) == bytes, "iteration ", i);
		CHECK(back.Events.size() == frame.Events.size());
		CHECK(back.Hash == frame.Hash);
		CHECK(back.FrameMicros == frame.FrameMicros);
	}
}


TEST_CASE("the limits of section 10 hold at their values and refuse one more")
{
	LimitsType limits;
	limits.String = 4;
	limits.Players = 2;
	limits.EventsPerFrame = 2;
	limits.EventPayload = 3;

	Message::Launch launch;
	launch.Players.resize(2);
	launch.Players[0].Name = "abcd";
	CHECK(Check_Limits(launch, limits) == RuleError::NONE);
	launch.Players.resize(3);
	CHECK(Check_Limits(launch, limits) == Broke(RuleError::TOO_MANY, "Launch", Field::Launch::PLAYERS));
	launch.Players.resize(2);
	launch.Players[0].Name = "abcde";
	CHECK(Check_Limits(launch, limits) == Broke(RuleError::TOO_LONG, "Player", Field::Player::NAME));

	Message::Frame frame;
	frame.Events.resize(2);
	frame.Events[0].Payload.assign(3, 0);
	CHECK(Check_Limits(frame, limits) == RuleError::NONE);
	frame.Events[1].SealedPayload = Replay::Bytes(4, 0);
	CHECK(Check_Limits(frame, limits) == Broke(RuleError::TOO_LONG, "Event", Field::Event::SEALED_PAYLOAD));
	frame.Events.resize(3);
	CHECK(Check_Limits(frame, limits) == Broke(RuleError::TOO_MANY, "Frame", Field::Frame::EVENTS));

	limits.IncomeKinds = 2;
	Message::RecordingInfo kinds;
	kinds.IncomeKinds.resize(2);
	kinds.IncomeKinds[0].Id = "abcd";
	CHECK(Check_Limits(kinds, limits) == RuleError::NONE);
	kinds.IncomeKinds[1].Id = "abcde";
	CHECK(Check_Limits(kinds, limits) == Broke(RuleError::TOO_LONG, "IncomeKind", Field::IncomeKind::ID));
	kinds.IncomeKinds.resize(3);
	CHECK(Check_Limits(kinds, limits) == Broke(RuleError::TOO_MANY, "RecordingInfo", Field::RecordingInfo::INCOME_KINDS));

	Message::Statistics statistics;
	statistics.Houses.resize(3);
	CHECK(Check_Limits(statistics, limits) == Broke(RuleError::TOO_MANY, "Statistics", Field::Statistics::HOUSES));
	statistics.Houses.resize(2);
	statistics.Houses[1].Crates.push_back(Message::CrateCount{ "abcd", 1, {} });
	CHECK(Check_Limits(statistics, limits) == RuleError::NONE);
	statistics.Houses[1].Crates.push_back(Message::CrateCount{ "abcde", 1, {} });
	CHECK(Check_Limits(statistics, limits) == Broke(RuleError::TOO_LONG, "CrateCount", Field::CrateCount::ID));

	Message::RecordingInfo info;
	info.Files.push_back(Message::FileHash{ "abcde", "", {} });
	CHECK(Check_Limits(info, limits) == Broke(RuleError::TOO_LONG, "FileHash", Field::FileHash::PATH));

	LimitsType const standard;
	CHECK(standard.SmallSection == 1024 * 1024);
	CHECK(standard.LargeSection == 32 * 1024 * 1024);
	CHECK(standard.Checkpoint == 64 * 1024 * 1024);
	CHECK(standard.FrameRecord == 1024 * 1024);
	CHECK(standard.EventsPerFrame == 16384);
	CHECK(standard.EventPayload == 4096);
	CHECK(standard.String == 1024);
	CHECK(standard.Players == 64);
	CHECK(standard.IncomeKinds == 4096);
}


namespace {

ByteVector Repeat(char const * hex, std::size_t times)
{
	ByteVector const unit = From_Hex(hex);
	ByteVector out;
	out.reserve(unit.size() * times);
	for (std::size_t i = 0; i < times; i++) {
		out.insert(out.end(), unit.begin(), unit.end());
	}
	return(out);
}

}


TEST_CASE("a limited decode refuses each occurrence past a limit, a replaced one included")
{
	LimitsType const limits;
	RuleBreak broken;

	// engine is one byte past the string limit, and a later engine field replaces it.
	ByteVector info_bytes = From_Hex("0a8108");
	info_bytes.insert(info_bytes.end(), 1025, 'a');
	ByteVector const replacement = From_Hex("0a06");
	info_bytes.insert(info_bytes.end(), replacement.begin(), replacement.end());
	ByteVector const engine = Bytes_Of("opents");
	info_bytes.insert(info_bytes.end(), engine.begin(), engine.end());
	Message::RecordingInfo info;
	REQUIRE(Decode(info_bytes, info));
	CHECK(info.Engine == "opents");
	CHECK(Check_Limits(info, limits) == RuleError::NONE);
	CHECK(!Decode(info_bytes, info, limits, broken));
	CHECK(broken == Broke(RuleError::TOO_LONG, "RecordingInfo", Field::RecordingInfo::ENGINE));

	// A payload one byte past the payload limit, which a second payload replaces.
	ByteVector event = From_Hex("228120");
	event.insert(event.end(), 4097, 0);
	ByteVector const small = From_Hex("220101");
	event.insert(event.end(), small.begin(), small.end());
	ByteVector frame_bytes = From_Hex("1a");
	Put_Varint(frame_bytes, event.size());
	frame_bytes.insert(frame_bytes.end(), event.begin(), event.end());
	Message::Frame frame;
	REQUIRE(Decode(frame_bytes, frame));
	CHECK(Check_Limits(frame, limits) == RuleError::NONE);
	CHECK(!Decode(frame_bytes, frame, limits, broken));
	CHECK(broken == Broke(RuleError::TOO_LONG, "Event", Field::Event::PAYLOAD));

	Message::Frame within;
	CHECK(Decode(Encoded(frame), within, limits, broken));
	CHECK(broken == RuleError::NONE);
	CHECK_MESSAGE(!Decode(From_Hex("0a05"), within, limits, broken), "a malformed message");
	CHECK(broken == RuleError::NONE);
}


TEST_CASE("a limited decode stops a count at its limit before storing more")
{
	LimitsType const limits;
	RuleBreak broken;

	// Far more empty players than the limit.
	Message::Launch launch;
	CHECK(!Decode(Repeat("1a00", 1 << 19), launch, limits, broken));
	CHECK(broken == Broke(RuleError::TOO_MANY, "Launch", Field::Launch::PLAYERS));
	CHECK(launch.Players.size() == limits.Players);
	CHECK(Decode(Repeat("1a00", limits.Players), launch, limits, broken));

	// Far more empty houses than the limit.
	Message::Statistics statistics;
	CHECK(!Decode(Repeat("1200", 1 << 24), statistics, limits, broken));
	CHECK(broken == Broke(RuleError::TOO_MANY, "Statistics", Field::Statistics::HOUSES));
	CHECK(statistics.Houses.size() == limits.Players);

	Message::Frame frame;
	CHECK(!Decode(Repeat("1a00", limits.EventsPerFrame + 1), frame, limits, broken));
	CHECK(broken == Broke(RuleError::TOO_MANY, "Frame", Field::Frame::EVENTS));
	CHECK(frame.Events.size() == limits.EventsPerFrame);
	CHECK(Decode(Repeat("1a00", limits.EventsPerFrame), frame, limits, broken));

	Message::RecordingInfo info;
	CHECK(!Decode(Repeat("7a00", limits.IncomeKinds + 1), info, limits, broken));
	CHECK(broken == Broke(RuleError::TOO_MANY, "RecordingInfo", Field::RecordingInfo::INCOME_KINDS));
	CHECK(info.IncomeKinds.size() == limits.IncomeKinds);
	CHECK(Decode(Repeat("7a00", limits.IncomeKinds), info, limits, broken));

	// A players field (3) of another wire type is kept as unknown and does not count.
	CHECK(Decode(Repeat("1800", limits.Players + 1), launch, limits, broken));
	CHECK(launch.Players.empty());
	REQUIRE(launch.Unknown.size() == limits.Players + 1);
	CHECK(launch.Unknown[0].Number == 3);
}


TEST_CASE("the writer's ordering rules catch lists out of order")
{
	Message::Launch launch;
	launch.Players.resize(2);
	launch.Players[0].House = 3;
	launch.Players[1].House = 1;
	CHECK(Check_Order(launch) == Broke(RuleError::NOT_ASCENDING, "Launch", Field::Launch::PLAYERS));
	launch.Players[1].House = 4;
	CHECK(Check_Order(launch) == RuleError::NONE);
	launch.Players[1].Allies = { 4 };
	CHECK_MESSAGE(Check_Order(launch) == Broke(RuleError::NOT_ASCENDING, "Player", Field::Player::ALLIES), "a house never lists itself");
	launch.Players[1].Allies = { 3 };

	launch.Options.resize(2);
	launch.Options[0].Key = "b";
	launch.Options[1].Key = "a";
	CHECK(Check_Order(launch) == Broke(RuleError::NOT_ASCENDING, "Launch", Field::Launch::OPTIONS));
	launch.Options[1].Key = "b";
	CHECK_MESSAGE(Check_Order(launch) == Broke(RuleError::NOT_ASCENDING, "Launch", Field::Launch::OPTIONS), "each key once");
	launch.Options[0].Key = "Z";
	launch.Options[1].Key = "\xC3\xA9";
	CHECK_MESSAGE(Check_Order(launch) == RuleError::NONE, "keys compare as unsigned bytes");

	Message::Frame frame;
	frame.Defeated = { 1, 1 };
	CHECK(Check_Order(frame) == Broke(RuleError::NOT_ASCENDING, "Frame", Field::Frame::DEFEATED));

	Message::RecordingInfo info;
	info.Colors.resize(2);
	CHECK(Check_Order(info) == Broke(RuleError::NOT_ASCENDING, "RecordingInfo", Field::RecordingInfo::COLORS));
}


namespace {

Message::RecordingInfo With_Kinds(std::vector<std::pair<std::uint32_t, std::string>> const & kinds)
{
	Message::RecordingInfo info;
	for (auto const & kind : kinds) {
		Message::IncomeKind entry;
		entry.Number = kind.first;
		entry.Id = kind.second;
		info.IncomeKinds.push_back(entry);
	}
	return(info);
}


Message::Frame Paying(std::vector<std::uint32_t> const & used)
{
	Message::Frame frame;
	for (std::uint32_t number : used) {
		Message::Income income;
		income.Kind = number;
		income.Amount = 1;
		frame.Income.push_back(income);
	}
	return(frame);
}

}


TEST_CASE("INFO lists each income kind once, and income uses only listed kinds")
{
	CHECK(Check_Income_Kinds(With_Kinds({})) == RuleError::NONE);
	CHECK(Check_Income_Kinds(With_Kinds({ { 1, "Crate" }, { 2, "Sold/Unit" } })) == RuleError::NONE);
	RuleBreak const listing = Broke(RuleError::INVALID, "RecordingInfo", Field::RecordingInfo::INCOME_KINDS);
	CHECK_MESSAGE(Check_Income_Kinds(With_Kinds({ { 1, "" } })) == listing, "an empty id");
	CHECK_MESSAGE(Check_Income_Kinds(With_Kinds({ { 1, "Crate" }, { 1, "Trigger" } })) == listing, "a number twice");
	CHECK_MESSAGE(Check_Income_Kinds(With_Kinds({ { 1, "Crate" }, { 2, "Crate" } })) == listing, "an id twice");

	IncomeKindSet const listed(With_Kinds({ { 1, "Crate" }, { 3, "Trigger" } }));
	CHECK(listed.Check(Paying({})) == RuleError::NONE);
	CHECK(listed.Check(Paying({ 3, 1, 3 })) == RuleError::NONE);
	RuleBreak const unlisted = Broke(RuleError::INVALID, "Income", Field::Income::KIND);
	CHECK_MESSAGE(listed.Check(Paying({ 1, 2 })) == unlisted, "a kind INFO does not list");
	CHECK_MESSAGE(IncomeKindSet().Check(Paying({ 0 })) == unlisted, "kind 0 must be listed too");
}


TEST_CASE("text is cleaned to valid UTF-8 and cut on a character boundary")
{
	std::string const fffd = "\xEF\xBF\xBD";
	CHECK(Clean_Text("abc", 1024) == "abc");
	CHECK(Clean_Text("\xC3\xA9", 1024) == "\xC3\xA9");
	CHECK(Clean_Text("\xF0\x9F\x98\x80", 1024) == "\xF0\x9F\x98\x80");
	CHECK(Clean_Text("a\xFF" "b", 1024) == "a" + fffd + "b");
	CHECK_MESSAGE(Clean_Text("\xE2\x82", 1024) == fffd, "a cut sequence is one replacement");
	CHECK(Clean_Text("\xE2\x82" "x", 1024) == fffd + "x");
	CHECK_MESSAGE(Clean_Text("\xED\xA0\x80", 1024) == fffd + fffd + fffd, "a surrogate is three");
	CHECK(Clean_Text("\xC0\xAF", 1024) == fffd + fffd);
	CHECK(Clean_Text("\xF4\x90\x80\x80", 1024) == fffd + fffd + fffd + fffd);

	CHECK(Clean_Text("a\xC3\xA9", 2) == "a");
	CHECK(Clean_Text("a\xC3\xA9", 3) == "a\xC3\xA9");
	CHECK(Clean_Text("abc", 0).empty());
}
