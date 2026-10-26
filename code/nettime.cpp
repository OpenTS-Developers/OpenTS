/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "nettime.h"

#if defined(_WIN32)
#include <windows.h>
#include <mmsystem.h>
#else
#include <chrono>
#endif


namespace NetTiming
{
	namespace
	{
		class SystemMillisecondClock final : public MillisecondClock
		{
			public:
				Milliseconds Now(void) const override;
		};
	}


	/// <summary>Reads the system's wrapping millisecond clock.</summary>
	Milliseconds SystemMillisecondClock::Now(void) const
	{
#if defined(_WIN32)
		return(static_cast<Milliseconds>(::timeGetTime()));
#else
		auto const now = std::chrono::steady_clock::now().time_since_epoch();
		auto const milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
		return(static_cast<Milliseconds>(milliseconds));
#endif
	}


	/// <summary>Returns the process-wide network clock.</summary>
	MillisecondClock const & Default_Clock(void)
	{
		static SystemMillisecondClock clock;
		return(clock);
	}
}
