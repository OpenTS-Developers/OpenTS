/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstdint>

// The field numbers of docs/launch/launch.proto, one namespace per message. Each number must
// match the schema, since the numbers are the wire format.

namespace Launch::Field
{
	namespace Game
	{
		constexpr std::uint32_t KIND = 1;
		constexpr std::uint32_t SEED = 2;
		constexpr std::uint32_t PLAYERS = 3;
		constexpr std::uint32_t OPTIONS = 4;
		constexpr std::uint32_t ANONYMOUS = 5;
		constexpr std::uint32_t GLOBAL_FLAGS = 6;
	}

	namespace Player
	{
		constexpr std::uint32_t HOUSE = 1;
		constexpr std::uint32_t NAME = 2;
		constexpr std::uint32_t HUMAN = 3;
		constexpr std::uint32_t OBSERVER = 4;
		constexpr std::uint32_t COUNTRY = 5;
		constexpr std::uint32_t COUNTRY_NAME = 6;
		constexpr std::uint32_t COLOR = 7;
		constexpr std::uint32_t START = 8;
		constexpr std::uint32_t AI_LEVEL = 9;
		constexpr std::uint32_t ALLIES = 10;
		constexpr std::uint32_t RESOLVED_COUNTRY = 11;
		constexpr std::uint32_t RESOLVED_COLOR = 12;
		constexpr std::uint32_t RESOLVED_START = 13;
	}

	namespace Option
	{
		constexpr std::uint32_t KEY = 1;
		constexpr std::uint32_t NUMBER = 2;
		constexpr std::uint32_t FLAG = 3;
		constexpr std::uint32_t TEXT = 4;
	}

	namespace FileHash
	{
		constexpr std::uint32_t PATH = 1;
		constexpr std::uint32_t HASH = 2;
	}

	namespace Request
	{
		constexpr std::uint32_t VERSION = 1;
		constexpr std::uint32_t PLAY = 2;
		constexpr std::uint32_t LOAD = 3;
		constexpr std::uint32_t REPLAY = 4;
		constexpr std::uint32_t VERIFY = 5;
		constexpr std::uint32_t CAPTURE = 6;
		constexpr std::uint32_t RESULT_PATH = 7;
		constexpr std::uint32_t CHANNEL = 8;
		constexpr std::uint32_t REQUEST_ID = 9;
	}

	namespace Play
	{
		constexpr std::uint32_t GAME = 1;
		constexpr std::uint32_t MAP_PATH = 2;
		constexpr std::uint32_t SESSION = 3;
		constexpr std::uint32_t NETWORK = 4;
		constexpr std::uint32_t RECORDING = 5;
		constexpr std::uint32_t FIXTURE = 6;
	}

	namespace Session
	{
		constexpr std::uint32_t LOCAL_PLAYER = 1;
		constexpr std::uint32_t HOST = 2;
		constexpr std::uint32_t AUTOSAVE_INTERVAL = 3;
		constexpr std::uint32_t NEXT_CAMPAIGN_AUTOSAVE = 4;
		constexpr std::uint32_t NEXT_SKIRMISH_AUTOSAVE = 5;
		constexpr std::uint32_t SKIP_SCORE_SCREEN = 6;
		constexpr std::uint32_t WRITE_STATISTICS = 7;
		constexpr std::uint32_t GAME_ID = 8;
		constexpr std::uint32_t MAP_HASH = 9;
		constexpr std::uint32_t MAP_NAME = 10;
		constexpr std::uint32_t GAME_MODE = 11;
		constexpr std::uint32_t LOAD_SCREEN = 12;
		constexpr std::uint32_t DIFFICULTY_NAME = 13;
	}

	namespace LoadScreen
	{
		constexpr std::uint32_t IMAGE = 1;
		constexpr std::uint32_t X = 2;
		constexpr std::uint32_t Y = 3;
	}

