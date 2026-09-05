/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Reads sound definitions shaped like the shipped Tiberian Sun files, like
// Yuri's Revenge, and like Vinifera, and checks what each key becomes. Needs
// no game data.

#include "audio/audiodefs.hh"
#include "audio/audioevent.h"
#include "ini.h"
#include "sndtype.h"
#include "xstraw.h"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace {

int Failures = 0;
int Checked = 0;


void Check(bool condition, char const * what)
{
	Checked++;
	if (!condition) {
		Failures++;
		std::printf("FAIL: %s\n", what);
	}
}


bool Near(float value, float expect)
{
	return(std::fabs(value - expect) < 0.001f);
}


void Read(INIClass & ini, char const * text)
{
	BufferStraw straw(text, (int)std::strlen(text));
	ini.Load(straw);
}


bool Is_Defaults(AudioEventTypeClass const & type)
{
	return(Near(type.Volume, 1.0f) && Near(type.MinVolume, 0.0f) && type.Range == 28 && type.Limit == 3 && type.Loop == 0
		&& type.DelayMin == 0 && type.DelayMax == 0 && type.FShiftMin == 0 && type.FShiftMax == 0 && type.VShiftMin == 0 && type.VShiftMax == 0
		&& type.AttackCount == 0 && type.DecayCount == 0 && type.Type == SOUND_TYPE_SCREEN && type.Control == SOUND_CONTROL_NONE);
}


void Test_Tiberian_Sun_Shape(void)
{
	INIClass ini;
	Read(ini,
		"[SoundList]\r\n"
		"0=GUN5\r\n"
		"1=EXPLOLG1\r\n"
		"2=SUPERWPN\r\n"
		"3=WHISPER\r\n"
		"4=CLICK\r\n"
		"\r\n"
		"[GUN5]\r\n"
		"Priority=100\r\n"
		"\r\n"
		"[EXPLOLG1]\r\n"
		"Priority=50\r\n"
		"\r\n"
		"[SUPERWPN]\r\n"
		"Priority=75\r\n"
		"\r\n"
		"[WHISPER]\r\n"
		"Priority=15\r\n"
		"\r\n"
		"[CLICK]\r\n");

	AudioEventTypeClass defaults;
	Sound_Type_Read_Defaults(ini, defaults);
	Check(defaults.Priority == 10 && Is_Defaults(defaults), "no [Defaults] leaves the engine defaults");
	Check(Sound_Type_Read_Channels(ini, 16) == 16, "no [General] leaves the channel count");

	AudioEventTypeClass type;
	Check(Sound_Type_Fill_In(ini, "GUN5", defaults, type), "section found");
	Check(type.Priority == 100 && Is_Defaults(type), "priority 100 with everything else default");
	Check(std::strcmp(type.Name, "GUN5") == 0 && type.SoundCount == 1 && std::strcmp(type.Sounds[0], "GUN5") == 0, "the section name is the sound");

	Sound_Type_Fill_In(ini, "EXPLOLG1", defaults, type);
	Check(type.Priority == 50, "priority 50");
	Sound_Type_Fill_In(ini, "SUPERWPN", defaults, type);
	Check(type.Priority == 75, "priority 75");
	Sound_Type_Fill_In(ini, "WHISPER", defaults, type);
	Check(type.Priority == 15, "priority 15");
	// The reader keeps no section without keys, so a keyless one reads as absent.
	Sound_Type_Fill_In(ini, "CLICK", defaults, type);
	Check(type.Priority == 10 && Is_Defaults(type) && std::strcmp(type.Sounds[0], "CLICK") == 0, "a keyless section takes the defaults");
	Check(!Sound_Type_Fill_In(ini, "MISSING", defaults, type) && type.Priority == 10 && Is_Defaults(type)
		&& std::strcmp(type.Sounds[0], "MISSING") == 0 && type.SoundCount == 1, "a missing section is the defaults named after it");
}


