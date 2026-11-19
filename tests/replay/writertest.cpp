/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "replay/replaywriter.h"

#include "fixtures.h"
#include "rawfile.h"

#include "doctest/doctest.h"

#include <cstdio>
#include <fstream>
#include <iterator>

using namespace Replay;


namespace {

Frame Hash_Frame(std::uint32_t hash)
{
	Frame frame;
	frame.Hash = hash;
	return(frame);
}


// Decodes the records of an inflated frame stream, stopping at the terminator or at the first
// record cut short.
std::vector<Frame> Parse_Records(ByteVector const & stream, bool * terminated = nullptr)
{
	std::vector<Frame> records;
	std::size_t offset = 0;
	if (terminated != nullptr) {
		*terminated = false;
	}
	while (offset < stream.size()) {
		std::uint64_t length = 0;
		if (Read_Varint(stream, offset, length) != WireError::NONE) {
			break;
		}
		if (length == 0) {
			if (terminated != nullptr) {
				*terminated = true;
			}
			break;
		}
		if (length > stream.size() - offset) {
			break;
		}
		Frame frame;
		REQUIRE(Decode(ByteSpan(stream.data() + offset, std::size_t(length)), frame));
		records.push_back(frame);
		offset += std::size_t(length);
	}
	return(records);
}


ByteVector Frames_Body(ByteVector const & file, SectionEntry const & frames)
{
	std::size_t end = (frames.Stored == ~0ull) ? file.size() : frames.Offset + std::size_t(frames.Stored);
	ByteVector stream;
	Inflate_All(ByteSpan(file.data() + frames.Offset, end - frames.Offset), stream, ~0ull);
	return(stream);
}


// Records a short game with a checkpoint and statistics. Returns the first result that was not
// OK, or OK.
WriteResult Record_Game(FileClass & sink, std::uint32_t frames)
{
	Writer writer(sink);
	ByteVector const init = Sample_Init();
	ByteVector const map = Bytes_Of("[Basic]\nName=Synthetic Map\n");
	WriteResult result = writer.Begin(Sample_Info(), Sample_Launch(), init, ByteSpan(map));
	for (std::uint32_t frame = 0; frame < frames && result == WriteResult::OK; frame++) {
		Frame record = Hash_Frame(frame * 7);
		if (frame % 10 == 0) {
			Event event;
			event.Type = 3;
			event.House = frame % 2;
			event.Payload = { std::uint8_t(frame), 1, 2, 3 };
			record.Events.push_back(event);
		}
		result = writer.Add_Frame(frame, record);
	}
	if (result == WriteResult::OK) {
		Checkpoint checkpoint;
		checkpoint.Frame = 0;
		checkpoint.State = Bytes_Of("state");
		result = writer.Add_Checkpoint(checkpoint);
	}
	if (result == WriteResult::OK) {
		RecordingSummary summary;
		summary.LastFrame = frames - 1;
		summary.Completed = true;
		Statistics statistics;
		statistics.Houses.resize(2);
		statistics.Houses[1].House = 1;
		result = writer.Close(summary, &statistics);
	}
	if (result != WriteResult::OK) {
		CHECK(writer.Add_Frame(frames + 1, Hash_Frame(1)) == WriteResult::FAILED);
		CHECK(writer.Failed());
	}
	return(result);
}

}


TEST_CASE("the writer holds the first sections until the first record")
{
	MemoryFile sink;
	Writer writer(sink);
	ByteVector const init = Sample_Init();
	REQUIRE(writer.Begin(Sample_Info(), Sample_Launch(), init, std::nullopt) == WriteResult::OK);
	CHECK(sink.Written().empty());
	CHECK_MESSAGE(writer.Add_Frame(3, Frame()) == WriteResult::OK, "an empty record is accepted");
	CHECK_MESSAGE(sink.Written().empty(), "and not written");

	REQUIRE(writer.Add_Frame(5, Hash_Frame(1)) == WriteResult::OK);
	auto const sections = Walk_Sections(sink.Written());
	CHECK(Section_Tags(sections) == "INFO,LNCH,INIT,FRMS");
	CHECK(Hex(ByteSpan(sink.Written().data(), 8)) == "434e435201000000");
	CHECK(sections[0].Codec == 0);
	CHECK(sections[1].Codec == 0);
	CHECK(sections[2].Codec == 1);
	CHECK_MESSAGE(sections[3].Stored == ~0ull, "the frame stream is open");
	CHECK(sections[3].Raw == ~0ull);
	CHECK_MESSAGE(sink.Flushed().size() >= sections[3].Offset, "the header reaches the OS at once");

	RecordingInfo info;
	REQUIRE(Decode(ByteSpan(sink.Written().data() + sections[0].Offset, std::size_t(sections[0].Stored)), info));
	CHECK(info.SealVersion == SEAL_VERSION);
	CHECK(info.Engine == "opents");

	CHECK(writer.Flush() == WriteResult::OK);
	auto const records = Parse_Records(Frames_Body(sink.Flushed(), Walk_Sections(sink.Flushed())[3]));
	REQUIRE(records.size() == 1);
	CHECK_MESSAGE(records[0].Skipped == 5, "the first record counts from frame -1");
}


