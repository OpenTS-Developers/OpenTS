/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "always.h"

#include "launcher.h"

#include "crc.h"
#include "dbgprint.h"
#include "except.h"
#include "gamedirs.h"
#include "globals.h"
#include "headless.h"
#include "house.h"
#include "init.h"
#include "language/language.h"
#include "launch/gamelaunch.h"
#include "launch/launchmessages.h"
#include "launcherapply.h"
#include "mapcapture.h"
#include "msgbox.h"
#include "scenario.h"
#include "session.h"
#include "spawner.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>
#include <system_error>
#include <variant>
#include <windows.h>


namespace {

// Raise when the engine plays the same game differently, so that sync reports from the two
// versions show different identities.
constexpr int SESSION_IDENTITY_VERSION = 3;

std::string RequestFile;
bool IsSpawnFileRequested = false;
char const * ConflictingSwitch = nullptr;
Launch::Message::Request Request;
bool IsGameRequested = false;
bool IsGameConsumed = false;
bool IsFromSpawnFile = false;
int SessionIdentity = 0;
bool IsHost = false;
bool IsCapture = false;
std::string RefusalText;

// Empty when the request gives no result file, or once the result is written.
std::string ResultFile;
Launch::Message::Result Result;
bool IsScenarioOpen = false;
bool IsScenarioJoined = false;
// The code a scenario's end gives the result in place of RESULT_CODE_OK.
Launch::ResultCode EndCode = Launch::ResultCode::RESULT_CODE_UNKNOWN;


bool Read_Request_File(std::string const & path, Launch::Bytes & bytes)
{
	std::FILE * file = std::fopen(path.c_str(), "rb");
	if (file == nullptr) {
		return(false);
	}

	// One byte past the limit is enough for the decoder to refuse the file as too large.
	bytes.resize(Launch::REQUEST_MAX_SIZE + 1);
	std::size_t const read = std::fread(bytes.data(), 1, bytes.size(), file);
	bool const ok = std::ferror(file) == 0;
	std::fclose(file);
	bytes.resize(read);
	return(ok);
}


/// <summary>
/// Replaces the file at path with bytes, creating its folder when it is missing. A reader never
/// sees a partly written file.
/// </summary>
bool Write_File_Whole(std::string const & path, Launch::Bytes const & bytes)
{
	std::error_code error;
	std::filesystem::path const folder = std::filesystem::path(path).parent_path();
	if (!folder.empty()) {
		std::filesystem::create_directories(folder, error);
	}

	std::string const temporary = path + ".tmp";
	return(Replace_File(path.c_str(), temporary.c_str(), bytes.data(), unsigned(bytes.size())));
}


std::string Absolute_Path(std::string const & path)
{
	std::error_code error;
	std::filesystem::path const absolute = std::filesystem::absolute(path, error);
	return(error ? path : absolute.string());
}


/// <summary>
/// Returns the session identity of a game, the same on every machine given the same game.
/// </summary>
int Identity_Of(Launch::Game const & game)
{
	Launch::Bytes bytes;
	Launch::Encode(game.To_Message(), bytes);

	CRCEngine crc;
	crc(SESSION_IDENTITY_VERSION);
	crc(bytes.data(), int(bytes.size()));
	return(crc());
}


/// <summary>
/// Registers what a crash writes to the result file: the result so far, as a crash. A crash
/// writes nothing when there is no result file or the result already has a code.
/// </summary>
void Register_Crash_Result(void)
{
	if (ResultFile.empty() || Result.Code != Launch::ResultCode::RESULT_CODE_UNKNOWN) {
		Exception_Register_Result(nullptr, nullptr, 0, 0);
		return;
	}

	Launch::Bytes bytes;
	Launch::Encode(Launch::Crash_Result(Result), bytes);
	if (!Exception_Register_Result(ResultFile.c_str(), bytes.data(), unsigned(bytes.size()), Launch::RESULT_CRASH_PATH_FIELD)) {
		DebugString("[Launch] A crash now writes no result file; the result or its path is too long.\n");
	}
}


/// <summary>
/// Records a refusal in the result, and for Launch::Refusal when there is no result file.
/// </summary>
/// <returns>false, so a caller can refuse and return in one statement.</returns>
bool Record_Refusal(Launch::ResultCode code, std::string const & text)
{
	DebugString("[Launch] Refusing the launch request: %s\n", text.c_str());
	Result.Code = code;
	Result.Message = text;
	if (ResultFile.empty()) {
		RefusalText = text;
	}
	Register_Crash_Result();
	return(false);
}


/// <summary>
/// Refuses a play or load request after the game window exists: in the result file when there
/// is one, and otherwise in a message box.
/// </summary>
bool Refuse_In_Game(Launch::ResultCode code, std::string const & text)
{
	Record_Refusal(code, text);
	if (ResultFile.empty()) {
		WWMessageBox().Process(text.c_str(), TXT_OK);
	}
	return(false);
}


void Start_Capture(Launch::Message::Capture const & capture)
{
	IsCapture = true;
	if (!capture.DescriptionPath.empty()) {
		MapCapture::Request_Description(capture.DescriptionPath.c_str());
		return;
	}

	MapCapture::Request_Map(capture.MapPath.c_str());
	MapCapture::Request_Output(Launcher_File_Name(capture.OutPath.c_str()).c_str());
}

}


