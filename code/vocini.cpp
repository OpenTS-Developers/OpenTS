/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The INI half of VocClass: reads SOUND.INI sections into the engine's sound
// type. The grammar follows Yuri's Revenge, with defaults that keep the
// shipped Tiberian Sun files playing as they did.

#include "voc.h"

#include "dbgprint.h"
#include "ini.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

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


// Splits on spaces, commas and tabs.
std::vector<std::string> Tokenize(char const * text)
{
	char const * const SEPARATORS = " ,\t";
	std::vector<std::string> tokens;
	std::string_view rest = text != nullptr ? text : "";
	for (;;) {
		size_t start = rest.find_first_not_of(SEPARATORS);
		if (start == std::string_view::npos) {
			break;
		}
		size_t end = rest.find_first_of(SEPARATORS, start);
		tokens.emplace_back(rest.substr(start, end - start));
		if (end == std::string_view::npos) {
			break;
		}
		rest.remove_prefix(end);
	}
	return(tokens);
}


// Digits with an optional sign and point; no exponent, hex or infinity.
bool Is_Number(char const * text)
{
	if (*text == '-' || *text == '+') {
		text++;
	}
	return(std::strpbrk(text, "0123456789") != nullptr && text[std::strspn(text, "0123456789.")] == '\0');
}


unsigned Parse_Flags(char const * text, unsigned fallback, std::span<FlagName const> names, char const * what)
{
	std::vector<std::string> tokens = Tokenize(text);
	if (tokens.empty()) {
		return(fallback);
	}
	unsigned flags = 0;
	for (std::string const & token : tokens) {
		auto match = std::find_if(names.begin(), names.end(), [&token](FlagName const & name) { return(_stricmp(token.c_str(), name.Name) == 0); });
		if (match != names.end()) {
			flags |= match->Value;
		} else {
			DebugString("SOUND.INI: unknown %s flag '%s'\n", what, token.c_str());
		}
	}
	return(flags);
}


bool Parse_Pair(char const * text, int & low, int & high, bool & single, bool & seconds)
{
	std::vector<std::string> tokens = Tokenize(text);
	if (tokens.empty() || !Is_Number(tokens[0].c_str()) || (tokens.size() > 1 && !Is_Number(tokens[1].c_str()))) {
		return(false);
	}
	seconds = tokens[0].find('.') != std::string::npos || (tokens.size() > 1 && tokens[1].find('.') != std::string::npos);
	double first = std::atof(tokens[0].c_str());
	double second = tokens.size() > 1 ? std::atof(tokens[1].c_str()) : first;
	if (seconds) {
		first *= 1000.0;
		second *= 1000.0;
	}
	low = (int)std::lround(first);
	high = (int)std::lround(second);
	single = (tokens.size() == 1);
	return(true);
}


void Read_Sounds(INIClass const & ini, char const * section, AudioEventTypeClass & type)
{
	std::string sounds = ini.Get_String(section, "Sounds", "");
	std::vector<std::string> tokens = Tokenize(sounds.c_str());
	if (tokens.empty()) {
		return;
	}
	type.SoundCount = 0;
	for (std::string const & token : tokens) {
		if (type.SoundCount >= (unsigned)AUDIO_MAX_SOUNDS) {
			break;
		}
		std::strncpy(type.Sounds[type.SoundCount], token.c_str(), sizeof(type.Sounds[0]) - 1);
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
		type.Priority = Sound_Parse_Priority(priority.c_str(), type.Priority);
	}
	std::string volume = ini.Get_String(section, "Volume", "");
	if (!volume.empty()) {
		type.Volume = Sound_Parse_Volume(volume.c_str(), type.Volume);
	}
	std::string minvolume = ini.Get_String(section, "MinVolume", "");
	if (!minvolume.empty()) {
		type.MinVolume = Sound_Parse_Volume(minvolume.c_str(), type.MinVolume);
	}
	int range = ini.Get_Int(section, "Range", type.Range);
	type.Range = std::clamp(range, 0, 1000);
	int limit = ini.Get_Int(section, "Limit", type.Limit);
	type.Limit = std::clamp(limit, 0, AUDIO_MAX_EVENTS);
	int loop = ini.Get_Int(section, "Loop", -1);
	if (loop < 0) {
		loop = ini.Get_Int(section, "LoopLimit", type.Loop);
	}
	type.Loop = std::clamp(loop, 0, 100000);

	int low;
	int high;
	std::string delay = ini.Get_String(section, "Delay", "");
	if (Sound_Parse_Delay(delay.c_str(), low, high)) {
		type.DelayMin = low;
		type.DelayMax = high;
	}
	std::string fshift = ini.Get_String(section, "FShift", "");
	if (Sound_Parse_Shift(fshift.c_str(), false, low, high)) {
		type.FShiftMin = low;
		type.FShiftMax = high;
	}
	std::string vshift = ini.Get_String(section, "VShift", "");
	if (Sound_Parse_Shift(vshift.c_str(), true, low, high)) {
		type.VShiftMin = low;
		type.VShiftMax = high;
	}
	std::string typeflags = ini.Get_String(section, "Type", "");
	if (!typeflags.empty()) {
		type.Type = Sound_Parse_Type(typeflags.c_str(), type.Type);
	}
	std::string control = ini.Get_String(section, "Control", "");
	if (!control.empty()) {
		type.Control = Sound_Parse_Control(control.c_str(), type.Control);
	}

	// A count given outright wins; otherwise the flag alone means one sound.
	int attack = ini.Get_Int(section, "Attack", -1);
	if (attack >= 0) {
		type.AttackCount = std::clamp(attack, 0, AUDIO_MAX_SOUNDS);
	} else if (!control.empty() || defaults) {
		type.AttackCount = (type.Control & SOUND_CONTROL_ATTACK) ? 1 : 0;
	}
	int decay = ini.Get_Int(section, "Decay", -1);
	if (decay >= 0) {
		type.DecayCount = std::clamp(decay, 0, AUDIO_MAX_SOUNDS);
	} else if (!control.empty() || defaults) {
		type.DecayCount = (type.Control & SOUND_CONTROL_DECAY) ? 1 : 0;
	}
}

} // namespace


