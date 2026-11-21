/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "proto/message.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

// The messages of launch.proto as plain structs. Encode writes the canonical encoding, Decode
// accepts any valid one, and fields kept in Unknown are written back in field-number order.

namespace Launch
{
	using ProtoWire::Bytes;
	using ProtoWire::ByteSpan;
	using ProtoMessage::UnknownField;
	using ProtoMessage::UnknownFields;
	using ProtoMessage::LimitsType;
	using ProtoMessage::RuleError;
	using ProtoMessage::RuleBreak;
	using ProtoMessage::Encoded;

	// The only request version this engine runs.
	constexpr std::uint32_t REQUEST_VERSION = 1;

	// The largest request file the engine reads; a larger one is refused as malformed.
	constexpr std::size_t REQUEST_MAX_SIZE = 1u << 20;

	// Open enumerations: a value outside the list is kept as it was read.
	enum class GameKind : std::int32_t {
		GAME_KIND_UNKNOWN = 0,
		GAME_KIND_CAMPAIGN = 1,
		GAME_KIND_SKIRMISH = 2,
		GAME_KIND_MULTIPLAYER = 3,
	};

	enum class ResultCode : std::int32_t {
		RESULT_CODE_UNKNOWN = 0,
		RESULT_CODE_OK = 1,
		RESULT_CODE_REFUSED = 2,
		RESULT_CODE_FAILED = 3,
		RESULT_CODE_CRASHED = 4,
		RESULT_CODE_UNREADABLE = 5,
		RESULT_CODE_MAP_MISSING = 6,
		RESULT_CODE_START_MISMATCH = 7,
		RESULT_CODE_HASH_MISMATCH = 8,
		RESULT_CODE_SEAL_MISMATCH = 9,
	};

	enum class GameOutcome : std::int32_t {
		GAME_OUTCOME_UNKNOWN = 0,
		GAME_OUTCOME_WON = 1,
		GAME_OUTCOME_LOST = 2,
		GAME_OUTCOME_DRAW = 3,
		GAME_OUTCOME_ABORTED = 4,
		GAME_OUTCOME_DESYNC = 5,
		GAME_OUTCOME_CONNECTION_LOST = 6,
		GAME_OUTCOME_NOT_STARTED = 7,
	};

	enum class NotRecorded : std::int32_t {
		NOT_RECORDED_NONE = 0,
		NOT_RECORDED_DISABLED = 1,
		NOT_RECORDED_RESUMED = 2,
		NOT_RECORDED_CHEATS = 3,
		NOT_RECORDED_TIME_LIMIT = 4,
		NOT_RECORDED_WRITE_FAILED = 5,
	};

	enum class RequestFault : std::uint8_t {
		NONE,
		MALFORMED,   // the bytes are not a valid encoding, or the file is past REQUEST_MAX_SIZE
		VERSION,     // version is not REQUEST_VERSION, 0 included
		NO_VERB,     // no verb this engine knows is set
		TEXT,        // a string is not valid UTF-8
	};

	namespace Message
	{
		struct Play;
		struct Load;
		struct Replay;
		struct Verify;
		struct Capture;
	}

	using Verb = std::variant<std::monostate, Message::Play, Message::Load, Message::Replay, Message::Verify, Message::Capture>;
}

// One struct per message, named as the schema names it, with the codec functions beside them
// so that argument-dependent lookup finds them.
namespace Launch::Message
{
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

	struct Option {
		std::string Key;
		std::variant<std::monostate, std::int64_t, bool, std::string> Value;
		UnknownFields Unknown;
	};

	// The Game message: what every machine in a game shares. A replay stores one as LNCH.
	struct Game {
		GameKind Kind = GameKind::GAME_KIND_UNKNOWN;
		std::optional<std::uint32_t> Seed;
		std::vector<Player> Players;
		std::vector<Option> Options;
		bool Anonymous = false;
		std::vector<std::uint32_t> GlobalFlags;
		UnknownFields Unknown;
	};

	struct FileHash {
		std::string Path;
		std::string Hash;
		UnknownFields Unknown;
	};

	struct LoadScreen {
		std::string Image;
		std::optional<std::int32_t> X;
		std::optional<std::int32_t> Y;
		UnknownFields Unknown;
	};

	struct Session {
		std::optional<std::uint32_t> LocalPlayer;
		bool Host = false;
		std::uint32_t AutosaveInterval = 0;
		std::uint32_t NextCampaignAutosave = 0;
		std::uint32_t NextSkirmishAutosave = 0;
		bool SkipScoreScreen = false;
		bool WriteStatistics = false;
		std::uint32_t GameId = 0;
		std::string MapHash;
		std::string MapName;
		std::string GameMode;
		Launch::Message::LoadScreen LoadScreen;
		std::string DifficultyName;
		UnknownFields Unknown;
	};

	struct Peer {
		std::uint32_t Player = 0;
		std::string Address;
		std::int32_t Port = 0;
		UnknownFields Unknown;
	};

	struct Tunnel {
		std::string Address;
		std::uint32_t Port = 0;
		UnknownFields Unknown;
	};

	// A present tunnel whose fields are all empty encodes as absent.
	struct Network {
		std::int32_t Port = 0;
		std::vector<Peer> Peers;
		std::optional<Launch::Message::Tunnel> Tunnel;
		std::uint32_t Protocol = 0;
		std::uint32_t FrameSendRate = 0;
		std::uint32_t MaxAhead = 0;
		std::uint32_t ConnectTimeout = 0;
		std::uint32_t ReconnectTimeout = 0;
		UnknownFields Unknown;
	};

