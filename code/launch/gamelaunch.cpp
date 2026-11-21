/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "launch/gamelaunch.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <set>
#include <type_traits>
#include <utility>
#include <variant>


using Launch::Fault;
using Launch::Is_Address;
using Launch::Is_Tunnel_Number;


/// <summary>Sets fault to a printf style description.</summary>
/// <returns>false, so a caller can set a fault and refuse in one statement.</returns>
bool Launch::Fault(std::string & fault, char const * format, ...)
{
	char buffer[256];

	va_list args;
	va_start(args, format);
	std::vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	fault = buffer;
	return(false);
}


/// <summary>Is this a dotted IPv4 address other than 0.0.0.0?</summary>
bool Launch::Is_Address(std::string const & text)
{
	unsigned quad[4] = {};
	char tail = '\0';

	if (std::sscanf(text.c_str(), "%u.%u.%u.%u%c", &quad[0], &quad[1], &quad[2], &quad[3], &tail) != 4) {
		return(false);
	}

	for (unsigned part : quad) {
		if (part > 255) {
			return(false);
		}
	}

	return(quad[0] != 0 || quad[1] != 0 || quad[2] != 0 || quad[3] != 0);
}


/// <summary>Accepts a tunnel number as the client writes it: nonzero, within sixteen bits either side of zero.</summary>
bool Launch::Is_Tunnel_Number(int value)
{
	return(value != 0 && value >= -65535 && value <= 65535);
}


