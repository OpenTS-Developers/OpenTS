/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Exercises the network event contract without starting the engine or loading game data.

#include "netadmit.h"
#include "netpacket.h"
#include "netreader.h"
#include "netglobal.h"
#include "autosave.h"

#include "doctest/doctest.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <string>
#include <utility>
#include <vector>


namespace {

using Bytes = std::vector<std::byte>;
using VariableDataType = decltype(std::declval<EventClass>().Data.Variable);
using NetworkReportType = decltype(std::declval<EventClass>().Data.NetworkReport);
using AbandonCountType = decltype(std::declval<EventClass>().Data.AbandonCount);

constexpr int Sender = 3;
constexpr int PacketFrame = 120;
constexpr std::size_t DataOffset = offsetof(EventClass, Data);
constexpr std::size_t FrameOffset = offsetof(EventClass, Frame);
constexpr std::size_t EnvelopeSize = DataOffset + sizeof(std::declval<EventClass>().Data.FrameInfo);
constexpr std::size_t FrameDelayOffset = DataOffset + offsetof(decltype(std::declval<EventClass>().Data.FrameInfo), Delay);
constexpr std::size_t VariableSizeOffset = offsetof(VariableDataType, Size);
constexpr std::size_t MegaWhomSize = sizeof(std::declval<EventClass>().Data.MegaMission.Whom);


// A rejected packet fails with the expected error and hands out no events.
void Check_Error(NetPacket::DecodeResult const & result, NetPacket::DecodeError expected)
{
	INFO("got ", NetPacket::Error_Name(result.Failure.Code), " at offset ", result.Failure.Offset);
	CHECK_FALSE(result.Succeeded());
	CHECK(result.Failure.Code == expected);
	CHECK(result.Events.empty());
}


template<typename T>
void Append_Value(Bytes & bytes, T const & value)
{
	std::byte const * first = reinterpret_cast<std::byte const *>(&value);
	bytes.insert(bytes.end(), first, first + sizeof(value));
}


void Append_Bytes(Bytes & bytes, std::span<std::byte const> value)
{
	bytes.insert(bytes.end(), value.begin(), value.end());
}


template<typename T>
void Write_Value(Bytes & bytes, std::size_t offset, T const & value)
{
	std::memcpy(bytes.data() + offset, &value, sizeof(value));
}


Bytes Full_Event(std::uint8_t type, int sender = Sender, int frame = PacketFrame)
{
	EventClass event;
	std::memset(&event, 0, sizeof(event));
	event.Type = type;
	event.Frame = frame;
	event.ID = sender;
	event.Data.FrameInfo.CRC = 0x12345678;
	event.Data.FrameInfo.CommandCount = 23;
	event.Data.FrameInfo.Delay = 4;

	Bytes bytes(sizeof(event));
	std::memcpy(bytes.data(), &event, sizeof(event));
	return(bytes);
}


Bytes Envelope(std::uint8_t type, int sender = Sender)
{
	Bytes bytes = Full_Event(type, sender);
	bytes.resize(EnvelopeSize);
	return(bytes);
}


Bytes Compressed_Packet(void)
{
	return(Envelope(EventClass::FRAMEINFO));
}


void Add_Compressed_Event(Bytes & packet, std::uint8_t type, std::span<std::byte const> data = {})
{
	packet.push_back(static_cast<std::byte>(type));
	Append_Bytes(packet, data);
}


NetPacket::DecodeResult Decode(Bytes const & packet, NetPacket::Encoding encoding = NetPacket::Encoding::COMPRESSED, int sender = Sender)
{
	return(NetPacket::Decode_Event_Packet(packet, encoding, sender));
}


Bytes Valid_Compressed_Event(std::uint8_t type)
{
	Bytes packet = Compressed_Packet();
	packet.push_back(static_cast<std::byte>(type));

	if (type == EventClass::MEGAMISSION) {
		packet.push_back(std::byte{1});
	} else if (type == EventClass::ADDPLAYER) {
		std::uint32_t const size = 0;
		Append_Value(packet, size);
	}

	if (type != EventClass::ADDPLAYER) {
		packet.insert(packet.end(), EventClass::EventLength[type], std::byte{0});
	}
	return(packet);
}


// A decoded packet holds the envelope and one ADDPLAYER that owns three bytes of data.
void Check_Add_Player(NetPacket::DecodeResult const & result)
{
	REQUIRE(result.Succeeded());
	REQUIRE(result.Events.size() == 2u);
	NetPacket::DecodedEvent const & event = result.Events[1];
	CHECK(event.Event.Type == EventClass::ADDPLAYER);
	CHECK(event.Event.Data.Variable.Size == 3u);
	REQUIRE(event.AddPlayerData.size() == 3u);
	CHECK(event.Event.Data.Variable.Pointer == event.AddPlayerData.data());
	CHECK(std::to_integer<int>(event.AddPlayerData[0]) == 0x11);
	CHECK(std::to_integer<int>(event.AddPlayerData[2]) == 0x33);
}


Bytes const AddPlayerPayload{std::byte{0x11}, std::byte{0x22}, std::byte{0x33}};


Bytes Compressed_Add_Player(void)
{
	Bytes compressed = Compressed_Packet();
	compressed.push_back(static_cast<std::byte>(EventClass::ADDPLAYER));
	std::uint32_t const size = static_cast<std::uint32_t>(AddPlayerPayload.size());
	Append_Value(compressed, size);
	Append_Bytes(compressed, AddPlayerPayload);
	return(compressed);
}


Bytes Uncompressed_Add_Player(void)
{
	Bytes uncompressed = Full_Event(EventClass::FRAMEINFO);
	Bytes add = Full_Event(EventClass::ADDPLAYER);
	std::uint32_t const size = static_cast<std::uint32_t>(AddPlayerPayload.size());
	Write_Value(add, DataOffset + VariableSizeOffset, size);
	Append_Bytes(uncompressed, add);
	Append_Bytes(uncompressed, AddPlayerPayload);
	return(uncompressed);
}


Bytes Two_Uncompressed_Events(void)
{
	Bytes packet = Full_Event(EventClass::FRAMEINFO);
	Bytes speed = Full_Event(EventClass::GAMESPEED, Sender, PacketFrame + 6);
	int const value = 5;
	Write_Value(speed, DataOffset, value);
	Append_Bytes(packet, speed);
	return(packet);
}


Bytes Datagram(Bytes const & payload)
{
	Bytes datagram;
	std::uint32_t const crc = NetAdmission::Calculate_Datagram_CRC(payload);
	Append_Value(datagram, crc);
	Append_Bytes(datagram, payload);
	return(datagram);
}


Bytes Connection_Packet(std::size_t header_size, std::uint8_t code, std::size_t payload_size)
{
	Bytes packet(header_size + payload_size, std::byte{0});
	std::uint16_t const magic = 0xCAFE;
	std::uint32_t const packet_id = 0x12345678;
	Write_Value(packet, 0, magic);
	Write_Value(packet, sizeof(magic), code);
	Write_Value(packet, sizeof(magic) + sizeof(code), packet_id);
	return(packet);
}


void Check_Admission_Error(NetAdmission::Error actual, NetAdmission::Error expected)
{
	INFO("got ", NetAdmission::Error_Name(actual));
	CHECK(actual == expected);
}


GlobalPacketType Global_Packet(NetCommandType command)
{
	GlobalPacketType packet = {};
	packet.Command = command;
	packet.Name[0] = '\0';
	packet.Message.Buf[0] = '\0';
	return(packet);
}


NetGlobal::ValidationContext Member_Context(void)
{
	NetGlobal::ValidationContext context;
	context.SenderIsMember = true;
	context.SenderPlayerID = 2;
	context.SenderPlayerColor = 3;
	context.ActivePlayers[2] = true;
	context.ActivePlayers[5] = true;
	return(context);
}


constexpr std::size_t GlobalPacketSize = sizeof(GlobalPacketType);


void Check_Global_Error(GlobalPacketType const & packet, std::size_t length, NetGlobal::ValidationContext const & context, NetGlobal::DecodeError expected)
{
	NetGlobal::DecodeError const actual = NetGlobal::Validate_In_Game_Packet(packet, length, context);
	INFO("got ", NetGlobal::Error_Name(actual));
	CHECK(actual == expected);
}


void Check_Global_Error(GlobalPacketType const & packet, NetGlobal::ValidationContext const & context, NetGlobal::DecodeError expected)
{
	Check_Global_Error(packet, GlobalPacketSize, context, expected);
}


NetGlobal::ValidationContext Master_Context(int master)
{
	NetGlobal::ValidationContext context = Member_Context();
	context.MasterPlayerID = master;
	return(context);
}

}	// namespace


