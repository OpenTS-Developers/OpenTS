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
#include "utf8.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// The encoder and decoder helpers that the files defining message structs share. Only those
// files include this header. The helpers find each struct's Encode and Read_Message by
// argument-dependent lookup, so both must be declared in the struct's own namespace.

namespace ProtoMessage
{

using ProtoWire::Bytes;
using ProtoWire::ByteSpan;
using ProtoWire::WireType;
using ProtoWire::WireWriter;
using ProtoWire::WireError;
using ProtoWire::WireField;
using ProtoWire::WireReader;
using ProtoWire::Read_Repeated_Uint32;

struct DecodeContext {
	LimitsType const & Limits;
	RuleBreak Broken;
	bool BadText = false;   // a string was not valid UTF-8
};


// Returns one of the limits a context applies; without a context nothing is limited.
inline std::size_t Limit(DecodeContext const * context, std::size_t LimitsType::* limit)
{
	return((context != nullptr) ? context->Limits.*limit : SIZE_MAX);
}


// Writes the known fields a caller gives in ascending field number, and puts each kept unknown
// field before the first known field numbered above it.
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

		void Uint(std::uint32_t field, std::uint64_t value) { At(field).Uint(field, value); }
		void Int32(std::uint32_t field, std::int32_t value) { At(field).Int32(field, value); }
		void Sint32(std::uint32_t field, std::int32_t value) { At(field).Sint32(field, value); }
		void Sint64(std::uint32_t field, std::int64_t value) { At(field).Sint64(field, value); }
		void Bool(std::uint32_t field, bool value) { At(field).Bool(field, value); }
		void Fixed32(std::uint32_t field, std::uint32_t value) { At(field).Fixed32(field, value); }
		void Blob(std::uint32_t field, ByteSpan value) { At(field).Blob(field, value); }
		void String(std::uint32_t field, std::string_view value) { At(field).String(field, value); }
		void Present_Sint64(std::uint32_t field, std::int64_t value) { At(field).Present_Sint64(field, value); }
		void Present_Bool(std::uint32_t field, bool value) { At(field).Present_Bool(field, value); }
		void Present_Blob(std::uint32_t field, ByteSpan value) { At(field).Present_Blob(field, value); }
		void Present_String(std::uint32_t field, std::string_view value) { At(field).Present_String(field, value); }
		void Packed_Uint32(std::uint32_t field, std::span<std::uint32_t const> value) { At(field).Packed_Uint32(field, value); }
		void Present_Message(std::uint32_t field, ByteSpan value) { At(field).Present_Message(field, value); }

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
		WireWriter & At(std::uint32_t field)
		{
			while (Next < Order.size() && Unknown[Order[Next]].Number < field) {
				Writer.Raw(Unknown[Order[Next++]].Encoded);
			}
			return(Writer);
		}

		WireWriter Writer;
		UnknownFields const & Unknown;
		std::vector<std::size_t> Order;
		std::size_t Next;
};


inline std::optional<std::uint64_t> Widen(std::optional<std::uint32_t> value)
{
	return(value ? std::optional<std::uint64_t>(*value) : std::nullopt);
}


inline bool Fits(std::string const & text, LimitsType const & limits)
{
	return(text.size() <= limits.String);
}


inline bool Ascending(std::vector<std::uint32_t> const & values)
{
	for (std::size_t i = 1; i < values.size(); i++) {
		if (values[i] <= values[i - 1]) {
			return(false);
		}
	}
	return(true);
}


// An ally list is in order when it ascends and leaves out the house it belongs to.
inline bool Allies_In_Order(std::vector<std::uint32_t> const & allies, std::uint32_t house)
{
	return(Ascending(allies) && std::find(allies.begin(), allies.end(), house) == allies.end());
}


// Reads the fields of one message, which the schema names message. A getter keeps a field of
// another wire type as unknown, and a malformed nested body makes the whole message malformed.
class Fields
{
	public:
		Fields(char const * message, ByteSpan data, UnknownFields & unknown, DecodeContext * context) :
			Reader(data),
			Name(message),
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
				return(Break(RuleError::TOO_LONG));
			}
			std::string_view const text(reinterpret_cast<char const *>(Field.Body.data()), Field.Body.size());
			if (!UTF8::Is_Valid(text)) {
				if (Context != nullptr) {
					Context->BadText = true;
				}
				Bad = true;
				return(false);
			}
			value.assign(text);
			return(true);
		}

		bool Limited_Blob(Bytes & value, std::size_t limit)
		{
			if (Field.Type == WireType::LENGTH && Context != nullptr && Field.Body.size() > limit) {
				return(Break(RuleError::TOO_LONG));
			}
			return(Blob(value));
		}

		template<typename T, typename Getter, typename... Args>
		void Optional(std::optional<T> & value, Getter get, Args... args)
		{
			T taken{};
			if ((this->*get)(taken, args...)) {
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
		void Repeated(std::vector<T> & values, std::size_t limit)
		{
			if (Field.Type == WireType::LENGTH && Context != nullptr && values.size() >= limit) {
				Break(RuleError::TOO_MANY);
				return;
			}
			Repeated(values);
		}

	private:
		bool Break(RuleError error)
		{
			if (Context->Broken == RuleError::NONE) {
				Context->Broken = RuleBreak{ error, Name, Field.Number };
			}
			Bad = true;
			return(false);
		}

		WireReader Reader;
		WireField Field;
		char const * Name;
		UnknownFields & Unknown;
		DecodeContext * Context;
		bool Bad;
};

}
