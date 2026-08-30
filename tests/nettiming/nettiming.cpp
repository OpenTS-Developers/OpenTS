/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "netsemantic.h"
#include "nettiming.h"

#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>


namespace
{
	class FakeClock final : public NetTiming::MillisecondClock
	{
		public:
			NetTiming::Milliseconds Now(void) const override {return(Current);}
			void Set(NetTiming::Milliseconds now) {Current = now;}

		private:
			NetTiming::Milliseconds Current = 0;
	};


	class FakeTransport
	{
		public:
			void Send(NetTiming::Milliseconds now)
			{
				Clock.Set(now);
				FirstSend = now;
				LastSend = now;
				TransmissionCount = 1;
				BaseRto = Estimator.Retransmit_Timeout();
			}

			bool Retry(NetTiming::Milliseconds now)
			{
				Clock.Set(now);
				if (!NetTiming::Retransmit_Is_Due(LastSend, now, BaseRto,
					TransmissionCount - 1, NetTiming::MINIMUM_CONNECTION_TIMEOUT)) {
					return(false);
				}
				LastSend = now;
				TransmissionCount++;
				return(true);
			}

			bool Acknowledge(NetTiming::Milliseconds now)
			{
				Clock.Set(now);
				return(Estimator.Acknowledge(FirstSend, TransmissionCount, Clock));
			}

			NetTiming::RttEstimator const & Rtt(void) const {return(Estimator);}

		private:
			FakeClock Clock;
			NetTiming::RttEstimator Estimator;
			NetTiming::Milliseconds FirstSend = 0;
			NetTiming::Milliseconds LastSend = 0;
			NetTiming::Milliseconds BaseRto = NetTiming::MINIMUM_RTO;
			unsigned int TransmissionCount = 0;
	};


	int Failures = 0;


	template<typename Actual, typename Expected>
	void Expect_Equal(std::string const & name, Actual const & actual, Expected const & expected)
	{
		if (actual == expected) {
			return;
		}

		std::cerr << name << ": expected " << expected << ", got " << actual << '\n';
		Failures++;
	}


	void Expect(std::string const & name, bool condition)
	{
		if (!condition) {
			std::cerr << name << " failed\n";
			Failures++;
		}
	}


	void Test_Rtt_Estimator(void)
	{
		using namespace NetTiming;

		RttEstimator estimator;
		Expect("estimator starts empty", !estimator.Has_Sample());
		Expect("first sample accepted", estimator.Add_Sample(100));
		Expect_Equal("first smoothed RTT", estimator.Smoothed_Rtt(), 100u);
		Expect_Equal("first variation", estimator.Rtt_Variation(), 50u);
		Expect_Equal("first RTO", estimator.Retransmit_Timeout(), 300u);

		Expect("second sample accepted", estimator.Add_Sample(140));
		Expect_Equal("alpha one eighth", estimator.Smoothed_Rtt(), 105u);
		Expect_Equal("beta one quarter", estimator.Rtt_Variation(), 48u);
		Expect_Equal("updated RTO", estimator.Retransmit_Timeout(), 297u);

		Expect("retransmitted sample rejected", !estimator.Add_Sample(900, true));
		Expect_Equal("Karn keeps smoothed RTT", estimator.Smoothed_Rtt(), 105u);
		Expect_Equal("Karn keeps RTO", estimator.Retransmit_Timeout(), 297u);

		RttEstimator minimum;
		minimum.Add_Sample(0);
		Expect_Equal("minimum RTO clamp", minimum.Retransmit_Timeout(), MINIMUM_RTO);

		RttEstimator maximum;
		maximum.Add_Sample(2000);
		Expect_Equal("maximum RTO clamp", maximum.Retransmit_Timeout(), MAXIMUM_RTO);

		RttEstimator fast_link;
		RttEstimator slow_link;
		fast_link.Add_Sample(50);
		slow_link.Add_Sample(300);
		Expect("unequal links keep independent RTOs",
			fast_link.Retransmit_Timeout() < slow_link.Retransmit_Timeout());

		estimator.Reset();
		Expect("reset clears estimator", !estimator.Has_Sample());
		Expect_Equal("reset restores RTO", estimator.Retransmit_Timeout(), MINIMUM_RTO);
	}


	void Test_Clock_And_Wrap(void)
	{
		using namespace NetTiming;

		FakeClock clock;
		clock.Set(0x00000020u);
		RttEstimator estimator;
		Expect("wrap sample accepted", estimator.Acknowledge(0xfffffff0u, 1, clock));
		Expect_Equal("wrap elapsed", estimator.Smoothed_Rtt(), 48u);
		Expect("retransmitted acknowledgement ignored", !estimator.Acknowledge(0, 2, clock));

		Expect("wrapped retry due", Retransmit_Is_Due(0xfffffff0u, 0x00000054u, 100, 0));
		Expect("wrapped retry not early", !Retransmit_Is_Due(0xfffffff0u, 0x00000040u, 100, 0));
	}


	void Test_Retransmit_Backoff(void)
	{
		using namespace NetTiming;

		Expect_Equal("base retry", Retransmit_Delay(100, 0), 100u);
		Expect_Equal("first backoff", Retransmit_Delay(100, 1), 200u);
		Expect_Equal("second backoff", Retransmit_Delay(100, 2), 400u);
		Expect_Equal("third backoff", Retransmit_Delay(100, 3), 800u);
		Expect_Equal("fourth backoff", Retransmit_Delay(100, 4), 1600u);
		Expect_Equal("backoff saturation", Retransmit_Delay(100, 20), MAXIMUM_RTO);
		Expect_Equal("base clamp", Retransmit_Delay(1, 0), MINIMUM_RTO);
		Expect_Equal("connection timeout minimum", Connection_Timeout(0), 2000u);
		Expect_Equal("connection timeout follows RTT", Connection_Timeout(500), 4250u);
		Expect_Equal("connection timeout ceiling", Connection_Timeout(10000), 30000u);
		Expect_Equal("backoff reaches connection timeout", Retransmit_Delay(500, 8, 4250), 4250u);
	}


