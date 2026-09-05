/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "audio/audiostream.h"

#include "audio/audiodevice.h"
#include "audio/audiomixer.h"

#include <cstring>

namespace {

// How much a producer may write in one pass, so one stream cannot hold the
// feeder for long.
unsigned const MAX_FRAMES_PER_PASS = 48000;

// Frames of one read from a miniaudio decoder.
unsigned const OTHER_READ_FRAMES = 4096;

// The most catch-up rendered in one recovery pass.
unsigned const MAX_PUMP_FRAMES = 4800;

} // namespace


AudioStreamClass::AudioStreamClass(void) :
	EndOfInput(false),
	Underruns(0),
	RateValue(0)
{
}


bool AudioStreamClass::Init(unsigned frames, unsigned channels, unsigned rate)
{
	if (rate == 0 || !Ring.Init(frames, channels)) {
		return(false);
	}
	RateValue = rate;
	Reset();
	return(true);
}


void AudioStreamClass::Reset(void)
{
	Ring.Reset();
	EndOfInput.store(false, std::memory_order_release);
	Underruns.store(0, std::memory_order_release);
}


AudioFileStreamProducerClass::AudioFileStreamProducerClass(void) :
	Loop(false),
	IsAud(false),
	Ended(false),
	DataStart(0),
	DataRemaining(0),
	RateValue(0),
	ChannelCount(0)
{
	std::memset(&Header, 0, sizeof(Header));
}


AudioFileStreamProducerClass::~AudioFileStreamProducerClass(void)
{
	Close();
}


bool AudioFileStreamProducerClass::Open(std::unique_ptr<AudioByteSourceClass> source, bool loop)
{
	Close();
	if (source == nullptr) {
		return(false);
	}
	Loop = loop;
	Ended = false;

	unsigned char head[sizeof(AUDHeaderType)];
	source->Seek(0);
	size_t got = source->Read(head, sizeof(head));
	if (got == sizeof(head) && Aud_Read_Header(head, source->Size(), Header)) {
		IsAud = true;
		Aud.Init(Header);
		DataStart = sizeof(AUDHeaderType);
		DataRemaining = (size_t)Header.Size;
		RateValue = Aud.Rate();
		ChannelCount = Aud.Channels();
		Compressed.resize(AUD_MAX_CHUNK_COMP_BYTES);
		Pcm.resize((size_t)Aud.Max_Chunk_Frames() * ChannelCount);
		Source = std::move(source);
		return(true);
	}

	source->Seek(0);
	if (Other.Open(*source)) {
		IsAud = false;
		RateValue = Other.Rate();
		ChannelCount = Other.Channels();
		Pcm.resize((size_t)OTHER_READ_FRAMES * ChannelCount);
		Source = std::move(source);
		return(true);
	}
	return(false);
}


void AudioFileStreamProducerClass::Close(void)
{
	Other.Close();
	Source.reset();
	Ended = true;
	RateValue = 0;
	ChannelCount = 0;
}


bool AudioFileStreamProducerClass::Rewind(void)
{
	if (IsAud) {
		if (!Source->Seek(DataStart)) {
			return(false);
		}
		Aud.Rewind();
		DataRemaining = (size_t)Header.Size;
		return(true);
	}
	return(Other.Rewind());
}


unsigned AudioFileStreamProducerClass::Min_Ring_Frames(void) const
{
	if (Source == nullptr) {
		return(0);
	}
	return(IsAud ? Aud.Max_Chunk_Frames() : OTHER_READ_FRAMES);
}


bool AudioFileStreamProducerClass::Fill(AudioStreamClass & stream)
{
	if (Source == nullptr || Ended) {
		return(false);
	}
	if (stream.Ring.Capacity() < Min_Ring_Frames()) {
		Ended = true;
		return(false);
	}
	return(IsAud ? Fill_Aud(stream) : Fill_Other(stream));
}


