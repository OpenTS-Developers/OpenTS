/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "replay/replayreader.h"
#include "replay/replaywriter.h"

#include "crc.h"
#include "fixtures.h"
#include "rawfile.h"
#include "synthfiles.h"

#include "doctest/doctest.h"

#include <cstdio>
#include <new>
#include <random>

using namespace Replay;


namespace {

struct ReadBack {
	ReadResult Opened = ReadResult::OK;
	std::vector<std::pair<std::uint32_t, Replay::Bytes>> Records;
	FrameStep Last = FrameStep::CUT;
	ReadResult FrameError = ReadResult::OK;
};


ReadBack Read_All(ByteVector const & file, LimitsType const & limits = LimitsType())
{
	ReadBack back;
	InputFile source(file);
	Reader reader(source, limits);
	back.Opened = reader.Open();
	if (back.Opened != ReadResult::OK) {
		return(back);
	}

	FrameReader frames(reader);
	std::uint32_t frame = 0;
	Message::Frame record;
	while ((back.Last = frames.Next(frame, record)) == FrameStep::RECORD) {
		back.Records.emplace_back(frame, Encoded(record));
	}
	back.FrameError = frames.Error();
	return(back);
}


Message::Frame Game_Record(std::uint32_t frame)
{
	Message::Frame record;
	record.Hash = frame ^ 0x5A5A5A5A;
	if (frame % 25 == 0) {
		Message::Event event;
		event.Type = 4;
		event.House = 1;
		event.Payload = { std::uint8_t(frame), 9, 9 };
		record.Events.push_back(event);
	}
	if (frame == 0) {
		record.FrameMicros = 16666;
	}
	return(record);
}


// A complete recording of frames 0 to last with checkpoints at 0 and 100 and statistics.
ByteVector Recording(std::uint32_t last, bool close = true)
{
	MemoryFile sink;
	Writer writer(sink);
	ByteVector const init = Sample_Init();
	ByteVector const map = Bytes_Of("[Basic]\nName=Synthetic Map\n");
	REQUIRE(writer.Begin(Sample_Info(), Sample_Launch(), init, ByteSpan(map)) == WriteResult::OK);
	for (std::uint32_t frame = 0; frame <= last; frame++) {
		REQUIRE(writer.Add_Frame(frame, Game_Record(frame)) == WriteResult::OK);
	}
	if (!close) {
		return(sink.Written());
	}
	for (std::uint32_t frame : { 0u, 100u }) {
		Message::Checkpoint checkpoint;
		checkpoint.Frame = frame;
		checkpoint.State = Bytes_Of("checkpoint state");
		checkpoint.Crc32 = CRC::Memory(checkpoint.State.data(), unsigned(checkpoint.State.size()));
		REQUIRE(writer.Add_Checkpoint(checkpoint) == WriteResult::OK);
	}
	Message::Statistics statistics;
	statistics.Houses.resize(2);
	statistics.Houses[1].House = 1;
	Message::RecordingSummary summary;
	summary.LastFrame = last;
	summary.Completed = true;
	REQUIRE(writer.Close(summary, &statistics) == WriteResult::OK);
	return(sink.Written());
}


struct RawSection {
	char const * Tag;
	std::uint32_t Codec;
	ByteVector Body;
	std::optional<std::uint64_t> Stored;
	std::optional<std::uint64_t> Raw;
};


ByteVector Build_File(std::vector<RawSection> const & sections, std::uint32_t version = 1)
{
	ByteVector file = Bytes_Of("CNCR");
	Put_Fixed32(file, version);
	for (RawSection const & section : sections) {
		file.insert(file.end(), section.Tag, section.Tag + 4);
		Put_Fixed32(file, section.Codec);
		Put_Fixed64(file, section.Stored.value_or(section.Body.size()));
		Put_Fixed64(file, section.Raw.value_or(section.Body.size()));
		file.insert(file.end(), section.Body.begin(), section.Body.end());
	}
	return(file);
}


// The frame stream of a closed file holding records for frames 0 and 3.
ByteVector Closed_Frames(bool terminated = true)
{
	ByteVector raw;
	for (ByteVector const & record : { Encoded(Game_Record(0)), From_Hex("0802 1501000000") }) {
		Put_Varint(raw, record.size());
		raw.insert(raw.end(), record.begin(), record.end());
	}
	if (terminated) {
		raw.push_back(0);
	}
	return(raw);
}


std::vector<RawSection> Minimal_Sections(void)
{
	Message::RecordingInfo info = Sample_Info();
	info.SealVersion = SEAL_VERSION;
	return(std::vector<RawSection>{
		{ "INFO", 0, Encoded(info), {}, {} },
		{ "LNCH", 0, Encoded(Sample_Launch()), {}, {} },
		{ "INIT", 0, Sample_Init(), {}, {} },
		{ "FRMS", 0, Closed_Frames(), {}, {} },
		{ "END ", 0, ByteVector(), {}, {} },
	});
}


ReadResult Open_Result(ByteVector const & file, LimitsType const & limits = LimitsType())
{
	InputFile source(file);
	Reader reader(source, limits);
	return(reader.Open());
}


SealReport Seal_Report(ByteVector const & file)
{
	InputFile source(file);
	Reader reader(source);
	REQUIRE(reader.Open() == ReadResult::OK);
	return(Verify_Seals(reader));
}


// Stores the frame stream of a complete file again, uncompressed, after change has edited
// its records.
template<typename Change>
ByteVector Rewrite_Frames(ByteVector const & file, Change change)
{
	ByteVector raw;
	for (auto const & [frame, bytes] : Read_All(file).Records) {
		Message::Frame record;
		REQUIRE(Decode(bytes, record));
		change(frame, record);
		Replay::Bytes const encoded = Encoded(record);
		Put_Varint(raw, encoded.size());
		raw.insert(raw.end(), encoded.begin(), encoded.end());
	}
	raw.push_back(0);

	auto const sections = Walk_Sections(file);
	SectionEntry const & frames = sections[4];
	REQUIRE(frames.Tag == TAG_FRMS);
	ByteVector edited(file.begin(), file.begin() + (frames.Offset - 24));
	ByteVector const header = Build_File({ { "FRMS", 0, raw, {}, {} } });
	edited.insert(edited.end(), header.begin() + 8, header.end());
	edited.insert(edited.end(), file.begin() + frames.Offset + std::size_t(frames.Stored), file.end());
	return(edited);
}


// The sections, which start as Minimal_Sections does, with a summary whose last_frame and seals
// match their records, so the file verifies.
std::vector<RawSection> Sealed_Sections(std::vector<RawSection> sections = Minimal_Sections())
{
	Message::RecordingInfo info;
	REQUIRE(Decode(sections[0].Body, info));
	SealChain chain(Seal_Start(info.SealVersion, info.Engine, info.DataVersion, sections[1].Body, sections[2].Body));
	Message::RecordingSummary summary;
	summary.LastFrame = 3;
	for (auto const & [frame, bytes] : Read_All(Build_File(sections)).Records) {
		Message::Frame record;
		REQUIRE(Decode(bytes, record));
		chain.Add(frame, record, summary.Seals);
	}
	chain.Finish(3, summary.Seals);
	sections[4].Body = Encoded(summary);
	return(sections);
}


ByteVector Checkpoint_Body(std::uint32_t frame)
{
	Message::Checkpoint checkpoint;
	checkpoint.Frame = frame;
	checkpoint.State = Bytes_Of("state");
	checkpoint.Crc32 = CRC::Memory(checkpoint.State.data(), unsigned(checkpoint.State.size()));
	return(Encoded(checkpoint));
}

}


