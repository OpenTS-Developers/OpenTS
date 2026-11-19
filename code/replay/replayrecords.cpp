/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "replay/replayrecords.h"

#include "utf8.h"

#include "replay/replayfields.hh"

#include <algorithm>
#include <utility>


namespace Replay
{

namespace F = Field;

namespace
{

struct DecodeContext {
	LimitsType const & Limits;
	RuleError Broken = RuleError::NONE;
};

}

bool Read_Message(ByteSpan data, FileHash & record, DecodeContext * context);
bool Read_Message(ByteSpan data, PlayerColor & record, DecodeContext * context);
bool Read_Message(ByteSpan data, RecordingInfo & record, DecodeContext * context);
bool Read_Message(ByteSpan data, Player & record, DecodeContext * context);
bool Read_Message(ByteSpan data, GameOption & record, DecodeContext * context);
bool Read_Message(ByteSpan data, Launch & record, DecodeContext * context);
bool Read_Message(ByteSpan data, Event & record, DecodeContext * context);
bool Read_Message(ByteSpan data, Point & record, DecodeContext * context);
bool Read_Message(ByteSpan data, Selection & record, DecodeContext * context);
bool Read_Message(ByteSpan data, ViewState & record, DecodeContext * context);
bool Read_Message(ByteSpan data, PlayerMessage & record, DecodeContext * context);
bool Read_Message(ByteSpan data, HouseSample & record, DecodeContext * context);
bool Read_Message(ByteSpan data, Income & record, DecodeContext * context);
bool Read_Message(ByteSpan data, IncomeKind & record, DecodeContext * context);
bool Read_Message(ByteSpan data, Seal & record, DecodeContext * context);
bool Read_Message(ByteSpan data, Frame & record, DecodeContext * context);
bool Read_Message(ByteSpan data, Checkpoint & record, DecodeContext * context);
bool Read_Message(ByteSpan data, ObjectType & record, DecodeContext * context);
bool Read_Message(ByteSpan data, TypeCount & record, DecodeContext * context);
bool Read_Message(ByteSpan data, CrateCount & record, DecodeContext * context);
bool Read_Message(ByteSpan data, HouseKills & record, DecodeContext * context);
bool Read_Message(ByteSpan data, HouseStatistics & record, DecodeContext * context);
bool Read_Message(ByteSpan data, Statistics & record, DecodeContext * context);
bool Read_Message(ByteSpan data, RecordingSummary & record, DecodeContext * context);


namespace
{

// Writes known fields through At in ascending field number and puts each kept unknown field
// before the first known field numbered above it.
class Emitter
{
	public:
		Emitter(Bytes & out, UnknownFields const & unknown) :
			Writer(out),
			Unknown(unknown),
			Next(0)
		{
			Order.reserve(unknown.size());
			for (std::size_t i = 0; i < unknown.size(); i++) {
				Order.push_back(i);
			}
			std::stable_sort(Order.begin(), Order.end(), [&](std::size_t a, std::size_t b) {
				return(unknown[a].Number < unknown[b].Number);
			});
		}

		~Emitter(void)
		{
			while (Next < Order.size()) {
				Writer.Raw(Unknown[Order[Next++]].Encoded);
			}
		}

		WireWriter & At(std::uint32_t field)
		{
			while (Next < Order.size() && Unknown[Order[Next]].Number < field) {
				Writer.Raw(Unknown[Order[Next++]].Encoded);
			}
			return(Writer);
		}

		template<typename T>
		void Message(std::uint32_t field, T const & child)
		{
			Bytes body;
			Encode(child, body);
			At(field).Message(field, body);
		}

		template<typename T>
		void Present_Message(std::uint32_t field, std::optional<T> const & child)
		{
			if (child) {
				Bytes body;
				Encode(*child, body);
				At(field).Present_Message(field, body);
			}
		}

		template<typename T>
		void Repeated(std::uint32_t field, std::vector<T> const & children)
		{
			for (T const & child : children) {
				Bytes body;
				Encode(child, body);
				At(field).Present_Message(field, body);
			}
		}

		void Optional_Uint(std::uint32_t field, std::optional<std::uint64_t> value)
		{
			if (value) {
				At(field).Present_Uint(field, *value);
			}
		}

		void Optional_Sint32(std::uint32_t field, std::optional<std::int32_t> value)
		{
			if (value) {
				At(field).Present_Sint32(field, *value);
			}
		}

		void Optional_Sint64(std::uint32_t field, std::optional<std::int64_t> value)
		{
			if (value) {
				At(field).Present_Sint64(field, *value);
			}
		}

		void Optional_Fixed32(std::uint32_t field, std::optional<std::uint32_t> value)
		{
			if (value) {
				At(field).Present_Fixed32(field, *value);
			}
		}

	private:
		WireWriter Writer;
		UnknownFields const & Unknown;
		std::vector<std::size_t> Order;
		std::size_t Next;
};


std::optional<std::uint64_t> Widen(std::optional<std::uint32_t> value)
{
	return(value ? std::optional<std::uint64_t>(*value) : std::nullopt);
}


// Reads the fields of one message. A getter keeps a field of another wire type as unknown, and
// a malformed nested body makes the whole message malformed.
class Fields
{
	public:
		Fields(ByteSpan data, UnknownFields & unknown, DecodeContext * context) :
			Reader(data),
			Unknown(unknown),
			Context(context),
			Bad(false)
		{
		}

		bool Next(void)
		{
			return(!Bad && Reader.Next(Field));
		}

		bool Good(void) const
		{
			return(!Bad && Reader.Error() == WireError::NONE);
		}

		std::uint32_t Number(void) const
		{
			return(Field.Number);
		}

		void Keep(void)
		{
			UnknownField kept;
			kept.Number = Field.Number;
			kept.Encoded.assign(Field.Encoded.begin(), Field.Encoded.end());
			Unknown.push_back(std::move(kept));
		}

