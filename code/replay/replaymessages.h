/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "proto/wire.h"

#include "replay/replay.hh"

#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// The messages of replay.proto as plain structs. Encode writes the canonical encoding, Decode
// accepts any valid one, and fields kept in Unknown are written back in field-number order.

namespace Replay
{
	using ProtoWire::Bytes;
	using ProtoWire::ByteSpan;
	using ProtoWire::WireType;
	using ProtoWire::MAX_FIELD_NUMBER;
	using ProtoWire::MAX_VARINT_SIZE;
	using ProtoWire::Zigzag32;
	using ProtoWire::Zigzag64;
	using ProtoWire::Unzigzag32;
	using ProtoWire::Unzigzag64;
	using ProtoWire::Varint_Size;
	using ProtoWire::Put_Varint;
	using ProtoWire::Put_Fixed32;
	using ProtoWire::Put_Fixed64;
	using ProtoWire::WireWriter;
	using ProtoWire::WireError;
	using ProtoWire::WireField;
	using ProtoWire::WireReader;
	using ProtoWire::Read_Varint;
	using ProtoWire::Read_Repeated_Uint32;

	using ProtoMessage::UnknownField;
	using ProtoMessage::UnknownFields;
	using ProtoMessage::RuleError;
	using ProtoMessage::Encoded;
}

// One struct per message, named as the schema names it, with the codec functions beside them
// so that argument-dependent lookup finds them.
namespace Replay::Message
{
	struct FileHash {
		std::string Path;
		std::string Hash;
		UnknownFields Unknown;
	};

	struct PlayerColor {
		std::uint32_t Number = 0;
		std::uint32_t Rgb = 0;
		UnknownFields Unknown;
	};

	struct IncomeKind {
		std::uint32_t Number = 0;
		std::string Id;
		UnknownFields Unknown;
	};

	struct RecordingInfo {
		std::string Engine;
		std::string EngineVersion;
		std::uint32_t DataVersion = 0;
		std::string Package;
		std::string PackageVersion;
		std::vector<FileHash> Files;
		std::uint64_t RecordedAt = 0;
		std::optional<std::uint32_t> Recorder;
		std::string MapName;
		std::string MapPath;
		Bytes MapSha256;
		std::string GameMode;
		std::uint32_t SealVersion = 0;
		std::vector<PlayerColor> Colors;
		std::vector<IncomeKind> IncomeKinds;
		UnknownFields Unknown;
	};

	struct Player {
		std::uint32_t House = 0;
		std::string Name;
		bool Human = false;
		bool Observer = false;
		std::int32_t Country = 0;
		std::string CountryName;
		std::int32_t Color = 0;
		std::int32_t Start = 0;
		std::optional<std::int32_t> AILevel;
		std::vector<std::uint32_t> Allies;
		std::optional<std::int32_t> ResolvedCountry;
		std::optional<std::int32_t> ResolvedColor;
		std::optional<std::int32_t> ResolvedStart;
		UnknownFields Unknown;
	};

	struct GameOption {
		std::string Key;
		std::variant<std::monostate, std::int64_t, bool, std::string> Value;
		UnknownFields Unknown;
	};

	struct Launch {
		GameKind Kind = GameKind::GAME_KIND_UNKNOWN;
		std::optional<std::uint32_t> Seed;
		std::vector<Player> Players;
		std::vector<GameOption> Options;
		bool Anonymous = false;
		UnknownFields Unknown;
	};

	struct Event {
		std::uint32_t Type = 0;
		std::uint32_t House = 0;
		std::int32_t Due = 0;
		Bytes Payload;
		bool Local = false;
		EventRole Role = EventRole::EVENT_ROLE_NONE;
		std::optional<Bytes> SealedPayload;
		UnknownFields Unknown;
	};

	struct Point {
		std::int32_t X = 0;
		std::int32_t Y = 0;
		UnknownFields Unknown;
	};

	struct Selection {
		std::vector<std::uint32_t> Ids;
		UnknownFields Unknown;
	};

	struct ViewState {
		std::optional<Replay::Message::Point> Camera;
		std::optional<Replay::Message::Selection> Selection;
		UnknownFields Unknown;
	};

	struct PlayerMessage {
		std::uint32_t House = 0;
		std::string Sender;
		std::string Text;
		std::int32_t Color = 0;
		MessageScope Scope = MessageScope::MESSAGE_SCOPE_UNKNOWN;
		std::vector<std::uint32_t> Recipients;
		Bytes Salt;
		UnknownFields Unknown;
	};