TEST_CASE("the reader reads back everything the writer wrote")
{
	ByteVector const file = Recording(1000);
	InputFile source(file);
	Reader reader(source);
	REQUIRE(reader.Open() == ReadResult::OK);
	CHECK(reader.Complete());

	Message::RecordingInfo info;
	REQUIRE(reader.Read_Info(info) == ReadResult::OK);
	Message::RecordingInfo expected_info = Sample_Info();
	expected_info.SealVersion = SEAL_VERSION;
	CHECK(Encoded(info) == Encoded(expected_info));

	Message::Launch launch;
	REQUIRE(reader.Read_Launch(launch) == ReadResult::OK);
	CHECK(Encoded(launch) == Encoded(Sample_Launch()));

	SectionInfo const * map = reader.Find(TAG_MAP);
	REQUIRE(map != nullptr);
	Replay::Bytes map_body;
	REQUIRE(reader.Read_Body(*map, map_body) == ReadResult::OK);
	CHECK(map_body == Bytes_Of("[Basic]\nName=Synthetic Map\n"));

	ReadBack const back = Read_All(file);
	CHECK(back.Last == FrameStep::END);
	REQUIRE(back.Records.size() == 1001);
	for (std::uint32_t frame = 0; frame <= 1000; frame++) {
		Message::Frame expected = Game_Record(frame);
		Message::Frame actual;
		REQUIRE(Decode(back.Records[frame].second, actual));
		CHECK(back.Records[frame].first == frame);
		expected.Seals = actual.Seals;
		CHECK(Encoded(actual) == Encoded(expected));
	}

	auto const checkpoints = reader.Checkpoints();
	REQUIRE(checkpoints.size() == 2);
	std::uint32_t frame = 99;
	REQUIRE(reader.Read_Checkpoint_Frame(*checkpoints[0], frame) == ReadResult::OK);
	CHECK_MESSAGE(frame == 0, "a checkpoint at frame 0 has no frame field");
	REQUIRE(reader.Read_Checkpoint_Frame(*checkpoints[1], frame) == ReadResult::OK);
	CHECK(frame == 100);
	Message::Checkpoint checkpoint;
	REQUIRE(reader.Read_Checkpoint(*checkpoints[1], checkpoint) == ReadResult::OK);
	CHECK(checkpoint.State == Bytes_Of("checkpoint state"));

	Message::Statistics statistics;
	CHECK(reader.Read_Statistics(statistics) == ReadResult::OK);
	CHECK(statistics.Houses.size() == 2);

	Message::RecordingSummary summary;
	REQUIRE(reader.Read_Summary(summary) == ReadResult::OK);
	CHECK(summary.LastFrame == 1000u);
	CHECK(summary.Seals.size() == 2);

	SealReport const report = Verify_Seals(reader);
	CHECK(report.Status == SealStatus::VERIFIED);
	CHECK(report.Compared == 3);
}


TEST_CASE("every synthetic file reads and verifies")
{
	for (SyntheticFile const & file : Synthetic_Files()) {
		InputFile source(file.Bytes);
		Reader reader(source);
		REQUIRE_MESSAGE(reader.Open() == ReadResult::OK, file.Name);
		SealReport const report = Verify_Seals(reader);
		CHECK_MESSAGE(report.Status == SealStatus::VERIFIED, file.Name);
		ReadBack const back = Read_All(file.Bytes);
		CHECK_MESSAGE(back.Last == (reader.Complete() ? FrameStep::END : FrameStep::CUT), file.Name);
		if (file.Name == "unclosed.cncr") {
			CHECK(!reader.Complete());
			CHECK_MESSAGE(back.Records.size() == 120, "the records up to the last flush");
		}
	}
}


TEST_CASE("a file cut at every byte reads as a prefix and never as an error")
{
	ByteVector const file = Recording(150);
	ReadBack const full = Read_All(file);
	REQUIRE(full.Records.size() == 151);
	auto const sections = Walk_Sections(file);
	REQUIRE(Section_Tags(sections) == "INFO,LNCH,MAP ,INIT,FRMS,CKPT,CKPT,STAT,END ");
	std::size_t const init_end = sections[3].Offset + std::size_t(sections[3].Stored);
	std::size_t const frames_start = sections[4].Offset;
	std::size_t const frames_end = frames_start + std::size_t(sections[4].Stored);

	std::size_t previous = 0;
	for (std::size_t cut = 0; cut <= file.size(); cut++) {
		ByteVector const prefix(file.begin(), file.begin() + cut);
		ReadBack const back = Read_All(prefix);
		if (cut < 8) {
			CHECK(back.Opened == ReadResult::NOT_REPLAY);
			continue;
		}
		if (cut < init_end) {
			CHECK_MESSAGE(back.Opened == ReadResult::MALFORMED, "without its first sections at ", cut);
			continue;
		}
		REQUIRE_MESSAGE(back.Opened == ReadResult::OK, "cut at ", cut);
		CHECK_MESSAGE(back.Last != FrameStep::FAILED, "cut at ", cut);
		CHECK_MESSAGE(back.Records.size() >= previous, "cut at ", cut);
		REQUIRE(back.Records.size() <= full.Records.size());
		for (std::size_t i = 0; i < back.Records.size(); i++) {
			CHECK(back.Records[i] == full.Records[i]);
		}
		if (cut >= frames_end) {
			CHECK_MESSAGE(back.Records.size() == full.Records.size(), "every record survives a cut after the stream, at ", cut);
		}
		previous = back.Records.size();

		InputFile source(prefix);
		Reader reader(source);
		REQUIRE(reader.Open() == ReadResult::OK);
		CHECK_MESSAGE(Verify_Seals(reader).Status == SealStatus::VERIFIED, "cut at ", cut);
	}
}


TEST_CASE("an unclosed recording holds the records up to its last flush")
{
	MemoryFile sink;
	Writer writer(sink);
	ByteVector const init = Sample_Init();
	REQUIRE(writer.Begin(Sample_Info(), Sample_Launch(), init, std::nullopt) == WriteResult::OK);
	for (std::uint32_t frame = 0; frame < 200; frame++) {
		REQUIRE(writer.Add_Frame(frame, Game_Record(frame)) == WriteResult::OK);
		ReadBack const back = Read_All(sink.Flushed());
		CHECK(back.Last == FrameStep::CUT);
		CHECK_MESSAGE(back.Records.size() == (frame + 1) / FLUSH_RECORDS * FLUSH_RECORDS, "after frame ", frame);
	}
}


