/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "audio/audiodevice.h"

#include <cstring>


NullAudioDeviceClass::NullAudioDeviceClass(unsigned periodframes, unsigned periods) :
	Callback(nullptr),
	Context(nullptr),
	RateValue(0),
	ChannelCount(0),
	PeriodFrames(periodframes),
	PeriodCount(periods),
	Opened(false),
	Running(false),
	Lost(false)
{
}


bool NullAudioDeviceClass::Open(unsigned rate, unsigned channels, RenderCallback callback, void * context)
{
	if (Opened || rate == 0 || channels == 0 || callback == nullptr) {
		return(false);
	}
	RateValue = rate;
	ChannelCount = channels;
	Callback = callback;
	Context = context;
	Opened = true;
	Lost.store(false, std::memory_order_release);
	return(true);
}


void NullAudioDeviceClass::Close(void)
{
	Stop();
	Opened = false;
	Callback = nullptr;
	Context = nullptr;
}


bool NullAudioDeviceClass::Start(void)
{
	if (!Opened) {
		return(false);
	}
	Lost.store(false, std::memory_order_release);
	Running.store(true, std::memory_order_release);
	return(true);
}


void NullAudioDeviceClass::Stop(void)
{
	Running.store(false, std::memory_order_release);
}


unsigned NullAudioDeviceClass::Pump(float * output, unsigned frames)
{
	if (!Is_Running() || Callback == nullptr) {
		return(0);
	}
	unsigned done = 0;
	while (done < frames) {
		unsigned count = frames - done;
		if (count > PeriodFrames) {
			count = PeriodFrames;
		}
		Callback(Context, output + (size_t)done * ChannelCount, count);
		done += count;
	}
	return(done);
}


void NullAudioDeviceClass::Set_Lost(bool lost)
{
	Lost.store(lost, std::memory_order_release);
	if (lost) {
		Running.store(false, std::memory_order_release);
	}
}