	struct HouseSample {
		std::uint32_t House = 0;
		std::optional<std::int64_t> Credits;
		std::optional<std::int64_t> StoredValue;
		std::optional<std::int64_t> Spent;
		std::optional<std::int64_t> Earned;
		std::optional<std::int32_t> PowerOutput;
		std::optional<std::int32_t> PowerDrain;
		std::optional<std::uint32_t> Vehicles;
		std::optional<std::uint32_t> Infantry;
		std::optional<std::uint32_t> Aircraft;
		std::optional<std::uint32_t> Buildings;
		std::optional<std::int64_t> ArmyValue;
		std::optional<std::int64_t> BuildingValue;
		std::optional<std::uint32_t> UnitsKilled;
		std::optional<std::uint32_t> BuildingsKilled;
		std::optional<std::uint32_t> UnitsLost;
		std::optional<std::uint32_t> BuildingsLost;
		std::optional<std::uint32_t> UnitsBuilt;
		std::optional<std::uint32_t> BuildingsBuilt;
		std::optional<std::int64_t> Score;
		bool Defeated = false;
		UnknownFields Unknown;
	};

	struct Income {
		std::uint32_t House = 0;
		std::uint32_t Kind = 0;
		std::int64_t Amount = 0;
		UnknownFields Unknown;
	};

	struct Seal {
		std::uint32_t Through = 0;
		Bytes Hash;
		UnknownFields Unknown;
	};

	struct Frame {
		std::uint32_t Skipped = 0;
		std::optional<std::uint32_t> Hash;
		std::vector<Event> Events;
		ViewState View;
		std::vector<PlayerMessage> Messages;
		std::optional<std::uint32_t> FrameMicros;
		std::vector<HouseSample> Samples;
		std::vector<Replay::Message::Income> Income;
		std::vector<Seal> Seals;
		std::vector<std::uint32_t> Defeated;
		UnknownFields Unknown;
	};

	struct Checkpoint {
		std::uint32_t Frame = 0;
		std::uint32_t Crc32 = 0;
		Bytes State;
		UnknownFields Unknown;
	};

	struct ObjectType {
		ObjectCategory Category = ObjectCategory::OBJECT_CATEGORY_UNKNOWN;
		std::uint32_t Index = 0;
		std::string Id;
		std::string Name;
		std::int32_t Cost = 0;
		std::string Icon;
		bool Naval = false;
		bool Unscored = false;
		bool Insignificant = false;
		UnknownFields Unknown;
	};

	struct TypeCount {
		CountKind Kind = CountKind::COUNT_KIND_UNKNOWN;
		ObjectCategory Category = ObjectCategory::OBJECT_CATEGORY_UNKNOWN;
		std::uint32_t Index = 0;
		std::uint32_t Count = 0;
		UnknownFields Unknown;
	};

	struct CrateCount {
		std::string Id;
		std::uint32_t Count = 0;
		UnknownFields Unknown;
	};

	struct HouseKills {
		std::uint32_t House = 0;
		std::uint32_t Units = 0;
		std::uint32_t Buildings = 0;
		UnknownFields Unknown;
	};

	struct HouseStatistics {
		std::uint32_t House = 0;
		std::string Name;
		std::string CountryName;
		std::int32_t Color = 0;
		std::int32_t Start = 0;
		std::vector<std::uint32_t> Allies;
		Replay::Outcome Outcome = Replay::Outcome::OUTCOME_UNKNOWN;
		LeaveReason Leave = LeaveReason::LEAVE_REASON_NONE;
		HouseSample Final;
		std::vector<TypeCount> Counts;
		std::vector<HouseKills> Kills;
		std::vector<CrateCount> Crates;
		UnknownFields Unknown;
	};

	struct Statistics {
		std::vector<ObjectType> Types;
		std::vector<HouseStatistics> Houses;
		UnknownFields Unknown;
	};

	struct RecordingSummary {
		std::optional<std::uint32_t> LastFrame;
		std::uint64_t EndedAt = 0;
		std::optional<std::uint32_t> DesyncFrame;
		bool Completed = false;
		std::vector<Seal> Seals;
		std::optional<std::uint64_t> ElapsedMicros;
		std::optional<std::uint64_t> GameMicros;
		UnknownFields Unknown;
	};

