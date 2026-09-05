/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "audio/audiomovie.h"

#include "audio/audioengine.h"
#include "audio/audiomovieclock.h"
#include "audio/audiostream.h"
#include "dbgprint.h"
#include "gametime.h"
#include "vqaplayp.h"

#include <cstring>

namespace {

// Blocks pushed but not yet fully heard; the ring holds fewer than this.
unsigned const MAX_PENDING_BLOCKS = AUDIO_MOVIE_RING_BLOCKS + 2;

// How long STOP waits for the mixer to let go of the ring.
int const STOP_WAIT_MS = 250;


// The one movie the player can have open, as a producer the feeder services.
// Fill runs on the feeder thread and does what the DirectSound timer used to:
// tell the player which block has been heard, then ask it for the next one.
class MovieSinkClass : public AudioStreamProducerClass
{
	public:
		MovieSinkClass(void) :
			Used(false),
			Started(false),
			Paused(false),
			Slot(-1),
			Stream(nullptr),
			Rate(0),
			Channels(0),
			Bits(0),
			BlockFrames(0),
			Level(1.0f),
			FillCallback(nullptr),
			DoneCallback(nullptr),
			Owner(nullptr),
			PendingHead(0),
			PendingCount(0)
		{
		}

		bool Fill(AudioStreamClass & stream) override
		{
			(void)stream;
			if (!Started || Paused) {
				return(true);
			}
			uint32_t consumed = Stream->Frames_Consumed();
			while (PendingCount > 0 && (int32_t)(consumed - Pending[PendingHead].End) >= 0) {
				void * buffer = Pending[PendingHead].Buffer;
				PendingHead = (PendingHead + 1) % MAX_PENDING_BLOCKS;
				PendingCount--;
				DoneCallback(Owner, buffer);
			}
			for (unsigned guard = 0; guard < AUDIO_MOVIE_RING_BLOCKS; guard++) {
				if (Stream->Ring.Available_Write() < BlockFrames || PendingCount >= MAX_PENDING_BLOCKS) {
					break;
				}
				unsigned before = PendingCount;
				FillCallback(Owner);
				if (PendingCount == before) {
					// Nothing was loaded: the track is over or the loader is behind.
					break;
				}
			}
			return(true);
		}

		void Close(void) override
		{
		}

		unsigned Min_Ring_Frames(void) const override { return(1); }

		// Called by the player's fill callback, on whichever thread ran it.
		long Load(void * buffer, long nbytes)
		{
			if (!Used || buffer == nullptr || nbytes <= 0 || PendingCount >= MAX_PENDING_BLOCKS) {
				return(VQAERR_AUDIO);
			}
			unsigned frames = Pusher.Push(buffer, (unsigned)nbytes, Bits == 8);
			if (frames == 0) {
				return(VQAERR_AUDIO);
			}
			unsigned tail = (PendingHead + PendingCount) % MAX_PENDING_BLOCKS;
			Pending[tail].Buffer = buffer;
			Pending[tail].End = Stream->Frames_Pushed();
			PendingCount++;
			return(VQAERR_NONE);
		}

		void Clear_Pending(void)
		{
			PendingHead = 0;
			PendingCount = 0;
		}

		bool Used;
		bool Started;
		bool Paused;
		int Slot;
		AudioStreamClass * Stream;
		unsigned Rate;
		unsigned Channels;
		unsigned Bits;
		unsigned BlockFrames;
		float Level;
		AHANDLE_CALLBACK_1 FillCallback;
		AHANDLE_CALLBACK_2 DoneCallback;
		VQAHandle * Owner;
		AudioHandle Handle;
		AudioPushStreamProducerClass Pusher;
		AudioMovieClockClass Clock;

	private:
		struct PendingBlock {
			void * Buffer;
			uint32_t End;
		};