		bool Is(WireType type)
		{
			if (Field.Type == type) {
				return(true);
			}
			Keep();
			return(false);
		}

		bool Uint32(std::uint32_t & value)
		{
			if (!Is(WireType::VARINT)) {
				return(false);
			}
			value = Field.As_Uint32();
			return(true);
		}

		bool Uint64(std::uint64_t & value)
		{
			if (!Is(WireType::VARINT)) {
				return(false);
			}
			value = Field.As_Uint64();
			return(true);
		}

		bool Sint32(std::int32_t & value)
		{
			if (!Is(WireType::VARINT)) {
				return(false);
			}
			value = Field.As_Sint32();
			return(true);
		}

		bool Sint64(std::int64_t & value)
		{
			if (!Is(WireType::VARINT)) {
				return(false);
			}
			value = Field.As_Sint64();
			return(true);
		}

		bool Bool(bool & value)
		{
			if (!Is(WireType::VARINT)) {
				return(false);
			}
			value = Field.As_Bool();
			return(true);
		}

		template<typename E>
		bool Enum(E & value)
		{
			if (!Is(WireType::VARINT)) {
				return(false);
			}
			value = E(Field.As_Int32());
			return(true);
		}

		bool Fixed32(std::uint32_t & value)
		{
			if (!Is(WireType::FIXED32)) {
				return(false);
			}
			value = Field.As_Uint32();
			return(true);
		}

		bool Blob(Bytes & value)
		{
			if (!Is(WireType::LENGTH)) {
				return(false);
			}
			value.assign(Field.Body.begin(), Field.Body.end());
			return(true);
		}

		// A string that is not valid UTF-8 makes the whole message malformed, as it does for a
		// generated parser.
		bool Text(std::string & value)
		{
			if (!Is(WireType::LENGTH)) {
				return(false);
			}
			if (Context != nullptr && Field.Body.size() > Context->Limits.String) {
				return(Break(RuleError::STRING_TOO_LONG));
			}
			std::string_view const text(reinterpret_cast<char const *>(Field.Body.data()), Field.Body.size());
			if (!UTF8::Is_Valid(text)) {
				Bad = true;
				return(false);
			}
			value.assign(text);
			return(true);
		}

		bool Payload(Bytes & value)
		{
			if (Field.Type == WireType::LENGTH && Context != nullptr && Field.Body.size() > Context->Limits.EventPayload) {
				return(Break(RuleError::PAYLOAD_TOO_LARGE));
			}
			return(Blob(value));
		}

		template<typename T, typename Getter>
		void Optional(std::optional<T> & value, Getter get)
		{
			T taken{};
			if ((this->*get)(taken)) {
				value = taken;
			}
		}

		void Packed(std::vector<std::uint32_t> & values)
		{
			if (Field.Type != WireType::VARINT && Field.Type != WireType::LENGTH) {
				Keep();
				return;
			}
			if (!Read_Repeated_Uint32(Field, values)) {
				Bad = true;
			}
		}

		template<typename T>
		bool Message(T & value)
		{
			if (!Is(WireType::LENGTH)) {
				return(false);
			}
			T child;
			if (!Read_Message(Field.Body, child, Context)) {
				Bad = true;
				return(false);
			}
			value = std::move(child);
			return(true);
		}

		template<typename T>
		void Optional_Message(std::optional<T> & value)
		{
			T child;
			if (Message(child)) {
				value = std::move(child);
			}
		}

		template<typename T>
		void Repeated(std::vector<T> & values)
		{
			T child;
			if (Message(child)) {
				values.push_back(std::move(child));
			}
		}

		// Refuses the element that would pass the limit before decoding it.
		template<typename T>
		void Repeated(std::vector<T> & values, std::size_t LimitsType::* limit, RuleError error)
		{
			if (Field.Type == WireType::LENGTH && Context != nullptr && values.size() >= Context->Limits.*limit) {
				Break(error);
				return;
			}
			Repeated(values);
		}

	private:
		bool Break(RuleError error)
		{
			if (Context->Broken == RuleError::NONE) {
				Context->Broken = error;
			}
			Bad = true;
			return(false);
		}

		WireReader Reader;
		WireField Field;
		UnknownFields & Unknown;
		DecodeContext * Context;
		bool Bad;
};


bool Fits(std::string const & text, LimitsType const & limits)
{
	return(text.size() <= limits.String);
}


bool Ascending(std::vector<std::uint32_t> const & values)
{
	for (std::size_t i = 1; i < values.size(); i++) {
		if (values[i] <= values[i - 1]) {
			return(false);
		}
	}
	return(true);
}


bool Allies_In_Order(std::vector<std::uint32_t> const & allies, std::uint32_t house)
{
	return(Ascending(allies) && std::find(allies.begin(), allies.end(), house) == allies.end());
}

}


void Encode(FileHash const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::FileHash::PATH).String(F::FileHash::PATH, record.Path);
	emit.At(F::FileHash::HASH).String(F::FileHash::HASH, record.Hash);
}


void Encode(PlayerColor const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::PlayerColor::NUMBER).Uint(F::PlayerColor::NUMBER, record.Number);
	emit.At(F::PlayerColor::RGB).Fixed32(F::PlayerColor::RGB, record.Rgb);
}


