/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "protowire.h"

#include <bit>

namespace ProtoWire
{

std::uint32_t Zigzag32(std::int32_t value)
{
	return((std::uint32_t(value) << 1) ^ std::uint32_t(value >> 31));
}


std::uint64_t Zigzag64(std::int64_t value)
{
	return((std::uint64_t(value) << 1) ^ std::uint64_t(value >> 63));
}


std::int32_t Unzigzag32(std::uint32_t value)
{
	return(std::int32_t((value >> 1) ^ (0u - (value & 1))));
}


std::int64_t Unzigzag64(std::uint64_t value)
{
	return(std::int64_t((value >> 1) ^ (0ull - (value & 1))));
}


std::size_t Varint_Size(std::uint64_t value)
{
	std::size_t size = 1;
	while (value >= 0x80) {
		value >>= 7;
		size++;
	}
	return(size);
}


void Put_Varint(Bytes & out, std::uint64_t value)
{
	while (value >= 0x80) {
		out.push_back(std::uint8_t(value | 0x80));
		value >>= 7;
	}
	out.push_back(std::uint8_t(value));
}


void Put_Fixed32(Bytes & out, std::uint32_t value)
{
	for (int i = 0; i < 4; i++) {
		out.push_back(std::uint8_t(value >> (i * 8)));
	}
}


void Put_Fixed64(Bytes & out, std::uint64_t value)
{
	for (int i = 0; i < 8; i++) {
		out.push_back(std::uint8_t(value >> (i * 8)));
	}
}


WireWriter::WireWriter(Bytes & out) :
	Out(out)
{
}


void WireWriter::Tag(std::uint32_t field, WireType type)
{
	Put_Varint(Out, (std::uint64_t(field) << 3) | std::uint64_t(type));
}


void WireWriter::Uint(std::uint32_t field, std::uint64_t value)
{
	if (value != 0) {
		Present_Uint(field, value);
	}
}


void WireWriter::Int32(std::uint32_t field, std::int32_t value)
{
	if (value != 0) {
		Present_Int32(field, value);
	}
}


void WireWriter::Int64(std::uint32_t field, std::int64_t value)
{
	if (value != 0) {
		Present_Int64(field, value);
	}
}


void WireWriter::Sint32(std::uint32_t field, std::int32_t value)
{
	if (value != 0) {
		Present_Sint32(field, value);
	}
}


void WireWriter::Sint64(std::uint32_t field, std::int64_t value)
{
	if (value != 0) {
		Present_Sint64(field, value);
	}
}


void WireWriter::Bool(std::uint32_t field, bool value)
{
	if (value) {
		Present_Bool(field, value);
	}
}


void WireWriter::Fixed32(std::uint32_t field, std::uint32_t value)
{
	if (value != 0) {
		Present_Fixed32(field, value);
	}
}


void WireWriter::Fixed64(std::uint32_t field, std::uint64_t value)
{
	if (value != 0) {
		Present_Fixed64(field, value);
	}
}


void WireWriter::Sfixed32(std::uint32_t field, std::int32_t value)
{
	Fixed32(field, std::uint32_t(value));
}


void WireWriter::Sfixed64(std::uint32_t field, std::int64_t value)
{
	Fixed64(field, std::uint64_t(value));
}


// Only positive zero is left out; -0.0 and NaN are written, as protobuf writes them.
void WireWriter::Float(std::uint32_t field, float value)
{
	Fixed32(field, std::bit_cast<std::uint32_t>(value));
}


void WireWriter::Double(std::uint32_t field, double value)
{
	Fixed64(field, std::bit_cast<std::uint64_t>(value));
}


void WireWriter::Blob(std::uint32_t field, ByteSpan value)
{
	if (!value.empty()) {
		Present_Blob(field, value);
	}
}


void WireWriter::String(std::uint32_t field, std::string_view value)
{
	if (!value.empty()) {
		Present_String(field, value);
	}
}


void WireWriter::Present_Uint(std::uint32_t field, std::uint64_t value)
{
	Tag(field, WireType::VARINT);
	Put_Varint(Out, value);
}


// A negative value is encoded in ten bytes, as proto3 encodes int32.
void WireWriter::Present_Int32(std::uint32_t field, std::int32_t value)
{
	Present_Int64(field, value);
}


void WireWriter::Present_Int64(std::uint32_t field, std::int64_t value)
{
	Tag(field, WireType::VARINT);
	Put_Varint(Out, std::uint64_t(value));
}


void WireWriter::Present_Sint32(std::uint32_t field, std::int32_t value)
{
	Tag(field, WireType::VARINT);
	Put_Varint(Out, Zigzag32(value));
}


void WireWriter::Present_Sint64(std::uint32_t field, std::int64_t value)
{
	Tag(field, WireType::VARINT);
	Put_Varint(Out, Zigzag64(value));
}


void WireWriter::Present_Bool(std::uint32_t field, bool value)
{
	Tag(field, WireType::VARINT);
	Out.push_back(value ? 1 : 0);
}


void WireWriter::Present_Fixed32(std::uint32_t field, std::uint32_t value)
{
	Tag(field, WireType::FIXED32);
	Put_Fixed32(Out, value);
}


void WireWriter::Present_Fixed64(std::uint32_t field, std::uint64_t value)
{
	Tag(field, WireType::FIXED64);
	Put_Fixed64(Out, value);
}


void WireWriter::Present_Sfixed32(std::uint32_t field, std::int32_t value)
{
	Present_Fixed32(field, std::uint32_t(value));
}


void WireWriter::Present_Sfixed64(std::uint32_t field, std::int64_t value)
{
	Present_Fixed64(field, std::uint64_t(value));
}


void WireWriter::Present_Float(std::uint32_t field, float value)
{
	Present_Fixed32(field, std::bit_cast<std::uint32_t>(value));
}


void WireWriter::Present_Double(std::uint32_t field, double value)
{
	Present_Fixed64(field, std::bit_cast<std::uint64_t>(value));
}


void WireWriter::Present_Blob(std::uint32_t field, ByteSpan value)
{
	Tag(field, WireType::LENGTH);
	Put_Varint(Out, value.size());
	Out.insert(Out.end(), value.begin(), value.end());
}


void WireWriter::Present_String(std::uint32_t field, std::string_view value)
{
	Present_Blob(field, ByteSpan(reinterpret_cast<std::uint8_t const *>(value.data()), value.size()));
}


template<typename T, typename Encode>
void WireWriter::Packed_Varint(std::uint32_t field, std::span<T const> values, Encode encode)
{
	if (values.empty()) {
		return;
	}

	std::size_t size = 0;
	for (T value : values) {
		size += Varint_Size(encode(value));
	}
	Tag(field, WireType::LENGTH);
	Put_Varint(Out, size);
	for (T value : values) {
		Put_Varint(Out, encode(value));
	}
}


template<typename T>
void WireWriter::Packed_Fixed(std::uint32_t field, std::span<T const> values)
{
	static_assert(sizeof(T) == 4 || sizeof(T) == 8);

	if (values.empty()) {
		return;
	}

	Tag(field, WireType::LENGTH);
	Put_Varint(Out, values.size() * sizeof(T));
	for (T value : values) {
		if constexpr (sizeof(T) == 4) {
			Put_Fixed32(Out, std::bit_cast<std::uint32_t>(value));
		} else {
			Put_Fixed64(Out, std::bit_cast<std::uint64_t>(value));
		}
	}
}


void WireWriter::Packed_Uint32(std::uint32_t field, std::span<std::uint32_t const> values)
{
	Packed_Varint(field, values, [](std::uint32_t value) { return(std::uint64_t(value)); });
}


void WireWriter::Packed_Uint64(std::uint32_t field, std::span<std::uint64_t const> values)
{
	Packed_Varint(field, values, [](std::uint64_t value) { return(value); });
}


void WireWriter::Packed_Int32(std::uint32_t field, std::span<std::int32_t const> values)
{
	Packed_Varint(field, values, [](std::int32_t value) { return(std::uint64_t(std::int64_t(value))); });
}


void WireWriter::Packed_Int64(std::uint32_t field, std::span<std::int64_t const> values)
{
	Packed_Varint(field, values, [](std::int64_t value) { return(std::uint64_t(value)); });
}


void WireWriter::Packed_Sint32(std::uint32_t field, std::span<std::int32_t const> values)
{
	Packed_Varint(field, values, [](std::int32_t value) { return(std::uint64_t(Zigzag32(value))); });
}


void WireWriter::Packed_Sint64(std::uint32_t field, std::span<std::int64_t const> values)
{
	Packed_Varint(field, values, [](std::int64_t value) { return(Zigzag64(value)); });
}


void WireWriter::Packed_Bool(std::uint32_t field, std::span<bool const> values)
{
	Packed_Varint(field, values, [](bool value) { return(std::uint64_t(value ? 1 : 0)); });
}


void WireWriter::Packed_Fixed32(std::uint32_t field, std::span<std::uint32_t const> values)
{
	Packed_Fixed(field, values);
}


void WireWriter::Packed_Fixed64(std::uint32_t field, std::span<std::uint64_t const> values)
{
	Packed_Fixed(field, values);
}


void WireWriter::Packed_Sfixed32(std::uint32_t field, std::span<std::int32_t const> values)
{
	Packed_Fixed(field, values);
}


void WireWriter::Packed_Sfixed64(std::uint32_t field, std::span<std::int64_t const> values)
{
	Packed_Fixed(field, values);
}


void WireWriter::Packed_Float(std::uint32_t field, std::span<float const> values)
{
	Packed_Fixed(field, values);
}


void WireWriter::Packed_Double(std::uint32_t field, std::span<double const> values)
{
	Packed_Fixed(field, values);
}


void WireWriter::Message(std::uint32_t field, ByteSpan body)
{
	if (!body.empty()) {
		Present_Blob(field, body);
	}
}


void WireWriter::Present_Message(std::uint32_t field, ByteSpan body)
{
	Present_Blob(field, body);
}


void WireWriter::Raw(ByteSpan bytes)
{
	Out.insert(Out.end(), bytes.begin(), bytes.end());
}


WireError Read_Varint(ByteSpan data, std::size_t & offset, std::uint64_t & value)
{
	std::uint64_t result = 0;
	for (std::size_t i = 0; i < MAX_VARINT_SIZE; i++) {
		if (offset + i >= data.size()) {
			return(WireError::TRUNCATED);
		}
		std::uint8_t byte = data[offset + i];

		// The tenth byte encodes only bit 63.
		if (i == MAX_VARINT_SIZE - 1 && byte > 1) {
			return(WireError::VARINT_TOO_LONG);
		}
		result |= std::uint64_t(byte & 0x7F) << (7 * i);
		if ((byte & 0x80) == 0) {
			offset += i + 1;
			value = result;
			return(WireError::NONE);
		}
	}
	return(WireError::VARINT_TOO_LONG);
}


WireReader::WireReader(ByteSpan data) :
	Data(data),
	Position(0),
	Failure(WireError::NONE)
{
}


bool WireReader::Next(WireField & field)
{
	if (Failure != WireError::NONE || Position >= Data.size()) {
		return(false);
	}

	std::size_t const start = Position;
	std::size_t offset = Position;
	std::uint64_t tag = 0;
	WireError error = Read_Varint(Data, offset, tag);
	if (error != WireError::NONE) {
		Failure = error;
		return(false);
	}

	std::uint64_t number = tag >> 3;
	if (number == 0 || number > MAX_FIELD_NUMBER) {
		Failure = WireError::BAD_FIELD_NUMBER;
		return(false);
	}

	field.Number = std::uint32_t(number);
	field.Type = WireType(tag & 7);
	field.Value = 0;
	field.Body = ByteSpan();

	switch (field.Type) {
		case WireType::VARINT:
			error = Read_Varint(Data, offset, field.Value);
			break;

		case WireType::FIXED64:
		case WireType::FIXED32:
		{
			std::size_t size = (field.Type == WireType::FIXED64) ? 8 : 4;
			if (Data.size() - offset < size) {
				error = WireError::TRUNCATED;
				break;
			}
			for (std::size_t i = 0; i < size; i++) {
				field.Value |= std::uint64_t(Data[offset + i]) << (8 * i);
			}
			offset += size;
			break;
		}

		case WireType::LENGTH:
		{
			std::uint64_t length = 0;
			error = Read_Varint(Data, offset, length);
			if (error == WireError::NONE) {
				if (length > Data.size() - offset) {
					error = WireError::TRUNCATED;
					break;
				}
				field.Body = Data.subspan(offset, std::size_t(length));
				offset += std::size_t(length);
			}
			break;
		}

		case WireType::GROUP_START:
		case WireType::GROUP_END:
			error = WireError::GROUP;
			break;

		default:
			error = WireError::BAD_WIRE_TYPE;
			break;
	}

	if (error != WireError::NONE) {
		Failure = error;
		return(false);
	}

	field.Encoded = Data.subspan(start, offset - start);
	Position = offset;
	return(true);
}


/// <summary>
/// Appends the values of a repeated uint32 field, packed or not, to values. A value wider than
/// 32 bits keeps its low bits, as protobuf parsers do. Returns false on a malformed body.
/// </summary>
bool Read_Repeated_Uint32(WireField const & field, std::vector<std::uint32_t> & values)
{
	if (field.Type == WireType::VARINT) {
		values.push_back(std::uint32_t(field.Value));
		return(true);
	}
	if (field.Type != WireType::LENGTH) {
		return(false);
	}

	std::size_t offset = 0;
	while (offset < field.Body.size()) {
		std::uint64_t value = 0;
		if (Read_Varint(field.Body, offset, value) != WireError::NONE) {
			return(false);
		}
		values.push_back(std::uint32_t(value));
	}
	return(true);
}


bool Read_Repeated_Varint(WireField const & field, std::vector<std::uint64_t> & values)
{
	if (field.Type == WireType::VARINT) {
		values.push_back(field.Value);
		return(true);
	}
	if (field.Type != WireType::LENGTH) {
		return(false);
	}

	std::size_t offset = 0;
	while (offset < field.Body.size()) {
		std::uint64_t value = 0;
		if (Read_Varint(field.Body, offset, value) != WireError::NONE) {
			return(false);
		}
		values.push_back(value);
	}
	return(true);
}


template<typename T>
static bool Read_Repeated_Fixed(WireField const & field, WireType type, std::vector<T> & values)
{
	if (field.Type == type) {
		values.push_back(T(field.Value));
		return(true);
	}
	if (field.Type != WireType::LENGTH || field.Body.size() % sizeof(T) != 0) {
		return(false);
	}

	for (std::size_t offset = 0; offset < field.Body.size(); offset += sizeof(T)) {
		T value = 0;
		for (std::size_t i = 0; i < sizeof(T); i++) {
			value |= T(field.Body[offset + i]) << (8 * i);
		}
		values.push_back(value);
	}
	return(true);
}


bool Read_Repeated_Fixed32(WireField const & field, std::vector<std::uint32_t> & values)
{
	return(Read_Repeated_Fixed(field, WireType::FIXED32, values));
}


bool Read_Repeated_Fixed64(WireField const & field, std::vector<std::uint64_t> & values)
{
	return(Read_Repeated_Fixed(field, WireType::FIXED64, values));
}

}