		PendingBlock Pending[MAX_PENDING_BLOCKS];
		unsigned PendingHead;
		unsigned PendingCount;
};

MovieSinkClass _sink;


long Open_Audio_Handler(VQAHandleP * vqap, AhandleInitParams * params, long nbytes)
{
	if (!AudioEngine.Is_Available() || params == nullptr || nbytes != (long)sizeof(AhandleInitParams) || _sink.Used) {
		return(VQAERR_AUDIO);
	}
	VQAConfig * config = &vqap->Config;

	unsigned rate;
	if (config->AudioRate != -1) {
		rate = (unsigned)config->AudioRate;
	} else if (config->FrameRate != vqap->FrameRate && vqap->FrameRate != 0) {
		rate = (unsigned)params->SampleRate * (unsigned)config->FrameRate / vqap->FrameRate;
	} else {
		rate = params->SampleRate;
	}
	unsigned channels = params->Channels;
	unsigned bits = params->BitsPerSample;
	if (rate == 0 || channels == 0 || channels > 2 || (bits != 8 && bits != 16) || config->HMIBufSize <= 0) {
		return(VQAERR_AUDIO);
	}
	unsigned blockframes = (unsigned)config->HMIBufSize / (channels * bits / 8);
	if (blockframes == 0) {
		return(VQAERR_AUDIO);
	}

	AudioStreamClass * stream = nullptr;
	int slot = AudioEngine.Acquire_Stream_Slot(&stream);
	if (slot < 0) {
		DebugString("Ahandle: no free stream for the movie\n");
		return(VQAERR_AUDIO);
	}
	if (!stream->Init(blockframes * AUDIO_MOVIE_RING_BLOCKS, channels, rate)) {
		AudioEngine.Release_Stream_Slot(slot);
		return(VQAERR_AUDIO);
	}

	_sink.Slot = slot;
	_sink.Stream = stream;
	_sink.Rate = rate;
	_sink.Channels = channels;
	_sink.Bits = bits;
	_sink.BlockFrames = blockframes;
	_sink.Level = (float)(config->Volume & 255) / 255.0f;
	_sink.FillCallback = (AHANDLE_CALLBACK_1)params->Callback1;
	_sink.DoneCallback = (AHANDLE_CALLBACK_2)params->Callback2;
	_sink.Owner = (VQAHandle *)vqap;
	_sink.Pusher.Open(*stream);
	_sink.Clear_Pending();
	_sink.Handle.Clear();
	_sink.Started = false;
	_sink.Paused = false;
	_sink.Used = true;

	vqap->AudioHandleIndex = 0;
	config->LatencyAdjustment = 0;
	return(VQAERR_NONE);
}


long Play_Audio_Handler(VQAHandleP * vqap)
{
	(void)vqap;
	if (!_sink.Used || !_sink.Started) {
		return(VQAERR_AUDIO);
	}
	if (_sink.Paused) {
		AudioEngine.Events().Resume(_sink.Handle);
		_sink.Clock.Resume(Get_Game_Time_50());
		_sink.Paused = false;
		DebugString("Ahandle: PauseAdjust %lu\n", _sink.Clock.Pause_Adjust());
	}
	return(VQAERR_NONE);
}


long Start_Audio_Handler(VQAHandleP * vqap)
{
	if (!_sink.Used) {
		return(VQAERR_AUDIO);
	}
	if (_sink.Paused) {
		return(Play_Audio_Handler(vqap));
	}
	if (_sink.Started) {
		return(VQAERR_NONE);
	}

	_sink.Stream->Reset();
	_sink.Clear_Pending();
	unsigned latency = (unsigned)((uint64_t)AudioEngine.Device_Latency_Frames() * _sink.Rate / AUDIO_MIX_RATE);
	_sink.Clock.Reset(_sink.Rate, _sink.BlockFrames, latency, VQA_TIMETICKS);

	// Two blocks before the voice starts, as the DirectSound path primed.
	_sink.FillCallback(_sink.Owner);
	_sink.FillCallback(_sink.Owner);

	if (!AudioEngine.Feeder_Ref().Attach((unsigned)_sink.Slot, _sink.Stream, &_sink)) {
		return(VQAERR_AUDIO);
	}
	_sink.Handle = AudioEngine.Events().Start_Stream(_sink.Stream, AUDIO_GROUP_MOVIE, _sink.Level, 0.0f);
	if (_sink.Handle.Is_Null()) {
		AudioEngine.Feeder_Ref().Detach((unsigned)_sink.Slot);
		return(VQAERR_AUDIO);
	}
	_sink.Started = true;
	return(VQAERR_NONE);
}


long Pause_Audio_Handler(VQAHandleP * vqap)
{
	(void)vqap;
	if (_sink.Used && _sink.Started && !_sink.Paused) {
		AudioEngine.Events().Pause(_sink.Handle);
		_sink.Clock.Pause();
		_sink.Paused = true;
	}
	return(VQAERR_NONE);
}


long Stop_Audio_Handler(VQAHandleP * vqap)
{
	(void)vqap;
	if (_sink.Used && _sink.Started) {
		AudioEngine.Feeder_Ref().Detach((unsigned)_sink.Slot);
		AudioEngine.Events().Stop(_sink.Handle);
		if (!AudioEngine.Wait_Finished(_sink.Handle, STOP_WAIT_MS)) {
			DebugString("Ahandle: movie voice did not stop in time\n");
		}
		_sink.Handle.Clear();
		_sink.Started = false;
		_sink.Paused = false;
		_sink.Stream->Reset();
		_sink.Clear_Pending();
	}
	return(VQAERR_NONE);
}


long Close_Audio_Handler(VQAHandleP * vqap)
{
	if (_sink.Used) {
		Stop_Audio_Handler(vqap);
		_sink.Pusher.Close();
		AudioEngine.Release_Stream_Slot(_sink.Slot);
		_sink.Slot = -1;
		_sink.Stream = nullptr;
		_sink.Owner = nullptr;
		_sink.Used = false;
	}
	return(VQAERR_NONE);
}

} // namespace


