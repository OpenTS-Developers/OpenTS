/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "audio/audioevent.h"

#include "audio/audiomixer.h"
#include "audio/audiosample.h"
#include "audio/audiostream.h"

#include <cstring>

namespace {

// A queued request waits this long for a voice before it is dropped.
unsigned const QUEUE_WAIT_MS = 2000;

// The longest jump the service clock takes in one step, so a stall does not
// expire every pending delay at once.
unsigned const MAX_TICK_MS = 250;

// Within this fraction of the quietest candidate, priority order decides alone.
float const VOLUME_SLACK = 0.1f;

int const DEFAULT_BUDGET = 16;

} // namespace


AudioEventTypeClass::AudioEventTypeClass(void) :
	SoundCount(0),
	Priority(10),
	Volume(1.0f),
	MinVolume(0.0f),
	Range(28),
	Limit(3),
	Loop(0),
	DelayMin(0),
	DelayMax(0),
	FShiftMin(0),
	FShiftMax(0),
	VShiftMin(0),
	VShiftMax(0),
	AttackCount(0),
	DecayCount(0),
	Type(SOUND_TYPE_SCREEN),
	Control(SOUND_CONTROL_NONE),
	Group(AUDIO_GROUP_SFX),
	LiveCount(0),
	SequentialIndex(0)
{
	Name[0] = '\0';
	std::memset(Sounds, 0, sizeof(Sounds));
}


unsigned AudioEventTypeClass::Body_Start(void) const
{
	unsigned attack = (AttackCount > 0) ? (unsigned)AttackCount : 0;
	unsigned decay = (DecayCount > 0) ? (unsigned)DecayCount : 0;
	return(attack + decay < SoundCount ? attack : 0);
}


unsigned AudioEventTypeClass::Body_Count(void) const
{
	unsigned attack = (AttackCount > 0) ? (unsigned)AttackCount : 0;
	unsigned decay = (DecayCount > 0) ? (unsigned)DecayCount : 0;
	if (attack + decay >= SoundCount) {
		// Not enough sounds to spare any: the whole list is the body.
		return(SoundCount);
	}
	return(SoundCount - attack - decay);
}


struct AudioEventPoolClass::EventClass {
	AudioEventState State;
	unsigned Generation;
	AudioEventTypeClass const * Type;
	AudioGroupType Group;
	AudioStreamClass * Stream;
	int Priority;
	float BaseVolume;
	float RequestVolume;
	float VShiftFactor;
	float Pitch;
	float Pan;
	int Voice;
	uint32_t VoiceGeneration;
	AudioSequenceClass Sequence;
	AudioSampleClass * Pins[AUDIO_MAX_SEQUENCE];
	unsigned PinCount;
	int CyclesLeft;               // for delayed loops; -1 until ended
	int BodyPick;                 // the body sound chosen at the first cycle; -1 before
	bool DelayedLoop;
	bool AttackPlayed;
	bool DecayIssued;
	bool EndRequested;
	bool NoAttack;
	bool Stolen;
	bool Queued;
	bool Counted;                 // holds one of its type's LiveCount
	unsigned NextStart;
	unsigned QueuedSince;
	void const * Tag;
	unsigned Order;

	bool Is_Live(void) const { return(State == AUDIO_EVENT_PENDING || State == AUDIO_EVENT_PLAYING || State == AUDIO_EVENT_GAP); }
	bool Counts_Toward_Budget(void) const { return(State == AUDIO_EVENT_PLAYING && Voice >= 0 && !Stolen && Stream == nullptr && (Group == AUDIO_GROUP_SFX || Group == AUDIO_GROUP_SYSTEM)); }
};


AudioEventPoolClass::AudioEventPoolClass(void) :
	Events(nullptr),
	Mixer(nullptr),
	Clips(nullptr),
	RandomProc(nullptr),
	RandomContext(nullptr),
	Seed(0x1234567u),
	BudgetValue(DEFAULT_BUDGET),
	LastNow(0),
	StartOrder(0),
	Ready(false)
{
	std::memset(VoiceGenerations, 0, sizeof(VoiceGenerations));
}


AudioEventPoolClass::~AudioEventPoolClass(void)
{
	Shutdown();
}