	// Appends the canonical encoding of a message to out.
	void Encode(RecordingInfo const & message, Bytes & out);
	void Encode(Launch const & message, Bytes & out);
	void Encode(Frame const & message, Bytes & out);
	void Encode(Checkpoint const & message, Bytes & out);
	void Encode(Statistics const & message, Bytes & out);
	void Encode(RecordingSummary const & message, Bytes & out);
	void Encode(FileHash const & message, Bytes & out);
	void Encode(PlayerColor const & message, Bytes & out);
	void Encode(IncomeKind const & message, Bytes & out);
	void Encode(Player const & message, Bytes & out);
	void Encode(GameOption const & message, Bytes & out);
	void Encode(Event const & message, Bytes & out);
	void Encode(ViewState const & message, Bytes & out);
	void Encode(Point const & message, Bytes & out);
	void Encode(Selection const & message, Bytes & out);
	void Encode(PlayerMessage const & message, Bytes & out);
	void Encode(HouseSample const & message, Bytes & out);
	void Encode(Income const & message, Bytes & out);
	void Encode(Seal const & message, Bytes & out);
	void Encode(ObjectType const & message, Bytes & out);
	void Encode(HouseStatistics const & message, Bytes & out);
	void Encode(TypeCount const & message, Bytes & out);
	void Encode(CrateCount const & message, Bytes & out);
	void Encode(HouseKills const & message, Bytes & out);

	// Replaces message with the decoded bytes; false on malformed input, message partly filled.
	bool Decode(ByteSpan data, RecordingInfo & message);
	bool Decode(ByteSpan data, Launch & message);
	bool Decode(ByteSpan data, Frame & message);
	bool Decode(ByteSpan data, Checkpoint & message);
	bool Decode(ByteSpan data, Statistics & message);
	bool Decode(ByteSpan data, RecordingSummary & message);
	bool Decode(ByteSpan data, FileHash & message);
	bool Decode(ByteSpan data, PlayerColor & message);
	bool Decode(ByteSpan data, IncomeKind & message);
	bool Decode(ByteSpan data, Player & message);
	bool Decode(ByteSpan data, GameOption & message);
	bool Decode(ByteSpan data, Event & message);
	bool Decode(ByteSpan data, ViewState & message);
	bool Decode(ByteSpan data, Point & message);
	bool Decode(ByteSpan data, Selection & message);
	bool Decode(ByteSpan data, PlayerMessage & message);
	bool Decode(ByteSpan data, HouseSample & message);
	bool Decode(ByteSpan data, Income & message);
	bool Decode(ByteSpan data, Seal & message);
	bool Decode(ByteSpan data, ObjectType & message);
	bool Decode(ByteSpan data, HouseStatistics & message);
	bool Decode(ByteSpan data, TypeCount & message);
	bool Decode(ByteSpan data, CrateCount & message);
	bool Decode(ByteSpan data, HouseKills & message);

	// As Decode, but false also on a broken limit, which broken names (NONE when malformed).
	bool Decode(ByteSpan data, RecordingInfo & message, LimitsType const & limits, RuleError & broken);
	bool Decode(ByteSpan data, Launch & message, LimitsType const & limits, RuleError & broken);
	bool Decode(ByteSpan data, Frame & message, LimitsType const & limits, RuleError & broken);
	bool Decode(ByteSpan data, Statistics & message, LimitsType const & limits, RuleError & broken);

	// Returns the first of the format's limits that message breaks, or NONE.
	RuleError Check_Limits(RecordingInfo const & message, LimitsType const & limits);
	RuleError Check_Limits(Launch const & message, LimitsType const & limits);
	RuleError Check_Limits(Frame const & message, LimitsType const & limits);
	RuleError Check_Limits(Statistics const & message, LimitsType const & limits);

	// Returns NOT_ASCENDING when a list in message breaks the order the format sets, or NONE.
	RuleError Check_Order(RecordingInfo const & message);
	RuleError Check_Order(Launch const & message);
	RuleError Check_Order(Frame const & message);
	RuleError Check_Order(Statistics const & message);

	// Returns INCOME_KIND when two income kinds in message share a number or an id, or one has an
	// empty id, and otherwise NONE.
	RuleError Check_Income_Kinds(RecordingInfo const & message);
}

namespace Replay
{
	using Message::Encode;
	using Message::Decode;
	using Message::Check_Limits;
	using Message::Check_Order;
	using Message::Check_Income_Kinds;

	// The income kind numbers that a file's RecordingInfo lists.
	class IncomeKindSet
	{
		public:
			IncomeKindSet(void) = default;
			explicit IncomeKindSet(Message::RecordingInfo const & info);

			// Returns INCOME_KIND when an Income entry in frame uses a kind not in the set, and otherwise NONE.
			RuleError Check(Message::Frame const & frame) const;

		private:
			std::set<std::uint32_t> Numbers;
	};

	// Returns text as valid UTF-8 of at most limit bytes.
	std::string Clean_Text(std::string_view text, std::size_t limit);
}
