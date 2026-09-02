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

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>


namespace NetTiming
{
	constexpr Milliseconds MINIMUM_RTO = 100;
	// Above any round trip a private link is expected to carry, so a slow link's first retry
	// does not precede its acknowledgement.
	constexpr Milliseconds MAXIMUM_RTO = 4000;
	constexpr Milliseconds MINIMUM_CONNECTION_TIMEOUT = 2000;
	constexpr Milliseconds MAXIMUM_CONNECTION_TIMEOUT = 30000;
	constexpr Milliseconds MAXIMUM_PROCESS_MILLISECONDS = 1000;
	constexpr Milliseconds MAXIMUM_REPORTED_RTT = UINT16_MAX - 1u;

	constexpr unsigned int MAX_TIMING_PLAYERS = 8;
	constexpr unsigned int MINIMUM_TIMING_RUNG = 1;
	constexpr unsigned int MAXIMUM_TIMING_RUNG = 10;
	constexpr unsigned int INITIAL_TIMING_RUNG = 2;
	constexpr unsigned int BOOTSTRAP_FALLBACK_RUNG = 3;
	constexpr unsigned int MAXIMUM_MAX_AHEAD = 250;

	constexpr std::uint32_t BOOTSTRAP_REPORT_INTERVAL = 32;
	constexpr std::uint32_t BOOTSTRAP_FIRST_EVALUATION = 64;
	constexpr std::uint32_t BOOTSTRAP_FINAL_EVALUATION = 128;
	constexpr std::uint32_t REPORT_INTERVAL = 128;
	constexpr std::uint32_t EVALUATION_INTERVAL = 256;
	constexpr std::uint32_t CHANGE_COOLDOWN = 256;
	constexpr std::uint32_t REPORT_EXPIRY = 512;
	constexpr unsigned int GOOD_EVALUATIONS_REQUIRED = 3;
	constexpr unsigned int DESCENT_EVALUATIONS_REQUIRED = 1;

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

	Milliseconds Connection_Timeout(Milliseconds smoothed_rtt, Milliseconds retransmit_timeout);
	Milliseconds Initial_Retry_Timeout(Milliseconds retransmit_timeout, Milliseconds connection_timeout);
	Milliseconds Retransmit_Delay(Milliseconds base_rto, unsigned int prior_retransmissions, Milliseconds maximum_delay = MAXIMUM_RTO);
	bool Retransmit_Is_Due(Milliseconds last_send, Milliseconds now, Milliseconds base_rto,
		unsigned int prior_retransmissions, Milliseconds maximum_delay = MAXIMUM_RTO);
	RetryDecision Evaluate_Retry(RetransmitState const & state, Milliseconds now, Milliseconds current_rto, Milliseconds connection_timeout,
		bool timeout_enabled, bool adaptive);

	struct TimingSettings {
		unsigned int FrameSendRate = INITIAL_TIMING_RUNG;
		unsigned int MaxAhead = 3 * INITIAL_TIMING_RUNG;

		bool operator==(TimingSettings const &) const = default;
	};

	enum class ConnectionQuality : unsigned char {
		Bad,
		Poor,
		Normal,
		Fast,
	};

	TimingSettings Settings_For_Rung(unsigned int rung);
	ConnectionQuality Connection_Quality_For_Settings(TimingSettings settings);
	bool Timing_Settings_Are_Valid(TimingSettings settings);
	bool Timing_Transition_Source_Is_Valid(TimingSettings settings);
	std::optional<unsigned int> Align_Max_Ahead(unsigned int required, unsigned int frame_send_rate);
	TimingSettings Select_Timing_Settings(Milliseconds worst_round_trip, unsigned int target_fps, bool require_headroom = false);
	bool Report_Is_Due(std::uint32_t elapsed_frames);
	bool Evaluation_Is_Due(std::uint32_t elapsed_frames);

	struct TimingCensus {
		unsigned int ActivePlayers = 0;
		unsigned int FreshProcessReports = 0;
		unsigned int FreshRoundTripReports = 0;
		Milliseconds WorstProcessMilliseconds = 0;
		Milliseconds WorstRoundTrip = 0;
		bool ProcessComplete = true;
		bool RoundTripComplete = true;
		bool RequiresConservativeTiming = false;
	};

