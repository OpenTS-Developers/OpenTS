/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "sndtype.h"

#include "dbgprint.h"
#include "ini.h"

#include <cstdlib>
#include <cstring>
#include <string>

namespace {

int const MAX_TOKENS = AUDIO_MAX_SOUNDS;
int const TOKEN_LENGTH = 32;

// Pitch stays within 0.5..2.0.
int const FSHIFT_MIN = -50;
int const FSHIFT_MAX = 100;

int const CHANNELS_MIN = 4;
int const CHANNELS_MAX = 32;


struct FlagName {
	char const * Name;
	unsigned Value;
};

FlagName const TYPE_NAMES[] = {
	{ "NORMAL", SOUND_TYPE_NORMAL },
	{ "VIOLENT", SOUND_TYPE_VIOLENT },
	{ "MOVEMENT", SOUND_TYPE_MOVEMENT },
	{ "QUIET", SOUND_TYPE_QUIET },
	{ "LOUD", SOUND_TYPE_LOUD },
	{ "GLOBAL", SOUND_TYPE_GLOBAL },
	{ "SCREEN", SOUND_TYPE_SCREEN },
	{ "LOCAL", SOUND_TYPE_LOCAL },
	{ "PLAYER", SOUND_TYPE_PLAYER },
	{ "NOISE_SHY", SOUND_TYPE_NOISE_SHY },
	{ "NOISESHY", SOUND_TYPE_NOISE_SHY },
	{ "GUN_SHY", SOUND_TYPE_GUN_SHY },
	{ "GUNSHY", SOUND_TYPE_GUN_SHY },
	{ "UNSHROUD", SOUND_TYPE_UNSHROUD },
	{ "SHROUD", SOUND_TYPE_SHROUD },
	{ "UNSHROUDED", SOUND_TYPE_SHROUD },
	{ "SHROUDED", SOUND_TYPE_HIDDEN },
	{ "AMBIENT", SOUND_TYPE_AMBIENT },
};

FlagName const CONTROL_NAMES[] = {
	{ "NORMAL", SOUND_CONTROL_NONE },
	{ "LOOP", SOUND_CONTROL_LOOP },
	{ "RANDOM", SOUND_CONTROL_RANDOM },
	{ "ALL", SOUND_CONTROL_ALL },
	{ "PREDELAY", SOUND_CONTROL_PREDELAY },
	{ "INTERRUPT", SOUND_CONTROL_INTERRUPT },
	{ "ATTACK", SOUND_CONTROL_ATTACK },
	{ "DECAY", SOUND_CONTROL_DECAY },
	{ "AMBIENT", SOUND_CONTROL_AMBIENT },
	{ "SEQUENTIAL", SOUND_CONTROL_SEQUENTIAL },
	{ "QUEUE", SOUND_CONTROL_QUEUE },
};

struct PriorityName {
	char const * Name;
	int Value;
};

PriorityName const PRIORITY_NAMES[] = {
	{ "LOWEST", 0 },
	{ "LOW", 10 },
	{ "NORMAL", 50 },
	{ "HIGH", 100 },
	{ "CRITICAL", 255 },
};


// Splits on spaces, commas and tabs. Returns the token count.
int Tokenize(char const * text, char tokens[MAX_TOKENS][TOKEN_LENGTH])
{
	int count = 0;
	if (text == nullptr) {
		return(0);
	}
	while (*text != '\0' && count < MAX_TOKENS) {
		while (*text == ' ' || *text == ',' || *text == '\t') {
			text++;
		}
		if (*text == '\0') {
			break;
		}
		int length = 0;
		while (*text != '\0' && *text != ' ' && *text != ',' && *text != '\t') {
			if (length < TOKEN_LENGTH - 1) {
				tokens[count][length++] = *text;
			}
			text++;
		}
		tokens[count][length] = '\0';
		count++;
	}
	return(count);
}


bool Is_Number(char const * text)
{
	if (*text == '-' || *text == '+') {
		text++;
	}
	bool digits = false;
	while (*text != '\0') {
		if (*text >= '0' && *text <= '9') {
			digits = true;
		} else if (*text != '.') {
			return(false);
		}
		text++;
	}
	return(digits);
}


int Clamp(int value, int low, int high)
{
	return(value < low ? low : (value > high ? high : value));
}


unsigned Parse_Flags(char const * text, unsigned fallback, FlagName const * names, int count, char const * what)
{
	char tokens[MAX_TOKENS][TOKEN_LENGTH];
	int found = Tokenize(text, tokens);
	if (found == 0) {
		return(fallback);
	}
	unsigned flags = 0;
	for (int i = 0; i < found; i++) {
		bool known = false;
		for (int j = 0; j < count; j++) {
			if (_stricmp(tokens[i], names[j].Name) == 0) {
				flags |= names[j].Value;
				known = true;
				break;
			}
		}
		if (!known) {
			DebugString("SOUND.INI: unknown %s flag '%s'\n", what, tokens[i]);
		}
	}
	return(flags);
}


bool Parse_Pair(char const * text, int & low, int & high, bool & single, bool & seconds)
{
	char tokens[MAX_TOKENS][TOKEN_LENGTH];
	int found = Tokenize(text, tokens);
	if (found == 0 || !Is_Number(tokens[0]) || (found > 1 && !Is_Number(tokens[1]))) {
		return(false);
	}
	seconds = std::strchr(tokens[0], '.') != nullptr || (found > 1 && std::strchr(tokens[1], '.') != nullptr);
	double first = std::atof(tokens[0]);
	double second = found > 1 ? std::atof(tokens[1]) : first;
	if (seconds) {
		first *= 1000.0;
		second *= 1000.0;
	}
	low = (int)(first < 0 ? first - 0.5 : first + 0.5);
	high = (int)(second < 0 ? second - 0.5 : second + 0.5);
	single = (found == 1);
	return(true);
}


void Read_Sounds(INIClass const & ini, char const * section, AudioEventTypeClass & type)
{
	std::string sounds = ini.Get_String(section, "Sounds", "");
	char tokens[MAX_TOKENS][TOKEN_LENGTH];
	int found = Tokenize(sounds.c_str(), tokens);
	if (found == 0) {
		return;
	}
	type.SoundCount = 0;
	for (int i = 0; i < found && type.SoundCount < (unsigned)AUDIO_MAX_SOUNDS; i++) {
		std::strncpy(type.Sounds[type.SoundCount], tokens[i], sizeof(type.Sounds[0]) - 1);
		type.Sounds[type.SoundCount][sizeof(type.Sounds[0]) - 1] = '\0';
		type.SoundCount++;
	}
}


// Each key is staged into a local of its own name and applied only when the
// section carries it, so a section keeps the defaults for what it omits.
void Read_Keys(INIClass const & ini, char const * section, AudioEventTypeClass & type, bool defaults)
{
	if (!defaults) {
		Read_Sounds(ini, section, type);
	}

	std::string priority = ini.Get_String(section, "Priority", "");
	if (!priority.empty()) {
		type.Priority = Sound_Type_Parse_Priority(priority.c_str(), type.Priority);
	}
	std::string volume = ini.Get_String(section, "Volume", "");
	if (!volume.empty()) {
		type.Volume = Sound_Type_Parse_Volume(volume.c_str(), type.Volume);
	}
	std::string minvolume = ini.Get_String(section, "MinVolume", "");
	if (!minvolume.empty()) {
		type.MinVolume = Sound_Type_Parse_Volume(minvolume.c_str(), type.MinVolume);
	}
	int range = ini.Get_Int(section, "Range", type.Range);
	type.Range = Clamp(range, 0, 1000);
	int limit = ini.Get_Int(section, "Limit", type.Limit);
	type.Limit = Clamp(limit, 0, AUDIO_MAX_EVENTS);
	int loop = ini.Get_Int(section, "Loop", -1);
	if (loop < 0) {
		loop = ini.Get_Int(section, "LoopLimit", type.Loop);
	}
	type.Loop = Clamp(loop, 0, 100000);

	int low;
	int high;
	std::string delay = ini.Get_String(section, "Delay", "");
	if (Sound_Type_Parse_Delay(delay.c_str(), low, high)) {
		type.DelayMin = low;
		type.DelayMax = high;
	}
	std::string fshift = ini.Get_String(section, "FShift", "");
	if (Sound_Type_Parse_Shift(fshift.c_str(), false, low, high)) {
		type.FShiftMin = low;
		type.FShiftMax = high;
	}
	std::string vshift = ini.Get_String(section, "VShift", "");
	if (Sound_Type_Parse_Shift(vshift.c_str(), true, low, high)) {
		type.VShiftMin = low;
		type.VShiftMax = high;
	}
	std::string typeflags = ini.Get_String(section, "Type", "");
	if (!typeflags.empty()) {
		type.Type = Sound_Type_Parse_Type(typeflags.c_str(), type.Type);
	}
	std::string control = ini.Get_String(section, "Control", "");
	if (!control.empty()) {
		type.Control = Sound_Type_Parse_Control(control.c_str(), type.Control);
	}

	// A count given outright wins; otherwise the flag alone means one sound.
	int attack = ini.Get_Int(section, "Attack", -1);
	if (attack >= 0) {
		type.AttackCount = Clamp(attack, 0, AUDIO_MAX_SOUNDS);
	} else if (!control.empty() || defaults) {
		type.AttackCount = (type.Control & SOUND_CONTROL_ATTACK) ? 1 : 0;
	}
	int decay = ini.Get_Int(section, "Decay", -1);
	if (decay >= 0) {
		type.DecayCount = Clamp(decay, 0, AUDIO_MAX_SOUNDS);
	} else if (!control.empty() || defaults) {
		type.DecayCount = (type.Control & SOUND_CONTROL_DECAY) ? 1 : 0;
	}
}

} // namespace


