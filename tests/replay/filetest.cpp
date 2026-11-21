/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "bfiofile.h"
#include "ramfile.h"
#include "rawfile.h"

#include "replaytest.h"

#include "doctest/doctest.h"

#include <cstdio>
#include <cstring>


static ByteVector Read_Whole_File(char const * name)
{
	ByteVector bytes;
	std::FILE * file = std::fopen(name, "rb");
	if (file == nullptr) {
		return(bytes);
	}
	std::uint8_t buffer[256];
	std::size_t count;
	while ((count = std::fread(buffer, 1, sizeof(buffer), file)) > 0) {
		bytes.insert(bytes.end(), buffer, buffer + count);
	}
	std::fclose(file);
	return(bytes);
}


TEST_CASE("RawFileClass::Flush makes written bytes visible before the file closes")
{
	char const * name = "replay-rawfile-flush.tmp";
	std::remove(name);

	RawFileClass file(name);
	REQUIRE(file.Open(FileClass::WRITE));

	ByteVector const first = Bytes_Of("first part");
	CHECK(file.Write(first.data(), int(first.size())) == int(first.size()));
	CHECK_MESSAGE(Read_Whole_File(name).empty(), "a small write waits in the runtime's buffer");
	CHECK(file.Flush());
	CHECK(Read_Whole_File(name) == first);

	ByteVector const second = Bytes_Of(" and more");
	CHECK(file.Write(second.data(), int(second.size())) == int(second.size()));
	CHECK(file.Flush());
	CHECK(Read_Whole_File(name) == Bytes_Of("first part and more"));

	file.Close();
	CHECK_MESSAGE(!file.Flush(), "a closed file has nothing to flush");
	std::remove(name);
}


static ByteVector Contents_Of(RAMFileClass const & file)
{
	std::span<std::uint8_t const> const contents = file.Contents();
	return(ByteVector(contents.begin(), contents.end()));
}


TEST_CASE("BufferIOFileClass::Flush writes its cache and flushes the file beneath it")
{
	char const * name = "replay-bfiofile-flush.tmp";
	ByteVector const first = Bytes_Of("first part");
	ByteVector const second = Bytes_Of(" and more");

	SUBCASE("with a cache") {
		std::remove(name);
		BufferIOFileClass file(name);
		REQUIRE(file.Cache(4096));
		REQUIRE(file.Open(FileClass::WRITE));

		CHECK(file.Write(first.data(), int(first.size())) == int(first.size()));
		CHECK_MESSAGE(Read_Whole_File(name).empty(), "the bytes wait in the cache");
		CHECK(file.Flush());
		CHECK(Read_Whole_File(name) == first);

		CHECK(file.Write(second.data(), int(second.size())) == int(second.size()));
		CHECK(file.Flush());
		CHECK(Read_Whole_File(name) == Bytes_Of("first part and more"));

		file.Close();
		CHECK_MESSAGE(!file.Flush(), "a closed file has nothing to flush");
	}

	SUBCASE("without a cache") {
		std::remove(name);
		BufferIOFileClass file(name);
		REQUIRE(file.Open(FileClass::WRITE));

		CHECK(file.Write(first.data(), int(first.size())) == int(first.size()));
		CHECK_MESSAGE(Read_Whole_File(name).empty(), "a small write waits in the runtime's buffer");
		CHECK(file.Flush());
		CHECK(Read_Whole_File(name) == first);

		file.Close();
		CHECK(!file.Flush());
	}

	std::remove(name);
}


TEST_CASE("BufferIOFileClass writes a change that starts inside its cache at the right bytes")
{
	char const * name = "replay-bfiofile-commit.tmp";
	ByteVector const before = Bytes_Of("0123456789");

	std::remove(name);
	{
		RawFileClass raw(name);
		REQUIRE(raw.Open(FileClass::WRITE));
		REQUIRE(raw.Write(before.data(), int(before.size())) == int(before.size()));
	}

	{
		BufferIOFileClass file(name);
		REQUIRE(file.Cache(4096));
		REQUIRE(file.Open(FileClass::READ | FileClass::WRITE));
		REQUIRE(file.Seek(4, SEEK_SET) == 4);
		CHECK(file.Write("AB", 2) == 2);
		file.Close();
	}

	CHECK(Read_Whole_File(name) == Bytes_Of("0123AB6789"));
	std::remove(name);
}


TEST_CASE("BufferIOFileClass reports a cache that does not reach the file")
{
	char const * name = "replay-bfiofile-fail.tmp";
	ByteVector const first = Bytes_Of("first part");

	std::remove(name);
	{
		BufferIOFileClass file(name);
		REQUIRE(file.Cache(BufferIOFileClass::MINIMUM_BUFFER_SIZE));
		REQUIRE(file.Open(FileClass::WRITE));
		CHECK(file.Write(first.data(), int(first.size())) == int(first.size()));

		// The stream beneath the cache is reopened for reading only, so every write to it fails.
		std::FILE * const handle = file.Get_File_Handle();
		REQUIRE(handle != nullptr);
		REQUIRE(std::freopen(name, "rb", handle) == handle);

		CHECK_MESSAGE(!file.Flush(), "the cached bytes did not reach the file");
		CHECK(Read_Whole_File(name).empty());

		ByteVector const block(BufferIOFileClass::MINIMUM_BUFFER_SIZE + 10, 0x5A);
		CHECK_MESSAGE(file.Write(block.data(), int(block.size())) == 0, "a write that fills the cache counts only bytes that reached the file");
		file.Close();
		CHECK(Read_Whole_File(name).empty());
	}
	std::remove(name);
}


