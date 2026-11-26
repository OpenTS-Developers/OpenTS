/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "doctest/doctest.h"
#include "proto/wire.h"
#include "replaytest.h"

#include <bit>
#include <climits>
#include <cmath>
#include <limits>

using namespace ProtoWire;


static std::string Varint_Hex(std::uint64_t value)
{
	Bytes out;
	Put_Varint(out, value);
	CHECK(out.size() == Varint_Size(value));
	return(Hex(out));
}


static std::uint64_t Decode_Varint(ByteVector const & bytes, WireError expected = WireError::NONE)
{
	std::size_t offset = 0;
	std::uint64_t value = 0;
	CHECK(Read_Varint(bytes, offset, value) == expected);
	if (expected == WireError::NONE) {
		CHECK_MESSAGE(offset == bytes.size(), "the varint uses every byte");
	}
	return(value);
}


TEST_CASE("varints take the fewest bytes and read back")
{
	CHECK(Varint_Hex(0) == "00");
	CHECK(Varint_Hex(1) == "01");
	CHECK(Varint_Hex(127) == "7f");
	CHECK(Varint_Hex(128) == "8001");
	CHECK(Varint_Hex(300) == "ac02");
	CHECK(Varint_Hex(16383) == "ff7f");
	CHECK(Varint_Hex(16384) == "808001");
	CHECK(Varint_Hex(0xFFFFFFFFull) == "ffffffff0f");
	CHECK(Varint_Hex(1ull << 63) == "80808080808080808001");
	CHECK(Varint_Hex(~0ull) == "ffffffffffffffffff01");

	for (std::uint64_t value : { 0ull, 1ull, 127ull, 128ull, 0xFFFFFFFFull, 1ull << 63, ~0ull }) {
		Bytes out;
		Put_Varint(out, value);
		CHECK(Decode_Varint(out) == value);
	}

	CHECK_MESSAGE(Decode_Varint(From_Hex("8000")) == 0, "a reader accepts a longer form than needed");
}


TEST_CASE("a varint past ten bytes or 64 bits, or cut short, is refused")
{
	Decode_Varint(From_Hex("ffffffffffffffffff02"), WireError::VARINT_TOO_LONG);
	Decode_Varint(From_Hex("8080808080808080808000"), WireError::VARINT_TOO_LONG);
	Decode_Varint(From_Hex("80"), WireError::TRUNCATED);
	Decode_Varint(From_Hex(""), WireError::TRUNCATED);
	Decode_Varint(From_Hex("ffffffffffffffffff"), WireError::TRUNCATED);
}


TEST_CASE("zigzag maps signed values to small unsigned ones")
{
	CHECK(Zigzag32(0) == 0u);
	CHECK(Zigzag32(-1) == 1u);
	CHECK(Zigzag32(1) == 2u);
	CHECK(Zigzag32(-2) == 3u);
	CHECK(Zigzag32(INT32_MAX) == 0xFFFFFFFEu);
	CHECK(Zigzag32(INT32_MIN) == 0xFFFFFFFFu);
	CHECK(Zigzag64(INT64_MAX) == 0xFFFFFFFFFFFFFFFEull);
	CHECK(Zigzag64(INT64_MIN) == 0xFFFFFFFFFFFFFFFFull);

	for (std::int32_t value : { 0, 1, -1, 2, -2, INT32_MAX, INT32_MIN }) {
		CHECK(Unzigzag32(Zigzag32(value)) == value);
	}
	for (std::int64_t value : { std::int64_t(0), std::int64_t(-1), INT64_MAX, INT64_MIN }) {
		CHECK(Unzigzag64(Zigzag64(value)) == value);
	}
}