int Sound_Type_Parse_Priority(char const * text, int fallback)
{
	char tokens[MAX_TOKENS][TOKEN_LENGTH];
	if (Tokenize(text, tokens) == 0) {
		return(fallback);
	}
	for (PriorityName const & name : PRIORITY_NAMES) {
		if (_stricmp(tokens[0], name.Name) == 0) {
			return(name.Value);
		}
	}
	if (!Is_Number(tokens[0])) {
		DebugString("SOUND.INI: unknown priority '%s'\n", tokens[0]);
		return(fallback);
	}
	return(Clamp(std::atoi(tokens[0]), 0, 255));
}


float Sound_Type_Parse_Volume(char const * text, float fallback)
{
	char tokens[MAX_TOKENS][TOKEN_LENGTH];
	if (Tokenize(text, tokens) == 0 || !Is_Number(tokens[0])) {
		return(fallback);
	}
	double value = std::atof(tokens[0]);
	if (value > 1.0) {
		// Percent, as Yuri's Revenge writes it.
		value /= 100.0;
	}
	if (value < 0.0) value = 0.0;
	if (value > 1.0) value = 1.0;
	return((float)value);
}


unsigned Sound_Type_Parse_Type(char const * text, unsigned fallback)
{
	return(Parse_Flags(text, fallback, TYPE_NAMES, (int)(sizeof(TYPE_NAMES) / sizeof(TYPE_NAMES[0])), "Type"));
}