TEST_CASE("BufferIOFileClass::Flush writes a cache turned on after the file opened for writing")
{
	char const * name = "replay-bfiofile-late.tmp";
	ByteVector const first = Bytes_Of("first part");

	std::remove(name);
	{
		BufferIOFileClass file(name);
		REQUIRE(file.Open(FileClass::WRITE));
		REQUIRE(file.Cache(BufferIOFileClass::MINIMUM_BUFFER_SIZE));
		CHECK(file.Write(first.data(), int(first.size())) == int(first.size()));
		CHECK_MESSAGE(Read_Whole_File(name).empty(), "the bytes wait in the cache");
		CHECK(file.Flush());
		CHECK(Read_Whole_File(name) == first);
	}
	std::remove(name);
}


TEST_CASE("a file class with no buffer of its own flushes by doing nothing")
{
	RAMFileClass file;
	REQUIRE(file.Open(FileClass::WRITE));
	FileClass & base = file;
	CHECK(base.Flush());
}


TEST_CASE("a growable RAMFileClass grows on a write past its end")
{
	RAMFileClass file;
	CHECK(file.Size() == 0);
	CHECK(file.Contents().empty());
	REQUIRE(file.Open(FileClass::WRITE));

	ByteVector expected = Bytes_Of("first part");
	CHECK(file.Write(expected.data(), int(expected.size())) == int(expected.size()));
	ByteVector const large(100000, 0x5A);
	CHECK(file.Write(large.data(), int(large.size())) == int(large.size()));
	expected.insert(expected.end(), large.begin(), large.end());
	CHECK(file.Size() == int(expected.size()));
	CHECK(file.Seek(0, SEEK_CUR) == int(expected.size()));

	CHECK(file.Seek(5, SEEK_SET) == 5);
	CHECK(file.Write("PATCH", 5) == 5);
	std::memcpy(expected.data() + 5, "PATCH", 5);
	CHECK_MESSAGE(file.Size() == int(expected.size()), "an overwrite inside the file leaves its size");
	CHECK_MESSAGE(file.Seek(10, SEEK_END) == int(expected.size()), "a seek stops at the end");
	CHECK_MESSAGE(file.Seek(-4, SEEK_END) == int(expected.size()) - 4, "and moves back from it");
	file.Close();
	CHECK(Contents_Of(file) == expected);

	REQUIRE(file.Open(FileClass::READ));
	ByteVector back(expected.size() + 10);
	CHECK_MESSAGE(file.Read(back.data(), int(back.size())) == int(expected.size()), "a read stops at the end");
	back.resize(expected.size());
	CHECK(back == expected);
	CHECK_MESSAGE(file.Write("PATCH", 5) == 0, "a file open for reading takes no write");
	file.Close();

	REQUIRE(file.Open(FileClass::WRITE));
	CHECK_MESSAGE(file.Size() == 0, "opening for writing empties it");
	CHECK(file.Contents().empty());
	file.Close();
}


TEST_CASE("a growable RAMFileClass appends its own bytes when the write moves them")
{
	RAMFileClass file;
	REQUIRE(file.Open(FileClass::WRITE));

	ByteVector expected;
	for (int i = 0; i < 100; i++) {
		expected.push_back(std::uint8_t(i));
	}
	REQUIRE(file.Write(expected.data(), int(expected.size())) == int(expected.size()));

	// Each append doubles the file, so its storage is reallocated during the write.
	for (int round = 0; round < 4; round++) {
		std::span<std::uint8_t const> const contents = file.Contents();
		CHECK(file.Write(contents.data(), int(contents.size())) == int(contents.size()));
		ByteVector const copy = expected;
		expected.insert(expected.end(), copy.begin(), copy.end());
		CHECK(Contents_Of(file) == expected);
	}
	file.Close();
}


TEST_CASE("a RAMFileClass over a fixed buffer stops writing at the buffer's end")
{
	char buffer[8] = {};
	RAMFileClass file(buffer, sizeof(buffer));
	CHECK(file.Size() == 8);
	REQUIRE(file.Open(FileClass::WRITE));
	CHECK(file.Size() == 0);
	CHECK(file.Write("0123456789", 10) == 8);
	CHECK(file.Write("x", 1) == 0);
	CHECK(file.Size() == 8);
	CHECK(file.Contents().data() == reinterpret_cast<std::uint8_t const *>(buffer));
	file.Close();
	CHECK(std::memcmp(buffer, "01234567", 8) == 0);
	CHECK(Contents_Of(file) == Bytes_Of("01234567"));

	RAMFileClass allocated(NULL, 4);
	REQUIRE(allocated.Open(FileClass::WRITE));
	CHECK_MESSAGE(allocated.Write("abcdef", 6) == 4, "a buffer the file allocated holds the length given");
	allocated.Close();
	CHECK(Contents_Of(allocated) == Bytes_Of("abcd"));

	RAMFileClass empty(NULL, 0);
	CHECK_MESSAGE(!empty.Open(FileClass::READ), "a fixed file with no buffer does not open");
}
