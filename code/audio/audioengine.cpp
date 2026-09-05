/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "audio/audioengine.h"

#include "audio/audiodecode.h"
#include "audio/audiodevice.h"
#include "_rand.h"
#include "ccfile.h"
#include "dbgprint.h"
#include "random.h"

#include <cstring>
#include <thread>

namespace {

// How long Stop_All_Streams waits for the voices to let go of their rings.
int const STREAM_STOP_WAIT_MS = 250;

// Dropped commands are reported at most this often.
unsigned const DROP_REPORT_MS = 1000;


// A byte source over the game's file layer, so a stream reads loose files and
// archive members alike, a block at a time.
class CcFileByteSourceClass : public AudioByteSourceClass
{
	public:
		explicit CcFileByteSourceClass(char const * filename) :
			File(filename),
			Cursor(0),
			Length(0),
			Opened(false)
		{
			if (File.Is_Available() && File.Open(FileClass::READ)) {
				Opened = true;
				int size = File.Size();
				Length = size > 0 ? (size_t)size : 0;
			}
		}

		~CcFileByteSourceClass(void)
		{
			if (Opened) {
				File.Close();
			}
		}

		bool Is_Open(void) const { return(Opened); }

		size_t Read(void * buffer, size_t bytes) override
		{
			if (!Opened || bytes == 0) {
				return(0);
			}
			int got = File.Read(buffer, (int)bytes);
			if (got < 0) {
				got = 0;
			}
			Cursor += (size_t)got;
			return((size_t)got);
		}

		bool Seek(size_t position) override
		{
			if (!Opened || position > Length) {
				return(false);
			}
			if (File.Seek((int)position, SEEK_SET) != (int)position) {
				return(false);
			}
			Cursor = position;
			return(true);
		}

		size_t Position(void) const override { return(Cursor); }
		size_t Size(void) const override { return(Length); }

	private:
		CCFileClass File;
		size_t Cursor;
		size_t Length;
		bool Opened;
};


class CcFileReaderClass : public AudioAssetReaderClass
{
	public:
		bool Read(char const * filename, std::vector<uint8_t> & bytes) override
		{
			CCFileClass file(filename);
			if (!file.Is_Available() || !file.Open(FileClass::READ)) {
				return(false);
			}
			int size = file.Size();
			bool ok = false;
			if (size > 0) {
				bytes.resize((size_t)size);
				ok = file.Read(bytes.data(), size) == size;
			}
			file.Close();
			return(ok);
		}
};


class CacheProviderClass : public AudioClipProviderClass
{
	public:
		CacheProviderClass(AudioSampleCacheClass & cache, AudioAssetReaderClass & reader) : Cache(cache), Reader(reader) {}

		AudioSampleClass * Acquire(AudioEventTypeClass const & type, unsigned sound) override
		{
			if (sound >= type.SoundCount || type.Sounds[sound][0] == '\0') {
				return(nullptr);
			}
			return(Cache.Acquire_Named(type.Sounds[sound], Reader));
		}

		void Release(AudioSampleClass * clip) override
		{
			Cache.Release(clip);
		}

	private:
		AudioSampleCacheClass & Cache;
		AudioAssetReaderClass & Reader;
};

} // namespace


AudioEngineClass::AudioEngineClass(void) :
	MasterGain(1.0f),
	LastDropped(0),
	LastDropReport(0),
	Available(false)
{
	for (int i = 0; i < AUDIO_GROUP_COUNT; i++) {
		GroupGains[i] = 1.0f;
	}
}


AudioEngineClass::~AudioEngineClass(void)
{
	End();
}


int AudioEngineClass::Random_Proc(int low, int high, void *)
{
	return(NonCriticalRandomNumber(low, high));
}


