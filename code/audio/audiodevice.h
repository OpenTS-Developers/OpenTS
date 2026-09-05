/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The output device the mixer renders into, behind an interface small enough
// that another audio library could stand in for miniaudio without touching
// the mixer. Open, Close and Stop run on the game thread; Start may also run
// on the feeder thread during recovery; the render callback runs on the
// device's own thread.

#pragma once

#include <atomic>
#include <memory>


class AudioDeviceClass
{
	public:
		typedef void (*RenderCallback)(void * context, float * output, unsigned frames);

		virtual ~AudioDeviceClass(void) = default;

		virtual bool Open(unsigned rate, unsigned channels, RenderCallback callback, void * context) = 0;
		virtual void Close(void) = 0;
		virtual bool Start(void) = 0;
		virtual void Stop(void) = 0;

		virtual bool Is_Open(void) const = 0;
		virtual bool Is_Running(void) const = 0;

		// True once the device stopped on its own after Start, until it runs again.
		virtual bool Is_Lost(void) const = 0;

		virtual unsigned Rate(void) const = 0;
		virtual unsigned Channels(void) const = 0;
		virtual unsigned Period_Frames(void) const = 0;
		virtual unsigned Periods(void) const = 0;
		virtual char const * Name(void) const = 0;
};


// A device with no output. Tests and the engine's recovery path drive the
// render callback through Pump instead of a device thread.
class NullAudioDeviceClass : public AudioDeviceClass
{
	public:
		NullAudioDeviceClass(unsigned periodframes = 480, unsigned periods = 3);

		bool Open(unsigned rate, unsigned channels, RenderCallback callback, void * context) override;
		void Close(void) override;
		bool Start(void) override;
		void Stop(void) override;

		bool Is_Open(void) const override { return(Opened); }
		bool Is_Running(void) const override { return(Running.load(std::memory_order_acquire)); }
		bool Is_Lost(void) const override { return(Lost.load(std::memory_order_acquire)); }
		unsigned Rate(void) const override { return(RateValue); }
		unsigned Channels(void) const override { return(ChannelCount); }
		unsigned Period_Frames(void) const override { return(PeriodFrames); }
		unsigned Periods(void) const override { return(PeriodCount); }
		char const * Name(void) const override { return("Null"); }

		// Renders into the caller's buffer, one period at a time, and returns the
		// frames rendered. Renders nothing unless the device is running.
		unsigned Pump(float * output, unsigned frames);

		// Lets a test stand in for the hardware going away and coming back.
		void Set_Lost(bool lost);

	private:
		RenderCallback Callback;
		void * Context;
		unsigned RateValue;
		unsigned ChannelCount;
		unsigned PeriodFrames;
		unsigned PeriodCount;
		bool Opened;
		std::atomic<bool> Running;
		std::atomic<bool> Lost;
};


// Creates the miniaudio-backed device. Returns null if the library cannot be set up.
std::unique_ptr<AudioDeviceClass> Audio_Create_Miniaudio_Device(void);