void Encode(RecordingInfo const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::RecordingInfo::ENGINE).String(F::RecordingInfo::ENGINE, record.Engine);
	emit.At(F::RecordingInfo::ENGINE_VERSION).String(F::RecordingInfo::ENGINE_VERSION, record.EngineVersion);
	emit.At(F::RecordingInfo::DATA_VERSION).Uint(F::RecordingInfo::DATA_VERSION, record.DataVersion);
	emit.At(F::RecordingInfo::PACKAGE).String(F::RecordingInfo::PACKAGE, record.Package);
	emit.At(F::RecordingInfo::PACKAGE_VERSION).String(F::RecordingInfo::PACKAGE_VERSION, record.PackageVersion);
	emit.Repeated(F::RecordingInfo::FILES, record.Files);
	emit.At(F::RecordingInfo::RECORDED_AT).Uint(F::RecordingInfo::RECORDED_AT, record.RecordedAt);
	emit.Optional_Uint(F::RecordingInfo::RECORDER, Widen(record.Recorder));
	emit.At(F::RecordingInfo::MAP_NAME).String(F::RecordingInfo::MAP_NAME, record.MapName);
	emit.At(F::RecordingInfo::MAP_PATH).String(F::RecordingInfo::MAP_PATH, record.MapPath);
	emit.At(F::RecordingInfo::MAP_SHA256).Blob(F::RecordingInfo::MAP_SHA256, record.MapSha256);
	emit.At(F::RecordingInfo::GAME_MODE).String(F::RecordingInfo::GAME_MODE, record.GameMode);
	emit.At(F::RecordingInfo::SEAL_VERSION).Uint(F::RecordingInfo::SEAL_VERSION, record.SealVersion);
	emit.Repeated(F::RecordingInfo::COLORS, record.Colors);
	emit.Repeated(F::RecordingInfo::INCOME_KINDS, record.IncomeKinds);
}


void Encode(Player const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::Player::HOUSE).Uint(F::Player::HOUSE, record.House);
	emit.At(F::Player::NAME).String(F::Player::NAME, record.Name);
	emit.At(F::Player::HUMAN).Bool(F::Player::HUMAN, record.Human);
	emit.At(F::Player::OBSERVER).Bool(F::Player::OBSERVER, record.Observer);
	emit.At(F::Player::COUNTRY).Sint32(F::Player::COUNTRY, record.Country);
	emit.At(F::Player::COUNTRY_NAME).String(F::Player::COUNTRY_NAME, record.CountryName);
	emit.At(F::Player::COLOR).Sint32(F::Player::COLOR, record.Color);
	emit.At(F::Player::START).Sint32(F::Player::START, record.Start);
	emit.Optional_Sint32(F::Player::AI_LEVEL, record.AiLevel);
	emit.At(F::Player::ALLIES).Packed_Uint32(F::Player::ALLIES, record.Allies);
	emit.Optional_Sint32(F::Player::RESOLVED_COUNTRY, record.ResolvedCountry);
	emit.Optional_Sint32(F::Player::RESOLVED_COLOR, record.ResolvedColor);
	emit.Optional_Sint32(F::Player::RESOLVED_START, record.ResolvedStart);
}


void Encode(GameOption const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::GameOption::KEY).String(F::GameOption::KEY, record.Key);
	if (auto number = std::get_if<std::int64_t>(&record.Value)) {
		emit.At(F::GameOption::NUMBER).Present_Sint64(F::GameOption::NUMBER, *number);
	} else if (auto flag = std::get_if<bool>(&record.Value)) {
		emit.At(F::GameOption::FLAG).Present_Bool(F::GameOption::FLAG, *flag);
	} else if (auto text = std::get_if<std::string>(&record.Value)) {
		emit.At(F::GameOption::TEXT).Present_String(F::GameOption::TEXT, *text);
	}
}


void Encode(Launch const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::Launch::KIND).Int32(F::Launch::KIND, std::int32_t(record.Kind));
	emit.Optional_Fixed32(F::Launch::SEED, record.Seed);
	emit.Repeated(F::Launch::PLAYERS, record.Players);
	emit.Repeated(F::Launch::OPTIONS, record.Options);
	emit.At(F::Launch::ANONYMOUS).Bool(F::Launch::ANONYMOUS, record.Anonymous);
}


void Encode(Event const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::Event::TYPE).Uint(F::Event::TYPE, record.Type);
	emit.At(F::Event::HOUSE).Uint(F::Event::HOUSE, record.House);
	emit.At(F::Event::DUE).Sint32(F::Event::DUE, record.Due);
	emit.At(F::Event::PAYLOAD).Blob(F::Event::PAYLOAD, record.Payload);
	emit.At(F::Event::LOCAL).Bool(F::Event::LOCAL, record.Local);
	emit.At(F::Event::ROLE).Int32(F::Event::ROLE, std::int32_t(record.Role));
	if (record.SealedPayload) {
		emit.At(F::Event::SEALED_PAYLOAD).Present_Blob(F::Event::SEALED_PAYLOAD, *record.SealedPayload);
	}
}


void Encode(Point const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::Point::X).Sint32(F::Point::X, record.X);
	emit.At(F::Point::Y).Sint32(F::Point::Y, record.Y);
}


void Encode(Selection const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::Selection::IDS).Packed_Uint32(F::Selection::IDS, record.Ids);
}


void Encode(ViewState const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.Present_Message(F::ViewState::CAMERA, record.Camera);
	emit.Present_Message(F::ViewState::SELECTION, record.Selection);
}


void Encode(PlayerMessage const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::PlayerMessage::HOUSE).Uint(F::PlayerMessage::HOUSE, record.House);
	emit.At(F::PlayerMessage::SENDER).String(F::PlayerMessage::SENDER, record.Sender);
	emit.At(F::PlayerMessage::TEXT).String(F::PlayerMessage::TEXT, record.Text);
	emit.At(F::PlayerMessage::COLOR).Sint32(F::PlayerMessage::COLOR, record.Color);
	emit.At(F::PlayerMessage::SCOPE).Int32(F::PlayerMessage::SCOPE, std::int32_t(record.Scope));
	emit.At(F::PlayerMessage::RECIPIENTS).Packed_Uint32(F::PlayerMessage::RECIPIENTS, record.Recipients);
	emit.At(F::PlayerMessage::SALT).Blob(F::PlayerMessage::SALT, record.Salt);
}