	class TimingReportCensus
	{
		public:
			void Reset(void);
			bool Set_Player_Active(unsigned int player, bool active, std::uint32_t frame);
			bool Is_Player_Active(unsigned int player) const;
			bool Record_Report(unsigned int player, Milliseconds process_milliseconds, std::optional<Milliseconds> round_trip, std::uint32_t frame);
			TimingCensus Inspect(std::uint32_t frame) const;

		private:
			struct PlayerReport {
				bool Active = false;
				bool HasReport = false;
				bool HasRoundTrip = false;
				bool EverHadRoundTrip = false;
				Milliseconds ProcessMilliseconds = 0;
				Milliseconds RoundTrip = 0;
				std::uint32_t ActiveSinceFrame = 0;
				std::uint32_t ReportFrame = 0;
			};

			std::array<PlayerReport, MAX_TIMING_PLAYERS> Reports = {};
	};

	unsigned int Select_Desired_Frame_Rate(TimingCensus const & census, unsigned int synchronized_fps, unsigned int game_speed_fps);

	struct TimingEvaluation {
		TimingSettings Settings;
		unsigned int Rung = INITIAL_TIMING_RUNG;
		bool Evaluated = false;
		bool Changed = false;
	};

	class BalancedTimingPolicy
	{
		public:
			void Reset(std::uint32_t frame = 0);
			void Reset_From(TimingSettings settings, std::uint32_t frame);
			TimingEvaluation Evaluate(TimingCensus const & census, unsigned int target_fps, std::uint32_t frame);

			unsigned int Current_Rung(void) const {return(CurrentRung);}
			TimingSettings Current_Settings(void) const {return(CurrentSettings);}
			unsigned int Good_Evaluations(void) const {return(GoodEvaluations);}
			bool Is_Bootstrapping(void) const {return(Bootstrapping);}
			std::uint32_t Cadence_Origin(void) const {return(BootstrapStartFrame);}

		private:
			void Change_To(TimingSettings settings, std::uint32_t frame);
			void Finish_Bootstrap(void);

			unsigned int CurrentRung = INITIAL_TIMING_RUNG;
			TimingSettings CurrentSettings = {INITIAL_TIMING_RUNG, 3 * INITIAL_TIMING_RUNG};
			unsigned int GoodEvaluations = 0;
			std::uint32_t BootstrapStartFrame = 0;
			std::uint32_t LastEvaluationFrame = 0;
			std::uint32_t LastChangeFrame = 0;
			bool HasEvaluated = false;
			bool HasChanged = false;
			bool Bootstrapping = true;
			// Set by an improvement; while it holds, each evaluation with headroom steps one more rung.
			bool ImprovementStreak = false;
	};

	struct StagedTimingUpdate {
		TimingSettings Settings;
		unsigned int InitialMaxAhead = 0;
		std::uint32_t ActivationFrame = 0;
		bool Deferred = false;
	};

	struct TimingTransitionState {
		StagedTimingUpdate Plan;
		std::uint32_t LastStepFrame = 0;
		bool Activated = false;
	};

	struct TimingTransitionAdvance {
		TimingSettings Settings;
		bool Changed = false;
		bool Complete = false;
	};

	enum class ScheduleResult
	{
		Rejected,
		Applied,
		Staged,
	};

	std::optional<StagedTimingUpdate> Stage_Timing_Update(TimingSettings current, TimingSettings requested, std::uint32_t event_frame);
	std::optional<std::uint32_t> Next_Send_Boundary(std::uint32_t frame, unsigned int frame_send_rate);
	std::optional<unsigned int> Next_Transition_Max_Ahead(TimingSettings current, TimingSettings requested);
	std::optional<TimingTransitionAdvance> Advance_Timing_Transition(TimingTransitionState & transition, TimingSettings current, std::uint32_t frame);
	bool Timing_Update_Is_Due(std::uint32_t frame, std::uint32_t activation_frame);
}