namespace {

using G = Launch::Game;
using O = Launch::Game::OptionsType;

constexpr unsigned MATCH = G::KIND_SKIRMISH | G::KIND_MULTIPLAYER;
constexpr unsigned EVERY = G::KIND_CAMPAIGN | MATCH;

constexpr std::int64_t FLAG_LOW = 0;
constexpr std::int64_t FLAG_HIGH = 1;

// Sorted by key in ascending byte order, the order To_Message writes them in.
G::OptionEntry const OPTIONS[] = {
	{ "AIDifficulty", G::OptionType::NUMBER, MATCH, &O::AIDifficulty, 0, G::DIFFICULTY_COUNT - 1 },
	{ "AlliesAllowed", G::OptionType::FLAG, MATCH, &O::AlliesAllowed, FLAG_LOW, FLAG_HIGH },
	{ "AttackNeutralUnits", G::OptionType::FLAG, MATCH, &O::AttackNeutralUnits, FLAG_LOW, FLAG_HIGH },
	{ "AutoDeployMCV", G::OptionType::FLAG, MATCH, &O::AutoDeployMCV, FLAG_LOW, FLAG_HIGH },
	{ "Bases", G::OptionType::FLAG, MATCH, &O::Bases, FLAG_LOW, FLAG_HIGH },
	{ "BridgeDestruction", G::OptionType::FLAG, MATCH, &O::BridgeDestruction, FLAG_LOW, FLAG_HIGH },
	{ "BuildOffAlly", G::OptionType::FLAG, MATCH, &O::BuildOffAlly, FLAG_LOW, FLAG_HIGH },
	{ "Campaign", G::OptionType::NUMBER, G::KIND_CAMPAIGN, &O::Campaign, -1, INT_MAX },
	{ "CampaignComputerDifficulty", G::OptionType::NUMBER, G::KIND_CAMPAIGN, &O::CampaignComputerDifficulty, 0, G::DIFFICULTY_COUNT - 1 },
	{ "CampaignPlayerDifficulty", G::OptionType::NUMBER, G::KIND_CAMPAIGN, &O::CampaignPlayerDifficulty, 0, G::DIFFICULTY_COUNT - 1 },
	{ "CoachMode", G::OptionType::FLAG, MATCH, &O::CoachMode, FLAG_LOW, FLAG_HIGH },
	{ "Crates", G::OptionType::FLAG, MATCH, &O::Crates, FLAG_LOW, FLAG_HIGH },
	{ "Credits", G::OptionType::NUMBER, MATCH, &O::Credits, INT_MIN, INT_MAX },
	{ "Firestorm", G::OptionType::FLAG, EVERY, &O::Firestorm, FLAG_LOW, FLAG_HIGH },
	{ "FogOfWar", G::OptionType::FLAG, MATCH, &O::FogOfWar, FLAG_LOW, FLAG_HIGH },
	{ "GameSpeed", G::OptionType::NUMBER, EVERY, &O::GameSpeed, 0, G::GAME_SPEED_COUNT - 1 },
	{ "HarvesterTruce", G::OptionType::FLAG, MATCH, &O::HarvesterTruce, FLAG_LOW, FLAG_HIGH },
	{ "MCVRedeploy", G::OptionType::FLAG, MATCH, &O::MCVRedeploy, FLAG_LOW, FLAG_HIGH },
	{ "MultiEngineer", G::OptionType::FLAG, MATCH, &O::MultiEngineer, FLAG_LOW, FLAG_HIGH },
	{ "PlayMovies", G::OptionType::FLAG, MATCH, &O::PlayMovies, FLAG_LOW, FLAG_HIGH },
	{ "ScrapMetal", G::OptionType::FLAG, MATCH, &O::ScrapMetal, FLAG_LOW, FLAG_HIGH },
	{ "SharedControl", G::OptionType::FLAG, MATCH, &O::SharedControl, FLAG_LOW, FLAG_HIGH },
	{ "ShortGame", G::OptionType::FLAG, MATCH, &O::ShortGame, FLAG_LOW, FLAG_HIGH },
	{ "SurrenderOnDisconnect", G::OptionType::FLAG, MATCH, &O::SurrenderOnDisconnect, FLAG_LOW, FLAG_HIGH },
	{ "TechLevel", G::OptionType::NUMBER, MATCH, &O::TechLevel, INT_MIN, INT_MAX },
	{ "UnitCount", G::OptionType::NUMBER, MATCH, &O::UnitCount, INT_MIN, INT_MAX },
};


struct FaultOut {
	std::string & Text;
	Launch::FaultSite * Site;
};


bool Fault(FaultOut & fault, char const * format, ...)
{
	char buffer[256];

	va_list args;
	va_start(args, format);
	std::vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	fault.Text = buffer;
	return(false);
}


/// <summary>Sets the fault text, and its site when the caller asked for one; player is -1 for none.</summary>
bool Fault_At(FaultOut & fault, int player, std::string const & field, char const * format, ...)
{
	char buffer[256];

	va_list args;
	va_start(args, format);
	std::vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	fault.Text = buffer;
	if (fault.Site != nullptr) {
		fault.Site->Player = player;
		fault.Site->Field = field;
	}
	return(false);
}


bool Same_Name(std::string const & one, std::string const & two)
{
	return(std::equal(one.begin(), one.end(), two.begin(), two.end(), [](char a, char b) {
		return(std::tolower((unsigned char)a) == std::tolower((unsigned char)b));
	}));
}


unsigned Kind_Bit(Launch::GameKind kind)
{
	switch (kind) {
		case Launch::GameKind::GAME_KIND_CAMPAIGN: return(G::KIND_CAMPAIGN);
		case Launch::GameKind::GAME_KIND_SKIRMISH: return(G::KIND_SKIRMISH);
		case Launch::GameKind::GAME_KIND_MULTIPLAYER: return(G::KIND_MULTIPLAYER);
		default: return(0);
	}
}


char const * Kind_Name(unsigned kind)
{
	switch (kind) {
		case G::KIND_CAMPAIGN: return("campaign");
		case G::KIND_SKIRMISH: return("skirmish");
		default: return("multiplayer");
	}
}


bool Is_Port(int value)
{
	return(value >= 1 && value <= 65535);
}


G::OptionEntry const * Find_Option(std::string const & key)
{
	for (G::OptionEntry const & entry : OPTIONS) {
		if (key == entry.Key) {
			return(&entry);
		}
	}
	return(nullptr);
}


/// <summary>Checks one option of a request against the table and stores its value in options.</summary>
bool Take_Option(Launch::Message::Option const & option, unsigned kind, G::LimitsType const & limits, O & options, FaultOut & fault)
{
	G::OptionEntry const * entry = Find_Option(option.Key);
	if (entry == nullptr) {
		return(Fault_At(fault, -1, "options." + option.Key, "The launch sets the option %s, which this version does not read.", option.Key.c_str()));
	}
	if ((entry->Kinds & kind) == 0) {
		return(Fault_At(fault, -1, "options." + option.Key, "The option %s does not apply to a %s game.", entry->Key, Kind_Name(kind)));
	}

	std::int64_t value = 0;
	if (entry->Type == G::OptionType::FLAG) {
		bool const * flag = std::get_if<bool>(&option.Value);
		if (flag == nullptr) {
			return(Fault_At(fault, -1, "options." + option.Key, "The option %s takes a flag.", entry->Key));
		}
		value = *flag ? 1 : 0;
	} else {
		std::int64_t const * number = std::get_if<std::int64_t>(&option.Value);
		if (number == nullptr) {
			return(Fault_At(fault, -1, "options." + option.Key, "The option %s takes a number.", entry->Key));
		}
		value = *number;
	}

	std::int64_t highest = entry->Highest;
	if (std::strcmp(entry->Key, "Campaign") == 0) {
		highest = limits.Campaigns - 1;
	}
	if (value < entry->Lowest || value > highest) {
		return(Fault_At(fault, -1, "options." + option.Key, "The option %s is %lld, outside %lld to %lld.", entry->Key,
			(long long)value, (long long)entry->Lowest, (long long)highest));
	}

	std::visit([&](auto member) {
		using Value = std::remove_reference_t<decltype(options.*member)>;
		options.*member = Value(value);
	}, entry->Member);
	return(true);
}


/// <summary>Checks the player at index against the rules of a request and converts it.</summary>
bool Take_Player(Launch::Message::Game const & request, std::size_t index, unsigned kind, G::LimitsType const & limits, G::PlayerType & taken, FaultOut & fault)
{
	Launch::Message::Player const & player = request.Players[index];
	int const number = int(index);

	if (player.Name.size() > G::NAME_LENGTH_MAX) {
		return(Fault_At(fault, number, "name", "Player %d has a name longer than %d bytes.", number, G::NAME_LENGTH_MAX));
	}
	if (player.Human && player.Name.empty()) {
		return(Fault_At(fault, number, "name", "Player %d is human and has no name.", number));
	}
	if (player.Observer && !player.Human) {
		return(Fault_At(fault, number, "observer", "Player %d is a computer player and an observer; only a human player observes.", number));
	}
	if (player.AILevel) {
		if (player.Human) {
			return(Fault_At(fault, number, "ai_level", "Player %d is human and sets ai_level, which only a computer player takes.", number));
		}
		if (*player.AILevel < 0 || *player.AILevel >= G::DIFFICULTY_COUNT) {
			return(Fault_At(fault, number, "ai_level", "Player %d sets ai_level %d, outside 0 to %d.", number, *player.AILevel, G::DIFFICULTY_COUNT - 1));
		}
	}

	// A campaign's player takes the house the mission gives it, so these are not read.
	if (kind != G::KIND_CAMPAIGN) {
		if (player.Country < -1 || player.Country >= limits.Countries) {
			return(Fault_At(fault, number, "country", "Player %d asks for country %d; the rules have %d, numbered from 0.", number, player.Country, limits.Countries));
		}
		if (player.Color < -1 || player.Color >= limits.Colors) {
			return(Fault_At(fault, number, "color", "Player %d asks for color %d; the rules have %d, numbered from 0.", number, player.Color, limits.Colors));
		}
		if (player.Start < -1 || player.Start >= G::START_MAX) {
			return(Fault_At(fault, number, "start", "Player %d asks for start position %d, outside -1 to %d.", number, player.Start, G::START_MAX - 1));
		}
		if (player.Human && player.Country == -1) {
			return(Fault_At(fault, number, "country", "Player %d is human and asks for a random country; only a computer player may.", number));
		}
		if (player.Human && player.Color == -1) {
			return(Fault_At(fault, number, "color", "Player %d is human and asks for a random color; only a computer player may.", number));
		}
	}

	int previous = -1;
	for (std::uint32_t ally : player.Allies) {
		if (ally >= request.Players.size()) {
			return(Fault_At(fault, number, "allies", "Player %d lists ally %u, past the last player.", number, ally));
		}
		if (int(ally) == number) {
			return(Fault_At(fault, number, "allies", "Player %d lists itself as an ally.", number));
		}
		if (request.Players[ally].Observer) {
			return(Fault_At(fault, number, "allies", "Player %d lists observer %u as an ally.", number, ally));
		}
		if (int(ally) <= previous) {
			return(Fault_At(fault, number, "allies", "Player %d lists its allies out of ascending order or twice.", number));
		}
		previous = int(ally);
		taken.Allies.push_back(int(ally));
	}

	taken.Name = player.Name;
	taken.Human = player.Human;
	taken.Observer = player.Observer;
	taken.Country = player.Country;
	taken.Color = player.Color;
	taken.Start = player.Start;
	taken.AILevel = player.AILevel;
	return(true);
}

}


