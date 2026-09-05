/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Drives the event pool over a mixer rendered by hand and checks the sequences
// it builds, its state machine, the two ways an event ends, per-type limits,
// the effects budget and its stealing order, queued and delayed starts, the
// clock clamp, handles going stale, and that every pinned clip comes back.
// Needs no game data.

#include "audio/audioevent.h"
#include "audio/audiolevel.h"
#include "audio/audiomixer.h"
#include "audio/audiosample.h"
#include "audio/audiostream.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace {

int Failures = 0;
int Checked = 0;

unsigned const RATE = 48000;
unsigned const CLIP_FRAMES = 4800;   // 100 ms


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


// Hands out clips by type name and sound index and counts the pins.
class ProviderClass : public AudioClipProviderClass
{
	public:
		ProviderClass(void) : Acquired(0), Released(0) {}

		AudioSampleClass * Add(char const * type, unsigned sound, float value = 0.0f)
		{
			std::unique_ptr<AudioSampleClass> clip(new AudioSampleClass());
			clip->Rate = RATE;
			clip->Channels = 1;
			clip->Frames = CLIP_FRAMES;
			clip->Pcm.reset(new int16_t[CLIP_FRAMES]);
			for (unsigned i = 0; i < CLIP_FRAMES; i++) {
				clip->Pcm[i] = (int16_t)(value * 32767.0f);
			}
			AudioSampleClass * raw = clip.get();
			Owned.push_back(std::move(clip));
			std::vector<AudioSampleClass *> & list = Table[type];
			if (list.size() <= sound) {
				list.resize(sound + 1, nullptr);
			}
			list[sound] = raw;
			return(raw);
		}

		AudioSampleClass * Clip(char const * type, unsigned sound) const
		{
			auto it = Table.find(type);
			if (it == Table.end() || sound >= it->second.size()) {
				return(nullptr);
			}
			return(it->second[sound]);
		}

		AudioSampleClass * Acquire(AudioEventTypeClass const & type, unsigned sound) override
		{
			AudioSampleClass * clip = Clip(type.Name, sound);
			if (clip != nullptr) {
				clip->PinCount++;
				Acquired++;
			}
			return(clip);
		}

		void Release(AudioSampleClass * clip) override
		{
			clip->PinCount--;
			Released++;
		}

		// Pins a clip the way the cache would for a raw play.
		AudioSampleClass * Pin(AudioSampleClass * clip)
		{
			clip->PinCount++;
			Acquired++;
			return(clip);
		}

		bool Balanced(void) const { return(Acquired == Released); }

		int Acquired;
		int Released;

	private:
		std::vector<std::unique_ptr<AudioSampleClass>> Owned;
		std::map<std::string, std::vector<AudioSampleClass *>> Table;
};


int Fixed_Random(int low, int high, void * context)
{
	return(*(int *)context ? high : low);
}


// A pool over a mixer whose output the test renders and keeps.
class RigClass
{
	public:
		RigClass(void) : Now(0), RandomHigh(0)
		{
			Ready = Mixer.Init(RATE, 2) && Pool.Init(&Mixer, &Provider);
			Pool.Set_Random(Fixed_Random, &RandomHigh);
			Pool.Service(Now);
		}

		~RigClass(void)
		{
			Pool.Shutdown();
			Mixer.Shutdown();
		}

		// Registers a type with `count` sounds and clips for each.
		AudioEventTypeClass & Type(char const * name, unsigned count, unsigned control = SOUND_CONTROL_NONE, int attack = 0, int decay = 0, float value = 0.0f)
		{
			std::unique_ptr<AudioEventTypeClass> type(new AudioEventTypeClass());
			std::strncpy(type->Name, name, sizeof(type->Name) - 1);
			type->SoundCount = count;
			for (unsigned i = 0; i < count; i++) {
				std::snprintf(type->Sounds[i], sizeof(type->Sounds[i]), "%s%u", name, i);
				Provider.Add(name, i, value);
			}
			type->Control = control;
			type->AttackCount = attack;
			type->DecayCount = decay;
			type->Limit = 0;
			type->Priority = 50;
			Types.push_back(std::move(type));
			return(*Types.back());
		}

		AudioSampleClass * Clip(char const * type, unsigned sound) const { return(Provider.Clip(type, sound)); }

		// Renders the mixer for `ms` and then services the pool at the new time.
		void Tick(unsigned ms)
		{
			Run(ms * (RATE / 1000));
			Now += ms;
			Pool.Service(Now);
		}

