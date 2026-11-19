/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "replay/replaywriter.h"

#include "replay/replayfields.hh"

#include <climits>
#include <cstdio>
#include <exception>
#include <utility>


namespace Replay
{

Writer::Writer(FileClass & file) :
	Output(file),
	State(StateType::IDLE),
	Offset(0),
	FramesOffset(0),
	FramesRaw(0),
	FramesStored(0),
	RecordsSinceFlush(0)
{
}


WriteResult Writer::Fail(void)
{
	State = StateType::FAILED;
	Checkpoints.clear();
	return(WriteResult::FAILED);
}


// A closed file is refused, because a FileClass write to one opens the file and closes it again.
bool Writer::Put(ByteSpan data)
{
	if (!Output.Is_Open() || data.size() > std::uint64_t(INT_MAX) - Offset) {
		return(false);
	}
	if (!data.empty() && Output.Write(data.data(), int(data.size())) != int(data.size())) {
		return(false);
	}
	Offset += data.size();
	return(true);
}


// Overwrites bytes already written and moves back to the end, where the next Put appends.
bool Writer::Put_At(std::uint64_t offset, ByteSpan data)
{
	if (!Output.Is_Open() || offset > Offset || data.size() > Offset - offset) {
		return(false);
	}
	bool const placed = Output.Seek(int(offset), SEEK_SET) == int(offset);
	bool const written = placed && Output.Write(data.data(), int(data.size())) == int(data.size());
	bool const returned = Output.Seek(int(Offset), SEEK_SET) == int(Offset);
	return(written && returned);
}


bool Writer::Put_Section_Header(std::uint32_t tag, Codec codec, std::uint64_t stored, std::uint64_t raw)
{
	Bytes header;
	Put_Fixed32(header, tag);
	Put_Fixed32(header, std::uint32_t(codec));
	Put_Fixed64(header, stored);
	Put_Fixed64(header, raw);
	return(Put(header));
}


bool Writer::Put_Section(std::uint32_t tag, Codec codec, ByteSpan body)
{
	if (codec == Codec::STORED) {
		return(Put_Section_Header(tag, codec, body.size(), body.size()) && Put(body));
	}

	Bytes packed;
	if (!Deflate_All(body, packed)) {
		return(false);
	}
	return(Put_Section_Header(tag, codec, packed.size(), body.size()) && Put(packed));
}


// Writes everything that was held and opens the frame stream.
bool Writer::Start_File(void)
{
	Bytes preamble;
	Put_Fixed32(preamble, MAGIC);
	Put_Fixed32(preamble, CONTAINER_VERSION);

	bool written = Put(preamble)
		&& Put_Section(TAG_INFO, Codec::STORED, InfoBody)
		&& Put_Section(TAG_LNCH, Codec::STORED, LaunchBody)
		&& (!MapBody || Put_Section(TAG_MAP, Codec::DEFLATE, *MapBody))
		&& Put_Section(TAG_INIT, Codec::DEFLATE, InitBody);
	if (!written) {
		return(false);
	}

	FramesOffset = Offset;
	if (!Put_Section_Header(TAG_FRMS, Codec::DEFLATE, OPEN_SIZE, OPEN_SIZE) || !Output.Flush()) {
		return(false);
	}

	InfoBody = Bytes();
	LaunchBody = Bytes();
	InitBody = Bytes();
	MapBody.reset();
	State = StateType::FRAMES;
	return(true);
}


bool Writer::Drain(void)
{
	if (Pending.empty()) {
		return(true);
	}
	if (!Put(Pending)) {
		return(false);
	}
	FramesStored += Pending.size();
	Pending.clear();
	return(true);
}


bool Writer::Put_Frame_Bytes(ByteSpan data)
{
	FramesRaw += data.size();
	return(Stream.Write(data, Pending) && Drain());
}


bool Writer::Sync(void)
{
	RecordsSinceFlush = 0;
	return(Stream.Sync_Flush(Pending) && Drain() && Output.Flush());
}


/// <summary>
/// Holds the first sections until the first frame record or Close writes them. The writer sets
/// the file's SealVersion to the version of the seals it computes and ignores the value in info.
/// </summary>
WriteResult Writer::Begin(RecordingInfo const & info, Launch const & launch, ByteSpan init,
	std::optional<ByteSpan> map)
{
	if (State == StateType::FAILED) {
		return(WriteResult::FAILED);
	}
	if (State != StateType::IDLE) {
		return(WriteResult::STATE);
	}
	try {
		return(Begin_Body(info, launch, init, map));
	} catch (std::exception const &) {
		return(Fail());
	}
}


WriteResult Writer::Begin_Body(RecordingInfo const & info, Launch const & launch, ByteSpan init,
	std::optional<ByteSpan> map)
{
	RecordingInfo sealed_info = info;
	sealed_info.SealVersion = SEAL_VERSION;

	if (Check_Limits(sealed_info, Limits) != RuleError::NONE || Check_Order(sealed_info) != RuleError::NONE
		|| Check_Income_Kinds(sealed_info) != RuleError::NONE
		|| Check_Limits(launch, Limits) != RuleError::NONE || Check_Order(launch) != RuleError::NONE) {
		return(WriteResult::REFUSED);
	}
	if (init.size() > Limits.LargeSection || (map && map->size() > Limits.LargeSection)) {
		return(WriteResult::REFUSED);
	}

	Bytes info_body;
	Bytes launch_body;
	Encode(sealed_info, info_body);
	Encode(launch, launch_body);
	if (info_body.size() > Limits.SmallSection || launch_body.size() > Limits.SmallSection) {
		return(WriteResult::REFUSED);
	}

	InfoBody = std::move(info_body);
	LaunchBody = std::move(launch_body);
	InitBody.assign(init.begin(), init.end());
	if (map) {
		MapBody = Bytes(map->begin(), map->end());
	}
	Chain.emplace(Seal_Start(SEAL_VERSION, sealed_info.Engine, sealed_info.DataVersion, LaunchBody, InitBody));
	Kinds = IncomeKindSet(sealed_info);
	State = StateType::HOLDING;
	return(WriteResult::OK);
}


/// <summary>
/// Writes the record for frame, which must follow the last record's frame, with Skipped and
/// Seals set by the writer. An empty record is not written.
/// </summary>
WriteResult Writer::Add_Frame(std::uint32_t frame, Frame record)
{
	if (State == StateType::FAILED) {
		return(WriteResult::FAILED);
	}
	if (State != StateType::HOLDING && State != StateType::FRAMES) {
		return(WriteResult::STATE);
	}
	try {
		return(Add_Frame_Body(frame, record));
	} catch (std::exception const &) {
		return(Fail());
	}
}


WriteResult Writer::Add_Frame_Body(std::uint32_t frame, Frame & record)
{
	if (LastRecord && frame <= *LastRecord) {
		return(WriteResult::STATE);
	}

	record.Skipped = 0;
	record.Seals.clear();
	if (Check_Limits(record, Limits) != RuleError::NONE || Check_Order(record) != RuleError::NONE
		|| Kinds.Check(record) != RuleError::NONE) {
		return(WriteResult::REFUSED);
	}

	SealChain chain = *Chain;
	chain.Add(frame, record, record.Seals);

	// Skipped is field 1 and is written first, so the rest of the record is encoded separately
	// to tell whether it is empty.
	Bytes rest;
	Encode(record, rest);
	if (rest.empty()) {
		return(WriteResult::OK);
	}

	std::uint32_t const skipped = LastRecord ? frame - *LastRecord - 1 : frame;
	Bytes body;
	WireWriter(body).Uint(Field::Frame::SKIPPED, skipped);
	body.insert(body.end(), rest.begin(), rest.end());
	if (body.size() > Limits.FrameRecord) {
		return(WriteResult::REFUSED);
	}

	if (State == StateType::HOLDING && !Start_File()) {
		return(Fail());
	}

	Bytes framed;
	Put_Varint(framed, body.size());
	framed.insert(framed.end(), body.begin(), body.end());
	if (!Put_Frame_Bytes(framed)) {
		return(Fail());
	}

	*Chain = std::move(chain);
	Seals.insert(Seals.end(), record.Seals.begin(), record.Seals.end());
	LastRecord = frame;

	if (++RecordsSinceFlush >= FLUSH_RECORDS && !Sync()) {
		return(Fail());
	}
	return(WriteResult::OK);
}


WriteResult Writer::Add_Checkpoint(Checkpoint const & checkpoint)
{
	if (State == StateType::FAILED) {
		return(WriteResult::FAILED);
	}
	if (State != StateType::HOLDING && State != StateType::FRAMES) {
		return(WriteResult::STATE);
	}
	try {
		return(Add_Checkpoint_Body(checkpoint));
	} catch (std::exception const &) {
		return(Fail());
	}
}


WriteResult Writer::Add_Checkpoint_Body(Checkpoint const & checkpoint)
{
	if (!Checkpoints.empty() && checkpoint.Frame <= Checkpoints.back().Frame) {
		return(WriteResult::REFUSED);
	}

	Bytes raw;
	Encode(checkpoint, raw);
	if (raw.size() > Limits.Checkpoint) {
		return(WriteResult::REFUSED);
	}

	HeldCheckpoint held;
	held.Frame = checkpoint.Frame;
	held.RawSize = raw.size();
	if (!Deflate_All(raw, held.Body)) {
		return(Fail());
	}
	Checkpoints.push_back(std::move(held));
	return(WriteResult::OK);
}


WriteResult Writer::Flush(void)
{
	if (State == StateType::FAILED) {
		return(WriteResult::FAILED);
	}
	if (State == StateType::CLOSED) {
		return(WriteResult::STATE);
	}
	if (State != StateType::FRAMES) {
		return(WriteResult::OK);
	}
	try {
		return(Sync() ? WriteResult::OK : Fail());
	} catch (std::exception const &) {
		return(Fail());
	}
}


/// <summary>
/// Ends the frame stream and writes the checkpoints, statistics if given, and the summary, whose
/// Seals the writer fills. Set summary.LastFrame when any record was written, no earlier than
/// the last record's frame; otherwise the call is refused.
/// </summary>
WriteResult Writer::Close(RecordingSummary summary, Statistics const * statistics)
{
	if (State == StateType::FAILED) {
		return(WriteResult::FAILED);
	}
	if (State != StateType::HOLDING && State != StateType::FRAMES) {
		return(WriteResult::STATE);
	}
	try {
		return(Close_Body(summary, statistics));
	} catch (std::exception const &) {
		return(Fail());
	}
}


WriteResult Writer::Close_Body(RecordingSummary & summary, Statistics const * statistics)
{
	if (LastRecord && (!summary.LastFrame || *summary.LastFrame < *LastRecord)) {
		return(WriteResult::REFUSED);
	}

	Bytes statistics_body;
	if (statistics != nullptr) {
		if (Check_Limits(*statistics, Limits) != RuleError::NONE || Check_Order(*statistics) != RuleError::NONE) {
			return(WriteResult::REFUSED);
		}
		Encode(*statistics, statistics_body);
		if (statistics_body.size() > Limits.LargeSection) {
			return(WriteResult::REFUSED);
		}
	}

	summary.Seals = Seals;
	if (summary.LastFrame) {
		SealChain chain = *Chain;
		chain.Finish(*summary.LastFrame, summary.Seals);
	}
	Bytes summary_body;
	Encode(summary, summary_body);
	if (summary_body.size() > Limits.SmallSection) {
		return(WriteResult::REFUSED);
	}

	if (State == StateType::HOLDING && !Start_File()) {
		return(Fail());
	}

	// The stream reaches the OS before the header names its sizes, and the raw size is patched
	// first, so a header with a real stored size always has a real raw size.
	Bytes const terminator = { 0 };
	Bytes raw_size;
	Bytes stored_size;
	bool closed = Put_Frame_Bytes(terminator) && Stream.Finish(Pending) && Drain() && Output.Flush();
	Put_Fixed64(raw_size, FramesRaw);
	Put_Fixed64(stored_size, FramesStored);
	closed = closed
		&& Put_At(FramesOffset + 16, raw_size)
		&& Put_At(FramesOffset + 8, stored_size)
		&& Output.Flush();
	if (!closed) {
		return(Fail());
	}

	for (HeldCheckpoint const & checkpoint : Checkpoints) {
		if (!Put_Section_Header(TAG_CKPT, Codec::DEFLATE, checkpoint.Body.size(), checkpoint.RawSize)
			|| !Put(checkpoint.Body)) {
			return(Fail());
		}
	}
	Checkpoints.clear();

	if (statistics != nullptr && !Put_Section(TAG_STAT, Codec::DEFLATE, statistics_body)) {
		return(Fail());
	}
	if (!Put_Section(TAG_END, Codec::STORED, summary_body) || !Output.Flush()) {
		return(Fail());
	}

	State = StateType::CLOSED;
	return(WriteResult::OK);
}

}