bool AudioEventPoolClass::Init(AudioMixerClass * mixer, AudioClipProviderClass * clips)
{
	if (Ready || mixer == nullptr || clips == nullptr) {
		return(false);
	}
	Events = new (std::nothrow) EventClass[AUDIO_MAX_EVENTS];
	if (Events == nullptr) {
		return(false);
	}
	for (int i = 0; i < AUDIO_MAX_EVENTS; i++) {
		std::memset(&Events[i], 0, sizeof(EventClass));
		Events[i].State = AUDIO_EVENT_FREE;
		Events[i].Generation = 1;
		Events[i].Voice = -1;
	}
	for (int i = 0; i < AUDIO_MAX_VOICES; i++) {
		VoiceGenerations[i] = 1;
	}
	Mixer = mixer;
	Clips = clips;
	Ready = true;
	return(true);
}


void AudioEventPoolClass::Shutdown(void)
{
	if (!Ready) {
		return;
	}
	for (int i = 0; i < AUDIO_MAX_EVENTS; i++) {
		EventClass & event = Events[i];
		if (event.Is_Live()) {
			Kill(event, 0.0f);
			Finish(event);
		}
	}
	delete[] Events;
	Events = nullptr;
	Ready = false;
}


void AudioEventPoolClass::Set_Random(AudioRandomProc proc, void * context)
{
	RandomProc = proc;
	RandomContext = context;
}


void AudioEventPoolClass::Set_Budget(int channels)
{
	if (channels < 1) {
		channels = 1;
	}
	if (channels > AUDIO_MAX_VOICES) {
		channels = AUDIO_MAX_VOICES;
	}
	BudgetValue = channels;
}


int AudioEventPoolClass::Random(int low, int high)
{
	if (high <= low) {
		return(low);
	}
	if (RandomProc != nullptr) {
		return(RandomProc(low, high, RandomContext));
	}
	Seed = Seed * 1103515245u + 12345u;
	return(low + (int)((Seed >> 8) % (unsigned)(high - low + 1)));
}


AudioEventPoolClass::EventClass * AudioEventPoolClass::Lookup(AudioHandle handle)
{
	if (!Ready || handle.Is_Null() || handle.Index() >= AUDIO_MAX_EVENTS) {
		return(nullptr);
	}
	EventClass & event = Events[handle.Index()];
	if (event.Generation != handle.Generation() || !event.Is_Live() || event.Stolen) {
		return(nullptr);
	}
	return(&event);
}


AudioEventPoolClass::EventClass const * AudioEventPoolClass::Lookup(AudioHandle handle) const
{
	return(const_cast<AudioEventPoolClass *>(this)->Lookup(handle));
}


AudioEventPoolClass::EventClass * AudioEventPoolClass::Allocate(void)
{
	// A finished event whose voice is still ramping down keeps its slot until the
	// voice is reaped, unless nothing else is free.
	for (int pass = 0; pass < 2; pass++) {
		for (int i = 0; i < AUDIO_MAX_EVENTS; i++) {
			EventClass & event = Events[i];
			bool free = event.State == AUDIO_EVENT_FREE || (event.State == AUDIO_EVENT_DONE && (event.Voice < 0 || pass == 1));
			if (!free) {
				continue;
			}
			if (event.Voice >= 0 && Mixer->Voice_State((unsigned)event.Voice) == AudioVoiceState::DONE) {
				Mixer->Free_Voice((unsigned)event.Voice);
			}
			unsigned generation = event.Generation;
			std::memset(&event, 0, sizeof(event));
			event.State = AUDIO_EVENT_FREE;
			event.Generation = generation;
			event.Voice = -1;
			event.Pitch = 1.0f;
			event.VShiftFactor = 1.0f;
			event.BaseVolume = 1.0f;
			event.RequestVolume = 1.0f;
			event.CyclesLeft = 1;
			event.BodyPick = -1;
			event.Order = ++StartOrder;
			return(&event);
		}
	}
	return(nullptr);
}


float AudioEventPoolClass::Current_Level(EventClass const & event) const
{
	float level = event.BaseVolume * event.RequestVolume * event.VShiftFactor;
	if (level < 0.0f) level = 0.0f;
	if (level > 1.0f) level = 1.0f;
	return(level);
}


void AudioEventPoolClass::Push_Level(EventClass & event, float ramp)
{
	if (event.Voice < 0) {
		return;
	}
	AudioCommand command;
	std::memset(&command, 0, sizeof(command));
	command.Type = AudioCommandType::SET_GAIN;
	command.Slot = (uint8_t)event.Voice;
	command.Generation = event.VoiceGeneration;
	command.A = Current_Level(event);
	command.B = ramp;
	Mixer->Push(command);
}