/// <summary>
/// Returns every option the engine reads from a launch, in ascending byte order of the key.
/// </summary>
std::span<Launch::Game::OptionEntry const> Launch::Game::Option_Table(void)
{
	return(OPTIONS);
}


/// <summary>
/// Stores the game a play request gives when it keeps every rule of launch.proto and every limit
/// of this version; otherwise sets fault to the first fault and leaves this unchanged.
/// </summary>
/// <param name="limits">The country, color, campaign, and global flag counts the request is checked against.</param>
/// <param name="fault">Where to leave the sentence describing the first fault found.</param>
bool Launch::Game::From_Message(Launch::Message::Game const & request, LimitsType const & limits, std::string & fault, Launch::FaultSite * site)
{
	FaultOut out{ fault, site };
	if (site != nullptr) {
		*site = Launch::FaultSite();
	}

	unsigned const kind = Kind_Bit(request.Kind);
	if (kind == 0) {
		return(Fault_At(out, -1, "kind", "The launch kind %d is not a campaign, a skirmish, or a multiplayer game.", int(request.Kind)));
	}
	if (request.Players.size() > PLAYER_MAX) {
		return(Fault_At(out, -1, "players", "The launch lists %d players; the most a game takes is %d.", int(request.Players.size()), PLAYER_MAX));
	}

	int const humans = int(std::count_if(request.Players.begin(), request.Players.end(),
		[](Launch::Message::Player const & player) { return(player.Human); }));
	if (kind == KIND_CAMPAIGN) {
		if (request.Players.size() != 1 || humans != 1) {
			return(Fault_At(out, -1, "players", "A campaign launch lists exactly one player, the human one."));
		}
	} else if (humans == 0) {
		return(Fault_At(out, -1, "players", "The launch lists no human player."));
	} else if ((humans > 1) != (kind == KIND_MULTIPLAYER)) {
		return(Fault_At(out, -1, "kind", "A launch with %d human players is not a %s game.", humans, Kind_Name(kind)));
	}
	if (kind == KIND_MULTIPLAYER && !request.Seed) {
		return(Fault_At(out, -1, "seed", "A multiplayer launch needs a seed."));
	}

	Launch::Game game;
	game.Kind = request.Kind;
	game.Seed = request.Seed;
	game.Anonymous = request.Anonymous;

	for (std::size_t index = 0; index < request.Players.size(); index++) {
		PlayerType player;
		if (!Take_Player(request, index, kind, limits, player, out)) {
			return(false);
		}
		game.Players.push_back(std::move(player));
	}

	// The engine creates human houses in ascending color, so a player's position is its house only
	// when the request lists them that way.
	if (kind != KIND_CAMPAIGN) {
		for (std::size_t index = 1; index < game.Players.size(); index++) {
			PlayerType const & player = game.Players[index];
			PlayerType const & before = game.Players[index - 1];
			if (player.Human && !before.Human) {
				return(Fault_At(out, int(index), "human", "Player %d is human and follows a computer player; human players come first.", int(index)));
			}
			if (player.Human && player.Color <= before.Color) {
				return(Fault_At(out, int(index), "color", "Player %d has color %d; each human player needs a higher color than the one before.", int(index), player.Color));
			}
		}
	}

	if (kind == KIND_MULTIPLAYER) {
		for (std::size_t index = 0; index < game.Players.size(); index++) {
			for (std::size_t other = 0; other < index; other++) {
				if (game.Players[index].Human && game.Players[other].Human && Same_Name(game.Players[other].Name, game.Players[index].Name)) {
					return(Fault_At(out, int(index), "name", "Players %d and %d have the same name.", int(other), int(index)));
				}
			}
		}
	}

	std::set<std::string> seen;
	for (Launch::Message::Option const & option : request.Options) {
		if (!seen.insert(option.Key).second) {
			return(Fault_At(out, -1, "options." + option.Key, "The launch sets the option %s twice.", option.Key.c_str()));
		}
		if (!Take_Option(option, kind, limits, game.Options, out)) {
			return(false);
		}
	}

	if (kind != KIND_CAMPAIGN && !request.GlobalFlags.empty()) {
		return(Fault_At(out, -1, "global_flags", "The launch sets global flags; only a campaign game takes them."));
	}
	for (std::size_t index = 0; index < request.GlobalFlags.size(); index++) {
		std::uint32_t const flag = request.GlobalFlags[index];
		if (flag >= std::uint32_t(limits.GlobalFlags)) {
			return(Fault_At(out, -1, "global_flags", "The launch sets global flag %u; a scenario has %d, numbered from 0.", flag, limits.GlobalFlags));
		}
		if (index > 0 && flag <= request.GlobalFlags[index - 1]) {
			return(Fault_At(out, -1, "global_flags", "The launch lists global flag %u out of ascending order or twice.", flag));
		}
		game.GlobalFlags.push_back(int(flag));
	}

	*this = std::move(game);
	return(true);
}


