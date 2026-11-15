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
#include "dbgprint.h"
#include "ini.h"
#include "voc.h"
#include "xstraw.h"

#include "doctest/doctest.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

namespace {

float Distance(float value, float expect)
{
	return(std::fabs(value - expect));
}


void Read(INIClass & ini, char const * text)
{
	BufferStraw straw(text, (int)std::strlen(text));
	ini.Load(straw);
}


// Checks every key but the priority and the sounds against the engine defaults.
void Check_Defaults(AudioEventTypeClass const & type)
{
	CHECK(Distance(type.Volume, 1.0f) < 0.001f);
	CHECK(Distance(type.MinVolume, 0.0f) < 0.001f);
	CHECK(type.Range == 28);
	CHECK(type.Limit == 3);
	CHECK(type.Loop == 0);
	CHECK(type.DelayMin == 0);
	CHECK(type.DelayMax == 0);
	CHECK(type.FShiftMin == 0);
	CHECK(type.FShiftMax == 0);
	CHECK(type.VShiftMin == 0);
	CHECK(type.VShiftMax == 0);
	CHECK(type.AttackCount == 0);
	CHECK(type.DecayCount == 0);
	CHECK(type.Type == SOUND_TYPE_SCREEN);
	CHECK(type.Control == SOUND_CONTROL_NONE);
}


char const TiberianSun[] =
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
	"[CLICK]\r\n";


char const YurisRevenge[] =
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
	"Delay=1.5\r\n";


char const Vinifera[] =
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
	"Volume=0.5\r\n";


// A sound file and the [Defaults] it gives.
struct SoundFile
{
	INIClass ini;
	AudioEventTypeClass defaults;
	AudioEventTypeClass type;

	explicit SoundFile(char const * text)
	{
		Read(ini, text);
		VocClass::Read_Defaults(ini, defaults);
	}

	bool Read_Type(char const * name) { return(VocClass::Read_Type(ini, name, defaults, type)); }
};


struct TiberianSunFixture : SoundFile { TiberianSunFixture(void) : SoundFile(TiberianSun) {} };
struct YurisRevengeFixture : SoundFile { YurisRevengeFixture(void) : SoundFile(YurisRevenge) {} };
struct ViniferaFixture : SoundFile { ViniferaFixture(void) : SoundFile(Vinifera) {} };


// The many-sound, long-name file the limits are read from.
std::string Limits_Text(void)
{
	std::string text = "[General]\r\nChannels=64\r\n\r\n[MANY]\r\nSounds=";
	for (int i = 0; i < 40; i++) {
		char name[16];
		std::snprintf(name, sizeof(name), "S%d ", i);
		text += name;
	}
	text += "\r\nControl=ATTACK\r\nAttack=2\r\n\r\n[FEW]\r\nChannels=2\r\n\r\n[AVERYLONGSECTIONNAMETHATGOESONANDONANDON]\r\nPriority=1\r\n";
	return(text);
}

}


TEST_CASE_FIXTURE(TiberianSunFixture, "with no [Defaults] or [General], the engine defaults and the channel count stand")
{
	CHECK(defaults.Priority == 10);
	Check_Defaults(defaults);
	CHECK(VocClass::Read_Channels(ini, 16) == 16);
}


TEST_CASE_FIXTURE(TiberianSunFixture, "a Tiberian Sun section sets its priority, takes the defaults for the rest, and names its own sound")
{
	REQUIRE(Read_Type("GUN5"));
	CHECK(type.Priority == 100);
	Check_Defaults(type);
	CHECK(std::string(type.Name) == "GUN5");
	CHECK(type.SoundCount == 1u);
	CHECK(std::string(type.Sounds[0]) == "GUN5");
}


TEST_CASE_FIXTURE(TiberianSunFixture, "each Tiberian Sun section reads its own priority")
{
	Read_Type("EXPLOLG1");
	CHECK(type.Priority == 50);
	Read_Type("SUPERWPN");
	CHECK(type.Priority == 75);
	Read_Type("WHISPER");
	CHECK(type.Priority == 15);
}


// The reader keeps no section without keys, so a keyless one reads as absent.
TEST_CASE_FIXTURE(TiberianSunFixture, "a keyless section takes the defaults")
{
	Read_Type("CLICK");
	CHECK(type.Priority == 10);
	Check_Defaults(type);
	CHECK(std::string(type.Sounds[0]) == "CLICK");
}


TEST_CASE_FIXTURE(TiberianSunFixture, "a missing section is the defaults named after it")
{
	CHECK_FALSE(Read_Type("MISSING"));
	CHECK(type.Priority == 10);
	Check_Defaults(type);
	CHECK(std::string(type.Sounds[0]) == "MISSING");
	CHECK(type.SoundCount == 1u);
}


TEST_CASE_FIXTURE(YurisRevengeFixture, "[General] sets the channel count and [Defaults] applies")
{
	CHECK(VocClass::Read_Channels(ini, 16) == 24);
	CHECK(Distance(defaults.Volume, 1.0f) < 0.001f);
	CHECK(defaults.Priority == 50);
	CHECK(defaults.Range == 10);
	CHECK(defaults.Limit == 5);
	CHECK(defaults.Type == SOUND_TYPE_SCREEN);
	CHECK(defaults.Control == SOUND_CONTROL_NONE);
}