void AudioEventPoolClass::Push_Pan(EventClass & event, float ramp)
{
	if (event.Voice < 0) {
		return;
	}
	AudioCommand command;
	std::memset(&command, 0, sizeof(command));
	command.Type = AudioCommandType::SET_PAN;
	command.Slot = (uint8_t)event.Voice;
	command.Generation = event.VoiceGeneration;
	command.A = event.Pan;
	command.B = ramp;
	Mixer->Push(command);
}


AudioSampleClass * AudioEventPoolClass::Pin(EventClass & event, unsigned sound)
{
	if (event.Type == nullptr || sound >= event.Type->SoundCount || event.PinCount >= AUDIO_MAX_SEQUENCE) {
		return(nullptr);
	}
	AudioSampleClass * clip = Clips->Acquire(*event.Type, sound);
	if (clip != nullptr) {
		event.Pins[event.PinCount++] = clip;
	}
	return(clip);
}


void AudioEventPoolClass::Release_Pins(EventClass & event)
{
	for (unsigned i = 0; i < event.PinCount; i++) {
		Clips->Release(event.Pins[i]);
		event.Pins[i] = nullptr;
	}
	event.PinCount = 0;
}


void AudioEventPoolClass::Release_Voice(EventClass & event)
{
	if (event.Voice >= 0) {
		Mixer->Free_Voice((unsigned)event.Voice);
		event.Voice = -1;
	}
}


// Sends the voice a stop and detaches the event from it. The voice ramps down
// on its own and is freed once the mixer reports it done.
void AudioEventPoolClass::Kill(EventClass & event, float fade)
{
	if (event.Voice >= 0) {
		AudioCommand command;
		std::memset(&command, 0, sizeof(command));
		command.Type = AudioCommandType::STOP;
		command.Slot = (uint8_t)event.Voice;
		command.Generation = event.VoiceGeneration;
		command.A = fade;
		Mixer->Push(command);
	}
	event.Stolen = true;
	event.EndRequested = true;
	event.DelayedLoop = false;
	if (event.Counted) {
		event.Type->LiveCount--;
		event.Counted = false;
	}
	if (event.State != AUDIO_EVENT_PLAYING) {
		Finish(event);
	}
}


void AudioEventPoolClass::Finish(EventClass & event)
{
	Release_Pins(event);
	if (event.Counted) {
		event.Type->LiveCount--;
		event.Counted = false;
	}
	// The voice, if still ramping, is reaped by Service once the mixer is done
	// with it; the event itself is over now.
	event.State = AUDIO_EVENT_DONE;
	event.Generation = AudioHandle::Next_Generation(event.Generation);
	event.Tag = nullptr;
}


bool AudioEventPoolClass::Enforce_Limit(EventClass & event)
{
	AudioEventTypeClass const * type = event.Type;
	if (type == nullptr || type->Limit <= 0) {
		return(true);
	}

	unsigned count = 0;
	EventClass * quietest = nullptr;
	for (int i = 0; i < AUDIO_MAX_EVENTS; i++) {
		EventClass & other = Events[i];
		if (&other == &event || !other.Is_Live() || other.Type != type || other.Stolen) {
			continue;
		}
		count++;
		if (quietest == nullptr || Current_Level(other) < Current_Level(*quietest) || (Current_Level(other) == Current_Level(*quietest) && other.Order < quietest->Order)) {
			quietest = &other;
		}
	}
	if (count < (unsigned)type->Limit) {
		return(true);
	}

	float mine = Current_Level(event);
	float theirs = Current_Level(*quietest);
	bool tie = (theirs >= mine * 0.99f && theirs <= mine * 1.01f);
	bool interrupt = (type->Control & SOUND_CONTROL_INTERRUPT) != 0;
	if (mine > theirs && !tie) {
		Kill(*quietest, AUDIO_STOP_RAMP_SECONDS);
		return(true);
	}
	if (tie && interrupt) {
		Kill(*quietest, AUDIO_STOP_RAMP_SECONDS);
		return(true);
	}
	return(false);
}


unsigned AudioEventPoolClass::Effects_In_Use(void) const
{
	unsigned count = 0;
	for (int i = 0; i < AUDIO_MAX_EVENTS; i++) {
		if (Events[i].Counts_Toward_Budget()) {
			count++;
		}
	}
	return(count);
}