unsigned long __cdecl Simple_Timer_Callback_Audio_Handler(VQAHandle *)
{
	return(Get_Game_Time_50());
}


unsigned long __cdecl Timer_Callback_Audio_Handler(VQAHandle * vqa)
{
	VQAHandleP * vqap = (VQAHandleP *)vqa;
	if (!_sink.Used || _sink.Owner != vqa) {
		return(Get_Game_Time_50());
	}
	if (!_sink.Started) {
		return(0);
	}
	return(_sink.Clock.Ticks(_sink.Stream->Frames_Consumed(), (unsigned)(vqap->RepeatedBuffers > 0 ? vqap->RepeatedBuffers : 0), Get_Game_Time_50()));
}


long __cdecl Lock_Audio_Handler(void)
{
	return(1);
}


long __cdecl Unlock_Audio_Handler(void)
{
	return(1);
}


long __cdecl Stream_Audio_Handler(VQAHandle * vqa, long action, void * buffer, long nbytes)
{
	VQAHandleP * vqap = (VQAHandleP *)vqa;

	switch (action) {
		case VQAAUDIO_INIT:
			vqap->Config.TimerCallback = Simple_Timer_Callback_Audio_Handler;
			vqap->Config.RefreshRate = VQA_TIMETICKS;
			return(VQAERR_NONE);

		case VQAAUDIO_OPEN:
			return(Open_Audio_Handler(vqap, (AhandleInitParams *)buffer, nbytes));

		case VQAAUDIO_CLOSE:
			return(Close_Audio_Handler(vqap));

		case VQAAUDIO_START:
			return(Start_Audio_Handler(vqap));

		case VQAAUDIO_LOAD:
			return(_sink.Owner == vqa ? _sink.Load(buffer, nbytes) : VQAERR_AUDIO);

		case VQAAUDIO_PAUSE:
			return(Pause_Audio_Handler(vqap));

		case VQAAUDIO_PLAY:
			return(Play_Audio_Handler(vqap));

		case VQAAUDIO_STOP:
			return(Stop_Audio_Handler(vqap));

		default:
			return(VQAERR_NONE);
	}
}