TEST_CASE_FIXTURE(YurisRevengeFixture, "a Yuri's Revenge loop section reads every key")
{
	REQUIRE(Read_Type("MYLOOP"));
	CHECK(type.SoundCount == 4u);
	CHECK(std::string(type.Sounds[0]) == "LOOPIN");
	CHECK(std::string(type.Sounds[3]) == "LOOPOUT");
	CHECK_MESSAGE(type.Priority == 100, "the named priority HIGH");
	CHECK_MESSAGE(Distance(type.Volume, 0.8f) < 0.001f, "the volumes are percentages");
	CHECK(Distance(type.MinVolume, 0.2f) < 0.001f);
	CHECK(type.Range == 20);
	CHECK(type.Limit == 1);
	CHECK(type.Loop == 0);
	CHECK_MESSAGE(type.DelayMin == 250, "the delay pair is in milliseconds");
	CHECK(type.DelayMax == 750);
	CHECK(type.FShiftMin == -5);
	CHECK(type.FShiftMax == 5);
	CHECK_MESSAGE(type.VShiftMin == -10, "a single volume shift attenuates");
	CHECK(type.VShiftMax == 0);
	CHECK(type.AttackCount == 1);
	CHECK(type.DecayCount == 1);
	CHECK(type.Type == SOUND_TYPE_SCREEN);
	CHECK(type.Control == (SOUND_CONTROL_LOOP | SOUND_CONTROL_RANDOM | SOUND_CONTROL_ATTACK | SOUND_CONTROL_DECAY));
	CHECK(type.Body_Start() == 1u);
	CHECK(type.Body_Count() == 2u);
	CHECK_MESSAGE(type.Never_Ends(), "the loop is endless");
}


TEST_CASE_FIXTURE(YurisRevengeFixture, "omitted keys come from [Defaults]")
{
	Read_Type("INHERITS");
	CHECK(type.Priority == 255);
	CHECK(type.Range == 10);
	CHECK(type.Limit == 5);
	CHECK(Distance(type.Volume, 1.0f) < 0.001f);
}


TEST_CASE_FIXTURE(YurisRevengeFixture, "two type and control flags combine, and a single delay with a point is in seconds")
{
	Read_Type("GLOBALONE");
	CHECK(type.Type == (SOUND_TYPE_GLOBAL | SOUND_TYPE_LOCAL));
	CHECK(type.Control == (SOUND_CONTROL_PREDELAY | SOUND_CONTROL_INTERRUPT));
	CHECK(type.DelayMin == 1500);
	CHECK(type.DelayMax == 1500);
	CHECK_MESSAGE(type.AttackCount == 0, "no attack or decay without the flags");
	CHECK(type.DecayCount == 0);
}


TEST_CASE_FIXTURE(ViniferaFixture, "a Vinifera section reads its comma lists, aliases and shift ranges")
{
	Read_Type("VINI");
	CHECK_MESSAGE(type.SoundCount == 3u, "comma separated sounds");
	CHECK(std::string(type.Sounds[1]) == "B");
	CHECK_MESSAGE(type.Loop == 3, "LoopLimit is an alias of Loop");
	CHECK_MESSAGE(type.DelayMin == 250, "the delay pair is in seconds");
	CHECK(type.DelayMax == 750);
	CHECK_MESSAGE(type.VShiftMin == 5, "a volume shift pair is a signed range");
	CHECK(type.VShiftMax == 10);
	CHECK_MESSAGE(type.FShiftMin == -10, "a single pitch shift spans both ways");
	CHECK(type.FShiftMax == 10);
	CHECK_MESSAGE(type.Type == SOUND_TYPE_SHROUD, "UNSHROUDED is SHROUD");
	CHECK(type.Control == (SOUND_CONTROL_SEQUENTIAL | SOUND_CONTROL_QUEUE | SOUND_CONTROL_LOOP));
	CHECK_MESSAGE(!type.Never_Ends(), "a counted loop ends");
}


TEST_CASE_FIXTURE(ViniferaFixture, "SHROUDED is the hidden type, with an integer priority and a fraction volume")
{
	Read_Type("HIDE");
	CHECK(type.Type == SOUND_TYPE_HIDDEN);
	CHECK(type.Priority == 2);
	CHECK(Distance(type.Volume, 0.5f) < 0.001f);
}


TEST_CASE("a priority is a name or clamped points, and anything else keeps the fallback")
{
	Debug_Init(0, (const char **)NULL);
	CHECK(Sound_Parse_Priority("LOWEST", 7) == 0);
	CHECK(Sound_Parse_Priority("low", 7) == 10);
	CHECK(Sound_Parse_Priority("Normal", 7) == 50);
	CHECK(Sound_Parse_Priority("HIGH", 7) == 100);
	CHECK(Sound_Parse_Priority("critical", 7) == 255);
	CHECK(Sound_Parse_Priority("300", 7) == 255);
	CHECK(Sound_Parse_Priority("-5", 7) == 0);
	CHECK(Sound_Parse_Priority("42", 7) == 42);
	CHECK(Sound_Parse_Priority("bogus", 7) == 7);
	CHECK(Sound_Parse_Priority("", 7) == 7);
}