	void Test_Loss_Jitter_And_Reordering(void)
	{
		using namespace NetTiming;

		FakeClock clock;
		RttEstimator reordered;
		clock.Set(1200);
		Expect("newer packet ACK samples first", reordered.Acknowledge(1100, 1, clock));
		clock.Set(1300);
		Expect("older packet ACK can sample after reordering", reordered.Acknowledge(1000, 1, clock));
		Expect_Equal("reordered samples keep alpha filter", reordered.Smoothed_Rtt(), 125u);
		Expect_Equal("reordered samples keep beta filter", reordered.Rtt_Variation(), 88u);

		clock.Set(2000);
		Expect("duplicate ambiguous ACK is excluded by Karn",
			!reordered.Acknowledge(1500, 2, clock));
		Expect_Equal("ambiguous ACK leaves SRTT unchanged", reordered.Smoothed_Rtt(), 125u);

		RttEstimator jitter;
		for (Milliseconds sample : {20u, 400u, 35u, 350u, 40u}) {
			jitter.Add_Sample(sample);
		}
		Expect("jitter raises variation", jitter.Rtt_Variation() > 0);
		Expect("jittered RTO remains bounded", jitter.Retransmit_Timeout() >= MINIMUM_RTO
			&& jitter.Retransmit_Timeout() <= MAXIMUM_RTO);

		Expect("loss does not retransmit before the base RTO",
			!Retransmit_Is_Due(1000, 1099, 100, 0, 2000));
		Expect("first loss retransmits at the base RTO",
			Retransmit_Is_Due(1000, 1100, 100, 0, 2000));
		Expect("second loss waits for exponential backoff",
			!Retransmit_Is_Due(1100, 1299, 100, 1, 2000));
		Expect("second loss retransmits at doubled RTO",
			Retransmit_Is_Due(1100, 1300, 100, 1, 2000));

		FakeTransport clean_transport;
		clean_transport.Send(1000);
		Expect("fake transport accepts a clean ACK sample", clean_transport.Acknowledge(1080));
		Expect_Equal("fake transport publishes clean RTT", clean_transport.Rtt().Smoothed_Rtt(), 80u);

		FakeTransport lossy_transport;
		lossy_transport.Send(1000);
		Expect("fake transport retries a lost packet", lossy_transport.Retry(1100));
		Expect("fake transport applies Karn after loss", !lossy_transport.Acknowledge(1180));
		Expect("lossy fake transport has no ambiguous RTT sample", !lossy_transport.Rtt().Has_Sample());
	}


	void Test_Census(void)
	{
		using namespace NetTiming;

		TimingReportCensus census;
		Expect("activate first peer", census.Set_Player_Active(1, true, 100));
		Expect("activate second peer", census.Set_Player_Active(2, true, 100));
		Expect("reject out of range peer", !census.Set_Player_Active(MAX_TIMING_PLAYERS, true, 100));
		Expect("active membership is queryable", census.Is_Player_Active(1));
		Expect("out of range membership is inactive", !census.Is_Player_Active(MAX_TIMING_PLAYERS));
		Expect("record first peer", census.Record_Report(1, 12, 80, 100));
		Expect("record second peer", census.Record_Report(2, 20, 180, 100));
		Expect("accept RTT above retransmit clamp", census.Record_Report(2, 20, MAXIMUM_RTO + 1, 100));
		Expect("reject process time beyond engine range", !census.Record_Report(2, MAXIMUM_PROCESS_MILLISECONDS + 1, 100, 150));
		Expect("reject RTT beyond wire range", !census.Record_Report(2, 1, MAXIMUM_REPORTED_RTT + 1, 150));

		TimingCensus result = census.Inspect(200);
		Expect_Equal("active peer count", result.ActivePlayers, 2u);
		Expect_Equal("fresh process report count", result.FreshProcessReports, 2u);
		Expect_Equal("fresh RTT report count", result.FreshRoundTripReports, 2u);
		Expect_Equal("worst process time", result.WorstProcessMilliseconds, 20u);
		Expect_Equal("unequal links publish worst", result.WorstRoundTrip, MAXIMUM_RTO + 1);
		Expect("fresh process census complete", result.ProcessComplete);
		Expect("fresh RTT census complete", result.RoundTripComplete);
		Expect("fresh census is not conservative", !result.RequiresConservativeTiming);
		BalancedTimingPolicy aggregate;
		TimingEvaluation const guest_degradation = aggregate.Evaluate(result, 60, 200);
		Expect("a guest-to-guest slow path worsens the master policy", guest_degradation.Changed && guest_degradation.Rung == MAXIMUM_TIMING_RUNG);

		result = census.Inspect(100 + REPORT_EXPIRY);
		Expect("process reports expire on boundary", !result.ProcessComplete);
		Expect("RTT reports expire on boundary", !result.RoundTripComplete);
		Expect("established RTT expiry is conservative", result.RequiresConservativeTiming);
		Expect_Equal("expired process reports not fresh", result.FreshProcessReports, 0u);
		Expect_Equal("expired RTT reports not fresh", result.FreshRoundTripReports, 0u);
		Expect_Equal("expired process time excluded", result.WorstProcessMilliseconds, 0u);

		Expect("departed peer removed", census.Set_Player_Active(2, false, 700));
		Expect("remaining peer refreshed", census.Record_Report(1, 15, 90, 700));
		result = census.Inspect(700);
		Expect("departure restores complete process census", result.ProcessComplete);
		Expect("departure restores complete RTT census", result.RoundTripComplete);
		Expect_Equal("departed peer excluded", result.ActivePlayers, 1u);
		Expect_Equal("remaining peer wins census", result.WorstRoundTrip, 90u);

		Expect("established unavailable RTT report accepted", census.Record_Report(1, 16, std::nullopt, 701));
		result = census.Inspect(701);
		Expect("unavailable RTT retains fresh process time", result.ProcessComplete && result.FreshProcessReports == 1);
		Expect("established unavailable RTT is incomplete", !result.RoundTripComplete);
		Expect("established unavailable RTT is immediately conservative", result.RequiresConservativeTiming);

		TimingReportCensus grace;
		Expect("activate grace peer", grace.Set_Player_Active(3, true, 1000));
		Expect("process-only initial report is accepted", grace.Record_Report(3, 30, std::nullopt, 1000));
		result = grace.Inspect(1000 + REPORT_EXPIRY - 1);
		Expect("process-only report remains complete before expiry", result.ProcessComplete);
		Expect("missing initial RTT is tolerated before expiry", !result.RequiresConservativeTiming);
		result = grace.Inspect(1000 + REPORT_EXPIRY);
		Expect("never-valid RTT becomes conservative at exact expiry", result.RequiresConservativeTiming);
		Expect("never-valid RTT remains incomplete", !result.RoundTripComplete);
		Expect("process data expires with its report", !result.ProcessComplete);
		Expect_Equal("stale process data retains synchronized FPS", Select_Desired_Frame_Rate(result, 42, 60), 42u);
		TimingCensus fresh_process;
		fresh_process.WorstProcessMilliseconds = 50;
		Expect_Equal("fresh process data respects game-speed FPS", Select_Desired_Frame_Rate(fresh_process, 42, 15), 15u);
		fresh_process.WorstProcessMilliseconds = 0;
		Expect_Equal("zero process time permits 60 FPS", Select_Desired_Frame_Rate(fresh_process, 42, 60), 60u);

		TimingReportCensus atomic;
		atomic.Set_Player_Active(4, true, 0);
		Expect("atomic baseline report accepted", atomic.Record_Report(4, 25, 125, 10));
		Expect("invalid process report rejected atomically", !atomic.Record_Report(4, MAXIMUM_PROCESS_MILLISECONDS + 1, 200, 20));
		Expect("invalid RTT report rejected atomically", !atomic.Record_Report(4, 50, MAXIMUM_REPORTED_RTT + 1, 20));
		result = atomic.Inspect(20);
		Expect_Equal("invalid report preserves process time", result.WorstProcessMilliseconds, 25u);
		Expect_Equal("invalid report preserves RTT", result.WorstRoundTrip, 125u);
		Expect("removing a peer clears its complete report", atomic.Set_Player_Active(4, false, 30));
		Expect_Equal("removed peer no longer contributes", atomic.Inspect(30).ActivePlayers, 0u);
		Expect("reactivated peer starts with a clean report", atomic.Set_Player_Active(4, true, 40));
		result = atomic.Inspect(40);
		Expect("reactivated peer has no inherited process report", !result.ProcessComplete);
		Expect("reactivated peer receives fresh RTT grace", !result.RequiresConservativeTiming);
	}


