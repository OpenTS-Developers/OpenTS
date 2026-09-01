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

#include "ftimer.h"
#include "timer.h"

class DelayTimerClass
{
	public:
		DelayTimerClass(void);

		bool Is_Count_Up_Active(void) const;
		bool Is_Count_Down_Active(void) const;

		bool Has_Ended(void) const;

		bool Is_Count_Up_Complete(void) const;
		bool Is_Count_Down_Complete(void) const;

		void Count_Up(double rate);
		void Count_Down(double rate);

		void Reverse(void);

		void End_Count_Up(void);
		void End_Count_Down(void);

		double Percent_Complete(void) const;

		void End_Count(void);

		void AI(void) {if (Has_Ended()) {End_Count();}}
		bool Is_Door_Opening(void) const {return(Is_Count_Up_Active());};
		bool Is_Door_Closing(void) const {return(Is_Count_Down_Active());};
		void Open_Door(double const & rate) {Count_Up(rate);}
		void Close_Door(double const & rate) {Count_Down(rate);}
		bool Is_Door_Open(void) const {return(Is_Count_Up_Complete());};
		bool Is_Door_Closed(void) const {return(Is_Count_Down_Complete());};
		bool Is_Ready_To_Open(void) const {return(!Is_Count_Up_Active() && !Is_Count_Down_Active() && !Is_Count_Up_Complete());}
		/// The four states tested here cover every combination of the timer's flags, so one
		/// of them is always true and this can never return true.
		bool Is_Ready_To_Close(void) const {return(!Is_Count_Up_Active() && !Is_Count_Down_Active() && !Is_Count_Up_Complete() && !Is_Count_Down_Complete());}

		// Carries the count in progress to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Duration);
			stream.Serialize(Timer);
			stream.Serialize(IsActive);
			stream.Serialize(IsToCountUp);
		}

	private:
		/*
		 * This is the full length of the count that was last started, expressed in game
		 * frames. Delays are specified in game minutes, so the conversion is performed
		 * once here and the timer is loaded from the result.
		 */
		double Duration;

		/*
		 * This is the timer that measures the delay. It runs on game frames rather than
		 * real time, so a count keeps pace with the game logic, and it can be reversed in
		 * place to back a count out from wherever it happened to reach.
		 */
		ProgressTimerClass<FrameTimerClass> Timer;

		/*
		 * If a count is currently running, then this flag will be true. Ending the count
		 * clears it, which parks the timer at whichever extreme it was heading for.
		 */
		bool IsActive;

		/*
		 * If the count is running forwards -- a door opening rather than closing -- then
		 * this flag will be true. It outlives the count, since it is what tells an idle
		 * timer which end of its travel it is resting at.
		 */
		bool IsToCountUp;
};