TEST_CASE("a volume is a fraction or a percentage, clamped, and anything else keeps the fallback")
{
	CHECK(Distance(Sound_Parse_Volume("1", 0.3f), 1.0f) < 0.001f);
	CHECK(Distance(Sound_Parse_Volume("100", 0.3f), 1.0f) < 0.001f);
	CHECK(Distance(Sound_Parse_Volume("0.5", 0.3f), 0.5f) < 0.001f);
	CHECK(Distance(Sound_Parse_Volume("50", 0.3f), 0.5f) < 0.001f);
	CHECK(Distance(Sound_Parse_Volume("150", 0.3f), 1.0f) < 0.001f);
	CHECK(Distance(Sound_Parse_Volume("-1", 0.3f), 0.0f) < 0.001f);
	CHECK(Distance(Sound_Parse_Volume("loud", 0.3f), 0.3f) < 0.001f);
}


TEST_CASE("type flags combine, unknown ones are ignored, and an empty type keeps the fallback")
{
	Debug_Init(0, (const char **)NULL);
	CHECK_MESSAGE(Sound_Parse_Type("NORMAL", 99) == 0u, "NORMAL is no flags");
	CHECK(Sound_Parse_Type("SCREEN SHROUD", 99) == (SOUND_TYPE_SCREEN | SOUND_TYPE_SHROUD));
	CHECK(Sound_Parse_Type("SCREEN WIBBLE", 99) == SOUND_TYPE_SCREEN);
	CHECK(Sound_Parse_Type("", 99) == 99u);
	CHECK_MESSAGE(Sound_Parse_Type("VIOLENT MOVEMENT QUIET LOUD PLAYER NOISE_SHY GUN_SHY UNSHROUD AMBIENT", 0)
		== (SOUND_TYPE_VIOLENT | SOUND_TYPE_MOVEMENT | SOUND_TYPE_QUIET | SOUND_TYPE_LOUD | SOUND_TYPE_PLAYER | SOUND_TYPE_NOISE_SHY | SOUND_TYPE_GUN_SHY | SOUND_TYPE_UNSHROUD | SOUND_TYPE_AMBIENT),
		"the reserved type flags are kept");
}


TEST_CASE("control flags read with commas and in lower case, and NORMAL is no flags")
{
	CHECK(Sound_Parse_Control("loop,all", 99) == (SOUND_CONTROL_LOOP | SOUND_CONTROL_ALL));
	CHECK(Sound_Parse_Control("NORMAL", 99) == 0u);
}


TEST_CASE("a delay is one value or a sorted pair, in seconds when either has a point")
{
	int low;
	int high;
	REQUIRE(Sound_Parse_Delay("500", low, high));
	CHECK(low == 500);
	CHECK(high == 500);
	REQUIRE(Sound_Parse_Delay("750 250", low, high));
	CHECK(low == 250);
	CHECK(high == 750);
	REQUIRE(Sound_Parse_Delay("1.5 2", low, high));
	CHECK(low == 1500);
	CHECK(high == 2000);
	CHECK_MESSAGE(!Sound_Parse_Delay("soon", low, high), "a word is not a delay");
}


TEST_CASE("a pitch shift clamps to the pitch range, and a volume shift to its own")
{
	int low;
	int high;
	REQUIRE(Sound_Parse_Shift("200", false, low, high));
	CHECK(low == -50);
	CHECK(high == 100);
	REQUIRE(Sound_Parse_Shift("-20 -5", true, low, high));
	CHECK(low == -20);
	CHECK(high == -5);
	REQUIRE(Sound_Parse_Shift("150", true, low, high));
	CHECK(low == -100);
	CHECK(high == 0);
}


TEST_CASE("the channel count clamps high and low")
{
	INIClass many;
	std::string text = Limits_Text();
	Read(many, text.c_str());
	CHECK(VocClass::Read_Channels(many, 16) == 32);

	INIClass few;
	Read(few, "[General]\r\nChannels=2\r\n");
	CHECK(VocClass::Read_Channels(few, 16) == 4);
}


TEST_CASE("more than 32 sounds are truncated, and an explicit attack count wins over the flag")
{
	std::string text = Limits_Text();
	SoundFile file(text.c_str());
	file.Read_Type("MANY");
	CHECK(file.type.SoundCount == 32u);
	CHECK(std::string(file.type.Sounds[31]) == "S31");
	CHECK(file.type.AttackCount == 2);
	CHECK(file.type.DecayCount == 0);
}


TEST_CASE("long names are cut to fit")
{
	std::string text = Limits_Text();
	SoundFile file(text.c_str());
	file.Read_Type("AVERYLONGSECTIONNAMETHATGOESONANDONANDON");
	CHECK(std::strlen(file.type.Name) == 31u);
	CHECK(std::strlen(file.type.Sounds[0]) == 31u);
	CHECK(file.type.Priority == 1);
}
