/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Single-producer, single-consumer rings. One carries commands from the game
// thread to the mixer; the other carries PCM frames from a stream producer to
// the mixer. Neither locks, and neither allocates after it is set up.

#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <memory>
#include <type_traits>


// Fixed-capacity ring of trivially copyable items. Push is for the producer thread
// only and Pop for the consumer thread only.
template<typename T, unsigned N>
class SpscRingClass
{
	static_assert((N & (N - 1)) == 0, "ring capacity must be a power of two");
	static_assert(std::is_trivially_copyable<T>::value, "ring items must be trivially copyable");

	public:
		bool Push(T const & item)
		{
			uint32_t tail = Tail.load(std::memory_order_relaxed);
			uint32_t head = Head.load(std::memory_order_acquire);
			if (tail - head >= N) {
				return(false);
			}
			Items[tail & (N - 1)] = item;
			Tail.store(tail + 1, std::memory_order_release);
			return(true);
		}

		bool Pop(T & item)
		{
			uint32_t head = Head.load(std::memory_order_relaxed);
			uint32_t tail = Tail.load(std::memory_order_acquire);
			if (tail == head) {
				return(false);
			}
			item = Items[head & (N - 1)];
			Head.store(head + 1, std::memory_order_release);
			return(true);
		}

		unsigned Count(void) const
		{
			return(Tail.load(std::memory_order_acquire) - Head.load(std::memory_order_acquire));
		}

		bool Is_Empty(void) const { return(Count() == 0); }
		unsigned Capacity(void) const { return(N); }

	private:
		T Items[N];
		std::atomic<uint32_t> Head{0};
		std::atomic<uint32_t> Tail{0};
};


// Ring of interleaved 16-bit PCM frames. Write is for the producer thread only and
// Read and Discard for the consumer thread only. The frame counters keep counting
// past the capacity, so the consumer's count doubles as a playback position.
class PcmRingClass
{
	public:
		PcmRingClass(void) = default;
		PcmRingClass(PcmRingClass const &) = delete;
		PcmRingClass & operator=(PcmRingClass const &) = delete;

		// Rounds the capacity up to a power of two. Allocates; call it before either
		// thread touches the ring.
		bool Init(unsigned frames, unsigned channels)
		{
			if (frames == 0 || channels == 0) {
				return(false);
			}
			unsigned capacity = 1;
			while (capacity < frames) {
				capacity <<= 1;
			}
			Buffer.reset(new (std::nothrow) int16_t[(size_t)capacity * channels]);
			if (Buffer == nullptr) {
				CapacityFrames = 0;
				return(false);
			}
			CapacityFrames = capacity;
			Mask = capacity - 1;
			ChannelCount = channels;
			Reset();
			return(true);
		}

		// Only while neither thread is using the ring.
		void Reset(void)
		{
			ReadPos.store(0, std::memory_order_relaxed);
			WritePos.store(0, std::memory_order_relaxed);
		}

		unsigned Capacity(void) const { return(CapacityFrames); }
		unsigned Channels(void) const { return(ChannelCount); }

		unsigned Available_Read(void) const
		{
			return(WritePos.load(std::memory_order_acquire) - ReadPos.load(std::memory_order_acquire));
		}

		unsigned Available_Write(void) const
		{
			return(CapacityFrames - Available_Read());
		}

		// Returns the number of frames actually written.
		unsigned Write(int16_t const * frames, unsigned count)
		{
			uint32_t write = WritePos.load(std::memory_order_relaxed);
			uint32_t read = ReadPos.load(std::memory_order_acquire);
			unsigned room = CapacityFrames - (write - read);
			if (count > room) {
				count = room;
			}
			Copy_In(write, frames, count);
			WritePos.store(write + count, std::memory_order_release);
			return(count);
		}

		// Returns the number of frames actually read.
		unsigned Read(int16_t * frames, unsigned count)
		{
			uint32_t read = ReadPos.load(std::memory_order_relaxed);
			uint32_t write = WritePos.load(std::memory_order_acquire);
			unsigned available = write - read;
			if (count > available) {
				count = available;
			}
			Copy_Out(read, frames, count);
			ReadPos.store(read + count, std::memory_order_release);
			return(count);
		}

		// Drops frames without copying them; returns how many were dropped.
		unsigned Discard(unsigned count)
		{
			uint32_t read = ReadPos.load(std::memory_order_relaxed);
			uint32_t write = WritePos.load(std::memory_order_acquire);
			unsigned available = write - read;
			if (count > available) {
				count = available;
			}
			ReadPos.store(read + count, std::memory_order_release);
			return(count);
		}

		uint32_t Frames_Pushed(void) const { return(WritePos.load(std::memory_order_acquire)); }
		uint32_t Frames_Consumed(void) const { return(ReadPos.load(std::memory_order_acquire)); }

	private:
		void Copy_In(uint32_t position, int16_t const * frames, unsigned count)
		{
			unsigned start = position & Mask;
			unsigned first = CapacityFrames - start;
			if (first > count) {
				first = count;
			}
			std::memcpy(&Buffer[(size_t)start * ChannelCount], frames, (size_t)first * ChannelCount * sizeof(int16_t));
			if (count > first) {
				std::memcpy(&Buffer[0], frames + (size_t)first * ChannelCount, (size_t)(count - first) * ChannelCount * sizeof(int16_t));
			}
		}

		void Copy_Out(uint32_t position, int16_t * frames, unsigned count)
		{
			unsigned start = position & Mask;
			unsigned first = CapacityFrames - start;
			if (first > count) {
				first = count;
			}
			std::memcpy(frames, &Buffer[(size_t)start * ChannelCount], (size_t)first * ChannelCount * sizeof(int16_t));
			if (count > first) {
				std::memcpy(frames + (size_t)first * ChannelCount, &Buffer[0], (size_t)(count - first) * ChannelCount * sizeof(int16_t));
			}
		}

		std::unique_ptr<int16_t[]> Buffer;
		unsigned CapacityFrames = 0;
		unsigned Mask = 0;
		unsigned ChannelCount = 0;
		std::atomic<uint32_t> ReadPos{0};
		std::atomic<uint32_t> WritePos{0};
};
