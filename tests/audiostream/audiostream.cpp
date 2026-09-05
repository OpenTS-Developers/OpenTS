/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Exercises the stream producers and the feeder: an AUD streamed a chunk at a
// time matches its whole decode and loops without a seam, a WAV streams through
// miniaudio, pushed PCM converts and stops at the ring's edge, the feeder
// thread fills and releases streams, and a lost device is pumped and restarted
// while the game thread does nothing. Needs no game data.

#include "audio/audiodecode.h"
#include "audio/audiodevice.h"
#include "audio/audiomixer.h"
#include "audio/audiomovieclock.h"
#include "audio/audiostream.h"
#include "audio/audiovoice.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>

namespace {

int Failures = 0;
int Checked = 0;

unsigned int Seed = 777;


void Check(bool condition, char const * what)
{
	Checked++;
	if (!condition) {
		Failures++;
		std::printf("FAIL: %s\n", what);
	}
}


unsigned int Next_Random(void)
{
	Seed = Seed * 1103515245u + 12345u;
	return(Seed >> 8);
}


void Append(std::vector<uint8_t> & blob, void const * data, size_t size)
{
	uint8_t const * bytes = (uint8_t const *)data;
	blob.insert(blob.end(), bytes, bytes + size);
}


class MemoryByteSourceClass : public AudioByteSourceClass
{
	public:
		explicit MemoryByteSourceClass(std::vector<uint8_t> const & bytes) : Bytes(bytes), Cursor(0) {}

		size_t Read(void * buffer, size_t bytes) override
		{
			size_t available = Bytes.size() - Cursor;
			if (bytes > available) bytes = available;
			std::memcpy(buffer, Bytes.data() + Cursor, bytes);
			Cursor += bytes;
			return(bytes);
		}

		bool Seek(size_t position) override
		{
			if (position > Bytes.size()) return(false);
			Cursor = position;
			return(true);
		}

		size_t Position(void) const override { return(Cursor); }
		size_t Size(void) const override { return(Bytes.size()); }

