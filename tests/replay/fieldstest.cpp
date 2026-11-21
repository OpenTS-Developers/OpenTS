/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "doctest/doctest.h"
#include "launch/launchmessages.h"

#include "launch/launchfields.hh"
#include "replay/replay.hh"
#include "replay/replayfields.hh"

#include <cctype>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace F = Replay::Field;
namespace L = Launch::Field;


namespace {

enum class Label {
	PLAIN,
	PRESENCE,
	REPEATED,
	ONEOF,
};

struct FieldEntry {
	char const * Message;
	char const * Name;
	std::uint32_t Number;
	char const * Type;
	Label Kind;
};

struct EnumEntry {
	char const * Enum;
	char const * Name;
	int Value;
};

// Every field of replay.proto, by the constant the codec writes it with.
FieldEntry const FIELDS[] = {
{ "RecordingInfo", "engine", F::RecordingInfo::ENGINE, "string", Label::PLAIN },
	{ "RecordingInfo", "engine_version", F::RecordingInfo::ENGINE_VERSION, "string", Label::PLAIN },
	{ "RecordingInfo", "data_version", F::RecordingInfo::DATA_VERSION, "uint32", Label::PLAIN },
	{ "RecordingInfo", "package", F::RecordingInfo::PACKAGE, "string", Label::PLAIN },
	{ "RecordingInfo", "package_version", F::RecordingInfo::PACKAGE_VERSION, "string", Label::PLAIN },
	{ "RecordingInfo", "files", F::RecordingInfo::FILES, "cnclaunch.FileHash", Label::REPEATED },
	{ "RecordingInfo", "recorded_at", F::RecordingInfo::RECORDED_AT, "uint64", Label::PLAIN },
	{ "RecordingInfo", "recorder", F::RecordingInfo::RECORDER, "uint32", Label::PRESENCE },
	{ "RecordingInfo", "map_name", F::RecordingInfo::MAP_NAME, "string", Label::PLAIN },
	{ "RecordingInfo", "map_path", F::RecordingInfo::MAP_PATH, "string", Label::PLAIN },
	{ "RecordingInfo", "map_sha256", F::RecordingInfo::MAP_SHA256, "bytes", Label::PLAIN },
	{ "RecordingInfo", "game_mode", F::RecordingInfo::GAME_MODE, "string", Label::PLAIN },
	{ "RecordingInfo", "seal_version", F::RecordingInfo::SEAL_VERSION, "uint32", Label::PLAIN },
	{ "RecordingInfo", "colors", F::RecordingInfo::COLORS, "PlayerColor", Label::REPEATED },
	{ "RecordingInfo", "income_kinds", F::RecordingInfo::INCOME_KINDS, "IncomeKind", Label::REPEATED },
	{ "PlayerColor", "number", F::PlayerColor::NUMBER, "uint32", Label::PLAIN },
	{ "PlayerColor", "rgb", F::PlayerColor::RGB, "fixed32", Label::PLAIN },
	{ "IncomeKind", "number", F::IncomeKind::NUMBER, "uint32", Label::PLAIN },
	{ "IncomeKind", "id", F::IncomeKind::ID, "string", Label::PLAIN },
	{ "Frame", "skipped", F::Frame::SKIPPED, "uint32", Label::PLAIN },
	{ "Frame", "hash", F::Frame::HASH, "fixed32", Label::PRESENCE },
	{ "Frame", "events", F::Frame::EVENTS, "Event", Label::REPEATED },
	{ "Frame", "view", F::Frame::VIEW, "ViewState", Label::PLAIN },
	{ "Frame", "messages", F::Frame::MESSAGES, "PlayerMessage", Label::REPEATED },
	{ "Frame", "frame_micros", F::Frame::FRAME_MICROS, "uint32", Label::PRESENCE },
	{ "Frame", "samples", F::Frame::SAMPLES, "HouseSample", Label::REPEATED },
	{ "Frame", "income", F::Frame::INCOME, "Income", Label::REPEATED },
	{ "Frame", "seals", F::Frame::SEALS, "Seal", Label::REPEATED },
	{ "Frame", "defeated", F::Frame::DEFEATED, "uint32", Label::REPEATED },
	{ "Event", "type", F::Event::TYPE, "uint32", Label::PLAIN },
	{ "Event", "house", F::Event::HOUSE, "uint32", Label::PLAIN },
	{ "Event", "due", F::Event::DUE, "sint32", Label::PLAIN },
	{ "Event", "payload", F::Event::PAYLOAD, "bytes", Label::PLAIN },
	{ "Event", "local", F::Event::LOCAL, "bool", Label::PLAIN },
	{ "Event", "role", F::Event::ROLE, "EventRole", Label::PLAIN },
	{ "Event", "sealed_payload", F::Event::SEALED_PAYLOAD, "bytes", Label::PRESENCE },
	{ "ViewState", "camera", F::ViewState::CAMERA, "Point", Label::PRESENCE },
	{ "ViewState", "selection", F::ViewState::SELECTION, "Selection", Label::PRESENCE },
	{ "Point", "x", F::Point::X, "sint32", Label::PLAIN },
	{ "Point", "y", F::Point::Y, "sint32", Label::PLAIN },
	{ "Selection", "ids", F::Selection::IDS, "uint32", Label::REPEATED },
	{ "PlayerMessage", "house", F::PlayerMessage::HOUSE, "uint32", Label::PLAIN },
	{ "PlayerMessage", "sender", F::PlayerMessage::SENDER, "string", Label::PLAIN },
	{ "PlayerMessage", "text", F::PlayerMessage::TEXT, "string", Label::PLAIN },
	{ "PlayerMessage", "color", F::PlayerMessage::COLOR, "sint32", Label::PLAIN },
	{ "PlayerMessage", "scope", F::PlayerMessage::SCOPE, "MessageScope", Label::PLAIN },
	{ "PlayerMessage", "recipients", F::PlayerMessage::RECIPIENTS, "uint32", Label::REPEATED },
	{ "PlayerMessage", "salt", F::PlayerMessage::SALT, "bytes", Label::PLAIN },
	{ "HouseSample", "house", F::HouseSample::HOUSE, "uint32", Label::PLAIN },
	{ "HouseSample", "credits", F::HouseSample::CREDITS, "sint64", Label::PRESENCE },
	{ "HouseSample", "stored_value", F::HouseSample::STORED_VALUE, "sint64", Label::PRESENCE },
	{ "HouseSample", "spent", F::HouseSample::SPENT, "sint64", Label::PRESENCE },
	{ "HouseSample", "earned", F::HouseSample::EARNED, "sint64", Label::PRESENCE },
	{ "HouseSample", "power_output", F::HouseSample::POWER_OUTPUT, "sint32", Label::PRESENCE },
	{ "HouseSample", "power_drain", F::HouseSample::POWER_DRAIN, "sint32", Label::PRESENCE },
	{ "HouseSample", "vehicles", F::HouseSample::VEHICLES, "uint32", Label::PRESENCE },
	{ "HouseSample", "infantry", F::HouseSample::INFANTRY, "uint32", Label::PRESENCE },
	{ "HouseSample", "aircraft", F::HouseSample::AIRCRAFT, "uint32", Label::PRESENCE },
	{ "HouseSample", "buildings", F::HouseSample::BUILDINGS, "uint32", Label::PRESENCE },
	{ "HouseSample", "army_value", F::HouseSample::ARMY_VALUE, "sint64", Label::PRESENCE },
	{ "HouseSample", "building_value", F::HouseSample::BUILDING_VALUE, "sint64", Label::PRESENCE },
	{ "HouseSample", "units_killed", F::HouseSample::UNITS_KILLED, "uint32", Label::PRESENCE },
	{ "HouseSample", "buildings_killed", F::HouseSample::BUILDINGS_KILLED, "uint32", Label::PRESENCE },
	{ "HouseSample", "units_lost", F::HouseSample::UNITS_LOST, "uint32", Label::PRESENCE },
	{ "HouseSample", "buildings_lost", F::HouseSample::BUILDINGS_LOST, "uint32", Label::PRESENCE },
	{ "HouseSample", "units_built", F::HouseSample::UNITS_BUILT, "uint32", Label::PRESENCE },
	{ "HouseSample", "buildings_built", F::HouseSample::BUILDINGS_BUILT, "uint32", Label::PRESENCE },
	{ "HouseSample", "score", F::HouseSample::SCORE, "sint64", Label::PRESENCE },
	{ "HouseSample", "defeated", F::HouseSample::DEFEATED, "bool", Label::PLAIN },
	{ "Income", "house", F::Income::HOUSE, "uint32", Label::PLAIN },
	{ "Income", "kind", F::Income::KIND, "uint32", Label::PLAIN },
	{ "Income", "amount", F::Income::AMOUNT, "sint64", Label::PLAIN },
	{ "Seal", "through", F::Seal::THROUGH, "uint32", Label::PLAIN },
	{ "Seal", "hash", F::Seal::HASH, "bytes", Label::PLAIN },
	{ "Checkpoint", "frame", F::Checkpoint::FRAME, "uint32", Label::PLAIN },
	{ "Checkpoint", "crc32", F::Checkpoint::CRC32, "fixed32", Label::PLAIN },
	{ "Checkpoint", "state", F::Checkpoint::STATE, "bytes", Label::PLAIN },
	{ "Statistics", "types", F::Statistics::TYPES, "ObjectType", Label::REPEATED },
	{ "Statistics", "houses", F::Statistics::HOUSES, "HouseStatistics", Label::REPEATED },
	{ "ObjectType", "category", F::ObjectType::CATEGORY, "ObjectCategory", Label::PLAIN },
	{ "ObjectType", "index", F::ObjectType::INDEX, "uint32", Label::PLAIN },
	{ "ObjectType", "id", F::ObjectType::ID, "string", Label::PLAIN },
	{ "ObjectType", "name", F::ObjectType::NAME, "string", Label::PLAIN },
	{ "ObjectType", "cost", F::ObjectType::COST, "sint32", Label::PLAIN },
	{ "ObjectType", "icon", F::ObjectType::ICON, "string", Label::PLAIN },
	{ "ObjectType", "naval", F::ObjectType::NAVAL, "bool", Label::PLAIN },
	{ "ObjectType", "unscored", F::ObjectType::UNSCORED, "bool", Label::PLAIN },
	{ "ObjectType", "insignificant", F::ObjectType::INSIGNIFICANT, "bool", Label::PLAIN },
	{ "HouseStatistics", "house", F::HouseStatistics::HOUSE, "uint32", Label::PLAIN },
	{ "HouseStatistics", "name", F::HouseStatistics::NAME, "string", Label::PLAIN },
	{ "HouseStatistics", "country_name", F::HouseStatistics::COUNTRY_NAME, "string", Label::PLAIN },
	{ "HouseStatistics", "color", F::HouseStatistics::COLOR, "sint32", Label::PLAIN },
	{ "HouseStatistics", "start", F::HouseStatistics::START, "sint32", Label::PLAIN },
	{ "HouseStatistics", "allies", F::HouseStatistics::ALLIES, "uint32", Label::REPEATED },
	{ "HouseStatistics", "outcome", F::HouseStatistics::OUTCOME, "Outcome", Label::PLAIN },
	{ "HouseStatistics", "leave", F::HouseStatistics::LEAVE, "LeaveReason", Label::PLAIN },
	{ "HouseStatistics", "final", F::HouseStatistics::FINAL, "HouseSample", Label::PLAIN },
	{ "HouseStatistics", "counts", F::HouseStatistics::COUNTS, "TypeCount", Label::REPEATED },
	{ "HouseStatistics", "kills", F::HouseStatistics::KILLS, "HouseKills", Label::REPEATED },
	{ "HouseStatistics", "crates", F::HouseStatistics::CRATES, "CrateCount", Label::REPEATED },
	{ "TypeCount", "kind", F::TypeCount::KIND, "CountKind", Label::PLAIN },
	{ "TypeCount", "category", F::TypeCount::CATEGORY, "ObjectCategory", Label::PLAIN },
	{ "TypeCount", "index", F::TypeCount::INDEX, "uint32", Label::PLAIN },
	{ "TypeCount", "count", F::TypeCount::COUNT, "uint32", Label::PLAIN },
	{ "CrateCount", "id", F::CrateCount::ID, "string", Label::PLAIN },
	{ "CrateCount", "count", F::CrateCount::COUNT, "uint32", Label::PLAIN },
	{ "HouseKills", "house", F::HouseKills::HOUSE, "uint32", Label::PLAIN },
	{ "HouseKills", "units", F::HouseKills::UNITS, "uint32", Label::PLAIN },
	{ "HouseKills", "buildings", F::HouseKills::BUILDINGS, "uint32", Label::PLAIN },
	{ "RecordingSummary", "last_frame", F::RecordingSummary::LAST_FRAME, "uint32", Label::PRESENCE },
	{ "RecordingSummary", "ended_at", F::RecordingSummary::ENDED_AT, "uint64", Label::PLAIN },
	{ "RecordingSummary", "desync_frame", F::RecordingSummary::DESYNC_FRAME, "uint32", Label::PRESENCE },
	{ "RecordingSummary", "completed", F::RecordingSummary::COMPLETED, "bool", Label::PLAIN },
	{ "RecordingSummary", "seals", F::RecordingSummary::SEALS, "Seal", Label::REPEATED },
	{ "RecordingSummary", "elapsed_micros", F::RecordingSummary::ELAPSED_MICROS, "uint64", Label::PRESENCE },
	{ "RecordingSummary", "game_micros", F::RecordingSummary::GAME_MICROS, "uint64", Label::PRESENCE },
};

EnumEntry const ENUMS[] = {
	{ "EventRole", "EVENT_ROLE_NONE", int(Replay::EventRole::EVENT_ROLE_NONE) },
	{ "EventRole", "EVENT_ROLE_CHAT_PROOF", int(Replay::EventRole::EVENT_ROLE_CHAT_PROOF) },
	{ "MessageScope", "MESSAGE_SCOPE_UNKNOWN", int(Replay::MessageScope::MESSAGE_SCOPE_UNKNOWN) },
	{ "MessageScope", "MESSAGE_SCOPE_ALL", int(Replay::MessageScope::MESSAGE_SCOPE_ALL) },
	{ "MessageScope", "MESSAGE_SCOPE_TEAM", int(Replay::MessageScope::MESSAGE_SCOPE_TEAM) },
	{ "MessageScope", "MESSAGE_SCOPE_PRIVATE", int(Replay::MessageScope::MESSAGE_SCOPE_PRIVATE) },
	{ "MessageScope", "MESSAGE_SCOPE_OBSERVERS", int(Replay::MessageScope::MESSAGE_SCOPE_OBSERVERS) },
	{ "ObjectCategory", "OBJECT_CATEGORY_UNKNOWN", int(Replay::ObjectCategory::OBJECT_CATEGORY_UNKNOWN) },
	{ "ObjectCategory", "OBJECT_CATEGORY_BUILDING", int(Replay::ObjectCategory::OBJECT_CATEGORY_BUILDING) },
	{ "ObjectCategory", "OBJECT_CATEGORY_INFANTRY", int(Replay::ObjectCategory::OBJECT_CATEGORY_INFANTRY) },
	{ "ObjectCategory", "OBJECT_CATEGORY_VEHICLE", int(Replay::ObjectCategory::OBJECT_CATEGORY_VEHICLE) },
	{ "ObjectCategory", "OBJECT_CATEGORY_AIRCRAFT", int(Replay::ObjectCategory::OBJECT_CATEGORY_AIRCRAFT) },
	{ "Outcome", "OUTCOME_UNKNOWN", int(Replay::Outcome::OUTCOME_UNKNOWN) },
	{ "Outcome", "OUTCOME_WON", int(Replay::Outcome::OUTCOME_WON) },
	{ "Outcome", "OUTCOME_LOST", int(Replay::Outcome::OUTCOME_LOST) },
	{ "Outcome", "OUTCOME_DRAW", int(Replay::Outcome::OUTCOME_DRAW) },
	{ "LeaveReason", "LEAVE_REASON_NONE", int(Replay::LeaveReason::LEAVE_REASON_NONE) },
	{ "LeaveReason", "LEAVE_REASON_RESIGNED", int(Replay::LeaveReason::LEAVE_REASON_RESIGNED) },
	{ "LeaveReason", "LEAVE_REASON_DISCONNECTED", int(Replay::LeaveReason::LEAVE_REASON_DISCONNECTED) },
	{ "CountKind", "COUNT_KIND_UNKNOWN", int(Replay::CountKind::COUNT_KIND_UNKNOWN) },
	{ "CountKind", "COUNT_KIND_BUILT", int(Replay::CountKind::COUNT_KIND_BUILT) },
	{ "CountKind", "COUNT_KIND_KILLED", int(Replay::CountKind::COUNT_KIND_KILLED) },
	{ "CountKind", "COUNT_KIND_LOST", int(Replay::CountKind::COUNT_KIND_LOST) },
	{ "CountKind", "COUNT_KIND_CAPTURED", int(Replay::CountKind::COUNT_KIND_CAPTURED) },
	{ "CountKind", "COUNT_KIND_REMAINING", int(Replay::CountKind::COUNT_KIND_REMAINING) },
};


// Every field of launch.proto, by the constant the codec writes it with.
FieldEntry const LAUNCH_FIELDS[] = {
	{ "Request", "version", L::Request::VERSION, "uint32", Label::PLAIN },
	{ "Request", "play", L::Request::PLAY, "Play", Label::ONEOF },
	{ "Request", "load", L::Request::LOAD, "Load", Label::ONEOF },
	{ "Request", "replay", L::Request::REPLAY, "Replay", Label::ONEOF },
	{ "Request", "verify", L::Request::VERIFY, "Verify", Label::ONEOF },
	{ "Request", "capture", L::Request::CAPTURE, "Capture", Label::ONEOF },
	{ "Request", "result_path", L::Request::RESULT_PATH, "string", Label::PLAIN },
	{ "Request", "channel", L::Request::CHANNEL, "string", Label::PLAIN },
	{ "Request", "request_id", L::Request::REQUEST_ID, "string", Label::PLAIN },
	{ "Play", "game", L::Play::GAME, "Game", Label::PLAIN },
	{ "Play", "map_path", L::Play::MAP_PATH, "string", Label::PLAIN },
	{ "Play", "session", L::Play::SESSION, "Session", Label::PLAIN },
	{ "Play", "network", L::Play::NETWORK, "Network", Label::PLAIN },
	{ "Play", "recording", L::Play::RECORDING, "Recording", Label::PLAIN },
	{ "Play", "fixture", L::Play::FIXTURE, "Fixture", Label::PLAIN },
	{ "Game", "kind", L::Game::KIND, "GameKind", Label::PLAIN },
	{ "Game", "seed", L::Game::SEED, "fixed32", Label::PRESENCE },
	{ "Game", "players", L::Game::PLAYERS, "Player", Label::REPEATED },
	{ "Game", "options", L::Game::OPTIONS, "Option", Label::REPEATED },
	{ "Game", "anonymous", L::Game::ANONYMOUS, "bool", Label::PLAIN },
	{ "Game", "global_flags", L::Game::GLOBAL_FLAGS, "uint32", Label::REPEATED },
	{ "Player", "house", L::Player::HOUSE, "uint32", Label::PLAIN },
	{ "Player", "name", L::Player::NAME, "string", Label::PLAIN },
	{ "Player", "human", L::Player::HUMAN, "bool", Label::PLAIN },
	{ "Player", "observer", L::Player::OBSERVER, "bool", Label::PLAIN },
	{ "Player", "country", L::Player::COUNTRY, "sint32", Label::PLAIN },
	{ "Player", "country_name", L::Player::COUNTRY_NAME, "string", Label::PLAIN },
	{ "Player", "color", L::Player::COLOR, "sint32", Label::PLAIN },
	{ "Player", "start", L::Player::START, "sint32", Label::PLAIN },
	{ "Player", "ai_level", L::Player::AI_LEVEL, "sint32", Label::PRESENCE },
	{ "Player", "allies", L::Player::ALLIES, "uint32", Label::REPEATED },
	{ "Player", "resolved_country", L::Player::RESOLVED_COUNTRY, "sint32", Label::PRESENCE },
	{ "Player", "resolved_color", L::Player::RESOLVED_COLOR, "sint32", Label::PRESENCE },
	{ "Player", "resolved_start", L::Player::RESOLVED_START, "sint32", Label::PRESENCE },
	{ "Option", "key", L::Option::KEY, "string", Label::PLAIN },
	{ "Option", "number", L::Option::NUMBER, "sint64", Label::ONEOF },
	{ "Option", "flag", L::Option::FLAG, "bool", Label::ONEOF },
	{ "Option", "text", L::Option::TEXT, "string", Label::ONEOF },
	{ "Session", "local_player", L::Session::LOCAL_PLAYER, "uint32", Label::PRESENCE },
	{ "Session", "host", L::Session::HOST, "bool", Label::PLAIN },
	{ "Session", "autosave_interval", L::Session::AUTOSAVE_INTERVAL, "uint32", Label::PLAIN },
	{ "Session", "next_campaign_autosave", L::Session::NEXT_CAMPAIGN_AUTOSAVE, "uint32", Label::PLAIN },
	{ "Session", "next_skirmish_autosave", L::Session::NEXT_SKIRMISH_AUTOSAVE, "uint32", Label::PLAIN },
	{ "Session", "skip_score_screen", L::Session::SKIP_SCORE_SCREEN, "bool", Label::PLAIN },
	{ "Session", "write_statistics", L::Session::WRITE_STATISTICS, "bool", Label::PLAIN },
	{ "Session", "game_id", L::Session::GAME_ID, "uint32", Label::PLAIN },
	{ "Session", "map_hash", L::Session::MAP_HASH, "string", Label::PLAIN },
	{ "Session", "map_name", L::Session::MAP_NAME, "string", Label::PLAIN },
	{ "Session", "game_mode", L::Session::GAME_MODE, "string", Label::PLAIN },
	{ "Session", "load_screen", L::Session::LOAD_SCREEN, "LoadScreen", Label::PLAIN },
	{ "Session", "difficulty_name", L::Session::DIFFICULTY_NAME, "string", Label::PLAIN },
	{ "LoadScreen", "image", L::LoadScreen::IMAGE, "string", Label::PLAIN },
	{ "LoadScreen", "x", L::LoadScreen::X, "sint32", Label::PRESENCE },
	{ "LoadScreen", "y", L::LoadScreen::Y, "sint32", Label::PRESENCE },
	{ "Network", "port", L::Network::PORT, "sint32", Label::PLAIN },
	{ "Network", "peers", L::Network::PEERS, "Peer", Label::REPEATED },
	{ "Network", "tunnel", L::Network::TUNNEL, "Tunnel", Label::PLAIN },
	{ "Network", "protocol", L::Network::PROTOCOL, "uint32", Label::PLAIN },
	{ "Network", "frame_send_rate", L::Network::FRAME_SEND_RATE, "uint32", Label::PLAIN },
	{ "Network", "max_ahead", L::Network::MAX_AHEAD, "uint32", Label::PLAIN },
	{ "Network", "connect_timeout", L::Network::CONNECT_TIMEOUT, "uint32", Label::PLAIN },
	{ "Network", "reconnect_timeout", L::Network::RECONNECT_TIMEOUT, "uint32", Label::PLAIN },
	{ "Peer", "player", L::Peer::PLAYER, "uint32", Label::PLAIN },
	{ "Peer", "address", L::Peer::ADDRESS, "string", Label::PLAIN },
	{ "Peer", "port", L::Peer::PORT, "sint32", Label::PLAIN },
	{ "Tunnel", "address", L::Tunnel::ADDRESS, "string", Label::PLAIN },
	{ "Tunnel", "port", L::Tunnel::PORT, "uint32", Label::PLAIN },
	{ "Recording", "enabled", L::Recording::ENABLED, "bool", Label::PRESENCE },
	{ "Recording", "file_out", L::Recording::FILE_OUT, "string", Label::PLAIN },
	{ "Recording", "package", L::Recording::PACKAGE, "string", Label::PLAIN },
	{ "Recording", "package_version", L::Recording::PACKAGE_VERSION, "string", Label::PLAIN },
	{ "Recording", "files", L::Recording::FILES, "FileHash", Label::REPEATED },
	{ "Recording", "embed_map", L::Recording::EMBED_MAP, "bool", Label::PRESENCE },
	{ "FileHash", "path", L::FileHash::PATH, "string", Label::PLAIN },
	{ "FileHash", "hash", L::FileHash::HASH, "string", Label::PLAIN },
	{ "Fixture", "headless", L::Fixture::HEADLESS, "bool", Label::PLAIN },
	{ "Fixture", "stop_frame", L::Fixture::STOP_FRAME, "uint32", Label::PRESENCE },
	{ "Fixture", "script_path", L::Fixture::SCRIPT_PATH, "string", Label::PLAIN },
	{ "Fixture", "checkpoint_check", L::Fixture::CHECKPOINT_CHECK, "CheckpointCheck", Label::PLAIN },
	{ "CheckpointCheck", "save_frame", L::CheckpointCheck::SAVE_FRAME, "uint32", Label::PLAIN },
	{ "CheckpointCheck", "run_frames", L::CheckpointCheck::RUN_FRAMES, "uint32", Label::PLAIN },
	{ "Load", "save", L::Load::SAVE, "string", Label::PLAIN },
	{ "Load", "session", L::Load::SESSION, "Session", Label::PLAIN },
	{ "Load", "network", L::Load::NETWORK, "Network", Label::PLAIN },
	{ "Load", "players", L::Load::PLAYERS, "Player", Label::REPEATED },
	{ "Load", "game_speed", L::Load::GAME_SPEED, "sint32", Label::PRESENCE },
	{ "Load", "anonymous", L::Load::ANONYMOUS, "bool", Label::PLAIN },
	{ "Replay", "file", L::Replay::FILE, "string", Label::PLAIN },
	{ "Replay", "watch_player", L::Replay::WATCH_PLAYER, "uint32", Label::PRESENCE },
	{ "Replay", "free_view", L::Replay::FREE_VIEW, "bool", Label::PLAIN },
	{ "Replay", "start_frame", L::Replay::START_FRAME, "uint32", Label::PLAIN },
	{ "Replay", "map_path", L::Replay::MAP_PATH, "string", Label::PLAIN },
	{ "Replay", "load_screen", L::Replay::LOAD_SCREEN, "LoadScreen", Label::PLAIN },
	{ "Verify", "file", L::Verify::FILE, "string", Label::PLAIN },
	{ "Verify", "out_path", L::Verify::OUT_PATH, "string", Label::PLAIN },
	{ "Verify", "map_path", L::Verify::MAP_PATH, "string", Label::PLAIN },
	{ "Capture", "map_path", L::Capture::MAP_PATH, "string", Label::PLAIN },
	{ "Capture", "out_path", L::Capture::OUT_PATH, "string", Label::PLAIN },
	{ "Capture", "description_path", L::Capture::DESCRIPTION_PATH, "string", Label::PLAIN },
	{ "Result", "request_id", L::Result::REQUEST_ID, "string", Label::PLAIN },
	{ "Result", "code", L::Result::CODE, "ResultCode", Label::PLAIN },
	{ "Result", "message", L::Result::MESSAGE, "string", Label::PLAIN },
	{ "Result", "scenarios", L::Result::SCENARIOS, "ScenarioResult", Label::REPEATED },
	{ "Result", "statistics_path", L::Result::STATISTICS_PATH, "string", Label::PLAIN },
	{ "Result", "different_build", L::Result::DIFFERENT_BUILD, "bool", Label::PLAIN },
	{ "Result", "unverified", L::Result::UNVERIFIED, "bool", Label::PLAIN },
	{ "Result", "mismatch_frame", L::Result::MISMATCH_FRAME, "uint32", Label::PRESENCE },
	{ "Result", "output_path", L::Result::OUTPUT_PATH, "string", Label::PLAIN },
	{ "Result", "crash_path", L::Result::CRASH_PATH, "string", Label::PLAIN },
	{ "ScenarioResult", "map_path", L::ScenarioResult::MAP_PATH, "string", Label::PLAIN },
	{ "ScenarioResult", "outcome", L::ScenarioResult::OUTCOME, "GameOutcome", Label::PLAIN },
	{ "ScenarioResult", "completed", L::ScenarioResult::COMPLETED, "bool", Label::PLAIN },
	{ "ScenarioResult", "last_frame", L::ScenarioResult::LAST_FRAME, "uint32", Label::PRESENCE },
	{ "ScenarioResult", "replay_path", L::ScenarioResult::REPLAY_PATH, "string", Label::PLAIN },
	{ "ScenarioResult", "not_recorded", L::ScenarioResult::NOT_RECORDED, "NotRecorded", Label::PLAIN },
};

EnumEntry const LAUNCH_ENUMS[] = {
	{ "GameKind", "GAME_KIND_UNKNOWN", int(Launch::GameKind::GAME_KIND_UNKNOWN) },
	{ "GameKind", "GAME_KIND_CAMPAIGN", int(Launch::GameKind::GAME_KIND_CAMPAIGN) },
	{ "GameKind", "GAME_KIND_SKIRMISH", int(Launch::GameKind::GAME_KIND_SKIRMISH) },
	{ "GameKind", "GAME_KIND_MULTIPLAYER", int(Launch::GameKind::GAME_KIND_MULTIPLAYER) },
	{ "ResultCode", "RESULT_CODE_UNKNOWN", int(Launch::ResultCode::RESULT_CODE_UNKNOWN) },
	{ "ResultCode", "RESULT_CODE_OK", int(Launch::ResultCode::RESULT_CODE_OK) },
	{ "ResultCode", "RESULT_CODE_REFUSED", int(Launch::ResultCode::RESULT_CODE_REFUSED) },
	{ "ResultCode", "RESULT_CODE_FAILED", int(Launch::ResultCode::RESULT_CODE_FAILED) },
	{ "ResultCode", "RESULT_CODE_CRASHED", int(Launch::ResultCode::RESULT_CODE_CRASHED) },
	{ "ResultCode", "RESULT_CODE_UNREADABLE", int(Launch::ResultCode::RESULT_CODE_UNREADABLE) },
	{ "ResultCode", "RESULT_CODE_MAP_MISSING", int(Launch::ResultCode::RESULT_CODE_MAP_MISSING) },
	{ "ResultCode", "RESULT_CODE_START_MISMATCH", int(Launch::ResultCode::RESULT_CODE_START_MISMATCH) },
	{ "ResultCode", "RESULT_CODE_HASH_MISMATCH", int(Launch::ResultCode::RESULT_CODE_HASH_MISMATCH) },
	{ "ResultCode", "RESULT_CODE_SEAL_MISMATCH", int(Launch::ResultCode::RESULT_CODE_SEAL_MISMATCH) },
	{ "GameOutcome", "GAME_OUTCOME_UNKNOWN", int(Launch::GameOutcome::GAME_OUTCOME_UNKNOWN) },
	{ "GameOutcome", "GAME_OUTCOME_WON", int(Launch::GameOutcome::GAME_OUTCOME_WON) },
	{ "GameOutcome", "GAME_OUTCOME_LOST", int(Launch::GameOutcome::GAME_OUTCOME_LOST) },
	{ "GameOutcome", "GAME_OUTCOME_DRAW", int(Launch::GameOutcome::GAME_OUTCOME_DRAW) },
	{ "GameOutcome", "GAME_OUTCOME_ABORTED", int(Launch::GameOutcome::GAME_OUTCOME_ABORTED) },
	{ "GameOutcome", "GAME_OUTCOME_DESYNC", int(Launch::GameOutcome::GAME_OUTCOME_DESYNC) },
	{ "GameOutcome", "GAME_OUTCOME_CONNECTION_LOST", int(Launch::GameOutcome::GAME_OUTCOME_CONNECTION_LOST) },
	{ "GameOutcome", "GAME_OUTCOME_NOT_STARTED", int(Launch::GameOutcome::GAME_OUTCOME_NOT_STARTED) },
	{ "NotRecorded", "NOT_RECORDED_NONE", int(Launch::NotRecorded::NOT_RECORDED_NONE) },
	{ "NotRecorded", "NOT_RECORDED_DISABLED", int(Launch::NotRecorded::NOT_RECORDED_DISABLED) },
	{ "NotRecorded", "NOT_RECORDED_RESUMED", int(Launch::NotRecorded::NOT_RECORDED_RESUMED) },
	{ "NotRecorded", "NOT_RECORDED_CHEATS", int(Launch::NotRecorded::NOT_RECORDED_CHEATS) },
	{ "NotRecorded", "NOT_RECORDED_TIME_LIMIT", int(Launch::NotRecorded::NOT_RECORDED_TIME_LIMIT) },
	{ "NotRecorded", "NOT_RECORDED_WRITE_FAILED", int(Launch::NotRecorded::NOT_RECORDED_WRITE_FAILED) },
};


struct ProtoField {
	std::string Type;
	Label Kind = Label::PLAIN;
	std::uint32_t Number = 0;
};

struct ProtoSchema {
	std::map<std::string, std::map<std::string, ProtoField>> Messages;
	std::map<std::string, std::map<std::string, int>> Enums;
	std::set<std::string> PrivateRange;
};


bool Is_Word_Char(char c)
{
	return(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '.');
}


std::vector<std::string> Tokenize(std::string const & text)
{
	std::vector<std::string> tokens;
	std::size_t i = 0;
	while (i < text.size()) {
		char c = text[i];
		if (std::isspace(static_cast<unsigned char>(c))) {
			i++;
		} else if (text.compare(i, 2, "//") == 0) {
			i = text.find('\n', i);
			if (i == std::string::npos) {
				i = text.size();
			}
		} else if (text.compare(i, 2, "/*") == 0) {
			i = text.find("*/", i);
			i = (i == std::string::npos) ? text.size() : i + 2;
		} else if (Is_Word_Char(c)) {
			std::size_t start = i;
			while (i < text.size() && Is_Word_Char(text[i])) {
				i++;
			}
			tokens.push_back(text.substr(start, i - start));
		} else if (c == '"') {
			std::size_t end = text.find('"', i + 1);
			end = (end == std::string::npos) ? text.size() : end + 1;
			tokens.push_back(text.substr(i, end - i));
			i = end;
		} else {
			tokens.push_back(std::string(1, c));
			i++;
		}
	}
	return(tokens);
}


// Parses the subset of proto3 that replay.proto and launch.proto use. A type from an imported
// file keeps its package prefix, as in cnclaunch.FileHash. Any other construct fails the parse, so
// a schema that uses one fails this test until the parser handles it.
class ProtoParser
{
	public:
		explicit ProtoParser(std::vector<std::string> tokens) :
			Tokens(std::move(tokens)),
			Position(0),
			Failed(false)
		{
		}

