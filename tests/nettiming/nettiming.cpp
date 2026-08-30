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
#include <string>


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
		Expect("activate first peer", census.Set_Player_Active(1, true));
		Expect("activate second peer", census.Set_Player_Active(2, true));
		Expect("reject out of range peer", !census.Set_Player_Active(MAX_TIMING_PLAYERS, true));
		Expect("record first peer", census.Record_Report(1, 80, 100));
		Expect("record second peer", census.Record_Report(2, 180, 100));
		Expect("accept RTT above retransmit clamp", census.Record_Report(2, MAXIMUM_RTO + 1, 100));
		Expect("reject RTT beyond wire range", !census.Record_Report(2, MAXIMUM_REPORTED_RTT + 1, 100));

		TimingCensus result = census.Inspect(200);
		Expect_Equal("active peer count", result.ActivePlayers, 2u);
		Expect_Equal("fresh report count", result.FreshReports, 2u);
		Expect_Equal("unequal links publish worst", result.WorstRoundTrip, MAXIMUM_RTO + 1);
		Expect("fresh census complete", result.Complete);
		BalancedTimingPolicy aggregate;
		TimingEvaluation const guest_degradation = aggregate.Evaluate(
			result, 60, LatencyFudge::None, 200);
		Expect("a guest-to-guest slow path worsens the master policy",
			guest_degradation.Changed && guest_degradation.Rung == MAXIMUM_TIMING_RUNG);

		result = census.Inspect(100 + REPORT_EXPIRY);
		Expect("reports expire on boundary", !result.Complete);
		Expect_Equal("expired reports not fresh", result.FreshReports, 0u);

		Expect("departed peer removed", census.Set_Player_Active(2, false));
		Expect("remaining peer refreshed", census.Record_Report(1, 90, 700));
		result = census.Inspect(700);
		Expect("departure restores complete census", result.Complete);
		Expect_Equal("departed peer excluded", result.ActivePlayers, 1u);
		Expect_Equal("remaining peer wins census", result.WorstRoundTrip, 90u);
		Expect("clear unavailable report", census.Clear_Report(1));
		Expect("cleared active report makes census incomplete", !census.Inspect(700).Complete);
	}


	void Test_Rungs_And_Fudge(void)
	{
		using namespace NetTiming;

		Expect_Equal("initial FSR", Settings_For_Rung(INITIAL_TIMING_RUNG).FrameSendRate, 3u);
		Expect_Equal("initial MaxAhead", Settings_For_Rung(INITIAL_TIMING_RUNG).MaxAhead, 9u);
		Expect_Equal("best rung MaxAhead", Settings_For_Rung(1).MaxAhead, 4u);
		Expect_Equal("worst rung MaxAhead", Settings_For_Rung(10).MaxAhead, 30u);
		Expect("rung settings valid", Timing_Settings_Are_Valid(Settings_For_Rung(10)));
		Expect("below-rung minimum invalid", !Timing_Settings_Are_Valid({3, 6}));
		Expect("unaligned settings invalid", !Timing_Settings_Are_Valid({3, 10}));

		Expect_Equal("no latency fudge", Apply_Latency_Fudge(100, LatencyFudge::None), 100u);
		Expect_Equal("half latency fudge", Apply_Latency_Fudge(100, LatencyFudge::Half), 150u);
		Expect_Equal("double latency fudge", Apply_Latency_Fudge(100, LatencyFudge::Double), 200u);
		Expect_Equal("triple latency fudge", Apply_Latency_Fudge(100, LatencyFudge::Triple), 300u);
		Expect_Equal("half fudge rounds up", Apply_Latency_Fudge(1, LatencyFudge::Half), 2u);

		Expect_Equal("zero RTT selects best rung", Select_Timing_Rung(0, 60, LatencyFudge::None), 1u);
		Expect_Equal("100 ms fits best rung", Select_Timing_Rung(100, 60, LatencyFudge::None), 1u);
		Expect_Equal("101 ms advances a rung", Select_Timing_Rung(101, 60, LatencyFudge::None), 2u);
		Expect_Equal("300 ms selects balanced rung", Select_Timing_Rung(300, 60, LatencyFudge::None), 5u);
		Expect_Equal("fudge raises selected rung", Select_Timing_Rung(100, 60, LatencyFudge::Half), 3u);
		TimingSettings const high_rtt = Select_Timing_Settings(2000, 60, LatencyFudge::None);
		Expect_Equal("two-second RTT selects highest FSR", high_rtt.FrameSendRate, 10u);
		Expect_Equal("two-second RTT carries needed aligned MaxAhead", high_rtt.MaxAhead, 70u);
		TimingSettings const capped = Select_Timing_Settings(
			MAXIMUM_REPORTED_RTT, 60, LatencyFudge::Triple);
		Expect_Equal("wire-maximum RTT selects highest FSR", capped.FrameSendRate, 10u);
		Expect_Equal("highest rung caps at largest aligned horizon", capped.MaxAhead, 250u);

		Expect("alignment rejects zero period", !Align_Max_Ahead(10, 0));
		Expect_Equal("alignment reaches cap", *Align_Max_Ahead(249, 10), 250u);
		Expect("alignment rejects over cap", !Align_Max_Ahead(250, 9));
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

		std::optional<NetTiming::TimingSettings> settings = Decode_Timing_Settings(60, 9, 3);
		Expect("timing look-ahead decodes directly", settings && *settings == NetTiming::TimingSettings{3, 9});
		Expect("zero desired FPS is rejected", !Decode_Timing_Settings(0, 9, 3));
		Expect("desired FPS above 60 is rejected", !Decode_Timing_Settings(61, 9, 3));
		Expect("zero send period is rejected", !Decode_Timing_Settings(60, 9, 0));
		Expect("below-minimum horizon is rejected", !Decode_Timing_Settings(60, 2, 3));
		Expect("unaligned horizon is rejected", !Decode_Timing_Settings(60, 10, 3));
		Expect("aligned 250-frame horizon is valid", Decode_Timing_Settings(60, 250, 10).has_value());
		Expect("horizon above 250 is rejected", !Decode_Timing_Settings(60, 251, 10));

		Expect("bounded report is valid", Network_Report_Is_Valid(1000, 65534));
		Expect("unavailable RTT sentinel is valid", Network_Report_Is_Valid(0, UINT16_MAX));
		Expect("process time above engine cap is rejected", !Network_Report_Is_Valid(1001, 10));
	}


	void Record_One(NetTiming::TimingReportCensus & census, NetTiming::Milliseconds rtt,
		std::uint32_t frame)
	{
		census.Record_Report(1, rtt, frame);
	}


	void Test_Hysteresis_And_Cooldown(void)
	{
		using namespace NetTiming;

		TimingReportCensus reports;
		reports.Set_Player_Active(1, true);
		BalancedTimingPolicy policy;

		Record_One(reports, 0, 0);
		TimingEvaluation result = policy.Evaluate(reports.Inspect(0), 60, LatencyFudge::None, 0);
		Expect("first good evaluation does not change", !result.Changed);
		Record_One(reports, 0, 256);
		result = policy.Evaluate(reports.Inspect(256), 60, LatencyFudge::None, 256);
		Expect("second good evaluation does not change", !result.Changed);
		Record_One(reports, 0, 512);
		result = policy.Evaluate(reports.Inspect(512), 60, LatencyFudge::None, 512);
		Expect("third good evaluation improves one rung", result.Changed);
		Expect_Equal("one-rung improvement", policy.Current_Rung(), 2u);

		Record_One(reports, 0, 600);
		result = policy.Evaluate(reports.Inspect(600), 60, LatencyFudge::None, 600);
		Expect("evaluation interval enforced", !result.Evaluated);
		Expect_Equal("cooldown leaves rung", policy.Current_Rung(), 2u);

		BalancedTimingPolicy headroom;
		TimingReportCensus edge;
		edge.Set_Player_Active(1, true);
		for (std::uint32_t frame : {0u, 256u, 512u}) {
			Record_One(edge, 120, frame);
			headroom.Evaluate(edge.Inspect(frame), 60, LatencyFudge::None, frame);
		}
		Expect_Equal("20 percent headroom blocks marginal improvement", headroom.Current_Rung(), 3u);

		Record_One(reports, 2000, 768);
		result = policy.Evaluate(reports.Inspect(768), 60, LatencyFudge::None, 768);
		Expect("worsening is immediate", result.Changed);
		Expect_Equal("worsening reaches required rung", policy.Current_Rung(), 10u);
		Expect_Equal("highest rung retains measured horizon",
			policy.Current_Settings().MaxAhead, 70u);

		for (std::uint32_t frame : {1024u, 1280u, 1536u}) {
			Record_One(reports, 1300, frame);
			result = policy.Evaluate(reports.Inspect(frame), 60, LatencyFudge::None, frame);
		}
		Expect("same-rung horizon reduction uses hysteresis", result.Changed);
		Expect_Equal("same-rung horizon retains aligned need",
			policy.Current_Settings().MaxAhead, 50u);
	}


	void Test_Stale_And_Transition_Budget(void)
	{
		using namespace NetTiming;

		TimingReportCensus stale;
		stale.Set_Player_Active(1, true);
		BalancedTimingPolicy stale_policy;
		TimingEvaluation result = stale_policy.Evaluate(stale.Inspect(0), 60, LatencyFudge::None, 0);
		Expect("startup waits for a complete census", !result.Changed);
		Expect_Equal("startup keeps initial rung", stale_policy.Current_Rung(), 3u);

		stale.Record_Report(1, 100, 256);
		stale_policy.Evaluate(stale.Inspect(256), 60, LatencyFudge::None, 256);
		result = stale_policy.Evaluate(stale.Inspect(256 + REPORT_EXPIRY), 60,
			LatencyFudge::None, 256 + REPORT_EXPIRY);
		Expect("established stale report worsens policy", result.Changed);
		Expect_Equal("established stale report chooses worst rung", stale_policy.Current_Rung(), 10u);

		stale.Set_Player_Active(1, false);
		for (std::uint32_t frame : {1024u, 1280u, 1536u}) {
			stale_policy.Evaluate(stale.Inspect(frame), 60, LatencyFudge::None, frame);
		}
		Expect_Equal("departed peer allows recovery", stale_policy.Current_Rung(), 9u);

		TimingReportCensus reports;
		reports.Set_Player_Active(1, true);
		BalancedTimingPolicy policy;
		std::uint32_t frame = 0;

		auto evaluate = [&](Milliseconds rtt) {
			Record_One(reports, rtt, frame);
			policy.Evaluate(reports.Inspect(frame), 60, LatencyFudge::None, frame);
			frame += EVALUATION_INTERVAL;
		};

		evaluate(2000); // 1: 3 -> 10
		for (int cycle = 0; cycle < 3; cycle++) {
			evaluate(0);
			evaluate(0);
			evaluate(0);    // even transition: 10 -> 9
			evaluate(2000); // odd transition: 9 -> 10
		}
		evaluate(0);
		evaluate(0);
		evaluate(0); // 8: 10 -> 9

		Expect_Equal("transition budget reached", policy.Reversible_Changes(), REVERSIBLE_CHANGE_LIMIT);
		Expect_Equal("eighth transition leaves rung nine", policy.Current_Rung(), 9u);
		for (int i = 0; i < 6; i++) {
			evaluate(0);
		}
		Expect_Equal("budget locks further improvement", policy.Current_Rung(), 9u);
		evaluate(2000);
		Expect_Equal("worsening remains available after budget", policy.Current_Rung(), 10u);
	}


	void Test_Staged_Decrease(void)
	{
		using namespace NetTiming;

		std::optional<StagedTimingUpdate> staged = Stage_Timing_Update({3, 9}, {1, 4}, 100);
		Expect("decrease stages", staged && staged->Deferred);
		Expect_Equal("old horizon and periods align", staged->ActivationFrame, 111u);
		Expect("staged update not early", !Timing_Update_Is_Due(110, staged->ActivationFrame));
		Expect("staged update due", Timing_Update_Is_Due(111, staged->ActivationFrame));

		staged = Stage_Timing_Update({3, 9}, {2, 6}, 100);
		Expect_Equal("both periods use LCM", staged->ActivationFrame, 114u);

		std::optional<StagedTimingUpdate> immediate = Stage_Timing_Update({1, 4}, {5, 15}, 100);
		Expect("worsening applies immediately", immediate && !immediate->Deferred);
		Expect_Equal("immediate frame", immediate->ActivationFrame, 100u);
		staged = immediate;
		Expect("an immediate worse update replaces a pending decrease",
			staged && !staged->Deferred && staged->Settings == TimingSettings{5, 15});

		Expect("zero-period staging rejected", !Stage_Timing_Update({0, 9}, {1, 4}, 100));
		Expect("unaligned staging rejected", !Stage_Timing_Update({3, 10}, {1, 4}, 100));
		Expect("overflowing staging rejected", !Stage_Timing_Update({10, 30}, {9, 27},
			std::numeric_limits<std::uint32_t>::max() - 10));
	}
}


int main(void)
{
	Test_Rtt_Estimator();
	Test_Clock_And_Wrap();
	Test_Retransmit_Backoff();
	Test_Loss_Jitter_And_Reordering();
	Test_Census();
	Test_Rungs_And_Fudge();
	Test_Event_Semantics();
	Test_Hysteresis_And_Cooldown();
	Test_Stale_And_Transition_Budget();
	Test_Staged_Decrease();

	if (Failures != 0) {
		std::cerr << Failures << " network timing checks failed\n";
		return(1);
	}

	std::cout << "All network timing checks passed\n";
	return(0);
}
