/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "nettiming.h"

#include <algorithm>


namespace NetTiming
{
	namespace
	{
		/// <summary>Constrains a retransmission timeout to the supported range.</summary>
		constexpr Milliseconds Clamp_Rto(std::uint64_t value)
		{
			return(static_cast<Milliseconds>(std::clamp<std::uint64_t>(value, MINIMUM_RTO, MAXIMUM_RTO)));
		}
	}


	/// <summary>Restores the estimator to its unsampled state.</summary>
	void RttEstimator::Reset(void)
	{
		Initialized = false;
		SmoothedRtt = 0;
		RttVariation = 0;
		RetransmitTimeout = MINIMUM_RTO;
	}


	/// <summary>Updates SRTT, RTTVAR, and RTO from an eligible sample.</summary>
	bool RttEstimator::Add_Sample(Milliseconds round_trip, bool retransmitted)
	{
		// Karn's rule excludes ambiguous acknowledgements after retransmission.
		if (retransmitted) {
			return(false);
		}

		if (!Initialized) {
			Initialized = true;
			SmoothedRtt = round_trip;
			RttVariation = (round_trip + 1) / 2;
		} else {
			Milliseconds const error = SmoothedRtt > round_trip ? SmoothedRtt - round_trip : round_trip - SmoothedRtt;
			RttVariation = static_cast<Milliseconds>((3ull * RttVariation + error + 2) / 4);
			SmoothedRtt = static_cast<Milliseconds>((7ull * SmoothedRtt + round_trip + 4) / 8);
		}

		std::uint64_t const variation = std::max<std::uint64_t>(1, 4ull * RttVariation);
		RetransmitTimeout = Clamp_Rto(static_cast<std::uint64_t>(SmoothedRtt) + variation);
		return(true);
	}


	/// <summary>Samples an acknowledgement when its send time is unambiguous.</summary>
	bool RttEstimator::Acknowledge(Milliseconds sent_at, unsigned int transmission_count, MillisecondClock const & clock)
	{
		if (transmission_count != 1) {
			return(false);
		}
		return(Add_Sample(Elapsed_Milliseconds(sent_at, clock.Now())));
	}


	/// <summary>Derives the connection timeout from smoothed latency.</summary>
	Milliseconds Connection_Timeout(Milliseconds smoothed_rtt)
	{
		std::uint64_t const timeout = 8ull * smoothed_rtt + 250;
		return(static_cast<Milliseconds>(std::clamp<std::uint64_t>(timeout, MINIMUM_CONNECTION_TIMEOUT, MAXIMUM_CONNECTION_TIMEOUT)));
	}


	/// <summary>Applies bounded exponential backoff to a packet's RTO.</summary>
	Milliseconds Retransmit_Delay(Milliseconds base_rto, unsigned int prior_retransmissions, Milliseconds maximum_delay)
	{
		maximum_delay = std::max(maximum_delay, MINIMUM_RTO);
		std::uint64_t delay = std::clamp(base_rto, MINIMUM_RTO, maximum_delay);
		while (prior_retransmissions-- > 0 && delay < maximum_delay) {
			delay = std::min<std::uint64_t>(delay * 2, maximum_delay);
		}
		return(static_cast<Milliseconds>(delay));
	}


	/// <summary>Checks whether a packet's current backoff interval has elapsed.</summary>
	bool Retransmit_Is_Due(Milliseconds last_send, Milliseconds now, Milliseconds base_rto, unsigned int prior_retransmissions, Milliseconds maximum_delay)
	{
		return(Milliseconds_Have_Elapsed(last_send, now, Retransmit_Delay(base_rto, prior_retransmissions, maximum_delay)));
	}
}