TEST_CASE("a closed file holds every section in order with real sizes")
{
	MemoryFile sink;
	REQUIRE(Record_Game(sink, 1000) == WriteResult::OK);
	ByteVector const & file = sink.Written();
	auto const sections = Walk_Sections(file);
	CHECK(Section_Tags(sections) == "INFO,LNCH,MAP ,INIT,FRMS,CKPT,STAT,END ");
	REQUIRE(sections.size() == 8);
	CHECK_MESSAGE(sections.back().Offset + sections.back().Stored == file.size(), "END is last");

	SectionEntry const & frames = sections[4];
	CHECK(frames.Stored != ~0ull);
	ByteVector const stream = Frames_Body(file, frames);
	CHECK(stream.size() == frames.Raw);
	bool terminated = false;
	auto const records = Parse_Records(stream, &terminated);
	CHECK(terminated);
	REQUIRE(records.size() == 1000);
	CHECK(records[0].Skipped == 0);
	CHECK(records[999].Skipped == 0);
	CHECK(records[900].Seals.size() == 1);
	CHECK(records[900].Seals[0].Through == 899);

	ByteVector summary_body(file.begin() + sections[7].Offset, file.end());
	RecordingSummary summary;
	REQUIRE(Decode(summary_body, summary));
	REQUIRE(summary.Seals.size() == 2);
	CHECK(summary.Seals[0].Hash == records[900].Seals[0].Hash);
	CHECK(summary.Seals[1].Through == 999);
}


TEST_CASE("the writer flushes the frame stream every 60 records")
{
	MemoryFile sink;
	Writer writer(sink);
	ByteVector const init = Sample_Init();
	REQUIRE(writer.Begin(Sample_Info(), Sample_Launch(), init, std::nullopt) == WriteResult::OK);
	for (std::uint32_t frame = 0; frame < 130; frame++) {
		REQUIRE(writer.Add_Frame(frame, Hash_Frame(frame)) == WriteResult::OK);
		auto const sections = Walk_Sections(sink.Flushed());
		std::size_t const flushed = (sections.size() == 4) ? Parse_Records(Frames_Body(sink.Flushed(), sections[3])).size() : 0;
		CHECK_MESSAGE(flushed == (frame + 1) / 60 * 60, "after frame ", frame);
	}
}


TEST_CASE("a recording in which no frame ran has an empty frame stream and no seals")
{
	MemoryFile sink;
	Writer writer(sink);
	ByteVector const init = Sample_Init();
	REQUIRE(writer.Begin(Sample_Info(), Sample_Launch(), init, std::nullopt) == WriteResult::OK);
	REQUIRE(writer.Close(RecordingSummary(), nullptr) == WriteResult::OK);

	auto const sections = Walk_Sections(sink.Written());
	CHECK(Section_Tags(sections) == "INFO,LNCH,INIT,FRMS,END ");
	CHECK(Hex(Frames_Body(sink.Written(), sections[3])) == "00");
	CHECK(sections[3].Raw == 1);
	CHECK_MESSAGE(sections[4].Stored == 0, "an empty summary");
	CHECK(writer.Add_Frame(0, Hash_Frame(1)) == WriteResult::STATE);
	CHECK(writer.Close(RecordingSummary(), nullptr) == WriteResult::STATE);
}


TEST_CASE("the writer refuses input that breaks the format and keeps recording")
{
	MemoryFile sink;
	Writer writer(sink);
	ByteVector const init = Sample_Init();

	Launch unordered = Sample_Launch();
	std::swap(unordered.Options[0], unordered.Options[1]);
	CHECK(writer.Begin(Sample_Info(), unordered, init, std::nullopt) == WriteResult::REFUSED);
	CHECK(writer.Add_Frame(0, Hash_Frame(1)) == WriteResult::STATE);
	REQUIRE(writer.Begin(Sample_Info(), Sample_Launch(), init, std::nullopt) == WriteResult::OK);
	CHECK(writer.Begin(Sample_Info(), Sample_Launch(), init, std::nullopt) == WriteResult::STATE);

	REQUIRE(writer.Add_Frame(10, Hash_Frame(1)) == WriteResult::OK);
	CHECK(writer.Add_Frame(10, Hash_Frame(2)) == WriteResult::STATE);
	CHECK(writer.Add_Frame(9, Hash_Frame(2)) == WriteResult::STATE);

	Frame big = Hash_Frame(3);
	big.Events.resize(1);
	big.Events[0].Payload.resize(4097);
	CHECK(writer.Add_Frame(11, big) == WriteResult::REFUSED);
	big.Events[0].Payload.resize(4096);
	CHECK(writer.Add_Frame(11, big) == WriteResult::OK);

	Checkpoint checkpoint;
	checkpoint.Frame = 5;
	CHECK(writer.Add_Checkpoint(checkpoint) == WriteResult::OK);
	CHECK(writer.Add_Checkpoint(checkpoint) == WriteResult::REFUSED);

	RecordingSummary summary;
	CHECK_MESSAGE(writer.Close(summary, nullptr) == WriteResult::REFUSED, "frames ran, so last_frame is needed");
	summary.LastFrame = 10;
	CHECK(writer.Close(summary, nullptr) == WriteResult::REFUSED);
	summary.LastFrame = 11;
	CHECK(writer.Close(summary, nullptr) == WriteResult::OK);
	CHECK(!writer.Failed());
}