TEST_CASE("field writers leave out zero values unless the field has presence")
{
	Bytes out;
	WireWriter writer(out);
	writer.Uint(1, 0);
	writer.Int32(2, 0);
	writer.Sint32(3, 0);
	writer.Sint64(4, 0);
	writer.Bool(5, false);
	writer.Fixed32(6, 0);
	writer.Blob(7, ByteSpan());
	writer.String(8, "");
	writer.Packed_Uint32(9, {});
	writer.Message(10, ByteSpan());
	writer.Int64(11, 0);
	writer.Fixed64(12, 0);
	writer.Sfixed32(13, 0);
	writer.Sfixed64(14, 0);
	writer.Float(15, 0.0f);
	writer.Double(16, 0.0);
	CHECK(out.empty());

	// The expected bytes in this file's writer tests are protoc output for the same values.
	writer.Present_Uint(1, 0);
	writer.Present_Int32(2, 0);
	writer.Present_Sint32(3, 0);
	writer.Present_Bool(5, false);
	writer.Present_Fixed32(6, 0);
	writer.Present_String(8, "");
	writer.Present_Message(10, ByteSpan());
	writer.Present_Int64(11, 0);
	writer.Present_Fixed64(12, 0);
	writer.Present_Sfixed32(13, 0);
	writer.Present_Sfixed64(14, 0);
	writer.Present_Float(15, 0.0f);
	writer.Present_Double(16, 0.0);
	CHECK(Hex(out) ==
		"0800" "1000" "1800" "2800" "3500000000" "4200" "5200"
		"5800" "610000000000000000" "6d00000000" "710000000000000000" "7d00000000" "81010000000000000000");
}


TEST_CASE("field writers produce the protobuf bytes")
{
	Bytes out;
	WireWriter writer(out);
	writer.Uint(1, 150);
	writer.Sint32(2, -1);
	writer.Sint64(3, -3);
	writer.Bool(4, true);
	writer.Fixed32(5, 0x01020304);
	writer.String(6, "hi");
	writer.Int32(7, -1);
	std::uint32_t const values[] = { 3, 270, 86942 };
	writer.Packed_Uint32(8, values);
	writer.Uint(2000, 1);
	CHECK(Hex(out) ==
		"089601"
		"1001"
		"1805"
		"2001"
		"2d04030201"
		"32026869"
		"38ffffffffffffffffff01"
		"4206038e029ea705"
		"807d01");
}


TEST_CASE("64-bit and signed fixed writers produce the protobuf bytes")
{
	Bytes out;
	WireWriter writer(out);
	writer.Int64(1, -1);
	writer.Int64(2, INT64_MIN);
	writer.Int64(3, INT64_MAX);
	writer.Int32(4, INT32_MIN);
	writer.Int32(5, INT32_MAX);
	writer.Fixed64(6, UINT64_MAX);
	writer.Fixed64(7, 0x0102030405060708ull);
	writer.Sfixed32(8, INT32_MIN);
	writer.Sfixed32(9, INT32_MAX);
	writer.Sfixed32(10, -2);
	writer.Sfixed64(11, INT64_MIN);
	writer.Sfixed64(12, INT64_MAX);
	writer.Sfixed64(13, -2);
	writer.Int64(14, 150);
	CHECK(Hex(out) ==
		"08ffffffffffffffffff01"
		"1080808080808080808001"
		"18ffffffffffffffff7f"
		"2080808080f8ffffffff01"
		"28ffffffff07"
		"31ffffffffffffffff"
		"390807060504030201"
		"4500000080"
		"4dffffff7f"
		"55feffffff"
		"590000000000000080"
		"61ffffffffffffff7f"
		"69feffffffffffffff"
		"709601");
}


TEST_CASE("float and double writers write the IEEE-754 bits and leave out only positive zero")
{
	Bytes out;
	WireWriter writer(out);
	writer.Float(1, -0.0f);
	writer.Float(2, std::numeric_limits<float>::quiet_NaN());
	writer.Float(3, std::numeric_limits<float>::infinity());
	writer.Float(4, -std::numeric_limits<float>::infinity());
	writer.Float(5, std::numeric_limits<float>::max());
	writer.Float(6, std::numeric_limits<float>::lowest());
	writer.Float(7, std::numeric_limits<float>::min());
	writer.Float(8, std::numeric_limits<float>::denorm_min());
	writer.Float(9, 1.5f);
	writer.Double(11, -0.0);
	writer.Double(12, std::numeric_limits<double>::quiet_NaN());
	writer.Double(13, std::numeric_limits<double>::infinity());
	writer.Double(14, -std::numeric_limits<double>::infinity());
	writer.Double(15, std::numeric_limits<double>::max());
	writer.Double(16, std::numeric_limits<double>::lowest());
	writer.Double(17, std::numeric_limits<double>::min());
	writer.Double(18, std::numeric_limits<double>::denorm_min());
	writer.Double(19, 1.5);
	CHECK(Hex(out) ==
		"0d00000080"
		"150000c07f"
		"1d0000807f"
		"25000080ff"
		"2dffff7f7f"
		"35ffff7fff"
		"3d00008000"
		"4501000000"
		"4d0000c03f"
		"590000000000000080"
		"61000000000000f87f"
		"69000000000000f07f"
		"71000000000000f0ff"
		"79ffffffffffffef7f"
		"8101ffffffffffffefff"
		"89010000000000001000"
		"91010100000000000000"
		"9901000000000000f83f");

	Bytes present;
	WireWriter present_writer(present);
	present_writer.Present_Float(1, -0.0f);
	present_writer.Present_Double(2, 1.5);
	CHECK(Hex(present) == "0d00000080" "11000000000000f83f");
}


