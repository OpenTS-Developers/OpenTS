/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "replay/replaywriter.h"

#include "crc.h"
#include "fixtures.h"
#include "synthfiles.h"

#include "doctest/doctest.h"

#include <filesystem>
#include <fstream>

using namespace Replay;


namespace {

void Begin(Writer & writer, std::optional<ByteVector> const & map = std::nullopt)
{
	Message::RecordingInfo info = Sample_Info();
	Message::IncomeKind kind;
	kind.Number = 1;
	kind.Id = "Starting/Multiplier";
	info.IncomeKinds.push_back(kind);
	ByteVector const init = Sample_Init();
	std::optional<ByteSpan> map_span;
	if (map) {
		Digest const digest = SHA256Class::Digest(*map);
		info.MapSha256.assign(digest.begin(), digest.end());
		map_span = ByteSpan(*map);
	}
	REQUIRE(writer.Begin(info, Sample_Launch(), init, map_span) == WriteResult::OK);
}


Message::Frame Game_Frame(std::uint32_t frame)
{
	Message::Frame record;
	record.Hash = frame * 2654435761u;
	if (frame == 0) {
		record.FrameMicros = 16666;
		record.View.Camera = Message::Point{ 1200, -340, {} };
		for (std::uint32_t house = 0; house < 2; house++) {
			Message::HouseSample sample;
			sample.House = house;
			sample.Credits = 10000;
			sample.Spent = 0;
			sample.Buildings = 1;
			record.Samples.push_back(sample);
		}
		Message::Income income;
		income.House = 0;
		income.Kind = 1;
		income.Amount = 10000;
		record.Income.push_back(income);
	}
	if (frame % 50 == 7) {
		Message::Event order;
		order.Type = 3;
		order.House = frame % 2;
		order.Due = 2;
		order.Payload = { 0x01, std::uint8_t(frame), std::uint8_t(frame >> 8), 0x40 };
		record.Events.push_back(order);

		Message::Event heartbeat;
		heartbeat.Type = 40;
		heartbeat.House = 0;
		heartbeat.Payload = { 0xAA, 0xBB };
		heartbeat.Local = true;
		record.Events.push_back(heartbeat);
	}
	return(record);
}


void Record_Frames(Writer & writer, std::uint32_t first, std::uint32_t last)
{
	for (std::uint32_t frame = first; frame <= last; frame++) {
		REQUIRE(writer.Add_Frame(frame, Game_Frame(frame)) == WriteResult::OK);
	}
}


void Close(Writer & writer, std::uint32_t last_frame, Message::Statistics const * statistics = nullptr)
{
	Message::RecordingSummary summary;
	summary.LastFrame = last_frame;
	summary.EndedAt = 1794800600;
	summary.Completed = true;
	summary.GameMicros = std::uint64_t(last_frame + 1) * 16666;
	REQUIRE(writer.Close(summary, statistics) == WriteResult::OK);
}


Message::Statistics Sample_Statistics(void)
{
	Message::Statistics statistics;
	Message::ObjectType yard;
	yard.Category = ObjectCategory::OBJECT_CATEGORY_BUILDING;
	yard.Index = 0;
	yard.Id = "GACNST";
	yard.Name = "Construction Yard";
	yard.Cost = 2500;
	statistics.Types.push_back(yard);

	for (std::uint32_t house = 0; house < 2; house++) {
		Message::HouseStatistics entry;
		entry.House = house;
		entry.Name = (house == 0) ? "Player" : "Computer";
		entry.CountryName = (house == 0) ? "GDI" : "Nod";
		entry.Color = (house == 0) ? 0 : 3;
		entry.Start = (house == 0) ? 2 : 0;
		entry.Outcome = (house == 0) ? Outcome::OUTCOME_WON : Outcome::OUTCOME_LOST;
		entry.Final.House = house;
		entry.Final.Credits = 5000;
		Message::TypeCount built;
		built.Kind = CountKind::COUNT_KIND_BUILT;
		built.Category = ObjectCategory::OBJECT_CATEGORY_BUILDING;
		built.Count = 1;
		entry.Counts.push_back(built);
		statistics.Houses.push_back(entry);
	}
	return(statistics);
}


ByteVector Minimal(void)
{
	MemoryFile sink;
	Writer writer(sink);
	Begin(writer);
	REQUIRE(writer.Add_Frame(0, Game_Frame(0)) == WriteResult::OK);
	Close(writer, 0);
	return(sink.Written());
}


ByteVector Three_Intervals(void)
{
	MemoryFile sink;
	Writer writer(sink);
	Begin(writer);
	Record_Frames(writer, 0, 2400);
	Close(writer, 2400, nullptr);
	return(sink.Written());
}


ByteVector Interval_Boundary(void)
{
	MemoryFile sink;
	Writer writer(sink);
	Begin(writer);
	Record_Frames(writer, 0, 1799);
	Close(writer, 1799);
	return(sink.Written());
}


ByteVector Embedded_Map(void)
{
	MemoryFile sink;
	Writer writer(sink);
	Begin(writer, Bytes_Of("[Basic]\nName=Synthetic Map\n\n[Map]\nSize=0,0,64,64\n"));
	Record_Frames(writer, 0, 99);
	Close(writer, 99);
	return(sink.Written());
}


ByteVector Checkpoint_At_Zero(void)
{
	MemoryFile sink;
	Writer writer(sink);
	Begin(writer);
	Record_Frames(writer, 0, 999);
	for (std::uint32_t frame : { 0u, 900u }) {
		Message::Checkpoint checkpoint;
		checkpoint.Frame = frame;
		checkpoint.State = Bytes_Of("synthetic save data at frame " + std::to_string(frame));
		checkpoint.Crc32 = CRC::Memory(checkpoint.State.data(), unsigned(checkpoint.State.size()));
		REQUIRE(writer.Add_Checkpoint(checkpoint) == WriteResult::OK);
	}
	Message::Statistics const statistics = Sample_Statistics();
	Close(writer, 999, &statistics);
	return(sink.Written());
}


ByteVector Sealed_Payload(void)
{
	MemoryFile sink;
	Writer writer(sink);
	Begin(writer);
	Record_Frames(writer, 0, 9);
	Message::Frame record = Game_Frame(10);
	Message::Event group;
	group.Type = 3;
	group.House = 1;
	group.Payload = From_Hex("01020304aabbccdd");
	group.SealedPayload = From_Hex("0102030400000000");
	record.Events.push_back(group);
	REQUIRE(writer.Add_Frame(10, record) == WriteResult::OK);
	Record_Frames(writer, 11, 20);
	Close(writer, 20);
	return(sink.Written());
}


ByteVector Unclosed(void)
{
	MemoryFile sink;
	Writer writer(sink);
	Begin(writer);
	Record_Frames(writer, 0, 149);
	return(sink.Flushed());
}

}