/// <summary>
/// Returns the game in its request form, with every option that applies to its kind, defaults
/// included, in ascending key order. Two games with the same values encode to the same bytes.
/// </summary>
Launch::Message::Game Launch::Game::To_Message(void) const
{
	Launch::Message::Game launch;
	launch.Kind = Kind;
	launch.Seed = Seed;
	launch.Anonymous = Anonymous;

	for (PlayerType const & player : Players) {
		Launch::Message::Player message;
		message.Name = player.Name;
		message.Human = player.Human;
		message.Observer = player.Observer;
		message.Country = player.Country;
		message.Color = player.Color;
		message.Start = player.Start;
		message.AILevel = player.AILevel;
		for (int ally : player.Allies) {
			message.Allies.push_back(std::uint32_t(ally));
		}
		launch.Players.push_back(std::move(message));
	}

	unsigned const kind = Kind_Bit(Kind);
	for (OptionEntry const & entry : OPTIONS) {
		if ((entry.Kinds & kind) == 0) {
			continue;
		}
		Launch::Message::Option option;
		option.Key = entry.Key;
		std::visit([&](auto member) {
			if (entry.Type == OptionType::FLAG) {
				option.Value = Options.*member != 0;
			} else {
				option.Value = std::int64_t(Options.*member);
			}
		}, entry.Member);
		launch.Options.push_back(std::move(option));
	}
	launch.GlobalFlags.assign(GlobalFlags.begin(), GlobalFlags.end());
	return(launch);
}