TEST_CASE("the reader copies fixed-width values, refuses a span past the remainder without moving, and resumes")
{
	Bytes bytes;
	std::uint16_t const first = 0x1234;
	std::uint32_t const second = 0x89ABCDEF;
	Append_Value(bytes, first);
	Append_Value(bytes, second);

	NetPacket::Reader reader(bytes);
	auto got_first = reader.Read_Value<std::uint16_t>();
	REQUIRE(got_first);
	CHECK(*got_first == first);
	CHECK_MESSAGE(reader.Offset() == sizeof(first), "the reader reports its consumed offset");

	std::size_t const before_failure = reader.Offset();
	CHECK_FALSE(reader.Take(bytes.size()));
	CHECK_MESSAGE(reader.Offset() == before_failure, "a failed read does not advance the cursor");

	auto got_second = reader.Read_Value<std::uint32_t>();
	REQUIRE(got_second);
	CHECK(*got_second == second);
	CHECK(reader.Empty());
	CHECK_MESSAGE(reader.Take(0).has_value(), "the reader can take an empty span at the end");
}


TEST_CASE("the inherited events keep their numbers, and the new ones append after LATENCYFUDGE")
{
	CHECK(EventClass::LATENCYFUDGE == 35);
	CHECK(EventClass::NETWORK_REPORT == 36);
	CHECK(EventClass::ABANDON_COUNT == 37);
	CHECK(EventClass::GRANT_CONTROL == 38);
	CHECK(EventClass::LAST_EVENT == 39);
}


TEST_CASE("the appended events have their payload sizes and diagnostic names")
{
	CHECK(EventClass::EventLength[EventClass::NETWORK_REPORT] == sizeof(NetworkReportType));
	CHECK(sizeof(NetworkReportType) == 6u);
	CHECK(std::string(EventClass::EventNames[EventClass::NETWORK_REPORT]) == "NETWORK_REPORT");
	CHECK(EventClass::EventLength[EventClass::ABANDON_COUNT] == sizeof(AbandonCountType));
	CHECK(sizeof(AbandonCountType) == 12u);
	CHECK(std::string(EventClass::EventNames[EventClass::ABANDON_COUNT]) == "ABANDON_COUNT");
	CHECK_MESSAGE(EventClass::EventLength[EventClass::GRANT_CONTROL] == sizeof(int), "GRANT_CONTROL carries one house index");
	CHECK(std::string(EventClass::EventNames[EventClass::GRANT_CONTROL]) == "GRANT_CONTROL");
}


TEST_CASE("the unavailable RTT sentinel is uint16 max, and the full and envelope layouts keep their sizes")
{
	CHECK(EventClass::NETWORK_RTT_UNAVAILABLE == UINT16_MAX);
	CHECK(sizeof(EventClass) == 46u);
	CHECK(EnvelopeSize == 17u);
}


