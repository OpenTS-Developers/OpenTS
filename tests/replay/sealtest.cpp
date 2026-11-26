/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "replay/replayseal.h"

#include "replaytest.h"

#include "doctest/doctest.h"

using namespace Replay;


// The expected digests were computed with Python's hashlib from the byte layouts the format
// gives, independently of this code.
namespace {

char const * const S0_HEX = "21838304e50e4e04246590d61601c33ff102289e3a65120c78b470e22d6c9d91";
char const * const S1_HEX = "75e67c9ce5215f554e34a5b7e643390d6cfface875ac4c43104c3c77911adbce";
char const * const S2_HEX = "69e69d097adb22d5696a99050980caf479f3d7bc28a222de619890ca98edd532";
char const * const S3_HEX = "cdce0b0a4aa6ae0791ff3a89bd54edcfcd86ff0c2dc93fda5105a30b297e2dc9";

Digest Sample_Start(void)
{
	ByteVector const launch = From_Hex("08021500000000");
	ByteVector const init = Bytes_Of("init-state");
	return(Seal_Start(1, "opents", 3, launch, init));
}


Message::Event Make_Event(std::uint32_t type, std::uint32_t house, std::int32_t due, char const * payload, bool local = false)
{
	Message::Event event;
	event.Type = type;
	event.House = house;
	event.Due = due;
	event.Payload = From_Hex(payload);
	event.Local = local;
	return(event);
}


// Records of a game whose frames 0, 5, 899, and 2000 have canonical bytes and whose frame 6 has
// none. Nothing happens between 900 and 1799.
std::vector<std::pair<std::uint32_t, Message::Frame>> Sample_Records(void)
{
	std::vector<std::pair<std::uint32_t, Message::Frame>> records(5);

	records[0].first = 0;
	records[0].second.Hash = 0xDEADBEEF;

	records[1].first = 5;
	records[1].second.Events.push_back(Make_Event(7, 1, 2, "010203"));
	records[1].second.Events.push_back(Make_Event(9, 1, 0, "ff", true));
	Message::Event sealed = Make_Event(8, 2, -1, "aa");
	sealed.SealedPayload = From_Hex("bbcc");
	sealed.Role = EventRole::EVENT_ROLE_CHAT_PROOF;
	records[1].second.Events.push_back(sealed);

	records[2].first = 6;
	records[2].second.Events.push_back(Make_Event(9, 1, 0, "ff", true));
	records[2].second.View.Camera = Message::Point();

	records[3].first = 899;
	records[3].second.Defeated = { 2, 3 };

	records[4].first = 2000;
	records[4].second.Hash = 1;
	return(records);
}

}


TEST_CASE("S0 covers the seal version, the engine, the data version, LNCH, and INIT")
{
	CHECK(Hex(Sample_Start()) == S0_HEX);
	CHECK(Hex(Seal_Start(1, "", 0, ByteSpan(), ByteSpan()))
		== "23aa66527edbdb6fa360143c56517ba1a63f2b6cb782f578ec9fb8759c29293f");

	ByteVector const launch = From_Hex("08021500000000");
	ByteVector const init = Bytes_Of("init-state");
	CHECK(Seal_Start(2, "opents", 3, launch, init) != Sample_Start());
	CHECK(Seal_Start(1, "openTS", 3, launch, init) != Sample_Start());
	CHECK(Seal_Start(1, "opents", 4, launch, init) != Sample_Start());
	CHECK(Seal_Start(1, "opents", 3, init, launch) != Sample_Start());
}


TEST_CASE("canonical frame bytes cover the hash, shared events by their sealed bytes, and defeats")
{
	auto const records = Sample_Records();
	Replay::Bytes bytes;
	REQUIRE(Canonical_Frame_Bytes(5, records[1].second, bytes));
	CHECK(Hex(bytes) ==
		"05000000" "00"
		"02000000"
		"07000000" "01000000" "02000000" "03000000" "010203"
		"08000000" "02000000" "ffffffff" "02000000" "bbcc"
		"00000000");

	bytes.clear();
	REQUIRE(Canonical_Frame_Bytes(0, records[0].second, bytes));
	CHECK(Hex(bytes) == "00000000" "01" "efbeadde" "00000000" "00000000");

	bytes.clear();
	Message::Frame unsorted;
	unsorted.Defeated = { 3, 2 };
	REQUIRE(Canonical_Frame_Bytes(899, unsorted, bytes));
	CHECK(Hex(bytes) == "83030000" "00" "00000000" "02000000" "02000000" "03000000");

	bytes.clear();
	CHECK_MESSAGE(!Canonical_Frame_Bytes(6, records[2].second, bytes), "local events and the view are not sealed");
	CHECK(bytes.empty());
}