TEST_CASE("packed writers write nothing for an empty list")
{
	Bytes out;
	WireWriter writer(out);
	writer.Packed_Uint32(1, {});
	writer.Packed_Uint64(2, {});
	writer.Packed_Int32(3, {});
	writer.Packed_Int64(4, {});
	writer.Packed_Sint32(5, {});
	writer.Packed_Sint64(6, {});
	writer.Packed_Bool(7, {});
	writer.Packed_Fixed32(8, {});
	writer.Packed_Fixed64(9, {});
	writer.Packed_Sfixed32(10, {});
	writer.Packed_Sfixed64(11, {});
	writer.Packed_Float(12, {});
	writer.Packed_Double(13, {});
	CHECK(out.empty());
}


TEST_CASE("packed writers produce the protobuf bytes for one value and many")
{
	std::uint64_t const uint64s[] = { 0, 1, UINT64_MAX };
	std::int32_t const int32s[] = { 0, -1, INT32_MIN, INT32_MAX };
	std::int64_t const int64s[] = { -1, INT64_MIN, INT64_MAX };
	std::int32_t const sint32s[] = { 0, -1, 1, INT32_MIN, INT32_MAX };
	bool const bools[] = { true, false, true };
	std::uint32_t const fixed32s[] = { 0, UINT32_MAX };
	std::uint64_t const fixed64s[] = { 0, UINT64_MAX };
	std::int32_t const sfixed32s[] = { -1, INT32_MIN, INT32_MAX };
	float const floats[] = { 1.5f, -0.0f, std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN() };
	double const doubles[] = { 1.5, -0.0, -std::numeric_limits<double>::infinity() };
	std::int32_t const enums[] = { 0, -1, 1 };
	std::uint64_t const one_uint64[] = { 300 };
	std::uint32_t const one_fixed32[] = { 1 };
	double const one_double[] = { -2.0 };
	std::int32_t const one_int32[] = { -1 };

	Bytes out;
	WireWriter writer(out);
	writer.Packed_Uint64(1, uint64s);
	writer.Packed_Int32(2, int32s);
	writer.Packed_Int64(3, int64s);
	writer.Packed_Sint32(4, sint32s);
	writer.Packed_Sint64(5, int64s);
	writer.Packed_Bool(6, bools);
	writer.Packed_Fixed32(7, fixed32s);
	writer.Packed_Fixed64(8, fixed64s);
	writer.Packed_Sfixed32(9, sfixed32s);
	writer.Packed_Sfixed64(10, int64s);
	writer.Packed_Float(11, floats);
	writer.Packed_Double(12, doubles);
	writer.Packed_Int32(13, enums);
	writer.Packed_Uint64(14, one_uint64);
	writer.Packed_Fixed32(15, one_fixed32);
	writer.Packed_Double(16, one_double);
	writer.Packed_Int32(17, one_int32);
	CHECK(Hex(out) ==
		"0a0c" "00" "01" "ffffffffffffffffff01"
		"121a" "00" "ffffffffffffffffff01" "80808080f8ffffffff01" "ffffffff07"
		"1a1d" "ffffffffffffffffff01" "80808080808080808001" "ffffffffffffffff7f"
		"220d" "00" "01" "02" "ffffffff0f" "feffffff0f"
		"2a15" "01" "ffffffffffffffffff01" "feffffffffffffffff01"
		"3203" "01" "00" "01"
		"3a08" "00000000" "ffffffff"
		"4210" "0000000000000000" "ffffffffffffffff"
		"4a0c" "ffffffff" "00000080" "ffffff7f"
		"5218" "ffffffffffffffff" "0000000000000080" "ffffffffffffff7f"
		"5a10" "0000c03f" "00000080" "0000807f" "0000c07f"
		"6218" "000000000000f83f" "0000000000000080" "000000000000f0ff"
		"6a0c" "00" "ffffffffffffffffff01" "01"
		"7202" "ac02"
		"7a04" "01000000"
		"820108" "00000000000000c0"
		"8a010a" "ffffffffffffffffff01");
}


