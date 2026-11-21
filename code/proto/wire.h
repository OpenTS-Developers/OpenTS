/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

// The proto3 wire encoding: varints, zigzag, fixed values, field writers that emit the canonical
// form, and a bounded field reader for untrusted input.

namespace ProtoWire
{
	using Bytes = std::vector<std::uint8_t>;
	using ByteSpan = std::span<std::uint8_t const>;

	enum class WireType : std::uint8_t {
		VARINT = 0,
		FIXED64 = 1,
		LENGTH = 2,
		GROUP_START = 3,
		GROUP_END = 4,
		FIXED32 = 5,
	};

	constexpr std::uint32_t MAX_FIELD_NUMBER = (1u << 29) - 1;
	constexpr std::size_t MAX_VARINT_SIZE = 10;

	std::uint32_t Zigzag32(std::int32_t value);
	std::uint64_t Zigzag64(std::int64_t value);
	std::int32_t Unzigzag32(std::uint32_t value);
	std::int64_t Unzigzag64(std::uint64_t value);

	std::size_t Varint_Size(std::uint64_t value);
	void Put_Varint(Bytes & out, std::uint64_t value);
	void Put_Fixed32(Bytes & out, std::uint32_t value);
	void Put_Fixed64(Bytes & out, std::uint64_t value);

	// Appends fields in canonical form. A plain scalar writer leaves out a zero, false, or empty
	// value; a Present writer always writes it, for fields with presence and oneof members.
	class WireWriter
	{
		public:
			explicit WireWriter(Bytes & out);

			void Tag(std::uint32_t field, WireType type);

			// Enum fields use the Int32 writers.
			void Uint(std::uint32_t field, std::uint64_t value);
			void Int32(std::uint32_t field, std::int32_t value);
			void Int64(std::uint32_t field, std::int64_t value);
			void Sint32(std::uint32_t field, std::int32_t value);
			void Sint64(std::uint32_t field, std::int64_t value);
			void Bool(std::uint32_t field, bool value);
			void Fixed32(std::uint32_t field, std::uint32_t value);
			void Fixed64(std::uint32_t field, std::uint64_t value);
			void Sfixed32(std::uint32_t field, std::int32_t value);
			void Sfixed64(std::uint32_t field, std::int64_t value);
			void Blob(std::uint32_t field, ByteSpan value);
			void String(std::uint32_t field, std::string_view value);

			// Do not use float or double fields in data whose bytes must match across machines.
			void Float(std::uint32_t field, float value);
			void Double(std::uint32_t field, double value);

			void Present_Uint(std::uint32_t field, std::uint64_t value);
			void Present_Int32(std::uint32_t field, std::int32_t value);
			void Present_Int64(std::uint32_t field, std::int64_t value);
			void Present_Sint32(std::uint32_t field, std::int32_t value);
			void Present_Sint64(std::uint32_t field, std::int64_t value);
			void Present_Bool(std::uint32_t field, bool value);
			void Present_Fixed32(std::uint32_t field, std::uint32_t value);
			void Present_Fixed64(std::uint32_t field, std::uint64_t value);
			void Present_Sfixed32(std::uint32_t field, std::int32_t value);
			void Present_Sfixed64(std::uint32_t field, std::int64_t value);
			void Present_Float(std::uint32_t field, float value);
			void Present_Double(std::uint32_t field, double value);
			void Present_Blob(std::uint32_t field, ByteSpan value);
			void Present_String(std::uint32_t field, std::string_view value);