void Encode(HouseSample const & record, Bytes & out)
{
	namespace S = F::HouseSample;
	Emitter emit(out, record.Unknown);
	emit.At(S::HOUSE).Uint(S::HOUSE, record.House);
	emit.Optional_Sint64(S::CREDITS, record.Credits);
	emit.Optional_Sint64(S::STORED_VALUE, record.StoredValue);
	emit.Optional_Sint64(S::SPENT, record.Spent);
	emit.Optional_Sint64(S::EARNED, record.Earned);
	emit.Optional_Sint32(S::POWER_OUTPUT, record.PowerOutput);
	emit.Optional_Sint32(S::POWER_DRAIN, record.PowerDrain);
	emit.Optional_Uint(S::VEHICLES, Widen(record.Vehicles));
	emit.Optional_Uint(S::INFANTRY, Widen(record.Infantry));
	emit.Optional_Uint(S::AIRCRAFT, Widen(record.Aircraft));
	emit.Optional_Uint(S::BUILDINGS, Widen(record.Buildings));
	emit.Optional_Sint64(S::ARMY_VALUE, record.ArmyValue);
	emit.Optional_Sint64(S::BUILDING_VALUE, record.BuildingValue);
	emit.Optional_Uint(S::UNITS_KILLED, Widen(record.UnitsKilled));
	emit.Optional_Uint(S::BUILDINGS_KILLED, Widen(record.BuildingsKilled));
	emit.Optional_Uint(S::UNITS_LOST, Widen(record.UnitsLost));
	emit.Optional_Uint(S::BUILDINGS_LOST, Widen(record.BuildingsLost));
	emit.Optional_Uint(S::UNITS_BUILT, Widen(record.UnitsBuilt));
	emit.Optional_Uint(S::BUILDINGS_BUILT, Widen(record.BuildingsBuilt));
	emit.Optional_Sint64(S::SCORE, record.Score);
	emit.At(S::DEFEATED).Bool(S::DEFEATED, record.Defeated);
}


void Encode(Income const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::Income::HOUSE).Uint(F::Income::HOUSE, record.House);
	emit.At(F::Income::KIND).Uint(F::Income::KIND, record.Kind);
	emit.At(F::Income::AMOUNT).Sint64(F::Income::AMOUNT, record.Amount);
}


void Encode(IncomeKind const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::IncomeKind::NUMBER).Uint(F::IncomeKind::NUMBER, record.Number);
	emit.At(F::IncomeKind::ID).String(F::IncomeKind::ID, record.Id);
}


void Encode(Seal const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::Seal::THROUGH).Uint(F::Seal::THROUGH, record.Through);
	emit.At(F::Seal::HASH).Blob(F::Seal::HASH, record.Hash);
}


void Encode(Frame const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::Frame::SKIPPED).Uint(F::Frame::SKIPPED, record.Skipped);
	emit.Optional_Fixed32(F::Frame::HASH, record.Hash);
	emit.Repeated(F::Frame::EVENTS, record.Events);
	emit.Message(F::Frame::VIEW, record.View);
	emit.Repeated(F::Frame::MESSAGES, record.Messages);
	emit.Optional_Uint(F::Frame::FRAME_MICROS, Widen(record.FrameMicros));
	emit.Repeated(F::Frame::SAMPLES, record.Samples);
	emit.Repeated(F::Frame::INCOME, record.Income);
	emit.Repeated(F::Frame::SEALS, record.Seals);
	emit.At(F::Frame::DEFEATED).Packed_Uint32(F::Frame::DEFEATED, record.Defeated);
}


void Encode(Checkpoint const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::Checkpoint::FRAME).Uint(F::Checkpoint::FRAME, record.Frame);
	emit.At(F::Checkpoint::CRC32).Fixed32(F::Checkpoint::CRC32, record.Crc32);
	emit.At(F::Checkpoint::STATE).Blob(F::Checkpoint::STATE, record.State);
}


void Encode(ObjectType const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::ObjectType::CATEGORY).Int32(F::ObjectType::CATEGORY, std::int32_t(record.Category));
	emit.At(F::ObjectType::INDEX).Uint(F::ObjectType::INDEX, record.Index);
	emit.At(F::ObjectType::ID).String(F::ObjectType::ID, record.Id);
	emit.At(F::ObjectType::NAME).String(F::ObjectType::NAME, record.Name);
	emit.At(F::ObjectType::COST).Sint32(F::ObjectType::COST, record.Cost);
	emit.At(F::ObjectType::ICON).String(F::ObjectType::ICON, record.Icon);
	emit.At(F::ObjectType::NAVAL).Bool(F::ObjectType::NAVAL, record.Naval);
	emit.At(F::ObjectType::UNSCORED).Bool(F::ObjectType::UNSCORED, record.Unscored);
	emit.At(F::ObjectType::INSIGNIFICANT).Bool(F::ObjectType::INSIGNIFICANT, record.Insignificant);
}


void Encode(TypeCount const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::TypeCount::KIND).Int32(F::TypeCount::KIND, std::int32_t(record.Kind));
	emit.At(F::TypeCount::CATEGORY).Int32(F::TypeCount::CATEGORY, std::int32_t(record.Category));
	emit.At(F::TypeCount::INDEX).Uint(F::TypeCount::INDEX, record.Index);
	emit.At(F::TypeCount::COUNT).Uint(F::TypeCount::COUNT, record.Count);
}


void Encode(CrateCount const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::CrateCount::ID).String(F::CrateCount::ID, record.Id);
	emit.At(F::CrateCount::COUNT).Uint(F::CrateCount::COUNT, record.Count);
}


void Encode(HouseKills const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.At(F::HouseKills::HOUSE).Uint(F::HouseKills::HOUSE, record.House);
	emit.At(F::HouseKills::UNITS).Uint(F::HouseKills::UNITS, record.Units);
	emit.At(F::HouseKills::BUILDINGS).Uint(F::HouseKills::BUILDINGS, record.Buildings);
}


