/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

#pragma once

#include "crc.h"
#include "stage.h"

#include <algorithm>
#include <climits>


// The stage numbers and rate settings of an animation type. Where the type leaves Stages or
// LoopEnd at -1, the value comes from the frame count of the type's art.
struct AnimSequence
{
	int Stages = 0;
	int Start = 0;
	int LoopStart = 0;
	int LoopEnd = 0;
	bool IsPingPong = false;
	bool IsReverse = false;
	int Delay = 0;
	int RandomRateMin = 0;
	int RandomRateMax = 0;
	bool IsNormalized = false;
	int RandomLoopDelayMin = 0;
	int RandomLoopDelayMax = 0;
	int Loops = 0;
};


enum class AnimStageResult
{
	PLAYING,
	TURNED,		// A ping-pong animation reversed its direction.
	LOOPED,		// A loop ended with another to go, and the stage went back to the loop start.
	ENDED,		// The last frame of the last loop has been shown.
};


// The stage, rate, remaining loops and delay of a playing animation. The delay pauses play at
// the start or between loops.
struct AnimStageClass
{
	StageClass Playback;

	/*
	**	Is this animation in a temporary suspended state?  If so, then it won't
	**	be rendered until this value is zero. The flag will be set to false
	**	after the first countdown timer reaches 0.
	*/
	int Delay = 0;

	/*
	**	This counter tells how many more times the animation should loop before it
	**	terminates.
	*/
	unsigned char Loops = 1;

	/*
	**	If the animation has just been created, then don't do any animation
	**	processing until it has been through the render loop at least once.
	*/
	bool IsBrandNew = true;

	void Begin(AnimSequence const & seq, int rate, int loops, int delay);
	bool Skip_First_Pass(void);
	bool Wait(void);
	bool Advance(void);
	AnimStageResult Check_End(AnimSequence const & seq);
	void Chain(AnimSequence const & next, int rate, int loops);

	void Compute_CRC(CRCEngine & crc) const;

	template<typename S>
	void Serialize(S & stream)
	{
		stream.Serialize(Playback);
		stream.Serialize(Delay);
		stream.Serialize(Loops);
		stream.Serialize(IsBrandNew);
	}
};


/// <summary>
/// Starts play at the first stage, or at the loop end running backwards for a reverse
/// sequence. A loop count that truncates to zero plays once.
/// </summary>
inline void AnimStageClass::Begin(AnimSequence const & seq, int rate, int loops, int delay)
{
	Playback.Set_Rate(rate);
	Playback.Set_Stage(seq.IsReverse ? seq.LoopEnd : 0);
	Playback.Set_Step(seq.IsReverse ? -1 : 1);
	Loops = std::max<unsigned char>((unsigned char)loops, 1);
	Delay = delay;
}


/// <summary>
/// Returns true the first time it is called, so that the owner draws the first stage before
/// it advances.
/// </summary>
inline bool AnimStageClass::Skip_First_Pass(void)
{
	if (IsBrandNew) {
		IsBrandNew = false;
		return(true);
	}
	return(false);
}


/// <summary>
/// Counts the delay down by one while it is not zero. The delay pauses play before the first
/// stage or, after a loop, before the next one. A negative delay moves away from zero, so play
/// never resumes.
/// </summary>
/// <returns>Returns true on the call that brings the delay to zero; the owner then runs its
/// start step, such as playing the type's sound.</returns>
inline bool AnimStageClass::Wait(void)
{
	if (Delay == 0) {
		return(false);
	}
	Delay--;
	return(Delay == 0);
}


/// <summary>
/// Moves to the next stage when the rate timer has run out.
/// </summary>
/// <returns>Returns true when the stage changed; the owner then calls Check_End.</returns>
inline bool AnimStageClass::Advance(void)
{
	return(Playback.Graphic_Logic());
}


/// <summary>
/// Checks the stage that Advance reached against the end of the sequence. It reverses a
/// ping-pong sequence at either end, takes a loop off at the end of a loop, and returns to the
/// loop start while loops remain. A loop count of UCHAR_MAX never runs out.
/// </summary>
inline AnimStageResult AnimStageClass::Check_End(AnimSequence const & seq)
{
	int const stage = Playback.Fetch_Stage();

	if (seq.IsPingPong) {
		if ((Loops <= 1 && (stage >= seq.Stages || stage == 0)) || (Loops > 1 && (stage >= seq.LoopEnd - seq.Start || stage == seq.Start))) {
			Playback.Set_Step(-Playback.Fetch_Step());
			return(AnimStageResult::TURNED);
		}
	}

	/*
	**	Check to see if the last frame has been displayed. If so, then the
	**	animation either ends or loops.
	*/
	if ((Loops <= 1 && stage >= seq.Stages) || (Loops > 1 && stage >= seq.LoopEnd - seq.Start) || (seq.IsReverse && stage <= seq.Start)) {

		/*
		**	Determine if this animation should loop another time. If so, then start the loop
		**	but if not, then proceed into the animation termination handler.
		*/
		if (Loops != 0 && Loops != UCHAR_MAX) {
			Loops--;
		}
		if (Loops != 0) {
			Playback.Set_Stage(seq.IsReverse ? seq.LoopEnd : seq.LoopStart - seq.Start);
			return(AnimStageResult::LOOPED);
		}
		return(AnimStageResult::ENDED);
	}

	return(AnimStageResult::PLAYING);
}


/// <summary>
/// Restarts play at the start stage of the sequence chained to, keeping the direction of play.
/// </summary>
inline void AnimStageClass::Chain(AnimSequence const & next, int rate, int loops)
{
	Loops = (unsigned char)loops;
	Playback.Set_Rate(rate);
	Playback.Set_Stage(next.Start);
}


inline void AnimStageClass::Compute_CRC(CRCEngine & crc) const
{
	crc(Playback.Fetch_Stage());
	crc(Playback.Fetch_Rate());
	crc(Playback.Fetch_Step());
	crc(Delay);
	crc(Loops);
	crc(IsBrandNew);
}
