/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Checks how AnimStageClass steps an animation's stages: the first-pass skip, the delay before
// the first stage and between loops, loop counts, ping-pong, reverse play and chaining.
//
// Needs no game data.

#include "animstage.h"

#include "doctest/doctest.h"

#include <climits>
#include <utility>
#include <vector>

// The game frame counter that the stage timer reads. The harness advances it by hand.
int Frame = 0;

namespace {

using Trace = std::vector<std::pair<int, AnimStageResult>>;

constexpr AnimStageResult P = AnimStageResult::PLAYING;
constexpr AnimStageResult T = AnimStageResult::TURNED;
constexpr AnimStageResult L = AnimStageResult::LOOPED;
constexpr AnimStageResult E = AnimStageResult::ENDED;

AnimSequence Plain(int stages)
{
	AnimSequence seq;
	seq.Stages = stages;
	seq.LoopEnd = stages;
	seq.Loops = 1;
	return(seq);
}

// One game frame of the stage handling that AnimClass::AI and MoveFlashClass::AI share. A frame
// that does not reach Check_End reports PLAYING.
AnimStageResult Step(AnimStageClass & stage, AnimSequence const & seq)
{
	Frame++;
	if (stage.Skip_First_Pass()) {
		return(AnimStageResult::PLAYING);
	}
	if (stage.Delay != 0) {
		stage.Wait();
		return(AnimStageResult::PLAYING);
	}
	if (!stage.Advance()) {
		return(AnimStageResult::PLAYING);
	}
	return(stage.Check_End(seq));
}

// Steps past the first pass, then records the stage and result of each frame until the
// sequence ends or the frame limit is reached.
Trace Run(AnimStageClass & stage, AnimSequence const & seq, int frames)
{
	Step(stage, seq);
	Trace trace;
	for (int i = 0; i < frames; i++) {
		AnimStageResult const result = Step(stage, seq);
		trace.emplace_back(stage.Playback.Fetch_Stage(), result);
		if (result == AnimStageResult::ENDED) {
			break;
		}
	}
	return(trace);
}

}


TEST_CASE("A new animation shows its first stage for a frame before it advances")
{
	AnimSequence const seq = Plain(4);
	AnimStageClass stage;
	stage.Begin(seq, 1, 1, 0);
	CHECK(stage.Playback.Fetch_Stage() == 0);
	CHECK(stage.Playback.Fetch_Step() == 1);
	CHECK(stage.Playback.Fetch_Rate() == 1);
	CHECK(stage.Loops == 1);
	CHECK(stage.Delay == 0);
	CHECK(stage.IsBrandNew);

	CHECK(Step(stage, seq) == AnimStageResult::PLAYING);
	CHECK(!stage.IsBrandNew);
	CHECK(stage.Playback.Fetch_Stage() == 0);

	CHECK(Step(stage, seq) == AnimStageResult::PLAYING);
	CHECK(stage.Playback.Fetch_Stage() == 1);
}

TEST_CASE("A start delay counts down after the first pass, and play resumes on the next frame")
{
	AnimSequence const seq = Plain(4);
	AnimStageClass stage;
	stage.Begin(seq, 1, 1, 3);

	Frame++;
	CHECK(stage.Skip_First_Pass());
	CHECK(stage.Delay == 3);

	Frame++;
	CHECK(!stage.Wait());
	Frame++;
	CHECK(!stage.Wait());
	Frame++;
	CHECK(stage.Wait());
	CHECK(stage.Delay == 0);
	CHECK(stage.Playback.Fetch_Stage() == 0);

	// The rate timer ran during the delay, so the first frame after it advances at once.
	CHECK(Step(stage, seq) == AnimStageResult::PLAYING);
	CHECK(stage.Playback.Fetch_Stage() == 1);

	CHECK(!stage.Wait());
	CHECK(stage.Delay == 0);
}

TEST_CASE("A negative start delay never reaches zero")
{
	AnimSequence const seq = Plain(4);
	AnimStageClass stage;
	stage.Begin(seq, 1, 1, -1);
	CHECK(Step(stage, seq) == AnimStageResult::PLAYING);

	for (int i = 0; i < 100; i++) {
		Frame++;
		CHECK(!stage.Wait());
	}
	CHECK(stage.Delay == -101);
	CHECK(stage.Playback.Fetch_Stage() == 0);
}

