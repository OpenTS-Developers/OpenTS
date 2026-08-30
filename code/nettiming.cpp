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
#include <limits>
#include <numeric>


namespace NetTiming
{
	namespace
	{
		/// <summary>Divides positive integers without losing a remainder.</summary>
		constexpr std::uint64_t Divide_Round_Up(std::uint64_t numerator, std::uint64_t denominator)
		{
			return((numerator + denominator - 1) / denominator);
		}


		/// <summary>Constrains a retransmission timeout to the supported range.</summary>
		constexpr Milliseconds Clamp_Rto(std::uint64_t value)
		{
			return(static_cast<Milliseconds>(std::clamp<std::uint64_t>(value, MINIMUM_RTO, MAXIMUM_RTO)));
		}


		/// <summary>Selects timing for the current report census.</summary>
		TimingSettings Desired_Settings(TimingCensus const & census, unsigned int target_fps, LatencyFudge fudge, bool require_headroom)
		{
			if (census.RequiresConservativeTiming) {
				return(TimingSettings{MAXIMUM_TIMING_RUNG, MAXIMUM_MAX_AHEAD});
			}
			if (census.ActivePlayers == 0) {
				return(Settings_For_Rung(INITIAL_TIMING_RUNG));
			}
			return(Select_Timing_Settings(census.WorstRoundTrip, target_fps, fudge, require_headroom));
		}


		/// <summary>Checks whether settings increase the scheduling horizon.</summary>
		bool Timing_Is_Worse(TimingSettings candidate, TimingSettings current)
		{
			return(candidate.FrameSendRate > current.FrameSendRate
				|| (candidate.FrameSendRate == current.FrameSendRate && candidate.MaxAhead > current.MaxAhead));
		}