	void Test_Rungs(void)
	{
		using namespace NetTiming;

		Expect_Equal("initial FSR", Settings_For_Rung(INITIAL_TIMING_RUNG).FrameSendRate, 2u);
		Expect_Equal("initial MaxAhead", Settings_For_Rung(INITIAL_TIMING_RUNG).MaxAhead, 6u);
		Expect("default settings match the bootstrap rung", TimingSettings{} == Settings_For_Rung(INITIAL_TIMING_RUNG));
		Expect_Equal("best rung MaxAhead", Settings_For_Rung(1).MaxAhead, 4u);
		Expect_Equal("worst rung MaxAhead", Settings_For_Rung(10).MaxAhead, 30u);
		Expect("rung settings valid", Timing_Settings_Are_Valid(Settings_For_Rung(10)));
		Expect("below-rung minimum invalid", !Timing_Settings_Are_Valid({3, 6}));
		Expect("legacy two-period horizon can source a transition", Timing_Transition_Source_Is_Valid({3, 6}));
		Expect("unaligned settings invalid", !Timing_Settings_Are_Valid({3, 10}));

		Expect_Equal("zero RTT selects best rung", Select_Timing_Settings(0, 60).FrameSendRate, 1u);
		Expect_Equal("100 ms fits best rung", Select_Timing_Settings(100, 60).FrameSendRate, 1u);
		Expect_Equal("101 ms advances a rung", Select_Timing_Settings(101, 60).FrameSendRate, 2u);
		Expect_Equal("300 ms selects balanced rung", Select_Timing_Settings(300, 60).FrameSendRate, 5u);
		TimingSettings const high_rtt = Select_Timing_Settings(2000, 60);
		Expect_Equal("two-second RTT selects highest FSR", high_rtt.FrameSendRate, 10u);
		Expect_Equal("two-second RTT carries needed aligned MaxAhead", high_rtt.MaxAhead, 70u);
		TimingSettings const capped = Select_Timing_Settings(MAXIMUM_REPORTED_RTT, 60);
		Expect_Equal("wire-maximum RTT selects highest FSR", capped.FrameSendRate, 10u);
		Expect_Equal("highest rung caps at largest aligned horizon", capped.MaxAhead, 250u);

		Expect("alignment rejects zero period", !Align_Max_Ahead(10, 0));
		Expect_Equal("alignment reaches cap", *Align_Max_Ahead(249, 10), 250u);
		Expect("alignment rejects over cap", !Align_Max_Ahead(250, 9));
	}