TEST_CASE("every file a failed write, seek, or flush leaves reads without an error")
{
	auto record = [](FileClass & sink) {
		Writer writer(sink);
		ByteVector const init = Sample_Init();
		WriteResult result = writer.Begin(Sample_Info(), Sample_Launch(), init, std::nullopt);
		for (std::uint32_t frame = 0; frame < 200 && result == WriteResult::OK; frame++) {
			result = writer.Add_Frame(frame, Game_Record(frame));
		}
		Message::Checkpoint checkpoint;
		checkpoint.State = Bytes_Of("state");
		checkpoint.Crc32 = CRC::Memory(checkpoint.State.data(), unsigned(checkpoint.State.size()));
		if (result == WriteResult::OK) {
			result = writer.Add_Checkpoint(checkpoint);
		}
		Message::RecordingSummary summary;
		summary.LastFrame = 199;
		Message::Statistics statistics;
		if (result == WriteResult::OK) {
			result = writer.Close(summary, &statistics);
		}
		return(result);
	};

	FailingFile counter(1000000, 0);
	REQUIRE(record(counter) == WriteResult::OK);

	for (int fail_at = 1; fail_at <= counter.Calls; fail_at++) {
		for (std::size_t partial : { std::size_t(0), std::size_t(5) }) {
			FailingFile sink(fail_at, partial);
			REQUIRE(record(sink) == WriteResult::FAILED);
			ReadBack const back = Read_All(sink.Written());
			if (back.Opened != ReadResult::OK) {
				CHECK_MESSAGE((back.Opened == ReadResult::MALFORMED || back.Opened == ReadResult::NOT_REPLAY),
					"failing call ", fail_at);
				continue;
			}
			CHECK_MESSAGE(back.Last != FrameStep::FAILED, "failing call ", fail_at);
			CHECK_MESSAGE(Read_All(sink.Flushed()).Records.size() <= back.Records.size(), "failing call ", fail_at);

			InputFile source(sink.Written());
			Reader reader(source);
			REQUIRE(reader.Open() == ReadResult::OK);
			CHECK_MESSAGE(Verify_Seals(reader).Status == SealStatus::VERIFIED, "failing call ", fail_at);
		}
	}
}


TEST_CASE("each limit holds at its value and refuses one more")
{
	ByteVector const file = Recording(30);
	auto const sections = Walk_Sections(file);
	ReadBack const full = Read_All(file);
	REQUIRE(full.Last == FrameStep::END);

	LimitsType limits;
	limits.SmallSection = std::max({ sections[0].Raw, sections[1].Raw, sections[8].Raw });
	CHECK(Open_Result(file, limits) == ReadResult::OK);
	limits.SmallSection--;
	CHECK(Open_Result(file, limits) == ReadResult::LIMIT);

	limits = LimitsType();
	limits.LargeSection = sections[2].Raw - 1;
	CHECK(Open_Result(file, limits) == ReadResult::LIMIT);

	limits = LimitsType();
	limits.Checkpoint = sections[5].Raw - 1;
	CHECK(Open_Result(file, limits) == ReadResult::LIMIT);

	std::size_t largest = 0;
	for (auto const & [frame, record] : full.Records) {
		largest = std::max(largest, record.size());
	}
	limits = LimitsType();
	limits.FrameRecord = largest;
	CHECK(Read_All(file, limits).Last == FrameStep::END);
	limits.FrameRecord = largest - 1;
	ReadBack const limited = Read_All(file, limits);
	CHECK(limited.Last == FrameStep::FAILED);
	CHECK(limited.FrameError == ReadResult::LIMIT);

	limits = LimitsType();
	limits.EventsPerRecord = 0;
	CHECK(Read_All(file, limits).FrameError == ReadResult::LIMIT);

	InputFile source(file);
	limits = LimitsType();
	limits.Players = 1;
	Reader few_players(source, limits);
	REQUIRE(few_players.Open() == ReadResult::OK);
	Message::Launch launch;
	CHECK(few_players.Read_Launch(launch) == ReadResult::LIMIT);

	limits = LimitsType();
	limits.String = 5;
	Reader short_strings(source, limits);
	REQUIRE(short_strings.Open() == ReadResult::OK);
	Message::RecordingInfo info;
	CHECK(short_strings.Read_Info(info) == ReadResult::LIMIT);
}


TEST_CASE("a forged raw size past a limit is refused before anything is read")
{
	auto sections = Minimal_Sections();
	sections[2].Codec = 1;
	sections[2].Raw = 1ull << 40;
	CHECK(Open_Result(Build_File(sections)) == ReadResult::LIMIT);

	sections = Minimal_Sections();
	sections[0].Codec = 1;
	sections[0].Raw = (1u << 20) + 1;
	CHECK(Open_Result(Build_File(sections)) == ReadResult::LIMIT);
}


TEST_CASE("malformed containers are refused with the reason")
{
	CHECK(Open_Result(Build_File(Minimal_Sections())) == ReadResult::OK);
	CHECK(Read_All(Build_File(Minimal_Sections())).Records.size() == 2);
	CHECK(Read_All(Build_File(Minimal_Sections())).Records[1].first == 3);

	ByteVector wrong_magic = Build_File(Minimal_Sections());
	wrong_magic[0] = 'X';
	CHECK(Open_Result(wrong_magic) == ReadResult::NOT_REPLAY);
	CHECK(Open_Result(Build_File(Minimal_Sections(), 2)) == ReadResult::VERSION);

	auto sections = Minimal_Sections();
	sections[2].Stored = OPEN_SIZE;
	sections[2].Raw = OPEN_SIZE;
	CHECK_MESSAGE(Open_Result(Build_File(sections)) == ReadResult::MALFORMED, "only FRMS may be open");

	sections = Minimal_Sections();
	sections[2].Raw = 3;
	CHECK_MESSAGE(Open_Result(Build_File(sections)) == ReadResult::MALFORMED, "a stored section's sizes differ");

	sections = Minimal_Sections();
	sections[3].Raw = OPEN_SIZE;
	CHECK_MESSAGE(Open_Result(Build_File(sections)) == ReadResult::MALFORMED, "a real stored size with an open raw size");

	sections = Minimal_Sections();
	sections.erase(sections.begin() + 2);
	CHECK_MESSAGE(Open_Result(Build_File(sections)) == ReadResult::MALFORMED, "no INIT");

	sections = Minimal_Sections();
	std::swap(sections[0], sections[1]);
	CHECK_MESSAGE(Open_Result(Build_File(sections)) == ReadResult::MALFORMED, "LNCH first");

	sections = Minimal_Sections();
	sections.push_back({ "STAT", 0, ByteVector(), {}, {} });
	CHECK_MESSAGE(Open_Result(Build_File(sections)) == ReadResult::MALFORMED, "a section after END");

	sections = Minimal_Sections();
	sections.insert(sections.begin() + 3, { "INIT", 0, Sample_Init(), {}, {} });
	CHECK_MESSAGE(Open_Result(Build_File(sections)) == ReadResult::MALFORMED, "two INIT sections");

	sections = Minimal_Sections();
	std::swap(sections[2], sections[3]);
	CHECK_MESSAGE(Open_Result(Build_File(sections)) == ReadResult::MALFORMED, "INIT after FRMS");

	sections = Minimal_Sections();
	sections.erase(sections.begin() + 3);
	CHECK_MESSAGE(Open_Result(Build_File(sections)) == ReadResult::MALFORMED, "END without FRMS");

	sections = Minimal_Sections();
	sections[3].Body = Closed_Frames(false);
	ReadBack const unterminated = Read_All(Build_File(sections));
	CHECK_MESSAGE(unterminated.FrameError == ReadResult::MALFORMED, "a closed stream without its terminator");

	sections = Minimal_Sections();
	sections[3].Body.pop_back();
	sections[3].Body.pop_back();
	CHECK(Read_All(Build_File(sections)).FrameError == ReadResult::MALFORMED);

	sections = Minimal_Sections();
	ByteVector const too_long = From_Hex("ffffffffffffffffffff01");
	sections[3].Body.insert(sections[3].Body.begin(), too_long.begin(), too_long.end());
	CHECK(Read_All(Build_File(sections)).FrameError == ReadResult::MALFORMED);
}