// Finds a free voice for the event, stealing one from a weaker effect when the
// budget is spent. Sets event.Voice on success.
bool AudioEventPoolClass::Acquire_Voice(EventClass & event)
{
	bool effect = (event.Stream == nullptr && (event.Group == AUDIO_GROUP_SFX || event.Group == AUDIO_GROUP_SYSTEM));
	if (effect && Effects_In_Use() >= (unsigned)BudgetValue) {
		EventClass * victim = nullptr;
		float mine = Current_Level(event);
		for (int i = 0; i < AUDIO_MAX_EVENTS; i++) {
			EventClass & other = Events[i];
			if (!other.Counts_Toward_Budget()) {
				continue;
			}
			if (victim == nullptr) {
				victim = &other;
				continue;
			}
			if (other.Priority < victim->Priority) {
				victim = &other;
			} else if (other.Priority == victim->Priority) {
				float a = Current_Level(other);
				float b = Current_Level(*victim);
				if (a < b * (1.0f - VOLUME_SLACK)) {
					victim = &other;
				} else if (a <= b * (1.0f + VOLUME_SLACK) && other.Order < victim->Order) {
					victim = &other;
				}
			}
		}
		if (victim == nullptr) {
			return(false);
		}
		bool weaker = victim->Priority < event.Priority || (victim->Priority == event.Priority && Current_Level(*victim) < mine * (1.0f - VOLUME_SLACK));
		if (!weaker) {
			return(false);
		}
		Kill(*victim, AUDIO_STOP_RAMP_SECONDS);
	}

	// A voice the mixer has finished with still belongs to its event until that
	// event has seen it, so only slots nobody owns are taken.
	Reap_Voices(LastNow);
	for (int slot = 0; slot < AUDIO_MAX_VOICES; slot++) {
		if (Mixer->Voice_State((unsigned)slot) == AudioVoiceState::FREE && Mixer->Allocate_Voice((unsigned)slot)) {
			VoiceGenerations[slot] = AudioHandle::Next_Generation(VoiceGenerations[slot]);
			event.Voice = slot;
			event.VoiceGeneration = VoiceGenerations[slot];
			return(true);
		}
	}
	return(false);
}


// Builds the sequence for one cycle group and sends it to the voice. The body
// pick is made once per event, so every cycle of a delayed loop plays the
// same sound.
bool AudioEventPoolClass::Issue(EventClass & event, bool attack, bool body, bool decay)
{
	AudioEventTypeClass const & type = *event.Type;
	AudioSequenceClass & sequence = event.Sequence;
	std::memset(&sequence, 0, sizeof(sequence));

	unsigned attackcount = (type.AttackCount > 0) ? (unsigned)type.AttackCount : 0;
	unsigned decaycount = (type.DecayCount > 0) ? (unsigned)type.DecayCount : 0;
	unsigned bodystart = type.Body_Start();
	unsigned bodycount = type.Body_Count();
	bool spare = (attackcount + decaycount < type.SoundCount);

	if (attack && spare && attackcount > 0) {
		unsigned pick = (unsigned)Random(0, (int)attackcount - 1);
		AudioSampleClass * clip = Pin(event, pick);
		if (clip != nullptr) {
			sequence.Clips[sequence.Count++] = clip;
		}
	}

	sequence.LoopStart = sequence.Count;
	if (body && bodycount > 0) {
		if (type.Control & SOUND_CONTROL_ALL) {
			for (unsigned i = 0; i < bodycount && sequence.Count < AUDIO_MAX_SEQUENCE; i++) {
				AudioSampleClass * clip = Pin(event, bodystart + i);
				if (clip != nullptr) {
					sequence.Clips[sequence.Count++] = clip;
				}
			}
		} else {
			if (event.BodyPick < 0) {
				if (type.Control & SOUND_CONTROL_RANDOM) {
					event.BodyPick = Random(0, (int)bodycount - 1);
				} else if (type.Control & SOUND_CONTROL_SEQUENTIAL) {
					event.BodyPick = (int)(type.SequentialIndex % bodycount);
					type.SequentialIndex++;
				} else {
					event.BodyPick = 0;
				}
			}
			// A missing body sound falls through to the next one so the type still plays.
			for (unsigned tries = 0; tries < bodycount; tries++) {
				AudioSampleClass * clip = Pin(event, bodystart + ((unsigned)event.BodyPick + tries) % bodycount);
				if (clip != nullptr) {
					sequence.Clips[sequence.Count++] = clip;
					break;
				}
			}
		}
	}
	sequence.LoopEnd = sequence.Count;

	if (decay && spare && decaycount > 0 && sequence.Count < AUDIO_MAX_SEQUENCE) {
		unsigned pick = type.SoundCount - decaycount + (unsigned)Random(0, (int)decaycount - 1);
		AudioSampleClass * clip = Pin(event, pick);
		if (clip != nullptr) {
			sequence.Clips[sequence.Count++] = clip;
			event.DecayIssued = true;
		}
	}

	if (sequence.Count == 0) {
		return(false);
	}

	bool loop = (type.Control & SOUND_CONTROL_LOOP) != 0;
	if (event.DelayedLoop || !loop || !body) {
		sequence.Cycles = 1;
	} else {
		sequence.Cycles = (type.Loop > 0) ? type.Loop : -1;
	}

	AudioCommand command;
	std::memset(&command, 0, sizeof(command));
	command.Type = AudioCommandType::PLAY_SEQUENCE;
	command.Group = (uint8_t)event.Group;
	command.Slot = (uint8_t)event.Voice;
	command.Generation = event.VoiceGeneration;
	command.A = Current_Level(event);
	command.B = event.Pan;
	command.C = event.Pitch;
	command.Ptr = &sequence;
	return(Mixer->Push(command));
}