	void Test_Connection_Quality(void)
	{
		using namespace NetTiming;

		Expect("rung one reports fast", Connection_Quality_For_Settings(Settings_For_Rung(1)) == ConnectionQuality::Fast);
		Expect("rung two reports fast", Connection_Quality_For_Settings(Settings_For_Rung(2)) == ConnectionQuality::Fast);
		Expect("rung three reports normal", Connection_Quality_For_Settings(Settings_For_Rung(3)) == ConnectionQuality::Normal);
		Expect("rung four reports normal", Connection_Quality_For_Settings(Settings_For_Rung(4)) == ConnectionQuality::Normal);
		Expect("rung five reports normal", Connection_Quality_For_Settings(Settings_For_Rung(5)) == ConnectionQuality::Normal);
		Expect("rung six reports poor", Connection_Quality_For_Settings(Settings_For_Rung(6)) == ConnectionQuality::Poor);
		Expect("rung seven reports poor", Connection_Quality_For_Settings(Settings_For_Rung(7)) == ConnectionQuality::Poor);
		Expect("rung eight reports poor", Connection_Quality_For_Settings(Settings_For_Rung(8)) == ConnectionQuality::Poor);
		Expect("rung nine reports bad", Connection_Quality_For_Settings(Settings_For_Rung(9)) == ConnectionQuality::Bad);
		Expect("rung ten reports bad", Connection_Quality_For_Settings(Settings_For_Rung(10)) == ConnectionQuality::Bad);
		Expect("bootstrap settings report fast", Connection_Quality_For_Settings({2, 6}) == ConnectionQuality::Fast);
		Expect("fallback settings report normal", Connection_Quality_For_Settings({3, 9}) == ConnectionQuality::Normal);
		Expect("extended conservative settings report bad", Connection_Quality_For_Settings({10, 250}) == ConnectionQuality::Bad);
		Expect("invalid settings report bad", Connection_Quality_For_Settings({0, 0}) == ConnectionQuality::Bad);
		Expect("extended fast-rung horizon reports bad", Connection_Quality_For_Settings({2, 8}) == ConnectionQuality::Bad);
	}


	void Test_Event_Semantics(void)
	{
		using namespace NetSemantic;

		Expect("zero index is valid", Index_Is_Valid(0, 8));
		Expect("last index is valid", Index_Is_Valid(7, 8));
		Expect("negative index is rejected", !Index_Is_Valid(-1, 8));
		Expect("one-past index is rejected", !Index_Is_Valid(8, 8));

		Expect("game speed zero remains 60 FPS", Game_Speed_Is_Valid(0));
		Expect("game speed six remains valid", Game_Speed_Is_Valid(6));
		Expect("negative game speed is rejected", !Game_Speed_Is_Valid(-1));
		Expect("game speed seven is rejected", !Game_Speed_Is_Valid(7));
		Expect("latency fudge zero is valid", Latency_Fudge_Is_Valid(0));
		Expect("latency fudge three is valid", Latency_Fudge_Is_Valid(3));
		Expect("latency fudge four is rejected", !Latency_Fudge_Is_Valid(4));

		Expect("animation sentinel is valid", Animation_Type_Is_Valid(-1, -1, 4));
		Expect("animation last index is valid", Animation_Type_Is_Valid(3, -1, 4));
		Expect("animation one-past index is rejected", !Animation_Type_Is_Valid(4, -1, 4));
		Expect("owner sentinel is valid", Animation_Owner_Is_Valid(-1, -1, 8));
		Expect("owner one-past index is rejected", !Animation_Owner_Is_Valid(8, -1, 8));

		Expect("resolved master is authorized", Timing_Authority_Is_Valid(2, 2));
		Expect("guest timing authority is rejected", !Timing_Authority_Is_Valid(3, 2));
		Expect("unresolved timing authority is rejected", !Timing_Authority_Is_Valid(2, -1));

		Expect("matching subject ownership is accepted", Subject_Owner_Is_Valid(3, 3));
		Expect("captured subject ownership is rejected", !Subject_Owner_Is_Valid(3, 4));
		Expect("missing subject owner is rejected", !Subject_Owner_Is_Valid(3, -1));

		Expect("legacy response-time minimum is accepted", Response_Time_Is_Valid(2, 2, 0, false));
		Expect("legacy response time above the compressed cap is accepted", Response_Time_Is_Valid(255, 2, 0, false));
		Expect("legacy response time below minimum is rejected", !Response_Time_Is_Valid(1, 2, 0, false));
		Expect("compressed response time accepts two aligned periods", Response_Time_Is_Valid(6, 2, 3, true));
		Expect("compressed response time accepts the highest aligned horizon", Response_Time_Is_Valid(250, 2, 10, true));
		Expect("compressed response time accepts the highest rate-three horizon", Response_Time_Is_Valid(249, 2, 3, true));
		Expect("compressed response time rejects wire maximum above the cap", !Response_Time_Is_Valid(255, 2, 3, true));
		Expect("compressed response time rejects an invalid period", !Response_Time_Is_Valid(6, 2, 0, true));
		Expect("compressed response time rejects one period", !Response_Time_Is_Valid(3, 2, 3, true));
		Expect("compressed response time rejects misalignment", !Response_Time_Is_Valid(7, 2, 3, true));

		std::optional<NetTiming::TimingSettings> settings = Decode_Timing_Settings(60, 9, 3);
		Expect("timing look-ahead decodes directly", settings && *settings == NetTiming::TimingSettings{3, 9});
		Expect("zero desired FPS is rejected", !Decode_Timing_Settings(0, 9, 3));
		Expect("desired FPS above 60 is rejected", !Decode_Timing_Settings(61, 9, 3));
		Expect("zero send period is rejected", !Decode_Timing_Settings(60, 9, 0));
		Expect("below-minimum horizon is rejected", !Decode_Timing_Settings(60, 2, 3));
		Expect("unaligned horizon is rejected", !Decode_Timing_Settings(60, 10, 3));
		Expect("aligned 250-frame horizon is valid", Decode_Timing_Settings(60, 250, 10).has_value());
		Expect("horizon above 250 is rejected", !Decode_Timing_Settings(60, 251, 10));
	}


	void Record_One(NetTiming::TimingReportCensus & census, NetTiming::Milliseconds rtt, std::uint32_t frame)
	{
		census.Record_Report(1, 10, rtt, frame);
	}