TEST_CASE("a compressed packet that is empty or does not begin with FRAMEINFO or FRAMESYNC is rejected")
{
	Check_Error(Decode({}), NetPacket::DecodeError::EMPTY_PACKET);

	Bytes invalid_prefix{static_cast<std::byte>(EventClass::GAMESPEED)};
	Check_Error(Decode(invalid_prefix), NetPacket::DecodeError::INVALID_PREFIX);

	Bytes unknown{static_cast<std::byte>(0xFF)};
	INFO("an unknown prefix type is rejected before table lookup");
	Check_Error(Decode(unknown), NetPacket::DecodeError::INVALID_EVENT_TYPE);
}


TEST_CASE("every incomplete compressed envelope is rejected transactionally")
{
	Bytes complete = Compressed_Packet();
	for (std::size_t size = 1; size < complete.size(); size++) {
		INFO("envelope cut to ", size, " bytes");
		Bytes truncated(complete.begin(), complete.begin() + size);
		Check_Error(Decode(truncated), NetPacket::DecodeError::TRUNCATED_ENVELOPE);
	}
}


TEST_CASE("a complete FRAMEINFO-only packet decodes and keeps FRAMEINFO for the execution queue")
{
	NetPacket::DecodeResult header = Decode(Compressed_Packet());
	REQUIRE(header.Succeeded());
	REQUIRE(header.Events.size() == 1u);
	CHECK(header.Events[0].Event.Type == EventClass::FRAMEINFO);
	CHECK_MESSAGE(header.Events[0].Event.ID == Sender, "FRAMEINFO carries the canonical sender");
	CHECK_MESSAGE(header.Events[0].Event.Frame == PacketFrame, "FRAMEINFO carries the canonical frame");
	CHECK_MESSAGE(header.Events[0].Event.Data.FrameInfo.CRC == 0x12345678u, "FRAMEINFO carries its complete data prefix");
}


TEST_CASE("the envelope sender must match the demultiplexer sender")
{
	Check_Error(Decode(Compressed_Packet(), NetPacket::Encoding::COMPRESSED, Sender + 1), NetPacket::DecodeError::SENDER_MISMATCH);
}


TEST_CASE("a sole short FRAMESYNC is accepted, exposed, and not queued, and rejects every trailing byte")
{
	for (NetPacket::Encoding encoding : {NetPacket::Encoding::COMPRESSED, NetPacket::Encoding::UNCOMPRESSED}) {
		INFO((encoding == NetPacket::Encoding::COMPRESSED ? "compressed" : "uncompressed"));
		Bytes framesync = Envelope(EventClass::FRAMESYNC);
		NetPacket::DecodeResult result = Decode(framesync, encoding);
		CHECK(result.Succeeded());
		CHECK(result.Events.empty());
		CHECK(result.HasEnvelope);
		CHECK(result.Envelope.Type == EventClass::FRAMESYNC);

		framesync.push_back(std::byte{0});
		Check_Error(Decode(framesync, encoding), NetPacket::DecodeError::FRAMESYNC_NOT_ALONE);
	}
}


TEST_CASE("a compressed packet rejects a nested envelope")
{
	Bytes nested = Compressed_Packet();
	Add_Compressed_Event(nested, EventClass::FRAMEINFO);
	Check_Error(Decode(nested), NetPacket::DecodeError::NESTED_ENVELOPE);
}


TEST_CASE("an uncompressed FRAMEINFO must carry the complete full event")
{
	Check_Error(Decode(Envelope(EventClass::FRAMEINFO), NetPacket::Encoding::UNCOMPRESSED), NetPacket::DecodeError::TRUNCATED_ENVELOPE);
}


TEST_CASE("frame arithmetic that would go negative or overflow rejects the packet")
{
	SUBCASE("a negative FRAMEINFO frame")
	{
		Bytes negative = Compressed_Packet();
		int const negative_frame = -1;
		Write_Value(negative, FrameOffset, negative_frame);
		Check_Error(Decode(negative), NetPacket::DecodeError::INVALID_FRAME_ARITHMETIC);
	}

	SUBCASE("a FRAMEINFO delay larger than its frame")
	{
		Bytes underflow = Compressed_Packet();
		int const early_frame = 3;
		std::uint8_t const excessive_delay = 4;
		Write_Value(underflow, FrameOffset, early_frame);
		Write_Value(underflow, FrameDelayOffset, excessive_delay);
		Check_Error(Decode(underflow), NetPacket::DecodeError::INVALID_FRAME_ARITHMETIC);
	}

	SUBCASE("the minimum signed FRAMESYNC frame, which must not overflow the subtraction")
	{
		Bytes minimum = Envelope(EventClass::FRAMESYNC);
		int const minimum_frame = (std::numeric_limits<int>::min)();
		Write_Value(minimum, FrameOffset, minimum_frame);
		Check_Error(Decode(minimum, NetPacket::Encoding::UNCOMPRESSED), NetPacket::DecodeError::INVALID_FRAME_ARITHMETIC);
	}
}


TEST_CASE("the reported frame is checked against underflow and the sender's lead")
{
	CHECK_MESSAGE(NetPacket::Compute_Reported_Frame(4, 4) == 0, "a delay equal to its frame reports frame zero");
	CHECK_MESSAGE(!NetPacket::Compute_Reported_Frame(3, 4), "checked sender-frame subtraction rejects underflow");
	CHECK_MESSAGE(NetPacket::Compute_Reported_Frame(350, 0, 100, 250) == 350, "the maximum 250-frame sender lead is accepted");
	CHECK_MESSAGE(!NetPacket::Compute_Reported_Frame(351, 0, 100, 250), "an excessive future sender frame is rejected");
	CHECK_MESSAGE(NetPacket::Compute_Reported_Frame((std::numeric_limits<int>::max)(), 0, (std::numeric_limits<int>::max)(), 250).has_value(),
		"receiver lead arithmetic remains safe at the signed-frame limit");
}


