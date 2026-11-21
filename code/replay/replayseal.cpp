/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "replay/replayseal.h"

#include <algorithm>


namespace Replay
{

namespace
{

void Hash_U32(SHA256Class & hash, std::uint32_t value)
{
	std::uint8_t bytes[4] = {
		std::uint8_t(value), std::uint8_t(value >> 8), std::uint8_t(value >> 16), std::uint8_t(value >> 24)
	};
	hash.Hash(bytes, sizeof(bytes));
}


// Returns the through of the first seal that differs between carried and the expected
// throughs, which outnumber carried.
template<typename Expected>
std::uint32_t First_Missing(std::vector<Message::Seal> const & carried, Expected expected)
{
	for (std::size_t i = 0; i < carried.size(); i++) {
		std::uint32_t const through = expected(i);
		if (carried[i].Through != through) {
			return(std::min(carried[i].Through, through));
		}
	}
	return(expected(carried.size()));
}

}


Digest Seal_Start(std::uint32_t seal_version, std::string_view engine, std::uint32_t data_version,
	ByteSpan launch, ByteSpan init)
{
	SHA256Class hash;
	hash.Hash("CNCR", 4);
	Hash_U32(hash, seal_version);
	Hash_U32(hash, std::uint32_t(engine.size()));
	hash.Hash(engine.data(), engine.size());
	Hash_U32(hash, data_version);
	Digest const launch_digest = SHA256Class::Digest(launch);
	Digest const init_digest = SHA256Class::Digest(init);
	hash.Hash(launch_digest.data(), launch_digest.size());
	hash.Hash(init_digest.data(), init_digest.size());
	return(hash.Result());
}


bool Canonical_Frame_Bytes(std::uint32_t frame, Message::Frame const & record, Bytes & out)
{
	std::uint32_t shared = 0;
	for (Message::Event const & event : record.Events) {
		if (!event.Local) {
			shared++;
		}
	}
	if (!record.Hash && shared == 0 && record.Defeated.empty()) {
		return(false);
	}

	Put_Fixed32(out, frame);
	out.push_back(record.Hash ? 1 : 0);
	if (record.Hash) {
		Put_Fixed32(out, *record.Hash);
	}

	Put_Fixed32(out, shared);
	for (Message::Event const & event : record.Events) {
		if (event.Local) {
			continue;
		}
		Bytes const & sealed = event.SealedPayload ? *event.SealedPayload : event.Payload;
		Put_Fixed32(out, event.Type);
		Put_Fixed32(out, event.House);
		Put_Fixed32(out, std::uint32_t(event.Due));
		Put_Fixed32(out, std::uint32_t(sealed.size()));
		out.insert(out.end(), sealed.begin(), sealed.end());
	}

	std::vector<std::uint32_t> defeated = record.Defeated;
	std::sort(defeated.begin(), defeated.end());
	Put_Fixed32(out, std::uint32_t(defeated.size()));
	for (std::uint32_t house : defeated) {
		Put_Fixed32(out, house);
	}
	return(true);
}


SealChain::SealChain(Digest const & start) :
	IntervalEnd(SEAL_INTERVAL - 1)
{
	Hash.Hash(start.data(), start.size());
}


void SealChain::Close_Interval(std::vector<Message::Seal> & seals)
{
	Digest const digest = Hash.Result();
	Message::Seal seal;
	seal.Through = std::uint32_t(IntervalEnd);
	seal.Hash.assign(digest.begin(), digest.end());
	seals.push_back(seal);

	Hash.Reset();
	Hash.Hash(digest.data(), digest.size());
	IntervalEnd += SEAL_INTERVAL;
}


void SealChain::Add(std::uint32_t frame, Message::Frame const & record, std::vector<Message::Seal> & closed)
{
	while (frame > IntervalEnd) {
		Close_Interval(closed);
	}

	Bytes canonical;
	if (Canonical_Frame_Bytes(frame, record, canonical)) {
		Hash.Hash(canonical);
	}
}


void SealChain::Finish(std::uint32_t last_frame, std::vector<Message::Seal> & seals)
{
	while (IntervalEnd < last_frame) {
		Close_Interval(seals);
	}

	Digest const digest = Hash.Result();
	Message::Seal seal;
	seal.Through = last_frame;
	seal.Hash.assign(digest.begin(), digest.end());
	seals.push_back(seal);
}


/// <summary>
/// Returns the number of intervals that end before frame and are not yet sealed: the number of
/// seals Add appends for a record on frame.
/// </summary>
std::uint64_t SealChain::Intervals_Before(std::uint32_t frame) const
{
	if (frame <= IntervalEnd) {
		return(0);
	}
	return((frame - 1 - IntervalEnd) / SEAL_INTERVAL + 1);
}


SealVerifier::SealVerifier(Digest const & start) :
	Chain(start),
	Matched(0),
	Stopped(false)
{
}


void SealVerifier::Compare(Message::Seal const & seal)
{
	auto computed = Computed.find(seal.Through);
	if (computed == Computed.end() || computed->second != seal.Hash) {
		if (!Mismatch) {
			Mismatch = seal.Through;
		}
		return;
	}
	Matched++;
}


void SealVerifier::Add(std::uint32_t frame, Message::Frame const & record)
{
	if (Stopped) {
		return;
	}

	// A record with fewer seals than the intervals that end before it lacks one, and those
	// intervals are not sealed, since a forged frame number can end millions of them.
	if (Chain.Intervals_Before(frame) > record.Seals.size()) {
		if (!Misplaced) {
			std::uint64_t const first = Chain.Interval_End();
			Misplaced = First_Missing(record.Seals, [&](std::size_t i) {
				return(std::uint32_t(first + std::uint64_t(i) * SEAL_INTERVAL));
			});
		}
		Stopped = true;
		return;
	}

	std::vector<Message::Seal> closed;
	Chain.Add(frame, record, closed);
	for (Message::Seal const & seal : closed) {
		Computed[seal.Through] = seal.Hash;
	}

	if (!Misplaced) {
		std::size_t const common = std::min(closed.size(), record.Seals.size());
		std::size_t i = 0;
		while (i < common && record.Seals[i].Through == closed[i].Through) {
			i++;
		}
		if (i < record.Seals.size() && i < closed.size()) {
			Misplaced = std::min(record.Seals[i].Through, closed[i].Through);
		} else if (i < record.Seals.size()) {
			Misplaced = record.Seals[i].Through;
		} else if (i < closed.size()) {
			Misplaced = closed[i].Through;
		}
	}

	for (Message::Seal const & seal : record.Seals) {
		Compare(seal);
	}
	LastFrame = frame;
}


void SealVerifier::Finish(Message::RecordingSummary const & summary)
{
	if (Stopped) {
		return;
	}
	if (!summary.LastFrame) {
		if (!summary.Seals.empty() && !Mismatch) {
			Mismatch = summary.Seals.front().Through;
		}
		return;
	}

	// A last frame before the last record cannot have sealed that record.
	if (LastFrame && *summary.LastFrame < *LastFrame) {
		if (!Mismatch) {
			Mismatch = *summary.LastFrame;
		}
		return;
	}

	std::vector<std::uint32_t> expected;
	for (auto const & [through, hash] : Computed) {
		expected.push_back(through);
	}

	// The summary lists every seal, the final one included, so one with fewer lacks a seal.
	std::uint32_t const last_frame = *summary.LastFrame;
	std::uint64_t const ending = Chain.Intervals_Before(last_frame);
	if (expected.size() + ending + 1 > summary.Seals.size()) {
		if (!Misplaced) {
			std::uint64_t const first = Chain.Interval_End();
			Misplaced = First_Missing(summary.Seals, [&](std::size_t i) {
				if (i < expected.size()) {
					return(expected[i]);
				}
				std::uint64_t const later = i - expected.size();
				return((later < ending) ? std::uint32_t(first + later * SEAL_INTERVAL) : last_frame);
			});
		}
		return;
	}

	std::vector<Message::Seal> seals;
	Chain.Finish(last_frame, seals);
	for (Message::Seal const & seal : seals) {
		Computed[seal.Through] = seal.Hash;
		expected.push_back(seal.Through);
	}
	for (std::size_t i = 0; i < summary.Seals.size(); i++) {
		if (i >= expected.size() || summary.Seals[i].Through != expected[i]) {
			if (!Mismatch) {
				Mismatch = summary.Seals[i].Through;
			}
			return;
		}
		Compare(summary.Seals[i]);
	}
}

}
