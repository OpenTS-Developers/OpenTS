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
	constexpr Milliseconds MAXIMUM_RTO = 2000;
	constexpr Milliseconds MINIMUM_CONNECTION_TIMEOUT = 2000;
	constexpr Milliseconds MAXIMUM_CONNECTION_TIMEOUT = 30000;
	constexpr Milliseconds MAXIMUM_PROCESS_MILLISECONDS = 1000;
	constexpr Milliseconds MAXIMUM_REPORTED_RTT = UINT16_MAX - 1u;

	constexpr unsigned int MAX_TIMING_PLAYERS = 8;
	constexpr unsigned int MINIMUM_TIMING_RUNG = 1;
	constexpr unsigned int MAXIMUM_TIMING_RUNG = 10;
	constexpr unsigned int INITIAL_TIMING_RUNG = 3;
	constexpr unsigned int MAXIMUM_MAX_AHEAD = 250;

	constexpr std::uint32_t REPORT_INTERVAL = 128;
	constexpr std::uint32_t EVALUATION_INTERVAL = 256;
	constexpr std::uint32_t CHANGE_COOLDOWN = 256;
	constexpr std::uint32_t REPORT_EXPIRY = 512;
	constexpr unsigned int GOOD_EVALUATIONS_REQUIRED = 3;
	constexpr unsigned int REVERSIBLE_CHANGE_LIMIT = 8;

	enum class LatencyFudge : unsigned char {
		None,
		Half,
		Double,
		Triple,
	};

	class RttEstimator
	{
		public:
			void Reset(void);
			bool Add_Sample(Milliseconds round_trip, bool retransmitted = false);
			bool Acknowledge(Milliseconds sent_at, unsigned int transmission_count, MillisecondClock const & clock = Default_Clock());

			bool Has_Sample(void) const {return(Initialized);}
			Milliseconds Smoothed_Rtt(void) const {return(SmoothedRtt);}
			Milliseconds Rtt_Variation(void) const {return(RttVariation);}
			Milliseconds Retransmit_Timeout(void) const {return(RetransmitTimeout);}

		private:
			bool Initialized = false;
			Milliseconds SmoothedRtt = 0;
			Milliseconds RttVariation = 0;
			Milliseconds RetransmitTimeout = MINIMUM_RTO;
	};

	Milliseconds Connection_Timeout(Milliseconds smoothed_rtt);
	Milliseconds Retransmit_Delay(Milliseconds base_rto, unsigned int prior_retransmissions, Milliseconds maximum_delay = MAXIMUM_RTO);
	bool Retransmit_Is_Due(Milliseconds last_send, Milliseconds now, Milliseconds base_rto,
		unsigned int prior_retransmissions, Milliseconds maximum_delay = MAXIMUM_RTO);

	struct TimingSettings {
		unsigned int FrameSendRate = 3;
		unsigned int MaxAhead = 9;

		bool operator==(TimingSettings const &) const = default;
	};

	TimingSettings Settings_For_Rung(unsigned int rung);
	bool Timing_Settings_Are_Valid(TimingSettings settings);
	Milliseconds Apply_Latency_Fudge(Milliseconds round_trip, LatencyFudge fudge);
	std::optional<unsigned int> Align_Max_Ahead(unsigned int required, unsigned int frame_send_rate);
	TimingSettings Select_Timing_Settings(Milliseconds worst_round_trip, unsigned int target_fps, LatencyFudge fudge, bool require_headroom = false);
	unsigned int Select_Timing_Rung(Milliseconds worst_round_trip, unsigned int target_fps, LatencyFudge fudge, bool require_headroom = false);

	struct TimingCensus {
		unsigned int ActivePlayers = 0;
		unsigned int FreshReports = 0;
		Milliseconds WorstRoundTrip = 0;
		bool Complete = true;
	};

	class TimingReportCensus
	{
		public:
			void Reset(void);
			bool Set_Player_Active(unsigned int player, bool active);
			bool Record_Report(unsigned int player, Milliseconds round_trip, std::uint32_t frame);
			bool Clear_Report(unsigned int player);
			TimingCensus Inspect(std::uint32_t frame) const;

		private:
			struct PlayerReport {
				bool Active = false;
				bool Present = false;
				Milliseconds RoundTrip = 0;
				std::uint32_t Frame = 0;
			};

			std::array<PlayerReport, MAX_TIMING_PLAYERS> Reports = {};
	};

	struct TimingEvaluation {
		TimingSettings Settings;
		unsigned int Rung = INITIAL_TIMING_RUNG;
		bool Evaluated = false;
		bool Changed = false;
	};

	class BalancedTimingPolicy
	{
		public:
			void Reset(void);
			TimingEvaluation Evaluate(TimingCensus const & census, unsigned int target_fps, LatencyFudge fudge, std::uint32_t frame);

			unsigned int Current_Rung(void) const {return(CurrentRung);}
			TimingSettings Current_Settings(void) const {return(CurrentSettings);}
			unsigned int Reversible_Changes(void) const {return(ReversibleChanges);}
			unsigned int Good_Evaluations(void) const {return(GoodEvaluations);}

		private:
			void Change_To(TimingSettings settings, std::uint32_t frame);

			unsigned int CurrentRung = INITIAL_TIMING_RUNG;
			TimingSettings CurrentSettings = {3, 9};
			unsigned int GoodEvaluations = 0;
			unsigned int ReversibleChanges = 0;
			std::uint32_t LastEvaluationFrame = 0;
			std::uint32_t LastChangeFrame = 0;
			bool HasEvaluated = false;
			bool HasChanged = false;
			bool HasCompleteCensus = false;
	};

	struct StagedTimingUpdate {
		TimingSettings Settings;
		std::uint32_t ActivationFrame = 0;
		bool Deferred = false;
	};

	std::optional<StagedTimingUpdate> Stage_Timing_Update(TimingSettings current, TimingSettings requested, std::uint32_t event_frame);
	bool Timing_Update_Is_Due(std::uint32_t frame, std::uint32_t activation_frame);
}