TEST_CASE("player orders accept no mission and a queued move, and nothing a player cannot order")
{
	CHECK(Mission_Is_Player_Order(MISSION_NONE));
	CHECK(Mission_Is_Player_Order(MISSION_QMOVE));
	CHECK_MESSAGE(!Mission_Is_Player_Order(MISSION_MISSILE), "a missile launch");
	CHECK_MESSAGE(!Mission_Is_Player_Order(MISSION_DECONSTRUCTION), "deconstruction");
	CHECK_MESSAGE(!Mission_Is_Player_Order(MISSION_COUNT), "a mission past the last one");
}


TEST_CASE("every compressed event decodes at its exact length and rejects one byte short")
{
	for (int index = 0; index < EventClass::LAST_EVENT; index++) {
		std::uint8_t const type = static_cast<std::uint8_t>(index);
		if (type == EventClass::FRAMEINFO || type == EventClass::FRAMESYNC) {
			continue;
		}
		INFO("event ", EventClass::EventNames[type]);

		Bytes packet = Valid_Compressed_Event(type);
		NetPacket::DecodeResult result = Decode(packet);
		CHECK(result.Succeeded());
		REQUIRE(result.Events.size() == 2u);
		CHECK(result.Events[1].Event.Type == type);

		std::size_t const variable_bytes = type == EventClass::MEGAMISSION ? 1 : 0;
		std::size_t const required = EventClass::EventLength[type] + variable_bytes;
		if (required == 0) {
			continue;
		}

		packet.pop_back();
		NetPacket::DecodeError expected = NetPacket::DecodeError::TRUNCATED_EVENT;
		if (type == EventClass::ADDPLAYER) {
			expected = NetPacket::DecodeError::TRUNCATED_ADDPLAYER;
		} else if (type == EventClass::MEGAMISSION) {
			expected = NetPacket::DecodeError::TRUNCATED_MEGAMISSION;
		}
		Check_Error(Decode(packet), expected);
	}
}


TEST_CASE("RESPONSE_TIME materializes its byte at FrameInfo.Delay")
{
	Bytes response = Compressed_Packet();
	std::byte const delay{42};
	Add_Compressed_Event(response, EventClass::RESPONSE_TIME, std::span<std::byte const>(&delay, 1));
	NetPacket::DecodeResult decoded = Decode(response);
	REQUIRE(decoded.Succeeded());
	REQUIRE(decoded.Events.size() == 2u);
	CHECK(decoded.Events[1].Event.Data.FrameInfo.Delay == 42);
}


TEST_CASE("NETWORK_REPORT preserves all three millisecond fields")
{
	Bytes report = Compressed_Packet();
	std::uint16_t const average = 17;
	std::uint16_t const worst = 240;
	std::uint16_t const stalled = 350;
	Bytes report_data;
	Append_Value(report_data, average);
	Append_Value(report_data, worst);
	Append_Value(report_data, stalled);
	Add_Compressed_Event(report, EventClass::NETWORK_REPORT, report_data);

	NetPacket::DecodeResult decoded = Decode(report);
	REQUIRE(decoded.Succeeded());
	REQUIRE(decoded.Events.size() == 2u);
	CHECK(decoded.Events[1].Event.Data.NetworkReport.AverageProcessMilliseconds == average);
	CHECK(decoded.Events[1].Event.Data.NetworkReport.WorstRoundTripMilliseconds == worst);
	CHECK(decoded.Events[1].Event.Data.NetworkReport.StallMilliseconds == stalled);
}


TEST_CASE("a MEGAMISSION for three units expands to three events that share all but their Whom field")
{
	std::size_t const data_size = EventClass::EventLength[EventClass::MEGAMISSION];
	Bytes mission(data_size, std::byte{0});
	std::array<int, 2> const whom1{1, 101};
	std::array<int, 2> const whom2{2, 202};
	std::array<int, 2> const whom3{3, 303};
	std::memcpy(mission.data(), whom1.data(), sizeof(whom1));
	std::uint32_t const marker = 0xA1B2C3D4;
	std::memcpy(mission.data() + MegaWhomSize, &marker, sizeof(marker));

	Bytes packet = Compressed_Packet();
	packet.push_back(static_cast<std::byte>(EventClass::MEGAMISSION));
	packet.push_back(std::byte{3});
	Append_Bytes(packet, mission);
	Append_Value(packet, whom2);
	Append_Value(packet, whom3);

	SUBCASE("the whole packet")
	{
		NetPacket::DecodeResult result = Decode(packet);
		REQUIRE(result.Succeeded());
		REQUIRE(result.Events.size() == 4u);
		CHECK_MESSAGE(std::memcmp(&result.Events[1].Event.Data.MegaMission.Whom, whom1.data(), sizeof(whom1)) == 0, "the first keeps its full record");
		CHECK_MESSAGE(std::memcmp(&result.Events[2].Event.Data.MegaMission.Whom, whom2.data(), sizeof(whom2)) == 0, "the second substitutes its Whom field");
		CHECK_MESSAGE(std::memcmp(&result.Events[3].Event.Data.MegaMission.Whom, whom3.data(), sizeof(whom3)) == 0, "the third substitutes its Whom field");
		CHECK_MESSAGE(std::memcmp(
				reinterpret_cast<std::byte const *>(&result.Events[2].Event.Data.MegaMission) + MegaWhomSize,
				mission.data() + MegaWhomSize,
				mission.size() - MegaWhomSize) == 0,
			"the repeated events inherit mission, target, and destination");
	}

	SUBCASE("cut short in its last repeat, which rejects the whole packet")
	{
		packet.pop_back();
		Check_Error(Decode(packet), NetPacket::DecodeError::TRUNCATED_MEGAMISSION);
	}
}


