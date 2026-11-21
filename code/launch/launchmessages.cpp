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
using ProtoWire::WireType;
using ProtoWire::WireError;
using ProtoWire::WireField;
using ProtoWire::WireReader;

namespace L = Field;

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
	Fields fields("GameOption", data, message.Unknown, context);
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
	Fields fields("Launch", data, message.Unknown, context);
	while (fields.Next()) {
		switch (fields.Number()) {
			case L::Game::KIND: fields.Enum(message.Kind); break;
			case L::Game::SEED: fields.Optional(message.Seed, &Fields::Fixed32); break;
			case L::Game::PLAYERS: fields.Repeated(message.Players, Limit(context, &LimitsType::Players)); break;
			case L::Game::OPTIONS: fields.Repeated(message.Options); break;
			case L::Game::ANONYMOUS: fields.Bool(message.Anonymous); break;
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
		return(RuleBreak{ RuleError::TOO_MANY, "Launch", L::Game::PLAYERS });
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
			return(RuleBreak{ RuleError::TOO_LONG, "GameOption", L::Option::KEY });
		}
		auto text = std::get_if<std::string>(&option.Value);
		if (text != nullptr && !Fits(*text, limits)) {
			return(RuleBreak{ RuleError::TOO_LONG, "GameOption", L::Option::TEXT });
		}
	}
	return(RuleBreak());
}


RuleBreak Check_Order(Game const & message)
{
	for (std::size_t i = 0; i < message.Players.size(); i++) {
		if (i > 0 && message.Players[i].House <= message.Players[i - 1].House) {
			return(RuleBreak{ RuleError::NOT_ASCENDING, "Launch", L::Game::PLAYERS });
		}
		if (!Allies_In_Order(message.Players[i].Allies, message.Players[i].House)) {
			return(RuleBreak{ RuleError::NOT_ASCENDING, "Player", L::Player::ALLIES });
		}
	}
	for (std::size_t i = 1; i < message.Options.size(); i++) {
		if (!(message.Options[i - 1].Key < message.Options[i].Key)) {
			return(RuleBreak{ RuleError::NOT_ASCENDING, "Launch", L::Game::OPTIONS });
		}
	}
	return(RuleBreak());
}

}