	struct Recording {
		std::optional<bool> Enabled;
		std::string FileOut;
		std::string Package;
		std::string PackageVersion;
		std::vector<FileHash> Files;
		std::optional<bool> EmbedMap;
		UnknownFields Unknown;
	};

	struct CheckpointCheck {
		std::uint32_t SaveFrame = 0;
		std::uint32_t RunFrames = 0;
		UnknownFields Unknown;
	};

	struct Fixture {
		bool Headless = false;
		std::optional<std::uint32_t> StopFrame;
		std::string ScriptPath;
		std::optional<Launch::Message::CheckpointCheck> CheckpointCheck;
		UnknownFields Unknown;
	};

	// Network and Fixture are optional because the rules of a request depend on whether the
	// launcher sent them.
	struct Play {
		Launch::Message::Game Game;
		std::string MapPath;
		Launch::Message::Session Session;
		std::optional<Launch::Message::Network> Network;
		Launch::Message::Recording Recording;
		std::optional<Launch::Message::Fixture> Fixture;
		UnknownFields Unknown;
	};

	struct Load {
		std::string Save;
		Launch::Message::Session Session;
		std::optional<Launch::Message::Network> Network;
		std::vector<Player> Players;
		std::optional<std::int32_t> GameSpeed;
		bool Anonymous = false;
		UnknownFields Unknown;
	};

	struct Replay {
		std::string File;
		std::optional<std::uint32_t> WatchPlayer;
		bool FreeView = false;
		std::uint32_t StartFrame = 0;
		std::string MapPath;
		Launch::Message::LoadScreen LoadScreen;
		UnknownFields Unknown;
	};

	struct Verify {
		std::string File;
		std::string OutPath;
		std::string MapPath;
		UnknownFields Unknown;
	};

	struct Capture {
		std::string MapPath;
		std::string OutPath;
		std::string DescriptionPath;
		UnknownFields Unknown;
	};

	struct Request {
		std::uint32_t Version = 0;
		Launch::Verb Verb;
		std::string ResultPath;
		std::string Channel;
		std::string RequestId;
		UnknownFields Unknown;
	};

	struct ScenarioResult {
		std::string MapPath;
		GameOutcome Outcome = GameOutcome::GAME_OUTCOME_UNKNOWN;
		bool Completed = false;
		std::optional<std::uint32_t> LastFrame;
		std::string ReplayPath;
		Launch::NotRecorded NotRecorded = Launch::NotRecorded::NOT_RECORDED_NONE;
		UnknownFields Unknown;
	};

	struct Result {
		std::string RequestId;
		ResultCode Code = ResultCode::RESULT_CODE_UNKNOWN;
		std::string Message;
		std::vector<ScenarioResult> Scenarios;
		std::string StatisticsPath;
		bool DifferentBuild = false;
		bool Unverified = false;
		std::optional<std::uint32_t> MismatchFrame;
		std::string OutputPath;
		std::string CrashPath;
		UnknownFields Unknown;
	};

	// Appends the canonical encoding of a message to out.
	void Encode(Game const & message, Bytes & out);
	void Encode(Player const & message, Bytes & out);
	void Encode(Option const & message, Bytes & out);
	void Encode(FileHash const & message, Bytes & out);
	void Encode(Request const & message, Bytes & out);
	void Encode(Result const & message, Bytes & out);
	void Encode(Play const & message, Bytes & out);
	void Encode(Load const & message, Bytes & out);
	void Encode(Replay const & message, Bytes & out);
	void Encode(Verify const & message, Bytes & out);
	void Encode(Capture const & message, Bytes & out);
	void Encode(Session const & message, Bytes & out);
	void Encode(LoadScreen const & message, Bytes & out);
	void Encode(Network const & message, Bytes & out);
	void Encode(Peer const & message, Bytes & out);
	void Encode(Tunnel const & message, Bytes & out);
	void Encode(Recording const & message, Bytes & out);
	void Encode(Fixture const & message, Bytes & out);
	void Encode(CheckpointCheck const & message, Bytes & out);
	void Encode(ScenarioResult const & message, Bytes & out);

	RequestFault Decode_Request(ByteSpan data, Request & request);

	// Replaces message with the decoded bytes; false on malformed input, message partly filled.
	bool Decode(ByteSpan data, Game & message);
	bool Decode(ByteSpan data, Player & message);
	bool Decode(ByteSpan data, Option & message);
	bool Decode(ByteSpan data, FileHash & message);
	bool Decode(ByteSpan data, Result & message);

	// As Decode, but false also on a broken limit, which broken names (NONE when malformed).
	bool Decode(ByteSpan data, Game & message, LimitsType const & limits, RuleBreak & broken);

	// Returns the first of the limits that message breaks, or NONE.
	RuleBreak Check_Limits(Game const & message, LimitsType const & limits);

	// Returns NOT_ASCENDING when a list in message is out of the order launch.proto sets, or NONE.
	RuleBreak Check_Order(Game const & message);

	// For the codec of another namespace whose messages nest these.
	bool Read_Message(ByteSpan data, Game & message, ProtoMessage::DecodeContext * context);
	bool Read_Message(ByteSpan data, Player & message, ProtoMessage::DecodeContext * context);
	bool Read_Message(ByteSpan data, Option & message, ProtoMessage::DecodeContext * context);
	bool Read_Message(ByteSpan data, FileHash & message, ProtoMessage::DecodeContext * context);
}

namespace Launch
{
	using Message::Encode;
	using Message::Decode;
	using Message::Decode_Request;
	using Message::Check_Limits;
	using Message::Check_Order;
}