TEST_CASE("unknown tags and codecs are skipped by their stored size")
{
	auto sections = Minimal_Sections();
	sections.insert(sections.begin() + 2, { "opts", 0, Bytes_Of("engine data"), {}, {} });
	sections.insert(sections.begin() + 3, { "XTRA", 7, Bytes_Of("future codec"), {}, std::uint64_t(99) });
	ByteVector const file = Build_File(sections);

	InputFile source(file);
	Reader reader(source);
	REQUIRE(reader.Open() == ReadResult::OK);
	CHECK(reader.Sections().size() == 7);
	CHECK(!reader.Unknown_Codec());
	REQUIRE(reader.Find(TAG_INIT) != nullptr);
	CHECK(reader.Find(TAG_INIT)->Codec == 0);
	CHECK(Read_All(file).Records.size() == 2);

	sections = Minimal_Sections();
	sections.push_back({ "opts", 0, Bytes_Of("late"), {}, {} });
	CHECK_MESSAGE(Open_Result(Build_File(sections)) == ReadResult::MALFORMED, "engine sections sit before END");
}


TEST_CASE("an unknown codec on INIT or FRMS leaves the file not verified")
{
	auto closed = [](std::uint32_t last_frame) {
		auto sections = Minimal_Sections();
		Message::RecordingSummary summary;
		summary.LastFrame = last_frame;
		sections[4].Body = Encoded(summary);
		return(sections);
	};

	auto sections = closed(3);
	sections[2].Codec = 9;
	ByteVector file = Build_File(sections);
	{
		InputFile source(file);
		Reader reader(source);
		REQUIRE(reader.Open() == ReadResult::OK);
		CHECK(reader.Unknown_Codec());
		CHECK(reader.Find(TAG_INIT) == nullptr);
	}
	CHECK_MESSAGE(Read_All(file).Records.size() == 2, "the frame stream is still read");
	CHECK(Seal_Report(file).Status == SealStatus::NOT_CHECKED);

	sections = closed(2);
	sections[2].Codec = 9;
	CHECK_MESSAGE(Seal_Report(Build_File(sections)).Status == SealStatus::MALFORMED, "malformed outranks not verified");

	sections = closed(3);
	sections[3].Codec = 9;
	file = Build_File(sections);
	CHECK(Read_All(file).Records.empty());
	CHECK(Seal_Report(file).Status == SealStatus::NOT_CHECKED);

	sections = closed(3);
	sections.insert(sections.begin() + 3, { "INIT", 9, Bytes_Of("second init"), {}, {} });
	CHECK_MESSAGE(Open_Result(Build_File(sections)) == ReadResult::MALFORMED, "an INIT of unknown codec still counts");

	sections = closed(3);
	sections[4].Codec = 9;
	CHECK_MESSAGE(Open_Result(Build_File(sections)) == ReadResult::MALFORMED, "END must use codec 0 or 1");
}


TEST_CASE("defeated houses out of ascending order make the frame stream malformed")
{
	auto with_defeated = [](std::vector<std::uint32_t> const & defeated) {
		Message::Frame record;
		record.Hash = 1;
		record.Defeated = defeated;
		Replay::Bytes const encoded = Encoded(record);
		ByteVector raw;
		Put_Varint(raw, encoded.size());
		raw.insert(raw.end(), encoded.begin(), encoded.end());
		raw.push_back(0);

		auto sections = Minimal_Sections();
		sections[3].Body = raw;
		Message::RecordingSummary summary;
		summary.LastFrame = 0;
		sections[4].Body = Encoded(summary);
		return(Read_All(Build_File(sections)));
	};

	ReadBack const ordered = with_defeated({ 1, 2 });
	CHECK(ordered.Last == FrameStep::END);
	CHECK(ordered.Records.size() == 1);

	ReadBack const reversed = with_defeated({ 2, 1 });
	CHECK(reversed.Last == FrameStep::FAILED);
	CHECK(reversed.FrameError == ReadResult::MALFORMED);
	CHECK_MESSAGE(with_defeated({ 1, 1 }).Last == FrameStep::FAILED, "a house listed twice");
}


TEST_CASE("INFO lists each income kind once, and a record whose income uses another kind is malformed")
{
	auto kind = [](std::uint32_t number, std::string const & id) {
		Message::IncomeKind entry;
		entry.Number = number;
		entry.Id = id;
		return(entry);
	};
	auto paid = [](std::uint32_t number) {
		Message::Income income;
		income.Kind = number;
		income.Amount = 10;
		return(income);
	};
	auto build = [](std::vector<Message::IncomeKind> const & kinds, std::vector<Message::Frame> const & records, bool closed) {
		ByteVector raw;
		for (Message::Frame const & record : records) {
			Replay::Bytes const encoded = Encoded(record);
			Put_Varint(raw, encoded.size());
			raw.insert(raw.end(), encoded.begin(), encoded.end());
		}
		auto sections = Minimal_Sections();
		Message::RecordingInfo info = Sample_Info();
		info.SealVersion = SEAL_VERSION;
		info.IncomeKinds = kinds;
		sections[0].Body = Encoded(info);
		if (closed) {
			raw.push_back(0);
			sections[3].Body = raw;
			Message::RecordingSummary summary;
			summary.LastFrame = std::uint32_t(records.size() - 1);
			sections[4].Body = Encoded(summary);
		} else {
			sections[3].Body = raw;
			sections[3].Stored = OPEN_SIZE;
			sections[3].Raw = OPEN_SIZE;
			sections.pop_back();
		}
		return(Build_File(sections));
	};
	auto info_result = [](ByteVector const & file, LimitsType const & limits = LimitsType()) {
		InputFile source(file);
		Reader reader(source, limits);
		REQUIRE(reader.Open() == ReadResult::OK);
		Message::RecordingInfo info;
		return(reader.Read_Info(info));
	};

	std::vector<Message::IncomeKind> const listed = { kind(1, "Crate"), kind(2, "Trigger") };
	std::vector<Message::Frame> records(3);
	records[0].Income = { paid(1) };
	records[1].Hash = 2;
	records[2].Income = { paid(2), paid(1) };
	ByteVector const good = build(listed, records, true);
	CHECK(info_result(good) == ReadResult::OK);
	ReadBack const good_back = Read_All(good);
	CHECK(good_back.Last == FrameStep::END);
	CHECK(good_back.Records.size() == 3);
	CHECK_MESSAGE(Read_All(build({ kind(1, "Crate"), kind(2, "Trigger"), kind(3, "Sold") }, records, true)).Last == FrameStep::END, "a listed kind no entry uses");

	std::vector<Message::Frame> unlisted = records;
	unlisted[1].Income = { paid(3) };
	ReadBack const unlisted_back = Read_All(build(listed, unlisted, true));
	CHECK_MESSAGE(unlisted_back.Last == FrameStep::FAILED, "a kind INFO does not list");
	CHECK(unlisted_back.FrameError == ReadResult::MALFORMED);
	CHECK(unlisted_back.Records.size() == 1);
	ReadBack const unlisted_open = Read_All(build(listed, unlisted, false));
	CHECK_MESSAGE(unlisted_open.Last == FrameStep::FAILED, "an open stream is judged up to its cut");
	CHECK(unlisted_open.Records.size() == 1);
	CHECK_MESSAGE(Read_All(build({}, records, true)).FrameError == ReadResult::MALFORMED, "income in a file that lists no kinds");

	auto check_broken = [&](std::vector<Message::IncomeKind> const & kinds, char const * why) {
		ByteVector const file = build(kinds, records, true);
		CHECK_MESSAGE(info_result(file) == ReadResult::MALFORMED, why);
		ReadBack const back = Read_All(file);
		CHECK_MESSAGE(back.FrameError == ReadResult::MALFORMED, why, ": the frame stream reader reads INFO first");
		CHECK(back.Records.empty());
	};
	check_broken({ kind(1, "Crate"), kind(1, "Trigger") }, "a number listed twice");
	check_broken({ kind(1, "Crate"), kind(2, "Crate") }, "an id listed twice");
	check_broken({ kind(1, "Crate"), kind(2, "") }, "an empty id");

	LimitsType limits;
	limits.IncomeKinds = 2;
	CHECK(info_result(good, limits) == ReadResult::OK);
	CHECK(Read_All(good, limits).Last == FrameStep::END);
	limits.IncomeKinds = 1;
	CHECK(info_result(good, limits) == ReadResult::LIMIT);
	CHECK(Read_All(good, limits).FrameError == ReadResult::LIMIT);
}