		bool Parse(ProtoSchema & schema)
		{
			while (!Failed && Position < Tokens.size()) {
				std::string word = Take();
				if (word == "syntax") {
					Expect("=");
					Take();
					Expect(";");
				} else if (word == "package" || word == "import") {
					Take();
					Expect(";");
				} else if (word == "message") {
					Parse_Message(schema);
				} else if (word == "enum") {
					Parse_Enum(schema);
				} else {
					Fail("unexpected at top level: " + word);
				}
			}
			return(!Failed);
		}

		std::string Problem;

	private:
		std::string Take(void)
		{
			if (Position >= Tokens.size()) {
				Fail("unexpected end");
				return(std::string());
			}
			return(Tokens[Position++]);
		}

		std::string Peek(void) const
		{
			return((Position < Tokens.size()) ? Tokens[Position] : std::string());
		}

		void Expect(char const * token)
		{
			std::string got = Take();
			if (got != token) {
				Fail(std::string("expected ") + token + ", got " + got);
			}
		}

		void Fail(std::string const & why)
		{
			if (!Failed) {
				Problem = why;
			}
			Failed = true;
			Position = Tokens.size();
		}

		std::uint32_t Number(void)
		{
			std::string text = Take();
			if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos) {
				Fail("expected a number, got " + text);
				return(0);
			}
			return(std::uint32_t(std::stoul(text)));
		}