bool AudioEventPoolClass::Try_Start(EventClass & event)
{
	if (!Enforce_Limit(event)) {
		return(false);
	}
	if (!Acquire_Voice(event)) {
		return(false);
	}

	bool attack = !event.AttackPlayed && !event.NoAttack;
	bool decay = true;
	if (event.DelayedLoop) {
		// Only the last cycle of a delayed loop carries the decay.
		decay = event.EndRequested || (event.CyclesLeft > 0 && event.CyclesLeft <= 1);
	}

	Release_Pins(event);
	if (!Issue(event, attack, true, decay)) {
		Release_Pins(event);
		Release_Voice(event);
		return(false);
	}
	event.AttackPlayed = true;
	event.State = AUDIO_EVENT_PLAYING;
	event.Queued = false;
	return(true);
}


AudioHandle AudioEventPoolClass::Start(AudioEventTypeClass const & type, AudioGroupType group, float volume, float pan, bool noattack)
{
	if (!Ready || type.SoundCount == 0 || group >= AUDIO_GROUP_COUNT) {
		return(AudioHandle());
	}
	EventClass * event = Allocate();
	if (event == nullptr) {
		return(AudioHandle());
	}

	event->Type = &type;
	event->Group = group;
	event->Priority = type.Priority;
	event->BaseVolume = type.Volume;
	event->RequestVolume = volume;
	event->Pan = pan;
	event->NoAttack = noattack;

	int vshift = Random(type.VShiftMin, type.VShiftMax);
	event->VShiftFactor = 1.0f + (float)vshift / 100.0f;
	if (event->VShiftFactor < 0.0f) event->VShiftFactor = 0.0f;
	if (event->VShiftFactor > 1.0f) event->VShiftFactor = 1.0f;

	int fshift = Random(type.FShiftMin, type.FShiftMax);
	event->Pitch = 1.0f + (float)fshift / 100.0f;
	if (event->Pitch < 0.5f) event->Pitch = 0.5f;
	if (event->Pitch > 2.0f) event->Pitch = 2.0f;

	bool loop = (type.Control & SOUND_CONTROL_LOOP) != 0;
	event->DelayedLoop = loop && type.Has_Delay();
	event->CyclesLeft = event->DelayedLoop ? ((type.Loop > 0) ? type.Loop : -1) : 1;

	event->State = AUDIO_EVENT_PENDING;
	type.LiveCount++;
	event->Counted = true;

	bool predelay = (type.Control & SOUND_CONTROL_PREDELAY) != 0 && type.Has_Delay();
	if (predelay) {
		event->NextStart = LastNow + (unsigned)Random(type.DelayMin, type.DelayMax);
		return(AudioHandle::Make((unsigned)(event - Events), event->Generation));
	}

	event->NextStart = LastNow;
	if (!Try_Start(*event)) {
		if (type.Control & SOUND_CONTROL_QUEUE) {
			event->Queued = true;
			event->QueuedSince = LastNow;
			return(AudioHandle::Make((unsigned)(event - Events), event->Generation));
		}
		Finish(*event);
		return(AudioHandle());
	}
	return(AudioHandle::Make((unsigned)(event - Events), event->Generation));
}