TEST_CASE("a zero MEGAMISSION count is rejected")
{
	Bytes zero = Compressed_Packet();
	zero.push_back(static_cast<std::byte>(EventClass::MEGAMISSION));
	zero.push_back(std::byte{0});
	zero.insert(zero.end(), EventClass::EventLength[EventClass::MEGAMISSION], std::byte{0});
	Check_Error(Decode(zero), NetPacket::DecodeError::ZERO_MEGAMISSION_COUNT);
}


TEST_CASE("a compressed ADDPLAYER owns and binds its variable data")
{
	Check_Add_Player(Decode(Compressed_Add_Player()));
}


TEST_CASE("copying a decoded packet rebinds ADDPLAYER to the copied bytes")
{
	NetPacket::DecodeResult result = Decode(Compressed_Add_Player());
	NetPacket::DecodeResult copied = result;
	Check_Add_Player(copied);
	REQUIRE(result.Events.size() == 2u);
	CHECK(copied.Events[1].Event.Data.Variable.Pointer != result.Events[1].Event.Data.Variable.Pointer);
}


TEST_CASE("a compressed ADDPLAYER rejects a payload shorter than its declared size")
{
	Bytes truncated = Compressed_Add_Player();
	truncated.pop_back();
	Check_Error(Decode(truncated), NetPacket::DecodeError::TRUNCATED_ADDPLAYER);
}


TEST_CASE("a late ADDPLAYER error discards every previously decoded event")
{
	Bytes transactional = Compressed_Packet();
	std::uint32_t const value = 7;
	Bytes value_data;
	Append_Value(value_data, value);
	Add_Compressed_Event(transactional, EventClass::GAMESPEED, value_data);
	transactional.push_back(static_cast<std::byte>(EventClass::ADDPLAYER));
	std::uint32_t const size = static_cast<std::uint32_t>(AddPlayerPayload.size());
	Append_Value(transactional, size);
	transactional.push_back(std::byte{0x11});
	Check_Error(Decode(transactional), NetPacket::DecodeError::TRUNCATED_ADDPLAYER);
}


TEST_CASE("an uncompressed ADDPLAYER owns and binds its variable data, and rejects a truncated payload")
{
	Bytes uncompressed = Uncompressed_Add_Player();
	Check_Add_Player(Decode(uncompressed, NetPacket::Encoding::UNCOMPRESSED));

	uncompressed.pop_back();
	Check_Error(Decode(uncompressed, NetPacket::Encoding::UNCOMPRESSED), NetPacket::DecodeError::TRUNCATED_ADDPLAYER);
}


TEST_CASE("two complete uncompressed events decode, each keeping its own fields, unexecuted")
{
	NetPacket::DecodeResult result = Decode(Two_Uncompressed_Events(), NetPacket::Encoding::UNCOMPRESSED);
	REQUIRE(result.Succeeded());
	REQUIRE(result.Events.size() == 2u);
	CHECK(result.Events[1].Event.Frame == PacketFrame + 6);
	CHECK(result.Events[1].Event.Data.General.Value == 5);
	CHECK_FALSE(result.Events[1].Event.IsExecuted);
}


TEST_CASE("every uncompressed event is bound to the demultiplexer sender")
{
	Bytes wrong_sender = Full_Event(EventClass::FRAMEINFO);
	Append_Bytes(wrong_sender, Full_Event(EventClass::GAMESPEED, Sender + 1));
	Check_Error(Decode(wrong_sender, NetPacket::Encoding::UNCOMPRESSED), NetPacket::DecodeError::SENDER_MISMATCH);
}


TEST_CASE("an uncompressed packet rejects a nested envelope, an unknown type, and a trailing partial record")
{
	SUBCASE("a nested envelope")
	{
		Bytes nested = Full_Event(EventClass::FRAMEINFO);
		Append_Bytes(nested, Full_Event(EventClass::FRAMEINFO));
		Check_Error(Decode(nested, NetPacket::Encoding::UNCOMPRESSED), NetPacket::DecodeError::NESTED_ENVELOPE);
	}

	SUBCASE("an unknown type, before the table lookup")
	{
		Bytes unknown = Full_Event(EventClass::FRAMEINFO);
		Append_Bytes(unknown, Full_Event(0xFF));
		Check_Error(Decode(unknown, NetPacket::Encoding::UNCOMPRESSED), NetPacket::DecodeError::INVALID_EVENT_TYPE);
	}

	SUBCASE("a trailing partial record")
	{
		Bytes trailing = Two_Uncompressed_Events();
		trailing.push_back(std::byte{0});
		Check_Error(Decode(trailing, NetPacket::Encoding::UNCOMPRESSED), NetPacket::DecodeError::TRAILING_BYTES);
	}
}


TEST_CASE("every datagram of the CRC alone or shorter is rejected")
{
	for (std::size_t size = 0; size <= sizeof(std::uint32_t); size++) {
		INFO("datagram of ", size, " bytes");
		Bytes short_datagram(size, std::byte{0});
		Check_Admission_Error(NetAdmission::Admit_Datagram(short_datagram).ErrorCode, NetAdmission::Error::DATAGRAM_TOO_SHORT);
	}
}


TEST_CASE("every legal datagram word and capacity boundary is accepted intact")
{
	for (std::size_t payload_size : {1u, 2u, 3u, 4u, 5u, 767u, 768u}) {
		INFO("payload of ", payload_size, " bytes");
		Bytes payload(payload_size, std::byte{0x5A});
		NetAdmission::DatagramResult const admission = NetAdmission::Admit_Datagram(Datagram(payload));
		CHECK(admission.Succeeded());
		CHECK(admission.Payload.size() == payload_size);
	}
}


TEST_CASE("a transport payload past the capacity is rejected rather than truncated")
{
	Bytes oversized_payload(NetAdmission::DATAGRAM_PAYLOAD_CAPACITY + 1, std::byte{0x33});
	Check_Admission_Error(NetAdmission::Admit_Datagram(Datagram(oversized_payload)).ErrorCode, NetAdmission::Error::DATAGRAM_TOO_LARGE);
}