TEST_CASE("the reader walks every wire type and skips nothing silently")
{
	ByteVector const bytes = From_Hex("089601" "1104030201aabbccdd" "1a03616263" "2504030201");
	WireReader reader(bytes);
	WireField field;

	REQUIRE(reader.Next(field));
	CHECK(field.Number == 1);
	CHECK(field.Type == WireType::VARINT);
	CHECK(field.Value == 150);
	CHECK(Hex(field.Encoded) == "089601");

	REQUIRE(reader.Next(field));
	CHECK(field.Number == 2);
	CHECK(field.Type == WireType::FIXED64);
	CHECK(field.Value == 0xddccbbaa01020304ull);

	REQUIRE(reader.Next(field));
	CHECK(field.Number == 3);
	CHECK(field.Type == WireType::LENGTH);
	CHECK(Hex(field.Body) == "616263");

	REQUIRE(reader.Next(field));
	CHECK(field.Number == 4);
	CHECK(field.Type == WireType::FIXED32);
	CHECK(field.Value == 0x01020304);

	CHECK(!reader.Next(field));
	CHECK(reader.Error() == WireError::NONE);
	CHECK(reader.Offset() == bytes.size());
}


static WireError First_Error(ByteVector const & bytes)
{
	WireReader reader(bytes);
	WireField field;
	while (reader.Next(field)) {
	}
	return(reader.Error());
}


TEST_CASE("malformed fields stop the reader with the reason")
{
	CHECK(First_Error(From_Hex("0001")) == WireError::BAD_FIELD_NUMBER);
	CHECK(First_Error(From_Hex("8080808010")) == WireError::BAD_FIELD_NUMBER);
	CHECK(First_Error(From_Hex("0b")) == WireError::GROUP);
	CHECK(First_Error(From_Hex("0c")) == WireError::GROUP);
	CHECK(First_Error(From_Hex("0e")) == WireError::BAD_WIRE_TYPE);
	CHECK(First_Error(From_Hex("0f")) == WireError::BAD_WIRE_TYPE);
	CHECK(First_Error(From_Hex("0a05616263")) == WireError::TRUNCATED);
	CHECK(First_Error(From_Hex("0affffffffffffffffff01")) == WireError::TRUNCATED);
	CHECK(First_Error(From_Hex("0d010203")) == WireError::TRUNCATED);
	CHECK(First_Error(From_Hex("0901020304050607")) == WireError::TRUNCATED);
	CHECK(First_Error(From_Hex("08")) == WireError::TRUNCATED);
	CHECK(First_Error(From_Hex("08ffffffffffffffffff7f")) == WireError::VARINT_TOO_LONG);
	CHECK_MESSAGE(First_Error(From_Hex("f8ffffff0f01")) == WireError::NONE, "field 2^29 - 1 is valid");
}


TEST_CASE("repeated uint32 fields read packed and unpacked alike")
{
	ByteVector const bytes = From_Hex("0803" "0a03 8e02 05" "0807");
	WireReader reader(bytes);
	WireField field;
	std::vector<std::uint32_t> values;
	while (reader.Next(field)) {
		CHECK(Read_Repeated_Uint32(field, values));
	}
	CHECK(values == std::vector<std::uint32_t>{ 3, 270, 5, 7 });

	WireField broken;
	broken.Type = WireType::LENGTH;
	ByteVector const cut = From_Hex("0380");
	broken.Body = cut;
	CHECK_MESSAGE(!Read_Repeated_Uint32(broken, values), "a packed body that ends inside a varint");

	WireField wide;
	wide.Type = WireType::VARINT;
	wide.Value = 0x100000005ull;
	values.clear();
	CHECK(Read_Repeated_Uint32(wide, values));
	CHECK(values == std::vector<std::uint32_t>{ 5 });
}