TEST_CASE("broken deflate data and wrong raw sizes are malformed")
{
	auto sections = Minimal_Sections();
	ByteVector packed;
	REQUIRE(Deflate_All(Sample_Init(), packed));
	sections[2] = { "INIT", 1, packed, {}, std::uint64_t(Sample_Init().size() + 1) };
	ByteVector file = Build_File(sections);
	InputFile source(file);
	Reader reader(source);
	REQUIRE(reader.Open() == ReadResult::OK);
	Replay::Bytes body;
	CHECK(reader.Read_Body(*reader.Find(TAG_INIT), body) == ReadResult::MALFORMED);

	sections[2] = { "INIT", 1, From_Hex("07"), {}, std::uint64_t(6) };
	file = Build_File(sections);
	InputFile broken_source(file);
	Reader broken(broken_source);
	REQUIRE(broken.Open() == ReadResult::OK);
	CHECK(broken.Read_Body(*broken.Find(TAG_INIT), body) == ReadResult::MALFORMED);
	CHECK(Verify_Seals(broken).Status == SealStatus::MALFORMED);

	sections = Minimal_Sections();
	Message::RecordingInfo info = Sample_Info();
	info.SealVersion = SEAL_VERSION + 1;
	sections[0].Body = Encoded(info);
	Message::RecordingSummary summary;
	summary.LastFrame = 3;
	sections[4].Body = Encoded(summary);
	file = Build_File(sections);
	InputFile later_source(file);
	Reader later(later_source);
	REQUIRE(later.Open() == ReadResult::OK);
	CHECK_MESSAGE(Verify_Seals(later).Status == SealStatus::NOT_CHECKED, "an unknown seal version is not compared");
}


TEST_CASE("a record after last_frame is malformed, whatever the seal version")
{
	auto sections = Minimal_Sections();
	Message::RecordingSummary summary;
	summary.LastFrame = 2;
	sections[4].Body = Encoded(summary);
	SealReport report = Seal_Report(Build_File(sections));
	CHECK(report.Status == SealStatus::MALFORMED);
	CHECK(report.Error == ReadResult::MALFORMED);
	CHECK_MESSAGE(report.LateRecord == 3u, "the record on frame 3 seals nothing and is still caught");

	Message::RecordingInfo info = Sample_Info();
	info.SealVersion = SEAL_VERSION + 1;
	sections[0].Body = Encoded(info);
	CHECK_MESSAGE(Seal_Report(Build_File(sections)).Status == SealStatus::MALFORMED, "malformed outranks not verified");

	sections[4].Body = ByteVector();
	CHECK_MESSAGE(Seal_Report(Build_File(sections)).LateRecord == 3u, "no last_frame means that no frame ran");
}


TEST_CASE("an interval seal missing from its record or on another record is malformed")
{
	ByteVector const file = Recording(1000);
	REQUIRE(Seal_Report(file).Status == SealStatus::VERIFIED);

	SealReport const missing = Seal_Report(Rewrite_Frames(file, [](std::uint32_t frame, Message::Frame & record) {
		if (frame == 900) {
			record.Seals.clear();
		}
	}));
	CHECK(missing.Status == SealStatus::MALFORMED);
	CHECK(missing.FirstMisplaced == 899u);
	CHECK_MESSAGE(!missing.FirstMismatch.has_value(), "the summary's seals are still right");

	std::vector<Message::Seal> moved;
	SealReport const later = Seal_Report(Rewrite_Frames(file, [&](std::uint32_t frame, Message::Frame & record) {
		if (frame == 900) {
			moved = record.Seals;
			record.Seals.clear();
		} else if (frame == 901) {
			record.Seals = moved;
		}
	}));
	CHECK(later.Status == SealStatus::MALFORMED);
	CHECK(later.FirstMisplaced == 899u);
	CHECK_MESSAGE(!later.FirstMismatch.has_value(), "the moved seal holds the right value");
}


TEST_CASE("checkpoints must ascend with no two on one frame")
{
	auto with_bodies = [](std::vector<ByteVector> const & bodies) {
		auto sections = Sealed_Sections();
		for (ByteVector const & body : bodies) {
			sections.insert(sections.end() - 1, { "CKPT", 0, body, {}, {} });
		}
		ByteVector const file = Build_File(sections);
		InputFile source(file);
		Reader reader(source);
		REQUIRE_MESSAGE(reader.Open() == ReadResult::OK, "Open does not read checkpoint bodies");
		CHECK(reader.Check_Checkpoints() == Verify_Seals(reader).Error);
		return(Verify_Seals(reader).Status);
	};
	auto with_checkpoints = [&](std::vector<std::uint32_t> const & frames) {
		std::vector<ByteVector> bodies;
		for (std::uint32_t frame : frames) {
			bodies.push_back(Checkpoint_Body(frame));
		}
		return(with_bodies(bodies));
	};

	CHECK(with_checkpoints({}) == SealStatus::VERIFIED);
	CHECK(with_checkpoints({ 0, 3 }) == SealStatus::VERIFIED);
	CHECK(with_checkpoints({ 3, 3 }) == SealStatus::MALFORMED);
	CHECK(with_checkpoints({ 0, 0 }) == SealStatus::MALFORMED);
	CHECK(with_checkpoints({ 3, 0 }) == SealStatus::MALFORMED);

	// The last frame field counts, wherever it sits.
	CHECK_MESSAGE(with_bodies({ From_Hex("0801 0802"), From_Hex("0802") }) == SealStatus::MALFORMED, "frames 2 and 2");
	CHECK_MESSAGE(with_bodies({ From_Hex("1500000000 0801"), From_Hex("1500000000 0802") }) == SealStatus::VERIFIED, "frames 1 and 2");
	CHECK_MESSAGE(with_bodies({ From_Hex("0802"), From_Hex("0881808080 10") }) == SealStatus::MALFORMED, "2^32 + 1 narrows to 1");

	ByteVector checkpoint = Checkpoint_Body(5);
	checkpoint.back() ^= 1;
	CHECK_MESSAGE(with_bodies({ checkpoint }) == SealStatus::MALFORMED, "a state that does not match its crc32");

	auto sections = Sealed_Sections();
	sections.insert(sections.end() - 1, { "CKPT", 0, From_Hex("0881808080 10"), {}, {} });
	ByteVector const file = Build_File(sections);
	InputFile source(file);
	Reader reader(source);
	REQUIRE(reader.Open() == ReadResult::OK);
	std::uint32_t frame = 0;
	CHECK(reader.Read_Checkpoint_Frame(*reader.Checkpoints()[0], frame) == ReadResult::OK);
	CHECK_MESSAGE(frame == 1, "a frame wider than 32 bits keeps its low bits");
}