AudioHandle AudioEventPoolClass::Start_Sample(AudioSampleClass * clip, AudioGroupType group, float volume, int priority, void const * tag)
{
	if (!Ready || clip == nullptr || group >= AUDIO_GROUP_COUNT) {
		return(AudioHandle());
	}
	EventClass * event = Allocate();
	if (event == nullptr) {
		Clips->Release(clip);
		return(AudioHandle());
	}
	event->Type = nullptr;
	event->Group = group;
	event->Priority = priority;
	event->RequestVolume = volume;
	event->Tag = tag;
	event->Pins[0] = clip;
	event->PinCount = 1;
	event->State = AUDIO_EVENT_PENDING;

	if (!Acquire_Voice(*event)) {
		Finish(*event);
		return(AudioHandle());
	}

	AudioSequenceClass & sequence = event->Sequence;
	std::memset(&sequence, 0, sizeof(sequence));
	sequence.Clips[0] = clip;
	sequence.Count = 1;
	sequence.LoopStart = 0;
	sequence.LoopEnd = 1;
	sequence.Cycles = 1;

	AudioCommand command;
	std::memset(&command, 0, sizeof(command));
	command.Type = AudioCommandType::PLAY_SEQUENCE;
	command.Group = (uint8_t)group;
	command.Slot = (uint8_t)event->Voice;
	command.Generation = event->VoiceGeneration;
	command.A = Current_Level(*event);
	command.B = 0.0f;
	command.C = 1.0f;
	command.Ptr = &sequence;
	if (!Mixer->Push(command)) {
		Release_Voice(*event);
		Finish(*event);
		return(AudioHandle());
	}
	event->State = AUDIO_EVENT_PLAYING;
	return(AudioHandle::Make((unsigned)(event - Events), event->Generation));
}


AudioHandle AudioEventPoolClass::Start_Stream(AudioStreamClass * stream, AudioGroupType group, float volume, float pan)
{
	if (!Ready || stream == nullptr || group >= AUDIO_GROUP_COUNT) {
		return(AudioHandle());
	}
	EventClass * event = Allocate();
	if (event == nullptr) {
		return(AudioHandle());
	}
	event->Type = nullptr;
	event->Group = group;
	event->Priority = 255;
	event->RequestVolume = volume;
	event->Pan = pan;
	event->Stream = stream;
	event->State = AUDIO_EVENT_PENDING;

	if (!Acquire_Voice(*event)) {
		Finish(*event);
		return(AudioHandle());
	}

	AudioCommand command;
	std::memset(&command, 0, sizeof(command));
	command.Type = AudioCommandType::PLAY_STREAM;
	command.Group = (uint8_t)group;
	command.Slot = (uint8_t)event->Voice;
	command.Generation = event->VoiceGeneration;
	command.A = Current_Level(*event);
	command.B = pan;
	command.C = 1.0f;
	command.Ptr = stream;
	if (!Mixer->Push(command)) {
		Release_Voice(*event);
		Finish(*event);
		return(AudioHandle());
	}
	event->State = AUDIO_EVENT_PLAYING;
	return(AudioHandle::Make((unsigned)(event - Events), event->Generation));
}


// Returns every voice the mixer has finished with to the pool and moves its
// event on: to the next cycle of a delayed loop, to the decay it still owes,
// or to done.
void AudioEventPoolClass::Reap_Voices(unsigned now)
{
	for (int i = 0; i < AUDIO_MAX_EVENTS; i++) {
		EventClass & event = Events[i];
		if (event.Voice < 0 || Mixer->Voice_State((unsigned)event.Voice) != AudioVoiceState::DONE) {
			continue;
		}
		Release_Voice(event);
		if (event.State != AUDIO_EVENT_PLAYING) {
			continue;
		}
		Release_Pins(event);
		bool more = event.DelayedLoop && !event.EndRequested && (event.CyclesLeft < 0 || event.CyclesLeft > 1);
		bool decayowed = event.DelayedLoop && event.EndRequested && !event.DecayIssued && event.Type->DecayCount > 0;
		if (event.Stolen) {
			Finish(event);
		} else if (more) {
			if (event.CyclesLeft > 0) {
				event.CyclesLeft--;
			}
			event.State = AUDIO_EVENT_GAP;
			event.NextStart = now + (unsigned)Random(event.Type->DelayMin, event.Type->DelayMax);
		} else if (decayowed) {
			// Ended between cycles: the decay plays on its own, right away.
			event.State = AUDIO_EVENT_GAP;
			event.NextStart = now;
		} else {
			Finish(event);
		}
	}
}


