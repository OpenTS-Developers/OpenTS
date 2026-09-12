/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "connect.h"

#include <deque>
#include <iostream>
#include <string>
#include <utility>
#include <vector>


namespace
{
	class TestClock final : public NetTiming::MillisecondClock
	{
		public:
			NetTiming::Milliseconds Now(void) const override {return(Current);}
			NetTiming::Milliseconds Current = 0;
	};


	class TestTransport
	{
		public:
			explicit TestTransport(TestClock const & clock) : Clock(clock) {}

			void Send(ConnectionClass * destination, char const * buffer, int length)
			{
				if (Connected) {
					Packets.push_back({destination, {buffer, buffer + length}, Clock.Now() + OneWayDelay});
				}
			}

			void Deliver(void)
			{
				while (!Packets.empty() && Packets.front().Arrival <= Clock.Now()) {
					Packet packet = std::move(Packets.front());
					Packets.pop_front();
					packet.Destination->Receive_Packet(packet.Bytes.data(), static_cast<int>(packet.Bytes.size()));
				}
			}

			bool Connected = true;
			NetTiming::Milliseconds OneWayDelay = 5;

		private:
			struct Packet
			{
				ConnectionClass * Destination;
				std::vector<char> Bytes;
				NetTiming::Milliseconds Arrival;
			};

			TestClock const & Clock;
			std::deque<Packet> Packets;
	};


	class TestConnection final : public ConnectionClass
	{
		public:
			TestConnection(TestClock const & clock, TestTransport & transport, int capacity)
				: ConnectionClass(capacity, capacity, sizeof(int), 1234, 6, -1, 120, 0, &clock), Transport(transport)
			{
				Init();
			}

			void Read(void)
			{
				int payload;
				int length;
				while (Get_Packet(&payload, sizeof(payload), &length)) {
					Received.push_back(payload);
				}
			}

			NetTiming::RttEstimator const & Rtt(void) const {return(RoundTripEstimator);}

			ConnectionClass * Remote = nullptr;
			std::vector<int> Received;

		private:
			int Send(char * buffer, int length, void *, int) override
			{
				Transport.Send(Remote, buffer, length);
				return(1);
			}

			TestTransport & Transport;
	};


	class ConnectionFixture
	{
		public:
			explicit ConnectionFixture(int capacity) : Transport(Clock), First(Clock, Transport, capacity), Second(Clock, Transport, capacity)
			{
				First.Remote = &Second;
				Second.Remote = &First;
			}

			void Service(void)
			{
				Transport.Deliver();
				First.Service();
				Second.Service();
				First.Read();
				Second.Read();
			}

			void Advance_To(NetTiming::Milliseconds time)
			{
				while (Clock.Current < time) {
					Clock.Current++;
					Service();
				}
			}

			TestClock Clock;
			TestTransport Transport;
			TestConnection First;
			TestConnection Second;
	};


	int Failures = 0;


	void Expect(std::string const & name, bool condition)
	{
		if (!condition) {
			std::cerr << name << " failed\n";
			Failures++;
		}
	}


	void Establish_Rtt(ConnectionFixture & fixture)
	{
		int payload = 0;
		Expect("first peer queues its initial packet", fixture.First.Send_Packet(&payload, sizeof(payload), true) != 0);
		Expect("second peer queues its initial packet", fixture.Second.Send_Packet(&payload, sizeof(payload), true) != 0);
		fixture.Service();
		fixture.Advance_To(10);
		Expect("both connections measure the initial round trip", fixture.First.Rtt().Smoothed_Rtt() == 10 && fixture.Second.Rtt().Smoothed_Rtt() == 10);
	}


	void Test_Unsampled_Retry(void)
	{
		ConnectionFixture fixture(2);
		fixture.Transport.Connected = false;
		int payload = 0;
		fixture.First.Send_Packet(&payload, sizeof(payload), true);
		fixture.Service();
		Expect("initial send needs no elapsed clock time", fixture.First.Queue->Get_Send(0)->SendCount == 1);
		fixture.Advance_To(99);
		Expect("unsampled connection retains its legacy retry delay", fixture.First.Queue->Get_Send(0)->SendCount == 1);
		fixture.Advance_To(100);
		Expect("unsampled connection retries after six engine ticks", fixture.First.Queue->Get_Send(0)->SendCount == 2);
		Expect("retry alone does not establish RTT", !fixture.First.Rtt().Has_Sample());
	}


	void Test_Saturated_Outage_Recovery(int capacity)
	{
		ConnectionFixture fixture(capacity);
		Establish_Rtt(fixture);
		Expect("read packets retain the latest sequence entry", fixture.First.Queue->Num_Receive() == 1 && fixture.Second.Queue->Num_Receive() == 1);
		fixture.Advance_To(100);
		fixture.Transport.Connected = false;
		for (int payload = 1; payload <= capacity; payload++) {
			Expect("first peer fills its send queue", fixture.First.Send_Packet(&payload, sizeof(payload), true) != 0);
			Expect("second peer fills its send queue", fixture.Second.Send_Packet(&payload, sizeof(payload), true) != 0);
		}
		fixture.Service();
		fixture.Advance_To(3000);
		Expect("the outage times out both peers", fixture.First.Is_Bad() && fixture.Second.Is_Bad());
		fixture.Transport.Connected = true;
		fixture.Advance_To(8000);

		std::vector<int> expected;
		for (int payload = 0; payload <= capacity; payload++) {
			expected.push_back(payload);
		}
		Expect("first peer receives the complete ordered backlog", fixture.First.Received == expected);
		Expect("second peer receives the complete ordered backlog", fixture.Second.Received == expected);
		Expect("restored connectivity drains both send queues", fixture.First.Queue->Num_Send() == 0 && fixture.Second.Queue->Num_Send() == 0);
		Expect("both connections recover from timeout", !fixture.First.Is_Bad() && !fixture.Second.Is_Bad());
	}


	void Test_Latency_Increase(void)
	{
		ConnectionFixture fixture(2);
		Establish_Rtt(fixture);
		fixture.Transport.OneWayDelay = 300;
		for (int payload = 1; payload <= 12; payload++) {
			Expect("slower link accepts another packet", fixture.First.Send_Packet(&payload, sizeof(payload), true) != 0);
			fixture.Service();
			fixture.Advance_To(fixture.Clock.Now() + 1000);
		}
		Expect("production retry capture lets the slower link become measurable", fixture.First.Rtt().Smoothed_Rtt() > 100);
		Expect("slower link obtains an unambiguous estimate", fixture.First.Rtt().Has_Sample() && !fixture.First.Rtt().Is_Provisional());
		Expect("slower link delivers every packet", fixture.Second.Received.size() == 13);
		Expect("slower link completes its acknowledgements", fixture.First.Queue->Num_Send() == 0);
	}
}


int main(void)
{
	Test_Unsampled_Retry();
	Test_Saturated_Outage_Recovery(2);
	Test_Saturated_Outage_Recovery(32);
	Test_Latency_Increase();

	if (Failures != 0) {
		std::cerr << Failures << " connection timing checks failed\n";
		return(1);
	}
	std::cout << "All connection timing checks passed\n";
	return(0);
}