		/// <summary>Checks whether settings reduce the scheduling horizon.</summary>
		bool Timing_Is_Better(TimingSettings candidate, TimingSettings current)
		{
			return(candidate.FrameSendRate < current.FrameSendRate
				|| (candidate.FrameSendRate == current.FrameSendRate && candidate.MaxAhead < current.MaxAhead));
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


	/// <summary>Maps a policy rung to its balanced timing settings.</summary>
	TimingSettings Settings_For_Rung(unsigned int rung)
	{
		rung = std::clamp(rung, MINIMUM_TIMING_RUNG, MAXIMUM_TIMING_RUNG);
		return(TimingSettings{rung, rung == 1 ? 4u : 3u * rung});
	}


	/// <summary>Maps balanced timing settings to player-facing connection quality.</summary>
	ConnectionQuality Connection_Quality_For_Settings(TimingSettings settings)
	{
		if (!Timing_Settings_Are_Valid(settings) || settings.MaxAhead > Settings_For_Rung(settings.FrameSendRate).MaxAhead) {
			return(ConnectionQuality::Bad);
		}
		if (settings.FrameSendRate <= 2) {
			return(ConnectionQuality::Fast);
		}
		if (settings.FrameSendRate <= 5) {
			return(ConnectionQuality::Normal);
		}
		if (settings.FrameSendRate <= 8) {
			return(ConnectionQuality::Poor);
		}
		return(ConnectionQuality::Bad);
	}


	/// <summary>Checks timing bounds and send-period alignment.</summary>
	bool Timing_Settings_Are_Valid(TimingSettings settings)
	{
		TimingSettings const minimum = Settings_For_Rung(settings.FrameSendRate);
		return(settings.FrameSendRate >= MINIMUM_TIMING_RUNG && settings.FrameSendRate <= MAXIMUM_TIMING_RUNG
			&& settings.MaxAhead >= minimum.MaxAhead && settings.MaxAhead <= MAXIMUM_MAX_AHEAD && settings.MaxAhead % settings.FrameSendRate == 0);
	}


	/// <summary>Accepts a legacy aligned horizon as the source of a safe transition.</summary>
	bool Timing_Transition_Source_Is_Valid(TimingSettings settings)
	{
		return(settings.FrameSendRate >= MINIMUM_TIMING_RUNG && settings.FrameSendRate <= MAXIMUM_TIMING_RUNG
			&& settings.MaxAhead >= 2 * settings.FrameSendRate && settings.MaxAhead <= MAXIMUM_MAX_AHEAD
			&& settings.MaxAhead % settings.FrameSendRate == 0);
	}


	/// <summary>Applies the selected RTT safety margin.</summary>
	Milliseconds Apply_Latency_Fudge(Milliseconds round_trip, LatencyFudge fudge)
	{
		std::uint64_t numerator = round_trip;
		std::uint64_t denominator = 1;

		switch (fudge) {
			case LatencyFudge::None:
				break;
			case LatencyFudge::Half:
				numerator *= 3;
				denominator = 2;
				break;
			case LatencyFudge::Double:
				numerator *= 2;
				break;
			case LatencyFudge::Triple:
				numerator *= 3;
				break;
		}

		return(static_cast<Milliseconds>(std::min<std::uint64_t>(Divide_Round_Up(numerator, denominator), std::numeric_limits<Milliseconds>::max())));
	}


	/// <summary>Rounds a scheduling horizon up to a complete send period.</summary>
	std::optional<unsigned int> Align_Max_Ahead(unsigned int required, unsigned int frame_send_rate)
	{
		if (frame_send_rate == 0) {
			return(std::nullopt);
		}

		std::uint64_t const aligned = Divide_Round_Up(required, frame_send_rate) * frame_send_rate;
		if (aligned > MAXIMUM_MAX_AHEAD) {
			return(std::nullopt);
		}
		return(static_cast<unsigned int>(aligned));
	}


	/// <summary>Chooses the lowest rung that covers the adjusted RTT.</summary>
	TimingSettings Select_Timing_Settings(Milliseconds worst_round_trip, unsigned int target_fps, LatencyFudge fudge, bool require_headroom)
	{
		target_fps = std::clamp(target_fps, 1u, 60u);

		std::uint64_t adjusted = Apply_Latency_Fudge(worst_round_trip, fudge);
		if (require_headroom) {
			adjusted = Divide_Round_Up(adjusted * 5, 4);
		}

		std::uint64_t const one_way_frames = Divide_Round_Up(adjusted * target_fps, 2000);
		// A rung must cover one-way flight time plus a complete send period.
		for (unsigned int rung = MINIMUM_TIMING_RUNG; rung < MAXIMUM_TIMING_RUNG; rung++) {
			TimingSettings const settings = Settings_For_Rung(rung);
			std::uint64_t const floor = 3ull * settings.FrameSendRate;
			std::uint64_t const needed = std::max(floor, one_way_frames + settings.FrameSendRate);
			if (needed > std::numeric_limits<unsigned int>::max()) {
				continue;
			}

			std::optional<unsigned int> const aligned = Align_Max_Ahead(static_cast<unsigned int>(needed), settings.FrameSendRate);
			if (aligned && *aligned <= settings.MaxAhead) {
				return(settings);
			}
		}

		TimingSettings settings = Settings_For_Rung(MAXIMUM_TIMING_RUNG);
		std::uint64_t const needed = std::max<std::uint64_t>(settings.MaxAhead, one_way_frames + settings.FrameSendRate);
		if (needed >= MAXIMUM_MAX_AHEAD) {
			settings.MaxAhead = MAXIMUM_MAX_AHEAD - (MAXIMUM_MAX_AHEAD % settings.FrameSendRate);
		} else {
			settings.MaxAhead = *Align_Max_Ahead(static_cast<unsigned int>(needed), settings.FrameSendRate);
		}
		return(settings);
	}


	/// <summary>Returns the policy rung selected for an adjusted RTT.</summary>
	unsigned int Select_Timing_Rung(Milliseconds worst_round_trip, unsigned int target_fps, LatencyFudge fudge, bool require_headroom)
	{
		return(Select_Timing_Settings(worst_round_trip, target_fps, fudge, require_headroom).FrameSendRate);
	}


	/// <summary>Uses two early reports before settling on the normal cadence.</summary>
	bool Report_Is_Due(std::uint32_t elapsed_frames)
	{
		return(elapsed_frames > 0 && ((elapsed_frames <= BOOTSTRAP_FIRST_EVALUATION && elapsed_frames % BOOTSTRAP_REPORT_INTERVAL == 0)
			|| elapsed_frames % REPORT_INTERVAL == 0));
	}


	/// <summary>Schedules two bootstrap evaluations and the steady-state cadence.</summary>
	bool Evaluation_Is_Due(std::uint32_t elapsed_frames)
	{
		return(elapsed_frames == BOOTSTRAP_FIRST_EVALUATION || elapsed_frames == BOOTSTRAP_FINAL_EVALUATION
			|| (elapsed_frames > 0 && elapsed_frames % EVALUATION_INTERVAL == 0));
	}


	/// <summary>Clears the active-player report census.</summary>
	void TimingReportCensus::Reset(void)
	{
		Reports = {};
	}


	/// <summary>Adds or removes a player from the census.</summary>
	bool TimingReportCensus::Set_Player_Active(unsigned int player, bool active, std::uint32_t frame)
	{
		if (player >= Reports.size()) {
			return(false);
		}

		PlayerReport & report = Reports[player];
		if (report.Active != active) {
			report = {};
			report.Active = active;
			report.ActiveSinceFrame = frame;
		}
		return(true);
	}


	/// <summary>Checks whether a player belongs to the timing census.</summary>
	bool TimingReportCensus::Is_Player_Active(unsigned int player) const
	{
		return(player < Reports.size() && Reports[player].Active);
	}


	/// <summary>Records process time and optional RTT as one report.</summary>
	bool TimingReportCensus::Record_Report(unsigned int player, Milliseconds process_milliseconds, std::optional<Milliseconds> round_trip, std::uint32_t frame)
	{
		if (player >= Reports.size() || !Reports[player].Active || process_milliseconds > MAXIMUM_PROCESS_MILLISECONDS
			|| (round_trip && *round_trip > MAXIMUM_REPORTED_RTT)) {
			return(false);
		}

		PlayerReport & report = Reports[player];
		report.HasReport = true;
		report.HasRoundTrip = round_trip.has_value();
		report.EverHadRoundTrip |= round_trip.has_value();
		report.ProcessMilliseconds = process_milliseconds;
		report.RoundTrip = round_trip.value_or(0);
		report.ReportFrame = frame;
		return(true);
	}


	/// <summary>Summarizes fresh reports for a simulation frame.</summary>
	TimingCensus TimingReportCensus::Inspect(std::uint32_t frame) const
	{
		TimingCensus result;
		for (PlayerReport const & report : Reports) {
			if (!report.Active) {
				continue;
			}

			result.ActivePlayers++;
			bool const fresh = report.HasReport && frame - report.ReportFrame < REPORT_EXPIRY;
			if (fresh) {
				result.FreshProcessReports++;
				result.WorstProcessMilliseconds = std::max(result.WorstProcessMilliseconds, report.ProcessMilliseconds);
			} else {
				result.ProcessComplete = false;
			}

			if (fresh && report.HasRoundTrip) {
				result.FreshRoundTripReports++;
				result.WorstRoundTrip = std::max(result.WorstRoundTrip, report.RoundTrip);
			} else {
				result.RoundTripComplete = false;
				if (report.EverHadRoundTrip || frame - report.ActiveSinceFrame >= REPORT_EXPIRY) {
					result.RequiresConservativeTiming = true;
				}
			}
		}
		return(result);
	}


	/// <summary>Uses fresh process reports without discarding the synchronized frame rate.</summary>
	unsigned int Select_Desired_Frame_Rate(TimingCensus const & census, unsigned int synchronized_fps, unsigned int game_speed_fps)
	{
		synchronized_fps = std::clamp(synchronized_fps, 1u, 60u);
		game_speed_fps = std::clamp(game_speed_fps, 1u, 60u);
		if (!census.ProcessComplete) {
			return(synchronized_fps);
		}

		unsigned int const process_fps = census.WorstProcessMilliseconds == 0 ? 60u
			: static_cast<unsigned int>(std::max<Milliseconds>(1, 1000 / census.WorstProcessMilliseconds));
		return(std::min(process_fps, game_speed_fps));
	}


	/// <summary>Restores the balanced policy's initial state.</summary>
	void BalancedTimingPolicy::Reset(std::uint32_t frame)
	{
		CurrentRung = INITIAL_TIMING_RUNG;
		CurrentSettings = Settings_For_Rung(INITIAL_TIMING_RUNG);
		GoodEvaluations = 0;
		ReversibleChanges = 0;
		BootstrapStartFrame = frame;
		LastEvaluationFrame = frame;
		LastChangeFrame = 0;
		HasEvaluated = false;
		HasChanged = false;
		Bootstrapping = true;
	}


	/// <summary>Restores synchronized policy state after a master handoff.</summary>
	void BalancedTimingPolicy::Reset_From(TimingSettings settings, unsigned int reversible_changes, std::uint32_t frame)
	{
		CurrentRung = std::clamp(settings.FrameSendRate, MINIMUM_TIMING_RUNG, MAXIMUM_TIMING_RUNG);
		CurrentSettings = settings;
		GoodEvaluations = 0;
		ReversibleChanges = std::min(reversible_changes, REVERSIBLE_CHANGE_LIMIT);
		LastEvaluationFrame = frame;
		LastChangeFrame = frame;
		HasEvaluated = true;
		HasChanged = true;
		Bootstrapping = false;
	}


	/// <summary>Commits a policy change and resets hysteresis.</summary>
	void BalancedTimingPolicy::Change_To(TimingSettings settings, std::uint32_t frame)
	{
		CurrentRung = std::clamp(settings.FrameSendRate, MINIMUM_TIMING_RUNG, MAXIMUM_TIMING_RUNG);
		CurrentSettings = settings;
		GoodEvaluations = 0;
		LastChangeFrame = frame;
		HasChanged = true;
		if (ReversibleChanges < REVERSIBLE_CHANGE_LIMIT) {
			ReversibleChanges++;
		}
	}


	/// <summary>Anchors steady-state evaluations to 256 frames after reset.</summary>
	void BalancedTimingPolicy::Finish_Bootstrap(void)
	{
		Bootstrapping = false;
		GoodEvaluations = 0;
		LastEvaluationFrame = BootstrapStartFrame;
		HasEvaluated = true;
	}


	/// <summary>Applies cadence, hysteresis, and the change budget.</summary>
	TimingEvaluation BalancedTimingPolicy::Evaluate(TimingCensus const & census, unsigned int target_fps, LatencyFudge fudge, std::uint32_t frame)
	{
		TimingEvaluation result{Current_Settings(), CurrentRung, false, false};
		if (Bootstrapping) {
			std::uint32_t const elapsed_frames = frame - BootstrapStartFrame;
			if (elapsed_frames < BOOTSTRAP_FIRST_EVALUATION || (HasEvaluated && elapsed_frames < BOOTSTRAP_FINAL_EVALUATION)) {
				return(result);
			}

			HasEvaluated = true;
			LastEvaluationFrame = frame;
			result.Evaluated = true;
			bool const complete = census.ProcessComplete && census.RoundTripComplete;
			if (census.RequiresConservativeTiming || complete || elapsed_frames >= BOOTSTRAP_FINAL_EVALUATION) {
				TimingSettings const selected = census.RequiresConservativeTiming ? Desired_Settings(census, target_fps, fudge, false)
					: complete ? Desired_Settings(census, target_fps, fudge, true) : Settings_For_Rung(BOOTSTRAP_FALLBACK_RUNG);
				if (selected != CurrentSettings) {
					Change_To(selected, frame);
					result.Changed = true;
				}
				Finish_Bootstrap();
				result.Settings = Current_Settings();
				result.Rung = CurrentRung;
			}
			return(result);
		}

		if (HasEvaluated && frame - LastEvaluationFrame < EVALUATION_INTERVAL) {
			return(result);
		}

		HasEvaluated = true;
		LastEvaluationFrame = frame;
		result.Evaluated = true;
		if (!census.RequiresConservativeTiming && census.ActivePlayers > 0 && !census.RoundTripComplete) {
			GoodEvaluations = 0;
			return(result);
		}

		// Worsening is immediate; improvement must clear the headroom, cadence, and change-budget gates.
		TimingSettings const desired_settings = Desired_Settings(census, target_fps, fudge, false);
		if (Timing_Is_Worse(desired_settings, CurrentSettings)) {
			Change_To(desired_settings, frame);
			result.Changed = true;
		} else if (Timing_Is_Better(desired_settings, CurrentSettings) && ReversibleChanges < REVERSIBLE_CHANGE_LIMIT
			&& (!HasChanged || frame - LastChangeFrame >= CHANGE_COOLDOWN)) {
			TimingSettings const headroom = Desired_Settings(census, target_fps, fudge, true);
			if (Timing_Is_Better(headroom, CurrentSettings)) {
				GoodEvaluations++;
				if (GoodEvaluations >= GOOD_EVALUATIONS_REQUIRED) {
					TimingSettings const next = desired_settings.FrameSendRate < CurrentRung
						? Settings_For_Rung(CurrentRung - 1) : desired_settings;
					Change_To(next, frame);
					result.Changed = true;
				}
			} else {
				GoodEvaluations = 0;
			}
		} else {
			GoodEvaluations = 0;
		}

		result.Settings = Current_Settings();
		result.Rung = CurrentRung;
		return(result);
	}


	/// <summary>Delays decreases until the old scheduling horizon drains.</summary>
	std::optional<StagedTimingUpdate> Stage_Timing_Update(TimingSettings current, TimingSettings requested, std::uint32_t event_frame)
	{
		if (!Timing_Transition_Source_Is_Valid(current) || !Timing_Settings_Are_Valid(requested)) {
			return(std::nullopt);
		}

		bool const decrease = requested.FrameSendRate < current.FrameSendRate || requested.MaxAhead < current.MaxAhead;
		if (!decrease) {
			return(StagedTimingUpdate{requested, requested.MaxAhead, event_frame, false});
		}

		std::uint64_t const period = std::lcm(current.FrameSendRate, requested.FrameSendRate);
		std::uint64_t const old_horizon = static_cast<std::uint64_t>(event_frame) + current.MaxAhead;
		std::uint64_t const activation = Divide_Round_Up(old_horizon, period) * period;
		if (activation > std::numeric_limits<std::uint32_t>::max()) {
			return(std::nullopt);
		}

		unsigned int const minimum_horizon = std::max(requested.MaxAhead, current.MaxAhead - current.FrameSendRate);
		std::optional<unsigned int> const initial_max_ahead = Align_Max_Ahead(minimum_horizon, requested.FrameSendRate);
		if (!initial_max_ahead) {
			return(std::nullopt);
		}

		return(StagedTimingUpdate{requested, *initial_max_ahead, static_cast<std::uint32_t>(activation), true});
	}


	/// <summary>Advances one catch-up step without dropping below the target horizon.</summary>
	std::optional<unsigned int> Next_Transition_Max_Ahead(TimingSettings current, TimingSettings requested)
	{
		if (!Timing_Settings_Are_Valid(current) || !Timing_Settings_Are_Valid(requested) || current.FrameSendRate != requested.FrameSendRate) {
			return(std::nullopt);
		}

		if (current.MaxAhead <= requested.MaxAhead) {
			return(requested.MaxAhead);
		}
		return(std::max(requested.MaxAhead, current.MaxAhead - requested.FrameSendRate));
	}


	/// <summary>Advances one deterministic drain or catch-up boundary.</summary>
	std::optional<TimingTransitionAdvance> Advance_Timing_Transition(TimingTransitionState & transition, TimingSettings current, std::uint32_t frame)
	{
		bool const current_is_valid = transition.Activated ? Timing_Settings_Are_Valid(current) : Timing_Transition_Source_Is_Valid(current);
		if (!transition.Plan.Deferred || !current_is_valid || !Timing_Settings_Are_Valid(transition.Plan.Settings)
			|| !Timing_Settings_Are_Valid({transition.Plan.Settings.FrameSendRate, transition.Plan.InitialMaxAhead})) {
			return(std::nullopt);
		}

		TimingTransitionAdvance result{current};
		if (!transition.Activated) {
			if (!Timing_Update_Is_Due(frame, transition.Plan.ActivationFrame)) {
				return(result);
			}
			result.Settings = {transition.Plan.Settings.FrameSendRate, transition.Plan.InitialMaxAhead};
			result.Changed = result.Settings != current;
			transition.LastStepFrame = frame;
			transition.Activated = true;
		} else if (current.MaxAhead > transition.Plan.Settings.MaxAhead && frame > transition.LastStepFrame
			&& frame % transition.Plan.Settings.FrameSendRate == 0) {
			std::optional<unsigned int> const next = Next_Transition_Max_Ahead(current, transition.Plan.Settings);
			if (!next) {
				return(std::nullopt);
			}
			result.Settings.MaxAhead = *next;
			result.Changed = result.Settings != current;
			transition.LastStepFrame = frame;
		}

		result.Complete = transition.Activated && result.Settings == transition.Plan.Settings;
		return(result);
	}


	/// <summary>Checks a staged activation frame with wraparound semantics.</summary>
	bool Timing_Update_Is_Due(std::uint32_t frame, std::uint32_t activation_frame)
	{
		return(static_cast<std::int32_t>(frame - activation_frame) >= 0);
	}
}
