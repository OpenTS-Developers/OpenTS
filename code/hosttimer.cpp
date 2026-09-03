/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The Windows side of hosttimer.h, over the multimedia timer.

#ifdef _WIN32

#include "always.h"

#include "hosttimer.h"

#include "win.h"


// The multimedia timer calls back on a thread of its own, and its accuracy follows the
// timer resolution in force, so the finest one is held for as long as a timer is armed.
// TIME_KILL_SYNCHRONOUS is what makes the disarm hosttimer.h promises: without it
// timeKillEvent returns while a callback may still be running. timeSetEvent wants a
// callback shape of its own, so each armed timer carries the portable one it stands for,
// and the resolution is released on the failure path as well as the ordinary one.
namespace {

struct ArmedTimerType
{
	MMRESULT Id;
	HostTimerCallbackType Callback;
	void * User;
};

ArmedTimerType _ArmedTimers[8];


void CALLBACK Deliver(UINT id, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR)
{
	for (ArmedTimerType const & armed : _ArmedTimers) {
		if (armed.Id == id && armed.Callback != nullptr) {
			armed.Callback((uint32_t)armed.Id, armed.User);
			return;
		}
	}
}

}


uint32_t Host_Timer_Arm(uint32_t period, HostTimerCallbackType callback, void * user)
{
	if (callback == nullptr || period == 0) {
		return(0);
	}

	for (ArmedTimerType & armed : _ArmedTimers) {
		if (armed.Id != 0) continue;

		armed.Callback = callback;
		armed.User = user;

		timeBeginPeriod(1);
		armed.Id = timeSetEvent((UINT)period, 1, Deliver, 0, TIME_PERIODIC | TIME_KILL_SYNCHRONOUS);

		if (armed.Id == 0) {
			timeEndPeriod(1);
			armed.Callback = nullptr;
			return(0);
		}
		return((uint32_t)armed.Id);
	}

	return(0);
}


void Host_Timer_Disarm(uint32_t timer)
{
	if (timer == 0) return;

	for (ArmedTimerType & armed : _ArmedTimers) {
		if (armed.Id != (MMRESULT)timer) continue;

		timeKillEvent(armed.Id);
		timeEndPeriod(1);
		armed.Id = 0;
		armed.Callback = nullptr;
		return;
	}
}

#endif
