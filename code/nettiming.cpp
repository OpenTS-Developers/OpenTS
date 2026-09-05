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
		Provisional = false;
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


	/// <summary>Samples an acknowledgement; an unmeasured link takes an ambiguous one as a provisional upper bound.</summary>
	bool RttEstimator::Acknowledge(Milliseconds sent_at, unsigned int transmission_count, MillisecondClock const & clock)
	{
		if (transmission_count == 0) {
			return(false);
		}

		Milliseconds const elapsed = Elapsed_Milliseconds(sent_at, clock.Now());
		if (transmission_count != 1) {
			if (Initialized) {
				return(false);
			}
			Provisional = Add_Sample(elapsed);
			return(Provisional);
		}

		// The first clean sample replaces a provisional seed instead of blending with it.
		if (Provisional) {
			Initialized = false;
			Provisional = false;
		}
		return(Add_Sample(elapsed));
	}


	/// <summary>Backs the timeout off once per retransmission era so a slower link stays measurable.</summary>
	void RttEstimator::Note_Retransmit(Milliseconds captured_rto)
	{
		if (!Initialized) {
			return;
		}

		// Only a packet sent under the current timeout proves that timeout too short.
		if (captured_rto >= RetransmitTimeout) {
			RetransmitTimeout = Clamp_Rto(2ull * RetransmitTimeout);
		}
	}


	/// <summary>Derives a timeout that covers measured latency and three transmissions at the current RTO.</summary>
	Milliseconds Connection_Timeout(Milliseconds smoothed_rtt, Milliseconds retransmit_timeout)
	{
		std::uint64_t const timeout = std::max(8ull * smoothed_rtt + 250, 4ull * retransmit_timeout);
		return(static_cast<Milliseconds>(std::clamp<std::uint64_t>(timeout, MINIMUM_CONNECTION_TIMEOUT, MAXIMUM_CONNECTION_TIMEOUT)));
	}


	/// <summary>Bounds a packet's first retry so the connection timeout allows at least three transmissions.</summary>
	Milliseconds Initial_Retry_Timeout(Milliseconds retransmit_timeout, Milliseconds connection_timeout)
	{
		return(std::max(MINIMUM_RTO, std::min(retransmit_timeout, connection_timeout / 4)));
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


	/// <summary>Chooses the next action for one queued packet; a timed-out packet still retries at its capped backoff.</summary>
	RetryDecision Evaluate_Retry(RetransmitState const & state, Milliseconds now, Milliseconds current_rto, Milliseconds connection_timeout,
		bool timeout_enabled, bool adaptive)
	{
		if (state.TransmissionCount == 0) {
			return(RetryDecision{true, false});
		}

		RetryDecision decision;
		decision.TimedOut = timeout_enabled && Milliseconds_Have_Elapsed(state.FirstSend, now, connection_timeout);
		decision.Send = adaptive
			? Retransmit_Is_Due(state.LastSend, now, state.CapturedRto, state.TransmissionCount - 1, connection_timeout)
			: Milliseconds_Have_Elapsed(state.LastSend, now, current_rto);
		return(decision);
	}
}