			// A packed repeated field; an empty list writes nothing.
			void Packed_Uint32(std::uint32_t field, std::span<std::uint32_t const> values);
			void Packed_Uint64(std::uint32_t field, std::span<std::uint64_t const> values);
			void Packed_Int32(std::uint32_t field, std::span<std::int32_t const> values);
			void Packed_Int64(std::uint32_t field, std::span<std::int64_t const> values);
			void Packed_Sint32(std::uint32_t field, std::span<std::int32_t const> values);
			void Packed_Sint64(std::uint32_t field, std::span<std::int64_t const> values);
			void Packed_Bool(std::uint32_t field, std::span<bool const> values);
			void Packed_Fixed32(std::uint32_t field, std::span<std::uint32_t const> values);
			void Packed_Fixed64(std::uint32_t field, std::span<std::uint64_t const> values);
			void Packed_Sfixed32(std::uint32_t field, std::span<std::int32_t const> values);
			void Packed_Sfixed64(std::uint32_t field, std::span<std::int64_t const> values);
			void Packed_Float(std::uint32_t field, std::span<float const> values);
			void Packed_Double(std::uint32_t field, std::span<double const> values);

			// A message field from its encoded body, left out when empty unlike Present_Message.
			void Message(std::uint32_t field, ByteSpan body);
			void Present_Message(std::uint32_t field, ByteSpan body);

			void Raw(ByteSpan bytes);

		private:
			template<typename T, typename Encode>
			void Packed_Varint(std::uint32_t field, std::span<T const> values, Encode encode);

			template<typename T>
			void Packed_Fixed(std::uint32_t field, std::span<T const> values);

			Bytes & Out;
	};

	enum class WireError : std::uint8_t {
		NONE,
		TRUNCATED,        // a varint, fixed value, or length runs past the input
		VARINT_TOO_LONG,  // more than MAX_VARINT_SIZE bytes, or a value past 64 bits
		BAD_FIELD_NUMBER, // zero, or past MAX_FIELD_NUMBER
		GROUP,            // wire type 3 or 4
		BAD_WIRE_TYPE,    // wire type 6 or 7
	};

	struct WireField {
		// The accessors convert Value without checking Type; the caller checks the wire type first.
		// Fixed and sfixed fields use the Uint and Int accessors of their width.
		std::uint32_t As_Uint32(void) const { return(std::uint32_t(Value)); }
		std::uint64_t As_Uint64(void) const { return(Value); }
		std::int32_t As_Int32(void) const { return(std::int32_t(std::uint32_t(Value))); }
		std::int64_t As_Int64(void) const { return(std::int64_t(Value)); }
		std::int32_t As_Sint32(void) const { return(Unzigzag32(std::uint32_t(Value))); }
		std::int64_t As_Sint64(void) const { return(Unzigzag64(Value)); }
		bool As_Bool(void) const { return(Value != 0); }
		float As_Float(void) const { return(std::bit_cast<float>(std::uint32_t(Value))); }
		double As_Double(void) const { return(std::bit_cast<double>(Value)); }

		std::uint32_t Number = 0;
		WireType Type = WireType::VARINT;
		std::uint64_t Value = 0;     // varint and fixed values
		ByteSpan Body;               // length-delimited values
		ByteSpan Encoded;            // the whole field as read, tag included
	};

	// Reads the fields of one message from untrusted bytes. Next returns false at the end of the
	// input and on the first malformed field, which Error then names.
	class WireReader
	{
		public:
			explicit WireReader(ByteSpan data);

			bool Next(WireField & field);
			WireError Error(void) const { return(Failure); }
			std::size_t Offset(void) const { return(Position); }

		private:
			ByteSpan Data;
			std::size_t Position;
			WireError Failure;
	};

	// Reads one varint at offset, advancing it. Returns the error, or NONE on success.
	WireError Read_Varint(ByteSpan data, std::size_t & offset, std::uint64_t & value);

	// Appends a repeated uint32 field's values, packed or not; false on a malformed body.
	bool Read_Repeated_Uint32(WireField const & field, std::vector<std::uint32_t> & values);

	// Appends a repeated numeric field's values, packed or not, without zigzag decoding or bit casts;
	// false on a malformed body or another wire type.
	bool Read_Repeated_Varint(WireField const & field, std::vector<std::uint64_t> & values);
	bool Read_Repeated_Fixed32(WireField const & field, std::vector<std::uint32_t> & values);
	bool Read_Repeated_Fixed64(WireField const & field, std::vector<std::uint64_t> & values);
}