		void Run(unsigned frames)
		{
			while (frames > 0) {
				unsigned block = frames < 480 ? frames : 480;
				float buffer[480 * 2];
				Mixer.Render(buffer, block);
				Output.insert(Output.end(), buffer, buffer + block * 2);
				frames -= block;
			}
		}

		float Last_Sample(void) const { return(Output.empty() ? 0.0f : Output[Output.size() - 2]); }

		bool Ready;
		ProviderClass Provider;
		AudioMixerClass Mixer;
		AudioEventPoolClass Pool;
		unsigned Now;
		int RandomHigh;
		std::vector<float> Output;
		std::vector<std::unique_ptr<AudioEventTypeClass>> Types;
};


bool Sequence_Is(AudioSequenceClass const & sequence, std::vector<AudioSampleClass const *> const & clips, unsigned loopstart, unsigned loopend, int cycles)
{
	bool same = sequence.Count == clips.size() && sequence.LoopStart == loopstart && sequence.LoopEnd == loopend && sequence.Cycles == cycles;
	for (unsigned i = 0; same && i < clips.size(); i++) {
		same = sequence.Clips[i] == clips[i];
	}
	if (!same) {
		std::printf("  sequence: count %u loop %u..%u cycles %d, expected count %u loop %u..%u cycles %d\n",
			sequence.Count, sequence.LoopStart, sequence.LoopEnd, sequence.Cycles, (unsigned)clips.size(), loopstart, loopend, cycles);
	}
	return(same);
}