/// <summary>
/// Returns whether this version runs a request, before anything is applied. Refuses a request
/// that did not decode, a verb or a part of one this version cannot run, and a capture request
/// with the wrong combination of paths. The rules of a play request's game are checked later,
/// by Game::From_Message, and those of a load request by Machine::From_Load.
/// </summary>
/// <param name="fault">What Decode_Request reported.</param>
/// <param name="refusal">Where to leave the sentence describing why the request is refused.</param>
/// <returns>bool; Does this version run the request?</returns>
bool Launch::Judge_Request(Launch::RequestFault fault, Launch::Message::Request const & request, std::string & refusal)
{
	switch (fault) {
		case Launch::RequestFault::NONE:
			break;
		case Launch::RequestFault::VERSION:
			return(Fault(refusal, "The launch request has version %u; this version runs version %u.",
				request.Version, Launch::REQUEST_VERSION));
		case Launch::RequestFault::NO_VERB:
			return(Fault(refusal, "The launch request asks for nothing this version can do."));
		case Launch::RequestFault::TEXT:
			return(Fault(refusal, "The launch request has text that is not UTF-8."));
		default:
			return(Fault(refusal, "The launch request is not a file this version can read."));
	}

	if (std::holds_alternative<Launch::Message::Replay>(request.Verb)) {
		return(Fault(refusal, "This version cannot play a replay."));
	}
	if (std::holds_alternative<Launch::Message::Verify>(request.Verb)) {
		return(Fault(refusal, "This version cannot verify a replay."));
	}

	if (Launch::Message::Play const * play = std::get_if<Launch::Message::Play>(&request.Verb)) {
		if (play->Fixture) {
			return(Fault(refusal, "The play request sets fixture, which this version does not run."));
		}
	}

	if (Launch::Message::Capture const * capture = std::get_if<Launch::Message::Capture>(&request.Verb)) {
		bool const shorthand = !capture->MapPath.empty() || !capture->OutPath.empty();
		if (!capture->DescriptionPath.empty() && shorthand) {
			return(Fault(refusal, "The capture request sets description_path together with map_path or out_path."));
		}
		if (capture->DescriptionPath.empty() && (capture->MapPath.empty() || capture->OutPath.empty())) {
			return(Fault(refusal, "The capture request needs map_path and out_path, or description_path alone."));
		}
	}

	refusal.clear();
	return(true);
}


/// <summary>
/// Returns the result entry for a scenario that has just started. This version writes no
/// replay, so the entry says recording was off.
/// </summary>
Launch::Message::ScenarioResult Launch::Scenario_Result(std::string const & map_path)
{
	Launch::Message::ScenarioResult result;
	result.MapPath = map_path;
	result.NotRecorded = Launch::NotRecorded::NOT_RECORDED_DISABLED;
	return(result);
}


