/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "protowire.h"

#include "replay/replay.hh"

#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// The records of replay.proto as plain structs. Encode writes the canonical encoding, Decode
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

	// A field kept as read: unknown to this codec, of another wire type, or engine-private.
	struct UnknownField {
		std::uint32_t Number = 0;
		Bytes Encoded;               // tag and value, as read
	};

	using UnknownFields = std::vector<UnknownField>;

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
		std::optional<std::int32_t> AiLevel;
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
		std::optional<Replay::Point> Camera;
		std::optional<Replay::Selection> Selection;
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
		std::vector<Replay::Income> Income;
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

	// Appends the canonical encoding of a record to out.
	void Encode(RecordingInfo const & record, Bytes & out);
	void Encode(Launch const & record, Bytes & out);
	void Encode(Frame const & record, Bytes & out);
	void Encode(Checkpoint const & record, Bytes & out);
	void Encode(Statistics const & record, Bytes & out);
	void Encode(RecordingSummary const & record, Bytes & out);
	void Encode(FileHash const & record, Bytes & out);
	void Encode(PlayerColor const & record, Bytes & out);
	void Encode(IncomeKind const & record, Bytes & out);
	void Encode(Player const & record, Bytes & out);
	void Encode(GameOption const & record, Bytes & out);
	void Encode(Event const & record, Bytes & out);
	void Encode(ViewState const & record, Bytes & out);
	void Encode(Point const & record, Bytes & out);
	void Encode(Selection const & record, Bytes & out);
	void Encode(PlayerMessage const & record, Bytes & out);
	void Encode(HouseSample const & record, Bytes & out);
	void Encode(Income const & record, Bytes & out);
	void Encode(Seal const & record, Bytes & out);
	void Encode(ObjectType const & record, Bytes & out);
	void Encode(HouseStatistics const & record, Bytes & out);
	void Encode(TypeCount const & record, Bytes & out);
	void Encode(CrateCount const & record, Bytes & out);
	void Encode(HouseKills const & record, Bytes & out);

	template<typename T>
	Bytes Encoded(T const & record)
	{
		Bytes out;
		Encode(record, out);
		return(out);
	}

	// Replaces record with the decoded bytes; false on malformed input, record partly filled.
	bool Decode(ByteSpan data, RecordingInfo & record);
	bool Decode(ByteSpan data, Launch & record);
	bool Decode(ByteSpan data, Frame & record);
	bool Decode(ByteSpan data, Checkpoint & record);
	bool Decode(ByteSpan data, Statistics & record);
	bool Decode(ByteSpan data, RecordingSummary & record);
	bool Decode(ByteSpan data, FileHash & record);
	bool Decode(ByteSpan data, PlayerColor & record);
	bool Decode(ByteSpan data, IncomeKind & record);
	bool Decode(ByteSpan data, Player & record);
	bool Decode(ByteSpan data, GameOption & record);
	bool Decode(ByteSpan data, Event & record);
	bool Decode(ByteSpan data, ViewState & record);
	bool Decode(ByteSpan data, Point & record);
	bool Decode(ByteSpan data, Selection & record);
	bool Decode(ByteSpan data, PlayerMessage & record);
	bool Decode(ByteSpan data, HouseSample & record);
	bool Decode(ByteSpan data, Income & record);
	bool Decode(ByteSpan data, Seal & record);
	bool Decode(ByteSpan data, ObjectType & record);
	bool Decode(ByteSpan data, HouseStatistics & record);
	bool Decode(ByteSpan data, TypeCount & record);
	bool Decode(ByteSpan data, CrateCount & record);
	bool Decode(ByteSpan data, HouseKills & record);

	enum class RuleError : std::uint8_t {
		NONE,
		STRING_TOO_LONG,
		TOO_MANY_PLAYERS,
		TOO_MANY_HOUSES,
		TOO_MANY_EVENTS,
		PAYLOAD_TOO_LARGE,
		NOT_ASCENDING,     // a list the format orders is out of order or repeats a value
		TOO_MANY_INCOME_KINDS,
		INCOME_KIND,       // two income kinds share a number or id, an id is empty, or a kind is not listed
	};

	// As Decode, but false also on a broken limit, which broken names (NONE when malformed).
	bool Decode(ByteSpan data, RecordingInfo & record, LimitsType const & limits, RuleError & broken);
	bool Decode(ByteSpan data, Launch & record, LimitsType const & limits, RuleError & broken);
	bool Decode(ByteSpan data, Frame & record, LimitsType const & limits, RuleError & broken);
	bool Decode(ByteSpan data, Statistics & record, LimitsType const & limits, RuleError & broken);

	// Returns the first of the format's limits that record breaks, or NONE.
	RuleError Check_Limits(RecordingInfo const & record, LimitsType const & limits);
	RuleError Check_Limits(Launch const & record, LimitsType const & limits);
	RuleError Check_Limits(Frame const & record, LimitsType const & limits);
	RuleError Check_Limits(Statistics const & record, LimitsType const & limits);

	// Returns NOT_ASCENDING when a list in record breaks the order the format sets, or NONE.
	RuleError Check_Order(RecordingInfo const & record);
	RuleError Check_Order(Launch const & record);
	RuleError Check_Order(Frame const & record);
	RuleError Check_Order(Statistics const & record);

	// Returns INCOME_KIND when two income kinds in record share a number or an id, or one has an
	// empty id, and otherwise NONE.
	RuleError Check_Income_Kinds(RecordingInfo const & record);

	// The income kind numbers that a file's RecordingInfo lists.
	class IncomeKindSet
	{
		public:
			IncomeKindSet(void) = default;
			explicit IncomeKindSet(RecordingInfo const & info);

			// Returns INCOME_KIND when an Income entry in record uses a kind not in the set, and otherwise NONE.
			RuleError Check(Frame const & record) const;

		private:
			std::set<std::uint32_t> Numbers;
	};

	// Returns text as valid UTF-8 of at most limit bytes.
	std::string Clean_Text(std::string_view text, std::size_t limit);
}
