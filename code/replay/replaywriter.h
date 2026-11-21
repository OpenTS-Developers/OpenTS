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
#include "replay/replayrecords.h"
#include "replay/replayseal.h"
#include "wwfile.h"

#include <cstdint>
#include <optional>
#include <vector>

namespace Replay
{
	enum class WriteResult : std::uint8_t {
		OK,
		FAILED,     // the file failed; the writer has stopped and writes nothing more
		REFUSED,    // the input breaks a limit, an ordering rule, or an income kind rule and was not written
		STATE,      // the call does not fit the writer's state, such as a frame after Close
	};

	// Writes one recording into an empty file opened for writing. No call throws; any failure
	// stops the writer for good. A file cannot pass 2 GiB, because FileClass positions are int.
	class Writer
	{
		public:
			explicit Writer(FileClass & file);

			// Holds the first sections; map is the MAP body, or absent to leave the map out.
			WriteResult Begin(RecordingInfo const & info, Launch const & launch, ByteSpan init,
				std::optional<ByteSpan> map);

			// Writes the record for frame, which must follow the last record's frame.
			WriteResult Add_Frame(std::uint32_t frame, Frame record);

			// Holds a checkpoint until Close. Frames must ascend.
			WriteResult Add_Checkpoint(Checkpoint const & checkpoint);

			// Sync-flushes the frame stream and flushes the file.
			WriteResult Flush(void);

			// Ends the frame stream and writes the checkpoints, statistics if given, and summary.
			WriteResult Close(RecordingSummary summary, Statistics const * statistics);

			bool Failed(void) const { return(State == StateType::FAILED); }

		private:
			enum class StateType : std::uint8_t {
				IDLE,
				HOLDING,
				FRAMES,
				CLOSED,
				FAILED,
			};

			struct HeldCheckpoint {
				std::uint32_t Frame = 0;
				std::uint64_t RawSize = 0;
				Bytes Body;
			};

			WriteResult Begin_Body(RecordingInfo const & info, Launch const & launch, ByteSpan init,
				std::optional<ByteSpan> map);
			WriteResult Add_Frame_Body(std::uint32_t frame, Frame & record);
			WriteResult Add_Checkpoint_Body(Checkpoint const & checkpoint);
			WriteResult Close_Body(RecordingSummary & summary, Statistics const * statistics);

			WriteResult Fail(void);
			bool Put(ByteSpan data);
			bool Put_At(std::uint64_t offset, ByteSpan data);
			bool Put_Section(std::uint32_t tag, Codec codec, ByteSpan body);
			bool Put_Section_Header(std::uint32_t tag, Codec codec, std::uint64_t stored, std::uint64_t raw);
			bool Start_File(void);
			bool Put_Frame_Bytes(ByteSpan data);
			bool Drain(void);
			bool Sync(void);

			FileClass & Output;
			StateType State;
			LimitsType Limits;
			std::uint64_t Offset;

			Bytes InfoBody;
			Bytes LaunchBody;
			Bytes InitBody;
			std::optional<Bytes> MapBody;

			std::optional<SealChain> Chain;
			std::vector<Seal> Seals;
			std::optional<std::uint32_t> LastRecord;
			IncomeKindSet Kinds;

			DeflaterClass Stream;
			Bytes Pending;
			std::uint64_t FramesOffset;
			std::uint64_t FramesRaw;
			std::uint64_t FramesStored;
			std::uint32_t RecordsSinceFlush;

			std::vector<HeldCheckpoint> Checkpoints;
	};
}