void Test_Yuris_Revenge_Shape(void)
{
	INIClass ini;
	Read(ini,
		"[General]\r\n"
		"Channels=24\r\n"
		"\r\n"
		"[Defaults]\r\n"
		"Volume=100\r\n"
		"MinVolume=0\r\n"
		"Priority=NORMAL\r\n"
		"Range=10\r\n"
		"Limit=5\r\n"
		"Type=SCREEN\r\n"
		"Control=NORMAL\r\n"
		"\r\n"
		"[MYLOOP]\r\n"
		"Sounds=LOOPIN LOOPBODY1 LOOPBODY2 LOOPOUT\r\n"
		"Priority=HIGH\r\n"
		"Volume=80\r\n"
		"MinVolume=20\r\n"
		"Range=20\r\n"
		"Limit=1\r\n"
		"Loop=0\r\n"
		"Delay=250 750\r\n"
		"FShift=-5 5\r\n"
		"VShift=10\r\n"
		"Attack=1\r\n"
		"Decay=1\r\n"
		"Type=SCREEN\r\n"
		"Control=LOOP RANDOM ATTACK DECAY\r\n"
		"\r\n"
		"[INHERITS]\r\n"
		"Priority=CRITICAL\r\n"
		"\r\n"
		"[GLOBALONE]\r\n"
		"Type=GLOBAL LOCAL\r\n"
		"Control=PREDELAY INTERRUPT\r\n"
		"Delay=1.5\r\n");

	Check(Sound_Type_Read_Channels(ini, 16) == 24, "[General] Channels");

	AudioEventTypeClass defaults;
	Sound_Type_Read_Defaults(ini, defaults);
	Check(Near(defaults.Volume, 1.0f) && defaults.Priority == 50 && defaults.Range == 10 && defaults.Limit == 5
		&& defaults.Type == SOUND_TYPE_SCREEN && defaults.Control == SOUND_CONTROL_NONE, "[Defaults] applied");

	AudioEventTypeClass type;
	Check(Sound_Type_Fill_In(ini, "MYLOOP", defaults, type), "loop section found");
	Check(type.SoundCount == 4 && std::strcmp(type.Sounds[0], "LOOPIN") == 0 && std::strcmp(type.Sounds[3], "LOOPOUT") == 0, "sound list");
	Check(type.Priority == 100, "named priority HIGH");
	Check(Near(type.Volume, 0.8f) && Near(type.MinVolume, 0.2f), "percent volumes");
	Check(type.Range == 20 && type.Limit == 1 && type.Loop == 0, "range, limit, loop");
	Check(type.DelayMin == 250 && type.DelayMax == 750, "delay pair in milliseconds");
	Check(type.FShiftMin == -5 && type.FShiftMax == 5, "pitch shift pair");
	Check(type.VShiftMin == -10 && type.VShiftMax == 0, "single volume shift attenuates");
	Check(type.AttackCount == 1 && type.DecayCount == 1, "attack and decay counts");
	Check(type.Type == SOUND_TYPE_SCREEN, "type flags");
	Check(type.Control == (SOUND_CONTROL_LOOP | SOUND_CONTROL_RANDOM | SOUND_CONTROL_ATTACK | SOUND_CONTROL_DECAY), "control flags");
	Check(type.Body_Start() == 1 && type.Body_Count() == 2 && type.Never_Ends(), "body range and endless loop");

	Sound_Type_Fill_In(ini, "INHERITS", defaults, type);
	Check(type.Priority == 255 && type.Range == 10 && type.Limit == 5 && Near(type.Volume, 1.0f), "omitted keys come from [Defaults]");

	Sound_Type_Fill_In(ini, "GLOBALONE", defaults, type);
	Check(type.Type == (SOUND_TYPE_GLOBAL | SOUND_TYPE_LOCAL), "two type flags");
	Check(type.Control == (SOUND_CONTROL_PREDELAY | SOUND_CONTROL_INTERRUPT), "two control flags");
	Check(type.DelayMin == 1500 && type.DelayMax == 1500, "a single delay with a point is seconds");
	Check(type.AttackCount == 0 && type.DecayCount == 0, "no attack or decay without the flags");
}


