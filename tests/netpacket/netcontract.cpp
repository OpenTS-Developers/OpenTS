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
#include "netsemantic.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <span>
#include <utility>
#include <vector>


namespace {

using Bytes = std::vector<std::byte>;
using VariableDataType = decltype(std::declval<EventClass>().Data.Variable);

constexpr int Sender = 3;
constexpr int Frame = 120;
constexpr std::size_t DataOffset = offsetof(EventClass, Data);
constexpr std::size_t FrameOffset = offsetof(EventClass, Frame);
constexpr std::size_t EnvelopeSize = DataOffset + sizeof(std::declval<EventClass>().Data.FrameInfo);
constexpr std::size_t FrameDelayOffset = DataOffset + offsetof(decltype(std::declval<EventClass>().Data.FrameInfo), Delay);
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
	NetPacket::DecodeResult const & result,
	NetPacket::DecodeError expected,
	char const * what)
{
	bool const matches = !result.Succeeded() && result.Failure.Code == expected && result.Events.empty();
	Check(matches, what);
	if (!matches) {
		std::printf("    got %s at %zu\n", NetPacket::Error_Name(result.Failure.Code), result.Failure.Offset);
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

	NetPacket::Reader reader(bytes);
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


void Test_Event_Contract(void)
{
	Check(EventClass::LATENCYFUDGE == 35, "the last inherited event keeps numeric ID 35");
	Check(EventClass::LAST_EVENT == 36, "the decoder preserves the inherited event range");
	Check(sizeof(EventClass) == 46 && EnvelopeSize == 17, "full and envelope event layouts match the legacy wire");
}


void Test_Envelope_Rules(void)
{
	Check_Error(
		NetPacket::Decode_Event_Packet({}, NetPacket::Encoding::COMPRESSED, Sender),
		NetPacket::DecodeError::EMPTY_PACKET,
		"an empty compressed packet is rejected");

	Bytes invalid_prefix{static_cast<std::byte>(EventClass::GAMESPEED)};
	Check_Error(
		NetPacket::Decode_Event_Packet(invalid_prefix, NetPacket::Encoding::COMPRESSED, Sender),
		NetPacket::DecodeError::INVALID_PREFIX,
		"a compressed packet must begin with FRAMEINFO or FRAMESYNC");

	Bytes unknown{static_cast<std::byte>(0xFF)};
	Check_Error(
		NetPacket::Decode_Event_Packet(unknown, NetPacket::Encoding::COMPRESSED, Sender),
		NetPacket::DecodeError::INVALID_EVENT_TYPE,
		"an unknown prefix type is rejected before table lookup");

	Bytes complete = Compressed_Packet();
	for (std::size_t size = 1; size < complete.size(); size++) {
		Bytes truncated(complete.begin(), complete.begin() + size);
		NetPacket::DecodeResult result = NetPacket::Decode_Event_Packet(truncated, NetPacket::Encoding::COMPRESSED, Sender);
		if (result.Failure.Code != NetPacket::DecodeError::TRUNCATED_ENVELOPE || !result.Events.empty()) {
			Check(false, "every incomplete compressed envelope is rejected transactionally");
			break;
		}
		if (size + 1 == complete.size()) {
			Check(true, "every incomplete compressed envelope is rejected transactionally");
		}
	}

	NetPacket::DecodeResult header = NetPacket::Decode_Event_Packet(complete, NetPacket::Encoding::COMPRESSED, Sender);
	Check(header.Succeeded() && header.Events.size() == 1, "a complete FRAMEINFO-only packet decodes");
	if (header.Succeeded() && header.Events.size() == 1) {
		Check(header.Events[0].Event.Type == EventClass::FRAMEINFO, "FRAMEINFO is retained for the execution queue");
		Check(header.Events[0].Event.ID == Sender && header.Events[0].Event.Frame == Frame,
			"FRAMEINFO carries the canonical sender and frame");
		Check(header.Events[0].Event.Data.FrameInfo.CRC == 0x12345678,
			"FRAMEINFO carries its complete data prefix");
	}

	Check_Error(
		NetPacket::Decode_Event_Packet(complete, NetPacket::Encoding::COMPRESSED, Sender + 1),
		NetPacket::DecodeError::SENDER_MISMATCH,
		"the envelope sender must match the demultiplexer sender");

	for (NetPacket::Encoding encoding : {NetPacket::Encoding::COMPRESSED, NetPacket::Encoding::UNCOMPRESSED}) {
		Bytes framesync = Envelope(EventClass::FRAMESYNC);
		NetPacket::DecodeResult result = NetPacket::Decode_Event_Packet(framesync, encoding, Sender);
		Check(result.Succeeded() && result.Events.empty() && result.HasEnvelope
			&& result.Envelope.Type == EventClass::FRAMESYNC,
			"a sole short FRAMESYNC is accepted, exposed, and not queued");

		framesync.push_back(std::byte{0});
		Check_Error(
			NetPacket::Decode_Event_Packet(framesync, encoding, Sender),
			NetPacket::DecodeError::FRAMESYNC_NOT_ALONE,
			"FRAMESYNC rejects every trailing byte");
	}

	Bytes nested = Compressed_Packet();
	Add_Compressed_Event(nested, EventClass::FRAMEINFO);
	Check_Error(
		NetPacket::Decode_Event_Packet(nested, NetPacket::Encoding::COMPRESSED, Sender),
		NetPacket::DecodeError::NESTED_ENVELOPE,
		"a compressed packet rejects a nested envelope");

	Bytes short_uncompressed = Envelope(EventClass::FRAMEINFO);
	Check_Error(
		NetPacket::Decode_Event_Packet(short_uncompressed, NetPacket::Encoding::UNCOMPRESSED, Sender),
		NetPacket::DecodeError::TRUNCATED_ENVELOPE,
		"an uncompressed FRAMEINFO must carry the complete full event");
}


void Test_Frame_Arithmetic(void)
{
	Bytes negative = Compressed_Packet();
	int const negative_frame = -1;
	Write_Value(negative, FrameOffset, negative_frame);
	Check_Error(NetPacket::Decode_Event_Packet(negative, NetPacket::Encoding::COMPRESSED, Sender),
		NetPacket::DecodeError::INVALID_FRAME_ARITHMETIC, "a negative FRAMEINFO frame is rejected transactionally");

	Bytes underflow = Compressed_Packet();
	int const early_frame = 3;
	std::uint8_t const excessive_delay = 4;
	Write_Value(underflow, FrameOffset, early_frame);
	Write_Value(underflow, FrameDelayOffset, excessive_delay);
	Check_Error(NetPacket::Decode_Event_Packet(underflow, NetPacket::Encoding::COMPRESSED, Sender),
		NetPacket::DecodeError::INVALID_FRAME_ARITHMETIC, "a FRAMEINFO delay larger than its frame is rejected");

	Bytes minimum = Envelope(EventClass::FRAMESYNC);
	int const minimum_frame = (std::numeric_limits<int>::min)();
	Write_Value(minimum, FrameOffset, minimum_frame);
	Check_Error(NetPacket::Decode_Event_Packet(minimum, NetPacket::Encoding::UNCOMPRESSED, Sender),
		NetPacket::DecodeError::INVALID_FRAME_ARITHMETIC, "the minimum signed FRAMESYNC frame cannot overflow subtraction");

	Check(NetPacket::Compute_Reported_Frame(4, 4) == 0, "a delay equal to its frame reports frame zero");
	Check(!NetPacket::Compute_Reported_Frame(3, 4), "checked sender-frame subtraction rejects underflow");
	Check(NetPacket::Compute_Reported_Frame(350, 0, 100, 250) == 350, "the maximum 250-frame sender lead is accepted");
	Check(!NetPacket::Compute_Reported_Frame(351, 0, 100, 250), "an excessive future sender frame is rejected");
	Check(NetPacket::Compute_Reported_Frame((std::numeric_limits<int>::max)(), 0,
		(std::numeric_limits<int>::max)(), 250).has_value(), "receiver lead arithmetic remains safe at the signed-frame limit");
}


void Test_Event_Semantics(void)
{
	Check(NetSemantic::Index_Is_Valid(0, 1), "index validation accepts the first collection entry");
	Check(!NetSemantic::Index_Is_Valid(-1, 1), "index validation rejects a negative entry");
	Check(!NetSemantic::Index_Is_Valid(1, 1), "index validation rejects the one-past entry");
	Check(NetSemantic::Subject_Owner_Is_Valid(3, 3), "a synchronized subject may be controlled by its owner");
	Check(!NetSemantic::Subject_Owner_Is_Valid(3, 4), "a synchronized subject rejects another player's identity");
	Check(NetSemantic::Game_Speed_Is_Valid(0) && NetSemantic::Game_Speed_Is_Valid(6), "game-speed validation accepts both legal edges");
	Check(!NetSemantic::Game_Speed_Is_Valid(-1) && !NetSemantic::Game_Speed_Is_Valid(7), "game-speed validation rejects both outside edges");
	Check(NetSemantic::Latency_Fudge_Is_Valid(0) && NetSemantic::Latency_Fudge_Is_Valid(3), "latency-margin validation accepts both legal edges");
	Check(!NetSemantic::Latency_Fudge_Is_Valid(-1) && !NetSemantic::Latency_Fudge_Is_Valid(4), "latency-margin validation rejects both outside edges");
	Check(NetSemantic::Animation_Type_Is_Valid(-1, -1, 5) && NetSemantic::Animation_Type_Is_Valid(4, -1, 5),
		"animation validation accepts its sentinel and last entry");
	Check(!NetSemantic::Animation_Type_Is_Valid(5, -1, 5), "animation validation rejects its one-past entry");
	Check(NetSemantic::Animation_Owner_Is_Valid(-1, -1, 5) && NetSemantic::Animation_Owner_Is_Valid(4, -1, 5),
		"animation-owner validation accepts its sentinel and last entry");
	Check(!NetSemantic::Animation_Owner_Is_Valid(5, -1, 5), "animation-owner validation rejects its one-past entry");
	Check(NetSemantic::Timing_Authority_Is_Valid(2, 2), "the resolved master may send timing events");
	Check(!NetSemantic::Timing_Authority_Is_Valid(3, 2), "a guest may not send timing events");
	Check(NetSemantic::Response_Time_Is_Valid(2, 2, 0, false), "legacy response time accepts its minimum");
	Check(NetSemantic::Response_Time_Is_Valid(255, 2, 0, false), "legacy response time preserves its byte range");
	Check(NetSemantic::Response_Time_Is_Valid(6, 2, 3, true), "compressed response time accepts two aligned periods");
	Check(!NetSemantic::Response_Time_Is_Valid(3, 2, 3, true), "compressed response time rejects one period");
	Check(!NetSemantic::Response_Time_Is_Valid(7, 2, 3, true), "compressed response time rejects misalignment");
	Check(!NetSemantic::Response_Time_Is_Valid(255, 2, 3, true), "compressed response time enforces the scheduling cap");
	Check(NetSemantic::Timing_Values_Are_Valid(60, 9, 3), "timing values accept a balanced setting");
	Check(!NetSemantic::Timing_Values_Are_Valid(0, 9, 3), "timing values reject zero desired FPS");
	Check(!NetSemantic::Timing_Values_Are_Valid(60, 10, 3), "timing values reject an unaligned horizon");
	Check(!NetSemantic::Timing_Values_Are_Valid(60, 251, 10), "timing values reject a horizon above the cap");
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
		NetPacket::DecodeResult result = NetPacket::Decode_Event_Packet(packet, NetPacket::Encoding::COMPRESSED, Sender);

		char label[96];
		std::snprintf(label, sizeof(label), "compressed event %-18s decodes at its exact length", EventClass::EventNames[type]);
		Check(result.Succeeded() && result.Events.size() == 2 && result.Events[1].Event.Type == type, label);

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

		std::snprintf(label, sizeof(label), "compressed event %-18s rejects one byte short", EventClass::EventNames[type]);
		Check_Error(NetPacket::Decode_Event_Packet(packet, NetPacket::Encoding::COMPRESSED, Sender), expected, label);
	}

	Bytes response = Compressed_Packet();
	std::byte const delay{42};
	Add_Compressed_Event(response, EventClass::RESPONSE_TIME, std::span<std::byte const>(&delay, 1));
	NetPacket::DecodeResult decoded_response = NetPacket::Decode_Event_Packet(response, NetPacket::Encoding::COMPRESSED, Sender);
	Check(decoded_response.Succeeded() && decoded_response.Events.size() == 2
		&& decoded_response.Events[1].Event.Data.FrameInfo.Delay == 42,
		"RESPONSE_TIME materializes its byte at FrameInfo.Delay");
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

	NetPacket::DecodeResult result = NetPacket::Decode_Event_Packet(packet, NetPacket::Encoding::COMPRESSED, Sender);
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
		NetPacket::Decode_Event_Packet(zero, NetPacket::Encoding::COMPRESSED, Sender),
		NetPacket::DecodeError::ZERO_MEGAMISSION_COUNT,
		"a zero MEGAMISSION count is rejected");

	packet.pop_back();
	Check_Error(
		NetPacket::Decode_Event_Packet(packet, NetPacket::Encoding::COMPRESSED, Sender),
		NetPacket::DecodeError::TRUNCATED_MEGAMISSION,
		"a truncated repeated MEGAMISSION rejects the whole packet");
}


