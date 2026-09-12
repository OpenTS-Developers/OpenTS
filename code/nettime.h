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


namespace NetTiming
{
	using Milliseconds = std::uint32_t;

	class MillisecondClock
	{
		public:
			virtual ~MillisecondClock() = default;
			virtual Milliseconds Now(void) const = 0;
	};

	MillisecondClock const & Default_Clock(void);

	constexpr Milliseconds Elapsed_Milliseconds(Milliseconds start, Milliseconds finish)
	{
		return(finish - start);
	}

	constexpr bool Milliseconds_Have_Elapsed(Milliseconds start, Milliseconds now, Milliseconds duration)
	{
		return(Elapsed_Milliseconds(start, now) >= duration);
	}
}