		void Parse_Field(std::map<std::string, ProtoField> & fields, Label kind, std::string const & type)
		{
			ProtoField field;
			field.Kind = kind;
			field.Type = type;
			std::string name = Take();
			Expect("=");
			field.Number = Number();
			Expect(";");
			if (fields.count(name) != 0) {
				Fail("duplicate field " + name);
			}
			fields[name] = field;
		}

		void Parse_Message(ProtoSchema & schema)
		{
			std::string name = Take();
			auto & fields = schema.Messages[name];
			Expect("{");
			while (!Failed && Peek() != "}") {
				std::string word = Take();
				if (word == "reserved") {
					std::uint32_t low = Number();
					Expect("to");
					std::uint32_t high = Number();
					Expect(";");
					if (low == 100 && high == 199) {
						schema.PrivateRange.insert(name);
					} else {
						Fail("unexpected reserved range in " + name);
					}
				} else if (word == "oneof") {
					Take();
					Expect("{");
					while (!Failed && Peek() != "}") {
						std::string type = Take();
						Parse_Field(fields, Label::ONEOF, type);
					}
					Expect("}");
				} else if (word == "optional" || word == "repeated") {
					std::string type = Take();
					Parse_Field(fields, (word == "optional") ? Label::PRESENCE : Label::REPEATED, type);
				} else {
					Parse_Field(fields, Label::PLAIN, word);
				}
			}
			Expect("}");
		}