int Sound_Parse_Priority(char const * text, int fallback)
{
	std::vector<std::string> tokens = Tokenize(text);
	if (tokens.empty()) {
		return(fallback);
	}
	char const * first = tokens[0].c_str();
	auto match = std::find_if(std::begin(PRIORITY_NAMES), std::end(PRIORITY_NAMES), [first](PriorityName const & name) { return(_stricmp(first, name.Name) == 0); });
	if (match != std::end(PRIORITY_NAMES)) {
		return(match->Value);
	}
	if (!Is_Number(first)) {
		DebugString("SOUND.INI: unknown priority '%s'\n", first);
		return(fallback);
	}
	return(std::clamp(std::atoi(first), 0, 255));
}


float Sound_Parse_Volume(char const * text, float fallback)
{
	std::vector<std::string> tokens = Tokenize(text);
	if (tokens.empty() || !Is_Number(tokens[0].c_str())) {
		return(fallback);
	}
	double value = std::atof(tokens[0].c_str());
	if (value > 1.0) {
		// Percent, as Yuri's Revenge writes it.
		value /= 100.0;
	}
	return((float)std::clamp(value, 0.0, 1.0));
}


unsigned Sound_Parse_Type(char const * text, unsigned fallback)
{
	return(Parse_Flags(text, fallback, TYPE_NAMES, "Type"));
}


unsigned Sound_Parse_Control(char const * text, unsigned fallback)
{
	return(Parse_Flags(text, fallback, CONTROL_NAMES, "Control"));
}


bool Sound_Parse_Delay(char const * text, int & low, int & high)
{
	bool single;
	bool seconds;
	if (!Parse_Pair(text, low, high, single, seconds)) {
		return(false);
	}
	low = std::max(low, 0);
	high = std::max(high, 0);
	if (high < low) {
		std::swap(low, high);
	}
	return(true);
}


bool Sound_Parse_Shift(char const * text, bool attenuate, int & low, int & high)
{
	bool single;
	bool seconds;
	if (!Parse_Pair(text, low, high, single, seconds)) {
		return(false);
	}
	if (single) {
		int span = std::abs(low);
		low = -span;
		high = attenuate ? 0 : span;
	}
	if (high < low) {
		std::swap(low, high);
	}
	if (attenuate) {
		low = std::clamp(low, -100, 100);
		high = std::clamp(high, -100, 100);
	} else {
		low = std::clamp(low, FSHIFT_MIN, FSHIFT_MAX);
		high = std::clamp(high, FSHIFT_MIN, FSHIFT_MAX);
	}
	return(true);
}


void VocClass::Read_Defaults(INIClass const & ini, AudioEventTypeClass & defaults)
{
	defaults = AudioEventTypeClass();
	std::strncpy(defaults.Name, "Defaults", sizeof(defaults.Name) - 1);
	if (ini.Is_Present("Defaults")) {
		Read_Keys(ini, "Defaults", defaults, true);
	}
}


int VocClass::Read_Channels(INIClass const & ini, int fallback)
{
	int channels = ini.Get_Int("General", "Channels", fallback);
	return(std::clamp(channels, CHANNELS_MIN, CHANNELS_MAX));
}


bool VocClass::Read_Type(INIClass const & ini, char const * section, AudioEventTypeClass const & defaults, AudioEventTypeClass & type)
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