/// <summary>
/// Gives a scenario's result entry its outcome, whether it was completed, and its last frame.
/// CONNECTION_LOST, WAIT_CANCELLED, and START_MISMATCH before every machine joined give
/// GAME_OUTCOME_NOT_STARTED and no last frame. An observer gets GAME_OUTCOME_UNKNOWN in place
/// of a win, a loss, or GAME_OUTCOME_ABORTED, and every other outcome as a player does.
/// </summary>
/// <param name="joined">Did every machine pass the scenario's start wait? Read only for
/// CONNECTION_LOST, WAIT_CANCELLED, and START_MISMATCH.</param>
/// <param name="frame">The frame counter when the scenario ended.</param>
void Launch::Close_Scenario(Launch::Message::ScenarioResult & entry, Launch::ScenarioEnd end, bool joined, bool observer, std::uint32_t frame)
{
	using Launch::GameOutcome;
	using Launch::ScenarioEnd;

	GameOutcome outcome = GameOutcome::GAME_OUTCOME_ABORTED;
	switch (end) {
		case ScenarioEnd::LEFT:
			outcome = GameOutcome::GAME_OUTCOME_ABORTED;
			break;
		case ScenarioEnd::WON:
			outcome = GameOutcome::GAME_OUTCOME_WON;
			break;
		case ScenarioEnd::LOST:
			outcome = GameOutcome::GAME_OUTCOME_LOST;
			break;
		case ScenarioEnd::DESYNC:
			outcome = GameOutcome::GAME_OUTCOME_DESYNC;
			break;
		case ScenarioEnd::CONNECTION_LOST:
			outcome = joined ? GameOutcome::GAME_OUTCOME_CONNECTION_LOST : GameOutcome::GAME_OUTCOME_NOT_STARTED;
			break;
		case ScenarioEnd::WAIT_CANCELLED:
			outcome = joined ? GameOutcome::GAME_OUTCOME_ABORTED : GameOutcome::GAME_OUTCOME_NOT_STARTED;
			break;
		case ScenarioEnd::START_MISMATCH:
			outcome = joined ? GameOutcome::GAME_OUTCOME_DESYNC : GameOutcome::GAME_OUTCOME_NOT_STARTED;
			break;
	}

	bool const personal = outcome == GameOutcome::GAME_OUTCOME_WON || outcome == GameOutcome::GAME_OUTCOME_LOST
		|| outcome == GameOutcome::GAME_OUTCOME_ABORTED;
	entry.Outcome = observer && personal ? GameOutcome::GAME_OUTCOME_UNKNOWN : outcome;
	entry.Completed = end == ScenarioEnd::WON || end == ScenarioEnd::LOST;
	if (outcome == GameOutcome::GAME_OUTCOME_NOT_STARTED) {
		entry.LastFrame.reset();
	} else {
		entry.LastFrame = frame;
	}
}


/// <summary>
/// Returns the result code a scenario's end gives the whole launch: RESULT_CODE_START_MISMATCH
/// when the machines' start states differed before every machine joined, and otherwise
/// RESULT_CODE_UNKNOWN, which leaves the code to the rest of the launch.
/// </summary>
Launch::ResultCode Launch::Scenario_End_Code(Launch::ScenarioEnd end, bool joined)
{
	if (end == Launch::ScenarioEnd::START_MISMATCH && !joined) {
		return(Launch::ResultCode::RESULT_CODE_START_MISMATCH);
	}
	return(Launch::ResultCode::RESULT_CODE_UNKNOWN);
}


/// <summary>
/// Returns whether the local player won a scenario the game decided: PlayerWins in a campaign
/// mission, and otherwise whether the local house is undefeated, because there PlayerWins follows
/// the house the game flagged to win, which can be the defeated opponent.
/// </summary>
bool Launch::Local_Win(bool campaign, bool player_wins, bool local_defeated)
{
	return(campaign ? player_wins : !local_defeated);
}


int Launch::Game::Human_Count(void) const
{
	return(int(std::count_if(Players.begin(), Players.end(), [](PlayerType const & player) { return(player.Human); })));
}


/// <summary>
/// Converts a wait a request gives in milliseconds into 16 ms ticks within the bounds the game
/// accepts, or returns fallback for 0.
/// </summary>
int Launch::Machine::Timeout_Ticks(std::uint32_t milliseconds, int fallback)
{
	if (milliseconds == 0) {
		return(fallback);
	}
	return(int(std::clamp<std::uint32_t>(milliseconds / 16, TIMEOUT_MIN, TIMEOUT_MAX)));
}


