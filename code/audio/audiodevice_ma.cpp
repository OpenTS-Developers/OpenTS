/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The miniaudio device. This is the only engine file that includes the
// library's device API; the mixer and everything above it see only
// AudioDeviceClass.

#include "audio/audiodevice.h"
#include "audio/audiodefs.hh"

#include "miniaudio.h"

#include <atomic>
#include <cstring>

namespace {

class MiniaudioDeviceClass : public AudioDeviceClass
{
	public:
		MiniaudioDeviceClass(void);
		~MiniaudioDeviceClass(void);

		bool Open(unsigned rate, unsigned channels, RenderCallback callback, void * context) override;
		void Close(void) override;
		bool Start(void) override;
		void Stop(void) override;

		bool Is_Open(void) const override { return(Opened); }
		bool Is_Running(void) const override;
		bool Is_Lost(void) const override { return(Lost.load(std::memory_order_acquire)); }
		unsigned Rate(void) const override { return(Opened ? Device.sampleRate : 0); }
		unsigned Channels(void) const override { return(Opened ? Device.playback.channels : 0); }
		unsigned Period_Frames(void) const override { return(Opened ? Device.playback.internalPeriodSizeInFrames : 0); }
		unsigned Periods(void) const override { return(Opened ? Device.playback.internalPeriods : 0); }
		char const * Name(void) const override { return(Opened ? Device.playback.name : ""); }

	private:
		static void Data_Callback(ma_device * device, void * output, void const * input, ma_uint32 frames);
		static void Notification_Callback(ma_device_notification const * notification);

		ma_context Context;
		ma_device Device;
		bool ContextReady;
		bool Opened;
		RenderCallback Callback;
		void * CallbackContext;

		// Set before Start and cleared before Stop, so a stop the device reports on
		// its own is the only one that marks it lost.
		std::atomic<bool> ExpectedRunning;
		std::atomic<bool> Lost;
};


MiniaudioDeviceClass::MiniaudioDeviceClass(void) :
	ContextReady(false),
	Opened(false),
	Callback(nullptr),
	CallbackContext(nullptr),
	ExpectedRunning(false),
	Lost(false)
{
	std::memset(&Context, 0, sizeof(Context));
	std::memset(&Device, 0, sizeof(Device));
}


MiniaudioDeviceClass::~MiniaudioDeviceClass(void)
{
	Close();
}


bool MiniaudioDeviceClass::Open(unsigned rate, unsigned channels, RenderCallback callback, void * context)
{
	if (Opened || rate == 0 || channels == 0 || callback == nullptr) {
		return(false);
	}

	// Every enabled real backend in the library's own priority order. The null
	// backend is left out so a machine without sound hardware reports no device
	// instead of playing into nothing.
	ma_backend backends[ma_backend_null];
	ma_uint32 count = 0;
	for (int backend = 0; backend < (int)ma_backend_null; backend++) {
		if (ma_is_backend_enabled((ma_backend)backend)) {
			backends[count++] = (ma_backend)backend;
		}
	}
	if (count == 0) {
		return(false);
	}

	ma_context_config contextconfig = ma_context_config_init();
	if (ma_context_init(backends, count, &contextconfig, &Context) != MA_SUCCESS) {
		return(false);
	}
	ContextReady = true;

	ma_device_config config = ma_device_config_init(ma_device_type_playback);
	config.playback.format = ma_format_f32;
	config.playback.channels = channels;
	config.sampleRate = rate;
	config.periodSizeInMilliseconds = AUDIO_PERIOD_MS;
	config.periods = AUDIO_PERIODS;
	config.performanceProfile = ma_performance_profile_low_latency;
	config.noPreSilencedOutputBuffer = MA_TRUE;
	config.dataCallback = Data_Callback;
	config.notificationCallback = Notification_Callback;
	config.pUserData = this;

	Callback = callback;
	CallbackContext = context;

	if (ma_device_init(&Context, &config, &Device) != MA_SUCCESS) {
		ma_context_uninit(&Context);
		ContextReady = false;
		Callback = nullptr;
		CallbackContext = nullptr;
		return(false);
	}

	Opened = true;
	Lost.store(false, std::memory_order_release);
	return(true);
}


void MiniaudioDeviceClass::Close(void)
{
	if (Opened) {
		Stop();
		ma_device_uninit(&Device);
		Opened = false;
	}
	if (ContextReady) {
		ma_context_uninit(&Context);
		ContextReady = false;
	}
	Callback = nullptr;
	CallbackContext = nullptr;
	Lost.store(false, std::memory_order_release);
}


bool MiniaudioDeviceClass::Start(void)
{
	if (!Opened) {
		return(false);
	}
	ExpectedRunning.store(true, std::memory_order_release);
	ma_result result = ma_device_start(&Device);
	if (result == MA_SUCCESS || ma_device_get_state(&Device) == ma_device_state_started) {
		Lost.store(false, std::memory_order_release);
		return(true);
	}
	return(false);
}


void MiniaudioDeviceClass::Stop(void)
{
	if (!Opened) {
		return;
	}
	ExpectedRunning.store(false, std::memory_order_release);
	ma_device_stop(&Device);
}


bool MiniaudioDeviceClass::Is_Running(void) const
{
	return(Opened && ma_device_get_state(&Device) == ma_device_state_started);
}


void MiniaudioDeviceClass::Data_Callback(ma_device * device, void * output, void const * input, ma_uint32 frames)
{
	MiniaudioDeviceClass * self = (MiniaudioDeviceClass *)device->pUserData;
	if (self != nullptr && self->Callback != nullptr) {
		self->Callback(self->CallbackContext, (float *)output, frames);
	} else {
		std::memset(output, 0, (size_t)frames * device->playback.channels * sizeof(float));
	}
	(void)input;
}


void MiniaudioDeviceClass::Notification_Callback(ma_device_notification const * notification)
{
	MiniaudioDeviceClass * self = (MiniaudioDeviceClass *)notification->pDevice->pUserData;
	if (self == nullptr) {
		return;
	}
	switch (notification->type) {
		case ma_device_notification_type_started:
			self->Lost.store(false, std::memory_order_release);
			break;

		case ma_device_notification_type_stopped:
			if (self->ExpectedRunning.load(std::memory_order_acquire)) {
				self->Lost.store(true, std::memory_order_release);
			}
			break;

		default:
			break;
	}
}

} // namespace


std::unique_ptr<AudioDeviceClass> Audio_Create_Miniaudio_Device(void)
{
	return(std::unique_ptr<AudioDeviceClass>(new (std::nothrow) MiniaudioDeviceClass()));
}