TEST_CASE("repeated numeric fields read packed, unpacked, and mixed occurrences alike")
{
	// Fields 1, 2, and 3 are repeated uint64, fixed32, and fixed64, each split across occurrences.
	ByteVector const bytes = From_Hex(
		"0803" "0a03 8e02 05" "08 ffffffffffffffffff01"
		"15 01000000" "1208 02000000 ffffffff" "15 03000000"
		"19 0200000000000000" "1a08 ffffffffffffffff");
	WireReader reader(bytes);
	WireField field;
	std::vector<std::uint64_t> varints;
	std::vector<std::uint32_t> fixed32s;
	std::vector<std::uint64_t> fixed64s;
	while (reader.Next(field)) {
		switch (field.Number) {
			case 1: CHECK(Read_Repeated_Varint(field, varints)); break;
			case 2: CHECK(Read_Repeated_Fixed32(field, fixed32s)); break;
			case 3: CHECK(Read_Repeated_Fixed64(field, fixed64s)); break;
			default: FAIL("unexpected field"); break;
		}
	}
	CHECK(reader.Error() == WireError::NONE);
	CHECK(varints == std::vector<std::uint64_t>{ 3, 270, 5, UINT64_MAX });
	CHECK(fixed32s == std::vector<std::uint32_t>{ 1, 2, UINT32_MAX, 3 });
	CHECK(fixed64s == std::vector<std::uint64_t>{ 2, UINT64_MAX });
}


TEST_CASE("repeated numeric fields read the bytes protoc writes")
{
	// protoc output for repeated sint64 [-1, INT64_MIN, INT64_MAX] packed as field 5, float
	// [1.5, -0.0, inf, NaN] as field 11, and double [1.5, -0.0, -inf] as field 12.
	ByteVector const bytes = From_Hex(
		"2a15 01 ffffffffffffffffff01 feffffffffffffffff01"
		"5a10 0000c03f 00000080 0000807f 0000c07f"
		"6218 000000000000f83f 0000000000000080 000000000000f0ff");
	WireReader reader(bytes);
	WireField field;
	std::vector<std::uint64_t> sint64s;
	std::vector<std::uint32_t> floats;
	std::vector<std::uint64_t> doubles;
	REQUIRE(reader.Next(field));
	CHECK(Read_Repeated_Varint(field, sint64s));
	REQUIRE(reader.Next(field));
	CHECK(Read_Repeated_Fixed32(field, floats));
	REQUIRE(reader.Next(field));
	CHECK(Read_Repeated_Fixed64(field, doubles));
	CHECK(!reader.Next(field));
	CHECK(reader.Error() == WireError::NONE);

	REQUIRE(sint64s.size() == 3);
	CHECK(Unzigzag64(sint64s[0]) == -1);
	CHECK(Unzigzag64(sint64s[1]) == INT64_MIN);
	CHECK(Unzigzag64(sint64s[2]) == INT64_MAX);

	REQUIRE(floats.size() == 4);
	CHECK(std::bit_cast<float>(floats[0]) == 1.5f);
	CHECK(std::signbit(std::bit_cast<float>(floats[1])));
	CHECK(std::bit_cast<float>(floats[2]) == std::numeric_limits<float>::infinity());
	CHECK(std::isnan(std::bit_cast<float>(floats[3])));

	REQUIRE(doubles.size() == 3);
	CHECK(std::bit_cast<double>(doubles[0]) == 1.5);
	CHECK(std::signbit(std::bit_cast<double>(doubles[1])));
	CHECK(std::bit_cast<double>(doubles[2]) == -std::numeric_limits<double>::infinity());
}


static WireField Packed_Field(ByteVector const & body)
{
	WireField field;
	field.Type = WireType::LENGTH;
	field.Body = body;
	return(field);
}


static WireField Scalar_Field(WireType type, std::uint64_t value)
{
	WireField field;
	field.Type = type;
	field.Value = value;
	return(field);
}