void Test_Vinifera_Shape(void)
{
	INIClass ini;
	Read(ini,
		"[VINI]\r\n"
		"Sounds=A,B,C\r\n"
		"LoopLimit=3\r\n"
		"Delay=0.25,0.75\r\n"
		"VShift=5 10\r\n"
		"FShift=10\r\n"
		"Type=UNSHROUDED\r\n"
		"Control=SEQUENTIAL QUEUE LOOP\r\n"
		"\r\n"
		"[HIDE]\r\n"
		"Type=SHROUDED\r\n"
		"Priority=2\r\n"
		"Volume=0.5\r\n");

	AudioEventTypeClass defaults;
	Sound_Type_Read_Defaults(ini, defaults);
	AudioEventTypeClass type;
	Sound_Type_Fill_In(ini, "VINI", defaults, type);
	Check(type.SoundCount == 3 && std::strcmp(type.Sounds[1], "B") == 0, "comma separated sounds");
	Check(type.Loop == 3, "LoopLimit is an alias of Loop");
	Check(type.DelayMin == 250 && type.DelayMax == 750, "delay pair in seconds");
	Check(type.VShiftMin == 5 && type.VShiftMax == 10, "a volume shift pair is a signed range");
	Check(type.FShiftMin == -10 && type.FShiftMax == 10, "a single pitch shift spans both ways");
	Check(type.Type == SOUND_TYPE_SHROUD, "UNSHROUDED is SHROUD");
	Check(type.Control == (SOUND_CONTROL_SEQUENTIAL | SOUND_CONTROL_QUEUE | SOUND_CONTROL_LOOP), "Vinifera control flags");
	Check(!type.Never_Ends(), "a counted loop ends");

	Sound_Type_Fill_In(ini, "HIDE", defaults, type);
	Check(type.Type == SOUND_TYPE_HIDDEN, "SHROUDED is the hidden type");
	Check(type.Priority == 2 && Near(type.Volume, 0.5f), "integer priority and fraction volume");
}