	private:
		std::vector<uint8_t> Bytes;
		size_t Cursor;
};


std::vector<uint8_t> Make_Sos_File(unsigned chunks, unsigned rate)
{
	AUDHeaderType header;
	header.Rate = (uint16_t)rate;
	header.Size = 0;
	header.UncompSize = 0;
	header.Flags = AUD_FLAG_16BIT;
	header.Compression = AUD_CODEC_SOS;
	std::vector<uint8_t> blob;
	Append(blob, &header, sizeof(header));
	int32_t uncomp = 0;
	for (unsigned i = 0; i < chunks; i++) {
		unsigned comp = 200 + (Next_Random() % 300);
		comp -= comp % 4;
		AUDChunkHeaderType chunk;
		chunk.CompSize = (uint16_t)comp;
		chunk.UncompSize = (uint16_t)(comp * 4);
		chunk.Magic = AUD_CHUNK_MAGIC;
		Append(blob, &chunk, sizeof(chunk));
		for (unsigned b = 0; b < comp; b++) {
			uint8_t byte = (uint8_t)(Next_Random() & 0xFF);
			blob.push_back(byte);
		}
		uncomp += comp * 4;
	}
	int32_t size = (int32_t)(blob.size() - sizeof(header));
	std::memcpy(&blob[2], &size, sizeof(size));
	std::memcpy(&blob[6], &uncomp, sizeof(uncomp));
	return(blob);
}


std::vector<uint8_t> Make_Wav(unsigned frames, unsigned rate)
{
	std::vector<uint8_t> wav;
	uint32_t datasize = frames * 2;
	uint32_t riffsize = 36 + datasize;
	Append(wav, "RIFF", 4);
	Append(wav, &riffsize, 4);
	Append(wav, "WAVEfmt ", 8);
	uint32_t fmtsize = 16;
	uint16_t format = 1;
	uint16_t channels = 1;
	uint32_t byterate = rate * 2;
	uint16_t align = 2;
	uint16_t bits = 16;
	Append(wav, &fmtsize, 4);
	Append(wav, &format, 2);
	Append(wav, &channels, 2);
	Append(wav, &rate, 4);
	Append(wav, &byterate, 4);
	Append(wav, &align, 2);
	Append(wav, &bits, 2);
	Append(wav, "data", 4);
	Append(wav, &datasize, 4);
	for (unsigned i = 0; i < frames; i++) {
		int16_t sample = (int16_t)(i * 37);
		Append(wav, &sample, sizeof(sample));
	}
	return(wav);
}


// Drains a ring into a vector, in pieces, until the producer says it is done or
// stops making progress.
std::vector<int16_t> Drain(AudioFileStreamProducerClass & producer, AudioStreamClass & stream, unsigned maxframes)
{
	std::vector<int16_t> out;
	std::vector<int16_t> piece(1000 * stream.Channels());
	bool more = true;
	int stalls = 0;
	while (out.size() / stream.Channels() < maxframes && stalls < 100) {
		if (more) {
			more = producer.Fill(stream);
		}
		unsigned got = stream.Ring.Read(piece.data(), 1000);
		if (got == 0) {
			if (!more) break;
			stalls++;
			continue;
		}
		stalls = 0;
		out.insert(out.end(), piece.begin(), piece.begin() + (size_t)got * stream.Channels());
	}
	return(out);
}


void Test_Aud_Stream(void)
{
	std::vector<uint8_t> file = Make_Sos_File(30, 22050);

	AUDHeaderType header;
	Aud_Read_Header(file.data(), file.size(), header);
	std::vector<int16_t> whole(Aud_Frame_Capacity(header));
	AudioPcmFormat format;
	unsigned frames = Aud_Decode(file.data(), file.size(), whole.data(), (unsigned)whole.size(), format);
	whole.resize(frames);

	AudioStreamClass stream;
	Check(stream.Init(16000, 1, 22050), "stream ring of sixteen thousand frames");

	AudioFileStreamProducerClass producer;
	Check(producer.Open(std::unique_ptr<AudioByteSourceClass>(new MemoryByteSourceClass(file)), false), "AUD producer opens");
	Check(producer.Rate() == 22050 && producer.Channels() == 1, "AUD producer reports its format");
	Check(producer.Min_Ring_Frames() > 0 && producer.Min_Ring_Frames() <= stream.Ring.Capacity(), "the ring is large enough for a chunk");

	// A ring smaller than one chunk cannot be fed and is reported as ended, not spun on.
	AudioStreamClass tiny;
	tiny.Init(100, 1, 22050);
	AudioFileStreamProducerClass tinyproducer;
	tinyproducer.Open(std::unique_ptr<AudioByteSourceClass>(new MemoryByteSourceClass(file)), false);
	Check(!tinyproducer.Fill(tiny), "a ring too small for a chunk ends the producer");

	std::vector<int16_t> streamed = Drain(producer, stream, frames * 2);
	Check(streamed.size() == frames, "streamed frame count equals the whole decode");
	Check(streamed == whole, "streamed samples equal the whole decode");
	Check(!producer.Fill(stream), "a finished producer keeps reporting the end");

	// Looping: the second pass repeats the first without a seam.
	AudioStreamClass loopstream;
	loopstream.Init(16000, 1, 22050);
	AudioFileStreamProducerClass looper;
	Check(looper.Open(std::unique_ptr<AudioByteSourceClass>(new MemoryByteSourceClass(file)), true), "looping AUD producer opens");
	std::vector<int16_t> twice = Drain(looper, loopstream, frames * 2 + 500);
	Check(twice.size() >= frames * 2 + 500, "a looping producer keeps producing past the end");
	Check(std::memcmp(twice.data(), whole.data(), frames * sizeof(int16_t)) == 0, "the first pass matches");
	Check(std::memcmp(twice.data() + frames, whole.data(), frames * sizeof(int16_t)) == 0, "the second pass repeats the first from its start");

	std::vector<uint8_t> junk(50, 0x11);
	AudioFileStreamProducerClass bad;
	Check(!bad.Open(std::unique_ptr<AudioByteSourceClass>(new MemoryByteSourceClass(junk)), false), "unknown data is refused");
}


void Test_Wav_Stream(void)
{
	std::vector<uint8_t> file = Make_Wav(9000, 44100);
	AudioStreamClass stream;
	stream.Init(8192, 1, 44100);

	AudioFileStreamProducerClass producer;
	Check(producer.Open(std::unique_ptr<AudioByteSourceClass>(new MemoryByteSourceClass(file)), true), "WAV producer opens");
	Check(producer.Rate() == 44100 && producer.Channels() == 1, "WAV producer reports its format");

	std::vector<int16_t> out = Drain(producer, stream, 9000 + 100);
	Check(out.size() >= 9100, "WAV streams and loops");
	bool match = true;
	for (unsigned i = 0; i < 9000; i++) {
		if (out[i] != (int16_t)(i * 37)) match = false;
	}
	for (unsigned i = 0; i < 100; i++) {
		if (out[9000 + i] != (int16_t)(i * 37)) match = false;
	}
	Check(match, "WAV samples stream in order and restart from the top");
}


void Test_Push(void)
{
	AudioStreamClass stream;
	stream.Init(100, 2, 22050);
	AudioPushStreamProducerClass producer;
	Check(producer.Open(stream), "push producer opens");
	Check(producer.Free_Frames() == 128, "free frames follow the ring");

	unsigned char eight[8] = {128, 128, 0, 255, 64, 192, 128, 128};
	Check(producer.Push(eight, sizeof(eight), true) == 4, "8-bit stereo bytes become frames");
	int16_t frames[8];
	Check(stream.Ring.Read(frames, 4) == 4, "pushed frames can be read");
	Check(frames[0] == 0 && frames[2] == -32768 && frames[3] == 32512 && frames[4] == -16384, "8-bit samples are widened");

	int16_t sixteen[6] = {1, 2, 3, 4, 5, 6};
	Check(producer.Push(sixteen, sizeof(sixteen), false) == 3, "16-bit bytes become frames");
	Check(stream.Ring.Read(frames, 3) == 3 && frames[0] == 1 && frames[5] == 6, "16-bit samples pass through");

	std::vector<int16_t> big(400, 9);
	Check(producer.Push(big.data(), (unsigned)(big.size() * sizeof(int16_t)), false) == 128, "a push stops at the ring's edge");
	Check(producer.Free_Frames() == 0, "the ring is full after a clamped push");

	Check(!stream.EndOfInput.load(), "the stream is open before the end is marked");
	producer.Mark_End();
	Check(stream.EndOfInput.load(), "marking the end sets the flag");
}


void Test_Feeder(void)
{
	AudioMixerClass mixer;
	Check(mixer.Init(48000, 2), "mixer for the feeder");
	AudioFeederClass feeder;
	Check(!feeder.Start(), "the feeder cannot start before setup");
	Check(feeder.Setup(&mixer, nullptr) && feeder.Start(), "feeder starts");
	Check(feeder.Is_Running(), "feeder reports running");

	std::vector<uint8_t> file = Make_Sos_File(40, 22050);
	AudioStreamClass stream;
	stream.Init(20000, 1, 22050);
	AudioFileStreamProducerClass * producer = new AudioFileStreamProducerClass();
	Check(producer->Open(std::unique_ptr<AudioByteSourceClass>(new MemoryByteSourceClass(file)), false), "producer for the feeder");

	Check(feeder.Attach(1, &stream, producer), "attach to a slot");
	Check(!feeder.Attach(1, &stream, producer), "a busy slot refuses a second attach");
	Check(feeder.Is_Attached(1), "the slot reports attached");

	auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
	while (stream.Frames_Pushed() < 10000 && std::chrono::steady_clock::now() < deadline) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	Check(stream.Frames_Pushed() >= 10000, "the feeder thread fills the stream");

	feeder.Detach(1);
	Check(!feeder.Is_Attached(1), "detach releases the slot");
	delete producer;

	unsigned passes = feeder.Passes();
	std::this_thread::sleep_for(std::chrono::milliseconds(60));
	Check(feeder.Passes() > passes, "the feeder keeps running with nothing attached");
	feeder.Stop();
	Check(!feeder.Is_Running(), "feeder stops");
}


void Test_Recovery(void)
{
	AudioMixerClass mixer;
	NullAudioDeviceClass device(480, 3);
	Check(mixer.Init(48000, 2), "mixer for recovery");
	Check(device.Open(48000, 2, AudioMixerClass::Render_Callback, &mixer) && device.Start(), "device for recovery");

	AudioStreamClass stream;
	stream.Init(96000, 1, 48000);
	AudioPushStreamProducerClass producer;
	producer.Open(stream);
	std::vector<int16_t> pcm(48000, 1000);
	producer.Push(pcm.data(), (unsigned)(pcm.size() * sizeof(int16_t)), false);

	AudioCommand play;
	std::memset(&play, 0, sizeof(play));
	play.Type = AudioCommandType::PLAY_STREAM;
	play.Group = AUDIO_GROUP_MOVIE;
	play.Slot = 2;
	play.Generation = 1;
	play.A = 1.0f;
	play.C = 1.0f;
	play.Ptr = &stream;
	Check(mixer.Allocate_Voice(2) && mixer.Push(play), "stream voice for recovery");

	std::vector<float> out(4800 * 2);
	device.Pump(out.data(), 4800);
	uint32_t before = stream.Frames_Consumed();
	Check(before > 0, "the device consumes the stream while it runs");

	AudioFeederClass feeder;
	Check(feeder.Setup(&mixer, &device), "feeder set up without its thread");
	feeder.Attach(0, &stream, &producer);
	device.Set_Lost(true);
	Check(device.Is_Lost() && !device.Is_Running(), "the device reports lost");

	// The game thread is doing nothing; only the feeder's passes advance things.
	feeder.Service();
	Check(feeder.Is_Recovering(), "the feeder notices the loss");
	std::this_thread::sleep_for(std::chrono::milliseconds(60));
	feeder.Service();
	uint32_t during = stream.Frames_Consumed();
	Check(during > before, "the feeder pumps the mixer while the device is lost");
	Check(during - before >= 48000 * 50 / 1000 && during - before <= 48000 * 200 / 1000, "the pump follows the wall clock");

	auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(AUDIO_DEVICE_RETRY_MS + 500);
	while (!device.Is_Running() && std::chrono::steady_clock::now() < deadline) {
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		feeder.Service();
	}
	Check(device.Is_Running(), "the feeder restarts the device");
	feeder.Service();
	Check(!feeder.Is_Recovering(), "recovery ends once the device runs");
	feeder.Detach(0);
}

// The movie clock: ticks from frames heard, wall time while the sound stands
// still, repeated blocks taken off, and a pause left out of the count.
void Test_Movie_Clock(void)
{
	AudioMovieClockClass clock;
	unsigned const rate = 22050;
	unsigned const block = 4096;
	unsigned const latency = 441;
	clock.Reset(rate, block, latency, 60);

	Check(clock.Ticks(0, 0, 100) == 0, "nothing heard yet");
	Check(clock.Ticks(latency, 0, 101) == 0, "device latency is not heard");
	Check(clock.Ticks(4410 + latency, 0, 105) == 12, "ticks follow the frames heard");
	Check(clock.Ticks(4410 + latency, 0, 110) == 17, "wall time carries the clock while the sound stands still");
	Check(clock.Ticks(4410 + latency, 0, 108) == 17, "the wall clock never runs the count backwards");
	Check(clock.Ticks(8820 + latency, 0, 120) == 24, "a new position resets the count from the sound");
	Check(clock.Ticks(8820 + latency, 1, 130) == 34, "a repeated block is taken off, so the sound seems to stand still");
	Check(clock.Ticks(13230 + latency, 1, 140) == 24, "later frames minus the repeated block");

	clock.Pause();
	Check(clock.Is_Paused() && clock.Ticks(20000, 1, 200) == 24, "paused clock holds");
	clock.Resume(260);
	Check(clock.Ticks(13230 + latency, 1, 265) == 29, "the pause is left out of the count");
	Check(clock.Pause_Adjust() == 120, "pause adjust is the wall time lost");
	clock.Resume(300);
	Check(clock.Ticks(13230 + latency, 1, 270) == 34, "resume while running changes nothing");
}

} // namespace


int main(void)
{
	Test_Aud_Stream();
	Test_Wav_Stream();
	Test_Push();
	Test_Feeder();
	Test_Recovery();
	Test_Movie_Clock();

	std::printf("audiostream: %d checks, %d failures\n", Checked, Failures);
	return(Failures == 0 ? 0 : 1);
}
