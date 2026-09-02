/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

/// High-resolution millisecond timer system.
class MillisecondTimerClass
{
	public:
		MillisecondTimerClass(void);
		~MillisecondTimerClass(void);

		operator double () const;

	private:
		/*
		 * This is the number of processor clock cycles that pass in one millisecond, and
		 * the raw cycle count is divided by it to yield a time. If it is 1.0, then the
		 * processor would not report its rate and the host clock is read instead.
		 */
		double Frequency;
};