	void Test_Bootstrap_Cadence(void)
	{
		using namespace NetTiming;

		Expect("frame zero does not report", !Report_Is_Due(0));
		Expect("bootstrap reports at frame 32", Report_Is_Due(32));
		Expect("bootstrap reports at frame 64", Report_Is_Due(64));
		Expect("bootstrap does not add a frame 96 report", !Report_Is_Due(96));
		Expect("normal reports start at frame 128", Report_Is_Due(128));
		Expect("normal reports continue at frame 256", Report_Is_Due(256));
		Expect("normal reports continue at frame 384", Report_Is_Due(384));
		Expect("off-cadence reports remain disabled", !Report_Is_Due(385));

		Expect("frame zero does not evaluate", !Evaluation_Is_Due(0));
		Expect("reports alone do not evaluate at frame 32", !Evaluation_Is_Due(32));
		Expect("bootstrap evaluates at frame 64", Evaluation_Is_Due(64));
		Expect("bootstrap evaluates again at frame 128", Evaluation_Is_Due(128));
		Expect("normal evaluations start at frame 256", Evaluation_Is_Due(256));
		Expect("frame 384 is not an evaluation", !Evaluation_Is_Due(384));
		Expect("normal evaluations continue at frame 512", Evaluation_Is_Due(512));
	}


	void Test_Bootstrap_Policy(void)
	{
		using namespace NetTiming;

		TimingReportCensus low_reports;
		low_reports.Set_Player_Active(1, true, 0);
		BalancedTimingPolicy low;
		Expect("new policy starts in bootstrap", low.Is_Bootstrapping());
		Expect("bootstrap starts at 2/6", low.Current_Settings() == TimingSettings{2, 6});
		TimingEvaluation result = low.Evaluate(low_reports.Inspect(32), 60, 32);
		Expect("bootstrap does not evaluate before frame 64", !result.Evaluated);
		Record_One(low_reports, 0, 38);
		result = low.Evaluate(low_reports.Inspect(64), 60, 64);
		Expect("complete low-latency census finishes at frame 64", result.Evaluated && result.Changed && !low.Is_Bootstrapping());
		Expect("low-latency bootstrap jumps directly to 1/4", low.Current_Settings() == TimingSettings{1, 4});
		result = low.Evaluate(low_reports.Inspect(255), 60, 255);
		Expect("steady evaluation remains anchored before frame 256", !result.Evaluated);
		Record_One(low_reports, 0, 256);
		result = low.Evaluate(low_reports.Inspect(256), 60, 256);
		Expect("steady evaluation is anchored at frame 256", result.Evaluated && !result.Changed);

		Expect("100 ms would select 1/4 without bootstrap headroom",
			Select_Timing_Settings(100, 60, false) == TimingSettings{1, 4});
		Expect("100 ms retains 2/6 with bootstrap headroom",
			Select_Timing_Settings(100, 60, true) == TimingSettings{2, 6});
		TimingReportCensus marginal_reports;
		marginal_reports.Set_Player_Active(1, true, 0);
		Record_One(marginal_reports, 100, 38);
		BalancedTimingPolicy marginal;
		result = marginal.Evaluate(marginal_reports.Inspect(64), 60, 64);
		Expect("marginal bootstrap completes without changing 2/6", result.Evaluated && !result.Changed && !marginal.Is_Bootstrapping());

		TimingReportCensus high_reports;
		high_reports.Set_Player_Active(1, true, 0);
		Record_One(high_reports, 2000, 38);
		BalancedTimingPolicy high;
		result = high.Evaluate(high_reports.Inspect(64), 60, 64);
		Expect("high-latency bootstrap worsens directly", result.Changed && high.Current_Settings() == TimingSettings{10, 90});

		TimingReportCensus delayed_reports;
		delayed_reports.Set_Player_Active(1, true, 0);
		delayed_reports.Record_Report(1, 10, std::nullopt, 38);
		BalancedTimingPolicy delayed;
		result = delayed.Evaluate(delayed_reports.Inspect(64), 60, 64);
		Expect("incomplete frame 64 census keeps bootstrap open", result.Evaluated && !result.Changed && delayed.Is_Bootstrapping());
		delayed_reports.Record_Report(1, 10, 0, 70);
		result = delayed.Evaluate(delayed_reports.Inspect(100), 60, 100);
		Expect("completed census waits for frame 128", !result.Evaluated && delayed.Is_Bootstrapping());
		result = delayed.Evaluate(delayed_reports.Inspect(128), 60, 128);
		Expect("second bootstrap evaluation accepts a complete census", result.Evaluated && result.Changed && !delayed.Is_Bootstrapping());
		Expect("frame 128 completion selects the measured target", delayed.Current_Settings() == TimingSettings{1, 4});

		TimingReportCensus incomplete_reports;
		incomplete_reports.Set_Player_Active(1, true, 0);
		incomplete_reports.Record_Report(1, 10, std::nullopt, 38);
		BalancedTimingPolicy incomplete;
		incomplete.Evaluate(incomplete_reports.Inspect(64), 60, 64);
		incomplete_reports.Record_Report(1, 10, std::nullopt, 70);
		result = incomplete.Evaluate(incomplete_reports.Inspect(128), 60, 128);
		Expect("incomplete final census falls back immediately", result.Evaluated && result.Changed && !incomplete.Is_Bootstrapping());
		Expect("incomplete bootstrap falls back to 3/9", incomplete.Current_Settings() == TimingSettings{3, 9});

		TimingReportCensus lost_reports;
		lost_reports.Set_Player_Active(1, true, 0);
		lost_reports.Set_Player_Active(2, true, 0);
		lost_reports.Record_Report(1, 10, 20, 38);
		lost_reports.Record_Report(2, 10, std::nullopt, 38);
		BalancedTimingPolicy lost;
		result = lost.Evaluate(lost_reports.Inspect(64), 60, 64);
		Expect("initial missing RTT keeps bootstrap open", result.Evaluated && !result.Changed && lost.Is_Bootstrapping());
		lost_reports.Record_Report(1, 10, std::nullopt, 70);
		result = lost.Evaluate(lost_reports.Inspect(128), 60, 128);
		Expect("established RTT loss remains immediately conservative", result.Changed && lost.Current_Settings() == TimingSettings{10, 250});

		for (std::uint32_t frame : {256u, 512u, 768u}) {
			Record_One(high_reports, 0, frame);
			result = high.Evaluate(high_reports.Inspect(frame), 60, frame);
		}
		Expect("bootstrap cooldown leaves only two good evaluations by frame 768", !result.Changed && high.Good_Evaluations() == 2);
		Record_One(high_reports, 0, 1024);
		result = high.Evaluate(high_reports.Inspect(1024), 60, 1024);
		Expect("normal hysteresis resumes after bootstrap cooldown", result.Changed && high.Current_Settings() == TimingSettings{9, 27});

		high.Reset();
		Expect("reset starts a new bootstrap", high.Is_Bootstrapping());
		Expect("reset restores 2/6", high.Current_Settings() == TimingSettings{2, 6});

		BalancedTimingPolicy handoff;
		handoff.Reset_From({10, 70}, 0);
		Expect("handoff does not regain bootstrap", !handoff.Is_Bootstrapping());
		Record_One(high_reports, 0, 64);
		result = handoff.Evaluate(high_reports.Inspect(64), 60, 64);
		Expect("handoff ignores bootstrap evaluation", !result.Evaluated && handoff.Current_Settings() == TimingSettings{10, 70});

		TimingReportCensus resumed_reports;
		resumed_reports.Set_Player_Active(1, true, 1024);
		BalancedTimingPolicy resumed;
		resumed.Reset(1024);
		Expect_Equal("resumed bootstrap records its cadence origin", resumed.Cadence_Origin(), 1024u);
		result = resumed.Evaluate(resumed_reports.Inspect(1056), 60, 1056);
		Expect("resumed bootstrap does not evaluate after only 32 frames", !result.Evaluated);
		Record_One(resumed_reports, 0, 1062);
		result = resumed.Evaluate(resumed_reports.Inspect(1088), 60, 1088);
		Expect("resumed bootstrap evaluates after 64 frames", result.Evaluated && result.Changed && !resumed.Is_Bootstrapping());
		Expect("resumed bootstrap selects its measured target", resumed.Current_Settings() == TimingSettings{1, 4});
	}