TEST_CASE("A sequence played once ends on the frame its stage reaches the stage count")
{
	AnimSequence const seq = Plain(4);
	AnimStageClass stage;
	stage.Begin(seq, 1, 1, 0);
	CHECK(Run(stage, seq, 20) == Trace{{1, P}, {2, P}, {3, P}, {4, E}});
	CHECK(stage.Loops == 0);
}

TEST_CASE("A loop count that truncates to zero plays once")
{
	AnimSequence const seq = Plain(4);
	AnimStageClass stage;

	stage.Begin(seq, 1, 0, 0);
	CHECK(stage.Loops == 1);

	stage.Begin(seq, 1, 256, 0);
	CHECK(stage.Loops == 1);

	stage.Begin(seq, 1, 257, 0);
	CHECK(stage.Loops == 1);
}

TEST_CASE("Loops return to the loop start, and the last loop runs to the stage count")
{
	AnimSequence seq;
	seq.Start = 2;
	seq.LoopStart = 4;
	seq.LoopEnd = 7;
	seq.Stages = 9;
	AnimStageClass stage;
	stage.Begin(seq, 1, 3, 0);

	// Stages count from Start, so a loop ends at LoopEnd - Start and restarts at
	// LoopStart - Start.
	CHECK(Run(stage, seq, 50) == Trace{
		{1, P}, {2, P}, {3, P}, {4, P}, {2, L},
		{3, P}, {4, P}, {2, L},
		{3, P}, {4, P}, {5, P}, {6, P}, {7, P}, {8, P}, {9, E}});
}

TEST_CASE("A loop count of UCHAR_MAX never runs out until the count is cleared")
{
	AnimSequence const seq = Plain(3);
	AnimStageClass stage;
	stage.Begin(seq, 1, UCHAR_MAX, 0);
	Step(stage, seq);

	int loops = 0;
	for (int i = 0; i < 60; i++) {
		AnimStageResult const result = Step(stage, seq);
		REQUIRE(result != AnimStageResult::ENDED);
		if (result == AnimStageResult::LOOPED) {
			loops++;
			CHECK(stage.Playback.Fetch_Stage() == 0);
		}
	}
	CHECK(loops == 20);
	CHECK(stage.Loops == UCHAR_MAX);

	// Shortening an attached animation sets its count to zero; it then ends at the stage count.
	stage.Loops = 0;
	Trace trace;
	for (int i = 0; i < 10; i++) {
		AnimStageResult const result = Step(stage, seq);
		trace.emplace_back(stage.Playback.Fetch_Stage(), result);
		if (result == AnimStageResult::ENDED) {
			break;
		}
	}
	CHECK(trace == Trace{{1, P}, {2, P}, {3, E}});
	CHECK(stage.Loops == 0);
}

TEST_CASE("A ping-pong sequence played once turns at the stage count and at stage zero")
{
	AnimSequence seq = Plain(3);
	seq.IsPingPong = true;
	AnimStageClass stage;
	stage.Begin(seq, 1, 1, 0);

	CHECK(Run(stage, seq, 9) == Trace{
		{1, P}, {2, P}, {3, T},
		{2, P}, {1, P}, {0, T},
		{1, P}, {2, P}, {3, T}});
	CHECK(stage.Playback.Fetch_Step() == -1);
	CHECK(stage.Loops == 1);
}

TEST_CASE("A looping ping-pong sequence turns at the loop end and at the start stage")
{
	AnimSequence seq;
	seq.LoopEnd = 2;
	seq.Stages = 4;
	seq.IsPingPong = true;
	AnimStageClass stage;
	stage.Begin(seq, 1, 2, 0);

	CHECK(Run(stage, seq, 6) == Trace{{1, P}, {2, T}, {1, P}, {0, T}, {1, P}, {2, T}});
	CHECK(stage.Loops == 2);
}

TEST_CASE("A reverse sequence starts at the loop end and runs down to the start stage")
{
	AnimSequence seq = Plain(4);
	seq.IsReverse = true;
	AnimStageClass stage;
	stage.Begin(seq, 1, 2, 0);
	CHECK(stage.Playback.Fetch_Stage() == 4);
	CHECK(stage.Playback.Fetch_Step() == -1);

	CHECK(Run(stage, seq, 20) == Trace{
		{3, P}, {2, P}, {1, P}, {4, L},
		{3, P}, {2, P}, {1, P}, {0, E}});
}