void AudioEventPoolClass::Service(unsigned now)
{
	if (!Ready) {
		return;
	}
	if (now - LastNow > MAX_TICK_MS) {
		unsigned shift = now - LastNow - MAX_TICK_MS;
		for (int i = 0; i < AUDIO_MAX_EVENTS; i++) {
			EventClass & event = Events[i];
			if (event.State == AUDIO_EVENT_PENDING || event.State == AUDIO_EVENT_GAP) {
				event.NextStart += shift;
			}
		}
	}
	LastNow = now;
	Reap_Voices(now);

	for (int i = 0; i < AUDIO_MAX_EVENTS; i++) {
		EventClass & event = Events[i];

		if ((event.State == AUDIO_EVENT_PENDING || event.State == AUDIO_EVENT_GAP) && (int)(now - event.NextStart) >= 0) {
			if (event.State == AUDIO_EVENT_GAP && event.EndRequested) {
				event.CyclesLeft = 0;
				bool playing = !event.DecayIssued && event.Type->DecayCount > 0 && Acquire_Voice(event) && Issue(event, false, false, true);
				if (playing) {
					event.State = AUDIO_EVENT_PLAYING;
				} else {
					Release_Voice(event);
					Finish(event);
				}
				continue;
			}

			if (!Try_Start(event)) {
				if (event.Queued && now - event.QueuedSince < QUEUE_WAIT_MS) {
					continue;
				}
				Finish(event);
			}
		}
	}
}


bool AudioEventPoolClass::Is_Valid(AudioHandle handle) const
{
	return(Lookup(handle) != nullptr);
}


bool AudioEventPoolClass::Is_Playing(AudioHandle handle) const
{
	return(Lookup(handle) != nullptr);
}


bool AudioEventPoolClass::Is_Finished(AudioHandle handle) const
{
	if (!Ready || handle.Is_Null() || handle.Index() >= AUDIO_MAX_EVENTS) {
		return(true);
	}
	EventClass const & event = Events[handle.Index()];
	return(event.Generation != handle.Generation() || !event.Is_Live());
}


AudioEventTypeClass const * AudioEventPoolClass::Type(AudioHandle handle) const
{
	EventClass const * event = Lookup(handle);
	return(event != nullptr ? event->Type : nullptr);
}


AudioEventState AudioEventPoolClass::State(AudioHandle handle) const
{
	EventClass const * event = Lookup(handle);
	return(event != nullptr ? event->State : AUDIO_EVENT_DONE);
}


void AudioEventPoolClass::Retarget(AudioHandle handle, float volume, float pan)
{
	EventClass * event = Lookup(handle);
	if (event == nullptr) {
		return;
	}
	event->RequestVolume = volume;
	event->Pan = pan;
	Push_Level(*event, AUDIO_RETARGET_RAMP_SECONDS);
	Push_Pan(*event, AUDIO_RETARGET_RAMP_SECONDS);
}


void AudioEventPoolClass::Set_Volume(AudioHandle handle, float volume)
{
	EventClass * event = Lookup(handle);
	if (event == nullptr) {
		return;
	}
	event->RequestVolume = volume;
	Push_Level(*event, AUDIO_RETARGET_RAMP_SECONDS);
}


void AudioEventPoolClass::Set_Pan(AudioHandle handle, float pan)
{
	EventClass * event = Lookup(handle);
	if (event == nullptr) {
		return;
	}
	event->Pan = pan;
	Push_Pan(*event, AUDIO_RETARGET_RAMP_SECONDS);
}


void AudioEventPoolClass::Stop(AudioHandle handle)
{
	EventClass * event = Lookup(handle);
	if (event != nullptr) {
		Kill(*event, AUDIO_STOP_RAMP_SECONDS);
	}
}


void AudioEventPoolClass::Fade(AudioHandle handle, int ms)
{
	EventClass * event = Lookup(handle);
	if (event != nullptr) {
		Kill(*event, ms > 0 ? (float)ms / 1000.0f : AUDIO_STOP_RAMP_SECONDS);
	}
}


void AudioEventPoolClass::End(AudioHandle handle)
{
	EventClass * event = Lookup(handle);
	if (event == nullptr) {
		return;
	}
	event->EndRequested = true;
	if (event->State == AUDIO_EVENT_PENDING) {
		Finish(*event);
		return;
	}
	if (event->State == AUDIO_EVENT_GAP) {
		// The decay, if any, plays at the next service instead of after the gap.
		event->NextStart = LastNow;
		return;
	}
	AudioCommand command;
	std::memset(&command, 0, sizeof(command));
	command.Type = AudioCommandType::END_SEQUENCE;
	command.Slot = (uint8_t)event->Voice;
	command.Generation = event->VoiceGeneration;
	command.Mode = (uint8_t)AudioEndMode::NOW;
	Mixer->Push(command);
}


