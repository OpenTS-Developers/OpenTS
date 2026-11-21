/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "deflate.h"
#include "replay/replaymessages.h"
#include "replay/replayseal.h"
#include "wwfile.h"

#include <cstdint>
#include <optional>
#include <vector>

// Reads the replay container. Every size and count in a file is checked against the reader's
// limits before anything is allocated for it.

namespace Replay
{
	enum class ReadResult : std::uint8_t {
		OK,
		NOT_REPLAY,   // no CNCR preamble
		VERSION,      // a container version this reader does not know
		MALFORMED,    // the file breaks a rule of the format
		LIMIT,        // a size or count is past the reader's limits
		MISSING,      // the file has no such section
		IO,           // the file could not be read up to the size it reported
		MEMORY,       // memory ran out
	};

	struct SectionInfo {
		std::uint32_t Tag = 0;
		std::uint32_t Codec = 0;
		std::uint64_t Offset = 0;    // of the body
		std::uint64_t Stored = 0;    // bytes of body in the file, up to its end for an open section
		std::uint64_t Raw = 0;       // OPEN_SIZE while the raw size is unknown
		bool Open = false;           // the body runs to the end of the file: unfinished or cut short
	};

	// Reads a file the caller opened for reading; a closed file reads as empty. No call throws,
	// and a file past 2 GiB cannot be read, because FileClass positions are int.
	class Reader
	{
		public:
			explicit Reader(FileClass & file, LimitsType const & limits = LimitsType());
			Reader(Reader const &) = delete;
			Reader & operator = (Reader const &) = delete;

			// Reads the preamble and every section header.
			ReadResult Open(void);

			std::vector<SectionInfo> const & Sections(void) const { return(List); }
			SectionInfo const * Find(std::uint32_t tag) const;

			// A file without END was not closed: the recording stopped without a summary.
			bool Complete(void) const { return(Find(TAG_END) != nullptr); }

			// True when INIT or FRMS uses a codec this reader does not know.
			bool Unknown_Codec(void) const;

			ReadResult Read_Info(Message::RecordingInfo & info);
			ReadResult Read_Launch(Message::Launch & launch);
			ReadResult Read_Statistics(Message::Statistics & statistics);
			ReadResult Read_Summary(Message::RecordingSummary & summary);

			// The uncompressed body of a section, within its limit.
			ReadResult Read_Body(SectionInfo const & section, Bytes & body);

			std::vector<SectionInfo const *> const & Checkpoints(void) const { return(CheckpointList); }
			ReadResult Read_Checkpoint(SectionInfo const & section, Message::Checkpoint & checkpoint);

			// Reads a checkpoint's frame by inflating only the start of its section.
			ReadResult Read_Checkpoint_Frame(SectionInfo const & section, std::uint32_t & frame);

			// Reads every checkpoint in full and checks its crc32 and the frame order.
			ReadResult Check_Checkpoints(void);

			FileClass & Input(void) { return(In); }
			LimitsType const & Limits(void) const { return(Limit); }

		private:
			ReadResult Open_Sections(void);
			ReadResult Check_Structure(void) const;
			ReadResult Read_Body_Pieces(SectionInfo const & section, Bytes & body);
			ReadResult Read_Checkpoint_Body(SectionInfo const & section, Message::Checkpoint & checkpoint);
			ReadResult Read_Checkpoint_Start(SectionInfo const & section, std::uint32_t & frame);
			ReadResult Check_Checkpoint_Bodies(void);
			std::uint64_t Raw_Limit(std::uint32_t tag) const;

			template<typename T>
			ReadResult Read_Record(std::uint32_t tag, T & record);

			FileClass & In;
			LimitsType Limit;
			std::vector<SectionInfo> List;
			std::vector<SectionInfo const *> CheckpointList;
	};

	enum class FrameStep : std::uint8_t {
		RECORD,   // a record was read
		END,      // the zero length that ends the stream
		CUT,      // the input ran out; a final record cut short is discarded
		FAILED,   // the stream is malformed or breaks a limit; Error gives the reason
	};

	// Reads the records of the frame stream in order, inflating it a piece at a time. Without an
	// FRMS of a known codec every call returns CUT. Otherwise the first call also reads INFO for
	// its income kinds, and fails with Read_Info's result when that fails.
	class FrameReader
	{
		public:
			explicit FrameReader(Reader & reader);

			FrameStep Next(std::uint32_t & frame, Message::Frame & record);
			ReadResult Error(void) const { return(Failure); }

		private:
			FrameStep Fail(ReadResult result);
			FrameStep Next_Record(std::uint32_t & frame, Message::Frame & record);
			FrameStep End_Closed(void);
			bool Fill(void);

			Reader & Owner;
			SectionInfo Section;
			bool Present;
			InflaterClass Stream;
			Bytes Buffer;
			std::size_t Consumed;
			std::uint64_t Position;
			bool InputDone;
			bool StreamDone;
			std::int64_t Previous;
			std::optional<IncomeKindSet> Kinds;
			ReadResult Failure;
	};

	enum class SealStatus : std::uint8_t {
		VERIFIED,      // every seal in the file matches the recomputed chain
		MISMATCH,      // a seal differs; FirstMismatch names it
		NOT_CHECKED,   // not verified: the seal version, or the codec of INIT or FRMS, is unknown
		MALFORMED,     // the file breaks a rule of the format; Error gives the read result that found it
	};

	struct SealReport {
		SealStatus Status = SealStatus::MALFORMED;
		ReadResult Error = ReadResult::OK;
		std::size_t Compared = 0;
		std::optional<std::uint32_t> FirstMismatch;
		std::optional<std::uint32_t> FirstMisplaced;   // an interval seal missing from its record or on another
		std::optional<std::uint32_t> LateRecord;       // the frame of a record after last_frame
	};

	// Recomputes the seal chain of an opened file and compares it with the seals in it.
	// It checks the seals, not every rule: Verify_File gives the file's result.
	SealReport Verify_Seals(Reader & reader);

	// The results of reading a whole file, as the format's section 11 lists them, and LIMIT.
	enum class FileStatus : std::uint8_t {
		COMPLETE,       // END is present and every seal matches the chain
		INCOMPLETE,     // END is missing and every seal matches the chain
		NOT_VERIFIED,   // the seal version, or the codec of INIT or FRMS, is unknown
		MISMATCH,       // a seal differs from the chain; Seals names the first
		MALFORMED,      // the file breaks a rule of the format, or Error is IO or MEMORY
		LIMIT,          // a size or count is past the reader's limits
		NOT_REPLAY,     // no CNCR preamble, or a container version this reader does not know
	};

	struct FileReport {
		FileStatus Status = FileStatus::MALFORMED;
		ReadResult Error = ReadResult::OK;
		std::optional<SealReport> Seals;   // absent when reading stopped before the seals
	};

	FileReport Verify_File(Reader & reader);
}