	void Test_Hysteresis_And_Cooldown(void)
	{
		using namespace NetTiming;

		TimingReportCensus reports;
		reports.Set_Player_Active(1, true, 0);
		BalancedTimingPolicy policy;
		policy.Reset_From({3, 9}, 0);

		Record_One(reports, 0, 256);
		TimingEvaluation result = policy.Evaluate(reports.Inspect(256), 60, 256);
		Expect("first good evaluation does not change", !result.Changed);
		Record_One(reports, 0, 512);
		result = policy.Evaluate(reports.Inspect(512), 60, 512);
		Expect("second good evaluation does not change", !result.Changed);
		Record_One(reports, 0, 768);
		result = policy.Evaluate(reports.Inspect(768), 60, 768);
		Expect("third good evaluation improves one rung", result.Changed);
		Expect_Equal("one-rung improvement", policy.Current_Rung(), 2u);

		Record_One(reports, 0, 800);
		result = policy.Evaluate(reports.Inspect(800), 60, 800);
		Expect("evaluation interval enforced", !result.Evaluated);
		Expect_Equal("cooldown leaves rung", policy.Current_Rung(), 2u);

		BalancedTimingPolicy headroom;
		headroom.Reset_From({3, 9}, 0);
		TimingReportCensus edge;
		edge.Set_Player_Active(1, true, 0);
		for (std::uint32_t frame : {256u, 512u, 768u}) {
			Record_One(edge, 120, frame);
			headroom.Evaluate(edge.Inspect(frame), 60, frame);
		}
		Expect_Equal("20 percent headroom blocks marginal improvement", headroom.Current_Rung(), 3u);

		Record_One(reports, 2000, 1024);
		result = policy.Evaluate(reports.Inspect(1024), 60, 1024);
		Expect("worsening is immediate", result.Changed);
		Expect_Equal("worsening reaches required rung", policy.Current_Rung(), 10u);
		Expect_Equal("highest rung retains measured horizon",
			policy.Current_Settings().MaxAhead, 70u);

		for (std::uint32_t frame : {1280u, 1536u, 1792u}) {
			Record_One(reports, 1300, frame);
			result = policy.Evaluate(reports.Inspect(frame), 60, frame);
		}
		Expect("same-rung horizon reduction uses hysteresis", result.Changed);
		Expect_Equal("same-rung horizon retains aligned need",
			policy.Current_Settings().MaxAhead, 50u);
	}


	void Test_Stale_And_Long_Term_Recovery(void)
	{
		using namespace NetTiming;

		TimingReportCensus stale;
		stale.Set_Player_Active(1, true, 0);
		BalancedTimingPolicy stale_policy;
		stale_policy.Reset_From({3, 9}, 0);
		TimingEvaluation result = stale_policy.Evaluate(stale.Inspect(0), 60, 0);
		Expect("startup waits for a complete census", !result.Changed);
		Expect_Equal("startup keeps initial rung", stale_policy.Current_Rung(), 3u);

		stale.Record_Report(1, 10, 100, 256);
		stale_policy.Evaluate(stale.Inspect(256), 60, 256);
		result = stale_policy.Evaluate(stale.Inspect(256 + REPORT_EXPIRY), 60, 256 + REPORT_EXPIRY);
		Expect("established stale report worsens policy", result.Changed);
		Expect_Equal("established stale report chooses worst rung", stale_policy.Current_Rung(), 10u);
		Expect_Equal("established stale report chooses conservative horizon", stale_policy.Current_Settings().MaxAhead, MAXIMUM_MAX_AHEAD);

		stale.Set_Player_Active(1, false, 1024);
		for (std::uint32_t frame : {1024u, 1280u, 1536u}) {
			stale_policy.Evaluate(stale.Inspect(frame), 60, frame);
		}
		Expect_Equal("departed peer allows recovery", stale_policy.Current_Rung(), 9u);

		TimingReportCensus reports;
		reports.Set_Player_Active(1, true, 0);
		BalancedTimingPolicy policy;
		policy.Reset_From({3, 9}, 0);
		std::uint32_t frame = EVALUATION_INTERVAL;

		auto evaluate = [&](Milliseconds rtt) {
			Record_One(reports, rtt, frame);
			policy.Evaluate(reports.Inspect(frame), 60, frame);
			frame += EVALUATION_INTERVAL;
		};

		for (int cycle = 0; cycle < 5; cycle++) {
			evaluate(2000);
			evaluate(0);
			evaluate(0);
			evaluate(0);
		}
		Expect_Equal("repeated degradation and recovery remains stable", policy.Current_Rung(), 9u);
		evaluate(0);
		evaluate(0);
		evaluate(0);
		Expect_Equal("recovery remains possible after more than eight changes", policy.Current_Rung(), 8u);
	}