void Check_Add_Player(NetPacket::DecodeResult const & result, char const * what)
{
	bool valid = result.Succeeded() && result.Events.size() == 2;
	if (valid) {
		NetPacket::DecodedEvent const & event = result.Events[1];
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

	NetPacket::DecodeResult result = NetPacket::Decode_Event_Packet(compressed, NetPacket::Encoding::COMPRESSED, Sender);
	Check_Add_Player(result, "compressed ADDPLAYER owns and binds its variable data");

	NetPacket::DecodeResult copied = result;
	Check_Add_Player(copied, "copying a decoded packet rebinds ADDPLAYER to the copied bytes");
	Check(copied.Events.size() == 2 && result.Events.size() == 2
		&& copied.Events[1].Event.Data.Variable.Pointer != result.Events[1].Event.Data.Variable.Pointer,
		"copied ADDPLAYER data does not point into the original result");

	Bytes truncated = compressed;
	truncated.pop_back();
	Check_Error(
		NetPacket::Decode_Event_Packet(truncated, NetPacket::Encoding::COMPRESSED, Sender),
		NetPacket::DecodeError::TRUNCATED_ADDPLAYER,
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
		NetPacket::Decode_Event_Packet(transactional, NetPacket::Encoding::COMPRESSED, Sender),
		NetPacket::DecodeError::TRUNCATED_ADDPLAYER,
		"a late ADDPLAYER error discards every previously decoded event");

	Bytes uncompressed = Full_Event(EventClass::FRAMEINFO);
	Bytes add = Full_Event(EventClass::ADDPLAYER);
	Write_Value(add, DataOffset + VariableSizeOffset, size);
	Append_Bytes(uncompressed, add);
	Append_Bytes(uncompressed, payload);
	Check_Add_Player(
		NetPacket::Decode_Event_Packet(uncompressed, NetPacket::Encoding::UNCOMPRESSED, Sender),
		"uncompressed ADDPLAYER owns and binds its variable data");

	uncompressed.pop_back();
	Check_Error(
		NetPacket::Decode_Event_Packet(uncompressed, NetPacket::Encoding::UNCOMPRESSED, Sender),
		NetPacket::DecodeError::TRUNCATED_ADDPLAYER,
		"uncompressed ADDPLAYER rejects a truncated owned payload");
}


void Test_Uncompressed(void)
{
	Bytes packet = Full_Event(EventClass::FRAMEINFO);
	Bytes speed = Full_Event(EventClass::GAMESPEED, Sender, Frame + 6);
	int const value = 5;
	Write_Value(speed, DataOffset, value);
	Append_Bytes(packet, speed);

	NetPacket::DecodeResult result = NetPacket::Decode_Event_Packet(packet, NetPacket::Encoding::UNCOMPRESSED, Sender);
	Check(result.Succeeded() && result.Events.size() == 2, "two complete uncompressed events decode transactionally");
	if (result.Succeeded() && result.Events.size() == 2) {
		Check(result.Events[1].Event.Frame == Frame + 6 && result.Events[1].Event.Data.General.Value == value,
			"an uncompressed event keeps its own common and data fields");
		Check(!result.Events[1].Event.IsExecuted, "received events are always materialized unexecuted");
	}

	Bytes wrong_sender = Full_Event(EventClass::FRAMEINFO);
	Append_Bytes(wrong_sender, Full_Event(EventClass::GAMESPEED, Sender + 1));
	Check_Error(
		NetPacket::Decode_Event_Packet(wrong_sender, NetPacket::Encoding::UNCOMPRESSED, Sender),
		NetPacket::DecodeError::SENDER_MISMATCH,
		"every uncompressed event is bound to the demultiplexer sender");

	Bytes nested = Full_Event(EventClass::FRAMEINFO);
	Append_Bytes(nested, Full_Event(EventClass::FRAMEINFO));
	Check_Error(
		NetPacket::Decode_Event_Packet(nested, NetPacket::Encoding::UNCOMPRESSED, Sender),
		NetPacket::DecodeError::NESTED_ENVELOPE,
		"an uncompressed packet rejects a nested envelope");

	Bytes unknown = Full_Event(EventClass::FRAMEINFO);
	Append_Bytes(unknown, Full_Event(0xFF));
	Check_Error(
		NetPacket::Decode_Event_Packet(unknown, NetPacket::Encoding::UNCOMPRESSED, Sender),
		NetPacket::DecodeError::INVALID_EVENT_TYPE,
		"an uncompressed unknown type is rejected before table lookup");

	Bytes trailing = packet;
	trailing.push_back(std::byte{0});
	Check_Error(
		NetPacket::Decode_Event_Packet(trailing, NetPacket::Encoding::UNCOMPRESSED, Sender),
		NetPacket::DecodeError::TRAILING_BYTES,
		"an uncompressed packet rejects a trailing partial record");
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


void Check_Admission_Error(NetAdmission::Error actual, NetAdmission::Error expected, char const * what)
{
	Check(actual == expected, what);
	if (actual != expected) {
		std::printf("    got %s\n", NetAdmission::Error_Name(actual));
	}
}


void Test_Datagram_Admission(void)
{
	for (std::size_t size = 0; size <= sizeof(std::uint32_t); size++) {
		Bytes short_datagram(size, std::byte{0});
		Check_Admission_Error(NetAdmission::Admit_Datagram(short_datagram).ErrorCode,
			NetAdmission::Error::DATAGRAM_TOO_SHORT,
			"every CRC-only or shorter datagram is rejected");
	}

	for (std::size_t payload_size : {1u, 2u, 3u, 4u, 5u, 767u, 768u}) {
		Bytes payload(payload_size, std::byte{0x5A});
		NetAdmission::DatagramResult const admission = NetAdmission::Admit_Datagram(Datagram(payload));
		Check(admission.Succeeded() && admission.Payload.size() == payload_size,
			"every legal datagram word/capacity boundary is accepted intact");
	}

	Bytes oversized_payload(NetAdmission::DATAGRAM_PAYLOAD_CAPACITY + 1, std::byte{0x33});
	Check_Admission_Error(NetAdmission::Admit_Datagram(Datagram(oversized_payload)).ErrorCode,
		NetAdmission::Error::DATAGRAM_TOO_LARGE,
		"a 769-byte transport payload is rejected rather than truncated");

	Bytes damaged = Datagram(Bytes{std::byte{1}, std::byte{2}, std::byte{3}});
	damaged.back() ^= std::byte{0x80};
	Check_Admission_Error(NetAdmission::Admit_Datagram(damaged).ErrorCode,
		NetAdmission::Error::BAD_CRC, "a damaged transport payload fails CRC admission");

	Bytes aligned = Datagram(Bytes{std::byte{9}, std::byte{8}, std::byte{7}, std::byte{6}});
	Bytes unaligned(1, std::byte{0});
	Append_Bytes(unaligned, aligned);
	NetAdmission::DatagramResult const admitted_unaligned = NetAdmission::Admit_Datagram(
		std::span<std::byte const>(unaligned).subspan(1));
	Check(admitted_unaligned.Succeeded() && admitted_unaligned.Payload.size() == 4,
		"an unaligned datagram is decoded with copied packed reads");
}


void Test_Connection_Admission(void)
{
	for (std::size_t size = 0; size < NetAdmission::PRIVATE_HEADER_SIZE; size++) {
		Bytes packet(size, std::byte{0});
		Check_Admission_Error(NetAdmission::Admit_Connection_Packet(
			packet, NetAdmission::PRIVATE_HEADER_SIZE, 64).ErrorCode,
			NetAdmission::Error::HEADER_TOO_SHORT,
			"every incomplete seven-byte private header is rejected");
	}
	for (std::size_t size = 0; size < NetAdmission::GLOBAL_HEADER_SIZE; size++) {
		Bytes packet(size, std::byte{0});
		Check_Admission_Error(NetAdmission::Admit_Connection_Packet(
			packet, NetAdmission::GLOBAL_HEADER_SIZE, 64).ErrorCode,
			NetAdmission::Error::HEADER_TOO_SHORT,
			"every incomplete nine-byte global header is rejected");
	}

	for (std::size_t header_size : {NetAdmission::PRIVATE_HEADER_SIZE, NetAdmission::GLOBAL_HEADER_SIZE}) {
		Bytes ack = Connection_Packet(header_size,
			static_cast<std::uint8_t>(NetAdmission::PacketCode::ACK), 0);
		NetAdmission::ConnectionResult admitted = NetAdmission::Admit_Connection_Packet(ack, header_size, ack.size());
		Check(admitted.Succeeded() && admitted.Magic == 0xCAFE
			&& admitted.PacketID == 0x12345678 && admitted.Payload.empty(),
			"an exact private/global ACK header is admitted and decoded");

		ack.push_back(std::byte{0});
		Check_Admission_Error(NetAdmission::Admit_Connection_Packet(ack, header_size, ack.size()).ErrorCode,
			NetAdmission::Error::INVALID_PACKET_LENGTH,
			"an ACK with application bytes is rejected");

		Bytes empty_data = Connection_Packet(header_size,
			static_cast<std::uint8_t>(NetAdmission::PacketCode::DATA_ACK), 0);
		Check_Admission_Error(NetAdmission::Admit_Connection_Packet(
			empty_data, header_size, empty_data.size()).ErrorCode,
			NetAdmission::Error::INVALID_PACKET_LENGTH,
			"a data header without application payload is rejected");

		Bytes data = Connection_Packet(header_size,
			static_cast<std::uint8_t>(NetAdmission::PacketCode::DATA_NOACK), 1);
		admitted = NetAdmission::Admit_Connection_Packet(data, header_size, data.size());
		Check(admitted.Succeeded() && admitted.Payload.size() == 1,
			"the minimum one-byte application payload is accepted");
		Check_Admission_Error(NetAdmission::Validate_Destination(admitted.Payload, 0),
			NetAdmission::Error::DESTINATION_TOO_SMALL,
			"a destination overflow is rejected before copying");
		Check_Admission_Error(NetAdmission::Validate_Destination(admitted.Payload, 1),
			NetAdmission::Error::NONE,
			"an exact-capacity destination accepts the payload");

		Check_Admission_Error(NetAdmission::Admit_Connection_Packet(
			data, header_size, data.size() - 1).ErrorCode,
			NetAdmission::Error::PACKET_TOO_LARGE,
			"a message above its connection capacity is rejected");
	}

	Bytes invalid_code = Connection_Packet(NetAdmission::PRIVATE_HEADER_SIZE,
		static_cast<std::uint8_t>(NetAdmission::PacketCode::COUNT), 1);
	Check_Admission_Error(NetAdmission::Admit_Connection_Packet(
		invalid_code, NetAdmission::PRIVATE_HEADER_SIZE, invalid_code.size()).ErrorCode,
		NetAdmission::Error::INVALID_PACKET_CODE,
		"a packet code outside DATA/ACK is rejected");

	Bytes aligned = Connection_Packet(NetAdmission::PRIVATE_HEADER_SIZE,
		static_cast<std::uint8_t>(NetAdmission::PacketCode::DATA_ACK), 1);
	Bytes unaligned(1, std::byte{0});
	Append_Bytes(unaligned, aligned);
	NetAdmission::ConnectionResult const admitted_unaligned = NetAdmission::Admit_Connection_Packet(
		std::span<std::byte const>(unaligned).subspan(1), NetAdmission::PRIVATE_HEADER_SIZE, aligned.size());
	Check(admitted_unaligned.Succeeded() && admitted_unaligned.PacketID == 0x12345678,
		"an unaligned reliable-message header is decoded with memcpy");
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


void Check_Global_Error(
	GlobalPacketType const & packet,
	std::size_t length,
	NetGlobal::ValidationContext const & context,
	NetGlobal::DecodeError expected,
	char const * what)
{
	NetGlobal::DecodeError const actual = NetGlobal::Validate_In_Game_Packet(packet, length, context);
	Check(actual == expected, what);
	if (actual != expected) {
		std::printf("    got %s\n", NetGlobal::Error_Name(actual));
	}
}


void Test_Global_Packets(void)
{
	constexpr std::size_t packet_size = sizeof(GlobalPacketType);
	NetGlobal::ValidationContext member = Member_Context();
	NetGlobal::ValidationContext outsider;
	GlobalPacketType packet = Global_Packet(NET_QUERY_GAME);
	GlobalPacketType poisoned;
	std::memset(&poisoned, 0xA5, sizeof(poisoned));
	NetGlobal::Initialize_Packet(poisoned, NET_PROPOSE_KICK);
	NetCommandType const initialized_command = NET_PROPOSE_KICK;
	std::byte const * initialized_bytes = reinterpret_cast<std::byte const *>(&poisoned);
	std::byte const * command_bytes = reinterpret_cast<std::byte const *>(&initialized_command);
	bool fully_initialized = true;
	for (std::size_t index = 0; index < sizeof(poisoned); index++) {
		std::byte const expected = index < sizeof(initialized_command)
			? command_bytes[index] : std::byte{0};
		fully_initialized = fully_initialized && initialized_bytes[index] == expected;
	}

	Check(fully_initialized,
		"global packet initialization overwrites poison across the current packet shape");

	std::array<NetGlobal::Endpoint, 3> endpoints{{{0x01020304, 1000}, {0x01020304, 2000}, {0x01020304, 0}}};
	NetGlobal::EndpointResolution resolution = NetGlobal::Resolve_Sender({0x01020304, 2000}, endpoints);
	Check(resolution.Error == NetGlobal::DecodeError::NONE && resolution.Match == NetGlobal::EndpointMatch::EXACT && resolution.RosterIndex == 1,
		"an exact IP and port selects the matching same-NAT player");
	resolution = NetGlobal::Resolve_Sender({0x01020304, 3000}, endpoints);
	Check(resolution.Error == NetGlobal::DecodeError::NONE && resolution.Match == NetGlobal::EndpointMatch::ZERO_PORT && resolution.RosterIndex == 2,
		"one zero-port roster entry provides the legacy same-IP fallback");
	std::array<NetGlobal::Endpoint, 2> duplicate_exact{{{0x01020304, 1000}, {0x01020304, 1000}}};
	Check(NetGlobal::Resolve_Sender({0x01020304, 1000}, duplicate_exact).Error == NetGlobal::DecodeError::AMBIGUOUS_SENDER,
		"duplicate exact endpoints are rejected as ambiguous");
	std::array<NetGlobal::Endpoint, 2> duplicate_wildcard{{{0x01020304, 0}, {0x01020304, 0}}};
	Check(NetGlobal::Resolve_Sender({0x01020304, 1000}, duplicate_wildcard).Error == NetGlobal::DecodeError::AMBIGUOUS_SENDER,
		"multiple zero-port candidates on one IP are rejected as ambiguous");
	Check(NetGlobal::Resolve_Sender({0x05060708, 1000}, endpoints).Error == NetGlobal::DecodeError::SENDER_NOT_MEMBER,
		"an unknown endpoint remains outside the session roster");
	Check_Global_Error(packet, packet_size - 1, outsider, NetGlobal::DecodeError::INVALID_LENGTH,
		"a short global packet is rejected before dispatch");
	Check_Global_Error(packet, packet_size + 1, outsider, NetGlobal::DecodeError::INVALID_LENGTH,
		"an oversized global packet is rejected before dispatch");
	Check_Global_Error(packet, packet_size, outsider, NetGlobal::DecodeError::NONE,
		"game discovery remains public during a match");

	packet = Global_Packet(NET_QUERY_PLAYER);
	Check_Global_Error(packet, packet_size, outsider, NetGlobal::DecodeError::NONE,
		"player discovery remains public during a match");
	std::memset(packet.Name, 'x', sizeof(packet.Name));
	Check_Global_Error(packet, packet_size, outsider, NetGlobal::DecodeError::UNTERMINATED_NAME,
		"player discovery requires a terminated game name");

	for (NetCommandType command : {
		NET_SIGN_OFF, NET_MESSAGE, NET_PROGRESS_REPORT, NET_READY_TO_GO, NET_PROPOSE_KICK}) {
		packet = Global_Packet(command);
		packet.Kick.KickeeID = 5;
		Check_Global_Error(packet, packet_size, outsider, NetGlobal::DecodeError::SENDER_NOT_MEMBER,
			"session-control commands reject a source outside Session.Players");
	}
	for (NetCommandType command : {NET_SIGN_OFF, NET_READY_TO_GO}) {
		packet = Global_Packet(command);
		Check_Global_Error(packet, packet_size, member, NetGlobal::DecodeError::NONE,
			"sign-off and ready commands accept a matched session member");
	}

	packet = Global_Packet(static_cast<NetCommandType>(999));
	Check_Global_Error(packet, packet_size, member, NetGlobal::DecodeError::INVALID_COMMAND,
		"the in-game callback rejects commands outside its explicit allowlist");

	packet = Global_Packet(NET_MESSAGE);
	std::memset(packet.Name, 'n', sizeof(packet.Name));
	Check_Global_Error(packet, packet_size, member, NetGlobal::DecodeError::UNTERMINATED_NAME,
		"chat rejects an unterminated claimed name before ignoring it");
	packet = Global_Packet(NET_MESSAGE);
	std::memset(packet.Message.Buf, 'm', sizeof(packet.Message.Buf));
	Check_Global_Error(packet, packet_size, member, NetGlobal::DecodeError::UNTERMINATED_MESSAGE,
		"chat rejects an unterminated message body");
	packet = Global_Packet(NET_MESSAGE);
	packet.Message.Color = 999;
	Check_Global_Error(packet, packet_size, member, NetGlobal::DecodeError::NONE,
		"chat ignores the wire color in favor of the matched member's color");
	NetGlobal::ValidationContext bad_color = member;
	bad_color.SenderPlayerColor = MAX_MPLAYER_COLORS;
	Check_Global_Error(packet, packet_size, bad_color, NetGlobal::DecodeError::INVALID_COLOR,
		"chat refuses an invalid canonical session color");

	packet = Global_Packet(NET_PROGRESS_REPORT);
	packet.Progress.Percent = -1;
	Check_Global_Error(packet, packet_size, member, NetGlobal::DecodeError::INVALID_PROGRESS,
		"progress rejects a negative percentage");
	packet.Progress.Percent = 101;
	Check_Global_Error(packet, packet_size, member, NetGlobal::DecodeError::INVALID_PROGRESS,
		"progress rejects a percentage above 100");
	packet.Progress.Percent = 100;
	Check_Global_Error(packet, packet_size, member, NetGlobal::DecodeError::NONE,
		"progress preserves the legal 100-percent edge");

	packet = Global_Packet(NET_PROPOSE_KICK);
	packet.Kick.KickerID = UINT32_MAX;
	packet.Kick.KickeeID = 5;
	Check_Global_Error(packet, packet_size, member, NetGlobal::DecodeError::NONE,
		"kick validation ignores the claimed voter and uses the matched member");
	packet.Kick.KickeeID = 2;
	Check_Global_Error(packet, packet_size, member, NetGlobal::DecodeError::SELF_KICK,
		"a member cannot vote to kick itself");
	packet.Kick.KickeeID = 7;
	Check_Global_Error(packet, packet_size, member, NetGlobal::DecodeError::INVALID_KICK_PLAYER,
		"a kick target must be a current session member");

	NetGlobal::RejectionCounters counters;
	NetGlobal::RejectionRecord first = counters.Record(NetGlobal::DecodeError::INVALID_LENGTH);
	NetGlobal::RejectionRecord second = counters.Record(NetGlobal::DecodeError::INVALID_LENGTH);
	NetGlobal::RejectionRecord third = counters.Record(NetGlobal::DecodeError::INVALID_LENGTH);
	NetGlobal::RejectionRecord fourth = counters.Record(NetGlobal::DecodeError::INVALID_LENGTH);
	Check(first.Count == 1 && first.ShouldLog, "the first global rejection is reported");
	Check(second.Count == 2 && second.ShouldLog, "the second global rejection is reported");
	Check(third.Count == 3 && !third.ShouldLog, "non-power-of-two global rejections stay quiet");
	Check(fourth.Count == 4 && fourth.ShouldLog, "power-of-two global rejections are reported");
	Check(counters.Count(NetGlobal::DecodeError::INVALID_LENGTH) == 4,
		"global rejection counters retain a stable per-error total");
	Check(counters.Record(NetGlobal::DecodeError::NONE).Count == 0,
		"successful packets do not enter rejection counters");
}

}	// namespace


int main(void)
{
	Test_Reader();
	Test_Event_Contract();
	Test_Envelope_Rules();
	Test_Frame_Arithmetic();
	Test_Event_Semantics();
	Test_Full_Compressed_Table();
	Test_Mega_Mission();
	Test_Add_Player();
	Test_Uncompressed();
	Test_Datagram_Admission();
	Test_Connection_Admission();
	Test_Global_Packets();

	std::printf("\n%s\n", Failures == 0 ? "All checks passed." : "Some checks FAILED.");
	return(Failures == 0 ? 0 : 1);
}