TEST_CASE("the writer refuses an income kind table that breaks the rules, and income of an unlisted kind")
{
	auto with_kinds = [](std::vector<std::pair<std::uint32_t, std::string>> const & listed) {
		RecordingInfo info = Sample_Info();
		for (auto const & entry : listed) {
			IncomeKind kind;
			kind.Number = entry.first;
			kind.Id = entry.second;
			info.IncomeKinds.push_back(kind);
		}
		return(info);
	};
	auto paying = [](std::vector<std::uint32_t> const & used) {
		Frame record = Hash_Frame(1);
		for (std::uint32_t number : used) {
			Income income;
			income.Kind = number;
			income.Amount = 5;
			record.Income.push_back(income);
		}
		return(record);
	};

	MemoryFile sink;
	Writer writer(sink);
	ByteVector const init = Sample_Init();
	CHECK_MESSAGE(writer.Begin(with_kinds({ { 1, "" } }), Sample_Launch(), init, std::nullopt) == WriteResult::REFUSED, "an empty id");
	CHECK_MESSAGE(writer.Begin(with_kinds({ { 1, "Crate" }, { 1, "Trigger" } }), Sample_Launch(), init, std::nullopt) == WriteResult::REFUSED, "a number listed twice");
	CHECK_MESSAGE(writer.Begin(with_kinds({ { 1, "Crate" }, { 2, "Crate" } }), Sample_Launch(), init, std::nullopt) == WriteResult::REFUSED, "an id listed twice");
	REQUIRE(writer.Begin(with_kinds({ { 1, "Crate" }, { 3, "Sold" } }), Sample_Launch(), init, std::nullopt) == WriteResult::OK);

	CHECK_MESSAGE(writer.Add_Frame(0, paying({ 1, 2 })) == WriteResult::REFUSED, "a kind INFO does not list");
	CHECK(writer.Add_Frame(0, paying({ 3, 1 })) == WriteResult::OK);
	CHECK(writer.Add_Frame(1, paying({ 1 })) == WriteResult::OK);

	RecordingSummary summary;
	summary.LastFrame = 1;
	CHECK(writer.Close(summary, nullptr) == WriteResult::OK);
	CHECK(!writer.Failed());

	MemoryFile big_sink;
	Writer big(big_sink);
	std::vector<std::pair<std::uint32_t, std::string>> many;
	for (std::uint32_t number = 0; number <= LimitsType().IncomeKinds; number++) {
		many.emplace_back(number, "Kind/" + std::to_string(number));
	}
	CHECK_MESSAGE(big.Begin(with_kinds(many), Sample_Launch(), init, std::nullopt) == WriteResult::REFUSED, "one kind past the limit");
	many.pop_back();
	CHECK(big.Begin(with_kinds(many), Sample_Launch(), init, std::nullopt) == WriteResult::OK);
}


TEST_CASE("a file failure at any call stops the writer and never stops the caller")
{
	MemoryFile good;
	REQUIRE(Record_Game(good, 200) == WriteResult::OK);

	int calls = 0;
	{
		FailingFile counter(1000000, 0);
		REQUIRE(Record_Game(counter, 200) == WriteResult::OK);
		calls = counter.Calls;
	}
	REQUIRE(calls > 10);

	for (int fail_at = 1; fail_at <= calls; fail_at++) {
		for (std::size_t partial : { std::size_t(0), std::size_t(3) }) {
			FailingFile sink(fail_at, partial);
			CHECK_MESSAGE(Record_Game(sink, 200) == WriteResult::FAILED, "failing call ", fail_at);
			CHECK(sink.Written().size() <= good.Written().size());
		}
	}
}


TEST_CASE("the writer puts the same bytes in a RawFileClass as in a RAMFileClass")
{
	MemoryFile memory;
	REQUIRE(Record_Game(memory, 300) == WriteResult::OK);

	char const * name = "replay-writer-rawfile.tmp";
	std::remove(name);
	{
		RawFileClass file(name);
		REQUIRE(file.Open(FileClass::WRITE));
		REQUIRE(Record_Game(file, 300) == WriteResult::OK);
		file.Close();
	}

	std::ifstream in(name, std::ios::binary);
	ByteVector const bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	in.close();
	CHECK(bytes == memory.Written());
	std::remove(name);

	RawFileClass closed(name);
	CHECK_MESSAGE(Record_Game(closed, 300) == WriteResult::FAILED, "a file the caller did not open");
	CHECK(!closed.Is_Available());
}