unsigned AudioEngineClass::Now_Ms(void) const
{
	auto elapsed = std::chrono::steady_clock::now() - Epoch;
	return((unsigned)std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
}


bool AudioEngineClass::Init(void)
{
	if (Available) {
		return(true);
	}
	Epoch = std::chrono::steady_clock::now();

	if (!Mixer.Init(AUDIO_MIX_RATE, AUDIO_MIX_CHANNELS)) {
		DebugString("Audio: mixer init failed\n");
		return(false);
	}

	Device = Audio_Create_Miniaudio_Device();
	if (Device == nullptr || !Device->Open(AUDIO_MIX_RATE, AUDIO_MIX_CHANNELS, AudioMixerClass::Render_Callback, &Mixer) || !Device->Start()) {
		DebugString("Audio: no output device, sound disabled\n");
		if (Device != nullptr) {
			Device->Close();
			Device.reset();
		}
		Mixer.Shutdown();
		return(false);
	}

	Reader.reset(new CcFileReaderClass());
	Provider.reset(new CacheProviderClass(Cache, *Reader));
	if (!Pool.Init(&Mixer, Provider.get())) {
		DebugString("Audio: event pool init failed\n");
		Device->Stop();
		Device->Close();
		Device.reset();
		Mixer.Shutdown();
		Provider.reset();
		Reader.reset();
		return(false);
	}
	Pool.Set_Random(Random_Proc, nullptr);

	Feeder.Setup(&Mixer, Device.get());
	Feeder.Start();

	LastDropped = 0;
	LastDropReport = 0;
	Available = true;

	// The option sliders may have been applied before the device existed.
	for (int i = 0; i < AUDIO_GROUP_COUNT; i++) {
		Push_Level(AudioCommandType::GROUP_SET_GAIN, (AudioGroupType)i, GroupGains[i], 0.0f);
	}
	Push_Level(AudioCommandType::MASTER_SET_GAIN, AUDIO_GROUP_SFX, MasterGain, 0.0f);

	DebugString("Audio: %s, %u Hz, %u x %u frames\n", Device->Name(), Device->Rate(), Device->Periods(), Device->Period_Frames());
	return(true);
}


void AudioEngineClass::End(void)
{
	if (!Available && Device == nullptr) {
		return;
	}
	Available = false;
	Pool.Shutdown();
	Feeder.Stop();
	for (int i = 0; i < AUDIO_MAX_STREAMS; i++) {
		Streams[i].Producer.reset();
		Streams[i].Handle.Clear();
		Streams[i].InUse = false;
		Streams[i].External = false;
	}
	if (Device != nullptr) {
		Device->Stop();
		Device->Close();
		Device.reset();
	}
	Mixer.Shutdown();
	Cache.Flush();
	Provider.reset();
	Reader.reset();
}


AudioHandle AudioEngineClass::Play_Event(AudioEventTypeClass const & type, AudioGroupType group, float volume, float pan, bool noattack)
{
	if (!Available) {
		return(AudioHandle());
	}
	return(Pool.Start(type, group, volume, pan, noattack));
}


AudioHandle AudioEngineClass::Play_Sample(void const * aud, AudioGroupType group, float volume, int priority)
{
	if (!Available || aud == nullptr) {
		return(AudioHandle());
	}
	AudioSampleClass * clip = Cache.Acquire_Blob(aud, 0);
	if (clip == nullptr) {
		return(AudioHandle());
	}
	return(Pool.Start_Sample(clip, group, volume, priority, aud));
}


int AudioEngineClass::Free_Stream_Slot(void) const
{
	for (int i = 0; i < AUDIO_MAX_STREAMS; i++) {
		if (!Streams[i].InUse) {
			return(i);
		}
	}
	return(-1);
}


AudioHandle AudioEngineClass::Open_Stream(char const * filename, AudioGroupType group, float volume, bool loop)
{
	if (!Available || filename == nullptr) {
		return(AudioHandle());
	}
	int slot = Free_Stream_Slot();
	if (slot < 0) {
		DebugString("Audio: no free stream for %s\n", filename);
		return(AudioHandle());
	}

	std::unique_ptr<CcFileByteSourceClass> source(new CcFileByteSourceClass(filename));
	if (!source->Is_Open()) {
		return(AudioHandle());
	}
	std::unique_ptr<AudioFileStreamProducerClass> producer(new AudioFileStreamProducerClass());
	if (!producer->Open(std::move(source), loop)) {
		DebugString("Audio: cannot decode %s\n", filename);
		return(AudioHandle());
	}

	StreamSlotClass & s = Streams[slot];
	unsigned frames = (unsigned)((float)producer->Rate() * AUDIO_FILE_STREAM_SECONDS);
	if (frames < producer->Min_Ring_Frames()) {
		frames = producer->Min_Ring_Frames();
	}
	if (!s.Stream.Init(frames, producer->Channels(), producer->Rate())) {
		return(AudioHandle());
	}
	s.Stream.Reset();

	// The producer is still ours here, so the first block is decoded now and
	// playback starts without waiting for a feeder pass.
	producer->Fill(s.Stream);

	if (!Feeder.Attach(slot, &s.Stream, producer.get())) {
		producer->Close();
		return(AudioHandle());
	}
	AudioHandle handle = Pool.Start_Stream(&s.Stream, group, volume, 0.0f);
	if (handle.Is_Null()) {
		Feeder.Detach(slot);
		return(AudioHandle());
	}
	s.Producer = std::move(producer);
	s.Handle = handle;
	s.InUse = true;
	return(handle);
}


void AudioEngineClass::Stop_Stream(AudioHandle handle)
{
	if (!Available || handle.Is_Null()) {
		return;
	}
	Pool.Stop(handle);
	if (!Wait_Finished(handle, STREAM_STOP_WAIT_MS)) {
		DebugString("Audio: stream did not stop in time\n");
	}
	for (int i = 0; i < AUDIO_MAX_STREAMS; i++) {
		if (Streams[i].InUse && !Streams[i].External && Streams[i].Handle == handle) {
			Close_Stream(i);
		}
	}
}


void AudioEngineClass::Close_Stream(int slot)
{
	StreamSlotClass & s = Streams[slot];
	Feeder.Detach((unsigned)slot);
	s.Producer.reset();
	s.Handle.Clear();
	s.InUse = false;
}


void AudioEngineClass::Reap_Streams(void)
{
	for (int i = 0; i < AUDIO_MAX_STREAMS; i++) {
		if (Streams[i].InUse && !Streams[i].External && Pool.Is_Finished(Streams[i].Handle)) {
			Close_Stream(i);
		}
	}
}


int AudioEngineClass::Acquire_Stream_Slot(AudioStreamClass ** stream)
{
	if (!Available || stream == nullptr) {
		return(-1);
	}
	int slot = Free_Stream_Slot();
	if (slot < 0) {
		return(-1);
	}
	Streams[slot].InUse = true;
	Streams[slot].External = true;
	*stream = &Streams[slot].Stream;
	return(slot);
}


void AudioEngineClass::Release_Stream_Slot(int slot)
{
	if (slot < 0 || slot >= AUDIO_MAX_STREAMS || !Streams[slot].External) {
		return;
	}
	Streams[slot].External = false;
	Streams[slot].InUse = false;
	Streams[slot].Handle.Clear();
}


unsigned AudioEngineClass::Device_Latency_Frames(void) const
{
	if (Device == nullptr || Device->Periods() == 0) {
		return(0);
	}
	return(Device->Period_Frames() * (Device->Periods() - 1));
}


bool AudioEngineClass::Wait_Finished(AudioHandle handle, int ms)
{
	if (!Available) {
		return(true);
	}
	unsigned start = Now_Ms();
	for (;;) {
		Pool.Service(Now_Ms());
		if (Pool.Is_Finished(handle)) {
			return(true);
		}
		if (Now_Ms() - start > (unsigned)ms) {
			return(false);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}


bool AudioEngineClass::Is_Sample_Playing(void const * aud) const
{
	return(Available && Pool.Is_Tag_Playing(aud));
}


void AudioEngineClass::Stop_Sample_Playing(void const * aud)
{
	if (Available) {
		Pool.Stop_Tag(aud);
	}
}


void AudioEngineClass::Set_Sample_Volume(void const * aud, float volume)
{
	if (Available) {
		Pool.Set_Tag_Volume(aud, volume);
	}
}


void AudioEngineClass::Release_Sample(void const * aud)
{
	if (Available) {
		Cache.Invalidate_Blob(aud);
	}
}


void AudioEngineClass::Push_Level(AudioCommandType type, AudioGroupType group, float level, float ramp)
{
	AudioCommand command;
	std::memset(&command, 0, sizeof(command));
	command.Type = type;
	command.Group = (uint8_t)group;
	command.A = level;
	command.B = ramp;
	Mixer.Push(command);
}


void AudioEngineClass::Set_Group_Gain(AudioGroupType group, float gain)
{
	if (group >= AUDIO_GROUP_COUNT) {
		return;
	}
	if (gain < 0.0f) gain = 0.0f;
	if (gain > 1.0f) gain = 1.0f;
	GroupGains[group] = gain;
	if (Available) {
		Push_Level(AudioCommandType::GROUP_SET_GAIN, group, gain, AUDIO_RETARGET_RAMP_SECONDS);
	}
}


float AudioEngineClass::Group_Gain(AudioGroupType group) const
{
	return(group < AUDIO_GROUP_COUNT ? GroupGains[group] : 0.0f);
}


void AudioEngineClass::Set_Duck(AudioGroupType group, float level, int ms)
{
	if (!Available || group >= AUDIO_GROUP_COUNT) {
		return;
	}
	if (level < 0.0f) level = 0.0f;
	if (level > 1.0f) level = 1.0f;
	Push_Level(AudioCommandType::GROUP_SET_DUCK, group, level, ms > 0 ? (float)ms / 1000.0f : AUDIO_DUCK_RAMP_SECONDS);
}


void AudioEngineClass::Set_Master_Gain(float gain, int ms)
{
	if (gain < 0.0f) gain = 0.0f;
	if (gain > 1.0f) gain = 1.0f;
	MasterGain = gain;
	if (Available) {
		Push_Level(AudioCommandType::MASTER_SET_GAIN, AUDIO_GROUP_SFX, gain, ms > 0 ? (float)ms / 1000.0f : 0.0f);
	}
}


void AudioEngineClass::Set_Channels(int channels)
{
	Pool.Set_Budget(channels);
}


void AudioEngineClass::Focus_Loss(void)
{
	Mixer.Set_Pause_All(true);
}


void AudioEngineClass::Focus_Restore(void)
{
	Mixer.Set_Pause_All(false);
}


void AudioEngineClass::Stop_All(void)
{
	if (Available) {
		Pool.Stop_All();
	}
}


void AudioEngineClass::Stop_All_Streams(void)
{
	if (!Available) {
		return;
	}
	unsigned start = Now_Ms();
	for (int i = 0; i < AUDIO_MAX_STREAMS; i++) {
		if (Streams[i].InUse && !Streams[i].External) {
			Pool.Stop(Streams[i].Handle);
		}
	}
	for (int i = 0; i < AUDIO_MAX_STREAMS; i++) {
		if (!Streams[i].InUse || Streams[i].External) {
			continue;
		}
		int left = STREAM_STOP_WAIT_MS - (int)(Now_Ms() - start);
		if (!Wait_Finished(Streams[i].Handle, left > 0 ? left : 0)) {
			// A voice that has not let go by now is on a dead device; the ring stays
			// allocated, so closing the file under it is safe.
			DebugString("Audio: stream %d did not stop in time\n", i);
		}
		Close_Stream(i);
	}
}


void AudioEngineClass::Sound_Callback(void)
{
	if (!Available) {
		return;
	}
	unsigned now = Now_Ms();
	Pool.Service(now);
	Reap_Streams();

	unsigned dropped = Mixer.Dropped_Commands();
	if (dropped != LastDropped && now - LastDropReport >= DROP_REPORT_MS) {
		DebugString("Audio: %u commands dropped\n", dropped - LastDropped);
		LastDropped = dropped;
		LastDropReport = now;
	}
}


bool AudioHandle::Is_Valid(void) const
{
	return(AudioEngine.Events().Is_Valid(*this));
}


bool AudioHandle::Is_Playing(void) const
{
	return(AudioEngine.Events().Is_Playing(*this));
}


AudioEventTypeClass const * AudioHandle::Type(void) const
{
	return(AudioEngine.Events().Type(*this));
}


void AudioHandle::Retarget(float volume, float pan)
{
	AudioEngine.Events().Retarget(*this, volume, pan);
}


void AudioHandle::Set_Volume(float volume)
{
	AudioEngine.Events().Set_Volume(*this, volume);
}


void AudioHandle::Set_Pan(float pan)
{
	AudioEngine.Events().Set_Pan(*this, pan);
}


void AudioHandle::Stop(void)
{
	AudioEngine.Events().Stop(*this);
}


void AudioHandle::End(void)
{
	AudioEngine.Events().End(*this);
}


void AudioHandle::End_Looping(void)
{
	AudioEngine.Events().End_Looping(*this);
}


void AudioHandle::Fade(int ms)
{
	AudioEngine.Events().Fade(*this, ms);
}