	namespace Network
	{
		constexpr std::uint32_t PORT = 1;
		constexpr std::uint32_t PEERS = 2;
		constexpr std::uint32_t TUNNEL = 3;
		constexpr std::uint32_t PROTOCOL = 4;
		constexpr std::uint32_t FRAME_SEND_RATE = 5;
		constexpr std::uint32_t MAX_AHEAD = 6;
		constexpr std::uint32_t CONNECT_TIMEOUT = 7;
		constexpr std::uint32_t RECONNECT_TIMEOUT = 8;
	}

	namespace Peer
	{
		constexpr std::uint32_t PLAYER = 1;
		constexpr std::uint32_t ADDRESS = 2;
		constexpr std::uint32_t PORT = 3;
	}

	namespace Tunnel
	{
		constexpr std::uint32_t ADDRESS = 1;
		constexpr std::uint32_t PORT = 2;
	}

	namespace Recording
	{
		constexpr std::uint32_t ENABLED = 1;
		constexpr std::uint32_t FILE_OUT = 2;
		constexpr std::uint32_t PACKAGE = 3;
		constexpr std::uint32_t PACKAGE_VERSION = 4;
		constexpr std::uint32_t FILES = 5;
		constexpr std::uint32_t EMBED_MAP = 6;
	}

	namespace Fixture
	{
		constexpr std::uint32_t HEADLESS = 1;
		constexpr std::uint32_t STOP_FRAME = 2;
		constexpr std::uint32_t SCRIPT_PATH = 3;
		constexpr std::uint32_t CHECKPOINT_CHECK = 4;
	}

	namespace CheckpointCheck
	{
		constexpr std::uint32_t SAVE_FRAME = 1;
		constexpr std::uint32_t RUN_FRAMES = 2;
	}

	namespace Load
	{
		constexpr std::uint32_t SAVE = 1;
		constexpr std::uint32_t SESSION = 2;
		constexpr std::uint32_t NETWORK = 3;
		constexpr std::uint32_t PLAYERS = 4;
		constexpr std::uint32_t GAME_SPEED = 5;
		constexpr std::uint32_t ANONYMOUS = 6;
	}

	namespace Replay
	{
		constexpr std::uint32_t FILE = 1;
		constexpr std::uint32_t WATCH_PLAYER = 2;
		constexpr std::uint32_t FREE_VIEW = 3;
		constexpr std::uint32_t START_FRAME = 4;
		constexpr std::uint32_t MAP_PATH = 5;
		constexpr std::uint32_t LOAD_SCREEN = 6;
	}

	namespace Verify
	{
		constexpr std::uint32_t FILE = 1;
		constexpr std::uint32_t OUT_PATH = 2;
		constexpr std::uint32_t MAP_PATH = 3;
	}

	namespace Capture
	{
		constexpr std::uint32_t MAP_PATH = 1;
		constexpr std::uint32_t OUT_PATH = 2;
		constexpr std::uint32_t DESCRIPTION_PATH = 3;
	}

	namespace Result
	{
		constexpr std::uint32_t REQUEST_ID = 1;
		constexpr std::uint32_t CODE = 2;
		constexpr std::uint32_t MESSAGE = 3;
		constexpr std::uint32_t SCENARIOS = 4;
		constexpr std::uint32_t STATISTICS_PATH = 5;
		constexpr std::uint32_t DIFFERENT_BUILD = 6;
		constexpr std::uint32_t UNVERIFIED = 7;
		constexpr std::uint32_t MISMATCH_FRAME = 8;
		constexpr std::uint32_t OUTPUT_PATH = 9;
		constexpr std::uint32_t CRASH_PATH = 10;
	}

	namespace ScenarioResult
	{
		constexpr std::uint32_t MAP_PATH = 1;
		constexpr std::uint32_t OUTCOME = 2;
		constexpr std::uint32_t COMPLETED = 3;
		constexpr std::uint32_t LAST_FRAME = 4;
		constexpr std::uint32_t REPLAY_PATH = 5;
		constexpr std::uint32_t NOT_RECORDED = 6;
	}
}