/// <summary>
/// Returns whether the command line asks for a launched game in place of the menus: true for
/// -SPAWN from the start, and for a play or load request once Launch::Read_Request accepts it.
/// </summary>
bool Launch::Is_Game_Requested(void)
{
	return(IsSpawnFileRequested || IsGameRequested);
}


bool Launch::Replaces_Menu(void)
{
	return(Is_Game_Requested() || Headless::Is_Active());
}


/// <summary>
/// Returns whether the game being set up or played comes from SPAWN.INI or a play or load
/// request. True from the start of Launch::Prepare.
/// </summary>
bool Launch::Is_Active(void)
{
	return(IsGameConsumed);
}


/// <summary>
/// Returns whether SPAWN.INI was read for the game being set up or played, even when its
/// request was then refused.
/// </summary>
bool Launch::Is_From_Spawn_File(void)
{
	return(IsFromSpawnFile);
}


/// <summary>
/// Returns the session identity of the launched game, or 0 unless a play request from -LAUNCH=
/// or SPAWN.INI started the game; a resume has none.
/// </summary>
int Launch::Session_Identity(void)
{
	return(SessionIdentity);
}


/// <summary>
/// Sets the request file -LAUNCH= names. A relative path is taken from the game's folder.
/// </summary>
void Launch::Request_File(char const * path)
{
	RequestFile = path;
}


void Launch::Request_Spawn_File(void)
{
	IsSpawnFileRequested = true;
}


/// <summary>
/// Records a switch that -LAUNCH= refuses to run with. Only the first switch is kept.
/// </summary>
void Launch::Note_Switch(char const * name)
{
	if (ConflictingSwitch == nullptr) {
		ConflictingSwitch = name;
	}
}