TEST_CASE("A rate of two advances every other frame, and a rate of zero never advances")
{
	AnimSequence const seq = Plain(4);
	AnimStageClass stage;
	stage.Begin(seq, 2, 1, 0);
	CHECK(Run(stage, seq, 20) == Trace{
		{1, P}, {1, P}, {2, P}, {2, P}, {3, P}, {3, P}, {4, E}});

	AnimStageClass held;
	held.Begin(seq, 0, 1, 0);
	for (int i = 0; i < 10; i++) {
		CHECK(Step(held, seq) == AnimStageResult::PLAYING);
	}
	CHECK(held.Playback.Fetch_Stage() == 0);
}

TEST_CASE("Chaining restarts at the next sequence's start stage and keeps the direction of play")
{
	AnimSequence seq = Plain(4);
	seq.IsReverse = true;
	AnimStageClass stage;
	stage.Begin(seq, 1, 1, 0);
	REQUIRE(Run(stage, seq, 20).back().second == AnimStageResult::ENDED);

	AnimSequence next = Plain(6);
	next.Start = 1;
	next.Loops = 2;
	stage.Chain(next, 3, next.Loops);
	CHECK(stage.Playback.Fetch_Stage() == 1);
	CHECK(stage.Playback.Fetch_Rate() == 3);
	CHECK(stage.Playback.Fetch_Step() == -1);
	CHECK(stage.Loops == 2);
	CHECK(stage.Delay == 0);
	CHECK(!stage.IsBrandNew);

	// The new rate starts timing from the chain.
	CHECK(!stage.Advance());
	Frame++;
	CHECK(!stage.Advance());
	Frame++;
	CHECK(!stage.Advance());
	Frame++;
	CHECK(stage.Advance());
	CHECK(stage.Playback.Fetch_Stage() == 0);
}

TEST_CASE("Chaining keeps a loop count of zero, so the next end check ends the animation")
{
	AnimSequence const seq = Plain(2);
	AnimStageClass stage;
	stage.Begin(seq, 1, 1, 0);
	REQUIRE(Run(stage, seq, 20).back().second == AnimStageResult::ENDED);

	AnimSequence next = Plain(2);
	next.Loops = 0;
	stage.Chain(next, 1, next.Loops);
	CHECK(stage.Loops == 0);
	CHECK(stage.Playback.Fetch_Stage() == 0);

	Trace trace;
	for (int i = 0; i < 10; i++) {
		AnimStageResult const result = Step(stage, next);
		trace.emplace_back(stage.Playback.Fetch_Stage(), result);
		if (result == AnimStageResult::ENDED) {
			break;
		}
	}
	CHECK(trace == Trace{{1, P}, {2, E}});
}

TEST_CASE("A loop count of -1 loops without end, from the start and through a chain")
{
	AnimSequence const seq = Plain(3);
	AnimStageClass stage;
	stage.Begin(seq, 1, -1, 0);
	CHECK(stage.Loops == UCHAR_MAX);

	stage.Chain(seq, 1, -1);
	CHECK(stage.Loops == UCHAR_MAX);
}

TEST_CASE("A ping-pong sequence played once turns at stage zero, not at the start stage")
{
	AnimSequence seq = Plain(5);
	seq.Start = 2;
	seq.LoopEnd = 6;
	seq.IsPingPong = true;
	AnimStageClass stage;
	stage.Begin(seq, 1, 1, 0);

	CHECK(Run(stage, seq, 11) == Trace{
		{1, P}, {2, P}, {3, P}, {4, P}, {5, T},
		{4, P}, {3, P}, {2, P}, {1, P}, {0, T},
		{1, P}});
}

TEST_CASE("A looping ping-pong sequence turns at the start stage, not at stage zero")
{
	AnimSequence seq = Plain(8);
	seq.Start = 2;
	seq.LoopEnd = 6;
	seq.IsPingPong = true;
	AnimStageClass stage;
	stage.Begin(seq, 1, 2, 0);

	// Play starts below the start stage, so the first turn comes on the way up and the stage
	// then runs on past zero.
	CHECK(Run(stage, seq, 4) == Trace{{1, P}, {2, T}, {1, P}, {0, P}});

	// A chain starts at the start stage, and the sequence then turns at LoopEnd - Start and at
	// the start stage.
	AnimStageClass chained;
	chained.Begin(seq, 1, 2, 0);
	Step(chained, seq);
	chained.Chain(seq, 1, 2);
	Trace trace;
	for (int i = 0; i < 6; i++) {
		AnimStageResult const result = Step(chained, seq);
		trace.emplace_back(chained.Playback.Fetch_Stage(), result);
	}
	CHECK(trace == Trace{{3, P}, {4, T}, {3, P}, {2, T}, {3, P}, {4, T}});
	CHECK(chained.Loops == 2);
}