bool AudioFileStreamProducerClass::Fill_Aud(AudioStreamClass & stream)
{
	unsigned framebytes = ChannelCount * (Aud_Bits(Header) / 8);
	unsigned chunkframes = Aud.Max_Chunk_Frames();
	unsigned written = 0;

	while (written < MAX_FRAMES_PER_PASS && stream.Ring.Available_Write() >= chunkframes) {
		unsigned frames = 0;

		if (DataRemaining == 0) {
			if (!Loop || !Rewind()) {
				Ended = true;
				return(false);
			}
			continue;
		}

		if (Aud.Is_Chunked()) {
			AUDChunkHeaderType chunk;
			if (DataRemaining < sizeof(chunk) || Source->Read(&chunk, sizeof(chunk)) != sizeof(chunk)) {
				DataRemaining = 0;
				continue;
			}
			DataRemaining -= sizeof(chunk);
			if (chunk.Magic != AUD_CHUNK_MAGIC || chunk.CompSize > DataRemaining || chunk.CompSize > Compressed.size()) {
				DataRemaining = 0;
				continue;
			}
			if (Source->Read(Compressed.data(), chunk.CompSize) != chunk.CompSize) {
				DataRemaining = 0;
				continue;
			}
			DataRemaining -= chunk.CompSize;
			frames = Aud.Decode_Chunk(Compressed.data(), chunk.CompSize, chunk.UncompSize, Pcm.data(), chunkframes);
			if (frames == 0) {
				DataRemaining = 0;
				continue;
			}
		} else {
			size_t bytes = (size_t)chunkframes * framebytes;
			if (bytes > DataRemaining) {
				bytes = DataRemaining;
			}
			bytes -= bytes % framebytes;
			if (bytes == 0) {
				DataRemaining = 0;
				continue;
			}
			size_t got = Source->Read(Compressed.data(), bytes < Compressed.size() ? bytes : Compressed.size());
			if (got == 0) {
				DataRemaining = 0;
				continue;
			}
			DataRemaining -= got;
			frames = Aud.Convert_Raw(Compressed.data(), got, Pcm.data(), chunkframes);
		}

		written += stream.Ring.Write(Pcm.data(), frames);
	}
	return(true);
}


bool AudioFileStreamProducerClass::Fill_Other(AudioStreamClass & stream)
{
	unsigned written = 0;
	while (written < MAX_FRAMES_PER_PASS && stream.Ring.Available_Write() >= OTHER_READ_FRAMES) {
		unsigned frames = Other.Read(Pcm.data(), OTHER_READ_FRAMES);
		if (frames == 0) {
			if (!Loop || !Rewind()) {
				Ended = true;
				return(false);
			}
			continue;
		}
		written += stream.Ring.Write(Pcm.data(), frames);
	}
	return(true);
}


AudioPushStreamProducerClass::AudioPushStreamProducerClass(void) :
	Stream(nullptr)
{
}


bool AudioPushStreamProducerClass::Open(AudioStreamClass & stream)
{
	Stream = &stream;
	Staging.resize((size_t)AUDIO_MAX_RENDER_FRAMES * 4 * stream.Channels());
	return(true);
}


unsigned AudioPushStreamProducerClass::Free_Frames(void) const
{
	return(Stream != nullptr ? Stream->Ring.Available_Write() : 0);
}


unsigned AudioPushStreamProducerClass::Push(void const * pcm, unsigned bytes, bool eightbit)
{
	if (Stream == nullptr || pcm == nullptr) {
		return(0);
	}
	unsigned channels = Stream->Channels();
	if (!eightbit) {
		unsigned frames = bytes / (channels * sizeof(int16_t));
		return(Stream->Ring.Write((int16_t const *)pcm, frames));
	}

	unsigned written = 0;
	unsigned frames = bytes / channels;
	unsigned char const * source = (unsigned char const *)pcm;
	unsigned stagingframes = (unsigned)(Staging.size() / channels);
	while (written < frames) {
		unsigned count = frames - written;
		if (count > stagingframes) {
			count = stagingframes;
		}
		for (unsigned i = 0; i < count * channels; i++) {
			Staging[i] = (int16_t)(((int)source[(size_t)written * channels + i] - 128) << 8);
		}
		unsigned done = Stream->Ring.Write(Staging.data(), count);
		written += done;
		if (done < count) {
			break;
		}
	}
	return(written);
}


void AudioPushStreamProducerClass::Mark_End(void)
{
	if (Stream != nullptr) {
		Stream->EndOfInput.store(true, std::memory_order_release);
	}
}


bool AudioPushStreamProducerClass::Fill(AudioStreamClass & stream)
{
	(void)stream;
	return(true);
}


void AudioPushStreamProducerClass::Close(void)
{
	Stream = nullptr;
}


AudioFeederClass::AudioFeederClass(void) :
	Exit(false),
	Running(false),
	Recovering(false),
	PassCount(0),
	Mixer(nullptr),
	Device(nullptr)
{
}


AudioFeederClass::~AudioFeederClass(void)
{
	Stop();
}


bool AudioFeederClass::Setup(AudioMixerClass * mixer, AudioDeviceClass * device)
{
	if (Running.load(std::memory_order_acquire) || mixer == nullptr) {
		return(false);
	}
	Mixer = mixer;
	Device = device;
	Scratch.reset(new (std::nothrow) float[(size_t)AUDIO_MAX_RENDER_FRAMES * AUDIO_MIX_CHANNELS]);
	return(Scratch != nullptr);
}


bool AudioFeederClass::Start(void)
{
	if (Running.load(std::memory_order_acquire) || Mixer == nullptr || Scratch == nullptr) {
		return(false);
	}
	Exit.store(false, std::memory_order_release);
	Running.store(true, std::memory_order_release);
	try {
		Thread = std::thread([this]() { Run(); });
	} catch (...) {
		Running.store(false, std::memory_order_release);
		return(false);
	}
	return(true);
}