/// <summary>
/// Reads and checks the request file -LAUNCH= names, deletes any file at its result path, and
/// requests the capture a capture request asks for. Launch::Prepare applies a play or load
/// request. Does nothing without -LAUNCH=.
/// </summary>
/// <returns>bool; Can startup go on? False when the request is refused.</returns>
bool Launch::Read_Request(void)
{
	if (RequestFile.empty()) {
		return(true);
	}

	if (IsSpawnFileRequested) {
		Launch::Note_Switch("-SPAWN");
	}

	Launch::Bytes bytes;
	if (!Read_Request_File(RequestFile, bytes)) {
		return(Record_Refusal(Launch::ResultCode::RESULT_CODE_REFUSED,
			"The launch request " + RequestFile + " is missing or cannot be read."));
	}

	// A request that fails to decode still gives its result path and request id if they can be read.
	Launch::RequestFault const fault = Launch::Decode_Request(bytes, Request);
	Result.RequestId = Request.RequestId;
	if (!Request.ResultPath.empty()) {
		ResultFile = Launcher_File_Name(Request.ResultPath.c_str());
		DeleteFileA(ResultFile.c_str());
	}

	std::string refusal;
	if (!Launch::Judge_Request(fault, Request, ConflictingSwitch, refusal)) {
		return(Record_Refusal(Launch::ResultCode::RESULT_CODE_REFUSED, refusal));
	}

	if (std::holds_alternative<Launch::Message::Play>(Request.Verb) || std::holds_alternative<Launch::Message::Load>(Request.Verb)) {
		IsGameRequested = true;
	} else if (Launch::Message::Capture const * capture = std::get_if<Launch::Message::Capture>(&Request.Verb)) {
		Start_Capture(*capture);
	}

	Register_Crash_Result();

	DebugString("[Launch] Read the launch request %s.\n", RequestFile.c_str());
	return(true);
}


/// <summary>
/// Sets up the game SPAWN.INI or a play or load request describes, in place of the menu.
/// Returns false when the launch is refused, and on every call after the first, so the process
/// exits without showing the menu once that game ends.
/// </summary>
/// <param name="gameloaded">Set when the launch resumed a saved game.</param>
/// <returns>bool; Is a game ready to start?</returns>
bool Launch::Prepare(bool & gameloaded)
{
	if (!(IsSpawnFileRequested || IsGameRequested) || IsGameConsumed) {
		return(false);
	}
	IsGameConsumed = true;

	// A player's country is a position in the rules' country list, so the roster is read first.
	Prepare_Side_Roster();

	std::string fault;
	std::optional<Spawner::Config> spawn_file;
	if (IsSpawnFileRequested) {
		bool const read = Spawner::Read_Request(Request, spawn_file, fault);
		IsFromSpawnFile = spawn_file.has_value();
		if (!read) {
			return(Refuse_In_Game(Launch::ResultCode::RESULT_CODE_REFUSED, fault));
		}
	}

	// A refusal of SPAWN.INI's request starts with the file's section and key.
	Launch::FaultSite site;
	auto const refuse = [&](void) {
		return(Refuse_In_Game(Launch::ResultCode::RESULT_CODE_REFUSED, spawn_file ? spawn_file->Describe_Fault(site, fault) : fault));
	};

	if (Launch::Message::Load const * load = std::get_if<Launch::Message::Load>(&Request.Verb)) {
		if (!Launch::Judge_Load(*load, fault, &site)) {
			return(refuse());
		}
		if (!Launch::Apply_Load(*load, gameloaded, fault)) {
			return(Refuse_In_Game(Launch::ResultCode::RESULT_CODE_FAILED, fault));
		}
		IsHost = load->Session.Host;

		DebugString("[Launch] Resuming %s.\n", load->Save.c_str());
		Launch::Scenario_Started();
		return(true);
	}

	Launch::Message::Play const & play = std::get<Launch::Message::Play>(Request.Verb);
	Launch::Game game;
	Launch::Machine machine;
	if (!game.From_Message(play.Game, Launch::Limits(), fault, &site) || !machine.From_Request(play, game, fault, &site)) {
		return(refuse());
	}
	if (!Launch::Apply(game, machine, fault)) {
		return(Refuse_In_Game(Launch::ResultCode::RESULT_CODE_REFUSED, fault));
	}
	IsHost = machine.Host;
	SessionIdentity = Identity_Of(game);

	DebugString("[Launch] Launching %s with session identity %08x.\n", Scen->ScenarioName, SessionIdentity);

	// The network opens last, once the session is fully set up.
	if (Session.Type == GAME_INTERNET && !Launch::Wire_Network(machine, fault)) {
		return(Refuse_In_Game(Launch::ResultCode::RESULT_CODE_FAILED, fault));
	}

	return(true);
}