void AudioEventPoolClass::End_Looping(AudioHandle handle)
{
	EventClass * event = Lookup(handle);
	if (event == nullptr) {
		return;
	}
	bool loops = event->Type != nullptr && (event->Type->Control & SOUND_CONTROL_LOOP) != 0;
	if (!loops) {
		return;
	}
	event->EndRequested = true;
	if (event->State == AUDIO_EVENT_PENDING) {
		Finish(*event);
		return;
	}
	if (event->State == AUDIO_EVENT_GAP) {
		event->NextStart = LastNow;
		return;
	}
	AudioCommand command;
	std::memset(&command, 0, sizeof(command));
	command.Type = AudioCommandType::END_SEQUENCE;
	command.Slot = (uint8_t)event->Voice;
	command.Generation = event->VoiceGeneration;
	command.Mode = (uint8_t)AudioEndMode::AFTER_CYCLE;
	Mixer->Push(command);
}


void AudioEventPoolClass::Pause(AudioHandle handle)
{
	EventClass * event = Lookup(handle);
	if (event == nullptr || event->Voice < 0) {
		return;
	}
	AudioCommand command;
	std::memset(&command, 0, sizeof(command));
	command.Type = AudioCommandType::PAUSE;
	command.Slot = (uint8_t)event->Voice;
	command.Generation = event->VoiceGeneration;
	Mixer->Push(command);
}


void AudioEventPoolClass::Resume(AudioHandle handle)
{
	EventClass * event = Lookup(handle);
	if (event == nullptr || event->Voice < 0) {
		return;
	}
	AudioCommand command;
	std::memset(&command, 0, sizeof(command));
	command.Type = AudioCommandType::RESUME;
	command.Slot = (uint8_t)event->Voice;
	command.Generation = event->VoiceGeneration;
	Mixer->Push(command);
}


bool AudioEventPoolClass::Is_Tag_Playing(void const * tag) const
{
	if (!Ready || tag == nullptr) {
		return(false);
	}
	for (int i = 0; i < AUDIO_MAX_EVENTS; i++) {
		EventClass const & event = Events[i];
		if (event.Is_Live() && !event.Stolen && event.Tag == tag) {
			return(true);
		}
	}
	return(false);
}


void AudioEventPoolClass::Stop_Tag(void const * tag)
{
	if (!Ready || tag == nullptr) {
		return;
	}
	for (int i = 0; i < AUDIO_MAX_EVENTS; i++) {
		EventClass & event = Events[i];
		if (event.Is_Live() && event.Tag == tag) {
			Kill(event, AUDIO_STOP_RAMP_SECONDS);
		}
	}
}


void AudioEventPoolClass::Set_Tag_Volume(void const * tag, float volume)
{
	if (!Ready || tag == nullptr) {
		return;
	}
	for (int i = 0; i < AUDIO_MAX_EVENTS; i++) {
		EventClass & event = Events[i];
		if (event.Is_Live() && !event.Stolen && event.Tag == tag) {
			event.RequestVolume = volume;
			Push_Level(event, AUDIO_RETARGET_RAMP_SECONDS);
		}
	}
}


void AudioEventPoolClass::Stop_All(void)
{
	if (!Ready) {
		return;
	}
	for (int i = 0; i < AUDIO_MAX_EVENTS; i++) {
		if (Events[i].Is_Live()) {
			Kill(Events[i], AUDIO_STOP_RAMP_SECONDS);
		}
	}
}


void AudioEventPoolClass::Stop_Group(AudioGroupType group, int fadems)
{
	if (!Ready) {
		return;
	}
	float fade = fadems > 0 ? (float)fadems / 1000.0f : AUDIO_STOP_RAMP_SECONDS;
	for (int i = 0; i < AUDIO_MAX_EVENTS; i++) {
		if (Events[i].Is_Live() && Events[i].Group == group) {
			Kill(Events[i], fade);
		}
	}
}


unsigned AudioEventPoolClass::Live_Count(void) const
{
	unsigned count = 0;
	for (int i = 0; Ready && i < AUDIO_MAX_EVENTS; i++) {
		if (Events[i].Is_Live() && !Events[i].Stolen) {
			count++;
		}
	}
	return(count);
}


unsigned AudioEventPoolClass::Live_Count(AudioEventTypeClass const & type) const
{
	unsigned count = 0;
	for (int i = 0; Ready && i < AUDIO_MAX_EVENTS; i++) {
		if (Events[i].Is_Live() && !Events[i].Stolen && Events[i].Type == &type) {
			count++;
		}
	}
	return(count);
}


bool AudioEventPoolClass::Get_Sequence(AudioHandle handle, AudioSequenceClass & sequence) const
{
	EventClass const * event = Lookup(handle);
	if (event == nullptr) {
		return(false);
	}
	sequence = event->Sequence;
	return(true);
}