namespace {

using M = Launch::Machine;


/// <summary>Converts the session values that play and load requests share.</summary>
bool Take_Session(Launch::Message::Session const & session, M & machine, FaultOut & fault)
{
	for (std::uint32_t value : { session.AutosaveInterval, session.NextCampaignAutosave, session.NextSkirmishAutosave }) {
		if (value > std::uint32_t(INT_MAX)) {
			return(Fault(fault, "The session gives an autosave value of %u, past %d.", value, INT_MAX));
		}
	}

	machine.Host = session.Host;
	machine.MapName = session.MapName;
	machine.GameMode = session.GameMode;
	machine.AutosaveInterval = int(session.AutosaveInterval);
	machine.NextCampaignAutosave = int(session.NextCampaignAutosave);
	machine.NextSkirmishAutosave = int(session.NextSkirmishAutosave);
	machine.SkipScoreScreen = session.SkipScoreScreen;
	machine.WriteStatistics = session.WriteStatistics;
	machine.GameId = session.GameId;
	machine.MapHash = session.MapHash;
	machine.DifficultyName = session.DifficultyName;
	machine.LoadScreen = session.LoadScreen.Image;
	machine.LoadScreenX = session.LoadScreen.X;
	machine.LoadScreenY = session.LoadScreen.Y;
	return(true);
}


/// <summary>
/// Checks a request's network against the players it reaches and converts it. humans[i] is true
/// when player i is human, and local is this machine's player.
/// </summary>
bool Take_Network(Launch::Message::Network const & network, std::vector<bool> const & humans, std::uint32_t local, M::NetworkType & taken, FaultOut & fault)
{
	taken = M::NetworkType();
	taken.Port = network.Port;

	if (network.Tunnel) {
		if (!Is_Address(network.Tunnel->Address)) {
			return(Fault_At(fault, -1, "network.tunnel.address", "The tunnel address %s is not a usable IPv4 address.", network.Tunnel->Address.c_str()));
		}
		if (network.Tunnel->Port < 1 || network.Tunnel->Port > 65535) {
			return(Fault_At(fault, -1, "network.tunnel.port", "The tunnel port %u is outside 1 to 65535.", network.Tunnel->Port));
		}
		if (!Is_Tunnel_Number(network.Port)) {
			return(Fault_At(fault, -1, "network.port", "With a tunnel, the network port %d must be nonzero and within -65535 to 65535.", network.Port));
		}
		taken.Tunnel = M::TunnelType{ network.Tunnel->Address, int(network.Tunnel->Port) };
	} else if (!Is_Port(network.Port)) {
		return(Fault_At(fault, -1, "network.port", "The network port %d is outside 1 to 65535.", network.Port));
	}

	std::vector<bool> reached(humans.size(), false);
	for (Launch::Message::Peer const & peer : network.Peers) {
		if (peer.Player >= humans.size() || !humans[peer.Player] || peer.Player == local) {
			return(Fault_At(fault, -1, "network.peers", "A peer is given for player %u, which is not another human player.", peer.Player));
		}
		if (reached[peer.Player]) {
			return(Fault_At(fault, -1, "network.peers", "Two peers are given for player %u.", peer.Player));
		}
		reached[peer.Player] = true;

		if (network.Tunnel) {
			if (!Is_Tunnel_Number(peer.Port)) {
				return(Fault_At(fault, int(peer.Player), "peer.port", "With a tunnel, the port of player %u must be nonzero and within -65535 to 65535.", peer.Player));
			}
		} else {
			if (!Is_Port(peer.Port)) {
				return(Fault_At(fault, int(peer.Player), "peer.port", "The port %d of player %u is outside 1 to 65535.", peer.Port, peer.Player));
			}
			if (!Is_Address(peer.Address)) {
				return(Fault_At(fault, int(peer.Player), "peer.address", "The address %s of player %u is not a usable IPv4 address.", peer.Address.c_str(), peer.Player));
			}
		}
		taken.Peers.push_back(M::PeerType{ int(peer.Player), peer.Address, peer.Port });
	}
	for (std::size_t index = 0; index < humans.size(); index++) {
		if (humans[index] && index != local && !reached[index]) {
			return(Fault_At(fault, int(index), "peer.port", "No peer is given for player %d.", int(index)));
		}
	}

	taken.ConnectTimeout = M::Timeout_Ticks(network.ConnectTimeout, M::CONNECT_TIMEOUT_DEFAULT);
	taken.ReconnectTimeout = M::Timeout_Ticks(network.ReconnectTimeout, M::RECONNECT_TIMEOUT_DEFAULT);
	return(true);
}

}


