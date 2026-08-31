/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "nettiming.h"

#include <cstdint>
#include <initializer_list>
#include <iostream>
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
				if (!NetTiming::Retransmit_Is_Due(LastSend, now, BaseRto, TransmissionCount - 1, NetTiming::MINIMUM_CONNECTION_TIMEOUT)) {
					return(false);
				}
				LastSend = now;
				TransmissionCount++;
				Estimator.Note_Retransmit(BaseRto, now);
				return(true);
			}

			bool Acknowledge(NetTiming::Milliseconds now)
			{
				Clock.Set(now);
				return(Estimator.Acknowledge(FirstSend, TransmissionCount, Clock));
			}

			NetTiming::RttEstimator const & Rtt(void) const {return(Estimator);}
			NetTiming::Milliseconds Base_Rto(void) const {return(BaseRto);}

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
		Expect("unequal links keep independent RTOs", fast_link.Retransmit_Timeout() < slow_link.Retransmit_Timeout());

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


	void Test_Retry_Decisions(void)
	{
		using namespace NetTiming;

		RetransmitState state;
		Expect("new packet sends immediately", Evaluate_Retry(state, 1000, 800, 2000, true, true) == RetryDecision::SEND);

		state = {1000, 1000, 100, 1};
		Expect("adaptive packet keeps captured RTO", Evaluate_Retry(state, 1099, 800, 2000, true, true) == RetryDecision::WAIT);
		Expect("adaptive packet sends at captured RTO", Evaluate_Retry(state, 1100, 800, 2000, true, true) == RetryDecision::SEND);

		state = {1000, 1100, 100, 2};
		Expect("adaptive retry waits through backoff", Evaluate_Retry(state, 1299, 800, 2000, true, true) == RetryDecision::WAIT);
		Expect("adaptive retry sends after backoff", Evaluate_Retry(state, 1300, 800, 2000, true, true) == RetryDecision::SEND);

		state = {1000, 1000, 100, 4};
		Expect("fixed channel uses current retry delay", Evaluate_Retry(state, 1399, 400, 2000, true, false) == RetryDecision::WAIT);
		Expect("fixed channel does not back off", Evaluate_Retry(state, 1400, 400, 2000, true, false) == RetryDecision::SEND);

		state = {1000, 1900, 100, 1};
		Expect("connection timeout wins over retry", Evaluate_Retry(state, 3000, 100, 2000, true, true) == RetryDecision::TIMED_OUT);
		Expect("disabled connection timeout still retries", Evaluate_Retry(state, 3000, 100, 2000, false, true) == RetryDecision::SEND);

		state = {0xffffff00u, 0xfffffff0u, 100, 1};
		Expect("retry decision handles clock wrap", Evaluate_Retry(state, 0x00000054u, 800, 2000, true, true) == RetryDecision::SEND);
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
		Expect("duplicate ambiguous ACK is excluded by Karn", !reordered.Acknowledge(1500, 2, clock));
		Expect_Equal("ambiguous ACK leaves SRTT unchanged", reordered.Smoothed_Rtt(), 125u);

		RttEstimator jitter;
		for (Milliseconds sample : {20u, 400u, 35u, 350u, 40u}) {
			jitter.Add_Sample(sample);
		}
		Expect("jitter raises variation", jitter.Rtt_Variation() > 0);
		Expect("jittered RTO remains bounded", jitter.Retransmit_Timeout() >= MINIMUM_RTO && jitter.Retransmit_Timeout() <= MAXIMUM_RTO);

		Expect("loss does not retransmit before the base RTO", !Retransmit_Is_Due(1000, 1099, 100, 0, 2000));
		Expect("first loss retransmits at the base RTO", Retransmit_Is_Due(1000, 1100, 100, 0, 2000));
		Expect("second loss waits for exponential backoff", !Retransmit_Is_Due(1100, 1299, 100, 1, 2000));
		Expect("second loss retransmits at doubled RTO", Retransmit_Is_Due(1100, 1300, 100, 1, 2000));

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


	void Test_Backoff_Persistence(void)
	{
		using namespace NetTiming;

		FakeTransport transport;
		transport.Send(0);
		Expect("fast link samples cleanly", transport.Acknowledge(10));
		Expect_Equal("fast link floors the RTO", transport.Rtt().Retransmit_Timeout(), MINIMUM_RTO);

		// The link now takes 500 ms, so every packet is retransmitted before its ACK arrives.
		transport.Send(1000);
		Expect("first era retransmits at the floor RTO", transport.Retry(1100));
		Expect_Equal("first era doubles the RTO", transport.Rtt().Retransmit_Timeout(), 200u);
		Expect("same era retransmits again", transport.Retry(1300));
		Expect_Equal("same era does not double twice", transport.Rtt().Retransmit_Timeout(), 200u);
		Expect("ambiguous ACK is excluded", !transport.Acknowledge(1500));

		transport.Send(2000);
		Expect_Equal("new packet captures the backed off RTO", transport.Base_Rto(), 200u);
		Expect("second era retransmits", transport.Retry(2200));
		Expect_Equal("second era doubles the RTO", transport.Rtt().Retransmit_Timeout(), 400u);

		transport.Send(3000);
		Expect_Equal("third packet captures the backed off RTO", transport.Base_Rto(), 400u);
		Expect("third era retransmits", transport.Retry(3400));
		Expect_Equal("third era doubles the RTO", transport.Rtt().Retransmit_Timeout(), 800u);

		// The RTO now exceeds the real round trip, so a first transmission is acknowledged.
		transport.Send(4000);
		Expect("backed off RTO lets a clean sample through", transport.Acknowledge(4500));
		Expect_Equal("recovered smoothed RTT", transport.Rtt().Smoothed_Rtt(), 71u);
		Expect_Equal("recovered variation", transport.Rtt().Rtt_Variation(), 126u);
		Expect_Equal("recovered RTO covers the slower link", transport.Rtt().Retransmit_Timeout(), 575u);
		Expect("recovered estimate is fresh", transport.Rtt().Has_Fresh_Sample(4500));
	}


	void Test_Sample_Staleness(void)
	{
		using namespace NetTiming;

		RttEstimator idle;
		Expect("unsampled estimator is never fresh", !idle.Has_Fresh_Sample(0));
		idle.Add_Sample(100);
		Expect("quiet link stays fresh indefinitely", idle.Has_Fresh_Sample(1000000));

		RttEstimator starved;
		starved.Add_Sample(100);
		starved.Note_Retransmit(starved.Retransmit_Timeout(), 1000);
		Expect("estimate is fresh before the lifetime", starved.Has_Fresh_Sample(1000 + RTT_SAMPLE_LIFETIME - 1));
		Expect("estimate is stale at the lifetime", !starved.Has_Fresh_Sample(1000 + RTT_SAMPLE_LIFETIME));
		Expect("stale estimate still paces retries", starved.Has_Sample());

		FakeClock clock;
		clock.Set(9500);
		Expect("clean ACK restores the estimate", starved.Acknowledge(9400, 1, clock));
		Expect("restored estimate is fresh again", starved.Has_Fresh_Sample(1000000));

		starved.Reset();
		Expect("reset clears freshness", !starved.Has_Fresh_Sample(0));

		RttEstimator wrapped;
		wrapped.Add_Sample(100);
		wrapped.Note_Retransmit(wrapped.Retransmit_Timeout(), 0xfffff000u);
		Expect("freshness survives the clock wrap", wrapped.Has_Fresh_Sample(0));
		Expect("staleness is measured across the wrap", !wrapped.Has_Fresh_Sample(0x00001000u));
	}


	void Test_Note_Retransmit_Guards(void)
	{
		using namespace NetTiming;

		RttEstimator unsampled;
		unsampled.Note_Retransmit(MINIMUM_RTO, 1000);
		Expect("retransmission does not invent a sample", !unsampled.Has_Sample());
		Expect_Equal("unsampled RTO is unchanged", unsampled.Retransmit_Timeout(), MINIMUM_RTO);

		RttEstimator ceiling;
		ceiling.Add_Sample(300);
		Expect_Equal("sampled RTO", ceiling.Retransmit_Timeout(), 900u);
		ceiling.Note_Retransmit(900, 1000);
		Expect_Equal("backoff doubles below the ceiling", ceiling.Retransmit_Timeout(), 1800u);
		ceiling.Note_Retransmit(1800, 2000);
		Expect_Equal("backoff clamps at the ceiling", ceiling.Retransmit_Timeout(), MAXIMUM_RTO);
		ceiling.Note_Retransmit(MAXIMUM_RTO, 3000);
		Expect_Equal("backoff stays at the ceiling", ceiling.Retransmit_Timeout(), MAXIMUM_RTO);

		// A packet captured during backoff can double a freshly lowered RTO once.
		RttEstimator recovered;
		recovered.Add_Sample(300);
		recovered.Note_Retransmit(900, 1000);
		recovered.Add_Sample(300);
		Expect_Equal("clean sample lowers the RTO", recovered.Retransmit_Timeout(), 752u);
		recovered.Note_Retransmit(1800, 2000);
		Expect_Equal("stale capture doubles the RTO once", recovered.Retransmit_Timeout(), 1504u);
	}
}


int main(void)
{
	Test_Rtt_Estimator();
	Test_Clock_And_Wrap();
	Test_Retransmit_Backoff();
	Test_Retry_Decisions();
	Test_Loss_Jitter_And_Reordering();
	Test_Backoff_Persistence();
	Test_Sample_Staleness();
	Test_Note_Retransmit_Guards();

	if (Failures != 0) {
		std::cerr << Failures << " network timing checks failed\n";
		return(1);
	}

	std::cout << "All network timing checks passed\n";
	return(0);
}