		void Parse_Enum(ProtoSchema & schema)
		{
			std::string name = Take();
			auto & values = schema.Enums[name];
			Expect("{");
			while (!Failed && Peek() != "}") {
				std::string value = Take();
				Expect("=");
				values[value] = int(Number());
				Expect(";");
			}
			Expect("}");
		}

		std::vector<std::string> Tokens;
		std::size_t Position;
		bool Failed;
};


ProtoSchema Load_Schema(char const * path)
{
	std::ifstream file(path);
	REQUIRE_MESSAGE(file.good(), "cannot open ", path);
	std::stringstream text;
	text << file.rdbuf();

	ProtoSchema schema;
	ProtoParser parser(Tokenize(text.str()));
	bool parsed = parser.Parse(schema);
	REQUIRE_MESSAGE(parsed, path, ": ", parser.Problem);
	return(schema);
}


template<std::size_t N>
void Check_Fields(ProtoSchema const & schema, FieldEntry const (&table)[N])
{
	REQUIRE(!schema.Messages.empty());

	std::map<std::string, std::size_t> listed;
	for (FieldEntry const & entry : table) {
		std::string const where = std::string(entry.Message) + "." + entry.Name;
		listed[entry.Message]++;
		auto message = schema.Messages.find(entry.Message);
		REQUIRE_MESSAGE(message != schema.Messages.end(), "the schema has no message ", std::string(entry.Message));
		auto field = message->second.find(entry.Name);
		REQUIRE_MESSAGE(field != message->second.end(), "the schema has no field ", where);
		CHECK_MESSAGE(field->second.Number == entry.Number, where, " is numbered ", field->second.Number, " in the schema");
		CHECK_MESSAGE(field->second.Type == entry.Type, where, " is of type ", field->second.Type, " in the schema");
		CHECK_MESSAGE(field->second.Kind == entry.Kind, where, " has another label in the schema");
	}

	for (auto const & [name, fields] : schema.Messages) {
		CHECK_MESSAGE(listed[name] == fields.size(), "the table lists ", listed[name], " of the ", fields.size(), " fields of ", name);
		for (auto const & [field, value] : fields) {
			bool shared = value.Number >= 1 && value.Number <= 99;
			CHECK_MESSAGE(shared, name, ".", field, " is outside the shared field numbers");
		}
	}
}