TEST_CASE("an edited record is found by the seal check")
{
	ByteVector const file = Recording(1000);
	ReadBack const back = Read_All(file);

	// The frame stream is stored again with one payload byte of frame 500 changed and every
	// carried seal kept as it was.
	ByteVector raw;
	for (auto const & [frame, bytes] : back.Records) {
		Message::Frame record;
		REQUIRE(Decode(bytes, record));
		if (frame == 500) {
			REQUIRE(!record.Events.empty());
			record.Events[0].Payload[1] ^= 0xFF;
		}
		Replay::Bytes const encoded = Encoded(record);
		Put_Varint(raw, encoded.size());
		raw.insert(raw.end(), encoded.begin(), encoded.end());
	}
	raw.push_back(0);

	auto const sections = Walk_Sections(file);
	SectionEntry const & frames = sections[4];
	REQUIRE(frames.Tag == TAG_FRMS);
	ByteVector edited(file.begin(), file.begin() + (frames.Offset - 24));
	ByteVector const header = Build_File({ { "FRMS", 0, raw, {}, {} } });
	edited.insert(edited.end(), header.begin() + 8, header.end());
	edited.insert(edited.end(), file.begin() + frames.Offset + std::size_t(frames.Stored), file.end());

	InputFile source(edited);
	Reader reader(source);
	REQUIRE(reader.Open() == ReadResult::OK);
	SealReport const report = Verify_Seals(reader);
	CHECK(report.Status == SealStatus::MISMATCH);
	CHECK(report.FirstMismatch == 899u);

	InputFile again(edited);
	Reader whole(again);
	FileReport const result = Verify_File(whole);
	CHECK(result.Status == FileStatus::MISMATCH);
	CHECK((result.Seals.has_value() && result.Seals->FirstMismatch == 899u));
}


namespace {

FileStatus File_Status(ByteVector const & file)
{
	InputFile source(file);
	Reader reader(source);
	return(Verify_File(reader).Status);
}

}


TEST_CASE("Verify_File applies the rules of the records that Verify_Seals does not decode")
{
	CHECK(File_Status(Build_File(Sealed_Sections())) == FileStatus::COMPLETE);
	CHECK(File_Status(Recording(200)) == FileStatus::COMPLETE);
	CHECK(File_Status(Recording(200, false)) == FileStatus::INCOMPLETE);
	CHECK(File_Status(Bytes_Of("not a replay file")) == FileStatus::NOT_REPLAY);

	// A lead byte followed by a byte that cannot continue it, and a string one byte past the limit.
	std::string const invalid = "\xC3\x28";
	std::string const too_long(1025, 'x');
	std::pair<std::string, FileStatus> const cases[] = {
		{ invalid, FileStatus::MALFORMED },
		{ too_long, FileStatus::LIMIT },
	};

	for (auto const & [text, expected] : cases) {
		CAPTURE(text.size());

		Message::Launch launch = Sample_Launch();
		launch.Players[1].CountryName = text;
		auto sections = Minimal_Sections();
		sections[1].Body = Encoded(launch);
		ByteVector const in_launch = Build_File(Sealed_Sections(sections));
		InputFile source(in_launch);
		Reader reader(source);
		REQUIRE(reader.Open() == ReadResult::OK);
		CHECK_MESSAGE(Verify_Seals(reader).Status == SealStatus::VERIFIED, "the seals read LNCH only as bytes");
		CHECK(File_Status(in_launch) == expected);

		Message::Statistics country;
		country.Houses.resize(1);
		country.Houses[0].CountryName = text;
		Message::Statistics crate;
		crate.Houses.resize(1);
		crate.Houses[0].Crates.resize(1);
		crate.Houses[0].Crates[0].Id = text;
		for (Message::Statistics const & statistics : { country, crate }) {
			auto with_statistics = Sealed_Sections();
			with_statistics.insert(with_statistics.begin() + 4, RawSection{ "STAT", 0, Encoded(statistics), {}, {} });
			ByteVector const in_statistics = Build_File(with_statistics);
			CHECK(Seal_Report(in_statistics).Status == SealStatus::VERIFIED);
			CHECK(File_Status(in_statistics) == expected);
		}
	}
}


TEST_CASE("the reader reads through RawFileClass what RawFileClass wrote")
{
	ByteVector const file = Recording(100);
	char const * name = "replay-reader-rawfile.tmp";
	std::remove(name);
	{
		RawFileClass output(name);
		REQUIRE(output.Open(FileClass::WRITE));
		REQUIRE(output.Write(file.data(), int(file.size())) == int(file.size()));
		output.Close();
	}

	RawFileClass input(name);
	REQUIRE(input.Open(FileClass::READ));
	CHECK(input.Size() == int(file.size()));
	Reader reader(input);
	REQUIRE(reader.Open() == ReadResult::OK);
	CHECK(Verify_Seals(reader).Status == SealStatus::VERIFIED);
	input.Close();
	std::remove(name);
}


namespace {

// Records the furthest byte read, to show where a reader stopped.
class CountingFile : public InputFile
{
	public:
		explicit CountingFile(ByteSpan data) : InputFile(data), Furthest(0) {}

		int Read(void * buffer, int size) override
		{
			int const start = Seek(0, SEEK_CUR);
			int const got = InputFile::Read(buffer, size);
			Furthest = std::max<std::uint64_t>(Furthest, std::uint64_t(start) + std::uint64_t(got));
			return(got);
		}

		std::uint64_t Furthest;
};


// Throws std::bad_alloc from every read once Fail is set.
class ThrowingFile : public InputFile
{
	public:
		explicit ThrowingFile(ByteSpan data) : InputFile(data), Fail(false) {}

		int Read(void * buffer, int size) override
		{
			if (Fail) {
				throw std::bad_alloc();
			}
			return(InputFile::Read(buffer, size));
		}

