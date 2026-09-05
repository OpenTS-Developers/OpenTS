/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Drives the mixer through the null device and checks what comes out: levels
// and the loudness curve, the pan law, resampling, gapless sequences and
// looping, the two ways a sequence ends, fades and stops, stale commands,
// pausing, the command ring's limit, the render token, streams, and that two
// identical runs produce identical output. Needs no game data.

#include "audio/audiodevice.h"
#include "audio/audiolevel.h"
#include "audio/audiomixer.h"
#include "audio/audiosample.h"
#include "audio/audiostream.h"
#include "audio/audiovoice.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

namespace {

int Failures = 0;
int Checked = 0;

unsigned const RATE = 48000;
float const PI = 3.14159265f;


void Check(bool condition, char const * what)
{
	Checked++;
	if (!condition) {
		Failures++;
		std::printf("FAIL: %s\n", what);
	}
}


bool Near(float value, float expect, float tolerance)
{
	return(std::fabs(value - expect) <= tolerance);
}


std::unique_ptr<AudioSampleClass> Make_Clip(unsigned rate, unsigned channels, unsigned frames)
{
	std::unique_ptr<AudioSampleClass> clip(new AudioSampleClass());
	clip->Rate = rate;
	clip->Channels = channels;
	clip->Frames = frames;
	clip->Pcm.reset(new int16_t[(size_t)frames * channels]());
	return(clip);
}


std::unique_ptr<AudioSampleClass> Make_Constant(unsigned rate, unsigned frames, float value)
{
	std::unique_ptr<AudioSampleClass> clip = Make_Clip(rate, 1, frames);
	for (unsigned i = 0; i < frames; i++) {
		clip->Pcm[i] = (int16_t)(value * 32767.0f);
	}
	return(clip);
}


std::unique_ptr<AudioSampleClass> Make_Sine(unsigned rate, unsigned frames, float hertz, float amplitude)
{
	std::unique_ptr<AudioSampleClass> clip = Make_Clip(rate, 1, frames);
	for (unsigned i = 0; i < frames; i++) {
		clip->Pcm[i] = (int16_t)(amplitude * 32767.0f * std::sin(2.0f * PI * hertz * (float)i / (float)rate));
	}
	return(clip);
}


AudioSequenceClass Single(AudioSampleClass const * clip, int cycles = 1)
{
	AudioSequenceClass sequence;
	std::memset(&sequence, 0, sizeof(sequence));
	sequence.Clips[0] = clip;
	sequence.Count = 1;
	sequence.LoopStart = 0;
	sequence.LoopEnd = 1;
	sequence.Cycles = cycles;
	return(sequence);
}


AudioCommand Play(unsigned slot, uint32_t generation, AudioSequenceClass const * sequence, float level = 1.0f, float pan = 0.0f, float pitch = 1.0f, AudioGroupType group = AUDIO_GROUP_SFX)
{
	AudioCommand command;
	std::memset(&command, 0, sizeof(command));
	command.Type = AudioCommandType::PLAY_SEQUENCE;
	command.Group = (uint8_t)group;
	command.Slot = (uint8_t)slot;
	command.Generation = generation;
	command.A = level;
	command.B = pan;
	command.C = pitch;
	command.Ptr = sequence;
	return(command);
}


AudioCommand Simple(AudioCommandType type, unsigned slot, uint32_t generation, float a = 0.0f, float b = 0.0f, uint8_t mode = 0)
{
	AudioCommand command;
	std::memset(&command, 0, sizeof(command));
	command.Type = type;
	command.Slot = (uint8_t)slot;
	command.Generation = generation;
	command.Mode = mode;
	command.A = a;
	command.B = b;
	return(command);
}


// A mixer wired to the null device with a capture buffer.
class RigClass
{
	public:
		RigClass(void) : Device(480, 3)
		{
			Ready = Mixer.Init(RATE, 2) && Device.Open(RATE, 2, AudioMixerClass::Render_Callback, &Mixer) && Device.Start();
		}

		// Renders frames and appends them to the capture.
		void Run(unsigned frames)
		{
			size_t start = Output.size();
			Output.resize(start + (size_t)frames * 2);
			Device.Pump(Output.data() + start, frames);
		}