TEST_CASE("a damaged transport payload fails CRC admission")
{
	Bytes damaged = Datagram(Bytes{std::byte{1}, std::byte{2}, std::byte{3}});
	damaged.back() ^= std::byte{0x80};
	Check_Admission_Error(NetAdmission::Admit_Datagram(damaged).ErrorCode, NetAdmission::Error::BAD_CRC);
}


TEST_CASE("an unaligned datagram is decoded with copied packed reads")
{
	Bytes aligned = Datagram(Bytes{std::byte{9}, std::byte{8}, std::byte{7}, std::byte{6}});
	Bytes unaligned(1, std::byte{0});
	Append_Bytes(unaligned, aligned);
	NetAdmission::DatagramResult const admitted = NetAdmission::Admit_Datagram(std::span<std::byte const>(unaligned).subspan(1));
	CHECK(admitted.Succeeded());
	CHECK(admitted.Payload.size() == 4u);
}


TEST_CASE("every incomplete private or global connection header is rejected")
{
	for (std::size_t size = 0; size < NetAdmission::PRIVATE_HEADER_SIZE; size++) {
		INFO("private header of ", size, " bytes");
		Bytes packet(size, std::byte{0});
		Check_Admission_Error(NetAdmission::Admit_Connection_Packet(packet, NetAdmission::PRIVATE_HEADER_SIZE, 64).ErrorCode, NetAdmission::Error::HEADER_TOO_SHORT);
	}
	for (std::size_t size = 0; size < NetAdmission::GLOBAL_HEADER_SIZE; size++) {
		INFO("global header of ", size, " bytes");
		Bytes packet(size, std::byte{0});
		Check_Admission_Error(NetAdmission::Admit_Connection_Packet(packet, NetAdmission::GLOBAL_HEADER_SIZE, 64).ErrorCode, NetAdmission::Error::HEADER_TOO_SHORT);
	}
}


TEST_CASE("a connection packet's length must fit its code, its connection and its destination")
{
	for (std::size_t header_size : {NetAdmission::PRIVATE_HEADER_SIZE, NetAdmission::GLOBAL_HEADER_SIZE}) {
		INFO("header of ", header_size, " bytes");

		Bytes ack = Connection_Packet(header_size, static_cast<std::uint8_t>(NetAdmission::PacketCode::ACK), 0);
		NetAdmission::ConnectionResult admitted = NetAdmission::Admit_Connection_Packet(ack, header_size, ack.size());
		CHECK_MESSAGE(admitted.Succeeded(), "an exact ACK header is admitted");
		CHECK(admitted.Magic == 0xCAFE);
		CHECK(admitted.PacketID == 0x12345678u);
		CHECK(admitted.Payload.empty());

		ack.push_back(std::byte{0});
		INFO("an ACK with application bytes is rejected");
		Check_Admission_Error(NetAdmission::Admit_Connection_Packet(ack, header_size, ack.size()).ErrorCode, NetAdmission::Error::INVALID_PACKET_LENGTH);

		Bytes empty_data = Connection_Packet(header_size, static_cast<std::uint8_t>(NetAdmission::PacketCode::DATA_ACK), 0);
		INFO("a data header without application payload is rejected");
		Check_Admission_Error(NetAdmission::Admit_Connection_Packet(empty_data, header_size, empty_data.size()).ErrorCode, NetAdmission::Error::INVALID_PACKET_LENGTH);

		Bytes data = Connection_Packet(header_size, static_cast<std::uint8_t>(NetAdmission::PacketCode::DATA_NOACK), 1);
		admitted = NetAdmission::Admit_Connection_Packet(data, header_size, data.size());
		CHECK_MESSAGE(admitted.Succeeded(), "the minimum one-byte application payload is accepted");
		CHECK(admitted.Payload.size() == 1u);
		INFO("a destination overflow is rejected before copying, and an exact-capacity one accepts the payload");
		Check_Admission_Error(NetAdmission::Validate_Destination(admitted.Payload, 0), NetAdmission::Error::DESTINATION_TOO_SMALL);
		Check_Admission_Error(NetAdmission::Validate_Destination(admitted.Payload, 1), NetAdmission::Error::NONE);

		INFO("a message above its connection capacity is rejected");
		Check_Admission_Error(NetAdmission::Admit_Connection_Packet(data, header_size, data.size() - 1).ErrorCode, NetAdmission::Error::PACKET_TOO_LARGE);
	}
}


TEST_CASE("a packet code outside DATA and ACK is rejected")
{
	Bytes invalid_code = Connection_Packet(NetAdmission::PRIVATE_HEADER_SIZE, static_cast<std::uint8_t>(NetAdmission::PacketCode::COUNT), 1);
	Check_Admission_Error(NetAdmission::Admit_Connection_Packet(invalid_code, NetAdmission::PRIVATE_HEADER_SIZE, invalid_code.size()).ErrorCode,
		NetAdmission::Error::INVALID_PACKET_CODE);
}


TEST_CASE("an unaligned reliable-message header is decoded with memcpy")
{
	Bytes aligned = Connection_Packet(NetAdmission::PRIVATE_HEADER_SIZE, static_cast<std::uint8_t>(NetAdmission::PacketCode::DATA_ACK), 1);
	Bytes unaligned(1, std::byte{0});
	Append_Bytes(unaligned, aligned);
	NetAdmission::ConnectionResult const admitted = NetAdmission::Admit_Connection_Packet(
		std::span<std::byte const>(unaligned).subspan(1), NetAdmission::PRIVATE_HEADER_SIZE, aligned.size());
	CHECK(admitted.Succeeded());
	CHECK(admitted.PacketID == 0x12345678u);
}


