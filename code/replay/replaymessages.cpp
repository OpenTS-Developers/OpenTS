/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "replay/replaymessages.h"

#include "proto/messagecodec.h"
#include "utf8.h"

#include "replay/replayfields.hh"

#include <algorithm>
#include <utility>


namespace Replay::Message
{

using ProtoMessage::DecodeContext;
using ProtoMessage::Emitter;
using ProtoMessage::Fields;
using ProtoMessage::Widen;

bool Read_Message(ByteSpan data, FileHash & message, DecodeContext * context);
bool Read_Message(ByteSpan data, PlayerColor & message, DecodeContext * context);
bool Read_Message(ByteSpan data, RecordingInfo & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Player & message, DecodeContext * context);
bool Read_Message(ByteSpan data, GameOption & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Launch & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Event & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Point & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Selection & message, DecodeContext * context);
bool Read_Message(ByteSpan data, ViewState & message, DecodeContext * context);
bool Read_Message(ByteSpan data, PlayerMessage & message, DecodeContext * context);
bool Read_Message(ByteSpan data, HouseSample & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Income & message, DecodeContext * context);
bool Read_Message(ByteSpan data, IncomeKind & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Seal & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Frame & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Checkpoint & message, DecodeContext * context);
bool Read_Message(ByteSpan data, ObjectType & message, DecodeContext * context);
bool Read_Message(ByteSpan data, TypeCount & message, DecodeContext * context);
bool Read_Message(ByteSpan data, CrateCount & message, DecodeContext * context);
bool Read_Message(ByteSpan data, HouseKills & message, DecodeContext * context);
bool Read_Message(ByteSpan data, HouseStatistics & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Statistics & message, DecodeContext * context);
bool Read_Message(ByteSpan data, RecordingSummary & message, DecodeContext * context);

namespace F = Field;

namespace
{

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


void Encode(FileHash const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.String(F::FileHash::PATH, message.Path);
	emit.String(F::FileHash::HASH, message.Hash);
}


void Encode(PlayerColor const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Uint(F::PlayerColor::NUMBER, message.Number);
	emit.Fixed32(F::PlayerColor::RGB, message.Rgb);
}


void Encode(RecordingInfo const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.String(F::RecordingInfo::ENGINE, message.Engine);
	emit.String(F::RecordingInfo::ENGINE_VERSION, message.EngineVersion);
	emit.Uint(F::RecordingInfo::DATA_VERSION, message.DataVersion);
	emit.String(F::RecordingInfo::PACKAGE, message.Package);
	emit.String(F::RecordingInfo::PACKAGE_VERSION, message.PackageVersion);
	emit.Repeated(F::RecordingInfo::FILES, message.Files);
	emit.Uint(F::RecordingInfo::RECORDED_AT, message.RecordedAt);
	emit.Optional_Uint(F::RecordingInfo::RECORDER, Widen(message.Recorder));
	emit.String(F::RecordingInfo::MAP_NAME, message.MapName);
	emit.String(F::RecordingInfo::MAP_PATH, message.MapPath);
	emit.Blob(F::RecordingInfo::MAP_SHA256, message.MapSha256);
	emit.String(F::RecordingInfo::GAME_MODE, message.GameMode);
	emit.Uint(F::RecordingInfo::SEAL_VERSION, message.SealVersion);
	emit.Repeated(F::RecordingInfo::COLORS, message.Colors);
	emit.Repeated(F::RecordingInfo::INCOME_KINDS, message.IncomeKinds);
}


void Encode(Player const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Uint(F::Player::HOUSE, message.House);
	emit.String(F::Player::NAME, message.Name);
	emit.Bool(F::Player::HUMAN, message.Human);
	emit.Bool(F::Player::OBSERVER, message.Observer);
	emit.Sint32(F::Player::COUNTRY, message.Country);
	emit.String(F::Player::COUNTRY_NAME, message.CountryName);
	emit.Sint32(F::Player::COLOR, message.Color);
	emit.Sint32(F::Player::START, message.Start);
	emit.Optional_Sint32(F::Player::AI_LEVEL, message.AILevel);
	emit.Packed_Uint32(F::Player::ALLIES, message.Allies);
	emit.Optional_Sint32(F::Player::RESOLVED_COUNTRY, message.ResolvedCountry);
	emit.Optional_Sint32(F::Player::RESOLVED_COLOR, message.ResolvedColor);
	emit.Optional_Sint32(F::Player::RESOLVED_START, message.ResolvedStart);
}


void Encode(GameOption const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.String(F::GameOption::KEY, message.Key);
	if (auto number = std::get_if<std::int64_t>(&message.Value)) {
		emit.Present_Sint64(F::GameOption::NUMBER, *number);
	} else if (auto flag = std::get_if<bool>(&message.Value)) {
		emit.Present_Bool(F::GameOption::FLAG, *flag);
	} else if (auto text = std::get_if<std::string>(&message.Value)) {
		emit.Present_String(F::GameOption::TEXT, *text);
	}
}


void Encode(Launch const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Int32(F::Launch::KIND, std::int32_t(message.Kind));
	emit.Optional_Fixed32(F::Launch::SEED, message.Seed);
	emit.Repeated(F::Launch::PLAYERS, message.Players);
	emit.Repeated(F::Launch::OPTIONS, message.Options);
	emit.Bool(F::Launch::ANONYMOUS, message.Anonymous);
}


void Encode(Event const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Uint(F::Event::TYPE, message.Type);
	emit.Uint(F::Event::HOUSE, message.House);
	emit.Sint32(F::Event::DUE, message.Due);
	emit.Blob(F::Event::PAYLOAD, message.Payload);
	emit.Bool(F::Event::LOCAL, message.Local);
	emit.Int32(F::Event::ROLE, std::int32_t(message.Role));
	if (message.SealedPayload) {
		emit.Present_Blob(F::Event::SEALED_PAYLOAD, *message.SealedPayload);
	}
}


void Encode(Point const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Sint32(F::Point::X, message.X);
	emit.Sint32(F::Point::Y, message.Y);
}


void Encode(Selection const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Packed_Uint32(F::Selection::IDS, message.Ids);
}


void Encode(ViewState const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Present_Message(F::ViewState::CAMERA, message.Camera);
	emit.Present_Message(F::ViewState::SELECTION, message.Selection);
}


void Encode(PlayerMessage const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Uint(F::PlayerMessage::HOUSE, message.House);
	emit.String(F::PlayerMessage::SENDER, message.Sender);
	emit.String(F::PlayerMessage::TEXT, message.Text);
	emit.Sint32(F::PlayerMessage::COLOR, message.Color);
	emit.Int32(F::PlayerMessage::SCOPE, std::int32_t(message.Scope));
	emit.Packed_Uint32(F::PlayerMessage::RECIPIENTS, message.Recipients);
	emit.Blob(F::PlayerMessage::SALT, message.Salt);
}


void Encode(HouseSample const & message, Bytes & out)
{
	namespace S = F::HouseSample;
	Emitter emit(out, message.Unknown);
	emit.Uint(S::HOUSE, message.House);
	emit.Optional_Sint64(S::CREDITS, message.Credits);
	emit.Optional_Sint64(S::STORED_VALUE, message.StoredValue);
	emit.Optional_Sint64(S::SPENT, message.Spent);
	emit.Optional_Sint64(S::EARNED, message.Earned);
	emit.Optional_Sint32(S::POWER_OUTPUT, message.PowerOutput);
	emit.Optional_Sint32(S::POWER_DRAIN, message.PowerDrain);
	emit.Optional_Uint(S::VEHICLES, Widen(message.Vehicles));
	emit.Optional_Uint(S::INFANTRY, Widen(message.Infantry));
	emit.Optional_Uint(S::AIRCRAFT, Widen(message.Aircraft));
	emit.Optional_Uint(S::BUILDINGS, Widen(message.Buildings));
	emit.Optional_Sint64(S::ARMY_VALUE, message.ArmyValue);
	emit.Optional_Sint64(S::BUILDING_VALUE, message.BuildingValue);
	emit.Optional_Uint(S::UNITS_KILLED, Widen(message.UnitsKilled));
	emit.Optional_Uint(S::BUILDINGS_KILLED, Widen(message.BuildingsKilled));
	emit.Optional_Uint(S::UNITS_LOST, Widen(message.UnitsLost));
	emit.Optional_Uint(S::BUILDINGS_LOST, Widen(message.BuildingsLost));
	emit.Optional_Uint(S::UNITS_BUILT, Widen(message.UnitsBuilt));
	emit.Optional_Uint(S::BUILDINGS_BUILT, Widen(message.BuildingsBuilt));
	emit.Optional_Sint64(S::SCORE, message.Score);
	emit.Bool(S::DEFEATED, message.Defeated);
}


void Encode(Income const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Uint(F::Income::HOUSE, message.House);
	emit.Uint(F::Income::KIND, message.Kind);
	emit.Sint64(F::Income::AMOUNT, message.Amount);
}


void Encode(IncomeKind const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Uint(F::IncomeKind::NUMBER, message.Number);
	emit.String(F::IncomeKind::ID, message.Id);
}


void Encode(Seal const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Uint(F::Seal::THROUGH, message.Through);
	emit.Blob(F::Seal::HASH, message.Hash);
}


void Encode(Frame const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Uint(F::Frame::SKIPPED, message.Skipped);
	emit.Optional_Fixed32(F::Frame::HASH, message.Hash);
	emit.Repeated(F::Frame::EVENTS, message.Events);
	emit.Message(F::Frame::VIEW, message.View);
	emit.Repeated(F::Frame::MESSAGES, message.Messages);
	emit.Optional_Uint(F::Frame::FRAME_MICROS, Widen(message.FrameMicros));
	emit.Repeated(F::Frame::SAMPLES, message.Samples);
	emit.Repeated(F::Frame::INCOME, message.Income);
	emit.Repeated(F::Frame::SEALS, message.Seals);
	emit.Packed_Uint32(F::Frame::DEFEATED, message.Defeated);
}


void Encode(Checkpoint const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Uint(F::Checkpoint::FRAME, message.Frame);
	emit.Fixed32(F::Checkpoint::CRC32, message.Crc32);
	emit.Blob(F::Checkpoint::STATE, message.State);
}


void Encode(ObjectType const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Int32(F::ObjectType::CATEGORY, std::int32_t(message.Category));
	emit.Uint(F::ObjectType::INDEX, message.Index);
	emit.String(F::ObjectType::ID, message.Id);
	emit.String(F::ObjectType::NAME, message.Name);
	emit.Sint32(F::ObjectType::COST, message.Cost);
	emit.String(F::ObjectType::ICON, message.Icon);
	emit.Bool(F::ObjectType::NAVAL, message.Naval);
	emit.Bool(F::ObjectType::UNSCORED, message.Unscored);
	emit.Bool(F::ObjectType::INSIGNIFICANT, message.Insignificant);
}


void Encode(TypeCount const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Int32(F::TypeCount::KIND, std::int32_t(message.Kind));
	emit.Int32(F::TypeCount::CATEGORY, std::int32_t(message.Category));
	emit.Uint(F::TypeCount::INDEX, message.Index);
	emit.Uint(F::TypeCount::COUNT, message.Count);
}


void Encode(CrateCount const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.String(F::CrateCount::ID, message.Id);
	emit.Uint(F::CrateCount::COUNT, message.Count);
}


void Encode(HouseKills const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Uint(F::HouseKills::HOUSE, message.House);
	emit.Uint(F::HouseKills::UNITS, message.Units);
	emit.Uint(F::HouseKills::BUILDINGS, message.Buildings);
}


void Encode(HouseStatistics const & message, Bytes & out)
{
	namespace H = F::HouseStatistics;
	Emitter emit(out, message.Unknown);
	emit.Uint(H::HOUSE, message.House);
	emit.String(H::NAME, message.Name);
	emit.String(H::COUNTRY_NAME, message.CountryName);
	emit.Sint32(H::COLOR, message.Color);
	emit.Sint32(H::START, message.Start);
	emit.Packed_Uint32(H::ALLIES, message.Allies);
	emit.Int32(H::OUTCOME, std::int32_t(message.Outcome));
	emit.Int32(H::LEAVE, std::int32_t(message.Leave));
	emit.Message(H::FINAL, message.Final);
	emit.Repeated(H::COUNTS, message.Counts);
	emit.Repeated(H::KILLS, message.Kills);
	emit.Repeated(H::CRATES, message.Crates);
}


void Encode(Statistics const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Repeated(F::Statistics::TYPES, message.Types);
	emit.Repeated(F::Statistics::HOUSES, message.Houses);
}


void Encode(RecordingSummary const & message, Bytes & out)
{
	namespace R = F::RecordingSummary;
	Emitter emit(out, message.Unknown);
	emit.Optional_Uint(R::LAST_FRAME, Widen(message.LastFrame));
	emit.Uint(R::ENDED_AT, message.EndedAt);
	emit.Optional_Uint(R::DESYNC_FRAME, Widen(message.DesyncFrame));
	emit.Bool(R::COMPLETED, message.Completed);
	emit.Repeated(R::SEALS, message.Seals);
	emit.Optional_Uint(R::ELAPSED_MICROS, message.ElapsedMicros);
	emit.Optional_Uint(R::GAME_MICROS, message.GameMicros);
}


bool Read_Message(ByteSpan data, FileHash & message, DecodeContext * context)
{
	message = FileHash();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::FileHash::PATH: fields.Text(message.Path); break;
			case F::FileHash::HASH: fields.Text(message.Hash); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, PlayerColor & message, DecodeContext * context)
{
	message = PlayerColor();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::PlayerColor::NUMBER: fields.Uint32(message.Number); break;
			case F::PlayerColor::RGB: fields.Fixed32(message.Rgb); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, RecordingInfo & message, DecodeContext * context)
{
	namespace I = F::RecordingInfo;
	message = RecordingInfo();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case I::ENGINE: fields.Text(message.Engine); break;
			case I::ENGINE_VERSION: fields.Text(message.EngineVersion); break;
			case I::DATA_VERSION: fields.Uint32(message.DataVersion); break;
			case I::PACKAGE: fields.Text(message.Package); break;
			case I::PACKAGE_VERSION: fields.Text(message.PackageVersion); break;
			case I::FILES: fields.Repeated(message.Files); break;
			case I::RECORDED_AT: fields.Uint64(message.RecordedAt); break;
			case I::RECORDER: fields.Optional(message.Recorder, &Fields::Uint32); break;
			case I::MAP_NAME: fields.Text(message.MapName); break;
			case I::MAP_PATH: fields.Text(message.MapPath); break;
			case I::MAP_SHA256: fields.Blob(message.MapSha256); break;
			case I::GAME_MODE: fields.Text(message.GameMode); break;
			case I::SEAL_VERSION: fields.Uint32(message.SealVersion); break;
			case I::COLORS: fields.Repeated(message.Colors); break;
			case I::INCOME_KINDS: fields.Repeated(message.IncomeKinds, &LimitsType::IncomeKinds, RuleError::TOO_MANY_INCOME_KINDS); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Player & message, DecodeContext * context)
{
	namespace P = F::Player;
	message = Player();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case P::HOUSE: fields.Uint32(message.House); break;
			case P::NAME: fields.Text(message.Name); break;
			case P::HUMAN: fields.Bool(message.Human); break;
			case P::OBSERVER: fields.Bool(message.Observer); break;
			case P::COUNTRY: fields.Sint32(message.Country); break;
			case P::COUNTRY_NAME: fields.Text(message.CountryName); break;
			case P::COLOR: fields.Sint32(message.Color); break;
			case P::START: fields.Sint32(message.Start); break;
			case P::AI_LEVEL: fields.Optional(message.AILevel, &Fields::Sint32); break;
			case P::ALLIES: fields.Packed(message.Allies); break;
			case P::RESOLVED_COUNTRY: fields.Optional(message.ResolvedCountry, &Fields::Sint32); break;
			case P::RESOLVED_COLOR: fields.Optional(message.ResolvedColor, &Fields::Sint32); break;
			case P::RESOLVED_START: fields.Optional(message.ResolvedStart, &Fields::Sint32); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, GameOption & message, DecodeContext * context)
{
	message = GameOption();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::GameOption::KEY:
				fields.Text(message.Key);
				break;

			case F::GameOption::NUMBER:
			{
				std::int64_t number = 0;
				if (fields.Sint64(number)) {
					message.Value = number;
				}
				break;
			}

			case F::GameOption::FLAG:
			{
				bool flag = false;
				if (fields.Bool(flag)) {
					message.Value = flag;
				}
				break;
			}

			case F::GameOption::TEXT:
			{
				std::string text;
				if (fields.Text(text)) {
					message.Value = std::move(text);
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


bool Read_Message(ByteSpan data, Launch & message, DecodeContext * context)
{
	message = Launch();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Launch::KIND: fields.Enum(message.Kind); break;
			case F::Launch::SEED: fields.Optional(message.Seed, &Fields::Fixed32); break;
			case F::Launch::PLAYERS: fields.Repeated(message.Players, &LimitsType::Players, RuleError::TOO_MANY_PLAYERS); break;
			case F::Launch::OPTIONS: fields.Repeated(message.Options); break;
			case F::Launch::ANONYMOUS: fields.Bool(message.Anonymous); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Event & message, DecodeContext * context)
{
	message = Event();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Event::TYPE: fields.Uint32(message.Type); break;
			case F::Event::HOUSE: fields.Uint32(message.House); break;
			case F::Event::DUE: fields.Sint32(message.Due); break;
			case F::Event::PAYLOAD: fields.Payload(message.Payload); break;
			case F::Event::LOCAL: fields.Bool(message.Local); break;
			case F::Event::ROLE: fields.Enum(message.Role); break;
			case F::Event::SEALED_PAYLOAD: fields.Optional(message.SealedPayload, &Fields::Payload); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Point & message, DecodeContext * context)
{
	message = Point();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Point::X: fields.Sint32(message.X); break;
			case F::Point::Y: fields.Sint32(message.Y); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Selection & message, DecodeContext * context)
{
	message = Selection();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Selection::IDS: fields.Packed(message.Ids); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, ViewState & message, DecodeContext * context)
{
	message = ViewState();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::ViewState::CAMERA: fields.Optional_Message(message.Camera); break;
			case F::ViewState::SELECTION: fields.Optional_Message(message.Selection); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, PlayerMessage & message, DecodeContext * context)
{
	namespace M = F::PlayerMessage;
	message = PlayerMessage();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case M::HOUSE: fields.Uint32(message.House); break;
			case M::SENDER: fields.Text(message.Sender); break;
			case M::TEXT: fields.Text(message.Text); break;
			case M::COLOR: fields.Sint32(message.Color); break;
			case M::SCOPE: fields.Enum(message.Scope); break;
			case M::RECIPIENTS: fields.Packed(message.Recipients); break;
			case M::SALT: fields.Blob(message.Salt); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, HouseSample & message, DecodeContext * context)
{
	namespace S = F::HouseSample;
	message = HouseSample();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case S::HOUSE: fields.Uint32(message.House); break;
			case S::CREDITS: fields.Optional(message.Credits, &Fields::Sint64); break;
			case S::STORED_VALUE: fields.Optional(message.StoredValue, &Fields::Sint64); break;
			case S::SPENT: fields.Optional(message.Spent, &Fields::Sint64); break;
			case S::EARNED: fields.Optional(message.Earned, &Fields::Sint64); break;
			case S::POWER_OUTPUT: fields.Optional(message.PowerOutput, &Fields::Sint32); break;
			case S::POWER_DRAIN: fields.Optional(message.PowerDrain, &Fields::Sint32); break;
			case S::VEHICLES: fields.Optional(message.Vehicles, &Fields::Uint32); break;
			case S::INFANTRY: fields.Optional(message.Infantry, &Fields::Uint32); break;
			case S::AIRCRAFT: fields.Optional(message.Aircraft, &Fields::Uint32); break;
			case S::BUILDINGS: fields.Optional(message.Buildings, &Fields::Uint32); break;
			case S::ARMY_VALUE: fields.Optional(message.ArmyValue, &Fields::Sint64); break;
			case S::BUILDING_VALUE: fields.Optional(message.BuildingValue, &Fields::Sint64); break;
			case S::UNITS_KILLED: fields.Optional(message.UnitsKilled, &Fields::Uint32); break;
			case S::BUILDINGS_KILLED: fields.Optional(message.BuildingsKilled, &Fields::Uint32); break;
			case S::UNITS_LOST: fields.Optional(message.UnitsLost, &Fields::Uint32); break;
			case S::BUILDINGS_LOST: fields.Optional(message.BuildingsLost, &Fields::Uint32); break;
			case S::UNITS_BUILT: fields.Optional(message.UnitsBuilt, &Fields::Uint32); break;
			case S::BUILDINGS_BUILT: fields.Optional(message.BuildingsBuilt, &Fields::Uint32); break;
			case S::SCORE: fields.Optional(message.Score, &Fields::Sint64); break;
			case S::DEFEATED: fields.Bool(message.Defeated); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Income & message, DecodeContext * context)
{
	message = Income();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Income::HOUSE: fields.Uint32(message.House); break;
			case F::Income::KIND: fields.Uint32(message.Kind); break;
			case F::Income::AMOUNT: fields.Sint64(message.Amount); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, IncomeKind & message, DecodeContext * context)
{
	message = IncomeKind();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::IncomeKind::NUMBER: fields.Uint32(message.Number); break;
			case F::IncomeKind::ID: fields.Text(message.Id); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Seal & message, DecodeContext * context)
{
	message = Seal();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Seal::THROUGH: fields.Uint32(message.Through); break;
			case F::Seal::HASH: fields.Blob(message.Hash); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Frame & message, DecodeContext * context)
{
	message = Frame();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Frame::SKIPPED: fields.Uint32(message.Skipped); break;
			case F::Frame::HASH: fields.Optional(message.Hash, &Fields::Fixed32); break;
			case F::Frame::EVENTS: fields.Repeated(message.Events, &LimitsType::EventsPerRecord, RuleError::TOO_MANY_EVENTS); break;
			case F::Frame::VIEW: fields.Message(message.View); break;
			case F::Frame::MESSAGES: fields.Repeated(message.Messages); break;
			case F::Frame::FRAME_MICROS: fields.Optional(message.FrameMicros, &Fields::Uint32); break;
			case F::Frame::SAMPLES: fields.Repeated(message.Samples); break;
			case F::Frame::INCOME: fields.Repeated(message.Income); break;
			case F::Frame::SEALS: fields.Repeated(message.Seals); break;
			case F::Frame::DEFEATED: fields.Packed(message.Defeated); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Checkpoint & message, DecodeContext * context)
{
	message = Checkpoint();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Checkpoint::FRAME: fields.Uint32(message.Frame); break;
			case F::Checkpoint::CRC32: fields.Fixed32(message.Crc32); break;
			case F::Checkpoint::STATE: fields.Blob(message.State); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, ObjectType & message, DecodeContext * context)
{
	namespace O = F::ObjectType;
	message = ObjectType();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case O::CATEGORY: fields.Enum(message.Category); break;
			case O::INDEX: fields.Uint32(message.Index); break;
			case O::ID: fields.Text(message.Id); break;
			case O::NAME: fields.Text(message.Name); break;
			case O::COST: fields.Sint32(message.Cost); break;
			case O::ICON: fields.Text(message.Icon); break;
			case O::NAVAL: fields.Bool(message.Naval); break;
			case O::UNSCORED: fields.Bool(message.Unscored); break;
			case O::INSIGNIFICANT: fields.Bool(message.Insignificant); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, TypeCount & message, DecodeContext * context)
{
	message = TypeCount();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::TypeCount::KIND: fields.Enum(message.Kind); break;
			case F::TypeCount::CATEGORY: fields.Enum(message.Category); break;
			case F::TypeCount::INDEX: fields.Uint32(message.Index); break;
			case F::TypeCount::COUNT: fields.Uint32(message.Count); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, CrateCount & message, DecodeContext * context)
{
	message = CrateCount();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::CrateCount::ID: fields.Text(message.Id); break;
			case F::CrateCount::COUNT: fields.Uint32(message.Count); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, HouseKills & message, DecodeContext * context)
{
	message = HouseKills();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::HouseKills::HOUSE: fields.Uint32(message.House); break;
			case F::HouseKills::UNITS: fields.Uint32(message.Units); break;
			case F::HouseKills::BUILDINGS: fields.Uint32(message.Buildings); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, HouseStatistics & message, DecodeContext * context)
{
	namespace H = F::HouseStatistics;
	message = HouseStatistics();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case H::HOUSE: fields.Uint32(message.House); break;
			case H::NAME: fields.Text(message.Name); break;
			case H::COUNTRY_NAME: fields.Text(message.CountryName); break;
			case H::COLOR: fields.Sint32(message.Color); break;
			case H::START: fields.Sint32(message.Start); break;
			case H::ALLIES: fields.Packed(message.Allies); break;
			case H::OUTCOME: fields.Enum(message.Outcome); break;
			case H::LEAVE: fields.Enum(message.Leave); break;
			case H::FINAL: fields.Message(message.Final); break;
			case H::COUNTS: fields.Repeated(message.Counts); break;
			case H::KILLS: fields.Repeated(message.Kills); break;
			case H::CRATES: fields.Repeated(message.Crates); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Statistics & message, DecodeContext * context)
{
	message = Statistics();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case F::Statistics::TYPES: fields.Repeated(message.Types); break;
			case F::Statistics::HOUSES: fields.Repeated(message.Houses, &LimitsType::Players, RuleError::TOO_MANY_HOUSES); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, RecordingSummary & message, DecodeContext * context)
{
	namespace R = F::RecordingSummary;
	message = RecordingSummary();
	Fields fields(data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case R::LAST_FRAME: fields.Optional(message.LastFrame, &Fields::Uint32); break;
			case R::ENDED_AT: fields.Uint64(message.EndedAt); break;
			case R::DESYNC_FRAME: fields.Optional(message.DesyncFrame, &Fields::Uint32); break;
			case R::COMPLETED: fields.Bool(message.Completed); break;
			case R::SEALS: fields.Repeated(message.Seals); break;
			case R::ELAPSED_MICROS: fields.Optional(message.ElapsedMicros, &Fields::Uint64); break;
			case R::GAME_MICROS: fields.Optional(message.GameMicros, &Fields::Uint64); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Decode(ByteSpan data, FileHash & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, PlayerColor & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, RecordingInfo & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, Player & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, GameOption & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, Launch & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, Event & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, Point & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, Selection & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, ViewState & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, PlayerMessage & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, HouseSample & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, Income & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, IncomeKind & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, Seal & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, Frame & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, Checkpoint & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, ObjectType & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, TypeCount & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, CrateCount & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, HouseKills & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, HouseStatistics & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, Statistics & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, RecordingSummary & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, RecordingInfo & message, LimitsType const & limits, RuleError & broken)
{
	DecodeContext context{ limits };
	bool const good = Read_Message(data, message, &context);
	broken = context.Broken;
	return(good);
}


bool Decode(ByteSpan data, Launch & message, LimitsType const & limits, RuleError & broken)
{
	DecodeContext context{ limits };
	bool const good = Read_Message(data, message, &context);
	broken = context.Broken;
	return(good);
}


bool Decode(ByteSpan data, Frame & message, LimitsType const & limits, RuleError & broken)
{
	DecodeContext context{ limits };
	bool const good = Read_Message(data, message, &context);
	broken = context.Broken;
	return(good);
}


bool Decode(ByteSpan data, Statistics & message, LimitsType const & limits, RuleError & broken)
{
	DecodeContext context{ limits };
	bool const good = Read_Message(data, message, &context);
	broken = context.Broken;
	return(good);
}


RuleError Check_Limits(RecordingInfo const & message, LimitsType const & limits)
{
	for (std::string const * text : { &message.Engine, &message.EngineVersion, &message.Package,
		&message.PackageVersion, &message.MapName, &message.GameMode, &message.MapPath }) {
		if (!Fits(*text, limits)) {
			return(RuleError::STRING_TOO_LONG);
		}
	}
	for (FileHash const & file : message.Files) {
		if (!Fits(file.Path, limits) || !Fits(file.Hash, limits)) {
			return(RuleError::STRING_TOO_LONG);
		}
	}
	if (message.IncomeKinds.size() > limits.IncomeKinds) {
		return(RuleError::TOO_MANY_INCOME_KINDS);
	}
	for (IncomeKind const & kind : message.IncomeKinds) {
		if (!Fits(kind.Id, limits)) {
			return(RuleError::STRING_TOO_LONG);
		}
	}
	return(RuleError::NONE);
}


RuleError Check_Limits(Launch const & message, LimitsType const & limits)
{
	if (message.Players.size() > limits.Players) {
		return(RuleError::TOO_MANY_PLAYERS);
	}
	for (Player const & player : message.Players) {
		if (!Fits(player.Name, limits) || !Fits(player.CountryName, limits)) {
			return(RuleError::STRING_TOO_LONG);
		}
	}
	for (GameOption const & option : message.Options) {
		auto text = std::get_if<std::string>(&option.Value);
		if (!Fits(option.Key, limits) || (text != nullptr && !Fits(*text, limits))) {
			return(RuleError::STRING_TOO_LONG);
		}
	}
	return(RuleError::NONE);
}


RuleError Check_Limits(Frame const & message, LimitsType const & limits)
{
	if (message.Events.size() > limits.EventsPerRecord) {
		return(RuleError::TOO_MANY_EVENTS);
	}
	for (Event const & event : message.Events) {
		if (event.Payload.size() > limits.EventPayload
			|| (event.SealedPayload && event.SealedPayload->size() > limits.EventPayload)) {
			return(RuleError::PAYLOAD_TOO_LARGE);
		}
	}
	for (PlayerMessage const & chat : message.Messages) {
		if (!Fits(chat.Sender, limits) || !Fits(chat.Text, limits)) {
			return(RuleError::STRING_TOO_LONG);
		}
	}
	return(RuleError::NONE);
}


RuleError Check_Limits(Statistics const & message, LimitsType const & limits)
{
	if (message.Houses.size() > limits.Players) {
		return(RuleError::TOO_MANY_HOUSES);
	}
	for (ObjectType const & type : message.Types) {
		if (!Fits(type.Id, limits) || !Fits(type.Name, limits) || !Fits(type.Icon, limits)) {
			return(RuleError::STRING_TOO_LONG);
		}
	}
	for (HouseStatistics const & house : message.Houses) {
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


RuleError Check_Order(RecordingInfo const & message)
{
	for (std::size_t i = 1; i < message.Colors.size(); i++) {
		if (message.Colors[i].Number <= message.Colors[i - 1].Number) {
			return(RuleError::NOT_ASCENDING);
		}
	}
	return(RuleError::NONE);
}


RuleError Check_Order(Launch const & message)
{
	for (std::size_t i = 0; i < message.Players.size(); i++) {
		if (i > 0 && message.Players[i].House <= message.Players[i - 1].House) {
			return(RuleError::NOT_ASCENDING);
		}
		if (!Allies_In_Order(message.Players[i].Allies, message.Players[i].House)) {
			return(RuleError::NOT_ASCENDING);
		}
	}
	for (std::size_t i = 1; i < message.Options.size(); i++) {
		if (!(message.Options[i - 1].Key < message.Options[i].Key)) {
			return(RuleError::NOT_ASCENDING);
		}
	}
	return(RuleError::NONE);
}


RuleError Check_Order(Frame const & message)
{
	if (!Ascending(message.Defeated)) {
		return(RuleError::NOT_ASCENDING);
	}
	for (std::size_t i = 1; i < message.Seals.size(); i++) {
		if (message.Seals[i].Through <= message.Seals[i - 1].Through) {
			return(RuleError::NOT_ASCENDING);
		}
	}
	return(RuleError::NONE);
}


RuleError Check_Order(Statistics const & message)
{
	for (HouseStatistics const & house : message.Houses) {
		if (!Allies_In_Order(house.Allies, house.House)) {
			return(RuleError::NOT_ASCENDING);
		}
	}
	return(RuleError::NONE);
}


RuleError Check_Income_Kinds(RecordingInfo const & message)
{
	std::set<std::uint32_t> numbers;
	std::set<std::string_view> ids;
	for (IncomeKind const & kind : message.IncomeKinds) {
		if (kind.Id.empty() || !numbers.insert(kind.Number).second || !ids.insert(kind.Id).second) {
			return(RuleError::INCOME_KIND);
		}
	}
	return(RuleError::NONE);
}

}


namespace Replay
{

IncomeKindSet::IncomeKindSet(Message::RecordingInfo const & info)
{
	for (Message::IncomeKind const & kind : info.IncomeKinds) {
		Numbers.insert(kind.Number);
	}
}


RuleError IncomeKindSet::Check(Message::Frame const & frame) const
{
	for (Message::Income const & income : frame.Income) {
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
