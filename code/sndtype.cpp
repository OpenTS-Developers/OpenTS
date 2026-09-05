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
	char buffer[AUDIO_MAX_SOUNDS * TOKEN_LENGTH];
	if (ini.Get_String(section, "Sounds", "", buffer, sizeof(buffer)) == 0) {
		return;
	}
	char tokens[MAX_TOKENS][TOKEN_LENGTH];
	int found = Tokenize(buffer, tokens);
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


void Read_Keys(INIClass const & ini, char const * section, AudioEventTypeClass & type, bool defaults)
{
	char buffer[256];

	if (!defaults) {
		Read_Sounds(ini, section, type);
	}

	if (ini.Get_String(section, "Priority", "", buffer, sizeof(buffer)) != 0) {
		type.Priority = Sound_Type_Parse_Priority(buffer, type.Priority);
	}
	if (ini.Get_String(section, "Volume", "", buffer, sizeof(buffer)) != 0) {
		type.Volume = Sound_Type_Parse_Volume(buffer, type.Volume);
	}
	if (ini.Get_String(section, "MinVolume", "", buffer, sizeof(buffer)) != 0) {
		type.MinVolume = Sound_Type_Parse_Volume(buffer, type.MinVolume);
	}
	if (ini.Is_Present(section, "Range")) {
		type.Range = Clamp(ini.Get_Int(section, "Range", type.Range), 0, 1000);
	}
	if (ini.Is_Present(section, "Limit")) {
		type.Limit = Clamp(ini.Get_Int(section, "Limit", type.Limit), 0, AUDIO_MAX_EVENTS);
	}
	if (ini.Is_Present(section, "Loop")) {
		type.Loop = Clamp(ini.Get_Int(section, "Loop", type.Loop), 0, 100000);
	} else if (ini.Is_Present(section, "LoopLimit")) {
		type.Loop = Clamp(ini.Get_Int(section, "LoopLimit", type.Loop), 0, 100000);
	}
	if (ini.Get_String(section, "Delay", "", buffer, sizeof(buffer)) != 0) {
		int low;
		int high;
		if (Sound_Type_Parse_Delay(buffer, low, high)) {
			type.DelayMin = low;
			type.DelayMax = high;
		}
	}
	if (ini.Get_String(section, "FShift", "", buffer, sizeof(buffer)) != 0) {
		int low;
		int high;
		if (Sound_Type_Parse_Shift(buffer, false, low, high)) {
			type.FShiftMin = low;
			type.FShiftMax = high;
		}
	}
	if (ini.Get_String(section, "VShift", "", buffer, sizeof(buffer)) != 0) {
		int low;
		int high;
		if (Sound_Type_Parse_Shift(buffer, true, low, high)) {
			type.VShiftMin = low;
			type.VShiftMax = high;
		}
	}
	if (ini.Get_String(section, "Type", "", buffer, sizeof(buffer)) != 0) {
		type.Type = Sound_Type_Parse_Type(buffer, type.Type);
	}
	if (ini.Get_String(section, "Control", "", buffer, sizeof(buffer)) != 0) {
		type.Control = Sound_Type_Parse_Control(buffer, type.Control);
	}

	// A count given outright wins; otherwise the flag alone means one sound.
	if (ini.Is_Present(section, "Attack")) {
		type.AttackCount = Clamp(ini.Get_Int(section, "Attack", 0), 0, AUDIO_MAX_SOUNDS);
	} else if (ini.Is_Present(section, "Control") || defaults) {
		type.AttackCount = (type.Control & SOUND_CONTROL_ATTACK) ? 1 : 0;
	}
	if (ini.Is_Present(section, "Decay")) {
		type.DecayCount = Clamp(ini.Get_Int(section, "Decay", 0), 0, AUDIO_MAX_SOUNDS);
	} else if (ini.Is_Present(section, "Control") || defaults) {
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