TEST_CASE("a chain of three intervals with an empty one and a final seal on a boundary")
{
	auto const records = Sample_Records();
	SealChain chain(Sample_Start());
	std::vector<Message::Seal> all;
	for (auto const & [frame, record] : records) {
		std::vector<Message::Seal> closed;
		chain.Add(frame, record, closed);
		if (frame < 2000) {
			CHECK_MESSAGE(closed.empty(), "no interval has ended before frame ", frame);
		} else {
			REQUIRE(closed.size() == 2);
			CHECK(closed[0].Through == 899);
			CHECK(Hex(closed[0].Hash) == S1_HEX);
			CHECK(closed[1].Through == 1799);
			CHECK_MESSAGE(Hex(closed[1].Hash) == S2_HEX, "an empty interval seals the previous seal alone");
		}
		all.insert(all.end(), closed.begin(), closed.end());
	}

	std::vector<Message::Seal> ending;
	chain.Finish(2699, ending);
	REQUIRE_MESSAGE(ending.size() == 1, "the interval seal is the final seal, with no second one");
	CHECK(ending[0].Through == 2699);
	CHECK(Hex(ending[0].Hash) == S3_HEX);
}


TEST_CASE("a final seal inside an interval covers part of it and closes the ones before")
{
	auto const records = Sample_Records();
	SealChain chain(Sample_Start());
	for (std::size_t i = 0; i < 4; i++) {
		std::vector<Message::Seal> closed;
		chain.Add(records[i].first, records[i].second, closed);
	}

	std::vector<Message::Seal> seals;
	chain.Finish(1000, seals);
	REQUIRE(seals.size() == 2);
	CHECK(seals[0].Through == 899);
	CHECK(Hex(seals[0].Hash) == S1_HEX);
	CHECK(seals[1].Through == 1000);
	CHECK_MESSAGE(Hex(seals[1].Hash) == Hex(SHA256Class::Digest(From_Hex(S1_HEX))),
		"frames 900 to 1000 have no canonical bytes");

	SealChain stopped_early(Sample_Start());
	std::vector<Message::Seal> early;
	std::vector<Message::Seal> closed;
	stopped_early.Add(0, records[0].second, closed);
	stopped_early.Finish(0, early);
	REQUIRE(early.size() == 1);
	CHECK(early[0].Through == 0);
}


TEST_CASE("two copies that stop on different frames agree on every interval seal")
{
	auto const records = Sample_Records();
	auto run = [&](std::uint32_t last_frame) {
		SealChain chain(Sample_Start());
		std::vector<Message::Seal> seals;
		for (auto const & [frame, record] : records) {
			if (frame <= last_frame) {
				chain.Add(frame, record, seals);
			}
		}
		chain.Finish(last_frame, seals);
		return(seals);
	};

	std::vector<Message::Seal> const longer = run(2500);
	std::vector<Message::Seal> const shorter = run(1900);
	REQUIRE(longer.size() == 3);
	REQUIRE(shorter.size() == 3);
	CHECK(longer[0].Hash == shorter[0].Hash);
	CHECK(longer[1].Hash == shorter[1].Hash);
	CHECK(longer[2].Through == 2500);
	CHECK(shorter[2].Through == 1900);
	CHECK(longer[2].Hash != shorter[2].Hash);
}


TEST_CASE("the verifier accepts the chain a writer emits and finds the first altered seal")
{
	auto records = Sample_Records();
	SealChain chain(Sample_Start());
	for (auto & [frame, record] : records) {
		chain.Add(frame, record, record.Seals);
	}
	Message::RecordingSummary summary;
	summary.LastFrame = 2699;
	for (auto const & [frame, record] : records) {
		summary.Seals.insert(summary.Seals.end(), record.Seals.begin(), record.Seals.end());
	}
	chain.Finish(2699, summary.Seals);
	REQUIRE(summary.Seals.size() == 3);

	auto verify = [&](std::vector<std::pair<std::uint32_t, Message::Frame>> const & file, Message::RecordingSummary const & end) {
		SealVerifier verifier(Sample_Start());
		for (auto const & [frame, record] : file) {
			verifier.Add(frame, record);
		}
		verifier.Finish(end);
		return(verifier);
	};

	SealVerifier const good = verify(records, summary);
	CHECK(good.Good());
	CHECK(good.Compared() == 5);

	auto altered = records;
	altered[1].second.Events[0].Payload[0] ^= 1;
	SealVerifier const bad = verify(altered, summary);
	CHECK(!bad.Good());
	CHECK(bad.First_Mismatch() == 899u);

	auto role_changed = records;
	role_changed[1].second.Events[0].Role = EventRole::EVENT_ROLE_CHAT_PROOF;
	role_changed[2].second.Events[0].Payload[0] ^= 1;
	CHECK_MESSAGE(verify(role_changed, summary).Good(), "role and local events are not covered");

	Message::RecordingSummary short_summary = summary;
	short_summary.Seals.pop_back();
	SealVerifier const lacking = verify(records, short_summary);
	CHECK(!lacking.Good());
	CHECK_MESSAGE(lacking.First_Misplaced() == 2699u, "a summary that lacks a seal is malformed");
	short_summary.Seals.erase(short_summary.Seals.begin());
	CHECK(verify(records, short_summary).First_Misplaced() == 899u);

	Message::RecordingSummary no_frames;
	no_frames.Seals.push_back(summary.Seals[0]);
	CHECK_MESSAGE(!verify({}, no_frames).Good(), "a recording with no frame has no seals");
	CHECK(verify({}, Message::RecordingSummary()).Good());
}