void Encode(HouseStatistics const & record, Bytes & out)
{
	namespace H = F::HouseStatistics;
	Emitter emit(out, record.Unknown);
	emit.At(H::HOUSE).Uint(H::HOUSE, record.House);
	emit.At(H::NAME).String(H::NAME, record.Name);
	emit.At(H::COUNTRY_NAME).String(H::COUNTRY_NAME, record.CountryName);
	emit.At(H::COLOR).Sint32(H::COLOR, record.Color);
	emit.At(H::START).Sint32(H::START, record.Start);
	emit.At(H::ALLIES).Packed_Uint32(H::ALLIES, record.Allies);
	emit.At(H::OUTCOME).Int32(H::OUTCOME, std::int32_t(record.Outcome));
	emit.At(H::LEAVE).Int32(H::LEAVE, std::int32_t(record.Leave));
	emit.Message(H::FINAL, record.Final);
	emit.Repeated(H::COUNTS, record.Counts);
	emit.Repeated(H::KILLS, record.Kills);
	emit.Repeated(H::CRATES, record.Crates);
}


void Encode(Statistics const & record, Bytes & out)
{
	Emitter emit(out, record.Unknown);
	emit.Repeated(F::Statistics::TYPES, record.Types);
	emit.Repeated(F::Statistics::HOUSES, record.Houses);
}


void Encode(RecordingSummary const & record, Bytes & out)
{
	namespace R = F::RecordingSummary;
	Emitter emit(out, record.Unknown);
	emit.Optional_Uint(R::LAST_FRAME, Widen(record.LastFrame));
	emit.At(R::ENDED_AT).Uint(R::ENDED_AT, record.EndedAt);
	emit.Optional_Uint(R::DESYNC_FRAME, Widen(record.DesyncFrame));
	emit.At(R::COMPLETED).Bool(R::COMPLETED, record.Completed);
	emit.Repeated(R::SEALS, record.Seals);
	emit.Optional_Uint(R::ELAPSED_MICROS, record.ElapsedMicros);
	emit.Optional_Uint(R::GAME_MICROS, record.GameMicros);
}


