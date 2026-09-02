/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#pragma once

#include "nettime.h"


namespace NetTiming
{
	constexpr Milliseconds MINIMUM_RTO = 100;
	// Above any round trip a private link is expected to carry, so a slow link's first retry
	// does not precede its acknowledgement.
	constexpr Milliseconds MAXIMUM_RTO = 4000;
	constexpr Milliseconds MINIMUM_CONNECTION_TIMEOUT = 2000;
	constexpr Milliseconds MAXIMUM_CONNECTION_TIMEOUT = 30000;

	struct RetryDecision
	{
		bool Send = false;
		bool TimedOut = false;
	};

	struct RetransmitState
	{
		Milliseconds FirstSend = 0;
		Milliseconds LastSend = 0;
		Milliseconds CapturedRto = MINIMUM_RTO;
		unsigned int TransmissionCount = 0;
	};

	class RttEstimator
	{
		public:
			void Reset(void);
			bool Add_Sample(Milliseconds round_trip, bool retransmitted = false);
			bool Acknowledge(Milliseconds sent_at, unsigned int transmission_count, MillisecondClock const & clock = Default_Clock());
			void Note_Retransmit(Milliseconds captured_rto);

			bool Has_Sample(void) const {return(Initialized);}
			bool Is_Provisional(void) const {return(Provisional);}
			Milliseconds Smoothed_Rtt(void) const {return(SmoothedRtt);}
			Milliseconds Rtt_Variation(void) const {return(RttVariation);}
			Milliseconds Retransmit_Timeout(void) const {return(RetransmitTimeout);}

		private:
			bool Initialized = false;
			Milliseconds SmoothedRtt = 0;
			Milliseconds RttVariation = 0;
			Milliseconds RetransmitTimeout = MINIMUM_RTO;
			// Set while the estimate comes from an ambiguous first acknowledgement.
			bool Provisional = false;
	};

	Milliseconds Connection_Timeout(Milliseconds smoothed_rtt);
	Milliseconds Retransmit_Delay(Milliseconds base_rto, unsigned int prior_retransmissions, Milliseconds maximum_delay = MAXIMUM_RTO);
	bool Retransmit_Is_Due(Milliseconds last_send, Milliseconds now, Milliseconds base_rto,
		unsigned int prior_retransmissions, Milliseconds maximum_delay = MAXIMUM_RTO);
	RetryDecision Evaluate_Retry(RetransmitState const & state, Milliseconds now, Milliseconds current_rto, Milliseconds connection_timeout,
		bool timeout_enabled, bool adaptive);
}