TEST_CASE("the verifier requires each interval seal on the first record after its interval")
{
	auto records = Sample_Records();
	SealChain chain(Sample_Start());
	for (auto & [frame, record] : records) {
		chain.Add(frame, record, record.Seals);
	}
	REQUIRE(records[4].second.Seals.size() == 2);
	Message::RecordingSummary summary;
	summary.LastFrame = 2699;
	summary.Seals = records[4].second.Seals;
	chain.Finish(2699, summary.Seals);

	auto verify = [&](std::vector<std::pair<std::uint32_t, Message::Frame>> const & file) {
		SealVerifier verifier(Sample_Start());
		for (auto const & [frame, record] : file) {
			verifier.Add(frame, record);
		}
		verifier.Finish(summary);
		return(verifier);
	};
	REQUIRE(verify(records).Good());
	CHECK(!verify(records).First_Misplaced().has_value());

	auto missing = records;
	missing[4].second.Seals.erase(missing[4].second.Seals.begin());
	SealVerifier const without = verify(missing);
	CHECK(!without.Good());
	CHECK(without.First_Misplaced() == 899u);
	CHECK_MESSAGE(!without.First_Mismatch().has_value(), "every seal present still matches");

	auto reversed = records;
	std::swap(reversed[4].second.Seals[0], reversed[4].second.Seals[1]);
	CHECK(verify(reversed).First_Misplaced() == 899u);

	auto later = records;
	later[4].second.Seals.clear();
	later.push_back({ 2100, Message::Frame() });
	later.back().second.Seals = records[4].second.Seals;
	SealVerifier const moved = verify(later);
	CHECK(moved.First_Misplaced() == 899u);
	CHECK_MESSAGE(!moved.First_Mismatch().has_value(), "the moved seals hold the right values");

	auto final_in_record = records;
	final_in_record[4].second.Seals.push_back(summary.Seals.back());
	CHECK_MESSAGE(verify(final_in_record).First_Misplaced() == 2699u, "the final seal belongs only in the summary");
}


TEST_CASE("the chain counts the intervals that end before a frame")
{
	SealChain chain(Sample_Start());
	CHECK(chain.Intervals_Before(0) == 0);
	CHECK(chain.Intervals_Before(899) == 0);
	CHECK(chain.Intervals_Before(900) == 1);
	CHECK(chain.Intervals_Before(1799) == 1);
	CHECK(chain.Intervals_Before(1800) == 2);
	CHECK(chain.Intervals_Before(0xFFFFFFFFu) == 4772185);

	std::vector<Message::Seal> closed;
	chain.Add(1000, Message::Frame(), closed);
	CHECK(closed.size() == 1);
	CHECK(chain.Interval_End() == 1799);
	CHECK(chain.Intervals_Before(1000) == 0);
	CHECK(chain.Intervals_Before(2700) == 2);
}


TEST_CASE("a forged frame number finds the missing seals without building the chain")
{
	Message::Frame record;
	record.Hash = 1;

	SealVerifier late(Sample_Start());
	late.Add(0xFFFFFFFFu, record);
	CHECK(late.First_Misplaced() == 899u);
	Message::RecordingSummary summary;
	summary.LastFrame = 0xFFFFFFFFu;
	late.Finish(summary);
	CHECK(late.First_Misplaced() == 899u);
	CHECK(!late.First_Mismatch().has_value());

	// The record has the first two seals; the third is the first one missing.
	SealChain chain(Sample_Start());
	std::vector<Message::Seal> seals;
	chain.Add(2000, Message::Frame(), seals);
	REQUIRE(seals.size() == 2);
	record.Seals = seals;
	SealVerifier partial(Sample_Start());
	partial.Add(0xFFFFFFFFu, record);
	CHECK(partial.First_Misplaced() == 2699u);

	SealVerifier unrecorded(Sample_Start());
	unrecorded.Finish(summary);
	CHECK(unrecorded.First_Misplaced() == 899u);
}