bool Read_Message(ByteSpan data, FileHash & record, DecodeContext * context)
{
	record = FileHash();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::FileHash::PATH: fields.Text(record.Path); break;
			case F::FileHash::HASH: fields.Text(record.Hash); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, PlayerColor & record, DecodeContext * context)
{
	record = PlayerColor();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::PlayerColor::NUMBER: fields.Uint32(record.Number); break;
			case F::PlayerColor::RGB: fields.Fixed32(record.Rgb); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, RecordingInfo & record, DecodeContext * context)
{
	namespace I = F::RecordingInfo;
	record = RecordingInfo();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case I::ENGINE: fields.Text(record.Engine); break;
			case I::ENGINE_VERSION: fields.Text(record.EngineVersion); break;
			case I::DATA_VERSION: fields.Uint32(record.DataVersion); break;
			case I::PACKAGE: fields.Text(record.Package); break;
			case I::PACKAGE_VERSION: fields.Text(record.PackageVersion); break;
			case I::FILES: fields.Repeated(record.Files); break;
			case I::RECORDED_AT: fields.Uint64(record.RecordedAt); break;
			case I::RECORDER: fields.Optional(record.Recorder, &Fields::Uint32); break;
			case I::MAP_NAME: fields.Text(record.MapName); break;
			case I::MAP_PATH: fields.Text(record.MapPath); break;
			case I::MAP_SHA256: fields.Blob(record.MapSha256); break;
			case I::GAME_MODE: fields.Text(record.GameMode); break;
			case I::SEAL_VERSION: fields.Uint32(record.SealVersion); break;
			case I::COLORS: fields.Repeated(record.Colors); break;
			case I::INCOME_KINDS: fields.Repeated(record.IncomeKinds, &LimitsType::IncomeKinds, RuleError::TOO_MANY_INCOME_KINDS); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Player & record, DecodeContext * context)
{
	namespace P = F::Player;
	record = Player();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case P::HOUSE: fields.Uint32(record.House); break;
			case P::NAME: fields.Text(record.Name); break;
			case P::HUMAN: fields.Bool(record.Human); break;
			case P::OBSERVER: fields.Bool(record.Observer); break;
			case P::COUNTRY: fields.Sint32(record.Country); break;
			case P::COUNTRY_NAME: fields.Text(record.CountryName); break;
			case P::COLOR: fields.Sint32(record.Color); break;
			case P::START: fields.Sint32(record.Start); break;
			case P::AI_LEVEL: fields.Optional(record.AiLevel, &Fields::Sint32); break;
			case P::ALLIES: fields.Packed(record.Allies); break;
			case P::RESOLVED_COUNTRY: fields.Optional(record.ResolvedCountry, &Fields::Sint32); break;
			case P::RESOLVED_COLOR: fields.Optional(record.ResolvedColor, &Fields::Sint32); break;
			case P::RESOLVED_START: fields.Optional(record.ResolvedStart, &Fields::Sint32); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, GameOption & record, DecodeContext * context)
{
	record = GameOption();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::GameOption::KEY:
				fields.Text(record.Key);
				break;

			case F::GameOption::NUMBER:
			{
				std::int64_t number = 0;
				if (fields.Sint64(number)) {
					record.Value = number;
				}
				break;
			}

			case F::GameOption::FLAG:
			{
				bool flag = false;
				if (fields.Bool(flag)) {
					record.Value = flag;
				}
				break;
			}

			case F::GameOption::TEXT:
			{
				std::string text;
				if (fields.Text(text)) {
					record.Value = std::move(text);
				}
				break;
			}

			default:
				fields.Keep();
				break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Launch & record, DecodeContext * context)
{
	record = Launch();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Launch::KIND: fields.Enum(record.Kind); break;
			case F::Launch::SEED: fields.Optional(record.Seed, &Fields::Fixed32); break;
			case F::Launch::PLAYERS: fields.Repeated(record.Players, &LimitsType::Players, RuleError::TOO_MANY_PLAYERS); break;
			case F::Launch::OPTIONS: fields.Repeated(record.Options); break;
			case F::Launch::ANONYMOUS: fields.Bool(record.Anonymous); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Event & record, DecodeContext * context)
{
	record = Event();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Event::TYPE: fields.Uint32(record.Type); break;
			case F::Event::HOUSE: fields.Uint32(record.House); break;
			case F::Event::DUE: fields.Sint32(record.Due); break;
			case F::Event::PAYLOAD: fields.Payload(record.Payload); break;
			case F::Event::LOCAL: fields.Bool(record.Local); break;
			case F::Event::ROLE: fields.Enum(record.Role); break;
			case F::Event::SEALED_PAYLOAD: fields.Optional(record.SealedPayload, &Fields::Payload); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Point & record, DecodeContext * context)
{
	record = Point();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Point::X: fields.Sint32(record.X); break;
			case F::Point::Y: fields.Sint32(record.Y); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Selection & record, DecodeContext * context)
{
	record = Selection();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Selection::IDS: fields.Packed(record.Ids); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, ViewState & record, DecodeContext * context)
{
	record = ViewState();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::ViewState::CAMERA: fields.Optional_Message(record.Camera); break;
			case F::ViewState::SELECTION: fields.Optional_Message(record.Selection); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, PlayerMessage & record, DecodeContext * context)
{
	namespace M = F::PlayerMessage;
	record = PlayerMessage();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case M::HOUSE: fields.Uint32(record.House); break;
			case M::SENDER: fields.Text(record.Sender); break;
			case M::TEXT: fields.Text(record.Text); break;
			case M::COLOR: fields.Sint32(record.Color); break;
			case M::SCOPE: fields.Enum(record.Scope); break;
			case M::RECIPIENTS: fields.Packed(record.Recipients); break;
			case M::SALT: fields.Blob(record.Salt); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, HouseSample & record, DecodeContext * context)
{
	namespace S = F::HouseSample;
	record = HouseSample();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case S::HOUSE: fields.Uint32(record.House); break;
			case S::CREDITS: fields.Optional(record.Credits, &Fields::Sint64); break;
			case S::STORED_VALUE: fields.Optional(record.StoredValue, &Fields::Sint64); break;
			case S::SPENT: fields.Optional(record.Spent, &Fields::Sint64); break;
			case S::EARNED: fields.Optional(record.Earned, &Fields::Sint64); break;
			case S::POWER_OUTPUT: fields.Optional(record.PowerOutput, &Fields::Sint32); break;
			case S::POWER_DRAIN: fields.Optional(record.PowerDrain, &Fields::Sint32); break;
			case S::VEHICLES: fields.Optional(record.Vehicles, &Fields::Uint32); break;
			case S::INFANTRY: fields.Optional(record.Infantry, &Fields::Uint32); break;
			case S::AIRCRAFT: fields.Optional(record.Aircraft, &Fields::Uint32); break;
			case S::BUILDINGS: fields.Optional(record.Buildings, &Fields::Uint32); break;
			case S::ARMY_VALUE: fields.Optional(record.ArmyValue, &Fields::Sint64); break;
			case S::BUILDING_VALUE: fields.Optional(record.BuildingValue, &Fields::Sint64); break;
			case S::UNITS_KILLED: fields.Optional(record.UnitsKilled, &Fields::Uint32); break;
			case S::BUILDINGS_KILLED: fields.Optional(record.BuildingsKilled, &Fields::Uint32); break;
			case S::UNITS_LOST: fields.Optional(record.UnitsLost, &Fields::Uint32); break;
			case S::BUILDINGS_LOST: fields.Optional(record.BuildingsLost, &Fields::Uint32); break;
			case S::UNITS_BUILT: fields.Optional(record.UnitsBuilt, &Fields::Uint32); break;
			case S::BUILDINGS_BUILT: fields.Optional(record.BuildingsBuilt, &Fields::Uint32); break;
			case S::SCORE: fields.Optional(record.Score, &Fields::Sint64); break;
			case S::DEFEATED: fields.Bool(record.Defeated); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Income & record, DecodeContext * context)
{
	record = Income();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Income::HOUSE: fields.Uint32(record.House); break;
			case F::Income::KIND: fields.Uint32(record.Kind); break;
			case F::Income::AMOUNT: fields.Sint64(record.Amount); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, IncomeKind & record, DecodeContext * context)
{
	record = IncomeKind();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::IncomeKind::NUMBER: fields.Uint32(record.Number); break;
			case F::IncomeKind::ID: fields.Text(record.Id); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Seal & record, DecodeContext * context)
{
	record = Seal();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Seal::THROUGH: fields.Uint32(record.Through); break;
			case F::Seal::HASH: fields.Blob(record.Hash); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Frame & record, DecodeContext * context)
{
	record = Frame();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Frame::SKIPPED: fields.Uint32(record.Skipped); break;
			case F::Frame::HASH: fields.Optional(record.Hash, &Fields::Fixed32); break;
			case F::Frame::EVENTS: fields.Repeated(record.Events, &LimitsType::EventsPerRecord, RuleError::TOO_MANY_EVENTS); break;
			case F::Frame::VIEW: fields.Message(record.View); break;
			case F::Frame::MESSAGES: fields.Repeated(record.Messages); break;
			case F::Frame::FRAME_MICROS: fields.Optional(record.FrameMicros, &Fields::Uint32); break;
			case F::Frame::SAMPLES: fields.Repeated(record.Samples); break;
			case F::Frame::INCOME: fields.Repeated(record.Income); break;
			case F::Frame::SEALS: fields.Repeated(record.Seals); break;
			case F::Frame::DEFEATED: fields.Packed(record.Defeated); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Checkpoint & record, DecodeContext * context)
{
	record = Checkpoint();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Checkpoint::FRAME: fields.Uint32(record.Frame); break;
			case F::Checkpoint::CRC32: fields.Fixed32(record.Crc32); break;
			case F::Checkpoint::STATE: fields.Blob(record.State); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, ObjectType & record, DecodeContext * context)
{
	namespace O = F::ObjectType;
	record = ObjectType();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case O::CATEGORY: fields.Enum(record.Category); break;
			case O::INDEX: fields.Uint32(record.Index); break;
			case O::ID: fields.Text(record.Id); break;
			case O::NAME: fields.Text(record.Name); break;
			case O::COST: fields.Sint32(record.Cost); break;
			case O::ICON: fields.Text(record.Icon); break;
			case O::NAVAL: fields.Bool(record.Naval); break;
			case O::UNSCORED: fields.Bool(record.Unscored); break;
			case O::INSIGNIFICANT: fields.Bool(record.Insignificant); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, TypeCount & record, DecodeContext * context)
{
	record = TypeCount();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::TypeCount::KIND: fields.Enum(record.Kind); break;
			case F::TypeCount::CATEGORY: fields.Enum(record.Category); break;
			case F::TypeCount::INDEX: fields.Uint32(record.Index); break;
			case F::TypeCount::COUNT: fields.Uint32(record.Count); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, CrateCount & record, DecodeContext * context)
{
	record = CrateCount();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::CrateCount::ID: fields.Text(record.Id); break;
			case F::CrateCount::COUNT: fields.Uint32(record.Count); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, HouseKills & record, DecodeContext * context)
{
	record = HouseKills();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::HouseKills::HOUSE: fields.Uint32(record.House); break;
			case F::HouseKills::UNITS: fields.Uint32(record.Units); break;
			case F::HouseKills::BUILDINGS: fields.Uint32(record.Buildings); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, HouseStatistics & record, DecodeContext * context)
{
	namespace H = F::HouseStatistics;
	record = HouseStatistics();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case H::HOUSE: fields.Uint32(record.House); break;
			case H::NAME: fields.Text(record.Name); break;
			case H::COUNTRY_NAME: fields.Text(record.CountryName); break;
			case H::COLOR: fields.Sint32(record.Color); break;
			case H::START: fields.Sint32(record.Start); break;
			case H::ALLIES: fields.Packed(record.Allies); break;
			case H::OUTCOME: fields.Enum(record.Outcome); break;
			case H::LEAVE: fields.Enum(record.Leave); break;
			case H::FINAL: fields.Message(record.Final); break;
			case H::COUNTS: fields.Repeated(record.Counts); break;
			case H::KILLS: fields.Repeated(record.Kills); break;
			case H::CRATES: fields.Repeated(record.Crates); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Statistics & record, DecodeContext * context)
{
	record = Statistics();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Statistics::TYPES: fields.Repeated(record.Types); break;
			case F::Statistics::HOUSES: fields.Repeated(record.Houses, &LimitsType::Players, RuleError::TOO_MANY_HOUSES); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, RecordingSummary & record, DecodeContext * context)
{
	namespace R = F::RecordingSummary;
	record = RecordingSummary();
	Fields fields(data, record.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case R::LAST_FRAME: fields.Optional(record.LastFrame, &Fields::Uint32); break;
			case R::ENDED_AT: fields.Uint64(record.EndedAt); break;
			case R::DESYNC_FRAME: fields.Optional(record.DesyncFrame, &Fields::Uint32); break;
			case R::COMPLETED: fields.Bool(record.Completed); break;
			case R::SEALS: fields.Repeated(record.Seals); break;
			case R::ELAPSED_MICROS: fields.Optional(record.ElapsedMicros, &Fields::Uint64); break;
			case R::GAME_MICROS: fields.Optional(record.GameMicros, &Fields::Uint64); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Decode(ByteSpan data, FileHash & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, PlayerColor & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, RecordingInfo & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, Player & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, GameOption & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, Launch & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, Event & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, Point & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, Selection & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, ViewState & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, PlayerMessage & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, HouseSample & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, Income & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, IncomeKind & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, Seal & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, Frame & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, Checkpoint & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, ObjectType & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, TypeCount & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, CrateCount & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, HouseKills & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, HouseStatistics & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, Statistics & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, RecordingSummary & record)
{
	return(Read_Message(data, record, nullptr));
}


bool Decode(ByteSpan data, RecordingInfo & record, LimitsType const & limits, RuleError & broken)
{
	DecodeContext context{ limits };
	bool const good = Read_Message(data, record, &context);
	broken = context.Broken;
	return(good);
}


bool Decode(ByteSpan data, Launch & record, LimitsType const & limits, RuleError & broken)
{
	DecodeContext context{ limits };
	bool const good = Read_Message(data, record, &context);
	broken = context.Broken;
	return(good);
}


bool Decode(ByteSpan data, Frame & record, LimitsType const & limits, RuleError & broken)
{
	DecodeContext context{ limits };
	bool const good = Read_Message(data, record, &context);
	broken = context.Broken;
	return(good);
}


bool Decode(ByteSpan data, Statistics & record, LimitsType const & limits, RuleError & broken)
{
	DecodeContext context{ limits };
	bool const good = Read_Message(data, record, &context);
	broken = context.Broken;
	return(good);
}


RuleError Check_Limits(RecordingInfo const & record, LimitsType const & limits)
{
	for (std::string const * text : { &record.Engine, &record.EngineVersion, &record.Package,
		&record.PackageVersion, &record.MapName, &record.GameMode, &record.MapPath }) {
		if (!Fits(*text, limits)) {
			return(RuleError::STRING_TOO_LONG);
		}
	}
	for (FileHash const & file : record.Files) {
		if (!Fits(file.Path, limits) || !Fits(file.Hash, limits)) {
			return(RuleError::STRING_TOO_LONG);
		}
	}
	if (record.IncomeKinds.size() > limits.IncomeKinds) {
		return(RuleError::TOO_MANY_INCOME_KINDS);
	}
	for (IncomeKind const & kind : record.IncomeKinds) {
		if (!Fits(kind.Id, limits)) {
			return(RuleError::STRING_TOO_LONG);
		}
	}
	return(RuleError::NONE);
}


RuleError Check_Limits(Launch const & record, LimitsType const & limits)
{
	if (record.Players.size() > limits.Players) {
		return(RuleError::TOO_MANY_PLAYERS);
	}
	for (Player const & player : record.Players) {
		if (!Fits(player.Name, limits) || !Fits(player.CountryName, limits)) {
			return(RuleError::STRING_TOO_LONG);
		}
	}
	for (GameOption const & option : record.Options) {
		auto text = std::get_if<std::string>(&option.Value);
		if (!Fits(option.Key, limits) || (text != nullptr && !Fits(*text, limits))) {
			return(RuleError::STRING_TOO_LONG);
		}
	}
	return(RuleError::NONE);
}


RuleError Check_Limits(Frame const & record, LimitsType const & limits)
{
	if (record.Events.size() > limits.EventsPerRecord) {
		return(RuleError::TOO_MANY_EVENTS);
	}
	for (Event const & event : record.Events) {
		if (event.Payload.size() > limits.EventPayload
			|| (event.SealedPayload && event.SealedPayload->size() > limits.EventPayload)) {
			return(RuleError::PAYLOAD_TOO_LARGE);
		}
	}
	for (PlayerMessage const & message : record.Messages) {
		if (!Fits(message.Sender, limits) || !Fits(message.Text, limits)) {
			return(RuleError::STRING_TOO_LONG);
		}
	}
	return(RuleError::NONE);
}


RuleError Check_Limits(Statistics const & record, LimitsType const & limits)
{
	if (record.Houses.size() > limits.Players) {
		return(RuleError::TOO_MANY_HOUSES);
	}
	for (ObjectType const & type : record.Types) {
		if (!Fits(type.Id, limits) || !Fits(type.Name, limits) || !Fits(type.Icon, limits)) {
			return(RuleError::STRING_TOO_LONG);
		}
	}
	for (HouseStatistics const & house : record.Houses) {
		if (!Fits(house.Name, limits) || !Fits(house.CountryName, limits)) {
			return(RuleError::STRING_TOO_LONG);
		}
		for (CrateCount const & crate : house.Crates) {
			if (!Fits(crate.Id, limits)) {
				return(RuleError::STRING_TOO_LONG);
			}
		}
	}
	return(RuleError::NONE);
}


RuleError Check_Order(RecordingInfo const & record)
{
	for (std::size_t i = 1; i < record.Colors.size(); i++) {
		if (record.Colors[i].Number <= record.Colors[i - 1].Number) {
			return(RuleError::NOT_ASCENDING);
		}
	}
	return(RuleError::NONE);
}


RuleError Check_Order(Launch const & record)
{
	for (std::size_t i = 0; i < record.Players.size(); i++) {
		if (i > 0 && record.Players[i].House <= record.Players[i - 1].House) {
			return(RuleError::NOT_ASCENDING);
		}
		if (!Allies_In_Order(record.Players[i].Allies, record.Players[i].House)) {
			return(RuleError::NOT_ASCENDING);
		}
	}
	for (std::size_t i = 1; i < record.Options.size(); i++) {
		if (!(record.Options[i - 1].Key < record.Options[i].Key)) {
			return(RuleError::NOT_ASCENDING);
		}
	}
	return(RuleError::NONE);
}


RuleError Check_Order(Frame const & record)
{
	if (!Ascending(record.Defeated)) {
		return(RuleError::NOT_ASCENDING);
	}
	for (std::size_t i = 1; i < record.Seals.size(); i++) {
		if (record.Seals[i].Through <= record.Seals[i - 1].Through) {
			return(RuleError::NOT_ASCENDING);
		}
	}
	return(RuleError::NONE);
}


RuleError Check_Order(Statistics const & record)
{
	for (HouseStatistics const & house : record.Houses) {
		if (!Allies_In_Order(house.Allies, house.House)) {
			return(RuleError::NOT_ASCENDING);
		}
	}
	return(RuleError::NONE);
}


RuleError Check_Income_Kinds(RecordingInfo const & record)
{
	std::set<std::uint32_t> numbers;
	std::set<std::string_view> ids;
	for (IncomeKind const & kind : record.IncomeKinds) {
		if (kind.Id.empty() || !numbers.insert(kind.Number).second || !ids.insert(kind.Id).second) {
			return(RuleError::INCOME_KIND);
		}
	}
	return(RuleError::NONE);
}


IncomeKindSet::IncomeKindSet(RecordingInfo const & info)
{
	for (IncomeKind const & kind : info.IncomeKinds) {
		Numbers.insert(kind.Number);
	}
}


RuleError IncomeKindSet::Check(Frame const & record) const
{
	for (Income const & income : record.Income) {
		if (Numbers.count(income.Kind) == 0) {
			return(RuleError::INCOME_KIND);
		}
	}
	return(RuleError::NONE);
}


/// <summary>
/// Returns text as valid UTF-8 of at most limit bytes, cut between characters. Each maximal
/// invalid subpart becomes U+FFFD, as Unicode recommends, because every implementation of the
/// format must clean a string to the same bytes.
/// </summary>
std::string Clean_Text(std::string_view text, std::size_t limit)
{
	static char const replacement[] = "\xEF\xBF\xBD";

	std::string clean;
	clean.reserve(text.size());
	std::size_t i = 0;
	while (i < text.size()) {
		int subpart = 0;
		int const length = UTF8::Sequence_At(text.substr(i), subpart);
		if (length > 0) {
			clean.append(text.substr(i, std::size_t(length)));
		} else {
			clean += replacement;
		}
		i += std::size_t(subpart);
	}

	if (clean.size() > limit) {
		std::size_t cut = limit;
		while (cut > 0 && UTF8::Is_Continuation(static_cast<unsigned char>(clean[cut]))) {
			cut--;
		}
		clean.resize(cut);
	}
	return(clean);
}

}