/// <summary>
/// Sends the host announcement in a launched network game whose request made this machine the
/// host; otherwise does nothing.
/// </summary>
void Launch::Announce_Master(void)
{
	if (IsGameConsumed && IsHost && Session.Type == GAME_INTERNET) {
		Session.Announce_Master();
	}
}


/// <summary>
/// Adds the scenario that has just started to the result, after closing an entry still open as
/// left. Does nothing unless a play or load request gives a result file.
/// </summary>
void Launch::Scenario_Started(void)
{
	if (ResultFile.empty() || !IsGameConsumed) {
		return;
	}

	Launch::Scenario_Ended(Launch::ScenarioEnd::LEFT);
	Result.Scenarios.push_back(Launch::Scenario_Result(Scen->ScenarioName));
	IsScenarioOpen = true;
	IsScenarioJoined = false;
	Register_Crash_Result();
}


/// <summary>
/// Records that every machine has passed the open scenario's start wait, so that a wait that
/// fails later ends the scenario with a lost connection or a desync, not as never started.
/// </summary>
void Launch::Scenario_Joined(void)
{
	IsScenarioJoined = true;
}


/// <summary>
/// Closes the open scenario entry with how it ended. The first call after a start decides the
/// entry; later calls before the next start change nothing.
/// </summary>
void Launch::Scenario_Ended(Launch::ScenarioEnd end)
{
	if (!IsScenarioOpen || Result.Scenarios.empty()) {
		return;
	}

	Launch::Close_Scenario(Result.Scenarios.back(), end, IsScenarioJoined, PlayerPtr != NULL && PlayerPtr->IsObserver, std::uint32_t(Frame));
	if (EndCode == Launch::ResultCode::RESULT_CODE_UNKNOWN) {
		EndCode = Launch::Scenario_End_Code(end, IsScenarioJoined);
	}
	IsScenarioOpen = false;
	Register_Crash_Result();
}


/// <summary>
/// Returns the refusal to show in the startup error box, or NULL when there is none or a
/// result file reports it.
/// </summary>
char const * Launch::Refusal(void)
{
	return(RefusalText.empty() ? NULL : RefusalText.c_str());
}


bool Launch::Has_Result_File(void)
{
	return(!ResultFile.empty());
}


/// <summary>
/// Writes the result file at the end of the run, when the request names one. Later calls write
/// nothing.
/// </summary>
void Launch::Write_Result(void)
{
	if (ResultFile.empty()) {
		return;
	}

	if (Result.Code == Launch::ResultCode::RESULT_CODE_UNKNOWN) {
		if (IsCapture) {
			if (Headless::Exit_Code() == EXIT_SUCCESS) {
				Result.Code = Launch::ResultCode::RESULT_CODE_OK;
				Result.OutputPath = Absolute_Path(MapCapture::Output_Path());
			} else {
				Result.Code = Launch::ResultCode::RESULT_CODE_FAILED;
				Result.Message = *Headless::Failure() != '\0' ? Headless::Failure() : "The capture failed.";
			}
		} else if (IsGameRequested && !IsGameConsumed) {
			Result.Code = Launch::ResultCode::RESULT_CODE_FAILED;
			Result.Message = "The game stopped before it set up the launch.";
		} else if (Result.Scenarios.empty()) {
			Result.Code = Launch::ResultCode::RESULT_CODE_FAILED;
			Result.Message = "The game ended before its scenario started.";
		} else if (EndCode == Launch::ResultCode::RESULT_CODE_START_MISMATCH) {
			Result.Code = EndCode;
			Result.Message = "The machines' start states differed, and the game ended before it started.";
		} else {
			Result.Code = Launch::ResultCode::RESULT_CODE_OK;
		}
	}

	Launch::Bytes bytes;
	Launch::Encode(Result, bytes);
	if (!Write_File_Whole(ResultFile, bytes)) {
		DebugString("[Launch] The result file %s could not be written.\n", ResultFile.c_str());
	}
	ResultFile.clear();
	Register_Crash_Result();
}