TEST_CASE("repeated numeric readers refuse cut, misaligned, and mistyped fields")
{
	ByteVector const empty;
	ByteVector const cut = From_Hex("0380");
	ByteVector const overlong = From_Hex("ffffffffffffffffff02");
	ByteVector const four = From_Hex("01000000");
	ByteVector const five = From_Hex("0100000002");
	ByteVector const twelve = From_Hex("010000000000000002000000");

	std::vector<std::uint64_t> varints;
	CHECK(Read_Repeated_Varint(Packed_Field(empty), varints));
	CHECK(varints.empty());
	CHECK_MESSAGE(!Read_Repeated_Varint(Packed_Field(cut), varints), "a packed body that ends inside a varint");
	CHECK(!Read_Repeated_Varint(Packed_Field(overlong), varints));
	CHECK(!Read_Repeated_Varint(Scalar_Field(WireType::FIXED32, 1), varints));
	CHECK(!Read_Repeated_Varint(Scalar_Field(WireType::FIXED64, 1), varints));

	std::vector<std::uint32_t> fixed32s;
	CHECK(Read_Repeated_Fixed32(Packed_Field(empty), fixed32s));
	CHECK(fixed32s.empty());
	CHECK_MESSAGE(!Read_Repeated_Fixed32(Packed_Field(five), fixed32s), "a body that is not whole values");
	CHECK(!Read_Repeated_Fixed32(Packed_Field(cut), fixed32s));
	CHECK(!Read_Repeated_Fixed32(Scalar_Field(WireType::VARINT, 1), fixed32s));
	CHECK(!Read_Repeated_Fixed32(Scalar_Field(WireType::FIXED64, 1), fixed32s));
	CHECK_MESSAGE(fixed32s.empty(), "a misaligned body appends nothing");

	std::vector<std::uint64_t> fixed64s;
	CHECK(Read_Repeated_Fixed64(Packed_Field(empty), fixed64s));
	CHECK(fixed64s.empty());
	CHECK(!Read_Repeated_Fixed64(Packed_Field(four), fixed64s));
	CHECK(!Read_Repeated_Fixed64(Packed_Field(twelve), fixed64s));
	CHECK(!Read_Repeated_Fixed64(Scalar_Field(WireType::VARINT, 1), fixed64s));
	CHECK(!Read_Repeated_Fixed64(Scalar_Field(WireType::FIXED32, 1), fixed64s));
	CHECK(fixed64s.empty());
}