TEST_CASE("A looping reverse sequence restarts at the loop end and ends at the start stage")
{
	AnimSequence seq = Plain(8);
	seq.Start = 2;
	seq.LoopEnd = 6;
	seq.IsReverse = true;
	AnimStageClass stage;
	stage.Begin(seq, 1, 2, 0);
	CHECK(stage.Playback.Fetch_Stage() == 6);

	// While more than one loop remains, any stage at or above LoopEnd - Start ends a loop, so
	// the first frame already loops back to LoopEnd.
	CHECK(Run(stage, seq, 20) == Trace{{6, L}, {5, P}, {4, P}, {3, P}, {2, E}});
}

TEST_CASE("A reverse sequence played once ends at the start stage")
{
	AnimSequence seq = Plain(8);
	seq.Start = 2;
	seq.LoopEnd = 6;
	seq.IsReverse = true;
	AnimStageClass stage;
	stage.Begin(seq, 1, 1, 0);

	CHECK(Run(stage, seq, 20) == Trace{{5, P}, {4, P}, {3, P}, {2, E}});
	CHECK(stage.Loops == 0);
}

TEST_CASE("A delay set between loops pauses play, and the frame it runs out restarts the loop")
{
	AnimSequence const seq = Plain(3);
	AnimStageClass stage;
	stage.Begin(seq, 1, 2, 0);
	Step(stage, seq);
	CHECK(Step(stage, seq) == AnimStageResult::PLAYING);
	CHECK(Step(stage, seq) == AnimStageResult::PLAYING);
	REQUIRE(Step(stage, seq) == AnimStageResult::LOOPED);
	CHECK(stage.Playback.Fetch_Stage() == 0);

	// The owner sets the delay from RandomLoopDelay when a loop ends.
	stage.Delay = 2;
	Frame++;
	CHECK(!stage.Wait());
	CHECK(stage.Playback.Fetch_Stage() == 0);
	Frame++;
	CHECK(stage.Wait());
	CHECK(stage.Playback.Fetch_Stage() == 0);

	Trace trace;
	for (int i = 0; i < 10; i++) {
		AnimStageResult const result = Step(stage, seq);
		trace.emplace_back(stage.Playback.Fetch_Stage(), result);
		if (result == AnimStageResult::ENDED) {
			break;
		}
	}
	CHECK(trace == Trace{{1, P}, {2, P}, {3, E}});
}

TEST_CASE("A sequence with no stages ends at its first stage advance")
{
	AnimSequence seq;
	seq.Loops = 1;
	AnimStageClass stage;
	stage.Begin(seq, 1, 1, 0);
	CHECK(Run(stage, seq, 10) == Trace{{1, E}});
}

TEST_CASE("A looping sequence with no stages takes one loop per stage advance, then ends")
{
	AnimSequence seq;
	seq.Loops = 3;
	AnimStageClass stage;
	stage.Begin(seq, 1, 3, 0);
	CHECK(Run(stage, seq, 10) == Trace{{0, L}, {0, L}, {1, E}});
}

TEST_CASE("A reverse sequence with no stages ends at its first stage advance")
{
	AnimSequence seq;
	seq.Loops = 1;
	seq.IsReverse = true;
	AnimStageClass stage;
	stage.Begin(seq, 1, 1, 0);
	CHECK(Run(stage, seq, 10) == Trace{{-1, E}});
}

TEST_CASE("A ping-pong sequence with no stages turns at every stage advance and does not end")
{
	AnimSequence seq;
	seq.Loops = 1;
	seq.IsPingPong = true;
	AnimStageClass stage;
	stage.Begin(seq, 1, 1, 0);
	Trace const trace = Run(stage, seq, 6);
	CHECK(trace == Trace{{1, T}, {0, T}, {1, T}, {0, T}, {1, T}, {0, T}});
}
