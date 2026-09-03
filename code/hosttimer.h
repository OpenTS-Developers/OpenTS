/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstdint>

// The routine a host timer runs, handed its own handle and the value it was armed with.
// The handle lets a callback tell its own delivery from one left over from a timer that
// has since been disarmed.
using HostTimerCallbackType = void (*)(uint32_t timer, void * user);

/*
 * Arms a callback to run every period milliseconds until it is disarmed, and answers with
 * a handle for it. Zero means the host would not arm one.
 *
 * The callback may arrive on a thread of the host's own, so it has to be safe to run while
 * the engine thread is somewhere else. A host with no thread to spare delivers it from
 * inside Host_Wait instead, which is the other half of the same contract: a caller that
 * never waits never gets a callback.
 */
uint32_t Host_Timer_Arm(uint32_t period, HostTimerCallbackType callback, void * user);

/*
 * Disarms a timer, and does not return while a callback of its own is still running. That
 * is what lets a caller tear down state the callback reads without racing it. The one
 * exception is a callback disarming its own timer, which is allowed and returns at once.
 */
void Host_Timer_Disarm(uint32_t timer);
