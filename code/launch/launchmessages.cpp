/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "launch/launchmessages.h"

#include "proto/messagecodec.h"
#include "utf8.h"

#include "launch/launchfields.hh"

#include <string_view>
#include <utility>


namespace Launch::Message
{

using ProtoMessage::DecodeContext;
using ProtoMessage::Emitter;
using ProtoMessage::Fields;
using ProtoMessage::Limit;
using ProtoMessage::Widen;
using ProtoMessage::Fits;
using ProtoMessage::Allies_In_Order;
using ProtoMessage::Ascending;
using ProtoWire::WireType;
using ProtoWire::WireError;
using ProtoWire::WireField;
using ProtoWire::WireReader;

namespace L = Field;

bool Read_Message(ByteSpan data, LoadScreen & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Session & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Peer & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Tunnel & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Network & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Recording & message, DecodeContext * context);
bool Read_Message(ByteSpan data, CheckpointCheck & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Fixture & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Play & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Load & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Replay & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Verify & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Capture & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Request & message, DecodeContext * context);
bool Read_Message(ByteSpan data, ScenarioResult & message, DecodeContext * context);
bool Read_Message(ByteSpan data, Result & message, DecodeContext * context);


namespace
{

// Writes a present child as an ordinary message field, which is left out when it encodes empty.
template<typename T>
void Optional_Plain_Message(Emitter & emit, std::uint32_t field, std::optional<T> const & child)
{
	if (child) {
		emit.Message(field, *child);
	}
}


void Optional_Bool(Emitter & emit, std::uint32_t field, std::optional<bool> value)
{
	if (value) {
		emit.Present_Bool(field, *value);
	}
}


template<typename T>
void Verb_Message(Fields & fields, Verb & verb)
{
	T child;
	if (fields.Message(child)) {
		verb = std::move(child);
	}
}


// Reads a string field into text when it is valid UTF-8, and leaves text alone otherwise.
void Salvage_Text(WireField const & field, std::string & text)
{
	if (field.Type != WireType::LENGTH) {
		return;
	}
	std::string_view const value(reinterpret_cast<char const *>(field.Body.data()), field.Body.size());
	if (UTF8::Is_Valid(value)) {
		text.assign(value);
	}
}

}


void Encode(LoadScreen const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.String(L::LoadScreen::IMAGE, message.Image);
	emit.Optional_Sint32(L::LoadScreen::X, message.X);
	emit.Optional_Sint32(L::LoadScreen::Y, message.Y);
}


void Encode(Session const & message, Bytes & out)
{
	namespace S = L::Session;
	Emitter emit(out, message.Unknown);
	emit.Optional_Uint(S::LOCAL_PLAYER, Widen(message.LocalPlayer));
	emit.Bool(S::HOST, message.Host);
	emit.Uint(S::AUTOSAVE_INTERVAL, message.AutosaveInterval);
	emit.Uint(S::NEXT_CAMPAIGN_AUTOSAVE, message.NextCampaignAutosave);
	emit.Uint(S::NEXT_SKIRMISH_AUTOSAVE, message.NextSkirmishAutosave);
	emit.Bool(S::SKIP_SCORE_SCREEN, message.SkipScoreScreen);
	emit.Bool(S::WRITE_STATISTICS, message.WriteStatistics);
	emit.Uint(S::GAME_ID, message.GameId);
	emit.String(S::MAP_HASH, message.MapHash);
	emit.String(S::MAP_NAME, message.MapName);
	emit.String(S::GAME_MODE, message.GameMode);
	emit.Message(S::LOAD_SCREEN, message.LoadScreen);
	emit.String(S::DIFFICULTY_NAME, message.DifficultyName);
}


void Encode(Peer const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Uint(L::Peer::PLAYER, message.Player);
	emit.String(L::Peer::ADDRESS, message.Address);
	emit.Sint32(L::Peer::PORT, message.Port);
}


void Encode(Tunnel const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.String(L::Tunnel::ADDRESS, message.Address);
	emit.Uint(L::Tunnel::PORT, message.Port);
}


void Encode(Network const & message, Bytes & out)
{
	namespace N = L::Network;
	Emitter emit(out, message.Unknown);
	emit.Sint32(N::PORT, message.Port);
	emit.Repeated(N::PEERS, message.Peers);
	Optional_Plain_Message(emit, N::TUNNEL, message.Tunnel);
	emit.Uint(N::PROTOCOL, message.Protocol);
	emit.Uint(N::FRAME_SEND_RATE, message.FrameSendRate);
	emit.Uint(N::MAX_AHEAD, message.MaxAhead);
	emit.Uint(N::CONNECT_TIMEOUT, message.ConnectTimeout);
	emit.Uint(N::RECONNECT_TIMEOUT, message.ReconnectTimeout);
}


void Encode(Recording const & message, Bytes & out)
{
	namespace R = L::Recording;
	Emitter emit(out, message.Unknown);
	Optional_Bool(emit, R::ENABLED, message.Enabled);
	emit.String(R::FILE_OUT, message.FileOut);
	emit.String(R::PACKAGE, message.Package);
	emit.String(R::PACKAGE_VERSION, message.PackageVersion);
	emit.Repeated(R::FILES, message.Files);
	Optional_Bool(emit, R::EMBED_MAP, message.EmbedMap);
}


void Encode(CheckpointCheck const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Uint(L::CheckpointCheck::SAVE_FRAME, message.SaveFrame);
	emit.Uint(L::CheckpointCheck::RUN_FRAMES, message.RunFrames);
}


void Encode(Fixture const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Bool(L::Fixture::HEADLESS, message.Headless);
	emit.Optional_Uint(L::Fixture::STOP_FRAME, Widen(message.StopFrame));
	emit.String(L::Fixture::SCRIPT_PATH, message.ScriptPath);
	Optional_Plain_Message(emit, L::Fixture::CHECKPOINT_CHECK, message.CheckpointCheck);
}


void Encode(Play const & message, Bytes & out)
{
	namespace P = L::Play;
	Emitter emit(out, message.Unknown);
	emit.Message(P::GAME, message.Game);
	emit.String(P::MAP_PATH, message.MapPath);
	emit.Message(P::SESSION, message.Session);
	Optional_Plain_Message(emit, P::NETWORK, message.Network);
	emit.Message(P::RECORDING, message.Recording);
	Optional_Plain_Message(emit, P::FIXTURE, message.Fixture);
}


void Encode(Load const & message, Bytes & out)
{
	namespace O = L::Load;
	Emitter emit(out, message.Unknown);
	emit.String(O::SAVE, message.Save);
	emit.Message(O::SESSION, message.Session);
	Optional_Plain_Message(emit, O::NETWORK, message.Network);
	emit.Repeated(O::PLAYERS, message.Players);
	emit.Optional_Sint32(O::GAME_SPEED, message.GameSpeed);
	emit.Bool(O::ANONYMOUS, message.Anonymous);
}


void Encode(Replay const & message, Bytes & out)
{
	namespace R = L::Replay;
	Emitter emit(out, message.Unknown);
	emit.String(R::FILE, message.File);
	emit.Optional_Uint(R::WATCH_PLAYER, Widen(message.WatchPlayer));
	emit.Bool(R::FREE_VIEW, message.FreeView);
	emit.Uint(R::START_FRAME, message.StartFrame);
	emit.String(R::MAP_PATH, message.MapPath);
	emit.Message(R::LOAD_SCREEN, message.LoadScreen);
}


void Encode(Verify const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.String(L::Verify::FILE, message.File);
	emit.String(L::Verify::OUT_PATH, message.OutPath);
	emit.String(L::Verify::MAP_PATH, message.MapPath);
}


void Encode(Capture const & message, Bytes & out)
{
	namespace C = L::Capture;
	Emitter emit(out, message.Unknown);
	emit.String(C::MAP_PATH, message.MapPath);
	emit.String(C::OUT_PATH, message.OutPath);
	emit.String(C::DESCRIPTION_PATH, message.DescriptionPath);
}


void Encode(Request const & message, Bytes & out)
{
	namespace Q = L::Request;
	Emitter emit(out, message.Unknown);
	emit.Uint(Q::VERSION, message.Version);
	if (auto play = std::get_if<Play>(&message.Verb)) {
		emit.Present_Message(Q::PLAY, Encoded(*play));
	} else if (auto load = std::get_if<Load>(&message.Verb)) {
		emit.Present_Message(Q::LOAD, Encoded(*load));
	} else if (auto replay = std::get_if<Replay>(&message.Verb)) {
		emit.Present_Message(Q::REPLAY, Encoded(*replay));
	} else if (auto verify = std::get_if<Verify>(&message.Verb)) {
		emit.Present_Message(Q::VERIFY, Encoded(*verify));
	} else if (auto capture = std::get_if<Capture>(&message.Verb)) {
		emit.Present_Message(Q::CAPTURE, Encoded(*capture));
	}
	emit.String(Q::RESULT_PATH, message.ResultPath);
	emit.String(Q::CHANNEL, message.Channel);
	emit.String(Q::REQUEST_ID, message.RequestId);
}


void Encode(ScenarioResult const & message, Bytes & out)
{
	namespace S = L::ScenarioResult;
	Emitter emit(out, message.Unknown);
	emit.String(S::MAP_PATH, message.MapPath);
	emit.Int32(S::OUTCOME, std::int32_t(message.Outcome));
	emit.Bool(S::COMPLETED, message.Completed);
	emit.Optional_Uint(S::LAST_FRAME, Widen(message.LastFrame));
	emit.String(S::REPLAY_PATH, message.ReplayPath);
	emit.Int32(S::NOT_RECORDED, std::int32_t(message.NotRecorded));
}


void Encode(Result const & message, Bytes & out)
{
	namespace R = L::Result;
	Emitter emit(out, message.Unknown);
	emit.String(R::REQUEST_ID, message.RequestId);
	emit.Int32(R::CODE, std::int32_t(message.Code));
	emit.String(R::MESSAGE, message.Message);
	emit.Repeated(R::SCENARIOS, message.Scenarios);
	emit.String(R::STATISTICS_PATH, message.StatisticsPath);
	emit.Bool(R::DIFFERENT_BUILD, message.DifferentBuild);
	emit.Bool(R::UNVERIFIED, message.Unverified);
	emit.Optional_Uint(R::MISMATCH_FRAME, Widen(message.MismatchFrame));
	emit.String(R::OUTPUT_PATH, message.OutputPath);
	emit.String(R::CRASH_PATH, message.CrashPath);
}


bool Read_Message(ByteSpan data, LoadScreen & message, DecodeContext * context)
{
	message = LoadScreen();
	Fields fields("LoadScreen", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case L::LoadScreen::IMAGE: fields.Text(message.Image); break;
			case L::LoadScreen::X: fields.Optional(message.X, &Fields::Sint32); break;
			case L::LoadScreen::Y: fields.Optional(message.Y, &Fields::Sint32); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Session & message, DecodeContext * context)
{
	namespace S = L::Session;
	message = Session();
	Fields fields("Session", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case S::LOCAL_PLAYER: fields.Optional(message.LocalPlayer, &Fields::Uint32); break;
			case S::HOST: fields.Bool(message.Host); break;
			case S::AUTOSAVE_INTERVAL: fields.Uint32(message.AutosaveInterval); break;
			case S::NEXT_CAMPAIGN_AUTOSAVE: fields.Uint32(message.NextCampaignAutosave); break;
			case S::NEXT_SKIRMISH_AUTOSAVE: fields.Uint32(message.NextSkirmishAutosave); break;
			case S::SKIP_SCORE_SCREEN: fields.Bool(message.SkipScoreScreen); break;
			case S::WRITE_STATISTICS: fields.Bool(message.WriteStatistics); break;
			case S::GAME_ID: fields.Uint32(message.GameId); break;
			case S::MAP_HASH: fields.Text(message.MapHash); break;
			case S::MAP_NAME: fields.Text(message.MapName); break;
			case S::GAME_MODE: fields.Text(message.GameMode); break;
			case S::LOAD_SCREEN: fields.Message(message.LoadScreen); break;
			case S::DIFFICULTY_NAME: fields.Text(message.DifficultyName); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Peer & message, DecodeContext * context)
{
	message = Peer();
	Fields fields("Peer", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case L::Peer::PLAYER: fields.Uint32(message.Player); break;
			case L::Peer::ADDRESS: fields.Text(message.Address); break;
			case L::Peer::PORT: fields.Sint32(message.Port); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Tunnel & message, DecodeContext * context)
{
	message = Tunnel();
	Fields fields("Tunnel", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case L::Tunnel::ADDRESS: fields.Text(message.Address); break;
			case L::Tunnel::PORT: fields.Uint32(message.Port); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Network & message, DecodeContext * context)
{
	namespace N = L::Network;
	message = Network();
	Fields fields("Network", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case N::PORT: fields.Sint32(message.Port); break;
			case N::PEERS: fields.Repeated(message.Peers, Limit(context, &LimitsType::Players)); break;
			case N::TUNNEL: fields.Optional_Message(message.Tunnel); break;
			case N::PROTOCOL: fields.Uint32(message.Protocol); break;
			case N::FRAME_SEND_RATE: fields.Uint32(message.FrameSendRate); break;
			case N::MAX_AHEAD: fields.Uint32(message.MaxAhead); break;
			case N::CONNECT_TIMEOUT: fields.Uint32(message.ConnectTimeout); break;
			case N::RECONNECT_TIMEOUT: fields.Uint32(message.ReconnectTimeout); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Recording & message, DecodeContext * context)
{
	namespace R = L::Recording;
	message = Recording();
	Fields fields("Recording", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case R::ENABLED: fields.Optional(message.Enabled, &Fields::Bool); break;
			case R::FILE_OUT: fields.Text(message.FileOut); break;
			case R::PACKAGE: fields.Text(message.Package); break;
			case R::PACKAGE_VERSION: fields.Text(message.PackageVersion); break;
			case R::FILES: fields.Repeated(message.Files); break;
			case R::EMBED_MAP: fields.Optional(message.EmbedMap, &Fields::Bool); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, CheckpointCheck & message, DecodeContext * context)
{
	message = CheckpointCheck();
	Fields fields("CheckpointCheck", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case L::CheckpointCheck::SAVE_FRAME: fields.Uint32(message.SaveFrame); break;
			case L::CheckpointCheck::RUN_FRAMES: fields.Uint32(message.RunFrames); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Fixture & message, DecodeContext * context)
{
	message = Fixture();
	Fields fields("Fixture", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case L::Fixture::HEADLESS: fields.Bool(message.Headless); break;
			case L::Fixture::STOP_FRAME: fields.Optional(message.StopFrame, &Fields::Uint32); break;
			case L::Fixture::SCRIPT_PATH: fields.Text(message.ScriptPath); break;
			case L::Fixture::CHECKPOINT_CHECK: fields.Optional_Message(message.CheckpointCheck); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Play & message, DecodeContext * context)
{
	namespace P = L::Play;
	message = Play();
	Fields fields("Play", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case P::GAME: fields.Message(message.Game); break;
			case P::MAP_PATH: fields.Text(message.MapPath); break;
			case P::SESSION: fields.Message(message.Session); break;
			case P::NETWORK: fields.Optional_Message(message.Network); break;
			case P::RECORDING: fields.Message(message.Recording); break;
			case P::FIXTURE: fields.Optional_Message(message.Fixture); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Load & message, DecodeContext * context)
{
	namespace O = L::Load;
	message = Load();
	Fields fields("Load", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case O::SAVE: fields.Text(message.Save); break;
			case O::SESSION: fields.Message(message.Session); break;
			case O::NETWORK: fields.Optional_Message(message.Network); break;
			case O::PLAYERS: fields.Repeated(message.Players, Limit(context, &LimitsType::Players)); break;
			case O::GAME_SPEED: fields.Optional(message.GameSpeed, &Fields::Sint32); break;
			case O::ANONYMOUS: fields.Bool(message.Anonymous); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Replay & message, DecodeContext * context)
{
	namespace R = L::Replay;
	message = Replay();
	Fields fields("Replay", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case R::FILE: fields.Text(message.File); break;
			case R::WATCH_PLAYER: fields.Optional(message.WatchPlayer, &Fields::Uint32); break;
			case R::FREE_VIEW: fields.Bool(message.FreeView); break;
			case R::START_FRAME: fields.Uint32(message.StartFrame); break;
			case R::MAP_PATH: fields.Text(message.MapPath); break;
			case R::LOAD_SCREEN: fields.Message(message.LoadScreen); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Verify & message, DecodeContext * context)
{
	message = Verify();
	Fields fields("Verify", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case L::Verify::FILE: fields.Text(message.File); break;
			case L::Verify::OUT_PATH: fields.Text(message.OutPath); break;
			case L::Verify::MAP_PATH: fields.Text(message.MapPath); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Capture & message, DecodeContext * context)
{
	namespace C = L::Capture;
	message = Capture();
	Fields fields("Capture", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case C::MAP_PATH: fields.Text(message.MapPath); break;
			case C::OUT_PATH: fields.Text(message.OutPath); break;
			case C::DESCRIPTION_PATH: fields.Text(message.DescriptionPath); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Request & message, DecodeContext * context)
{
	namespace Q = L::Request;
	message = Request();
	Fields fields("Request", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case Q::VERSION: fields.Uint32(message.Version); break;
			case Q::PLAY: Verb_Message<Play>(fields, message.Verb); break;
			case Q::LOAD: Verb_Message<Load>(fields, message.Verb); break;
			case Q::REPLAY: Verb_Message<Replay>(fields, message.Verb); break;
			case Q::VERIFY: Verb_Message<Verify>(fields, message.Verb); break;
			case Q::CAPTURE: Verb_Message<Capture>(fields, message.Verb); break;
			case Q::RESULT_PATH: fields.Text(message.ResultPath); break;
			case Q::CHANNEL: fields.Text(message.Channel); break;
			case Q::REQUEST_ID: fields.Text(message.RequestId); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, ScenarioResult & message, DecodeContext * context)
{
	namespace S = L::ScenarioResult;
	message = ScenarioResult();
	Fields fields("ScenarioResult", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case S::MAP_PATH: fields.Text(message.MapPath); break;
			case S::OUTCOME: fields.Enum(message.Outcome); break;
			case S::COMPLETED: fields.Bool(message.Completed); break;
			case S::LAST_FRAME: fields.Optional(message.LastFrame, &Fields::Uint32); break;
			case S::REPLAY_PATH: fields.Text(message.ReplayPath); break;
			case S::NOT_RECORDED: fields.Enum(message.NotRecorded); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Result & message, DecodeContext * context)
{
	namespace R = L::Result;
	message = Result();
	Fields fields("Result", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case R::REQUEST_ID: fields.Text(message.RequestId); break;
			case R::CODE: fields.Enum(message.Code); break;
			case R::MESSAGE: fields.Text(message.Message); break;
			case R::SCENARIOS: fields.Repeated(message.Scenarios); break;
			case R::STATISTICS_PATH: fields.Text(message.StatisticsPath); break;
			case R::DIFFERENT_BUILD: fields.Bool(message.DifferentBuild); break;
			case R::UNVERIFIED: fields.Bool(message.Unverified); break;
			case R::MISMATCH_FRAME: fields.Optional(message.MismatchFrame, &Fields::Uint32); break;
			case R::OUTPUT_PATH: fields.Text(message.OutputPath); break;
			case R::CRASH_PATH: fields.Text(message.CrashPath); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


/// <summary>
/// Decodes a launch request and returns why it cannot be run, or NONE. The version, the result
/// path, and the request id are filled whenever they can be read, even from a refused request,
/// so that the refusal can be reported; one of those strings that is not valid UTF-8 stays
/// empty. When two verbs are set, the last one in the bytes counts.
/// </summary>
RequestFault Decode_Request(ByteSpan data, Request & request)
{
	namespace Q = L::Request;
	request = Request();

	// Read first and alone, because these three fields keep their meaning in every version.
	WireReader reader(data);
	WireField field;
	while (reader.Next(field)) {
		if (field.Number == Q::VERSION && field.Type == WireType::VARINT) {
			request.Version = field.As_Uint32();
		} else if (field.Number == Q::RESULT_PATH) {
			Salvage_Text(field, request.ResultPath);
		} else if (field.Number == Q::REQUEST_ID) {
			Salvage_Text(field, request.RequestId);
		}
	}
	if (reader.Error() != WireError::NONE || data.size() > REQUEST_MAX_SIZE) {
		return(RequestFault::MALFORMED);
	}
	if (request.Version != REQUEST_VERSION) {
		return(RequestFault::VERSION);
	}

	// The request's own size cap bounds every allocation, so only the format's player count
	// limit applies.
	LimitsType limits;
	limits.String = REQUEST_MAX_SIZE;
	DecodeContext context{ limits };
	Request decoded;
	if (!Read_Message(data, decoded, &context)) {
		return(context.BadText ? RequestFault::TEXT : RequestFault::MALFORMED);
	}
	request = std::move(decoded);
	if (std::holds_alternative<std::monostate>(request.Verb)) {
		return(RequestFault::NO_VERB);
	}
	return(RequestFault::NONE);
}


bool Decode(ByteSpan data, Result & message)
{
	return(Read_Message(data, message, nullptr));
}


void Encode(FileHash const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.String(L::FileHash::PATH, message.Path);
	emit.String(L::FileHash::HASH, message.Hash);
}


void Encode(Player const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Uint(L::Player::HOUSE, message.House);
	emit.String(L::Player::NAME, message.Name);
	emit.Bool(L::Player::HUMAN, message.Human);
	emit.Bool(L::Player::OBSERVER, message.Observer);
	emit.Sint32(L::Player::COUNTRY, message.Country);
	emit.String(L::Player::COUNTRY_NAME, message.CountryName);
	emit.Sint32(L::Player::COLOR, message.Color);
	emit.Sint32(L::Player::START, message.Start);
	emit.Optional_Sint32(L::Player::AI_LEVEL, message.AILevel);
	emit.Packed_Uint32(L::Player::ALLIES, message.Allies);
	emit.Optional_Sint32(L::Player::RESOLVED_COUNTRY, message.ResolvedCountry);
	emit.Optional_Sint32(L::Player::RESOLVED_COLOR, message.ResolvedColor);
	emit.Optional_Sint32(L::Player::RESOLVED_START, message.ResolvedStart);
}


void Encode(Option const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.String(L::Option::KEY, message.Key);
	if (auto number = std::get_if<std::int64_t>(&message.Value)) {
		emit.Present_Sint64(L::Option::NUMBER, *number);
	} else if (auto flag = std::get_if<bool>(&message.Value)) {
		emit.Present_Bool(L::Option::FLAG, *flag);
	} else if (auto text = std::get_if<std::string>(&message.Value)) {
		emit.Present_String(L::Option::TEXT, *text);
	}
}


void Encode(Game const & message, Bytes & out)
{
	Emitter emit(out, message.Unknown);
	emit.Int32(L::Game::KIND, std::int32_t(message.Kind));
	emit.Optional_Fixed32(L::Game::SEED, message.Seed);
	emit.Repeated(L::Game::PLAYERS, message.Players);
	emit.Repeated(L::Game::OPTIONS, message.Options);
	emit.Bool(L::Game::ANONYMOUS, message.Anonymous);
	emit.Packed_Uint32(L::Game::GLOBAL_FLAGS, message.GlobalFlags);
}


bool Read_Message(ByteSpan data, FileHash & message, DecodeContext * context)
{
	message = FileHash();
	Fields fields("FileHash", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case L::FileHash::PATH: fields.Text(message.Path); break;
			case L::FileHash::HASH: fields.Text(message.Hash); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Read_Message(ByteSpan data, Player & message, DecodeContext * context)
{
	namespace P = L::Player;
	message = Player();
	Fields fields("Player", data, message.Unknown, context);
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


bool Read_Message(ByteSpan data, Option & message, DecodeContext * context)
{
	message = Option();
	Fields fields("Option", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case L::Option::KEY:
				fields.Text(message.Key);
				break;

			case L::Option::NUMBER:
			{
				std::int64_t number = 0;
				if (fields.Sint64(number)) {
					message.Value = number;
				}
				break;
			}

			case L::Option::FLAG:
			{
				bool flag = false;
				if (fields.Bool(flag)) {
					message.Value = flag;
				}
				break;
			}

			case L::Option::TEXT:
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


bool Read_Message(ByteSpan data, Game & message, DecodeContext * context)
{
	message = Game();
	Fields fields("Game", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case L::Game::KIND: fields.Enum(message.Kind); break;
			case L::Game::SEED: fields.Optional(message.Seed, &Fields::Fixed32); break;
			case L::Game::PLAYERS: fields.Repeated(message.Players, Limit(context, &LimitsType::Players)); break;
			case L::Game::OPTIONS: fields.Repeated(message.Options); break;
			case L::Game::ANONYMOUS: fields.Bool(message.Anonymous); break;
			case L::Game::GLOBAL_FLAGS: fields.Packed(message.GlobalFlags); break;
			default: fields.Keep(); break;
		}
	}
	return(fields.Good());
}


bool Decode(ByteSpan data, FileHash & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, Player & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, Option & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, Game & message)
{
	return(Read_Message(data, message, nullptr));
}


bool Decode(ByteSpan data, Game & message, LimitsType const & limits, RuleBreak & broken)
{
	DecodeContext context{ limits };
	bool const good = Read_Message(data, message, &context);
	broken = context.Broken;
	return(good);
}


RuleBreak Check_Limits(Game const & message, LimitsType const & limits)
{
	if (message.Players.size() > limits.Players) {
		return(RuleBreak{ RuleError::TOO_MANY, "Game", L::Game::PLAYERS });
	}
	for (Player const & player : message.Players) {
		if (!Fits(player.Name, limits)) {
			return(RuleBreak{ RuleError::TOO_LONG, "Player", L::Player::NAME });
		}
		if (!Fits(player.CountryName, limits)) {
			return(RuleBreak{ RuleError::TOO_LONG, "Player", L::Player::COUNTRY_NAME });
		}
	}
	for (Option const & option : message.Options) {
		if (!Fits(option.Key, limits)) {
			return(RuleBreak{ RuleError::TOO_LONG, "Option", L::Option::KEY });
		}
		auto text = std::get_if<std::string>(&option.Value);
		if (text != nullptr && !Fits(*text, limits)) {
			return(RuleBreak{ RuleError::TOO_LONG, "Option", L::Option::TEXT });
		}
	}
	return(RuleBreak());
}


RuleBreak Check_Order(Game const & message)
{
	for (std::size_t i = 0; i < message.Players.size(); i++) {
		if (i > 0 && message.Players[i].House <= message.Players[i - 1].House) {
			return(RuleBreak{ RuleError::NOT_ASCENDING, "Game", L::Game::PLAYERS });
		}
		if (!Allies_In_Order(message.Players[i].Allies, message.Players[i].House)) {
			return(RuleBreak{ RuleError::NOT_ASCENDING, "Player", L::Player::ALLIES });
		}
	}
	for (std::size_t i = 1; i < message.Options.size(); i++) {
		if (!(message.Options[i - 1].Key < message.Options[i].Key)) {
			return(RuleBreak{ RuleError::NOT_ASCENDING, "Game", L::Game::OPTIONS });
		}
	}
	if (!Ascending(message.GlobalFlags)) {
		return(RuleBreak{ RuleError::NOT_ASCENDING, "Game", L::Game::GLOBAL_FLAGS });
	}
	return(RuleBreak());
}

}