unsigned Sound_Type_Parse_Control(char const * text, unsigned fallback)
{
	return(Parse_Flags(text, fallback, CONTROL_NAMES, (int)(sizeof(CONTROL_NAMES) / sizeof(CONTROL_NAMES[0])), "Control"));
}


bool Sound_Type_Parse_Delay(char const * text, int & low, int & high)
{
	bool single;
	bool seconds;
	if (!Parse_Pair(text, low, high, single, seconds)) {
		return(false);
	}
	if (low < 0) low = 0;
	if (high < 0) high = 0;
	if (high < low) {
		int swap = low;
		low = high;
		high = swap;
	}
	return(true);
}


bool Sound_Type_Parse_Shift(char const * text, bool attenuate, int & low, int & high)
{
	bool single;
	bool seconds;
	if (!Parse_Pair(text, low, high, single, seconds)) {
		return(false);
	}
	if (single) {
		int span = low < 0 ? -low : low;
		low = -span;
		high = attenuate ? 0 : span;
	}
	if (high < low) {
		int swap = low;
		low = high;
		high = swap;
	}
	if (attenuate) {
		low = Clamp(low, -100, 100);
		high = Clamp(high, -100, 100);
	} else {
		low = Clamp(low, FSHIFT_MIN, FSHIFT_MAX);
		high = Clamp(high, FSHIFT_MIN, FSHIFT_MAX);
	}
	return(true);
}


void Sound_Type_Read_Defaults(INIClass const & ini, AudioEventTypeClass & defaults)
{
	defaults = AudioEventTypeClass();
	std::strncpy(defaults.Name, "Defaults", sizeof(defaults.Name) - 1);
	if (ini.Is_Present("Defaults")) {
		Read_Keys(ini, "Defaults", defaults, true);
	}
}


int Sound_Type_Read_Channels(INIClass const & ini, int fallback)
{
	int channels = ini.Get_Int("General", "Channels", fallback);
	return(Clamp(channels, CHANNELS_MIN, CHANNELS_MAX));
}


bool Sound_Type_Fill_In(INIClass const & ini, char const * section, AudioEventTypeClass const & defaults, AudioEventTypeClass & type)
{
	type = defaults;
	std::strncpy(type.Name, section, sizeof(type.Name) - 1);
	type.Name[sizeof(type.Name) - 1] = '\0';
	std::strncpy(type.Sounds[0], type.Name, sizeof(type.Sounds[0]) - 1);
	type.Sounds[0][sizeof(type.Sounds[0]) - 1] = '\0';
	type.SoundCount = 1;
	type.LiveCount = 0;
	type.SequentialIndex = 0;

	if (!ini.Is_Present(section)) {
		return(false);
	}
	Read_Keys(ini, section, type, false);
	return(true);
}