void AudioFeederClass::Stop(void)
{
	if (Thread.joinable()) {
		Exit.store(true, std::memory_order_release);
		Thread.join();
	}
	Running.store(false, std::memory_order_release);
	for (SlotClass & slot : Slots) {
		if (slot.Producer != nullptr) {
			slot.Producer->Close();
		}
		slot.Producer = nullptr;
		slot.Stream = nullptr;
		slot.State.store(SLOT_IDLE, std::memory_order_release);
	}
	Recovering.store(false, std::memory_order_release);
}


bool AudioFeederClass::Attach(unsigned slot, AudioStreamClass * stream, AudioStreamProducerClass * producer)
{
	if (slot >= AUDIO_MAX_STREAMS || stream == nullptr || producer == nullptr) {
		return(false);
	}
	SlotClass & s = Slots[slot];
	if (s.State.load(std::memory_order_acquire) != SLOT_IDLE || stream->Ring.Capacity() < producer->Min_Ring_Frames()) {
		return(false);
	}
	s.Stream = stream;
	s.Producer = producer;
	s.State.store(SLOT_ACTIVE, std::memory_order_release);
	return(true);
}


void AudioFeederClass::Detach(unsigned slot)
{
	if (slot >= AUDIO_MAX_STREAMS) {
		return;
	}
	SlotClass & s = Slots[slot];
	if (s.State.load(std::memory_order_acquire) != SLOT_ACTIVE) {
		return;
	}
	if (!Running.load(std::memory_order_acquire)) {
		s.Producer->Close();
		s.Producer = nullptr;
		s.Stream = nullptr;
		s.State.store(SLOT_IDLE, std::memory_order_release);
		return;
	}
	s.State.store(SLOT_CLOSING, std::memory_order_release);
	while (s.State.load(std::memory_order_acquire) != SLOT_IDLE) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}


bool AudioFeederClass::Is_Attached(unsigned slot) const
{
	return(slot < AUDIO_MAX_STREAMS && Slots[slot].State.load(std::memory_order_acquire) == SLOT_ACTIVE);
}


void AudioFeederClass::Run(void)
{
	while (!Exit.load(std::memory_order_acquire)) {
		Service();
		std::this_thread::sleep_for(std::chrono::milliseconds(AUDIO_FEEDER_PERIOD_MS));
	}
}


void AudioFeederClass::Service(void)
{
	Service_Streams();
	Service_Device();
	PassCount.fetch_add(1, std::memory_order_relaxed);
}


void AudioFeederClass::Service_Streams(void)
{
	for (SlotClass & slot : Slots) {
		int state = slot.State.load(std::memory_order_acquire);
		if (state == SLOT_ACTIVE) {
			if (!slot.Producer->Fill(*slot.Stream)) {
				slot.Stream->EndOfInput.store(true, std::memory_order_release);
			}
		} else if (state == SLOT_CLOSING) {
			slot.Producer->Close();
			slot.Producer = nullptr;
			slot.Stream = nullptr;
			slot.State.store(SLOT_IDLE, std::memory_order_release);
		}
	}
}


void AudioFeederClass::Service_Device(void)
{
	if (Device == nullptr || Mixer == nullptr) {
		return;
	}
	bool lost = Device->Is_Lost() && !Device->Is_Running();
	if (!lost) {
		Recovering.store(false, std::memory_order_release);
		return;
	}

	auto now = std::chrono::steady_clock::now();
	if (!Recovering.load(std::memory_order_acquire)) {
		Recovering.store(true, std::memory_order_release);
		LostSince = now;
		LastRetry = now;
		LastPump = now;
	}

	// Render into the void at the wall-clock rate so voices finish, streams
	// drain and the movie clock advances.
	double seconds = std::chrono::duration<double>(now - LastPump).count();
	unsigned frames = (unsigned)(seconds * Mixer->Rate());
	if (frames > MAX_PUMP_FRAMES) {
		frames = MAX_PUMP_FRAMES;
	}
	if (frames > 0) {
		LastPump = now;
		while (frames > 0) {
			unsigned count = frames > AUDIO_MAX_RENDER_FRAMES ? AUDIO_MAX_RENDER_FRAMES : frames;
			Mixer->Render(Scratch.get(), count);
			frames -= count;
		}
	}

	if (now - LastRetry >= std::chrono::milliseconds(AUDIO_DEVICE_RETRY_MS)) {
		LastRetry = now;
		if (!Device->Start() && now - LostSince >= std::chrono::milliseconds(AUDIO_DEVICE_REOPEN_MS)) {
			unsigned rate = Mixer->Rate();
			unsigned channels = Mixer->Channels();
			Device->Close();
			if (Device->Open(rate, channels, AudioMixerClass::Render_Callback, Mixer)) {
				Device->Start();
			}
			LostSince = now;
		}
	}
}