/// <summary>
/// Stores this machine's part of a play request when it keeps the rules of launch.proto;
/// otherwise sets fault to the first fault and leaves this unchanged.
/// </summary>
/// <param name="game">The game Game::From_Message stored from the same request.</param>
/// <param name="fault">Where to leave the sentence describing the first fault found.</param>
bool Launch::Machine::From_Request(Launch::Message::Play const & request, Launch::Game const & game, std::string & fault, Launch::FaultSite * site)
{
	FaultOut out{ fault, site };
	if (site != nullptr) {
		*site = Launch::FaultSite();
	}

	if (request.Fixture) {
		return(Fault(out, "The play request sets fixture, which this version does not run."));
	}
	if (request.MapPath.empty()) {
		return(Fault_At(out, -1, "map_path", "The play request gives no map_path."));
	}

	Launch::Message::Session const & session = request.Session;
	if (!session.LocalPlayer) {
		return(Fault_At(out, -1, "session.local_player", "The play request's session gives no local_player."));
	}
	std::uint32_t const local = *session.LocalPlayer;
	if (game.Kind == Launch::GameKind::GAME_KIND_CAMPAIGN) {
		if (local != 0) {
			return(Fault_At(out, -1, "session.local_player", "local_player is %u; a campaign's is 0.", local));
		}
	} else if (local >= game.Players.size() || !game.Players[local].Human) {
		return(Fault_At(out, -1, "session.local_player", "local_player %u is not a human player.", local));
	}

	bool const multiplayer = game.Kind == Launch::GameKind::GAME_KIND_MULTIPLAYER;
	if (multiplayer != request.Network.has_value()) {
		return(Fault(out, multiplayer ? "A multiplayer play request needs network." : "Only a multiplayer play request sets network."));
	}

	Launch::Machine machine;
	if (!Take_Session(session, machine, out)) {
		return(false);
	}

	if (multiplayer) {
		std::vector<bool> humans;
		for (Launch::Game::PlayerType const & player : game.Players) {
			humans.push_back(player.Human);
		}
		NetworkType taken;
		if (!Take_Network(*request.Network, humans, local, taken, out)) {
			return(false);
		}
		machine.Network = std::move(taken);
	}

	machine.LocalPlayer = int(local);
	machine.MapPath = request.MapPath;

	*this = std::move(machine);
	return(true);
}


/// <summary>
/// Stores this machine's part of a load request when it keeps the rules of launch.proto;
/// otherwise sets fault to the first fault and leaves this unchanged. Set network for a save
/// from a game against other machines; only then are the players, the network, and the local
/// player read.
/// </summary>
/// <param name="fault">Where to leave the sentence describing the first fault found.</param>
bool Launch::Machine::From_Load(Launch::Message::Load const & request, bool network, std::string & fault, Launch::FaultSite * site)
{
	FaultOut out{ fault, site };
	if (site != nullptr) {
		*site = Launch::FaultSite();
	}

	if (request.Save.empty()) {
		return(Fault_At(out, -1, "save", "No saved game is given."));
	}
	if (request.Save.find_first_of("/\\:") != std::string::npos) {
		return(Fault_At(out, -1, "save", "The saved game %s has a folder part; give the file name alone.", request.Save.c_str()));
	}
	if (request.GameSpeed && (*request.GameSpeed < 0 || *request.GameSpeed >= Launch::Game::GAME_SPEED_COUNT)) {
		return(Fault_At(out, -1, "game_speed", "The load request sets game_speed %d, outside 0 to %d.", *request.GameSpeed, Launch::Game::GAME_SPEED_COUNT - 1));
	}

	Launch::Machine machine;
	if (!Take_Session(request.Session, machine, out)) {
		return(false);
	}
	machine.MapPath = request.Save;

	if (network) {
		if (!request.Network || request.Players.empty()) {
			return(Fault(out, "Resuming a saved game against other machines needs network and players."));
		}
		if (request.Players.size() > Launch::Game::PLAYER_MAX) {
			return(Fault(out, "The load request lists %d players; the most a game takes is %d.", int(request.Players.size()), Launch::Game::PLAYER_MAX));
		}
		// A player is matched to a saved house by name, ignoring case.
		for (std::size_t index = 0; index < request.Players.size(); index++) {
			Launch::Message::Player const & player = request.Players[index];
			if (player.Name.empty() || player.Name.size() > G::NAME_LENGTH_MAX) {
				return(Fault_At(out, int(index), "name", "Player %d needs a name of 1 to %d bytes.", int(index), G::NAME_LENGTH_MAX));
			}
			for (std::size_t other = 0; other < index; other++) {
				if (Same_Name(request.Players[other].Name, player.Name)) {
					return(Fault_At(out, int(index), "name", "Players %d and %d have the same name.", int(other), int(index)));
				}
			}
		}

		Launch::Message::Session const & session = request.Session;
		if (!session.LocalPlayer || *session.LocalPlayer >= request.Players.size()) {
			return(Fault_At(out, -1, "session.local_player", "local_player must be one of the load request's players."));
		}

		std::vector<bool> const humans(request.Players.size(), true);
		NetworkType taken;
		if (!Take_Network(*request.Network, humans, *session.LocalPlayer, taken, out)) {
			return(false);
		}
		machine.Network = std::move(taken);
		machine.LocalPlayer = int(*session.LocalPlayer);
	}

	*this = std::move(machine);
	return(true);
}
