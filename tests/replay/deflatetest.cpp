/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "deflate.h"

#include "replaytest.h"

#include "doctest/doctest.h"

#include <algorithm>


namespace {

using ByteSpan = std::span<std::uint8_t const>;

// Text-like data that compresses, with enough variety to need dynamic blocks.
ByteVector Sample_Data(std::size_t size, unsigned seed)
{
	ByteVector data(size);
	unsigned state = seed;
	for (std::size_t i = 0; i < size; i++) {
		state = state * 1103515245u + 12345u;
		data[i] = std::uint8_t('a' + ((state >> 16) % 6));
	}
	return(data);
}


bool Is_Prefix(ByteVector const & prefix, ByteVector const & whole)
{
	return(prefix.size() <= whole.size() && std::equal(prefix.begin(), prefix.end(), whole.begin()));
}

}


TEST_CASE("deflate round trips whole streams, empty ones included")
{
	for (std::size_t size : { std::size_t(0), std::size_t(1), std::size_t(1000), std::size_t(300000) }) {
		ByteVector const data = Sample_Data(size, unsigned(size));
		ByteVector packed;
		REQUIRE(Deflate_All(data, packed));
		ByteVector unpacked;
		CHECK(Inflate_All(packed, unpacked, ~0ull) == InflateStatus::DONE);
		CHECK_MESSAGE(unpacked == data, "size ", size);
	}
}


TEST_CASE("after a sync flush the bytes so far inflate to exactly the data so far")
{
	ByteVector const data = Sample_Data(5000, 7);
	DeflaterClass deflater;
	ByteVector packed;
	std::size_t given = 0;
	for (std::size_t piece : { 1, 99, 900, 2000, 2000 }) {
		REQUIRE(deflater.Write(ByteSpan(data.data() + given, piece), packed));
		given += piece;
		REQUIRE(deflater.Sync_Flush(packed));
		REQUIRE(packed.size() >= 4);
		CHECK_MESSAGE(Hex(ByteSpan(packed.data() + packed.size() - 4, 4)) == "0000ffff",
			"a sync flush ends with an empty stored block");

		ByteVector unpacked;
		CHECK(Inflate_All(packed, unpacked, ~0ull) == InflateStatus::MORE);
		CHECK(unpacked == ByteVector(data.begin(), data.begin() + given));
	}
	REQUIRE(deflater.Finish(packed));
	CHECK_MESSAGE(!deflater.Write(data, packed), "nothing is written after the end");

	ByteVector unpacked;
	CHECK(Inflate_All(packed, unpacked, ~0ull) == InflateStatus::DONE);
	CHECK(unpacked == data);
}


TEST_CASE("a stream cut at any byte inflates to a prefix without an error")
{
	ByteVector const data = Sample_Data(4000, 3);
	DeflaterClass deflater;
	ByteVector packed;
	REQUIRE(deflater.Write(ByteSpan(data.data(), 1500), packed));
	REQUIRE(deflater.Sync_Flush(packed));
	std::size_t const flushed = packed.size();
	REQUIRE(deflater.Write(ByteSpan(data.data() + 1500, 2500), packed));
	REQUIRE(deflater.Finish(packed));

	for (std::size_t cut = 0; cut < packed.size(); cut++) {
		ByteVector unpacked;
		InflateStatus status = Inflate_All(ByteSpan(packed.data(), cut), unpacked, ~0ull);
		CHECK_MESSAGE(status == InflateStatus::MORE, "cut at ", cut);
		CHECK_MESSAGE(Is_Prefix(unpacked, data), "cut at ", cut);
		if (cut >= flushed) {
			CHECK_MESSAGE(unpacked.size() >= 1500, "everything before the flush survives a cut at ", cut);
		}
	}
}


TEST_CASE("input given one byte at a time inflates to the same data")
{
	ByteVector const data = Sample_Data(3000, 11);
	ByteVector packed;
	REQUIRE(Deflate_All(data, packed));

	InflaterClass inflater(~0ull);
	ByteVector unpacked;
	InflateStatus status = InflateStatus::MORE;
	for (std::size_t i = 0; i < packed.size(); i++) {
		REQUIRE(status == InflateStatus::MORE);
		status = inflater.Write(ByteSpan(packed.data() + i, 1), unpacked);
	}
	CHECK(status == InflateStatus::DONE);
	CHECK(unpacked == data);
	CHECK(inflater.Total_Out() == data.size());
}


TEST_CASE("inflate stops at its output limit and refuses broken data")
{
	ByteVector const data(100000, 'x');
	ByteVector packed;
	REQUIRE(Deflate_All(data, packed));

	ByteVector unpacked;
	CHECK(Inflate_All(packed, unpacked, 100000) == InflateStatus::DONE);
	CHECK(Inflate_All(packed, unpacked, 99999) == InflateStatus::LIMIT);
	CHECK(unpacked.size() == 99999);

	// A block of the reserved type 3.
	CHECK(Inflate_All(From_Hex("07"), unpacked, ~0ull) == InflateStatus::BROKEN);
	// A stored block whose length check disagrees with its length.
	CHECK(Inflate_All(From_Hex("010500fbff"), unpacked, ~0ull) == InflateStatus::BROKEN);
}


TEST_CASE("an inflater counts the input it used and leaves bytes after the stream unused")
{
	ByteVector const data = Sample_Data(50000, 7);
	ByteVector packed;
	REQUIRE(Deflate_All(data, packed));
	std::size_t const length = packed.size();
	packed.insert(packed.end(), { 0x00, 0x01, 0x02 });

	InflaterClass inflater(~0ull);
	ByteVector unpacked;
	CHECK(inflater.Write(packed, unpacked) == InflateStatus::DONE);
	CHECK(unpacked == data);
	CHECK(inflater.Total_In() == length);

	// An empty final block followed by one byte.
	InflaterClass trailing(~0ull);
	CHECK(trailing.Write(From_Hex("030000"), unpacked) == InflateStatus::DONE);
	CHECK(trailing.Total_In() == 2);
}


TEST_CASE("a lowered output limit refuses any further output")
{
	DeflaterClass deflater;
	ByteVector packed;
	REQUIRE(deflater.Write(Bytes_Of("head"), packed));
	REQUIRE(deflater.Sync_Flush(packed));
	std::size_t const flushed = packed.size();
	REQUIRE(deflater.Write(ByteVector(1 << 20, 0), packed));
	REQUIRE(deflater.Finish(packed));

	InflaterClass inflater(~0ull);
	ByteVector unpacked;
	REQUIRE(inflater.Write(ByteSpan(packed).first(flushed), unpacked) == InflateStatus::MORE);
	REQUIRE(unpacked == Bytes_Of("head"));
	inflater.Set_Limit(inflater.Total_Out());
	CHECK(inflater.Write(ByteSpan(packed).subspan(flushed), unpacked) == InflateStatus::LIMIT);
	CHECK(unpacked == Bytes_Of("head"));
}
