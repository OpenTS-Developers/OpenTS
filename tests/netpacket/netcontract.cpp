/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Exercises the network event contract without starting the engine or loading game data.

#include "netpacket.h"
#include "netreader.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <span>
#include <utility>
#include <vector>


namespace {

using Bytes = std::vector<std::byte>;
using VariableDataType = decltype(std::declval<EventClass>().Data.Variable);

constexpr int Sender = 3;
constexpr int Frame = 120;
constexpr std::size_t DataOffset = offsetof(EventClass, Data);
constexpr std::size_t EnvelopeSize = DataOffset + sizeof(std::declval<EventClass>().Data.FrameInfo);
constexpr std::size_t VariableSizeOffset = offsetof(VariableDataType, Size);
constexpr std::size_t MegaWhomSize = sizeof(std::declval<EventClass>().Data.MegaMission.Whom);

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-72s %s\n", what, condition ? "ok" : "FAILED");
	if (!condition) {
		Failures++;
	}
}


void Check_Error(
	NetPacketDecodeResult const & result,
	NetPacketDecodeError expected,
	char const * what)
{
	bool const matches = !result.Succeeded() && result.Failure.Code == expected && result.Events.empty();
	Check(matches, what);
	if (!matches) {
		std::printf("    got %s at %zu\n", Net_Packet_Error_Name(result.Failure.Code), result.Failure.Offset);
	}
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


Bytes Full_Event(std::uint8_t type, int sender = Sender, int frame = Frame)
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


void Test_Reader(void)
{
	Bytes bytes;
	std::uint16_t const first = 0x1234;
	std::uint32_t const second = 0x89ABCDEF;
	Append_Value(bytes, first);
	Append_Value(bytes, second);

	NetReaderClass reader(bytes);
	auto got_first = reader.Read_Value<std::uint16_t>();
	Check(got_first && *got_first == first, "reader copies a fixed-width value");
	Check(reader.Offset() == sizeof(first), "reader reports its consumed offset");

	std::size_t const before_failure = reader.Offset();
	Check(!reader.Take(bytes.size()), "reader refuses a span larger than the remainder");
	Check(reader.Offset() == before_failure, "a failed read does not advance the cursor");

	auto got_second = reader.Read_Value<std::uint32_t>();
	Check(got_second && *got_second == second, "reader resumes after a failed read");
	Check(reader.Empty(), "reader reports an exhausted packet");
	Check(reader.Take(0).has_value(), "reader can take an empty span at the end");
}


void Test_Layout(void)
{
	Check(EventClass::LATENCYFUDGE == 35, "the last inherited event keeps numeric ID 35");
	Check(EventClass::NETWORK_REPORT == 36, "NETWORK_REPORT is appended as numeric ID 36");
	Check(EventClass::LAST_EVENT == 37, "LAST_EVENT advances without renumbering old events");
	Check(EventClass::EventLength[EventClass::NETWORK_REPORT] == 4, "NETWORK_REPORT has a four-byte wire payload");
	Check(std::strcmp(EventClass::EventNames[EventClass::NETWORK_REPORT], "NETWORK_REPORT") == 0,
		"NETWORK_REPORT has a diagnostic name");
	Check(EventClass::NETWORK_RTT_UNAVAILABLE == UINT16_MAX, "the unavailable RTT sentinel is uint16 max");
	Check(sizeof(EventClass) == 46 && EnvelopeSize == 17, "full and envelope event layouts match the legacy wire");
}


void Test_Envelope_Rules(void)
{
	Check_Error(
		Decode_Event_Packet({}, NetPacketEncoding::COMPRESSED, Sender),
		NetPacketDecodeError::EMPTY_PACKET,
		"an empty compressed packet is rejected");

	Bytes invalid_prefix{static_cast<std::byte>(EventClass::GAMESPEED)};
	Check_Error(
		Decode_Event_Packet(invalid_prefix, NetPacketEncoding::COMPRESSED, Sender),
		NetPacketDecodeError::INVALID_PREFIX,
		"a compressed packet must begin with FRAMEINFO or FRAMESYNC");

	Bytes unknown{static_cast<std::byte>(0xFF)};
	Check_Error(
		Decode_Event_Packet(unknown, NetPacketEncoding::COMPRESSED, Sender),
		NetPacketDecodeError::INVALID_EVENT_TYPE,
		"an unknown prefix type is rejected before table lookup");

	Bytes complete = Compressed_Packet();
	for (std::size_t size = 1; size < complete.size(); size++) {
		Bytes truncated(complete.begin(), complete.begin() + size);
		NetPacketDecodeResult result = Decode_Event_Packet(truncated, NetPacketEncoding::COMPRESSED, Sender);
		if (result.Failure.Code != NetPacketDecodeError::TRUNCATED_ENVELOPE || !result.Events.empty()) {
			Check(false, "every incomplete compressed envelope is rejected transactionally");
			break;
		}
		if (size + 1 == complete.size()) {
			Check(true, "every incomplete compressed envelope is rejected transactionally");
		}
	}

	NetPacketDecodeResult header = Decode_Event_Packet(complete, NetPacketEncoding::COMPRESSED, Sender);
	Check(header.Succeeded() && header.Events.size() == 1, "a complete FRAMEINFO-only packet decodes");
	if (header.Succeeded() && header.Events.size() == 1) {
		Check(header.Events[0].Event.Type == EventClass::FRAMEINFO, "FRAMEINFO is retained for the execution queue");
		Check(header.Events[0].Event.ID == Sender && header.Events[0].Event.Frame == Frame,
			"FRAMEINFO carries the canonical sender and frame");
		Check(header.Events[0].Event.Data.FrameInfo.CRC == 0x12345678,
			"FRAMEINFO carries its complete data prefix");
	}

	Check_Error(
		Decode_Event_Packet(complete, NetPacketEncoding::COMPRESSED, Sender + 1),
		NetPacketDecodeError::SENDER_MISMATCH,
		"the envelope sender must match the demultiplexer sender");

	for (NetPacketEncoding encoding : {NetPacketEncoding::COMPRESSED, NetPacketEncoding::UNCOMPRESSED}) {
		Bytes framesync = Envelope(EventClass::FRAMESYNC);
		NetPacketDecodeResult result = Decode_Event_Packet(framesync, encoding, Sender);
		Check(result.Succeeded() && result.Events.empty() && result.HasEnvelope
			&& result.Envelope.Type == EventClass::FRAMESYNC,
			"a sole short FRAMESYNC is accepted, exposed, and not queued");

		framesync.push_back(std::byte{0});
		Check_Error(
			Decode_Event_Packet(framesync, encoding, Sender),
			NetPacketDecodeError::FRAMESYNC_NOT_ALONE,
			"FRAMESYNC rejects every trailing byte");
	}

	Bytes nested = Compressed_Packet();
	Add_Compressed_Event(nested, EventClass::FRAMEINFO);
	Check_Error(
		Decode_Event_Packet(nested, NetPacketEncoding::COMPRESSED, Sender),
		NetPacketDecodeError::NESTED_ENVELOPE,
		"a compressed packet rejects a nested envelope");

	Bytes short_uncompressed = Envelope(EventClass::FRAMEINFO);
	Check_Error(
		Decode_Event_Packet(short_uncompressed, NetPacketEncoding::UNCOMPRESSED, Sender),
		NetPacketDecodeError::TRUNCATED_ENVELOPE,
		"an uncompressed FRAMEINFO must carry the complete full event");
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


void Test_Full_Compressed_Table(void)
{
	for (int index = 0; index < EventClass::LAST_EVENT; index++) {
		std::uint8_t const type = static_cast<std::uint8_t>(index);
		if (type == EventClass::FRAMEINFO || type == EventClass::FRAMESYNC) {
			continue;
		}

		Bytes packet = Valid_Compressed_Event(type);
		NetPacketDecodeResult result = Decode_Event_Packet(packet, NetPacketEncoding::COMPRESSED, Sender);

		char label[96];
		std::snprintf(label, sizeof(label), "compressed event %-18s decodes at its exact length", EventClass::EventNames[type]);
		Check(result.Succeeded() && result.Events.size() == 2 && result.Events[1].Event.Type == type, label);

		std::size_t const variable_bytes = type == EventClass::MEGAMISSION ? 1 : 0;
		std::size_t const required = EventClass::EventLength[type] + variable_bytes;
		if (required == 0) {
			continue;
		}

		packet.pop_back();
		NetPacketDecodeError expected = NetPacketDecodeError::TRUNCATED_EVENT;
		if (type == EventClass::ADDPLAYER) {
			expected = NetPacketDecodeError::TRUNCATED_ADDPLAYER;
		} else if (type == EventClass::MEGAMISSION) {
			expected = NetPacketDecodeError::TRUNCATED_MEGAMISSION;
		}

		std::snprintf(label, sizeof(label), "compressed event %-18s rejects one byte short", EventClass::EventNames[type]);
		Check_Error(Decode_Event_Packet(packet, NetPacketEncoding::COMPRESSED, Sender), expected, label);
	}

	Bytes response = Compressed_Packet();
	std::byte const delay{42};
	Add_Compressed_Event(response, EventClass::RESPONSE_TIME, std::span<std::byte const>(&delay, 1));
	NetPacketDecodeResult decoded_response = Decode_Event_Packet(response, NetPacketEncoding::COMPRESSED, Sender);
	Check(decoded_response.Succeeded() && decoded_response.Events.size() == 2
		&& decoded_response.Events[1].Event.Data.FrameInfo.Delay == 42,
		"RESPONSE_TIME materializes its byte at FrameInfo.Delay");

	Bytes report = Compressed_Packet();
	std::uint16_t const average = 17;
	std::uint16_t const worst = 240;
	Bytes report_data;
	Append_Value(report_data, average);
	Append_Value(report_data, worst);
	Add_Compressed_Event(report, EventClass::NETWORK_REPORT, report_data);
	NetPacketDecodeResult decoded_report = Decode_Event_Packet(report, NetPacketEncoding::COMPRESSED, Sender);
	Check(decoded_report.Succeeded() && decoded_report.Events.size() == 2
		&& decoded_report.Events[1].Event.Data.NetworkReport.AverageProcessMilliseconds == average
		&& decoded_report.Events[1].Event.Data.NetworkReport.WorstRoundTripMilliseconds == worst,
		"NETWORK_REPORT preserves both millisecond fields");
}


void Test_Mega_Mission(void)
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

	NetPacketDecodeResult result = Decode_Event_Packet(packet, NetPacketEncoding::COMPRESSED, Sender);
	Check(result.Succeeded() && result.Events.size() == 4, "a three-unit MEGAMISSION expands to three events");
	if (result.Succeeded() && result.Events.size() == 4) {
		Check(std::memcmp(&result.Events[1].Event.Data.MegaMission.Whom, whom1.data(), sizeof(whom1)) == 0,
			"the first MEGAMISSION keeps its full record");
		Check(std::memcmp(&result.Events[2].Event.Data.MegaMission.Whom, whom2.data(), sizeof(whom2)) == 0,
			"the second MEGAMISSION substitutes its Whom field");
		Check(std::memcmp(&result.Events[3].Event.Data.MegaMission.Whom, whom3.data(), sizeof(whom3)) == 0,
			"the third MEGAMISSION substitutes its Whom field");
		Check(std::memcmp(
				reinterpret_cast<std::byte const *>(&result.Events[2].Event.Data.MegaMission) + MegaWhomSize,
				mission.data() + MegaWhomSize,
				mission.size() - MegaWhomSize) == 0,
			"repeated MEGAMISSION events inherit mission, target, and destination");
	}

	Bytes zero = Compressed_Packet();
	zero.push_back(static_cast<std::byte>(EventClass::MEGAMISSION));
	zero.push_back(std::byte{0});
	zero.insert(zero.end(), data_size, std::byte{0});
	Check_Error(
		Decode_Event_Packet(zero, NetPacketEncoding::COMPRESSED, Sender),
		NetPacketDecodeError::ZERO_MEGAMISSION_COUNT,
		"a zero MEGAMISSION count is rejected");

	packet.pop_back();
	Check_Error(
		Decode_Event_Packet(packet, NetPacketEncoding::COMPRESSED, Sender),
		NetPacketDecodeError::TRUNCATED_MEGAMISSION,
		"a truncated repeated MEGAMISSION rejects the whole packet");
}


void Check_Add_Player(NetPacketDecodeResult const & result, char const * what)
{
	bool valid = result.Succeeded() && result.Events.size() == 2;
	if (valid) {
		NetDecodedEvent const & event = result.Events[1];
		valid = event.Event.Type == EventClass::ADDPLAYER
			&& event.Event.Data.Variable.Size == 3
			&& event.AddPlayerData.size() == 3
			&& event.Event.Data.Variable.Pointer == event.AddPlayerData.data()
			&& std::to_integer<int>(event.AddPlayerData[0]) == 0x11
			&& std::to_integer<int>(event.AddPlayerData[2]) == 0x33;
	}
	Check(valid, what);
}


void Test_Add_Player(void)
{
	Bytes payload{std::byte{0x11}, std::byte{0x22}, std::byte{0x33}};
	Bytes compressed = Compressed_Packet();
	compressed.push_back(static_cast<std::byte>(EventClass::ADDPLAYER));
	std::uint32_t const size = static_cast<std::uint32_t>(payload.size());
	Append_Value(compressed, size);
	Append_Bytes(compressed, payload);

	NetPacketDecodeResult result = Decode_Event_Packet(compressed, NetPacketEncoding::COMPRESSED, Sender);
	Check_Add_Player(result, "compressed ADDPLAYER owns and binds its variable data");

	NetPacketDecodeResult copied = result;
	Check_Add_Player(copied, "copying a decoded packet rebinds ADDPLAYER to the copied bytes");
	Check(copied.Events.size() == 2 && result.Events.size() == 2
		&& copied.Events[1].Event.Data.Variable.Pointer != result.Events[1].Event.Data.Variable.Pointer,
		"copied ADDPLAYER data does not point into the original result");

	Bytes truncated = compressed;
	truncated.pop_back();
	Check_Error(
		Decode_Event_Packet(truncated, NetPacketEncoding::COMPRESSED, Sender),
		NetPacketDecodeError::TRUNCATED_ADDPLAYER,
		"compressed ADDPLAYER rejects a payload shorter than its declared size");

	Bytes transactional = Compressed_Packet();
	std::uint32_t const value = 7;
	Bytes value_data;
	Append_Value(value_data, value);
	Add_Compressed_Event(transactional, EventClass::GAMESPEED, value_data);
	transactional.push_back(static_cast<std::byte>(EventClass::ADDPLAYER));
	Append_Value(transactional, size);
	transactional.push_back(std::byte{0x11});
	Check_Error(
		Decode_Event_Packet(transactional, NetPacketEncoding::COMPRESSED, Sender),
		NetPacketDecodeError::TRUNCATED_ADDPLAYER,
		"a late ADDPLAYER error discards every previously decoded event");

	Bytes uncompressed = Full_Event(EventClass::FRAMEINFO);
	Bytes add = Full_Event(EventClass::ADDPLAYER);
	Write_Value(add, DataOffset + VariableSizeOffset, size);
	Append_Bytes(uncompressed, add);
	Append_Bytes(uncompressed, payload);
	Check_Add_Player(
		Decode_Event_Packet(uncompressed, NetPacketEncoding::UNCOMPRESSED, Sender),
		"uncompressed ADDPLAYER owns and binds its variable data");

	uncompressed.pop_back();
	Check_Error(
		Decode_Event_Packet(uncompressed, NetPacketEncoding::UNCOMPRESSED, Sender),
		NetPacketDecodeError::TRUNCATED_ADDPLAYER,
		"uncompressed ADDPLAYER rejects a truncated owned payload");
}


void Test_Uncompressed(void)
{
	Bytes packet = Full_Event(EventClass::FRAMEINFO);
	Bytes speed = Full_Event(EventClass::GAMESPEED, Sender, Frame + 6);
	int const value = 5;
	Write_Value(speed, DataOffset, value);
	Append_Bytes(packet, speed);

	NetPacketDecodeResult result = Decode_Event_Packet(packet, NetPacketEncoding::UNCOMPRESSED, Sender);
	Check(result.Succeeded() && result.Events.size() == 2, "two complete uncompressed events decode transactionally");
	if (result.Succeeded() && result.Events.size() == 2) {
		Check(result.Events[1].Event.Frame == Frame + 6 && result.Events[1].Event.Data.General.Value == value,
			"an uncompressed event keeps its own common and data fields");
		Check(!result.Events[1].Event.IsExecuted, "received events are always materialized unexecuted");
	}

	Bytes wrong_sender = Full_Event(EventClass::FRAMEINFO);
	Append_Bytes(wrong_sender, Full_Event(EventClass::GAMESPEED, Sender + 1));
	Check_Error(
		Decode_Event_Packet(wrong_sender, NetPacketEncoding::UNCOMPRESSED, Sender),
		NetPacketDecodeError::SENDER_MISMATCH,
		"every uncompressed event is bound to the demultiplexer sender");

	Bytes nested = Full_Event(EventClass::FRAMEINFO);
	Append_Bytes(nested, Full_Event(EventClass::FRAMEINFO));
	Check_Error(
		Decode_Event_Packet(nested, NetPacketEncoding::UNCOMPRESSED, Sender),
		NetPacketDecodeError::NESTED_ENVELOPE,
		"an uncompressed packet rejects a nested envelope");

	Bytes unknown = Full_Event(EventClass::FRAMEINFO);
	Append_Bytes(unknown, Full_Event(0xFF));
	Check_Error(
		Decode_Event_Packet(unknown, NetPacketEncoding::UNCOMPRESSED, Sender),
		NetPacketDecodeError::INVALID_EVENT_TYPE,
		"an uncompressed unknown type is rejected before table lookup");

	Bytes trailing = packet;
	trailing.push_back(std::byte{0});
	Check_Error(
		Decode_Event_Packet(trailing, NetPacketEncoding::UNCOMPRESSED, Sender),
		NetPacketDecodeError::TRAILING_BYTES,
		"an uncompressed packet rejects a trailing partial record");
}

}	// namespace


int main(void)
{
	Test_Reader();
	Test_Layout();
	Test_Envelope_Rules();
	Test_Full_Compressed_Table();
	Test_Mega_Mission();
	Test_Add_Player();
	Test_Uncompressed();

	std::printf("\n%s\n", Failures == 0 ? "All checks passed." : "Some checks FAILED.");
	return(Failures == 0 ? 0 : 1);
}