	void Test_Master_Handoff_State(void)
	{
		using namespace NetTiming;

		TimingReportCensus reports;
		reports.Set_Player_Active(1, true, 1000);
		BalancedTimingPolicy policy;
		policy.Reset_From({10, 70}, 1000);
		Expect("handoff restores authoritative settings", policy.Current_Settings() == TimingSettings{10, 70});
		Expect_Equal("handoff discards improvement evidence", policy.Good_Evaluations(), 0u);

		Record_One(reports, 0, 1000);
		TimingEvaluation result = policy.Evaluate(reports.Inspect(1000), 60, 1000);
		Expect("handoff starts an evaluation cooldown", !result.Evaluated);
		Record_One(reports, 0, 1256);
		result = policy.Evaluate(reports.Inspect(1256), 60, 1256);
		Expect("one good evaluation preserves the handoff target", result.Evaluated && !result.Changed && policy.Current_Settings() == TimingSettings{10, 70});

		TimingReportCensus recovery_reports;
		recovery_reports.Set_Player_Active(1, true, 0);
		BalancedTimingPolicy recover;
		recover.Reset_From({10, 250}, 0);
		for (std::uint32_t frame : {256u, 512u, 768u}) {
			Record_One(recovery_reports, 0, frame);
			result = recover.Evaluate(recovery_reports.Inspect(frame), 60, frame);
		}
		Expect("10/250 improves one rung after hysteresis", result.Changed && recover.Current_Settings() == TimingSettings{9, 27});

		TimingReportCensus same_rung_reports;
		same_rung_reports.Set_Player_Active(1, true, 0);
		BalancedTimingPolicy same_rung;
		same_rung.Reset_From({10, 70}, 0);
		for (std::uint32_t frame : {256u, 512u, 768u}) {
			Record_One(same_rung_reports, 1300, frame);
			result = same_rung.Evaluate(same_rung_reports.Inspect(frame), 60, frame);
		}
		Expect("10/70 catches up toward 10/50 after hysteresis", result.Changed && same_rung.Current_Settings() == TimingSettings{10, 50});

		TimingReportCensus legacy_reports;
		legacy_reports.Set_Player_Active(1, true, 0);
		legacy_reports.Record_Report(1, 10, 200, 256);
		BalancedTimingPolicy legacy;
		legacy.Reset_From({3, 6}, 0);
		result = legacy.Evaluate(legacy_reports.Inspect(256), 60, 256);
		Expect("adaptive policy recovers from a legacy two-period horizon", result.Changed && legacy.Current_Settings() == TimingSettings{3, 9});
	}


	void Test_Staged_Decrease(void)
	{
		using namespace NetTiming;

		std::optional<StagedTimingUpdate> staged = Stage_Timing_Update({3, 9}, {1, 4}, 100);
		Expect("decrease stages", staged && staged->Deferred);
		Expect_Equal("old horizon and periods align", staged->ActivationFrame, 111u);
		Expect_Equal("activation preserves most of the old horizon", staged->InitialMaxAhead, 6u);
		Expect("staged update not early", !Timing_Update_Is_Due(110, staged->ActivationFrame));
		Expect("staged update due", Timing_Update_Is_Due(111, staged->ActivationFrame));
		Expect_Equal("first catch-up step removes one new period", *Next_Transition_Max_Ahead({1, 6}, {1, 4}), 5u);
		Expect_Equal("second catch-up step reaches target", *Next_Transition_Max_Ahead({1, 5}, {1, 4}), 4u);
		Expect_Equal("catch-up stays at target", *Next_Transition_Max_Ahead({1, 4}, {1, 4}), 4u);

		staged = Stage_Timing_Update({3, 9}, {2, 6}, 100);
		Expect_Equal("both periods use LCM", staged->ActivationFrame, 114u);
		Expect_Equal("adjacent decrease activates at target horizon", staged->InitialMaxAhead, 6u);

		staged = Stage_Timing_Update({10, 250}, {9, 27}, 100);
		Expect_Equal("wide decrease aligns activation to both periods", staged->ActivationFrame, 360u);
		Expect_Equal("wide decrease preserves a safe initial horizon", staged->InitialMaxAhead, 243u);
		Expect_Equal("wide catch-up removes one new period", *Next_Transition_Max_Ahead({9, 243}, {9, 27}), 234u);

		staged = Stage_Timing_Update({10, 70}, {10, 50}, 100);
		Expect_Equal("same-rate decrease drains at old horizon", staged->ActivationFrame, 170u);
		Expect_Equal("same-rate decrease keeps one intermediate period", staged->InitialMaxAhead, 60u);
		Expect_Equal("same-rate catch-up reaches requested horizon", *Next_Transition_Max_Ahead({10, 60}, {10, 50}), 50u);

		staged = Stage_Timing_Update({9, 234}, {8, 24}, 360);
		Expect("replacement decrease restages from effective settings", staged && staged->Deferred);
		Expect_Equal("replacement decrease safely rebases its horizon", staged->InitialMaxAhead, 232u);

		std::optional<StagedTimingUpdate> immediate = Stage_Timing_Update({1, 4}, {5, 15}, 100);
		Expect("worsening applies immediately", immediate && !immediate->Deferred);
		Expect_Equal("immediate frame", immediate->ActivationFrame, 100u);
		Expect_Equal("immediate update uses requested horizon", immediate->InitialMaxAhead, 15u);
		staged = immediate;
		Expect("an immediate worse update replaces a pending decrease", staged && !staged->Deferred && staged->Settings == TimingSettings{5, 15});

		immediate = Stage_Timing_Update({9, 234}, {10, 250}, 360);
		Expect("conservative update cancels catch-up immediately", immediate && !immediate->Deferred && immediate->InitialMaxAhead == 250);

		Expect("zero-period staging rejected", !Stage_Timing_Update({0, 9}, {1, 4}, 100));
		Expect("unaligned staging rejected", !Stage_Timing_Update({3, 10}, {1, 4}, 100));
		std::optional<StagedTimingUpdate> const legacy_recovery = Stage_Timing_Update({3, 6}, {3, 9}, 100);
		Expect("legacy response horizon can recover immediately", legacy_recovery && !legacy_recovery->Deferred);
		Expect("overflowing staging rejected", !Stage_Timing_Update({10, 30}, {9, 27}, (std::numeric_limits<std::uint32_t>::max)() - 10));
		Expect("catch-up rejects mismatched send periods", !Next_Transition_Max_Ahead({9, 243}, {8, 24}));
		Expect("catch-up rejects invalid effective settings", !Next_Transition_Max_Ahead({9, 242}, {9, 27}));
	}