TEST_CASE("typed accessors convert a field value as protobuf parsers do")
{
	CHECK(Scalar_Field(WireType::VARINT, 0).As_Uint32() == 0u);
	CHECK(Scalar_Field(WireType::VARINT, UINT32_MAX).As_Uint32() == UINT32_MAX);
	CHECK_MESSAGE(Scalar_Field(WireType::VARINT, 0x100000005ull).As_Uint32() == 5u, "a wider value keeps its low 32 bits");
	CHECK(Scalar_Field(WireType::VARINT, UINT64_MAX).As_Uint32() == UINT32_MAX);
	CHECK(Scalar_Field(WireType::FIXED32, UINT32_MAX).As_Uint32() == UINT32_MAX);

	CHECK(Scalar_Field(WireType::VARINT, 0).As_Uint64() == 0u);
	CHECK(Scalar_Field(WireType::VARINT, UINT64_MAX).As_Uint64() == UINT64_MAX);
	CHECK(Scalar_Field(WireType::FIXED64, 0x0102030405060708ull).As_Uint64() == 0x0102030405060708ull);

	CHECK_MESSAGE(Scalar_Field(WireType::VARINT, UINT64_MAX).As_Int32() == -1, "int32 -1 as protobuf writes it");
	CHECK(Scalar_Field(WireType::VARINT, 0xFFFFFFFF80000000ull).As_Int32() == INT32_MIN);
	CHECK(Scalar_Field(WireType::VARINT, 0x7FFFFFFFull).As_Int32() == INT32_MAX);
	CHECK(Scalar_Field(WireType::VARINT, 0x80000000ull).As_Int32() == INT32_MIN);
	CHECK(Scalar_Field(WireType::VARINT, 0x100000005ull).As_Int32() == 5);
	CHECK(Scalar_Field(WireType::FIXED32, 0xFFFFFFFEull).As_Int32() == -2);

	CHECK(Scalar_Field(WireType::VARINT, UINT64_MAX).As_Int64() == -1);
	CHECK(Scalar_Field(WireType::VARINT, 1ull << 63).As_Int64() == INT64_MIN);
	CHECK(Scalar_Field(WireType::VARINT, 0x7FFFFFFFFFFFFFFFull).As_Int64() == INT64_MAX);
	CHECK(Scalar_Field(WireType::FIXED64, 0xFFFFFFFFFFFFFFFEull).As_Int64() == -2);

	CHECK(Scalar_Field(WireType::VARINT, 0).As_Sint32() == 0);
	CHECK(Scalar_Field(WireType::VARINT, 1).As_Sint32() == -1);
	CHECK(Scalar_Field(WireType::VARINT, 2).As_Sint32() == 1);
	CHECK(Scalar_Field(WireType::VARINT, 0xFFFFFFFEull).As_Sint32() == INT32_MAX);
	CHECK(Scalar_Field(WireType::VARINT, 0xFFFFFFFFull).As_Sint32() == INT32_MIN);
	CHECK(Scalar_Field(WireType::VARINT, 0x100000001ull).As_Sint32() == -1);

	CHECK(Scalar_Field(WireType::VARINT, 3).As_Sint64() == -2);
	CHECK(Scalar_Field(WireType::VARINT, 0xFFFFFFFFFFFFFFFEull).As_Sint64() == INT64_MAX);
	CHECK(Scalar_Field(WireType::VARINT, UINT64_MAX).As_Sint64() == INT64_MIN);

	CHECK(!Scalar_Field(WireType::VARINT, 0).As_Bool());
	CHECK(Scalar_Field(WireType::VARINT, 1).As_Bool());
	CHECK(Scalar_Field(WireType::VARINT, 2).As_Bool());
	CHECK(Scalar_Field(WireType::VARINT, 1ull << 32).As_Bool());

	CHECK(Scalar_Field(WireType::FIXED32, 0x3FC00000ull).As_Float() == 1.5f);
	CHECK(Scalar_Field(WireType::FIXED32, 0xFFFFFFFF3FC00000ull).As_Float() == 1.5f);
	CHECK(std::bit_cast<std::uint32_t>(Scalar_Field(WireType::FIXED32, 0x80000000ull).As_Float()) == 0x80000000u);
	CHECK(std::bit_cast<std::uint32_t>(Scalar_Field(WireType::FIXED32, 0x7FC00000ull).As_Float()) == 0x7FC00000u);
	CHECK(std::isnan(Scalar_Field(WireType::FIXED32, 0x7FC00000ull).As_Float()));
	CHECK(Scalar_Field(WireType::FIXED32, 0x7F800000ull).As_Float() == std::numeric_limits<float>::infinity());
	CHECK(Scalar_Field(WireType::FIXED32, 0xFF800000ull).As_Float() == -std::numeric_limits<float>::infinity());

	CHECK(Scalar_Field(WireType::FIXED64, 0x3FF8000000000000ull).As_Double() == 1.5);
	CHECK(std::bit_cast<std::uint64_t>(Scalar_Field(WireType::FIXED64, 0x8000000000000000ull).As_Double()) == 0x8000000000000000ull);
	CHECK(std::bit_cast<std::uint64_t>(Scalar_Field(WireType::FIXED64, 0x7FF8000000000000ull).As_Double()) == 0x7FF8000000000000ull);
	CHECK(std::isnan(Scalar_Field(WireType::FIXED64, 0x7FF8000000000000ull).As_Double()));
	CHECK(Scalar_Field(WireType::FIXED64, 0x7FF0000000000000ull).As_Double() == std::numeric_limits<double>::infinity());
	CHECK(Scalar_Field(WireType::FIXED64, 0xFFF0000000000000ull).As_Double() == -std::numeric_limits<double>::infinity());
}


// A crash handler appends a field to bytes it encoded earlier, into storage it set aside then.
TEST_CASE("a text field written without allocating matches the writer and fits or writes nothing")
{
	std::string const text(200, 'x');
	Bytes expected;
	WireWriter(expected).String(10, text);

	std::uint8_t buffer[256] = {};
	std::size_t offset = 3;
	REQUIRE(Put_Text_Field(buffer, offset, 10, text));
	CHECK(offset == 3 + expected.size());
	CHECK(Bytes(buffer + 3, buffer + offset) == expected);

	std::size_t full = 256 - expected.size() + 1;
	CHECK_FALSE(Put_Text_Field(buffer, full, 10, text));
	CHECK_MESSAGE(full == 256 - expected.size() + 1, "a field that does not fit leaves the offset alone");

	std::size_t empty = 0;
	REQUIRE(Put_Text_Field(buffer, empty, 3, ""));
	CHECK_MESSAGE(Hex(Bytes(buffer, buffer + empty)) == "1a00", "empty text is written with its tag");

	std::size_t bad = 0;
	CHECK_FALSE(Put_Text_Field(buffer, bad, 0, "a"));
	CHECK_FALSE(Put_Text_Field(buffer, bad, MAX_FIELD_NUMBER + 1, "a"));
	CHECK(bad == 0);
}