TEST_CASE("global packet initialization overwrites poison across the current packet shape")
{
	GlobalPacketType poisoned;
	std::memset(&poisoned, 0xA5, sizeof(poisoned));
	NetGlobal::Initialize_Packet(poisoned, NET_PROPOSE_KICK);
	NetCommandType const initialized_command = NET_PROPOSE_KICK;
	std::byte const * initialized_bytes = reinterpret_cast<std::byte const *>(&poisoned);
	std::byte const * command_bytes = reinterpret_cast<std::byte const *>(&initialized_command);
	for (std::size_t index = 0; index < sizeof(poisoned); index++) {
		INFO("byte ", index);
		std::byte const expected = index < sizeof(initialized_command) ? command_bytes[index] : std::byte{0};
		CHECK(std::to_integer<int>(initialized_bytes[index]) == std::to_integer<int>(expected));
	}
}


TEST_CASE("a sender is resolved by exact endpoint first, then by one zero-port entry on its IP")
{
	std::array<NetGlobal::Endpoint, 3> endpoints{{{0x01020304, 1000}, {0x01020304, 2000}, {0x01020304, 0}}};

	NetGlobal::EndpointResolution resolution = NetGlobal::Resolve_Sender({0x01020304, 2000}, endpoints);
	CHECK(resolution.Error == NetGlobal::DecodeError::NONE);
	CHECK(resolution.Match == NetGlobal::EndpointMatch::EXACT);
	CHECK_MESSAGE(resolution.RosterIndex == 1, "an exact IP and port selects the matching same-NAT player");

	resolution = NetGlobal::Resolve_Sender({0x01020304, 3000}, endpoints);
	CHECK(resolution.Error == NetGlobal::DecodeError::NONE);
	CHECK(resolution.Match == NetGlobal::EndpointMatch::ZERO_PORT);
	CHECK_MESSAGE(resolution.RosterIndex == 2, "one zero-port roster entry provides the legacy same-IP fallback");

	CHECK_MESSAGE(NetGlobal::Resolve_Sender({0x05060708, 1000}, endpoints).Error == NetGlobal::DecodeError::SENDER_NOT_MEMBER,
		"an unknown endpoint remains outside the session roster");
}


TEST_CASE("duplicate exact endpoints, or several zero-port entries on one IP, are ambiguous")
{
	std::array<NetGlobal::Endpoint, 2> duplicate_exact{{{0x01020304, 1000}, {0x01020304, 1000}}};
	CHECK(NetGlobal::Resolve_Sender({0x01020304, 1000}, duplicate_exact).Error == NetGlobal::DecodeError::AMBIGUOUS_SENDER);
	std::array<NetGlobal::Endpoint, 2> duplicate_wildcard{{{0x01020304, 0}, {0x01020304, 0}}};
	CHECK(NetGlobal::Resolve_Sender({0x01020304, 1000}, duplicate_wildcard).Error == NetGlobal::DecodeError::AMBIGUOUS_SENDER);
}


TEST_CASE("a global packet of the wrong length is rejected before dispatch")
{
	NetGlobal::ValidationContext outsider;
	GlobalPacketType packet = Global_Packet(NET_QUERY_GAME);
	Check_Global_Error(packet, GlobalPacketSize - 1, outsider, NetGlobal::DecodeError::INVALID_LENGTH);
	Check_Global_Error(packet, GlobalPacketSize + 1, outsider, NetGlobal::DecodeError::INVALID_LENGTH);
}


TEST_CASE("game and player discovery remain public during a match, with a terminated game name")
{
	NetGlobal::ValidationContext outsider;
	Check_Global_Error(Global_Packet(NET_QUERY_GAME), outsider, NetGlobal::DecodeError::NONE);

	GlobalPacketType packet = Global_Packet(NET_QUERY_PLAYER);
	Check_Global_Error(packet, outsider, NetGlobal::DecodeError::NONE);
	std::memset(packet.Name, 'x', sizeof(packet.Name));
	Check_Global_Error(packet, outsider, NetGlobal::DecodeError::UNTERMINATED_NAME);
}


TEST_CASE("session-control commands reject a source outside Session.Players")
{
	NetGlobal::ValidationContext outsider;
	for (NetCommandType command : {NET_SIGN_OFF, NET_MESSAGE, NET_PROGRESS_REPORT, NET_READY_TO_GO, NET_PROPOSE_KICK, NET_MOVIE_SKIP}) {
		INFO("command ", (int)command);
		GlobalPacketType packet = Global_Packet(command);
		packet.Kick.KickeeID = 5;
		Check_Global_Error(packet, outsider, NetGlobal::DecodeError::SENDER_NOT_MEMBER);
	}
}


TEST_CASE("sign-off, ready and movie commands accept a matched session member")
{
	NetGlobal::ValidationContext member = Member_Context();
	for (NetCommandType command : {NET_SIGN_OFF, NET_READY_TO_GO, NET_MOVIE_SKIP}) {
		INFO("command ", (int)command);
		Check_Global_Error(Global_Packet(command), member, NetGlobal::DecodeError::NONE);
	}
}


TEST_CASE("the in-game callback rejects commands outside its explicit allowlist")
{
	Check_Global_Error(Global_Packet(static_cast<NetCommandType>(999)), Member_Context(), NetGlobal::DecodeError::INVALID_COMMAND);
}


TEST_CASE("chat rejects an unterminated name or message body")
{
	NetGlobal::ValidationContext member = Member_Context();
	GlobalPacketType packet = Global_Packet(NET_MESSAGE);
	std::memset(packet.Name, 'n', sizeof(packet.Name));
	INFO("the claimed name is checked before it is ignored");
	Check_Global_Error(packet, member, NetGlobal::DecodeError::UNTERMINATED_NAME);

	packet = Global_Packet(NET_MESSAGE);
	std::memset(packet.Message.Buf, 'm', sizeof(packet.Message.Buf));
	Check_Global_Error(packet, member, NetGlobal::DecodeError::UNTERMINATED_MESSAGE);
}