	struct TransitionTrace
	{
		std::vector<std::pair<std::uint32_t, NetTiming::TimingSettings>> Changes;
		std::vector<std::uint64_t> CommandTargets;

		bool operator==(TransitionTrace const &) const = default;
	};


	TransitionTrace Run_Transition(NetTiming::TimingSettings current, NetTiming::TimingSettings requested, std::uint32_t event_frame, std::uint32_t final_frame)
	{
		TransitionTrace trace;
		std::optional<NetTiming::StagedTimingUpdate> const plan = NetTiming::Stage_Timing_Update(current, requested, event_frame);
		if (!plan || !plan->Deferred) {
			return(trace);
		}

		NetTiming::TimingTransitionState transition{*plan};
		std::uint32_t const first_frame = event_frame - event_frame % current.FrameSendRate;
		for (std::uint32_t frame = first_frame; frame <= final_frame; frame++) {
			std::optional<NetTiming::TimingTransitionAdvance> const advance = NetTiming::Advance_Timing_Transition(transition, current, frame);
			if (!advance) {
				trace.CommandTargets.clear();
				return(trace);
			}
			if (advance->Changed) {
				current = advance->Settings;
				trace.Changes.emplace_back(frame, current);
			}
			if (frame % current.FrameSendRate == 0) {
				trace.CommandTargets.push_back(static_cast<std::uint64_t>(frame) + current.MaxAhead);
			}
			if (advance->Complete) {
				break;
			}
		}
		return(trace);
	}


	void Test_Transition_Sequences(void)
	{
		using namespace NetTiming;

		for (std::pair<TimingSettings, TimingSettings> const & transition : {
			std::pair{TimingSettings{10, 250}, TimingSettings{9, 27}},
			std::pair{TimingSettings{10, 70}, TimingSettings{10, 50}},
			std::pair{TimingSettings{3, 9}, TimingSettings{2, 6}},
			std::pair{TimingSettings{2, 6}, TimingSettings{1, 4}}}) {
			TransitionTrace const live = Run_Transition(transition.first, transition.second, 100, 700);
			TransitionTrace const replay = Run_Transition(transition.first, transition.second, 100, 700);
			Expect("live and replay transition steps are identical", live == replay);
			Expect("a transition reaches its requested settings", !live.Changes.empty() && live.Changes.back().second == transition.second);
			bool nondecreasing = !live.CommandTargets.empty();
			for (std::size_t index = 1; index < live.CommandTargets.size(); index++) {
				nondecreasing = nondecreasing && live.CommandTargets[index] >= live.CommandTargets[index - 1];
			}
			Expect("transition command targets never move backward", nondecreasing);
		}

		std::optional<StagedTimingUpdate> const plan = Stage_Timing_Update({10, 250}, {9, 27}, 100);
		TimingTransitionState state{*plan};
		TimingSettings current{10, 250};
		for (std::uint32_t frame = 100; frame <= 369; frame++) {
			std::optional<TimingTransitionAdvance> const advance = Advance_Timing_Transition(state, current, frame);
			if (advance && advance->Changed) {
				current = advance->Settings;
			}
		}
		std::optional<StagedTimingUpdate> const replacement = Stage_Timing_Update(current, {8, 24}, 369);
		Expect("an active catch-up can be safely replaced", replacement && replacement->Deferred && replacement->InitialMaxAhead >= current.MaxAhead - current.FrameSendRate);
		std::optional<StagedTimingUpdate> const conservative = Stage_Timing_Update(current, {10, 250}, 369);
		Expect("a fully conservative replacement applies immediately", conservative && !conservative->Deferred);
	}
}


int main(void)
{
	Test_Rtt_Estimator();
	Test_Clock_And_Wrap();
	Test_Retransmit_Backoff();
	Test_Loss_Jitter_And_Reordering();
	Test_Census();
	Test_Rungs();
	Test_Connection_Quality();
	Test_Event_Semantics();
	Test_Bootstrap_Cadence();
	Test_Bootstrap_Policy();
	Test_Hysteresis_And_Cooldown();
	Test_Stale_And_Long_Term_Recovery();
	Test_Master_Handoff_State();
	Test_Staged_Decrease();
	Test_Transition_Sequences();

	if (Failures != 0) {
		std::cerr << Failures << " network timing checks failed\n";
		return(1);
	}

	std::cout << "All network timing checks passed\n";
	return(0);
}