		bool Fail;
};


ByteVector Deflated(ByteVector const & data)
{
	ByteVector packed;
	REQUIRE(Deflate_All(data, packed));
	return(packed);
}


// A deflate stream of data followed by count MiB of zeros.
ByteVector Deflated_With_Zeros(ByteVector const & data, int count)
{
	DeflaterClass deflater;
	ByteVector packed;
	REQUIRE(deflater.Write(data, packed));
	ByteVector const zeros(1 << 20, 0);
	for (int i = 0; i < count; i++) {
		REQUIRE(deflater.Write(zeros, packed));
	}
	REQUIRE(deflater.Finish(packed));
	return(packed);
}


// The frame stream of a file without END, whose body is records for frames 0 and 3 that end
// in a block of the reserved deflate type 3.
ByteVector Broken_Tail(void)
{
	DeflaterClass deflater;
	ByteVector packed;
	REQUIRE(deflater.Write(Closed_Frames(false), packed));
	REQUIRE(deflater.Sync_Flush(packed));
	packed.push_back(0x07);
	return(packed);
}

}


TEST_CASE("a closed frame stream holds nothing after its terminator")
{
	auto sections = Minimal_Sections();
	sections[3].Body.push_back(0x01);
	ReadBack back = Read_All(Build_File(sections));
	CHECK(back.Records.size() == 2);
	CHECK(back.FrameError == ReadResult::MALFORMED);

	ByteVector trailing = Closed_Frames();
	trailing.push_back(0x01);
	sections = Minimal_Sections();
	sections[3] = { "FRMS", 1, Deflated(trailing), {}, std::uint64_t(trailing.size()) };
	CHECK_MESSAGE(Read_All(Build_File(sections)).FrameError == ReadResult::MALFORMED, "an inflated byte after the terminator");

	sections = Minimal_Sections();
	sections[3] = { "FRMS", 1, Deflated(Closed_Frames()), {}, std::uint64_t(Closed_Frames().size()) };
	CHECK(Read_All(Build_File(sections)).Last == FrameStep::END);
}


TEST_CASE("bytes after a closed stream's terminator fail without being read")
{
	auto stops_early = [](ByteVector const & body, std::uint64_t raw) {
		auto sections = Minimal_Sections();
		sections[3] = { "FRMS", 1, body, {}, raw };
		ByteVector const file = Build_File(sections);
		CountingFile source(file);
		Reader reader(source);
		REQUIRE(reader.Open() == ReadResult::OK);
		SectionInfo const frames_section = *reader.Find(TAG_FRMS);
		source.Furthest = 0;

		FrameReader frames(reader);
		std::uint32_t frame = 0;
		Message::Frame record;
		FrameStep step;
		while ((step = frames.Next(frame, record)) == FrameStep::RECORD) {
		}
		CHECK(step == FrameStep::FAILED);
		CHECK(frames.Error() == ReadResult::MALFORMED);
		CHECK_MESSAGE(source.Furthest < frames_section.Offset + frames_section.Stored / 2, "read up to ", source.Furthest);
	};

	// The zeros follow the terminator in the same inflated piece.
	stops_early(Deflated_With_Zeros(Closed_Frames(), 64), Closed_Frames().size() + (64ull << 20));

	// A stored block that ends exactly at the end of the first piece the reader inflates holds
	// one record and the terminator, so the zeros come only from later input.
	ByteVector stream;
	for (std::size_t padding = 4000; stream.size() + 5 != 4096; padding++) {
		Message::Frame padded;
		padded.Hash = 1;
		UnknownField field;
		field.Number = 100;
		field.Encoded = From_Hex("a206");
		Put_Varint(field.Encoded, padding);
		field.Encoded.insert(field.Encoded.end(), padding, 0x55);
		padded.Unknown.push_back(field);
		ByteVector const encoded = Encoded(padded);
		stream.clear();
		Put_Varint(stream, encoded.size());
		stream.insert(stream.end(), encoded.begin(), encoded.end());
		stream.push_back(0);
		REQUIRE(stream.size() <= 4091);
	}
	ByteVector body = { 0x00, std::uint8_t(stream.size()), std::uint8_t(stream.size() >> 8),
		std::uint8_t(~stream.size()), std::uint8_t(~stream.size() >> 8) };
	body.insert(body.end(), stream.begin(), stream.end());
	REQUIRE(body.size() == 4096);
	ByteVector const zeros = Deflated_With_Zeros(ByteVector(), 64);
	body.insert(body.end(), zeros.begin(), zeros.end());
	stops_early(body, stream.size() + (64ull << 20));
}


TEST_CASE("a closed section's deflate stream ends exactly at its stored size")
{
	auto sections = Minimal_Sections();
	ByteVector init = Deflated(Sample_Init());
	init.push_back(0x00);
	sections[2] = { "INIT", 1, init, {}, std::uint64_t(Sample_Init().size()) };
	ByteVector file = Build_File(sections);
	{
		InputFile source(file);
		Reader reader(source);
		REQUIRE(reader.Open() == ReadResult::OK);
		Replay::Bytes body;
		CHECK(reader.Read_Body(*reader.Find(TAG_INIT), body) == ReadResult::MALFORMED);
	}

	ByteVector frames = Deflated(Closed_Frames());
	frames.push_back(0x00);
	sections = Minimal_Sections();
	sections[3] = { "FRMS", 1, frames, {}, std::uint64_t(Closed_Frames().size()) };
	ReadBack const back = Read_All(Build_File(sections));
	CHECK(back.Records.size() == 2);
	CHECK(back.FrameError == ReadResult::MALFORMED);
}


TEST_CASE("broken deflate data ends an unfinished frame stream as the end of the input does")
{
	ByteVector const tail = Broken_Tail();
	auto sections = Minimal_Sections();
	sections.pop_back();
	sections[3] = { "FRMS", 1, tail, OPEN_SIZE, OPEN_SIZE };
	ByteVector file = Build_File(sections);
	ReadBack back = Read_All(file);
	CHECK(back.Records.size() == 2);
	CHECK(back.Last == FrameStep::CUT);
	CHECK(Seal_Report(file).Status == SealStatus::VERIFIED);

	sections[3] = { "FRMS", 1, tail, std::uint64_t(tail.size() + 100), std::uint64_t(1000) };
	back = Read_All(Build_File(sections));
	CHECK_MESSAGE(back.Last == FrameStep::CUT, "a closed stream whose stored size runs past the end of the file");
	CHECK(back.Records.size() == 2);

	sections = Minimal_Sections();
	sections[3] = { "FRMS", 1, tail, {}, std::uint64_t(1000) };
	back = Read_All(Build_File(sections));
	CHECK_MESSAGE(back.FrameError == ReadResult::MALFORMED, "a closed stream that lies whole in the file");
}