std::vector<SyntheticFile> Synthetic_Files(void)
{
	std::vector<SyntheticFile> files;
	files.push_back({ "minimal.cncr", Minimal() });
	files.push_back({ "three-intervals.cncr", Three_Intervals() });
	files.push_back({ "interval-boundary.cncr", Interval_Boundary() });
	files.push_back({ "embedded-map.cncr", Embedded_Map() });
	files.push_back({ "checkpoint-frame0.cncr", Checkpoint_At_Zero() });
	files.push_back({ "sealed-payload.cncr", Sealed_Payload() });
	files.push_back({ "unclosed.cncr", Unclosed() });
	return(files);
}


TEST_CASE("synthetic replay files are written for other readers")
{
	std::filesystem::path const directory(REPLAY_OUT_DIR);
	std::error_code error;
	std::filesystem::create_directories(directory, error);
	REQUIRE_MESSAGE(!error, "cannot create ", directory.string());

	for (SyntheticFile const & file : Synthetic_Files()) {
		std::ofstream out(directory / file.Name, std::ios::binary | std::ios::trunc);
		out.write(reinterpret_cast<char const *>(file.Bytes.data()), std::streamsize(file.Bytes.size()));
		CHECK_MESSAGE(out.good(), "cannot write ", file.Name);
	}
}