void Test_Values(void)
{
	Check(Sound_Type_Parse_Priority("LOWEST", 7) == 0 && Sound_Type_Parse_Priority("low", 7) == 10 && Sound_Type_Parse_Priority("Normal", 7) == 50
		&& Sound_Type_Parse_Priority("HIGH", 7) == 100 && Sound_Type_Parse_Priority("critical", 7) == 255, "priority names");
    Check(Sound_Type_Parse_Priority("300", 7) == 255 && Sound_Type_Parse_Priority("-5", 7) == 0 && Sound_Type_Parse_Priority("42", 7) == 42, "priority points clamp");
	Check(Sound_Type_Parse_Priority("bogus", 7) == 7 && Sound_Type_Parse_Priority("", 7) == 7, "unknown priority keeps the fallback");

	Check(Near(Sound_Type_Parse_Volume("1", 0.3f), 1.0f) && Near(Sound_Type_Parse_Volume("100", 0.3f), 1.0f), "1 and 100 are both full volume");
	Check(Near(Sound_Type_Parse_Volume("0.5", 0.3f), 0.5f) && Near(Sound_Type_Parse_Volume("50", 0.3f), 0.5f), "0.5 and 50 are both half");
	Check(Near(Sound_Type_Parse_Volume("150", 0.3f), 1.0f) && Near(Sound_Type_Parse_Volume("-1", 0.3f), 0.0f), "volume clamps");
	Check(Near(Sound_Type_Parse_Volume("loud", 0.3f), 0.3f), "unknown volume keeps the fallback");

	Check(Sound_Type_Parse_Type("NORMAL", 99) == 0, "NORMAL type is no flags");
	Check(Sound_Type_Parse_Type("SCREEN SHROUD", 99) == (SOUND_TYPE_SCREEN | SOUND_TYPE_SHROUD), "type flags combine");
	Check(Sound_Type_Parse_Type("SCREEN WIBBLE", 99) == SOUND_TYPE_SCREEN, "unknown type flags are ignored");
	Check(Sound_Type_Parse_Type("", 99) == 99, "empty type keeps the fallback");
	Check(Sound_Type_Parse_Type("VIOLENT MOVEMENT QUIET LOUD PLAYER NOISE_SHY GUN_SHY UNSHROUD AMBIENT", 0)
		== (SOUND_TYPE_VIOLENT | SOUND_TYPE_MOVEMENT | SOUND_TYPE_QUIET | SOUND_TYPE_LOUD | SOUND_TYPE_PLAYER | SOUND_TYPE_NOISE_SHY | SOUND_TYPE_GUN_SHY | SOUND_TYPE_UNSHROUD | SOUND_TYPE_AMBIENT), "reserved type flags are kept");
	Check(Sound_Type_Parse_Control("loop,all", 99) == (SOUND_CONTROL_LOOP | SOUND_CONTROL_ALL), "control flags with commas and lower case");
	Check(Sound_Type_Parse_Control("NORMAL", 99) == 0, "NORMAL control is no flags");

	int low;
	int high;
	Check(Sound_Type_Parse_Delay("500", low, high) && low == 500 && high == 500, "single delay");
	Check(Sound_Type_Parse_Delay("750 250", low, high) && low == 250 && high == 750, "reversed delay pair is sorted");
	Check(Sound_Type_Parse_Delay("1.5 2", low, high) && low == 1500 && high == 2000, "a point anywhere makes both seconds");
	Check(!Sound_Type_Parse_Delay("soon", low, high), "a word is not a delay");
	Check(Sound_Type_Parse_Shift("200", false, low, high) && low == -50 && high == 100, "pitch shift clamps to the pitch range");
	Check(Sound_Type_Parse_Shift("-20 -5", true, low, high) && low == -20 && high == -5, "volume shift pair");
	Check(Sound_Type_Parse_Shift("150", true, low, high) && low == -100 && high == 0, "volume shift clamps");
}


void Test_Limits(void)
{
	char text[4096];
	std::strcpy(text, "[General]\r\nChannels=64\r\n\r\n[MANY]\r\nSounds=");
	for (int i = 0; i < 40; i++) {
		char name[16];
		std::snprintf(name, sizeof(name), "S%d ", i);
		std::strcat(text, name);
	}
	std::strcat(text, "\r\nControl=ATTACK\r\nAttack=2\r\n\r\n[FEW]\r\nChannels=2\r\n\r\n[AVERYLONGSECTIONNAMETHATGOESONANDONANDON]\r\nPriority=1\r\n");

	INIClass ini;
	Read(ini, text);
	Check(Sound_Type_Read_Channels(ini, 16) == 32, "channels clamp high");

	AudioEventTypeClass defaults;
	Sound_Type_Read_Defaults(ini, defaults);
	AudioEventTypeClass type;
	Sound_Type_Fill_In(ini, "MANY", defaults, type);
	Check(type.SoundCount == 32 && std::strcmp(type.Sounds[31], "S31") == 0, "more than 32 sounds are truncated");
	Check(type.AttackCount == 2 && type.DecayCount == 0, "explicit attack count wins over the flag");

	INIClass few;
	Read(few, "[General]\r\nChannels=2\r\n");
	Check(Sound_Type_Read_Channels(few, 16) == 4, "channels clamp low");

	Sound_Type_Fill_In(ini, "AVERYLONGSECTIONNAMETHATGOESONANDONANDON", defaults, type);
	Check(std::strlen(type.Name) == 31 && std::strlen(type.Sounds[0]) == 31 && type.Priority == 1, "long names are cut to fit");
}

} // namespace


int main(void)
{
	Test_Tiberian_Sun_Shape();
	Test_Yuris_Revenge_Shape();
	Test_Vinifera_Shape();
	Test_Values();
	Test_Limits();

	std::printf("soundini: %d checks, %d failures\n", Checked, Failures);
	return(Failures == 0 ? 0 : 1);
}