TEST_CASE("chat ignores the wire color and takes the matched member's, which must be a color the session holds")
{
	GlobalPacketType packet = Global_Packet(NET_MESSAGE);
	packet.Message.Color = 999;
	Check_Global_Error(packet, Member_Context(), NetGlobal::DecodeError::NONE);

	NetGlobal::ValidationContext bad_color = Member_Context();
	bad_color.SenderPlayerColor = MAX_MPLAYER_COLORS;
	Check_Global_Error(packet, bad_color, NetGlobal::DecodeError::INVALID_COLOR);

	NetGlobal::ValidationContext launched_color = Member_Context();
	launched_color.SenderPlayerColor = 36;
	launched_color.ColorCount = 40;
	INFO("a launch file's [Colors] position");
	Check_Global_Error(packet, launched_color, NetGlobal::DecodeError::NONE);
}


TEST_CASE("progress accepts 0 to 100 percent only")
{
	NetGlobal::ValidationContext member = Member_Context();
	GlobalPacketType packet = Global_Packet(NET_PROGRESS_REPORT);
	packet.Progress.Percent = -1;
	Check_Global_Error(packet, member, NetGlobal::DecodeError::INVALID_PROGRESS);
	packet.Progress.Percent = 101;
	Check_Global_Error(packet, member, NetGlobal::DecodeError::INVALID_PROGRESS);
	packet.Progress.Percent = 100;
	Check_Global_Error(packet, member, NetGlobal::DecodeError::NONE);
}


TEST_CASE("a kick vote is the matched member's, and must name another current member")
{
	NetGlobal::ValidationContext member = Member_Context();
	GlobalPacketType packet = Global_Packet(NET_PROPOSE_KICK);
	packet.Kick.KickerID = UINT32_MAX;
	packet.Kick.KickeeID = 5;
	INFO("the claimed voter is ignored");
	Check_Global_Error(packet, member, NetGlobal::DecodeError::NONE);
	packet.Kick.KickeeID = 2;
	Check_Global_Error(packet, member, NetGlobal::DecodeError::SELF_KICK);
	packet.Kick.KickeeID = 7;
	Check_Global_Error(packet, member, NetGlobal::DecodeError::INVALID_KICK_PLAYER);
}


TEST_CASE("the global packet keeps its wire size")
{
	CHECK(sizeof(GlobalPacketType) == 1036u);
}


TEST_CASE("the out-of-sync and load commands reject a source outside Session.Players")
{
	NetGlobal::ValidationContext outsider;
	for (NetCommandType command : {NET_HOST_ANNOUNCE, NET_DESYNC_HEARTBEAT, NET_DESYNC_CONTINUE, NET_LOAD_GAME}) {
		INFO("command ", (int)command);
		GlobalPacketType packet = Global_Packet(command);
		packet.LoadGame.Slot = 0;
		Check_Global_Error(packet, outsider, NetGlobal::DecodeError::SENDER_NOT_MEMBER);
	}
}


TEST_CASE("any member may announce itself or send a heartbeat")
{
	NetGlobal::ValidationContext guest = Master_Context(5);
	INFO("adoption of an announced host is judged at dispatch");
	Check_Global_Error(Global_Packet(NET_HOST_ANNOUNCE), guest, NetGlobal::DecodeError::NONE);
	Check_Global_Error(Global_Packet(NET_DESYNC_HEARTBEAT), guest, NetGlobal::DecodeError::NONE);
}


TEST_CASE("only a known master may decide to continue")
{
	GlobalPacketType packet = Global_Packet(NET_DESYNC_CONTINUE);
	Check_Global_Error(packet, Master_Context(5), NetGlobal::DecodeError::SENDER_NOT_MASTER);
	INFO("a member with no known master");
	Check_Global_Error(packet, Member_Context(), NetGlobal::DecodeError::SENDER_NOT_MASTER);
	Check_Global_Error(packet, Master_Context(2), NetGlobal::DecodeError::NONE);
}


TEST_CASE("only the master may request a load, naming a slot the numbered saves can hold")
{
	GlobalPacketType packet = Global_Packet(NET_LOAD_GAME);
	packet.LoadGame.Slot = 7;
	Check_Global_Error(packet, Master_Context(5), NetGlobal::DecodeError::SENDER_NOT_MASTER);
	Check_Global_Error(packet, Master_Context(2), NetGlobal::DecodeError::NONE);
	packet.LoadGame.Slot = MULTIPLAYER_SAVE_SLOTS;
	Check_Global_Error(packet, Master_Context(2), NetGlobal::DecodeError::INVALID_SAVE_SLOT);
}


TEST_CASE("global rejections are counted per error and reported on each power of two")
{
	NetGlobal::RejectionCounters counters;
	NetGlobal::RejectionRecord first = counters.Record(NetGlobal::DecodeError::INVALID_LENGTH);
	NetGlobal::RejectionRecord second = counters.Record(NetGlobal::DecodeError::INVALID_LENGTH);
	NetGlobal::RejectionRecord third = counters.Record(NetGlobal::DecodeError::INVALID_LENGTH);
	NetGlobal::RejectionRecord fourth = counters.Record(NetGlobal::DecodeError::INVALID_LENGTH);
	CHECK(first.Count == 1u);
	CHECK(first.ShouldLog);
	CHECK(second.Count == 2u);
	CHECK(second.ShouldLog);
	CHECK(third.Count == 3u);
	CHECK_FALSE(third.ShouldLog);
	CHECK(fourth.Count == 4u);
	CHECK(fourth.ShouldLog);
	CHECK(counters.Count(NetGlobal::DecodeError::INVALID_LENGTH) == 4u);
	CHECK_MESSAGE(counters.Record(NetGlobal::DecodeError::NONE).Count == 0u, "successful packets do not enter the counters");
}
