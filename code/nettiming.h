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
	constexpr Milliseconds MAXIMUM_RTO = 2000;
	constexpr Milliseconds MINIMUM_CONNECTION_TIMEOUT = 2000;
	constexpr Milliseconds MAXIMUM_CONNECTION_TIMEOUT = 30000;
	// Long enough for the backoff ladder to climb from the minimum to the maximum RTO and
	// still measure a clean acknowledgement, so a recovering link is not reported stale.
	constexpr Milliseconds RTT_SAMPLE_LIFETIME = 8000;

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
			void Note_Retransmit(Milliseconds captured_rto, Milliseconds now);

			bool Has_Sample(void) const {return(Initialized);}
			bool Has_Fresh_Sample(Milliseconds now) const;
			Milliseconds Smoothed_Rtt(void) const {return(SmoothedRtt);}
			Milliseconds Rtt_Variation(void) const {return(RttVariation);}
			Milliseconds Retransmit_Timeout(void) const {return(RetransmitTimeout);}

		private:
			bool Initialized = false;
			Milliseconds SmoothedRtt = 0;
			Milliseconds RttVariation = 0;
			Milliseconds RetransmitTimeout = MINIMUM_RTO;
			// Start of the current stretch of retransmissions without an eligible sample.
			Milliseconds RetransmitEpochAt = 0;
			bool RetransmittedSinceLastSample = false;
	};

	Milliseconds Connection_Timeout(Milliseconds smoothed_rtt);
	Milliseconds Retransmit_Delay(Milliseconds base_rto, unsigned int prior_retransmissions, Milliseconds maximum_delay = MAXIMUM_RTO);
	bool Retransmit_Is_Due(Milliseconds last_send, Milliseconds now, Milliseconds base_rto,
		unsigned int prior_retransmissions, Milliseconds maximum_delay = MAXIMUM_RTO);
	RetryDecision Evaluate_Retry(RetransmitState const & state, Milliseconds now, Milliseconds current_rto, Milliseconds connection_timeout,
		bool timeout_enabled, bool adaptive);
}
