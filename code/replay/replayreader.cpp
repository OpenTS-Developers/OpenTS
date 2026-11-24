/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "replay/replayreader.h"

#include "crc.h"

#include <algorithm>
#include <climits>
#include <cstdio>
#include <exception>
#include <span>


namespace Replay
{

namespace
{

constexpr std::size_t READ_PIECE = 64 * 1024;
constexpr std::size_t FRAME_PIECE = 4 * 1024;
constexpr std::uint64_t NO_LIMIT = ~0ull;

// Enough of a checkpoint body to hold its first field: a one-byte tag and the longest varint.
constexpr std::size_t CHECKPOINT_PEEK = 11;


std::uint64_t Little_Endian(std::uint8_t const * bytes, int size)
{
	std::uint64_t value = 0;
	for (int i = 0; i < size; i++) {
		value |= std::uint64_t(bytes[i]) << (8 * i);
	}
	return(value);
}


bool Known_Codec(std::uint32_t codec)
{
	return(codec == std::uint32_t(Codec::STORED) || codec == std::uint32_t(Codec::DEFLATE));
}


// Runs body and reports any exception as MEMORY, so body must throw only when memory runs out.
template<typename Body>
ReadResult Guarded(Body body)
{
	try {
		return(body());
	} catch (std::exception const &) {
		return(ReadResult::MEMORY);
	}
}


template<typename T>
bool Decode_Record(ByteSpan data, T & record, LimitsType const & limits, RuleBreak & broken)
{
	return(Decode(data, record, limits, broken));
}


bool Decode_Record(ByteSpan data, Message::RecordingSummary & record, LimitsType const &, RuleBreak & broken)
{
	broken = RuleBreak();
	return(Decode(data, record));
}


// A closed file has size 0, because measuring a closed FileClass would open it.
std::uint64_t File_Size(FileClass & file)
{
	if (!file.Is_Open()) {
		return(0);
	}
	int const size = file.Size();
	return((size > 0) ? std::uint64_t(size) : 0);
}


// Returns the bytes read, which is less than asked only at the end of the file or on a failed
// seek or read.
std::size_t Read_At(FileClass & file, std::uint64_t offset, std::span<std::uint8_t> out)
{
	if (!file.Is_Open() || offset > std::uint64_t(INT_MAX)) {
		return(0);
	}
	if (file.Seek(int(offset), SEEK_SET) != int(offset)) {
		return(0);
	}
	int const want = int(std::min<std::size_t>(out.size(), std::size_t(INT_MAX - int(offset))));
	int const got = file.Read(out.data(), want);
	return((got > 0) ? std::size_t(got) : 0);
}

}


Reader::Reader(FileClass & file, LimitsType const & limits) :
	In(file),
	Limit(limits)
{
}


std::uint64_t Reader::Raw_Limit(std::uint32_t tag) const
{
	switch (tag) {
		case TAG_INFO:
		case TAG_LNCH:
		case TAG_END:
			return(Limit.SmallSection);

		case TAG_MAP:
		case TAG_INIT:
		case TAG_STAT:
			return(Limit.LargeSection);

		case TAG_CKPT:
			return(Limit.Checkpoint);

		default:
			return(NO_LIMIT);
	}
}


/// <summary>
/// Returns true when INIT or FRMS uses a codec this reader does not know. Find does not return
/// such a section, and the file's seals cannot be verified.
/// </summary>
bool Reader::Unknown_Codec(void) const
{
	for (SectionInfo const & section : List) {
		if ((section.Tag == TAG_INIT || section.Tag == TAG_FRMS) && !Known_Codec(section.Codec)) {
			return(true);
		}
	}
	return(false);
}


SectionInfo const * Reader::Find(std::uint32_t tag) const
{
	for (SectionInfo const & section : List) {
		if (section.Tag == tag && Known_Codec(section.Codec)) {
			return(&section);
		}
	}
	return(nullptr);
}


/// <summary>
/// Reads the preamble and every section header. A section cut short by the end of the file is
/// left out, except FRMS, which is read as an open section.
/// </summary>
ReadResult Reader::Open(void)
{
	return(Guarded([&] { return(Open_Sections()); }));
}


ReadResult Reader::Open_Sections(void)
{
	List.clear();
	CheckpointList.clear();

	std::uint64_t const size = File_Size(In);
	std::uint8_t preamble[PREAMBLE_SIZE];
	if (size < PREAMBLE_SIZE) {
		return(ReadResult::NOT_REPLAY);
	}
	if (Read_At(In, 0, preamble) != PREAMBLE_SIZE) {
		return(ReadResult::IO);
	}
	if (std::uint32_t(Little_Endian(preamble, 4)) != MAGIC) {
		return(ReadResult::NOT_REPLAY);
	}
	if (std::uint32_t(Little_Endian(preamble + 4, 4)) != CONTAINER_VERSION) {
		return(ReadResult::VERSION);
	}

	std::uint64_t offset = PREAMBLE_SIZE;
	while (size - offset >= SECTION_HEADER_SIZE) {
		std::uint8_t header[SECTION_HEADER_SIZE];
		if (Read_At(In, offset, header) != SECTION_HEADER_SIZE) {
			return(ReadResult::IO);
		}

		SectionInfo section;
		section.Tag = std::uint32_t(Little_Endian(header, 4));
		section.Codec = std::uint32_t(Little_Endian(header + 4, 4));
		std::uint64_t const stored = Little_Endian(header + 8, 8);
		section.Raw = Little_Endian(header + 16, 8);
		section.Offset = offset + SECTION_HEADER_SIZE;
		std::uint64_t const remaining = size - section.Offset;

		if (stored == OPEN_SIZE) {
			if (section.Tag != TAG_FRMS) {
				return(ReadResult::MALFORMED);
			}
			section.Open = true;
			section.Stored = remaining;
			List.push_back(section);
			break;
		}
		if (section.Raw == OPEN_SIZE) {
			return(ReadResult::MALFORMED);
		}

		// A closed section cut short by the end of the file is ignored, except the frame
		// stream, which is read as far as it goes.
		if (stored > remaining) {
			if (section.Tag == TAG_FRMS) {
				section.Open = true;
				section.Stored = remaining;
				List.push_back(section);
			}
			break;
		}

		section.Stored = stored;
		if (section.Codec == std::uint32_t(Codec::STORED) && stored != section.Raw) {
			return(ReadResult::MALFORMED);
		}

		// The limit of a known tag applies even when its codec is unknown and the body is skipped.
		if (section.Raw > Raw_Limit(section.Tag)) {
			return(ReadResult::LIMIT);
		}
		List.push_back(section);
		offset = section.Offset + stored;
	}

	ReadResult const result = Check_Structure();
	if (result != ReadResult::OK) {
		return(result);
	}
	for (SectionInfo const & section : List) {
		if (section.Tag == TAG_CKPT && Known_Codec(section.Codec)) {
			CheckpointList.push_back(&section);
		}
	}
	return(ReadResult::OK);
}


// Checks the order and number of sections the format allows. A known tag counts with any
// codec, and a section this reader skips still counts for the order.
ReadResult Reader::Check_Structure(void) const
{
	int info = 0;
	int launch = 0;
	int map = 0;
	int init = 0;
	int frames = 0;
	int statistics = 0;
	int end = 0;
	std::size_t position = 0;

	for (SectionInfo const & section : List) {
		if (end > 0) {
			return(ReadResult::MALFORMED);
		}

		// END must use a known codec so the file can be listed, and INFO and LNCH must be the
		// first two sections and use a known codec.
		if (section.Tag == TAG_END && !Known_Codec(section.Codec)) {
			return(ReadResult::MALFORMED);
		}
		if (position == 0 && (section.Tag != TAG_INFO || !Known_Codec(section.Codec))) {
			return(ReadResult::MALFORMED);
		}
		if (position == 1 && (section.Tag != TAG_LNCH || !Known_Codec(section.Codec))) {
			return(ReadResult::MALFORMED);
		}
		position++;

		switch (section.Tag) {
			case TAG_INFO: info++; break;
			case TAG_LNCH: launch++; break;
			case TAG_MAP:
				map++;
				if (frames > 0) {
					return(ReadResult::MALFORMED);
				}
				break;
			case TAG_INIT:
				init++;
				if (frames > 0) {
					return(ReadResult::MALFORMED);
				}
				break;
			case TAG_FRMS: frames++; break;
			case TAG_CKPT:
			case TAG_STAT:
				if (frames == 0) {
					return(ReadResult::MALFORMED);
				}
				statistics += (section.Tag == TAG_STAT) ? 1 : 0;
				break;
			case TAG_END: end++; break;
			default: break;
		}
	}

	if (info != 1 || launch != 1 || init != 1 || map > 1 || frames > 1 || statistics > 1 || end > 1) {
		return(ReadResult::MALFORMED);
	}
	if (end > 0 && frames == 0) {
		return(ReadResult::MALFORMED);
	}
	return(ReadResult::OK);
}


ReadResult Reader::Read_Body(SectionInfo const & section, Bytes & body)
{
	return(Guarded([&] { return(Read_Body_Pieces(section, body)); }));
}


ReadResult Reader::Read_Body_Pieces(SectionInfo const & section, Bytes & body)
{
	body.clear();
	if (section.Open || !Known_Codec(section.Codec)) {
		return(ReadResult::MALFORMED);
	}
	if (section.Raw > Raw_Limit(section.Tag)) {
		return(ReadResult::LIMIT);
	}

	if (section.Codec == std::uint32_t(Codec::STORED)) {
		body.resize(std::size_t(section.Raw));
		if (Read_At(In, section.Offset, body) != body.size()) {
			return(ReadResult::IO);
		}
		return(ReadResult::OK);
	}

	InflaterClass inflater(section.Raw);
	Bytes piece;
	InflateStatus status = InflateStatus::MORE;
	std::uint64_t position = 0;
	while (position < section.Stored && status == InflateStatus::MORE) {
		piece.resize(std::size_t(std::min<std::uint64_t>(section.Stored - position, READ_PIECE)));
		if (Read_At(In, section.Offset + position, piece) != piece.size()) {
			return(ReadResult::IO);
		}
		position += piece.size();
		status = inflater.Write(piece, body);
	}
	if (status == InflateStatus::MEMORY) {
		return(ReadResult::MEMORY);
	}

	if (status != InflateStatus::DONE || inflater.Total_In() != section.Stored || body.size() != section.Raw) {
		return(ReadResult::MALFORMED);
	}
	return(ReadResult::OK);
}


template<typename T>
ReadResult Reader::Read_Record(std::uint32_t tag, T & record)
{
	SectionInfo const * section = Find(tag);
	if (section == nullptr) {
		return(ReadResult::MISSING);
	}
	Bytes body;
	ReadResult result = Read_Body_Pieces(*section, body);
	if (result != ReadResult::OK) {
		return(result);
	}
	RuleBreak broken;
	if (!Decode_Record(body, record, Limit, broken)) {
		return((broken != RuleError::NONE) ? ReadResult::LIMIT : ReadResult::MALFORMED);
	}
	return(ReadResult::OK);
}


/// <summary>
/// Reads INFO. The result is MALFORMED also when two of its income kinds share a number or an
/// id, or one has an empty id.
/// </summary>
ReadResult Reader::Read_Info(Message::RecordingInfo & info)
{
	return(Guarded([&] {
		ReadResult const result = Read_Record(TAG_INFO, info);
		if (result == ReadResult::OK && Check_Income_Kinds(info) != RuleError::NONE) {
			return(ReadResult::MALFORMED);
		}
		return(result);
	}));
}


ReadResult Reader::Read_Launch(Launch::Message::Game & launch)
{
	return(Guarded([&] { return(Read_Record(TAG_LNCH, launch)); }));
}


ReadResult Reader::Read_Statistics(Message::Statistics & statistics)
{
	return(Guarded([&] { return(Read_Record(TAG_STAT, statistics)); }));
}


ReadResult Reader::Read_Summary(Message::RecordingSummary & summary)
{
	return(Guarded([&] { return(Read_Record(TAG_END, summary)); }));
}


ReadResult Reader::Read_Checkpoint(SectionInfo const & section, Message::Checkpoint & checkpoint)
{
	return(Guarded([&] { return(Read_Checkpoint_Body(section, checkpoint)); }));
}


ReadResult Reader::Read_Checkpoint_Body(SectionInfo const & section, Message::Checkpoint & checkpoint)
{
	Bytes body;
	ReadResult result = Read_Body_Pieces(section, body);
	if (result != ReadResult::OK) {
		return(result);
	}
	if (!Decode(body, checkpoint)) {
		return(ReadResult::MALFORMED);
	}
	if (CRC::Memory(checkpoint.State.data(), unsigned(checkpoint.State.size())) != checkpoint.Crc32) {
		return(ReadResult::MALFORMED);
	}
	return(ReadResult::OK);
}


/// <summary>
/// Reads a checkpoint's frame from the start of its section. The result is the checkpoint's
/// frame when its writer put frame first, as a canonical writer does.
/// </summary>
ReadResult Reader::Read_Checkpoint_Frame(SectionInfo const & section, std::uint32_t & frame)
{
	return(Guarded([&] { return(Read_Checkpoint_Start(section, frame)); }));
}


ReadResult Reader::Read_Checkpoint_Start(SectionInfo const & section, std::uint32_t & frame)
{
	frame = 0;
	if (section.Open || !Known_Codec(section.Codec)) {
		return(ReadResult::MALFORMED);
	}
	if (section.Raw > Raw_Limit(section.Tag)) {
		return(ReadResult::LIMIT);
	}

	std::size_t const wanted = std::size_t(std::min<std::uint64_t>(section.Raw, CHECKPOINT_PEEK));
	Bytes start;
	if (section.Codec == std::uint32_t(Codec::STORED)) {
		start.resize(wanted);
		if (Read_At(In, section.Offset, start) != start.size()) {
			return(ReadResult::IO);
		}
	} else {
		InflaterClass inflater(wanted);
		Bytes piece;
		std::uint64_t position = 0;
		InflateStatus status = InflateStatus::MORE;
		while (start.size() < wanted && position < section.Stored && status == InflateStatus::MORE) {
			piece.resize(std::size_t(std::min<std::uint64_t>(section.Stored - position, 256)));
			if (Read_At(In, section.Offset + position, piece) != piece.size()) {
				return(ReadResult::IO);
			}
			position += piece.size();
			status = inflater.Write(piece, start);
		}
		if (status == InflateStatus::MEMORY) {
			return(ReadResult::MEMORY);
		}
		if (status == InflateStatus::BROKEN || start.size() < wanted) {
			return(ReadResult::MALFORMED);
		}
	}

	// frame is written first; a checkpoint at frame 0 has no frame field. A value wider than
	// 32 bits keeps its low bits, as a full decode does.
	if (start.empty() || start[0] != 0x08) {
		return(ReadResult::OK);
	}
	std::size_t offset = 1;
	std::uint64_t value = 0;
	if (Read_Varint(start, offset, value) != WireError::NONE) {
		return(ReadResult::MALFORMED);
	}
	frame = std::uint32_t(value);
	return(ReadResult::OK);
}


/// <summary>
/// Reads every checkpoint in full. Each state must match its crc32, and the frames must ascend
/// with no two checkpoints on one frame.
/// </summary>
ReadResult Reader::Check_Checkpoints(void)
{
	return(Guarded([&] { return(Check_Checkpoint_Bodies()); }));
}


ReadResult Reader::Check_Checkpoint_Bodies(void)
{
	std::optional<std::uint32_t> previous;
	for (SectionInfo const * section : CheckpointList) {
		Message::Checkpoint checkpoint;
		ReadResult const result = Read_Checkpoint_Body(*section, checkpoint);
		if (result != ReadResult::OK) {
			return(result);
		}
		if (previous && checkpoint.Frame <= *previous) {
			return(ReadResult::MALFORMED);
		}
		previous = checkpoint.Frame;
	}
	return(ReadResult::OK);
}


FrameReader::FrameReader(Reader & reader) :
	Owner(reader),
	Present(false),
	Stream(NO_LIMIT),
	Consumed(0),
	Position(0),
	InputDone(false),
	StreamDone(false),
	Previous(-1),
	Failure(ReadResult::OK)
{
	SectionInfo const * section = reader.Find(TAG_FRMS);
	if (section != nullptr) {
		Section = *section;
		Present = true;
	}
}


FrameStep FrameReader::Fail(ReadResult result)
{
	Failure = result;
	return(FrameStep::FAILED);
}


// Inflates the next piece of input onto Buffer. Returns false when no input is left, or when
// the stream fails, which sets Failure.
bool FrameReader::Fill(void)
{
	if (InputDone || StreamDone) {
		return(false);
	}
	std::uint64_t const remaining = Section.Stored - Position;
	if (remaining == 0) {
		InputDone = true;
		return(false);
	}

	if (Consumed > (1u << 20)) {
		Buffer.erase(Buffer.begin(), Buffer.begin() + std::ptrdiff_t(Consumed));
		Consumed = 0;
	}

	std::uint8_t piece[FRAME_PIECE];
	std::size_t const wanted = std::size_t(std::min<std::uint64_t>(remaining, FRAME_PIECE));
	std::size_t const got = Read_At(Owner.Input(), Section.Offset + Position, std::span<std::uint8_t>(piece, wanted));
	if (got != wanted) {
		Failure = ReadResult::IO;
		return(false);
	}
	Position += got;

	if (Section.Codec == std::uint32_t(Codec::STORED)) {
		Buffer.insert(Buffer.end(), piece, piece + got);
		return(true);
	}

	switch (Stream.Write(ByteSpan(piece, got), Buffer)) {
		case InflateStatus::MORE:
			return(true);

		case InflateStatus::DONE:
			StreamDone = true;
			return(true);

		// An unfinished stream can end in bytes the writer never wrote, so broken data ends it
		// as the end of the input does, after what it inflated so far.
		case InflateStatus::BROKEN:
			if (Section.Open) {
				InputDone = true;
				return(true);
			}
			Failure = ReadResult::MALFORMED;
			return(false);

		case InflateStatus::MEMORY:
			Failure = ReadResult::MEMORY;
			return(false);

		// Only a closed stream past its terminator has a limit.
		case InflateStatus::LIMIT:
		default:
			Failure = ReadResult::MALFORMED;
			return(false);
	}
}


FrameStep FrameReader::Next(std::uint32_t & frame, Message::Frame & record)
{
	if (Failure != ReadResult::OK) {
		return(FrameStep::FAILED);
	}
	if (!Present) {
		return(FrameStep::CUT);
	}
	try {
		if (!Kinds) {
			Message::RecordingInfo info;
			ReadResult const result = Owner.Read_Info(info);
			if (result != ReadResult::OK) {
				return(Fail(result));
			}
			Kinds.emplace(info);
		}
		return(Next_Record(frame, record));
	} catch (std::exception const &) {
		return(Fail(ReadResult::MEMORY));
	}
}


FrameStep FrameReader::Next_Record(std::uint32_t & frame, Message::Frame & record)
{
	for (;;) {
		std::size_t offset = Consumed;
		std::uint64_t length = 0;
		WireError error = Read_Varint(Buffer, offset, length);
		bool complete = (error == WireError::NONE) && (length == 0 || Buffer.size() - offset >= length);

		if (error == WireError::VARINT_TOO_LONG) {
			return(Fail(ReadResult::MALFORMED));
		}
		if (error == WireError::NONE && length > Owner.Limits().FrameRecord) {
			return(Fail(ReadResult::LIMIT));
		}
		if (!complete) {
			if (Fill()) {
				continue;
			}
			if (Failure != ReadResult::OK) {
				return(FrameStep::FAILED);
			}
			// A stream that was closed must end with its terminator.
			return(Section.Open ? FrameStep::CUT : Fail(ReadResult::MALFORMED));
		}

		if (length == 0) {
			Consumed = offset;
			return(Section.Open ? FrameStep::END : End_Closed());
		}

		RuleBreak broken;
		if (!Decode(ByteSpan(Buffer.data() + offset, std::size_t(length)), record, Owner.Limits(), broken)) {
			return(Fail((broken != RuleError::NONE) ? ReadResult::LIMIT : ReadResult::MALFORMED));
		}

		// The seals cover the defeated houses in ascending order, so another order would give
		// the list two readings.
		for (std::size_t i = 1; i < record.Defeated.size(); i++) {
			if (record.Defeated[i] <= record.Defeated[i - 1]) {
				return(Fail(ReadResult::MALFORMED));
			}
		}
		if (Kinds->Check(record) != RuleError::NONE) {
			return(Fail(ReadResult::MALFORMED));
		}
		Consumed = offset + std::size_t(length);

		std::int64_t const number = Previous + 1 + std::int64_t(record.Skipped);
		if (number > std::int64_t(0xFFFFFFFFll)) {
			return(Fail(ReadResult::MALFORMED));
		}
		Previous = number;
		frame = std::uint32_t(number);
		return(FrameStep::RECORD);
	}
}


// A closed stream has nothing after its terminator, inflated or stored, and its deflate data
// ends at the stored size.
FrameStep FrameReader::End_Closed(void)
{
	if (Consumed != Buffer.size()) {
		return(Fail(ReadResult::MALFORMED));
	}
	if (Section.Codec == std::uint32_t(Codec::STORED)) {
		return((Position == Section.Stored) ? FrameStep::END : Fail(ReadResult::MALFORMED));
	}

	// The rest of the input may end the stream but not inflate to anything, so a byte it would
	// inflate to fails the limit before it is buffered.
	Stream.Set_Limit(Stream.Total_Out());
	while (Fill()) {
	}
	if (Failure != ReadResult::OK) {
		return(FrameStep::FAILED);
	}
	if (!StreamDone || Stream.Total_In() != Section.Stored || Stream.Total_Out() != Section.Raw) {
		return(Fail(ReadResult::MALFORMED));
	}
	return(FrameStep::END);
}


namespace
{

SealReport Check_Seals(Reader & reader)
{
	SealReport report;

	Message::RecordingInfo info;
	report.Error = reader.Read_Info(info);
	if (report.Error != ReadResult::OK) {
		return(report);
	}

	// INIT is missing here only when its codec is unknown, since Open requires it.
	SectionInfo const * launch_section = reader.Find(TAG_LNCH);
	SectionInfo const * init_section = reader.Find(TAG_INIT);
	if (launch_section == nullptr) {
		report.Error = ReadResult::MISSING;
		return(report);
	}
	Bytes launch;
	Bytes init;
	report.Error = reader.Read_Body(*launch_section, launch);
	if (report.Error == ReadResult::OK && init_section != nullptr) {
		report.Error = reader.Read_Body(*init_section, init);
	}
	if (report.Error != ReadResult::OK) {
		return(report);
	}

	// A file that cannot be verified is still read as far as it can be, so that a malformed one
	// is reported as malformed and not as not verified.
	std::optional<SealVerifier> verifier;
	if (info.SealVersion == SEAL_VERSION && !reader.Unknown_Codec()) {
		verifier.emplace(Seal_Start(info.SealVersion, info.Engine, info.DataVersion, launch, init));
	}

	FrameReader frames(reader);
	std::uint32_t frame = 0;
	std::optional<std::uint32_t> last_record;
	Message::Frame record;
	for (;;) {
		FrameStep step = frames.Next(frame, record);
		if (step == FrameStep::RECORD) {
			if (verifier) {
				verifier->Add(frame, record);
			}
			last_record = frame;
			continue;
		}
		if (step == FrameStep::FAILED) {
			report.Error = frames.Error();
			return(report);
		}
		break;
	}

	Message::RecordingSummary summary;
	if (reader.Complete()) {
		report.Error = reader.Read_Summary(summary);
		if (report.Error != ReadResult::OK) {
			return(report);
		}

		// A summary without last_frame means that no frame ran, so any record is too late.
		if (last_record && (!summary.LastFrame || *last_record > *summary.LastFrame)) {
			report.Error = ReadResult::MALFORMED;
			report.LateRecord = last_record;
			return(report);
		}
	}

	report.Error = reader.Check_Checkpoints();
	if (report.Error != ReadResult::OK) {
		return(report);
	}

	if (!verifier) {
		report.Status = SealStatus::NOT_CHECKED;
		return(report);
	}
	if (reader.Complete()) {
		verifier->Finish(summary);
	}

	report.Compared = verifier->Compared();
	report.FirstMismatch = verifier->First_Mismatch();
	report.FirstMisplaced = verifier->First_Misplaced();
	if (report.FirstMisplaced) {
		report.Error = ReadResult::MALFORMED;
		report.Status = SealStatus::MALFORMED;
	} else {
		report.Status = verifier->Good() ? SealStatus::VERIFIED : SealStatus::MISMATCH;
	}
	return(report);
}

}


/// <summary>
/// Compares the seal chain recomputed from an opened file with the seals in it, and checks
/// every checkpoint as Check_Checkpoints does. An unclosed file is checked as far as its frame
/// stream goes. A malformed file reports MALFORMED even when its seal version is unknown, and
/// running out of memory reports MALFORMED with Error MEMORY.
/// </summary>
SealReport Verify_Seals(Reader & reader)
{
	try {
		return(Check_Seals(reader));
	} catch (std::exception const &) {
		SealReport report;
		report.Error = ReadResult::MEMORY;
		return(report);
	}
}


namespace
{

FileStatus Failed_Status(ReadResult result)
{
	return((result == ReadResult::LIMIT) ? FileStatus::LIMIT : FileStatus::MALFORMED);
}


FileReport Check_File(Reader & reader)
{
	FileReport report;
	report.Error = reader.Open();
	if (report.Error == ReadResult::NOT_REPLAY || report.Error == ReadResult::VERSION) {
		report.Status = FileStatus::NOT_REPLAY;
		return(report);
	}

	// Verify_Seals reads LNCH only as bytes and skips MAP and STAT, so their records are decoded here.
	Message::RecordingInfo info;
	Launch::Message::Game launch;
	Bytes map;
	Message::Statistics statistics;
	SectionInfo const * map_section = reader.Find(TAG_MAP);
	SectionInfo const * stat_section = reader.Find(TAG_STAT);
	if (report.Error == ReadResult::OK) {
		report.Error = reader.Read_Info(info);
	}
	if (report.Error == ReadResult::OK) {
		report.Error = reader.Read_Launch(launch);
	}
	if (report.Error == ReadResult::OK && map_section != nullptr) {
		report.Error = reader.Read_Body(*map_section, map);
	}
	if (report.Error == ReadResult::OK && stat_section != nullptr) {
		report.Error = reader.Read_Statistics(statistics);
	}
	if (report.Error != ReadResult::OK) {
		report.Status = Failed_Status(report.Error);
		return(report);
	}

	report.Seals = Verify_Seals(reader);
	switch (report.Seals->Status) {
		case SealStatus::VERIFIED:
			report.Status = reader.Complete() ? FileStatus::COMPLETE : FileStatus::INCOMPLETE;
			break;
		case SealStatus::MISMATCH:
			report.Status = FileStatus::MISMATCH;
			break;
		case SealStatus::NOT_CHECKED:
			report.Status = FileStatus::NOT_VERIFIED;
			break;
		case SealStatus::MALFORMED:
		default:
			report.Error = report.Seals->Error;
			report.Status = Failed_Status(report.Error);
			break;
	}
	return(report);
}

}


/// <summary>
/// Opens the file and reads all of it: every section the reader knows, the frame stream, the
/// checkpoints, and the seals. Returns the first result that applies in the order of section 11:
/// NOT_REPLAY, then MALFORMED, LIMIT, or MISMATCH, then NOT_VERIFIED, then INCOMPLETE. A failed
/// read of the file, or running out of memory, reports MALFORMED with Error IO or MEMORY.
/// </summary>
FileReport Verify_File(Reader & reader)
{
	try {
		return(Check_File(reader));
	} catch (std::exception const &) {
		FileReport report;
		report.Error = ReadResult::MEMORY;
		return(report);
	}
}

}