TEST_CASE("a skipped section still counts for the section table")
{
	auto with = [](std::size_t at, RawSection const & section) {
		auto sections = Minimal_Sections();
		sections.insert(sections.begin() + std::ptrdiff_t(at), section);
		return(Open_Result(Build_File(sections)));
	};

	CHECK(with(2, { "XTRA", 7, Bytes_Of("x"), {}, std::uint64_t(1) << 40 }) == ReadResult::OK);
	CHECK(with(5, { "XTRA", 0, ByteVector(), {}, {} }) == ReadResult::MALFORMED);
	CHECK(with(5, { "opts", 0, ByteVector(), {}, {} }) == ReadResult::MALFORMED);
	CHECK(with(5, { "CKPT", 7, ByteVector(), {}, {} }) == ReadResult::MALFORMED);
	CHECK(with(1, { "XTRA", 0, ByteVector(), {}, {} }) == ReadResult::MALFORMED);
	CHECK(with(1, { "opts", 0, ByteVector(), {}, {} }) == ReadResult::MALFORMED);
	CHECK(with(4, { "MAP ", 2, ByteVector(), {}, {} }) == ReadResult::MALFORMED);
	CHECK(with(2, { "XTRA", 0, ByteVector(), {}, std::uint64_t(1) }) == ReadResult::MALFORMED);

	auto sections = Minimal_Sections();
	sections.insert(sections.begin() + 2, { "MAP ", 0, ByteVector(), {}, {} });
	sections.insert(sections.begin() + 3, { "MAP ", 7, ByteVector(), {}, {} });
	CHECK_MESSAGE(Open_Result(Build_File(sections)) == ReadResult::MALFORMED, "a second MAP of unknown codec");

	sections = Minimal_Sections();
	sections[2] = { "INIT", 9, ByteVector(), {}, std::uint64_t((32u << 20) + 1) };
	CHECK_MESSAGE(Open_Result(Build_File(sections)) == ReadResult::LIMIT, "an INIT of unknown codec past its limit");
	sections[2].Raw = std::uint64_t(32u << 20);
	CHECK(Open_Result(Build_File(sections)) == ReadResult::OK);
}


TEST_CASE("the reader applies limits while it decodes")
{
	ByteVector bomb;
	for (int i = 0; i < (1 << 19); i++) {
		bomb.insert(bomb.end(), { 0x1a, 0x00 });
	}
	auto sections = Sealed_Sections();
	sections[1] = { "LNCH", 1, Deflated(bomb), {}, std::uint64_t(bomb.size()) };
	ByteVector file = Build_File(sections);
	InputFile source(file);
	Reader reader(source);
	REQUIRE(reader.Open() == ReadResult::OK);
	Message::Launch launch;
	CHECK(reader.Read_Launch(launch) == ReadResult::LIMIT);
	CHECK(launch.Players.size() == 64);

	ByteVector info = From_Hex("6801 0a8108");
	info.insert(info.end(), 1025, 'a');
	ByteVector const engine = From_Hex("0a06");
	info.insert(info.end(), engine.begin(), engine.end());
	ByteVector const name = Bytes_Of("opents");
	info.insert(info.end(), name.begin(), name.end());
	sections = Sealed_Sections();
	sections[0].Body = info;
	file = Build_File(sections);
	InputFile info_source(file);
	Reader info_reader(info_source);
	REQUIRE(info_reader.Open() == ReadResult::OK);
	Message::RecordingInfo decoded;
	CHECK(info_reader.Read_Info(decoded) == ReadResult::LIMIT);
	CHECK(Verify_Seals(info_reader).Error == ReadResult::LIMIT);
}


TEST_CASE("a record on frame 0xFFFFFFFF without its seals is malformed and quick to find")
{
	ByteVector record = From_Hex("08ffffffff0f 1501000000");
	ByteVector raw;
	Put_Varint(raw, record.size());
	raw.insert(raw.end(), record.begin(), record.end());
	raw.push_back(0);

	Message::RecordingSummary summary;
	summary.LastFrame = 0xFFFFFFFFu;
	auto sections = Minimal_Sections();
	sections[3].Body = raw;
	sections[4].Body = Encoded(summary);
	SealReport report = Seal_Report(Build_File(sections));
	CHECK(report.Status == SealStatus::MALFORMED);
	CHECK(report.FirstMisplaced == 899u);

	sections = Minimal_Sections();
	sections[3].Body = { 0x00 };
	sections[4].Body = Encoded(summary);
	report = Seal_Report(Build_File(sections));
	CHECK_MESSAGE(report.Status == SealStatus::MALFORMED, "a summary on frame 0xFFFFFFFF without its seals");
	CHECK(report.FirstMisplaced == 899u);
}


TEST_CASE("no call lets an allocation failure escape")
{
	ByteVector const file = Recording(150);
	ThrowingFile source(file);
	source.Fail = true;
	Reader failing(source);
	CHECK(failing.Open() == ReadResult::MEMORY);

	source.Fail = false;
	Reader reader(source);
	REQUIRE(reader.Open() == ReadResult::OK);
	REQUIRE(reader.Checkpoints().size() == 2);
	source.Fail = true;

	Message::RecordingInfo info;
	Message::Launch launch;
	Message::Statistics statistics;
	Message::RecordingSummary summary;
	Replay::Bytes body;
	Message::Checkpoint checkpoint;
	std::uint32_t frame = 0;
	CHECK(reader.Read_Info(info) == ReadResult::MEMORY);
	CHECK(reader.Read_Launch(launch) == ReadResult::MEMORY);
	CHECK(reader.Read_Statistics(statistics) == ReadResult::MEMORY);
	CHECK(reader.Read_Summary(summary) == ReadResult::MEMORY);
	CHECK(reader.Read_Body(*reader.Find(TAG_INIT), body) == ReadResult::MEMORY);
	CHECK(reader.Read_Checkpoint(*reader.Checkpoints()[0], checkpoint) == ReadResult::MEMORY);
	CHECK(reader.Read_Checkpoint_Frame(*reader.Checkpoints()[1], frame) == ReadResult::MEMORY);
	CHECK(reader.Check_Checkpoints() == ReadResult::MEMORY);

	FrameReader frames(reader);
	Message::Frame record;
	CHECK(frames.Next(frame, record) == FrameStep::FAILED);
	CHECK(frames.Error() == ReadResult::MEMORY);

	SealReport const report = Verify_Seals(reader);
	CHECK(report.Status == SealStatus::MALFORMED);
	CHECK(report.Error == ReadResult::MEMORY);
}


TEST_CASE("mutated files never crash the reader")
{
	ByteVector const original = Recording(120);
	std::mt19937 random(424242);
	int opened = 0;
	for (int i = 0; i < 3000; i++) {
		ByteVector file = original;
		int edits = 1 + int(random() % 4);
		for (int e = 0; e < edits; e++) {
			std::size_t at = random() % file.size();
			switch (random() % 5) {
				case 0: file[at] ^= std::uint8_t(1u << (random() % 8)); break;
				case 1: file.insert(file.begin() + std::ptrdiff_t(at), std::uint8_t(random())); break;
				case 2: file.erase(file.begin() + std::ptrdiff_t(at)); break;
				case 3: file[at] = 0xFF; break;
				default: file.resize(at + 1); break;
			}
			if (file.empty()) {
				file.push_back(0);
			}
		}

		InputFile source(file);
		Reader reader(source);
		if (reader.Open() != ReadResult::OK) {
			continue;
		}
		opened++;
		Message::RecordingInfo info;
		Message::Launch launch;
		Message::RecordingSummary summary;
		Message::Statistics statistics;
		reader.Read_Info(info);
		reader.Read_Launch(launch);
		reader.Read_Summary(summary);
		reader.Read_Statistics(statistics);
		for (SectionInfo const * checkpoint : reader.Checkpoints()) {
			std::uint32_t frame = 0;
			Message::Checkpoint body;
			reader.Read_Checkpoint_Frame(*checkpoint, frame);
			reader.Read_Checkpoint(*checkpoint, body);
		}
		Verify_Seals(reader);
	}
	CHECK_MESSAGE(opened > 100, "enough mutations still open to exercise the readers past the headers");
}