template<std::size_t N>
void Check_Enums(ProtoSchema const & schema, EnumEntry const (&table)[N])
{
	std::map<std::string, std::size_t> listed;
	for (EnumEntry const & entry : table) {
		listed[entry.Enum]++;
		auto values = schema.Enums.find(entry.Enum);
		REQUIRE_MESSAGE(values != schema.Enums.end(), "the schema has no enum ", std::string(entry.Enum));
		auto value = values->second.find(entry.Name);
		REQUIRE_MESSAGE(value != values->second.end(), "the schema has no value ", std::string(entry.Name));
		CHECK_MESSAGE(value->second == entry.Value, std::string(entry.Name), " is ", value->second, " in the schema");
	}

	for (auto const & [name, values] : schema.Enums) {
		CHECK_MESSAGE(listed[name] == values.size(), "the table lists ", listed[name], " of the ", values.size(), " values of ", name);
	}
}

}


TEST_CASE("the field table matches replay.proto")
{
	Check_Fields(Load_Schema(REPLAY_PROTO_FILE), FIELDS);
}


TEST_CASE("the enumerations match replay.proto")
{
	Check_Enums(Load_Schema(REPLAY_PROTO_FILE), ENUMS);
}


TEST_CASE("the launch field table matches launch.proto")
{
	Check_Fields(Load_Schema(LAUNCH_PROTO_FILE), LAUNCH_FIELDS);
}


TEST_CASE("the launch enumerations match launch.proto")
{
	Check_Enums(Load_Schema(LAUNCH_PROTO_FILE), LAUNCH_ENUMS);
}