		float Rms(size_t from, size_t to, int channel) const
		{
			double sum = 0.0;
			size_t count = 0;
			for (size_t i = from; i < to && i < Output.size() / 2; i++) {
				float v = Output[i * 2 + channel];
				sum += (double)v * v;
				count++;
			}
			return(count ? (float)std::sqrt(sum / (double)count) : 0.0f);
		}

		bool Silent(size_t from, size_t to) const
		{
			for (size_t i = from; i < to && i < Output.size() / 2; i++) {
				if (Output[i * 2] != 0.0f || Output[i * 2 + 1] != 0.0f) {
					return(false);
				}
			}
			return(true);
		}

		size_t Last_Audible(float threshold = 0.0005f) const
		{
			for (size_t i = Output.size() / 2; i > 0; i--) {
				if (std::fabs(Output[(i - 1) * 2]) > threshold || std::fabs(Output[(i - 1) * 2 + 1]) > threshold) {
					return(i - 1);
				}
			}
			return(0);
		}

		bool Start(unsigned slot, AudioCommand const & command)
		{
			return(Mixer.Allocate_Voice(slot) && Mixer.Push(command));
		}

		bool Ready;
		AudioMixerClass Mixer;
		NullAudioDeviceClass Device;
		std::vector<float> Output;
};


void Test_Level_And_Curve(void)
{
	RigClass rig;
	Check(rig.Ready, "rig initialises");

	std::unique_ptr<AudioSampleClass> sine = Make_Sine(RATE, RATE * 3, 440.0f, 0.5f);
	AudioSequenceClass sequence = Single(sine.get());

	Check(rig.Start(0, Play(0, 1, &sequence, 1.0f)), "play at full level");
	rig.Run(RATE / 2);
	float rms = rig.Rms(1000, RATE / 2, 0);
	Check(Near(rms, 0.5f / 1.41421f, 0.01f), "full level plays the sine at its own amplitude");
	Check(Near(rig.Rms(1000, RATE / 2, 1), rms, 0.001f), "centre pan is equal in both channels");

	// A 0.5 level is the DirectSound curve's 0.317 gain, and 128/255 the same.
	rig.Mixer.Push(Simple(AudioCommandType::SET_GAIN, 0, 1, 0.5f, 0.0f));
	rig.Run(RATE / 2);
	float half = rig.Rms(RATE / 2 + 1000, RATE, 0);
	Check(Near(half / rms, Audio_Perceptual_Gain(0.5f), 0.01f), "half level follows the loudness curve");
	Check(Near(Audio_Perceptual_Gain(128.0f / 255.0f), 0.317f, 0.002f), "the curve matches the old driver at 128");

	// The group level multiplies the voice level before the curve, so two halves make a quarter.
	AudioCommand group = Simple(AudioCommandType::GROUP_SET_GAIN, 0, 0, 0.5f, 0.0f);
	group.Group = AUDIO_GROUP_SFX;
	rig.Mixer.Push(group);
	rig.Run(RATE / 2);
	float quarter = rig.Rms(RATE + 1000, RATE * 3 / 2, 0);
	Check(Near(quarter / rms, Audio_Perceptual_Gain(0.25f), 0.01f), "group and voice levels multiply before the curve");

	AudioCommand duck = Simple(AudioCommandType::GROUP_SET_DUCK, 0, 0, 0.5f, 0.0f);
	duck.Group = AUDIO_GROUP_SFX;
	rig.Mixer.Push(duck);
	rig.Mixer.Push(Simple(AudioCommandType::MASTER_SET_GAIN, 0, 0, 0.5f, 0.0f));
	rig.Run(RATE / 2);
	float sixteenth = rig.Rms(RATE * 3 / 2 + 1000, RATE * 2, 0);
	Check(Near(sixteenth / rms, Audio_Perceptual_Gain(0.0625f), 0.005f), "duck and master multiply in as well");
}


void Test_Pan(void)
{
	RigClass rig;
	std::unique_ptr<AudioSampleClass> sine = Make_Sine(RATE, RATE * 2, 440.0f, 0.5f);
	AudioSequenceClass sequence = Single(sine.get());

	Check(rig.Start(0, Play(0, 1, &sequence, 1.0f, 1.0f)), "play panned right");
	rig.Run(4800);
	Check(rig.Rms(100, 4800, 0) < 0.0001f && rig.Rms(100, 4800, 1) > 0.3f, "full right pan silences the left channel");

	rig.Mixer.Push(Simple(AudioCommandType::SET_PAN, 0, 1, -1.0f, 0.0f));
	rig.Run(4800);
	Check(rig.Rms(4900, 9600, 1) < 0.0001f && rig.Rms(4900, 9600, 0) > 0.3f, "full left pan silences the right channel");

	rig.Mixer.Push(Simple(AudioCommandType::SET_PAN, 0, 1, 0.0f, 0.0f));
	rig.Run(4800);
	Check(Near(rig.Rms(9700, 14400, 0), rig.Rms(9700, 14400, 1), 0.001f), "centre pan is unity on both sides");
	Check(Near(rig.Rms(9700, 14400, 0), 0.5f / 1.41421f, 0.01f), "centre pan does not attenuate");

	// A ramped pan moves gradually.
	rig.Mixer.Push(Simple(AudioCommandType::SET_PAN, 0, 1, 1.0f, 0.1f));
	rig.Run(2400);
	float mid = rig.Rms(14400 + 2000, 14400 + 2400, 0);
	Check(mid > 0.05f && mid < 0.3f, "a pan ramp is part way at half time");
}


void Test_Loop_And_Resample(void)
{
	RigClass rig;

	std::unique_ptr<AudioSampleClass> dc = Make_Constant(RATE, 100, 0.5f);
	AudioSequenceClass forever = Single(dc.get(), -1);
	Check(rig.Start(0, Play(0, 1, &forever)), "play a looping clip");
	rig.Run(RATE);
	bool gap = false;
	for (size_t i = 200; i < RATE; i++) {
		if (rig.Output[i * 2] < 0.4f) gap = true;
	}
	Check(!gap, "a looping clip has no gap at the seam");
	Check(rig.Mixer.Voice_State(0) == AudioVoiceState::PLAYING, "a looping clip keeps playing");
	rig.Mixer.Push(Simple(AudioCommandType::STOP, 0, 1));
	rig.Run(480);
	Check(rig.Mixer.Voice_State(0) == AudioVoiceState::DONE, "a stopped loop finishes");
	rig.Mixer.Free_Voice(0);

	// One second at 22050 lasts one second at 48000.
	RigClass low;
	std::unique_ptr<AudioSampleClass> sine = Make_Sine(22050, 22050, 440.0f, 0.5f);
	AudioSequenceClass once = Single(sine.get());
	Check(low.Start(0, Play(0, 1, &once)), "play a 22050 clip");
	low.Run(RATE * 2);
	Check(low.Mixer.Voice_State(0) == AudioVoiceState::DONE, "a one-shot finishes");
	size_t last = low.Last_Audible();
	Check(last >= RATE - 32 && last <= RATE + 32, "a 22050 clip lasts its length at 48000");
	Check(Near(low.Rms(2000, RATE - 2000, 0), 0.5f / 1.41421f, 0.02f), "resampled sine keeps its amplitude");

	// Pitch doubles the rate ratio, so the clip plays in half the time.
	RigClass fast;
	Check(fast.Start(0, Play(0, 1, &once, 1.0f, 0.0f, 2.0f)), "play at double pitch");
	fast.Run(RATE);
	last = fast.Last_Audible();
	Check(last >= RATE / 2 - 32 && last <= RATE / 2 + 32, "double pitch halves the duration");

	// Finite cycles: three cycles of a 100-frame clip, then done.
	RigClass thrice;
	AudioSequenceClass three = Single(dc.get(), 3);
	Check(thrice.Start(0, Play(0, 1, &three)), "play three cycles");
	thrice.Run(2000);
	Check(thrice.Mixer.Voice_State(0) == AudioVoiceState::DONE, "three cycles finish");
	last = thrice.Last_Audible(0.05f);
	Check(last >= 300 - 8 && last <= 300 + 24, "three cycles last three clip lengths");
}


void Test_Sequence(void)
{
	// 5 ms attack, 5 ms body looping forever, 5 ms decay at a different level.
	unsigned const SEG = RATE / 200;
	std::unique_ptr<AudioSampleClass> attack = Make_Constant(RATE, SEG, 0.5f);
	std::unique_ptr<AudioSampleClass> body = Make_Constant(RATE, SEG, 0.5f);
	std::unique_ptr<AudioSampleClass> decay = Make_Constant(RATE, SEG, 0.25f);

	AudioSequenceClass sequence;
	std::memset(&sequence, 0, sizeof(sequence));
	sequence.Clips[0] = attack.get();
	sequence.Clips[1] = body.get();
	sequence.Clips[2] = decay.get();
	sequence.Count = 3;
	sequence.LoopStart = 1;
	sequence.LoopEnd = 2;
	sequence.Cycles = -1;

	RigClass rig;
	Check(rig.Start(0, Play(0, 1, &sequence)), "play a short attack/body/decay sequence");
	rig.Run(RATE / 2);
	bool gap = false;
	for (size_t i = 100; i < RATE / 2; i++) {
		if (rig.Output[i * 2] < 0.4f) gap = true;
	}
	Check(!gap, "five millisecond segments chain without a gap while no command arrives for half a second");
	Check(rig.Mixer.Voice_State(0) == AudioVoiceState::PLAYING, "the body keeps looping");

	// After the cycle: the rest of the body, then the decay once, then done.
	rig.Mixer.Push(Simple(AudioCommandType::END_SEQUENCE, 0, 1, 0.0f, 0.0f, (uint8_t)AudioEndMode::AFTER_CYCLE));
	size_t mark = rig.Output.size() / 2;
	rig.Run(RATE / 10);
	Check(rig.Mixer.Voice_State(0) == AudioVoiceState::DONE, "the sequence finishes after the cycle");
	unsigned decayframes = 0;
	unsigned bodyframes = 0;
	for (size_t i = mark; i < rig.Output.size() / 2; i++) {
		float v = rig.Output[i * 2];
		if (v > 0.15f && v < 0.35f) decayframes++;
		if (v > 0.4f) bodyframes++;
	}
	Check(decayframes >= SEG - 24 && decayframes <= SEG + 24, "the decay plays exactly once after the cycle");
	Check(bodyframes <= SEG + 8, "no more than one body cycle plays after the end request");
	rig.Mixer.Free_Voice(0);

	// End now on a long body: silence within the ramp, then the decay once.
	std::unique_ptr<AudioSampleClass> longbody = Make_Constant(RATE, RATE * 2, 0.5f);
	sequence.Clips[1] = longbody.get();
	RigClass now;
	Check(now.Start(0, Play(0, 1, &sequence)), "play a long body");
	now.Run(RATE / 2);
	now.Mixer.Push(Simple(AudioCommandType::END_SEQUENCE, 0, 1, 0.0f, 0.0f, (uint8_t)AudioEndMode::NOW));
	mark = now.Output.size() / 2;
	now.Run(RATE / 2);
	Check(now.Mixer.Voice_State(0) == AudioVoiceState::DONE, "end now finishes the voice");
	size_t quiet = mark;
	for (size_t i = mark; i < now.Output.size() / 2; i++) {
		if (now.Output[i * 2] < 0.01f) {
			quiet = i;
			break;
		}
	}
	Check(quiet - mark <= RATE / 4 + 64, "end now reaches silence within the ramp");
	decayframes = 0;
	for (size_t i = quiet; i < now.Output.size() / 2; i++) {
		float v = now.Output[i * 2];
		if (v > 0.15f && v < 0.35f) decayframes++;
	}
	Check(decayframes >= SEG - 24 && decayframes <= SEG + 24, "end now plays the decay exactly once");

	// A sequence with clips at two rates chains without dropping either.
	std::unique_ptr<AudioSampleClass> slow = Make_Constant(22050, 2205, 0.5f);
	AudioSequenceClass mixed;
	std::memset(&mixed, 0, sizeof(mixed));
	mixed.Clips[0] = slow.get();
	mixed.Clips[1] = body.get();
	mixed.Count = 2;
	mixed.LoopStart = 1;
	mixed.LoopEnd = 2;
	mixed.Cycles = 1;
	RigClass rates;
	Check(rates.Start(0, Play(0, 1, &mixed)), "play clips at two rates");
	rates.Run(RATE / 4);
	Check(rates.Mixer.Voice_State(0) == AudioVoiceState::DONE, "mixed-rate sequence finishes");
	size_t last = rates.Last_Audible(0.05f);
	Check(last >= 4800 + SEG - 40 && last <= 4800 + SEG + 40, "both rates contribute their full length");
}


void Test_Stop_Pause_Generation(void)
{
	RigClass rig;
	std::unique_ptr<AudioSampleClass> sine = Make_Sine(RATE, RATE * 4, 440.0f, 0.5f);
	AudioSequenceClass sequence = Single(sine.get());

	Check(rig.Start(0, Play(0, 7, &sequence)), "play for the fade test");
	rig.Run(4800);
	rig.Mixer.Push(Simple(AudioCommandType::STOP, 0, 7, 0.1f));
	rig.Run(RATE / 5);
	Check(rig.Mixer.Voice_State(0) == AudioVoiceState::DONE, "a fading stop reaches done");
	size_t last = rig.Last_Audible();
	// The level ramps linearly and the curve makes the tail inaudible a little early.
	Check(last >= 4800 + RATE / 10 - 200 && last <= 4800 + RATE / 10 + 64, "the fade lasts the requested time");
	rig.Mixer.Free_Voice(0);

	Check(rig.Start(0, Play(0, 8, &sequence)), "play for the generation test");
	rig.Run(480);
	unsigned dropped = rig.Mixer.Dropped_Commands();
	rig.Mixer.Push(Simple(AudioCommandType::SET_GAIN, 0, 9, 0.0f, 0.0f));
	size_t mark = rig.Output.size() / 2;
	rig.Run(480);
	Check(rig.Mixer.Dropped_Commands() == dropped + 1, "a stale generation is dropped");
	Check(rig.Rms(mark, mark + 480, 0) > 0.3f, "a stale gain change has no effect");

	rig.Mixer.Push(Simple(AudioCommandType::PAUSE, 0, 8));
	mark = rig.Output.size() / 2;
	rig.Run(480);
	Check(rig.Silent(mark, mark + 480), "a paused voice is silent");
	Check(rig.Mixer.Voice_State(0) == AudioVoiceState::PAUSED, "the voice reports paused");
	rig.Mixer.Push(Simple(AudioCommandType::RESUME, 0, 8));
	mark = rig.Output.size() / 2;
	rig.Run(480);
	Check(rig.Rms(mark + 64, mark + 480, 0) > 0.3f, "a resumed voice plays again");

	// Pausing everything ramps down, freezes, and resumes where it left off.
	rig.Mixer.Set_Pause_All(true);
	mark = rig.Output.size() / 2;
	rig.Run(4800);
	Check(rig.Silent(mark + 1000, mark + 4800), "pause all freezes the mix after its ramp");
	rig.Mixer.Set_Pause_All(false);
	mark = rig.Output.size() / 2;
	rig.Run(480);
	Check(rig.Rms(mark + 300, mark + 480, 0) > 0.3f, "resume all plays again");

	rig.Mixer.Push(Simple(AudioCommandType::STOP_ALL, 0, 0));
	rig.Run(480);
	Check(rig.Mixer.Voice_State(0) == AudioVoiceState::DONE, "stop all finishes every voice");

	// Play on a slot that was not allocated is refused.
	dropped = rig.Mixer.Dropped_Commands();
	rig.Mixer.Free_Voice(0);
	rig.Mixer.Push(Play(0, 10, &sequence));
	rig.Run(64);
	Check(rig.Mixer.Dropped_Commands() == dropped + 1, "a play on an unallocated voice is dropped");
	Check(rig.Mixer.Voice_State(0) == AudioVoiceState::FREE, "the unallocated voice stays free");
}


void Test_Ring_And_Token(void)
{
	RigClass rig;
	unsigned pushed = 0;
	for (unsigned i = 0; i < AUDIO_COMMAND_QUEUE_SIZE + 5; i++) {
		if (rig.Mixer.Push(Simple(AudioCommandType::MASTER_SET_GAIN, 0, 0, 1.0f, 0.0f))) {
			pushed++;
		}
	}
	Check(pushed == AUDIO_COMMAND_QUEUE_SIZE, "the command ring refuses pushes past its capacity");
	rig.Run(64);
	Check(rig.Mixer.Push(Simple(AudioCommandType::MASTER_SET_GAIN, 0, 0, 1.0f, 0.0f)), "the ring accepts again once drained");

	std::unique_ptr<AudioSampleClass> sine = Make_Sine(RATE, RATE, 440.0f, 0.5f);
	AudioSequenceClass sequence = Single(sine.get(), -1);
	Check(rig.Start(0, Play(0, 1, &sequence)), "play for the token test");
	size_t mark = rig.Output.size() / 2;
	rig.Run(480);
	Check(rig.Rms(mark + 100, mark + 480, 0) > 0.3f, "the voice is audible before the token is taken");
	Check(rig.Mixer.Acquire_Render_Token(), "a second renderer can take the token when it is free");
	mark = rig.Output.size() / 2;
	rig.Run(480);
	Check(rig.Silent(mark, mark + 480), "a render call without the token writes silence");
	rig.Mixer.Release_Render_Token();
	mark = rig.Output.size() / 2;
	rig.Run(480);
	Check(rig.Rms(mark + 64, mark + 480, 0) > 0.3f, "rendering resumes once the token is released");
}


void Test_Stream(void)
{
	RigClass rig;
	AudioStreamClass stream;
	Check(stream.Init(4096, 1, 22050), "stream initialises");

	std::vector<int16_t> chunk(2205, (int16_t)(0.5f * 32767.0f));
	Check(stream.Ring.Write(chunk.data(), (unsigned)chunk.size()) == chunk.size(), "first chunk fits");

	AudioCommand play;
	std::memset(&play, 0, sizeof(play));
	play.Type = AudioCommandType::PLAY_STREAM;
	play.Group = AUDIO_GROUP_MUSIC;
	play.Slot = 3;
	play.Generation = 1;
	play.A = 1.0f;
	play.C = 1.0f;
	play.Ptr = &stream;
	Check(rig.Start(3, play), "play the stream");

	rig.Run(2400);
	Check(rig.Rms(100, 2400, 0) > 0.3f, "stream audio plays");
	Check(stream.Frames_Consumed() > 1000, "the stream counts frames consumed");

	// Let it run dry: silence, an underrun count, and the voice stays alive.
	rig.Run(RATE / 4);
	Check(stream.Frames_Consumed() == 2205, "silence during an underrun is not counted as consumed");
	Check(stream.Underruns.load() > 0, "underruns are counted");
	Check(rig.Mixer.Voice_State(3) == AudioVoiceState::PLAYING, "an underrun keeps the voice alive");
	Check(rig.Silent(RATE / 8, RATE / 4), "an underrun plays silence");

	Check(stream.Ring.Write(chunk.data(), (unsigned)chunk.size()) == chunk.size(), "refill after underrun");
	rig.Run(2400);
	Check(rig.Rms(2400 + RATE / 4 + 200, 2400 + RATE / 4 + 2400, 0) > 0.3f, "playback resumes when data arrives");

	stream.EndOfInput.store(true);
	rig.Run(RATE / 4);
	Check(rig.Mixer.Voice_State(3) == AudioVoiceState::DONE, "end of input finishes the stream voice once drained");
	Check(stream.Frames_Consumed() == 4410, "every pushed frame was consumed");
}


void Test_Determinism(void)
{
	std::unique_ptr<AudioSampleClass> sine = Make_Sine(22050, 22050, 330.0f, 0.4f);
	std::unique_ptr<AudioSampleClass> dc = Make_Constant(RATE, 300, 0.3f);
	AudioSequenceClass a = Single(sine.get());
	AudioSequenceClass b = Single(dc.get(), -1);

	std::vector<float> runs[2];
	for (int run = 0; run < 2; run++) {
		RigClass rig;
		rig.Start(0, Play(0, 1, &a, 0.8f, -0.3f));
		rig.Start(5, Play(5, 2, &b, 0.6f, 0.5f));
		rig.Run(4000);
		rig.Mixer.Push(Simple(AudioCommandType::SET_GAIN, 5, 2, 0.2f, 0.05f));
		rig.Mixer.Push(Simple(AudioCommandType::SET_PAN, 0, 1, 0.9f, 0.02f));
		rig.Run(4000);
		rig.Mixer.Push(Simple(AudioCommandType::STOP, 5, 2, 0.03f));
		rig.Run(20000);
		runs[run] = rig.Output;
	}
	Check(runs[0].size() == runs[1].size() && std::memcmp(runs[0].data(), runs[1].data(), runs[0].size() * sizeof(float)) == 0, "two identical runs render identical output");
}

} // namespace


int main(void)
{
	Test_Level_And_Curve();
	Test_Pan();
	Test_Loop_And_Resample();
	Test_Sequence();
	Test_Stop_Pause_Generation();
	Test_Ring_And_Token();
	Test_Stream();
	Test_Determinism();

	std::printf("audiomixer: %d checks, %d failures\n", Checked, Failures);
	return(Failures == 0 ? 0 : 1);
}