void Test_One_Shot(void)
{
	RigClass rig;
	Check(rig.Ready, "rig ready");

	AudioEventTypeClass & type = rig.Type("SHOT", 1);
	AudioHandle handle = rig.Pool.Start(type, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	Check(!handle.Is_Null(), "one-shot starts");
	Check(rig.Pool.Is_Valid(handle), "handle valid");
	Check(rig.Pool.Is_Playing(handle), "playing before any render");
	Check(rig.Pool.State(handle) == AUDIO_EVENT_PLAYING, "state playing");
	Check(rig.Pool.Type(handle) == &type, "type reported");
	Check(rig.Pool.Live_Count() == 1 && rig.Pool.Live_Count(type) == 1 && type.LiveCount == 1, "live counts");
	Check(rig.Pool.Effects_In_Use() == 1, "one effect in use");

	AudioSequenceClass sequence;
	Check(rig.Pool.Get_Sequence(handle, sequence), "sequence readable");
	Check(Sequence_Is(sequence, { rig.Clip("SHOT", 0) }, 0, 1, 1), "one-shot sequence");
	Check(rig.Provider.Acquired == 1, "one clip pinned");

	rig.Tick(50);
	Check(rig.Pool.Is_Playing(handle), "still playing mid-clip");
	rig.Tick(100);
	Check(!rig.Pool.Is_Valid(handle), "gone after the clip");
	Check(rig.Pool.Live_Count() == 0 && type.LiveCount == 0, "counts back to zero");
	Check(rig.Pool.Effects_In_Use() == 0, "no effect in use");
	Check(rig.Provider.Balanced(), "pins released");
	Check(rig.Mixer.Voice_State(0) == AudioVoiceState::FREE, "voice freed");

	AudioHandle none = rig.Pool.Start(rig.Type("EMPTY", 0), AUDIO_GROUP_SFX, 1.0f, 0.0f);
	Check(none.Is_Null(), "a type without sounds refuses");
}


void Test_Attack_Body_Decay(void)
{
	RigClass rig;
	unsigned flags = SOUND_CONTROL_ATTACK | SOUND_CONTROL_DECAY;

	AudioEventTypeClass & plain = rig.Type("ABD", 5, flags, 1, 1);
	AudioHandle handle = rig.Pool.Start(plain, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	AudioSequenceClass sequence;
	rig.Pool.Get_Sequence(handle, sequence);
	Check(Sequence_Is(sequence, { rig.Clip("ABD", 0), rig.Clip("ABD", 1), rig.Clip("ABD", 4) }, 1, 2, 1), "attack, first body, decay");
	Check(plain.Body_Start() == 1 && plain.Body_Count() == 3, "body range");

	rig.RandomHigh = 1;
	AudioEventTypeClass & random = rig.Type("RND", 5, flags | SOUND_CONTROL_RANDOM, 1, 1);
	handle = rig.Pool.Start(random, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Pool.Get_Sequence(handle, sequence);
	Check(Sequence_Is(sequence, { rig.Clip("RND", 0), rig.Clip("RND", 3), rig.Clip("RND", 4) }, 1, 2, 1), "random picks the last body sound");
	rig.RandomHigh = 0;

	AudioEventTypeClass & all = rig.Type("ALL", 5, flags | SOUND_CONTROL_ALL, 1, 1);
	handle = rig.Pool.Start(all, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Pool.Get_Sequence(handle, sequence);
	Check(Sequence_Is(sequence, { rig.Clip("ALL", 0), rig.Clip("ALL", 1), rig.Clip("ALL", 2), rig.Clip("ALL", 3), rig.Clip("ALL", 4) }, 1, 4, 1), "all plays the whole body in order");

	AudioEventTypeClass & sequential = rig.Type("SEQ", 3, SOUND_CONTROL_SEQUENTIAL);
	for (unsigned i = 0; i < 4; i++) {
		handle = rig.Pool.Start(sequential, AUDIO_GROUP_SFX, 1.0f, 0.0f);
		rig.Pool.Get_Sequence(handle, sequence);
		Check(Sequence_Is(sequence, { rig.Clip("SEQ", i % 3) }, 0, 1, 1), "sequential steps through the body");
	}

	AudioEventTypeClass & noattack = rig.Type("NOATK", 5, flags, 1, 1);
	handle = rig.Pool.Start(noattack, AUDIO_GROUP_SFX, 1.0f, 0.0f, true);
	rig.Pool.Get_Sequence(handle, sequence);
	Check(Sequence_Is(sequence, { rig.Clip("NOATK", 1), rig.Clip("NOATK", 4) }, 0, 1, 1), "no-attack start skips the attack");

	// Too few sounds to spare an attack and a decay: the whole list is the body.
	AudioEventTypeClass & tight = rig.Type("TIGHT", 2, flags, 1, 1);
	handle = rig.Pool.Start(tight, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Pool.Get_Sequence(handle, sequence);
	Check(Sequence_Is(sequence, { rig.Clip("TIGHT", 0) }, 0, 1, 1), "two sounds with attack and decay play as a body");
}


void Test_Loops(void)
{
	RigClass rig;
	AudioSequenceClass sequence;

	AudioEventTypeClass & twice = rig.Type("TWICE", 1, SOUND_CONTROL_LOOP);
	twice.Loop = 2;
	AudioHandle handle = rig.Pool.Start(twice, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Pool.Get_Sequence(handle, sequence);
	Check(sequence.Cycles == 2, "loop count becomes cycles");
	rig.Tick(150);
	Check(rig.Pool.Is_Playing(handle), "second cycle still playing");
	rig.Tick(100);
	Check(!rig.Pool.Is_Valid(handle), "two cycles then done");

	AudioEventTypeClass & forever = rig.Type("EVER", 1, SOUND_CONTROL_LOOP);
	handle = rig.Pool.Start(forever, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Pool.Get_Sequence(handle, sequence);
	Check(sequence.Cycles == -1 && forever.Never_Ends(), "endless loop");
	for (int i = 0; i < 10; i++) {
		rig.Tick(100);
	}
	Check(rig.Pool.Is_Playing(handle), "endless loop keeps playing");
	rig.Pool.End_Looping(handle);
	rig.Tick(30);
	Check(rig.Pool.Is_Playing(handle), "cycle finishes after end-looping");
	rig.Tick(200);
	Check(!rig.Pool.Is_Valid(handle), "done after the cycle");

	AudioEventTypeClass & oneshot = rig.Type("ONCE", 1);
	handle = rig.Pool.Start(oneshot, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Pool.End_Looping(handle);
	Check(rig.Pool.Is_Playing(handle), "end-looping is a no-op on a one-shot");
	Check(rig.Provider.Acquired == rig.Provider.Released + 1, "only the live event holds a pin");
}


void Test_Stop_And_End(void)
{
	RigClass rig;
	unsigned flags = SOUND_CONTROL_ATTACK | SOUND_CONTROL_DECAY | SOUND_CONTROL_LOOP;

	AudioEventTypeClass & type = rig.Type("SE", 3, flags, 1, 1);
	AudioHandle stop = rig.Pool.Start(type, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Tick(150);
	rig.Pool.Stop(stop);
	Check(!rig.Pool.Is_Playing(stop) && !rig.Pool.Is_Valid(stop), "stop is immediate to the caller");
	Check(rig.Pool.Live_Count() == 0, "stopped event is not live");
	rig.Tick(20);
	Check(rig.Mixer.Voice_State(0) == AudioVoiceState::FREE, "stopped voice freed within the ramp");
	Check(rig.Provider.Balanced(), "stop releases the pins");

	AudioHandle end = rig.Pool.Start(type, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Tick(150);
	rig.Pool.End(end);
	Check(rig.Pool.Is_Playing(end), "end keeps the event alive for the decay");
	rig.Tick(100);
	Check(rig.Pool.Is_Playing(end), "still fading");
	rig.Tick(300);
	Check(!rig.Pool.Is_Valid(end), "decay played after the fade");
	Check(rig.Provider.Balanced(), "end releases the pins");

	AudioHandle fade = rig.Pool.Start(type, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Pool.Fade(fade, 500);
	Check(!rig.Pool.Is_Valid(fade), "fade ends the event for the caller");
	rig.Tick(200);
	Check(rig.Mixer.Voice_State(0) != AudioVoiceState::FREE, "fading voice still busy");
	rig.Tick(400);
	Check(rig.Mixer.Voice_State(0) == AudioVoiceState::FREE, "fade done");

	AudioHandle unknown = AudioHandle::Make(5, 77);
	rig.Pool.Stop(unknown);
	rig.Pool.End(unknown);
	rig.Pool.Retarget(unknown, 0.5f, 0.0f);
	Check(!rig.Pool.Is_Valid(unknown) && rig.Pool.State(unknown) == AUDIO_EVENT_DONE, "operations on a stale handle are no-ops");
}


void Test_Delays(void)
{
	RigClass rig;
	AudioSequenceClass sequence;

	AudioEventTypeClass & pre = rig.Type("PRE", 1, SOUND_CONTROL_PREDELAY);
	pre.DelayMin = 100;
	pre.DelayMax = 100;
	AudioHandle handle = rig.Pool.Start(pre, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	Check(rig.Pool.Is_Playing(handle) && rig.Pool.State(handle) == AUDIO_EVENT_PENDING, "predelay waits");
	Check(rig.Pool.Effects_In_Use() == 0, "a pending event holds no voice");
	rig.Tick(50);
	Check(rig.Pool.State(handle) == AUDIO_EVENT_PENDING, "still waiting at 50 ms");
	rig.Tick(50);
	Check(rig.Pool.State(handle) == AUDIO_EVENT_PLAYING, "starts at 100 ms");

	AudioEventTypeClass & cycles = rig.Type("CYC", 3, SOUND_CONTROL_LOOP | SOUND_CONTROL_ATTACK | SOUND_CONTROL_DECAY, 1, 1);
	cycles.Loop = 3;
	cycles.DelayMin = 100;
	cycles.DelayMax = 100;
	handle = rig.Pool.Start(cycles, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Pool.Get_Sequence(handle, sequence);
	Check(Sequence_Is(sequence, { rig.Clip("CYC", 0), rig.Clip("CYC", 1) }, 1, 2, 1), "first delayed cycle: attack and body");
	rig.Tick(210);
	Check(rig.Pool.State(handle) == AUDIO_EVENT_GAP, "gap after the first cycle");
	rig.Tick(50);
	Check(rig.Pool.State(handle) == AUDIO_EVENT_GAP, "gap lasts the delay");
	rig.Tick(50);
	Check(rig.Pool.State(handle) == AUDIO_EVENT_PLAYING, "second cycle starts after the delay");
	rig.Pool.Get_Sequence(handle, sequence);
	Check(Sequence_Is(sequence, { rig.Clip("CYC", 1) }, 0, 1, 1), "middle cycle: body only");
	rig.Tick(110);
	Check(rig.Pool.State(handle) == AUDIO_EVENT_GAP, "second gap");
	rig.Tick(100);
	rig.Pool.Get_Sequence(handle, sequence);
	Check(Sequence_Is(sequence, { rig.Clip("CYC", 1), rig.Clip("CYC", 4 - 2) }, 0, 1, 1), "last cycle: body and decay");
	rig.Tick(210);
	Check(!rig.Pool.Is_Valid(handle), "delayed loop done after three cycles");
	Check(rig.Provider.Balanced(), "delayed loop released every pin");

	AudioEventTypeClass & ended = rig.Type("ENDGAP", 3, SOUND_CONTROL_LOOP | SOUND_CONTROL_ATTACK | SOUND_CONTROL_DECAY, 1, 1);
	ended.DelayMin = 500;
	ended.DelayMax = 500;
	handle = rig.Pool.Start(ended, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Tick(210);
	Check(rig.Pool.State(handle) == AUDIO_EVENT_GAP, "endless delayed loop rests in the gap");
	rig.Pool.End(handle);
	rig.Tick(10);
	Check(rig.Pool.State(handle) == AUDIO_EVENT_PLAYING, "end during the gap plays the decay at once");
	rig.Pool.Get_Sequence(handle, sequence);
	Check(Sequence_Is(sequence, { rig.Clip("ENDGAP", 2) }, 0, 0, 1), "decay alone");
	rig.Tick(110);
	Check(!rig.Pool.Is_Valid(handle), "decay finished the event");

	handle = rig.Pool.Start(ended, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Tick(150);
	rig.Pool.End(handle);
	rig.Tick(300);
	Check(rig.Pool.State(handle) == AUDIO_EVENT_PLAYING, "end mid-cycle of a delayed loop still owes the decay");
	rig.Pool.Get_Sequence(handle, sequence);
	Check(Sequence_Is(sequence, { rig.Clip("ENDGAP", 2) }, 0, 0, 1), "decay after the fade");
	rig.Tick(110);
	Check(!rig.Pool.Is_Valid(handle) && rig.Provider.Balanced(), "done and balanced");

	AudioEventTypeClass & stopped = rig.Type("STOPGAP", 1, SOUND_CONTROL_LOOP);
	stopped.DelayMin = 500;
	stopped.DelayMax = 500;
	handle = rig.Pool.Start(stopped, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Tick(110);
	Check(rig.Pool.State(handle) == AUDIO_EVENT_GAP, "in the gap");
	rig.Pool.Stop(handle);
	Check(!rig.Pool.Is_Valid(handle) && rig.Pool.Live_Count() == 0, "stop during the gap ends at once");
}


void Test_Clock_Clamp(void)
{
	RigClass rig;
	AudioEventTypeClass & pre = rig.Type("CLAMP", 1, SOUND_CONTROL_PREDELAY);
	pre.DelayMin = 1000;
	pre.DelayMax = 1000;
	AudioHandle handle = rig.Pool.Start(pre, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Now += 5000;
	rig.Pool.Service(rig.Now);
	Check(rig.Pool.State(handle) == AUDIO_EVENT_PENDING, "a five second stall does not fire the delay");
	rig.Tick(250);
	rig.Tick(250);
	Check(rig.Pool.State(handle) == AUDIO_EVENT_PENDING, "delay still running after the clamped ticks");
	rig.Tick(250);
	Check(rig.Pool.State(handle) == AUDIO_EVENT_PLAYING, "delay fires once the clamped clock catches up");
}


void Test_Limit(void)
{
	RigClass rig;

	AudioEventTypeClass & type = rig.Type("LIM", 1);
	type.Limit = 2;
	AudioHandle loud = rig.Pool.Start(type, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	AudioHandle quiet = rig.Pool.Start(type, AUDIO_GROUP_SFX, 0.5f, 0.0f);
	AudioHandle mid = rig.Pool.Start(type, AUDIO_GROUP_SFX, 0.8f, 0.0f);
	Check(!mid.Is_Null(), "third start admitted");
	Check(!rig.Pool.Is_Playing(quiet), "quietest evicted");
	Check(rig.Pool.Is_Playing(loud) && rig.Pool.Is_Playing(mid), "louder two remain");
	Check(rig.Pool.Live_Count(type) == 2 && type.LiveCount == 2, "limit holds");
	AudioHandle softer = rig.Pool.Start(type, AUDIO_GROUP_SFX, 0.3f, 0.0f);
	Check(softer.Is_Null(), "a quieter newcomer is refused");
	Check(rig.Pool.Live_Count(type) == 2 && type.LiveCount == 2, "refusal leaves the count");

	AudioEventTypeClass & single = rig.Type("ONE", 1);
	single.Limit = 1;
	AudioHandle first = rig.Pool.Start(single, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	AudioHandle second = rig.Pool.Start(single, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	Check(second.Is_Null() && rig.Pool.Is_Playing(first), "equal volume: newcomer refused");

	AudioEventTypeClass & interrupt = rig.Type("INT", 1, SOUND_CONTROL_INTERRUPT);
	interrupt.Limit = 1;
	first = rig.Pool.Start(interrupt, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	second = rig.Pool.Start(interrupt, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	Check(!second.Is_Null() && !rig.Pool.Is_Playing(first) && rig.Pool.Is_Playing(second), "interrupt: oldest evicted on a tie");

	AudioEventTypeClass & unlimited = rig.Type("ANY", 1);
	unlimited.Limit = 0;
	for (int i = 0; i < 6; i++) {
		Check(!rig.Pool.Start(unlimited, AUDIO_GROUP_SFX, 1.0f, 0.0f).Is_Null(), "unlimited type admits");
	}

	AudioEventTypeClass & queued = rig.Type("QUE", 1, SOUND_CONTROL_QUEUE);
	queued.Limit = 1;
	first = rig.Pool.Start(queued, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	second = rig.Pool.Start(queued, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	Check(!second.Is_Null() && rig.Pool.State(second) == AUDIO_EVENT_PENDING, "queue parks the newcomer");
	rig.Tick(50);
	Check(rig.Pool.State(second) == AUDIO_EVENT_PENDING && rig.Pool.Is_Playing(first), "parked while the first plays");
	rig.Tick(60);
	Check(!rig.Pool.Is_Valid(first) && rig.Pool.State(second) == AUDIO_EVENT_PLAYING, "queued event starts when the slot frees");
	AudioHandle third = rig.Pool.Start(queued, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Pool.Stop(second);
	for (int i = 0; i < 25; i++) {
		rig.Tick(100);
	}
	Check(!rig.Pool.Is_Valid(third), "a queued event that waited out its turn was dropped or finished");
}


void Test_Budget(void)
{
	RigClass rig;
	rig.Pool.Set_Budget(2);
	Check(rig.Pool.Budget() == 2, "budget set");

	AudioEventTypeClass & low = rig.Type("LOW", 1);
	low.Priority = 10;
	AudioEventTypeClass & high = rig.Type("HIGH", 1);
	high.Priority = 50;
	AudioEventTypeClass & lowest = rig.Type("LOWEST", 1);
	lowest.Priority = 5;

	AudioHandle a = rig.Pool.Start(low, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	AudioHandle b = rig.Pool.Start(low, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	Check(rig.Pool.Effects_In_Use() == 2, "budget full");
	AudioHandle c = rig.Pool.Start(high, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	Check(!c.Is_Null(), "higher priority steals");
	Check(!rig.Pool.Is_Playing(a) && rig.Pool.Is_Playing(b), "the older of two equals is stolen");
	Check(rig.Pool.Effects_In_Use() == 2, "stolen voice leaves the budget at once");

	AudioHandle d = rig.Pool.Start(lowest, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	Check(d.Is_Null(), "lower priority is refused when the budget is full");

	AudioHandle e = rig.Pool.Start(high, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	Check(!e.Is_Null() && !rig.Pool.Is_Playing(b) && rig.Pool.Is_Playing(c), "lowest priority is the victim, not the oldest");

	AudioHandle f = rig.Pool.Start(high, AUDIO_GROUP_SFX, 0.5f, 0.0f);
	Check(f.Is_Null(), "same priority, quieter: refused");
	AudioHandle g = rig.Pool.Start(high, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	Check(g.Is_Null(), "same priority, same volume: refused");

	rig.Pool.Retarget(c, 0.5f, 0.0f);
	AudioHandle h = rig.Pool.Start(high, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	Check(!h.Is_Null() && !rig.Pool.Is_Playing(c) && rig.Pool.Is_Playing(e), "same priority: the one quieter by ten percent is stolen");

	AudioStreamClass stream;
	stream.Init(4800, 1, RATE);
	AudioHandle music = rig.Pool.Start_Stream(&stream, AUDIO_GROUP_MUSIC, 1.0f, 0.0f);
	Check(!music.Is_Null() && rig.Pool.Effects_In_Use() == 2, "a stream is outside the budget");
	AudioHandle speech = rig.Pool.Start_Stream(&stream, AUDIO_GROUP_SPEECH, 1.0f, 0.0f);
	Check(!speech.Is_Null(), "streams start while the effects budget is full");
	rig.Pool.Stop(music);
	rig.Pool.Stop(speech);

	rig.Tick(20);
	Check(rig.Pool.Effects_In_Use() == 2 && rig.Pool.Live_Count() == 2, "after the ramps only the two winners remain");
	rig.Tick(200);
	Check(rig.Pool.Live_Count() == 0 && rig.Provider.Balanced(), "all done and balanced");
}


void Test_Levels(void)
{
	RigClass rig;

	AudioEventTypeClass & type = rig.Type("LVL", 1, SOUND_CONTROL_LOOP, 0, 0, 0.5f);
	AudioHandle handle = rig.Pool.Start(type, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Tick(20);
	Check(Near(rig.Last_Sample(), 0.5f, 0.01f), "full level passes the clip through");

	rig.Pool.Set_Volume(handle, 0.5f);
	rig.Tick(100);
	Check(Near(rig.Last_Sample(), 0.5f * Audio_Perceptual_Gain(0.5f), 0.01f), "set volume goes through the loudness curve");

	rig.Pool.Retarget(handle, 1.0f, -1.0f);
	rig.Tick(100);
	Check(Near(rig.Output[rig.Output.size() - 2], 0.5f, 0.01f) && Near(rig.Output[rig.Output.size() - 1], 0.0f, 0.01f), "hard left pan");
	rig.Pool.Stop(handle);
	rig.Tick(20);

	AudioEventTypeClass & half = rig.Type("HALF", 1, SOUND_CONTROL_LOOP, 0, 0, 0.5f);
	half.Volume = 0.5f;
	handle = rig.Pool.Start(half, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Tick(20);
	Check(Near(rig.Last_Sample(), 0.5f * Audio_Perceptual_Gain(0.5f), 0.01f), "type volume scales the level");
	rig.Pool.Stop(handle);
	rig.Tick(20);

	AudioEventTypeClass & shifted = rig.Type("VSH", 1, SOUND_CONTROL_LOOP, 0, 0, 0.5f);
	shifted.VShiftMin = -50;
	shifted.VShiftMax = -50;
	handle = rig.Pool.Start(shifted, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Tick(20);
	Check(Near(rig.Last_Sample(), 0.5f * Audio_Perceptual_Gain(0.5f), 0.01f), "volume shift attenuates once per event");
	rig.Pool.Stop(handle);
	rig.Tick(20);

	AudioEventTypeClass & pitched = rig.Type("FSH", 1, SOUND_CONTROL_NONE, 0, 0, 0.0f);
	pitched.FShiftMin = 100;
	pitched.FShiftMax = 100;
	handle = rig.Pool.Start(pitched, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Tick(60);
	Check(!rig.Pool.Is_Valid(handle), "double pitch halves the play time");

	AudioEventTypeClass & paused = rig.Type("PAUSE", 1, SOUND_CONTROL_NONE);
	handle = rig.Pool.Start(paused, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Pool.Pause(handle);
	rig.Tick(300);
	Check(rig.Pool.Is_Playing(handle), "paused event does not advance");
	rig.Pool.Resume(handle);
	rig.Tick(120);
	Check(!rig.Pool.Is_Valid(handle), "resumed event finishes");
}


void Test_Samples_And_Handles(void)
{
	RigClass rig;

	AudioSampleClass * clip = rig.Provider.Add("RAW", 0, 0.25f);
	int tag = 0;
	AudioHandle raw = rig.Pool.Start_Sample(rig.Provider.Pin(clip), AUDIO_GROUP_SYSTEM, 1.0f, 255, &tag);
	Check(!raw.Is_Null() && rig.Pool.Type(raw) == nullptr, "raw play has no type");
	Check(rig.Pool.Is_Tag_Playing(&tag), "tag found");
	Check(rig.Pool.Effects_In_Use() == 1, "system group counts toward the budget");
	rig.Tick(20);
	Check(Near(rig.Last_Sample(), 0.25f, 0.01f), "raw clip plays at its level");
	rig.Pool.Stop_Tag(&tag);
	Check(!rig.Pool.Is_Tag_Playing(&tag) && !rig.Pool.Is_Valid(raw), "stop by tag");
	rig.Tick(20);
	Check(rig.Provider.Balanced(), "raw pin returned");

	AudioEventTypeClass & type = rig.Type("REUSE", 1);
	AudioHandle first = rig.Pool.Start(type, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	unsigned index = first.Index();
	rig.Tick(120);
	Check(!rig.Pool.Is_Valid(first), "first event done");
	AudioHandle again;
	for (int i = 0; i < AUDIO_MAX_EVENTS && (again.Is_Null() || again.Index() != index); i++) {
		again = rig.Pool.Start(type, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	}
	Check(again.Index() == index && again.Generation() != first.Generation(), "slot reused with a new generation");
	Check(!rig.Pool.Is_Valid(first) && rig.Pool.Is_Valid(again), "stale handle stays invalid after reuse");

	rig.Pool.Stop_Group(AUDIO_GROUP_SFX, 0);
	Check(rig.Pool.Live_Count() == 0, "stop group empties the group");
	rig.Tick(20);

	AudioEventTypeClass & missing = rig.Type("GAP", 3);
	rig.Provider.Add("GAP", 0)->Frames = 0;
	AudioHandle skipped = rig.Pool.Start(missing, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	AudioSequenceClass sequence;
	rig.Pool.Get_Sequence(skipped, sequence);
	Check(sequence.Count == 1 && sequence.Clips[0] == rig.Clip("GAP", 0), "an unplayable clip is still handed to the mixer, which skips it");
	rig.Tick(20);
	Check(!rig.Pool.Is_Valid(skipped) && rig.Provider.Balanced(), "the mixer refused it and the pool cleaned up");

	AudioEventTypeClass & none = rig.Type("NONE", 2);
	std::strcpy(none.Name, "OTHER");
	AudioHandle refused = rig.Pool.Start(none, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	Check(refused.Is_Null() && none.LiveCount == 0, "a type whose clips are all missing refuses cleanly");

	AudioEventTypeClass & live = rig.Type("LIVE", 1, SOUND_CONTROL_LOOP);
	rig.Pool.Start(live, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Pool.Start(live, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	Check(rig.Pool.Live_Count() == 2, "two loops live");
	rig.Pool.Stop_All();
	Check(rig.Pool.Live_Count() == 0 && live.LiveCount == 0, "stop all");
	rig.Tick(20);
	Check(rig.Provider.Balanced(), "everything released");
}


// A start between two service calls must not take the voice of an event whose
// clip has ended but which has not been serviced yet.
void Test_Voice_Ownership(void)
{
	RigClass rig;
	AudioEventTypeClass & type = rig.Type("OWN", 1);
	AudioHandle first = rig.Pool.Start(type, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Run(120 * (RATE / 1000));
	Check(rig.Mixer.Voice_State(0) == AudioVoiceState::DONE, "first voice done but not yet serviced");
	Check(rig.Pool.Is_Playing(first), "first event not yet reaped");

	AudioHandle second = rig.Pool.Start(type, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	Check(!second.Is_Null(), "second start admitted");
	Check(!rig.Pool.Is_Valid(first), "starting another event reaps the finished one");
	rig.Tick(50);
	Check(rig.Pool.Is_Playing(second), "second event keeps its voice through the next service");
	Check(rig.Pool.Effects_In_Use() == 1 && rig.Pool.Live_Count() == 1, "one live event");
	rig.Tick(100);
	Check(!rig.Pool.Is_Valid(second) && rig.Provider.Balanced(), "second event finishes on its own");

	// The same with the finished voice belonging to an event of another type.
	AudioEventTypeClass & other = rig.Type("OWN2", 1, SOUND_CONTROL_LOOP);
	first = rig.Pool.Start(type, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	rig.Run(120 * (RATE / 1000));
	second = rig.Pool.Start(other, AUDIO_GROUP_SFX, 1.0f, 0.0f);
	for (int i = 0; i < 10; i++) {
		rig.Tick(100);
	}
	Check(rig.Pool.Is_Playing(second) && rig.Pool.Effects_In_Use() == 1, "a loop started over a finished voice keeps playing");
}


void Test_Shutdown_Releases(void)
{
	ProviderClass provider;
	AudioMixerClass mixer;
	AudioEventTypeClass type;
	std::strcpy(type.Name, "SD");
	type.SoundCount = 1;
	type.Control = SOUND_CONTROL_LOOP;
	provider.Add("SD", 0);
	{
		AudioEventPoolClass pool;
		Check(mixer.Init(RATE, 2) && pool.Init(&mixer, &provider), "pool ready");
		Check(!pool.Init(&mixer, &provider), "second init refused");
		AudioHandle handle = pool.Start(type, AUDIO_GROUP_SFX, 1.0f, 0.0f);
		Check(pool.Is_Playing(handle), "playing before shutdown");
		pool.Shutdown();
		Check(provider.Balanced() && type.LiveCount == 0, "shutdown releases pins and counts");
		Check(!pool.Is_Valid(handle), "handles die with the pool");
	}
	mixer.Shutdown();
}

} // namespace


int main(void)
{
	Test_One_Shot();
	Test_Attack_Body_Decay();
	Test_Loops();
	Test_Stop_And_End();
	Test_Delays();
	Test_Clock_Clamp();
	Test_Limit();
	Test_Budget();
	Test_Levels();
	Test_Samples_And_Handles();
	Test_Voice_Ownership();
	Test_Shutdown_Releases();

	std::printf("%d checks, %d failures\n", Checked, Failures);
	return(Failures == 0 ? 0 : 1);
}
